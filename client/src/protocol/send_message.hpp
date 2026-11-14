#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
namespace mir3 {
// 0x47D8B0 / 0x47C730: one sequence for all messages on the connection.
struct SendMessages {
    unsigned sequence=0;
    std::string packet(std::uint16_t opcode,std::uint32_t recognition=0,
        std::uint32_t instance=0,std::uint16_t quantity=0,std::string_view body={}) {
        std::array<std::uint8_t,12> header{};
        for (unsigned i=0;i<4;++i) header[i]=recognition>>(8*i);
        header[4]=opcode&255;header[5]=opcode>>8;
        for (unsigned i=0;i<4;++i) header[6+i]=instance>>(8*i);
        header[10]=quantity&255;header[11]=quantity>>8;
        sequence=sequence%9+1;
        std::string result{"#"};result+=char('0'+sequence);
        const auto encode=[&](std::string_view bytes) {
            unsigned accumulator=0;int bits=0;
            for (unsigned char byte:bytes) {
                accumulator=(accumulator<<8)|byte;bits+=8;
                while (bits>=6) { bits-=6;result+=char(60+((accumulator>>bits)&63)); }
            }
            if (bits) result+=char(60+((accumulator<<(6-bits))&63));
        };
        encode({reinterpret_cast<const char*>(header.data()),header.size()});
        encode(body.substr(0,body.find(char(0))));
        result+='!';return result;
    }
};
}
