#pragma once

#include "hud/human_actions.hpp"
#include "magic/spell_routes.hpp"
#include "resources/effect_catalog.hpp"
#include <optional>

namespace mir3 {
struct CastVisual {
    HumanActionClock clock;
    std::optional<EffectResource> resource;
    int direction = 0;
    std::array<std::uint8_t,3> tint{};
    std::uint64_t tick = 0;

    void advance(std::uint64_t now) {
        if (clock.finished()) return;
        clock.advance(static_cast<std::uint32_t>(now-tick));
        tick=now;
    }
    int pose() const {
        return clock.finished() ? -1 : HumanActions::table[clock.action()].start+clock.frame();
    }
};
}
