#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace mir3_cui {

enum class ItemRequest { None, Use, Drop, DropStack, DropGold };
enum class DropDialog { None, Item, Gold };

struct ItemKey {
    std::uint32_t id = 0;
    std::string name;

    bool operator==(const ItemKey& other) const { return id == other.id && name == other.name; }
};

struct InventoryItem {
    ItemKey key;
    std::string label;
    unsigned count = 0;
    unsigned durability = 0;
    unsigned maxDurability = 0;
    unsigned mode = 0;
    unsigned stackMode = 0;

    bool Stacks() const { return stackMode >= 2; }
    bool IsGold() const { return key.name == "\xbd\xf0\xb1\xd2"; }
};

struct InventoryPanel {
    std::vector<InventoryItem> items;
    ItemKey selection;
    InventoryItem pending;
    std::uint32_t gold = 0;
    std::uint64_t deadline = 0;
    unsigned dropCount = 0;
    ItemRequest request = ItemRequest::None;
    DropDialog drop = DropDialog::None;
    char amount[12] = {};
    int editor = 0;
    bool open = false;
    bool initialized = false;
    bool waiting = false;
    bool notice = false;
    std::string status;

    InventoryItem* Find(const ItemKey& key)
    {
        const auto item = std::find_if(items.begin(), items.end(), [&](const InventoryItem& value) {
            return value.key == key;
        });
        return item == items.end() ? nullptr : &*item;
    }

    void Remove(const ItemKey& key)
    {
        const auto item = std::find_if(items.begin(), items.end(), [&](const InventoryItem& value) {
            return value.key == key;
        });
        if (item != items.end()) items.erase(item);
        if (selection == key) selection = {};
    }
};

} // namespace mir3_cui
