#pragma once
#include "SceneWeather.h"
#include "SpellSpriteRenderer.h"

namespace NativeWeather {
constexpr int SnowFrame=500,FogFrame=550;
// CPU equivalent of the reference weather quads and their two blend shaders.
inline void Quad(DDSURFACEDESC2& surface,const NativeSpellSprites::Sprite& sprite,
    float centerX,float centerY,float halfWidth,float halfHeight,float angle,
    const std::array<std::uint8_t,3>& tint,bool weatherUV,const std::vector<WORD>* background) {
    if(halfWidth<=0 || halfHeight<=0) return;
    const float cosine=std::cos(angle),sine=std::sin(angle);
    const float extentX=std::abs(cosine)*halfWidth+std::abs(sine)*halfHeight;
    const float extentY=std::abs(sine)*halfWidth+std::abs(cosine)*halfHeight;
    const int left=(std::max)(0,int(std::ceil(centerX-extentX-.5f)));
    const int top=(std::max)(0,int(std::ceil(centerY-extentY-.5f)));
    const int right=(std::min)(int(surface.dwWidth),int(std::ceil(centerX+extentX-.5f)));
    const int bottom=(std::min)(int(surface.dwHeight),int(std::ceil(centerY+extentY-.5f)));
    const auto mix=[&](int source,int destination,int maximum,int color) {
        const double src=double(source)/maximum*color/255.0,dst=double(destination)/maximum;
        const double value=src+dst*(background?1-src:254.0/255.0);
        return int((std::min)(1.0,(std::max)(0.0,value))*maximum+.5);
    };
    for(int y=top;y<bottom;++y) {
        WORD* row=reinterpret_cast<WORD*>(static_cast<BYTE*>(surface.lpSurface)+y*surface.lPitch);
        for(int x=left;x<right;++x) {
            const float dx=x+.5f-centerX,dy=y+.5f-centerY;
            const float tx=(cosine*dx+sine*dy+halfWidth)/(2*halfWidth);
            const float ty=(-sine*dx+cosine*dy+halfHeight)/(2*halfHeight);
            if(tx<0 || tx>=1 || ty<0 || ty>=1) continue;
            const float u=weatherUV?1-ty:tx,v=weatherUV?1-tx:ty;
            const int sx=(std::min)(sprite.width-1,int(u*sprite.width));
            const int sy=(std::min)(sprite.height-1,int(v*sprite.height));
            const int index=sy*sprite.width+sx;if(!sprite.opaque[index]) continue;
            const WORD src=sprite.pixels[index],dst=background?(*background)[y*surface.dwWidth+x]:row[x];
            row[x]=WORD((mix(src>>11,dst>>11,31,tint[0])<<11)|
                (mix((src>>5)&63,(dst>>5)&63,63,tint[1])<<5)|mix(src&31,dst&31,31,tint[2]));
        }
    }
}
inline void Snapshot(const DDSURFACEDESC2& surface,std::vector<WORD>& pixels) {
    pixels.resize(surface.dwWidth*surface.dwHeight);
    for(unsigned y=0;y<surface.dwHeight;++y)
        memcpy(pixels.data()+y*surface.dwWidth,static_cast<const BYTE*>(surface.lpSurface)+y*surface.lPitch,surface.dwWidth*sizeof(WORD));
}
inline void Draw(const mir3::SnowWeather& weather,bool enabled) {
    if(!(enabled && (weather.snow || weather.fog)) && !weather.other) return;
    DDSURFACEDESC2 surface={};surface.dwSize=sizeof(surface);auto* buffer=g_xMainWnd.GetBackBuffer();
    if(FAILED(buffer->Lock(nullptr,&surface,DDLOCK_WAIT,nullptr))) return;
    std::vector<WORD> background;
    if(enabled && weather.fog) if(const auto* fog=NativeSpellSprites::Frame(FogFrame)) {
        Snapshot(surface,background);
        Quad(surface,*fog,400+weather.fog_x,300-weather.fog_y,2000,600,0,{100,200,200},false,&background);
    }
    if(enabled && weather.snow) if(const auto* snow=NativeSpellSprites::Frame(SnowFrame))
        for(const auto& p:weather.particles) if(p.active)
            Quad(surface,*snow,p.x,p.y,p.size*.5f,p.size*.5f,0,p.rgb,true,nullptr);
    if(weather.other) {
        // Rain samples the snapshot before the entire layer, not each preceding rain quad.
        Snapshot(surface,background);
        for(const auto& p:weather.rain_particles) if(p.active) if(const auto* rain=NativeSpellSprites::Frame(p.texture_frame())) {
            const float dx=p.x-p.previous_x;
            const float angle=dx==0?0:std::atan2((p.y-p.previous_y)/dx,1.f)-1.57079632679f;
            Quad(surface,*rain,p.x,p.y,p.splash()?16.f:.75f,p.splash()?16.f:p.size*.5f,angle,p.rgb,true,&background);
        }
    }
    buffer->Unlock(nullptr);
}
}
