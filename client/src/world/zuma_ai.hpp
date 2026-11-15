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
enum class ZumaKind { statue, king, ranged };

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

    static bool within(int x,int y,int other_x,int other_y,int radius);

    // 每个实例是同一地图。visible(id)由世界的视野/敌我关系层提供。
    // spawn(owner, prototype)返回实际成功生成的对象ID；失败不占随从名额。
    // alive(id)包括死亡/幽灵检查。random(bound)必须返回[0,bound)。
    using Visible=std::function<std::vector<Target>(int)>;
    using Spawn=std::function<std::optional<int>(const Actor&,ZumaSlavePrototype)>;
    using Alive=std::function<bool(int)>;
    using Random=MonsterPursuit::Random;
    Result tick(Time now,const Visible& visible,const Spawn& spawn,const Alive& alive,const Random& random);
};
}
