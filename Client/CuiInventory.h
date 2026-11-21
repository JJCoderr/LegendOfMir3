#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace mir3_cui {

enum class ItemRequest { None, Use, Drop, DropStack, DropGold };
enum class DropDialog { None, Item, Gold };

struct ItemKey {
    std::uint32_t id = 0;
    std::string name;

    bool operator==(const ItemKey& other) const { return id == other.id && name == other.name; }
};

struct InventoryItem {
    ItemKey key;
    std::string label;
    unsigned icon = 0;
    unsigned count = 0;
    unsigned durability = 0;
    unsigned maxDurability = 0;
    unsigned mode = 0;
    unsigned subMode = 0;
    unsigned stackMode = 0;
    int cell = -1;
    int width = 1, height = 1;

    bool Stacks() const { return stackMode >= 2; }
    bool IsGold() const { return key.name == "\xbd\xf0\xb1\xd2"; }
    bool Medicine() const { return mode == 0 || mode == 3; }
    bool QuickEligible() const { return Medicine() && !IsGold(); }
};

struct MerchantService {
    enum class Kind { Sell, Repair, Deposit };
    enum class Phase { Choosing, Quoting, Quoted, Unavailable, Paying };

    std::uint32_t actor = 0;
    unsigned revision = 0;
    Kind kind = Kind::Sell;
    Phase phase = Phase::Choosing;
    ItemKey item;
    std::uint32_t price = 0;
    int notice = 0;
    bool quantityOpen = false;
    int editor = 0;
    char quantity[32]{};
};

struct MerchantResult {
    int message = 0;
    std::string item;
    unsigned quantity = 0;
};

struct InventoryPanel {
    static constexpr int Columns = 6;
    static constexpr int Cells = Columns * 100;
    static constexpr std::size_t Capacity = 46;
    using Occupancy = std::array<bool, Cells>;
    std::vector<InventoryItem> items;
    ItemKey selection;
    ItemKey returningEquipment;
    int returnCell = -1;
    InventoryItem pending;
    std::uint32_t gold = 0;
    std::uint64_t deadline = 0;
    std::uint64_t lastUse = 0;
    unsigned dropCount = 0;
    ItemRequest request = ItemRequest::None;
    DropDialog drop = DropDialog::None;
    char amount[32] = {};
    bool sceneDrop = false;
    int editor = 0;
    bool open = false;
    bool heldGold = false;
    bool initialized = false;
    bool waiting = false;
    bool notice = false;
    std::string status;
    std::array<ItemKey, 6> shortcuts;
    int heldShortcut = -1, pendingShortcut = -1;
    bool shortcutsVisible = false;
    bool bindingShortcut = false;
    int clickedShortcut = -1;
    std::uint64_t shortcutClickTime = 0;
    std::string shortcutStatus;

    InventoryItem* ResolveShortcut(std::size_t slot)
    {
        const auto& key = shortcuts[slot];
        if (key.name.empty()) return nullptr;
        if (auto* exact = Find(key)) return exact->QuickEligible() ? exact : nullptr;
        const auto same = std::find_if(items.rbegin(), items.rend(), [&](const InventoryItem& item) {
            return item.key.name == key.name && item.QuickEligible();
        });
        return same == items.rend() ? nullptr : &*same;
    }

    InventoryItem* Find(const ItemKey& key)
    {
        const auto item = std::find_if(items.begin(), items.end(), [&](const InventoryItem& value) {
            return value.key == key;
        });
        return item == items.end() ? nullptr : &*item;
    }

    InventoryItem* AtCell(int cell)
    {
        const auto item = std::find_if(items.begin(), items.end(), [&](const InventoryItem& value) {
            return OnGrid(value, value.cell) && cell >= 0 && cell < Cells &&
                cell % Columns >= value.cell % Columns && cell % Columns < value.cell % Columns + value.width &&
                cell / Columns >= value.cell / Columns && cell / Columns < value.cell / Columns + value.height;
        });
        return item == items.end() ? nullptr : &*item;
    }

    static bool OnGrid(const InventoryItem& item, int cell)
    {
        return cell >= 0 && cell < Cells && item.width > 0 && item.height > 0 &&
            item.width <= Columns - cell % Columns && item.height <= Cells / Columns - cell / Columns;
    }

    static bool Fits(const Occupancy& occupied, const InventoryItem& item, int cell)
    {
        if (!OnGrid(item, cell)) return false;
        for (int y = 0; y < item.height; ++y)
            for (int x = 0; x < item.width; ++x)
                if (occupied[cell + y * Columns + x]) return false;
        return true;
    }

    static void Occupy(Occupancy& occupied, const InventoryItem& item, int cell)
    {
        for (int y = 0; y < item.height; ++y)
            for (int x = 0; x < item.width; ++x)
                occupied[cell + y * Columns + x] = true;
    }

    Occupancy Occupied(const InventoryItem* first = nullptr, const InventoryItem* second = nullptr) const
    {
        Occupancy occupied{};
        for (const auto& item : items)
            if (&item != first && &item != second && OnGrid(item, item.cell)) Occupy(occupied, item, item.cell);
        return occupied;
    }

    static int FirstFit(const Occupancy& occupied, const InventoryItem& item, int preferred = -1)
    {
        if (Fits(occupied, item, preferred)) return preferred;
        for (int cell = 0; cell < Cells; ++cell)
            if (Fits(occupied, item, cell)) return cell;
        return -1;
    }

    bool Move(InventoryItem& item, InventoryItem* target, int cell)
    {
        auto occupied = Occupied(&item, target);
        cell = FirstFit(occupied, item, cell);
        if (cell < 0) return false;
        Occupy(occupied, item, cell);
        if (target) {
            const int destination = FirstFit(occupied, *target, item.cell);
            if (destination < 0) return false;
            target->cell = destination;
        }
        item.cell = cell;
        return true;
    }

    void Replace(std::vector<InventoryItem> records)
    {
        Occupancy occupied{};
        for (auto& record : records) {
            if (Fits(occupied, record, record.cell)) Occupy(occupied, record, record.cell);
            else record.cell = -1;
        }
        for (auto& record : records) {
            if (record.cell >= 0) continue;
            const auto* existing = Find(record.key);
            if (existing && Fits(occupied, record, existing->cell)) {
                record.cell = existing->cell;
                Occupy(occupied, record, record.cell);
            }
        }
        for (auto& record : records) {
            if (record.cell >= 0) continue;
            record.cell = FirstFit(occupied, record);
            if (record.cell >= 0) Occupy(occupied, record, record.cell);
        }
        items = std::move(records);
    }

    void Remove(const ItemKey& key)
    {
        const auto item = std::find_if(items.begin(), items.end(), [&](const InventoryItem& value) {
            return value.key == key;
        });
        if (item != items.end()) items.erase(item);
        if (selection == key) selection = {};
    }
};

} // namespace mir3_cui
