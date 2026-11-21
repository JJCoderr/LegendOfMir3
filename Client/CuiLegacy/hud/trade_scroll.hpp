#pragma once
#include <algorithm>
#include <array>
#include <utility>

namespace mir3 {
struct TradeScroll {
    static constexpr std::array<int,2> x{206,457};
    static constexpr int top=58, travel=158, last=33;
    std::array<int,2> first{};
    int captured=-1;

    int offset(int side) const { return first[side]*travel/last; }

    bool pointer(int kind,int px,int py,int width,int height) {
        if (kind==2) return std::exchange(captured,-1)>=0;
        if (kind==1 && captured>=0) {
            const auto ratio=std::clamp(float(py-top)/(travel+height),0.f,1.f);
            first[captured]=int(ratio*last);
            return true;
        }
        if (kind!=0 || py<top || py>=top+travel+height) return false;
        for (int side=0;side<2;++side) {
            if (px<x[side] || px>=x[side]+width) continue;
            const int thumb=top+offset(side);
            if (py>=thumb && py<thumb+height) captured=side;
            else first[side]=std::clamp(first[side]+(py<thumb?-6:6),0,last);
            return true;
        }
        return false;
    }
};
}
