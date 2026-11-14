#pragma once
#include <array>
namespace mir3 {
struct SystemOptions {
    bool show_mon=true,force_attack=true,shadow_blend=false,user_photos=true;
    std::array<bool,7> confirmed{},displayed{};
    int attack_mode=0;
    static constexpr std::array<int,7> command_ids{2123,2124,2125,2126,2127,2128,2129};
    static constexpr std::array<int,7> toggle_x{186,186,186,186,186,186,430};
    static constexpr std::array<int,7> toggle_y{175,205,235,265,295,325,55};
    int request(int index) {
        if (index<0 || index>=7) return -1;
        displayed[index]=!confirmed[index];return command_ids[index];
    }
};
}
