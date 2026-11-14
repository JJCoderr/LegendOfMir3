#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace mir3 {
// 417D50、419B40：仅负责时间条件，待执行的动作由视图持有。
struct LocalActionClock {
    std::uint32_t move_admitted=0, moved=0, attacked=0;
    static constexpr std::uint32_t remaining(std::uint32_t now,std::uint32_t since,std::uint32_t duration) {
        const auto elapsed=std::uint32_t(now-since);
        return elapsed>=duration?0:duration-elapsed;
    }
    bool admit_move(std::uint32_t now) {
        if (remaining(now,move_admitted,600)) return false;
        move_admitted=now;
        return true;
    }
    std::uint32_t shared_wait(std::uint32_t now,std::uint32_t attack_interval) const {
        // 保留原版double比较；整数时间达到向上取整值才可执行。
        const auto attack_wait=std::uint32_t(std::ceil(double(attack_interval)*0.8));
        return std::max({remaining(now,attacked,attack_wait),remaining(now,moved,480),
            remaining(now,0,800)}); // 62908仅确认初始化为0，不推断其他写入源。
    }
};
}
