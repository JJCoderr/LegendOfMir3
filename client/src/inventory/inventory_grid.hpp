#pragma once
#include <array>
#include <utility>
#include <cstdint>
#include <string>
#include <limits>
namespace mir3 {
struct BagItem {
    int kind = -1, icon = -1, count = 0, width = 1, height = 1;
    int std_mode=40;
    std::uint32_t make_index=0;
    std::string name;
    int stack_mode=0;
    explicit operator bool() const { return icon >= 0; }
};
class InventoryGrid {
public:
    static constexpr int columns = 6, rows = 100, capacity = 46;
    std::array<BagItem, capacity> items{};
    std::array<int, capacity> anchors{};
    std::array<short, columns * rows> cells{};
    BagItem held;
    int held_slot = -1;
    int held_equipment_slot = -1;
    bool pending_use=false;
    std::uint32_t last_release=0,last_use=0;
    int scroll_row = 0;
    InventoryGrid() { cells.fill(-1); anchors.fill(-1); }
    // 0x440130: half-open 38px cells, eight visible rows, +88 scroll row.
    int slot_at(int x, int y, int origin_x, int origin_y) const {
        x -= origin_x + 17; y -= origin_y + 60;
        if (x < 0 || y < 0 || x >= 228 || y >= 304) return -1;
        return x / 38 + 6 * (y / 38 + scroll_row);
    }
    int item_at(int slot) const {
        if (slot < 0 || slot >= static_cast<int>(cells.size()) || cells[slot] < 0) return -1;
        return cells[slot] % 1000; // 0x440220, +708 int16 occupancy map.
    }
    static std::pair<int,int> dimensions(int width, int height) {
        return {(width + 37) / 38, (height + 37) / 38}; // 0x4406B0
    }
    static std::array<int,4> placement(const BagItem& item, int slot, int mx, int my, int ox, int oy) {
        if (slot < 0 || slot >= columns*rows) return {-1,-1,item.width,item.height};
        int x = slot%columns, y = slot/columns;
        if (item.width%2 == 0 && mx > ox+17+38*x+19) ++x;
        if (item.height%2 == 0 && my > oy+60+38*y+19) ++y;
        return {x-item.width/2,y-item.height/2,item.width,item.height};
    }
    bool fits(int x, int y, const BagItem& item) const {
        if (item.width < 1 || item.height < 1 || x < 0 || y < 0 ||
            x + item.width > columns || y + item.height > rows) return false;
        for (int r = y; r < y + item.height; ++r)
            for (int c = x; c < x + item.width; ++c)
                if (cells[r * columns + c] != -1) return false;
        return true;
    }
    // 0x440CB0 -> 0x440280 requested placement, else 0x440420 first fit.
    bool insert(BagItem item, int slot = -1, int mouse_x = 0, int mouse_y = 0,
                int origin_x = 0, int origin_y = 0) {
        if (!item) return false;
        int id = 0;
        while (id < capacity && items[id]) ++id; // 0x440260
        if (id == capacity) return false;
        // Shared 0x440280 placement calculation also feeds the draw preview.
        const auto candidate = placement(item,slot,mouse_x,mouse_y,origin_x,origin_y);
        int x = candidate[0], y = candidate[1];
        if (!fits(x, y, item)) {
            bool found = false;
            for (y = 0; y < rows && !found; ++y)
                for (x = 0; x < columns; ++x)
                    if (fits(x,y,item)) { found = true; break; }
            if (!found) return false;
            --y;
        }
        items[id] = item; anchors[id] = y*columns+x;
        for (int r = y; r < y+item.height; ++r)
            for (int c = x; c < x+item.width; ++c)
                cells[r*columns+c] = static_cast<short>(id);
        cells[anchors[id]] = static_cast<short>(1000+id); // anchor marker
        return true;
    }
    bool restore_trade_item(const BagItem& returned) {
        if (returned.stack_mode>=2 && returned.make_index && returned.count>0) {
            for (auto& item:items) {
                if (!item || item.make_index!=returned.make_index || item.name!=returned.name ||
                    item.icon!=returned.icon || item.stack_mode!=returned.stack_mode) continue;
                if (item.count>std::numeric_limits<int>::max()-returned.count) return false;
                item.count+=returned.count;
                return true;
            }
        }
        return insert(returned);
    }
    BagItem take(int id) {
        if (id < 0 || id >= capacity || !items[id]) return {};
        auto item = items[id]; items[id] = {}; anchors[id] = -1;
        for (auto& cell : cells) if (cell == id || cell == id+1000) cell = -1;
        return item; // 0x440D50 -> 0x440BA0
    }
    bool release(int mouse_x, int mouse_y, int origin_x, int origin_y) {
        const int slot = slot_at(mouse_x,mouse_y,origin_x,origin_y);
        if (slot < 0) return false;
        const int id = item_at(slot);
        if (!held) {
            if (id >= 0) { held = take(id); held_slot = slot; } // 0x440F90
        } else {
            BagItem previous;
            if (id >= 0) previous = take(id); // 0x440E90 swap
            if (insert(held,slot,mouse_x,mouse_y,origin_x,origin_y)) { held = previous; held_slot = slot; }
            else if (previous) insert(previous,slot,mouse_x,mouse_y,origin_x,origin_y);
        }
        return true;
    }
    // 0x441310, normal bag source only. 1=request ready, 2=cooldown notice,
    // 0=ordinary release. A denied rapid click still follows the placement path.
    int release_action(int x, int y, int ox, int oy, std::uint32_t now) {
        if (pending_use) { last_release = now; return 0; }
        int result = 0;
        if (held && held.make_index && now-last_release < 300) {
            const auto elapsed = now-last_use;
            const bool medicine = held.std_mode == 0 || held.std_mode == 3;
            const bool allowed = !medicine || elapsed > 2000 ||
                (held.std_mode == 0 && elapsed > 1000 && (held.icon == 20 || held.icon == 21 || held.icon == 70));
            if (allowed) {
                last_release = now;
                if (medicine) last_use = now;
                // 0x4418D0: if another item is under the pointer, exchange first.
                const int slot = slot_at(x,y,ox,oy);
                if (item_at(slot) >= 0) release(x,y,ox,oy);
                if (held && held.make_index) { pending_use = true; return 1; }
                return 0;
            }
            result = 2;
        }
        last_release = now;
        release(x,y,ox,oy);
        return result;
    }
    // 0x4282C0, normal bag source. No client decrement: the reply supplies the
    // remaining quantity for mode3. Ordinary items are removed as a whole.
    bool apply_use_reply(int identifier, int remaining, int ox, int oy, int mouse_x = 0, int mouse_y = 0) {
        if (!pending_use || !held || (identifier != 635 && identifier != 636)) return false;
        bool restore = !(identifier == 635);
        if ((identifier == 635) && held.stack_mode == 3) {
            if (held.count <= 1) {
                // 0x440C00 removes first record matching makeIndex AND name.
                for (int i=0;i<capacity;++i) if (items[i] && items[i].make_index == held.make_index && items[i].name == held.name) {
                    take(i); break;
                }
            } else {
                // 0x441DD0 updates bag copies by makeIndex before restoring held.
                for (auto& item : items) if (item && item.make_index == held.make_index) item.count = remaining;
                held.count = remaining;
                restore = true;
            }
        }
        pending_use = false;
        if (restore && held && !insert(held,held_slot,mouse_x,mouse_y,ox,oy)) return true;
        held = {};
        return true;
    }
};
}
