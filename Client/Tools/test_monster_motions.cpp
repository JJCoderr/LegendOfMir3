#include "../GameProcess/MonsterMotion.h"
#include <cstdio>

// 逐项核对 MonsterMotion.h 与原版分档（对照 Godot 分支 world/Monster.cpp）。
int main()
{
    using mir3::MonsterMotion;
    int failures = 0;
    const auto expect = [&](const char* what, MonsterMotion actual, MonsterMotion wanted) {
        if (actual.first != wanted.first || actual.frames != wanted.frames || actual.delay != wanted.delay) {
            std::printf("FAIL %s: got {%u,%u,%u} want {%u,%u,%u}\n", what,
                actual.first, actual.frames, actual.delay, wanted.first, wanted.frames, wanted.delay);
            ++failures;
        }
    };

    const auto idle = [&](const char* what, int shape, MonsterMotion wanted) {
        expect(what, mir3::MonsterIdleMotion(shape), wanted); };
    idle("idle/95", 95, {0,1,250});
    idle("idle/9", 9, {0,1,250});
    idle("idle/530", 530, {0,1,200});
    idle("idle/538", 538, {0,1,200});
    idle("idle/145", 145, {0,4,150});
    idle("idle/158", 158, {0,4,200});
    idle("idle/161", 161, {0,4,250});
    idle("idle/166", 166, {0,4,250});
    idle("idle/3", 3, {0,1,250});
    idle("idle/default", 2000, {0,4,300});

    const auto attack = [&](const char* what, int shape, int action, MonsterMotion wanted) {
        expect(what, mir3::MonsterAttackMotion(shape, action), wanted); };
    attack("attack1/0", 0, 2, {160,6,100});
    attack("attack1/3", 3, 2, {160,6,140});
    attack("attack1/33", 33, 2, {160,6,90});
    attack("attack1/90", 90, 2, {160,6,160});
    attack("attack1/95", 95, 2, {160,10,90});
    attack("attack1/120", 120, 2, {160,10,90});
    attack("attack1/164", 164, 2, {160,9,120});
    attack("attack1/186", 186, 2, {400,8,90});
    attack("attack1/default", 2000, 2, {160,6,130});
    attack("attack2/159", 159, 5, {400,6,140});
    attack("attack2/164", 164, 5, {400,9,120});
    attack("attack2/default", 3, 5, {400,6,150});
    attack("spell1/10", 10, 6, {400,6,150});
    attack("spell1/141", 141, 6, {480,6,120});
    attack("spell1/164", 164, 6, {480,8,150});
    attack("spell1/139", 139, 6, {480,6,200});
    attack("spell1/default", 3, 6, {480,10,150});
    attack("spell2/141", 141, 7, {560,6,120});
    attack("spell2/default", 3, 7, {560,10,150});

    expect("struck/116", mir3::MonsterStruckMotion(116), {240,3,150});
    expect("struck/145", mir3::MonsterStruckMotion(145), {240,2,150});
    expect("struck/149", mir3::MonsterStruckMotion(149), {240,2,300});
    expect("struck/158", mir3::MonsterStruckMotion(158), {240,2,200});
    expect("struck/530", mir3::MonsterStruckMotion(530), {240,2,200});
    expect("struck/default", mir3::MonsterStruckMotion(3), {240,2,180});

    expect("death/8", mir3::MonsterDeathMotion(8), {320,10,130});
    expect("death/30", mir3::MonsterDeathMotion(30), {320,4,220});
    expect("death/104", mir3::MonsterDeathMotion(104), {320,20,120});
    expect("death/141", mir3::MonsterDeathMotion(141), {720,20,120});
    expect("death/145", mir3::MonsterDeathMotion(145), {320,10,95});
    expect("death/149", mir3::MonsterDeathMotion(149), {320,10,120});
    expect("death/156", mir3::MonsterDeathMotion(156), {320,10,145});
    expect("death/159", mir3::MonsterDeathMotion(159), {320,10,110});
    expect("death/534", mir3::MonsterDeathMotion(534), {320,9,120});
    expect("death/default", mir3::MonsterDeathMotion(3), {320,10,140});

    expect("burrow/83", mir3::MonsterBurrowMotion(83), {640,6,120});
    expect("burrow/85", mir3::MonsterBurrowMotion(85), {640,20,120});
    expect("burrow/90", mir3::MonsterBurrowMotion(90), {640,10,100});
    expect("burrow/530", mir3::MonsterBurrowMotion(530), {640,7,120});
    expect("burrow/default", mir3::MonsterBurrowMotion(3), {640,10,120});

    // 整表：行走与特殊动作1保留固定值，其余按外观分档。
    const auto plain = mir3::MonsterMotionTable(3);
    expect("table[1]", plain[1], {80,6,90});
    expect("table[9]", plain[9], {720,6,150});
    for (int slot = 0; slot < mir3::MonsterMotionSlots; ++slot)
        if (plain[slot].frames == 0) { std::printf("FAIL table slot %d has no frames\n", slot); ++failures; }

    // 分档确实生效：不能退回全局常数。
    const auto fast = mir3::MonsterMotionTable(33);
    const auto slow = mir3::MonsterMotionTable(2000);
    if (fast[2].delay == slow[2].delay) { std::printf("FAIL attack tiers collapsed\n"); ++failures; }
    if (mir3::MonsterIdleMotion(530).frames != 1) { std::printf("FAIL idle tier 530\n"); ++failures; }
    if (mir3::MonsterDeathMotion(141).first != 720) { std::printf("FAIL death base 141\n"); ++failures; }

    // 406750 方向规则：按动作起始帧重判。
    const auto dir = [&](const char* what, int shape, int base, int facing, int wanted) {
        const int actual = mir3::MonsterResourceDirection(shape, base, facing);
        if (actual != wanted) { std::printf("FAIL %s: got %d want %d\n", what, actual, wanted); ++failures; } };
    dir("dir/31 idle", 31, 0, 5, 0);
    dir("dir/31 attack", 31, 160, 5, 5);
    dir("dir/67 attack", 67, 160, 3, 1);
    dir("dir/67 idle", 67, 0, 3, 0);
    dir("dir/73", 73, 160, 3, 0);
    dir("dir/104", 104, 320, 3, 0);
    dir("dir/106", 106, 0, 3, 1);
    dir("dir/127", 127, 160, 3, 0);
    dir("dir/534", 534, 320, 3, 0);
    dir("dir/530 death", 530, 320, 3, 0);
    dir("dir/530 walk", 530, 80, 3, 3);
    dir("dir/ordinary", 3, 160, 3, 3);
    std::printf("MONSTER_MOTIONS_OK: idle/attack/struck/death/burrow tiers, failures=%d\n", failures);
    return failures ? 1 : 0;
}