#pragma once
#include "hud/hud_layout.hpp"
#include <algorithm>
#include <cstdint>

namespace mir3 {
struct Minimap {
    bool visible=false, pending=false, translucent=false, large=false, dragging=false;
    HudLayout::Point position{672,0}, drag_offset{};
    int width=0, height=0, player_x=0, player_y=0;
    std::uint64_t reply_time=0;

    int size() const { return large?256:128; }
    HudLayout::Rect bounds() const {
        return {position[0],position[1],std::min(size(),width),std::min(size(),height)};
    }
    HudLayout::Point source() const {
        const auto [x,y,w,h]=bounds();
        const auto map_x=std::int64_t(player_x)*3/2;
        return {int(std::clamp<std::int64_t>(map_x-w/2,0,width-w)),
            int(std::clamp<std::int64_t>(std::int64_t(player_y)-h/2,0,height-h))};
    }
    bool toggle(std::uint64_t now,std::uint64_t cooldown) {
        if (reply_time && now-reply_time<=cooldown) return false;
        if (visible) {
            visible=false;
            dragging=false;
            return false;
        }
        pending=true;
        return true;
    }
    void receive(int w,int h,std::uint64_t now) {
        width=w;
        height=h;
        reply_time=now;
        if (pending) visible=true;
        pending=false;
        place_at_top_right();
    }
    void resize() {
        large=!large;
        place_at_top_right();
    }
    void place_at_top_right() {
        position={800-std::min(size(),width),0};
        dragging=false;
    }
};
}
