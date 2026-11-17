#pragma once

#include <windows.h>
#include <string>

namespace mir3 {

inline void ConsoleLog(const wchar_t* service, const wchar_t* message)
{
    static SRWLOCK consoleLogLock = SRWLOCK_INIT;
    SYSTEMTIME time;
    GetLocalTime(&time);
    wchar_t prefix[128];
    swprintf_s(prefix, L"[%02u:%02u:%02u.%03u] [%ls] ",
        time.wHour, time.wMinute, time.wSecond, time.wMilliseconds, service);
    const std::wstring line = std::wstring(prefix) + message + L"\r\n";
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode, written;
    AcquireSRWLockExclusive(&consoleLogLock);
    if (GetConsoleMode(output, &mode)) {
        WriteConsoleW(output, line.data(), static_cast<DWORD>(line.size()), &written, nullptr);
    } else {
        const int size = WideCharToMultiByte(CP_UTF8, 0, line.data(), int(line.size()), nullptr, 0, nullptr, nullptr);
        std::string utf8(size, '\0');
        WideCharToMultiByte(CP_UTF8, 0, line.data(), int(line.size()), &utf8[0], size, nullptr, nullptr);
        WriteFile(output, utf8.data(), static_cast<DWORD>(utf8.size()), &written, nullptr);
    }
    ReleaseSRWLockExclusive(&consoleLogLock);
}

}
