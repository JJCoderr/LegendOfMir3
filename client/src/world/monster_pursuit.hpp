#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <span>

namespace mir3 {
struct MonsterAiTarget {
    int x=0,y=0;
    bool alive=true,proper=true,inspector=false;
    int id=0;
    bool hidden=false;
};

// ObjectMonster.cpp: MonsterNormalAttack / AttackTarget / GotoTargetXY.
// 输入必须是同地图可见对象；此处只判定追踪/近战，不结算伤害。
struct MonsterPursuit {
    enum class Action { idle, walk, attack, blocked };
    using Time=std::uint64_t;
    std::optional<int> target_id;
    Time focused_at=0,searched_at=0,next_hit=0;
    unsigned attack_interval_ms=1200;
    int facing=0;
    static constexpr std::array<std::array<int,2>,8> directions{{
        {0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}}};
    static bool eligible(const MonsterAiTarget& target) {
        return target.alive && target.proper && !target.inspector;
    }
    static std::int64_t distance(int x,int y,const MonsterAiTarget& target) {
        return std::abs(std::int64_t(x)-target.x)+std::abs(std::int64_t(y)-target.y);
    }
    static int direction(int x,int y,int tx,int ty) {
        const int dx=(tx>x)-(tx<x),dy=(ty>y)-(ty<y);
        for (int index=0;index<8;++index)
            if (directions[index][0]==dx && directions[index][1]==dy) return index;
        return -1;
    }
    template<class Walk,class Attack,class Random>
    Action tick(Time now,int x,int y,std::span<const MonsterAiTarget> visible,Walk&& walk,Attack&& attack,Random&& random) {
        const MonsterAiTarget* target=nullptr;
        if (target_id) {
            for (const auto& candidate:visible) if(candidate.id==*target_id) {target=&candidate;break;}
            if (!target || !eligible(*target) || now-focused_at>30000 ||
                std::abs(std::int64_t(x)-target->x)>15 || std::abs(std::int64_t(y)-target->y)>15) {
                target=nullptr;target_id.reset();
            }
        }
        // 参考实现按视野列表顺序选第一个，并非每帧寻找最近目标。
        if (now-searched_at>8000 || (!target && now-searched_at>1000)) {
            searched_at=now;
            for (const auto& candidate:visible) {
                if (!eligible(candidate) || candidate.hidden || distance(x,y,candidate)>=999) continue;
                target=&candidate;target_id=candidate.id;focused_at=now;break;
            }
        }
        if (!target) return Action::idle;
        const int dir=direction(x,y,target->x,target->y);
        if (dir<0) return Action::idle; // 同格不在近战攻击范围内。
        if (std::abs(std::int64_t(x)-target->x)<=1 && std::abs(std::int64_t(y)-target->y)<=1) {
            if (now<=next_hit) return Action::idle; // 等攻击冷却时不能继续向目标格走。
            if (!attack(dir,*target)) return Action::blocked;
            facing=dir;next_hit=now+attack_interval_ms;focused_at=now;
            return Action::attack;
        }
        // GotoTargetXY只查看视野列表首项；较近时切换目标，本轮不移动。
        if (!visible.empty() && eligible(visible.front()) &&
            distance(x,y,visible.front())<distance(x,y,*target)) {
            target_id=visible.front().id;focused_at=now;return Action::idle;
        }
        int candidate=dir;
        for (int attempt=0;attempt<8;++attempt) {
            if (walk(x+directions[candidate][0],y+directions[candidate][1],candidate)) {
                facing=candidate;return Action::walk;
            }
            if (attempt<7) candidate=(candidate+(random(3)?1:7))%8;
        }
        return Action::blocked;
    }
};
}
