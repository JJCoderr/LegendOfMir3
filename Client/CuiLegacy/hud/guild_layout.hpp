#pragma once
#include <array>
#include <algorithm>
#include <cstdint>
namespace mir3 {
// 0x4327E0 scrollbar+1900: page19, travel258, step12, vertical.
class GuildScroll {
public:
    bool captured=false;
    std::uint32_t last_time=0;
    static int thumb_y(int first,int count,int origin_y) {
        const float ratio=count>1 ? float(first)/(count-1) : 0.f;
        return int(origin_y+60.0+258.0*ratio);
    }
    bool input(int kind,int x,int y,int ox,int oy,int width,int height,int count,int& first,std::uint32_t now) {
        const int top=oy+60,bottom=top+258+height,left=ox+557;
        if (kind==2) { last_time=now;const bool was=captured;captured=false;return was; }
        if (kind==1) {
            if (!captured) return false;
            const float ratio=std::clamp(float(y-top)/(bottom-top),0.f,1.f);
            // 0x4339C3 fmul(count-1); Hex-Rays omits this x87 operand.
            first=int(double(std::max(0,count-1))*ratio);return true;
        }
        if (kind!=0 || width<=0 || height<=0 || x<left || x>=left+width || y<top || y>=bottom) return false;
        if (now-last_time<=10) return true;
        last_time=now;
        const int thumb=thumb_y(first,count,oy);
        if (y>=thumb && y<thumb+height) { captured=true;return true; }
        const float old=count>1 ? float(first)/(count-1) : 0.f;
        const float page=count>0 ? 19.f/count : 0.f;
        const float ratio=std::clamp(old+(y<thumb+6?-page:page),0.f,1.f);
        first=int(double(std::max(0,count-1))*ratio);return true;
    }
};
struct GuildLayout {
    // 0x4327E0 construction,0x432A40 post-positioning. frame,x,y,hint.
    static constexpr std::array<std::array<int,4>,9> buttons{{
        {1221,554,401,1052},{1301,39,385,4010},{1304,89,385,4009},
        {1307,139,385,4008},{1310,289,385,4011},{1313,339,385,4012},
        {1316,389,385,4013},{1320,439,385,4014},{1323,489,385,4015}
    }};
    int tab=0,pressed=-1;
    static int denial(int index,bool manager) {
        //0x433420 management controls require+148. Manager actions are separate.
        return index>=4 && index<=8 && !manager ? 4000 : -1;
    }
    int tooltip(int index) const {
        return index>=0 && index<9 && index!=pressed ? buttons[index][3] : -1;
    }
    std::array<int,2> frames(int index) const {
        if (index<0 || index>=9) return {-1,-1};
        // 0x4327E0 passes hover_draw=0, overlay=normal to0x41FA00.
        // 0x41FAF0 therefore draws one overlay even when hovered.
        return {buttons[index][0]+(pressed==index ? 1 : 0),-1};
    }
    bool release(int button) {
        const bool activate=button>=0 && button==pressed;pressed=-1;
        if (activate && button>=1 && button<=3) tab=button-1;
        return activate;
    }
    static int title(int tab) { return tab==0?4010:(tab==1?4009:4008); }
    static std::array<int,4> notice_rect(int row,int text_height) {
        return {35,60+(text_height+5)*row,514,text_height}; //0x433060
    }
};
}
