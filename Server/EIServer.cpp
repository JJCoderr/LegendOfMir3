#include "Service/CoreHeaders.h"
#include "EIServer.h"
#include <iphlpapi.h>

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
    GetModuleFileNameW(nullptr,path,MAX_PATH);
    std::wstring folder(path);folder.resize(folder.find_last_of(L"\\/"));
    SetCurrentDirectoryW(folder.c_str());
    if(GetFileAttributesW(L"EIServer.ini")==INVALID_FILE_ATTRIBUTES) {
        fwprintf(stderr,L"EIServer.ini is required beside EIServer.exe\n");return 1;
    }
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
    while(GetMessage(&msg,nullptr,0,0)>0) {
        if(!msg.hwnd && msg.message==WM_TIMER && current<_countof(services)) {
            const auto& core=services[current];
            if(!starting) {
                wprintf(L"[%s] starting\n",core.name);fflush(stdout);
                if(!core.initialize(instance,windows[current])) return 1;
                core.start();starting=true;deadline=GetTickCount64()+StartupTimeoutMs;
            }
            int port=GetPrivateProfileIntW(core.section,L"LocalPort",core.defaultPort,L".\\EIServer.ini");
            if(Listening(port)) {
                wprintf(L"[%s] listening on %d\n",core.name,port);fflush(stdout);
                ++current;starting=false;
                if(current==_countof(services)) { puts("All services ready. Ctrl+C to exit.");fflush(stdout); }
            } else if(GetTickCount64()>=deadline) {
                fwprintf(stderr,L"[%s] startup timed out\n",core.name);return 1;
            }
        } else {
            TranslateMessage(&msg);DispatchMessage(&msg);
        }
    }
    return 0;
}
}
int main() { const int result=ei::Run();ExitProcess(result); }
