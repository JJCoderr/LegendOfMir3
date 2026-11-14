#pragma once
#include "hud/melee_cooldown.hpp"
#include <stdexcept>

// 测试内容：原版等级/速度双上限、负速度、手持超重、严格边界、拒绝不续期和DWORD回绕。
inline void melee_cooldown_checks() {
    using mir3::MeleeCooldown;
    const auto require=[](bool ok,const char* message) { if(!ok) throw std::runtime_error(message); };
    require(MeleeCooldown::interval(1,0,0,0)==1386,"level one interval");
    require(MeleeCooldown::interval(26,0,0,0)==1036,"below level cap");
    require(MeleeCooldown::interval(27,0,0,0)==1030,"level cap");
    require(MeleeCooldown::interval(255,100,0,0)==600,"total discount cap");
    require(MeleeCooldown::interval(1,-1,0,0)==1446,"signed negative speed");
    require(MeleeCooldown::interval(1,0,9,9)==1386,"equal weight allowed");
    require(MeleeCooldown::interval(1,0,10,9)==2886,"overweight penalty");
    require(MeleeCooldown::interval(1,-32768,0,0)==1967466,"signed speed minimum");
    MeleeCooldown gate;
    require(!gate.accept(1386,1,0,0) && gate.accept(1387,1,0,0),"strict initial boundary");
    require(!gate.accept(2000,1,0,0) && !gate.accept(2773,1,0,0),"rejection and exact boundary");
    require(gate.accept(2774,1,0,0),"rejection must not extend cooldown");
    gate.last_allowed=0xfffffff0u;
    require(!gate.accept(1370,1,0,0) && gate.accept(1371,1,0,0),"DWORD wraparound");
    gate={};gate.speed=100;
    require(gate.accept(601,1,0,0),"updated speed used");
    require(!gate.accept(2000,1,2,1) && gate.accept(2702,1,2,1),"updated load used");
}
