#pragma once
#include <array>
#include "damage_numbers.hpp"
#include <cstdint>
namespace mir3 {
// 0x408281/0x410A00, registry 0x428110: health and registration are separate.
struct SceneHealth {
    DamageNumbers damage;
    std::uint16_t current=0, mana=0, maximum=0;
    std::array<std::uint16_t,3> registration_fields{};
    std::uint32_t status_tick=0, saved_tick=0;
    bool registered=false, excluded=false, raised=false;
    double age_ms=0;
    static std::uint8_t percent(unsigned current,unsigned maximum) {
        return static_cast<std::uint8_t>(static_cast<unsigned>(maximum?double(current)/maximum*100.0:0));
    }
    static int width(std::uint8_t value) { return int(double(value)*double(0.01f)*36.0); }
    bool apply(int opcode,int hp,int mp,int max_hp) {
        if ((opcode!=53 && opcode!=1312) || hp<0 || hp>65535 || mp<0 || mp>65535 || max_hp<0 || max_hp>65535) return false;
        if (opcode==53) { current=hp;mana=mp;maximum=max_hp; }
        else { registration_fields={std::uint16_t(hp),std::uint16_t(mp),std::uint16_t(max_hp)};registered=true;age_ms=0; }
        return true;
    }
    bool visible(bool local=false) const { return (local || (registered && age_ms<5000)) && status_tick==saved_tick && !excluded; }
    void advance(double ms) { if (registered) { age_ms+=ms;if(age_ms>=5000) registered=false; } }
};
}
