#pragma once
#include <string>
#include <cstring>

inline std::string LimitAnsiEdit(const std::string& previous,const std::string& next,size_t limit)
{
    const auto boundary=[](const std::string& text,size_t index) {
        size_t i=0; while (i<index) i+=IsDBCSLeadByteEx(CP_ACP,BYTE(text[i])) && i+1<text.size()?2:1;
        return i==index;
    };
    size_t prefix=0,suffix=0;
    while (prefix<previous.size() && prefix<next.size() && previous[prefix]==next[prefix]) ++prefix;
    while (prefix && (!boundary(previous,prefix) || !boundary(next,prefix))) --prefix;
    while (suffix<previous.size()-prefix && suffix<next.size()-prefix && previous[previous.size()-1-suffix]==next[next.size()-1-suffix]) ++suffix;
    while (suffix && (!boundary(previous,previous.size()-suffix) || !boundary(next,next.size()-suffix))) --suffix;
    const size_t retained=prefix+suffix,available=retained<limit?limit-retained:0;
    size_t inserted=min(next.size()-retained,available);
    while (inserted && !boundary(next,prefix+inserted)) --inserted;
    return next.substr(0,prefix)+next.substr(prefix,inserted)+next.substr(next.size()-suffix);
}

inline bool IsAnsiFileComponent(const std::string& name,UINT codePage=CP_ACP)
{
    if(name.empty()) return false;
    for(size_t i=0;i<name.size();++i) {
        const BYTE c=BYTE(name[i]);
        if(IsDBCSLeadByteEx(codePage,c)) {if(++i==name.size() || !name[i]) return false;continue;}
        if(c<32 || std::strchr("\\/:*?\"<>|",c)) return false;
    }
    return true;
}
