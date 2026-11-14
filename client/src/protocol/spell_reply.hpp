#pragma once
#include "receive_message.hpp"
#include "message_ids.hpp"
#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

namespace mir3 {
// 413150 / 4138C0：638正文为独立编码DWORD，1410为编码后的十进制目标列表。
struct SpellReply {
    std::uint16_t skill=0,x=0,y=0;
    bool multiple=false;
    std::vector<std::uint32_t> targets;
    static std::optional<SpellReply> parse(const ReceiveHeader& header,std::span<const std::uint8_t> encoded) {
        if (header.opcode!=protocol::SM_MAGICFIRE && header.opcode!=protocol::SM_MAGICFIRE_LIST) return {};
        const auto decoded=decode_six_bit(encoded.data(),encoded.size());
        if (!decoded) return {};
        SpellReply result;
        result.skill=header.parameters[2]&255;
        result.x=header.parameters[0];result.y=header.parameters[1];
        result.multiple=header.opcode==protocol::SM_MAGICFIRE_LIST;
        if (!result.multiple) {
            if (decoded->size()!=4) return {};
            std::uint32_t target=0;
            for (int i=0;i<4;++i) target|=std::uint32_t(std::uint8_t((*decoded)[i]))<<(8*i);
            result.targets.push_back(target);
        } else {
            if (decoded->size()>1024) return {};
            // 原版atoi允许空项/前导空白/符号，遇到NUL停止；不把编号当六位二进制。
            const std::string text=decoded->substr(0,decoded->find(char(0)));
            std::size_t start=0;
            for (;;) {
                const auto end=text.find('/',start);
                const auto token=text.substr(start,end==std::string::npos?end:end-start);
                const auto number=std::clamp(std::strtoll(token.c_str(),nullptr,10),-2147483648LL,2147483647LL);
                result.targets.push_back(static_cast<std::uint32_t>(number));
                if (end==std::string::npos) break;
                start=end+1;
            }
        }
        return result;
    }
};
}
