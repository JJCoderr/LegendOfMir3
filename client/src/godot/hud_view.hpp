#pragma once
#include "hud/hud_model.hpp"
#include "hud/audio_options.hpp"
#include "hud/portrait_picker.hpp"
#include "inventory/inventory_panel.hpp"
#include "inventory/inventory_grid.hpp"
#include "status/equipment.hpp"
#include "guild/guild_layout.hpp"
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
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/text_edit.hpp>

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
    bool set_music_map(godot::String map);
    bool apply_portrait_reply(godot::PackedByteArray jpeg);
    bool load_portrait_cache(godot::String server,godot::String character);
    void set_portrait_names(godot::PackedStringArray names);
    void set_guild_manager(bool allowed);
    void set_guild_members(godot::PackedStringArray lines);
    void set_guild_notices(godot::PackedStringArray lines);
    void set_guild_overview(godot::String name,godot::PackedStringArray lines);
    void set_group_roster(godot::String player,godot::PackedStringArray members);
    bool set_learned_skill(int id,int level,int key,int experience,int required,godot::String name);
    bool apply_equipment_reply(int identifier);
    bool apply_inventory_use_reply(int identifier,int remaining);
    bool update_chat_rejection(int channel,bool rejected);
protected:
    static void _bind_methods();
private:
    mir3::HudModel model;
    mir3::PortraitPicker portrait_picker;
    bool portrait_visible=false,portrait_entry_pressed=false;
    int portrait_pressed=-1;
    void draw_portrait_picker();
    void enumerate_portraits();
    void load_portrait_preview();
    void request_portrait_change(int button,std::uint32_t tick);
    godot::Ref<godot::FileAccess> open_selected_portrait() const;
    godot::Ref<godot::ImageTexture> active_portrait;
    godot::String portrait_directory,portrait_cache_path;
    bool display_portrait(godot::PackedByteArray jpeg);
    godot::Ref<godot::ImageTexture> portrait_preview;
    int portrait_button_at(godot::Vector2 point);
    bool portrait_input(int kind,godot::Vector2 point);
    void close_portrait_picker();
    mir3::DraggablePanel options{{145,30,510,468}};
    mir3::AudioOptions audio_options;
    bool magic_light=true,damage_numbers=true;
    int audio_drag=-1;
    void toggle_options();
    void load_options();
    void save_options();
    godot::AudioStreamPlayer* hud_sound=nullptr;
    void sync_hud_sound();
    godot::AudioStreamPlayer* map_music=nullptr;
    godot::String audio_root,music_file;
    godot::PackedByteArray music_table;
    godot::String options_path;
    void draw_options();
    bool options_pointer(int kind,godot::Vector2 point);
    mir3::InventoryPanel inventory;
    mir3::DraggablePanel guild{{102,22,594,444}};
    mir3::GuildLayout guild_layout;
    mir3::GuildScroll guild_scroll;
    int guild_first=0;
    bool guild_manager=false;
    godot::String guild_name;
    std::array<godot::PackedStringArray,3> guild_pages;
    void draw_guild();
    void open_guild_editor(bool announcement);
    void draw_guild_editor();
    bool guild_editor_input(const godot::Ref<godot::InputEvent>& event);
    void guild_editor_changed();
    mir3::DraggablePanel guild_editor{{107,110,584,252}};
    godot::TextEdit* guild_edit=nullptr;
    bool guild_edit_announcement=true;
    int guild_edit_pressed=-1;
    std::string guild_edit_ansi;
    bool guild_pointer(int kind,godot::Vector2 point);
    mir3::DraggablePanel group{{272,123,258,244}};
    bool group_allow=false;
    int group_pressed=-1;
    godot::String group_player;
    godot::PackedStringArray group_members;
    void toggle_group();
    void draw_group();
    mir3::HudLayout::Rect group_dialog{};
    int group_request=0,group_selected=0,group_dialog_pressed=-1;
    void open_group_dialog(int action);
    void finish_group_dialog(bool confirm);
    void draw_group_dialog();
    bool group_dialog_input(const godot::Ref<godot::InputEvent>& event);
    bool group_pointer(int kind,godot::Vector2 point);
    int group_button_at(godot::Vector2 point);
    mir3::SkillPanel skills;
    mir3::WilArchive magic_icons;
    std::map<int,godot::Ref<godot::ImageTexture>> skill_frames;
    void toggle_skills();
    void draw_skills();
    void draw_skill_description();
    mir3::SkillDescriptions skill_descriptions;
    std::map<int,std::vector<mir3::DescriptionRow>> description_rows;
    godot::Rect2 skill_key_menu;
    void bind_skill(int id,int key);
    void draw_skill_key_menu();
    bool skill_menu_input(int kind,godot::Vector2 point,bool double_click);
    bool bind_skill_at_cursor(int key);
    bool skill_pointer(int kind,godot::Vector2 position);
    int skill_button_at(godot::Vector2 position);
    mir3::DraggablePanel status{{0,0,328,466}};
    bool status_close_pressed=false;
    godot::Ref<godot::ImageTexture> status_body;
    godot::Vector2 status_body_offset;
    void draw_status();
    void toggle_status();
    enum class Panel { group, skills, status, inventory };
    std::array<Panel,4> panel_order{Panel::group,Panel::skills,Panel::status,Panel::inventory};
    void raise_panel(Panel panel);
    const mir3::DraggablePanel& panel_state(Panel panel) const;
    bool panel_covered(Panel panel,godot::Vector2 point) const;
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
    void text(const std::string& value,mir3::HudLayout::Rect rect,unsigned color,mir3::TextFont font=mir3::TextFont::normal,bool centered=true,bool inverted=false);
    void chat_lines();
    bool scroll_input(int kind,godot::Vector2 position);
    void buttons();
    std::string message(int id) const;
    void tooltip(int kind);
    void button_hint();
    void legacy_rect(godot::Rect2 bounds,godot::Color color);
};
