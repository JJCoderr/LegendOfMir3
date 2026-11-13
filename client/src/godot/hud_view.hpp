#pragma once
#include "hud/hud_model.hpp"
#include "resources/wil_archive.hpp"
#include "text/native_text.hpp"
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <map>

class Mir3HudView : public godot::Node2D {
    GDCLASS(Mir3HudView,godot::Node2D)
public:
    void _ready() override;
    void _draw() override;
    void _input(const godot::Ref<godot::InputEvent>& event) override;
    void _notification(int what);
protected:
    static void _bind_methods();
private:
    mir3::HudModel model;
    mir3::HudController controller{model};
    mir3::WilArchive archive;
    bool loaded=false;
    std::map<int,godot::Ref<godot::ImageTexture>> frames;
    std::map<std::string,godot::Ref<godot::ImageTexture>> text_cache;
    godot::Ref<godot::ImageTexture> frame(int id);
    void sprite(int id,mir3::HudLayout::Point position);
    void text(const std::string& value,mir3::HudLayout::Rect rect,unsigned color,mir3::TextFont font=mir3::TextFont::normal);
    void buttons();
};
