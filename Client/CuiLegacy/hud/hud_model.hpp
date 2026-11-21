#pragma once
#include "hud_layout.hpp"
#include "button_skin.hpp"
#include <algorithm>

namespace mir3 {
// Display data only. Offline preview values are explicit, not server state.
struct HudModel {
    HudLayout layout;
    std::string map_name;
    int map_x=0, map_y=0, level=1, profession=0, appearance=0;
    std::uint16_t hp=0, max_hp=0, mp=0, max_mp=0;
    unsigned experience=0, next_experience=0, weight=0, max_weight=0;
    std::array<unsigned,2> wear_weight{}, hand_weight{}, status_bonus_percent{};
    std::array<int,2> defence{}, attack{}, magic_defence{}, magic_attack{}, spirit_attack{};
    std::array<bool,2> primary_stat{true,true};
    HudLayout::Point pointer{-1,-1};
    std::uint32_t emblem_selector=0;
    std::array<std::uint32_t,4> emblem_references{};
    int hovered=-1, pressed=-1;
    std::array<HudLayout::Point,23> button_sizes{};
    bool shared_hp() const { return profession==0 && level<24; }
    int portrait_frame() const { return profession>=0 && profession<=2?650+profession*2+(appearance!=0):-1; }
    std::array<int,3> stat(int side) const {
        const auto range=side==0?(primary_stat[0]?defence:magic_defence):
            (primary_stat[1]?attack:profession==2?spirit_attack:magic_attack);
        return {1500+side+(primary_stat[side]?0:2),range[0],range[1]};
    }
    bool press_stat(int x,int y) {
        if (y<570 || y>=590) return false;
        const int side=x>=634 && x<664?0:x>=717 && x<747?1:-1;
        if (side<0) return false;
        primary_stat[side]=!primary_stat[side];return true;
    }
    int orb_frame(int side) const { return side==0?1163:shared_hp()?1165:1164; }
    HudLayout::Rect orb_crop(int side,HudLayout::Point left,HudLayout::Point right) const {
        const auto size=side==0 || shared_hp()?left:right;
        if (side<0 || side>1 || size[0]<=0 || size[1]<=0) return {};
        const auto amount=side==0 || shared_hp()?hp:mp;
        const auto capacity=side==0 || shared_hp()?max_hp:max_mp;
        const float ratio=capacity?static_cast<float>(std::min(1.0,double(amount)/capacity)):0.0f;
        const int top=static_cast<int>(size[1]-size[1]*double(ratio));
        return {0,top,size[0],size[1]-top};
    }
    int meter_crop(int meter,int height) const {
        const auto amount=meter==0?experience:weight, capacity=meter==0?next_experience:max_weight;
        const float ratio=capacity?static_cast<float>(std::min(1.0,double(amount)/capacity)):0.0f;
        return static_cast<int>(height-height*double(ratio));
    }
    HudLayout::Rect button_rect(int index) const {
        if (index<0 || index>=HudLayout::count(layout.expanded)) return {};
        const auto size=button_sizes[index];
        return HudLayout::hit_rect(index,layout.expanded,size[0],size[1],layout.repositioned);
    }
};
class HudController {
public:
    explicit HudController(HudModel& model): model(model) {}
    int hit(int x,int y) const {
        for (int i=0;i<HudLayout::count(model.layout.expanded);++i) {
            const auto r=model.button_rect(i);
            if (x>=r[0] && y>=r[1] && x<r[0]+r[2] && y<r[1]+r[3]) return i;
        }
        return -1;
    }
    void motion(int x,int y) { model.pointer={x,y};model.hovered=hit(x,y); }
    int pointer(int x,int y,bool down) {
        motion(x,y);
        if (down) { model.pressed=model.press_stat(x,y)?-1:model.hovered; return -1; }
        model.pressed=-1;
        // 0x41FCD0 activates on release inside, even if pressed outside.
        const int action=model.hovered;
        if (action==14) { model.layout.toggle(); motion(x,y); }
        return action;
    }
    void cancel() { model.pressed=model.hovered=-1;model.pointer={-1,-1}; }
private:
    HudModel& model;
};
}
