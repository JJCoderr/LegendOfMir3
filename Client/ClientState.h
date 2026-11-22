#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <xaudio2.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cerrno>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <filesystem>
#include <fstream>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "cui.h"
#include "CuiLegacy/hud/ground_items.hpp"
#include "CuiLegacy/hud/casting.hpp"
#include "CuiLegacy/hud/local_action_clock.hpp"
#include "CuiLegacy/hud/melee_cooldown.hpp"
#include "CuiLegacy/magic/melee_spells.hpp"
#include "CuiLegacy/hud/level_up_effect.hpp"
#include "CuiLegacy/magic/spell_color.hpp"
#include "CuiLegacy/magic/spell_effects.hpp"
#include "CuiLegacy/magic/spell_sound_voices.hpp"
#include "CuiLegacy/magic/spell_timing.hpp"
#include "CuiLegacy/hud/sound_position.hpp"
#include "CuiLegacy/resources/sound_list.hpp"
#include "CuiLegacy/hud/damage_numbers.hpp"
#include "CuiLegacy/hud/monster_motion.hpp"
#include "CuiLegacy/hud/scene_lighting.hpp"
#include "CuiLegacy/hud/scene_name.hpp"
#include "CuiLegacy/hud/speech_bubble.hpp"
#include "CuiLegacy/hud/snow_weather.hpp"
#include "Common/EnDecode.h"
#include "Common/Protocol.h"

// Client data and audio declarations.
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
    unsigned groundLooks = 0;
    std::array<std::uint8_t,100> serverRecord{};

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

} // namespace mir3_cui

namespace mir3_cui {

constexpr int EquipmentSlots = 12;
enum class EquipmentRequest { None, Wear, Remove };

inline bool EquipmentFits(unsigned mode, int slot)
{
    switch (mode) {
    case 5: case 6: case 54: case 55: case 56: return slot == 1;
    case 10: case 11: return slot == 0;
    case 15: return slot == 4;
    case 19: case 20: case 21: case 57: return slot == 3;
    case 22: case 23: return slot == 7 || slot == 8;
    case 24: case 26: return slot == 5 || slot == 6;
    case 25: return slot == 5 || slot == 9;
    case 30: return slot == 2;
    case 53: return slot == 10;
    case 60: return slot == 11;
    default: return false;
    }
}

inline bool EquipmentActivates(const InventoryItem& item, int slot)
{
    return !item.key.name.empty() && EquipmentFits(item.mode, slot) &&
        item.mode != 5 && item.mode != 6 && item.mode != 10 && item.mode != 11 &&
        item.mode != 15 && item.mode != 53;
}

} // namespace mir3_cui

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

} // namespace mir3_cui

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

} // namespace mir3_cui

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

} // namespace mir3_cui

namespace mir3_cui {

constexpr unsigned QuestQuery = 1046;
constexpr unsigned QuestChildrenQuery = 1048;
constexpr unsigned QuestTextQuery = 1049;
enum class QuestReply : unsigned { Full = 1301, Added = 1302, ChildAdded = 1303, Children = 1310, Text = 1311 };

struct QuestRecord {
    int id = 0;
    std::string title;
    int childCount = 0;
    bool completed = false;
    std::string summary;
};

struct QuestEntry {
    QuestRecord record;
    std::vector<QuestRecord> children;
    bool childrenLoaded = false;

    QuestRecord* FindChild(int id)
    {
        for (auto& child : children) if (child.id == id) return &child;
        return nullptr;
    }
};

inline bool DecodeQuestRecords(QuestReply reply, const std::string& body, unsigned count,
    int& parent, std::vector<QuestRecord>& result)
{
    if (count > body.size() || body.size() > 8191) return false;
    const bool full = reply == QuestReply::Full;
    const bool children = reply == QuestReply::Children;
    const bool childAdded = reply == QuestReply::ChildAdded;
    std::size_t next = 0;
    const auto take = [&](std::string& field, bool allowEnd = false) {
        const auto slash = body.find('/', next);
        if (slash == std::string::npos && !allowEnd) return false;
        field = body.substr(next, slash == std::string::npos ? slash : slash - next);
        next = slash == std::string::npos ? body.size() : slash + 1;
        return true;
    };
    const auto number = [&](int& output, bool allowEnd = false) {
        std::string field;
        if (!take(field, allowEnd) || field.empty() ||
            (field.front() != '-' && (field.front() < '0' || field.front() > '9'))) return false;
        char* end = nullptr;
        errno = 0;
        const auto value = std::strtol(field.c_str(), &end, 10);
        if (*end || errno == ERANGE) return false;
        output = static_cast<int>(value);
        return true;
    };
    if (childAdded && !number(parent)) return false;

    std::vector<QuestRecord> records;
    records.reserve(count);
    for (unsigned i = 0; i < count; ++i) {
        QuestRecord record;
        const bool last = i + 1 == count;
        if (!number(record.id) || record.id < 0 ||
            !take(record.title, last && !full && !children && !childAdded) ||
            record.title.empty() || record.title.size() > 511)
            return false;
        bool hasSummary = childAdded;
        if (full) {
            if (!number(record.childCount) || record.childCount < 0) return false;
            hasSummary = body.find('/', next) != std::string::npos;
            int completed = 0;
            if (!number(completed, last)) return false;
            record.completed = completed != 0;
        }
        if (hasSummary && (!take(record.summary, last) || record.summary.size() > 1023))
            return false;
        records.push_back(std::move(record));
    }
    if (next != body.size()) return false;
    result = std::move(records);
    return true;
}

} // namespace mir3_cui

namespace mir3_cui {

struct CharacterAppearance {
    bool available = false;
    unsigned body = 0;
    unsigned weapon = 0;
    unsigned clothing = 0;
    unsigned hair = 0;
    unsigned profession = 0;

    bool Apply(std::uint32_t feature)
    {
        const auto type = feature & 0xff;
        const auto dress = (feature >> 16) & 0xff;
        if (type > 1 || dress >= 9) return false;

        const auto weaponId = (feature >> 8) & 0xff;
        const auto hairId = (feature >> 24) & 0x3f;
        available = true;
        body = type;
        weapon = weaponId < 50 ? weaponId : 0;
        clothing = dress;
        hair = hairId < 30 ? hairId : 0;
        profession = feature >> 30;
        return true;
    }
};

struct CharacterStatus {
    using Range = std::array<unsigned, 2>;

    bool received = false;
    unsigned level = 0;
    std::uint32_t experience = 0;
    std::uint32_t nextExperience = 0;
    Range hp{}, mp{}, weight{}, wear{}, hand{};
    std::array<Range, 5> combat{};
    std::array<std::array<unsigned, 7>, 3> elements{};
    std::array<std::uint16_t,7> attackElements{};
    unsigned accuracy = 0;
    unsigned agility = 0;
    std::uint16_t weaponTraining = 0;

    bool ApplyAbility(const std::string& bytes, unsigned profession)
    {
        if (bytes.size() != 63 || profession > 2) return false;
        const auto byte = [&](int offset) { return static_cast<unsigned char>(bytes[offset]); };
        const auto word = [&](int offset) { return unsigned(byte(offset)) | (unsigned(byte(offset + 1)) << 8); };
        const auto dword = [&](int offset) { return std::uint32_t(word(offset)) | (std::uint32_t(word(offset + 2)) << 16); };

        received = true;
        level = byte(0);
        combat = {{{byte(5), byte(6)}, {byte(7), byte(8)}, {byte(9), byte(10)},
            {byte(1), byte(2)}, {byte(3), byte(4)}}};
        hp = {word(11), word(15)};
        mp = {word(13), word(17)};
        experience = dword(19);
        nextExperience = dword(23);
        weight = {word(27), word(29)};
        wear = {byte(31), byte(32)};
        hand = {byte(33), byte(34)};
        for (int column = 0; column < 7; ++column) {
            attackElements[column] = static_cast<std::uint16_t>(word(35 + 2 * column));
            elements[0][column] = byte(36 + 2 * column);
            elements[1][column] = byte(49 + column);
            elements[2][column] = byte(56 + column);
        }
        return true;
    }

    double ExperiencePercent() const
    {
        return nextExperience ? 100.0 * experience / nextExperience : 0.0;
    }

    unsigned DisplayedElement(int row, int column) const
    {
        const auto raw = elements[row][column];
        return row == 0 || raw < 128 ? raw : 0;
    }
};

} // namespace mir3_cui

namespace mir3_cui {

// Read frame headers only. The CUI grid does not depend on decoded sprite pixels.
class InventoryDimensions {
public:
    void SetResourceRoot(const std::filesystem::path& root)
    {
        const auto directory = root / "Data";
        if (directory==data_) return;
        data_ = directory;
        archive_ = {};
        index_ = {};
        archiveSize_ = indexSize_ = 0;
        tableOffset_ = headerSize_ = 0;
        attempted_ = false;
        entries_.clear();
    }

    std::array<int, 2> Get(unsigned icon)
    {
        return Find(icon).value_or(std::array{1,1});
    }

    bool HasFrame(unsigned icon) { return Find(icon).has_value(); }

private:
    std::optional<std::array<int,2>> Find(unsigned icon)
    {
        if (!attempted_) Open();
        if (!headerSize_) return {};
        const auto cached = entries_.find(icon);
        if (cached != entries_.end()) return cached->second;
        return entries_.emplace(icon, Read(icon)).first->second;
    }

    std::filesystem::path data_ = "Data";
    std::ifstream archive_, index_;
    std::uint64_t archiveSize_ = 0, indexSize_ = 0;
    unsigned tableOffset_ = 0, headerSize_ = 0;
    bool attempted_ = false;
    std::map<unsigned, std::optional<std::array<int, 2>>> entries_;

    static unsigned Word(const unsigned char* bytes)
    {
        return bytes[0] | (unsigned(bytes[1]) << 8);
    }

    void Open()
    {
        if (attempted_) return;
        attempted_ = true;
        archive_.open(data_ / "Inventory.wil", std::ios::binary | std::ios::ate);
        index_.open(data_ / "Inventory.wix", std::ios::binary | std::ios::ate);
        if (!archive_ || !index_ || archive_.tellg() < 28 || index_.tellg() < 24) return;
        archiveSize_ = static_cast<std::uint64_t>(archive_.tellg());
        indexSize_ = static_cast<std::uint64_t>(index_.tellg());
        unsigned char version[2]{};
        archive_.seekg(22);
        archive_.read(reinterpret_cast<char*>(version), sizeof version);
        if (!archive_) return;
        switch (Word(version)) {
        case 17: tableOffset_ = 24; headerSize_ = 17; break;
        case 5000: tableOffset_ = 28; headerSize_ = 21; break;
        case 6000: tableOffset_ = 32; headerSize_ = 21; break;
        }
    }

    std::optional<std::array<int,2>> Read(unsigned icon)
    {
        const auto entry = std::uint64_t(tableOffset_) + std::uint64_t(icon) * 4;
        if (entry + 4 > indexSize_) return {};
        unsigned char bytes[4]{};
        index_.clear();
        index_.seekg(entry);
        if (!index_.read(reinterpret_cast<char*>(bytes), sizeof bytes)) return {};
        const auto offset = Word(bytes) | (std::uint64_t(Word(bytes + 2)) << 16);
        if (!offset || offset + headerSize_ > archiveSize_) return {};
        archive_.clear();
        archive_.seekg(offset);
        if (!archive_.read(reinterpret_cast<char*>(bytes), sizeof bytes)) return {};
        const auto width = Word(bytes), height = Word(bytes + 2);
        if (!width || !height || width > 6 * 38 || height > 100 * 38) return {};
        return std::array{int((width + 37) / 38), int((height + 37) / 38)};
    }
};

} // namespace mir3_cui

namespace mir3_cui {

enum class GroundCell : unsigned char { Outside, Walkable, Blocked, DoorOpen, DoorClosed };

class MapTerrain {
public:
    static constexpr int ViewColumns = 41;
    static constexpr int ViewRows = 9;
    using View = std::array<GroundCell, ViewColumns * ViewRows>;
    static constexpr int LightColumns = 58, LightRows = 59;
    using LightView = std::array<unsigned, LightColumns * LightRows>;

    struct Graphic {
        int package = 255, frame = 65535, animation = 255;
    };
    struct GraphicsView {
        static constexpr int FirstColumn = -5, LastColumn = 21;
        static constexpr int FirstRow = -4, LastRow = 31;
        std::array<Graphic, 100> tiles{};
        using Objects = std::array<Graphic, 2>;
        std::array<Objects, (LastColumn-FirstColumn)*(LastRow-FirstRow)> objects{};
    };

    bool Open(const std::string& id, const std::filesystem::path& directory = "Map")
    {
        ++revision_;
        file_ = std::ifstream(directory / (id + ".map"), std::ios::binary | std::ios::ate);
        width_ = height_ = 0;
        doors_.clear();
        if (!file_ || file_.tellg() < HeaderSize) return false;
        const auto length = file_.tellg();
        std::array<unsigned char, HeaderSize> header{};
        file_.seekg(0);
        if (!file_.read(reinterpret_cast<char*>(header.data()), header.size())) return false;
        const int width = header[22] | (header[23] << 8);
        const int height = header[24] | (header[25] << 8);
        if (width < 2 || height < 2 || width > 8192 || height > 8192 || width % 2 || height % 2) return false;
        const auto cells = std::int64_t(width) * height;
        const auto start = HeaderSize + (cells / 4) * 3;
        if (length != start + cells * CellSize) return false;
        width_ = width;
        height_ = height;
        start_ = start;
        return true;
    }

    bool OpenAtSpawn(const std::string& id, const std::filesystem::path& directory, int x, int y, View& view)
    {
        MapTerrain next;
        if (!next.Open(id,directory) || !next.Contains(x,y)) return false;
        auto ground=next.ReadView(x,y);
        const auto center=ground[ViewRows/2*ViewColumns+ViewColumns/2];
        if (center!=GroundCell::Walkable && center!=GroundCell::DoorOpen) return false;
        next.revision_=revision_+1;
        *this=std::move(next);
        view=ground;
        return true;
    }

    bool Ready() const { return width_ != 0; }
    std::uint64_t Revision() const { return revision_; }
    bool Contains(int x, int y) const { return x >= 0 && y >= 0 && x < width_ && y < height_; }

    bool BaseWalkable(int x,int y)
    {
        std::array<unsigned char,CellSize> cell{};
        return Contains(x,y) && ReadColumn(x,y,1,cell.data()) && (cell[0]&1);
    }

    unsigned DoorAt(int x, int y)
    {
        if (!Contains(x, y)) return 0;
        std::array<unsigned char, CellSize> cell{};
        return ReadColumn(x, y, 1, cell.data()) ? DoorId(cell.data()) : 0;
    }

    View ReadView(int x, int y)
    {
        View view{};
        const int left = x - ViewColumns / 2;
        const int top = y - ViewRows / 2;
        const int first = (std::max)(top, 0);
        const int last = (std::min)(top + ViewRows, height_);
        if (last <= first) return view;

        std::array<unsigned char, ViewRows * CellSize> column{};
        for (int cx = (std::max)(left, 0); cx < (std::min)(left + ViewColumns, width_); ++cx) {
            if (!ReadColumn(cx, first, last - first, column.data())) continue;
            for (int cy = first; cy < last; ++cy) {
                const auto* row = column.data() + (cy - first) * CellSize;
                auto& cell = view[(cy - top) * ViewColumns + cx - left];
                if (DoorId(row)) {
                    const auto override = doors_.find(Index(cx, cy));
                    const bool open = override != doors_.end() ? override->second : (row[11] & 128) != 0;
                    cell = open && (row[0] & 1) ? GroundCell::DoorOpen : GroundCell::DoorClosed;
                } else {
                    cell = (row[0] & 1) ? GroundCell::Walkable : GroundCell::Blocked;
                }
            }
        }
        return view;
    }

    LightView ReadLights(int left, int top)
    {
        LightView view{};
        const int first=(std::max)(0,top), last=(std::min)(height_,top+LightRows);
        if (last<=first) return view;
        std::array<unsigned char,LightRows*CellSize> cells{};
        for (int x=(std::max)(0,left);x<(std::min)(width_,left+LightColumns);++x) {
            if (!ReadColumn(x,first,last-first,cells.data())) continue;
            for (int y=first;y<last;++y) {
                const unsigned packed=Word(cells.data()+(y-first)*CellSize+12);
                if (((packed>>4)&255)<10) view[(x-left)*LightRows+y-top]=packed;
            }
        }
        return view;
    }

    bool UpdateDoor(int x, int y, bool open)
    {
        if (!Contains(x, y)) return false;
        std::array<unsigned char, CellSize> cell{};
        if (!ReadColumn(x, y, 1, cell.data())) return false;
        const auto id = DoorId(cell.data());
        if (!id) return false;
        ++revision_;
        const int first = (std::max)(0, y - 8);
        const int last = (std::min)(height_, y + 10);
        std::array<unsigned char, 18 * CellSize> column{};
        for (int cx = (std::max)(0, x - 8); cx < (std::min)(width_, x + 10); ++cx) {
            if (!ReadColumn(cx, first, last - first, column.data())) continue;
            for (int cy = first; cy < last; ++cy) {
                const auto* row = column.data() + (cy - first) * CellSize;
                if ((row[9] & 127) == id) doors_[Index(cx, cy)] = open;
            }
        }
        return true;
    }

    GraphicsView ReadGraphics(int x, int y)
    {
        GraphicsView view;
        if (!Ready()) return view;
        constexpr int tilePitch=3;
        const int tileY=y/2;
        const int firstTile=(std::max)(0,tileY), lastTile=(std::min)(height_/2,tileY+10);
        std::array<unsigned char,10*tilePitch> tiles{};
        for (int column=0;column<10 && firstTile<lastTile;++column) {
            const int tileX=x/2+column;
            if (tileX<0 || tileX>=width_/2) continue;
            file_.clear();
            file_.seekg(HeaderSize+(std::int64_t(tileX)*(height_/2)+firstTile)*tilePitch);
            if (!file_.read(reinterpret_cast<char*>(tiles.data()),(lastTile-firstTile)*tilePitch)) continue;
            for (int row=firstTile;row<lastTile;++row) {
                const auto* tile=tiles.data()+(row-firstTile)*tilePitch;
                view.tiles[(row-tileY)*10+column]={tile[0],Word(tile+1),255};
            }
        }
        constexpr int columns=GraphicsView::LastColumn-GraphicsView::FirstColumn;
        const int top=y+GraphicsView::FirstRow;
        const int first=(std::max)(0,top), last=(std::min)(height_,y+GraphicsView::LastRow);
        std::array<unsigned char,(GraphicsView::LastRow-GraphicsView::FirstRow)*CellSize> cells{};
        for (int column=0;column<columns && first<last;++column) {
            const int cx=x+GraphicsView::FirstColumn+column;
            if (cx<0 || cx>=width_ || !ReadColumn(cx,first,last-first,cells.data())) continue;
            for (int cy=first;cy<last;++cy) {
                const auto* row=cells.data()+(cy-first)*CellSize;
                auto& objects=view.objects[(cy-top)*columns+column];
                const auto door=doors_.find(Index(cx,cy));
                const bool open=door!=doors_.end() ? door->second : (row[11]&128)!=0;
                for (int layer=0;layer<2;++layer) {
                    int id=Word(row+5+2*layer);
                    if (id!=65535 && (row[9]&127) && open) id+=row[11]&127;
                    objects[layer]={row[4-layer],id,row[1+layer]};
                }
            }
        }
        return view;
    }

private:
    static constexpr int HeaderSize = 28;
    static constexpr int CellSize = 14;
    std::ifstream file_;
    int width_ = 0, height_ = 0;
    std::int64_t start_ = 0;
    std::uint64_t revision_ = 0;
    std::map<int, bool> doors_;

    int Index(int x, int y) const { return x * height_ + y; }
    static unsigned DoorId(const unsigned char* cell) { return cell[9] & 128 ? cell[9] & 127 : 0; }
    static int Word(const unsigned char* bytes) { return bytes[0] | (bytes[1]<<8); }

    bool ReadColumn(int x, int y, int count, unsigned char* data)
    {
        file_.clear();
        file_.seekg(start_ + std::int64_t(Index(x, y)) * CellSize);
        return bool(file_.read(reinterpret_cast<char*>(data), count * CellSize));
    }
};

} // namespace mir3_cui

namespace mir3_cui {

constexpr unsigned ClientOptions = 1401;

struct ServerOption {
    const char* label;
    std::uint32_t mask;
    const char* command;
};

// These are protocol identifiers carried as GBK text in CM_SAY.
constexpr ServerOption ServerOptionList[] = {
    {"Block resurrection", 0x40000000u, "@\xbe\xdc\xbe\xf8\xbb\xd8\xc9\xfa\xca\xf5"},
    {"Block group teleport", 0x80000000u, "@\xbe\xdc\xbe\xf8\xcc\xec\xb5\xd8\xba\xcf\xd2\xbb"},
    {"Block trades", 0x08000000u, "@\xbe\xdc\xbe\xf8\xbd\xbb\xd2\xd7"},
    {"Block private messages", 0x04000000u, "@\xbe\xdc\xbe\xf8\xcb\xbd\xc1\xc4"},
    {"Block guild chat", 0x02000000u, "@\xbe\xdc\xbe\xf8\xd0\xd0\xbb\xe1\xc1\xc4\xcc\xec"},
    {"Allow joining a guild", 0x01000000u, "@\xbc\xd3\xc8\xeb\xd0\xd0\xbb\xe1"},
    {"Block all chat", 0x00800000u, "@\xbe\xdc\xbe\xf8\xcb\xf9\xd3\xd0\xc8\xcb\xc1\xc4\xcc\xec"}
};
constexpr std::size_t ServerOptionCount = sizeof ServerOptionList / sizeof *ServerOptionList;
constexpr char AttackModeCommand[] = "@\xb9\xa5\xbb\xf7\xb7\xbd\xca\xbd";

struct ServerOptions {
    std::uint32_t confirmed = 0;
    std::uint32_t displayed = 0;
    std::uint16_t attackMode = 0;
    bool attackModePending = false;
    bool open = false;
    std::string error;

    void Request(const ServerOption& option)
    {
        displayed = (displayed & ~option.mask) | (~confirmed & option.mask);
    }

    std::uint32_t Apply(std::uint32_t flags, std::uint16_t mode)
    {
        const auto changed = confirmed ^ flags;
        confirmed = flags;
        displayed = (displayed & ~changed) | (flags & changed);
        attackMode = mode;
        attackModePending = false;
        return changed;
    }
};

} // namespace mir3_cui

namespace mir3_cui {

struct AudioOptions {
    bool music = true;
    bool effects = true;
    bool reversePan = false;
    int musicLevel = 0;
    int effectsLevel = 0;

    static constexpr int KnobFromLevel(int level) { return 9 * (level + 100) / 5; }
    static constexpr int LevelFromKnob(int offset) { return (5 * offset - 900) / 9; }
    static constexpr int KnobOffset(int x) { return x < 288 ? 0 : x > 468 ? 185 : x - 288; }
    static constexpr int TrackStep(int offset, int x)
    {
        if (x <= 266 || x >= 487) return offset;
        if (x > offset + 292 || x > 477) return offset + 9 > 180 ? 185 : offset + 9;
        if (x < offset + 276 || x < 275) return offset < 9 ? 0 : offset - 9;
        return offset;
    }

    void Load(const wchar_t* path);
    bool Save(const wchar_t* path) const;
};

class AudioPlayer {
public:
    AudioPlayer() = default;
    AudioPlayer(const AudioPlayer&) = delete;
    AudioPlayer& operator=(const AudioPlayer&) = delete;
    ~AudioPlayer();

    void SetMusic(const wchar_t* path, const AudioOptions& options);
    void PauseMusic(bool pause);
    void PlayEffect(const wchar_t* path, const AudioOptions& options);
    bool PlaySceneEffect(std::int64_t actor, const wchar_t* path, int volume, int pan, const AudioOptions& options);
    bool SceneEffectPlaying(std::int64_t actor) const;
    void MoveSceneEffect(std::int64_t actor,int volume,int pan,const AudioOptions& options);
    void StopSceneEffect(std::int64_t actor);
    void ClearSceneEffects(bool keepLocal = false);
    void PlayMovieAudio(const std::vector<unsigned char>& samples, int channels, int rate, int bits,
                        const AudioOptions& options);
    void StopMovieAudio();
    void Apply(const AudioOptions& options);
    const std::string& Error() const { return error_; }

private:
    struct Track {
        IXAudio2SourceVoice* voice = nullptr;
        std::vector<unsigned char> samples;
        WAVEFORMATEX format = {};
        void Clear();
    };

    IXAudio2* engine_ = nullptr;
    IXAudio2MasteringVoice* output_ = nullptr;
    Track music_;
    Track effect_;
    Track movie_;
    struct SceneTrack {
        Track audio;
        int volume = 0, pan = 0;
    };
    std::map<std::int64_t, SceneTrack> scene_;
    bool musicPaused_ = false;
    std::wstring musicPath_;
    std::string error_;

    bool Open();
    bool LoadWave(const wchar_t* path, Track& track);
    bool Start(Track& track, bool loop, const AudioOptions& options);
    void ApplySceneTrack(const SceneTrack& sound,const AudioOptions& options);
};

} // namespace mir3_cui

namespace mir3 {
// Original client 800x600 space. 0x435820: HUD this+0xC5C/+0xC60,
// y=601-GameInter[1160].height=467 for the supplied packed resources.
// Button object stride 180; RECT at +4, draw x/y at +40/+44 (0x41FA00).
struct HudLayout {
    using Point = std::array<int, 2>;
    // 0x435820: RECT at this+0xC5C; supplied GameInter[1160] is 134px high.
    static constexpr Point origin{0, 601 - 134};
    // 0x438000 local custom portrait and remote human custom portrait blits.
    static constexpr std::array<int,4> custom_portrait_rect(bool remote) {
        return {origin[0]+534,origin[1]+(remote?13:20),remote?84:82,remote?103:94};
    }
    // 0x438652..0x4386DE; raw frame origin, first dye RGB, second dye white.
    static constexpr Point decoration_position{origin[0]-10,origin[1]-107};
    static constexpr int decoration_frame(std::uint8_t selector) { return 1689+selector; }
    // 0x438000 portrait; 0x4390E0 orbs, meters and stat labels.
    // Preserve raw image origins (these calls do not add WIL frame offsets).
    enum class Image { skin, portrait, hp, mp, experience, weight, left_stat, right_stat, target_badge, target_hp_border, target_hp_fill };
    static constexpr Point image_position(Image image) {
        constexpr std::array<Point, 11> offsets{{
            {0,0}, {534,20}, {33,8}, {73,8},
            {152,26}, {165,26}, {635,105}, {715,105}, {595,94}, {533,116}, {534,117}
        }};
        const auto index = static_cast<unsigned>(image);
        if (index >= offsets.size()) return {};
        return {origin[0] + offsets[index][0], origin[1] + offsets[index][1]};
    }
    bool expanded = false;
    bool repositioned = false;
    void toggle() { expanded = !expanded; repositioned = true; }
    using Rect = std::array<int, 4>; // x, y, width, height
    // 0x436A30 positions object +0x6960 at the HUD origin plus (535,100).
    static constexpr Point portrait_entry_position{origin[0]+535,origin[1]+100};
    static constexpr int portrait_entry_frame(bool pressed) { return pressed?2097:2096; }
    // 0x4359AF..0x4359CA sets the portrait hover RECT at this+0x7270.
    static constexpr Rect portrait_hover{535,486,81,94};
    // 0x43849D..0x4384BD: float origin/extent, drawn BEFORE GameInter[1160].
    static constexpr Rect chat_backing{185,317,433,213};
    // 0x438599/0x4385E1: offsets from HUD origin; sizes from packed frames.
    static constexpr Rect chat_frame(bool expanded) {
        return {origin[0]+178,origin[1]-(expanded?186:19),456,expanded?320:152};
    }
    // 0x435820 this+0xDDC/+0xDEC; 0x436DE0/0x436E90 TextOutA rows.
    static constexpr Rect chat_line(bool expanded) { return {189,expanded?320:492,expanded?429:341,15}; }
    static constexpr int chat_rows(bool expanded) { return expanded?14:5; }
    static constexpr std::array<std::array<int, 3>, 23> buttons{{
        {1170,637,473}, {1172,637,507}, {1174,637,542},
        {1176,7,539}, {1178,108,539}, {1180,706,475},
        {1182,674,478}, {1184,739,500}, {1186,742,478},
        {1188,706,539}, {1190,742,528}, {1192,674,528},
        {1194,682,500}, {1196,711,498}, {1168,619,472},
        {1205,150,469}, {1330,186,557}, {1333,226,557},
        {1335,265,556}, {1337,305,556}, {1338,346,556},
        {1340,386,556}, {1342,426,556}
    }};
    // 0x435820 explicit SetRect overrides for buttons 5..15.
    static constexpr std::array<Rect, 11> hit_overrides{{
        {708,476,34,19}, {674,475,29,25}, {740,502,28,29},
        {742,478,35,24}, {709,539,35,20}, {744,531,36,24},
        {675,532,34,22}, {682,504,30,27}, {712,498,28,37},
        {619,472,13,17}, {150,469,24,15}
    }};
    static int count(bool expanded) { return expanded ? 23 : 16; }
    static std::array<int, 3> button(int index, bool expanded) {
        if (index < 0 || index >= count(expanded)) return {};
        auto value = buttons[index];
        if (index == 14 && expanded) value[2] = 305; // 0x43EF10
        return value;
    }
    static Rect hit_rect(int index, bool expanded, int frame_width, int frame_height, bool repositioned = false) {
        if (index < 0 || index >= count(expanded)) return {};
        if (index >= 5 && index <= 15) {
            auto rect = hit_overrides[index - 5];
            if (index == 14 && expanded) rect[1] = 305;
            // 0x43EF10 calls 0x41FD20 after SetRect. The latter overwrites
            // the bounds with frame dimensions, including after collapsing.
            if (index == 14 && repositioned && frame_width > 0 && frame_height > 0) {
                rect[2] = frame_width; rect[3] = frame_height;
            }
            return rect;
        }
        const auto position = button(index, expanded);
        // 0x41FA00 gets initial bounds from the first frame's raw dimensions.
        // 0x435820 overrides all seven expanded-mode buttons to 32x32.
        return {position[1], position[2], index >= 16 ? 32 : frame_width,
                index >= 16 ? 32 : frame_height};
    }
    // 0x4390E0: draw RECTs; these are not button hit regions.
    static constexpr Rect coordinates{6,575,133,17};
    // 0x4390E0 -> 0x48BD63: COLORREF (BGR), not an RGB literal.
    // Element 0: coordinates, 1: level, 2/3: left/right stat.
    static constexpr unsigned text_color(int element) {
        return element == 0 ? 0xC8FFFFu : (element == 1 ? 0x00E1E1u : 0x32C8FFu);
    }
    static constexpr unsigned coordinate_shadow_color = 0x0A0A0A;
    static constexpr std::array<Point, 4> coordinate_shadows{{{-1,-1},{1,-1},{-1,1},{1,1}}};
    static Rect level(bool expanded) { return {393, expanded ? 298 : 466,16,16}; }
    // 0x43914A..0x43926B: selector 0x824750 compared in this exact order
    // against 0x8F1CD4, 0x8F1CD0, 0x8F1CD8, 0x8F1CDC. Their semantic
    // meaning is not yet recovered. No match uses LOWORD(experience).
    static constexpr int level_emblem_frame(std::uint32_t selector,
            const std::array<std::uint32_t,4>& references, std::uint32_t experience) {
        constexpr std::array<int,4> frames{1201,1200,1203,1202};
        for (unsigned i=0;i<references.size();++i)
            if (selector==references[i]) return frames[i];
        return static_cast<std::uint16_t>(experience);
    }
    static constexpr Point level_emblem_position(bool expanded) {
        return {origin[0]+(expanded?384:385),origin[1]-(expanded?178:11)};
    }
    // 0x435820: RECT fields +0xC6C/+0xC7C/+0xC8C/+0xC9C.
    // 0x4390E0 tests these via PtInRect (exclusive right/bottom).
    static constexpr int tooltip_at(Point p, bool shared_hp) {
        constexpr std::array<Rect,4> areas{{{33,475,36,88},{73,475,36,88},
            {148,490,12,96},{161,490,12,96}}};
        for (int i=0;i<4;++i) {
            const auto& r=areas[i];
            if (p[0]>=r[0] && p[0]<r[0]+r[2] && p[1]>=r[1] && p[1]<r[1]+r[3])
                return i<2 ? (shared_hp?2:i) : (i==2?4:3);
        }
        return -1;
    }
    // Kind: HP, MP, shared HP, weight, experience. Original integer SIZE/2
    // truncates before SetRect; odd glyph extents must not be rounded up.
    static constexpr Rect tooltip_rect(int kind, Point text_size) {
        constexpr std::array<Point,5> centers{{{55,528},{105,528},{70,528},{210,535},{152,544}}};
        if (kind<0 || kind>=5) return {};
        const auto c=centers[kind];
        const int x=text_size[0]/2,y=text_size[1]/2;
        return {c[0]-x-5,c[1]-y-1,2*x+10,2*y+2};
    }
    static Rect stat(int side) { return side == 0 ? Rect{663,570,50,24} : Rect{742,570,56,24}; }
};
// 0x41F840: button RECT right/bottom at +0x0C/+0x10, caption at +0x34.
// Global shared state: 0x4B5644=1, 0x4B5648=6, previous caption 0x903884.
struct HudButtonTooltip {
    std::string previous;
    bool expanding = true;
    int height = 6;
    HudLayout::Rect draw(std::string_view caption, HudLayout::Point text_size,
                         HudLayout::Point anchor) {
        if (caption.empty()) return {};
        const int width = text_size[0] + 12, full_height = text_size[1] + 4;
        int left = anchor[0];
        if (left + width > 800) left = 800 - width;
        // Caption changes do not reset an in-progress height counter.
        if (previous != caption) { previous = caption; expanding = true; }
        const int drawn_height = expanding ? std::min(++height, full_height) : full_height;
        if (expanding && height >= full_height) { expanding = false; height = 6; }
        return {left, anchor[1] - drawn_height, width, drawn_height};
    }
};

}

namespace mir3 {
struct Minimap {
    bool visible=false, pending=false, translucent=false, large=false, dragging=false, release_captured=false;
    HudLayout::Point position{672,0}, drag_offset{};
    int width=0, height=0, player_x=0, player_y=0;
    std::uint64_t reply_time=0;
    std::vector<HudLayout::Point> type_five_actors, nearby_green, nearby_orange;

    static std::int64_t map_pixel_x(int cell_x) { return std::int64_t(cell_x)*3/2; }
    static int actor_marker_x(int cell_x) {
        // The original subtracts one before converting to a screen pixel.
        return int((std::int64_t(cell_x)*3-2)/2);
    }

    int size() const { return large?256:128; }
    HudLayout::Rect bounds() const {
        return {position[0],position[1],std::min(size(),width),std::min(size(),height)};
    }
    HudLayout::Point button_position(int index) const {
        const auto [x,y,w,h]=bounds();
        return {x+w-38+19*index,y+h-19};
    }
    int button_hint(int index) const {
        return index==0?(translucent?1802:1800):(large?1803:1801);
    }
    HudLayout::Point source() const {
        const auto [x,y,w,h]=bounds();
        const auto map_x=map_pixel_x(player_x);
        return {int(std::clamp<std::int64_t>(map_x-w/2,0,width-w)),
            int(std::clamp<std::int64_t>(std::int64_t(player_y)-h/2,0,height-h))};
    }
    // 0x45E370 sets the staging surface size, 0x45CE80 centers its crop.
    HudLayout::Rect portrait_source(bool full_archive) const {
        const int w=full_archive?256:180, h=full_archive?256:130;
        return {int(std::clamp<std::int64_t>(map_pixel_x(player_x)-std::min(w,width)/2,0,std::max(0,width-w))),
            int(std::clamp<std::int64_t>(std::int64_t(player_y)-std::min(h,height)/2,0,std::max(0,height-h))),
            std::min(w,width),std::min(h,height)};
    }
    // 0x45DAD0 divides the cell and crop coordinates before applying the 1.5 X factor.
    static HudLayout::Point portrait_marker(HudLayout::Point cell,HudLayout::Point crop,
            HudLayout::Point origin,bool half_scale,bool player) {
        const int divisor=half_scale?2:1;
        const int inset=player?2:1;
        const int x_twice=3*(cell[0]/divisor)+2*(origin[0]-crop[0]/divisor-inset);
        return {x_twice/2,cell[1]/divisor+origin[1]-crop[1]/divisor-inset};
    }
    bool toggle(std::uint64_t now,std::uint64_t cooldown) {
        if (reply_time && now-reply_time<=cooldown) return false;
        if (visible) {
            visible=false;
            dragging=release_captured=false;
            return false;
        }
        pending=true;
        return true;
    }
    void receive(int w,int h,std::uint64_t now) {
        width=w;
        height=h;
        reply_time=now;
        if (pending) visible=true;
        pending=false;
        place_at_top_right();
    }
    void resize() {
        large=!large;
        place_at_top_right();
    }
    void place_at_top_right() {
        position={800-std::min(size(),width),0};
        dragging=false;
    }
};
}

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

namespace mir3 {
struct MountPanel : DraggablePanel {
    MountPanel():DraggablePanel({0,0,276,322}) {}
    static constexpr int background=1400;
    static constexpr std::array<std::array<int,4>,5> buttons{{
        {1221,236,279,1052}, {1402,19,232,4100}, {1405,77,232,4101},
        {1408,134,232,4102}, {1410,191,232,4103}
    }};
    int pressed=-1;
    std::uint8_t shape=0;

    void apply_reply(std::uint32_t recognition) { shape=static_cast<std::uint8_t>(recognition); }

    int command(int button) const {
        switch (button) {
        case 1: return shape?0:4106;
        case 2: return shape?4105:0;
        case 3: return 4104;
        case 4: return 4105;
        default: return 0;
        }
    }
};
}

namespace mir3 {
struct SkillPosition { int id,x,y; };
struct SkillLayout {
    // 0x450E70: window+2272, eight groups, 15 records each, stride12.
    static constexpr std::array<std::array<SkillPosition,15>,8> positions{{
        {{{1,30,76},{5,30,142},{9,150,142},{23,270,206},{22,150,272},{113,30,272},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}},
        {{{39,30,76},{40,30,142},{53,150,206},{33,150,272},{110,150,336},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}},
        {{{41,30,76},{11,30,142},{10,150,206},{24,150,272},{111,150,336},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}},
        {{{67,30,76},{8,270,142},{74,30,142},{73,150,206},{31,270,206},{72,150,272},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}},
        {{{2,30,76},{37,150,142},{29,30,206},{38,150,206},{77,30,272},{120,270,272},{122,270,336},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}},
        {{{6,30,76},{13,210,76},{16,30,142},{14,90,142},{18,150,76},{15,90,206},{19,150,142},{89,90,272},{94,90,336},{121,150,336},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}},
        {{{17,30,76},{20,150,76},{21,210,76},{30,30,142},{32,150,142},{104,210,142},{105,30,206},{123,30,272},{112,150,272},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}},
        {{{3,30,76},{27,210,272},{4,270,76},{7,30,142},{36,270,272},{12,30,206},{25,90,272},{26,150,336},{35,90,336},{34,150,402},{103,90,402},{107,210,336},{102,210,402},{106,210,532},{108,210,466}}}
    }};
    // 0x44F3E0: classification is independent of whether a tree slot exists.
    static constexpr int category(int id) {
        switch(id) {
        case 1: case 5: case 9: case 22: case 23: case 47: case 113: case 150: return 0;
        case 2: case 29: case 37: case 38: case 77: case 120: case 122: return 4;
        case 3: case 4: case 7: case 12: case 25: case 26: case 27: case 34: case 35: case 36: case 102: case 103: case 106: return 7;
        case 6: case 13: case 14: case 15: case 16: case 18: case 19: case 48: case 78: case 85: case 86: case 87: case 88: case 89: case 90: case 91: case 92: case 93: case 94: case 95: case 96: case 97: case 98: case 99: case 100: case 101: case 121: return 5;
        case 8: case 31: case 67: case 72: case 73: case 74: return 3;
        case 10: case 11: case 24: case 41: case 63: case 64: case 66: case 111: return 2;
        case 17: case 20: case 21: case 30: case 32: case 104: case 105: case 112: case 123: return 6;
        case 33: case 39: case 40: case 53: case 110: return 1;
        default:return 7;
        }
    }
    static int slot(int category,int id) {
        if (category<0 || category>=8 || id<=0) return -1;
        for (int i=0;i<15;++i) if (positions[category][i].id==id) return i;
        return -1;
    }
};
struct SkillDisplay {
    int id=0,level=0,key=0;
    std::uint32_t experience=0;
    int required=0;
    std::string name;
    std::optional<std::array<int,3>> thresholds;
    // 418D80 / 4197C0: record fields for casting delay and the mana threshold.
    std::uint32_t casting_delay=0;
    std::uint8_t mana_threshold=0;
    std::string progress_text() const {
        return std::to_string(static_cast<std::int32_t>(experience))+"/"+std::to_string(required);
    }
};

struct SkillPanel : DraggablePanel {
    SkillPanel() : DraggablePanel({348,0,360,488}) {}
    std::array<std::vector<SkillDisplay>,8> learned{};
    std::array<int,8> selected{-1,-1,-1,-1,-1,-1,-1,-1};
    std::array<int,13> bindings{}; // Original shortcut slots store the low byte of each skill ID.
    int tab=0,scroll=0,pressed=-1;
    int menu_target=-1; // -1 closes the menu, 0 leaves it open without a target.
    const SkillDisplay* find(int id) const {
        const auto& group = learned[SkillLayout::category(id)];
        const auto skill = std::find_if(group.begin(),group.end(),[id](const auto& entry) { return entry.id==id; });
        return skill==group.end() ? nullptr : &*skill;
    }
    bool remove(int id) {
        auto& group=learned[SkillLayout::category(id)];
        const auto skill=std::find_if(group.begin(),group.end(),[id](const auto& entry) { return entry.id==id; });
        const bool removed=skill!=group.end();
        if (removed) {
            group.erase(skill);
            if (menu_target==id) menu_target=-1;
        }
        select_nonempty();
        return removed;
    }
    void select_nonempty() {
        if (learned[tab].empty()) {
            for (int i=0;i<int(learned.size());++i) {
                if (!learned[i].empty()) { tab=i;break; }
            }
        }
    }
    bool update_progress(int id,std::uint32_t experience,int level) {
        auto& group=learned[SkillLayout::category(id)];
        const auto skill=std::find_if(group.begin(),group.end(),[id](const auto& entry) { return entry.id==id; });
        if (skill==group.end()) return false;
        skill->experience=experience;
        skill->level=level;
        if (skill->thresholds && level<3) skill->required=(*skill->thresholds)[level];
        return true;
    }
    void wheel(bool up) { scroll=std::clamp(scroll+(up?-30:30),0,190); }
    int thumb_y() const { return 100+static_cast<int>(238.0*static_cast<float>(double(scroll)/189.0)); }
    bool scroll_captured=false;
    std::uint32_t scroll_time=0;
    // 0x44E750/0x44EA70 and input0x450A40/0x450D10.
    bool scroll_input(int kind,int x,int y,int width,int height,std::uint32_t now) {
        if (kind==2) { scroll_time=now;const bool was=scroll_captured;scroll_captured=false;return was; }
        const int top=100,bottom=100+238+height;
        if (kind==1) {
            if (!scroll_captured) return false;
            const float ratio=std::clamp(float(y-top)/(bottom-top),0.f,1.f);
            scroll=int(190.0*ratio);return true;
        }
        if (kind!=0 || width<=0 || height<=0 || x<322 || x>=322+width || y<top || y>=bottom) return false;
        float ratio=float(double(scroll)/189.0);
        if (now-scroll_time>10) {
            const int thumb=thumb_y();
            if (y>=thumb && y<thumb+height) scroll_captured=true;
            else ratio=std::clamp(ratio+(y<thumb+7?-7.f/190:7.f/190),0.f,1.f);
            scroll_time=now;
        }
        // Click clamps to189; dragging and wheel can reach190 in the original.
        scroll=std::min(189,int(190.0*ratio));return true;
    }
    // 0x44EA70 draw-time positions supersede constructor coordinates.
    static std::array<int,3> button(int index) {
        if (index==0) return {1221,316,422};
        if (index>=1 && index<=8) return {1640+2*(index-1),46+34*(index-1),30};
        return {-1,0,0};
    }
    static constexpr std::array<int,2> tree_origin{18,63}; //0x44EA70
    static constexpr int tree_height=331;
    static constexpr int visible_skill_limit=15;
    bool release_button(int index) {
        const bool clicked=index==pressed && index>=0 && index<=8;
        pressed=-1;
        if (clicked && index>0) tab=index-1; //0x450860: no scroll reset
        return clicked;
    }
    int tree_frame() const { return 1622+2*tab; }
};
}

namespace mir3 {
struct TargetAttributes {
    int category=-1, state=-1, icon=-1, element=0;
    std::array<int,7> modifiers{};
    bool visible() const { return category!=-1 && state!=-1 && icon!=-1; }
    std::array<int,7> order() const {
        std::array<int,7> indices{0,1,2,3,4,5,6};
        for (int end=6;end>0;--end)
            for (int i=end;i>0;--i)
                if (modifiers[indices[i]]>modifiers[indices[i-1]])
                    std::swap(indices[i],indices[i-1]);
        for (int i=0;i<7;++i) {
            if (modifiers[indices[i]]) continue;
            for (int next=i+1;next<7;++next) {
                if (!modifiers[indices[next]]) continue;
                std::swap(indices[i],indices[next]);
                break;
            }
        }
        return indices;
    }
};
// 0x438000 actor fields: +69 type, +71/+72 shape, +400432 exclusion.
struct TargetPortrait {
    static bool special_shape(std::uint16_t shape) { return shape>=530 && shape<=533; }
    // 0x43F300: actor+68 profession, actor+69 human type (0 or 1).
    static int human_frame(std::uint8_t profession,std::uint8_t type) {
        if (type>1) return -1;
        return profession<=2?650+2*profession+(type!=0):0;
    }
    static int monster_frame(std::uint8_t type, std::uint16_t shape, bool excluded) {
        if (type != 3 || excluded || shape == 127 || shape == 153) return -1;
        return shape >= 2000 || special_shape(shape) ? shape : shape & 255;
    }
    static int fallback_frame(int frame, bool available) {
        return frame < 0 ? -1 : (available ? frame : 1999);
    }
    // 0x438310: +400660 == 1 selects 635; special 530..533 omit badge.
    static int badge(std::uint16_t shape, int state) {
        return special_shape(shape) ? -1 : (state == 1 ? 635 : 634);
    }
};
// actor+0x61C38 points to stats: percent +9, maximum u16 +10, current u16 +12.
// 0x4387FC..0x438A17, checked against assembly (decompiler loses RECT arguments).
struct TargetPortraitVitals {
    std::uint8_t percent = 0;
    std::uint16_t maximum = 0, current = 0;
    int bar_width() const {
        // x87 loads 0x4AF49C as float and truncates via 0x49FE1C.
        return static_cast<int>(double(percent) * double(0.01f) * 84.0);
    }
    std::string text(int part) const {
        if (part == 1) return "/";
        if (part != 0 && part != 2) return {};
        return current ? std::to_string(part == 0 ? current : maximum) : "?";
    }
    static std::array<int,4> text_rect(int part) {
        constexpr std::array<std::array<int,4>,3> rects{{
            {532,570,30,15}, {562,570,5,15}, {565,570,30,15}
        }};
        return part >= 0 && part < 3 ? rects[part] : std::array<int,4>{};
    }
};
}

namespace mir3 {
struct TradeScroll {
    static constexpr std::array<int,2> x{206,457};
    static constexpr int top=58, travel=158, last=33;
    std::array<int,2> first{};
    int captured=-1;

    int offset(int side) const { return first[side]*travel/last; }

    bool pointer(int kind,int px,int py,int width,int height) {
        if (kind==2) return std::exchange(captured,-1)>=0;
        if (kind==1 && captured>=0) {
            const auto ratio=std::clamp(float(py-top)/(travel+height),0.f,1.f);
            first[captured]=int(ratio*last);
            return true;
        }
        if (kind!=0 || py<top || py>=top+travel+height) return false;
        for (int side=0;side<2;++side) {
            if (px<x[side] || px>=x[side]+width) continue;
            const int thumb=top+offset(side);
            if (py>=thumb && py<thumb+height) captured=side;
            else first[side]=std::clamp(first[side]+(py<thumb?-6:6),0,last);
            return true;
        }
        return false;
    }
};
}

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
    bool captured() const { return dragging; }
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

namespace mir3 {
std::vector<std::string> wrap_system_notice(std::string_view utf8);

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

namespace mir3_cui {

enum class TradePrompt { None, Failed, Quantity, Gold };

struct TradePanel : mir3::DraggablePanel {
    static constexpr int Columns = 5, Rows = 40, Capacity = 12;
    using Items = std::vector<InventoryItem>;
    struct Feedback { int message, sample, itemMode, subMode; std::string item; };

    TradePanel() : DraggablePanel({0,0,494,330}) {}
    std::array<Items,2> items;
    std::array<std::uint32_t,2> gold{};
    mir3::TradeScroll scroll;
    std::string partner;
    std::vector<Feedback> feedback;
    bool repositionInventory = false;
    bool confirmed = false, confirmPressed = false, goldPending = false, returnPending = false;
    int offerSlot = -1, returnSlot = -1;
    unsigned offerCount = 0;
    InventoryItem offeredItem;
    TradePrompt prompt = TradePrompt::None;
    unsigned editor = 0, openRevision = 0;
    char input[32]{};

    void OpenPrompt(TradePrompt next)
    {
        prompt = next;
        ++editor;
        input[0] = '\0';
    }

    void Clear()
    {
        const auto bounds = rect;
        const auto revision = openRevision;
        const auto inputId = editor;
        auto pendingFeedback = std::move(feedback);
        *this = {};
        rect = bounds;
        openRevision = revision;
        editor = inputId;
        feedback = std::move(pendingFeedback);
    }

    static const InventoryItem* AtCell(const Items& side, int cell)
    {
        const auto item = std::find_if(side.begin(),side.end(),[cell](const auto& value) {
            return cell>=0 && cell<Columns*Rows && cell%Columns>=value.cell%Columns &&
                cell%Columns<value.cell%Columns+value.width && cell/Columns>=value.cell/Columns &&
                cell/Columns<value.cell/Columns+value.height;
        });
        return item == side.end() ? nullptr : &*item;
    }

    const InventoryItem* SelectedReturn() const { return AtCell(items[0],returnSlot); }

    static bool Fits(const Items& side, const InventoryItem& item, int slot)
    {
        if (slot<0 || slot>=Columns*Rows || item.width<1 || item.height<1 ||
            slot%Columns+item.width>Columns || slot/Columns+item.height>Rows) return false;
        const int x = slot%Columns, y = slot/Columns;
        return std::none_of(side.begin(),side.end(),[&](const auto& other) {
            const int ox = other.cell%Columns, oy = other.cell/Columns;
            return x<ox+other.width && x+item.width>ox && y<oy+other.height && y+item.height>oy;
        });
    }

    static bool Insert(Items& side, InventoryItem item, int preferred = -1)
    {
        if (side.size()>=Capacity) return false;
        if (!Fits(side,item,preferred)) {
            preferred = 0;
            while (preferred<Columns*Rows && !Fits(side,item,preferred)) ++preferred;
            if (preferred==Columns*Rows) return false;
        }
        item.cell = preferred;
        side.push_back(std::move(item));
        return true;
    }
};

} // namespace mir3_cui

namespace mir3_cui {

struct NpcDialogue {
    struct Row {
        std::string label;
        std::string command;
        unsigned color;
        int line = 0;
    };
    static constexpr unsigned DefaultColor = (std::numeric_limits<unsigned>::max)();
    std::uint32_t actor = 0;
    unsigned originX = 0, originY = 0;
    bool canClose = true;
    bool resetScroll = false;
    std::vector<Row> rows;
    int explicitBreaks = 0;
    Row prompt{};
    char answer[361]{};
    int editor = 0;
    bool answerTooLong = false;

    bool Apply(std::uint32_t id, const std::string& script)
    {
        const auto prefix = script.find('/');
        if (prefix == std::string::npos) return false;
        auto closeAllowed = id != actor || canClose;
        std::vector<Row> parsed;
        unsigned color = DefaultColor;
        int line = 0;
        std::size_t at = prefix + 1;
        while (at < script.size()) {
            const auto next = script.find_first_of("\\<{", at);
            if (next == std::string::npos) {
                parsed.push_back({script.substr(at), "", color, line});
                break;
            }
            if (next != at) parsed.push_back({script.substr(at, next - at), "", color, line});
            if (script[next] == '\\') {
                if (next == at && next > prefix + 1 && script[next - 1] == '\\') parsed.push_back({" ", "", color, line});
                ++line;
                at = next + 1;
                continue;
            }
            const bool link = script[next] == '<';
            const auto end = script.find(link ? '>' : '}', next + 1);
            if (end == std::string::npos) break;
            const auto tag = script.substr(next + 1, end - next - 1);
            const auto slash = tag.find('/');
            if (slash != std::string::npos) {
                const auto label = tag.substr(0, slash);
                const auto value = tag.substr(slash + 1);
                if (link && !label.empty()) parsed.push_back({label, value, color, line});
                else if (!link && label == "NOTCLOSE") closeAllowed = false;
                else if (!link && label == "FCOLOR" && !value.empty() && value.size() <= 2) {
                    unsigned index = 0;
                    bool valid = true;
                    for (const char digit : value) {
                        if (digit < '0' || digit > '9') { valid = false; break; }
                        index = index * 10 + digit - '0';
                    }
                    if (valid && index < 17) color = index;
                }
            }
            at = end + 1;
        }
        const auto nextEditor = editor + 1;
        *this = {};
        actor = id;
        canClose = closeAllowed;
        resetScroll = true;
        rows = std::move(parsed);
        explicitBreaks = line;
        editor = nextEditor;
        return true;
    }
};

} // namespace mir3_cui

namespace mir3_cui {

struct NpcShop {
    struct Good {
        std::string name, label;
        unsigned mode = 0, price = 0, stock = 0, icon = 0, requirementType = 0, requirement = 0;
    };
    struct Offer {
        std::string name, label;
        std::uint32_t id = 0;
        unsigned price = 0, icon = 0, count = 0, stackMode = 0;
    };
    struct Purchase {
        std::uint32_t actor = 0, offer = 0;
        unsigned revision = 0;
        std::uint64_t deadline = 0;
        bool crafting = false;
    };

    std::uint32_t actor = 0;
    unsigned revision = 0;
    unsigned catalogueRevision = 0;
    std::vector<Good> goods;
    std::vector<Offer> offers;
    int selected = -1, category = -1, offer = -1;
    unsigned page = 0;
    bool detailed = false, bulk = false, offersVisible = false, sourceLayout = false;
    bool storage = false, withdrawalPrompt = false, crafting = false;
    std::uint32_t withdrawing = 0;
    int notice = 0;
    std::uint64_t detailDeadline = 0;
    int lastRow = -1;
    std::uint64_t lastClick = 0;
    char quantity[12] = "1";
    std::string status;

    unsigned Quantity() const
    {
        const auto count = quantity[0] ? std::strtol(quantity, nullptr, 10) : 1;
        return count > 0 && count <= 65535 ? static_cast<unsigned>(count) : 0;
    }
};

} // namespace mir3_cui

namespace mir3_cui {

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

struct EquipmentPanel {
    std::array<InventoryItem, EquipmentSlots> slots;
    InventoryItem pending;
    ItemKey previous;
    ItemKey held;
    int heldSlot = -1;
    std::uint64_t lastRelease = 0;
    EquipmentRequest request = EquipmentRequest::None;
    int pendingSlot = -1;
    int selected = 0;
    std::uint64_t deadline = 0;
    bool open = false;
    std::string status;

    void ReleaseHeld()
    {
        held = {};
        heldSlot = -1;
        lastRelease = 0;
    }
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

    const char* ChannelPrefix() const { return channel == 1 ? "!!" : channel == 2 ? "!~" : ""; }

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

struct PartyPanel {
    bool open = false;
    bool allowed = false;
    PartyAction action = PartyAction::None;
    PartyNotice notice;
    std::vector<std::string> members;
    char name[361] = {};
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

struct QuestPanel {
    bool open = false;
    bool initialized = false;
    bool waiting = false;
    int parent = -1;
    int child = -1;
    int expanded = -1;
    bool resetScroll = false;
    bool resetDescription = false;
    std::vector<QuestEntry> entries;
    std::deque<int> notices;
    std::string error;

    QuestEntry* Find(int id)
    {
        for (auto& entry : entries) if (entry.record.id == id) return &entry;
        return nullptr;
    }

    const std::string* Description()
    {
        auto* entry = Find(parent);
        if (!entry) return nullptr;
        if (child < 0) return &entry->record.summary;
        const auto* record = entry->FindChild(child);
        return record ? &record->summary : nullptr;
    }

    void Sync(std::vector<QuestRecord>& records, bool full)
    {
        if (full) {
            entries.clear();
            initialized = !records.empty();
            waiting = false;
        }
        for (auto& record : records) {
            if (!full && Find(record.id)) break;
            if (entries.size() >= 2048) break;
            entries.push_back({std::move(record)});
        }
        std::stable_sort(entries.begin(), entries.end(), [](const QuestEntry& a, const QuestEntry& b) {
            return a.record.id < b.record.id;
        });
        parent = child = expanded = -1;
        resetScroll = resetDescription = true;
    }
};

} // namespace mir3_cui

namespace mir3_client {

struct IndependentWindows {
    std::array<cui_vec2,5> positions{{{516,0},{100,100},{60,20},{0,22},{0,0}}};
    std::array<int,5> stacking{{0,1,2,3,4}};
    std::array<bool,5> wasOpen{};
    std::array<cui_id,5> pressedButtons{};
    int dragging = -1;
    cui_vec2 dragOffset{};
    struct Profile {
        bool open = false;
        unsigned body = 0;
        std::string name, guild;
        std::array<mir3_cui::InventoryItem, 12> equipment;
    } profile;
    struct Orders {
        bool enabled = false, open = false;
        unsigned x = 0, y = 0, targetX = 0, targetY = 0, range = 0;
        std::uint32_t object = 0;
        mir3_cui::ItemKey ammunition;
        char quantity[8]{};
        std::uint64_t loadUntil = 0;
    } orders;
    struct Listing {
        std::uint32_t id = 0, price = 0;
        std::string seller, server, description;
        mir3_cui::InventoryItem item;
    };
    struct Market {
        bool open = false, registration = false;
        unsigned page = 0, pages = 0;
        std::uint32_t actor = 0, selected = 0;
        std::uint64_t waitUntil = 0;
        std::vector<Listing> listings;
        mir3_cui::ItemKey item;
        std::string status;
        char filter[96]{}, sellerFilter[96]{}, price[16]{}, description[361]{};
        unsigned confirmation = 0;
        bool byName = true;
    } market;
    struct Letter {
        std::uint32_t id = 0;
        std::string name, date, subject;
        bool read = false;
    };
    struct Mail {
        bool unreadNotice = false, open = false, composing = false, reading = false;
        unsigned folder = 0;
        std::uint32_t selected = 0, readingId = 0;
        std::array<std::vector<Letter>, 2> folders;
        std::array<int,2> firstVisible{};
        std::string sender, subject, text, status;
        mir3_cui::InventoryItem gift;
        std::uint32_t gold = 0;
        char recipient[97]{}, draft[1504]{};
    } mail;

    bool Open() const { return profile.open || orders.open || market.open || market.registration || mail.open; }
};
enum class CharacterDialog { None, Create, Delete };



}


// Backward-compatible inventory persistence.
namespace mir3_cui {

struct InventoryLayout {
    static constexpr int BagStride = 3120;
    static constexpr int ShortcutStride = 3112;
    static constexpr int CellOffset = static_cast<int>(InventoryPanel::Capacity) * BagStride;
    static constexpr int ShortcutOffset = CellOffset + InventoryPanel::Cells * 2;
    static constexpr int FileSize = ShortcutOffset + 6 * ShortcutStride;
    static constexpr unsigned EmptyCell = 0xffff;
    static constexpr unsigned Anchor = 1000;

    static std::string Encode(InventoryPanel& bag)
    {
        std::string data(FileSize, '\0');
        std::fill(data.begin() + CellOffset, data.begin() + ShortcutOffset, static_cast<char>(0xff));
        int index = 0;
        for (const auto& item : bag.items) {
            if (index == InventoryPanel::Capacity) break;
            if (!InventoryPanel::OnGrid(item, item.cell)) continue;
            const int at = index * BagStride;
            data[at] = 1;
            WriteWord(data, at + 4, item.width);
            WriteWord(data, at + 8, item.height);
            WriteItem(data, at + 16, item);
            for (int y = 0; y < item.height; ++y)
                for (int x = 0; x < item.width; ++x)
                    WriteWord(data, CellOffset + (item.cell + y * InventoryPanel::Columns + x) * 2, index);
            WriteWord(data, CellOffset + item.cell * 2, Anchor + index);
            ++index;
        }
        for (std::size_t slot = 0; slot < bag.shortcuts.size(); ++slot) {
            const auto* item = bag.ResolveShortcut(slot);
            if (!item) continue;
            const int at = ShortcutOffset + static_cast<int>(slot) * ShortcutStride;
            data[at] = 1;
            WriteItem(data, at + 8, *item);
        }
        return data;
    }

    static bool Restore(const std::string& data, std::vector<InventoryItem>& records,
        std::array<ItemKey, 6>* shortcuts)
    {
        if (data.size() != FileSize) return false;
        std::array<int, InventoryPanel::Capacity> anchors;
        anchors.fill(-1);
        std::array<unsigned, InventoryPanel::Capacity> footprints{};
        for (int cell = 0; cell < InventoryPanel::Cells; ++cell) {
            const auto code = Word(data, CellOffset + cell * 2);
            if (code == EmptyCell) continue;
            const auto index = code % Anchor;
            if (code >= 2 * Anchor || index >= footprints.size()) return false;
            ++footprints[index];
            if (code >= Anchor) anchors[index] = cell;
        }

        for (std::size_t index = 0; index < anchors.size(); ++index) {
            const int at = static_cast<int>(index) * BagStride;
            if (Dword(data, at) == 0 || anchors[index] < 0) continue;
            const auto name = Name(data, at + 16);
            const auto id = Dword(data, at + 78);
            const auto durability = Word(data, at + 82);
            const auto maximum = Word(data, at + 84);
            const auto found = std::find_if(records.begin(), records.end(), [&](const InventoryItem& item) {
                return item.cell < 0 && item.key.id == id && item.key.name == name &&
                    item.durability == durability && item.maxDurability == maximum;
            });
            const int anchor = anchors[index];
            if (found == records.end() || !InventoryPanel::OnGrid(*found, anchor) ||
                footprints[index] != static_cast<unsigned>(found->width * found->height)) continue;
            bool matches = true;
            for (int y = 0; y < found->height && matches; ++y) {
                for (int x = 0; x < found->width; ++x) {
                    const int position = anchor + y * InventoryPanel::Columns + x;
                    if (Word(data, CellOffset + position * 2) != index + (x == 0 && y == 0 ? Anchor : 0)) {
                        matches = false;
                        break;
                    }
                }
            }
            if (matches) found->cell = anchor;
        }

        if (shortcuts) {
            for (std::size_t slot = 0; slot < shortcuts->size(); ++slot) {
                const int at = ShortcutOffset + static_cast<int>(slot) * ShortcutStride;
                if (Dword(data, at) == 0) continue;
                const auto name = Name(data, at + 8);
                const auto found = std::find_if(records.rbegin(), records.rend(), [&](const InventoryItem& item) {
                    return item.key.name == name && item.QuickEligible();
                });
                if (found != records.rend() && std::none_of(shortcuts->begin(), shortcuts->end(),
                    [&](const ItemKey& bound) { return bound.name == name; }))
                    (*shortcuts)[slot] = found->key;
            }
        }
        return true;
    }

private:
    static unsigned Word(const std::string& data, int at)
    {
        return static_cast<unsigned char>(data[at]) | (unsigned(static_cast<unsigned char>(data[at + 1])) << 8);
    }

    static std::uint32_t Dword(const std::string& data, int at)
    {
        return Word(data, at) | (std::uint32_t(Word(data, at + 2)) << 16);
    }

    static std::string Name(const std::string& data, int at)
    {
        const auto first = data.begin() + at;
        return std::string(first, std::find(first, first + 30, '\0'));
    }

    static void WriteWord(std::string& data, int at, unsigned value)
    {
        data[at] = static_cast<char>(value);
        data[at + 1] = static_cast<char>(value >> 8);
    }

    static void WriteItem(std::string& data, int at, const InventoryItem& item)
    {
        std::copy(item.key.name.begin(), item.key.name.end(), data.begin() + at);
        data[at + 30] = static_cast<char>(item.mode);
        WriteWord(data, at + 36, item.icon);
        data[at + 61] = static_cast<char>(item.stackMode);
        WriteWord(data, at + 62, item.key.id);
        WriteWord(data, at + 64, item.key.id >> 16);
        WriteWord(data, at + 66, item.durability);
        WriteWord(data, at + 68, item.maxDurability);
        WriteWord(data, at + 71, item.count);
    }
};

} // namespace mir3_cui
