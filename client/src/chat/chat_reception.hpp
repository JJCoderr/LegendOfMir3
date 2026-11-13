#pragma once
#include <array>
namespace mir3 {
struct ChatReceptionButtons {
    // 0x43C130 cases20..22: visual prediction from this+72A8/72AC/72B0.
    std::array<bool, 3> changed{};
    std::array<int, 3> frames{-1, -1, -1};
    std::array<int, 3> hints{2637, 2639, 2643};
    int press(int button, bool rejected) {
        if (button < 20 || button > 22) return -1;
        const int index = button - 20;
        changed[index] = true;
        frames[index] = rejected ? -1 : 1338 + 2 * index;
        hints[index] = (index == 0 ? 2637 : (index == 1 ? 2639 : 2643)) + (rejected ? 0 : 1);
        return index == 0 ? 1070 : (index == 1 ? 1071 : 1073);
    }
};
}
