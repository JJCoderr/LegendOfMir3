#pragma once
#include "EffectCatalog.h"
#include <map>
#include <memory>

namespace NativeEffects {
enum class Library : WORD { Magic=81,Mon4=91,MonMagic=129,MagicEx=137,MonMagicEx=138,MonMagicEx2=140,MonMagicEx3=143,MonMagicEx4=148,MagicEx2=149 };
inline CWHWilImageData* Image(WORD library) {
    const char* path=nullptr;
    switch(static_cast<Library>(library)) {
    case Library::Magic:path="./Data/Magic.wil";break;
    case Library::Mon4:path="./Data/Mon-4.wil";break;
    case Library::MonMagic:path="./Data/MonMagic.wil";break;
    case Library::MagicEx:path="./Data/MagicEx.wil";break;
    case Library::MonMagicEx:path="./Data/MonMagicEx.wil";break;
    case Library::MonMagicEx2:path="./Data/MonMagicEx2.wil";break;
    case Library::MonMagicEx3:path="./Data/MonMagicEx3.wil";break;
    case Library::MonMagicEx4:path="./Data/MonMagicEx4.wil";break;
    case Library::MagicEx2:path="./Data/MagicEx2.wil";break;
    default:return nullptr;
    }
    static std::map<WORD,std::unique_ptr<CWHWilImageData>> images;
    auto found=images.find(library);
    if(found!=images.end()) return found->second.get();
    auto image=std::make_unique<CWHWilImageData>();
    if(!image->NewLoad(const_cast<char*>(path),TRUE,TRUE)) return nullptr;
    auto* result=image.get();images.emplace(library,std::move(image));return result;
}
// RGB565 backbuffer equivalent of the reference level_effect.gdshader.
inline void Draw(const mir3::EffectResource& resource,int frame,int x,int y) {
    auto* image=Image(resource.library);if(!image || !image->NewSetIndex(frame)) return;
    const auto& info=*image->m_lpstNewCurrWilImageInfo;x+=info.shPX;y+=info.shPY;
    DDSURFACEDESC2 surface={};surface.dwSize=sizeof(surface);auto* buffer=g_xMainWnd.GetBackBuffer();
    if(FAILED(buffer->Lock(nullptr,&surface,DDLOCK_WAIT,nullptr))) return;
    const WORD* next=reinterpret_cast<const WORD*>(image->m_pbCurrImage);
    const auto channel=[&](int pixel,int destination,int maximum,int tint) {
        const double texture=double(pixel)/maximum,dst=double(destination)/maximum;
        double src=texture*tint/255.0,result=src+dst*(1-src);
        const double opacity=resource.opacity/255.0;
        switch(resource.blend) {
        case 0:result=src+dst*(1-opacity);break;
        case 1:result=src+dst;break;
        case 3:src=(1-texture)*tint/255.0;result=resource.opacity==255?0:src+dst*(1-opacity);break;
        case 4:case 5:src=(1-texture)*tint/255.0;result=src+dst*(1-dst);break;
        case 6:result=dst*src;break;
        }
        return int(max(0.0,min(1.0,result))*maximum+0.5);
    };
    for(int rowIndex=0;rowIndex<info.shHeight;++rowIndex) {
        const WORD* run=next+1;const WORD* end=run+*next;next=end;
        const int dy=y+rowIndex;if(dy<0 || dy>=int(surface.dwHeight)) continue;
        WORD* row=reinterpret_cast<WORD*>(static_cast<BYTE*>(surface.lpSurface)+dy*surface.lPitch);
        int dx=x;
        while(run+1<end) {
            const WORD tag=*run++,count=*run++;
            if(tag==0xc0) {dx+=count;continue;}
            if(tag<0xc1 || tag>0xc3 || run+count>end) break;
            for(int i=0;i<count;++i,++dx) {
                if(dx<0 || dx>=int(surface.dwWidth)) continue;
                const WORD pixel=run[i],dst=row[dx];
                row[dx]=WORD((channel(pixel>>11,dst>>11,31,resource.tint[0])<<11) |
                    (channel((pixel>>5)&63,(dst>>5)&63,63,resource.tint[1])<<5) |
                    channel(pixel&31,dst&31,31,resource.tint[2]));
            }
            run+=count;
        }
    }
    buffer->Unlock(nullptr);
}
}
