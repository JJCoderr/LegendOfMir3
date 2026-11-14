#pragma once
#include "magic/spell_sound_voices.hpp"
#include <set>
#include <stdexcept>

// 声道轮转、满载不抢占、播放结束复用及50缓存的一秒严格淘汰边界。
inline void spell_sound_checks() {
    using mir3::SpellSoundVoices;
    const auto require=[](bool ok,const char* message) {if(!ok) throw std::runtime_error(message);};
    SpellSoundVoices pool;
    std::set<std::int64_t> playing;
    const auto active=[&](std::int64_t key) {return playing.contains(key);};
    for(int i=0;i<10;++i) {
        const auto voice=pool.select(10532,100,active);
        require(voice && voice->play && !playing.contains(voice->key),"同一声音十声道独立播放");
        playing.insert(voice->key);
    }
    const auto full=pool.select(10532,101,active);
    require(full && !full->play,"十声道已满不抢占");
    playing.erase(SpellSoundVoices::key(0,3));
    const auto free=pool.select(10532,102,active);
    require(free && free->play && free->key==SpellSoundVoices::key(0,3),"找到停止声道后复用");
    require(free->key<INT32_MIN,"声道键不占用任何32位角色编号");
    const auto other=pool.select(10011,103,active);
    require(other && other->play && !playing.contains(other->key),"不同音效不互相覆盖");
    SpellSoundVoices cache;
    for(int i=0;i<50;++i) require(cache.select(10000+i,0,active).has_value(),"填满五十音效缓存");
    require(!cache.select(11000,1000,active),"缓存创建后一秒整不淘汰");
    const auto replaced=cache.select(11000,1001,active);
    require(replaced && replaced->replaced && replaced->cache==0,"超过一秒按槽位淘汰首项");
}
