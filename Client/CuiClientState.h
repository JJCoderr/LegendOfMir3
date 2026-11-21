#pragma once

#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include "cui.h"
#include "CuiAudio.h"
#include "CuiCharacterStatus.h"
#include "CuiChat.h"
#include "CuiEquipment.h"
#include "CuiGuild.h"
#include "CuiInventory.h"
#include "CuiInventoryDimensions.h"
#include "CuiInventoryLayout.h"
#include "CuiMapTerrain.h"
#include "CuiLegacy/hud/minimap.hpp"
#include "CuiLegacy/hud/mount_panel.hpp"
#include "CuiLegacy/hud/skill_panel.hpp"
#include "CuiLegacy/hud/target_portrait.hpp"
#include "CuiNpcDialogue.h"
#include "CuiNpcShop.h"
#include "CuiParty.h"
#include "CuiQuests.h"
#include "CuiServerOptions.h"
#include "CuiTrade.h"
#include "Common/EnDecode.h"
#include "Common/Protocol.h"

#include <cerrno>
#include <charconv>
#include <cmath>
#include <array>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <map>
#include <random>
#include <string>
#include <vector>

namespace mir3_client {

enum class Stage {
    Groups, Login, Connecting, SigningIn, Servers, Selecting, Handoff,
    ConnectingCharacters, Characters, Complete, ConnectingGame, AuthenticatingGame,
    GameSession, SigningOutGame, GameDisconnected
};
enum class CharacterDialog { None, Create, Delete };
inline constexpr wchar_t LoginMusic[] = L"CuiAudio/Login.wav";
inline constexpr wchar_t SessionMusic[] = L"CuiAudio/Session.wav";
inline constexpr wchar_t ClickSound[] = L"CuiAudio/Click.wav";
inline constexpr WORD GameLogon = 50;
inline constexpr WORD GameNewMap = 51;
inline constexpr WORD GameMapDescription = 54;
inline constexpr WORD GameMinimap = 710;
inline constexpr WORD GameMinimapUnavailable = 711;
inline constexpr WORD ClientMinimap = 1033;
inline constexpr WORD GameAddSkill = 210, GameSkillList = 211, GameRemoveSkill = 212;
inline constexpr WORD GameSkillProgress = 640, ClientSkillBinding = 1008;
inline constexpr WORD GameRideHorse = 1300;
inline constexpr WORD GameTradeOpen = 673, GameTradeFailed = 674, GameTradeAddOk = 675, GameTradeAddFail = 676;
inline constexpr WORD GameTradeReturnOk = 677, GameTradeReturnFail = 678, GameTradeCancelled = 681;
inline constexpr WORD GameTradePeerAdd = 682, GameTradePeerRemove = 683, GameTradeGold = 684, GameTradePeerGold = 686;
inline constexpr WORD GameTradeComplete = 687;
inline constexpr WORD ClientTradeRequest = 1025, ClientTradeOffer = 1026, ClientTradeReturn = 1027;
inline constexpr WORD ClientTradeGold = 1029, ClientTradeConfirm = 1030;
inline constexpr WORD GameChangeMap = 634;
inline constexpr WORD GameBackstep = 9;
inline constexpr WORD GameTurn = 10;
inline constexpr WORD GameWalk = 11;
inline constexpr WORD GameRun = 13;
inline constexpr WORD GameMonsterDigUp = 20, GameMonsterRise = 27;
inline constexpr WORD GameActorExit = 29;
inline constexpr WORD GameActorDisappear = 30;
inline constexpr WORD GameActorDeath = 32;
inline constexpr WORD GameMonsterSkeleton = 33;
inline constexpr WORD GameActorStruck = 31;
inline constexpr WORD GameActorDead = 34;
inline constexpr WORD GameActorName = 42;
inline constexpr WORD GameTargetPortrait = 1403;
inline constexpr WORD GameTargetAttributes = 1304, ClientTargetAttributes = 1047;
inline constexpr WORD GameActorStruckWithSource = 1309;
inline constexpr WORD GameClearActors = 633;
inline constexpr WORD GameActorRemove = 803;
inline constexpr WORD GameActorRevive = 1316;
inline constexpr WORD GameActorReviveRefresh = 1321;
inline constexpr WORD GameNpcDialogue = 643;
inline constexpr WORD GameNpcClose = 644;
inline constexpr WORD GameShopGoods = 645;
inline constexpr WORD GameShopBought = 650;
inline constexpr WORD GameShopRejected = 651;
inline constexpr WORD GameShopOffers = 652;
inline constexpr WORD ClientClickNpc = 1010;
inline constexpr WORD ClientNpcSelect = 1011;
inline constexpr WORD ClientBuy = 1014;
inline constexpr WORD ClientShopOffers = 1015;
inline constexpr WORD GameOpenSell = 646;
inline constexpr WORD GameSellQuote = 647;
inline constexpr WORD GameSellOk = 648;
inline constexpr WORD GameSellFail = 649;
inline constexpr WORD GameOpenRepair = 668;
inline constexpr WORD GameRepairOk = 669;
inline constexpr WORD GameRepairFail = 670;
inline constexpr WORD GameRepairQuote = 671;
inline constexpr WORD GameSellStackOk = 1314;
inline constexpr WORD GameSellStackFail = 1315;
inline constexpr WORD ClientQuoteSell = 1012;
inline constexpr WORD ClientSell = 1013;
inline constexpr WORD ClientRepair = 1023;
inline constexpr WORD ClientQuoteRepair = 1024;
inline constexpr WORD GameStorageOpen = 700;
inline constexpr WORD GameStorageDepositOk = 701;
inline constexpr WORD GameStorageFull = 702;
inline constexpr WORD GameStorageDepositFail = 703;
inline constexpr WORD GameStorageList = 704;
inline constexpr WORD GameStorageWithdrawOk = 705;
inline constexpr WORD GameStorageWithdrawFail = 706;
inline constexpr WORD GameStorageBagFull = 707;
inline constexpr WORD ClientStorageDeposit = 1031;
inline constexpr WORD ClientStorageWithdraw = 1032;
inline constexpr WORD GameDoorOpen = 612;
inline constexpr WORD GameDoorClosed = 614;
inline constexpr WORD ClientOpenDoor = 1002;
inline constexpr WORD ClientWalk = 3011;
inline constexpr WORD ClientRun = 3013;
inline constexpr WORD GameLogout = 1009;
inline constexpr WORD GameDecodeKey = 1330;
inline constexpr WORD GameDecodeKeyResponse = 1150;
inline constexpr WORD GameExperience = 44;
inline constexpr WORD GameLevelUp = 45;
inline constexpr WORD GameAbility = 52;
inline constexpr WORD GameVitals = 53;
inline constexpr WORD GameWeight = 622;
inline constexpr WORD GameSubAbility = 752;

struct Gateway {
    std::string host;
    unsigned port = 0;
};

struct LocalScene {
    struct Movement {
        unsigned x = 0, y = 0, facing = 0;
        std::uint64_t deadline = 0;
    };

    struct Player {
        unsigned x = 0, y = 0, facing = 0;
        bool dead = false;
        unsigned npcType = 0;
        mir3_cui::CharacterAppearance appearance;
        std::string name;
        unsigned npcShape = 0;
        mir3::TargetPortraitVitals monsterVitals;
        mir3::TargetAttributes monsterAttributes;
        bool monsterAttributesRequested = false;
        std::uint32_t portraitVersion = 0;
        std::uint32_t resolvedPortraitVersion = 0;
        std::string portraitJpeg;
        std::uint64_t portraitQueryAt = 0;

        bool IsMonster() const { return npcType == 3; }
        bool IsNpc() const { return npcType == 4 || npcType == 5; }
    };

    Movement pending;
    std::uint64_t lastMove = 0;
    unsigned x = 0, y = 0, facing = 0;
    std::string mapId;
    std::string mapDescription;
    mir3_cui::MapTerrain terrain;
    mir3_cui::MapTerrain::View ground{};
    std::map<std::uint32_t, Player> players;

    bool Occupied(int cellX, int cellY) const
    {
        return std::any_of(players.begin(), players.end(), [&](const auto& entry) {
            const auto& player = entry.second;
            return !player.dead && static_cast<int>(player.x) == cellX && static_cast<int>(player.y) == cellY;
        });
    }

    mir3_cui::GroundCell GroundAt(int nextX, int nextY) const
    {
        constexpr int columns = mir3_cui::MapTerrain::ViewColumns;
        constexpr int rows = mir3_cui::MapTerrain::ViewRows;
        const int column = nextX - static_cast<int>(x) + columns / 2;
        const int row = nextY - static_cast<int>(y) + rows / 2;
        if (column < 0 || column >= columns || row < 0 || row >= rows) return mir3_cui::GroundCell::Outside;
        return ground[row * columns + column];
    }

    void SetCell(unsigned nextX, unsigned nextY)
    {
        if (x == nextX && y == nextY) return;
        x = nextX;
        y = nextY;
        ground = terrain.ReadView(x, y);
    }
};

inline constexpr std::array<std::array<int, 2>, 8> SceneDirections{{
    {0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}
}};
inline constexpr float SceneCellPitch = 24;

struct Character {
    std::string name;
    std::string label;
    unsigned profession = 0;
    unsigned level = 0;
    unsigned gender = 0;
    bool operator==(const Character&) const = default;
};

struct Server {
    std::string name;
    std::string label;
    WORD index = 0;
    unsigned load = 0;
};

struct BillingNotice {
    int message = 0;
    int argument = 0;
};

inline BillingNotice ReadBilling(const _TDEFAULTMESSAGE& header)
{
    if (header.wTag) return header.wTag == 1 ? BillingNotice{102} : BillingNotice{103, header.wTag - 1};
    if (header.wSeries) return header.wSeries == 1 ? BillingNotice{105} : BillingNotice{104, header.wSeries - 1};
    if (header.wParam) return header.wParam <= 100 ? BillingNotice{106, header.wParam} : BillingNotice{};
    if (header.nRecog > 0) return {107, header.nRecog};
    return {};
}

inline unsigned Number(const std::string& text, unsigned maximum, unsigned invalid = 0)
{
    if (text.empty()) return invalid;
    unsigned value = 0;
    for (char digit : text) {
        if (digit < '0' || digit > '9' || value > maximum / 10 ||
            (value == maximum / 10 && static_cast<unsigned>(digit - '0') > maximum % 10))
            return invalid;
        value = value * 10 + digit - '0';
    }
    return value;
}

inline std::string ConvertText(const std::string& text, UINT source, UINT destination)
{
    const int length = static_cast<int>(text.size());
    const int wideLength = MultiByteToWideChar(source, MB_ERR_INVALID_CHARS, text.data(), length, nullptr, 0);
    if (!wideLength) return {};

    std::wstring wide(wideLength, L'\0');
    MultiByteToWideChar(source, MB_ERR_INVALID_CHARS, text.data(), length, &wide[0], wideLength);
    BOOL substituted = FALSE;
    BOOL* usedDefault = destination == CP_UTF8 ? nullptr : &substituted;
    const DWORD flags = usedDefault ? WC_NO_BEST_FIT_CHARS : 0;
    const int size = WideCharToMultiByte(destination, flags, wide.data(), wideLength, nullptr, 0, nullptr, usedDefault);
    if (!size || substituted) return {};

    std::string result(size, '\0');
    WideCharToMultiByte(destination, flags, wide.data(), wideLength, &result[0], size, nullptr, usedDefault);
    return result;
}

inline Gateway ReadGateway(const std::string& body, bool requireTerminator)
{
    const auto slash = body.find('/');
    if (slash == std::string::npos || slash > 15) return {};
    const auto end = body.find('/', slash + 1);
    if (requireTerminator && end == std::string::npos) return {};
    const auto port = body.substr(slash + 1, end == std::string::npos ? end : end - slash - 1);
    Gateway gateway{body.substr(0, slash), Number(port, 65535)};
    in_addr address = {};
    if (!gateway.port || InetPtonA(AF_INET, gateway.host.c_str(), &address) != 1) return {};
    return gateway;
}

inline bool ReadCharacters(int count, const std::string& body, std::vector<Character>& result)
{
    if (count < 0 || body.size() > 255) return false;
    if (count > 4) count = 4;
    std::vector<Character> next;
    size_t start = 0;
    for (int role = 0; role < count; ++role) {
        if (start < body.size() && body[start] == '*') ++start;
        std::array<std::string, 5> fields;
        for (size_t i = 0; i < fields.size(); ++i) {
            const auto end = body.find('/', start);
            if (end == std::string::npos && (i != fields.size() - 1 || role + 1 < count)) return false;
            fields[i] = body.substr(start, end - start);
            start = end == std::string::npos ? body.size() : end + 1;
        }
        const auto profession = Number(fields[1], 2, 3);
        const auto level = Number(fields[3], 255, 256);
        const auto gender = Number(fields[4], 1, 2);
        const auto label = ConvertText(fields[0], 936, CP_UTF8);
        if (label.empty() || fields[0].size() > 24 || profession > 2 || level > 255 || gender > 1)
            return false;
        next.push_back({fields[0], label, profession, level, gender});
    }
    result = std::move(next);
    return true;
}

inline bool ValidCharacterName(const std::string& name)
{
    if (name.empty() || name.size() > 13) return false;
    for (size_t i = 0; i < name.size(); ++i) {
        const auto ch = static_cast<unsigned char>(name[i]);
        if (ch >= 0x80 && i + 1 < name.size()) {
            ++i;
        } else if (!(ch >= 'A' && ch <= 'Z') && !(ch >= 'a' && ch <= 'z') &&
                   !(ch >= '0' && ch <= '9')) {
            return false;
        }
    }
    return true;
}

inline std::string TrimIni(const std::string& value)
{
    const auto first = value.find_first_not_of(" \t\r");
    if (first == std::string::npos) return {};
    return value.substr(first, value.find_last_not_of(" \t\r") - first + 1);
}

inline std::string IniKey(std::string value)
{
    for (char& ch : value) {
        if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
    }
    return value;
}

inline std::vector<std::string> CommaFields(const std::string& value)
{
    std::vector<std::string> fields;
    size_t start = 0;
    for (auto end = value.find(','); end != std::string::npos; end = value.find(',', start)) {
        fields.push_back(value.substr(start, end - start));
        start = end + 1;
    }
    fields.push_back(value.substr(start));
    return fields;
}

struct GroupLogin {
    std::string host;
    unsigned port = 0;
    std::string registration;
    std::string recovery;
    std::vector<Server> servers;
};

struct GroupLine {
    std::string caption;
    std::string loginSection;
    unsigned row = 0;
};

struct ServerGroup {
    std::string label;
    std::string title;
    std::vector<GroupLine> lines;
    unsigned x = 0, y = 0, rowCount = 0;
};

class ServerGroups {
public:
    std::vector<ServerGroup> groups;
    int selected = -1;

    bool Load(std::istream& file)
    {
        ServerGroups next;
        std::string section, line;
        while (std::getline(file, line)) {
            line = TrimIni(line);
            if (line.empty() || line.front() == ';') continue;
            if (line.front() == '[' && line.back() == ']') {
                section = IniKey(TrimIni(line.substr(1, line.size() - 2)));
                continue;
            }
            const auto split = line.find('=');
            if (split == std::string::npos) continue;
            auto value = TrimIni(line.substr(split + 1));
            if (value.size() >= 2 && value.front() == value.back() &&
                (value.front() == '"' || value.front() == '\''))
                value = value.substr(1, value.size() - 2);
            next.ini_.emplace(section + '/' + IniKey(TrimIni(line.substr(0, split))), std::move(value));
        }

        const unsigned count = Number(next.Get("ServerGroup", "Count"), 40);
        if (!count) return false;
        for (unsigned i = 1; i <= count; ++i) {
            const auto parts = CommaFields(next.Get("ServerGroup", "Server" + std::to_string(i)));
            if (parts.size() != 4) return false;
            const auto x = Number(TrimIni(parts[2]), 800, 801);
            const auto y = Number(TrimIni(parts[3]), 600, 601);
            if (x > 800 || y > 600) return false;

            const auto groupSection = "ServerGroup" + std::to_string(i);
            const unsigned rows = Number(next.Get(groupSection, "Count"), 40, 41);
            if (rows > 40) return false;
            ServerGroup group{ConvertText(parts[0], 936, CP_UTF8), ConvertText(parts[1], 936, CP_UTF8), {}, x, y, rows};
            for (unsigned j = 1; j <= rows; ++j) {
                const auto row = CommaFields(next.Get(groupSection, "Server" + std::to_string(j)));
                if (row.size() != 3 || row[0].size() > 31 || row[1].size() > 31 || row[2].size() > 31)
                    return false;
                if (row[0] != "empty") group.lines.push_back({ConvertText(row[0], 936, CP_UTF8), row[1], j - 1});
            }
            if (x || y) next.groups.push_back(std::move(group));
        }
        if (next.groups.empty()) return false;
        *this = std::move(next);
        return true;
    }

    bool Login(const GroupLine& line, GroupLogin& output) const
    {
        if (line.loginSection.empty() || IniKey(line.loginSection) == "empty") return false;
        GroupLogin next;
        next.host = Get(line.loginSection, "ServerAddr");
        next.port = Number(Get(line.loginSection, "Param1"), 65535);
        in_addr address = {};
        if (!next.port || next.host.size() > 15 ||
            InetPtonA(AF_INET, next.host.c_str(), &address) != 1)
            return false;
        next.registration = ConvertText(Get(line.loginSection, "Param2"), 936, CP_UTF8);
        next.recovery = ConvertText(Get(line.loginSection, "Param3"), 936, CP_UTF8);
        if (next.registration.size() >= 512 || next.recovery.size() >= 512) return false;

        const auto list = Get(line.loginSection, "ServerList");
        for (size_t open = list.find('('); open != std::string::npos;) {
            const auto close = list.find(')', open + 1);
            const auto slash = list.find('/', open + 1);
            if (close == std::string::npos || slash >= close || slash + 1 == close ||
                next.servers.size() == 256)
                return false;
            const auto name = list.substr(slash + 1, close - slash - 1);
            auto caption = ConvertText(list.substr(open + 1, slash - open - 1), 936, CP_UTF8);
            if (caption.empty()) caption = ConvertText(name, 936, CP_UTF8);
            next.servers.push_back({name, caption});
            open = list.find('(', close + 1);
        }
        output = std::move(next);
        return true;
    }

private:
    std::map<std::string, std::string> ini_;

    std::string Get(const std::string& section, const std::string& key) const
    {
        const auto found = ini_.find(IniKey(section) + '/' + IniKey(key));
        return found == ini_.end() ? std::string{} : found->second;
    }
};

class LoginClient {
public:
    char host[16] = "127.0.0.1";
    char port[6] = "7001";
    char account[65] = {};
    char password[65] = {};
    char registration[512] = {};
    char recovery[512] = {};
    bool legacy = false;
    bool settingsOpen = false;
    unsigned noticeDismissals = 0;
    unsigned noticePasswordLength = 0;
    int loginNoticeId = 0;
    Stage stage = Stage::Login;
    ServerGroups serverGroups;
    std::vector<Server> configuredServers;
    std::vector<Server> servers;
    BillingNotice billing;
    int selectionSession = 0;
    Gateway selectionGateway;
    Gateway gameGateway;
    std::string gameCharacter;
    static constexpr std::uint64_t GameExitFadeMs = 2500;
    bool showSignOut = false;
    bool quitGameRequested = false;
    std::uint64_t returnToCharactersAt = 0;
    std::array<std::uint32_t,3> exitGuardTicks{};
    std::uint8_t exitGuardAction = 0;
    mir3_cui::ChatPanel chat;
    mir3_cui::ServerOptions serverOptions;
    mir3_cui::PartyPanel party;
    mir3_cui::GuildPanel guild;
    mir3_cui::QuestPanel quests;
    mir3_cui::InventoryPanel inventory;
    mir3_cui::MerchantService merchant;
    std::deque<mir3_cui::MerchantResult> merchantResults;
    unsigned merchantRevision = 0;
    mir3_cui::EquipmentPanel equipment;
    mir3_cui::CharacterStatus characterStatus;
    mir3_cui::CharacterAppearance characterAppearance;
    LocalScene scene;
    mir3::SkillPanel skills;
    mir3::MountPanel mount;
    mir3_cui::TradePanel trade;
    std::uint64_t mountInputUntil = 0;
    bool skillShortcutsVisible = false;
    mir3::Minimap minimap;
    WORD minimapReply = 0;
    bool minimapUnavailable = false, minimapShowActors = true;
    bool portraitMapMode = false, portraitMapPending = false, portraitMapReady = false;
    bool forceAttack = true, magicLight = true, damageNumbers = true, shadowBlend = false, userPhotos = true;
    struct LocalOption {
        bool LoginClient::*value;
        const wchar_t* key;
        bool initial;
        int enabledMessage, disabledMessage;
    };
    static constexpr std::array<LocalOption,6> LocalOptions{{
        {&LoginClient::minimapShowActors,L"ShowMon",true,2646,2647},
        {&LoginClient::forceAttack,L"AttackUser",true,1515,1516},
        {&LoginClient::magicLight,L"MagicLight",true,1518,1517},
        {&LoginClient::damageNumbers,L"BloodNum",true,2122,2121},
        {&LoginClient::shadowBlend,L"ShadowBlend",false,0,0},
        {&LoginClient::userPhotos,L"UserPic",true,2132,2131}
    }};
    std::uint64_t minimapDeadline = 0;
    mir3_cui::NpcDialogue npcDialogue;
    mir3_cui::NpcShop shop;
    mir3_cui::NpcShop::Purchase purchase;
    unsigned shopRevision = 0;
    std::uint64_t lastNpcClick = 0, lastNpcAction = 0;
    mir3_cui::AudioOptions audioOptions;
    mir3_cui::AudioPlayer audio;
    std::wstring audioOptionsPath;
    std::string audioSaveError;
    bool audioOpen = false;
    bool audioDirty = false;
    std::string chatHistoryDirectory = "CuiChatHistory";
    std::string itemShortcutDirectory = "CuiItemShortcuts";
    std::string inventoryLayoutDirectory = "CuiInventoryLayouts";
    std::filesystem::path portraitCacheDirectory;
    std::vector<Character> characters;
    int characterIndex = -1;
    int characterPage = 0;
    int profession = 0;
    int gender = 0;
    int characterEditor = 0;
    bool deleteCancelSelected = false;
    bool deleteTabDown = false;
    char characterInput[361] = {};
    CharacterDialog characterDialog = CharacterDialog::None;
    std::string characterNotice;
    int characterNoticeId = 0;
    unsigned characterListRevision = 0;
    WORD characterRequest = 0;
    std::string status;

    ~LoginClient() { Reset(); }

    std::filesystem::path PortraitCachePath() const
    {
        const auto valid = [](const std::string& value) {
            return !value.empty() && value.find_first_of("/\\:\"<>|?*")==std::string::npos &&
                std::none_of(value.begin(),value.end(),[](unsigned char ch) { return ch<32; });
        };
        const auto server = ConvertText(realm_,936,CP_UTF8);
        const auto role = ConvertText(gameRole_,936,CP_UTF8);
        if (!valid(server) || !valid(role)) return {};
        const auto directory = portraitCacheDirectory.empty() ? resourceRoot_/"Data" : portraitCacheDirectory;
        return directory/std::filesystem::u8path(server+"_"+role+".fac");
    }

    bool SendPortrait(const std::string& jpeg)
    {
        constexpr WORD GamePortraitUpload = 1151;
        if (stage!=Stage::GameSession || jpeg.size()>4095 || output_.size()>=8192) return false;
        Queue(GamePortraitUpload,0,jpeg);
        return true;
    }

    void RequestTargetPortrait(LocalScene::Player& player)
    {
        if (stage!=Stage::GameSession || !userPhotos || player.npcType || player.name.empty() ||
            player.portraitVersion==player.resolvedPortraitVersion || output_.size()>=8192) return;
        const auto now = GetTickCount64();
        if (now-player.portraitQueryAt<=1000) return;
        const auto name = ConvertText(player.name,CP_UTF8,936);
        if (name.empty()) return;
        Queue(1152,0,name);
        player.portraitQueryAt = now;
    }

    void Reset(const char* message = "")
    {
        if (stage == Stage::GameSession || stage == Stage::SigningOutGame) SaveInventoryLayout();
        Close();
        servers.clear();
        billing = {};
        selectionSession = 0;
        selectionGateway = {};
        ClearGameState();
        ClearCharacterSelection();
        signedInAccount_.clear();
        realm_.clear();
        stage = Stage::Login;
        status = message;
        noticePasswordLength = 0;
        loginNoticeId = 0;
        SecureZeroMemory(password, sizeof password);
    }

private:
    void ClearCharacterSelection()
    {
        characters.clear();
        characterIndex = -1;
        characterPage = 0;
        characterRequest = 0;
        characterNotice.clear();
        characterNoticeId = 0;
        CloseCharacterDialog();
    }

    void ClearGameState()
    {
        gameGateway = {};
        gameCharacter.clear();
        gameRole_.clear();
        showSignOut = false;
        quitGameRequested = false;
        returnToCharactersAt = 0;
        exitGuardTicks = {};
        exitGuardAction = 0;
        chat = {};
        serverOptions = {};
        party = {};
        guild = {};
        quests = {};
        skills = {};
        mount = {};
        trade = {};
        mountInputUntil = 0;
        skillShortcutsVisible = false;
        inventory = {};
        itemShortcutFilePresent_ = false;
        equipment = {};
        characterStatus = {};
        characterAppearance = {};
        scene = {};
        ClearMinimap();
        CloseNpcDialogue();
        merchant = {};
        merchantResults.clear();
        purchase = {};
        shopRevision = 0;
        lastNpcClick = lastNpcAction = 0;
        audioOpen = false;
    }

public:
    void DismissNotice()
    {
        status.clear();
        loginNoticeId = 0;
        noticePasswordLength = 0;
        SecureZeroMemory(password, sizeof password);
        ++noticeDismissals;
    }

    void ShowLoginNotice(int id, const char* fallback)
    {
        const auto length = password[0] ? static_cast<unsigned>(ConvertText(password, CP_UTF8, 936).size())
                                        : noticePasswordLength;
        Reset(fallback);
        loginNoticeId = id;
        noticePasswordLength = length;
    }

    bool OpenGroupLine(const GroupLine& line)
    {
        GroupLogin config;
        if (!serverGroups.Login(line, config)) {
            status = "This line has an invalid login configuration.";
            return false;
        }
        Reset();
        strcpy_s(host, config.host.c_str());
        sprintf_s(port, "%u", config.port);
        strcpy_s(registration, config.registration.c_str());
        strcpy_s(recovery, config.recovery.c_str());
        configuredServers = std::move(config.servers);
        settingsOpen = false;
        legacy = false;
        return true;
    }

    bool Active() const { return socket_ != INVALID_SOCKET; }

    void SignIn()
    {
        sockaddr_in address = {};
        address.sin_family = AF_INET;
        address.sin_port = htons(static_cast<u_short>(Number(port, 65535)));
        if (!address.sin_port || InetPtonA(AF_INET, host, &address.sin_addr) != 1) {
            status = "Enter an IPv4 address and a port from 1 to 65535.";
            return;
        }
        const auto user = ConvertText(account, CP_UTF8, 936);
        auto secret = ConvertText(password, CP_UTF8, 936);
        if (user.empty() || secret.empty() || user.find('/') != std::string::npos ||
            secret.find('/') != std::string::npos) {
            if (!secret.empty()) SecureZeroMemory(&secret[0], secret.size());
            status = "Enter an account and password supported by the server. Omit slash characters.";
            return;
        }
        if ((!legacy && user.size() > 27) || (legacy && user.size() + 1 + secret.size() >= 32)) {
            SecureZeroMemory(&secret[0], secret.size());
            status = legacy ? "The legacy gateway accepts up to 31 bytes for account, slash, and password."
                            : "The character gateway accepts account names up to 27 bytes.";
            return;
        }

        const int result = Connect(address);
        if (result == SOCKET_ERROR) {
            SecureZeroMemory(&secret[0], secret.size());
            Reset("Could not connect to the login gateway.");
            return;
        }

        signedInAccount_ = user;
        std::string credentials = user + '/';
        credentials += secret;
        Queue(CM_IDPASSWORD, 0, credentials);
        noticePasswordLength = static_cast<unsigned>(secret.size());
        SecureZeroMemory(&credentials[0], credentials.size());
        SecureZeroMemory(&secret[0], secret.size());
        SecureZeroMemory(password, sizeof password);
        Wait(result == 0 ? Stage::SigningIn : Stage::Connecting, "Contacting the login gateway...");
    }

    void Select(const Server& server)
    {
        if (stage != Stage::Servers || billing.message) return;
        realm_ = server.name;
        Queue(CM_SELECTSERVER, server.index, legacy ? "" : server.name);
        Wait(Stage::Selecting, "Waiting for the selection endpoint...");
    }

    std::uint64_t HandoffRemainingMs() const
    {
        const auto now = GetTickCount64();
        return deadline_ > now ? (std::min)(deadline_ - now, std::uint64_t{2000}) : 0;
    }

    void OpenCharacterDialog(CharacterDialog dialog)
    {
        if (dialog == CharacterDialog::Delete && characterIndex < 0) {
            ShowCharacterNotice(227, "Select a character to delete.");
            return;
        }
        characterDialog = dialog;
        profession = 0;
        gender = 0;
        ++characterEditor;
        deleteCancelSelected = false;
        deleteTabDown = false;
        SecureZeroMemory(characterInput, sizeof characterInput);
    }

    void CloseCharacterDialog()
    {
        characterDialog = CharacterDialog::None;
        SecureZeroMemory(characterInput, sizeof characterInput);
    }

    void SubmitCharacterDialog()
    {
        if (stage != Stage::Characters || characterRequest || !characterNotice.empty()) return;
        auto text = ConvertText(characterInput, CP_UTF8, 936);
        if (characterDialog == CharacterDialog::Create) {
            if (!ValidCharacterName(text)) {
                ShowCharacterNotice(225, "Use letters, digits, or Chinese characters, up to 13 bytes.");
                return;
            }
            RequestCharacter(CM_NEWCHR, signedInAccount_ + '/' + text + "/1/" +
                std::to_string(profession) + '/' + std::to_string(gender));
        } else if (characterDialog == CharacterDialog::Delete && characterIndex >= 0) {
            if ((characterInput[0] && text.empty()) || text.size() > 90 || text.find('/') != std::string::npos) {
                ShowCharacterNotice(227, "Enter confirmation text up to 90 bytes, without slashes.");
                return;
            }
            RequestCharacter(CM_DELCHR, characters[characterIndex].name + '/' + text);
            QueryCharacters();
            CloseCharacterDialog();
            if (!text.empty()) SecureZeroMemory(&text[0], text.size());
        }
    }

    void StartCharacter()
    {
        if (stage != Stage::Characters || characterRequest || !characterNotice.empty()) return;
        if (characterIndex < 0) {
            ShowCharacterNotice(230, "Select a character to continue.");
            return;
        }
        RequestCharacter(CM_SELCHR, signedInAccount_ + '/' + characters[characterIndex].name);
    }

    void RefreshCharacterList()
    {
        if (stage == Stage::Characters && !characterRequest) QueryCharacters();
    }

    void ShowCharacterNotice(int id, const char* fallback)
    {
        characterNoticeId = id;
        characterNotice = fallback;
    }

    bool GameExitAllowed() const
    {
        if (exitGuardAction==19) return true;
        const auto now = GetTickCount();
        return std::none_of(exitGuardTicks.begin(),exitGuardTicks.end(),
            [now](auto tick) { return std::uint32_t(now-tick)<=8000u; });
    }

    void SignOutGame(bool returnToCharacters = false)
    {
        if (stage != Stage::GameSession || (returnToCharacters && !GameExitAllowed())) return;
        showSignOut = false;
        Queue(GameLogout, 0, "");
        Wait(Stage::SigningOutGame, "Signing out...");
        if (returnToCharacters) {
            SaveInventoryLayout();
            if (audioOpen) SaveAudioSettings();
            returnToCharactersAt = GetTickCount64()+GameExitFadeMs+1;
        }
    }

    void SendChat(bool addChannelPrefix = true)
    {
        if (stage != Stage::GameSession || showSignOut || chat.receptionOpen || party.open || party.notice.id ||
            guild.open || guild.requested || guild.notice || guild.action != mir3_cui::GuildAction::None ||
            quests.open || inventory.open || inventory.notice || equipment.open ||
            !chat.draft[0]) return;
        const std::string draft = chat.draft;
        const bool command = draft.front() == '/' || draft.front() == '@' ||
            draft.front() == '#' || draft.front() == '!';
        const std::string text = addChannelPrefix && !command ? chat.ChannelPrefix() + draft : draft;
        const auto wire = ConvertText(text, CP_UTF8, 936);
        if (wire.empty() || wire.size() > 90 || draft.find_first_of("\r\n") != std::string::npos) {
            chat.error = "The message must fit within 90 GBK bytes.";
            return;
        }
        if (draft.front() != '#' && output_.size() >= 8192) {
            chat.error = "The connection is busy. Try sending again.";
            return;
        }
        chat.error.clear();
        chat.draft[0] = '\0';
        if (wire == "@\xcc\xed\xbc\xd3\xb3\xc9\xd4\xb1") {
            BeginGuildAction(mir3_cui::GuildAction::Add);
            return;
        }
        if (draft.front() == '#') {
            if (chat.Remember(chat.phrases, draft.substr(1))) SaveChatHistory();
            return;
        }
        if (draft.front() == '/') {
            const auto end = draft.find(' ');
            if (draft.size() > 1 && end != 1 && chat.Remember(chat.contacts, draft.substr(0, end) + ' '))
                SaveChatHistory();
            chat.Append(103, draft, 16750230);
        }
        if (draft == "@chatsave") chat.logging = !chat.logging;
        Queue(mir3_cui::Say, 0, wire);
    }

    void ToggleChatReception(mir3_cui::ReceptionChannel channel)
    {
        if (SendOptionCommand(mir3_cui::ChatReception::ToggleCommand(channel), chat.reception.error))
            chat.reception.Request(channel);
    }

    void ToggleServerOption(const mir3_cui::ServerOption& option)
    {
        if (SendOptionCommand(option.command, serverOptions.error)) serverOptions.Request(option);
    }

    void RequestAttackMode()
    {
        if (SendOptionCommand(mir3_cui::AttackModeCommand, serverOptions.error))
            serverOptions.attackModePending = true;
    }

    void BindSkill(int id, int key)
    {
        if (stage != Stage::GameSession || showSignOut || key < 1 || key > 12) return;
        std::size_t messages = skills.find(id) ? 1 : 0;
        for (const auto& group : skills.learned)
            messages += std::count_if(group.begin(),group.end(),[key](const auto& skill) { return skill.key == key; });
        if (output_.size() + messages*19 > 8192) return;
        skills.bindings[key] = 0;
        for (auto& group : skills.learned) for (auto& skill : group) {
            if (skill.key != key) continue;
            skill.key = 0;
            Queue(ClientSkillBinding,0,"",skill.id);
        }
        for (auto& skill : skills.learned[mir3::SkillLayout::category(id)]) {
            if (skill.id != id) continue;
            if (skill.key && skills.bindings[skill.key] == (id&255)) skills.bindings[skill.key] = 0;
            skill.key = key;
            skills.bindings[key] = id&255;
            Queue(ClientSkillBinding,key <= 9 ? '0'+key : 'A'+key-10,"",id);
            break;
        }
    }

    void SendMountCommand(const std::string& text)
    {
        if (stage != Stage::GameSession || showSignOut || output_.size() >= 8192) return;
        const auto command = ConvertText(text, CP_UTF8, 936);
        if (command.empty()) return;
        Queue(mir3_cui::Say,0,command);
        mountInputUntil = GetTickCount64()+300;
    }

    void RequestTrade()
    {
        if (stage != Stage::GameSession || showSignOut || output_.size() >= 8192 ||
            scene.facing >= SceneDirections.size()) return;
        const auto [dx,dy] = SceneDirections[scene.facing];
        for (const auto& [id,actor] : scene.players) {
            if (actor.dead || actor.npcType || actor.name.empty() || static_cast<int>(actor.x)!=static_cast<int>(scene.x)+dx ||
                static_cast<int>(actor.y)!=static_cast<int>(scene.y)+dy) continue;
            const auto name = ConvertText(actor.name,CP_UTF8,936);
            if (!name.empty()) Queue(ClientTradeRequest,0,name);
            break;
        }
    }

    int ActivateTradeCell(int slot)
    {
        using namespace mir3_cui;
        if (!trade.visible || trade.confirmed || trade.offerSlot>=0 || trade.returnPending || trade.goldPending || inventory.heldGold) return 0;
        if (const auto* item = HeldItem()) {
            if (!item->key.id || equipment.heldSlot>=0 || inventory.heldShortcut>=0 ||
                inventory.request!=ItemRequest::None || equipment.request!=EquipmentRequest::None || output_.size()>=8192) return 0;
            if (trade.items[0].size()>=TradePanel::Capacity || !TradePanel::Fits(trade.items[0],*item,slot))
                return TradePanel::AtCell(trade.items[0],slot) ? 1306 : 0;
            trade.offerSlot = slot;
            trade.offeredItem = *item;
            if (item->Stacks() && item->count>1) trade.OpenPrompt(TradePrompt::Quantity);
            else SendTradeOffer((std::max)(1u,item->count));
        } else {
            const auto* selected = TradePanel::AtCell(trade.items[0],slot);
            trade.returnSlot = selected && selected->key.id ? selected->cell : -1;
        }
        return 0;
    }

    void BeginTradeGold()
    {
        if (trade.visible && !trade.confirmed && !TradeItemLocked() && !trade.goldPending && !trade.gold[0] &&
            inventory.gold && !inventory.heldGold && !HeldItem()) trade.OpenPrompt(mir3_cui::TradePrompt::Gold);
    }

    void FinishTradePrompt(bool accept)
    {
        const auto prompt = std::exchange(trade.prompt,mir3_cui::TradePrompt::None);
        unsigned amount = 0;
        const auto end = trade.input+std::strlen(trade.input);
        const auto result = std::from_chars(trade.input,end,amount);
        const bool valid = accept && trade.visible && !trade.confirmed && result.ec==std::errc{} && result.ptr==end &&
            amount && output_.size()<8192;
        if (prompt == mir3_cui::TradePrompt::Quantity) {
            const auto* held = HeldItem();
            if (valid && trade.offerSlot>=0 && held && held->key==trade.offeredItem.key && amount<=held->count)
                SendTradeOffer(amount);
            else trade.offerSlot = -1;
        } else if (prompt == mir3_cui::TradePrompt::Gold && valid && !trade.gold[0] && !trade.goldPending && amount<=inventory.gold) {
            trade.goldPending = true;
            Queue(ClientTradeGold,0,"",amount);
        }
    }

    void ConfirmTrade()
    {
        if (!trade.visible || trade.confirmed || TradeItemLocked() || trade.goldPending || output_.size()>=8192) return;
        trade.confirmed = true;
        Queue(ClientTradeConfirm,0,"");
    }

    bool TradeItemLocked() const { return trade.offerSlot>=0 || trade.returnSlot>=0; }

    void CancelTradeReturnSelection()
    {
        if (!trade.returnPending) trade.returnSlot = -1;
    }

    void SetPartyAllowed(bool enabled)
    {
        if (stage != Stage::GameSession || showSignOut) return;
        if (output_.size() >= 8192) {
            party.error = "The connection is busy. Try changing the party setting again.";
            return;
        }
        Queue(mir3_cui::PartyMode, enabled ? 1 : 0, "");
        party.allowed = enabled;
        party.error.clear();
    }

    void OpenQuests()
    {
        if (stage != Stage::GameSession) return;
        quests.open = true;
        if (!quests.initialized && quests.entries.empty()) RequestQuests();
    }

    void OpenInventory()
    {
        if (stage != Stage::GameSession) return;
        inventory.open = true;
        if (!inventory.initialized && !inventory.waiting) RequestInventory();
    }

    void SetResourceRoot(const std::filesystem::path& root)
    {
        resourceRoot_ = root;
        inventoryDimensions_.SetResourceRoot(root);
    }

    void LoadMapOptions(const wchar_t* path)
    {
        portraitMapMode = GetPrivateProfileIntW(L"Options",L"InformationMode",0,path) != 0;
        for (const auto& option : LocalOptions)
            this->*option.value = GetPrivateProfileIntW(L"Options",option.key,option.initial,path) != 0;
    }

    void ToggleMinimap(std::uint64_t cooldown)
    {
        if (stage != Stage::GameSession || output_.size() >= 8192) return;
        if (minimap.toggle(GetTickCount64(),cooldown)) RequestMinimap();
    }

    void TogglePortraitMap()
    {
        const auto now=GetTickCount64();
        if (stage != Stage::GameSession || (minimap.reply_time && now-minimap.reply_time<=1000)) return;
        if (!portraitMapMode && output_.size() >= 8192) return;
        portraitMapMode = !portraitMapMode;
        portraitMapPending = portraitMapMode;
        portraitMapReady = false;
        if (portraitMapPending) RequestMinimap();
        if (!audioOptionsPath.empty())
            WritePrivateProfileStringW(L"Options",L"InformationMode",portraitMapMode ? L"1" : L"0",
                audioOptionsPath.c_str());
    }

    void CloseNpcDialogue()
    {
        SecureZeroMemory(npcDialogue.answer, sizeof npcDialogue.answer);
        npcDialogue = {};
        shop = {};
        if (merchant.actor) CloseInventory();
    }

    void CloseInventory()
    {
        inventory.open = false;
        const int notice = merchant.notice;
        merchant = {};
        merchant.notice = notice;
    }

    bool BeginHeldDrop()
    {
        if (TradeItemLocked()) return false;
        if (equipment.heldSlot >= 0 || inventory.heldShortcut >= 0) return false;
        if (inventory.heldGold) {
            BeginInventoryDrop(mir3_cui::DropDialog::Gold,true);
            if (inventory.drop == mir3_cui::DropDialog::Gold) inventory.heldGold = false;
            return true;
        }
        const auto* item = inventory.Find(equipment.held);
        if (!item || !item->key.id || item->IsGold()) return false;
        inventory.selection = item->key;
        BeginInventoryDrop(mir3_cui::DropDialog::Item,true);
        if (inventory.drop != mir3_cui::DropDialog::None && (!item->Stacks() || item->count <= 1))
            SubmitInventoryDrop();
        return true;
    }

    void QuoteMerchantItem(int cell)
    {
        using Service = mir3_cui::MerchantService;
        if (!merchant.actor || !inventory.open || merchant.quantityOpen ||
            merchant.phase == Service::Phase::Quoting || merchant.phase == Service::Phase::Paying ||
            !InventoryCanQueue()) return;
        const auto* item = inventory.AtCell(cell);
        if (!item) return;
        merchant.item = item->key;
        merchant.price = 0;
        if (merchant.kind == Service::Kind::Deposit) {
            merchant.phase = Service::Phase::Quoted;
            return;
        }
        merchant.phase = Service::Phase::Quoting;
        Queue(merchant.kind == Service::Kind::Sell ? ClientQuoteSell : ClientQuoteRepair,
            static_cast<WORD>(item->key.id), item->key.name, merchant.actor, 0, static_cast<WORD>(item->key.id >> 16));
    }

    void ConfirmMerchantItem(unsigned quantity = 0)
    {
        using Service = mir3_cui::MerchantService;
        if (!merchant.actor || !inventory.open || merchant.phase != Service::Phase::Quoted ||
            !InventoryCanQueue()) return;
        const auto* item = inventory.Find(merchant.item);
        if (!item) {
            merchant.phase = Service::Phase::Choosing;
            merchant.price = 0;
            return;
        }
        const bool deposit = merchant.kind == Service::Kind::Deposit;
        const bool selling = merchant.kind == Service::Kind::Sell;
        if ((selling || deposit) && item->Stacks() && item->count > 1 && !quantity) {
            merchant.quantityOpen = true;
            merchant.quantity[0] = '\0';
            ++merchant.editor;
            return;
        }
        if (deposit && item->Stacks() && !quantity) quantity = 1;
        if (quantity > item->count) return;
        merchant.phase = Service::Phase::Paying;
        Queue(deposit ? ClientStorageDeposit : selling ? ClientSell : ClientRepair,
            static_cast<WORD>(item->key.id), item->key.name,
            merchant.actor, static_cast<WORD>(quantity), static_cast<WORD>(item->key.id >> 16));
    }

    void SubmitMerchantQuantity()
    {
        if (!std::exchange(merchant.quantityOpen, false)) return;
        const auto* item = inventory.Find(merchant.item);
        const long requested = ReadMerchantQuantity(merchant.quantity, item ? item->count : 0);
        if (requested > 0 && requested <= 65535) ConfirmMerchantItem(static_cast<unsigned>(requested));
    }

    void WithdrawStorage(unsigned quantity = 0)
    {
        if (stage != Stage::GameSession || !shop.storage || !shop.actor || shop.withdrawing ||
            shop.offer < 0 || shop.offer >= static_cast<int>(shop.offers.size()) || output_.size() >= 8192) return;
        const auto& item = shop.offers[shop.offer];
        if (item.stackMode >= 2 && item.count > 1 && !quantity) {
            shop.withdrawalPrompt = true;
            shop.quantity[0] = '\0';
            return;
        }
        if (item.stackMode >= 2 && !quantity) quantity = 1;
        if (quantity > item.count) return;
        shop.withdrawing = item.id;
        Queue(ClientStorageWithdraw, static_cast<WORD>(item.id), item.name, shop.actor,
            static_cast<WORD>(quantity), static_cast<WORD>(item.id >> 16));
    }

    void SubmitStorageQuantity()
    {
        if (!std::exchange(shop.withdrawalPrompt, false) || shop.offer < 0 ||
            shop.offer >= static_cast<int>(shop.offers.size())) return;
        const long requested = ReadMerchantQuantity(shop.quantity, shop.offers[shop.offer].count);
        if (requested > 0 && requested <= 65535) WithdrawStorage(static_cast<unsigned>(requested));
    }

    void PageStorage(int direction)
    {
        if (!shop.storage || shop.withdrawing) return;
        const int last = shop.offers.empty() ? 0 : static_cast<int>((shop.offers.size()-1)/12);
        shop.page = static_cast<unsigned>(std::clamp(static_cast<int>(shop.page)+direction,0,last));
        shop.offer = -1;
    }

    void RequestShopOffers(int category, unsigned page)
    {
        if (stage != Stage::GameSession || !shop.actor || shop.detailDeadline || purchase.deadline ||
            category < 0 || category >= static_cast<int>(shop.goods.size()) || page > 65535 / 10 ||
            output_.size() >= 8192) return;
        const auto& good = shop.goods[category];
        if (category != shop.category) {
            shop.offers.clear();
            shop.offer = -1;
        }
        shop.category = category;
        shop.page = page;
        shop.bulk = good.mode == 2;
        shop.detailed = !shop.bulk;
        shop.status.clear();
        shop.lastRow = -1;
        if (shop.bulk) {
            shop.offersVisible = false;
            shop.quantity[0] = '\0';
        } else {
            Queue(ClientShopOffers, static_cast<WORD>(page * 10), good.name, shop.actor);
            shop.detailDeadline = GetTickCount64() + 8000;
        }
    }

    void BuyShopItem()
    {
        if (stage != Stage::GameSession || !shop.actor || purchase.deadline || shop.detailDeadline ||
            output_.size() >= 8192) return;
        unsigned id = 0, count = 0, pendingOffer = 0;
        const std::string* name = nullptr;
        if (shop.bulk && shop.category >= 0) {
            const auto& good = shop.goods[shop.category];
            count = shop.Quantity();
            if (!count) {
                shop.status = "Enter a quantity from 1 to 65535.";
                return;
            }
            id = good.stock;
            name = &good.name;
        } else if (shop.detailed && shop.offer >= 0 && shop.offer < static_cast<int>(shop.offers.size())) {
            const auto& offer = shop.offers[shop.offer];
            id = pendingOffer = offer.id;
            name = &offer.name;
        }
        if (!name) return;
        Queue(ClientBuy, static_cast<WORD>(id), *name, shop.actor, static_cast<WORD>(count),
            static_cast<WORD>(id >> 16));
        purchase = {shop.actor, pendingOffer, shop.revision, GetTickCount64() + 8000};
        shop.status.clear();
    }

    bool ClickNpc(int x, int y)
    {
        const auto found = std::find_if(scene.players.begin(), scene.players.end(), [&](const auto& entry) {
            const auto& actor = entry.second;
            return actor.IsNpc() && !actor.dead && static_cast<int>(actor.x) == x && static_cast<int>(actor.y) == y;
        });
        return found != scene.players.end() && ClickNpcActor(found->first);
    }

    bool ClickNpcActor(std::uint32_t id)
    {
        const auto found = scene.players.find(id);
        if (found == scene.players.end() || !found->second.IsNpc() || found->second.dead) return false;
        const auto& npc = found->second;
        const auto now = GetTickCount64();
        if (stage == Stage::GameSession && (!lastNpcClick || now - lastNpcClick > 1500) &&
            std::abs(static_cast<int>(scene.x) - static_cast<int>(npc.x)) <= 2 &&
            std::abs(static_cast<int>(scene.y) - static_cast<int>(npc.y)) <= 2 &&
            output_.size() < 8192) {
            Queue(ClientClickNpc, 0, "", id);
            lastNpcClick = now;
        }
        return true;
    }

    void RequestMonsterAttributes(std::uint32_t id, LocalScene::Player& monster)
    {
        if (stage != Stage::GameSession || monster.monsterAttributes.icon != -1 || output_.size() >= 8192) return;
        Queue(ClientTargetAttributes, static_cast<WORD>(monster.x), "", id, 0, static_cast<WORD>(monster.y));
        monster.monsterAttributesRequested = true;
    }

    void ChooseNpcOption(const mir3_cui::NpcDialogue::Row& option)
    {
        const auto now = GetTickCount64();
        if (stage != Stage::GameSession || !npcDialogue.actor || option.command.empty() ||
            (lastNpcAction && now - lastNpcAction <= 1000) || output_.size() >= 8192) return;
        const auto command = ConvertText(option.command, CP_UTF8, 936);
        if (command.empty()) return;
        lastNpcAction = now;
        if (option.command.compare(0, 2, "@@") == 0) {
            npcDialogue.prompt = option;
            npcDialogue.answer[0] = '\0';
            npcDialogue.answerTooLong = false;
            ++npcDialogue.editor;
        } else {
            Queue(ClientNpcSelect, 0, command, npcDialogue.actor);
        }
    }

    void EndNpcPrompt(bool submit)
    {
        if (submit) {
            if (stage != Stage::GameSession || output_.size() >= 8192 || !npcDialogue.actor) return;
            const auto& option = npcDialogue.prompt.command;
            const auto command = ConvertText(option, CP_UTF8, 936);
            const auto answer = ConvertText(npcDialogue.answer, CP_UTF8, 936);
            if (answer.size() > NpcAnswerLimit() || std::strpbrk(npcDialogue.answer, "\r\n") ||
                (npcDialogue.answer[0] && answer.empty())) {
                npcDialogue.answerTooLong = true;
                return;
            }
            Queue(ClientNpcSelect, 0, command + '\r' + answer, npcDialogue.actor);
        }
        SecureZeroMemory(npcDialogue.answer, sizeof npcDialogue.answer);
        npcDialogue.prompt = {};
        npcDialogue.answerTooLong = false;
    }

    unsigned NpcAnswerLimit() const
    {
        const auto& option = npcDialogue.prompt.command;
        if (option.starts_with("@@*") &&
            (option.find("enter_stgpasswd") != option.npos || option.find("Input_stgpasswd") != option.npos ||
             option.find("Change_stgpasswd") != option.npos || option.find("confirm_passwd") != option.npos)) {
            const auto delimiter = option.find('_', 3);
            const unsigned limit = delimiter >= 10 ? 10 : Number(option.substr(3, delimiter - 3), 90);
            if (limit) return limit;
        }
        return 90;
    }

    void MoveInScene(int targetX, int targetY, bool run)
    {
        using mir3_cui::GroundCell;
        if (stage != Stage::GameSession || showSignOut || !scene.terrain.Ready() || scene.pending.deadline ||
            output_.size() >= 8192 || (characterStatus.hp[1] && !characterStatus.hp[0])) return;
        if (!scene.terrain.Contains(targetX,targetY)) return;
        const int x = static_cast<int>(scene.x), y = static_cast<int>(scene.y);
        if (targetX == x && targetY == y) return;
        const int dx = (targetX > x) - (targetX < x), dy = (targetY > y) - (targetY < y);
        const auto heading = [&](int stepX, int stepY) {
            const std::array<int, 2> step{stepX, stepY};
            return static_cast<unsigned>(std::find(SceneDirections.begin(), SceneDirections.end(), step) -
                SceneDirections.begin());
        };
        if (run && std::abs(targetX - x) <= 1 && std::abs(targetY - y) <= 1) {
            scene.facing = heading(dx, dy);
            return;
        }
        const auto now = GetTickCount64();
        if (scene.lastMove && now - scene.lastMove < 600) return;
        const auto coordinates = [](int cellX, int cellY) {
            return std::uint32_t(cellX) | (std::uint32_t(cellY) << 16);
        };
        const auto walkable = [&](int cellX, int cellY) {
            const auto ground = scene.GroundAt(cellX, cellY);
            return (ground == GroundCell::Walkable || ground == GroundCell::DoorOpen) &&
                !scene.Occupied(cellX, cellY);
        };
        int nextX = x + dx, nextY = y + dy;
        if (scene.GroundAt(nextX, nextY) == GroundCell::DoorClosed) {
            if (const auto door = scene.terrain.DoorAt(nextX, nextY)) {
                Queue(ClientOpenDoor, static_cast<WORD>(nextX), "", door, 0, static_cast<WORD>(nextY));
                scene.lastMove = now;
            }
            return;
        }
        if (walkable(nextX, nextY)) {
            if (run && walkable(nextX + dx, nextY + dy)) {
                nextX += dx;
                nextY += dy;
            }
        } else {
            nextX = x;
            nextY = y;
            int nearest = 10000;
            for (const auto& direction : SceneDirections) {
                const int candidateX = x + direction[0], candidateY = y + direction[1];
                if (!walkable(candidateX, candidateY)) continue;
                const int distanceX = candidateX - targetX, distanceY = candidateY - targetY;
                const int distance = distanceX * distanceX + distanceY * distanceY;
                if (distance > nearest) continue;
                nearest = distance;
                nextX = candidateX;
                nextY = candidateY;
            }
        }
        if (nextX == x && nextY == y) return;
        const unsigned facing = heading((nextX > x) - (nextX < x), (nextY > y) - (nextY < y));
        const auto tick = static_cast<DWORD>(now);
        const WORD opcode = std::abs(nextX - x) > 1 || std::abs(nextY - y) > 1 ? ClientRun : ClientWalk;
        Queue(opcode, static_cast<WORD>(tick), "", coordinates(nextX, nextY), static_cast<WORD>(tick >> 16),
            static_cast<WORD>(facing));
        scene.pending = {static_cast<unsigned>(nextX), static_cast<unsigned>(nextY), facing, now + 8000};
        scene.lastMove = now;
    }

    void RequestInventory()
    {
        if (!InventoryCanQueue()) return;
        Queue(81, 0, "");
        inventory.waiting = true;
    }

    void UseInventoryItem()
    {
        const auto* item = inventory.Find(inventory.selection);
        if (item) UseItem(*item);
    }

    void ReleaseBagCell(int cell, bool rightHalf = false, bool bottomHalf = false)
    {
        if (trade.offerSlot>=0) return;
        if (const auto* item = trade.SelectedReturn()) {
            if (!trade.returnPending && output_.size()<8192 && !merchant.actor) {
                Queue(ClientTradeReturn,0,item->key.name,item->key.id);
                trade.returnPending = true;
            }
            return;
        }
        if (inventory.request != mir3_cui::ItemRequest::None ||
            equipment.request != mir3_cui::EquipmentRequest::None) return;
        if (inventory.heldShortcut >= 0) {
            inventory.heldShortcut = -1;
            equipment.ReleaseHeld();
            return;
        }
        auto* target = inventory.AtCell(cell);
        const auto placement = [&](const mir3_cui::InventoryItem& item) {
            constexpr int columns = mir3_cui::InventoryPanel::Columns;
            const int x = cell % columns - item.width / 2 + (item.width % 2 == 0 && rightHalf);
            const int y = cell / columns - item.height / 2 + (item.height % 2 == 0 && bottomHalf);
            return x < 0 || x >= columns || y < 0 ? -1 : y * columns + x;
        };
        if (equipment.heldSlot >= 0) {
            const auto* item = HeldItem();
            if (!item) return;
            const int anchor = mir3_cui::InventoryPanel::FirstFit(inventory.Occupied(), *item, placement(*item));
            if (anchor < 0)
                equipment.status = "There is no room for this item.";
            else UnequipItem(anchor);
            return;
        }
        const auto now = GetTickCount64();
        auto* source = inventory.Find(equipment.held);
        if (!source) {
            if (!target) return;
            inventory.selection = target->key;
            equipment.held = target->key;
            equipment.lastRelease = now;
            return;
        }
        if (source == target && now - equipment.lastRelease < 300 && source->key.id) {
            UseItem(*source);
            return;
        }
        const int origin = source->cell;
        const int anchor = target == source ? origin : placement(*source);
        if (!inventory.Move(*source, target == source ? nullptr : target, anchor)) {
            inventory.status = "The item does not fit there.";
            return;
        }
        inventory.status.clear();
        if (target && target != source) {
            equipment.held = target->key;
            inventory.selection = target->key;
            equipment.lastRelease = now;
        } else {
            inventory.selection = source->key;
            equipment.ReleaseHeld();
        }
        if (origin != source->cell || (target && target != source)) SaveInventoryLayout();
    }

    void OpenEquipment(bool fromInventory = false)
    {
        if (stage != Stage::GameSession) return;
        equipment.open = true;
        const auto* item = inventory.Find(inventory.selection);
        if (fromInventory && item && equipment.heldSlot < 0) {
            for (int slot = 0; slot < mir3_cui::EquipmentSlots; ++slot) {
                if (mir3_cui::EquipmentFits(item->mode, slot)) {
                    equipment.selected = slot;
                    equipment.held = item->key;
                    break;
                }
            }
        }
    }

    void EquipItem()
    {
        if (!EquipmentCanQueue()) return;
        if (equipment.heldSlot >= 0) return;
        const auto* item = inventory.Find(equipment.held.name.empty() ? inventory.selection : equipment.held);
        if (!item || !mir3_cui::EquipmentFits(item->mode, equipment.selected)) return;
        Queue(1003, static_cast<WORD>(equipment.selected), item->key.name, item->key.id);
        equipment.pending = *item;
        equipment.pendingSlot = equipment.selected;
        equipment.previous = equipment.slots[equipment.selected].key;
        equipment.request = mir3_cui::EquipmentRequest::Wear;
        equipment.deadline = GetTickCount64() + 10000;
    }

    void ReleaseEquipmentSlot(int slot)
    {
        equipment.selected = slot;
        if (!EquipmentCanQueue()) return;
        const auto now = GetTickCount64();
        auto& target = equipment.slots[slot];
        if (equipment.held.name.empty()) {
            if (target.key.name.empty()) return;
            equipment.held = target.key;
            equipment.heldSlot = slot;
            equipment.lastRelease = now;
            return;
        }
        if (equipment.heldSlot < 0) {
            const auto* item = inventory.Find(equipment.held);
            if (!item || !mir3_cui::EquipmentFits(item->mode, slot)) {
                equipment.status = "This item does not fit that slot.";
                return;
            }
            EquipItem();
            return;
        }
        auto& source = equipment.slots[equipment.heldSlot];
        if (!(source.key == equipment.held)) {
            equipment.ReleaseHeld();
            return;
        }
        const bool quick = now - equipment.lastRelease < 300;
        equipment.lastRelease = now;
        if (!quick || !mir3_cui::EquipmentActivates(source, slot)) {
            if (slot == equipment.heldSlot ||
                (target.key.name.empty() && mir3_cui::EquipmentFits(source.mode, slot))) equipment.ReleaseHeld();
            return;
        }
        if (slot != equipment.heldSlot) {
            auto* destination = &target;
            if (!target.key.name.empty()) {
                const auto free = std::find_if(equipment.slots.begin(), equipment.slots.end(),
                    [](const mir3_cui::InventoryItem& item) { return item.key.name.empty(); });
                if (free == equipment.slots.end()) return;
                destination = &*free;
            }
            *destination = std::move(source);
            source = {};
        }
        equipment.ReleaseHeld();
        Queue(1124, 0, "", slot);
    }

    void UnequipItem(int cell = -1)
    {
        if (!EquipmentCanQueue()) return;
        const int slot = equipment.heldSlot >= 0 ? equipment.heldSlot : equipment.selected;
        const auto& item = equipment.slots[slot];
        if (item.key.name.empty()) return;
        if (inventory.initialized && inventory.items.size() >= mir3_cui::InventoryPanel::Capacity) {
            equipment.status = "Free a bag slot first.";
            return;
        }
        Queue(1004, static_cast<WORD>(slot), item.key.name, item.key.id);
        equipment.pending = item;
        equipment.pendingSlot = slot;
        equipment.request = mir3_cui::EquipmentRequest::Remove;
        equipment.deadline = GetTickCount64() + 10000;
        inventory.returningEquipment = item.key;
        inventory.returnCell = cell;
    }

    const mir3_cui::InventoryItem* HeldItem()
    {
        if (equipment.held.name.empty()) return nullptr;
        if (equipment.heldSlot < 0) return inventory.Find(equipment.held);
        const auto& item = equipment.slots[equipment.heldSlot];
        return item.key == equipment.held ? &item : nullptr;
    }

    void ActivateEquipment()
    {
        if (!EquipmentCanQueue()) return;
        if ((!HeldItem() || equipment.heldSlot == equipment.selected) &&
            mir3_cui::EquipmentActivates(equipment.slots[equipment.selected], equipment.selected)) {
            equipment.ReleaseHeld();
            Queue(1124, 0, "", equipment.selected);
        }
    }

    void ToggleItemShortcuts()
    {
        inventory.shortcutsVisible = !inventory.shortcutsVisible;
        inventory.clickedShortcut = -1;
        if (inventory.shortcutsVisible && !inventory.initialized && !inventory.waiting) RequestInventory();
    }

    void UseItemShortcut(std::size_t slot)
    {
        const auto* item = inventory.ResolveShortcut(slot);
        if (!item) {
            inventory.shortcutStatus = "No usable item is bound to that slot.";
            if (!inventory.shortcuts[slot].name.empty() && !inventory.initialized && !inventory.waiting)
                RequestInventory();
            return;
        }
        UseItem(*item);
        inventory.shortcutStatus = inventory.status;
    }

    void ReleaseShortcutSlot(std::size_t slot)
    {
        if (trade.returnSlot>=0) { CancelTradeReturnSelection(); return; }
        if (!InventoryCanQueue() || equipment.heldSlot >= 0 || inventory.heldGold) return;
        const auto* target = inventory.ResolveShortcut(slot);
        const auto* held = HeldItem();
        if (!held) {
            if (!target) return;
            const auto key = target->key;
            inventory.shortcuts[slot] = {};
            equipment.held = key;
            inventory.heldShortcut = static_cast<int>(slot);
        } else if (held->QuickEligible()) {
            const auto previous = target ? target->key : mir3_cui::ItemKey{};
            if (previous.name == held->key.name) {
                merchantResults.push_back({1069,held->label,static_cast<unsigned>(slot+1)});
                return;
            }
            if (!target && std::any_of(inventory.shortcuts.begin(),inventory.shortcuts.end(),
                [&](const auto& key) { return key.name == held->key.name; })) {
                merchantResults.push_back({1067,held->label,static_cast<unsigned>(slot+1)});
                return;
            }
            if (inventory.heldShortcut < 0 && !target)
                merchantResults.push_back({1068,held->label,static_cast<unsigned>(slot+1)});
            inventory.shortcuts[slot] = held->key;
            equipment.ReleaseHeld();
            equipment.held = previous;
            inventory.heldShortcut = target ? static_cast<int>(slot) : -1;
        }
        SaveItemShortcuts();
    }

    void ActivateShortcutSlot(std::size_t slot, bool mouse)
    {
        if (!InventoryCanQueue() || equipment.heldSlot >= 0 || inventory.heldGold) return;
        const auto* target = mouse ? HeldItem() : inventory.ResolveShortcut(slot);
        const auto now = GetTickCount64();
        const bool fast = target && target->mode == 0 &&
            (target->icon == 20 || target->icon == 21 || target->icon == 70);
        if (inventory.lastUse && now-inventory.lastUse <= (fast ? 1000u : 2000u)) {
            merchantResults.push_back({target ? 1051 : 1066,"",static_cast<unsigned>(slot+1)});
            return;
        }
        if (!mouse) {
            const auto* held = HeldItem();
            if (!target && held && held->QuickEligible())
                for (auto& key : inventory.shortcuts)
                    if (key.name == held->key.name) key = {};
            ReleaseShortcutSlot(slot);
        }
        if (const auto* held = HeldItem(); held && held->QuickEligible() && inventory.heldShortcut >= 0) {
            const int source = inventory.heldShortcut;
            if (UseItem(*held)) {
                inventory.pendingShortcut = source;
                inventory.heldShortcut = -1;
            }
        } else merchantResults.push_back({1066,"",static_cast<unsigned>(slot+1)});
    }

    void BindItemShortcut(std::size_t slot)
    {
        if (stage != Stage::GameSession || inventory.request != mir3_cui::ItemRequest::None) return;
        const auto* selected = inventory.Find(inventory.selection);
        if (!selected || !selected->QuickEligible() || selected->key.name.find_first_of("\r\n") != std::string::npos)
            return;
        for (auto& key : inventory.shortcuts) if (key.name == selected->key.name) key = {};
        inventory.shortcuts[slot] = selected->key;
        inventory.shortcutsVisible = true;
        inventory.bindingShortcut = false;
        inventory.clickedShortcut = -1;
        inventory.shortcutStatus.clear();
        SaveItemShortcuts();
    }

    void ClearItemShortcut(std::size_t slot)
    {
        if (stage != Stage::GameSession || inventory.request != mir3_cui::ItemRequest::None ||
            inventory.shortcuts[slot].name.empty()) return;
        inventory.shortcuts[slot] = {};
        inventory.clickedShortcut = -1;
        inventory.shortcutStatus.clear();
        SaveItemShortcuts();
    }

    void BeginInventoryDrop(mir3_cui::DropDialog drop, bool sceneDrop = false)
    {
        if (!InventoryCanQueue()) return;
        inventory.drop = drop;
        inventory.sceneDrop = sceneDrop;
        inventory.amount[0] = '\0';
        ++inventory.editor;
    }

    void SubmitInventoryDrop()
    {
        using mir3_cui::DropDialog;
        using mir3_cui::ItemRequest;
        if (!InventoryCanQueue()) return;
        const auto drop = inventory.drop;
        if (inventory.sceneDrop) inventory.drop = DropDialog::None;
        if (drop == DropDialog::Gold) {
            const auto parsed = std::strtol(inventory.amount,nullptr,10);
            const auto amount = inventory.sceneDrop
                ? (parsed > 0 ? static_cast<unsigned>(parsed) : 0u) : Number(inventory.amount, inventory.gold);
            if (!amount || amount >= inventory.gold) {
                inventory.status = "Enter a positive amount below your gold balance.";
                return;
            }
            Queue(1016, 0, "", amount);
            inventory.request = ItemRequest::DropGold;
        } else if (drop == DropDialog::Item) {
            const auto* item = inventory.Find(inventory.selection);
            if (!item) {
                inventory.status = "The item is no longer in the bag.";
                inventory.drop = DropDialog::None;
                return;
            }
            inventory.pending = *item;
            if (item->Stacks() && item->count > 1) {
                const std::string text = inventory.amount;
                const long asked = text.empty() || text == "*" ? static_cast<long>(item->count) :
                    inventory.sceneDrop ? std::strtol(inventory.amount,nullptr,10) : Number(text, 65535);
                const auto quantity = (std::min)(asked,100L);
                if (quantity <= 0 || static_cast<unsigned>(quantity) > item->count) {
                    inventory.status = "Enter a quantity available in the stack.";
                    return;
                }
                inventory.dropCount = quantity;
                Queue(1101, 0, item->key.name, item->key.id, static_cast<WORD>(quantity));
                inventory.request = ItemRequest::DropStack;
            } else {
                const DWORD tick = GetTickCount();
                Queue(1001, static_cast<WORD>(tick), item->key.name, item->key.id, static_cast<WORD>(tick >> 16));
                inventory.request = ItemRequest::Drop;
            }
        } else {
            return;
        }
        if (inventory.sceneDrop && drop == DropDialog::Item) equipment.ReleaseHeld();
        inventory.drop = DropDialog::None;
        inventory.deadline = GetTickCount64() + 10000;
    }

    void RequestQuests()
    {
        if (!QuestCanQueue()) return;
        Queue(mir3_cui::QuestQuery, 0, "");
        quests.waiting = true;
    }

    void SelectQuest(int id, int child = -1)
    {
        quests.parent = id;
        quests.child = child;
        quests.resetDescription = true;
        const auto* summary = quests.Description();
        if (child >= 0 && child <= 65535 && id <= 65535 && summary && summary->empty() && QuestCanQueue())
            Queue(mir3_cui::QuestTextQuery, 0, "", child, static_cast<WORD>(id));
    }

    void ExpandQuest(mir3_cui::QuestEntry& entry, bool toggle = true)
    {
        const int id = entry.record.id;
        quests.expanded = toggle && quests.expanded == id ? -1 : id;
        if (quests.expanded < 0) return;
        if (quests.parent != id || quests.child >= 0) SelectQuest(id);
        if (!entry.childrenLoaded && id <= 65535 && QuestCanQueue())
            Queue(mir3_cui::QuestChildrenQuery, 0, "", 0, static_cast<WORD>(id));
    }

    void RequestGuildPage(int page)
    {
        if (stage != Stage::GameSession || (page != 2 && !GuildCanQueue())) return;
        guild.tab = page;
        guild.first = 0;
        if (page == 0) {
            Queue(mir3_cui::GuildHome, 0, "");
            guild.requested = !guild.open;
        } else if (page == 1) {
            Queue(mir3_cui::GuildMembers, 0, "");
        }
    }

    void BeginGuildAction(mir3_cui::GuildAction action)
    {
        if (!guild.overview.manager) {
            guild.notice = 4000;
            return;
        }
        guild.Begin(action);
        if (!guild.Multiline()) return;
        const auto& rows = action == mir3_cui::GuildAction::Notice ? guild.overview.editable : guild.roster.editable;
        if (action == mir3_cui::GuildAction::Ranks && !guild.roster.received) {
            guild.Begin(mir3_cui::GuildAction::None);
            guild.error = "Load the member list before editing ranks.";
            return;
        }
        std::string text;
        bool first = true;
        for (const auto& row : rows) {
            const auto line = ConvertText(row, 936, CP_UTF8);
            if (text.size() + line.size() + 1 >= sizeof guild.input) {
                guild.error = "This text is too large to edit.";
                return;
            }
            if (!first) text += '\n';
            text += line;
            first = false;
        }
        strcpy_s(guild.input, text.c_str());
    }

    void SubmitGuildAction()
    {
        using mir3_cui::GuildAction;
        if (stage != Stage::GameSession || !guild.overview.manager || guild.action == GuildAction::None ||
            !GuildCanQueue())
            return;
        const auto raw = ConvertText(guild.input, CP_UTF8, 936);
        std::string body;
        if (guild.Multiline()) {
            bool emptyLine = true;
            for (std::size_t i = 0; i < raw.size(); ++i) {
                const char ch = raw[i];
                if (ch == '\r' && i + 1 < raw.size() && raw[i + 1] == '\n') continue;
                if (ch == '\r' || ch == '\n') {
                    if (emptyLine) body += ' ';
                    body += '\r';
                    emptyLine = true;
                } else {
                    body += ch;
                    emptyLine = false;
                }
            }
            if (body.empty() || body.size() > 3999) {
                guild.error = "Enter text that fits within 3999 GBK bytes.";
                return;
            }
        } else {
            body = raw;
            const std::size_t limit = guild.action == GuildAction::BreakAlliance ? 63 : 31;
            if (body.empty() || body.size() > limit || body.find_first_of("/\r\n") != std::string::npos) {
                guild.error = "Enter a name of up to " + std::to_string(limit) + " GBK bytes without a slash.";
                return;
            }
        }

        if (guild.action == GuildAction::BreakAlliance) {
            Queue(mir3_cui::Say, 0, "\xc6\xc6\xbb\xb5\xbd\xe1\xc3\xcb " + body);
            Queue(mir3_cui::GuildHome, 0, "");
        } else {
            Queue(static_cast<WORD>(guild.action), 0, body);
            if (guild.action == GuildAction::Remove) Queue(mir3_cui::GuildMembers, 0, "");
        }
        guild.Begin(GuildAction::None);
    }

    void SubmitPartyMember()
    {
        if (stage != Stage::GameSession || party.action == mir3_cui::PartyAction::None) return;
        const auto name = ConvertText(party.name, CP_UTF8, 936);
        if (name.empty() || name.size() > 31 || name.find_first_of("/\r\n") != std::string::npos) {
            party.error = "Enter a character name of up to 31 GBK bytes without a slash.";
            return;
        }
        if (output_.size() >= 8192) {
            party.error = "The connection is busy. Try submitting the name again.";
            return;
        }
        Queue(static_cast<WORD>(party.action), 0, name);
        party.Begin(mir3_cui::PartyAction::None);
    }

    bool SaveAudioSettings()
    {
        if (!audioDirty) return true;
        if (!audioOptionsPath.empty() && audioOptions.Save(audioOptionsPath.c_str())) {
            audioSaveError.clear();
            audioDirty = false;
            return true;
        }
        audioSaveError = "Could not save audio settings.";
        return false;
    }

    void Poll()
    {
        const auto now = GetTickCount64();
        if (returnToCharactersAt && now>=returnToCharactersAt && (!Active() || output_.empty() || now>=deadline_)) {
            Close();
            ClearGameState();
            ClearCharacterSelection();
            OpenCharacters();
            return;
        }
        if (minimapDeadline && now>=minimapDeadline) {
            minimapDeadline=0;
            minimap.pending=portraitMapPending=false;
        }
        if (npcDialogue.actor && npcDialogue.originX && npcDialogue.originY &&
            ((std::max)(std::abs(static_cast<int>(scene.x) - static_cast<int>(npcDialogue.originX)),
                        std::abs(static_cast<int>(scene.y) - static_cast<int>(npcDialogue.originY))) > 5))
            CloseNpcDialogue();
        if (shop.detailDeadline && now >= shop.detailDeadline) {
            shop.detailDeadline = 0;
            shop.status = "No response for this page.";
        }
        if (purchase.deadline && now >= purchase.deadline) {
            if (shop.actor == purchase.actor) shop.status = "The purchase request timed out.";
            purchase = {};
        }
        if (scene.pending.deadline && GetTickCount64() >= scene.pending.deadline) scene.pending = {};
        if (equipment.request != mir3_cui::EquipmentRequest::None && GetTickCount64() >= equipment.deadline) {
            if (equipment.request == mir3_cui::EquipmentRequest::Remove &&
                inventory.returningEquipment == equipment.pending.key) {
                inventory.returningEquipment = {};
                inventory.returnCell = -1;
            }
            equipment.request = mir3_cui::EquipmentRequest::None;
            equipment.ReleaseHeld();
            equipment.status = "The equipment request timed out.";
        }
        if (inventory.request != mir3_cui::ItemRequest::None && GetTickCount64() >= inventory.deadline) {
            FinishShortcutUse(true);
            inventory.request = mir3_cui::ItemRequest::None;
            RequestInventory();
            inventory.status = "The item request timed out. Refreshing the bag.";
        }
        if (stage == Stage::Handoff && GetTickCount64() >= deadline_) OpenCharacters();
        if (stage == Stage::ConnectingGame && !Active()) OpenGameGateway();
        if (!Active()) return;
        const bool characterConnection = stage == Stage::Characters || stage == Stage::ConnectingCharacters;
        const bool gameConnection = IsGameConnection();
        if (stage != Stage::Servers && stage != Stage::GameSession &&
            (stage != Stage::Characters || characterRequest) &&
            GetTickCount64() >= deadline_) {
            if (gameConnection) EndGameSession("The game gateway timed out.");
            else Reset(characterConnection ? "The character gateway timed out." : "The login gateway timed out.");
            return;
        }
        if (stage == Stage::Connecting || stage == Stage::ConnectingCharacters || stage == Stage::ConnectingGame) {
            fd_set writable, failed;
            FD_ZERO(&writable);
            FD_ZERO(&failed);
            FD_SET(socket_, &writable);
            FD_SET(socket_, &failed);
            timeval timeout = {};
            if (select(0, nullptr, &writable, &failed, &timeout) == SOCKET_ERROR ||
                FD_ISSET(socket_, &failed)) {
                if (gameConnection) EndGameSession("Could not connect to the game gateway.");
                else Reset(characterConnection ? "Could not connect to the character gateway."
                                               : "Could not connect to the login gateway.");
                return;
            }
            if (!FD_ISSET(socket_, &writable)) return;
            if (gameConnection) {
                AuthenticateGame();
            } else if (characterConnection) {
                stage = Stage::Characters;
                QueryCharacters();
            } else {
                Wait(Stage::SigningIn, "Waiting for sign-in...");
            }
        }

        if (!output_.empty()) {
            const int sent = send(socket_, output_.data() + sent_,
                static_cast<int>(output_.size() - sent_), 0);
            if (sent > 0) {
                sent_ += sent;
                if (sent_ == output_.size()) ClearOutput();
            } else if (sent == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) {
                if (gameConnection) EndGameSession("Could not send to the game gateway.");
                else Reset(characterConnection ? "Could not send to the character gateway."
                                               : "Could not send to the login gateway.");
                return;
            }
        }

        if (stage == Stage::SigningOutGame && output_.empty() && !returnToCharactersAt) {
            EndGameSession("You signed out.");
            return;
        }
        char buffer[4096];
        const int count = recv(socket_, buffer, sizeof buffer, 0);
        if (count == 0 || (count == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)) {
            if (gameConnection) EndGameSession("The game gateway disconnected.");
            else Reset(characterConnection ? "The character gateway disconnected." : "The login gateway disconnected.");
            return;
        }
        for (int i = 0; i < count && Active(); ++i) Receive(buffer[i]);
    }

private:
    bool EquipmentCanQueue()
    {
        if (stage != Stage::GameSession || TradeItemLocked()) return false;
        if (equipment.request != mir3_cui::EquipmentRequest::None ||
            inventory.request != mir3_cui::ItemRequest::None || output_.size() >= 8192) {
            equipment.status = "Wait for the current item request before trying again.";
            return false;
        }
        equipment.status.clear();
        return true;
    }

    bool UseItem(const mir3_cui::InventoryItem& item)
    {
        if (!InventoryCanQueue() || item.IsGold()) return false;
        const auto now = GetTickCount64();
        const bool fast = item.mode == 0 && (item.icon == 20 || item.icon == 21 || item.icon == 70);
        if (item.Medicine() && inventory.lastUse && now - inventory.lastUse <= (fast ? 1000u : 2000u)) {
            inventory.status = "This item is cooling down.";
            return false;
        }
        const auto tick = static_cast<DWORD>(now);
        Queue(1006, static_cast<WORD>(tick), item.key.name, item.key.id, static_cast<WORD>(tick >> 16));
        inventory.pending = item;
        if (equipment.held == item.key && equipment.heldSlot < 0) equipment.ReleaseHeld();
        inventory.request = mir3_cui::ItemRequest::Use;
        inventory.deadline = now + 10000;
        if (item.Medicine()) inventory.lastUse = now;
        return true;
    }

    bool InventoryCanQueue()
    {
        if (stage != Stage::GameSession || TradeItemLocked()) return false;
        if (inventory.request != mir3_cui::ItemRequest::None ||
            equipment.request != mir3_cui::EquipmentRequest::None) {
            inventory.status = "Waiting for the previous item request.";
            return false;
        }
        if (output_.size() >= 8192) {
            inventory.status = "The connection is busy. Try again.";
            return false;
        }
        inventory.status.clear();
        return true;
    }

    bool QuestCanQueue()
    {
        if (stage != Stage::GameSession) return false;
        if (output_.size() >= 8192) {
            quests.error = "The connection is busy. Try again.";
            return false;
        }
        quests.error.clear();
        return true;
    }

    bool GuildCanQueue()
    {
        if (output_.size() < 8192) {
            guild.error.clear();
            return true;
        }
        guild.error = "The connection is busy. Try again.";
        return false;
    }

    bool SendOptionCommand(const char* command, std::string& error)
    {
        if (stage != Stage::GameSession || showSignOut) return false;
        if (output_.size() >= 8192) {
            error = "The connection is busy. Try changing the option again.";
            return false;
        }
        Queue(mir3_cui::Say, 0, command);
        error.clear();
        return true;
    }

    SOCKET socket_ = INVALID_SOCKET;
    unsigned sequence_ = 0;
    ULONGLONG deadline_ = 0;
    bool receiving_ = false;
    std::string input_;
    std::string output_;
    size_t sent_ = 0;
    std::string signedInAccount_;
    std::string gameRole_;
    std::string realm_;
    std::array<unsigned char, 4> sceneKey_ = {};
    std::uint32_t localActor_ = 0;
    bool itemShortcutFilePresent_ = false;
    mir3_cui::InventoryDimensions inventoryDimensions_;
    std::filesystem::path resourceRoot_ = ".";

    std::string CharacterStoragePath(const std::string& directory, const char* extension = ".txt") const
    {
        static const char hex[] = "0123456789abcdef";
        std::string file = directory + '/' + host + '-' + port + '-';
        for (const auto* identity : {&realm_, &gameRole_}) {
            for (unsigned char ch : *identity) {
                file += hex[ch >> 4];
                file += hex[ch & 15];
            }
            file += '-';
        }
        return file + extension;
    }

    void LoadChatHistory()
    {
        std::ifstream file(CharacterStoragePath(chatHistoryDirectory), std::ios::binary);
        if (!file) {
            LoadLegacyChatHistory();
            return;
        }
        std::string line;
        while (std::getline(file, line)) {
            if (line.size() < 2 || line.size() > sizeof chat.draft || line.find('\0') != std::string::npos)
                continue;
            const auto value = line.substr(1);
            if (line.front() == 'P') chat.Remember(chat.phrases, value);
            if (line.front() == 'C') chat.Remember(chat.contacts, value);
        }
    }

    void LoadLegacyChatHistory()
    {
        const auto role = ConvertText(gameRole_,936,CP_UTF8);
        const auto realm = ConvertText(realm_,936,CP_UTF8);
        const auto safe = [](const std::string& name) {
            return !name.empty() && name!="." && name!=".." &&
                name.find_first_of("/\\:\"<>|?*")==std::string::npos &&
                std::none_of(name.begin(),name.end(),[](unsigned char byte) { return byte<32; });
        };
        if (!safe(role) || !safe(realm)) return;
        for (bool phrase : {false,true}) {
            const auto suffix = phrase ? ".mlt" : ".slt";
            const auto path = resourceRoot_/"Data"/std::filesystem::u8path(role+"("+realm+")"+suffix);
            std::ifstream file(path,std::ios::binary);
            if (!file) continue;
            std::array<char,10241> bytes{};
            file.read(bytes.data(),bytes.size());
            if (!file.eof() || file.bad()) continue;
            const std::string_view contents(bytes.data(),static_cast<std::size_t>(file.gcount()));
            auto& history = phrase ? chat.phrases : chat.contacts;
            std::size_t start = 0;
            while (history.size()<10) {
                const auto end = contents.find('/',start);
                if (end==contents.npos || end-start>1023) break;
                auto entry = ConvertText(std::string(contents.substr(start,end-start)),936,CP_UTF8);
                if (!entry.empty() && entry.size()+1<sizeof chat.draft && entry.find_first_of("\r\n")==std::string::npos &&
                    entry.find('\0')==std::string::npos) {
                    if (!phrase) entry.insert(entry.begin(),'/');
                    chat.Remember(history,entry);
                }
                start = end+1;
            }
        }
    }

    void SaveChatHistory()
    {
        const auto path = CharacterStoragePath(chatHistoryDirectory);
        const auto temporary = path + '.' + std::to_string(GetCurrentProcessId()) + ".tmp";
        CreateDirectoryA(chatHistoryDirectory.c_str(), nullptr);
        std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
        for (const auto& line : chat.contacts) file << 'C' << line << '\n';
        for (const auto& line : chat.phrases) file << 'P' << line << '\n';
        file.close();
        if (!file || !MoveFileExA(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING)) {
            DeleteFileA(temporary.c_str());
            chat.error = "Could not save chat shortcuts.";
        }
    }

    void LoadItemShortcuts()
    {
        std::ifstream file(CharacterStoragePath(itemShortcutDirectory), std::ios::binary);
        itemShortcutFilePresent_ = file.is_open();
        std::string name;
        for (auto& key : inventory.shortcuts) {
            if (!std::getline(file, name)) break;
            if (!name.empty() && name.size() <= 30 && name.find('\0') == std::string::npos &&
                name.find('\r') == std::string::npos &&
                std::none_of(inventory.shortcuts.begin(), inventory.shortcuts.end(), [&](const auto& bound) {
                    return bound.name == name;
                }))
                key = {0, name};
        }
    }

    void SaveItemShortcuts()
    {
        const auto path = CharacterStoragePath(itemShortcutDirectory);
        const auto temporary = path + '.' + std::to_string(GetCurrentProcessId()) + ".tmp";
        CreateDirectoryA(itemShortcutDirectory.c_str(), nullptr);
        std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
        for (const auto& key : inventory.shortcuts) file << key.name << '\n';
        file.close();
        if (!file || !MoveFileExA(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING)) {
            DeleteFileA(temporary.c_str());
            inventory.shortcutStatus = "Could not save item shortcuts.";
        }
    }

    void RestoreInventoryLayout(std::vector<mir3_cui::InventoryItem>& records)
    {
        std::ifstream file(CharacterStoragePath(inventoryLayoutDirectory, ".itms"), std::ios::binary);
        if (!file) return;
        std::string data(mir3_cui::InventoryLayout::FileSize, '\0');
        if (!file.read(&data[0], data.size()) || file.peek() != std::char_traits<char>::eof()) return;
        mir3_cui::InventoryLayout::Restore(data, records, itemShortcutFilePresent_ ? nullptr : &inventory.shortcuts);
    }

    void SaveInventoryLayout()
    {
        if (!inventory.initialized || gameRole_.empty()) return;
        const auto data = mir3_cui::InventoryLayout::Encode(inventory);
        const auto path = CharacterStoragePath(inventoryLayoutDirectory, ".itms");
        const auto temporary = path + '.' + std::to_string(GetCurrentProcessId()) + ".tmp";
        CreateDirectoryA(inventoryLayoutDirectory.c_str(), nullptr);
        std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
        file.write(data.data(), data.size());
        file.close();
        if (!file || !MoveFileExA(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING)) {
            DeleteFileA(temporary.c_str());
            inventory.status = "Could not save the bag layout.";
        }
    }

    void RemoveInventoryItem(const mir3_cui::ItemKey& key)
    {
        if (equipment.heldSlot < 0 && equipment.held == key) {
            equipment.ReleaseHeld();
            inventory.heldShortcut = -1;
        }
        // Resolve references before erasing the record they may be bound to.
        bool changed = false;
        for (std::size_t i = 0; i < inventory.shortcuts.size(); ++i) {
            const auto* item = inventory.ResolveShortcut(i);
            if (item && item->key == key) {
                inventory.shortcuts[i] = {};
                changed = true;
            }
        }
        inventory.Remove(key);
        if (changed) SaveItemShortcuts();
    }

    void FinishShortcutUse(bool restore)
    {
        const int slot = std::exchange(inventory.pendingShortcut,-1);
        if (slot < 0 || !restore || !inventory.Find(inventory.pending.key)) return;
        const auto same = std::find_if(inventory.shortcuts.begin(),inventory.shortcuts.end(),
            [&](const auto& item) { return item.name == inventory.pending.key.name; });
        if (same != inventory.shortcuts.end()) return;
        auto target = inventory.shortcuts.begin()+slot;
        if (!target->name.empty())
            target = std::find_if(inventory.shortcuts.begin(),inventory.shortcuts.end(),
                [](const auto& item) { return item.name.empty(); });
        if (target != inventory.shortcuts.end()) {
            *target = inventory.pending.key;
            SaveItemShortcuts();
        }
    }

    bool IsGameConnection() const
    {
        return stage == Stage::ConnectingGame || stage == Stage::AuthenticatingGame ||
            stage == Stage::GameSession || stage == Stage::SigningOutGame;
    }

    void EndGameSession(const char* message)
    {
        if (returnToCharactersAt) {
            Close();
            return;
        }
        if (stage == Stage::GameSession || stage == Stage::SigningOutGame) SaveInventoryLayout();
        Close();
        CloseNpcDialogue();
        merchant = {};
        merchantResults.clear();
        skills = {};
        mount = {};
        trade = {};
        mountInputUntil = 0;
        skillShortcutsVisible = false;
        ClearMinimap();
        stage = Stage::GameDisconnected;
        status = message;
        showSignOut = false;
        quitGameRequested = false;
        if (audioOpen) SaveAudioSettings();
        audioOpen = false;
    }

    void AuthenticateGame()
    {
        sceneKey_ = {};
        localActor_ = 0;
        // The first game frame has a sequence digit and an encoded body, with no default header.
        constexpr std::uint32_t version = 0x0131a4f0u;
        const auto session = static_cast<std::uint32_t>(selectionSession);
        const auto signedText = [](std::uint32_t value) {
            return std::to_string(static_cast<std::int64_t>(value) -
                (value >= 0x80000000u ? 0x100000000LL : 0));
        };
        const std::string body = "**" + signedInAccount_ + '/' + gameRole_ + '/' +
            signedText(session ^ 0x3eb2c5ccu) + '/' +
            signedText(version ^ session ^ 0x5580af27u) + '/' +
            signedText(version ^ 0xfa0280afu) + "/1";
        QueueFrame(nullptr, body);
        Wait(Stage::AuthenticatingGame, "Waiting for game sign-in...");
    }

    void OpenGameGateway()
    {
        sockaddr_in address = {};
        address.sin_family = AF_INET;
        address.sin_port = htons(static_cast<u_short>(gameGateway.port));
        InetPtonA(AF_INET, gameGateway.host.c_str(), &address.sin_addr);
        const int result = Connect(address);
        if (result == SOCKET_ERROR) EndGameSession("Could not connect to the game gateway.");
        else if (result == 0) AuthenticateGame();
        else Wait(Stage::ConnectingGame, "Connecting to the game gateway...");
    }

    int Connect(const sockaddr_in& address)
    {
        Close();
        socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        u_long nonBlocking = 1;
        if (!Active() || ioctlsocket(socket_, FIONBIO, &nonBlocking) == SOCKET_ERROR)
            return SOCKET_ERROR;
        const int result = connect(socket_, reinterpret_cast<const sockaddr*>(&address), sizeof address);
        return result == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK ? 1 : result;
    }

    void RequestCharacter(WORD opcode, const std::string& body)
    {
        Queue(opcode, 0, body);
        characterRequest = opcode;
        deadline_ = GetTickCount64() + 10000;
    }

    void QueryCharacters()
    {
        RequestCharacter(CM_QUERYCHR, signedInAccount_ + '/' + std::to_string(selectionSession));
    }

    void OpenCharacters()
    {
        sockaddr_in address = {};
        address.sin_family = AF_INET;
        address.sin_port = htons(static_cast<u_short>(selectionGateway.port));
        InetPtonA(AF_INET, selectionGateway.host.c_str(), &address.sin_addr);
        const int result = Connect(address);
        if (result == SOCKET_ERROR) {
            Reset("Could not connect to the character gateway.");
        } else if (result == 0) {
            stage = Stage::Characters;
            QueryCharacters();
        } else {
            Wait(Stage::ConnectingCharacters, "Connecting to the character gateway...");
        }
    }

    void HandleCharacterFrame(const _TDEFAULTMESSAGE& header, const std::string& body)
    {
        switch (header.wIdent) {
        case SM_QUERYCHR:
            if (characterRequest != CM_QUERYCHR) break;
            characterRequest = 0;
            if (!ReadCharacters(header.nRecog, body, characters)) {
                ShowCharacterNotice(220, "The character gateway sent an invalid character list.");
                break;
            }
            characterPage = 0;
            characterIndex = characters.empty() ? -1 : 0;
            ++characterListRevision;
            if (characters.empty() && characterNotice.empty()) ShowCharacterNotice(219, "This account has no characters.");
            break;
        case SM_QUERYCHR_FAIL:
            if (characterRequest != CM_QUERYCHR) break;
            characterRequest = 0;
            ShowCharacterNotice(220, "Could not load the character list.");
            break;
        case SM_NEWCHR_SUCCESS:
            if (characterRequest != CM_NEWCHR) break;
            CloseCharacterDialog();
            QueryCharacters();
            break;
        case SM_NEWCHR_FAIL:
            if (characterRequest != CM_NEWCHR) break;
            characterRequest = 0;
            ShowCharacterNotice(header.nRecog == 2 ? 224 : header.nRecog == 3 ? 226 : 232,
                header.nRecog == 2 ? "That character name already exists." :
                header.nRecog == 3 ? "This account has no free character slot." : "Could not create the character.");
            break;
        case SM_DELCHR_FAIL:
            characterNotice = "The character gateway declined to delete the character.";
            break;
        case SM_STARTPLAY:
            if (characterRequest != CM_SELCHR) break;
            characterRequest = 0;
            gameGateway = ReadGateway(body, false);
            if (!gameGateway.port) {
                ShowCharacterNotice(221, "The character gateway returned an invalid game address.");
                break;
            }
            gameRole_ = characters[characterIndex].name;
            gameCharacter = characters[characterIndex].label;
            Close();
            Wait(Stage::ConnectingGame, "Connecting to the game gateway...");
            break;
        case SM_STARTFAIL:
            if (characterRequest != CM_SELCHR) break;
            characterRequest = 0;
            ShowCharacterNotice(221, "The character could not enter the game.");
            break;
        case SM_OUTOFCONNECTION:
            Reset("The character gateway ended the session.");
            break;
        }
    }

    void ClearOutput()
    {
        if (!output_.empty()) SecureZeroMemory(&output_[0], output_.size());
        output_.clear();
        sent_ = 0;
    }

    void Close()
    {
        if (Active()) closesocket(socket_);
        socket_ = INVALID_SOCKET;
        sequence_ = 0;
        receiving_ = false;
        input_.clear();
        ClearOutput();
    }

    void Wait(Stage next, const char* message)
    {
        stage = next;
        status = message;
        deadline_ = GetTickCount64() + 10000;
    }

    void Queue(WORD opcode, WORD parameter, const std::string& body, std::uint32_t recognition = 0, WORD series = 0,
        WORD tag = 0)
    {
        _TDEFAULTMESSAGE header;
        static_assert(sizeof(header) == 12, "Login header wire size");
        fnMakeDefMessage(&header, opcode, static_cast<int>(recognition), parameter, tag, series);
        char encodedHeader[DEFBLOCKSIZE + 1];
        fnEncodeMessage(&header, encodedHeader, sizeof encodedHeader);
        QueueFrame(encodedHeader, body);
    }

    void QueueFrame(const char* encodedHeader, const std::string& body)
    {
        std::vector<char> encodedBody((body.size() * 4 + 2) / 3 + 1);
        fnEncode6BitBuf(reinterpret_cast<unsigned char*>(const_cast<char*>(body.data())),
            encodedBody.data(), static_cast<int>(body.size()), static_cast<int>(encodedBody.size()));
        sequence_ = sequence_ % 9 + 1;
        output_ += '#';
        output_ += static_cast<char>('0' + sequence_);
        if (encodedHeader) output_ += encodedHeader;
        output_ += encodedBody.data();
        output_ += '!';
        SecureZeroMemory(encodedBody.data(), encodedBody.size());
    }

    void Receive(char byte)
    {
        if (byte == '*') {
            if (output_.size() >= 8192) {
                if (IsGameConnection()) EndGameSession("The game gateway sent too many pending heartbeats.");
                else Reset("The gateway sent too many pending heartbeats.");
                return;
            }
            output_ += '*';
        } else if (byte == '#') {
            input_.clear();
            receiving_ = true;
        } else if (receiving_) {
            if (byte == '!') {
                receiving_ = false;
                HandleFrame();
                input_.clear();
            } else {
                const bool game = IsGameConnection();
                if (input_.size() >= (game ? 65536u : 4096u) || (!game && (byte < 60 || byte > 123))) {
                    if (game) EndGameSession("The game gateway sent an oversized reply.");
                    else Reset("The gateway sent an invalid reply.");
                    return;
                }
                input_ += byte;
            }
        }
    }

    void HandleGameFrame()
    {
        if (input_.compare(0, 5, "+GOOD") == 0) {
            if (scene.pending.deadline) {
                scene.SetCell(scene.pending.x, scene.pending.y);
                scene.facing = scene.pending.facing;
                scene.pending = {};
            }
            return;
        }
        if (input_.compare(0, 5, "+FAIL") == 0) {
            scene.pending = {};
            return;
        }
        if (input_.size() < DEFBLOCKSIZE) return;
        std::array<char, DEFBLOCKSIZE + 1> encodedHeader = {};
        for (std::size_t i = 0; i < DEFBLOCKSIZE; ++i) {
            if (input_[i] < 60 || input_[i] > 123) return;
            encodedHeader[i] = input_[i];
        }
        std::array<unsigned char, 12> header = {};
        if (fnDecode6BitBuf(encodedHeader.data(), reinterpret_cast<char*>(header.data()), static_cast<int>(header.size())) != 12)
            return;
        const unsigned opcode = (header[0] ^ sceneKey_[0]) |
            (static_cast<unsigned>(header[4] ^ sceneKey_[2] ^ sceneKey_[1]) << 8);
        const std::uint32_t actor = (header[5] ^ sceneKey_[3]) | (std::uint32_t(header[7]) << 8) |
            (std::uint32_t(header[9]) << 16) | (std::uint32_t(header[11]) << 24);
        const auto first = static_cast<WORD>(header[1] | (header[6] << 8));
        const auto second = static_cast<WORD>(header[2] | (header[8] << 8));
        const auto third = static_cast<WORD>(header[3] | (header[10] << 8));
        if (opcode == GameLogon && stage == Stage::AuthenticatingGame) {
            if (!actor || actor > 0x7fffffffu || input_.size() != DEFBLOCKSIZE + 51 ||
                (third & 0xff) >= 8) return;
            // These sections have independent six-bit padding.
            const auto feature = DecodeGameBytes(input_.substr(DEFBLOCKSIZE, 22));
            const auto extension = DecodeGameBytes(input_.substr(DEFBLOCKSIZE + 22, 7));
            const auto extra = DecodeGameBytes(input_.substr(DEFBLOCKSIZE + 29, 22));
            if (feature.size() != 16 || extension.size() != 5 || extra.size() != 16) return;
            mir3_cui::CharacterAppearance appearance;
            if (!appearance.Apply(PackedAppearance(feature)) || appearance.profession > 2) return;

            stage = Stage::GameSession;
            characterAppearance = appearance;
            const auto mountShape = static_cast<unsigned char>(extension[0]);
            mount.shape = mountShape < 4 ? mountShape : 0;
            localActor_ = actor;
            scene.SetCell(first, second);
            scene.facing = third & 0xff;
            LoadChatHistory();
            LoadItemShortcuts();
            if (portraitMapMode && !scene.mapId.empty()) {
                portraitMapPending=true;
                RequestMinimap();
            }
            status = "Signed in to the game gateway.";
        } else if (opcode == SM_CERTIFICATION_FAIL || opcode == SM_OUTOFCONNECTION) {
            EndGameSession(opcode == SM_CERTIFICATION_FAIL ? "The game gateway rejected sign-in."
                                                         : "The game gateway ended the session.");
        } else if (opcode == GameDecodeKey && stage == Stage::GameSession) {
            const auto body = ReadGameBody();
            if (body.size() == 60 && body.find('\0') == std::string::npos) ApplySceneKey(body);
        } else if (opcode == GameNewMap || opcode == GameChangeMap || opcode == GameMapDescription) {
            HandleMapUpdate(opcode, first, second);
        } else if (stage == Stage::GameSession && (opcode == GameMinimap || opcode == GameMinimapUnavailable)) {
            if (input_.size() != DEFBLOCKSIZE) return;
            if (opcode == GameMinimap) {
                if (first) minimapReply=first;
            } else {
                minimapReply=0;
                minimapDeadline=0;
                minimap.reply_time=GetTickCount64();
                minimap.visible=minimap.pending=minimap.dragging=minimap.release_captured=false;
                portraitMapReady=portraitMapPending=false;
                minimapUnavailable=true;
            }
        } else if (stage == Stage::GameSession && (opcode == GameAddSkill || opcode == GameSkillList)) {
            HandleSkillRecords(opcode == GameSkillList, third & 0xff);
        } else if (stage == Stage::GameSession && opcode == GameRideHorse) {
            if (input_.size() == DEFBLOCKSIZE) {
                mount.apply_reply(actor);
                scene.pending = {};
                scene.lastMove = 0;
            }
        } else if (stage == Stage::GameSession && opcode>=GameTradeOpen && opcode<=GameTradeComplete) {
            HandleTradeReply(opcode,actor,first,second);
        } else if (stage == Stage::GameSession && (opcode == GameRemoveSkill || opcode == GameSkillProgress)) {
            if (actor && actor <= 65535 && input_.size() == DEFBLOCKSIZE) {
                if (opcode == GameRemoveSkill) skills.remove(static_cast<int>(actor));
                else skills.update_progress(static_cast<int>(actor),second | (std::uint32_t(third)<<16),first & 0xff);
            }
        } else if (stage == Stage::GameSession && (opcode == GameDoorOpen || opcode == GameDoorClosed)) {
            if (input_.size() == DEFBLOCKSIZE && scene.terrain.UpdateDoor(first, second, opcode == GameDoorOpen))
                scene.ground = scene.terrain.ReadView(scene.x, scene.y);
        } else if (stage == Stage::GameSession &&
                   (opcode == GameBackstep || opcode == GameTurn || opcode == GameWalk || opcode == GameRun)) {
            if (actor == localActor_) HandleLocalMovement(opcode, first, second, third & 0xff);
            else HandleRemotePlayer(opcode, actor, first, second, third & 0xff);
        } else if (stage == Stage::GameSession &&
                   (opcode == GameActorDeath || opcode == GameActorDead ||
                    opcode == GameActorRevive || opcode == GameActorReviveRefresh ||
                    opcode == GameMonsterDigUp || opcode == GameMonsterRise || opcode == GameMonsterSkeleton)) {
            if (actor != localActor_) HandleRemotePlayer(opcode, actor, first, second, third & 0xff);
        } else if (stage == Stage::GameSession && opcode == GameActorName) {
            const auto player = scene.players.find(actor);
            if (player != scene.players.end() && input_.size() <= DEFBLOCKSIZE + 344) {
                auto body = ReadGameBody();
                if (!body.empty()) SetRemoteName(player->second, std::move(body));
            }
        } else if (stage == Stage::GameSession && opcode == GameTargetPortrait) {
            ApplyTargetPortrait(actor,first | (std::uint32_t(second)<<16));
        } else if (stage == Stage::GameSession && opcode == GameTargetAttributes) {
            ApplyMonsterAttributes(actor,first,second);
        } else if (stage == Stage::GameSession && (opcode == GameActorStruck || opcode == GameActorStruckWithSource)) {
            if (input_.size()==DEFBLOCKSIZE+22) {
                const auto feature=ReadGameBody();
                if (feature.size()==16 && feature[0]==3) ApplyMonsterVitals(actor,first,second);
            }
        } else if (stage == Stage::GameSession && opcode == GameNpcDialogue) {
            if (actor && actor <= 0x7fffffffu && input_.size() <= DEFBLOCKSIZE + (8192 * 4 + 2) / 3) {
                auto body = ReadGameBody();
                const auto terminator = body.find('\0');
                if (terminator != std::string::npos) body.resize(terminator);
                if (!body.empty() && body.size() <= 8192) {
                    const auto previous = npcDialogue.actor;
                    if (npcDialogue.Apply(actor, ConvertText(body, 936, CP_UTF8))) {
                        npcDialogue.originX = scene.x;
                        npcDialogue.originY = scene.y;
                        if (previous != actor) shop = {};
                    }
                }
            }
        } else if (stage == Stage::GameSession && opcode == GameNpcClose) {
            if (input_.size() == DEFBLOCKSIZE) CloseNpcDialogue();
        } else if (stage == Stage::GameSession && (opcode == GameShopGoods || opcode == GameShopOffers ||
                   opcode == GameShopBought || opcode == GameShopRejected)) {
            HandleShopReply(opcode, actor, first, second);
        } else if (stage == Stage::GameSession &&
                   ((opcode >= GameOpenSell && opcode <= GameSellFail) ||
                    (opcode >= GameOpenRepair && opcode <= GameRepairQuote) ||
                    opcode == GameSellStackOk || opcode == GameSellStackFail)) {
            HandleMerchantReply(opcode, actor, first, second);
        } else if (stage == Stage::GameSession && opcode >= GameStorageOpen && opcode <= GameStorageBagFull) {
            HandleStorageReply(opcode, actor, first, second, third);
        } else if (stage == Stage::GameSession &&
                   (opcode == GameActorExit || opcode == GameActorDisappear || opcode == GameActorRemove ||
                    opcode == GameClearActors)) {
            if (input_.size() == DEFBLOCKSIZE) {
                if (opcode == GameClearActors) scene.players.clear();
                else scene.players.erase(actor);
            }
        } else if ((stage == Stage::GameSession || stage == Stage::AuthenticatingGame) &&
                   (opcode == GameAbility || opcode == GameVitals || opcode == GameWeight ||
                    opcode == GameSubAbility || opcode == GameExperience || opcode == GameLevelUp)) {
            HandleCharacterStatus(opcode, actor, first, second, third);
        } else if (opcode == mir3_cui::ClientOptions && stage == Stage::GameSession) {
            if (input_.size() == DEFBLOCKSIZE) {
                const auto changed = serverOptions.Apply(actor, first);
                chat.reception.Apply(actor, changed);
            }
        } else if (stage == Stage::GameSession &&
                   opcode >= static_cast<unsigned>(mir3_cui::PartyReply::Created) &&
                   opcode <= static_cast<unsigned>(mir3_cui::PartyReply::Roster)) {
            HandlePartyReply(static_cast<mir3_cui::PartyReply>(opcode), actor);
        } else if (stage == Stage::GameSession && opcode >= 753 && opcode <= 776) {
            HandleGuildReply(opcode, actor);
        } else if (stage == Stage::GameSession &&
                   ((opcode >= 1301 && opcode <= 1303) || opcode == 1310 || opcode == 1311)) {
            HandleQuestReply(static_cast<mir3_cui::QuestReply>(opcode), first, third);
        } else if (stage == Stage::GameSession &&
                   ((opcode >= 200 && opcode <= 203) || opcode == 600 || opcode == 601 ||
                    opcode == 635 || opcode == 636 || opcode == 653 || opcode == 1313)) {
            HandleInventoryReply(opcode, actor, first, third);
        } else if ((stage == Stage::GameSession || stage == Stage::AuthenticatingGame) &&
                   (opcode == 621 || opcode == 642 || opcode == 615 || opcode == 616 || opcode == 619 || opcode == 620)) {
            HandleEquipmentReply(opcode, actor, first, second);
        } else if (mir3_cui::ChatChannel(opcode) && stage == Stage::GameSession &&
                   input_.size() <= DEFBLOCKSIZE + 1024) {
            auto body = ReadGameBody();
            if (body.empty() && input_.size() != DEFBLOCKSIZE) return;
            const auto terminator = body.find('\0');
            if (terminator != std::string::npos) body.resize(terminator);
            if ((opcode == 40 || opcode == 1501) && body.size() > 259) return;
            // Received chat has two independent append paths in the V1.45 client.
            const int copies = (opcode != 104 ? 1 : 0) + (chat.logging && opcode != 100 ? 1 : 0);
            if (copies) {
                std::ofstream log("Chat.txt", std::ios::binary | std::ios::app);
                for (int i = 0; i < copies && log; ++i) log << body << "\r\n";
            }
            const auto text = ConvertText(body, 936, CP_UTF8);
            if (text.empty()) return;
            constexpr std::array<unsigned,8> palette{0,657930,0xffffff,255,65280,16750230,0x50ffff,33023};
            const auto color = [&](unsigned index) { return index < palette.size() ? palette[index] : 0u; };
            chat.Append(opcode, text, color(opcode == 104 ? 4 : first & 255u),
                color(opcode == 104 ? 0 : second & 255u));
            if (opcode == 104) {
                for (auto& row : mir3::wrap_system_notice(text))
                    guild.AppendHistory({0,std::move(row)});
            }
        }
    }

    static long ReadMerchantQuantity(const char* value, unsigned available)
    {
        return !std::strcmp(value,"*") ? static_cast<long>(available) : std::strtol(value,nullptr,10);
    }

    void StartMerchantService(std::uint32_t actor, mir3_cui::MerchantService::Kind kind)
    {
        merchant = {};
        merchant.actor = actor;
        merchant.revision = ++merchantRevision;
        merchant.kind = kind;
        inventory.open = true;
    }

    void ApplyBagCount(mir3_cui::InventoryItem& item, WORD count)
    {
        if (count > item.count)
            merchantResults.push_back({1707,item.label,count-item.count});
        item.count = count;
    }

    void HandleStorageReply(unsigned opcode, std::uint32_t actor, WORD first, WORD second, WORD count)
    {
        using Service = mir3_cui::MerchantService;
        if (opcode == GameStorageList) {
            const auto storageActor = shop.storage ? shop.actor : actor;
            constexpr std::size_t itemLength = 134, recordLength = itemLength+1;
            if (!storageActor || storageActor > 0x7fffffffu ||
                input_.size() != DEFBLOCKSIZE + count*recordLength) return;
            std::vector<mir3_cui::NpcShop::Offer> offers;
            offers.reserve(count);
            for (std::size_t at = DEFBLOCKSIZE; at < input_.size(); at += recordLength) {
                mir3_cui::InventoryItem item;
                if (input_[at+itemLength] != '/' ||
                    !DecodeInventoryItem(input_.substr(at,itemLength),item,true) || !item.key.id) return;
                offers.push_back({std::move(item.key.name),std::move(item.label),item.key.id,
                    0,item.icon,item.count,item.stackMode});
            }
            const auto pending = shop.storage ? shop.withdrawing : 0;
            const int notice = merchant.notice;
            shop = {};
            merchant = {};
            merchant.notice = notice;
            shop.actor = storageActor;
            shop.storage = shop.offersVisible = true;
            shop.withdrawing = pending;
            shop.revision = ++shopRevision;
            shop.catalogueRevision = shop.revision;
            shop.offers = std::move(offers);
            shop.offer = shop.offers.empty() ? -1 : 0;
            return;
        }
        if (input_.size() != DEFBLOCKSIZE) return;
        if (opcode == GameStorageOpen) {
            if (!actor) return;
            shop = {};
            StartMerchantService(actor,Service::Kind::Deposit);
            return;
        }
        if (opcode <= GameStorageDepositFail) {
            if (opcode != GameStorageDepositOk) merchant.notice = opcode == GameStorageFull ? 1449 : 1450;
            if (!merchant.actor || merchant.kind != Service::Kind::Deposit || merchant.phase != Service::Phase::Paying)
                return;
            if (opcode == GameStorageDepositOk) {
                merchantResults.push_back({second ? 1447 : 1448,ConvertText(merchant.item.name,936,CP_UTF8),second});
                if (!first) RemoveInventoryItem(merchant.item);
                else if (auto* item = inventory.Find(merchant.item)) ApplyBagCount(*item,first);
            }
            merchant.phase = Service::Phase::Choosing;
            return;
        }
        if (opcode != GameStorageWithdrawOk)
            merchant.notice = opcode == GameStorageWithdrawFail ? 1445 : 1446;
        if (!shop.storage || !shop.withdrawing) return;
        const auto pending = std::exchange(shop.withdrawing,0u);
        if (opcode != GameStorageWithdrawOk) return;
        if (second) {
            const auto bagItem = std::find_if(inventory.items.begin(),inventory.items.end(),
                [&](const auto& item) { return item.key.id == pending; });
            if (bagItem != inventory.items.end()) ApplyBagCount(*bagItem,second);
        }
        const auto storedItem = std::find_if(shop.offers.begin(),shop.offers.end(),
            [&](const auto& item) { return item.id == pending; });
        if (storedItem == shop.offers.end()) return;
        if (first) {
            storedItem->count = first;
        } else {
            shop.offers.erase(storedItem);
            shop.offer = -1;
            const auto last = shop.offers.empty() ? 0u : static_cast<unsigned>((shop.offers.size()-1)/12);
            shop.page = (std::min)(shop.page,last);
        }
    }

    void HandleMerchantReply(unsigned opcode, std::uint32_t actor, WORD first, WORD second)
    {
        using Service = mir3_cui::MerchantService;
        if (input_.size() != DEFBLOCKSIZE) return;
        if (opcode == GameOpenSell || opcode == GameOpenRepair) {
            if (!actor) return;
            StartMerchantService(actor,opcode == GameOpenSell ? Service::Kind::Sell : Service::Kind::Repair);
            return;
        }
        if (opcode == GameSellQuote || opcode == GameRepairQuote) {
            const auto kind = opcode == GameSellQuote ? Service::Kind::Sell : Service::Kind::Repair;
            if (merchant.actor && merchant.kind == kind &&
                merchant.phase != Service::Phase::Choosing && merchant.phase != Service::Phase::Paying) {
                merchant.price = actor;
                merchant.phase = actor ? Service::Phase::Quoted : Service::Phase::Unavailable;
            }
            return;
        }

        const bool stack = opcode == GameSellStackOk;
        const bool repair = opcode == GameRepairOk || opcode == GameRepairFail;
        const bool success = opcode == GameSellOk || stack || opcode == GameRepairOk;
        if (success) inventory.gold = actor;
        if (opcode == GameSellFail || opcode == GameRepairFail) merchant.notice = repair ? 1457 : 1455;
        if (!merchant.actor || merchant.kind != (repair ? Service::Kind::Repair : Service::Kind::Sell) ||
            merchant.phase != Service::Phase::Paying) return;
        if (success) {
            merchantResults.push_back({repair ? 1456 : stack ? 1454 : 1453,
                ConvertText(merchant.item.name, 936, CP_UTF8), second});
            if (repair) {
                if (auto* item = inventory.Find(merchant.item)) {
                    item->durability = first;
                    item->maxDurability = second;
                }
            } else if (stack && first) {
                if (auto* item = inventory.Find(merchant.item)) item->count = first;
            } else if (stack) {
                RemoveInventoryItem(merchant.item);
            } else {
                if (equipment.heldSlot < 0 && equipment.held == merchant.item) {
                    equipment.ReleaseHeld();
                    inventory.heldShortcut = -1;
                }
                inventory.Remove(merchant.item);
            }
        }
        merchant.phase = Service::Phase::Choosing;
        merchant.price = 0;
    }

    void HandleShopReply(unsigned opcode, std::uint32_t actor, WORD count, WORD offset)
    {
        if (opcode == GameShopBought || opcode == GameShopRejected) {
            if (input_.size() != DEFBLOCKSIZE || (opcode == GameShopRejected && (actor < 1 || actor > 3))) return;
            if (opcode == GameShopBought) inventory.gold = actor;
            if (shop.storage) {
                purchase = {};
                return;
            }
            if (purchase.deadline && purchase.actor == shop.actor && purchase.revision == shop.revision) {
                if (purchase.offer && (opcode == GameShopBought || actor == 1)) {
                    const auto item = std::find_if(shop.offers.begin(), shop.offers.end(),
                        [&](const mir3_cui::NpcShop::Offer& offer) { return offer.id == purchase.offer; });
                    if (item != shop.offers.end()) shop.offers.erase(item);
                    if (shop.offer >= static_cast<int>(shop.offers.size()))
                        shop.offer = shop.sourceLayout ? -1 : static_cast<int>(shop.offers.size()) - 1;
                }
                static const char* const reasons[] = {"", "This item is sold out.", "You do not have enough gold.",
                    "The shop could not complete the purchase."};
                shop.status = opcode == GameShopBought ? "Purchase confirmed." : reasons[actor];
            } else if (!purchase.deadline && shop.sourceLayout && shop.offersVisible && shop.offer >= 0 &&
                       shop.offer < static_cast<int>(shop.offers.size()) && (opcode == GameShopBought || actor == 1)) {
                shop.offers.erase(shop.offers.begin() + shop.offer);
                if (shop.offer >= static_cast<int>(shop.offers.size())) shop.offer = -1;
            }
            if (shop.actor && opcode == GameShopRejected) shop.notice = 1436 + static_cast<int>(actor);
            purchase = {};
            return;
        }
        if (!actor || count > 255 || input_.size() > DEFBLOCKSIZE + (10240 * 4 + 2) / 3) return;
        auto body = ReadGameBody();
        if (input_.size() > DEFBLOCKSIZE && body.empty()) return;
        if (opcode == GameShopGoods) {
            if (actor != npcDialogue.actor || body.size() > 10240) return;
            const auto zero = body.find('\0');
            if (zero != std::string::npos) body.resize(zero);
            std::vector<mir3_cui::NpcShop::Good> goods;
            std::size_t start = 0;
            for (WORD index = 0; index < count; ++index) {
                std::array<std::string, 7> fields;
                for (auto& field : fields) {
                    const auto end = body.find('/', start);
                    if (end == std::string::npos || end == start) return;
                    field = body.substr(start, end - start);
                    start = end + 1;
                }
                mir3_cui::NpcShop::Good good;
                good.name = std::move(fields[0]);
                good.label = ConvertText(good.name, 936, CP_UTF8);
                if (good.label.empty()) return;
                unsigned* values[] = {&good.mode, &good.price, &good.stock, &good.icon,
                    &good.requirementType, &good.requirement};
                for (int i = 0; i < 6; ++i) {
                    *values[i] = Number(fields[i + 1], i == 3 ? 65535u : 0x7fffffffu, 0xffffffffu);
                    if (*values[i] == 0xffffffffu) return;
                }
                goods.push_back(std::move(good));
            }
            if (start != body.size()) return;
            shop = {};
            shop.actor = actor;
            shop.revision = ++shopRevision;
            shop.catalogueRevision = shop.revision;
            shop.goods = std::move(goods);
            shop.selected = shop.goods.empty() ? -1 : 0;
            return;
        }
        constexpr std::size_t encodedRecordSize = 134;
        if (actor != shop.actor || !shop.detailed || shop.category < 0 || body.size() > 8196 ||
            body.size() != count * (encodedRecordSize + 1)) return;
        std::vector<mir3_cui::NpcShop::Offer> offers;
        for (std::size_t start = 0; start < body.size(); start += encodedRecordSize + 1) {
            mir3_cui::InventoryItem item;
            if (body[start + encodedRecordSize] != '/' ||
                !DecodeInventoryItem(body.substr(start, encodedRecordSize), item, true) || !item.key.id) return;
            offers.push_back({std::move(item.key.name), std::move(item.label), item.key.id, item.maxDurability, item.icon});
        }
        shop.offers = std::move(offers);
        shop.offer = shop.offers.empty() ? -1 : 0;
        shop.page = offset / 10;
        shop.detailDeadline = 0;
        shop.offersVisible = true;
        shop.revision = ++shopRevision;
        shop.status.clear();
        shop.lastRow = -1;
    }

    static std::uint32_t PackedAppearance(const std::string& feature)
    {
        std::uint32_t packed = 0;
        for (int byte = 0; byte < 4; ++byte)
            packed |= std::uint32_t(static_cast<unsigned char>(feature[byte])) << (8 * byte);
        return packed;
    }

    void SendTradeOffer(unsigned amount)
    {
        trade.offerCount = amount;
        const auto& key = trade.offeredItem.key;
        Queue(ClientTradeOffer,0,key.name,key.id,static_cast<WORD>(amount));
    }

    static bool RestoreTradeItem(mir3_cui::InventoryPanel& bag, mir3_cui::InventoryItem item, int preferred = -1)
    {
        if (item.IsGold() || bag.Find(item.key)) return true;
        if (item.Stacks() && std::any_of(bag.items.begin(),bag.items.end(),[&](const auto& owned) {
            return owned.key.name == item.key.name;
        })) return true;
        if (bag.items.size()>=mir3_cui::InventoryPanel::Capacity) return false;
        item.cell = mir3_cui::InventoryPanel::FirstFit(bag.Occupied(),item,preferred);
        if (item.cell<0) return false;
        bag.items.push_back(std::move(item));
        return true;
    }

    void HandleTradeOfferReply(unsigned opcode, std::uint32_t id, WORD remaining)
    {
        if (trade.offerSlot<0 || !trade.offerCount) return;
        auto item = trade.offeredItem;
        auto nextBag = inventory;
        auto nextOffers = trade.items[0];
        if (opcode==GameTradeAddOk) {
            if (remaining && (remaining>=item.count || !id)) return;
            if (remaining) {
                item.count -= remaining;
                item.key.id = id;
            }
            if (!mir3_cui::TradePanel::Insert(nextOffers,item,trade.offerSlot)) return;
            if (remaining) {
                if (auto* owned=nextBag.Find(trade.offeredItem.key)) owned->count = remaining;
                else {
                    auto leftover = trade.offeredItem;
                    leftover.count = remaining;
                    if (!RestoreTradeItem(nextBag,leftover,leftover.cell)) return;
                }
            } else {
                nextBag.Remove(trade.offeredItem.key);
                for (auto& shortcut : nextBag.shortcuts)
                    if (shortcut.name==item.key.name) shortcut = {};
            }
        } else if (!RestoreTradeItem(nextBag,item,item.cell)) return;
        inventory = std::move(nextBag);
        trade.items[0] = std::move(nextOffers);
        if (opcode==GameTradeAddOk && !remaining) SaveItemShortcuts();
        trade.feedback.push_back({0,0,static_cast<int>(item.mode),static_cast<int>(item.subMode),item.label});
        if (equipment.held==trade.offeredItem.key) equipment.ReleaseHeld();
        trade.offerSlot = -1;
        trade.offerCount = 0;
        trade.offeredItem = {};
    }

    void HandleTradeReply(unsigned opcode, std::uint32_t actor, WORD first, WORD second)
    {
        using namespace mir3_cui;
        if (opcode==GameTradeFailed) {
            if (input_.size()==DEFBLOCKSIZE) trade.OpenPrompt(TradePrompt::Failed);
            return;
        }
        if (opcode==GameTradeOpen) {
            if (!trade.visible) {
                const auto body = ReadGameBody();
                if (body.empty() || body.size()>60 || body.find('\0')!=std::string::npos) return;
                auto partner = ConvertText(body,936,CP_UTF8);
                if (partner.empty()) return;
                trade.Clear();
                trade.partner = std::move(partner);
                trade.visible = true;
                ++trade.openRevision;
            }
            if (!inventory.open) {
                trade.repositionInventory = true;
                OpenInventory();
            }
            return;
        }
        if (!trade.visible) return;
        if (opcode==GameTradePeerAdd || opcode==GameTradePeerRemove) {
            InventoryItem item;
            if (!DecodeInventoryItem(input_.substr(DEFBLOCKSIZE),item) || !inventoryDimensions_.HasFrame(item.icon)) return;
            auto& peer = trade.items[1];
            if (opcode==GameTradePeerAdd) {
                if (!TradePanel::Insert(peer,item)) return;
            } else {
                const auto found = std::find_if(peer.begin(),peer.end(),[&](const auto& other) { return other.key==item.key; });
                if (found!=peer.end()) peer.erase(found);
            }
            trade.feedback.push_back({opcode==GameTradePeerAdd ? 1435 : 1436,0,static_cast<int>(item.mode),
                static_cast<int>(item.subMode),item.label});
            return;
        }
        if (input_.size()!=DEFBLOCKSIZE) return;
        switch (opcode) {
        case GameTradeAddOk:
        case GameTradeAddFail:
            HandleTradeOfferReply(opcode,actor,first);
            break;
        case GameTradeReturnOk:
        case GameTradeReturnFail:
            if (const auto* selected = trade.SelectedReturn(); selected && trade.returnPending) {
                const auto item = *selected;
                if (opcode==GameTradeReturnOk && !RestoreTradeItem(inventory,item)) return;
                auto& offers = trade.items[0];
                std::erase_if(offers,[&](const auto& other) { return other.cell==trade.returnSlot; });
                if (opcode==GameTradeReturnFail) TradePanel::Insert(offers,item);
                trade.returnPending = false;
                trade.returnSlot = -1;
                trade.feedback.push_back({0,0,static_cast<int>(item.mode),static_cast<int>(item.subMode),item.label});
            }
            break;
        case GameTradeGold:
            trade.gold[0] = actor;
            trade.goldPending = false;
            inventory.gold = first | (std::uint32_t(second)<<16);
            trade.feedback.push_back({0,106,-1,0,{}});
            break;
        case GameTradePeerGold:
            trade.gold[1] = actor;
            trade.feedback.push_back({0,106,-1,0,{}});
            break;
        case GameTradeCancelled: {
            auto restored = inventory;
            for (const auto& item : trade.items[0])
                if (!item.Stacks() && !RestoreTradeItem(restored,item)) return;
            if (trade.offerSlot>=0) {
                if (!RestoreTradeItem(restored,trade.offeredItem,trade.offeredItem.cell)) return;
                if (equipment.held==trade.offeredItem.key) equipment.ReleaseHeld();
            }
            restored.gold += trade.gold[0];
            inventory = std::move(restored);
            trade.Clear();
            break;
        }
        case GameTradeComplete:
            if (trade.offerSlot<0 && !trade.returnPending) {
                for (const auto& side : trade.items) for (const auto& item : side) {
                    if (item.stackMode<=2 || std::any_of(inventory.items.begin(),inventory.items.end(),
                        [&](const auto& owned) { return owned.icon==item.icon; })) continue;
                    for (auto& shortcut : inventory.shortcuts)
                        if (shortcut.name==item.key.name) shortcut = {};
                }
                SaveItemShortcuts();
                trade.Clear();
            }
            break;
        }
    }

    void HandleSkillRecords(bool full, int count)
    {
        auto next = skills;
        if (full) for (auto& group : next.learned) group.clear();
        else count = 1;
        std::string_view body(input_.data()+DEFBLOCKSIZE,input_.size()-DEFBLOCKSIZE);
        for (int i=0;i<count;++i) {
            const auto end = full ? body.find('/') : body.size();
            if (end == body.npos || end != 99) return;
            const auto record = DecodeGameBytes(std::string(body.substr(0,end)));
            if (record.size() != 74) return;
            const auto byte = [&](int at) { return static_cast<std::uint8_t>(record[at]); };
            const auto dword = [&](int at) {
                return byte(at) | (std::uint32_t(byte(at+1))<<8) | (std::uint32_t(byte(at+2))<<16) |
                    (std::uint32_t(byte(at+3))<<24);
            };
            const int id = byte(6) | (byte(7)<<8), level = byte(5);
            if (!id) return;
            const auto decimal = std::strtol(record.c_str()+4,nullptr,10);
            const int label = decimal>0 && decimal<10 ? int(decimal) : int(byte(4))-55;
            const int binding = full ? label : static_cast<std::uint8_t>(decimal);
            if (binding>=1 && binding<=12) next.bindings[binding] = byte(6);
            std::string_view name(record.data()+8,66);
            const std::array thresholds{static_cast<int>(dword(31)),static_cast<int>(dword(35)),static_cast<int>(dword(39))};
            next.learned[mir3::SkillLayout::category(id)].push_back({id,level,label>=1 && label<=12 ? label : 0,
                dword(0),level<3 ? thresholds[level] : 0,ConvertText(std::string(name.substr(0,name.find('\0'))),936,CP_UTF8),
                thresholds,dword(49),byte(53)});
            if (full) body.remove_prefix(end+1);
        }
        const int previous = next.tab;
        if (full) {
            next.tab = 0;
            next.menu_target = -1;
        }
        next.select_nonempty();
        if (full && next.learned[next.tab].empty()) next.tab = previous;
        skills = std::move(next);
    }

    void HandleMapUpdate(unsigned opcode, WORD x, WORD y)
    {
        if (input_.size() > DEFBLOCKSIZE + 1024) return;
        auto body = ReadGameBody();
        if (body.empty()) return;
        const auto zero = body.find('\0');
        if (zero != std::string::npos) body.resize(zero);
        if (opcode == GameMapDescription) {
            if (body.size() <= 259) scene.mapDescription = ConvertText(body, 936, CP_UTF8);
            return;
        }
        if (body.empty() || body.size() > 64 || !std::all_of(body.begin(), body.end(), [](char c) {
            return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                c == '_' || c == '-';
        })) return;
        scene.mapId = std::move(body);
        CloseNpcDialogue();
        scene.mapDescription.clear();
        scene.pending = {};
        scene.lastMove = 0;
        scene.players.clear();
        scene.x = x;
        scene.y = y;
        scene.terrain.Open(scene.mapId,resourceRoot_/"Map");
        scene.ground = scene.terrain.ReadView(x, y);
        const bool reopen=minimap.visible || minimap.pending;
        const bool large=minimap.large, translucent=minimap.translucent;
        ClearMinimap();
        minimap.large=large;
        minimap.translucent=translucent;
        minimap.pending=reopen;
        portraitMapPending=portraitMapMode;
        if (reopen || portraitMapPending) RequestMinimap();
    }

    void RequestMinimap()
    {
        if (stage!=Stage::GameSession || minimapDeadline || output_.size()>=8192) return;
        Queue(ClientMinimap,0,"");
        minimapDeadline=GetTickCount64()+8000;
    }

    void ClearMinimap()
    {
        minimap={};
        minimapReply=0;
        minimapDeadline=0;
        minimapUnavailable=false;
        portraitMapPending=portraitMapReady=false;
    }

    void HandleLocalMovement(unsigned opcode, WORD x, WORD y, unsigned facing)
    {
        if (facing >= 8) return;
        const auto size = input_.size() - DEFBLOCKSIZE;
        if (opcode == GameBackstep && size != 29) return;
        if (opcode == GameTurn && (size < 29 || size > 1024)) return;
        if (opcode != GameTurn && size != 0 && size != 22 && size != 29) return;
        if (size) {
            const auto feature = DecodeGameBytes(input_.substr(DEFBLOCKSIZE, 22));
            if (feature.size() != 16) return;
            if (size >= 29 && DecodeGameBytes(input_.substr(DEFBLOCKSIZE + 22, 7)).size() != 5) return;
            if (size > 29 && DecodeGameBytes(input_.substr(DEFBLOCKSIZE + 29)).empty()) return;
            if (!characterAppearance.Apply(PackedAppearance(feature))) return;
        }
        scene.SetCell(x, y);
        scene.facing = facing;
        scene.pending = {};
    }

    static void SetRemoteName(LocalScene::Player& player, std::string body)
    {
        const auto zero = body.find('\0');
        if (zero != std::string::npos) body.resize(zero);
        const auto separator = body.find_first_of("\r/");
        if (separator != std::string::npos) body.resize(separator);
        if (body.size() > 63) return;
        const auto name = ConvertText(body, 936, CP_UTF8);
        if ((body.empty() || !name.empty()) && player.name!=name) {
            player.name = name;
            player.portraitJpeg.clear();
            player.resolvedPortraitVersion = 0;
            player.portraitQueryAt = 0;
        }
    }

    void ApplyTargetPortrait(std::uint32_t actor, std::uint32_t version)
    {
        const auto player = scene.players.find(actor);
        if (player==scene.players.end() || player->second.npcType) return;
        auto& target = player->second;
        if (!version && input_.size()==DEFBLOCKSIZE) {
            target.portraitJpeg.clear();
            target.resolvedPortraitVersion = target.portraitVersion;
            return;
        }
        if (!version || version!=target.portraitVersion || input_.size()>DEFBLOCKSIZE+5462) return;
        if (target.resolvedPortraitVersion==version && !target.portraitJpeg.empty()) return;
        auto bytes = ReadGameBody();
        if (bytes.size()<2 || bytes.size()>4096 || bytes.compare(0,2,"\xff\xd8")!=0) return;
        target.portraitJpeg = std::move(bytes);
        target.resolvedPortraitVersion = version;
    }

    void ApplyMonsterAttributes(std::uint32_t actor, WORD category, WORD state)
    {
        const auto found=scene.players.find(actor);
        if (found==scene.players.end() || !found->second.IsMonster() || !found->second.monsterAttributesRequested ||
            input_.size()!=DEFBLOCKSIZE+14) return;
        const auto body=ReadGameBody();
        if (body.size()!=10) return;
        auto& attributes=found->second.monsterAttributes;
        attributes.category=category;
        attributes.state=state;
        attributes.icon=static_cast<std::int8_t>(body[0]);
        attributes.element=static_cast<unsigned char>(body[2]);
        for (std::size_t i=0; i<attributes.modifiers.size(); ++i)
            attributes.modifiers[i]=static_cast<std::int8_t>(body[i+3]);
    }

    void ApplyMonsterVitals(std::uint32_t actor, WORD current, WORD maximum)
    {
        const auto found=scene.players.find(actor);
        if (found==scene.players.end() || !found->second.IsMonster()) return;
        const auto percent=maximum ? (std::min)(unsigned(current)*100/maximum,100u) : 0u;
        found->second.monsterVitals={static_cast<std::uint8_t>(percent),maximum,current};
    }

    void HandleRemotePlayer(unsigned opcode, std::uint32_t actor, WORD x, WORD y, unsigned facing)
    {
        if (!actor || actor > 0x7fffffffu || facing >= 8 ||
            (scene.terrain.Ready() && !scene.terrain.Contains(x, y))) return;
        const auto size = input_.size() - DEFBLOCKSIZE;
        if (size > 344) return;
        const auto feature = size >= 22 ? DecodeGameBytes(input_.substr(DEFBLOCKSIZE, 22)) : std::string();
        const unsigned type = feature.size() == 16 ? static_cast<unsigned char>(feature[0]) : 0;
        if (type == 3) {
            const auto existing=scene.players.find(actor);
            if ((opcode==GameActorDead || opcode==GameActorRevive || opcode==GameActorReviveRefresh) &&
                (existing==scene.players.end() || !existing->second.IsMonster())) return;
            const auto shape=unsigned(static_cast<unsigned char>(feature[2])) |
                (unsigned(static_cast<unsigned char>(feature[3]))<<8);
            auto name=size>22 ? DecodeGameBytes(input_.substr(DEFBLOCKSIZE+22)) : std::string{};
            if (size>22 && (name.empty() || name.size()>64)) return;
            auto& monster=scene.players[actor];
            if (!monster.IsMonster()) monster={};
            monster.x=x;
            monster.y=y;
            monster.facing=facing;
            monster.npcType=type;
            monster.npcShape=shape;
            if (opcode==GameActorDeath || opcode==GameActorDead || opcode==GameMonsterSkeleton) monster.dead=true;
            else if (opcode==GameActorRevive || opcode==GameActorReviveRefresh || opcode==GameMonsterRise) monster.dead=false;
            if (!name.empty()) SetRemoteName(monster,std::move(name));
            return;
        }
        if (opcode==GameMonsterDigUp || opcode==GameMonsterRise || opcode==GameMonsterSkeleton) return;
        if (type == 4 || type == 5) {
            auto& npc = scene.players[actor];
            if (!npc.IsNpc()) npc={};
            npc.x = x;
            npc.y = y;
            npc.facing = facing;
            npc.npcType = type;
            npc.npcShape = static_cast<unsigned char>(feature[type == 4 ? 1 : 2]);
            npc.dead = opcode == GameActorDeath || opcode == GameActorDead;
            if (size > 22) {
                auto name = DecodeGameBytes(input_.substr(DEFBLOCKSIZE + 22));
                if (!name.empty() && name.size() <= 64) SetRemoteName(npc, std::move(name));
            }
            return;
        }
        const bool named = opcode == GameTurn || opcode == GameActorRevive || opcode == GameActorReviveRefresh;
        if (named && (size < 29 || size > 344)) return;
        if (opcode == GameBackstep && size != 29) return;
        if (!named && size != 0 && size != 22 && size != 29) return;
        auto found = scene.players.find(actor);
        const bool newPlayer = found == scene.players.end();
        if (!newPlayer && found->second.IsMonster() && !size) return;
        if (newPlayer && size < 29) return;
        LocalScene::Player next = newPlayer ? LocalScene::Player{} : found->second;
        if (size) {
            if (feature.size() != 16 || !next.appearance.Apply(PackedAppearance(feature))) return;
            next.npcType = 0;
            std::uint32_t portraitVersion = 0;
            for (unsigned i=0;i<4;++i)
                portraitVersion |= std::uint32_t(static_cast<unsigned char>(feature[8+i]))<<(8*i);
            if (next.portraitVersion!=portraitVersion) {
                next.portraitVersion = portraitVersion;
                next.resolvedPortraitVersion = 0;
                next.portraitJpeg.clear();
                next.portraitQueryAt = 0;
            }
            if (size >= 29 && DecodeGameBytes(input_.substr(DEFBLOCKSIZE + 22, 7)).size() != 5) return;
            if (size > 29) {
                auto name = DecodeGameBytes(input_.substr(DEFBLOCKSIZE + 29));
                if (name.empty()) return;
                SetRemoteName(next, std::move(name));
            }
        }
        next.x = x;
        next.y = y;
        next.facing = facing;
        if (opcode == GameActorDeath || opcode == GameActorDead) next.dead = true;
        else if (opcode == GameActorRevive || opcode == GameActorReviveRefresh) next.dead = false;
        if (newPlayer) scene.players.emplace(actor, std::move(next));
        else found->second = std::move(next);
    }

    void HandleCharacterStatus(unsigned opcode, std::uint32_t actor, WORD first, WORD second, WORD third)
    {
        auto& stats = characterStatus;
        if (opcode == GameAbility) {
            if (stats.ApplyAbility(ReadGameBody(), first & 0xff)) inventory.gold = actor;
            return;
        }
        if (input_.size() != DEFBLOCKSIZE) return;
        switch (opcode) {
        case GameVitals:
            if (actor == 0 || actor == localActor_) {
                stats.hp = {first, third};
                stats.mp[0] = second;
                if (third && !first) scene.pending = {};
            } else ApplyMonsterVitals(actor,first,third);
            break;
        case GameWeight:
            stats.weight[0] = actor & 0xffff;
            stats.wear[0] = first & 0xff;
            stats.hand[0] = second & 0xff;
            break;
        case GameSubAbility:
            stats.accuracy = first & 0xff;
            stats.agility = first >> 8;
            break;
        case GameExperience:
            stats.experience = actor;
            if (second)
                stats.weaponTraining = (third & 0xff) ? second : static_cast<WORD>(stats.weaponTraining + second);
            if (third >> 8) chat.Append(100, "Bonus experience.");
            if (second) chat.Append(100, "Experience +" + std::to_string(first) + "  Weapon training +" + std::to_string(second));
            else chat.Append(100, "Experience +" + std::to_string(first));
            break;
        case GameLevelUp:
            if (third == 0) {
                stats.level = first & 0xff;
                stats.experience = actor;
                chat.Append(100, "Level up: " + std::to_string(stats.level));
            }
            break;
        }
    }

    void HandleEquipmentReply(unsigned opcode, std::uint32_t actor, WORD first, WORD second)
    {
        using mir3_cui::EquipmentRequest;
        if (opcode == 621) {
            auto slots = equipment.slots;
            std::size_t cursor = DEFBLOCKSIZE;
            for (int entries = 0; cursor < input_.size(); ++entries) {
                if (entries >= mir3_cui::EquipmentSlots) return;
                const auto slash = input_.find('/', cursor);
                if (slash == std::string::npos || slash - cursor > 2) return;
                const auto slot = Number(input_.substr(cursor, slash - cursor), mir3_cui::EquipmentSlots - 1,
                    mir3_cui::EquipmentSlots);
                if (slot >= mir3_cui::EquipmentSlots || slash + 135 >= input_.size() ||
                    input_[slash + 135] != '/' || !DecodeInventoryItem(input_.substr(slash + 1, 134), slots[slot]))
                    return;
                cursor = slash + 136;
            }
            equipment.slots = std::move(slots);
            if (!HeldItem()) {
                equipment.ReleaseHeld();
                inventory.heldShortcut = -1;
            }
            return;
        }
        if (input_.size() != DEFBLOCKSIZE) return;
        if (opcode == 642) {
            if (first >= equipment.slots.size()) return;
            auto& item = equipment.slots[first];
            if (item.key.name.empty()) return;
            item.durability = actor & 0xffff;
            item.maxDurability = second;
            const auto signedValue = static_cast<std::int64_t>(actor) - (actor >= 0x80000000u ? 0x100000000LL : 0);
            const int threshold = (item.mode == 54 || item.mode == 55) ? 50 : 500;
            if (signedValue < threshold) chat.Append(100, "Low durability: " + item.label);
            return;
        }

        const bool wearing = equipment.request == EquipmentRequest::Wear;
        if (equipment.request == EquipmentRequest::None ||
            (wearing && opcode != 615 && opcode != 616) || (!wearing && opcode != 619 && opcode != 620))
            return;
        auto& slot = equipment.slots[equipment.pendingSlot];
        if (opcode == 615) {
            const auto worn = std::find_if(equipment.slots.begin(), equipment.slots.end(),
                [&](const mir3_cui::InventoryItem& item) { return item.key == equipment.pending.key; });
            if (worn == equipment.slots.end()) {
                if (slot.key == equipment.previous || slot.key.name.empty()) slot = equipment.pending;
                else {
                    const auto free = std::find_if(equipment.slots.begin(), equipment.slots.end(),
                        [](const mir3_cui::InventoryItem& item) { return item.key.name.empty(); });
                    if (free != equipment.slots.end()) *free = equipment.pending;
                }
            }
            RemoveInventoryItem(equipment.pending.key);
            characterAppearance.Apply(actor);
            equipment.status.clear();
        } else if (opcode == 619) {
            for (auto& worn : equipment.slots) {
                if (worn.key == equipment.pending.key) worn = {};
            }
            characterAppearance.Apply(actor);
            equipment.status.clear();
        } else {
            equipment.status = wearing ? "The server declined to equip this item."
                                       : "The server declined to remove this item.";
            if (!wearing && inventory.returningEquipment == equipment.pending.key) {
                inventory.returningEquipment = {};
                inventory.returnCell = -1;
            }
        }
        equipment.request = EquipmentRequest::None;
        equipment.ReleaseHeld();
    }

    void HandleInventoryReply(unsigned opcode, std::uint32_t actor, WORD first, WORD third)
    {
        using mir3_cui::ItemRequest;
        if (opcode >= 200 && opcode <= 203) {
            ApplyBagRecords(opcode, third);
            return;
        }
        if (opcode == 601) {
            auto name = ReadGameBody();
            const auto zero = name.find('\0');
            if (zero != std::string::npos) name.resize(zero);
            if ((inventory.request == ItemRequest::Drop || inventory.request == ItemRequest::DropStack) &&
                actor == inventory.pending.key.id && name == inventory.pending.key.name) {
                inventory.request = ItemRequest::None;
                inventory.status = "The server declined the item drop.";
            }
            return;
        }
        if (input_.size() != DEFBLOCKSIZE) return;
        switch (opcode) {
        case 600:
            if ((inventory.request == ItemRequest::Drop || inventory.request == ItemRequest::DropStack) &&
                (!actor || actor == inventory.pending.key.id)) {
                if (inventory.request == ItemRequest::Drop || inventory.dropCount >= inventory.pending.count)
                    RemoveInventoryItem(inventory.pending.key);
                inventory.request = ItemRequest::None;
                inventory.status.clear();
            }
            break;
        case 635:
        case 636:
            if (inventory.request != ItemRequest::Use) break;
            inventory.request = ItemRequest::None;
            if (opcode == 636) {
                FinishShortcutUse(true);
                inventory.status = "The server declined to use " + inventory.pending.label + ".";
                inventory.shortcutStatus = inventory.status;
                inventory.notice = inventory.pending.mode == 51;
                break;
            }
            if (inventory.pendingShortcut >= 0 && inventory.pending.stackMode != 3) {
                FinishShortcutUse(false);
            } else if (inventory.pending.stackMode == 3 && inventory.pending.count > 1 &&
                (first || inventory.pendingShortcut >= 0)) {
                if (auto* item = inventory.Find(inventory.pending.key)) item->count = first;
                FinishShortcutUse(true);
            } else {
                RemoveInventoryItem(inventory.pending.key);
                FinishShortcutUse(false);
            }
            inventory.status = "Used " + inventory.pending.label + ".";
            inventory.shortcutStatus = inventory.status;
            break;
        case 653:
            inventory.gold = actor;
            if (inventory.request == ItemRequest::DropGold) {
                inventory.request = ItemRequest::None;
                inventory.status.clear();
            }
            break;
        case 1313:
            for (auto& item : inventory.items) {
                if (item.key.id == actor) ApplyBagCount(item,first);
            }
            if (!first) {
                std::vector<mir3_cui::ItemKey> emptyStacks;
                for (const auto& item : inventory.items) {
                    if (item.key.id == actor && item.Stacks()) emptyStacks.push_back(item.key);
                }
                for (const auto& key : emptyStacks) RemoveInventoryItem(key);
            }
            if (inventory.request == ItemRequest::DropStack && inventory.pending.key.id == actor) {
                inventory.request = ItemRequest::None;
                inventory.status.clear();
            }
            break;
        }
    }

    bool DecodeInventoryItem(const std::string& encoded, mir3_cui::InventoryItem& item, bool requireTerminator = false)
    {
        const auto record = DecodeGameBytes(encoded);
        if (record.size() != 100) return false;
        const auto word = [&](std::size_t offset) {
            return static_cast<unsigned char>(record[offset]) |
                (static_cast<unsigned>(static_cast<unsigned char>(record[offset + 1])) << 8);
        };
        auto name = record.substr(0, 30);
        const auto end = name.find('\0');
        if (requireTerminator && end == std::string::npos) return false;
        if (end != std::string::npos) name.resize(end);
        const auto label = ConvertText(name, 936, CP_UTF8);
        if (label.empty()) return false;
        item.key = {word(62) | (std::uint32_t(word(64)) << 16), std::move(name)};
        item.label = label;
        item.icon = word(36);
        const auto dimensions = inventoryDimensions_.Get(item.icon);
        item.width = dimensions[0];
        item.height = dimensions[1];
        item.count = word(71);
        item.durability = word(66);
        item.maxDurability = word(68);
        item.mode = static_cast<unsigned char>(record[30]);
        item.subMode = static_cast<unsigned char>(record[31]);
        item.stackMode = static_cast<unsigned char>(record[61]);
        return true;
    }

    void ApplyBagRecords(unsigned opcode, WORD count)
    {
        const bool full = opcode == 201;
        if (!full) count = 1;
        if (count > 46) return;
        const std::size_t encodedRecordSize = 134;
        const auto size = input_.size() - DEFBLOCKSIZE;
        if (size != count * (encodedRecordSize + (full ? 1 : 0))) return;

        std::vector<mir3_cui::InventoryItem> records;
        std::size_t start = DEFBLOCKSIZE;
        for (WORD i = 0; i < count; ++i) {
            mir3_cui::InventoryItem item;
            if ((full && input_[start + encodedRecordSize] != '/') ||
                !DecodeInventoryItem(input_.substr(start, encodedRecordSize), item)) return;
            records.push_back(std::move(item));
            start += encodedRecordSize + (full ? 1 : 0);
        }
        if (full) {
            if (!inventory.initialized) RestoreInventoryLayout(records);
            inventory.Replace(std::move(records));
            inventory.initialized = true;
            inventory.waiting = false;
            if (!inventory.Find(inventory.selection)) inventory.selection = {};
            if (!HeldItem()) {
                equipment.ReleaseHeld();
                inventory.heldShortcut = -1;
            }
            return;
        }

        auto& item = records.front();
        if (opcode == 202 || opcode == 203) {
            for (auto& slot : equipment.slots) {
                if (!(slot.key == item.key)) continue;
                slot = {};
                if (opcode == 203) {
                    for (auto& freeSlot : equipment.slots) {
                        if (!freeSlot.key.name.empty()) continue;
                        freeSlot = item;
                        break;
                    }
                }
                break;
            }
        }
        auto* current = inventory.Find(item.key);
        // The replaced item can arrive before the wear confirmation frees its bag slot.
        const bool earlyReturn = equipment.request == mir3_cui::EquipmentRequest::Wear &&
            item.key == equipment.previous;
        const bool returnedGear = item.key == inventory.returningEquipment;
        if (opcode == 202) RemoveInventoryItem(item.key);
        else if (current) {
            item.cell = mir3_cui::InventoryPanel::FirstFit(inventory.Occupied(current), item, current->cell);
            *current = std::move(item);
        } else if (opcode == 200 && (inventory.items.size() < mir3_cui::InventoryPanel::Capacity || earlyReturn)) {
            item.cell = mir3_cui::InventoryPanel::FirstFit(inventory.Occupied(), item,
                returnedGear ? inventory.returnCell : -1);
            inventory.items.push_back(std::move(item));
        }
        if (opcode == 200 && returnedGear) {
            inventory.returningEquipment = {};
            inventory.returnCell = -1;
        }
        if (!HeldItem()) {
            equipment.ReleaseHeld();
            inventory.heldShortcut = -1;
        }
    }

    void HandleQuestReply(mir3_cui::QuestReply reply, WORD first, WORD third)
    {
        using mir3_cui::QuestReply;
        const bool text = reply == QuestReply::Text;
        const auto size = input_.size() - DEFBLOCKSIZE;
        if (size > (text ? 1366u : 10922u)) return;
        auto body = ReadGameBody();
        if (size && body.empty()) return;
        const auto zero = body.find('\0');
        if (zero != std::string::npos) body.resize(zero);
        if (text) {
            if (body.size() > 1023) return;
            auto* entry = quests.Find(third);
            auto* child = entry ? entry->FindChild(first) : nullptr;
            if (!child) return;
            const auto decoded = ConvertText(body, 936, CP_UTF8);
            if (decoded.empty() && !body.empty()) return;
            child->summary = decoded;
            return;
        }

        const bool full = reply == QuestReply::Full;
        const bool children = reply == QuestReply::Children;
        int parentId = third;
        std::vector<mir3_cui::QuestRecord> records;
        if (!mir3_cui::DecodeQuestRecords(reply, body, full || children ? first : third, parentId, records)) return;
        for (auto& record : records) {
            record.title = ConvertText(record.title, 936, CP_UTF8);
            const auto summary = ConvertText(record.summary, 936, CP_UTF8);
            if (record.title.empty() || (summary.empty() && !record.summary.empty())) return;
            record.summary = summary;
        }
        const bool childAdded = reply == QuestReply::ChildAdded;
        if (childAdded && records.empty()) return;
        if (childAdded || reply == QuestReply::Added) {
            if (quests.notices.size() == 79) quests.notices.pop_front();
            quests.notices.push_back(childAdded ? 1432 : 1433);
        }
        if (!full && !quests.initialized) {
            RequestQuests();
            return;
        }
        if (full || reply == QuestReply::Added) {
            quests.Sync(records, full);
            return;
        }
        auto* entry = quests.Find(parentId);
        if (!entry || records.empty()) return;
        if (children) {
            for (auto& record : records) record.completed = entry->record.completed;
            entry->children = std::move(records);
            entry->childrenLoaded = true;
            if (quests.parent == parentId) {
                quests.child = -1;
                quests.resetDescription = true;
            }
        } else {
            auto* duplicateParent = quests.Find(first);
            for (auto& record : records) {
                if ((duplicateParent && duplicateParent->FindChild(record.id)) || entry->children.size() >= 2048)
                    break;
                record.completed = entry->record.completed;
                entry->children.push_back(std::move(record));
                entry->childrenLoaded = true;
            }
        }
    }

    void HandleGuildReply(unsigned opcode, std::uint32_t value)
    {
        const bool overview = opcode == 753 || opcode == 773 || opcode == 774;
        const bool roster = opcode == 756 || opcode == 775 || opcode == 776;
        if (overview || roster) {
            const auto size = input_.size() - DEFBLOCKSIZE;
            if (size > (8191 * 4 + 2) / 3) return;
            auto body = ReadGameBody();
            if (size && body.empty()) return;
            const auto zero = body.find('\0');
            if (zero != std::string::npos) body.resize(zero);
            if (overview) {
                const auto previous = guild.overview.name;
                if (guild.overview.Apply(body, opcode == 753) && opcode == 753) {
                    if (guild.overview.name != previous) {
                        guild.roster = {};
                        guild.Begin(mir3_cui::GuildAction::None);
                    } else if (!guild.overview.manager) {
                        guild.Begin(mir3_cui::GuildAction::None);
                    }
                    guild.open = true;
                    guild.requested = false;
                    guild.tab = 0;
                    guild.first = 0;
                    ++guild.overviewRevision;
                }
            } else {
                guild.roster.Apply(body, guild.overview.manager);
            }
            return;
        }
        if (input_.size() != DEFBLOCKSIZE) return;
        if (opcode == 754) guild.requested = false;
        if ((opcode == 757 || opcode == 759) && GuildCanQueue()) Queue(mir3_cui::GuildMembers, 0, "");
        const auto reason = static_cast<std::int64_t>(value) - (value >= 0x80000000u ? 0x100000000LL : 0);
        const int notice = mir3_cui::GuildNoticeId(opcode, reason);
        if (notice) {
            guild.Begin(mir3_cui::GuildAction::None);
            guild.notice = notice;
            if (!guild.overview.name.empty()) guild.AppendHistory({notice,{}});
        }
    }

    void HandlePartyReply(mir3_cui::PartyReply reply, std::uint32_t value)
    {
        using mir3_cui::PartyReply;
        if (reply == PartyReply::Roster) {
            const auto size = input_.size() - DEFBLOCKSIZE;
            if (size > (8192 * 4 + 2) / 3) return;
            const auto body = ReadGameBody();
            if ((size && body.empty()) || body.size() > 8192 || body.find('\0') != std::string::npos)
                return;
            std::vector<std::string> members;
            size_t start = 0;
            for (auto end = body.find('/'); end != std::string::npos; end = body.find('/', start)) {
                if (end - start > 31) return;
                const auto name = body.substr(start, end - start);
                auto label = ConvertText(name, 936, CP_UTF8);
                if (label.empty() && !name.empty()) return;
                members.push_back(std::move(label));
                start = end + 1;
            }
            party.members = std::move(members);
            return;
        }
        if (input_.size() != DEFBLOCKSIZE) return;
        if (reply == PartyReply::Created) party.allowed = true;
        else if (reply == PartyReply::Cancelled) party.members.clear();
        else {
            const auto notice = mir3_cui::PartyFailure(reply, value);
            if (notice.id) {
                party.Begin(mir3_cui::PartyAction::None);
                party.notice = notice;
            }
        }
    }

    std::string ReadGameBody()
    {
        return DecodeGameBytes(input_.substr(DEFBLOCKSIZE));
    }

    static std::string DecodeGameBytes(std::string encoded)
    {
        const size_t encodedSize = encoded.size();
        if (!encodedSize || encodedSize % 4 == 1) return {};
        for (char byte : encoded) if (byte < 60 || byte > 123) return {};
        const unsigned padding = static_cast<unsigned>((encodedSize * 6) % 8);
        if ((encoded.back() - 60) & ((1u << padding) - 1)) return {};
        std::string body(encodedSize * 3 / 4, '\0');
        body.resize(fnDecode6BitBuf(&encoded[0], &body[0], static_cast<int>(body.size())));
        return body;
    }

    void ApplySceneKey(const std::string& challenge)
    {
        const auto decodeWord = [&](size_t start) {
            unsigned bits = 0;
            for (size_t group = 0; group < 2; ++group) {
                const size_t at = start + group * 10;
                const unsigned threshold = static_cast<unsigned char>(challenge[at]) +
                    static_cast<unsigned char>(challenge[at + 9]);
                for (size_t bit = 0; bit < 8; ++bit) {
                    if (static_cast<unsigned char>(challenge[at + bit + 1]) >= threshold)
                        bits |= 0x8000u >> (group * 8 + bit);
                }
            }
            return ((bits & 255) ^ 0x87u) << 8 | ((bits >> 8) ^ 0x87u);
        };
        const unsigned identity = decodeWord(0);
        const unsigned message = decodeWord(40);
        std::random_device random;
        const auto pick = [&](int high) { return std::uniform_int_distribution<int>(0, high)(random); };
        std::string response(40, '\0');
        for (size_t group = 0; group < 4; ++group) {
            const auto value = ((localActor_ >> (group * 8)) & 255u) ^ 0x87u;
            const int base = 10 + pick(49);
            const int threshold = 10 + pick(19);
            const auto at = group * 10;
            response[at] = static_cast<char>(base);
            response[at + 9] = static_cast<char>(threshold);
            for (size_t bit = 0; bit < 8; ++bit)
                response[at + bit + 1] = static_cast<char>(base + pick(threshold - 1) +
                    ((value & (0x80u >> bit)) ? threshold : 0));
        }
        Queue(GameDecodeKeyResponse, 0, response);
        sceneKey_ = {static_cast<unsigned char>(message), static_cast<unsigned char>(message >> 8),
            static_cast<unsigned char>(identity), static_cast<unsigned char>(identity >> 8)};
    }

    void HandleFrame()
    {
        if (IsGameConnection()) {
            HandleGameFrame();
            return;
        }
        _TDEFAULTMESSAGE header = {};
        if (input_.size() < DEFBLOCKSIZE || fnDecodeMessage(&header, &input_[0]) != sizeof header) {
            Reset("The login gateway sent an invalid reply.");
            return;
        }
        const bool characterFrame = stage == Stage::Characters;
        if (!characterFrame && header.wIdent == SM_CERTIFICATION_FAIL) {
            ShowLoginNotice(119, "The login gateway rejected authentication.");
            return;
        }
        if (!characterFrame && header.wIdent == SM_ID_NOTFOUND) {
            ShowLoginNotice(112, "The account was not found.");
            return;
        }
        if (!characterFrame && header.wIdent == SM_PASSWD_FAIL) {
            switch (header.nRecog) {
            case -7: break;
            case -6: ShowLoginNotice(112, "The account was not found."); break;
            case -5: ShowLoginNotice(117, "The account is unavailable."); break;
            case -2: ShowLoginNotice(114, "The account is already signed in."); break;
            case -1: ShowLoginNotice(113, "The password is incorrect."); break;
            default: ShowLoginNotice(116, "The login gateway rejected sign-in."); break;
            }
            return;
        }
        if (!characterFrame && header.wIdent == SM_OUTOFCONNECTION) {
            ShowLoginNotice(121, "The login gateway ended the session.");
            return;
        }
        if (!characterFrame && header.wIdent != SM_PASSOK_SELECTSERVER && header.wIdent != SM_SELECTSERVER_OK)
            return;

        const size_t size = input_.size() - DEFBLOCKSIZE;
        std::string body(size, '\0');
        if (size) {
            const int decoded = fnDecode6BitBuf(&input_[DEFBLOCKSIZE], &body[0], static_cast<int>(size));
            body.resize(decoded);
        }
        if (size % 4 == 1 || body.find('\0') != std::string::npos) {
            Reset("The login gateway sent invalid selection data.");
            return;
        }
        if (characterFrame) {
            HandleCharacterFrame(header, body);
        } else if (header.wIdent == SM_PASSOK_SELECTSERVER && stage == Stage::SigningIn) {
            if (!ReadServers(body)) {
                Reset("The login gateway sent an invalid server list.");
                return;
            }
            billing = legacy ? BillingNotice{} : ReadBilling(header);
            stage = Stage::Servers;
            status = servers.empty() ? "The gateway returned no selectable servers." : "";
        } else if (header.wIdent == SM_SELECTSERVER_OK &&
            (stage == Stage::Selecting || stage == Stage::Servers)) {
            selectionGateway = ReadGateway(body, true);
            if (!selectionGateway.port) {
                Reset("The gateway returned an invalid selection endpoint.");
                return;
            }
            status = selectionGateway.host + ':' + std::to_string(selectionGateway.port);
            selectionSession = header.nRecog;
            Close();
            stage = legacy ? Stage::Complete : Stage::Handoff;
            deadline_ = GetTickCount64() + 2000;
        }
    }

    bool ReadServers(const std::string& body)
    {
        servers = legacy ? std::vector<Server>{} : configuredServers;
        size_t start = 0;
        const char separator = legacy ? ',' : ';';
        while (start < body.size()) {
            const size_t end = body.find(separator, start);
            if (end == std::string::npos) break;
            auto name = body.substr(start, end - start);
            start = end + 1;
            WORD index = 0;
            unsigned load = 0;
            if (legacy) {
                const size_t nameEnd = body.find(',', start);
                if (nameEnd == std::string::npos) break;
                const unsigned parsed = Number(name, 65535);
                if (!parsed && name != "0") break;
                index = static_cast<WORD>(parsed);
                name = body.substr(start, nameEnd - start);
                start = nameEnd + 1;
            } else {
                const size_t open = name.rfind('(');
                if (open == std::string::npos || name.back() != ')') return false;
                const auto value = name.substr(open + 1, name.size() - open - 2);
                char* end = nullptr;
                errno = 0;
                const long parsed = std::strtol(value.c_str(), &end, 10);
                if (value.empty() || (value.front() != '-' && (value.front() < '0' || value.front() > '9')) ||
                    *end || errno == ERANGE)
                    return false;
                if (parsed >= 1 && parsed <= 3) load = static_cast<unsigned>(parsed);
                name.resize(open);
                if (!configuredServers.empty()) {
                    for (auto& server : servers) {
                        if (server.name == name) server.load = load;
                    }
                    continue;
                }
            }
            const std::string label = ConvertText(name, 936, CP_UTF8);
            if (!label.empty() && servers.size() < 256) servers.push_back({name, label, index, load});
        }
        return true;
    }
};

} // namespace mir3_client
