#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <string>
#include <vector>

namespace {
constexpr DWORD StartupTimeoutMs = 60000;
constexpr DWORD PollIntervalMs = 100;
struct Service { const wchar_t* name; const wchar_t* section; int port; };
const Service services[] = {
    {L"DBSvr", L"DatabaseSvr", 6000}, {L"LoginSvr", L"LoginSvr", 5500},
    {L"GameSvr", L"GameServer", 5000}, {L"LoginGate", L"LoginGate", 7000},
    {L"SelGate", L"SelGate", 7100}, {L"GameGate", L"GameGate", 7200}
};

bool Listening(int port)
{
    ULONG size = 0;
    if (GetTcpTable2(nullptr, &size, FALSE) != ERROR_INSUFFICIENT_BUFFER) return false;
    std::vector<DWORD> buffer((size + sizeof(DWORD) - 1) / sizeof(DWORD));
    auto table = reinterpret_cast<PMIB_TCPTABLE2>(buffer.data());
    if (GetTcpTable2(table, &size, FALSE) != NO_ERROR) return false;
    for (DWORD i = 0; i < table->dwNumEntries; ++i) {
        const auto& row = table->table[i];
        if (row.dwOwningPid == GetCurrentProcessId() && row.dwState == MIB_TCP_STATE_LISTEN
            && ntohs(static_cast<u_short>(row.dwLocalPort)) == port) return true;
    }
    return false;
}

std::wstring Extract(HINSTANCE instance, int id, const std::wstring& directory, const wchar_t* name)
{
    HRSRC resource = FindResourceW(instance, MAKEINTRESOURCEW(id), RT_RCDATA);
    if (!resource) return {};
    const DWORD size = SizeofResource(instance, resource);
    const void* bytes = LockResource(LoadResource(instance, resource));
    if (!bytes || !size) return {};
    const auto path = directory + L"\\" + name + L".dll";
    HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return {};
    DWORD written = 0;
    bool ok = WriteFile(file, bytes, size, &written, nullptr) && written == size;
    CloseHandle(file);
    return ok ? path : std::wstring();
}
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
    wchar_t executable[MAX_PATH];
    GetModuleFileNameW(nullptr, executable, MAX_PATH);
    std::wstring directory(executable);
    directory.resize(directory.find_last_of(L"\\/"));
    SetCurrentDirectoryW(directory.c_str());
    if (GetFileAttributesW(L"EIServer.ini") == INVALID_FILE_ATTRIBUTES) {
        MessageBoxW(nullptr, L"Place EIServer.ini beside EIServer.exe.", L"EIServer", MB_ICONERROR);
        return 1;
    }

    wchar_t temp[MAX_PATH], unique[MAX_PATH];
    if (!GetTempPathW(MAX_PATH, temp) || !GetTempFileNameW(temp, L"EIS", 0, unique)) return 1;
    DeleteFileW(unique);
    if (!CreateDirectoryW(unique, nullptr)) return 1;
    // Keep modules loaded until process exit: legacy worker threads refer to their code.
    const auto admin = Extract(instance, 7, unique, L"AdminCmd");
    if (admin.empty() || !LoadLibraryW(admin.c_str())) return 1;
    std::vector<HANDLE> threads;
    for (unsigned i = 0; i < sizeof(services) / sizeof(services[0]); ++i) {
        const auto& service = services[i];
        const auto path = Extract(instance, i + 1, unique, service.name);
        HMODULE module = path.empty() ? nullptr : LoadLibraryW(path.c_str());
        auto run = module ? reinterpret_cast<LPTHREAD_START_ROUTINE>(GetProcAddress(module, "_RunServer@4")) : nullptr;
        if (!run && module) run = reinterpret_cast<LPTHREAD_START_ROUTINE>(GetProcAddress(module, "RunServer"));
        HANDLE thread = run ? CreateThread(nullptr, 0, run, module, 0, nullptr) : nullptr;
        if (!thread) {
            MessageBoxW(nullptr, service.name, L"EIServer module failed", MB_ICONERROR);
            ExitProcess(1);
        }
        threads.push_back(thread);
        int port = GetPrivateProfileIntW(service.section, L"LocalPort", service.port, L".\\EIServer.ini");
        const ULONGLONG deadline = GetTickCount64() + StartupTimeoutMs;
        while (!Listening(port)) {
            if (WaitForSingleObject(thread, PollIntervalMs) != WAIT_TIMEOUT || GetTickCount64() >= deadline) {
                MessageBoxW(nullptr, service.name, L"EIServer startup failed or timed out", MB_ICONERROR);
                ExitProcess(1);
            }
        }
    }
    // Closing any service ends the integrated instance, so no half-running stack remains.
    WaitForMultipleObjects(static_cast<DWORD>(threads.size()), threads.data(), FALSE, INFINITE);
    ExitProcess(0);
}
