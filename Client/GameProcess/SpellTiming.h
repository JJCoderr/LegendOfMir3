#pragma once
#include <cstdint>

namespace mir3 {
// 418D80 / 4197C0 / 417D50 的 DWORD 时序；不在此处修改 HP、MP 或服务器状态。
struct SpellTiming {
    std::uint32_t cast_at=0;          // 628D8
    std::uint32_t restricted_at=0;    // 628DC
    std::uint32_t movement_delay=0;   // 628FC
    std::uint32_t casting_delay=0;    // 62900，记录的延迟再加 200
    std::uint32_t rush_at=0, special_at=0; // 628E8 / 628EC
    std::uint32_t fire_sword_at=0, skill35_at=0, skill34_at=0;

    static constexpr bool elapsed_after(std::uint32_t now,std::uint32_t since,std::uint32_t duration) {
        return std::uint32_t(now-since)>duration;
    }
    bool key_ready(std::uint32_t now,int slot=0,bool repeat=false) const {
        if (repeat && slot!=10 && slot!=12) return false;
        // 4199A0: 415510 initializes the three timing fields to 9, 8 and 28.
        // Their default yields 1000, which also satisfies the original mask check.
        constexpr std::uint32_t repeat_delay=10u*(9u*8u+28u);
        static_assert((repeat_delay&0xf20fu)==0x208u);
        return elapsed_after(now,cast_at,casting_delay+(repeat?repeat_delay:1000u));
    }
    bool movement_ready(std::uint32_t now) const {
        // 417DA1 的 cmp / sbb / neg 为严格大于，不是大于等于。
        return elapsed_after(now,cast_at,movement_delay);
    }
    void begin_animated(std::uint32_t now,std::uint32_t record_delay) {
        cast_at=now;
        casting_delay=record_delay+200u;
    }
    // 401710(1,1100)+300：两次 rand 的差取绝对值后取模，不是均匀随机分布。
    static constexpr std::uint32_t human_target_delay(std::uint32_t first,std::uint32_t second) {
        const auto difference=first>=second?first-second:second-first;
        return 301u+difference%1100u;
    }
    void finish_target(std::uint32_t target,int target_type,std::uint32_t first_random,std::uint32_t second_random) {
        movement_delay=target && (target_type==0 || target_type==1)?
            human_target_delay(first_random,second_random):0;
    }
};
}
