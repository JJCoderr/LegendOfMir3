#pragma once
#include <algorithm>
#include <array>
#include <cstdint>

namespace mir3 {
// 150、336及冰系共用原版三类池；各发射器保留独立初始参数。
struct SpellParticle {
    enum class Kind { spark, glow, lightning_glow, lightning_fragment, ice_glow, ice_fragment };
    Kind kind=Kind::spark;
    std::array<float,2> position{},velocity{},acceleration{};
    std::array<std::uint8_t,3> initial_color{},color{};
    int age=0,life=0,frame_clock=0,interval=0,frame=0;
    float initial_size=0,size=0,mass=1000;

    template<class Random>
    static SpellParticle create(Kind kind,std::array<float,2> position,Random& random) {
        SpellParticle particle;
        particle.kind=kind;
        particle.position=position;
        // 46A5D0 / 468000：冰波和类型9退场共用光点、碎片池。
        if (kind==Kind::ice_glow || kind==Kind::ice_fragment) {
            const bool fragment=kind==Kind::ice_fragment;
            particle.velocity[1]=float(fragment?random.between(-180,-50):random.between(-120,-80));
            particle.velocity[0]=float(fragment?random.between(-75,75):random.between(-12,12));
            particle.life=fragment?random.between(800,1200):random.between(300,600);
            particle.mass=fragment?1.f:1000.f;
            const float fraction=fragment?float(random.random()%1000)*0.001f:0.f;
            particle.initial_size=particle.size=float(fragment?random.between(2,10):random.between(10,30))+fraction;
            particle.initial_color={std::uint8_t(fragment?random.between(50,100):random.between(100,150)),
                std::uint8_t(fragment?random.between(100,200):random.between(150,200)),
                std::uint8_t(fragment?random.between(200,255):random.between(255,255))};
            particle.color=particle.initial_color;particle.interval=random.between(200,300);
            return particle;
        }
        // 44CDA0的336首帧：46AAB0一枚扩张光点，467D40二十一枚碎片。
        if (kind==Kind::lightning_glow || kind==Kind::lightning_fragment) {
            if (kind==Kind::lightning_glow) {
                particle.velocity={-20,-20};particle.life=1000;particle.mass=10000;
                particle.initial_size=particle.size=18;
                particle.initial_color={std::uint8_t(random.between(60,70)),std::uint8_t(random.between(60,70)),std::uint8_t(random.between(70,80))};
            } else {
                particle.velocity[1]=float(random.between(-300,150));
                particle.velocity[0]=float(random.between(-300,300));
                particle.life=random.between(300,1600);particle.mass=10;
                particle.initial_size=particle.size=2.f+float(random.random()%1000)*0.001f;
                particle.initial_color={std::uint8_t(random.between(30,60)),std::uint8_t(random.between(130,160)),std::uint8_t(random.between(130,160))};
            }
            particle.color=particle.initial_color;particle.interval=random.between(200,300);
            return particle;
        }
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
        // 光点与火星各一张贴图，碎片在520至523之间循环。
        if (frame_clock>interval) {frame_clock=0;frame=fragment()?(frame+1)%4:0;}
        life=std::max(life,1);
        const double progress=double(age)/life;
        if (glow()) {
            size=float((progress*7.0+1.0)*initial_size);
            velocity[0]+=float(random.between(-1,1));
        }
        if (fragment()) {mass+=3;size=float(initial_size*(1-progress));}
        // 4691D4 / 469BC2：反编译遗漏了乘年龄比例和减法，实际RGB线性衰减。
        if (!fragment())
            for (unsigned channel=0;channel<3;++channel)
                color[channel]=std::uint8_t(int(initial_color[channel]-progress*initial_color[channel]));
        const float step=float(double(std::max(1,milliseconds/17))*double(0.02f));
        for (unsigned axis=0;axis<2;++axis) {
            // 4653A0先写float阻力，465400累积加速度，再推进速度及位置。
            const float drag=-(velocity[axis]*-0.05f);
            const float force=axis==1?(fragment()?200.f:1000.f):kind==Kind::spark?100.f:0.f;
            acceleration[axis]=float((double(drag)+force)/mass+acceleration[axis]);
            velocity[axis]=float(double(acceleration[axis])*step+velocity[axis]);
            position[axis]=float(double(velocity[axis])*step+position[axis]);
        }
        return true;
    }

    bool fragment() const {return kind==Kind::lightning_fragment || kind==Kind::ice_fragment;}
    bool ice() const {return kind==Kind::ice_glow || kind==Kind::ice_fragment;}
    bool glow() const {return kind==Kind::glow || kind==Kind::lightning_glow || kind==Kind::ice_glow;}
    int resource_frame() const { return glow()?530:520+frame; }
};
}
