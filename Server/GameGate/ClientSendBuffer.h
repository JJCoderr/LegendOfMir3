#pragma once
#include <winsock2.h>
#include <string>

namespace gatewire {
constexpr size_t MaxPendingBytes = 1024 * 1024;

inline bool AppendFrame(std::string& pending, const char* header, size_t headerSize,
                        const char* body, size_t bodySize)
{
    // Positive-length GameSvr messages include the body's terminating NUL.
    // Already encoded messages (no separate header) carry their exact wire length.
    if (headerSize && bodySize && body[bodySize - 1] == '\0') --bodySize;
    if (headerSize > MaxPendingBytes - 2 || bodySize > MaxPendingBytes - 2 - headerSize
        || pending.size() > MaxPendingBytes - 2 - headerSize - bodySize) return false;
    pending += '#';
    pending.append(header, headerSize);
    pending.append(body, bodySize);
    pending += '!';
    return true;
}

// The caller holds the session send lock. Keep every byte not accepted by Winsock.
inline int SendPending(SOCKET socket, std::string& pending, decltype(&WSASend) send = WSASend)
{
    if (pending.empty()) return 0;
    WSABUF buffer{static_cast<ULONG>(pending.size()), &pending[0]};
    DWORD sent = 0;
    if (send(socket, &buffer, 1, &sent, 0, nullptr, nullptr) == SOCKET_ERROR) {
        const int error = WSAGetLastError();
        return error == WSAEWOULDBLOCK ? 0 : error;
    }
    pending.erase(0, sent);
    return 0;
}
}
