#pragma once
#include <climits>

// x87 operation order and the deliberate double loss spill match 0x4D0BC4.
namespace MerchantPricing {
constexpr unsigned short Control = 0x037f;
constexpr int MeatMode = 40, OreMode = 43, MinimumOreMaximum = 10000;
// Match the reference server's extended-precision operations and ROUND.
// See Test/Shop/native-price-audit.md; MSVC double/round changes half-way cases.
inline long long Purchase(int basePrice, int percent)
{
    static const float PercentDivisor = 100.0f;
    static const unsigned char Surcharge[] = {0xcd,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0x8c,0xff,0x3f};
    const unsigned short PriceControl = 0x037f; // 64-bit precision, nearest/even, masked exceptions.
    unsigned short savedControl;
    long long price;
    __asm {
        fnstcw savedControl
        fldcw PriceControl
        fild basePrice
        fdiv PercentDivisor
        fild percent
        fmulp st(1), st(0)
        fld tbyte ptr Surcharge
        fmulp st(1), st(0)
        fistp qword ptr price
        fldcw savedControl
    }
    return price;
}

inline long long Scale(int price, int divisor, int multiplier)
{
    unsigned short saved; long long value;
    __asm {
        fnstcw saved
        fldcw Control
        fild price
        fild divisor
        fdivp st(1), st(0)
        fild multiplier
        fmulp st(1), st(0)
        fistp qword ptr value
        fldcw saved
    }
    return value;
}
inline long long Damage(int price, int current, int maximum)
{
    static const float Two = 2.0f;
    const int missing = maximum-current;
    double loss; long long value; unsigned short saved;
    __asm {
        fnstcw saved
        fldcw Control
        fild price
        fdiv Two
        fild maximum
        fdivp st(1), st(0)
        fild missing
        fmulp st(1), st(0)
        fstp qword ptr loss
        fild price
        fsub qword ptr loss
        fistp qword ptr value
        fldcw saved
    }
    return value < 2 ? 2 : value;
}
inline long long Premium(int price, int current, int maximum, bool ore)
{
    static const float Two = 2.0f;
    static const unsigned char OreFactor[] = {0x66,0x66,0x66,0x66,0x66,0x66,0x66,0xa6,0xff,0x3f};
    const int excess = current-maximum;
    unsigned short saved; long long value;
    __asm {
        fnstcw saved
        fldcw Control
        fild price
        fild maximum
        fdivp st(1), st(0)
        cmp ore, 0
        je meatFactor
        fld tbyte ptr OreFactor
        fmulp st(1), st(0)
        jmp multiplied
    meatFactor:
        fmul Two
    multiplied:
        fild excess
        fmulp st(1), st(0)
        fistp qword ptr value
        fldcw saved
    }
    return price+value;
}
inline int Base(int price, int standard, int current, int maximum, int mode, int stackMode)
{
    if (price<0 || stackMode>=2 || !standard || !maximum) return price;
    long long value = price;
    if (mode==OreMode && maximum<MinimumOreMaximum) maximum=MinimumOreMaximum;
    if (mode==MeatMode || mode==OreMode) {
        value=current>maximum ? Premium(price,current,maximum,mode==OreMode) : Damage(price,current,maximum);
        if (value>INT_MAX) return -1;
    }
    value=Scale(int(value),standard,maximum);
    if (value<0 || value>INT_MAX) return -1;
    value=Damage(int(value),current,maximum);
    return value>INT_MAX ? -1 : int(value);
}
}
