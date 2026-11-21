#pragma once

#include "CuiInventory.h"

namespace mir3_cui {

constexpr int EquipmentSlots = 12;
enum class EquipmentRequest { None, Wear, Remove };

inline bool EquipmentFits(unsigned mode, int slot)
{
    switch (mode) {
    case 5: case 6: case 54: case 55: case 56: return slot == 1;
    case 10: case 11: return slot == 0;
    case 15: return slot == 4;
    case 19: case 20: case 21: case 57: return slot == 3;
    case 22: case 23: return slot == 7 || slot == 8;
    case 24: case 26: return slot == 5 || slot == 6;
    case 25: return slot == 5 || slot == 9;
    case 30: return slot == 2;
    case 53: return slot == 10;
    case 60: return slot == 11;
    default: return false;
    }
}

inline bool EquipmentActivates(const InventoryItem& item, int slot)
{
    return !item.key.name.empty() && EquipmentFits(item.mode, slot) &&
        item.mode != 5 && item.mode != 6 && item.mode != 10 && item.mode != 11 &&
        item.mode != 15 && item.mode != 53;
}

struct EquipmentPanel {
    std::array<InventoryItem, EquipmentSlots> slots;
    InventoryItem pending;
    ItemKey previous;
    ItemKey held;
    int heldSlot = -1;
    std::uint64_t lastRelease = 0;
    EquipmentRequest request = EquipmentRequest::None;
    int pendingSlot = -1;
    int selected = 0;
    std::uint64_t deadline = 0;
    bool open = false;
    std::string status;

    void ReleaseHeld()
    {
        held = {};
        heldSlot = -1;
        lastRelease = 0;
    }
};

} // namespace mir3_cui
