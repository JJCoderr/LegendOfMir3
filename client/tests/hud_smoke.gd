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

func key(code: int, unicode_value: int = 0, alt: bool = false) -> void:
	var event := InputEventKey.new()
	event.keycode = code
	event.physical_keycode = code
	event.unicode = unicode_value
	event.alt_pressed = alt
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
	await create_timer(0.05).timeout
	click_at(Vector2(621, 397), true)
	key(KEY_N)
	var chat_drag := InputEventMouseMotion.new()
	chat_drag.position = Vector2(400, 455)
	Input.parse_input_event(chat_drag)
	click_at(chat_drag.position, false)
	var drag_first: int = hud.hud_snapshot().chat_expanded_first
	if not check(drag_first > 5, "chat drag remains captured across overlapping options"): return
	chat_drag = chat_drag.duplicate()
	chat_drag.position = Vector2(621, 360)
	Input.parse_input_event(chat_drag)
	if not check(hud.hud_snapshot().chat_expanded_first == drag_first, "covered release ends chat drag"): return
	if not await capture("hud-chat-scroll-overlap"): return
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
	var map_requests: Array[int] = []
	hud.minimap_requested.connect(func(identifier: int): map_requests.append(identifier))
	hud.set_map_location("比奇省", 337, 267)
	key(KEY_V)
	if not check(map_requests == [1033] and not hud.hud_snapshot().minimap_visible, "minimap waits for reply"): return
	if not check(hud.apply_minimap_reply(1001), "read Bichon from original Fmmap"): return
	if not check(hud.hud_snapshot().minimap_bounds == Rect2(672, 0, 128, 128), "compact minimap geometry"): return
	if not check(hud.hud_snapshot().minimap_source == Vector2(441, 203), "minimap centered on player"): return
	key(KEY_V)
	if not check(hud.hud_snapshot().minimap_visible, "minimap reply cooldown"): return
	hud.set_minimap_markers(PackedVector2Array([Vector2(322, 251)]),
		PackedVector2Array([Vector2(350, 284)]), PackedVector2Array([Vector2(360, 276), Vector2(380, 283)]))
	if not await capture("hud-minimap-markers"): return
	if OS.get_cmdline_user_args().has("--screenshots"):
		var rendered := root.get_texture().get_image()
		if not check(rendered.get_pixel(713, 47).is_equal_approx(Color.YELLOW), "yellow minimap marker"): return
		if not check(rendered.get_pixel(755, 80).is_equal_approx(Color8(100, 250, 100)), "green minimap marker"): return
		if not check(rendered.get_pixel(770, 72).is_equal_approx(Color8(250, 150, 50)), "orange minimap marker"): return
		if not check(not rendered.get_pixel(799, 79).is_equal_approx(Color8(250, 150, 50)), "distant minimap marker clipped"): return
	hud.set_minimap_position(350, 280)
	if not check(hud.hud_snapshot().map_position == Vector2i(350, 280) and hud.hud_snapshot().minimap_source == Vector2(461, 216), "HUD and minimap follow the same movement"): return
	if not await capture("hud-map-location"): return
	hud.set_minimap_position(337, 267)
	var map_hint_motion := InputEventMouseMotion.new()
	map_hint_motion.position = Vector2(792, 120)
	Input.parse_input_event(map_hint_motion)
	await create_timer(0.3).timeout
	if not await capture("hud-minimap-size-hint"): return
	if OS.get_cmdline_user_args().has("--screenshots"):
		var rendered := root.get_texture().get_image()
		if not check(rendered.get_pixel(797, 124).is_equal_approx(Color8(255, 255, 150)), "minimap size tooltip background"): return
	click_at(Vector2(792, 120), true)
	click_at(Vector2(792, 120), false)
	if not check(hud.hud_snapshot().minimap_bounds == Rect2(544, 0, 256, 256), "minimap size button"): return
	key(KEY_Y)
	if not check(hud.hud_snapshot().minimap_translucent, "minimap transparency shortcut"): return
	if not await capture("hud-minimap-expanded"): return
	key(KEY_T)
	if not check(hud.hud_snapshot().minimap_bounds == Rect2(672, 0, 128, 128), "minimap size shortcut"): return
	var map_drag := InputEventMouseButton.new()
	map_drag.button_index = MOUSE_BUTTON_LEFT
	map_drag.ctrl_pressed = true
	map_drag.pressed = true
	map_drag.position = Vector2(730, 64)
	Input.parse_input_event(map_drag)
	var map_motion := InputEventMouseMotion.new()
	map_motion.position = Vector2(710, 88)
	map_motion.ctrl_pressed = true
	Input.parse_input_event(map_motion)
	map_drag = map_drag.duplicate()
	map_drag.position = map_motion.position
	map_drag.pressed = false
	Input.parse_input_event(map_drag)
	if not check(hud.hud_snapshot().minimap_bounds == Rect2(652, 24, 128, 128), "minimap Ctrl drag"): return
	await create_timer(1.01).timeout
	key(KEY_V)
	if not check(not hud.hud_snapshot().minimap_visible, "minimap closes with V"): return
	click_at(Vector2(643, 513), true)
	click_at(Vector2(643, 513), false)
	if not check(map_requests == [1033, 1033] and hud.apply_minimap_reply(1), "HUD button requests original Mmap"): return
	hud.set_map_location("盟重省", 220, 240)
	if not check(map_requests == [1033, 1033, 1033] and not hud.hud_snapshot().minimap_visible, "map transition clears old frame and requests refresh"): return
	if not check(not hud.apply_minimap_reply(0) and not hud.hud_snapshot().minimap_visible, "invalid reply keeps previous map hidden"): return
	if not check(hud.apply_minimap_reply(1002) and hud.hud_snapshot().minimap_visible, "new map reply restores open minimap"): return
	hud.set_map_location("盟重省", 221, 241)
	if not check(map_requests.size() == 3 and hud.hud_snapshot().minimap_visible, "movement in same map does not re-request"): return
	if not await capture("hud-minimap-map-transition"): return
	key(KEY_Y)
	await create_timer(0.2).timeout
	click_at(Vector2(643, 513), true)
	click_at(Vector2(643, 513), false)
	if not check(not hud.hud_snapshot().minimap_visible, "close refreshed minimap"): return
	hud.set_map_location("比奇省", 337, 267)
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
	if not check(hud.set_character_vitals(36, 60, 12, 40, 25, 80), "apply character vitals"): return
	if not check(not hud.set_character_vitals(1, 65536, 0, 0, 0, 0), "reject out-of-range character vitals"): return
	var vitals: Dictionary = hud.hud_snapshot()
	if not check(vitals.character_hp == Vector2i(36, 60) and vitals.character_mp == Vector2i(12, 40) and vitals.character_weight == Vector2i(25, 80), "character vitals update together"): return
	if not await capture("character-status-vitals"): return
	if not check(hud.set_character_load_bonuses(Vector2i(9, 28), Vector2i(7, 15), Vector2i(6, 12)), "apply wear and hand weights and bonuses"): return
	if not check(not hud.set_character_load_bonuses(Vector2i.ZERO, Vector2i.ZERO, Vector2i(256, 0)), "reject invalid bonus byte"): return
	var load_stats: Dictionary = hud.hud_snapshot()
	if not check(load_stats.character_wear_weight == Vector2i(9, 28) and load_stats.character_hand_weight == Vector2i(7, 15) and load_stats.character_bonuses == Vector2i(6, 12), "keep weights and bonuses after invalid update"): return
	if not await capture("character-status-load-bonuses"): return
	if not check(hud.set_character_progress(24, 0, 1, 350, 1000), "apply character progress"): return
	if not check(not hud.set_character_progress(24, 3, 0, 0, 0), "reject unknown character profession"): return
	var progress: Dictionary = hud.hud_snapshot()
	if not check(progress.character_level == 24 and progress.character_portrait == 651 and not progress.character_shared_hp, "level switches orbs and appearance selects portrait"): return
	if not await capture("character-status-progress"): return
	if not check(hud.set_character_combat(Vector2i(12, 31), Vector2i(2, 8), Vector2i(4, 16), Vector2i(5, 12), Vector2i(3, 7)), "apply combat ranges"): return
	if not check(not hud.set_character_combat(Vector2i(31, 12), Vector2i.ZERO, Vector2i.ZERO, Vector2i.ZERO, Vector2i.ZERO), "reject reversed combat range"): return
	var combat: Dictionary = hud.hud_snapshot()
	if not check(combat.hud_attack == Vector2i(12, 31) and combat.hud_defence == Vector2i(3, 7), "combat data drives HUD ranges"): return
	if not await capture("character-status-combat"): return
	click_at(Vector2(640, 580), true)
	click_at(Vector2(640, 580), false)
	if not check(hud.hud_snapshot().hud_defence == Vector2i(5, 12), "HUD toggles to physical defence"): return
	click_at(Vector2(640, 580), true)
	click_at(Vector2(640, 580), false)
	if not check(hud.set_character_progress(1, 0, 0, 0, 0), "restore default character progress"): return
	if not check(hud.set_character_vitals(0, 0, 0, 0, 0, 0), "restore empty character vitals"): return
	status_hover = InputEventMouseMotion.new()
	status_hover.position = Vector2(72, 388)
	Input.parse_input_event(status_hover)
	if not check(hud.set_character_elements(PackedByteArray([2, 0, 0, 4, 0, 0, 7]), PackedByteArray([1, 255, 0, 0, 2, 0, 0]), PackedByteArray([0, 0, 3, 0, 0, 0, 1])), "apply raw character elements"): return
	if not check(not hud.set_character_elements(PackedByteArray(), PackedByteArray(), PackedByteArray()), "reject incomplete element rows"): return
	if not await capture("character-status-element"): return
	hud.set_character_identity("比奇旅人", "", "", "")
	if not await capture("character-status-name"): return
	hud.set_character_identity("比奇旅人", "比奇城", "守望者", "比奇")
	if not await capture("character-status-identity"): return
	hud.set_character_identity("", "", "", "")
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
	var spell_requests: Array[Array] = []
	hud.spell_key_requested.connect(func(slot: int, id: int, pointer: Vector2): spell_requests.append([slot, id, pointer]))
	key(KEY_F1)
	key(KEY_F12)
	if not check(spell_requests.is_empty(), "skill tree binding does not cast"): return
	if not check(skill_requests == [Vector2i(1, 0), Vector2i(5, 49), Vector2i(5, 67)], "magic shortcut conflict and F12 encoding"): return
	if not await capture("magic-key-binding"): return
	key(KEY_B)
	if not check(hud.hud_snapshot().skill_shortcuts_visible, "B opens magic shortcuts"): return
	if not await capture("hud-magic-shortcuts"): return
	var cast_pointer := InputEventMouseMotion.new()
	cast_pointer.position = Vector2(180, 160)
	Input.parse_input_event(cast_pointer)
	key(KEY_F1)
	key(KEY_F12)
	if not check(spell_requests == [[12, 5, Vector2(180, 160)]], "only bound shortcut dispatches spell"): return
	if not await capture("hud-spell-key-dispatch"): return
	for point in [Vector2(390, 30), Vector2(340, 480), Vector2(340, 550)]:
		cast_pointer = cast_pointer.duplicate()
		cast_pointer.position = point
		Input.parse_input_event(cast_pointer)
		key(KEY_F12)
	if not check(spell_requests.size() == 1, "HUD regions block spell requests"): return
	cast_pointer = cast_pointer.duplicate()
	cast_pointer.position = Vector2(180, 160)
	Input.parse_input_event(cast_pointer)
	key(KEY_V)
	if not check(hud.apply_minimap_reply(1001), "open minimap for shortcut gate"): return
	key(KEY_F12)
	if not check(spell_requests.size() == 1, "open minimap blocks scene spell request"): return
	if not await capture("hud-spell-minimap-gate"): return
	await create_timer(0.2).timeout
	click_at(Vector2(643, 513), true)
	click_at(Vector2(643, 513), false)
	if not check(not hud.hud_snapshot().minimap_visible, "close minimap before resuming spells"): return
	cast_pointer = cast_pointer.duplicate()
	Input.parse_input_event(cast_pointer)
	key(KEY_F12)
	if not check(spell_requests.size() == 2, "spell request resumes after closing minimap"): return
	magic_hover = magic_hover.duplicate()
	Input.parse_input_event(magic_hover)
	key(KEY_B)
	if not check(not hud.hud_snapshot().skill_shortcuts_visible, "B closes magic shortcuts"): return
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
	click_at(Vector2(490, 316), true)
	var modal_drag := InputEventMouseMotion.new()
	modal_drag.position = Vector2(280, 348)
	Input.parse_input_event(modal_drag)
	click_at(Vector2(280, 348), false)
	click_at(Vector2(490, 316), false)
	if not check(hud.hud_snapshot().group_request == 1022, "release over edit clears modal button press"): return
	if not await capture("group-dialog-release"): return
	key(KEY_TAB)
	key(KEY_ENTER)
	if not check(hud.hud_snapshot().group_request == 0 and member_requests.size() == 1, "cancel member request"): return
	key(KEY_ESCAPE)
	await process_frame
	if not check(not hud.hud_snapshot().group_visible, "group escape closes"): return
	key(KEY_E)
	key(KEY_G)
	key(KEY_W)
	key(KEY_Q)
	if not await capture("hud-panel-stack"): return
	key(KEY_ESCAPE)
	if not check(not hud.hud_snapshot().status_visible and hud.hud_snapshot().inventory_visible, "status closes first in four-panel stack"): return
	key(KEY_ESCAPE)
	if not check(not hud.hud_snapshot().inventory_visible and hud.hud_snapshot().group_visible, "inventory closes second"): return
	key(KEY_ESCAPE)
	if not check(not hud.hud_snapshot().group_visible and hud.hud_snapshot().skills_visible, "group closes third"): return
	key(KEY_ESCAPE)
	if not check(not hud.hud_snapshot().skills_visible, "magic closes last"): return
	var guild_queries: Array[bool] = []
	hud.guild_overview_requested.connect(func(): guild_queries.append(true))
	click_at(Vector2(686, 485), false)
	if not check(guild_queries == [true] and not hud.hud_snapshot().guild_visible, "guild overview requested from HUD"): return
	hud.set_guild_overview("比奇行会", PackedStringArray(["欢迎加入比奇行会", "今晚集合练级"]), PackedStringArray(["今晚集合练级"]))
	if not await capture("guild-overview"): return
	var guild_hover := InputEventMouseMotion.new()
	guild_hover.position = Vector2(196, 410)
	Input.parse_input_event(guild_hover)
	for i in 30: await process_frame
	if not await capture("guild-button-hint"): return
	click_at(Vector2(196, 410), true)
	click_at(Vector2(196, 410), false)
	if not check(hud.hud_snapshot().guild_tab == 1, "guild members tab"): return
	hud.set_guild_members(PackedStringArray(["会长  比奇旅人", "会员  银杏剑客"]), PackedStringArray(["#1 会长", "比奇旅人", "#2 会员", "银杏剑客"]))
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
	var alliance_events: Array[String] = []
	var record_command := func(value: String): alliance_events.append(value)
	var record_refresh := func(): alliance_events.append("refresh")
	hud.chat_requested.connect(record_command)
	hud.guild_overview_requested.connect(record_refresh)
	click_at(Vector2(597, 414), true)
	click_at(Vector2(597, 414), false)
	if not check(hud.hud_snapshot().group_request == 1045, "open break alliance dialog"): return
	for ch in "银杏行会": key(0, ch.unicode_at(0))
	if not await capture("guild-break-alliance"): return
	key(KEY_ENTER)
	if not check(alliance_events == ["破坏结盟 银杏行会", "refresh"], "break alliance command then overview refresh"): return
	requests.pop_back()
	click_at(Vector2(597, 414), true)
	click_at(Vector2(597, 414), false)
	key(KEY_TAB)
	key(KEY_ENTER)
	if not check(hud.hud_snapshot().group_request == 0 and alliance_events.size() == 2, "cancel alliance change emits nothing"): return
	hud.chat_requested.disconnect(record_command)
	hud.guild_overview_requested.disconnect(record_refresh)
	var guild_edits: Array = []
	hud.guild_edit_requested.connect(func(id: int, body: String): guild_edits.append([id, body]))
	click_at(Vector2(400, 415), true)
	click_at(Vector2(400, 415), false)
	var editor: TextEdit = hud.get_node("GuildEdit")
	if not check(editor.visible and editor.has_focus() and editor.size == Vector2(468, 170), "guild announcement editor layout"): return
	if not check(editor.text == "今晚集合练级\n", "announcement prefill uses editable lines only"): return
	editor.text = "行会公告\n\n今晚集合"
	if not await capture("guild-announcement-editor"): return
	click_at(Vector2(575, 130), true)
	click_at(Vector2(575, 130), false)
	if not check(not editor.visible and guild_edits == [[1040, "行会公告\r \r今晚集合"]], "guild announcement submit"): return
	click_at(Vector2(550, 415), true)
	click_at(Vector2(550, 415), false)
	if not check(editor.visible and editor.text == "#1 会长\n比奇旅人\n#2 会员\n银杏剑客\n", "guild member editor uses rank source"): return
	if not await capture("guild-member-editor"): return
	click_at(Vector2(660, 328), true)
	click_at(Vector2(660, 328), false)
	if not check(not editor.visible and guild_edits.size() == 1, "guild editor cancel"): return
	key(KEY_N)
	key(KEY_E)
	if not await capture("hud-six-panel-order"): return
	key(KEY_ESCAPE)
	if not check(not hud.hud_snapshot().skills_visible and hud.hud_snapshot().options_visible and hud.hud_snapshot().guild_visible, "magic above options and guild"): return
	key(KEY_ESCAPE)
	if not check(not hud.hud_snapshot().options_visible and hud.hud_snapshot().guild_visible, "options above guild"): return
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
	key(KEY_Q)
	if not await capture("portrait-behind-status"): return
	key(KEY_ESCAPE)
	if not check(not hud.hud_snapshot().status_visible and hud.hud_snapshot().portrait_visible, "status closes before underlying portrait picker"): return
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
	var mount_button := Vector2(689, 542)
	click_at(mount_button, true)
	click_at(mount_button, false)
	if not check(hud.hud_snapshot().mount_visible and hud.hud_snapshot().mount_bounds == Rect2(0, 0, 276, 322), "mount window HUD button and original bounds"): return
	if not await capture("hud-mount-window"): return
	var before_mount := requests.size()
	for point in [Vector2(85, 240), Vector2(25, 240)]:
		click_at(point, true)
		click_at(point, false)
	if not check(requests.slice(before_mount) == ["@骑马"], "ride allowed and dismount suppressed while on foot"): return
	if not check(hud.hud_snapshot().mount_shape == 0, "ride request waits for acknowledgement"): return
	if not check(not hud.apply_mount_reply(1301, 2) and hud.hud_snapshot().mount_shape == 0, "unrelated reply keeps mount state"): return
	if not check(hud.apply_mount_reply(1300, 0x12340002) and hud.hud_snapshot().mount_shape == 2, "mount reply uses low recognition byte"): return
	var dismount_motion := InputEventMouseMotion.new()
	dismount_motion.position = Vector2(85, 240)
	Input.parse_input_event(dismount_motion)
	await create_timer(0.3).timeout
	if not await capture("hud-mount-dismount-hint"): return
	for point in [Vector2(25, 240), Vector2(85, 240), Vector2(140, 240), Vector2(200, 240)]:
		click_at(point, true)
		click_at(point, false)
	if not check(requests.slice(before_mount) == ["@骑马", "@下马", "@包马", "@下马"], "original mounted and visibility commands"): return
	if not check(hud.hud_snapshot().mount_shape == 2, "dismount waits for acknowledgement"): return
	if not check(hud.apply_mount_reply(1300, 0x12340000) and hud.hud_snapshot().mount_shape == 0, "dismount reply clears shape"): return
	click_at(Vector2(250, 288), true)
	click_at(Vector2(250, 288), false)
	if not check(not hud.hud_snapshot().mount_visible, "mount close button"): return
	key(KEY_S)
	if not check(hud.hud_snapshot().mount_visible, "original S shortcut"): return
	click_at(Vector2(100, 20), true)
	var mount_drag := InputEventMouseMotion.new()
	mount_drag.position = Vector2(140, 50)
	Input.parse_input_event(mount_drag)
	click_at(mount_drag.position, false)
	if not check(hud.hud_snapshot().mount_bounds == Rect2(40, 30, 276, 322), "mount window dragging"): return
	key(KEY_S)
	if not check(not hud.hud_snapshot().mount_visible, "S closes mount window"): return
	click_at(Vector2(750, 490), true)
	click_at(Vector2(750, 490), false)
	if not check(hud.hud_snapshot().quest_visible and hud.hud_snapshot().quest_bounds == Rect2(0, 0, 340, 440), "quest HUD button and original bounds"): return
	if not await capture("hud-quest-window"): return
	var quest_names := PackedStringArray(["比奇省的委托", "送信给药店老板", "寻找失踪的矿工"])
	if not check(hud.set_quest_entries(quest_names, PackedInt32Array([0, 1, 0])), "apply quest list"): return
	var quest_details: Array[int] = []
	hud.quest_detail_requested.connect(func(index: int): quest_details.append(index))
	click_at(Vector2(83, 110), true)
	click_at(Vector2(83, 110), false)
	if not check(hud.hud_snapshot().quest_selected == 1, "quest row selection"): return
	click_at(Vector2(280, 90), true)
	click_at(Vector2(280, 90), false)
	if not check(quest_details == [1], "quest detail button uses selected entry"): return
	if not await capture("hud-quest-list"): return
	if not check(hud.set_quest_details(1, PackedStringArray(["到比奇城药店交付书信。", "回报后领取奖励。"])), "apply quest detail response"): return
	if not await capture("hud-quest-detail"): return
	click_at(Vector2(83, 155), true)
	click_at(Vector2(83, 155), false)
	if not check(hud.hud_snapshot().quest_selected == 2, "inline detail shifts following quest hit box"): return
	click_at(Vector2(83, 110), true)
	click_at(Vector2(83, 110), false)
	click_at(Vector2(280, 90), true)
	click_at(Vector2(280, 90), false)
	if not check(hud.set_quest_details(1, PackedStringArray(["请先前往比奇城药店交付书信，再回到城门处寻找委托人领取奖励。"])), "apply unwrapped quest detail"): return
	if not await capture("hud-quest-detail-wrapped"): return
	click_at(Vector2(83, 155), true)
	click_at(Vector2(83, 155), false)
	if not check(hud.hud_snapshot().quest_selected == 2, "wrapped quest detail occupies two rows"): return
	quest_names.clear()
	var quest_flags := PackedInt32Array()
	for i in 25:
		quest_names.append("比奇省委托 %02d" % i)
		quest_flags.append(i % 2)
	if not check(hud.set_quest_entries(quest_names, quest_flags), "apply long quest list"): return
	click_at(Vector2(278, 310), true)
	click_at(Vector2(278, 310), false)
	if not check(hud.hud_snapshot().quest_first == 5, "quest scrollbar page step"): return
	click_at(Vector2(278, 202), true)
	var quest_drag := InputEventMouseMotion.new()
	quest_drag.position = Vector2(278, 390)
	Input.parse_input_event(quest_drag)
	click_at(quest_drag.position, false)
	if not check(hud.hud_snapshot().quest_first == 24, "quest scrollbar drags to last row"): return
	if not await capture("hud-quest-scroll-end"): return
	quest_drag = quest_drag.duplicate()
	quest_drag.position = Vector2(278, 160)
	Input.parse_input_event(quest_drag)
	if not check(hud.hud_snapshot().quest_first == 24, "released quest scrollbar ignores movement"): return
	click_at(Vector2(275, 115), true)
	click_at(Vector2(275, 115), false)
	if not check(not hud.hud_snapshot().quest_visible, "quest close button"): return
	key(KEY_D)
	if not check(hud.hud_snapshot().quest_visible, "quest D shortcut"): return
	key(KEY_D)
	if not check(not hud.hud_snapshot().quest_visible, "D closes quest window"): return
	key(KEY_R)
	var trades: Array[Array] = []
	hud.trade_requested.connect(func(id: int, player: String): trades.append([id, player]))
	hud.set_map_location("比奇省", 337, 267)
	for target in [[Vector2i(337, 266), 0, false], [Vector2i(338, 267), 3, false], [Vector2i(338, 267), 0, true]]:
		hud.set_trade_context(2, target[0], target[1], target[2], "比奇旅人")
		key(KEY_C)
	if not check(trades.is_empty(), "trade rejects side cells, NPCs and dead actors"): return
	hud.set_trade_context(2, Vector2i(338, 267), 1, false, "比奇旅人")
	key(KEY_C)
	click_at(Vector2(644, 484), true)
	click_at(Vector2(644, 484), false)
	if not check(trades == [[1025, "比奇旅人"], [1025, "比奇旅人"]], "C and HUD send original trade request"): return
	var trade_hover := InputEventMouseMotion.new()
	trade_hover.position = Vector2(644, 484)
	Input.parse_input_event(trade_hover)
	await create_timer(0.3).timeout
	if not await capture("hud-trade-entry"): return
	hud.set_character_identity("比奇守卫", "", "", "")
	if not check(hud.open_trade("比奇旅人") and hud.hud_snapshot().trade_visible and hud.hud_snapshot().inventory_visible, "trade reply opens paired panel and inventory"): return
	if not await capture("hud-trade-window"): return
	if not check(hud.set_trade_item(0, 0, 301, 2) and hud.set_trade_item(1, 6, 5, 1), "apply both sides of trade offer"): return
	if not check(not hud.set_trade_item(2, 0, 301, 1) and not hud.set_trade_item(0, 200, 301, 1), "reject invalid trade slot"): return
	if not await capture("hud-trade-offers"): return
	var offers: Array[Array] = []
	hud.trade_offer_requested.connect(func(id: int, item: int, label: String, count: int): offers.append([id, item, label, count]))
	if not check(hud.add_inventory_item(301, 1, hud.hud_snapshot().inventory_row * 6 + 4, 98765, "Trade potion", 20), "create trade pickup fixture"): return
	var trade_pickup: Vector2 = Vector2(hud.hud_snapshot().inventory_position) + Vector2(185, 76)
	click_at(trade_pickup, true)
	click_at(trade_pickup, false)
	if not check(hud.hud_snapshot().held_inventory_icon == 301, "pick up trade item"): return
	var trade_chat_before: int = hud.hud_snapshot().chat_compact_count
	click_at(Vector2(27, 75), true)
	click_at(Vector2(27, 75), false)
	if not check(offers.is_empty() and hud.hud_snapshot().held_inventory_icon == 301 and hud.hud_snapshot().chat_compact_count > trade_chat_before, "occupied trade cell keeps held item and reports rejection"): return
	if not await capture("hud-trade-occupied"): return
	click_at(Vector2(70, 75), true)
	click_at(Vector2(70, 75), false)
	if not check(offers == [[1026, 98765, "Trade potion", 1]] and hud.hud_snapshot().held_inventory_icon == 301, "offer waits for server reply"): return
	key(KEY_Q)
	var trade_status_origin: Vector2 = Vector2(hud.hud_snapshot().status_position)
	click_at(trade_status_origin + Vector2(185, 100), true)
	click_at(trade_status_origin + Vector2(185, 100), false)
	if not check(hud.hud_snapshot().held_inventory_icon == 301 and not hud.hud_snapshot().equipment_pending, "pending trade locks held item across windows"): return
	if not await capture("hud-trade-pending-equipment"): return
	key(KEY_Q)
	if not check(hud.apply_trade_offer_reply(675) and hud.hud_snapshot().held_inventory_icon == -1, "offer reply transfers item into local trade grid"): return
	if not await capture("hud-trade-offer-accepted"): return
	click_at(Vector2(70, 75), true)
	click_at(Vector2(70, 75), false)
	if not check(hud.hud_snapshot().trade_return_slot == 1 and hud.hud_snapshot().held_inventory_icon == -1, "select trade item without moving it into the bag"): return
	var return_preview := InputEventMouseMotion.new()
	return_preview.position = trade_pickup
	Input.parse_input_event(return_preview)
	if not await capture("hud-trade-return-preview"): return
	click_at(trade_pickup, true)
	click_at(trade_pickup, false)
	if not check(offers[-1] == [1027, 98765, "Trade potion", 1], "placing selected offer in bag sends return request"): return
	if not check(hud.apply_trade_offer_reply(677) and not hud.apply_trade_offer_reply(677), "return acknowledgement applies once"): return
	if not await capture("hud-trade-item-returned"): return
	if not check(hud.add_inventory_item(301, 5, hud.hud_snapshot().inventory_row * 6 + 5, 98766, "Trade stack", 0, 3), "add stack for splitting"): return
	click_at(trade_pickup + Vector2(38, 0), true)
	click_at(trade_pickup + Vector2(38, 0), false)
	click_at(Vector2(70, 75), true)
	click_at(Vector2(70, 75), false)
	if not check(hud.hud_snapshot().group_request == 1307, "stack opens quantity dialog"): return
	key(0, 50)
	if not await capture("hud-trade-stack-quantity"): return
	key(KEY_ENTER)
	if not check(offers[-1] == [1026, 98766, "Trade stack", 2] and not hud.apply_trade_offer_reply(675, 3), "send selected quantity and reject excess acknowledgement"): return
	if not check(hud.apply_trade_offer_reply(675, 2) and hud.hud_snapshot().held_inventory_icon == -1, "split offer restores remainder"): return
	if not await capture("hud-trade-stack-split"): return
	click_at(Vector2(70, 75), true)
	click_at(Vector2(70, 75), false)
	click_at(trade_pickup, true)
	click_at(trade_pickup, false)
	if not check(offers[-1] == [1027, 98766, "Trade stack", 2] and hud.apply_trade_offer_reply(677), "return split quantity"): return
	if not await capture("hud-trade-stack-merged"): return
	if not check(hud.set_trade_item(0, 30, 301, 3), "apply trade item below first page"): return
	click_at(Vector2(211, 170), true)
	click_at(Vector2(211, 170), false)
	if not check(hud.hud_snapshot().trade_scroll == Vector2i(6, 0), "trade track scrolls local side only"): return
	if not await capture("hud-trade-scroll"): return
	click_at(Vector2(462, 63), true)
	var trade_drag := InputEventMouseMotion.new()
	trade_drag.position = Vector2(462, 250)
	Input.parse_input_event(trade_drag)
	click_at(trade_drag.position, false)
	if not check(hud.hud_snapshot().trade_scroll == Vector2i(6, 33), "trade drag scrolls remote side independently"): return
	if not check(hud.set_trade_gold(12500, 3420), "apply trade gold totals"): return
	click_at(Vector2(238, 285), true)
	click_at(Vector2(238, 285), false)
	if not check(hud.hud_snapshot().trade_confirmed and trades[-1] == [1030, ""], "confirm sends original trade request"): return
	if not await capture("hud-trade-gold-confirm"): return
	var sent_trades := trades.size()
	click_at(Vector2(238, 285), true)
	click_at(Vector2(238, 285), false)
	if not check(trades.size() == sent_trades and hud.hud_snapshot().trade_confirmed, "confirmed trade does not send twice"): return
	var confirm_hover := InputEventMouseMotion.new()
	confirm_hover.position = Vector2(238, 285)
	Input.parse_input_event(confirm_hover)
	await create_timer(0.3).timeout
	if not await capture("hud-trade-confirmed-hint"): return
	if not check(hud.apply_trade_cancel() and not hud.hud_snapshot().trade_visible and hud.hud_snapshot().inventory_visible, "trade cancellation closes offer and preserves bag"): return
	if not check(not hud.apply_trade_cancel(), "duplicate trade cancellation ignored"): return
	if not await capture("hud-trade-cancelled"): return
	if not check(hud.open_trade("比奇旅人") and hud.set_trade_item(1, 0, 301, 1), "open trade for settlement"): return
	if not check(not hud.apply_trade_complete(19320) and hud.hud_snapshot().trade_visible and hud.hud_snapshot().inventory_gold == 0, "incomplete received item preserves trade and balance"): return
	if not check(hud.set_trade_item(1, 0, 301, 1, 87654, "Received potion", 0, 0), "apply received item identity"): return
	if not check(hud.apply_trade_complete(19320) and not hud.hud_snapshot().trade_visible and hud.hud_snapshot().inventory_gold == 19320, "settle received item and authoritative balance"): return
	if not check(not hud.apply_trade_complete(19320), "duplicate settlement ignored"): return
	if not await capture("hud-trade-completed"): return
	var gold_requests: Array[Vector2i] = []
	hud.trade_gold_requested.connect(func(id: int, amount: int): gold_requests.append(Vector2i(id, amount)))
	if not check(hud.open_trade("比奇旅人"), "open trade for gold input"): return
	click_at(Vector2(81, 288), true)
	click_at(Vector2(81, 288), false)
	if not check(hud.hud_snapshot().group_request == 1029, "open trade gold dialog"): return
	for digit in "12500": key(0, digit.unicode_at(0))
	if not await capture("hud-trade-gold-input"): return
	key(KEY_ENTER)
	if not check(gold_requests == [Vector2i(1029, 12500)] and hud.hud_snapshot().inventory_gold == 19320, "gold offer waits for reply"): return
	if not check(hud.hud_snapshot().trade_gold_pending, "gold offer is pending"): return
	if not check(hud.apply_trade_gold_reply(686, 3420) and hud.hud_snapshot().trade_gold_pending and hud.hud_snapshot().inventory_gold == 19320, "partner gold does not acknowledge local offer"): return
	if not check(hud.apply_trade_gold_reply(684, 12500, 6820, 0) and not hud.hud_snapshot().trade_gold_pending, "local gold reply clears pending offer"): return
	var gold_snapshot: Dictionary = hud.hud_snapshot()
	if not check(gold_snapshot.trade_gold_local == 12500 and gold_snapshot.trade_gold_remote == 3420 and gold_snapshot.inventory_gold == 6820, "gold reply updates both visible totals"): return
	if not await capture("hud-trade-gold-reply"): return
	if not check(not hud.apply_trade_cancel() and hud.hud_snapshot().trade_visible and hud.hud_snapshot().inventory_gold == 6820, "await reconciled balance after gold cancellation"): return
	if not check(hud.apply_trade_cancel(19320) and not hud.hud_snapshot().trade_visible and hud.hud_snapshot().inventory_gold == 19320, "cancel restores authoritative balance"): return
	if not await capture("hud-trade-gold-refund"): return
	var exit_requests: Array[int] = []
	hud.exit_confirmed.disconnect(hud.handle_exit_confirmation)
	hud.exit_confirmed.connect(func(id: int): exit_requests.append(id))
	key(KEY_Q, 0, true)
	if not check(hud.hud_snapshot().group_request == 109, "Alt+Q opens program exit confirmation"): return
	if not await capture("hud-exit-program"): return
	key(KEY_TAB)
	key(KEY_SPACE)
	if not check(hud.hud_snapshot().group_request == 0 and exit_requests.is_empty(), "cancel program exit"): return
	key(KEY_X, 0, true)
	if not check(hud.hud_snapshot().group_request == 101, "Alt+X opens game exit confirmation"): return
	if not await capture("hud-exit-game"): return
	key(KEY_ENTER)
	if not check(exit_requests == [101] and hud.hud_snapshot().group_request == 0, "confirm game exit"): return
	for point in [Vector2(15, 550), Vector2(115, 550)]:
		click_at(point, true)
		click_at(point, false)
		if not check(hud.hud_snapshot().group_request in [101, 109], "HUD opens exit confirmation"): return
		key(KEY_ENTER)
	if not check(exit_requests == [101, 109, 101], "HUD exit requests"): return
	print("HUD_SMOKE_OK: resources, expansion, collapse, stat switching, action dispatch")
	hud.exit_confirmed.connect(hud.handle_exit_confirmation)
	key(KEY_Q, 0, true)
	if not check(hud.hud_snapshot().group_request == 109, "reopen program exit"): return
	if not await capture("hud-program-exit-confirm"): return
	key(KEY_ENTER)
	await create_timer(0.5).timeout
	check(false, "confirmed program exit must stop the scene tree")
