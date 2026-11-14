// 测试内容：原版各聊天通道的日志追加次数、开关不去重、GBK原字节、空文本与NUL终止和非聊天消息排除。
#pragma once
#include "chat/chat_log.hpp"
#include <stdexcept>
inline void chat_log_checks() {
    using namespace mir3;using namespace mir3::protocol;
    auto check=[](bool condition){if(!condition) throw std::runtime_error("original chat log");};
    for(int opcode:{SM_HEAR,SM_GROUPMESSAGE,SM_CRY,SM_WHISPER,SM_MONSTERSAY}) {
        check(ChatLog::payload(opcode,false,"message")=="message\r\n");
        check(ChatLog::payload(opcode,true,"message")=="message\r\nmessage\r\n");
    }
    check(ChatLog::payload(SM_SYSMESSAGE,false,"system")=="system\r\n");
    check(ChatLog::payload(SM_SYSMESSAGE,true,"system")=="system\r\n");
    check(ChatLog::payload(SM_GUILDMESSAGE,false,"guild").empty());
    check(ChatLog::payload(SM_GUILDMESSAGE,true,"guild")=="guild\r\n");
    check(ChatLog::payload(CM_SAY,true,"outgoing").empty());
    check(ChatLog::payload(SM_SYSMESSAGE,false,"")=="\r\n");
    check(ChatLog::payload(SM_SYSMESSAGE,false,std::string_view("a\0b",3))=="a\r\n");
    const std::string chinese="\xbc\xaf\xba\xcf";
    check(ChatLog::payload(SM_WHISPER,false,chinese)==chinese+"\r\n");
}
