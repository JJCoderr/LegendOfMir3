#pragma once
#include <array>
#include <cstdint>
namespace mir3 {
struct SelectionRole {
    std::string name;
    std::uint8_t profession=0,gender=0,level=0,action=0;
    int x=0,y=0,frame=0;
    std::uint16_t elapsed=0;
    bool occupied=false;
    int animation_index() const { return action+5*(gender+2*profession); }
    int first_frame() const { return 200+animation_index()*60; }
    int last_frame() const {
        static constexpr std::array<int,30> lengths{10,19,11,7,17,10,10,10,11,15,10,11,10,10,14,10,17,10,8,16,10,16,19,11,16,10,16,10,9,14};
        return first_frame()+lengths[animation_index()];
    }
    void start(std::uint8_t value) { action=value;frame=first_frame();elapsed=0; }
    void advance(std::uint32_t ms) {
        elapsed=std::uint16_t(elapsed+ms);
        if (elapsed<=120) return;
        elapsed=0;
        if (++frame<=last_frame()) return;
        if (action==1) start(2);
        else if (action==3) start(0);
        else frame=first_frame();
    }
};
}
