#pragma once
#include "inventory/inventory_panel.hpp"
#include <string>
#include <vector>

namespace mir3 {
struct QuestPanel : DraggablePanel {
    QuestPanel():DraggablePanel({0,0,340,440}) {}
    static constexpr int background=1420;
    static constexpr std::array<std::array<int,3>,2> buttons{{
        {1422,267,75}, {1425,267,107}
    }};
    static constexpr int visible_rows=19;
    struct Entry { std::string title; bool completed=false; };
    std::vector<Entry> entries;
    int pressed=-1, selected=-1, first=0;

    void scroll(int delta) {
        first=std::clamp(first+delta,0,std::max(0,int(entries.size())-visible_rows));
    }
    std::uint32_t title_color(int index) const {
        if (index==selected) return entries[index].completed?0x191a32:0x1919c8;
        return entries[index].completed?0x191a64:0x191996;
    }
};
}
