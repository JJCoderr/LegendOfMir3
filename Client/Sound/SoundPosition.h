#pragma once
#include <algorithm>
#include <cmath>

namespace mir3 {
struct SoundPosition {
    int volume;
    int pan;

    // 0x453B60 supplies view pixels. 0x483E40 and 0x483EF0
    // quantize them independently for attenuation and DirectSound pan.
    static SoundPosition from_view(int x,int y) {
        if (!x && !y) return {0,0};
        const int column=(x+24)/47;
        const int row=(y+16)/31;
        const int pan_column=column==7 || column==9 ? 8 : column;
        const int pan=column<0?-10000:column>=17?10000:-600*(pan_column-8);
        if (column<0 || column>17 || row<0 || row>=16) return {-10000,pan};
        const int dx=column>=6 && column<=10 ? 0 : (std::min)(column,16)-8;
        const int dy=row>=5 && row<=9 ? 0 : (std::min)(row,14)-7;
        // 0x482D40 initializes the table at 0x8FC5FC.
        return {-150*static_cast<int>(std::sqrt(dx*dx+dy*dy)),pan};
    }
};
}
