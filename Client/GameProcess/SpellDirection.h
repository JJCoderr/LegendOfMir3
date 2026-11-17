#pragma once
#include <array>
#include <cmath>
namespace mir3 {
// 453C00/453D20 use rounded float cosine thresholds and world-pixel deltas.
inline int SpellDirection(float x,float y,bool eight=false) {
    if(x==0 && y==0) return 0;
    const int count=eight?8:16,quadrant=x<0?(y<0?3:2):(y<0?0:1);
    const double leading=quadrant%2?std::abs(x):std::abs(y);
    constexpr std::array<float,8> thresholds={1.f,.980785f,.923880f,.831470f,.707107f,.555570f,.382683f,.195090f};
    const double cosine=leading/std::hypot(double(x),double(y));
    const int stride=eight?2:1;int slot=0;
    while(slot<count/2 && cosine<=thresholds[slot*stride]) ++slot;
    return (quadrant*(count/4)+slot/2)%count;
}
}
