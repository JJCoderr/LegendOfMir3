#pragma once
#include <algorithm>
#include <cstdint>
namespace mir3 {
// 0x435820 -> 0x41FE80 (frame/page/width/travel/step/orientation).
// 0x4386E3..0x438729 supplies x/y to 0x41FED0 each frame.
struct ChatScrollLayout {
    static constexpr int frame=1207, x=619, page_step=12;
    static constexpr int top(bool expanded) { return expanded?342:507; }
    static constexpr int travel(bool expanded) { return expanded?203:43; }
    static constexpr int rows(bool expanded) { return expanded?14:5; }
    static int thumb_y(int first,int count,bool expanded) {
        const float ratio=count>1 ? static_cast<float>(static_cast<double>(std::clamp(first,0,count-1))/(count-1)):0.0f;
        return static_cast<int>(top(expanded)+travel(expanded)*static_cast<double>(ratio));
    }
};
class ChatScroll {
public:
    int first(int count, bool expanded) const {
        return std::clamp(index, 0, std::max(0, count - 1));
    }
    int position() const { return index; }
    float fraction() const { return ratio; }
    void set_position(int value) { index = value; }
    void latest(int count, bool expanded) {
        index = std::max(0, count - ChatScrollLayout::rows(expanded));
        dragging = false;
    }
    void wheel(int direction, int count, bool expanded) {
        index = std::clamp(first(count, expanded) - direction, 0,
            std::max(0, count - ChatScrollLayout::rows(expanded)));
    }
    // Vertical branch of 0x41FED0, 0x420200, 0x420180, 0x420360.
    // kind 0=down, 1=motion, 2=up. Width/height come from original frame1207.
    bool input(int kind, int x, int y, int count, bool expanded,
        int width, int height, std::uint32_t now) {
        const int top = ChatScrollLayout::top(expanded);
        const int travel = ChatScrollLayout::travel(expanded);
        const int bottom = top + height + travel;
        if (kind == 2) {
            last_time = now;
            const bool captured = dragging;
            dragging = false;
            return captured;
        }
        if (kind == 1) {
            if (!dragging) return false;
            ratio = std::clamp(static_cast<float>(y - top) / (bottom - top), 0.0f, 1.0f);
            index = static_cast<int>(std::max(0, count - 1) * ratio);
            return true;
        }
        if (kind != 0 || width <= 0 || height <= 0 || x < ChatScrollLayout::x || x >= ChatScrollLayout::x + width || y < top || y >= bottom) return false;
        ratio = count > 1 ? static_cast<float>(first(count, expanded)) / (count - 1) : 0.0f;
        if (now - last_time <= 10) return true;
        const int thumb_top = top + static_cast<int>(travel * static_cast<double>(ratio));
        last_time = now;
        if (y >= thumb_top && y < thumb_top + height) { dragging = true; return true; }
        const float page = count > 0 ? static_cast<float>(ChatScrollLayout::rows(expanded)) / count : 0.0f;
        ratio = std::clamp(ratio + (y < thumb_top + (ChatScrollLayout::page_step>>1) ? -page : page), 0.0f, 1.0f);
        index = static_cast<int>(std::max(0, count - 1) * ratio);
        return true;
    }
private:
    int index = 0;
    float ratio = 0;
    bool dragging = false;
    std::uint32_t last_time = 0;
};
// 0x435820 (43/203 travel), 0x438000 (507/342 origins), 0x41FED0.
inline int chat_thumb_y(int first, int count, bool expanded)
{
    return ChatScrollLayout::thumb_y(first,count,expanded);
}
}
