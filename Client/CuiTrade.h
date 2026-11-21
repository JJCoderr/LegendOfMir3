#pragma once

#include "CuiInventory.h"
#include "CuiLegacy/hud/draggable_panel.hpp"
#include "CuiLegacy/hud/trade_scroll.hpp"

namespace mir3_cui {

enum class TradePrompt { None, Failed, Quantity, Gold };

struct TradePanel : mir3::DraggablePanel {
    static constexpr int Columns = 5, Rows = 40, Capacity = 12;
    using Items = std::vector<InventoryItem>;
    struct Feedback { int message, sample, itemMode, subMode; std::string item; };

    TradePanel() : DraggablePanel({0,0,494,330}) {}
    std::array<Items,2> items;
    std::array<std::uint32_t,2> gold{};
    mir3::TradeScroll scroll;
    std::string partner;
    std::vector<Feedback> feedback;
    bool repositionInventory = false;
    bool confirmed = false, confirmPressed = false, goldPending = false, returnPending = false;
    int offerSlot = -1, returnSlot = -1;
    unsigned offerCount = 0;
    InventoryItem offeredItem;
    TradePrompt prompt = TradePrompt::None;
    unsigned editor = 0, openRevision = 0;
    char input[32]{};

    void OpenPrompt(TradePrompt next)
    {
        prompt = next;
        ++editor;
        input[0] = '\0';
    }

    void Clear()
    {
        const auto bounds = rect;
        const auto revision = openRevision;
        const auto inputId = editor;
        auto pendingFeedback = std::move(feedback);
        *this = {};
        rect = bounds;
        openRevision = revision;
        editor = inputId;
        feedback = std::move(pendingFeedback);
    }

    static const InventoryItem* AtCell(const Items& side, int cell)
    {
        const auto item = std::find_if(side.begin(),side.end(),[cell](const auto& value) {
            return cell>=0 && cell<Columns*Rows && cell%Columns>=value.cell%Columns &&
                cell%Columns<value.cell%Columns+value.width && cell/Columns>=value.cell/Columns &&
                cell/Columns<value.cell/Columns+value.height;
        });
        return item == side.end() ? nullptr : &*item;
    }

    const InventoryItem* SelectedReturn() const { return AtCell(items[0],returnSlot); }

    static bool Fits(const Items& side, const InventoryItem& item, int slot)
    {
        if (slot<0 || slot>=Columns*Rows || item.width<1 || item.height<1 ||
            slot%Columns+item.width>Columns || slot/Columns+item.height>Rows) return false;
        const int x = slot%Columns, y = slot/Columns;
        return std::none_of(side.begin(),side.end(),[&](const auto& other) {
            const int ox = other.cell%Columns, oy = other.cell/Columns;
            return x<ox+other.width && x+item.width>ox && y<oy+other.height && y+item.height>oy;
        });
    }

    static bool Insert(Items& side, InventoryItem item, int preferred = -1)
    {
        if (side.size()>=Capacity) return false;
        if (!Fits(side,item,preferred)) {
            preferred = 0;
            while (preferred<Columns*Rows && !Fits(side,item,preferred)) ++preferred;
            if (preferred==Columns*Rows) return false;
        }
        item.cell = preferred;
        side.push_back(std::move(item));
        return true;
    }
};

} // namespace mir3_cui
