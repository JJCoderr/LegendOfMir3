#pragma once

#include <windows.h>
#include <string>

namespace mir3 {

inline const char* BoolJson(bool value) { return value ? "true" : "false"; }

inline std::string QuoteLocalText(const char* text)
{
    const int length = MultiByteToWideChar(CP_ACP, 0, text, -1, nullptr, 0);
    if (!length) return "\"\"";
    std::wstring wide(length, L'\0');
    MultiByteToWideChar(CP_ACP, 0, text, -1, &wide[0], length);
    const int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (!size) return "\"\"";
    std::string utf8(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, &utf8[0], size, nullptr, nullptr);
    std::string quoted = "\"";
    for (unsigned char ch : utf8) {
        if (ch == '"' || ch == '\\') quoted += '\\';
        if (ch >= 32) quoted += static_cast<char>(ch);
    }
    return quoted + '"';
}

class DebugState
{
    std::wstring path, temporary;
    std::string prefix;
    ULONGLONG nextWrite = 0;

public:
    explicit DebugState(const wchar_t* role)
    {
        wchar_t directory[MAX_PATH];
        DWORD length = GetEnvironmentVariableW(L"MIR3_DEBUG_STATE_DIR", directory, MAX_PATH);
        if (!length || length >= MAX_PATH) return;
        const auto pid = GetCurrentProcessId();
        path = std::wstring(directory) + L"\\" + role + L"-" + std::to_wstring(pid) + L".json";
        temporary = path + L".tmp";
        const auto name = std::wstring(role);
        prefix = "{\"schema\":1,\"role\":\"" + std::string(name.begin(), name.end())
            + "\",\"pid\":" + std::to_string(pid);
    }

    ~DebugState()
    {
        if (Enabled()) {
            DeleteFileW(temporary.c_str());
            DeleteFileW(path.c_str());
        }
    }

    bool Enabled() const { return !path.empty(); }

    template<class Capture>
    void Poll(Capture capture)
    {
        if (!Enabled()) return;
        const auto now = GetTickCount64();
        if (now < nextWrite) return;
        nextWrite = now + 500;
        const std::string json = prefix + capture() + "}\n";
        HANDLE file = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr,
                                 CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) return;
        DWORD written = 0;
        const bool complete = WriteFile(file, json.data(), static_cast<DWORD>(json.size()), &written, nullptr)
            && written == json.size();
        CloseHandle(file);
        if (complete) MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING);
    }
};

}
