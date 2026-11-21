#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace mir3 {
// Shared movement and melee admission timing from 417D50 and 419B40.
struct LocalActionClock {
    std::uint32_t moved=0, attacked=0;
    static constexpr std::uint32_t remaining(std::uint32_t now,std::uint32_t since,std::uint32_t duration) {
        const auto elapsed=std::uint32_t(now-since);
        return elapsed>=duration?0:duration-elapsed;
    }
    std::uint32_t shared_wait(std::uint32_t now,std::uint32_t attack_interval) const {
        // Round up to preserve the original double comparison.
        const auto attack_wait=std::uint32_t(std::ceil(double(attack_interval)*0.8));
        return std::max({remaining(now,attacked,attack_wait),remaining(now,moved,480),
            remaining(now,0,800)});
    }
};
}
