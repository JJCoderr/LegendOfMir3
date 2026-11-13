#pragma once
#include <string>
#include <string_view>
#include <array>
namespace mir3 {
// 0x47B880: CreateFontA(-12, ..., 400, ..., DEFAULT_CHARSET),
// WM_SETFONT (48), EM_LIMITTEXT (197) with wParam=90.
struct ChatInputSpec {
    static constexpr int font_pixels = 12;
    static constexpr int font_weight = 400;
    static constexpr int limit = 90;
    static constexpr int font_message = 1074;
};
struct ChatMode {
    int mode = 0; // 0x435180 this+0x7258; 0 normal, 1 group, 2 guild.
    bool changed = false;
    int toggle(int button) {
        if (button != 18 && button != 19) return -1;
        const int requested = button == 18 ? 1 : 2;
        mode = mode == requested ? 0 : requested;
        changed = true;
        return mode == 0 ? 2626 : (mode == 1 ? 2633 : 2635);
    }
    int frame(int button) const {
        if (!changed) return -1;
        return button == 18 ? (mode == 1 ? 1335 : 1334) : (mode == 2 ? 1337 : 1336);
    }
    int tooltip(int button) const {
        return button == 18 ? (mode == 1 ? 2631 : 2618) : (mode == 2 ? 2630 : 2619);
    }
    std::string_view prefix() const { return mode == 1 ? "!!" : (mode == 2 ? "!~" : ""); }
};
struct ChatInputEdit {
    std::string text;
    int caret = 0;
    int selection_from = -1, selection_to = -1;
};
class ChatInput {
public:
    int byte_limit = ChatInputSpec::limit;
    const std::string& set(std::string_view text);
    ChatInputEdit edit(std::string_view candidate, int caret);
private:
    std::string value;
};
std::string limit_chat_input(std::string_view utf8, int byte_limit = ChatInputSpec::limit);
}
