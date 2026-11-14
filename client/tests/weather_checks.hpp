// 测试内容：原版天气选择、雪花生成提前结束、寿命与淡出边界、暂停恢复、雾帧边界、雨滴运动与水花寿命和帧切换。
#pragma once
#include "hud/snow_weather.hpp"
#include "chat/chat_submission.hpp"
#include <stdexcept>
#include <cmath>
inline void weather_checks() {
    auto check=[](bool value,const char* message){if(!value) throw std::runtime_error(message);};
    mir3::SnowWeather weather;
    check(mir3::ChatSubmission{}.snow,"snow switch defaults on");
    weather.select(1);weather.advance(17,true);
    check(weather.snow && weather.fog && weather.active==1,"original early exit after first active particle");
    const auto first=weather.particles[0];
    check(first.life>=2500 && first.life<=7000 && first.size>=2 && first.size<7 && first.rgb[2]==first.rgb[0]+100,"spawn ranges");
    weather.advance(5000,false);
    check(weather.particles[0].age==first.age && weather.fog_frame==1,"disabled freezes both effects");
    weather.advance(17,true);
    check(weather.active==1 && weather.particles[0].y>first.y,"active prefix early return retained");
    auto& p=weather.particles[0];p.age=p.life;p.y=10;
    weather.advance(1,true);
    check(weather.active==2 && p.y==10,"fade branch continues to next empty slot");
    p.age=p.life+254;p.rgb={150,150,250};
    weather.advance(1,true);
    check(p.active && p.rgb[0]==0 && p.rgb[2]==0,"255ms fade ends black but still active");
    weather.advance(1,true);check(!p.active,"256ms releases slot");
    weather.select(1);check(weather.active==0,"snow reentry resets particles");
    weather.select(2);weather.fog_frame=798;weather.fog_x=weather.fog_y=0;
    weather.advance(17,true);check(weather.fog_frame==799 && std::abs(weather.fog_x+1.9f)<0.001,"fog negative through 799");
    weather.advance(17,true);check(weather.fog_frame==800 && std::abs(weather.fog_x)<0.001,"fog reverses at 800");
    weather.fog_frame=1599;weather.advance(17,true);check(weather.fog_frame==0 && weather.fog_x==0,"1600 resets counter without motion");
    weather.select(3);weather.select(0,true);check(weather.other && !weather.snow && !weather.fog,"map preserves original other weather flag");
    weather.select(0);check(!weather.other,"immediate weather resets other flag");
    weather.select(257);check(!weather.snow && !weather.fog,"immediate weather is full WORD");

    weather.select(3);
    weather.advance(17,false);
    auto& rain=weather.rain_particles[0];
    check(weather.rain_active==1 && rain.age==17 && rain.vy>500 && rain.y>rain.previous_y,
          "rain starts independently of snow and saves previous position");
    check(rain.life>=800 && rain.life<=1400 && rain.size>=3 && rain.size<31 &&
          rain.rgb[0]>=120 && rain.rgb[0]<=180 && rain.rgb[1]==rain.rgb[0] &&
          rain.rgb[2]==125 && rain.texture_frame()==500,"rain spawn ranges and texture");
    weather.select(0);
    weather.advance(17,true);
    check(rain.age==17,"immediate clear freezes rain");
    weather.select(3,true);
    weather.select(2,true);
    weather.advance(17,false);
    check(rain.age==34,"map preserves rain and repeated selection does not reset particles");

    rain.age=rain.life-1;rain.y=40;rain.frame_ms=49;
    weather.advance(1,false);
    check(!rain.splash() && rain.texture_frame()==500,"lifetime itself still draws streak");
    rain.frame_ms=49;
    const float resting_y=rain.y;
    weather.advance(1,false);
    check(weather.rain_active==2 && rain.texture_frame()==510 && rain.y==resting_y,
          "splash stays in place, 50ms retains frame and scan reaches the next slot");
    weather.advance(1,false);
    check(rain.texture_frame()==511 && rain.frame_ms==0,"51ms advances splash once");
    rain.frame=4;rain.frame_ms=50;
    weather.advance(1,false);
    check(rain.texture_frame()==510,"splash frame wraps");
    rain.age=rain.life+249;rain.rgb={255,255,255};
    weather.advance(1,false);
    check(rain.active && rain.rgb[0]==188,"250ms still fades using integer division");
    weather.advance(1,false);
    check(!rain.active,"251ms frees rain slot");
    weather.clear_rain();
    check(weather.rain_active==0 && !weather.rain_particles[1].active,"scene cleanup clears rain slots");
}
