#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include "cui.h"
#include "Common/EnDecode.h"
#include "Common/Protocol.h"

#include <cerrno>
#include <cstdlib>
#include <string>
#include <vector>

namespace {

enum class Stage { Login, Connecting, SigningIn, Servers, Selecting, Handoff, Complete };

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

unsigned Number(const std::string& text, unsigned maximum)
{
    unsigned value = 0;
    for (char digit : text) {
        if (digit < '0' || digit > '9' || value > (maximum - (digit - '0')) / 10)
            return 0;
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
    std::vector<Server> configuredServers;
    std::vector<Server> servers;
    BillingNotice billing;
    int selectionSession = 0;
    std::string status;

    ~LoginClient() { Reset(); }

    void Reset(const char* message = "")
    {
        Close();
        servers.clear();
        billing = {};
        selectionSession = 0;
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
        if (legacy && user.size() + 1 + secret.size() >= 32) {
            SecureZeroMemory(&secret[0], secret.size());
            status = "The legacy gateway accepts up to 31 bytes for account, slash, and password.";
            return;
        }

        Close();
        socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        u_long nonBlocking = 1;
        if (!Active() || ioctlsocket(socket_, FIONBIO, &nonBlocking) == SOCKET_ERROR) {
            SecureZeroMemory(&secret[0], secret.size());
            Reset("Could not open the login connection.");
            return;
        }

        const int result = connect(socket_, reinterpret_cast<const sockaddr*>(&address), sizeof address);
        if (result == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) {
            SecureZeroMemory(&secret[0], secret.size());
            Reset("Could not connect to the login gateway.");
            return;
        }

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

    void Poll()
    {
        if (stage == Stage::Handoff && GetTickCount64() >= deadline_) stage = Stage::Complete;
        if (!Active()) return;
        if (stage != Stage::Servers && GetTickCount64() >= deadline_) {
            Reset("The login gateway timed out.");
            return;
        }
        if (stage == Stage::Connecting) {
            fd_set writable, failed;
            FD_ZERO(&writable);
            FD_ZERO(&failed);
            FD_SET(socket_, &writable);
            FD_SET(socket_, &failed);
            timeval timeout = {};
            if (select(0, nullptr, &writable, &failed, &timeout) == SOCKET_ERROR ||
                FD_ISSET(socket_, &failed)) {
                Reset("Could not connect to the login gateway.");
                return;
            }
            if (!FD_ISSET(socket_, &writable)) return;
            Wait(Stage::SigningIn, "Waiting for sign-in...");
        }

        if (!output_.empty()) {
            const int sent = send(socket_, output_.data() + sent_,
                static_cast<int>(output_.size() - sent_), 0);
            if (sent > 0) {
                sent_ += sent;
                if (sent_ == output_.size()) ClearOutput();
            } else if (sent == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) {
                Reset("Could not send to the login gateway.");
                return;
            }
        }

        char buffer[4096];
        const int count = recv(socket_, buffer, sizeof buffer, 0);
        if (count == 0 || (count == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)) {
            Reset("The login gateway disconnected.");
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
        if (header.wIdent == SM_CERTIFICATION_FAIL) {
            Reset("The login gateway rejected authentication.");
            return;
        }
        if (header.wIdent == SM_ID_NOTFOUND) {
            Reset("The account was not found.");
            return;
        }
        if (header.wIdent == SM_PASSWD_FAIL) {
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
        if (header.wIdent == SM_OUTOFCONNECTION) {
            Reset("The login gateway ended the session.");
            return;
        }
        if (header.wIdent != SM_PASSOK_SELECTSERVER && header.wIdent != SM_SELECTSERVER_OK) return;

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
        if (header.wIdent == SM_PASSOK_SELECTSERVER && stage == Stage::SigningIn) {
            if (!ReadServers(body)) {
                Reset("The login gateway sent an invalid server list.");
                return;
            }
            billing = legacy ? BillingNotice{} : ReadBilling(header);
            stage = Stage::Servers;
            status = servers.empty() ? "The gateway returned no selectable servers." : "";
        } else if (header.wIdent == SM_SELECTSERVER_OK &&
            (stage == Stage::Selecting || stage == Stage::Servers)) {
            const auto slash = body.find('/');
            const auto end = slash == std::string::npos ? slash : body.find('/', slash + 1);
            in_addr ip = {};
            if (end == std::string::npos || InetPtonA(AF_INET, body.substr(0, slash).c_str(), &ip) != 1 ||
                !Number(body.substr(slash + 1, end - slash - 1), 65535)) {
                Reset("The gateway returned an invalid selection endpoint.");
                return;
            }
            status = body.substr(0, slash) + ':' + body.substr(slash + 1, end - slash - 1);
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

bool Button(cui_ctx* ui, const char* label, bool primary = false)
{
    cui_button_opts options = {};
    options.variant = primary ? CUI_BUTTON_PRIMARY : CUI_BUTTON_SECONDARY;
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
    if (client.status.empty() && !client.settingsOpen && (submit || (result & CUI_SUBMITTED)))
        client.SignIn();
}

void Draw(cui_ctx* ui, LoginClient& client)
{
    cui_box page = {};
    page.width = CUI_FILL;
    page.height = CUI_FILL;
    page.pad = 28;
    page.gap = 16;
    cui_column_begin(ui, &page);
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
