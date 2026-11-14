#pragma once
#include <array>
#include <cstdint>
namespace mir3 {
// 0x421F60: +628E0, +628DC, +628D4 on the local actor.
struct ExitGuard {
    std::array<std::uint32_t,3> ticks{};
    std::uint8_t action=0;
    bool allowed(std::uint32_t now) const {
        if (action==19) return true;
        for (const auto tick:ticks) if (std::uint32_t(now-tick)<=8000u) return false;
        return true;
    }
};
}
