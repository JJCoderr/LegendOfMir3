#include "hud_view.hpp"
#include "protocol/receive_message.hpp"
#include "protocol/message_ids.hpp"
#include <cstdlib>

using namespace godot;

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
