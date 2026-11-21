#pragma once

#include <cstdint>
#include <cstdlib>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace mir3_cui {

constexpr unsigned GuildHome = 1036;
constexpr unsigned GuildMembers = 1037;
enum class GuildAction : unsigned {
    None, Add = 1038, Remove = 1039, Notice = 1040, Ranks = 1041, BreakAlliance = 1045
};

struct GuildOverview {
    std::string name;
    std::string partial;
    bool manager = false;
    int section = 0;
    std::vector<std::string> rows;
    std::map<std::size_t, int> headingIds;
    std::vector<std::string> editable;

    bool Apply(const std::string& body, bool initial)
    {
        if (body.empty() || body.size() > 8191 || (!initial && name.empty())) return false;
        GuildOverview next = initial ? GuildOverview{} : *this;
        if (initial) {
            const auto first = body.find('\r');
            if (first == std::string::npos || !first || first > 63) return false;
            const auto second = body.find('\r', first + 1);
            if (second == std::string::npos) return false;
            const auto third = body.find('\r', second + 1);
            if (third == std::string::npos) return false;
            next.name = body.substr(0, first);
            next.manager = body[second + 1] == '1';
            if (!next.ReadRows(body, third + 1, true)) return false;
        } else if (!next.ReadRows(body, 0, false)) {
            return false;
        }
        *this = std::move(next);
        return true;
    }

private:
    bool ReadRows(const std::string& body, std::size_t cursor, bool initial)
    {
        for (auto end = body.find('\r', cursor); end != std::string::npos; end = body.find('\r', cursor)) {
            if (rows.size() >= 2048) return false;
            const auto line = body.substr(cursor, end - cursor);
            cursor = end + 1;
            if (line.compare(0, 8, "<Notice>") == 0) {
                section = 1;
                headingIds.emplace(rows.size(), 4004);
                rows.emplace_back("Announcement");
            } else if (line == "<KillGuilds>" || line == "<AllyGuilds>") {
                section = line == "<KillGuilds>" ? 2 : 3;
                headingIds.emplace(rows.size(), section == 2 ? 4005 : 4006);
                rows.emplace_back(section == 2 ? "Rival guilds" : "Allied guilds");
            } else if (initial) {
                if (line.size() > 4095) return false;
                rows.push_back(line);
                if (section == 1 || section == 2) editable.push_back(line);
            } else {
                // Continuation keeps its accumulated fragment, including the second append for editable rows.
                if (partial.size() + line.size() > 4095) return false;
                partial += line;
                rows.push_back(partial);
                if (section == 1 || section == 2) {
                    if (partial.size() + line.size() > 4095) return false;
                    partial += line;
                    editable.push_back(partial);
                }
            }
        }
        if (initial) partial = body.substr(cursor);
        return partial.size() <= 4095;
    }
};

struct GuildRoster {
    std::vector<std::string> rows;
    std::vector<std::string> editable;
    bool received = false;

    bool Apply(const std::string& body, bool manager)
    {
        if (body.size() > 8191) return false;
        GuildRoster next;
        next.received = true;
        std::string line;
        long rank = 0;
        std::size_t cursor = 0;
        for (auto end = body.find('/'); end != std::string::npos; end = body.find('/', cursor)) {
            const auto token = body.substr(cursor, end - cursor);
            if (token.size() > 259) return false;
            cursor = end + 1;
            if (!token.empty() && token.front() == '#') {
                rank = std::strtol(token.c_str() + 1, nullptr, 10);
                line.clear();
            } else if (!token.empty() && token.front() == '*') {
                const auto title = '<' + token.substr(1) + '>';
                next.rows.push_back(manager ? '(' + std::to_string(rank) + ") " + title : title);
                next.editable.push_back('#' + std::to_string(rank) + ' ' + title);
                line.clear();
            } else {
                if (line.size() + token.size() + 1 > 259) return false;
                line += token + ' ';
                if (line.size() > 60 || cursor == body.size() || body[cursor] == '#') {
                    next.rows.push_back(line);
                    next.editable.push_back(line);
                    line.clear();
                }
            }
        }
        *this = std::move(next);
        return true;
    }
};

inline int GuildNoticeId(unsigned opcode, std::int64_t reason)
{
    switch (opcode) {
    case 754: return 1473;
    case 757: return 1474;
    case 758: return reason >= 1 && reason <= 5 ? 1474 + static_cast<int>(reason) : 0;
    case 760: return reason == 1 ? 1475 : (reason >= 2 && reason <= 4 ? 1478 + static_cast<int>(reason) : 0);
    case 761: return reason >= -7 && reason <= -2 ? 1481 - static_cast<int>(reason) : 0;
    case 762: return 1504;
    case 763: return reason >= -3 && reason <= -1 ? 1504 - static_cast<int>(reason) : 0;
    case 768: case 769: return reason >= -4 && reason <= -1 ? 1488 - static_cast<int>(reason) : 0;
    case 770: case 771: return reason == -1 ? 1489 : (reason >= -3 && reason <= -2 ? 1491 - static_cast<int>(reason) : 0);
    default: return 0;
    }
}

inline const char* GuildNoticeText(int id)
{
    if (id == 1473) return "You do not belong to a guild.";
    if (id == 1474) return "The member was added.";
    if (id >= 1475 && id <= 1482) return "The guild member request failed.";
    if (id >= 1483 && id <= 1488) return "The rank update failed.";
    if (id >= 1489 && id <= 1494) return "The server responded to the alliance request.";
    if (id == 1504) return "The guild was created.";
    if (id >= 1505 && id <= 1507) return "The guild could not be created.";
    if (id == 4000) return "Guild manager permission is required.";
    return "The server reported a guild event.";
}

struct GuildPanel {
    struct HistoryRow {
        int messageId = 0;
        std::string text;
    };
    bool open = false;
    bool requested = false;
    int tab = 0;
    int first = 0;
    GuildAction action = GuildAction::None;
    GuildOverview overview;
    GuildRoster roster;
    std::vector<HistoryRow> history;
    unsigned overviewRevision = 0;
    int notice = 0;
    int editor = 0;
    char input[16000] = {};
    std::string error;

    bool Multiline() const { return action == GuildAction::Notice || action == GuildAction::Ranks; }

    void AppendHistory(HistoryRow row)
    {
        if (static_cast<int>(history.size())-first == 18) ++first;
        history.push_back(std::move(row));
        if (history.size() >= 80) {
            history.erase(history.begin());
            if (first > 0) --first;
        }
    }

    void Begin(GuildAction next)
    {
        action = next;
        input[0] = '\0';
        error.clear();
        ++editor;
    }
};

} // namespace mir3_cui
