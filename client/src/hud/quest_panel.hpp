#pragma once
#include "inventory/inventory_panel.hpp"

namespace mir3 {
struct QuestPanel : DraggablePanel {
    QuestPanel():DraggablePanel({0,0,340,440}) {}
    static constexpr int background=1420;
    static constexpr std::array<std::array<int,3>,2> buttons{{
        {1422,267,75}, {1425,267,107}
    }};
    int pressed=-1;
};
}
