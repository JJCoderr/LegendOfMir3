#pragma once

namespace mir3 {

struct MonsterMotion { int base=0, frames=0, interval=0; };

inline MonsterMotion monster_attack_motion(int shape,int action) {
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
    // 46F690 selects the attack row by the current feature, including alternate bases.
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

inline MonsterMotion monster_struck_motion(int shape) {
    if (shape>=116 && shape<=118) return {240,3,150};
    if (shape>=145 && shape<=147) return {240,2,150};
    if (shape==149 || (shape>=154 && shape<=157) || (shape>=159 && shape<=164)) return {240,2,300};
    if (shape==158 || shape==165 || shape==166 || (shape>=530 && shape<=533) || shape==538)
        return {240,2,200};
    return {240,2,180};
}

inline MonsterMotion monster_death_motion(int shape) {
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

inline int monster_resource_direction(int shape,int action_base,int facing) {
    // 406750 applies the direction rules again whenever the monster starts an action.
    switch (shape) {
    case 31: return action_base==0?0:facing;
    case 67: return action_base==160?1:0;
    case 73: case 104: case 127: case 153: case 179:
    case 187: case 188: case 189: case 534: case 535: case 536: case 537: return 0;
    case 530: case 531: case 532: case 533: case 538: return action_base==320?0:facing;
    case 106: return 1;
    default: return facing;
    }
}

}
