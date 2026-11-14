#pragma once
#include <array>
#include <vector>
#include <cstdint>
#include <string>

namespace mir3 {
struct DamageGlyph { int frame,x,y; bool operator==(const DamageGlyph&) const = default; };
// 0x4081F0/0x415740 enqueue; 0x40E730/0x410940 draw and age.
// Values +0x62650, screen positions +0x62670, cursor +0x626B0, ages +0x626B4.
struct DamageNumbers {
    std::array<int,8> values{};
    std::array<std::uint16_t,8> ages{};
    int cursor=0;
    bool enabled=true; // adapter for global 0x8F1D30
    void changed(int old_hp,int new_hp) {
        if (!enabled || !old_hp || old_hp==new_hp) return;
        push(new_hp-old_hp);
    }
    void push(int value) {
        if (!enabled) return;
        values[cursor]=value; ages[cursor]=0;
        if (++cursor>7) cursor=0;
    }
    // 0x42FC60; GameInter object 0x5A041C. HP deltas fit five digits.
    static std::vector<DamageGlyph> glyphs(int value,int x,int y) {
        if (!value) return {{1850,x-10,y}};
        const bool healing=value>0;
        const int magnitude=healing ? value : -value;
        if (magnitude>65535) return {}; // domain of unsigned WORD HP differences
        std::vector<DamageGlyph> result;
        result.push_back({healing?1821:(magnitude<100?1800:1840),x-(healing?10:8),y+(healing?0:4)});
        const int base=healing?1810:(magnitude<100?1790:1830);
        for (char digit : std::to_string(magnitude)) {
            result.push_back({base+digit-'0',x,y}); x+=11;
        }
        return result;
    }
    std::vector<DamageGlyph> draw(int screen_x,int screen_y,bool human) {
        std::vector<DamageGlyph> result;
        if (!enabled) return result;
        // Original loops [0,cursor), not all eight slots. Preserve wrap behavior.
        for (int i=0;i<cursor;++i) if (ages[i]<=40) {
            const int x=screen_x+ages[i]+20;
            const int y=static_cast<int>(double(screen_y-(human?40:30))-double(ages[i])*1.3);
            const auto digits=glyphs(values[i],x,y);
            result.insert(result.end(),digits.begin(),digits.end());
            ++ages[i];
        }
        return result;
    }
};
}
