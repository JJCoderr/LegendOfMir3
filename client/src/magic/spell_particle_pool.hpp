#pragma once
#include "spell_particle.hpp"
#include <optional>

namespace mir3 {
// 421BDA / 421BF7分别以1000初始化两个池。按第一个空槽分配，保留绘制顺序。
class SpellParticlePool {
public:
    static constexpr unsigned capacity=1000;
    using Slots=std::array<std::optional<SpellParticle>,capacity>;
    Slots sparks{},glows{},fragments{};

    template<class Random>
    void emit_lightning(std::array<float,2> screen_position,Random& random) {
        for (auto& slot:glows) if (!slot) {slot=SpellParticle::create(SpellParticle::Kind::lightning_glow,screen_position,random);break;}
        unsigned count=0;
        for (auto& slot:fragments) if (!slot) {
            slot=SpellParticle::create(SpellParticle::Kind::lightning_fragment,screen_position,random);
            if (++count==21) break;
        }
    }

    template<class Random>
    void emit(std::array<float,2> screen_position,Random& random) {
        const auto spawn=[&](Slots& slots,SpellParticle::Kind kind,unsigned count) {
            for (auto& slot:slots) {
                if (slot) continue;
                slot=SpellParticle::create(kind,screen_position,random);
                if (--count==0) break;
            }
        };
        spawn(sparks,SpellParticle::Kind::spark,5);
        spawn(glows,SpellParticle::Kind::glow,1);
    }

    template<class Random>
    void advance(int milliseconds,Random& random) {
        // 4246AB/4246FB：先完整推进火星池，再推进光点池。
        for (auto* slots:{&sparks,&glows,&fragments})
            for (auto& slot:*slots)
                if (slot && !slot->advance(milliseconds,random)) slot.reset();
    }

    void clear() {
        for (auto* slots:{&sparks,&glows,&fragments})
            for (auto& slot:*slots) slot.reset();
    }
    unsigned count() const {
        unsigned result=0;
        for (const auto* slots:{&sparks,&glows,&fragments})
            for (const auto& slot:*slots) result+=slot.has_value();
        return result;
    }
};
}
