#pragma once
#include "world/monster_pursuit.hpp"
#include <stdexcept>
#include <vector>

// 测试：列表顺序选敌、八方向近战、同格排除、独立攻击冷却、阻挡绕行和目标失效。
inline void monster_pursuit_checks() {
    using mir3::MonsterPursuit;
    using mir3::MonsterAiTarget;
    using Action=MonsterPursuit::Action;
    const auto require=[](bool ok,const char* message) {if(!ok) throw std::runtime_error(message);};
    const auto random=[](unsigned) {return 1u;};
    std::vector<int> walks,hits;
    const auto walk=[&](int,int,int direction) {walks.push_back(direction);return true;};
    const auto hit=[&](int direction,const MonsterAiTarget&) {hits.push_back(direction);return true;};
    MonsterPursuit brain;
    std::vector<MonsterAiTarget> targets{{5,0,true,true,false,10},{1,0,true,true,false,11}};
    require(brain.tick(1000,0,0,targets,walk,hit,random)==Action::idle,"strict initial search delay");
    require(brain.tick(1001,0,0,targets,walk,hit,random)==Action::walk && brain.target_id==10 && walks.back()==2,
        "first visible target wins over nearer second target");
    for(int dir=0;dir<8;++dir) {
        MonsterPursuit melee;
        const auto delta=MonsterPursuit::directions[dir];
        std::vector<MonsterAiTarget> adjacent{{delta[0],delta[1],true,true,false,1}};
        require(melee.tick(1001,0,0,adjacent,walk,hit,random)==Action::attack && hits.back()==dir,"eight adjacent melee directions");
        const auto walk_count=walks.size(),hit_count=hits.size();
        require(melee.tick(2201,0,0,adjacent,walk,hit,random)==Action::idle && walks.size()==walk_count && hits.size()==hit_count,
            "cooldown boundary neither attacks nor walks into target");
        require(melee.tick(2202,0,0,adjacent,walk,hit,random)==Action::attack,"attack resumes after strict cooldown");
    }
    MonsterPursuit overlapping;
    std::vector<MonsterAiTarget> same{{0,0,true,true,false,20}};
    require(overlapping.tick(1001,0,0,same,walk,hit,random)==Action::idle,"same cell is not melee range");
    MonsterPursuit obstacle;
    std::vector<MonsterAiTarget> far{{4,0,true,true,false,21}};
    walks.clear();
    require(obstacle.tick(1001,0,0,far,[&](int,int,int direction) {walks.push_back(direction);return direction==6;},hit,random)==Action::walk &&
        walks==std::vector<int>({2,3,4,5,6}) && obstacle.facing==6,"failed movement rotates along reference fallback sequence");
    walks.clear();
    require(obstacle.tick(1602,0,0,far,[&](int,int,int direction) {walks.push_back(direction);return false;},hit,random)==Action::blocked &&
        walks.size()==8 && obstacle.facing==6,"eight failures leave logical facing unchanged");
    far[0].alive=false;
    require(obstacle.tick(1700,0,0,far,walk,hit,random)==Action::idle && !obstacle.target_id,"dead target released");
    MonsterPursuit hidden;
    far[0].alive=true;far[0].hidden=true;
    require(hidden.tick(1001,0,0,far,walk,hit,random)==Action::idle,"hidden target excluded from initial search");
    MonsterPursuit switcher;
    switcher.target_id=31;switcher.focused_at=1000;switcher.searched_at=1000;
    std::vector<MonsterAiTarget> nearer{{2,0,true,true,false,30},{6,0,true,true,false,31}};
    require(switcher.tick(1200,0,0,nearer,walk,hit,random)==Action::idle && switcher.target_id==30,"closer first visible object retargets without walking this turn");
}
