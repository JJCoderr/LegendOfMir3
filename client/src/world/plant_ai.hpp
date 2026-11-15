#pragma once
#include "world/monster_pursuit.hpp"

namespace mir3 {
// LegendOfMir3_Src CStickObject::Run 的显隐状态。
// 世界层保留此状态；客户端收到钻回消息后删除显示对象不等于服务端怪物死亡。
struct PlantAi {
    enum class Action { idle, emerge, retreat, combat };
    bool hidden=true;
    std::uint64_t walked_at=0;
    unsigned walk_interval_ms=600; // 离线夹具值，实际值由怪物数据库提供。
    std::optional<int> target_id;
    std::uint64_t next_hit=1200;
    unsigned attack_interval_ms=1200;
    int facing=0;

    const MonsterAiTarget* target(std::span<const MonsterAiTarget> visible) const;

    bool fight(std::uint64_t now,int x,int y,std::span<const MonsterAiTarget> visible,const MonsterPursuit::Attack& attack);

    Action tick(std::uint64_t now,int x,int y,bool dead,bool ghost,
        std::span<const MonsterAiTarget> visible,const MonsterAiTarget* target);
};
}
