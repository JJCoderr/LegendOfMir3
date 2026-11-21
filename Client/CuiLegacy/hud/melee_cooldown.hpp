#pragma once
#include <algorithm>
#include <cstdint>

namespace mir3 {
// Level, signed attack speed, and hand load determine melee cadence (417C40).
struct MeleeCooldown {
    std::uint32_t last_allowed=0;
    std::int16_t speed=0;

    static constexpr std::uint32_t interval(unsigned level,std::int16_t speed,
            unsigned hand,unsigned capacity) {
        const int discount=std::min(800,std::min(370,int(level)*14)+60*int(speed));
        return std::uint32_t(1400-discount+(hand>capacity?1500:0));
    }
    bool accept(std::uint32_t now,unsigned level,unsigned hand,unsigned capacity) {
        if (std::uint32_t(now-last_allowed)<=interval(level,speed,hand,capacity)) return false;
        last_allowed=now;
        return true;
    }
};
}
