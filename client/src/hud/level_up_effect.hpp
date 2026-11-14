#pragma once
#include "resources/effect_catalog.hpp"
#include "hud/scene_lighting.hpp"
#include <cstdint>

namespace mir3 {
// 只实现4495D0明确归入静止目标分支的升级效果；不是通用法术状态机。
struct LevelUpEffect {
    static constexpr std::uint16_t resource_id=422;
    static constexpr int sound_id=10*resource_id+10002;
    enum class Phase { playing,shrinking,finished };
    EffectResource resource;
    int cell_x=0,cell_y=0;
    std::uint32_t frame=0,frame_elapsed=0,light_elapsed=0,total_elapsed=0;
    std::uint8_t inner_radius=0,outer_radius=0,light_phase=0;
    Phase phase=Phase::playing;

    LevelUpEffect(const EffectResource& value,int x,int y):resource(value),cell_x(x),cell_y(y),
        frame(value.first),inner_radius(value.inner_radius),outer_radius(value.outer_radius) {
        if (value.end<=value.first) phase=Phase::finished;
    }
    void advance(std::uint32_t milliseconds) {
        if (phase==Phase::finished) return;
        // 44B7E0先更新动画；严格大于，单次最多一帧，不保留余量。
        total_elapsed+=milliseconds;frame_elapsed+=milliseconds;
        if (total_elapsed>60000) { phase=Phase::finished;return; }
        if (frame_elapsed>resource.interval) {
            frame_elapsed=0;
            if (++frame>=resource.end) { frame=resource.end-1;phase=Phase::shrinking; }
        }
        // 44A110随后更新光尾；动画结束不会直接丢弃最后一帧。
        light_elapsed+=milliseconds;
        if (light_elapsed>200) {
            if (phase==Phase::playing) {
                light_elapsed=0;
                light_phase=std::uint8_t(light_phase+1);
                if (light_phase>resource.light_steps) light_phase=0;
            } else {
                light_phase=0;light_elapsed=150;
                if (inner_radius<=1 || outer_radius<=inner_radius) phase=Phase::finished;
                else { --inner_radius;--outer_radius; }
            }
        }
    }
    SceneLight light(int camera_x,int camera_y) const {
        return {cell_x*48-camera_x+24,cell_y*32-camera_y+16,inner_radius+light_phase,
            {resource.inner_color[0],resource.inner_color[1],resource.inner_color[2]},outer_radius+light_phase,
            {resource.outer_color[0],resource.outer_color[1],resource.outer_color[2]}};
    }
};
}
