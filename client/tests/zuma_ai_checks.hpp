#pragma once
#include "world/zuma_ai.hpp"
#include "world/monster_prototypes.hpp"
#include <set>
#include <stdexcept>

// 测试：视野/敌我过滤、方形距离边界、不递归群体唤醒、冷却、召唤失败及名额回收。
inline void zuma_ai_checks() {
    using namespace mir3;
    const auto require=[](bool ok,const char* message) { if(!ok) throw std::runtime_error(message); };
    require(int(reference_zuma_prototype(ZumaSlavePrototype::archer)->appearance)==82 &&
        int(reference_zuma_prototype(ZumaSlavePrototype::fanatic)->appearance)==83 &&
        int(reference_zuma_prototype(ZumaSlavePrototype::guardian)->appearance)==84 &&
        int(reference_zuma_prototype(ZumaSlavePrototype::moth)->appearance)==75,
        "reference prototype ids resolve to distinct client appearances");
    require(!reference_zuma_prototype(static_cast<ZumaSlavePrototype>(999)),"unknown prototype cannot silently become an appearance");
    ZumaAi world;
    world.actors.emplace(1,ZumaAi::Actor{1,ZumaKind::statue,0,0});
    world.actors.emplace(2,ZumaAi::Actor{2,ZumaKind::statue,7,7});
    world.actors.emplace(3,ZumaAi::Actor{3,ZumaKind::statue,8,8});
    world.actors.emplace(4,ZumaAi::Actor{4,ZumaKind::king,1,1});
    std::vector<ZumaAi::Target> targets{{2,2,false,true,false},{2,2,true,false,false},{2,2,true,true,true},{3,0}};
    const auto visible=[&](int id) { return id==1?targets:std::vector<ZumaAi::Target>{}; };
    std::set<int> living;
    std::vector<ZumaSlavePrototype> requested;
    bool fail_spawn=false;
    int next_id=100;
    const auto spawn=[&](const ZumaAi::Actor&,ZumaSlavePrototype prototype)->std::optional<int> {
        requested.push_back(prototype);
        if(fail_spawn) return {};
        living.insert(next_id);
        return next_id++;
    };
    const auto alive=[&](int id) { return living.contains(id); };
    const auto random=[](unsigned bound) { return bound-1; };
    require(world.tick(1,visible,spawn,alive,random).woke.empty(),"ineligible and out-of-range targets must not wake");
    targets={{2,2}};
    auto result=world.tick(2,visible,spawn,alive,random);
    require(result.woke==std::vector<int>({1,2}),"inclusive square range, no chained wake and no king wake");
    require(world.actors.at(3).stone && world.actors.at(4).stone,"outside neighbors remain stone");
    require(world.actors.at(1).next_turn==2602,"trigger has two second pause plus normal walk gate");
    require(std::find(result.ready.begin(),result.ready.end(),2)!=result.ready.end(),"indirect guard has no extra trigger pause");
    result=world.tick(2602,visible,spawn,alive,random);
    require(std::find(result.ready.begin(),result.ready.end(),1)==result.ready.end(),"strict turn deadline");
    result=world.tick(2603,visible,spawn,alive,random);
    require(std::find(result.ready.begin(),result.ready.end(),1)!=result.ready.end(),"trigger becomes active after deadline");
    world.actors.clear();
    auto& king=world.actors.emplace(10,ZumaAi::Actor{10,ZumaKind::king}).first->second;
    king.stone=false;king.hp=99;
    const auto none=[](int) { return std::vector<ZumaAi::Target>{}; };
    world.tick(1,none,spawn,alive,random);
    require(king.slaves.size()==11 && king.danger==4,"reference integer division triggers below full health");
    require(std::all_of(requested.begin(),requested.end(),[](auto prototype) {return prototype==ZumaSlavePrototype::moth;}),"prototype selection retains database identity");
    world.tick(602,none,spawn,alive,random);
    world.tick(1203,none,spawn,alive,random);
    require(king.slaves.size()==30,"successful live summons capped at thirty");
    for(int id:king.slaves) living.erase(id);
    auto before=requested.size();
    world.tick(1804,none,spawn,alive,random);
    require(requested.size()==before && king.slaves.empty(),"cleanup follows capped summon attempt");
    fail_spawn=true;
    world.tick(2405,none,spawn,alive,random);
    require(king.slaves.empty() && king.danger==0 && requested.size()==before+11,"failed spawns consume attempt but not capacity");
    king.hp=100;
    world.tick(3006,none,spawn,alive,random);
    require(king.danger==5,"full health rearms danger levels");
    king.hp=1;king.petrified=true;
    before=requested.size();
    world.tick(4000,none,spawn,alive,random);
    require(requested.size()==before && king.danger==5,"petrified king performs no special AI");
}
