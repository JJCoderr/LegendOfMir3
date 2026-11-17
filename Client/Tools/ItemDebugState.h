#pragma once
#include "../../Server/Def/Mir3Item.h"

// Included after StdAfx.h and Shared/DebugState.h. Captured only when debug state is enabled.
namespace mir3 {
inline std::string ItemDebugState(CGameProcess& game)
{
    const auto& ui = game.m_xInterface;
    const auto& ability = game.m_xMyHero.m_stAbility;
    const auto range = [](WORD value) {
        return "[" + std::to_string(LOBYTE(value)) + "," + std::to_string(HIBYTE(value)) + "]";
    };
    const auto item = [&](const CItem& value, int slot, int cell = -1) {
        std::string upgrades="[";
        for (int at=Mir3Item::Upgrades;at<Mir3Item::Size;++at) {
            if(at!=Mir3Item::Upgrades) upgrades+=',';
            upgrades+=std::to_string(value.m_serverRecord[at]);
        }
        upgrades+=']';
        const auto base=[&](int at) { return range(WORD(value.m_serverRecord[at]|(WORD(value.m_serverRecord[at+1])<<8))); };
        return std::string("{\"slot\":") + std::to_string(slot)
            + ",\"cell\":" + std::to_string(cell)
            + ",\"id\":" + std::to_string(value.m_stItemInfo.nMakeIndex)
            + ",\"looks\":" + std::to_string(value.m_stItemInfo.stStdItem.wLooks)
            + ",\"shape\":" + std::to_string(value.m_stItemInfo.stStdItem.bShape)
            + ",\"mode\":" + std::to_string(value.m_stItemInfo.stStdItem.bStdMode)
            + ",\"standard_durability\":" + std::to_string(value.m_stItemInfo.stStdItem.wDuraMax)
            + ",\"durability\":" + std::to_string(value.m_stItemInfo.nDura)
            + ",\"max_durability\":" + std::to_string(value.m_stItemInfo.nDuraMax)
            + ",\"upgrades\":" + upgrades
            + ",\"base_defence\":" + base(Mir3Item::Defence)
            + ",\"base_magic_defence\":" + base(Mir3Item::MagicDefence)
            + ",\"base_attack\":" + base(Mir3Item::Attack)
            + ",\"name\":" + QuoteLocalText(value.Name()) + "}";
    };
    const auto list = [&](const auto& slots, bool bag = false) {
        std::string result = "[";
        for (int i = 0; i < _countof(slots); ++i) {
            if (!slots[i].bSetted) continue;
            if (result.size() > 1) result += ',';
            result += item(slots[i].xItem, i, bag ? ui.m_xInventoryWnd.ItemAnchor(i) : -1);
        }
        return result + ']';
    };
    const auto panel = [](const CGameWnd& window) {
        return std::string("{\"open\":") + BoolJson(window.m_bActive != FALSE)
            + ",\"x\":" + std::to_string(window.m_rcWnd.left)
            + ",\"y\":" + std::to_string(window.m_rcWnd.top) + "}";
    };
    int bagCount = 0;
    for (const auto& slot : ui.m_xInventoryWnd.m_stInventoryItem) if (slot.bSetted) ++bagCount;
    std::string ground="[";
    auto& items=game.m_stMapItemList;items.MoveCurrentToTop();
    for(int i=0;i<items.GetCounter();++i) {
        const auto item=items.GetCurrentData();if(i) ground+=',';
        ground+="{\"id\":"+std::to_string(item->nRecog)+",\"x\":"+std::to_string(item->shTileX)
            +",\"y\":"+std::to_string(item->shTileY)+",\"name\":"+QuoteLocalText(item->szItemName)+"}";
        items.MoveNextNode();
    }
    ground+=']';
    return ",\"ground_items\":" + ground
        + ",\"bag_count\":" + std::to_string(bagCount)
        + ",\"bag_capacity\":" + std::to_string(_MAX_INVEN_ITEM)
        + ",\"drop_pending\":" + BoolJson(ui.m_xInventoryWnd.m_pendingDrop.bSetted != FALSE)
        + ",\"use_pending\":" + BoolJson(ui.m_xInventoryWnd.m_usePending)
        + ",\"dress\":" + std::to_string(game.m_xMyHero.m_stFeature.bDress)
        + ",\"defence\":" + range(ability.wAC)
        + ",\"magic_defence\":" + range(ability.wMAC)
        + ",\"bag_scroll_row\":" + std::to_string(ui.m_xInventoryWnd.ScrollRow())
        + ",\"bag\":" + list(ui.m_xInventoryWnd.m_stInventoryItem, true)
        + ",\"equipment\":" + list(ui.m_xStatusWnd.m_stEquipItem)
        + ",\"held_item\":" + (ui.m_stCommonItem.bSetted ? item(ui.m_stCommonItem.xItem, -1) : "null")
        + ",\"equipment_pending\":" + std::to_string(ui.m_xStatusWnd.m_equipmentRequest)
        + ",\"bag_panel\":" + panel(ui.m_xInventoryWnd)
        + ",\"equipment_panel\":" + panel(ui.m_xStatusWnd);
}
}
