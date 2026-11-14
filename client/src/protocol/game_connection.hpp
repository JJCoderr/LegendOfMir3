#pragma once
#include "protocol/receive_message.hpp"
namespace mir3 {
struct GameConnection {
    std::array<std::uint8_t,4> key{};
    std::uint16_t secondary=0;
    static std::uint16_t challenge_word(std::string_view block) {
        if (block.size()!=20) return 0;
        std::uint16_t bits=0;
        for (int group=0;group<2;++group) {
            const int at=group*10;
            const unsigned threshold=std::uint8_t(block[at])+std::uint8_t(block[at+9]);
            for (int i=0;i<8;++i) if (std::uint8_t(block[at+i+1])>=threshold) bits|=0x8000>>(group*8+i);
        }
        return std::uint16_t(((bits&255)^0x87)<<8)|std::uint16_t((bits>>8)^0x87);
    }
    bool change_key(std::string_view body) {
        if (body.size()!=60 || body.find(char(0))!=std::string_view::npos) return false;
        const auto recognition=challenge_word(body.substr(0,20));
        const auto opcode=challenge_word(body.substr(40,20));
        key={std::uint8_t(opcode),std::uint8_t(opcode>>8),std::uint8_t(recognition),std::uint8_t(recognition>>8)};
        secondary=challenge_word(body.substr(20,20));return true;
    }
    // 0x430230：四个原始身份字节各占10字节，随机区间两端均包含。
    template<class Random> static std::string challenge_response(std::uint32_t actor,Random random) {
        std::string result(40,char(0));
        for (int byte=0;byte<4;++byte) {
            const unsigned value=((actor>>(8*byte))&255)^0x87;
            const int base=random(0,49)+10,threshold=random(0,19)+10,at=byte*10;
            result[at]=char(base);result[at+9]=char(threshold);
            for (int bit=0;bit<8;++bit) result[at+1+bit]=char(base+random(0,threshold-1)+((value&(128>>bit))?threshold:0));
        }
        return result;
    }
};
}
