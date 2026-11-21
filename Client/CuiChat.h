#pragma once

#include <algorithm>
#include <deque>
#include <string>
#include <vector>

namespace mir3_cui {

constexpr unsigned Say = 3030;

inline const char* ChatChannel(unsigned opcode)
{
    switch (opcode) {
    case 40: return "Nearby";
    case 100: return "System";
    case 101: return "Party";
    case 102: return "Shout";
    case 103: return "Private";
    case 104: return "Guild";
    case 1501: return "Creature";
    default: return nullptr;
    }
}

struct ChatLine {
    unsigned opcode;
    std::string text;
};

struct ChatPanel {
    std::deque<ChatLine> lines;
    std::vector<std::string> contacts;
    std::vector<std::string> phrases;
    char draft[361] = {};
    int channel = 0;
    int editor = 0;
    int contactIndex = 0;
    int phraseIndex = 0;
    bool composing = true;
    bool expanded = false;
    bool scrollToEnd = false;
    bool logging = false;
    std::string error;

    void Append(unsigned opcode, const std::string& text)
    {
        if (lines.size() == 79) lines.pop_front();
        lines.push_back({opcode, text});
        scrollToEnd = true;
    }

    static bool Remember(std::vector<std::string>& history, const std::string& text)
    {
        if (text.empty() || std::find(history.begin(), history.end(), text) != history.end())
            return false;
        if (history.size() == 10) history.erase(history.begin());
        history.push_back(text);
        return true;
    }

    void FillDraft(const std::string& text)
    {
        if (text.size() >= sizeof draft) return;
        text.copy(draft, text.size());
        draft[text.size()] = '\0';
        ++editor;
    }
};

} // namespace mir3_cui
