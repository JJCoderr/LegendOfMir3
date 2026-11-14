#pragma once
#include <algorithm>
#include <array>
#include <cstdint>

namespace mir3 {
// 技能150使用两个原版粒子池：468A90火星、46A770光点。
// 与冰系地表粒子的尺寸、颜色和推进规则分开保存。
struct SpellParticle {
    enum class Kind { spark, glow };
    Kind kind=Kind::spark;
    std::array<float,2> position{},velocity{},acceleration{};
    std::array<std::uint8_t,3> initial_color{},color{};
    int age=0,life=0,frame_clock=0,interval=0;
    float initial_size=0,size=0;

    template<class Random>
    static SpellParticle create(Kind kind,std::array<float,2> position,Random& random) {
        SpellParticle particle;
        particle.kind=kind;
        particle.position=position;
        if (kind==Kind::spark) {
            particle.velocity[1]=float(random.between(-30,60));
            particle.velocity[0]=float(random.between(-50,50));
            particle.life=random.between(500,900);
        } else {
            particle.velocity[0]=float(random.between(-2,2));
            particle.life=random.between(200,400);
        }
        const float fraction=float(double(random.random()%1000)*double(0.001f));
        particle.initial_size=particle.size=float(random.between(kind==Kind::spark?3:8,kind==Kind::spark?5:12))+fraction;
        if (kind==Kind::spark) {
            particle.initial_color[0]=std::uint8_t(random.between(200,255));
            particle.initial_color[1]=particle.initial_color[0]>>1;
            particle.initial_color[2]=std::uint8_t(random.between(0,30));
        } else {
            particle.initial_color={std::uint8_t(random.between(220,255)),
                std::uint8_t(random.between(140,160)),std::uint8_t(random.between(60,80))};
        }
        particle.color=particle.initial_color;
        particle.interval=random.between(200,300);
        return particle;
    }

    template<class Random>
    bool advance(int milliseconds,Random& random) {
        age+=milliseconds;
        if (age>life) return false;
        frame_clock+=milliseconds;
        // 两个池均只有一张贴图，计时器回零但不切换到冰系的后续帧。
        if (frame_clock>interval) frame_clock=0;
        life=std::max(life,1);
        const double progress=double(age)/life;
        if (kind==Kind::glow) {
            size=float((progress*7.0+1.0)*initial_size);
            velocity[0]+=float(random.between(-1,1));
        }
        // 4691D4 / 469BC2：反编译遗漏了乘年龄比例和减法，实际RGB线性衰减。
        for (unsigned channel=0;channel<3;++channel)
            color[channel]=std::uint8_t(int(initial_color[channel]-progress*initial_color[channel]));
        const float step=float(double(std::max(1,milliseconds/17))*double(0.02f));
        for (unsigned axis=0;axis<2;++axis) {
            // 4653A0先写float阻力，465400累积加速度，再推进速度及位置。
            const float drag=-(velocity[axis]*-0.05f);
            const float force=axis==1?1000.f:kind==Kind::spark?100.f:0.f;
            acceleration[axis]=float((double(drag)+force)/1000.0+acceleration[axis]);
            velocity[axis]=float(double(acceleration[axis])*step+velocity[axis]);
            position[axis]=float(double(velocity[axis])*step+position[axis]);
        }
        return true;
    }

    int resource_frame() const { return kind==Kind::spark?520:530; }
};
}
