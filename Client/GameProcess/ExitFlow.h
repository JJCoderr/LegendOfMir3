#pragma once
#include <array>
#include "Mir3SkillIds.h"

namespace ExitFlow {
constexpr int CharacterDialog=101,ProgramDialog=109;
constexpr int CharacterCaption=1065,ProgramCaption=2636,BlockedCaption=1407,ConnectingCaption=200;
constexpr WORD Logout=1009;
constexpr DWORD CombatDelay=8000,TransitionDuration=2500;
constexpr BYTE DeathAction=19;
enum Clock { Struck,Spell,Attack };
inline bool RestrictsExit(WORD spell) {
    using mir3::SkillId;
    switch(static_cast<SkillId>(spell)) {
    case SkillId::fireball:
    case SkillId::fireball2:
    case SkillId::amyounsul:
    case SkillId::fire:
    case SkillId::shootlighten:
    case SkillId::ergum:
    case SkillId::firecharm:
    case SkillId::earthfire:
    case SkillId::lightflower:
    case SkillId::banwol:
    case SkillId::firesword:
    case SkillId::mootebo:
    case SkillId::killundead:
    case SkillId::snowwind:
    case SkillId::original_37:
    case SkillId::original_38:
    case SkillId::original_39:
    case SkillId::original_40:
    case SkillId::original_41:
    case SkillId::original_53:
    case SkillId::original_64:
    case SkillId::original_72:
    case SkillId::original_73:
    case SkillId::original_74:
    case SkillId::original_103:
    case SkillId::original_107:
    case SkillId::original_110:
    case SkillId::original_111:
    case SkillId::original_120:
    case SkillId::original_123:
    case SkillId::original_150:
        return true;
    default:return false;
    }
}

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
