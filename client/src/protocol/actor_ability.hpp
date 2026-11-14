#pragma once
#include "receive_message.hpp"
#include "message_ids.hpp"
#include <algorithm>
namespace mir3 {
// 字段顺序参考Mir3EIClient/GameProcess/Actor.h的ACTORABILITY；1.45长度由42F5A0确认。
// 不使用C结构体内存布局：线上字段紧凑排列，核心35字节，之后是七组双字节元素攻击和两组七字节元素属性。
struct ActorAbility {
    std::uint32_t gold=0,experience=0,next_experience=0;
    std::uint8_t profession=0,level=0;
    std::array<int,2> defence{},magic_defence{},attack{},magic{},spirit{};
    std::uint16_t hp=0,mp=0,max_hp=0,max_mp=0,weight=0,max_weight=0;
    std::array<unsigned,2> wear{},hand{};
    std::array<std::uint8_t,28> extension{};
    std::array<std::uint8_t,7> element_attack{},element_strength{},element_weakness{};
    std::array<std::uint16_t,7> element_attack_words{};
    std::array<std::uint8_t,4> header_options{};
    static std::optional<ActorAbility> parse(const ReceiveHeader& header,std::span<const std::uint8_t> encoded) {
        if (header.opcode!=protocol::SM_ABILITY || (header.parameters[0]&255)>2) return {};
        const auto decoded=decode_six_bit(encoded.data(),encoded.size());
        if (!decoded || decoded->size()!=63) return {};
        const auto byte=[&](int at) { return std::uint8_t((*decoded)[at]); };
        const auto word=[&](int at) { return std::uint16_t(byte(at)|(unsigned(byte(at+1))<<8)); };
        const auto dword=[&](int at) { return std::uint32_t(word(at))|(std::uint32_t(word(at+2))<<16); };
        const auto range=[&](int at) { return std::array<int,2>{byte(at),byte(at+1)}; };
        ActorAbility value;value.gold=header.recognition;value.profession=std::uint8_t(header.parameters[0]);value.level=byte(0);
        value.defence=range(1);value.magic_defence=range(3);value.attack=range(5);value.magic=range(7);value.spirit=range(9);
        value.hp=word(11);value.mp=word(13);value.max_hp=word(15);value.max_mp=word(17);
        value.experience=dword(19);value.next_experience=dword(23);value.weight=word(27);value.max_weight=word(29);
        value.wear={byte(31),byte(32)};value.hand={byte(33),byte(34)};
        for (int i=0;i<28;++i) value.extension[i]=byte(35+i);
        for (int i=0;i<7;++i) {
            value.element_attack_words[i]=word(35+2*i);
            value.element_attack[i]=byte(36+2*i);value.element_strength[i]=byte(49+i);value.element_weakness[i]=byte(56+i);
        }
        value.header_options={std::uint8_t(header.parameters[1]),std::uint8_t((header.parameters[1]>>8)*16),
            std::uint8_t(header.parameters[2]*16),std::uint8_t((header.parameters[2]>>8)*16)};
        return value;
    }
};
}
