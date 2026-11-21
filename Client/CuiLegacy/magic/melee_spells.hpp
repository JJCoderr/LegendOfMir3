#pragma once
#include "skill_ids.hpp"
#include "hud/human_actions.hpp"
#include <string_view>

namespace mir3 {
// Consume one-shot readiness only when an attack is sent.
struct MeleeSpells {
    enum Opcode : std::uint16_t { hit=3014, firehit=3025, backhit=3032, randhit=3031, longhit=3019, powerhit=3018, widehit=3024, widehit2=3033, forcehit=3015 };
    bool fire=false,back=false,round=false,power=false,long_hit=false,wide=false,wide2=false;
    struct Attack { int skill=0,opcode=hit,action=9,facing=0,sound=0; };
    static int frame_sound(int skill,int type) {
        switch (SkillId(skill)) {
        case SkillId::firesword:return 137;
        case SkillId::randswing:return 140;
        case SkillId::spacemove2:return 141;
        case SkillId::yedo:return type?131:130;
        case SkillId::ergum:return 132;
        case SkillId::banwol:case SkillId::original_103:return 133;
        default:return 0;
        }
    }
    bool receive(std::string_view text) {
        if (!text.starts_with('+')) return false;
        text.remove_prefix(1);
        if (text.starts_with("ULNG")) long_hit=false;
        else if (text.starts_with("LNG")) long_hit=true;
        else if (text.starts_with("UWID")) wide=false;
        else if (text.starts_with("WID")) {wide=true;wide2=false;}
        else if (text.starts_with("UMAN")) wide2=false;
        else if (text.starts_with("MAN")) {wide2=true;wide=false;}
        else if (text.starts_with("UFIR")) fire=false;
        else if (text.starts_with("FIR")) fire=true;
        else if (text.starts_with("URND")) round=false;
        else if (text.starts_with("RND")) round=true;
        else if (text.starts_with("UBKS")) back=false;
        else if (text.starts_with("BKS")) back=true;
        else if (text.starts_with("PWR")) power=true;
        else return false;
        return true;
    }
    Attack select(int weapon,int facing,int mp,bool distant_actor,int type) {
        Attack a{0,hit,HumanActions::weapon_attack(weapon),facing,0};
        if (fire && mp>7) {fire=false;a.skill=int(SkillId::firesword);a.opcode=firehit;a.sound=137;}
        else if (back && mp>11) {back=false;a.skill=int(SkillId::spacemove2);a.opcode=backhit;a.action=16;a.sound=141;}
        else if (round && mp>15) {round=false;a.skill=int(SkillId::randswing);a.opcode=randhit;a.action=17;a.sound=140;}
        else if (long_hit && distant_actor) {a.skill=int(SkillId::ergum);a.opcode=longhit;a.sound=132;}
        else if (power) {power=false;a.skill=int(SkillId::yedo);a.opcode=powerhit;a.sound=type?131:130;}
        else if (wide && mp>3) {a.skill=int(SkillId::banwol);a.opcode=widehit;a.action=HumanActions::weapon_attack(weapon,false);a.sound=133;}
        else if (wide2 && mp>7) {a.skill=int(SkillId::original_103);a.opcode=widehit2;a.action=HumanActions::weapon_attack(weapon,false);a.facing=0;a.sound=133;}
        return a;
    }
};
}
