#pragma once
#include <array>
#include <cstdint>
namespace mir3 {
struct HumanAction { std::uint16_t start, count, interval_ms; };
struct HumanActions {
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
        spell_active=false;spell_relative=0;spell_limit=10;
    }
    void start_spell(int action,int limit=10,bool catch_up=false) {
        start(action,catch_up);
        spell_active=action==2 || action==3;
        spell_limit=static_cast<std::uint8_t>(limit);
    }
    void advance(std::uint32_t milliseconds,std::uint32_t flags=0) {
        if (finished()) return;
        elapsed+=milliseconds;
        if (static_cast<std::int32_t>(elapsed)>static_cast<std::int32_t>(
            HumanActions::interval(selected,interval_override,flags))) {
            elapsed=0;
            ++current;
            if (spell_active) ++spell_relative;
            // 0x414220 halves the interval after the first frame increment.
            if (halve_after_first_frame) {
                interval_override=HumanActions::interval(selected,interval_override)/2;
                halve_after_first_frame=false;
            }

            if (spell_active) {
                const int end=HumanActions::table[selected].count;
                if (selected==3) {
                    if (current>=end-1 && spell_relative<=static_cast<std::uint8_t>(spell_limit-2)) {
                        interval_override=70;
                        current=end-2;
                    }
                } else if (current>=end-3 && current<end-2) {
                    if (spell_relative<static_cast<std::uint8_t>(spell_limit-4)) current=end-4;
                } else if (current>=end-2 && current<end-1) interval_override=200;
                else if (current>=end-1) interval_override=80;
            }
        }
    }
    bool finished() const { return selected<0 || selected>=33 || current>=HumanActions::table[selected].count; }
    void set_interval(std::uint32_t milliseconds) { interval_override=milliseconds; }
    int action() const { return selected; }
    int spell_frame() const { return spell_relative; }
    int spell_frame_limit() const { return spell_limit; }
    int frame() const { return finished() ? (selected>=0 && selected<33 ? HumanActions::table[selected].count-1 : 0) : current; }
private:
    int selected=-1,current=0;
    std::uint32_t elapsed=0;
    std::uint32_t interval_override=0;
    bool halve_after_first_frame=false;
    bool spell_active=false;
    int spell_relative=0;
    std::uint8_t spell_limit=10;
};
}
