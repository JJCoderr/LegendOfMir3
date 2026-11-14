#pragma once
#include <cstdint>
#include "protocol/message_ids.hpp"

namespace mir3 {
// 4085A0/40E7F0：祖玛三类的特征状态位1不是普通颜色状态。
// 入场和动作结束时将动作8冻结在首帧；20号消息播放完整苏醒动作。
struct MonsterBehavior {
    enum class Birth { ordinary, dormant_statue };
    Birth birth=Birth::ordinary;
    int wake_frames=0;
    int wake_interval_ms=0;
    int wake_facing=-1;
    static constexpr std::uint32_t dormant_flag=1;

    static constexpr MonsterBehavior for_shape(int shape) {
        if (shape==83 || shape==84) return {Birth::dormant_statue,6,120,-1};
        if (shape==85) return {Birth::dormant_statue,20,120,0};
        return {};
    }
    constexpr bool dormant(std::uint32_t flags) const {
        return birth==Birth::dormant_statue && (flags&dormant_flag)!=0;
    }
    static constexpr int attack_message(int shape) {
        // 40B0A0 的外层 switch(shape-7)，内部方向分支不是外观号。
        switch (shape) {
        case 7: case 19: case 33: case 46: case 82: case 86: case 95: case 96:
        case 124: case 125: case 144: case 145: case 146: case 156: case 167:
        case 169: case 183: return protocol::SM_FLYAXE;
        default: return protocol::SM_HIT;
        }
    }
};
}
