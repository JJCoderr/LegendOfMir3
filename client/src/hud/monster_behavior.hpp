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

    static MonsterBehavior for_shape(int shape);
    bool dormant(std::uint32_t flags) const;
    static int attack_message(int shape);
};
}
