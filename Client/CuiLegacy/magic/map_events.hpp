#pragma once

#include "resources/effect_catalog.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <map>

namespace mir3 {
struct MapEventSprite {
    EffectResource resource;
    std::array<int,2> cell;
    int frame=0;
    unsigned elapsed=0;
    int remaining=-1;
    std::uint8_t residueTint=255;
    bool layered=false, ending=false, walkable=false;
};

struct MapEventGround {
    std::array<int,2> cell;
    int frame;
    unsigned age=0, lifetime=65535;
    std::uint8_t opacity=255;
    bool hidden=false;
};

class MapEvents {
public:
    std::map<std::uint32_t,MapEventSprite> sprites;
    std::map<std::uint32_t,MapEventGround> ground;
    std::map<std::array<int,2>,MapEventGround> iceMarks;

    bool active() const { return !sprites.empty() || !ground.empty() || !iceMarks.empty(); }

    template<class Random>
    void show(const EffectCatalog& catalog,std::uint32_t id,unsigned kind,std::array<int,2> cell,bool walkable,Random& random)
    {
        if (sprites.contains(id) || ground.contains(id)) return;
        if (kind==3 || kind==6) {
            if (ground.size()<1024) ground.emplace(id,MapEventGround{cell,kind==3 ? 230 : 210});
            return;
        }
        if ((kind!=4 && kind!=5 && kind!=9) || sprites.size()>=512) return;
        const bool layered=kind==9;
        const auto* resource=find(catalog,layered ? 140 : kind==4 ? 16 : 22,layered ? 137 : 81);
        if (!resource) return;
        const int frame=int(resource->first)+(layered ? 0 : random.between(1,8)%int(resource->end-resource->first));
        sprites.emplace(id,MapEventSprite{*resource,cell,frame,0,-1,255,layered,false,walkable});
    }

    template<class Random>
    void hide(const EffectCatalog& catalog,std::uint32_t id,Random& random)
    {
        if (auto tile=ground.find(id);tile!=ground.end() && tile->second.frame!=220) tile->second.hidden=true;
        const auto found=sprites.find(id);
        if (found==sprites.end()) return;
        auto& event=found->second;
        if (event.layered) { event.ending=true; return; }
        if (event.remaining>=0) return;
        if (event.resource.id==22) {
            const int duration=random.between(20000,30000);
            if (const auto* resource=find(catalog,random.between(330,334),81)) {
                event.resource=*resource;
                event.frame=int(resource->first);
                event.elapsed=0;
                event.remaining=duration;
                ground.emplace(id,MapEventGround{event.cell,220,0,unsigned(duration+800),180});
                return;
            }
        }
        sprites.erase(found);
    }

    template<class Emit>
    void advance(unsigned ms,Emit emitIce)
    {
        if (!ms) return;
        const auto expire=[&](auto& tiles) {
            for (auto& [id,tile]:tiles) tile.age+=ms;
            std::erase_if(tiles,[](const auto& entry) { return entry.second.hidden || entry.second.age>entry.second.lifetime; });
        };
        expire(ground);
        expire(iceMarks);
        for (auto it=sprites.begin();it!=sprites.end();) {
            if (!advanceSprite(it->second,ms,emitIce)) it=sprites.erase(it);
            else ++it;
        }
    }

private:
    static const EffectResource* find(const EffectCatalog& catalog,std::uint16_t id,std::uint16_t library)
    {
        const auto* resource=catalog.find(EffectCatalog::Section::magic,id);
        return resource && resource->library==library && resource->end>resource->first &&
            resource->end<=0x7fffffffu && resource->interval ? resource : nullptr;
    }

    template<class Emit>
    bool advanceSprite(MapEventSprite& event,unsigned ms,Emit emitIce)
    {
        if (event.layered && event.walkable) {
            if (event.frame>=101 && event.frame<106) emitIce(event.cell);
            if (event.frame>=107) iceMarks.insert_or_assign(event.cell,MapEventGround{event.cell,260,0,3000,180});
        }
        if (event.remaining>=0) {
            event.remaining=std::max(0,event.remaining-int(ms));
            if (!event.remaining) {
                if (event.residueTint<=5) return false;
                event.residueTint-=5;
            }
        }
        if ((event.elapsed+=ms)<=event.resource.interval) return true;
        event.elapsed=0;
        ++event.frame;
        if (event.layered) {
            if (!event.ending && event.frame>=99) event.frame=100;
            return !event.ending || event.frame<110;
        }
        if (event.frame>=int(event.resource.end)) event.frame=int(event.resource.first);
        return true;
    }
};
}
