#pragma once
#include <array>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace mir3 {
// 0x448C30: light is sampled on the scene's 18-by-19 vertex grid.
struct SceneLight {
    using Rgb=std::array<int,3>;
    int x,y;
    int inner_radius;
    Rgb inner;
    int outer_radius_cells;
    Rgb outer;

    // 0x40EB70 selects radius and color from the actor's light byte.
    static SceneLight actor(int x,int y,unsigned attribute,bool local) {
        const int radius=attribute==0?4:attribute==2?(local?8:4):attribute==8?(local?10:8):(local?14:10);
        const Rgb color=attribute==0 && !local?Rgb{10,5,5}:
            attribute==0 || attribute==2?Rgb{100,100,100}:Rgb{255,255,255};
        return {x,y,0,{},radius,color};
    }

    bool diagonal_top_left_to_bottom_right(int column,int row) const {
        const int source_column=(x+8)/48;
        const int source_row=(y+29)/32;
        return (column>source_column && row<source_row) ||
            (column<source_column && row>source_row);
    }

    Rgb sample(int column,int row) const {
        const int source_column=(x+8)/48;
        const int source_row=(y+29)/32;
        if (column<0 || column>=18 || row<0 || row>=19 ||
            std::abs(column-source_column)>outer_radius_cells ||
            std::abs(row-source_row)>outer_radius_cells || outer_radius_cells<=0) return {};
        const double distance=std::hypot(double(x+8-48*column),double(y+29-32*row));
        if (distance<inner_radius) return falloff(inner,inner_radius,distance);
        const int outer_radius=32*outer_radius_cells;
        if (distance<=outer_radius) return falloff(outer,outer_radius,distance);
        return {};
    }
private:
    static Rgb falloff(Rgb color,int radius,double distance) {
        for (auto& channel:color) channel-=static_cast<int>(channel*distance/radius);
        return color;
    }
};

// 0x42EE00 entry, 0x429213 update, 0x425E10 timer.
struct SceneAmbient {
    std::uint8_t mode=0;
    int half_level=0;
    int display_level=255;
    std::array<int,4> base_values{};
    int selected_base=0;
    double elapsed_ms=0;
    bool fading=false;
    int level() const { return display_level; }
    void select_base(std::uint8_t index,int roll) {
        const int mid=(roll+37)/2;
        base_values={roll,mid,mid+46,mid+75};
        if(index<base_values.size()) selected_base=base_values[index];
    }
    void enter(std::uint16_t value) {
        constexpr std::array levels{255,15,85};
        if(mode==value) { elapsed_ms=0;fading=false; }
        mode=static_cast<std::uint8_t>(value);
        display_level=mode<levels.size()?levels[mode]:10;
    }
    void update(std::uint16_t value) {
        if (value!=mode) { fading=true;elapsed_ms=0; }
        mode=static_cast<std::uint8_t>(value);
    }
    void advance(double ms) {
        if (!fading || !std::isfinite(ms) || ms<=0) return;
        constexpr std::array levels{510,30,170};
        const int target=mode<levels.size()?levels[mode]:0;
        elapsed_ms+=ms;
        const double ticks=std::floor(elapsed_ms/50);
        if(ticks<1) return;
        const int distance=std::abs(target-half_level);
        const int steps=static_cast<int>((std::min)(ticks,double(distance)));
        elapsed_ms-=ticks*50;
        half_level+=half_level<target?steps:-steps;
        display_level=half_level/2;
        if(ticks>distance) { fading=false;elapsed_ms=0; }
    }
};
struct SceneLightGrid {
    std::array<SceneLight::Rgb,18*19> samples;
    std::array<bool,17*18> diagonals{};
    explicit SceneLightGrid(int ambient) { samples.fill({ambient,ambient,ambient}); }
    void add(const SceneLight& light) {
        const int cx=(light.x+8)/48,cy=(light.y+29)/32,r=light.outer_radius_cells;
        for (int y=(std::max)(0,cy-r);y<=(std::min)(18,cy+r);++y)
            for (int x=(std::max)(0,cx-r);x<=(std::min)(17,cx+r);++x) {
                const auto rgb=light.sample(x,y);
                for (int c=0;c<3;++c) samples[y*18+x][c]=(std::min)(255,samples[y*18+x][c]+rgb[c]);
                if (x<17 && y<18) diagonals[y*17+x]=light.diagonal_top_left_to_bottom_right(x,y);
            }
    }
};
}
