#pragma once

// Included after StdAfx.h and Shared/DebugState.h. Captured only when debug state is enabled.
namespace mir3 {
inline std::string ItemDebugState(const CGameProcess& game)
{
    const auto& ui = game.m_xInterface;
    const auto& ability = game.m_xMyHero.m_stAbility;
    const auto range = [](WORD value) {
        return "[" + std::to_string(LOBYTE(value)) + "," + std::to_string(HIBYTE(value)) + "]";
    };
    const auto item = [](const CItem& value, int slot) {
        return std::string("{\"slot\":") + std::to_string(slot)
            + ",\"id\":" + std::to_string(value.m_stItemInfo.nMakeIndex)
            + ",\"looks\":" + std::to_string(value.m_stItemInfo.stStdItem.wLooks)
            + ",\"shape\":" + std::to_string(value.m_stItemInfo.stStdItem.bShape)
            + ",\"name\":" + QuoteLocalText(value.Name()) + "}";
    };
    const auto list = [&](const auto& slots) {
        std::string result = "[";
        for (int i = 0; i < _countof(slots); ++i) {
            if (!slots[i].bSetted) continue;
            if (result.size() > 1) result += ',';
            result += item(slots[i].xItem, i);
        }
        return result + ']';
    };
    const auto panel = [](const CGameWnd& window) {
        return std::string("{\"open\":") + BoolJson(window.m_bActive != FALSE)
            + ",\"x\":" + std::to_string(window.m_rcWnd.left)
            + ",\"y\":" + std::to_string(window.m_rcWnd.top) + "}";
    };
    return ",\"dress\":" + std::to_string(game.m_xMyHero.m_stFeature.bDress)
        + ",\"defence\":" + range(ability.wAC)
        + ",\"magic_defence\":" + range(ability.wMAC)
        + ",\"bag\":" + list(ui.m_xInventoryWnd.m_stInventoryItem)
        + ",\"equipment\":" + list(ui.m_xStatusWnd.m_stEquipItem)
        + ",\"held_item\":" + (ui.m_stCommonItem.bSetted ? item(ui.m_stCommonItem.xItem, -1) : "null")
        + ",\"equipment_pending\":" + std::to_string(ui.m_xStatusWnd.m_equipmentRequest)
        + ",\"bag_panel\":" + panel(ui.m_xInventoryWnd)
        + ",\"equipment_panel\":" + panel(ui.m_xStatusWnd);
}
}
