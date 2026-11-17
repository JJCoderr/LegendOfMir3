#include "CoreHeaders.h"
#include "../EIServer.h"
namespace {
constexpr int StartCommand=1;
constexpr int StopCommand=2;
bool started=false;
LRESULT CALLBACK WindowProc(HWND window,UINT message,WPARAM w,LPARAM l) {
    switch(message) {
    case WM_CREATE:
        CreateWindowW(L"BUTTON",L"Start",WS_CHILD|WS_VISIBLE,20,20,100,32,window,(HMENU)StartCommand,nullptr,nullptr);
        CreateWindowW(L"BUTTON",L"Stop",WS_CHILD|WS_VISIBLE,140,20,100,32,window,(HMENU)StopCommand,nullptr,nullptr);
        return 0;
    case WM_COMMAND:
        if(LOWORD(w)==StartCommand && !started) { ei::SERVICE_ROLE::Start();started=true; }
        if(LOWORD(w)==StopCommand && started) { ei::SERVICE_ROLE::Stop();started=false; }
        return 0;
    case WM_CLOSE:
        if(started) ei::SERVICE_ROLE::Stop();
        DestroyWindow(window);return 0;
    case WM_DESTROY: PostQuitMessage(0);return 0;
    }
    const auto result=ei::SERVICE_ROLE::Dispatch(window,message,w,l);
    return message>=WM_USER?result:DefWindowProcW(window,message,w,l);
}
}
int WINAPI WinMain(HINSTANCE instance,HINSTANCE,LPSTR args,int show) {
    wchar_t executable[MAX_PATH];GetModuleFileNameW(nullptr,executable,MAX_PATH);
    std::wstring directory(executable);directory.resize(directory.find_last_of(L"\\/"));
    SetCurrentDirectoryW(directory.c_str());
    WNDCLASSW wc{};wc.hInstance=instance;wc.lpfnWndProc=WindowProc;wc.lpszClassName=L"Mir3ServiceWindow";
    RegisterClassW(&wc);
    HWND window=CreateWindowW(wc.lpszClassName,L"Mir3 Service",WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,CW_USEDEFAULT,360,160,nullptr,nullptr,instance,nullptr);
    if(!window || !ei::SERVICE_ROLE::Initialize(instance,window)) return 1;
    ShowWindow(window,show);
    if(!strcmp(args,"--auto-start")) PostMessageW(window,WM_COMMAND,StartCommand,0);
    MSG msg{};while(GetMessageW(&msg,nullptr,0,0)>0) { TranslateMessage(&msg);DispatchMessageW(&msg); }
    ExitProcess(static_cast<UINT>(msg.wParam));
}
