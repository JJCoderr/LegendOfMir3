#include "../../Server/Def/MerchantPricing.h"
#include <cstdio>
int main() {
    struct Case { int price,standard,current,maximum,mode,stack,expected; };
    const Case cases[]={
        {100,8000,4000,8000,30,0,75},{1000,5000,4000,5000,25,0,900},
        {200,10000,8000,10000,40,0,162},{200,10000,12000,10000,40,0,308},
        {500,10000,12000,10000,43,0,693},{500,10000,8000,8000,43,0,405},
        {11,8000,4000,8000,30,0,8},{13,8000,4000,4000,30,0,6},
        {100,8000,0,8000,30,0,50},{0,8000,4000,8000,30,0,2},
        {11,0,4000,8000,30,0,11},{11,8000,4000,0,30,0,11},
        {11,8000,4000,8000,30,2,11},{INT_MAX,1,65535,65535,30,0,-1}
    };
    if(MerchantPricing::Purchase(15,100)!=17 || MerchantPricing::Purchase(35,100)!=38 || MerchantPricing::Purchase(50,150)!=82) return 3;
    unsigned short original,after;
    const unsigned short unusual=0x0c7f;
    __asm { fnstcw original }
    __asm { fldcw unusual }
    for(const auto& c:cases) {
        const int got=MerchantPricing::Base(c.price,c.standard,c.current,c.maximum,c.mode,c.stack);
        if(got!=c.expected) { std::printf("FAIL mode=%d got=%d expected=%d\n",c.mode,got,c.expected);return 1; }
    }
    __asm { fnstcw after }
    __asm { fldcw original }
    if(after!=unusual) return 2;
    std::puts("PASS: 17 native pricing cases and caller x87 control restoration");
}
