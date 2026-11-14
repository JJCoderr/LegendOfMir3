#pragma once
#include "hud/local_action_clock.hpp"
#include <stdexcept>

// 测试内容：移动600毫秒准入、攻击80%等待取整、移动480毫秒等待、初始800毫秒及DWORD回绕。
inline void local_action_clock_checks() {
    using mir3::LocalActionClock;
    const auto require=[](bool ok,const char* message) {if(!ok) throw std::runtime_error(message);};
    LocalActionClock clock;
    require(!clock.admit_move(599) && clock.admit_move(600),"movement boundary inclusive");
    require(!clock.admit_move(1100) && clock.admit_move(1200),"blocked poll does not extend movement throttle");
    require(clock.shared_wait(799,600)==1 && clock.shared_wait(800,600)==0,"reserved timestamp startup boundary");
    clock.attacked=10000;
    require(clock.shared_wait(10500,1386)==609,"fractional attack interval rounds upward");
    require(clock.shared_wait(11108,1386)==1 && clock.shared_wait(11109,1386)==0,"attack wait exact boundary");
    clock.moved=11000;
    require(clock.shared_wait(11109,1386)==371,"movement dominates common wait");
    require(clock.shared_wait(11479,1386)==1 && clock.shared_wait(11480,1386)==0,"movement 480 boundary");
    clock.attacked=12000;
    require(clock.shared_wait(12000,2886)==2309,"overweight affects common wait");
    clock.move_admitted=0xfffffff0u;
    require(!clock.admit_move(583) && clock.admit_move(584),"movement admission wraps");
    require(LocalActionClock::remaining(463,0xfffffff0u,480)==1 &&
        LocalActionClock::remaining(464,0xfffffff0u,480)==0,"pending wait wraps");
    clock.attacked=0xfffffff0u;clock.moved=0xfffffff0u;
    require(clock.shared_wait(1092,1386)==1 && clock.shared_wait(1093,1386)==0,"common wait wraps");
}
