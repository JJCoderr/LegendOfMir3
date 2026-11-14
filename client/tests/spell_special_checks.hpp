#pragma once
#include "magic/staged_spell.hpp"
#include <stdexcept>
#include "magic/spell_routes.hpp"
#include "hud/human_actions.hpp"
// 44DF59/44DACC/44DAE1：74实际帧段、严格间隔、回卷先于一秒切换、爆炸累计保留。
inline void spell_special_checks() {
    using mir3::StagedSpell;
    const auto require=[](bool ok,const char* message) { if(!ok) throw std::runtime_error(message); };
    // 414220/416E70：104首帧40ms边界，以及原版的三个加长起手限制。
    const auto& route=mir3::SpellRoutes::get(104);
    require(route.action_interval==40,"104初始间隔40ms");
    mir3::HumanActionClock clock;
    clock.start_spell(int(route.action),route.action_limit);clock.set_interval(route.action_interval);
    clock.advance(40);require(clock.frame()==0,"104等于间隔不推进");
    clock.advance(1);require(clock.frame()==1,"104超过间隔推进首帧");
    require(mir3::SpellRoutes::get(11).action_limit==13 && mir3::SpellRoutes::get(28).action_limit==20 &&
        mir3::SpellRoutes::get(78).action_limit==14,"11/28/78独立长起手限制");
    StagedSpell spell(mir3::SkillId::original_74,2,{1980,1995,80});
    require(spell.offset==std::array{100,100},"74初始化临时偏移");
    spell.advance(80);
    require(spell.frame==1980 && spell.offset==std::array{0,10},"74不超间隔不进帧，投影先归目标下10像素");
    spell.advance(1);
    require(spell.frame==1981 && spell.stage==StagedSpell::Stage::moving,"74以固定1050门槛进入阶段，不等MAGIC末帧");
    spell.advance(999);
    require(spell.stage==StagedSpell::Stage::moving && spell.offset==std::array{0,10},"74不套用73摆动轨迹");
    require(spell.advance(1)==StagedSpell::Change::load_explosion,"74一秒切换爆炸");
    spell.load_explosion({2000,2010,100});
    spell.advance(99);
    require(spell.frame==2000,"切换保留累计且严格大于间隔");
    spell.advance(1);
    require(spell.frame==2001,"爆炸承接累计");
    for(int i=0;i<8;++i) spell.advance(101);
    require(spell.advance(101)==StagedSpell::Change::finished,"爆炸2010终止");
    StagedSpell wrap(mir3::SkillId::original_74,0,{1980,1995,80});
    wrap.advance(81);wrap.frame=1994;
    require(wrap.advance(1000)==StagedSpell::Change::none && wrap.frame==1980 && wrap.stage==StagedSpell::Stage::moving,"回卷优先，不在同次推进转爆炸");
    require(wrap.advance(1)==StagedSpell::Change::load_explosion,"回卷下一次进入爆炸");
}
