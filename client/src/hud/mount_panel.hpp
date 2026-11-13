#pragma once
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
    bool mounted=false;

    int command(int button) const {
        switch (button) {
        case 1: return mounted?0:4106;
        case 2: return mounted?4105:0;
        case 3: return 4104;
        case 4: return 4105;
        default: return 0;
        }
    }
};
}
