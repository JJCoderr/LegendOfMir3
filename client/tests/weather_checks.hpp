// 测试内容：原版天气选择、雪花生成提前结束、寿命与淡出边界、暂停恢复、雾帧边界和随机范围。
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
}
