#include "Dialogs.h"
#include "LoginClient.h"

using namespace mir3_client;

namespace {

void ReadSettings(const std::filesystem::path& root, char* host, DWORD hostSize, char* port, DWORD portSize, bool& legacy,
    char* registration, DWORD registrationSize, char* recovery, DWORD recoverySize,
    std::vector<Server>& configuredServers)
{
    wchar_t path[MAX_PATH];
    DWORD length = GetFullPathNameW((root / L"Mir3.ini").c_str(), MAX_PATH, path, nullptr);
    if (!length || length >= MAX_PATH) return;
    if (GetFileAttributesW(path) == INVALID_FILE_ATTRIBUTES) {
        length = GetFullPathNameW((root / L"Mir2Ei.ini").c_str(), MAX_PATH, path, nullptr);
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
    window.min_height = 300;
    window.design_width = 640;
    window.design_height = 480;
    window.theme = CUI_THEME_DARK;
    window.accent = CUI_ACCENT_AMBER;
    cui_app* app = cui_app_create(&window);
    if (app) {
        LoginClient client;
        Dialogs dialogs;
        wchar_t legacyRoot[MAX_PATH]{};
        const auto legacyRootLength = GetEnvironmentVariableW(L"MIR3_RESOURCE_ROOT", legacyRoot, MAX_PATH);
        const std::filesystem::path resourceRoot = legacyRootLength && legacyRootLength < MAX_PATH ? legacyRoot : L".";
        client.SetResourceRoot(resourceRoot);
        if (!dialogs.Open(cui_app_ui(app), resourceRoot)) {
            const auto message = L"无法加载登录素材。请检查 CmList.dat、Data/Interface1c 和 Data/GameInter。\n资源目录：" +
                std::filesystem::absolute(resourceRoot).wstring();
            MessageBoxW(nullptr, message.c_str(), L"传奇3", MB_OK | MB_ICONERROR);
            dialogs.Close(cui_app_ui(app));
            cui_app_destroy(app);
            if (SUCCEEDED(com)) CoUninitialize();
            WSACleanup();
            return 1;
        }
        wchar_t audioOptionsPath[MAX_PATH];
        const auto audioPathLength = GetFullPathNameW(L"CuiOptions.ini", MAX_PATH, audioOptionsPath, nullptr);
        if (audioPathLength && audioPathLength < MAX_PATH) {
            client.audioOptionsPath = audioOptionsPath;
            client.audioOptions.Load(audioOptionsPath);
            client.LoadMapOptions(audioOptionsPath);
        }
        ReadSettings(resourceRoot, client.host, sizeof client.host, client.port, sizeof client.port, client.legacy,
            client.registration, sizeof client.registration, client.recovery, sizeof client.recovery,
            client.configuredServers);
        std::ifstream groupsFile(resourceRoot / L"SelLogin.ini", std::ios::binary);
        if (client.serverGroups.Load(groupsFile)) client.stage = Stage::Groups;
        {
            const bool groups = client.stage == Stage::Groups;
            cui_app_set_size(app, groups ? 800 : 640, groups ? 600 : 480);
        }
        auto previousStage = client.stage;
        client.audio.SetMusic(LoginMusic, client.audioOptions);
        LoadChineseFont(cui_app_ui(app));
        auto drawFrame = [&](cui_ctx* ui) {
            if (!dialogs.DefersGameEntry()) client.Poll();
            DrawDialogs(ui, client, &dialogs);
            if (client.stage != previousStage) {
                client.audio.SetMusic(client.stage == Stage::GameSession || client.returnToCharactersAt ? SessionMusic : LoginMusic,
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
        };
        cui_app_set_frame_callback(app, [](cui_ctx* ui, void* context) {
            (*static_cast<decltype(drawFrame)*>(context))(ui);
        }, &drawFrame);
        while (cui_app_frame(app)) drawFrame(cui_app_ui(app));
        cui_app_set_frame_callback(app, nullptr, nullptr);
        client.SaveAudioSettings();
        dialogs.Close(cui_app_ui(app));
        cui_app_destroy(app);
    }
    if (SUCCEEDED(com)) CoUninitialize();
    WSACleanup();
    return app ? 0 : 1;
}
