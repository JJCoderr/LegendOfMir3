#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include "cui.h"
#include "Common/EnDecode.h"
#include "Common/Protocol.h"

#include <cerrno>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace {

enum class Stage {
    Groups, Login, Connecting, SigningIn, Servers, Selecting, Handoff,
    ConnectingCharacters, Characters, Complete, GameEndpoint
};
enum class CharacterDialog { None, Create, Delete };

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
        characters.clear();
        characterIndex = -1;
        characterPage = 0;
        characterRequest = 0;
        characterNotice.clear();
        CloseCharacterDialog();
        signedInAccount_.clear();
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

    void Poll()
    {
        if (stage == Stage::Handoff && GetTickCount64() >= deadline_) OpenCharacters();
        if (!Active()) return;
        const bool characterConnection = stage == Stage::Characters || stage == Stage::ConnectingCharacters;
        if (stage != Stage::Servers && (stage != Stage::Characters || characterRequest) &&
            GetTickCount64() >= deadline_) {
            Reset(characterConnection ? "The character gateway timed out." : "The login gateway timed out.");
            return;
        }
        if (stage == Stage::Connecting || stage == Stage::ConnectingCharacters) {
            fd_set writable, failed;
            FD_ZERO(&writable);
            FD_ZERO(&failed);
            FD_SET(socket_, &writable);
            FD_SET(socket_, &failed);
            timeval timeout = {};
            if (select(0, nullptr, &writable, &failed, &timeout) == SOCKET_ERROR ||
                FD_ISSET(socket_, &failed)) {
                Reset(characterConnection ? "Could not connect to the character gateway."
                                          : "Could not connect to the login gateway.");
                return;
            }
            if (!FD_ISSET(socket_, &writable)) return;
            if (characterConnection) {
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
                Reset(characterConnection ? "Could not send to the character gateway."
                                          : "Could not send to the login gateway.");
                return;
            }
        }

        char buffer[4096];
        const int count = recv(socket_, buffer, sizeof buffer, 0);
        if (count == 0 || (count == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)) {
            Reset(characterConnection ? "The character gateway disconnected." : "The login gateway disconnected.");
            return;
        }
        for (int i = 0; i < count && Active(); ++i) Receive(buffer[i]);
    }

private:
    SOCKET socket_ = INVALID_SOCKET;
    unsigned sequence_ = 0;
    ULONGLONG deadline_ = 0;
    bool receiving_ = false;
    std::string input_;
    std::string output_;
    size_t sent_ = 0;
    std::string signedInAccount_;

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
            Close();
            stage = Stage::GameEndpoint;
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

    void Queue(WORD opcode, WORD parameter, const std::string& body)
    {
        _TDEFAULTMESSAGE header;
        static_assert(sizeof(header) == 12, "Login header wire size");
        fnMakeDefMessage(&header, opcode, 0, parameter, 0, 0);
        char encodedHeader[DEFBLOCKSIZE + 1];
        fnEncodeMessage(&header, encodedHeader, sizeof encodedHeader);
        std::vector<char> encodedBody((body.size() * 4 + 2) / 3 + 1);
        fnEncode6BitBuf(reinterpret_cast<unsigned char*>(const_cast<char*>(body.data())),
            encodedBody.data(), static_cast<int>(body.size()), static_cast<int>(encodedBody.size()));
        sequence_ = sequence_ % 9 + 1;
        output_ += '#';
        output_ += static_cast<char>('0' + sequence_);
        output_ += encodedHeader;
        output_ += encodedBody.data();
        output_ += '!';
        SecureZeroMemory(encodedBody.data(), encodedBody.size());
    }

    void Receive(char byte)
    {
        if (byte == '*') {
            if (output_.size() >= 8192) {
                Reset("The login gateway sent too many pending heartbeats.");
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
            } else if (byte < 60 || byte > 123 || input_.size() >= 4096) {
                Reset("The login gateway sent an invalid reply.");
            } else {
                input_ += byte;
            }
        }
    }

    void HandleFrame()
    {
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
    if (client.stage == Stage::Login && client.status.empty() && !client.settingsOpen &&
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
    cui_heading(ui, "Mir3 Client", 1);
    cui_spacer(ui);
    if (client.stage == Stage::Login) {
        cui_push_disabled(ui, !client.status.empty() || client.settingsOpen);
        DrawAccount(ui, client);
        cui_pop_disabled(ui);
    }
    cui_box_end(ui);

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
        if (!client.status.empty()) {
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
    case Stage::GameEndpoint:
        if (cui_dialog_begin(ui, "Game gateway", &open, &modal)) {
            const auto address = client.gameGateway.host + ':' + std::to_string(client.gameGateway.port);
            cui_code(ui, address.c_str());
            cui_dialog_footer_begin(ui);
            if (Button(ui, "Back to sign-in")) client.Reset();
            cui_dialog_footer_end(ui);
            cui_dialog_end(ui);
        }
        break;
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
        ReadSettings(client.host, sizeof client.host, client.port, sizeof client.port, client.legacy,
            client.registration, sizeof client.registration, client.recovery, sizeof client.recovery,
            client.configuredServers);
        std::ifstream groupsFile("SelLogin.ini", std::ios::binary);
        if (client.serverGroups.Load(groupsFile)) client.stage = Stage::Groups;
        LoadChineseFont(cui_app_ui(app));
        while (cui_app_frame(app)) {
            auto* ui = cui_app_ui(app);
            client.Poll();
            Draw(ui, client);
            if (client.Active() || client.stage == Stage::Handoff) cui_request_wakeup(ui, 0.03f);
        }
        cui_app_destroy(app);
    }
    WSACleanup();
    return app ? 0 : 1;
}
