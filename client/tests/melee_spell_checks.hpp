#pragma once
#include "magic/melee_spells.hpp"
#include <stdexcept>
// 原版近战状态：开关互斥、优先级、严格MP门槛、蓄力只消费一次。
inline void melee_spell_checks() {
    using namespace mir3;
    const auto require=[](bool ok,const char* text){if(!ok)throw std::runtime_error(text);};
    MeleeSpells s;
    require(!s.receive("FIR") && !s.receive("+UNKNOWN"),"只接收已知加号状态消息");
    s.receive("+FIR/123");s.receive("+BKS");s.receive("+RND");s.receive("+PWR");
    require(s.select(5,2,7,false,0).skill==int(SkillId::yedo) && s.fire,"MP等于门槛时不消费烈火，退回攻杀");
    require(s.select(5,2,100,false,0).skill==int(SkillId::firesword) && !s.fire,"烈火先于其他蓄力");
    require(s.select(5,2,100,false,0).action==16 && !s.back,"BKS动作16消费一次");
    require(s.select(5,2,100,false,0).action==17 && !s.round,"RND动作17消费一次");
    require(s.select(5,2,100,false,0).skill==0,"已消费蓄力不重复释放");
    s.receive("+WID");s.receive("+MAN");require(!s.wide && s.wide2,"十方斩关闭半月");
    auto attack=s.select(5,2,8,false,0);
    require(attack.skill==int(SkillId::original_103) && attack.facing==0 && attack.action==12,"十方斩固定特效方向并使用横斩动作");
    s.receive("+WID");require(s.wide && !s.wide2,"半月关闭十方斩");
    require(s.select(5,2,3,false,0).skill==0 && s.select(5,2,4,false,0).skill==int(SkillId::banwol),"半月MP严格大于3");
    s.receive("+LNG");s.receive("+PWR");
    require(s.select(5,2,100,true,0).skill==int(SkillId::ergum) && s.power,"隔格目标触发刺杀并保留攻杀蓄力");
    require(s.select(5,2,100,false,1).sound==131,"无隔格目标退回女性攻杀声音");
    for(auto text:{"+UFIR","+UBKS","+URND","+ULNG","+UWID","+UMAN"})s.receive(text);
    require(s.select(5,2,100,true,0).skill==0,"关闭状态恢复普通挥击");
}
