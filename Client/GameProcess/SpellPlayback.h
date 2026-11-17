#pragma once
#include "EffectCatalog.h"
#include "MagicRules.h"
#include "SpellFlight.h"
#include "SpellLight.h"
#include <optional>

namespace mir3 {
// Common 44B7E0 playback. Ground waves, 73/74 and 150 subclasses have separate advancement.
struct SpellPlayback {
    static constexpr std::uint32_t MaximumAge=60000,UntargetedAge=3000,LightInterval=200,LightTailCarry=150;
    enum class Change { none,impact,finished };
    const EffectResource* resource=nullptr;
    SkillId skill=SkillId::none;
    MagicRules rules;
    SpellFlight flight;
    SpellFlight::Point position={};
    std::uint32_t age=0,elapsed=0,delay=0,light_clock=0;
    int frame=0,frame_offset=0,light_phase=0,light_shrink=0,light_fade=0;
    bool particle_only=false;
    bool flying=false,bound=false,one_shot=true,light=false,tail=false,finished=false;
    void begin(SkillId id,const EffectResource& initial,SpellFlight::Point source,
        SpellFlight::Point target,int direction,bool target_bound,std::uint32_t wait=0) {
        *this=SpellPlayback{};skill=id;resource=&initial;rules=MagicRules::for_skill(id);
        frame_offset=direction*10;frame=int(initial.first)+frame_offset;delay=wait;
        bound=target_bound && rules.track_target;flying=!rules.at_target;one_shot=!rules.repeat;
        light=rules.light && initial.outer_radius!=0;
        if(rules.use_source_as_target) target=source;
        flight.begin(source,target);position=rules.at_target?target:source;
    }
    Change stop() {finished=true;return Change::finished;}
    Change advance(std::uint32_t ms,std::optional<SpellFlight::Point> existing_target,const EffectCatalog& catalog) {
        if(finished) return Change::finished;
        age+=ms;elapsed+=ms;
        if(age>MaximumAge) return stop();
        if(bound && !existing_target) bound=false; // Never reacquire a replacement with the same network id.
        if(bound) {
            flight.destination=*existing_target;
            if(!flying && rules.at_target) position=*existing_target;
        }
        if(delay && age<delay) {elapsed=0;return Change::none;}
        delay=0;
        if(!tail && elapsed>resource->interval) {
            elapsed=0;
            if(++frame>=int(resource->end)+frame_offset) {
                if(one_shot) {frame=int(resource->end)+frame_offset-1;tail=true;}
                else frame=int(resource->first)+frame_offset;
            }
        }
        if(flying) {
            const bool hit=flight.advance(ms,bound);position=flight.position;
            if(!bound && rules.untargeted_timeout) {
                if(age>=UntargetedAge) return stop();
            } else if(hit) {
                const auto* explosion=catalog.find(EffectCatalog::Section::explosion,std::uint16_t(skill));
                if(!explosion || (!particle_only && (explosion->end<=explosion->first || !explosion->interval))) return stop();
                resource=explosion;frame=int(explosion->first);frame_offset=0;
                flying=false;one_shot=true;tail=false;position=flight.destination;
                rules.at_target=true;return Change::impact; // Preserve frame/light clocks and target binding.
            }
        }
        return advance_light(ms);
    }
    Change advance_light(std::uint32_t ms,bool ground_wave=false) {
        if(tail && !light) return stop();
        if(light) {
            light_clock+=ms;
            if(tail && (ground_wave || SpellLight::fades(skill))) {
                light_phase=0;if(ms) light_fade+=2;
                int strongest=0;
                for(const auto& color:{resource->inner_color,resource->outer_color})
                    for(auto value:color) strongest=(std::max)(strongest,value<2?int(value)+256:int(value));
                if(strongest<=light_fade+2) return stop();
            } else if(light_clock>LightInterval) {
                if(tail) {
                    if(int(resource->inner_radius)-light_shrink<=1 || resource->outer_radius<=resource->inner_radius) return stop();
                    light_phase=0;++light_shrink;light_clock=LightTailCarry;
                } else {light_clock=0;light_phase=(light_phase+1)%(int(resource->light_steps)+1);}
            }
        }
        return Change::none;
    }
};
}
