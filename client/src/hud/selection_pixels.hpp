#pragma once
#include "resources/wil_archive.hpp"
namespace mir3 {
enum class SelectionBlend { normal, inactive, shadow, additive };
// 原版矩形采用 ONE / INVSRCALPHA；创建页先合成说明框，再绘制人物阴影。
inline void selection_rectangle(std::vector<std::uint8_t>& pixels,int x,int y,int width,int height,
        unsigned color,unsigned alpha) {
    if (pixels.size()!=640*480*4 || alpha>255) return;
    for (int py=std::max(0,y);py<std::min(480,y+height);++py)
        for (int px=std::max(0,x);px<std::min(640,x+width);++px) {
            const auto offset=(std::size_t(py)*640+px)*4;
            for (int c=0;c<3;++c) pixels[offset+c]=std::uint8_t(std::min(255u,
                ((color>>(8*c))&255u)+(pixels[offset+c]*(255-alpha)+127)/255));
            pixels[offset+3]=255;
        }
}
inline void selection_text(std::vector<std::uint8_t>& pixels,const std::vector<std::uint8_t>& glyphs,
        int x,int y,int width,int height,unsigned color) {
    if (pixels.size()!=640*480*4 || width<=0 || height<=0 || glyphs.size()!=std::size_t(width)*height*4) return;
    for (int sy=0;sy<height;++sy) for (int sx=0;sx<width;++sx) {
        const int dx=x+sx,dy=y+sy;if (dx<0 || dy<0 || dx>=640 || dy>=480) continue;
        const auto alpha=glyphs[(std::size_t(sy)*width+sx)*4+3];
        if (!alpha) continue;
        const auto offset=(std::size_t(dy)*640+dx)*4;
        for (int c=0;c<3;++c) pixels[offset+c]=std::uint8_t(
            (((color>>(8*c))&255u)*alpha+pixels[offset+c]*(255-alpha)+127)/255);
    }
}

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
