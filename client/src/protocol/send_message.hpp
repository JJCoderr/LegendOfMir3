#pragma once
#include "default_message.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <string>
#include <string_view>
namespace mir3 {
// 0x47D8B0 / 0x47C730: one sequence for all messages on the connection.
struct SendMessages {
    unsigned sequence=0;
    std::string packet(std::uint16_t opcode,std::uint32_t recognition=0,
        std::uint32_t instance=0,std::uint16_t quantity=0,std::string_view body={}) {
        const auto header=protocol::DefaultMessage{recognition,opcode,
            {std::uint16_t(instance),std::uint16_t(instance>>16),quantity}}.encode();
        sequence=sequence%9+1;
        std::string result{"#"};result+=char('0'+sequence);
        encode(result,{reinterpret_cast<const char*>(header.data()),header.size()});
        encode(result,body.substr(0,body.find(char(0))));
        result+='!';return result;
    }
    // 0x47BC80：游戏认证首包没有普通消息头，与后续请求共享序号。
    std::string game_authentication(std::string_view account,std::string_view role,std::int32_t session) {
        constexpr std::uint32_t version=0x0131a4f0;
        const auto number=std::bit_cast<std::uint32_t>(session);
        const auto signed_text=[](std::uint32_t value) { return std::to_string(std::bit_cast<std::int32_t>(value)); };
        std::string body="**";
        body+=account.substr(0,account.find(char(0)));body+='/';
        body+=role.substr(0,role.find(char(0)));body+='/';
        body+=signed_text(number^0x3eb2c5cc);body+='/';
        body+=signed_text(version^number^0x5580af27);body+='/';
        body+=signed_text(version^0xfa0280af);body+="/1";
        sequence=sequence%9+1;
        std::string result="#";result+=char('0'+sequence);encode(result,body);result+='!';return result;
    }
private:
    static void encode(std::string& result,std::string_view bytes) {
        unsigned accumulator=0;int bits=0;
        for (unsigned char byte:bytes) {
            accumulator=(accumulator<<8)|byte;bits+=8;
            while (bits>=6) { bits-=6;result+=char(60+((accumulator>>bits)&63)); }
        }
        if (bits) result+=char(60+((accumulator<<(6-bits))&63));
    }

};
}
