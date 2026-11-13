#pragma once
#include <string>
#include <optional>
#include <vector>
#include "inventory/inventory_panel.hpp"
namespace mir3 {
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
struct SkillDisplay {
    int id=0,level=0,key=0;
    std::uint32_t experience=0;
    int required=0;
    std::string name;
    std::optional<std::array<int,3>> thresholds;
};

struct SkillPanel : DraggablePanel {
    SkillPanel() : DraggablePanel({348,0,360,488}) {}
    std::array<std::vector<SkillDisplay>,8> learned{};
    std::array<int,8> selected{-1,-1,-1,-1,-1,-1,-1,-1};
    std::array<int,13> bindings{}; // Original shortcut slots store the low byte of each skill ID.
    int tab=0,scroll=0,pressed=-1;
    bool remove(int id) {
        auto& group=learned[SkillLayout::category(id)];
        const auto skill=std::find_if(group.begin(),group.end(),[id](const auto& entry) { return entry.id==id; });
        const bool removed=skill!=group.end();
        if (removed) {
            group.erase(skill);
        }
        select_nonempty();
        return removed;
    }
    void select_nonempty() {
        if (learned[tab].empty()) {
            for (int i=0;i<int(learned.size());++i) {
                if (!learned[i].empty()) { tab=i;break; }
            }
        }
    }
    bool update_progress(int id,std::uint32_t experience,int level) {
        auto& group=learned[SkillLayout::category(id)];
        const auto skill=std::find_if(group.begin(),group.end(),[id](const auto& entry) { return entry.id==id; });
        if (skill==group.end()) return false;
        skill->experience=experience;
        skill->level=level;
        if (skill->thresholds && level<3) skill->required=(*skill->thresholds)[level];
        return true;
    }
    void wheel(bool up) { scroll=std::clamp(scroll+(up?-30:30),0,190); }
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
    static constexpr int visible_skill_limit=15;
    bool release_button(int index) {
        const bool clicked=index==pressed && index>=0 && index<=8;
        pressed=-1;
        if (clicked && index>0) tab=index-1; //0x450860: no scroll reset
        return clicked;
    }
    int tree_frame() const { return 1622+2*tab; }
};
}
