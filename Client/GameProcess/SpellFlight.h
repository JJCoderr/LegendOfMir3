#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace mir3 {
// 4497CD / 44B7E0, world pixels. Scene ownership resolves target identity separately.
struct SpellFlight {
    using Point=std::array<float,2>;
    Point position={},destination={},velocity={},previous_distance={};
    static constexpr float Step=0.0011111111380159855f;
    static constexpr int Speed=500,ImpactRadius=15;
    void begin(Point source,Point target) {
        position=source;destination=target;
        const Point delta={target[0]-source[0],target[1]-source[1]};
        const int x=(std::max)(1,std::abs(int(delta[0]))),y=(std::max)(1,std::abs(int(delta[1])));
        const double factor=x>y?double(Speed)/x:double(float(double(Speed)/y));
        for(int axis=0;axis<2;++axis) {
            velocity[axis]=float(std::trunc(double(delta[axis])*factor));
            previous_distance[axis]=std::abs(delta[axis]);
        }
    }
    // Returns impact geometry only; the caller applies skill-specific 3s/lifetime rules.
    bool advance(std::uint32_t milliseconds,bool following) {
        if(following) {
            const Point delta={destination[0]-std::trunc(position[0]),destination[1]-std::trunc(position[1])};
            const int dominant=(std::max)(1,int((std::max)(std::abs(delta[0]),std::abs(delta[1]))));
            const int factor=Speed/dominant;
            velocity={delta[0]*factor,delta[1]*factor};
        }
        Point distance={};
        for(int axis=0;axis<2;++axis) {
            position[axis]=float(double(velocity[axis])*double(Step)*milliseconds+position[axis]);
            distance[axis]=std::abs(destination[axis]-std::trunc(position[axis]));
        }
        const bool approaching=distance[0]<previous_distance[0] || distance[1]<previous_distance[1];
        const bool impact=(milliseconds>0 && !approaching) || (distance[0]<=ImpactRadius && distance[1]<=ImpactRadius);
        previous_distance=distance;return impact;
    }
};
}
