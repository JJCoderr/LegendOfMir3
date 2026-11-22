#include "Service/CoreHeaders.h"
#include "Def/JsonRpc.h"
#include "EIServer.h"
#include "Def/DebugService.h"

namespace ei {
static const ServiceCore services[] = {
    {L"DBSvr",L"DatabaseSvr",6000,DBSvr::Initialize,DBSvr::Start,DBSvr::Dispatch},
    {L"LoginSvr",L"LoginSvr",5500,LoginSvr::Initialize,LoginSvr::Start,LoginSvr::Dispatch},
    {L"GameSvr",L"GameServer",5000,GameSvr::Initialize,GameSvr::Start,GameSvr::Dispatch},
    {L"LoginGate",L"LoginGate",7000,LoginGate::Initialize,LoginGate::Start,LoginGate::Dispatch},
    {L"SelGate",L"SelGate",7100,SelGate::Initialize,SelGate::Start,SelGate::Dispatch},
    {L"GameGate",L"GameGate",7200,GameGate::Initialize,GameGate::Start,GameGate::Dispatch}
};
constexpr UINT_PTR StartupTimer = 1;
constexpr DWORD StartupTimeoutMs = 60000;
constexpr wchar_t ConfigName[] = L"EIServer.ini";
static DWORD mainThread;
static BOOL WINAPI ConsoleControl(DWORD event)
{
    if(event==CTRL_C_EVENT || event==CTRL_BREAK_EVENT || event==CTRL_CLOSE_EVENT) {
        PostThreadMessage(mainThread,WM_QUIT,0,0);
        return TRUE;
    }
    return FALSE;
}
static bool Listening(int port)
{
    ULONG size=0;
    if(GetTcpTable2(nullptr,&size,FALSE)!=ERROR_INSUFFICIENT_BUFFER) return false;
    std::vector<DWORD> data((size+sizeof(DWORD)-1)/sizeof(DWORD));
    auto table=reinterpret_cast<PMIB_TCPTABLE2>(data.data());
    if(GetTcpTable2(table,&size,FALSE)!=NO_ERROR) return false;
    for(DWORD i=0;i<table->dwNumEntries;++i) {
        const auto& row=table->table[i];
        if(row.dwOwningPid==GetCurrentProcessId() && row.dwState==MIB_TCP_STATE_LISTEN &&
           ntohs(static_cast<u_short>(row.dwLocalPort))==port) return true;
    }
    return false;
}
static LRESULT CALLBACK MessageReceiver(HWND window, UINT message, WPARAM w, LPARAM l)
{
    if(message==WM_NCCREATE) {
        auto create=reinterpret_cast<CREATESTRUCTW*>(l);
        SetWindowLongPtrW(window,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(create->lpCreateParams));
    }
    auto core=reinterpret_cast<const ServiceCore*>(GetWindowLongPtrW(window,GWLP_USERDATA));
    if(core && message>=WM_USER) return core->dispatch(window,message,w,l);
    return DefWindowProcW(window,message,w,l);
}
int Run()
{
    wchar_t path[MAX_PATH];
    if (!GetCurrentDirectoryW(_countof(path), path)) return 1;
    const std::wstring folder(path);
    if (GetFileAttributesW(ConfigName) == INVALID_FILE_ATTRIBUTES) {
        mir3::ConsoleLog(L"EIServer", (std::wstring(L"Cannot read ./EIServer.ini in ") + folder).c_str());
        return 1;
    }
    mir3::ConsoleLog(L"EIServer", (std::wstring(L"Runtime directory: ") + folder).c_str());
    HINSTANCE instance=GetModuleHandleW(nullptr);
    HWND windows[_countof(services)]{};
    for(unsigned i=0;i<_countof(services);++i) {
        const auto& core=services[i];
        WNDCLASSW wc{};wc.lpfnWndProc=MessageReceiver;wc.hInstance=instance;wc.lpszClassName=core.name;
        if(!RegisterClassW(&wc)) return 1;
        windows[i]=CreateWindowW(core.name,L"",0,0,0,0,0,HWND_MESSAGE,nullptr,instance,const_cast<ServiceCore*>(&core));
        if(!windows[i]) return 1;
    }
    MSG msg{};PeekMessage(&msg,nullptr,0,0,PM_NOREMOVE);
    mainThread=GetCurrentThreadId();SetConsoleCtrlHandler(ConsoleControl,TRUE);
    SetTimer(nullptr,StartupTimer,100,nullptr);
    unsigned current=0;
    ULONGLONG deadline=0;
    bool starting=false;
    mir3::DebugState debugState(L"EIServer");
    mir3::JsonRpc rpc(GetCommandLineA(), L"Server");
    for (;;) {
        const BOOL result = GetMessage(&msg,nullptr,0,0);
        wchar_t message[256];
        if (result == -1) {
            const DWORD error = GetLastError();
            swprintf_s(message, L"GetMessage failed: error=%lu", error);
            mir3::ConsoleLog(L"EIServer", message);
            return 1;
        }
        if (result == 0) {
            swprintf_s(message, L"WM_QUIT received: exitCode=%llu",
                static_cast<unsigned long long>(msg.wParam));
            mir3::ConsoleLog(L"EIServer", message);
            return static_cast<int>(msg.wParam);
        }
        if (msg.hwnd && IsWindow(msg.hwnd)) {
            swprintf_s(message,
                L"MSG hwnd=%p message=0x%04X wParam=0x%llX lParam=0x%llX time=%lu pt=(%ld,%ld)",
                static_cast<void*>(msg.hwnd), msg.message,
                static_cast<unsigned long long>(msg.wParam),
                static_cast<unsigned long long>(static_cast<ULONG_PTR>(msg.lParam)),
                msg.time, msg.pt.x, msg.pt.y);
            mir3::ConsoleLog(L"EIServer", message);
        }
        rpc.Poll([&](const std::string& method, const std::string&) {
            if (method != "state.get" && method != "state.wait_ready") throw mir3::JsonRpc::Error{-32601, "Method not found"};
            if (method == "state.wait_ready" && current < _countof(services)) throw mir3::JsonRpc::Pending{StartupTimeoutMs};
            return std::string("{\"schema\":1,\"ready\":") + mir3::BoolJson(current==_countof(services))
                + ",\"services_ready\":" + std::to_string(current) + mir3::ServiceTcpState() + "}";
        });
        if(!msg.hwnd && msg.message==WM_TIMER) {
            debugState.Poll([&] {
                return std::string(",\"ready\":") + mir3::BoolJson(current==_countof(services))
                    + ",\"services_ready\":" + std::to_string(current) + mir3::ServiceTcpState();
            });
        }
        if(!msg.hwnd && msg.message==WM_TIMER && current<_countof(services)) {
            const auto& core=services[current];
            if(!starting) {
                mir3::ConsoleLog(core.name, L"Starting");
                if(!core.initialize(instance,windows[current])) {
                    mir3::ConsoleLog(core.name, (std::wstring(L"Initialization failed in ") + folder).c_str());
                    return 1;
                }
                core.start();starting=true;deadline=GetTickCount64()+StartupTimeoutMs;
            }
            int port=GetPrivateProfileIntW(core.section,L"LocalPort",core.defaultPort,L".\\EIServer.ini");
            if(Listening(port)) {
                mir3::ConsoleLog(core.name, (std::wstring(L"Listening on ") + std::to_wstring(port)).c_str());
                ++current;starting=false;
                if(current==_countof(services)) { mir3::ConsoleLog(L"EIServer", L"All services ready. Ctrl+C to exit."); }
            } else if(GetTickCount64()>=deadline) {
                mir3::ConsoleLog(core.name, L"Startup timed out");return 1;
            }
        } else {
            TranslateMessage(&msg);DispatchMessage(&msg);
        }
    }
}
}
int main() { const int result=ei::Run();ExitProcess(result); }
