// 怪物系统：先完整移植参考工程，再按宝藏传奇3.exe及其实际加载模块的证据修正。
// Mir3.exe仅作辅助对照；参考差异不能自动覆盖宝藏版行为。
// 同一文件分别编译核心规则与 Godot 适配部分，模型测试不依赖引擎运行时。
#include "world/Monster.hpp"
#if !defined(MIR3_MONSTER_GODOT)
#include "hud/selection_pixels.hpp"
#include <cmath>
#include <stdexcept>
#include <type_traits>
#include <limits>
namespace mir3 {
namespace {
void monster145_word(std::vector<std::uint8_t>& data,std::size_t offset,std::uint16_t value) {
    data[offset]=std::uint8_t(value);data[offset+1]=std::uint8_t(value>>8);
}
void monster145_dword(std::vector<std::uint8_t>& data,std::size_t offset,std::uint32_t value) {
    for (unsigned i=0;i<4;++i) data[offset+i]=std::uint8_t(value>>(8*i));
}
void monster145_feature(Monster145Message& packet,Monster145Appearance appearance) {
    packet.body.resize(16);packet.body[0]=3;
    monster145_word(packet.body,2,appearance.shape);monster145_dword(packet.body,4,appearance.status);
}
}
std::string Monster145Message::encode() const {
    std::string result;
    const auto append=[&](const auto& bytes) {
        unsigned accumulator=0;int bits=0;
        for (std::uint8_t byte:bytes) {
            accumulator=(accumulator<<8)|byte;bits+=8;
            while (bits>=6) {bits-=6;result+=char(60+((accumulator>>bits)&63));}
        }
        if (bits) result+=char(60+((accumulator<<(6-bits))&63));
    };
    append(header.encode());append(body);return result;
}
Monster145Message Monster145Message::pose(protocol::MessageId opcode,std::uint32_t actor,int x,int y,int direction,Monster145Appearance appearance) {
    Monster145Message packet{{actor,opcode,{std::uint16_t(x),std::uint16_t(y),std::uint16_t(direction|(unsigned(appearance.light)<<8))}},{}};
    monster145_feature(packet,appearance);return packet;
}
Monster145Message Monster145Message::melee(std::uint32_t actor,int x,int y,int direction) {
    return {{actor,protocol::SM_HIT,{std::uint16_t(x),std::uint16_t(y),std::uint16_t(direction)}},{}};
}
Monster145Message Monster145Message::projectile(std::uint32_t actor,int x,int y,int direction,int tx,int ty,std::uint32_t target) {
    Monster145Message packet{{actor,protocol::SM_FLYAXE,{std::uint16_t(x),std::uint16_t(y),std::uint16_t(direction)}},std::vector<std::uint8_t>(8)};
    monster145_word(packet.body,0,std::uint16_t(tx));monster145_word(packet.body,2,std::uint16_t(ty));
    monster145_dword(packet.body,4,target);return packet;
}
Monster145Message Monster145Message::struck(std::uint32_t actor,Monster145Appearance appearance,std::uint16_t hp,
    std::uint16_t maximum_hp,std::uint16_t damage,std::uint16_t hit_variant) {
    Monster145Message packet{{actor,protocol::SM_STRUCK,{hp,maximum_hp,damage}},{}};
    monster145_feature(packet,appearance);monster145_word(packet.body,14,hit_variant);return packet;
}
Monster145Message Monster145Message::health(std::uint32_t actor,std::uint16_t hp,std::uint16_t mp,std::uint16_t maximum_hp) {
    return {{actor,protocol::SM_HEALTHSPELLCHANGED,{hp,mp,maximum_hp}},{}};
}
Monster145Message Monster145Message::change_appearance(std::uint32_t actor,std::uint16_t shape) {
    return {{actor,protocol::SM_FEATURECHANGED,{3,shape,0}},{}};
}
namespace {
int zircon_direction(int x,int y,int tx,int ty) {
    const int result=MonsterPursuit::direction(x,y,tx,ty);
    return result<0?0:result;
}
void zircon_walk_rotation(int facing,const zircon::CombatWorld& world) {
    const int rotation=world.random(2)==0?1:7;
    for (int attempt=0;attempt<8;++attempt) {
        if (world.walk(facing)) break;
        facing=(facing+rotation)%8;
    }
}
using ZMagic=zircon::CombatWorld::Magic;
template<class Monster>
void zircon_move_toward(Monster& monster,const zircon::CombatWorld& world) {
    if (monster.x==monster.target->x && monster.y==monster.target->y)
        zircon_walk_rotation(int(world.random(8)),world);
    else world.move_to(monster.target->x,monster.target->y);
}
template<class Monster>
void zircon_mage_target(Monster& monster,zircon::Time now,const zircon::CombatWorld& world) {
    if (!monster.target) return;
    if (!monster.in_attack_range()) {
        if (world.can_attack() && world.random(2)==0) monster.range_attack(now,world);
        zircon_move_toward(monster,world);
    }
    if (!world.can_attack()) return;
    if (world.random(5)>0) {if (monster.in_attack_range()) monster.attack(now,world);}
    else monster.range_attack(now,world);
}
template<class Monster>
void zircon_base_target(Monster& monster,zircon::Time now,const zircon::CombatWorld& world) {
    if (!monster.target) return;
    if (monster.fear) {
        world.walk(zircon_direction(monster.target->x,monster.target->y,monster.x,monster.y));return;
    }
    if (!monster.in_attack_range()) {zircon_move_toward(monster,world);return;}
    if (world.can_attack()) monster.attack(now,world);
}
template<class Monster>
void zircon_sama_target(Monster& monster,zircon::Time now,const zircon::CombatWorld& world) {
    if (!monster.target) return;
    if (!monster.in_attack_range()) {
        if (world.can_attack() && world.random(unsigned(monster.range_chance))==0) monster.range_attack(now,world);
        zircon_move_toward(monster,world);
    }
    if (!world.can_attack()) return;
    if (world.random(unsigned(monster.range_chance))>0) {if (monster.in_attack_range()) monster.attack(now,world);}
    else monster.range_attack(now,world);
}
bool zircon_teleport_to_target(int x,int y,const MonsterAiTarget& target,int& direction,const zircon::CombatWorld& world) {
    const int facing=zircon_direction(x,y,target.x,target.y);
    for (int i=0;i<8;++i) {
        const auto delta=MonsterPursuit::directions[(facing+i)%8];
        const int tx=target.x+delta[0],ty=target.y+delta[1];
        if (!world.teleport_cell_available(tx,ty)) continue;
        direction=zircon_direction(tx,ty,target.x,target.y);world.teleport(tx,ty);return true;
    }
    return false;
}
template<class Monster>
void zircon_monkey_target(Monster& monster,zircon::Time now,const zircon::CombatWorld& world) {
    if (!monster.target) return;
    if (monster.in_attack_range() && world.can_attack() && world.random(2)==0) monster.attack(now,world);
    zircon_move_toward(monster,world);
    if (monster.in_attack_range() && world.can_attack()) monster.attack(now,world);
}
template<class Monster>
void zircon_normal_attack(Monster& monster,zircon::Time now,const zircon::CombatWorld& world) {
    if (!monster.target) return;
    monster.direction=zircon_direction(monster.x,monster.y,monster.target->x,monster.target->y);
    world.broadcast(monster.direction,false,monster.target->id);monster.clock.update(now);
    world.enqueue({monster.target->id,world.get_dc(),now+400,monster.attack_element});
}
void zircon_single_magic(int x,int y,const MonsterAiTarget& target,int& direction,zircon::AttackClock& clock,
    zircon::Time now,ZMagic magic,zircon::Element element,bool travel,const zircon::CombatWorld& world) {
    direction=zircon_direction(x,y,target.x,target.y);
    auto cast=std::make_shared<zircon::CombatWorld::MagicCast>();
    cast->direction=direction;cast->magic=magic;cast->targets.push_back(target.id);
    world.broadcast_magic(cast);clock.update(now);
    const auto distance=std::max(std::abs(std::int64_t(target.x)-x),std::abs(std::int64_t(target.y)-y));
    world.enqueue({target.id,world.get_dc(),now+500+(travel?distance*48:0),element});
}
std::shared_ptr<zircon::CombatWorld::MagicCast> zircon_begin_magic(int x,int y,const MonsterAiTarget& target,
    int& direction,zircon::AttackClock& clock,zircon::Time now,ZMagic magic,const zircon::CombatWorld& world) {
    direction=zircon_direction(x,y,target.x,target.y);
    auto cast=std::make_shared<zircon::CombatWorld::MagicCast>();cast->direction=direction;cast->magic=magic;
    world.broadcast_magic(cast);clock.update(now);return cast;
}
void zircon_area_magic(int x,int y,const MonsterAiTarget& target,int& direction,zircon::AttackClock& clock,
    zircon::Time now,int radius,ZMagic magic,zircon::Element element,int damage,const zircon::CombatWorld& world) {
    direction=zircon_direction(x,y,target.x,target.y);
    auto cast=std::make_shared<zircon::CombatWorld::MagicCast>();cast->direction=direction;cast->magic=magic;
    cast->locations.emplace_back(target.x,target.y);world.broadcast_magic(cast);clock.update(now);
    for (int id:world.get_targets(target.x,target.y,radius))
        world.enqueue({id,damage==0?world.get_dc():damage,now+500,element});
}
void zircon_mass_magic(int x,int y,const MonsterAiTarget& target,int& direction,zircon::AttackClock& clock,
    zircon::Time now,ZMagic magic,const zircon::CombatWorld& world) {
    auto cast=zircon_begin_magic(x,y,target,direction,clock,now,magic,world);
    if (magic==ZMagic::lightning_ball) {
        for (int i=-20;i<20;i+=5) cast->locations.emplace_back(x-20,y-i);
        for (int i=-20;i<20;i+=5) cast->locations.emplace_back(x+20,y-i);
        for (int i=-20;i<20;i+=5) cast->locations.emplace_back(x+i,y-20);
        for (int i=-20;i<20;i+=5) cast->locations.emplace_back(x+i,y+20);
        for (int id:world.get_targets(x,y,world.maximum_view_range)) {
            cast->targets.push_back(id);
            const auto [tx,ty]=world.target_position(id);
            const auto distance=std::max(std::abs(std::int64_t(tx)-x),std::abs(std::int64_t(ty)-y));
            world.enqueue({id,world.get_dc(),now+500+distance*48,zircon::Element::lightning});
        }
        return;
    }
    const bool cyclone=magic==ZMagic::cyclone;
    for (const auto& cell:world.cells_in_range(x,y,world.maximum_view_range)) {
        if (!cell.objects) {
            if (world.random(cyclone?30:50)==0) cast->locations.emplace_back(cell.x,cell.y);
            continue;
        }
        for (int id:*cell.objects) {
            // 随机筛选先于CanAttackTarget，空列表不抽取空格特效随机数。
            const auto roll=world.random(cyclone?4:2);
            if ((cyclone?roll==0:roll>0) || !world.can_attack_target(id)) continue;
            cast->targets.push_back(id);
            world.enqueue({id,world.get_dc(),now+500,cyclone?zircon::Element::wind:zircon::Element::lightning});
        }
    }
}
void zircon_line_aoe(int x,int y,const MonsterAiTarget& target,int& direction,zircon::AttackClock& clock,
    zircon::Time now,int distance,int min,int max,ZMagic magic,zircon::Element element,const zircon::CombatWorld& world) {
    auto cast=zircon_begin_magic(x,y,target,direction,clock,now,magic,world);
    const bool endpoint=magic==ZMagic::lightning_beam || magic==ZMagic::blow_earth;
    for (int offset=min;offset<=max;++offset) {
        const int ray=((direction+offset)%8+8)%8;
        const auto delta=MonsterPursuit::directions[ray];
        if (endpoint) cast->locations.emplace_back(x+delta[0]*distance,y+delta[1]*distance);
        for (int step=1;step<=distance;++step) {
            const int cx=x+delta[0]*step,cy=y+delta[1]*step;
            const auto center=world.cell(cx,cy);
            if (!center) continue;
            if (!endpoint) cast->locations.emplace_back(cx,cy);
            const auto hit=[&](const zircon::CombatWorld::MagicCell& cell,bool half) {
                if (!cell.objects) return;
                for (int id:*cell.objects) if (world.can_attack_target(id)) {
                    const int damage=world.get_dc();
                    world.enqueue({id,half?damage/2:damage,now+500+step*75,element});
                }
            };
            hit(*center,false);
            const int shift=ray%2==0?2:1;
            for (int side:{-shift,shift}) {
                const auto lateral=MonsterPursuit::directions[(ray+side+8)%8];
                if (const auto cell=world.cell(cx+lateral[0],cy+lateral[1])) hit(*cell,true);
            }
        }
    }
}
}
zircon::SamaBlack::~SamaBlack()=default;
bool zircon::SamaBlack::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,2);
}
void zircon::SamaBlack::process_target(Time now,const CombatWorld& world) {zircon_sama_target(*this,now,world);}
void zircon::SamaBlack::attack(Time now,const CombatWorld& world) {zircon_normal_attack(*this,now,world);}
void zircon::SamaBlack::range_attack(Time now,const CombatWorld& world) {
    if (!target) return;
    switch (world.random(5)) {
    case 0:zircon_single_magic(x,y,*target,direction,clock,now,ZMagic::ice_blades,Element::ice,true,world);break;
    case 1:zircon_line_aoe(x,y,*target,direction,clock,now,10,-2,2,ZMagic::greater_frozen_earth,Element::ice,world);break;
    case 2:zircon_area_magic(x,y,*target,direction,clock,now,2,ZMagic::ice_storm,Element::ice,0,world);break;
    case 3:zircon_area_magic(x,y,*target,direction,clock,now,3,ZMagic::sama_guardian_ice,Element::ice,0,world);break;
    case 4:{
        const int damage=world.get_dc()*2;
        zircon_area_magic(x,y,*target,direction,clock,now,3,ZMagic::sama_black_ice,Element::ice,damage,world);break;
    }
    }
}
zircon::SamaBlue::~SamaBlue()=default;
bool zircon::SamaBlue::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,2);
}
void zircon::SamaBlue::process_target(Time now,const CombatWorld& world) {zircon_sama_target(*this,now,world);}
void zircon::SamaBlue::attack(Time now,const CombatWorld& world) {zircon_normal_attack(*this,now,world);}
void zircon::SamaBlue::range_attack(Time now,const CombatWorld& world) {
    if (!target) return;
    switch (world.random(5)) {
    case 0:zircon_single_magic(x,y,*target,direction,clock,now,ZMagic::thunder_bolt,Element::lightning,false,world);break;
    case 1:zircon_line_aoe(x,y,*target,direction,clock,now,10,-2,2,ZMagic::lightning_beam,Element::lightning,world);break;
    case 2:zircon_area_magic(x,y,*target,direction,clock,now,2,ZMagic::lightning_wave,Element::lightning,0,world);break;
    case 3:zircon_area_magic(x,y,*target,direction,clock,now,3,ZMagic::sama_guardian_lightning,Element::lightning,0,world);break;
    case 4:{
        const int damage=world.get_dc()*2;
        zircon_area_magic(x,y,*target,direction,clock,now,3,ZMagic::sama_blue_lightning,Element::lightning,damage,world);break;
    }
    }
}
zircon::SamaWhite::~SamaWhite()=default;
bool zircon::SamaWhite::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,2);
}
void zircon::SamaWhite::process_target(Time now,const CombatWorld& world) {zircon_sama_target(*this,now,world);}
void zircon::SamaWhite::attack(Time now,const CombatWorld& world) {zircon_normal_attack(*this,now,world);}
void zircon::SamaWhite::range_attack(Time now,const CombatWorld& world) {
    if (!target) return;
    switch (world.random(5)) {
    case 0:zircon_single_magic(x,y,*target,direction,clock,now,ZMagic::cyclone,Element::wind,false,world);break;
    case 1:zircon_line_aoe(x,y,*target,direction,clock,now,10,-2,2,ZMagic::blow_earth,Element::wind,world);break;
    case 2:zircon_area_magic(x,y,*target,direction,clock,now,2,ZMagic::dragon_tornado,Element::wind,0,world);break;
    case 3:zircon_area_magic(x,y,*target,direction,clock,now,3,ZMagic::sama_guardian_wind,Element::wind,0,world);break;
    case 4:{
        const int damage=world.get_dc()*2;
        zircon_area_magic(x,y,*target,direction,clock,now,3,ZMagic::sama_white_wind,Element::wind,damage,world);break;
    }
    }
}
zircon::SamaIceGuardian::~SamaIceGuardian()=default;
bool zircon::SamaIceGuardian::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,2);
}
void zircon::SamaIceGuardian::process_target(Time now,const CombatWorld& world) {zircon_sama_target(*this,now,world);}
void zircon::SamaIceGuardian::attack(Time now,const CombatWorld& world) {zircon_normal_attack(*this,now,world);}
void zircon::SamaIceGuardian::range_attack(Time now,const CombatWorld& world) {
    if (!target) return;
    switch (world.random(4)) {
    case 0:zircon_single_magic(x,y,*target,direction,clock,now,ZMagic::ice_blades,Element::ice,true,world);break;
    case 1:zircon_line_aoe(x,y,*target,direction,clock,now,10,-2,2,ZMagic::greater_frozen_earth,Element::ice,world);break;
    case 2:zircon_area_magic(x,y,*target,direction,clock,now,2,ZMagic::ice_storm,Element::ice,0,world);break;
    case 3:zircon_area_magic(x,y,*target,direction,clock,now,3,ZMagic::sama_guardian_ice,Element::ice,0,world);break;
    }
}
zircon::SamaLightningGuardian::~SamaLightningGuardian()=default;
bool zircon::SamaLightningGuardian::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,2);
}
void zircon::SamaLightningGuardian::process_target(Time now,const CombatWorld& world) {zircon_sama_target(*this,now,world);}
void zircon::SamaLightningGuardian::attack(Time now,const CombatWorld& world) {zircon_normal_attack(*this,now,world);}
void zircon::SamaLightningGuardian::range_attack(Time now,const CombatWorld& world) {
    if (!target) return;
    switch (world.random(4)) {
    case 0:zircon_single_magic(x,y,*target,direction,clock,now,ZMagic::thunder_bolt,Element::lightning,false,world);break;
    case 1:zircon_line_aoe(x,y,*target,direction,clock,now,10,-2,2,ZMagic::lightning_beam,Element::lightning,world);break;
    case 2:zircon_area_magic(x,y,*target,direction,clock,now,2,ZMagic::lightning_wave,Element::lightning,0,world);break;
    case 3:zircon_area_magic(x,y,*target,direction,clock,now,3,ZMagic::sama_guardian_lightning,Element::lightning,0,world);break;
    }
}
zircon::SamaWindGuardian::~SamaWindGuardian()=default;
bool zircon::SamaWindGuardian::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,2);
}
void zircon::SamaWindGuardian::process_target(Time now,const CombatWorld& world) {zircon_sama_target(*this,now,world);}
void zircon::SamaWindGuardian::attack(Time now,const CombatWorld& world) {zircon_normal_attack(*this,now,world);}
void zircon::SamaWindGuardian::range_attack(Time now,const CombatWorld& world) {
    if (!target) return;
    switch (world.random(4)) {
    case 0:zircon_single_magic(x,y,*target,direction,clock,now,ZMagic::cyclone,Element::wind,false,world);break;
    case 1:zircon_line_aoe(x,y,*target,direction,clock,now,10,-2,2,ZMagic::blow_earth,Element::wind,world);break;
    case 2:zircon_area_magic(x,y,*target,direction,clock,now,2,ZMagic::dragon_tornado,Element::wind,0,world);break;
    case 3:zircon_area_magic(x,y,*target,direction,clock,now,3,ZMagic::sama_guardian_wind,Element::wind,0,world);break;
    }
}
zircon::SamaFireGuardian::~SamaFireGuardian()=default;
bool zircon::SamaFireGuardian::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,2);
}
void zircon::SamaFireGuardian::process_target(Time now,const CombatWorld& world) {zircon_sama_target(*this,now,world);}
void zircon::SamaFireGuardian::attack(Time now,const CombatWorld& world) {zircon_normal_attack(*this,now,world);}
void zircon::SamaFireGuardian::range_attack(Time now,const CombatWorld& world) {
    if (!target) return;
    switch (world.random(4)) {
    case 0:zircon_single_magic(x,y,*target,direction,clock,now,ZMagic::adamantine_fire_ball,Element::fire,true,world);break;
    case 1:zircon_line_aoe(x,y,*target,direction,clock,now,10,-2,2,ZMagic::scortched_earth,Element::fire,world);break;
    case 2:zircon_area_magic(x,y,*target,direction,clock,now,2,ZMagic::fire_storm,Element::fire,0,world);break;
    case 3:zircon_area_magic(x,y,*target,direction,clock,now,3,ZMagic::sama_guardian_fire,Element::fire,0,world);break;
    }
}
zircon::Doll::Doll(Time created_at):appear_at_(created_at+500) {}
bool zircon::Doll::can_move() const {return false;}
bool zircon::Doll::can_attack() const {return false;}
bool zircon::Doll::can_be_seen(const std::function<bool()>& base) const {return visible && base();}
void zircon::Doll::spawned(Time now,const std::function<void()>& base,const std::function<void()>& effect,
    const std::function<void(std::optional<int>)>& add_cursed) {
    base();effect();action_at=now+1000;add_cursed(doll_target);
}
void zircon::Doll::process_actions(Time now,const std::function<void()>& add_all_objects) {
    if (!pending_ || now<appear_at_) return;
    pending_=false;visible=true;add_all_objects();
}
void zircon::Doll::process(Time now,const std::function<void()>& base,const std::function<std::optional<LinkedObject>()>& caster,
    const std::function<std::optional<LinkedObject>()>& victim,const std::function<void(int)>& set_hp) {
    base();
    const auto owner=caster();if (!owner || !owner->registered || owner->dead) set_hp(0);
    const auto target=victim();if (!target || !target->registered || target->dead) set_hp(0);
    if (alive_at<now) set_hp(0); // 三个独立判断，不能提前return或合并死亡回调。
}
void zircon::Doll::process_search() {}
void zircon::Doll::activate(const std::function<void()>& add_active) {
    if (activated) return;
    activated=true;add_active();
}
void zircon::Doll::deactivate() {}
void zircon::Doll::die(Time now,const std::function<void()>& base) {base();dead_at=now+1000;}
void zircon::Doll::despawned(const std::function<void()>& base,const std::function<void(std::optional<int>)>& remove_first_cursed) {
    base();remove_first_cursed(doll_target);
}
int zircon::Doll::attacked(int power,AttackOptions options,bool attacker_same_map,int ax,int ay,int tx,int ty,
    const std::function<void(int,int,AttackOptions)>& forward) {
    if (doll_target && attacker_same_map && ZumaAi::within(ax,ay,tx,ty,10)) forward(*doll_target,power,options);
    return 0;
}
std::optional<std::string> zircon::Doll::custom_name(Race race,const std::string& target_name) const {
    if (race==Race::player || race==Race::monster) return target_name+" Doll";
    return std::nullopt;
}
bool zircon::BlockingObject::can_move() const {return false;}
bool zircon::BlockingObject::can_attack() const {return false;}
bool zircon::BlockingObject::walk(int) const {return false;}
bool zircon::BlockingObject::blocking(const std::function<bool()>& parent) const {return visible && parent();}
bool zircon::BlockingObject::can_be_seen(const std::function<bool()>& base) const {return visible && base();}
void zircon::BlockingObject::process_roam() {}
void zircon::BlockingObject::process_search() {}
int zircon::BlockingObject::attacked(int power,AttackOptions options,const std::function<int(int,AttackOptions)>& parent) {
    return parent(power,options);
}
void zircon::BlockingObject::hide(const std::function<void()>& broadcast_hide) {
    visible=false;if (has_map) broadcast_hide();
}
void zircon::BlockingObject::show(const std::function<void()>& add_all_objects,const std::function<void()>& broadcast_show) {
    visible=true;if (!has_map) return;
    add_all_objects();broadcast_show();
}
void zircon::BlockingObject::die(const std::function<void()>& broadcast_hide) {hide(broadcast_hide);}
std::string zircon::BlockingObject::custom_name() const {return " ";}
bool zircon::IcySpiritWarrior::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,attack_range);
}
void zircon::IcySpiritWarrior::process_target(Time now,const CombatWorld& world,const std::function<void(int,Time)>& enqueue_function) {
    if (!target) return;
    if (in_attack_range() && world.can_attack()) attack(now,world,enqueue_function);
    zircon_move_toward(*this,world);
    if (in_attack_range() && world.can_attack()) attack(now,world,enqueue_function);
}
void zircon::IcySpiritWarrior::attack(Time now,const CombatWorld& world,const std::function<void(int,Time)>& enqueue_function) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);
    if (world.random(2)==0) {
        world.broadcast(direction,true,target->id);clock.update(now);enqueue_function(target->id,now+400);
    } else zircon_area_magic(x,y,*target,direction,clock,now,1,ZMagic::monster_ice_storm,Element::ice,0,world);
}
bool zircon::IcySpiritGeneral::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,attack_range);
}
void zircon::IcySpiritGeneral::process_target(Time now,const CombatWorld& world,const std::function<void(int,Time)>& enqueue_function) {
    if (!target) return;
    if (in_attack_range() && world.can_attack()) attack(now,world,enqueue_function);
    zircon_move_toward(*this,world);
    if (in_attack_range() && world.can_attack()) attack(now,world,enqueue_function);
}
void zircon::IcySpiritGeneral::attack(Time now,const CombatWorld& world,const std::function<void(int,Time)>& enqueue_function) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);
    if (world.random(2)==0) {
        world.broadcast(direction,true,target->id);clock.update(now);enqueue_function(target->id,now+400);
    } else zircon_area_magic(x,y,*target,direction,clock,now,1,ZMagic::monster_ice_storm,Element::ice,0,world);
}
void zircon::IcySpiritWarrior::deliver_function(int) {
    // Warrior未覆盖ProcessAction，MonsterObject/MapObject均不处理Function；不凭General补伤害。
}
int zircon::IcySpiritGeneral::deliver_function(int target,const std::function<int()>& get_dc,
    const std::function<int(int,int,Element)>& base_attack,const std::function<int(int)>& current_mp,
    const std::function<void(int,int)>& change_mp) {
    const int result=base_attack(target,get_dc(),Element::dark);
    if (result>0) change_mp(target,-std::min(current_mp(target),result));
    return result;
}
bool zircon::IcyGoddess::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,attack_range);
}
void zircon::IcyGoddess::process_target(Time now,const CombatWorld& world) {
    if (!target) return;
    if (in_attack_range() && visible && world.can_attack()) attack(now,world);
    zircon_move_toward(*this,world);
    if (in_attack_range() && visible && world.can_attack()) attack(now,world);
}
void zircon::IcyGoddess::attack(Time now,const CombatWorld& world) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);clock.update(now);action_at=clock.action_at;
    const bool melee=ZumaAi::within(x,y,target->x,target->y,1);
    world.broadcast(direction,!melee,target->id);
    const auto distance=std::max(std::abs(std::int64_t(x)-target->x),std::abs(std::int64_t(y)-target->y));
    world.enqueue({target->id,world.get_dc(),now+400+(melee?0:distance*48),attack_element});
}
void zircon::MonsterPoisonousCloud::cast(int x,int y,const MonsterAiTarget& target,int& direction,AttackClock& clock,Time now,
    const CombatWorld& combat,const std::function<void(Spawn)>& spawn) {
    zircon_begin_magic(x,y,target,direction,clock,now,ZMagic::poisonous_cloud,combat);
    for (const auto& cell:combat.cells_in_range(x,y,2))
        spawn({cell.x,cell.y,x,y,cell.x==x && cell.y==y,1,20000,20});
}
bool zircon::ChaosKnight::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,1);
}
void zircon::ChaosKnight::process_target(Time now,const CombatWorld& combat,const std::function<void(MonsterPoisonousCloud::Spawn)>& spawn) {
    if (!target) return;
    if (in_attack_range() && combat.can_attack() && now>cast_at) {
        cast_at=now+20000;MonsterPoisonousCloud::cast(x,y,*target,direction,clock,now,combat,spawn);
    }
    zircon_base_target(*this,now,combat);
}
void zircon::ChaosKnight::attack(Time now,const CombatWorld& combat) {zircon_normal_attack(*this,now,combat);}
bool zircon::RedMoonTheFallen::can_move() const {return false;}
bool zircon::RedMoonTheFallen::in_attack_range(bool same_map) const {return same_map;}
void zircon::RedMoonTheFallen::attack(Time now,const CombatWorld& combat,
    const std::function<void(int,std::shared_ptr<std::vector<int>>)>& broadcast) {
    auto ids=std::make_shared<std::vector<int>>();broadcast(direction,ids);clock.update(now);
    for (int id:combat.get_targets(x,y,40)) {
        ids->push_back(id);combat.enqueue({id,combat.get_dc(),now+500,attack_element});
    }
}
bool zircon::CrimsonNecromancer::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,1);
}
void zircon::CrimsonNecromancer::process_target(Time now,const CombatWorld& world,const std::function<void(int,Weakness)>& add_weakness) {
    if (world.can_attack() && now>debuff_at) {
        debuff_at=now+10000;
        const auto targets=world.get_targets(x,y,3);
        if (!targets.empty()) {
            const auto [tx,ty]=world.target_position(targets.front());direction=zircon_direction(x,y,tx,ty);
            world.broadcast(direction,true,0);clock.update(now);
            for (int id:targets) add_weakness(id,{});
        }
    }
    zircon_base_target(*this,now,world);
}
void zircon::CrimsonNecromancer::attack(Time now,const CombatWorld& world) {zircon_normal_attack(*this,now,world);}
void zircon::DoomClaw::spawned(Time now,const std::function<void()>& base,const std::function<void()>& show) {
    base();clock.action_at=now+2000;show();
}
int zircon::DoomClaw::pushed(int,int) const {return 0;}
int zircon::DoomClaw::attacked(int ax,int ay,int power,AttackOptions options,const std::function<int(int,AttackOptions)>& base) {
    if (!ZumaAi::within(x,y,ax,ay,10)) return 0;
    options.struck=true;return base(power,options);
}
void zircon::DoomClaw::process_ai(Time now,const CombatWorld& combat,const World& world) {
    if (dead) return;
    world.regen();if (!combat.can_attack()) return;
    const auto right=combat.get_targets(x+5,y,5);
    const auto left=combat.get_targets(x,y+5,5);
    const auto middle=combat.get_targets(x+5,y+5,combat.maximum_view_range);
    clock.update(now);
    if (combat.random(20)==0) {
        const auto all=combat.get_targets(x,y,combat.maximum_view_range);
        if (!all.empty()) perform(Move::wave,all,now,combat,world);
        return;
    }
    int value=int(combat.random(unsigned(right.size()+left.size()+middle.size())));
    if ((value-=int(right.size()))<0) {perform(combat.random(10)>0?Move::right_pinch:Move::right_swipe,middle,now,combat,world);return;}
    if ((value-=int(left.size()))<0) {perform(combat.random(10)>0?Move::left_pinch:Move::left_swipe,middle,now,combat,world);return;}
    if ((value-=int(middle.size()))<0) perform(Move::spit,middle,now,combat,world);
}
void zircon::DoomClaw::perform(Move move,const std::vector<int>& targets,Time now,const CombatWorld& combat,const World& world) {
    if (move!=Move::spit) world.broadcast(move,direction);
    clock.update(now);
    for (int id:targets) {
        int damage=combat.get_dc(),reduction=0;
        switch (world.victim_class(id)) {
        case VictimClass::warrior:reduction=4;break;
        case VictimClass::wizard:reduction=3;break;
        case VictimClass::taoist:reduction=6;break;
        case VictimClass::assassin:reduction=2;break;
        default:break;
        }
        damage-=damage*reduction/10;
        combat.enqueue({id,damage,now+400,attack_element});
        if (move==Move::wave) world.pushed(id,3,15);
        else if (move==Move::right_swipe) world.pushed(id,5,5);
        else if (move==Move::left_swipe) world.pushed(id,1,5);
    }
    if (move==Move::spit) world.broadcast(move,direction);
}
bool zircon::ChristmasMonster::can_move() const {return false;}
bool zircon::ChristmasMonster::can_attack() const {return false;}
void zircon::ChristmasMonster::refresh_stats(const std::function<void()>& base) {base();}
int zircon::ChristmasMonster::attacked(AttackOptions options,const std::function<int(int,AttackOptions)>& base) {
    options.critical=false;return base(1,options);
}
std::uint32_t zircon::ChristmasMonster::name_colour(Time now) const {
    if (now<shock_at) return 0xffcd853fu; // Peru优先于Rage。
    if (now<rage_at) return 0xffff0000u;
    return 0xfff0f8ffu; // AliceBlue
}
void zircon::ChristmasMonster::die(const MonsterPursuit::Random& random,const World& world,const std::function<void()>& base) {
    if (random(15)==0) {
        const auto objects=world.map_objects();
        for (auto it=objects.rbegin();it!=objects.rend();++it) {
            if (!it->monster || it->pet || it->guard || it->christmas || it->dead || it->move_delay==0 ||
                !it->can_move || it->has_target || it->level>=300) continue;
            world.teleport_random(it->id,x,y,15);
        }
    }
    if (exp_owner) {
        const auto targets=world.owner_targets(*exp_owner,x,y,18);
        for (const auto& mob:targets) {
            if (!mob.monster || mob.boss || mob.dead || (mob.exp_owner && mob.exp_owner!=exp_owner) || mob.christmas) continue;
            world.set_experience(mob.id,std::max(mob.extra_experience_rate,extra_experience_rate),*exp_owner);
            world.set_hp(mob.id,0);
        }
    }
    base();
}
bool zircon::Tornado::blocking() const {return false;}
int zircon::Tornado::pushed(int,int) const {return 0;}
bool zircon::Tornado::can_attack_target(const Target& target) const {
    if (target.self || !target.registered || target.dead || !target.visible || target.guard || target.castle_lord) return false;
    switch (target.race) {
    case Race::player:return !target.game_master && !safe_zone && !target.safe_zone;
    case Race::monster:return true;
    case Race::item:case Race::npc:case Race::spell:return false;
    default:throw std::logic_error("Tornado target race is not implemented in reference");
    }
}
void zircon::Tornado::spawned(Time now,const std::function<void()>& base,const std::function<void()>& show) {
    base();clock.action_at=now+2000;show();
}
void zircon::Tornado::die(Time now,const std::function<void()>& base) {base();dead_at=now+700;}
void zircon::Tornado::process(Time now,const std::function<void()>& base,const std::function<void()>& die) {
    base();if (now<=visible_at || dead) return;die();
}
void zircon::Tornado::process_roam(Time now,bool can_move,int seen_count,const std::function<bool()>& other_blocking,
    const CombatWorld& combat,const std::function<void(int)>& turn) {
    if (!can_move || now<roam_at || seen_count==0) return;
    roam_at=now+roam_delay;
    if (other_blocking() && combat.can_attack()) attack(now,combat);
    if (combat.random(5)>0) combat.walk(direction);
    else turn(int(combat.random(8)));
}
void zircon::Tornado::attack(Time now,const CombatWorld& combat) {
    for (int victim:combat.get_targets(x,y,1)) {
        if (victim==id || !combat.can_attack_target(victim)) continue;
        combat.enqueue({victim,combat.get_dc(),now+100,attack_element});
    }
    clock.update(now);
}
bool zircon::SamaScorcer::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,3);
}
int zircon::SamaScorcer::attacked(int power,AttackOptions options,const std::function<int(int,AttackOptions)>& base,const SamaWorld& world) {
    for (const auto& ob:world.map_objects()) if (ob.monster && ob.flag==SamaWorld::Flag::blood_stone) return 0;
    options.struck=true;return base(power,options);
}
void zircon::SamaScorcer::process_target(Time now,const CombatWorld& combat,const SamaWorld& world) {
    if (!target || !combat.can_attack()) return;
    if (combat.random(5)==0 && !ZumaAi::within(x,y,target->x,target->y,5)) world.teleport_random(target->id,x,y,2);
    zircon_base_target(*this,now,combat);
}
void zircon::SamaScorcer::attack(Time now,const CombatWorld& combat) {zircon_normal_attack(*this,now,combat);}
zircon::SamaProphet::~SamaProphet()=default;
bool zircon::SamaProphet::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,12);
}
void zircon::SamaProphet::spawned(const std::function<void()>& base,const SamaWorld& world) {
    base();world.spawn_minions(1,0,target);
}
bool zircon::SamaProphet::spawn_minion(int id,const SamaWorld& world) {return world.spawn_random(id,x-5,y+5,4);}
void zircon::SamaProphet::process(Time now,const std::function<void()>& base,const SamaWorld& world) {
    base();if (dead || now<check_at) return;
    check_at=now+10000;
    for (const auto& ob:world.map_objects()) {
        if (!ob.monster || ob.flag!=SamaWorld::Flag::sama_sorcerer || ZumaAi::within(x,y,ob.x,ob.y,view_range-3)) continue;
        world.teleport_random(ob.id,x-2,y+2,1);break;
    }
}
int zircon::SamaProphet::attacked(int power,AttackOptions options,const std::function<int(int,AttackOptions)>& base,const SamaWorld& world) {
    for (const auto& ob:world.map_objects())
        if (ob.monster && (ob.flag==SamaWorld::Flag::blood_stone || ob.flag==SamaWorld::Flag::sama_sorcerer)) return 0;
    options.struck=true;return base(power,options);
}
void zircon::SamaProphet::process_target(Time now,const CombatWorld& combat,const SamaWorld& world) {
    if (!target || !combat.can_attack()) return;
    if (combat.random(5)==0 && !ZumaAi::within(x,y,target->x,target->y,5)) world.teleport_random(target->id,x-2,y+2,1);
    range_attack(now,combat);
}
void zircon::SamaProphet::range_attack(Time now,const CombatWorld& combat) {
    if (!target) return;
    constexpr ZMagic magic[]={ZMagic::sama_prophet_fire,ZMagic::sama_prophet_wind,ZMagic::sama_prophet_lightning};
    // 源码SamaProphetWind使用Ice属性，不能按名称改成Wind。
    constexpr Element elements[]={Element::fire,Element::ice,Element::lightning};
    const auto choice=combat.random(3);
    zircon_area_magic(x,y,*target,direction,clock,now,15,magic[choice],elements[choice],0,combat);
}
zircon::SamaPhoenix::~SamaPhoenix()=default;
bool zircon::SamaPhoenix::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,2);
}
void zircon::SamaPhoenix::process_target(Time now,const CombatWorld& world) {zircon_sama_target(*this,now,world);}
void zircon::SamaPhoenix::attack(Time now,const CombatWorld& world) {zircon_normal_attack(*this,now,world);}
void zircon::SamaPhoenix::range_attack(Time now,const CombatWorld& world) {
    if (!target) return;
    switch (world.random(5)) {
    case 0:zircon_single_magic(x,y,*target,direction,clock,now,ZMagic::adamantine_fire_ball,Element::fire,true,world);break;
    case 1:zircon_line_aoe(x,y,*target,direction,clock,now,10,-2,2,ZMagic::scortched_earth,Element::fire,world);break;
    case 2:zircon_area_magic(x,y,*target,direction,clock,now,2,ZMagic::fire_storm,Element::fire,0,world);break;
    case 3:zircon_area_magic(x,y,*target,direction,clock,now,3,ZMagic::sama_guardian_fire,Element::fire,0,world);break;
    case 4:{const int damage=world.get_dc()*2;
        zircon_area_magic(x,y,*target,direction,clock,now,3,ZMagic::sama_phoenix_fire,Element::fire,damage,world);break;}
    }
}
zircon::MonsterObject::~MonsterObject()=default;
namespace {
using CompanionData=zircon::CompanionState;
void add_companion_stats(CompanionData::Stats& to,const CompanionData::Stats& from) {
    for (auto [stat,value]:from.values) to.values[stat]+=value;
    to.bag_weight+=from.bag_weight;to.inventory+=from.inventory;to.hunger+=from.hunger;
}
bool companion_stack(const CompanionData::Item& a,const CompanionData::Item& b) {
    return a.info==b.info && a.count<a.info->stack_size && !a.expirable && a.bound==b.bound &&
        a.worthless==b.worthless && a.non_refinable==b.non_refinable && a.stats==b.stats;
}
int companion_weight(const CompanionData::Item& item) {
    if (item.info->type==CompanionData::ItemType::amulet || item.info->type==CompanionData::ItemType::poison) return item.info->weight;
    return int(std::min<std::int64_t>(std::numeric_limits<int>::max(),item.info->weight*item.count));
}
}
zircon::Companion::Companion(Time now,int user_companion,const std::vector<ItemPtr>& items) {
    companion_id=user_companion;visible=false;next_chat_at=next_idle_at=now+10000;
    for (const auto& item:items) {
        if (item->slot<equipment_offset) continue;
        const int slot=item->slot-equipment_offset;if (slot>=equipment_count) continue;
        if (item->info->type==ItemType::head) head_shape=item->info->shape;
        else if (item->info->type==ItemType::back) back_shape=item->info->shape;
        equipment[slot]=item;
    }
    for (const auto& item:items) {
        if (item->slot>=equipment_offset || item->slot>=inventory_count) continue;
        inventory[item->slot]=item;
    }
}
bool zircon::Companion::blocking() const {return false;}
bool zircon::Companion::can_data_be_seen() const {return false;}
bool zircon::Companion::can_be_seen(bool viewer_is_owner,const std::function<bool()>& base,const std::function<bool()>& owner_visible) const {
    return viewer_is_owner?base():owner_visible();
}
void zircon::Companion::activate(const std::function<void()>& add_active) {if (!activated) {activated=true;add_active();}}
void zircon::Companion::deactivate() {}
void zircon::Companion::refresh_stats(const World& world) {
    stats=monster_stats;
    for (const auto& item:equipment) if (item) {add_companion_stats(stats,item->info->stats);add_companion_stats(stats,item->stats);}
    stats.bag_weight+=level_weight;stats.inventory+=level_space;refresh_weight(world);
}
void zircon::Companion::refresh_weight(const World& world) {
    bag_weight=0;
    for (const auto& item:inventory) if (item) {
        bag_weight+=companion_weight(*item);
        if (item->info->type==ItemType::head) head_shape=item->info->shape;
        else if (item->info->type==ItemType::back) back_shape=item->info->shape;
    }
    world.weight_update(bag_weight,stats.bag_weight,stats.inventory);
}
void zircon::Companion::send_shape_update(const World& world) {
    head_shape=back_shape=0;
    for (const auto& item:equipment) if (item) {
        if (item->info->type==ItemType::head) head_shape=item->info->shape;
        else if (item->info->type==ItemType::back) back_shape=item->info->shape;
    }
    world.shape_update(head_shape,back_shape);
}
void zircon::Companion::recall(const World& world) {
    const auto [ox,oy,dir]=world.owner_position();const auto d=MonsterPursuit::directions[dir];
    int tx=ox-d[0],ty=oy-d[1];
    if (!world.cell_exists(tx,ty) || world.cell_has_movements(tx,ty)) {tx=ox;ty=oy;}
    world.teleport_to_owner_map(tx,ty);std::tie(x,y)=world.position();
}
bool zircon::Companion::walk(int facing,const World& world) {
    const auto d=MonsterPursuit::directions[facing];const int tx=x+d[0],ty=y+d[1];
    if (!world.cell_exists(tx,ty)) return false;
    direction=facing;world.update_move_time();world.move(tx,ty,facing);x=tx;y=ty;return true;
}
void zircon::Companion::move_to(int tx,int ty,const MonsterPursuit::Random& random,const World& world) {
    if (!world.can_move() || (x==tx && y==ty)) return;
    int facing=zircon_direction(x,y,tx,ty);const int rotation=random(2)==0?1:7;
    for (int i=0;i<8;++i) {if (walk(facing,world)) return;facing=(facing+rotation)%8;}
}
void zircon::Companion::process_ai(Time now,const MonsterPursuit::Random& random,const World& world) {
    if (!world.owner_sees()) recall(world);
    const auto item=target_item?world.ground_item(*target_item):std::nullopt;
    if (!item || !item->registered || !item->same_map || !ZumaAi::within(x,y,item->x,item->y,view_range)) target_item.reset();
    process_search(now,random,world);process_roam(now,random,world);process_target(now,random,world);
}
void zircon::Companion::process_search(Time now,const MonsterPursuit::Random& random,const World& world) {
    if (!world.can_move() || now<search_at) return;
    int distance=std::numeric_limits<int>::max();std::vector<int> closest;
    for (const auto& item:world.visible_items()) {
        const int d=std::max(std::abs(x-item.x),std::abs(y-item.y));
        if (d>view_range || d>distance || !item.monster_drop || !item.pickable) continue;
        if (d!=distance) closest.clear();
        closest.push_back(item.id);distance=d;
    }
    if (closest.empty()) {search_at=now+10000;return;}
    target_item=closest[random(unsigned(closest.size()))];
}
bool zircon::Companion::try_chat(Action action,Time now,const MonsterPursuit::Random& random,const World& world) {
    if (now<next_chat_at) return false;next_chat_at=now+30000;
    std::vector<const Speech*> choices;for (const auto& speech:speeches) if (speech.action==action) choices.push_back(&speech);
    if (choices.empty()) return false;
    world.chat(choices[random(unsigned(choices.size()))]->text);return true;
}
void zircon::Companion::process_roam(Time now,const MonsterPursuit::Random& random,const World& world) {
    if (target_item) return;
    const auto [ox,oy,dir]=world.owner_position();
    if (!ZumaAi::within(x,y,ox,oy,7)) move_to_owner=true;
    if (move_to_owner) {
        const auto d=MonsterPursuit::directions[dir];move_to(ox-d[0],oy-d[1],random,world);
        if (random(100)>=60) try_chat(Action::moving,now,random,world);
        if (ZumaAi::within(x,y,ox,oy,1)) move_to_owner=false;
        next_idle_at=now+30000;
    } else if (random(100)>=60) process_idle(now,random,world);
    else if (random(100)>=60) try_chat(Action::idle,now,random,world);
}
void zircon::Companion::process_idle(Time now,const MonsterPursuit::Random& random,const World& world) {
    if (now<next_idle_at) return;next_idle_at=now+30000;int delay=600;
    if (image==Image::pig) {
        switch (random(4)) {
        case 0:world.idle(1);world.idle(2);world.idle(3);delay=1700;break;
        case 1:world.idle(4);world.idle(5);world.idle(6);delay=1700;break;
        case 2:world.idle(7);break;
        case 3:world.idle(8);break;
        }
    } else {
        const int count=image==Image::sheep?2:(image==Image::tusk_lord || image==Image::skeleton_lord ||
            image==Image::banyo_lord || image==Image::rabbit || image==Image::dog)?3:4;
        world.idle(1+int(random(unsigned(count))));
    }
    clock.attack_at=now+clock.attack_delay;clock.action_at=now+delay;
}
void zircon::Companion::process_target(Time now,const MonsterPursuit::Random& random,const World& world) {
    if (!target_item) return;
    const auto item=world.ground_item(*target_item);move_to(item->x,item->y,random,world);
    if (x!=item->x || y!=item->y) return;
    world.pickup(item->id);
    if (random(100)>=60) try_chat(Action::pickup,now,random,world);
    next_idle_at=now+30000;
}
void zircon::Companion::process_target(Time now,const CombatWorld& world) {process_target(now,world.random,interactions);}
void zircon::Companion::remove_item(Item& item) {
    item.slot=-1;item.character.reset();item.account.reset();item.mail.reset();item.auction.reset();item.companion.reset();item.guild.reset();item.locked=false;
}
bool zircon::Companion::can_use_item(const ItemInfo& info) const {
    if (info.requirement==Requirement::minimum_level && level<info.required_amount) return false;
    if (info.requirement==Requirement::maximum_level && level>info.required_amount) return false;
    return true;
}
bool zircon::Companion::can_wear_item(const Item& item,Slot slot,const std::function<bool(ItemType,Slot)>& correct_slot) const {
    return correct_slot(item.info->type,slot) && can_use_item(*item.info);
}
void zircon::Companion::auto_feed(Time now,const MonsterPursuit::Random& random,const World& world) {
    if (hunger>0) return;
    auto item=equipment[int(Slot::food)];
    if (!item || !can_use_item(*item->info)) {try_chat(Action::hunger,now,random,world);return;}
    try_chat(Action::eating,now,random,world);hunger=std::min(maximum_hunger,item->info->stats.hunger);
    if (item->count>1) {--item->count;world.food_changed(item->count);}
    else {remove_item(*item);equipment[int(Slot::food)].reset();world.delete_item(item);world.food_changed(0);}
}
zircon::Companion::Stats zircon::Companion::get_skill(int requested_level,const std::vector<Skill>& table,const MonsterPursuit::Random& random) const {
    int total=0;for (const auto& skill:table) if (skill.level==requested_level) total+=skill.weight;
    int value=int(random(unsigned(total)));Stats result;
    for (const auto& skill:table) if (skill.level==requested_level) {
        value-=skill.weight;if (value>=0) continue;
        result.values[skill.stat]=int(random(unsigned(skill.maximum)))+1;break;
    }
    return result;
}
void zircon::Companion::check_skills(const std::vector<Skill>& table,const MonsterPursuit::Random& random,const World& world) {
    bool changed=false;
    for (int threshold:{3,5,7,10,11,13,15}) if (level>=threshold && skills[threshold].values.empty()) {
        skills[threshold]=get_skill(threshold,table,random);changed=true;
    }
    world.refresh_owner_buff();if (changed) world.skills_changed();
}
bool zircon::Companion::filter_picks(const Item& item) const {
    if (item.info->currency) return true;
    const auto& info=item.part_info?*item.part_info:*item.info;
    if (!filter_types.empty() && std::find(filter_types.begin(),filter_types.end(),info.type_name)==filter_types.end()) return false;
    if (!filter_rarities.empty() && std::find(filter_rarities.begin(),filter_rarities.end(),info.rarity)==filter_rarities.end()) return false;
    return info.all_classes || (info.classes&filter_classes)==filter_classes;
}
bool zircon::Companion::can_gain_items(bool check_weight,const std::vector<ItemPtr>& items) const {
    int index=0;
    for (const auto& item:items) {
        if (item->quest) continue;
        if (!filter_picks(*item)) return false;
        auto count=item->count;const auto& info=*item->info;
        if (info.experience || info.currency) continue;
        if (check_weight) {
            const auto weight=(info.type==ItemType::amulet || info.type==ItemType::poison)?info.weight:info.weight*count;
            if (bag_weight+weight>stats.bag_weight) return false;
        }
        if (info.stack_size>1 && !item->expirable) {
            for (const auto& old:inventory) if (old && companion_stack(*old,*item)) {
                count-=info.stack_size-old->count;if (count<=0) break;
            }
            if (count<=0) break; // 参考退出整个checks循环，不改为continue。
        }
        for (int i=index;i<inventory_count;++i) {
            if (i>=stats.inventory) break;
            ++index;if (!inventory[i]) {count-=info.stack_size;if (count<=0) break;}
        }
        if (count>0) return false;
    }
    return true;
}
void zircon::Companion::gain_items(const std::vector<ItemPtr>& items,const World& world) {
    std::vector<ItemPtr> notices;for (const auto& item:items) if (!item->info->experience) notices.push_back(item);
    world.items_gained(notices);std::vector<int> changed_quests;
    const auto destroy=[&](const ItemPtr& item){item->temporary=true;world.delete_item(item);};
    for (const auto& item:items) {
        if (item->task) {
            auto& task=*item->task;if (task.amount>=task.required) continue;
            task.amount=std::min(task.required,task.amount+item->count);
            if (std::find(changed_quests.begin(),changed_quests.end(),task.quest)==changed_quests.end()) changed_quests.push_back(task.quest);
            if (task.amount>=task.required) for (auto i=task.objects.rbegin();i!=task.objects.rend();++i) world.despawn_task_object(*i);
            item->task.reset();item->quest=false;destroy(item);continue;
        }
        if (item->info->currency) {
            world.gain_currency(*item->info,item->count);destroy(item);world.log_currency(*item->info,item->count);continue;
        }
        if (item->info->experience) {world.gain_experience(item->count);destroy(item);continue;}
        bool handled=false;
        if (item->info->stack_size>1 && !item->expirable) {
            for (auto& old:inventory) if (old && companion_stack(*old,*item)) {
                if (old->count+item->count<=item->info->stack_size) {
                    old->count+=item->count;destroy(item);handled=true;break;
                }
                item->count-=item->info->stack_size-old->count;old->count=item->info->stack_size;
            }
            if (handled) continue;
        }
        for (int i=0;i<inventory_count;++i) {
            if (i>=stats.inventory) break;
            if (inventory[i]) continue;
            inventory[i]=item;item->slot=i;item->companion=companion_id;item->temporary=false;break;
        }
    }
    for (int quest:changed_quests) world.quest_changed(quest);
    refresh_stats(world);
}
void zircon::MonsterObject::death_cloud(Time now,int tx,int ty,const CombatWorld& world) {
    bool show=true;
    for (const auto& cell:world.cells_in_range(tx,ty,2)) {
        death_clouds.push_back({now+500,cell.x,cell.y,tx,ty,show});show=false;
    }
}
void zircon::MonsterObject::process_death_clouds(Time now,const CombatWorld& world,
    const std::function<void(int,int,int,int,bool,Time)>& spawn) {
    for (std::size_t i=death_clouds.size();i>0;--i) {
        if (death_clouds[i-1].due>now) continue;
        const auto cloud=death_clouds[i-1];death_clouds.erase(death_clouds.begin()+std::ptrdiff_t(i-1));
        spawn(cloud.x,cloud.y,cloud.display_x,cloud.display_y,cloud.visible,
            now+death_cloud_duration_min+world.random(unsigned(death_cloud_duration_random)));
    }
}
void zircon::MonsterObject::dragon_repulse(Time now,const CombatWorld& world) {
    direction=zircon_direction(x,y,target->x,target->y);
    auto cast=std::make_shared<CombatWorld::MagicCast>();cast->direction=direction;cast->magic=ZMagic::dragon_repulse;
    world.broadcast_magic(cast);clock.update(now);world.add_dragon_repulse_buff(6000,1000,500);
}
void zircon::MonsterObject::dragon_repulse_tick(Time now,const CombatWorld& world,const std::function<void(int,Time)>& enqueue) {
    const auto cells=world.cells_in_range(x,y,5);
    for (auto cell=cells.rbegin();cell!=cells.rend();++cell) {
        if (!cell->objects || std::abs(cell->x-x)+std::abs(cell->y-y)>5) continue;
        for (auto victim=cell->objects->rbegin();victim!=cell->objects->rend();++victim)
            if (world.can_attack_target(*victim)) enqueue(*victim,now+400);
    }
}
void zircon::MonsterObject::dragon_repulse_end(int victim,const CombatWorld& world,
    const std::function<int(int,int,Element)>& attack,const std::function<int(int,int,int)>& push) {
    if (attack(victim,world.get_dc(),attack_element)<=0) return;
    const auto [tx,ty]=world.target_position(victim);const int dir=zircon_direction(x,y,tx,ty);
    if (push(victim,dir,1)!=0) return;
    const int rotation=world.random(2)==0?1:7;
    if (push(victim,(dir+rotation)%8,1)>0) return;
    push(victim,(dir+8-rotation)%8,1);
}
zircon::FrostLordHwa::FrostLordHwa(int day_of_week) {
    affinity=day_of_week==0?Element::phantom:Element(day_of_week);
}
void zircon::FrostLordHwa::apply_bonus_stats(const std::function<void()>& base,const std::function<void(Element,int)>& set_affinity) {
    base();set_affinity(affinity,1);
}
void zircon::FrostLordHwa::process_ai(Time now,const std::function<void()>& base,const CombatWorld& world) {
    base();if (dead) return;
    if (now>ting_at) {
        ting_at=now+60000;
        for (int victim:world.get_targets(x,y,world.maximum_view_range)) {
            interactions.teleport_random_near(victim,x,y,world.maximum_view_range);
            interactions.poison(victim,Poison::abyss,10000,1);
            interactions.poison(victim,Poison::silenced,10000,1);
            interactions.poison(victim,Poison::red,10000,1);
            interactions.poison(victim,Poison::wraith_grip,5000,1);
        }
    }
}
void zircon::FrostLordHwa::process_target(Time now,const CombatWorld& world) {
    if (!target) return;
    if (!ZumaAi::within(x,y,target->x,target->y,3) && now>teleport_at &&
        zircon_teleport_to_target(x,y,*target,direction,world)) teleport_at=now+5000;
    if (interactions.is_monster(target->id)) {interactions.set_hp(target->id,0);return;}
    MonsterObject::process_target(now,world);
}
bool zircon::PachontheChaosbringer::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,2);
}
void zircon::PachontheChaosbringer::process_target(Time now,const CombatWorld& world) {
    if (!target) return;
    if (!ZumaAi::within(x,y,target->x,target->y,8) && now>cast_at) {
        if (zircon_teleport_to_target(x,y,*target,direction,world)) cast_at=now+10000;
    } else if (in_attack_range() && now>cast_at) {cast_at=now+20000;dragon_repulse(now,world);}
    MonsterObject::process_target(now,world);
}
void zircon::BanyoLordGuzak::process(Time now,const std::function<void()>& base,const CombatWorld& world,
    const std::function<void(int)>& purify,const std::function<std::vector<DragonLord::Minion>()>& minions,
    const std::function<void(int)>& clear_exp_owner,const std::function<void(int)>& die,const std::function<void(int)>& despawn,
    const std::function<void(int,int,std::optional<MonsterAiTarget>)>& spawn_minions) {
    base();if (dead) return;
    if (now>purify_at) {
        purify_at=now+20000;
        for (int victim:world.get_targets(x,y,world.maximum_view_range)) purify(victim);
    }
    if (now>=slave_at) {
        slave_at=now+45000;const auto children=minions();
        for (auto i=children.rbegin();i!=children.rend();++i) {
            if (i->same_map && ZumaAi::within(x,y,i->x,i->y,10)) continue;
            clear_exp_owner(i->id);die(i->id);despawn(i->id);
        }
        spawn_minions(5-int(minions().size()),0,target);
    }
}
bool zircon::BanyoLordGuzak::spawn_minion(int id,const std::function<bool(int,int,int,int)>& spawn_random) const {return spawn_random(id,x,y,10);}
bool zircon::JinchonDevil::in_attack_range() const {
    if (!target || !same_map) return false;
    const int dx=std::abs(x-target->x),dy=std::abs(y-target->y);
    return (dx||dy) && dx<=3 && dy<=3 && (dx==0 || dy==0 || dx==dy);
}
void zircon::JinchonDevil::process_target(Time now,const CombatWorld& world) {
    if (!target) return;
    if (world.can_attack() && now>cast_at) {
        const auto targets=world.get_targets(x,y,view_range);
        if (!targets.empty()) {
            for (int victim:targets) {
                if (hp>maximum_hp/2 && world.random(2)>0) continue;
                const auto [tx,ty]=world.target_position(victim);death_cloud(now,tx,ty,world);
            }
            clock.update(now);
            auto cast=std::make_shared<CombatWorld::MagicCast>();
            cast->direction=direction;cast->magic=ZMagic::none;cast->targets.push_back(target->id);
            world.broadcast_magic(cast);cast_at=now+cast_delay;
        }
    }
    if (!in_attack_range()) {zircon_move_toward(*this,world);return;}
    if (world.can_attack()) attack(now,world);
}
void zircon::JinchonDevil::attack(Time now,const CombatWorld& world) {
    direction=zircon_direction(x,y,target->x,target->y);
    const auto d=MonsterPursuit::directions[direction];
    if (world.random(3)==0 || !ZumaAi::within(x,y,target->x,target->y,2)) {
        world.broadcast(direction,true,0);
        for (int step=1;step<=3;++step) {
            const int tx=x+d[0]*step,ty=y+d[1]*step;
            if (tx==target->x && ty==target->y) world.enqueue({target->id,world.get_dc(),now+400,attack_element});
            else if (auto cell=world.cell(tx,ty);cell && cell->objects) {
                for (int victim:*cell->objects) if (world.can_attack_target(victim)) {
                    world.enqueue({victim,world.get_dc(),now+400,attack_element});break;
                }
            }
        }
    } else {
        world.broadcast(direction,false,0);
        for (int victim:world.get_targets(x+d[0],y+d[1],1)) {
            world.enqueue({victim,world.get_dc(),now+400,attack_element});break;
        }
    }
    clock.update(now);
}
zircon::CarnivorousPlant::CarnivorousPlant() {visible=false;}
void zircon::CarnivorousPlant::process_target(Time now,const CombatWorld& world) {
    if (visible) MonsterObject::process_target(now,world);
}
void zircon::JinhwanSpirit::die(const std::function<void()>& base,const MonsterPursuit::Random& random,
    const std::function<void(int,int,std::optional<MonsterAiTarget>)>& spawn_minions) {
    base();if (random(unsigned(spawn_chance))>0) return;spawn_minions(20,10,std::nullopt);
}
void zircon::ArchLichTaedu::spawned(const std::function<void()>& base) {base();stage=max_stage;}
void zircon::ArchLichTaedu::process(const std::function<void()>& base,const std::function<void()>& show,
    const std::function<void(int,int,std::optional<MonsterAiTarget>)>& spawn_minions) {
    base();if (dead) return;
    if (hp*max_stage/maximum_hp>=stage || stage<=0) return;
    --stage;clock.action_at+=1000;show();spawn_minions(min_spawn,random_spawn,target);
}
void zircon::ArchLichTaedu::process_target(Time now,const CombatWorld& world) {zircon_mage_target(*this,now,world);}
void zircon::ArchLichTaedu::range_attack(Time now,const CombatWorld& world) {
    direction=zircon_direction(x,y,target->x,target->y);clock.update(now);
    world.broadcast(direction,true,target->id);world.enqueue({target->id,world.get_dc(),now+400,attack_element});
}
int zircon::EnragedArchLichTaedu::attacked(int power,AttackOptions options,const std::function<int(int,AttackOptions)>& base,
    const std::function<void(int,int)>& teleport_nearby) {
    options.struck=true;const int result=base(power,options);
    if (result<0 || dead || !can_teleport || hp>maximum_hp/2) return result;
    can_teleport=false;teleport_nearby(7,12);return result;
}
void zircon::EnragedArchLichTaedu::process_target(Time now,const CombatWorld& world) {
    if (!target) return;
    if (!ZumaAi::within(x,y,target->x,target->y,1) && now>teleport_at && world.can_attack()) {
        teleport_at=now+1000;
        if (world.random(7)==0 && zircon_teleport_to_target(x,y,*target,direction,world)) bonus=true;
    }
    ArchLichTaedu::process_target(now,world);
}
void zircon::EnragedArchLichTaedu::attack(Time now,const CombatWorld& world) {
    direction=zircon_direction(x,y,target->x,target->y);world.broadcast(direction,false,0);clock.update(now);
    int damage=world.get_dc();if (bonus) damage*=2;
    world.enqueue({target->id,damage,now+400,attack_element});bonus=false;
}
bool zircon::LordNiJae::in_attack_range() const {return target && same_map && ZumaAi::within(x,y,target->x,target->y,10);}
void zircon::LordNiJae::attack(Time now,const CombatWorld& world) {
    world.broadcast(direction,false,0);clock.update(now);
    for (int victim:world.get_targets(x,y,10)) {
        const auto [tx,ty]=world.target_position(victim);
        world.enqueue({victim,world.get_dc(),now+500+48*std::max(std::abs(tx-x),std::abs(ty-y)),attack_element});
    }
}
int zircon::LordNiJae::deliver(int victim,int power,Element element,const std::function<int(int,int,Element)>& base,
    const MonsterPursuit::Random& random,const std::function<int()>& get_sc,const std::function<void(int,PoisonInfo)>& apply_poison) {
    const int result=base(victim,power,element);if (result<=0) return result;
    if (random(5)==0) apply_poison(victim,{Poison::green,get_sc(),2000,10});
    if (random(10)==0) apply_poison(victim,{Poison::paralysis,0,5000,1});
    return result;
}
void zircon::EnragedLordNiJae::process(Time now,const std::function<void()>& base,
    const std::function<std::vector<DragonLord::Minion>()>& minions,const std::function<void(int)>& clear_exp_owner,
    const std::function<void(int)>& die,const std::function<void(int)>& despawn,
    const std::function<void(int,int,std::optional<MonsterAiTarget>)>& spawn_minions) {
    base();if (dead || now<slave_at) return;slave_at=now+30000;
    const auto children=minions();
    for (auto i=children.rbegin();i!=children.rend();++i) {
        if (i->same_map && ZumaAi::within(x,y,i->x,i->y,10)) continue;
        clear_exp_owner(i->id);die(i->id);despawn(i->id);
    }
    spawn_minions(5-int(minions().size()),0,target);
}
bool zircon::EnragedLordNiJae::spawn_minion(int id,const std::function<bool(int,int,int,int)>& spawn_random) const {return spawn_random(id,x,y,10);}
zircon::ArachnidGrazer::ArachnidGrazer(const MonsterPursuit::Random& random) {direction=int(random(3));}
bool zircon::ArachnidGrazer::can_move() const {return false;}
bool zircon::ArachnidGrazer::in_attack_range() const {return target && same_map;}
void zircon::ArachnidGrazer::attack(Time now,const CombatWorld& world) {
    world.broadcast(direction,false,0);clock.update(now);hatch_actions.push_back(now+600);
}
void zircon::ArachnidGrazer::process_actions(Time now,
    const std::function<void(int,int,std::optional<MonsterAiTarget>)>& spawn_minions,const std::function<void()>& base_function) {
    for (std::size_t i=hatch_actions.size();i>0;--i) {
        if (hatch_actions[i-1]>now) continue;
        hatch_actions.erase(hatch_actions.begin()+std::ptrdiff_t(i-1));
        spawn_minions(1,0,target);base_function(); // 参考break后仍调用base.ProcessAction，目标在孵化时读取。
    }
}
bool zircon::ArachnidGrazer::spawn_minion(int id,const std::function<bool(int,int,int)>& spawn) const {
    const auto d=MonsterPursuit::directions[(direction+3)%8];return spawn(id,x+d[0],y+d[1]);
}
bool zircon::DragonLord::in_attack_range() const {return target && same_map && ZumaAi::within(x,y,target->x,target->y,attack_range);}
void zircon::DragonLord::process_target(Time now,const CombatWorld& world) {
    if (!target) return;
    if (!in_attack_range()) {zircon_move_toward(*this,world);return;}
    if (world.can_attack()) attack(now,world);
}
void zircon::DragonLord::attack(Time now,const CombatWorld& world) {
    direction=zircon_direction(x,y,target->x,target->y);
    auto packet=std::make_shared<CombatWorld::RangeCast>();packet->direction=direction;
    world.broadcast_range(packet);clock.update(now);
    for (int victim:world.get_targets(x,y,attack_range)) {
        if (world.random(10)>3) continue;
        packet->targets.push_back(victim);
        const auto [tx,ty]=world.target_position(victim);
        for (int other:world.get_targets(tx,ty,2)) world.enqueue({other,world.get_dc(),now+1000,attack_element});
    }
}
void zircon::DragonLord::process(Time now,const std::function<void()>& base,const std::function<std::vector<Minion>()>& minions,
    const std::function<void(int)>& clear_exp_owner,const std::function<void(int)>& die,
    const std::function<void(int)>& despawn,const std::function<void(int,int,std::optional<MonsterAiTarget>)>& spawn_minions) {
    base();if (dead || now<slave_at) return;
    slave_at=now+15000;
    const auto children=minions();
    for (auto i=children.rbegin();i!=children.rend();++i) {
        if ((i->same_map && ZumaAi::within(x,y,i->x,i->y,15)) || i->boss) continue;
        clear_exp_owner(i->id);die(i->id);despawn(i->id);
    }
    spawn_minions(10-int(minions().size()),0,target);
}
bool zircon::DragonLord::spawn_minion(int id,const std::function<bool(int,int,int,int)>& spawn_random) const {return spawn_random(id,x,y,10);}
void zircon::MonasteryBoss::die(const std::function<void()>& base,
    const std::function<void(int,int,std::optional<MonsterAiTarget>)>& spawn_minions) {
    base();if (has_died) return;has_died=true;spawn_minions(1,0,std::nullopt);
}
void zircon::MonasteryBoss::range_attack(Time now,const CombatWorld& world) {
    if (ZumaAi::within(x,y,target->x,target->y,10))
        zircon_single_magic(x,y,*target,direction,clock,now,ZMagic::green_sludge_ball,Element::wind,true,world);
}
bool zircon::HalloweenMonster::in_attack_range() const {return target && same_map && ZumaAi::within(x,y,target->x,target->y,10);}
std::uint32_t zircon::HalloweenMonster::name_colour() const {return 0xFF9370DB;}
long double zircon::HalloweenMonster::experience(long double base) const {return base+bonus_experience;}
void zircon::HalloweenMonster::spawned(const std::function<void()>& base,long double& experience_rate,long double& drop_rate,long double& gold_rate) {
    base();experience_rate*=10;drop_rate*=10;gold_rate*=30;
}
bool zircon::HalloweenMonster::can_attack_target(const Victim& victim,const World& world) const {
    if (!victim.registered || victim.dead || !victim.visible || victim.guard || victim.castle_lord || victim.self) return false;
    if (victim.player) return world.base_can_attack(victim.id);
    return victim.monster && !victim.boss;
}
void zircon::HalloweenMonster::attack(Time now,const CombatWorld& combat) {attack(now,combat,interactions);}
void zircon::HalloweenMonster::attack(Time now,const CombatWorld& combat,const World& world) {
    const auto victim=world.object(target->id);
    if (!victim || !can_attack_target(*victim,world) || (now<=next_monster_kill && victim->monster)) {target.reset();return;}
    const auto d=MonsterPursuit::directions[victim->direction];
    world.broadcast_pose(victim->x-d[0],victim->y-d[1],victim->direction,false);
    world.broadcast_pose(x,y,direction,true);clock.update(now);attacks.push_back({victim->id,now+300});
}
void zircon::HalloweenMonster::process_actions(Time now,const CombatWorld& combat,const World& world) {
    for (std::size_t i=attacks.size();i>0;--i) {
        if (attacks[i-1].due>now) continue;
        const int id=attacks[i-1].target;attacks.erase(attacks.begin()+std::ptrdiff_t(i-1));
        const auto victim=world.object(id);if (!victim || !victim->registered || victim->dead) continue;
        const bool wild=victim->monster && !victim->pet;
        int power;
        if (wild) {world.clear_exp_owner(id);power=victim->hp;bonus_experience+=victim->experience;next_monster_kill=now+60000;}
        else power=combat.get_dc();
        world.attacked(id,power,Element::none);
        if (wild && combat.random(5)==0) world.teleport_random();
    }
}
void zircon::HalloweenMonster::activate(const std::function<void()>& add_active) {if (!activated) {activated=true;add_active();}}
void zircon::HalloweenMonster::deactivate() {}
void zircon::HalloweenMonster::die(const std::function<void()>& base,bool safe_zone,const CombatWorld& combat,const World& world) {
    base();if (safe_zone) return;
    if (combat.random(2)==0) {
        const auto choices=world.bosses();
        for (;;) {
            const auto [id,level]=choices[combat.random(unsigned(choices.size()))];
            if (level>=300) continue;
            world.spawn_boss_random(id,x,y,2);break;
        }
    } else {
        const auto objects=world.map_objects();
        for (auto i=objects.rbegin();i!=objects.rend();++i) {
            if (!i->monster || i->pet || i->guard || i->dead || i->move_delay==0 || !i->can_move || i->has_target || i->level>=300) continue;
            world.teleport_near(i->id,x,y,15);
        }
    }
}
bool zircon::MonsterObject::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,1);
}
void zircon::MonsterObject::process_target(Time now,const CombatWorld& world) {zircon_base_target(*this,now,world);}
void zircon::MonsterObject::attack(Time now,const CombatWorld& world) {zircon_normal_attack(*this,now,world);}
zircon::Shinsu::Shinsu(Time now) {visible=false;appear_at=now+1000;}
bool zircon::Shinsu::can_attack(bool base) const {return base && mode;}
bool zircon::Shinsu::can_be_seen(bool base) const {return visible && base;}
void zircon::Shinsu::process_target(Time now,const CombatWorld& world) {
    auto mode_world=world;mode_world.can_attack=[&]{return mode && world.can_attack();};
    MonsterObject::process_target(now,mode_world);
}
void zircon::Shinsu::spawned(Time now,const std::function<void()>& base,const std::function<void(Event)>& notify) {
    base();notify(Event::summon);clock.action_at=now+2000;
}
void zircon::Shinsu::process_actions(Time now,const std::function<void(Event)>& notify) {
    if (appearance_pending && now>=appear_at) {appearance_pending=false;visible=true;notify(Event::add_objects);}
}
void zircon::Shinsu::process(Time now,const std::function<void()>& base,const std::function<void(Event)>& notify) {
    if (!dead && now>clock.action_at) {
        if (target) mode_at=now+10000;
        if (!mode && now<mode_at) {mode=true;notify(Event::show);clock.action_at=now+2000;}
        else if (mode && now>mode_at) {mode=false;notify(Event::hide);clock.action_at=now+2000;}
    }
    base();
}
zircon::MirrorImage::MirrorImage(Time now) {explode_at=now+15000;}
bool zircon::MirrorImage::can_move() const {return false;}
bool zircon::MirrorImage::can_attack() const {return false;}
int zircon::MirrorImage::attacked() const {return 0;}
bool zircon::MirrorImage::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,view_range);
}
void zircon::MirrorImage::process(Time now,const std::function<void()>& base,const std::function<bool()>& owner_registered,
    const std::function<bool()>& owner_dead,const std::function<void()>& despawn,const std::function<void(int)>& set_hp) {
    base();
    if (!owner_registered()) {despawn();return;}
    if ((now>explode_at || owner_dead()) && !dead) {set_hp(0);return;}
    if (dead && spawned) set_hp(0);
}
void zircon::MirrorImage::process_target(Time now,const CombatWorld& world) {if (target) range_attack(now,world);}
void zircon::MirrorImage::range_attack(Time now,const CombatWorld& world) {
    if (now<cooldown) return;
    // 参考四种外观均使用Ice伤害；不按外观“修正”元素。
    switch (element) {
    case Element::fire:zircon_area_magic(x,y,*target,direction,clock,now,2,ZMagic::fire_storm,Element::ice,world.get_mc(),world);break;
    case Element::ice:zircon_area_magic(x,y,*target,direction,clock,now,2,ZMagic::ice_storm,Element::ice,world.get_mc(),world);break;
    case Element::lightning:zircon_area_magic(x,y,*target,direction,clock,now,2,ZMagic::chain_lightning,Element::ice,world.get_mc(),world);break;
    case Element::wind:zircon_area_magic(x,y,*target,direction,clock,now,2,ZMagic::dragon_tornado,Element::ice,world.get_mc(),world);break;
    default:break;
    }
    cooldown=now+1500;
}
void zircon::MirrorImage::activate(const std::function<void()>& add_active) {if (!activated) {activated=true;add_active();}}
void zircon::MirrorImage::die(Time now,const std::function<void()>& clear_master,const std::function<void()>& effect) {
    dead=true;clear_master();effect();dead_at=now;
}
void zircon::MirrorImage::despawned(const std::function<void()>& base,const std::function<void()>& remove_pet) {base();remove_pet();}
zircon::Puppet::Puppet(Time now) {explode_at=now+5000;}
bool zircon::Puppet::can_move() const {return false;}
bool zircon::Puppet::can_attack() const {return false;}
void zircon::Puppet::process_search() {}
void zircon::Puppet::process(Time now,const std::function<void()>& base,const std::function<bool()>& owner_registered,
    const std::function<bool()>& owner_dead,const std::function<void()>& despawn,const std::function<void(int)>& set_hp) {
    base();
    if (!owner_registered()) {despawn();return;}
    if (dead || now<explode_at || owner_dead()) return;
    set_hp(0);
}
void zircon::Puppet::process_target(const std::function<void(int)>& set_hp) {if (target) set_hp(0);}
void zircon::Puppet::process_target(Time,const CombatWorld& world) {process_target(world.set_hp);}
int zircon::Puppet::attacked(int power,AttackOptions options,const std::function<int(int,AttackOptions)>& base,const std::function<void(int)>& set_hp) {
    const int result=base(power,options);set_hp(0);return result;
}
void zircon::Puppet::activate(const std::function<void()>& add_active) {if (!activated) {activated=true;add_active();}}
void zircon::Puppet::deactivate() {}
void zircon::Puppet::despawned(const std::function<void()>& base,const std::function<void()>& remove_pet) {base();remove_pet();}
void zircon::Puppet::die(Time now,const std::function<void()>& base,const std::function<bool()>& owner_registered,
    const std::function<std::vector<MonsterAiTarget>(int,int,int)>& owner_targets,Element affinity,
    const std::function<void(Explosion)>& enqueue_on_owner,const std::function<void(Effect)>& broadcast) {
    base();
    if (!owner_registered()) return;
    for (const auto& victim:owner_targets(x,y,2))
        enqueue_on_owner({victim.id,now+800,ZumaAi::within(x,y,victim.x,victim.y,1),magics,dark_stone_stats,0});
    Effect effect=Effect::puppet;
    switch (affinity) {
    case Element::fire:effect=Effect::fire;break;
    case Element::ice:effect=Effect::ice;break;
    case Element::lightning:effect=Effect::lightning;break;
    case Element::wind:effect=Effect::wind;break;
    default:break;
    }
    broadcast(effect);dead_at=now+2000;
}
void zircon::EscortCommander::process_target(Time now,const CombatWorld& world) {
    if (!target) return;
    if (!in_attack_range()) {
        if (world.can_attack() && world.random(2)==0) range_attack(now,world);
        zircon_move_toward(*this,world);
    }
    if (!world.can_attack()) return;
    if (world.random(2)==0) {if (in_attack_range()) attack(now,world);}
    else range_attack(now,world);
}
void zircon::EscortCommander::range_attack(Time now,const CombatWorld& world) {
    if (!target) return;
    if (ZumaAi::within(x,y,target->x,target->y,2) && world.random(2)==0) {
        const int damage=world.get_dc();
        direction=zircon_direction(x,y,target->x,target->y);
        auto cast=std::make_shared<CombatWorld::MagicCast>();
        cast->direction=direction;cast->magic=ZMagic::monster_thunder_storm;cast->locations.emplace_back(x,y);
        world.broadcast_magic(cast);clock.update(now);
        for (int victim:world.get_targets(x,y,2)) world.enqueue({victim,damage,now+500,Element::lightning});
    } else if (ZumaAi::within(x,y,target->x,target->y,10))
        zircon_single_magic(x,y,*target,direction,clock,now,ZMagic::thunder_bolt,Element::lightning,false,world);
}
namespace {
using Conquest=zircon::ConquestWorld;
bool conquest_hidden(const Conquest::Object& victim,bool cool_eye,int level,int x,int y) {
    return (victim.invisible && !cool_eye) || victim.transparent ||
        (victim.cloak && (!cool_eye || !ZumaAi::within(x,y,victim.x,victim.y,2) || victim.level>=level));
}
// .NET Math.Round默认中点取偶；不使用std::round的中点远离零。
int castle_damage_level(int hp,int maximum) {
    const double value=3.0*hp/maximum;
    const int floor=int(std::floor(value));const double fraction=value-floor;
    return std::max(1,floor+int(fraction>0.5 || (fraction==0.5 && (floor&1))));
}
void castle_message(const zircon::CastleObject& object,Conquest::Event event,const Conquest& world,int value=0,
    std::optional<int> guild={},std::optional<int> other={}) {
    world.emit({event,object.id,object.x,object.y,object.direction,value,guild,other});
}
}
bool zircon::Guard::can_attack_target(const ConquestWorld::Object& victim,const ConquestWorld& world) const {
    if (!victim.registered || victim.dead || !victim.visible || victim.guard || victim.castle_lord) return false;
    if (victim.race==ConquestWorld::Race::player) return victim.pk>=red_point && victim.redemption==0;
    if (victim.race!=ConquestWorld::Race::monster) return false;
    if (!victim.pet_owner) return !victim.passive;
    const auto owner=world.object(*victim.pet_owner);
    return owner && owner->pk>=red_point && owner->redemption==0;
}
bool zircon::Guard::blocking() const {return true;}
bool zircon::Guard::can_move() const {return false;}
int zircon::Guard::attacked() const {return 0;}
std::uint32_t zircon::Guard::name_colour() const {return 0xFF87CEEB;}
void zircon::Guard::activate(const std::function<void()>& add_active) {if (!activated) {activated=true;add_active();}}
void zircon::Guard::deactivate() {}
void zircon::Guard::advance(Time now,const CombatWorld& combat,const ConquestWorld& world) {
    for (std::size_t i=pending_.size();i>0;--i) {
        if (pending_[i-1].due>now) continue;
        const int victim_id=pending_[i-1].target;pending_.erase(pending_.begin()+std::ptrdiff_t(i-1));
        const auto victim=world.object(victim_id);
        if (!victim || !victim->registered || victim->dead) continue;
        const bool monster=victim->race==ConquestWorld::Race::monster;
        if (monster) world.clear_exp_owner(victim_id);
        // Guard直接调用Attacked，绕过Attack的命中/AC阶段，怪物伤害在投递时取当前HP。
        world.attacked(victim_id,monster?victim->hp:combat.get_dc(),Element::none,{});
    }
    if (!target) return;
    const auto victim=world.object(*target);
    if (!victim || !can_attack_target(*victim,world)) {target.reset();return;}
    if (!victim->same_map || !ZumaAi::within(x,y,victim->x,victim->y,view_range) ||
        now<clock.attack_at || now<clock.action_at || !combat.can_attack()) return;
    const auto d=MonsterPursuit::directions[victim->direction];
    world.emit({ConquestWorld::Event::attack,id,victim->x-d[0],victim->y-d[1],victim->direction});
    world.emit({ConquestWorld::Event::turn,id,x,y,direction});
    clock.update(now);pending_.push_back({now+300,*target});
}
zircon::CastleObject::~CastleObject()=default;
bool zircon::CastleObject::can_move(bool base) const {return base;}
bool zircon::CastleObject::can_attack(bool base) const {return base;}
bool zircon::CastleObject::regenerates() const {return true;}
bool zircon::CastleObject::accepts_poison() const {return true;}
bool zircon::CastleGuard::can_move(bool) const {return false;}
bool zircon::CastleGate::can_move(bool) const {return false;}
bool zircon::CastleGate::can_attack(bool) const {return false;}
bool zircon::CastleGate::regenerates() const {return false;}
bool zircon::CastleFlag::can_move(bool) const {return false;}
bool zircon::CastleFlag::regenerates() const {return false;}
bool zircon::CastleFlag::accepts_poison() const {return false;}
bool zircon::CastleLord::regenerates() const {return false;}
bool zircon::CastleLord::accepts_poison() const {return false;}
std::unique_ptr<zircon::CastleObject> zircon::CastleObject::create(int ai) {
    switch (ai) {
    case 1001:return std::make_unique<CastleFlag>();
    case 1002:return std::make_unique<CastleGate>();
    case 1003:return std::make_unique<CastleGuard>();
    default:throw std::invalid_argument("unsupported castle AI");
    }
}
void zircon::CastleObject::spawned(const ConquestWorld& world) {
    if (!castle) throw std::invalid_argument("cannot spawn objective without a castle");
    guild=world.castle_owner(*castle);
}
void zircon::CastleObject::refresh(const ConquestWorld&) {}
void zircon::CastleObject::process(Time,const CombatWorld&,const ConquestWorld& world) {
    const auto owner=world.castle_owner(*castle);
    if (owner!=guild) {guild=owner;refresh(world);}
}
bool zircon::CastleObject::participant(std::optional<int> candidate) const {
    return candidate && std::find(participants.begin(),participants.end(),*candidate)!=participants.end();
}
bool zircon::CastleObject::can_attack_target(const ConquestWorld::Object& victim) const {
    return victim.id!=id && victim.registered && !victim.dead && victim.visible && war_castle &&
        victim.race==ConquestWorld::Race::player && victim.guild_castle!=war_castle;
}
bool zircon::CastleObject::should_attack_target(const ConquestWorld::Object& victim) const {
    return can_attack_target(victim) && !conquest_hidden(victim,cool_eye,level,x,y);
}
int zircon::CastleObject::attacked(const ConquestWorld::Object&,int power,Element element,AttackOptions options,const ConquestWorld& world) {
    return world.attacked(id,power,element,options);
}
void zircon::CastleGuard::process(Time now,const CombatWorld& combat,const ConquestWorld& world) {
    if (!dead && target && now>=clock.attack_at && now>=clock.action_at && combat.can_attack()) {
        const auto victim=world.object(*target);
        if (!victim || !can_attack_target(*victim)) target.reset();
        else if (victim->same_map && ZumaAi::within(x,y,victim->x,victim->y,attack_range) &&
            !victim->guild_castle && (participants.empty() || participant(victim->guild))) {
            direction=zircon_direction(x,y,victim->x,victim->y);
            combat.broadcast(direction,true,victim->id);clock.update(now);
            combat.enqueue({victim->id,combat.get_dc(),now+400+48*std::max(std::abs(x-victim->x),std::abs(y-victim->y)),attack_element});
        }
    }
    CastleObject::process(now,combat,world);
}
int zircon::CastleGuard::attacked(const ConquestWorld::Object& source,int power,Element element,AttackOptions options,const ConquestWorld& world) {
    if (!war_castle || source.race!=ConquestWorld::Race::player || source.guild==guild || !participant(source.guild)) return 0;
    return CastleObject::attacked(source,power,element,options,world);
}
void zircon::CastleGuard::repair(const ConquestWorld& world) {
    const bool revive=hp<=0;if (revive) dead=false;
    hp=maximum_hp;world.set_health(id,hp,revive);
    if (revive) castle_message(*this,ConquestWorld::Event::revive,world);
}
void zircon::CastleGate::spawn_blockers(int face,const ConquestWorld& world) {
    std::vector<std::pair<int,int>> offsets;
    switch (face) {
    case 1:offsets={{1,0},{0,1},{-1,-1},{-1,0},{0,-1}};break;
    case 2:offsets={{1,-1},{-1,0},{0,-1},{0,1},{1,0}};break;
    case 3:offsets={{-1,-1},{-1,0},{0,1},{1,0},{0,-1}};break;
    }
    direction=0;
    for (auto [dx,dy]:offsets) if (auto blocker=world.spawn_blocker(id,x+dx,y+dy,maximum_hp)) blockers.push_back(*blocker);
}
bool zircon::CastleGate::blocking(bool base) const {return closed && base;}
void zircon::CastleGate::wall(bool show,const ConquestWorld& world) {for (int blocker:blockers) world.show_blocker(blocker,show);}
void zircon::CastleGate::open(const ConquestWorld& world) {
    if (!closed) return;
    direction=7;closed=false;castle_message(*this,ConquestWorld::Event::attack,world);wall(false,world);
}
void zircon::CastleGate::close(const ConquestWorld& world) {
    if (closed) return;
    direction=3-castle_damage_level(hp,maximum_hp);closed=true;
    castle_message(*this,ConquestWorld::Event::range_attack,world);wall(true,world);
}
void zircon::CastleGate::check_direction(const ConquestWorld& world) {
    const int next=3-castle_damage_level(hp,maximum_hp);
    if (direction!=next) {direction=next;castle_message(*this,ConquestWorld::Event::turn,world);}
}
int zircon::CastleGate::attacked(const ConquestWorld::Object& source,int power,Element element,AttackOptions options,const ConquestWorld& world) {
    if (!closed) return 0;
    check_direction(world); // 参考在扣血之前检查，不在此补写攻城名单限制。
    return CastleObject::attacked(source,power,element,options,world);
}
void zircon::CastleGate::repair(const ConquestWorld& world) {
    const bool revive=hp<=0;if (revive) dead=false;
    hp=maximum_hp;world.set_health(id,hp,revive);
    if (revive) castle_message(*this,ConquestWorld::Event::revive,world);
    check_direction(world);wall(closed,world);
}
void zircon::CastleGate::die(const ConquestWorld& world) {
    dead=true;wall(false,world);castle_message(*this,ConquestWorld::Event::keep_corpse_one_year,world);
}
void zircon::CastleGate::process(Time now,const CombatWorld& combat,const ConquestWorld& world) {
    if (!dead) {
        const auto players=world.players();
        if (now>=search_at && !players.empty()) {
            search_at=now+search_delay;
            if (closed && auto_open && !war_castle) for (const auto& player:players) {
                if (ZumaAi::within(x,y,player.x,player.y,4) && player.guild==guild) {open(world);close_at=now+10000;break;}
            }
        }
        if (!closed && close_at>0 && close_at<now && std::none_of(players.begin(),players.end(),[&](const auto& player){
            return ZumaAi::within(x,y,player.x,player.y,4);
        })) {close(world);close_at=0;}
    }
    CastleObject::process(now,combat,world);
}
void zircon::CastleFlag::spawned(const ConquestWorld& world) {CastleObject::spawned(world);refresh(world);}
void zircon::CastleFlag::refresh(const ConquestWorld& world) {castle_message(*this,ConquestWorld::Event::refresh_flag,world,0,guild);}
int zircon::CastleFlag::attacked(const ConquestWorld::Object&,int,Element,AttackOptions,const ConquestWorld&) {return 0;}
void zircon::CastleFlag::cancel(const ConquestWorld& world) {
    if (contester) castle_message(*this,ConquestWorld::Event::stopped_taking,world,0,contester);
    contester.reset();contest_at=0;
}
void zircon::CastleFlag::process(Time now,const CombatWorld& combat,const ConquestWorld& world) {
    const auto victim=target?world.object(*target):std::nullopt;
    const bool in_range=victim && victim->same_map && ZumaAi::within(x,y,victim->x,victim->y,view_range);
    if (!victim || !can_attack_target(*victim)) target.reset();
    if (!dead && target && in_range && now>=clock.attack_at && now>=clock.action_at && combat.can_attack()) {
        clock.update(now);
        if (victim->guild && !victim->guild_castle && (participants.empty() || participant(victim->guild))) {
            if (!contester) {
                contester=victim->guild;contest_at=now+contest_delay;
                castle_message(*this,ConquestWorld::Event::taking_flag,world,30,contester);
            } else {
                bool contest_near=false,interrupted=false;
                for (const auto& player:world.players()) {
                    if (!ZumaAi::within(x,y,player.x,player.y,view_range) || !player.guild) continue;
                    // 不合资格玩家会提前结束本次Attack，而非跳过该玩家。
                    if (!participants.empty() && !participant(player.guild)) {interrupted=true;break;}
                    if (player.guild!=contester) {
                        contest_at=now+contest_delay;
                        castle_message(*this,ConquestWorld::Event::preventing_flag,world,0,contester,player.guild);
                        interrupted=true;break;
                    }
                    contest_near=true;
                }
                if (!interrupted) {
                    if (!contest_near) cancel(world);
                    else {
                        // TimeSpan.Seconds是秒分量，保留负值及一分钟取余语义。
                        castle_message(*this,ConquestWorld::Event::taking_flag,world,int((contest_at-now)/1000%60),contester);
                        if (contest_at<=now) {
                            world.set_castle_owner(*war_castle,contester);
                            castle_message(*this,ConquestWorld::Event::capture,world,0,contester);
                            contester.reset();
                        }
                    }
                }
            }
        }
    }
    CastleObject::process(now,combat,world);
    if (target && !in_range) target.reset();
    if (!target && contester) cancel(world);
}
bool zircon::CastleLord::can_attack_target(const ConquestWorld::Object& victim) const {
    return CastleObject::can_attack_target(victim) && !victim.guard && !victim.game_master;
}
bool zircon::CastleLord::should_attack_target(const ConquestWorld::Object& victim) const {
    return !passive && !victim.castle_lord && can_attack_target(victim) && !conquest_hidden(victim,cool_eye,level,x,y);
}
int zircon::CastleLord::attacked(const ConquestWorld::Object& source,int,Element element,AttackOptions options,const ConquestWorld& world) {
    if (source.race!=ConquestWorld::Race::player || !war_castle || !source.guild || source.guild_castle ||
        (!participants.empty() && !participant(source.guild))) return 0;
    options.struck=true; // 参考调用没有转发canStruck参数。
    const int damage=CastleObject::attacked(source,1,element,options,world);
    world.conquest_stats(source.id,damage,false);exp_owner.reset();world.clear_exp_owner(id);
    return damage;
}
bool zircon::CastleLord::die(Time now,const ConquestWorld& world,const std::function<void()>& base_die) {
    if (war_castle) {
        const auto owner=exp_owner?world.object(*exp_owner):std::nullopt;
        if (!owner || !owner->registered || !owner->guild || owner->guild_castle ||
            (!participants.empty() && !participant(owner->guild))) return false;
        world.conquest_stats(owner->id,0,true);
        world.set_castle_owner(*war_castle,owner->guild);
        castle_message(*this,ConquestWorld::Event::capture,world,0,owner->guild);
        world.finish_lord_capture(*war_castle,now);
        war_castle.reset();
    }
    base_die();dead=true;return true;
}
void zircon::CastleLord::process(Time now,const CombatWorld& combat,const ConquestWorld& world) {
    for (std::size_t i=clouds_.size();i>0;--i) {
        if (clouds_[i-1].due>now) continue;
        const auto cloud=clouds_[i-1];clouds_.erase(clouds_.begin()+std::ptrdiff_t(i-1));
        world.spawn_death_cloud(id,cloud.x,cloud.y,cloud.display_x,cloud.display_y,cloud.visible,now+4000);
    }
    const auto process_target=[&] {
        if (dead || !target) return;
        const auto victim=world.object(*target);
        if (!victim || !can_attack_target(*victim)) {target.reset();return;}
        const auto can_attack=[&]{return now>=clock.attack_at && now>=clock.action_at && combat.can_attack();};
        if (can_attack() && now>cast_at) {
            const auto victims=combat.get_targets(x,y,view_range);
            if (!victims.empty()) {
                for (int victim_id:victims) {
                    if (hp>maximum_hp/2 && combat.random(2)>0) continue;
                    const auto [tx,ty]=combat.target_position(victim_id);bool visible=true;
                    for (const auto& cell:combat.cells_in_range(tx,ty,2)) {
                        clouds_.push_back({now+500,cell.x,cell.y,tx,ty,visible});visible=false;
                    }
                }
                clock.update(now);
                auto cast=std::make_shared<CombatWorld::MagicCast>();
                cast->direction=direction;cast->magic=ZMagic::none;cast->targets.push_back(*target);
                combat.broadcast_magic(cast);cast_at=now+cast_delay;
            }
        }
        const int dx=std::abs(x-victim->x),dy=std::abs(y-victim->y);
        const bool in_range=victim->same_map && (dx||dy) && dx<=3 && dy<=3 && (dx==0 || dy==0 || dx==dy);
        if (!in_range) {
            if (!dx && !dy) zircon_walk_rotation(int(combat.random(8)),combat);
            else combat.move_to(victim->x,victim->y);
            return;
        }
        if (!can_attack()) return;
        direction=zircon_direction(x,y,victim->x,victim->y);
        const auto d=MonsterPursuit::directions[direction];
        if (combat.random(3)==0 || !ZumaAi::within(x,y,victim->x,victim->y,2)) {
            combat.broadcast(direction,true,0);
            for (int step=1;step<=3;++step) {
                const int tx=x+d[0]*step,ty=y+d[1]*step;
                if (tx==victim->x && ty==victim->y) combat.enqueue({victim->id,combat.get_dc(),now+400,attack_element});
                else if (const auto cell=combat.cell(tx,ty);cell && cell->objects) {
                    for (int other:*cell->objects) if (combat.can_attack_target(other)) {
                        combat.enqueue({other,combat.get_dc(),now+400,attack_element});break;
                    }
                }
            }
        } else {
            combat.broadcast(direction,false,0);
            for (int other:combat.get_targets(x+d[0],y+d[1],1)) {
                combat.enqueue({other,combat.get_dc(),now+400,attack_element});break;
            }
        }
        clock.update(now);
    };
    process_target();CastleObject::process(now,combat,world);
}
struct zircon::Controller::Impl {
    std::function<void(Position)> place;
    std::function<Position()> position;
    std::function<void(int,int,Element)> configure;
    std::function<void(Time,std::optional<MonsterAiTarget>,const CombatWorld&,const std::function<Position()>&)> step;
    std::vector<DelayedHit> pending;
    template<class Brain> void bind(RuntimeAI ai) {
        auto brain=std::make_shared<Brain>();
        if constexpr (std::is_same_v<Brain,SkeletonAxeThrower>) {
            if (ai==RuntimeAI::fear_axe) {brain->fear_rate=2;brain->fear_duration=4;}
            if (ai==RuntimeAI::long_axe) brain->attack_range=9;
        }
        if constexpr (std::is_same_v<Brain,WingedHorror>) brain->range_chance=1;
        place=[brain](Position p){brain->x=p.x;brain->y=p.y;brain->direction=p.direction;};
        position=[brain]{return Position{brain->x,brain->y,brain->direction};};
        configure=[brain](int attack,int move,Element element){
            brain->clock.attack_delay=attack;brain->clock.move_delay=move;brain->attack_element=element;
        };
        step=[this,brain](Time now,std::optional<MonsterAiTarget> target,const CombatWorld& host,
            const std::function<Position()>& live_position) {
            brain->target=target;
            const auto sync=[&]{const auto p=live_position();brain->x=p.x;brain->y=p.y;brain->direction=p.direction;};
            sync();
            auto world=host;
            world.can_attack=[&]{return now>=brain->clock.attack_at && now>=brain->clock.action_at && host.can_attack();};
            world.walk=[&](int direction){if(now<brain->clock.action_at) return false;const bool moved=host.walk(direction);sync();return moved;};
            world.move_to=[&](int x,int y){if(now<brain->clock.action_at) return;host.move_to(x,y);sync();};
            world.teleport=[&](int x,int y){host.teleport(x,y);sync();};
            world.enqueue=[this](DelayedHit hit){pending.push_back(hit);};
            brain->process_target(now,world);
        };
    }
};
zircon::Controller::Controller(RuntimeAI ai):impl_(std::make_unique<Impl>()) {
    switch(ai) {
    case RuntimeAI::axe:impl_->bind<SkeletonAxeThrower>(ai);break;
    case RuntimeAI::uma_king:impl_->bind<UmaKing>(ai);break;
    case RuntimeAI::fear_axe:impl_->bind<SkeletonAxeThrower>(ai);break;
    case RuntimeAI::numa_mage:impl_->bind<NumaMage>(ai);break;
    case RuntimeAI::axe_alt:impl_->bind<SkeletonAxeThrower>(ai);break;
    case RuntimeAI::long_axe:impl_->bind<SkeletonAxeThrower>(ai);break;
    case RuntimeAI::numa_mage_alt:impl_->bind<NumaMage>(ai);break;
    case RuntimeAI::banya_guard:impl_->bind<BanyaLeftGuard>(ai);break;
    case RuntimeAI::emperor:impl_->bind<EmperorSaWoo>(ai);break;
    case RuntimeAI::numa_high_mage:impl_->bind<NumaHighMage>(ai);break;
    case RuntimeAI::winged_horror:impl_->bind<WingedHorror>(ai);break;
    case RuntimeAI::escort_commander:impl_->bind<EscortCommander>(ai);break;
    case RuntimeAI::fiery_dancer:impl_->bind<FieryDancer>(ai);break;
    case RuntimeAI::fire_guardian:impl_->bind<SamaFireGuardian>(ai);break;
    case RuntimeAI::ice_guardian:impl_->bind<SamaIceGuardian>(ai);break;
    case RuntimeAI::lightning_guardian:impl_->bind<SamaLightningGuardian>(ai);break;
    case RuntimeAI::wind_guardian:impl_->bind<SamaWindGuardian>(ai);break;
    case RuntimeAI::phoenix:impl_->bind<SamaPhoenix>(ai);break;
    case RuntimeAI::black:impl_->bind<SamaBlack>(ai);break;
    case RuntimeAI::blue:impl_->bind<SamaBlue>(ai);break;
    case RuntimeAI::white:impl_->bind<SamaWhite>(ai);break;
    case RuntimeAI::oma_mage:impl_->bind<OmaMage>(ai);break;
    case RuntimeAI::pink_bat:impl_->bind<PinkBat>(ai);break;
    default:throw std::invalid_argument("unsupported reference monster AI");
    }
}
zircon::Controller::~Controller()=default;
zircon::CombatState::Result zircon::CombatState::resolve(CombatState& attacker,CombatState& victim,int power,
    Element element,Time now,bool attacker_registered,bool same_map,bool within_view,const MonsterPursuit::Random& random) {
    Result result;
    if (victim.dead) return result;
    if (attacker.abyss && random(2)>0) {result.outcome=Outcome::dodged;return result;}
    if (element==Element::none && int(random(unsigned(std::max(0,victim.agility))))>attacker.accuracy) {
        result.outcome=Outcome::dodged;return result;
    }
    const bool physical=element==Element::none;
    const int minimum=std::max(0,physical?victim.min_ac:victim.min_mr);
    const int maximum=physical?victim.max_ac:victim.max_mr;
    int armour=maximum;
    if (minimum<maximum && !(physical && victim.defensive_mastery>0 &&
        (victim.defensive_mastery>=10 || int(random(10))<victim.defensive_mastery)))
        armour=minimum+int(random(unsigned(maximum-minimum+1)));
    power-=armour;
    const int resistance=victim.resistance[unsigned(element)];
    if (resistance>0) power-=power*resistance/10;
    else if (resistance<0) power-=power*resistance/5;
    if (power<=0) {result.outcome=Outcome::blocked;return result;}
    // MonsterObject.Attacked的门控在Attack护甲/抗性抽样之后。
    if (!attacker_registered || !same_map || !within_view || victim.invincible) return result;
    victim.shock_at=0;
    if (!victim.struck_disabled && now>victim.struck_at+300) {victim.struck_at=now;result.struck=true;}
    if (victim.red_poison) power=int(float(power)*1.2f);
    for (int i=0;i<attacker.rebirth;++i) power=int(float(power)*1.5f);
    if (victim.magic_shield_buff) victim.shield_duration-=Time(power)*10;
    power-=power*victim.shield_percent/100;
    if (int(random(100))<attacker.critical_chance && power>0) {
        power+=power+power*attacker.critical_damage/100;result.critical=true;
    }
    if (victim.superior_shield_buff) {
        victim.superior_shield-=power;
        if (victim.superior_shield<=0) {victim.superior_shield_buff=false;result.shield_removed=true;}
    } else {
        int loss=power;
        if (loss>0 && victim.protection_ring) {const int consumed=std::min(victim.mp,loss);victim.mp-=consumed;loss-=consumed;}
        victim.hp=std::min(victim.maximum_hp,victim.hp-loss);
        if (victim.hp<=0) {victim.dead=true;result.killed=true;}
    }
    result.outcome=Outcome::hit;result.power=power;
    if (power<=0) return result;
    attacker.life_steal_hundredths+=std::int64_t(power)*attacker.life_steal;
    if (attacker.life_steal_hundredths>100) {
        const int heal=int(attacker.life_steal_hundredths/100);attacker.life_steal_hundredths%=100;
        if (!attacker.dead) {result.healed=std::min(heal,attacker.maximum_hp-attacker.hp);attacker.hp+=result.healed;}
    }
    return result;
}
bool zircon::Controller::supports(int ai) {
    switch(RuntimeAI(ai)) {
    case RuntimeAI::axe:
    case RuntimeAI::uma_king:
    case RuntimeAI::fear_axe:
    case RuntimeAI::numa_mage:
    case RuntimeAI::axe_alt:
    case RuntimeAI::long_axe:
    case RuntimeAI::numa_mage_alt:
    case RuntimeAI::banya_guard:
    case RuntimeAI::emperor:
    case RuntimeAI::numa_high_mage:
    case RuntimeAI::winged_horror:
    case RuntimeAI::escort_commander:
    case RuntimeAI::fiery_dancer:
    case RuntimeAI::fire_guardian:
    case RuntimeAI::ice_guardian:
    case RuntimeAI::lightning_guardian:
    case RuntimeAI::wind_guardian:
    case RuntimeAI::phoenix:
    case RuntimeAI::black:
    case RuntimeAI::blue:
    case RuntimeAI::white:
    case RuntimeAI::oma_mage:
    case RuntimeAI::pink_bat:
        return true;
    default:return false;
    }
}
void zircon::Controller::place(Position position) {impl_->place(position);}
zircon::Controller::Position zircon::Controller::position() const {return impl_->position();}
void zircon::Controller::configure(int attack_delay,int move_delay,Element element) {impl_->configure(attack_delay,move_delay,element);}
void zircon::Controller::advance(Time now,std::optional<MonsterAiTarget> target,bool alive,const CombatWorld& host,
    const std::function<Position()>& live_position,const std::function<void(DelayedHit)>& deliver) {
    // 每个对象倒序投递；死亡不抹掉已发出的攻击，删除对象才销毁队列。
    for (std::size_t i=impl_->pending.size();i>0;--i) {
        if (impl_->pending[i-1].due>now) continue;
        const auto hit=impl_->pending[i-1];impl_->pending.erase(impl_->pending.begin()+std::ptrdiff_t(i-1));deliver(hit);
    }
    // 到期攻击可能先杀死当前目标，不能把投递前的目标快照继续交给AI。
    if (target && !host.can_attack_target(target->id)) target.reset();
    if (alive) impl_->step(now,target,host,live_position);
}
void zircon::MonsterPurification::cast(int x,int y,const MonsterAiTarget& target,int& direction,AttackClock& clock,
    Time now,const CombatWorld& combat,const World& world) {
    direction=zircon_direction(x,y,target.x,target.y);
    auto cast=std::make_shared<CombatWorld::MagicCast>();cast->direction=direction;cast->magic=ZMagic::purification;
    cast->targets.push_back(target.id);combat.broadcast_magic(cast);clock.update(now);world.enqueue(target.id,now+500);
}
int zircon::MonsterPurification::deliver(int id,int caster_level,const MonsterPursuit::Random& random,const World& world) {
    const auto target=world.target(id);
    if (!target || !target->registered || target->dead) return 0;
    int result=0;
    std::vector<int> remove;
    if (world.can_help(id)) {
        const auto poisons=world.poisons(id);result+=int(poisons.size());
        for (auto it=poisons.rbegin();it!=poisons.rend();++it) if (!it->parasite) world.remove_poison(id,it->id);
        for (const auto& buff:world.buffs(id)) if (buff.type==BuffType::magic_weakness || buff.type==BuffType::defensive_blow) {
            remove.push_back(buff.id);++result;
        }
    }
    if (world.can_attack(id) && (target->level<=caster_level || random(100)<42)) {
        for (const auto& buff:world.buffs(id)) {
            switch (buff.type) {
            case BuffType::heal:case BuffType::invisibility:case BuffType::magic_resistance:case BuffType::resilience:
            case BuffType::magic_shield:case BuffType::superior_magic_shield:case BuffType::elemental_superiority:
            case BuffType::blood_lust:case BuffType::defiance:case BuffType::might:case BuffType::reflect_damage:
            case BuffType::judgement_of_heaven:case BuffType::strength_of_faith:case BuffType::celestial_light:
            case BuffType::transparency:case BuffType::renounce:case BuffType::cloak:case BuffType::ghost_walk:
            case BuffType::life_steal:case BuffType::dark_conversion:case BuffType::evasion:case BuffType::raging_wind:
            case BuffType::invincibility:remove.push_back(buff.id);++result;break;
            default:break;
            }
        }
    }
    for (auto it=remove.rbegin();it!=remove.rend();++it) world.remove_buff(id,*it);
    return result;
}
bool zircon::BanyoCaptain::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,1);
}
void zircon::BanyoCaptain::process_target(Time now,const CombatWorld& combat,const MonsterPurification::World& purification) {
    if (!target) return;
    if (combat.can_attack() && now>cast_at) {
        cast_at=now+3000;
        if (combat.random(5)==0) MonsterPurification::cast(x,y,*target,direction,clock,now,combat,purification);
        else {
            const auto distance=std::max(std::abs(std::int64_t(x)-target->x),std::abs(std::int64_t(y)-target->y));
            const int damage=100+(distance>2?50:0)+(distance>4?50:0);
            direction=zircon_direction(x,y,target->x,target->y);
            auto cast=std::make_shared<CombatWorld::MagicCast>();cast->direction=direction;cast->magic=ZMagic::thunder_bolt;
            cast->targets.push_back(target->id);combat.broadcast_magic(cast);clock.update(now);
            combat.enqueue({target->id,damage,now+500,Element::lightning});
        }
    }
    zircon_base_target(*this,now,combat);
}
void zircon::BanyoCaptain::attack(Time now,const CombatWorld& combat) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);combat.broadcast(direction,false,target->id);clock.update(now);
    int damage=combat.get_dc();if (combat.random(10)==0) damage*=2;
    combat.enqueue({target->id,damage,now+400,attack_element});
}
void zircon::MonsterFireWall::cast(int x,int y,const MonsterAiTarget& target,int& direction,AttackClock& clock,
    Time now,const CombatWorld& combat,const World& world) {
    zircon_begin_magic(x,y,target,direction,clock,now,ZMagic::fire_wall,combat);
    const auto targets=combat.get_targets(x,y,20);
    if (targets.empty()) return;
    const auto [tx,ty]=combat.target_position(targets[combat.random(unsigned(targets.size()))]);
    // 保留Cell引用所属地图与无效Cell，不能在投递时改用怪物届时所在地图。
    world.enqueue({world.current_map_cell(tx,ty),now+500});
    for (int dir:{0,4,6,2}) {
        const auto delta=MonsterPursuit::directions[dir];
        world.enqueue({world.current_map_cell(tx+delta[0],ty+delta[1]),now+500});
    }
}
void zircon::MonsterFireWall::deliver(Pending pending,const World& world) {
    if (!pending.cell) return;
    auto objects=world.objects(*pending.cell);
    for (auto it=objects.rbegin();it!=objects.rend();++it)
        if (it->spell && (it->effect==Effect::fire_wall || it->effect==Effect::tempest)) world.despawn(it->id);
    world.spawn({*pending.cell,15,2000});
}
zircon::ZumaGuardian::~ZumaGuardian()=default;
zircon::ZumaKing::ZumaKing() {avoid_firewall=false;}
void zircon::ZumaKing::spawned(const std::function<void()>& base) {base();stage=max_stage;}
void zircon::ZumaKing::wake(Time now,const std::function<void(ZumaGuardian&)>& show) {
    ZumaGuardian::wake(now,show);action_at=now+2000;clock.action_at=action_at;
}
void zircon::ZumaKing::process(Time now,const std::function<void()>& base,
    const std::function<std::vector<ZumaGuardian*>(int,int,int)>& nearby,
    const std::function<void(ZumaGuardian&)>& show,
    const std::function<void(int,int,std::optional<MonsterAiTarget>)>& spawn_minions) {
    ZumaGuardian::process(now,base,nearby,show);
    if (dead) return;
    if (maximum_hp<=0 || std::int64_t(hp)*max_stage/maximum_hp>=stage || stage<=0) return;
    --stage;spawn_minions(4,8,target); // 每次Process只下降一个阶段。
}
bool zircon::ZumaKing::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,1);
}
void zircon::ZumaKing::process_target(Time now,const CombatWorld& world,const MonsterFireWall::World& fire_wall) {
    if (!target) return;
    if (!in_attack_range()) {
        if (visible && world.can_attack() && world.random(5)==0) range_attack(now,world,fire_wall);
        zircon_move_toward(*this,world);
    }
    if (!visible || !world.can_attack()) return;
    if (world.random(5)>0) {if (in_attack_range()) attack(now,world);}
    else range_attack(now,world,fire_wall);
}
void zircon::ZumaKing::range_attack(Time now,const CombatWorld& world,const MonsterFireWall::World& fire_wall) {
    if (!target) return;
    if (world.random(3)==0) MonsterFireWall::cast(x,y,*target,direction,clock,now,world,fire_wall);
    else zircon_line_aoe(x,y,*target,direction,clock,now,12,-2,2,ZMagic::monster_scortched_earth,Element::fire,world);
    action_at=clock.action_at;
}
void zircon::ZumaKing::attack(Time now,const CombatWorld& world) {
    zircon_normal_attack(*this,now,world);action_at=clock.action_at;
}
int zircon::OmaWarlord::attacked(int power,AttackOptions options,const std::function<int(int,AttackOptions)>& base,
    const std::function<void(int,int)>& teleport_nearby) {
    options.struck=true;
    const int result=base(power,options);
    if (result<0 || dead || !can_teleport || hp>maximum_hp/2) return result;
    can_teleport=false;teleport_nearby(7,12);return result;
}
bool zircon::OmaWarlord::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,1);
}
void zircon::OmaWarlord::process_target(Time now,const CombatWorld& world) {
    if (!target) return;
    if (!ZumaAi::within(x,y,target->x,target->y,1) && now>teleport_at && world.can_attack()) {
        teleport_at=now+1000;
        if (world.random(7)==0 && zircon_teleport_to_target(x,y,*target,direction,world)) bonus=true;
    }
    zircon_base_target(*this,now,world);
}
void zircon::OmaWarlord::attack(Time now,const CombatWorld& world) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);world.broadcast(direction,false,target->id);clock.update(now);
    int damage=world.get_dc();if (bonus) damage*=2;
    world.enqueue({target->id,damage,now+400,attack_element});bonus=false;
}
bool zircon::NumaWarlord::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,2);
}
std::optional<std::pair<int,int>> zircon::NumaWarlord::teleport_cell(int id,const CombatWorld& combat,const World& world) const {
    const auto victim=world.target_state(id);
    if (!victim || !victim->registered || victim->dead || !victim->same_map) return std::nullopt;
    const int facing=zircon_direction(x,y,victim->x,victim->y);
    const int start=int(combat.random(8)),rotation=combat.random(2)==0?1:-1;
    for (int i=0;i<8;++i) {
        const auto delta=MonsterPursuit::directions[((facing+start+i*rotation)%8+8)%8];
        const int tx=victim->x+delta[0],ty=victim->y+delta[1];
        if (combat.teleport_cell_available(tx,ty)) return std::pair{tx,ty};
    }
    return std::nullopt;
}
void zircon::NumaWarlord::process_target(Time now,const CombatWorld& combat,const World& world) {
    if (!target) return;
    const auto distance=std::max(std::abs(std::int64_t(x)-target->x),std::abs(std::int64_t(y)-target->y));
    if (combat.can_attack() && distance>4 && now>=teleport_at && teleport_cell(target->id,combat,world)) {
        teleport_attack(now,combat,world);return;
    }
    if (combat.can_attack() && distance>2 && distance<=5 && now>=pull_at && pull_attack(now,combat,world)) return;
    if (!in_attack_range()) {zircon_move_toward(*this,combat);return;}
    if (combat.can_attack()) swing_attack(now,combat,world);
}
void zircon::NumaWarlord::teleport_attack(Time now,const CombatWorld& combat,const World& world) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);combat.broadcast(direction,false,target->id);
    clock.update(now);teleport_at=now+6000;world.enqueue({Action::teleport,{target->id},now+500});
}
void zircon::NumaWarlord::swing_attack(Time now,const CombatWorld& combat,const World& world) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);
    const auto delta=MonsterPursuit::directions[direction];
    const auto targets=combat.get_targets(x+delta[0],y+delta[1],1);
    world.broadcast_range(direction,targets);clock.update(now);
    for (int id:targets) combat.enqueue({id,combat.get_dc(),now+400,attack_element});
}
bool zircon::NumaWarlord::pull_attack(Time now,const CombatWorld& combat,const World& world) {
    if (!target) return false;
    auto targets=combat.get_targets(x,y,5);
    if (targets.empty()) return false;
    direction=zircon_direction(x,y,target->x,target->y);
    auto cast=std::make_shared<CombatWorld::MagicCast>();cast->direction=direction;cast->magic=ZMagic::none;cast->targets=targets;
    combat.broadcast_magic(cast);clock.update(now);pull_at=now+10000;
    world.enqueue({Action::pull,std::move(targets),now+400});return true;
}
void zircon::NumaWarlord::deliver(const Pending& pending,const CombatWorld& combat,const World& world) {
    if (dead) return;
    if (pending.action==Action::teleport) {
        if (pending.targets.empty()) return;
        const int id=pending.targets.front();
        if (!combat.can_attack_target(id)) return;
        auto victim=world.target_state(id);
        if (!victim || !victim->same_map) return;
        const auto cell=teleport_cell(id,combat,world); // 预检查结果不缓存，到期重新随机选格。
        if (!cell) return;
        victim=world.target_state(id);if (!victim) return;
        direction=zircon_direction(cell->first,cell->second,victim->x,victim->y);
        world.teleport(cell->first,cell->second,false,false);return;
    }
    for (int id:pending.targets) {
        if (!combat.can_attack_target(id)) continue;
        auto victim=world.target_state(id);
        if (!victim || !victim->same_map || !ZumaAi::within(x,y,victim->x,victim->y,5)) continue;
        world.attack(id,std::max(1,combat.get_dc()/2),attack_element);
        victim=world.target_state(id);
        if (!victim || victim->dead || !victim->same_map) continue;
        const auto distance=std::max(std::abs(std::int64_t(x)-victim->x),std::abs(std::int64_t(y)-victim->y));
        const int pull_distance=int(std::min<std::int64_t>(3,distance-1));
        if (pull_distance>0) world.pushed(id,zircon_direction(victim->x,victim->y,x,y),pull_distance);
    }
}
void zircon::QuartzTurtleSub::process(Time now,const std::function<void()>& base,const std::function<int()>& minion_count,
    const std::function<void(int,int,std::optional<MonsterAiTarget>)>& spawn_minions) {
    base();if (dead) return;
    if (now>=slave_at) {slave_at=now+20000;spawn_minions(10-minion_count(),0,target);}
}
bool zircon::QuartzTurtleSub::spawn_minion(int id,const std::function<bool(int,int,int,int)>& spawn_random) {
    return spawn_random(id,x,y,10);
}
void zircon::Affliction::attack(const std::function<void(int)>& set_hp) {set_hp(0);}
void zircon::Affliction::die(Time now,const CombatWorld& world,const std::function<void()>& base) {
    base();for (int id:world.get_targets(x,y,1)) world.enqueue({id,world.get_dc(),now+800,attack_element});
}
void zircon::InfernalSoldier::spawned(Time now,const std::function<void()>& base,const std::function<void()>& show) {
    base();clock.action_at=now+2000;show();
}
bool zircon::InfernalSoldier::can_attack_target(const std::function<bool()>& base) const {return base();}
bool zircon::InfernalSoldier::in_attack_range(Time now) const {
    if (!target || !same_map || (x==target->x && y==target->y)) return false;
    const int range=now>range_at && (can_pvp_range || !target_is_player)?attack_range:1;
    return ZumaAi::within(x,y,target->x,target->y,range);
}
void zircon::InfernalSoldier::process_target(Time now,const CombatWorld& world,const std::function<void(PendingHit)>& enqueue) {
    if (!target) return;
    if (in_attack_range(now) && world.can_attack()) attack(now,world,enqueue);
    zircon_move_toward(*this,world);
    if (in_attack_range(now) && world.can_attack()) attack(now,world,enqueue);
}
void zircon::InfernalSoldier::attack(Time now,const CombatWorld& world,const std::function<void(PendingHit)>& enqueue) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);clock.update(now);
    const bool melee=ZumaAi::within(x,y,target->x,target->y,1);
    if (!melee) range_at=now+range_cooldown;
    world.broadcast(direction,!melee,target->id);enqueue({target->id,melee,now+400});
}
void zircon::InfernalSoldier::deliver(PendingHit hit,const std::function<int()>& get_dc,
    const std::function<void(int,int,Element)>& base_attack) {
    base_attack(hit.target,get_dc(),attack_element);
}
void zircon::Larva::process(const std::function<void()>& base,const std::function<void(int)>& set_hp) {
    base();if (!dead && !target) set_hp(0);
}
void zircon::Larva::attack(const std::function<void(int)>& set_hp) {set_hp(0);}
void zircon::Larva::die(Time now,const CombatWorld& world,const std::function<void()>& base) {
    base();
    for (int id:world.get_targets(x,y,range)) world.enqueue({id,world.get_dc(),now+800,attack_element});
}
zircon::Terracotta::~Terracotta()=default;
bool zircon::Terracotta::can_attack(bool base) const {return visible && base;}
bool zircon::Terracotta::blocking(bool base) const {return visible && base;}
bool zircon::Terracotta::can_be_seen(const std::function<bool()>& base) const {return visible && base();}
bool zircon::Terracotta::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,1);
}
void zircon::Terracotta::phase_in(Time now,const std::function<void(Event)>& notify) {
    visible=true;cell_at=now+500;clock.action_at=now+1000;
    notify(Event::add_all_objects);notify(Event::show);
}
void zircon::Terracotta::phase_out(Time now,const std::function<void(Event)>& notify) {
    if (phase_at>now) return;
    phase_at=now+5000;notify(Event::hide);hide_actions_.push_back(now+900);
    move_at=now+1200;clock.action_at=now+1200;
}
void zircon::Terracotta::process_actions(Time now) {
    for (std::size_t i=hide_actions_.size();i>0;--i) if (now>=hide_actions_[i-1]) {
        visible=false;hide_actions_.erase(hide_actions_.begin()+std::ptrdiff_t(i-1));
    }
}
void zircon::Terracotta::update_move_time(Time now,const std::function<void()>& base) {
    if (!visible) {move_at=now+20;clock.action_at=now+20;}
    else base();
}
void zircon::Terracotta::process_target(Time now,const CombatWorld& world,const std::function<void(Event)>& notify) {
    if (!target) return;
    if (x==target->x && y==target->y) zircon_walk_rotation(int(world.random(8)),world);
    else if (can_phase && visible && !ZumaAi::within(x,y,target->x,target->y,4) && world.random(45)==0)
        phase_out(now,notify);
    else world.move_to(target->x,target->y);
    if (ZumaAi::within(x,y,target->x,target->y,2) && !visible) {phase_in(now,notify);return;}
    if (in_attack_range() && visible && world.can_attack()) choose_attack(now,world);
}
void zircon::Terracotta::choose_attack(Time now,const CombatWorld& world) {attack(now,world);}
void zircon::Terracotta::attack(Time now,const CombatWorld& world) {zircon_normal_attack(*this,now,world);}
zircon::TerracottaSub::TerracottaSub() {can_phase=true;}
void zircon::TerracottaSub::choose_attack(Time now,const CombatWorld& world) {
    if (world.random(2)==0) attack(now,world);else attack2(now,world);
}
void zircon::TerracottaSub::attack(Time now,const CombatWorld& world) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);world.broadcast(direction,false,target->id);clock.update(now);
    for (int radius=1;radius<=2;++radius) for (int offset=-1;offset<=1;++offset) {
        const auto delta=MonsterPursuit::directions[(direction+offset+8)%8];
        for (int id:world.get_targets(x+delta[0]*radius,y+delta[1]*radius,0))
            world.enqueue({id,world.get_dc(),now+400,attack_element});
    }
}
void zircon::TerracottaSub::attack2(Time now,const CombatWorld& world) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);
    auto cast=std::make_shared<CombatWorld::MagicCast>();cast->direction=direction;cast->magic=ZMagic::none;
    world.broadcast_magic(cast);clock.update(now);
    const auto delta=MonsterPursuit::directions[direction];
    for (int id:world.get_targets(x+delta[0],y+delta[1],1)) world.enqueue({id,world.get_dc(),now+400,attack_element});
}
bool zircon::TerracottaBoss::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,attack_range);
}
bool zircon::TerracottaBoss::in_melee_attack_range() const {
    if (!in_attack_range()) return false;
    const auto dx=std::abs(std::int64_t(target->x)-x),dy=std::abs(std::int64_t(target->y)-y);
    return dx<=2 && dy<=2 && (dx==0 || dy==0 || dx==dy);
}
void zircon::TerracottaBoss::process_target(Time now,const CombatWorld& world) {
    if (!target) return;
    if (x==target->x && y==target->y) zircon_walk_rotation(int(world.random(8)),world);
    else if (!in_melee_attack_range()) world.move_to(target->x,target->y);
    if (in_attack_range() && world.can_attack()) attack(now,world);
}
void zircon::TerracottaBoss::attack(Time now,const CombatWorld& world) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);clock.update(now);
    if (!in_melee_attack_range() || world.random(5)>0)
        zircon_line_aoe(x,y,*target,direction,clock,now,12,1,1,ZMagic::none,Element::dark,world);
    else {
        world.broadcast(direction,false,target->id);
        const auto delta=MonsterPursuit::directions[direction];
        for (int id:world.get_targets(x+delta[0]*3,y+delta[1]*3,2))
            world.enqueue({id,world.get_mc(),now+400,attack_element});
    }
}
bool zircon::FlameDemon::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,1);
}
void zircon::FlameDemon::process_target(Time now,const CombatWorld& world) {
    if (!target) return;
    if ((cast<=0 || now>cast_at) && world.can_attack() && ZumaAi::within(x,y,target->x,target->y,8)) {
        cast=3;cast_at=now+5000;
        zircon_line_aoe(x,y,*target,direction,clock,now,12,min,max,ZMagic::monster_scortched_earth,Element::fire,world);
    }
    zircon_base_target(*this,now,world);
}
void zircon::FlameDemon::attack(Time now,const CombatWorld& world) {--cast;zircon_normal_attack(*this,now,world);}
bool zircon::CorrosivePoisonSpitter::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,1);
}
void zircon::CorrosivePoisonSpitter::process_target(Time now,const CombatWorld& world) {
    if (!target) return;
    if (!ZumaAi::within(x,y,target->x,target->y,3) && now>teleport_at &&
        zircon_teleport_to_target(x,y,*target,direction,world)) teleport_at=now+5000;
    zircon_base_target(*this,now,world);
}
void zircon::CorrosivePoisonSpitter::attack(Time now,const CombatWorld& world) {zircon_normal_attack(*this,now,world);}
void zircon::EvilElephant::attack(Time now,const MonsterAiTarget& target,const CombatWorld& world,
    const std::function<void(PendingHit)>& enqueue) {
    direction=zircon_direction(x,y,target.x,target.y);clock.update(now);
    if (world.random(6)>0) {
        world.broadcast(direction,false,target.id);enqueue({target.id,false,now+400});
    } else {
        world.broadcast(direction,true,0);
        for (int id:world.get_targets(x,y,1)) enqueue({id,true,now+400});
    }
}
void zircon::EvilElephant::deliver(PendingHit hit,const std::function<int()>& get_dc,const std::function<int()>& get_sc,
    const std::function<int(int,int,Element)>& base_attack,const std::function<void(int,GreenPoison)>& apply_poison) {
    const int damage=base_attack(hit.target,get_dc(),attack_element);
    if (hit.apply_poison && damage>0) apply_poison(hit.target,{get_sc(),2000,10});
}
zircon::BanyaLeftGuard::~BanyaLeftGuard()=default;
bool zircon::BanyaLeftGuard::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,1);
}
void zircon::BanyaLeftGuard::process_target(Time now,const CombatWorld& world) {zircon_mage_target(*this,now,world);}
void zircon::BanyaLeftGuard::attack(Time now,const CombatWorld& world) {zircon_normal_attack(*this,now,world);}
void zircon::BanyaLeftGuard::range_attack(Time now,const CombatWorld& world) {
    if (target && ZumaAi::within(x,y,target->x,target->y,10))
        zircon_single_magic(x,y,*target,direction,clock,now,ZMagic::fire_ball,Element::fire,true,world);
}
int zircon::SunFeralWarrior::attacked(int power,AttackOptions options,const std::function<int(int,AttackOptions)>& base,
    const std::function<void(int,int,std::optional<MonsterAiTarget>)>& spawn_minions) {
    options.struck=true;
    const int result=base(power,options);
    if (!can_spawn || result<=0) return result;
    can_spawn=false;spawn_minions(2,6,std::nullopt);return result;
}
void zircon::SunFeralWarrior::range_attack(Time now,const CombatWorld& world) {
    if (!target || !ZumaAi::within(x,y,target->x,target->y,10)) return;
    if (world.random(2)==1) zircon_single_magic(x,y,*target,direction,clock,now,ZMagic::fire_ball,Element::fire,true,world);
    else {
        direction=zircon_direction(x,y,target->x,target->y);
        auto cast=std::make_shared<CombatWorld::MagicCast>();cast->direction=direction;cast->magic=ZMagic::fire_storm;
        cast->locations.emplace_back(target->x,target->y);world.broadcast_magic(cast);clock.update(now);
        for (int id:world.get_targets(target->x,target->y,1)) world.enqueue({id,world.get_dc(),now+500,Element::fire});
    }
}
void zircon::MoonFeralWarrior::process_target(Time now,const CombatWorld& world) {
    if (!target) return;
    if (!ZumaAi::within(x,y,target->x,target->y,1) && now>teleport_at && world.can_attack()) {
        teleport_at=now+1000;
        if (world.random(7)==0 && zircon_teleport_to_target(x,y,*target,direction,world)) bonus=true;
    }
    NumaMage::process_target(now,world);
}
void zircon::MoonFeralWarrior::attack(Time now,const CombatWorld& world) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);world.broadcast(direction,false,target->id);clock.update(now);
    int damage=world.get_dc();if (bonus) damage*=2;
    world.enqueue({target->id,damage,now+400,attack_element});bonus=false;
}
bool zircon::OxFeralGeneral::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,1);
}
void zircon::OxFeralGeneral::process_target(Time now,const CombatWorld& world) {
    if (!target) return;
    if ((cast<=0 || now>cast_at) && world.can_attack() && ZumaAi::within(x,y,target->x,target->y,8)) {
        cast=3;cast_at=now+5000;
        constexpr ZMagic magic[]={ZMagic::greater_frozen_earth,ZMagic::monster_scortched_earth,ZMagic::lightning_beam,ZMagic::blow_earth};
        constexpr Element element[]={Element::ice,Element::fire,Element::lightning,Element::wind};
        const auto choice=world.random(4);
        zircon_line_aoe(x,y,*target,direction,clock,now,12,-2,2,magic[choice],element[choice],world);
    }
    zircon_base_target(*this,now,world);
}
void zircon::OxFeralGeneral::attack(Time now,const CombatWorld& world) {--cast;zircon_normal_attack(*this,now,world);}
bool zircon::BanyoWarrior::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,1);
}
void zircon::BanyoWarrior::process_target(Time now,const CombatWorld& world) {
    if (!target) return;
    if (!ZumaAi::within(x,y,target->x,target->y,1) && now>teleport_at && world.can_attack()) {
        teleport_at=now+1000;
        if (world.random(7)==0 && zircon_teleport_to_target(x,y,*target,direction,world)) bonus=true;
    }
    zircon_base_target(*this,now,world);
}
void zircon::BanyoWarrior::attack(Time now,const CombatWorld& world) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);world.broadcast(direction,false,target->id);clock.update(now);
    int damage=world.get_dc();if (bonus && double_damage) damage*=2;
    world.enqueue({target->id,damage,now+400,attack_element});bonus=false;
}
zircon::NumaMage::~NumaMage()=default;
void zircon::NumaMage::process_target(Time now,const CombatWorld& world) {zircon_mage_target(*this,now,world);}
void zircon::NumaMage::range_attack(Time now,const CombatWorld& world) {
    if (target && ZumaAi::within(x,y,target->x,target->y,10))
        zircon_single_magic(x,y,*target,direction,clock,now,ZMagic::thunder_bolt,Element::lightning,false,world);
}
zircon::PinkBat::~PinkBat()=default;
bool zircon::PinkBat::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,1);
}
void zircon::PinkBat::process_target(Time now,const CombatWorld& world) {zircon_mage_target(*this,now,world);}
void zircon::PinkBat::attack(Time now,const CombatWorld& world) {zircon_normal_attack(*this,now,world);}
void zircon::PinkBat::range_attack(Time now,const CombatWorld& world) {
    if (target && ZumaAi::within(x,y,target->x,target->y,10))
        zircon_single_magic(x,y,*target,direction,clock,now,ZMagic::pink_fire_ball,Element::phantom,true,world);
}
zircon::Monkey::~Monkey()=default;
bool zircon::Monkey::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,2);
}
void zircon::Monkey::process_target(Time now,const CombatWorld& world) {zircon_monkey_target(*this,now,world);}
void zircon::Monkey::attack(Time now,const CombatWorld& world) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);clock.update(now);
    world.broadcast(direction,!ZumaAi::within(x,y,target->x,target->y,1),target->id);
    world.enqueue({target->id,world.get_dc(),now+400,attack_element});
}
bool zircon::NumaHighMage::in_attack_range() const {
    if (!target || !same_map) return false;
    const auto dx=std::abs(std::int64_t(target->x)-x),dy=std::abs(std::int64_t(target->y)-y);
    return (dx || dy) && dx<=attack_range && dy<=attack_range && (dx==0 || dy==0 || dx==dy);
}
void zircon::NumaHighMage::attack(Time now,const CombatWorld& world) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);clock.update(now);
    const bool melee=ZumaAi::within(x,y,target->x,target->y,1);
    world.broadcast(direction,!melee,target->id);
    if (melee) {world.enqueue({target->id,world.get_dc(),now+400,attack_element});return;}
    const auto delta=MonsterPursuit::directions[direction];
    for (int step=1;step<=attack_range;++step) {
        const int cx=x+delta[0]*step,cy=y+delta[1]*step;
        const auto id=(cx==target->x && cy==target->y)?std::optional<int>{target->id}:world.first_attackable(cx,cy);
        if (id) world.enqueue({*id,world.get_dc(),now+400,attack_element});
    }
}
bool zircon::FieryDancer::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,attack_range);
}
void zircon::FieryDancer::process_target(Time now,const CombatWorld& world) {zircon_monkey_target(*this,now,world);}
void zircon::FieryDancer::attack(Time now,const CombatWorld& world) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);clock.update(now);
    const bool ranged=world.random(5)==0;
    world.broadcast(direction,ranged,target->id);
    if (!ranged) world.enqueue({target->id,world.get_dc(),now+400,attack_element});
    else for (int id:world.get_targets(target->x,target->y,2)) world.enqueue({id,world.get_dc(),now+400,attack_element});
}
bool zircon::GiantLizard::in_attack_range(Time now) const {
    if (!target || !same_map || (x==target->x && y==target->y)) return false;
    const int range=now>range_at && (can_pvp_range || !target_is_player)?attack_range:1;
    return ZumaAi::within(x,y,target->x,target->y,range);
}
void zircon::GiantLizard::process_target(Time now,const CombatWorld& world,const std::function<void(PendingHit)>& enqueue) {
    if (!target) return;
    if (in_attack_range(now) && world.can_attack()) attack(now,world,enqueue);
    zircon_move_toward(*this,world);
    if (in_attack_range(now) && world.can_attack()) attack(now,world,enqueue);
}
void zircon::GiantLizard::attack(Time now,const CombatWorld& world,const std::function<void(PendingHit)>& enqueue) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);clock.update(now);
    const bool melee=ZumaAi::within(x,y,target->x,target->y,1);
    if (!melee) range_at=now+range_cooldown;
    world.broadcast(direction,!melee,target->id);
    enqueue({target->id,melee,now+400});
}
void zircon::GiantLizard::deliver(PendingHit hit,const std::function<int()>& get_dc,
    const std::function<void(int,int,Element)>& base_attack) {
    // 参考的近战/远程投递分支均在命中时读取DC和当前属性。
    base_attack(hit.target,get_dc(),attack_element);
}
zircon::UmaKing::~UmaKing()=default;
bool zircon::UmaKing::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,1);
}
void zircon::UmaKing::process_target(Time now,const CombatWorld& world) {
    if (!target) return;
    if (!in_attack_range()) {
        if (world.can_attack() && world.random(unsigned(range_chance))==0) range_attack(now,world);
        if (x==target->x && y==target->y) zircon_walk_rotation(int(world.random(8)),world);
        else world.move_to(target->x,target->y);
    }
    if (!world.can_attack()) return;
    if (world.random(unsigned(range_chance))>0) {if (in_attack_range()) attack(now,world);}
    else range_attack(now,world);
}
void zircon::UmaKing::range_attack(Time now,const CombatWorld& world) {
    if (!target) return;
    const auto magic=world.random(3)==0?ZMagic::lightning_ball:ZMagic::thunder_bolt;
    zircon_mass_magic(x,y,*target,direction,clock,now,magic,world);
}
void zircon::UmaKing::attack(Time now,const CombatWorld& world) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);world.broadcast(direction,false,target->id);clock.update(now);
    world.enqueue({target->id,world.get_dc(),now+400,attack_element});
}
void zircon::WingedHorror::range_attack(Time now,const CombatWorld& world) {
    if (!target) return;
    const auto choice=world.random(3);
    if (choice==1) zircon_line_aoe(x,y,*target,direction,clock,now,10,0,8,ZMagic::lightning_beam,Element::lightning,world);
    else zircon_mass_magic(x,y,*target,direction,clock,now,choice==0?ZMagic::lightning_ball:ZMagic::thunder_bolt,world);
}
bool zircon::EmperorSaWoo::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,1);
}
void zircon::EmperorSaWoo::process_target(Time now,const CombatWorld& world) {
    if (!target) return;
    if (!in_attack_range()) {
        if (world.can_attack() && world.random(3)==0) range_attack(now,world);
        if (x==target->x && y==target->y) zircon_walk_rotation(int(world.random(8)),world);
        else world.move_to(target->x,target->y);
    }
    if (!world.can_attack()) return;
    if (world.random(5)>0) {if (in_attack_range()) attack(now,world);}
    else range_attack(now,world);
}
void zircon::EmperorSaWoo::attack(Time now,const CombatWorld& world) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);world.broadcast(direction,false,target->id);clock.update(now);
    world.enqueue({target->id,world.get_dc(),now+400,attack_element});
}
void zircon::EmperorSaWoo::range_attack(Time now,const CombatWorld& world) {
    if (!target) return;
    if (world.random(3)==0) zircon_mass_magic(x,y,*target,direction,clock,now,ZMagic::cyclone,world);
    else {
        direction=zircon_direction(x,y,target->x,target->y);clock.update(now);world.broadcast(direction,true,target->id);
        for (int id:world.get_targets(target->x,target->y,2)) world.enqueue({id,world.get_dc(),now+400,attack_element});
    }
}
void zircon::OmaMage::attack(Time now,const CombatWorld& world) {
    if (!target) return;
    const auto choice=world.random(3);
    const int damage=choice==1?world.get_dc():world.get_dc()*2/3;
    direction=zircon_direction(x,y,target->x,target->y);
    auto cast=std::make_shared<CombatWorld::MagicCast>();cast->direction=direction;
    cast->magic=choice==0?ZMagic::lightning_ball:choice==1?ZMagic::thunder_bolt:ZMagic::lightning_wave;
    if (choice==2) cast->locations.emplace_back(target->x,target->y);else cast->targets.push_back(target->id);
    world.broadcast_magic(cast);clock.update(now);
    const auto victims=choice==2?world.get_targets(target->x,target->y,1):std::vector<int>{target->id};
    for (int id:victims) world.enqueue({id,damage==0?world.get_dc():damage,now+500,Element::lightning});
}
zircon::VoraciousGhost::VoraciousGhost(const MonsterPursuit::Random& random):revive_count(int(random(4))) {}
int zircon::VoraciousGhost::experience_divisor() const {return 1<<revive_count;}
bool zircon::VoraciousGhost::can_drop_rewards() const {return revive_count==0;}
void zircon::VoraciousGhost::die(Time now,Time corpse_duration,const MonsterPursuit::Random& random,const std::function<void()>& base) {
    base();revive_at=now+(Time(random(5))+3)*1000;dead_at=revive_at+corpse_duration;++death_count;
}
void zircon::VoraciousGhost::process(Time now,const std::function<void()>& base,const std::function<void()>& show,
    const std::function<void(int)>& set_hp) {
    base();
    if (!dead || revive_count==0 || now<revive_at) return;
    show();action_at=now+1500;dead=false;
    set_hp(int(double(maximum_hp)/std::pow(2.0,death_count)));--revive_count;
}
zircon::UndeadSoul::UndeadSoul(Time created_at):appear_at_(created_at+500) {}
void zircon::UndeadSoul::spawned(Time now,const std::function<void()>& base,const std::function<void()>& undead_soul_effect) {
    base();undead_soul_effect();action_at=now+1000;
}
bool zircon::UndeadSoul::can_be_seen(const std::function<bool()>& base) const {return visible && base();}
void zircon::UndeadSoul::process(Time now,const std::function<void()>& add_all_objects) {
    if (!pending_ || now<appear_at_) return;
    pending_=false;visible=true;add_all_objects();
}
bool zircon::SkeletonAxeThrower::in_attack_range() const {
    return target && same_map && ZumaAi::within(x,y,target->x,target->y,attack_range);
}
zircon::SkeletonAxeThrower::~SkeletonAxeThrower()=default;
int zircon::GoruArcher::attacked(int power,AttackOptions options,const std::function<int(int,AttackOptions)>& base,
    const std::function<void(int,int)>& teleport_nearby) {
    options.struck=true; // 原调用省略canStruck，因此使用基类默认true。
    const int result=base(power,options);
    if (result<0 || dead || !can_teleport || hp>maximum_hp/2) return result;
    can_teleport=false;teleport_nearby(7,12);
    return result;
}
void zircon::GoruArcher::process_target(Time now,const CombatWorld& world) {
    if (!target) return;
    if (!ZumaAi::within(x,y,target->x,target->y,1) && now>teleport_at && world.can_attack() && now>=fear_at) {
        teleport_at=now+1000;
        if (world.random(7)==0) {
            const int facing=zircon_direction(x,y,target->x,target->y);
            for (int i=0;i<8;++i) {
                const auto delta=MonsterPursuit::directions[(facing+i)%8];
                const int tx=target->x+delta[0],ty=target->y+delta[1];
                if (!world.teleport_cell_available(tx,ty)) continue;
                direction=zircon_direction(tx,ty,target->x,target->y);
                world.teleport(tx,ty);bonus=true;break;
            }
        }
    }
    SkeletonAxeThrower::process_target(now,world);
}
void zircon::GoruArcher::attack(Time now,const CombatWorld& world) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);world.broadcast(direction,false,target->id);clock.update(now);
    int damage=world.get_dc();if (bonus) damage*=2;
    world.enqueue({target->id,damage,now+400,attack_element});bonus=false;
}
void zircon::NumaStoneThrower::attack(Time now,const CombatWorld& world) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);
    world.broadcast(direction,true,target->id);clock.update(now);
    if (world.random(unsigned(fear_rate))==0) fear_at=now+(Time(fear_duration)+world.random(4))*1000;
    for (int id:world.get_targets(target->x,target->y,2)) {
        const auto distance=std::max(std::abs(std::int64_t(x)-target->x),std::abs(std::int64_t(y)-target->y));
        world.enqueue({id,world.get_dc(),now+400+distance*48,attack_element});
    }
}
bool zircon::HealerAnt::can_help_target(const HealTarget& target,const std::function<bool()>& base) const {
    return base() && target.hp<target.maximum && !target.heal_buff;
}
bool zircon::HealerAnt::can_attack_target(const HealTarget& target,const std::function<bool()>& base_help) const {
    return can_help_target(target,base_help);
}
bool zircon::HealerAnt::should_attack_target(const HealTarget& target,const std::function<bool()>& base_help) const {
    return can_attack_target(target,base_help);
}
void zircon::HealerAnt::process_search(const std::function<void()>& proper_search) {proper_search();}
void zircon::HealerAnt::heal(const HealTarget* target,const std::function<void(int,HealBuff)>& add_buff) {
    if (!target || !target->registered || target->dead) return;
    // 到期只检查Node/Dead，不重新检查距离、满血或已有Heal。
    add_buff(target->id,{healing,healing_cap});
}
void zircon::HealerAnt::attack(Time now,const CombatWorld& world) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);world.broadcast(direction,true,target->id);clock.update(now);
    const auto distance=std::max(std::abs(std::int64_t(x)-target->x),std::abs(std::int64_t(y)-target->y));
    world.enqueue_heal(target->id,now+400+distance*48);
}
void zircon::WindfurySorcerer::attack(Time now,const MonsterAiTarget& target,const CombatWorld& world) {
    direction=zircon_direction(x,y,target.x,target.y);world.broadcast(direction,true,target.id);clock.update(now);
    world.enqueue({target.id,world.get_dc(),now+400,attack_element});
}
zircon::YumgonWitch::~YumgonWitch()=default;
zircon::Element zircon::YumgonWitch::aoe_element() const {return aoe_element_;}
void zircon::YumgonWitch::set_aoe_element(Element element) {aoe_element_=element;}
bool zircon::YumgonWitch::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,attack_range);
}
void zircon::YumgonWitch::process_target(Time now,const CombatWorld& world) {
    if (!target) return;
    if (in_attack_range() && world.can_attack() && world.random(2)==0) attack(now,world);
    if (x==target->x && y==target->y) zircon_walk_rotation(int(world.random(8)),world);
    else world.move_to(target->x,target->y);
    if (in_attack_range() && world.can_attack()) attack(now,world);
}
void zircon::YumgonWitch::attack(Time now,const CombatWorld& world) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);clock.update(now);
    if (world.random(5)>0 || !ZumaAi::within(x,y,target->x,target->y,3)) {
        world.broadcast(direction,false,target->id);
        world.enqueue({target->id,world.get_dc(),now+400,attack_element});
    } else {
        // ObjectRangeAttack不填Targets；0表示广播没有主目标字段。
        world.broadcast(direction,true,0);
        for (int id:world.get_targets(x,y,3)) world.enqueue({id,world.get_dc(),now+400,aoe_element()});
    }
}
zircon::Element zircon::DragonQueen::aoe_element() const {return attack_element;}
void zircon::DragonQueen::set_aoe_element(Element) {}
bool zircon::DragonQueen::spawn_minion(int id,const World& world) {return world.spawn_on_map_random(id,x,y,10);}
void zircon::DragonQueen::process(Time now,const World& world) {
    world.base_process();
    if (dead || now<slave_at) return;
    slave_at=now+15000;
    const auto list=world.minions();
    for (auto it=list.rbegin();it!=list.rend();++it) {
        if (it->same_map && ZumaAi::within(x,y,it->x,it->y,15)) continue;
        world.clear_exp_owner(it->id);world.die(it->id);world.despawn(it->id);
    }
    world.spawn_minions(10-int(world.minions().size()),0,target?std::optional<int>{target->id}:std::nullopt);
}
void zircon::DragonQueen::die(const World& world) {
    world.base_die();
    const auto lord=world.create_dragon_lord();
    if (lord) world.spawn_exact(*lord,x,y); // 不设置Master或Target，也不因Spawn失败回滚死亡。
}
void zircon::SkeletonAxeThrower::process_target(Time now,const CombatWorld& world) {
    if (!target) return;
    if (!in_attack_range()) {
        if (x==target->x && y==target->y) zircon_walk_rotation(int(world.random(8)),world);
        else world.move_to(target->x,target->y);
        return;
    }
    if (ZumaAi::within(x,y,target->x,target->y,attack_range-1))
        zircon_walk_rotation(zircon_direction(target->x,target->y,x,y),world);
    if (!world.can_attack() || now<fear_at) return;
    attack(now,world);
}
void zircon::SkeletonAxeThrower::attack(Time now,const CombatWorld& world) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);
    world.broadcast(direction,true,target->id);clock.update(now);
    if (world.random(unsigned(fear_rate))==0) fear_at=now+(Time(fear_duration)+world.random(4))*1000;
    const auto distance=std::max(std::abs(std::int64_t(x)-target->x),std::abs(std::int64_t(y)-target->y));
    constexpr int projectile_speed=48; // Zircon Globals.ProjectileSpeed
    world.enqueue({target->id,world.get_dc(),now+400+distance*projectile_speed,attack_element});
}
bool zircon::SonicLizard::in_attack_range() const {
    if (!target || !same_map) return false;
    const auto dx=std::abs(std::int64_t(target->x)-x),dy=std::abs(std::int64_t(target->y)-y);
    return (dx || dy) && dx<=range && dy<=range && (dx==0 || dy==0 || dx==dy);
}
void zircon::SonicLizard::process_target(Time now,const CombatWorld& world,const std::function<void(PendingHit)>& enqueue) {
    if (!target) return;
    if (in_attack_range() && world.can_attack()) attack(now,world,enqueue);
    if (x==target->x && y==target->y) zircon_walk_rotation(int(world.random(8)),world);
    else world.move_to(target->x,target->y);
    // 移动回调更新本实例坐标，CanAttack重新读取动作时间，不能复用移动前的判定。
    if (in_attack_range() && world.can_attack()) attack(now,world,enqueue);
}
void zircon::SonicLizard::attack(Time now,const CombatWorld& world,const std::function<void(PendingHit)>& enqueue) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);clock.update(now);
    if (ZumaAi::within(x,y,target->x,target->y,1) && world.random(4)>0) {
        world.broadcast(direction,false,target->id);enqueue({target->id,true,now+400});
    } else {
        world.broadcast(direction,true,target->id);
        const auto delta=MonsterPursuit::directions[direction];
        for (int step=1;step<=range;++step) {
            const int cx=x+delta[0]*step,cy=y+delta[1]*step;
            const auto id=(cx==target->x && cy==target->y)?std::optional<int>{target->id}:world.first_attackable(cx,cy);
            if (id) enqueue({*id,false,now+400});
        }
    }
}
void zircon::SonicLizard::deliver(PendingHit hit,const std::function<int()>& get_dc,
    const std::function<void(int,int,Element)>& base_attack) {
    // 原melee=true/false两个分支相同，均在投递时抽取GetDC和当前AttackElement。
    const int damage=get_dc();
    base_attack(hit.target,damage,attack_element);
}
void zircon::Stomper::attack(Time now,const MonsterAiTarget& target,const CombatWorld& world,
    const std::function<std::vector<int>(int,int,int)>& get_targets) {
    direction=zircon_direction(x,y,target.x,target.y);world.broadcast(direction,false,target.id);clock.update(now);
    for (int id:get_targets(x,y,1)) world.enqueue({id,world.get_dc(),now+400,attack_element});
}

bool zircon::CarnivorousPlant::can_attack(bool base) const {return visible && base;}
bool zircon::CarnivorousPlant::can_move(bool base) const {return visible && base;}
bool zircon::CarnivorousPlant::blocking(bool base) const {return visible && base;}
bool zircon::CarnivorousPlant::can_be_seen(const std::function<bool()>& base) const {return visible && base();}
void zircon::CarnivorousPlant::process(Time now,const std::function<void()>& base,
    const std::function<bool()>& base_can_attack,const std::function<bool()>& in_attack_range,
    const std::function<void(Event)>& notify) {
    base();
    if (dead || now<shock_at || now<=visible_at) return;
    visible_at=now+3000;
    const bool desired=target && (ZumaAi::within(x,y,target->x,target->y,visible?hide_range:find_range) ||
        (visible && in_attack_range()));
    if (!visible && desired) {
        visible=true;cell_at=now+500;clock.action_at=now+1000;
        notify(Event::add_objects);notify(Event::show);
    }
    if (visible && !desired && base_can_attack()) {
        visible=false;visible_at=now+4000;
        notify(Event::hide);removals_.push_back(now+3000);
        notify(Event::restore_health);notify(Event::clear_poison);
    }
}
void zircon::CarnivorousPlant::process_actions(Time now,const std::function<void(Event)>& notify) {
    for (std::size_t i=removals_.size();i>0;--i) {
        if (now<removals_[i-1]) continue;
        removals_.erase(removals_.begin()+std::ptrdiff_t(i-1));
        notify(Event::remove_objects);
    }
}
bool zircon::ZumaGuardian::can_move(bool base) const {return base && visible;}
bool zircon::ZumaGuardian::can_attack(bool base) const {return base && visible;}
bool zircon::ZumaGuardian::apply_poison(const std::function<bool()>& base) const {return visible && base();}
int zircon::ZumaGuardian::attacked(int power,AttackOptions options,const std::function<int(int,AttackOptions)>& base) const {
    if (!visible) return 0;
    return base(power,{options.ignore_shield,options.ignore_shield,options.critical,true});
}
void zircon::ZumaGuardian::wake(Time now,const std::function<void(ZumaGuardian&)>& show) {
    action_at=now+1000;show(*this);visible=true;
}
void zircon::ZumaGuardian::process(Time now,const std::function<void()>& base,
    const std::function<std::vector<ZumaGuardian*>(int,int,int)>& nearby,
    const std::function<void(ZumaGuardian&)>& show) {
    base();
    if (dead || !target || visible || !ZumaAi::within(x,y,target->x,target->y,3)) return;
    wake(now,show);
    for (auto* guardian:nearby(x,y,wake_range)) {
        if (!guardian || guardian->visible) continue;
        // 原WakeAll不再检查邻居Dead，且先唤醒再复制Target。
        guardian->wake(now,show);guardian->target=target;
    }
}
bool zircon::TreeMonster::can_move() const {return false;}
bool zircon::TreeMonster::can_attack() const {return false;}
bool zircon::TreeMonster::can_attack_target() const {return false;}
bool zircon::TreeMonster::apply_poison() const {return false;}
void zircon::TreeMonster::process_roam() {}
void zircon::TreeMonster::process_search() {}
void zircon::TreeMonster::process_target() {}
int zircon::TreeMonster::attacked(AttackOptions options,const std::function<int(int,AttackOptions)>& base) const {
    // 原调用仅提供前五个参数：ignoreShield进入canReflect，canCrit进入ignoreShield。
    return base(1,{options.ignore_shield,options.critical,true,true});
}
bool zircon::QuartzTree::spawn_minion(int id,const World& world) {return world.spawn_on_map_random(id,x,y,5);}
void zircon::QuartzTree::spawn_sub(const World& world) {
    const auto sub=world.create_sub();
    if (!sub || !spawn_minion(*sub,world)) return;
    world.set_target(*sub,target); // 子Boss不是普通随从，不加Master或MinionList。
}
void zircon::QuartzTree::process(Time now,const World& world) {
    world.base_process();
    if (dead) return;
    if (!sub_spawned && hp<maximum_hp/4) {sub_spawned=true;spawn_sub(world);}
    if (now<slave_at) return;
    slave_at=now+60000;
    const auto list=world.minions();
    for (auto it=list.rbegin();it!=list.rend();++it) {
        if (it->same_map && ZumaAi::within(x,y,it->x,it->y,30)) continue;
        world.clear_exp_owner(it->id);world.die(it->id);world.despawn(it->id);
    }
    world.show();
    world.spawn_minions(10-int(world.minions().size()),0,target);
}
void zircon::QuartzTree::process_regen() {}

bool zircon::FerociousIceTiger::in_attack_range(const MonsterAiTarget& target,bool same_map) const {
    return same_map && (x!=target.x || y!=target.y) && ZumaAi::within(x,y,target.x,target.y,3);
}
void zircon::FerociousIceTiger::location_changed(const std::function<void()>& base) {base();spinning=false;}
void zircon::FerociousIceTiger::attack(Time now,const MonsterAiTarget& target,const CombatWorld& world,
    const std::function<bool(int)>& player_warrior) {
    direction=zircon_direction(x,y,target.x,target.y);
    // Random在左侧，即使已旋转也必须抽样。
    if (world.random(5)==0 || spinning) {
        world.broadcast(direction,true,0);
        for (int id:world.get_targets(x,y,3)) {
            int damage=world.get_dc();
            if (player_warrior(id)) damage-=damage*2/10;
            world.enqueue({id,damage,now+400,attack_element});
        }
        clock.action_at=now+600;clock.attack_at=now+640;spinning=true;
    } else {
        world.broadcast(direction,false,target.id);
        const auto delta=MonsterPursuit::directions[direction];
        for (int id:world.get_targets(x+delta[0]*3,y+delta[1]*3,3))
            world.enqueue({id,world.get_dc()/2,now+600,attack_element});
        clock.update(now);
    }
}
void zircon::QueenOfDawn::process_ai(Time now,const std::function<void()>& base,
    const std::function<std::vector<int>()>& map_snapshot,const std::function<bool(int)>& can_attack_target,
    const std::function<void(int)>& teleport_random) {
    base();
    if (dead || now<=ting_at) return;
    ting_at=now+60000;
    const auto objects=map_snapshot();
    for (int id:objects) if (can_attack_target(id)) teleport_random(id);
}
void zircon::QueenOfDawn::process_target(Time now,const CombatWorld& world,const std::function<void()>& base) {
    if (!target) return;
    if (!ZumaAi::within(x,y,target->x,target->y,1) && now>teleport_at && world.can_attack() && has_spawn_info) {
        teleport_at=now+1000;
        if (world.random(10)==0) {
            const int facing=zircon_direction(x,y,target->x,target->y);
            for (int i=0;i<8;++i) {
                const auto delta=MonsterPursuit::directions[(facing+i)%8];
                const int tx=target->x+delta[0],ty=target->y+delta[1];
                if (!world.teleport_cell_available(tx,ty)) continue;
                direction=zircon_direction(tx,ty,target->x,target->y);world.teleport(tx,ty);bonus=true;break;
            }
        }
    }
    base();
}
void zircon::QueenOfDawn::attack(Time now,const CombatWorld& world) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);clock.update(now);
    if (world.random(5)>0) {
        world.broadcast(direction,false,target->id);world.enqueue({target->id,world.get_dc(),now+400,attack_element});
    } else {
        world.broadcast(direction,true,target->id);
        for (int id:world.get_targets(x,y,2)) world.enqueue({id,world.get_dc(),now+400,attack_element});
    }
    // Bonus在该源码中不参与伤害，也不在Attack清除；不能照搬GoruArcher的双倍效果。
}
bool zircon::MutantFlea::in_attack_range(const MonsterAiTarget& target,bool same_map) const {
    return same_map && (x!=target.x || y!=target.y) && ZumaAi::within(x,y,target.x,target.y,attack_range);
}
void zircon::MutantFlea::attack(Time now,const MonsterAiTarget& target,const CombatWorld& world) {
    direction=zircon_direction(x,y,target.x,target.y);
    const bool melee=ZumaAi::within(x,y,target.x,target.y,1);
    world.broadcast(direction,!melee,target.id);clock.update(now);
    const auto distance=std::max(std::abs(std::int64_t(x)-target.x),std::abs(std::int64_t(y)-target.y));
    world.enqueue({target.id,world.get_dc(),now+400+(melee?0:distance*48),attack_element});
}
bool zircon::BlasterMutantFlea::in_attack_range(const MonsterAiTarget& target,bool same_map) const {
    return same_map && (x!=target.x || y!=target.y) && ZumaAi::within(x,y,target.x,target.y,attack_range);
}
void zircon::BlasterMutantFlea::attack(Time now,const MonsterAiTarget& target,const CombatWorld& world) {
    direction=zircon_direction(x,y,target.x,target.y);world.broadcast(direction,true,target.id);clock.update(now);
    const auto distance=std::max(std::abs(std::int64_t(x)-target.x),std::abs(std::int64_t(y)-target.y));
    world.enqueue({target.id,world.get_dc(),now+400+distance*48,attack_element});
}

zircon::SpittingSpider::~SpittingSpider()=default;
bool zircon::SpittingSpider::in_attack_range() const {return target && in_attack_range(*target,same_map);}
void zircon::SpittingSpider::attack(Time now,const CombatWorld& world) {attack(now,*target,world);}
int zircon::GoruSpearman::attacked(int power,AttackOptions options,const std::function<int(int,AttackOptions)>& base,
    const std::function<void(int,int)>& teleport_nearby) {
    options.struck=true;
    const int result=base(power,options);
    if (result<0 || dead || !can_teleport || hp>maximum_hp/2) return result;
    can_teleport=false;teleport_nearby(7,12);return result;
}
bool zircon::GoruSpearman::in_attack_range() const {
    return target && SpittingSpider::in_attack_range(*target,same_map);
}
void zircon::GoruSpearman::process_target(Time now,const CombatWorld& world) {
    if (!target) return;
    if (!ZumaAi::within(x,y,target->x,target->y,1) && now>teleport_at && world.can_attack()) {
        teleport_at=now+1000;
        if (world.random(7)==0 && zircon_teleport_to_target(x,y,*target,direction,world)) bonus=true;
    }
    zircon_base_target(*this,now,world);
}
void zircon::GoruSpearman::attack(Time now,const CombatWorld& world) {
    if (target) attack(now,*target,world);
}
void zircon::GoruSpearman::attack(Time now,const MonsterAiTarget& victim,const CombatWorld& world) {
    direction=zircon_direction(x,y,victim.x,victim.y);world.broadcast(direction,false,victim.id);clock.update(now);
    int damage=world.get_dc();if (bonus) damage*=2;
    world.enqueue({victim.id,damage,now+400,attack_element});bonus=false;
}
bool zircon::SpittingSpider::in_attack_range(const MonsterAiTarget& target,bool same_map) const {
    if (!same_map) return false;
    const auto dx=std::abs(std::int64_t(target.x)-x),dy=std::abs(std::int64_t(target.y)-y);
    return (dx || dy) && dx<=2 && dy<=2 && (dx==0 || dy==0 || dx==dy);
}
void zircon::SpittingSpider::attack(Time now,const MonsterAiTarget& target,const CombatWorld& world) {
    direction=zircon_direction(x,y,target.x,target.y);world.broadcast(direction,false,target.id);clock.update(now);
    const auto delta=MonsterPursuit::directions[direction];
    for (int step=1;step<=2;++step) {
        const int cx=x+delta[0]*step,cy=y+delta[1]*step;
        const auto id=(cx==target.x && cy==target.y)?std::optional<int>{target.id}:world.first_attackable(cx,cy);
        if (id) world.enqueue({*id,world.get_dc(),now+400,attack_element});
    }
}
bool zircon::RazorTusk::in_attack_range() const {
    return target && same_map && (x!=target->x || y!=target->y) && ZumaAi::within(x,y,target->x,target->y,3);
}
void zircon::RazorTusk::process_target(Time now,const CombatWorld& world,const std::function<void(PendingHit)>& enqueue) {
    if (!target) return;
    if (in_attack_range() && world.can_attack() && world.random(3)>0) attack(now,world,enqueue);
    if (x==target->x && y==target->y) zircon_walk_rotation(int(world.random(8)),world);
    else world.move_to(target->x,target->y);
    if (in_attack_range() && world.can_attack()) attack(now,world,enqueue);
}
void zircon::RazorTusk::attack(Time now,const CombatWorld& world,const std::function<void(PendingHit)>& enqueue) {
    if (!target) return;
    direction=zircon_direction(x,y,target->x,target->y);clock.update(now);
    const bool melee=ZumaAi::within(x,y,target->x,target->y,1);
    world.broadcast(direction,!melee,target->id);enqueue({target->id,melee,now+400});
}
void zircon::RazorTusk::deliver(PendingHit hit,const std::function<int()>& get_dc,
    const std::function<void(int,int,Element)>& base_attack) {
    const int damage=get_dc();base_attack(hit.target,damage,attack_element);
}
void zircon::DepartedMonster::die(Time now,const CombatWorld& world,const std::function<void()>& base_die,
    const std::function<void(Effect)>& broadcast_effect,const std::function<bool(int)>& alive_player,
    const std::function<void(int,int,int)>& spawn_minions) {
    base_die();
    const auto selected=world.random(5);
    const Element element=Element(selected);
    constexpr Effect effects[]={Effect::puppet,Effect::puppet_fire,Effect::puppet_ice,Effect::puppet_lightning,Effect::puppet_wind};
    broadcast_effect(effects[selected]);
    const auto targets=world.get_targets(x,y,2);
    for (int id:targets) world.enqueue({id,world.get_dc(),now+400,element});
    // 同一个目标快照分两轮处理；第二轮重新读取玩家死亡状态。
    for (int id:targets) if (alive_player(id)) spawn_minions(4,0,id);
}
bool zircon::DepartedMonster::spawn_minion(int id,const std::function<bool(int,int,int)>& spawn) {return spawn(id,x,y);}
bool zircon::NetherworldGate::can_move() const {return false;}
bool zircon::NetherworldGate::can_attack() const {return false;}
int zircon::NetherworldGate::attacked() const {return 0;}
bool zircon::NetherworldGate::apply_poison() const {return false;}
std::uint32_t zircon::NetherworldGate::name_colour() const {return 0xff00ff00u;}
void zircon::NetherworldGate::process_search() {}
void zircon::NetherworldGate::activate(const std::function<void()>& add_active) {
    if (activated) return;
    activated=true;add_active();
}
void zircon::NetherworldGate::deactivate() {}
void zircon::NetherworldGate::spawned(Time now,const std::function<void()>& base,const std::function<void(Event)>& notify) {
    base();despawn_at=now+20*60*1000;notify(Event::gate_open);
}
void zircon::NetherworldGate::process(Time now,const World& world) {
    world.base_process();
    if (now>=despawn_at) {
        if (has_spawn_info) world.notify(Event::decrement_alive);
        world.notify(Event::gate_closed);has_spawn_info=false;
        world.notify(Event::clear_spawn_info);world.notify(Event::despawn);return;
    }
    if (now<search_at || !world.has_ship_region()) return;
    search_at=now+3000;
    // 以当前Instance/InstanceSequence取MysteryShip地图；缺图后等60秒。
    if (!world.resolve_instance_map()) {search_at=now+60000;return;}
    const auto points=world.ensure_points(); // 仅PointList为null时按目标地图宽度生成。
    if (points.empty()) return;
    const auto objects=world.map_objects();
    for (auto it=objects.rbegin();it!=objects.rend();++it) {
        if (it->self || it->guard || (!it->player && !it->monster) || it->safe || it->dead ||
            !ZumaAi::within(x,y,it->x,it->y,view_range)) continue;
        const auto [tx,ty]=points[world.random(unsigned(points.size()))];
        world.teleport_to_ship(it->id,tx,ty);
    }
}
bool zircon::JinamStoneGate::can_move() const {return false;}
bool zircon::JinamStoneGate::can_attack() const {return false;}
int zircon::JinamStoneGate::attacked() const {return 0;}
bool zircon::JinamStoneGate::apply_poison() const {return false;}
std::uint32_t zircon::JinamStoneGate::name_colour() const {return 0xff00ff00u;}
void zircon::JinamStoneGate::process_search() {}
void zircon::JinamStoneGate::spawned(Time now,const std::function<void()>& base,const std::function<void(Event)>& notify) {
    base();despawn_at=now+20*60*1000;notify(Event::gate_open);
}
void zircon::JinamStoneGate::process(Time now,const World& world) {
    world.base_process();
    if (now>=despawn_at) {
        if (has_spawn_info) world.notify(Event::decrement_alive);
        world.notify(Event::gate_closed);has_spawn_info=false;
        world.notify(Event::clear_spawn_info);world.notify(Event::despawn);return;
    }
    if (now<search_at || !world.has_lair_region()) return;
    search_at=now+3000;
    // 回调按当前地图Instance/InstanceSequence取目标地图，不跨实例传送。
    if (!world.resolve_instance_map()) {search_at=now+60000;return;}
    const auto points=world.ensure_points(); // 仅PointList为null时创建，空列表保持为空。
    if (points.empty()) return;
    const auto objects=world.map_objects();
    for (auto it=objects.rbegin();it!=objects.rend();++it) {
        if (it->self || it->guard || !it->player || it->safe || it->dead || !ZumaAi::within(x,y,it->x,it->y,view_range)) continue;
        const auto [tx,ty]=points[world.random(unsigned(points.size()))];
        world.teleport_to_lair(it->id,tx,ty);
    }
}

bool zircon::GhostMage::can_attack(bool base) const {return visible && base;}
bool zircon::GhostMage::can_move(bool base) const {return visible && base;}
bool zircon::GhostMage::blocking(bool base) const {return visible && base;}
bool zircon::GhostMage::can_be_seen(const std::function<bool()>& base) const {return visible && base();}
void zircon::GhostMage::process(Time now,const std::function<void()>& base,const std::function<void(Event)>& notify) {
    base();
    if (dead || now<shock_at || visible || now<=visible_at) return;
    visible_at=now+3000;
    if (!target || !ZumaAi::within(x,y,target->x,target->y,5)) return;
    visible=true;cell_at=now+500;clock.action_at=now+1000;
    notify(Event::zombie_hole);notify(Event::add_objects);notify(Event::show);
}
void zircon::GhostMage::attack(Time now,const std::function<void(int)>& broadcast,const std::function<int()>& get_dc,
    const MonsterPursuit::Random& random,const std::function<void(DelayedHit)>& enqueue) {
    if (!target) return;
    direction=MonsterPursuit::direction(x,y,target->x,target->y);if (direction<0) direction=0;
    broadcast(direction);clock.update(now);
    const int damage=get_dc();
    const Element element=spectral_hit?Element(random(8)):attack_element;
    enqueue({target->id,damage,now+400,element});
}
bool zircon::GhostSorcerer::in_attack_range(const MonsterAiTarget& target,bool same_map) const {
    if (!same_map) return false;
    const auto dx=std::abs(std::int64_t(target.x)-x),dy=std::abs(std::int64_t(target.y)-y);
    return (dx || dy) && dx<=attack_range && dy<=attack_range && (dx==0 || dy==0 || dx==dy);
}
void zircon::GhostSorcerer::attack(Time now,const MonsterAiTarget& target,const std::function<void(int)>& broadcast,
    const std::function<std::optional<int>(int,int)>& first_attackable,const std::function<int()>& get_dc,
    const std::function<void(DelayedHit)>& enqueue) {
    direction=MonsterPursuit::direction(x,y,target.x,target.y);if (direction<0) direction=0;
    broadcast(direction);clock.update(now);
    const auto delta=MonsterPursuit::directions[direction];
    for (int step=1;step<=attack_range;++step) {
        const int cx=x+delta[0]*step,cy=y+delta[1]*step;
        const auto id=(cx==target.x && cy==target.y)?std::optional<int>{target.id}:first_attackable(cx,cy);
        if (id) enqueue({*id,get_dc(),now+400,attack_element});
    }
}

void zircon::AttackClock::update(Time now) {
    attack_at=now+attack_delay;
    action_at=now+std::min(move_delay,attack_delay-100);
    if (slow) {attack_at+=Time(*slow)*100;action_at+=Time(*slow)*100;}
    if (chain_slow) {attack_at+=100;action_at+=100;}
    if (neutralized) {attack_at+=attack_delay;action_at+=std::min(move_delay,attack_delay-100);}
}

zircon::WhiteBone::WhiteBone(Time created_at):appear_at_(created_at+1000) {}
void zircon::WhiteBone::spawned(Time now,const std::function<void()>& base,const std::function<void()>& summon_effect) {
    base();summon_effect();action_at=now+1000;
}
bool zircon::WhiteBone::can_be_seen(const std::function<bool()>& base) const { return visible && base(); }
void zircon::WhiteBone::process(Time now,const std::function<void()>& add_all_objects) {
    if (!pending_ || now<appear_at_) return;
    pending_=false;visible=true;add_all_objects();
    // Function分支在Appear后直接返回，不继续调用基类ProcessAction。
}

void zircon::Warewolf::process_target(Time now,int x,int y,const MonsterAiTarget* target,bool can_attack,const World& world) {
    if (!target) return;
    if (now>cast_at && can_attack && ZumaAi::within(x,y,target->x,target->y,8)) {
        cast_at=now+10000;
        direction=MonsterPursuit::direction(x,y,target->x,target->y);
        if (direction<0) direction=0; // Zircon DirectionFromPoint同格为Up，与旧C++不同。
        auto cast=std::make_shared<Cast>();cast->direction=direction;
        world.broadcast(cast); // 参考包保存列表引用，广播后继续填充locations。
        clock.update(now);
        for (int offset=-2;offset<=2;++offset) {
            const int ray=(direction+offset+8)%8;
            const auto delta=MonsterPursuit::directions[ray];
            for (int step=1;step<=8;++step) {
                const int cx=x+delta[0]*step,cy=y+delta[1]*step;
                if (!world.cell_exists(cx,cy)) continue;
                cast->locations.emplace_back(cx,cy);
                const auto hits=[&](int hx,int hy,bool half) {
                    if (!world.cell_exists(hx,hy)) return;
                    for (const int id:world.attackable_objects(hx,hy)) {
                        const int dc=world.get_dc();
                        world.enqueue_hit({id,half?dc/2:dc,now+500+step*75});
                    }
                };
                hits(cx,cy,false);
                const int shift=ray%2==0?2:1;
                for (const int side:{-shift,shift}) {
                    const auto lateral=MonsterPursuit::directions[(ray+side+8)%8];
                    hits(cx+lateral[0],cy+lateral[1],true);
                }
            }
        }
    }
    world.base_target(); // 施法之后仍执行普通追踪/攻击，交由更新的时钟限制。
}

bool zircon::WedgeMothLarva::can_move() const {return false;}
bool zircon::WedgeMothLarva::in_attack_range(bool same_map) const {return same_map;}
void zircon::WedgeMothLarva::attack(Time now,const std::function<void()>& broadcast) {
    broadcast();clock.update(now);pending_.push_back(now+600);
}
void zircon::WedgeMothLarva::process(Time now,int x,int y,const World& world) {
    const auto count=pending_.size();
    // MapObject.Process从ActionList尾部向前投递，同轮到期项按逆插入顺序处理。
    for (std::size_t index=count;index>0;--index) {
        const auto due=pending_[index-1];
        if (now<due) continue;
        pending_.erase(pending_.begin()+std::ptrdiff_t(index-1));
        // SpawnMinions(1,0,Target)仍调用Random.Next(1)，上限在投递时检查。
        const int requested=1+int(world.random(1));
        const int spawn_count=std::min(maximum_minions-int(minions.size()),requested);
        for (int i=0;i<spawn_count;++i) {
            const auto child=world.create_minion();
            if (!child) continue;
            if (!world.spawn_on_current_map(*child,x,y)) break;
            world.bind_target_and_master(*child,target);minions.push_back(*child);
        }
        // 和WhiteBone不同：Function处理后仍落入基类ProcessAction。
        world.base_function();
    }
}

int MonsterVitals::mitigate(int damage,const MonsterPursuit::Random& random) {
    damage=MonsterMeleeAttack::armor_damage(damage,ac,random);
    if (damage>0 && bubble) {
        // ROUND 的输入已经过整数除法，不能改成浮点百分比或补成“减去护盾比例”。
        damage=(damage/100)*int(bubble_level+2)*8;
        if (bubble_time>0) bubble_time=bubble_time>3?bubble_time-3:1;
    }
    return damage;
}

void MonsterVitals::damage_health(int damage) {
    const auto remaining=std::int64_t(hp)-damage;
    hp=int(damage>0?std::max<std::int64_t>(0,remaining):std::min<std::int64_t>(maximum,remaining));
}

int MonsterVitals::struck_damage(int damage,const MonsterPursuit::Random& random) {
    if (damage<=0) return 0;
    (void)random(10); // StruckDamage 在检查 m_pUserInfo 前抽取损耗，怪物也会消费。
    if (damage_armor) damage=int(double(damage)*1.2+0.5);
    damage_health(damage);
    return damage;
}

int MonsterMeleeAttack::armor_damage(int damage,std::uint16_t ac,const MonsterPursuit::Random& random) {
    const int low=ac&0xff,high=ac>>8,span=high-low+1;
    const int armor=low+(span>0?int(random(unsigned(span))):0);
    return std::max(0,damage-armor);
}

MonsterMeleeAttack::Outcome MonsterMeleeAttack::resolve(std::uint16_t dc,unsigned hit_point,int weapon_strong,
    const Victim& victim,const MonsterPursuit::Random& random,const Hooks& hooks) {
    const int low=dc&0xff,high=dc>>8;
    int power=low+int(random(unsigned(std::max(0,high-low)+1)));
    if (!victim.proper) power=0;
    else if (victim.hit_point>0) {
        // 参考在此无保护地对目标速度取模；未定义输入不能归类为命中或闪避。
        if (!victim.speed_point) return Outcome::undefined_speed;
        if (hit_point<random(victim.speed_point)) power=0;
    }
    if (power<=0) return Outcome::missed;
    power=hooks.mitigate(power);
    const int wear=int(random(5))+2-weapon_strong;
    if (wear>0) hooks.wear_weapon(wear);
    if (power<=0) return Outcome::missed;
    hooks.damage(power);
    hooks.delayed_struck(power,550);
    if (!victim.human) hooks.immediate_struck(power);
    return Outcome::damaged;
}

bool reference_monster_petrified(std::uint32_t flags) {
    // ObjectEngine.h POISON_STONE=5；GetCharStatus 对状态数组使用 0x80000000 >> i。
    return (flags&std::uint32_t(ReferenceMonsterStatus::petrified))!=0;
}
bool MonsterRunState::struck(Time now,unsigned level,const Target* hitter) {
    const bool selected=hitter && hitter->proper;
    if (selected) {target=*hitter;focused_at=now;}
    if (level<50) hit_at+=150-std::min(130u,level*4);
    return selected;
}

bool MonsterRunState::walk_to(int facing,const std::function<bool(int,int)>& safe,const MonsterPursuit::Walk& walk) {
    if (holy_seize || facing<0 || facing>=8) return false;
    const auto& offset=MonsterPursuit::directions[facing];
    const int next_x=x+offset[0],next_y=y+offset[1];
    if (escape && !safe(next_x,next_y)) return false;
    // 世界 WalkXY 回调更新坐标/朝向并广播；不能只依返回值假设位置已改变。
    return walk(next_x,next_y,facing);
}

void MonsterRunState::wander(const MonsterPursuit::Random& random,const std::function<void(int)>& turn,
    const std::function<void(int)>& walk) {
    if (random(20)!=0) return;
    if (random(4)==1) turn(int(random(8)));
    else walk(direction);
}

void MonsterRunState::go_to_target(Time now,const MonsterAiTarget* head,
    const std::function<void(const MonsterAiTarget&)>& select,const MonsterPursuit::Random& random,
    const std::function<void(int)>& walk) {
    if (x==target_x && y==target_y) return;
    if (head && head->alive && head->proper && !head->inspector &&
        MonsterPursuit::distance(x,y,*head)<std::abs(std::int64_t(x)-target_x)+std::abs(std::int64_t(y)-target_y)) {
        // 此参考分支不检查 HumHideMode，且重选后本轮不移动。
        select(*head);target_x=head->x;target_y=head->y;
        return;
    }
    found_path_at=now;
    int facing=MonsterPursuit::direction(x,y,target_x,target_y);
    const int old_x=x,old_y=y;
    walk(facing);
    for (int retry=0;retry<7 && old_x==x && old_y==y;++retry) {
        facing=(facing+(random(3)!=0?1:7))%8;
        walk(facing);
    }
}

bool MonsterRunState::think(Time now,const std::function<int()>& duplicate_count,const MonsterPursuit::Random& random,
    const std::function<void(int)>& walk) {
    if (Time(now-thought_at)>3000) {
        thought_at=now;
        if (duplicate_count()>=2) duplicate=true;
        if (target && !target->proper) target.reset();
    }
    if (duplicate) {
        const int old_x=x;
        walk(int(random(8)));
        // 参考重复比较旧X与新X，没有比较Y；仅纵向移动不会清除 DupMode。
        if (old_x!=x) {duplicate=false;return true;}
    }
    return false;
}

bool MonsterRunState::attack_target(Time now,const std::function<void()>& hit,const std::function<void()>& announce,
    const std::function<void()>& break_holy) {
    if (!target) return false;
    const auto dx=std::int64_t(target->x)-x,dy=std::int64_t(target->y)-y;
    if (std::abs(dx)<=1 && std::abs(dy)<=1 && (dx!=0 || dy!=0)) {
        target_direction=MonsterPursuit::direction(x,y,target->x,target->y);
        if (Time(now-hit_at)>hit_interval) {
            hit_at=focused_at=now;
            direction=target_direction;
            hit();announce();break_holy();
        }
        // 冷却中的邻格目标仍视为已处理，阻止基类继续移动。
        return true;
    }
    if (target->same_map) {target_x=target->x;target_y=target->y;}
    else {target.reset();target_x=-1;target_y=-1;}
    return false;
}

void MonsterRunState::run(Time now,const Hooks& hooks) {
    if (!dead && !hidden && !ghost && !stone && !petrified) {
        if (target && (Time(now-focused_at)>30000 || target->dead || target->ghost ||
            std::abs(std::int64_t(target->x)-x)>15 || std::abs(std::int64_t(target->y)-y)>15)) target.reset();
        if (holy_seize && Time(now-holy_started)>holy_duration) hooks.break_holy();
        if (hooks.think()) { hooks.inherited();return; }
        if (Time(now-walked_at)>walk_interval) {
            walked_at=now;
            if (!run_away) {
                if (!no_attack) {
                    if (target) {
                        if (hooks.attack()) { hooks.inherited();return; }
                    } else {
                        target_x=-1;
                        if (has_mission) {target_x=mission_x;target_y=mission_y;}
                    }
                }
                if (master) {
                    if (!target) {
                        const auto [back_x,back_y]=hooks.master_back();
                        if (std::abs(std::int64_t(target_x)-back_x)>1 || std::abs(std::int64_t(target_y)-back_y)>1) {
                            target_x=back_x;target_y=back_y;
                            // 参考条件是任一轴相距不超过两格，不改成方形距离判断。
                            if ((std::abs(std::int64_t(x)-back_x)<=2 || std::abs(std::int64_t(y)-back_y)<=2) &&
                                hooks.occupied(back_x,back_y)) {target_x=x;target_y=y;}
                        }
                    }
                    if (std::abs(std::int64_t(x)-master->x)>20 || std::abs(std::int64_t(y)-master->y)>20)
                        hooks.teleport_to_master_map(target_x,target_y);
                }
            } else if (runaway_duration>0 && Time(now-runaway_started)>runaway_duration) {
                run_away=false;runaway_duration=0;
            }
            if (target_x!=-1) hooks.go_to_target();
            else if (!target) hooks.wander();
        }
    }
    // 对应 InheritedRun：恢复/死亡及 CCharObject::Run 由同一世界回调按顺序完成。
    hooks.inherited();
}

void AnimalMonster::run(std::uint32_t now,const MonsterRunState::Hooks& hooks) {
    state.run(now,hooks);
}

void MinorNumaMonster::run(std::uint32_t now,const std::function<std::optional<int>()>& select,const std::function<void()>& base) {
    if (!dead && (std::uint32_t(now-searched_at)>8000 || (!target && std::uint32_t(now-searched_at)>1000))) {
        searched_at=now;
        if (const auto selected=select()) target=selected;
    }
    base();
}

void OmaMonster::run(std::uint32_t now,const std::function<std::optional<int>()>& select,const std::function<void()>& base) {
    if (!dead && !ghost && (std::uint32_t(now-searched_at)>8000 || (!target && std::uint32_t(now-searched_at)>1000))) {
        searched_at=now;
        if (const auto selected=select()) target=selected;
    }
    base();
}

void SpitSpiderMonster::spit(int facing,int x,int y,std::uint16_t dc,unsigned hit_point,
    const MonsterPursuit::Random& random,const std::function<void()>& announce,
    const Query& query,const std::function<int(int)>& mitigate,const Damage& apply) {
    // 显式方向入口；不模拟参考 TargetInSpitRange 近距离未初始化的输出参数。
    if (facing<0 || facing>=8) return;
    direction=facing;
    const int low=dc&0xff,high=dc>>8,span=high-low+1;
    // 正常数据库值 high>=low。异常负除数仍按C++余数处理，不把它转换为无符号上限。
    int damage=low;
    if (span!=0) damage+=int(random(unsigned(std::abs(span))));
    if (damage<=0) return;
    announce();
    // g_SpitMap 各方向只有两个直线格；原 SpitAttack 只扫描 i,k<4，正向第二格被裁掉。
    constexpr int dx[8]={0,1,1,1,0,-1,-1,-1};
    constexpr int dy[8]={-1,-1,0,1,1,1,0,-1};
    for (int row=0;row<4;++row) for (int column=0;column<4;++column) {
        const int ox=column-2,oy=row-2;
        if (!((ox==dx[facing] && oy==dy[facing]) || (ox==2*dx[facing] && oy==2*dy[facing]))) continue;
        const auto victim=query(x+ox,y+oy);
        if (!victim || victim->self || !victim->proper || !victim->speed) continue;
        if (random(victim->speed)>=hit_point) continue;
        // 原参考在攻击者上调用减伤，且修改共享 nDamage；不按目标重置初始伤害。
        damage=mitigate(damage);
        if (damage>0) apply(victim->id,damage,300);
    }
}

void WhiteSkeletonMonster::run(std::uint32_t now,const SummonNotify& notify,const SummonBaseRun& base) {
    if (first) {
        first=false;direction=5;hidden=false;
        notify(SummonEvent::emerge);
        walked_at=now+2000;
    }
    base();
}

void ElfMonster::appear_now(std::uint32_t now) {
    first=false;hidden=false;walked_at=now+800;
}

void ElfWarriorMonster::appear_now(std::uint32_t now,const SummonNotify& notify) {
    first=false;hidden=false;
    notify(SummonEvent::emerge);
    walked_at=now+800;
}

void ElfMonster::run(std::uint32_t now,const std::function<ElfWarriorMonster*()>& clone,
    const SummonNotify& notify,const SummonBaseRun& base) {
    if (first) {
        first=false;hidden=false;notify(SummonEvent::emerge);walked_at=now+2000;
    }
    if (dead) {
        if (std::uint32_t(now-death_at)>2000) notify(SummonEvent::make_ghost);
    } else if (has_target || master_has_target) {
        if (auto* warrior=clone()) warrior->appear_now(now,notify);
        // 参考在克隆失败时同样解除主人并 KickException；此分支不再运行基类。
        notify(SummonEvent::release_master);
        notify(SummonEvent::remove);
        return;
    }
    base();
}

void ElfWarriorMonster::run(std::uint32_t now,const std::function<ElfMonster*()>& clone,
    const SummonNotify& notify,const SummonBaseRun& base) {
    if (first) {
        first=false;hidden=false;notify(SummonEvent::emerge);walked_at=now+2000;
    }
    if (dead) {
        if (std::uint32_t(now-death_at)>2000) notify(SummonEvent::make_ghost);
    } else if (!has_target && !master_has_target) {
        if (auto* elf=clone()) {
            notify(SummonEvent::retreat);
            notify(SummonEvent::change_face);
            elf->appear_now(now);
        }
        notify(SummonEvent::release_master);
        notify(SummonEvent::remove);
    }
    // 战斗形态的参考 Run 没有 return，移除必须延迟到本轮基类调用之后。
    base();
}

RonMonster::RonMonster(Time now,unsigned attack_interval_ms)
    : searched_at_(now),hit_at_(now),attack_interval_ms_(attack_interval_ms) {}

bool RonMonster::run(Time now,int x,int y,const SelectTarget& select_target,const QueryObjects& query,const Attack& attack) {
    if (dead || ghost || petrified) return false;
    if (Time(now-searched_at_)>8000 || (!target && Time(now-searched_at_)>1000)) {
        searched_at_=now;
        if (const auto selected=select_target()) target=selected;
    }
    if (!target || !ZumaAi::within(x,y,target->x,target->y,5) || Time(now-hit_at_)<=attack_interval_ms_) return false;
    hit_at_=now;
    // CRonObject::AroundAttack 使用已缓存的 TargetDir，不重新朝向远处主目标。
    facing=target_direction;
    for (const int victim:query(x,y,1)) attack(victim);
    // 即使周围没有对象，原参考仍广播 RM_HIT。
    return true;
}

LightningZombieMonster::LightningZombieMonster(Time now,unsigned walk_interval_ms,unsigned attack_interval_ms)
    : walked_at(now),searched_at_(now),hit_at_(now),walk_interval_ms_(walk_interval_ms),attack_interval_ms_(attack_interval_ms) {}

LightningZombieMonster::Result LightningZombieMonster::run(Time now,int x,int y,const SelectTarget& select_target,
    const MonsterPursuit::Random& random) {
    Result result;
    if (dead || ghost || petrified) return result;
    if (Time(now-searched_at_)>8000 || (!target && Time(now-searched_at_)>1000)) {
        searched_at_=now;
        if (const auto selected=select_target()) target=selected;
    }
    if (!target) return result;
    // 此专用分支只读取 WalkTime；基类执行移动后才更新它。
    if (Time(now-walked_at)>walk_interval_ms_ && ZumaAi::within(x,y,target->x,target->y,4)) {
        if (ZumaAi::within(x,y,target->x,target->y,2) && random(3)!=0) {
            result.early_base=true;
            return result;
        }
        result.back_position=true;
    }
    if (ZumaAi::within(x,y,target->x,target->y,5) && Time(now-hit_at_)>attack_interval_ms_) {
        hit_at_=now;
        result.lightning=true;
        result.direction=MonsterPursuit::direction(x,y,target->x,target->y);
        // CCharObject::GetNextDirection 同格返回 DR_DOWN，通用近战 direction 的 -1 不适用。
        if (result.direction<0) result.direction=4;
    }
    return result;
}

RockManMonster::Action RockManMonster::run(Time now,int x,int y,std::span<const MonsterAiTarget> visible,
    const SelectTarget& select_target) {
    if (dead || ghost) return Action::idle;
    if (hidden) {
        // CRockManObject::Run 在隐藏分支每次都写 WalkTime，不依赖是否显现成功。
        walked_at=now+1000;
        for (const auto& candidate:visible) {
            if (MonsterPursuit::eligible(candidate) && ZumaAi::within(x,y,candidate.x,candidate.y,8)) {
                hidden=false;
                return Action::emerge;
            }
        }
    } else if (Time(now-searched_at)>8000 || (!target_id && Time(now-searched_at)>1000)) {
        searched_at=now;
        // MonsterNormalAttack 未找到新敌人时保留已有目标；基类负责目标失效。
        if (const auto selected=select_target()) target_id=selected;
        if (!target_id) {
            hidden=true;
            return Action::retreat;
        }
    }
    return Action::idle;
}

SandMobMonster::SandMobMonster(Time now,unsigned walk_interval_ms,unsigned attack_interval_ms)
    : walked_at(now),hit_at(now),walk_interval_ms_(walk_interval_ms),attack_interval_ms_(attack_interval_ms) {}

SandMobMonster::Result SandMobMonster::run(Time now,int x,int y,unsigned hp,unsigned maximum,
    std::span<const MonsterAiTarget> visible,const MonsterAiTarget* target) {
    Result result;
    if (dead || ghost || Time(now-walked_at)<=walk_interval_ms_) return result;
    walked_at=now;
    if (hidden) {
        if (hp>maximum/20 && std::any_of(visible.begin(),visible.end(),[&](const auto& candidate) {
            return MonsterPursuit::eligible(candidate) && ZumaAi::within(x,y,candidate.x,candidate.y,8);
        })) appeared_at=now;
        // 参考代码仅写 AppearStart，没有 ComeOut，不伪造显现事件。
        return result;
    }
    if (hp>0 && hp<maximum/20 && Time(now-appeared_at)>3000) {
        hidden=true;
        result.retreat=true;
    } else if (target && std::abs(std::int64_t(x)-target->x)>15 && std::abs(std::int64_t(y)-target->y)>15) {
        hidden=true;
        result.retreat=true;
        result.skip_base=true;
        return result;
    }
    // 低血钻回后仍执行 MonsterNormalAttack 的条件检查；远离分支则提前返回。
    result.search=Time(now-hit_at)>attack_interval_ms_;
    result.attack_target=!hidden;
    return result;
}

BeeQueenMonster::BeeQueenMonster(Time now,unsigned walk_interval_ms,unsigned attack_interval_ms)
    : walked_at_(now),hit_at_(now),walk_interval_ms_(walk_interval_ms),attack_interval_ms_(attack_interval_ms) {}

unsigned BeeQueenMonster::operate(Time now,const Spawn& spawn) {
    // CBeeQueen::Operate 只扫描调用开始时队列里的消息；到期时间相等即可投递。
    const auto count=deliveries_.size();
    unsigned spawned=0;
    for (std::size_t index=0;index<count;++index) {
        const auto due=deliveries_.front();
        deliveries_.pop_front();
        if (now<due) { deliveries_.push_back(due);continue; }
        if (const auto child=spawn(target_id)) {
            slaves.push_back(*child);
            ++spawned;
        }
    }
    return spawned;
}

bool BeeQueenMonster::run(Time now,const SelectTarget& select_target,const Alive& alive) {
    if (dead || ghost || petrified) return false;
    bool attacked=false;
    if (now-walked_at_>walk_interval_ms_) {
        walked_at_=now;
        if (now-hit_at_>attack_interval_ms_) {
            hit_at_=now;
            // MonsterNormalAttack 没找到新目标时，不主动清除原目标。
            if (const auto selected=select_target()) target_id=selected;
            if (target_id && slaves.size()<15) {
                deliveries_.push_back(now+500);
                attacked=true;
            }
        }
    }
    // 原版先排程，再移除死亡/幽灵随从；刚释放的名额不能用于本轮排程。
    std::erase_if(slaves,[&](int child) {return !alive(child);});
    return attacked;
}

bool MonsterPursuit::eligible(const MonsterAiTarget& target) {
        return target.alive && target.proper && !target.inspector;
    }

std::int64_t MonsterPursuit::distance(int x,int y,const MonsterAiTarget& target) {
        return std::abs(std::int64_t(x)-target.x)+std::abs(std::int64_t(y)-target.y);
    }

int MonsterPursuit::direction(int x,int y,int tx,int ty) {
        const int dx=(tx>x)-(tx<x),dy=(ty>y)-(ty<y);
        for (int index=0;index<8;++index)
            if (directions[index][0]==dx && directions[index][1]==dy) return index;
        return -1;
    }

MonsterPursuit::Action MonsterPursuit::tick(Time now,int x,int y,std::span<const MonsterAiTarget> visible,const Walk& walk,const Attack& attack,const Random& random) {
        const MonsterAiTarget* target=nullptr;
        if (target_id) {
            for (const auto& candidate:visible) if(candidate.id==*target_id) {target=&candidate;break;}
            if (!target || !eligible(*target) || now-focused_at>30000 ||
                std::abs(std::int64_t(x)-target->x)>15 || std::abs(std::int64_t(y)-target->y)>15) {
                target=nullptr;target_id.reset();
            }
        }
        // 参考实现按视野列表顺序选第一个，并非每帧寻找最近目标。
        if (now-searched_at>8000 || (!target && now-searched_at>1000)) {
            searched_at=now;
            for (const auto& candidate:visible) {
                if (!eligible(candidate) || candidate.hidden || distance(x,y,candidate)>=999) continue;
                target=&candidate;target_id=candidate.id;focused_at=now;break;
            }
        }
        if (!target) return Action::idle;
        const int dir=direction(x,y,target->x,target->y);
        if (dir<0) return Action::idle; // 同格不在近战攻击范围内。
        const int range=burst_limit?7:1;
        if (std::abs(std::int64_t(x)-target->x)<=range && std::abs(std::int64_t(y)-target->y)<=range) {
            if (now<=next_hit) return Action::idle; // 等攻击冷却时不能继续向目标格走。
            if (burst_limit && burst_shots==burst_limit) {
                next_hit=now+attack_interval_ms;
                if (random(5)==0) burst_shots=0;
                return Action::idle;
            }
            if (!attack(dir,*target)) return Action::blocked;
            if (burst_limit) ++burst_shots;
            facing=dir;next_hit=now+attack_interval_ms;focused_at=now;
            return Action::attack;
        }
        // GotoTargetXY只查看视野列表首项；较近时切换目标，本轮不移动。
        if (!visible.empty() && eligible(visible.front()) &&
            distance(x,y,visible.front())<distance(x,y,*target)) {
            target_id=visible.front().id;focused_at=now;return Action::idle;
        }
        int candidate=dir;
        for (int attempt=0;attempt<8;++attempt) {
            if (walk(x+directions[candidate][0],y+directions[candidate][1],candidate)) {
                facing=candidate;return Action::walk;
            }
            if (attempt<7) candidate=(candidate+(random(3)?1:7))%8;
        }
        return Action::blocked;
    }

bool ZumaAi::within(int x,int y,int other_x,int other_y,int radius) {
        return std::abs(std::int64_t(x)-other_x)<=radius && std::abs(std::int64_t(y)-other_y)<=radius;
    }

ZumaAi::Result ZumaAi::tick(Time now,const Visible& visible,const Spawn& spawn,const Alive& alive,const Random& random) {
        Result result;
        const auto wake=[&](Actor& actor) {
            actor.stone=false;
            result.woke.push_back(actor.id);
        };
        for (auto& [id,actor]:actors) {
            if (actor.dead || actor.ghost || actor.petrified) continue;
            if (now>actor.next_turn) {
                if (actor.stone) {
                    const auto targets=visible(id);
                    const bool near=std::any_of(targets.begin(),targets.end(),[&](const Target& target) {
                        return target.alive && target.proper && !target.inspector &&
                            within(actor.x,actor.y,target.x,target.y,2);
                    });
                    if (near) {
                        wake(actor);
                        if (actor.kind==ZumaKind::statue) {
                            // MeltStoneAll不递归，且只影响两种卫士，不唤醒教主。
                            // GetMapObject排除死亡/幽灵；不会排除石化中但仍活着的邻居。
                            for (auto& [other_id,other]:actors)
                                if (other.stone && !other.dead && !other.ghost && other.kind==ZumaKind::statue &&
                                    within(actor.x,actor.y,other.x,other.y,7)) wake(other);
                        }
                        actor.next_turn=now+2000+actor.walk_interval_ms;
                    }
                } else {
                    if (actor.kind==ZumaKind::king && actor.max_hp>0) {
                        // 保留参考C++整数除法次序，不暗改为(HP*5)/MaxHP。
                        if (actor.danger>0 && (actor.hp/actor.max_hp)*5<actor.danger) {
                            --actor.danger;
                            const auto count=6+random(6);
                            constexpr std::array prototypes{ZumaSlavePrototype::archer,ZumaSlavePrototype::fanatic,
                                ZumaSlavePrototype::guardian,ZumaSlavePrototype::moth};
                            for (unsigned index=0;index<count && actor.slaves.size()<30;++index) {
                                const auto spawned=spawn(actor,prototypes[random(4)]);
                                if (spawned) actor.slaves.push_back(*spawned);
                            }
                        }
                        if (actor.hp==actor.max_hp) actor.danger=5;
                    }
                    actor.next_turn=now+actor.walk_interval_ms;
                    result.ready.push_back(id);
                }
            }
            // 原参考Run先尝试召唤，随后清理已死亡随从；本轮释放的名额下轮可用。
            if (actor.kind==ZumaKind::king)
                std::erase_if(actor.slaves,[&](int slave) { return !alive(slave); });
        }
        return result;
    }

std::optional<MonsterPrototype> reference_zuma_prototype(ZumaSlavePrototype prototype) {
    switch (prototype) {
    case ZumaSlavePrototype::archer: return MonsterPrototype{MonsterAppearance::zuma_archer,"祖玛弓箭手"};
    case ZumaSlavePrototype::fanatic: return MonsterPrototype{MonsterAppearance::zuma_fanatic,"祖玛狂信徒"};
    case ZumaSlavePrototype::guardian: return MonsterPrototype{MonsterAppearance::zuma_guardian,"祖玛卫士"};
    case ZumaSlavePrototype::moth: return MonsterPrototype{MonsterAppearance::wedge_moth,"楔蛾"};
    }
    return {};
}

MonsterBehavior MonsterBehavior::for_shape(int shape) {
        if (shape==83 || shape==84) return {Birth::dormant_statue,6,120,-1};
        if (shape==85) return {Birth::dormant_statue,20,120,0};
        return {};
    }

bool MonsterBehavior::dormant(std::uint32_t flags) const {
        return birth==Birth::dormant_statue && (flags&dormant_flag)!=0;
    }

int MonsterBehavior::attack_message(int shape) {
        // 40B0A0 的外层 switch(shape-7)，内部方向分支不是外观号。
        switch (shape) {
        case 7: case 19: case 33: case 46: case 82: case 86: case 95: case 96:
        case 124: case 125: case 144: case 145: case 146: case 156: case 167:
        case 169: case 183: return protocol::SM_FLYAXE;
        default: return protocol::SM_HIT;
        }
    }

unsigned reference_ranged_burst(MonsterAppearance appearance) {
    switch (appearance) {
    case MonsterAppearance::skeleton_axe_thrower: return 1;
    case MonsterAppearance::spined_dark_lizard: return 2;
    case MonsterAppearance::zuma_archer: return 5;
    default: return 0;
    }
}

const MonsterAiTarget* PlantAi::target(std::span<const MonsterAiTarget> visible) const {
    for (const auto& candidate:visible)
        if (target_id==candidate.id && MonsterPursuit::eligible(candidate)) return &candidate;
    return nullptr;
}

bool PlantAi::fight(std::uint64_t now,int x,int y,std::span<const MonsterAiTarget> visible,const MonsterPursuit::Attack& attack) {
    if (hidden) return false;
    if (!target(visible)) target_id.reset();
    if (now<=next_hit) return false;
    // CStickObject 每次攻击冷却结束重新按视野列表顺序选敌，不走通用追踪计时。
    for (const auto& candidate:visible) {
        if (MonsterPursuit::eligible(candidate) && !candidate.hidden &&
            MonsterPursuit::distance(x,y,candidate)<999) {
            target_id=candidate.id;
            break;
        }
    }
    const auto selected=target(visible);
    if (!selected || std::abs(std::int64_t(x)-selected->x)>1 ||
                     std::abs(std::int64_t(y)-selected->y)>1) return false;
    const int direction=MonsterPursuit::direction(x,y,selected->x,selected->y);
    if (direction<0 || !attack(direction,*selected)) return false;
    facing=direction;
    next_hit=now+attack_interval_ms;
    return true;
}

PlantAi::Action PlantAi::tick(std::uint64_t now,int x,int y,bool dead,bool ghost,
    std::span<const MonsterAiTarget> visible,const MonsterAiTarget* target) {
    if (dead || ghost || now-walked_at<=walk_interval_ms) return Action::idle;
    walked_at=now;
    if (hidden) {
        for (const auto& candidate:visible) {
            // CheckComeOut 不检查目标的隐藏位，与通用主动选敌条件不同。
            if (MonsterPursuit::eligible(candidate) &&
                std::abs(std::int64_t(x)-candidate.x)<=4 &&
                std::abs(std::int64_t(y)-candidate.y)<=4) {
                hidden=false;
                return Action::emerge;
            }
        }
        return Action::idle;
    }
    if (target && std::abs(std::int64_t(x)-target->x)>4 &&
                  std::abs(std::int64_t(y)-target->y)>4) {
        hidden=true;
        return Action::retreat;
    }
    return Action::combat;
}


std::string ordinary_monster_archive(int library) {
    // 47DA90 registers these archives relative to the first library slot.
    if (library>=88 && library<=107) return "Mon-"+std::to_string(library-87);
    if (library>=108 && library<=127) return "MonS-"+std::to_string(library-107);
    switch (library) {
    case 135: return "DMon-1";
    case 136: return "DMonS-1";
    case 141: return "Mon-54";
    default: return {};
    }
}

MonsterMotion monster_idle_motion(int shape) {
    if (shape==95 || shape<10) return {0,1,250};
    if ((shape>=530 && shape<=533) || shape==538) return {0,1,200};
    if (shape>=145 && shape<=147) return {0,4,150};
    if (shape==158) return {0,4,200};
    if (shape==161 || shape==165 || shape==166) return {0,4,250};
    return {0,4,300};
}

MonsterMotion monster_attack_motion(int shape,int action) {
    if (action==7) return shape==141?MonsterMotion{560,6,120}:MonsterMotion{560,10,150};
    if (action==5) {
        switch (shape) {
        case 159: case 160: return {400,6,140};
        case 164: return {400,9,120};
        default: return {400,6,150};
        }
    }
    if (action==6) {
        switch (shape) {
        case 10: case 181: return {400,6,150};
        case 13: case 157: case 182: case 185: return {480,6,150};
        case 139: case 158: return {480,6,200};
        case 141: return {480,6,120};
        case 161: return {480,6,140};
        case 164: case 186: return {480,8,150};
        default: return {480,10,150};
        }
    }
    // 46F690 selects the attack row by the current feature, including alternate bases.
    switch (shape) {
    case 0: case 1: case 2: case 12: case 99: return {160,6,100};
    case 3: case 5: case 6: case 7: case 8: case 9: case 15: case 19:
    case 93: case 96: case 97: case 98: case 159: case 160: case 161: return {160,6,140};
    case 4: case 10: case 11: case 13: case 17: case 18: case 94:
    case 145: case 146: case 149: case 156: case 157: case 165: case 166: return {160,6,150};
    case 14: case 16: case 92: case 147: case 154: case 155: case 162: case 163: return {160,6,120};
    case 33: case 46: case 82: return {160,6,90};
    case 90: return {160,6,160};
    case 95: case 120: return {160,10,90};
    case 164: return {160,9,120};
    case 186: return {400,8,90};
    default: return {160,6,130};
    }
}

MonsterMotion monster_struck_motion(int shape) {
    if (shape>=116 && shape<=118) return {240,3,150};
    if (shape>=145 && shape<=147) return {240,2,150};
    if (shape==149 || (shape>=154 && shape<=157) || (shape>=159 && shape<=164)) return {240,2,300};
    if (shape==158 || shape==165 || shape==166 || (shape>=530 && shape<=533) || shape==538)
        return {240,2,200};
    return {240,2,180};
}

MonsterMotion monster_death_motion(int shape) {
    switch (shape) {
    case 8: case 86: case 94: case 105: return {320,10,130};
    case 30: case 37: return {320,4,220};
    case 104: return {320,20,120};
    case 127: case 153: case 179: case 187: case 188: case 189:
    case 534: case 535: case 536: case 537: return {320,9,120};
    case 141: return {720,20,120};
    case 145: case 146: case 147: case 154: case 155: return {320,10,95};
    case 149: case 162: case 163: case 164: return {320,10,120};
    case 156: case 157: return {320,10,145};
    case 158: case 165: case 166: return {320,10,150};
    case 159: case 160: case 161: return {320,10,110};
    case 530: case 531: case 532: case 533: case 538: return {320,8,120};
    default: return {320,10,140};
    }
}

MonsterMotion monster_burrow_motion(int shape) {
    if (shape==83 || shape==84) return {640,6,120};
    if (shape==85) return {640,20,120};
    if (shape==90) return {640,10,100};
    if ((shape>=530 && shape<=533) || shape==538) return {640,7,120};
    return {640,10,120};
}

std::uint32_t monster_feature_status(std::string_view feature) {
    std::uint32_t flags=0;
    for (int index=0;index<4;++index)
        flags|=std::uint32_t(std::uint8_t(feature[4+index]))<<(8*index);
    return flags;
}

int monster_feature_shape(std::string_view feature) {
    return std::uint8_t(feature[2])|(int(std::uint8_t(feature[3]))<<8);
}

int actor_status_color(std::uint32_t flags) {
    // 405C40 resolves overlapping status masks in this priority order.
    if (flags&0x00020000u) return 1;
    if (flags&0x80000000u) return 2016;
    if (flags&0x40000000u) return 64690;
    if (flags&0x08000000u) return 38079;
    if (flags&0x20000000u) return 64511;
    if (flags&0x04000000u) return 0;
    if (flags&0x00800000u) return 1;
    return 65535;
}

int monster_body_dye(int shape) {
    if (shape<2000) return 65535;
    // 4014A0 packs the selected COLORREF into the original RGB565 surface.
    constexpr std::array<std::uint32_t,10> palette{
        0xffffff,13132900,0x5a5ab4,16751430,0x8080ff,16742520,0x646464,11842740,0x64b4b4,15349364};
    const auto color=palette[shape%10];
    return mir3::Rgb565::pack(color&255,(color>>8)&255,(color>>16)&255);
}

int monster_resource_direction(int shape,int action_base,int facing) {
    // 406750 applies the direction rules again whenever the monster starts an action.
    switch (shape) {
    case 31: return action_base==0?0:facing;
    case 67: return action_base==160?1:0;
    case 73: case 104: case 127: case 153: case 179:
    case 187: case 188: case 189: case 534: case 535: case 536: case 537: return 0;
    case 530: case 531: case 532: case 533: case 538: return action_base==320?0:facing;
    case 106: return 1;
    default: return facing;
    }
}
}
#else
#include "godot/hud_view.hpp"
#include "protocol/receive_message.hpp"
#include "hud/selection_pixels.hpp"
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <climits>
#include <cmath>
using namespace godot;
using namespace mir3;
namespace {
Ref<ImageTexture> texture(const std::vector<std::uint8_t>& rgba,int width,int height) {
    if (rgba.empty()) return {};
    PackedByteArray bytes;bytes.resize(rgba.size());
    std::copy(rgba.begin(),rgba.end(),bytes.ptrw());
    return ImageTexture::create_from_image(Image::create_from_data(width,height,false,Image::FORMAT_RGBA8,bytes));
}
}
int mir3::monster_projectile_direction(Vector2 delta,bool eight_directions) {
    // 453C00 / 453D20 compare the leading axis against successive cosine thresholds.
    if (delta==Vector2()) return 0;
    const int count=eight_directions?8:16;
    const int quadrant=delta.x<0?(delta.y<0?3:2):(delta.y<0?0:1);
    const double leading=(quadrant%2)?std::abs(delta.x):std::abs(delta.y);
    // The original tables contain rounded float constants, not exact cosines.
    constexpr std::array<float,8> thresholds{
        1.f,0.980785f,0.923880f,0.831470f,0.707107f,0.555570f,0.382683f,0.195090f};
    const double cosine=leading/std::hypot(double(delta.x),double(delta.y));
    const int stride=eight_directions?2:1;
    int slot=0;
    while (slot<count/2 && cosine<=thresholds[slot*stride]) ++slot;
    return (quadrant*(count/4)+slot/2)%count;
}

bool Mir3HudView::set_scene_chicken(int actor,Vector2i cell) {
    return terrain.walkable(cell.x,cell.y) && place_scene_chicken(actor,cell,6);
}

bool Mir3HudView::place_scene_chicken(int actor,Vector2i cell,int facing,int shape,unsigned light) {
    if (actor==local_actor_identity) return false;
    if (!terrain_ready || actor<=0 || scene_npcs.contains(actor) || scene_humans.contains(actor) || !terrain.contains(cell.x,cell.y)) return false;
    SceneChicken chicken{};
    chicken.cell=cell;
    chicken.facing=facing;
    if (!refresh_monster_shape(chicken,shape)) return false;
    chicken.light_attribute=light;
    if (const auto previous=scene_chickens.find(actor);previous!=scene_chickens.end())
        chicken.called=previous->second.called;
    scene_chickens.insert_or_assign(actor,std::move(chicken));
    monster_ai.actors.erase(actor);
    queue_redraw();
    return true;
}

void Mir3HudView::play_animal_call(int actor,SceneChicken& animal) {
    if (weather.random()%(animal.called?25:3)!=0) return;
    animal.called=true;
    play_actor_sound(actor,10*animal.shape+201,animal.cell);
}

const Mir3HudView::MapActorFrame* Mir3HudView::monster_frame(int shape,int action_base,int facing,int frame_offset,bool shadow) {
    // 405E50: ordinary monster archives group ten shapes, 1000 slots per shape.
    if (shape<0 || (shape>=1000 && shape<2000)) return nullptr;
    int library=shape<1000?88+shape/10:((shape-2000)/100-121)&255;
    if (shadow) {
        if (shape>=530 && shape<=533) return nullptr;
        if (shape>=2000 && library==135) ++library;
        else if (shape<200 && library>=88 && library<=107) library+=20;
        else return nullptr;
    }
    if (library>=150) return nullptr;
    const int group=shape<1000?shape%10:(shape-2000)/10;
    const bool death=action_base==320;
    const auto motion=monster_death_motion(shape);
    const int resource_facing=monster_resource_direction(shape,action_base,facing);
    const int direction_stride=death && shape==141?20:10;
    const int id=group*1000+(death?motion.base:action_base)+resource_facing*direction_stride+frame_offset;
    const auto key=std::pair{library,id};
    const auto name=ordinary_monster_archive(library);
    if (name.empty()) return nullptr;
    auto [archive,opened]=monster_archives.try_emplace(library);
    if (opened) {
        const auto path=std::filesystem::u8path(actor_resource_root.utf8().get_data())/name;
        mir3::WilArchive candidate;
        if (candidate.open(path)) archive->second.emplace(std::move(candidate));
    }
    if (!archive->second) return nullptr;
    auto [cached,inserted]=monster_frames.try_emplace(key);
    if (inserted) {
        if (const auto frame=archive->second->frame(id))
            cached->second=MapActorFrame{texture(frame->pixels.rgba,frame->width,frame->height),
                Vector2(frame->offset_x,frame->offset_y),frame->pixels.regions,
                frame->shadow_mode,Vector2(frame->shadow_x,frame->shadow_y)};
    }
    return cached->second?&*cached->second:nullptr;
}

const Mir3HudView::MapActorFrame* Mir3HudView::project_monster_shadow(const MapActorFrame& body,bool flat) {
    // 48F4AB mode 49 keeps even rows and shears each one. Mode 50 keeps the silhouette.
    if (!flat && body.shadow_mode!=49 && body.shadow_mode!=50) return nullptr;
    const auto key=std::pair{std::uint64_t(body.image->get_instance_id()),flat};
    if (const auto cached=projected_monster_shadows.find(key);cached!=projected_monster_shadows.end())
        return &cached->second;
    const int width=body.image->get_width(), height=body.image->get_height();
    const bool shear=!flat && body.shadow_mode==49;
    const int projected_width=width+(shear?height/2:0);
    const int projected_height=shear?(height+1)/2:height;
    std::vector<std::uint8_t> rgba(std::size_t(projected_width)*projected_height*4);
    for (int row=0;row<height;row+=shear?2:1) {
        const int x_offset=shear?(height-row)/2:0;
        const int y=shear?row/2:row;
        for (int x=0;x<width;++x)
            if (body.regions[std::size_t(row)*width+x])
                rgba[(std::size_t(y)*projected_width+x+x_offset)*4+3]=255;
    }
    return &projected_monster_shadows.emplace(key,MapActorFrame{
        texture(rgba,projected_width,projected_height),flat?body.offset+Vector2(3,2):body.shadow_offset,{}}).first->second;
}

bool Mir3HudView::refresh_monster_shape(SceneChicken& monster,int shape) {
    // 4064B0 validates the feature. 40ECB0 loads the current frame at draw time.
    const int library=shape<2000?(88+shape/10)&255:((shape-2000)/100-121)&255;
    if (shape<0 || library>=150) return false;
    if (monster.shape!=shape) {
        monster.shape=shape;
        monster.idle_frame%=monster_idle_motion(shape).frames;
    }
    return true;
}

bool Mir3HudView::move_scene_chicken(int actor,SceneChicken::Turn turn) {
    const auto [cell,facing,shape,light,backward,status,force_walk]=turn;
    auto& chicken=scene_chickens.at(actor);
    if (chicken.death_frame>=0 || chicken.buried || chicken.burrow_frame>=0) return true;
    const bool walking=cell!=chicken.cell || backward || force_walk;
    if (!refresh_monster_shape(chicken,shape)) return false;
    chicken.status_flags=status;
    if (!walking) {
        chicken.facing=facing;
        chicken.light_attribute=light;
        update_actor_bar_status(actor,(status&2)!=0);
        queue_redraw();
        return true;
    }
    const auto direction=mir3::map_directions[backward?(facing+4)%8:facing];
    chicken.walk_origin=cell-Vector2i(direction[0],direction[1]);
    chicken.cell=cell;
    chicken.facing=facing;
    chicken.light_attribute=light;
    chicken.hidden_pose=false;
    chicken.walk_frame=0;
    chicken.walking_backward=backward;
    chicken.skip_movement_tick=false;
    update_actor_bar_status(actor,(status&2)!=0);
    chicken.walk_catch_up=false;
    queue_redraw();
    return true;
}

bool Mir3HudView::apply_monster_hit(int actor,const SceneChicken::Hit& hit) {
    auto& chicken=scene_chickens.at(actor);
    const auto& p=hit.health;
    const int shape=hit.shape;
    if (!refresh_monster_shape(chicken,shape)) return false;
    mir3::SceneHumanMessage message{};
    message.opcode=mir3::protocol::SM_HUMAN_STRUCK;
    message.health=p;
    apply_actor_health_record(actor,message);
    if (chicken.death_frame>=0) {
        // 40975C selects struck for nonzero damage; 40DAC0 then restores the corpse.
        if (p[2]) chicken.skeleton=false;
        if (!chicken.skeleton) chicken.death_frame=monster_death_motion(chicken.shape).frames-1;
        return true;
    }
    if (chicken.buried) return true;
    if (p[2]) {
        if (hit.variant<=65535-400) show_following_hit_effect(400+hit.variant,actor,chicken.cell);
    }
    if (!p[0]) {
        chicken.walk_frame=-1;
        chicken.struck_frame=-1;
        chicken.death_frame=0;
        chicken.death_ms=0;
        chicken.death_catch_up=false;
        play_monster_death_effect(actor,chicken);
        update_scene_target(inventory_cursor);
        target_focus.remove(actor);
    } else if (p[2]) {
        chicken.walk_frame=-1;
        chicken.struck_frame=0;
        chicken.struck_ms=0;
        chicken.struck_catch_up=false;
        if (shape==161) {
            show_scene_effect(mir3::EffectCatalog::Section::spell,585,chicken.cell,
                10*chicken.facing,0,actor,std::nullopt,MonsterEffectPose::struck);
            play_actor_sound(actor,10*(585+1000),chicken.cell);
        }
    }
    return true;
}

bool Mir3HudView::apply_monster_death(int actor,const SceneChicken::Death& death) {
    const auto [cell,facing,packet_shape,opcode,light,status]=death;
    const bool skeleton=opcode==mir3::protocol::SM_SKELETON;
    const int shape=packet_shape>=0?packet_shape:scene_chickens.at(actor).shape;
    const int frames=skeleton?1:monster_death_motion(shape).frames;
    // 4093B0 refreshes the feature before selecting death or skeleton.
    if (const auto existing=scene_chickens.find(actor);existing!=scene_chickens.end()) {
        if (!refresh_monster_shape(existing->second,shape)) return false;
    } else {
        const int base=skeleton?880:320;
        for (int i=0;i<frames;++i) if (!monster_frame(shape,base,facing,i)) return false;
    }
    if (!scene_chickens.contains(actor) && !place_scene_chicken(actor,cell,facing,shape)) return false;
    auto& chicken=scene_chickens.at(actor);
    chicken.cell=cell;
    chicken.facing=facing;
    if (status) { chicken.status_flags=*status;update_actor_bar_status(actor,(*status&2)!=0); }
    chicken.walk_frame=-1;
    chicken.struck_frame=-1;
    chicken.burrow_frame=-1;
    chicken.attack_frame=-1;
    chicken.buried=false;
    chicken.light_attribute=light;
    if (auto health=scene_health.find(actor);health!=scene_health.end()) health->second.empty_bar=true;
    const bool animated=opcode==mir3::protocol::SM_NOWDEATH;
    // 40947A selects action 4 for opcode 34. If already dead, 40DAC0 selects its last frame.
    chicken.death_frame=animated && chicken.death_frame<0?0:frames-1;
    chicken.death_ms=0;
    chicken.death_catch_up=false;
    chicken.skeleton=skeleton;
    if (animated) play_monster_death_effect(actor,chicken);
    update_scene_target(inventory_cursor);
    target_focus.remove(actor);
    queue_redraw();
    return true;
}

bool Mir3HudView::configure_reference_animal_combat(int actor,int hp,int dc,int ac,int hit,int speed,int level) {
    if (!bool(get_meta("offline_preview",false)) || hp<=0 || hp>65535 || dc<0 || dc>65535 || ac<0 || ac>65535 ||
        hit<0 || hit>255 || speed<0 || speed>255 || level<0 || level>255 || (hit>0 && speed==0)) return false;
    const auto found=animals.find(actor);
    const auto body=scene_chickens.find(actor);
    if (found==animals.end() || found->second.vitals || body==scene_chickens.end() || body->second.death_frame>=0) return false;
    auto& animal=found->second;
    animal.vitals=mir3::MonsterVitals{};animal.vitals->hp=animal.vitals->maximum=hp;animal.vitals->ac=std::uint16_t(ac);
    animal.dc=std::uint16_t(dc);animal.hit=unsigned(hit);animal.speed=unsigned(speed);animal.level=unsigned(level);
    return apply_scene_health(actor,mir3::protocol::SM_HEALTHSPELLCHANGED,hp,0,hp);
}

bool Mir3HudView::notify_reference_animal_struck(int actor,int hitter,Vector2i hitter_cell,bool proper,int level) {
    if (!bool(get_meta("offline_preview",false)) || level<0 || level>255 || hitter<0 || hitter==actor) return false;
    const auto found=animals.find(actor);
    const auto body=scene_chickens.find(actor);
    if (found==animals.end() || body==scene_chickens.end() || body->second.death_frame>=0 || body->second.buried) return false;
    auto& animal=found->second;
    const mir3::MonsterRunState::Target target{hitter_cell.x,hitter_cell.y,false,false,true,proper};
    if (animal.vitals && animal.vitals->hp<=0) return false;
    if (animal.brain.state.struck(std::uint32_t(monster_ai_clock),unsigned(level),hitter?&target:nullptr)) animal.target_id=hitter;
    return true;
}

bool Mir3HudView::spawn_reference_animal(int actor,Vector2i cell,int shape,int facing,Vector2i mission) {
    if (!bool(get_meta("offline_preview",false)) || actor<=0 || actor==local_actor_identity || facing<0 || facing>7 ||
        !terrain_ready || !terrain.walkable(cell.x,cell.y) || scene_occupied(cell) || cell==player_cell ||
        scene_chickens.contains(actor) || scene_humans.contains(actor) || scene_npcs.contains(actor) ||
        plant_actors.contains(actor) || animals.contains(actor) || !monster_frame(shape,0,facing,0)) return false;
    if (!apply_monster_entry(actor,SceneChicken::Entry{cell,facing,shape,0,0,false,{}})) return false;
    AnimalActor animal;animal.shape=shape;
    auto& state=animal.brain.state;
    state.x=cell.x;state.y=cell.y;state.direction=facing;
    state.walked_at=state.thought_at=std::uint32_t(monster_ai_clock);
    state.walk_interval=600;state.hit_interval=1200;
    state.has_mission=mission.x>=0;
    state.mission_x=mission.x;state.mission_y=mission.y;
    animals.emplace(actor,std::move(animal));
    return true;
}
bool Mir3HudView::bind_reference_monster(int actor,int ai,int dc,int attack_delay,int move_delay,int element) {
    if (!bool(get_meta("offline_preview",false)) || !scene_chickens.contains(actor) || animals.contains(actor) ||
        plant_actors.contains(actor) || bee_queens.contains(actor) || bees.contains(actor) ||
        !mir3::zircon::Controller::supports(ai) || dc<0 || dc>65535 || attack_delay<100 || move_delay<0 || element<0 || element>7) return false;
    auto controller=std::make_unique<mir3::zircon::Controller>(mir3::zircon::RuntimeAI(ai));
    const auto& body=scene_chickens.at(actor);
    controller->place({body.cell.x,body.cell.y,body.facing});
    controller->configure(attack_delay,move_delay,mir3::zircon::Element(element));
    reference_monsters[actor]={std::move(controller),ai,dc,move_delay,0};return true;
}
bool Mir3HudView::configure_reference_combat(int actor,Dictionary stats) {
    if (!bool(get_meta("offline_preview",false))) return false;
    const auto found=reference_monsters.find(actor);
    if (found==reference_monsters.end() || found->second.combat) return false;
    mir3::zircon::CombatState state;
    state.hp=state.maximum_hp=int(stats.get("hp",0));state.mp=int(stats.get("mp",0));
    if (state.hp<=0 || state.hp>65535 || state.mp<0 || state.mp>65535) return false;
    state.accuracy=int(stats.get("accuracy",0));state.agility=int(stats.get("agility",0));
    state.min_ac=int(stats.get("min_ac",0));state.max_ac=int(stats.get("max_ac",0));
    state.min_mr=int(stats.get("min_mr",0));state.max_mr=int(stats.get("max_mr",0));
    state.defensive_mastery=int(stats.get("defensive_mastery",0));
    state.critical_chance=int(stats.get("critical_chance",0));state.critical_damage=int(stats.get("critical_damage",0));
    state.life_steal=int(stats.get("life_steal",0));state.shield_percent=int(stats.get("magic_shield",0));
    state.protection_ring=bool(stats.get("protection_ring",false));state.invincible=bool(stats.get("invincible",false));
    if (state.accuracy<0 || state.agility<0 || state.agility>65535 || state.critical_chance<0 || state.critical_chance>100 ||
        state.critical_damage<0 || state.critical_damage>1000 || state.life_steal<0 || state.life_steal>100 ||
        state.shield_percent<0 || state.shield_percent>100) return false;
    for (int value:{state.min_ac,state.max_ac,state.min_mr,state.max_mr}) if (value<0 || value>65535) return false;
    const Array resistance=stats.get("resistance",Array());
    if (!resistance.is_empty() && resistance.size()!=8) return false;
    for (int i=0;i<resistance.size();++i) {state.resistance[i]=int(resistance[i]);if (state.resistance[i]<-100 || state.resistance[i]>10) return false;}
    found->second.combat=state;
    found->second.combat_team=int(stats.get("team",0));
    return apply_scene_health(actor,mir3::protocol::SM_HEALTHSPELLCHANGED,state.hp,state.mp,state.maximum_hp);
}
Dictionary Mir3HudView::advance_reference_monsters(std::span<const mir3::MonsterAiTarget> targets) {
    using namespace mir3::zircon;
    using ReferenceTime=mir3::zircon::Time;
    Dictionary result;Array damage,magic,movement,attacks,packets;
    const auto deliver_145=[&](const mir3::Monster145Message& packet) {
        const auto text=packet.encode();PackedByteArray encoded;encoded.resize(text.size());
        std::copy(text.begin(),text.end(),encoded.ptrw());
        packets.append(encoded);
        return apply_monster_wire(packet.header.identifier,int(packet.header.recognition),packet.header.parameters,encoded).value();
    };
    const auto appearance_145=[](const SceneChicken& body) {
        return mir3::Monster145Appearance{std::uint16_t(body.shape),body.status_flags,std::uint8_t(body.light_attribute)};
    };
    for (auto& [id,actor]:reference_monsters) {
        const auto found=scene_chickens.find(id);if (found==scene_chickens.end()) continue;
        auto& body=found->second;
        CombatWorld world;
        world.random=[](unsigned bound){return bound?unsigned(UtilityFunctions::randi())%bound:0u;};
        const auto idle=[&]{return body.walk_frame<0 && body.attack_frame<0 && body.struck_frame<0 && body.burrow_frame<0 && body.actions.empty();};
        world.can_attack=idle;
        const auto eligible=[&](const mir3::MonsterAiTarget& t){
            const auto live=scene_chickens.find(t.id);
            if (live!=scene_chickens.end() && (live->second.death_frame>=0 || live->second.buried)) return false;
            const auto other=reference_monsters.find(t.id);
            return t.id!=id && mir3::MonsterPursuit::eligible(t) &&
                (other==reference_monsters.end() || (other->second.combat_team!=actor.combat_team &&
                    (!other->second.combat || !other->second.combat->dead)));
        };
        world.can_attack_target=[&](int victim){return std::any_of(targets.begin(),targets.end(),[&](const auto& t){return t.id==victim && eligible(t);});};
        world.target_position=[&](int victim){
            if (const auto live=scene_chickens.find(victim);live!=scene_chickens.end()) return std::pair{live->second.cell.x,live->second.cell.y};
            for (const auto& t:targets) if (t.id==victim) return std::pair{t.x,t.y};return std::pair{0,0};
        };
        world.get_targets=[&](int x,int y,int radius){std::vector<int> ids;for (const auto& t:targets) {
            const auto [tx,ty]=world.target_position(t.id);
            if (eligible(t) && mir3::ZumaAi::within(x,y,tx,ty,radius)) ids.push_back(t.id);
        }return ids;};
        world.get_dc=[&]{return actor.dc;};
        world.get_mc=world.get_dc;
        const auto free_cell=[&](int x,int y){return terrain.walkable(x,y) && !scene_occupied(Vector2i(x,y)) && Vector2i(x,y)!=player_cell &&
            std::none_of(targets.begin(),targets.end(),[&](const auto& t){
                // 场景对象已由scene_occupied检查实时位置；旧快照不能继续占住移走的格。
                return !scene_chickens.contains(t.id) && t.alive && t.x==x && t.y==y;
            });};
        world.walk=[&](int facing){
            if (!idle() || ReferenceTime(monster_ai_clock)<actor.move_at) return false;
            const auto d=mir3::MonsterPursuit::directions[facing];const auto to=body.cell+Vector2i(d[0],d[1]);
            if (!free_cell(to.x,to.y)) return false;
            if (!deliver_145(mir3::Monster145Message::pose(mir3::protocol::SM_WALK,id,to.x,to.y,facing,appearance_145(body)))) return false;
            actor.move_at=ReferenceTime(monster_ai_clock)+actor.move_delay;movement.append(id);return true;
        };
        world.move_to=[&](int x,int y){
            if (body.cell==Vector2i(x,y)) return;
            if (mir3::ZumaAi::within(body.cell.x,body.cell.y,x,y,1) && !free_cell(x,y)) return;
            int facing=mir3::MonsterPursuit::direction(body.cell.x,body.cell.y,x,y);
            const int rotation=world.random(2)==0?1:7;
            for (int i=0;i<8;++i) {if (world.walk(facing)) return;facing=(facing+rotation)%8;}
        };
        world.broadcast=[&](int facing,bool ranged,int victim){
            bool mapped=false;
            if (!ranged) mapped=deliver_145(mir3::Monster145Message::melee(id,body.cell.x,body.cell.y,facing));
            else if (victim) {
                const auto [tx,ty]=world.target_position(victim);
                mapped=deliver_145(mir3::Monster145Message::projectile(id,body.cell.x,body.cell.y,facing,tx,ty,victim));
            }
            // 空目标范围动作需逐招映射1.45特殊攻击，不能伪装成普通近战。
            Dictionary event;event["actor"]=id;event["ranged"]=ranged;event["target"]=victim;event["mapped_145"]=mapped;attacks.append(event);
        };
        std::vector<std::shared_ptr<CombatWorld::MagicCast>> casts;
        world.broadcast_magic=[&](auto cast){casts.push_back(std::move(cast));};
        world.cell=[&](int x,int y)->std::optional<CombatWorld::MagicCell>{
            // Zircon Map.Load只为可通行地形创建Cell；阻挡地形也返回null。
            if (!terrain.walkable(x,y)) return {};
            CombatWorld::MagicCell cell{x,y,{}};std::vector<int> objects;
            for (const auto& t:targets) if (world.target_position(t.id)==std::pair{x,y}) objects.push_back(t.id);
            if (!objects.empty()) cell.objects=std::move(objects);return cell;
        };
        world.cells_in_range=[&](int x,int y,int radius){std::vector<CombatWorld::MagicCell> cells;
            for (int cy=y-radius;cy<=y+radius;++cy) for (int cx=x-radius;cx<=x+radius;++cx)
                if (auto cell=world.cell(cx,cy)) cells.push_back(std::move(*cell));return cells;};
        world.first_attackable=[&](int x,int y)->std::optional<int>{for (const auto& t:targets) if (world.target_position(t.id)==std::pair{x,y} && eligible(t)) return t.id;return {};};
        world.maximum_view_range=std::max(0,int(get_meta("reference_max_view_range",18))); // Zircon Config默认18，可由场景配置覆盖。
        std::optional<mir3::MonsterAiTarget> selected;
        const auto visible_target=[&](const mir3::MonsterAiTarget& t){
            const auto [x,y]=world.target_position(t.id);
            return eligible(t) && mir3::ZumaAi::within(body.cell.x,body.cell.y,x,y,world.maximum_view_range);
        };
        for (auto t:targets) if (actor.target_id==t.id && visible_target(t)) {
            const auto p=world.target_position(t.id);t.x=p.first;t.y=p.second;selected=t;break;
        }
        if (!selected) for (auto t:targets) if (visible_target(t)) {
            const auto p=world.target_position(t.id);t.x=p.first;t.y=p.second;
            if (!selected || mir3::MonsterPursuit::distance(body.cell.x,body.cell.y,t)<
                mir3::MonsterPursuit::distance(body.cell.x,body.cell.y,*selected)) selected=t;
        }
        actor.target_id=selected?std::optional<int>{selected->id}:std::nullopt;
        actor.controller->advance(ReferenceTime(monster_ai_clock),selected,body.death_frame<0 && !body.buried && (!actor.combat || !actor.combat->dead),world,
            [&]{return Controller::Position{body.cell.x,body.cell.y,body.facing};},[&](DelayedHit hit){
                // 到期按对象存活投递，不重做选敌/隐身条件；受击阶段另检查距离。
                const auto victim=reference_monsters.find(hit.target);
                const auto victim_body=scene_chickens.find(hit.target);
                Dictionary event;event["source"]=id;event["target"]=hit.target;event["power"]=hit.damage;
                event["element"]=int(hit.element);event["due"]=hit.due;event["resolved"]=false;
                if (actor.combat && victim!=reference_monsters.end() && victim->second.combat && victim_body!=scene_chickens.end()) {
                    auto& defender=*victim->second.combat;
                    const auto& pose=victim_body->second;
                    const auto outcome=CombatState::resolve(*actor.combat,defender,hit.damage,hit.element,ReferenceTime(monster_ai_clock),true,true,
                        mir3::ZumaAi::within(body.cell.x,body.cell.y,pose.cell.x,pose.cell.y,world.maximum_view_range),world.random);
                    event["resolved"]=true;event["outcome"]=int(outcome.outcome);event["damage"]=outcome.power;
                    event["critical"]=outcome.critical;event["killed"]=outcome.killed;
                    if (outcome.outcome==CombatState::Outcome::hit && !defender.dead) victim->second.target_id=id;
                    deliver_145(mir3::Monster145Message::health(hit.target,std::uint16_t(std::max(0,defender.hp)),
                        std::uint16_t(defender.mp),std::uint16_t(defender.maximum_hp)));
                    if (outcome.struck && !outcome.killed) deliver_145(mir3::Monster145Message::struck(hit.target,appearance_145(pose),
                        std::uint16_t(std::max(0,defender.hp)),std::uint16_t(defender.maximum_hp),std::uint16_t(std::clamp(outcome.power,0,65535))));
                    if (outcome.killed) deliver_145(mir3::Monster145Message::pose(mir3::protocol::SM_DEATH,hit.target,
                        pose.cell.x,pose.cell.y,pose.facing,appearance_145(pose)));
                    if (outcome.healed) deliver_145(mir3::Monster145Message::health(id,std::uint16_t(actor.combat->hp),
                        std::uint16_t(actor.combat->mp),std::uint16_t(actor.combat->maximum_hp)));
                }
                damage.append(event);
            });
        for (const auto& cast:casts) {
            Dictionary event;Array ids,locations;for (int target:cast->targets) ids.append(target);
            for (const auto& [x,y]:cast->locations) locations.append(Vector2i(x,y));
            event["source"]=id;event["magic"]=int(cast->magic);event["direction"]=cast->direction;
            event["targets"]=ids;event["locations"]=locations;magic.append(event);
        }
    }
    result["damage_requests"]=damage;result["magic_requests"]=magic;result["moved"]=movement;result["attacks"]=attacks;
    result["messages_145"]=packets;
    return result;
}

void Mir3HudView::advance_animal_monsters(std::span<const mir3::MonsterAiTarget> visible,Array& attacked,Array& moved) {
    for (auto it=monster_struck_deliveries.begin();it!=monster_struck_deliveries.end();) {
        if (it->due>monster_ai_clock) {++it;continue;}
        if (scene_chickens.contains(it->target)) receive_monster_action(it->target,it->hit);
        it=monster_struck_deliveries.erase(it);
    }
    for (auto& [id,animal]:animals) {
        const auto found=scene_chickens.find(id);
        if (found==scene_chickens.end() || found->second.shape!=animal.shape) continue;
        auto& monster=found->second;
        auto& state=animal.brain.state;
        state.dead=monster.death_frame>=0 || (animal.vitals && animal.vitals->hp==0);state.ghost=monster.buried;
        state.petrified=mir3::reference_monster_petrified(monster.status_flags);
        state.x=monster.cell.x;state.y=monster.cell.y;state.direction=monster.facing;
        if (monster.walk_frame>=0 || monster.attack_frame>=0 || monster.struck_frame>=0 ||
            monster.burrow_frame>=0 || !monster.actions.empty()) continue;
        const auto now=std::uint32_t(monster_ai_clock);
        const auto selected=std::find_if(visible.begin(),visible.end(),[&](const auto& t){return animal.target_id==t.id;});
        if (selected==visible.end()) {state.target.reset();animal.target_id.reset();}
        else state.target=mir3::MonsterRunState::Target{selected->x,selected->y,!selected->alive,false,true,selected->proper};
        if (state.target && animal.target_id) {
            const auto target_actor=animals.find(*animal.target_id);
            if (target_actor!=animals.end() && target_actor->second.vitals && target_actor->second.vitals->hp<=0)
                state.target->dead=true;
        }
        const auto random=[](unsigned bound){return unsigned(UtilityFunctions::randi())%bound;};
        const auto walk=[&](int facing) {
            state.walk_to(facing,[](int,int){return true;},[&](int x,int y,int direction) {
                const Vector2i destination(x,y);
                if (!terrain.walkable(x,y) || scene_occupied(destination) || destination==player_cell ||
                    std::any_of(visible.begin(),visible.end(),[&](const auto& t){return t.alive && t.x==x && t.y==y;})) return false;
                if (!receive_monster_action(id,SceneChicken::Turn{destination,direction,monster.shape,monster.light_attribute,false,monster.status_flags,true})) return false;
                state.x=x;state.y=y;state.direction=direction;moved.append(id);return true;
            });
        };
        mir3::MonsterRunState::Hooks hooks;
        hooks.break_holy=[&]{state.holy_seize=false;};
        hooks.think=[&]{return state.think(now,[&] {
            int count=0;
            for (const auto& [other,body]:scene_chickens) if (body.death_frame<0 && !body.buried && body.cell==monster.cell) ++count;
            for (const auto& t:visible) if (t.alive && t.x==state.x && t.y==state.y) ++count;
            return count;
        },random,walk);};
        hooks.attack=[&]{return state.attack_target(now,[&] {
            if (!animal.vitals || !animal.target_id) return;
            const auto victim=animals.find(*animal.target_id);
            if (victim==animals.end() || !victim->second.vitals || victim->second.vitals->hp<=0) return;
            auto& defender=victim->second;
            auto& health=*defender.vitals;
            mir3::MonsterMeleeAttack::Hooks combat;
            combat.mitigate=[&](int damage){return health.mitigate(damage,random);};
            combat.wear_weapon=[](int){}; // 怪物无人物装备。
            combat.damage=[&](int damage){
                // 在命中时读取目标最新状态，不能依赖目标在本轮遍历中的先后顺序。
                const auto target_body=scene_chickens.find(victim->first);
                health.damage_armor=target_body!=scene_chickens.end() &&
                    (target_body->second.status_flags&std::uint32_t(mir3::ReferenceMonsterStatus::damage_armor))!=0;
                health.struck_damage(damage,random);
                apply_scene_health(victim->first,mir3::protocol::SM_HEALTHSPELLCHANGED,health.hp,0,health.maximum);
            };
            combat.delayed_struck=[&](int damage,unsigned delay){
                monster_struck_deliveries.push_back({id,victim->first,monster_ai_clock+delay,
                    SceneChicken::Hit{defender.shape,0,{std::uint16_t(health.hp),std::uint16_t(health.maximum),std::uint16_t(damage)}}});
            };
            combat.immediate_struck=[&](int){
                if (health.hp>0) notify_reference_animal_struck(victim->first,id,monster.cell,true,int(defender.level));
            };
            mir3::MonsterMeleeAttack::resolve(animal.dc,animal.hit,0,{true,false,defender.hit,defender.speed},random,combat);
        },[&] {
            if (receive_monster_action(id,SceneChicken::Attack{monster.cell,state.direction,{},0,0})) attacked.append(id);
        },hooks.break_holy);};
        hooks.go_to_target=[&]{state.go_to_target(now,visible.empty()?nullptr:&visible.front(),[&](const auto& t) {
            animal.target_id=t.id;state.target=mir3::MonsterRunState::Target{t.x,t.y,!t.alive,false,true,t.proper};state.focused_at=now;
        },random,walk);};
        hooks.wander=[&]{state.wander(random,[&](int facing) {
            if (receive_monster_action(id,SceneChicken::Turn{monster.cell,facing,monster.shape,monster.light_attribute,false,monster.status_flags,false})) state.direction=facing;
        },walk);};
        // 离线实体由消息与场景生命周期管理；不在这里伪造服务端恢复、伤害或掉落。
        hooks.inherited=[]{ };
        animal.brain.run(now,hooks);
        if (!state.target) animal.target_id.reset();
    }
}

bool Mir3HudView::spawn_reference_bee_queen(int actor,Vector2i cell,int shape,int child_shape) {
    // 映射必须由调用方提供；124/125是参考race/数据库索引，绝不能直接用作外观。
    if (!bool(get_meta("offline_preview",false)) || actor<=0 || actor==local_actor_identity ||
        !terrain_ready || !terrain.walkable(cell.x,cell.y) || scene_chickens.contains(actor) ||
        scene_humans.contains(actor) || scene_npcs.contains(actor) || plant_actors.contains(actor) ||
        bee_queens.contains(actor) || !monster_frame(shape,0,0,0) || !monster_frame(child_shape,0,0,0)) return false;
    if (!apply_monster_entry(actor,SceneChicken::Entry{cell,0,shape,0,0,false,{}})) return false;
    bee_queens.emplace(actor,BeeQueenActor{mir3::BeeQueenMonster(monster_ai_clock,600,1200),shape,child_shape});
    return true;
}

void Mir3HudView::advance_bee_monsters(std::span<const mir3::MonsterAiTarget> visible,Array& spawned,
    Array& failures,Array& attacked,Array& moved) {
    const auto alive=[&](int id) {
        const auto found=scene_chickens.find(id);
        return found!=scene_chickens.end() && found->second.death_frame<0 && !found->second.buried;
    };
    // 先推进已有幼蜂，刚生成的实体从下一轮开始运行。
    for (auto& [id,bee]:bees) {
        if (!alive(id) || monster_ai_clock<=bee.next_turn) continue;
        auto& monster=scene_chickens.at(id);
        if (monster.walk_frame>=0 || monster.attack_frame>=0 || monster.struck_frame>=0 ||
            monster.burrow_frame>=0 || !monster.actions.empty()) continue;
        bee.next_turn=monster_ai_clock+600;
        const auto action=bee.pursuit.tick(monster_ai_clock,monster.cell.x,monster.cell.y,visible,
            [&](int x,int y,int facing) {
                const Vector2i destination(x,y);
                if (!terrain.walkable(x,y) || scene_occupied(destination) || destination==player_cell ||
                    std::any_of(visible.begin(),visible.end(),[&](const auto& t) {return t.alive && t.x==x && t.y==y;})) return false;
                return receive_monster_action(id,SceneChicken::Turn{destination,facing,monster.shape,monster.light_attribute,false,monster.status_flags,true});
            },[&](int facing,const auto&) {
                return receive_monster_action(id,SceneChicken::Attack{monster.cell,facing,{},0,0});
            },[](unsigned bound) {return unsigned(UtilityFunctions::randi())%bound;});
        if (action==mir3::MonsterPursuit::Action::attack) attacked.append(id);
        if (action==mir3::MonsterPursuit::Action::walk) moved.append(id);
    }
    for (auto& [id,queen]:bee_queens) {
        const auto found=scene_chickens.find(id);
        if (found==scene_chickens.end() || found->second.shape!=queen.shape) continue;
        auto& monster=found->second;
        queen.brain.dead=monster.death_frame>=0;
        queen.brain.ghost=monster.buried;
        queen.brain.petrified=mir3::reference_monster_petrified(monster.status_flags);
        queen.brain.operate(monster_ai_clock,[&](std::optional<int> target)->std::optional<int> {
            Dictionary request;request["owner"]=id;request["shape"]=queen.child_shape;request["cell"]=monster.cell;
            request["prototype"]=int(mir3::BeeSlavePrototype::bee);
            while (monster_ai_next_id<INT_MAX && (scene_chickens.contains(monster_ai_next_id) ||
                plant_actors.contains(monster_ai_next_id) || scene_humans.contains(monster_ai_next_id) ||
                scene_npcs.contains(monster_ai_next_id) || monster_ai_next_id==local_actor_identity)) ++monster_ai_next_id;
            if (monster_ai_next_id==INT_MAX) {
                request["reason"]="actor_id_exhausted";failures.append(request);return {};
            }
            const int child=monster_ai_next_id++;
            if (!apply_monster_entry(child,SceneChicken::Entry{monster.cell,0,queen.child_shape,0,0,false,{}})) {
                request["reason"]="entry_rejected";failures.append(request);return {};
            }
            mir3::BeeMonster bee;
            bee.pursuit.target_id=target;
            bee.pursuit.focused_at=bee.pursuit.searched_at=monster_ai_clock;
            bee.next_turn=monster_ai_clock+600;
            bees.emplace(child,std::move(bee));
            request["actor"]=child;request["target"]=target.value_or(0);spawned.append(request);
            return child;
        });
        // 离线适配按显示动作逐步推进；延迟投递不受身体攻击动画阻塞。
        if (monster.attack_frame>=0 || monster.struck_frame>=0 || monster.burrow_frame>=0 || !monster.actions.empty()) continue;
        if (queen.brain.run(monster_ai_clock,[&]()->std::optional<int> {
            for (const auto& t:visible)
                if (mir3::MonsterPursuit::eligible(t) && !t.hidden &&
                    mir3::MonsterPursuit::distance(monster.cell.x,monster.cell.y,t)<999) return t.id;
            return {};
        },alive)) {
            if (receive_monster_action(id,SceneChicken::Attack{monster.cell,monster.facing,{},0,0})) attacked.append(id);
        }
        // CMonsterObject::Run 的目标失效检查位于专用 Run 之后。
        if (queen.brain.target_id && std::none_of(visible.begin(),visible.end(),[&](const auto& t) {
            return t.id==*queen.brain.target_id && t.alive &&
                std::abs(t.x-monster.cell.x)<=15 && std::abs(t.y-monster.cell.y)<=15;
        })) queen.brain.target_id.reset();
    }
}

bool Mir3HudView::spawn_reference_monster(int actor,Vector2i cell,int shape,int facing) {
    if (!bool(get_meta("offline_preview",false)) || !terrain_ready || actor<=0 || actor==local_actor_identity ||
        facing<0 || facing>=8 || !terrain.walkable(cell.x,cell.y) || scene_chickens.contains(actor) ||
        scene_npcs.contains(actor) || scene_humans.contains(actor) || plant_actors.contains(actor) ||
        shape!=int(mir3::MonsterAppearance::carnivorous_plant)) return false;
    // 隐藏出生只建立世界实体，首次钻出才向客户端显示层提交对象。
    PlantActor plant{{},SceneChicken::Entry{cell,facing,shape,0,0,false,{},true}};
    plant.brain.walked_at=monster_ai_clock;
    plant.brain.next_hit=monster_ai_clock+plant.brain.attack_interval_ms;
    plant.brain.facing=facing;
    plant_actors.emplace(actor,std::move(plant));
    return true;
}

Dictionary Mir3HudView::advance_monster_ai(int elapsed_ms,Array targets) {
    Dictionary result;
    // 显式离线世界适配器；联网场景仍由服务器驱动，不能擅自发送唤醒或召唤。
    if (!bool(get_meta("offline_preview",false)) || elapsed_ms<0 || elapsed_ms>10000) {
        result["error"]="offline preview and elapsed_ms in [0,10000] required";
        return result;
    }
    std::vector<mir3::ZumaAi::Target> visible;
    for (int index=0;index<targets.size();++index) {
        if (targets[index].get_type()!=Variant::DICTIONARY) {
            result["error"]="target dictionary required";return result;
        }
        const Dictionary target=targets[index];
        if (!target.has("cell") || target["cell"].get_type()!=Variant::VECTOR2I) {
            result["error"]="target cell required";return result;
        }
        const Vector2i cell=target["cell"];
        visible.push_back({cell.x,cell.y,bool(target.get("alive",true)),
            bool(target.get("proper",true)),bool(target.get("inspector",false)),
            int(target.get("id",index+1)),bool(target.get("hidden",false))});
    }
    const auto ai_kind=[](int shape)->std::optional<mir3::ZumaKind> {
        using mir3::MonsterAppearance;
        using mir3::ZumaKind;
        switch (MonsterAppearance(shape)) {
        case MonsterAppearance::zuma_fanatic:
        case MonsterAppearance::zuma_guardian: return ZumaKind::statue;
        case MonsterAppearance::zuma_king: return ZumaKind::king;
        default:
            if (mir3::reference_ranged_burst(MonsterAppearance(shape))) return ZumaKind::ranged;
            return {};
        }
    };
    std::erase_if(monster_ai.actors,[&](const auto& entry) {
        const auto found=scene_chickens.find(entry.first);
        return found==scene_chickens.end() || ai_kind(found->second.shape)!=entry.second.kind ||
            mir3::reference_ranged_burst(mir3::MonsterAppearance(found->second.shape))!=entry.second.pursuit.burst_limit;
    });
    for (const auto& [id,monster]:scene_chickens) {
        if (bee_queens.contains(id) || bees.contains(id) || animals.contains(id)) continue;
        const auto kind=ai_kind(monster.shape);
        if (!kind) continue;
        auto [found,inserted]=monster_ai.actors.try_emplace(id);
        auto& brain=found->second;
        if (inserted) {
            brain.id=id;
            brain.kind=*kind;
            brain.pursuit.facing=monster.facing;
            brain.pursuit.burst_limit=mir3::reference_ranged_burst(mir3::MonsterAppearance(monster.shape));
            brain.stone=mir3::MonsterBehavior::for_shape(monster.shape).dormant(monster.status_flags);
            brain.next_turn=monster_ai_clock+brain.walk_interval_ms;
        }
        brain.x=monster.cell.x;brain.y=monster.cell.y;
        brain.dead=monster.death_frame>=0;
        brain.ghost=monster.buried;
        brain.petrified=mir3::reference_monster_petrified(monster.status_flags);
        if (const auto health=scene_health.find(id);health!=scene_health.end() && health->second.maximum>0) {
            brain.hp=health->second.current;brain.max_hp=health->second.maximum;
        }
    }
    monster_ai_clock+=elapsed_ms;
    Array spawned,spawn_failures;
    const auto events=monster_ai.tick(monster_ai_clock,[&](int) {return visible;},
        [&](const mir3::ZumaAi::Actor& owner,mir3::ZumaSlavePrototype prototype)->std::optional<int> {
            Dictionary request;request["owner"]=owner.id;request["prototype"]=int(prototype);
            const auto binding=mir3::reference_zuma_prototype(prototype);
            if (!binding) {
                request["reason"]="unresolved_reference_prototype";spawn_failures.append(request);return {};
            }
            const auto& summoner=scene_chickens.at(owner.id);
            // 服务器逻辑方向独立保留；教主客户端苏醒会强制显示方向0。
            const auto direction=mir3::map_directions[owner.pursuit.facing%8];
            const auto cell=summoner.cell+Vector2i(direction[0],direction[1]);
            // CallSlave每次取同一个前方格，AddCreature(...,FALSE)不搜索替代位置。
            // 允许同格多实体；不能为展示便利擅自改成环形散布。
            while (monster_ai_next_id<INT_MAX && (scene_chickens.contains(monster_ai_next_id) ||
                plant_actors.contains(monster_ai_next_id) ||
                scene_npcs.contains(monster_ai_next_id) || scene_humans.contains(monster_ai_next_id) ||
                monster_ai_next_id==local_actor_identity)) ++monster_ai_next_id;
            if (monster_ai_next_id==INT_MAX) {
                request["reason"]="actor_id_exhausted";spawn_failures.append(request);return {};
            }
            const int id=monster_ai_next_id++;
            const int shape=int(binding->appearance);
            const auto behavior=mir3::MonsterBehavior::for_shape(shape);
            const auto status=behavior.birth==mir3::MonsterBehavior::Birth::dormant_statue?
                mir3::MonsterBehavior::dormant_flag:0u;
            mir3::SceneName name;
            name.update(mir3::protocol::SM_USERNAME,255,mir3::utf8_to_ansi(binding->name));
            if (!apply_monster_entry(id,SceneChicken::Entry{cell,int(UtilityFunctions::randi()%8),shape,0,
                    status,false,name,behavior.dormant(status),false})) {
                request["reason"]="entry_rejected";request["shape"]=shape;request["cell"]=cell;
                spawn_failures.append(request);return {};
            }
            request["actor"]=id;request["shape"]=shape;request["cell"]=cell;
            spawned.append(request);
            return id;
        },[&](int id) { const auto found=scene_chickens.find(id);return found!=scene_chickens.end() && found->second.death_frame<0; },
        [](unsigned bound) {return unsigned(UtilityFunctions::randi())%bound;});
    Array woke,ready,rejected;
    for (int id:events.woke) {
        const auto& monster=scene_chickens.at(id);
        if (receive_monster_action(id,SceneChicken::Burrow{monster.cell,monster.facing,monster.shape,
                monster.light_attribute,monster.status_flags&~mir3::MonsterBehavior::dormant_flag})) woke.append(id);
        else { monster_ai.actors.at(id).stone=true;rejected.append(id); }
    }
    Array moved,attacked,blocked;
    for (int id:events.ready) {
        ready.append(id);
        auto& monster=scene_chickens.at(id);
        auto& brain=monster_ai.actors.at(id);
        // 离线适配器按完成动作逐步推进；避免测试时快进时钟堆积无限动作。
        if (monster.walk_frame>=0 || monster.struck_frame>=0 || monster.burrow_frame>=0 ||
            monster.attack_frame>=0 || !monster.actions.empty()) continue;
        const auto action=brain.pursuit.tick(monster_ai_clock,monster.cell.x,monster.cell.y,visible,
            [&](int x,int y,int facing) {
                const Vector2i destination(x,y);
                if (!terrain.walkable(x,y) || scene_occupied(destination) || destination==player_cell ||
                    std::any_of(visible.begin(),visible.end(),[&](const auto& target) {return target.alive && target.x==x && target.y==y;})) return false;
                return receive_monster_action(id,SceneChicken::Turn{destination,facing,monster.shape,
                    monster.light_attribute,false,monster.status_flags,true});
            },[&](int facing,const mir3::MonsterAiTarget& target) {
                if (brain.kind==mir3::ZumaKind::ranged)
                    return receive_monster_action(id,SceneChicken::Attack{monster.cell,facing,
                        Vector2i(target.x,target.y),std::uint32_t(target.id),0});
                return receive_monster_action(id,SceneChicken::Attack{monster.cell,facing,std::nullopt,0,0});
            },[](unsigned bound) {return unsigned(UtilityFunctions::randi())%bound;});
        if (action==mir3::MonsterPursuit::Action::walk) moved.append(id);
        else if (action==mir3::MonsterPursuit::Action::attack) attacked.append(id);
        else if (action==mir3::MonsterPursuit::Action::blocked) blocked.append(id);
    }
    Array emerged,retreated,hidden_plants;
    std::erase_if(plant_actors,[&](const auto& entry) {
        const auto display=scene_chickens.find(entry.first);
        return display!=scene_chickens.end() && display->second.shape!=entry.second.entry.shape;
    });
    for (auto& [id,plant]:plant_actors) {
        if (const auto display=scene_chickens.find(id);display!=scene_chickens.end()) {
            const auto& monster=display->second;
            plant.dead=monster.death_frame>=0;
            plant.entry.status=monster.status_flags;
            plant.entry.light=monster.light_attribute;
            if (const auto name=scene_names.find(id);name!=scene_names.end()) plant.entry.name=name->second;
            if (const auto health=scene_health.find(id);health!=scene_health.end())
                plant.health=std::array{health->second.current,health->second.mana,health->second.maximum};
            // 等待已有动画结束；钻回后显示被删，世界实体仍保留。
            if (monster.burrow_frame>=0 || monster.attack_frame>=0 || monster.struck_frame>=0 ||
                !monster.actions.empty()) continue;
        }
        const auto cell=plant.entry.cell;
        const auto action=plant.brain.tick(monster_ai_clock,cell.x,cell.y,plant.dead,false,
            visible,plant.brain.target(visible));
        if (action==mir3::PlantAi::Action::emerge) {
            plant.entry.facing=plant.brain.facing;
            if (apply_monster_entry(id,plant.entry) && receive_monster_action(id,
                SceneChicken::Burrow{cell,plant.brain.facing,plant.entry.shape,plant.entry.light,plant.entry.status}))
            {
                emerged.append(id);
                if (plant.health) scene_health[id].apply(mir3::protocol::SM_HEALTHSPELLCHANGED,
                    (*plant.health)[0],(*plant.health)[1],(*plant.health)[2]);
            }
            else { plant.brain.hidden=true;rejected.append(id);remove_scene_actor_display(id); }
        } else if (action==mir3::PlantAi::Action::retreat) {
            if (receive_monster_action(id,SceneChicken::ReverseBurrow{cell})) retreated.append(id);
            else { plant.brain.hidden=false;rejected.append(id); }
        } else if (action==mir3::PlantAi::Action::combat) {
            if (plant.brain.fight(monster_ai_clock,cell.x,cell.y,visible,[&](int facing,const auto&) {
                return receive_monster_action(id,SceneChicken::Attack{cell,facing,std::nullopt,0,0});
            })) attacked.append(id);
        }
        if (plant.brain.hidden) hidden_plants.append(id);
    }
    advance_bee_monsters(visible,spawned,spawn_failures,attacked,moved);
    advance_animal_monsters(visible,attacked,moved);
    result["reference"]=advance_reference_monsters(visible);
    result["emerged"]=emerged;result["retreated"]=retreated;result["hidden_plants"]=hidden_plants;
    result["woke"]=woke;result["ready"]=ready;result["rejected"]=rejected;
    result["spawned"]=spawned;result["spawn_failures"]=spawn_failures;
    result["moved"]=moved;result["attacked"]=attacked;result["blocked"]=blocked;
    return result;
}

Dictionary Mir3HudView::monster_behavior_profile(int shape) {
    const auto behavior=mir3::MonsterBehavior::for_shape(shape);
    Dictionary profile;
    profile["birth_opcode"]=int(mir3::protocol::SM_TURN);
    profile["birth_status"]=behavior.birth==mir3::MonsterBehavior::Birth::dormant_statue?int(mir3::MonsterBehavior::dormant_flag):0;
    profile["wake_opcode"]=behavior.wake_frames?int(mir3::protocol::SM_DIGUP):0;
    profile["wake_frames"]=behavior.wake_frames;
    profile["wake_interval_ms"]=behavior.wake_interval_ms;
    profile["wake_facing"]=behavior.wake_facing;
    profile["attack_opcode"]=mir3::MonsterBehavior::attack_message(shape);
    // 展示能力只描述现有资源，不据此推断服务端是否允许移动。
    // 原版40ECB0允许空帧，不能用这张表拒绝网络动作。
    Array walk_directions;
    const auto idle=monster_idle_motion(shape);
    for (int direction=0;direction<8;++direction) {
        bool complete=true;
        for (int frame=0;frame<6 && complete;++frame)
            complete=monster_frame(shape,80,direction,frame)!=nullptr;
        for (int frame=0;frame<idle.frames && complete;++frame)
            complete=monster_frame(shape,0,direction,frame)!=nullptr;
        if (complete) walk_directions.append(direction);
    }
    profile["visible_walk_directions"]=walk_directions;
    return profile;
}

bool Mir3HudView::apply_monster_burrow(int actor,SceneChicken::Burrow burrow) {
    auto& monster=scene_chickens.at(actor);
    if (!terrain.contains(burrow.cell.x,burrow.cell.y)) return false;
    if (monster.death_frame>=0 || monster.buried) return true;
    const int shape=burrow.shape>=0?burrow.shape:monster.shape;
    const int facing=shape==31 || shape==85 || (shape>=530 && shape<=533)?0:burrow.facing;
    if (!refresh_monster_shape(monster,shape)) return false;
    monster.cell=burrow.cell;
    monster.facing=facing;
    if (burrow.status) { monster.status_flags=*burrow.status;update_actor_bar_status(actor,(*burrow.status&2)!=0); }
    monster.light_attribute=burrow.light;
    monster.burrow_frame=0;
    monster.hidden_pose=false;
    monster.burrow_ms=0;
    monster.hide_after_burrow=false;
    monster.reverse_burrow=shape==3 || shape==31;
    monster.burrow_catch_up=false;
    if (shape==56 || shape==89) show_map_impact(shape==56?17:30,burrow.cell);
    if (shape==4 || shape==49) {
        const int frame=(shape==4?200:240)+facing;
        ground_events.emplace(harvest_effect_id++,GroundEvent{burrow.cell,frame,-1200,false,58800});
    }
    update_scene_target(inventory_cursor);
    target_focus.remove(actor);
    queue_redraw();
    return true;
}

bool Mir3HudView::apply_monster_action(int actor,const SceneChicken::Action& action) {
    if (const auto* special=std::get_if<SceneChicken::SpecialAttack>(&action))
        return apply_monster_special(actor,*special);
    if (const auto* relay=std::get_if<SceneChicken::RelayedHit>(&action)) {
        if (!apply_monster_hit(actor,relay->hit)) return false;
        auto& actions=scene_chickens.at(actor).actions;
        if (actions.size()<64) actions.emplace_back(relay->hit);
        return true;
    }
    if (const auto* transfer=std::get_if<SceneChicken::Transfer>(&action)) {
        if (transfer->entry) {
            if (!apply_monster_entry(actor,*transfer->entry)) return false;
            const int effect=transfer->opcode==mir3::protocol::SM_801?321:
                transfer->opcode==mir3::protocol::SM_808?323:0;
            if (effect) show_map_impact(effect,scene_chickens.at(actor).cell);
        } else {
            const auto cell=scene_chickens.at(actor).cell;
            play_actor_sound(actor,110,cell);
            show_map_impact(transfer->opcode==mir3::protocol::SM_800?322:320,cell);
        }
        return true;
    }
    if (const auto* turn=std::get_if<SceneChicken::Turn>(&action)) return move_scene_chicken(actor,*turn);
    if (const auto* hit=std::get_if<SceneChicken::Hit>(&action)) return apply_monster_hit(actor,*hit);
    if (const auto* death=std::get_if<SceneChicken::Death>(&action)) return apply_monster_death(actor,*death);
    if (const auto* burrow=std::get_if<SceneChicken::Burrow>(&action)) return apply_monster_burrow(actor,*burrow);
    if (const auto* reverse=std::get_if<SceneChicken::ReverseBurrow>(&action)) {
        auto& monster=scene_chickens.at(actor);
        if (monster.death_frame>=0) return true;
        monster.cell=reverse->cell;
        if (monster.shape==31 || (monster.shape>=530 && monster.shape<=533)) monster.facing=0;
        monster.buried=false;
        monster.hide_after_burrow=true;
        monster.reverse_burrow=monster.shape!=3 && monster.shape!=31;
        monster.burrow_frame=0;
        monster.burrow_ms=0;
        monster.burrow_catch_up=false;
        queue_redraw();
        return true;
    }
    if (const auto* rise=std::get_if<SceneChicken::Rise>(&action)) {
        auto& monster=scene_chickens.at(actor);
        const int shape=rise->shape>=0?rise->shape:monster.shape;
        if (!refresh_monster_shape(monster,shape)) return false;
        monster.cell=rise->cell;
        monster.facing=rise->facing;
        if (rise->status) { monster.status_flags=*rise->status;update_actor_bar_status(actor,(*rise->status&2)!=0); }
        monster.light_attribute=rise->light;
        monster.death_frame=-1;
        monster.skeleton=false;
        monster.buried=false;
        monster.hide_after_burrow=false;
        monster.reverse_burrow=false;
        monster.burrow_catch_up=false;
        monster.hidden_pose=false;
        monster.burrow_frame=0;
        monster.burrow_ms=0;
        monster.idle_frame=0;
        monster.idle_ms=0;
        queue_redraw();
        return true;
    }
    if (const auto* entry=std::get_if<SceneChicken::Entry>(&action)) return apply_monster_entry(actor,*entry);
    if (const auto* status=std::get_if<SceneChicken::BarStatus>(&action)) {
        scene_chickens.at(actor).status_flags=status->flags;
        update_actor_bar_status(actor,(status->flags&2)!=0);
        return true;
    }
    if (const auto* attack=std::get_if<SceneChicken::Attack>(&action)) {
        auto& monster=scene_chickens.at(actor);
        if (monster.death_frame>=0 || monster.buried) return true;
        const int action=attack->action_override?attack->action_override:
            attack->target?2:monster.shape==141?6:monster.shape==151?5:2;
        const auto motion=monster_attack_motion(monster.shape,action);
        monster.attack_action=action;
        if (!attack->action_override) {
            monster.cell=attack->cell;
            monster.facing=monster_resource_direction(monster.shape,motion.base,attack->facing);
        }
        if (attack->target && !play_monster_ranged_attack(actor,monster,*attack)) {
            queue_redraw();
            return true;
        }
        monster.attack_frame=0;
        monster.attack_ms=0;
        monster.attack_interval=motion.interval;
        if (!attack->target && !attack->action_override) play_monster_attack_effect(actor,monster);
        if (action==6 && monster.shape==10) show_map_impact(501,monster.cell);
        sync_monster_action_overlay(actor,monster,true);
        monster.attack_catch_up=false;
        queue_redraw();
        return true;
    }
    const auto& values=std::get<SceneChicken::Health>(action).values;
    auto& health=scene_health[actor];
    if (health.current) {
        health.damage.enabled=damage_numbers;
        health.damage.changed(health.current,values[0]);
    }
    health.current=values[0];health.mana=values[1];health.maximum=values[2];health.empty_bar=false;
    queue_redraw();
    return true;
}

bool Mir3HudView::apply_monster_entry(int actor,const SceneChicken::Entry& entry) {
    const auto existing=scene_chickens.find(actor);
    if (existing==scene_chickens.end()) {
        if (!place_scene_chicken(actor,entry.cell,entry.facing,entry.shape,entry.light)) return false;
    } else {
        auto& monster=existing->second;
        // 4085B4 and 407CEF apply the server position independently of walkability.
        if (!terrain.contains(entry.cell.x,entry.cell.y) ||
            !refresh_monster_shape(monster,entry.shape)) return false;
        monster.cell=entry.cell;
        monster.facing=entry.facing;
        monster.light_attribute=entry.light;
        monster.buried=false;
        monster.idle_frame=0;
        monster.idle_ms=0;
        if (entry.revive) {
            // 4085A0: SM_REVIVE clears the corpse flag before selecting standing.
            monster.death_frame=-1;
            monster.death_ms=0;
            monster.skeleton=false;
        } else if (monster.death_frame>=0) {
            // 4085A0 and 407CC0 retain the corpse flag on ordinary refresh.
            // 40DA8A then selects the last frame of action 4.
            monster.skeleton=false;
            monster.death_frame=monster_death_motion(monster.shape).frames-1;
            monster.death_ms=0;
        }
    }
    scene_chickens.at(actor).hidden_pose=entry.hidden_pose;
    if (entry.revive) {
        auto& monster=scene_chickens.at(actor);
        monster.appearance_duration_ms=SceneChicken::revive_appearance_ms;
        monster.appearance_elapsed_ms=0;
    }
    scene_chickens.at(actor).status_flags=entry.status;
    update_actor_bar_status(actor,(entry.status&2)!=0);
    if (entry.name) {
        auto& name=scene_names[actor];
        const auto color=name.color;
        name=*entry.name;
        if (entry.preserve_name_color) name.color=color;
    }
    if (entry.appearance_sound) play_actor_sound(actor,109,entry.cell);
    return true;
}

bool Mir3HudView::receive_monster_action(int actor,SceneChicken::Action action,bool from_wire) {
    auto& monster=scene_chickens.at(actor);
    // 4082F0 and 409500 derive the animation origin with 453450.
    if (const auto* turn=std::get_if<SceneChicken::Turn>(&action);
        turn && !terrain.contains(turn->cell.x,turn->cell.y)) return false;
    // 42EDC7 -> 40D7F0: fold prior 31s before appending an outside message.
    if (from_wire && monster.actions.size()>1) {
        std::erase_if(monster.actions,[&](const SceneChicken::Action& queued) {
            const auto* hit=std::get_if<SceneChicken::Hit>(&queued);
            if (!hit) return false;
            mir3::SceneHumanMessage health{mir3::protocol::SM_STRUCK};
            health.health=hit->health;
            apply_actor_health_record(actor,health);
            return true;
        });
    }
    if (monster.walk_frame>=0 || monster.struck_frame>=0 || monster.burrow_frame>=0 || monster.attack_frame>=0 ||
        (monster.death_frame>=0 && monster.death_frame<monster_death_motion(monster.shape).frames-1 && !monster.skeleton) ||
        !monster.actions.empty()) {
        if (monster.actions.size()>=64) return false;
        monster.actions.push_back(action);
        return true;
    }
    return apply_monster_action(actor,action);
}

bool Mir3HudView::apply_monster_special(int actor,const SceneChicken::SpecialAttack& special) {
    const auto& encoded=special.encoded;
    const auto& p=special.parameters;
        const auto body=mir3::decode_six_bit(encoded.ptr()+16,encoded.size()-16);
        const int facing=p[2]&255;
        if (!body || body->size()!=16 || facing>=8) return false;
        const auto dword=[&](int offset) {
            std::uint32_t value=0;
            for (int byte=0;byte<4;++byte)
                value|=std::uint32_t(std::uint8_t((*body)[offset+byte]))<<(8*byte);
            return value;
        };
        auto& monster=scene_chickens.at(actor);
        const auto branch=dword(12);
        int effect=0, delay=0, action=0, overlay=0;
        bool at_target=false, travels=false, eight_directions=false, use_facing=false, ground_wave=false;
        switch (branch) {
        case 1: effect=515; action=2; use_facing=true; break;
        case 2: effect=538; at_target=true;
            if (monster.shape==13) { delay=500; action=6; }
            break;
        case 3: effect=539; break;
        case 4: effect=543; at_target=true; break;
        case 5: effect=23; at_target=true; break;
        case 6: effect=9; ground_wave=true; break;
        case 7: effect=576; delay=500; travels=true; overlay=550; break;
        case 8: effect=538; delay=500; at_target=true; break;
        case 9: effect=555; at_target=true; break;
        case 10: case 11: effect=590; delay=400; travels=true; eight_directions=true; action=5; break;
        case 12: effect=581; travels=true; action=5; break;
        case 13: overlay=579; action=6; break;
        case 14: effect=524; break;
        case 15: effect=592; travels=true; break;
        case 16: effect=584; use_facing=true; break;
        case 17: effect=576; delay=500; travels=true; action=7; break;
        case 18: effect=588; break;
        case 19: overlay=587; action=5; break;
        case 20: effect=597; delay=800; travels=true; eight_directions=true; break;
        case 21: effect=603; delay=400; at_target=true; use_facing=true; break;
        case 22: effect=606; action=6; use_facing=true; break;
        case 23: effect=2; delay=700; at_target=true; break;
        case 24: effect=29; at_target=true; break;
        case 25: effect=53; ground_wave=true; break;
        case 26: effect=120; delay=700; break;
        default: return false;
        }
        // 409DB0 branches 4 and 6 update the facing without applying the packet cell.
        const Vector2i source=branch==4 || branch==6?monster.cell:Vector2i(p[0],p[1]);
        const Vector2i target(static_cast<std::int32_t>(dword(0)),static_cast<std::int32_t>(dword(4)));
        using Section=mir3::EffectCatalog::Section;
        if ((at_target || travels || ground_wave) && (target.x<0 || target.y<0 || target.x>65535 || target.y>65535)) return false;
        const bool starts_action=action && monster.death_frame<0;
        // The position update moves the base cell without changing the remaining step offset.
        if (monster.walk_frame>=0 && !starts_action)
            monster.walk_origin+=source-monster.cell;
        monster.cell=source;
        monster.facing=facing;
        if (starts_action) {
            monster.walk_frame=-1;
            monster.struck_frame=-1;
            monster.burrow_frame=-1;
            monster.buried=false;
            monster.attack_action=action;
            monster.attack_frame=0;
            monster.attack_ms=0;
            monster.attack_interval=monster_attack_motion(monster.shape,action).interval;
            monster.attack_catch_up=false;
            sync_monster_action_overlay(actor,monster,true);
        }
        if (overlay) {
            const auto id=harvest_effect_id;
            show_scene_effect(Section::spell,overlay,source,10*facing,0,starts_action?actor:0);
            if (branch==7) {
                if (const auto event=map_events.find(id);event!=map_events.end())
                    event->second.follow_actor=actor;
            }
            play_actor_sound(actor,10*(overlay+1000),source);
        }
        if (ground_wave) {
            show_ground_magic(static_cast<mir3::MagicEffectId>(effect),source,target);
        } else if (travels) {
            const auto start=Vector2(source*Vector2i(48,32));
            const auto destination=Vector2(target*Vector2i(48,32));
            const int frame_offset=10*monster_projectile_direction(destination-start,eight_directions);
            show_scene_effect(Section::magic,effect,source,frame_offset,delay,0,
                MonsterProjectile{start,destination,dword(8),effect});
        } else if (effect) {
            const auto id=harvest_effect_id;
            show_scene_effect(Section::magic,effect,at_target?target:source,use_facing?10*facing:0,delay);
            if (effect==2 || effect==120 || effect==543 || effect==555) {
                if (const auto event=map_events.find(id);event!=map_events.end())
                    event->second.follow_actor=dword(8);
            }
        }
        queue_redraw();
        return true;
}

void Mir3HudView::sync_monster_action_overlay(int actor,SceneChicken& monster,bool restart) {
    auto pose=MonsterEffectPose::idle;
    int action=0;
    if (monster.skeleton) { pose=MonsterEffectPose::skeleton; action=10; }
    else if (monster.death_frame>=0) { pose=MonsterEffectPose::death; action=4; }
    else if (monster.burrow_frame>=0) { pose=MonsterEffectPose::burrow; action=8; }
    else if (monster.struck_frame>=0) { pose=MonsterEffectPose::struck; action=3; }
    else if (monster.walk_frame>=0) { pose=MonsterEffectPose::walk; action=1; }
    else if (monster.attack_frame>=0) { pose=MonsterEffectPose::attack; action=monster.attack_action; }

    const auto active=map_events.find(monster.action_overlay);
    const bool has_overlay=!monster.buried && (monster.shape==141 || (monster.shape==184 && action!=4));
    const int effect=(monster.shape==141?559:598)+action;
    const int offset=monster.facing*(action==4?20:10);
    if (active!=map_events.end()) {
        if (!restart && has_overlay && active->second.resource->id==effect &&
            active->second.frame_offset==offset) return;
        active->second.copy->hide();
        active->second.copy->queue_free();
        map_events.erase(active);
    }
    monster.action_overlay=0;
    if (!has_overlay) return;
    const auto id=harvest_effect_id;
    show_scene_effect(mir3::EffectCatalog::Section::spell,effect,monster.cell,offset,0,actor,std::nullopt,pose);
    if (map_events.contains(id)) {
        monster.action_overlay=id;
        play_actor_sound(actor,10*(effect+1000),monster.cell);
    }
}

void Mir3HudView::play_monster_attack_effect(int actor,const SceneChicken& monster) {
    // 40CBF0 creates independent MAGIC effects or 40DB10 actor overlays.
    using Section=mir3::EffectCatalog::Section;
    switch (monster.shape) {
    case 5: show_scene_effect(Section::magic,502,monster.cell); break;
    case 9: show_scene_effect(Section::magic,501,monster.cell); break;
    case 19: show_scene_effect(Section::magic,511,monster.cell); break;
    case 37: show_scene_effect(Section::magic,535,monster.cell,10*monster.facing); break;
    case 43: show_scene_effect(Section::magic,521,monster.cell,10*monster.facing); break;
    case 45: show_scene_effect(Section::magic,513,monster.cell); break;
    case 47: show_scene_effect(Section::magic,536,monster.cell,10*monster.facing); break;
    case 67: show_scene_effect(Section::magic,522,monster.cell); break;
    case 75: show_scene_effect(Section::magic,529,monster.cell,10*monster.facing); break;
    case 85: show_scene_effect(Section::magic,516,monster.cell); break;
    case 90: show_scene_effect(Section::magic,514,monster.cell); break;
    case 94: show_scene_effect(Section::magic,524,monster.cell); break;
    case 97: show_scene_effect(Section::magic,543,monster.cell); break;
    case 132: show_scene_effect(Section::magic,551,monster.cell,10*monster.facing,600); break;
    case 151: show_scene_effect(Section::magic,575,monster.cell,10*monster.facing); break;
    default: {
        constexpr std::pair<int,int> overlays[]={{98,530},{113,537},{120,532},{130,546},{131,548},
            {139,554},{150,570},{152,572},{164,591},{186,608}};
        for (const auto& [shape,effect]:overlays) {
            if (monster.shape!=shape) continue;
            show_scene_effect(Section::spell,effect,monster.cell,10*monster.facing,0,actor);
            play_actor_sound(actor,10*(effect+1000),monster.cell);
            break;
        }
        break;
    }
    }
}

void Mir3HudView::play_monster_death_effect(int actor,const SceneChicken& monster) {
    // 408C10 chooses a stationary effect or attaches SPELL frames to the death pose.
    using Section=mir3::EffectCatalog::Section;
    int effect=0, delay=0;
    switch (monster.shape) {
    case 4: effect=531; break;
    case 5: case 6: case 11: case 19: effect=526; break;
    case 32: case 33: case 34: case 35: case 36: effect=534; break;
    case 40: effect=527; break;
    case 48: effect=528; break;
    case 99: effect=525; break;
    case 101: effect=541; break;
    case 105: effect=540; break;
    case 120: effect=533; break;
    case 164: effect=589; break;
    case 182: case 185: effect=605; break;
    case 184: effect=602; delay=800; break;
    case 186: effect=610; delay=800; break;
    default: break;
    }
    // 408C10 passes the delay to 4495D0; 44A500 suppresses drawing until it elapses.
    // 449AF5 and 449B77 shift both frame bounds for 602 and 605.
    if (effect) show_scene_effect(Section::magic,effect,monster.cell,
        (effect==602 || effect==605)?10*monster.facing:0,delay);
    constexpr std::pair<int,int> overlays[]={{130,547},{131,549},{138,553},{140,557},
        {142,556},{143,556},{144,556},{159,582},{160,583},{161,586}};
    for (const auto& [shape,overlay]:overlays) {
        if (monster.shape!=shape) continue;
        show_scene_effect(Section::spell,overlay,monster.cell,shape==140?10*monster.facing:0,0,actor,
            std::nullopt,MonsterEffectPose::death);
        play_actor_sound(actor,10*(overlay+1000),monster.cell);
        break;
    }
}

bool Mir3HudView::play_monster_ranged_attack(int actor,const SceneChicken& monster,const SceneChicken::Attack& attack) {
    // 40B0A0 passes the decoded target and a shape-specific launch delay to 4495D0.
    using Section=mir3::EffectCatalog::Section;
    int effect=0, delay=0;
    auto target=*attack.target;
    auto target_actor=attack.target_actor;
    // 4495D0 resolves the object once, before the launch delay starts.
    if (target_actor!=std::uint32_t(local_actor_identity) && !scene_chickens.contains(target_actor) &&
        !scene_humans.contains(target_actor) && !scene_npcs.contains(target_actor)) target_actor=0;
    const auto start=Vector2(monster.cell*Vector2i(48,32));
    switch (monster.shape) {
    case 7: effect=505; delay=500; break;
    case 19: effect=511; delay=500; break;
    case 33: effect=520; delay=365; break;
    case 46: effect=517; delay=365; break;
    case 82: case 144: effect=519; delay=365; break;
    case 86: effect=518; delay=365; break;
    case 95: effect=545; delay=700; break;
    case 96: effect=544; delay=500; break;
    case 124: case 125: {
        // 40B0A0 sends the caster's ID for 569, then the packet target for 2.
        const auto id=harvest_effect_id;
        show_scene_effect(Section::magic,569,monster.cell);
        if (const auto event=map_events.find(id);event!=map_events.end())
            event->second.follow_actor=actor;
        effect=2; delay=700;
        break;
    }
    case 145: case 146: effect=573; break;
    case 156: effect=578; delay=700; break;
    case 167: {
        const Vector2i offsets[]={{0,-20},{20,-30},{20,-5},{20,10},{0,20},{-20,10},{-20,-5},{-20,-30}};
        target=monster.cell+offsets[monster.facing];
        target_actor=0;
        effect=593; delay=700;
        break;
    }
    case 169: {
        const Vector2i offsets[]={{0,-20},{20,-30},{30,-8},{20,12},{0,20},{-20,12},{-30,-8},{-20,-30}};
        const Vector2i spread[]={{0,0},{2,0},{0,2},{2,2},{3,2},{-2,0},{0,-2},{-2,-2},{-3,-2}};
        for (int index=0;index<9;++index) {
            const auto endpoint=monster.cell+offsets[monster.facing]+spread[index];
            const auto destination=Vector2(endpoint*Vector2i(48,32));
            show_scene_effect(Section::magic,595,monster.cell,
                10*monster_projectile_direction(Vector2(endpoint-monster.cell),true),index*70,0,
                MonsterProjectile{start,destination,0,595});
        }
        return true;
    }
    case 183: effect=23; delay=700; break;
    default: return false;
    }
    if (effect==2 || effect==23 || effect==505 || effect==511) {
        const auto id=harvest_effect_id;
        show_scene_effect(Section::magic,effect,target,0,delay);
        if (const auto event=map_events.find(id);event!=map_events.end()) {
            if (effect!=23) event->second.follow_actor=target_actor;
            // 449480 enables lighting. Neither 2 nor 23 clears it in 4495D0.
            event->second.emits_light=event->second.resource->outer_radius>0;
        }
    }
    else {
        const auto destination=Vector2(target*Vector2i(48,32));
        const bool eight=effect==520 || effect==578 || effect==593;
        // 4495D0 selects projectile frames from tile coordinates, before pixel scaling.
        const int offset=effect==544 || effect==545?0:
            10*monster_projectile_direction(Vector2(target-monster.cell),eight);
        show_scene_effect(Section::magic,effect,monster.cell,offset,delay,0,
            MonsterProjectile{start,destination,target_actor,effect});
    }
    return monster.shape!=124 && monster.shape!=125;
}

void Mir3HudView::advance_monsters(double delta,bool scene_movement_tick) {
    if (!reference_monsters.empty() && bool(get_meta("offline_preview",false)) && bool(get_meta("reference_auto_tick",true))) {
        reference_tick_remainder_ms+=std::max(0.0,delta)*1000;
        const int elapsed=int(std::min(10000.0,reference_tick_remainder_ms));
        if (elapsed>0) {
            reference_tick_remainder_ms-=elapsed;
            Array targets;
            for (const auto& [id,actor]:reference_monsters) {
                const auto found=scene_chickens.find(id);if(found==scene_chickens.end()) continue;
                Dictionary target;target["id"]=id;target["cell"]=found->second.cell;
                target["alive"]=found->second.death_frame<0 && !found->second.buried && (!actor.combat || !actor.combat->dead);
                target["proper"]=true;targets.append(target);
            }
            advance_monster_ai(elapsed,targets);
        }
    }
    for (auto& [actor,chicken]:scene_chickens) {
        if (chicken.appearance_duration_ms>0) {
            chicken.appearance_elapsed_ms+=delta*1000;
            if (chicken.appearance_elapsed_ms>=chicken.appearance_duration_ms) {
                chicken.appearance_duration_ms=0;
                chicken.appearance_elapsed_ms=0;
            }
            queue_redraw();
        }
        if (chicken.burrow_frame>=0) {
            chicken.burrow_ms+=delta*1000;
            const auto motion=monster_burrow_motion(chicken.shape);
            if (chicken.burrow_ms>(chicken.burrow_catch_up && chicken.burrow_frame>0?motion.interval/2:motion.interval)*(chicken.slowed()?2:1)) {
                chicken.burrow_ms=0;
                ++chicken.burrow_frame;
                if (chicken.burrow_frame==1)
                    play_actor_sound(actor,10*(chicken.shape+20),chicken.cell);
                if (chicken.burrow_frame>=motion.frames) {
                    chicken.burrow_frame=-1;
                    // 40E7F0 changes DIGDOWN state 2 to 3; 421FF0 deletes it.
                    // Gate shapes instead return to their open/closed standing direction.
                    const bool gate=(chicken.shape>=530 && chicken.shape<=533) || chicken.shape==538;
                    if (gate) chicken.facing=chicken.hide_after_burrow?0:7;
                    else if (chicken.hide_after_burrow) scene_removals.push_back(actor);
                    chicken.buried=false;
                    chicken.hide_after_burrow=false;
                    chicken.reverse_burrow=false;
                    chicken.hidden_pose=mir3::MonsterBehavior::for_shape(chicken.shape).dormant(chicken.status_flags);
                    if (chicken.buried) target_focus.remove(actor);
                }
                queue_redraw();
            }
            continue;
        }
        if (chicken.struck_frame>=0) {
            chicken.struck_ms+=delta*1000;
            const auto struck=monster_struck_motion(chicken.shape);
            if (chicken.struck_ms>(chicken.struck_catch_up && chicken.struck_frame>0?struck.interval/2:struck.interval)*(chicken.slowed()?2:1)) {
                chicken.struck_ms=0;
                ++chicken.struck_frame;
                // 406E60 selects the sample by monster shape and relative frame.
                if (chicken.struck_frame==1) play_actor_sound(actor,10*chicken.shape+204,chicken.cell);
                if (chicken.struck_frame>=struck.frames) chicken.struck_frame=-1;
                queue_redraw();
            }
        }
        if (chicken.walk_frame>=0 && scene_movement_tick) {
            const bool slowed=chicken.slowed() && !chicken.walking_backward;
            const bool advance=!slowed || !chicken.skip_movement_tick;
            chicken.skip_movement_tick=slowed && advance;
            if (advance) chicken.walk_frame+=(chicken.walking_backward || chicken.walk_catch_up)?2:1;
            if (advance && chicken.walk_frame==1) play_animal_call(actor,chicken);
            if (chicken.walk_frame>=6) {
                chicken.walk_frame=-1;
                chicken.idle_frame=0;
                chicken.idle_ms=0;
            }
            queue_redraw();
        }
        if (chicken.attack_frame>=0) {
            chicken.attack_ms+=delta*1000;
            if (chicken.attack_ms>chicken.attack_interval*(chicken.slowed()?2:1)) {
                chicken.attack_ms=0;
                ++chicken.attack_frame;
                if (chicken.attack_action==2 && (chicken.attack_frame==1 || chicken.attack_frame==2))
                    play_actor_sound(actor,10*chicken.shape+201+chicken.attack_frame,chicken.cell);
                if (chicken.attack_frame==1 && chicken.attack_action!=2)
                    play_actor_sound(actor,10*chicken.shape+202+chicken.attack_action,chicken.cell);
                if (chicken.attack_frame==1 && chicken.attack_catch_up) chicken.attack_interval/=2;
                if (chicken.attack_frame>=monster_attack_motion(chicken.shape,chicken.attack_action).frames) {
                    chicken.attack_frame=-1;
                    chicken.idle_frame=0;
                    chicken.idle_ms=0;
                }
                queue_redraw();
            }
        }
        // 40E7F0：每次动作返回待机时重新应用祖玛休眠位，而非仅在入场时判断。
        if (chicken.walk_frame<0 && chicken.struck_frame<0 && chicken.attack_frame<0 && chicken.death_frame<0)
            chicken.hidden_pose=mir3::MonsterBehavior::for_shape(chicken.shape).dormant(chicken.status_flags);
        if (!chicken.actions.empty() && chicken.walk_frame<0 && chicken.struck_frame<0 && chicken.attack_frame<0 &&
            (chicken.death_frame<0 || chicken.death_frame>=monster_death_motion(chicken.shape).frames-1 || chicken.skeleton)) {
            const bool catch_up=chicken.actions.size()>=3;
            const auto action=chicken.actions.front();
            chicken.actions.pop_front();
            if (apply_monster_action(actor,action)) {
                if (chicken.walk_frame==0) chicken.walk_catch_up=catch_up;
                if (chicken.attack_frame==0) chicken.attack_catch_up=catch_up;
                if (chicken.struck_frame==0) chicken.struck_catch_up=catch_up;
                if (chicken.burrow_frame==0) chicken.burrow_catch_up=catch_up;
                if (chicken.death_frame==0) chicken.death_catch_up=catch_up;
            }
        }
        if (!chicken.buried && !chicken.hidden_pose && chicken.death_frame<0 && chicken.walk_frame<0 && chicken.struck_frame<0 && chicken.attack_frame<0) {
            chicken.idle_ms+=delta*1000;
            const auto motion=monster_idle_motion(chicken.shape);
            if (chicken.idle_ms>motion.interval*(chicken.slowed()?2:1)) {
                chicken.idle_ms=0;
                chicken.idle_frame=(chicken.idle_frame+1)%motion.frames;
                if (chicken.idle_frame==1) play_animal_call(actor,chicken);
                queue_redraw();
            }
        }
        const auto death=monster_death_motion(chicken.shape);
        if (chicken.death_frame<0 || chicken.death_frame>=death.frames-1 || chicken.skeleton) continue;
        chicken.death_ms+=delta*1000;
        if (chicken.death_ms>(chicken.death_catch_up && chicken.death_frame>0?death.interval/2:death.interval)*(chicken.slowed()?2:1)) {
            chicken.death_ms=0;
            ++chicken.death_frame;
            if (chicken.death_frame==2 || chicken.death_frame==3)
                play_actor_sound(actor,10*chicken.shape+203+chicken.death_frame,chicken.cell);
            queue_redraw();
        }
    }
}

void Mir3HudView::draw_monsters(int tile_y,const SceneActorPainter& draw_scene_actor) {
        for (auto& [actor,chicken]:scene_chickens) {
            if (tile_y!=chicken.cell.y || chicken.buried) continue;
            const auto dye=monster_body_dye(chicken.shape);
            const int status_color=actor_status_color(chicken.status_flags);
            const auto draw_monster=[&](const MapActorFrame& body,Vector2i offset,const MapActorFrame* shadow) {
                const bool highlighted=hovered_target==std::uint32_t(actor) || locked_spell_actor==std::uint32_t(actor);
                const int appearance=chicken.appearance_weight(highlighted);
                draw_scene_actor(chicken,180,&body,offset,dye,status_color,shadow,appearance,highlighted);
                // 40ECB0 centers the pointer bounds within the current sprite.
                const auto full_size=Vector2i(chicken.bounds.size);
                const Vector2i excess(std::max(0,full_size.x-58),std::max(0,full_size.y-40));
                chicken.target_bounds=Rect2(chicken.bounds.position+Vector2(excess/2),Vector2(full_size-excess));
            };
            const auto shadow_frame=[&](const MapActorFrame& body,int action,int facing,int frame) {
                if (chicken.shape>=530 && chicken.shape<=533) return static_cast<const MapActorFrame*>(nullptr);
                if (const auto* shadow=monster_frame(chicken.shape,action,facing,frame,true)) return shadow;
                const bool flat=chicken.death_frame>=0 && chicken.shape!=127 && chicken.shape!=153 &&
                    (chicken.skeleton || chicken.death_frame>=monster_death_motion(chicken.shape).frames-1);
                return project_monster_shadow(body,flat);
            };
            if (chicken.burrow_frame>=0) {
                const int frame=chicken.reverse_burrow?monster_burrow_motion(chicken.shape).frames-1-chicken.burrow_frame:
                    chicken.burrow_frame;
                if (const auto* pose=monster_frame(chicken.shape,640,chicken.facing,frame))
                    draw_monster(*pose,{},shadow_frame(*pose,640,chicken.facing,frame));
                continue;
            }
            if (chicken.walk_frame>=0) {
                if (const auto* pose=monster_frame(chicken.shape,80,chicken.facing,chicken.walk_pose()))
                    draw_monster(*pose,chicken.walk_pixel()-chicken.cell*Vector2i(48,32),
                        shadow_frame(*pose,80,chicken.facing,chicken.walk_pose()));
                continue;
            }
            const int action=chicken.death_frame>=0?(chicken.skeleton?880:320):chicken.struck_frame>=0?240:
                chicken.attack_frame>=0?monster_attack_motion(chicken.shape,chicken.attack_action).base:
                chicken.hidden_pose?640:0;
            const int relative=chicken.death_frame>=0?chicken.death_frame:chicken.struck_frame>=0?chicken.struck_frame:
                chicken.attack_frame>=0?chicken.attack_frame:chicken.hidden_pose?0:chicken.idle_frame;
            const int facing=chicken.hidden_pose && action==640?0:chicken.facing;
            if (const auto* pose=monster_frame(chicken.shape,action,facing,relative))
                draw_monster(*pose,{},shadow_frame(*pose,action,facing,relative));
        }
}

// 原版怪物虚表起点为4AF3D4；4AF3D0属于前一个链表节点类型。
// 与40D8C0和4282C0的虚调用偏移逐项配对，不按相邻函数位置推断消息。
std::optional<bool> Mir3HudView::apply_monster_wire(int opcode,int actor,
    const std::array<std::uint16_t,3>& p,const PackedByteArray& encoded) {
    using namespace mir3::protocol;
    if (actor<=0 || actor==local_actor_identity) return std::nullopt;
    const auto body=mir3::decode_six_bit(encoded.ptr()+16,std::min<std::int64_t>(22,encoded.size()-16));
    const bool feature=body && body->size()==16 && (*body)[0]==3;
    const bool creates=opcode==SM_BACKSTEP || opcode==SM_TURN || opcode==SM_WALK || opcode==SM_RUN ||
        opcode==SM_DIGUP || opcode==SM_27 || opcode==SM_DEATH || opcode==SM_SKELETON ||
        opcode==SM_801 || opcode==SM_ACTOR_STATUS || opcode==SM_808;
    // 424280 calls 42E780 before 42EC70: pending removals precede actor creation.
    if (feature && creates && (scene_clear_pending || !scene_removals.empty()))
        apply_scene_removals();
    const bool exists=scene_chickens.contains(actor);
    if (!exists && !feature) return std::nullopt;
    if (!exists && !creates) return false;
    const int shape=feature?std::uint8_t((*body)[2])|(int(std::uint8_t((*body)[3]))<<8):exists?scene_chickens.at(actor).shape:-1;
    std::uint32_t status=0;
    if (feature) for (int i=0;i<4;++i) status|=std::uint32_t(std::uint8_t((*body)[4+i]))<<(i*8);
    const Vector2i cell(p[0],p[1]);
    const int facing=p[2]&255;
    const unsigned light=p[2]>>8;
    if (!exists) {
        // 42EABB -> 4061E7/4061F3 initializes the position from the packet.
        // The constructor supplies direction 0. Opcode 13 never consumes the packet direction.
        if (!feature || (opcode!=SM_RUN && facing>=8) ||
            !place_scene_chicken(actor,cell,0,shape)) return false;
    }
    auto& monster=scene_chickens.at(actor);
    if (!exists) monster.appearance_duration_ms=SceneChicken::arrival_appearance_ms;
    const auto entry=[&](bool appearance) {
        SceneChicken::Entry value{cell,facing,shape,light,status,appearance,std::nullopt,
            mir3::MonsterBehavior::for_shape(shape).dormant(status),opcode==SM_REVIVE};
        if (encoded.size()>38) {
            const auto name=mir3::decode_six_bit(encoded.ptr()+38,encoded.size()-38);
            if (name) {
                // 4085A0/407CC0 write the palette only when the name has a suffix.
                const auto slash=name->rfind('/');
                value.preserve_name_color=slash==std::string::npos;
                const auto color=slash==std::string::npos?255:
                    static_cast<unsigned char>(std::strtol(name->c_str()+slash+1,nullptr,10));
                mir3::SceneName identity;
                if (identity.update(SM_USERNAME,color,*name)) value.name=std::move(identity);
            }
        }
        return value;
    };
    const auto enqueue=[&](SceneChicken::Action action) { return receive_monster_action(actor,std::move(action),exists); };
    switch (opcode) {
    case SM_TURN: case SM_REVIVE: // 10/1316 -> +72 -> 4085A0: entry/standing refresh.
        if (!feature || facing>=8) return false;
        return enqueue(entry(false));
    case SM_WALK: case SM_BACKSTEP: // 11 -> 4082F0, 9 -> 409500.
        if (!feature || facing>=8) return false;
        return enqueue(SceneChicken::Turn{cell,facing,shape,light,opcode==SM_BACKSTEP,status,true});
    case SM_RUN: return true; // whitelist permits creation, but ordinary monster queue has no 13 branch.
    case SM_HIT: // 14 -> +100 -> 40CBF0.
        if (facing>=8) return false;
        return enqueue(SceneChicken::Attack{cell,facing,std::nullopt});
    case SM_DIGUP: // 20 -> +76 -> 408720.
        if (!feature || facing>=8) return false;
        return enqueue(SceneChicken::Burrow{cell,facing,shape,light,status});
    case SM_DIGDOWN: // 21 -> +80 -> 408B70.
        return enqueue(SceneChicken::ReverseBurrow{cell});
    case SM_FLYAXE: { // 22 -> +104 -> 40B0A0: WORD x/y + DWORD target.
        const auto target=mir3::decode_six_bit(encoded.ptr()+16,encoded.size()-16);
        if (!target || target->size()!=8 || facing>=8) return false;
        const auto word=[&](int i){ return std::uint8_t((*target)[i])|(int(std::uint8_t((*target)[i+1]))<<8); };
        std::uint32_t id=0;
        for (int i=0;i<4;++i) id|=std::uint32_t(std::uint8_t((*target)[4+i]))<<(8*i);
        return enqueue(SceneChicken::Attack{cell,facing,Vector2i(word(0),word(2)),id});
    }
    case SM_LIGHTING: // 23 -> +108 -> 409DB0; participates in the normal queue.
        if (encoded.size()!=38 || facing>=8) return false;
        return enqueue(SceneChicken::SpecialAttack{p,encoded});
    case SM_27: // +88 -> 4094A0: rise/revive, never death.
        if (!feature || facing>=8 || !terrain.contains(cell.x,cell.y)) return false;
        return enqueue(SceneChicken::Rise{cell,facing,shape,light,status});
    case SM_STRUCK: case SM_HUMAN_STRUCK: { // 31/1309 -> +96 -> 409600.
        if (!feature) return false;
        const int variant=std::uint8_t((*body)[14])|(int(std::uint8_t((*body)[15]))<<8);
        const SceneChicken::Hit hit{shape,variant,p};
        if (opcode==SM_HUMAN_STRUCK) return enqueue(SceneChicken::RelayedHit{hit});
        return enqueue(hit);
    }
    case SM_DEATH: case SM_SKELETON: case SM_NOWDEATH: // 32/33/34 -> +84 -> 4093B0.
        if (!feature || facing>=8) return false;
        return enqueue(SceneChicken::Death{cell,facing,shape,opcode,light,status});
    case SM_801: case SM_ACTOR_STATUS: case SM_808:
        if (!feature || facing>=8) return false;
        return enqueue(SceneChicken::Transfer{opcode,entry(true)});
    case SM_800: case SM_806:
        return enqueue(SceneChicken::Transfer{opcode,std::nullopt});
    case SM_CHARSTATUSCHANGE: // 657 -> +112 -> 407C60: immediate state DWORD.
        monster.status_flags=std::uint32_t(p[0])|(std::uint32_t(p[1])<<16);
        update_actor_bar_status(actor,(monster.status_flags&2)!=0);
        return true;
    case SM_HEALTHSPELLCHANGED: // 53 -> +64 -> 4081F0: current, mana, maximum.
        return apply_scene_health(actor,opcode,p[0],p[1],p[2]);
    case SM_CHANGELIGHT: monster.light_attribute=p[0]&255;return true;
    case SM_OPENHEALTH: update_actor_bar_status(actor,true);return true;
    case SM_CLOSEHEALTH: update_actor_bar_status(actor,false);return true;
    case SM_CHANGENAMECOLOR:
        return apply_scene_name(actor,SM_CHANGENAMECOLOR,p[0]&255,String());
    case SM_USERNAME: {
        const auto name=mir3::decode_six_bit(encoded.ptr()+16,encoded.size()-16);
        if (!name) return false;
        auto& value=scene_names[actor];
        return value.update(SM_USERNAME,p[0]&255,*name);
    }
    case SM_FEATURECHANGED: { // 4081C0 reads the feature DWORD from header +6, not the body.
        const auto packed=std::uint32_t(p[0])|(std::uint32_t(p[1])<<16);
        if ((packed&255)!=3) return false;
        return refresh_monster_shape(monster,int(packed>>16));
    }
    case SM_1317:
        return enqueue(SceneChicken::Attack{monster.cell,monster.facing,std::nullopt,0,6});
    default: return std::nullopt;
    }
}

bool Mir3HudView::remove_scene_actor(int actor) {
    reference_monsters.erase(actor);
    std::erase_if(monster_struck_deliveries,[&](const auto& event){return event.source==actor || event.target==actor;});
    animals.erase(actor);
    bee_queens.erase(actor);
    bees.erase(actor);
    const bool world_removed=plant_actors.erase(actor)!=0;
    return remove_scene_actor_display(actor) || world_removed;
}

bool Mir3HudView::remove_scene_actor_display(int actor) {
    const auto monster=scene_chickens.find(actor);
    const auto human=scene_humans.find(actor);
    const auto npc=scene_npcs.find(actor);
    std::optional<Vector2i> last_cell;
    if (monster!=scene_chickens.end()) last_cell=monster->second.cell;
    else if (human!=scene_humans.end()) last_cell=human->second.cell;
    else if (npc!=scene_npcs.end()) last_cell=npc->second.cell;
    const bool monster_removed=scene_chickens.erase(actor)!=0;
    monster_ai.actors.erase(actor);
    if (!monster_removed && !scene_npcs.erase(actor) && !scene_humans.erase(actor)) return false;
    const auto id=static_cast<std::uint32_t>(actor);
    for (auto& [event_id,event]:map_events) {
        if (event.projectile && event.projectile->target_actor==id) {
            // 44BE36 reconstructs the impact position from the last target tile.
            if (last_cell) event.projectile->destination=Vector2(*last_cell*Vector2i(48,32));
            event.projectile->target_actor=0;
        }
        if (event.follow_actor==id) {
            event.follow_actor=0;
            // 44BDB7 falls back to the last target tile once the target is gone.
            if (last_cell) {
                event.cell=*last_cell;
                event.follow_position.reset();
            }
        }
    }
    std::erase_if(map_events,[actor](const auto& entry) {
        const auto& event=entry.second;
        if (event.attached_actor!=actor) return false;
        event.copy->hide();
        event.copy->queue_free();
        return true;
    });
    remove_actor_sound(actor);
    scene_damage_glyphs.erase(actor);
    scene_health.erase(actor);
    scene_health_bounds.erase(actor);
    scene_names.erase(actor);
    scene_speech.erase(actor);
    speech_bounds.erase(actor);
    target_attributes_by_actor.erase(id);
    if (hovered_target==id) hovered_target=0;
    if (locked_spell_actor==id) locked_spell_actor=0;
    target_focus.remove(id);
    if (npc_dialogue_actor==actor) close_npc_dialogue();
    queue_redraw();
    return true;
}

void Mir3HudView::apply_scene_removals() {
    if (scene_clear_pending) {
        plant_actors.clear();
        while (!scene_humans.empty()) remove_scene_actor(scene_humans.begin()->first);
        while (!scene_npcs.empty()) remove_scene_actor(scene_npcs.begin()->first);
        while (!scene_chickens.empty()) remove_scene_actor(scene_chickens.begin()->first);
        scene_clear_pending=false;
    }
    for (const int actor:scene_removals) {
        const auto monster=scene_chickens.find(actor);
        // 42E780 preserves a monster while DIGDOWN state 2 is playing.
        if (monster!=scene_chickens.end() && monster->second.burrow_frame>=0 && monster->second.hide_after_burrow) {
            monster->second.actions.clear();
            continue;
        }
        const auto plant=plant_actors.find(actor);
        if (plant!=plant_actors.end() && plant->second.brain.hidden && !plant->second.dead &&
            (monster==scene_chickens.end() || monster->second.death_frame<0))
            remove_scene_actor_display(actor);
        else remove_scene_actor(actor);
    }
    scene_removals.clear();
}

void Mir3HudView::show_source_hit_effect(int actor,Vector2i cell,std::uint32_t source_id,
        std::optional<std::uint8_t>& last_monster_shape) {
    // 412781–4127E8 replace the cached feature only if the source resolves.
    if (const auto source=scene_chickens.find(source_id);source!=scene_chickens.end())
        last_monster_shape=std::uint8_t(source->second.shape);
    else if ((local_actor_identity && source_id==std::uint32_t(local_actor_identity)) ||
        scene_humans.contains(source_id) || scene_npcs.contains(source_id))
        last_monster_shape.reset();
    if (!last_monster_shape) return;
    switch (*last_monster_shape) {
    case 2: show_following_hit_effect(500,actor,cell);break;
    case 67: show_following_hit_effect(523,actor,cell);break;
    case 104: show_following_hit_effect(542,actor,cell);break;
    case 150: show_following_hit_effect(574,actor,cell);break;
    }
}

bool Mir3HudView::SceneChicken::can_be_butchered() const { return shape==20 || shape==21 || shape==119; }

int Mir3HudView::SceneChicken::appearance_weight(bool highlighted) const {
            if (!appearance_duration_ms) return -1;
            // 40ECB0 uses the integer frame clock and resets an out-of-range weight to zero.
            const int weight=31*int(appearance_elapsed_ms)/appearance_duration_ms+(highlighted?5:0);
            return weight>31?0:weight;
        }

bool Mir3HudView::SceneChicken::slowed() const { return (status_flags & 0x08000000u)!=0; }

int Mir3HudView::SceneChicken::walk_pose() const { return walk_frame>=0 && walking_backward?5-walk_frame:walk_frame; }

godot::Vector2i Mir3HudView::SceneChicken::walk_pixel() const {
            const auto direction=cell-walk_origin;
            const int step=walk_frame+1;
            // 4075B0 reanchors the backstep to the destination before the final frame.
            if (walking_backward && walk_frame>=(facing==7?4:1)) {
                const int remaining=6-step;
                return cell*godot::Vector2i(48,32)-direction*godot::Vector2i(8*remaining,5*remaining);
            }
            const int y=direction.x==0?32*step/6:5*step;
            return walk_origin*godot::Vector2i(48,32)+direction*godot::Vector2i(8*step,y);
        }

bool Mir3HudView::occupied_by_monster(Vector2i cell) const {
    return std::ranges::any_of(scene_chickens,[cell](const auto& actor) {
            const auto& monster=actor.second;
            if (monster.death_frame>=0 || monster.buried) return false;
            const auto offset=cell-monster.cell;
            const bool left_gate=monster.shape==530 || monster.shape==531;
            const bool right_gate=monster.shape==532 || monster.shape==533 || monster.shape==538;
            if (!left_gate && !right_gate) return offset==Vector2i();
            // 4536EB tests the occupied tiles of the gate in its current pose.
            if (monster.facing==7)
                return offset==Vector2i(0,-2) || offset==Vector2i(left_gate?1:-1,-1);
            if (monster.facing>=4) return false;
            if (offset==Vector2i() || offset==Vector2i(0,-1)) return true;
            if (left_gate) return offset==Vector2i(-1,0) || offset==Vector2i(1,-1);
            return offset==Vector2i(-1,-1) || offset==Vector2i(1,0) || offset==Vector2i(1,1) ||
                (monster.shape==538 && offset==Vector2i(2,1));
        });
}

int Mir3HudView::butcher_monster_at(Vector2 p) const {
    int corpse=0;
            for (const auto& [actor,chicken]:scene_chickens) {
                const auto distance=chicken.cell-player_cell;
                if (chicken.can_be_butchered() && chicken.death_frame>=0 && !chicken.skeleton && chicken.target_bounds.has_point(p) &&
                    std::max(std::abs(distance.x),std::abs(distance.y))<=1)
                    corpse=actor;
            }
    return corpse;
}

int Mir3HudView::monster_overlay_pose(const SceneChicken& actor,MonsterEffectPose attached_pose) const {
    int pose=-1;
                switch (attached_pose) {
                case MonsterEffectPose::attack: pose=actor.attack_frame; break;
                case MonsterEffectPose::struck: pose=actor.struck_frame; break;
                case MonsterEffectPose::death:
                    if (!actor.skeleton && (actor.shape==141 ||
                        actor.death_frame<monster_death_motion(actor.shape).frames-1))
                        pose=actor.death_frame;
                    break;
                case MonsterEffectPose::idle: pose=actor.idle_frame; break;
                case MonsterEffectPose::walk: pose=actor.walk_pose(); break;
                case MonsterEffectPose::burrow:
                    pose=actor.burrow_frame;
                    if (pose>=0 && actor.reverse_burrow) pose=monster_burrow_motion(actor.shape).frames-1-pose;
                    break;
                case MonsterEffectPose::skeleton: pose=actor.skeleton?0:-1; break;
                }
    return pose;
}

void Mir3HudView::clear_monsters() {
    reference_monsters.clear();
    reference_tick_remainder_ms=0;
    monster_struck_deliveries.clear();
    animals.clear();
    bee_queens.clear();
    bees.clear();
    plant_actors.clear();
    while (!scene_chickens.empty()) remove_scene_actor(scene_chickens.begin()->first);
}
void Mir3HudView::append_monster_snapshot(Dictionary& result) const {
    Dictionary chicken_bounds, monster_facings, monster_reverse_burrows;
    Dictionary monster_revive_elapsed, monster_appearance_elapsed, monster_appearance_weight;
    for (const auto& [actor,chicken]:scene_chickens) {
        monster_facings[actor]=chicken.facing;
        monster_reverse_burrows[actor]=chicken.reverse_burrow;
        monster_revive_elapsed[actor]=chicken.appearance_duration_ms==SceneChicken::revive_appearance_ms?
            chicken.appearance_elapsed_ms:-1;
        monster_appearance_elapsed[actor]=chicken.appearance_duration_ms>0?chicken.appearance_elapsed_ms:-1;
        monster_appearance_weight[actor]=chicken.appearance_weight(hovered_target==std::uint32_t(actor) || locked_spell_actor==std::uint32_t(actor));
    }
    result["scene_monster_facings"]=monster_facings;
    Dictionary monster_lights;
    for (const auto& [actor,chicken]:scene_chickens) monster_lights[actor]=chicken.light_attribute;
    result["scene_monster_lights"]=monster_lights;
    result["scene_monster_reverse_burrows"]=monster_reverse_burrows;
    result["scene_monster_revive_elapsed_ms"]=monster_revive_elapsed;
    result["scene_monster_appearance_elapsed_ms"]=monster_appearance_elapsed;
    result["scene_monster_appearance_weight"]=monster_appearance_weight;
    result["locked_spell_actor"]=locked_spell_actor;
    for (const auto& [actor,chicken]:scene_chickens) chicken_bounds[actor]=chicken.bounds;
    result["scene_chicken_bounds"]=chicken_bounds;
    Dictionary chicken_cells, chicken_steps, chicken_walk_poses, chicken_struck, chicken_pixels, monster_shapes, monster_burrow_frames, monster_attack_frames, monster_attack_actions, monster_death_frames, monster_body_dyes, actor_status_colors, monster_dormant;
    for (const auto& [actor,chicken]:scene_chickens) {
        chicken_cells[actor]=chicken.cell;
        chicken_steps[actor]=chicken.walk_frame;
        chicken_walk_poses[actor]=chicken.walk_pose();
        monster_shapes[actor]=chicken.shape;
        monster_dormant[actor]=chicken.hidden_pose;
        monster_body_dyes[actor]=monster_body_dye(chicken.shape);
        actor_status_colors[actor]=actor_status_color(chicken.status_flags);
        monster_burrow_frames[actor]=chicken.burrow_frame;
        monster_attack_frames[actor]=chicken.attack_frame;
        monster_attack_actions[actor]=chicken.attack_frame>=0?chicken.attack_action:0;
        monster_death_frames[actor]=chicken.death_frame;
        chicken_struck[actor]=chicken.struck_frame;
        chicken_pixels[actor]=chicken.walk_frame<0?chicken.cell*Vector2i(48,32):chicken.walk_pixel();
    }
    result["scene_chicken_cells"]=chicken_cells;
    result["scene_chicken_steps"]=chicken_steps;
    result["scene_monster_walk_poses"]=chicken_walk_poses;
    result["scene_monster_shapes"]=monster_shapes;
    result["scene_monster_dormant"]=monster_dormant;
    result["scene_monster_body_dyes"]=monster_body_dyes;
    result["scene_monster_status_colors"]=actor_status_colors;
    result["scene_monster_burrow_frames"]=monster_burrow_frames;
    result["scene_monster_attack_frames"]=monster_attack_frames;
    result["scene_monster_attack_actions"]=monster_attack_actions;
    result["scene_monster_death_frames"]=monster_death_frames;
    result["scene_chicken_struck"]=chicken_struck;
    result["scene_chicken_pixels"]=chicken_pixels;
}

void Mir3HudView::sync_monster_overlays() {
    for (auto& [actor,monster]:scene_chickens)
        if (monster.shape==141 || monster.shape==184 || monster.action_overlay) sync_monster_action_overlay(actor,monster);
}

void Mir3HudView::add_monster_lights(mir3::SceneLightGrid& grid) const {
    for (const auto& [id,animal]:scene_chickens) {
        if (animal.death_frame>=0 || animal.buried || !animal.bounds.has_area()) continue;
        grid.add(mir3::SceneLight::actor(int(animal.anchor.x)+24,int(animal.anchor.y)+16,animal.light_attribute,false));
    }
}

bool Mir3HudView::SceneChicken::accepts_melee_target() const {
    return death_frame<0 && !buried && burrow_frame<0;
}
bool Mir3HudView::SceneChicken::protected_from_melee() const {
    const int low_shape=shape&255;
    return low_shape==26 || low_shape==86 || (low_shape>=113 && low_shape<=115) ||
        low_shape==165 || low_shape==166 || (shape>=530 && shape<=533 && facing==7);
}
#endif
