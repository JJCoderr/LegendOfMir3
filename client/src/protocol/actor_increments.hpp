#pragma once
#include "receive_message.hpp"
#include "message_ids.hpp"
namespace mir3 {
// 原版4282C0：仅覆盖当前负重，最大负重由SM_ABILITY维护。
struct ActorWeightChange {
    std::uint16_t weight=0;
    std::uint8_t wear=0,hand=0;
    static std::optional<ActorWeightChange> parse(const ReceiveHeader& header) {
        if (header.opcode!=protocol::SM_WEIGHTCHANGED) return {};
        return ActorWeightChange{std::uint16_t(header.recognition),std::uint8_t(header.parameters[0]),std::uint8_t(header.parameters[1])};
    }
};
// 参考Mir3EIClient的ACTORSUBABILITY；原版42F650确认每个头字段的有效位。
struct ActorSubAbility {
    std::uint16_t anti_magic=0;
    std::uint8_t hit_point=0,speed_point=0,anti_poison=0,poison_recover=0,health_recover=0,spell_recover=0;
    static std::optional<ActorSubAbility> parse(const ReceiveHeader& header) {
        if (header.opcode!=protocol::SM_SUBABILITY) return {};
        return ActorSubAbility{std::uint8_t(header.recognition),std::uint8_t(header.parameters[0]),std::uint8_t(header.parameters[0]>>8),
            std::uint8_t(header.parameters[1]),std::uint8_t(header.parameters[1]>>8),std::uint8_t(header.parameters[2]),std::uint8_t(header.parameters[2]>>8)};
    }
};
}
