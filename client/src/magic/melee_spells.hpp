#pragma once
#include "skill_ids.hpp"
#include "protocol/message_ids.hpp"
#include "hud/human_actions.hpp"
#include <string_view>

namespace mir3 {
// 427C50文本状态与415F90挥击选择；蓄力在实际发起攻击时消费。
struct MeleeSpells {
    bool fire=false,back=false,round=false,power=false,long_hit=false,wide=false,wide2=false;
    struct Attack { int skill=0,opcode=protocol::CM_HIT,action=9,facing=0,sound=0; };
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
        Attack a{0,protocol::CM_HIT,HumanActions::weapon_attack(weapon),facing,0};
        if (fire && mp>7) {fire=false;a.skill=int(SkillId::firesword);a.opcode=protocol::CM_FIREHIT;a.sound=137;}
        else if (back && mp>11) {back=false;a.skill=int(SkillId::spacemove2);a.opcode=protocol::CM_BACKHIT;a.action=16;a.sound=141;}
        else if (round && mp>15) {round=false;a.skill=int(SkillId::randswing);a.opcode=protocol::CM_RANDHIT;a.action=17;a.sound=140;}
        else if (long_hit && distant_actor) {a.skill=int(SkillId::ergum);a.opcode=protocol::CM_LONGHIT;a.sound=132;}
        else if (power) {power=false;a.skill=int(SkillId::yedo);a.opcode=protocol::CM_POWERHIT;a.sound=type?131:130;}
        else if (wide && mp>3) {a.skill=int(SkillId::banwol);a.opcode=protocol::CM_WIDEHIT;a.action=HumanActions::weapon_attack(weapon,false);a.sound=133;}
        else if (wide2 && mp>7) {a.skill=int(SkillId::original_103);a.opcode=protocol::CM_WIDEHIT2;a.action=HumanActions::weapon_attack(weapon,false);a.facing=0;a.sound=133;}
        return a;
    }
};
}
