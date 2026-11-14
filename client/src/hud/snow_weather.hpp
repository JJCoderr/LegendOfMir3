#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>

namespace mir3 {
// 465F60/466030/4666A0；状态与算法证据见 docs/evidence/weather.md。
struct SnowWeather {
    struct Particle {
        bool active=false;
        int age=0,life=0,frame_ms=0;
        float x=0,y=0,vx=0,vy=0,ax=0,ay=0,mass=1,size=0;
        std::array<std::uint8_t,3> rgb{255,255,255};
    };
    struct RainParticle : Particle {
        float previous_x=0,previous_y=0;
        int frame=0,frame_interval=0;

        bool splash() const { return age>life && age-life<510; }
        int texture_frame() const { return (splash()?510:500)+frame; }
    };
    std::array<RainParticle,400> rain_particles{};
    int rain_active=0;
    std::array<Particle,500> particles{};
    bool snow=false,fog=false,other=false;
    int active=0,fog_frame=0;
    float fog_x=0,fog_y=0;
    std::uint32_t random_state=1;
    int random() {random_state=random_state*214013u+2531011u;return int((random_state>>16)&32767);}
    int between(int low,int high) {const int a=random(),b=random();return low+std::abs(a-b)%(high-low+1);}
    void select(unsigned type,bool changing_map=false) {
        snow=type==1;fog=type==1 || type==2;
        // 42EE00保留天气3；即时46消息先清除三种天气。
        if(!changing_map) other=false;
        if(type==3) other=true;
        if(snow) {particles={};active=0;}
    }
    void spawn(Particle& p) {
        p={};p.active=true;p.y=float(between(-300,0));p.x=float(between(0,800));
        p.vy=float(between(70,100));p.vx=float(between(-30,30));p.life=between(2500,7000);
        p.mass=1000.f+float(random()%1000)*0.001f;
        const float fraction=float(random()%1000)*0.001f;
        p.size=float(between(2,6))+fraction;
        const auto color=std::uint8_t(between(120,150));p.rgb={color,color,std::uint8_t(color+100)};
    }
    void spawn_rain(RainParticle& p) {
        p={};
        p.active=true;
        p.y=float(between(-500,0));
        p.x=float(between(-100,900));
        p.vy=500.f;
        p.life=between(800,1400);
        p.mass=100.f;
        const float fraction=float(random()%1000)*0.001f;
        p.size=float(between(3,30))+fraction;
        const auto color=std::uint8_t(between(120,180));
        p.rgb={color,color,125};
        p.frame_interval=between(50,150);
    }
    void advance_rain(int ms) {
        const float step=float(std::max(1,ms/17))*0.02f;
        int created=0,processed=0;
        for(auto& p:rain_particles) {
            if(!p.active && created<11) {spawn_rain(p);++created;++rain_active;}
            if(!p.active) {if(processed>=rain_active) break;continue;}
            p.age+=ms;
            const int excess=p.age-p.life;
            if(excess<=0 && p.y<600.f) {
                p.ax+=(-p.vx*(-0.05f)+10.f)/p.mass;
                p.ay+=(-p.vy*(-0.05f)+100.f)/p.mass;
                p.vx+=p.ax*step;p.vy+=p.ay*step;
                p.previous_x=p.x;p.previous_y=p.y;
                p.x+=p.vx*step;p.y+=p.vy*step;
                p.frame_ms+=ms;
                if(p.frame_ms>p.frame_interval) {p.frame_ms=0;p.frame=0;}
                if(++processed>=rain_active) break;
            } else if(excess<=250) {
                const auto fade=std::uint8_t(250-excess/4);
                for(auto& channel:p.rgb) channel=std::min(channel,fade);
                p.frame_interval=50;
                p.frame_ms+=ms;
                if(p.frame_ms>50) {p.frame_ms=0;p.frame=(p.frame+1)%5;}
                ++processed;
            } else {
                p={};
                --rain_active;
                --processed;
            }
        }
    }
    void clear_rain() {
        rain_particles={};
        rain_active=0;
    }
    void advance(int ms,bool enabled) {
        if(ms<0) return;
        if(enabled) advance_snow_and_fog(ms);
        if(other) advance_rain(ms);
    }
    void advance_snow_and_fog(int ms) {
        if(fog) {
            ++fog_frame;
            if(fog_frame<800) {fog_x-=1.9f;fog_y-=0.2f;}
            else if(fog_frame<1600) {fog_x+=1.9f;fog_y+=0.2f;}
            else fog_frame=0;
        }
        if(!snow) return;
        const float step=float(std::max(1,ms/17))*0.02f;
        int created=0,processed=0;
        for(auto& p:particles) {
            if(!p.active && created<5) {spawn(p);++created;++active;}
            if(!p.active) {if(processed>=active) break;continue;}
            p.age+=ms;
            if(p.age<=p.life && p.y<600.f) {
                p.ax+=(-p.vx*(-0.05f)+10.f)/p.mass;
                p.ay+=(-p.vy*(-0.05f)+100.f)/p.mass;
                p.vx+=p.ax*step;p.vy+=p.ay*step;
                p.x+=p.vx*step;p.y+=p.vy*step;
                p.frame_ms+=ms;if(p.frame_ms>300) p.frame_ms=0;
                ++processed;
            } else if(p.age-p.life<=255) {
                const auto fade=std::uint8_t(p.life-p.age-1);
                for(auto& channel:p.rgb) channel=std::min(channel,fade);
                ++processed;continue;
            } else {p={};--active;--processed;continue;}
            // 4661A9只在正常更新和空槽后提前结束；衰减/释放直接跳4661B6。
            if(processed>=active) break;
        }
    }
};
}
