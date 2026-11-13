#pragma once
#include "hud/hud_model.hpp"
#include "inventory/inventory_panel.hpp"
#include "inventory/inventory_grid.hpp"
#include "status/equipment.hpp"
#include "magic/skill_panel.hpp"
#include "resources/wil_archive.hpp"
#include "resources/messages.hpp"
#include "text/native_text.hpp"
#include "chat/chat_history.hpp"
#include "chat/chat_reception.hpp"
#include "chat_input_view.hpp"
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <map>

class Mir3HudView : public godot::Node2D {
    GDCLASS(Mir3HudView,godot::Node2D)
public:
    void _ready() override;
    void _draw() override;
    void _process(double delta) override;
    void _input(const godot::Ref<godot::InputEvent>& event) override;
    void _unhandled_key_input(const godot::Ref<godot::InputEvent>& event) override;
    void _notification(int what);
    godot::Dictionary hud_snapshot() const;
    void post_chat(godot::String value,int64_t foreground,int64_t background);
    bool add_inventory_item(int icon,int count,int slot,int64_t make_index,godot::String name,int std_mode,int stack_mode);
    bool set_learned_skill(int id,int level,int key);
    bool apply_equipment_reply(int identifier);
    bool apply_inventory_use_reply(int identifier,int remaining);
    bool update_chat_rejection(int channel,bool rejected);
protected:
    static void _bind_methods();
private:
    mir3::HudModel model;
    mir3::InventoryPanel inventory;
    mir3::SkillPanel skills;
    mir3::WilArchive magic_icons;
    std::map<int,godot::Ref<godot::ImageTexture>> skill_frames;
    bool skills_on_top=false;
    void toggle_skills();
    void draw_skills();
    void draw_skill_description();
    mir3::SkillDescriptions skill_descriptions;
    std::map<int,std::vector<mir3::DescriptionRow>> description_rows;
    bool bind_skill_at_cursor(int key);
    bool skill_pointer(int kind,godot::Vector2 position);
    int skill_button_at(godot::Vector2 position);
    mir3::DraggablePanel status{{0,0,328,466}};
    bool status_close_pressed=false;
    bool status_on_top=false;
    godot::Ref<godot::ImageTexture> status_body;
    godot::Vector2 status_body_offset;
    void draw_status();
    void toggle_status();
    bool panel_pointer(int kind,godot::Vector2 position);
    bool status_pointer(int kind,godot::Vector2 position);
    mir3::InventoryGrid bag;
    mir3::EquipmentState equipment;
    mir3::WilArchive equipment_archive;
    void emit_equipment_request();
    std::map<int,std::pair<godot::Ref<godot::ImageTexture>,godot::Vector2>> equipment_frames;
    mir3::WilArchive item_archive;
    std::map<int,godot::Ref<godot::ImageTexture>> item_frames;
    godot::Ref<godot::ImageTexture> item_frame(int id);
    godot::Vector2 inventory_cursor;
    void draw_inventory();
    void toggle_inventory();
    bool inventory_pointer(int kind,godot::Vector2 position);
    mir3::HudController controller{model};
    mir3::WilArchive archive;
    mir3::WilArchive portraits;
    mir3::Messages messages;
    mir3::HudButtonTooltip button_tooltip;
    mir3::ChatHistory chat_history;
    mir3::ChatReceptionButtons reception_buttons;
    std::array<bool,3> rejected_chat{};
    Mir3ChatInput* chat_input=nullptr;
    void chat_requested(godot::String value);
    bool loaded=false;
    std::map<int,godot::Ref<godot::ImageTexture>> frames;
    std::map<int,godot::Ref<godot::ImageTexture>> portrait_frames;
    std::map<std::string,godot::Ref<godot::ImageTexture>> text_cache;
    godot::Ref<godot::ImageTexture> frame(int id);
    void sprite(int id,mir3::HudLayout::Point position);
    void text(const std::string& value,mir3::HudLayout::Rect rect,unsigned color,mir3::TextFont font=mir3::TextFont::normal,bool centered=true);
    void chat_lines();
    bool scroll_input(int kind,godot::Vector2 position);
    void buttons();
    std::string message(int id) const;
    void tooltip(int kind);
    void button_hint();
    void legacy_rect(godot::Rect2 bounds,godot::Color color);
};
