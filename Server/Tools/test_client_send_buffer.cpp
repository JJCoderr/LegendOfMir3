#include "../GameGate/ClientSendBuffer.h"
#include <cstdio>

static int WINAPI ShortSend(SOCKET, LPWSABUF buffer, DWORD, LPDWORD sent, DWORD,
                            LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE)
{
    *sent = buffer->len > 7 ? 7 : buffer->len;
    return 0;
}
static int WINAPI BlockedSend(SOCKET, LPWSABUF, DWORD, LPDWORD, DWORD,
                              LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE)
{
    WSASetLastError(WSAEWOULDBLOCK);
    return SOCKET_ERROR;
}

int main()
{
    WSADATA data{};
    if (WSAStartup(MAKEWORD(2,2), &data)) return 1;
    std::string framing;
    if (!gatewire::AppendFrame(framing,"abcdefghijklmnop",16,"XYZ",4)
        || framing != "#abcdefghijklmnopXYZ!") return 12;
    framing.clear();
    if (!gatewire::AppendFrame(framing,"abcdefghijklmnop",16,"",0)
        || framing != "#abcdefghijklmnop!") return 13;
    framing.clear();
    if (!gatewire::AppendFrame(framing,"",0,"+LNG",4)
        || framing != "#+LNG!") return 14;
    SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in address{}; address.sin_family=AF_INET; address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    if (bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) || listen(listener,1)) return 2;
    int addressSize=sizeof(address);
    getsockname(listener,reinterpret_cast<sockaddr*>(&address),&addressSize);
    SOCKET sender=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (connect(sender,reinterpret_cast<sockaddr*>(&address),sizeof(address))) return 3;
    SOCKET receiver=accept(listener,nullptr,nullptr);
    closesocket(listener);
    int sendSize=1024;
    setsockopt(sender,SOL_SOCKET,SO_SNDBUF,reinterpret_cast<char*>(&sendSize),sizeof(sendSize));
    u_long nonblocking=1;
    ioctlsocket(sender,FIONBIO,&nonblocking); ioctlsocket(receiver,FIONBIO,&nonblocking);
    std::string pending;
    const std::string frame="#abcdefghijklmnopXYZ!";
    for (int i=0;i<20000;++i)
        if (!gatewire::AppendFrame(pending,"abcdefghijklmnop",16,"XYZ",3)) return 4;
    const std::string expected=pending;
    if (expected.size()!=frame.size()*20000 || expected.substr(0,frame.size())!=frame) return 5;
    std::string full(gatewire::MaxPendingBytes,'x');
    if (gatewire::AppendFrame(full,"",0,"",0) || full.size()!=gatewire::MaxPendingBytes) return 6;
    std::string controlled=expected;
    if (gatewire::SendPending(INVALID_SOCKET,controlled,ShortSend)
        || controlled!=expected.substr(7)) return 7;
    const std::string remainder=controlled;
    if (gatewire::SendPending(INVALID_SOCKET,controlled,BlockedSend)
        || controlled!=remainder) return 8;
    std::string received;
    const ULONGLONG deadline=GetTickCount64()+10000;
    while (received.size()<expected.size() && GetTickCount64()<deadline) {
        if (gatewire::SendPending(sender,pending)) return 9;
        char buffer[8192]; const int count=recv(receiver,buffer,sizeof(buffer),0);
        if (count>0) received.append(buffer,count);
        else if (count==SOCKET_ERROR && WSAGetLastError()==WSAEWOULDBLOCK) Sleep(1);
        else return 10;
    }
    closesocket(sender); closesocket(receiver); WSACleanup();
    if (received!=expected || !pending.empty()) return 11;
    std::printf("PASS: 20000 frames, %zu bytes, TCP verified; controlled partial send/backpressure preserved\n",received.size());
    return 0;
}
