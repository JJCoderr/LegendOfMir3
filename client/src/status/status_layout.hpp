#pragma once
#include <array>
namespace mir3 {
// Normal character panel only. Coordinates are relative to this+24/this+28.
// Evidence: analysis/004710F0.txt and 00470E60.txt, unpacked treasure-hud.
struct StatusLayout {
    using Rect = std::array<int,4>; // x,y,width,height; original RECT is half-open
    static constexpr Rect name{100,30,130,20}; // 0x470E60, no secondary identity
    static constexpr std::array<int,2> close{289,14}, body{82,199};
    static constexpr std::array<int,5> combat_messages{1061,5101,5102,1060,1406};
    static constexpr std::array<Rect,5> combat_labels{{
        {11,320,58,15},{13,349,58,15},{118,349,60,15},{118,320,60,15},{218,349,60,15}}};
    static constexpr std::array<Rect,5> combat_values{{
        {58,320,58,15},{60,349,58,15},{163,349,60,15},{163,320,60,15},{270,349,60,15}}};
    // 0x4710F0: these rows are deliberately 377/408/438, not 30px from377.
    static constexpr std::array<Rect,3> element_labels{{{11,377,50,15},{11,408,50,15},{12,438,50,15}}};
    static constexpr std::array<int,3> element_messages{5178,5179,5180};
    static constexpr std::array<int,9> tooltip_messages{2400,2401,-1,-1,2404,2402,2403,1053,1054};
    static constexpr bool element_valid(int row,int column) {
        return row>=0 && row<3 && column>=0 && column<7;
    }
    static constexpr Rect element_rect(int row,int column,int part) {
        if (!element_valid(row,column)) return {};
        if (part==0) return {63+37*column,375+30*row,0,0}; // image origin
        if (part==1) return {81+37*column,row==0 ? 379 : (row==1 ? 408 : 439),20,15};
        // this+41488+16*(7*row+column): independent PtInRect area
        if (part==2) return {62+37*column,382+30*row,36,19};
        return {72+37*column,372+30*row,61,19}; // tooltip: left+10,right+35,y-10
    }
    static constexpr int element_frame(int row,int column) {
        return element_valid(row,column) ? (row==2 ? 1520 : 1510)+column : -1;
    }
    static int element_at(int x,int y) {
        for (int row=0;row<3;++row) for (int col=0;col<7;++col) {
            const auto r=element_rect(row,col,2);
            if (x>=r[0] && y>=r[1] && x<r[0]+r[2] && y<r[1]+r[3]) return row*7+col;
        }
        return -1;
    }
    static constexpr Rect value(int row, int part=0) {
        if (row<0 || row>=9 || part<0 || part>2) return {};
        if (row<2 || row>=7) return {245,91+24*row,67,15};
        // Slash is TextOutA at +276,+140, not centered DrawTextA.
        if (part==2) return {276,92+24*row,6,15};
        return {part==0 ? 245 : 282,91+24*row,30,15};
    }
    static constexpr Rect tooltip(int row) {
        return row>=0 && row<9 ? Rect{210,84+24*row,78,18} : Rect{};
    }
    static int tooltip_at(int x,int y) { // this+640, stride16; PtInRect
        for (int i=0;i<9;++i) {
            const auto r=tooltip(i);
            if (x>=r[0] && y>=r[1] && x<r[0]+r[2] && y<r[1]+r[3]) return i;
        }
        return -1;
    }
};
}
