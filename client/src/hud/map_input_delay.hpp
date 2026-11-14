#pragma once
#include <algorithm>
#include <cmath>
namespace mir3 {
// 0x42EE30 / 0x4242C6 / 0x42655A: map-entry left-button-down delay.
struct MapInputDelay {
    double remaining_ms=0;
    bool active() const { return remaining_ms>0; }
    void enter() { remaining_ms=1500; }
    void advance(double ms) {
        if (std::isfinite(ms) && ms>0) remaining_ms=std::max(0.0,remaining_ms-ms);
    }
};
}
