#pragma once
#include <string>

// The legacy message parser consumes complete frames and returns the first
// incomplete frame, or nullptr when it consumed the whole buffer.
class PacketBuffer {
    std::string pending;
public:
    template<class Parser>
    void Append(const char* packet,Parser parse) {
        pending+=packet;
        char* data=&pending[0];
        char* remaining=parse(data);
        if (remaining) pending.erase(0,size_t(remaining-data));
        else pending.clear();
    }
    void Clear() {pending.clear();}
};
