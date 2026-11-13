#pragma once
#include "chat_scroll.hpp"
#include <array>
#include <deque>
#include <string>
#include <string_view>
namespace mir3 {
// 0x4366D0 record: COLORREF +0/+4, ANSI text +8 (allocation 0x408).
struct ChatLine {
    std::string text;
    std::uint32_t foreground = 0xffffff, background = 0;
};
class ChatHistory {
public:
    void post(std::string_view utf8, std::uint32_t foreground = 0xffffff, std::uint32_t background = 0);
    const std::deque<ChatLine>& lines(bool expanded) const { return logs[expanded]; }
    ChatScroll& scroll(bool expanded) { return scrolls[expanded]; }
    const ChatScroll& scroll(bool expanded) const { return scrolls[expanded]; }
    bool input(int kind, int x, int y, bool expanded, int width, int height, std::uint32_t now) {
        auto& control = scrolls[expanded];
        const int previous = control.position();
        const bool consumed = control.input(kind, x, y, static_cast<int>(logs[expanded].size()),
            expanded, width, height, now);
        if (consumed && kind == 0) {
            // 0x43BFCC..0x43BFE3 always writes expanded +0xE00 using +0xE30,
            // even when the selected scrollbar is the compact one (+0x6A14).
            control.set_position(previous);
            scrolls[1].set_position(static_cast<int>((static_cast<int>(logs[1].size()) - 1) * control.fraction()));
        }
        return consumed;
    }
    int first(bool expanded) const { return scrolls[expanded].first(static_cast<int>(logs[expanded].size()), expanded); }
private:
    std::array<std::deque<ChatLine>, 2> logs;
    std::array<ChatScroll, 2> scrolls;
};
}
