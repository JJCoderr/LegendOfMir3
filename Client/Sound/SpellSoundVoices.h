#pragma once
#include <array>
#include <cstdint>
#include <optional>

namespace mir3 {
// 4832D0/482F10：50个音效缓存；483B20：每种10声道，满时不抢占。
struct SpellSoundVoices {
    static constexpr int samples=50,voices=10;
    struct Sample { int id=0,cursor=0;std::uint32_t loaded=0; };
    struct Selection { std::int64_t key=0;int cache=-1;bool play=false,replaced=false; };
    std::array<Sample,samples> cache{};
    // 场景角色编号可覆盖整个DWORD，声道键放在32位范围外，避免与角色碰撞。
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
        // 重载缓存的旧播放器先由调用者释放，不把其播放状态算到新资源上。
        if(replaced) return Selection{key(slot,0),slot,true,true};
        for(int i=0;i<voices;++i) {
            if(!playing(key(slot,item.cursor))) return Selection{key(slot,item.cursor),slot,true,false};
            item.cursor=(item.cursor+1)%voices;
        }
        // 原版仍返回当前声道编号，但没有发起播放。
        return Selection{key(slot,item.cursor),slot,false,false};
    }
};
}
