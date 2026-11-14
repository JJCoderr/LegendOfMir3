#pragma once
#include "protocol/message_ids.hpp"
#include "hud/login_flow.hpp"
#include <charconv>
namespace mir3 {
// 0x4876D0：525只准备端点及角色；电影结束后才交接游戏连接。
struct GameEntry {
    bool pending=false;
    LoginEndpoint endpoint;
    std::string role;
    bool receive(std::span<const std::uint8_t> message,std::string_view selected_role) {
        const auto header=ReceiveHeader::decode_plain(message);
        if (!header || header->opcode!=mir3::protocol::SM_STARTPLAY) return false;
        const auto body=decode_six_bit(message.data()+16,message.size()-16);
        if (!body || body->size()>255 || body->find(char(0))!=std::string::npos) return false;
        const auto slash=body->find('/');
        if (slash==std::string::npos || slash==0 || slash>15) return false;
        auto port_text=std::string_view(*body).substr(slash+1);
        // 原版atoi允许端口后的斜杠；同时接受不带尾分隔符的回包。
        port_text=port_text.substr(0,port_text.find('/'));
        unsigned port=0;
        const auto parsed=std::from_chars(port_text.data(),port_text.data()+port_text.size(),port);
        if (parsed.ec!=std::errc{} || parsed.ptr!=port_text.data()+port_text.size() || port==0 || port>65535) return false;
        endpoint={body->substr(0,slash),std::uint16_t(port)};
        if (!selected_role.empty()) role=selected_role;
        pending=true;return true;
    }
    bool finish_transition() {
        if (!pending) return false;
        pending=false;return true;
    }
};
}
