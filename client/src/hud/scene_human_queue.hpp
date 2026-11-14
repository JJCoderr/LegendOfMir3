#pragma once
#include "protocol/message_ids.hpp"
#include <cstdint>
#include <deque>
#include <optional>
#include <array>
#include <vector>
#include "human_appearance.hpp"
#include "protocol/spell_reply.hpp"
namespace mir3 {
// Decoded subset of the original actor queue (0x481A30 / 0x413F90).
// Wire header/body decoding and appearance updates remain in the transport layer.
struct SceneHumanMessage {
    int opcode=0,x=0,y=0,facing=0,hit_weapon=0;
    std::uint32_t flags=0;
    std::optional<HumanAppearance> appearance;
    std::optional<std::array<std::uint16_t,3>> health;
    std::uint32_t hit_source=0;
    int light_attribute=-1;
    bool harvest=false;
    std::optional<std::int16_t> status_speed;
    std::optional<std::uint16_t> spell;
    std::optional<SpellReply> spell_reply;
    std::uint8_t weapon_element=7;
    std::optional<std::string> transfer_name;
    std::optional<std::uint16_t> hit_variant;
    static bool accepts(int opcode) {
        switch (opcode) {
        case mir3::protocol::SM_SITDOWN: case mir3::protocol::SM_BUTCH: case mir3::protocol::SM_SPELL:
        case mir3::protocol::SM_MAGICFIRE: case mir3::protocol::SM_MAGICFIRE_LIST:
        case mir3::protocol::SM_BACKSTEP: case mir3::protocol::SM_DIGUP:
        case mir3::protocol::SM_27: case mir3::protocol::SM_SKELETON:
        case mir3::protocol::SM_800: case mir3::protocol::SM_806:
        case mir3::protocol::SM_801: case mir3::protocol::SM_ACTOR_STATUS: case mir3::protocol::SM_808:
        case mir3::protocol::SM_STRUCK: case mir3::protocol::SM_BUMP: case mir3::protocol::SM_TURN: case mir3::protocol::SM_WALK: case mir3::protocol::SM_RUN: case mir3::protocol::SM_HIT: case mir3::protocol::SM_HIT2: case mir3::protocol::SM_DEATH: case mir3::protocol::SM_NOWDEATH: case mir3::protocol::SM_REVIVE: case mir3::protocol::SM_REVIVE_REFRESH:
        case mir3::protocol::SM_HUMAN_CAST: case mir3::protocol::SM_HUMAN_CAST_ALT: case mir3::protocol::SM_HUMAN_ACTION18: case mir3::protocol::SM_HUMAN_STRUCK: case mir3::protocol::SM_HUMAN_ATTACK: return true;
        case mir3::protocol::SM_FIREHIT: case mir3::protocol::SM_POWERHIT:
        case mir3::protocol::SM_LONGHIT: case mir3::protocol::SM_WIDEHIT: case mir3::protocol::SM_WIDEHIT2: return true;
        default: return false;
        }
    }
};
class SceneHumanQueue {
public:
    bool push(SceneHumanMessage message) {
        if (!SceneHumanMessage::accepts(message.opcode) || message.facing<0 || message.facing>=8 ||
            message.hit_weapon<0 || message.hit_weapon>=50 || pending.size()>=99999) return false;
        pending.push_back(message);
        return true;
    }
    bool catching_up() const { return pending.size()>=3; }
    std::size_t size() const { return pending.size(); }
    std::vector<SceneHumanMessage> fold_health() {
        std::vector<SceneHumanMessage> updates;
        if (pending.size()<2) return updates;
        for (auto it=pending.begin();it!=pending.end();) {
            if (it->opcode==mir3::protocol::SM_STRUCK) { updates.push_back(*it);it=pending.erase(it); }
            else ++it;
        }
        return updates;
    }
    std::optional<SceneHumanMessage> take() {
        if (pending.empty()) return {};
        const auto message=pending.front();
        pending.pop_front();
        return message;
    }
private:
    std::deque<SceneHumanMessage> pending;
};
}
