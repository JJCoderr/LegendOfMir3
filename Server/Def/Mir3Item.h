#pragma once
#include <array>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace Mir3Item {
constexpr int Size = 100, EncodedSize = 134, LegacyKeySize = 12;
enum Offset {
    Name = 0, Mode = 30, Shape = 31, Weight = 32, Animation = 33,
    Source = 34, Identify = 35, Looks = 36, StandardDurability = 38,
    Defence = 40, MagicDefence = 42, Attack = 44,
    Requirement = 54, RequirementValue = 55, Price = 56,
    StackMode = 61, Instance = 62, Durability = 66, MaxDurability = 68,
    Count = 71, Upgrades = 86, WeaponStrengthBonus = 93
};
using Record = std::array<unsigned char, Size>;
inline void Word(Record& record, int at, unsigned value) {
    record[at] = static_cast<unsigned char>(value);
    record[at + 1] = static_cast<unsigned char>(value >> 8);
}
inline void Dword(Record& record, int at, std::uint32_t value) {
    Word(record, at, value); Word(record, at + 2, value >> 16);
}

template<class Legacy>
Record FromLegacy(const Legacy& item, std::uint32_t instance)
{
    Record record = {};
    const auto& standard = item.tStdItem;
    std::memcpy(record.data() + Name, standard.szName, sizeof(standard.szName));
    record[Mode] = standard.btStdMode; record[Shape] = standard.btShape;
    record[Weight] = standard.btWeight; record[Animation] = standard.btAniCount;
    record[Source] = standard.btSource; record[Identify] = standard.btNeedIdentify;
    Word(record, Looks, standard.wLooks);
    Word(record, StandardDurability, standard.wDuraMax);
    Word(record, Defence, standard.wAC); Word(record, MagicDefence, standard.wMAC);
    Word(record, Attack, standard.wDC);
    record[Requirement] = standard.btNeed; record[RequirementValue] = standard.btNeedLevel;
    Dword(record, Price, standard.nPrice); Dword(record, Instance, instance);
    Word(record, Durability, item.nDura); Word(record, MaxDurability, item.nDuraMax);
    Word(record, Count, 1);
    return record;
}

// Reference 0x514270 copies 14 instance bonuses, clipping weapon strength only on the wire.
inline void ApplyUpgrades(Record& record, const unsigned char* values) {
    constexpr int UpgradeCount=14, FirstWeaponMode=5, LastWeaponMode=6, MaximumStrengthBonus=5;
    std::memcpy(record.data()+Upgrades,values,UpgradeCount);
    if (record[Mode]>=FirstWeaponMode && record[Mode]<=LastWeaponMode && record[WeaponStrengthBonus]>MaximumStrengthBonus)
        record[WeaponStrengthBonus]=MaximumStrengthBonus;
}

// IDs live for one character connection. Resolve against owned items before use.
// Equipment uses its full legacy key; general items also use their live record.
// Equal potion keys describe equal attributes, not the same inventory instance.
class Identities {
    struct Entry { std::string key; const void* item; bool active; };
    std::vector<Entry> keys;
public:
    void Clear() { keys.clear(); }
    std::uint32_t Get(const char* key, const void* item = nullptr) {
        const std::string value(key, LegacyKeySize);
        for (std::size_t i = 0; i < keys.size(); ++i)
            if (keys[i].active && keys[i].key == value && keys[i].item == item) return static_cast<std::uint32_t>(i + 1);
        keys.push_back({value, item, true});
        return static_cast<std::uint32_t>(keys.size());
    }
    bool Matches(std::uint32_t id, const void* item) const {
        return id && id <= keys.size() && keys[id - 1].active && keys[id - 1].item == item;
    }
    void Forget(const void* item) {
        for (auto& entry : keys) if (entry.item == item) entry.active = false;
    }
    bool Resolve(std::uint32_t id, char* key) const {
        if (!id || id > keys.size() || !keys[id - 1].active) return false;
        std::memcpy(key, keys[id - 1].key.data(), LegacyKeySize);
        key[LegacyKeySize] = 0;
        return true;
    }
};
}
