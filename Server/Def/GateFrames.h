#pragma once
#include <string>

// DBSvr replies are %socket/#payload!$ frames, optionally followed by NUL padding.
class GateFrames {
    std::string pending;
public:
    template<class Receiver>
    void Append(const char* data,size_t size,Receiver receive) {
        pending.append(data,size);
        size_t consumed=0;
        while (true) {
            const auto begin=pending.find('%',consumed);
            if (begin==std::string::npos) {pending.clear();return;}
            const auto end=pending.find('$',begin);
            if (end==std::string::npos) {pending.erase(0,begin);return;}
            receive(pending.substr(begin,end-begin));
            consumed=end+1;
        }
    }
};
