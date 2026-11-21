#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include "cui.h"
#include "CuiAudio.h"
#include "CuiChat.h"
#include "CuiGuild.h"
#include "CuiParty.h"
#include "CuiQuests.h"
#include "CuiServerOptions.h"
#include "Common/EnDecode.h"
#include "Common/Protocol.h"

#include <cerrno>
#include <array>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <map>
#include <random>
#include <string>
#include <vector>

namespace {

enum class Stage {
    Groups, Login, Connecting, SigningIn, Servers, Selecting, Handoff,
    ConnectingCharacters, Characters, Complete, ConnectingGame, AuthenticatingGame,
    GameSession, SigningOutGame, GameDisconnected
};
enum class CharacterDialog { None, Create, Delete };
constexpr wchar_t LoginMusic[] = L"CuiAudio/Login.wav";
constexpr wchar_t SessionMusic[] = L"CuiAudio/Session.wav";
constexpr wchar_t ClickSound[] = L"CuiAudio/Click.wav";
constexpr WORD GameLogon = 50;
constexpr WORD GameLogout = 1009;
constexpr WORD GameDecodeKey = 1330;
constexpr WORD GameDecodeKeyResponse = 1150;

struct Gateway {
    std::string host;
    unsigned port = 0;
};

struct Character {
    std::string name;
    std::string label;
    unsigned profession = 0;
    unsigned level = 0;
    unsigned gender = 0;
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

BillingNotice ReadBilling(const _TDEFAULTMESSAGE& header)
{
    if (header.wTag) return header.wTag == 1 ? BillingNotice{102} : BillingNotice{103, header.wTag - 1};
    if (header.wSeries) return header.wSeries == 1 ? BillingNotice{105} : BillingNotice{104, header.wSeries - 1};
    if (header.wParam) return header.wParam <= 100 ? BillingNotice{106, header.wParam} : BillingNotice{};
    if (header.nRecog > 0) return {107, header.nRecog};
    return {};
}

unsigned Number(const std::string& text, unsigned maximum, unsigned invalid = 0)
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

std::string ConvertText(const std::string& text, UINT source, UINT destination)
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

Gateway ReadGateway(const std::string& body, bool requireTerminator)
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

bool ReadCharacters(int count, const std::string& body, std::vector<Character>& result)
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

bool ValidCharacterName(const std::string& name)
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

std::string TrimIni(const std::string& value)
{
    const auto first = value.find_first_not_of(" \t\r");
    if (first == std::string::npos) return {};
    return value.substr(first, value.find_last_not_of(" \t\r") - first + 1);
}

std::string IniKey(std::string value)
{
    for (char& ch : value) {
        if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
    }
    return value;
}

std::vector<std::string> CommaFields(const std::string& value)
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
};

struct ServerGroup {
    std::string label;
    std::string title;
    std::vector<GroupLine> lines;
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
            ServerGroup group{ConvertText(parts[0], 936, CP_UTF8), ConvertText(parts[1], 936, CP_UTF8)};
            for (unsigned j = 1; j <= rows; ++j) {
                const auto row = CommaFields(next.Get(groupSection, "Server" + std::to_string(j)));
                if (row.size() != 3 || row[0].size() > 31 || row[1].size() > 31 || row[2].size() > 31)
                    return false;
                if (row[0] != "empty") group.lines.push_back({ConvertText(row[0], 936, CP_UTF8), row[1]});
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

void ReadSettings(char* host, DWORD hostSize, char* port, DWORD portSize, bool& legacy,
    char* registration, DWORD registrationSize, char* recovery, DWORD recoverySize,
    std::vector<Server>& configuredServers)
{
    wchar_t path[MAX_PATH];
    DWORD length = GetFullPathNameW(L"Mir3.ini", MAX_PATH, path, nullptr);
    if (!length || length >= MAX_PATH) return;
    if (GetFileAttributesW(path) == INVALID_FILE_ATTRIBUTES) {
        length = GetFullPathNameW(L"Mir2Ei.ini", MAX_PATH, path, nullptr);
        if (!length || length >= MAX_PATH) return;
        legacy = GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES;
    }
    const auto read = [&](const wchar_t* section, const wchar_t* key, char* value, DWORD capacity) {
        wchar_t setting[512];
        const DWORD count = GetPrivateProfileStringW(section, key, L"", setting, 512, path);
        if (count && count < 511 &&
            WideCharToMultiByte(CP_UTF8, 0, setting, -1, nullptr, 0, nullptr, nullptr) <= static_cast<int>(capacity))
            WideCharToMultiByte(CP_UTF8, 0, setting, -1, value, capacity, nullptr, nullptr);
    };
    read(L"Initial", L"ServerAddr", host, hostSize);
    read(L"Initial", L"Param1", port, portSize);
    read(L"Initial", L"Param2", registration, registrationSize);
    read(L"Initial", L"Param3", recovery, recoverySize);
    const UINT count = GetPrivateProfileIntW(L"Server", L"ServerCount", 0, path);
    for (UINT i = 1; i <= count && i <= 256; ++i) {
        const std::wstring key = L"server" + std::to_wstring(i);
        char name[512] = {};
        char caption[512] = {};
        read(L"Server", (key + L"name").c_str(), name, sizeof name);
        read(L"Server", (key + L"caption").c_str(), caption, sizeof caption);
        const auto wireName = ConvertText(name, CP_UTF8, 936);
        if (!wireName.empty()) configuredServers.push_back({wireName, caption[0] ? caption : name});
    }
}

void LoadChineseFont(cui_ctx* ui)
{
    char windows[MAX_PATH];
    const UINT size = GetWindowsDirectoryA(windows, MAX_PATH);
    if (!size || size >= MAX_PATH) return;

    for (const char* filename : {"msyh.ttc", "simsun.ttc"}) {
        const std::string path = std::string(windows) + "\\Fonts\\" + filename;
        const cui_font font = cui_font_load_file(ui, path.c_str());
        if (font >= 0) {
            cui_font_add_fallback(ui, CUI_FONT_BODY, font);
            cui_font_add_fallback(ui, CUI_FONT_MEDIUM, font);
            return;
        }
    }
}

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
    Stage stage = Stage::Login;
    ServerGroups serverGroups;
    std::vector<Server> configuredServers;
    std::vector<Server> servers;
    BillingNotice billing;
    int selectionSession = 0;
    Gateway selectionGateway;
    Gateway gameGateway;
    std::string gameCharacter;
    bool showSignOut = false;
    mir3_cui::ChatPanel chat;
    mir3_cui::ServerOptions serverOptions;
    mir3_cui::PartyPanel party;
    mir3_cui::GuildPanel guild;
    mir3_cui::QuestPanel quests;
    mir3_cui::AudioOptions audioOptions;
    mir3_cui::AudioPlayer audio;
    std::wstring audioOptionsPath;
    std::string audioSaveError;
    bool audioOpen = false;
    bool audioDirty = false;
    std::string chatHistoryDirectory = "CuiChatHistory";
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
    WORD characterRequest = 0;
    std::string status;

    ~LoginClient() { Reset(); }

    void Reset(const char* message = "")
    {
        Close();
        servers.clear();
        billing = {};
        selectionSession = 0;
        selectionGateway = {};
        gameGateway = {};
        gameCharacter.clear();
        showSignOut = false;
        chat = {};
        serverOptions = {};
        party = {};
        guild = {};
        quests = {};
        audioOpen = false;
        characters.clear();
        characterIndex = -1;
        characterPage = 0;
        characterRequest = 0;
        characterNotice.clear();
        CloseCharacterDialog();
        signedInAccount_.clear();
        gameRole_.clear();
        realm_.clear();
        stage = Stage::Login;
        status = message;
        SecureZeroMemory(password, sizeof password);
    }

    void DismissNotice()
    {
        status.clear();
        SecureZeroMemory(password, sizeof password);
        ++noticeDismissals;
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

    void OpenCharacterDialog(CharacterDialog dialog)
    {
        if (dialog == CharacterDialog::Delete && characterIndex < 0) {
            characterNotice = "Select a character to delete.";
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
                characterNotice = "Use letters, digits, or Chinese characters, up to 13 bytes.";
                return;
            }
            RequestCharacter(CM_NEWCHR, signedInAccount_ + '/' + text + "/1/" +
                std::to_string(profession) + '/' + std::to_string(gender));
        } else if (characterDialog == CharacterDialog::Delete && characterIndex >= 0) {
            if ((characterInput[0] && text.empty()) || text.size() > 90 || text.find('/') != std::string::npos) {
                characterNotice = "Enter confirmation text up to 90 bytes, without slashes.";
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
            characterNotice = "Select a character to continue.";
            return;
        }
        RequestCharacter(CM_SELCHR, signedInAccount_ + '/' + characters[characterIndex].name);
    }

    void SignOutGame()
    {
        if (stage != Stage::GameSession) return;
        showSignOut = false;
        Queue(GameLogout, 0, "");
        Wait(Stage::SigningOutGame, "Signing out...");
    }

    void SendChat()
    {
        if (stage != Stage::GameSession || showSignOut || chat.receptionOpen || party.open || party.notice.id ||
            guild.open || guild.requested || guild.notice || guild.action != mir3_cui::GuildAction::None || quests.open ||
            serverOptions.open || !chat.draft[0]) return;
        const std::string draft = chat.draft;
        const bool command = draft.front() == '/' || draft.front() == '@' ||
            draft.front() == '#' || draft.front() == '!';
        static const char* const prefixes[] = {"", "!!", "!~"};
        const std::string text = command ? draft : prefixes[chat.channel] + draft;
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
            chat.Append(103, draft);
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

    void ExpandQuest(mir3_cui::QuestEntry& entry)
    {
        const int id = entry.record.id;
        quests.expanded = quests.expanded == id ? -1 : id;
        if (quests.expanded < 0) return;
        SelectQuest(id);
        if (!entry.childrenLoaded && id <= 65535 && QuestCanQueue())
            Queue(mir3_cui::QuestChildrenQuery, 0, "", 0, static_cast<WORD>(id));
    }

    void RequestGuildPage(int page)
    {
        if (stage != Stage::GameSession || !GuildCanQueue()) return;
        guild.tab = page;
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
        if (action == mir3_cui::GuildAction::Ranks && rows.empty()) {
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

        if (stage == Stage::SigningOutGame && output_.empty()) {
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

    std::string ChatHistoryPath() const
    {
        static const char hex[] = "0123456789abcdef";
        std::string file = chatHistoryDirectory + '/' + host + '-' + port + '-';
        for (const auto* identity : {&realm_, &gameRole_}) {
            for (unsigned char ch : *identity) {
                file += hex[ch >> 4];
                file += hex[ch & 15];
            }
            file += '-';
        }
        return file + ".txt";
    }

    void LoadChatHistory()
    {
        std::ifstream file(ChatHistoryPath(), std::ios::binary);
        std::string line;
        while (std::getline(file, line)) {
            if (line.size() < 2 || line.size() > sizeof chat.draft || line.find('\0') != std::string::npos)
                continue;
            const auto value = line.substr(1);
            if (line.front() == 'P') chat.Remember(chat.phrases, value);
            if (line.front() == 'C') chat.Remember(chat.contacts, value);
        }
    }

    void SaveChatHistory()
    {
        const auto path = ChatHistoryPath();
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

    bool IsGameConnection() const
    {
        return stage == Stage::ConnectingGame || stage == Stage::AuthenticatingGame ||
            stage == Stage::GameSession || stage == Stage::SigningOutGame;
    }

    void EndGameSession(const char* message)
    {
        Close();
        stage = Stage::GameDisconnected;
        status = message;
        showSignOut = false;
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
                characterNotice = "The character gateway sent an invalid character list.";
                break;
            }
            characterPage = 0;
            characterIndex = characters.empty() ? -1 : 0;
            if (characters.empty() && characterNotice.empty()) characterNotice = "This account has no characters.";
            break;
        case SM_QUERYCHR_FAIL:
            if (characterRequest != CM_QUERYCHR) break;
            characterRequest = 0;
            characterNotice = "Could not load the character list.";
            break;
        case SM_NEWCHR_SUCCESS:
            if (characterRequest != CM_NEWCHR) break;
            CloseCharacterDialog();
            QueryCharacters();
            break;
        case SM_NEWCHR_FAIL:
            if (characterRequest != CM_NEWCHR) break;
            characterRequest = 0;
            characterNotice = header.nRecog == 2 ? "That character name already exists." :
                header.nRecog == 3 ? "This account has no free character slot." : "Could not create the character.";
            break;
        case SM_DELCHR_FAIL:
            characterNotice = "The character gateway declined to delete the character.";
            break;
        case SM_STARTPLAY:
            if (characterRequest != CM_SELCHR) break;
            characterRequest = 0;
            gameGateway = ReadGateway(body, false);
            if (!gameGateway.port) {
                characterNotice = "The character gateway returned an invalid game address.";
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
            characterNotice = "The character could not enter the game.";
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

    void Queue(WORD opcode, WORD parameter, const std::string& body, int recognition = 0, WORD series = 0)
    {
        _TDEFAULTMESSAGE header;
        static_assert(sizeof(header) == 12, "Login header wire size");
        fnMakeDefMessage(&header, opcode, recognition, parameter, 0, series);
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
        if (input_.size() < DEFBLOCKSIZE) return;
        for (char byte : input_) {
            if (byte < 60 || byte > 123) return;
        }
        std::array<unsigned char, 12> header = {};
        if (fnDecode6BitBuf(&input_[0], reinterpret_cast<char*>(header.data()), static_cast<int>(header.size())) != 12)
            return;
        const unsigned opcode = (header[0] ^ sceneKey_[0]) |
            (static_cast<unsigned>(header[4] ^ sceneKey_[2] ^ sceneKey_[1]) << 8);
        const std::uint32_t actor = (header[5] ^ sceneKey_[3]) | (std::uint32_t(header[7]) << 8) |
            (std::uint32_t(header[9]) << 16) | (std::uint32_t(header[11]) << 24);
        if (opcode == GameLogon && actor && actor <= 0x7fffffffu &&
            stage == Stage::AuthenticatingGame && input_.size() == DEFBLOCKSIZE + 51) {
            stage = Stage::GameSession;
            localActor_ = actor;
            LoadChatHistory();
            status = "Signed in to the game gateway.";
        } else if (opcode == SM_CERTIFICATION_FAIL || opcode == SM_OUTOFCONNECTION) {
            EndGameSession(opcode == SM_CERTIFICATION_FAIL ? "The game gateway rejected sign-in."
                                                         : "The game gateway ended the session.");
        } else if (opcode == GameDecodeKey && stage == Stage::GameSession) {
            const auto body = ReadGameBody();
            if (body.size() == 60 && body.find('\0') == std::string::npos) ApplySceneKey(body);
        } else if (opcode == mir3_cui::ClientOptions && stage == Stage::GameSession) {
            if (input_.size() == DEFBLOCKSIZE) {
                const auto mode = static_cast<std::uint16_t>(header[1] | (header[6] << 8));
                const auto changed = serverOptions.Apply(actor, mode);
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
            const auto first = static_cast<WORD>(header[1] | (header[6] << 8));
            const auto third = static_cast<WORD>(header[3] | (header[10] << 8));
            HandleQuestReply(static_cast<mir3_cui::QuestReply>(opcode), first, third);
        } else if (mir3_cui::ChatChannel(opcode) && stage == Stage::GameSession &&
                   input_.size() <= DEFBLOCKSIZE + 1024) {
            auto body = ReadGameBody();
            const auto terminator = body.find('\0');
            if (terminator != std::string::npos) body.resize(terminator);
            if ((opcode == 40 || opcode == 1501) && body.size() > 259) return;
            const auto text = ConvertText(body, 936, CP_UTF8);
            if (text.empty()) return;
            chat.Append(opcode, text);
            if (chat.logging) {
                std::ofstream log("Chat.txt", std::ios::binary | std::ios::app);
                log << body << "\r\n";
            }
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
            if (quests.parent == third && quests.child == first) quests.resetDescription = true;
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
                entry->children.push_back(std::move(record));
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
                        guild.history.clear();
                        guild.Begin(mir3_cui::GuildAction::None);
                    } else if (!guild.overview.manager) {
                        guild.Begin(mir3_cui::GuildAction::None);
                    }
                    guild.open = true;
                    guild.requested = false;
                    guild.tab = 0;
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
            if (!guild.overview.name.empty()) {
                if (guild.history.size() == 79) guild.history.erase(guild.history.begin());
                guild.history.emplace_back(mir3_cui::GuildNoticeText(notice));
            }
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
        const size_t encodedSize = input_.size() - DEFBLOCKSIZE;
        if (!encodedSize || encodedSize % 4 == 1) return {};
        const unsigned padding = static_cast<unsigned>((encodedSize * 6) % 8);
        if ((input_.back() - 60) & ((1u << padding) - 1)) return {};
        std::string body(encodedSize * 3 / 4, '\0');
        body.resize(fnDecode6BitBuf(&input_[DEFBLOCKSIZE], &body[0], static_cast<int>(body.size())));
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
            Reset("The login gateway rejected authentication.");
            return;
        }
        if (!characterFrame && header.wIdent == SM_ID_NOTFOUND) {
            Reset("The account was not found.");
            return;
        }
        if (!characterFrame && header.wIdent == SM_PASSWD_FAIL) {
            switch (header.nRecog) {
            case -7: break;
            case -6: Reset("The account was not found."); break;
            case -5: Reset("The account is unavailable."); break;
            case -2: Reset("The account is already signed in."); break;
            case -1: Reset("The password is incorrect."); break;
            default: Reset("The login gateway rejected sign-in."); break;
            }
            return;
        }
        if (!characterFrame && header.wIdent == SM_OUTOFCONNECTION) {
            Reset("The login gateway ended the session.");
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

int Input(cui_ctx* ui, const char* label, char* value, size_t capacity, bool password = false,
    bool autofocus = false)
{
    cui_input_opts options = {};
    options.label = label;
    options.password = password;
    options.autofocus = autofocus;
    options.width = CUI_FILL;
    return cui_input_ex(ui, label, value, capacity, &options);
}

bool Button(cui_ctx* ui, const char* label, bool primary = false, bool fullWidth = false)
{
    cui_button_opts options = {};
    options.variant = primary ? CUI_BUTTON_PRIMARY : CUI_BUTTON_SECONDARY;
    if (fullWidth) options.width = CUI_FILL;
    return cui_button_ex(ui, label, &options);
}

std::string BillingText(const BillingNotice& notice)
{
    const auto amount = std::to_string(notice.argument);
    switch (notice.message) {
    case 102: return "The gateway reports a billing notice for this account.";
    case 103: return "The gateway reports a billing balance of " + amount + " for this account.";
    case 105: return "The gateway reports a subscription notice for this account.";
    case 104: return "The gateway reports a subscription balance of " + amount + " for this account.";
    case 106: return "The gateway reports a remaining balance of " + amount + ".";
    case 107: return "The gateway reports an account balance of " + amount + ".";
    default: return {};
    }
}

bool IsAccountUrl(const std::string& url)
{
    return url.compare(0, 8, "https://") == 0 || url.compare(0, 7, "http://") == 0;
}

void DrawServerGroups(cui_ctx* ui, LoginClient& client)
{
    auto& model = client.serverGroups;
    cui_box toolbar = {};
    toolbar.width = CUI_FILL;
    toolbar.align = CUI_ALIGN_CENTER;
    toolbar.justify = CUI_JUSTIFY_BETWEEN;
    cui_row_begin(ui, &toolbar);
    cui_heading(ui, "Server groups", 1);
    if (Button(ui, "Exit")) cui_app_quit(nullptr);
    cui_box_end(ui);

    cui_box panes = {};
    panes.width = CUI_FILL;
    panes.height = CUI_FILL;
    panes.gap = 18;
    cui_row_begin(ui, &panes);

    const ServerGroup* group = model.selected < 0 ? nullptr : &model.groups[model.selected];
    cui_box lines = {};
    lines.width = CUI_PCT(44);
    lines.height = CUI_FILL;
    lines.gap = 10;
    cui_column_begin(ui, &lines);
    const std::string title = group && !group->title.empty() ? group->title : "Server lines";
    cui_heading(ui, title.c_str(), 2);
    cui_scroll_opts list = {};
    list.id = "group-lines";
    list.width = CUI_FILL;
    list.height = CUI_FILL;
    list.gap = 6;
    cui_scroll_begin(ui, &list);
    if (!group) {
        cui_secondary(ui, "Choose a server group.");
    } else if (group->lines.empty()) {
        cui_secondary(ui, "This group has no server lines.");
    } else {
        for (size_t i = 0; i < group->lines.size(); ++i) {
            const auto& line = group->lines[i];
            cui_push_id_int(ui, static_cast<int>(i));
            cui_push_disabled(ui, line.loginSection.empty() || IniKey(line.loginSection) == "empty");
            const auto label = line.caption + "###line";
            if (Button(ui, label.c_str(), false, true) && client.OpenGroupLine(line))
                cui_request_redraw(ui);
            cui_pop_disabled(ui);
            cui_pop_id(ui);
        }
    }
    cui_scroll_end(ui);
    if (!client.status.empty()) cui_secondary(ui, client.status.c_str());
    cui_box_end(ui);

    list.id = "groups";
    cui_scroll_begin(ui, &list);
    bool groupChanged = false;
    for (size_t i = 0; i < model.groups.size(); ++i) {
        cui_push_id_int(ui, static_cast<int>(i));
        const auto& entry = model.groups[i];
        const auto label = (entry.label.empty() ? entry.title : entry.label) + "###group";
        if (Button(ui, label.c_str(), model.selected == static_cast<int>(i), true)) {
            model.selected = static_cast<int>(i);
            client.status.clear();
            groupChanged = true;
        }
        cui_pop_id(ui);
    }
    cui_scroll_end(ui);
    if (groupChanged) {
        cui_scroll_to(ui, "group-lines", 0);
        cui_request_redraw(ui);
    }
    cui_box_end(ui);
}

void DrawAccount(cui_ctx* ui, LoginClient& client)
{
    cui_box row = {};
    row.width = CUI_FILL;
    row.gap = 12;
    row.wrap = true;
    row.align = CUI_ALIGN_END;
    cui_row_begin(ui, &row);
    if (IsAccountUrl(client.registration) && Button(ui, "Create account"))
        cui_app_open_url(client.registration);
    if (IsAccountUrl(client.recovery) && Button(ui, "Change password"))
        cui_app_open_url(client.recovery);
    if (Button(ui, "Gateway settings")) client.settingsOpen = true;
    if (Button(ui, "Audio")) client.audioOpen = true;
    if (!client.serverGroups.groups.empty() && Button(ui, "Server groups")) {
        client.Reset();
        client.settingsOpen = false;
        client.stage = Stage::Groups;
    }
    cui_box_end(ui);
    cui_separator(ui);

    cui_row_begin(ui, &row);
    cui_box field = {};
    field.width = CUI_FILL;
    field.min_width = 130;
    cui_column_begin(ui, &field);
    Input(ui, "Account", client.account, sizeof client.account, false, client.noticeDismissals == 0);
    cui_box_end(ui);
    cui_column_begin(ui, &field);
    cui_push_id_int(ui, static_cast<int>(client.noticeDismissals));
    const int result = Input(ui, "Password", client.password, sizeof client.password, true,
        client.noticeDismissals != 0);
    cui_pop_id(ui);
    cui_box_end(ui);
    const bool submit = Button(ui, "Sign in", true);
    cui_box_end(ui);
    if (client.stage == Stage::Login && client.status.empty() && !client.settingsOpen && !client.audioOpen &&
        (submit || (result & CUI_SUBMITTED)))
        client.SignIn();
}

const char* const professions[] = {"Warrior", "Wizard", "Taoist"};
const char* const genders[] = {"Male", "Female"};

void DrawCharacters(cui_ctx* ui, LoginClient& client)
{
    cui_heading(ui, "Choose a character", 1);
    const bool blocked = client.characterRequest || !client.characterNotice.empty() ||
        client.characterDialog != CharacterDialog::None;
    cui_push_disabled(ui, blocked);

    cui_box row = {};
    row.width = CUI_FILL;
    row.gap = 12;
    cui_row_begin(ui, &row);
    const int first = client.characterPage * 2;
    for (int i = first; i < first + 2 && i < static_cast<int>(client.characters.size()); ++i) {
        const auto& role = client.characters[i];
        cui_push_id_int(ui, i);
        cui_card_opts card = {};
        card.width = CUI_FILL;
        card.pad = 16;
        card.gap = 8;
        card.selected = i == client.characterIndex;
        cui_card_begin(ui, &card);
        if (Button(ui, (role.label + "###role").c_str(), card.selected, true))
            client.characterIndex = i;
        cui_textf(ui, "%s / %s", professions[role.profession], genders[role.gender]);
        cui_textf(ui, "Level %u", role.level);
        cui_card_end(ui);
        cui_pop_id(ui);
    }
    cui_box_end(ui);

    if (client.characters.size() > 2) {
        cui_row_begin(ui, &row);
        if (Button(ui, "Previous page") && client.characterPage == 1) {
            client.characterPage = 0;
            client.characterIndex = 0;
        }
        cui_spacer(ui);
        if (Button(ui, "Next page") && client.characterPage == 0) {
            client.characterPage = 1;
            client.characterIndex = 2;
        }
        cui_box_end(ui);
    }
    cui_spacer(ui);
    cui_row_begin(ui, &row);
    if (Button(ui, "Create")) client.OpenCharacterDialog(CharacterDialog::Create);
    if (Button(ui, "Delete")) client.OpenCharacterDialog(CharacterDialog::Delete);
    if (Button(ui, "Start", true)) client.StartCharacter();
    if (Button(ui, "Back to sign-in")) client.Reset();
    cui_box_end(ui);
    cui_pop_disabled(ui);
    if (client.characterRequest) cui_secondary(ui, "Waiting for the character gateway...");
}

void DrawCharacterDialog(cui_ctx* ui, LoginClient& client, cui_dialog_opts modal)
{
    bool open = true;
    if (!client.characterNotice.empty()) {
        modal.description = client.characterNotice.c_str();
        if (cui_dialog_begin(ui, "Character notice", &open, &modal)) {
            cui_dialog_footer_begin(ui);
            const bool dismiss = Button(ui, "OK", true) || cui_key_pressed(ui, CUI_KEY_ENTER) ||
                cui_key_pressed(ui, CUI_KEY_SPACE);
            cui_dialog_footer_end(ui);
            cui_dialog_end(ui);
            if (dismiss) {
                client.characterNotice.clear();
                if (client.characterDialog == CharacterDialog::Create) {
                    SecureZeroMemory(client.characterInput, sizeof client.characterInput);
                    ++client.characterEditor;
                }
            }
        }
        return;
    }
    if (client.characterDialog == CharacterDialog::None) return;
    const bool creating = client.characterDialog == CharacterDialog::Create;
    const std::string prompt = creating ? "" : "Delete " + client.characters[client.characterIndex].label + "?";
    modal.description = prompt.c_str();
    if (!cui_dialog_begin(ui, creating ? "Create character" : "Delete character", &open, &modal)) return;
    cui_push_disabled(ui, client.characterRequest != 0);
    if (creating) {
        cui_label(ui, "Profession");
        cui_segmented_ex(ui, "profession", professions, 3, &client.profession, nullptr);
        cui_label(ui, "Appearance");
        cui_segmented_ex(ui, "appearance", genders, 2, &client.gender, nullptr);
    }
    cui_push_id_int(ui, client.characterEditor);
    const int input = Input(ui, creating ? "Name" : "Confirmation text", client.characterInput,
        sizeof client.characterInput, false, true);
    cui_pop_id(ui);
    if (!creating) {
        const bool tabDown = cui_key_down(ui, CUI_KEY_TAB);
        if (tabDown && !client.deleteTabDown) client.deleteCancelSelected = !client.deleteCancelSelected;
        client.deleteTabDown = tabDown;
    }
    const bool keyActivate = (input & CUI_SUBMITTED) || cui_key_pressed(ui, CUI_KEY_ENTER) ||
        (!creating && cui_key_pressed(ui, CUI_KEY_SPACE));
    cui_dialog_footer_begin(ui);
    const bool cancel = Button(ui, "Cancel", !creating && client.deleteCancelSelected);
    const bool confirm = Button(ui, creating ? "Create" : "Delete", creating || !client.deleteCancelSelected);
    cui_dialog_footer_end(ui);
    cui_pop_disabled(ui);
    cui_dialog_end(ui);
    if (client.characterRequest) return;
    if (!creating && keyActivate) {
        if (client.deleteCancelSelected) client.CloseCharacterDialog();
        else client.SubmitCharacterDialog();
    } else if (cancel) {
        client.CloseCharacterDialog();
    } else if (confirm || keyActivate) {
        client.SubmitCharacterDialog();
    }
}

void ChatShortcuts(cui_ctx* ui, mir3_cui::ChatPanel& chat, bool phrase)
{
    const auto& history = phrase ? chat.phrases : chat.contacts;
    if (history.empty()) return;
    auto& selected = phrase ? chat.phraseIndex : chat.contactIndex;
    const int last = static_cast<int>(history.size()) - 1;
    int target = -1;
    if (cui_key_pressed(ui, CUI_KEY_UP)) target = (std::max)(0, selected - 1);
    if (cui_key_pressed(ui, CUI_KEY_DOWN)) target = (std::min)(last, selected + 1);
    for (int i = 0; i <= last; ++i) {
        const auto digit = static_cast<cui_key>(CUI_KEY_0 + (i + 1) % 10);
        if (cui_key_pressed(ui, digit)) target = i;
    }
    cui_label(ui, phrase ? "Quick phrases" : "Recent contacts");
    for (int i = 0; i <= last; ++i) {
        cui_push_id_int(ui, i);
        const auto label = std::to_string((i + 1) % 10) + "  " + history[i] + "###shortcut";
        if (Button(ui, label.c_str(), i == selected)) target = i;
        cui_pop_id(ui);
    }
    if (target >= 0) {
        selected = target;
        chat.FillDraft(history[target]);
    }
}

void DrawGameChat(cui_ctx* ui, LoginClient& client)
{
    auto& chat = client.chat;
    const auto hasOverlay = [&]() {
        return client.showSignOut || chat.receptionOpen || client.serverOptions.open || client.audioOpen ||
            client.party.open || client.party.notice.id || client.guild.open || client.guild.requested ||
            client.guild.notice || client.guild.action != mir3_cui::GuildAction::None || client.quests.open;
    };
    const bool hadOverlay = hasOverlay();
    cui_push_disabled(ui, hadOverlay);
    cui_box row = {};
    row.width = CUI_FILL;
    row.gap = 10;
    row.align = CUI_ALIGN_CENTER;
    cui_box toolbar = row;
    toolbar.wrap = true;
    cui_row_begin(ui, &toolbar);
    cui_heading(ui, "Chat", 1);
    cui_spacer(ui);
    if (Button(ui, chat.expanded ? "Compact" : "Expand")) chat.expanded = !chat.expanded;
    if (Button(ui, "Reception")) chat.receptionOpen = true;
    if (Button(ui, "Party") || (!chat.composing && !hadOverlay && cui_shortcut(ui, 0, CUI_KEY_G)))
        client.party.open = true;
    if (Button(ui, "Guild") || (!chat.composing && !hadOverlay && cui_shortcut(ui, 0, CUI_KEY_F)))
        client.RequestGuildPage(0);
    if (Button(ui, "Quests") || (!chat.composing && !hadOverlay && cui_shortcut(ui, 0, CUI_KEY_D)))
        client.OpenQuests();
    if (Button(ui, "Server options") ||
        (!chat.composing && !hadOverlay && cui_shortcut(ui, 0, CUI_KEY_N)))
        client.serverOptions.open = true;
    if (Button(ui, "Audio")) client.audioOpen = true;
    if (Button(ui, "Sign out")) client.showSignOut = true;
    cui_box_end(ui);
    const bool overlay = hasOverlay();

    cui_scroll_opts list = {};
    list.id = "chat-history";
    list.width = CUI_FILL;
    list.height = chat.expanded ? CUI_FILL : CUI_PX(174);
    list.gap = 6;
    if (chat.scrollToEnd) {
        cui_scroll_to(ui, list.id, 100000.0f);
        chat.scrollToEnd = false;
    }
    cui_scroll_begin(ui, &list);
    for (const auto& line : chat.lines) {
        const auto text = std::string("[") + mir3_cui::ChatChannel(line.opcode) + "] " + line.text;
        cui_text_ex(ui, text.c_str(), nullptr);
    }
    if (chat.lines.empty()) cui_secondary(ui, "No messages yet.");
    cui_scroll_end(ui);

    const int mods = cui_key_mods(ui);
    if (chat.composing && !overlay && (mods & (CUI_MOD_CTRL | CUI_MOD_ALT))) {
        cui_box shortcuts = {};
        shortcuts.width = CUI_FILL;
        shortcuts.gap = 6;
        shortcuts.wrap = true;
        cui_row_begin(ui, &shortcuts);
        ChatShortcuts(ui, chat, (mods & CUI_MOD_ALT) != 0);
        cui_box_end(ui);
    }

    if (!chat.composing) {
        if (Button(ui, "Write a message") || (!overlay &&
            (cui_key_pressed(ui, CUI_KEY_ENTER) || cui_key_pressed(ui, CUI_KEY_SPACE)))) {
            chat.composing = true;
            ++chat.editor;
        }
    } else {
        const char* const channels[] = {"Nearby", "Party", "Guild"};
        cui_segmented_ex(ui, "chat-channel", channels, 3, &chat.channel, nullptr);
        cui_row_begin(ui, &row);
        cui_box editor = {};
        editor.width = CUI_FILL;
        cui_column_begin(ui, &editor);
        cui_push_id_int(ui, chat.editor);
        const int input = Input(ui, "Message", chat.draft, sizeof chat.draft, false, true);
        cui_pop_id(ui);
        cui_box_end(ui);
        const bool send = Button(ui, "Send", true);
        cui_box_end(ui);
        if (!overlay && cui_key_pressed(ui, CUI_KEY_ESCAPE)) {
            chat.composing = false;
        } else if (!overlay && (send || (input & CUI_SUBMITTED))) {
            client.SendChat();
        }
    }
    if (!chat.error.empty()) cui_secondary(ui, chat.error.c_str());
    cui_pop_disabled(ui);
}

void DrawChatReception(cui_ctx* ui, LoginClient& client)
{
    auto& chat = client.chat;
    if (!cui_dialog_begin(ui, "Chat reception", &chat.receptionOpen, nullptr)) return;
    static const char* const labels[] = {"Block private messages", "Block guild chat", "Block all chat"};
    for (std::size_t i = 0; i < chat.reception.displayed.size(); ++i) {
        bool blocked = chat.reception.displayed[i];
        if (cui_checkbox_ex(ui, labels[i], &blocked, nullptr))
            client.ToggleChatReception(static_cast<mir3_cui::ReceptionChannel>(i));
        if (chat.reception.displayed[i] != chat.reception.confirmed[i])
            cui_muted(ui, "Waiting for server confirmation");
    }
    if (!chat.reception.error.empty()) cui_secondary(ui, chat.reception.error.c_str());
    cui_dialog_footer_begin(ui);
    if (Button(ui, "Done", true)) chat.receptionOpen = false;
    cui_dialog_footer_end(ui);
    cui_dialog_end(ui);
}

void DrawParty(cui_ctx* ui, LoginClient& client)
{
    using mir3_cui::PartyAction;
    auto& party = client.party;
    cui_dialog_opts dialog = {};
    dialog.persistent = true;
    dialog.no_close_button = true;
    bool open = true;

    if (party.notice.id) {
        if (!cui_dialog_begin(ui, "Party notice", &open, &dialog)) return;
        cui_textf(ui, "%s failed (server code %d).", party.notice.action, party.notice.reason);
        cui_dialog_footer_begin(ui);
        const bool dismiss = Button(ui, "OK", true) || cui_key_pressed(ui, CUI_KEY_ENTER) ||
            cui_key_pressed(ui, CUI_KEY_SPACE);
        cui_dialog_footer_end(ui);
        cui_dialog_end(ui);
        if (dismiss) party.notice = {};
        return;
    }

    if (party.action != PartyAction::None) {
        const char* title = party.action == PartyAction::Create ? "Create party" :
            party.action == PartyAction::Invite ? "Invite member" : "Remove member";
        if (!cui_dialog_begin(ui, title, &open, &dialog)) return;
        cui_push_id_int(ui, party.editor);
        const int input = Input(ui, "Character name", party.name, sizeof party.name, false, true);
        cui_pop_id(ui);
        if (input & CUI_FOCUSED) party.editorFocused = true;
        if (input & CUI_BLURRED) party.editorFocused = false;
        const bool tab = cui_key_down(ui, CUI_KEY_TAB);
        if (tab && !party.tabDown) party.cancelSelected = !party.cancelSelected;
        party.tabDown = tab;
        const bool activate = (input & CUI_SUBMITTED) || cui_key_pressed(ui, CUI_KEY_ENTER) ||
            (!party.editorFocused && cui_key_pressed(ui, CUI_KEY_SPACE));
        if (!party.error.empty()) cui_secondary(ui, party.error.c_str());
        cui_dialog_footer_begin(ui);
        const bool cancel = Button(ui, "Cancel", party.cancelSelected);
        const bool confirm = Button(ui, "Confirm", !party.cancelSelected);
        cui_dialog_footer_end(ui);
        cui_dialog_end(ui);
        if (activate) {
            if (party.cancelSelected) party.Begin(PartyAction::None);
            else client.SubmitPartyMember();
        } else if (cancel) {
            party.Begin(PartyAction::None);
        } else if (confirm) {
            client.SubmitPartyMember();
        }
        return;
    }

    if (!cui_dialog_begin(ui, "Party", &party.open, nullptr)) return;
    cui_label(ui, client.gameCharacter.c_str());
    bool allowed = party.allowed;
    if (cui_checkbox_ex(ui, "Allow party invitations", &allowed, nullptr)) client.SetPartyAllowed(allowed);
    if (party.members.empty()) {
        cui_secondary(ui, "No party members.");
    } else {
        cui_scroll_opts list = {};
        list.id = "party-roster";
        list.width = CUI_FILL;
        list.height = CUI_PX(144);
        list.gap = 6;
        cui_scroll_begin(ui, &list);
        for (const auto& member : party.members) cui_text_ex(ui, member.c_str(), nullptr);
        cui_scroll_end(ui);
    }
    if (!party.error.empty()) cui_secondary(ui, party.error.c_str());
    cui_box row = {};
    row.width = CUI_FILL;
    row.gap = 8;
    row.wrap = true;
    cui_row_begin(ui, &row);
    if (Button(ui, "Create party")) party.Begin(PartyAction::Create);
    if (Button(ui, "Invite member")) party.Begin(PartyAction::Invite);
    if (Button(ui, "Remove member")) party.Begin(PartyAction::Remove);
    cui_box_end(ui);
    cui_dialog_footer_begin(ui);
    if (Button(ui, "Done", true) || cui_shortcut(ui, 0, CUI_KEY_G)) party.open = false;
    cui_dialog_footer_end(ui);
    cui_dialog_end(ui);
}

const char* GuildActionTitle(mir3_cui::GuildAction action)
{
    using mir3_cui::GuildAction;
    switch (action) {
    case GuildAction::Add: return "Add member";
    case GuildAction::Remove: return "Remove member";
    case GuildAction::Notice: return "Edit announcement";
    case GuildAction::Ranks: return "Edit ranks";
    case GuildAction::BreakAlliance: return "End alliance";
    default: return "";
    }
}

void DrawGuild(cui_ctx* ui, LoginClient& client)
{
    using mir3_cui::GuildAction;
    auto& guild = client.guild;
    cui_dialog_opts modal = {};
    modal.width = 530;
    bool open = true;

    if (guild.notice) {
        modal.persistent = true;
        modal.no_close_button = true;
        if (!cui_dialog_begin(ui, "Guild notice", &open, &modal)) return;
        cui_text_ex(ui, mir3_cui::GuildNoticeText(guild.notice), nullptr);
        cui_dialog_footer_begin(ui);
        const bool dismiss = Button(ui, "OK", true) || cui_key_pressed(ui, CUI_KEY_ENTER) ||
            cui_key_pressed(ui, CUI_KEY_SPACE);
        cui_dialog_footer_end(ui);
        cui_dialog_end(ui);
        if (dismiss) guild.notice = 0;
        return;
    }

    if (guild.action != GuildAction::None) {
        modal.persistent = true;
        modal.no_close_button = true;
        if (!cui_dialog_begin(ui, GuildActionTitle(guild.action), &open, &modal)) return;
        cui_push_id_int(ui, guild.editor);
        int input = 0;
        if (guild.Multiline()) {
            cui_input_opts options = {};
            options.label = guild.action == GuildAction::Notice ? "Announcement" : "Ranks";
            options.height = 180;
            options.width = CUI_FILL;
            options.autofocus = true;
            cui_textarea_ex(ui, "guild-editor", guild.input, sizeof guild.input, &options);
        } else {
            input = Input(ui, "Name", guild.input, sizeof guild.input, false, true);
        }
        cui_pop_id(ui);
        if (!guild.error.empty()) cui_secondary(ui, guild.error.c_str());
        cui_dialog_footer_begin(ui);
        const bool cancel = Button(ui, "Cancel");
        const bool confirm = Button(ui, "Confirm", true);
        cui_dialog_footer_end(ui);
        cui_dialog_end(ui);
        if (cancel) guild.Begin(GuildAction::None);
        else if (confirm || (input & CUI_SUBMITTED)) client.SubmitGuildAction();
        return;
    }

    if (guild.requested && !guild.open) {
        if (cui_dialog_begin(ui, "Guild", &guild.requested, &modal)) {
            cui_secondary(ui, "Waiting for the guild overview...");
            if (!guild.error.empty()) cui_secondary(ui, guild.error.c_str());
            if (Button(ui, "Cancel")) guild.requested = false;
            cui_dialog_end(ui);
        }
        return;
    }

    if (!cui_dialog_begin(ui, "Guild", &guild.open, &modal)) return;
    const auto name = ConvertText(guild.overview.name, 936, CP_UTF8);
    cui_heading(ui, name.c_str(), 2);
    const char* const tabs[] = {"Overview", "Members", "History"};
    cui_box row = {};
    row.width = CUI_FILL;
    row.gap = 8;
    row.wrap = true;
    cui_row_begin(ui, &row);
    for (int tab = 0; tab < 3; ++tab) {
        if (Button(ui, tabs[tab], guild.tab == tab)) client.RequestGuildPage(tab);
    }
    cui_box_end(ui);
    const auto& rows = guild.tab == 0 ? guild.overview.rows : guild.tab == 1 ? guild.roster.rows : guild.history;
    cui_scroll_opts list = {};
    const char* const ids[] = {"guild-home", "guild-members", "guild-history"};
    list.id = ids[guild.tab];
    list.width = CUI_FILL;
    list.height = CUI_PX(150);
    list.gap = 4;
    cui_scroll_begin(ui, &list);
    if (rows.empty()) cui_secondary(ui, "No entries.");
    for (const auto& row : rows) {
        const auto text = guild.tab == 2 ? row : ConvertText(row, 936, CP_UTF8);
        cui_text_ex(ui, text.c_str(), nullptr);
    }
    cui_scroll_end(ui);
    if (!guild.error.empty()) cui_secondary(ui, guild.error.c_str());
    cui_row_begin(ui, &row);
    for (auto action : {GuildAction::Notice, GuildAction::Ranks, GuildAction::Add, GuildAction::Remove,
                        GuildAction::BreakAlliance}) {
        if (Button(ui, GuildActionTitle(action))) client.BeginGuildAction(action);
    }
    cui_box_end(ui);
    cui_dialog_footer_begin(ui);
    if (Button(ui, "Done", true) || cui_shortcut(ui, 0, CUI_KEY_F)) guild.open = false;
    cui_dialog_footer_end(ui);
    cui_dialog_end(ui);
}

void DrawQuests(cui_ctx* ui, LoginClient& client)
{
    auto& quests = client.quests;
    cui_dialog_opts dialog = {};
    dialog.width = 510;
    if (!cui_dialog_begin(ui, "Quests", &quests.open, &dialog)) return;
    cui_scroll_opts list = {};
    list.id = "quest-list";
    list.width = CUI_FILL;
    list.height = CUI_PX(150);
    list.gap = 6;
    if (quests.resetScroll) {
        cui_scroll_to(ui, list.id, 0);
        quests.resetScroll = false;
    }
    cui_scroll_begin(ui, &list);
    if (quests.entries.empty())
        cui_secondary(ui, quests.waiting ? "Waiting for the quest list..." : "No quests.");

    cui_box row = {};
    row.width = CUI_FILL;
    row.gap = 6;
    for (auto& entry : quests.entries) {
        const auto& quest = entry.record;
        cui_push_id_int(ui, quest.id);
        cui_row_begin(ui, &row);
        const std::string title = quest.title + (quest.completed ? " (Completed)" : "") + "###quest";
        if (Button(ui, title.c_str(), quests.parent == quest.id && quests.child < 0, true))
            client.SelectQuest(quest.id);
        if (quest.childCount || !entry.children.empty()) {
            if (Button(ui, quests.expanded == quest.id ? "Collapse" : "Expand"))
                client.ExpandQuest(entry);
        }
        cui_box_end(ui);
        if (quests.expanded == quest.id) {
            if (entry.children.empty()) cui_muted(ui, entry.childrenLoaded ? "No steps." : "Waiting for quest steps...");
            for (const auto& child : entry.children) {
                cui_push_id_int(ui, child.id);
                const std::string label = "  " + child.title + "###step";
                if (Button(ui, label.c_str(), quests.parent == quest.id && quests.child == child.id, true))
                    client.SelectQuest(quest.id, child.id);
                cui_pop_id(ui);
            }
        }
        cui_pop_id(ui);
    }
    cui_scroll_end(ui);
    cui_separator(ui);
    cui_label(ui, "Description");
    cui_scroll_opts description = {};
    description.id = "quest-description";
    description.width = CUI_FILL;
    description.height = CUI_PX(96);
    if (quests.resetDescription) {
        cui_scroll_to(ui, description.id, 0);
        quests.resetDescription = false;
    }
    cui_scroll_begin(ui, &description);
    const auto* text = quests.Description();
    if (text && !text->empty()) cui_text_ex(ui, text->c_str(), nullptr);
    else cui_secondary(ui, quests.parent < 0 ? "Select a quest." : "No description yet.");
    cui_scroll_end(ui);
    if (!quests.error.empty()) cui_secondary(ui, quests.error.c_str());
    cui_dialog_footer_begin(ui);
    if (Button(ui, "Refresh")) client.RequestQuests();
    if (Button(ui, "Done", true) || cui_shortcut(ui, 0, CUI_KEY_D)) quests.open = false;
    cui_dialog_footer_end(ui);
    cui_dialog_end(ui);
}

void DrawServerOptions(cui_ctx* ui, LoginClient& client)
{
    auto& options = client.serverOptions;
    cui_dialog_opts dialog = {};
    dialog.width = 510;
    if (!cui_dialog_begin(ui, "Server options", &options.open, &dialog)) return;

    const auto drawOption = [&](const mir3_cui::ServerOption& option) {
        bool checked = (options.displayed & option.mask) != 0;
        if (cui_checkbox_ex(ui, option.label, &checked, nullptr))
            client.ToggleServerOption(option);
        if ((options.displayed ^ options.confirmed) & option.mask)
            cui_tooltip(ui, "Waiting for server confirmation");
    };
    cui_box row = {};
    row.width = CUI_FILL;
    row.gap = 14;
    cui_box column = {};
    column.width = CUI_FILL;
    column.gap = 10;
    cui_row_begin(ui, &row);
    cui_column_begin(ui, &column);
    for (std::size_t i = 0; i < 4; ++i) drawOption(mir3_cui::ServerOptionList[i]);
    cui_box_end(ui);
    cui_column_begin(ui, &column);
    for (std::size_t i = 4; i < mir3_cui::ServerOptionCount; ++i) drawOption(mir3_cui::ServerOptionList[i]);
    cui_box_end(ui);
    cui_box_end(ui);

    static const char* const attackModes[] = {"All targets", "Peaceful", "Party", "Guild", "Good and evil"};
    cui_separator(ui);
    if (options.attackMode < sizeof attackModes / sizeof *attackModes)
        cui_textf(ui, "Attack mode: %s", attackModes[options.attackMode]);
    else
        cui_textf(ui, "Attack mode: %u", static_cast<unsigned>(options.attackMode));
    if (Button(ui, "Change attack mode")) client.RequestAttackMode();
    if (options.attackModePending || options.displayed != options.confirmed)
        cui_muted(ui, "Waiting for server confirmation");
    if (!options.error.empty()) cui_secondary(ui, options.error.c_str());
    cui_dialog_footer_begin(ui);
    if (Button(ui, "Done", true)) options.open = false;
    cui_dialog_footer_end(ui);
    cui_dialog_end(ui);
}

void DrawAudioSettings(cui_ctx* ui, LoginClient& client)
{
    cui_dialog_opts dialog = {};
    dialog.persistent = true;
    dialog.no_close_button = true;
    if (!cui_dialog_begin(ui, "Audio settings", &client.audioOpen, &dialog)) return;

    auto& settings = client.audioOptions;
    bool changed = cui_checkbox_ex(ui, "Background music", &settings.music, nullptr);
    cui_slider_opts slider = {};
    slider.step = 1;
    slider.show_value = true;
    slider.format = "%.0f";
    slider.width = CUI_FILL;
    const auto drawLevel = [&](const char* label, int& level) {
        float value = static_cast<float>(level);
        if (cui_slider_ex(ui, label, &value, -100, 0, &slider)) {
            level = static_cast<int>(value);
            changed = true;
        }
    };
    drawLevel("Music volume", settings.musicLevel);
    if (cui_checkbox_ex(ui, "Sound effects", &settings.effects, nullptr)) changed = true;
    drawLevel("Effects volume", settings.effectsLevel);
    if (cui_checkbox_ex(ui, "Reverse stereo effects", &settings.reversePan, nullptr)) changed = true;
    if (changed) {
        client.audio.Apply(settings);
        client.audioDirty = true;
    }
    cui_push_disabled(ui, !settings.effects);
    if (Button(ui, "Play test sound")) client.audio.PlayEffect(ClickSound, settings);
    cui_pop_disabled(ui);
    if (!client.audio.Error().empty()) cui_secondary(ui, client.audio.Error().c_str());
    if (!client.audioSaveError.empty()) cui_secondary(ui, client.audioSaveError.c_str());
    cui_dialog_footer_begin(ui);
    const bool done = Button(ui, "Done", true) || cui_key_pressed(ui, CUI_KEY_ESCAPE);
    cui_dialog_footer_end(ui);
    cui_dialog_end(ui);
    if (done && client.SaveAudioSettings()) client.audioOpen = false;
}

void Draw(cui_ctx* ui, LoginClient& client)
{
    cui_box page = {};
    page.width = CUI_FILL;
    page.height = CUI_FILL;
    page.pad = 28;
    page.gap = 16;
    cui_column_begin(ui, &page);
    if (client.stage == Stage::Groups) {
        DrawServerGroups(ui, client);
        cui_box_end(ui);
        return;
    }
    if (client.stage == Stage::Characters) {
        DrawCharacters(ui, client);
        cui_box_end(ui);
        cui_dialog_opts modal = {};
        modal.width = 360;
        modal.persistent = true;
        modal.no_close_button = true;
        if (client.stage == Stage::Characters) DrawCharacterDialog(ui, client, modal);
        return;
    }
    if (client.stage == Stage::GameSession) {
        DrawGameChat(ui, client);
        cui_box_end(ui);
        if (client.chat.receptionOpen) DrawChatReception(ui, client);
        else if (client.serverOptions.open) DrawServerOptions(ui, client);
        else if (client.audioOpen) DrawAudioSettings(ui, client);
        else if (client.party.open || client.party.notice.id) DrawParty(ui, client);
        else if (client.guild.open || client.guild.requested || client.guild.notice ||
                 client.guild.action != mir3_cui::GuildAction::None)
            DrawGuild(ui, client);
        else if (client.quests.open) DrawQuests(ui, client);
        if (!client.showSignOut) return;
    } else {
        cui_heading(ui, "Mir3 Client", 1);
        cui_spacer(ui);
        if (client.stage == Stage::Login) {
            cui_push_disabled(ui, !client.status.empty() || client.settingsOpen || client.audioOpen);
            DrawAccount(ui, client);
            cui_pop_disabled(ui);
        }
        cui_box_end(ui);
    }

    cui_dialog_opts modal = {};
    modal.width = 340;
    modal.persistent = true;
    modal.no_close_button = true;

    bool open = true;
    switch (client.stage) {
    case Stage::Groups:
    case Stage::Characters:
        break;
    case Stage::Login: {
        if (client.audioOpen) {
            DrawAudioSettings(ui, client);
        } else if (!client.status.empty()) {
            modal.description = client.status.c_str();
            if (cui_dialog_begin(ui, "Sign-in notice", &open, &modal)) {
                cui_dialog_footer_begin(ui);
                const bool dismiss = Button(ui, "OK", true) || cui_key_pressed(ui, CUI_KEY_ENTER) ||
                    cui_key_pressed(ui, CUI_KEY_SPACE);
                cui_dialog_footer_end(ui);
                cui_dialog_end(ui);
                if (dismiss) client.DismissNotice();
            }
        } else if (client.settingsOpen && cui_dialog_begin(ui, "Gateway settings", &client.settingsOpen, &modal)) {
            Input(ui, "Gateway IPv4 address", client.host, sizeof client.host);
            Input(ui, "Port", client.port, sizeof client.port);
            cui_checkbox_ex(ui, "Use legacy server numbering", &client.legacy, nullptr);
            cui_dialog_footer_begin(ui);
            if (Button(ui, "Done", true)) client.settingsOpen = false;
            cui_dialog_footer_end(ui);
            cui_dialog_end(ui);
        }
        break;
    }
    case Stage::Servers:
    case Stage::Handoff: {
        const bool handingOff = client.stage == Stage::Handoff;
        if (client.billing.message) {
            const auto description = BillingText(client.billing);
            modal.description = description.c_str();
            if (cui_dialog_begin(ui, "Account notice", &open, &modal)) {
                cui_dialog_footer_begin(ui);
                if (Button(ui, "OK", true) || cui_key_pressed(ui, CUI_KEY_ENTER) ||
                    cui_key_pressed(ui, CUI_KEY_SPACE))
                    client.billing = {};
                cui_dialog_footer_end(ui);
                cui_dialog_end(ui);
            }
            break;
        }
        if (!cui_dialog_begin(ui, "Select a server", &open, &modal)) break;
        cui_push_disabled(ui, handingOff);
        if (!client.servers.empty()) {
            cui_scroll_opts list = {};
            list.id = "servers";
            list.height = CUI_PX(188);
            list.width = CUI_FILL;
            list.gap = 6;
            cui_scroll_begin(ui, &list);
            for (size_t i = 0; i < client.servers.size(); ++i) {
                cui_push_id_int(ui, static_cast<int>(i));
                const auto& server = client.servers[i];
                static const char* const loads[] = {"", " (Low load)", " (Medium load)", " (High load)"};
                const std::string label = server.label + loads[server.load < 4 ? server.load : 0] + "###server";
                if (Button(ui, label.c_str())) client.Select(server);
                cui_pop_id(ui);
            }
            cui_scroll_end(ui);
        } else {
            cui_secondary(ui, client.status.c_str());
        }
        cui_dialog_footer_begin(ui);
        if (Button(ui, "Back")) client.Reset();
        cui_dialog_footer_end(ui);
        cui_pop_disabled(ui);
        if (handingOff) cui_secondary(ui, "Opening character selection...");
        cui_dialog_end(ui);
        break;
    }
    case Stage::Complete:
        if (cui_dialog_begin(ui, "Character gateway", &open, &modal)) {
            cui_code(ui, client.status.c_str());
            cui_dialog_footer_begin(ui);
            if (Button(ui, "Back to sign-in")) client.Reset();
            cui_dialog_footer_end(ui);
            cui_dialog_end(ui);
        }
        break;
    case Stage::ConnectingGame:
    case Stage::AuthenticatingGame:
    case Stage::GameSession:
    case Stage::SigningOutGame:
    case Stage::GameDisconnected: {
        const bool disconnected = client.stage == Stage::GameDisconnected;
        modal.description = client.showSignOut ? "End the current game session?" : client.status.c_str();
        if (cui_dialog_begin(ui, client.showSignOut ? "Sign out" : "Game gateway", &open, &modal)) {
            const auto address = client.gameGateway.host + ':' + std::to_string(client.gameGateway.port);
            cui_code(ui, address.c_str());
            cui_label(ui, client.gameCharacter.c_str());
            cui_dialog_footer_begin(ui);
            if (client.showSignOut) {
                if (Button(ui, "Cancel")) client.showSignOut = false;
                else if (Button(ui, "Confirm", true)) client.SignOutGame();
            } else if (disconnected) {
                if (Button(ui, "Back to sign-in")) client.Reset();
            } else if (client.stage != Stage::SigningOutGame && Button(ui, "Cancel")) {
                client.Reset();
            }
            cui_dialog_footer_end(ui);
            cui_dialog_end(ui);
        }
        break;
    }
    default:
        modal.description = client.status.c_str();
        if (cui_dialog_begin(ui, "Connecting", &open, &modal)) {
            cui_dialog_footer_begin(ui);
            if (Button(ui, "Cancel")) client.Reset();
            cui_dialog_footer_end(ui);
            cui_dialog_end(ui);
        }
        break;
    }
}

} // namespace

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    WSADATA winsock;
    if (WSAStartup(MAKEWORD(2, 2), &winsock) != 0) return 1;
    const HRESULT com = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    cui_app_desc window = {};
    window.title = "Mir3 CUI Client";
    window.width = 680;
    window.height = 570;
    window.min_width = 410;
    window.min_height = 530;
    window.theme = CUI_THEME_DARK;
    window.accent = CUI_ACCENT_AMBER;
    cui_app* app = cui_app_create(&window);
    if (app) {
        LoginClient client;
        wchar_t audioOptionsPath[MAX_PATH];
        const auto audioPathLength = GetFullPathNameW(L"CuiOptions.ini", MAX_PATH, audioOptionsPath, nullptr);
        if (audioPathLength && audioPathLength < MAX_PATH) {
            client.audioOptionsPath = audioOptionsPath;
            client.audioOptions.Load(audioOptionsPath);
        }
        ReadSettings(client.host, sizeof client.host, client.port, sizeof client.port, client.legacy,
            client.registration, sizeof client.registration, client.recovery, sizeof client.recovery,
            client.configuredServers);
        std::ifstream groupsFile("SelLogin.ini", std::ios::binary);
        if (client.serverGroups.Load(groupsFile)) client.stage = Stage::Groups;
        auto previousStage = client.stage;
        client.audio.SetMusic(LoginMusic, client.audioOptions);
        LoadChineseFont(cui_app_ui(app));
        while (cui_app_frame(app)) {
            auto* ui = cui_app_ui(app);
            client.Poll();
            Draw(ui, client);
            if (client.stage != previousStage) {
                client.audio.SetMusic(client.stage == Stage::GameSession ? SessionMusic : LoginMusic,
                    client.audioOptions);
                if ((client.stage == Stage::Servers || client.stage == Stage::Characters ||
                     client.stage == Stage::GameSession) &&
                    GetFileAttributesW(ClickSound) != INVALID_FILE_ATTRIBUTES &&
                    client.audioOptions.effects)
                    client.audio.PlayEffect(ClickSound, client.audioOptions);
                previousStage = client.stage;
            }
            if (client.Active() || client.stage == Stage::Handoff || client.stage == Stage::ConnectingGame)
                cui_request_wakeup(ui, 0.03f);
        }
        client.SaveAudioSettings();
        cui_app_destroy(app);
    }
    if (SUCCEEDED(com)) CoUninitialize();
    WSACleanup();
    return app ? 0 : 1;
}
