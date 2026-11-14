#pragma once
#include <cstdint>
#include <optional>
#include <span>
namespace mir3 {
// Decoded feature payload used by 0x4120A0 / 0x412350 / 0x412500.
struct HumanAppearance {
    int type=0,weapon=0,clothing=0,hair=0,profession=0;
    std::optional<int> mount;
    std::uint32_t flags=0;
    static std::optional<HumanAppearance> parse(std::span<const std::uint8_t> bytes,bool hit) {
        if ((bytes.size()!=16 && bytes.size()!=21) || bytes[0]>1 || bytes[2]>=9) return {};
        HumanAppearance value;
        value.type=bytes[0];value.weapon=bytes[1]<50?bytes[1]:0;value.clothing=bytes[2];
        value.hair=(bytes[3]&63)<30?bytes[3]&63:0;value.profession=bytes[3]>>6;
        if (!hit) value.mount=bytes.size()==21 && bytes[16]<4?bytes[16]:0;
        for (int i=0;i<4;++i) value.flags|=std::uint32_t(bytes[4+i])<<(8*i);
        return value;
    }
};
}
