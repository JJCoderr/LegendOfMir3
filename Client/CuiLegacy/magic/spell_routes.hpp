#pragma once
#include "skill_ids.hpp"
#include <array>
#include <cstdint>
#include <initializer_list>

namespace mir3 {
enum class SpellAction : int { none=-1, directed=2, area=3, special=18, rush=23 };
enum class DirectSpell : std::uint8_t { none, rapid, predicted, fire_sword, skill35, skill34 };
struct SpellRoute {
    SpellAction action=SpellAction::none;
    DirectSpell direct=DirectSpell::none;
    bool self_target=false;
    bool bypass_mana=false;
    bool directional_effect=false;
    bool restrict_followup=false;
    int action_limit=10;
    std::uint32_t action_interval=0;
};



struct SpellRoutes {
    static constexpr int maximum_id=150;
    static constexpr auto make() {
        std::array<SpellRoute,maximum_id+1> routes{};
        const auto actions=[&](std::initializer_list<SkillId> ids,SpellAction action) {
            for (auto id:ids) routes[static_cast<int>(id)].action=action;
        };
        actions({SkillId::fireball,SkillId::fireball2,SkillId::fire,SkillId::shootlighten,SkillId::firecharm,SkillId::hangmajinbub,SkillId::dejiwonho,SkillId::bigcloak,SkillId::sinsu,SkillId::original_37,SkillId::original_38,SkillId::original_39,SkillId::original_40,SkillId::original_41,SkillId::original_53,SkillId::original_67,
            SkillId::original_85,SkillId::original_86,SkillId::original_87,SkillId::original_88,SkillId::original_89,SkillId::original_90,SkillId::original_91,SkillId::original_92,SkillId::original_93,SkillId::original_94,SkillId::original_95,SkillId::original_96,SkillId::original_97,SkillId::original_98,SkillId::original_99,SkillId::original_100,SkillId::original_101,SkillId::original_108,SkillId::original_110,SkillId::original_113,SkillId::original_150},SpellAction::directed);
        actions({SkillId::healling,SkillId::amyounsul,SkillId::firewind,SkillId::lightening,SkillId::holyshield,SkillId::skelleton,SkillId::cloak,SkillId::tamming,SkillId::spacemove,SkillId::earthfire,SkillId::fireboom,SkillId::lightflower,SkillId::showhp,SkillId::bighealling,SkillId::shield,SkillId::killundead,SkillId::snowwind,SkillId::original_47,SkillId::original_48,SkillId::original_63,SkillId::original_64,SkillId::original_66,
            SkillId::original_72,SkillId::original_73,SkillId::original_74,SkillId::original_77,SkillId::original_78,SkillId::original_104,SkillId::original_105,SkillId::original_107,SkillId::original_111,SkillId::original_120,SkillId::original_121,SkillId::original_122,SkillId::original_123},SpellAction::area);
        actions({SkillId::mootebo},SpellAction::rush);
        actions({SkillId::spacemove3},SpellAction::special);
        routes[static_cast<int>(SkillId::showhp)].action_limit=20;
        routes[static_cast<int>(SkillId::lightening)].action_limit=13;
        routes[static_cast<int>(SkillId::original_78)].action_limit=14;
        routes[static_cast<int>(SkillId::original_104)].action_interval=40;
        for (auto id:{SkillId::ergum,SkillId::banwol,SkillId::original_103}) routes[static_cast<int>(id)].direct=DirectSpell::rapid;
        for (auto id:{SkillId::original_102,SkillId::original_106,SkillId::original_112}) routes[static_cast<int>(id)].direct=DirectSpell::predicted;
        routes[static_cast<int>(SkillId::firesword)].direct=DirectSpell::fire_sword;
        routes[static_cast<int>(SkillId::spacemove2)].direct=DirectSpell::skill35;
        routes[static_cast<int>(SkillId::randswing)].direct=DirectSpell::skill34;
        for (auto id:{SkillId::shield,SkillId::original_122}) routes[static_cast<int>(id)].self_target=true;
        for (auto id:{SkillId::banwol,SkillId::original_103}) routes[static_cast<int>(id)].bypass_mana=true;
        for (auto id:{SkillId::fireball,SkillId::fireball2,SkillId::fire,SkillId::shootlighten,SkillId::hangmajinbub,SkillId::dejiwonho,SkillId::firecharm,SkillId::sinsu,SkillId::bigcloak,SkillId::original_53,SkillId::original_39,SkillId::original_40,SkillId::original_41,SkillId::original_37,SkillId::original_38,SkillId::original_94,SkillId::original_89,SkillId::original_90,SkillId::original_91,SkillId::original_92,SkillId::original_93,SkillId::original_67,
            SkillId::original_85,SkillId::original_86,SkillId::original_87,SkillId::original_88,SkillId::original_95,SkillId::original_96,SkillId::original_97,SkillId::original_98,SkillId::original_99,SkillId::original_100,SkillId::original_101,SkillId::original_150,SkillId::original_110,SkillId::original_113,SkillId::original_108}) routes[static_cast<int>(id)].directional_effect=true;
        for (auto id:{SkillId::fireball,SkillId::fireball2,SkillId::amyounsul,SkillId::fire,SkillId::shootlighten,SkillId::ergum,SkillId::firecharm,SkillId::earthfire,SkillId::lightflower,SkillId::banwol,SkillId::firesword,SkillId::mootebo,SkillId::killundead,SkillId::snowwind,SkillId::original_37,SkillId::original_38,SkillId::original_39,SkillId::original_40,SkillId::original_41,SkillId::original_53,SkillId::original_64,
            SkillId::original_72,SkillId::original_73,SkillId::original_74,SkillId::original_103,SkillId::original_107,SkillId::original_110,SkillId::original_111,SkillId::original_120,SkillId::original_123,SkillId::original_150}) routes[static_cast<int>(id)].restrict_followup=true;
        return routes;
    }
    static const SpellRoute& get(int id) {
        static constexpr auto routes=make();
        return routes[id>=0 && id<=maximum_id?id:0];
    }
    static SpellAction remote_action(int id) {

        return id!=static_cast<int>(SkillId::sinsu) && get(id).action==SpellAction::directed?
            SpellAction::directed:SpellAction::area;
    }
};
}
