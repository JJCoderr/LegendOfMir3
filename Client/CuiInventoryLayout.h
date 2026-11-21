#pragma once

#include "CuiInventory.h"

namespace mir3_cui {

struct InventoryLayout {
    static constexpr int BagStride = 3120;
    static constexpr int ShortcutStride = 3112;
    static constexpr int CellOffset = static_cast<int>(InventoryPanel::Capacity) * BagStride;
    static constexpr int ShortcutOffset = CellOffset + InventoryPanel::Cells * 2;
    static constexpr int FileSize = ShortcutOffset + 6 * ShortcutStride;
    static constexpr unsigned EmptyCell = 0xffff;
    static constexpr unsigned Anchor = 1000;

    static std::string Encode(InventoryPanel& bag)
    {
        std::string data(FileSize, '\0');
        std::fill(data.begin() + CellOffset, data.begin() + ShortcutOffset, static_cast<char>(0xff));
        int index = 0;
        for (const auto& item : bag.items) {
            if (index == InventoryPanel::Capacity) break;
            if (!InventoryPanel::OnGrid(item, item.cell)) continue;
            const int at = index * BagStride;
            data[at] = 1;
            WriteWord(data, at + 4, item.width);
            WriteWord(data, at + 8, item.height);
            WriteItem(data, at + 16, item);
            for (int y = 0; y < item.height; ++y)
                for (int x = 0; x < item.width; ++x)
                    WriteWord(data, CellOffset + (item.cell + y * InventoryPanel::Columns + x) * 2, index);
            WriteWord(data, CellOffset + item.cell * 2, Anchor + index);
            ++index;
        }
        for (std::size_t slot = 0; slot < bag.shortcuts.size(); ++slot) {
            const auto* item = bag.ResolveShortcut(slot);
            if (!item) continue;
            const int at = ShortcutOffset + static_cast<int>(slot) * ShortcutStride;
            data[at] = 1;
            WriteItem(data, at + 8, *item);
        }
        return data;
    }

    static bool Restore(const std::string& data, std::vector<InventoryItem>& records,
        std::array<ItemKey, 6>* shortcuts)
    {
        if (data.size() != FileSize) return false;
        std::array<int, InventoryPanel::Capacity> anchors;
        anchors.fill(-1);
        std::array<unsigned, InventoryPanel::Capacity> footprints{};
        for (int cell = 0; cell < InventoryPanel::Cells; ++cell) {
            const auto code = Word(data, CellOffset + cell * 2);
            if (code == EmptyCell) continue;
            const auto index = code % Anchor;
            if (code >= 2 * Anchor || index >= footprints.size()) return false;
            ++footprints[index];
            if (code >= Anchor) anchors[index] = cell;
        }

        for (std::size_t index = 0; index < anchors.size(); ++index) {
            const int at = static_cast<int>(index) * BagStride;
            if (Dword(data, at) == 0 || anchors[index] < 0) continue;
            const auto name = Name(data, at + 16);
            const auto id = Dword(data, at + 78);
            const auto durability = Word(data, at + 82);
            const auto maximum = Word(data, at + 84);
            const auto found = std::find_if(records.begin(), records.end(), [&](const InventoryItem& item) {
                return item.cell < 0 && item.key.id == id && item.key.name == name &&
                    item.durability == durability && item.maxDurability == maximum;
            });
            const int anchor = anchors[index];
            if (found == records.end() || !InventoryPanel::OnGrid(*found, anchor) ||
                footprints[index] != static_cast<unsigned>(found->width * found->height)) continue;
            bool matches = true;
            for (int y = 0; y < found->height && matches; ++y) {
                for (int x = 0; x < found->width; ++x) {
                    const int position = anchor + y * InventoryPanel::Columns + x;
                    if (Word(data, CellOffset + position * 2) != index + (x == 0 && y == 0 ? Anchor : 0)) {
                        matches = false;
                        break;
                    }
                }
            }
            if (matches) found->cell = anchor;
        }

        if (shortcuts) {
            for (std::size_t slot = 0; slot < shortcuts->size(); ++slot) {
                const int at = ShortcutOffset + static_cast<int>(slot) * ShortcutStride;
                if (Dword(data, at) == 0) continue;
                const auto name = Name(data, at + 8);
                const auto found = std::find_if(records.rbegin(), records.rend(), [&](const InventoryItem& item) {
                    return item.key.name == name && item.QuickEligible();
                });
                if (found != records.rend() && std::none_of(shortcuts->begin(), shortcuts->end(),
                    [&](const ItemKey& bound) { return bound.name == name; }))
                    (*shortcuts)[slot] = found->key;
            }
        }
        return true;
    }

private:
    static unsigned Word(const std::string& data, int at)
    {
        return static_cast<unsigned char>(data[at]) | (unsigned(static_cast<unsigned char>(data[at + 1])) << 8);
    }

    static std::uint32_t Dword(const std::string& data, int at)
    {
        return Word(data, at) | (std::uint32_t(Word(data, at + 2)) << 16);
    }

    static std::string Name(const std::string& data, int at)
    {
        const auto first = data.begin() + at;
        return std::string(first, std::find(first, first + 30, '\0'));
    }

    static void WriteWord(std::string& data, int at, unsigned value)
    {
        data[at] = static_cast<char>(value);
        data[at + 1] = static_cast<char>(value >> 8);
    }

    static void WriteItem(std::string& data, int at, const InventoryItem& item)
    {
        std::copy(item.key.name.begin(), item.key.name.end(), data.begin() + at);
        data[at + 30] = static_cast<char>(item.mode);
        WriteWord(data, at + 36, item.icon);
        data[at + 61] = static_cast<char>(item.stackMode);
        WriteWord(data, at + 62, item.key.id);
        WriteWord(data, at + 64, item.key.id >> 16);
        WriteWord(data, at + 66, item.durability);
        WriteWord(data, at + 68, item.maxDurability);
        WriteWord(data, at + 71, item.count);
    }
};

} // namespace mir3_cui
