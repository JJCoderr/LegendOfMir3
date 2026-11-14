#pragma once
#include "protocol/message_ids.hpp"
#include "protocol/receive_message.hpp"
#include "hud/human_appearance.hpp"
#include <algorithm>
namespace mir3 {
struct LoginActor {
    std::uint32_t identity=0;
    int x=0,y=0,facing=0,light=0;
    HumanAppearance appearance;
    std::array<std::uint8_t,16> extra{};
    // 0x42F190：三个独立编码段，不能整体六位解码后按字节切分。
    static std::optional<LoginActor> parse(const ReceiveHeader& header,std::span<const std::uint8_t> encoded) {
        if (header.opcode!=mir3::protocol::SM_LOGON || !header.recognition || header.recognition>INT32_MAX || encoded.size()!=51) return {};
        const auto feature=decode_six_bit(encoded.data(),22);
        const auto extension=decode_six_bit(encoded.data()+22,7);
        const auto extra=decode_six_bit(encoded.data()+29,22);
        if (!feature || !extension || !extra || feature->size()!=16 || extension->size()!=5 || extra->size()!=16) return {};
        const auto combined=*feature+*extension;
        const auto appearance=HumanAppearance::parse({reinterpret_cast<const std::uint8_t*>(combined.data()),combined.size()},false);
        if (!appearance || appearance->profession>2 || (header.parameters[2]&255)>=8) return {};
        LoginActor actor;actor.identity=header.recognition;actor.x=header.parameters[0];actor.y=header.parameters[1];
        actor.facing=header.parameters[2]&255;actor.light=header.parameters[2]>>8;actor.appearance=*appearance;
        std::copy(extra->begin(),extra->end(),actor.extra.begin());return actor;
    }
};
}
