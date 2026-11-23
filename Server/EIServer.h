#pragma once
#include <windows.h>
#include <string>
namespace ei {
struct ServiceCore {
    const wchar_t* name;
    const wchar_t* section;
    int defaultPort;
    bool (*initialize)(HINSTANCE, HWND);
    void (*start)();
    LRESULT (CALLBACK *dispatch)(HWND, UINT, WPARAM, LPARAM);
};
namespace DBSvr { bool Initialize(HINSTANCE,HWND); void Start(); void Stop(); LRESULT CALLBACK Dispatch(HWND,UINT,WPARAM,LPARAM); }
namespace LoginSvr { bool Initialize(HINSTANCE,HWND); void Start(); void Stop(); LRESULT CALLBACK Dispatch(HWND,UINT,WPARAM,LPARAM); }
namespace GameSvr {
bool Initialize(HINSTANCE,HWND);
void Start();
void Stop();
LRESULT CALLBACK Dispatch(HWND,UINT,WPARAM,LPARAM);
std::string MonsterShowcase(const std::string& params);
}
namespace LoginGate { bool Initialize(HINSTANCE,HWND); void Start(); void Stop(); LRESULT CALLBACK Dispatch(HWND,UINT,WPARAM,LPARAM); }
namespace SelGate { bool Initialize(HINSTANCE,HWND); void Start(); void Stop(); LRESULT CALLBACK Dispatch(HWND,UINT,WPARAM,LPARAM); }
namespace GameGate { bool Initialize(HINSTANCE,HWND); void Start(); void Stop(); LRESULT CALLBACK Dispatch(HWND,UINT,WPARAM,LPARAM); }
}
