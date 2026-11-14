#pragma once
#include <array>
#include <cstdint>
namespace mir3 {
struct HumanAction { std::uint16_t start, count, interval_ms; };
struct HumanActions {
    // 0x410B20: action 15 uses actor+400437 and clothing at +71.
    static int hit_sound(int frame,std::uint8_t weapon,std::uint8_t clothing,std::uint8_t type) {
        if (frame==2) return 138+int(type!=0);
        if (frame!=1) return 0;
        int sound=73;
        switch (weapon) {
        case 1: case 4: case 8: case 9: case 11: case 13: case 14: case 18:
        case 20: case 21: case 22: case 23: case 25: case 26: case 27: case 28:
        case 29: case 30: case 31: case 32: case 33: case 38: case 40: case 45: case 46:
            sound=70; break;
        case 5: case 10: case 12: case 15: sound=71; break;
        case 2: case 3: case 6: case 7: case 16: case 17: case 19: case 24: sound=72; break;
        }
        return sound+(clothing==3 || clothing==6?10:0);
    }
    // 0x410B20: actions 9..14 play the weapon swing at relative frame 2.
    static int weapon_swing_sound(std::uint8_t weapon) {
        switch (weapon) {
        case 1: case 4: case 11: case 13: case 14: case 20: case 25:
        case 29: case 31: case 33: case 38: case 40: case 45: case 46: return 53;
        case 2: case 3: case 6: case 7: case 16: case 17: case 19: case 24: return 56;
        case 5: case 10: case 12: return 54;
        case 8: case 9: return 51;
        case 15: return 55;
        case 18: case 22: case 23: case 26: case 27: case 28: case 30: case 32: return 52;
        case 21: return 50;
        default: return 57;
        }
    }
    // 0x410DF0, called after the first increment and before cell commit.
    static constexpr int movement_sound(int action,int relative) {
        if (action==31) return relative==3?35:0;
        if (action==30) return relative==1?33:relative==4?34:0;
        if (action==21 || action==22) return relative==1 || relative==4?1:0;
        return 0;
    }
    // 0x46EFA0 ->0x4872D0; 33 six-byte records at0x8FBA68.
    static constexpr std::array<HumanAction,33> table{{
        {0,4,200}, // action 0
        {80,6,100}, // action 1
        {160,5,75}, // action 2
        {240,5,75}, // action 3
        {320,1,100}, // action 4
        {400,1,100}, // action 5
        {480,1,100}, // action 6
        {560,3,200}, // action 7
        {640,2,400}, // action 8
        {720,6,85}, // action 9
        {800,6,85}, // action 10
        {880,6,85}, // action 11
        {960,6,85}, // action 12
        {1040,6,85}, // action 13
        {1120,6,85}, // action 14
        {1200,3,100}, // action 15
        {1280,10,70}, // action 16
        {1360,10,90}, // action 17
        {1440,10,70}, // action 18
        {1520,10,120}, // action 19
        {1600,10,100}, // action 20
        {1680,6,90}, // action 21
        {1760,6,120}, // action 22
        {1840,6,100}, // action 23
        {1920,10,100}, // action 24
        {2000,4,180}, // action 25
        {2080,3,180}, // action 26
        {2160,8,180}, // action 27
        {2240,8,180}, // action 28
        {2320,4,200}, // action 29
        {2400,6,100}, // action 30
        {2480,6,100}, // action 31
        {2560,3,100}, // action 32
    }};
    // 0x4702F0; ordinary=false maps9/10/13 to11/12/14 for skill callers.
    static int weapon_attack(std::uint8_t weapon,bool ordinary=true) {
        int action=9;
        switch (weapon) {
        case 2: case 3: case 6: case 16: case 17: case 19: action=13; break;
        case 4: case 5: case 7: case 15: action=10; break;
        default: break;
        }
        return ordinary ? action : (action==13 ? 14 : action+2);
    }
    static constexpr int for_motion(int motion) {
        constexpr std::array<int,5> actions{0,21,22,9,8};
        return motion>=0 && motion<int(actions.size()) ? actions[motion] : -1;
    }
    // 0x406750: actor+112 start, +116 exclusive end, +124 current frame.
    static int frame(int action,int direction,int relative) {
        if (action<0 || action>=int(table.size()) || direction<0 || direction>=8 ||
            relative<0 || relative>=table[action].count) return -1;
        return table[action].start+direction*10+relative;
    }
    static std::uint32_t interval(int action,std::uint32_t override_ms=0,std::uint32_t flags=0) {
        if (action<0 || action>=int(table.size())) return 0;
        auto value=override_ms ? override_ms : table[action].interval_ms;
        return flags & 0x08000000u ? value*2 : value;
    }
};
// Finite ordinary attack path of0x416E70; completion at exclusive frame end.
class HumanActionClock {
public:
    void start(int action,bool catch_up=false) {
        selected=action; current=0; elapsed=0;
        halve_after_first_frame=catch_up;
        interval_override=0;
    }
    void advance(std::uint32_t milliseconds,std::uint32_t flags=0) {
        if (finished()) return;
        elapsed+=milliseconds;
        if (static_cast<std::int32_t>(elapsed)>static_cast<std::int32_t>(
            HumanActions::interval(selected,interval_override,flags))) {
            elapsed=0;
            ++current;
            // 0x414220 halves the interval after the first frame increment.
            if (halve_after_first_frame) {
                interval_override=HumanActions::interval(selected)/2;
                halve_after_first_frame=false;
            }
        }
    }
    bool finished() const { return selected<0 || selected>=33 || current>=HumanActions::table[selected].count; }
    int action() const { return selected; }
    int frame() const { return finished() ? (selected>=0 && selected<33 ? HumanActions::table[selected].count-1 : 0) : current; }
private:
    int selected=-1,current=0;
    std::uint32_t elapsed=0;
    std::uint32_t interval_override=0;
    bool halve_after_first_frame=false;
};
}
