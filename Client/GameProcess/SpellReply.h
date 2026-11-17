#pragma once
#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <string>
#include <vector>

namespace SpellWire {
constexpr WORD SingleReply=638,MultipleReply=1410;
constexpr int MaximumListBytes=1024,TargetBytes=4;
constexpr int MaximumEncodedList=(MaximumListBytes*8+5)/6;
constexpr unsigned char EncodingFirst=0x3c,EncodingLast=0x7b;
struct Reply {
    WORD skill=0,x=0,y=0;
    bool multiple=false;
    std::vector<DWORD> targets;
    static std::optional<Reply> Parse(const _TDEFAULTMESSAGE& header,const char* encoded) {
        if(header.wIdent!=SingleReply && header.wIdent!=MultipleReply) return {};
        const size_t length=strlen(encoded);
        if(length>MaximumEncodedList) return {};
        for(size_t i=0;i<length;++i) {
            const auto value=static_cast<unsigned char>(encoded[i]);
            if(value<EncodingFirst || value>EncodingLast) return {};
        }
        std::array<char,MaximumListBytes+1> decoded={};
        const int count=fnDecode6BitBuf(const_cast<char*>(encoded),decoded.data(),int(decoded.size()));
        if(count<0 || count>MaximumListBytes) return {};
        Reply result;result.skill=LOBYTE(header.wSeries);result.x=header.wParam;result.y=header.wTag;
        result.multiple=header.wIdent==MultipleReply;
        if(!result.multiple) {
            if(length!=(TargetBytes*8+5)/6 || count!=TargetBytes) return {};
            DWORD target=0;for(int i=0;i<TargetBytes;++i) target|=DWORD(static_cast<unsigned char>(decoded[i]))<<(8*i);
            result.targets.push_back(target);
        } else {
            const std::string text(decoded.data());size_t start=0;
            for(;;) {
                const auto end=text.find('/',start);const auto token=text.substr(start,end==text.npos?end:end-start);
                const auto number=(std::clamp)(std::strtoll(token.c_str(),nullptr,10),-2147483648LL,2147483647LL);
                result.targets.push_back(DWORD(number));
                if(end==text.npos) break;start=end+1;
            }
        }
        return result;
    }
};
}
