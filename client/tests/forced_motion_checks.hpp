#pragma once
#include "hud/forced_motion.hpp"
#include <stdexcept>
// 测试内容：八方向起点推导、后退反向、冲撞折返帧、双步推进和骑乘/后退姿态。
inline void forced_motion_checks() {
    using namespace mir3;
    const auto require=[](bool ok,const char* message) {if(!ok) throw std::runtime_error(message);};
    for (int facing=0;facing<8;++facing) {
        ForcedMotion rush{protocol::SM_RUSH,facing,{100,100}};
        require(rush.origin()==MapCell{100-map_directions[facing][0],100-map_directions[facing][1]},"rush origin");
        ForcedMotion back{protocol::SM_BACKSTEP,facing,{100,100}};
        require(back.origin()==MapCell{100+map_directions[facing][0],100+map_directions[facing][1]},"reverse origin");
        require(back.pose(0,false)==400+10*facing && back.pose(4,false)==back.pose(0,false),"fixed backstep pose");
    }
    ForcedMotion bump{protocol::SM_BUMP,2,{100,100}};
    require(!bump.returns(2) && bump.returns(4) && bump.increment()==2,"bump returns at four");
    ForcedMotion walk{protocol::SM_REVIVE_REFRESH,4,{100,100}};
    require(walk.increment()==1 && walk.pose(2,false)==1722 && walk.pose(2,true)==2442,"forced walk mount pose");
    require(!ForcedMotion::accepts(protocol::SM_WALK) && ForcedMotion::accepts(protocol::SM_RUSH),"local dispatch distinct from ordinary echo");
}
