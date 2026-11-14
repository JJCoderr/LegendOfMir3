#pragma once
#include <godot_cpp/classes/shader_material.hpp>
#include "protocol/actor_ability.hpp"
#include "protocol/actor_increments.hpp"
#include "protocol/actor_experience.hpp"
#include "protocol/gold_change.hpp"
#include "protocol/group_message.hpp"
#include "protocol/guild_notice.hpp"
#include "protocol/skill_wire.hpp"
#include "protocol/inventory_wire.hpp"
#include "protocol/equipment_wire.hpp"
#include "hud/level_up_effect.hpp"
#include "hud/hud_model.hpp"
#include "hud/human_idle.hpp"
#include "hud/human_actions.hpp"
#include "hud/melee_cooldown.hpp"
#include "hud/local_action_clock.hpp"
#include "hud/action_prediction.hpp"
#include "hud/forced_motion.hpp"
#include "hud/scene_human_queue.hpp"
#include "hud/human_life.hpp"
#include "hud/ground_items.hpp"
#include "hud/scene_lighting.hpp"
#include "hud/combat_stance.hpp"
#include "resources/sound_list.hpp"
#include "hud/scene_name.hpp"
#include "hud/scene_health.hpp"
#include "hud/speech_bubble.hpp"
#include "hud/minimap.hpp"
#include "hud/mount_panel.hpp"
#include "hud/quest_panel.hpp"
#include "hud/trade_scroll.hpp"
#include "hud/audio_options.hpp"
#include "hud/system_options.hpp"
#include "hud/exit_guard.hpp"
#include "hud/exit_transition.hpp"
#include "hud/character_selection.hpp"
#include "hud/create_character.hpp"
#include "resources/login_config.hpp"
#include "hud/server_groups.hpp"
#include "hud/portrait_picker.hpp"
#include "resources/map_terrain.hpp"
#include "protocol/map_message.hpp"
#include "protocol/send_message.hpp"
#include "hud/map_input_delay.hpp"
#include "hud/target_portrait.hpp"
#include "inventory/inventory_panel.hpp"
#include "inventory/inventory_grid.hpp"
#include "inventory/quick_items.hpp"
#include "inventory/item_layout.hpp"
#include "chat/chat_submission.hpp"
#include "chat/chat_log.hpp"
#include "inventory/trade_grid.hpp"
#include "status/equipment.hpp"
#include "status/element_attributes.hpp"
#include "guild/guild_layout.hpp"
#include "guild/guild_overview.hpp"
#include "magic/skill_panel.hpp"
#include "hud/snow_weather.hpp"
#include "resources/wil_archive.hpp"
#include "resources/messages.hpp"
#include "text/native_text.hpp"
#include "chat/chat_history.hpp"
#include "chat/chat_reception.hpp"
#include "chat_input_view.hpp"
#include "client_connection.hpp"
#include "resources/legacy_movie.hpp"
#include "hud/game_entry.hpp"
#include "hud/login_audio.hpp"
#include "hud/login_announcement.hpp"
#include "protocol/game_connection.hpp"
#include "protocol/login_actor.hpp"
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/back_buffer_copy.hpp>
#include <godot_cpp/classes/sprite2d.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <map>
#include <tuple>
#include <variant>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/audio_stream_wav.hpp>
#include <godot_cpp/classes/text_edit.hpp>

class Mir3HudView : public godot::Node2D {
    GDCLASS(Mir3HudView,godot::Node2D)
public:
    void _ready() override;
    void set_connection_enabled(bool enabled);
    bool open_server_groups();
    void _exit_tree() override;
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
    void apply_minimap_unavailable();
    void handle_exit_confirmation(int dialog_id);
    bool open_login();
    bool apply_login_message(godot::PackedByteArray message);
    bool set_selection_session(godot::String account,int64_t number);
    bool request_selection_list();
    bool apply_selection_message(godot::PackedByteArray message);
    bool set_local_exit_state(int action,godot::PackedInt64Array ticks);
    bool apply_trade_offer_reply(int identifier,int remaining=0,int64_t new_index=0);
    bool set_trade_item(int side,int slot,int icon,int count,int64_t make_index,godot::String name,int std_mode,int stack_mode);
    bool apply_trade_gold_reply(int identifier,int64_t amount,int low_word,int high_word);
    bool set_trade_gold(int64_t local,int64_t remote);
    bool apply_trade_cancel(int64_t balance);
    bool apply_trade_complete(int64_t balance);
    bool open_trade(godot::String partner);
    void set_trade_context(int direction,godot::Vector2i cell,int actor_type,bool dead,godot::String name);
    void set_map_location(godot::String name,int x,int y);
    bool apply_scene_health(int actor,int opcode,int current,int mana,int maximum);
    bool set_scene_health_flags(int actor,int64_t status_tick,int64_t saved_tick,bool excluded,bool raised);
    void draw_scene_health();
    bool apply_scene_speech(int actor,int opcode,godot::String value);
    void draw_scene_speech();
    bool apply_scene_name(int actor,int opcode,int color,godot::String value);
    void draw_scene_name();
    void draw_ground_name();
    bool set_scene_npc(int actor,godot::Vector2i cell,int first_frame);
    bool set_scene_chicken(int actor,godot::Vector2i cell);
    bool set_scene_human(int actor,godot::Vector2i cell,int facing,int profession,godot::String name);
    bool set_scene_human_body(int actor,int type,int clothing);
    bool set_player_mount(int shape);
    bool set_scene_human_mount(int actor,int shape);
    bool set_player_dyes(int clothing,int hair);
    bool set_scene_human_dyes(int actor,int clothing,int hair);
    bool set_player_attachments(int hair,int weapon);
    bool set_scene_human_attachments(int actor,int hair,int weapon);
    bool set_player_body(int type,int clothing);
    bool apply_scene_human_life(int actor,int opcode,godot::Vector2i cell,int facing);
    bool apply_scene_message(godot::PackedByteArray encoded,godot::PackedByteArray key);
    void apply_actor_health_record(int actor,const mir3::SceneHumanMessage& message);
    bool queue_scene_human_state(int actor,int opcode,godot::Vector2i cell,int facing,const godot::PackedByteArray& body,int hit_weapon=0);
    bool apply_scene_appearance(int actor,const mir3::HumanAppearance& appearance);
    bool queue_scene_human_message(int actor,int opcode,godot::Vector2i cell,int facing,int hit_weapon=0,std::int64_t flags=0);
    bool dispatch_scene_human_message(int actor);
    bool apply_scene_human_action(int actor,int opcode,int facing,int hit_weapon=0);
    bool apply_scene_human_stand(int actor,godot::Vector2i cell,int facing);
    bool apply_scene_human_step(int actor,int opcode,godot::Vector2i destination,int facing);
    bool remove_scene_actor(int actor);
    void set_minimap_position(int x,int y);
    void set_minimap_markers(godot::PackedVector2Array type_five,godot::PackedVector2Array nearby_green,
        godot::PackedVector2Array nearby_orange);
    void set_hovered_target(std::uint32_t actor,int type,int shape,bool excluded,int state,int hp,int max_hp,int percent,
        godot::Vector2i cell={-1,-1});
    bool apply_target_attributes(std::uint32_t actor,int category,int state,godot::PackedByteArray body);
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
    bool apply_trade_wire(const mir3::ReceiveHeader& header,std::span<const std::uint8_t> body);
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
    bool portrait_map_mode=false, portrait_map_pending=false, portrait_map_visible=false, portrait_map_full_archive=false;
    double portrait_map_blink_ms=0;
    void toggle_portrait_map();
    void draw_portrait_map();
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
    mir3::SystemOptions system_options;
    bool apply_options_state(godot::PackedByteArray flags,int attack_mode);
    bool magic_light=true,damage_numbers=true;
    int audio_drag=-1;
    void toggle_options();
    void load_options();
    void save_options();
    godot::AudioStreamPlayer* hud_sound=nullptr;
    godot::AudioStreamPlayer* gold_sound=nullptr;
    unsigned gold_sound_count=0;
    void sync_hud_sound();
    godot::AudioStreamPlayer* map_music=nullptr;
    godot::AudioStreamPlayer* footstep_sound=nullptr;
    std::map<int,godot::Ref<godot::AudioStreamWAV>> movement_sounds;
    mir3::SoundList sound_list;
    int last_movement_sound=0;
    struct ActorSound {
        godot::AudioStreamPlayer* player=nullptr;
        godot::StringName bus;
        int sample=0, count=0, volume=0, pan=0;
    };
    std::map<int,ActorSound> actor_sounds;
    void play_actor_sound(int actor,int sample,godot::Vector2i cell);
    void remove_actor_sound(int actor);
    godot::StringName footstep_bus;
    godot::String audio_root,music_file;
    godot::PackedByteArray music_table;
    godot::String options_path;
    mir3::ChatSubmission chat_submission;
    godot::String chat_storage_key,chat_log_path;
    void append_chat_log(int opcode,std::string_view ansi);
    godot::String chat_history_file(bool phrases) const;
    void save_chat_shortcuts(bool phrases);
    void load_chat_shortcuts();
    bool chat_shortcut_input(const godot::Ref<godot::InputEvent>& event);
    void draw_chat_shortcuts();
    void draw_options();
    bool options_pointer(int kind,godot::Vector2 point);
    mir3::InventoryPanel inventory;
    struct MerchantService {
        enum class Phase { choosing, quoting, quoted, paying };
        std::uint32_t actor=0,item=0,price=0;
        std::string name;
        Phase phase=Phase::choosing;
        bool pressed=false,selling=false,stack=false,storage=false;
    } merchant_service;
    void confirm_merchant_service(int quantity=0);
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
    enum class Panel { group, skills, status, inventory, guild, options, portrait, mount, quest, trade, quick };
    std::array<Panel,11> panel_order{Panel::group,Panel::skills,Panel::status,Panel::inventory,Panel::guild,Panel::options,Panel::portrait,Panel::mount,Panel::quest,Panel::trade,Panel::quick};
    void raise_panel(Panel panel);
    const mir3::DraggablePanel& panel_state(Panel panel) const;
    bool panel_covered(Panel panel,godot::Vector2 point) const;
    void cancel_pointer_interactions();
    bool panel_pointer(int kind,godot::Vector2 position,bool captured_only=false);
    bool status_pointer(int kind,godot::Vector2 position);
    mir3::InventoryGrid bag;
    mir3::QuickItems quick_items;
    godot::String item_layout_directory;
    godot::String item_layout_file() const;
    void save_item_layout();
    void restore_item_layout(mir3::InventoryGrid& items,mir3::QuickItems& references);

    mir3::DraggablePanel quick_panel{{73,413,280,48}};
    bool quick_close_pressed=false;
    void toggle_quick();
    void draw_quick();
    int quick_slot_at(godot::Vector2 position) const;
    bool quick_pointer(int kind,godot::Vector2 position);
    void activate_quick(int slot,bool mouse=false);
    void quick_notice(int resource,int slot,const std::string& name);

    mir3::EquipmentState equipment;
    mir3::WilArchive equipment_archive;
    void emit_equipment_request();
    void play_item_sound(const mir3::BagItem& item);
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
    bool change_map(godot::String map_id,godot::String name,godot::Vector2i spawn,int ambient_mode=0);
    bool set_local_actor_identity(int actor);
    int local_actor_identity=0;
    unsigned player_light_attribute=0;
    mir3::SceneAmbient scene_ambient;
    godot::PackedColorArray scene_light_samples;
    godot::Node2D *scene_light_layer=nullptr, *hud_base=nullptr;
    void draw_scene_lighting();
    void draw_hud_base();
    void reset_map_overlays();
    godot::String map_directory, current_map_id="0";
    std::deque<mir3::MapChangeMessage> map_changes;
    int map_series=0;
    mir3::SnowWeather weather;
    godot::Node2D* snow_layer=nullptr;
    godot::BackBufferCopy* fog_copy=nullptr;
    godot::Sprite2D* fog_sprite=nullptr;
    godot::Ref<godot::ImageTexture> snow_texture;
    godot::BackBufferCopy* rain_copy=nullptr;
    godot::Node2D* rain_layer=nullptr;
    std::array<godot::Ref<godot::ImageTexture>,15> rain_textures;
    void draw_rain();
    void draw_snow();
    void update_weather_layers();
    mir3::MapInputDelay map_input_delay;
    mir3::MapTerrain terrain;
    std::array<mir3::WilArchive,75> terrain_archives;
    bool animated_map_objects=false;
    struct MapActorFrame { godot::Ref<godot::ImageTexture> image; godot::Vector2 offset; std::vector<std::uint8_t> regions;
        int shadow_mode=48; godot::Vector2 shadow_offset{}; };
    mir3::WilArchive map_human, map_female, map_npc, map_horse;
    mir3::GroundItems ground_items;
    mir3::WilArchive ground_archive;
    std::map<int,MapActorFrame> ground_frames;
    std::map<int,godot::Rect2> ground_bounds;
    int displayed_ground_item=0;
    std::map<int,MapActorFrame> horse_frames;
    const MapActorFrame* horse_frame(int shape,int motion_frame);
    struct HumanBody {
        std::array<MapActorFrame,80> standing, walking, running;
    };
    std::map<std::pair<int,int>,HumanBody> human_bodies;
    std::map<std::pair<int,int>,HumanBody> riding_bodies;
    const HumanBody* human_body(int type,int clothing,bool riding=false);
    std::map<std::tuple<int,int,int>,MapActorFrame> human_action_frames;
    const MapActorFrame* human_action_frame(int type,int clothing,int frame);
    int player_mount=0;
    int player_type=0, player_clothing=0, player_hair=0, player_weapon=0;
    int player_clothing_dye=65535, player_hair_dye=65535;
    std::map<std::tuple<std::uint64_t,int,int>,godot::Ref<godot::ImageTexture>> dyed_frames;
    godot::Ref<godot::ImageTexture> dyed_frame(const MapActorFrame& frame,int clothing,int hair,int status_color=65535);
    godot::String actor_resource_root;
    godot::PackedByteArray weapon_order;
    std::map<std::string,mir3::WilArchive> attachment_archives;
    std::map<std::pair<std::string,int>,MapActorFrame> attachment_frames;
    const MapActorFrame* attachment_frame(int type,int shape,bool weapon,int motion_frame);
    void draw_human_layers(const MapActorFrame& body,godot::Vector2 anchor,int type,int hair,int weapon,int motion_frame,int clothing_dye,int hair_dye,int mount_shape=0,float opacity=1.f,int status_color=65535,bool corpse=false);
    const MapActorFrame* project_weapon_shadow(const MapActorFrame& frame);
    const MapActorFrame* horse_shadow_frame(int motion_frame);
    std::map<std::uint64_t,MapActorFrame> weapon_shadows;
    std::vector<godot::Rect2> human_shadow_bounds;
    void draw_weapon_glow(godot::Vector2 anchor,int type,int motion_frame);
    std::map<int,MapActorFrame> weapon_glow_frames;
    std::vector<godot::Vector2> weapon_glow_lights;
    std::vector<int> drawn_weapon_glow_frames;
    std::array<MapActorFrame,80> map_human_frames, map_walk_frames, map_run_frames;
    std::map<int,mir3::SceneHealth> scene_health;
    std::map<int,godot::Rect2> scene_health_bounds;
    std::map<int,std::vector<mir3::DamageGlyph>> scene_damage_glyphs;
    godot::Ref<godot::ImageTexture> health_border, health_fill;
    std::map<int,mir3::SpeechBubble> scene_speech;
    std::map<int,godot::Rect2> speech_bounds;
    std::map<int,mir3::SceneName> scene_names;
    int displayed_name_actor=-1;
    godot::Rect2 displayed_name_bounds, player_bounds;
    struct SceneActor {
        godot::Vector2 anchor;
        godot::Vector2i cell;
        std::array<MapActorFrame,4> frames;
        godot::Rect2 bounds;
    };
    std::map<int,SceneActor> scene_npcs;
    struct SceneChicken : SceneActor {
        godot::Rect2 target_bounds;
        int shape=20;
        int burrow_frame=-1;
        bool hide_after_burrow=false, reverse_burrow=false;
        bool turned_burrow_once=false;
        bool hidden_pose=false;
        double burrow_ms=0;
        bool buried=false;
        int idle_frame=0;
        double idle_ms=0;
        bool called=false;
        unsigned light_attribute=0;
        bool can_be_butchered() const { return shape==20 || shape==21 || shape==119; }
        int facing=6, death_frame=-1;
        int walk_frame=-1;
        bool walk_catch_up=false, walking_backward=false;
        bool skip_movement_tick=false;
        std::uint32_t status_flags=0;
        bool slowed() const { return (status_flags & 0x08000000u)!=0; }
        int struck_frame=-1;
        int attack_frame=-1, attack_interval=120;
        int attack_action=2;
        std::uint64_t action_overlay=0;
        double attack_ms=0;
        bool attack_catch_up=false, struck_catch_up=false, burrow_catch_up=false, death_catch_up=false;
        godot::Vector2i walk_origin;
        int walk_pose() const { return walk_frame>=0 && walking_backward?5-walk_frame:walk_frame; }
        godot::Vector2i walk_pixel() const {
            const auto direction=cell-walk_origin;
            const int step=walk_frame+1;
            // 4075B0 reanchors the backstep to the destination before the final frame.
            if (walking_backward && walk_frame>=(facing==7?4:1)) {
                const int remaining=6-step;
                return cell*godot::Vector2i(48,32)-direction*godot::Vector2i(8*remaining,5*remaining);
            }
            const int y=direction.x==0?32*step/6:5*step;
            return walk_origin*godot::Vector2i(48,32)+direction*godot::Vector2i(8*step,y);
        }
        bool skeleton=false;
        double death_ms=0, struck_ms=0;
        struct Turn { godot::Vector2i cell; int facing; int shape; unsigned light; bool backward; std::uint32_t status; bool force_walk=false; };
        struct Hit { int shape; int variant; std::array<std::uint16_t,3> health; };
        struct Death { godot::Vector2i cell; int facing; int shape; int opcode; unsigned light; };
        struct Burrow { godot::Vector2i cell; int facing, shape; unsigned light; };
        struct ReverseBurrow { godot::Vector2i cell; };
        struct Rise { godot::Vector2i cell; int facing, shape; unsigned light; };
        struct Attack {
            godot::Vector2i cell;
            int facing;
            std::optional<godot::Vector2i> target;
            std::uint32_t target_actor=0;
            int action_override=0;
        };
        struct Entry { godot::Vector2i cell; int facing,shape; unsigned light; std::uint32_t status; bool appearance_sound; std::optional<mir3::SceneName> name; bool hidden_pose=false; };
        struct BarStatus { std::uint32_t flags; };
        struct Health { std::array<std::uint16_t,3> values; };
        struct RelayedBackstep { Turn turn; };
        using Action=std::variant<Turn,Hit,Death,Burrow,Entry,BarStatus,Health,RelayedBackstep,Attack,Rise,ReverseBurrow>;
        std::deque<Action> actions;
    };
    std::map<int,SceneChicken> scene_chickens;
    std::map<std::pair<int,int>,std::optional<MapActorFrame>> monster_frames;
    std::map<int,std::optional<mir3::WilArchive>> monster_archives;
    std::map<std::pair<std::uint64_t,bool>,MapActorFrame> projected_monster_shadows;
    const MapActorFrame* project_monster_shadow(const MapActorFrame& body,bool flat);
    const MapActorFrame* monster_frame(int shape,int action_base,int facing,int frame_offset,bool shadow=false);
    bool place_scene_chicken(int actor,godot::Vector2i cell,int facing,int shape=20,unsigned light=0);
    bool move_scene_chicken(int actor,SceneChicken::Turn turn);
    bool apply_monster_hit(int actor,const SceneChicken::Hit& hit);
    bool apply_monster_death(int actor,const SceneChicken::Death& death);
    bool apply_monster_burrow(int actor,SceneChicken::Burrow burrow);
    bool apply_monster_entry(int actor,const SceneChicken::Entry& entry);
    bool apply_monster_action(int actor,const SceneChicken::Action& action);
    bool receive_monster_action(int actor,SceneChicken::Action action);
    void update_actor_bar_status(int actor,bool status_bit);
    bool refresh_monster_shape(SceneChicken& monster,int shape,int action=-1,int facing=-1);
    void play_animal_call(int actor,SceneChicken& animal);
    struct SceneHuman : SceneActor {
        int profession=0, type=0, clothing=0, hair=0, weapon=0;
        int clothing_dye=65535, hair_dye=65535, mount_shape=0;
        godot::String name;
        int facing=0, movement_frame=-1, step_distance=0;
        mir3::HumanIdleClock idle;
        mir3::HumanActionClock action;
        int hit_weapon=0;
        std::int16_t attack_speed=0;
        mir3::SceneHumanQueue messages;
        mir3::HumanLife life;
        mir3::CombatStance combat;
        unsigned light_attribute=0;
        bool bump=false;
        bool catching_up=false;
        std::uint32_t action_flags=0;
        bool harvest=false;
        unsigned slow_ticks=0;
        double movement_ms=0;
        godot::Vector2i step_origin, step_destination;
    };
    std::map<int,SceneHuman> scene_humans;
    double scene_movement_ms=0;
    std::vector<int> scene_removals;
    bool scene_clear_pending=false;
    void apply_scene_removals();
    void update_scene_target(godot::Vector2 pointer);
    bool scene_occupied(godot::Vector2i cell) const;
    godot::Ref<godot::ImageTexture> terrain_texture(int package,int id);
    std::map<std::pair<int,int>,godot::Ref<godot::ImageTexture>> terrain_frames;
    bool terrain_ready=false;
    godot::Vector2i player_cell{475,360}, step_origin{}, step_destination{};
    int walk_frame=-1, player_facing=4;
    mir3::HumanIdleClock player_idle;
    mir3::HumanActionClock player_action;
    mir3::MeleeCooldown melee_cooldown;
    mir3::LocalActionClock local_action_clock;
    mir3::ActionPrediction action_prediction;
    mir3::HumanLife player_life;
    struct LocalActorMessage {
        mir3::ForcedMotion motion;
        mir3::HumanAppearance appearance;
        unsigned light;
        std::optional<mir3::SceneHumanMessage> hit;
        unsigned hit_variant=0;
        std::optional<mir3::SceneHumanMessage> status;
    };
    std::deque<LocalActorMessage> local_actor_messages;
    std::optional<mir3::ForcedMotion> forced_motion;
    std::uint32_t local_struck_tick=0;
    std::uint8_t local_folded_hits=0;
    int local_hit_weapon=0;
    std::uint32_t player_status_flags=0;
    void process_local_actor_message();
    struct PendingLocalAction {
        bool movement;
        godot::Vector2i origin,destination;
        int facing,weapon,opcode;
        std::optional<std::uint32_t> waiting_since;
        std::uint32_t wait_ms=0;
    };
    std::optional<PendingLocalAction> pending_local_action;
    void request_melee_action(int facing,int opcode);
    void advance_local_action();
    bool player_harvest_impact=false;
    bool map_shift_pressed=false;
    mir3::EffectCatalog effect_catalog;
    mir3::WilArchive event_archive;
    std::map<int,MapActorFrame> ground_event_frames;
    struct GroundEvent { godot::Vector2i cell; int frame; double age=0; bool hidden=false; double lifetime=65535; float opacity=1.f; };
    std::map<std::uint64_t,GroundEvent> ground_events;
    std::map<int,MapActorFrame> event_frames;
    std::map<int,mir3::WilArchive> monster_effect_archives;
    struct MonsterProjectile {
        godot::Vector2 position, destination;
        std::uint32_t target_actor;
        int effect;
        godot::Vector2 velocity{};
        godot::Vector2 previous_distance{10000,10000};
    };
    enum class MonsterEffectPose { attack, struck, death, idle, walk, burrow, skeleton };
    struct MapEvent {
        const mir3::EffectResource* resource;
        godot::Vector2i cell;
        godot::BackBufferCopy* copy;
        godot::Sprite2D* sprite;
        int frame;
        double elapsed=0;
        godot::Sprite2D* secondary=nullptr;
        bool ending=false,one_shot=false;
        int remaining_ms=-1, tint=255;
        int frame_offset=0, delay_ms=0, attached_actor=0;
        std::optional<MonsterProjectile> projectile;
        MonsterEffectPose attached_pose=MonsterEffectPose::attack;
        bool emits_light=false;
        double light_clock=0;
        int light_phase=0, light_shrink=0, light_fade=0;
        bool light_tail=false;
        bool ground_wave=false;
        std::uint32_t follow_actor=0;
        std::optional<godot::Vector2> follow_position;
    };
    std::map<std::uint64_t,MapEvent> map_events;
    std::uint64_t harvest_effect_id=std::uint64_t{1}<<32;
    void show_harvest_impact(godot::Vector2i cell);
    void show_map_impact(int effect,godot::Vector2i cell);
    void show_scene_effect(mir3::EffectCatalog::Section section,int effect,godot::Vector2i cell,
        int frame_offset=0,int delay_ms=0,int attached_actor=0,
        std::optional<MonsterProjectile> projectile=std::nullopt,
        MonsterEffectPose attached_pose=MonsterEffectPose::attack);
    void play_monster_attack_effect(int actor,const SceneChicken& monster);
    void sync_monster_action_overlay(int actor,SceneChicken& monster,bool restart=false);
    void play_monster_death_effect(int actor,const SceneChicken& monster);
    bool play_monster_ranged_attack(const SceneChicken& monster,const SceneChicken::Attack& attack);
    struct EventParticle {
        godot::Vector2 position,velocity,acceleration;
        int age=0,life=0,interval=0,frame_clock=0,frame=0;
        float size=0,mass=1;
        godot::Color tint;
        bool shrinking=false;
    };
    std::vector<EventParticle> event_particles;
    std::map<std::pair<int,int>,GroundEvent> event_marks;
    void emit_event_particles(godot::Vector2i cell);
    void advance_map_events(double milliseconds);
    void clear_map_events();
    mir3::WilArchive level_archive;
    std::map<int,MapActorFrame> level_frames;
    godot::Node2D* level_layer=nullptr;
    struct LevelVisual {
        mir3::LevelUpEffect playback;
        godot::BackBufferCopy* copy=nullptr;
        godot::Sprite2D* sprite=nullptr;
    };
    std::vector<LevelVisual> level_effects;
    bool receive_level_up(const mir3::ReceiveHeader& header);
    void advance_level_effects(double milliseconds);
    void clear_level_effects();
    const MapActorFrame* level_frame(int index);

    std::optional<mir3::ActorAbility> server_ability;
    mir3::ActorSubAbility server_sub_ability;
    std::uint16_t weapon_training=0;
    double movement_ms=0;
    int map_pointer_button=0;
    double map_pointer_seconds=0;
    void step_toward_pointer(bool run=false,bool force_attack=false,bool cut=false);
    bool merchant_pointer(godot::Vector2 pointer);
    bool apply_npc_message(godot::PackedByteArray encoded,godot::PackedByteArray key);
    bool apply_npc_body(int opcode,int actor,int count,godot::PackedByteArray encoded);
    bool apply_npc_dialogue(int opcode,int actor,godot::String body);
    struct NpcDialogueRow { std::string text, command; mir3::HudLayout::Rect bounds; unsigned color; int option; };
    std::vector<NpcDialogueRow> npc_dialogue_rows;
    int npc_dialogue_actor=0, npc_dialogue_pressed=-1;
    std::uint64_t npc_dialogue_last_action=0;
    std::string npc_dialogue_prompt, npc_dialogue_prompt_command;
    int npc_dialogue_prompt_actor=0;
    int npc_dialogue_extra_rows=0, npc_dialogue_first=0, npc_dialogue_scroll_limit=0;
    std::array<godot::Ref<godot::ImageTexture>,13> npc_dialogue_backgrounds;
    int npc_dialogue_arrow_pressed=0;
    int npc_dialogue_arrow_frame=0;
    bool npc_dialogue_down_lit=false;
    bool npc_dialogue_scroll_enabled=false;
    bool npc_dialogue_can_close=true, npc_dialogue_close_pressed=false;
    mir3::HudLayout::Rect npc_dialogue_bounds() const { return {0,0,386,204+20*std::max(1,npc_dialogue_extra_rows)}; }
    mir3::HudLayout::Rect npc_dialogue_text_bounds(const NpcDialogueRow& row) const;
    int npc_dialogue_option_at(godot::Vector2 pointer) const;
    godot::Vector2i npc_dialogue_origin;
    mir3::LoginConfig login_config;
    mir3::LoginFlow login_flow;
    bool login_visible=false,login_notice_pressed=false;
    double login_fraction_ms=0;
    std::array<unsigned char,4> login_buttons{};
    godot::Vector2 login_pointer;
    std::vector<godot::Rect2> login_server_bounds;
    godot::Rect2 login_server_box;
    enum class LoginMoviePurpose { none,opening,create,return_selection,enter_game };
    LoginMoviePurpose login_movie_purpose=LoginMoviePurpose::none;
    mir3::LegacyMovie login_movie;
    godot::Ref<godot::ImageTexture> login_movie_surface;
    godot::AudioStreamPlayer* login_movie_audio=nullptr;
    void start_login_movie(LoginMoviePurpose purpose,const char* filename,bool reverse=false);
    void advance_login_movie(double delta);
    void finish_game_entry();
    mir3::LoginAnnouncement login_announcement;
    struct AnnouncementImage { godot::Ref<godot::ImageTexture> image;int width=0,height=0; };
    std::vector<AnnouncementImage> announcement_images;
    void load_login_announcement();
    void draw_login_announcement();
    mir3::LoginAudio login_audio;
    std::map<std::string,godot::AudioStreamPlayer*> login_effects;
    void play_login_sound(std::string_view file,bool loop=false);
    void advance_login_audio(std::uint32_t ms);
    mir3::GameEntry game_entry;
    mir3::GameConnection game_connection;
    std::optional<mir3::LoginActor> login_actor_pending;
    std::array<std::uint8_t,16> login_actor_extra{};
    bool apply_login_actor();
    bool apply_game_message(const godot::PackedByteArray& message);
    void process_map_changes();
    mir3::ServerGroups server_groups;
    mir3::WilArchive server_group_archive;
    std::map<int,godot::Ref<godot::ImageTexture>> server_group_frames;
    godot::Vector2 server_group_pointer;
    godot::Vector2i server_glow_offset;
    godot::Node2D* server_glow=nullptr;
    bool server_group_exit_pressed=false,server_group_scroll_drag=false;
    std::uint64_t server_group_scroll_tick=0;
    bool open_configured_login(mir3::LoginConfig configured);
    void draw_server_groups();
    void draw_server_group_glow();
    void server_groups_input(const godot::Ref<godot::InputEvent>& event);
    void clear_game_scene();
    void sync_login_editor();
    void draw_login();
    void login_input(const godot::Ref<godot::InputEvent>& event);
    void send_login_request(const mir3::LoginRequest& request);
    mir3::ExitTransition exit_transition;
    mir3::CharacterSelectionPage character_selection;
    double exit_fraction_ms=0;
    mir3::WilArchive selection_archive;
    std::map<int,mir3::WilFrame> selection_sources;
    godot::Ref<godot::ImageTexture> selection_surface;
    std::optional<mir3::CharacterSelectionPage> selection_pending;
    std::array<godot::Rect2,4> selection_bounds;
    std::uint32_t selection_elapsed=0;
    double selection_fraction_ms=0;
    mir3::SelectionNotice selection_notice;
    mir3::CreateCharacterPage character_creation;
    std::array<godot::Rect2,2> creation_bounds;
    std::vector<std::string> creation_description;
    std::vector<std::uint8_t> creation_backdrop;
    void open_character_creation();
    void show_character_creation();
    void close_character_creation();
    void submit_character_creation();
    void update_creation_description();
    void draw_character_creation();
    void creation_input(const godot::Ref<godot::InputEvent>& event);
    void close_selection_notice();
    mir3::DeleteCharacterDialog selection_delete;
    void open_selection_delete();
    void finish_selection_delete(bool confirm);
    mir3::SelectionSession selection_session;
    int selection_pending_notice=0;
    godot::Vector2 selection_pointer;
    int selection_pressed=-1;
    void finish_login_exit(const godot::String& url);
    const mir3::WilFrame* selection_source(int index);
    void selection_input(const godot::Ref<godot::InputEvent>& event);

    std::map<int,godot::Ref<godot::ImageTexture>> selection_frames;
    godot::Node2D* exit_layer=nullptr;
    bool prepare_character_selection();
    void enter_character_selection();
    void draw_character_selection();
    void draw_exit_transition();
    mir3::ExitGuard exit_guard;
    bool check_exit_allowed();
    enum class ConnectionPage { none,login,selection,game };
    mir3::ClientConnection connection;
    mir3::LoginEndpoint connection_endpoint;
    ConnectionPage connection_page=ConnectionPage::none;
    bool connection_enabled=true,connection_failed=false;
    std::uint64_t connection_retry_at=0;
    void connect_page(ConnectionPage page,const mir3::LoginEndpoint& endpoint);
    void poll_connection(double delta);
    mir3::SendMessages outgoing_messages;
    godot::PackedByteArray send_client_packet(int identifier,int actor,const godot::String& body,std::uint32_t instance=0,std::uint16_t quantity=0);
    void emit_npc_packet(int identifier,int actor,const godot::String& body,std::uint32_t instance=0,std::uint16_t quantity=0);
    void close_shop();
    void close_npc_dialogue();
    bool apply_shop_purchase_reply(int identifier,int64_t value);
    bool apply_shop_goods(int actor,int count,godot::String body,bool wire_format=false);
    struct ShopGood { std::string name; int variants, price, stock, icon=-1, requirement_type=0, requirement=0; };
    std::vector<ShopGood> shop_goods;
    int shop_actor=0, shop_pressed=-1, shop_category_first=0;
    int shop_offer_index=-1;
    int shop_offer_page=0;
    bool shop_offer_loading=false;
    bool shop_show_offers=false;
    struct ShopOffer { std::string name; int icon, price; std::uint32_t id; int count=0,stack_mode=0; };
    std::vector<ShopOffer> shop_offers;
    bool storage_list=false;
    std::uint32_t storage_pending=0;
    void withdraw_storage(int quantity=0);
    int shop_selected=-1;
    int shop_bulk_index=-1;
    int shop_highlighted=-1;
    bool shop_scroll_dragging=false;
    double shop_scroll_fraction=0;
    bool shop_scroll_input(int kind,godot::Vector2 pointer);
    bool apply_shop_offers(int actor,int index,godot::PackedStringArray names,godot::PackedInt32Array icons,
        godot::PackedInt32Array prices,godot::PackedInt64Array identifiers,int offset=-1);
    void draw_shop();
    void draw_shop_requirement(const ShopGood& good,int x,int y,int width);
    void close_shop_offers();
    static constexpr int shop_top=225;
    int shop_window_top() const { return storage_list?234:shop_top; }
    int shop_window_left() const { return storage_list?-292:0; }
    int shop_window_height() const { return storage_list?205:224; }
    int shop_width() const { return shop_bulk_index>=0?516:shop_show_offers?488:292; }
    mir3::HudLayout::Rect shop_offer_cell(int cell) const {
        if(storage_list) return {22+38*(cell%4),266+38*(cell/4),37,37};
        return {315+38*(cell%4),shop_top+42+38*(cell/4),35,35};
    }
    static constexpr int shop_goods_rows=4, shop_goods_pitch=42, shop_goods_row_top=16;
    bool shop_input(int kind,godot::Vector2 pointer,bool double_click);
    void buy_shop_offer(int index);
    void buy_shop_bulk(std::uint16_t quantity);
    void open_shop_category();
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
    void draw_terrain_texture(const godot::Ref<godot::ImageTexture>& image,godot::Vector2 position,
        godot::Color modulate=godot::Color(1,1,1),int rgb565_weight=0,bool weapon_glow=false);
    std::vector<godot::RID> terrain_draw_items;
    godot::Node2D* terrain_layer=nullptr;
    std::size_t terrain_draw_count=0;
    godot::RID terrain_draw_batch;
    godot::Ref<godot::ShaderMaterial> monster_shadow_mix,monster_body_mix;
    godot::Ref<godot::ShaderMaterial> weapon_glow_mix;
    void draw_hud_tail();
    void set_hud_decoration(godot::Vector4i state);
    godot::Node2D* paint_canvas=this;
    godot::Node2D* hud_tail=nullptr;
    godot::BackBufferCopy* hud_copy=nullptr;
    godot::Sprite2D* hud_decoration=nullptr;
    int hud_decoration_frame=-1;
    mir3::WilArchive portraits, monster_portraits;
    mir3::PortraitFocus target_focus;
    mir3::TargetPortraitVitals target_vitals;
    std::uint32_t hovered_target=0;
    int target_frame=-1, target_badge=-1;
    bool target_is_human=false;
    bool target_special_shape=false;
    std::map<std::uint32_t,mir3::TargetAttributes> target_attributes_by_actor;
    godot::String target_portrait_query_name;
    godot::Ref<godot::ImageTexture> target_custom_portrait;
    godot::Ref<godot::ImageTexture> target_portrait;
    std::map<int,godot::Ref<godot::ImageTexture>> monster_portrait_frames;
    godot::Ref<godot::ImageTexture> portrait_texture(mir3::WilArchive& archive,
        std::map<int,godot::Ref<godot::ImageTexture>>& cache,int id);
    void draw_target_vitals();
    mir3::Messages messages;
    mir3::HudButtonTooltip button_tooltip;
    bool button_hint_animating=false;
    mir3::ChatHistory chat_history;
    mir3::ChatReceptionButtons reception_buttons;
    std::array<bool,3> rejected_chat{};
    Mir3ChatInput* chat_input=nullptr;
    void chat_requested(godot::String value);
    void send_chat_command(godot::String value);
    bool apply_inventory_wire(const mir3::ReceiveHeader& header,std::span<const std::uint8_t> body);
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
