#pragma once

#include "CuiLegacy/hud/chat_history.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace mir3_cui {

constexpr unsigned Say = 3030;

enum class ReceptionChannel { Private, Guild, All };

struct ChatReception {
    std::array<bool, 3> confirmed = {};
    std::array<bool, 3> displayed = {};
    std::string error;

    static const char* ToggleCommand(ReceptionChannel channel)
    {
        // V1.45 reception buttons send GBK commands through CM_SAY.
        switch (channel) {
        case ReceptionChannel::Private: return "@\xbe\xdc\xbe\xf8\xc7\xc4\xc7\xc4\xbb\xb0";
        case ReceptionChannel::Guild: return "@\xbe\xdc\xbe\xf8\xd0\xd0\xbb\xe1\xc1\xc4\xcc\xec";
        case ReceptionChannel::All: return "@\xbe\xdc\xbe\xf8\xcb\xf9\xd3\xd0\xc8\xcb\xc1\xc4\xcc\xec ";
        }
        return "";
    }

    void Request(ReceptionChannel channel)
    {
        const auto index = static_cast<std::size_t>(channel);
        displayed[index] = !confirmed[index];
    }

    void Apply(std::uint32_t flags, std::uint32_t changed)
    {
        constexpr std::uint32_t masks[] = {0x04000000u, 0x02000000u, 0x00800000u};
        for (std::size_t i = 0; i < confirmed.size(); ++i) {
            if (changed & masks[i]) displayed[i] = confirmed[i] = (flags & masks[i]) != 0;
        }
        // The all-chat confirmation also changes guild reception.
        if (changed & masks[2]) displayed[1] = confirmed[1] = confirmed[2];
    }
};

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
    mir3::ChatHistory nativeHistory;
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
    bool receptionOpen = false;
    ChatReception reception;
    std::string error;

    void Append(unsigned opcode, const std::string& text, unsigned ink = 0xffffff, unsigned backing = 0)
    {
        if (lines.size() == 79) lines.pop_front();
        lines.push_back({opcode, text});
        nativeHistory.post(text,ink,backing);
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
