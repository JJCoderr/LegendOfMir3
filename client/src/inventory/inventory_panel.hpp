#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <utility>
namespace mir3 {
struct DraggablePanel {
    bool visible = false;
    std::array<int, 4> rect{};
    bool dragging = false;
    int drag_x = 0, drag_y = 0;
    explicit DraggablePanel(std::array<int, 4> initial) : rect(initial) {}
    void toggle() { visible = !visible; dragging = false; }
    bool begin_drag(int mouse_x, int mouse_y, bool on_button) {
        if (!visible || on_button || mouse_x < rect[0] || mouse_y < rect[1] ||
            mouse_x >= rect[0] + rect[2] || mouse_y >= rect[1] + rect[3]) return false;
        // 0x43B6A0 -> 0x4319F0: panel+72/+76 stores the grab offset.
        drag_x = mouse_x - rect[0]; drag_y = mouse_y - rect[1];
        dragging = true;
        return true;
    }
    bool move_drag(int mouse_x, int mouse_y) {
        if (!visible || !dragging) return false;
        // 0x43B370 -> 0x4318D0, preserve the unusual 570 -> 565 threshold.
        rect[0] = mouse_x - drag_x;
        rect[1] = mouse_y - drag_y;
        if (rect[1] + rect[3] > 570) rect[1] = 565 - rect[3];
        return true;
    }
    bool end_drag() { const bool captured = dragging; dragging = false; return captured; }
};
// 0x43FA00/0x43FB10: frame1225, page8, travel254, threshold14,
// track origin window+(250,70). Deliberately retain draw divisor91 vs
// input multiplier92 (0x441230/0x441A10, float at0x4AF930).
class InventoryScroll {
public:
    bool captured = false;
    std::uint32_t last_time = 0;
    static int thumb_y(int row, int origin_y) {
        const float ratio = static_cast<float>(row) / 91;
        return static_cast<int>(origin_y + 70 + 254.0 * ratio);
    }
    bool input(int kind, int x, int y, int origin_x, int origin_y,
               int width, int height, std::uint32_t now, int& row) {
        const int top = origin_y+70, bottom = top+height+254;
        if (kind == 2) {
            last_time = now;
            return std::exchange(captured,false);
        }
        if (kind == 1) {
            if (!captured || height <= 0) return false;
            const float ratio = std::clamp(static_cast<float>(y-top)/(bottom-top),0.0f,1.0f);
            row = static_cast<int>(ratio*92.0f);
            return true;
        }
        if (kind != 0 || width <= 0 || height <= 0 || x < origin_x+250 ||
            x >= origin_x+250+width || y < top || y >= bottom) return false;
        if (now-last_time <= 10) return true;
        last_time = now;
        const int thumb = thumb_y(row,origin_y);
        if (y >= thumb && y < thumb+height) { captured = true; return true; }
        float ratio = static_cast<float>(row)/91;
        ratio = std::clamp(ratio + (y < thumb+7 ? -8.0f/92 : 8.0f/92),0.0f,1.0f);
        row = static_cast<int>(ratio*92.0f);
        return true;
    }
};
struct InventoryPanel : DraggablePanel {
    InventoryPanel() : DraggablePanel({518,0,282,466}) {}
    InventoryScroll scroll;
    int row=0;
    bool close_pressed=false;
};
}
