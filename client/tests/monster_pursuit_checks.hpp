#pragma once
#include "world/monster_pursuit.hpp"
#include "world/monster_prototypes.hpp"
#include "world/plant_ai.hpp"
#include "world/Monster.hpp"
#include <stdexcept>
#include <vector>

// 测试：列表顺序选敌、八方向近战、同格排除、独立攻击冷却、阻挡绕行和目标失效。
inline void monster_pursuit_checks() {
    using mir3::MonsterPursuit;
    using mir3::MonsterAiTarget;
    using Action=MonsterPursuit::Action;
    const auto require=[](bool ok,const char* message) {if(!ok) throw std::runtime_error(message);};
    // 怪物实际生命对象：整数护盾、破甲增伤与近战通知仍用护甲后原伤害。
    {
        mir3::MonsterVitals health;
        health.hp=health.maximum=100;health.ac=0x0303;
        std::vector<unsigned> bounds;
        const auto random=[&](unsigned n){bounds.push_back(n);return 0u;};
        health.bubble=true;health.bubble_level=1;health.bubble_time=4;
        require(health.mitigate(102,random)==0 && health.bubble_time==1,"bubble uses integer division and consumes duration on positive pre-bubble damage");
        require(health.mitigate(103,random)==24 && health.bubble_time==1,"bubble threshold one hundred and duration floor one");
        health.bubble_time=8;
        require(health.mitigate(3,random)==0 && health.bubble_time==8,"fully armored hit does not consume bubble");
        health.bubble=false;health.damage_armor=true;bounds.clear();
        require(health.struck_damage(7,random)==8 && health.hp==92 && bounds==std::vector<unsigned>{10},"armor poison rounds damage and consumes durability roll without equipment");
        bounds.clear();require(health.struck_damage(0,random)==0 && bounds.empty() && health.hp==92,"nonpositive struck damage does not heal or draw random");
        health.damage_health(-20);require(health.hp==100,"negative direct health damage heals up to maximum");
        health.damage_health(100);require(health.hp==0,"exact lethal damage clamps at zero");
        health.damage_health(-5);require(health.hp==5,"health function itself does not prohibit healing zero hp");
        health.hp=20;health.damage_armor=true;health.ac=0;bounds.clear();
        using M=mir3::MonsterMeleeAttack;
        M::Hooks hooks;
        hooks.mitigate=[&](int damage){return health.mitigate(damage,random);};
        hooks.wear_weapon=[](int){};
        hooks.damage=[&](int damage){health.struck_damage(damage,random);};
        int delayed=0,immediate=0;
        hooks.delayed_struck=[&](int damage,unsigned delay){require(health.hp==8 && damage==10 && delay==550,"notification observes post-poison hp but original damage");++delayed;};
        hooks.immediate_struck=[&](int damage){require(damage==10,"immediate message original damage");++immediate;};
        require(M::resolve(0x0a0a,0,0,{true,false,0,0},random,hooks)==M::Outcome::damaged &&
            bounds==std::vector<unsigned>({1,1,5,10}) && delayed==1 && immediate==1,
            "melee and vitals integrated random and notification ordering");
    }
    // 普通近战随机顺序与副作用：相等命中、全护甲仍损耗、非人物追加即时受击。
    {
        using M=mir3::MonsterMeleeAttack;
        std::vector<unsigned> bounds;
        std::string events;
        int hp=100;
        const auto random=[&](unsigned n){bounds.push_back(n);return n-1;};
        M::Hooks hooks;
        hooks.mitigate=[&](int n){events+='A';return M::armor_damage(n,0x0302,random);};
        hooks.wear_weapon=[&](int n){require(n==6,"melee weapon wear uses third roll");events+='W';};
        hooks.damage=[&](int n){hp-=n;events+='D';};
        hooks.delayed_struck=[&](int n,unsigned delay){require(n==7 && delay==550 && hp==93,"melee notification follows health change");events+='L';};
        hooks.immediate_struck=[&](int n){require(n==7,"immediate struck amount");events+='I';};
        require(M::resolve(0x0a08,9,0,{true,false,1,10},random,hooks)==M::Outcome::damaged &&
            bounds==std::vector<unsigned>({3,10,2,5}) && events=="AWDLI","ordinary melee exact hit boundary and ordering");
        bounds.clear();events.clear();hp=100;
        require(M::resolve(0x0a08,8,0,{true,false,1,10},random,hooks)==M::Outcome::missed &&
            bounds==std::vector<unsigned>({3,10}) && events.empty(),"miss skips armor and equipment rolls");
        bounds.clear();
        require(M::resolve(0x0a08,8,0,{false,false,1,0},random,hooks)==M::Outcome::missed &&
            bounds==std::vector<unsigned>{3},"improper target bypasses undefined speed");
        bounds.clear();
        require(M::resolve(0x0a08,8,0,{true,false,1,0},random,hooks)==M::Outcome::undefined_speed &&
            bounds==std::vector<unsigned>{3},"zero speed is explicitly unresolved");
        bounds.clear();events.clear();
        hooks.mitigate=[&](int n){events+='A';return M::armor_damage(n,0x1414,random);};
        require(M::resolve(0x0a08,0,0,{true,false,0,0},random,hooks)==M::Outcome::missed &&
            bounds==std::vector<unsigned>({3,1,5}) && events=="AW","zero target hit point skips dodge but full armor still wears weapon");
        bounds.clear();events.clear();hp=100;
        hooks.mitigate=[](int){return 7;};
        require(M::resolve(0x080a,0,10,{true,true,0,0},random,hooks)==M::Outcome::damaged &&
            bounds==std::vector<unsigned>({1,5}) && events=="DL","inverted DC clamps range and human receives no immediate struck");
        bounds.clear();
        require(M::armor_damage(20,0x0205,random)==15 && bounds.empty(),"inverted armor uses low bound without random");
    }
    require(mir3::reference_monster_petrified(0x04000000u) && mir3::reference_monster_petrified(0xffffffffu) &&
        !mir3::reference_monster_petrified(0x08000001u),"reference petrification bit is distinct from other status and statue bits");
    // 受击不重设攻击时钟，仅按等级加值；不合格攻击者不会替换原目标。
    {
        mir3::MonsterRunState s;
        mir3::MonsterRunState::Target hitter{4,5};
        s.hit_at=1000;
        require(s.struck(1100,0,&hitter) && s.hit_at==1150 && s.focused_at==1100 && s.target->x==4,"level zero struck selects and adds 150");
        hitter.proper=false;
        require(!s.struck(1200,10,&hitter) && s.hit_at==1260 && s.focused_at==1100,"improper hitter preserves focus but still delays hit");
        require(!s.struck(1300,33,nullptr) && s.hit_at==1280,"level 33 reaches minimum 20 delay without hitter");
        s.struck(1400,49,nullptr);require(s.hit_at==1300,"level 49 still delays by 20");
        s.struck(1500,50,nullptr);require(s.hit_at==1300,"level 50 no longer delays");
        s.hit_at=0xfffffff0u;s.struck(1600,0,nullptr);
        require(s.hit_at==134,"struck timestamp preserves DWORD wrap");
    }
    // 参考移动：重选只看首项，绕行以实际坐标变化停止，游走保持随机调用顺序。
    {
        mir3::MonsterRunState s;
        s.target_x=8;s.target_y=0;
        std::vector<int> directions;
        int selected=0,draws=0;
        const auto select=[&](const MonsterAiTarget& t){selected=t.id;};
        const auto random=[&](unsigned n){require(n==3,"path retry random bound");++draws;return 1u;};
        const auto blocked=[&](int dir){directions.push_back(dir);};
        MonsterAiTarget head{2,0,true,true,false,77,true};
        s.go_to_target(100,&head,select,random,blocked);
        require(selected==77 && s.target_x==2 && directions.empty() && s.found_path_at==0,
            "closer hidden head is selected without moving or path timestamp");
        head.inspector=true;s.target_x=8;
        s.go_to_target(101,&head,select,random,blocked);
        require(directions==std::vector<int>({2,3,4,5,6,7,0,1}) && draws==7 && s.found_path_at==101,
            "ineligible head does not stop eight blocked walk attempts");
        directions.clear();draws=0;
        s.target_x=0;s.target_y=-8;
        s.go_to_target(102,nullptr,select,[](unsigned){return 0u;},[&](int dir){
            directions.push_back(dir);if(directions.size()==2) --s.y;
        });
        require(directions==std::vector<int>({0,7}) && s.y==-1,"counterclockwise wraps and any coordinate change stops retry");
        directions.clear();s.x=s.target_x;s.y=s.target_y;
        s.go_to_target(103,nullptr,select,random,blocked);
        require(directions.empty() && draws==0 && s.found_path_at==102,"arrived target does not consume randomness");
        s.x=0;s.y=0;s.direction=6;
        int safe_calls=0,moves=0;
        const auto safe=[&](int x,int y){++safe_calls;require(x==1 && y==0,"safe cell coordinate");return false;};
        const auto move=[&](int x,int y,int d){++moves;s.x=x;s.y=y;s.direction=d;return true;};
        s.holy_seize=true;
        require(!s.walk_to(2,safe,move) && safe_calls==0 && moves==0,"holy seize stops walk before queries");
        s.holy_seize=false;s.escape=true;
        require(!s.walk_to(2,safe,move) && safe_calls==1 && moves==0,"escape requires safe destination");
        s.escape=false;
        require(s.walk_to(2,safe,move) && safe_calls==1 && moves==1 && s.x==1,"ordinary walk delegates actual position update");
        std::vector<unsigned> bounds;
        int turned=-1,walked=-1;
        const auto turn=[&](int d){turned=d;};const auto walk=[&](int d){walked=d;};
        s.wander([&](unsigned n){bounds.push_back(n);return 1u;},turn,walk);
        require(bounds==std::vector<unsigned>{20} && turned==-1 && walked==-1,"wander first gate suppresses other random calls");
        bounds.clear();
        s.wander([&](unsigned n){bounds.push_back(n);return n==20?0u:n==4?1u:7u;},turn,walk);
        require(bounds==std::vector<unsigned>({20,4,8}) && turned==7 && walked==-1,"wander turn consumes direction only on turn branch");
        bounds.clear();
        s.wander([&](unsigned n){bounds.push_back(n);return 0u;},turn,walk);
        require(bounds==std::vector<unsigned>({20,4}) && walked==s.direction,"wander walks existing direction");
        // 动物任务调度通过真实追踪/WalkTo更新位置，避免只测试回调请求。
        mir3::AnimalMonster animal;
        auto& a=animal.state;
        a.has_mission=true;a.mission_x=3;a.mission_y=0;a.walk_interval=600;
        unsigned inherited=0;
        const auto actual_walk=[&](int d){a.walk_to(d,[](int,int){return true;},[&](int x,int y,int dir){
            a.x=x;a.y=y;a.direction=dir;return true;
        });};
        mir3::MonsterRunState::Hooks hooks;
        hooks.think=[]{return false;};hooks.inherited=[&]{++inherited;};
        std::uint32_t now=601;
        hooks.go_to_target=[&]{a.go_to_target(now,nullptr,select,random,actual_walk);};
        animal.run(now,hooks);
        now=1202;animal.run(now,hooks);
        now=1803;animal.run(now,hooks);
        now=2404;animal.run(now,hooks);
        require(a.x==3 && a.y==0 && a.direction==2 && inherited==4 && a.found_path_at==1803,
            "animal mission reaches destination through actual movement and then stays");
    }
    // 参考Think的重叠移动与三秒目标检查，以及真实近战方法和动物调度的组合。
    {
        mir3::AnimalMonster animal;
        auto& s=animal.state;
        int counts=0,walks=0;
        const auto count=[&]{++counts;return 2;};
        const auto random=[](unsigned n){if(n!=8) throw std::runtime_error("duplicate direction bound");return 4u;};
        const auto vertical=[&](int d){require(d==4,"duplicate walk direction");++walks;++s.y;};
        s.target=mir3::MonsterRunState::Target{1,0,false,false,true,false};
        require(!s.think(3000,count,random,vertical) && counts==0 && s.target,"strict think boundary");
        require(!s.think(3001,count,random,vertical) && counts==1 && walks==1 && !s.target && s.duplicate,
            "vertical duplicate escape retains reference duplicate flag and rejects target");
        require(s.think(3002,count,random,[&](int){++s.x;}) && counts==1 && !s.duplicate,
            "duplicate retry does not wait three seconds and horizontal change clears flag");
        std::string events;
        const auto hit=[&]{events+='D';};const auto announce=[&]{events+='A';};const auto release=[&]{events+='H';};
        s.x=0;s.y=0;s.hit_interval=1200;
        s.target=mir3::MonsterRunState::Target{1,-1};
        require(s.attack_target(1200,hit,announce,release) && events.empty() && s.target_direction==1 && s.direction==4,
            "cooldown attack sets target direction but preserves facing and suppresses movement");
        require(s.attack_target(1201,hit,announce,release) && events=="DAH" && s.direction==1 && s.focused_at==1201,
            "melee damage precedes action and holy release with focus refresh");
        events.clear();s.target=mir3::MonsterRunState::Target{0,0};
        require(!s.attack_target(3000,hit,announce,release) && events.empty() && s.target_x==0 && s.target_y==0,
            "same cell only updates movement destination");
        s.target=mir3::MonsterRunState::Target{5,6,false,false,false};
        require(!s.attack_target(3001,hit,announce,release) && !s.target && s.target_x==-1 && s.target_y==-1,
            "out of range other map clears target and both coordinates");
        // 原代码先判邻格，未先判地图；保持此顺序，待EXE核对。
        s.target=mir3::MonsterRunState::Target{1,0,false,false,false};
        require(s.attack_target(3002,hit,announce,release) && events=="DAH","adjacent range precedes map check");
        events.clear();s.target=mir3::MonsterRunState::Target{1,0};
        s.walk_interval=600;s.walked_at=0;s.thought_at=3002;
        std::uint32_t now=3602;
        mir3::MonsterRunState::Hooks hooks;
        hooks.break_holy=release;
        hooks.think=[&]{events+='T';return s.think(now,[]{return 1;},random,vertical);};
        hooks.attack=[&]{return s.attack_target(now,hit,announce,release);};
        hooks.go_to_target=[&]{events+='G';};hooks.wander=[&]{events+='W';};hooks.inherited=[&]{events+='I';};
        animal.run(now,hooks);
        require(events=="TI","integrated animal does not move toward adjacent target during hit cooldown");
        events.clear();now=4203;animal.run(now,hooks);
        require(events=="TDAHI" && s.hit_at==now && s.walked_at==now,"integrated animal retaliates with reference ordering");
    }
    // 动物只运行基类：先检查目标/禁锢/Think，再行动，所有出口都执行继承调度。
    {
        mir3::AnimalMonster animal;
        auto& s=animal.state;
        require(!animal.meat_quality && !animal.body_leathery && !animal.skeleton,"animal reference fields remain unknown");
        std::string order;
        bool think_result=false,attack_result=false,occupied=true;
        mir3::MonsterRunState::Hooks h;
        h.break_holy=[&]{order+='H';s.holy_seize=false;};
        h.think=[&]{order+='T';return think_result;};
        h.attack=[&]{order+='A';return attack_result;};
        h.master_back=[&]{order+='B';return std::pair{1,50};};
        h.occupied=[&](int x,int y){require(x==1 && y==50,"master back query coordinate");order+='O';return occupied;};
        h.teleport_to_master_map=[&](int x,int y){require(x==s.target_x && y==s.target_y,"teleport uses current destination");order+='P';};
        h.go_to_target=[&]{order+='G';};h.wander=[&]{order+='W';};h.inherited=[&]{order+='I';};
        s.walk_interval=600;
        animal.run(600,h);
        require(order=="TI" && s.walked_at==0,"strict walk boundary still thinks and inherits");
        order.clear();animal.run(601,h);
        require(order=="TWI" && !s.target,"animal wanders without proactive target search");
        order.clear();s.holy_seize=true;s.holy_duration=700;think_result=true;
        animal.run(701,h);
        require(order=="HTI" && s.walked_at==601,"think early exit precedes walk and attack");
        order.clear();think_result=false;s.target=mir3::MonsterRunState::Target{15,15};s.focused_at=1;
        s.walked_at=30001;s.walk_interval=600;
        animal.run(30001,h);
        require(s.target.has_value(),"target survives exact thirty seconds and fifteen cells");
        order.clear();animal.run(30002,h);
        require(!s.target && order=="TI","target expires even when walk is not ready");
        s.target=mir3::MonsterRunState::Target{16,0};s.focused_at=30002;
        animal.run(30003,h);require(!s.target,"either target axis over fifteen expires");
        order.clear();s.target=mir3::MonsterRunState::Target{1,0};attack_result=true;s.walked_at=0;
        animal.run(30004,h);
        require(order=="TAI" && s.walked_at==30004,"handled attack skips movement but inherits");
        order.clear();s.target.reset();s.master=mir3::MonsterRunState::Master{30,0};s.walked_at=0;
        animal.run(30005,h);
        require(order=="TBOPGI" && s.target_x==0 && s.target_y==0,"master back obstruction uses OR axes then teleports and moves");
        order.clear();s.master.reset();s.has_mission=true;s.mission_x=7;s.mission_y=8;s.walked_at=0;
        animal.run(30006,h);
        require(order=="TGI" && s.target_x==7 && s.target_y==8,"mission replaces absent target destination");
        order.clear();s.run_away=true;s.runaway_started=30000;s.runaway_duration=10;s.walked_at=0;
        animal.run(30010,h);require(s.run_away,"runaway exact duration retained");
        order.clear();s.walked_at=0;animal.run(30011,h);
        require(!s.run_away && s.runaway_duration==0 && order=="TGI","runaway expiry still moves along old destination");
        for (bool* gate:{&s.dead,&s.hidden,&s.ghost,&s.stone,&s.petrified}) {
            *gate=true;order.clear();s.target=mir3::MonsterRunState::Target{50,50};
            animal.run(60000,h);
            require(order=="I" && s.target.has_value(),"inactive gate skips target cleanup and actions but inherits");
            *gate=false;
        }
    }
    // 诺玛只屏蔽死亡；沃玛额外屏蔽幽灵。专用逻辑结束始终调用基类。
    {
        mir3::MinorNumaMonster numa;
        mir3::OmaMonster oma;
        int searches=0,bases=0;
        const auto select=[&]{++searches;return std::optional<int>{81};};
        const auto base=[&]{++bases;};
        numa.run(1000,select,base);oma.run(1000,select,base);
        require(searches==0 && bases==2,"numa oma strict initial search");
        numa.run(1001,select,base);oma.run(1001,select,base);
        require(searches==2 && numa.target==81 && oma.target==81,"numa oma find target");
        numa.run(9001,select,base);oma.run(9001,select,base);
        require(searches==2,"numa oma strict eight seconds");
        oma.ghost=true;
        numa.run(9002,select,base);oma.run(9002,select,base);
        require(searches==3 && bases==8,"oma ghost suppresses search but not base");
        numa.dead=true;oma.ghost=false;oma.dead=true;
        numa.run(20000,select,base);oma.run(20000,select,base);
        require(searches==3 && bases==10,"dead monsters still call base");
        numa.dead=false;
        numa.run(20001,[]{return std::optional<int>{};},base);
        require(numa.target==81,"no replacement retains previous numa target");
    }
    // 喷吐按行遍历，南/东边界只保留第一格，不能改成完整两格线段。
    {
        mir3::SpitSpiderMonster spider;
        const std::vector<std::vector<std::pair<int,int>>> expected={
            {{0,-2},{0,-1}},{{1,-1}},{{1,0}},{{1,1}},{{0,1}},{{-1,1}},{{-2,0},{-1,0}},{{-2,-2},{-1,-1}}};
        for(int dir=0;dir<8;++dir) {
            std::vector<std::pair<int,int>> cells;
            std::vector<int> damage;
            bool announced=false;
            spider.spit(dir,20,30,0x0a0a,10,[](unsigned){return 0u;},[&]{announced=true;},
                [&](int x,int y)->std::optional<mir3::SpitSpiderMonster::Victim> {
                    require(announced,"spit announcement precedes queries");cells.emplace_back(x-20,y-30);
                    return mir3::SpitSpiderMonster::Victim{int(cells.size()),false,true,10};
                },[](int value){return value-2;},[&](int,int value,unsigned delay){
                    require(delay==300,"spit struck message delay");damage.push_back(value);
                });
            require(cells==expected[dir] && spider.direction==dir,"spit exact reference scan order");
            require(damage.front()==8 && (damage.size()==1 || damage.back()==6),"spit damage is reduced cumulatively");
        }
        int draws=0,queries=0,hits=0,announces=0;
        const auto random=[&](unsigned bound){++draws;return bound-1;};
        const auto announce=[&]{++announces;};
        const auto apply=[&](int,int,unsigned){++hits;};
        const auto identity=[](int value){return value;};
        spider.spit(0,0,0,0,1,random,announce,[&](int,int)->std::optional<mir3::SpitSpiderMonster::Victim>{++queries;return {};},identity,apply);
        require(draws==1 && announces==0 && queries==0,"zero spit damage emits no action");
        spider.spit(0,0,0,0x0a0a,1,random,announce,[&](int,int)->std::optional<mir3::SpitSpiderMonster::Victim>{
            ++queries;return mir3::SpitSpiderMonster::Victim{1,false,true,queries==1?0u:10u};
        },identity,apply);
        require(draws==3 && hits==0 && queries==2 && announces==1,"zero speed skips roll and high dodge roll misses");
    }
    // 召唤物首次出现、双形态切换顺序及克隆失败；remove 回调只能排队移除。
    {
        using E=mir3::SummonEvent;
        std::vector<E> events;
        int bases=0,clones=0;
        const auto notify=[&](E e){events.push_back(e);};
        const auto base=[&]{++bases;};
        mir3::WhiteSkeletonMonster skeleton;
        skeleton.run(0xffffff00u,notify,base);
        require(!skeleton.first && !skeleton.hidden && skeleton.direction==5 && skeleton.walked_at==1744,
            "skeleton first appearance uses direction five and DWORD wrap");
        skeleton.run(10,notify,base);
        require(events==std::vector<E>{E::emerge} && bases==2,"skeleton appears only once and always runs base");
        events.clear();bases=0;
        mir3::ElfMonster elf;
        mir3::ElfWarriorMonster warrior;
        elf.master_has_target=true;
        elf.run(100,[&]{++clones;return &warrior;},notify,base);
        require(events==std::vector<E>({E::emerge,E::emerge,E::release_master,E::remove}) && bases==0 && clones==1,
            "elf master target clones warrior and skips base after removal");
        require(!warrior.first && !warrior.hidden && warrior.walked_at==900 && elf.no_attack,
            "warrior clone appearance uses eight hundred millisecond timestamp");
        events.clear();
        mir3::ElfMonster restored;
        warrior.run(200,[&]{return &restored;},notify,base);
        require(events==std::vector<E>({E::retreat,E::change_face,E::release_master,E::remove}) && bases==1,
            "warrior reverts with retreat and identity change then still runs base");
        require(!restored.first && !restored.hidden && restored.walked_at==1000,"restored elf appears silently");
        events.clear();bases=0;
        restored.has_target=true;
        restored.run(300,[]{return static_cast<mir3::ElfWarriorMonster*>(nullptr);},notify,base);
        require(events==std::vector<E>({E::release_master,E::remove}) && bases==0,"failed elf clone still removes original");
        events.clear();
        warrior.run(300,[]{return static_cast<mir3::ElfMonster*>(nullptr);},notify,base);
        require(events==std::vector<E>({E::release_master,E::remove}) && bases==1,"failed warrior clone removes without face messages then calls base");
        events.clear();bases=0;
        mir3::ElfMonster dying;
        dying.dead=true;dying.death_at=100;
        const auto forbidden=[&]() -> mir3::ElfWarriorMonster* {throw std::runtime_error("dead elf must not clone");};
        dying.run(2100,forbidden,notify,base);
        require(events==std::vector<E>{E::emerge} && bases==1,"dead first elf still appears and strict death boundary holds");
        dying.run(2101,forbidden,notify,base);
        require(events.back()==E::make_ghost && bases==2,"elf ghosts strictly after two seconds and calls base");
        events.clear();bases=0;
        warrior.master_has_target=true;
        warrior.run(400,[]{return static_cast<mir3::ElfMonster*>(nullptr);},notify,base);
        require(events.empty() && bases==1,"master target keeps warrior form");
    }
    // Ron触发范围不等于实际伤害范围；查询和伤害回调保持参考调用顺序。
    {
        mir3::RonMonster ron(0,1200);
        int searches=0,queries=0;
        auto select=[&] {++searches;return std::optional<MonsterAiTarget>{{5,5,true,true,false,61}};};
        std::vector<int> hits;
        const auto query=[&](int x,int y,int radius) {
            ++queries;require(x==0 && y==0 && radius==1,"ron queries its own one-cell area");
            return std::vector<int>{62,63};
        };
        const auto hit=[&](int id) {hits.push_back(id);};
        require(!ron.run(1000,0,0,select,query,hit) && searches==0,"ron strict initial search");
        require(!ron.run(1001,0,0,select,query,hit) && searches==1,"ron finds target before attack cooldown");
        require(!ron.run(1200,0,0,select,query,hit),"ron strict attack boundary");
        ron.target_direction=6;
        require(ron.run(1201,0,0,select,query,hit) && hits==std::vector<int>({62,63}) && ron.facing==6,"ron attacks query results in order using cached direction");
        ron.target->x=6;
        require(!ron.run(3000,0,0,select,query,hit) && queries==1,"ron axis distance six cannot trigger");
        ron.target->x=5;
        require(ron.run(3001,0,0,select,[](int,int,int) {return std::vector<int>{};},hit) && hits.size()==2,"ron empty area still emits attack animation");
        ron.petrified=true;
        require(!ron.run(10000,0,0,select,query,hit) && searches==1,"petrified ron does not search or attack");
        ron.petrified=false;ron.dead=true;
        require(!ron.run(10001,0,0,select,query,hit),"dead ron cannot attack");
        ron.dead=false;ron.ghost=true;
        require(!ron.run(10002,0,0,select,query,hit),"ghost ron cannot attack");
    }
    // 雷电僵尸只在行动时机且两格内抽随机数，提前转基类不得消耗放电冷却。
    {
        using Zombie=mir3::LightningZombieMonster;
        Zombie zombie(0,600,1200);
        zombie.target=MonsterAiTarget{2,2,true,true,false,71};
        const auto select=[] {return std::optional<MonsterAiTarget>{};};
        int draws=0;
        const auto random=[&](unsigned bound) {++draws;require(bound==3,"zombie random range three");return 1u;};
        auto result=zombie.run(600,0,0,select,random);
        require(!result.back_position && !result.lightning && draws==0,"zombie strict walk interval does not draw random");
        result=zombie.run(1201,0,0,select,random);
        require(result.early_base && !result.lightning && !result.back_position && draws==1,"zombie near random nonzero skips retreat and lightning");
        result=zombie.run(1201,0,0,select,[](unsigned) {return 0u;});
        require(result.back_position && result.lightning && result.direction==3 && !result.early_base,"zombie near zero requests back position then lightning without prior cooldown consumption");
        zombie.target->x=4;zombie.target->y=4;
        result=zombie.run(2401,0,0,select,random);
        require(result.back_position && !result.lightning && draws==1,"zombie four-cell retreat and exact attack cooldown");
        zombie.target->x=5;zombie.target->y=5;
        result=zombie.run(2402,0,0,select,random);
        require(!result.back_position && result.lightning,"zombie five-cell lightning without retreat");
        zombie.target->x=6;
        result=zombie.run(4000,0,0,select,random);
        require(!result.back_position && !result.lightning,"zombie six-cell boundary excludes lightning");
        zombie.target->x=1;zombie.target->y=0;zombie.walked_at=4000;
        result=zombie.run(4000,0,0,select,random);
        require(result.lightning && !result.back_position && draws==1,"zombie lightning does not require walk cooldown");
        zombie.petrified=true;
        result=zombie.run(9000,0,0,select,random);
        require(!result.lightning && !result.back_position && !result.early_base && draws==1,"petrified zombie suppresses dedicated actions");
        zombie.petrified=false;zombie.target->x=0;zombie.target->y=0;zombie.walked_at=9001;
        result=zombie.run(9001,0,0,select,random);
        require(result.lightning && result.direction==4,"reference lightning same-cell direction is down");
    }
    // 两种隐藏怪分别保留参考边界；不把 SandMob 缺失的显现调用当作 RockMan 行为。
    {
        using mir3::RockManMonster;
        using mir3::SandMobMonster;
        std::vector<MonsterAiTarget> visible{{9,8,true,true,false,51}};
        RockManMonster rock;
        int searches=0;
        const auto none=[&]() {++searches;return std::optional<int>{};};
        require(rock.run(0,0,0,visible,none)==RockManMonster::Action::idle && rock.hidden && rock.walked_at==1000,"rock nine cells stays hidden but updates walk time");
        visible[0].x=8;visible[0].hidden=true;
        require(rock.run(1,0,0,visible,none)==RockManMonster::Action::emerge && !rock.hidden && searches==0,"rock square eight boundary emerges without same-turn search");
        require(rock.run(1000,0,0,{},none)==RockManMonster::Action::idle,"rock strict initial search delay");
        require(rock.run(1001,0,0,{},none)==RockManMonster::Action::retreat && rock.hidden,"rock hides when search has no target");
        rock.dead=true;
        require(rock.run(2000,0,0,visible,none)==RockManMonster::Action::idle && rock.hidden,"dead rock cannot emerge");
        rock.dead=false;rock.ghost=true;
        require(rock.run(2000,0,0,visible,none)==RockManMonster::Action::idle,"ghost rock cannot emerge");
        rock.ghost=false;rock.hidden=false;rock.target_id=51;rock.searched_at=0;
        require(rock.run(8000,0,0,visible,none)==RockManMonster::Action::idle && searches==1,"rock existing target strict eight-second search");
        require(rock.run(8001,0,0,visible,none)==RockManMonster::Action::idle && rock.target_id==51 && searches==2,"rock empty selection does not discard existing target");
        SandMobMonster sand(0,100,200);
        require(!sand.run(100,0,0,6,100,visible,nullptr).retreat && sand.appeared_at==0,"sand strict walk interval");
        sand.run(101,0,0,5,100,visible,nullptr);
        require(sand.hidden && sand.appeared_at==0,"sand equal five-percent threshold does not refresh appearance");
        sand.run(202,0,0,6,100,visible,nullptr);
        require(sand.hidden && sand.appeared_at==202,"sand nearby enemy only updates time without emergence");
        sand.hidden=false;sand.appeared_at=0;
        auto result=sand.run(3000,0,0,4,100,{},nullptr);
        require(!result.retreat && result.attack_target,"sand strict three-second low-health timer");
        result=sand.run(3101,0,0,4,100,{},nullptr);
        require(result.retreat && result.search && !result.attack_target && !result.skip_base,"sand low-health retreat still checks search and reaches base");
        sand.hidden=false;visible[0].x=16;visible[0].y=15;
        result=sand.run(3202,0,0,5,100,visible,&visible[0]);
        require(!result.retreat && result.attack_target,"sand one far axis does not retreat");
        visible[0].y=16;
        result=sand.run(3303,0,0,5,100,visible,&visible[0]);
        require(result.retreat && result.skip_base && !result.search && !result.attack_target,"sand both far axes return before search and base");
        sand.hidden=false;
        require(!sand.run(3404,0,0,0,100,{},nullptr).retreat,"sand zero HP is not low-positive-health retreat");
        require(!sand.run(3505,0,0,1,19,{},nullptr).retreat,"sand integer threshold truncates below twenty max HP");
        sand.dead=true;
        result=sand.run(3606,0,0,4,100,visible,&visible[0]);
        require(!result.retreat && !result.search && !result.attack_target,"dead sand suppresses dedicated behavior");
        SandMobMonster wrapping(0xfffffff0u,100,200);
        wrapping.run(85,0,0,6,100,std::vector<MonsterAiTarget>{{8,8,true,true,false,52}},nullptr);
        require(wrapping.appeared_at==85,"sand elapsed arithmetic preserves DWORD rollover");
    }
    // 蜂后：严格行动/攻击间隔、500ms到期边界，以及先投递再运行的参考顺序。
    {
        mir3::BeeQueenMonster queen(0,100,200);
        const auto select=[] {return std::optional<int>{41};};
        const auto alive=[](int) {return true;};
        std::vector<std::optional<int>> inherited;
        const auto spawn=[&](std::optional<int> target) {inherited.push_back(target);return std::optional<int>{100+int(inherited.size())};};
        require(!queen.run(100,select,alive),"queen strict walk interval");
        require(!queen.run(200,select,alive),"queen strict attack interval");
        require(queen.run(301,select,alive),"queen schedules attack after both intervals");
        require(queen.operate(800,spawn)==0,"queen does not deliver one millisecond early");
        queen.target_id=42;
        require(queen.operate(801,spawn)==1 && inherited.back()==42,"queen inherits delivery-time target at exact deadline");
        require(queen.operate(1000,spawn)==0,"queen consumes each delayed message once");
        queen.slaves.assign(15,101);
        require(!queen.run(1000,select,[](int) {return false;}) && queen.slaves.empty(),"queen checks cap before reclaiming dead slaves");
        require(queen.run(1201,select,alive),"queen schedules after reclaimed capacity");
        queen.dead=true;
        require(!queen.run(1500,select,alive),"dead queen cannot schedule new children");
        queen.target_id.reset();
        require(queen.operate(1701,spawn)==1 && !inherited.back(),"already queued delivery survives death and accepts absent target");
        // 上限在排程时检查。短间隔允许多个待投递消息，不擅自增加投递期上限。
        mir3::BeeQueenMonster queued(0,0,0);
        queued.slaves.assign(14,1);
        require(queued.run(1,select,alive) && queued.run(2,select,alive),"queen can queue two births below scheduling cap");
        require(queued.operate(502,spawn)==2 && queued.slaves.size()==16,"queen delivery does not recheck scheduling cap");
        mir3::BeeQueenMonster failure(0,0,0);
        require(failure.run(1,select,alive),"queen schedules failed spawn fixture");
        require(failure.operate(501,[](auto) {return std::optional<int>{};})==0 && failure.slaves.empty(),"failed creation does not occupy capacity");
        require(failure.operate(600,spawn)==0,"failed delivery is consumed rather than retried");
        failure.petrified=true;
        require(!failure.run(700,select,alive),"petrified queen cannot schedule");
        failure.petrified=false;failure.ghost=true;
        require(!failure.run(800,select,alive),"ghost queen cannot schedule");
        mir3::BeeQueenMonster no_target(0,0,0);
        require(!no_target.run(1,[] {return std::optional<int>{};},alive),"queen without target cannot schedule");
    }
    // 食人花出生隐藏；四格含对角线，严格超过行动间隔才检查。
    {
        mir3::PlantAi plant;
        using PlantAction=mir3::PlantAi::Action;
        std::vector<MonsterAiTarget> visible{{4,4,true,true,false,1}};
        require(plant.hidden,"plant starts hidden");
        require(plant.tick(600,0,0,false,false,visible,nullptr)==PlantAction::idle,"plant strict walk interval");
        require(plant.tick(601,0,0,false,false,visible,nullptr)==PlantAction::emerge && !plant.hidden,"plant diagonal emerge boundary");
        auto target=visible.front();target.x=5;
        require(plant.tick(1202,0,0,false,false,visible,&target)==PlantAction::combat,"plant one far axis does not retreat");
        target.y=5;
        require(plant.tick(1803,0,0,false,false,visible,&target)==PlantAction::retreat && plant.hidden,"plant both far axes retreat");
        visible[0].x=5;
        require(plant.tick(2404,0,0,false,false,visible,nullptr)==PlantAction::idle,"plant cannot emerge outside square");
        visible[0].x=4;visible[0].inspector=true;
        require(plant.tick(3005,0,0,false,false,visible,nullptr)==PlantAction::idle,"plant excludes inspector");
        visible[0].inspector=false;visible[0].proper=false;
        require(plant.tick(3606,0,0,false,false,visible,nullptr)==PlantAction::idle,"plant excludes friendly target");
        visible[0].proper=true;visible[0].alive=false;
        require(plant.tick(4207,0,0,false,false,visible,nullptr)==PlantAction::idle,"plant excludes dead target");
        visible[0].alive=true;visible[0].hidden=true;
        require(plant.tick(4808,0,0,true,false,visible,nullptr)==PlantAction::idle,"dead plant stays hidden");
        require(plant.tick(4808,0,0,false,true,visible,nullptr)==PlantAction::idle,"ghost plant stays hidden");
        require(plant.tick(4808,0,0,false,false,visible,nullptr)==PlantAction::emerge,"plant reemerges and CheckComeOut permits hidden enemy");
        require(plant.tick(5409,0,0,false,false,{},nullptr)==PlantAction::combat && !plant.hidden,"target loss alone does not hide plant");
        // 固定位置近战：冷却结束按列表选敌，同格和远处均不攻击也不移动。
        int attacks=0;
        const auto attack=[&](int direction,const auto&) {++attacks;return direction==2;};
        visible={{3,0,true,true,false,2},{1,0,true,true,false,3}};
        require(!plant.fight(5500,0,0,visible,attack) && plant.target_id==2 && attacks==0,"plant keeps first eligible distant target");
        visible.erase(visible.begin());
        require(plant.fight(5500,0,0,visible,attack) && attacks==1 && plant.facing==2,"plant attacks adjacent target facing east");
        require(!plant.fight(6700,0,0,visible,attack) && attacks==1,"plant strict attack cooldown");
        require(plant.fight(6701,0,0,visible,attack) && attacks==2,"plant resumes attack after cooldown");
        visible[0].alive=false;
        require(!plant.fight(8000,0,0,visible,attack) && !plant.target_id,"plant releases dead target");
        visible[0].alive=true;visible[0].x=0;
        require(!plant.fight(8000,0,0,visible,attack),"plant cannot attack overlapping target");
        visible[0].x=1;plant.hidden=true;
        require(!plant.fight(8000,0,0,visible,attack) && attacks==2,"hidden plant cannot attack");
    }
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

    // 三种远程怪物在七格内停步，射完各自的连射次数后才进入随机恢复。
    using mir3::MonsterAppearance;
    const std::array profiles{
        std::pair{MonsterAppearance::skeleton_axe_thrower,1u},
        std::pair{MonsterAppearance::spined_dark_lizard,2u},
        std::pair{MonsterAppearance::zuma_archer,5u}};
    for (const auto [appearance,shots]:profiles) {
        MonsterPursuit ranged;
        ranged.burst_limit=mir3::reference_ranged_burst(appearance);
        require(ranged.burst_limit==shots,"shape selects reference burst size");
        std::vector<MonsterAiTarget> enemy{{8,7,true,true,false,82}};
        require(ranged.tick(1001,0,0,enemy,walk,hit,random)==Action::walk,"eight tiles away requires pursuit");
        MonsterPursuit::Time now=1002;
        for (unsigned shot=0;shot<shots;++shot,now+=1201) {
            require(ranged.tick(now,1,0,enemy,walk,hit,random)==Action::attack && hits.back()==3,
                "seven tiles on both axes is within range");
            require(ranged.tick(now+1200,1,0,enemy,walk,hit,random)==Action::idle,"ranged cooldown holds position");
        }
        require(ranged.tick(now,1,0,enemy,walk,hit,random)==Action::idle && ranged.burst_shots==shots,
            "failed recovery roll keeps burst exhausted");
        now+=1201;
        const auto recover=[&](unsigned bound) {require(bound==5,"reference recovery odds");return 0u;};
        require(ranged.tick(now,1,0,enemy,walk,hit,recover)==Action::idle && ranged.burst_shots==0,
            "successful recovery resets burst without firing");
        require(ranged.tick(now+1200,1,0,enemy,walk,hit,random)==Action::idle &&
            ranged.tick(now+1201,1,0,enemy,walk,hit,random)==Action::attack,"firing resumes on following cooldown");
    }
}
