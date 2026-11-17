#pragma once
#include <array>
#include "SpellRoutes.h"

namespace ExitFlow {
constexpr int CharacterDialog=101,ProgramDialog=109;
constexpr int CharacterCaption=1065,ProgramCaption=2636,BlockedCaption=1407,ConnectingCaption=200;
constexpr WORD Logout=1009;
constexpr DWORD CombatDelay=8000,TransitionDuration=2500;
constexpr BYTE DeathAction=19;
enum Clock { Struck,Spell,Attack };
inline bool RestrictsExit(WORD spell) { return mir3::SpellRoutes::get(spell).restrict_followup; }
struct Guard {
    std::array<DWORD,3> ticks={};
    bool Allowed(DWORD now,BYTE action) const {
        if(action==DeathAction) return true;
        for(DWORD tick:ticks) if(DWORD(now-tick)<=CombatDelay) return false;
        return true;
    }
};
struct Transition {
    bool active=false;
    DWORD elapsed=0;
    void Begin() {active=true;elapsed=0;}
    bool Advance(DWORD milliseconds) {
        if(!active) return false;
        elapsed+=milliseconds;
        if(elapsed<=TransitionDuration) return false;
        active=false;elapsed=0;return true;
    }
    float Brightness() const {return active?float((255.0-double(elapsed)*double(0.1f))*double(1.f/255.f)):1.f;}
};
}
