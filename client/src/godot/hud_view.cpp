#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/audio_stream_wav.hpp>
#include <godot_cpp/classes/audio_stream_mp3.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include "hud_view.hpp"
#include "hud/portrait_fac.hpp"
#include <godot_cpp/classes/config_file.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/shader.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <chrono>
#include <godot_cpp/classes/project_settings.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <godot_cpp/classes/style_box_empty.hpp>
#include "status/status_layout.hpp"
#include <godot_cpp/classes/input_event_key.hpp>
#include <cstdio>
#include <iterator>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
namespace {
Ref<ImageTexture> texture(const std::vector<std::uint8_t>& rgba,int width,int height) {
    if (rgba.empty()) return {};
    PackedByteArray bytes; bytes.resize(rgba.size());
    std::copy(rgba.begin(),rgba.end(),bytes.ptrw());
    return ImageTexture::create_from_image(Image::create_from_data(width,height,false,Image::FORMAT_RGBA8,bytes));
}
Rect2 rect(mir3::HudLayout::Rect r) { return Rect2(r[0],r[1],r[2],r[3]); }
Vector2 point(mir3::HudLayout::Point p) { return Vector2(p[0],p[1]); }
std::uint64_t ticks_msec() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}
}
void Mir3HudView::_bind_methods() {
    ClassDB::bind_method(D_METHOD("add_inventory_item","icon","count","slot","make_index","name","std_mode","stack_mode"),&Mir3HudView::add_inventory_item,DEFVAL(0),DEFVAL(String()),DEFVAL(40),DEFVAL(0));
    ClassDB::bind_method(D_METHOD("apply_inventory_use_reply","identifier","remaining"),&Mir3HudView::apply_inventory_use_reply);
    ADD_SIGNAL(MethodInfo("spell_key_requested",PropertyInfo(Variant::INT,"key"),
        PropertyInfo(Variant::INT,"skill_id"),PropertyInfo(Variant::VECTOR2,"pointer")));
    ADD_SIGNAL(MethodInfo("skill_key_requested",PropertyInfo(Variant::INT,"id"),PropertyInfo(Variant::INT,"legacy_key")));
    ClassDB::bind_method(D_METHOD("set_group_roster","player","members"),&Mir3HudView::set_group_roster);
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
    ClassDB::bind_method(D_METHOD("set_quest_details","index","lines"),&Mir3HudView::set_quest_details);
    ADD_SIGNAL(MethodInfo("quest_detail_requested",PropertyInfo(Variant::INT,"index")));
    ClassDB::bind_method(D_METHOD("apply_mount_reply","identifier","recognition"),&Mir3HudView::apply_mount_reply);
    ADD_SIGNAL(MethodInfo("minimap_requested",PropertyInfo(Variant::INT,"identifier")));
    ClassDB::bind_method(D_METHOD("apply_minimap_reply","map_number"),&Mir3HudView::apply_minimap_reply);
    ClassDB::bind_method(D_METHOD("set_map_location","name","x","y"),&Mir3HudView::set_map_location);
    ClassDB::bind_method(D_METHOD("set_minimap_position","x","y"),&Mir3HudView::set_minimap_position);
    ClassDB::bind_method(D_METHOD("set_minimap_markers","type_five","nearby_green","nearby_orange"),&Mir3HudView::set_minimap_markers);
    ADD_SIGNAL(MethodInfo("portrait_list_requested"));
    ADD_SIGNAL(MethodInfo("portrait_upload_requested",PropertyInfo(Variant::PACKED_BYTE_ARRAY,"jpeg")));
    ClassDB::bind_method(D_METHOD("load_portrait_cache","server","character"),&Mir3HudView::load_portrait_cache);
    ClassDB::bind_method(D_METHOD("apply_portrait_reply","jpeg"),&Mir3HudView::apply_portrait_reply);
    ClassDB::bind_method(D_METHOD("set_portrait_names","names"),&Mir3HudView::set_portrait_names);
    ADD_SIGNAL(MethodInfo("guild_members_requested"));
    ADD_SIGNAL(MethodInfo("guild_edit_requested",PropertyInfo(Variant::INT,"identifier"),PropertyInfo(Variant::STRING,"body")));
    ADD_SIGNAL(MethodInfo("guild_member_removal_requested",PropertyInfo(Variant::STRING,"name")));
    ClassDB::bind_method(D_METHOD("set_guild_manager","allowed"),&Mir3HudView::set_guild_manager);
    ClassDB::bind_method(D_METHOD("set_guild_members","lines","editable"),&Mir3HudView::set_guild_members,DEFVAL(PackedStringArray()));
    ClassDB::bind_method(D_METHOD("set_guild_notices","lines"),&Mir3HudView::set_guild_notices);
    ADD_SIGNAL(MethodInfo("group_member_requested",PropertyInfo(Variant::INT,"identifier"),PropertyInfo(Variant::STRING,"name")));
    ADD_SIGNAL(MethodInfo("group_allow_requested",PropertyInfo(Variant::BOOL,"allowed")));
    ClassDB::bind_method(D_METHOD("set_learned_skill","id","level","key","experience","required","name"),&Mir3HudView::set_learned_skill,DEFVAL(0),DEFVAL(0),DEFVAL(String()));
    ClassDB::bind_method(D_METHOD("apply_equipment_reply","identifier"),&Mir3HudView::apply_equipment_reply);
    ADD_SIGNAL(MethodInfo("equipment_requested",PropertyInfo(Variant::INT,"identifier"),PropertyInfo(Variant::INT,"slot"),PropertyInfo(Variant::INT,"make_index"),PropertyInfo(Variant::STRING,"name")));
    ADD_SIGNAL(MethodInfo("inventory_use_requested",PropertyInfo(Variant::INT,"make_index"),PropertyInfo(Variant::INT,"tick"),PropertyInfo(Variant::STRING,"name")));
    ClassDB::bind_method(D_METHOD("hud_snapshot"),&Mir3HudView::hud_snapshot);
    ClassDB::bind_method(D_METHOD("post_chat","value","foreground","background"),&Mir3HudView::post_chat);
    ClassDB::bind_method(D_METHOD("update_chat_rejection","channel","rejected"),&Mir3HudView::update_chat_rejection);
    ClassDB::bind_method(D_METHOD("handle_exit_confirmation","dialog_id"),&Mir3HudView::handle_exit_confirmation);
    ClassDB::bind_method(D_METHOD("set_trade_context","direction","cell","actor_type","dead","name"),&Mir3HudView::set_trade_context);
    ClassDB::bind_method(D_METHOD("open_trade","partner"),&Mir3HudView::open_trade);
    ADD_SIGNAL(MethodInfo("trade_requested",PropertyInfo(Variant::INT,"identifier"),PropertyInfo(Variant::STRING,"name")));
    ADD_SIGNAL(MethodInfo("exit_confirmed",PropertyInfo(Variant::INT,"dialog_id")));
    ADD_SIGNAL(MethodInfo("hud_action",PropertyInfo(Variant::INT,"button")));
    ADD_SIGNAL(MethodInfo("chat_requested",PropertyInfo(Variant::STRING,"text")));
}
Dictionary Mir3HudView::hud_snapshot() const {
    Dictionary result;
    result["ready"]=loaded;
    result["character_level"]=model.level;
    result["character_portrait"]=model.portrait_frame();
    result["character_shared_hp"]=model.shared_hp();
    result["character_hp"]=Vector2i(model.hp,model.max_hp);
    result["character_mp"]=Vector2i(model.mp,model.max_mp);
    result["character_weight"]=Vector2i(model.weight,model.max_weight);
    result["character_wear_weight"]=Vector2i(model.wear_weight[0],model.wear_weight[1]);
    result["character_hand_weight"]=Vector2i(model.hand_weight[0],model.hand_weight[1]);
    result["character_bonuses"]=Vector2i(model.status_bonus_percent[0],model.status_bonus_percent[1]);
    result["expanded"]=model.layout.expanded;
    result["quest_first"]=quest.first;
    result["quest_selected"]=quest.selected;
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
    result["chat_compact_count"]=int(chat_history.lines(false).size());
    result["chat_expanded_count"]=int(chat_history.lines(true).size());
    result["chat_compact_first"]=chat_history.first(false);
    result["chat_expanded_first"]=chat_history.first(true);
    result["portrait_visible"]=portrait_panel.visible;
    result["custom_portrait"]=active_portrait.is_valid();
    result["portrait_preview"]=portrait_preview.is_valid();
    result["portrait_count"]=int64_t(portrait_picker.names.size());
    result["portrait_first"]=portrait_picker.first;
    result["portrait_selected"]=portrait_picker.selected;
    result["options_visible"]=options.visible;
    result["music_enabled"]=audio_options.music_enabled;
    result["music_level"]=audio_options.music_level;
    result["guild_visible"]=guild.visible;
    result["guild_first"]=guild_first;
    result["guild_tab"]=guild_layout.tab;
    result["group_request"]=group_request;
    result["group_visible"]=group.visible;
    result["group_allow"]=group_allow;
    result["skill_shortcuts_visible"]=skill_shortcuts_visible;
    result["skills_visible"]=skills.visible;
    result["skill_tab"]=skills.tab;
    result["skill_scroll"]=skills.scroll;
    result["status_visible"]=status.visible;
    result["status_position"]=Vector2i(status.rect[0],status.rect[1]);
    result["inventory_visible"]=inventory.visible;
    result["inventory_position"]=Vector2i(inventory.rect[0],inventory.rect[1]);
    result["inventory_use_pending"]=bag.pending_use;
    result["equipment_pending"]=equipment.pending;
    result["held_inventory_icon"]=bag.held.icon;
    result["inventory_row"]=inventory.row;
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
    loaded=archive.open(std::filesystem::u8path(root.utf8().get_data())/"Data"/"GameInter");
    if (!loaded || frame(1160).is_null()) {
        loaded=false; UtilityFunctions::push_error("Cannot read original GameInter WIL/WIX resources."); return;
    }
    audio_root=root.path_join("Sound");
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
    portrait_directory=ProjectSettings::get_singleton()->globalize_path(String(get_meta("portrait_directory",root)));
    const auto resource_root=std::filesystem::u8path(root.utf8().get_data());
    if (!portraits.open(resource_root/"Data"/"ProgUse")) UtilityFunctions::push_error("Cannot read ProgUse portraits.");
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
    for (int i=0;i<23;++i) {
        auto image=frame(mir3::HudLayout::buttons[i][0]);
        if (image.is_valid()) model.button_sizes[i]={image->get_width(),image->get_height()};
    }
    UtilityFunctions::print("HUD ready: 800x600; original WIL resources; C++ MVC.");
    queue_redraw();
}
Ref<ImageTexture> Mir3HudView::frame(int id) {
    if (const auto found=frames.find(id);found!=frames.end()) return found->second;
    auto decoded=archive.frame(id);
    return frames.emplace(id,decoded?texture(decoded->pixels.rgba,decoded->width,decoded->height):Ref<ImageTexture>{}).first->second;
}
void Mir3HudView::sprite(int id,mir3::HudLayout::Point position) {
    auto image=frame(id);
    if (image.is_valid()) draw_texture(image,point(position));
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
    if (found->second.is_valid()) draw_texture(found->second,Vector2(r[0],r[1]),Color(float(color&255)/255,float((color>>8)&255)/255,float((color>>16)&255)/255));
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
void Mir3HudView::_draw() {
    using Layout=mir3::HudLayout;
    if (!loaded) return;
    if (!model.layout.expanded) {
        sprite(1490,Layout::image_position(Layout::Image::portrait));
        const int id=model.portrait_frame();
        if (active_portrait.is_valid()) {
            draw_texture_rect(active_portrait,Rect2(point(Layout::image_position(Layout::Image::portrait)),Vector2(82,94)),false);
        } else if (id>=0) {
            if (!portrait_frames.contains(id)) {
                auto decoded=portraits.frame(id);
                portrait_frames[id]=decoded?texture(decoded->pixels.rgba,decoded->width,decoded->height):Ref<ImageTexture>{};
            }
            if (portrait_frames[id].is_valid()) draw_texture(portrait_frames[id],point(Layout::image_position(Layout::Image::portrait)));
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
            if (crop[3]>0) draw_texture_rect_region(side==0?left:right,dest,rect(crop));
        }
    }
    const int tooltip_kind=Layout::tooltip_at(model.pointer,model.shared_hp());
    if (tooltip_kind>=0 && tooltip_kind<=2 && left.is_valid() && right.is_valid()) tooltip(tooltip_kind);
    for (int meter:{1,0}) {
        auto image=frame(1166+meter); if (image.is_null()) continue;
        int hidden=model.meter_crop(meter,image->get_height());
        auto crop=Rect2(0,hidden,image->get_width(),image->get_height()-hidden);
        auto dest=crop; dest.position+=point(Layout::image_position(meter==0?Layout::Image::experience:Layout::Image::weight));
        if (crop.size.y>0) draw_texture_rect_region(image,dest,crop);
        if (tooltip_kind==(meter==0?4:3)) tooltip(tooltip_kind);
    }
    text(std::to_string(model.level),Layout::level(model.layout.expanded),Layout::text_color(1),mir3::TextFont::level);
    if (!model.map_name.empty()) {
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
    sprite(mir3::ChatScrollLayout::frame,{mir3::ChatScrollLayout::x,mir3::chat_thumb_y(chat_history.first(model.layout.expanded),int(chat_history.lines(model.layout.expanded).size()),model.layout.expanded)});
    buttons();
    for (const auto panel:panel_order) {
        if (!panel_state(panel).visible) continue;
        switch (panel) {
        case Panel::group: draw_group(); break;
        case Panel::skills: draw_skills(); break;
        case Panel::status: draw_status(); break;
        case Panel::inventory: draw_inventory(); break;
        case Panel::guild: draw_guild(); break;
        case Panel::options: draw_options(); break;
        case Panel::portrait: draw_portrait_picker(); break;
        case Panel::mount: draw_mount(); break;
        case Panel::quest: draw_quest(); break;
        case Panel::trade: draw_trade(); break;
        }
    }
    draw_minimap();
    draw_skill_shortcuts();
    draw_skill_key_menu();
    draw_skill_description();
    if (guild_editor.visible) draw_guild_editor();
    if (!model.layout.expanded && Rect2(535,486,81,94).has_point(inventory_cursor))
        sprite(mir3::HudLayout::portrait_entry_frame(portrait_entry_pressed),mir3::HudLayout::portrait_entry_position);
    if (group_request) draw_group_dialog();
    button_hint();
    if (bag.held) {
        const auto icon=item_frame(bag.held.icon);
        if (icon.is_valid()) draw_texture(icon,(inventory_cursor-icon->get_size()*0.5).floor());
    }
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
            draw_rect(Rect2(bounds[0],bounds[1],measured.text_width,measured.text_height),Color(float(c&255)/255,float((c>>8)&255)/255,float((c>>16)&255)/255));
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
void Mir3HudView::chat_requested(String value) { emit_signal("chat_requested",value); }
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
    draw_rect(rect(bounds),borders[kind],false);
}
void Mir3HudView::button_hint() {
    if (group_request || (guild_editor.visible && rect(guild_editor.rect).has_point(inventory_cursor))) return;
    int hint=-1;
    Vector2 anchor;
    bool panel_hit=false;
    if (minimap.visible) {
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
            for (int index=1;index<int(mount.buttons.size());++index)
                if (mount.pressed!=index) resolve(mount.buttons[index],mount.buttons[index][3]);
        } else if (*it==Panel::quest) {
            if (quest.pressed!=0) resolve(quest.buttons[0],2200);
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

    draw_rect(rect(bounds),Color(0,0,0));
    if (bounds[2]>2 && bounds[3]>2) draw_rect(rect(bounds).grow(-1),Color(1,1,150.f/255));
    text(caption,bounds,0);
}
void Mir3HudView::_process(double) {
    // Original button tooltip grows once per draw, not once per mouse event.
    if (minimap.visible || model.hovered>=0 || (button_tooltip.expanding && !button_tooltip.previous.empty())) queue_redraw();
}
void Mir3HudView::legacy_rect(Rect2 bounds,Color color) {
    const auto a=bounds.position,b=bounds.get_end();
    PackedVector2Array vertices,uv;
    vertices.push_back(a);vertices.push_back(Vector2(b.x,a.y));vertices.push_back(b);vertices.push_back(Vector2(a.x,b.y));
    for (int i=0;i<4;++i) uv.push_back(Vector2(-2,-2));
    PackedColorArray colors;colors.push_back(color);
    draw_polygon(vertices,colors,uv);
}
void Mir3HudView::_input(const Ref<InputEvent>& event) {
    if (group_request && group_dialog_input(event)) return;
    if (guild_editor.visible && guild_editor_input(event)) return;
    Ref<InputEventMouseMotion> motion=event;
    if (motion.is_valid()) {
        const auto p=motion->get_position();
        inventory_cursor=p;
        if (chat_history.scroll(model.layout.expanded).captured() && scroll_input(1,p)) return;
        if (minimap_pointer(1,p,motion->is_ctrl_pressed()) || panel_pointer(1,p)) return;
        controller.motion(int(p.x),int(p.y));
        queue_redraw();
    }
    Ref<InputEventMouseButton> button=event;
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
    const auto covers_wheel=[&](const mir3::DraggablePanel& panel) {
        return button.is_valid() && panel.visible && rect(panel.rect).has_point(button->get_position());
    };
    if (covers_wheel(quest) && !panel_covered(Panel::quest,button->get_position()) && button->is_pressed()) {
        const auto index=button->get_button_index();
        if (index==MOUSE_BUTTON_WHEEL_UP || index==MOUSE_BUTTON_WHEEL_DOWN) {
            quest.scroll(index==MOUSE_BUTTON_WHEEL_UP?-1:1);
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
        !covers_wheel(status) && !covers_wheel(inventory) && !covers_wheel(mount) && !covers_wheel(quest) && !covers_wheel(trade)) {
        chat_history.scroll(model.layout.expanded).wheel(button->get_button_index()==MOUSE_BUTTON_WHEEL_UP?1:-1,int(chat_history.lines(model.layout.expanded).size()),model.layout.expanded);
        get_viewport()->set_input_as_handled();queue_redraw();return;
    }
    if (button.is_valid() && button->get_button_index()==MOUSE_BUTTON_LEFT) {
        auto p=button->get_position();
        inventory_cursor=p;
        const int kind=button->is_pressed()?0:2;
        if (kind==2 && chat_history.scroll(model.layout.expanded).captured() && scroll_input(kind,p)) return;
        if (skill_menu_input(kind,p,button->is_double_click())) return;
        if (minimap_pointer(kind,p,button->is_ctrl_pressed()) || panel_pointer(kind,p) || scroll_input(kind,p)) return;
        const auto entry=frame(2096);
        const auto anchor=mir3::HudLayout::portrait_entry_position;
        const bool portrait_hit=!model.layout.expanded && entry.is_valid() &&
            Rect2(Vector2(anchor[0],anchor[1]),entry->get_size()).has_point(p) && Rect2(535,486,81,94).has_point(p);
        if (portrait_hit || portrait_entry_pressed) {
            portrait_entry_pressed=kind==0 && portrait_hit;
            if (kind==2 && portrait_hit) {
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
            chat_requested(String::utf8(message(id).c_str()));
        }
        if (action==0) request_trade();
        if (action==1) toggle_minimap(100);
        if (action==2) skill_shortcuts_visible=!skill_shortcuts_visible;
        if (action==3 || action==4) open_group_dialog(action==3?109:101);
        if (action==6) emit_signal("guild_overview_requested");
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
        if (captured) get_viewport()->set_input_as_handled();
        queue_redraw();
    }
}
void Mir3HudView::_notification(int what) {
    if (what==NOTIFICATION_APPLICATION_FOCUS_OUT) { minimap.dragging=false;trade.end_drag();mount.end_drag();mount.pressed=-1;quest.end_drag();quest.pressed=-1;quest.scroll_captured=false;controller.cancel();portrait_entry_pressed=false;portrait_pressed=-1;portrait_picker.scrolling=false;options.end_drag();audio_drag=-1;guild.end_drag();guild_layout.pressed=-1;guild_scroll.captured=false;group.end_drag();group_pressed=-1;skills.end_drag();skills.pressed=-1;skills.scroll_captured=false;inventory_cursor=Vector2(-1,-1);status.end_drag();status_close_pressed=false;inventory.end_drag();inventory.close_pressed=false;inventory.scroll.captured=false;chat_history.input(2,0,0,false,0,0,0);chat_history.input(2,0,0,true,0,0,0);queue_redraw(); }
}

void Mir3HudView::draw_inventory() {
    const auto [x,y,w,h]=inventory.rect;
    const auto skin=frame(1220);
    if (skin.is_valid())
        sprite(1220,{x-(skin->get_width()-w)/2,y-(skin->get_height()-h)/2});
    text(message(1701),{x+12,y+20,74,16},0xF8DCFA);
    const auto weight=message(2501)+": "+std::to_string(model.weight)+" / "+std::to_string(model.max_weight);
    text(weight,{x+95,y+21,126,16},0x0a0a0a);
    text(weight,{x+94,y+20,126,16},0xF8C8C8);
    sprite(inventory.close_pressed?1222:1221,{x+242,y+423});
    sprite(1225,{x+250,mir3::InventoryScroll::thumb_y(inventory.row,y)});
    text("0",{x+72,y+402,77,17},0x64C8F8,mir3::TextFont::gold);
    const Rect2 clip(x+17,y+60,228,304);
    // 0x440280 shares placement between the held-item preview and release.
    if (bag.held && clip.has_point(inventory_cursor)) {
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
        draw_texture_rect_region(icon,visible,Rect2(visible.position-target.position,visible.size));
        const int label_x=int(bounds.position.x)+18,label_y=int(bounds.position.y)+22;
        if (item.count>1 && clip.encloses(Rect2(label_x,label_y,20,16)))
            text(std::to_string(item.count),{label_x,label_y,20,16},0xffffff,mir3::TextFont::normal,false);
    }

}

void Mir3HudView::toggle_inventory() {
    if (frame(1220).is_null()) return;
    inventory.toggle();
    if (inventory.visible) raise_panel(Panel::inventory);
    inventory.close_pressed=false;
    inventory.scroll.captured=false;
    controller.cancel();
    queue_redraw();
}

bool Mir3HudView::inventory_pointer(int kind,Vector2 position) {
    if (!inventory.visible) return false;
    const auto [x,y,w,h]=inventory.rect;
    const auto close=frame(1221);
    const bool on_close=close.is_valid() && Rect2(x+242,y+423,close->get_width(),close->get_height()).has_point(position);
    const bool inside=Rect2(x,y,w,h).has_point(position);
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
        if (!consumed && !inventory.dragging && !inventory.close_pressed) {
            bag.scroll_row=inventory.row;
            consumed=bag.slot_at(int(position.x),int(position.y),x,y)>=0;
            if (consumed && !equipment.pending && bag.held_equipment_slot>=0) {
                if (equipment.takeoff(bag)) emit_equipment_request();
            } else if (consumed && !equipment.pending) {
                const auto tick=static_cast<std::uint32_t>(now);
                const int action=bag.release_action(int(position.x),int(position.y),x,y,tick);
                if (action==1) emit_signal("inventory_use_requested",int64_t(bag.held.make_index),int64_t(tick),String::utf8(bag.held.name.c_str()));
                if (action==2) post_chat(String::utf8(message(1051).c_str()),11861940,0);
            }
        }
        consumed=inventory.end_drag() || inventory.close_pressed || consumed;
        inventory.close_pressed=false;
        if (close_window) toggle_inventory();
    }
    bag.scroll_row=inventory.row;
    if (!inside && !consumed) return false;
    controller.cancel();
    get_viewport()->set_input_as_handled();
    queue_redraw();
    return true;
}

void Mir3HudView::_unhandled_key_input(const Ref<InputEvent>& event) {
    Ref<InputEventKey> key=event;
    if (key.is_null() || !key->is_pressed() || key->is_echo()) return;
    const auto code=key->get_physical_keycode();
    if (code>=KEY_F1 && code<=KEY_F12) {
        const int slot=code-KEY_F1+1;
        if (!bind_skill_at_cursor(slot)) {
            const int id=bound_skill(slot);
            if (id && spell_pointer_available()) emit_signal("spell_key_requested",slot,id,inventory_cursor);
        }
        get_viewport()->set_input_as_handled();
        return;
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
    if (code==KEY_V || (minimap.visible && (code==KEY_T || code==KEY_Y))) {
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
            }
            get_viewport()->set_input_as_handled();
            return;
        }
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
    if (!bag.apply_use_reply(identifier,remaining,inventory.rect[0],inventory.rect[1],
                             int(inventory_cursor.x),int(inventory_cursor.y))) return false;
    queue_redraw();
    return true;
}

void Mir3HudView::draw_status() {
    using Layout=mir3::StatusLayout;
    const auto [x,y,w,h]=status.rect;
    const auto skin=frame(1280);
    if (skin.is_valid()) sprite(1280,{x-(skin->get_width()-w)/2,y-(skin->get_height()-h)/2});
    sprite(status_close_pressed?1222:1221,{x+Layout::close[0],y+Layout::close[1]});
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
                draw_polygon(vertices,colors,uv,highlight);
            }
        }
    }
    if (status_body.is_valid())
        draw_texture(status_body,Vector2(x+Layout::body[0],y+Layout::body[1])+status_body_offset);
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
        draw_texture(icon,Vector2(x+pos[0],y+pos[1]));
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
        draw_rect(bounds,Color("644b32"),false);
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
        if (activate) toggle_status();
        if (!captured && slot>=0 && (!bag.held || bag.held.make_index)) {
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

bool Mir3HudView::panel_pointer(int kind,Vector2 position) {
    if (trade.dragging) return trade_pointer(kind,position);
    if (quest.dragging || quest.pressed>=0 || quest.scroll_captured) return quest_pointer(kind,position);
    if (mount.dragging || mount.pressed>=0) return mount_pointer(kind,position);
    if (portrait_picker.scrolling || portrait_pressed>=0) return portrait_input(kind,position);
    if (options.dragging || audio_drag>=0) return options_pointer(kind,position);
    if (guild.dragging || guild_scroll.captured || guild_layout.pressed>=0) return guild_pointer(kind,position);
    if (status.dragging || status_close_pressed) return status_pointer(kind,position);
    if (inventory.dragging || inventory.close_pressed || inventory.scroll.captured)
        return inventory_pointer(kind,position);
    if (skills.dragging || skills.pressed>=0 || skills.scroll_captured) return skill_pointer(kind,position);
    if (group.dragging || group_pressed>=0) return group_pointer(kind,position);
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
        draw_texture_rect_region(tree,Rect2(Vector2(x+origin[0],y+origin[1]),crop.size),crop);
    }
    sprite(1672,{x+322,y+skills.thumb_y()});
    const auto& learned=skills.learned[skills.tab];
    for (int i=0;i<int(learned.size());++i) {
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
                draw_rect(Rect2(left,upper,42,1),edge);
                draw_rect(Rect2(left,upper+41,42,1),edge);
                draw_rect(Rect2(left,upper,1,42),edge);
                draw_rect(Rect2(left+41,upper,1,42),edge);
            }
            text(skill.name,{x+20,y+387,80,61},0xfafafa,mir3::TextFont::normal,true,true);
            const auto level=skill.level>=3?std::string("Level : Max"):"Level : "+std::to_string(skill.level);
            text(level,{x+20,y+403,80,62},0xfafafa,mir3::TextFont::normal,true,true);
            if (skill.level<3)
                text("Exp : "+std::to_string(skill.experience)+"/"+std::to_string(skill.required),
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
            draw_texture_rect_region(icon,Rect2(Vector2(x+pos.x,y+top+hidden_top),crop.size),crop);
        };
        if (const auto icon=skill_frames.find(id);icon!=skill_frames.end()) draw_icon(icon->second);
        if (skill.key) draw_icon(frame(1659+skill.key));
        if (top+37>56 && top+37<385)
            text(std::to_string(skill.level),{x+pos.x+34,y+top+37,15,15},0x64fafa,mir3::TextFont::skill_level);
    }
    for (const int index:{0,8,7,6,5,4,3,2,1}) {
        const auto [base,left,top]=mir3::SkillPanel::button(index);
        if (index==0 || skills.pressed==index || skills.learned[index-1][0].id)
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
        if (skills.release_button(button) && button==0) toggle_skills();
    }
    if (!inside && !consumed) return false;
    controller.cancel();
    get_viewport()->set_input_as_handled();
    queue_redraw();
    return true;
}

void Mir3HudView::emit_equipment_request() {
    const auto& request=equipment.request;
    emit_signal("equipment_requested",request.identifier,request.slot,int64_t(request.make_index),String::utf8(request.name.c_str()));
}

bool Mir3HudView::apply_equipment_reply(int identifier) {
    if (!equipment.reply(identifier,bag)) return false;
    queue_redraw();
    return true;
}

bool Mir3HudView::set_learned_skill(int id,int level,int key,int experience,int required,String name) {
    if (id<1 || id>65535 || level<0 || level>255 || key<0 || key>12) return false;
    auto& learned=skills.learned[mir3::SkillLayout::category(id)];
    auto found=std::find_if(learned.begin(),learned.end(),[id](const auto& skill) { return skill.id==id; });
    if (found==learned.end())
        found=std::find_if(learned.begin(),learned.end(),[](const auto& skill) { return skill.id==0; });
    if (found==learned.end()) return false;
    *found={id,level,key,experience,required,name.utf8().get_data()};
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
    if (!skills.visible || skills.scroll_captured || !rect(skills.rect).has_point(inventory_cursor)) return;
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
            std::to_string(skill.experience)+"/"+std::to_string(skill.required);
        text(progress,{x,y,skill.level>=3?80:50,15},0x1414fa);
        break;
    }

}

void Mir3HudView::bind_skill(int id,int key) {
    for (auto& group:skills.learned) for (auto& skill:group) {
        if (!skill.id || skill.key!=key) continue;
        skill.key=0;
        emit_signal("skill_key_requested",skill.id,0);
    }
    for (auto& skill:skills.learned[skills.tab]) {
        if (!skill.id || skill.id!=id) continue;
        skill.key=key;
        const int legacy_key=key<=9?'0'+key:'A'+key-10;
        emit_signal("skill_key_requested",skill.id,legacy_key);
        break;
    }
    queue_redraw();
}

bool Mir3HudView::skill_menu_input(int kind,Vector2 point,bool double_click) {
    if (!skills.visible) return false;
    if (skill_key_menu.has_area()) {
        if (skill_key_menu.has_point(point)) {
            if (kind==2) {
                const Vector2 local=point-skill_key_menu.position;
                const int key=int(local.y)/40*4+int(local.x)/40+1;
                const int selected=skills.selected[skills.tab];
                if (selected>=0 && selected<15)
                    bind_skill(mir3::SkillLayout::positions[skills.tab][selected].id,key);
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
    if (panel_covered(Panel::skills,point)) return false;
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
        skills.selected[skills.tab]=index;
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
    if (minimap.visible || group_request || !Rect2(0,0,800,Layout::origin[1]).has_point(pointer) ||
        rect(Layout::chat_frame(model.layout.expanded)).has_point(pointer)) return false;
    if (guild_editor.visible && rect(guild_editor.rect).has_point(pointer)) return false;
    for (const auto panel:panel_order) {
        const auto& state=panel_state(panel);
        if (state.visible && rect(state.rect).has_point(pointer)) return false;
    }
    return true;
}

int Mir3HudView::bound_skill(int key) const {
    for (const auto& group:skills.learned) for (const auto& skill:group)
        if (skill.id && skill.key==key) return skill.id;
    return 0;
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
                draw_texture(it->second,Vector2(x,2),Color(1,1,1,200.f/255));
        }
        const auto label=frame(1450+index);
        if (label.is_valid()) draw_texture(label,Vector2(x-13,-11),Color(150.f/255,150.f/255,150.f/255,150.f/255));
    }
}

void Mir3HudView::draw_skill_key_menu() {
    if (!skills.visible || !skill_key_menu.has_area()) return;
    legacy_rect(skill_key_menu,Color(100.f/255,100.f/255,50.f/255,150.f/255));
    for (int index=0;index<12;++index) {
        const Vector2 point=skill_key_menu.position+Vector2(40*(index%4),40*(index/4));
        for (const auto& group:skills.learned) for (const auto& skill:group) {
            if (skill.id && skill.key==index+1) {
                const int frame_id=2*(skill.id&255)-2;
                if (!skill_frames.contains(frame_id)) {
                    const auto decoded=magic_icons.frame(frame_id);
                    if (decoded) skill_frames.emplace(frame_id,texture(decoded->pixels.rgba,decoded->width,decoded->height));
                }
                if (const auto it=skill_frames.find(frame_id);it!=skill_frames.end()) draw_texture(it->second,point);
            }
        }
        sprite(1660+index,{int(point.x),int(point.y)});
    }
}

namespace {
constexpr std::array group_button_frames{1221,1362,1365,1368,1370};
constexpr std::array<std::array<int,2>,5> group_button_offsets{{
    {217,200},{38,192},{98,192},{158,192},{23,47}
}};
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
        if (clicked && button==0) toggle_group();
        if (clicked && button>=1 && button<=3) open_group_dialog(button);
        if (clicked && button==4) {
            group_allow=!group_allow;
            emit_signal("group_allow_requested",group_allow);
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

void Mir3HudView::handle_exit_confirmation(int dialog_id) {
    // 109 ends the program. 101 is consumed by the game session when connected.
    if (dialog_id==109) get_tree()->quit();
}

bool Mir3HudView::dialog_is_alert() const {
    return group_request==4000 || group_request==4017 || group_request==4714 || group_request==4716;
}

bool Mir3HudView::dialog_has_input() const {
    return group_request!=101 && group_request!=109 && !dialog_is_alert();
}

void Mir3HudView::open_group_dialog(int action) {
    const auto skin=frame(1240);
    if (skin.is_null() || !chat_input) return;
    const int w=skin->get_width(),h=skin->get_height();
    group_dialog={400-w/2,246-h/2,w,h};
    group_request=action<=3?1019+action:action;
    group_selected=0;
    group_dialog_pressed=-1;
    if (!dialog_has_input()) { chat_input->release_focus();return; }
    chat_input->set_position(Vector2(group_dialog[0]+24,group_dialog[1]+228));
    chat_input->set_size(Vector2(309,15));
    chat_input->focus_chat("",true);
}

void Mir3HudView::finish_group_dialog(bool confirm) {
    const int request=group_request;
    const bool has_input=dialog_has_input();
    const String name=has_input?chat_input->get_text():String();
    group_request=0;
    group_dialog_pressed=-1;
    if (has_input) {
        chat_input->focus_chat("",true);
        chat_input->release_focus();
        chat_input->layout(model.layout.expanded);
    }
    queue_redraw();
    if (!confirm) return;
    if (request==101 || request==109) {
        emit_signal("exit_confirmed",request);
        return;
    }
    if (!has_input || name.is_empty()) return;
    if (request==1039) {
        emit_signal("guild_member_removal_requested",name);
        emit_signal("guild_members_requested");
    } else if (request==1045) {
        chat_requested(String::utf8(message(4016).c_str())+name);
        emit_signal("guild_overview_requested");
    } else {
        emit_signal("group_member_requested",request,name);
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
    const auto prompt=group_request==1039?"'"+std::string(guild_name.utf8().get_data())+"' "+message(4001):
        message(group_request==1045?4002:group_request==1022?1606:1605);
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
        const bool activate=code==KEY_ENTER || code==KEY_KP_ENTER || (!dialog_has_input() && code==KEY_SPACE);
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
                if (clicked) finish_group_dialog(hit==0);
            }
            queue_redraw();
        }
    }
    get_viewport()->set_input_as_handled();
    return true;
}

void Mir3HudView::set_guild_overview(String name,PackedStringArray lines,PackedStringArray editable) {
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
    text(message(mir3::GuildLayout::title(guild_layout.tab)),{x+45,y+10,112,25},0x96c8ff);
    text(guild_name.utf8().get_data(),{x+340,y+10,213,24},0x96c8ff);
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
            text(value,bounds,heading?0x0097ff:0xffffff,mir3::TextFont::normal,false);
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
                if (hit==0) guild.toggle();
                if (hit>=1 && hit<=3) guild_first=0;
                if (hit==1) emit_signal("guild_overview_requested");
                if (hit==2) emit_signal("guild_members_requested");
                if (mir3::GuildLayout::denial(hit,guild_manager)==4000) open_group_dialog(4000);
                else if (hit==4 || hit==7) open_guild_editor(hit==4);
                else if (hit==5) open_group_dialog(1039);
                else if (hit==8) open_group_dialog(1045);
            }
        }
    }
    if (!inside && !captured) return false;
    controller.cancel();
    get_viewport()->set_input_as_handled();
    queue_redraw();
    return true;
}

void Mir3HudView::set_guild_members(PackedStringArray lines,PackedStringArray editable) {
    guild_editable[1]=editable;
    guild_pages[1]=lines;
    if (guild_layout.tab==1) guild_first=std::min(guild_first,std::max(0,int(lines.size())-1));
    queue_redraw();
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
    guild_edit_ansi=mir3::edit_multiline_ansi("",initial.utf8().get_data());
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
    label(guild_edit_announcement?1903:1900,70,20,0x96c8ff);
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
                    if (!body.empty() && body.size()<=3999)
                        emit_signal("guild_edit_requested",guild_edit_announcement?1040:1041,String::utf8(mir3::ansi_to_utf8(body).c_str()));
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
    audio_drag=-1;
    controller.cancel();
    queue_redraw();
}

void Mir3HudView::draw_options() {
    const int x=options.rect[0],y=options.rect[1];
    sprite(1290,{x,y});
    text(message(2104),{x+60,y+30,390,15},0xf8fafa);
    for (int i=0;i<3;++i) {
        const bool enabled=i==0?audio_options.music_enabled:i==1?audio_options.effects_enabled:audio_options.reverse_pan;
        sprite(enabled?1293:1294,{x+430,y+85+60*i});
        text(message(2100+i),{x+266,y+89+60*i,156,21},0x0096ff);
        if (i<2) {
            const int level=i==0?audio_options.music_level:audio_options.effects_level;
            sprite(1291,{x+276+mir3::AudioOptions::knob_from_level(level),y+118+60*i});
        }
    }
    sprite(magic_light?1293:1294,{x+186,y+115});
    sprite(damage_numbers?1293:1294,{x+186,y+145});
    text(message(2107),{x+20,y+122,155,15},0x0096ff);
    text(message(2108),{x+20,y+152,155,15},0x0096ff);
    sprite(1221,{x+448,y+415});
}

bool Mir3HudView::options_pointer(int kind,Vector2 point) {
    if (!options.visible) return false;
    const Vector2 local=point-Vector2(options.rect[0],options.rect[1]);
    const bool inside=rect(options.rect).has_point(point);
    const bool captured=options.dragging || audio_drag>=0;
    if (!inside && !captured) return false;
    const auto hit=[&](int id,int x,int y) {
        const auto image=frame(id);
        return image.is_valid() && Rect2(Vector2(x,y),image->get_size()).has_point(local);
    };
    bool changed=false;
    if (audio_drag>=0) {
        if (kind==1) audio_options.drag_volume(audio_drag==0,int(local.x));
        if (kind==2) { audio_drag=-1;changed=true; }
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
        toggle(magic_light,186,115);
        toggle(damage_numbers,186,145);
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
    magic_light=read(L"MagicLight",1)!=0;
    damage_numbers=read(L"BloodNum",1)!=0;
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
    saved=write(L"MagicLight",magic_light) && saved;
    saved=write(L"BloodNum",damage_numbers) && saved;
    if (!saved) UtilityFunctions::push_warning("Could not save system options to ",options_path);
}

void Mir3HudView::sync_hud_sound() {
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
    quest.selected=-1;
    quest.expanded=-1;
    quest.first=0;
    quest.scroll_captured=false;
    queue_redraw();
    return true;
}

bool Mir3HudView::set_quest_details(int index,PackedStringArray lines) {
    if (index<0 || index>=int(quest.entries.size()) || lines.size()>INT_MAX-int(quest.entries.size())) return false;
    auto& details=quest.entries[index].details;
    details.clear();
    details.reserve(lines.size());
    constexpr int wrap_threshold=170; // 0x46C3C7 measures each character prefix.
    for (int64_t i=0;i<lines.size();++i) {
        const String paragraph=lines[i];
        String row;
        for (int64_t offset=0;offset<paragraph.length();++offset) {
            row+=paragraph.substr(offset,1);
            const auto utf8=row.utf8();
            if (mir3::raster_text(utf8.get_data(),1,1,false,mir3::TextFont::normal).text_width<wrap_threshold) continue;
            details.emplace_back(utf8.get_data());
            row=String();
        }
        if (!row.is_empty() || paragraph.is_empty()) details.emplace_back(row.utf8().get_data());
    }
    quest.scroll(0);
    queue_redraw();
    return true;
}

void Mir3HudView::toggle_quest() {
    quest.toggle();
    quest.scroll_captured=false;
    quest.pressed=-1;
    if (quest.visible) raise_panel(Panel::quest);
    queue_redraw();
}

void Mir3HudView::draw_quest() {
    const auto [x,y,w,h]=quest.rect;
    const auto skin=frame(quest.background);
    if (skin.is_valid()) {
        const Vector2 anchor(x-(skin->get_width()-w)/2,y-(skin->get_height()-h)/2);
        draw_texture(skin,anchor,Color(1,1,1,220.f/255));
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
                const auto& detail=quest.entries[quest.expanded].details[line-quest.expanded-1];
                text(detail,{x+75,top,190,15},quest.title_color(quest.expanded),mir3::TextFont::normal,false);
            }
        }
    }
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
            if (index>=0) quest.selected=index;
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
                if (quest.entries[quest.selected].details.empty())
                    emit_signal("quest_detail_requested",quest.selected);
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
    return mount.apply_reply(identifier,static_cast<std::uint32_t>(recognition));
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
        draw_texture(skin,anchor,Color(1,1,1,220.f/255));
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
            else if (const int command=mount.command(hit);command!=0)
                chat_requested(String::utf8(message(command).c_str()));
        }
    }
    if (!inside && !captured) return false;
    controller.cancel();
    get_viewport()->set_input_as_handled();
    queue_redraw();
    return true;
}

void Mir3HudView::toggle_minimap(std::uint64_t cooldown) {
    if (minimap.toggle(ticks_msec(),cooldown)) emit_signal("minimap_requested",1033);
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
    queue_redraw();
    return true;
}

bool Mir3HudView::open_trade(String partner) {
    if (partner.is_empty()) {
        if (trade.visible) trade.toggle();
        queue_redraw();
        return true;
    }
    if (frame(1390).is_null()) return false;
    if (trade.visible) return true;
    trade_partner=partner;
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
}

bool Mir3HudView::trade_pointer(int kind,Vector2 position) {
    const bool inside=rect(trade.rect).has_point(position);
    bool captured=false;
    if (kind==0 && inside) {
        raise_panel(Panel::trade);
        trade.begin_drag(int(position.x),int(position.y),position.y>=trade.rect[1]+56);
    } else if (kind==1) captured=trade.move_drag(int(position.x),int(position.y));
    else if (kind==2) captured=trade.end_drag();
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
    emit_signal("trade_requested",1025,trade_actor_name);
    return true;
}

void Mir3HudView::set_map_location(String name,int x,int y) {
    const auto utf8=name.utf8();
    const bool changed=model.map_name!=utf8.get_data();
    model.map_name=utf8.get_data();
    set_minimap_position(x,y);
    if (!changed) return;
    trade_actor_name=String();

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
    if (refresh) emit_signal("minimap_requested",1033);
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
    draw_texture_rect_region(minimap_image,bounds,Rect2(source,bounds.size),
        Color(light,light,light,minimap.translucent?175.f/255:1.f));
    draw_rect(bounds,Color(100.f/255,100.f/255,100.f/255),false);
    const auto origin=bounds.position-source;
    const auto player=origin+Vector2(float(mir3::Minimap::map_pixel_x(minimap.player_x)-2),
        float(std::int64_t(minimap.player_y)-2));
    const auto draw_markers=[&](const auto& cells,Color color,bool nearby) {
        for (const auto& [x,y]:cells) {
            const auto marker=origin+Vector2(mir3::Minimap::actor_marker_x(x),y-1);
            if (nearby && (std::abs(marker.x-player.x)>=minimap.size()/2 ||
                std::abs(marker.y-player.y)>=minimap.size()/2)) continue;
            const auto area=Rect2(marker-Vector2(nearby,nearby),Vector2(nearby?3:2,nearby?3:2)).intersection(bounds);
            if (area.has_area()) draw_rect(area,color);
        }
    };
    draw_markers(minimap.type_five_actors,Color(1,1,0),false);
    draw_markers(minimap.nearby_green,Color(100.f/255,250.f/255,100.f/255),true);
    draw_markers(minimap.nearby_orange,Color(250.f/255,150.f/255,50.f/255),true);
    if (ticks_msec()%801<500) {
        const auto area=Rect2(player,Vector2(4,4)).intersection(bounds);
        if (area.has_area()) draw_rect(area,Color(100.f/255,250.f/255,100.f/255),false);
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
    } else if (kind==0 && Rect2(end-Vector2(38,19),Vector2(19,19)).has_point(position)) {
        minimap.translucent=!minimap.translucent;
    } else if (kind==0 && Rect2(end-Vector2(19,19),Vector2(19,19)).has_point(position)) {
        minimap.resize();
    } else if (kind==1 && minimap.dragging && ctrl && bounds.has_point(position)) {
        minimap.position={int(position.x)-minimap.drag_offset[0],int(position.y)-minimap.drag_offset[1]};
    } else if (kind==2 && minimap.dragging) {
        minimap.dragging=false;
    } else {
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
    character_name=name;
    character_secondary=secondary;
    character_tertiary=tertiary;
    character_badge=badge;
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
    if (portrait_preview.is_valid()) draw_texture_rect(portrait_preview,rect(Picker::preview),false);
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
    }
    return inventory;
}

bool Mir3HudView::panel_covered(Panel panel,Vector2 point) const {
    const auto covers=[&](const mir3::DraggablePanel& state) {
        return state.visible && rect(state.rect).has_point(point);
    };
    if (covers(guild_editor) || group_request)
        return true;
    for (auto it=panel_order.rbegin();it!=panel_order.rend() && *it!=panel;++it)
        if (covers(panel_state(*it))) return true;
    return false;
}
