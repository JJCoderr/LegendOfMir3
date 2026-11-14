#pragma once
#include <algorithm>
namespace mir3 {
// 0x412000 / 0x412350 and the 0x414220 death/revival branches.
struct HumanLife {
    bool dead=false,dying=false;
    double revive_ms=600;
    bool die(bool animate) {
        dying=!dead && animate;
        dead=true;
        return dying;
    }
    void revive() { dead=false;dying=false;revive_ms=0; }
    void advance(double milliseconds) { revive_ms=std::min(600.0,revive_ms+milliseconds); }
    float opacity() const { return revive_ms>=600?1.f:float(int(31*revive_ms/600))/32.f; }
};
}
