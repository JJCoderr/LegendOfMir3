#pragma once
#include "world/zuma_ai.hpp"
#include <string_view>

namespace mir3 {
// CBeeQueen::Operate 用 g_pMonRaceInfo[_MONSTER_BEE]，此值是参考数据库索引，不是shape。
enum class BeeSlavePrototype : int { bee=125 };
enum class MonsterAppearance : int {
    carnivorous_plant=31,
    skeleton_axe_thrower=33, spined_dark_lizard=46, wedge_moth=75,
    zuma_archer=82, zuma_fanatic=83, zuma_guardian=84, zuma_king=85
};
// ProcessMonster.cpp 的 CDualAxeObject 配置，成功发射次数为 ChainShotCount - 1。
unsigned reference_ranged_burst(MonsterAppearance appearance);
struct MonsterPrototype {
    MonsterAppearance appearance;
    std::string_view name;
};

// Reference CallSlave的CP949注释 -> Zircon同名怪物的Mon库/组 -> 1.45外观。
// 这是参考语义映射，不是声称恢复了FLD_APPR数据库行；未知原型必须失败。
std::optional<MonsterPrototype> reference_zuma_prototype(ZumaSlavePrototype prototype);
}
