#pragma once
#include <algorithm>
namespace mir3 {
// 0x412C80 enables once; 0x414220 clears after more than 5000 ms.
struct CombatStance {
    double remaining_ms=0;
    void attack() { if (remaining_ms==0) remaining_ms=5001; }
    void clear() { remaining_ms=0; }
    bool advance(double milliseconds) {
        const bool active=remaining_ms>0;
        remaining_ms=std::max(0.0,remaining_ms-milliseconds);
        return active && remaining_ms==0;
    }
    int idle_action(bool mounted) const { return mounted?29:remaining_ms>0?7:0; }
};
}
