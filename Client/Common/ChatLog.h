#pragma once
#include <cstdio>
#include <cstring>
#include <string>

// MIR3 42F380: the two append branches are independent.
struct CNativeChatLog {
    bool enabled=false;
    std::string path;
    static int Repetitions(int opcode,bool enabled) {
        const bool chat=opcode==SM_HEAR || opcode==SM_MONSTERSAY || (opcode>=SM_SYSMESSAGE && opcode<=SM_GUILDMESSAGE);
        if(!chat) return 0;
        return (enabled && opcode!=SM_SYSMESSAGE?1:0)+(opcode!=SM_GUILDMESSAGE?1:0);
    }
    void Append(int opcode,const char* text) const {
        if(path.empty()) return;
        const int count=Repetitions(opcode,enabled);if(!count) return;
        FILE* file=fopen(path.c_str(),"ab");if(!file) return;
        for(int i=0;i<count;++i) {fwrite(text,1,strlen(text),file);fwrite("\r\n",1,2,file);}
        fclose(file);
    }
};
inline CNativeChatLog& NativeChatLog() {static CNativeChatLog value;return value;}
