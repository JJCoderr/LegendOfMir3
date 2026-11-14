#pragma once
#include "protocol/message_ids.hpp"
#include "resources/map_terrain.hpp"
#include "hud/human_actions.hpp"

namespace mir3 {
struct ForcedMotion {
    int opcode,facing;
    MapCell destination;
    static bool accepts(int opcode) {
        return opcode==protocol::SM_RUSH || opcode==protocol::SM_BUMP ||
            opcode==protocol::SM_BACKSTEP || opcode==protocol::SM_REVIVE_REFRESH;
    }
    int direction() const {return opcode==protocol::SM_BACKSTEP?(facing+4)%8:facing;}
    MapCell origin() const {
        const auto d=map_directions[direction()];
        return {destination[0]-d[0],destination[1]-d[1]};
    }
    bool walking() const {return opcode==protocol::SM_REVIVE_REFRESH;}
    int increment() const {return walking()?1:2;}
    bool returns(int frame) const {return opcode==protocol::SM_BUMP && frame>=4;}
    int pose(int frame,bool mounted) const {
        const int action=walking()?(mounted?30:21):opcode==protocol::SM_BACKSTEP?5:23;
        return HumanActions::frame(action,facing,opcode==protocol::SM_BACKSTEP?0:frame);
    }
};
}
