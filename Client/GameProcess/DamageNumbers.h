#pragma once
#include <array>
#include <string>
#include <vector>
struct CDamageNumbers {
    struct Glyph { int frame,x,y; };
    std::array<int,8> values{};
    std::array<WORD,8> ages{};
    int cursor=0;
    bool enabled=true;
    void Push(int value) {if(!enabled) return;values[cursor]=value;ages[cursor]=0;if(++cursor>7) cursor=0;}
    void Changed(int before,int after) {if(enabled && before && before!=after) Push(after-before);}
    static std::vector<Glyph> Glyphs(int value,int x,int y) {
        if(!value) return {{1850,x-10,y}};
        if(value < -65535 || value>65535) return {};
        const bool heal=value>0;const int amount=heal?value:-value;
        std::vector<Glyph> result={{heal?1821:amount<100?1800:1840,x-(heal?10:8),y+(heal?0:4)}};
        const int base=heal?1810:amount<100?1790:1830;
        for(char digit:std::to_string(amount)) {result.push_back({base+digit-'0',x,y});x+=11;}return result;
    }
    std::vector<Glyph> Draw(int x,int y,bool human) {
        std::vector<Glyph> result;if(!enabled) return result;
        // Preserve MIR3's [0,cursor) loop, including its eight-entry wrap.
        for(int i=0;i<cursor;++i) if(ages[i]<=40) {
            auto glyphs=Glyphs(values[i],x+ages[i]+20,int(double(y-(human?40:30))-double(ages[i])*1.3));
            result.insert(result.end(),glyphs.begin(),glyphs.end());++ages[i];
        }
        return result;
    }
};
