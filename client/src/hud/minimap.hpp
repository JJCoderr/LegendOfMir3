#pragma once
#include "hud/hud_layout.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>

namespace mir3 {
struct Minimap {
    bool visible=false, pending=false, translucent=false, large=false, dragging=false, button_captured=false;
    HudLayout::Point position{672,0}, drag_offset{};
    int width=0, height=0, player_x=0, player_y=0;
    std::uint64_t reply_time=0;
    std::vector<HudLayout::Point> type_five_actors, nearby_green, nearby_orange;

    static std::int64_t map_pixel_x(int cell_x) { return std::int64_t(cell_x)*3/2; }
    static int actor_marker_x(int cell_x) {
        // The original subtracts one before converting to a screen pixel.
        return int((std::int64_t(cell_x)*3-2)/2);
    }

    int size() const { return large?256:128; }
    HudLayout::Rect bounds() const {
        return {position[0],position[1],std::min(size(),width),std::min(size(),height)};
    }
    HudLayout::Point button_position(int index) const {
        const auto [x,y,w,h]=bounds();
        return {x+w-38+19*index,y+h-19};
    }
    int button_hint(int index) const {
        return index==0?(translucent?1802:1800):(large?1803:1801);
    }
    HudLayout::Point source() const {
        const auto [x,y,w,h]=bounds();
        const auto map_x=map_pixel_x(player_x);
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
