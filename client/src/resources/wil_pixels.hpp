#pragma once
#include <algorithm>
#include <cstdint>
#include <span>
#include <utility>
#include <vector>

namespace mir3 {
struct WilPixels {
    std::vector<std::uint8_t> rgba;
    // 0=transparent, 1=C1 normal, 2=C2 first dye, 3=C3 second dye.
    std::vector<std::uint8_t> regions;
};

// RLE row structure and C0..C3 runs: 0x48EF15, also 0x48CD97.
// Bounds checks deliberately reject truncated streams instead of original OOB reads.
inline bool decode_wil_pixels(std::span<const std::uint8_t> source,int width,int height,WilPixels& out) {
    if (width<=0 || height<=0 || width>4096 || height>4096) return false;
    const auto word=[&](std::size_t p) { return unsigned(source[p]) | (unsigned(source[p+1])<<8); };
    WilPixels next;
    const auto size=static_cast<std::size_t>(width)*height;
    next.rgba.resize(size*4);
    next.regions.resize(size);
    std::size_t cursor=0;
    for (int y=0;y<height;++y) {
        if (cursor+2>source.size()) return false;
        const auto end=cursor+word(cursor)*2+2;
        cursor+=2;
        if (end>source.size()) return false;
        unsigned x=0;
        while (cursor<end) {
            if (cursor+4>end) return false;
            const unsigned opcode=word(cursor),count=word(cursor+2);
            cursor+=4;
            if (opcode!=0xC0) {
                if (opcode<0xC1 || opcode>0xC3 || cursor+count*2>end) return false;
                const unsigned visible=x<unsigned(width)?std::min(count,unsigned(width)-x):0;
                for (unsigned n=0;n<visible;++n) {
                    const auto rgb=word(cursor+n*2);
                    const auto pixel=static_cast<std::size_t>(y)*width+x+n;
                    next.rgba[pixel*4]=static_cast<std::uint8_t>((rgb>>8)&0xF8);
                    next.rgba[pixel*4+1]=static_cast<std::uint8_t>((rgb>>3)&0xFC);
                    next.rgba[pixel*4+2]=static_cast<std::uint8_t>((rgb<<3)&0xF8);
                    next.rgba[pixel*4+3]=255;
                    next.regions[pixel]=static_cast<std::uint8_t>(opcode-0xC0);
                }
                cursor+=count*2;
            }
            x+=count;
        }
    }
    out=std::move(next);
    return true;
}

struct Rgb565 {
    // 0x48C2DB with RGB565 surface bit counts/shifts.
    static constexpr std::uint16_t pack(std::uint8_t r,std::uint8_t g,std::uint8_t b) {
        return ((r>>3)<<11)|((g>>2)<<5)|(b>>3);
    }
    // 0x48EF15 C2/C3: truncate each channel before packed blending.
    static constexpr std::uint16_t tint(std::uint16_t source,std::uint16_t color) {
        return (((source>>11)*(color>>11)/31)<<11) |
            ((((source>>5)&63)*((color>>5)&63)/63)<<5) |
            ((source&31)*(color&31)/31);
    }
    // 0x48F31D/0x48F397: separately truncate source and destination terms.
    // This differs from both RGBA alpha and a single rounded weighted average.
    static constexpr std::uint16_t blend(std::uint16_t source,std::uint16_t destination,unsigned alpha) {
        if (alpha>32) alpha=32; // safety guard; HUD caller uses exactly 25.
        const auto scale=[](std::uint16_t value,unsigned weight) {
            return (((value>>11)*weight/32)<<11) |
                ((((value>>5)&63)*weight/32)<<5) | ((value&31)*weight/32);
        };
        return static_cast<std::uint16_t>(scale(source,alpha)+scale(destination,32-alpha));
    }
};
}
