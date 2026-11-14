#pragma once
#include <array>
#include <cstdint>
namespace mir3 {
struct SystemOptions {
    bool show_mon=true,force_attack=true,shadow_blend=false,user_photos=true;
    std::array<bool,7> confirmed{},displayed{};
    int attack_mode=0;
    static constexpr std::array<int,7> command_ids{2123,2124,2125,2126,2127,2128,2129};
    static constexpr std::array<int,7> toggle_x{186,186,186,186,186,186,430};
    static constexpr std::array<int,7> toggle_y{175,205,235,265,295,325,55};
    // 0x464A00: unchanged confirmations deliberately retain predicted frames.
    unsigned receive(std::uint32_t flags,std::uint16_t mode) {
        constexpr std::array<std::uint32_t,7> masks{0x40000000u,0x80000000u,0x08000000u,
            0x04000000u,0x02000000u,0x01000000u,0x00800000u};
        unsigned changed=0;
        for (int i=0;i<7;++i) {
            const bool value=(flags&masks[i])!=0;
            if (confirmed[i]!=value) {
                confirmed[i]=displayed[i]=value;changed|=1u<<i;
            }
        }
        attack_mode=mode;return changed;
    }
    int request(int index) {
        if (index<0 || index>=7) return -1;
        displayed[index]=!confirmed[index];return command_ids[index];
    }
};
}
