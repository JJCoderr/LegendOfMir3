#pragma once
#include <cstdint>
namespace mir3 {
struct ExitTransition {
    bool active=false;
    std::uint32_t elapsed=0;
    void begin() { active=true;elapsed=0; }
    bool advance(std::uint32_t milliseconds) {
        if (!active) return false;
        elapsed+=milliseconds;
        if (elapsed<=2500) return false;
        active=false;elapsed=0;return true;
    }
    float brightness() const { return active?float((255.0-double(elapsed)*double(0.1f))*double(1.f/255.f)):1.f; }
};
struct CharacterSelectionPage {
    bool visible=false,connecting=true;
    static constexpr int width=640,height=480;
};
}
