#pragma once
#include "protocol/spell_request.hpp"
#include "magic/spell_timing.hpp"
#include "magic/spell_routes.hpp"
#include "magic/staged_spell.hpp"
#include "magic/spell_particle_pool.hpp"
#include "magic/spell_color.hpp"
#include "magic/spell_light.hpp"
#include "hud/human_actions.hpp"
#include "hud/snow_weather.hpp"
#include <stdexcept>
#include "spell_sound_checks.hpp"
#include "melee_spell_checks.hpp"

// 定向验收：协议字段独立性、DWORD冷却边界、人物循环动作、特殊阶段及粒子生命周期。
inline void spell_weapon_color_checks() {
    using mir3::SpellColor;
    const auto require=[](bool ok,const char* message) { if(!ok) throw std::runtime_error(message); };
    std::array<std::uint8_t,100> record{};
    std::array<std::uint16_t,7> attack{};
    record[SpellColor::element]=2;
    require(SpellColor::local_weapon_effect(record)==0,"普通元素强度为零不启用附魔");
    record[SpellColor::power+1]=1;
    require(SpellColor::local_weapon_effect(record)==409,"普通元素强度使用完整WORD");
    attack[4]=1;
    require(SpellColor::local_weapon_color(record,attack,{0,0,0})==SpellColor::palette[4],"本地元素攻击优先于普通武器底色");
    record[SpellColor::special_element]=3;
    require(SpellColor::local_weapon_effect(record)==410 &&
        SpellColor::local_weapon_color(record,attack,{0,0,0})==SpellColor::palette[2],"特殊武器元素优先于本地攻击与普通底色");
    record[SpellColor::special_element]=128;
    require(SpellColor::local_weapon_effect(record)==409,"特殊选择器按有符号BYTE判断");
    attack={};
    require(SpellColor::local_weapon_color(record,attack,{0,0,0})==SpellColor::palette[1],"无元素攻击退回武器底色");
    record[SpellColor::element]=255;
    require(SpellColor::local_weapon_effect(record)==0,"非法元素不能越界访问调色板");
}

// 疾光电影336：每格1光点+21碎片，独立寿命和离场清理。
inline void spell_lightning_checks() {
    using namespace mir3;
    const auto require=[](bool ok,const char* message) { if(!ok) throw std::runtime_error(message); };
    SnowWeather random;
    SpellParticlePool pool;
    pool.emit_lightning({100,200},random);
    require(pool.count()==22 && pool.glows[0] && pool.fragments[20] && !pool.fragments[21],"336每次生成一光点及二十一碎片");
    require(pool.glows[0]->size==18 && pool.glows[0]->life==1000 && pool.glows[0]->mass==10000,"光点初始尺寸寿命质量符合46AAB0");
    require(pool.fragments[0]->size>=2 && pool.fragments[0]->size<3 && pool.fragments[0]->mass==10,"碎片尺寸质量符合467D40");
    const auto color=pool.fragments[0]->color;
    const auto size=pool.fragments[0]->size;
    pool.advance(100,random);
    require(pool.glows[0]->size>18 && pool.fragments[0]->size<size && pool.fragments[0]->color==color,"光点扩张、碎片收缩且不改色");
    pool.advance(1601,random);
    require(pool.count()==0,"336粒子寿命结束释放");
    pool.emit_lightning({0,0},random);pool.clear();
    require(pool.count()==0,"换图清理336粒子");
}

// 冰波、类型9退场与疾光电影共用两个池，满池不覆盖，空槽优先复用。
inline void spell_shared_particle_checks() {
    using namespace mir3;
    const auto require=[](bool ok,const char* message) { if(!ok) throw std::runtime_error(message); };
    SnowWeather random;
    SpellParticlePool pool;
    pool.emit_ice({100,200},random);
    require(pool.count()==3 && pool.ice_count()==3,"冰系每次一光点和两碎片");
    require(pool.glows[0]->resource_frame()==530 && pool.fragments[0]->position[1]==216,"共享光点资源530及碎片16像素偏移");
    const auto size=pool.glows[0]->size;
    const auto color=pool.fragments[0]->color;
    pool.advance(100,random);
    require(pool.glows[0]->size>size && pool.fragments[0]->mass==4 && pool.fragments[0]->color==color,"光点扩张衰减与碎片恒色增重");
    for(int i=0;i<500;++i) pool.emit_ice({100,200},random);
    require(pool.count()==1501,"冰碎片共享1000上限，光点独立计数");
    pool.emit_lightning({0,0},random);
    require(pool.count()==1502 && pool.fragments[0]->ice(),"碎片满池后闪电仅增加光点，不覆盖冰碎片");
    for(int i=0;i<600;++i) pool.emit({0,0},random);
    require(pool.count()==3000,"150与冰系及闪电共享三池各1000容量");
    pool.glows[7].reset();pool.fragments[3].reset();
    pool.emit_lightning({0,0},random);
    require(pool.count()==3000 && !pool.glows[7]->ice() && !pool.fragments[3]->ice(),"混合发射复用首个空槽");
    pool.advance(1601,random);
    require(pool.count()==0,"混合粒子超过寿命全部释放");
}

inline void spell_checks() {
    melee_spell_checks();
    spell_shared_particle_checks();
    spell_sound_checks();
    spell_weapon_color_checks();
    spell_lightning_checks();
    using namespace mir3;
    const auto require=[](bool ok,const char* message) { if(!ok) throw std::runtime_error(message); };
    const SpellRequest request{std::uint16_t(SkillId::fireball),123,456,0x89abcdef,0x80000000};
    const auto header=request.header();
    require(header.identifier==protocol::CM_SPELL && header.recognition==0x01c8007b,
        "施法协议：坐标组合与消息编号");
    require(header.parameters==std::array<std::uint16_t,3>{0xcdef,1,0x89ab} && request.body()=="-2147483648",
        "施法协议：目标高低字与有符号时间文本");
    SpellTiming timing;
    timing.begin_animated(0xfffffff0u,100);
    require(!timing.key_ready(0x504u) && timing.key_ready(0x505u),"冷却跨DWORD回绕，等于边界仍拒绝");
    require(!timing.key_ready(0x504u,10,true) && timing.key_ready(0x505u,10,true) &&
        timing.key_ready(0x505u,12,true) && !timing.key_ready(0x505u,11,true),"持续按键只接受F10和F12并保留严格时间边界");
    timing.finish_target(8,0,0,1099);
    require(timing.movement_delay==1400,"人物目标移动延迟上界");
    timing.finish_target(8,3,0,1099);
    require(timing.movement_delay==0,"怪物目标不沿用人物延迟");
    require(SpellRoutes::get(int(SkillId::mootebo)).action==SpellAction::rush &&
        SpellRoutes::get(int(SkillId::spacemove3)).action==SpellAction::special,"两类特殊动作分离");
    require(SpellRoutes::get(int(SkillId::earthfire)).restrict_followup &&
        SpellRoutes::get(int(SkillId::firecharm)).restrict_followup &&
        !SpellRoutes::get(int(SkillId::hangmajinbub)).restrict_followup,"移动限制按第三张技能表登记22、13和14");

    HumanActionClock action;
    action.start_spell(int(SpellAction::area),20);
    for(int i=0;i<19;++i) action.advance(201);
    require(!action.finished() && action.spell_frame()==19,"长起手法术保持人物末段直到特效帧完成");
    action.advance(201);
    require(action.finished(),"长起手动作在下一帧结束");

    StagedSpell staged(SkillId::original_73,2,{1049,1060,75});
    require(staged.advance(75)==StagedSpell::Change::none && staged.stage==StagedSpell::Stage::opening,
        "特殊起手严格超过间隔才推进");
    staged.advance(1);
    require(staged.stage==StagedSpell::Stage::moving && staged.frame==1050,"1050进入移动阶段");
    staged.advance(17);
    require(staged.offset[0]>=23 && staged.offset[0]<=24 && staged.offset[1]>=6 && staged.offset[1]<=7,
        "73首个东向位移包含0.8比例和正弦偏移，容许三角运算截断的一像素边界");
    require(staged.advance(983)==StagedSpell::Change::load_explosion,"移动满一秒切换爆炸");
    staged.load_explosion({20,22,50});
    staged.advance(51);
    require(staged.advance(51)==StagedSpell::Change::finished,"爆炸独立帧段结束");

    std::array<std::uint16_t,7> attack{1,1,0,0,0,0,0};
    require(SpellColor::strongest(attack,{0,0,0})==SpellColor::palette[0],"元素低字节参与比较，平局保留先项");
    require(SpellLight::channel(0,2)==254 && SpellLight::channel(3,2)==2,"光尾保留BYTE回绕和下界");
    SnowWeather random;
    SpellParticlePool pool;
    for(int i=0;i<200;++i) pool.emit({400,300},random);
    require(pool.count()==1200,"两个粒子池独立容量：1000火星与200光点");
    for(int i=0;i<800;++i) pool.emit({400,300},random);
    require(pool.count()==2000,"满池不挤占另一池或覆盖已有粒子");
    pool.advance(901,random);
    require(pool.count()==0,"超过所有寿命后两池全部释放");
    pool.emit({400,300},random);pool.clear();
    require(pool.count()==0,"离场清理两类粒子");
}
