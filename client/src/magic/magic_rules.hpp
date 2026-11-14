#pragma once
#include "skill_ids.hpp"

namespace mir3 {
enum class MagicEffectId : int { fire_wave=9, ice_wave=53, three_rays=110, lightning_trail=336 };
enum class MagicDirection { sixteen, eight, fixed };
// 4495D0：是否循环、是否位于目标上、是否保留目标引用是三个独立字段。
struct MagicRules {
    bool repeat=false;
    bool at_target=true;
    bool track_target=true;
    bool light=true;
    bool reject_same_cell=false;
    bool untargeted_timeout=false;
    bool use_source_as_target=false;
    MagicDirection direction=MagicDirection::sixteen;

    static constexpr MagicRules for_skill(SkillId skill) {
        MagicRules r;
        switch (skill) {
        case SkillId::fireball: case SkillId::fireball2: case SkillId::firecharm:
        case SkillId::original_39: case SkillId::original_41: case SkillId::original_67:
        case SkillId::original_113: case SkillId::original_150:
            r.repeat=true;r.at_target=false;break;
        case SkillId::original_37: case SkillId::original_38: case SkillId::original_40:
            r.repeat=true;r.at_target=false;r.direction=MagicDirection::fixed;break;
        case SkillId::hangmajinbub: case SkillId::dejiwonho: case SkillId::bigcloak:
        case SkillId::original_85: case SkillId::original_86: case SkillId::original_87:
        case SkillId::original_88: case SkillId::original_89: case SkillId::original_90:
        case SkillId::original_91: case SkillId::original_92: case SkillId::original_93:
        case SkillId::original_94: case SkillId::original_95: case SkillId::original_96:
        case SkillId::original_97: case SkillId::original_98: case SkillId::original_99:
        case SkillId::original_100: case SkillId::original_101:
            r.repeat=true;r.at_target=false;r.track_target=false;break;
        case SkillId::shootlighten:
            r.direction=MagicDirection::eight;r.track_target=false;r.use_source_as_target=true;break;
        case SkillId::fire: case SkillId::original_53:
        case SkillId::lightening: case SkillId::holyshield: case SkillId::earthfire:
        case SkillId::fireboom: case SkillId::lightflower: case SkillId::bighealling:
        case SkillId::snowwind: case SkillId::original_47: case SkillId::original_48:
        case SkillId::original_63: case SkillId::original_64: case SkillId::original_66:
        case SkillId::original_72: case SkillId::original_73: case SkillId::original_74:
        case SkillId::original_77: case SkillId::original_111:
            r.direction=MagicDirection::fixed;r.track_target=false;break;
        case SkillId::healling: case SkillId::amyounsul: case SkillId::skelleton:
        case SkillId::tamming: case SkillId::mootebo: case SkillId::showhp:
        case SkillId::sinsu: case SkillId::shield: case SkillId::killundead:
        case SkillId::original_104: case SkillId::original_105: case SkillId::original_107:
        case SkillId::original_120: case SkillId::original_122: case SkillId::original_123:
            r.direction=MagicDirection::fixed;break;
        case SkillId::original_102: case SkillId::original_106: case SkillId::original_112:
            r.direction=MagicDirection::fixed;r.light=false;break;
        case SkillId::original_110:
            r.direction=MagicDirection::fixed;r.at_target=false;r.track_target=false;break;
        default:break;
        }
        switch (skill) {
        case SkillId::firecharm: case SkillId::fireball2: case SkillId::fireball:
        case SkillId::original_37: case SkillId::original_39: case SkillId::original_40:
        case SkillId::original_41: case SkillId::original_67:
            r.reject_same_cell=true;r.untargeted_timeout=true;break;
        case SkillId::original_150:r.reject_same_cell=true;break;
        case SkillId::original_113:r.untargeted_timeout=true;break;
        default:break;
        }
        return r;
    }
};
}
