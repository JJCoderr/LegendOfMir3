#pragma once
#include "MagicRules.h"
#include <array>
#include <vector>
#include <cstdint>
#include <cstdlib>

namespace mir3 {
// 40DFE0/44C7D0: segment order matters, including coincident ray origins.
struct GroundSpell {
    using Cell=std::array<int,2>;
    struct Segment { Cell cell; std::uint32_t delay; };
    static constexpr int WaveSegments=9,RaySegments=5,WaveDelay=150,LightningDelay=30;
    static constexpr int FireSkipEnd=1915,FireSlowStart=1925,FireSlowInterval=80;
    static constexpr int FireResidueOffset=48,IceResidueOffset=26;
    static constexpr int FireEndOffset=30,IceEndOffset=20;
    static Cell direction(Cell source,Cell target) {
        const int x=target[0]-source[0],y=target[1]-source[1];
        Cell step={(x>0)-(x<0),(y>0)-(y<0)};
        if(std::abs(y)>2 && std::abs(x)<=1) step[0]=0;
        if(std::abs(x)>2 && std::abs(y)<=1) step[1]=0;
        if(step==Cell{}) step[1]=1;
        return step;
    }
    static std::vector<Segment> segments(MagicEffectId effect,Cell source,Cell target,int facing) {
        std::vector<Segment> result;
        if(effect==MagicEffectId::three_rays) {
            if(facing<0 || facing>=8) return result;
            static constexpr Cell headings[8]={{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}};
            static constexpr int rays[8][3]={{0,7,1},{1,0,2},{1,2,3},{3,2,4},
                {4,3,5},{5,4,6},{6,7,5},{7,0,6}};
            const Cell start={source[0]+headings[facing][0],source[1]+headings[facing][1]};
            for(const int ray:rays[facing]) {
                const auto step=direction(start,{source[0]+headings[ray][0]*RaySegments,
                    source[1]+headings[ray][1]*RaySegments});
                for(int i=0;i<RaySegments;++i)
                    result.push_back({{start[0]+step[0]*i,start[1]+step[1]*i},std::uint32_t(WaveDelay*i)});
            }
        } else {
            const auto step=direction(source,target);
            const int interval=effect==MagicEffectId::lightning_trail?LightningDelay:WaveDelay;
            for(int i=0;i<WaveSegments;++i)
                result.push_back({{source[0]+step[0]*(i+1),source[1]+step[1]*(i+1)},std::uint32_t(interval*i)});
        }
        return result;
    }
    struct Change { bool lightning=false,residue=false,ice_particles=false,finished=false; };
    static Change next(MagicEffectId effect,int& frame,int first,int end,bool walkable) {
        Change change;
        const int previous=frame++;
        change.lightning=effect==MagicEffectId::lightning_trail && previous==first;
        const bool fire=effect==MagicEffectId::fire_wave,ice=effect==MagicEffectId::ice_wave;
        if(fire || ice) {
            if(fire && walkable && frame<FireSkipEnd) ++frame;
            const int residue=end-(fire?FireResidueOffset:IceResidueOffset);
            change.residue=walkable && previous<residue && frame>=residue;
            change.ice_particles=ice && walkable && frame<residue;
            if(frame>=end-(fire?FireEndOffset:IceEndOffset)) frame=end;
        }
        change.finished=frame>=end;
        return change;
    }
    static int interval(MagicEffectId effect,int frame,int resource_interval,bool walkable) {
        return effect==MagicEffectId::fire_wave && frame>=FireSlowStart && walkable?
            FireSlowInterval:resource_interval;
    }
};
}
