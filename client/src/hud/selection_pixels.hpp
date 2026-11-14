#pragma once
#include "resources/wil_archive.hpp"
namespace mir3 {
enum class SelectionBlend { normal, inactive, shadow, additive };
inline void selection_blit(std::vector<std::uint8_t>& destination,const WilFrame& source,int x,int y,SelectionBlend blend) {
    if (destination.size()!=640*480*4) return;
    x+=source.offset_x;y+=source.offset_y;
    for (int sy=0;sy<source.height;++sy) for (int sx=0;sx<source.width;++sx) {
        const int dx=x+sx,dy=y+sy;
        if (dx<0 || dy<0 || dx>=640 || dy>=480) continue;
        const auto si=(std::size_t(sy)*source.width+sx)*4,di=(std::size_t(dy)*640+dx)*4;
        if (!source.pixels.rgba[si+3]) continue;
        int r=source.pixels.rgba[si]>>3,g=source.pixels.rgba[si+1]>>2,b=source.pixels.rgba[si+2]>>3;
        if (blend==SelectionBlend::inactive) {
            const int gray=(r+(g>>1)+b)/3;
            r=b=std::min(31,gray+int(gray*0.6f));g=r*2;
        } else if (blend==SelectionBlend::shadow) {
            r=(r+(destination[di]>>3))/2;g=(g+(destination[di+1]>>2))/2;b=(b+(destination[di+2]>>3))/2;
        } else if (blend==SelectionBlend::additive) {
            r=std::min(31,r+(destination[di]>>3));g=std::min(63,g+(destination[di+1]>>2));b=std::min(31,b+(destination[di+2]>>3));
        }
        destination[di]=r<<3;destination[di+1]=g<<2;destination[di+2]=b<<3;destination[di+3]=255;
    }
}
}
