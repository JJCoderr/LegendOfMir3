#pragma once
#include <array>
#include <cstdint>

// MIR3 1.45 怪物动作帧表。
// 参照 Godot 分支 client/src/world/Monster.cpp 的 monster_idle_motion /
// monster_attack_motion / monster_struck_motion / monster_death_motion /
// monster_burrow_motion（原版 46F3B0、46F690 等入口按外观编号分档）。
// first 为库内起始帧，frames 为动作帧数，delay 为帧间隔毫秒。
namespace mir3 {
struct MonsterMotion {
    std::uint16_t first=0,frames=1,delay=300;
};

// 槽位与 Define.h 的 _MT_MON_* 一致：0 空闲、1 行走、2 普攻1、3 受击、4 死亡、
// 5 普攻2、6 魔攻1、7 魔攻2、8 出现/消失、9 特殊动作1。
constexpr int MonsterMotionSlots=10;

inline MonsterMotion MonsterIdleMotion(int shape)
{
    if (shape==95 || shape<10) return {0,1,250};
    if ((shape>=530 && shape<=533) || shape==538) return {0,1,200};
    if (shape>=145 && shape<=147) return {0,4,150};
    if (shape==158) return {0,4,200};
    if (shape==161 || shape==165 || shape==166) return {0,4,250};
    return {0,4,300};
}

// action 取 2/5/6/7，对应普攻1、普攻2、魔攻1、魔攻2；其他值按普攻1处理。
inline MonsterMotion MonsterAttackMotion(int shape,int action)
{
    if (action==7) return shape==141?MonsterMotion{560,6,120}:MonsterMotion{560,10,150};
    if (action==5) {
        switch (shape) {
        case 159: case 160: return {400,6,140};
        case 164: return {400,9,120};
        default: return {400,6,150};
        }
    }
    if (action==6) {
        switch (shape) {
        case 10: case 181: return {400,6,150};
        case 13: case 157: case 182: case 185: return {480,6,150};
        case 139: case 158: return {480,6,200};
        case 141: return {480,6,120};
        case 161: return {480,6,140};
        case 164: case 186: return {480,8,150};
        default: return {480,10,150};
        }
    }
    switch (shape) {
    case 0: case 1: case 2: case 12: case 99: return {160,6,100};
    case 3: case 5: case 6: case 7: case 8: case 9: case 15: case 19:
    case 93: case 96: case 97: case 98: case 159: case 160: case 161: return {160,6,140};
    case 4: case 10: case 11: case 13: case 17: case 18: case 94:
    case 145: case 146: case 149: case 156: case 157: case 165: case 166: return {160,6,150};
    case 14: case 16: case 92: case 147: case 154: case 155: case 162: case 163: return {160,6,120};
    case 33: case 46: case 82: return {160,6,90};
    case 90: return {160,6,160};
    case 95: case 120: return {160,10,90};
    case 164: return {160,9,120};
    case 186: return {400,8,90};
    default: return {160,6,130};
    }
}

inline MonsterMotion MonsterStruckMotion(int shape)
{
    if (shape>=116 && shape<=118) return {240,3,150};
    if (shape>=145 && shape<=147) return {240,2,150};
    if (shape==149 || (shape>=154 && shape<=157) || (shape>=159 && shape<=164)) return {240,2,300};
    if (shape==158 || shape==165 || shape==166 || (shape>=530 && shape<=533) || shape==538)
        return {240,2,200};
    return {240,2,180};
}

inline MonsterMotion MonsterDeathMotion(int shape)
{
    switch (shape) {
    case 8: case 86: case 94: case 105: return {320,10,130};
    case 30: case 37: return {320,4,220};
    case 104: return {320,20,120};
    case 127: case 153: case 179: case 187: case 188: case 189:
    case 534: case 535: case 536: case 537: return {320,9,120};
    case 141: return {720,20,120};
    case 145: case 146: case 147: case 154: case 155: return {320,10,95};
    case 149: case 162: case 163: case 164: return {320,10,120};
    case 156: case 157: return {320,10,145};
    case 158: case 165: case 166: return {320,10,150};
    case 159: case 160: case 161: return {320,10,110};
    case 530: case 531: case 532: case 533: case 538: return {320,8,120};
    default: return {320,10,140};
    }
}

inline MonsterMotion MonsterBurrowMotion(int shape)
{
    if (shape==83 || shape==84) return {640,6,120};
    if (shape==85) return {640,20,120};
    if (shape==90) return {640,10,100};
    if ((shape>=530 && shape<=533) || shape==538) return {640,7,120};
    return {640,10,120};
}

// 整表：行走与特殊动作1不由外观分档，保留固定值。
inline std::array<MonsterMotion,MonsterMotionSlots> MonsterMotionTable(int shape)
{
    std::array<MonsterMotion,MonsterMotionSlots> motions{};
    motions[0]=MonsterIdleMotion(shape);
    motions[1]={80,6,90};
    motions[2]=MonsterAttackMotion(shape,2);
    motions[3]=MonsterStruckMotion(shape);
    motions[4]=MonsterDeathMotion(shape);
    motions[5]=MonsterAttackMotion(shape,5);
    motions[6]=MonsterAttackMotion(shape,6);
    motions[7]=MonsterAttackMotion(shape,7);
    motions[8]=MonsterBurrowMotion(shape);
    motions[9]={720,6,150};
    return motions;
}
// 原版 406750：怪物每次起动作时重新套用方向规则，actionBase 为动作起始帧
// （0 空闲、160 普攻1、320 死亡 等），与 MonsterMotionTable 的 first 一致。
inline int MonsterResourceDirection(int shape,int actionBase,int facing)
{
    switch (shape) {
    case 31: return actionBase==0?0:facing;
    case 67: return actionBase==160?1:0;
    case 73: case 104: case 127: case 153: case 179:
    case 187: case 188: case 189: case 534: case 535: case 536: case 537: return 0;
    case 530: case 531: case 532: case 533: case 538: return actionBase==320?0:facing;
    case 106: return 1;
    default: return facing;
    }
}
}