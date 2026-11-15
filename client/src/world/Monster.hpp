#pragma once
#include "world/monster_pursuit.hpp"
#include "world/plant_ai.hpp"
#include "world/zuma_ai.hpp"
#include "world/monster_prototypes.hpp"
#include "hud/monster_behavior.hpp"
#include <string>
#include <deque>
#include <memory>
#include <map>
#include <tuple>
#include "protocol/default_message.hpp"
#include "protocol/message_ids.hpp"

namespace mir3 {
// 参考版本分离：同名/相似怪物不能用Zircon规则覆盖旧C++规则，最终按宝藏版取舍。
namespace zircon {
using Time=std::int64_t;
struct AttackOptions {bool reflect=true,ignore_shield=false,critical=true,struck=true;};
struct CombatWorld;
struct AttackClock {
    Time attack_at=0,action_at=0;
    int attack_delay=0,move_delay=0;
    std::optional<int> slow;
    bool chain_slow=false,neutralized=false;
    void update(Time now);
};
enum class Element : std::uint8_t {none,fire,ice,lightning,wind,holy,dark,phantom};
struct MonsterObjectState {
    int x=0,y=0,direction=0;
    bool same_map=true,fear=false,dead=false,visible=true,activated=false;
    Time dead_at=0;
    std::optional<MonsterAiTarget> target;
    AttackClock clock;
    Element attack_element=Element::none;
    int death_cloud_duration_min=4000,death_cloud_duration_random=0;
    struct DeathCloud {Time due;int x,y,display_x,display_y;bool visible;};
    std::vector<DeathCloud> death_clouds;
};
class MonsterObject : public MonsterObjectState {
public:
    virtual ~MonsterObject();
    virtual bool in_attack_range() const;
    virtual void process_target(Time now,const CombatWorld& world);
    virtual void attack(Time now,const CombatWorld& world);
    void death_cloud(Time now,int tx,int ty,const CombatWorld& world);
    void process_death_clouds(Time now,const CombatWorld& world,
        const std::function<void(int,int,int,int,bool,Time)>& spawn);
    void dragon_repulse(Time now,const CombatWorld& world);
    void dragon_repulse_tick(Time now,const CombatWorld& world,const std::function<void(int,Time)>& enqueue);
    void dragon_repulse_end(int victim,const CombatWorld& world,const std::function<int(int,int,Element)>& attack,
        const std::function<int(int,int,int)>& push);
};
class Doll final {
public:
    enum class Race {player,monster,other};
    struct LinkedObject {bool registered,dead;};
    explicit Doll(Time created_at);
    bool visible=false,activated=false;
    int direction=5;
    Time alive_at=0,action_at=0,dead_at=0;
    std::optional<int> doll_target;
    bool can_move() const;
    bool can_attack() const;
    bool can_be_seen(const std::function<bool()>& base) const;
    void spawned(Time now,const std::function<void()>& base,const std::function<void()>& effect,
        const std::function<void(std::optional<int>)>& add_cursed);
    void process_actions(Time now,const std::function<void()>& add_all_objects);
    void process(Time now,const std::function<void()>& base,const std::function<std::optional<LinkedObject>()>& caster,
        const std::function<std::optional<LinkedObject>()>& victim,const std::function<void(int)>& set_hp);
    void process_search();
    void activate(const std::function<void()>& add_active);
    void deactivate();
    void die(Time now,const std::function<void()>& base);
    void despawned(const std::function<void()>& base,const std::function<void(std::optional<int>)>& remove_first_cursed);
    int attacked(int power,AttackOptions options,bool attacker_same_map,int ax,int ay,int tx,int ty,
        const std::function<void(int,int,AttackOptions)>& forward);
    std::optional<std::string> custom_name(Race race,const std::string& target_name) const;
private:
    Time appear_at_;
    bool pending_=true;
};
class BlockingObject {
public:
    bool visible=true,has_map=false;
    bool can_move() const;
    bool can_attack() const;
    bool walk(int direction) const;
    bool blocking(const std::function<bool()>& parent) const;
    bool can_be_seen(const std::function<bool()>& base) const;
    void process_roam();
    void process_search();
    int attacked(int power,AttackOptions options,const std::function<int(int,AttackOptions)>& parent);
    void hide(const std::function<void()>& broadcast_hide);
    void show(const std::function<void()>& add_all_objects,const std::function<void()>& broadcast_show);
    void die(const std::function<void()>& broadcast_hide);
    std::string custom_name() const;
};
struct CarnivorousPlantState {
    int find_range=5,hide_range=5;
    Time visible_at=0,shock_at=0,cell_at=0;
};
class CarnivorousPlant : public MonsterObject, public CarnivorousPlantState {
public:
    enum class Event { add_objects,show,hide,restore_health,clear_poison,remove_objects };
    CarnivorousPlant();
    void process_target(Time now,const CombatWorld& world) override;
    bool can_attack(bool base) const;
    bool can_move(bool base) const;
    bool blocking(bool base) const;
    bool can_be_seen(const std::function<bool()>& base) const;
    void process(Time now,const std::function<void()>& base,const std::function<bool()>& base_can_attack,
        const std::function<bool()>& in_attack_range,const std::function<void(Event)>& notify);
    void process_actions(Time now,const std::function<void(Event)>& notify);
private:
    std::deque<Time> removals_;
};
class ZumaGuardian {
public:
    virtual ~ZumaGuardian();
    bool visible=false,dead=false,avoid_firewall=true;
    int x=0,y=0,wake_range=7;
    Time action_at=0;
    std::optional<MonsterAiTarget> target;
    bool can_move(bool base) const;
    bool can_attack(bool base) const;
    bool apply_poison(const std::function<bool()>& base) const;
    int attacked(int power,AttackOptions options,const std::function<int(int,AttackOptions)>& base) const;
    virtual void wake(Time now,const std::function<void(ZumaGuardian&)>& show);
    void process(Time now,const std::function<void()>& base,
        const std::function<std::vector<ZumaGuardian*>(int,int,int)>& nearby,
        const std::function<void(ZumaGuardian&)>& show);
};
class TreeMonster {
public:
    int direction=0;
    bool passive=true;
    bool can_move() const;
    bool can_attack() const;
    bool can_attack_target() const;
    bool apply_poison() const;
    void process_roam();
    void process_search();
    void process_target();
    int attacked(AttackOptions options,const std::function<int(int,AttackOptions)>& base) const;
};
class QuartzTree : public TreeMonster {
public:
    struct Minion {int id,x,y;bool same_map;};
    bool dead=false,sub_spawned=false;
    int x=0,y=0,hp=0,maximum_hp=0;
    Time slave_at=0;
    std::optional<int> target;
    struct World {
        std::function<void()> base_process;
        std::function<std::optional<int>()> create_sub;
        std::function<bool(int,int,int,int)> spawn_on_map_random;
        std::function<void(int,std::optional<int>)> set_target;
        std::function<std::vector<Minion>()> minions;
        std::function<void(int)> clear_exp_owner,die,despawn;
        std::function<void()> show;
        std::function<void(int,int,std::optional<int>)> spawn_minions;
    };
    void process(Time now,const World& world);
    void spawn_sub(const World& world);
    bool spawn_minion(int id,const World& world);
    void process_regen();
};
class WhiteBone {
public:
    explicit WhiteBone(Time created_at);
    bool visible=false;
    int direction=5;
    Time action_at=0;
    void spawned(Time now,const std::function<void()>& base,const std::function<void()>& summon_effect);
    bool can_be_seen(const std::function<bool()>& base) const;
    void process(Time now,const std::function<void()>& add_all_objects);
private:
    Time appear_at_;
    bool pending_=true;
};
struct DelayedHit {int target,damage;Time due;Element element;};
class VoraciousGhost {
public:
    explicit VoraciousGhost(const MonsterPursuit::Random& random);
    int death_count=0,revive_count=0,maximum_hp=0;
    bool dead=false;
    Time revive_at=0,dead_at=0,action_at=0;
    int experience_divisor() const;
    bool can_drop_rewards() const;
    void die(Time now,Time corpse_duration,const MonsterPursuit::Random& random,const std::function<void()>& base);
    void process(Time now,const std::function<void()>& base,const std::function<void()>& show,const std::function<void(int)>& set_hp);
};
class UndeadSoul final {
public:
    explicit UndeadSoul(Time created_at);
    bool visible=false;
    int direction=5;
    Time action_at=0;
    void spawned(Time now,const std::function<void()>& base,const std::function<void()>& undead_soul_effect);
    bool can_be_seen(const std::function<bool()>& base) const;
    void process(Time now,const std::function<void()>& add_all_objects);
private:
    Time appear_at_;
    bool pending_=true;
};
struct CombatWorld {
    enum class Magic {none,lightning_ball,thunder_bolt,lightning_wave,cyclone,lightning_beam,pink_fire_ball,fire_ball,monster_scortched_earth,fire_wall,fire_storm,greater_frozen_earth,blow_earth,purification,
        ice_blades,ice_storm,sama_guardian_ice,sama_black_ice,sama_guardian_lightning,sama_blue_lightning,
        dragon_tornado,sama_guardian_wind,sama_white_wind,adamantine_fire_ball,scortched_earth,sama_guardian_fire,
        sama_phoenix_fire,sama_prophet_fire,sama_prophet_wind,sama_prophet_lightning,monster_ice_storm,poisonous_cloud,monster_thunder_storm,chain_lightning,green_sludge_ball,dragon_repulse};
    struct MagicCast {int direction;Magic magic;std::vector<int> targets;std::vector<std::pair<int,int>> locations;};
    struct RangeCast {int direction;std::vector<int> targets;};
    struct MagicCell {int x,y;std::optional<std::vector<int>> objects;};
    MonsterPursuit::Random random;
    std::function<bool()> can_attack;
    std::function<bool(int)> walk;
    std::function<void(int,int)> move_to;
    std::function<void(int,bool,int)> broadcast; // direction, ranged, primary target id
    std::function<int()> get_dc;
    std::function<int()> get_mc;
    std::function<void(DelayedHit)> enqueue;
    std::function<std::optional<int>(int,int)> first_attackable;
    std::function<std::vector<int>(int,int,int)> get_targets;
    std::function<void(int,Time)> enqueue_heal;
    std::function<bool(int,int)> teleport_cell_available; // cell非空且Movements为null
    std::function<void(int,int)> teleport;
    int maximum_view_range=0;
    std::function<void(std::shared_ptr<MagicCast>)> broadcast_magic;
    std::function<std::vector<MagicCell>(int,int,int)> cells_in_range;
    std::function<std::optional<MagicCell>(int,int)> cell;
    std::function<bool(int)> can_attack_target;
    std::function<std::pair<int,int>(int)> target_position;
    std::function<void(int)> set_hp;
    std::function<void(std::shared_ptr<RangeCast>)> broadcast_range;
    std::function<void(Time,Time,Time)> add_dragon_repulse_buff; // duration,frequency,first tick delay
};
struct MonsterPurification {
    enum class BuffType {other,magic_weakness,defensive_blow,heal,invisibility,magic_resistance,resilience,
        magic_shield,superior_magic_shield,elemental_superiority,blood_lust,defiance,might,reflect_damage,
        judgement_of_heaven,strength_of_faith,celestial_light,transparency,renounce,cloak,ghost_walk,
        life_steal,dark_conversion,evasion,raging_wind,invincibility};
    struct Buff {int id;BuffType type;};
    struct Poison {int id;bool parasite;};
    struct Target {bool registered,dead;int level;};
    struct World {
        std::function<std::optional<Target>(int)> target;
        std::function<bool(int)> can_help,can_attack;
        std::function<std::vector<Poison>(int)> poisons;
        std::function<std::vector<Buff>(int)> buffs;
        std::function<void(int,int)> remove_poison,remove_buff;
        std::function<void(int,Time)> enqueue;
    };
    static void cast(int x,int y,const MonsterAiTarget& target,int& direction,AttackClock& clock,
        Time now,const CombatWorld& combat,const World& world);
    static int deliver(int target,int caster_level,const MonsterPursuit::Random& random,const World& world);
};
struct SamaBlackState {
    int range_chance=5;
};
class SamaBlack : public MonsterObject, public SamaBlackState {
public:
    virtual ~SamaBlack();
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
    virtual void range_attack(Time now,const CombatWorld& world);
};
struct SamaBlueState {
    int range_chance=5;
};
class SamaBlue : public MonsterObject, public SamaBlueState {
public:
    virtual ~SamaBlue();
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
    virtual void range_attack(Time now,const CombatWorld& world);
};
struct SamaWhiteState {
    int range_chance=5;
};
class SamaWhite : public MonsterObject, public SamaWhiteState {
public:
    virtual ~SamaWhite();
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
    virtual void range_attack(Time now,const CombatWorld& world);
};
struct SamaIceGuardianState {
    int range_chance=5;
};
class SamaIceGuardian : public MonsterObject, public SamaIceGuardianState {
public:
    virtual ~SamaIceGuardian();
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
    virtual void range_attack(Time now,const CombatWorld& world);
};
struct SamaLightningGuardianState {
    int range_chance=5;
};
class SamaLightningGuardian : public MonsterObject, public SamaLightningGuardianState {
public:
    virtual ~SamaLightningGuardian();
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
    virtual void range_attack(Time now,const CombatWorld& world);
};
struct SamaWindGuardianState {
    int range_chance=5;
};
class SamaWindGuardian : public MonsterObject, public SamaWindGuardianState {
public:
    virtual ~SamaWindGuardian();
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
    virtual void range_attack(Time now,const CombatWorld& world);
};
struct SamaFireGuardianState {
    int range_chance=5;
};
class SamaFireGuardian : public MonsterObject, public SamaFireGuardianState {
public:
    virtual ~SamaFireGuardian();
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
    virtual void range_attack(Time now,const CombatWorld& world);
};
struct SamaWorld {
    enum class Flag {other,blood_stone,sama_sorcerer};
    struct Object {int id,x,y;bool monster;Flag flag;};
    std::function<std::vector<Object>()> map_objects;
    std::function<void(int,int,int,int)> teleport_random; // id,center x/y,radius,目标地图为自身地图
    std::function<void(int,int,std::optional<MonsterAiTarget>)> spawn_minions;
    std::function<bool(int,int,int,int)> spawn_random;
};
struct SamaScorcerState {};
class SamaScorcer : public MonsterObject, public SamaScorcerState {
public:
    bool in_attack_range() const;
    int attacked(int power,AttackOptions options,const std::function<int(int,AttackOptions)>& base,const SamaWorld& world);
    void process_target(Time now,const CombatWorld& combat,const SamaWorld& world);
    void attack(Time now,const CombatWorld& combat);
};
struct SamaProphetState {
    int view_range=0;
    Time check_at=0;
};
class SamaProphet : public MonsterObject, public SamaProphetState {
public:
    virtual ~SamaProphet();
    bool in_attack_range() const;
    void spawned(const std::function<void()>& base,const SamaWorld& world);
    bool spawn_minion(int id,const SamaWorld& world);
    void process(Time now,const std::function<void()>& base,const SamaWorld& world);
    int attacked(int power,AttackOptions options,const std::function<int(int,AttackOptions)>& base,const SamaWorld& world);
    void process_target(Time now,const CombatWorld& combat,const SamaWorld& world);
    virtual void range_attack(Time now,const CombatWorld& combat);
};
struct SamaPhoenixState {
    int range_chance=5;
};
class SamaPhoenix : public MonsterObject, public SamaPhoenixState {
public:
    virtual ~SamaPhoenix();
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
    virtual void range_attack(Time now,const CombatWorld& world);
};
struct ChristmasMonsterState {
    int extra_experience_rate=10;
    Time shock_at=0,rage_at=0;
    std::optional<int> exp_owner;
};
class ChristmasMonster : public MonsterObject, public ChristmasMonsterState {
public:
    struct Monster {int id,level,move_delay,extra_experience_rate;bool monster,pet,guard,christmas,dead,can_move,has_target,boss;std::optional<int> exp_owner;};
    struct World {
        std::function<std::vector<Monster>()> map_objects;
        std::function<void(int,int,int,int)> teleport_random;
        std::function<std::vector<Monster>(int,int,int,int)> owner_targets;
        std::function<void(int,int,int)> set_experience; // id,ExtraExperienceRate,EXPOwner
        std::function<void(int,int)> set_hp;
    };
    bool can_move() const;
    bool can_attack() const;
    void refresh_stats(const std::function<void()>& base);
    int attacked(AttackOptions options,const std::function<int(int,AttackOptions)>& base);
    std::uint32_t name_colour(Time now) const;
    void die(const MonsterPursuit::Random& random,const World& world,const std::function<void()>& base);
};
struct TornadoState {
    int id=0;
    bool safe_zone=false;
    Time visible_at=0,roam_at=0,roam_delay=1000;
};
class Tornado final : public MonsterObject, public TornadoState {
public:
    enum class Race {player,monster,item,npc,spell,unsupported};
    struct Target {Race race;bool self,registered,dead,visible,guard,castle_lord,game_master,safe_zone;};
    bool blocking() const;
    int pushed(int direction,int distance) const;
    bool can_attack_target(const Target& target) const;
    void spawned(Time now,const std::function<void()>& base,const std::function<void()>& show);
    void die(Time now,const std::function<void()>& base);
    void process(Time now,const std::function<void()>& base,const std::function<void()>& die);
    void process_roam(Time now,bool can_move,int seen_count,const std::function<bool()>& other_blocking,
        const CombatWorld& combat,const std::function<void(int)>& turn);
    void attack(Time now,const CombatWorld& combat);
};
struct IcySpiritWarriorState {
    int attack_range=8;
};
class IcySpiritWarrior : public MonsterObject, public IcySpiritWarriorState {
public:
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world,const std::function<void(int,Time)>& enqueue_function);
    void attack(Time now,const CombatWorld& world,const std::function<void(int,Time)>& enqueue_function);
    void deliver_function(int target);
};
struct IcySpiritGeneralState {
    int attack_range=8;
};
class IcySpiritGeneral : public MonsterObject, public IcySpiritGeneralState {
public:
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world,const std::function<void(int,Time)>& enqueue_function);
    void attack(Time now,const CombatWorld& world,const std::function<void(int,Time)>& enqueue_function);
    int deliver_function(int target,const std::function<int()>& get_dc,const std::function<int(int,int,Element)>& base_attack,
        const std::function<int(int)>& current_mp,const std::function<void(int,int)>& change_mp);
};
struct IcyGoddessState {
    int attack_range=8;
};
class IcyGoddess : public CarnivorousPlant, public IcyGoddessState {
public:
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
};
struct CrimsonNecromancerState {
    Time debuff_at=0;
};
class CrimsonNecromancer : public MonsterObject, public CrimsonNecromancerState {
public:
    struct Weakness {Time duration=10000;bool visible=false,pause=false;Time tick_frequency=0;};
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world,const std::function<void(int,Weakness)>& add_weakness);
    void attack(Time now,const CombatWorld& world);
};
struct DoomClawState {};
class DoomClaw : public MonsterObject, public DoomClawState {
public:
    enum class Move {wave,spit,right_pinch,right_swipe,left_pinch,left_swipe};
    enum class VictimClass {other,warrior,wizard,taoist,assassin};
    struct World {
        std::function<void()> regen;
        std::function<void(Move,int)> broadcast; // spit发送空Locations列表，其余未指定Locations。
        std::function<VictimClass(int)> victim_class; // 非玩家返回other。
        std::function<void(int,int,int)> pushed;
    };
    void spawned(Time now,const std::function<void()>& base,const std::function<void()>& show);
    int pushed(int direction,int distance) const;
    int attacked(int ax,int ay,int power,AttackOptions options,const std::function<int(int,AttackOptions)>& base);
    void process_ai(Time now,const CombatWorld& combat,const World& world);
    void perform(Move move,const std::vector<int>& targets,Time now,const CombatWorld& combat,const World& world);
};
struct MonsterPoisonousCloud {
    struct Spawn {int x,y,display_x,display_y;bool visible;int tick_count=1;Time tick_frequency=20000;int power=20;};
    static void cast(int x,int y,const MonsterAiTarget& target,int& direction,AttackClock& clock,Time now,
        const CombatWorld& combat,const std::function<void(Spawn)>& spawn); // Owner为当前怪物。
};
struct ChaosKnightState {
    Time cast_at=0;
};
class ChaosKnight : public MonsterObject, public ChaosKnightState {
public:
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& combat,const std::function<void(MonsterPoisonousCloud::Spawn)>& spawn);
    void attack(Time now,const CombatWorld& combat);
};
struct RedMoonTheFallenState {};
class RedMoonTheFallen : public MonsterObject, public RedMoonTheFallenState {
public:
    bool can_move() const;
    bool in_attack_range(bool same_map) const;
    void attack(Time now,const CombatWorld& combat,const std::function<void(int,std::shared_ptr<std::vector<int>>)>& broadcast);
};
struct BanyoCaptainState {
    Time cast_at=0;
};
class BanyoCaptain : public MonsterObject, public BanyoCaptainState {
public:
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& combat,const MonsterPurification::World& purification);
    void attack(Time now,const CombatWorld& combat);
};
struct MonsterFireWall {
    struct Cell {int map,x,y;};
    struct Pending {std::optional<Cell> cell;Time due;};
    enum class Effect {other,fire_wall,tempest};
    struct Object {int id;bool spell;Effect effect;};
    struct Spawn {Cell cell;int tick_count=15;Time tick_frequency=2000;};
    struct World {
        std::function<std::optional<Cell>(int,int)> current_map_cell;
        std::function<void(Pending)> enqueue;
        std::function<std::vector<Object>(Cell)> objects;
        std::function<void(int)> despawn;
        std::function<void(Spawn)> spawn; // Owner绑定施法怪物，Effect为FireWall。
    };
    static void cast(int x,int y,const MonsterAiTarget& target,int& direction,AttackClock& clock,
        Time now,const CombatWorld& combat,const World& world);
    static void deliver(Pending pending,const World& world);
};
class ZumaKing : public ZumaGuardian {
public:
    ZumaKing();
    int direction=0,max_stage=7,stage=0,hp=0,maximum_hp=0;
    bool same_map=true;
    AttackClock clock;
    Element attack_element=Element::none;
    void spawned(const std::function<void()>& base);
    void wake(Time now,const std::function<void(ZumaGuardian&)>& show) override;
    void process(Time now,const std::function<void()>& base,
        const std::function<std::vector<ZumaGuardian*>(int,int,int)>& nearby,
        const std::function<void(ZumaGuardian&)>& show,
        const std::function<void(int,int,std::optional<MonsterAiTarget>)>& spawn_minions);
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world,const MonsterFireWall::World& fire_wall);
    void range_attack(Time now,const CombatWorld& world,const MonsterFireWall::World& fire_wall);
    void attack(Time now,const CombatWorld& world);
};
struct OmaWarlordState {
    int hp=0,maximum_hp=0;
    bool can_teleport=true,bonus=false;
    Time teleport_at=0;
};
class OmaWarlord : public MonsterObject, public OmaWarlordState {
public:
    int attacked(int power,AttackOptions options,const std::function<int(int,AttackOptions)>& base,
        const std::function<void(int,int)>& teleport_nearby);
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
};
struct NumaWarlordState {
    Time teleport_at=0,pull_at=0;
};
class NumaWarlord : public MonsterObject, public NumaWarlordState {
public:
    enum class Action {teleport,pull};
    struct Pending {Action action;std::vector<int> targets;Time due;};
    struct TargetState {int x,y;bool registered,dead,same_map;};
    struct World {
        std::function<std::optional<TargetState>(int)> target_state;
        std::function<void(int,const std::vector<int>&)> broadcast_range;
        std::function<void(Pending)> enqueue;
        std::function<void(int,int,bool,bool)> teleport;
        std::function<void(int,int,Element)> attack;
        std::function<void(int,int,int)> pushed;
    };
    bool in_attack_range() const;
    std::optional<std::pair<int,int>> teleport_cell(int id,const CombatWorld& combat,const World& world) const;
    void process_target(Time now,const CombatWorld& combat,const World& world);
    void swing_attack(Time now,const CombatWorld& combat,const World& world);
    void teleport_attack(Time now,const CombatWorld& combat,const World& world);
    bool pull_attack(Time now,const CombatWorld& combat,const World& world);
    void deliver(const Pending& pending,const CombatWorld& combat,const World& world);
};
struct QuartzTurtleSubState {
    Time slave_at=0;
};
class QuartzTurtleSub : public MonsterObject, public QuartzTurtleSubState {
public:
    void process(Time now,const std::function<void()>& base,const std::function<int()>& minion_count,
        const std::function<void(int,int,std::optional<MonsterAiTarget>)>& spawn_minions);
    bool spawn_minion(int id,const std::function<bool(int,int,int,int)>& spawn_random);
};
class Affliction {
public:
    int x=0,y=0;
    Element attack_element=Element::none;
    void attack(const std::function<void(int)>& set_hp);
    void die(Time now,const CombatWorld& world,const std::function<void()>& base);
};
class InfernalSoldier final {
public:
    struct PendingHit {int target;bool melee;Time due;};
    int x=0,y=0,direction=0,attack_range=7;
    bool same_map=true,can_pvp_range=true,target_is_player=false;
    Time range_at=0,range_cooldown=0;
    std::optional<MonsterAiTarget> target;
    AttackClock clock;
    Element attack_element=Element::none;
    void spawned(Time now,const std::function<void()>& base,const std::function<void()>& show);
    bool can_attack_target(const std::function<bool()>& base) const;
    bool in_attack_range(Time now) const;
    void process_target(Time now,const CombatWorld& world,const std::function<void(PendingHit)>& enqueue);
    void attack(Time now,const CombatWorld& world,const std::function<void(PendingHit)>& enqueue);
    void deliver(PendingHit hit,const std::function<int()>& get_dc,const std::function<void(int,int,Element)>& base_attack);
};
class Larva {
public:
    int x=0,y=0,range=1;
    bool dead=false;
    std::optional<MonsterAiTarget> target;
    Element attack_element=Element::none;
    void process(const std::function<void()>& base,const std::function<void(int)>& set_hp);
    void attack(const std::function<void(int)>& set_hp);
    void die(Time now,const CombatWorld& world,const std::function<void()>& base);
};
class Terracotta {
public:
    enum class Event {add_all_objects,show,hide};
    virtual ~Terracotta();
    int x=0,y=0,direction=0;
    bool visible=false,can_phase=false,same_map=true;
    Time phase_at=0,cell_at=0,move_at=0;
    std::optional<MonsterAiTarget> target;
    AttackClock clock;
    Element attack_element=Element::none;
    bool can_attack(bool base) const;
    bool blocking(bool base) const;
    bool can_be_seen(const std::function<bool()>& base) const;
    bool in_attack_range() const;
    void phase_in(Time now,const std::function<void(Event)>& notify);
    void phase_out(Time now,const std::function<void(Event)>& notify);
    void process_actions(Time now);
    void update_move_time(Time now,const std::function<void()>& base);
    void process_target(Time now,const CombatWorld& world,const std::function<void(Event)>& notify);
    virtual void choose_attack(Time now,const CombatWorld& world);
    virtual void attack(Time now,const CombatWorld& world);
private:
    std::deque<Time> hide_actions_;
};
class TerracottaSub : public Terracotta {
public:
    TerracottaSub();
    void choose_attack(Time now,const CombatWorld& world) override;
    void attack(Time now,const CombatWorld& world) override;
    virtual void attack2(Time now,const CombatWorld& world);
};
struct TerracottaBossState {
    int attack_range=11;
};
class TerracottaBoss : public MonsterObject, public TerracottaBossState {
public:
    bool in_attack_range() const;
    bool in_melee_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
};
struct FlameDemonState {
    int cast=0,min=0,max=0;
    Time cast_at=0;
};
class FlameDemon : public MonsterObject, public FlameDemonState {
public:
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
};
struct CorrosivePoisonSpitterState {
    Time teleport_at=0;
};
class CorrosivePoisonSpitter : public MonsterObject, public CorrosivePoisonSpitterState {
public:
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
};
struct EvilElephantState {};
class EvilElephant : public MonsterObject, public EvilElephantState {
public:
    struct PendingHit {int target;bool apply_poison;Time due;};
    struct GreenPoison {int value;Time tick_frequency=2000;int tick_count=10;};
    void attack(Time now,const MonsterAiTarget& target,const CombatWorld& world,const std::function<void(PendingHit)>& enqueue);
    void deliver(PendingHit hit,const std::function<int()>& get_dc,const std::function<int()>& get_sc,
        const std::function<int(int,int,Element)>& base_attack,const std::function<void(int,GreenPoison)>& apply_poison);
};
struct BanyaLeftGuardState {};
class BanyaLeftGuard : public MonsterObject, public BanyaLeftGuardState {
public:
    virtual ~BanyaLeftGuard();
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
    virtual void range_attack(Time now,const CombatWorld& world);
};
struct SunFeralWarriorState {
    bool can_spawn=true;
};
class SunFeralWarrior : public BanyaLeftGuard, public SunFeralWarriorState {
public:
    int attacked(int power,AttackOptions options,const std::function<int(int,AttackOptions)>& base,
        const std::function<void(int,int,std::optional<MonsterAiTarget>)>& spawn_minions);
    void range_attack(Time now,const CombatWorld& world) override;
};
struct NumaMageState {};
class NumaMage : public MonsterObject, public NumaMageState {
public:
    ~NumaMage() override;
    void process_target(Time now,const CombatWorld& world) override;
    void range_attack(Time now,const CombatWorld& world);
};
struct EscortCommanderState {};
class EscortCommander : public MonsterObject, public EscortCommanderState {
public:
    void process_target(Time now,const CombatWorld& world) override;
    void range_attack(Time now,const CombatWorld& world);
};
struct MoonFeralWarriorState {
    Time teleport_at=0;
    bool bonus=false;
};
class MoonFeralWarrior : public NumaMage, public MoonFeralWarriorState {
public:
    void process_target(Time now,const CombatWorld& world) override;
    void attack(Time now,const CombatWorld& world) override;
};
struct OxFeralGeneralState {
    int cast=0;
    Time cast_at=0;
};
class OxFeralGeneral : public MonsterObject, public OxFeralGeneralState {
public:
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
};
struct BanyoWarriorState {
    Time teleport_at=0;
    bool bonus=false,double_damage=false;
};
class BanyoWarrior : public MonsterObject, public BanyoWarriorState {
public:
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
};
struct PinkBatState {};
class PinkBat : public MonsterObject, public PinkBatState {
public:
    virtual ~PinkBat();
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
    virtual void range_attack(Time now,const CombatWorld& world);
};
struct MonkeyState {};
class Monkey : public MonsterObject, public MonkeyState {
public:
    virtual ~Monkey();
    virtual bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    virtual void attack(Time now,const CombatWorld& world);
};
struct NumaHighMageState {
    int attack_range=6;
};
class NumaHighMage : public Monkey, public NumaHighMageState {
public:
    bool in_attack_range() const override;
    void attack(Time now,const CombatWorld& world) override;
};
struct FieryDancerState {
    int attack_range=10;
};
class FieryDancer : public MonsterObject, public FieryDancerState {
public:
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
};
struct SkeletonAxeThrowerState {
    int attack_range=7,fear_rate=6,fear_duration=2;
    Time fear_at=0;
};
class SkeletonAxeThrower : public MonsterObject, public SkeletonAxeThrowerState {
public:
    virtual ~SkeletonAxeThrower();
    bool in_attack_range() const;
    virtual void process_target(Time now,const CombatWorld& world);
    virtual void attack(Time now,const CombatWorld& world);
};
struct NumaStoneThrowerState {};
class NumaStoneThrower : public SkeletonAxeThrower, public NumaStoneThrowerState {
public:
    void attack(Time now,const CombatWorld& world) override;
};
struct OmaMageState {};
class OmaMage : public SkeletonAxeThrower, public OmaMageState {
public:
    void attack(Time now,const CombatWorld& world) override;
};
struct UmaKingState {
    int range_chance=5;
};
class UmaKing : public MonsterObject, public UmaKingState {
public:
    virtual ~UmaKing();
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    virtual void range_attack(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
};
struct WingedHorrorState {};
class WingedHorror : public UmaKing, public WingedHorrorState {
public:
    void range_attack(Time now,const CombatWorld& world) override;
};
struct EmperorSaWooState {};
class EmperorSaWoo : public MonsterObject, public EmperorSaWooState {
public:
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    void range_attack(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
};
struct GoruArcherState {
    bool can_teleport=true,bonus=false;
    int hp=0,maximum_hp=0;
    Time teleport_at=0;
};
class GoruArcher : public SkeletonAxeThrower, public GoruArcherState {
public:
    int attacked(int power,AttackOptions options,const std::function<int(int,AttackOptions)>& base,
        const std::function<void(int,int)>& teleport_nearby);
    void process_target(Time now,const CombatWorld& world) override;
    void attack(Time now,const CombatWorld& world) override;
};
struct HealerAntState {
    int healing=0,healing_cap=0;
};
class HealerAnt : public SkeletonAxeThrower, public HealerAntState {
public:
    struct HealTarget {int id,hp,maximum;bool registered,dead,heal_buff;};
    struct HealBuff {int healing,cap;bool infinite=true,visible=false,pause=false;Time tick_ms=1000;};
    bool can_help_target(const HealTarget& target,const std::function<bool()>& base) const;
    bool can_attack_target(const HealTarget& target,const std::function<bool()>& base_help) const;
    bool should_attack_target(const HealTarget& target,const std::function<bool()>& base_help) const;
    void process_search(const std::function<void()>& proper_search);
    void heal(const HealTarget* target,const std::function<void(int,HealBuff)>& add_buff);
    void attack(Time now,const CombatWorld& world) override;
};
struct WindfurySorcererState {};
class WindfurySorcerer : public MonsterObject, public WindfurySorcererState {
public:
    void attack(Time now,const MonsterAiTarget& target,const CombatWorld& world);
};
struct YumgonWitchState {
    int attack_range=10;
    Element aoe_element_=Element::none;
};
class YumgonWitch : public MonsterObject, public YumgonWitchState {
public:
    virtual ~YumgonWitch();
    virtual Element aoe_element() const;
    virtual void set_aoe_element(Element element);
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
};
struct DragonQueenState {
    Time slave_at=0;
};
class DragonQueen : public YumgonWitch, public DragonQueenState {
public:
    struct World {
        std::function<void()> base_process,base_die;
        std::function<std::vector<QuartzTree::Minion>()> minions;
        std::function<void(int)> clear_exp_owner,die,despawn;
        std::function<void(int,int,std::optional<int>)> spawn_minions;
        std::function<bool(int,int,int,int)> spawn_on_map_random;
        std::function<std::optional<int>()> create_dragon_lord;
        std::function<bool(int,int,int)> spawn_exact;
    };
    Element aoe_element() const override;
    void set_aoe_element(Element element) override;
    void process(Time now,const World& world);
    bool spawn_minion(int id,const World& world);
    void die(const World& world);
};
struct SonicLizardState {
    int range=3;
};
class SonicLizard : public MonsterObject, public SonicLizardState {
public:
    struct PendingHit {int target;bool melee;Time due;};
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world,const std::function<void(PendingHit)>& enqueue);
    void attack(Time now,const CombatWorld& world,const std::function<void(PendingHit)>& enqueue);
    void deliver(PendingHit hit,const std::function<int()>& get_dc,const std::function<void(int,int,Element)>& base_attack);
};
struct GiantLizardState {
    int attack_range=7;
    bool can_pvp_range=true,target_is_player=false;
    Time range_at=0,range_cooldown=0;
};
class GiantLizard : public MonsterObject, public GiantLizardState {
public:
    struct PendingHit {int target;bool melee;Time due;};
    bool in_attack_range(Time now) const;
    void process_target(Time now,const CombatWorld& world,const std::function<void(PendingHit)>& enqueue);
    void attack(Time now,const CombatWorld& world,const std::function<void(PendingHit)>& enqueue);
    void deliver(PendingHit hit,const std::function<int()>& get_dc,const std::function<void(int,int,Element)>& base_attack);
};
struct StomperState {};
class Stomper : public MonsterObject, public StomperState {
public:
    void attack(Time now,const MonsterAiTarget& target,const CombatWorld& world,
        const std::function<std::vector<int>(int,int,int)>& get_targets);
};
class GhostMage {
public:
    enum class Event { zombie_hole,add_objects,show };
    bool visible=false,dead=false,spectral_hit=false;
    int x=0,y=0,direction=0;
    Time visible_at=0,shock_at=0,cell_at=0;
    AttackClock clock;
    std::optional<MonsterAiTarget> target;
    Element attack_element=Element::none;
    bool can_attack(bool base) const;
    bool can_move(bool base) const;
    bool blocking(bool base) const;
    bool can_be_seen(const std::function<bool()>& base) const;
    // zombie_hole：当前位置/朝向，Owner为自身，TickCount=1，TickFrequency=60000ms。
    void process(Time now,const std::function<void()>& base,const std::function<void(Event)>& notify);
    void attack(Time now,const std::function<void(int)>& broadcast,const std::function<int()>& get_dc,
        const MonsterPursuit::Random& random,const std::function<void(DelayedHit)>& enqueue);
};
struct FerociousIceTigerState {
    bool spinning=false;
};
class FerociousIceTiger : public MonsterObject, public FerociousIceTigerState {
public:
    bool in_attack_range(const MonsterAiTarget& target,bool same_map) const;
    void location_changed(const std::function<void()>& base);
    void attack(Time now,const MonsterAiTarget& target,const CombatWorld& world,const std::function<bool(int)>& player_warrior);
};
struct QueenOfDawnState {
    bool has_spawn_info=false,bonus=false;
    Time teleport_at=0,ting_at=0;
};
class QueenOfDawn : public MonsterObject, public QueenOfDawnState {
public:
    void process_ai(Time now,const std::function<void()>& base,const std::function<std::vector<int>()>& map_snapshot,
        const std::function<bool(int)>& can_attack_target,const std::function<void(int)>& teleport_random);
    void process_target(Time now,const CombatWorld& world,const std::function<void()>& base);
    void attack(Time now,const CombatWorld& world);
};
struct MutantFleaState {
    int attack_range=7;
};
class MutantFlea final : public MonsterObject, public MutantFleaState {
public:
    bool in_attack_range(const MonsterAiTarget& target,bool same_map) const;
    void attack(Time now,const MonsterAiTarget& target,const CombatWorld& world);
};
struct BlasterMutantFleaState {
    int attack_range=5;
};
class BlasterMutantFlea final : public MonsterObject, public BlasterMutantFleaState {
public:
    bool in_attack_range(const MonsterAiTarget& target,bool same_map) const;
    void attack(Time now,const MonsterAiTarget& target,const CombatWorld& world);
};
struct SpittingSpiderState {};
class SpittingSpider : public MonsterObject, public SpittingSpiderState {
public:
    ~SpittingSpider() override;
    bool in_attack_range() const override;
    void attack(Time now,const CombatWorld& world) override;
    bool in_attack_range(const MonsterAiTarget& target,bool same_map) const;
    virtual void attack(Time now,const MonsterAiTarget& target,const CombatWorld& world);
};
struct GoruSpearmanState {
    bool can_teleport=true,bonus=false;
    int hp=0,maximum_hp=0;
    Time teleport_at=0;
};
class GoruSpearman : public SpittingSpider, public GoruSpearmanState {
public:
    int attacked(int power,AttackOptions options,const std::function<int(int,AttackOptions)>& base,
        const std::function<void(int,int)>& teleport_nearby);
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world);
    void attack(Time now,const CombatWorld& world);
    void attack(Time now,const MonsterAiTarget& target,const CombatWorld& world) override;
};
struct ShinsuState {
    bool mode=false;
    Time mode_at=0,appear_at=0;
    bool appearance_pending=true;
};
class Shinsu : public SpittingSpider, public ShinsuState {
public:
    enum class Event {summon,add_objects,show,hide};
    explicit Shinsu(Time now);
    bool can_attack(bool base) const;
    bool can_be_seen(bool base) const;
    void process_target(Time now,const CombatWorld& world) override;
    void spawned(Time now,const std::function<void()>& base,const std::function<void(Event)>& notify);
    void process(Time now,const std::function<void()>& base,const std::function<void(Event)>& notify);
    void process_actions(Time now,const std::function<void(Event)>& notify);
};
struct MirrorImageState {
    int range_chance=5,view_range=0;
    Element element=Element::none;
    Time cooldown=0,explode_at=0;
    bool spawned=false;
};
class MirrorImage : public MonsterObject, public MirrorImageState {
public:
    explicit MirrorImage(Time now);
    bool can_move() const;
    bool can_attack() const;
    int attacked() const;
    bool in_attack_range() const override;
    void process(Time now,const std::function<void()>& base,const std::function<bool()>& owner_registered,
        const std::function<bool()>& owner_dead,const std::function<void()>& despawn,const std::function<void(int)>& set_hp);
    void process_target(Time now,const CombatWorld& world) override;
    void range_attack(Time now,const CombatWorld& world);
    void activate(const std::function<void()>& add_active);
    void die(Time now,const std::function<void()>& clear_master,const std::function<void()>& effect);
    void despawned(const std::function<void()>& base,const std::function<void()>& remove_pet);
};
struct PuppetState {
    Time explode_at=0;
    std::vector<int> magics;
    std::vector<std::pair<int,int>> dark_stone_stats;
};
class Puppet final : public MonsterObject, public PuppetState {
public:
    enum class Effect {puppet,fire,ice,lightning,wind};
    struct Explosion {int target;Time due;bool near;std::vector<int> magics;std::vector<std::pair<int,int>> dark_stone_stats;int bonus=0;};
    explicit Puppet(Time now);
    bool can_move() const;
    bool can_attack() const;
    void process_search();
    void process(Time now,const std::function<void()>& base,const std::function<bool()>& owner_registered,
        const std::function<bool()>& owner_dead,const std::function<void()>& despawn,const std::function<void(int)>& set_hp);
    void process_target(const std::function<void(int)>& set_hp);
    void process_target(Time now,const CombatWorld& world) override;
    int attacked(int power,AttackOptions options,const std::function<int(int,AttackOptions)>& base,const std::function<void(int)>& set_hp);
    void activate(const std::function<void()>& add_active);
    void deactivate();
    void despawned(const std::function<void()>& base,const std::function<void()>& remove_pet);
    void die(Time now,const std::function<void()>& base,const std::function<bool()>& owner_registered,
        const std::function<std::vector<MonsterAiTarget>(int,int,int)>& owner_targets,Element affinity,
        const std::function<void(Explosion)>& enqueue_on_owner,const std::function<void(Effect)>& broadcast);
};
struct RazorTuskState {};
class RazorTusk : public MonsterObject, public RazorTuskState {
public:
    using PendingHit=SonicLizard::PendingHit;
    bool in_attack_range() const;
    void process_target(Time now,const CombatWorld& world,const std::function<void(PendingHit)>& enqueue);
    void attack(Time now,const CombatWorld& world,const std::function<void(PendingHit)>& enqueue);
    void deliver(PendingHit hit,const std::function<int()>& get_dc,const std::function<void(int,int,Element)>& base_attack);
};
struct DepartedMonsterState {};
class DepartedMonster : public MonsterObject, public DepartedMonsterState {
public:
    enum class Effect {puppet,puppet_fire,puppet_ice,puppet_lightning,puppet_wind};
    void die(Time now,const CombatWorld& world,const std::function<void()>& base_die,
        const std::function<void(Effect)>& broadcast_effect,const std::function<bool(int)>& alive_player,
        const std::function<void(int,int,int)>& spawn_minions);
    bool spawn_minion(int id,const std::function<bool(int,int,int)>& spawn);
};
class NetherworldGate {
public:
    enum class Event {gate_open,gate_closed,decrement_alive,clear_spawn_info,despawn};
    struct Object {int id,x,y;bool self,guard,player,monster,safe,dead;};
    int x=0,y=0,direction=0,view_range=0;
    Time despawn_at=0,search_at=0;
    bool has_spawn_info=false,activated=false;
    struct World {
        std::function<void()> base_process;
        std::function<void(Event)> notify;
        std::function<bool()> has_ship_region,resolve_instance_map;
        std::function<std::vector<std::pair<int,int>>()> ensure_points;
        std::function<std::vector<Object>()> map_objects;
        MonsterPursuit::Random random;
        std::function<void(int,int,int)> teleport_to_ship;
    };
    bool can_move() const;
    bool can_attack() const;
    int attacked() const;
    bool apply_poison() const;
    std::uint32_t name_colour() const;
    void process_search();
    void activate(const std::function<void()>& add_active);
    void deactivate();
    void spawned(Time now,const std::function<void()>& base,const std::function<void(Event)>& notify);
    void process(Time now,const World& world);
};
class JinamStoneGate {
public:
    enum class Event {gate_open,gate_closed,decrement_alive,clear_spawn_info,despawn};
    struct Object {int id,x,y;bool self,guard,player,safe,dead;};
    int x=0,y=0,direction=0;
    int view_range=0;
    Time despawn_at=0,search_at=0;
    bool has_spawn_info=false;
    struct World {
        std::function<void()> base_process;
        std::function<void(Event)> notify;
        std::function<bool()> has_lair_region,resolve_instance_map;
        std::function<std::vector<std::pair<int,int>>()> ensure_points;
        std::function<std::vector<Object>()> map_objects;
        MonsterPursuit::Random random;
        std::function<void(int,int,int)> teleport_to_lair;
    };
    bool can_move() const;
    bool can_attack() const;
    int attacked() const;
    bool apply_poison() const;
    std::uint32_t name_colour() const;
    void process_search();
    void spawned(Time now,const std::function<void()>& base,const std::function<void(Event)>& notify);
    void process(Time now,const World& world);
};
class GhostSorcerer {
public:
    int x=0,y=0,direction=0,attack_range=6;
    AttackClock clock;
    Element attack_element=Element::none;
    bool in_attack_range(const MonsterAiTarget& target,bool same_map) const;
    void attack(Time now,const MonsterAiTarget& target,const std::function<void(int)>& broadcast,
        const std::function<std::optional<int>(int,int)>& first_attackable,
        const std::function<int()>& get_dc,const std::function<void(DelayedHit)>& enqueue);
};
class Warewolf {
public:
    enum class Magic { greater_frozen_earth };
    enum class Element { ice };
    struct Cast { int direction;Magic magic=Magic::greater_frozen_earth;std::vector<std::pair<int,int>> locations; };
    struct Hit { int target,damage;Time due;Element element=Element::ice; };
    struct World {
        std::function<bool(int,int)> cell_exists;
        std::function<std::vector<int>(int,int)> attackable_objects;
        std::function<int()> get_dc;
        std::function<void(std::shared_ptr<Cast>)> broadcast;
        std::function<void(Hit)> enqueue_hit;
        std::function<void()> base_target;
    };
    Time cast_at=0;
    int direction=0;
    AttackClock clock;
    void process_target(Time now,int x,int y,const MonsterAiTarget* target,bool can_attack,const World& world);
};
class WedgeMothLarva {
public:
    int direction=0;
    std::optional<int> target;
    int maximum_minions=0;
    std::vector<int> minions;
    AttackClock clock;
    bool can_move() const;
    bool in_attack_range(bool same_map) const;
    void attack(Time now,const std::function<void()>& broadcast);
    struct World {
        MonsterPursuit::Random random;
        std::function<std::optional<int>()> create_minion;
        std::function<bool(int,int,int)> spawn_on_current_map;
        std::function<void(int,std::optional<int>)> bind_target_and_master;
        std::function<void()> base_function;
    };
    void process(Time now,int x,int y,const World& world);
private:
    std::deque<Time> pending_;
};
}
enum class ReferenceMonsterStatus : std::uint32_t { damage_armor=0x40000000u,petrified=0x04000000u };
bool reference_monster_petrified(std::uint32_t flags);
struct MonsterMeleeAttack {
    enum class Outcome { missed,damaged,undefined_speed };
    struct Victim { bool proper=true,human=false;unsigned hit_point=0,speed_point=0; };
    struct Hooks {
        std::function<int(int)> mitigate;
        std::function<void(int)> wear_weapon,damage;
        std::function<void(int,unsigned)> delayed_struck;
        std::function<void(int)> immediate_struck;
    };
    // 对应 CM_HIT 的有效目标路径。装备存在性由 wear_weapon 处理，护盾由 mitigate 处理。
    static Outcome resolve(std::uint16_t dc,unsigned hit_point,int weapon_strong,const Victim& victim,
        const MonsterPursuit::Random& random,const Hooks& hooks);
    static int armor_damage(int damage,std::uint16_t ac,const MonsterPursuit::Random& random);
};
struct MonsterVitals {
    int hp=0,maximum=0;
    std::uint16_t ac=0;
    bool bubble=false,damage_armor=false;
    unsigned bubble_level=0,bubble_time=0;
    int mitigate(int damage,const MonsterPursuit::Random& random);
    // 仅处理无人物装备的怪物路径；仍消费参考的衣服损耗随机数。
    int struck_damage(int damage,const MonsterPursuit::Random& random);
    void damage_health(int damage);
};
struct MonsterRunState {
    using Time=std::uint32_t;
    struct Target { int x,y;bool dead=false,ghost=false,same_map=true,proper=true; };
    struct Master { int x,y; };
    bool dead=false,hidden=false,ghost=false,stone=false,petrified=false;
    bool holy_seize=false,run_away=false,no_attack=false,has_mission=false;
    int x=0,y=0,target_x=-1,target_y=-1,mission_x=0,mission_y=0;
    std::optional<Target> target;
    std::optional<Master> master;
    Time focused_at=0,holy_started=0,holy_duration=0,walked_at=0,walk_interval=0;
    Time runaway_started=0,runaway_duration=0;
    Time thought_at=0,hit_at=0,hit_interval=0;
    bool duplicate=false;
    int direction=4,target_direction=0;
    struct Hooks {
        std::function<void()> break_holy;
        std::function<bool()> think,attack;
        std::function<std::pair<int,int>()> master_back;
        std::function<bool(int,int)> occupied;
        std::function<void(int,int)> teleport_to_master_map;
        std::function<void()> go_to_target,wander,inherited;
    };
    void run(Time now,const Hooks& hooks);
    // walk 必须同步更新 x/y；命中与广播分开，保留参考先伤害后动作的顺序。
    bool think(Time now,const std::function<int()>& duplicate_count,const MonsterPursuit::Random& random,
        const std::function<void(int)>& walk);
    bool attack_target(Time now,const std::function<void()>& hit,const std::function<void()>& announce,
        const std::function<void()>& break_holy);
    bool struck(Time now,unsigned level,const Target* hitter);
    Time found_path_at=0;
    bool escape=false;
    bool walk_to(int facing,const std::function<bool(int,int)>& safe,const MonsterPursuit::Walk& walk);
    void wander(const MonsterPursuit::Random& random,const std::function<void(int)>& turn,
        const std::function<void(int)>& walk);
    // head 只代表可见链表首项，空首项不得用后续对象替代。
    void go_to_target(Time now,const MonsterAiTarget* head,const std::function<void(const MonsterAiTarget&)>& select,
        const MonsterPursuit::Random& random,const std::function<void(int)>& walk);
};
class AnimalMonster {
public:
    MonsterRunState state;
    // CAnimalObject 未初始化这三个字段；未知值不得作为可采肉量或骨架状态使用。
    std::optional<bool> skeleton;
    std::optional<int> meat_quality,body_leathery;
    void run(std::uint32_t now,const MonsterRunState::Hooks& hooks);
};
class MinorNumaMonster {
public:
    bool dead=false;
    std::optional<int> target;
    std::uint32_t searched_at=0;
    void run(std::uint32_t now,const std::function<std::optional<int>()>& select,const std::function<void()>& base);
};
class OmaMonster {
public:
    bool dead=false,ghost=false;
    std::optional<int> target;
    std::uint32_t searched_at=0;
    void run(std::uint32_t now,const std::function<std::optional<int>()>& select,const std::function<void()>& base);
};
class SpitSpiderMonster {
public:
    struct Victim { int id;bool self,proper;unsigned speed; };
    using Query=std::function<std::optional<Victim>(int,int)>;
    using Damage=std::function<void(int,int,unsigned)>;
    int direction=0;
    // mitigate 对应蜘蛛自身 GetMagStruckDamage；apply 立即扣血并排程 delay_ms 后受击。
    void spit(int facing,int x,int y,std::uint16_t dc,unsigned hit_point,
        const MonsterPursuit::Random& random,const std::function<void()>& announce,
        const Query& query,const std::function<int(int)>& mitigate,const Damage& apply);
};
enum class SummonEvent { emerge,retreat,change_face,make_ghost,release_master,remove };
// notify 只记录事件/排队操作，不得同步销毁正在 Run 的对象。
// clone 回调负责复制世界属性并保存新旧对象标识；普通形态切换中的 emerge 属于新战斗形态。
using SummonNotify=std::function<void(SummonEvent)>;
using SummonBaseRun=std::function<void()>;
class WhiteSkeletonMonster {
public:
    bool first=true,hidden=true;
    int direction=0;
    std::uint32_t walked_at=0;
    void run(std::uint32_t now,const SummonNotify& notify,const SummonBaseRun& base);
};
class ElfWarriorMonster;
class ElfMonster {
public:
    bool first=true,hidden=true,no_attack=true,dead=false;
    bool has_target=false,master_has_target=false;
    std::uint32_t walked_at=0,death_at=0;
    void appear_now(std::uint32_t now);
    void run(std::uint32_t now,const std::function<ElfWarriorMonster*()>& clone,
        const SummonNotify& notify,const SummonBaseRun& base);
};
class ElfWarriorMonster {
public:
    bool first=true,hidden=false,dead=false;
    bool has_target=false,master_has_target=false;
    std::uint32_t walked_at=0,death_at=0;
    void appear_now(std::uint32_t now,const SummonNotify& notify);
    void run(std::uint32_t now,const std::function<ElfMonster*()>& clone,
        const SummonNotify& notify,const SummonBaseRun& base);
};
class RonMonster {
public:
    using Time=std::uint32_t;
    using SelectTarget=std::function<std::optional<MonsterAiTarget>()>;
    using QueryObjects=std::function<std::vector<int>(int,int,int)>;
    using Attack=std::function<void(int)>;
    RonMonster(Time now,unsigned attack_interval_ms);
    bool dead=false,ghost=false,petrified=false;
    std::optional<MonsterAiTarget> target;
    int facing=0,target_direction=0;
    // 调用方的 QueryObjects 对应 GetMapObject，Attack 对应 _Attack，返回值表示发送普通攻击动作。
    bool run(Time now,int x,int y,const SelectTarget& select_target,const QueryObjects& query,const Attack& attack);
private:
    Time searched_at_,hit_at_;
    unsigned attack_interval_ms_;
};

class LightningZombieMonster {
public:
    using Time=std::uint32_t;
    using SelectTarget=RonMonster::SelectTarget;
    struct Result { bool back_position=false,lightning=false,early_base=false; int direction=-1; };
    LightningZombieMonster(Time now,unsigned walk_interval_ms,unsigned attack_interval_ms);
    bool dead=false,ghost=false,petrified=false;
    std::optional<MonsterAiTarget> target;
    Time walked_at;
    Result run(Time now,int x,int y,const SelectTarget& select_target,const MonsterPursuit::Random& random);
private:
    Time searched_at_,hit_at_;
    unsigned walk_interval_ms_,attack_interval_ms_;
};

class RockManMonster {
public:
    using Time=std::uint32_t;
    enum class Action { idle,emerge,retreat };
    using SelectTarget=std::function<std::optional<int>()>;
    bool hidden=true,dead=false,ghost=false;
    std::optional<int> target_id;
    Time searched_at=0,walked_at=0;
    Action run(Time now,int x,int y,std::span<const MonsterAiTarget> visible,const SelectTarget& select_target);
};

class SandMobMonster {
public:
    using Time=std::uint32_t;
    struct Result {
        bool retreat=false;
        bool search=false;
        bool attack_target=false;
        bool skip_base=false;
    };
    SandMobMonster(Time now,unsigned walk_interval_ms,unsigned attack_interval_ms);
    bool hidden=true,dead=false,ghost=false;
    Time appeared_at=0,walked_at,hit_at;
    Result run(Time now,int x,int y,unsigned hp,unsigned maximum,
        std::span<const MonsterAiTarget> visible,const MonsterAiTarget* target);
private:
    unsigned walk_interval_ms_,attack_interval_ms_;
};

// 子体独立实例。当前世界夹具复用通用追踪；数据库行的 wRaceIndex 仍待核对。
struct BeeMonster {
    MonsterPursuit pursuit;
    std::uint64_t next_turn=0;
};
// CBeeQueen 的独立状态。坐标/朝向由世界实体保存；创建回调在蜂后当前格生成幼蜂。
// 计时参数由调用方提供，不把测试夹具的间隔当作原服数据库值。
class BeeQueenMonster {
public:
    using Time=std::uint64_t;
    using SelectTarget=std::function<std::optional<int>()>;
    using Spawn=std::function<std::optional<int>(std::optional<int>)>;
    using Alive=std::function<bool(int)>;
    BeeQueenMonster(Time now,unsigned walk_interval_ms,unsigned attack_interval_ms);
    bool dead=false,ghost=false,petrified=false;
    std::optional<int> target_id;
    std::vector<int> slaves;
    // 对应 Operate 的延迟队列阶段，先于 Run；不因死亡/石化而丢弃已排程消息。
    unsigned operate(Time now,const Spawn& spawn);
    // 返回 true 时世界层发送 RM_HIT；此类不做行走或伪造幼蜂数据库外观。
    bool run(Time now,const SelectTarget& select_target,const Alive& alive);
private:
    Time walked_at_,hit_at_;
    unsigned walk_interval_ms_,attack_interval_ms_;
    std::deque<Time> deliveries_;
};

// 怪物逻辑统一由 Monster.cpp 实现；本文件只声明跨模块使用的接口。
namespace zircon {
// 工厂值来自MonsterObject.GetMonster；未接入的AI明确拒绝，不能静默变为普通怪物。
enum class RuntimeAI : int {axe=7,uma_king=16,fear_axe=20,numa_mage=26,axe_alt=29,long_axe=34,
    numa_mage_alt=36,banya_guard=38,emperor=41,numa_high_mage=62,winged_horror=84,escort_commander=93,fiery_dancer=94,
    fire_guardian=107,ice_guardian=108,lightning_guardian=109,wind_guardian=110,phoenix=111,
    black=112,blue=113,white=114,oma_mage=118,pink_bat=121};
class Controller {
public:
    struct Position {int x,y,direction;};
    explicit Controller(RuntimeAI ai);
    ~Controller();
    Controller(const Controller&)=delete;
    Controller& operator=(const Controller&)=delete;
    static bool supports(int ai);
    void place(Position position);
    Position position() const;
    void configure(int attack_delay,int move_delay,Element element);
    void advance(Time now,std::optional<MonsterAiTarget> target,bool alive,const CombatWorld& host,
        const std::function<Position()>& live_position,const std::function<void(DelayedHit)>& deliver);
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
// 攻城世界接口：归属/参与名单使用真实标识，不借用离线combat_team。
struct ConquestWorld {
    enum class Race {player,monster,other};
    struct Object {
        int id=0,x=0,y=0,direction=0,hp=0,level=0,pk=0,redemption=0;
        Race race=Race::other;
        bool registered=false,dead=false,visible=true,same_map=true,passive=false;
        bool guard=false,castle_lord=false,game_master=false,invisible=false,cloak=false,transparent=false;
        std::optional<int> guild,guild_castle,pet_owner;
    };
    enum class Event {attack,range_attack,turn,revive,refresh_flag,taking_flag,preventing_flag,
        stopped_taking,capture,keep_corpse_one_year};
    struct Message {Event type;int actor=0,x=0,y=0,direction=0,value=0;std::optional<int> guild,other_guild;};
    std::function<std::optional<Object>(int)> object;
    std::function<std::vector<Object>()> players;
    std::function<std::optional<int>(int)> castle_owner;
    std::function<void(int,std::optional<int>)> set_castle_owner;
    std::function<void(const Message&)> emit;
    std::function<void(int)> clear_exp_owner;
    std::function<int(int,int,Element,AttackOptions)> attacked;
    std::function<std::optional<int>(int,int,int,int)> spawn_blocker; // parent,x,y,hp；host选择Blocker数据行并更新共享Health。
    std::function<void(int,bool)> show_blocker;
    std::function<void(int,int,bool)> set_health; // actor,hp,revive
    std::function<void(int,int,int,int,int,bool,Time)> spawn_death_cloud; // owner,cell x/y,display x/y,visible,tick_at
    std::function<void(int,int,bool)> conquest_stats; // player,damage,kill
    std::function<void(int,Time)> finish_lord_capture; // 清目标、Ping、重生Boss、延长至至少now+15min、更新全体城堡Buff。
};
class Guard {
public:
    int id=0,x=0,y=0,direction=0,view_range=0,red_point=0;
    bool activated=false;
    AttackClock clock;
    std::optional<int> target;
    bool can_attack_target(const ConquestWorld::Object& victim,const ConquestWorld& world) const;
    void advance(Time now,const CombatWorld& combat,const ConquestWorld& world);
    void activate(const std::function<void()>& add_active);
    void deactivate();
    bool blocking() const;
    bool can_move() const;
    int attacked() const;
    std::uint32_t name_colour() const;
private:
    struct Pending {Time due;int target;};
    std::vector<Pending> pending_;
};
class CastleObject {
public:
    virtual ~CastleObject();
    int id=0,x=0,y=0,direction=0,hp=0,maximum_hp=0,view_range=0,level=0;
    bool dead=false,cool_eye=false;
    std::optional<int> castle,guild,war_castle,target;
    std::vector<int> participants;
    AttackClock clock;
    Element attack_element=Element::none;
    static std::unique_ptr<CastleObject> create(int ai);
    virtual void spawned(const ConquestWorld& world);
    virtual void refresh(const ConquestWorld& world);
    virtual void process(Time now,const CombatWorld& combat,const ConquestWorld& world);
    virtual bool can_attack_target(const ConquestWorld::Object& victim) const;
    virtual bool should_attack_target(const ConquestWorld::Object& victim) const;
    virtual bool can_move(bool base) const;
    virtual bool can_attack(bool base) const;
    virtual bool regenerates() const;
    virtual bool accepts_poison() const;
    bool participant(std::optional<int> candidate) const;
    virtual int attacked(const ConquestWorld::Object& source,int power,Element element,AttackOptions options,const ConquestWorld& world);
};
class CastleGuard : public CastleObject {
public:
    int attack_range=15;
    bool can_move(bool base) const override;
    void process(Time now,const CombatWorld& combat,const ConquestWorld& world) override;
    int attacked(const ConquestWorld::Object& source,int power,Element element,AttackOptions options,const ConquestWorld& world) override;
    void repair(const ConquestWorld& world);
};
class CastleGate : public CastleObject {
public:
    bool closed=true,auto_open=true;
    Time close_at=0,search_at=0,search_delay=0;
    std::vector<int> blockers;
    bool can_move(bool base) const override;
    bool can_attack(bool base) const override;
    bool regenerates() const override;
    void spawn_blockers(int face,const ConquestWorld& world);
    void process(Time now,const CombatWorld& combat,const ConquestWorld& world) override;
    int attacked(const ConquestWorld::Object& source,int power,Element element,AttackOptions options,const ConquestWorld& world) override;
    bool blocking(bool base) const;
    void open(const ConquestWorld& world);
    void close(const ConquestWorld& world);
    void repair(const ConquestWorld& world);
    void die(const ConquestWorld& world);
    void check_direction(const ConquestWorld& world);
private:
    void wall(bool closed,const ConquestWorld& world);
};
class CastleFlag : public CastleObject {
public:
    Time contest_delay=30000,contest_at=0;
    std::optional<int> contester;
    bool can_move(bool base) const override;
    bool regenerates() const override;
    bool accepts_poison() const override;
    void spawned(const ConquestWorld& world) override;
    void refresh(const ConquestWorld& world) override;
    void process(Time now,const CombatWorld& combat,const ConquestWorld& world) override;
    int attacked(const ConquestWorld::Object& source,int power,Element element,AttackOptions options,const ConquestWorld& world) override;
private:
    void cancel(const ConquestWorld& world);
};
class CastleLord : public CastleObject {
public:
    bool passive=false;
    Time cast_at=0,cast_delay=15000;
    std::optional<int> exp_owner;
    bool regenerates() const override;
    bool accepts_poison() const override;
    bool can_attack_target(const ConquestWorld::Object& victim) const override;
    bool should_attack_target(const ConquestWorld::Object& victim) const override;
    void process(Time now,const CombatWorld& combat,const ConquestWorld& world) override;
    int attacked(const ConquestWorld::Object& source,int power,Element element,AttackOptions options,const ConquestWorld& world) override;
    bool die(Time now,const ConquestWorld& world,const std::function<void()>& base_die);
private:
    struct Cloud {Time due;int x,y,display_x,display_y;bool visible;};
    std::vector<Cloud> clouds_;
};
struct ArachnidGrazerState {std::vector<Time> hatch_actions;};
class ArachnidGrazer : public MonsterObject, public ArachnidGrazerState {
public:
    explicit ArachnidGrazer(const MonsterPursuit::Random& random);
    bool can_move() const;
    bool in_attack_range() const override;
    void attack(Time now,const CombatWorld& world) override;
    void process_actions(Time now,const std::function<void(int,int,std::optional<MonsterAiTarget>)>& spawn_minions,
        const std::function<void()>& base_function);
    bool spawn_minion(int id,const std::function<bool(int,int,int)>& spawn) const;
};
struct DragonLordState {Time slave_at=0;int attack_range=12;};
class DragonLord : public MonsterObject, public DragonLordState {
public:
    struct Minion {int id,x,y;bool same_map,boss;};
    bool in_attack_range() const override;
    void process_target(Time now,const CombatWorld& world) override;
    void attack(Time now,const CombatWorld& world) override;
    void process(Time now,const std::function<void()>& base,const std::function<std::vector<Minion>()>& minions,
        const std::function<void(int)>& clear_exp_owner,const std::function<void(int)>& die,
        const std::function<void(int)>& despawn,const std::function<void(int,int,std::optional<MonsterAiTarget>)>& spawn_minions);
    bool spawn_minion(int id,const std::function<bool(int,int,int,int)>& spawn_random) const;
};
struct MonasteryBossState {bool has_died=false;};
class MonasteryBoss : public PinkBat, public MonasteryBossState {
public:
    void die(const std::function<void()>& base,const std::function<void(int,int,std::optional<MonsterAiTarget>)>& spawn_minions);
    void range_attack(Time now,const CombatWorld& world) override;
};
struct HalloweenMonsterState {
    long double bonus_experience=0;
    Time next_monster_kill=0;
    struct Pending {int target;Time due;};
    std::vector<Pending> attacks;
};
class HalloweenMonster : public MonsterObject, public HalloweenMonsterState {
public:
    struct Victim {
        int id=0,x=0,y=0,direction=0,hp=0,level=0,move_delay=0;
        bool registered=false,dead=false,visible=true,guard=false,castle_lord=false,self=false;
        bool player=false,monster=false,boss=false,pet=false,can_move=false,has_target=false;
        long double experience=0;
    };
    struct World {
        std::function<std::optional<Victim>(int)> object;
        std::function<bool(int)> base_can_attack;
        std::function<void(int)> clear_exp_owner;
        std::function<void(int,int,Element)> attacked;
        std::function<void(int,int,int,bool)> broadcast_pose; // x,y,direction,is_turn
        std::function<void()> teleport_random;
        std::function<std::vector<Victim>()> map_objects;
        std::function<std::vector<std::pair<int,int>>()> bosses; // database id,level
        std::function<void(int,int,int,int)> spawn_boss_random,teleport_near;
    };
    World interactions;
    bool in_attack_range() const override;
    std::uint32_t name_colour() const;
    long double experience(long double base) const;
    void spawned(const std::function<void()>& base,long double& experience_rate,long double& drop_rate,long double& gold_rate);
    bool can_attack_target(const Victim& victim,const World& world) const;
    void attack(Time now,const CombatWorld& combat) override;
    void attack(Time now,const CombatWorld& combat,const World& world);
    void process_actions(Time now,const CombatWorld& combat,const World& world);
    void activate(const std::function<void()>& add_active);
    void deactivate();
    void die(const std::function<void()>& base,bool safe_zone,const CombatWorld& combat,const World& world);
};
struct JinhwanSpiritState {int spawn_chance=100;};
class JinhwanSpirit : public MonsterObject, public JinhwanSpiritState {
public:
    void die(const std::function<void()>& base,const MonsterPursuit::Random& random,
        const std::function<void(int,int,std::optional<MonsterAiTarget>)>& spawn_minions);
};
struct ArchLichTaeduState {
    int max_stage=7,stage=0,min_spawn=20,random_spawn=5,hp=0,maximum_hp=0,max_minions=50;
    bool avoid_firewall=false;
};
class ArchLichTaedu : public MonsterObject, public ArchLichTaeduState {
public:
    void spawned(const std::function<void()>& base);
    void process(const std::function<void()>& base,const std::function<void()>& show,
        const std::function<void(int,int,std::optional<MonsterAiTarget>)>& spawn_minions);
    void process_target(Time now,const CombatWorld& world) override;
    void range_attack(Time now,const CombatWorld& world);
};
struct EnragedArchLichTaeduState {bool can_teleport=true,bonus=false;Time teleport_at=0;};
class EnragedArchLichTaedu : public ArchLichTaedu, public EnragedArchLichTaeduState {
public:
    int attacked(int power,AttackOptions options,const std::function<int(int,AttackOptions)>& base,
        const std::function<void(int,int)>& teleport_nearby);
    void process_target(Time now,const CombatWorld& world) override;
    void attack(Time now,const CombatWorld& world) override;
};
struct LordNiJaeState {};
class LordNiJae : public CarnivorousPlant, public LordNiJaeState {
public:
    enum class Poison {green,paralysis};
    struct PoisonInfo {Poison type;int value;Time tick_frequency;int ticks;};
    bool in_attack_range() const override;
    void attack(Time now,const CombatWorld& world) override;
    int deliver(int victim,int power,Element element,const std::function<int(int,int,Element)>& base,
        const MonsterPursuit::Random& random,const std::function<int()>& get_sc,
        const std::function<void(int,PoisonInfo)>& apply_poison);
};
struct EnragedLordNiJaeState {Time slave_at=0;};
class EnragedLordNiJae : public LordNiJae, public EnragedLordNiJaeState {
public:
    void process(Time now,const std::function<void()>& base,const std::function<std::vector<DragonLord::Minion>()>& minions,
        const std::function<void(int)>& clear_exp_owner,const std::function<void(int)>& die,
        const std::function<void(int)>& despawn,const std::function<void(int,int,std::optional<MonsterAiTarget>)>& spawn_minions);
    bool spawn_minion(int id,const std::function<bool(int,int,int,int)>& spawn_random) const;
};
struct FrostLordHwaState {Element affinity=Element::phantom;Time teleport_at=0,ting_at=0;};
class FrostLordHwa : public MonsterObject, public FrostLordHwaState {
public:
    enum class Poison {abyss,silenced,red,wraith_grip};
    struct World {
        std::function<bool(int)> is_monster;
        std::function<void(int,int)> set_hp;
        std::function<void(int,int,int,int)> teleport_random_near;
        std::function<void(int,Poison,Time,int)> poison;
    };
    World interactions;
    explicit FrostLordHwa(int day_of_week); // Sunday=0，采用服务器日期。
    void apply_bonus_stats(const std::function<void()>& base,const std::function<void(Element,int)>& set_affinity);
    void process_ai(Time now,const std::function<void()>& base,const CombatWorld& world);
    void process_target(Time now,const CombatWorld& world) override;
};
struct PachontheChaosbringerState {Time cast_at=0;};
class PachontheChaosbringer : public MonsterObject, public PachontheChaosbringerState {
public:
    bool in_attack_range() const override;
    void process_target(Time now,const CombatWorld& world) override;
};
struct BanyoLordGuzakState {Time slave_at=0,purify_at=0;};
class BanyoLordGuzak : public PachontheChaosbringer, public BanyoLordGuzakState {
public:
    void process(Time now,const std::function<void()>& base,const CombatWorld& world,const std::function<void(int)>& purify,
        const std::function<std::vector<DragonLord::Minion>()>& minions,const std::function<void(int)>& clear_exp_owner,
        const std::function<void(int)>& die,const std::function<void(int)>& despawn,
        const std::function<void(int,int,std::optional<MonsterAiTarget>)>& spawn_minions);
    bool spawn_minion(int id,const std::function<bool(int,int,int,int)>& spawn_random) const;
};
struct JinchonDevilState {Time cast_at=0,cast_delay=15000;int hp=0,maximum_hp=0,view_range=0;};
class JinchonDevil : public SpittingSpider, public JinchonDevilState {
public:
    bool in_attack_range() const override;
    void process_target(Time now,const CombatWorld& world) override;
    void attack(Time now,const CombatWorld& world) override;
};
struct CompanionState {
    enum class Action {none,moving,pickup,hunger,eating,idle};
    enum class Image {other,pig,sheep,tusk_lord,skeleton_lord,banyo_lord,rabbit,dog};
    enum class ItemType {other,head,back,amulet,poison};
    enum class Requirement {none,minimum_level,maximum_level};
    enum class Slot {bag,head,back,food};
    static constexpr int inventory_count=30,equipment_count=4,equipment_offset=1000;
    struct Stats {
        std::map<int,int> values;
        int bag_weight=0,inventory=0,hunger=0;
        bool operator==(const Stats&) const = default;
    };
    struct ItemInfo {
        int id=0,shape=0,weight=0,required_amount=0;
        std::int64_t stack_size=1;
        ItemType type=ItemType::other;
        Requirement requirement=Requirement::none;
        std::string type_name,rarity;
        unsigned classes=0;
        bool all_classes=true,currency=false,experience=false;
        Stats stats;
    };
    struct Task {int quest=0;std::int64_t amount=0,required=0;std::vector<int> objects;};
    struct Item {
        int id=0,slot=-1;
        const ItemInfo* info=nullptr;
        const ItemInfo* part_info=nullptr;
        std::int64_t count=1;
        bool quest=false,expirable=false,bound=false,worthless=false,non_refinable=false,locked=false,temporary=false;
        std::optional<int> character,account,mail,auction,companion,guild;
        Stats stats;
        std::shared_ptr<Task> task;
    };
    using ItemPtr=std::shared_ptr<Item>;
    struct GroundItem {int id,x,y;bool registered,same_map,monster_drop,pickable;};
    struct Skill {int level,weight,stat,maximum;};
    struct Speech {Action action;std::string text;};
    int companion_id=0,level=0,bag_weight=0,head_shape=0,back_shape=0,view_range=0;
    int hunger=0,maximum_hunger=0,level_weight=0,level_space=0;
    bool prevent_spell_check=true,move_to_owner=false;
    Image image=Image::other;
    Time search_at=0,next_chat_at=0,next_idle_at=0;
    std::optional<int> target_item;
    Stats stats,monster_stats;
    std::array<ItemPtr,inventory_count> inventory{};
    std::array<ItemPtr,equipment_count> equipment{};
    std::map<int,Stats> skills;
    std::vector<Speech> speeches;
    std::vector<std::string> filter_types,filter_rarities;
    unsigned filter_classes=0;
};
class Companion final : public MonsterObject, public CompanionState {
public:
    struct World {
        std::function<bool()> owner_sees,can_move;
        std::function<std::tuple<int,int,int>()> owner_position;
        std::function<std::pair<int,int>()> position;
        std::function<bool(int,int)> cell_exists,cell_has_movements;
        std::function<void(int,int)> teleport_to_owner_map;
        std::function<void(int,int,int)> move; // 更新格子、清隐身/透明、移除/加入可见对象、广播一步移动。
        std::function<void()> update_move_time;
        std::function<std::vector<GroundItem>()> visible_items;
        std::function<std::optional<GroundItem>(int)> ground_item;
        std::function<void(int)> pickup;
        std::function<void(int)> idle;
        std::function<void(const std::string&)> chat;
        std::function<void(int,int,int)> weight_update;
        std::function<void(int,int)> shape_update;
        std::function<void(std::int64_t)> food_changed;
        std::function<void(const ItemPtr&)> delete_item;
        std::function<void()> refresh_owner_buff,skills_changed;
        std::function<void(const std::vector<ItemPtr>&)> items_gained;
        std::function<void(int)> despawn_task_object,quest_changed;
        std::function<void(const ItemInfo&,std::int64_t)> gain_currency;
        std::function<void(const ItemInfo&,std::int64_t)> log_currency;
        std::function<void(std::int64_t)> gain_experience;
    };
    World interactions;
    Companion(Time now,int user_companion,const std::vector<ItemPtr>& items);
    bool blocking() const;
    bool can_data_be_seen() const;
    bool can_be_seen(bool viewer_is_owner,const std::function<bool()>& base,const std::function<bool()>& owner_visible) const;
    void activate(const std::function<void()>& add_active);
    void deactivate();
    void refresh_stats(const World& world);
    void refresh_weight(const World& world);
    void send_shape_update(const World& world);
    void recall(const World& world);
    void process_ai(Time now,const MonsterPursuit::Random& random,const World& world);
    void process_search(Time now,const MonsterPursuit::Random& random,const World& world);
    void process_roam(Time now,const MonsterPursuit::Random& random,const World& world);
    void process_target(Time now,const MonsterPursuit::Random& random,const World& world);
    void process_target(Time now,const CombatWorld& world) override;
    void process_idle(Time now,const MonsterPursuit::Random& random,const World& world);
    bool try_chat(Action action,Time now,const MonsterPursuit::Random& random,const World& world);
    void move_to(int tx,int ty,const MonsterPursuit::Random& random,const World& world);
    bool walk(int facing,const World& world);
    void remove_item(Item& item);
    bool can_use_item(const ItemInfo& info) const;
    bool can_wear_item(const Item& item,Slot slot,const std::function<bool(ItemType,Slot)>& correct_slot) const;
    void auto_feed(Time now,const MonsterPursuit::Random& random,const World& world);
    Stats get_skill(int level,const std::vector<Skill>& table,const MonsterPursuit::Random& random) const;
    void check_skills(const std::vector<Skill>& table,const MonsterPursuit::Random& random,const World& world);
    bool filter_picks(const Item& item) const;
    bool can_gain_items(bool check_weight,const std::vector<ItemPtr>& items) const;
    void gain_items(const std::vector<ItemPtr>& items,const World& world);
};
struct CombatState {
    int hp=0,maximum_hp=0,mp=0,accuracy=0,agility=0;
    int min_ac=0,max_ac=0,min_mr=0,max_mr=0,defensive_mastery=0;
    std::array<int,8> resistance{};
    int critical_chance=0,critical_damage=0,rebirth=0,life_steal=0;
    std::int64_t life_steal_hundredths=0;
    bool dead=false,abyss=false,red_poison=false,invincible=false,protection_ring=false;
    bool magic_shield_buff=false,superior_shield_buff=false;
    int shield_percent=0,superior_shield=0;
    Time shield_duration=0,shock_at=0,struck_at=0;
    bool struck_disabled=false;
    enum class Outcome {invalid,dodged,blocked,hit};
    struct Result {Outcome outcome=Outcome::invalid;int power=0,healed=0;bool struck=false,critical=false,shield_removed=false,killed=false;};
    static Result resolve(CombatState& attacker,CombatState& victim,int power,Element element,Time now,
        bool attacker_registered,bool same_map,bool within_view,const MonsterPursuit::Random& random);
};
}
// 1.45外观由服务端数据/场景feature指定，不能用Zircon AI编号替代。
struct Monster145Appearance {std::uint16_t shape=0;std::uint32_t status=0;std::uint8_t light=0;};
struct Monster145Message {
    protocol::DefaultMessage header;
    std::vector<std::uint8_t> body;
    // 已编码场景消息，不含连接加密、分帧；由连接层处理传输。
    std::string encode() const;
    static Monster145Message pose(protocol::MessageId opcode,std::uint32_t actor,int x,int y,int direction,Monster145Appearance appearance);
    static Monster145Message melee(std::uint32_t actor,int x,int y,int direction);
    static Monster145Message projectile(std::uint32_t actor,int x,int y,int direction,int tx,int ty,std::uint32_t target);
    static Monster145Message struck(std::uint32_t actor,Monster145Appearance appearance,std::uint16_t hp,
        std::uint16_t maximum_hp,std::uint16_t damage,std::uint16_t hit_variant=0);
    static Monster145Message health(std::uint32_t actor,std::uint16_t hp,std::uint16_t mp,std::uint16_t maximum_hp);
    static Monster145Message change_appearance(std::uint32_t actor,std::uint16_t shape);
};
struct MonsterMotion { int base,frames,interval; };
std::string ordinary_monster_archive(int library);
MonsterMotion monster_idle_motion(int shape);
MonsterMotion monster_attack_motion(int shape,int action=2);
MonsterMotion monster_struck_motion(int shape);
MonsterMotion monster_death_motion(int shape);
MonsterMotion monster_burrow_motion(int shape);
std::uint32_t monster_feature_status(std::string_view feature);
int monster_feature_shape(std::string_view feature);
int actor_status_color(std::uint32_t flags);
int monster_body_dye(int shape);
int monster_resource_direction(int shape,int action_base,int facing);
}
