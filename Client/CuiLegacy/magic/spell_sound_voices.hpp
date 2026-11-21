#pragma once
#include <array>
#include <cstdint>
#include <optional>

namespace mir3 {

struct SpellSoundVoices {
    static constexpr int samples=50,voices=10;
    struct Sample { int id=0,cursor=0;std::uint32_t loaded=0; };
    struct Selection { std::int64_t key=0;int cache=-1;bool play=false,replaced=false; };
    std::array<Sample,samples> cache{};

    static constexpr std::int64_t key(int slot,int voice) {return -10'000'000'001LL-slot*voices-voice;}

    template<class Playing>
    std::optional<Selection> select(int sample,std::uint32_t now,Playing playing) {
        int slot=-1;
        for(int i=0;i<samples;++i) if(cache[i].id==sample) {slot=i;break;}
        bool replaced=false;
        if(slot<0) {
            for(int i=0;i<samples;++i) if(!cache[i].id) {slot=i;break;}
            if(slot<0) for(int i=0;i<samples;++i)
                if(std::uint32_t(now-cache[i].loaded)>1000) {slot=i;break;}
            if(slot<0) return {};
            cache[slot]={sample,0,now};replaced=true;
        }
        auto& item=cache[slot];

        if(replaced) return Selection{key(slot,0),slot,true,true};
        for(int i=0;i<voices;++i) {
            if(!playing(key(slot,item.cursor))) return Selection{key(slot,item.cursor),slot,true,false};
            item.cursor=(item.cursor+1)%voices;
        }

        return Selection{key(slot,item.cursor),slot,false,false};
    }
};
}
