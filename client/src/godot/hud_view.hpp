#pragma once
#include "hud/hud_model.hpp"
#include "hud/minimap.hpp"
#include "hud/mount_panel.hpp"
#include "hud/quest_panel.hpp"
#include "hud/trade_scroll.hpp"
#include "hud/audio_options.hpp"
#include "hud/portrait_picker.hpp"
#include "resources/map_terrain.hpp"
#include "hud/target_portrait.hpp"
#include "inventory/inventory_panel.hpp"
#include "inventory/inventory_grid.hpp"
#include "status/equipment.hpp"
#include "status/element_attributes.hpp"
#include "guild/guild_layout.hpp"
#include "guild/guild_overview.hpp"
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
    bool apply_inventory_add(int identifier,int icon,int count,int64_t make_index,godot::String name,int std_mode,int stack_mode);
    bool add_inventory_item(int icon,int count,int slot,int64_t make_index,godot::String name,int std_mode,int stack_mode);
    void set_character_identity(godot::String name,godot::String secondary,godot::String tertiary,godot::String badge);
    bool set_character_elements(godot::PackedByteArray attack,godot::PackedByteArray strength,godot::PackedByteArray weakness);
    bool set_character_combat(godot::Vector2i attack,godot::Vector2i magic,godot::Vector2i spirit,
        godot::Vector2i defence,godot::Vector2i magic_defence);
    bool set_character_progress(int level,int profession,int appearance,int64_t experience,int64_t next_experience);
    bool set_character_load_bonuses(godot::Vector2i wear,godot::Vector2i hand,godot::Vector2i bonuses);
    bool set_character_vitals(int hp,int max_hp,int mp,int max_mp,int weight,int max_weight);
    bool set_music_map(godot::String map);
    bool fail_quest_detail_request(int index);
    bool set_quest_details(int index,godot::PackedStringArray lines);
    bool set_quest_entries(godot::PackedStringArray titles,godot::PackedInt32Array completed);
    bool apply_mount_reply(int identifier,int64_t recognition);
    bool apply_minimap_reply(int map_number);
    void handle_exit_confirmation(int dialog_id);
    bool apply_trade_offer_reply(int identifier,int accepted_count=0);
    bool set_trade_item(int side,int slot,int icon,int count,int64_t make_index,godot::String name,int std_mode,int stack_mode);
    bool apply_trade_gold_reply(int identifier,int64_t amount,int low_word,int high_word);
    bool set_trade_gold(int64_t local,int64_t remote);
    bool apply_trade_cancel(int64_t balance);
    bool apply_trade_complete(int64_t balance);
    bool open_trade(godot::String partner);
    void set_trade_context(int direction,godot::Vector2i cell,int actor_type,bool dead,godot::String name);
    void set_map_location(godot::String name,int x,int y);
    void set_minimap_position(int x,int y);
    void set_minimap_markers(godot::PackedVector2Array type_five,godot::PackedVector2Array nearby_green,
        godot::PackedVector2Array nearby_orange);
    void set_hovered_target(std::uint32_t actor,int type,int shape,bool excluded,int state,int hp,int max_hp,int percent);
    void set_hovered_human(std::uint32_t actor,int type,int profession,godot::String name);
    bool apply_target_portrait(std::uint32_t actor,godot::PackedByteArray jpeg);
    bool apply_portrait_reply(godot::PackedByteArray jpeg);
    bool load_portrait_cache(godot::String server,godot::String character);
    void set_portrait_names(godot::PackedStringArray names);
    void set_guild_manager(bool allowed);
    bool apply_guild_members_reply(int identifier,godot::PackedByteArray body);
    void set_guild_members(godot::PackedStringArray lines,godot::PackedStringArray editable);
    void post_system_notice(godot::String value,int64_t foreground,int64_t background);
    void set_guild_notices(godot::PackedStringArray lines);
    bool apply_guild_overview_reply(int identifier,godot::PackedByteArray body);
    void set_guild_overview(godot::String name,godot::PackedStringArray lines,godot::PackedStringArray editable);
    void set_group_roster(godot::String player,godot::PackedStringArray members);
    bool apply_skill_records(int identifier,int count,godot::PackedByteArray records);
    bool apply_skill_removal(int id);
    bool apply_skill_progress(int id,int64_t experience,int level);
    bool set_learned_skill(int id,int level,int key,int experience,int required,godot::String name);
    bool apply_equipment_reply(int identifier);
    bool apply_inventory_use_reply(int identifier,int remaining);
    bool update_chat_rejection(int channel,bool rejected);
protected:
    static void _bind_methods();
private:
    mir3::HudModel model;
    int trade_direction=0,trade_actor_type=-1;
    godot::Vector2i trade_actor_cell;
    bool trade_actor_dead=false;
    godot::String trade_actor_name;
    bool request_trade();
    mir3::DraggablePanel trade{{0,0,494,330}};
    godot::String trade_partner;
    mir3::TradeScroll trade_scroll;
    std::uint32_t inventory_gold=0;
    bool trade_gold_pending=false,trade_gold_debited=false;
    std::array<std::uint32_t,2> trade_gold{};
    std::array<std::array<mir3::BagItem,200>,2> trade_items;
    void send_trade_offer(int count);
    int trade_offer_count=0;
    int trade_pending_slot=-1, trade_return_slot=-1;
    bool trade_return_pending=false;
    int trade_return_bag_slot=-1;
    godot::Vector2i trade_return_bag_pointer;
    bool trade_confirm_pressed=false,trade_confirmed=false;
    void draw_trade();
    bool trade_pointer(int kind,godot::Vector2 position);
    mir3::ElementAttributes elements;
    godot::String character_name,character_secondary,character_tertiary,character_badge;
    mir3::Minimap minimap;
    bool skill_shortcuts_visible=false;
    void draw_skill_shortcuts();
    int bound_skill(int key) const;
    bool spell_pointer_available() const;
    mir3::MountPanel mount;
    mir3::QuestPanel quest;
    void toggle_quest();
    void draw_quest();
    bool quest_pointer(int kind,godot::Vector2 position);
    void toggle_mount();
    void draw_mount();
    bool mount_pointer(int kind,godot::Vector2 position);
    mir3::WilArchive minimap_archive, full_minimap_archive;
    godot::Ref<godot::ImageTexture> minimap_image;
    void toggle_minimap(std::uint64_t cooldown);
    void draw_minimap();
    bool minimap_pointer(int kind,godot::Vector2 position,bool ctrl);
    mir3::PortraitPicker portrait_picker;
    mir3::DraggablePanel portrait_panel{mir3::PortraitPicker::initial};
    bool portrait_entry_pressed=false;
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
    mir3::GuildOverview guild_overview;
    bool guild_overview_started=false;
    int guild_first=0;
    bool guild_manager=false;
    godot::String guild_name;
    std::array<godot::PackedStringArray,3> guild_pages;
    std::array<godot::PackedStringArray,2> guild_editable;
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
    bool dialog_has_input() const;
    bool dialog_is_alert() const;
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
    int skill_menu_target=0;
    void bind_skill(int id,int key);
    void draw_skill_key_menu();
    bool skill_menu_input(int kind,godot::Vector2 point,bool double_click);
    bool bind_skill_at_cursor(int key);
    bool skill_pointer(int kind,godot::Vector2 position);
    int skill_button_at(godot::Vector2 position);
    mir3::DraggablePanel status{{0,0,328,466}};
    bool status_close_pressed=false;
    void load_status_body();
    godot::Ref<godot::ImageTexture> status_body;
    godot::Vector2 status_body_offset;
    void draw_status();
    void toggle_status();
    enum class Panel { group, skills, status, inventory, guild, options, portrait, mount, quest, trade };
    std::array<Panel,10> panel_order{Panel::group,Panel::skills,Panel::status,Panel::inventory,Panel::guild,Panel::options,Panel::portrait,Panel::mount,Panel::quest,Panel::trade};
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
    mir3::MapTerrain terrain;
    std::array<mir3::WilArchive,75> terrain_archives;
    bool animated_map_objects=false;
    struct MapActorFrame { godot::Ref<godot::ImageTexture> image; godot::Vector2 offset; };
    mir3::WilArchive map_human, map_npc;
    std::array<MapActorFrame,80> map_human_frames, map_walk_frames, map_run_frames;
    std::array<MapActorFrame,4> map_npc_frames;
    godot::Ref<godot::ImageTexture> terrain_texture(int package,int id);
    std::map<std::pair<int,int>,godot::Ref<godot::ImageTexture>> terrain_frames;
    bool terrain_ready=false;
    godot::Vector2i player_cell{475,360}, step_origin{}, step_destination{};
    int walk_frame=-1, player_facing=4;
    double movement_ms=0;
    int map_pointer_button=0;
    double map_pointer_seconds=0;
    void step_toward_pointer(bool run=false);
    bool merchant_pointer(godot::Vector2 pointer);
    bool apply_npc_message(godot::PackedByteArray encoded,godot::PackedByteArray key);
    bool apply_npc_body(int opcode,int actor,int count,godot::PackedByteArray encoded);
    bool apply_npc_dialogue(int opcode,int actor,godot::String body);
    struct NpcDialogueRow { std::string text, command; mir3::HudLayout::Rect bounds; };
    std::vector<NpcDialogueRow> npc_dialogue_rows;
    int npc_dialogue_actor=0, npc_dialogue_pressed=-1;
    godot::Vector2i npc_dialogue_origin;
    unsigned npc_sequence=0;
    void emit_npc_packet(int identifier,int actor,const godot::String& body);
    void close_shop();
    void close_npc_dialogue();
    bool apply_shop_purchase_reply(int identifier,int64_t value);
    bool apply_shop_goods(int actor,int count,godot::String body);
    struct ShopGood { std::string name; int variants, price, stock; };
    std::vector<ShopGood> shop_goods;
    int shop_actor=0, shop_pressed=-1, shop_first=0, shop_category_first=0;
    bool shop_show_offers=false;
    struct ShopOffer { std::string name; int icon, price; std::uint32_t id; };
    std::vector<ShopOffer> shop_offers;
    int shop_selected=-1;
    bool apply_shop_offers(int actor,int index,godot::PackedStringArray names,godot::PackedInt32Array icons,
        godot::PackedInt32Array prices,godot::PackedInt64Array identifiers);
    void draw_shop();
    bool shop_input(int kind,godot::Vector2 pointer);
    void draw_npc_dialogue();
    bool npc_dialogue_input(int kind,godot::Vector2 pointer);
    std::uint64_t last_merchant_request=0;
    bool merchant_click_captured=false;
    void advance_map_step(double delta);
    void start_map_step(godot::Vector2i destination);
    std::deque<mir3::MapCell> map_route;
    bool walk_to_map_cell(godot::Vector2i destination);
    godot::Vector2i player_pixel() const;
    godot::Vector2i map_camera() const;
    void draw_terrain();
    mir3::WilArchive portraits, monster_portraits;
    mir3::PortraitFocus target_focus;
    mir3::TargetPortraitVitals target_vitals;
    std::uint32_t hovered_target=0;
    int target_frame=-1, target_badge=-1;
    bool target_is_human=false;
    godot::String target_portrait_query_name;
    godot::Ref<godot::ImageTexture> target_custom_portrait;
    godot::Ref<godot::ImageTexture> target_portrait;
    std::map<int,godot::Ref<godot::ImageTexture>> monster_portrait_frames;
    godot::Ref<godot::ImageTexture> portrait_texture(mir3::WilArchive& archive,
        std::map<int,godot::Ref<godot::ImageTexture>>& cache,int id);
    void draw_target_vitals();
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
