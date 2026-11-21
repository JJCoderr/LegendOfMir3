#pragma once
#include <array>

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
}
