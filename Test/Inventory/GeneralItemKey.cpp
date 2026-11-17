#include "../../Server/Def/GeneralItemKey.h"
#include <cstring>
#include <cstdio>
#include <initializer_list>

bool RoundTrip(unsigned standard, unsigned durability, unsigned maximum)
{
    char buffer[15]; std::memset(buffer, '#', sizeof(buffer));
    GeneralItemKey::Encode(buffer+1,standard,durability,maximum);
    GeneralItemKey::Fields fields;
    return buffer[0]=='#' && buffer[14]=='#' && buffer[13]==0 && std::strlen(buffer+1)==12 &&
        GeneralItemKey::Decode(buffer+1,fields) && fields.standard==standard &&
        fields.durability==durability && fields.maximum==maximum;
}
int main()
{
    char key[13];
    GeneralItemKey::Encode(key,999,9999,9999);
    if(std::strcmp(key,"G99999999999")) return 1;
    GeneralItemKey::Encode(key,65535,65535,65535);
    if(std::strcmp(key,"G~7VVVVVVVVV")) return 2;
    GeneralItemKey::Fields fields;
    if(!GeneralItemKey::Decode("G00080008000",fields) || fields.standard || fields.durability!=8000 || fields.maximum!=8000) return 3;
    for(unsigned value=0;value<=65535;++value)
        if(!RoundTrip(value,10000,65535) || !RoundTrip(999,value,8000) || !RoundTrip(1007,9999,value)) return 4;
    for(const char* invalid : {"A00080008000","G0008000800x","G~8VVVVVVVVV","G~7VVVVVVVVW","G~7VVVVVVVV/"})
        if(GeneralItemKey::Decode(invalid,fields)) return 5;
    std::puts("PASS: legacy and packed golden keys, 196608 full-width round trips, buffer guards, malformed keys");
    return 0;
}
