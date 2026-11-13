#pragma once
#include "inventory/inventory_panel.hpp"
namespace mir3 {
struct SkillPanel : DraggablePanel {
    SkillPanel() : DraggablePanel({348,0,360,488}) {}
    int tab=0,scroll=0,pressed=-1;
    int thumb_y() const { return 100+static_cast<int>(238.0*static_cast<float>(double(scroll)/189.0)); }
    bool scroll_captured=false;
    std::uint32_t scroll_time=0;
    // 0x44E750/0x44EA70 and input0x450A40/0x450D10.
    bool scroll_input(int kind,int x,int y,int width,int height,std::uint32_t now) {
        if (kind==2) { scroll_time=now;const bool was=scroll_captured;scroll_captured=false;return was; }
        const int top=100,bottom=100+238+height;
        if (kind==1) {
            if (!scroll_captured) return false;
            const float ratio=std::clamp(float(y-top)/(bottom-top),0.f,1.f);
            scroll=int(190.0*ratio);return true;
        }
        if (kind!=0 || width<=0 || height<=0 || x<322 || x>=322+width || y<top || y>=bottom) return false;
        float ratio=float(double(scroll)/189.0);
        if (now-scroll_time>10) {
            const int thumb=thumb_y();
            if (y>=thumb && y<thumb+height) scroll_captured=true;
            else ratio=std::clamp(ratio+(y<thumb+7?-7.f/190:7.f/190),0.f,1.f);
            scroll_time=now;
        }
        // Click clamps to189; dragging and wheel can reach190 in the original.
        scroll=std::min(189,int(190.0*ratio));return true;
    }
    // 0x44EA70 draw-time positions supersede constructor coordinates.
    static std::array<int,3> button(int index) {
        if (index==0) return {1221,316,422};
        if (index>=1 && index<=8) return {1640+2*(index-1),46+34*(index-1),30};
        return {-1,0,0};
    }
    static constexpr std::array<int,2> tree_origin{18,63}; //0x44EA70
    static constexpr int tree_height=331;
    bool release_button(int index) {
        const bool clicked=index==pressed && index>=0 && index<=8;
        pressed=-1;
        if (clicked && index>0) tab=index-1; //0x450860: no scroll reset
        return clicked;
    }
    int tree_frame() const { return 1622+2*tab; }
};
}
