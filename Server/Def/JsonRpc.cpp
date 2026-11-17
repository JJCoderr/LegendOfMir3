#include "JsonRpc.h"
#include "json.hpp"
#include <sstream>
#include <stdexcept>

namespace mir3 {
JsonRpc::JsonRpc(const char* commandLine, const wchar_t* role)
{
    std::istringstream arguments(commandLine);
    std::string argument;
    bool enabled = false;
    while (arguments >> argument) if (argument == "--debug-pipe") enabled = true;
    if (!enabled) return;
    const auto name = std::wstring(L"\\\\.\\pipe\\Mir3.") + role + L"." + std::to_wstring(GetCurrentProcessId());
    pipe = CreateNamedPipeW(name.c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_NOWAIT | PIPE_REJECT_REMOTE_CLIENTS,
        1, 65536, 65536, 0, nullptr);
    if (pipe == INVALID_HANDLE_VALUE) throw std::runtime_error("Cannot create client debug pipe");
}
JsonRpc::~JsonRpc() { if (pipe != INVALID_HANDLE_VALUE) CloseHandle(pipe); }
void JsonRpc::Disconnect()
{
    DisconnectNamedPipe(pipe);
    connected = false;
    input.clear();
    output.clear();
}
void JsonRpc::PollRaw(void* context, Handler handler)
{
    if (pipe == INVALID_HANDLE_VALUE) return;
    if (!connected) {
        if (!ConnectNamedPipe(pipe, nullptr) && GetLastError() != ERROR_PIPE_CONNECTED) return;
        connected = true;
    }
    if (!output.empty()) {
        DWORD written = 0;
        if (WriteFile(pipe, output.data(), static_cast<DWORD>(output.size()), &written, nullptr))
            output.erase(0, written);
        else if (GetLastError() != ERROR_NO_DATA) Disconnect();
        return;
    }
    char buffer[4096];
    DWORD received = 0;
    if (ReadFile(pipe, buffer, sizeof(buffer), &received, nullptr)) input.append(buffer, received);
    else if (GetLastError() != ERROR_NO_DATA) { Disconnect(); return; }
    constexpr size_t MaxRequest = 65536;
    if (input.size() > MaxRequest) { Disconnect(); return; }
    const auto newline = input.find('\n');
    if (newline == std::string::npos) return;
    const auto request = input.substr(0, newline);
    input.erase(0, newline + 1);
    using Json = nlohmann::json;
    const auto error = [](const Json& id, int code, const std::string& message) {
        return Json{{"jsonrpc", "2.0"}, {"id", id}, {"error", {{"code", code}, {"message", message}}}};
    };
    const auto call = [&](const Json& value) -> Json {
        if (!value.is_object() || !value.contains("jsonrpc") || value["jsonrpc"] != "2.0" ||
            !value.contains("method") || !value["method"].is_string())
            return error(nullptr, -32600, "Invalid Request");
        const Json id = value.value("id", Json());
        if (!(id.is_null() || id.is_string() || id.is_number()))
            return error(nullptr, -32600, "Invalid Request");
        const bool notification = !value.contains("id");
        Json result;
        try {
            const Json params = value.value("params", Json::object());
            if (!(params.is_object() || params.is_array())) throw Error{-32602, "Invalid params"};
            result = Json{{"jsonrpc", "2.0"}, {"id", id},
                {"result", Json::parse(handler(context, value["method"].get<std::string>(), params.dump()))}};
        } catch (const Error& e) { result = error(id, e.code, e.message); }
          catch (const std::exception&) { result = error(id, -32603, "Internal error"); }
        return notification ? Json() : result;
    };
    Json response;
    const Json value = Json::parse(request, nullptr, false);
    if (value.is_discarded()) response = error(nullptr, -32700, "Parse error");
    else if (value.is_array()) {
        if (value.empty()) response = error(nullptr, -32600, "Invalid Request");
        else {
            response = Json::array();
            for (const auto& entry : value) {
                const auto reply = call(entry);
                if (!reply.is_null()) response.push_back(reply);
            }
            if (response.empty()) response = nullptr;
        }
    } else response = call(value);
    if (!response.is_null()) output = response.dump() + "\n";
}
}
