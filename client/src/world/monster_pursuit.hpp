#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <span>
#include <functional>

namespace mir3 {
struct MonsterAiTarget {
    int x=0,y=0;
    bool alive=true,proper=true,inspector=false;
    int id=0;
    bool hidden=false;
};

// ObjectMonster.cpp: MonsterNormalAttack / AttackTarget / GotoTargetXY.
// 输入必须是同地图可见对象。攻击回调负责提交动作。
struct MonsterPursuit {
    enum class Action { idle, walk, attack, blocked };
    using Time=std::uint64_t;
    using Walk=std::function<bool(int,int,int)>;
    using Attack=std::function<bool(int,const MonsterAiTarget&)>;
    using Random=std::function<unsigned(unsigned)>;
    std::optional<int> target_id;
    Time focused_at=0,searched_at=0,next_hit=0;
    unsigned attack_interval_ms=1200;
    unsigned burst_limit=0, burst_shots=0;
    int facing=0;
    static constexpr std::array<std::array<int,2>,8> directions{{
        {0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}}};
    static bool eligible(const MonsterAiTarget& target);
    static std::int64_t distance(int x,int y,const MonsterAiTarget& target);
    static int direction(int x,int y,int tx,int ty);
    Action tick(Time now,int x,int y,std::span<const MonsterAiTarget> visible,const Walk& walk,const Attack& attack,const Random& random);
};
}
