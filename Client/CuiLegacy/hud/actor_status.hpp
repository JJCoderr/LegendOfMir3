#pragma once

#include <algorithm>
#include <cstdint>
#include <span>

namespace mir3 {
inline constexpr std::uint16_t actor_status_color(std::uint32_t flags) {
    if (flags&0x00020000u) return 1;
    if (flags&0x80000000u) return 2016;
    if (flags&0x40000000u) return 64690;
    if (flags&0x08000000u) return 38079;
    if (flags&0x20000000u) return 64511;
    if (flags&0x04000000u) return 0;
    if (flags&0x00800000u) return 1;
    return 65535;
}

inline void tint_status_pixels(std::span<std::uint8_t> rgba,std::span<const std::uint8_t> regions,
                               std::uint16_t color,bool highlighted) {
    const auto channel=[highlighted](int source,int tint,int maximum) {
        const double level=double(source)*tint/maximum;
        return std::min(maximum,int(level)+(highlighted ? int(level*double(0.6f)) : 0));
    };
    const auto count=std::min(regions.size(),rgba.size()/4);
    for (std::size_t i=0;i<count;++i) {
        if (!regions[i]) continue;
        auto* pixel=rgba.data()+i*4;
        if (!color) {
            const int gray=(pixel[0]/8+pixel[1]/8+pixel[2]/8)/3;
            const int value=8*std::min(31,gray+(highlighted ? gray*3/5 : 0));
            pixel[0]=pixel[1]=pixel[2]=std::uint8_t(value);
        } else {
            pixel[0]=std::uint8_t(8*channel(pixel[0]/8,color>>11,31));
            pixel[1]=std::uint8_t(4*channel(pixel[1]/4,(color>>5)&63,63));
            pixel[2]=std::uint8_t(8*channel(pixel[2]/8,color&31,31));
        }
    }
}
}
