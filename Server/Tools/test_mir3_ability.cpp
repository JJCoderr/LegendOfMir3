#include "../Def/Mir3Ability.h"
#include <cstdio>
#include <cstring>

// Deliberately use the old field order, distinct ranges and full-width values.
struct OldAbility {
    std::uint8_t Level;
    std::uint16_t HP, MP, MaxHP, MaxMP, Weight, MaxWeight;
    std::uint32_t Exp, MaxExp;
    std::uint8_t WearWeight, MaxWearWeight, HandWeight, MaxHandWeight;
    std::uint16_t DC, MC, SC, AC, MAC;
};

int main()
{
    const OldAbility source = {
        37, 0x0c0b, 0x0e0d, 0x100f, 0x1211, 0x1c1b, 0x1e1d,
        0x16151413, 0x1a191817, 31, 32, 33, 34,
        0x0605, 0x0807, 0x0a09, 0x0201, 0x0403
    };
    const auto record = Mir3Ability::FromAbility(source);
    const unsigned char expected[63] = {
        37, 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,
        19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34
    };
    if (std::memcmp(&record, expected, sizeof(expected)) != 0) {
        std::puts("FAIL: MIR3 ability golden bytes");
        return 1;
    }
    const auto empty = Mir3Ability::FromAbility(OldAbility{});
    const unsigned char zeros[63] = {};
    if (std::memcmp(&empty, zeros, sizeof(zeros)) != 0) return 1;
    std::puts("PASS: MIR3 ability order, widths, ranges, neutral extension and zero values");
    return 0;
}
