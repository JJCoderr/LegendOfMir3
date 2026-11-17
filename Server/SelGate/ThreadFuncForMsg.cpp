#include "stdafx.h"
#include "../Def/GateFrames.h"

extern CWHQueue g_xMsgQueue;
extern BOOL g_fTerminated;

DWORD WINAPI ThreadFuncForMsg(LPVOID lpParameter)
{
    GateFrames frames;
    while (!g_fTerminated) {
        const int count=g_xMsgQueue.GetCount();
        for (int i=0;i<count;++i) {
            char* data=(char*)g_xMsgQueue.PopQ();
            if (!data) continue;
            // The server pads wire frames with NUL bytes. Preserve the received byte
            // count so coalesced frames and fragments after padding survive.
            frames.Append(data+sizeof(unsigned),*reinterpret_cast<unsigned*>(data),[](std::string frame) {
                if (frame.compare(0,2,"%+")==0) {
                    if (frame.compare(0,3,"%+-")==0) InsertLogMsg(_TEXT("Kick User."));
                    return;
                }
                const auto separator=frame.find('/');
                if (separator==std::string::npos) return;
                const SOCKET socket=SOCKET(AnsiStrToVal(&frame[1]));
                if (!socket) return;
                WSABUF buffer;
                buffer.buf=&frame[separator+1];
                buffer.len=ULONG(frame.size()-separator-1);
                DWORD sent=0;
                WSASend(socket,&buffer,1,&sent,0,NULL,NULL);
            });
            delete[] data;
        }
        SleepEx(1,TRUE);
    }
    return 0;
}
