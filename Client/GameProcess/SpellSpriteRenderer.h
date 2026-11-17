#pragma once
#include "SpellParticlePool.h"
#include <map>
#include <vector>
#include <cmath>

namespace NativeSpellSprites {
struct Sprite {
    int width=0,height=0,offsetX=0,offsetY=0;
    std::vector<WORD> pixels;
    std::vector<BYTE> opaque;
};
inline const Sprite* Frame(int frame) {
    static CWHWilImageData image;
    static const bool loaded=image.NewLoad("./Data/ProgUse.wil",TRUE,TRUE)!=FALSE;
    static std::map<int,Sprite> frames;
    if(!loaded) return nullptr;
    const auto found=frames.find(frame);if(found!=frames.end()) return &found->second;
    if(!image.NewSetIndex(frame)) return nullptr;
    const auto& info=*image.m_lpstNewCurrWilImageInfo;
    Sprite sprite;sprite.width=info.shWidth;sprite.height=info.shHeight;
    sprite.offsetX=info.shPX;sprite.offsetY=info.shPY;
    sprite.pixels.resize(sprite.width*sprite.height);sprite.opaque.resize(sprite.pixels.size());
    const WORD* next=reinterpret_cast<const WORD*>(image.m_pbCurrImage);
    for(int y=0;y<sprite.height;++y) {
        const WORD* run=next+1;const WORD* end=run+*next;next=end;int x=0;
        while(run+1<end) {
            const WORD tag=*run++,count=*run++;
            if(tag==0xc0) {x+=count;continue;}
            if(tag<0xc1 || tag>0xc3 || run+count>end) break;
            for(int i=0;i<count;++i,++x) if(x<sprite.width) {
                const int index=y*sprite.width+x;sprite.pixels[index]=run[i];sprite.opaque[index]=1;
            }
            run+=count;
        }
    }
    return &frames.emplace(frame,std::move(sprite)).first->second;
}
enum class Blend { mark,spark,glow };
inline void Draw(DDSURFACEDESC2& surface,const Sprite& sprite,float left,float top,float width,float height,
    const std::array<std::uint8_t,3>& tint,Blend blend) {
    if(width<=0 || height<=0 || !sprite.width || !sprite.height) return;
    const int x0=(std::max)(0,int(std::ceil(left-.5f))),y0=(std::max)(0,int(std::ceil(top-.5f)));
    const int x1=(std::min)(int(surface.dwWidth),int(std::ceil(left+width-.5f)));
    const int y1=(std::min)(int(surface.dwHeight),int(std::ceil(top+height-.5f)));
    const auto mix=[&](int src,int dst,int maximum,int color) {
        const double source=double(src)/maximum*color/255.0,destination=double(dst)/maximum;
        constexpr double MarkOpacity=180.0/255.0,ParticleOpacity=1.0/255.0;
        const double result=blend==Blend::mark?source*MarkOpacity+destination*(1-MarkOpacity):
            source+destination*(blend==Blend::glow?1-source:1-ParticleOpacity);
        return int((std::min)(1.0,(std::max)(0.0,result))*maximum+.5);
    };
    for(int y=y0;y<y1;++y) {
        WORD* row=reinterpret_cast<WORD*>(static_cast<BYTE*>(surface.lpSurface)+y*surface.lPitch);
        const int sy=(std::min)(sprite.height-1,int((y+.5f-top)/height*sprite.height));
        for(int x=x0;x<x1;++x) {
            const int sx=(std::min)(sprite.width-1,int((x+.5f-left)/width*sprite.width));
            const int index=sy*sprite.width+sx;if(!sprite.opaque[index]) continue;
            const WORD source=sprite.pixels[index],destination=row[x];
            row[x]=WORD((mix(source>>11,destination>>11,31,tint[0])<<11)|
                (mix((source>>5)&63,(destination>>5)&63,63,tint[1])<<5)|
                mix(source&31,destination&31,31,tint[2]));
        }
    }
}
}
