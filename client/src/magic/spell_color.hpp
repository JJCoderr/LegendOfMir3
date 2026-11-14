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
};
}
