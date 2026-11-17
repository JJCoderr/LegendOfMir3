#define NOMINMAX
#include "JsonRpc.h"
#define PICOJSON_USE_INT64
#include "picojson.h"
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
    using Json = picojson::value;
    using Object = picojson::object;
    using Array = picojson::array;
    const auto parse = [](Json& value, const std::string& text) {
        std::string error;
        auto end = picojson::parse(value, text.begin(), text.end(), &error);
        while (end != text.end() && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) ++end;
        return error.empty() && end == text.end();
    };
    const auto error = [](const Json& id, int code, const std::string& message) {
        return Json(Object{{"jsonrpc", Json(std::string("2.0"))}, {"id", id},
            {"error", Json(Object{{"code", Json(double(code))}, {"message", Json(message)}})}});
    };
    const auto call = [&](const Json& value) -> Json {
        if (!value.is<Object>() || value.get("jsonrpc") != Json(std::string("2.0")) ||
            !value.get("method").is<std::string>())
            return error(Json(), -32600, "Invalid Request");
        const Json id = value.get("id");
        if (!(id.is<picojson::null>() || id.is<std::string>() || id.is<double>() || id.is<int64_t>()))
            return error(Json(), -32600, "Invalid Request");
        const bool notification = !value.contains("id");
        Json result;
        try {
            const Json params = value.contains("params") ? value.get("params") : Json(Object());
            if (!(params.is<Object>() || params.is<Array>())) throw Error{-32602, "Invalid params"};
            Json body;
            if (!parse(body, handler(context, value.get("method").get<std::string>(), params.serialize())))
                throw Error{-32603, "Internal error"};
            result = Json(Object{{"jsonrpc", Json(std::string("2.0"))}, {"id", id}, {"result", body}});
        } catch (const Error& e) { result = error(id, e.code, e.message); }
          catch (const std::exception&) { result = error(id, -32603, "Internal error"); }
        return notification ? Json() : result;
    };
    Json response, value;
    if (!parse(value, request)) response = error(Json(), -32700, "Parse error");
    else if (value.is<Array>()) {
        if (value.get<Array>().empty()) response = error(Json(), -32600, "Invalid Request");
        else {
            Array replies;
            for (const auto& entry : value.get<Array>()) {
                const auto reply = call(entry);
                if (!reply.is<picojson::null>()) replies.push_back(reply);
            }
            if (!replies.empty()) response = Json(replies);
        }
    } else response = call(value);
    if (!response.is<picojson::null>()) output = response.serialize() + "\n";
}
}
