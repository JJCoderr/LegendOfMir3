#include <windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);

// Each DLL owns its legacy globals, resources, and window message queue.
extern "C" __declspec(dllexport) DWORD WINAPI RunServer(void* module)
{
    char arguments[] = "--auto-start";
    return WinMain(static_cast<HINSTANCE>(module), nullptr, arguments, SW_HIDE);
}
