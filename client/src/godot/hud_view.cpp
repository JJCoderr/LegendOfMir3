#include "hud_view.hpp"
#include <godot_cpp/classes/config_file.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/shader.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <chrono>
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
}
void Mir3HudView::_bind_methods() {
    ClassDB::bind_method(D_METHOD("add_inventory_item","icon","count","slot","make_index","name","std_mode","stack_mode"),&Mir3HudView::add_inventory_item,DEFVAL(0),DEFVAL(String()),DEFVAL(40),DEFVAL(0));
    ClassDB::bind_method(D_METHOD("apply_inventory_use_reply","identifier","remaining"),&Mir3HudView::apply_inventory_use_reply);
    ClassDB::bind_method(D_METHOD("apply_equipment_reply","identifier"),&Mir3HudView::apply_equipment_reply);
    ADD_SIGNAL(MethodInfo("equipment_requested",PropertyInfo(Variant::INT,"identifier"),PropertyInfo(Variant::INT,"slot"),PropertyInfo(Variant::INT,"make_index"),PropertyInfo(Variant::STRING,"name")));
    ADD_SIGNAL(MethodInfo("inventory_use_requested",PropertyInfo(Variant::INT,"make_index"),PropertyInfo(Variant::INT,"tick"),PropertyInfo(Variant::STRING,"name")));
    ClassDB::bind_method(D_METHOD("hud_snapshot"),&Mir3HudView::hud_snapshot);
    ClassDB::bind_method(D_METHOD("post_chat","value","foreground","background"),&Mir3HudView::post_chat);
    ClassDB::bind_method(D_METHOD("update_chat_rejection","channel","rejected"),&Mir3HudView::update_chat_rejection);
    ADD_SIGNAL(MethodInfo("hud_action",PropertyInfo(Variant::INT,"button")));
    ADD_SIGNAL(MethodInfo("chat_requested",PropertyInfo(Variant::STRING,"text")));
}
Dictionary Mir3HudView::hud_snapshot() const {
    Dictionary result;
    result["ready"]=loaded;
    result["expanded"]=model.layout.expanded;
    result["pressed"]=model.pressed;
    result["hovered"]=model.hovered;
    result["primary_defence"]=model.primary_stat[0];
    result["primary_attack"]=model.primary_stat[1];
    result["chat_compact_count"]=int(chat_history.lines(false).size());
    result["chat_expanded_count"]=int(chat_history.lines(true).size());
    result["chat_compact_first"]=chat_history.first(false);
    result["chat_expanded_first"]=chat_history.first(true);
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
    Ref<ConfigFile> config; config.instantiate();
    if (config->load("res://resource_path.cfg")!=OK) {
        UtilityFunctions::push_error("Missing resource_path.cfg; configure the client with CMake."); return;
    }
    String root=config->get_value("resources","root","");
    loaded=archive.open(std::filesystem::u8path(root.utf8().get_data())/"Data"/"GameInter");
    if (!loaded || frame(1160).is_null()) {
        loaded=false; UtilityFunctions::push_error("Cannot read original GameInter WIL/WIX resources."); return;
    }
    const auto resource_root=std::filesystem::u8path(root.utf8().get_data());
    if (!portraits.open(resource_root/"Data"/"ProgUse")) UtilityFunctions::push_error("Cannot read ProgUse portraits.");
    if (const auto body=portraits.frame(model.appearance)) {
        status_body=texture(body->pixels.rgba,body->width,body->height);
        status_body_offset=Vector2(body->offset_x,body->offset_y);
    }
    equipment_archive.open(resource_root/"Data"/"Equip");
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
void Mir3HudView::text(const std::string& value,mir3::HudLayout::Rect r,unsigned color,mir3::TextFont font,bool centered) {
    if (value.empty()) return;
    const auto key=std::to_string(r[2])+":"+std::to_string(r[3])+":"+std::to_string(int(font))+":"+std::to_string(centered)+":"+value;
    auto found=text_cache.find(key);
    if (found==text_cache.end()) {
        if (text_cache.size()>=256) text_cache.clear();
        const auto bitmap=mir3::raster_text(value,r[2],r[3],centered,font,messages.get(1074));
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
        if (id>=0) {
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
    if (skills.visible && !skills_on_top) draw_skills();
    if (status.visible && !status_on_top) draw_status();
    if (inventory.visible) draw_inventory();
    if (status.visible && status_on_top) draw_status();
    if (skills.visible && skills_on_top) draw_skills();
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
    if (model.hovered<0) return;
    const int hint=chat_input && (model.hovered==18 || model.hovered==19)?chat_input->mode.tooltip(model.hovered):
        model.hovered>=20 && model.hovered<=22?reception_buttons.hints[model.hovered-20]:2600+model.hovered;
    const auto caption=message(hint);
    if (caption.empty()) return;
    const auto measured=mir3::raster_text(caption,800,24,false,mir3::TextFont::normal);
    const auto hit=model.button_rect(model.hovered);
    const auto bounds=button_tooltip.draw(caption,{measured.text_width,measured.text_height},{hit[0]+hit[2],hit[1]+hit[3]});
    draw_rect(rect(bounds),Color(0,0,0));
    if (bounds[2]>2 && bounds[3]>2) draw_rect(rect(bounds).grow(-1),Color(1,1,150.f/255));
    text(caption,bounds,0);
}
void Mir3HudView::_process(double) {
    // Original button tooltip grows once per draw, not once per mouse event.
    if (model.hovered>=0) queue_redraw();
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
    Ref<InputEventMouseMotion> motion=event;
    if (motion.is_valid()) { auto p=motion->get_position();inventory_cursor=p;if (panel_pointer(1,p)) return;controller.motion(int(p.x),int(p.y));if (scroll_input(1,p)) return;queue_redraw(); }
    Ref<InputEventMouseButton> button=event;
    if (button.is_valid() && button->is_pressed() && (button->get_button_index()==MOUSE_BUTTON_WHEEL_UP || button->get_button_index()==MOUSE_BUTTON_WHEEL_DOWN) && rect(mir3::HudLayout::chat_frame(model.layout.expanded)).has_point(button->get_position())) {
        chat_history.scroll(model.layout.expanded).wheel(button->get_button_index()==MOUSE_BUTTON_WHEEL_UP?1:-1,int(chat_history.lines(model.layout.expanded).size()),model.layout.expanded);
        get_viewport()->set_input_as_handled();queue_redraw();return;
    }
    if (button.is_valid() && button->get_button_index()==MOUSE_BUTTON_LEFT) {
        auto p=button->get_position();
        inventory_cursor=p;
        const int kind=button->is_pressed()?0:2;
        if (panel_pointer(kind,p) || scroll_input(kind,p)) return;
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
        if (action==7) toggle_skills();
        if (action==12) toggle_inventory();
        if (action==13) toggle_status();
        if (action>=0) emit_signal("hud_action",action);
        if (captured) get_viewport()->set_input_as_handled();
        queue_redraw();
    }
}
void Mir3HudView::_notification(int what) {
    if (what==NOTIFICATION_APPLICATION_FOCUS_OUT) { controller.cancel();skills.end_drag();skills.pressed=-1;skills.scroll_captured=false;inventory_cursor=Vector2(-1,-1);status.end_drag();status_close_pressed=false;inventory.end_drag();inventory.close_pressed=false;inventory.scroll.captured=false;chat_history.input(2,0,0,false,0,0,0);chat_history.input(2,0,0,true,0,0,0);queue_redraw(); }
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
    if (inventory.visible) skills_on_top=false;
    if (inventory.visible) status_on_top=false;
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
        skills_on_top=false;
        status_on_top=false;
        inventory.close_pressed=on_close;
        inventory.begin_drag(int(position.x),int(position.y),on_close || Rect2(x+17,y+60,228,304).has_point(position) || Rect2(x+15,y+390,42,27).has_point(position));
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
    if (code==KEY_E || (code==KEY_ESCAPE && skills.visible && (skills_on_top || (!status.visible && !inventory.visible)))) {
        toggle_skills();
        get_viewport()->set_input_as_handled();
        return;
    }
    if (code==KEY_Q || (code==KEY_ESCAPE && status.visible && (status_on_top || !inventory.visible))) {
        toggle_status();
        get_viewport()->set_input_as_handled();
        return;
    }
    if (code==KEY_W || (code==KEY_ESCAPE && inventory.visible)) {
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
    const auto [bag_x,bag_y,bag_width,bag_height]=inventory.rect;
    const bool obscured=inventory.visible && !status_on_top &&
        Rect2(bag_x,bag_y,bag_width,bag_height).has_point(inventory_cursor);
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
        {model.hp,model.max_hp},{model.mp,model.max_mp},{model.weight,model.max_weight},{0,0},{0,0}}};
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
    for (int i=7;i<9;++i) label(Layout::value(i),"+0%");
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
            const auto r=Layout::element_rect(row,column,0);
            sprite(Layout::element_frame(row,column),{x+r[0],y+r[1]});
            label(Layout::element_rect(row,column,1),"0");
            if (hovered_element==row*7+column) hint(Layout::element_rect(row,column,3),message(5161+column));
        }
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
    if (status.visible) skills_on_top=false;
    status_close_pressed=false;
    if (status.visible) status_on_top=true;
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
        skills_on_top=false;
        status_close_pressed=on_close;
        if (status.begin_drag(int(position.x),int(position.y),on_close || slot>=0)) status_on_top=true;
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
    if (status.dragging || status_close_pressed) return status_pointer(kind,position);
    if (inventory.dragging || inventory.close_pressed || inventory.scroll.captured)
        return inventory_pointer(kind,position);
    if (skills.dragging || skills.pressed>=0 || skills.scroll_captured) return skill_pointer(kind,position);
    if (skills_on_top && skill_pointer(kind,position)) return true;
    const bool handled=status_on_top ? status_pointer(kind,position) || inventory_pointer(kind,position)
                                     : inventory_pointer(kind,position) || status_pointer(kind,position);
    return handled || (!skills_on_top && skill_pointer(kind,position));
}

void Mir3HudView::toggle_skills() {
    if (frame(1620).is_null()) return;
    skills.toggle();
    skills.pressed=-1;
    skills.scroll_captured=false;
    if (skills.visible) skills_on_top=true;
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
    for (const int index:{0,8,7,6,5,4,3,2,1}) {
        const auto [base,left,top]=mir3::SkillPanel::button(index);
        if (index==0 || skills.pressed==index)
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
        skills_on_top=true;
        if (!consumed) {
            skills.pressed=skill_button_at(position);
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
