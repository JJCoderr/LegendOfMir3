#pragma once
#include <array>

// Native layout port of the completed client SkillLayout.
struct SkillPosition { int id,x,y; };
struct SkillLayout {
    // 0x450E70: window+2272, eight groups, 15 records each, stride12.
    static constexpr std::array<std::array<SkillPosition,15>,8> positions{{
        {{{1,30,76},{5,30,142},{9,150,142},{23,270,206},{22,150,272},{113,30,272},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}},
        {{{39,30,76},{40,30,142},{53,150,206},{33,150,272},{110,150,336},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}},
        {{{41,30,76},{11,30,142},{10,150,206},{24,150,272},{111,150,336},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}},
        {{{67,30,76},{8,270,142},{74,30,142},{73,150,206},{31,270,206},{72,150,272},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}},
        {{{2,30,76},{37,150,142},{29,30,206},{38,150,206},{77,30,272},{120,270,272},{122,270,336},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}},
        {{{6,30,76},{13,210,76},{16,30,142},{14,90,142},{18,150,76},{15,90,206},{19,150,142},{89,90,272},{94,90,336},{121,150,336},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}},
        {{{17,30,76},{20,150,76},{21,210,76},{30,30,142},{32,150,142},{104,210,142},{105,30,206},{123,30,272},{112,150,272},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}},
        {{{3,30,76},{27,210,272},{4,270,76},{7,30,142},{36,270,272},{12,30,206},{25,90,272},{26,150,336},{35,90,336},{34,150,402},{103,90,402},{107,210,336},{102,210,402},{106,210,532},{108,210,466}}}
    }};
    // 0x44F3E0: classification is independent of whether a tree slot exists.
    static constexpr int category(int id) {
        switch(id) {
        case 1: case 5: case 9: case 22: case 23: case 47: case 113: case 150: return 0;
        case 2: case 29: case 37: case 38: case 77: case 120: case 122: return 4;
        case 3: case 4: case 7: case 12: case 25: case 26: case 27: case 34: case 35: case 36: case 102: case 103: case 106: return 7;
        case 6: case 13: case 14: case 15: case 16: case 18: case 19: case 48: case 78: case 85: case 86: case 87: case 88: case 89: case 90: case 91: case 92: case 93: case 94: case 95: case 96: case 97: case 98: case 99: case 100: case 101: case 121: return 5;
        case 8: case 31: case 67: case 72: case 73: case 74: return 3;
        case 10: case 11: case 24: case 41: case 63: case 64: case 66: case 111: return 2;
        case 17: case 20: case 21: case 30: case 32: case 104: case 105: case 112: case 123: return 6;
        case 33: case 39: case 40: case 53: case 110: return 1;
        default:return 7;
        }
    }
    static int slot(int category,int id) {
        if (category<0 || category>=8 || id<=0) return -1;
        for (int i=0;i<15;++i) if (positions[category][i].id==id) return i;
        return -1;
    }
};
