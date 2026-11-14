#pragma once
#include <algorithm>
#include <array>
#include <string>
#include <vector>
namespace mir3 {
struct GroundItem { int id,x,y,looks;std::string name; };
class GroundItems {
public:
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
