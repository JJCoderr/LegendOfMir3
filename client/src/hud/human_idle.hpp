#pragma once

namespace mir3 {
// Ported from mir3_1.45's HumanIdleClock, ordinary standing action (0x416E70).
class HumanIdleClock {
public:
    void reset() { elapsed_ms=0; current=0; }
    void advance(double milliseconds) {
        elapsed_ms+=milliseconds;
        if (elapsed_ms>=201) {
            elapsed_ms=0;
            current=(current+1)%4;
        }
    }
    int frame() const { return current; }
private:
    double elapsed_ms=0;
    int current=0;
};
}
