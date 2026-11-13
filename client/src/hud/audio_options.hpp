#pragma once

namespace mir3 {
struct AudioOptions {
    bool music_enabled=true;
    bool effects_enabled=true;
    bool reverse_pan=false;
    int music_level=0;
    int effects_level=0;

    static constexpr int normalized_level(int level) {
        return level>=-100 && level<=0 ? level : 0;
    }
    // 0x4640C0: knob offset is relative to panel x + 288.
    static constexpr int knob_offset(int relative_x) {
        return relative_x<288 ? 0 : relative_x>468 ? 185 : relative_x-288;
    }
    // 0x464990: divide the offset by 1.8 and truncate toward zero.
    static constexpr int level_from_knob(int offset) {
        return (5*offset-900)/9;
    }
    static constexpr int knob_from_level(int level) {
        return 9*(level+100)/5;
    }
    // 0x463500 uses strict track boundaries and steps by nine pixels.
    static constexpr int track_step(int offset,int x) {
        if (x<=266 || x>=487) return offset;
        if (x>offset+292 || x>477) return offset+9>180?185:offset+9;
        if (x<offset+276 || x<275) return offset<9?0:offset-9;
        return offset;
    }
    void drag_volume(bool music,int relative_x) {
        (music ? music_level : effects_level)=level_from_knob(knob_offset(relative_x));
    }
    static constexpr int volume(int level) { return 40*level; }
};
}
