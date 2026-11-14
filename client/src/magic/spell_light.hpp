#pragma once
#include "skill_ids.hpp"
#include <algorithm>

namespace mir3 {
struct SpellLight {
    // 44A110：这些玩家法术结束后减颜色，其他法术按50ms节奏收缩半径。
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
        // 原版BYTE先减2再比较；0/1会发生字节回绕，不能直接饱和减法。
        const int start=original<2?original+256:original;
        return std::max(2,start-decrement);
    }
};
}
