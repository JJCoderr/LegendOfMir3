#pragma once
#include <windows.h>
#include <string>

namespace mir3 {
class JsonRpc {
    HANDLE pipe = INVALID_HANDLE_VALUE;
    bool connected = false;
    std::string input, output;
    ULONGLONG pendingDeadline = 0;
    using Handler = std::string (*)(void*, const std::string&, const std::string&);
    void Disconnect();
    void PollRaw(void* context, Handler handler);
public:
    struct Error { int code; std::string message; };
    struct Pending { DWORD timeoutMs; };
    bool Enabled() const { return pipe != INVALID_HANDLE_VALUE; }
    JsonRpc(const char* commandLine, const wchar_t* role);
    ~JsonRpc();
    JsonRpc(const JsonRpc&) = delete;
    JsonRpc& operator=(const JsonRpc&) = delete;
    template<class Callback> void Poll(Callback callback) {
        PollRaw(&callback, [](void* context, const std::string& method, const std::string& params) {
            return (*static_cast<Callback*>(context))(method, params);
        });
    }
};
}
