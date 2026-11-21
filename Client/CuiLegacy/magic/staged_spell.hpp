#pragma once
#include "skill_ids.hpp"
#include <array>
#include <cmath>
#include <cstdint>

namespace mir3 {


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

        if (skill==SkillId::original_74) offset={100,100};
    }

    void load_explosion(Frames animation) {
        frames=animation;
        frame=animation.first;

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

            if (next_frame() && frame>=1050) stage=Stage::moving;
            break;
        case Stage::moving:
            next_frame();
            movement_clock+=milliseconds;

            if (frame>=frames.end) {
                frame=frames.first;
                break;
            }
            if (movement_clock>=1000) {
                stage=Stage::explosion;
                return Change::load_explosion;
            }
            if (skill==SkillId::original_73) {

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
