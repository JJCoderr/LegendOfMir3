#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mir3_cui {

constexpr unsigned PartyMode = 1019;
enum class PartyAction : unsigned { None, Create = 1020, Invite = 1021, Remove = 1022 };

enum class PartyReply : unsigned {
    Created = 660, CreateFailed = 661, InviteFailed = 664, RemoveFailed = 665,
    Cancelled = 666, Roster = 667
};

struct PartyNotice {
    const char* action = nullptr;
    int id = 0;
    int reason = 0;
};

inline PartyNotice PartyFailure(PartyReply reply, std::uint32_t value)
{
    if (value < 0xfffffffbu) return {};
    const int reason = static_cast<int>(0xffffffffu - value) + 1;
    switch (reply) {
    case PartyReply::CreateFailed:
        if (reason <= 4) return {"Create party", 1494 + reason, -reason};
        break;
    case PartyReply::InviteFailed: {
        constexpr int ids[] = {1499, 1500, 1501, 1498, 1502};
        return {"Invite member", ids[reason - 1], -reason};
    }
    case PartyReply::RemoveFailed:
        if (reason <= 3) return {"Remove member", reason == 3 ? 1503 : 1498 + reason, -reason};
        break;
    default:
        break;
    }
    return {};
}

struct PartyPanel {
    bool open = false;
    bool allowed = false;
    PartyAction action = PartyAction::None;
    PartyNotice notice;
    std::vector<std::string> members;
    char name[125] = {};
    int editor = 0;
    bool cancelSelected = false;
    bool tabDown = false;
    bool editorFocused = false;
    std::string error;

    void Begin(PartyAction next)
    {
        action = next;
        name[0] = '\0';
        error.clear();
        cancelSelected = false;
        tabDown = false;
        editorFocused = next != PartyAction::None;
        ++editor;
    }
};

} // namespace mir3_cui
