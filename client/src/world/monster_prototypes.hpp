#pragma once
#include "world/zuma_ai.hpp"
#include <string_view>

namespace mir3 {
enum class MonsterAppearance : int { wedge_moth=75, zuma_archer=82, zuma_fanatic=83, zuma_guardian=84, zuma_king=85 };
struct MonsterPrototype {
    MonsterAppearance appearance;
    std::string_view name;
};

// Reference CallSlave的CP949注释 -> Zircon同名怪物的Mon库/组 -> 1.45外观。
// 这是参考语义映射，不是声称恢复了FLD_APPR数据库行；未知原型必须失败。
constexpr std::optional<MonsterPrototype> reference_zuma_prototype(ZumaSlavePrototype prototype) {
    switch (prototype) {
    case ZumaSlavePrototype::archer: return MonsterPrototype{MonsterAppearance::zuma_archer,"祖玛弓箭手"};
    case ZumaSlavePrototype::fanatic: return MonsterPrototype{MonsterAppearance::zuma_fanatic,"祖玛狂信徒"};
    case ZumaSlavePrototype::guardian: return MonsterPrototype{MonsterAppearance::zuma_guardian,"祖玛卫士"};
    case ZumaSlavePrototype::moth: return MonsterPrototype{MonsterAppearance::wedge_moth,"楔蛾"};
    }
    return {};
}
}
