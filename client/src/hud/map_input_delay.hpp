#pragma once
#include <algorithm>
#include <cmath>
namespace mir3 {
// 0x437BD8共用动作延迟：换图1500ms，骑马按钮300ms；0x4242C6期满清零。
struct MapInputDelay {
    double remaining_ms=0;
    bool active() const { return remaining_ms>0; }
    void enter() { remaining_ms=1500; }
    void mount_command() { remaining_ms=300; }
    void advance(double ms) {
        if (std::isfinite(ms) && ms>0) remaining_ms=std::max(0.0,remaining_ms-ms);
    }
};
}
