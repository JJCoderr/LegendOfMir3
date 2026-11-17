#pragma once
#include <array>
#include <cstdint>
namespace mir3 {
struct ForcedMotion {
    static constexpr unsigned Rush=6,Bump=7,Backstep=9,Interval=95,Frames=6,ReturnFrame=4;
    static constexpr std::array<std::array<int,2>,8> Steps={{{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}}};
    unsigned opcode=Rush,facing=0,frame=0;
    std::uint32_t elapsed=0;
    std::array<int,2> origin={},destination={};
    unsigned direction() const {return opcode==Backstep?(facing+4)%8:facing;}
    static ForcedMotion begin(unsigned opcode,unsigned facing,int x,int y) {
        ForcedMotion result;result.opcode=opcode;result.facing=facing%8;result.destination={x,y};
        const auto step=Steps[result.direction()];result.origin={x-step[0],y-step[1]};return result;
    }
    bool advance(std::uint32_t ms) {
        elapsed+=ms;if(elapsed<=Interval) return false;
        elapsed=0;frame+=2;return true;
    }
    bool returned() const {return opcode==Bump && frame>=ReturnFrame;}
    bool finished() const {return returned() || frame>=Frames;}
    std::array<int,2> cell() const {
        return returned() || frame<(direction()==7?4u:1u)?origin:destination;
    }
    std::array<int,2> offset() const {
        if(finished()) return {};
        const bool committed=frame>=(direction()==7?4u:1u);
        const int frames=committed?int(frame)-5:int(frame)+1;
        const auto step=Steps[direction()];
        const int y=committed && direction()%2?32*step[1]/6:5*step[1];
        return {step[0]*8*frames,y*frames};
    }
};
}
