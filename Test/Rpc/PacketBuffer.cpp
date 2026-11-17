#include "../../Client/Common/PacketBuffer.h"
#include <cstring>
#include <iostream>
#include <vector>

int main() {
    PacketBuffer buffer;std::vector<std::string> messages;
    auto parse=[&](char* data)->char* {
        while(char* end=std::strchr(data,'!')) {
            *end=0;messages.emplace_back(data);data=end+1;
        }
        return *data?data:nullptr;
    };
    const std::string stream="#first!#second!#third!";
    for(size_t split=0;split<=stream.size();++split) {
        buffer.Clear();messages.clear();
        buffer.Append(stream.substr(0,split).c_str(),parse);
        buffer.Append(stream.substr(split).c_str(),parse);
        if(messages!=std::vector<std::string>{"#first","#second","#third"}) return 1;
    }
    buffer.Clear();messages.clear();
    for(char c:stream) {const char chunk[]={c,0};buffer.Append(chunk,parse);}
    if(messages.size()!=3 || messages.back()!="#third") return 2;
    buffer.Clear();messages.clear();
    std::string bulk;
    for(int i=0;i<2000;++i) bulk+="#message"+std::to_string(i)+"!";
    buffer.Append((bulk+"#partial").c_str(),parse);
    if(messages.size()!=2000 || messages.front()!="#message0" || messages.back()!="#message1999") return 3;
    buffer.Append("tail!",parse);
    if(messages.back()!="#partialtail") return 4;
    const std::string large="#"+std::string(32768,'x');
    buffer.Append(large.c_str(),parse);
    buffer.Append("!",parse);
    if(messages.back()!=large) return 5;
    buffer.Append("#old_session",parse);buffer.Clear();
    buffer.Append("#new_session!",parse);
    if(messages.back()!="#new_session") return 6;
    std::cout<<"PASS packet buffer: split boundaries, byte chunks, 2000 coalesced frames, 32KiB partial frame, session reset\n";
}
