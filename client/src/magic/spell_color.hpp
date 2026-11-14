#pragma once
#include <array>
#include <cstdint>

namespace mir3 {
struct SpellColor {
    using Rgb=std::array<std::uint8_t,3>;
    // 40DB10读取4B51FC的七系颜色；严格大于才替换，平局保留较早的元素。
    static constexpr std::array<Rgb,7> palette{{
        {{240,180,100}},{{100,140,190}},{{50,70,200}},{{140,200,150}},
        {{200,200,200}},{{160,100,180}},{{200,220,130}}
    }};
    static constexpr Rgb strongest(const std::array<std::uint16_t,7>& attack,Rgb fallback) {
        std::uint16_t maximum=0;
        for (unsigned i=0;i<attack.size();++i) {
            if (attack[i]<=maximum) continue;
            maximum=attack[i];fallback=palette[i];
        }
        return fallback;
    }
    // 412C80 supplies a zero-based element in the attack header's high byte.
    static constexpr int weapon_effect(unsigned element) {
        return element<palette.size()?408+int(element):0;
    }
    // 装备窗口+928+3112*武器槽+8为100字节原包：78C191/192/1C1对应49/50/97。
    enum WeaponField : unsigned { element=49, power=50, special_element=97 };
    static constexpr int local_weapon_effect(const std::array<std::uint8_t,100>& record) {
        const auto special=record[special_element];
        const unsigned selector=special>0 && special<128?special:
            (record[power] || record[power+1]?record[element]:0);
        return selector>=1 && selector<=palette.size()?weapon_effect(selector-1):0;
    }
    static constexpr Rgb local_weapon_color(const std::array<std::uint8_t,100>& record,
        const std::array<std::uint16_t,7>& attack,Rgb fallback) {
        const auto special=record[special_element];
        if (special>=1 && special<=palette.size()) return palette[special-1];
        for (const auto value:attack) if (value) return strongest(attack,fallback);
        const auto base=record[element];
        return base>=1 && base<=palette.size()?palette[base-1]:fallback;
    }
};
}
