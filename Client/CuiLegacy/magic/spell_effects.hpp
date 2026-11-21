#pragma once

#include "hud/scene_lighting.hpp"
#include "magic/magic_rules.hpp"
#include "magic/map_events.hpp"
#include "magic/spell_light.hpp"
#include "magic/spell_orbit.hpp"
#include "magic/spell_particle_pool.hpp"
#include "magic/spell_reply.hpp"
#include "magic/staged_spell.hpp"
#include "resources/effect_catalog.hpp"
#include <cmath>

namespace mir3 {
inline const char* spell_archive(unsigned library)
{
    switch (library) {
    case 81: return "Magic";
    case 91: return "Mon-4";
    case 129: return "MonMagic";
    case 137: return "MagicEx";
    case 138: return "MonMagicEx";
    case 140: return "MonMagicEx2";
    case 143: return "MonMagicEx3";
    case 148: return "MonMagicEx4";
    case 149: return "MagicEx2";
    default: return nullptr;
    }
}

using SpellCell = std::array<int,2>;
using SpellPixel = std::array<float,2>;
inline SpellPixel spell_pixel(SpellCell cell) { return {float(cell[0]*48),float(cell[1]*32)}; }
inline SpellCell spell_cell(SpellPixel point) {
    return {int(std::lround(point[0]/48)),int(std::lround(point[1]/32))};
}

inline int spell_direction(float dx,float dy,bool eight=false)
{
    if (!dx && !dy) return 0;
    const int count=eight ? 8 : 16;
    const int quadrant=dx<0 ? (dy<0 ? 3 : 2) : (dy<0 ? 0 : 1);
    const double cosine=((quadrant%2) ? std::abs(dx) : std::abs(dy))/std::hypot(double(dx),double(dy));
    constexpr std::array thresholds{1.f,.980785f,.923880f,.831470f,.707107f,.555570f,.382683f,.195090f};
    int slot=0;
    while (slot<count/2 && cosine<=thresholds[slot*(eight ? 2 : 1)]) ++slot;
    return (quadrant*(count/4)+slot/2)%count;
}

struct SpellFlight {
    SpellPixel destination{}, velocity{}, previousDistance{10000,10000};
    std::uint32_t target=0;
    bool expiresWithoutTarget=false;
    std::optional<SpellOrbit> orbit;
    int direction=0;

    SpellFlight(SpellPixel source,SpellPixel end,std::uint32_t actor,bool timeout)
        :destination(end),target(actor),expiresWithoutTarget(timeout)
    {
        const float dx=end[0]-source[0], dy=end[1]-source[1];
        const int x=std::max(1,std::abs(int(dx))), y=std::max(1,std::abs(int(dy)));
        const double factor=x>y ? 500.0/x : double(float(500.0/y));
        velocity={float(std::trunc(dx*factor)),float(std::trunc(dy*factor))};
    }
};

enum class MonsterEffectPose { attack, struck, death, idle, walk, burrow, skeleton };

struct SpellEffect {
    EffectResource resource;
    SpellCell cell{};
    SpellPixel position{};
    unsigned delay=0, elapsed=0, age=0, lightClock=0, remaining=0;
    int frame=0, offset=0, lightStep=0, shrink=0, fade=0;
    std::int64_t soundVoice=0;
    std::uint32_t follow=0;
    std::uint32_t poseActor=0;
    MonsterEffectPose monsterPose=MonsterEffectPose::attack;
    bool repeat=false, wave=false, baseWalkable=false, light=false, tail=false, finished=false;
    bool timedResidue=false;
    bool monsterEffect=false;
    bool waitForTarget=false;
    bool monsterActionOverlay=false;
    std::optional<SpellFlight> flight;
    std::optional<StagedSpell> staged;

    SceneLight illumination(int cameraX,int cameraY) const {
        const auto color=[&](const auto& rgb) -> SceneLight::Rgb {
            return {SpellLight::channel(rgb[0],fade),SpellLight::channel(rgb[1],fade),SpellLight::channel(rgb[2],fade)};
        };
        return {int(position[0])+24-cameraX,int(position[1])+16-cameraY,
            resource.inner_radius+lightStep-shrink,color(resource.inner_color),
            resource.outer_radius+lightStep-shrink,color(resource.outer_color)};
    }
    bool drawsSprite() const {
        return !delay && !finished && !(flight && flight->orbit) && resource.id!=int(MagicEffectId::lightning_trail) &&
            (!tail || wave || monsterEffect);
    }
};

struct SpellGroundMark { SpellCell cell; int frame; unsigned age=0; };

class SpellEffects {
public:
    std::vector<SpellEffect> effects;
    std::vector<SpellGroundMark> marks;
    SpellParticlePool particles;
    MapEvents mapEvents;
    std::array<std::array<int,2>,150> archiveOffsets{};

    template<class Sound>
    void harvest(const EffectCatalog& catalog,SpellCell cell,Sound sound) {
        for (auto& [id,tile]:mapEvents.ground) {
            if (tile.cell!=cell || tile.hidden || tile.frame<230 || tile.frame>234) continue;
            tile.frame=std::min(tile.frame+1,234);
            break;
        }
        append(catalog,335,cell,0,0,false,sound);
    }

    template<class Sound>
    void followingImpact(const EffectCatalog& catalog,int id,SpellCell cell,std::uint32_t actor,Sound sound) {
        if (auto* effect=append(catalog,id,cell,0,0,false,sound)) effect->follow=actor;
    }

    void groundImpact(const EffectCatalog& catalog,int id,SpellCell cell) {
        append(catalog,id,cell,0,0,false,[](int,SpellCell) { return 0; });
    }

    template<class Sound>
    void monsterAttack(const EffectCatalog& catalog,int id,SpellCell source,SpellCell target,std::uint32_t actor,
                       int offset,unsigned delay,bool projectile,bool light,Sound sound,bool waitForTarget=false,
                       EffectCatalog::Section section=EffectCatalog::Section::magic)
    {
        if (auto* effect=append(catalog,id,source,offset,delay,projectile,sound,section)) {
            effect->monsterEffect=true;
            effect->waitForTarget=waitForTarget;
            effect->light=light && effect->resource.outer_radius>0;
            effect->repeat=projectile;
            if (projectile) effect->flight.emplace(effect->position,spell_pixel(target),actor,false);
            else effect->follow=actor;
        }
    }

    bool monsterOverlay(const EffectCatalog& catalog,int id,SpellCell cell,int offset,
                        std::uint32_t poseActor,std::uint32_t follow,MonsterEffectPose pose=MonsterEffectPose::attack,
                        bool actionOverlay=false)
    {
        const auto* resource=find(catalog,EffectCatalog::Section::spell,id);
        if (!resource || effects.size()>=512) return false;
        auto& e=effects.emplace_back();
        e.resource=*resource;
        e.cell=cell;
        e.position=spell_pixel(cell);
        e.frame=int(resource->first)+offset;
        e.offset=offset;
        e.poseActor=poseActor;
        e.monsterPose=pose;
        e.monsterActionOverlay=actionOverlay;
        e.follow=follow;
        e.monsterEffect=true;
        return true;
    }

    template<class Sound,class Walkable>
    void monsterWave(const EffectCatalog& catalog,int id,SpellCell source,SpellCell target,Sound sound,Walkable walkable)
    {
        wave(catalog,id,source,target,0,sound,walkable);
    }

    void forgetActor(std::uint32_t id,std::optional<SpellPixel> position={},std::optional<SpellCell> lastCell={})
    {
        for (auto& effect:effects) {
            if (effect.poseActor==id) effect.finished=true;
            if (effect.follow==id) {
                if (effect.monsterEffect && lastCell) {
                    effect.cell=*lastCell;
                    effect.position=spell_pixel(*lastCell);
                } else if (position) effect.position=*position;
                effect.follow=0;
            }
            if (effect.flight && effect.flight->target==id) {
                if (effect.monsterEffect && lastCell) effect.flight->destination=spell_pixel(*lastCell);
                effect.flight->target=0;
            }
        }
    }

    template<class Track,class Sound,class Walkable>
    void play(const EffectCatalog& catalog,const SpellReply& reply,SpellCell source,unsigned facing,
        Track track,Sound sound,Walkable walkable)
    {
        using S=SkillId;
        const auto skill=static_cast<S>(reply.skill);
        if (reply.multiple) {
            if (skill==S::original_113) for (const auto id:reply.targets)
                if (const auto target=track(id)) spawn(catalog,reply.skill,source,spell_cell(*target),id,sound);
            return;
        }
        const SpellCell end{reply.x,reply.y};
        const auto id=reply.targets.empty() ? 0 : reply.targets.front();
        switch (skill) {
        case S::original_104: case S::spacemove: case S::sinsu: case S::firewind:
        case S::cloak: case S::skelleton: case S::original_78: case S::original_64:
        case S::original_105: case S::original_121: case S::earthfire: case S::holyshield: case S::original_63:
            return;
        case S::fire: case S::original_53: case S::original_110:
            wave(catalog,reply.skill,source,end,facing,sound,walkable);
            return;
        default: break;
        }
        spawn(catalog,reply.skill,source,end,id && track(id) ? id : 0,sound);
        if (skill==S::shootlighten) wave(catalog,int(MagicEffectId::lightning_trail),source,end,facing,sound,walkable);
    }

    template<class Track,class Sound,class PositionSound,class Random>
    void advance(const EffectCatalog& catalog,unsigned ms,SpellCell camera,Track track,Sound sound,PositionSound positionSound,Random& random)
    {
        if (!ms) return;
        for (auto& mark:marks) mark.age+=ms;
        std::erase_if(marks,[](const auto& m){return m.age>2000;});
        std::vector<std::pair<int,SpellCell>> embers;
        for (auto& e:effects) {
            e.age+=ms;
            if ((!e.wave && !e.staged && e.age>60000) || e.finished) { e.finished=true; continue; }
            if (e.follow) {
                if (const auto pos=track(e.follow)) e.position=*pos;
                else if (!e.waitForTarget) e.follow=0;
            }
            if (e.poseActor) continue;
            unsigned activeMs=ms;
            if (e.delay) {
                if (ms<e.delay) { e.delay-=ms; continue; }
                activeMs=e.flight && !e.monsterEffect ? ms : ms-e.delay;
                e.delay=0;
            }
            if (e.monsterEffect && e.flight) {
                if (advanceFlight(e,catalog,activeMs,camera,track,sound,random)) {
                    if (!e.finished) e.elapsed+=activeMs;
                    continue;
                }
            }
            if (e.monsterEffect) {
                advanceLight(e,activeMs);
                if (e.finished) continue;
            }
            if (e.timedResidue) {
                e.remaining=e.remaining>ms ? e.remaining-ms : 0;
                if (!e.remaining && (e.fade+=5)>=255) { e.finished=true; continue; }
            }
            if (e.staged && !e.tail) {
                auto& staged=*e.staged;
                const auto change=staged.advance(activeMs);
                if (change==StagedSpell::Change::load_explosion) {
                    const auto* hit=find(catalog,EffectCatalog::Section::explosion,e.resource.id);
                    if (!hit) {e.finished=true; continue;}
                    e.resource=*hit;
                    staged.load_explosion({int(hit->first),int(hit->end),hit->interval});
                }
                e.frame=staged.frame;
                e.position=spell_pixel(e.cell);
                for (int i=0;i<2;++i) e.position[i]+=staged.offset[i];
                if (change==StagedSpell::Change::finished) {e.tail=true; --e.frame;}
            }
            if (!e.wave && !e.staged && !e.tail) advanceFrame(e,activeMs);
            if (!e.monsterEffect && e.flight && advanceFlight(e,catalog,activeMs,camera,track,sound,random)) continue;
            if (e.flight && e.soundVoice) positionSound(e.soundVoice,e.resource.id*10+10001,e.position);
            if (!e.monsterEffect) advanceLight(e,activeMs);
            if (e.wave && !e.tail) advanceWave(e,activeMs,camera,random,embers);
            if (e.tail && !e.light) e.finished=true;
        }
        std::erase_if(effects,[](const auto& e){return e.finished;});
        for (const auto& [id,cell]:embers)
            if (auto* effect=append(catalog,id,cell,0,0,false,sound)) {
                effect->repeat=true;
                effect->remaining=10000;
                effect->timedResidue=true;
            }
        mapEvents.advance(ms,[&](SpellCell cell) {
            particles.emit_ice({float(cell[0]*48+24-camera[0]),float(cell[1]*32-camera[1])},random);
        });
        particles.advance(int(ms),random);
    }

private:
    static const EffectResource* find(const EffectCatalog& catalog,EffectCatalog::Section section,int id,bool particle=false) {
        const auto* r=catalog.find(section,std::uint16_t(id));
        if (!r || !spell_archive(r->library) || r->blend>2 || r->end>0x7ffffeffu ||
            (!particle && (r->end<=r->first || !r->interval))) return nullptr;
        return r;
    }

    template<class Sound>
    SpellEffect* append(const EffectCatalog& catalog,int id,SpellCell cell,int offset,unsigned delay,bool flight,Sound sound,
                        EffectCatalog::Section section=EffectCatalog::Section::magic) {
        const bool particle=id==150 || id==int(MagicEffectId::lightning_trail);
        const auto* resource=find(catalog,section,id,particle);
        if (!resource || effects.size()>=512) return nullptr;
        auto& e=effects.emplace_back();
        e.resource=*resource; e.cell=cell; e.position=spell_pixel(cell);
        e.frame=int(resource->first)+offset; e.offset=offset; e.delay=delay;
        e.soundVoice=sound(id*10+(flight ? 10001 : 10002),cell);
        return &e;
    }

    template<class Sound>
    void spawn(const EffectCatalog& catalog,int id,SpellCell source,SpellCell end,std::uint32_t target,Sound sound) {
        const auto first=spell_pixel(source), last=spell_pixel(end);
        const float dx=last[0]-first[0], dy=last[1]-first[1];
        const int direction=spell_direction(dx,dy);
        if (id==150) {
            if (source==end) return;
            spawn(catalog,1,source,end,0,sound);
            for (int i=0;i<2;++i) if (auto* e=append(catalog,id,source,10*direction,30*i,true,sound)) {
                e->repeat=true;
                e->light=e->resource.outer_radius>0;
                e->flight.emplace(first,last,0,false);
                e->flight->direction=direction;
                e->flight->orbit=SpellOrbit{0,i==1};
            }
            return;
        }
        if (id==73 || id==74) {
            if (auto* e=append(catalog,id,end,0,0,false,sound)) {
                e->cell=id==73 ? source : end;
                e->staged.emplace(static_cast<SkillId>(id),direction>>1,
                    StagedSpell::Frames{int(e->resource.first),int(e->resource.end),e->resource.interval});
                e->position=spell_pixel(e->cell);
                for (int i=0;i<2;++i) e->position[i]+=e->staged->offset[i];
                e->light=e->resource.outer_radius>0;
            }
            return;
        }
        const auto rules=MagicRules::for_skill(static_cast<SkillId>(id));
        if (rules.reject_same_cell && source==end) return;
        if (!rules.track_target) target=0;
        const int offset=10*(rules.direction==MagicDirection::fixed ? 0 :
            rules.direction==MagicDirection::eight ? spell_direction(dx,dy,true) : direction);
        if (auto* e=append(catalog,id,rules.at_target ? (rules.use_source_as_target ? source : end) : source,
            offset,0,!rules.at_target,sound)) {
            e->repeat=rules.repeat;
            e->light=rules.light && e->resource.outer_radius>0;
            if (rules.at_target) e->follow=target;
            else e->flight.emplace(first,last,target,rules.untargeted_timeout);
        }
    }

    template<class Sound,class Walkable>
    void wave(const EffectCatalog& catalog,int id,SpellCell source,SpellCell end,unsigned facing,Sound sound,Walkable walkable) {
        const auto step=[](SpellCell a,SpellCell b) {
            const int x=b[0]-a[0],y=b[1]-a[1];
            return SpellCell{std::abs(y)>2 && std::abs(x)<=1 ? 0 : (x>0)-(x<0),
                x || y ? (std::abs(x)>2 && std::abs(y)<=1 ? 0 : (y>0)-(y<0)) : 1};
        };
        const auto segment=[&](SpellCell cell,unsigned delay) {
            if (auto* e=append(catalog,id,cell,0,delay,false,sound)) {
                e->wave=true;
                e->baseWalkable=walkable(cell);
                e->light=e->resource.outer_radius>0;
            }
        };
        if (id==110 && facing<8) {
            constexpr std::array<SpellCell,8> headings{{{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}}};
            constexpr std::array<std::array<int,3>,8> rays{{{0,7,1},{1,0,2},{1,2,3},{3,2,4},{4,3,5},{5,4,6},{6,7,5},{7,0,6}}};
            const SpellCell start{source[0]+headings[facing][0],source[1]+headings[facing][1]};
            for (int dir:rays[facing]) {
                const auto d=step(start,{source[0]+5*headings[dir][0],source[1]+5*headings[dir][1]});
                for (int i=0;i<5;++i) segment({start[0]+d[0]*i,start[1]+d[1]*i},150*i);
            }
        } else {
            const auto d=step(source,end);
            const int interval=id==int(MagicEffectId::lightning_trail) ? 30 : 150;
            for (int i=0;i<9;++i) segment({source[0]+d[0]*(i+1),source[1]+d[1]*(i+1)},interval*i);
        }
    }

    static void advanceFrame(SpellEffect& e,unsigned ms) {
        if ((e.elapsed+=ms)<=e.resource.interval) return;
        e.elapsed=0;
        const int end=int(e.resource.end)+e.offset;
        if (++e.frame<end) return;
        e.frame=e.repeat ? int(e.resource.first)+e.offset : end-1;
        e.tail=!e.repeat;
    }

    template<class Track,class Sound,class Random>
    bool advanceFlight(SpellEffect& e,const EffectCatalog& catalog,unsigned ms,SpellCell camera,Track track,Sound sound,Random& random)
    {
        auto& f=*e.flight;
        const auto followed=f.target ? track(f.target) : std::optional<SpellPixel>{};
        if (followed) {
            f.destination=*followed;
            const float dx=f.destination[0]-std::trunc(e.position[0]),dy=f.destination[1]-std::trunc(e.position[1]);
            const int scale=500/std::max(1,int(std::max(std::abs(dx),std::abs(dy))));
            f.velocity={dx*scale,dy*scale};
        } else if (!e.waitForTarget) f.target=0;
        SpellPixel distance{};
        for (int i=0;i<2;++i) {
            e.position[i]=float(double(f.velocity[i])*double(0.0011111111380159855f)*ms+e.position[i]);
            distance[i]=std::abs(f.destination[i]-std::trunc(e.position[i]));
        }
        const bool timeout=!followed && f.expiresWithoutTarget;
        if (timeout && e.age>=3000) {e.finished=true;return true;}
        const bool closer=distance[0]<f.previousDistance[0] || distance[1]<f.previousDistance[1];
        if (!timeout && ((ms && !closer) || (distance[0]<=15 && distance[1]<=15))) {
            const int id=e.resource.id;
            const auto* hit=find(catalog,EffectCatalog::Section::explosion,id,f.orbit.has_value());
            if (!hit || f.orbit) {e.finished=true;return true;}
            e.resource=*hit; e.frame=int(hit->first); e.offset=0;
            e.cell=spell_cell(f.destination); e.position=f.destination;
            e.follow=f.target;
            e.repeat=false;
            e.flight.reset();
            if (followed) sound(id*10+10002,e.cell);
            return true;
        }
        f.previousDistance=distance;
        if (f.orbit && ms) {
            const auto at=f.orbit->advance(e.position[0],e.position[1],f.direction);
            const auto& offset=archiveOffsets[e.resource.library];
            const auto point=SpellOrbit::emitter(at,f.direction,offset[0],offset[1]);
            particles.emit({float(point[0]-camera[0]),float(point[1]-camera[1])},random);
        }
        return false;
    }

    static void advanceLight(SpellEffect& e,unsigned ms) {
        if (!e.light) return;
        e.lightClock+=ms;
        const auto& r=e.resource;
        const bool monsterFade=e.monsterEffect && (r.id==538 || r.id==539);
        if (e.tail && (e.wave || monsterFade || (!e.monsterEffect && SpellLight::fades(static_cast<SkillId>(r.id))))) {
            e.lightStep=0;
            e.fade+=2;
            int brightest=0;
            for (const auto& rgb:{r.inner_color,r.outer_color})
                for (const auto c:rgb) brightest=std::max(brightest,!monsterFade && c<2 ? c+256 : int(c));
            if (brightest<=e.fade+2) e.finished=true;
        } else if (e.lightClock>200) {
            if (e.tail) {
                if (r.inner_radius-e.shrink<=1 || r.outer_radius<=r.inner_radius) { e.finished=true; return; }
                e.lightStep=0; ++e.shrink; e.lightClock=150;
            } else {
                e.lightClock=0;
                e.lightStep=(e.lightStep+1)%(r.light_steps+1);
            }
        }
    }

    template<class Random>
    void advanceWave(SpellEffect& e,unsigned ms,SpellCell camera,Random& random,std::vector<std::pair<int,SpellCell>>& embers)
    {
        const int id=e.resource.id;
        const bool fire=id==9, ice=id==53;
        const auto interval=fire && e.baseWalkable && e.frame>=1925 ? 80 : e.resource.interval;
        if ((e.elapsed+=ms)<=static_cast<unsigned>(interval)) return;
        e.elapsed=0;
        const int previous=e.frame++;
        auto pixel=spell_pixel(e.cell);
        pixel[0]+=24-camera[0];
        pixel[1]-=camera[1];
        if (id==int(MagicEffectId::lightning_trail) && previous==int(e.resource.first)) {
            pixel[1]-=16;
            particles.emit_lightning(pixel,random);
        }
        const int end=int(e.resource.end)+e.offset;
        if (fire || ice) {
            if (fire && e.baseWalkable && e.frame<1915) ++e.frame;
            const int residue=end-(fire ? 48 : 26);
            if (e.baseWalkable && previous<residue && e.frame>=residue) {
                if (fire) embers.emplace_back(random.between(330,334),e.cell);
                if (marks.size()<512) marks.push_back({e.cell,fire ? 220 : 260+random.between(0,1)});
            }
            if (ice && e.baseWalkable && e.frame<end-26) particles.emit_ice(pixel,random);
            if (e.frame>=end-(fire ? 30 : 20)) e.frame=end;
        }
        if (e.frame>=end) { e.tail=true; --e.frame; }
    }
};
}
