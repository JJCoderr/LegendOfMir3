#pragma once
#include "Mir3SkillIds.h"
#include <array>
#include <cmath>
#include <cstdint>

namespace mir3 {
// 44D680 / 44D9B0：73、74共用起手、移动、爆炸三个阶段。
// 坐标为相对锚点的像素偏移；摄像机投影由视图处理。
struct StagedSpell {
    enum class Stage { opening, moving, explosion, finished };
    enum class Change { none, load_explosion, finished };
    struct Frames {
        int first=0, end=0;
        std::uint32_t interval=0;
    };
    SkillId skill=SkillId::original_73;
    unsigned direction=0;
    Stage stage=Stage::opening;
    Frames frames;
    int frame=0;
    std::uint32_t frame_clock=0, movement_clock=0;
    float phase=0;
    std::array<int,2> offset{};

    StagedSpell(SkillId id,unsigned facing,Frames animation)
        :skill(id),direction(facing&7),frames(animation),frame(animation.first) {
        // 74创建时暂置目标右下100像素，第一次推进才改为目标下方10像素。
        if (skill==SkillId::original_74) offset={100,100};
    }

    void load_explosion(Frames animation) {
        frames=animation;
        frame=animation.first;
        // 原入口切换资源时不重置帧累计、光源状态或最终位移。
    }

    Change advance(std::uint32_t milliseconds) {
        if (stage==Stage::finished) return Change::none;
        frame_clock+=milliseconds;
        const auto next_frame=[&] {
            if (frame_clock<=frames.interval) return false;
            frame_clock=0;
            ++frame;
            return true;
        };
        switch (stage) {
        case Stage::opening:
            if (skill==SkillId::original_74) offset={0,10};
            // 这里是固定资源帧1050，不是MAGIC记录的结束帧。
            if (next_frame() && frame>=1050) stage=Stage::moving;
            break;
        case Stage::moving:
            next_frame();
            movement_clock+=milliseconds;
            // 先处理动画回卷；回卷这次调用不做位移或一秒阶段切换。
            if (frame>=frames.end) {
                frame=frames.first;
                break;
            }
            if (movement_clock>=1000) {
                stage=Stage::explosion;
                return Change::load_explosion;
            }
            if (skill==SkillId::original_73) {
                // x87使用未落float内存的加法结果计算当前帧；存储相位仍为float。
                const double next=double(phase)+30.0;
                phase=float(next);
                const int x=int(next*0.8);
                constexpr double radians=double(0.01744999922811985f);
                const int y=int(std::sin(next*radians)*15.0);
                const double radius=std::sqrt(double(x)*x+double(y)*y);
                constexpr std::array<int,8> degrees{90,30,0,330,270,210,180,150};
                const double angle=degrees[direction]*radians-std::atan(double(y)/x);
                offset={int(std::cos(angle)*radius),-int(std::sin(angle)*radius)};
            } else {
                phase=15.0f;
                offset={0,10};
            }
            break;
        case Stage::explosion:
            if (next_frame() && frame>=frames.end) {
                stage=Stage::finished;
                return Change::finished;
            }
            break;
        case Stage::finished: break;
        }
        return Change::none;
    }
};
}
