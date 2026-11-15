#include "protocol/message_ids.hpp"
#include "hud/selection_pixels.hpp"
#include "chat/scene_chat.hpp"
#include "protocol/receive_message.hpp"
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/os.hpp>
#include "hud/sound_position.hpp"
#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/classes/audio_effect_panner.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/audio_stream_wav.hpp>
#include <godot_cpp/classes/audio_stream_mp3.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include "hud_view.hpp"
#include <climits>
#include "hud/portrait_fac.hpp"
#include "guild/guild_members.hpp"
#include <godot_cpp/classes/config_file.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/shader.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <chrono>
#include <godot_cpp/classes/project_settings.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <godot_cpp/classes/style_box_empty.hpp>
#include "status/status_layout.hpp"
#include <godot_cpp/classes/input_event_key.hpp>
#include <cstdio>
#include <cstdlib>
#include <charconv>
#include <iterator>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
using namespace mir3;
namespace {
constexpr std::array group_button_frames{1221,1362,1365,1368,1370};
constexpr std::array<std::array<int,2>,5> group_button_offsets{{
    {217,200},{38,192},{98,192},{158,192},{23,47}
}};
constexpr std::array group_button_hints{1052,1600,1601,1602,-1};

Ref<ImageTexture> texture(const std::vector<std::uint8_t>& rgba,int width,int height) {
    if (rgba.empty()) return {};
    PackedByteArray bytes; bytes.resize(rgba.size());
    std::copy(rgba.begin(),rgba.end(),bytes.ptrw());
    return ImageTexture::create_from_image(Image::create_from_data(width,height,false,Image::FORMAT_RGBA8,bytes));
}
Rect2 rect(mir3::HudLayout::Rect r) { return Rect2(r[0],r[1],r[2],r[3]); }
Vector2 point(mir3::HudLayout::Point p) { return Vector2(p[0],p[1]); }

Vector2 spell_initial_velocity(Vector2 delta) {
    // 4497CD–44984C：横轴占优保留x87商，纵轴占优先把商存为float。
    const auto x=std::max(1,std::abs(int(delta.x))),y=std::max(1,std::abs(int(delta.y)));
    const double factor=x>y?500.0/x:double(float(500.0/y));
    return Vector2(std::trunc(delta.x*factor),std::trunc(delta.y*factor));
}

std::uint64_t ticks_msec() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}
}
void Mir3HudView::_bind_methods() {
#define MIR3_MESSAGE(name,value) ClassDB::bind_integer_constant(get_class_static(),StringName(),#name,mir3::protocol::name);
#include "protocol/message_ids.inc"
#undef MIR3_MESSAGE

    ADD_SIGNAL(MethodInfo("pickup_requested",PropertyInfo(Variant::INT,"identifier"),PropertyInfo(Variant::VECTOR2I,"cell")));
    ClassDB::bind_method(D_METHOD("set_hud_decoration","state"),&Mir3HudView::set_hud_decoration);
    ClassDB::bind_method(D_METHOD("apply_inventory_add","identifier","icon","count","make_index","name","std_mode","stack_mode"),&Mir3HudView::apply_inventory_add);
    ClassDB::bind_method(D_METHOD("add_inventory_item","icon","count","slot","make_index","name","std_mode","stack_mode"),&Mir3HudView::add_inventory_item,DEFVAL(0),DEFVAL(String()),DEFVAL(40),DEFVAL(0));
    ClassDB::bind_method(D_METHOD("apply_inventory_use_reply","identifier","remaining"),&Mir3HudView::apply_inventory_use_reply);
    ADD_SIGNAL(MethodInfo("spell_key_requested",PropertyInfo(Variant::INT,"key"),
        PropertyInfo(Variant::INT,"skill_id"),PropertyInfo(Variant::VECTOR2,"pointer")));
    ADD_SIGNAL(MethodInfo("skill_key_requested",PropertyInfo(Variant::INT,"id"),PropertyInfo(Variant::INT,"legacy_key")));
    ClassDB::bind_method(D_METHOD("set_group_roster","player","members"),&Mir3HudView::set_group_roster);
    ClassDB::bind_method(D_METHOD("apply_guild_overview_reply","identifier","body"),&Mir3HudView::apply_guild_overview_reply);
    ClassDB::bind_method(D_METHOD("set_guild_overview","name","lines","editable"),&Mir3HudView::set_guild_overview,DEFVAL(PackedStringArray()));
    ADD_SIGNAL(MethodInfo("guild_overview_requested"));
    ClassDB::bind_method(D_METHOD("set_character_identity","name","secondary","tertiary","badge"),&Mir3HudView::set_character_identity);
    ClassDB::bind_method(D_METHOD("set_character_elements","attack","strength","weakness"),&Mir3HudView::set_character_elements);
    ClassDB::bind_method(D_METHOD("set_character_combat","attack","magic","spirit","defence","magic_defence"),&Mir3HudView::set_character_combat);
    ClassDB::bind_method(D_METHOD("set_character_progress","level","profession","appearance","experience","next_experience"),&Mir3HudView::set_character_progress);
    ClassDB::bind_method(D_METHOD("set_character_load_bonuses","wear","hand","bonuses"),&Mir3HudView::set_character_load_bonuses);
    ClassDB::bind_method(D_METHOD("set_character_vitals","hp","max_hp","mp","max_mp","weight","max_weight"),&Mir3HudView::set_character_vitals);
    ClassDB::bind_method(D_METHOD("set_music_map","map"),&Mir3HudView::set_music_map);
    ClassDB::bind_method(D_METHOD("set_quest_entries","titles","completed"),&Mir3HudView::set_quest_entries);
    ClassDB::bind_method(D_METHOD("fail_quest_detail_request","index"),&Mir3HudView::fail_quest_detail_request);
    ClassDB::bind_method(D_METHOD("set_quest_details","index","lines"),&Mir3HudView::set_quest_details);
    ADD_SIGNAL(MethodInfo("quest_detail_requested",PropertyInfo(Variant::INT,"index")));
    ClassDB::bind_method(D_METHOD("apply_mount_reply","identifier","recognition"),&Mir3HudView::apply_mount_reply);
    ADD_SIGNAL(MethodInfo("minimap_requested",PropertyInfo(Variant::INT,"identifier")));
    ClassDB::bind_method(D_METHOD("apply_minimap_reply","map_number"),&Mir3HudView::apply_minimap_reply);
    ClassDB::bind_method(D_METHOD("apply_minimap_unavailable"),&Mir3HudView::apply_minimap_unavailable);
    ADD_SIGNAL(MethodInfo("npc_packet_ready",PropertyInfo(Variant::PACKED_BYTE_ARRAY,"packet")));
    ADD_SIGNAL(MethodInfo("client_packet_ready",PropertyInfo(Variant::PACKED_BYTE_ARRAY,"packet")));
    ADD_SIGNAL(MethodInfo("game_connection_close_requested"));
    ADD_SIGNAL(MethodInfo("character_selection_connection_requested"));
    ClassDB::bind_method(D_METHOD("open_login"),&Mir3HudView::open_login);
    ClassDB::bind_method(D_METHOD("open_server_groups"),&Mir3HudView::open_server_groups);
    ClassDB::bind_method(D_METHOD("set_connection_enabled","enabled"),&Mir3HudView::set_connection_enabled);
    ClassDB::bind_method(D_METHOD("apply_login_message","message"),&Mir3HudView::apply_login_message);
    ClassDB::bind_method(D_METHOD("apply_game_message","message"),&Mir3HudView::apply_game_message);
    ClassDB::bind_method(D_METHOD("monster_behavior_profile","shape"),&Mir3HudView::monster_behavior_profile);
    ClassDB::bind_method(D_METHOD("advance_monster_ai","elapsed_ms","targets"),&Mir3HudView::advance_monster_ai);
    ClassDB::bind_method(D_METHOD("bind_reference_monster","actor","ai","dc","attack_delay","move_delay","element"),&Mir3HudView::bind_reference_monster);
    ClassDB::bind_method(D_METHOD("configure_reference_combat","actor","stats"),&Mir3HudView::configure_reference_combat);
    ClassDB::bind_method(D_METHOD("spawn_reference_monster","actor","cell","shape","facing"),&Mir3HudView::spawn_reference_monster);
    ClassDB::bind_method(D_METHOD("spawn_reference_bee_queen","actor","cell","shape","child_shape"),&Mir3HudView::spawn_reference_bee_queen);
    ClassDB::bind_method(D_METHOD("spawn_reference_animal","actor","cell","shape","facing","mission"),&Mir3HudView::spawn_reference_animal);
    ClassDB::bind_method(D_METHOD("notify_reference_animal_struck","actor","hitter","hitter_cell","proper","level"),&Mir3HudView::notify_reference_animal_struck);
    ClassDB::bind_method(D_METHOD("configure_reference_animal_combat","actor","hp","dc","ac","hit","speed","level"),&Mir3HudView::configure_reference_animal_combat);
    ADD_SIGNAL(MethodInfo("login_connection_requested",PropertyInfo(Variant::STRING,"host"),PropertyInfo(Variant::INT,"port")));
    ADD_SIGNAL(MethodInfo("login_connection_close_requested"));
    ADD_SIGNAL(MethodInfo("selection_endpoint_ready",PropertyInfo(Variant::STRING,"host"),PropertyInfo(Variant::INT,"port")));
    ADD_SIGNAL(MethodInfo("login_url_requested",PropertyInfo(Variant::STRING,"url")));
    ClassDB::bind_method(D_METHOD("set_selection_session","account","number"),&Mir3HudView::set_selection_session);
    ClassDB::bind_method(D_METHOD("request_selection_list"),&Mir3HudView::request_selection_list);
    ClassDB::bind_method(D_METHOD("apply_selection_message","message"),&Mir3HudView::apply_selection_message);
    ADD_SIGNAL(MethodInfo("npc_request",PropertyInfo(Variant::INT,"identifier"),PropertyInfo(Variant::INT,"actor"),
        PropertyInfo(Variant::STRING,"body")));
    ADD_SIGNAL(MethodInfo("npc_dialogue_requested",PropertyInfo(Variant::INT,"actor")));
    ClassDB::bind_method(D_METHOD("apply_shop_purchase_reply","identifier","value"),&Mir3HudView::apply_shop_purchase_reply);
    ClassDB::bind_method(D_METHOD("apply_shop_goods","actor","count","body","wire_format"),&Mir3HudView::apply_shop_goods,DEFVAL(false));
    ClassDB::bind_method(D_METHOD("apply_shop_offers","actor","index","names","icons","prices","identifiers","offset"),&Mir3HudView::apply_shop_offers,DEFVAL(-1));
    ADD_SIGNAL(MethodInfo("shop_purchase_requested",PropertyInfo(Variant::INT,"identifier"),
        PropertyInfo(Variant::INT,"actor"),PropertyInfo(Variant::STRING,"name")));
    ADD_SIGNAL(MethodInfo("shop_offer_selected",PropertyInfo(Variant::INT,"actor"),PropertyInfo(Variant::INT,"identifier")));
    ADD_SIGNAL(MethodInfo("shop_good_requested",PropertyInfo(Variant::INT,"actor"),PropertyInfo(Variant::INT,"index"),PropertyInfo(Variant::STRING,"name")));
    ADD_SIGNAL(MethodInfo("npc_dialogue_action",PropertyInfo(Variant::INT,"actor"),PropertyInfo(Variant::STRING,"command")));
    ClassDB::bind_method(D_METHOD("apply_npc_message","encoded","key"),&Mir3HudView::apply_npc_message);
    ClassDB::bind_method(D_METHOD("apply_npc_body","opcode","actor","count","encoded"),&Mir3HudView::apply_npc_body);
    ClassDB::bind_method(D_METHOD("apply_npc_dialogue","opcode","actor","body"),&Mir3HudView::apply_npc_dialogue);
    ClassDB::bind_method(D_METHOD("walk_to_map_cell","destination"),&Mir3HudView::walk_to_map_cell);
    ADD_SIGNAL(MethodInfo("map_change_finished",PropertyInfo(Variant::INT,"opcode"),PropertyInfo(Variant::STRING,"map_id"),PropertyInfo(Variant::BOOL,"success")));
    ClassDB::bind_method(D_METHOD("set_local_actor_identity","actor"),&Mir3HudView::set_local_actor_identity);
    ClassDB::bind_method(D_METHOD("change_map","map_id","name","spawn","ambient_mode"),&Mir3HudView::change_map,DEFVAL(0));
    ClassDB::bind_method(D_METHOD("set_map_location","name","x","y"),&Mir3HudView::set_map_location);
    ClassDB::bind_method(D_METHOD("apply_scene_health","actor","opcode","current","mana","maximum"),&Mir3HudView::apply_scene_health);
    ClassDB::bind_method(D_METHOD("set_scene_health_flags","actor","status_tick","saved_tick","excluded","raised"),&Mir3HudView::set_scene_health_flags);
    ClassDB::bind_method(D_METHOD("apply_scene_speech","actor","opcode","value"),&Mir3HudView::apply_scene_speech);
    ClassDB::bind_method(D_METHOD("apply_scene_name","actor","opcode","color","value"),&Mir3HudView::apply_scene_name);
    ClassDB::bind_method(D_METHOD("set_scene_npc","actor","cell","first_frame"),&Mir3HudView::set_scene_npc);
    ClassDB::bind_method(D_METHOD("set_scene_chicken","actor","cell"),&Mir3HudView::set_scene_chicken);
    ClassDB::bind_method(D_METHOD("set_scene_human","actor","cell","facing","profession","name"),&Mir3HudView::set_scene_human);
    ClassDB::bind_method(D_METHOD("set_scene_human_body","actor","type","clothing"),&Mir3HudView::set_scene_human_body);
    ClassDB::bind_method(D_METHOD("set_player_mount","shape"),&Mir3HudView::set_player_mount);
    ClassDB::bind_method(D_METHOD("set_scene_human_mount","actor","shape"),&Mir3HudView::set_scene_human_mount);
    ClassDB::bind_method(D_METHOD("set_player_dyes","clothing","hair"),&Mir3HudView::set_player_dyes);
    ClassDB::bind_method(D_METHOD("set_scene_human_dyes","actor","clothing","hair"),&Mir3HudView::set_scene_human_dyes);
    ClassDB::bind_method(D_METHOD("set_player_attachments","hair","weapon"),&Mir3HudView::set_player_attachments);
    ClassDB::bind_method(D_METHOD("set_scene_human_attachments","actor","hair","weapon"),&Mir3HudView::set_scene_human_attachments);
    ClassDB::bind_method(D_METHOD("set_player_body","type","clothing"),&Mir3HudView::set_player_body);
    ClassDB::bind_method(D_METHOD("apply_scene_human_life","actor","opcode","cell","facing"),&Mir3HudView::apply_scene_human_life);
    ClassDB::bind_method(D_METHOD("apply_scene_message","encoded","key"),&Mir3HudView::apply_scene_message);
    ClassDB::bind_method(D_METHOD("queue_scene_human_state","actor","opcode","cell","facing","body","hit_weapon"),&Mir3HudView::queue_scene_human_state,DEFVAL(0));
    ClassDB::bind_method(D_METHOD("queue_scene_human_message","actor","opcode","cell","facing","hit_weapon","flags"),&Mir3HudView::queue_scene_human_message,DEFVAL(0),DEFVAL(0));
    ClassDB::bind_method(D_METHOD("apply_scene_human_action","actor","opcode","facing","hit_weapon"),&Mir3HudView::apply_scene_human_action,DEFVAL(0));
    ClassDB::bind_method(D_METHOD("apply_scene_human_stand","actor","cell","facing"),&Mir3HudView::apply_scene_human_stand);
    ClassDB::bind_method(D_METHOD("apply_scene_human_step","actor","opcode","destination","facing"),&Mir3HudView::apply_scene_human_step);
    ClassDB::bind_method(D_METHOD("remove_scene_actor","actor"),&Mir3HudView::remove_scene_actor);
    ClassDB::bind_method(D_METHOD("set_minimap_position","x","y"),&Mir3HudView::set_minimap_position);
    ClassDB::bind_method(D_METHOD("set_minimap_markers","type_five","nearby_green","nearby_orange"),&Mir3HudView::set_minimap_markers);
    ClassDB::bind_method(D_METHOD("set_hovered_target","actor","type","shape","excluded","state","hp","max_hp","percent","cell"),
        &Mir3HudView::set_hovered_target,DEFVAL(Vector2i(-1,-1)));
    ADD_SIGNAL(MethodInfo("target_attributes_requested",PropertyInfo(Variant::INT,"identifier"),
        PropertyInfo(Variant::INT,"actor"),PropertyInfo(Variant::VECTOR2I,"cell")));
    ClassDB::bind_method(D_METHOD("apply_target_attributes","actor","category","state","body"),&Mir3HudView::apply_target_attributes);
    ClassDB::bind_method(D_METHOD("set_hovered_human","actor","type","profession","name"),&Mir3HudView::set_hovered_human,DEFVAL(String()));
    ClassDB::bind_method(D_METHOD("apply_target_portrait","actor","jpeg"),&Mir3HudView::apply_target_portrait);
    ADD_SIGNAL(MethodInfo("target_portrait_requested",PropertyInfo(Variant::INT,"identifier"),
        PropertyInfo(Variant::INT,"actor"),PropertyInfo(Variant::STRING,"name")));
    ADD_SIGNAL(MethodInfo("portrait_list_requested"));
    ADD_SIGNAL(MethodInfo("portrait_upload_requested",PropertyInfo(Variant::PACKED_BYTE_ARRAY,"jpeg")));
    ClassDB::bind_method(D_METHOD("load_portrait_cache","server","character"),&Mir3HudView::load_portrait_cache);
    ClassDB::bind_method(D_METHOD("apply_portrait_reply","jpeg"),&Mir3HudView::apply_portrait_reply);
    ClassDB::bind_method(D_METHOD("set_portrait_names","names"),&Mir3HudView::set_portrait_names);
    ADD_SIGNAL(MethodInfo("guild_members_requested"));
    ADD_SIGNAL(MethodInfo("guild_edit_requested",PropertyInfo(Variant::INT,"identifier"),PropertyInfo(Variant::STRING,"body")));
    ADD_SIGNAL(MethodInfo("guild_edit_bytes_requested",PropertyInfo(Variant::INT,"identifier"),PropertyInfo(Variant::PACKED_BYTE_ARRAY,"body")));
    ADD_SIGNAL(MethodInfo("guild_member_removal_requested",PropertyInfo(Variant::STRING,"name")));
    ClassDB::bind_method(D_METHOD("set_guild_manager","allowed"),&Mir3HudView::set_guild_manager);
    ClassDB::bind_method(D_METHOD("apply_guild_members_reply","identifier","body"),&Mir3HudView::apply_guild_members_reply);
    ClassDB::bind_method(D_METHOD("set_guild_members","lines","editable"),&Mir3HudView::set_guild_members,DEFVAL(PackedStringArray()));
    ClassDB::bind_method(D_METHOD("post_system_notice","text","foreground","background"),&Mir3HudView::post_system_notice);
    ClassDB::bind_method(D_METHOD("set_guild_notices","lines"),&Mir3HudView::set_guild_notices);
    ADD_SIGNAL(MethodInfo("group_member_requested",PropertyInfo(Variant::INT,"identifier"),PropertyInfo(Variant::STRING,"name")));
    ADD_SIGNAL(MethodInfo("group_allow_requested",PropertyInfo(Variant::BOOL,"allowed")));
    ClassDB::bind_method(D_METHOD("apply_skill_records","identifier","count","records"),&Mir3HudView::apply_skill_records);
    ClassDB::bind_method(D_METHOD("apply_skill_removal","id"),&Mir3HudView::apply_skill_removal);
    ClassDB::bind_method(D_METHOD("apply_skill_progress","id","experience","level"),&Mir3HudView::apply_skill_progress);
    ClassDB::bind_method(D_METHOD("set_learned_skill","id","level","key","experience","required","name"),&Mir3HudView::set_learned_skill,DEFVAL(0),DEFVAL(0),DEFVAL(String()));
    ClassDB::bind_method(D_METHOD("apply_equipment_reply","identifier"),&Mir3HudView::apply_equipment_reply);
    ADD_SIGNAL(MethodInfo("equipment_requested",PropertyInfo(Variant::INT,"identifier"),PropertyInfo(Variant::INT,"slot"),PropertyInfo(Variant::INT,"make_index"),PropertyInfo(Variant::STRING,"name")));
    ADD_SIGNAL(MethodInfo("inventory_use_requested",PropertyInfo(Variant::INT,"make_index"),PropertyInfo(Variant::INT,"tick"),PropertyInfo(Variant::STRING,"name")));
    ClassDB::bind_method(D_METHOD("hud_snapshot"),&Mir3HudView::hud_snapshot);
    ClassDB::bind_method(D_METHOD("post_chat","value","foreground","background"),&Mir3HudView::post_chat);
    ClassDB::bind_method(D_METHOD("update_chat_rejection","channel","rejected"),&Mir3HudView::update_chat_rejection);
    ClassDB::bind_method(D_METHOD("handle_exit_confirmation","dialog_id"),&Mir3HudView::handle_exit_confirmation);
    ClassDB::bind_method(D_METHOD("set_local_exit_state","action","ticks"),&Mir3HudView::set_local_exit_state);
    ClassDB::bind_method(D_METHOD("set_trade_context","direction","cell","actor_type","dead","name"),&Mir3HudView::set_trade_context);
    ADD_SIGNAL(MethodInfo("trade_gold_requested",PropertyInfo(Variant::INT,"identifier"),PropertyInfo(Variant::INT,"amount")));
    ADD_SIGNAL(MethodInfo("trade_offer_requested",PropertyInfo(Variant::INT,"identifier"),
        PropertyInfo(Variant::INT,"make_index"),PropertyInfo(Variant::STRING,"name"),PropertyInfo(Variant::INT,"count")));
    ClassDB::bind_method(D_METHOD("apply_trade_offer_reply","identifier","remaining","new_index"),&Mir3HudView::apply_trade_offer_reply,DEFVAL(0),DEFVAL(0));
    ClassDB::bind_method(D_METHOD("set_trade_item","side","slot","icon","count","make_index","name","std_mode","stack_mode"),&Mir3HudView::set_trade_item,DEFVAL(0),DEFVAL(String()),DEFVAL(40),DEFVAL(0));
    ClassDB::bind_method(D_METHOD("apply_trade_gold_reply","identifier","amount","low_word","high_word"),&Mir3HudView::apply_trade_gold_reply,DEFVAL(0),DEFVAL(0));
    ClassDB::bind_method(D_METHOD("set_trade_gold","local","remote"),&Mir3HudView::set_trade_gold);
    ClassDB::bind_method(D_METHOD("apply_trade_complete","balance"),&Mir3HudView::apply_trade_complete);
    ClassDB::bind_method(D_METHOD("apply_trade_cancel","balance"),&Mir3HudView::apply_trade_cancel,DEFVAL(-1));
    ClassDB::bind_method(D_METHOD("open_trade","partner"),&Mir3HudView::open_trade);
    ADD_SIGNAL(MethodInfo("trade_requested",PropertyInfo(Variant::INT,"identifier"),PropertyInfo(Variant::STRING,"name")));
    ADD_SIGNAL(MethodInfo("exit_confirmed",PropertyInfo(Variant::INT,"dialog_id")));
    ADD_SIGNAL(MethodInfo("hud_action",PropertyInfo(Variant::INT,"button")));
    ClassDB::bind_method(D_METHOD("apply_options_state","flags","attack_mode"),&Mir3HudView::apply_options_state);
    ADD_SIGNAL(MethodInfo("chat_requested",PropertyInfo(Variant::STRING,"text")));
}
Dictionary Mir3HudView::hud_snapshot() const {
    Dictionary result;
    result["ready"]=loaded;
    result["character_level"]=model.level;
    result["terrain_ready"]=terrain_ready;
    result["displayed_name_actor"]=displayed_name_actor;
    result["displayed_name_bounds"]=displayed_name_bounds;
    result["displayed_name_color"]=scene_names.contains(displayed_name_actor)?scene_names.at(displayed_name_actor).color:0xffffff;
    result["displayed_name_text"]=scene_names.contains(displayed_name_actor)?
        String::utf8(mir3::ansi_to_utf8(scene_names.at(displayed_name_actor).lines[0]).c_str()):String();
    result["player_bounds"]=player_bounds;
    Array shadows;for (const auto& bounds:human_shadow_bounds) shadows.push_back(bounds);
    result["human_shadow_bounds"]=shadows;
    Array glows;for (int frame:drawn_weapon_glow_frames) glows.push_back(frame);
    result["weapon_glow_frames"]=glows;
    result["weapon_glow_light_count"]=int(weapon_glow_lights.size());
    Dictionary bubbles, bubble_lines;
    for (const auto& [actor,bounds]:speech_bounds) bubbles[actor]=bounds;
    for (const auto& [actor,bubble]:scene_speech) {
        PackedStringArray lines;
        for (const auto& line:bubble.lines) lines.push_back(String::utf8(line.text.c_str()));
        bubble_lines[actor]=lines;
    }
    result["speech_bounds"]=bubbles;
    result["speech_lines"]=bubble_lines;
    Dictionary bars;
    for (const auto& [actor,bounds]:scene_health_bounds) bars[actor]=bounds;
    result["scene_health_bounds"]=bars;
    Dictionary damage_frames, damage_positions;
    for (const auto& [actor,glyphs]:scene_damage_glyphs) {
        PackedInt32Array frames;
        for (const auto& glyph:glyphs) frames.push_back(glyph.frame);
        damage_frames[actor]=frames;
        if (!glyphs.empty()) damage_positions[actor]=Vector2i(glyphs.front().x,glyphs.front().y);
    }
    result["scene_damage_frames"]=damage_frames;
    result["scene_damage_positions"]=damage_positions;
    Dictionary npc_bounds, npc_standing, npc_standing_frames, npc_arrival;
    for (const auto& [actor,npc]:scene_npcs) {
        npc_bounds[actor]=npc.bounds;
        npc_standing[actor]=Vector2i(int(npc.standing.size()),npc.standing_interval);
        npc_standing_frames[actor]=int(npc.standing_frame);
        npc_arrival[actor]=npc.arrival_ms;
    }
    result["scene_npc_bounds"]=npc_bounds;
    result["scene_npc_standing"]=npc_standing;
    result["scene_npc_standing_frames"]=npc_standing_frames;
    result["scene_npc_arrival_ms"]=npc_arrival;
    append_monster_snapshot(result);
    Dictionary actor_health_values;
    for (const auto& [actor,health]:scene_health) {
        PackedInt32Array values;
        values.push_back(health.current);values.push_back(health.mana);values.push_back(health.maximum);
        actor_health_values[actor]=values;
    }
    result["scene_health_values"]=actor_health_values;
    Dictionary item_bounds, item_cells, item_sparkles;
    for (const auto& [id,bounds]:ground_bounds) item_bounds[id]=bounds;
    for (const auto& item:ground_items.items) {
        item_cells[item.id]=Vector2i(item.x,item.y);
        if (item.sparkle_frame>=0 && ground_sparkles[item.sparkle_frame].is_valid())
            item_sparkles[item.id]=40+item.sparkle_frame;
    }
    result["ground_item_bounds"]=item_bounds;
    result["ground_item_cells"]=item_cells;
    result["ground_item_sparkles"]=item_sparkles;
    result["displayed_ground_item"]=displayed_ground_item;
    Dictionary human_bounds;
    Dictionary human_cells;
    Dictionary human_idle_frames;
    Dictionary human_actions;
    Dictionary human_pending, human_movement, human_catchup, human_life, human_stance;
    Dictionary human_facings;
    Dictionary human_appearances;
    Dictionary human_attachments;
    Dictionary human_dyes;
    Dictionary human_mounts;
    for (const auto& [actor,human]:scene_humans) human_bounds[actor]=human.bounds;
    for (const auto& [actor,human]:scene_humans) human_cells[actor]=human.cell;
    for (const auto& [actor,human]:scene_humans) {
        human_idle_frames[actor]=human.idle.frame();
        Dictionary life;
        life["dead"]=human.life.dead; life["dying"]=human.life.dying;
        life["opacity"]=human.life.opacity();
        life["frame"]=human.life.dead?(human.life.dying?human.action.frame():9):-1;
        human_life[actor]=life;
        human_stance[actor]=human.combat.idle_action(human.mount_shape!=0);
        human_pending[actor]=int(human.messages.size());
        human_movement[actor]=human.movement_frame;
        human_catchup[actor]=human.catching_up;
        human_actions[actor]=Vector2i(human.action.finished()?-1:human.action.action(),human.action.frame());
        human_facings[actor]=human.facing;
        human_mounts[actor]=human.mount_shape;
        human_dyes[actor]=Vector2i(human.clothing_dye,human.hair_dye);
        human_attachments[actor]=Vector2i(human.hair,human.weapon);
        human_appearances[actor]=Vector2i(human.type,human.clothing);
    }
    result["scene_human_bodies"]=human_appearances;
    result["scene_human_attachments"]=human_attachments;
    result["scene_human_dyes"]=human_dyes;
    result["scene_human_mounts"]=human_mounts;
    result["player_mount"]=player_mount;
    result["player_dyes"]=Vector2i(player_clothing_dye,player_hair_dye);
    result["player_attachments"]=Vector2i(player_hair,player_weapon);
    result["player_body"]=Vector2i(player_type,player_clothing);
    result["scene_human_idle_frames"]=human_idle_frames;
    result["scene_human_actions"]=human_actions;
    result["scene_human_life"]=human_life;
    result["scene_human_stance"]=human_stance;
    result["scene_human_pending"]=human_pending;
    result["scene_human_movement"]=human_movement;
    result["scene_human_catchup"]=human_catchup;
    result["scene_human_facings"]=human_facings;
    result["scene_human_bounds"]=human_bounds;
    Dictionary human_anchors;
    for (const auto& [actor,human]:scene_humans) human_anchors[actor]=human.anchor;
    result["scene_human_anchors"]=human_anchors;
    result["scene_human_cells"]=human_cells;
    Dictionary human_arrival;
    for (const auto& [actor,human]:scene_humans) human_arrival[actor]=human.arrival_ms;
    result["scene_human_arrival_ms"]=human_arrival;
    result["player_idle_frame"]=player_idle.frame();
    result["player_facing"]=player_facing;
    result["npc_dialogue_actor"]=npc_dialogue_actor;
    result["local_actor_identity"]=local_actor_identity;
    result["player_light_attribute"]=player_light_attribute;
    Dictionary lights;
    for (const auto& [id,human]:scene_humans) lights[id]=human.light_attribute;
    result["scene_human_lights"]=lights;
    result["scene_light_samples"]=scene_light_samples;
    result["scene_ambient"]=scene_ambient.level();
    result["scene_ambient_selected_base"]=scene_ambient.selected_base;
    result["scene_ambient_mode"]=scene_ambient.mode;
    result["scene_ambient_fading"]=scene_ambient.fading;
    result["pending_map_changes"]=int(map_changes.size());
    result["map_series"]=map_series;
    result["weather_snow"]=weather.snow;result["weather_fog"]=weather.fog;result["weather_other"]=weather.other;
    result["snow_particle_count"]=weather.active;result["weather_fog_frame"]=weather.fog_frame;
    result["weather_fog_position"]=Vector2(weather.fog_x,weather.fog_y);
    result["rain_particle_count"]=weather.rain_active;
    result["rain_resources_loaded"]=std::all_of(rain_textures.begin()+10,rain_textures.end(),
        [](const auto& image){return image.is_valid();});
    for(const auto& p:weather.rain_particles) if(p.active) {
        if(p.splash()) {
            result["rain_splash_position"]=Vector2(p.x,p.y);
            break;
        }
        if(!result.has("rain_first_position")) {
            result["rain_first_position"]=Vector2(p.x,p.y);
            result["rain_first_age"]=p.age;
        }
    }
    result["snow_resource_loaded"]=snow_texture.is_valid();
    result["fog_resource_loaded"]=fog_sprite && fog_sprite->get_texture().is_valid();
    if(weather.active) for(const auto& p:weather.particles) if(p.active) {
        result["snow_first_position"]=Vector2(p.x,p.y);result["snow_first_age"]=p.age;break;
    }
    result["map_left_delay_ms"]=map_input_delay.remaining_ms;
    result["map_id"]=current_map_id;
    result["map_name"]=String::utf8(model.map_name.c_str());
    result["player_cell"]=player_cell;
    result["walk_frame"]=walk_frame;
    result["map_camera_pixel"]=map_camera();
    result["map_player_pixel"]=player_pixel();
    const auto movement=step_destination-step_origin;
    result["map_running"]=walk_frame>=0 && std::max(std::abs(movement.x),std::abs(movement.y))>1;
    result["map_step_distance"]=std::max(std::abs(movement.x),std::abs(movement.y));
    result["map_step_origin"]=step_origin;
    result["map_step_destination"]=step_destination;
    result["route_destination"]=map_route.empty()?Vector2i(-1,-1):Vector2i(map_route.back()[0],map_route.back()[1]);
    result["last_movement_sound"]=last_movement_sound;
    Dictionary actor_audio;
    for (const auto& [actor,sound]:actor_sounds) {
        Dictionary entry;
        entry["sample"]=sound.sample; entry["count"]=sound.count;
        entry["volume"]=sound.volume; entry["pan"]=sound.pan;
        entry["playing"]=sound.player->is_playing();
        actor_audio[actor]=entry;
    }
    result["scene_actor_sounds"]=actor_audio;
    result["character_portrait"]=model.portrait_frame();
    result["target_portrait"]=model.layout.expanded || !target_focus.displayed() || target_portrait.is_null()?-1:target_frame;
    result["target_hp_width"]=!model.layout.expanded && target_focus.displayed() && !target_is_human && target_portrait.is_valid()?target_vitals.bar_width():0;
    result["target_custom_portrait"]=!model.layout.expanded && target_focus.displayed() && system_options.user_photos && target_custom_portrait.is_valid();
    result["character_shared_hp"]=model.shared_hp();
    Array level_states;
    for (const auto& effect:level_effects) {
        Dictionary state;state["frame"]=int(effect.playback.frame);state["phase"]=int(effect.playback.phase);
        state["cell"]=Vector2i(effect.playback.cell_x,effect.playback.cell_y);
        state["radii"]=Vector2i(effect.playback.inner_radius,effect.playback.outer_radius);level_states.append(state);
    }
    result["level_effects"]=level_states;
    result["map_event_count"]=int(map_events.size()+ground_events.size());
    Dictionary event_states, ground_wave_delays;
    Array spell_states, scene_effect_states;
    for(const auto& [id,event]:map_events) {
        event_states[int64_t(id)]=event.frame;
        if (event.ground_wave) ground_wave_delays[event.cell]=event.delay_ms;
        {
            Dictionary state;
            state["skill"]=int(event.resource->id);state["frame"]=event.frame;
            state["library"]=int(event.resource->library);
            state["sprite_visible"]=event.sprite && event.sprite->is_visible_in_tree() && event.sprite->get_texture().is_valid();
            state["first"]=int(event.resource->first);state["end"]=int(event.resource->end);
            state["interval"]=int(event.resource->interval);state["frame_offset"]=event.frame_offset;
            state["animation_elapsed_ms"]=event.elapsed;
            state["repeat"]=!event.one_shot;
            state["target_actor"]=int64_t(event.projectile?event.projectile->target_actor:event.follow_actor);
            state["attached_actor"]=event.attached_actor;
            state["projectile"]=event.projectile.has_value();state["delay"]=event.delay_ms;
            if (event.projectile) {
                state["velocity"]=event.projectile->velocity;
                state["destination"]=event.projectile->destination;
            }
            state["light_tail"]=event.light_tail;
            state["emits_light"]=event.emits_light;
            state["ground_wave"]=event.ground_wave;
            state["cell"]=event.cell;
            state["world_position"]=event.projectile?event.projectile->position:
                event.follow_position.value_or(Vector2(event.cell*Vector2i(48,32)));
            state["stage"]=event.staged_spell?int(event.staged_spell->stage):-1;
            if (event.staged_spell) state["stage_offset"]=Vector2i(event.staged_spell->offset[0],event.staged_spell->offset[1]);
            scene_effect_states.append(state);
            if (event.spell_effect || event.staged_spell || event.ground_wave) spell_states.append(state);
        }
    }
    result["spell_effects"]=spell_states;
    result["scene_effects"]=scene_effect_states;
    result["human_spell_light_count"]=int(human_spell_lights.size());
    result["drawn_human_spells"]=drawn_human_spells;
    result["map_ground_wave_delays"]=ground_wave_delays;
    for(const auto& [id,event]:ground_events) if(!event_states.has(int64_t(id))) event_states[int64_t(id)]=event.frame;
    result["map_event_particle_count"]=int(spell_particles.ice_count());
    result["spell_particle_count"]=int(spell_particles.count());
    result["map_event_mark_count"]=int(event_marks.size());
    result["map_ground_event_count"]=int(ground_events.size());
    result["map_event_frames"]=event_states;
    result["player_level_action"]=!player_action.finished() && player_action.action()==7;
    result["player_action"]=player_action.finished()?-1:player_action.action();
    result["gold_sound_count"]=gold_sound_count;
    result["weapon_training"]=weapon_training;
    result["character_experience"]=int64_t(model.experience);result["character_next_experience"]=int64_t(model.next_experience);
    result["character_hp"]=Vector2i(model.hp,model.max_hp);
    result["character_mp"]=Vector2i(model.mp,model.max_mp);
    result["character_weight"]=Vector2i(model.weight,model.max_weight);
    result["character_wear_weight"]=Vector2i(model.wear_weight[0],model.wear_weight[1]);
    result["character_hand_weight"]=Vector2i(model.hand_weight[0],model.hand_weight[1]);
    result["melee_speed"]=melee_cooldown.speed;
    result["player_status_flags"]=int64_t(player_status_flags);
    result["player_status_color"]=actor_status_color(player_status_flags);
    Dictionary human_colors;
    for (const auto& [id,human]:scene_humans) human_colors[id]=actor_status_color(human.action_flags);
    result["scene_human_status_colors"]=human_colors;
    Dictionary human_status;
    for (const auto& [id,human]:scene_humans) human_status[id]=Vector2i(human.action_flags,human.attack_speed);
    result["scene_human_status"]=human_status;
    result["local_action_pending"]=pending_local_action.has_value();
    result["active_spell"]=active_spell;
    result["player_spell_frame"]=player_action.finished()?-1:player_action.spell_frame();
    result["spell_cast_tick"]=int64_t(spell_timing.cast_at);
    result["spell_cast_delay"]=int64_t(spell_timing.casting_delay);
    result["spell_movement_delay"]=int64_t(spell_timing.movement_delay);
    result["action_reply_pending"]=action_prediction.waiting;
    result["player_dead"]=player_life.dead;
    result["player_dying"]=player_life.dying;
    result["forced_motion_opcode"]=forced_motion?forced_motion->opcode:0;
    result["forced_motion_queue"]=int(local_actor_messages.size());
    result["local_action_wait_ms"]=pending_local_action?pending_local_action->wait_ms:0;
    result["melee_interval_ms"]=mir3::MeleeCooldown::interval(model.level,melee_cooldown.speed,model.hand_weight[0],model.hand_weight[1]);
    result["character_bonuses"]=Vector2i(model.status_bonus_percent[0],model.status_bonus_percent[1]);
    result["expanded"]=model.layout.expanded;
    result["quest_first"]=quest.first;
    result["quest_selected"]=quest.selected;
    result["quest_detail_rows"]=quest.detail_rows();
    const auto quest_description=quest.description();
    result["quest_description"]=quest_description.empty()?String():String::utf8(quest_description.data(),quest_description.size());
    PackedStringArray quest_titles;
    PackedInt32Array quest_completion;
    for(const auto& entry:quest.entries) {
        quest_titles.append(String::utf8(entry.title.c_str()));
        quest_completion.append(entry.completed?1:0);
    }
    result["quest_titles"]=quest_titles;
    result["quest_completed"]=quest_completion;
    result["trade_scroll"]=Vector2i(trade_scroll.first[0],trade_scroll.first[1]);
    result["trade_gold_pending"]=trade_gold_pending;
    PackedInt32Array trade_item_counts;
    for (const auto& side:trade_items) {
        int count=0;for (const auto& item:side) if (item) ++count;
        trade_item_counts.append(count);
    }
    result["trade_item_counts"]=trade_item_counts;
    Array trade_records;
    for(int side=0;side<2;++side) for(int slot=0;slot<200;++slot) if(trade_items[side][slot]) {
        const auto& item=trade_items[side][slot];Dictionary row;
        row["side"]=side;row["slot"]=slot;row["id"]=int64_t(item.make_index);row["name"]=String::utf8(item.name.c_str());row["count"]=item.count;
        trade_records.append(row);
    }
    result["trade_records"]=trade_records;
    result["trade_partner"]=trade_partner;
    result["shop_goods_count"]=int(shop_goods.size());result["shop_offers_count"]=int(shop_offers.size());
    result["shop_offer_loading"]=shop_offer_loading;
    result["shop_storage_page"]=storage_list?shop_offer_page+1:0;
    result["trade_gold_local"]=int64_t(trade_gold[0]);
    result["trade_gold_remote"]=int64_t(trade_gold[1]);
    result["inventory_gold"]=int64_t(inventory_gold);
    Array bag_records;
    for (int i=0;i<mir3::InventoryGrid::capacity;++i) if (bag.items[i]) {
        const auto& item=bag.items[i];Dictionary row;
        row["id"]=int64_t(item.make_index);row["name"]=String::utf8(item.name.c_str());
        row["icon"]=item.icon;row["count"]=item.count;row["anchor"]=bag.anchors[i];
        row["durability"]=int(item.server_record[66])|(int(item.server_record[67])<<8);
        bag_records.append(row);
    }
    result["bag_records"]=bag_records;
    Array equipment_records;
    for (int i=0;i<12;++i) if (equipment.slots[i]) {
        const auto& item=equipment.slots[i];Dictionary row;
        row["slot"]=i;row["id"]=int64_t(item.make_index);row["icon"]=item.icon;
        row["name"]=String::utf8(item.name.c_str());
        row["durability"]=int(item.server_record[66])|(int(item.server_record[67])<<8);
        row["max_durability"]=int(item.server_record[68])|(int(item.server_record[69])<<8);
        equipment_records.append(row);
    }
    result["equipment_records"]=equipment_records;
    result["trade_return_slot"]=trade_return_slot;
    result["trade_confirmed"]=trade_confirmed;
    result["trade_visible"]=trade.visible;
    result["quest_visible"]=quest.visible;
    result["quest_bounds"]=rect(quest.rect);
    result["mount_visible"]=mount.visible;
    result["mount_shape"]=mount.shape;
    result["mount_bounds"]=rect(mount.rect);
    result["map_position"]=Vector2i(model.map_x,model.map_y);
    result["minimap_visible"]=minimap.visible;
    result["minimap_bounds"]=rect(minimap.bounds());
    result["minimap_translucent"]=minimap.translucent;
    result["minimap_source"]=point(minimap.source());
    result["pressed"]=model.pressed;
    result["hovered"]=model.hovered;
    result["primary_defence"]=model.primary_stat[0];
    result["primary_attack"]=model.primary_stat[1];
    const auto [defence_frame,defence_min,defence_max]=model.stat(0);
    const auto [attack_frame,attack_min,attack_max]=model.stat(1);
    result["hud_defence"]=Vector2i(defence_min,defence_max);
    result["hud_attack"]=Vector2i(attack_min,attack_max);
    if (!chat_history.lines(false).empty()) {
        const auto& line=chat_history.lines(false).back();
        result["chat_last_text"]=String::utf8(line.text.c_str());
        result["chat_last_foreground"]=line.foreground;
        result["chat_last_background"]=line.background;
    }
    result["chat_compact_count"]=int(chat_history.lines(false).size());
    result["chat_expanded_count"]=int(chat_history.lines(true).size());
    result["chat_compact_first"]=chat_history.first(false);
    result["portrait_map_mode"]=portrait_map_mode;
    result["portrait_map_visible"]=portrait_map_visible && !model.layout.expanded;
    result["chat_expanded_first"]=chat_history.first(true);
    result["portrait_visible"]=portrait_panel.visible;
    result["custom_portrait"]=active_portrait.is_valid();
    result["portrait_preview"]=portrait_preview.is_valid();
    result["portrait_count"]=int64_t(portrait_picker.names.size());
    result["portrait_first"]=portrait_picker.first;
    result["portrait_selected"]=portrait_picker.selected;
    result["client_page"]=server_groups.visible?0:(login_visible?1:(character_selection.visible?2:3));
    result["server_group_count"]=int(server_groups.groups.size());result["server_group_selected"]=server_groups.selected;
    result["server_group_first"]=server_groups.first;result["login_movie_active"]=login_movie_purpose!=LoginMoviePurpose::none;
    result["login_phase"]=int(login_flow.phase);result["login_field"]=login_flow.field;
    result["login_notice"]=login_flow.notice.message;result["login_editor_visible"]=login_visible && login_flow.editor_visible && !login_flow.notice.message;
    result["login_server_count"]=int(login_flow.servers.size());
    result["exit_transition_active"]=exit_transition.active;
    result["exit_transition_ms"]=int64_t(exit_transition.elapsed);
    result["exit_brightness"]=exit_transition.brightness();
    result["selection_connecting"]=character_selection.visible && character_selection.connecting;
    result["selection_count"]=character_selection.count;result["selection_selected"]=character_selection.selected;
    result["selection_page"]=character_selection.page;result["selection_empty_alert"]=selection_notice.message==219;result["selection_notice"]=selection_notice.message;
    result["creation_visible"]=character_creation.visible;result["creation_profession"]=character_creation.profession;result["creation_gender"]=character_creation.selected;
    result["selection_delete_visible"]=selection_delete.visible;result["selection_delete_selected"]=selection_delete.selected;
    Array selection_roles;
    for (int i=0;i<character_selection.count;++i) {
        const auto& role=character_selection.roles[i];Dictionary entry;
        entry["name"]=String::utf8(mir3::ansi_to_utf8(role.name).c_str());entry["frame"]=role.frame;
        entry["action"]=role.action;entry["position"]=Vector2i(role.x,role.y);entry["bounds"]=selection_bounds[i];
        selection_roles.append(entry);
    }
    result["selection_roles"]=selection_roles;
    result["exit_clock"]=int64_t(GetTickCount());
    result["exit_allowed"]=exit_guard.allowed(GetTickCount());
    result["exit_action"]=int(exit_guard.action);
    result["options_visible"]=options.visible;
    result["option_show_mon"]=system_options.show_mon;
    result["option_force_attack"]=system_options.force_attack;
    result["option_shadow_blend"]=system_options.shadow_blend;
    result["option_user_photos"]=system_options.user_photos;
    result["option_attack_mode"]=system_options.attack_mode;
    PackedInt32Array option_buttons;
    for (auto state:system_options.button_states) option_buttons.push_back(state);
    result["option_button_states"]=option_buttons;
    PackedByteArray confirmed_options,displayed_options;
    for (int i=0;i<7;++i) { confirmed_options.push_back(system_options.confirmed[i]);displayed_options.push_back(system_options.displayed[i]); }
    result["option_confirmed"]=confirmed_options;result["option_displayed"]=displayed_options;
    result["music_enabled"]=audio_options.music_enabled;
    result["reverse_pan"]=audio_options.reverse_pan;
    result["magic_light"]=magic_light;
    result["damage_numbers"]=damage_numbers;
    result["music_level"]=audio_options.music_level;
    result["guild_visible"]=guild.visible;
    result["guild_first"]=guild_first;
    result["guild_name"]=guild_name;result["guild_manager"]=guild_manager;
    result["guild_overview"]=guild_pages[0];result["guild_members"]=guild_pages[1];
    result["guild_notice_count"]=guild_pages[2].size();
    result["guild_tab"]=guild_layout.tab;
    result["group_request"]=group_request;
    result["group_visible"]=group.visible;
    result["group_allow"]=group_allow;
    result["group_members"]=group_members;
    result["skill_shortcuts_visible"]=skill_shortcuts_visible;
    result["skills_visible"]=skills.visible;
    result["skill_tab"]=skills.tab;
    result["skill_scroll"]=skills.scroll;
    Array learned_skills;
    for (const auto& category:skills.learned) for (const auto& skill:category) {
        Dictionary entry;entry["id"]=skill.id;entry["level"]=skill.level;entry["key"]=skill.key;
        entry["experience"]=int64_t(skill.experience);entry["required"]=skill.required;
        learned_skills.append(entry);
    }
    result["learned_skills"]=learned_skills;
    PackedInt32Array skill_bindings;for (const int id:skills.bindings) skill_bindings.append(id);
    result["skill_bindings"]=skill_bindings;
    result["status_visible"]=status.visible;
    result["status_position"]=Vector2i(status.rect[0],status.rect[1]);
    result["inventory_visible"]=inventory.visible;
    result["inventory_position"]=Vector2i(inventory.rect[0],inventory.rect[1]);
    result["inventory_use_pending"]=bag.pending_use;
    result["quick_visible"]=quick_panel.visible;
    result["quick_position"]=Vector2i(quick_panel.rect[0],quick_panel.rect[1]);
    result["held_quick_slot"]=bag.held_quick_slot;
    Array quick_records;
    for(int i=0;i<6;++i) if(const auto& item=quick_items.slots[i];item) {
        Dictionary entry;entry["slot"]=i;entry["id"]=int64_t(item.make_index);
        entry["name"]=String::utf8(item.name.c_str());entry["count"]=item.count;quick_records.append(entry);
    }
    result["quick_records"]=quick_records;
    result["equipment_pending"]=equipment.pending;
    result["held_inventory_icon"]=held_gold?124:bag.held.icon;
    result["inventory_row"]=inventory.row;
    result["chat_save_enabled"]=chat_submission.save_chat;result["snow_enabled"]=chat_submission.snow;
    result["chat_contacts_visible"]=chat_submission.control_down && chat_input && chat_input->has_focus() && !group_request && !guild_editor.visible && !chat_submission.contacts.empty();
    result["chat_phrases_visible"]=chat_submission.alt_down && chat_input && chat_input->has_focus() && !group_request && !guild_editor.visible && !chat_submission.phrases.empty();
    result["chat_contact_selected"]=chat_submission.contact_selected;result["chat_phrase_selected"]=chat_submission.phrase_selected;
    result["private_chat_prefix"]=String::utf8(chat_submission.private_prefix.c_str());
    PackedStringArray contacts,phrases;
    for(const auto& item:chat_submission.contacts) contacts.append(String::utf8(item.c_str()));
    for(const auto& item:chat_submission.phrases) phrases.append(String::utf8(item.c_str()));
    result["private_chat_contacts"]=contacts;result["quick_chat_phrases"]=phrases;
    result["chat_mode"]=chat_input?chat_input->mode.mode:0;
    PackedInt32Array frames,hints,rejected;
    for (int i=0;i<3;++i) { frames.push_back(reception_buttons.frames[i]);hints.push_back(reception_buttons.hints[i]);rejected.push_back(rejected_chat[i]); }
    result["reception_frames"]=frames;result["reception_hints"]=hints;result["rejected_chat"]=rejected;
    return result;
}
void Mir3HudView::_ready() {
    const auto configured=get_meta("options_path",String("user://Config.ini"));
    options_path=ProjectSettings::get_singleton()->globalize_path(String(configured));
    load_options();
    Ref<ConfigFile> config; config.instantiate();
    if (config->load("res://resource_path.cfg")!=OK) {
        UtilityFunctions::push_error("Missing resource_path.cfg; configure the client with CMake."); return;
    }
    String root=config->get_value("resources","root","");
    item_layout_directory=ProjectSettings::get_singleton()->globalize_path(String(get_meta("item_layout_directory",config->get_value("storage","item_layout_directory","user://Data"))));
    const String log_path=get_meta("chat_log_path",config->get_value("storage","chat_log_path","user://Chat.txt"));
    chat_log_path=log_path.is_empty()?String():ProjectSettings::get_singleton()->globalize_path(log_path);
    if(bool(get_meta("offline_preview",false)) && !has_meta("chat_log_path")) chat_log_path=String();
    // 离线测试须显式指定独立目录，避免写入玩家布局。
    if(bool(get_meta("offline_preview",false)) && !has_meta("item_layout_directory")) item_layout_directory=String();
    loaded=archive.open(std::filesystem::u8path(root.utf8().get_data())/"Data"/"GameInter");
    if (!loaded || frame(1160).is_null()) {
        loaded=false; UtilityFunctions::push_error("Cannot read original GameInter WIL/WIX resources."); return;
    }
    map_directory=root.path_join("Map");
    actor_resource_root=root.path_join("Data");
    const auto effect_file=FileAccess::get_file_as_bytes(root.path_join("MInfo.dat"));
    effect_catalog.load({effect_file.ptr(),std::size_t(effect_file.size())});
    event_archive.open(std::filesystem::u8path(actor_resource_root.utf8().get_data())/"Magic");
    level_archive.open(std::filesystem::u8path(actor_resource_root.utf8().get_data())/"MagicEx");
    weapon_order=FileAccess::get_file_as_bytes(root.path_join("Weapon.ord"));
    audio_root=root.path_join("Sound");
    const auto sound_index=FileAccess::get_file_as_bytes(root.path_join("SoundList.wwl"));
    if (!sound_list.parse({sound_index.ptr(),static_cast<std::size_t>(sound_index.size())}))
        UtilityFunctions::push_warning("Cannot parse SoundList.wwl");
    music_table=FileAccess::get_file_as_bytes(root.path_join("BgmList.wwl"));
    map_music=memnew(AudioStreamPlayer);
    map_music->set_name("MapMusic");
    add_child(map_music);
    const auto sample=AudioStreamWAV::load_from_file(root.path_join("Sound/105.wav"));
    if (sample.is_valid()) {
        hud_sound=memnew(AudioStreamPlayer);
        hud_sound->set_name("HudClickSound");
        hud_sound->set_stream(sample);
        add_child(hud_sound);
        sync_hud_sound();
    }
    const auto gold_file=sound_list.file(106);
    if (!gold_file.empty()) {
        const auto stream=AudioStreamWAV::load_from_file(audio_root.path_join(String::utf8(gold_file.c_str())));
        if (stream.is_valid()) {
            gold_sound=memnew(AudioStreamPlayer);gold_sound->set_stream(stream);add_child(gold_sound);sync_hud_sound();
        }
    }
    portrait_directory=ProjectSettings::get_singleton()->globalize_path(String(get_meta("portrait_directory",root)));
    const auto resource_root=std::filesystem::u8path(root.utf8().get_data());
    if (bool(get_meta("show_terrain",true)) && terrain.open(resource_root/"Map"/"0.map")) {
        constexpr std::array names{"Tilesc","Tiles30c","Tiles5c","SmTilesc","Housesc","Cliffsc",
            "Dungeonsc","Innersc","Furnituresc","Wallsc","SmObjectsc","Animationsc","Object1c","Object2c","Custom"};
        constexpr std::array regions{"","Wood","Sand","Snow","Forest"};
        for (std::size_t region=0;region<regions.size();++region) {
            const auto directory=resource_root/"Data"/regions[region];
            if (!std::filesystem::is_directory(directory)) continue;
            for (std::size_t i=0;i<names.size();++i) {
                const bool opened=terrain_archives[region*names.size()+i].open(directory/names[i]);
                if (opened && i<3) terrain_ready=true;
            }
        }
    }
    if (terrain_ready) {
        const auto load_actor=[&](auto& archive,auto& frames,const char* library,int first) {
            if (library && !archive.open(resource_root/"Data"/library)) return;
            for (int i=0;i<int(frames.size());++i) {
                if (const auto frame=archive.frame(first+i))
                    frames[i]={texture(frame->pixels.rgba,frame->width,frame->height),Vector2(frame->offset_x,frame->offset_y),frame->pixels.regions,frame->shadow_mode,Vector2(frame->shadow_x,frame->shadow_y)};
            }
        };
        load_actor(map_human,map_human_frames,"M-Hum",0);
        load_actor(map_human,map_walk_frames,nullptr,1680);
        load_actor(map_human,map_run_frames,nullptr,1760);
        mir3::WilArchive progress;
        if (progress.open(resource_root/"Data"/"ProgUse")) {
            if (const auto frame=progress.frame(2)) health_border=texture(frame->pixels.rgba,frame->width,frame->height);
            if (const auto frame=progress.frame(3)) health_fill=texture(frame->pixels.rgba,frame->width,frame->height);
            for (int i=0;i<int(ground_sparkles.size());++i)
                if (const auto frame=progress.frame(40+i)) ground_sparkles[i]=texture(frame->pixels.rgba,frame->width,frame->height);
        }
        scene_health.try_emplace(0);
        ground_archive.open(resource_root/"Data"/"Ground");
        map_horse.open(resource_root/"Data"/"Horse");
        map_female.open(resource_root/"Data"/"WM-Hum");
        human_bodies.emplace(std::pair{0,0},HumanBody{map_human_frames,map_walk_frames,map_run_frames});
        map_npc.open(resource_root/"Data"/"NPC");
        map_flag.open(resource_root/"Data"/"Flag");
        set_scene_npc(2,Vector2i(413,409),110);
        for (const int sample:{1,33,34,35})
            movement_sounds.emplace(sample,AudioStreamWAV::load_from_file(audio_root.path_join(String::num_int64(sample)+".wav")));
        const auto footstep=movement_sounds.at(1);
        if (footstep.is_valid()) {
            footstep_sound=memnew(AudioStreamPlayer);
            footstep_sound->set_name("FootstepSound");
            footstep_sound->set_stream(footstep);
            add_child(footstep_sound);
            auto* audio=AudioServer::get_singleton();
            const int bus=audio->get_bus_count();
            footstep_bus=String("Mir3Footsteps")+String::num_uint64(get_instance_id());
            audio->add_bus(bus);
            audio->set_bus_name(bus,footstep_bus);
            Ref<AudioEffectPanner> panner;
            panner.instantiate();
            audio->add_bus_effect(bus,panner);
            footstep_sound->set_bus(footstep_bus);
            sync_hud_sound();
        }
        const Vector2i spawn=get_meta("map_spawn_cell",player_cell);
        if (!scene_occupied(spawn) && terrain.walkable(spawn.x,spawn.y)) player_cell=spawn;
    }
    if (!portraits.open(resource_root/"Data"/"ProgUse")) UtilityFunctions::push_error("Cannot read ProgUse portraits.");
    if (!monster_portraits.open(resource_root/"Data"/"MonImg")) UtilityFunctions::push_error("Cannot read MonImg portraits.");
    load_status_body();
    minimap_archive.open(resource_root/"Data"/"Mmap");
    full_minimap_archive.open(resource_root/"Data"/"Fmmap");
    magic_icons.open(resource_root/"Data"/"MIcon");
    equipment_archive.open(resource_root/"Data"/"Equip");
    std::ifstream descriptions(resource_root/"Magic.exp",std::ios::binary);
    const std::vector<std::uint8_t> description_bytes{std::istreambuf_iterator<char>(descriptions),{}};
    skill_descriptions.load(description_bytes);
    item_archive.open(resource_root/"Data"/"Inventory");
    std::ifstream message_file(resource_root/"CmList.dat",std::ios::binary);
    std::vector<std::uint8_t> message_bytes{std::istreambuf_iterator<char>(message_file),{}};
    if (!messages.load(message_bytes)) UtilityFunctions::push_error("Cannot read CmList.dat captions.");
    chat_input=memnew(Mir3ChatInput);
    add_child(chat_input);
    chat_input->configure(String::utf8(message(mir3::ChatInputSpec::font_message).c_str()));
    chat_input->connect("chat_requested",callable_mp(this,&Mir3HudView::chat_requested));
    // 0x47F3F0 mode 0: legacy rectangles use ONE / INVSRCALPHA.
    // Textures use premultiplied alpha. Alpha rectangles are submitted as polygons.
    Ref<Shader> shader;shader.instantiate();
    shader->set_code("shader_type canvas_item; render_mode unshaded, blend_premul_alpha; varying vec4 source_modulate; void vertex() { source_modulate=COLOR; } void fragment() { if (UV.x < -1.0) { COLOR=source_modulate*texture(TEXTURE,UV+vec2(2.0)); } else { COLOR.rgb *= COLOR.a; } }");
    Ref<ShaderMaterial> material;material.instantiate();material->set_shader(shader);set_material(material);
    terrain_layer=memnew(Node2D);
    terrain_layer->set_material(material);
    add_child(terrain_layer);
    const Ref<Shader> monster_mix=ResourceLoader::get_singleton()->load("res://monster_mix.gdshader");
    monster_shadow_mix.instantiate();
    monster_shadow_mix->set_shader(monster_mix);
    monster_body_mix.instantiate();
    monster_body_mix->set_shader(monster_mix);
    monster_body_mix->set_shader_parameter("source_weight",8.0);
    actor_highlight.instantiate();
    actor_highlight->set_shader(monster_mix);
    actor_highlight->set_shader_parameter("highlight_opaque",true);
    for (int weight=0;weight<32;++weight) {
        monster_appearance_mix[weight].instantiate();
        monster_appearance_mix[weight]->set_shader(monster_mix);
        monster_appearance_mix[weight]->set_shader_parameter("source_weight",double(weight));
    }
    monster_tint_appearance_mix.instantiate();
    monster_tint_appearance_mix->set_shader(monster_mix);
    monster_tint_appearance_mix->set_shader_parameter("combine_before_floor",true);
    mir3::WilArchive weather_archive;
    if(weather_archive.open(resource_root/"Data"/"ProgUse")) {
        if(const auto source=weather_archive.frame(500)) snow_texture=texture(source->pixels.rgba,source->width,source->height);
    }
    for(const int id:{200,201,202,203,204,205,206,207,210,220,230,231,232,233,234,
            240,241,242,243,244,245,246,247,260,261,520,521,522,523,530}) {
        if(const auto source=weather_archive.frame(id))
            ground_event_frames.emplace(id,MapActorFrame{
                texture(source->pixels.rgba,source->width,source->height),
                Vector2(source->offset_x,source->offset_y),{}});
    }
    fog_copy=memnew(BackBufferCopy);fog_copy->set_copy_mode(BackBufferCopy::COPY_MODE_VIEWPORT);add_child(fog_copy);
    fog_sprite=memnew(Sprite2D);fog_sprite->set_centered(false);fog_copy->add_child(fog_sprite);
    if(const auto source=weather_archive.frame(550)) fog_sprite->set_texture(texture(source->pixels.rgba,source->width,source->height));
    Ref<ShaderMaterial> fog_material;fog_material.instantiate();
    fog_material->set_shader(ResourceLoader::get_singleton()->load("res://level_effect.gdshader"));fog_sprite->set_material(fog_material);
    fog_sprite->set_modulate(Color(100.f/255,200.f/255,200.f/255));fog_copy->hide();
    snow_layer=memnew(Node2D);snow_layer->set_name("SnowWeather");add_child(snow_layer);
    Ref<Shader> snow_shader;snow_shader.instantiate();
    snow_shader->set_code("shader_type canvas_item; render_mode unshaded, blend_premul_alpha; varying vec3 tint; void vertex(){tint=COLOR.rgb;} void fragment(){vec4 s=texture(TEXTURE,UV); COLOR=vec4(s.rgb*tint*s.a,s.a/255.0);}");
    Ref<ShaderMaterial> snow_material;snow_material.instantiate();snow_material->set_shader(snow_shader);snow_layer->set_material(snow_material);
    snow_layer->connect("draw",callable_mp(this,&Mir3HudView::draw_snow));
    rain_copy=memnew(BackBufferCopy);
    rain_copy->set_copy_mode(BackBufferCopy::COPY_MODE_VIEWPORT);
    add_child(rain_copy);
    rain_layer=memnew(Node2D);
    rain_layer->set_name("RainWeather");
    rain_copy->add_child(rain_layer);
    rain_layer->set_material(fog_material);
    rain_layer->connect("draw",callable_mp(this,&Mir3HudView::draw_rain));
    rain_textures[0]=snow_texture;
    for(int frame=1;frame<int(rain_textures.size());++frame) {
        if(frame>=5 && frame<10) continue;
        if(const auto source=weather_archive.frame(500+frame))
            rain_textures[frame]=texture(source->pixels.rgba,source->width,source->height);
    }
    rain_copy->hide();
    weather.random_state=std::uint32_t(ticks_msec());
    scene_light_layer=memnew(Node2D);
    Ref<Shader> light_shader;light_shader.instantiate();
    light_shader->set_code("shader_type canvas_item; render_mode unshaded, blend_mul;");
    Ref<ShaderMaterial> light_material;light_material.instantiate();light_material->set_shader(light_shader);
    scene_light_layer->set_material(light_material);
    add_child(scene_light_layer);
    scene_light_layer->connect("draw",callable_mp(this,&Mir3HudView::draw_scene_lighting));
    level_layer=memnew(Node2D);add_child(level_layer);
    spell_particle_layer=memnew(Node2D);add_child(spell_particle_layer);
    for (int i=0;i<2;++i) {
        auto& particle_material=spell_particle_materials[i];
        particle_material.instantiate();
        particle_material->set_shader(ResourceLoader::get_singleton()->load("res://spell_particle.gdshader"));
        particle_material->set_shader_parameter("glow",i==1);
    }
    // 42499C renders ground sparkles after world labels and before the interface.
    world_overlay_layer=memnew(Node2D);
    world_overlay_layer->set_material(material);
    add_child(world_overlay_layer);
    world_overlay_layer->connect("draw",callable_mp(this,&Mir3HudView::draw_world_overlays));
    ground_sparkle_layer=memnew(Node2D);
    Ref<Shader> sparkle_shader;sparkle_shader.instantiate();
    // 49294C/492956 use ONE and INVSRCALPHA for the ground sparkle draw.
    // 424D02 and 49291C modulate texture alpha by the material's 1/255 before ONE/INVSRCALPHA blending.
    sparkle_shader->set_code("shader_type canvas_item; render_mode unshaded, blend_premul_alpha; void fragment(){ COLOR.a /= 255.0; }");
    Ref<ShaderMaterial> sparkle_material;sparkle_material.instantiate();
    sparkle_material->set_shader(sparkle_shader);
    ground_sparkle_layer->set_material(sparkle_material);
    add_child(ground_sparkle_layer);
    ground_sparkle_layer->connect("draw",callable_mp(this,&Mir3HudView::draw_ground_sparkles));
    hud_base=memnew(Node2D);
    hud_base->set_material(material);
    add_child(hud_base);
    hud_base->connect("draw",callable_mp(this,&Mir3HudView::draw_hud_base));
    hud_copy=memnew(BackBufferCopy);
    hud_copy->set_copy_mode(BackBufferCopy::COPY_MODE_VIEWPORT);
    add_child(hud_copy);
    hud_copy->hide();
    hud_decoration=memnew(Sprite2D);
    hud_decoration->set_centered(false);
    hud_decoration->set_position(point(mir3::HudLayout::decoration_position));
    Ref<ShaderMaterial> dye;
    dye.instantiate();
    dye->set_shader(ResourceLoader::get_singleton()->load("res://hud_dye.gdshader"));
    hud_decoration->set_material(dye);
    add_child(hud_decoration);
    hud_decoration->hide();
    hud_tail=memnew(Node2D);
    hud_tail->set_material(material);
    add_child(hud_tail);
    hud_tail->connect("draw",callable_mp(this,&Mir3HudView::draw_hud_tail));

    exit_layer=memnew(Node2D);
    exit_layer->set_material(light_material);
    exit_layer->set_z_index(100);
    add_child(exit_layer);
    exit_layer->connect("draw",callable_mp(this,&Mir3HudView::draw_exit_transition));
    for (int i=0;i<23;++i) {
        auto image=frame(mir3::HudLayout::buttons[i][0]);
        if (image.is_valid()) model.button_sizes[i]={image->get_width(),image->get_height()};
    }
    if (!bool(get_meta("offline_preview",false))) {
        if (!open_server_groups() && !open_login()) UtilityFunctions::push_error("Cannot load login configuration or interface resources.");
        return;
    }
    UtilityFunctions::print("HUD ready: 800x600; original WIL resources; C++ MVC.");
    // A restored map request can receive a synchronous reply, so dispatch it
    // after the map archives and the HUD have been initialized.
    if (terrain_ready)
        set_map_location(String::utf8("\xe6\xaf\x94\xe5\xa5\x87\xe7\x9c\x81"),player_cell.x,player_cell.y);
    else if (portrait_map_pending)
        emit_signal("minimap_requested",mir3::protocol::CM_MINIMAP);
    queue_redraw();
}
void Mir3HudView::_exit_tree() {
    auto* renderer=RenderingServer::get_singleton();
    for (const auto& item:terrain_draw_items) renderer->free_rid(item);
    terrain_draw_items.clear();
    for (const auto& item:spell_particle_items) renderer->free_rid(item);
    spell_particle_items.clear();
    if(!login_visible && !character_selection.visible && !exit_transition.active) save_item_layout();
    connection.close();login_movie.close();
    const auto stop=[](AudioStreamPlayer* player) {
        if (player) { player->stop();player->set_stream(Ref<AudioStream>()); }
    };
    stop(login_movie_audio);stop(map_music);stop(hud_sound);stop(gold_sound);stop(footstep_sound);
    for (auto& [file,player]:login_effects) stop(player);
    while (!actor_sounds.empty()) remove_actor_sound(actor_sounds.begin()->first);
    if (footstep_bus==StringName()) return;
    footstep_sound->set_bus("Master");
    auto* audio=AudioServer::get_singleton();
    const int bus=audio->get_bus_index(footstep_bus);
    if (bus>=0) audio->remove_bus(bus);
    footstep_bus=StringName();
}
Ref<ImageTexture> Mir3HudView::frame(int id) {
    if (const auto found=frames.find(id);found!=frames.end()) return found->second;
    auto decoded=archive.frame(id);
    return frames.emplace(id,decoded?texture(decoded->pixels.rgba,decoded->width,decoded->height):Ref<ImageTexture>{}).first->second;
}
void Mir3HudView::sprite(int id,mir3::HudLayout::Point position) {
    auto image=frame(id);
    if (image.is_valid()) paint_canvas->draw_texture(image,point(position));
}
void Mir3HudView::text(const std::string& value,mir3::HudLayout::Rect r,unsigned color,mir3::TextFont font,bool centered,bool inverted) {
    if (value.empty()) return;
    const auto key=std::to_string(r[2])+":"+std::to_string(r[3])+":"+std::to_string(int(font))+":"+std::to_string(centered)+":"+value+(inverted?"|inv":"|normal");
    auto found=text_cache.find(key);
    if (found==text_cache.end()) {
        if (text_cache.size()>=256) text_cache.clear();
        const auto bitmap=mir3::raster_text(value,r[2],r[3],centered,font,messages.get(1074),inverted);
        found=text_cache.emplace(key,texture(bitmap.rgba,r[2],r[3])).first;
    }
    if (found->second.is_valid()) paint_canvas->draw_texture(found->second,Vector2(r[0],r[1]),Color(float(color&255)/255,float((color>>8)&255)/255,float((color>>16)&255)/255));
}
void Mir3HudView::buttons() {
    for (int i=mir3::HudLayout::count(model.layout.expanded)-1;i>=0;--i) {
        const auto button=mir3::HudLayout::button(i,model.layout.expanded);
        auto layers=mir3::main_button_frames(i,model.hovered==i,model.pressed==i);
        if (chat_input && chat_input->mode.changed && (i==18 || i==19)) {
            const int id=chat_input->mode.frame(i);
            layers=mir3::button_frames({id,id,id,false},model.pressed==i?2:model.hovered==i?1:0);
        }
        if (i>=20 && i<=22 && reception_buttons.changed[i-20]) {
            const int id=reception_buttons.frames[i-20];
            layers=mir3::button_frames({id,id,id,false},model.pressed==i?2:model.hovered==i?1:0);
        }
        for (auto id:layers)
            if (id>=0) sprite(id,{button[1],button[2]});
    }
}
Ref<ImageTexture> Mir3HudView::portrait_texture(mir3::WilArchive& archive,
        std::map<int,Ref<ImageTexture>>& cache,int id) {
    if (id<0) return {};
    if (const auto it=cache.find(id); it!=cache.end()) return it->second;
    const auto decoded=archive.frame(id);
    return cache.emplace(id,decoded?texture(decoded->pixels.rgba,decoded->width,decoded->height):Ref<ImageTexture>{}).first->second;
}

void Mir3HudView::set_hovered_target(std::uint32_t actor,int type,int shape,bool excluded,int state,int hp,int max_hp,int percent,
        Vector2i cell) {
    const bool changed=target_focus.changes_to(actor);
    hovered_target=actor;
    target_focus.update(actor,static_cast<std::uint32_t>(ticks_msec()));
    if (actor) {
        target_is_human=false;
        target_custom_portrait.unref();
        target_frame=mir3::TargetPortrait::monster_frame(static_cast<std::uint8_t>(type),
            static_cast<std::uint16_t>(shape),excluded);
        target_portrait=portrait_texture(monster_portraits,monster_portrait_frames,target_frame);
        target_frame=mir3::TargetPortrait::fallback_frame(target_frame,target_portrait.is_valid());
        if (target_portrait.is_null()) target_portrait=portrait_texture(monster_portraits,monster_portrait_frames,target_frame);
        // 0x43823D..0x438269 skips both the monster image and its badge.
        target_badge=target_frame<0?-1:mir3::TargetPortrait::badge(static_cast<std::uint16_t>(shape),state);
        target_special_shape=mir3::TargetPortrait::special_shape(static_cast<std::uint16_t>(shape));
        target_vitals={static_cast<std::uint8_t>(percent),static_cast<std::uint16_t>(max_hp),static_cast<std::uint16_t>(hp)};
        // 0x4248F7 / 0x47BEC0 requests missing attributes on a focus change.
        if (changed && cell.x>=0 && cell.x<=65535 && cell.y>=0 && cell.y<=65535) {
            const auto& attributes=target_attributes_by_actor.try_emplace(actor).first->second;
            if (attributes.icon==-1)
                emit_signal("target_attributes_requested",mir3::protocol::CM_TARGET_ATTRIBUTES,actor,cell);
        }
    }
    queue_redraw();
}

bool Mir3HudView::apply_target_attributes(std::uint32_t actor,int category,int state,PackedByteArray body) {
    const bool displayed_monster=actor==target_focus.displayed() && !target_is_human;
    if (!actor || (!displayed_monster && !target_attributes_by_actor.contains(actor)) || body.size()!=10 ||
        category<0 || category>65535 || state<0 || state>65535) return false;
    // Decoded 1304 body at actor+0x61D18. The second byte is unused here.
    auto& target_attributes=target_attributes_by_actor[actor];
    target_attributes.category=category;
    target_attributes.state=state;
    target_attributes.icon=static_cast<std::int8_t>(body[0]);
    target_attributes.element=body[2];
    for (int i=0;i<7;++i) target_attributes.modifiers[i]=static_cast<std::int8_t>(body[i+3]);
    queue_redraw();
    return true;
}

void Mir3HudView::set_hovered_human(std::uint32_t actor,int type,int profession,String name) {
    if (!actor || type<0 || type>1 || profession<0 || profession>255) return;
    if (!target_is_human || target_focus.displayed()!=actor) {
        target_custom_portrait.unref();
        target_portrait_query_name=String();
    }
    hovered_target=actor;
    target_focus.update(actor,static_cast<std::uint32_t>(ticks_msec()));
    target_is_human=true;
    target_badge=-1;
    target_frame=mir3::TargetPortrait::human_frame(static_cast<std::uint8_t>(profession),static_cast<std::uint8_t>(type));
    target_portrait=portrait_texture(portraits,portrait_frames,target_frame);
    if (!name.is_empty() && target_portrait_query_name!=name) {
        target_portrait_query_name=name;
        // 0x47D3A0 queries the character name. The actor is local reply context.
        emit_signal("target_portrait_requested",mir3::protocol::CM_TARGET_PORTRAIT,actor,name);
    }
    queue_redraw();
}

bool Mir3HudView::apply_target_portrait(std::uint32_t actor,PackedByteArray jpeg) {
    if (!actor || !target_is_human || target_focus.displayed()!=actor || jpeg.size()>4096) return false;
    if (jpeg.is_empty()) {
        target_custom_portrait.unref();
    } else {
        Ref<Image> image;
        image.instantiate();
        if (image->load_jpg_from_buffer(jpeg)!=OK) return false;
        target_custom_portrait=ImageTexture::create_from_image(image);
    }
    queue_redraw();
    return true;
}

void Mir3HudView::draw_target_vitals() {
    using Layout=mir3::HudLayout;
    if (model.layout.expanded || !target_focus.displayed() || target_is_human || target_portrait.is_null()) return;
    for (int i=0;i<3;++i)
        text(target_vitals.text(i),mir3::TargetPortraitVitals::text_rect(i),0xe6e6e6);
    const auto border=portrait_texture(portraits,portrait_frames,660);
    if (border.is_valid()) paint_canvas->draw_texture(border,point(Layout::image_position(Layout::Image::target_hp_border)));
    const auto fill=portrait_texture(portraits,portrait_frames,661);
    if (fill.is_valid()) {
        const int width=std::min(target_vitals.bar_width(),fill->get_width());
        if (width>0) {
            const Rect2 source(0,0,width,fill->get_height());
            paint_canvas->draw_texture_rect_region(fill,Rect2(point(Layout::image_position(Layout::Image::target_hp_fill)),source.size),source);
        }
    }
}

Ref<ImageTexture> Mir3HudView::terrain_texture(int package,int id) {
    if (package>=int(terrain_archives.size()) || id>=65535) return {};
    const auto key=std::pair{package,id};
    if (const auto found=terrain_frames.find(key);found!=terrain_frames.end()) return found->second;
    const auto decoded=terrain_archives[package].frame(id);
    return terrain_frames.emplace(key,decoded?texture(decoded->pixels.rgba,decoded->width,decoded->height):Ref<ImageTexture>{}).first->second;
}

Vector2i Mir3HudView::map_camera() const {
    if (has_meta("map_camera_cell")) {
        const Vector2i cell=get_meta("map_camera_cell");
        return {cell.x*48,cell.y*32};
    }
    const auto focus=walk_frame>=0?step_origin:player_cell;
    Vector2i camera((focus.x-12)*48+200,(focus.y-12)*32+157);
    if (walk_frame>=0) {
        constexpr std::array xs{8,16,24,32,40,48}, ys{5,11,16,21,27,32};
        const auto step=step_destination-step_origin;
        camera+=Vector2i(step.x*xs[walk_frame],step.y*ys[walk_frame]);
    }
    return camera;
}

Vector2i Mir3HudView::player_pixel() const {
    Vector2i pixel(player_cell.x*48,player_cell.y*32);
    if (walk_frame>=0) {
        const int direction=forced_motion?forced_motion->direction():player_facing;
        const bool committed=walk_frame>=(direction==7?4:1);
        const int frames=committed?walk_frame-5:walk_frame+1;
        const auto step=step_destination-step_origin;
        const int y_step=committed && direction%2!=0?32*step.y/6:5*step.y;
        pixel+=Vector2i(step.x*8*frames,y_step*frames);
    }
    return pixel;
}

void Mir3HudView::start_map_step(Vector2i destination) {
    if (player_life.dead || pending_local_action || action_prediction.waiting || !local_actor_messages.empty()) return;
    const auto delta=destination-player_cell;
    const mir3::MapCell direction{(delta.x>0)-(delta.x<0),(delta.y>0)-(delta.y<0)};
    const int facing=int(std::find(mir3::map_directions.begin(),mir3::map_directions.end(),direction)-mir3::map_directions.begin());
    const bool running=std::max(std::abs(delta.x),std::abs(delta.y))>1;
    pending_local_action=PendingLocalAction{true,player_cell,destination,facing,player_weapon,
        running?mir3::protocol::CM_RUN:mir3::protocol::CM_WALK,{}};
    advance_local_action();
}

void Mir3HudView::request_melee_action(int facing,int opcode) {
    if (player_life.dead || pending_local_action || action_prediction.waiting) return;
    const auto now=std::uint32_t(GetTickCount());
    if (!melee_cooldown.accept(now,model.level,model.hand_weight[0],model.hand_weight[1])) return;
    map_route.clear();
    pending_local_action=PendingLocalAction{false,player_cell,player_cell,facing,player_weapon,opcode,{}};
    advance_local_action();
}

void Mir3HudView::advance_local_action() {
    if (!pending_local_action) return;
    const auto now=std::uint32_t(GetTickCount());
    auto& pending=*pending_local_action;
    if (!terrain_ready || player_cell!=pending.origin || (model.max_hp && model.hp==0)) {
        pending_local_action.reset();
        return;
    }
    if (!pending.waiting_since) {
        if (pending.movement) {
            if (!local_action_clock.admit_move(now)) return;
            if (!spell_timing.movement_ready(now)) {
                pending_local_action.reset();
                return;
            }
        }
        pending.waiting_since=now;
        pending.wait_ms=local_action_clock.shared_wait(now,
            mir3::MeleeCooldown::interval(model.level,melee_cooldown.speed,model.hand_weight[0],model.hand_weight[1]));
    }
    if (mir3::LocalActionClock::remaining(now,*pending.waiting_since,pending.wait_ms)) return;
    auto action=pending;
    pending_local_action.reset(); // 先消费，再发信号，防止回调重复执行。
    forced_motion.reset();
    action_prediction.begin({player_cell.x,player_cell.y},player_facing);
    player_facing=action.facing;
    player_harvest_impact=false;
    if (action.movement) {
        local_action_clock.moved=now;
        step_origin=action.origin;
        step_destination=action.destination;
        walk_frame=0;
        movement_ms=0;
        player_action={};
    } else {
        local_action_clock.attacked=now;
        const auto step=mir3::map_directions[action.facing];
        const auto distant=player_cell+Vector2i(step[0],step[1])*2;
        const auto at_distant=[&](const auto& entry) {return entry.second.cell==distant;};
        const bool distant_actor=(std::any_of(scene_humans.begin(),scene_humans.end(),at_distant) ||
             std::any_of(scene_chickens.begin(),scene_chickens.end(),at_distant) ||
             std::any_of(scene_npcs.begin(),scene_npcs.end(),at_distant));
        mir3::MeleeSpells::Attack selected{0,action.opcode,mir3::HumanActions::weapon_attack(action.weapon),action.facing,0};
        if (action.opcode==mir3::protocol::CM_HIT)
            selected=melee_spells.select(action.weapon,action.facing,model.mp,distant_actor,player_type);
        action.opcode=selected.opcode;
        player_melee_sound=selected.sound;
        player_action.start(selected.action);
        player_spell={};
        // 415F90普通挥击使用装备槽原包选择附魔，40DB10再确定颜色优先级。
        const auto& weapon=equipment.slots[1];
        const int effect=selected.skill?selected.skill:weapon?mir3::SpellColor::local_weapon_effect(weapon.server_record):0;
        if (const auto* resource=effect_catalog.find(mir3::EffectCatalog::Section::spell,std::uint16_t(effect));effect && resource) {
            player_spell={effect,selected.facing};
            player_spell.weapon=true;
            player_spell.tint=mir3::SpellColor::local_weapon_color(weapon.server_record,elements.attack_words,resource->tint);
            play_spell_sound(10*(effect+1000),player_cell);
        }
    }
    const auto tick=action.movement?local_action_clock.move_admitted:now;
    const auto position=std::uint32_t(action.destination.x)|(std::uint32_t(action.destination.y)<<16);
    const auto parameters=(tick&0xffffu)|(std::uint32_t(action.facing)<<16);
    if (action.movement && action.opcode==mir3::protocol::CM_WALK) {
        // 41603D scans the eight neighbors of the walking destination in direction order.
        for (const auto [x,y]:mir3::map_directions)
            if (request_map_door(action.destination+Vector2i(x,y))) break;
    } else if (action.movement && action.opcode==mir3::protocol::CM_RUN) {
        // 4161AF starts at the first step and checks each ray up to the run length.
        const auto [dx,dy]=mir3::map_directions[action.facing];
        const auto first=action.origin+Vector2i(dx,dy);
        const auto delta=action.destination-action.origin;
        const int distance=std::max(std::abs(delta.x),std::abs(delta.y));
        for (const auto [x,y]:mir3::map_directions)
            for (int step=1;step<=distance;++step)
                if (request_map_door(first+Vector2i(x,y)*step)) break;
    }
    send_client_packet(action.opcode,position,String(),parameters,tick>>16);
    queue_redraw();
}

bool Mir3HudView::request_map_door(Vector2i cell) {
    if (!terrain.door_closed(cell.x,cell.y)) return false;
    const auto coordinates=std::uint32_t(cell.x)|(std::uint32_t(cell.y)<<16);
    send_client_packet(mir3::protocol::CM_OPENDOOR,terrain.door_id(cell.x,cell.y),String(),coordinates);
    return true;
}

bool Mir3HudView::walk_to_map_cell(Vector2i destination) {
    if (player_life.dead || !terrain_ready || pending_local_action || action_prediction.waiting || !local_actor_messages.empty()) return false;
    const auto start=walk_frame>=0?step_destination:player_cell;
    map_route=terrain.route({start.x,start.y},{destination.x,destination.y},
        [this](mir3::MapCell cell) { return scene_occupied(Vector2i(cell[0],cell[1])); });
    return !map_route.empty();
}

bool Mir3HudView::apply_npc_message(PackedByteArray encoded,PackedByteArray key) {
    if (server_groups.visible || login_visible || character_selection.visible) return false;
    constexpr int header_size=16;
    if (key.size()!=4 || encoded.size()<header_size || encoded.size()>header_size+(10240*4+2)/3) return false;
    const auto header=mir3::decode_six_bit(encoded.ptr(),header_size);
    if (!header) return false;
    const auto b=[&](int index) { return std::uint32_t(std::uint8_t((*header)[index])); };
    // 0x428308..0x428399 reconstructs these fields using the connection key.
    const int opcode=(b(0)^key[0])|((b(4)^key[2]^key[1])<<8);
    const auto actor=(b(5)^key[3])|(b(7)<<8)|(b(9)<<16)|(b(11)<<24);
    const int first_parameter=b(1)|(b(6)<<8);
    if (opcode==mir3::protocol::SM_SENDDETAILGOODSLIST) {
        // 0x42A8B6 / 0x4796A0: count and offset precede a nested six-bit item list.
        const int offset=b(2)|(b(8)<<8);
        if (first_parameter>255 || shop_selected<0) return false;
        const auto list=mir3::decode_six_bit(encoded.ptr()+header_size,encoded.size()-header_size);
        if (!list || list->size()>8196) return false;
        std::string_view remaining=*list;
        PackedStringArray names;
        PackedInt32Array icons, prices;
        PackedInt64Array identifiers;
        for (int i=0;i<first_parameter;++i) {
            const auto delimiter=remaining.find('/');
            if (delimiter==remaining.npos) return false;
            const auto item=mir3::decode_six_bit(reinterpret_cast<const std::uint8_t*>(remaining.data()),delimiter);
            if (!item || item->size()!=100) return false;
            const auto byte=[&](int position) { return std::uint32_t(std::uint8_t((*item)[position])); };
            const auto end=item->find(char(0));
            if (end==0 || end>=30) return false;
            names.push_back(String::utf8(mir3::ansi_to_utf8(std::string_view(*item).substr(0,end)).c_str()));
            // 0x4420F0 copies the wire item into the object at +4.
            icons.push_back(byte(36)|(byte(37)<<8));
            prices.push_back(byte(68)|(byte(69)<<8));
            identifiers.push_back(byte(62)|(byte(63)<<8)|(byte(64)<<16)|(byte(65)<<24));
            remaining.remove_prefix(delimiter+1);
        }
        if (!remaining.empty()) return false;
        return apply_shop_offers(shop_actor,shop_selected,names,icons,prices,identifiers,offset);
    }
    if (opcode==mir3::protocol::SM_BUYITEM_SUCCESS || opcode==mir3::protocol::SM_BUYITEM_FAIL)
        return encoded.size()==header_size && apply_shop_purchase_reply(opcode,actor);
    if (actor>INT_MAX) return false;
    return apply_npc_body(opcode,static_cast<int>(actor),first_parameter,encoded.slice(header_size));
}

bool Mir3HudView::apply_npc_body(int opcode,int actor,int count,PackedByteArray encoded) {
    if (opcode==mir3::protocol::SM_MERCHANTDLGCLOSE) return encoded.is_empty() && apply_npc_dialogue(opcode,actor,String());
    const int limit=opcode==mir3::protocol::SM_MERCHANTSAY?8192:opcode==mir3::protocol::SM_SENDGOODSLIST?10240:0;
    if (!limit || encoded.size()>(limit*4+2)/3) return false;

    const auto decoded=mir3::decode_six_bit(encoded.ptr(),encoded.size());
    if (!decoded) return false;
    const auto& ansi=*decoded;
    const auto end=ansi.find(char(0));
    const auto body=String::utf8(mir3::ansi_to_utf8(std::string_view(ansi).substr(0,end)).c_str());
    return opcode==mir3::protocol::SM_MERCHANTSAY?apply_npc_dialogue(opcode,actor,body):apply_shop_goods(actor,count,body,true);
}

bool Mir3HudView::apply_npc_dialogue(int opcode,int actor,String body) {
    if (opcode==mir3::protocol::SM_MERCHANTDLGCLOSE) {
        close_npc_dialogue();
        return true;
    } else {
        if (opcode!=mir3::protocol::SM_MERCHANTSAY || actor<=0) return false;
        const auto utf8=body.utf8();
        const std::string_view message(utf8.get_data(),utf8.length());
        if (message.size()>8192*4 || mir3::utf8_to_ansi(message).size()>8192) return false;
        auto script=message.substr(0,message.find(char(0)));
        const auto slash=script.find('/');
        if (slash==script.npos) return false;
        script.remove_prefix(slash+1);
        std::vector<NpcDialogueRow> rows;
        constexpr std::array<unsigned,17> colors{0,0xff,0x8000,0x8080,0x808080,0x80,0x808000,0x800000,
            0xc0c0c0,0x800080,0xff00,0xff0000,0xffffff,0xff00ff,0xffff00,0xffff,0x10101};
        unsigned color=0xffffff;
        // 0x460120 restores the close action when the window is dismissed.
        bool can_close=npc_dialogue_actor!=actor || npc_dialogue_can_close;
        int line=0;
        int estimated_lines=0;
        int option_id=0;
        while (!script.empty()) {
            int x=0, ansi_length=0;
            const auto append=[&](std::string_view label,std::string_view command={}) {
                ansi_length+=int(mir3::utf8_to_ansi(label).size());
                const int option=command.empty()?-1:option_id++;
                while (!label.empty()) {
                    std::size_t length=0;
                    int width=0, height=0;
                    const int limit=command.empty() && line<7?198:305;
                    const auto whole=label.size()<=1024?mir3::raster_text(label,1,1,false,mir3::TextFont::normal):mir3::NativeText{};
                    const bool fits=x+whole.text_width<=limit && whole.text_height>0;
                    if (fits) { length=label.size(); width=whole.text_width; height=whole.text_height; }
                    while (length<label.size()) {
                        auto next=length+1;
                        while (next<label.size() && (static_cast<unsigned char>(label[next])&0xc0)==0x80) ++next;
                        const auto measured=mir3::raster_text(label.substr(0,next),1,1,false,mir3::TextFont::normal);
                        length=next;
                        width=measured.text_width;
                        height=measured.text_height;
                        if (x+width>=305) break;
                    }
                    rows.push_back({std::string(label.substr(0,length)),std::string(command),{35+x,line,width,height},color,option});
                    label.remove_prefix(length);
                    if (!fits && x+width>=305) { ++line; x=0; }
                    else x+=width;
                }
            };
            // 0x45F5F0 scans for a line break only outside complete tags.
            while (!script.empty()) {
                const auto open=script.find_first_of("\\<{");
                append(script.substr(0,open));
                if (open==script.npos) { script={}; break; }
                script.remove_prefix(open);
                if (script.front()==char(92)) break;
                const bool option=script.front()=='<';
                const auto close=script.find(option?'>':'}',1);
                if (close==script.npos) { script={}; break; }
                const auto tag=script.substr(1,close-1);
                const auto split=tag.find('/');
                if (split!=tag.npos) {
                    const auto name=tag.substr(0,split);
                    const auto value=tag.substr(split+1);
                    if (option) append(name,value);
                    else if (name=="NOTCLOSE") can_close=false;
                    else if (name=="FCOLOR") {
                        int index=0;
                        const auto parsed=std::from_chars(value.data(),value.data()+value.size(),index);
                        if (parsed.ec==std::errc{} && index>=0 && index<int(colors.size())) color=colors[index];
                    }
                }
                script.remove_prefix(close+1);
            }
            if (ansi_length || !script.empty()) estimated_lines+=std::max(1,(ansi_length+49)/50);
            if (script.empty()) break;
            script.remove_prefix(1);
            ++line;
        }
        if (npc_dialogue_actor && npc_dialogue_actor!=actor) close_npc_dialogue();
        npc_dialogue_rows=std::move(rows);
        npc_dialogue_extra_rows=std::clamp(estimated_lines-9,0,13);
        npc_dialogue_first=0;
        // 0x461730 enables scrolling by ANSI line estimate; 0x460F01 records the actual last row.
        npc_dialogue_scroll_enabled=estimated_lines>22;
        npc_dialogue_scroll_limit=npc_dialogue_scroll_enabled?std::max(0,line-21):0;
        npc_dialogue_arrow_pressed=0;
        if (npc_dialogue_actor!=actor) {
            map_pointer_button=0;
            npc_dialogue_arrow_frame=0;
            npc_dialogue_down_lit=false;
        }
        npc_dialogue_can_close=can_close;
        npc_dialogue_close_pressed=false;
        npc_dialogue_actor=actor;
        npc_dialogue_origin=player_cell;
    }
    npc_dialogue_pressed=-1;
    queue_redraw();
    return true;
}

void Mir3HudView::set_connection_enabled(bool enabled) {
    connection_enabled=enabled;
    if (!enabled) { connection.close();connection_failed=false;connection_retry_at=0; }
}

void Mir3HudView::connect_page(ConnectionPage page,const mir3::LoginEndpoint& endpoint) {
    connection.close();connection_page=page;connection_endpoint=endpoint;
    connection_failed=false;connection_retry_at=0;outgoing_messages.sequence=0;
    if (connection_enabled) connection_failed=!connection.open(String::utf8(endpoint.host.c_str()),endpoint.port);
}

void Mir3HudView::poll_connection(double delta) {
    if (!connection_enabled) return;
    if (connection_retry_at) {
        if (ticks_msec()>=connection_retry_at) {
            connection_retry_at=0;
            connection_failed=!connection.open(String::utf8(connection_endpoint.host.c_str()),connection_endpoint.port);
        }
        return;
    }
    if (connection_failed) {
        connection_failed=false;
        // 0x47C430：系统Yes/No框；确认后等待2000ms重连，取消退出。
        const auto caption=mir3::utf8_to_ansi(message(132));
        const int answer=MessageBoxA(GetActiveWindow(),caption.c_str(),"Legend Of Mir 3",MB_YESNO);
        if (answer==IDYES) connection_retry_at=ticks_msec()+2000;
        else get_tree()->quit();
        return;
    }
    const auto page=connection_page;
    auto result=connection.poll();
    if (result.failed) { connection_failed=true;return; }
    if (result.opened && page==ConnectionPage::selection) request_selection_list();
    else if (result.opened && page==ConnectionPage::game) {
        game_connection={};
        const auto packet=outgoing_messages.game_authentication(selection_session.account,game_entry.role,selection_session.number);
        PackedByteArray bytes;bytes.resize(packet.size());std::copy(packet.begin(),packet.end(),bytes.ptrw());
        connection.send(bytes);emit_signal("client_packet_ready",bytes);
    }
    for (const auto& payload:result.messages) {
        if (page!=connection_page) break;
        PackedByteArray bytes;bytes.resize(payload.size());std::copy(payload.begin(),payload.end(),bytes.ptrw());
        if (page==ConnectionPage::login) apply_login_message(bytes);
        else if (page==ConnectionPage::selection) apply_selection_message(bytes);
        else if (page==ConnectionPage::game) apply_game_message(bytes);
    }
}

PackedByteArray Mir3HudView::send_client_packet(int identifier,int actor,const String& body,std::uint32_t instance,std::uint16_t quantity) {
    const auto utf8=body.utf8();
    const auto ansi=mir3::utf8_to_ansi({utf8.get_data(),static_cast<std::size_t>(utf8.length())});
    const auto packet=outgoing_messages.packet(identifier,std::uint32_t(actor),instance,quantity,ansi);
    PackedByteArray bytes;
    bytes.resize(packet.size());
    std::copy(packet.begin(),packet.end(),bytes.ptrw());
    if (connection_enabled) connection.send(bytes);
    emit_signal("client_packet_ready",bytes);
    return bytes;
}

void Mir3HudView::emit_npc_packet(int identifier,int actor,const String& body,std::uint32_t instance,std::uint16_t quantity) {
    const auto bytes=send_client_packet(identifier,actor,body,instance,quantity);
    emit_signal("npc_packet_ready",bytes);
}

void Mir3HudView::close_shop() {
    storage_list=false;storage_pending=0;
    if(group_request==1706 || group_request==1707) finish_group_dialog(false);
    shop_scroll_fraction=0;
    if (group_request==110) finish_group_dialog(false);
    shop_bulk_index=-1;
    shop_scroll_dragging=false;
    shop_actor=0;
    shop_goods.clear();
    shop_selected=-1;
    shop_highlighted=-1;
    shop_offers.clear();
    shop_offer_index=-1;
    shop_offer_page=0;
    shop_offer_loading=false;
    shop_show_offers=false;
    shop_pressed=-1;
    shop_category_first=0;
    queue_redraw();
}

void Mir3HudView::close_npc_dialogue() {
    // 424B37 closes the inventory when it is in a merchant service mode.
    if (inventory.visible && merchant_service.actor) toggle_inventory();
    merchant_service={};
    if (group_request==mir3::protocol::CM_MERCHANTDLGSELECT || group_request==1705) finish_group_dialog(false);
    close_shop();
    npc_dialogue_actor=0;
    npc_dialogue_origin={};
    npc_dialogue_rows.clear();
    npc_dialogue_pressed=-1;
    npc_dialogue_close_pressed=false;
    npc_dialogue_arrow_pressed=0;
    queue_redraw();
}

bool Mir3HudView::apply_shop_purchase_reply(int identifier,int64_t value) {
    const auto remove_selected_offer=[&] {
        // 478750 removes the selected row from the retained list, including during page requests.
        if(!shop_show_offers || shop_offer_index<0 || shop_offer_index>=int(shop_offers.size()))
            return;
        shop_offers.erase(shop_offers.begin()+shop_offer_index);
        if(shop_offer_index>=int(shop_offers.size())) shop_offer_index=-1;
    };
    if (identifier==mir3::protocol::SM_BUYITEM_SUCCESS && value>=0 && value<=UINT32_MAX) {
        inventory_gold=static_cast<std::uint32_t>(value);
        remove_selected_offer();
    } else if (identifier==mir3::protocol::SM_BUYITEM_FAIL && value>=1 && value<=3) {
        if(value==1) remove_selected_offer();
        open_group_dialog(1436+static_cast<int>(value));
    } else return false;
    queue_redraw();
    return true;
}

bool Mir3HudView::apply_shop_goods(int actor,int count,String body,bool wire_format) {
    const auto utf8=body.utf8();
    if (actor<=0 || actor!=npc_dialogue_actor || count<0 || count>255 || utf8.length()>10240*4) return false;
    const std::string_view message(utf8.get_data(),utf8.length());
    if (mir3::utf8_to_ansi(message).size()>10240) return false;
    auto remaining=message.substr(0,message.find(char(0)));
    std::vector<ShopGood> goods;
    for (int i=0;i<count;++i) {
        std::array<std::string_view,7> fields;
        const int field_count=wire_format?7:4;
        for (int j=0;j<field_count;++j) {
            const auto slash=remaining.find('/');
            if (slash==remaining.npos || slash==0) return false;
            fields[j]=remaining.substr(0,slash);
            remaining.remove_prefix(slash+1);
        }
        ShopGood good{std::string(fields[0]),0,0,0};
        const std::array values{&good.variants,&good.price,&good.stock,&good.icon,&good.requirement_type,&good.requirement};
        for (int j=0;j<field_count-1;++j) {
            const auto field=fields[j+1];
            const auto end=field.data()+field.size();
            const auto parsed=std::from_chars(field.data(),end,*values[j]);
            if (parsed.ec!=std::errc{} || parsed.ptr!=end || *values[j]<0) return false;
        }
        if (good.icon>65535) return false;
        goods.push_back(std::move(good));
    }
    if (wire_format && !remaining.empty()) return false;
    if (group_request==110) finish_group_dialog(false);
    storage_list=false;storage_pending=0;
    shop_goods=std::move(goods);
    shop_scroll_fraction=0;
    shop_bulk_index=-1;
    shop_scroll_dragging=false;
    shop_highlighted=shop_goods.empty()?-1:0;
    shop_actor=actor;
    shop_selected=-1;
    shop_offers.clear();
    shop_offer_index=-1;
    shop_offer_page=0;
    shop_offer_loading=false;
    shop_category_first=0;
    shop_show_offers=false;
    shop_pressed=-1;
    queue_redraw();
    return true;
}

bool Mir3HudView::apply_shop_offers(int actor,int index,PackedStringArray names,PackedInt32Array icons,
        PackedInt32Array prices,PackedInt64Array identifiers,int offset) {
    if (!shop_actor || actor!=shop_actor || index<0 || index!=shop_selected ||
        offset< -1 || offset>65535 ||
        names.size()!=icons.size() || names.size()!=prices.size() || names.size()!=identifiers.size() ||
        names.size()>255) return false;
    std::vector<ShopOffer> offers;
    for (int i=0;i<names.size();++i) {
        if (names[i].is_empty() || icons[i]<0 || icons[i]>65535 || prices[i]<0 ||
            identifiers[i]<=0 || identifiers[i]>UINT32_MAX) return false;
        offers.push_back({names[i].utf8().get_data(),icons[i],prices[i],static_cast<std::uint32_t>(identifiers[i])});
    }
    if (group_request==110) finish_group_dialog(false);
    shop_offers=std::move(offers);
    shop_bulk_index=-1;
    shop_offer_index=shop_offers.empty()?-1:0;
    shop_offer_loading=false;
    // 47974D sets the displayed page from the reply's item offset.
    if (offset>=0) shop_offer_page=offset/10;
    else if (!shop_show_offers) shop_offer_page=0;
    shop_show_offers=true;
    shop_pressed=-1;
    queue_redraw();
    return true;
}

void Mir3HudView::draw_shop() {
    if (!shop_actor) return;
    // 0x479410 positions the goods window below the NPC dialogue.
    const auto skin=frame(shop_bulk_index>=0?1262:shop_show_offers?1261:1260);
    if (skin.is_valid())
        paint_canvas->draw_texture(skin,Vector2(shop_window_left()+(shop_width()-skin->get_width())/2,shop_window_top()+(shop_window_height()-skin->get_height())/2));
    const int last=std::max(0,int(shop_goods.size())-1);
    // 0x47776B draws frame 1225 at (260,32), with 85 pixels of travel.
    if (last>0) sprite(1225,{260,shop_top+32+int(std::lround(85.0*shop_scroll_fraction))});
    // 0x478000 repositions the controls after the shop changes modes.
    const int detail_x=shop_window_left(),detail_y=storage_list?388:shop_top+163;
    if (shop_pressed==-5 && shop_show_offers) sprite(1222,{detail_x+449,detail_y-4});
    if (shop_pressed==-7) sprite(1222,{258,shop_top+183});
    if (shop_pressed==-6) sprite(1264,{106,shop_top+186});
    if (shop_bulk_index>=0) {
        const auto& good=shop_goods[shop_bulk_index];
        // 0x477127 / 0x477C80: the two preview cells use the same tint.
        const Rect2 icon_cell(305,shop_top+17,36,36), description_cell(352,shop_top+17,148,36);
        const Color tint(80.f/255,80.f/255,160.f/255,1.f/255);
        legacy_rect(icon_cell,tint);
        legacy_rect(description_cell,tint);
        paint_canvas->draw_rect(icon_cell,Color(180.f/255,180.f/255,180.f/255),false);
        paint_canvas->draw_rect(description_cell,Color(250.f/255,200.f/255,150.f/255),false);
        const auto icon=item_frame(good.icon);
        if (icon.is_valid()) paint_canvas->draw_texture(icon,Vector2(324-icon->get_width()/2,shop_top+36-icon->get_height()/2));
        const int y=shop_top+(good.requirement?19:30);
        auto price=message(2300);
        const auto marker=price.find("%d");
        if (marker!=price.npos) price.replace(marker,2,std::to_string(good.price));
        else price=std::to_string(good.price);
        const int name_width=mir3::raster_text(good.name,1,1,false,mir3::TextFont::normal).text_width;
        text(good.name,{357,y-1,126,16},0x0a0a0a,mir3::TextFont::normal,false);
        text(good.name,{358,y,126,16},0xfafa,mir3::TextFont::normal,false);
        text(price,{359+name_width,y-1,140,16},0x0a0a0a,mir3::TextFont::normal,false);
        text(price,{360+name_width,y,140,16},0x64b4fa,mir3::TextFont::normal,false);
        draw_shop_requirement(good,358,y+20,142);
        if (shop_pressed==-8) sprite(1273,{367,shop_top+59});
        if (shop_pressed==-5) sprite(1222,{475,shop_top+56});
    }
    if (shop_show_offers) {
        // 0x477780: 652 acknowledges the page; the request path displays an ellipsis.
        if(storage_list) {
            const int pages=std::max(1,(int(shop_offers.size())+11)/12);
            text(std::to_string(shop_offer_page+1),{52,229,40,30},0x96c8ff);
            text("/    "+std::to_string(pages),{82,229,50,30},0x96c8ff);
        } else text(shop_offer_loading?"...":std::to_string(shop_offer_page),
            {detail_x+310,shop_window_top()-3,163,44},0x96c8ff);
        if (shop_pressed==-2) sprite(1266,{detail_x+358,detail_y});
        if (shop_pressed==-3) sprite(1269,{detail_x+316,detail_y});
        if (shop_pressed==-4) sprite(1271,{detail_x+406,detail_y});
        for (int row=0;row<12;++row) {
            const int index=row+(storage_list?12*shop_offer_page:0);
            if(index>=int(shop_offers.size())) break;
            const auto& offer=shop_offers[index];
            const auto cell=rect(shop_offer_cell(row));
            const auto icon=item_frame(offer.icon);
            if (icon.is_valid()) {
                if (index==shop_offer_index) {
                    legacy_rect(cell,Color(50.f/255,150.f/255,150.f/255,1.f/255));
                    paint_canvas->draw_rect(cell,Color(180.f/255,180.f/255,180.f/255),false);
                }
                // 0x442200 draws native-size icons around (19,19) in the 38px grid.
                paint_canvas->draw_texture(icon,cell.position+Vector2(19-icon->get_width()/2,19-icon->get_height()/2));
                if(storage_list) continue; // 477780 uses the item drawing alone in mode 2.
                const auto price=std::to_string(offer.price);
                const auto bounds=shop_offer_cell(row);
                text(price,{bounds[0]-1,bounds[1]+20,35,15},0x0a0a0a,mir3::TextFont::skill_level);
                text(price,{bounds[0],bounds[1]+21,35,15},0xffffff,mir3::TextFont::skill_level);
            }
        }
    }
    for (int i=0;i<std::min(shop_goods_rows,int(shop_goods.size())-shop_category_first);++i) {
        const int index=shop_category_first+i;
        const auto& good=shop_goods[index];
        const int y=shop_top+shop_goods_row_top+shop_goods_pitch*i;
        const Rect2 icon_cell(20,y,36,36), description_cell(62,y,189,36);
        // 0x477160 highlights both rectangles and uses a separate tint for zero variants.
        const bool selected=index==shop_highlighted;
        const Color selected_border(250.f/255,200.f/255,150.f/255);
        if (selected) {
            const auto tint=good.variants?Color(40.f/255,120.f/255,120.f/255,1.f/255):
                Color(80.f/255,80.f/255,160.f/255,1.f/255);
            legacy_rect(icon_cell,tint);
            legacy_rect(description_cell,tint);
            paint_canvas->draw_rect(description_cell,selected_border,false);
        }
        if (good.icon>=0) {
            const auto icon=item_frame(good.icon);
            if (icon.is_valid()) paint_canvas->draw_texture(icon,Vector2(39-icon->get_width()/2,y+19-icon->get_height()/2));
        }
        if (selected || good.variants)
            paint_canvas->draw_rect(icon_cell,selected?selected_border:Color(180.f/255,180.f/255,180.f/255),false);
        // 0x477160 places the localized price two pixels after the measured name.
        const bool has_requirement=good.variants && good.requirement>0;
        const int label_y=y+(good.icon==304 || has_requirement?2:13);
        const int name_width=mir3::raster_text(good.name,1,1,false,mir3::TextFont::normal).text_width;
        auto price=message(2300);
        const auto marker=price.find("%d");
        if (marker!=price.npos) price.replace(marker,2,std::to_string(good.price));
        else price=std::to_string(good.price);
        if (selected) {
            text(good.name,{65,label_y-1,184,16},0x0a0a0a,mir3::TextFont::normal,false);
            text(price,{67+name_width,label_y-1,124,16},0x0a0a0a,mir3::TextFont::normal,false);
        }
        text(good.name,{66,label_y,184,16},0xfafa,mir3::TextFont::normal,false);
        text(price,{68+name_width,label_y,124,16},0x64b4fa,mir3::TextFont::normal,false);
        draw_shop_requirement(good,66,label_y+20,184);
    }
}

void Mir3HudView::draw_shop_requirement(const ShopGood& good,int x,int y,int width) {
    // 0x478250: frame 304 displays a localized profession requirement.
    if (good.icon==304 && good.requirement<=2)
        text(message(5170+good.requirement),{x,y,width,16},
            model.profession==good.requirement?13172680:1974015,mir3::TextFont::normal,false);
    else if (good.icon!=304 && good.variants && good.requirement>0 && good.requirement_type<12) {
        // 0x4782E0 compares level, maximum attack, or the minimum across the requested elements.
        const auto& attack=elements.bytes[0];
        const auto minimum=[&](int first,int last) {
            return int(*std::min_element(attack.begin()+first,attack.begin()+last));
        };
        const std::array attributes{model.level,model.attack[1],minimum(0,4),minimum(4,7),minimum(0,7),
            int(attack[0]),int(attack[1]),int(attack[2]),int(attack[3]),int(attack[4]),int(attack[5]),int(attack[6])};
        const auto label=message(5140+good.requirement_type)+" "+std::to_string(good.requirement);
        text(label,{x,y,width,16},attributes[good.requirement_type]>=good.requirement?16773360:1974015,
            mir3::TextFont::normal,false);
    }
}

void Mir3HudView::withdraw_storage(int quantity) {
    if(!storage_list || storage_pending || shop_offer_index<0 || shop_offer_index>=int(shop_offers.size())) return;
    const auto& offer=shop_offers[shop_offer_index];
    if(!quantity && offer.stack_mode>=2 && offer.count>1) {
        open_group_dialog(1707);
        return;
    }
    if(!quantity && offer.stack_mode>=2) quantity=1;
    if(quantity<0 || quantity>offer.count) return;
    storage_pending=offer.id;
    send_client_packet(mir3::protocol::CM_USERTAKEBACKSTORAGEITEM,shop_actor,String::utf8(offer.name.c_str()),offer.id,std::uint16_t(quantity));
}

void Mir3HudView::buy_shop_offer(int index) {
    if(storage_list) {withdraw_storage();return;}
    if (shop_offer_loading) return;
    const auto& offer=shop_offers[index];
    const auto name=String::utf8(offer.name.c_str());
    const auto instance=offer.id;
    const int actor=shop_actor;
    emit_signal("shop_purchase_requested",mir3::protocol::CM_USERBUYITEM,actor,name);
    emit_npc_packet(mir3::protocol::CM_USERBUYITEM,actor,name,instance);
}

void Mir3HudView::open_shop_category() {
    // 0x478850 only branches on mode 2; mode 0 also requests details.
    if (shop_highlighted<0) return;
    if (shop_goods[shop_highlighted].variants==2) {
        shop_bulk_index=shop_highlighted;
        shop_show_offers=false;
        shop_selected=shop_offer_index=-1;
        shop_offer_page=0;
        shop_offer_loading=false;
        shop_offers.clear();
        return;
    }
    shop_bulk_index=-1;
    if (shop_selected!=shop_highlighted) {
        shop_offers.clear();
        shop_offer_index=-1;
    }
    shop_selected=shop_highlighted;
    shop_offer_page=0;
    if (shop_show_offers) shop_offer_loading=true;
    const int actor=shop_actor;
    const int index=shop_selected;
    const auto name=String::utf8(shop_goods[index].name.c_str());
    emit_signal("shop_good_requested",actor,index,name);
    emit_signal("npc_request",mir3::protocol::CM_USERGETDETAILITEM,actor,name);
    emit_npc_packet(mir3::protocol::CM_USERGETDETAILITEM,actor,name);
}

void Mir3HudView::buy_shop_bulk(std::uint16_t quantity) {
    const auto& good=shop_goods[shop_bulk_index];
    const auto item=String::utf8(good.name.c_str());
    const int actor=shop_actor;
    emit_npc_packet(mir3::protocol::CM_USERBUYITEM,actor,item,good.stock,quantity);
    emit_signal("shop_purchase_requested",mir3::protocol::CM_USERBUYITEM,actor,item);
}

bool Mir3HudView::shop_input(int kind,Vector2 pointer,bool double_click) {
    if (!shop_actor) return false;
    if (shop_scroll_input(kind,pointer)) return true;
    const bool inside=Rect2(shop_window_left(),shop_window_top(),shop_width(),shop_window_height()).has_point(pointer);
    const int detail_x=shop_window_left(),detail_y=storage_list?388:shop_top+163;
    const bool details=shop_show_offers;
    constexpr int confirm_button=-2;
    constexpr int category_button=-6;
    int control=0;
    const auto close_icon=frame(1221);
    if (close_icon.is_valid()) {
        // 0x478850 checks the list close button even in mode 1 (detail grid).
        if (!storage_list && Rect2(Vector2(258,shop_top+183),close_icon->get_size()).has_point(pointer)) control=-7;
        else if (details && Rect2(Vector2(detail_x+449,detail_y-4),close_icon->get_size()).has_point(pointer)) control=-5;
        else if (shop_bulk_index>=0 && Rect2(Vector2(475,shop_top+56),close_icon->get_size()).has_point(pointer)) control=-5;
    }
    if (shop_bulk_index>=0 && !control) {
        const auto confirm=frame(1272);
        if (confirm.is_valid() && Rect2(Vector2(367,shop_top+59),confirm->get_size()).has_point(pointer)) control=-8;
    }
    if (details && !control) {
        for (const auto& spec:std::array{std::array{confirm_button,1265,358},std::array{-3,1268,316},std::array{-4,1270,406}}) {
            const auto image=frame(spec[1]);
            if (image.is_valid() && Rect2(Vector2(detail_x+spec[2],detail_y),image->get_size()).has_point(pointer)) {
                control=spec[0];
                break;
            }
        }
    }
    if (!control && !storage_list) {
        const auto confirm=frame(1263);
        if (confirm.is_valid() && Rect2(Vector2(106,shop_top+186),confirm->get_size()).has_point(pointer))
            control=category_button;
    }
    if (!control && kind==0 && double_click && shop_bulk_index>=0 &&
        (Rect2(305,shop_top+17,36,36).has_point(pointer) || Rect2(352,shop_top+17,148,36).has_point(pointer))) {
        // 0x479100 buys one item when either bulk preview cell is double-clicked.
        shop_pressed=-1;
        buy_shop_bulk(1);
        get_viewport()->set_input_as_handled();
        queue_redraw();
        return true;
    }
    if (control || shop_pressed<=confirm_button) {
        const int activate=kind==2 && control==shop_pressed?control:0;
        shop_pressed=kind==0 && control?control:-1;
        if (activate==-7) close_shop();
        else if (activate==-8) open_group_dialog(110);
        else if (activate==-5) close_shop_offers();
        else if (activate==category_button) {
            open_shop_category();
        } else if (activate==confirm_button && details && shop_offer_index>=0 && shop_offer_index<int(shop_offers.size())) {
            buy_shop_offer(shop_offer_index);
        } else if((activate==-3 || activate==-4) && storage_list && !storage_pending) {
            shop_offer_page=std::clamp(shop_offer_page+(activate==-3?-1:1),0,std::max(0,(int(shop_offers.size())-1)/12));
            shop_offer_index=-1;
        } else if ((activate==-3 || activate==-4) && shop_selected>=0 && !shop_offer_loading) {
            // 0x478850 requests a server page with a ten-item offset.
            const int page=std::clamp(shop_offer_page+(activate==-3?-1:1),0,65535/10);
            if (page!=shop_offer_page) {
                shop_offer_page=page;
                shop_offer_loading=true;
                const auto name=String::utf8(shop_goods[shop_selected].name.c_str());
                emit_npc_packet(mir3::protocol::CM_USERGETDETAILITEM,shop_actor,name,std::uint32_t(10*page));
            }
        }
        get_viewport()->set_input_as_handled();
        queue_redraw();
        return true;
    }
    int row=-1;
    bool offer_cell=false;
    if (details) {
        for (int i=0;i<12;++i) {
            if (rect(shop_offer_cell(i)).has_point(pointer)) { row=i+(storage_list?12*shop_offer_page:0); offer_cell=true; break; }
        }
    }
    if (!offer_cell && !storage_list) {
        // 0x4770B4 / 0x4785A0: icon and text are separate hit rectangles.
        for (int i=0;i<shop_goods_rows;++i) {
            const int y=shop_top+shop_goods_row_top+shop_goods_pitch*i;
            if (Rect2(20,y,36,36).has_point(pointer) || Rect2(62,y,189,36).has_point(pointer)) {
                row=shop_category_first+i;
                break;
            }
        }
    }
    const bool hit=row>=0 && (offer_cell?!shop_offer_loading && row<int(shop_offers.size()):
        row<int(shop_goods.size()));
    const int hit_id=offer_cell?256+row:row;
    const int pressed=shop_pressed;
    shop_pressed=kind==0 && hit?hit_id:-1;
    if (kind==0 && double_click && hit) {
        // 0x479100 buys the item under the pointer on a double-click.
        shop_pressed=-1;
        if (offer_cell) {
            shop_offer_index=row;
            buy_shop_offer(row);
        } else {
            shop_highlighted=row;
            open_shop_category();
        }
    } else if (kind==2 && hit && pressed==hit_id) {
        if (offer_cell) {
            shop_offer_index=row;
            emit_signal("shop_offer_selected",shop_actor,int64_t(shop_offers[row].id));
        } else {
            shop_highlighted=row;
        }
    }
    if (!inside && pressed<0) return false;
    get_viewport()->set_input_as_handled();
    queue_redraw();
    return true;
}

bool Mir3HudView::shop_scroll_input(int kind,Vector2 pointer) {
    if (!shop_actor || shop_goods.size()<2) return false;
    const auto thumb=frame(1225);
    if (thumb.is_null()) return false;
    constexpr int x=260, top=shop_top+32, travel=85;
    const int last=int(shop_goods.size())-1;
    const int y=top+int(std::lround(travel*shop_scroll_fraction));
    if (kind==0 && Rect2(x,top,thumb->get_width(),travel+thumb->get_height()).has_point(pointer)) {
        shop_scroll_dragging=Rect2(x,y,thumb->get_width(),thumb->get_height()).has_point(pointer);
        if (!shop_scroll_dragging) {
            // 0x420200 changes the normalized position by 5 / item count.
            const int direction=pointer.y<y+thumb->get_height()/2?-1:1;
            shop_scroll_fraction=std::clamp(shop_scroll_fraction+direction*5.0/shop_goods.size(),0.0,1.0);
            shop_category_first=int(std::lround(last*shop_scroll_fraction));
        }
    } else if (kind==1 && shop_scroll_dragging) {
        shop_scroll_fraction=std::clamp((pointer.y-top)/double(travel+thumb->get_height()),0.0,1.0);
        shop_category_first=int(std::lround(last*shop_scroll_fraction));
    } else if (kind==2 && shop_scroll_dragging) shop_scroll_dragging=false;
    else return false;
    shop_pressed=-1;
    get_viewport()->set_input_as_handled();
    queue_redraw();
    return true;
}

void Mir3HudView::close_shop_offers() {
    if(storage_list) {close_shop();queue_redraw();return;}
    if (shop_bulk_index>=0) {
        shop_bulk_index=-1;
        queue_redraw();
        return;
    }
    shop_show_offers=false;
    shop_selected=shop_pressed=shop_offer_index=-1;
    shop_offers.clear();
    shop_offer_page=0;
    queue_redraw();
}

void Mir3HudView::draw_npc_dialogue() {
    if (!npc_dialogue_actor) return;
    // 0x45F9B0 / 0x45FD00: modulate the assembled background, then draw text and controls separately.
    const Color background_modulation(180.f/255,180.f/255,180.f/255,220.f/255);
    const int strips=std::max(1,npc_dialogue_extra_rows);
    auto& background=npc_dialogue_backgrounds[strips-1];
    if (background.is_null()) {
        auto surface=Image::create_empty(386,npc_dialogue_bounds()[3],false,Image::FORMAT_RGBA8);
        const auto compose=[&](int id,int y) {
            const auto part=frame(id);
            if (part.is_null()) return;
            const auto image=part->get_image();
            if (image.is_valid()) surface->blend_rect(image,Rect2i(Vector2i(),image->get_size()),Vector2i(0,y));
        };
        compose(1353,0);
        for (int i=0;i<strips;++i) compose(1351,160+20*i);
        compose(1352,160+20*strips);
        background=ImageTexture::create_from_image(surface);
    }
    if (background.is_valid()) paint_canvas->draw_texture(background,Vector2(),background_modulation);
    const int bottom=npc_dialogue_bounds()[3];
    // 0x460F07 draws the button even when NOTCLOSE disables dismissal.
    sprite(npc_dialogue_close_pressed?1222:1221,{346,bottom-42});
    if (npc_dialogue_scroll_enabled) {
        sprite(npc_dialogue_arrow_pressed==-1?1358:1357,{183,bottom-48});
        if (npc_dialogue_first<npc_dialogue_scroll_limit && npc_dialogue_down_lit)
            sprite(npc_dialogue_arrow_pressed==1?1356:1355,{183,bottom-30});
    }
    const int focused=shop_actor && Rect2(shop_window_left(),shop_window_top(),shop_width(),shop_window_height()).has_point(inventory_cursor)?-1:npc_dialogue_option_at(inventory_cursor);
    for (const auto& row:npc_dialogue_rows) {
        if (row.bounds[1]<npc_dialogue_first || row.bounds[1]>npc_dialogue_first+21) continue;
        const bool clickable=!row.command.empty();
        const auto color=!clickable?row.color:npc_dialogue_pressed==row.option?0x0000ff:0x00ffff;
        const auto bounds=npc_dialogue_text_bounds(row);
        text(row.text,bounds,color,clickable && focused==row.option?mir3::TextFont::npc_link:mir3::TextFont::normal,false);
    }
}

mir3::HudLayout::Rect Mir3HudView::npc_dialogue_text_bounds(const NpcDialogueRow& row) const {
    auto bounds=row.bounds;
    bounds[1]=40+(bounds[3]+5)*(bounds[1]-npc_dialogue_first);
    return bounds;
}

int Mir3HudView::npc_dialogue_option_at(Vector2 pointer) const {
    int option=-1, rectangles=0;
    for (const auto& row:npc_dialogue_rows) {
        if (row.option<0 || row.bounds[1]<npc_dialogue_first || row.bounds[1]>npc_dialogue_first+21) continue;
        if (option!=row.option) { option=row.option; rectangles=0; }
        // 0x461B70 stores up to five visible rectangles per script option.
        if (++rectangles<=5 && rect(npc_dialogue_text_bounds(row)).has_point(pointer)) return option;
    }
    return -1;
}

bool Mir3HudView::npc_dialogue_input(int kind,Vector2 pointer) {
    if (!npc_dialogue_actor) return false;
    const bool inside=rect(npc_dialogue_bounds()).has_point(pointer);
    const int bottom=npc_dialogue_bounds()[3];
    const auto hit_button=[&](int id,int x,int y) {
        const auto image=frame(id);
        return image.is_valid() && Rect2(x,y,image->get_width(),image->get_height()).has_point(pointer);
    };
    const bool close_hit=hit_button(1221,346,bottom-42);
    if (close_hit || npc_dialogue_close_pressed) {
        const bool close=kind==2 && close_hit && npc_dialogue_close_pressed && npc_dialogue_can_close;
        npc_dialogue_close_pressed=kind==0 && close_hit;
        if (close) close_npc_dialogue();
        get_viewport()->set_input_as_handled();
        queue_redraw();
        return true;
    }
    if (npc_dialogue_scroll_enabled) {
        // 0x461016 keeps the up arrow visible but skips its press handler at row zero.
        const bool up=npc_dialogue_first>0 && hit_button(1357,183,bottom-48);
        const bool down=npc_dialogue_first<npc_dialogue_scroll_limit && hit_button(1355,183,bottom-30);
        const int arrow=up?-1:down?1:0;
        if (arrow || npc_dialogue_arrow_pressed) {
            if (kind==2 && arrow==npc_dialogue_arrow_pressed)
                npc_dialogue_first=std::clamp(npc_dialogue_first+arrow,0,npc_dialogue_scroll_limit);
            npc_dialogue_arrow_pressed=kind==0?arrow:0;
            npc_dialogue_pressed=-1;
            get_viewport()->set_input_as_handled();
            queue_redraw();
            return true;
        }
    }
    const int option=npc_dialogue_option_at(pointer);
    const int pressed=npc_dialogue_pressed;
    npc_dialogue_pressed=kind==0?option:-1;
    // 0x4610D0 tests the release position and shares a one-second cooldown across options.
    const auto now=ticks_msec();
    if (kind==2 && option>=0 && (!npc_dialogue_last_action || now-npc_dialogue_last_action>1000)) {
        const auto item=std::find_if(npc_dialogue_rows.begin(),npc_dialogue_rows.end(),[option](const auto& row) {
            return row.option==option;
        });
        const auto command=String::utf8(item->command.c_str());
        const int actor=npc_dialogue_actor;
        npc_dialogue_last_action=now;
        if (item->command.starts_with("@@")) {
            // 0x4610D0 opens a text prompt; 0x4614D0 joins the answer with CR.
            npc_dialogue_prompt.clear();
            if (item->command=="@@buildguildnow") npc_dialogue_prompt=message(2000);
            else {
                for (auto part=item;part!=npc_dialogue_rows.end() && part->option==option;++part)
                    npc_dialogue_prompt+=part->text;
            }
            npc_dialogue_prompt_command=item->command;
            npc_dialogue_prompt_actor=actor;
            open_group_dialog(mir3::protocol::CM_MERCHANTDLGSELECT);
            const std::string_view script_command=item->command;
            if (script_command.starts_with("@@*") && group_request==mir3::protocol::CM_MERCHANTDLGSELECT) {
                chat_input->set_secret_character("*");
                chat_input->set_secret(true);
                // 0x4612E7: storage-password commands optionally carry a byte limit before '_'.
                if (script_command.find("enter_stgpasswd")!=script_command.npos ||
                    script_command.find("Input_stgpasswd")!=script_command.npos ||
                    script_command.find("Change_stgpasswd")!=script_command.npos ||
                    script_command.find("confirm_passwd")!=script_command.npos) {
                    const auto delimiter=script_command.find('_',3);
                    int bytes=0;
                    if (delimiter<10) {
                        const auto end=script_command.data()+delimiter;
                        const auto parsed=std::from_chars(script_command.data()+3,end,bytes);
                        if (parsed.ec!=std::errc{} || parsed.ptr!=end) bytes=0;
                    } else bytes=10;
                    chat_input->set_byte_limit(bytes>0?bytes:mir3::ChatInputSpec::limit);
                }
            }
            queue_redraw();
        } else {
            emit_signal("npc_dialogue_action",actor,command);
            emit_signal("npc_request",mir3::protocol::CM_MERCHANTDLGSELECT,actor,command);
            emit_npc_packet(mir3::protocol::CM_MERCHANTDLGSELECT,actor,command);
        }
    }
    if (!inside && pressed<0) return false;
    get_viewport()->set_input_as_handled();
    queue_redraw();
    return true;
}

bool Mir3HudView::apply_scene_health(int actor,int opcode,int current,int mana,int maximum) {
    if (!terrain_ready || (actor!=0 && !scene_humans.contains(actor) && !scene_chickens.contains(actor) &&
            !(opcode==mir3::protocol::SM_HEALTH_PERCENT && scene_npcs.contains(actor)))) return false;
    auto health=scene_health.contains(actor)?scene_health.at(actor):mir3::SceneHealth{};
    const int previous=actor==0?model.hp:health.current;
    if (!health.apply(opcode,current,mana,maximum)) return false;
    health.damage.enabled=damage_numbers;
    if (opcode==mir3::protocol::SM_HEALTHSPELLCHANGED) health.damage.changed(previous,current);
    scene_health.insert_or_assign(actor,health);
    if (actor==0 && opcode==mir3::protocol::SM_HEALTHSPELLCHANGED) { model.hp=current;model.max_hp=maximum;model.mp=mana; }
    queue_redraw();
    return true;
}

bool Mir3HudView::set_scene_health_flags(int actor,int64_t status_tick,int64_t saved_tick,bool excluded,bool raised) {
    if (!terrain_ready || status_tick<0 || status_tick>UINT32_MAX || saved_tick<0 || saved_tick>UINT32_MAX ||
        (actor!=0 && !scene_humans.contains(actor) && !scene_chickens.contains(actor))) return false;
    auto& health=scene_health[actor];
    health.status_tick=static_cast<std::uint32_t>(status_tick);
    health.saved_tick=static_cast<std::uint32_t>(saved_tick);
    health.excluded=excluded;
    health.raised=raised;
    queue_redraw();
    return true;
}

void Mir3HudView::draw_scene_health() {
    scene_health_bounds.clear();
    scene_damage_glyphs.clear();
    for (auto& [actor,health]:scene_health) {
        Vector2 anchor;
        const bool human=actor==0 || scene_humans.contains(actor);
        if (actor==0) {
            if (!player_bounds.has_area()) continue;
            anchor=Vector2(player_pixel()-map_camera());
        } else {
            const SceneActor* source=nullptr;
            if (const auto it=scene_humans.find(actor);it!=scene_humans.end()) source=&it->second;
            else if (const auto it=scene_chickens.find(actor);it!=scene_chickens.end()) source=&it->second;
            if (!source || !source->bounds.has_area()) continue;
            anchor=source->anchor;
        }
        const auto draw_damage=[&]() {
            health.damage.enabled=damage_numbers;
            auto glyphs=health.damage.draw(int(anchor.x),int(anchor.y),human);
            if (glyphs.empty()) return;
            for (const auto& glyph:glyphs)
                if (const auto image=frame(glyph.frame);image.is_valid()) paint_canvas->draw_texture(image,Vector2(glyph.x,glyph.y));
            scene_damage_glyphs[actor]=std::move(glyphs);
            animated_map_objects=true;
        };
        // 0x424280: world damage precedes bars; the local actor draws damage last.
        if (actor!=0) draw_damage();
        const auto monster=scene_chickens.find(actor);
        if ((monster==scene_chickens.end() || monster->second.death_frame<0) &&
            health.visible(actor==0) && health_border.is_valid() && health_fill.is_valid()) {
            const auto position=anchor+Vector2(7,human && health.raised?-80:-56);
            const auto percent=actor==0?mir3::SceneHealth::percent(model.hp,model.max_hp):mir3::SceneHealth::percent(health.current,health.maximum);
            const int width=mir3::SceneHealth::width(health.empty_bar?0:percent);
            paint_canvas->draw_texture(health_border,position);
            scene_health_bounds[actor]=Rect2(position,Vector2(width,4));
            if (width>0) paint_canvas->draw_texture_rect_region(health_fill,Rect2(position,Vector2(width,4)),Rect2(0,0,width,4));
        }
        if (actor==0) draw_damage();
    }
}

bool Mir3HudView::apply_scene_speech(int actor,int opcode,String value) {
    if (!terrain_ready || (actor!=0 && !scene_humans.contains(actor) && !scene_npcs.contains(actor) && !scene_chickens.contains(actor))) return false;
    auto bubble=scene_speech.contains(actor)?scene_speech.at(actor):mir3::SpeechBubble{};
    bubble.actor_id=static_cast<std::uint32_t>(actor);
    const auto utf8=value.utf8();
    if (!bubble.receive({opcode,static_cast<std::uint32_t>(actor),std::string(utf8.get_data(),utf8.length())})) return false;
    if (bubble.lines.empty()) scene_speech.erase(actor);
    else scene_speech.insert_or_assign(actor,std::move(bubble));
    queue_redraw();
    return true;
}

void Mir3HudView::draw_scene_speech() {
    speech_bounds.clear();
    for (const auto& [actor,bubble]:scene_speech) {
        Vector2 anchor;
        if (actor==0) {
            if (!player_bounds.has_area()) continue;
            anchor=Vector2(player_pixel()-map_camera());
        } else {
            const SceneActor* source=nullptr;
            if (const auto it=scene_humans.find(actor);it!=scene_humans.end()) source=&it->second;
            else if (const auto it=scene_npcs.find(actor);it!=scene_npcs.end()) source=&it->second;
            else if (const auto it=scene_chickens.find(actor);it!=scene_chickens.end()) source=&it->second;
            if (!source || !source->bounds.has_area()) continue;
            anchor=source->anchor;
        }
        const auto bounds=rect(bubble.background(int(anchor.x),int(anchor.y)));
        speech_bounds[actor]=bounds;
        legacy_rect(bounds,Color(80.f/255,60.f/255,40.f/255,120.f/255));
        for (int row=0;row<int(bubble.lines.size());++row) {
            const auto position=bubble.position(row,int(anchor.x),int(anchor.y));
            const auto& line=bubble.lines[row];
            const mir3::HudLayout::Rect shadow{position[0]+1,position[1]+1,line.width,14};
            text(line.text,shadow,0x0a0a0a,mir3::TextFont::normal,false);
            text(line.text,{position[0],position[1],line.width,14},0xffffff,mir3::TextFont::normal,false);
        }
    }
}

bool Mir3HudView::apply_scene_name(int actor,int opcode,int color,String value) {
    if (color<0 || color>255 || (!scene_humans.contains(actor) && !scene_npcs.contains(actor) && !scene_chickens.contains(actor))) return false;
    auto identity=scene_names.contains(actor)?scene_names.at(actor):mir3::SceneName{};
    const auto ansi=mir3::utf8_to_ansi(value.utf8().get_data());
    if (!identity.update(opcode,static_cast<std::uint8_t>(color),ansi)) return false;
    scene_names.insert_or_assign(actor,std::move(identity));
    if (auto human=scene_humans.find(actor);human!=scene_humans.end())
        human->second.name=String::utf8(mir3::ansi_to_utf8(scene_names.at(actor).lines[0]).c_str());
    queue_redraw();
    return true;
}

void Mir3HudView::draw_ground_name() {
    displayed_ground_item=0;
    if (!terrain_ready || !spell_pointer_available()) return;
    const auto camera=map_camera();
    const Vector2i cell(int(inventory_cursor.x+camera.x)/48,int(inventory_cursor.y+camera.y)/32);
    const auto coin_name=messages.get(1409);
    int y=cell.y*32-camera.y-4;
    int rows=0;
    for (const auto& item:ground_items.items) {
        if (item.x!=cell.x || item.y!=cell.y) continue;
        const auto caption=mir3::ansi_to_utf8(item.name);
        const int width=mir3::raster_text(caption,800,15,false,mir3::TextFont::normal).text_width;
        const int x=item.x*48-camera.x+24-width/2;
        // 423A27/423B31 draw a brown label panel and stack subsequent labels upward.
        legacy_rect(Rect2(x,y,width,15),Color(80.f/255,60.f/255,40.f/255,150.f/255));
        const bool coin_variant=!coin_name.empty() && item.name.size()==coin_name.size()+1 &&
            item.name.starts_with(coin_name) && item.name.back()>='0' && item.name.back()<='9';
        const auto label=coin_variant?mir3::ansi_to_utf8(coin_name):caption;
        text(label,{x-1,y+2,width,15},0,mir3::TextFont::normal,false);
        text(label,{x,y+1,width,15},0x7dffff,mir3::TextFont::normal,false);
        if (!rows) displayed_ground_item=item.id;
        if (coin_variant || ++rows==mir3::GroundItems::cell_capacity) break;
        y-=16;
    }
}

void Mir3HudView::draw_scene_name() {
    displayed_name_actor=-1;
    displayed_name_bounds=Rect2();
    const auto draw_name=[&](const mir3::SceneName* identity,Vector2 anchor) {
        Rect2 bounds;
        for (int line=0;line<3;++line) {
            const auto caption=mir3::ansi_to_utf8(identity->lines[line]);
            if (caption.empty()) continue;
            const auto measured=mir3::raster_text(caption,800,15,false,mir3::TextFont::normal);
            const auto r=mir3::SceneName::rect(int(anchor.x),int(anchor.y),measured.text_width,line);
            bounds=bounds.has_area()?bounds.merge(rect(r)):rect(r);
            for (const auto [x,y]:std::array<std::array<int,2>,4>{{{-1,-1},{1,-1},{-1,1},{1,1}}}) {
                auto outline=r;outline[0]+=x;outline[1]+=y;
                text(caption,outline,0x0a0a0a,mir3::TextFont::normal,true);
            }
            text(caption,r,identity->color,mir3::TextFont::normal,true);
        }
        return bounds;
    };
    // 离线全集仅扩大姓名显示范围；字体、颜色、描边和布局复用游戏绘制。
    if (bool(get_meta("offline_preview",false)) && bool(get_meta("showcase_monster_names",false))) {
        for (const auto& [actor,monster]:scene_chickens)
            if (const auto name=scene_names.find(actor);name!=scene_names.end()) draw_name(&name->second,monster.anchor);
        return;
    }
    if (!spell_pointer_available()) return;
    Vector2 anchor;
    const auto pick=[&](const auto& actors) {
        for (const auto& [actor,value]:actors) {
            if (!value.bounds.has_point(inventory_cursor)) continue;
            if (displayed_name_actor>=0 && value.anchor.y<anchor.y) continue;
            displayed_name_actor=actor;
            anchor=value.anchor;
        }
    };
    pick(scene_npcs);pick(scene_chickens);pick(scene_humans);
    mir3::SceneName local;
    const mir3::SceneName* identity=nullptr;
    if (player_bounds.has_point(inventory_cursor) && (displayed_name_actor<0 || player_pixel().y-map_camera().y>=anchor.y)) {
        displayed_name_actor=0;
        anchor=Vector2(player_pixel()-map_camera());
        local.lines={mir3::utf8_to_ansi(character_name.utf8().get_data()),mir3::utf8_to_ansi(character_secondary.utf8().get_data()),mir3::utf8_to_ansi(character_tertiary.utf8().get_data())};
        identity=&local;
    } else if (const auto found=scene_names.find(displayed_name_actor);found!=scene_names.end()) identity=&found->second;
    if (!identity || identity->lines[0].empty()) { displayed_name_actor=-1; return; }
    displayed_name_bounds=draw_name(identity,anchor);
}
bool Mir3HudView::set_scene_npc(int actor,Vector2i cell,int first_frame) {
    return place_scene_npc(actor,cell,first_frame,4,200,true);
}

bool Mir3HudView::place_scene_npc(int actor,Vector2i cell,int first_frame,int count,int interval,
        bool fallback_to_first_frame,bool flag,int idle_first,int idle_count,int idle_interval,bool revive) {
    if (actor==local_actor_identity) return false;
    if (!terrain_ready || actor<=0 || scene_chickens.contains(actor) || scene_humans.contains(actor) || first_frame<0 || first_frame>65536-count) return false;
    SceneNpc npc{};
    npc.cell=cell;
    npc.standing.resize(count);
    npc.standing_interval=interval;
    npc.fallback_to_first_frame=fallback_to_first_frame;
    if (!fallback_to_first_frame) {
        // 42EDC7 routes existing objects without running the arrival constructor.
        const auto previous=scene_npcs.find(actor);
        npc.arrival_ms=revive || previous==scene_npcs.end()?0:previous->second.arrival_ms;
        // 4102D4 starts a 600 ms appearance transition on SM_REVIVE.
        if (revive) npc.appearance_duration_ms=600;
        else if (previous!=scene_npcs.end()) npc.appearance_duration_ms=previous->second.appearance_duration_ms;
    }
    const auto load_frames=[&](auto& frames,int first) {
        for (int i=0;i<int(frames.size());++i)
            if (const auto frame=(flag?map_flag:map_npc).frame(first+i))
                frames[i]={texture(frame->pixels.rgba,frame->width,frame->height),Vector2(frame->offset_x,frame->offset_y),frame->pixels.regions,frame->shadow_mode,Vector2(frame->shadow_x,frame->shadow_y)};
    };
    load_frames(npc.standing,first_frame);
    npc.idle_after_refresh.resize(idle_count);
    npc.idle_interval=idle_interval;
    if (idle_count) load_frames(npc.idle_after_refresh,idle_first);
    if (fallback_to_first_frame && npc.standing.front().image.is_null()) return false;
    std::erase_if(map_events,[actor](const auto& entry) {
        const auto& effect=entry.second;
        if (effect.attached_actor!=actor || effect.resource->id!=607) return false;
        effect.copy->hide();
        effect.copy->queue_free();
        return true;
    });
    scene_npcs.insert_or_assign(actor,std::move(npc));
    queue_redraw();
    return true;
}

bool Mir3HudView::scene_occupied(Vector2i cell) const {
    const auto occupies=[cell](const auto& actor) { return actor.second.cell==cell; };
    return std::ranges::any_of(scene_npcs,occupies) ||
        occupied_by_monster(cell) ||
        std::ranges::any_of(scene_humans,[cell](const auto& actor) { return !actor.second.life.dead && actor.second.cell==cell; });
}

void Mir3HudView::update_actor_bar_status(int actor,bool status_bit) {
    auto& health=scene_health[actor];
    health.status_tick=status_bit?std::uint32_t(ticks_msec()):0;
    if (status_bit) health.saved_tick=health.status_tick;
    queue_redraw();
}

const Mir3HudView::MapActorFrame* Mir3HudView::attachment_frame(int type,int shape,bool weapon,int motion_frame) {
    if (!shape || (weapon && shape==37)) return nullptr;
    // mir3_1.45 ActorAppearance, 0x414650 / 0x414870.
    const std::string prefix=type?"WM-":"M-";
    const std::string library=prefix+(weapon?"Weapon"+std::to_string((shape-1)/10+1):
        shape<=10?"Hair":shape<=20?"Helmet1":"Helmet2");
    const int index=3000*((shape-1)%10)+motion_frame;
    const auto key=std::pair{library,index};
    if (const auto found=attachment_frames.find(key);found!=attachment_frames.end()) return &found->second;
    auto [archive,inserted]=attachment_archives.try_emplace(library);
    if (inserted && !archive->second.open(std::filesystem::u8path(actor_resource_root.utf8().get_data())/library)) {
        attachment_archives.erase(archive);
        return nullptr;
    }
    const auto source=archive->second.frame(index);
    MapActorFrame frame;
    if (source) frame={texture(source->pixels.rgba,source->width,source->height),Vector2(source->offset_x,source->offset_y),source->pixels.regions,source->shadow_mode,Vector2(source->shadow_x,source->shadow_y)};
    return &attachment_frames.emplace(key,std::move(frame)).first->second;
}

bool Mir3HudView::set_player_attachments(int hair,int weapon) {
    if (!terrain_ready || hair<0 || hair>=30 || weapon<0 || weapon>=50 ||
        (weapon && weapon_order.size()!=2640)) return false;
    player_hair=hair;
    player_weapon=weapon;
    queue_redraw();
    return true;
}

bool Mir3HudView::set_scene_human_attachments(int actor,int hair,int weapon) {
    const auto found=scene_humans.find(actor);
    if (found==scene_humans.end() || hair<0 || hair>=30 || weapon<0 || weapon>=50 ||
        (weapon && weapon_order.size()!=2640)) return false;
    found->second.hair=hair;
    found->second.weapon=weapon;
    queue_redraw();
    return true;
}

const Mir3HudView::MapActorFrame* Mir3HudView::horse_frame(int shape,int motion_frame) {
    if (shape<1 || shape>3 || motion_frame<2320) return nullptr;
    // 0x414870: horse groups are 400 frames; riding human actions start at 2320.
    const int index=400*shape+motion_frame-2720;
    if (const auto found=horse_frames.find(index);found!=horse_frames.end()) return &found->second;
    const auto source=map_horse.frame(index);
    if (!source) return nullptr;
    MapActorFrame frame{texture(source->pixels.rgba,source->width,source->height),Vector2(source->offset_x,source->offset_y),source->pixels.regions,source->shadow_mode,Vector2(source->shadow_x,source->shadow_y)};
    return &horse_frames.emplace(index,std::move(frame)).first->second;
}

bool Mir3HudView::set_player_mount(int shape) {
    if (!terrain_ready || shape<0 || shape>3) return false;
    if (shape && (!human_body(player_type,player_clothing,true) || !horse_frame(shape,2320))) return false;
    player_mount=shape;
    queue_redraw();
    return true;
}

bool Mir3HudView::set_scene_human_mount(int actor,int shape) {
    const auto found=scene_humans.find(actor);
    if (found==scene_humans.end() || shape<0 || shape>3) return false;
    const auto& human=found->second;
    if (shape && (!human_body(human.type,human.clothing,true) || !horse_frame(shape,2320))) return false;
    found->second.mount_shape=shape;
    found->second.idle.reset();
    found->second.bounds=Rect2();
    queue_redraw();
    return true;
}

bool Mir3HudView::set_player_dyes(int clothing,int hair) {
    if (!terrain_ready || clothing<0 || clothing>65535 || hair<0 || hair>65535) return false;
    player_clothing_dye=clothing;
    player_hair_dye=hair;
    queue_redraw();
    return true;
}

bool Mir3HudView::set_scene_human_dyes(int actor,int clothing,int hair) {
    const auto found=scene_humans.find(actor);
    if (found==scene_humans.end() || clothing<0 || clothing>65535 || hair<0 || hair>65535) return false;
    found->second.clothing_dye=clothing;
    found->second.hair_dye=hair;
    queue_redraw();
    return true;
}

Ref<ImageTexture> Mir3HudView::dyed_frame(const MapActorFrame& frame,int clothing,int hair,int status_color,bool highlight_status) {
    const bool status_tint=status_color!=65535 && status_color!=1;
    if ((!status_tint && clothing==65535 && hair==65535) || frame.regions.empty()) return frame.image;
    const auto key=std::tuple{std::uint64_t(frame.image->get_instance_id()),status_tint?(highlight_status?-1:-2):clothing,status_tint?status_color:hair};
    if (const auto found=dyed_frames.find(key);found!=dyed_frames.end()) return found->second;
    auto pixels=frame.image->get_image()->get_data();
    if (status_tint) {
        auto* rgba=pixels.ptrw();
        for (std::size_t index=0;index<frame.regions.size();++index) {
            if (!frame.regions[index]) continue;
            auto* pixel=rgba+4*index;
            if (status_color==0) {
                const int gray=(pixel[0]/8+pixel[1]/8+pixel[2]/8)/3;
                const auto intensity=std::min(31,gray+(highlight_status?gray*3/5:0));
                pixel[0]=pixel[1]=pixel[2]=std::uint8_t(intensity*8);
            } else {
                const auto channel=[highlight_status](int source,int color,int maximum) {
                    const double base=double(source)*color/maximum;
                    return std::min(maximum,int(base)+(highlight_status?int(base*double(0.6f)):0));
                };
                pixel[0]=std::uint8_t(8*channel(pixel[0]/8,status_color>>11,31));
                pixel[1]=std::uint8_t(4*channel(pixel[1]/4,(status_color>>5)&63,63));
                pixel[2]=std::uint8_t(8*channel(pixel[2]/8,status_color&31,31));
            }
        }
    } else {
        mir3::tint_wil_pixels({pixels.ptrw(),static_cast<std::size_t>(pixels.size())},frame.regions,
            static_cast<std::uint16_t>(clothing),static_cast<std::uint16_t>(hair));
    }
    const auto image=ImageTexture::create_from_image(Image::create_from_data(frame.image->get_width(),frame.image->get_height(),false,Image::FORMAT_RGBA8,pixels));
    return dyed_frames.emplace(key,image).first->second;
}

void Mir3HudView::draw_human_layers(const MapActorFrame& body,Vector2 anchor,int type,int hair,int weapon,int motion_frame,int clothing_dye,int hair_dye,int mount_shape,float opacity,int status_color,bool corpse,bool highlighted,int appearance_weight) {
    const auto* hair_frame=attachment_frame(type,hair,false,motion_frame);
    const auto* weapon_frame=motion_frame>=2000?nullptr:attachment_frame(type,weapon,true,motion_frame);
    // 414870：武器影、坐骑独立影、身体资源投影均先于人物彩色图层。
    const auto shadow=[&](const MapActorFrame* frame) {
        if (!frame || frame->image.is_null()) return;
        const auto position=anchor+frame->offset;
        draw_terrain_texture(frame->image,position,Color(1,1,1),
            system_options.shadow_blend || status_color==1 || opacity<1.f || appearance_weight>=0?13:0);
        human_shadow_bounds.emplace_back(position,frame->image->get_size());
    };
    if (weapon_frame && motion_frame/80!=8 && !corpse) shadow(project_weapon_shadow(*weapon_frame));
    if (mount_shape && motion_frame>=2320) shadow(horse_shadow_frame(motion_frame));
    shadow(project_monster_shadow(body,corpse));
    const auto draw=[&](const MapActorFrame* frame,bool tint,int layer_weight) {
        if (frame && frame->image.is_valid()) {
            Ref<ShaderMaterial> appearance;
            if (appearance_weight>=0 && status_color!=1)
                appearance=status_color==65535?monster_appearance_mix[layer_weight]:monster_tint_appearance_mix;
            else if (highlighted && status_color==65535 && opacity==1.f)
                appearance=actor_highlight;
            draw_terrain_texture(dyed_frame(*frame,tint?clothing_dye:65535,tint?hair_dye:65535,status_color,highlighted),
                anchor+frame->offset,Color(1,1,1,status_color==1?1.f:opacity),status_color==1?8:0,false,
                appearance);
        }
    };
    // LegendOfMir3_Src Actor.cpp: weapon behind, body, hair, weapon in front.
    const bool behind=motion_frame>=0 && motion_frame<weapon_order.size() && weapon_order[motion_frame]!=0;
    const auto draw_weapon=[&]() {
        draw(weapon_frame,false,behind?10:22); // 414F90 / 4150F5
        if (weapon==46 && weapon_frame && weapon_frame->image.is_valid()) draw_weapon_glow(anchor,type,motion_frame);
    };
    if (mount_shape) draw(horse_frame(mount_shape,motion_frame),false,appearance_weight);
    if (behind) draw_weapon();
    draw(&body,true,appearance_weight);
    draw(hair_frame,true,20); // 41506F
    if (!behind) draw_weapon();
}

void Mir3HudView::draw_weapon_glow(Vector2 anchor,int type,int motion_frame) {
    // 414870 -> 415160：库148；光效与武器层同步，光源在缺少某帧时仍产生。
    const int index=(type?3900:1900)+motion_frame;
    weapon_glow_lights.push_back(anchor+Vector2(24,16));
    if (!monster_effect_archives.contains(148)) {
        mir3::WilArchive archive;
        if (!archive.open(std::filesystem::u8path(actor_resource_root.utf8().get_data())/"MonMagicEx4")) return;
        monster_effect_archives.emplace(148,std::move(archive));
    }
    auto found=weapon_glow_frames.find(index);
    if (found==weapon_glow_frames.end()) {
        const auto frame=monster_effect_archives.at(148).frame(index);
        if (!frame) return;
        found=weapon_glow_frames.emplace(index,MapActorFrame{texture(frame->pixels.rgba,frame->width,frame->height),
            Vector2(frame->offset_x,frame->offset_y),frame->pixels.regions}).first;
    }
    if (weapon_glow_mix.is_null()) {
        weapon_glow_mix.instantiate();
        weapon_glow_mix->set_shader(ResourceLoader::get_singleton()->load("res://level_effect.gdshader"));
        weapon_glow_mix->set_shader_parameter("effect_blend",2);
    }
    draw_terrain_texture(found->second.image,anchor+found->second.offset,Color(1,1,1),0,true);
    drawn_weapon_glow_frames.push_back(index);
}

const Mir3HudView::MapActorFrame* Mir3HudView::project_weapon_shadow(const MapActorFrame& frame) {
    const auto key=std::uint64_t(frame.image->get_instance_id());
    if (const auto found=weapon_shadows.find(key);found!=weapon_shadows.end()) return &found->second;
    const int width=frame.image->get_width(),height=frame.image->get_height();
    if (frame.regions.size()!=std::size_t(width)*height) return nullptr;
    // 48F7C7取奇数扫描行；投影位置以武器资源Y偏移减16后的一半为基准。
    const int half=(int(frame.offset.y)-16)/2,spread=height/2;
    const int out_width=width+spread,out_height=height/2+1;
    std::vector<std::uint8_t> rgba(std::size_t(out_width)*out_height*4);
    for (int row=1;row<height;row+=2) for (int x=0;x<width;++x)
        if (frame.regions[std::size_t(row)*width+x])
            rgba[(std::size_t((row+1)/2)*out_width+x+spread-row/2)*4+3]=255;
    return &weapon_shadows.emplace(key,MapActorFrame{texture(rgba,out_width,out_height),
        frame.offset-Vector2(half+spread,half),{}}).first->second;
}

const Mir3HudView::MapActorFrame* Mir3HudView::horse_shadow_frame(int motion_frame) {
    if (motion_frame<2320) return nullptr;
    const int index=10000+(motion_frame-2320)%400;
    if (const auto found=horse_frames.find(index);found!=horse_frames.end()) return &found->second;
    const auto source=map_horse.frame(index);
    if (!source) return nullptr;
    return &horse_frames.emplace(index,MapActorFrame{texture(source->pixels.rgba,source->width,source->height),
        Vector2(source->offset_x,source->offset_y),source->pixels.regions}).first->second;
}

const Mir3HudView::HumanBody* Mir3HudView::human_body(int type,int clothing,bool riding) {
    if (!terrain_ready || type<0 || type>1 || clothing<0 || clothing>=9) return nullptr;
    const auto key=std::pair{type,clothing};
    auto& cache=riding?riding_bodies:human_bodies;
    if (const auto found=cache.find(key);found!=cache.end()) return &found->second;
    auto& source=type?map_female:map_human;
    HumanBody body;
    // mir3_1.45 ActorAppearance::body_action_frame; also Actor.cpp's
    // bDress * _MAX_HERO_FRAME + direction * 10 in LegendOfMir3_Src.
    const auto load=[&](auto& frames,int first,int count) {
        for (int direction=0;direction<8;++direction) for (int relative=0;relative<count;++relative) {
            const int index=direction*10+relative;
            const auto frame=source.frame(clothing*3000+first+index);
            if (!frame) return false;
            frames[index]={texture(frame->pixels.rgba,frame->width,frame->height),Vector2(frame->offset_x,frame->offset_y),frame->pixels.regions,frame->shadow_mode,Vector2(frame->shadow_x,frame->shadow_y)};
        }
        return true;
    };
    if (!load(body.standing,riding?2320:0,4) || !load(body.walking,riding?2400:1680,6) || !load(body.running,riding?2480:1760,6)) return nullptr;
    return &cache.emplace(key,std::move(body)).first->second;
}

bool Mir3HudView::set_player_body(int type,int clothing) {
    const auto* body=human_body(type,clothing);
    if (!body || (player_mount && !human_body(type,clothing,true))) return false;
    map_human_frames=body->standing;
    map_walk_frames=body->walking;
    map_run_frames=body->running;
    player_type=type;
    player_clothing=clothing;
    queue_redraw();
    return true;
}

bool Mir3HudView::set_scene_human_body(int actor,int type,int clothing) {
    const auto found=scene_humans.find(actor);
    if (found==scene_humans.end()) return false;
    const auto* body=human_body(type,clothing);
    if (!body || (found->second.mount_shape && !human_body(type,clothing,true))) return false;
    auto& human=found->second;
    human.type=type;
    human.clothing=clothing;
    human.bounds=Rect2();
    std::copy_n(body->standing.begin()+human.facing*10,human.frames.size(),human.frames.begin());
    if (target_is_human && hovered_target==actor)
        set_hovered_human(actor,type,human.profession,human.name);
    queue_redraw();
    return true;
}

bool Mir3HudView::set_scene_human(int actor,Vector2i cell,int facing,int profession,String name) {
    if (actor==local_actor_identity) return false;
    if (!terrain_ready || actor<=0 || facing<0 || facing>=8 || profession<0 || profession>2 ||
        scene_npcs.contains(actor) || scene_chickens.contains(actor) || !terrain.contains(cell.x,cell.y)) return false;
    SceneHuman human{};
    human.cell=cell;
    human.facing=facing;
    human.profession=profession;
    human.name=std::move(name);
    std::copy_n(human_body(human.type,human.clothing)->standing.begin()+facing*10,human.frames.size(),human.frames.begin());
    if (human.frames.front().image.is_null()) return false;
    mir3::SceneName identity;
    if (!identity.update(42,255,mir3::utf8_to_ansi(human.name.utf8().get_data()))) return false;
    scene_names.insert_or_assign(actor,std::move(identity));
    scene_humans.insert_or_assign(actor,std::move(human));
    queue_redraw();
    return true;
}

const Mir3HudView::MapActorFrame* Mir3HudView::human_action_frame(int type,int clothing,int index) {
    const auto key=std::tuple{type,clothing,index};
    if (auto it=human_action_frames.find(key);it!=human_action_frames.end()) return &it->second;
    const auto frame=(type?map_female:map_human).frame(clothing*3000+index);
    if (!frame) return nullptr;
    return &human_action_frames.emplace(key,MapActorFrame{texture(frame->pixels.rgba,frame->width,frame->height),
        Vector2(frame->offset_x,frame->offset_y),frame->pixels.regions,frame->shadow_mode,Vector2(frame->shadow_x,frame->shadow_y)}).first->second;
}

bool Mir3HudView::apply_scene_human_life(int actor,int opcode,Vector2i cell,int facing) {
    const auto it=scene_humans.find(actor);
    if (it==scene_humans.end() || (opcode!=mir3::protocol::SM_DEATH && opcode!=mir3::protocol::SM_NOWDEATH && opcode!=mir3::protocol::SM_REVIVE && opcode!=mir3::protocol::SM_REVIVE_REFRESH) ||
        facing<0 || facing>=8 || !terrain.contains(cell.x,cell.y)) return false;
    auto& human=it->second;
    const bool revive=opcode==mir3::protocol::SM_REVIVE || opcode==mir3::protocol::SM_REVIVE_REFRESH;
    if (!revive) {
        for (int i=0;i<10;++i)
            if (!human_action_frame(human.type,human.clothing,mir3::HumanActions::frame(19,facing,i))) return false;
    }
    apply_scene_human_stand(actor,cell,facing);
    if (revive) {
        human.life.revive();
        human.combat.clear();
        human.arrival_ms=0;
        human.appearance_duration_ms=SceneHuman::revive_duration_ms; // 4123C2
    }
    else {
        if (human.life.die(opcode==mir3::protocol::SM_NOWDEATH)) human.action.start(19);
        if (hovered_target==static_cast<std::uint32_t>(actor)) set_hovered_target(0,0,0,false,0,0,0,0);
        target_focus.remove(actor);
    }
    queue_redraw();
    return true;
}

bool Mir3HudView::set_local_actor_identity(int actor) {
    if (actor<=0 || scene_humans.contains(actor) || scene_npcs.contains(actor) || scene_chickens.contains(actor)) return false;
    if (local_actor_identity!=actor) {
        player_light_attribute=0;melee_cooldown={};local_action_clock={};spell_timing={};melee_spells={};player_melee_sound=0;active_spell=0;player_spell={};pending_local_action.reset();
        action_prediction={};
        local_actor_messages.clear();forced_motion.reset();
        player_life={};local_struck_tick=0;local_folded_hits=0;local_hit_weapon=0;local_last_monster_hit_shape.reset();player_status_flags=0;
    }
    local_actor_identity=actor;
    queue_redraw();
    return true;
}

bool Mir3HudView::apply_scene_message(PackedByteArray encoded,PackedByteArray key) {
    if (server_groups.visible || login_visible || character_selection.visible) return false;
    if (encoded.size()<mir3::ReceiveHeader::encoded_size || encoded.size()>mir3::ReceiveHeader::encoded_size+mir3::ChatLog::max_encoded_text) return false;
    const auto header=mir3::ReceiveHeader::decode({encoded.ptr(),static_cast<std::size_t>(encoded.size())},
        {key.ptr(),static_cast<std::size_t>(key.size())});
    if (!header || (header->recognition>INT_MAX && header->opcode!=mir3::protocol::SM_REVIVE_REFRESH && header->opcode!=mir3::protocol::SM_NEWMAP && header->opcode!=mir3::protocol::SM_CHANGEMAP && header->opcode!=mir3::protocol::SM_MAPDESCRIPTION && header->opcode!=mir3::protocol::SM_CLIENT_OPTIONS && header->opcode!=mir3::protocol::SM_DAYCHANGING && header->opcode!=mir3::protocol::SM_SHOWEVENT && header->opcode!=mir3::protocol::SM_HIDEEVENT)) return false;
    const int actor=header->recognition, opcode=header->opcode;
    const auto& p=header->parameters;
    if (opcode==mir3::protocol::SM_MAGICFIRE || opcode==mir3::protocol::SM_MAGICFIRE_LIST) {
        const auto reply=mir3::SpellReply::parse(*header,{encoded.ptr()+16,std::size_t(encoded.size()-16)});
        if (!reply) return false;
        if (local_actor_identity && actor==local_actor_identity) {
            if (local_actor_messages.size()>=64) return false;
            LocalActorMessage message{{opcode,0,{}},{},0};message.spell_reply=*reply;
            local_actor_messages.push_back(std::move(message));return true;
        }
        const auto found=scene_humans.find(actor);
        if (found==scene_humans.end()) return false;
        mir3::SceneHumanMessage message;message.opcode=opcode;message.spell_reply=*reply;
        return enqueue_scene_human_message(actor,std::move(message));
    }
    if (local_actor_identity && actor==local_actor_identity &&
        (opcode==mir3::protocol::SM_SITDOWN || opcode==mir3::protocol::SM_BUTCH)) {
        // 415AD0没有12/637动作分支，但仍按FIFO逐条消费，不能中断本地预测动作。
        if (local_actor_messages.size()>=64) return false;
        local_actor_messages.push_back({{opcode,0,{}},{},0});
        return true;
    }
    if (local_actor_identity && actor==local_actor_identity &&
        (opcode==mir3::protocol::SM_STRUCK || opcode==mir3::protocol::SM_HUMAN_STRUCK)) {
        if (encoded.size()!=38 || local_actor_messages.size()>=64) return false;
        const auto body=mir3::decode_six_bit(encoded.ptr()+16,22);
        if (!body || body->size()!=16) return false;
        const auto appearance=mir3::HumanAppearance::parse({reinterpret_cast<const std::uint8_t*>(body->data()),body->size()},true);
        if (!appearance) return false;
        mir3::SceneHumanMessage hit{opcode};
        hit.health=p;
        for (int i=0;i<4;++i) hit.hit_source|=std::uint32_t(std::uint8_t((*body)[8+i]))<<(8*i);
        const unsigned variant=std::uint8_t((*body)[14])|(unsigned(std::uint8_t((*body)[15]))<<8);
        if (opcode==mir3::protocol::SM_STRUCK) {
            // 42EC70：特殊受击后的500ms内，非致死31只同步生命数值。
            if (std::uint32_t(ticks_msec())-local_struck_tick<500) {
                model.hp=p[0];model.max_hp=p[1];
                if (p[0]) { queue_redraw();return true; }
            }
            // 40D7F0：仅当队长大于1时，折叠已排队的31，保留其他消息顺序。
            local_folded_hits=0;
            if (local_actor_messages.size()>1) {
                for (auto it=local_actor_messages.begin();it!=local_actor_messages.end();) {
                    if (it->hit && it->motion.opcode==mir3::protocol::SM_STRUCK) {
                        const auto& hp=*it->hit->health;
                        model.hp=hp[0];model.max_hp=hp[1];
                        apply_actor_health_record(0,*it->hit);
                        ++local_folded_hits;
                        it=local_actor_messages.erase(it);
                    } else ++it;
                }
            }
        }
        local_actor_messages.push_back({{opcode,player_facing,{player_cell.x,player_cell.y}},*appearance,0,hit,variant});
        return true;
    }
    const bool local_death=opcode==mir3::protocol::SM_DEATH || opcode==mir3::protocol::SM_NOWDEATH;
    if ((mir3::ForcedMotion::accepts(opcode) || local_death) && local_actor_identity &&
            (actor==local_actor_identity || opcode==mir3::protocol::SM_REVIVE_REFRESH)) {
        if ((encoded.size()!=45 && !(local_death && encoded.size()==38)) || (p[2]&255)>=8 || local_actor_messages.size()>=64) return false;
        const auto base=mir3::decode_six_bit(encoded.ptr()+16,22);
        const auto extra=encoded.size()==45?mir3::decode_six_bit(encoded.ptr()+38,7):std::optional<std::string>(std::string(5,0));
        if (!base || base->size()!=16 || !extra || extra->size()!=5) return false;
        const auto body=*base+*extra;
        const auto appearance=mir3::HumanAppearance::parse({reinterpret_cast<const std::uint8_t*>(body.data()),body.size()},false);
        const mir3::ForcedMotion motion{opcode,p[2]&255,{p[0],p[1]}};
        const auto origin=motion.origin();
        if (!appearance || !terrain.contains(p[0],p[1]) || (!local_death && !terrain.contains(origin[0],origin[1]))) return false;
        local_actor_messages.push_back({motion,*appearance,unsigned(p[2]>>8)});
        return true;
    }
    const bool human_transfer=opcode==mir3::protocol::SM_801 || opcode==mir3::protocol::SM_ACTOR_STATUS || opcode==mir3::protocol::SM_808;
    // 413F90 +116 -> 411BA0. Extended records can also supply the creation feature.
    if (human_transfer &&
            (encoded.size()==16 || (local_actor_identity && actor==local_actor_identity)) &&
            ((local_actor_identity && actor==local_actor_identity) || scene_humans.contains(actor))) {
        if (encoded.size()!=16) return false;
        mir3::SceneHumanMessage status;
        status.opcode=opcode;status.flags=std::uint32_t(p[0])|(std::uint32_t(p[1])<<16);
        status.status_speed=std::int16_t(p[2]);
        if (actor==local_actor_identity) {
            if (local_actor_messages.size()>=64) return false;
            LocalActorMessage message{{opcode,0,{}},{},0};message.status=status;
            local_actor_messages.push_back(message);return true;
        }
        return enqueue_scene_human_message(actor,std::move(status));
    }
    // 42E9A0 constructs type 4 and 5 actors from these entry messages.
    const bool npc_entry=opcode==mir3::protocol::SM_TURN || opcode==mir3::protocol::SM_BACKSTEP ||
        opcode==mir3::protocol::SM_WALK || opcode==mir3::protocol::SM_RUN || opcode==mir3::protocol::SM_DIGUP ||
        opcode==mir3::protocol::SM_27 || opcode==mir3::protocol::SM_DEATH || opcode==mir3::protocol::SM_SKELETON ||
        opcode==mir3::protocol::SM_801 || opcode==mir3::protocol::SM_ACTOR_STATUS || opcode==mir3::protocol::SM_808;
    const bool npc_revive=opcode==mir3::protocol::SM_REVIVE && scene_npcs.contains(actor);
    if ((npc_entry || npc_revive) && encoded.size()>=38 && actor!=local_actor_identity) {
        const auto feature=mir3::decode_six_bit(encoded.ptr()+16,22);
        const bool npc_feature=feature && feature->size()==16 && ((*feature)[0]==4 || (*feature)[0]==5);
        if (npc_feature && npc_entry && (scene_clear_pending || !scene_removals.empty())) apply_scene_removals();
        const bool refresh=scene_npcs.contains(actor);
        const bool place_npc=npc_feature && (!refresh || opcode==mir3::protocol::SM_TURN || npc_revive);
        const Vector2i cell(p[0],p[1]);
        const auto place_wire_npc=[&](int first,int count,int interval,bool flag=false,
                int idle_first=-1,int idle_count=0,int idle_interval=300) {
            if (!place_scene_npc(actor,cell,first,count,interval,false,flag,idle_first,idle_count,idle_interval,npc_revive))
                return false;
            // 4057BF starts at zero. 4102C2 applies the high byte on refresh.
            if (refresh) {
                scene_npcs.at(actor).light_attribute=p[2]>>8;
                // 410352 reads the flags after the first four feature bytes.
                update_actor_bar_status(actor,((*feature)[4]&2)!=0);
            }
            // 41030D decodes the optional name after the 22 encoded feature bytes on refresh.
            if (refresh && encoded.size()>38) {
                const auto name=mir3::decode_six_bit(encoded.ptr()+38,encoded.size()-38);
                if (name && name->size()<=64) {
                    const auto ansi=std::string_view(*name).substr(0,name->find('\0'));
                    const auto slash=ansi.rfind('/');
                    const auto previous=scene_names.find(actor);
                    const auto index=slash==ansi.npos?255:std::uint8_t(std::strtol(name->c_str()+slash+1,nullptr,10));
                    mir3::SceneName identity;
                    if (identity.update(mir3::protocol::SM_USERNAME,index,ansi)) {
                        if (slash==ansi.npos && previous!=scene_names.end()) identity.color=previous->second.color;
                        scene_names.insert_or_assign(actor,std::move(identity));
                    }
                }
            }
            return true;
        };
        if (place_npc && (*feature)[0]==4) {
            // 406073 uses feature byte 1 and a 100-frame stride in Flag.
            const int shape=std::uint8_t((*feature)[1]);
            return terrain.contains(p[0],p[1]) &&
                place_wire_npc(100*shape,shape==10?10:4,shape==10?150:300,true);
        }
        if (place_npc) {
            // 4061AE nests the ten-frame direction stride inside a 100-frame shape stride.
            const int shape=std::uint8_t((*feature)[2]);
            int count=4, interval=300;
            switch (shape) { // 46F3B0, standing (action 0)
            case 51: case 52: case 53: case 54: case 55: case 59: case 64: case 65:
                count=1; break;
            case 56: case 57: count=12; interval=80; break;
            case 87: count=10; interval=150; break;
            }
            if (shape>=100 || !terrain.contains(p[0],p[1]) || (refresh && (p[2]&255)>=8)) return false;
            if (refresh && shape!=87) {
                // 410399 selects action 1. 406750 folds some shapes back to action 0.
                const int direction=(shape==40 || count!=4)?0:(p[2]&255)%3;
                const bool idle_only=count!=4 || shape==24 || shape==25 || shape==34 || shape==35 ||
                    (shape>=43 && shape<=50) || shape==58 || shape==69 || shape==76 ||
                    shape==80 || (shape>=82 && shape<=86);
                const int first=100*shape+10*direction;
                if (idle_only) return place_wire_npc(first,count,interval);
                const bool shared=shape==19 || shape==88 || shape==89;
                const int action_frames=shared?4:shape==27?6:shape==28?9:10;
                return place_wire_npc(first+(shared?0:30),action_frames,300,false,first,count,interval);
            }
            if (!place_wire_npc(100*shape,count,interval)) return false;
            if (refresh && shape==87) {
                // 406D03 binds spell 607 to the NPC and uses direction modulo three.
                show_scene_effect(mir3::EffectCatalog::Section::spell,607,cell,10*((p[2]&255)%3),0,actor,
                    std::nullopt,MonsterEffectPose::idle);
                play_actor_sound(actor,16070,cell);
            }
            return true;
        }
    }
    if (const auto monster_result=apply_monster_wire(opcode,actor,p,encoded)) return *monster_result;
    if (scene_npcs.contains(actor) && (opcode==mir3::protocol::SM_OPENHEALTH || opcode==mir3::protocol::SM_CLOSEHEALTH)) {
        if (encoded.size()!=16) return false;
        // NPC vtable +52/+56 uses 407C20/407C50. 4247AF omits NPCs from bar drawing.
        update_actor_bar_status(actor,opcode==mir3::protocol::SM_OPENHEALTH);
        return true;
    }
    // 4282C0虚表+52/+56：所有已存在角色即时更新，与动作队列无关。
    if (scene_chickens.contains(actor) || scene_humans.contains(actor) || (local_actor_identity && actor==local_actor_identity)) {
        if (opcode==mir3::protocol::SM_OPENHEALTH) {
            if (encoded.size()!=16) return false;
            if (actor==local_actor_identity) player_light_attribute=p[0]&255;
            else if (scene_humans.contains(actor)) scene_humans.at(actor).light_attribute=p[0]&255;
            else scene_chickens.at(actor).light_attribute=p[0]&255;
            queue_redraw();
            return true;
        }
        if (opcode==mir3::protocol::SM_CLOSEHEALTH) {
            if (encoded.size()!=16) return false;
            update_actor_bar_status(actor==local_actor_identity?0:actor,true);
            return true;
        }
    }
    if (opcode==mir3::protocol::SM_HIT2 && local_actor_identity && actor==local_actor_identity) {
        if (encoded.size()>17 || (p[2]&255)>=8) return false;
        if (player_life.dead) return true;
        if (player_action.finished() || player_action.action()!=mir3::HumanActions::weapon_attack(player_weapon) ||
            player_facing!=(p[2]&255) || player_action.frame()>=3) {
            walk_frame=-1;movement_ms=0;map_route.clear();
            player_facing=p[2]&255;
            player_action.start(mir3::HumanActions::weapon_attack(player_weapon));
        }
        player_harvest_impact=encoded.size()==17 && encoded[16]!=0;
        queue_redraw();
        return true;
    }
    if(opcode==mir3::protocol::SM_HIDEEVENT) {
        if(encoded.size()!=16) return false;
        if(const auto ground=ground_events.find(header->recognition);ground!=ground_events.end() && ground->second.frame!=220)
            ground->second.hidden=true;
        const auto event=map_events.find(header->recognition);
        if(event!=map_events.end()) {
            auto& visual=event->second;
            if(visual.resource->id==22) {
                const int duration=weather.between(20000,30000);
                const auto* tail=effect_catalog.find(mir3::EffectCatalog::Section::magic,weather.between(330,334));
                if(tail && tail->library==81 && tail->end>tail->first && tail->interval) {
                    visual.resource=tail;
                    visual.frame=tail->first;
                    visual.elapsed=0;
                    visual.remaining_ms=duration;
                    visual.sprite->set_modulate(Color(1,1,1));
                    ground_events.emplace(header->recognition,GroundEvent{visual.cell,220,0,false,double(duration)+800,180.f/255});
                    advance_map_events(0);
                    return true;
                }
            }
            if(visual.secondary) visual.ending=true;
            else if(visual.remaining_ms>=0) return true;
            else {
                event->second.copy->hide();
                event->second.copy->queue_free();
                map_events.erase(event);
            }
        }
        return true;
    }
    if(opcode==mir3::protocol::SM_SHOWEVENT) {
        const auto body=mir3::decode_six_bit(encoded.ptr()+16,encoded.size()-16);
        if(!body || body->size()!=4 || !terrain.contains(p[1],p[2])) return false;
        if(map_events.contains(header->recognition) || ground_events.contains(header->recognition)) return true;
        if(p[0]==3 || p[0]==6) {
            ground_events.emplace(header->recognition,GroundEvent{{p[1],p[2]},p[0]==3?230:210});
            return true;
        }
        if(p[0]!=4 && p[0]!=5 && p[0]!=9) return false;
        const bool layered=p[0]==9;
        const auto* resource=effect_catalog.find(mir3::EffectCatalog::Section::magic,layered?140:p[0]==4?16:22);
        if(!resource || resource->library!=(layered?137:81) || resource->end<=resource->first || !resource->interval) return false;
        auto* copy=memnew(BackBufferCopy);
        copy->set_copy_mode(BackBufferCopy::COPY_MODE_VIEWPORT);
        level_layer->add_child(copy);
        auto* sprite=memnew(Sprite2D);
        sprite->set_centered(false);
        copy->add_child(sprite);
        Ref<ShaderMaterial> material;
        material.instantiate();
        material->set_shader(ResourceLoader::get_singleton()->load("res://level_effect.gdshader"));
        const auto tint=Color(resource->tint[0]/255.f,resource->tint[1]/255.f,resource->tint[2]/255.f);
        sprite->set_material(material);
        sprite->set_modulate(tint);
        const int frame=layered?resource->first:resource->first+weather.between(1,8)%(resource->end-resource->first);
        auto& event=map_events.emplace(header->recognition,MapEvent{resource,{p[1],p[2]},copy,sprite,frame}).first->second;
        if(layered) {
            auto* secondary_copy=memnew(BackBufferCopy);
            secondary_copy->set_copy_mode(BackBufferCopy::COPY_MODE_VIEWPORT);
            copy->add_child(secondary_copy);
            event.secondary=memnew(Sprite2D);
            event.secondary->set_centered(false);
            event.secondary->set_material(material);
            event.secondary->set_modulate(tint);
            secondary_copy->add_child(event.secondary);
        }
        advance_map_events(0);
        return true;
    }
    if(opcode==mir3::protocol::SM_OPENDOOR_OK || opcode==mir3::protocol::SM_CLOSEDOOR ||
            opcode==mir3::protocol::SM_OPENDOOR_LOCK) {
        if(encoded.size()!=16) return false;
        if(opcode==mir3::protocol::SM_OPENDOOR_LOCK)
            post_chat(String::utf8(message(1472).c_str()),11861940,0);
        else if(terrain.update_door(p[0],p[1],opcode==mir3::protocol::SM_OPENDOOR_OK))
            queue_redraw();
        return true;
    }
    if (opcode==mir3::protocol::SM_NEWMAP || opcode==mir3::protocol::SM_CHANGEMAP || opcode==mir3::protocol::SM_MAPDESCRIPTION) {
        const auto decoded=mir3::decode_six_bit(encoded.ptr()+16,encoded.size()-16);
        if (!decoded) return false;
        if (opcode==mir3::protocol::SM_MAPDESCRIPTION) {
            const auto ansi=decoded->substr(0,decoded->find(char(0)));
            if (ansi.size()>259) return false;
            // 0x4282C0: description changes do not reset the map or its overlays.
            model.map_name=mir3::ansi_to_utf8(ansi);
            queue_redraw();return true;
        }
        const auto message=mir3::MapChangeMessage::parse(opcode,p,*decoded);
        if (!message || map_changes.size()>=99999) return false;
        map_changes.push_back(*message);return true;
    }
    if (opcode==mir3::protocol::SM_CLIENT_OPTIONS) {
        if (encoded.size()!=16) return false;
        const auto changed=system_options.receive(header->recognition,p[0]);
        reception_buttons.receive_options(changed,rejected_chat);
        queue_redraw();return true;
    }
    if (opcode==mir3::protocol::SM_DAYCHANGING) {
        if (encoded.size()!=16) return false;
        scene_ambient.select_base(std::uint8_t(p[0]),weather.random());
        scene_ambient.update(p[1]);
        weather.select(p[2]);update_weather_layers();
        queue_redraw();
        return true;
    }
    const int size=encoded.size()-16;
    if (opcode==mir3::protocol::SM_ITEMHIDE) {
        if (size) return false;
        ground_items.hide(actor);ground_bounds.erase(actor);queue_redraw();return true;
    }
    if (opcode==mir3::protocol::SM_ITEMSHOW) {
        if (!terrain_ready || actor<=0 || !terrain.contains(p[0],p[1])) return false;
        const auto decoded=mir3::decode_six_bit(encoded.ptr()+16,size);
        if (!decoded) return false;
        auto name=decoded->substr(0,decoded->find(char(0)));
        int stack_mode=0;
        // 429869 splits the optional stack mode from the ground name at the first slash.
        if (const auto slash=name.find('/');slash!=name.npos) {
            stack_mode=std::strtol(name.c_str(),nullptr,10);
            name.erase(0,slash+1);
        }
        if (name.size()>39) return false;
        if (!ground_frames.contains(p[2])) {
            const auto source=ground_archive.frame(p[2]);
            auto& frame=ground_frames[p[2]];
            if (source) frame={texture(source->pixels.rgba,source->width,source->height),{},source->pixels.regions};
        }
        const auto previous_count=ground_items.items.size();
        const bool accepted=ground_items.show({actor,p[0],p[1],p[2],name,stack_mode});
        if (ground_items.items.size()!=previous_count) {
            auto& item=ground_items.items.back();
            item.sparkle_started_tick=GetTickCount();
            // 401710 takes the absolute difference between two calls to the CRT PRNG at 49FDAE.
            const int first_roll=std::rand();
            item.sparkle_delay_ms=mir3::GroundItems::sparkle_delay(first_roll,std::rand());
            queue_redraw();
        }
        return accepted;
    }
    // 4282C0 routes all four headers through 42C154 to the 42E780 removal queue.
    if (opcode==mir3::protocol::SM_29 || opcode==mir3::protocol::SM_DISAPPEAR ||
        opcode==mir3::protocol::SM_803 || opcode==mir3::protocol::SM_CLEAROBJECTS) {
        if (size || scene_removals.size()>=99999) return false;
        if (opcode==mir3::protocol::SM_CLEAROBJECTS) scene_clear_pending=true;
        else if (actor>0) scene_removals.push_back(actor);
        return true;
    }
    if (opcode==mir3::protocol::SM_CHANGENAMECOLOR) {
        if (!scene_chickens.contains(actor))
            return size==0 && apply_scene_name(actor,opcode,p[0]&255,String());
        const auto decoded=mir3::decode_six_bit(encoded.ptr()+16,size);
        if (!decoded) return false;
        const auto name=std::string_view(*decoded).substr(0,decoded->find(char(0)));
        mir3::SceneName identity;
        if (!identity.update(mir3::protocol::SM_USERNAME,p[0]&255,name)) return false;
        scene_names.insert_or_assign(actor,std::move(identity));
        queue_redraw();
        return true;
    }
    if (opcode==mir3::protocol::SM_USERNAME || mir3::chat_opcode(opcode)) {
        if(mir3::chat_opcode(opcode) && size>mir3::ChatLog::max_encoded_text) return false;
        const auto decoded=mir3::decode_six_bit(encoded.ptr()+16,size);
        if (!decoded) return false;
        const auto ansi=std::string_view(*decoded).substr(0,decoded->find(char(0)));
        if ((opcode==mir3::protocol::SM_HEAR || opcode==mir3::protocol::SM_MONSTERSAY) && ansi.size()>259) return false;
        const auto text=String::utf8(mir3::ansi_to_utf8(ansi).c_str());
        if (opcode==mir3::protocol::SM_USERNAME) return apply_scene_name(actor,opcode,p[0]&255,text);
        // 42F380：聊天窗口显示一次，文件独立分支可能追加两次；40/1501另更新头顶文字。
        if (opcode==mir3::protocol::SM_HEAR || opcode==mir3::protocol::SM_MONSTERSAY) {
            const bool present=actor==0 || scene_humans.contains(actor) || scene_npcs.contains(actor) || scene_chickens.contains(actor);
            if (present && !apply_scene_speech(actor,opcode,text)) return false;
        }
        append_chat_log(opcode,ansi);
        post_chat(text,mir3::chat_palette(opcode==mir3::protocol::SM_GUILDMESSAGE?4:p[0]&255),mir3::chat_palette(opcode==mir3::protocol::SM_GUILDMESSAGE?0:p[1]&255));
        return true;
    }
    if (opcode==mir3::protocol::SM_HEALTHSPELLCHANGED || opcode==mir3::protocol::SM_HEALTH_PERCENT)
        return size==0 && apply_scene_health(local_actor_identity && actor==local_actor_identity?0:actor,opcode,p[0],p[1],p[2]);
    auto found=scene_humans.find(actor);
    const bool backstep=opcode==mir3::protocol::SM_BACKSTEP;
    const bool transfer=opcode==mir3::protocol::SM_800 || opcode==mir3::protocol::SM_806;
    const bool named_entry=transfer || opcode==mir3::protocol::SM_TURN ||
        opcode==mir3::protocol::SM_REVIVE || opcode==mir3::protocol::SM_REVIVE_REFRESH;
    const bool death=opcode==mir3::protocol::SM_DEATH || opcode==mir3::protocol::SM_NOWDEATH;
    const bool spawn_without_action=opcode==mir3::protocol::SM_DIGUP || opcode==mir3::protocol::SM_27 ||
        opcode==mir3::protocol::SM_SKELETON;
    const bool creates_human=backstep || opcode==mir3::protocol::SM_TURN || opcode==mir3::protocol::SM_WALK ||
        opcode==mir3::protocol::SM_RUN || opcode==mir3::protocol::SM_DEATH || spawn_without_action || human_transfer;
    if ((backstep && size!=29) || (transfer && size<29)) return false;
    if (!mir3::SceneHumanMessage::accepts(opcode) ||
        (found==scene_humans.end() && !creates_human)) return false;
    mir3::SceneHumanMessage message{opcode,p[0],p[1],opcode==mir3::protocol::SM_HUMAN_STRUCK || opcode==mir3::protocol::SM_STRUCK?0:p[2]&255};
    message.weapon_element=std::uint8_t(p[2]>>8);
    // Remote walking and running share the header light update at 4120C4 / 412214.
    if (backstep || death || named_entry || opcode==mir3::protocol::SM_WALK ||
        opcode==mir3::protocol::SM_RUN) message.light_attribute=p[2]>>8;
    if (opcode==mir3::protocol::SM_SPELL) {
        // 413C80：正文是原始十进制技能文本，不是六位编码后的外观记录。
        const std::string text(reinterpret_cast<const char*>(encoded.ptr()+16),size);
        message.spell=static_cast<std::uint16_t>(std::strtol(text.c_str(),nullptr,10));
        message.facing=0; // 出队时按当前角色格与目标格求方向。
    } else if (opcode==mir3::protocol::SM_STRUCK && !size) {
        if (size) return false;
        message.health=p;
    } else if ((creates_human && !spawn_without_action) || death || transfer || opcode==mir3::protocol::SM_BUMP || opcode==mir3::protocol::SM_STRUCK || opcode==mir3::protocol::SM_HUMAN_STRUCK || opcode==mir3::protocol::SM_REVIVE || opcode==mir3::protocol::SM_REVIVE_REFRESH ||
        (spawn_without_action && size==29)) {
        const bool required=opcode==mir3::protocol::SM_TURN || opcode==mir3::protocol::SM_HUMAN_STRUCK || opcode==mir3::protocol::SM_REVIVE || opcode==mir3::protocol::SM_REVIVE_REFRESH;
        if (size || required) {
            if (size!=22 && size!=29 && !named_entry) return false;
            if (human_transfer && size!=29) return false;
            if ((opcode==mir3::protocol::SM_TURN || opcode==mir3::protocol::SM_REVIVE || opcode==mir3::protocol::SM_REVIVE_REFRESH) && size<29) return false;
            auto body=mir3::decode_six_bit(encoded.ptr()+16,22);
            if (!body || body->size()!=16) return false;
            if (size>=29) {
                const auto extra=mir3::decode_six_bit(encoded.ptr()+38,7);
                if (!extra || extra->size()!=5) return false;
                *body+=*extra;
            }
            const bool hit=opcode==mir3::protocol::SM_HUMAN_STRUCK || opcode==mir3::protocol::SM_STRUCK;
            if (hit && size!=22) return false;
            message.appearance=mir3::HumanAppearance::parse({reinterpret_cast<const std::uint8_t*>(body->data()),body->size()},hit);
            if (!message.appearance) return false;
            // 4120A0 / 4121F0 read the mount from the separate extension.
            if (size==22 && (opcode==mir3::protocol::SM_WALK || opcode==mir3::protocol::SM_RUN))
                message.appearance->mount.reset();
            message.flags=message.appearance->flags;
            if (named_entry && size>29) {
                message.transfer_name=mir3::decode_six_bit(encoded.ptr()+45,size-29);
                if (!message.transfer_name) return false;
            }
            if (hit) {
                message.health=p;
                message.hit_variant=std::uint8_t((*body)[14])|(std::uint16_t(std::uint8_t((*body)[15]))<<8);
                for (int i=0;i<4;++i) message.hit_source|=std::uint32_t(std::uint8_t((*body)[8+i]))<<(8*i);
            }
        }
    } else if(opcode==mir3::protocol::SM_HIT2) {
        if(size>1) return false;
        message.harvest=size==1 && encoded[16]!=0;
    } else if(size && !spawn_without_action && opcode!=mir3::protocol::SM_SITDOWN && opcode!=mir3::protocol::SM_BUTCH) return false;
    const bool server_step=backstep || opcode==mir3::protocol::SM_WALK || opcode==mir3::protocol::SM_RUN;
    // 41419D dispatches these attacks to 412C80, which writes the packet coordinates.
    const bool server_attack=opcode==mir3::protocol::SM_HIT || opcode==mir3::protocol::SM_HIT2 ||
        opcode==mir3::protocol::SM_FIREHIT || opcode==mir3::protocol::SM_POWERHIT ||
        opcode==mir3::protocol::SM_LONGHIT || opcode==mir3::protocol::SM_WIDEHIT ||
        opcode==mir3::protocol::SM_WIDEHIT2 || opcode==mir3::protocol::SM_HUMAN_ATTACK;
    const bool server_position=named_entry || server_step || death || server_attack ||
        opcode==mir3::protocol::SM_SITDOWN || opcode==mir3::protocol::SM_BUTCH;
    if (server_position && !terrain.contains(message.x,message.y)) return false;
    if (!server_position && !spawn_without_action && opcode!=mir3::protocol::SM_SPELL &&
        opcode!=mir3::protocol::SM_HUMAN_STRUCK && opcode!=mir3::protocol::SM_STRUCK &&
        !terrain.walkable(message.x,message.y)) return false;
    const bool full_creation_feature=size==29 || (named_entry && size>29);
    if (human_transfer) {
        message.flags=std::uint32_t(p[0])|(std::uint32_t(p[1])<<16);
        message.status_speed=std::int16_t(p[2]);
        message.facing=0;
    }
    if (creates_human && full_creation_feature && (scene_clear_pending || !scene_removals.empty())) {
        apply_scene_removals();
        found=scene_humans.find(actor);
    }
    if (found!=scene_humans.end()) return enqueue_scene_human_message(actor,std::move(message));
    // 42E9A0 requires both feature blocks to initialize an unknown human.
    if (!full_creation_feature || !message.appearance || (!spawn_without_action && message.facing>=8)) return false;
    if (!set_scene_human(actor,{message.x,message.y},0,0,String())) return false;
    if (!apply_scene_appearance(actor,*message.appearance)) {
        remove_scene_actor(actor);
        return false;
    }
    auto& human=scene_humans.at(actor);
    human.arrival_ms=0;
    human.action_flags=message.appearance->flags;
    if (spawn_without_action) message.facing=0;
    return human.messages.push(message);
}

bool Mir3HudView::enqueue_scene_human_message(int actor,mir3::SceneHumanMessage message) {
    auto& queue=scene_humans.at(actor).messages;
    // 42EDC7 folds pending 31 records before appending a routed message.
    for (auto& update:queue.fold_health()) {
        update.hit_variant.reset();
        apply_actor_health_record(actor,update);
    }
    return queue.push(std::move(message));
}

void Mir3HudView::apply_actor_health_record(int actor,const mir3::SceneHumanMessage& message) {
    if (!message.health) return;
    auto& health=scene_health[actor];
    const auto& p=*message.health;
    health.current=p[0];health.maximum=p[1];health.empty_bar=false;
    health.damage.enabled=damage_numbers;
    const auto remote=scene_humans.find(actor);
    const bool remote_dead=remote!=scene_humans.end() && remote->second.life.dead;
    const auto monster=scene_chickens.find(actor);
    const bool monster_death_action=monster!=scene_chickens.end() &&
        monster->second.death_frame>=0 && !monster->second.skeleton;
    if (!remote_dead && !monster_death_action && (message.opcode==mir3::protocol::SM_HUMAN_STRUCK ||
        (actor!=0 && message.opcode==mir3::protocol::SM_STRUCK && message.hit_variant)) &&
        (actor!=0 || !player_life.dead)) health.damage.push(-int(p[2]));
    queue_redraw();
}

bool Mir3HudView::queue_scene_human_state(int actor,int opcode,Vector2i cell,int facing,const PackedByteArray& body,int hit_weapon) {
    switch (opcode) {
    case mir3::protocol::SM_BUMP: case mir3::protocol::SM_TURN: case mir3::protocol::SM_WALK: case mir3::protocol::SM_RUN: case mir3::protocol::SM_HUMAN_STRUCK: case mir3::protocol::SM_REVIVE: case mir3::protocol::SM_REVIVE_REFRESH: break;
    default: return false;
    }
    const auto found=scene_humans.find(actor);
    if (found==scene_humans.end() || (opcode!=mir3::protocol::SM_HUMAN_STRUCK && !terrain.walkable(cell.x,cell.y))) return false;
    if ((opcode==mir3::protocol::SM_TURN || opcode==mir3::protocol::SM_REVIVE || opcode==mir3::protocol::SM_REVIVE_REFRESH) && body.size()!=21) return false;
    const auto appearance=mir3::HumanAppearance::parse({body.ptr(),static_cast<std::size_t>(body.size())},opcode==mir3::protocol::SM_HUMAN_STRUCK);
    if (!appearance) return false;
    return found->second.messages.push({opcode,cell.x,cell.y,facing,hit_weapon,appearance->flags,appearance});
}

bool Mir3HudView::apply_scene_appearance(int actor,const mir3::HumanAppearance& appearance) {
    auto& human=scene_humans.at(actor);
    const int mount=appearance.mount.value_or(human.mount_shape);
    const auto* body=human_body(appearance.type,appearance.clothing);
    if (!body || (mount && (!human_body(appearance.type,appearance.clothing,true) || !horse_frame(mount,2320))) ||
        (appearance.weapon && weapon_order.size()!=2640)) return false;
    human.type=appearance.type;
    human.clothing=appearance.clothing;
    human.hair=appearance.hair;
    human.weapon=appearance.weapon;
    human.profession=appearance.profession;
    human.mount_shape=mount;
    human.bounds=Rect2();
    human.idle.reset();
    std::copy_n(body->standing.begin()+human.facing*10,human.frames.size(),human.frames.begin());
    if (!human.life.dead && target_is_human && hovered_target==actor)
        set_hovered_human(actor,human.type,human.profession,human.name);
    queue_redraw();
    return true;
}

bool Mir3HudView::queue_scene_human_message(int actor,int opcode,Vector2i cell,int facing,int hit_weapon,std::int64_t flags) {
    const auto it=scene_humans.find(actor);
    if (it==scene_humans.end() || flags<0 || flags>0xFFFFFFFFLL ||
        (opcode!=mir3::protocol::SM_HUMAN_STRUCK && !terrain.walkable(cell.x,cell.y))) return false;
    return it->second.messages.push({opcode,cell.x,cell.y,facing,hit_weapon,static_cast<std::uint32_t>(flags)});
}

bool Mir3HudView::dispatch_scene_human_message(int actor) {
    auto& human=scene_humans.at(actor);
    const bool catch_up=human.messages.catching_up(); // sampled before dequeue
    const auto next=human.messages.take();
    if (!next) return false;
    // 414130 handles the hit, then appends a copy as opcode 31.
    if (next->opcode==mir3::protocol::SM_HUMAN_STRUCK && next->hit_variant) {
        auto relay=*next;
        relay.opcode=mir3::protocol::SM_STRUCK;
        human.messages.push(std::move(relay));
    }
    // 413F90 drains these records without an action for human objects.
    if (next->opcode==mir3::protocol::SM_DIGUP || next->opcode==mir3::protocol::SM_27 ||
        next->opcode==mir3::protocol::SM_SKELETON) return true;
    bool accepted=false;
    const Vector2i cell(next->x,next->y);
    if (next->spell_reply) { play_spell_reply(actor,*next->spell_reply);return true; }
    if (next->spell) {
        if (human.life.dead) return true;
        const int skill=*next->spell;
        const auto delta=cell-human.cell;
        const mir3::MapCell direction{
            std::abs(delta.y)>2 && std::abs(delta.x)<=1?0:(delta.x>0)-(delta.x<0),
            std::abs(delta.x)>2 && std::abs(delta.y)<=1?0:(delta.y>0)-(delta.y<0)};
        const auto heading=std::find(mir3::map_directions.begin(),mir3::map_directions.end(),direction);
        human.facing=heading==mir3::map_directions.end()?4:int(heading-mir3::map_directions.begin());
        const auto& route=mir3::SpellRoutes::get(skill);
        human.action.start_spell(int(mir3::SpellRoutes::remote_action(skill)),route.action_limit,catch_up);
        if (route.action_interval) human.action.set_interval(route.action_interval);
        human.spell={skill,route.directional_effect?human.facing:0};
        human.catching_up=catch_up;human.harvest=false;human.idle.reset();
        if (effect_catalog.find(mir3::EffectCatalog::Section::spell,std::uint16_t(skill)))
            play_spell_sound(10*(skill+1000),human.cell);
        queue_redraw();return true;
    }
    if (next->status_speed) {
        human.attack_speed=*next->status_speed;human.action_flags=next->flags;
        update_actor_bar_status(actor,(next->flags&2)!=0);return true;
    }
    const auto apply_entry_name=[&] {
        if (next->transfer_name) {
            const auto name=std::string_view(*next->transfer_name).substr(0,next->transfer_name->find('\0'));
            const auto slash=name.rfind('/');
            const auto previous=scene_names.find(actor);
            mir3::SceneName identity;
            const auto palette=slash==name.npos?255:static_cast<std::uint8_t>(std::strtol(next->transfer_name->c_str()+slash+1,nullptr,10));
            if (identity.update(mir3::protocol::SM_USERNAME,palette,name)) {
                if (slash==name.npos && previous!=scene_names.end()) identity.color=previous->second.color;
                human.name=String::utf8(mir3::ansi_to_utf8(identity.lines[0]).c_str());
                scene_names.insert_or_assign(actor,std::move(identity));
            }
        }
    };
    // 4140E3 -> human vtable +120 -> 4117D0.
    if (next->opcode==mir3::protocol::SM_800 || next->opcode==mir3::protocol::SM_806) {
        if (!next->appearance || !apply_scene_appearance(actor,*next->appearance) ||
            !apply_scene_human_stand(actor,cell,next->facing)) return true;
        human.action_flags=next->flags;
        human.light_attribute=next->light_attribute;
        apply_entry_name();
        update_actor_bar_status(actor,(next->flags&2)!=0);
        play_actor_sound(actor,109,cell);
        return true;
    }
    if (next->opcode==mir3::protocol::SM_STRUCK && !next->hit_variant) { apply_actor_health_record(actor,*next);return true; }
    // 412568–412585 update HP even when the corpse skips the ordinary hit effect.
    if (human.life.dead && next->hit_variant && next->health) {
        apply_actor_health_record(actor,*next);
        show_source_hit_effect(actor,human.cell,next->hit_source,human.last_monster_hit_shape);
        return true;
    }
    // 413F90虚表+176 -> 413F60：12/637仅坐标、朝向及动作8，正文不使用。
    if (next->opcode==mir3::protocol::SM_SITDOWN || next->opcode==mir3::protocol::SM_BUTCH) {
        human.cell=cell;human.facing=next->facing;
        human.harvest=false;human.idle.reset();
        if (!human.life.dead) human.action.start(8,catch_up);
        queue_redraw();return true;
    }
    // Dead actors only accept standing/revival feature refreshes.
    const bool refresh=next->opcode==mir3::protocol::SM_TURN || next->opcode==mir3::protocol::SM_REVIVE || next->opcode==mir3::protocol::SM_REVIVE_REFRESH;
    const bool death=next->opcode==mir3::protocol::SM_DEATH || next->opcode==mir3::protocol::SM_NOWDEATH;
    if (next->appearance && (!human.life.dead || refresh || death)) {
        auto appearance=*next->appearance;
        if (death) appearance.mount.reset(); // 411700
        if (!apply_scene_appearance(actor,appearance)) return true;
    }
    if (next->opcode==mir3::protocol::SM_DEATH || next->opcode==mir3::protocol::SM_NOWDEATH || next->opcode==mir3::protocol::SM_REVIVE || next->opcode==mir3::protocol::SM_REVIVE_REFRESH) {
        accepted=apply_scene_human_life(actor,next->opcode,cell,next->facing);
        if (accepted && next->light_attribute>=0) human.light_attribute=next->light_attribute;
        if (accepted && !death) apply_entry_name();
        if (accepted && human.life.dying) human.action.start(19,catch_up);
    } else if (next->opcode==mir3::protocol::SM_TURN) {
        accepted=apply_scene_human_stand(actor,cell,next->facing);
        if (accepted) {
            if (next->light_attribute>=0) human.light_attribute=next->light_attribute;
            apply_entry_name();
        }
    }
    else if (human.life.dead) return true; // dead actors drain ordinary records without acting
    else if (next->opcode==mir3::protocol::SM_BACKSTEP || next->opcode==mir3::protocol::SM_BUMP || next->opcode==mir3::protocol::SM_WALK || next->opcode==mir3::protocol::SM_RUN) {
        accepted=apply_scene_human_step(actor,next->opcode,cell,next->facing);
        if (accepted && next->light_attribute>=0) human.light_attribute=next->light_attribute;
    } else {
        int hit_weapon=next->hit_weapon;
        if ((next->opcode==mir3::protocol::SM_HUMAN_STRUCK || next->opcode==mir3::protocol::SM_STRUCK) && next->health) {
            apply_actor_health_record(actor,*next);
            show_source_hit_effect(actor,human.cell,next->hit_source,human.last_monster_hit_shape);
            if ((*next->health)[2]==0) return true; // miss updates HP but does not interrupt with hit animation
            if (next->hit_variant && *next->hit_variant<=65535-400) show_following_hit_effect(400+*next->hit_variant,actor,human.cell);
            if (const auto source=scene_humans.find(next->hit_source);source!=scene_humans.end()) hit_weapon=source->second.weapon;
        }
        accepted=apply_scene_human_action(actor,next->opcode,next->facing,hit_weapon);
        if (accepted) {
            if (next->opcode!=mir3::protocol::SM_HUMAN_STRUCK && next->opcode!=mir3::protocol::SM_STRUCK) human.cell=cell;
            human.action.start(human.action.action(),catch_up);
            int effect=0;
            switch (next->opcode) {
            case mir3::protocol::SM_HIT: effect=mir3::SpellColor::weapon_effect(next->weapon_element);break;
            case mir3::protocol::SM_FIREHIT: effect=int(mir3::SkillId::firesword);break;
            case mir3::protocol::SM_POWERHIT: effect=int(mir3::SkillId::yedo);break;
            case mir3::protocol::SM_LONGHIT: effect=int(mir3::SkillId::ergum);break;
            case mir3::protocol::SM_WIDEHIT: effect=int(mir3::SkillId::banwol);break;
            case mir3::protocol::SM_WIDEHIT2: effect=int(mir3::SkillId::original_103);break;
            case mir3::protocol::SM_HUMAN_ACTION17: effect=int(mir3::SkillId::randswing);break;
            case mir3::protocol::SM_HUMAN_ACTION16: effect=int(mir3::SkillId::spacemove2);break;
            default:break;
            }
            human.spell={};
            if (effect && effect_catalog.find(mir3::EffectCatalog::Section::spell,std::uint16_t(effect))) {
                human.spell={effect,next->opcode==mir3::protocol::SM_WIDEHIT2?0:next->facing};
                human.spell.weapon=true;
                if (next->weapon_element<mir3::SpellColor::palette.size())
                    human.spell.tint=mir3::SpellColor::palette[next->weapon_element];
                play_spell_sound(10*(effect+1000),human.cell);
            }
        }
    }
    if (accepted) {
        human.catching_up=catch_up;
        // 412500只更新基础外观；受击正文中的状态位置不能覆盖已有状态。
        if (next->opcode!=mir3::protocol::SM_HUMAN_STRUCK && next->opcode!=mir3::protocol::SM_STRUCK && (next->appearance || next->flags)) {
            human.action_flags=next->flags;
            if (next->appearance) update_actor_bar_status(actor,(next->flags&2)!=0);
        }
        human.harvest=next->harvest;
    }
    return true; // consume at most one record per update, including rejected state
}

bool Mir3HudView::apply_scene_human_action(int actor,int opcode,int facing,int hit_weapon) {
    const auto it=scene_humans.find(actor);
    if (it==scene_humans.end() || facing<0 || facing>=8 || hit_weapon<0 || hit_weapon>=50) return false;
    auto& human=it->second;
    // Decoded presentation adapter; caller retains messages while movement is active.
    if (human.movement_frame>=0 || human.life.dead) return false;
    int action=-1;
    switch (opcode) {
    case mir3::protocol::SM_SITDOWN: action=8; break;
    case mir3::protocol::SM_HIT: case mir3::protocol::SM_HIT2: action=mir3::HumanActions::weapon_attack(human.weapon); break;
    case mir3::protocol::SM_FIREHIT: case mir3::protocol::SM_POWERHIT: case mir3::protocol::SM_LONGHIT:
        action=mir3::HumanActions::weapon_attack(human.weapon);break;
    case mir3::protocol::SM_WIDEHIT: case mir3::protocol::SM_WIDEHIT2:
        action=mir3::HumanActions::weapon_attack(human.weapon,false);break;
    case mir3::protocol::SM_HUMAN_ACTION17: action=17; break;
    case mir3::protocol::SM_HUMAN_ACTION16: action=16; facing=human.facing; break;
    case mir3::protocol::SM_HUMAN_ACTION18: action=18; break;
    case mir3::protocol::SM_HUMAN_ATTACK: action=9; break;
    case mir3::protocol::SM_STRUCK: case mir3::protocol::SM_HUMAN_STRUCK: action=human.mount_shape?32:15; facing=human.facing; break;
    default: return false;
    }
    if (human.mount_shape && opcode!=mir3::protocol::SM_HUMAN_STRUCK && opcode!=mir3::protocol::SM_STRUCK) return false;
    for (int i=0;i<mir3::HumanActions::table[action].count;++i)
        if (!human_action_frame(human.type,human.clothing,mir3::HumanActions::frame(action,facing,i))) return false;
    human.facing=facing;
    human.action.start(action);
    human.catching_up=false;
    human.harvest=false;
    human.hit_weapon=hit_weapon;
    if (opcode!=mir3::protocol::SM_HUMAN_STRUCK && opcode!=mir3::protocol::SM_STRUCK && opcode!=mir3::protocol::SM_SITDOWN) human.combat.attack();
    human.idle.reset();
    queue_redraw();
    return true;
}

bool Mir3HudView::apply_scene_human_stand(int actor,Vector2i cell,int facing) {
    const auto entry=scene_humans.find(actor);
    if (entry==scene_humans.end() || facing<0 || facing>=8 || !terrain.contains(cell.x,cell.y)) return false;
    auto& human=entry->second;
    // Standing state does not revive a dead actor.
    if (human.life.dead) human.life.dying=false;
    // Standing state from the decoded 0x412350 update; preserve identity/portrait.
    human.cell=cell;
    human.facing=facing;
    human.movement_frame=-1;
    human.movement_ms=0;
    human.step_distance=0;
    human.bump=false;
    human.idle.reset();
    human.action=mir3::HumanActionClock{};
    human.catching_up=false;
    human.harvest=false;
    human.bounds=Rect2();
    std::copy_n(human_body(human.type,human.clothing)->standing.begin()+facing*10,human.frames.size(),human.frames.begin());
    queue_redraw();
    return true;
}

bool Mir3HudView::apply_scene_human_step(int actor,int opcode,Vector2i destination,int facing) {
    auto entry=scene_humans.find(actor);
    if (entry==scene_humans.end() || (opcode!=mir3::protocol::SM_BACKSTEP && opcode!=mir3::protocol::SM_BUMP && opcode!=mir3::protocol::SM_WALK && opcode!=mir3::protocol::SM_RUN) || facing<0 || facing>=8 ||
        !terrain.contains(destination.x,destination.y)) return false;
    auto& human=entry->second;
    if (human.life.dead) return false;
    if (opcode==mir3::protocol::SM_BUMP) {
        for (int i=0;i<6;++i)
            if (!human_action_frame(human.type,human.clothing,mir3::HumanActions::frame(23,facing,i))) return false;
        for (int i=0;i<3;++i)
            if (!human_action_frame(human.type,human.clothing,mir3::HumanActions::frame(7,facing,i))) return false;
    }
    human.bump=opcode==mir3::protocol::SM_BUMP;
    human.backstep=opcode==mir3::protocol::SM_BACKSTEP;
    // 0x453450 derives the step origin from the server destination and facing.
    const int distance=opcode==mir3::protocol::SM_RUN?(human.mount_shape?3:2):1;
    const auto [dx,dy]=mir3::map_directions[human.backstep?(facing+4)%8:facing];
    human.facing=facing;
    human.step_origin=destination-Vector2i(dx,dy)*distance;
    human.step_destination=destination;
    human.cell=human.step_origin;
    human.idle.reset();
    human.action=mir3::HumanActionClock{};
    human.catching_up=false;
    human.harvest=false;
    human.movement_frame=0;
    human.movement_ms=0;
    human.step_distance=distance;
    human.bounds=Rect2();
    std::copy_n(human_body(human.type,human.clothing)->standing.begin()+facing*10,human.frames.size(),human.frames.begin());
    queue_redraw();
    return true;
}

void Mir3HudView::update_scene_target(Vector2 pointer) {
    const auto pick=[pointer](const auto& actors) {
        auto hit=actors.end();
        for (auto it=actors.begin();it!=actors.end();++it)
            if (it->second.death_frame<0 && !it->second.buried && it->second.burrow_frame<0 && it->second.target_bounds.has_point(pointer) &&
                (hit==actors.end() || it->second.cell.y>=hit->second.cell.y))
                hit=it;
        return hit;
    };
    if (spell_pointer_available()) {
        const auto chicken=pick(scene_chickens);
        auto human=scene_humans.end();
        for (auto it=scene_humans.begin();it!=scene_humans.end();++it)
            if (!it->second.life.dead && it->second.bounds.has_point(pointer) &&
                (human==scene_humans.end() || it->second.cell.y>=human->second.cell.y)) human=it;
        if (human!=scene_humans.end() && (chicken==scene_chickens.end() || human->second.cell.y>=chicken->second.cell.y)) {
            set_hovered_human(human->first,human->second.type,human->second.profession,human->second.name);
            return;
        }
        if (chicken!=scene_chickens.end()) {
            set_hovered_target(chicken->first,3,chicken->second.shape,false,0,0,0,0,chicken->second.cell);
            return;
        }
    }
    if (scene_chickens.contains(int(hovered_target)) || scene_humans.contains(int(hovered_target)))
        set_hovered_target(0,0,0,false,0,0,0,0);
}

bool Mir3HudView::merchant_pointer(Vector2 pointer) {
    auto hit=scene_npcs.end();
    for (auto it=scene_npcs.begin();it!=scene_npcs.end();++it)
        if (it->second.bounds.has_point(pointer) && (hit==scene_npcs.end() || it->second.cell.y>=hit->second.cell.y))
            hit=it;
    if (hit==scene_npcs.end()) return false;
    map_route.clear();
    const auto now=ticks_msec();
    if (!last_merchant_request || now-last_merchant_request>1500) {
        last_merchant_request=now;
        const auto delta=player_cell-hit->second.cell;
        if (std::max(std::abs(delta.x),std::abs(delta.y))<=2) {
            const int actor=hit->first;
            emit_signal("npc_dialogue_requested",actor);
            emit_signal("npc_request",mir3::protocol::CM_CLICKNPC,actor,String());
            emit_npc_packet(mir3::protocol::CM_CLICKNPC,actor,String());
        }
    }
    return true;
}

void Mir3HudView::step_toward_pointer(bool run,bool force_attack,bool cut) {
    if (player_life.dead || pending_local_action || action_prediction.waiting || !local_actor_messages.empty() || map_input_delay.active() || walk_frame>=0 || !spell_pointer_available()) return;
    if (!player_action.finished() && player_action.action()!=7) return;
    const auto p=inventory_cursor;
    const auto camera=map_camera();
    Vector2i target(int(p.x+camera.x)/48,int(p.y+camera.y)/32);
    const bool left_click=map_pointer_button==MOUSE_BUTTON_LEFT;
    const bool ranged_weapon=player_weapon>=34 && player_weapon<=37;
    const auto monster=scene_chickens.find(hovered_target);
    const auto human=scene_humans.find(hovered_target);
    const bool monster_target=left_click && !cut && !ranged_weapon && monster!=scene_chickens.end() &&
        monster->second.accepts_melee_target();
    if (monster_target && monster->second.protected_from_melee()) return;
    const bool human_target=left_click && !cut && !ranged_weapon && human!=scene_humans.end() &&
        !human->second.life.dead;
    if (human_target) {
        const auto name=scene_names.find(hovered_target);
        const auto color=name==scene_names.end()?0xffffffu:name->second.color;
        // 4264D0 lets Shift and AttackUser bypass the target name color gate.
        if (!force_attack && !system_options.force_attack && color!=255 && color!=27631) return;
        target=human->second.cell;
    }
    if (monster_target) target=monster->second.cell;
    const bool melee_target=monster_target || human_target;
    const auto delta=target-player_cell;
    // 0x417E10 chooses one step along the cursor heading.
    const int dx=std::abs(delta.y)>2 && std::abs(delta.x)<=1?0:(delta.x>0)-(delta.x<0);
    const int dy=std::abs(delta.x)>2 && std::abs(delta.y)<=1?0:(delta.y>0)-(delta.y<0);
    if (map_pointer_button==MOUSE_BUTTON_LEFT && cut && !force_attack) {
        if (player_mount || model.hp==0) return;
        int corpse=0;
        if (!hovered_target) {
            corpse=butcher_monster_at(p);
            for (const auto& [actor,human]:scene_humans) {
                const auto distance=human.cell-player_cell;
                if (human.life.dead && human.bounds.has_point(p) &&
                    std::max(std::abs(distance.x),std::abs(distance.y))<=1)
                    corpse=actor;
            }
        }
        if (target!=player_cell) {
            const mir3::MapCell heading{dx,dy};
            player_facing=int(std::find(mir3::map_directions.begin(),mir3::map_directions.end(),heading)-mir3::map_directions.begin());
        }
        map_route.clear();
        player_harvest_impact=false;
        player_action.start(8);
        action_prediction.begin({player_cell.x,player_cell.y},player_facing);
        const auto position=std::uint32_t(player_cell.x)|(std::uint32_t(player_cell.y)<<16);
        send_client_packet(mir3::protocol::CM_SITDOWN,position,String(),std::uint32_t(player_facing)<<16);
        if (corpse) send_client_packet(mir3::protocol::CM_BUTCH,corpse,String(),position,player_facing);
        queue_redraw();
        return;
    }
    if (left_click && model.hp>0 && !player_mount && !ranged_weapon &&
        ((melee_target && std::max(std::abs(delta.x),std::abs(delta.y))==1) ||
         (force_attack && (melee_target || (!hovered_target && player_weapon!=7))))) {
        const mir3::MapCell heading{dx,dy};
        int facing=player_facing;
        if (target!=player_cell)
            facing=int(std::find(mir3::map_directions.begin(),mir3::map_directions.end(),heading)-mir3::map_directions.begin());
        request_melee_action(facing,mir3::protocol::CM_HIT);
        return;
    }
    if (melee_target && std::max(std::abs(delta.x),std::abs(delta.y))==1) return;
    if (map_pointer_button==MOUSE_BUTTON_LEFT && player_weapon==7 && !player_mount && model.hp>0 &&
        !hovered_target && std::none_of(scene_npcs.begin(),scene_npcs.end(),[p](const auto& npc) {
            return npc.second.bounds.has_point(p);
        })) {
        const auto blocked=[this](mir3::MapCell direction) {
            const auto cell=player_cell+Vector2i(direction[0],direction[1]);
            return !terrain.walkable(cell.x,cell.y) || scene_occupied(cell);
        };
        const mir3::MapCell heading{dx,dy};
        const bool same_cell=target==player_cell;
        const auto facing=mir3::map_directions[player_facing];
        if (force_attack || (!same_cell && blocked(heading) && blocked(facing))) {
            int facing=player_facing;
            if (force_attack && !same_cell)
                facing=int(std::find(mir3::map_directions.begin(),mir3::map_directions.end(),heading)-mir3::map_directions.begin());
            request_melee_action(facing,mir3::protocol::CM_FORCEHIT);
            return;
        }
    }
    if (target==player_cell) {
        const auto ground=std::ranges::find_if(ground_items.items,[&](const auto& item) {
            return item.x==player_cell.x && item.y==player_cell.y;
        });
        if (map_pointer_button==MOUSE_BUTTON_LEFT && map_route.empty() && ground!=ground_items.items.end()) {
            const auto gold=messages.get(1050);
            const bool is_gold=gold.size()>=4 && ground->name.compare(0,4,gold,0,4)==0;
            // 426918/4411A0 compares the original ground-look WORD at record offset 48.
            if (std::ranges::all_of(bag.items,[](const auto& item) { return bool(item); }) &&
                    ((!is_gold && ground->stack_mode<2) ||
                        (ground->stack_mode>=2 && std::ranges::none_of(bag.items,[&](const auto& item) {
                            return (item.server_record[48]|(int(item.server_record[49])<<8))==ground->looks;
                        })))) {
                post_chat(String::utf8(message(1417).c_str()),11861940,0);
                return;
            }
            // 426974 → 47C1C0 sends the ground cell as two WORDs with recognition zero.
            const auto position=std::uint32_t(player_cell.x)|(std::uint32_t(player_cell.y)<<16);
            send_client_packet(mir3::protocol::CM_PICKUP,0,String(),position);
            emit_signal("pickup_requested",mir3::protocol::CM_PICKUP,player_cell);
        }
        return;
    }
    if (map_pointer_button==MOUSE_BUTTON_RIGHT && std::max(std::abs(delta.x),std::abs(delta.y))==1) {
        const mir3::MapCell heading{dx,dy};
        player_facing=int(std::find(mir3::map_directions.begin(),mir3::map_directions.end(),heading)-mir3::map_directions.begin());
        queue_redraw();
        return;
    }
    const auto traversable=[this](Vector2i cell) {
        return !scene_occupied(cell) && terrain.walkable(cell.x,cell.y);
    };
    auto destination=player_cell+Vector2i(dx,dy);
    const auto second=destination+Vector2i(dx,dy);
    if(request_map_door(destination)) return;
    if(run && traversable(destination)) request_map_door(second);
    if (run && traversable(destination) && traversable(second)) {
        const auto third=second+Vector2i(dx,dy);
        if (!player_mount || traversable(third)) destination=player_mount?third:second;
    }
    if (!traversable(destination) && melee_target) return;
    if (!traversable(destination)) {
        destination=player_cell;
        int best_distance=100;
        for (const auto [x,y]:mir3::map_directions) {
            const auto candidate=player_cell+Vector2i(x,y);
            if (!traversable(candidate)) continue;
            const auto offset=candidate-target;
            const int distance=int(std::hypot(offset.x,offset.y));
            if (distance<=best_distance) {
                best_distance=distance;
                destination=candidate;
            }
        }
    }
    if (destination==player_cell) return;
    map_route.clear();
    start_map_step(destination);
}

void Mir3HudView::process_local_actor_message() {
    if (local_actor_messages.empty() || pending_local_action || walk_frame>=0 ||
        (!player_action.finished() && player_action.action()!=7 && !(player_life.dead && !player_life.dying))) return;
    const auto message=local_actor_messages.front();
    local_actor_messages.pop_front();
    if (message.spell_reply) { play_spell_reply(0,*message.spell_reply);return; }
    if (message.motion.opcode==mir3::protocol::SM_SITDOWN || message.motion.opcode==mir3::protocol::SM_BUTCH) return;
    if (message.status) {
        player_status_flags=message.status->flags;
        melee_cooldown.speed=*message.status->status_speed;
        update_actor_bar_status(0,(player_status_flags&2)!=0);return;
    }
    if (message.hit) {
        const auto& appearance=message.appearance;
        const auto& hp=*message.hit->health;
        model.hp=hp[0];model.max_hp=hp[1];
        apply_actor_health_record(0,*message.hit);
        if (!player_life.dead && damage_numbers && message.motion.opcode==mir3::protocol::SM_STRUCK)
            scene_health[0].damage.push(-int(hp[2]));
        set_player_body(appearance.type,appearance.clothing);
        set_player_attachments(appearance.hair,appearance.weapon);
        model.profession=appearance.profession;model.appearance=appearance.type;
        if (message.hit->hit_source==std::uint32_t(local_actor_identity)) local_hit_weapon=player_weapon;
        else if (const auto source=scene_humans.find(message.hit->hit_source);source!=scene_humans.end())
            local_hit_weapon=source->second.weapon;
        if (!player_life.dead && hp[2]) {
            player_spell={};
            player_action.start(player_mount?32:15);
            player_harvest_impact=false;
            show_following_hit_effect(400+message.hit_variant,local_actor_identity,player_cell);
        }
        show_source_hit_effect(local_actor_identity,player_cell,message.hit->hit_source,local_last_monster_hit_shape);
        player_action.set_interval(std::max(80,200-3*int(std::uint8_t(model.level)))+
            (message.hit_variant?15*local_folded_hits:0));
        if (message.hit_variant) local_struck_tick=std::uint32_t(ticks_msec());
        if (message.motion.opcode==mir3::protocol::SM_HUMAN_STRUCK) {
            auto duplicate=message;
            duplicate.motion.opcode=mir3::protocol::SM_STRUCK;
            duplicate.hit->opcode=mir3::protocol::SM_STRUCK;
            local_actor_messages.push_back(duplicate);
        }
        queue_redraw();return;
    }
    const bool death=message.motion.opcode==mir3::protocol::SM_DEATH || message.motion.opcode==mir3::protocol::SM_NOWDEATH;
    const auto& appearance=message.appearance;
    const bool keep_mount=death || message.motion.opcode==mir3::protocol::SM_RUSH || message.motion.opcode==mir3::protocol::SM_BUMP;
    const int shape=keep_mount?player_mount:appearance.mount.value_or(0);
    if (!human_body(appearance.type,appearance.clothing) || !human_body(appearance.type,appearance.clothing,shape!=0) ||
        (shape && !horse_frame(shape,2320))) return;
    set_player_body(appearance.type,appearance.clothing);
    set_player_attachments(appearance.hair,appearance.weapon);
    set_player_mount(shape);mount.shape=std::uint8_t(shape);
    model.profession=appearance.profession;model.appearance=appearance.type;
    player_light_attribute=message.light;
    player_status_flags=appearance.flags;
    update_actor_bar_status(0,(player_status_flags&2)!=0);
    if (player_life.dead && !death) {
        const auto origin=message.motion.origin();
        player_cell=step_origin=step_destination=Vector2i(origin[0],origin[1]);
        player_facing=message.motion.facing;walk_frame=-1;movement_ms=0;
        pending_local_action.reset();forced_motion.reset();map_route.clear();player_action={};
        if (message.motion.walking()) action_prediction.confirm();
        set_minimap_position(player_cell.x,player_cell.y);
        queue_redraw();return;
    }
    if (death) {
        player_spell={};
        pending_local_action.reset();forced_motion.reset();map_route.clear();
        walk_frame=-1;movement_ms=0;player_harvest_impact=false;
        player_cell=step_origin=step_destination=Vector2i(message.motion.destination[0],message.motion.destination[1]);
        player_facing=message.motion.facing;
        if (player_life.die(message.motion.opcode==mir3::protocol::SM_NOWDEATH)) player_action.start(19);
        else {player_action={};model.hp=0;}
        if (scene_health.contains(0)) scene_health.at(0).empty_bar=true;
        set_minimap_position(player_cell.x,player_cell.y);
        queue_redraw();return;
    }
    forced_motion=message.motion;
    const auto origin=forced_motion->origin();
    player_cell=step_origin=Vector2i(origin[0],origin[1]);
    step_destination=Vector2i(forced_motion->destination[0],forced_motion->destination[1]);
    player_facing=forced_motion->facing;
    map_route.clear();player_action={};player_spell={};player_harvest_impact=false;
    walk_frame=0;movement_ms=0;
    if (forced_motion->walking()) action_prediction.confirm();
    set_minimap_position(player_cell.x,player_cell.y);
    queue_redraw();
}

void Mir3HudView::advance_map_step(double delta) {
    if (pending_local_action) return;
    if (!player_life.dead && walk_frame<0 && !map_route.empty() && local_actor_messages.empty() && player_action.finished() && !action_prediction.waiting) {
        const auto [x,y]=map_route.front();
        map_route.pop_front();
        if(!terrain.walkable(x,y)) {map_route.clear();return;}
        start_map_step({x,y});
        return;
    }
    if (walk_frame<0) return;
    movement_ms+=delta*1000;
    if (movement_ms<=95) return;
    movement_ms=0;
    walk_frame+=forced_motion?forced_motion->increment():1;
    if (forced_motion && forced_motion->returns(walk_frame)) {
        player_cell=step_origin;
        step_destination=step_origin;
        walk_frame=-1;forced_motion.reset();
        action_prediction.begin({player_cell.x,player_cell.y},player_facing);
        action_prediction.reject(false);
        player_action.start(7);
        set_minimap_position(player_cell.x,player_cell.y);
        queue_redraw();return;
    }
    // mir3_1.45 HumanActions::movement_sound (0x410DF0).
    const auto step=step_destination-step_origin;
    const bool running=std::max(std::abs(step.x),std::abs(step.y))>1;
    const int sample=forced_motion && !forced_motion->walking()?0:player_mount?(running?(walk_frame==3?35:0):(walk_frame==1?33:walk_frame==4?34:0)):
        (walk_frame==1 || walk_frame==4?1:0);
    if (sample && footstep_sound && audio_options.effects_enabled && movement_sounds.at(sample).is_valid()) {
        const auto camera=map_camera();
        const auto mix=mir3::SoundPosition::from_view(player_cell.x*48-camera.x,player_cell.y*32-camera.y);
        const int volume=mix.volume+mir3::AudioOptions::volume(audio_options.effects_level);
        if (volume>-10000) {
            auto* audio=AudioServer::get_singleton();
            const int bus=audio->get_bus_index(footstep_bus);
            if (bus>=0) {
                const Ref<AudioEffectPanner> panner=audio->get_bus_effect(bus,0);
                const float magnitude=1.f-std::pow(10.f,-std::abs(mix.pan)/2000.f);
                panner->set_pan((mix.pan<0?-1.f:1.f)*(audio_options.reverse_pan?-1.f:1.f)*magnitude);
            }
            footstep_sound->set_volume_db(mix.volume/100.f);
            footstep_sound->set_stream(movement_sounds.at(sample));
            footstep_sound->play();
            last_movement_sound=sample;
        }
    }
    const int direction=forced_motion?forced_motion->direction():player_facing;
    if (walk_frame>=(direction==7?4:1) && player_cell!=step_destination) {
        player_cell=step_destination;
        set_minimap_position(player_cell.x,player_cell.y);
    }
    if (walk_frame>=6) {walk_frame=-1;forced_motion.reset();}
    queue_redraw();
}

void Mir3HudView::draw_human_spell(HumanSpellVisual& spell,const mir3::HumanActionClock& action,Vector2 anchor) {
    if (!spell.skill || action.finished() || (!spell.weapon && action.action()!=int(mir3::SpellAction::directed) &&
        action.action()!=int(mir3::SpellAction::area))) return;
    const auto* resource=effect_catalog.find(mir3::EffectCatalog::Section::spell,std::uint16_t(spell.skill));
    if (!resource) return;
    // 40E1F0：光源不依赖该帧贴图存在；半程后收缩，外半径降到0便不再更新。
    if (spell.light_active && resource->outer_radius) {
        const int relative=spell.weapon?action.frame():std::uint8_t(action.spell_frame()),limit=action.spell_frame_limit();
        const int outer=spell.weapon?((resource->end-resource->first<=8?relative:relative/2)+3):
            std::uint8_t(relative>=(limit>>1)?limit-relative:relative+1);
        const int inner=std::uint8_t(outer-1);
        human_spell_lights.push_back({int(anchor.x)+24,int(anchor.y)+16,inner,
            {resource->inner_color[0],resource->inner_color[1],resource->inner_color[2]},outer,
            {resource->outer_color[0],resource->outer_color[1],resource->outer_color[2]}});
        if (!outer) spell.light_active=false;
    }
    const int index=int(resource->first)+10*spell.direction+(spell.weapon?action.frame():action.spell_frame());
    const MapActorFrame* frame=nullptr;
    if (resource->library==137) frame=level_frame(index);
    else {
        if (!prepare_effect_library(resource->library)) return;
        const int key=(resource->library<<16)|index;
        auto found=event_frames.find(key);
        if (found==event_frames.end()) {
            auto& archive=resource->library==81?event_archive:monster_effect_archives.at(resource->library);
            if (const auto decoded=archive.frame(index))
                found=event_frames.emplace(key,MapActorFrame{texture(decoded->pixels.rgba,decoded->width,decoded->height),
                    Vector2(decoded->offset_x,decoded->offset_y),{}}).first;
        }
        if (found!=event_frames.end()) frame=&found->second;
    }
    if (!frame || frame->image.is_null()) return;
    effect_library_offsets[resource->library]=frame->offset;
    const auto key=std::pair{int(resource->blend),int(resource->opacity)};
    auto& material=human_spell_materials[key];
    if (material.is_null()) {
        material.instantiate();
        material->set_shader(ResourceLoader::get_singleton()->load("res://level_effect.gdshader"));
        material->set_shader_parameter("effect_blend",int(resource->blend));
        material->set_shader_parameter("effect_opacity",resource->opacity/255.f);
    }
    const auto tint=spell.tint.value_or(resource->tint);
    Dictionary drawn;
    drawn["skill"]=spell.skill;drawn["frame"]=index;
    drawn["tint"]=Color(tint[0]/255.f,tint[1]/255.f,tint[2]/255.f);
    drawn["weapon"]=spell.weapon;
    drawn["relative_frame"]=spell.weapon?action.frame():action.spell_frame();
    drawn["bounds"]=Rect2(anchor+frame->offset,frame->image->get_size());
    drawn_human_spells.append(drawn);
    draw_terrain_texture(frame->image,anchor+frame->offset,
        Color(tint[0]/255.f,tint[1]/255.f,tint[2]/255.f),0,false,material);
}

void Mir3HudView::draw_terrain_texture(const Ref<ImageTexture>& image,Vector2 position,Color modulate,int rgb565_weight,bool weapon_glow,
        const Ref<ShaderMaterial>& effect_material) {
    auto* renderer=RenderingServer::get_singleton();
    const bool blend=rgb565_weight || weapon_glow || effect_material.is_valid();
    // Keep opaque draws batched, with a fresh screen copy at each translucent sprite.
    if (!terrain_draw_batch.is_valid() || blend) {
        if (terrain_draw_count==terrain_draw_items.size()) {
            auto item=renderer->canvas_item_create();
            renderer->canvas_item_set_parent(item,terrain_layer->get_canvas_item());
            renderer->canvas_item_set_draw_index(item,int(terrain_draw_count));
            terrain_draw_items.push_back(item);
        }
        terrain_draw_batch=terrain_draw_items[terrain_draw_count++];
        renderer->canvas_item_set_visible(terrain_draw_batch,true);
        renderer->canvas_item_set_use_parent_material(terrain_draw_batch,!blend);
        renderer->canvas_item_set_copy_to_backbuffer(terrain_draw_batch,blend,
            blend?Rect2(position,image->get_size()):Rect2());
        renderer->canvas_item_set_material(terrain_draw_batch,
            effect_material.is_valid()?effect_material->get_rid():weapon_glow?weapon_glow_mix->get_rid():rgb565_weight==8?monster_body_mix->get_rid():rgb565_weight?monster_shadow_mix->get_rid():RID());
    }
    renderer->canvas_item_add_texture_rect(terrain_draw_batch,Rect2(position,image->get_size()),image->get_rid(),false,modulate);
    if (blend) terrain_draw_batch=RID();
}

void Mir3HudView::draw_terrain() {
    if (!terrain_ready) return;
    const auto camera=map_camera();
    const bool ground_hover=spell_pointer_available();
    const Vector2i ground_hover_cell(int(inventory_cursor.x+camera.x)/48,int(inventory_cursor.y+camera.y)/32);
    const int start_x=(camera.x/96)*2,start_y=(camera.y/64)*2;
    const Vector2 offset(start_x*48-camera.x,start_y*32-camera.y);
    for (int row=0;row<10;++row) for (int column=0;column<10;++column) {
        const auto [package,id]=terrain.tile(start_x+2*column,start_y+2*row);
        if (const auto image=terrain_texture(package,id);image.is_valid())
            draw_terrain_texture(image,offset+Vector2(96*column,64*row));
    }
    player_bounds=Rect2();
    human_shadow_bounds.clear();
    weapon_glow_lights.clear();drawn_weapon_glow_frames.clear();
    human_spell_lights.clear();
    drawn_human_spells.clear();
    animated_map_objects=false;
    for (auto& [actor,npc]:scene_npcs) npc.bounds=Rect2();
    for (auto& [actor,chicken]:scene_chickens) {
        chicken.bounds=Rect2();
        chicken.target_bounds=Rect2();
    }
    for (auto& [actor,human]:scene_humans) human.bounds=Rect2();
    if (!bool(get_meta("show_map_objects",true))) return;

    struct Sprite { Ref<ImageTexture> image; Vector2 position; int row; };
    std::vector<Sprite> tall_objects;
    constexpr std::array animation_ms{150,200,250,300,350,400,420,450};
    const auto tick=ticks_msec();
    for (int row=-4;row<31;++row) for (int column=-5;column<21;++column) {
        const Vector2 pixel=offset+Vector2(48*column,32*row);
        for (int layer=0;layer<2;++layer) {
            const auto entry=terrain.object(start_x+column,start_y+row,layer);
            if (entry.package>=75 || entry.package%15<3 || entry.frame==65535) continue;
            const int count=entry.animation&15;
            const bool animated=entry.package%15==11 && entry.animation!=255 && count>1;
            animated_map_objects|=animated;
            const int id=entry.frame+(animated?int(tick/animation_ms[(entry.animation>>4)&7]%count):0);
            const auto image=terrain_texture(entry.package,id);
            if (image.is_null()) continue;
            const auto position=pixel+Vector2(0,32-image->get_height());
            if (image->get_width()<=48 && image->get_height()<=32) draw_terrain_texture(image,position);
            else tall_objects.push_back({image,position,row});
        }
    }
    for(const auto& [id,event]:ground_events) {
        if(event.age<=0) continue;
        const auto image=ground_event_frames.find(event.frame);
        if(image==ground_event_frames.end()) continue;
        const auto position=Vector2(event.cell.x*48-camera.x,event.cell.y*32-camera.y)+image->second.offset;
        draw_terrain_texture(image->second.image,position,Color(1,1,1,event.opacity));
    }
    for(const auto& [cell,event]:event_marks) {
        const auto image=ground_event_frames.find(event.frame);
        if(event.age<=0 || image==ground_event_frames.end()) continue;
        const auto position=Vector2(event.cell.x*48-camera.x,event.cell.y*32-camera.y)+image->second.offset;
        draw_terrain_texture(image->second.image,position,Color(1,1,1,event.opacity));
    }
    const auto draw_scene_actor=[&](SceneActor& npc,int interval,const MapActorFrame* pose=nullptr,
            Vector2i offset={},int dye=65535,int status_color=65535,const MapActorFrame* shadow=nullptr,int appearance_weight=-1,
            bool highlighted=false) {
        const auto& selected=npc.frames[tick/interval%4];
        const auto& frame=pose?*pose:selected.image.is_valid()?selected:npc.frames.front();
        if (npc.cell.x<start_x-5 || npc.cell.x>=start_x+21) return;
        const auto position=Vector2(npc.cell.x*48-camera.x,npc.cell.y*32-camera.y)+frame.offset+Vector2(offset);
        npc.anchor=position-frame.offset;
        npc.bounds=Rect2(position,frame.image->get_size());
        if (shadow)
            draw_terrain_texture(shadow->image,npc.anchor+shadow->offset,
                Color(1,1,1),(appearance_weight>=0 || system_options.shadow_blend || status_color==1)?13:0);
        // 405CB0 uses the 8/32 source weight from 48EF15 for status mode 1.
        // 405CB0：隐身优先保持8/32；有颜色状态的显现按490821作半混合。
        Ref<ShaderMaterial> appearance;
        if (appearance_weight>=0 && status_color!=1)
            appearance=status_color==65535?monster_appearance_mix[appearance_weight]:monster_tint_appearance_mix;
        else if (highlighted && status_color==65535)
            appearance=actor_highlight;
        draw_terrain_texture(dyed_frame(frame,dye,65535,status_color,highlighted),
            position,Color(1,1,1),status_color==1?8:0,false,appearance);
        animated_map_objects=true;
    };
    ground_bounds.clear();
    auto object=tall_objects.begin();
    for (int row=-4;row<31;++row) {
        while (object!=tall_objects.end() && object->row==row) {
            draw_terrain_texture(object->image,object->position);
            ++object;
        }
        constexpr int first_ground_column=-5, past_last_ground_column=21;
        std::array<int,past_last_ground_column-first_ground_column> ground_counts{};
        for (const auto& item:ground_items.items) {
            const int column=item.x-start_x;
            if (item.y!=start_y+row || column<first_ground_column || column>=past_last_ground_column) continue;
            // 42288D assigns the first 100 records to the cell before any sprite lookup.
            auto& count=ground_counts[column-first_ground_column];
            if (count==mir3::GroundItems::cell_capacity) continue;
            ++count;
            const auto& frame=ground_frames.at(item.looks);
            if (frame.image.is_null()) continue;
            const auto offset=mir3::GroundItems::image_offset(frame.image->get_width(),frame.image->get_height());
            const Vector2 position(item.x*48-camera.x+offset[0],item.y*32-camera.y+offset[1]);
            // 4253ED passes mode 15, so 48F4AB leaves the destination coordinates at (0,0).
            // 42541C selects the same opaque highlight blitter as hovered actors.
            const bool highlighted=ground_hover && Vector2i(item.x,item.y)==ground_hover_cell;
            draw_terrain_texture(frame.image,position,Color(1,1,1),0,false,
                highlighted?actor_highlight:Ref<ShaderMaterial>());
            ground_bounds[item.id]=Rect2(position,frame.image->get_size());
        }
        for (auto& [actor,npc]:scene_npcs)
            if (start_y+row==npc.cell.y) {
                const auto& selected=npc.standing[npc.standing_frame];
                const int weight=npc.arrival_ms<0?-1:int(31*npc.arrival_ms/npc.appearance_duration_ms);
                if (selected.image.is_valid())
                    draw_scene_actor(npc,npc.standing_interval,&selected,{},65535,65535,nullptr,weight);
                else if (npc.fallback_to_first_frame) draw_scene_actor(npc,npc.standing_interval,&npc.standing.front());
            }
        draw_monsters(start_y+row,draw_scene_actor);
        for (auto& [actor,human]:scene_humans) {
            const bool highlighted=hovered_target==std::uint32_t(actor) || locked_spell_actor==std::uint32_t(actor);
            if (human.movement_frame<0) {
                if (start_y+row!=human.cell.y || human.cell.x<start_x-5 || human.cell.x>=start_x+21) continue;
                const auto* body=human_body(human.type,human.clothing,human.mount_shape!=0);
                const bool standing=human.action.finished() && !human.life.dead;
                const int idle_action=human.combat.idle_action(human.mount_shape!=0);
                const int motion=human.life.dead?mir3::HumanActions::frame(19,human.facing,human.life.dying?human.action.frame():9):human.action.finished()?mir3::HumanActions::frame(idle_action,human.facing,human.idle.frame()):
                    mir3::HumanActions::frame(human.action.action(),human.facing,human.action.frame());
                const auto* selected=standing && idle_action!=7?&body->standing[human.facing*10+human.idle.frame()]:
                    human_action_frame(human.type,human.clothing,motion);
                if (!selected) continue;
                const auto& frame=*selected;
                if (frame.image.is_null()) continue;
                const auto position=Vector2(human.cell.x*48-camera.x,human.cell.y*32-camera.y)+frame.offset;
                human.anchor=position-frame.offset;
                human.bounds=Rect2(position,frame.image->get_size());
                draw_human_layers(frame,position-frame.offset,human.type,human.hair,human.weapon,motion,human.clothing_dye,human.hair_dye,human.life.dead?0:human.mount_shape,human.life.opacity(),actor_status_color(human.action_flags),human.life.dead && !human.life.dying,highlighted,human.arrival_weight(highlighted));
                if (!human.life.dead) draw_human_spell(human.spell,human.action,position-frame.offset);
                animated_map_objects=true;
                continue;
            }
            const auto* body=human_body(human.type,human.clothing,human.mount_shape!=0);
            const auto& frames=human.step_distance>1?body->running:body->walking;
            const int forced_pose=human.backstep?mir3::HumanActions::frame(5,human.facing,0):
                human.bump?1840+human.facing*10+human.movement_frame:-1;
            const auto* selected=forced_pose>=0?human_action_frame(human.type,human.clothing,forced_pose):&frames[human.facing*10+human.movement_frame];
            if (!selected || selected->image.is_null()) continue;
            const auto& frame=*selected;
            const auto step=human.step_destination-human.step_origin;
            const bool committed=human.movement_frame>=human.movement_commit_frame();
            const int offset_frames=committed?human.movement_frame-5:human.movement_frame+1;
            // 407120 truncates the per-cell vertical step before applying run distance.
            // 4075B0 divides after multiplying the pre-commit backstep offset.
            const int y_offset=human.backstep && !committed?32*step.y*offset_frames/6:
                (32/6)*step.y*offset_frames;
            const Vector2i pixel(human.cell.x*48+step.x*8*offset_frames,
                human.cell.y*32+y_offset);
            if (start_y+row!=int(std::round(pixel.y/32.0))) continue;
            const auto position=Vector2(pixel-camera)+frame.offset;
            human.anchor=position-frame.offset;
            human.bounds=Rect2(position,frame.image->get_size());
            draw_human_layers(frame,position-frame.offset,human.type,human.hair,human.weapon,
                forced_pose>=0?forced_pose:(human.mount_shape?(human.step_distance>1?2480:2400):(human.step_distance>1?1760:1680))+human.facing*10+human.movement_frame,human.clothing_dye,human.hair_dye,forced_pose>=0?0:human.mount_shape,human.life.opacity(),actor_status_color(human.action_flags),human.life.dead && !human.life.dying,highlighted,human.arrival_weight(highlighted));
            animated_map_objects=true;
        }
        const auto pixel=player_pixel();
        if (start_y+row==int(std::round(pixel.y/32.0))) {
            const auto step=step_destination-step_origin;
            const auto* body=human_body(player_type,player_clothing,player_mount!=0 && !player_life.dead);
            const bool running=std::max(std::abs(step.x),std::abs(step.y))>1;
            const auto& movement=running?body->running:body->walking;
            const int forced_pose=player_life.dead?mir3::HumanActions::frame(19,player_facing,player_life.dying?player_action.frame():9):
                forced_motion && walk_frame>=0?forced_motion->pose(walk_frame,player_mount!=0):-1;
            const auto* action_pose=forced_pose>=0?human_action_frame(player_type,player_clothing,forced_pose):
                player_action.finished()?nullptr:human_action_frame(player_type,player_clothing,
                    mir3::HumanActions::frame(player_action.action(),player_facing,player_action.frame()));
            const auto& actor=action_pose?*action_pose:(walk_frame>=0?movement[player_facing*10+walk_frame]:
                body->standing[player_facing*10+player_idle.frame()]);
            if (actor.image.is_valid()) {
                player_bounds=Rect2(Vector2(pixel-camera)+actor.offset,actor.image->get_size());
                draw_human_layers(actor,Vector2(pixel-camera),player_type,player_hair,player_weapon,
                    action_pose?(forced_pose>=0?forced_pose:mir3::HumanActions::frame(player_action.action(),player_facing,player_action.frame())):
                    (player_mount?(walk_frame>=0?(running?2480:2400):2320):(walk_frame>=0?(running?1760:1680):0))+
                    player_facing*10+(walk_frame>=0?walk_frame:player_idle.frame()),player_clothing_dye,player_hair_dye,
                    player_life.dead || (action_pose && !(forced_motion && forced_motion->walking()))?0:player_mount,1.f,actor_status_color(player_status_flags),player_life.dead && !player_life.dying);
                draw_human_spell(player_spell,player_action,Vector2(pixel-camera));
                animated_map_objects=true;
            }
        }
    }
}

void Mir3HudView::update_weather_layers() {
    if(!snow_layer || !fog_copy) return;
    const bool visible=terrain_ready && !login_visible && !server_groups.visible && !character_selection.visible && login_movie_purpose==LoginMoviePurpose::none;
    snow_layer->set_visible(visible && chat_submission.snow && weather.snow);
    fog_copy->set_visible(visible && chat_submission.snow && weather.fog);
    if(const auto image=fog_sprite->get_texture();image.is_valid()) {
        fog_sprite->set_position(Vector2(-1600+weather.fog_x,-300-weather.fog_y));
        fog_sprite->set_scale(Vector2(4000,1200)/image->get_size());
    }
    snow_layer->queue_redraw();
    rain_copy->set_visible(visible && weather.other);
    if(rain_copy->is_visible()) rain_layer->queue_redraw();
}
void Mir3HudView::draw_snow() {
    if(snow_texture.is_null() || !terrain_ready || !weather.snow || !chat_submission.snow) return;
    const PackedVector2Array uv{Vector2(1,1),Vector2(1,0),Vector2(0,0),Vector2(0,1)};
    for(const auto& p:weather.particles) if(p.active) {
        const float half=p.size*0.5f;
        PackedVector2Array vertices{Vector2(p.x-half,p.y-half),Vector2(p.x+half,p.y-half),Vector2(p.x+half,p.y+half),Vector2(p.x-half,p.y+half)};
        PackedColorArray colors;colors.push_back(Color(p.rgb[0]/255.f,p.rgb[1]/255.f,p.rgb[2]/255.f));
        snow_layer->draw_polygon(vertices,colors,uv,snow_texture);
    }
}

void Mir3HudView::draw_rain() {
    const PackedVector2Array uv{Vector2(1,1),Vector2(1,0),Vector2(0,0),Vector2(0,1)};
    for(const auto& p:weather.rain_particles) if(p.active) {
        const auto& image=rain_textures[p.texture_frame()-500];
        if(image.is_null()) continue;
        const float half_width=p.splash()?16.f:0.75f;
        const float half_height=p.splash()?16.f:p.size*0.5f;
        const float dx=p.x-p.previous_x;
        const float angle=dx==0.f?0.f:std::atan2((p.y-p.previous_y)/dx,1.f)-1.57079632679f;
        const Vector2 center(p.x,p.y);
        PackedVector2Array vertices;
        for(const auto& corner:{Vector2(-half_width,-half_height),Vector2(half_width,-half_height),
                Vector2(half_width,half_height),Vector2(-half_width,half_height)})
            vertices.push_back(center+corner.rotated(angle));
        PackedColorArray colors;
        colors.push_back(Color(p.rgb[0]/255.f,p.rgb[1]/255.f,p.rgb[2]/255.f));
        rain_layer->draw_polygon(vertices,colors,uv,image);
    }
}

void Mir3HudView::draw_scene_lighting() {
    scene_light_samples.clear();
    if (!terrain_ready || scene_ambient.level()==255) return;
    mir3::SceneLightGrid grid(scene_ambient.level());
    const auto camera=map_camera();
    constexpr std::array radii{4,8,12,20};
    constexpr std::array<mir3::SceneLight::Rgb,10> colors{{
        {173,198,214},{90,140,165},{41,57,115},{222,198,99},{41,115,49},
        {255,255,255},{255,255,255},{255,255,255},{255,255,255},{255,255,255}}};
    for (int y=camera.y/32-20;y<=camera.y/32+38;++y)
        for (int x=camera.x/48-20;x<=camera.x/48+37;++x)
            if (const auto packed=terrain.light(x,y))
                grid.add({x*48-camera.x,y*32-camera.y,0,{},radii[packed>>14],colors[(packed>>4)&255]});
    if (magic_light) {
        for (const auto& light:human_spell_lights) grid.add(light);
        for (const auto& effect:level_effects) grid.add(effect.playback.light(camera.x,camera.y));
        for (const auto& [id,event]:map_events) {
            if (!event.emits_light || event.delay_ms) continue;
            const auto& r=*event.resource;
            const auto world=event.projectile?event.projectile->position:
                event.follow_position.value_or(Vector2(event.cell*Vector2i(48,32)));
            const auto p=world-Vector2(camera)+Vector2(24,16);
            const auto light_color=[&](const auto& rgb) -> mir3::SceneLight::Rgb {
                if (!event.light_fade) return {rgb[0],rgb[1],rgb[2]};
                if (event.spell_effect || event.ground_wave) return {mir3::SpellLight::channel(rgb[0],event.light_fade),
                    mir3::SpellLight::channel(rgb[1],event.light_fade),mir3::SpellLight::channel(rgb[2],event.light_fade)};
                return {std::max(2,rgb[0]-event.light_fade),std::max(2,rgb[1]-event.light_fade),
                    std::max(2,rgb[2]-event.light_fade)};
            };
            grid.add({int(p.x),int(p.y),r.inner_radius+event.light_phase-event.light_shrink,
                light_color(r.inner_color),r.outer_radius+event.light_phase-event.light_shrink,
                light_color(r.outer_color)});
        }
    }
    const auto local=player_pixel()-camera;
    grid.add(mir3::SceneLight::actor(local.x,local.y,player_light_attribute,true));
    for (const auto& [id,human]:scene_humans) {
        if (human.life.dead || !human.bounds.has_area()) continue;
        grid.add(mir3::SceneLight::actor(int(human.anchor.x)+24,int(human.anchor.y)+16,human.light_attribute,false));
    }
    // 4255E2 adds light after drawing a visible type 4 or 5 NPC.
    for (const auto& [id,npc]:scene_npcs) {
        if (!npc.bounds.has_area()) continue;
        grid.add(mir3::SceneLight::actor(int(npc.anchor.x)+24,int(npc.anchor.y)+16,npc.light_attribute,false));
    }
    add_monster_lights(grid);
    for (const auto position:weapon_glow_lights)
        grid.add({int(position.x),int(position.y),2,{40,40,250},5,{40,40,250}});
    const auto color=[&](int i) {
        const auto& c=grid.samples[i];return Color(c[0]/255.f,c[1]/255.f,c[2]/255.f);
    };
    for (int i=0;i<18*19;++i) scene_light_samples.push_back(color(i));
    const auto triangle=[&](Vector2 a,Vector2 b,Vector2 c,Color ca,Color cb,Color cc) {
        PackedVector2Array points;points.push_back(a);points.push_back(b);points.push_back(c);
        PackedColorArray colors;colors.push_back(ca);colors.push_back(cb);colors.push_back(cc);
        scene_light_layer->draw_polygon(points,colors);
    };
    // Full grid; opaque HUD is drawn afterwards, so no clipped cell changes its diagonal.
    for (int y=0;y<18;++y) for (int x=0;x<17;++x) {
        const int i=y*18+x;
        const Vector2 a(x*48,y*32),b((x+1)*48,y*32),c((x+1)*48,(y+1)*32),d(x*48,(y+1)*32);
        if (grid.diagonals[y*17+x]) {
            triangle(a,b,c,color(i),color(i+1),color(i+19));
            triangle(a,c,d,color(i),color(i+19),color(i+18));
        } else {
            triangle(a,b,d,color(i),color(i+1),color(i+18));
            triangle(b,c,d,color(i+1),color(i+19),color(i+18));
        }
    }
}

void Mir3HudView::draw_exit_transition() {
    if (login_visible && login_flow.phase==mir3::LoginFlow::Phase::leaving) {
        const float value=login_flow.brightness();
        exit_layer->draw_rect(Rect2(0,0,640,480),Color(value,value,value));return;
    }
    if (!exit_transition.active || character_selection.visible) return;
    const float value=exit_transition.brightness();
    exit_layer->draw_rect(Rect2(0,0,800,600),Color(value,value,value));
}

bool Mir3HudView::prepare_character_selection() {
    if (!selection_frames.empty()) return true;
    if (!selection_archive.open(std::filesystem::u8path(actor_resource_root.path_join("Interface1c").utf8().get_data()))) return false;
    std::map<int,Ref<ImageTexture>> loaded_frames;
    for (int index:{50,51,52,53,54,55,56,57,58}) {
        const auto source=selection_archive.frame(index);
        if (!source) return false;
        loaded_frames.emplace(index,texture(source->pixels.rgba,source->width,source->height));
    }
    if (frame(1248).is_null()) return false;
    selection_frames=std::move(loaded_frames);return true;
}

void Mir3HudView::clear_game_scene() {
    clear_map_events();clear_level_effects();player_action={};player_harvest_impact=false;
    login_audio.enter(mir3::LoginAudio::Page::none);
    for (auto& [file,player]:login_effects) player->stop();
    cancel_pointer_interactions();
    while (!scene_humans.empty()) remove_scene_actor(scene_humans.begin()->first);
    while (!scene_npcs.empty()) remove_scene_actor(scene_npcs.begin()->first);
    clear_monsters();
    scene_removals.clear();scene_clear_pending=false;map_changes.clear();
    scene_ambient={};
    weather.select(0);weather.particles={};weather.active=0;weather.clear_rain();update_weather_layers();
    ground_items.items.clear();ground_bounds.clear();displayed_ground_item=-1;
    scene_speech.clear();speech_bounds.clear();scene_health.clear();scene_health_bounds.clear();scene_damage_glyphs.clear();
    map_route.clear();walk_frame=-1;movement_ms=0;
    map_input_delay={};player_mount=0;mount.shape=0;
    terrain_ready=false;terrain=mir3::MapTerrain{};terrain_frames.clear();current_map_id=String();
    target_focus.clear();local_actor_identity=0;player_light_attribute=0;exit_guard={};melee_cooldown={};
    pending_local_action.reset();local_action_clock={};spell_timing={};melee_spells={};player_melee_sound=0;active_spell=0;player_spell={};
    action_prediction={};
    local_actor_messages.clear();forced_motion.reset();
    player_life={};local_struck_tick=0;local_folded_hits=0;local_hit_weapon=0;local_last_monster_hit_shape.reset();player_status_flags=0;
    login_actor_pending.reset();login_actor_extra={};server_ability.reset();server_sub_ability={};weapon_training=0;
    guild_overview={};guild_overview_started=false;guild_manager=false;guild_name=String();
    for (auto& page:guild_pages) page.clear();
    for (auto& editable:guild_editable) editable.clear();
    for (auto& category:skills.learned) category.clear();
    skills.bindings.fill(0);skills.selected.fill(-1);skills.tab=0;skills.scroll=0;
    skill_key_menu={};skill_menu_target=0;skill_shortcuts_visible=false;
    group_request=0;group_members.clear();group_allow=false;chat_submission.clear_history();chat_storage_key=String();
    // 421E80 -> 4217E0 -> 4355C0/435180：离场重建物品窗口和持有记录。
    quick_items={};quick_panel=mir3::DraggablePanel{{73,413,280,48}};quick_close_pressed=false;
    bag={};pending_drop={};held_gold=false;equipment={};inventory.row=0;inventory_cursor=Vector2(-1,-1);
    inventory_gold=0;trade_gold={};trade_gold_pending=false;trade_gold_debited=false;
    for (auto& side:trade_items) for (auto& item:side) item={};
    trade_scroll={};trade_partner=String();trade_offer_count=0;
    trade_pending_slot=trade_return_slot=trade_return_bag_slot=-1;
    trade_return_pending=false;trade_return_bag_pointer={};trade_confirmed=false;trade_confirm_pressed=false;
    trade_actor_name=String();trade_actor_cell={};trade_actor_type=-1;trade_actor_dead=false;trade_direction=0;
    close_npc_dialogue();
    last_merchant_request=0;merchant_click_captured=false;
    inventory.visible=status.visible=skills.visible=group.visible=guild.visible=false;
    quest={};
    options.visible=portrait_panel.visible=mount.visible=trade.visible=false;
    minimap.visible=false;
    if (map_music) { map_music->stop();map_music->set_stream(Ref<AudioStream>()); }
    if (footstep_sound) footstep_sound->stop();
    if (hud_sound) hud_sound->stop();
    if (gold_sound) gold_sound->stop();
    while (!actor_sounds.empty()) remove_actor_sound(actor_sounds.begin()->first);
    chat_input->release_focus();chat_input->hide();
    scene_light_layer->hide();world_overlay_layer->hide();snow_layer->hide();fog_copy->hide();rain_copy->hide();hud_base->hide();hud_copy->hide();hud_decoration->hide();hud_tail->hide();exit_layer->hide();
}

void Mir3HudView::load_login_announcement() {
    login_announcement={};announcement_images.clear();
    const auto path=actor_resource_root.get_base_dir().path_join("Notice.ntc");
    if (!FileAccess::file_exists(path)) return;
    const auto bytes=FileAccess::get_file_as_bytes(path);
    const auto measure=[](std::string_view ansi,int points,bool bold) {
        return mir3::raster_text(mir3::ansi_to_utf8(ansi),512,128,false,mir3::TextFont::normal,{},false,points,bold?700:400).text_width;
    };
    if (!login_announcement.load({bytes.ptr(),std::size_t(bytes.size())},measure)) return;
    for (const auto& row:login_announcement.rows) {
        const auto bitmap=mir3::raster_text(mir3::ansi_to_utf8(row.ansi),224,128,false,mir3::TextFont::normal,{},false,row.points,row.bold?700:400);
        announcement_images.push_back({texture(bitmap.rgba,224,128),bitmap.text_width,bitmap.text_height});
    }
}

void Mir3HudView::draw_login_announcement() {
    if (login_announcement.rows.empty()) return;
    legacy_rect(Rect2(18,115,234,238),Color(75.f/255,50.f/255,0,100.f/255));
    constexpr unsigned border=3296100;
    draw_rect(Rect2(18,115,234,238),Color((border&255)/255.f,((border>>8)&255)/255.f,((border>>16)&255)/255.f),false);
    for (int i=0;i<int(login_announcement.rows.size());++i) {
        const int y=120+14*i+login_announcement.offset;
        if (y<120 || y+14>=353) continue;
        const auto& row=login_announcement.rows[i];const auto& image=announcement_images[i];
        const auto color=[](unsigned value) { return Color((value&255)/255.f,((value>>8)&255)/255.f,((value>>16)&255)/255.f); };
        const auto background=mir3::chat_palette(row.background);
        if (background) draw_rect(Rect2(23,y,image.width,image.height),color(background));
        if (image.image.is_valid()) draw_texture(image.image,Vector2(23,y),color(mir3::chat_palette(row.foreground)));
    }
}

void Mir3HudView::play_login_sound(std::string_view file,bool loop) {
    if (file.empty() || !audio_options.effects_enabled) return;
    const std::string name(file);
    auto found=login_effects.find(name);
    if (found==login_effects.end()) {
        const auto path=audio_root.path_join(String::utf8(name.c_str()));
        if (!FileAccess::file_exists(path)) return;
        const auto stream=AudioStreamWAV::load_from_file(path);
        if (stream.is_null()) return;
        if (loop) {
            stream->set_loop_mode(AudioStreamWAV::LOOP_FORWARD);
            stream->set_loop_begin(0);
            stream->set_loop_end(int(stream->get_data().size())/(stream->is_stereo()?2:1)/(stream->get_format()==AudioStreamWAV::FORMAT_16_BITS?2:1));
        }
        auto* player=memnew(AudioStreamPlayer);player->set_stream(stream);add_child(player);
        found=login_effects.emplace(name,player).first;
    }
    found->second->set_volume_db(mir3::AudioOptions::volume(audio_options.effects_level)/100.f);
    found->second->play();
}

void Mir3HudView::advance_login_audio(std::uint32_t ms) {
    const auto file=login_audio.advance(ms);if (file.empty()) return;
    if (file.ends_with(".wav")) { play_login_sound(file,true);return; }
    if (!map_music || !audio_options.music_enabled) return;
    const auto path=audio_root.path_join(String::utf8(file.data(),file.size()));
    if (!FileAccess::file_exists(path)) return;
    const auto stream=AudioStreamMP3::load_from_file(path);
    if (stream.is_null()) return;
    stream->set_loop(true);map_music->set_stream(stream);
    map_music->set_volume_db(mir3::AudioOptions::volume(audio_options.music_level)/100.f);map_music->play();
}

void Mir3HudView::start_login_movie(LoginMoviePurpose purpose,const char* filename,bool reverse) {
    login_movie_purpose=purpose;login_movie_surface.unref();
    login_audio.enter(mir3::LoginAudio::Page::none);
    if (map_music) map_music->stop();
    if (login_movie_audio) login_movie_audio->stop();
    if (purpose==LoginMoviePurpose::create) play_login_sound("CreateChr.wav");
    else if (purpose==LoginMoviePurpose::enter_game) play_login_sound("StartGame.wav");
    chat_input->release_focus();chat_input->hide();
    const auto path=std::filesystem::u8path(actor_resource_root.utf8().get_data())/filename;
    if (login_movie.open(path,reverse)) {
        login_movie_surface=texture(login_movie.pixels(),login_movie.width(),login_movie.height());
        const auto& audio=login_movie.audio();
        if (!audio.pcm.empty() && audio_options.effects_enabled) {
            if (!login_movie_audio) { login_movie_audio=memnew(AudioStreamPlayer);add_child(login_movie_audio); }
            Ref<AudioStreamWAV> stream;stream.instantiate();
            stream->set_format(audio.bits==8?AudioStreamWAV::FORMAT_8_BITS:AudioStreamWAV::FORMAT_16_BITS);
            stream->set_stereo(audio.channels==2);stream->set_mix_rate(audio.rate);
            PackedByteArray pcm;pcm.resize(audio.pcm.size());std::copy(audio.pcm.begin(),audio.pcm.end(),pcm.ptrw());
            // Godot的8位PCM为有符号，AVI PCM为无符号。
            if (audio.bits==8) for (int64_t i=0;i<pcm.size();++i) pcm.set(i,pcm[i]^0x80);
            stream->set_data(pcm);login_movie_audio->set_stream(stream);
            login_movie_audio->set_volume_db(mir3::AudioOptions::volume(audio_options.effects_level)/100.f);login_movie_audio->play();
        }
    }
    queue_redraw();
}

void Mir3HudView::advance_login_movie(double delta) {
    if (login_movie_purpose==LoginMoviePurpose::none) return;
    if (login_movie.advance(delta*1000)) login_movie_surface=texture(login_movie.pixels(),login_movie.width(),login_movie.height());
    if (login_movie.playing()) { queue_redraw();return; }
    const auto purpose=login_movie_purpose;login_movie_purpose=LoginMoviePurpose::none;
    login_movie.close();login_movie_surface.unref();if (login_movie_audio) login_movie_audio->stop();
    if (purpose==LoginMoviePurpose::opening) { sync_login_editor();login_audio.enter(mir3::LoginAudio::Page::credentials); }
    else if (purpose==LoginMoviePurpose::create) show_character_creation();
    else if (purpose==LoginMoviePurpose::enter_game) finish_game_entry();
    else { login_audio.enter(mir3::LoginAudio::Page::selection);if (character_selection.selected>=0) character_selection.roles[character_selection.selected].start(1); }
    queue_redraw();
}

void Mir3HudView::finish_game_entry() {
    if (!game_entry.finish_transition()) return;
    clear_game_scene();character_selection.visible=false;character_creation.close();selection_notice.close();
    login_visible=false;login_flow.previous_endpoint=login_flow.endpoint;login_flow.endpoint=game_entry.endpoint;
    game_connection={};inventory_gold=0;elements={};
    set_character_vitals(0,0,0,0,0,0);
    set_character_progress(0,0,0,0,0);
    set_character_combat(Vector2i(),Vector2i(),Vector2i(),Vector2i(),Vector2i());
    set_character_load_bonuses(Vector2i(),Vector2i(),Vector2i());
    get_window()->set_content_scale_size(Vector2i(800,600));get_window()->set_size(Vector2i(800,600));
    scene_light_layer->show();world_overlay_layer->show();hud_base->show();hud_copy->show();hud_decoration->show();hud_tail->show();
    connect_page(ConnectionPage::game,game_entry.endpoint);queue_redraw();
}

bool Mir3HudView::apply_login_actor() {
    if (!login_actor_pending || !terrain_ready) return false;
    const auto actor=*login_actor_pending;const auto& appearance=actor.appearance;
    if (!terrain.walkable(actor.x,actor.y)) return false;
    const int mount=appearance.mount.value_or(0);
    const auto* body=human_body(appearance.type,appearance.clothing);
    if (!body || (mount && (!human_body(appearance.type,appearance.clothing,true) || !horse_frame(mount,2320))) ||
        (appearance.weapon && weapon_order.size()!=2640)) return false;
    player_life={};local_struck_tick=0;local_folded_hits=0;local_hit_weapon=0;local_last_monster_hit_shape.reset();player_status_flags=0;player_action={};action_prediction={};pending_local_action.reset();
    spell_timing={};active_spell=0;player_spell={};
    local_actor_messages.clear();forced_motion.reset();
    player_mount=0;set_player_body(appearance.type,appearance.clothing);
    set_player_mount(mount);this->mount.shape=std::uint8_t(mount);set_player_attachments(appearance.hair,appearance.weapon);
    player_cell=Vector2i(actor.x,actor.y);player_facing=actor.facing;player_light_attribute=actor.light;
    player_status_flags=appearance.flags;
    update_actor_bar_status(0,(player_status_flags&2)!=0);
    player_idle.reset();map_route.clear();walk_frame=-1;movement_ms=0;
    model.profession=appearance.profession;model.appearance=appearance.type;
    set_character_identity(String::utf8(mir3::ansi_to_utf8(game_entry.role).c_str()),String(),String(),String());
    set_minimap_position(actor.x,actor.y);
    login_actor_extra=actor.extra;login_actor_pending.reset();
    send_client_packet(mir3::protocol::CM_QUERYBAGITEMS,0,String());queue_redraw();return true;
}

void Mir3HudView::process_map_changes() {
    // Snapshot the count: synchronous signal callbacks enqueue for the next update.
    const auto maps_to_process=map_changes.size();
    for (std::size_t i=0;i<maps_to_process;++i) {
        const auto message=std::move(map_changes.front());map_changes.pop_front();
        const auto id=String::utf8(message.map_id.c_str());
        const auto name=model.map_name.empty()?id:String::utf8(model.map_name.c_str());
        const bool success=change_map(id,name,Vector2i(message.x,message.y),message.ambient|(int(message.series)<<8));
        if (success) {map_series=message.series;weather.select(message.series,true);update_weather_layers();}
        emit_signal("map_change_finished",message.opcode,id,success);
    }

}

const Mir3HudView::MapActorFrame* Mir3HudView::level_frame(int index) {
    if (const auto it=level_frames.find(index);it!=level_frames.end()) return &it->second;
    const auto decoded=level_archive.frame(index);if (!decoded) return nullptr;
    return &level_frames.emplace(index,MapActorFrame{texture(decoded->pixels.rgba,decoded->width,decoded->height),
        Vector2(decoded->offset_x,decoded->offset_y),decoded->pixels.regions}).first->second;
}
void Mir3HudView::show_harvest_impact(Vector2i cell) {
    for(auto& [id,event]:ground_events) {
        if(event.cell==cell && !event.hidden && event.frame>=230 && event.frame<=234) {
            event.frame=std::min(event.frame+1,234);
            break;
        }
    }
    show_map_impact(335,cell);
}

void Mir3HudView::show_following_hit_effect(int effect,int actor,Vector2i cell) {
    const auto event_id=harvest_effect_id;
    show_map_impact(effect,cell);
    if (const auto event=map_events.find(event_id);event!=map_events.end())
        event->second.follow_actor=actor;
}

void Mir3HudView::show_map_impact(int effect,Vector2i cell) {
    show_scene_effect(mir3::EffectCatalog::Section::magic,effect,cell);
}

void Mir3HudView::show_ground_magic(mir3::MagicEffectId effect,Vector2i source,Vector2i target,int facing) {
    const auto direction=[](Vector2i source,Vector2i target) {
        const auto delta=target-source;
        Vector2i step((delta.x>0)-(delta.x<0),(delta.y>0)-(delta.y<0));
        if (std::abs(delta.y)>2 && std::abs(delta.x)<=1) step.x=0;
        if (std::abs(delta.x)>2 && std::abs(delta.y)<=1) step.y=0;
        if (step==Vector2i()) step.y=1;
        return step;
    };
    const auto segment=[&](Vector2i cell,int delay) {
        const auto id=harvest_effect_id;
        show_scene_effect(mir3::EffectCatalog::Section::magic,int(effect),cell,0,delay);
        if (const auto event=map_events.find(id);event!=map_events.end()) {
            event->second.ground_wave=true;
            // 44C7D0 delegates creation to 4495D0, whose light flag starts enabled.
            event->second.emits_light=event->second.resource->outer_radius>0;
        }
    };
    if (effect==mir3::MagicEffectId::three_rays) {
        if (facing<0 || facing>=8) return;
        // 413150 -> 40DFE0：调用顺序及重叠起点都保留，不能按坐标去重。
        static constexpr std::array<std::array<int,3>,8> rays{{
            {{0,7,1}},{{1,0,2}},{{1,2,3}},{{3,2,4}},
            {{4,3,5}},{{5,4,6}},{{6,7,5}},{{7,0,6}}
        }};
        const auto forward=mir3::map_directions[facing];
        const auto start=source+Vector2i(forward[0],forward[1]);
        for (const int ray:rays[facing]) {
            const auto heading=mir3::map_directions[ray];
            const auto step=direction(start,source+Vector2i(heading[0],heading[1])*5);
            for (int i=0;i<5;++i) segment(start+step*i,150*i);
        }
    } else {
        const auto step=direction(source,target);
        const int interval=effect==mir3::MagicEffectId::lightning_trail?30:150;
        for (int i=0;i<9;++i) segment(source+step*(i+1),interval*i);
    }
}

void Mir3HudView::play_spell_reply(int actor,const mir3::SpellReply& reply) {
    const auto source=actor==0?player_cell:scene_humans.at(actor).cell;
    const auto skill=static_cast<mir3::SkillId>(reply.skill);
    if (!reply.multiple) {
        // 413150：这些回包完全忽略，连当前技能编号也不更新。
        switch (skill) {
        case mir3::SkillId::original_104: case mir3::SkillId::spacemove:
        case mir3::SkillId::sinsu: case mir3::SkillId::firewind:
        case mir3::SkillId::cloak: case mir3::SkillId::skelleton:
        case mir3::SkillId::original_78: case mir3::SkillId::original_64:
        case mir3::SkillId::original_105: case mir3::SkillId::original_121: return;
        default: break;
        }
        const Vector2i target(reply.x,reply.y);
        const auto target_actor=reply.targets.empty()?0:reply.targets.front();
        switch (skill) {
        case mir3::SkillId::fire:
            show_ground_magic(mir3::MagicEffectId::fire_wave,source,target);break;
        case mir3::SkillId::original_53:
            show_ground_magic(mir3::MagicEffectId::ice_wave,source,target);break;
        case mir3::SkillId::original_110:
            show_ground_magic(mir3::MagicEffectId::three_rays,source,target,
                actor==0?player_facing:scene_humans.at(actor).facing);break;
        case mir3::SkillId::earthfire: case mir3::SkillId::holyshield:
        case mir3::SkillId::original_63: break;
        default:
            show_player_magic(skill,source,target,target_actor);
            if (skill==mir3::SkillId::shootlighten)
                show_ground_magic(mir3::MagicEffectId::lightning_trail,source,target);
            break;
        }
    }
    // 4138C0 只有113创建目标列表效果；其他技能仍更新当前技能编号。
    if (reply.multiple && skill==mir3::SkillId::original_113) {
        for (const auto target:reply.targets) {
            std::optional<Vector2i> cell;
            if (local_actor_identity && target==std::uint32_t(local_actor_identity)) cell=player_cell;
            else if (const auto human=scene_humans.find(target);human!=scene_humans.end()) cell=human->second.cell;
            else if (const auto monster=scene_chickens.find(target);monster!=scene_chickens.end()) cell=monster->second.cell;
            else if (const auto npc=scene_npcs.find(target);npc!=scene_npcs.end()) cell=npc->second.cell;
            if (cell) show_player_magic(skill,source,*cell,target);
        }
    }
    if (actor==0) active_spell=reply.skill;
}

void Mir3HudView::show_player_magic(mir3::SkillId skill,Vector2i source,Vector2i target,std::uint32_t target_actor) {
    if (skill==mir3::SkillId::original_150) {
        show_player_magic(mir3::SkillId::fireball,source,target,0);
        const auto delta=Vector2((target-source)*Vector2i(48,32));
        const int direction=monster_projectile_direction(delta,false);
        if (source==target) return;
        for (int i=0;i<2;++i) {
            const auto id=harvest_effect_id;
            MonsterProjectile flight{Vector2(source*Vector2i(48,32)),Vector2(target*Vector2i(48,32)),0,int(skill)};
            show_scene_effect(mir3::EffectCatalog::Section::magic,int(skill),source,10*direction,30*i,0,flight);
            if (const auto found=map_events.find(id);found!=map_events.end()) {
                auto& event=found->second;
                event.spell_effect=true;event.one_shot=false;
                event.projectile->velocity=spell_initial_velocity(delta);
                event.orbit=mir3::SpellOrbit{0,i==1};
                event.orbit_direction=unsigned(direction);
                event.sprite->hide();
            }
        }
        return;
    }
    if (skill==mir3::SkillId::original_73 || skill==mir3::SkillId::original_74) {
        const auto anchor=skill==mir3::SkillId::original_73?source:target;
        const auto event_id=harvest_effect_id;
        show_scene_effect(mir3::EffectCatalog::Section::magic,int(skill),target);
        if (const auto found=map_events.find(event_id);found!=map_events.end()) {
            auto& event=found->second;
            event.cell=anchor;
            const auto& resource=*event.resource;
            const auto delta=Vector2((target-source)*Vector2i(48,32));
            event.staged_spell.emplace(skill,monster_projectile_direction(delta,false)>>1,
                mir3::StagedSpell::Frames{int(resource.first),int(resource.end),resource.interval});
            const auto offset=event.staged_spell->offset;
            event.follow_position=Vector2(anchor*Vector2i(48,32))+Vector2(offset[0],offset[1]);
            event.emits_light=resource.outer_radius>0;
            // 子类44D9B0不调用普通弹道推进，也没有其60秒寿命限制。
        }
        return;
    }
    const auto rules=mir3::MagicRules::for_skill(skill);
    if (rules.reject_same_cell && source==target) return;
    // 原版创建时查找目标对象，未知编号不能在以后出现时才绑定。
    if (!rules.track_target || (!scene_humans.contains(target_actor) && !scene_chickens.contains(target_actor) && !scene_npcs.contains(target_actor) &&
        (!local_actor_identity || target_actor!=std::uint32_t(local_actor_identity)))) target_actor=0;
    const auto delta=Vector2((target-source)*Vector2i(48,32));
    const int direction=rules.direction==mir3::MagicDirection::fixed?0:
        monster_projectile_direction(delta,rules.direction==mir3::MagicDirection::eight);
    if (rules.use_source_as_target) target=source;
    const auto event_id=harvest_effect_id;
    if (rules.at_target) {
        show_scene_effect(mir3::EffectCatalog::Section::magic,int(skill),target,10*direction);
    } else {
        MonsterProjectile projectile{Vector2(source*Vector2i(48,32)),Vector2(target*Vector2i(48,32)),target_actor,int(skill)};
        projectile.untargeted_timeout=rules.untargeted_timeout;
        show_scene_effect(mir3::EffectCatalog::Section::magic,int(skill),source,10*direction,0,0,projectile);
    }
    if (auto found=map_events.find(event_id);found!=map_events.end()) {
        auto& event=found->second;
        event.spell_effect=true;
        if (event.projectile) event.projectile->velocity=spell_initial_velocity(event.projectile->destination-event.projectile->position);
        event.one_shot=!rules.repeat;
        event.emits_light=rules.light && event.resource->outer_radius>0;
        if (rules.at_target) event.follow_actor=target_actor;
    }
}

bool Mir3HudView::prepare_effect_library(int library) {
    if (library!=81 && library!=137) {
        const char* archive_name=nullptr;
        switch (library) {
        case 91: archive_name="Mon-4"; break;
        case 129: archive_name="MonMagic"; break;
        case 138: archive_name="MonMagicEx"; break;
        case 140: archive_name="MonMagicEx2"; break;
        case 143: archive_name="MonMagicEx3"; break;
        case 148: archive_name="MonMagicEx4"; break;
        case 149: archive_name="MagicEx2"; break;
        default: return false;
        }
        if (!monster_effect_archives.contains(library)) {
            mir3::WilArchive archive;
            const auto data=std::filesystem::u8path(actor_resource_root.utf8().get_data());
            if (!archive.open(data/archive_name)) return false;
            monster_effect_archives.emplace(library,std::move(archive));
        }
    }
    return true;
}

void Mir3HudView::show_scene_effect(mir3::EffectCatalog::Section section,int effect,Vector2i cell,
        int frame_offset,int delay_ms,int attached_actor,std::optional<MonsterProjectile> projectile,
        MonsterEffectPose attached_pose) {
    const auto* resource=effect_catalog.find(section,effect);
    const bool particle_only=section!=mir3::EffectCatalog::Section::spell &&
        (effect==int(mir3::SkillId::original_150) || effect==int(mir3::MagicEffectId::lightning_trail));
    if (!resource || (!particle_only && (resource->end<=resource->first || !resource->interval)) || !prepare_effect_library(resource->library)) return;
    auto* copy=memnew(BackBufferCopy);
    copy->set_copy_mode(BackBufferCopy::COPY_MODE_VIEWPORT);
    level_layer->add_child(copy);
    auto* sprite=memnew(Sprite2D);
    sprite->set_centered(false);
    copy->add_child(sprite);
    Ref<ShaderMaterial> material;
    material.instantiate();
    material->set_shader(ResourceLoader::get_singleton()->load("res://level_effect.gdshader"));
    material->set_shader_parameter("effect_blend",int(resource->blend));
    material->set_shader_parameter("effect_opacity",resource->opacity/255.f);
    sprite->set_material(material);
    sprite->set_modulate(Color(resource->tint[0]/255.f,resource->tint[1]/255.f,resource->tint[2]/255.f,1));
    auto& event=map_events.emplace(harvest_effect_id++,MapEvent{resource,cell,copy,sprite,
        int(resource->first)+frame_offset}).first->second;
    event.one_shot=true;
    event.frame_offset=frame_offset;
    event.delay_ms=delay_ms;
    event.attached_actor=attached_actor;
    event.attached_pose=attached_pose;
    if (projectile) {
        projectile->velocity=spell_initial_velocity(projectile->destination-projectile->position);
    }
    event.projectile=projectile;
    // 4495D0 leaves the light flag enabled for these independent effects.
    const bool monster_effect=effect>=500 || projectile.has_value() || attached_actor;
    const bool light_disabled=effect==520 || effect==521 || effect==529 ||
        effect==535 || effect==536 || effect==544 || effect==545 || effect==551 ||
        effect==575 || effect==578 || effect==593 || effect==595;
    event.emits_light=monster_effect && !light_disabled && !attached_actor && resource->outer_radius>0;
    if (section==mir3::EffectCatalog::Section::magic) {
        const int sample=10*effect+(projectile?10001:10002);
        if (effect<500) {event.sound_sample=sample;event.sound_voice=play_spell_sound(sample,cell);}
        else play_actor_sound(0,sample,cell);
    }
    if (delay_ms) sprite->hide();
}

void Mir3HudView::clear_map_events() {
    while (!actor_sounds.empty() && actor_sounds.begin()->first<=mir3::SpellSoundVoices::key(0,0))
        remove_actor_sound(actor_sounds.begin()->first);
    spell_sound_voices={};
    ground_events.clear();
    spell_particles.clear();
    sync_spell_particles();
    event_marks.clear();
    for(auto& [id,event]:map_events) {
        event.copy->hide();
        event.copy->queue_free();
    }
    map_events.clear();
}

void Mir3HudView::sync_spell_particles() {
    if (!spell_particle_layer) return;
    auto* renderer=RenderingServer::get_singleton();
    std::size_t used=0;
    for (const auto* slots:{&spell_particles.sparks,&spell_particles.glows,&spell_particles.fragments}) {
        for (const auto& slot:*slots) {
            if (!slot) continue;
            const auto& particle=*slot;
            const auto image=ground_event_frames.find(particle.resource_frame());
            if (image==ground_event_frames.end() || particle.size<=0) continue;
            if (used==spell_particle_items.size()) {
                const auto item=renderer->canvas_item_create();
                renderer->canvas_item_set_parent(item,spell_particle_layer->get_canvas_item());
                renderer->canvas_item_set_draw_index(item,int(used));
                spell_particle_items.push_back(item);
            }
            const auto item=spell_particle_items[used++];
            renderer->canvas_item_clear(item);
            renderer->canvas_item_set_visible(item,true);
            const Vector2 extent(particle.size,particle.size);
            const Rect2 bounds(Vector2(particle.position[0],particle.position[1])-extent*0.5f,extent);
            renderer->canvas_item_set_copy_to_backbuffer(item,true,bounds);
            renderer->canvas_item_set_material(item,spell_particle_materials[particle.glow()?1:0]->get_rid());
            const auto& tint=particle.color;
            renderer->canvas_item_add_texture_rect(item,bounds,image->second.image->get_rid(),false,
                Color(tint[0]/255.f,tint[1]/255.f,tint[2]/255.f));
        }
    }
    for (;used<spell_particle_items.size();++used) renderer->canvas_item_set_visible(spell_particle_items[used],false);
}

void Mir3HudView::emit_event_particles(Vector2i cell) {
    const auto camera=map_camera();
    spell_particles.emit_ice({float(cell.x*48+24-camera.x),float(cell.y*32-camera.y)},weather);
}

void Mir3HudView::advance_map_events(double milliseconds) {
    const auto camera=map_camera();
    const int ms=int(milliseconds);
    if(ms>0) {
        for(auto& [cell,mark]:event_marks) mark.age+=milliseconds;
        std::erase_if(event_marks,[](const auto& entry){return entry.second.age>entry.second.lifetime;});
        queue_redraw();
    }
    // A hit or a ground wave can create effects while advancing this map.
    // Newly spawned effects begin with their full lifetime on the next update.
    const auto next_effect_id=harvest_effect_id;
    for(auto& [id,event]:map_events) {
        if (id>=next_effect_id) break;
        if(ms>0 && event.secondary && terrain.base_walkable(event.cell.x,event.cell.y)) {
            if(event.frame>=101 && event.frame<106) emit_event_particles(event.cell);
            if(event.frame>=107)
                event_marks.insert_or_assign({event.cell.x,event.cell.y},
                    GroundEvent{event.cell,260,0,false,3000,180.f/255});
        }
        event.elapsed+=milliseconds;
        if (event.spell_effect) {
            event.lifetime_ms+=milliseconds;
            if (event.lifetime_ms>60000) {
                event.frame=int(event.resource->end)+event.frame_offset;
                event.copy->hide();event.copy->queue_free();continue;
            }
        }
        if(event.remaining_ms>=0) {
            event.remaining_ms=std::max(0,event.remaining_ms-int(milliseconds));
            if(!event.remaining_ms) {
                event.tint=std::max(0,event.tint-5);
                const float tint=event.tint/255.f;
                event.sprite->set_modulate(Color(tint,tint,tint));
                if(!event.tint) {
                    event.copy->hide();
                    event.copy->queue_free();
                    continue;
                }
            }
        }
        if (event.attached_actor) {
            const auto monster=scene_chickens.find(event.attached_actor);
            const auto npc=scene_npcs.find(event.attached_actor);
            int pose=-1;
            if (monster!=scene_chickens.end()) {
                const auto& actor=monster->second;
                pose=monster_overlay_pose(actor,event.attached_pose);
            }
            if (npc!=scene_npcs.end()) pose=int(npc->second.standing_frame);
            if (pose<0) {
                event.frame=int(event.resource->end)+event.frame_offset;
                event.copy->hide();
                event.copy->queue_free();
                continue;
            }
            event.cell=monster!=scene_chickens.end()?monster->second.cell:npc->second.cell;
            event.frame=int(event.resource->first)+event.frame_offset+pose;
            if (event.frame>=int(event.resource->end)+event.frame_offset &&
                monster!=scene_chickens.end() && monster->second.action_overlay==id)
                event.frame=int(event.resource->end)+event.frame_offset-1;
            if (event.frame>=int(event.resource->end)+event.frame_offset) {
                event.copy->hide();
                event.copy->queue_free();
                continue;
            }
            event.elapsed=0;
        }
        if (event.follow_actor) {
            const auto actor=event.follow_actor;
            if (actor==static_cast<std::uint32_t>(local_actor_identity))
                event.follow_position=Vector2(player_pixel());
            else if (const auto monster=scene_chickens.find(actor);monster!=scene_chickens.end()) {
                const auto& target=monster->second;
                event.follow_position=Vector2(target.walk_frame>=0?target.walk_pixel():target.cell*Vector2i(48,32));
            } else if (const auto human=scene_humans.find(actor);human!=scene_humans.end()) {
                const auto& target=human->second;
                event.follow_position=target.bounds.has_area()?target.anchor+Vector2(camera):
                    Vector2(target.cell*Vector2i(48,32));
            } else if (const auto npc=scene_npcs.find(actor);npc!=scene_npcs.end())
                event.follow_position=Vector2(npc->second.cell*Vector2i(48,32));
            else if (event.spell_effect) event.follow_actor=0;
        }
        double flight_ms=milliseconds;
        if (event.delay_ms) {
            if (event.spell_effect) {
                // 44B7E0比较总年龄；等待中清帧计时，解除时使用本次完整delta。
                if (event.lifetime_ms<event.delay_ms) { event.elapsed=0;continue; }
            } else {
                if (event.elapsed<event.delay_ms) continue;
                event.elapsed-=event.delay_ms;
                flight_ms=event.elapsed;
            }
            event.delay_ms=0;
        }
        if (event.staged_spell && !event.light_tail) {
            auto& staged=*event.staged_spell;
            const auto change=staged.advance(std::uint32_t(std::max(0,ms)));
            if (change==mir3::StagedSpell::Change::load_explosion) {
                const auto* resource=effect_catalog.find(mir3::EffectCatalog::Section::explosion,int(staged.skill));
                if (!resource || resource->end<=resource->first || !resource->interval ||
                    !prepare_effect_library(resource->library)) {
                    event.frame=int(event.resource->end);
                    event.copy->hide();event.copy->queue_free();continue;
                }
                event.resource=resource;
                staged.load_explosion({int(resource->first),int(resource->end),resource->interval});
                Ref<ShaderMaterial> material=event.sprite->get_material();
                material->set_shader_parameter("effect_blend",int(resource->blend));
                material->set_shader_parameter("effect_opacity",resource->opacity/255.f);
                event.sprite->set_modulate(Color(resource->tint[0]/255.f,resource->tint[1]/255.f,resource->tint[2]/255.f));
            }
            event.frame=staged.frame;
            event.follow_position=Vector2(event.cell*Vector2i(48,32))+Vector2(staged.offset[0],staged.offset[1]);
            if (change==mir3::StagedSpell::Change::finished) {
                if (!event.emits_light) {
                    event.copy->hide();event.copy->queue_free();continue;
                }
                event.light_tail=true;
                --event.frame;
            }
        }
        bool spell_impact=false;
        if (event.spell_effect && !event.light_tail && event.elapsed>event.resource->interval) {
            event.elapsed=0;
            ++event.frame;
            if (event.frame>=int(event.resource->end)+event.frame_offset) {
                if (event.one_shot) {
                    event.frame=int(event.resource->end)+event.frame_offset-1;
                    event.light_tail=true;
                } else event.frame=int(event.resource->first)+event.frame_offset;
            }
        }
        if (event.projectile) {
            auto& flight=*event.projectile;
            const auto actor=flight.target_actor;
            bool following=false;
            if (actor) {
                following=true;
                if (const auto monster=scene_chickens.find(actor);monster!=scene_chickens.end())
                    flight.destination=Vector2(monster->second.walk_frame>=0?monster->second.walk_pixel():
                        monster->second.cell*Vector2i(48,32));
                else if (const auto human=scene_humans.find(actor);human!=scene_humans.end())
                    flight.destination=human->second.bounds.has_area()?
                        human->second.anchor+Vector2(camera):Vector2(human->second.cell*Vector2i(48,32));
                else if (const auto npc=scene_npcs.find(actor);npc!=scene_npcs.end())
                    flight.destination=Vector2(npc->second.cell*Vector2i(48,32));
                else if (actor==static_cast<std::uint32_t>(local_actor_identity))
                    flight.destination=Vector2(player_pixel());
                else following=false;
            }
            if (!following && event.spell_effect) flight.target_actor=0;
            if (following) {
                const auto position=Vector2(std::trunc(flight.position.x),std::trunc(flight.position.y));
                const auto delta=flight.destination-position;
                const int dominant=std::max(1,int(std::max(std::abs(delta.x),std::abs(delta.y))));
                flight.velocity=delta*(500/dominant);
            }
            // 44B7E0 uses the same float accumulator for both projectile sources.
            constexpr double step=double(0.0011111111380159855f);
            flight.position=Vector2(float(double(flight.velocity.x)*step*flight_ms+float(flight.position.x)),
                float(double(flight.velocity.y)*step*flight_ms+float(flight.position.y)));
            const auto pixel=Vector2(std::trunc(flight.position.x),std::trunc(flight.position.y));
            const auto distance=(flight.destination-pixel).abs();
            const bool approaching=distance.x<flight.previous_distance.x || distance.y<flight.previous_distance.y;
            if (event.spell_effect && !following && flight.untargeted_timeout && event.lifetime_ms>=3000) {
                event.frame=int(event.resource->end)+event.frame_offset;
                event.copy->hide();event.copy->queue_free();continue;
            }
            const bool timeout_flight=event.spell_effect && !following && flight.untargeted_timeout;
            if (!timeout_flight && ((flight_ms>0 && !approaching) || (distance.x<=15 && distance.y<=15))) {
                const auto target=Vector2i((flight.destination/Vector2(48,32)).round());
                const auto* resource=effect_catalog.find(mir3::EffectCatalog::Section::explosion,flight.effect);
                if (!resource || (!event.orbit && (resource->end<=resource->first || !resource->interval)) ||
                    !prepare_effect_library(resource->library)) {
                    event.copy->hide();event.copy->queue_free();continue;
                }
                // 44B7E0 swaps resources in place and retains the animation timer.
                event.resource=resource;event.frame=int(resource->first);event.frame_offset=0;
                event.cell=target;event.follow_actor=following?actor:0;
                event.follow_position=flight.destination;event.one_shot=true;
                if (event.spell_effect && following) play_spell_sound(10*flight.effect+10002,target);
                if (!event.spell_effect && (following || flight.effect==594))
                    play_actor_sound(0,10*flight.effect+10002,target);
                Ref<ShaderMaterial> material=event.sprite->get_material();
                material->set_shader_parameter("effect_blend",int(resource->blend));
                material->set_shader_parameter("effect_opacity",resource->opacity/255.f);
                event.sprite->set_modulate(Color(resource->tint[0]/255.f,resource->tint[1]/255.f,resource->tint[2]/255.f));
                event.projectile.reset();
                spell_impact=true;
            }
            if (!spell_impact) flight.previous_distance=distance;
            if (!spell_impact && event.orbit && ms>0) {
                const auto pixel=event.orbit->advance(float(flight.position.x),float(flight.position.y),event.orbit_direction);
                event.follow_position=Vector2(pixel[0],pixel[1]);
            }
        }
        if (!spell_impact && event.emits_light) {
            event.light_clock+=flight_ms;
            const auto effect=event.resource->id;
            if (event.light_tail && (effect==22 || effect==538 || effect==539 || event.ground_wave ||
                (event.spell_effect && mir3::SpellLight::fades(static_cast<mir3::SkillId>(effect))))) {
                event.light_phase=0;
                if (milliseconds>0) event.light_fade+=2;
                const auto& r=*event.resource;
                int strongest=std::max(*std::ranges::max_element(r.inner_color),
                    *std::ranges::max_element(r.outer_color));
                if (event.spell_effect || event.ground_wave) {
                    strongest=0;
                    for (const auto& color:{r.inner_color,r.outer_color})
                        for (const auto channel:color) strongest=std::max(strongest,channel<2?int(channel)+256:int(channel));
                }
                if (strongest<=event.light_fade+2) {
                    event.frame=int(r.end)+event.frame_offset;
                    event.copy->hide();
                    event.copy->queue_free();
                    continue;
                }
            } else if (event.light_clock>200) {
                if (event.light_tail) {
                    const auto& r=*event.resource;
                    if (r.inner_radius-event.light_shrink<=1 || r.outer_radius<=r.inner_radius) {
                        event.frame=int(r.end)+event.frame_offset;
                        event.copy->hide();
                        event.copy->queue_free();
                        continue;
                    }
                    event.light_phase=0;
                    ++event.light_shrink;
                    event.light_clock=150;
                } else {
                    event.light_clock=0;
                    event.light_phase=(event.light_phase+1)%(event.resource->light_steps+1);
                }
            }
        }
        const int interval=event.ground_wave && event.resource->id==9 && event.frame>=1925 &&
            terrain.base_walkable(event.cell.x,event.cell.y)?80:event.resource->interval;
        if(!spell_impact && !event.spell_effect && !event.staged_spell && !event.light_tail && event.elapsed>interval) {
            event.elapsed=0;
            const int previous_frame=event.frame++;
            if (event.ground_wave && event.resource->id==int(mir3::MagicEffectId::lightning_trail) && previous_frame==int(event.resource->first)) {
                const auto pixel=event.cell*Vector2i(48,32)-camera+Vector2i(24,-16);
                spell_particles.emit_lightning({float(pixel.x),float(pixel.y)},weather);
            }
            if (event.ground_wave && (event.resource->id==int(mir3::MagicEffectId::fire_wave) ||
                event.resource->id==int(mir3::MagicEffectId::ice_wave))) {
                const bool fire=event.resource->id==int(mir3::MagicEffectId::fire_wave);
                const int end=int(event.resource->end)+event.frame_offset;
                const bool walkable=terrain.base_walkable(event.cell.x,event.cell.y);
                if (fire && walkable && event.frame<1915) ++event.frame;
                const int residue_frame=end-(fire?48:26);
                if (walkable && previous_frame<residue_frame && event.frame>=residue_frame) {
                    if (fire) {
                        const auto id=harvest_effect_id;
                        show_map_impact(weather.between(330,334),event.cell);
                        if (const auto ember=map_events.find(id);ember!=map_events.end()) {
                            ember->second.remaining_ms=10000;
                            ember->second.one_shot=false;
                            ember->second.sprite->set_modulate(Color(1,1,1));
                        }
                    }
                    const int mark=fire?220:260+weather.between(0,1);
                    ground_events.emplace(harvest_effect_id++,GroundEvent{event.cell,mark,0,false,2000,180.f/255});
                }
                if (!fire && walkable && event.frame<end-26) emit_event_particles(event.cell);
                if (event.frame>=end-(fire?30:20)) event.frame=end;
            }
            if(event.secondary) {
                if(!event.ending && event.frame>=99) event.frame=100;
                if(event.ending && event.frame>=110) {
                    event.copy->hide();
                    event.copy->queue_free();
                    continue;
                }
            } else if(event.frame>=int(event.resource->end)+event.frame_offset) {
                if(event.one_shot && (!event.projectile || event.spell_effect)) {
                    if (!event.emits_light) {
                        event.copy->hide();
                        event.copy->queue_free();
                        continue;
                    }
                    event.light_tail=true;
                    --event.frame;
                } else {
                    event.frame=int(event.resource->first)+event.frame_offset;
                }
            }
        }
        if(event.secondary) {
            const auto anchor=Vector2(event.cell.x*48-camera.x,event.cell.y*32-camera.y);
            const auto update_sprite=[this](Sprite2D* view,int id,Vector2 position) {
                const auto* image=level_frame(id);
                view->set_visible(image!=nullptr);
                if(image) {
                    view->set_texture(image->image);
                    view->set_position(position+image->offset);
                }
            };
            update_sprite(event.sprite,event.frame,anchor);
            if(event.frame<=100) update_sprite(event.secondary,event.frame+20,anchor+Vector2(12,8));
            else event.secondary->hide();
            continue;
        }
        auto world=event.projectile?event.projectile->position:
            event.follow_position.value_or(Vector2(event.cell*Vector2i(48,32)));
        if (event.projectile && event.sound_voice && (-event.sound_voice-1)%mir3::SpellSoundVoices::voices<5)
            position_spell_sound(event.sound_voice,event.sound_sample,Vector2i(world/Vector2(48,32)));
        if (event.ground_wave && event.resource->id==int(mir3::MagicEffectId::lightning_trail)) {
            event.sprite->hide();continue; // 44CDA0仅触发粒子，不调用普通WIL贴图绘制。
        }
        if ((event.staged_spell || event.spell_effect) && event.light_tail) {
            if (event.spell_effect && !event.emits_light) {
                event.copy->hide();event.copy->queue_free();continue;
            }
            event.sprite->hide();
            continue;
        }
        if (event.attached_pose==MonsterEffectPose::walk && event.attached_actor)
            world=Vector2(scene_chickens.at(event.attached_actor).walk_pixel());
        const auto emit_orbit=[&]() {
            if (!event.orbit) return false;
            event.sprite->hide();
            if (event.projectile && !event.light_tail && ms>0) {
                const auto pixel=event.follow_position.value_or(event.projectile->position)-Vector2(camera);
                const auto offset=effect_library_offsets[event.resource->library];
                const auto emitter=mir3::SpellOrbit::emitter({int(pixel.x),int(pixel.y)},event.orbit_direction,
                    int(offset.x),int(offset.y));
                spell_particles.emit({float(emitter[0]),float(emitter[1])},weather);
            }
            return true;
        };
        if (emit_orbit()) continue;
        if (event.resource->library==137) {
            const auto* image=level_frame(event.frame);
            event.sprite->set_visible(image!=nullptr);
            if (image) {
                effect_library_offsets[event.resource->library]=image->offset;
                event.sprite->set_texture(image->image);
                event.sprite->set_position(world-Vector2(camera)+image->offset);
            }
            continue;
        }
        const int frame_key=(event.resource->library<<16)|event.frame;
        auto found=event_frames.find(frame_key);
        if(found==event_frames.end()) {
            auto& archive=event.resource->library==81?event_archive:monster_effect_archives.at(event.resource->library);
            if(const auto source=archive.frame(event.frame))
                found=event_frames.emplace(frame_key,MapActorFrame{
                    texture(source->pixels.rgba,source->width,source->height),
                    Vector2(source->offset_x,source->offset_y),{}}).first;
        }
        event.sprite->set_visible(found!=event_frames.end());
        if(found!=event_frames.end()) {
            effect_library_offsets[event.resource->library]=found->second.offset;
            event.sprite->set_texture(found->second.image);
            event.sprite->set_position(world-Vector2(camera)+found->second.offset);
        }
    }
    std::erase_if(map_events,[](const auto& pair) {
        const auto& event=pair.second;
        return event.copy->is_queued_for_deletion() || !event.tint || (event.ending && event.frame>=110) ||
            (event.one_shot && event.frame>=int(event.resource->end)+event.frame_offset);
    });
    if (ms>0) spell_particles.advance(ms,weather);
    sync_spell_particles();
}

void Mir3HudView::clear_level_effects() {
    for (auto& effect:level_effects) { effect.copy->hide();effect.copy->queue_free(); }
    level_effects.clear();
}
bool Mir3HudView::receive_level_up(const mir3::ReceiveHeader& header) {
    const bool local=header.parameters[2]==0;
    if (!local && header.parameters[2]!=1) return true; // 原版忽略其余分支。
    Vector2i cell=player_cell;
    int actor=0;
    if (local) {
        model.level=std::uint8_t(header.parameters[0]);model.experience=header.recognition;
        player_facing=4;walk_frame=-1;movement_ms=0;map_route.clear();player_harvest_impact=false;player_action.start(7);
    } else {
        if (header.recognition>INT_MAX) return true;
        actor=int(header.recognition);
        const auto it=scene_humans.find(actor);if (it==scene_humans.end()) return true;
        auto& human=it->second;cell=human.cell;human.facing=4;
        human.movement_frame=-1;human.movement_ms=0;human.bump=false;human.action.start(7);
    }
    const auto* resource=effect_catalog.find(mir3::EffectCatalog::Section::magic,mir3::LevelUpEffect::resource_id);
    if (resource && resource->library==137 && resource->blend==2 && resource->end>resource->first) {
        auto* copy=memnew(BackBufferCopy);copy->set_copy_mode(BackBufferCopy::COPY_MODE_VIEWPORT);
        level_layer->add_child(copy);
        auto* sprite=memnew(Sprite2D);sprite->set_centered(false);copy->add_child(sprite);
        Ref<ShaderMaterial> material;material.instantiate();
        material->set_shader(ResourceLoader::get_singleton()->load("res://level_effect.gdshader"));
        sprite->set_material(material);
        sprite->set_modulate(Color(resource->tint[0]/255.f,resource->tint[1]/255.f,resource->tint[2]/255.f,1));
        level_effects.push_back({mir3::LevelUpEffect(*resource,cell.x,cell.y),copy,sprite});
        play_actor_sound(actor,mir3::LevelUpEffect::sound_id,cell);
        advance_level_effects(0);
    }
    if (local) post_chat(String::utf8(message(1469).c_str()),mir3::chat_palette(2),mir3::chat_palette(7));
    queue_redraw();return true;
}
void Mir3HudView::advance_level_effects(double milliseconds) {
    const auto camera=map_camera();
    for (auto& effect:level_effects) {
        effect.playback.advance(std::uint32_t(milliseconds));
        if (effect.playback.phase==mir3::LevelUpEffect::Phase::finished) { effect.copy->hide();effect.copy->queue_free();continue; }
        const auto* frame=level_frame(int(effect.playback.frame));
        effect.sprite->set_visible(frame!=nullptr);
        if (frame) {
            effect.sprite->set_texture(frame->image);
            effect.sprite->set_position(Vector2(effect.playback.cell_x*48-camera.x,effect.playback.cell_y*32-camera.y)+frame->offset);
        }
    }
    std::erase_if(level_effects,[](const auto& effect) {return effect.playback.phase==mir3::LevelUpEffect::Phase::finished;});
    if (!level_effects.empty()) queue_redraw();
}

bool Mir3HudView::apply_game_message(const PackedByteArray& encoded) {
    if (server_groups.visible || login_visible || character_selection.visible || login_movie_purpose!=LoginMoviePurpose::none) return false;
    if (melee_spells.receive({reinterpret_cast<const char*>(encoded.ptr()),std::size_t(encoded.size())})) return true;
    const auto reply=mir3::parse_action_reply({reinterpret_cast<const char*>(encoded.ptr()),std::size_t(encoded.size())});
    if (reply==mir3::ActionReply::good) { action_prediction.confirm();return true; }
    if (reply==mir3::ActionReply::fail) {
        if (const auto origin=action_prediction.reject(player_life.dead || (model.max_hp && model.hp==0))) {
            pending_local_action.reset();map_route.clear();
            forced_motion.reset();
            player_cell=step_origin=step_destination=Vector2i(origin->cell[0],origin->cell[1]);
            player_facing=origin->facing;walk_frame=-1;movement_ms=0;
            player_action={};player_spell={};player_harvest_impact=false;player_idle.reset();
            set_minimap_position(player_cell.x,player_cell.y);
            queue_redraw();
        }
        return true;
    }
    const auto header=mir3::ReceiveHeader::decode({encoded.ptr(),std::size_t(encoded.size())},game_connection.key);
    if (!header) return false;
    using RepairPhase=MerchantService::Phase;
    switch(header->opcode) {
    case mir3::protocol::SM_DROPITEM_SUCCESS:
        if(encoded.size()!=16) return false;
        pending_drop={};
        return true;
    case mir3::protocol::SM_DROPITEM_FAIL: {
        const auto name=mir3::decode_six_bit(encoded.ptr()+16,encoded.size()-16);
        if(!name || name->empty() || name->size()>15) return false;
        if(!pending_drop || pending_drop.make_index!=header->recognition ||
                mir3::ansi_to_utf8(std::string_view(*name).substr(0,name->find('\0')))!=pending_drop.name) return true;
        // 429785 restores a rejected ordinary drop to the bag.
        if (!bag.insert(pending_drop)) {
            if (bag.held) return false;
            bag.held=pending_drop;
            bag.held_slot=-1;
        }
        pending_drop={};
        queue_redraw();
        return true;
    }
    case mir3::protocol::SM_INVENTITEM:
        if(encoded.size()!=16) return false;
        update_bag_stack_count(header->recognition,header->parameters[0]);
        return true;
    case mir3::protocol::SM_STORAGE_OPEN:
    case mir3::protocol::SM_SENDUSERSELL:
    case mir3::protocol::SM_OPEN_REPAIR:
        if(encoded.size()!=16 || !header->recognition) return false;
        if(header->opcode==mir3::protocol::SM_STORAGE_OPEN) close_shop();
        merchant_service={};
        merchant_service.storage=header->opcode==mir3::protocol::SM_STORAGE_OPEN;
        merchant_service.actor=header->recognition;
        merchant_service.selling=header->opcode==mir3::protocol::SM_SENDUSERSELL;
        if(!inventory.visible) toggle_inventory();
        // 4282C0 positions the service inventory at (519, 0) even if it was open.
        inventory.rect[0]=519;
        inventory.rect[1]=0;
        raise_panel(Panel::inventory);
        queue_redraw();
        return true;
    case mir3::protocol::SM_STORAGE_LIST: {
        // 4282C0 binds the merchant at LABEL_436 only when opening the shop window.
        if(!storage_list && (!header->recognition || header->recognition>INT_MAX)) return false;
        const int actor=storage_list?shop_actor:int(header->recognition);
        std::string_view remaining(reinterpret_cast<const char*>(encoded.ptr()+16),encoded.size()-16);
        std::vector<ShopOffer> records;
        for(unsigned i=0;i<header->parameters[2];++i) {
            const auto end=remaining.find('/');
            if(end==remaining.npos || end!=134) return false;
            const auto data=mir3::decode_six_bit(reinterpret_cast<const std::uint8_t*>(remaining.data()),end);
            if(!data || data->size()!=100) return false;
            const auto b=[&](int n){return std::uint32_t(std::uint8_t((*data)[n]));};
            const auto name=data->find(char(0));
            if(!name || name>=30) return false;
            const auto id=b(62)|(b(63)<<8)|(b(64)<<16)|(b(65)<<24);
            if(!id) return false;
            records.push_back({mir3::ansi_to_utf8(std::string_view(*data).substr(0,name)),
                int(b(36)|(b(37)<<8)),0,id,int(b(71)|(b(72)<<8)),int(b(61))});
            remaining.remove_prefix(end+1);
        }
        if(!remaining.empty()) return false;
        const auto pending_withdrawal=storage_list?storage_pending:0u;
        close_shop();
        merchant_service={};
        shop_actor=actor;
        shop_offers=std::move(records);
        storage_list=shop_show_offers=true;
        storage_pending=pending_withdrawal;
        shop_offer_index=shop_offers.empty()?-1:0;
        queue_redraw();
        return true;
    }
    case mir3::protocol::SM_STORAGE_DEPOSIT_OK:
    case mir3::protocol::SM_STORAGE_FULL:
    case mir3::protocol::SM_STORAGE_DEPOSIT_FAIL:
        if(encoded.size()!=16) return false;
        if(header->opcode!=mir3::protocol::SM_STORAGE_DEPOSIT_OK)
            open_group_dialog(header->opcode==mir3::protocol::SM_STORAGE_FULL?1449:1450);
        if(!merchant_service.storage || merchant_service.phase!=RepairPhase::paying) return true;
        if(header->opcode==mir3::protocol::SM_STORAGE_DEPOSIT_OK) {
            auto caption=message(header->parameters[1]?1447:1448);
            if(auto at=caption.find("%s");at!=std::string::npos) caption.replace(at,2,merchant_service.name);
            if(auto at=caption.find("%d");at!=std::string::npos) caption.replace(at,2,std::to_string(header->parameters[1]));
            post_chat(String::utf8(caption.c_str()),11861940,0);
            update_bag_stack_count(merchant_service.item,header->parameters[0],true);
        }
        merchant_service.phase=RepairPhase::choosing;
        queue_redraw();
        return true;
    case mir3::protocol::SM_STORAGE_WITHDRAW_OK:
    case mir3::protocol::SM_STORAGE_WITHDRAW_FAIL:
    case mir3::protocol::SM_STORAGE_BAG_FULL: {
        if(encoded.size()!=16) return false;
        if(header->opcode!=mir3::protocol::SM_STORAGE_WITHDRAW_OK)
            open_group_dialog(header->opcode==mir3::protocol::SM_STORAGE_WITHDRAW_FAIL?1445:1446);
        if(!storage_list || !storage_pending) return true;
        const auto instance=std::exchange(storage_pending,0u);
        if(header->opcode==mir3::protocol::SM_STORAGE_WITHDRAW_OK) {
            const auto quantity=header->parameters[1];
            if(quantity) update_bag_stack_count(instance,quantity);
            const auto item=std::ranges::find(shop_offers,instance,&ShopOffer::id);
            if(item!=shop_offers.end() && header->parameters[0]) item->count=header->parameters[0];
            else if(item!=shop_offers.end()) {
                shop_offers.erase(item);
                shop_offer_index=-1;
                shop_offer_page=std::min(shop_offer_page,std::max(0,(int(shop_offers.size())-1)/12));
            }
        }
        queue_redraw();
        return true;
    }
    case mir3::protocol::SM_SENDBUYPRICE:
    case mir3::protocol::SM_REPAIR_QUOTE:
        if(encoded.size()!=16) return false;
        // 42A9BF/42AAEC replace the current service quote when another price arrives.
        if(merchant_service.actor && !merchant_service.storage &&
                (merchant_service.phase==RepairPhase::quoting || merchant_service.phase==RepairPhase::quoted ||
                    merchant_service.phase==RepairPhase::unavailable) &&
                merchant_service.selling==(header->opcode==mir3::protocol::SM_SENDBUYPRICE)) {
            merchant_service.price=header->recognition;
            // 42AB20 disables confirmation but preserves the selected item.
            merchant_service.phase=merchant_service.price?RepairPhase::quoted:RepairPhase::unavailable;
            queue_redraw();
        }
        return true;
    case mir3::protocol::SM_USERSELLITEM_OK:
    case mir3::protocol::SM_SELL_STACK_OK:
    case mir3::protocol::SM_USERSELLITEM_FAIL:
    case mir3::protocol::SM_SELL_STACK_FAIL: {
        if(encoded.size()!=16) return false;
        const bool stack=header->opcode==mir3::protocol::SM_SELL_STACK_OK;
        const bool success=stack || header->opcode==mir3::protocol::SM_USERSELLITEM_OK;
        // 648 and 1314 update the balance before checking the service window. 649 always prompts.
        if(success) inventory_gold=header->recognition;
        if(header->opcode==mir3::protocol::SM_USERSELLITEM_FAIL) open_group_dialog(1455);
        if(!merchant_service.actor || !merchant_service.selling || merchant_service.phase!=RepairPhase::paying) {
            if(success) queue_redraw();
            return true;
        }
        if(success) {
            auto caption=message(stack?1454:1453);
            if(auto at=caption.find("%s");at!=std::string::npos) caption.replace(at,2,merchant_service.name);
            if(auto at=caption.find("%d");at!=std::string::npos) caption.replace(at,2,std::to_string(header->parameters[1]));
            post_chat(String::utf8(caption.c_str()),11861940,0);
            if (stack) update_bag_stack_count(merchant_service.item,header->parameters[0],true);
            else {
                // 42AA6F clears only the bag slot on ordinary sale success.
                const auto item=std::ranges::find_if(bag.items,[&](const auto& entry) {
                    return entry && entry.make_index==merchant_service.item;
                });
                if (item!=bag.items.end()) bag.take(int(item-bag.items.begin()));
            }
        }
        merchant_service.phase=RepairPhase::choosing;
        merchant_service.price=0;
        queue_redraw();
        return true;
    }
    case mir3::protocol::SM_REPAIR_OK:
    case mir3::protocol::SM_REPAIR_FAIL:
        if(encoded.size()!=16) return false;
        // 42AB37 updates the balance before testing the repair window; 42AB9A always opens the failure prompt.
        if(header->opcode==mir3::protocol::SM_REPAIR_OK) inventory_gold=header->recognition;
        else open_group_dialog(1457);
        if(merchant_service.actor && merchant_service.phase==RepairPhase::paying && !merchant_service.selling && !merchant_service.storage) {
            if(header->opcode==mir3::protocol::SM_REPAIR_OK) {
                for(auto& item:bag.items) if(item && item.make_index==merchant_service.item && item.name==merchant_service.name) {
                    for(int field=0;field<2;++field) {
                        const auto value=header->parameters[field];
                        item.server_record[66+2*field]=std::uint8_t(value);
                        item.server_record[67+2*field]=std::uint8_t(value>>8);
                    }
                    break;
                }
                post_chat(String::utf8(message(1456).c_str()),11861940,0);
            }
            merchant_service.phase=RepairPhase::choosing;
            merchant_service.price=0;
        }
        queue_redraw();
        return true;
    default:break;
    }
    if(header->opcode==mir3::protocol::SM_QUEST_CHILD_ADDED) {
        constexpr auto offset=mir3::ReceiveHeader::encoded_size;
        int parent_id=0;
        auto records=mir3::decode_quest_list(*header,{encoded.ptr()+offset,std::size_t(encoded.size()-offset)},&parent_id);
        if(!records || records->empty()) return false;
        post_chat(String::utf8(message(1432).c_str()),mir3::chat_palette(2),mir3::chat_palette(3));
        if(!quest.list_initialized) {
            send_client_packet(mir3::protocol::CM_QUERYQUESTLIST,0,String());
            return true;
        }
        const auto parent=std::ranges::find_if(quest.entries,[&](const auto& entry) {
            return entry.wire && entry.wire->id==parent_id;
        });
        if(parent==quest.entries.end()) return true;
        const auto duplicate_parent=std::ranges::find_if(quest.entries,[&](const auto& entry) {
            return entry.wire && entry.wire->id==header->parameters[0];
        });
        for(auto& record:*records) {
            if(duplicate_parent!=quest.entries.end() &&
                    std::ranges::find(duplicate_parent->children,record.id,&mir3::QuestRecord::id)!=duplicate_parent->children.end()) break;
            record.title=mir3::ansi_to_utf8(record.title);
            record.summary=mir3::ansi_to_utf8(record.summary);
            record.completed=parent->completed;
            parent->children.push_back(std::move(record));
            parent->children_loaded=true;
        }
        queue_redraw();
        return true;
    }
    if(header->opcode==mir3::protocol::SM_QUEST_CHILDREN || header->opcode==mir3::protocol::SM_QUEST_CHILD_TEXT) {
        constexpr auto offset=mir3::ReceiveHeader::encoded_size;
        const std::span<const std::uint8_t> body{encoded.ptr()+offset,std::size_t(encoded.size()-offset)};
        const auto parent=std::ranges::find_if(quest.entries,[&](const auto& entry) {
            return entry.wire && entry.wire->id==header->parameters[2];
        });
        if(parent==quest.entries.end()) {
            if(!quest.list_initialized && header->opcode==mir3::protocol::SM_QUEST_CHILDREN)
                send_client_packet(mir3::protocol::CM_QUERYQUESTLIST,0,String());
            return true;
        }
        if(header->opcode==mir3::protocol::SM_QUEST_CHILDREN) {
            auto records=mir3::decode_quest_list(*header,body);
            if(!records || records->empty()) return false;
            for(auto& record:*records) {
                record.title=mir3::ansi_to_utf8(record.title);
                record.completed=parent->completed;
            }
            parent->children=std::move(*records);
            parent->children_loaded=true;
            if(quest.description_parent==parent-quest.entries.begin()) {
                quest.description_child=-1;
                quest.description_first=0;
            }
            quest.scroll(0);
        } else {
            if(body.size()>1366) return false;
            const auto decoded=mir3::decode_six_bit(body.data(),body.size());
            if(!decoded) return false;
            const auto ansi=std::string_view(*decoded).substr(0,decoded->find(char(0)));
            if(ansi.size()>1023) return false;
            const auto child=std::ranges::find(parent->children,int(header->parameters[0]),&mir3::QuestRecord::id);
            if(child!=parent->children.end()) child->summary=mir3::ansi_to_utf8(ansi);
        }
        queue_redraw();
        return true;
    }
    if(header->opcode==mir3::protocol::SM_QUEST_LIST || header->opcode==mir3::protocol::SM_QUEST_ADDED) {
        constexpr auto offset=mir3::ReceiveHeader::encoded_size;
        auto records=mir3::decode_quest_list(*header,{encoded.ptr()+offset,std::size_t(encoded.size()-offset)});
        if(!records) return false;
        const bool full=header->opcode==mir3::protocol::SM_QUEST_LIST;
        if(!full) {
            post_chat(String::utf8(message(1433).c_str()),mir3::chat_palette(2),mir3::chat_palette(3));
            if(!quest.list_initialized) {
                send_client_packet(mir3::protocol::CM_QUERYQUESTLIST,0,String());
                return true;
            }
        }
        for(auto& record:*records) {
            record.title=mir3::ansi_to_utf8(record.title);
            record.summary=mir3::ansi_to_utf8(record.summary);
        }
        quest.sync_list(std::move(*records),full);
        queue_redraw();
        return true;
    }
    switch(header->opcode) {
    case mir3::protocol::SM_TRADE_OPEN:case mir3::protocol::SM_TRADE_OPEN_FAIL:
    case mir3::protocol::SM_TRADE_ADD_OK:case mir3::protocol::SM_TRADE_ADD_FAIL:
    case mir3::protocol::SM_TRADE_REMOVE_OK:case mir3::protocol::SM_TRADE_REMOVE_FAIL:
    case mir3::protocol::SM_TRADE_CANCELLED:case mir3::protocol::SM_TRADE_PEER_ADD:
    case mir3::protocol::SM_TRADE_PEER_REMOVE:case mir3::protocol::SM_TRADE_GOLD:
    case mir3::protocol::SM_TRADE_PEER_GOLD:case mir3::protocol::SM_TRADE_COMPLETE:
        return apply_trade_wire(*header,{encoded.ptr()+16,std::size_t(encoded.size()-16)});
    default:break;
    }
    if (header->opcode==mir3::protocol::SM_EAT_OK || header->opcode==mir3::protocol::SM_EAT_FAIL)
        return apply_inventory_use_reply(header->opcode,header->parameters[0]);
    if (header->opcode==mir3::protocol::SM_TAKEON_OK || header->opcode==mir3::protocol::SM_TAKEON_FAIL ||
        header->opcode==mir3::protocol::SM_TAKEOFF_OK || header->opcode==mir3::protocol::SM_TAKEOFF_FAIL) {
        if (!apply_equipment_reply(header->opcode)) return false;
        if (header->opcode==mir3::protocol::SM_TAKEON_OK || header->opcode==mir3::protocol::SM_TAKEOFF_OK) {
            std::array<std::uint8_t,16> feature{};
            for (int i=0;i<4;++i) feature[i]=std::uint8_t(header->recognition>>(8*i));
            if (const auto appearance=mir3::HumanAppearance::parse(feature,false)) {
                set_player_body(appearance->type,appearance->clothing);
                set_player_attachments(appearance->hair,appearance->weapon);
                model.profession=appearance->profession;
            }
        }
        return true;
    }
    if (header->opcode==mir3::protocol::SM_SENDUSEITEMS) {
        const auto decoded=mir3::EquipmentWire::parse(*header,{encoded.ptr()+16,std::size_t(encoded.size()-16)});
        if (!decoded) return false;
        auto next=equipment.slots;
        for (const auto& entry:decoded->records) {
            const auto& record=entry.item;
            const auto sprite=item_frame(record.icon());
            if (sprite.is_null()) return false;
            const auto [width,height]=mir3::InventoryGrid::dimensions(sprite->get_width(),sprite->get_height());
            next[entry.slot]={mir3::EquipmentState::category(record.mode()),record.icon(),record.count(),width,height,
                record.mode(),record.identity(),mir3::ansi_to_utf8(record.name()),record.stack_mode(),record.raw};
        }
        equipment.slots=std::move(next);queue_redraw();return true;
    }
    if (const auto change=mir3::EquipmentDurability::parse(*header)) {
        if (change->slot<12 && equipment.slots[change->slot]) {
            auto& item=equipment.slots[change->slot];
            item.server_record[66]=std::uint8_t(change->current);item.server_record[67]=std::uint8_t(change->current>>8);
            item.server_record[68]=std::uint8_t(change->maximum);item.server_record[69]=std::uint8_t(change->maximum>>8);
            if (change->warns(item.std_mode)) {
                auto caption=message(1470);
                if (const auto at=caption.find("%s");at!=std::string::npos) caption.replace(at,2,item.name);
                post_chat(String::utf8(caption.c_str()),mir3::chat_palette(2),mir3::chat_palette(7));
            }
            queue_redraw();
        }
        return true;
    }
    if (header->opcode==mir3::protocol::SM_ADDITEM || header->opcode==mir3::protocol::SM_BAGITEMS ||
        header->opcode==mir3::protocol::SM_DELITEM || header->opcode==mir3::protocol::SM_UPDATEITEM)
        return apply_inventory_wire(*header,{encoded.ptr()+16,std::size_t(encoded.size()-16)});
    if (header->opcode==mir3::protocol::SM_ADDMAGIC || header->opcode==mir3::protocol::SM_SENDMYMAGIC) {
        const auto wire=mir3::SkillWire::parse(*header,{encoded.ptr()+16,std::size_t(encoded.size()-16)});
        if (!wire) return false;
        PackedByteArray records;records.resize(wire->records.size());
        std::copy(wire->records.begin(),wire->records.end(),records.ptrw());
        return apply_skill_records(header->opcode,wire->count,records);
    }
    if (header->opcode==mir3::protocol::SM_DELMAGIC)
        return apply_skill_removal(header->recognition);
    if (header->opcode==mir3::protocol::SM_MAGIC_LVEXP)
        return apply_skill_progress(header->recognition,std::uint32_t(header->parameters[1])|(std::uint32_t(header->parameters[2])<<16),header->parameters[0]&255);
    if (header->opcode==mir3::protocol::SM_RIDEHORSE) return apply_mount_reply(header->opcode,header->recognition);
    switch (header->opcode) {
    case mir3::protocol::SM_OPENGUILDDLG:
    case mir3::protocol::SM_GUILD_OVERVIEW:
    case mir3::protocol::SM_GUILD_OVERVIEW_UPDATE:
    case mir3::protocol::SM_SENDGUILDMEMBERLIST:
    case mir3::protocol::SM_GUILD_MEMBERS_UPDATE:
    case mir3::protocol::SM_GUILD_MEMBERS_REFRESH: {
        const auto decoded=mir3::decode_six_bit(encoded.ptr()+16,encoded.size()-16);
        if (!decoded || decoded->size()>8191) return false;
        PackedByteArray body;body.resize(decoded->size());
        std::copy(decoded->begin(),decoded->end(),body.ptrw());
        const bool members=header->opcode==mir3::protocol::SM_SENDGUILDMEMBERLIST ||
            header->opcode==mir3::protocol::SM_GUILD_MEMBERS_UPDATE || header->opcode==mir3::protocol::SM_GUILD_MEMBERS_REFRESH;
        return members?apply_guild_members_reply(header->opcode,body):apply_guild_overview_reply(header->opcode,body);
    }
    default:break;
    }
    if (const auto value=mir3::GuildNotice::parse(*header)) {
        if (value->refresh_members) {
            emit_signal("guild_members_requested");
            send_client_packet(mir3::protocol::CM_GUILDMEMBERLIST,0,String());
        }
        if (value->message) open_group_dialog(value->message);
        return true;
    }
    if (const auto value=mir3::GroupMessage::parse(*header,{encoded.ptr()+16,std::size_t(encoded.size()-16)})) {
        using Kind=mir3::GroupMessage::Kind;
        if (value->kind==Kind::allow) group_allow=true;
        if (value->kind==Kind::clear) group_members.clear();
        if (value->kind==Kind::members) {
            PackedStringArray members;
            for (const auto& member:value->members) members.append(String::utf8(mir3::ansi_to_utf8(member).c_str()));
            group_members=members;
        }
        if (value->notice) open_group_dialog(value->notice);
        queue_redraw();return true;
    }
    if (const auto value=mir3::GoldChange::parse(*header)) {
        if (gold_sound && audio_options.effects_enabled) { gold_sound->play();++gold_sound_count; }
        if (value->gain(inventory_gold)>0)
            chat_history.post(value->caption(message(1471),inventory_gold),mir3::chat_palette(2),mir3::chat_palette(7));
        inventory_gold=value->balance;queue_redraw();return true;
    }
    if (header->opcode==mir3::protocol::SM_LEVELUP) return receive_level_up(*header);
    if (const auto value=mir3::ActorExperience::parse(*header)) {
        model.experience=value->total;weapon_training=value->apply_training(weapon_training);
        if (value->extra_notice) chat_history.post(message(1513),mir3::chat_palette(2),mir3::chat_palette(3));
        chat_history.post(value->caption(message(value->message_id())),mir3::chat_palette(2),mir3::chat_palette(3));
        queue_redraw();return true;
    }
    if (const auto value=mir3::ActorWeightChange::parse(*header)) {
        model.weight=value->weight;model.wear_weight[0]=value->wear;model.hand_weight[0]=value->hand;
        queue_redraw();return true;
    }
    if (const auto value=mir3::ActorSubAbility::parse(*header)) {
        server_sub_ability=*value;model.status_bonus_percent={value->hit_point,value->speed_point};
        queue_redraw();return true;
    }
    if (header->opcode==mir3::protocol::SM_ABILITY) {
        const auto value=mir3::ActorAbility::parse(*header,{encoded.ptr()+16,std::size_t(encoded.size()-16)});
        if (!value) return false;
        server_ability=*value;inventory_gold=value->gold;
        model.level=value->level;model.profession=value->profession;
        model.defence=value->defence;model.magic_defence=value->magic_defence;
        model.attack=value->attack;model.magic_attack=value->magic;model.spirit_attack=value->spirit;
        model.hp=value->hp;model.mp=value->mp;model.max_hp=value->max_hp;model.max_mp=value->max_mp;
        model.experience=value->experience;model.next_experience=value->next_experience;
        model.weight=value->weight;model.max_weight=value->max_weight;
        model.wear_weight=value->wear;model.hand_weight=value->hand;
        for (int i=0;i<7;++i) {
            elements.set(0,i,value->element_attack[i]);elements.set(1,i,value->element_strength[i]);elements.set(2,i,value->element_weakness[i]);
        }
        elements.attack_words=value->element_attack_words;
        // 能力同步不是受击消息：更新头顶血条，不产生伤害跳字。
        auto& health=scene_health[0];health.current=value->hp;health.mana=value->mp;health.maximum=value->max_hp;
        health.damage.enabled=damage_numbers;
        queue_redraw();return true;
    }
    if (header->opcode==mir3::protocol::SM_LOGON) {
        const auto actor=mir3::LoginActor::parse(*header,{encoded.ptr()+16,std::size_t(encoded.size()-16)});
        if (!actor) return false;
        login_actor_pending=*actor;set_local_actor_identity(int(actor->identity));
        apply_login_actor();return true;
    }
    if (header->opcode==mir3::protocol::SM_DECODEKEY) {
        const auto body=mir3::decode_six_bit(encoded.ptr()+16,encoded.size()-16);
        if (!body) return false;
        const auto response=mir3::GameConnection::challenge_response(std::uint32_t(local_actor_identity),
            [](int low,int high) { return int(UtilityFunctions::randi_range(low,high)); });
        send_client_packet(mir3::protocol::CM_DECODEKEY_RESPONSE,0,String::utf8(response.c_str(),response.size()));
        return game_connection.change_key(*body);
    }
    PackedByteArray key;key.resize(4);std::copy(game_connection.key.begin(),game_connection.key.end(),key.ptrw());
    const bool accepted=apply_scene_message(encoded,key) || apply_npc_message(encoded,key);
    // 42EC70：公共队列中的换图先完成，随后的人物包才能使用新地图。
    if (accepted && (header->opcode==mir3::protocol::SM_NEWMAP || header->opcode==mir3::protocol::SM_CHANGEMAP)) {
        process_map_changes();apply_login_actor();
    }
    return accepted;
}

void Mir3HudView::enter_character_selection() {
    const bool from_login=login_visible;
    clear_game_scene();login_visible=false;
    chat_input->set_text("");chat_input->set_secret(false);chat_input->set_byte_limit(mir3::ChatInputSpec::limit);
    character_creation={};creation_bounds={};
    character_selection={};character_selection.visible=true;
    login_audio.enter(mir3::LoginAudio::Page::selection);
    selection_pending.reset();selection_pending_notice=0;selection_elapsed=0;selection_fraction_ms=0;selection_notice.close();selection_bounds={};
    outgoing_messages.sequence=0;
    get_window()->set_content_scale_size(Vector2i(640,480));
    get_window()->set_size(Vector2i(640,480));
    queue_redraw();
    if (from_login) {
        emit_signal("login_connection_close_requested");
        emit_signal("selection_endpoint_ready",String::utf8(login_flow.endpoint.host.c_str()),login_flow.endpoint.port);
    } else emit_signal("game_connection_close_requested");
    emit_signal("character_selection_connection_requested");
    if (from_login) connect_page(ConnectionPage::selection,login_flow.endpoint);
    else if (!login_flow.previous_endpoint.host.empty()) connect_page(ConnectionPage::selection,login_flow.previous_endpoint);
}

bool Mir3HudView::open_server_groups() {
    if (!loaded) return false;
    const auto data=std::filesystem::u8path(actor_resource_root.utf8().get_data());
    mir3::ServerGroups next;if (!next.load(data.parent_path()/"SelLogin.ini")) return false;
    if (!server_group_archive.open(data/"LoginInterface")) return false;
    std::map<int,Ref<ImageTexture>> images;
    for (int id:{0,1,3,4,7,8,11,12,13,15}) {
        const auto source=server_group_archive.frame(id);
        if (!source) { if (id==1) continue;return false; }
        images[id]=texture(source->pixels.rgba,source->width,source->height);
        if (id==1) server_glow_offset=Vector2i(source->offset_x,source->offset_y);
    }
    clear_game_scene();connection.close();connection_page=ConnectionPage::none;
    connection_failed=false;connection_retry_at=0;
    login_movie.close();login_movie_purpose=LoginMoviePurpose::none;
    login_visible=false;character_selection.visible=false;
    server_groups=std::move(next);server_group_frames=std::move(images);
    server_group_exit_pressed=server_group_scroll_drag=false;server_group_pointer=Vector2(-1,-1);
    server_group_scroll_tick=ticks_msec();
    if (!server_glow) {
        server_glow=memnew(Node2D);add_child(server_glow);
        Ref<Shader> shader;shader.instantiate();shader->set_code("shader_type canvas_item; render_mode unshaded, blend_add;");
        Ref<ShaderMaterial> material;material.instantiate();material->set_shader(shader);server_glow->set_material(material);
        server_glow->connect("draw",callable_mp(this,&Mir3HudView::draw_server_group_glow));
    }
    server_glow->show();get_window()->set_content_scale_size(Vector2i(800,600));get_window()->set_size(Vector2i(800,600));
    queue_redraw();return true;
}

void Mir3HudView::draw_server_group_glow() {
    if (!server_groups.visible || server_groups.selected<0 || !server_group_frames.contains(1)) return;
    const auto& group=server_groups.groups[server_groups.selected];
    server_glow->draw_texture(server_group_frames.at(1),Vector2(group.x-55,group.y-120)+server_glow_offset);
}

void Mir3HudView::draw_server_groups() {
    paint_canvas=this;draw_texture(server_group_frames.at(0),Vector2());
    draw_texture(server_group_frames.at(server_group_exit_pressed?4:3),Vector2(760,13));
    for (int i=0;i<int(server_groups.groups.size());++i) {
        const auto& group=server_groups.groups[i];if (!group.x && !group.y) continue;
        const auto label=mir3::ansi_to_utf8(group.label);
        for (const auto d:std::array<Vector2i,4>{{{-1,0},{1,0},{0,-1},{0,1}}})
            text(label,{group.x+5+d.x,group.y+25+d.y,260,16},0,mir3::TextFont::normal,false);
        text(label,{group.x+5,group.y+25,260,16},15790320,mir3::TextFont::normal,false);
        const bool selected=server_groups.selected==i;
        const int frame=selected?13:Rect2(group.x,group.y,33,21).has_point(server_group_pointer)?12:11;
        draw_texture(server_group_frames.at(frame),Vector2(group.x-(selected?3:0),group.y-(selected?3:0)));
    }
    if (server_groups.selected>=0) {
        const auto& group=server_groups.groups[server_groups.selected];
        text(mir3::ansi_to_utf8(group.title),{28,20,172,15},16448250,mir3::TextFont::normal,true);
        for (int row=0;row<20 && server_groups.first+row<int(group.servers.size());++row) {
            const auto& server=group.servers[server_groups.first+row];if (server.caption=="empty") continue;
            const Rect2 bounds(10,70+25*row,190,15);
            draw_texture(server_group_frames.at(bounds.has_point(server_group_pointer)?8:7),bounds.position+Vector2(7,-5));
            text(mir3::ansi_to_utf8(server.caption),{10,70+25*row,190,15},16448250,mir3::TextFont::normal,true);
        }
        const float ratio=group.servers.size()>1?float(server_groups.first)/float(group.servers.size()-1):0;
        draw_texture(server_group_frames.at(15),Vector2(204,84+int(447*ratio)));
    } else draw_texture(server_group_frames.at(15),Vector2(204,84));
    server_glow->queue_redraw();
}

void Mir3HudView::server_groups_input(const Ref<InputEvent>& event) {
    const auto scroll=[&](float y) {
        if (server_groups.selected<0) return;
        const int count=int(server_groups.groups[server_groups.selected].servers.size());
        const float span=447+server_group_frames.at(15)->get_height();
        server_groups.first=std::clamp(int(count*std::clamp((y-84)/span,0.f,1.f)),0,std::max(0,count-1));
    };
    const Ref<InputEventMouseMotion> motion=event;
    if (motion.is_valid()) {
        server_group_pointer=motion->get_position();if (server_group_scroll_drag) scroll(server_group_pointer.y);
        queue_redraw();return;
    }
    const Ref<InputEventMouseButton> mouse=event;if (mouse.is_null()) return;
    const auto at=mouse->get_position();server_group_pointer=at;
    if (mouse->is_pressed() && (mouse->get_button_index()==MOUSE_BUTTON_WHEEL_UP || mouse->get_button_index()==MOUSE_BUTTON_WHEEL_DOWN)) {
        server_groups.scroll(mouse->get_button_index()==MOUSE_BUTTON_WHEEL_UP?-1:1);queue_redraw();return;
    }
    if (mouse->get_button_index()!=MOUSE_BUTTON_LEFT) return;
    const bool exit_hit=Rect2(Vector2(760,13),server_group_frames.at(3)->get_size()).has_point(at);
    if (!mouse->is_pressed()) {
        server_group_scroll_drag=false;server_group_exit_pressed=false;
        if (exit_hit) get_tree()->quit();queue_redraw();return;
    }
    for (int i=0;i<int(server_groups.groups.size());++i) {
        const auto& group=server_groups.groups[i];
        if (Rect2(group.x,group.y,40,40).has_point(at)) server_groups.choose_group(i);
    }
    if (server_groups.selected>=0) {
        for (int row=0;row<20;++row) if (Rect2(10,70+25*row,190,15).has_point(at)) {
            if (const auto config=server_groups.login(row)) { open_configured_login(*config);return; }
        }
        const auto size=server_group_frames.at(15)->get_size();
        const int count=int(server_groups.groups[server_groups.selected].servers.size());
        const float ratio=count>1?float(server_groups.first)/(count-1):0;
        if (Rect2(204,84,size.x,447+size.y).has_point(at) && ticks_msec()-server_group_scroll_tick>10) {
            if (Rect2(Vector2(204,84+int(447*ratio)),size).has_point(at)) server_group_scroll_drag=true;
            else if (count) {
                const float next=std::clamp(ratio+(at.y<84+int(447*ratio)+15?-10.f:10.f)/count,0.f,1.f);
                server_groups.first=std::min(count-1,int(count*next));
            }
            server_group_scroll_tick=ticks_msec();
        }
    }
    server_group_exit_pressed=exit_hit;queue_redraw();
}

bool Mir3HudView::open_login() {
    if (!loaded || !prepare_character_selection()) return false;
    mir3::LoginConfig configured;
    const auto root=std::filesystem::u8path(actor_resource_root.utf8().get_data()).parent_path();
    if (!configured.load(root/"Mir3.ini")) return false;
    return open_configured_login(std::move(configured));
}

bool Mir3HudView::open_configured_login(mir3::LoginConfig configured) {
    if (!loaded || !prepare_character_selection()) return false;
    server_groups.visible=false;if (server_glow) server_glow->hide();
    for (const int index:{0,2,10,11,12,13,14,15,16,17}) {
        const auto source=selection_source(index);if (!source) return false;
        selection_frames[index]=texture(source->pixels.rgba,source->width,source->height);
    }
    clear_game_scene();character_selection.visible=false;character_creation.close();selection_delete.close();selection_notice.close();
    selection_pending.reset();selection_pending_notice=0;exit_transition={};
    load_login_announcement();
    login_config=std::move(configured);login_flow={};login_flow.servers=login_config.servers;login_flow.endpoint=login_config.gateway;
    login_visible=true;login_fraction_ms=0;login_notice_pressed=false;login_buttons={};outgoing_messages.sequence=0;
    int max_width=0,max_height=0;
    for (const auto& server:login_flow.servers) {
        const auto measure=mir3::raster_text(mir3::ansi_to_utf8(server.caption),600,30,false,mir3::TextFont::normal);
        max_width=std::max(max_width,measure.text_width);max_height=std::max(max_height,measure.text_height);
    }
    const int row_height=max_height+20;
    login_server_box=Rect2(20,60,max_width+90,row_height*login_flow.servers.size()+20);
    login_server_bounds.clear();
    for (int i=0;i<int(login_flow.servers.size());++i) login_server_bounds.emplace_back(20,70+i*row_height,max_width+90,row_height);
    get_window()->set_content_scale_size(Vector2i(640,480));get_window()->set_size(Vector2i(640,480));
    sync_login_editor();queue_redraw();
    emit_signal("login_connection_requested",String::utf8(login_flow.endpoint.host.c_str()),login_flow.endpoint.port);
    connect_page(ConnectionPage::login,login_flow.endpoint);
    start_login_movie(LoginMoviePurpose::opening,"wemade.dat");
    return true;
}

void Mir3HudView::sync_login_editor() {
    if (!login_visible) return;
    if (login_flow.phase!=mir3::LoginFlow::Phase::credentials || !login_flow.editor_visible || login_flow.notice.message) {
        chat_input->release_focus();chat_input->hide();return;
    }
    chat_input->set_byte_limit(24);chat_input->set_secret_character("*");chat_input->set_secret(login_flow.field==1);
    chat_input->set_position(Vector2(login_flow.field==1?326:128,440));chat_input->set_size(Vector2(99,14));
    chat_input->show();chat_input->focus_chat(String::utf8(mir3::ansi_to_utf8(login_flow.editor_text()).c_str()),true);
    chat_input->select_all();
}

void Mir3HudView::send_login_request(const mir3::LoginRequest& request) {
    send_client_packet(request.opcode,0,String::utf8(mir3::ansi_to_utf8(request.body).c_str()));
}

bool Mir3HudView::apply_login_message(PackedByteArray message) {
    if (!login_visible || !login_flow.receive({message.ptr(),std::size_t(message.size())})) return false;
    if (login_flow.phase==mir3::LoginFlow::Phase::leaving) {
        selection_session.configure(login_flow.account_name(),login_flow.session);
        exit_layer->show();exit_layer->queue_redraw();
    }
    sync_login_editor();queue_redraw();return true;
}

void Mir3HudView::finish_login_exit(const String& url) {
    emit_signal("login_url_requested",url);
    // 自动化场景只观察请求，不启动外部浏览器或结束测试进程。
    if (bool(get_meta("offline_preview",false))) return;
    OS::get_singleton()->shell_open(url);
    get_tree()->quit();
}

void Mir3HudView::login_input(const Ref<InputEvent>& event) {
    const Ref<InputEventMouseMotion> motion=event;
    if (motion.is_valid()) { login_pointer=motion->get_position();queue_redraw();return; }
    if (login_flow.phase==mir3::LoginFlow::Phase::leaving) return;
    const Ref<InputEventKey> key=event;
    const Ref<InputEventMouseButton> mouse=event;
    if (login_flow.notice.message) {
        bool close=key.is_valid() && key->is_pressed() && (key->get_keycode()==KEY_ENTER || key->get_keycode()==KEY_SPACE);
        if (mouse.is_valid() && mouse->get_button_index()==MOUSE_BUTTON_LEFT) {
            const bool hit=Rect2(Vector2(277,298),frame(1243)->get_size()).has_point(mouse->get_position());
            if (mouse->is_pressed()) login_notice_pressed=hit;
            else { close=hit;login_notice_pressed=false; }
        }
        if (close) { login_flow.close_notice();sync_login_editor(); }
        queue_redraw();return;
    }
    if (login_flow.phase==mir3::LoginFlow::Phase::credentials) {
        const auto editor=[&]() { const auto utf8=chat_input->limited_text().utf8();return mir3::utf8_to_ansi({utf8.get_data(),std::size_t(utf8.length())}); };
        if (key.is_valid() && key->is_pressed()) {
            if (key->get_keycode()==KEY_TAB) { login_flow.change_field(!login_flow.field,editor());sync_login_editor(); }
            else if (key->get_keycode()==KEY_ENTER) {
                const auto request=login_flow.submit(false,editor());sync_login_editor();if (request) send_login_request(*request);
            }
        }
        if (mouse.is_valid() && mouse->get_button_index()==MOUSE_BUTTON_LEFT) {
            const auto pos=mouse->get_position();login_pointer=pos;
            const std::array<Vector2i,4> origins{{{459,436},{139,379},{279,379},{439,379}}};
            int hit=-1;for (int i=0;i<4;++i) if (Rect2(origins[i],selection_frames.at(10+i*2)->get_size()).has_point(pos)) hit=i;
            if (mouse->is_pressed()) {
                if (Rect2(128,440,99,14).has_point(pos)) { login_flow.change_field(0,editor());sync_login_editor(); }
                else if (Rect2(326,440,99,14).has_point(pos)) { login_flow.change_field(1,editor());sync_login_editor(); }
                else if (hit>=0) login_buttons[hit]=2;
            } else {
                login_buttons={};
                if (hit>=0 && hud_sound) hud_sound->play();
                if (hit==0) { const auto request=login_flow.submit(true,editor());sync_login_editor();if (request) send_login_request(*request); }
                else if (hit==1 || hit==2) finish_login_exit(String::utf8(mir3::ansi_to_utf8(hit==1?login_config.account_url:login_config.password_url).c_str()));
                else if (hit==3) finish_login_exit(String::utf8(message(1077).c_str()));
            }
        }
    } else if (mouse.is_valid() && mouse->get_button_index()==MOUSE_BUTTON_LEFT) {
        int hit=-1;for (int i=0;i<int(login_server_bounds.size());++i) if (login_server_bounds[i].has_point(mouse->get_position())) hit=i;
        if (mouse->is_pressed()) login_flow.press_server(hit);
        else if (const auto request=login_flow.release_server(hit)) send_login_request(*request);
    }
    queue_redraw();
}

void Mir3HudView::draw_login() {
    paint_canvas=this;draw_rect(Rect2(0,0,640,480),Color(0,0,0));
    draw_texture(selection_frames.at(0),Vector2(0,60));
    if (login_flow.phase==mir3::LoginFlow::Phase::credentials) {
        draw_login_announcement();
        legacy_rect(Rect2(0,372,640,48),Color(50.f/255,50.f/255,60.f/255,100.f/255));
        draw_rect(Rect2(0,372,640,48),Color(100.f/255,100.f/255,100.f/255),false);
        const std::array<Vector2i,4> origins{{{459,436},{139,379},{279,379},{439,379}}};
        for (int i=0;i<4;++i) draw_texture(selection_frames.at(10+i*2+(login_buttons[i]==2?1:0)),origins[i]);
        const Rect2 login_bounds(origins[0],selection_frames.at(10)->get_size());
        if (login_bounds.has_point(login_pointer)) {
            legacy_rect(login_bounds,Color(150.f/255,100.f/255,50.f/255,50.f/255));
            draw_rect(login_bounds,Color(200.f/255,150.f/255,100.f/255),false);
        }
        draw_texture(selection_frames.at(2),Vector2(100,436));
        const auto inactive_text=[](std::string ansi) {
            const auto width=[&]() { return mir3::raster_text(mir3::ansi_to_utf8(ansi),400,20,false,mir3::TextFont::normal).text_width; };
            if (width()>100) while (!ansi.empty() && width()>=100) ansi.pop_back();
            return mir3::ansi_to_utf8(ansi);
        };
        if (login_flow.field!=1) text(inactive_text(login_flow.password_mask()),{326,440,100,14},0xffffff,mir3::TextFont::normal,false);
        if (login_flow.field!=0) text(inactive_text(login_flow.account_name()),{128,440,100,14},0xffffff,mir3::TextFont::normal,false);
    } else {
        legacy_rect(login_server_box,Color(100.f/255,100.f/255,100.f/255,120.f/255));
        draw_rect(login_server_box,Color(150.f/255,150.f/255,150.f/255),false);
        for (int i=0;i<int(login_flow.servers.size());++i) {
            const auto& server=login_flow.servers[i];const auto bounds=login_server_bounds[i];
            const bool chosen=login_flow.pressed_server==i,hover=login_flow.pressed_server==-1 && bounds.has_point(login_pointer);
            if (chosen || hover) { legacy_rect(bounds,chosen?Color(1,100.f/255,100.f/255,50.f/255):Color(100.f/255,100.f/255,100.f/255,50.f/255));draw_rect(bounds,chosen?Color(1,100.f/255,100.f/255):Color(100.f/255,100.f/255,200.f/255),false); }
            auto caption=mir3::ansi_to_utf8(server.caption);if (server.load>=1 && server.load<=3) caption+=" ("+message(125+server.load)+")";
            text(caption,{int(bounds.position.x)+20,int(bounds.position.y)+10,600,20},chosen?0x6464ff:(hover?0xff0000:0x1e1e1e),mir3::TextFont::normal,false);
        }
        for (int i=0;i<3;++i) text(message(129+i),{40+200*i,440,180,16},3328200,mir3::TextFont::normal,false);
    }
    if (login_flow.notice.message) {
        sprite(1240,{140,110});sprite(login_notice_pressed?1244:1243,{277,298});
        auto caption=message(login_flow.notice.message);
        if (login_flow.notice.argument) { const auto marker=caption.find("%d");if (marker!=std::string::npos) caption.replace(marker,2,std::to_string(*login_flow.notice.argument)); }
        text(caption,{163,194,308,95},0xc8faff,mir3::TextFont::dialog);
    }
}

const mir3::WilFrame* Mir3HudView::selection_source(int index) {
    auto found=selection_sources.find(index);
    if (found!=selection_sources.end()) return &found->second;
    auto source=selection_archive.frame(index);
    if (!source) return nullptr;
    return &selection_sources.emplace(index,std::move(*source)).first->second;
}

void Mir3HudView::update_creation_description() {
    creation_description.clear();
    const String source=String::utf8(message(216+character_creation.profession).c_str());
    String line;
    for (int i=0;i<source.length();++i) {
        line+=source.substr(i,1);const auto bytes=line.utf8();
        const auto size=mir3::raster_text({bytes.get_data(),std::size_t(bytes.length())},500,16,false,mir3::TextFont::normal);
        if (size.text_width>=430 || i+1==source.length()) { creation_description.emplace_back(bytes.get_data(),bytes.length());line=String(); }
    }
    const auto background=selection_source(80);
    creation_backdrop.clear();
    if (!background || background->width!=640 || background->height!=480) return;
    creation_backdrop=background->pixels.rgba;
    const int height=18*(int(creation_description.size())+1)+20;
    mir3::selection_rectangle(creation_backdrop,100,15,450,height,0x323232,50);
    mir3::selection_rectangle(creation_backdrop,100,15,450,1,0x969696,255);
    mir3::selection_rectangle(creation_backdrop,100,15,1,height,0x969696,255);
    mir3::selection_rectangle(creation_backdrop,549,15,1,height,0x969696,255);
    mir3::selection_rectangle(creation_backdrop,100,14+height,450,1,0x969696,255);
    const auto title=message(211+character_creation.selected)+message(213+character_creation.profession);
    constexpr std::array<unsigned,3> colors{0x96c8fa,11184890,0x96dc96};
    const auto title_bitmap=mir3::raster_text(title,430,18,false,mir3::TextFont::gold,messages.get(1074));
    // 486D60使用选角槽5180索引创建槽，空选角时落在已清零的前置控件区。
    const int title_job=character_selection.selected>=0 && character_selection.selected<2?
        character_creation.roles[character_selection.selected].profession:0;
    mir3::selection_text(creation_backdrop,title_bitmap.rgba,110,25,430,18,colors[title_job]);
    for (int i=0;i<int(creation_description.size());++i) {
        const auto bitmap=mir3::raster_text(creation_description[i],430,18,false,mir3::TextFont::normal,messages.get(1074));
        mir3::selection_text(creation_backdrop,bitmap.rgba,110,50+i*18,430,18,16775930);
    }

}

void Mir3HudView::open_character_creation() {
    if (character_selection.count>=2) { selection_notice.show(226);queue_redraw();return; }
    start_login_movie(LoginMoviePurpose::create,"CreateChr.dat");
}

void Mir3HudView::show_character_creation() {
    for (int id:{80,81,82,85,86,87,88,89,90,91,92,93,94,95,96,97,98}) {
        const auto source=selection_source(id);if (!source) return;
        selection_frames[id]=texture(source->pixels.rgba,source->width,source->height);
    }
    selection_notice.close();character_creation.open();creation_bounds={};update_creation_description();
    login_audio.enter(mir3::LoginAudio::Page::creation);
    chat_input->set_secret(false);chat_input->set_byte_limit(13);
    chat_input->set_position(Vector2(288,405));chat_input->set_size(Vector2(75,13));
    chat_input->show();chat_input->focus_chat("",true);queue_redraw();
}

void Mir3HudView::close_character_creation() {
    character_creation.close();chat_input->set_text("");chat_input->release_focus();chat_input->hide();
    chat_input->set_secret(false);chat_input->set_byte_limit(mir3::ChatInputSpec::limit);
    queue_redraw();
}

void Mir3HudView::close_selection_notice() {
    selection_notice.close();
    if (character_creation.visible) { chat_input->show();chat_input->focus_chat("",true); }
    queue_redraw();
}

void Mir3HudView::submit_character_creation() {
    if (!character_creation.visible || selection_notice.message) return;
    const auto bytes=chat_input->get_text().utf8();
    const auto name=mir3::utf8_to_ansi({bytes.get_data(),std::size_t(bytes.length())});
    if (name.empty()) return;
    if (const int error=mir3::CreateCharacterPage::name_error(name)) {
        selection_notice.show(error);chat_input->release_focus();chat_input->hide();queue_redraw();return;
    }
    if (const auto body=character_creation.request(selection_session.account,name))
        send_client_packet(mir3::protocol::CM_NEWCHR,0,String::utf8(mir3::ansi_to_utf8(*body).c_str()));
}

void Mir3HudView::creation_input(const Ref<InputEvent>& event) {
    const Ref<InputEventKey> key=event;
    if (key.is_valid() && key->is_pressed() && key->get_keycode()==KEY_ENTER) { submit_character_creation();return; }
    const std::array<Vector2i,5> origins{{{266,419},{308,419},{352,419},{450,444},{491,444}}};
    constexpr std::array<int,5> normal{91,94,97,85,88};
    const auto hit_at=[&](Vector2 pos) { for (int i=0;i<5;++i) if (Rect2(origins[i],selection_frames.at(normal[i])->get_size()).has_point(pos)) return i;return -1; };
    const Ref<InputEventMouseMotion> motion=event;
    if (motion.is_valid()) {
        const int hit=hit_at(motion->get_position());
        for (int i=0;i<5;++i) if (character_creation.buttons[i]!=2) character_creation.buttons[i]=i==hit?1:0;
        queue_redraw();return;
    }
    const Ref<InputEventMouseButton> mouse=event;
    if (mouse.is_null() || mouse->get_button_index()!=MOUSE_BUTTON_LEFT) return;
    const auto pos=mouse->get_position();const int hit=hit_at(pos);
    if (mouse->is_pressed()) {
        for (int i=0;i<2;++i) if (creation_bounds[i].has_point(pos) && character_creation.select(i)) update_creation_description();
        if (hit>=0) character_creation.buttons[hit]=2;
    } else {
        if (Rect2(294,404,77,15).has_point(pos)) chat_input->grab_focus();
        character_creation.buttons={};
        if (hit>=0 && hud_sound) hud_sound->play();
        if (hit>=0 && hit<3) { character_creation.set_profession(hit);update_creation_description(); }
        else if (hit==3) submit_character_creation();
        else if (hit==4) { close_character_creation();start_login_movie(LoginMoviePurpose::return_selection,"CreateChr.dat",true);request_selection_list(); }
    }
    queue_redraw();
}

void Mir3HudView::draw_character_creation() {
    const auto background=selection_source(80);if (!background || background->width!=640 || background->height!=480) return;
    if (creation_backdrop.size()!=640*480*4) return;
    auto pixels=creation_backdrop;
    for (int i=0;i<2;++i) {
        const auto& role=character_creation.roles[i];
        if (const auto shadow=selection_source(role.frame+20)) mir3::selection_blit(pixels,*shadow,role.x,role.y,mir3::SelectionBlend::shadow);
        const auto body=selection_source(i==character_creation.selected?role.frame:role.first_frame());
        if (body) mir3::selection_blit(pixels,*body,role.x,role.y,i==character_creation.selected?mir3::SelectionBlend::normal:mir3::SelectionBlend::inactive);
        if (i==character_creation.selected) if (const auto light=selection_source(role.frame+40)) mir3::selection_blit(pixels,*light,role.x,role.y,mir3::SelectionBlend::additive);
    }
    if (const auto shadow=selection_source(82)) { auto positioned=*shadow;positioned.offset_x=positioned.offset_y=0;mir3::selection_blit(pixels,positioned,201,434,mir3::SelectionBlend::shadow); }
    selection_surface=texture(pixels,640,480);draw_texture(selection_surface,Vector2());
    draw_texture(selection_frames.at(81),Vector2(247,384));
    const std::array<Vector2i,5> origins{{{266,419},{308,419},{352,419},{450,444},{491,444}}};
    constexpr std::array<std::array<int,3>,5> frames{{{91,92,93},{94,95,96},{97,98,98},{85,86,87},{88,89,90}}};
    for (int i=0;i<5;++i) draw_texture(selection_frames.at(frames[i][character_creation.buttons[i]]),origins[i]);
    draw_rect(Rect2(287,404,77,15),Color(120.f/255,90.f/255,60.f/255),false);
    const std::array<Vector2i,5> anchors{{{289,438},{332,438},{376,438},{469,458},{510,458}}};
    for (int i=0;i<5;++i) {
        if (!Rect2(origins[i],selection_frames.at(frames[i][0])->get_size()).has_point(selection_pointer)) continue;
        const auto caption=message(201+i);
        const int width=6*int(mir3::utf8_to_ansi(caption).size());
        const int x=anchors[i].x+10,y=anchors[i].y+2;
        const Rect2 bounds(x,y,width,15);
        legacy_rect(bounds,Color(125.f/255,125.f/255,75.f/255,50.f/255));
        draw_rect(bounds,Color(180.f/255,180.f/255,120.f/255),false);
        for (const auto offset:std::array<Vector2i,4>{{{1,2},{3,2},{2,1},{2,3}}})
            text(caption,{x+offset.x,y+offset.y,width+4,16},1644825,mir3::TextFont::normal,false);
        text(caption,{x+2,y+2,width+4,16},11862015,mir3::TextFont::normal,false);
        break;
    }
}

void Mir3HudView::open_selection_delete() {
    if (character_selection.selected<0) { selection_notice.show(227);queue_redraw();return; }
    selection_notice.close();selection_delete.open();
    chat_input->set_secret(false);chat_input->set_byte_limit(90);
    chat_input->set_position(Vector2(164,338));chat_input->set_size(Vector2(309,15));
    chat_input->show();chat_input->focus_chat("",true);queue_redraw();
}

void Mir3HudView::finish_selection_delete(bool confirm) {
    if (!selection_delete.visible) return;
    const auto input=chat_input->limited_text();
    selection_delete.close();chat_input->focus_chat("",true);chat_input->release_focus();chat_input->hide();
    chat_input->set_byte_limit(mir3::ChatInputSpec::limit);chat_input->layout(model.layout.expanded);
    if (confirm && character_selection.selected>=0 && !selection_session.account.empty()) {
        const auto& role=character_selection.roles[character_selection.selected];
        send_client_packet(mir3::protocol::CM_DELCHR,0,String::utf8(mir3::ansi_to_utf8(role.name).c_str())+"/"+input);
        request_selection_list();
    }
    queue_redraw();
}

bool Mir3HudView::set_selection_session(String account,int64_t number) {
    const auto utf8=account.utf8();
    return selection_session.configure(mir3::utf8_to_ansi({utf8.get_data(),std::size_t(utf8.length())}),number);
}

bool Mir3HudView::request_selection_list() {
    if (!character_selection.visible || selection_session.account.empty()) return false;
    if (selection_delete.visible) finish_selection_delete(false);
    selection_elapsed=0;selection_fraction_ms=0;selection_pending.reset();selection_pending_notice=0;
    character_selection.connecting=true;selection_notice.close();
    send_client_packet(mir3::protocol::CM_QUERYCHR,0,String::utf8(mir3::ansi_to_utf8(selection_session.query_body()).c_str()));
    queue_redraw();return true;
}

bool Mir3HudView::apply_selection_message(PackedByteArray message) {
    if (!character_selection.visible) return false;
    const auto header=mir3::ReceiveHeader::decode_plain({message.ptr(),std::size_t(message.size())});
    if (!header) return false;
    if (header->opcode==mir3::protocol::SM_STARTPLAY) {
        const auto name=character_selection.selected>=0?character_selection.roles[character_selection.selected].name:std::string{};
        if (!game_entry.receive({message.ptr(),std::size_t(message.size())},name)) return false;
        if (map_music) map_music->stop();
        start_login_movie(LoginMoviePurpose::enter_game,"StartGame.dat");return true;
    }
    if (header->opcode==mir3::protocol::SM_NEWCHR_SUCCESS) {
        close_character_creation();start_login_movie(LoginMoviePurpose::return_selection,"CreateChr.dat",true);return request_selection_list();
    }
    if (header->opcode==mir3::protocol::SM_NEWCHR_FAIL) {
        selection_notice.show(mir3::CreateCharacterPage::failure_message(header->recognition));
        chat_input->release_focus();chat_input->hide();queue_redraw();return true;
    }
    if (header->opcode==mir3::protocol::SM_DELCHR_SUCCESS || header->opcode==mir3::protocol::SM_DELCHR_FAIL) return request_selection_list();
    if (const int notice=mir3::SelectionNotice::reply_message(header->opcode)) {
        if (selection_pending) selection_pending_notice=notice;
        else { if (selection_delete.visible) finish_selection_delete(false);character_selection.connecting=false;selection_notice.show(notice); }
        queue_redraw();return true;
    }
    auto next=character_selection;
    if (!next.receive({message.ptr(),std::size_t(message.size())})) return false;
    selection_pending_notice=0;
    if (selection_elapsed<500) selection_pending=std::move(next);
    else { if (selection_delete.visible) finish_selection_delete(false);character_selection=std::move(next);selection_notice.show(character_selection.count==0?219:0);selection_bounds={}; }
    queue_redraw();return true;
}

void Mir3HudView::selection_input(const Ref<InputEvent>& event) {
    const Ref<InputEventMouseMotion> motion=event;
    if (motion.is_valid()) {
        selection_pointer=motion->get_position();
        if (character_creation.visible && !selection_notice.message) creation_input(event);
        queue_redraw();return;
    }
    if (character_selection.connecting) return;
    const Ref<InputEventKey> key=event;
    if (selection_delete.visible) {
        if (key.is_valid() && key->is_pressed()) {
            if (key->get_keycode()==KEY_TAB) selection_delete.tab();
            else if (key->get_keycode()==KEY_ENTER || key->get_keycode()==KEY_SPACE) finish_selection_delete(selection_delete.selected==0);
        }
        const Ref<InputEventMouseButton> button=event;
        if (button.is_valid() && button->get_button_index()==MOUSE_BUTTON_LEFT) {
            int hit=-1;
            for (const int i:{0,2}) {
                const auto icon=frame(i==0?1241:1245);
                if (icon.is_valid() && Rect2(Vector2(i==0?181:374,298),icon->get_size()).has_point(button->get_position())) hit=i;
            }
            if (button->is_pressed()) { selection_delete.pressed=hit;if (hit>=0) selection_delete.selected=hit; }
            else { selection_delete.pressed=-1;if (hit>=0) finish_selection_delete(hit==0); }
        }
        queue_redraw();return;
    }
    if (selection_notice.message && key.is_valid() && key->is_pressed() && (key->get_keycode()==KEY_ENTER || key->get_keycode()==KEY_SPACE)) {
        close_selection_notice();return;
    }
    if (character_creation.visible && !selection_notice.message) { creation_input(event);return; }
    const Ref<InputEventMouseButton> mouse=event;
    if (mouse.is_null() || mouse->get_button_index()!=MOUSE_BUTTON_LEFT) return;
    const auto pos=mouse->get_position();selection_pointer=pos;
    if (!mouse->is_pressed()) {
        selection_pressed=-1;
        if (!selection_notice.message && hud_sound) {
            const std::array<Vector2i,4> origins{{{440,93},{79,243},{259,49},{28,438}}};
            bool hit=false;
            for (int i=0;i<4;++i) hit|=Rect2(origins[i],selection_frames.at(52+2*i)->get_size()).has_point(pos);
            if (character_selection.count>2) {
                hit|=!character_selection.page && Rect2(Vector2(550,400),frame(1153)->get_size()).has_point(pos);
                hit|=character_selection.page && Rect2(Vector2(500,400),frame(1150)->get_size()).has_point(pos);
            }
            if (hit) hud_sound->play();
        }
    }
    if (selection_notice.message) {
        const auto button=frame(1243);
        const bool hit=button.is_valid() && Rect2(Vector2(277,298),button->get_size()).has_point(pos);
        if (mouse->is_pressed()) selection_notice.pressed=hit;
        else { if (hit) close_selection_notice();selection_notice.pressed=false; }
    } else if (mouse->is_pressed()) {
        const std::array<Vector2i,4> origins{{{440,93},{79,243},{259,49},{28,438}}};
        selection_pressed=-1;
        for (int i=0;i<4;++i)
            if (Rect2(origins[i],selection_frames.at(52+2*i)->get_size()).has_point(pos)) selection_pressed=i;
        if (character_selection.count>2) {
            if (!character_selection.page && Rect2(Vector2(550,400),frame(1153)->get_size()).has_point(pos)) selection_pressed=4;
            if (character_selection.page && Rect2(Vector2(500,400),frame(1150)->get_size()).has_point(pos)) selection_pressed=5;
        }
        for (int i=character_selection.page*2;i<std::min(character_selection.count,character_selection.page*2+2);++i)
            if (selection_bounds[i].has_point(pos)) { character_selection.select(i);break; }
    } else if (Rect2(Vector2(440,93),selection_frames.at(52)->get_size()).has_point(pos)) {
        open_character_creation();
    } else if (Rect2(Vector2(79,243),selection_frames.at(54)->get_size()).has_point(pos)) {
        open_selection_delete();
    } else if (Rect2(Vector2(259,49),selection_frames.at(56)->get_size()).has_point(pos)) {
        if (character_selection.selected<0) selection_notice.show(230);
        else if (!selection_session.account.empty()) {
            const auto& role=character_selection.roles[character_selection.selected];
            send_client_packet(mir3::protocol::CM_SELCHR,0,String::utf8(mir3::ansi_to_utf8(selection_session.start_body(role.name)).c_str()));
        }
    } else if (Rect2(Vector2(28,438),selection_frames.at(58)->get_size()).has_point(pos)) {
        finish_login_exit(String::utf8(message(1077).c_str()));
    } else if (character_selection.count>2) {
        const auto right=frame(1153),left=frame(1150);
        if (right.is_valid() && Rect2(Vector2(550,400),right->get_size()).has_point(pos)) character_selection.turn_page(1);
        else if (left.is_valid() && Rect2(Vector2(500,400),left->get_size()).has_point(pos)) character_selection.turn_page(0);
    }
    queue_redraw();
}

void Mir3HudView::draw_character_selection() {
    paint_canvas=this;
    if (character_creation.visible) {
        draw_character_creation();
        if (selection_notice.message) {
            sprite(1240,{140,110});sprite(selection_notice.pressed?1244:1243,{277,298});
            text(message(selection_notice.message),{163,194,308,95},0xc8faff,mir3::TextFont::dialog);
        }
        return;
    }
    const auto background=selection_source(50);
    if (!background || background->width!=640 || background->height!=480) return;
    auto pixels=background->pixels.rgba;
    for (int i=character_selection.page*2;i<std::min(character_selection.count,character_selection.page*2+2);++i) {
        const auto& role=character_selection.roles[i];
        if (const auto source=selection_source(role.frame+20)) mir3::selection_blit(pixels,*source,role.x,role.y,mir3::SelectionBlend::shadow);
        if (const auto source=selection_source(role.frame)) {
            mir3::selection_blit(pixels,*source,role.x,role.y,role.action?mir3::SelectionBlend::normal:mir3::SelectionBlend::inactive);
        }
        if (const auto source=selection_source(role.frame+40)) mir3::selection_blit(pixels,*source,role.x,role.y,mir3::SelectionBlend::additive);
    }
    selection_surface=texture(pixels,640,480);
    draw_texture(selection_surface,Vector2());
    constexpr std::array<mir3::HudLayout::Point,4> positions{{{440,93},{79,243},{259,49},{28,438}}};
    for (int i=0;i<4;++i) {
        const auto normal=selection_frames.at(52+i*2);
        const Rect2 bounds(point(positions[i]),normal->get_size());
        draw_texture(selection_frames.at(52+i*2-(selection_pressed==i?1:0)),point(positions[i]));
        if (bounds.has_point(selection_pointer)) {
            legacy_rect(bounds,Color(150.f/255,100.f/255,50.f/255,50.f/255));
            draw_rect(bounds,Color(200.f/255,150.f/255,100.f/255),false);
        }
    }
    if (character_selection.count>2) {
        sprite(character_selection.page?1155:(selection_pressed==4?1154:1153),{550,400});
        sprite(character_selection.page?(selection_pressed==5?1151:1150):1152,{500,400});
    }
    if (character_selection.selected>=0) {
        const auto& role=character_selection.roles[character_selection.selected];
        const auto name=message(206)+"   "+mir3::ansi_to_utf8(role.name),level=message(207)+"   "+std::to_string(role.level),job=message(208+role.profession);
        const auto measure=[&](const std::string& value) { return mir3::raster_text(value,500,16,false,mir3::TextFont::selection,messages.get(1074)).text_width; };
        const int width=std::max(measure(name),measure(job))+20;
        const Rect2 box(79,108,width,70);
        legacy_rect(box,Color(50.f/255,50.f/255,25.f/255,100.f/255));
        draw_rect(box,Color(100.f/255,100.f/255,80.f/255),false);
        text(name,{89,118,width-20,16},0x96c8ff,mir3::TextFont::selection);
        text(level,{89,138,width-20,16},0x96f096,mir3::TextFont::selection);
        text(job,{89,158,width-20,16},0xafffff,mir3::TextFont::selection);
    }
    if (character_selection.connecting) {
        sprite(1248,{155,150});
        text(message(200),{178,234,308,95},0xc8faff,mir3::TextFont::dialog);
    } else if (selection_delete.visible) {
        sprite(1240,{140,110});
        sprite(selection_delete.pressed==0?1242:1241,{181,298});
        sprite(selection_delete.pressed==2?1246:1245,{374,298});
        text(message(228),{183,194,278,75},0xc8faff,mir3::TextFont::dialog);
    } else if (selection_notice.message) {
        sprite(1240,{140,110});sprite(selection_notice.pressed?1244:1243,{277,298});
        text(message(selection_notice.message),{163,194,308,95},0xc8faff,mir3::TextFont::dialog);
    }
}

void Mir3HudView::_draw() {
    using Layout=mir3::HudLayout;
    if (!loaded) return;
    ground_sparkle_layer->queue_redraw();
    auto* renderer=RenderingServer::get_singleton();
    for (std::size_t index=0;index<terrain_draw_count;++index) {
        renderer->canvas_item_clear(terrain_draw_items[index]);
        renderer->canvas_item_set_visible(terrain_draw_items[index],false);
    }
    terrain_draw_count=0;
    terrain_draw_batch=RID();
    if (server_groups.visible) { draw_server_groups();return; }
    if (login_movie_purpose!=LoginMoviePurpose::none) {
        draw_rect(Rect2(0,0,640,480),Color(0,0,0));
        if (login_movie_surface.is_valid()) draw_texture_rect(login_movie_surface,Rect2(0,0,640,480),false);
        return;
    }
    if (login_visible) { draw_login();return; }
    if (character_selection.visible) { draw_character_selection();return; }
    draw_terrain();
    scene_light_layer->queue_redraw();
    world_overlay_layer->queue_redraw();
    hud_base->queue_redraw();
}

void Mir3HudView::draw_ground_sparkles() {
    if (!terrain_ready || login_visible || character_selection.visible || server_groups.visible ||
            login_movie_purpose!=LoginMoviePurpose::none) return;
    const auto camera=map_camera();
    for (const auto& item:ground_items.items) {
        if (item.sparkle_frame<0) continue;
        const auto& image=ground_sparkles[item.sparkle_frame];
        const Vector2 position(item.x*48-camera.x+8,item.y*32-camera.y);
        // 424C48 uses a cell anchor; 47F3F0 draws a quad sized from the frame, clipped by the device viewport.
        if (image.is_valid()) ground_sparkle_layer->draw_texture(image,position);
    }
}

void Mir3HudView::draw_world_overlays() {
    paint_canvas=world_overlay_layer;
    draw_scene_name();
    draw_ground_name();
    draw_scene_speech();
    draw_scene_health();
    paint_canvas=this;
}

void Mir3HudView::draw_hud_base() {
    using Layout=mir3::HudLayout;
    paint_canvas=hud_base;
    if (!model.layout.expanded) {
        sprite(1490,Layout::image_position(Layout::Image::portrait));
        const auto origin=point(Layout::image_position(Layout::Image::portrait));
        if (portrait_map_mode) {
            draw_portrait_map();
        } else if (active_portrait.is_valid()) {
            paint_canvas->draw_texture_rect(active_portrait,Rect2(point(Layout::image_position(Layout::Image::portrait)),Vector2(82,94)),false);
        } else if (const auto portrait=portrait_texture(portraits,portrait_frames,model.portrait_frame());portrait.is_valid()) {
            paint_canvas->draw_texture(portrait,origin);
        }
        if (target_focus.displayed()) {
            if (system_options.user_photos && target_custom_portrait.is_valid())
                paint_canvas->draw_texture_rect(target_custom_portrait,rect(Layout::custom_portrait_rect(true)),false);
            else if (target_portrait.is_valid())
                paint_canvas->draw_texture(target_portrait,origin);
            const auto badge=portrait_texture(portraits,portrait_frames,target_badge);
            if (badge.is_valid()) paint_canvas->draw_texture(badge,point(Layout::image_position(Layout::Image::target_badge)));
        }
    }
    // Preserve the recovered order from 0x438000 / 0x4390E0.
    if (model.layout.expanded) legacy_rect(rect(Layout::chat_backing),Color(30.f/255,30.f/255,30.f/255,150.f/255));
    sprite(1160,Layout::origin);
    auto chat=Layout::chat_frame(model.layout.expanded);
    sprite(model.layout.expanded?1162:1161,{chat[0],chat[1]});
    chat_lines();
    sprite(Layout::level_emblem_frame(model.emblem_selector,model.emblem_references,model.experience),Layout::level_emblem_position(model.layout.expanded));
    auto left=frame(1163),right=frame(model.orb_frame(1));
    if (left.is_valid() && right.is_valid()) {
        for (int side=0;side<2;++side) {
            auto crop=model.orb_crop(side,{left->get_width(),left->get_height()},{right->get_width(),right->get_height()});
            auto dest=rect(crop); dest.position+=point(Layout::image_position(side==0?Layout::Image::hp:Layout::Image::mp));
            if (crop[3]>0) paint_canvas->draw_texture_rect_region(side==0?left:right,dest,rect(crop));
        }
    }
    const int tooltip_kind=Layout::tooltip_at(model.pointer,model.shared_hp());
    if (tooltip_kind>=0 && tooltip_kind<=2 && left.is_valid() && right.is_valid()) tooltip(tooltip_kind);
    for (int meter:{1,0}) {
        auto image=frame(1166+meter); if (image.is_null()) continue;
        int hidden=model.meter_crop(meter,image->get_height());
        auto crop=Rect2(0,hidden,image->get_width(),image->get_height()-hidden);
        auto dest=crop; dest.position+=point(Layout::image_position(meter==0?Layout::Image::experience:Layout::Image::weight));
        if (crop.size.y>0) paint_canvas->draw_texture_rect_region(image,dest,crop);
        if (tooltip_kind==(meter==0?4:3)) tooltip(tooltip_kind);
    }
    text(std::to_string(model.level),Layout::level(model.layout.expanded),Layout::text_color(1),mir3::TextFont::level);
    if (!model.map_name.empty()) {
        // 0x4390E0 composites this backing before the four text shadows.
        legacy_rect(rect(Layout::coordinates),Color(100.f/255,75.f/255,50.f/255,100.f/255));
        const auto label=model.map_name+" : ["+std::to_string(model.map_x)+","+std::to_string(model.map_y)+"]";
        for (auto offset:Layout::coordinate_shadows) { auto r=Layout::coordinates;r[0]+=offset[0];r[1]+=offset[1];text(label,r,Layout::coordinate_shadow_color); }
        text(label,Layout::coordinates,Layout::text_color(0));
    }
    for (int side=0;side<2;++side) {
        const auto values=model.stat(side);
        sprite(values[0],Layout::image_position(side==0?Layout::Image::left_stat:Layout::Image::right_stat));
        text(std::to_string(values[1])+"-"+std::to_string(values[2]),Layout::stat(side),Layout::text_color(2+side));
    }
    buttons();
    hud_tail->queue_redraw();
    paint_canvas=this;
}

void Mir3HudView::draw_hud_tail() {
    using Layout=mir3::HudLayout;
    paint_canvas=hud_tail;
    sprite(mir3::ChatScrollLayout::frame,{mir3::ChatScrollLayout::x,mir3::chat_thumb_y(chat_history.first(model.layout.expanded),int(chat_history.lines(model.layout.expanded).size()),model.layout.expanded)});
    buttons();
    // 0x43877F..0x4387B6 draws the upload entry in the HUD pass.
    if (!model.layout.expanded && rect(Layout::portrait_hover).has_point(inventory_cursor))
        sprite(Layout::portrait_entry_frame(portrait_entry_pressed),Layout::portrait_entry_position);
    for (const auto panel:panel_order) {
        if (!panel_state(panel).visible) continue;
        switch (panel) {
        case Panel::group: draw_group(); break;
        case Panel::skills: draw_skills(); draw_skill_key_menu(); draw_skill_description(); break;
        case Panel::status: draw_status(); break;
        case Panel::inventory: draw_inventory(); break;
        case Panel::guild: draw_guild(); break;
        case Panel::options: draw_options(); break;
        case Panel::portrait: draw_portrait_picker(); break;
        case Panel::mount: draw_mount(); break;
        case Panel::quest: draw_quest(); break;
        case Panel::trade: draw_trade(); break;
        case Panel::quick: draw_quick(); break;
        }
    }
    draw_minimap();
    // 0x4387E5 draws the minimap before the target HP text and bar.
    draw_target_vitals();
    draw_skill_shortcuts();
    draw_npc_dialogue();
    draw_shop();
    if (guild_editor.visible) draw_guild_editor();
    // 0x438CBD..0x438D9F draws the monster information backing even when its fields are absent.
    if (!model.layout.expanded && target_focus.displayed() && !target_is_human &&
        !target_special_shape && target_frame>=0) {
        const Rect2 information(615,487,105,85);
        legacy_rect(information,Color(50.f/255,50.f/255,50.f/255,230.f/255));
        paint_canvas->draw_rect(information.grow(-0.5),Color(120.f/255,120.f/255,120.f/255),false);
        const auto attributes=target_attributes_by_actor.find(target_focus.displayed());
        if (attributes!=target_attributes_by_actor.end() && attributes->second.visible()) {
            const auto& target_attributes=attributes->second;
            const auto draw_icon=[&](int frame,Layout::Point at) {
                const auto icon=portrait_texture(portraits,portrait_frames,frame);
                if (icon.is_valid()) paint_canvas->draw_texture(icon,point(at));
            };
            draw_icon(target_attributes.category==0?632:631,{620,490});
            if (target_attributes.element) draw_icon(599+target_attributes.element,{641,490});
            draw_icon(620+target_attributes.icon,{target_attributes.element?662:641,490});
            int slot=0;
            for (int index:target_attributes.order()) {
                const int value=target_attributes.modifiers[index];
                if (!value) break;
                const int x=620+32*(slot%3), y=510+20*(slot/3);
                draw_icon(610+index,{x,y});
                text((value>0?" +":" ")+std::to_string(value)+"%",{x+15,y+5,40,18},
                    0xe6e6e6,mir3::TextFont::normal,false);
                ++slot;
            }
        }
    }
    draw_chat_shortcuts();
    if (group_request) draw_group_dialog();
    button_hint();
    const auto& held=trade.visible && trade_return_slot>=0?trade_items[0][trade_return_slot]:bag.held;
    if ((held || held_gold) && Rect2(0,0,800,600).has_point(inventory_cursor)) {
        const auto icon=item_frame(held_gold?124:held.icon);
        if (icon.is_valid()) paint_canvas->draw_texture(icon,(inventory_cursor-icon->get_size()*0.5).floor());
    }
    paint_canvas=this;
}

void Mir3HudView::set_hud_decoration(Vector4i state) {
    if (!hud_decoration || state.x<0 || state.x>255) return;
    const int id=mir3::HudLayout::decoration_frame(static_cast<std::uint8_t>(state.x));
    const Ref<ShaderMaterial> dye=hud_decoration->get_material();
    if (id!=hud_decoration_frame) {
        hud_decoration_frame=id;
        const auto decoded=archive.frame(id);
        if (decoded) {
            PackedByteArray regions;
            regions.resize(decoded->pixels.regions.size());
            std::copy(decoded->pixels.regions.begin(),decoded->pixels.regions.end(),regions.ptrw());
            dye->set_shader_parameter("dye_regions",ImageTexture::create_from_image(Image::create_from_data(
                decoded->width,decoded->height,false,Image::FORMAT_L8,regions)));
        }
        hud_decoration->set_texture(frame(id));
    }
    const auto color=mir3::Rgb565::pack(static_cast<std::uint8_t>(state.y),static_cast<std::uint8_t>(state.z),
        static_cast<std::uint8_t>(state.w));
    dye->set_shader_parameter("first_dye",Vector3(color>>11,(color>>5)&63,color&31));
    const bool visible=hud_decoration->get_texture().is_valid();
    hud_copy->set_visible(visible);
    hud_decoration->set_visible(visible);
    queue_redraw();
}

void Mir3HudView::post_chat(String value,int64_t foreground,int64_t background) {
    const auto utf8=value.utf8();
    chat_history.post({utf8.get_data(),static_cast<std::size_t>(utf8.length())},static_cast<unsigned>(foreground),static_cast<unsigned>(background));
    queue_redraw();
}
void Mir3HudView::chat_lines() {
    const bool expanded=model.layout.expanded;
    const auto& lines=chat_history.lines(expanded);
    const int first=chat_history.first(expanded);
    auto bounds=mir3::HudLayout::chat_line(expanded);
    for (int i=first;i<std::min(int(lines.size()),first+mir3::HudLayout::chat_rows(expanded));++i) {
        const auto& line=lines[i];
        if (line.background) {
            const auto measured=mir3::raster_text(line.text,800,24,false,mir3::TextFont::normal);
            const auto c=line.background;
            paint_canvas->draw_rect(Rect2(bounds[0],bounds[1],measured.text_width,measured.text_height),Color(float(c&255)/255,float((c>>8)&255)/255,float((c>>16)&255)/255));
        }
        text(line.text,bounds,line.foreground,mir3::TextFont::normal,false);
        bounds[1]+=bounds[3];
    }
}
bool Mir3HudView::scroll_input(int kind,Vector2 position) {
    const auto thumb=frame(mir3::ChatScrollLayout::frame);
    if (thumb.is_null()) return false;
    const auto now=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    const bool consumed=chat_history.input(kind,int(position.x),int(position.y),model.layout.expanded,thumb->get_width(),thumb->get_height(),static_cast<std::uint32_t>(now));
    if (consumed) { get_viewport()->set_input_as_handled();queue_redraw(); }
    return consumed;
}
std::string Mir3HudView::message(int id) const { return mir3::ansi_to_utf8(messages.get(id)); }
void Mir3HudView::append_chat_log(int opcode,std::string_view ansi) {
    if(chat_log_path.is_empty()) return;
    const auto contents=mir3::ChatLog::payload(opcode,chat_submission.save_chat,ansi);
    if(contents.empty()) return;
    std::error_code error;
    std::filesystem::create_directories(std::filesystem::u8path(chat_log_path.get_base_dir().utf8().get_data()),error);
    if(error) return;
    const auto file=FileAccess::open(chat_log_path,FileAccess::file_exists(chat_log_path)?FileAccess::READ_WRITE:FileAccess::WRITE);
    if(file.is_null()) return; // 原版打不开日志时仍继续显示收到的消息。
    file->seek_end();
    PackedByteArray bytes;bytes.resize(contents.size());std::copy(contents.begin(),contents.end(),bytes.ptrw());
    file->store_buffer(bytes);file->flush();
}

bool Mir3HudView::chat_shortcut_input(const Ref<InputEvent>& event) {
    const Ref<InputEventKey> key=event;if(key.is_null()) return false;
    const auto code=key->get_keycode()!=KEY_NONE?key->get_keycode():key->get_physical_keycode();
    chat_submission.control_down=code==KEY_CTRL?key->is_pressed():key->is_ctrl_pressed();
    chat_submission.alt_down=code==KEY_ALT?key->is_pressed():key->is_alt_pressed();
    queue_redraw();
    if(!key->is_pressed() || !chat_input || !chat_input->has_focus() || group_request || guild_editor.visible ||
       (!chat_submission.control_down && !chat_submission.alt_down)) return false;
    const bool phrase=!chat_submission.control_down;
    const bool relative=code==KEY_UP || code==KEY_DOWN;
    int index=-1;
    if(code>=KEY_1 && code<=KEY_9) index=int(code-KEY_1);
    else if(code==KEY_0) index=9;
    if(!relative && index<0) return false;
    const auto value=chat_submission.select(phrase,relative?(code==KEY_UP?-1:1):index,relative);
    if(value) chat_input->focus_chat(String::utf8(value->c_str()),true);
    get_viewport()->set_input_as_handled();return true;
}

void Mir3HudView::draw_chat_shortcuts() {
    if(!chat_input || !chat_input->has_focus() || group_request || guild_editor.visible) return;
    const int top=model.layout.expanded?365:405;
    for(bool phrase:{false,true}) {
        const auto& values=phrase?chat_submission.phrases:chat_submission.contacts;
        if(values.empty() || !(phrase?chat_submission.alt_down:chat_submission.control_down)) continue;
        const int width=phrase && !model.layout.expanded?100:99;
        const Rect2 bounds(183,top,width,165);
        legacy_rect(bounds,phrase?Color(150.f/255,150.f/255,50.f/255,180.f/255):Color(200.f/255,100.f/255,0,180.f/255));
        paint_canvas->draw_rect(bounds.grow(-0.5),Color(0,0,0),false);
        for(int i=0;i<int(values.size());++i) {
            if(i==(phrase?chat_submission.phrase_selected:chat_submission.contact_selected))
                legacy_rect(Rect2(184,top+2+16*i,width-2,16),phrase?Color(50.f/255,50.f/255,200.f/255,120.f/255):Color(1,0,0,120.f/255));
            auto caption=values[i];
            if(phrase && mir3::raster_text(caption,1,1,false,mir3::TextFont::normal).text_width>358) {
                auto ansi=mir3::utf8_to_ansi(caption);ansi=ansi.substr(0,58)+"...";caption=mir3::ansi_to_utf8(ansi);
            }
            // 原版TextOut不裁剪到99像素面板，长短语可以向右延伸。
            text(caption,{187,top+4+16*i,600,16},657930,mir3::TextFont::normal,false);
            text(caption,{188,top+5+16*i,600,16},0xffffff,mir3::TextFont::normal,false);
        }
    }
}

String Mir3HudView::chat_history_file(bool phrases) const {
    const auto base=item_layout_file();
    if(base.is_empty()) return String();
    const auto server=String::utf8(mir3::ansi_to_utf8(login_flow.selected_server).c_str());
    for(const auto token:{"/","\\",":","*","?","\"","<",">","|"}) if(server.contains(token)) return String();
    return base.get_basename()+String("(")+server+String(phrases?").mlt":").slt");
}

void Mir3HudView::save_chat_shortcuts(bool phrases) {
    const auto path=chat_history_file(phrases);if(path.is_empty()) return;
    std::error_code error;
    std::filesystem::create_directories(std::filesystem::u8path(item_layout_directory.utf8().get_data()),error);
    if(error) return;
    const auto file=FileAccess::open(path,FileAccess::WRITE);if(file.is_null()) return;
    std::string contents;
    for(const auto& value:phrases?chat_submission.phrases:chat_submission.contacts) contents+=mir3::utf8_to_ansi(value)+"/";
    PackedByteArray bytes;bytes.resize(contents.size());std::copy(contents.begin(),contents.end(),bytes.ptrw());file->store_buffer(bytes);
}

void Mir3HudView::load_chat_shortcuts() {
    const auto key=chat_history_file(false);if(key.is_empty() || key==chat_storage_key) return;
    chat_storage_key=key;chat_submission.clear_history();
    for(bool phrases:{false,true}) {
        const auto path=chat_history_file(phrases);if(!FileAccess::file_exists(path)) continue;
        const auto file=FileAccess::open(path,FileAccess::READ);if(file.is_null() || file->get_length()>10240) continue;
        const auto bytes=file->get_buffer(file->get_length());
        const std::string contents(reinterpret_cast<const char*>(bytes.ptr()),bytes.size());
        auto& values=phrases?chat_submission.phrases:chat_submission.contacts;
        std::size_t start=0;
        while(values.size()<10) {
            const auto end=contents.find('/',start);if(end==std::string::npos) break;
            if(end-start>1023) break;
            values.push_back(mir3::ansi_to_utf8(std::string_view(contents).substr(start,end-start)));start=end+1;
        }
    }
}

void Mir3HudView::chat_requested(String value) {
    if(login_visible || character_selection.visible || server_groups.visible) return;
    emit_signal("chat_requested",value);
    const auto utf8=value.utf8();
    const auto action=chat_submission.submit({utf8.get_data(),std::size_t(utf8.length())},guild_manager,
        message(1425),message(1418),message(1428),message(1419));
    if(action.dialog) {
        open_group_dialog(action.dialog==1426?mir3::protocol::CM_GUILDADDMEMBER:1427);return;
    }
    update_weather_layers();
    if(action.contacts_changed) save_chat_shortcuts(false);
    if(action.phrases_changed) save_chat_shortcuts(true);
    if(action.private_echo) post_chat(value,mir3::chat_palette(5),mir3::chat_palette(0));
    if(action.delay) map_input_delay.mount_command();
    if(action.send) send_client_packet(mir3::protocol::CM_SAY,0,value);
}
void Mir3HudView::send_chat_command(String value) {
    // 0x47CA80：界面命令直接发送；不经过0x427400的输入框本地指令分支。
    emit_signal("chat_requested",value);
    send_client_packet(mir3::protocol::CM_SAY,0,value);
}
bool Mir3HudView::update_chat_rejection(int channel,bool rejected) {
    if (channel<0 || channel>=3) return false;
    // State producer is external. The recovered click path reads these flags;
    // it does not toggle them or infer an acknowledgement from the command.
    rejected_chat[channel]=rejected;
    return true;
}
void Mir3HudView::tooltip(int kind) {
    std::string caption;
    if (kind<=2) caption=message(kind==1?2623:2622)+std::to_string(kind==1?model.mp:model.hp)+"/"+std::to_string(kind==1?model.max_mp:model.max_hp);
    else if (kind==3) caption=message(2625)+std::to_string(model.weight)+"/"+std::to_string(model.max_weight);
    else {
        const float ratio=model.next_experience?static_cast<float>(std::min(1.0,double(model.experience)/model.next_experience)):0.f;
        char amount[64]{};std::snprintf(amount,sizeof(amount),"%.2f%%",double(ratio)*100);
        caption=message(2624)+amount;
    }
    const auto measured=mir3::raster_text(caption,800,24,false,mir3::TextFont::normal);
    const auto bounds=mir3::HudLayout::tooltip_rect(kind,{measured.text_width,measured.text_height});
    const Color backgrounds[]={Color(120.f/255,60.f/255,60.f/255,150.f/255),Color(60.f/255,60.f/255,120.f/255,150.f/255),Color(120.f/255,60.f/255,60.f/255,150.f/255),Color(100.f/255,100.f/255,0,150.f/255),Color(80.f/255,60.f/255,40.f/255,150.f/255)};
    legacy_rect(rect(bounds),backgrounds[kind]);text(caption,bounds,0xffffff);
    const Color borders[]={Color("a05050"),Color("5050a0"),Color("a05050"),Color("787800"),Color("644b32")};
    paint_canvas->draw_rect(rect(bounds),borders[kind],false);
}
void Mir3HudView::button_hint() {
    button_hint_animating=false;
    if (group_request || (guild_editor.visible && rect(guild_editor.rect).has_point(inventory_cursor))) return;
    int hint=-1;
    Vector2 anchor;
    bool panel_hit=shop_actor && Rect2(shop_window_left(),shop_window_top(),shop_width(),shop_window_height()).has_point(inventory_cursor);
    if (!panel_hit && npc_dialogue_actor && rect(npc_dialogue_bounds()).has_point(inventory_cursor)) {
        panel_hit=true;
        // 0x45FDD0 configures the close button with CMList 1052.
        const auto icon=frame(1221);
        const Rect2 hit(Vector2(346,npc_dialogue_bounds()[3]-42),icon.is_valid()?icon->get_size():Vector2());
        if (!npc_dialogue_close_pressed && hit.has_point(inventory_cursor)) {
            hint=1052;
            anchor=hit.get_end();
        }
    }
    if (!panel_hit && minimap.visible) {
        panel_hit=rect(minimap.bounds()).has_point(inventory_cursor);
        for (int index=1;index>=0;--index) {
            const auto icon=frame(index==0?(minimap.translucent?1482:1483):(minimap.large?1485:1484));
            if (icon.is_null()) continue;
            const Rect2 hit(point(minimap.button_position(index)),icon->get_size());
            if (!hit.has_point(inventory_cursor)) continue;
            hint=minimap.button_hint(index);
            anchor=hit.get_end();
            panel_hit=true;
            break;
        }
    }
    for (auto it=panel_order.rbegin();!panel_hit && it!=panel_order.rend();++it) {
        const auto& panel=panel_state(*it);
        if (!panel.visible || !rect(panel.rect).has_point(inventory_cursor)) continue;
        panel_hit=true;
        const auto resolve=[&](const auto& spec,int message_id) {
            if (message_id<0) return;
            const auto icon=frame(spec[0]);
            if (icon.is_null()) return;
            const Rect2 hit(Vector2(panel.rect[0]+spec[1],panel.rect[1]+spec[2]),icon->get_size());
            if (!hit.has_point(inventory_cursor)) return;
            hint=message_id;
            anchor=hit.get_end();
        };
        if (*it==Panel::mount) {
            for (int index=0;index<int(mount.buttons.size());++index)
                if (mount.pressed!=index) resolve(mount.buttons[index],mount.buttons[index][3]);
        } else if (*it==Panel::trade) {
            if (!trade_confirm_pressed)
                resolve(std::array{1394,226,279},trade_confirmed?1303:1300);
        } else if (*it==Panel::quest) {
            if (quest.pressed!=0) resolve(quest.buttons[0],2200);
            if (quest.pressed!=1) resolve(quest.buttons[1],1052);
        } else if (*it==Panel::inventory) {
            if (!inventory.close_pressed) resolve(std::array{1221,242,423},1052);
        } else if (*it==Panel::status) {
            const auto [left,top]=mir3::StatusLayout::close;
            if (!status_close_pressed) resolve(std::array{1221,left,top},1052);
        } else if (*it==Panel::group) {
            // 0x431B80 assigns captions to the close and roster buttons.
            for (int i=0;i<int(group_button_frames.size());++i) {
                const auto [left,top]=group_button_offsets[i];
                if (group_pressed!=i) resolve(std::array{group_button_frames[i],left,top},group_button_hints[i]);
            }
        } else if (*it==Panel::guild) {
            for (int i=0;i<9;++i) resolve(mir3::GuildLayout::buttons[i],guild_layout.tooltip(i));
        } else if (*it==Panel::skills) {
            for (int i=0;i<9;++i)
                if (skills.pressed!=i) resolve(mir3::SkillPanel::button(i),i==0?1052:5049+i);
        } else if (*it==Panel::portrait) {
            constexpr std::array messages{1052,4700,4701};
            for (int i=0;i<3;++i)
                if (portrait_pressed!=i) resolve(mir3::PortraitPicker::buttons[i],messages[i]);
        }
        break;
    }
    if (!panel_hit && !model.layout.expanded && rect(mir3::HudLayout::portrait_hover).has_point(inventory_cursor)) {
        const auto entry=frame(mir3::HudLayout::portrait_entry_frame(false));
        if (entry.is_valid()) {
            const Rect2 hit(point(mir3::HudLayout::portrait_entry_position),entry->get_size());
            if (hit.has_point(inventory_cursor)) {
                panel_hit=true;
                if (!portrait_entry_pressed) {
                    hint=4715; // 0x4362CE.
                    anchor=hit.get_end();
                }
            }
        }
    }
    if (!panel_hit && model.hovered>=0 && model.hovered!=model.pressed) {
        hint=chat_input && (model.hovered==18 || model.hovered==19)?chat_input->mode.tooltip(model.hovered):
            model.hovered>=20 && model.hovered<=22?reception_buttons.hints[model.hovered-20]:2600+model.hovered;
        const auto hit=model.button_rect(model.hovered);
        anchor=Vector2(hit[0]+hit[2],hit[1]+hit[3]);
    }
    if (hint<0) return;
    const auto caption=message(hint);
    if (caption.empty()) return;
    const auto measured=mir3::raster_text(caption,800,24,false,mir3::TextFont::normal);
    const auto bounds=button_tooltip.draw(caption,{measured.text_width,measured.text_height},{int(anchor.x),int(anchor.y)});

    button_hint_animating=button_tooltip.expanding;
    paint_canvas->draw_rect(rect(bounds),Color(0,0,0));
    if (bounds[2]>2 && bounds[3]>2) paint_canvas->draw_rect(rect(bounds).grow(-1),Color(1,1,150.f/255));
    text(caption,bounds,0);
}
void Mir3HudView::_process(double delta) {
    if (server_groups.visible) return;
    poll_connection(delta);
    if (connection_retry_at) return;
    if (login_movie_purpose!=LoginMoviePurpose::none) { advance_login_movie(delta);return; }
    if (login_visible) {
        login_fraction_ms+=delta*1000;const auto ms=std::uint32_t(login_fraction_ms);login_fraction_ms-=ms;
        advance_login_audio(ms);
        if (login_flow.phase==mir3::LoginFlow::Phase::credentials) login_announcement.advance(ms);
        if (login_flow.advance(ms)) enter_character_selection();
        else if (login_flow.phase==mir3::LoginFlow::Phase::leaving) exit_layer->queue_redraw();
        queue_redraw();return;
    }
    if (character_selection.visible) {
        selection_fraction_ms+=delta*1000;const auto ms=std::uint32_t(selection_fraction_ms);selection_fraction_ms-=ms;
        advance_login_audio(ms);
        selection_elapsed=std::min(500u,selection_elapsed+ms);
        if (selection_pending && selection_elapsed>=500) {
            if (selection_delete.visible) finish_selection_delete(false);
            character_selection=std::move(*selection_pending);selection_pending.reset();
            selection_notice.show(selection_pending_notice?selection_pending_notice:(character_selection.count==0?219:0));selection_pending_notice=0;selection_bounds={};
        }
        if (character_creation.visible) {
            for (int i=0;i<2;++i) {
                const auto& role=character_creation.roles[i];
                if (std::uint16_t(role.elapsed+ms)>120) if (const auto source=selection_source(role.frame+1))
                    creation_bounds[i]=Rect2(role.x+source->offset_x,role.y+source->offset_y,source->width,source->height);
            }
            const auto& selected=character_creation.roles[character_creation.selected];
            if (std::uint16_t(selected.elapsed+ms)>120 && selected.frame==selected.first_frame())
                play_login_sound(mir3::LoginAudio::voice(true,selected.profession,selected.gender));
            character_creation.advance(ms);queue_redraw();return;
        }
        if (!character_selection.connecting) {
            for (int i=character_selection.page*2;i<std::min(character_selection.count,character_selection.page*2+2);++i) {
                const auto& role=character_selection.roles[i];
                if (std::uint16_t(role.elapsed+ms)>120) {
                    // 原版在推进帧后、动作末帧转换前更新矩形；选择动作本身不立即更新。
                    if (const auto source=selection_source(role.frame+1))
                        selection_bounds[i]=Rect2(role.x+source->offset_x,role.y+source->offset_y,source->width,source->height);
                }
            }
        }
        if (!character_selection.connecting) for (int i=character_selection.page*2;i<std::min(character_selection.count,character_selection.page*2+2);++i) {
            const auto& role=character_selection.roles[i];
            if (role.action==1 && role.frame==role.first_frame() && std::uint16_t(role.elapsed+ms)>120)
                play_login_sound(mir3::LoginAudio::voice(false,role.profession,role.gender));
        }
        character_selection.advance(ms);queue_redraw();return;
    }
    if (exit_transition.active) {
        exit_fraction_ms+=delta*1000;
        const auto elapsed=static_cast<std::uint32_t>(exit_fraction_ms);
        exit_fraction_ms-=elapsed;
        if (exit_transition.advance(elapsed)) { enter_character_selection();return; }
        exit_layer->queue_redraw();
    }
    advance_level_effects(delta*1000);
    // 原版用整数时钟差；不能逐帧丢弃亚毫秒，否则高于1000FPS时动作停止。
    actor_frame_remainder_ms+=delta*1000;
    const auto actor_tick_ms=std::uint32_t(actor_frame_remainder_ms);
    actor_frame_remainder_ms-=actor_tick_ms;
    action_prediction.advance(actor_tick_ms);
    const bool advancing_action=!player_action.finished();
    const int previous_action_frame=player_action.frame();
    if (walk_frame>=0) { player_action={};player_harvest_impact=false; }
    player_action.advance(actor_tick_ms);
    if (player_life.dying && player_action.frame()>=9) {
        player_life.dying=false;model.hp=0;
        if (scene_health.contains(0)) scene_health.at(0).current=0;
        queue_redraw();
    }
    if (player_life.dead && !player_life.dying && model.hp) {
        model.hp=0;
        if (scene_health.contains(0)) scene_health.at(0).current=0;
        queue_redraw();
    }
    if (advancing_action && player_action.frame()!=previous_action_frame) {
        if (player_action.frame()==1 && player_melee_sound &&
            ((player_action.action()>=9 && player_action.action()<=14) || player_action.action()==16 || player_action.action()==17)) {
            play_spell_sound(player_melee_sound,player_cell);player_melee_sound=0;
        } else if (const int sound=mir3::HumanActions::frame_sound(player_action.action(),player_action.frame(),player_weapon,local_hit_weapon,player_clothing,player_type))
            play_actor_sound(0,sound,player_cell);
        if (player_harvest_impact && player_action.frame()==3) {
            player_harvest_impact=false;
            show_harvest_impact(player_cell);
        }
    }
    map_input_delay.advance(delta*1000);
    process_map_changes();
    if(terrain_ready) weather.advance(int(delta*1000),chat_submission.snow);
    update_weather_layers();

    apply_login_actor();
    process_local_actor_message();
    advance_local_action();
    scene_movement_ms+=delta*1000;
    const bool scene_movement_tick=scene_movement_ms>95;
    if (scene_movement_tick) scene_movement_ms=0;
    advance_monsters(delta,scene_movement_tick);
    if (scene_ambient.fading) { scene_ambient.advance(delta*1000);queue_redraw(); }
    // Default object messages are processed outside each actor's animation queue.
    apply_scene_removals();
    if (locked_spell_actor) {
        // 415F50 / 453320 center the 24x24 list on the logical player cell.
        const Rect2i actor_list(player_cell-Vector2i(12,12),Vector2i(24,24));
        const auto monster=scene_chickens.find(locked_spell_actor);
        const auto human=scene_humans.find(locked_spell_actor);
        const bool present=(monster!=scene_chickens.end() && actor_list.has_point(monster->second.cell)) ||
            (human!=scene_humans.end() && actor_list.has_point(human->second.cell));
        if (!present) {
            locked_spell_actor=0;
            queue_redraw();
        }
    }

    for (auto& [actor,health]:scene_health) {
        const bool registered=health.registered;
        health.advance(delta*1000);
        if (registered && !health.registered) queue_redraw();
    }
    for (auto it=scene_speech.begin();it!=scene_speech.end();) {
        it->second.advance(delta*1000);
        if (it->second.lines.empty()) it=scene_speech.erase(it);
        else ++it;
        queue_redraw();
    }
    for (auto& [actor,human]:scene_humans) {
        if (human.arrival_ms>=0) {
            human.arrival_ms+=delta*1000;
            if (human.arrival_ms>=human.appearance_duration_ms) human.arrival_ms=-1;
            queue_redraw();
        }
        human.life.advance(delta*1000);
        if (human.combat.advance(delta*1000)) human.idle.reset();
        if (human.life.dying) {
            const int previous=human.action.frame();
            human.action.advance(actor_tick_ms,human.action_flags);
            if (previous==0 && human.action.frame()==1) play_actor_sound(actor,144+human.type,human.cell);
            if (human.action.frame()>=9) human.life.dying=false;
            queue_redraw();
            continue;
        }
        if (human.life.dead) {
            dispatch_scene_human_message(actor);
            continue;
        }

        if (human.movement_frame<0) {
            if (!human.action.finished()) {
                const int previous=human.action.frame();
                human.action.advance(actor_tick_ms,human.action_flags);
                if(human.harvest && human.action.frame()==3) {
                    human.harvest=false;
                    show_harvest_impact(human.cell);
                }
                if (previous!=human.action.frame()) {
                    const bool melee_action=(human.action.action()>=9 && human.action.action()<=14) || human.action.action()==16 || human.action.action()==17;
                    const int melee_sound=human.spell.weapon && melee_action && human.action.frame()==1?
                        mir3::MeleeSpells::frame_sound(human.spell.skill,human.type):0;
                    if (melee_sound) play_spell_sound(melee_sound,human.cell);
                    else play_actor_sound(actor,mir3::HumanActions::frame_sound(human.action.action(),human.action.frame(),
                        human.weapon,human.hit_weapon,human.clothing,human.type),human.cell);
                }
                queue_redraw();
                if (!human.action.finished()) continue;
                human.idle.reset();
                dispatch_scene_human_message(actor);
            } else if (!dispatch_scene_human_message(actor)) human.idle.advance(delta*1000,human.combat.idle_action(human.mount_shape!=0)==7?3:4);
            continue;
        }
        if (!scene_movement_tick) continue;
        if (human.action_flags&0x08000000u) {
            if (++human.slow_ticks>100000) human.slow_ticks=1;
            if (human.slow_ticks&1) continue;
        }
        if (human.bump) {
            human.movement_frame+=2;
            if (human.movement_frame>=4) {
                human.cell=human.step_origin;
                human.movement_frame=-1;
                human.action.start(7);
                human.idle.reset();
                human.bump=false;
            } else if (human.movement_frame>=human.movement_commit_frame()) human.cell=human.step_destination;
            queue_redraw();
            continue;
        }
        human.movement_frame+=human.backstep?2:1;
        const int movement_action=human.mount_shape?(human.step_distance>1?31:30):(human.step_distance>1?22:21);
        if (!human.backstep) {
            play_actor_sound(actor,mir3::HumanActions::movement_sound(movement_action,human.movement_frame),human.cell);
            human.movement_frame+=int(human.catching_up);
        }
        if (human.movement_frame>=human.movement_commit_frame()) human.cell=human.step_destination;
        if (human.movement_frame>=6) {
            human.movement_frame=-1;
            human.idle.reset();
        }
        queue_redraw();
    }
    if (terrain_ready) {
        if (walk_frame>=0 || advancing_action) player_idle.reset();
        else player_idle.advance(delta*1000);
    }
    if (portrait_map_visible && !model.layout.expanded) {
        portrait_map_blink_ms+=delta*1000;
        if (portrait_map_blink_ms>800) portrait_map_blink_ms=0;
        queue_redraw();
    }
    if (npc_dialogue_actor && npc_dialogue_first<npc_dialogue_scroll_limit) {
        // 0x460F43 alternates the down-arrow frame after 16 draw updates.
        if (npc_dialogue_arrow_frame==16) {
            npc_dialogue_arrow_frame=0;
            npc_dialogue_down_lit=!npc_dialogue_down_lit;
            queue_redraw();
        }
        ++npc_dialogue_arrow_frame;
    }
    if (chat_input && chat_input->has_focus()) map_pointer_button=0;
    advance_map_step(delta);
    // 42201F–422049 and 42281B prune the registered drops with a half-open player-centered RECT.
    if (std::erase_if(ground_items.items,[this](const auto& item) {
            const auto offset=Vector2i(item.x,item.y)-player_cell;
            return offset.x<-24 || offset.x>=24 || offset.y<-24 || offset.y>=24;
        })) queue_redraw();
    const auto sparkle_tick=std::uint32_t(GetTickCount());
    for (auto& item:ground_items.items) {
        if (sparkle_tick-item.sparkle_started_tick<=std::uint32_t(item.sparkle_delay_ms)) continue;
        if (item.sparkle_frame<0) item.sparkle_frame=0;
        item.sparkle_elapsed_ms+=delta*1000;
        if (item.sparkle_elapsed_ms>100) {
            item.sparkle_elapsed_ms=0;
            if (++item.sparkle_frame==int(ground_sparkles.size())) {
                item.sparkle_started_tick=sparkle_tick;
                item.sparkle_frame=-1;
            }
        }
        ground_sparkle_layer->queue_redraw();
    }
    // 42495A–424994 checks the current coordinates every frame, including server corrections.
    if (npc_dialogue_actor && npc_dialogue_origin.x && npc_dialogue_origin.y) {
        const auto distance=player_cell-npc_dialogue_origin;
        if (std::max(std::abs(distance.x),std::abs(distance.y))>5) close_npc_dialogue();
    }
    sync_monster_overlays();
    for (auto& [actor,npc]:scene_npcs) {
        if (npc.arrival_ms>=0) {
            npc.arrival_ms+=delta*1000;
            if (npc.arrival_ms>=npc.appearance_duration_ms) npc.arrival_ms=-1;
            queue_redraw();
        }
        if (npc.standing.size()<=1) continue;
        npc.standing_elapsed+=delta*1000;
        // 40E86F advances once and discards the elapsed interval.
        if (npc.standing_elapsed>npc.standing_interval) {
            npc.standing_elapsed=0;
            if (++npc.standing_frame==npc.standing.size()) {
                npc.standing_frame=0;
                if (!npc.idle_after_refresh.empty()) {
                    npc.standing=std::move(npc.idle_after_refresh);
                    npc.standing_interval=npc.idle_interval;
                }
            }
            queue_redraw();
        }
    }
    if(!map_events.empty() || !event_marks.empty() || spell_particles.count()) advance_map_events(delta*1000);
    if(!ground_events.empty()) {
        for(auto& [id,event]:ground_events) event.age+=delta*1000;
        std::erase_if(ground_events,[](const auto& entry) {
            return entry.second.hidden || entry.second.age>entry.second.lifetime;
        });
        queue_redraw();
    }
    if (map_pointer_button==MOUSE_BUTTON_RIGHT && walk_frame<0 && map_route.empty()) {
        step_toward_pointer(model.hp>=10);
    } else if (map_pointer_button==MOUSE_BUTTON_LEFT) {
        map_pointer_seconds+=delta;
        if (map_pointer_seconds>0.3 && map_route.empty()) {
            update_scene_target(inventory_cursor);
            step_toward_pointer(false,map_shift_pressed,chat_submission.alt_down);
        }
    }
    if (!hovered_target && target_focus.displayed()) {
        target_focus.update(0,static_cast<std::uint32_t>(ticks_msec()));
        if (!target_focus.displayed()) queue_redraw();
    }
    // Original button tooltip grows once per draw, not once per mouse event.
    if (animated_map_objects || minimap.visible || model.hovered>=0 || button_hint_animating) queue_redraw();
}
void Mir3HudView::legacy_rect(Rect2 bounds,Color color) {
    const auto a=bounds.position,b=bounds.get_end();
    PackedVector2Array vertices,uv;
    vertices.push_back(a);vertices.push_back(Vector2(b.x,a.y));vertices.push_back(b);vertices.push_back(Vector2(a.x,b.y));
    for (int i=0;i<4;++i) uv.push_back(Vector2(-2,-2));
    PackedColorArray colors;colors.push_back(color);
    paint_canvas->draw_polygon(vertices,colors,uv);
}
void Mir3HudView::_input(const Ref<InputEvent>& event) {
    if (const Ref<InputEventKey> key=event;key.is_valid() && key->get_keycode()==KEY_SHIFT)
        map_shift_pressed=key->is_pressed();
    if (server_groups.visible) { server_groups_input(event);get_viewport()->set_input_as_handled();return; }
    if (connection_retry_at || login_movie_purpose!=LoginMoviePurpose::none) { get_viewport()->set_input_as_handled();return; }
    if (login_visible) {
        login_input(event);
        if (login_flow.phase==mir3::LoginFlow::Phase::credentials && !login_flow.notice.message && login_flow.editor_visible) {
            const Ref<InputEventKey> key=event;
            if (key.is_valid() && key->get_keycode()!=KEY_ENTER && key->get_keycode()!=KEY_TAB && key->get_keycode()!=KEY_ESCAPE) return;
            const Ref<InputEventMouseButton> mouse=event;
            if (mouse.is_valid() && Rect2(chat_input->get_position(),chat_input->get_size()).has_point(mouse->get_position())) return;
        }
        get_viewport()->set_input_as_handled();return;
    }
    if (character_selection.visible) {
        selection_input(event);
        if (character_creation.visible && !selection_notice.message && chat_input->has_focus()) {
            const Ref<InputEventKey> name_key=event;
            if (name_key.is_valid() && name_key->get_keycode()!=KEY_ENTER && name_key->get_keycode()!=KEY_ESCAPE) return;
            const Ref<InputEventMouseButton> name_mouse=event;
            if (name_mouse.is_valid() && Rect2(Vector2(288,405),Vector2(75,13)).has_point(name_mouse->get_position())) return;
        }
        if (selection_delete.visible && chat_input->has_focus()) {
            const Ref<InputEventKey> input_key=event;
            if (input_key.is_valid()) {
                const auto code=input_key->get_keycode();
                if (code!=KEY_ENTER && code!=KEY_SPACE && code!=KEY_TAB && code!=KEY_ESCAPE) return;
            }
            const Ref<InputEventMouseButton> input_mouse=event;
            if (input_mouse.is_valid() && Rect2(Vector2(164,338),Vector2(309,15)).has_point(input_mouse->get_position())) return;
        }
        get_viewport()->set_input_as_handled();return;
    }
    const Ref<InputEventMouseButton> button=event;
    const bool left_released=button.is_valid() && button->get_button_index()==MOUSE_BUTTON_LEFT && !button->is_pressed();
    if (left_released && portrait_entry_pressed) {
        portrait_entry_pressed=false;
        queue_redraw();
    }
    if (merchant_click_captured) {
        const Ref<InputEventMouseButton> release=event;
        if (release.is_valid() && release->get_button_index()==MOUSE_BUTTON_LEFT && !release->is_pressed()) {
            merchant_click_captured=false;
            get_viewport()->set_input_as_handled();
            return;
        }
    }
    if (map_pointer_button) {
        const Ref<InputEventMouseButton> release=event;
        if (release.is_valid() && release->get_button_index()==map_pointer_button && !release->is_pressed()) {
            map_pointer_button=0;
            get_viewport()->set_input_as_handled();
            return;
        }
    }
    if (minimap.release_captured) {
        const Ref<InputEventMouseButton> release=event;
        if (release.is_valid() && release->get_button_index()==MOUSE_BUTTON_LEFT && !release->is_pressed()) {
            minimap.release_captured=false;
            minimap.dragging=false;
            get_viewport()->set_input_as_handled();
            return;
        }
    }
    if(chat_shortcut_input(event)) return;
    if (group_request && group_dialog_input(event)) return;
    if (map_input_delay.active() && button.is_valid() && button->get_button_index()==MOUSE_BUTTON_LEFT && button->is_pressed()) {
        get_viewport()->set_input_as_handled();return;
    }
    if (guild_editor.visible && guild_editor_input(event)) return;
    Ref<InputEventMouseMotion> motion=event;
    if (motion.is_valid()) {
        const auto p=motion->get_position();
        inventory_cursor=p;
        update_scene_target(p);
        if (shop_scroll_dragging && shop_scroll_input(1,p)) return;
        if (chat_history.scroll(model.layout.expanded).captured() && scroll_input(1,p)) return;
        if (minimap_pointer(1,p,motion->is_ctrl_pressed()) || panel_pointer(1,p)) return;
        controller.motion(int(p.x),int(p.y));
        queue_redraw();
    }
    const auto covers_wheel=[&](const mir3::DraggablePanel& panel) {
        return button.is_valid() && panel.visible && rect(panel.rect).has_point(button->get_position());
    };
    if (shop_actor && button.is_valid() && button->is_pressed() &&
        (button->get_button_index()==MOUSE_BUTTON_WHEEL_UP || button->get_button_index()==MOUSE_BUTTON_WHEEL_DOWN) &&
        Rect2(shop_window_left(),shop_window_top(),shop_width(),shop_window_height()).has_point(button->get_position())) {
        // 0x43D310 / 0x43D490 dispatch by the window under the pointer, including its frame.
        const int direction=button->get_button_index()==MOUSE_BUTTON_WHEEL_DOWN?1:-1;
        const int last=std::max(0,int(shop_goods.size())-1);
        shop_category_first=std::clamp(shop_category_first+direction,0,last);
        shop_scroll_fraction=last?double(shop_category_first)/last:0;
        shop_pressed=-1;
        get_viewport()->set_input_as_handled();
        queue_redraw();
        return;
    }
    // 0x43D310 / 0x43D490 route the wheel to the NPC panel under the cursor.
    if (npc_dialogue_actor && button.is_valid() && button->is_pressed() &&
        rect(npc_dialogue_bounds()).has_point(button->get_position()) &&
        !(shop_actor && Rect2(shop_window_left(),shop_window_top(),shop_width(),shop_window_height()).has_point(button->get_position()))) {
        const auto index=button->get_button_index();
        if (index==MOUSE_BUTTON_WHEEL_UP || index==MOUSE_BUTTON_WHEEL_DOWN) {
            const int delta=index==MOUSE_BUTTON_WHEEL_UP?-1:1;
            npc_dialogue_first=std::clamp(npc_dialogue_first+delta,0,npc_dialogue_scroll_limit);
            npc_dialogue_pressed=-1;
            npc_dialogue_arrow_pressed=0;
            get_viewport()->set_input_as_handled();
            queue_redraw();
            return;
        }
    }
    if (minimap.visible && button.is_valid() && button->is_pressed() &&
        (button->get_button_index()==MOUSE_BUTTON_WHEEL_UP || button->get_button_index()==MOUSE_BUTTON_WHEEL_DOWN) &&
        rect(minimap.bounds()).has_point(button->get_position())) {
        get_viewport()->set_input_as_handled();
        return;
    }
    if (portrait_panel.visible && button.is_valid() && button->is_pressed() &&
        rect(portrait_panel.rect).has_point(button->get_position()) &&
        !panel_covered(Panel::portrait,button->get_position())) {
        const auto index=button->get_button_index();
        if (index==MOUSE_BUTTON_WHEEL_UP || index==MOUSE_BUTTON_WHEEL_DOWN) {
            portrait_picker.wheel(index==MOUSE_BUTTON_WHEEL_UP?1:-1);
            get_viewport()->set_input_as_handled();
            queue_redraw();
            return;
        }
    }
    if (covers_wheel(quest) && !panel_covered(Panel::quest,button->get_position()) && button->is_pressed()) {
        const auto index=button->get_button_index();
        if (index==MOUSE_BUTTON_WHEEL_UP || index==MOUSE_BUTTON_WHEEL_DOWN) {
            const int delta=index==MOUSE_BUTTON_WHEEL_UP?-1:1;
            const auto [x,y,w,h]=quest.rect;
            const auto position=button->get_position();
            if(Rect2(x+80,y+310,170,70).has_point(position) && !quest.description().empty())
                quest.description_first=std::clamp(quest.description_first+delta,0,3);
            if(position.y<=y+310) quest.scroll(delta);
            get_viewport()->set_input_as_handled();
            queue_redraw();
            return;
        }
    }
    if (covers_wheel(options) && !panel_covered(Panel::options,button->get_position()) && button->is_pressed() &&
        (button->get_button_index()==MOUSE_BUTTON_WHEEL_UP || button->get_button_index()==MOUSE_BUTTON_WHEEL_DOWN)) {
        get_viewport()->set_input_as_handled();
        return;
    }
    if (covers_wheel(guild) && button->is_pressed() && !panel_covered(Panel::guild,button->get_position())) {
        const auto index=button->get_button_index();
        if (index==MOUSE_BUTTON_WHEEL_UP || index==MOUSE_BUTTON_WHEEL_DOWN) {
            const int count=int(guild_pages[guild_layout.tab].size());
            if (index==MOUSE_BUTTON_WHEEL_UP && guild_first>0) --guild_first;
            if (index==MOUSE_BUTTON_WHEEL_DOWN && guild_first<count-1) ++guild_first;
            get_viewport()->set_input_as_handled();
            queue_redraw();
            return;
        }
    }
    if (skills.visible && button.is_valid() && button->is_pressed()) {
        const auto index=button->get_button_index();
        const auto position=button->get_position();
        if ((index==MOUSE_BUTTON_WHEEL_UP || index==MOUSE_BUTTON_WHEEL_DOWN) &&
            rect(skills.rect).has_point(position) && !panel_covered(Panel::skills,position)) {
            skills.wheel(index==MOUSE_BUTTON_WHEEL_UP);
            get_viewport()->set_input_as_handled();
            queue_redraw();
            return;
        }
    }
    if (button.is_valid() && button->is_pressed() && (button->get_button_index()==MOUSE_BUTTON_WHEEL_UP || button->get_button_index()==MOUSE_BUTTON_WHEEL_DOWN) && rect(mir3::HudLayout::chat_frame(model.layout.expanded)).has_point(button->get_position()) &&
        !covers_wheel(portrait_panel) && !covers_wheel(options) && !covers_wheel(guild) && !covers_wheel(group) && !covers_wheel(skills) &&
        !covers_wheel(status) && !covers_wheel(inventory) && !covers_wheel(mount) && !covers_wheel(quest) && !covers_wheel(trade) && !covers_wheel(quick_panel)) {
        chat_history.scroll(model.layout.expanded).wheel(button->get_button_index()==MOUSE_BUTTON_WHEEL_UP?1:-1,int(chat_history.lines(model.layout.expanded).size()),model.layout.expanded);
        get_viewport()->set_input_as_handled();queue_redraw();return;
    }
    if (terrain_ready && button.is_valid() && button->get_button_index()==MOUSE_BUTTON_RIGHT && button->is_pressed()) {
        inventory_cursor=button->get_position();
        if (spell_pointer_available() && !map_pointer_button) {
            map_pointer_button=MOUSE_BUTTON_RIGHT;
            step_toward_pointer();
            get_viewport()->set_input_as_handled();
            return;
        }
    }
    if (button.is_valid() && button->get_button_index()==MOUSE_BUTTON_LEFT) {
        auto p=button->get_position();
        inventory_cursor=p;
        const int kind=button->is_pressed()?0:2;
        if (kind==2 && chat_history.scroll(model.layout.expanded).captured() && scroll_input(kind,p)) return;
        if (kind==2 && panel_pointer(kind,p,true)) return;
        if (shop_input(kind,p,button->is_double_click()) || npc_dialogue_input(kind,p) || skill_menu_input(kind,p,button->is_double_click())) return;
        if (kind==0 && button->is_double_click() && quick_panel.visible && !panel_covered(Panel::quick,p) && quick_slot_at(p)>=0) {
            activate_quick(quick_slot_at(p),true);get_viewport()->set_input_as_handled();return;
        }
        if (minimap_pointer(kind,p,button->is_ctrl_pressed()) || panel_pointer(kind,p) || scroll_input(kind,p)) return;
        const auto entry=frame(mir3::HudLayout::portrait_entry_frame(false));
        const auto anchor=mir3::HudLayout::portrait_entry_position;
        const bool portrait_hit=!model.layout.expanded && entry.is_valid() &&
            Rect2(Vector2(anchor[0],anchor[1]),entry->get_size()).has_point(p) && rect(mir3::HudLayout::portrait_hover).has_point(p);
        if (portrait_hit) {
            portrait_entry_pressed=kind==0;
            if (kind==2) {
                if (portrait_panel.visible) close_portrait_picker();
                else {
                    portrait_panel.visible=true;
                    raise_panel(Panel::portrait);
                    enumerate_portraits();
                    emit_signal("portrait_list_requested");
                }
                if (hud_sound && audio_options.effects_enabled) hud_sound->play();
            }
            get_viewport()->set_input_as_handled();
            queue_redraw();
            return;
        }
        const bool captured=model.pressed>=0 || controller.hit(int(p.x),int(p.y))>=0;
        int action=controller.pointer(int(p.x),int(p.y),button->is_pressed());
        if (chat_input) {
            if (action==14) chat_input->layout(model.layout.expanded);
            if (action==16 || action==17) chat_input->focus_chat(String::utf8(action==16?message(1072).c_str():"!"),true);
            if (action==18 || action==19) {
                const int id=chat_input->mode.toggle(action);
                post_chat(String::utf8(message(id).c_str()),0xffffff,0x80ff);
            }
        }
        if (action>=20 && action<=22) {
            const int id=reception_buttons.press(action,rejected_chat[action-20]);
            send_chat_command(String::utf8(message(id).c_str()));
        }
        if (action==0) request_trade();
        if (action==1) toggle_minimap(100);
        if (!captured && kind==2 && !model.layout.expanded && rect(mir3::HudLayout::portrait_hover).has_point(p)) {
            toggle_portrait_map();
            get_viewport()->set_input_as_handled();
            return;
        }
        if (action==2) skill_shortcuts_visible=!skill_shortcuts_visible;
        if (action==3 || action==4) open_group_dialog(action==3?109:101);
        if (action==6) { emit_signal("guild_overview_requested");send_client_packet(mir3::protocol::CM_GUILDHOME,0,String()); }
        if (action==5) toggle_group();
        if (action==8) toggle_quest();
        if (action==9) toggle_options();
        if (action==7) toggle_skills();
        if (action==11) toggle_mount();
        if (action==12) toggle_inventory();
        if (action==13) toggle_status();
        if (action>=0) {
            if (hud_sound && audio_options.effects_enabled) hud_sound->play();
            emit_signal("hud_action",action);
        }
        if (!captured && kind==0 && terrain_ready && spell_pointer_available()) {
            if (held_gold) {
                if (!map_input_delay.active()) {
                    held_gold=false;
                    open_group_dialog(102);
                }
                get_viewport()->set_input_as_handled();
                return;
            }
            if (bag.held && bag.held.make_index && bag.held_equipment_slot<0 && bag.held_quick_slot<0 &&
                    !bag.pending_use && !equipment.pending && bag.held.name!=message(1050)) {
                if (!pending_drop && !map_input_delay.active()) {
                    if (bag.held.stack_mode>=2 && bag.held.count>1) {
                        open_group_dialog(104);
                        get_viewport()->set_input_as_handled();
                        return;
                    }
                    // 42677D / 47C0E0 passes the item identity and clock as two separated WORDs.
                    pending_drop=std::exchange(bag.held,{});
                    bag.held_slot=-1;
                    const auto tick=std::uint32_t(GetTickCount());
                    send_client_packet(mir3::protocol::CM_DROPITEM,int(pending_drop.make_index),
                        String::utf8(pending_drop.name.c_str()),tick&0xffff,std::uint16_t(tick>>16));
                    queue_redraw();
                }
                get_viewport()->set_input_as_handled();
                return;
            }
            if (merchant_pointer(p)) {
                map_pointer_button=0;
                merchant_click_captured=true;
                get_viewport()->set_input_as_handled();
                return;
            }
            if (!map_pointer_button) map_pointer_button=MOUSE_BUTTON_LEFT;
            map_pointer_seconds=0;
            update_scene_target(p);
            map_shift_pressed=button->is_shift_pressed();
            chat_submission.alt_down=button->is_alt_pressed();
            step_toward_pointer(false,map_shift_pressed,chat_submission.alt_down);
            get_viewport()->set_input_as_handled();
        }
        if (captured) get_viewport()->set_input_as_handled();
        queue_redraw();
    }
}
void Mir3HudView::_notification(int what) {
    if (what==NOTIFICATION_WM_MOUSE_EXIT) {
        inventory_cursor=Vector2(-1,-1);
        controller.motion(-1,-1);
        queue_redraw();
    }
    if (what==NOTIFICATION_APPLICATION_FOCUS_OUT || what==NOTIFICATION_WM_WINDOW_FOCUS_OUT) {
        chat_submission.control_down=chat_submission.alt_down=false;
        cancel_pointer_interactions();
        inventory_cursor=Vector2(-1,-1);
        queue_redraw();
    }
}

void Mir3HudView::cancel_pointer_interactions() {
    map_pointer_button=0;
    map_shift_pressed=false;
    merchant_click_captured=false;
    group_dialog_pressed=-1;
    npc_dialogue_close_pressed=false;
    npc_dialogue_arrow_pressed=0;
    shop_pressed=npc_dialogue_pressed=-1;
    shop_scroll_dragging=false;
    minimap.dragging=false;
    minimap.release_captured=false;
    quick_panel.end_drag();quick_close_pressed=false;
    trade.end_drag();
    trade_scroll.captured=-1;
    trade_confirm_pressed=false;
    mount.end_drag();
    mount.pressed=-1;
    quest.end_drag();
    quest.pressed=-1;
    quest.scroll_captured=false;
    controller.cancel();
    portrait_entry_pressed=false;
    portrait_pressed=-1;
    portrait_picker.scrolling=false;
    options.end_drag();
    system_options.cancel_buttons();
    audio_drag=-1;
    guild.end_drag();
    guild_layout.pressed=-1;
    guild_scroll.captured=false;
    guild_editor.end_drag();
    guild_edit_pressed=-1;
    group.end_drag();
    group_pressed=-1;
    skills.end_drag();
    skills.pressed=-1;
    skills.scroll_captured=false;
    status.end_drag();
    status_close_pressed=false;
    inventory.end_drag();
    merchant_service.pressed=false;
    inventory.close_pressed=false;
    inventory.scroll.captured=false;
    chat_history.input(2,0,0,false,0,0,0);
    chat_history.input(2,0,0,true,0,0,0);
}

void Mir3HudView::toggle_quick() {
    if(frame(1210).is_null()) return;
    quick_panel.toggle();quick_close_pressed=false;
    if(quick_panel.visible) raise_panel(Panel::quick);
    controller.cancel();queue_redraw();
}

int Mir3HudView::quick_slot_at(Vector2 position) const {
    const auto [x,y,w,h]=quick_panel.rect;
    for(int i=0;i<6;++i) if(Rect2(x+17+42*i,y+6,38,38).has_point(position)) return i;
    return -1;
}

void Mir3HudView::draw_quick() {
    const auto [x,y,w,h]=quick_panel.rect;
    const auto skin=frame(1210);
    if(skin.is_valid()) sprite(1210,{x-(skin->get_width()-w)/2,y-(skin->get_height()-h)/2});
    sprite(quick_close_pressed?1212:1211,{x+264,y+32});
    for(int i=0;i<6;++i) if(const auto& item=quick_items.slots[i];item) {
        const auto icon=item_frame(item.icon);
        if(icon.is_valid()) paint_canvas->draw_texture(icon,Vector2(x+17+42*i+19-icon->get_width()/2,y+6+19-icon->get_height()/2));
        if(item.stack_mode>=2 && item.count>0) {
            const int offset=int(5*(5-std::log10(double(item.count))));
            text(std::to_string(item.count),{x+17+42*i+offset,y+6+23,38,16},0xffffff,mir3::TextFont::normal,false);
        }
    }
}

void Mir3HudView::quick_notice(int resource,int slot,const std::string& name) {
    if(resource<=0) return;
    auto caption=message(resource);
    if(auto at=caption.find("%s");at!=std::string::npos) caption.replace(at,2,name);
    if(auto at=caption.find("%d");at!=std::string::npos) caption.replace(at,2,std::to_string(slot+1));
    post_chat(String::utf8(caption.c_str()),11861940,0);
}

void Mir3HudView::activate_quick(int slot,bool mouse) {
    if(equipment.pending || trade_pending_slot>=0 || trade_return_slot>=0) return;
    const auto tick=std::uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
    const auto before=bag.held;
    const auto result=mouse?quick_items.mouse_activate(slot,bag,tick,message(1050)):quick_items.activate(slot,bag,tick,message(1050));
    quick_notice(result.binding_notice,slot,before.name);
    quick_notice(result.notice,slot,before.name);
    if(result.request) {
        const auto name=String::utf8(bag.held.name.c_str());
        emit_signal("inventory_use_requested",int64_t(bag.held.make_index),int64_t(tick),name);
        send_client_packet(mir3::protocol::CM_EAT,bag.held.make_index,name,std::uint16_t(tick),std::uint16_t(tick>>16));
    }
    queue_redraw();
}

bool Mir3HudView::quick_pointer(int kind,Vector2 position) {
    if(!quick_panel.visible) return false;
    const auto [x,y,w,h]=quick_panel.rect;
    const auto close=frame(1211);
    const bool on_close=close.is_valid() && Rect2(x+264,y+32,close->get_width(),close->get_height()).has_point(position);
    const bool inside=rect(quick_panel.rect).has_point(position);
    const int slot=quick_slot_at(position);
    bool consumed=false;
    if(kind==0 && inside) {
        raise_panel(Panel::quick);quick_close_pressed=on_close;
        quick_panel.begin_drag(int(position.x),int(position.y),on_close || slot>=0);
    } else if(kind==1) consumed=quick_panel.move_drag(int(position.x),int(position.y));
    else if(kind==2) {
        const bool closing=quick_close_pressed && on_close;
        if(!quick_panel.dragging && !quick_close_pressed && slot>=0 && !equipment.pending && trade_pending_slot<0) {
            if(trade_return_slot>=0) {
                // 41A8A0 -> 41DB70：交易来源放回交易栏，只取消持有，不绑定快捷栏。
                if(!trade_return_pending) {play_item_sound(trade_items[0][trade_return_slot]);trade_return_slot=-1;}
            } else {
                const auto sound=bag.held?bag.held:quick_items.slots[slot];
                const int result=quick_items.release(slot,bag,message(1050));
                quick_notice(result,slot,sound.name);
                if(result==0 || result==1068) play_item_sound(sound);
            }
            consumed=true;
        }
        consumed=quick_panel.end_drag() || quick_close_pressed || consumed;
        quick_close_pressed=false;
        if(closing) toggle_quick();
    }
    if(!inside && !consumed) return false;
    controller.cancel();get_viewport()->set_input_as_handled();queue_redraw();return true;
}

void Mir3HudView::draw_inventory() {
    const auto [x,y,w,h]=inventory.rect;
    const auto skin=frame(1220);
    if (skin.is_valid())
        sprite(1220,{x-(skin->get_width()-w)/2,y-(skin->get_height()-h)/2});
    text(message(merchant_service.actor?(merchant_service.storage?1704:merchant_service.selling?1703:1702):1701),{x+12,y+20,74,16},0xF8DCFA);
    if(merchant_service.actor) {
        sprite((merchant_service.storage?1231:merchant_service.selling?1229:1227)-int(merchant_service.pressed),{x+198,y+401});
        if(merchant_service.price) {
            auto caption=message(merchant_service.selling && merchant_service.stack?1451:1452);
            if(const auto at=caption.find("%d");at!=std::string::npos)
                caption.replace(at,2,std::to_string(merchant_service.price));
            text(caption,{x+70,y+423,85,14},0xF8DCFA);
        }
    }
    const auto weight=message(2501)+": "+std::to_string(model.weight)+" / "+std::to_string(model.max_weight);
    text(weight,{x+95,y+21,126,16},0x0a0a0a);
    text(weight,{x+94,y+20,126,16},0xF8C8C8);
    sprite(inventory.close_pressed?1222:1221,{x+242,y+423});
    sprite(1225,{x+250,mir3::InventoryScroll::thumb_y(inventory.row,y)});
    text(std::to_string(inventory_gold),{x+72,y+402,77,17},0x64C8F8,mir3::TextFont::gold);
    const Rect2 clip(x+17,y+60,228,304);
    // 0x440280 shares placement between the held-item preview and release.
    if (bag.held && clip.has_point(inventory_cursor) && !panel_covered(Panel::inventory,inventory_cursor)) {
        const int mouse_x=int(inventory_cursor.x),mouse_y=int(inventory_cursor.y);
        const int slot=bag.slot_at(mouse_x,mouse_y,x,y);
        const auto [column,row,width,height]=mir3::InventoryGrid::placement(bag.held,slot,mouse_x,mouse_y,x,y);
        const Rect2 footprint(x+17+column*38,y+60+(row-inventory.row)*38,width*38,height*38);
        const Rect2 visible=clip.intersection(footprint);
        const bool fits=bag.fits(column,row,bag.held);
        if (visible.has_area())
            legacy_rect(visible,fits?Color(50.f/255,100.f/255,50.f/255,1.f/255):
                                     Color(100.f/255,50.f/255,50.f/255,1.f/255));
    }
    for (int id=0;id<mir3::InventoryGrid::capacity;++id) {
        const auto& item=bag.items[id];
        if (!item) continue;
        const auto icon=item_frame(item.icon);
        if (icon.is_null()) continue;
        const int slot=bag.anchors[id];
        const Rect2 bounds(x+17+(slot%6)*38,y+60+(slot/6-inventory.row)*38,item.width*38,item.height*38);
        const Rect2 target((bounds.get_center()-icon->get_size()*0.5).floor(),icon->get_size());
        const Rect2 visible=target.intersection(clip);
        if (!visible.has_area()) continue;
        paint_canvas->draw_texture_rect_region(icon,visible,Rect2(visible.position-target.position,visible.size));
        const int label_x=int(bounds.position.x)+18,label_y=int(bounds.position.y)+22;
        if (item.count>1 && clip.encloses(Rect2(label_x,label_y,20,16)))
            text(std::to_string(item.count),{label_x,label_y,20,16},0xffffff,mir3::TextFont::normal,false);
    }

}

void Mir3HudView::toggle_inventory() {
    if (frame(1220).is_null()) return;
    inventory.toggle();
    if(!inventory.visible) merchant_service={};
    if (inventory.visible) raise_panel(Panel::inventory);
    merchant_service.pressed=false;
    inventory.close_pressed=false;
    inventory.scroll.captured=false;
    controller.cancel();
    queue_redraw();
}

void Mir3HudView::confirm_merchant_service(int quantity) {
    using Phase=MerchantService::Phase;
    if(!merchant_service.actor || merchant_service.phase!=Phase::quoted) return;
    const auto current=std::ranges::find_if(bag.items,[&](const auto& item) {
        return item && item.make_index==merchant_service.item && item.name==merchant_service.name;
    });
    if(current==bag.items.end()) {
        merchant_service.phase=Phase::choosing;
        merchant_service.price=0;
        return;
    }
    if((merchant_service.selling || merchant_service.storage) && current->stack_mode>=2 && current->count>1 && !quantity) {
        open_group_dialog(merchant_service.storage?1706:1705);
        return;
    }
    if(!quantity && merchant_service.storage && current->stack_mode>=2) quantity=1;
    if(quantity<0 || quantity>current->count) return;
    merchant_service.phase=Phase::paying;
    send_client_packet(merchant_service.storage?mir3::protocol::CM_USERSTORAGEITEM:merchant_service.selling?mir3::protocol::CM_USERSELLITEM:mir3::protocol::CM_USERREPAIRITEM,
        merchant_service.actor,String::utf8(merchant_service.name.c_str()),merchant_service.item,std::uint16_t(quantity));
}

bool Mir3HudView::inventory_pointer(int kind,Vector2 position) {
    if (!inventory.visible) return false;
    const auto [x,y,w,h]=inventory.rect;
    const auto close=frame(1221);
    const bool on_close=close.is_valid() && Rect2(x+242,y+423,close->get_width(),close->get_height()).has_point(position);
    const bool inside=Rect2(x,y,w,h).has_point(position);
    // 441230 / 441050 lifts the gold icon from its inventory rectangle.
    if (Rect2(x+15,y+390,42,27).has_point(position)) {
        if (kind==2 && !inventory.dragging && !bag.held && !held_gold) {
            held_gold=true;
            if (gold_sound && audio_options.effects_enabled) { gold_sound->play();++gold_sound_count; }
            queue_redraw();
        }
        get_viewport()->set_input_as_handled();
        return true;
    }
    const auto repair_button=frame(merchant_service.storage?1231:merchant_service.selling?1229:1227);
    const bool on_repair=merchant_service.actor && repair_button.is_valid() &&
        Rect2(Vector2(x+198,y+401),repair_button->get_size()).has_point(position);
    if(on_repair || merchant_service.pressed) {
        if(kind==0) {merchant_service.pressed=true;raise_panel(Panel::inventory);}
        if(kind==2) {
            using Phase=MerchantService::Phase;
            if(on_repair && merchant_service.pressed && merchant_service.phase==Phase::quoted) {
                confirm_merchant_service();
            }
            merchant_service.pressed=false;
        }
        get_viewport()->set_input_as_handled();
        queue_redraw();
        return true;
    }
    const auto thumb=frame(1225);
    const auto now=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    bag.scroll_row=inventory.row;
    bool consumed=thumb.is_valid() && inventory.scroll.input(kind,int(position.x),int(position.y),x,y,
        thumb->get_width(),thumb->get_height(),static_cast<std::uint32_t>(now),inventory.row);
    if (!consumed && kind==0 && inside) {
        inventory.close_pressed=on_close;
        if (inventory.begin_drag(int(position.x),int(position.y),on_close || Rect2(x+17,y+60,228,304).has_point(position) || Rect2(x+15,y+390,42,27).has_point(position))) raise_panel(Panel::inventory);
    } else if (!consumed && kind==1) {
        consumed=inventory.move_drag(int(position.x),int(position.y));
    } else if (kind==2) {
        const bool close_window=inventory.close_pressed && on_close;
        if (!consumed && !inventory.dragging && !inventory.close_pressed && trade_pending_slot<0) {
            bag.scroll_row=inventory.row;
            const int slot=bag.slot_at(int(position.x),int(position.y),x,y);
            consumed=slot>=0;
            if (consumed && held_gold) {
                get_viewport()->set_input_as_handled();
                return true;
            }
            if(consumed && merchant_service.actor) {
                using Phase=MerchantService::Phase;
                const int item_index=bag.item_at(slot);
                if(item_index>=0 && merchant_service.phase!=Phase::quoting && merchant_service.phase!=Phase::paying) {
                    const auto& item=bag.items[item_index];
                    merchant_service.item=item.make_index;
                    merchant_service.name=item.name;
                    merchant_service.stack=item.stack_mode>=2;
                    merchant_service.price=0;
                    merchant_service.phase=merchant_service.storage?Phase::quoted:Phase::quoting;
                    if(!merchant_service.storage)
                    send_client_packet(merchant_service.selling?mir3::protocol::CM_MERCHANTQUERYSELLPRICE:mir3::protocol::CM_MERCHANTQUERYREPAIRCOST,merchant_service.actor,String::utf8(item.name.c_str()),item.make_index);
                }
            } else if (consumed && trade_return_slot>=0) {
                if (!trade_return_pending) {
                    const auto& item=trade_items[0][trade_return_slot];
                    trade_return_pending=true;
                    trade_return_bag_slot=slot;
                    trade_return_bag_pointer=Vector2i(int(position.x)-x,int(position.y)-y);
                    emit_signal("trade_offer_requested",mir3::protocol::CM_TRADE_REMOVE_ITEM,int64_t(item.make_index),String::utf8(item.name.c_str()),item.count);
                    send_client_packet(mir3::protocol::CM_TRADE_REMOVE_ITEM,item.make_index,String::utf8(item.name.c_str()));
                }
            } else if (consumed && !equipment.pending && bag.held_equipment_slot>=0) {
                if (equipment.takeoff(bag)) emit_equipment_request();
            } else if (consumed && !equipment.pending) {
                const auto tick=static_cast<std::uint32_t>(now);
                const int action=bag.release_action(int(position.x),int(position.y),x,y,tick,bag.held && bag.held.name==message(1050));
                if (action==1) {
                    const auto name=String::utf8(bag.held.name.c_str());
                    emit_signal("inventory_use_requested",int64_t(bag.held.make_index),int64_t(tick),name);
                    send_client_packet(mir3::protocol::CM_EAT,bag.held.make_index,name,std::uint16_t(tick),std::uint16_t(tick>>16));
                }
                if (action==2) post_chat(String::utf8(message(1051).c_str()),11861940,0);
            }
        }
        consumed=inventory.end_drag() || inventory.close_pressed || consumed;
        inventory.close_pressed=false;
        if (close_window) {
            // 0x441310 enables the close button's release sound.
            if (hud_sound && audio_options.effects_enabled) hud_sound->play();
            toggle_inventory();
        }
    }
    bag.scroll_row=inventory.row;
    if (!inside && !consumed) return false;
    controller.cancel();
    get_viewport()->set_input_as_handled();
    queue_redraw();
    return true;
}

void Mir3HudView::_unhandled_key_input(const Ref<InputEvent>& event) {
    if (login_visible || character_selection.visible) { get_viewport()->set_input_as_handled();return; }
    Ref<InputEventKey> key=event;
    if (key.is_null() || !key->is_pressed()) return;
    if (guild_edit && guild_edit->has_focus()) return;
    const auto code=key->get_physical_keycode();
    if (key->is_echo()) {
        if (code==KEY_F10 || code==KEY_F12) {
            request_skill_cast(code-KEY_F1+1,true);
            get_viewport()->set_input_as_handled();
        }
        return;
    }
    if (code>=KEY_F1 && code<=KEY_F12) {
        const int slot=code-KEY_F1+1;
        if (!bind_skill_at_cursor(slot)) {
            const int id=bound_skill(slot);
            if (id && !map_input_delay.active() && !player_mount && spell_pointer_available()) {
                emit_signal("spell_key_requested",slot,id,inventory_cursor);
                request_skill_cast(slot,false);
            }
        }
        get_viewport()->set_input_as_handled();
        return;
    }
    if (code==KEY_Z) { toggle_quick();get_viewport()->set_input_as_handled();return; }
    if (code>=KEY_1 && code<=KEY_6 && !(key->is_shift_pressed() && code<=KEY_2)) {
        activate_quick(int(code-KEY_1));get_viewport()->set_input_as_handled();return;
    }
    if (code==KEY_R) {
        scroll_input(2,inventory_cursor);
        model.layout.toggle();
        controller.cancel();
        if (chat_input) chat_input->layout(model.layout.expanded);
        queue_redraw();
        get_viewport()->set_input_as_handled();
        return;
    }
    if (code==KEY_J || code==KEY_V || (minimap.visible && (code==KEY_T || code==KEY_Y))) {
        if (code==KEY_J) toggle_portrait_map();
        if (code==KEY_V) toggle_minimap(1000);
        if (code==KEY_T) minimap.resize();
        if (code==KEY_Y) minimap.translucent=!minimap.translucent;
        get_viewport()->set_input_as_handled();
        queue_redraw();
        return;
    }
    if (key->is_alt_pressed() && (code==KEY_Q || code==KEY_X)) {
        open_group_dialog(code==KEY_Q?109:101);
        get_viewport()->set_input_as_handled();
        return;
    }
    if (code==KEY_C) {
        request_trade();
        get_viewport()->set_input_as_handled();
        return;
    }
    if (code==KEY_B) {
        skill_shortcuts_visible=!skill_shortcuts_visible;
        queue_redraw();
        get_viewport()->set_input_as_handled();
        return;
    }
    if (code==KEY_D) {
        toggle_quest();get_viewport()->set_input_as_handled();return;
    }
    if (code==KEY_S) {
        toggle_mount();get_viewport()->set_input_as_handled();return;
    }
    if (code==KEY_N) {
        toggle_options();get_viewport()->set_input_as_handled();return;
    }
    if (code==KEY_ESCAPE) {
        if (shop_actor && (shop_show_offers || shop_bulk_index>=0)) {
            close_shop_offers();
            get_viewport()->set_input_as_handled();
            return;
        }
        if (shop_actor) {
            close_shop();
            get_viewport()->set_input_as_handled();
            return;
        }
        for (auto it=panel_order.rbegin();it!=panel_order.rend();++it) {
            if (!panel_state(*it).visible) continue;
            switch (*it) {
            case Panel::group: toggle_group(); break;
            case Panel::skills: toggle_skills(); break;
            case Panel::status: toggle_status(); break;
            case Panel::inventory: toggle_inventory(); break;
            case Panel::guild: guild.toggle();queue_redraw();break;
            case Panel::options: toggle_options();break;
            case Panel::portrait: close_portrait_picker();break;
            case Panel::mount: toggle_mount();break;
            case Panel::quest: toggle_quest();break;
            case Panel::trade: break;
            case Panel::quick: toggle_quick();break;
            }
            get_viewport()->set_input_as_handled();
            return;
        }
    }
    if (code==KEY_F) {
        emit_signal("guild_overview_requested");send_client_packet(mir3::protocol::CM_GUILDHOME,0,String());
        get_viewport()->set_input_as_handled();return;
    }
    if (code==KEY_G) {
        toggle_group();
        get_viewport()->set_input_as_handled();
        return;
    }
    if (code==KEY_E) {
        toggle_skills();
        get_viewport()->set_input_as_handled();
        return;
    }
    if (code==KEY_Q) {
        toggle_status();
        get_viewport()->set_input_as_handled();
        return;
    }
    if (code==KEY_W) {
        toggle_inventory();
        get_viewport()->set_input_as_handled();
    }
}

Ref<ImageTexture> Mir3HudView::item_frame(int id) {
    if (const auto it=item_frames.find(id);it!=item_frames.end()) return it->second;
    const auto decoded=item_archive.frame(id);
    if (!decoded) return {};
    return item_frames.emplace(id,texture(decoded->pixels.rgba,decoded->width,decoded->height)).first->second;
}

String Mir3HudView::item_layout_file() const {
    if(item_layout_directory.is_empty() || character_name.is_empty()) return String();
    // 角色名只作为文件名，不能让远端名称跳出配置目录。
    if(character_name=="." || character_name=="..") return String();
    for(const auto token:{"/","\\",":","*","?","\"","<",">","|"}) if(character_name.contains(token)) return String();
    return item_layout_directory.path_join(character_name+String(".itms"));
}

void Mir3HudView::save_item_layout() {
    const auto path=item_layout_file();
    if(path.is_empty()) return;
    std::error_code error;
    std::filesystem::create_directories(std::filesystem::u8path(item_layout_directory.utf8().get_data()),error);
    if(error) {UtilityFunctions::push_warning("Cannot create item layout directory.");return;}
    const auto data=mir3::ItemLayout::encode(bag,quick_items);
    const auto file=FileAccess::open(path,FileAccess::WRITE);
    if(file.is_null()) {UtilityFunctions::push_warning("Cannot save item layout: ",path);return;}
    PackedByteArray bytes;bytes.resize(data.size());std::copy(data.begin(),data.end(),bytes.ptrw());
    file->store_buffer(bytes);file->flush();
}

void Mir3HudView::restore_item_layout(mir3::InventoryGrid& items,mir3::QuickItems& references) {
    const auto path=item_layout_file();
    if(path.is_empty() || !FileAccess::file_exists(path)) return;
    const auto file=FileAccess::open(path,FileAccess::READ);
    if(file.is_null() || file->get_length()!=mir3::ItemLayout::file_size) return;
    const auto bytes=file->get_buffer(mir3::ItemLayout::file_size);
    mir3::ItemLayout::restore({bytes.ptr(),std::size_t(bytes.size())},items,references);
}

bool Mir3HudView::apply_inventory_wire(const mir3::ReceiveHeader& header,std::span<const std::uint8_t> body) {
    using namespace mir3::protocol;
    const auto decoded=mir3::InventoryWire::parse(header,body);
    if (!decoded) return false;
    auto next=bag;
    auto equipped=equipment.slots;
    if (header.opcode==SM_BAGITEMS) for (int i=0;i<mir3::InventoryGrid::capacity;++i) next.take(i);
    for (const auto& record:decoded->records) {
        const auto name=mir3::ansi_to_utf8(record.name());
        const auto matches=[&](const mir3::BagItem& item) { return item && item.make_index==record.identity() && item.name==name; };
        bool removed=false;
        if (header.opcode==SM_DELITEM || header.opcode==SM_UPDATEITEM) {
            for (int i=0;i<mir3::InventoryGrid::capacity;++i) if (matches(next.items[i])) {
                next.take(i);removed=true;break;
            }
        }
        mir3::BagItem item;
        if (header.opcode!=SM_DELITEM) {
            const auto sprite=item_frame(record.icon());
            if (sprite.is_null()) return false;
            const auto [width,height]=mir3::InventoryGrid::dimensions(sprite->get_width(),sprite->get_height());
            item={mir3::EquipmentState::category(record.mode()),record.icon(),record.count(),width,height,
                record.mode(),record.identity(),name,record.stack_mode(),record.raw};
            if ((header.opcode!=SM_UPDATEITEM || removed) && !next.insert(item)) return false;
        }
        if (header.opcode==SM_DELITEM || header.opcode==SM_UPDATEITEM)
            for (auto& slot:equipped) if (matches(slot)) {
                slot={};
                // 203以未指定槽位调用476440，重新放入第一个空装备槽。
                if (header.opcode==SM_UPDATEITEM)
                    for (auto& target:equipped) if (!target) { target=item;break; }
                break;
            }
    }
    if(header.opcode==SM_BAGITEMS) restore_item_layout(next,quick_items);
    bag=std::move(next);equipment.slots=std::move(equipped);
    if (header.opcode==SM_ADDITEM || header.opcode==SM_UPDATEITEM) {
        const auto& record=decoded->records.front();
        mir3::BagItem sound_item;sound_item.std_mode=record.mode();sound_item.name=mir3::ansi_to_utf8(record.name());
        sound_item.server_record=record.raw;play_item_sound(sound_item);
    }
    if (header.opcode==SM_ADDITEM || header.opcode==SM_DELITEM) {
        const auto& record=decoded->records.front();
        int resource=header.opcode==SM_DELITEM?1466:(record.stack_mode()<=2?1465:1461);
        if (header.opcode==SM_ADDITEM) for (int i=0;i<10;++i)
            if (record.name()==messages.get(1409)+std::to_string(i)) { resource=1463;break; }
        auto caption=message(resource);
        if (const auto at=caption.find("%s");at!=std::string::npos) caption.replace(at,2,mir3::ansi_to_utf8(record.name()));
        if (const auto at=caption.find("%d");at!=std::string::npos) caption.replace(at,2,std::to_string(record.count()));
        // Both 200 and 202 pass 0xB4FFB4 to the original system chat routine.
        post_chat(String::utf8(caption.c_str()),11861940,0);
    }
    queue_redraw();return true;
}

bool Mir3HudView::apply_inventory_add(int identifier,int icon,int count,int64_t make_index,String name,int std_mode,int stack_mode) {
    // Message 200 inserts an already decoded item using the normal bag placement path.
    if (identifier!=mir3::protocol::SM_ADDITEM || count<0 || count>65535 || make_index<=0 ||
        std_mode<0 || std_mode>255 || stack_mode<0 || stack_mode>255) return false;
    return add_inventory_item(icon,count,-1,make_index,name,std_mode,stack_mode);
}

bool Mir3HudView::add_inventory_item(int icon,int count,int slot,int64_t make_index,String name,int std_mode,int stack_mode) {
    if (make_index<0 || make_index>0xffffffffLL || (make_index && name.is_empty())) return false;
    if (count<0 || slot < -1 || slot>=mir3::InventoryGrid::columns*mir3::InventoryGrid::rows) return false;
    const auto sprite=item_frame(icon);
    if (sprite.is_null()) return false;
    const auto [width,height]=mir3::InventoryGrid::dimensions(sprite->get_width(),sprite->get_height());
    const int x=17+38*(slot%6)+20,y=60+38*(slot/6)+20;
    const bool inserted=bag.insert({-1,icon,count,width,height,std_mode,static_cast<std::uint32_t>(make_index),name.utf8().get_data(),stack_mode},slot,x,y);
    if (inserted) queue_redraw();
    return inserted;
}

bool Mir3HudView::apply_inventory_use_reply(int identifier,int remaining) {
    if (remaining<0 || remaining>65535) return false;
    const auto used=bag.held;
    const bool from_quick=bag.held_quick_slot>=0;
    if (from_quick) {
        if(!quick_items.reply(identifier,remaining,bag)) return false;
    } else if (!bag.apply_use_reply(identifier,remaining,inventory.rect[0],inventory.rect[1],
                             int(inventory_cursor.x),int(inventory_cursor.y))) return false;
    if (identifier==mir3::protocol::SM_EAT_OK) {
        if(used.stack_mode==3 && !from_quick) {
            if(used.count<=1) quick_items.remove(used.name);else quick_items.update_count(used.name,remaining);
        }
        auto name=used.name;
        for (int i=0;i<10;++i) if (name==message(1409)+std::to_string(i)) { name=message(1409);break; }
        auto caption=message(1464);
        if (const auto at=caption.find("%s");at!=std::string::npos) caption.replace(at,2,name);
        post_chat(String::utf8(caption.c_str()),mir3::chat_palette(2),mir3::chat_palette(7));
        if(from_quick) {
            auto extra=message(remaining?1467:1468);
            if(auto at=extra.find("%s");at!=std::string::npos) extra.replace(at,2,used.name);
            if(auto at=extra.find("%d");at!=std::string::npos) extra.replace(at,2,std::to_string(remaining));
            post_chat(String::utf8(extra.c_str()),mir3::chat_palette(2),mir3::chat_palette(7));
        }
        if (used.std_mode<=2) play_login_sound(sound_list.file(used.std_mode==0?108:107),false);
    } else if (used.std_mode==51) open_group_dialog(1414);
    queue_redraw();
    return true;
}

void Mir3HudView::draw_status() {
    using Layout=mir3::StatusLayout;
    const auto [x,y,w,h]=status.rect;
    const auto skin=frame(1280);
    if (skin.is_valid()) sprite(1280,{x-(skin->get_width()-w)/2,y-(skin->get_height()-h)/2});
    const bool obscured=panel_covered(Panel::status,inventory_cursor);
    const int local_x=int(inventory_cursor.x)-x,local_y=int(inventory_cursor.y)-y;
    if (bag.held && !obscured) {
        using Equipment=mir3::EquipmentState;
        const int slot=Equipment::hit(local_x,local_y,0,0);
        const auto [frame_id,left,top]=Equipment::preview(bag.held.std_mode,slot);
        if (frame_id>=0) {
            const auto highlight=frame(frame_id);
            if (highlight.is_valid()) {
                const Vector2 start(x+left,y+top);
                const Vector2 end=start+highlight->get_size();
                PackedVector2Array vertices,uv;
                for (const auto point:{start,Vector2(end.x,start.y),end,Vector2(start.x,end.y)})
                    vertices.push_back(point);
                for (const auto point:{Vector2(-2,-2),Vector2(-1,-2),Vector2(-1,-1),Vector2(-2,-1)})
                    uv.push_back(point);
                PackedColorArray colors;
                colors.push_back(Color(1,1,1,20.f/255));
                paint_canvas->draw_polygon(vertices,colors,uv,highlight);
            }
        }
    }
    if (status_body.is_valid())
        paint_canvas->draw_texture(status_body,Vector2(x+Layout::body[0],y+Layout::body[1])+status_body_offset);
    for (int slot=0;slot<12;++slot) {
        const auto& item=equipment.slots[slot];
        if (!item) continue;
        Ref<ImageTexture> icon;
        Vector2 offset;
        if (mir3::EquipmentState::uses_paperdoll(slot)) {
            if (!equipment_frames.contains(item.icon)) {
                const auto decoded=equipment_archive.frame(item.icon);
                if (!decoded) continue;
                equipment_frames.emplace(item.icon,std::make_pair(
                    texture(decoded->pixels.rgba,decoded->width,decoded->height),Vector2(decoded->offset_x,decoded->offset_y)));
            }
            const auto& cached=equipment_frames.at(item.icon);
            icon=cached.first;
            offset=cached.second;
        } else {
            icon=item_frame(item.icon);
        }
        if (icon.is_null()) continue;
        const auto pos=mir3::EquipmentState::draw_position(slot,icon->get_width(),icon->get_height(),int(offset.x),int(offset.y));
        paint_canvas->draw_texture(icon,Vector2(x+pos[0],y+pos[1]));
    }
    const int hovered_element=obscured?-1:Layout::element_at(local_x,local_y);
    const auto label=[&](Layout::Rect r,const std::string& value,unsigned color=0xfafafa,bool centered=true) {
        r[0]+=x;r[1]+=y;
        text(value,r,color,mir3::TextFont::normal,centered);
    };
    label(Layout::value(0),std::to_string(model.level));
    char experience[32]{};
    std::snprintf(experience,sizeof(experience),"%.2f%%",model.next_experience?100.0*model.experience/model.next_experience:0.0);
    label(Layout::value(1),experience,0x0000ff);
    const std::array<std::array<unsigned,2>,5> vitals{{
        {model.hp,model.max_hp},{model.mp,model.max_mp},{model.weight,model.max_weight},model.wear_weight,model.hand_weight}};
    for (int i=0;i<5;++i) {
        label(Layout::value(i+2,0),std::to_string(vitals[i][0]));
        label(Layout::value(i+2,1),std::to_string(vitals[i][1]));
        label(Layout::value(i+2,2),"/",0xfafafa,false);
    }
    const std::array ranges{model.attack,model.magic_attack,model.spirit_attack,model.defence,model.magic_defence};
    for (int i=0;i<5;++i) {
        label(Layout::combat_labels[i],message(Layout::combat_messages[i]));
        label(Layout::combat_values[i],std::to_string(ranges[i][0])+"-"+std::to_string(ranges[i][1]));
    }
    for (int i=0;i<2;++i) label(Layout::value(i+7),"+"+std::to_string(model.status_bonus_percent[i])+"%");
    const auto hint=[&](Layout::Rect r,const std::string& caption) {
        if (caption.empty() || r[2]<=0) return;
        const Rect2 bounds(x+r[0],y+r[1],r[2],r[3]);
        legacy_rect(bounds,Color(80.f/255,60.f/255,40.f/255,150.f/255));
        label(r,caption,0xffffff);
        paint_canvas->draw_rect(bounds,Color("644b32"),false);
    };
    for (int row=0;row<3;++row) {
        label(Layout::element_labels[row],message(Layout::element_messages[row]));
        for (int column=6;column>=0;--column) {
            if (!elements.visible(row,column)) continue;
            const auto r=Layout::element_rect(row,column,0);
            sprite(Layout::element_frame(row,column),{x+r[0],y+r[1]});
            label(Layout::element_rect(row,column,1),elements.text(row,column));
            if (hovered_element==row*7+column) hint(Layout::element_rect(row,column,3),message(5161+column));
        }
    }
    // 0x470E60 draws identity after the attributes and before the close button.
    auto name_rect=Layout::name;
    name_rect[0]+=x;
    name_rect[1]+=y-(character_secondary.is_empty()?0:16);
    text(character_name.utf8().get_data(),name_rect,0xdcffdc,mir3::TextFont::character_name);
    if (!character_secondary.is_empty()) {
        label({82,29,172,20},character_secondary.utf8().get_data(),0xb4faff);
        label({82,43,172,20},character_tertiary.utf8().get_data(),0x96c8ff);
    }
    if (!character_badge.is_empty()) {
        label({200,66,105,15},character_badge.utf8().get_data(),0xeb3ef4);
        sprite(1298,{x+181,y+67});
    }
    const int row=obscured?-1:Layout::tooltip_at(local_x,local_y);
    if (row>=0) {
        const auto caption=row==2?"HP":row==3?"MP":message(Layout::tooltip_messages[row]);
        const auto measured=mir3::raster_text(caption,800,24,false,mir3::TextFont::normal);
        const auto hit=Layout::tooltip(row);
        hint({hit[0]-10,hit[1]-15,measured.text_width,hit[3]},caption);
    }
    sprite(status_close_pressed?1222:1221,{x+Layout::close[0],y+Layout::close[1]});
}

void Mir3HudView::toggle_status() {
    if (frame(1280).is_null()) return;
    status.toggle();
    status_close_pressed=false;
    if (status.visible) raise_panel(Panel::status);
    controller.cancel();
    queue_redraw();
}

bool Mir3HudView::status_pointer(int kind,Vector2 position) {
    if (!status.visible) return false;
    const auto [x,y,w,h]=status.rect;
    const bool inside=Rect2(x,y,w,h).has_point(position);
    const auto close=frame(1221);
    const auto anchor=mir3::StatusLayout::close;
    const bool on_close=close.is_valid() && Rect2(x+anchor[0],y+anchor[1],close->get_width(),close->get_height()).has_point(position);
    const int slot=mir3::EquipmentState::hit(int(position.x),int(position.y),x,y);
    bool captured=false;
    if (kind==0 && inside) {
        raise_panel(Panel::status);
        status_close_pressed=on_close;
        if (status.begin_drag(int(position.x),int(position.y),on_close || slot>=0)) raise_panel(Panel::status);
    } else if (kind==1) {
        captured=status.move_drag(int(position.x),int(position.y));
    } else if (kind==2) {
        const bool activate=status_close_pressed && on_close;
        captured=status.end_drag() || status_close_pressed;
        status_close_pressed=false;
        if (activate) {
            // 0x4767E5 enables the close button's release sound.
            if (hud_sound && audio_options.effects_enabled) hud_sound->play();
            toggle_status();
        }
        if (!captured && slot>=0 && trade_pending_slot<0 && trade_return_slot<0 &&
            (!bag.held || bag.held.make_index)) {
            const auto now=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
            const bool was_pending=equipment.pending;
            equipment.release_action(slot,bag,static_cast<std::uint32_t>(now));
            if (equipment.use_rejected_notice) post_chat(String::utf8(message(2413).c_str()),11861940,0);
            if ((!was_pending && equipment.pending) || equipment.quick_request_ready) {
                equipment.quick_request_ready=false;
                emit_equipment_request();
            }
        }
    }
    if (!inside && !captured) return false;
    controller.cancel();
    get_viewport()->set_input_as_handled();
    queue_redraw();
    return true;
}

bool Mir3HudView::panel_pointer(int kind,Vector2 position,bool captured_only) {
    if (kind==1 && options.visible && !rect(options.rect).has_point(position) && !options.dragging && audio_drag<0) {
        system_options.cancel_buttons();queue_redraw();
    }
    if (quick_panel.dragging || quick_close_pressed) return quick_pointer(kind,position);
    if (trade.dragging || trade_confirm_pressed || trade_scroll.captured>=0) return trade_pointer(kind,position);
    if (quest.dragging || quest.pressed>=0 || quest.scroll_captured) return quest_pointer(kind,position);
    if (mount.dragging || mount.pressed>=0) return mount_pointer(kind,position);
    if (portrait_picker.scrolling || portrait_pressed>=0) return portrait_input(kind,position);
    if (options.dragging || audio_drag>=0) return options_pointer(kind,position);
    if (guild.dragging || guild_scroll.captured || guild_layout.pressed>=0) return guild_pointer(kind,position);
    if (status.dragging || status_close_pressed) return status_pointer(kind,position);
    if (merchant_service.pressed || inventory.dragging || inventory.close_pressed || inventory.scroll.captured)
        return inventory_pointer(kind,position);
    if (skills.dragging || skills.pressed>=0 || skills.scroll_captured) return skill_pointer(kind,position);
    if (group.dragging || group_pressed>=0) return group_pointer(kind,position);
    if (captured_only) return false;
    const auto order=panel_order;
    for (auto it=order.rbegin();it!=order.rend();++it) {
        switch (*it) {
        case Panel::group: if (group_pointer(kind,position)) return true; break;
        case Panel::skills: if (skill_pointer(kind,position)) return true; break;
        case Panel::status: if (status_pointer(kind,position)) return true; break;
        case Panel::inventory: if (inventory_pointer(kind,position)) return true; break;
        case Panel::guild: if (guild_pointer(kind,position)) return true; break;
        case Panel::options: if (options_pointer(kind,position)) return true; break;
        case Panel::portrait: if (portrait_input(kind,position)) return true; break;
        case Panel::mount: if (mount_pointer(kind,position)) return true; break;
        case Panel::quest: if (quest_pointer(kind,position)) return true; break;
        case Panel::trade: if (trade_pointer(kind,position)) return true; break;
        case Panel::quick: if (quick_pointer(kind,position)) return true; break;
        }
    }
    return false;
}

void Mir3HudView::toggle_skills() {
    if (frame(1620).is_null()) return;
    skills.toggle();
    skill_key_menu={};
    skills.pressed=-1;
    skills.scroll_captured=false;
    if (skills.visible) raise_panel(Panel::skills);
    controller.cancel();
    queue_redraw();
}

void Mir3HudView::draw_skills() {
    const auto [x,y,w,h]=skills.rect;
    const auto skin=frame(1620);
    if (skin.is_valid()) sprite(1620,{x-(skin->get_width()-w)/2,y-(skin->get_height()-h)/2});
    const auto tree=frame(skills.tree_frame());
    if (tree.is_valid()) {
        const Rect2 crop(0,skills.scroll,tree->get_width(),mir3::SkillPanel::tree_height);
        const auto origin=mir3::SkillPanel::tree_origin;
        paint_canvas->draw_texture_rect_region(tree,Rect2(Vector2(x+origin[0],y+origin[1]),crop.size),crop);
    }
    sprite(1672,{x+322,y+skills.thumb_y()});
    const auto& learned=skills.learned[skills.tab];
    for (int i=0;i<std::min(int(learned.size()),mir3::SkillPanel::visible_skill_limit);++i) {
        const auto& skill=learned[i];
        if (!skill.id) continue;
        const int slot=mir3::SkillLayout::slot(skills.tab,skill.id);
        const auto pos=slot>=0?mir3::SkillLayout::positions[skills.tab][slot]:
            mir3::SkillPosition{skill.id,i<6?55:0,i<6?27+46*i:0};
        const int top=pos.y-(slot>=0?skills.scroll:0);
        if (skills.selected[skills.tab]==(slot<0?15:slot)) {
            if (top-1>62 && top-1<352) {
                const int left=x+pos.x-1,upper=y+top-1;
                const Color edge("c8c800");
                paint_canvas->draw_rect(Rect2(left,upper,42,1),edge);
                paint_canvas->draw_rect(Rect2(left,upper+41,42,1),edge);
                paint_canvas->draw_rect(Rect2(left,upper,1,42),edge);
                paint_canvas->draw_rect(Rect2(left+41,upper,1,42),edge);
            }
            text(skill.name,{x+20,y+387,80,61},0xfafafa,mir3::TextFont::normal,true,true);
            const auto level=skill.level>=3?std::string("Level : Max"):"Level : "+std::to_string(skill.level);
            text(level,{x+20,y+403,80,62},0xfafafa,mir3::TextFont::normal,true,true);
            if (skill.level<3)
                text("Exp : "+skill.progress_text(),
                     {x+100,y+403,80,62},0xfafafa,mir3::TextFont::normal,true,true);
        }
        const int id=2*skill.id-2;
        if (!skill_frames.contains(id)) {
            const auto decoded=magic_icons.frame(id);
            if (decoded) skill_frames.emplace(id,texture(decoded->pixels.rgba,decoded->width,decoded->height));
        }
        const auto draw_icon=[&](const Ref<ImageTexture>& icon) {
            if (icon.is_null()) return;
            const int hidden_top=std::max(0,64-top);
            const int hidden_bottom=std::max(0,top-355);
            const int height=icon->get_height()-hidden_top-hidden_bottom;
            if (height<=0) return;
            const Rect2 crop(0,hidden_top,icon->get_width(),height);
            paint_canvas->draw_texture_rect_region(icon,Rect2(Vector2(x+pos.x,y+top+hidden_top),crop.size),crop);
        };
        if (const auto icon=skill_frames.find(id);icon!=skill_frames.end()) draw_icon(icon->second);
        if (skill.key) draw_icon(frame(1659+skill.key));
        if (top+37>56 && top+37<385)
            text(std::to_string(skill.level),{x+pos.x+34,y+top+37,15,15},0x64fafa,mir3::TextFont::skill_level);
    }
    for (const int index:{0,8,7,6,5,4,3,2,1}) {
        const auto [base,left,top]=mir3::SkillPanel::button(index);
        if (index==0 || skills.pressed==index || !skills.learned[index-1].empty())
            sprite(base+(index==0 && skills.pressed==0),{x+left,y+top});
        if (index>0 && skills.tab==index-1) sprite(base+1,{x+left,y+top});
    }
}

int Mir3HudView::skill_button_at(Vector2 position) {
    for (int index=0;index<=8;++index) {
        const auto [base,x,y]=mir3::SkillPanel::button(index);
        const auto icon=frame(base);
        if (icon.is_valid() && Rect2(Vector2(skills.rect[0]+x,skills.rect[1]+y),icon->get_size()).has_point(position))
            return index;
    }
    return -1;
}

bool Mir3HudView::skill_pointer(int kind,Vector2 position) {
    if (!skills.visible) return false;
    const auto [x,y,w,h]=skills.rect;
    const bool inside=Rect2(x,y,w,h).has_point(position);
    const auto thumb=frame(1672);
    const auto now=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    bool consumed=thumb.is_valid() && skills.scroll_input(kind,int(position.x)-x,int(position.y)-y,
        thumb->get_width(),thumb->get_height(),static_cast<std::uint32_t>(now));
    if (kind==0 && inside) {
        raise_panel(Panel::skills);
        if (!consumed) {
            skills.pressed=skill_button_at(position);
            if (skills.pressed<0) {
                const auto& positions=mir3::SkillLayout::positions[skills.tab];
                for (int i=0;i<int(positions.size());++i) {
                    const auto pos=positions[i];
                    if (Rect2(x+pos.x,y+pos.y-skills.scroll,40,40).has_point(position))
                        skills.selected[skills.tab]=i;
                }
            }
            skills.begin_drag(int(position.x),int(position.y),skills.pressed>=0);
        }
    } else if (kind==1 && !consumed) {
        consumed=skills.move_drag(int(position.x),int(position.y));
    } else if (kind==2 && !consumed) {
        const int button=skill_button_at(position);
        consumed=skills.end_drag() || skills.pressed>=0;
        if (skills.release_button(button)) {
            // 0x450860 enables release sound on close and all eight tabs.
            if (hud_sound && audio_options.effects_enabled) hud_sound->play();
            if (button==0) toggle_skills();
        }
    }
    if (!inside && !consumed) return false;
    controller.cancel();
    get_viewport()->set_input_as_handled();
    queue_redraw();
    return true;
}

void Mir3HudView::update_bag_stack_count(std::uint32_t instance,int quantity,bool remove_when_empty) {
    // 441DF8 updates the item held by the cursor even when its bag slot is empty.
    if ((!remove_when_empty || quantity) && bag.held && bag.held_quick_slot<0 && bag.held.make_index==instance) {
        bag.held.set_count(quantity);
        queue_redraw();
    }
    for (int i=0;i<int(bag.items.size());++i) {
        auto& item=bag.items[i];
        if (!item || item.make_index!=instance) continue;
        if (!quantity && remove_when_empty) {
            quick_items.remove(item.name);
            bag.take(i);
            queue_redraw();
            return;
        }
        // 441E33 reports increases and plays the normal item sound.
        if (quantity>item.count) {
            auto caption=message(1707);
            if (const auto at=caption.find("%s");at!=std::string::npos) caption.replace(at,2,item.name);
            if (const auto at=caption.find("%d");at!=std::string::npos) caption.replace(at,2,std::to_string(quantity-item.count));
            post_chat(String::utf8(caption.c_str()),11861940,0);
            play_item_sound(item);
        }
        item.set_count(quantity);
        quick_items.update_count(item.name,quantity);
        queue_redraw();
        break;
    }
}

void Mir3HudView::play_item_sound(const mir3::BagItem& item) {
    int sample=118;
    switch(item.std_mode) {
    case 0:sample=108;break;case 5:case 6:sample=111;break;
    case 10:case 11:sample=112;break;case 15:sample=116;break;
    case 19:case 20:case 21:sample=115;break;case 22:case 23:sample=113;break;
    case 24:case 26:sample=(item.name.find(message(2641))!=std::string::npos || item.name.find(message(2642))!=std::string::npos)?117:114;break;
    case 31:sample=item.server_record[31]<106?108:118;break;
    }
    play_login_sound(sound_list.file(sample),false);
}

void Mir3HudView::emit_equipment_request() {
    const auto& request=equipment.request;
    emit_signal("equipment_requested",request.identifier,request.slot,int64_t(request.make_index),String::utf8(request.name.c_str()));
    if (request.identifier==mir3::protocol::CM_EQUIPMENT_QUICK_ACTION)
        send_client_packet(request.identifier,request.slot,String());
    else
        send_client_packet(request.identifier,request.make_index,String::utf8(request.name.c_str()),request.slot);
}

bool Mir3HudView::apply_equipment_reply(int identifier) {
    const auto incoming=equipment.incoming,held=bag.held;
    if (!equipment.reply(identifier,bag)) return false;
    if (identifier==mir3::protocol::SM_TAKEON_OK || identifier==mir3::protocol::SM_TAKEON_FAIL) play_item_sound(incoming);
    else if (held) play_item_sound(held);
    if (identifier==mir3::protocol::SM_TAKEON_FAIL && held) play_item_sound(held);
    queue_redraw();
    return true;
}

bool Mir3HudView::set_learned_skill(int id,int level,int key,int experience,int required,String name) {
    if (id<1 || id>65535 || level<0 || level>255 || key<0 || key>12) return false;
    auto& learned=skills.learned[mir3::SkillLayout::category(id)];
    auto found=std::find_if(learned.begin(),learned.end(),[id](const auto& skill) { return skill.id==id; });
    const mir3::SkillDisplay entry{id,level,key,static_cast<std::uint32_t>(experience),required,name.utf8().get_data(),{}};
    if (found==learned.end()) learned.push_back(entry);
    else {
        if (found->key && skills.bindings[found->key]==(id&255)) skills.bindings[found->key]=0;
        *found=entry;
    }
    if (key) skills.bindings[key]=id&255;
    queue_redraw();
    return true;
}

bool Mir3HudView::apply_skill_records(int identifier,int count,PackedByteArray records) {
    const bool full=identifier==mir3::protocol::SM_SENDMYMAGIC;
    if ((!full && (identifier!=mir3::protocol::SM_ADDMAGIC || count!=1)) || count<0 || count>255 || records.size()!=74*count) return false;
    // The transport supplies fixed-size records after decoding each wire segment.
    auto next=skills.learned;
    auto bindings=skills.bindings;
    if (full) for (auto& group:next) group.clear();
    for (int i=0;i<count;++i) {
        const auto* data=records.ptr()+74*i;
        const auto word=[data](int offset) {
            std::uint32_t value=0;
            for (int j=0;j<4;++j) value|=std::uint32_t(data[offset+j])<<(8*j);
            return value;
        };
        const int id=data[6]|(data[7]<<8), level=data[5];
        if (!id) return false;
        const std::string key_text(reinterpret_cast<const char*>(data+4),70);
        const long decimal=std::strtol(key_text.c_str(),nullptr,10);
        const int label=decimal>0 && decimal<10?int(decimal):int(data[4])-55;
        const int slot=full?label:static_cast<std::uint8_t>(decimal);
        if (slot>=1 && slot<=12) bindings[slot]=data[6];
        const std::string_view name(reinterpret_cast<const char*>(data+8),66);
        const std::array thresholds{int(word(31)),int(word(35)),int(word(39))};
        next[mir3::SkillLayout::category(id)].push_back({id,level,label>=1 && label<=12?label:0,
            word(0),level<3?thresholds[level]:0,mir3::ansi_to_utf8(name.substr(0,name.find(char(0)))),thresholds,
            word(49),data[53]});
    }
    skills.learned=std::move(next);
    skills.bindings=bindings;
    const int previous=skills.tab;
    if (full) {
        skills.tab=0;
        skill_key_menu={};
        skill_menu_target=0;
    }
    skills.select_nonempty();
    if (full && skills.learned[skills.tab].empty()) skills.tab=previous;
    queue_redraw();
    return true;
}

bool Mir3HudView::apply_skill_removal(int id) {
    if (id<1 || id>65535) return false;
    // 0x44F670 (212) removes the record but leaves the shortcut table intact.
    const bool removed=skills.remove(id);
    if (removed && skill_menu_target==id) {
        skill_key_menu={};
        skill_menu_target=0;
    }
    queue_redraw();
    return removed;
}

bool Mir3HudView::apply_skill_progress(int id,int64_t experience,int level) {
    // 0x44F870 (640): update the first matching learned record.
    if (id<1 || id>65535 || level<0 || level>255 || experience<0 || experience>UINT32_MAX) return false;
    if (!skills.update_progress(id,static_cast<std::uint32_t>(experience),level)) return false;
    queue_redraw();
    return true;
}

bool Mir3HudView::bind_skill_at_cursor(int key) {
    const auto [x,y,w,h]=skills.rect;
    if (!skills.visible || !Rect2(x,y,w,h).has_point(inventory_cursor)) return false;
    if (panel_covered(Panel::skills,inventory_cursor)) return false;
    // 0x44F8E0 clears a conflicting binding even over an unlearned tree slot.
    for (const auto pos:mir3::SkillLayout::positions[skills.tab]) {
        const Rect2 hit(x+pos.x,y+pos.y-skills.scroll,40,40);
        if (!hit.has_point(inventory_cursor)) continue;
        bind_skill(pos.id,key);
    }
    queue_redraw();
    return true;
}

void Mir3HudView::draw_skill_description() {
    if (!skills.visible || skills.scroll_captured || skill_key_menu.has_area() ||
        !rect(skills.rect).has_point(inventory_cursor)) return;
    if (panel_covered(Panel::skills,inventory_cursor)) return;

    int id=0;
    for (const auto pos:mir3::SkillLayout::positions[skills.tab]) {
        if (pos.id && Rect2(skills.rect[0]+pos.x,skills.rect[1]+pos.y-skills.scroll,40,40).has_point(inventory_cursor)) {
            id=pos.id;
            break;
        }
    }
    if (!id) return;
    if (!description_rows.contains(id))
        description_rows.emplace(id,mir3::wrap_skill_description(skill_descriptions.section(id)));
    const auto& rows=description_rows.at(id);
    if (rows.empty()) return;
    int width=0;
    for (const auto& row:rows) if (row.width>width) width=row.width+7;
    const int mouse_x=int(inventory_cursor.x);
    const int x=mouse_x+190>800?630:mouse_x+20;
    int y=int(inventory_cursor.y)+20;
    legacy_rect(Rect2(x-2,y,width,int(rows.size())*15+30),Color(100.f/255,100.f/255,50.f/255,150.f/255));
    for (const auto& row:rows) {
        y+=15;
        if (row.title) {
            for (const auto shadow:mir3::HudLayout::coordinate_shadows)
                text(row.text,{x+shadow[0],y+shadow[1],300,24},0x0a0a0a,mir3::TextFont::skill_title,false);
        }
        text(row.text,{x,y,300,24},row.title?0x96c8fa:0xfafafa,
             row.title?mir3::TextFont::skill_title:mir3::TextFont::normal,false);
        if (row.title) y+=15;
    }
    for (const auto& skill:skills.learned[skills.tab]) {
        if (skill.id!=id) continue;
        const auto progress=skill.level>=3?std::string("Level : Max"):
            skill.progress_text();
        text(progress,{x,y,skill.level>=3?80:50,15},0x1414fa);
        break;
    }

}

void Mir3HudView::bind_skill(int id,int key) {
    skills.bindings[key]=0;
    for (auto& group:skills.learned) for (auto& skill:group) {
        if (!skill.id || skill.key!=key) continue;
        skill.key=0;
        emit_signal("skill_key_requested",skill.id,0);
        send_client_packet(mir3::protocol::CM_MAGICKEYCHANGE,skill.id,String());
    }
    for (auto& skill:skills.learned[mir3::SkillLayout::category(id)]) {
        if (!skill.id || skill.id!=id) continue;
        if (skill.key && skills.bindings[skill.key]==(skill.id&255)) skills.bindings[skill.key]=0;
        skill.key=key;
        skills.bindings[key]=skill.id&255;
        const int legacy_key=key<=9?'0'+key:'A'+key-10;
        emit_signal("skill_key_requested",skill.id,legacy_key);
        send_client_packet(mir3::protocol::CM_MAGICKEYCHANGE,skill.id,String(),legacy_key);
        break;
    }
    queue_redraw();
}

bool Mir3HudView::skill_menu_input(int kind,Vector2 point,bool double_click) {
    if (!skills.visible || panel_covered(Panel::skills,point)) return false;
    if (skill_key_menu.has_area()) {
        if (skill_key_menu.has_point(point)) {
            if (kind==2) {
                const Vector2 local=point-skill_key_menu.position;
                const int key=int(local.y)/40*4+int(local.x)/40+1;
                const auto& learned=skills.learned[mir3::SkillLayout::category(skill_menu_target)];
                if (std::any_of(learned.begin(),learned.end(),[this](const auto& skill) {
                    return skill.id==skill_menu_target;
                })) bind_skill(skill_menu_target,key);
            }
            get_viewport()->set_input_as_handled();
            return true;
        }
        if (kind==0) {
            skill_key_menu={};
            queue_redraw();
        }
    }
    if (kind!=0 || !double_click || !rect(skills.rect).has_point(point)) return false;
    const auto [x,y,w,h]=skills.rect;
    const auto& positions=mir3::SkillLayout::positions[skills.tab];
    const auto& learned=skills.learned[skills.tab];
    // 0x450BD0 uses the first ten unscrolled positions and compares both axes against RECT.right.
    for (int index=0;index<10;++index) {
        const auto pos=positions[index];
        if (!pos.id || !Rect2(x+pos.x,y+pos.y,40,40).has_point(point)) continue;
        if (std::none_of(learned.begin(),learned.end(),[&](const auto& skill){return skill.id==pos.id;})) continue;
        const int left=std::min(int(point.x),x+w-160);
        const int top=point.y+160>x+w?y+h-120:int(point.y);
        skill_key_menu=Rect2(left,top+40,160,120);
        // 0x450BD0 opens over a learned slot without changing selection.
        const int selected=skills.selected[skills.tab];
        skill_menu_target=selected>=0 && selected<int(positions.size())?positions[selected].id:0;
        raise_panel(Panel::skills);
        skills.end_drag();
        skills.pressed=-1;
        get_viewport()->set_input_as_handled();
        queue_redraw();
        return true;
    }
    return false;
}

bool Mir3HudView::spell_pointer_available() const {
    using Layout=mir3::HudLayout;
    const auto pointer=inventory_cursor;
    if (minimap.visible || group_request || (shop_actor && Rect2(shop_window_left(),shop_window_top(),shop_width(),shop_window_height()).has_point(pointer)) || (npc_dialogue_actor && rect(npc_dialogue_bounds()).has_point(pointer)) || !Rect2(0,0,800,Layout::origin[1]).has_point(pointer) ||
        rect(Layout::chat_frame(model.layout.expanded)).has_point(pointer)) return false;
    if (guild_editor.visible && rect(guild_editor.rect).has_point(pointer)) return false;
    for (const auto panel:panel_order) {
        const auto& state=panel_state(panel);
        if (state.visible && rect(state.rect).has_point(pointer)) return false;
    }
    return true;
}

void Mir3HudView::request_skill_cast(int slot,bool repeat) {
    if (slot<1 || slot>12 || !terrain_ready || player_life.dead || player_mount ||
        map_input_delay.active() || !spell_pointer_available() || pending_local_action) return;
    const auto now=std::uint32_t(GetTickCount());
    if (!spell_timing.key_ready(now,slot,repeat)) return;
    const int id=bound_skill(slot);
    const auto& group=skills.learned[mir3::SkillLayout::category(id)];
    const auto found=std::find_if(group.begin(),group.end(),[id](const auto& skill) { return skill.id==id; });
    if (found==group.end()) return; // 删除技能后原版快捷表可残留，仍须查实际技能记录。
    const auto& route=mir3::SpellRoutes::get(id);
    if (!route.bypass_mana && model.mp<int(found->mana_threshold)+10) {
        post_system_notice(String::utf8(message(1000).c_str()),11861940,0);
        return;
    }
    active_spell=id;
    const auto camera=map_camera();
    Vector2i target(int(inventory_cursor.x+camera.x)/48,int(inventory_cursor.y+camera.y)/32);
    std::uint32_t target_actor=0;
    int target_type=-1;
    if (const auto human=scene_humans.find(hovered_target);human!=scene_humans.end()) {
        target=human->second.cell;target_actor=hovered_target;target_type=human->second.type;
        const auto name=scene_names.find(hovered_target);
        constexpr auto lockable_name_color=27631u; // 427014, original palette index 69.
        if (name!=scene_names.end() && name->second.color==lockable_name_color &&
            ((target_type==0 && !human->second.life.dead) || target_type==1)) {
            locked_spell_actor=hovered_target;
        }
    } else if (const auto monster=scene_chickens.find(hovered_target);monster!=scene_chickens.end()) {
        target=monster->second.cell;target_actor=hovered_target;target_type=3;
        // 42610D / 427016 remember the current monster for subsequent skill keys.
        locked_spell_actor=hovered_target;
        queue_redraw();
    } else if (locked_spell_actor) {
        using mir3::SkillId;
        // 430050 allows reuse only for these bound skill IDs.
        switch (static_cast<SkillId>(id)) {
        case SkillId::fireball: case SkillId::fireball2: case SkillId::amyounsul:
        case SkillId::lightening: case SkillId::firecharm: case SkillId::tamming: case SkillId::killundead:
        case SkillId::original_37: case SkillId::original_38: case SkillId::original_39: case SkillId::original_40:
        case SkillId::original_41: case SkillId::original_67: case SkillId::original_74:
            if (const auto human=scene_humans.find(locked_spell_actor);human!=scene_humans.end()) {
                target=human->second.cell;target_actor=locked_spell_actor;target_type=human->second.type;
            } else if (const auto monster=scene_chickens.find(locked_spell_actor);monster!=scene_chickens.end()) {
                target=monster->second.cell;target_actor=locked_spell_actor;target_type=3;
            }
            break;
        default: break;
        }
    }
    const auto delta=target-player_cell;
    const mir3::MapCell direction{
        std::abs(delta.y)>2 && std::abs(delta.x)<=1?0:(delta.x>0)-(delta.x<0),
        std::abs(delta.x)>2 && std::abs(delta.y)<=1?0:(delta.y>0)-(delta.y<0)};
    const auto heading=std::find(mir3::map_directions.begin(),mir3::map_directions.end(),direction);
    const int facing=heading==mir3::map_directions.end()?4:int(heading-mir3::map_directions.begin());
    auto action=route.action;
    auto followup_target=target_actor;
    bool send=false,predict=false;
    std::uint32_t* direct_tick=nullptr;
    std::uint32_t direct_interval=0;
    if (!action_prediction.waiting && action!=mir3::SpellAction::none) {
        if (action==mir3::SpellAction::rush) {
            if (mir3::SpellTiming::elapsed_after(now,spell_timing.rush_at,3000)) {
                spell_timing.rush_at=now;send=predict=true;target={facing,0};target_actor=0;
            }
        } else if (action==mir3::SpellAction::special) {
            if (mir3::SpellTiming::elapsed_after(now,spell_timing.special_at,3000)) {
                spell_timing.special_at=now;send=predict=true;target={facing,0};target_actor=0;
            }
        } else {
            spell_timing.begin_animated(now,found->casting_delay);
            if (route.self_target) {
                target=player_cell;target_actor=std::uint32_t(local_actor_identity);followup_target=target_actor;
            }
            send=predict=true;
        }
    }
    switch (route.direct) {
    case mir3::DirectSpell::rapid: direct_tick=&spell_timing.cast_at;direct_interval=200;break;
    case mir3::DirectSpell::predicted: direct_tick=&spell_timing.cast_at;direct_interval=1000;break;
    case mir3::DirectSpell::fire_sword: direct_tick=&spell_timing.fire_sword_at;direct_interval=10000;break;
    case mir3::DirectSpell::skill35: direct_tick=&spell_timing.skill35_at;direct_interval=10000;break;
    case mir3::DirectSpell::skill34: direct_tick=&spell_timing.skill34_at;direct_interval=10000;break;
    case mir3::DirectSpell::none: break;
    }
    if (direct_tick && mir3::SpellTiming::elapsed_after(now,*direct_tick,direct_interval)) {
        *direct_tick=now;send=true;target={0,0};
        predict=route.direct==mir3::DirectSpell::predicted;
        if (route.direct==mir3::DirectSpell::rapid) spell_timing.casting_delay=0;
    }
    if (send) {
        if (predict) {
            action_prediction.begin({player_cell.x,player_cell.y},player_facing);
            map_route.clear();forced_motion.reset();player_harvest_impact=false;
            // 415F90的23分支只发请求并等待服务器冲撞，不提前播放1840动作。
            if (action!=mir3::SpellAction::none && action!=mir3::SpellAction::rush) {
                walk_frame=-1;movement_ms=0;player_facing=facing;
                if (action==mir3::SpellAction::directed || action==mir3::SpellAction::area) {
                    player_action.start_spell(static_cast<int>(action),route.action_limit);
                    if (route.action_interval) player_action.set_interval(route.action_interval);
                    player_spell={id,route.directional_effect?facing:0};
                    if (const auto* resource=effect_catalog.find(mir3::EffectCatalog::Section::spell,std::uint16_t(id))) {
                        player_spell.tint=mir3::SpellColor::strongest(elements.attack_words,resource->tint);
                        play_spell_sound(10*(id+1000),player_cell);
                    }
                } else {
                    player_action.start(static_cast<int>(action));player_spell={};
                    if (action==mir3::SpellAction::special) play_actor_sound(0,142,player_cell);
                }
            }
        }
        const mir3::SpellRequest request{std::uint16_t(id),std::uint16_t(target.x),std::uint16_t(target.y),target_actor,now};
        const auto header=request.header();
        send_client_packet(header.identifier,header.recognition,String::utf8(request.body().c_str()),
            std::uint32_t(header.parameters[0])|(std::uint32_t(header.parameters[1])<<16),header.parameters[2]);
    }
    if (route.restrict_followup) spell_timing.restricted_at=now;
    if (followup_target && (target_type==0 || target_type==1)) {
        const auto first=std::uint32_t(std::rand()),second=std::uint32_t(std::rand());
        spell_timing.finish_target(followup_target,target_type,first,second);
    } else spell_timing.movement_delay=0;
    queue_redraw();
}

int Mir3HudView::bound_skill(int key) const {
    return skills.bindings[key];
}

void Mir3HudView::draw_skill_shortcuts() {
    if (!skill_shortcuts_visible) return;
    for (int index=0;index<12;++index) {
        // 0x43A5D0: slot origins advance 40 px, plus a gap after F4 and F8.
        const int x=40*(index+index/4);
        if (const int id=bound_skill(index+1)&255;id!=0) {
            const int frame_id=999+id;
            if (!skill_frames.contains(frame_id)) {
                const auto decoded=magic_icons.frame(frame_id);
                if (decoded) skill_frames.emplace(frame_id,texture(decoded->pixels.rgba,decoded->width,decoded->height));
            }
            if (const auto it=skill_frames.find(frame_id);it!=skill_frames.end())
                paint_canvas->draw_texture(it->second,Vector2(x,2),Color(1,1,1,200.f/255));
        }
        const auto label=frame(1450+index);
        if (label.is_valid()) paint_canvas->draw_texture(label,Vector2(x-13,-11),Color(150.f/255,150.f/255,150.f/255,150.f/255));
    }
}

void Mir3HudView::draw_skill_key_menu() {
    if (!skills.visible || !skill_key_menu.has_area()) return;
    legacy_rect(skill_key_menu,Color(100.f/255,100.f/255,50.f/255,150.f/255));
    for (int index=0;index<12;++index) {
        const Vector2 point=skill_key_menu.position+Vector2(40*(index%4),40*(index/4));
        // The popup reads the shortcut table, including bindings kept by 211/212.
        if (const int id=bound_skill(index+1)&255;id!=0) {
            const int frame_id=2*id-2;
            if (!skill_frames.contains(frame_id)) {
                const auto decoded=magic_icons.frame(frame_id);
                if (decoded) skill_frames.emplace(frame_id,texture(decoded->pixels.rgba,decoded->width,decoded->height));
            }
            if (const auto it=skill_frames.find(frame_id);it!=skill_frames.end()) paint_canvas->draw_texture(it->second,point);
        }
        sprite(1660+index,{int(point.x),int(point.y)});
    }
}

void Mir3HudView::toggle_group() {
    if (frame(1360).is_null()) return;
    group.toggle();
    group_pressed=-1;
    if (group.visible) raise_panel(Panel::group);
    controller.cancel();
    queue_redraw();
}

int Mir3HudView::group_button_at(Vector2 point) {
    for (int i=0;i<5;++i) {
        const auto icon=frame(group_button_frames[i]);
        const auto [x,y]=group_button_offsets[i];
        if (icon.is_valid() && Rect2(Vector2(group.rect[0]+x,group.rect[1]+y),icon->get_size()).has_point(point))
            return i;
    }
    return -1;
}

void Mir3HudView::draw_group() {
    const auto [x,y,w,h]=group.rect;
    const auto skin=frame(1360);
    if (skin.is_valid()) sprite(1360,{x-(skin->get_width()-w)/2,y-(skin->get_height()-h)/2});
    text(group_player.utf8().get_data(),{x+52,y+15,180,15},0xdce6c8,mir3::TextFont::normal,false);
    for (int i=0;i<group_members.size();++i) {
        const int row=90+20*(i/2);
        if (row+15>192) break;
        text(group_members[i].utf8().get_data(),{x+(i%2?45:145),y+row,100,15},0xffffff,mir3::TextFont::normal,false);
    }
    text(message(group_allow?1603:1604),{x+110,y+52,140,15},0xdce6c8,mir3::TextFont::normal,false);
    for (int i=0;i<5;++i) {
        const auto [left,top]=group_button_offsets[i];
        if (i==4) sprite(group_allow?1370:1371,{x+left,y+top});
        else if (i==0 || group_pressed==i)
            sprite(group_button_frames[i]+(group_pressed==i),{x+left,y+top});
    }
}

bool Mir3HudView::group_pointer(int kind,Vector2 point) {
    if (!group.visible) return false;
    const bool inside=rect(group.rect).has_point(point);
    const int button=group_button_at(point);
    bool captured=false;
    if (kind==0 && inside) {
        raise_panel(Panel::group);
        group_pressed=button;
        group.begin_drag(int(point.x),int(point.y),button>=0);
    } else if (kind==1) {
        captured=group.move_drag(int(point.x),int(point.y));
    } else if (kind==2) {
        const bool clicked=button>=0 && button==group_pressed;
        captured=group.end_drag() || group_pressed>=0;
        group_pressed=-1;
        // 0x431F20 enables the release sound for all five group controls.
        if (clicked && hud_sound && audio_options.effects_enabled) hud_sound->play();
        if (clicked && button==0) toggle_group();
        if (clicked && button>=1 && button<=3) open_group_dialog(button);
        if (clicked && button==4) {
            group_allow=!group_allow;
            emit_signal("group_allow_requested",group_allow);
            send_client_packet(mir3::protocol::CM_GROUPMODE,0,String(),group_allow?1:0);
        }
    }
    if (!inside && !captured) return false;
    controller.cancel();
    get_viewport()->set_input_as_handled();
    queue_redraw();
    return true;
}

void Mir3HudView::set_group_roster(String player,PackedStringArray members) {
    group_player=player;
    group_members=members;
    queue_redraw();
}

bool Mir3HudView::set_local_exit_state(int action,PackedInt64Array ticks) {
    if (action<0 || action>255 || ticks.size()!=3) return false;
    for (int i=0;i<3;++i) if (ticks[i]<0 || ticks[i]>UINT32_MAX) return false;
    for (int i=0;i<3;++i) exit_guard.ticks[i]=std::uint32_t(ticks[i]);
    exit_guard.action=std::uint8_t(action);return true;
}

bool Mir3HudView::check_exit_allowed() {
    if (exit_guard.allowed(GetTickCount())) return true;
    post_chat(String::utf8(message(1407).c_str()),mir3::chat_palette(2),mir3::chat_palette(3));
    queue_redraw();return false;
}

void Mir3HudView::handle_exit_confirmation(int dialog_id) {
    if (login_visible || character_selection.visible) return;
    if ((dialog_id==101 || dialog_id==109) && !check_exit_allowed()) return;
    // 0x4256B0 / 0x47BE10: game logout sends an empty 1009; it is not WM_DESTROY.
    if (dialog_id==101) {
        if (!prepare_character_selection()) {
            UtilityFunctions::push_error("Cannot read original character selection resources.");return;
        }
        send_client_packet(mir3::protocol::CM_LOGOUT,0,String());
        save_item_layout();
        exit_transition.begin();exit_fraction_ms=0;exit_layer->queue_redraw();
    }
    if (dialog_id==109) get_tree()->quit();
}

bool Mir3HudView::dialog_is_alert() const {
    return (group_request>=1445 && group_request<=1450) || group_request==1455 || group_request==1457 || group_request==1427 || group_request==1434 || group_request==1414 || (group_request>=1473 && group_request<=1507) || (group_request>=1437 && group_request<=1439) || group_request==4000 || group_request==4017 || group_request==4714 || group_request==4716;
}

bool Mir3HudView::dialog_has_input() const {
    return group_request!=101 && group_request!=109 && !dialog_is_alert();
}

void Mir3HudView::open_group_dialog(int action) {
    if ((action==101 || action==109) && !check_exit_allowed()) return;
    const auto skin=frame(1240);
    if (skin.is_null() || !chat_input) return;
    if (group_request) finish_group_dialog(false);
    cancel_pointer_interactions();
    const int w=skin->get_width(),h=skin->get_height();
    group_dialog={400-w/2,246-h/2,w,h};
    group_request=action<=3?mir3::protocol::CM_GROUPMODE+action:action;
    group_selected=0;
    group_dialog_pressed=-1;
    queue_redraw();
    if (!dialog_has_input()) { chat_input->release_focus();return; }
    chat_input->set_position(Vector2(group_dialog[0]+24,group_dialog[1]+228));
    chat_input->set_size(Vector2(309,15));
    chat_input->focus_chat("",true);
}

void Mir3HudView::finish_group_dialog(bool confirm) {
    const int request=group_request;
    const bool has_input=dialog_has_input();
    const String name=has_input?(request==mir3::protocol::CM_MERCHANTDLGSELECT?chat_input->limited_text():chat_input->get_text()):String();
    group_request=0;
    group_dialog_pressed=-1;
    if (has_input) {
        chat_input->set_secret(false);
        chat_input->set_byte_limit(mir3::ChatInputSpec::limit);
        chat_input->focus_chat("",true);
        chat_input->release_focus();
        chat_input->layout(model.layout.expanded);
    }
    queue_redraw();
    if (request==mir3::protocol::CM_MERCHANTDLGSELECT) {
        const auto command=String::utf8(npc_dialogue_prompt_command.c_str())+"\r"+name;
        const int actor=std::exchange(npc_dialogue_prompt_actor,0);
        npc_dialogue_prompt.clear();
        npc_dialogue_prompt_command.clear();
        if (confirm && actor && actor==npc_dialogue_actor) {
            emit_signal("npc_dialogue_action",actor,command);
            emit_signal("npc_request",mir3::protocol::CM_MERCHANTDLGSELECT,actor,command);
            emit_npc_packet(mir3::protocol::CM_MERCHANTDLGSELECT,actor,command);
        }
        return;
    }
    if (request==102) {
        if (!confirm) return;
        const auto utf8=name.utf8();
        const auto amount=std::strtol(utf8.get_data(),nullptr,10);
        // 4257FF requires an amount strictly below the balance; 653 supplies the new balance.
        if (amount>0 && std::uint32_t(amount)<inventory_gold)
            send_client_packet(mir3::protocol::CM_DROPGOLD,int(amount),String());
        return;
    }
    if (request==104) {
        if (!confirm || pending_drop || !bag.held || bag.held.stack_mode<2 || bag.held.count<=1) return;
        const auto input=name.utf8();
        const long requested=name.is_empty() || name=="*"?bag.held.count:std::strtol(input.get_data(),nullptr,10);
        const int quantity=int(std::min(requested,100L));
        if (quantity<=0 || quantity>bag.held.count) return;
        const auto instance=bag.held.make_index;
        const auto item_name=String::utf8(bag.held.name.c_str());
        // 4259DF puts the original stack back for a partial drop; 1313 supplies the new count.
        if (quantity<bag.held.count) {
            if (!bag.insert(bag.held)) return;
            bag.held={};
        } else pending_drop=std::exchange(bag.held,{});
        bag.held_slot=-1;
        send_client_packet(mir3::protocol::CM_DROP_STACK,int(instance),item_name,0,std::uint16_t(quantity));
        return;
    }
    if(request==1705 || request==1706 || request==1707) {
        if(!confirm) return;
        const auto input=name.utf8();
        const std::string_view digits(input.get_data(),input.length());
        int quantity=0;
        if(digits=="*" && request==1707 && shop_offer_index>=0 && shop_offer_index<int(shop_offers.size())) {
            quantity=shop_offers[shop_offer_index].count;
        } else if(digits=="*") {
            const auto current=std::ranges::find_if(bag.items,[&](const auto& item) {
                return item && item.make_index==merchant_service.item && item.name==merchant_service.name;
            });
            if(current!=bag.items.end()) quantity=current->count;
        } else {
            // 4256B0's quantity handlers consume the decimal prefix before checking the stack size.
            const auto parsed=std::strtol(input.get_data(),nullptr,10);
            if(parsed<=0 || parsed>UINT16_MAX) return;
            quantity=int(parsed);
        }
        if(quantity>0) {
            if(request==1707) withdraw_storage(quantity);
            else confirm_merchant_service(quantity);
        }
        return;
    }
    if (request==1307) {
        const auto input=name.utf8();
        const std::string_view digits(input.get_data());
        int count=0;
        const auto end=digits.data()+digits.size();
        const auto parsed=std::from_chars(digits.data(),end,count);
        if (confirm && trade.visible && trade_pending_slot>=0 && bag.held &&
            parsed.ec==std::errc{} && parsed.ptr==end && count>0 && count<=bag.held.count)
            send_trade_offer(count);
        else trade_pending_slot=-1;
        return;
    }
    if (request==110) {
        if (!confirm || shop_bulk_index<0) return;
        const auto utf8=name.utf8();
        // 42590A reads the decimal prefix on the bulk purchase path.
        const long quantity=name.is_empty()?1:std::strtol(utf8.get_data(),nullptr,10);
        if (quantity<=0 || quantity>UINT16_MAX) return;
        buy_shop_bulk(static_cast<std::uint16_t>(quantity));
        return;
    }
    if (!confirm) return;
    if (request==101 || request==109) {
        if (!check_exit_allowed()) return;
        emit_signal("exit_confirmed",request);
        return;
    }
    if (!has_input || name.is_empty()) return;
    if (request==mir3::protocol::CM_TRADE_GOLD) {
        const auto utf8=name.utf8();
        const std::string_view digits(utf8.get_data());
        std::uint32_t amount=0;
        const auto end=digits.data()+digits.size();
        const auto parsed=std::from_chars(digits.data(),end,amount);
        if (trade.visible && !trade_confirmed && !trade_gold_pending && !trade_gold[0] &&
            parsed.ec==std::errc{} && parsed.ptr==end && amount>0 && amount<=inventory_gold) {
            trade_gold_pending=true;
            emit_signal("trade_gold_requested",mir3::protocol::CM_TRADE_GOLD,int64_t(amount));
            send_client_packet(mir3::protocol::CM_TRADE_GOLD,amount,String());
        }
    } else if (request==mir3::protocol::CM_GUILDADDMEMBER) {
        send_client_packet(request,0,name);
    } else if (request==mir3::protocol::CM_GUILDDELMEMBER) {
        emit_signal("guild_member_removal_requested",name);
        send_client_packet(mir3::protocol::CM_GUILDDELMEMBER,0,name);
        emit_signal("guild_members_requested");
        send_client_packet(mir3::protocol::CM_GUILDMEMBERLIST,0,String());
    } else if (request==mir3::protocol::CM_GUILDBREAKALLY) {
        const auto command=String::utf8(message(4016).c_str())+name;
        send_chat_command(command);
        emit_signal("guild_overview_requested");
        send_client_packet(mir3::protocol::CM_GUILDHOME,0,String());
    } else {
        emit_signal("group_member_requested",request,name);
        if (request==mir3::protocol::CM_CREATEGROUP || request==mir3::protocol::CM_ADDGROUPMEMBER || request==mir3::protocol::CM_DELGROUPMEMBER)
            send_client_packet(request,0,name);
    }
}

void Mir3HudView::draw_group_dialog() {
    const int x=group_dialog[0],y=group_dialog[1];
    sprite(1240,{x,y});
    if (dialog_is_alert()) {
        text(message(group_request),{x+23,y+84,308,95},0xc8faff,mir3::TextFont::dialog);
        sprite(1243+(group_dialog_pressed==0),{x+137,y+188});
        return;
    }
    auto prompt=group_request==mir3::protocol::CM_MERCHANTDLGSELECT?npc_dialogue_prompt:
        group_request==mir3::protocol::CM_GUILDDELMEMBER?"'"+std::string(guild_name.utf8().get_data())+"' "+message(4001):
        message(group_request==102?1415:group_request==104?1416:group_request==1707?2301:(group_request==1705 || group_request==1706)?group_request:group_request==mir3::protocol::CM_GUILDADDMEMBER?1426:group_request==110?2302:group_request==1307?1307:group_request==mir3::protocol::CM_TRADE_GOLD?1304:group_request==mir3::protocol::CM_GUILDBREAKALLY?4002:group_request==mir3::protocol::CM_DELGROUPMEMBER?1606:1605);
    if(group_request==mir3::protocol::CM_GUILDADDMEMBER) if(auto at=prompt.find("%s");at!=std::string::npos) prompt.replace(at,2,guild_name.utf8().get_data());
    if (dialog_has_input()) text(prompt,{x+43,y+84,278,75},0xc8faff,mir3::TextFont::dialog);
    else text(message(group_request==109?2636:1065),{x+23,y+84,308,95},0xc8faff,mir3::TextFont::dialog);
    for (const int index:{0,2}) {
        const int base=index==0?1241:1245;
        const auto position=mir3::HudLayout::Point{x+(index==0?41:234),y+188};
        const bool pressed=group_dialog_pressed==index;
        sprite(base+pressed,position);
        // 0x420840 redraws the selected button's normal frame unless pressed.
        if (group_selected==index && !pressed) sprite(base,position);
    }
}

bool Mir3HudView::group_dialog_input(const Ref<InputEvent>& event) {
    Ref<InputEventKey> key=event;
    if (key.is_valid()) {
        const auto code=key->get_keycode();
        if (dialog_is_alert()) {
            if (key->is_pressed() && !key->is_echo() && (code==KEY_ENTER || code==KEY_KP_ENTER || code==KEY_SPACE))
                finish_group_dialog(false);
            get_viewport()->set_input_as_handled();
            return true;
        }
        const bool activate=code==KEY_ENTER || code==KEY_KP_ENTER || (code==KEY_SPACE && (!dialog_has_input() || !chat_input->has_focus()));
        if (code!=KEY_TAB && code!=KEY_ESCAPE && !activate && dialog_has_input()) return false;
        if (key->is_pressed() && !key->is_echo()) {
            if (code==KEY_TAB) { group_selected=group_selected==0?2:0;queue_redraw(); }
            if (activate) finish_group_dialog(group_selected==0);
        }
        get_viewport()->set_input_as_handled();
        return true;
    }
    const bool alert=dialog_is_alert();
    const auto hit_button=[&](Vector2 point) {
        for (const int index:{0,2}) {
            if (alert && index==2) continue;
            const auto icon=frame(alert?1243:index==0?1241:1245);
            const Vector2 pos(group_dialog[0]+(alert?137:index==0?41:234),group_dialog[1]+188);
            if (icon.is_valid() && Rect2(pos,icon->get_size()).has_point(point)) return index;
        }
        return -1;
    };
    Ref<InputEventMouseMotion> motion=event;
    if (motion.is_valid() && group_dialog_pressed>=0 &&
        hit_button(motion->get_position())!=group_dialog_pressed) {
        group_dialog_pressed=-1;
        queue_redraw();
    }
    Ref<InputEventMouseButton> mouse=event;
    if (mouse.is_valid()) {
        if (dialog_has_input() && group_dialog_pressed<0 && chat_input->get_rect().has_point(mouse->get_position())) return false;
        if (mouse->get_button_index()==MOUSE_BUTTON_LEFT) {
            const int hit=hit_button(mouse->get_position());
            if (mouse->is_pressed()) group_dialog_pressed=hit;
            else {
                const bool clicked=hit>=0 && hit==group_dialog_pressed;
                group_dialog_pressed=-1;
                if (clicked) {
                    // 0x420BB0 enables sound for mouse releases. 0x420A10 handles keys separately.
                    if (hud_sound && audio_options.effects_enabled) hud_sound->play();
                    finish_group_dialog(hit==0);
                }
            }
            queue_redraw();
        }
    }
    get_viewport()->set_input_as_handled();
    return true;
}

bool Mir3HudView::apply_guild_overview_reply(int identifier,PackedByteArray body) {
    if (body.is_empty() || body.size()>8191) return false;
    const mir3::GuildReply reply{identifier,std::string(reinterpret_cast<const char*>(body.ptr()),body.size())};
    const bool initial=identifier==mir3::protocol::SM_OPENGUILDDLG;
    if (initial ? !guild_overview.receive(reply) :
        !guild_overview_started || !guild_overview.receive_continuation(reply)) return false;

    PackedStringArray lines,editable;
    for (const auto& row:guild_overview.overview)
        lines.append(String::utf8((row.message?message(row.message):mir3::ansi_to_utf8(row.ansi)).c_str()));
    for (const auto& row:guild_overview.editable)
        editable.append(String::utf8(mir3::ansi_to_utf8(row).c_str()));
    if (initial) {
        set_guild_overview(String::utf8(mir3::ansi_to_utf8(guild_overview.name).c_str()),lines,editable);
        guild_overview_started=true;
        guild_manager=guild_overview.manager;
    } else {
        guild_pages[0]=lines;
        guild_editable[0]=editable;
        queue_redraw();
    }
    return true;
}

void Mir3HudView::set_guild_overview(String name,PackedStringArray lines,PackedStringArray editable) {
    guild_overview_started=false;
    guild_editable[0]=editable;
    guild_name=name;
    guild_pages[0]=lines;
    guild_layout.tab=0;
    guild_first=0;
    if (!guild.visible) raise_panel(Panel::guild);
    guild.visible=true;
    queue_redraw();
}

void Mir3HudView::draw_guild() {
    const auto [x,y,w,h]=guild.rect;
    const auto skin=frame(1300);
    if (skin.is_valid()) sprite(1300,{x-(skin->get_width()-w)/2,y-(skin->get_height()-h)/2});
    // 0x432A40 draws the guild name before dispatching the active tab header.
    constexpr unsigned heading_color=0x96c8ff; // COLORREF, as expected by text().
    text(guild_name.utf8().get_data(),{x+340,y+10,213,24},heading_color);
    text(message(mir3::GuildLayout::title(guild_layout.tab)),{x+45,y+10,112,25},heading_color);
    const auto& lines=guild_pages[guild_layout.tab];
    const int count=int(lines.size());
    const auto metrics=mir3::raster_text(guild_name.utf8().get_data(),300,24,false,mir3::TextFont::normal);
    for (int row=0;row<18 && guild_first+row<count;++row) {
        auto bounds=mir3::GuildLayout::notice_rect(row,metrics.text_height);
        bounds[0]+=x;
        bounds[1]+=y;
        const std::string value=lines[guild_first+row].utf8().get_data();
        if (guild_layout.tab==2) {
            auto shadow=bounds;
            --shadow[0];
            --shadow[1];
            text(value,shadow,0x0a140a,mir3::TextFont::normal,false);
            text(value,bounds,0x00ff00,mir3::TextFont::normal,false);
        } else {
            const bool heading=guild_layout.tab==1?value==message(4007):
                value==message(4004) || value==message(4005) || value==message(4006);
            text(value,bounds,heading?38655:0xffffff,mir3::TextFont::normal,false);
        }
    }
    sprite(1225,{x+557,mir3::GuildScroll::thumb_y(guild_first,count,y)});
    for (int i=0;i<9;++i) {
        const auto spec=mir3::GuildLayout::buttons[i];
        sprite(guild_layout.frames(i)[0],{x+spec[1],y+spec[2]});
    }
}

bool Mir3HudView::guild_pointer(int kind,Vector2 point) {
    if (!guild.visible) return false;
    const auto [x,y,w,h]=guild.rect;
    const bool inside=rect(guild.rect).has_point(point);
    const auto thumb=frame(1225);
    const auto tick=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    const auto& lines=guild_pages[guild_layout.tab];
    const int count=int(lines.size());
    bool captured=thumb.is_valid() && guild_scroll.input(kind,int(point.x),int(point.y),x,y,
        thumb->get_width(),thumb->get_height(),count,guild_first,static_cast<std::uint32_t>(tick));
    if (!captured) {
        int hit=-1;
        for (int i=0;i<9;++i) {
            const auto spec=mir3::GuildLayout::buttons[i];
            const auto icon=frame(spec[0]);
            if (icon.is_valid() && Rect2(Vector2(x+spec[1],y+spec[2]),icon->get_size()).has_point(point)) { hit=i;break; }
        }
        if (kind==0 && inside) {
            guild_layout.pressed=hit;
            if (guild.begin_drag(int(point.x),int(point.y),hit>=0)) raise_panel(Panel::guild);
        } else if (kind==1) {
            captured=guild.move_drag(int(point.x),int(point.y));
        } else if (kind==2) {
            captured=guild.end_drag() || guild_layout.pressed>=0;
            if (guild_layout.release(hit)) {
                // 0x433420 enables the 0x41FCD0 release sound for guild buttons.
                if (hud_sound && audio_options.effects_enabled) hud_sound->play();
                if (hit==0) guild.toggle();
                if (hit>=1 && hit<=3) guild_first=0;
                if (hit==1) { emit_signal("guild_overview_requested");send_client_packet(mir3::protocol::CM_GUILDHOME,0,String()); }
                if (hit==2) { emit_signal("guild_members_requested");send_client_packet(mir3::protocol::CM_GUILDMEMBERLIST,0,String()); }
                if (mir3::GuildLayout::denial(hit,guild_manager)==4000) open_group_dialog(4000);
                else if (hit==4 || hit==7) open_guild_editor(hit==4);
                else if (hit==5) open_group_dialog(mir3::protocol::CM_GUILDDELMEMBER);
                else if (hit==8) open_group_dialog(mir3::protocol::CM_GUILDBREAKALLY);
            }
        }
    }
    if (!inside && !captured) return false;
    controller.cancel();
    get_viewport()->set_input_as_handled();
    queue_redraw();
    return true;
}

bool Mir3HudView::apply_guild_members_reply(int identifier,PackedByteArray body) {
    if (body.size()>8191) return false;
    mir3::GuildReply reply{identifier,{}};
    if (!body.is_empty()) reply.ansi.assign(reinterpret_cast<const char*>(body.ptr()),body.size());
    mir3::GuildMembers members;
    if (!members.receive(reply,guild_manager)) return false;
    PackedStringArray lines,editable;
    for (const auto& row:members.members)
        lines.append(String::utf8((row.message?message(row.message):mir3::ansi_to_utf8(row.ansi)).c_str()));
    for (const auto& row:members.editable_members)
        editable.append(String::utf8(mir3::ansi_to_utf8(row).c_str()));
    set_guild_members(lines,editable);
    return true;
}

void Mir3HudView::set_guild_members(PackedStringArray lines,PackedStringArray editable) {
    guild_editable[1]=editable;
    guild_pages[1]=lines;
    if (guild_layout.tab==1) guild_first=std::min(guild_first,std::max(0,int(lines.size())-1));
    queue_redraw();
}

void Mir3HudView::post_system_notice(String value,int64_t foreground,int64_t background) {
    post_chat(value,foreground,background);
    auto& notices=guild_pages[2];
    const auto utf8=value.utf8();
    for (const auto& line:mir3::wrap_system_notice({utf8.get_data(),static_cast<std::size_t>(utf8.length())})) {
        if (notices.size()-guild_first==18) ++guild_first;
        notices.append(String::utf8(line.c_str()));
    }
    while (notices.size()>=80) {
        notices.remove_at(0);
        if (guild_first>0) --guild_first;
    }
}

void Mir3HudView::set_guild_notices(PackedStringArray lines) {
    guild_pages[2]=lines;
    if (guild_layout.tab==2) guild_first=std::min(guild_first,std::max(0,int(lines.size())-1));
    queue_redraw();
}

void Mir3HudView::set_guild_manager(bool allowed) { guild_manager=allowed; }

void Mir3HudView::open_guild_editor(bool announcement) {
    if (!announcement && guild_pages[1].is_empty()) {
        open_group_dialog(4017);
        return;
    }
    if (frame(1326).is_null()) return;
    if (!guild_edit) {
        guild_edit=memnew(TextEdit);
        guild_edit->set_z_index(500);guild_edit->set_name("GuildEdit");
        guild_edit->set_line_wrapping_mode(TextEdit::LINE_WRAPPING_BOUNDARY);
        guild_edit->add_theme_font_override("font",chat_input->get_theme_font("font"));
        guild_edit->add_theme_font_size_override("font_size",12);
        for (const auto* style:{"normal","focus","read_only"}) {
            Ref<StyleBoxEmpty> empty;
            empty.instantiate();
            guild_edit->add_theme_stylebox_override(style,empty);
        }
        add_child(guild_edit);
        guild_edit->connect("text_changed",callable_mp(this,&Mir3HudView::guild_editor_changed));
    }
    guild_edit_announcement=announcement;
    const auto& editable=guild_editable[announcement?0:1];
    String initial;
    if (!editable.is_empty()) initial=String("\n").join(editable)+String("\n");
    guild_edit_ansi=mir3::multiline_to_ansi(initial.utf8().get_data());
    guild_edit->set_text(String::utf8(mir3::ansi_to_utf8(guild_edit_ansi).c_str()).replace("\r\n","\n"));
    guild_editor.visible=true;
    guild_edit_pressed=-1;
    guild_edit->set_position(Vector2(guild_editor.rect[0]+46,guild_editor.rect[1]+54));
    guild_edit->set_size(Vector2(468,170));
    guild_edit->show();
    guild_edit->grab_focus();
    queue_redraw();
}

void Mir3HudView::guild_editor_changed() {
    guild_edit_ansi=mir3::edit_multiline_ansi(guild_edit_ansi,guild_edit->get_text().utf8().get_data());
    const auto accepted=String::utf8(mir3::ansi_to_utf8(guild_edit_ansi).c_str()).replace("\r\n","\n");
    if (accepted!=guild_edit->get_text()) guild_edit->set_text(accepted);
}

void Mir3HudView::draw_guild_editor() {
    const auto [x,y,w,h]=guild_editor.rect;
    const auto skin=frame(1326);
    if (skin.is_valid()) sprite(1326,{x-(skin->get_width()-w)/2,y-(skin->get_height()-h)/2});
    const auto label=[&](int id,int left,int top,unsigned color) {
        text(message(id),{x+left,y+top,420,16},color,mir3::TextFont::normal,false);
    };
    // 0x45EF10 selects a 9-point regular font from CMList 1074 for the title.
    text(message(guild_edit_announcement?1903:1900),{x+70,y+20,420,16},
        0x96c8ff,mir3::TextFont::localized_normal,false);
    if (guild_edit_announcement) {
        label(1904,154,21,0x323232);
        label(1904,154,22,0x96c8ff);
    } else {
        label(1901,154,14,0x96c8ff);
        label(1902,154,28,0x96c8ff);
    }
    sprite(1221+(guild_edit_pressed==0),{x+546,y+212});
    sprite(1327+(guild_edit_pressed==1),{x+461,y+12});
}

bool Mir3HudView::guild_editor_input(const Ref<InputEvent>& event) {
    Ref<InputEventMouseMotion> motion=event;
    if (motion.is_valid()) {
        const auto pos=motion->get_position();
        if (guild_editor.move_drag(int(pos.x),int(pos.y))) {
            guild_edit->set_position(Vector2(guild_editor.rect[0]+46,guild_editor.rect[1]+54));
            queue_redraw();
        }
        return false;
    }
    Ref<InputEventMouseButton> mouse=event;
    if (mouse.is_null()) return false;
    const auto pos=mouse->get_position();
    if (guild_edit->get_rect().has_point(pos) && !guild_editor.dragging && guild_edit_pressed<0) return false;
    const auto [x,y,w,h]=guild_editor.rect;
    int hit=-1;
    for (const int i:{0,1}) {
        const auto icon=frame(i==0?1221:1327);
        const Vector2 origin(x+(i==0?546:461),y+(i==0?212:12));
        if (icon.is_valid() && Rect2(origin,icon->get_size()).has_point(pos)) hit=i;
    }
    if (mouse->get_button_index()==MOUSE_BUTTON_LEFT) {
        if (mouse->is_pressed()) {
            guild_edit_pressed=hit;
            guild_editor.begin_drag(int(pos.x),int(pos.y),hit>=0);
        } else {
            guild_editor.end_drag();
            if (hit>=0 && hit==guild_edit_pressed) {
                // 0x45F100 releases both buttons with sound enabled in 0x41FCD0.
                if (hud_sound && audio_options.effects_enabled) hud_sound->play();
                if (hit==1) guild_editor_changed();
                guild_editor.visible=false;
                guild_edit->hide();
                guild_edit->release_focus();
                if (hit==1) {
                    std::string body;
                    std::string_view input(guild_edit_ansi);
                    input=input.substr(0,3999);
                    while (!input.empty()) {
                        const auto end=input.find('\n');
                        const auto line=input.substr(0,end);
                        if (end!=input.npos && !line.empty() && line.front()=='\r') body+=" \r";
                        else body+=line;
                        if (end==input.npos) break;
                        input.remove_prefix(end+1);
                    }
                    if (!body.empty() && body.size()<=3999) {
                        const int identifier=guild_edit_announcement?mir3::protocol::CM_GUILDUPDATENOTICE:mir3::protocol::CM_GUILDUPDATERANKINFO;
                        PackedByteArray bytes;
                        bytes.resize(body.size());
                        std::copy(body.begin(),body.end(),bytes.ptrw());
                        emit_signal("guild_edit_bytes_requested",identifier,bytes);
                        emit_signal("guild_edit_requested",identifier,String::utf8(mir3::ansi_to_utf8(body).c_str()));
                        send_client_packet(identifier,0,String::utf8(mir3::ansi_to_utf8(body).c_str()));
                    }
                }
            }
            guild_edit_pressed=-1;
        }
        queue_redraw();
    }
    get_viewport()->set_input_as_handled();
    return true;
}

void Mir3HudView::toggle_options() {
    if (frame(1290).is_null()) return;
    options.toggle();
    if (options.visible) raise_panel(Panel::options);
    system_options.cancel_buttons();
    audio_drag=-1;
    controller.cancel();
    queue_redraw();
}

bool Mir3HudView::apply_options_state(PackedByteArray flags,int attack_mode) {
    if (flags.size()!=7 || attack_mode<0 || attack_mode>4) return false;
    for (int i=0;i<7;++i) if (flags[i]>1) return false;
    for (int i=0;i<7;++i) system_options.confirmed[i]=system_options.displayed[i]=flags[i]!=0;
    system_options.attack_mode=attack_mode;
    system_options.cancel_buttons();
    queue_redraw();return true;
}

void Mir3HudView::draw_options() {
    const int x=options.rect[0],y=options.rect[1];
    sprite(1290,{x,y});
    const auto draw_toggle=[&](int index,bool value,mir3::HudLayout::Point position) {
        if (system_options.button_states[index]==1) sprite(value?1293:1294,position);
        sprite(system_options.toggle_frame(index,value),position);
    };
    for (int i=0;i<2;++i) {
        const bool enabled=i==0?audio_options.music_enabled:audio_options.effects_enabled;
        const int level=i==0?audio_options.music_level:audio_options.effects_level;
        draw_toggle(1+i,enabled,{x+430,y+85+60*i});
        sprite(1291,{x+276+mir3::AudioOptions::knob_from_level(level),y+118+60*i});
    }
    draw_toggle(3,audio_options.reverse_pan,{x+430,y+205});
    sprite(system_options.button_states[0]==2?1222:1221,{x+448,y+415});
    // 0x462850 draws the audio captions after their controls.
    text(message(2104),{x+60,y+30,390,15},0xf8fafa);
    for (int i=0;i<3;++i)
        text(message(2100+i),{x+266,y+89+60*i,156,21},0x0096ff);
    for (int i=0;i<10;++i)
        text(message(2105+i),{x+20,y+62+30*i,155,15},0x0096ff);
    const std::array local{system_options.show_mon,system_options.force_attack,magic_light,damage_numbers};
    for (int i=0;i<4;++i) draw_toggle(7+i,local[i],{x+186,y+55+30*i});
    for (int i=0;i<7;++i) draw_toggle(i<4?11+i:(i<6?12+i:18),system_options.displayed[i],
        {x+system_options.toggle_x[i],y+system_options.toggle_y[i]});
    draw_toggle(4,system_options.shadow_blend,{x+430,y+235});
    draw_toggle(20,system_options.user_photos,{x+430,y+295});
    sprite(system_options.button_states[19]==2?1297:1296,{x+430,y+265});
    text(message(2115),{x+266,y+59,156,21},0x0096ff);
    text(message(2103),{x+266,y+239,156,21},0x0096ff);
    text(message(2116+system_options.attack_mode),{x+266,y+269,156,21},0x0096ff);
    text(message(2130),{x+266,y+300,156,20},0x0096ff);
}

bool Mir3HudView::options_pointer(int kind,Vector2 point) {
    if (!options.visible) return false;
    const Vector2 local=point-Vector2(options.rect[0],options.rect[1]);
    const bool inside=rect(options.rect).has_point(point);
    const bool captured=options.dragging || audio_drag>=0;
    if (!inside && !captured) {
        system_options.cancel_buttons();queue_redraw();return false;
    }
    const auto hit=[&](int id,int x,int y) {
        const auto image=frame(id);
        return image.is_valid() && Rect2(Vector2(x,y),image->get_size()).has_point(local);
    };
    int button=-1;
    if (hit(1221,448,415)) button=0;
    for (int i=0;i<2;++i) {
        if (hit(1292,430,85+60*i)) button=1+i;
        const int level=i==0?audio_options.music_level:audio_options.effects_level;
        if (hit(1291,276+mir3::AudioOptions::knob_from_level(level),118+60*i)) button=5+i;
    }
    if (hit(1292,430,205)) button=3;
    if (hit(1292,430,235)) button=4;
    for (int i=0;i<4;++i) if (hit(1292,186,55+30*i)) button=7+i;
    for (int i=0;i<7;++i) if (hit(1292,system_options.toggle_x[i],system_options.toggle_y[i]))
        button=i<4?11+i:(i<6?12+i:18);
    if (hit(1296,430,265)) button=19;
    if (hit(1292,430,295)) button=20;
    if (kind==0) system_options.press(button);
    if (kind==1 && !captured) system_options.motion(button);
    if (kind==2) system_options.release(button);
    if (hud_sound && audio_options.effects_enabled &&
        ((kind==0 && (button==5 || button==6)) ||
         (kind==2 && !captured && button>=0 && button!=5 && button!=6))) hud_sound->play();
    bool changed=false;
    if (audio_drag>=0) {
        if (kind==1) {
            audio_options.drag_volume(audio_drag==0,int(local.x));
            sync_hud_sound();
            save_options();
        }
        if (kind==2) audio_drag=-1;
    } else if (options.dragging && kind!=0) {
        if (kind==1) options.move_drag(int(point.x),int(point.y));
        if (kind==2) options.end_drag();
    } else if (kind!=1) {
        bool control=false;
        if (hit(1221,448,415)) {
            control=true;
            if (kind==2) toggle_options();
        }
        const auto toggle=[&](bool& value,int x,int y) {
            if (!hit(1292,x,y)) return;
            control=true;
            if (kind==2) { value=!value;changed=true; }
        };
        const auto local_toggle=[&](bool& value,int x,int y,int enabled_message,int disabled_message) {
            const bool previous=value;toggle(value,x,y);
            if (value!=previous && enabled_message)
                post_chat(String::utf8(message(value?enabled_message:disabled_message).c_str()),mir3::chat_palette(2),mir3::chat_palette(7));
        };
        local_toggle(system_options.show_mon,186,55,2646,2647);
        local_toggle(system_options.force_attack,186,85,1515,1516);
        local_toggle(magic_light,186,115,1518,1517);
        local_toggle(damage_numbers,186,145,2122,2121);
        local_toggle(system_options.shadow_blend,430,235,0,0);
        local_toggle(system_options.user_photos,430,295,2132,2131);
        for (int i=0;i<7;++i) if (hit(1292,system_options.toggle_x[i],system_options.toggle_y[i])) {
            control=true;
            if (kind==2) send_chat_command(String::utf8(message(system_options.request(i)).c_str()));
        }
        if (hit(1296,430,265)) {
            control=true;if (kind==2) send_chat_command(String::utf8(message(1422).c_str()));
        }
        toggle(audio_options.reverse_pan,430,205);
        for (int i=0;i<2;++i) {
            toggle(i==0?audio_options.music_enabled:audio_options.effects_enabled,430,85+60*i);
            int& level=i==0?audio_options.music_level:audio_options.effects_level;
            const int offset=mir3::AudioOptions::knob_from_level(level);
            if (hit(1291,276+offset,118+60*i)) {
                control=true;
                if (kind==0) audio_drag=i;
            } else if (local.x>266 && local.x<487 && local.y>116+60*i && local.y<136+60*i) {
                control=true;
                if (kind==2) {
                    level=mir3::AudioOptions::level_from_knob(mir3::AudioOptions::track_step(offset,int(local.x)));
                    changed=true;
                }
            }
        }
        if (!control && kind==0 && options.begin_drag(int(point.x),int(point.y),false)) raise_panel(Panel::options);
    }
    if (changed) { sync_hud_sound();save_options(); }
    controller.cancel();
    get_viewport()->set_input_as_handled();
    queue_redraw();
    return true;
}

void Mir3HudView::load_options() {
    const auto path=options_path.utf16();
    const auto* filename=reinterpret_cast<const wchar_t*>(path.get_data());
    const auto read=[filename](const wchar_t* key,int fallback) {
        return static_cast<int>(GetPrivateProfileIntW(L"Options",key,fallback,filename));
    };
    audio_options.music_enabled=read(L"BGM",1)!=0;
    audio_options.effects_enabled=read(L"EffectSound",1)!=0;
    audio_options.reverse_pan=read(L"ReversePan",0)==1;
    audio_options.music_level=mir3::AudioOptions::normalized_level(read(L"BGMLevel",0));
    audio_options.effects_level=mir3::AudioOptions::normalized_level(read(L"EffectSoundLevel",0));
    system_options.show_mon=read(L"ShowMon",1)!=0;
    system_options.force_attack=read(L"AttackUser",1)!=0;
    system_options.shadow_blend=read(L"ShadowBlend",0)!=0;
    system_options.user_photos=read(L"UserPic",1)!=0;
    magic_light=read(L"MagicLight",1)!=0;
    damage_numbers=read(L"BloodNum",1)!=0;
    portrait_map_mode=read(L"InformationMode",0)!=0;
    portrait_map_pending=portrait_map_mode;
}

void Mir3HudView::save_options() {
    const auto path=options_path.utf16();
    const auto* filename=reinterpret_cast<const wchar_t*>(path.get_data());
    const auto write=[filename](const wchar_t* key,int value) {
        const auto text=std::to_wstring(value);
        return WritePrivateProfileStringW(L"Options",key,text.c_str(),filename)!=0;
    };
    bool saved=write(L"BGM",audio_options.music_enabled);
    saved=write(L"EffectSound",audio_options.effects_enabled) && saved;
    saved=write(L"ReversePan",audio_options.reverse_pan) && saved;
    saved=write(L"BGMLevel",audio_options.music_level) && saved;
    saved=write(L"EffectSoundLevel",audio_options.effects_level) && saved;
    saved=write(L"ShowMon",system_options.show_mon) && saved;
    saved=write(L"AttackUser",system_options.force_attack) && saved;
    saved=write(L"ShadowBlend",system_options.shadow_blend) && saved;
    saved=write(L"UserPic",system_options.user_photos) && saved;
    saved=write(L"MagicLight",magic_light) && saved;
    saved=write(L"BloodNum",damage_numbers) && saved;
    if (!saved) UtilityFunctions::push_warning("Could not save system options to ",options_path);
}

void Mir3HudView::remove_actor_sound(std::int64_t actor) {
    auto found=actor_sounds.find(actor);
    if (found==actor_sounds.end()) return;
    found->second.player->stop();
    found->second.player->set_stream(Ref<AudioStream>());
    found->second.player->set_bus("Master");
    found->second.player->queue_free();
    auto* audio=AudioServer::get_singleton();
    const int bus=audio->get_bus_index(found->second.bus);
    if (bus>=0) audio->remove_bus(bus);
    actor_sounds.erase(found);
}

void Mir3HudView::play_actor_sound(std::int64_t actor,int sample,Vector2i cell) {
    if (!sample || !audio_options.effects_enabled) return;
    const auto camera=map_camera();
    const auto mix=mir3::SoundPosition::from_view(cell.x*48-camera.x,cell.y*32-camera.y);
    const int volume=mix.volume+mir3::AudioOptions::volume(audio_options.effects_level);
    if (volume<=-10000) return;
    auto stream=movement_sounds.find(sample);
    if (stream==movement_sounds.end()) {
        const auto name=sound_list.file(sample);
        if (name.empty()) return;
        const auto path=audio_root.path_join(String::utf8(name.c_str()));
        if (!FileAccess::file_exists(path)) return;
        stream=movement_sounds.emplace(sample,AudioStreamWAV::load_from_file(path)).first;
    }
    if (stream->second.is_null()) return;
    auto* audio=AudioServer::get_singleton();
    auto& sound=actor_sounds[actor];
    if (!sound.player) {
        sound.player=memnew(AudioStreamPlayer);
        add_child(sound.player);
        sound.bus=String("Mir3Actor")+String::num_uint64(get_instance_id())+"_"+String::num_int64(actor);
        const int bus=audio->get_bus_count();
        audio->add_bus(bus);
        audio->set_bus_name(bus,sound.bus);
        Ref<AudioEffectPanner> panner;
        panner.instantiate();
        audio->add_bus_effect(bus,panner);
        sound.player->set_bus(sound.bus);
    }
    const int bus=audio->get_bus_index(sound.bus);
    const Ref<AudioEffectPanner> panner=audio->get_bus_effect(bus,0);
    sound.pan=mix.pan*(audio_options.reverse_pan?-1:1);
    const float magnitude=1.f-std::pow(10.f,-std::abs(sound.pan)/2000.f);
    panner->set_pan((sound.pan<0?-1.f:1.f)*magnitude);
    audio->set_bus_volume_db(bus,mir3::AudioOptions::volume(audio_options.effects_level)/100.f);
    sound.player->set_volume_db(mix.volume/100.f);
    sound.player->set_stream(stream->second);
    sound.player->play();
    sound.sample=sample;
    sound.volume=volume;
    ++sound.count;
}

std::int64_t Mir3HudView::play_spell_sound(int sample,Vector2i cell) {
    if (!audio_options.effects_enabled) return 0;
    const auto name=sound_list.file(sample);
    if (name.empty() || !FileAccess::file_exists(audio_root.path_join(String::utf8(name.c_str())))) return 0;
    const auto selected=spell_sound_voices.select(sample,std::uint32_t(ticks_msec()),[&](std::int64_t key) {
        const auto sound=actor_sounds.find(key);
        return sound!=actor_sounds.end() && sound->second.player->is_playing();
    });
    if (!selected) return 0;
    if (selected->replaced)
        for(int i=0;i<mir3::SpellSoundVoices::voices;++i) remove_actor_sound(mir3::SpellSoundVoices::key(selected->cache,i));
    if (selected->play) play_actor_sound(selected->key,sample,cell);
    return selected->key;
}

void Mir3HudView::position_spell_sound(std::int64_t key,int sample,Vector2i cell) {
    const auto found=actor_sounds.find(key);
    if (found==actor_sounds.end() || found->second.sample!=sample) return;
    auto& sound=found->second;
    const auto camera=map_camera();
    const auto mix=mir3::SoundPosition::from_view(cell.x*48-camera.x,cell.y*32-camera.y);
    auto* audio=AudioServer::get_singleton();
    const int bus=audio->get_bus_index(sound.bus);
    if (bus<0) return;
    const Ref<AudioEffectPanner> panner=audio->get_bus_effect(bus,0);
    sound.pan=mix.pan*(audio_options.reverse_pan?-1:1);
    panner->set_pan((sound.pan<0?-1.f:1.f)*(1.f-std::pow(10.f,-std::abs(sound.pan)/2000.f)));
    sound.volume=mix.volume+mir3::AudioOptions::volume(audio_options.effects_level);
    sound.player->set_volume_db(mix.volume/100.f);
}

void Mir3HudView::sync_hud_sound() {
    if (gold_sound) {
        gold_sound->set_volume_db(mir3::AudioOptions::volume(audio_options.effects_level)/100.f);
        if (!audio_options.effects_enabled) gold_sound->stop();
    }
    for (auto& [file,player]:login_effects) {
        player->set_volume_db(mir3::AudioOptions::volume(audio_options.effects_level)/100.f);
        if (!audio_options.effects_enabled) player->stop();
    }
    if (login_movie_audio) {
        login_movie_audio->set_volume_db(mir3::AudioOptions::volume(audio_options.effects_level)/100.f);
        if (!audio_options.effects_enabled) login_movie_audio->stop();
    }
    for (auto& [actor,sound]:actor_sounds) {
        auto* audio=AudioServer::get_singleton();
        const int bus=audio->get_bus_index(sound.bus);
        if (bus>=0) audio->set_bus_volume_db(bus,mir3::AudioOptions::volume(audio_options.effects_level)/100.f);
        if (!audio_options.effects_enabled) sound.player->stop();
    }
    if (footstep_sound) {
        auto* audio=AudioServer::get_singleton();
        const int bus=audio->get_bus_index(footstep_bus);
        if (bus>=0) audio->set_bus_volume_db(bus,mir3::AudioOptions::volume(audio_options.effects_level)/100.f);
        if (!audio_options.effects_enabled) footstep_sound->stop();
    }
    if (hud_sound) {
        hud_sound->set_volume_db(mir3::AudioOptions::volume(audio_options.effects_level)/100.f);
        if (!audio_options.effects_enabled) hud_sound->stop();
    }
    if (map_music) {
        map_music->set_volume_db(mir3::AudioOptions::volume(audio_options.music_level)/100.f);
        if (!audio_options.music_enabled) map_music->stop();
        else if (map_music->get_stream().is_valid() && !map_music->is_playing()) map_music->play();
    }
}

bool Mir3HudView::set_quest_entries(PackedStringArray titles,PackedInt32Array completed) {
    if (titles.size()!=completed.size() || titles.size()>INT_MAX) return false;
    quest.entries.clear();
    quest.entries.reserve(titles.size());
    for (int64_t i=0;i<titles.size();++i)
        quest.entries.push_back({titles[i].utf8().get_data(),completed[i]!=0});
    quest.list_initialized=false;
    quest.reset_list_view();
    queue_redraw();
    return true;
}

bool Mir3HudView::set_quest_details(int index,PackedStringArray lines) {
    if (index<0 || index>=int(quest.entries.size()) || lines.size()>INT_MAX-int(quest.entries.size())) return false;
    auto& entry=quest.entries[index];
    entry.detail=mir3::QuestPanel::Entry::Detail::received;
    auto& details=entry.details;
    details.clear();
    details.reserve(lines.size());
    for (int64_t i=0;i<lines.size();++i) {
        const auto utf8=lines[i].utf8();
        auto rows=mir3::wrap_quest_detail({utf8.get_data(),static_cast<std::size_t>(utf8.length())});
        for (auto& row:rows) details.push_back(std::move(row));
    }
    quest.scroll(0);
    queue_redraw();
    return true;
}

bool Mir3HudView::fail_quest_detail_request(int index) {
    if (index<0 || index>=int(quest.entries.size())) return false;
    auto& detail=quest.entries[index].detail;
    using Detail=mir3::QuestPanel::Entry::Detail;
    if (detail!=Detail::pending) return false;
    detail=Detail::idle;
    return true;
}

void Mir3HudView::toggle_quest() {
    quest.toggle();
    quest.scroll_captured=false;
    quest.pressed=-1;
    if (quest.visible) {
        raise_panel(Panel::quest);
        if (!quest.list_initialized && quest.entries.empty())
            send_client_packet(mir3::protocol::CM_QUERYQUESTLIST,0,String());
    }
    queue_redraw();
}

void Mir3HudView::draw_quest() {
    const auto [x,y,w,h]=quest.rect;
    const auto skin=frame(quest.background);
    if (skin.is_valid()) {
        const Vector2 anchor(x-(skin->get_width()-w)/2,y-(skin->get_height()-h)/2);
        paint_canvas->draw_texture(skin,anchor,Color(1,1,1,220.f/255));
    }
    sprite(1225,{x+quest.scroll_x,quest.thumb_y()});
    if (!quest.entries.empty()) {
        for (int row=0;row<quest.visible_rows && row<=quest.last_row()-quest.first;++row) {
            const int line=quest.first+row;
            const int index=quest.entry_at(line);
            const int top=y+90+15*row;
            if (index>=0) {
                text(quest.entries[index].title,{x+65,top,200,15},
                    quest.title_color(index),mir3::TextFont::normal,false);
            } else {
                const auto& entry=quest.entries[quest.expanded];
                const int child=line-quest.expanded-1;
                const auto& detail=entry.wire?entry.children[child].title:entry.details[child];
                const auto color=entry.wire?
                    mir3::QuestPanel::title_color(entry.children[child].completed,
                        quest.description_parent==quest.expanded && quest.description_child==child):
                    quest.title_color(quest.expanded);
                text(detail,{x+75,top,190,15},color,mir3::TextFont::normal,false);
            }
        }
    }
    const auto description=mir3::wrap_quest_detail(quest.description());
    for(int row=0;row<3 && row+quest.description_first<int(description.size());++row)
        text(description[row+quest.description_first],{x+84,y+347+15*row,165,15},0x7d0000,mir3::TextFont::normal,false);
    for (int index=0;index<int(quest.buttons.size());++index) {
        const auto [id,left,top]=quest.buttons[index];
        sprite(id+(index==quest.pressed),{x+left,y+top});
    }
}

bool Mir3HudView::quest_pointer(int kind,Vector2 position) {
    if (!quest.visible) return false;
    const auto thumb=frame(1225);
    if (thumb.is_valid() && quest.scroll_pointer(kind,int(position.x),int(position.y),thumb->get_width(),thumb->get_height())) {
        if (kind==0) raise_panel(Panel::quest);
        controller.cancel();
        get_viewport()->set_input_as_handled();
        queue_redraw();
        return true;
    }
    const auto [x,y,w,h]=quest.rect;
    const bool inside=rect(quest.rect).has_point(position);
    int hit=-1;
    for (int index=0;index<int(quest.buttons.size());++index) {
        const auto [id,left,top]=quest.buttons[index];
        const auto icon=frame(id);
        if (icon.is_valid() && Rect2(Vector2(x+left,y+top),icon->get_size()).has_point(position)) {
            hit=index;
            break;
        }
    }
    bool captured=false;
    if (kind==0 && inside) {
        raise_panel(Panel::quest);
        quest.pressed=hit;
        const int row=(int(position.y)-y-90)/15;
        const bool in_list=Rect2(x+65,y+90,200,15*quest.visible_rows).has_point(position);
        quest.begin_drag(int(position.x),int(position.y),hit>=0 || in_list);
        if (in_list) {
            const int index=quest.entry_at(quest.first+row);
            if(index>=0) {
                quest.selected=index;
                quest.description_parent=index;
                quest.description_child=-1;
                quest.description_first=0;
            } else if(quest.expanded>=0 && quest.first+row>quest.expanded) {
                const auto& entry=quest.entries[quest.expanded];
                const int child=quest.first+row-quest.expanded-1;
                if(entry.wire && child<int(entry.children.size())) {
                    quest.selected=-1;
                    quest.description_parent=quest.expanded;
                    quest.description_child=child;
                    quest.description_first=0;
                    const auto& item=entry.children[child];
                    if(item.summary.empty())
                        send_client_packet(mir3::protocol::CM_QUERYQUESTCHILDTEXT,item.id,String(),0,std::uint16_t(entry.wire->id));
                }
            }
        }
    } else if (kind==1) {
        captured=quest.move_drag(int(position.x),int(position.y)) || quest.pressed>=0;
    } else if (kind==2) {
        const bool activate=hit>=0 && hit==quest.pressed;
        captured=quest.end_drag() || quest.pressed>=0;
        quest.pressed=-1;
        if (activate) {
            if (hud_sound && audio_options.effects_enabled) hud_sound->play();
            if (hit==1) toggle_quest();
            else if (quest.selected>=0) {
                quest.expanded=quest.selected;
                quest.scroll(0);
                auto& entry=quest.entries[quest.selected];
                using Detail=mir3::QuestPanel::Entry::Detail;
                if(entry.wire) {
                    if(!entry.children_loaded)
                        send_client_packet(mir3::protocol::CM_QUERYQUESTCHILDREN,0,String(),0,std::uint16_t(entry.wire->id));
                } else if (entry.detail==Detail::idle) {
                    entry.detail=Detail::pending;
                    emit_signal("quest_detail_requested",quest.selected);
                }
            }
        }
    }
    if (!inside && !captured) return false;
    controller.cancel();
    get_viewport()->set_input_as_handled();
    queue_redraw();
    return true;
}

bool Mir3HudView::apply_mount_reply(int identifier,int64_t recognition) {
    if (recognition<0 || recognition>UINT32_MAX) return false;
    if (!mount.apply_reply(identifier,static_cast<std::uint32_t>(recognition))) return false;
    pending_local_action.reset();
    // 4282C0/1300：更新外观后强制29或0，保留方向并重置动作计时。
    player_mount=mount.shape;
    if (terrain_ready && player_mount) human_body(player_type,player_clothing,true);
    walk_frame=-1;movement_ms=0;map_route.clear();
    step_origin=step_destination=player_cell;
    player_action={};player_harvest_impact=false;player_idle.reset();
    queue_redraw();return true;
}

void Mir3HudView::toggle_mount() {
    mount.toggle();
    mount.pressed=-1;
    if (mount.visible) raise_panel(Panel::mount);
    queue_redraw();
}

void Mir3HudView::draw_mount() {
    const auto [x,y,w,h]=mount.rect;
    const auto skin=frame(mount.background);
    if (skin.is_valid()) {
        const Vector2 anchor(x-(skin->get_width()-w)/2,y-(skin->get_height()-h)/2);
        paint_canvas->draw_texture(skin,anchor,Color(1,1,1,220.f/255));
    }
    for (int index=0;index<int(mount.buttons.size());++index) {
        const auto [id,left,top,hint]=mount.buttons[index];
        sprite(id+(index==mount.pressed),{x+left,y+top});
    }
}

bool Mir3HudView::mount_pointer(int kind,Vector2 position) {
    if (!mount.visible) return false;
    const bool inside=rect(mount.rect).has_point(position);
    int hit=-1;
    for (int index=0;index<int(mount.buttons.size());++index) {
        const auto [id,x,y,hint]=mount.buttons[index];
        const auto icon=frame(id);
        if (icon.is_valid() && Rect2(Vector2(mount.rect[0]+x,mount.rect[1]+y),icon->get_size()).has_point(position)) {
            hit=index;
            break;
        }
    }
    bool captured=false;
    if (kind==0 && inside) {
        raise_panel(Panel::mount);
        mount.pressed=hit;
        mount.begin_drag(int(position.x),int(position.y),hit>=0);
    } else if (kind==1) {
        captured=mount.move_drag(int(position.x),int(position.y)) || mount.pressed>=0;
    } else if (kind==2) {
        const bool activate=hit>=0 && hit==mount.pressed;
        captured=mount.end_drag() || mount.pressed>=0;
        mount.pressed=-1;
        if (activate) {
            if (hud_sound && audio_options.effects_enabled) hud_sound->play();
            if (hit==0) toggle_mount();
            else if (const int command=mount.command(hit);command!=0) {
                send_chat_command(String::utf8(message(command).c_str()));
                map_input_delay.mount_command();
            }
        }
    }
    if (!inside && !captured) return false;
    controller.cancel();
    get_viewport()->set_input_as_handled();
    queue_redraw();
    return true;
}

void Mir3HudView::toggle_minimap(std::uint64_t cooldown) {
    if (minimap.toggle(ticks_msec(),cooldown)) emit_signal("minimap_requested",mir3::protocol::CM_MINIMAP);
    queue_redraw();
}

bool Mir3HudView::apply_minimap_reply(int map_number) {
    if (map_number<=0 || map_number>65535) return false;
    const int id=map_number-1;
    auto decoded=id<1000?minimap_archive.frame(id):full_minimap_archive.frame(id-1000);
    if (!decoded) return false;
    minimap_image=texture(decoded->pixels.rgba,decoded->width,decoded->height);
    if (minimap_image.is_null()) return false;
    minimap.receive(decoded->width,decoded->height,ticks_msec());
    portrait_map_full_archive=id>=1000;
    if (portrait_map_pending) portrait_map_visible=portrait_map_mode;
    portrait_map_pending=false;
    queue_redraw();
    return true;
}

void Mir3HudView::apply_minimap_unavailable() {
    // 0x4282C0 case 711 completes the request and posts CMList 1458.
    minimap.reply_time=ticks_msec();
    minimap.visible=false;
    minimap.pending=false;
    minimap.dragging=false;
    portrait_map_visible=portrait_map_pending=false;
    post_chat(String::utf8(message(1458).c_str()),0xffffff,0xff);
    queue_redraw();
}

void Mir3HudView::toggle_portrait_map() {
    if (minimap.reply_time && ticks_msec()-minimap.reply_time<=1000) return;
    portrait_map_mode=!portrait_map_mode;
    portrait_map_pending=portrait_map_mode;
    portrait_map_visible=false;
    if (portrait_map_pending) emit_signal("minimap_requested",mir3::protocol::CM_MINIMAP);
    const auto path=options_path.utf16();
    if (!WritePrivateProfileStringW(L"Options",L"InformationMode",portrait_map_mode?L"1":L"0",
            reinterpret_cast<const wchar_t*>(path.get_data())))
        UtilityFunctions::push_warning("Could not save InformationMode to ",options_path);
    queue_redraw();
}

void Mir3HudView::draw_portrait_map() {
    if (!portrait_map_visible || minimap_image.is_null()) return;
    const auto source=rect(minimap.portrait_source(portrait_map_full_archive));
    constexpr float light=150.f/255;
    const Vector2 surface=portrait_map_full_archive?Vector2(256,256):Vector2(180,130);
    const Vector2 origin=portrait_map_full_archive?
        Vector2(source.size.x>=256?510:530,source.size.y>=256?470:485):Vector2(510,470);
    const Vector2 scale=Vector2(128,128)/surface;
    const Rect2 bounds(origin,Vector2(128,128));
    paint_canvas->draw_rect(bounds,Color(0,0,0));
    paint_canvas->draw_texture_rect_region(minimap_image,Rect2(origin,source.size*scale),source,Color(light,light,light));
    constexpr float gray=100.f/255;
    const Color green(gray,250.f/255,gray);
    // 0x48C359 mode 1 uses GDI FrameRect, whose one-pixel stroke is inside the RECT.
    paint_canvas->draw_rect(bounds.grow(-0.5),Color(gray,gray,gray),false);
    const auto marker=[&](mir3::HudLayout::Point cell,bool player) {
        return point(mir3::Minimap::portrait_marker(cell,{int(source.position.x),int(source.position.y)},
            {int(origin.x),int(origin.y)},portrait_map_full_archive,player));
    };
    const auto player=marker({minimap.player_x,minimap.player_y},true);
    const auto draw_nearby=[&](const auto& cells,Color color) {
        for (const auto& cell:cells) {
            const auto at=marker(cell,false);
            if (std::abs(at.x-player.x)>=surface.x/2 || std::abs(at.y-player.y)>=surface.y/2) continue;
            const auto area=Rect2(at-Vector2(1,1),Vector2(3,3)).intersection(bounds);
            if (area.has_area()) paint_canvas->draw_rect(area,color);
        }
    };
    draw_nearby(minimap.nearby_green,green);
    draw_nearby(minimap.nearby_orange,Color(250.f/255,150.f/255,50.f/255));
    if (portrait_map_blink_ms>0 && portrait_map_blink_ms<500)
        paint_canvas->draw_rect(Rect2(player,Vector2(4,4)).grow(-0.5),green,false);
}

bool Mir3HudView::apply_trade_wire(const mir3::ReceiveHeader& header,std::span<const std::uint8_t> body) {
    using namespace mir3::protocol;
    if(header.opcode==SM_TRADE_OPEN_FAIL) {open_group_dialog(1434);return true;}
    if(header.opcode==SM_TRADE_OPEN) {
        if(trade.visible) {
            if(!inventory.visible) {inventory.rect[0]=518;inventory.rect[1]=0;toggle_inventory();}
            return true;
        }
        const auto name=mir3::decode_six_bit(body.data(),body.size());
        if(!name || name->empty() || name->size()>60 || name->find(char(0))!=std::string::npos) return false;
        return open_trade(String::utf8(mir3::ansi_to_utf8(*name).c_str()));
    }
    if(!trade.visible) return false;
    if(header.opcode==SM_TRADE_ADD_OK || header.opcode==SM_TRADE_ADD_FAIL ||
        header.opcode==SM_TRADE_REMOVE_OK || header.opcode==SM_TRADE_REMOVE_FAIL)
        return apply_trade_offer_reply(header.opcode,header.parameters[0],header.recognition);
    if(header.opcode==SM_TRADE_CANCELLED) return apply_trade_cancel(-1);
    if(header.opcode==SM_TRADE_COMPLETE) return apply_trade_complete(inventory_gold);
    if(header.opcode==SM_TRADE_GOLD || header.opcode==SM_TRADE_PEER_GOLD) {
        if(!apply_trade_gold_reply(header.opcode,header.recognition,header.parameters[0],header.parameters[1])) return false;
        play_login_sound(sound_list.file(106),false);return true;
    }
    const auto decoded=mir3::decode_six_bit(body.data(),body.size());
    if(!decoded || decoded->size()!=100) return false;
    mir3::InventoryRecord record;std::copy(decoded->begin(),decoded->end(),record.raw.begin());
    if(record.name().empty()) return false;
    const auto sprite=item_frame(record.icon());if(sprite.is_null()) return false;
    const auto [width,height]=mir3::InventoryGrid::dimensions(sprite->get_width(),sprite->get_height());
    mir3::BagItem item{mir3::EquipmentState::category(record.mode()),record.icon(),record.count(),width,height,
        record.mode(),record.identity(),mir3::ansi_to_utf8(record.name()),record.stack_mode(),record.raw};
    auto next=trade_items[1];
    if(header.opcode==SM_TRADE_PEER_ADD) {
        if(mir3::TradeGrid::insert(next,item)<0) return false;
    } else if(header.opcode==SM_TRADE_PEER_REMOVE) {
        for(auto& other:next) if(other && other.make_index==item.make_index && other.name==item.name) {other={};break;}
    } else return false;
    trade_items[1]=std::move(next);play_item_sound(item);
    auto caption=message(header.opcode==SM_TRADE_PEER_ADD?1435:1436);
    if(const auto at=caption.find("%s");at!=std::string::npos) caption.replace(at,2,item.name);
    post_chat(String::utf8(caption.c_str()),mir3::chat_palette(2),mir3::chat_palette(7));
    queue_redraw();return true;
}

void Mir3HudView::send_trade_offer(int count) {
    trade_offer_count=count;
    emit_signal("trade_offer_requested",mir3::protocol::CM_TRADE_ADD_ITEM,int64_t(bag.held.make_index),String::utf8(bag.held.name.c_str()),count);
    send_client_packet(mir3::protocol::CM_TRADE_ADD_ITEM,bag.held.make_index,String::utf8(bag.held.name.c_str()),0,std::uint16_t(count));
}

bool Mir3HudView::apply_trade_offer_reply(int identifier,int remaining,int64_t new_index) {
    if (!trade.visible) return false;
    auto next_bag=bag;auto next_items=trade_items[0];
    mir3::BagItem sound_item;
    if (identifier==mir3::protocol::SM_TRADE_REMOVE_OK || identifier==mir3::protocol::SM_TRADE_REMOVE_FAIL) {
        if (!trade_return_pending || trade_return_slot<0 || !next_items[trade_return_slot]) return false;
        const auto item=next_items[trade_return_slot];sound_item=item;next_items[trade_return_slot]={};
        if (identifier==mir3::protocol::SM_TRADE_REMOVE_OK) {
            bool exists=false;
            if(item.stack_mode>=2) for(const auto& other:next_bag.items) if(other && other.name==item.name) {exists=true;break;}
            if(item.name!=message(1050) && !exists && !next_bag.insert(item)) return false;
        } else if(mir3::TradeGrid::insert(next_items,item)<0) return false;
        trade_return_slot=-1;trade_return_pending=false;
    } else {
        if (trade_pending_slot<0 || !trade_offer_count || !next_bag.held ||
            (identifier!=mir3::protocol::SM_TRADE_ADD_OK && identifier!=mir3::protocol::SM_TRADE_ADD_FAIL)) return false;
        sound_item=next_bag.held;
        if(identifier==mir3::protocol::SM_TRADE_ADD_OK) {
            if(remaining<0 || (remaining && remaining>=next_bag.held.count) || new_index<0 || new_index>UINT32_MAX) return false;
            auto offered=next_bag.held;
            if(remaining) {
                offered.set_count(offered.count-remaining);offered.make_index=std::uint32_t(new_index);
                for(int i=0;i<4;++i) offered.server_record[62+i]=std::uint8_t(offered.make_index>>(8*i));
                next_bag.held.set_count(remaining);
                if(!next_bag.insert(next_bag.held)) return false;
            }
            if(mir3::TradeGrid::insert(next_items,offered,trade_pending_slot)<0) return false;
        } else if(!next_bag.insert(next_bag.held)) return false;
        next_bag.held={};next_bag.held_slot=-1;trade_pending_slot=-1;trade_offer_count=0;
    }
    bag=std::move(next_bag);trade_items[0]=std::move(next_items);
    if(identifier==mir3::protocol::SM_TRADE_ADD_OK) {
        if(remaining) quick_items.update_count(sound_item.name,remaining);else quick_items.remove(sound_item.name);
    }
    play_item_sound(sound_item); // 分堆路径4410D0也调用43ED40。
    queue_redraw();return true;
}

bool Mir3HudView::set_trade_item(int side,int slot,int icon,int count,int64_t make_index,String name,int std_mode,int stack_mode) {
    if (!trade.visible || side<0 || side>=2 || slot<0 || slot>=200 || icon< -1 || count<0 || make_index<0 || make_index>UINT32_MAX) return false;
    auto item=trade_items[side][slot];
    if (icon<0) item={};
    else {
        const auto image=item_frame(icon);
        if (image.is_null()) return false;
        const auto [width,height]=mir3::InventoryGrid::dimensions(image->get_width(),image->get_height());
        if (slot%5+width>5 || slot/5+height>40) return false;
        item={};
        item.icon=icon;
        item.count=count;
        item.make_index=static_cast<std::uint32_t>(make_index);
        item.name=name.utf8().get_data();
        item.std_mode=std_mode;
        item.stack_mode=stack_mode;
        item.width=width;
        item.height=height;
    }
    auto grid=trade_items[side];grid[slot]={};
    if(item && (mir3::TradeGrid::count(grid)>=12 || !mir3::TradeGrid::fits(grid,slot,item))) return false;
    trade_items[side][slot]=item;
    if (side==0 && slot==trade_return_slot && !item) {
        trade_return_slot=-1;
        trade_return_pending=false;
    }
    queue_redraw();
    return true;
}

bool Mir3HudView::apply_trade_gold_reply(int identifier,int64_t amount,int low_word,int high_word) {
    if (!trade.visible || amount<0 || amount>UINT32_MAX) return false;
    if (identifier==mir3::protocol::SM_TRADE_GOLD) {
        if (low_word<0 || low_word>UINT16_MAX || high_word<0 || high_word>UINT16_MAX) return false;
        trade_gold[0]=static_cast<std::uint32_t>(amount);
        inventory_gold=std::uint32_t(low_word) | (std::uint32_t(high_word)<<16);
        trade_gold_pending=false;
        trade_gold_debited=amount>0;
    } else if (identifier==mir3::protocol::SM_TRADE_PEER_GOLD) {
        trade_gold[1]=static_cast<std::uint32_t>(amount);
    } else return false;
    queue_redraw();
    return true;
}

bool Mir3HudView::set_trade_gold(int64_t local,int64_t remote) {
    if (!trade.visible || local<0 || local>UINT32_MAX || remote<0 || remote>UINT32_MAX) return false;
    trade_gold={static_cast<std::uint32_t>(local),static_cast<std::uint32_t>(remote)};
    queue_redraw();
    return true;
}

bool Mir3HudView::apply_trade_complete(int64_t balance) {
    if (!trade.visible || balance<0 || balance>UINT32_MAX || trade_pending_slot>=0 || trade_return_pending) return false;
    // 687只清交易；物品和余额通过独立权威消息更新，不复制对方展示物品。
    for(const auto& side:trade_items) for(const auto& item:side) if(item && item.stack_mode>2) {
        bool exists=false;
        for(const auto& owned:bag.items) if(owned && owned.icon==item.icon) {exists=true;break;}
        if(!exists) quick_items.remove(item.name);
    }
    inventory_gold=std::uint32_t(balance);trade_items={};
    return open_trade(String());
}

bool Mir3HudView::apply_trade_cancel(int64_t balance) {
    if (!trade.visible || balance< -1 || balance>UINT32_MAX) return false;
    auto restored=bag;
    for(const auto& item:trade_items[0]) if(item && item.stack_mode<2 && !restored.insert(item)) return false;
    if(trade_pending_slot>=0 && restored.held) {
        bool exists=false;
        if(restored.held.stack_mode>=2) for(const auto& item:restored.items) if(item && item.name==restored.held.name) {exists=true;break;}
        if(!exists && restored.held.name!=message(1050) && !restored.insert(restored.held)) return false;
        restored.held={};restored.held_slot=-1;
    }
    bag=std::move(restored);
    inventory_gold=balance<0?inventory_gold+trade_gold[0]:std::uint32_t(balance);
    trade_pending_slot=-1;trade_items={};
    return open_trade(String());
}

bool Mir3HudView::open_trade(String partner) {
    if (partner.is_empty()) {
        if (trade_pending_slot>=0) {
            if (trade_offer_count) apply_trade_offer_reply(mir3::protocol::SM_TRADE_ADD_FAIL);
            else trade_pending_slot=-1;
        }
        if (trade.visible) trade.toggle();
        trade_confirm_pressed=false;
        if (group_request==1307 || group_request==mir3::protocol::CM_TRADE_GOLD) finish_group_dialog(false);
        trade_offer_count=0;trade_partner=String();
        trade_confirmed=false;
        trade_gold_pending=false;
        trade_gold_debited=false;
        trade_gold={};
        trade_return_slot=-1;
        trade_return_pending=false;
        trade_scroll.captured=-1;
        queue_redraw();
        return true;
    }
    if (frame(1390).is_null()) return false;
    if (trade.visible) return true;
    trade_partner=partner;
    trade_gold={};
    trade_items={};
    trade_scroll={};
    trade_confirmed=false;
    trade.toggle();
    raise_panel(Panel::trade);
    if (!inventory.visible) {
        inventory.rect[0]=518;
        inventory.rect[1]=0;
        toggle_inventory();
    }
    queue_redraw();
    return true;
}

void Mir3HudView::draw_trade() {
    const auto [x,y,w,h]=trade.rect;
    const auto skin=frame(1390);
    if (skin.is_valid()) sprite(1390,{x-(skin->get_width()-w)/2,y-(skin->get_height()-h)/2});
    text(character_name.utf8().get_data(),{x,y,228,30},0xfac8c8);
    text(trade_partner.utf8().get_data(),{x+247,y,120,30},0xfac8c8);
    for (int side=0;side<2;++side) {
        // 0x41D870: five columns, 38 px per cell, separate origins.
        const int left=x+(side==0?14:264);
        const Rect2 clip(left,y+56,190,190);
        const int first=trade_scroll.first[side];
        sprite(1225,{x+mir3::TradeScroll::x[side],y+mir3::TradeScroll::top+trade_scroll.offset(side)});
        for (int slot=0;slot<5*(first+5);++slot) {
            const auto& item=trade_items[side][slot];
            if (!item || slot/5+item.height<=first) continue;
            const auto icon=item_frame(item.icon);
            if (icon.is_null()) continue;
            const Vector2 cell(left+38*(slot%5),y+56+38*(slot/5-first));
            const Vector2 padding((38*item.width-icon->get_width())/2,(38*item.height-icon->get_height())/2);
            const Rect2 target(cell+padding,icon->get_size());
            const Rect2 visible=target.intersection(clip);
            if (!visible.has_area()) continue;
            paint_canvas->draw_texture_rect_region(icon,visible,Rect2(visible.position-target.position,visible.size),
                side==0 && slot==trade_return_slot?Color(1,1,1,0.5f):Color(1,1,1));
            const int label_x=int(cell.x)+22,label_y=int(cell.y)+23;
            if (item.count>1 && clip.encloses(Rect2(label_x,label_y,16,15)))
                text(std::to_string(item.count),{label_x,label_y,16,15},0xffffff,mir3::TextFont::normal,false);
        }
        auto amount=std::to_string(trade_gold[side]);
        for (int i=int(amount.size())-3;i>0;i-=3) amount.insert(i,1,',');
        text(amount,{x+(side==0?70:350),y+289,side==0?113:85,25},0xfac8c8);
    }
    sprite(trade_confirmed?1394:trade_confirm_pressed?1396:1392,{x+226,y+279});
}

bool Mir3HudView::trade_pointer(int kind,Vector2 position) {
    if (!trade.visible) return false;
    const auto thumb=frame(1225);
    if (thumb.is_valid() && trade_scroll.pointer(kind,int(position.x)-trade.rect[0],int(position.y)-trade.rect[1],
            thumb->get_width(),thumb->get_height())) {
        controller.cancel();
        get_viewport()->set_input_as_handled();
        queue_redraw();
        return true;
    }
    const bool inside=rect(trade.rect).has_point(position);
    const auto confirm=frame(1394);
    const bool on_confirm=confirm.is_valid() &&
        Rect2(Vector2(trade.rect[0]+226,trade.rect[1]+279),confirm->get_size()).has_point(position);
    bool captured=false;
    if (kind==0 && inside) {
        raise_panel(Panel::trade);
        trade_confirm_pressed=on_confirm;
        trade.begin_drag(int(position.x),int(position.y),on_confirm || position.y>=trade.rect[1]+56);
    } else if (kind==1) captured=trade.move_drag(int(position.x),int(position.y)) || trade_confirm_pressed;
    else if (kind==2) {
        const bool activate=trade_confirm_pressed && on_confirm && !trade_confirmed && !trade_gold_pending && trade_pending_slot<0 && trade_return_slot<0;
        captured=trade.end_drag() || trade_confirm_pressed;
        trade_confirm_pressed=false;
        if (activate) {
            trade_confirmed=true;
            emit_signal("trade_requested",mir3::protocol::CM_TRADE_CONFIRM,String());
            send_client_packet(mir3::protocol::CM_TRADE_CONFIRM,0,String());
        }
        const auto [x,y,w,h]=trade.rect;
        const Rect2 local_grid(x+14,y+56,190,190);
        if (!captured && !trade_confirmed && trade_pending_slot<0 && trade_return_slot<0 &&
            !bag.held && !trade_gold_pending && !trade_gold[0] && inventory_gold &&
            Rect2(x+34,y+270,122,34).has_point(position)) {
            open_group_dialog(mir3::protocol::CM_TRADE_GOLD);
        }
        if (!captured && !trade_confirmed && trade_pending_slot<0 && !trade_return_pending &&
            !bag.held && local_grid.has_point(position)) {
            const int column=(int(position.x)-x-14)/38;
            const int row=trade_scroll.first[0]+(int(position.y)-y-56)/38;
            trade_return_slot=-1;
            for (int index=0;index<200;++index) {
                const auto& item=trade_items[0][index];
                if (!item || !item.make_index) continue;
                if (!Rect2(index%5,index/5,item.width,item.height).has_point(Vector2(column,row))) continue;
                trade_return_slot=index;
                break;
            }
        }
        if (!captured && !trade_confirmed && trade_pending_slot<0 && trade_return_slot<0 && bag.held &&
            bag.held.make_index && bag.held_equipment_slot<0 && bag.held_quick_slot<0 && !bag.pending_use && !equipment.pending &&
            Rect2(x+14,y+56,190,190).has_point(position)) {
            const int column=(int(position.x)-x-14)/38;
            const int row=trade_scroll.first[0]+(int(position.y)-y-56)/38;
            const int slot=row*5+column;
            const auto& held=bag.held;
            bool fits=mir3::TradeGrid::count(trade_items[0])<12 && column+held.width<=5 && row+held.height<=40;
            bool occupied=false;
            for (int index=0;index<200;++index) {
                const auto& item=trade_items[0][index];
                if (!item) continue;
                const Rect2 existing(index%5,index/5,item.width,item.height);
                if (existing.has_point(Vector2(column,row))) occupied=true;
                if (Rect2(column,row,held.width,held.height).intersects(existing)) fits=false;
            }
            if (fits) {
                trade_pending_slot=slot;
                if (held.stack_mode>=2 && held.count>1) open_group_dialog(1307);
                else send_trade_offer(std::max(1,held.count));
            } else if (occupied) {
                post_chat(String::utf8(message(1306).c_str()),0xffff32,0);
            }
        }
    }
    if (!inside && !captured) return false;
    controller.cancel();
    get_viewport()->set_input_as_handled();
    queue_redraw();
    return true;
}

void Mir3HudView::set_trade_context(int direction,Vector2i cell,int actor_type,bool dead,String name) {
    trade_direction=direction;
    trade_actor_cell=cell;
    trade_actor_type=actor_type;
    trade_actor_dead=dead;
    trade_actor_name=name;
}

bool Mir3HudView::request_trade() {
    // 0x4281B0 selects a live type 0/1 actor exactly one cell ahead.
    constexpr std::array<std::array<int,2>,8> directions{{
        {0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}
    }};
    if (trade_direction<0 || trade_direction>=int(directions.size()) ||
        trade_actor_type<0 || trade_actor_type>1 || trade_actor_dead || trade_actor_name.is_empty()) return false;
    const auto [dx,dy]=directions[trade_direction];
    if (std::int64_t(trade_actor_cell.x)!=std::int64_t(model.map_x)+dx ||
        std::int64_t(trade_actor_cell.y)!=std::int64_t(model.map_y)+dy) return false;
    emit_signal("trade_requested",mir3::protocol::CM_TRADE_REQUEST,trade_actor_name);
    send_client_packet(mir3::protocol::CM_TRADE_REQUEST,0,trade_actor_name);
    return true;
}

bool Mir3HudView::change_map(String map_id,String name,Vector2i spawn,int ambient_mode) {
    // Decode into a temporary map: failure must not destroy the live scene.
    if (ambient_mode<0 || ambient_mode>65535 || !loaded || name.is_empty() ||
        !mir3::valid_map_id(map_id.utf8().get_data())) return false;
    mir3::MapTerrain next;
    if (!next.open(std::filesystem::u8path(map_directory.path_join(map_id+".map").utf8().get_data())) ||
        !next.walkable(spawn.x,spawn.y)) return false;
    clear_map_events();clear_level_effects();player_action={};player_spell={};active_spell=0;player_harvest_impact=false;
    // 本地人物不经过 remove_scene_actor，换图时也必须释放其动作声道。
    remove_actor_sound(0);
    terrain=std::move(next);terrain_ready=true;
    pending_local_action.reset();
    action_prediction={};
    local_actor_messages.clear();forced_motion.reset();
    scene_ambient.enter(static_cast<std::uint16_t>(ambient_mode));
    current_map_id=map_id;
    map_input_delay.enter();
    map_series=0;weather.select(0,true);update_weather_layers();
    while (!scene_humans.empty()) remove_scene_actor(scene_humans.begin()->first);
    while (!scene_npcs.empty()) remove_scene_actor(scene_npcs.begin()->first);
    clear_monsters();
    scene_removals.clear();
    scene_clear_pending=false;
    scene_movement_ms=0;
    ground_items.items.clear();
    ground_bounds.clear();
    displayed_ground_item=-1;
    scene_speech.clear();
    speech_bounds.clear();
    scene_damage_glyphs.clear();
    if (auto local=scene_health.find(0);local!=scene_health.end()) local->second.damage={};
    scene_health_bounds.clear();
    displayed_name_actor=-1;
    map_route.clear();
    walk_frame=-1;
    movement_ms=0;
    map_pointer_button=0;
    map_pointer_seconds=0;
    merchant_click_captured=false;
    player_cell=step_origin=step_destination=spawn;
    player_idle.reset();
    player_bounds=Rect2();
    human_shadow_bounds.clear();
    if (footstep_sound) footstep_sound->stop();
    model.map_name=name.utf8().get_data();
    set_minimap_position(spawn.x,spawn.y);
    // Same-name map instances also need fresh minimap and target state.
    reset_map_overlays();
    if (map_music) map_music->stop();
    music_file=String();
    set_music_map(map_id);
    queue_redraw();
    return true;
}

void Mir3HudView::set_map_location(String name,int x,int y) {
    const auto utf8=name.utf8();
    const bool changed=model.map_name!=utf8.get_data();
    model.map_name=utf8.get_data();
    set_minimap_position(x,y);
    if (changed) reset_map_overlays();
}

void Mir3HudView::reset_map_overlays() {
    close_npc_dialogue();
    trade_actor_name=String();
    hovered_target=0;
    locked_spell_actor=0;
    target_focus.clear();
    target_attributes_by_actor.clear();

    const bool refresh=minimap.visible || minimap.pending;
    minimap.visible=false;
    minimap.pending=refresh;
    minimap.dragging=false;
    minimap.reply_time=0;
    minimap.width=0;
    minimap.height=0;
    minimap.type_five_actors.clear();
    minimap.nearby_green.clear();
    minimap.nearby_orange.clear();
    minimap_image.unref();
    portrait_map_visible=false;
    portrait_map_pending=portrait_map_mode;
    if (refresh || portrait_map_pending) emit_signal("minimap_requested",mir3::protocol::CM_MINIMAP);
}

void Mir3HudView::set_minimap_position(int x,int y) {
    model.map_x=x;
    model.map_y=y;
    minimap.player_x=x;
    minimap.player_y=y;
    queue_redraw();
}

void Mir3HudView::set_minimap_markers(PackedVector2Array type_five,PackedVector2Array nearby_green,
        PackedVector2Array nearby_orange) {
    const auto replace=[](auto& destination,const PackedVector2Array& cells) {
        destination.clear();
        destination.reserve(cells.size());
        for (int64_t i=0;i<cells.size();++i) {
            const auto cell=cells[i];
            if (cell.x>=0 && cell.x<=65535 && cell.y>=0 && cell.y<=65535)
                destination.push_back({int(cell.x),int(cell.y)});
        }
    };
    replace(minimap.type_five_actors,type_five);
    replace(minimap.nearby_green,nearby_green);
    replace(minimap.nearby_orange,nearby_orange);
    queue_redraw();
}

void Mir3HudView::draw_minimap() {
    if (!minimap.visible || minimap_image.is_null()) return;
    const auto bounds=rect(minimap.bounds());
    const auto end=bounds.get_end();
    const auto source=point(minimap.source());
    constexpr float light=150.f/255;
    paint_canvas->draw_texture_rect_region(minimap_image,bounds,Rect2(source,bounds.size),
        Color(light,light,light,minimap.translucent?175.f/255:1.f));
    paint_canvas->draw_rect(bounds,Color(100.f/255,100.f/255,100.f/255),false);
    const auto origin=bounds.position-source;
    const auto player=origin+Vector2(float(mir3::Minimap::map_pixel_x(minimap.player_x)-2),
        float(std::int64_t(minimap.player_y)-2));
    const auto draw_markers=[&](const auto& cells,Color color,bool nearby) {
        for (const auto& [x,y]:cells) {
            const auto marker=origin+Vector2(mir3::Minimap::actor_marker_x(x),y-1);
            if (nearby && (std::abs(marker.x-player.x)>=minimap.size()/2 ||
                std::abs(marker.y-player.y)>=minimap.size()/2)) continue;
            const auto area=Rect2(marker-Vector2(nearby,nearby),Vector2(nearby?3:2,nearby?3:2)).intersection(bounds);
            if (area.has_area()) paint_canvas->draw_rect(area,color);
        }
    };
    if (system_options.show_mon) draw_markers(minimap.type_five_actors,Color(1,1,0),false);
    draw_markers(minimap.nearby_green,Color(100.f/255,250.f/255,100.f/255),true);
    draw_markers(minimap.nearby_orange,Color(250.f/255,150.f/255,50.f/255),true);
    if (ticks_msec()%801<500) {
        const auto area=Rect2(player,Vector2(4,4)).intersection(bounds);
        if (area.has_area()) paint_canvas->draw_rect(area,Color(100.f/255,250.f/255,100.f/255),false);
    }
    sprite(1486,minimap.position);
    sprite(1487,{int(end.x)-32,int(bounds.position.y)});
    sprite(1488,{int(bounds.position.x),int(end.y)-35});
    sprite(minimap.translucent?1482:1483,minimap.button_position(0));
    sprite(minimap.large?1485:1484,minimap.button_position(1));
}

bool Mir3HudView::minimap_pointer(int kind,Vector2 position,bool ctrl) {
    if (!minimap.visible) return false;
    const auto bounds=rect(minimap.bounds());
    const auto end=bounds.get_end();
    if (kind==0 && ctrl && bounds.has_point(position)) {
        minimap.drag_offset={int(position.x)-minimap.position[0],int(position.y)-minimap.position[1]};
        minimap.dragging=true;
        minimap.release_captured=true;
    } else if (kind==0 && Rect2(end-Vector2(38,19),Vector2(19,19)).has_point(position)) {
        minimap.translucent=!minimap.translucent;
        minimap.release_captured=true;
    } else if (kind==0 && Rect2(end-Vector2(19,19),Vector2(19,19)).has_point(position)) {
        minimap.resize();
        minimap.release_captured=true;
    } else if (kind==1 && minimap.dragging && ctrl && bounds.has_point(position)) {
        minimap.position={int(position.x)-minimap.drag_offset[0],int(position.y)-minimap.drag_offset[1]};
    } else if (kind==1 || !bounds.has_point(position)) {
        return false;
    }
    controller.cancel();
    get_viewport()->set_input_as_handled();
    queue_redraw();
    return true;
}

void Mir3HudView::load_status_body() {
    status_body.unref();
    status_body_offset=Vector2();
    if (const auto body=portraits.frame(model.appearance)) {
        status_body=texture(body->pixels.rgba,body->width,body->height);
        status_body_offset=Vector2(body->offset_x,body->offset_y);
    }
}

void Mir3HudView::set_character_identity(String name,String secondary,String tertiary,String badge) {
    group_player=name;
    character_name=name;
    character_secondary=secondary;
    character_tertiary=tertiary;
    character_badge=badge;
    load_chat_shortcuts();
    queue_redraw();
}

bool Mir3HudView::set_character_elements(PackedByteArray attack,PackedByteArray strength,PackedByteArray weakness) {
    const std::array rows{attack,strength,weakness};
    for (const auto& row:rows) if (row.size()!=7) return false;
    for (int row=0;row<3;++row)
        for (int column=0;column<7;++column) elements.set(row,column,rows[row][column]);
    queue_redraw();
    return true;
}

bool Mir3HudView::set_character_combat(Vector2i attack,Vector2i magic,Vector2i spirit,
        Vector2i defence,Vector2i magic_defence) {
    for (const auto range:{attack,magic,spirit,defence,magic_defence})
        if (range.x<0 || range.y<range.x || range.y>UINT16_MAX) return false;
    model.attack={attack.x,attack.y};
    model.magic_attack={magic.x,magic.y};
    model.spirit_attack={spirit.x,spirit.y};
    model.defence={defence.x,defence.y};
    model.magic_defence={magic_defence.x,magic_defence.y};
    queue_redraw();
    return true;
}

bool Mir3HudView::set_character_progress(int level,int profession,int appearance,int64_t experience,int64_t next_experience) {
    if (level<0 || level>255 || profession<0 || profession>2 || appearance<0 || appearance>1 ||
        experience<0 || experience>UINT32_MAX || next_experience<0 || next_experience>UINT32_MAX) return false;
    const bool appearance_changed=model.appearance!=appearance;
    model.level=level;
    model.profession=profession;
    model.appearance=appearance;
    model.experience=static_cast<std::uint32_t>(experience);
    model.next_experience=static_cast<std::uint32_t>(next_experience);
    if (appearance_changed && loaded) load_status_body();
    queue_redraw();
    return true;
}

bool Mir3HudView::set_character_load_bonuses(Vector2i wear,Vector2i hand,Vector2i bonuses) {
    for (const auto pair:{wear,hand,bonuses})
        if (pair.x<0 || pair.x>UINT8_MAX || pair.y<0 || pair.y>UINT8_MAX) return false;
    model.wear_weight={unsigned(wear.x),unsigned(wear.y)};
    model.hand_weight={unsigned(hand.x),unsigned(hand.y)};
    model.status_bonus_percent={unsigned(bonuses.x),unsigned(bonuses.y)};
    queue_redraw();
    return true;
}

bool Mir3HudView::set_character_vitals(int hp,int max_hp,int mp,int max_mp,int weight,int max_weight) {
    for (const int value:{hp,max_hp,mp,max_mp})
        if (value<0 || value>UINT16_MAX) return false;
    if (weight<0 || max_weight<0) return false;
    model.hp=static_cast<std::uint16_t>(hp);
    model.max_hp=static_cast<std::uint16_t>(max_hp);
    model.mp=static_cast<std::uint16_t>(mp);
    model.max_mp=static_cast<std::uint16_t>(max_mp);
    model.weight=weight;
    model.max_weight=max_weight;
    queue_redraw();
    return true;
}

bool Mir3HudView::set_music_map(String map) {
    constexpr int header=58,stride=24;
    if (!map_music || music_table.size()<header) return false;
    const auto count=music_table.decode_u32(54);
    if (count>(music_table.size()-header)/stride) return false;
    for (int64_t i=0;i<count;++i) {
        const auto* record=music_table.ptr()+header+i*stride;
        const auto field=[](const std::uint8_t* begin,int size) {
            return String::utf8(reinterpret_cast<const char*>(begin),std::find(begin,begin+size,0)-begin);
        };
        if (field(record+1,9)!=map) continue;
        const auto file=field(record+10,14);
        if (file.is_empty()) continue;
        if (file=="none" || file=="nobgm") return true;
        if (file.get_file()!=file || file.contains(":")) return false;
        if (file==music_file) return true;
        const auto stream=AudioStreamMP3::load_from_file(audio_root.path_join(file));
        if (stream.is_null()) return false;
        stream->set_loop(true);
        map_music->stop();
        map_music->set_stream(stream);
        music_file=file;
        sync_hud_sound();
        return true;
    }
    return false;
}

void Mir3HudView::set_portrait_names(PackedStringArray names) {
    portrait_preview.unref();
    portrait_picker.close();
    for (int i=0;i<names.size();++i) portrait_picker.names.emplace_back(names[i].utf8().get_data());
    queue_redraw();
}

void Mir3HudView::close_portrait_picker() {
    portrait_panel.visible=false;
    portrait_pressed=-1;
    portrait_preview.unref();
    portrait_picker.close();
    queue_redraw();
}

void Mir3HudView::draw_portrait_picker() {
    using Picker=mir3::PortraitPicker;
    const auto skin=frame(Picker::frame);
    if (skin.is_valid()) sprite(Picker::frame,{-(skin->get_width()-Picker::initial[2])/2,-(skin->get_height()-Picker::initial[3])/2});
    for (int i=0;i<Picker::visible_rows;++i) {
        const auto bounds=Picker::row(i);
        if (i==portrait_picker.hovered) legacy_rect(rect(bounds),Color(30.f/255,30.f/255,30.f/255,180.f/255));
        if (i==portrait_picker.selected_row()) legacy_rect(rect(bounds),Color(35.f/255,114.f/255,156.f/255,180.f/255));
        const int index=portrait_picker.first+i;
        if (index<int(portrait_picker.names.size()))
            text(portrait_picker.names[index],{bounds[0],bounds[1]+5,bounds[2],bounds[3]},0xfafafa,mir3::TextFont::normal,false);
        const auto at=Picker::instruction(i);
        text(message(Picker::instruction_message(i)),{at[0],at[1],260,15},0xfafafa,mir3::TextFont::normal,false);
    }
    if (portrait_preview.is_valid()) paint_canvas->draw_texture_rect(portrait_preview,rect(Picker::preview),false);
    sprite(Picker::scroll[0],{Picker::scroll[1],portrait_picker.thumb_y()});
    const int hovered=portrait_button_at(inventory_cursor);
    for (int i=0;i<3;++i) {
        const auto spec=Picker::buttons[i];
        const auto layers=mir3::button_frames({spec[0],spec[0]+int(i==0),i==0?spec[0]:-1,i==0},
            portrait_pressed==i?2:hovered==i?1:0);
        for (const int id:layers) if (id>=0) sprite(id,{spec[1],spec[2]});
    }
}

int Mir3HudView::portrait_button_at(Vector2 point) {
    const auto& buttons=mir3::PortraitPicker::buttons;
    for (int i=0;i<int(buttons.size());++i) {
        const auto spec=buttons[i];
        const auto icon=frame(spec[0]);
        if (icon.is_valid() && Rect2(Vector2(spec[1],spec[2]),icon->get_size()).has_point(point))
            return i;
    }
    return -1;
}

bool Mir3HudView::portrait_input(int kind,Vector2 point) {
    if (!portrait_panel.visible) return false;
    using Picker=mir3::PortraitPicker;
    const auto thumb=frame(Picker::scroll[0]);
    const auto tick=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    const bool scroll=thumb.is_valid() && portrait_picker.scroll_input(kind,int(point.x),int(point.y),
        thumb->get_width(),thumb->get_height(),static_cast<std::uint32_t>(tick));
    if (!scroll && !rect(Picker::initial).has_point(point) && portrait_pressed<0) return false;
    if (!scroll) {
        const int hit=portrait_button_at(point);
        if (kind==1) portrait_picker.move(int(point.x),int(point.y));
        if (kind==0) {
            portrait_pressed=hit;
            if (hit<0 && portrait_picker.select(int(point.x),int(point.y))) load_portrait_preview();
        }
        if (kind==2) {
            if (hit>=0 && hit==portrait_pressed) {
                if (hit==0) close_portrait_picker();
                else request_portrait_change(hit,static_cast<std::uint32_t>(tick));
                if (hud_sound && audio_options.effects_enabled) hud_sound->play();
            }
            portrait_pressed=-1;
        }
    }
    get_viewport()->set_input_as_handled();
    queue_redraw();
    return true;
}

void Mir3HudView::enumerate_portraits() {
    portrait_preview.unref();
    portrait_picker.close();
    std::error_code error;
    const auto directory=std::filesystem::u8path(portrait_directory.utf8().get_data());
    for (std::filesystem::directory_iterator it(directory,error),end;!error && it!=end;it.increment(error)) {
        std::error_code entry_error;
        if (!it->is_regular_file(entry_error) || it->is_symlink(entry_error)) continue;
        auto extension=it->path().extension().u8string();
        for (auto& ch:extension) if (ch>=u8'A' && ch<=u8'Z') ch+=32;
        if (extension!=u8".jpg") continue;
        const auto size=it->file_size(entry_error);
        if (entry_error || !mir3::PortraitPicker::accepts_size(size)) continue;
        const auto name=it->path().filename().u8string();
        portrait_picker.names.emplace_back(reinterpret_cast<const char*>(name.data()),name.size());
    }
}

void Mir3HudView::load_portrait_preview() {
    portrait_preview.unref();
    const auto file=open_selected_portrait();
    if (file.is_null() || !mir3::PortraitPicker::accepts_size(file->get_length())) return;
    Ref<Image> image;
    image.instantiate();
    if (image->load_jpg_from_buffer(file->get_buffer(file->get_length()))==OK)
        portrait_preview=ImageTexture::create_from_image(image);
}

Ref<FileAccess> Mir3HudView::open_selected_portrait() const {
    if (portrait_picker.selected<0 || portrait_picker.selected>=int(portrait_picker.names.size())) return {};
    const String name=String::utf8(portrait_picker.names[portrait_picker.selected].c_str());
    if (name.get_file()!=name || name.contains(":") || name.get_extension().to_lower()!="jpg") return {};
    return FileAccess::open(portrait_directory.path_join(name),FileAccess::READ);
}

void Mir3HudView::request_portrait_change(int button,std::uint32_t tick) {
    if (button==2) {
        emit_signal("portrait_upload_requested",PackedByteArray{});
        return;
    }
    if (portrait_picker.selected<0) return;
    const auto file=open_selected_portrait();
    if (file.is_null()) { open_group_dialog(4716);return; }
    const auto length=file->get_length();
    if (!mir3::PortraitPicker::accepts_size(length)) { open_group_dialog(4714);return; }
    const auto bytes=file->get_buffer(length);
    Ref<Image> image;
    image.instantiate();
    if (image->load_jpg_from_buffer(bytes)!=OK) { open_group_dialog(4716);return; }
    if (portrait_picker.accept_upload_tick(tick)) emit_signal("portrait_upload_requested",bytes);
}

bool Mir3HudView::display_portrait(PackedByteArray jpeg) {
    if (jpeg.is_empty()) {
        active_portrait.unref();
    } else {
        if (!mir3::PortraitPicker::accepts_size(jpeg.size())) return false;
        Ref<Image> image;
        image.instantiate();
        if (image->load_jpg_from_buffer(jpeg)!=OK) return false;
        active_portrait=ImageTexture::create_from_image(image);
    }
    queue_redraw();
    return true;
}

bool Mir3HudView::load_portrait_cache(String server,String character) {
    const auto valid=[](const String& name) {
        return !name.is_empty() && name.validate_filename()==name && name!="." && name!="..";
    };
    if (!valid(server) || !valid(character)) return false;
    portrait_cache_path="user://"+server+"_"+character+".fac";
    display_portrait({});
    const auto file=FileAccess::open(portrait_cache_path,FileAccess::READ);
    if (file.is_null() || file->get_length()>8+mir3::PortraitFac::max_payload) return false;
    const auto bytes=file->get_buffer(file->get_length());
    if (bytes.size()<8) return false;
    const auto decoded=mir3::PortraitFac::decode({reinterpret_cast<const char*>(bytes.ptr()),static_cast<std::size_t>(bytes.size())});
    if (!decoded) return false;
    PackedByteArray jpeg;
    jpeg.resize(decoded->size());
    if (!decoded->empty()) std::copy(decoded->begin(),decoded->end(),jpeg.ptrw());
    return display_portrait(jpeg);
}

bool Mir3HudView::apply_portrait_reply(PackedByteArray jpeg) {
    if (!display_portrait(jpeg)) return false;
    if (portrait_cache_path.is_empty()) return true;
    const auto payload=jpeg.is_empty()?std::string_view{}:
        std::string_view(reinterpret_cast<const char*>(jpeg.ptr()),static_cast<std::size_t>(jpeg.size()));
    const auto encoded=mir3::PortraitFac::encode(payload);
    if (!encoded) return false;
    const auto path=ProjectSettings::get_singleton()->globalize_path(portrait_cache_path);
    const auto temporary=path+String(".tmp");
    auto file=FileAccess::open(temporary,FileAccess::WRITE);
    bool saved=false;
    if (file.is_valid()) {
        const auto* bytes=reinterpret_cast<const std::uint8_t*>(encoded->data());
        PackedByteArray buffer;
        buffer.resize(encoded->size());
        std::copy(bytes,bytes+encoded->size(),buffer.ptrw());
        saved=file->store_buffer(buffer);
        file->flush();
        saved=saved && file->get_error()==OK;
        file.unref();
    }
    const auto target=path.utf16(),source=temporary.utf16();
    const auto* source_path=reinterpret_cast<const wchar_t*>(source.get_data());
    if (!saved || !MoveFileExW(source_path,reinterpret_cast<const wchar_t*>(target.get_data()),
                              MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        DeleteFileW(source_path);
        UtilityFunctions::push_warning("Could not save portrait cache");
    }
    return true;
}

void Mir3HudView::raise_panel(Panel panel) {
    const auto entry=std::find(panel_order.begin(),panel_order.end(),panel);
    std::rotate(entry,entry+1,panel_order.end());
}

const mir3::DraggablePanel& Mir3HudView::panel_state(Panel panel) const {
    switch (panel) {
    case Panel::group: return group;
    case Panel::skills: return skills;
    case Panel::status: return status;
    case Panel::inventory: return inventory;
    case Panel::guild: return guild;
    case Panel::options: return options;
    case Panel::portrait: return portrait_panel;
    case Panel::mount: return mount;
    case Panel::quest: return quest;
    case Panel::trade: return trade;
    case Panel::quick: return quick_panel;
    }
    return inventory;
}

bool Mir3HudView::panel_covered(Panel panel,Vector2 point) const {
    const auto covers=[&](const mir3::DraggablePanel& state) {
        return state.visible && rect(state.rect).has_point(point);
    };
    if (covers(guild_editor) || group_request || (minimap.visible && rect(minimap.bounds()).has_point(point)) ||
        (shop_actor && Rect2(shop_window_left(),shop_window_top(),shop_width(),shop_window_height()).has_point(point)) ||
        (npc_dialogue_actor && rect(npc_dialogue_bounds()).has_point(point)))
        return true;
    for (auto it=panel_order.rbegin();it!=panel_order.rend() && *it!=panel;++it)
        if (covers(panel_state(*it))) return true;
    return false;
}
