#pragma once
#include <cstdint>
#include <optional>
// MIR3 human base descriptor (16 bytes), plus separately encoded mount extension (5 bytes).
struct CMir3HumanAppearance {
    std::uint8_t type=0,weapon=0,clothing=0,hair=0,profession=0;
    std::uint32_t flags=0;
    std::optional<std::uint8_t> mount;
    std::optional<std::uint32_t> portraitVersion,hitSource;
    static std::optional<CMir3HumanAppearance> Parse(const std::uint8_t* bytes,std::size_t size,bool hit,bool moving=false) {
        if((size!=16 && size!=21) || bytes[0]>1 || bytes[2]>=9) return {};
        CMir3HumanAppearance value;
        value.type=bytes[0];value.weapon=bytes[1]<50?bytes[1]:0;value.clothing=bytes[2];
        value.hair=(bytes[3]&63)<30?bytes[3]&63:0;value.profession=bytes[3]>>6;
        std::uint32_t context=0;
        for(int i=0;i<4;++i) {value.flags|=std::uint32_t(bytes[4+i])<<(8*i);context|=std::uint32_t(bytes[8+i])<<(8*i);}
        if(hit) value.hitSource=context;
        else {
            value.portraitVersion=context;
            if(size==21) value.mount=bytes[16]<4?bytes[16]:0;
            else if(!moving) value.mount=0;
        }
        return value;
    }
};
