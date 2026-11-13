#include "hud_view.hpp"
#include <godot_cpp/classes/config_file.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/shader.hpp>
#include <godot_cpp/classes/shader_material.hpp>
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
    ClassDB::bind_method(D_METHOD("hud_snapshot"),&Mir3HudView::hud_snapshot);
    ADD_SIGNAL(MethodInfo("hud_action",PropertyInfo(Variant::INT,"button")));
}
Dictionary Mir3HudView::hud_snapshot() const {
    Dictionary result;
    result["ready"]=loaded;
    result["expanded"]=model.layout.expanded;
    result["pressed"]=model.pressed;
    result["hovered"]=model.hovered;
    result["primary_defence"]=model.primary_stat[0];
    result["primary_attack"]=model.primary_stat[1];
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
    std::ifstream message_file(resource_root/"CmList.dat",std::ios::binary);
    std::vector<std::uint8_t> message_bytes{std::istreambuf_iterator<char>(message_file),{}};
    if (!messages.load(message_bytes)) UtilityFunctions::push_error("Cannot read CmList.dat captions.");
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
void Mir3HudView::text(const std::string& value,mir3::HudLayout::Rect r,unsigned color,mir3::TextFont font) {
    if (value.empty()) return;
    const auto key=std::to_string(r[2])+":"+std::to_string(r[3])+":"+std::to_string(int(font))+":"+value;
    auto found=text_cache.find(key);
    if (found==text_cache.end()) {
        if (text_cache.size()>=256) text_cache.clear();
        const auto bitmap=mir3::raster_text(value,r[2],r[3],true,font);
        found=text_cache.emplace(key,texture(bitmap.rgba,r[2],r[3])).first;
    }
    if (found->second.is_valid()) draw_texture(found->second,Vector2(r[0],r[1]),Color(float(color&255)/255,float((color>>8)&255)/255,float((color>>16)&255)/255));
}
void Mir3HudView::buttons() {
    for (int i=mir3::HudLayout::count(model.layout.expanded)-1;i>=0;--i) {
        const auto button=mir3::HudLayout::button(i,model.layout.expanded);
        for (auto id:mir3::main_button_frames(i,model.hovered==i,model.pressed==i))
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
    sprite(1207,{619,model.layout.expanded?342:507});
    buttons();
    button_hint();
}
std::string Mir3HudView::message(int id) const { return mir3::ansi_to_utf8(messages.get(id)); }
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
    const auto caption=message(model.hovered==22?2643:2600+model.hovered);
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
    if (motion.is_valid()) { auto p=motion->get_position();controller.motion(int(p.x),int(p.y));queue_redraw(); }
    Ref<InputEventMouseButton> button=event;
    if (button.is_valid() && button->get_button_index()==MOUSE_BUTTON_LEFT) {
        auto p=button->get_position();
        const bool captured=model.pressed>=0 || controller.hit(int(p.x),int(p.y))>=0;
        int action=controller.pointer(int(p.x),int(p.y),button->is_pressed());
        if (action>=0) emit_signal("hud_action",action);
        if (captured) get_viewport()->set_input_as_handled();
        queue_redraw();
    }
}
void Mir3HudView::_notification(int what) {
    if (what==NOTIFICATION_APPLICATION_FOCUS_OUT) { controller.cancel();queue_redraw(); }
}
