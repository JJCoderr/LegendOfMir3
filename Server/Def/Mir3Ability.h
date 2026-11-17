#pragma once
#include <cstdint>

// MIR3 SM_ABILITY body; do not serialize CObjectAbility's memory layout.
namespace Mir3Ability {
#pragma pack(push, 1)
struct Record {
    std::uint8_t level;
    std::uint16_t defence, magicDefence, attack, magic, spirit;
    std::uint16_t health, mana, maxHealth, maxMana;
    std::uint32_t experience, nextExperience;
    std::uint16_t weight, maxWeight;
    std::uint8_t wear, maxWear, hand, maxHand;
    std::uint8_t extension[28];
};
#pragma pack(pop)
static_assert(sizeof(Record) == 63, "MIR3 ability body size");

template<class Ability>
Record FromAbility(const Ability& value)
{
    Record result = {};
    result.level = value.Level;
    result.defence = value.AC; result.magicDefence = value.MAC;
    result.attack = value.DC; result.magic = value.MC; result.spirit = value.SC;
    result.health = value.HP; result.mana = value.MP;
    result.maxHealth = value.MaxHP; result.maxMana = value.MaxMP;
    result.experience = value.Exp; result.nextExperience = value.MaxExp;
    result.weight = value.Weight; result.maxWeight = value.MaxWeight;
    result.wear = value.WearWeight; result.maxWear = value.MaxWearWeight;
    result.hand = value.HandWeight; result.maxHand = value.MaxHandWeight;
    // The old five element ranges do not define MIR3's seven-element extension.
    // Leave unsupported extension fields neutral instead of reinterpreting bytes.
    return result;
}
}
