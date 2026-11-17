#pragma once
#include <array>
#include <cmath>

namespace mir3 {
// 44C180 / 44C360：技能150的两个粒子发射器共享基础弹道，摆动只修改显示坐标。
struct SpellOrbit {
    float phase=0;
    bool inverted=false;
    static constexpr std::array<std::array<float,2>,16> weights{{
        {{1,0}},{{.75f,.25f}},{{.5f,.5f}},{{.25f,.75f}},
        {{0,1}},{{.25f,.75f}},{{1.25f,0}},{{1.25f,.25f}},
        {{1,0}},{{.75f,.25f}},{{.5f,.5f}},{{.25f,.75f}},
        {{0,1}},{{.25f,.75f}},{{0,1.25f}},{{1.25f,.25f}}
    }};
    static constexpr std::array<std::array<int,2>,16> emit_offsets{{
        {{34,37}},{{47,38}},{{62,48}},{{73,41}},
        {{76,34}},{{72,43}},{{58,53}},{{50,48}},
        {{34,49}},{{45,49}},{{55,49}},{{56,43}},
        {{48,35}},{{46,40}},{{46,47}},{{47,38}}
    }};
    std::array<int,2> advance(float base_x,float base_y,unsigned direction) {
        // 原版每次成功推进+0.4f，不乘delta；基础float位置先向零取整再加偏移。
        const double next=double(phase)+double(0.4f);
        phase=float(next);
        const double amplitude=std::sin(next)*20.0*(inverted?-1.0:1.0);
        const auto& weight=weights[direction&15];
        return {int(int(base_x)+amplitude*weight[0]),int(int(base_y)+amplitude*weight[1])};
    }
    static std::array<int,2> emitter(std::array<int,2> pixel,unsigned direction,int resource_x,int resource_y) {
        const auto& offset=emit_offsets[direction&15];
        return {pixel[0]+offset[0]+resource_x,pixel[1]+offset[1]+resource_y};
    }
};
}
