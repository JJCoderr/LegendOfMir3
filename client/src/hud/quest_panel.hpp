#pragma once
#include "inventory/inventory_panel.hpp"
#include "protocol/quest_wire.hpp"
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
    struct Entry {
        std::string title;
        bool completed=false;
        std::vector<std::string> details;
        enum class Detail { idle, pending, received };
        Detail detail=Detail::idle;
        std::optional<QuestRecord> wire;
    };
    std::vector<Entry> entries;
    bool list_initialized=false;

    void reset_list_view() {
        selected=expanded=-1;
        first=0;
        scroll_captured=false;
    }

    void sync_list(std::vector<QuestRecord> records,bool full) {
        if(full) {
            entries.clear();
            list_initialized=!records.empty();
        }
        for(auto& record:records) {
            if(!full && std::ranges::any_of(entries,[&](const Entry& entry) {
                return entry.wire && entry.wire->id==record.id;
            })) break;
            Entry entry;
            entry.title=record.title;
            entry.completed=record.completed;
            entry.wire=std::move(record);
            entries.push_back(std::move(entry));
        }
        std::ranges::stable_sort(entries,{},[](const Entry& entry){return entry.wire->id;});
        reset_list_view();
    }

    int pressed=-1, selected=-1, expanded=-1, first=0;

    static constexpr int scroll_x=272, scroll_top=160, scroll_travel=182;
    bool scroll_captured=false;

    int detail_rows() const { return expanded>=0?int(entries[expanded].details.size()):0; }
    int last_row() const { return std::max(0,int(entries.size())-1)+detail_rows(); }
    int entry_at(int row) const {
        if (expanded>=0 && row>expanded) {
            if (row<=expanded+detail_rows()) return -1;
            row-=detail_rows();
        }
        return row>=0 && row<int(entries.size())?row:-1;
    }
    int thumb_y() const {
        return rect[1]+scroll_top+(last_row()?int(std::int64_t(scroll_travel)*first/last_row()):0);
    }
    void scroll(int delta) {
        first=int(std::clamp<std::int64_t>(std::int64_t(first)+delta,0,last_row()));
    }
    bool scroll_pointer(int kind,int x,int y,int width,int height) {
        if (kind==2) return std::exchange(scroll_captured,false);
        const int top=rect[1]+scroll_top;
        if (kind==1 && scroll_captured) {
            const int offset=std::clamp(y-top,0,scroll_travel+height);
            first=int(std::int64_t(last_row())*offset/(scroll_travel+height));
            return true;
        }
        if (kind!=0 || x<rect[0]+scroll_x || x>=rect[0]+scroll_x+width ||
            y<top || y>=top+scroll_travel+height) return false;
        if (y>=thumb_y() && y<thumb_y()+height) scroll_captured=true;
        else scroll(y<thumb_y()+7?-5:5);
        return true;
    }
    std::uint32_t title_color(int index) const {
        if (index==selected) return entries[index].completed?0x191a32:0x1919c8;
        return entries[index].completed?0x191a64:0x191996;
    }
};
}
