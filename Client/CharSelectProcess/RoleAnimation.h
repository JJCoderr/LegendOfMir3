#pragma once
#include <cstdint>
// MIR3 selection/creation clocks discard excess time and advance at most one frame.
struct RoleAnimation {
    enum {Idle=0,Select=1,Selected=2,Deselect=3,Create=4,Interval=120,First=200,Stride=60};
    int action=Idle,frame=0;
    std::uint16_t elapsed=0;
    void Start(int next) {action=next;frame=0;elapsed=0;}
    int FirstFrame(int job,int gender) const {return First+Stride*(action+5*(gender+2*job));}
    int ImageFrame(int job,int gender) const {return FirstFrame(job,gender)+frame;}
    bool Due(unsigned ms) const {return std::uint16_t(elapsed+ms)>Interval;}
    bool Advance(unsigned ms,int job,int gender) {
        elapsed=std::uint16_t(elapsed+ms);
        if(elapsed<=Interval) return false;
        elapsed=0;
        const bool voice=frame==0 && (action==Select || action==Create);
        static constexpr int last[]={10,19,11,7,17,10,10,10,11,15,10,11,10,10,14,
            10,17,10,8,16,10,16,19,11,16,10,16,10,9,14};
        if(++frame>last[action+5*(gender+2*job)]) {
            frame=0;
            if(action==Select) action=Selected;
            else if(action==Deselect) action=Idle;
        }
        return voice;
    }
};
