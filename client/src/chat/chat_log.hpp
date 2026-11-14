#pragma once
#include "scene_chat.hpp"
#include <string>
#include <string_view>
namespace mir3 {
// 42F380有两个独立追加分支；不能把它化简为一个开关。
struct ChatLog {
    static constexpr int max_encoded_text=1024;
    static int repetitions(int opcode,bool enabled) {
        if(!chat_opcode(opcode)) return 0;
        return (enabled && opcode!=protocol::SM_SYSMESSAGE?1:0)+(opcode!=protocol::SM_GUILDMESSAGE?1:0);
    }
    static std::string payload(int opcode,bool enabled,std::string_view ansi) {
        ansi=ansi.substr(0,ansi.find(char(0)));
        std::string result;
        for(int i=0;i<repetitions(opcode,enabled);++i) {result.append(ansi);result+="\r\n";}
        return result;
    }
};
}
