#pragma once
#include "protocol/message_ids.hpp"
#include "inventory/inventory_panel.hpp"

namespace mir3 {
struct MountPanel : DraggablePanel {
    MountPanel():DraggablePanel({0,0,276,322}) {}
    static constexpr int background=1400;
    static constexpr std::array<std::array<int,4>,5> buttons{{
        {1221,236,279,1052}, {1402,19,232,4100}, {1405,77,232,4101},
        {1408,134,232,4102}, {1410,191,232,4103}
    }};
    int pressed=-1;
    std::uint8_t shape=0;

    bool apply_reply(int identifier,std::uint32_t recognition) {
        if (identifier!=mir3::protocol::SM_RIDEHORSE) return false;
        // 0x4282C0 case 1300 copies the low recognition byte into the rider state.
        shape=static_cast<std::uint8_t>(recognition);
        return true;
    }

    int command(int button) const {
        switch (button) {
        case 1: return shape?0:4106;
        case 2: return shape?4105:0;
        case 3: return 4104;
        case 4: return 4105;
        default: return 0;
        }
    }
};
}
