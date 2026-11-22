#pragma once
#include <cstdint>
#include <cstdlib>
#include <string>
namespace GroundItemDisplay {
constexpr int CellCapacity=100,NameLimit=39,NearbyRadius=24;
constexpr int SparkleFirst=41,SparkleFrames=9,SparkleInterval=100,CoinNameMessage=1409;
struct Sparkle {
    std::uint32_t started=0,elapsed=0;
    int delay=3000,frame=-1;
    void Begin(std::uint32_t now,int first,int second) {
        started=now;elapsed=0;frame=-1;delay=3000+std::abs(first-second)%4001;
    }
    void Advance(std::uint32_t now,std::uint32_t delta) {
        if(now-started<=std::uint32_t(delay)) return;
        if(frame<0) frame=0;
        elapsed+=delta;
        if(elapsed>SparkleInterval) {
            elapsed=0;
            if(++frame==SparkleFrames) {started=now;frame=-1;}
        }
    }
};
inline bool Nearby(int dx,int dy) {
    return dx>=-NearbyRadius && dx<NearbyRadius && dy>=-NearbyRadius && dy<NearbyRadius;
}
inline bool CoinVariant(const std::string& name,const std::string& coin) {
    return !coin.empty() && name.size()==coin.size()+1 && name.compare(0,coin.size(),coin)==0 && name.back()>='0' && name.back()<='9';
}
}
