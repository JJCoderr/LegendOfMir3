#pragma once
#include "skill_ids.hpp"
#include <algorithm>

namespace mir3 {
struct SpellLight {

    static constexpr bool fades(SkillId skill) {
        switch (skill) {
        case SkillId::earthfire: case SkillId::holyshield: case SkillId::fire:
        case SkillId::sinsu: case SkillId::lightening: case SkillId::shootlighten:
        case SkillId::original_53: case SkillId::original_63:
        case SkillId::original_104: case SkillId::original_110: return true;
        default: return false;
        }
    }
    static constexpr int channel(int original,int decrement) {
        if (!decrement) return original;

        const int start=original<2?original+256:original;
        return std::max(2,start-decrement);
    }
};
}
