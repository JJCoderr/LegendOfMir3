# Test harness only. Client behavior lives in C++.
extends SceneTree

var hud: Node
var actions: Array[int] = []
var requests: Array[String] = []

func _initialize() -> void:
	call_deferred("run")

func check(condition: bool, label: String) -> bool:
	if not condition:
		push_error("HUD smoke failed: " + label)
		quit(1)
	return condition

func click_at(position: Vector2, pressed: bool) -> void:
	var event := InputEventMouseButton.new()
	event.button_index = MOUSE_BUTTON_LEFT
	event.position = position
	event.global_position = position
	event.pressed = pressed
	Input.parse_input_event(event)

func key(code: int, unicode_value: int = 0) -> void:
	var event := InputEventKey.new()
	event.keycode = code
	event.physical_keycode = code
	event.unicode = unicode_value
	event.pressed = true
	Input.parse_input_event(event)
	event = event.duplicate()
	event.pressed = false
	Input.parse_input_event(event)

func capture(name: String) -> bool:
	if not OS.get_cmdline_user_args().has("--screenshots"):
		return true
	await process_frame
	await RenderingServer.frame_post_draw
	var image := root.get_texture().get_image()
	if not check(image != null and image.get_size() == Vector2i(800, 600), "viewport capture size"):
		return false
	var directory := ProjectSettings.globalize_path("res://../build/screenshots")
	DirAccess.make_dir_recursive_absolute(directory)
	if not check(image.save_png(directory.path_join(name + ".png")) == OK, "save screenshot"):
		return false
	print("SCREENSHOT: ", directory.path_join(name + ".png"))
	return true

func run() -> void:
	hud = load("res://main.tscn").instantiate()
	root.size = Vector2i(800, 600)
	Input.use_accumulated_input = false
	var options_file := "user://smoke-options-%d.ini" % OS.get_process_id()
	hud.set_meta("options_path", options_file)
	var portrait_dir := "user://smoke-portraits-%d" % OS.get_process_id()
	DirAccess.make_dir_recursive_absolute(portrait_dir)
	var portrait_image := Image.create_empty(82, 94, false, Image.FORMAT_RGB8)
	portrait_image.fill(Color("304c64"))
	portrait_image.fill_rect(Rect2i(16, 20, 50, 54), Color("cc9b60"))
	if not check(portrait_image.save_jpg(portrait_dir.path_join("sample.JPG")) == OK, "write portrait fixture"): return
	hud.set_meta("portrait_directory", portrait_dir)
	root.add_child(hud)
	hud.hud_action.connect(func(button: int): actions.append(button))
	hud.chat_requested.connect(func(value: String): requests.append(value))
	await process_frame
	if not check(hud.hud_snapshot().ready, "original resource loading"): return
	for i in 20:
		hud.post_chat("[测试 %02d] 比奇省：聊天文字与滚动验证。" % i, 0xffffff if i % 2 == 0 else 0x96ffff, 0)
	await process_frame
	if not check(hud.hud_snapshot().chat_compact_count == 20 and hud.hud_snapshot().chat_compact_first == 15, "chat compact following"): return
	if not await capture("hud-chat-compact"): return
	click_at(Vector2(0, 0), true)
	click_at(Vector2(620, 473), false)
	await process_frame
	if not check(hud.hud_snapshot().expanded and actions == [14], "release-inside expansion"): return
	if not check(hud.hud_snapshot().chat_expanded_first == 6, "independent expanded following"): return
	if not await capture("hud-chat-expanded"): return
	var wheel := InputEventMouseButton.new()
	wheel.position = Vector2(300, 350)
	wheel.button_index = MOUSE_BUTTON_WHEEL_UP
	wheel.pressed = true
	Input.parse_input_event(wheel)
	await process_frame
	if not check(hud.hud_snapshot().chat_expanded_first == 5 and hud.hud_snapshot().chat_compact_first == 15, "independent wheel scroll"): return
	key(KEY_N)
	Input.parse_input_event(wheel)
	if not check(hud.hud_snapshot().options_visible and hud.hud_snapshot().chat_expanded_first == 5, "options blocks wheel to underlying chat"): return
	if not await capture("options-over-chat"): return
	key(KEY_N)
	click_at(Vector2(620, 306), true)
	click_at(Vector2(620, 306), false)
	await process_frame
	if not check(not hud.hud_snapshot().expanded and actions == [14, 14], "collapse"): return
	click_at(Vector2(634, 570), true)
	click_at(Vector2(634, 570), false)
	await process_frame
	if not check(not hud.hud_snapshot().primary_defence, "stat switching"): return
	click_at(Vector2(685, 510), true)
	click_at(Vector2(685, 510), false)
	await process_frame
	if not check(actions == [14, 14, 12] and hud.hud_snapshot().pressed == -1, "inventory action dispatch"): return
	if not check(hud.hud_snapshot().inventory_visible and hud.hud_snapshot().inventory_position == Vector2i(518, 0), "inventory origin"): return
	if not await capture("inventory-empty"): return
	if not check(hud.add_inventory_item(5, 3, 0) and hud.add_inventory_item(1043, 1, 14) and hud.add_inventory_item(15, 1, 54), "inventory fixtures"): return
	if not await capture("inventory-items"): return
	click_at(Vector2(550, 75), false)
	if not check(hud.hud_snapshot().held_inventory_icon == 5, "inventory pickup"): return
	var preview_motion := InputEventMouseMotion.new()
	preview_motion.position = Vector2(590, 75)
	Input.parse_input_event(preview_motion)
	if not await capture("inventory-placement"): return
	preview_motion = InputEventMouseMotion.new()
	preview_motion.position = Vector2(628, 152)
	Input.parse_input_event(preview_motion)
	if not await capture("inventory-occupied-placement"): return
	click_at(Vector2(590, 75), false)
	if not check(hud.hud_snapshot().held_inventory_icon == -1, "inventory placement"): return
	click_at(Vector2(590, 75), false)
	if not check(hud.hud_snapshot().held_inventory_icon == 5, "inventory moved to new slot"): return
	click_at(Vector2(590, 75), false)
	OS.delay_msec(20)
	click_at(Vector2(770, 150), true)
	click_at(Vector2(770, 150), false)
	await process_frame
	if not check(hud.hud_snapshot().inventory_row == 8, "inventory track pages eight rows"): return
	if not await capture("inventory-scrolled"): return
	var use_requests: Array[int] = []
	hud.inventory_use_requested.connect(func(id: int, _tick: int, _name: String): use_requests.append(id))
	if not check(hud.add_inventory_item(5, 3, 51, 12345, "Test potion", 40, 3), "usable stack fixture"): return
	click_at(Vector2(665, 75), false)
	click_at(Vector2(665, 75), false)
	if not check(use_requests == [12345] and hud.hud_snapshot().inventory_use_pending, "rapid release requests use"): return
	click_at(Vector2(665, 75), false)
	if not check(use_requests.size() == 1 and not hud.apply_inventory_use_reply(634, 2), "pending use and invalid reply"): return
	if not check(hud.apply_inventory_use_reply(635, 2) and not hud.hud_snapshot().inventory_use_pending, "accepted stack returns to bag"): return
	if not await capture("inventory-use-reply"): return
	click_at(Vector2(665, 75), false)
	click_at(Vector2(665, 75), false)
	if not check(use_requests.size() == 2 and hud.apply_inventory_use_reply(636, 0), "failed use restores item"): return
	click_at(Vector2(530, 25), true)
	var drag := InputEventMouseMotion.new()
	drag.position = Vector2(450, 155)
	Input.parse_input_event(drag)
	click_at(Vector2(450, 155), false)
	await process_frame
	if not check(hud.hud_snapshot().inventory_position == Vector2i(438, 99), "inventory drag and bottom clamp"): return
	click_at(Vector2(681, 523), true)
	click_at(Vector2(681, 523), false)
	await process_frame
	if not check(not hud.hud_snapshot().inventory_visible, "inventory close button"): return
	key(KEY_W)
	await process_frame
	if not check(hud.hud_snapshot().inventory_visible, "inventory W shortcut"): return
	key(KEY_ESCAPE)
	await process_frame
	if not check(not hud.hud_snapshot().inventory_visible, "inventory escape"): return
	click_at(Vector2(720, 510), false)
	await process_frame
	if not check(hud.hud_snapshot().status_visible and hud.hud_snapshot().status_position == Vector2i.ZERO, "status HUD entry and origin"): return
	if not await capture("character-status"): return
	var status_hover := InputEventMouseMotion.new()
	status_hover.position = Vector2(227, 137)
	Input.parse_input_event(status_hover)
	if not await capture("character-status-hp"): return
	status_hover = InputEventMouseMotion.new()
	status_hover.position = Vector2(72, 388)
	Input.parse_input_event(status_hover)
	if not await capture("character-status-element"): return
	key(KEY_W)
	await process_frame
	if not check(hud.hud_snapshot().inventory_visible and hud.hud_snapshot().status_visible, "inventory and status coexist"): return
	var equipment_requests: Array[Vector2i] = []
	hud.equipment_requested.connect(func(id: int, slot: int, _index: int, _name: String): equipment_requests.append(Vector2i(id, slot)))
	if not check(hud.add_inventory_item(1520, 1, 48, 54321, "Test necklace", 20), "equipment fixture"): return
	click_at(Vector2(465, 170), false)
	var equip_hover := InputEventMouseMotion.new()
	equip_hover.position = Vector2(180, 100)
	Input.parse_input_event(equip_hover)
	if not await capture("character-equipment-preview"): return
	click_at(Vector2(180, 100), false)
	if not check(equipment_requests == [Vector2i(1003, 3)] and hud.hud_snapshot().equipment_pending, "equip request"): return
	if not check(not hud.apply_equipment_reply(619) and hud.apply_equipment_reply(615), "equip response"): return
	if not await capture("character-equipment"): return
	click_at(Vector2(180, 100), false)
	click_at(Vector2(465, 170), false)
	if not check(equipment_requests == [Vector2i(1003, 3), Vector2i(1004, 3)] and hud.apply_equipment_reply(620), "failed takeoff restores equipment"): return
	click_at(Vector2(180, 100), false)
	click_at(Vector2(465, 170), false)
	if not check(equipment_requests.size() == 3 and hud.apply_equipment_reply(619), "takeoff returns item"): return
	key(KEY_ESCAPE)
	key(KEY_ESCAPE)
	await process_frame
	if not check(not hud.hud_snapshot().status_visible and not hud.hud_snapshot().inventory_visible, "escape closes the top panel first"): return
	key(KEY_Q)
	await process_frame
	if not check(hud.hud_snapshot().status_visible, "status Q shortcut"): return
	click_at(Vector2(291, 16), true)
	click_at(Vector2(291, 16), false)
	await process_frame
	if not check(not hud.hud_snapshot().status_visible, "status close button"): return
	key(KEY_E)
	await process_frame
	if not check(hud.hud_snapshot().skills_visible and hud.hud_snapshot().skill_tab == 0, "magic opens on first tab"): return
	if not check(hud.set_learned_skill(1, 2, 1) and hud.set_learned_skill(5, 1, 0, 42, 500, "大火球") and not hud.set_learned_skill(1, 2, 13), "learned magic data"): return
	if not await capture("magic-tree-learned"): return
	var skill_requests: Array[Vector2i] = []
	hud.skill_key_requested.connect(func(id: int, legacy_key: int): skill_requests.append(Vector2i(id, legacy_key)))
	var magic_hover := InputEventMouseMotion.new()
	magic_hover.position = Vector2(385, 152)
	Input.parse_input_event(magic_hover)
	key(KEY_F1)
	key(KEY_F12)
	if not check(skill_requests == [Vector2i(1, 0), Vector2i(5, 49), Vector2i(5, 67)], "magic shortcut conflict and F12 encoding"): return
	if not await capture("magic-key-binding"): return
	if not await capture("magic-description"): return
	click_at(Vector2(385, 152), true)
	click_at(Vector2(385, 152), false)
	if not await capture("magic-progress"): return
	var popup_event := InputEventMouseButton.new()
	popup_event.position = Vector2(385, 152)
	popup_event.button_index = MOUSE_BUTTON_LEFT
	popup_event.pressed = true
	popup_event.double_click = true
	Input.parse_input_event(popup_event)
	click_at(Vector2(385, 152), false)
	magic_hover = InputEventMouseMotion.new()
	magic_hover.position = Vector2(435, 202)
	Input.parse_input_event(magic_hover)
	if not await capture("magic-key-menu"): return
	click_at(Vector2(435, 202), true)
	click_at(Vector2(435, 202), false)
	if not check(skill_requests.back() == Vector2i(5, 50), "shortcut menu binds F2"): return
	click_at(Vector2(636, 32), true)
	click_at(Vector2(636, 32), false)
	if not check(hud.hud_snapshot().skill_tab == 7, "magic category selection"): return
	OS.delay_msec(20)
	click_at(Vector2(671, 160), true)
	click_at(Vector2(671, 160), false)
	if not check(hud.hud_snapshot().skill_scroll == 6, "magic scroll step"): return
	if not await capture("magic-tree-scrolled"): return
	var skill_wheel := InputEventMouseButton.new()
	skill_wheel.position = Vector2(490, 210)
	skill_wheel.button_index = MOUSE_BUTTON_WHEEL_DOWN
	skill_wheel.pressed = true
	Input.parse_input_event(skill_wheel)
	if not check(hud.hud_snapshot().skill_scroll == 36, "magic wheel advances 30 pixels"): return
	for i in 8: Input.parse_input_event(skill_wheel)
	if not check(hud.hud_snapshot().skill_scroll == 190, "magic wheel stops at bottom"): return
	if not await capture("magic-wheel-bottom"): return
	skill_wheel.button_index = MOUSE_BUTTON_WHEEL_UP
	for i in 8: Input.parse_input_event(skill_wheel)
	if not check(hud.hud_snapshot().skill_scroll == 0, "magic wheel stops at top"): return
	key(KEY_ESCAPE)
	await process_frame
	if not check(not hud.hud_snapshot().skills_visible, "magic escape"): return
	var allowance_requests: Array[bool] = []
	hud.group_allow_requested.connect(func(allowed: bool): allowance_requests.append(allowed))
	hud.set_group_roster("比奇旅人", PackedStringArray(["比奇旅人", "银杏剑客", "边境法师", "山谷道士"]))
	key(KEY_G)
	await process_frame
	if not check(hud.hud_snapshot().group_visible, "group keyboard opens"): return
	click_at(Vector2(300, 174), true)
	click_at(Vector2(300, 174), false)
	if not check(hud.hud_snapshot().group_allow and allowance_requests == [true], "group allowance request"): return
	if not await capture("group-members"): return
	var member_requests: Array = []
	hud.group_member_requested.connect(func(id: int, name: String): member_requests.append([id, name]))
	click_at(Vector2(375, 322), true)
	click_at(Vector2(375, 322), false)
	if not check(hud.hud_snapshot().group_request == 1021, "open add member dialog"): return
	for ch in "银杏剑客": key(0, ch.unicode_at(0))
	if not await capture("group-member-dialog"): return
	key(KEY_ENTER)
	if not check(member_requests == [[1021, "银杏剑客"]], "submit member request"): return
	click_at(Vector2(435, 322), true)
	click_at(Vector2(435, 322), false)
	if not check(hud.hud_snapshot().group_request == 1022, "open remove member dialog"): return
	key(KEY_TAB)
	key(KEY_ENTER)
	if not check(hud.hud_snapshot().group_request == 0 and member_requests.size() == 1, "cancel member request"): return
	key(KEY_ESCAPE)
	await process_frame
	if not check(not hud.hud_snapshot().group_visible, "group escape closes"): return
	var guild_queries: Array[bool] = []
	hud.guild_overview_requested.connect(func(): guild_queries.append(true))
	click_at(Vector2(686, 485), false)
	if not check(guild_queries == [true] and not hud.hud_snapshot().guild_visible, "guild overview requested from HUD"): return
	hud.set_guild_overview("比奇行会", PackedStringArray(["欢迎加入比奇行会", "今晚集合练级"]))
	if not await capture("guild-overview"): return
	click_at(Vector2(196, 410), true)
	click_at(Vector2(196, 410), false)
	if not check(hud.hud_snapshot().guild_tab == 1, "guild members tab"): return
	hud.set_guild_members(PackedStringArray(["会长  比奇旅人", "会员  银杏剑客"]))
	if not await capture("guild-members"): return
	hud.set_guild_notices(PackedStringArray(["第一条行会通知", "第二条行会通知"]))
	click_at(Vector2(246, 410), true)
	click_at(Vector2(246, 410), false)
	if not check(hud.hud_snapshot().guild_tab == 2, "guild notices tab"): return
	var guild_wheel := InputEventMouseButton.new()
	guild_wheel.position = Vector2(250, 180)
	guild_wheel.button_index = MOUSE_BUTTON_WHEEL_DOWN
	guild_wheel.pressed = true
	Input.parse_input_event(guild_wheel)
	if not check(hud.hud_snapshot().guild_first == 1, "guild notices wheel"): return
	if not await capture("guild-notices"): return
	click_at(Vector2(450, 415), true)
	click_at(Vector2(450, 415), false)
	if not check(hud.hud_snapshot().group_request == 4000, "guild permission dialog"): return
	if not await capture("guild-denied"): return
	key(KEY_ENTER)
	hud.set_guild_manager(true)
	var removals: Array[String] = []
	hud.guild_member_removal_requested.connect(func(name: String): removals.append(name))
	click_at(Vector2(450, 415), true)
	click_at(Vector2(450, 415), false)
	if not check(hud.hud_snapshot().group_request == 1039, "guild removal dialog"): return
	for ch in "银杏剑客": key(0, ch.unicode_at(0))
	if not await capture("guild-remove"): return
	key(KEY_ENTER)
	if not check(removals == ["银杏剑客"], "guild removal request"): return
	var guild_edits: Array = []
	hud.guild_edit_requested.connect(func(id: int, body: String): guild_edits.append([id, body]))
	click_at(Vector2(400, 415), true)
	click_at(Vector2(400, 415), false)
	var editor: TextEdit = hud.get_node("GuildEdit")
	if not check(editor.visible and editor.has_focus() and editor.size == Vector2(468, 170), "guild announcement editor layout"): return
	editor.text = "行会公告\n\n今晚集合"
	if not await capture("guild-announcement-editor"): return
	click_at(Vector2(575, 130), true)
	click_at(Vector2(575, 130), false)
	if not check(not editor.visible and guild_edits == [[1040, "行会公告\r \r今晚集合"]], "guild announcement submit"): return
	click_at(Vector2(550, 415), true)
	click_at(Vector2(550, 415), false)
	if not check(editor.visible, "guild member editor opens"): return
	if not await capture("guild-member-editor"): return
	click_at(Vector2(660, 328), true)
	click_at(Vector2(660, 328), false)
	if not check(not editor.visible and guild_edits.size() == 1, "guild editor cancel"): return
	key(KEY_ESCAPE)
	if not check(not hud.hud_snapshot().guild_visible, "guild escape closes"): return
	if not check(hud.set_music_map("0"), "original map music lookup"): return
	var music: AudioStreamPlayer = hud.get_node("MapMusic")
	var original_track := music.stream
	if not check(original_track != null and music.playing, "map music starts"): return
	if not check(hud.set_music_map("D2011") and music.stream == original_track, "no music sentinel preserves track"): return
	key(KEY_N)
	if not check(hud.hud_snapshot().options_visible, "options keyboard entry"): return
	click_at(Vector2(580, 120), false)
	if not check(not hud.hud_snapshot().music_enabled and not music.playing, "music toggle"): return
	click_at(Vector2(460, 154), false)
	if not check(hud.hud_snapshot().music_level == -5 and is_equal_approx(music.volume_db, -2.0), "music slider step"): return
	var click_sound: AudioStreamPlayer = hud.get_node("HudClickSound")
	if not check(click_sound.stream != null, "original HUD click sample loads"): return
	click_at(Vector2(460, 214), false)
	if not check(is_equal_approx(click_sound.volume_db, -2.0), "effect slider updates player volume"): return
	click_at(Vector2(580, 180), false)
	if not check(not click_sound.playing, "effects switch stops click sound"): return
	if not await capture("system-options"): return
	click_at(Vector2(595, 447), false)
	if not check(not hud.hud_snapshot().options_visible, "options close"): return
	var reloaded: Node = load("res://main.tscn").instantiate()
	reloaded.set_meta("options_path", options_file)
	root.add_child(reloaded)
	var persisted: Dictionary = reloaded.hud_snapshot()
	reloaded.free()
	DirAccess.remove_absolute(ProjectSettings.globalize_path(options_file))
	if not check(not persisted.music_enabled and persisted.music_level == -5, "system options reload from original INI keys"): return
	click_at(Vector2(540, 570), false)
	if not check(hud.hud_snapshot().portrait_visible, "portrait HUD entry"): return
	click_at(Vector2(40, 32), true)
	click_at(Vector2(40, 32), false)
	if not check(hud.hud_snapshot().portrait_selected == 0 and hud.hud_snapshot().portrait_count == 1 and hud.hud_snapshot().portrait_preview, "portrait row selection"): return
	if not await capture("portrait-preview"): return
	var portrait_server := "smoke-%d" % OS.get_process_id()
	hud.load_portrait_cache(portrait_server, "player")
	var portrait_uploads: Array[PackedByteArray] = []
	hud.portrait_upload_requested.connect(func(jpeg: PackedByteArray): portrait_uploads.append(jpeg))
	click_at(Vector2(70, 350), true)
	click_at(Vector2(70, 350), false)
	if not check(portrait_uploads.size() == 1 and not portrait_uploads[0].is_empty(), "portrait upload reads actual file"): return
	if not check(not hud.hud_snapshot().custom_portrait and hud.apply_portrait_reply(portrait_uploads[0]), "portrait applies on reply"): return
	if not check(hud.load_portrait_cache(portrait_server, "player") and hud.hud_snapshot().custom_portrait, "portrait reloads from FAC"): return
	if not await capture("portrait-cached"): return
	click_at(Vector2(185, 350), true)
	click_at(Vector2(185, 350), false)
	if not check(portrait_uploads.size() == 2 and portrait_uploads[1].is_empty() and hud.apply_portrait_reply(portrait_uploads[1]), "reset portrait reply"): return
	if not check(not hud.hud_snapshot().custom_portrait, "restore profession portrait"): return
	var names := PackedStringArray()
	for index in range(14): names.append("portrait-%02d.jpg" % index)
	hud.set_portrait_names(names)
	var portrait_wheel := InputEventMouseButton.new()
	portrait_wheel.position = Vector2(48, 40)
	portrait_wheel.button_index = MOUSE_BUTTON_WHEEL_DOWN
	portrait_wheel.pressed = true
	Input.parse_input_event(portrait_wheel)
	if not check(hud.hud_snapshot().portrait_first == 1, "portrait wheel advances one row"): return
	if not await capture("portrait-scrolled"): return
	if not check(hud.load_portrait_cache(portrait_server, "player") and not hud.hud_snapshot().custom_portrait, "FAC preserves default portrait"): return
	DirAccess.remove_absolute("user://%s_player.fac" % portrait_server)
	DirAccess.remove_absolute(portrait_dir.path_join("sample.JPG"))
	DirAccess.remove_absolute(portrait_dir)
	key(KEY_ESCAPE)
	if not check(not hud.hud_snapshot().portrait_visible, "portrait escape"): return
	var input: LineEdit = hud.get_node("ChatInput")
	if not check(input.position == Vector2(185, 575) and input.size == Vector2(344, 15), "compact input bounds"): return
	key(KEY_R)
	if not check(hud.hud_snapshot().expanded and input.position == Vector2(185, 534), "R expands chat and moves input"): return
	if not await capture("hud-chat-keyboard-expanded"): return
	key(KEY_R)
	if not check(not hud.hud_snapshot().expanded and input.position == Vector2(185, 575), "R restores compact chat"): return
	key(KEY_ENTER)
	await process_frame
	if not check(input.has_focus(), "enter focuses input"): return
	key(KEY_R, "r".unicode_at(0))
	if not check(not hud.hud_snapshot().expanded and input.text == "r", "focused R types instead of toggling"): return
	key(KEY_BACKSPACE)
	for ch in "你好，比奇省": key(0, ch.unicode_at(0))
	await process_frame
	if not check(input.text == "你好，比奇省", "Unicode input events"): return
	if not await capture("hud-chat-input"): return
	var count: int = hud.hud_snapshot().chat_compact_count
	key(KEY_ENTER)
	await process_frame
	if not check(requests == ["你好，比奇省"] and input.text.is_empty() and not input.has_focus() and hud.hud_snapshot().chat_compact_count == count, "send request without fake reply"): return
	key(KEY_SPACE)
	await process_frame
	for i in 46: key(0, "测".unicode_at(0))
	await process_frame
	if not check(input.text == "测".repeat(45), "90 ANSI byte limit in widget"): return
	key(KEY_ESCAPE)
	await process_frame
	if not check(not input.has_focus() and input.text == "测".repeat(45), "escape preserves text"): return
	click_at(Vector2(620, 473), false)
	click_at(Vector2(270, 560), false)
	await process_frame
	key(KEY_ENTER)
	await process_frame
	if not check(hud.hud_snapshot().chat_mode == 1 and input.text == "!!" and input.position == Vector2(185, 534) and input.size == Vector2(432, 15), "group prefix and expanded input"): return
	for ch in "组队聊天测试": key(0, ch.unicode_at(0))
	await process_frame
	if not await capture("hud-chat-group-input"): return
	key(KEY_ESCAPE)
	await process_frame
	if not check(hud.hud_snapshot().reception_hints == PackedInt32Array([2637, 2639, 2643]), "initial reception hints"): return
	for x in [350, 390, 430]:
		click_at(Vector2(x, 560), true)
		click_at(Vector2(x, 560), false)
	await process_frame
	if not check(requests.slice(1) == ["@拒绝悄悄话", "@拒绝行会聊天", "@拒绝所有人聊天 "], "original command payloads"): return
	if not check(hud.hud_snapshot().reception_frames == PackedInt32Array([1338, 1340, 1342]) and hud.hud_snapshot().rejected_chat == PackedInt32Array([0, 0, 0]), "predicted skins without fabricated acknowledgement"): return
	if not await capture("hud-chat-reception"): return
	for i in 3:
		if not check(hud.update_chat_rejection(i, true), "external rejection state"): return
	if not check(not hud.update_chat_rejection(3, true), "invalid rejection state channel"): return
	for x in [350, 390, 430]: click_at(Vector2(x, 560), false)
	await process_frame
	if not check(hud.hud_snapshot().reception_frames == PackedInt32Array([-1, -1, -1]) and hud.hud_snapshot().reception_hints == PackedInt32Array([2637, 2639, 2643]), "allow skins from supplied state"): return
	if not await capture("hud-chat-reception-allow"): return
	print("HUD_SMOKE_OK: resources, expansion, collapse, stat switching, action dispatch")
	quit(0)
