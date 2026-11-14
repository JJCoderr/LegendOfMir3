#pragma once
#include "world/monster_pursuit.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <optional>
#include <span>
#include <vector>

namespace mir3 {
// Reference: ObjectMonster.cpp CScultureObject / CScultureKingObject.
// 数据库索引不是客户端shape，也不是ProcessMonster.cpp的行为分类编号。
enum class ZumaSlavePrototype : int { archer=143, fanatic=145, guardian=148, moth=126 };
enum class ZumaKind { statue, king };

class ZumaAi {
public:
    using Time=std::uint64_t;
    using Target=MonsterAiTarget;
    struct Actor {
        int id=0;
        ZumaKind kind=ZumaKind::statue;
        int x=0,y=0;
        bool stone=true, dead=false, ghost=false, petrified=false;
        int hp=100,max_hp=100;
        int danger=5;
        Time next_turn=0;
        unsigned walk_interval_ms=600;
        std::vector<int> slaves;
        MonsterPursuit pursuit;
    };
    struct Result {
        std::vector<int> woke;
        // 供通用追踪/移动/攻击层执行一次动作；此类不伪造伤害或寻路结果。
        std::vector<int> ready;
    };
    std::map<int,Actor> actors;

    static bool within(int x,int y,int other_x,int other_y,int radius) {
        return std::abs(std::int64_t(x)-other_x)<=radius && std::abs(std::int64_t(y)-other_y)<=radius;
    }

    // 每个实例是同一地图。visible(id)由世界的视野/敌我关系层提供。
    // spawn(owner, prototype)返回实际成功生成的对象ID；失败不占随从名额。
    // alive(id)包括死亡/幽灵检查。random(bound)必须返回[0,bound)。
    template<class Visible,class Spawn,class Alive,class Random>
    Result tick(Time now,Visible&& visible,Spawn&& spawn,Alive&& alive,Random&& random) {
        Result result;
        const auto wake=[&](Actor& actor) {
            actor.stone=false;
            result.woke.push_back(actor.id);
        };
        for (auto& [id,actor]:actors) {
            if (actor.dead || actor.ghost || actor.petrified) continue;
            if (now>actor.next_turn) {
                if (actor.stone) {
                    const auto targets=visible(id);
                    const bool near=std::any_of(targets.begin(),targets.end(),[&](const Target& target) {
                        return target.alive && target.proper && !target.inspector &&
                            within(actor.x,actor.y,target.x,target.y,2);
                    });
                    if (near) {
                        wake(actor);
                        if (actor.kind==ZumaKind::statue) {
                            // MeltStoneAll不递归，且只影响两种卫士，不唤醒教主。
                            // GetMapObject排除死亡/幽灵；不会排除石化中但仍活着的邻居。
                            for (auto& [other_id,other]:actors)
                                if (other.stone && !other.dead && !other.ghost && other.kind==ZumaKind::statue &&
                                    within(actor.x,actor.y,other.x,other.y,7)) wake(other);
                        }
                        actor.next_turn=now+2000+actor.walk_interval_ms;
                    }
                } else {
                    if (actor.kind==ZumaKind::king && actor.max_hp>0) {
                        // 保留参考C++整数除法次序，不暗改为(HP*5)/MaxHP。
                        if (actor.danger>0 && (actor.hp/actor.max_hp)*5<actor.danger) {
                            --actor.danger;
                            const auto count=6+random(6);
                            constexpr std::array prototypes{ZumaSlavePrototype::archer,ZumaSlavePrototype::fanatic,
                                ZumaSlavePrototype::guardian,ZumaSlavePrototype::moth};
                            for (unsigned index=0;index<count && actor.slaves.size()<30;++index) {
                                const auto spawned=spawn(actor,prototypes[random(4)]);
                                if (spawned) actor.slaves.push_back(*spawned);
                            }
                        }
                        if (actor.hp==actor.max_hp) actor.danger=5;
                    }
                    actor.next_turn=now+actor.walk_interval_ms;
                    result.ready.push_back(id);
                }
            }
            // 原参考Run先尝试召唤，随后清理已死亡随从；本轮释放的名额下轮可用。
            if (actor.kind==ZumaKind::king)
                std::erase_if(actor.slaves,[&](int slave) { return !alive(slave); });
        }
        return result;
    }
};
}
