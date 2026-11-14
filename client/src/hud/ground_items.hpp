#pragma once
#include <cstdint>
#include <algorithm>
#include <array>
#include <string>
#include <vector>
namespace mir3 {
struct GroundItem {
    int id,x,y,looks;
    std::string name;
    int stack_mode=0;
    int sparkle_delay_ms=3000;
    std::uint32_t sparkle_started_tick=0;
    double sparkle_elapsed_ms=0;
    int sparkle_frame=-1;
};
class GroundItems {
public:
    static constexpr int cell_capacity=100;
    static constexpr int sparkle_delay(int first_roll,int second_roll) {
        const int difference=first_roll-second_roll;
        return 3000+(difference<0?-difference:difference)%4001;
    }
    std::vector<GroundItem> items;
    bool show(GroundItem item) {
        if (item.id<=0 || item.x<0 || item.y<0 || item.looks<0 || item.looks>65535 || item.name.size()>39) return false;
        if (std::ranges::any_of(items,[&](const auto& existing) { return existing.id==item.id; })) return true;
        items.push_back(std::move(item));return true;
    }
    void hide(int id) { std::erase_if(items,[id](const auto& item) { return item.id==id; }); }
    static std::array<int,2> image_offset(int width,int height) { return {24-width/2,16-height/2}; }
};
}
