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

func wait_for_npc_cooldown() -> void:
	var deadline := Time.get_ticks_msec() + 1010
	while Time.get_ticks_msec() < deadline:
		await process_frame

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
	hud.set_meta("show_terrain", OS.get_cmdline_user_args().has("--terrain"))
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
	if OS.get_cmdline_user_args().has("--terrain"):
		if not check(hud.hud_snapshot().terrain_ready, "Bichon terrain loaded"): return
		hud.set_meta("show_map_objects", false)
		hud.queue_redraw()
		if not await capture("bichon-terrain-hud"): return
		var ground: Image
		if OS.get_cmdline_user_args().has("--screenshots"):
			ground = root.get_texture().get_image()
		hud.set_meta("show_map_objects", true)
		hud.queue_redraw()
		if not await capture("bichon-map-objects"): return
		if ground != null:
			var scene := root.get_texture().get_image()
			if not check(ground.get_region(Rect2i(0, 0, 800, 450)).get_data() != scene.get_region(Rect2i(0, 0, 800, 450)).get_data(), "original map objects change scene pixels"): return
		if OS.get_cmdline_user_args().has("--screenshots"):
			var frame := root.get_texture().get_image()
			if not check(frame.get_pixel(100, 100) != frame.get_pixel(200, 200), "Bichon terrain pixels"): return
		for destination in [Vector2(390, 240), Vector2(650, 240)]:
			var step := InputEventMouseButton.new()
			step.position = destination
			step.button_index = MOUSE_BUTTON_LEFT
			step.pressed = true
			root.push_input(step, true)
			step = step.duplicate()
			step.pressed = false
			root.push_input(step, true)
			if not check(hud.hud_snapshot().walk_frame == (-1 if destination.x == 390 else 0), "current cell and distant click heading"): return
		if not await capture("bichon-walking"): return
		var footstep: AudioStreamPlayer = hud.get_node("FootstepSound")
		var heard_step := footstep.playing
		var walk_deadline := Time.get_ticks_msec() + 2000
		while hud.hud_snapshot().walk_frame >= 0 and Time.get_ticks_msec() < walk_deadline:
			await process_frame
			heard_step = heard_step or footstep.playing
		if not check(footstep.stream != null and heard_step, "walking plays original footstep sample"): return
		if not check(hud.hud_snapshot().walk_frame == -1 and hud.hud_snapshot().player_cell == Vector2i(476, 360), "walk finishes in destination cell"): return
		var footstep_bus := AudioServer.get_bus_index(footstep.bus)
		var footstep_panner := AudioServer.get_bus_effect(footstep_bus, 0) as AudioEffectPanner
		key(KEY_N)
		click_at(Vector2(602, 212), true)
		var volume_motion := InputEventMouseMotion.new()
		volume_motion.position = Vector2(523, 212)
		root.push_input(volume_motion, true)
		if not check(is_equal_approx(AudioServer.get_bus_volume_db(footstep_bus), -20.0),
			"footstep volume follows effect slider before release"): return
		if not await capture("bichon-footstep-volume-drag"): return
		volume_motion = volume_motion.duplicate()
		volume_motion.position = Vector2(613, 212)
		root.push_input(volume_motion, true)
		click_at(volume_motion.position, false)
		key(KEY_N)
		hud.set_meta("map_camera_cell", Vector2i(470, 353))
		if not check(hud.walk_to_map_cell(Vector2i(476, 361)), "step with an off-center camera"): return
		await create_timer(0.25).timeout
		var normal_pan := footstep_panner.pan
		if not check(absf(normal_pan) > 0.01, "off-center footsteps have a stereo position"): return
		await create_timer(0.45).timeout
		key(KEY_N)
		click_at(Vector2(580, 240), false)
		key(KEY_N)
		if not check(hud.walk_to_map_cell(Vector2i(476, 360)), "step with reversed stereo"): return
		await create_timer(0.25).timeout
		if not check(is_equal_approx(footstep_panner.pan, -normal_pan), "reverse pan flips footstep stereo position"): return
		if not await capture("bichon-reversed-footsteps"): return
		await create_timer(0.45).timeout
		key(KEY_N)
		click_at(Vector2(580, 240), false)
		key(KEY_N)
		hud.remove_meta("map_camera_cell")
		if not await capture("bichon-step-camera"): return
		if not check(hud.walk_to_map_cell(Vector2i(476, 362)), "plan route across Bichon cells"): return
		var route_deadline := Time.get_ticks_msec() + 3000
		while Time.get_ticks_msec() < route_deadline:
			await process_frame
			if hud.hud_snapshot().player_cell == Vector2i(476, 362) and hud.hud_snapshot().walk_frame == -1: break
		if not check(hud.hud_snapshot().player_cell == Vector2i(476, 362), "route reaches destination"): return
		if not await capture("bichon-route-arrival"): return
		key(KEY_N)
		click_at(Vector2(580, 180), false)
		if not check(not footstep.playing, "turning effects off stops the footstep voice"): return
		key(KEY_N)
		if not check(hud.walk_to_map_cell(Vector2i(476, 361)), "walk with effects disabled"): return
		var muted_deadline := Time.get_ticks_msec() + 2000
		while Time.get_ticks_msec() < muted_deadline:
			await process_frame
			if not check(not footstep.playing, "muted movement never plays footsteps"): return
			if hud.hud_snapshot().player_cell == Vector2i(476, 361) and hud.hud_snapshot().walk_frame == -1: break
		if not check(hud.hud_snapshot().player_cell == Vector2i(476, 361) and hud.hud_snapshot().walk_frame == -1,
			"muted step reaches its destination"): return
		key(KEY_N)
		if not await capture("bichon-footsteps-muted"): return
		click_at(Vector2(580, 180), false)
		key(KEY_N)
		if not check(hud.walk_to_map_cell(Vector2i(476, 362)), "walk after re-enabling effects"): return
		var resumed_deadline := Time.get_ticks_msec() + 2000
		var resumed_sound := false
		while Time.get_ticks_msec() < resumed_deadline:
			await process_frame
			resumed_sound = resumed_sound or footstep.playing
			if hud.hud_snapshot().player_cell == Vector2i(476, 362) and hud.hud_snapshot().walk_frame == -1: break
		if not check(resumed_sound and hud.hud_snapshot().player_cell == Vector2i(476, 362) and hud.hud_snapshot().walk_frame == -1,
			"re-enabled effects restore footsteps"): return
		var held_walk := InputEventMouseButton.new()
		held_walk.position = Vector2(390, 100)
		held_walk.button_index = MOUSE_BUTTON_LEFT
		held_walk.pressed = true
		root.push_input(held_walk, true)
		var held_deadline := Time.get_ticks_msec() + 3000
		while hud.hud_snapshot().player_cell.y > 360 and Time.get_ticks_msec() < held_deadline:
			await process_frame
		held_walk = held_walk.duplicate()
		held_walk.pressed = false
		root.push_input(held_walk, true)
		if not check(hud.hud_snapshot().player_cell == Vector2i(476, 360), "held pointer repeats walking"): return
		if not await capture("bichon-held-walk"): return
		await create_timer(0.8).timeout
		if not check(hud.hud_snapshot().player_cell == Vector2i(476, 360) and hud.hud_snapshot().walk_frame == -1, "release ends repeated walking"): return
		if not check(hud.walk_to_map_cell(Vector2i(475, 360)), "approach blocked western tile"): return
		var approach_deadline := Time.get_ticks_msec() + 2000
		while Time.get_ticks_msec() < approach_deadline:
			await process_frame
			if hud.hud_snapshot().player_cell == Vector2i(475, 360) and hud.hud_snapshot().walk_frame == -1: break
		if not check(hud.hud_snapshot().player_cell == Vector2i(475, 360) and hud.hud_snapshot().walk_frame == -1, "arrive at obstacle fixture"): return
		var blocked_step := InputEventMouseButton.new()
		blocked_step.position = Vector2(350, 240)
		blocked_step.button_index = MOUSE_BUTTON_LEFT
		blocked_step.pressed = true
		root.push_input(blocked_step, true)
		blocked_step = blocked_step.duplicate()
		blocked_step.pressed = false
		root.push_input(blocked_step, true)
		var detour_deadline := Time.get_ticks_msec() + 2000
		while hud.hud_snapshot().walk_frame >= 0 and Time.get_ticks_msec() < detour_deadline:
			await process_frame
		if not check(hud.hud_snapshot().player_cell == Vector2i(475, 361) and hud.hud_snapshot().walk_frame == -1, "blocked heading takes last equally close traversable neighbor"): return
		if not await capture("bichon-blocked-heading"): return
		hud.set_character_vitals(30, 30, 20, 20, 0, 0)
		var run := InputEventMouseButton.new()
		run.position = Vector2(390, 100)
		run.button_index = MOUSE_BUTTON_RIGHT
		run.pressed = true
		root.push_input(run, true)
		if not check(hud.hud_snapshot().walk_frame == 0 and not hud.hud_snapshot().map_running, "right press starts with a walk"): return
		var run_deadline := Time.get_ticks_msec() + 2500
		while not hud.hud_snapshot().map_running and Time.get_ticks_msec() < run_deadline:
			await process_frame
		if not check(hud.hud_snapshot().map_running, "held right button starts running after first walk"): return
		run = run.duplicate()
		run.pressed = false
		root.push_input(run, true)
		var run_start: Vector2i = hud.hud_snapshot().player_cell
		var camera_y := [10, 22, 32, 42, 54, 64]
		var actor_y := [10, 24, 34, 44, 54, 64]
		var seen_frames := {}
		var finish_deadline := Time.get_ticks_msec() + 2000
		while Time.get_ticks_msec() < finish_deadline:
			var state: Dictionary = hud.hud_snapshot()
			var index: int = state.walk_frame
			if index < 0: break
			var first_visit := not seen_frames.has(index)
			seen_frames[index] = true
			var origin_pixel := run_start * Vector2i(48, 32)
			if not check(state.map_running and state.map_camera_pixel == origin_pixel - Vector2i(376, 227 + camera_y[index]), "run camera follows frame offsets %s %s %s" % [state.map_camera_pixel, state.player_cell, index]): return
			if not check(state.map_player_pixel == origin_pixel - Vector2i(0, actor_y[index]), "run actor follows frame offsets"): return
			if index == 0 and first_visit and not await capture("bichon-right-run"): return
			await process_frame
		if not check(seen_frames.size() == 6 and hud.hud_snapshot().player_cell == run_start + Vector2i(0, -2), "run renders six frames and release stops after two cells"): return
		var modal_walk := held_walk.duplicate() as InputEventMouseButton
		modal_walk.pressed = true
		root.push_input(modal_walk, true)
		if not check(hud.hud_snapshot().walk_frame == 0, "start held walk before modal"): return
		key(KEY_Q, 0, true)
		if not check(hud.hud_snapshot().group_request == 109, "open exit confirmation during held walk"): return
		if not await capture("bichon-exit-during-walk"): return
		await create_timer(0.85).timeout
		var modal_stop_cell: Vector2i = hud.hud_snapshot().player_cell
		if not check(hud.hud_snapshot().walk_frame == -1, "current step finishes behind modal"): return
		key(KEY_TAB)
		key(KEY_ENTER)
		await create_timer(0.75).timeout
		if not check(hud.hud_snapshot().group_request == 0 and hud.hud_snapshot().player_cell == modal_stop_cell
			and hud.hud_snapshot().walk_frame == -1, "closing modal does not resume held walk"): return
		modal_walk = modal_walk.duplicate()
		modal_walk.pressed = false
		root.push_input(modal_walk, true)
		var chat_walk := modal_walk.duplicate() as InputEventMouseButton
		chat_walk.pressed = true
		root.push_input(chat_walk, true)
		if not check(hud.hud_snapshot().walk_frame == 0, "start held walk before chat"): return
		key(KEY_ENTER)
		var terrain_chat: LineEdit = hud.get_node("ChatInput")
		if not check(terrain_chat.has_focus(), "open chat during held walk"): return
		key(0, 65)
		if not await capture("bichon-chat-during-walk"): return
		await create_timer(0.85).timeout
		var chat_stop_cell: Vector2i = hud.hud_snapshot().player_cell
		if not check(hud.hud_snapshot().walk_frame == -1, "current step finishes while chat is focused"): return
		key(KEY_ESCAPE)
		await create_timer(0.75).timeout
		if not check(hud.hud_snapshot().player_cell == chat_stop_cell and hud.hud_snapshot().walk_frame == -1,
			"closing chat does not resume held walk"): return
		chat_walk = chat_walk.duplicate()
		chat_walk.pressed = false
		root.push_input(chat_walk, true)
		var npc_walk := chat_walk.duplicate() as InputEventMouseButton
		npc_walk.pressed = true
		root.push_input(npc_walk, true)
		if not check(hud.hud_snapshot().walk_frame == 0, "start held walk before NPC dialogue"): return
		if not check(hud.apply_npc_dialogue(643, 2, "老黄/<购买药品/@buy>"), "open NPC dialogue during held walk"): return
		if not await capture("bichon-npc-during-walk"): return
		await create_timer(0.85).timeout
		var npc_stop_cell: Vector2i = hud.hud_snapshot().player_cell
		if not check(hud.hud_snapshot().walk_frame == -1, "current step finishes while NPC dialogue is open"): return
		hud.apply_npc_dialogue(644, 2, "")
		await create_timer(0.75).timeout
		if not check(hud.hud_snapshot().player_cell == npc_stop_cell and hud.hud_snapshot().walk_frame == -1,
			"closing NPC dialogue does not resume held walk"): return
		npc_walk = npc_walk.duplicate()
		npc_walk.pressed = false
		root.push_input(npc_walk, true)
		hud.set_meta("map_camera_cell", Vector2i(404, 402))
		hud.queue_redraw()
		if not await capture("bichon-merchant"): return
		var distant_requests: Array[int] = []
		hud.npc_dialogue_requested.connect(func(actor: int): distant_requests.append(actor))
		await process_frame
		var distant_cell: Vector2i = hud.hud_snapshot().player_cell
		click_at(Vector2(457, 215), true)
		click_at(Vector2(457, 215), false)
		if not check(distant_requests.is_empty() and hud.hud_snapshot().player_cell == distant_cell
			and hud.hud_snapshot().walk_frame == -1, "distant merchant click neither sends a request nor walks"): return
		if not await capture("bichon-merchant-out-of-range"): return
		var merchant_hud: Node = load("res://main.tscn").instantiate()
		merchant_hud.set_meta("options_path", options_file)
		merchant_hud.set_meta("map_spawn_cell", Vector2i(413, 410))
		merchant_hud.set_meta("map_camera_cell", Vector2i(404, 402))
		root.remove_child(hud)
		hud.queue_free()
		hud = merchant_hud
		root.add_child(hud)
		var merchant_requests: Array[int] = []
		var npc_protocol: Array = []
		var npc_packets: Array[String] = []
		hud.npc_packet_ready.connect(func(packet: PackedByteArray): npc_packets.append(packet.get_string_from_ascii()))
		hud.npc_request.connect(func(opcode: int, actor: int, body: String): npc_protocol.append([opcode, actor, body]))
		hud.npc_dialogue_requested.connect(func(actor: int): merchant_requests.append(actor))
		await process_frame
		if not DisplayServer.get_name() == "headless":
			await RenderingServer.frame_post_draw
		for i in 2:
			var talk := InputEventMouseButton.new()
			talk.position = Vector2(457, 215)
			talk.button_index = MOUSE_BUTTON_LEFT
			talk.pressed = true
			root.push_input(talk, true)
			talk = talk.duplicate()
			talk.pressed = false
			root.push_input(talk, true)
		if not check(merchant_requests == [2] and hud.hud_snapshot().walk_frame == -1, "merchant click requests dialogue once without moving"): return
		if not await capture("bichon-merchant-interaction"): return
		var dialogue_actions: Array = []
		hud.npc_dialogue_action.connect(func(actor: int, command: String): dialogue_actions.append([actor, command]))
		var script := "老黄/要买点什么？" + char(92) + "<购买药品/@buy>" + char(92) + "<离开/@exit>"
		if not check(hud.apply_npc_message("`\\<<<>=B<?H<<<<<lHzwm^{NfhGniZKFhhJpdwyXKGbvlkGNfXVsGp=^YS`zS?o<vwzfGp=aZBapK\\".to_ascii_buffer(), PackedByteArray([17, 34, 51, 68])), "decode merchant dialogue reply"): return
		if not await capture("bichon-merchant-dialogue"): return
		var close_hover := InputEventMouseMotion.new()
		close_hover.position = Vector2(350, 186)
		root.push_input(close_hover, true)
		for frame in range(8):
			await process_frame
		if not await capture("bichon-merchant-close-hint"): return
		if OS.get_cmdline_user_args().has("--screenshots"):
			var close_hint_image := root.get_texture().get_image()
			var hint_visible := false
			for y in range(200, 230):
				for x in range(375, 440):
					hint_visible = hint_visible or close_hint_image.get_pixel(x, y).is_equal_approx(Color8(255, 255, 150))
			if not check(hint_visible, "NPC close tooltip background"): return
		click_at(Vector2(350, 186), true)
		click_at(Vector2(350, 186), false)
		if not check(hud.hud_snapshot().npc_dialogue_actor == 0, "NPC footer button closes locally"): return
		var locked_script := "老黄/{NOTCLOSE/1}{FCOLOR/10}请先选择一项" + char(92) + "<购买药品/@buy>"
		if not check(hud.apply_npc_dialogue(643, 2, locked_script), "open NPC dialogue with script tags"): return
		if not await capture("bichon-merchant-notclose"): return
		for reply in [locked_script, script]:
			if not check(hud.apply_npc_dialogue(643, 2, reply), "update open NPC dialogue"): return
			click_at(Vector2(350, 186), true)
			click_at(Vector2(350, 186), false)
			if not check(hud.hud_snapshot().npc_dialogue_actor == 2, "NOTCLOSE persists while the dialogue stays open"): return
		if not check(hud.apply_npc_dialogue(644, 2, "") and hud.apply_npc_dialogue(643, 2, script), "reopen after server dismissal"): return
		click_at(Vector2(350, 186), true)
		click_at(Vector2(350, 186), false)
		if not check(hud.hud_snapshot().npc_dialogue_actor == 0, "server dismissal resets NOTCLOSE"): return
		var long_script := script + char(92).repeat(8) + "比".repeat(2800)
		if not check(hud.apply_npc_dialogue(643, 2, long_script), "dialogue limit counts ANSI bytes"): return
		if not check(not hud.apply_npc_dialogue(643, 2, long_script + "比".repeat(1400)), "reject dialogue above ANSI byte limit"): return
		if not check(hud.apply_npc_dialogue(643, 2, script), "restore short NPC dialogue"): return
		var option_before: Image
		if OS.get_cmdline_user_args().has("--screenshots"):
			option_before = root.get_texture().get_image().get_region(Rect2i(35, 57, 48, 24))
		var option_hover := InputEventMouseMotion.new()
		option_hover.position = Vector2(45, 63)
		root.push_input(option_hover, true)
		if not await capture("bichon-merchant-option-hover"): return
		if option_before != null:
			var focused := root.get_texture().get_image().get_region(Rect2i(35, 57, 48, 24))
			if not check(option_before.get_data() != focused.get_data(), "focused NPC link changes GDI glyph pixels"): return
		var option := InputEventMouseButton.new()
		option.position = Vector2(45, 63)
		option.button_index = MOUSE_BUTTON_LEFT
		option.pressed = true
		root.push_input(option, true)
		option = option.duplicate()
		option.pressed = false
		root.push_input(option, true)
		if not check(dialogue_actions == [[2, "@buy"]], "merchant dialogue option emits command"): return
		if not check(npc_protocol == [[1010, 2, ""], [1011, 2, "@buy"]], "NPC click and choice use reference protocol fields"): return
		if not check(npc_packets == ["#1<\\<<<KD?<<<<<<<<!", "#2<\\<<<KH?<<<<<<<<LBEqZL!"], "NPC requests match encoded header and body"): return
		if not check(hud.apply_npc_dialogue(643, 2, "Merchant/AA <Buy/@buy> <Exit/@exit>"), "inline NPC options"): return
		if not await capture("bichon-merchant-inline"): return
		click_at(Vector2(78, 45), false)
		if not check(dialogue_actions.size() == 1, "NPC cooldown survives a replacement reply"): return
		await wait_for_npc_cooldown()
		click_at(Vector2(39, 45), true)
		click_at(Vector2(39, 45), false)
		click_at(Vector2(58, 45), true)
		click_at(Vector2(58, 45), false)
		await wait_for_npc_cooldown()
		click_at(Vector2(39, 45), true)
		click_at(Vector2(78, 45), false)
		if not check(dialogue_actions == [[2, "@buy"], [2, "@buy"], [2, "@exit"]], "only inline labels dispatch their own commands"): return
		var tagged_command := "@route" + char(92) + "药店"
		var tagged_script := "老黄/{FCOLOR/10}请选择 <前往/" + tagged_command + "> {FCOLOR/12}药店"
		if not check(hud.apply_npc_dialogue(643, 2, tagged_script), "parse script delimiter inside option"): return
		if not await capture("bichon-merchant-script-tags"): return
		await wait_for_npc_cooldown()
		click_at(Vector2(82, 45), true)
		click_at(Vector2(82, 45), false)
		if not check(dialogue_actions.back() == [2, tagged_command], "line delimiter inside option stays in command"): return
		if not check(hud.apply_npc_dialogue(643, 2, "Merchant/<" + "比".repeat(30) + "/@wrapped>"), "wrap long NPC line"): return
		if not await capture("bichon-merchant-wrapped-line"): return
		await wait_for_npc_cooldown()
		click_at(Vector2(48, 45), true)
		click_at(Vector2(48, 63), false)
		if not check(dialogue_actions.back() == [2, "@wrapped"], "wrapped option shares press and release across rows"): return
		var separator := char(92)
		var scroll_script := "Merchant/<第一页/@first>" + separator + "<下一行/@second>"
		for i in range(2, 26):
			scroll_script += separator + ("药店说明 %02d" % i if i < 25 else "<最后一项/@last>")
		if not check(hud.apply_npc_dialogue(643, 2, scroll_script), "open scrollable NPC dialogue"): return
		if OS.get_cmdline_user_args().has("--screenshots"):
			if not await capture("bichon-merchant-dialogue-first-row"): return
			var up_rect := Rect2i(183, 416, 21, 18)
			var up_idle := root.get_texture().get_image().get_region(up_rect).get_data()
			click_at(Vector2(188, 420), true)
			await process_frame
			await RenderingServer.frame_post_draw
			var up_at_start := root.get_texture().get_image().get_region(up_rect).get_data()
			click_at(Vector2(188, 420), false)
			if not check(up_idle == up_at_start, "NPC up arrow stays idle at first row"): return
		var portrait_names: Array[String] = []
		for index in range(14): portrait_names.append("portrait-%02d.jpg" % index)
		hud.set_portrait_names(portrait_names)
		click_at(Vector2(540, 570), true)
		click_at(Vector2(540, 570), false)
		if not check(hud.hud_snapshot().portrait_visible, "portrait picker overlaps NPC dialogue"): return
		var npc_wheel := InputEventMouseButton.new()
		npc_wheel.position = Vector2(50, 45)
		npc_wheel.button_index = MOUSE_BUTTON_WHEEL_DOWN
		npc_wheel.pressed = true
		root.push_input(npc_wheel, true)
		if not check(hud.hud_snapshot().portrait_first == 0, "NPC wheel does not scroll the covered portrait picker"): return
		if not await capture("bichon-merchant-over-portrait"): return
		click_at(Vector2(544, 369), true)
		click_at(Vector2(544, 369), false)
		if not check(not hud.hud_snapshot().portrait_visible, "close the portrait picker outside the NPC dialogue"): return
		if OS.get_cmdline_user_args().has("--screenshots"):
			await RenderingServer.frame_post_draw
			var arrow_rect := Rect2i(183, 434, 21, 18)
			var arrow_before := root.get_texture().get_image().get_region(arrow_rect).get_data()
			for frame in range(16):
				await process_frame
			await RenderingServer.frame_post_draw
			var arrow_after := root.get_texture().get_image().get_region(arrow_rect).get_data()
			if not check(arrow_before != arrow_after, "NPC down arrow blinks while more rows are available"): return
			if not await capture("bichon-merchant-dialogue-arrow"): return
		if not await capture("bichon-merchant-dialogue-scrolled"): return
		await wait_for_npc_cooldown()
		click_at(Vector2(50, 45), true)
		click_at(Vector2(50, 45), false)
		if not check(dialogue_actions.back() == [2, "@second"], "NPC wheel moves option hitboxes"): return
		click_at(Vector2(188, 420), true)
		click_at(Vector2(188, 420), false)
		await wait_for_npc_cooldown()
		click_at(Vector2(50, 45), true)
		click_at(Vector2(50, 45), false)
		if not check(dialogue_actions.back() == [2, "@first"], "NPC up arrow restores first option"): return
		for i in range(8):
			root.push_input(npc_wheel, true)
		if not await capture("bichon-merchant-dialogue-last-row"): return
		await wait_for_npc_cooldown()
		click_at(Vector2(50, 402), true)
		click_at(Vector2(50, 402), false)
		if not check(dialogue_actions.back() == [2, "@last"], "NPC scroll clamps with the last option on the final visible row"): return
		if not check(hud.apply_npc_dialogue(643, 2, "老黄/<请输入名字/@@name>"), "open NPC script with text input option"): return
		await wait_for_npc_cooldown()
		var actions_before_prompt := dialogue_actions.size()
		click_at(Vector2(50, 45), false)
		if not await capture("bichon-merchant-text-prompt"): return
		if not check(dialogue_actions.size() == actions_before_prompt, "NPC prompt waits for an answer"): return
		var npc_input: LineEdit = hud.get_node("ChatInput")
		npc_input.text = "比奇"
		key(KEY_ENTER)
		if not check(dialogue_actions.size() == actions_before_prompt + 1 and dialogue_actions.back() == [2, "@@name\r比奇"], "NPC prompt joins command and answer with CR"): return
		await wait_for_npc_cooldown()
		var secret_command := "@@*4_enter_stgpasswd"
		if not check(hud.apply_npc_dialogue(643, 2, "老黄/<请输入仓库密码/" + secret_command + ">"), "open masked NPC prompt"): return
		click_at(Vector2(50, 45), false)
		if not check(npc_input.secret and npc_input.secret_character == "*", "NPC password input uses asterisk mask"): return
		npc_input.text = "123456"
		npc_input.caret_column = 6
		npc_input.text_changed.emit(npc_input.text)
		if not check(npc_input.text == "1234", "NPC password limit applies during editing"): return
		if not await capture("bichon-merchant-masked-prompt"): return
		key(KEY_ENTER)
		if not check(not npc_input.secret and dialogue_actions.back() == [2, secret_command + "\r1234"], "NPC password byte limit and mask reset"): return
		var confirmed_actions := dialogue_actions.size()
		for server_close in [false, true]:
			await wait_for_npc_cooldown()
			if not check(hud.apply_npc_dialogue(643, 2, "老黄/<请输入仓库密码/" + secret_command + ">"), "reopen masked NPC prompt"): return
			click_at(Vector2(50, 45), false)
			if not check(npc_input.secret, "masked prompt opened again"): return
			npc_input.text = "9999"
			if server_close:
				if not check(hud.apply_npc_dialogue(644, 2, ""), "server closes pending NPC prompt"): return
			else:
				key(KEY_ESCAPE)
				if not check(npc_input.secret and npc_input.text == "9999" and npc_input.has_focus(), "NPC prompt ignores Escape"): return
				if not await capture("bichon-merchant-masked-prompt-after-escape"): return
				click_at(Vector2(490, 316), true)
				click_at(Vector2(490, 316), false)
			if not check(not npc_input.secret and npc_input.text.is_empty() and not npc_input.has_focus() and dialogue_actions.size() == confirmed_actions, "dismissal clears masked input without sending"): return
			if not server_close and not await capture("bichon-merchant-masked-prompt-cancelled"): return
		if not check(hud.apply_npc_dialogue(643, 2, script), "reopen NPC dialogue for shop reply"): return
		var goods_requests: Array = []
		hud.shop_good_requested.connect(func(actor: int, index: int, name: String): goods_requests.append([actor, index, name]))
		if not check(hud.apply_npc_message("a<D<<>=B<?H<<<<<PbQ`DC=kYBakW^xmGoDlGoPkHo<mGo<kH>y>WCQaDC=kYBakW^xlGoDqGoLkHo<nGo<kH>x".to_ascii_buffer(), PackedByteArray([17, 34, 51, 68])), "645 goods list with original icon fields"): return
		if not check(not hud.apply_shop_goods(2, 1, "Broken/1/"), "reject truncated goods list"): return
		if not await capture("bichon-merchant-goods"): return
		click_at(Vector2(45, 285), true)
		click_at(Vector2(45, 285), false)
		click_at(Vector2(135, 420), true)
		click_at(Vector2(135, 420), false)
		if not check(goods_requests == [[2, 1, "Blue potion"]], "mode zero goods also request details"): return
		if not await capture("bichon-merchant-mode-zero-request"): return
		goods_requests.clear()
		click_at(Vector2(45, 265), true)
		click_at(Vector2(45, 265), false)
		if not check(goods_requests.is_empty(), "selecting a category does not request details"): return
		click_at(Vector2(135, 420), true)
		if not await capture("bichon-merchant-category-confirm"): return
		click_at(Vector2(135, 420), false)
		if not check(goods_requests == [[2, 0, "Red potion"]], "valid goods selection survives invalid reply"): return
		if not check(not hud.apply_npc_body(645, 2, 2, "!!!!".to_ascii_buffer()), "reject invalid six-bit alphabet"): return
		if not check(hud.apply_npc_body(645, 2, 1, "PrieWBl\\TbykVnxlGoDqGoLkHo<pGo<kHNx".to_ascii_buffer()), "decode shop profession requirement"): return
		if not await capture("bichon-merchant-profession-requirement"): return
		if not check(hud.apply_npc_body(645, 2, 1, "QCE]VRueWbX\\VSMaWNxmGoDqGoLkHo<mGo<kHO<k".to_ascii_buffer()), "decode shop level requirement"): return
		if not await capture("bichon-merchant-level-requirement-unmet"): return
		if not check(hud.set_character_progress(10, 0, 0, 0, 0), "reach required shop level"): return
		if not await capture("bichon-merchant-level-requirement-met"): return
		if not check(hud.set_character_progress(1, 0, 0, 0, 0), "restore level after shop requirement check"): return
		if not check(hud.apply_npc_body(645, 2, 1, "MRmaWRQjY>=eYBQiGo@kH_PkI>xoH?@kH^xoGl".to_ascii_buffer()), "decode grouped element requirement"): return
		var empty_elements := PackedByteArray([0, 0, 0, 0, 0, 0, 0])
		if not check(hud.set_character_elements(PackedByteArray([3, 3, 3, 2, 0, 0, 0]), empty_elements, empty_elements), "set one element below requirement"): return
		if not await capture("bichon-merchant-element-unmet"): return
		if not check(hud.set_character_elements(PackedByteArray([3, 3, 3, 3, 0, 0, 0]), empty_elements, empty_elements), "meet all required elements"): return
		if not await capture("bichon-merchant-element-met"): return
		if not check(hud.set_character_elements(empty_elements, empty_elements, empty_elements), "restore character elements"): return
		var stock_list := ""
		for i in 10: stock_list += "Potion %d/1/20/5/" % i
		if not check(hud.apply_shop_goods(2, 10, stock_list), "shop list exceeds one screen"): return
		click_at(Vector2(265, 325), true)
		click_at(Vector2(265, 325), false)
		if not await capture("bichon-merchant-goods-track-click"): return
		click_at(Vector2(80, 250), true)
		click_at(Vector2(80, 250), false)
		click_at(Vector2(135, 420), true)
		click_at(Vector2(135, 420), false)
		if not check(goods_requests.back() == [2, 5, "Potion 5"], "shop track click advances by a normalized page"): return
		goods_requests.pop_back()
		click_at(Vector2(265, 265), true)
		click_at(Vector2(265, 265), false)
		click_at(Vector2(80, 250), true)
		click_at(Vector2(80, 250), false)
		click_at(Vector2(135, 420), true)
		click_at(Vector2(135, 420), false)
		if not check(goods_requests.back() == [2, 0, "Potion 0"], "opposite track clicks restore the first row"): return
		goods_requests.pop_back()
		if not await capture("bichon-merchant-goods-track-return"): return
		if not check(hud.apply_shop_goods(2, 10, stock_list), "reset shop list before dragging"): return
		click_at(Vector2(265, 261), true)
		var shop_drag := InputEventMouseMotion.new()
		shop_drag.position = Vector2(265, 450)
		root.push_input(shop_drag, true)
		click_at(Vector2(265, 450), false)
		if not await capture("bichon-merchant-goods-dragged"): return
		click_at(Vector2(80, 250), true)
		click_at(Vector2(80, 250), false)
		click_at(Vector2(135, 420), true)
		click_at(Vector2(135, 420), false)
		if not check(goods_requests.back() == [2, 9, "Potion 9"], "dragging shop scrollbar to end selects final row"): return
		goods_requests.pop_back()
		if not check(hud.apply_shop_goods(2, 10, stock_list), "reset shop list before alert interrupts dragging"): return
		click_at(Vector2(265, 261), true)
		if not check(hud.apply_shop_purchase_reply(651, 1), "open purchase alert during shop drag"): return
		click_at(Vector2(265, 450), false)
		key(KEY_ENTER)
		root.push_input(shop_drag, true)
		if not await capture("bichon-merchant-scroll-after-alert"): return
		click_at(Vector2(80, 250), true)
		click_at(Vector2(80, 250), false)
		click_at(Vector2(135, 420), true)
		click_at(Vector2(135, 420), false)
		if not check(goods_requests.back() == [2, 0, "Potion 0"], "dismissing alert does not resume shop drag"): return
		goods_requests.pop_back()
		if not check(hud.apply_shop_goods(2, 10, stock_list), "reset shop list before wheel test"): return
		for i in 6:
			var wheel_down := InputEventMouseButton.new()
			wheel_down.button_index = MOUSE_BUTTON_WHEEL_DOWN
			wheel_down.pressed = true
			wheel_down.position = Vector2(12, 245)
			root.push_input(wheel_down, true)
		if not await capture("bichon-merchant-goods-scrolled"): return
		click_at(Vector2(80, 378), true)
		click_at(Vector2(80, 378), false)
		click_at(Vector2(135, 420), true)
		click_at(Vector2(135, 420), false)
		if not check(goods_requests.back() == [2, 9, "Potion 9"], "wheel over shop frame scrolls goods and preserves their original index"): return
		if not check(npc_protocol.back() == [1015, 2, "Potion 9"], "detail request uses NPC and item name"): return
		var offer_selections: Array = []
		var purchases: Array = []
		hud.shop_purchase_requested.connect(func(opcode: int, actor: int, name: String): purchases.append([opcode, actor, name]))
		hud.shop_offer_selected.connect(func(actor: int, identifier: int): offer_selections.append([actor, identifier]))
		var names := PackedStringArray(["Red potion"])
		var icons := PackedInt32Array([301])
		var prices := PackedInt32Array([20])
		var ids := PackedInt64Array([10001])
		if not check(not hud.apply_shop_offers(2, 8, names, icons, prices, ids), "ignore stale goods details"): return
		var offer_packet := "cL@<<>=@<?H<<<<<PBEMT@M?KRiULbAgQqlxK?lxK?lxK?lxK?lxK?lxK?lxK?lxK?lxK?lxK?lxK?lxMpm<K?lxK?lxK?lxK?lxK?lxK?lxK?lxK?lxK?lxK?lxK?mIMRlxK?lxKA<xK?lxK?lxK?lxK?lxK?lxK?lxK?lxK?lxK?lxK?lxK?lxK?lxK?lxK?lk".to_ascii_buffer()
		var offer_key := PackedByteArray([17, 34, 51, 68])
		if not check(not hud.apply_npc_message(offer_packet.slice(0, -2), offer_key), "reject truncated encoded 652 offer"): return
		if not check(hud.apply_npc_message(offer_packet, offer_key), "decode nested 652 offer from wire packet"): return
		if not await capture("bichon-merchant-offers"): return
		click_at(Vector2(427, 399), true)
		if not await capture("bichon-merchant-offers-next-page"): return
		click_at(Vector2(427, 399), false)
		if not check(npc_packets.back().substr(2, 16) == "<\\<<<KX?>\\<<<<<<", "next offer page requests offset ten"): return
		var page_packet_count := npc_packets.size()
		click_at(Vector2(427, 399), true)
		click_at(Vector2(427, 399), false)
		click_at(Vector2(334, 399), true)
		click_at(Vector2(334, 399), false)
		if not check(npc_packets.size() == page_packet_count, "shop ignores both page arrows while awaiting 652"): return
		click_at(Vector2(378, 399), true)
		click_at(Vector2(378, 399), false)
		if not check(purchases.is_empty() and npc_packets.size() == page_packet_count,
			"confirm cannot purchase a stale offer while loading another page"): return
		click_at(Vector2(330, 280), true)
		click_at(Vector2(330, 280), false)
		if not check(offer_selections.is_empty(), "old offer cells cannot be selected while loading another page"): return
		if not await capture("bichon-merchant-offers-awaiting-page"): return
		var next_offer_packet := "cL@F<>=@<?H<<<<<".to_ascii_buffer() + offer_packet.slice(16)
		if not check(not hud.apply_npc_message(offer_packet, offer_key), "ignore previous page packet while next page is requested"): return
		if not check(hud.apply_npc_message(next_offer_packet, offer_key), "apply encoded next offer page at offset ten"): return
		if not await capture("bichon-merchant-offers-page-reply"): return
		click_at(Vector2(334, 399), true)
		click_at(Vector2(334, 399), false)
		if not check(npc_packets.back().substr(2, 16) == "<\\<<<KX?<<<<<<<<", "previous offer page requests offset zero"): return
		if not check(not hud.apply_npc_message(next_offer_packet, offer_key), "ignore delayed next page when returning to first page"): return
		if not check(hud.apply_npc_message(offer_packet, offer_key), "apply encoded first page reply"): return
		click_at(Vector2(330, 280), true)
		click_at(Vector2(330, 280), false)
		if not check(offer_selections == [[2, 10001]], "select offer by its instance identifier"): return
		if not check(purchases.is_empty(), "clicking a grid cell only selects the offer"): return
		click_at(Vector2(378, 399), true)
		if not await capture("bichon-merchant-offer-confirm"): return
		click_at(Vector2(378, 399), false)
		if not check(purchases == [[1014, 2, "Red potion"]], "purchase uses NPC and item name"): return
		if not check(npc_packets.back().substr(2, 16) == "<\\<<<KT?@NX<<<<<", "purchase header contains selected instance identifier"): return
		var offer_double_click := InputEventMouseButton.new()
		offer_double_click.button_index = MOUSE_BUTTON_LEFT
		offer_double_click.position = Vector2(330, 280)
		offer_double_click.pressed = true
		offer_double_click.double_click = true
		root.push_input(offer_double_click, true)
		click_at(Vector2(330, 280), false)
		if not check(purchases.size() == 2 and npc_packets.back().substr(2, 16) == "<\\<<<KT?@NX<<<<<", "double-click buys exactly once using the clicked instance"): return
		if not await capture("bichon-merchant-offer-double-click"): return
		if not check(hud.apply_npc_message("bsp<<>=B<?H<<<<<".to_ascii_buffer(), PackedByteArray([17, 34, 51, 68])) and hud.hud_snapshot().inventory_gold == 125, "650 applies server gold balance"): return
		if not check(not hud.apply_inventory_add(650, 301, 1, 10001, "Red potion", 0, 3), "gold reply is not an item addition"): return
		if not check(hud.apply_inventory_add(200, 301, 1, 10001, "Red potion", 0, 3), "200 places purchased item in bag"): return
		key(KEY_W)
		if not check(hud.hud_snapshot().inventory_visible, "open bag after purchase"): return
		if not check(not hud.apply_npc_message("bl<<<>=B<?H<<<<<Z<".to_ascii_buffer(), PackedByteArray([17, 34, 51, 68])) and hud.hud_snapshot().inventory_gold == 125, "unexpected body cannot overwrite balance"): return
		if not await capture("bichon-merchant-purchased-item"): return
		var pickup := Vector2(hud.hud_snapshot().inventory_position) + Vector2(30, 75)
		click_at(pickup, true)
		click_at(pickup, false)
		if not check(hud.hud_snapshot().held_inventory_icon == 301, "purchased potion is in first available slot"): return
		# An immediate second release invokes item use instead of putting the item back.
		var put_back_at := Time.get_ticks_msec() + 310
		while Time.get_ticks_msec() < put_back_at:
			await process_frame
		click_at(pickup, true)
		click_at(pickup, false)
		if not check(hud.hud_snapshot().held_inventory_icon != 301, "put purchased item back before continuing shop tests"): return
		key(KEY_W)
		if not check(not hud.apply_npc_message("b\\L<<>=B<?H<<<<<".to_ascii_buffer(), PackedByteArray([17, 34, 51, 68])) and hud.apply_npc_message("b\\D<<>=B<?H<<<<<".to_ascii_buffer(), PackedByteArray([17, 34, 51, 68])), "651 maps known purchase failure"): return
		if not check(hud.hud_snapshot().group_request == 1438 and hud.hud_snapshot().inventory_gold == 125, "failure opens original caption without changing gold"): return
		if not await capture("bichon-merchant-purchase-failed"): return
		key(KEY_ENTER)
		if not check(hud.apply_shop_offers(2, 9, PackedStringArray(), PackedInt32Array(), PackedInt32Array(), PackedInt64Array()), "accept empty detail page"): return
		click_at(Vector2(330, 280), true)
		click_at(Vector2(330, 280), false)
		if not check(goods_requests.size() == 2, "empty details stay in detail view"): return
		click_at(Vector2(465, 398), true)
		if not await capture("bichon-merchant-offer-close-pressed"): return
		click_at(Vector2(465, 398), false)
		if not check(not hud.apply_shop_offers(2, 9, names, icons, prices, ids), "late detail cannot reopen dismissed page"): return
		if not await capture("bichon-merchant-return-to-goods"): return
		click_at(Vector2(80, 378), true)
		click_at(Vector2(80, 378), false)
		click_at(Vector2(135, 420), true)
		click_at(Vector2(135, 420), false)
		if not check(goods_requests.size() == 3 and goods_requests.back() == [2, 9, "Potion 9"], "return restores category scroll"): return
		click_at(Vector2(80, 378), true)
		hud.notification(NOTIFICATION_APPLICATION_FOCUS_OUT)
		click_at(Vector2(80, 378), false)
		if not check(goods_requests.size() == 3, "focus loss cancels pressed goods row"): return
		if not await capture("bichon-merchant-focus-reset"): return
		if not check(hud.apply_shop_offers(2, 9, names, icons, prices, ids), "open details before switching category"): return
		click_at(Vector2(80, 336), true)
		click_at(Vector2(80, 336), false)
		if not check(goods_requests.size() == 3, "selection beside the detail grid sends no request"): return
		click_at(Vector2(135, 420), true)
		click_at(Vector2(135, 420), false)
		if not check(goods_requests.back() == [2, 8, "Potion 8"], "request a different category while details are open"): return
		var purchases_before_switch := purchases.size()
		click_at(Vector2(330, 280), true)
		click_at(Vector2(330, 280), false)
		click_at(Vector2(378, 399), true)
		click_at(Vector2(378, 399), false)
		if not check(purchases.size() == purchases_before_switch, "old instances cannot be purchased under a new category"): return
		if not await capture("bichon-merchant-category-loading"): return
		if not check(not hud.apply_shop_offers(2, 9, names, icons, prices, ids), "reject previous category reply"): return
		if not check(hud.apply_shop_offers(2, 8, names, icons, prices, ids, 0), "accept new category at offset zero"): return
		if not await capture("bichon-merchant-switch-category"): return
		click_at(Vector2(273, 421), true)
		if not await capture("bichon-merchant-close-from-details"): return
		click_at(Vector2(273, 421), false)
		if not check(hud.hud_snapshot().npc_dialogue_actor == 2, "closing shop keeps NPC dialogue"): return
		if not check(not hud.apply_shop_offers(2, 9, names, icons, prices, ids), "closed shop ignores pending details"): return
		if not check(not hud.apply_shop_offers(2, 8, names, icons, prices, ids), "list close dismisses the active detail grid"): return
		if not await capture("bichon-merchant-shop-closed"): return
		if not check(hud.apply_shop_goods(2, 2, "Potion/1/20/5/301/0/0/Bundle/2/20/5/301/0/10/", true),
			"open ordinary and bulk categories together"): return
		click_at(Vector2(135, 420), true)
		click_at(Vector2(135, 420), false)
		if not check(goods_requests.back() == [2, 0, "Potion"], "request details before switching to bulk"): return
		var requests_before_bulk := goods_requests.size()
		offer_double_click.position = Vector2(80, 292)
		root.push_input(offer_double_click, true)
		click_at(Vector2(80, 292), false)
		if not check(goods_requests.size() == requests_before_bulk, "bulk double-click opens a local preview"): return
		if not check(not hud.apply_npc_message(offer_packet, offer_key), "late detail packet cannot replace bulk preview"): return
		if not await capture("bichon-merchant-bulk-preview"): return
		click_at(Vector2(380, 296), true)
		click_at(Vector2(380, 296), false)
		if not check(hud.hud_snapshot().group_request == 110, "bulk purchase opens quantity input"): return
		npc_input.text = "3"
		if not await capture("bichon-merchant-bulk-quantity"): return
		key(KEY_ENTER)
		if not check(npc_packets.back().substr(2, 16) == "<\\<<<KT?=L<<<<H<", "bulk purchase includes item key and quantity"): return
		for position in [Vector2(320, 259), Vector2(395, 259)]:
			var before_bulk_double := purchases.size()
			offer_double_click.position = position
			root.push_input(offer_double_click, true)
			click_at(position, false)
			if not check(hud.hud_snapshot().group_request == 0 and purchases.size() == before_bulk_double + 1 and npc_packets.back().substr(2, 16) == "<\\<<<KT?=L<<<<@<", "bulk preview double-click buys one without a prompt"): return
		if not await capture("bichon-merchant-bulk-double-click"): return
		var before_bulk_cancel := purchases.size()
		click_at(Vector2(380, 296), true)
		click_at(Vector2(380, 296), false)
		npc_input.text = "4"
		click_at(Vector2(490, 316), true)
		click_at(Vector2(490, 316), false)
		if not check(hud.hud_snapshot().group_request == 0 and npc_input.text.is_empty() and purchases.size() == before_bulk_cancel, "cancel bulk quantity without purchase"): return
		click_at(Vector2(380, 296), true)
		click_at(Vector2(380, 296), false)
		if not check(hud.hud_snapshot().group_request == 110, "reopen bulk quantity before server closes shop"): return
		npc_input.text = "5"
		if not check(hud.apply_npc_message("aL<<<>=B<?H<<<<<".to_ascii_buffer(), PackedByteArray([17, 34, 51, 68])) and hud.hud_snapshot().npc_dialogue_actor == 0, "merchant close reply dismisses dialogue"): return
		if not check(hud.hud_snapshot().group_request == 0 and npc_input.text.is_empty() and not npc_input.has_focus() and purchases.size() == before_bulk_cancel, "server close clears pending bulk quantity"): return
		if not await capture("bichon-merchant-bulk-server-closed"): return
		if not check(not hud.apply_shop_goods(2, 10, stock_list), "late goods reply cannot reopen closed merchant"): return
		if not check(hud.apply_npc_dialogue(643, 2, script) and hud.apply_shop_goods(2, 10, stock_list),
			"reopen merchant dialogue and shop before walking away"): return
		if not await capture("bichon-merchant-before-walk-away"): return
		if not check(hud.walk_to_map_cell(Vector2i(419, 410)), "walk away from open merchant shop"): return
		var leave_deadline := Time.get_ticks_msec() + 6000
		while hud.hud_snapshot().player_cell.x < 419 and Time.get_ticks_msec() < leave_deadline:
			await process_frame
			if hud.hud_snapshot().player_cell.x <= 418:
				if not check(hud.hud_snapshot().npc_dialogue_actor == 2, "dialogue stays visible within five cells"): return
		if not check(hud.hud_snapshot().player_cell == Vector2i(419, 410) and hud.hud_snapshot().npc_dialogue_actor == 0, "dialogue closes on the sixth cell"): return
		if not check(not hud.apply_shop_goods(2, 10, stock_list), "shop refresh cannot reopen the merchant after walking away"): return
		if not await capture("bichon-merchant-walk-away"): return
		quit()
		return
	for i in 20:
		hud.post_chat("[测试 %02d] 比奇省：聊天文字与滚动验证。" % i, 0xffffff if i % 2 == 0 else 0x96ffff, 0)
	await process_frame
	if not check(hud.hud_snapshot().chat_compact_count == 20 and hud.hud_snapshot().chat_compact_first == 15, "chat compact following"): return
	if not await capture("hud-chat-compact"): return
	if OS.get_cmdline_user_args().has("--screenshots"):
		var decoration_area := Rect2i(0, 360, 140, 107)
		var undecorated := root.get_texture().get_image().get_region(decoration_area)
		hud.set_hud_decoration(Vector4i(1, 237, 87, 149))
		if not await capture("hud-decoration-dyed"): return
		var decorated := root.get_texture().get_image().get_region(decoration_area)
		if not check(decorated.get_data() != undecorated.get_data(), "draw original HUD decoration"): return
		var ornament: Sprite2D
		for child in hud.get_children():
			if child is Sprite2D:
				ornament = child
				break
		if not check(ornament != null and ornament.position == Vector2(-10, 360), "original decoration anchor"): return
		var source := ornament.texture.get_image()
		var mask: Texture2D = ornament.material.get_shader_parameter("dye_regions")
		var regions := mask.get_image()
		var samples := 0
		for y in range(mini(decoration_area.size.y, source.get_height())):
			for x in range(mini(decoration_area.size.x, source.get_width() - 10)):
				var region := regions.get_pixel(x + 10, y).r8
				if region != 2: continue
				var src := source.get_pixel(x + 10, y)
				var dst := undecorated.get_pixel(x, y)
				var r := ((src.r8 >> 3) * (237 >> 3) / 31 * 25 / 32 + (dst.r8 >> 3) * 7 / 32) << 3
				var g := ((src.g8 >> 2) * (87 >> 2) / 63 * 25 / 32 + (dst.g8 >> 2) * 7 / 32) << 2
				var b := ((src.b8 >> 3) * (149 >> 3) / 31 * 25 / 32 + (dst.b8 >> 3) * 7 / 32) << 3
				var actual := decorated.get_pixel(x, y)
				if not check(absi(actual.r8-r) <= 1 and absi(actual.g8-g) <= 1 and absi(actual.b8-b) <= 1,
					"visible C2 decoration pixel matches RGB565 integer blending"): return
				samples += 1
		if not check(samples > 0, "original ornament has visible C2 pixels"): return
		hud.set_hud_decoration(Vector4i(1, 20, 230, 40))
		if not await capture("hud-decoration-recolored"): return
		if not check(root.get_texture().get_image().get_region(decoration_area).get_data() != decorated.get_data(),
			"HUD decoration dye changes rendered pixels"): return
		hud.set_hud_decoration(Vector4i.ZERO)
		await process_frame
		await RenderingServer.frame_post_draw
		if not check(root.get_texture().get_image().get_region(decoration_area).get_data() == undecorated.get_data(),
			"zero decoration returns to original HUD background"): return
	hud.set_hovered_target(1, 3, 20, false, 1, 5, 10, 50)
	if not check(hud.hud_snapshot().target_portrait == 20 and hud.hud_snapshot().target_hp_width == 41, "monster portrait and half HP"): return
	if not await capture("hud-target-portrait"): return
	hud.set_hovered_target(0, 0, 0, false, 0, 0, 0, 0)
	if not check(hud.hud_snapshot().target_portrait == 20, "portrait persists after pointer leaves"): return
	var portrait_deadline := Time.get_ticks_msec() + 1200
	while hud.hud_snapshot().target_portrait != -1 and Time.get_ticks_msec() < portrait_deadline:
		await process_frame
	if not check(hud.hud_snapshot().target_portrait == -1, "portrait expires"): return
	var portrait_queries: Array = []
	hud.target_portrait_requested.connect(func(opcode: int, actor: int, name: String):
		portrait_queries.append([opcode, actor, name]))
	hud.set_hovered_human(2, 1, 2)
	hud.set_hovered_human(2, 1, 2, "比奇旅人")
	hud.set_hovered_human(2, 1, 2, "比奇旅人")
	if not check(portrait_queries == [[1152, 2, "比奇旅人"]], "query on name availability once per displayed target"): return
	if not check(hud.hud_snapshot().target_portrait == 655 and hud.hud_snapshot().target_hp_width == 0, "human profession portrait without monster HP"): return
	if not await capture("hud-target-human"): return
	var jpeg := portrait_image.save_jpg_to_buffer()
	if not check(not hud.apply_target_portrait(3, jpeg) and hud.apply_target_portrait(2, jpeg), "custom portrait reply requires displayed actor"): return
	if not check(hud.hud_snapshot().target_custom_portrait, "remote custom portrait"): return
	if not await capture("hud-target-human-custom"): return
	hud.set_hovered_human(3, 0, 0, "盟重旅人")
	if not check(portrait_queries == [[1152, 2, "比奇旅人"], [1152, 3, "盟重旅人"]], "new target requests its own portrait"): return
	if not check(not hud.apply_target_portrait(2, jpeg) and not hud.hud_snapshot().target_custom_portrait and hud.hud_snapshot().target_portrait == 650, "changing target clears previous custom portrait"): return
	hud.set_hovered_target(1, 3, 20, false, 0, 5, 10, 50)
	if not check(not hud.apply_target_portrait(1, jpeg) and hud.hud_snapshot().target_hp_width == 41, "monster rejects human portrait reply"): return
	hud.set_hovered_target(0, 0, 0, false, 0, 0, 0, 0)
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
	var overlap_origin: Vector2 = Vector2(hud.hud_snapshot().inventory_position)
	var overlap_cell := overlap_origin + Vector2(36, 79)
	if not check(hud.add_inventory_item(301, 1, 0, 45678, "Minimap test potion", 0, 3), "add item under expanded minimap"): return
	click_at(overlap_cell, true)
	click_at(overlap_cell, false)
	if not check(hud.hud_snapshot().held_inventory_icon == -1, "minimap surface blocks clicks on the covered bag slot"): return
	click_at(overlap_origin + Vector2(12, 25), true)
	click_at(overlap_cell, false)
	var after_release := InputEventMouseMotion.new()
	after_release.position = overlap_origin + Vector2(85, 200)
	Input.parse_input_event(after_release)
	if not check(Vector2(hud.hud_snapshot().inventory_position) == overlap_origin, "release over minimap ends the existing inventory drag"): return
	if not check(hud.hud_snapshot().held_inventory_icon == -1, "ending inventory drag does not pick up an item"): return
	if not await capture("hud-minimap-over-inventory"): return
	click_at(Vector2(792, 249), true)
	click_at(overlap_cell, false)
	if not check(hud.hud_snapshot().held_inventory_icon == -1 and hud.hud_snapshot().minimap_bounds == Rect2(672, 0, 128, 128), "minimap resize owns release over inventory"): return
	if not await capture("hud-minimap-resize-over-inventory"): return
	if OS.get_cmdline_user_args().has("--screenshots"):
		click_at(overlap_cell, true)
		click_at(overlap_cell, false)
		if not check(hud.hud_snapshot().held_inventory_icon == 301, "pick up potion from exposed bag"): return
		var held_motion := InputEventMouseMotion.new()
		held_motion.position = overlap_cell + Vector2(38, 0)
		Input.parse_input_event(held_motion)
		if not await capture("hud-inventory-held-preview"): return
		key(KEY_T)
		if not await capture("hud-minimap-covered-held-preview"): return
		var covered_frame := root.get_texture().get_image()
		held_motion = InputEventMouseMotion.new()
		held_motion.position = Vector2(480, 250)
		Input.parse_input_event(held_motion)
		await process_frame
		await RenderingServer.frame_post_draw
		var idle_frame := root.get_texture().get_image()
		var edge := Rect2i(Vector2i(overlap_origin) + Vector2i(55, 60), Vector2i(4, 38))
		if not check(covered_frame.get_region(edge).get_data() == idle_frame.get_region(edge).get_data(),
			"covered bag cell has no placement highlight"): return
		key(KEY_T)
		await create_timer(0.31).timeout
		click_at(overlap_cell, true)
		click_at(overlap_cell, false)
		if not check(hud.hud_snapshot().held_inventory_icon == -1, "return potion to exposed bag"): return
	key(KEY_T)
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
	map_drag = map_drag.duplicate()
	map_drag.pressed = true
	Input.parse_input_event(map_drag)
	for step in range(1, 9):
		map_motion = map_motion.duplicate()
		map_motion.position = map_drag.position + Vector2(-30, 50) * step
		Input.parse_input_event(map_motion)
	map_drag = map_drag.duplicate()
	map_drag.position = map_motion.position
	map_drag.pressed = false
	Input.parse_input_event(map_drag)
	var chat_before_map_wheel: int = hud.hud_snapshot().chat_compact_first
	var map_wheel := wheel.duplicate() as InputEventMouseButton
	map_wheel.position = Vector2(470, 510)
	if not check(hud.hud_snapshot().minimap_bounds.has_point(map_wheel.position), "drag minimap over chat"): return
	Input.parse_input_event(map_wheel)
	if not check(hud.hud_snapshot().chat_compact_first == chat_before_map_wheel, "minimap blocks wheel over chat"): return
	if not await capture("hud-minimap-over-chat"): return
	if OS.get_cmdline_user_args().has("--screenshots"):
		var target_bar := Rect2i(534, 584, 41, 5)
		var unobstructed_bar := Image.load_from_file(ProjectSettings.globalize_path(
			"res://../build/screenshots/hud-target-portrait.png")).get_region(target_bar)
		var map_bounds: Rect2 = hud.hud_snapshot().minimap_bounds
		map_drag = map_drag.duplicate()
		map_drag.position = map_bounds.get_center()
		map_drag.pressed = true
		Input.parse_input_event(map_drag)
		for step in range(1, 3):
			map_motion = map_motion.duplicate()
			map_motion.position = map_drag.position + Vector2(30, 38) * step
			Input.parse_input_event(map_motion)
		map_drag = map_drag.duplicate()
		map_drag.position = map_motion.position
		map_drag.pressed = false
		Input.parse_input_event(map_drag)
		if not check(hud.hud_snapshot().minimap_bounds.encloses(Rect2(target_bar)), "minimap covers the target HP bar"): return
		await process_frame
		await RenderingServer.frame_post_draw
		var map_only := root.get_texture().get_image().get_region(target_bar)
		hud.set_hovered_target(1, 3, 20, false, 1, 5, 10, 50)
		if not await capture("hud-minimap-target-vitals"): return
		var map_with_bar := root.get_texture().get_image().get_region(target_bar)
		var visible_bar_pixels := 0
		for y in target_bar.size.y:
			for x in target_bar.size.x:
				var bar_pixel := unobstructed_bar.get_pixel(x, y)
				if bar_pixel == map_with_bar.get_pixel(x, y) and bar_pixel != map_only.get_pixel(x, y):
					visible_bar_pixels += 1
		if not check(visible_bar_pixels > 40, "target HP bar draws above overlapping minimap"): return
		hud.set_hovered_target(0, 0, 0, false, 0, 0, 0, 0)
	await create_timer(1.01).timeout
	key(KEY_V)
	if not check(not hud.hud_snapshot().minimap_visible, "minimap closes with V"): return
	click_at(Vector2(643, 513), true)
	click_at(Vector2(643, 513), false)
	if not check(map_requests == [1033, 1033] and hud.apply_minimap_reply(1), "HUD button requests original Mmap"): return
	map_drag = map_drag.duplicate()
	map_drag.position = Vector2(730, 64)
	map_drag.pressed = true
	Input.parse_input_event(map_drag)
	hud.set_map_location("盟重省", 220, 240)
	click_at(overlap_cell, false)
	if not check(hud.hud_snapshot().held_inventory_icon == -1, "map transition preserves ownership of Ctrl drag release"): return
	if not check(map_requests == [1033, 1033, 1033] and not hud.hud_snapshot().minimap_visible, "map transition clears old frame and requests refresh"): return
	if not check(not hud.apply_minimap_reply(0) and not hud.hud_snapshot().minimap_visible, "invalid reply keeps previous map hidden"): return
	if not check(hud.apply_minimap_reply(1002) and hud.hud_snapshot().minimap_visible, "new map reply restores open minimap"): return
	hud.set_map_location("盟重省", 221, 241)
	if not check(map_requests.size() == 3 and hud.hud_snapshot().minimap_visible, "movement in same map does not re-request"): return
	if not await capture("hud-minimap-transition-release"): return
	key(KEY_Y)
	await create_timer(0.2).timeout
	click_at(Vector2(643, 513), true)
	click_at(Vector2(643, 513), false)
	if not check(not hud.hud_snapshot().minimap_visible, "close refreshed minimap"): return
	hud.set_map_location("比奇省", 337, 267)
	click_at(Vector2(643, 513), true)
	click_at(Vector2(643, 513), false)
	if not check(map_requests.size() == 4, "request map before unavailable reply"): return
	var chat_before_unavailable: int = hud.hud_snapshot().chat_compact_count
	hud.apply_minimap_unavailable()
	if not check(not hud.hud_snapshot().minimap_visible and hud.hud_snapshot().chat_compact_count > chat_before_unavailable,
		"711 closes map and posts an unavailable message"): return
	key(KEY_V)
	if not check(map_requests.size() == 4, "711 path respects the map request cooldown"): return
	if not check(hud.apply_minimap_reply(1001) and not hud.hud_snapshot().minimap_visible,
		"711 clears the pending open before a subsequent map reply"): return
	if not await capture("hud-minimap-unavailable"): return
	await create_timer(1.01).timeout
	click_at(Vector2(540, 570), true)
	click_at(Vector2(550, 510), false)
	if not check(hud.hud_snapshot().portrait_map_mode and not hud.hud_snapshot().portrait_map_visible
		and not hud.hud_snapshot().portrait_visible and map_requests.size() == 5,
		"release over the portrait falls through from the upload button to the map request"): return
	if not check(hud.apply_minimap_reply(1001) and hud.hud_snapshot().portrait_map_visible
		and not hud.hud_snapshot().minimap_visible, "710 shows the portrait map independently"): return
	hud.set_minimap_markers(PackedVector2Array(), PackedVector2Array([Vector2(350, 284)]),
		PackedVector2Array([Vector2(360, 276)]))
	if not await capture("hud-portrait-map-markers"): return
	if OS.get_cmdline_user_args().has("--screenshots"):
		var portrait_map_image := root.get_texture().get_image()
		var player_outline := Color8(100, 250, 100)
		for y in range(531, 537):
			for x in range(571, 577):
				var on_outline := x >= 572 and x <= 575 and y >= 532 and y <= 535 and (
					x == 572 or x == 575 or y == 532 or y == 535)
				if not check((portrait_map_image.get_pixel(x, y) == player_outline) == on_outline,
					"portrait map player stroke stays inside the original 4x4 rectangle"): return
		if not check(portrait_map_image.get_pixel(583, 542) == Color8(100, 250, 100)
			and portrait_map_image.get_pixel(591, 538) == Color8(250, 150, 50),
			"portrait map markers use original half-scale coordinates and colors"): return
	key(KEY_J)
	if not check(hud.hud_snapshot().portrait_map_mode, "J respects the 710 cooldown"): return
	var portrait_map_deadline := Time.get_ticks_msec() + 1010
	if OS.get_cmdline_user_args().has("--screenshots"):
		var marker_pixel := Vector2i(572, 532)
		var green := Color8(100, 250, 100)
		var blink_deadline := Time.get_ticks_msec() + 1500
		while root.get_texture().get_image().get_pixelv(marker_pixel) == green and Time.get_ticks_msec() < blink_deadline:
			await RenderingServer.frame_post_draw
		if not check(root.get_texture().get_image().get_pixelv(marker_pixel) != green,
			"portrait map player marker enters the hidden phase"): return
		if not await capture("hud-portrait-map-marker-hidden"): return
		while root.get_texture().get_image().get_pixelv(marker_pixel) != green and Time.get_ticks_msec() < blink_deadline:
			await RenderingServer.frame_post_draw
		if not check(root.get_texture().get_image().get_pixelv(marker_pixel) == green,
			"portrait map player marker returns after the hidden phase"): return
	var restored_map_hud: Node = load("res://main.tscn").instantiate()
	restored_map_hud.set_meta("options_path", options_file)
	var restored_map_requests: Array[int] = []
	restored_map_hud.minimap_requested.connect(func(id: int):
		restored_map_requests.append(id)
		restored_map_hud.apply_minimap_reply(1001))
	root.add_child(restored_map_hud)
	var restored_map: Dictionary = restored_map_hud.hud_snapshot()
	restored_map_hud.free()
	if not check(restored_map_requests == [1033] and restored_map.portrait_map_visible
		and not restored_map.minimap_visible, "reload InformationMode and accept a synchronous map reply"): return
	key(KEY_R)
	if not check(hud.hud_snapshot().expanded and hud.hud_snapshot().portrait_map_mode
		and not hud.hud_snapshot().portrait_map_visible, "expanded chat hides the portrait map without clearing the mode"): return
	if not await capture("hud-portrait-map-expanded-chat"): return
	if OS.get_cmdline_user_args().has("--screenshots"):
		if not check(root.get_texture().get_image().get_pixel(583, 542) != Color8(100, 250, 100),
			"expanded chat suppresses the rendered portrait map markers"): return
	key(KEY_R)
	if not check(hud.hud_snapshot().portrait_map_visible and map_requests.size() == 5,
		"collapsed chat restores the portrait map without a new request"): return
	if not await capture("hud-portrait-map-restored"): return
	if OS.get_cmdline_user_args().has("--screenshots"):
		if not check(root.get_texture().get_image().get_pixel(583, 542) == Color8(100, 250, 100),
			"collapsed chat restores the rendered portrait map markers"): return
		# 0x438222 draws the portrait map before the target portrait at 0x43830B.
		var target_sample := Rect2i(549, 500, 24, 24)
		var map_pixels := root.get_texture().get_image().get_region(target_sample).get_data()
		hud.set_hovered_target(1, 3, 20, false, 1, 5, 10, 50)
		if not await capture("hud-target-over-portrait-map"): return
		var target_pixels := root.get_texture().get_image().get_region(target_sample).get_data()
		if not check(target_pixels != map_pixels and hud.hud_snapshot().portrait_map_visible
			and hud.hud_snapshot().target_hp_width == 41, "target portrait overlays the active portrait map"): return
		hud.set_hovered_target(0, 0, 0, false, 0, 0, 0, 0)
		var focus_deadline := Time.get_ticks_msec() + 1200
		while hud.hud_snapshot().target_portrait != -1 and Time.get_ticks_msec() < focus_deadline:
			await process_frame
		if not check(hud.hud_snapshot().target_portrait == -1, "target focus expires over the portrait map"): return
		await process_frame
		await RenderingServer.frame_post_draw
		if not check(root.get_texture().get_image().get_region(target_sample).get_data() == map_pixels,
			"target expiration reveals the same portrait map without another request"): return
	while Time.get_ticks_msec() < portrait_map_deadline:
		await process_frame
	key(KEY_V)
	if not check(map_requests.size() == 6 and hud.apply_minimap_reply(1001)
		and hud.hud_snapshot().minimap_visible and hud.hud_snapshot().portrait_map_visible,
		"both map views can display the same reply"): return
	hud.set_map_location("盟重省", 220, 240)
	if not check(map_requests.size() == 7 and hud.hud_snapshot().portrait_map_mode
		and not hud.hud_snapshot().portrait_map_visible and not hud.hud_snapshot().minimap_visible,
		"map change hides both old views and sends a single request"): return
	if not check(hud.apply_minimap_reply(1002) and hud.hud_snapshot().portrait_map_visible
		and hud.hud_snapshot().minimap_visible, "710 restores both views with the new map"): return
	var close_maps_deadline := Time.get_ticks_msec() + 1010
	if not await capture("hud-both-maps-after-transition"): return
	while Time.get_ticks_msec() < close_maps_deadline:
		await process_frame
	key(KEY_V)
	key(KEY_J)
	if not check(not hud.hud_snapshot().portrait_map_mode and not hud.hud_snapshot().portrait_map_visible,
		"J returns from the portrait map"): return
	if not check(not hud.hud_snapshot().minimap_visible, "V closes the restored corner map"): return
	hud.set_map_location("比奇省", 337, 267)
	if not check(hud.hud_snapshot().inventory_visible and hud.hud_snapshot().inventory_position == Vector2i(518, 0), "inventory origin"): return
	click_at(overlap_cell, true)
	click_at(overlap_cell, false)
	click_at(overlap_cell, true)
	click_at(overlap_cell, false)
	if not check(hud.apply_inventory_use_reply(635, 0), "remove minimap test item"): return
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
	var close_hover := InputEventMouseMotion.new()
	close_hover.position = Vector2(681, 523)
	Input.parse_input_event(close_hover)
	if not await capture("inventory-close-hint"): return
	var inventory_sound: AudioStreamPlayer = hud.get_node("HudClickSound")
	inventory_sound.stop()
	click_at(Vector2(681, 523), true)
	if not check(not inventory_sound.playing, "inventory close press stays silent"): return
	if not await capture("inventory-close-pressed"): return
	click_at(Vector2(681, 523), false)
	if not check(inventory_sound.playing, "inventory close release plays original click"): return
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
	close_hover = InputEventMouseMotion.new()
	close_hover.position = Vector2(291, 16)
	Input.parse_input_event(close_hover)
	if not await capture("character-status-close-hint"): return
	var status_sound: AudioStreamPlayer = hud.get_node("HudClickSound")
	status_sound.stop()
	click_at(Vector2(291, 16), true)
	if not check(not status_sound.playing, "status close press stays silent"): return
	if not await capture("character-status-close-pressed"): return
	click_at(Vector2(291, 16), false)
	if not check(status_sound.playing, "status close release plays original click"): return
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
	if not check(hud.apply_npc_dialogue(643, 2, "药店老板/<购买/@buy>"), "open NPC dialogue above magic tree"): return
	key(KEY_F2)
	if not check(skill_requests.size() == 3 and spell_requests.is_empty(), "covered magic slot cannot bind or cast"): return
	if not await capture("magic-covered-by-npc"): return
	if not check(hud.apply_npc_dialogue(644, 2, ""), "close NPC dialogue above magic tree"): return
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
	if not check(hud.set_learned_skill(261, 1, 11), "skill ID above one byte"): return
	key(KEY_F11)
	if not check(spell_requests.pop_back() == [11, 5, Vector2(180, 160)], "shortcut dispatch uses low byte of skill ID"): return
	if not check(hud.apply_skill_removal(261), "remove high ID fixture before category tests"): return
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
	if not check(not hud.apply_skill_progress(65535, 215, 2) and not hud.apply_skill_progress(5, -1, 2), "skill progress rejects unknown skill and invalid experience"): return
	if not check(hud.apply_skill_progress(5, 215, 2), "640 updates learned skill progress"): return
	if not await capture("magic-progress-reply"): return
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
	var binding_count := skill_requests.size()
	key(KEY_N)
	if not check(hud.hud_snapshot().options_visible, "options above skill popup"): return
	click_at(Vector2(435, 202), true)
	click_at(Vector2(435, 202), false)
	if not check(skill_requests.size() == binding_count, "covered popup does not bind"): return
	if not await capture("magic-popup-covered"): return
	key(KEY_N)
	click_at(Vector2(435, 202), true)
	click_at(Vector2(435, 202), false)
	if not check(skill_requests.back() == Vector2i(5, 50), "shortcut menu binds F2"): return
	click_at(Vector2(700, 470), true)
	click_at(Vector2(700, 470), false)
	var other_popup := popup_event.duplicate() as InputEventMouseButton
	other_popup.position = Vector2(385, 86)
	Input.parse_input_event(other_popup)
	click_at(Vector2(390, 130), true)
	click_at(Vector2(390, 130), false)
	if not check(skill_requests.back() == Vector2i(5, 49), "double click opens popup without replacing selected skill"): return
	if not await capture("magic-popup-preserved-selection"): return
	var magic_sound: AudioStreamPlayer = hud.get_node("HudClickSound")
	magic_sound.stop()
	click_at(Vector2(636, 32), true)
	if not check(not magic_sound.playing, "magic category press stays silent"): return
	click_at(Vector2(636, 32), false)
	if not check(magic_sound.playing, "magic category release plays original click"): return
	if not await capture("magic-category-selected"): return
	if not check(hud.hud_snapshot().skill_tab == 7, "magic category selection"): return
	OS.delay_msec(20)
	click_at(Vector2(671, 160), true)
	click_at(Vector2(671, 160), false)
	if not check(hud.hud_snapshot().skill_scroll == 6, "magic scroll step"): return
	if not await capture("magic-tree-scrolled"): return
	if not check(hud.set_learned_skill(3, 1, 0) and hud.apply_skill_removal(3), "212 removes last skill in current category"): return
	if not check(hud.hud_snapshot().skill_tab == 0 and hud.hud_snapshot().skill_scroll == 6, "212 selects first nonempty category and preserves scroll"): return
	if not await capture("magic-removal-reply"): return
	var record := PackedByteArray()
	record.resize(74)
	record[6] = 39
	record[5] = 2
	record[4] = 65
	record[8] = 65
	if not check(not hud.apply_skill_records(211, 2, record), "truncated skill list rejected"): return
	if not check(hud.apply_skill_records(211, 1, record) and hud.hud_snapshot().skill_tab == 1, "211 replaces list and selects first populated category"): return
	record[6] = 5
	if not check(hud.apply_skill_records(210, 1, record) and hud.hud_snapshot().skill_tab == 1, "210 adds skill while retaining active category"): return
	if not await capture("magic-list-reply"): return
	var refreshed_popup := popup_event.duplicate() as InputEventMouseButton
	refreshed_popup.position = Vector2(385, 86)
	refreshed_popup.pressed = true
	refreshed_popup.double_click = true
	click_at(Vector2(385, 86), true)
	click_at(Vector2(385, 86), false)
	Input.parse_input_event(refreshed_popup)
	if not await capture("magic-refreshed-key-menu"): return
	if OS.get_cmdline_user_args().has("--screenshots"):
		var popup_image := root.get_texture().get_image()
		if not check(popup_image.get_pixel(485, 226) != popup_image.get_pixel(445, 226), "popup displays retained shortcut icon"): return
	var binds_before_removal := skill_requests.size()
	if not check(hud.apply_skill_removal(39) and hud.hud_snapshot().skill_tab == 0, "removing popup target changes category"): return
	click_at(Vector2(390, 130), true)
	click_at(Vector2(390, 130), false)
	if not check(skill_requests.size() == binds_before_removal, "stale popup cannot change bindings"): return
	if not await capture("magic-removed-popup-target"): return
	click_at(Vector2(700, 470), true)
	click_at(Vector2(700, 470), false)
	var crowded_records := PackedByteArray()
	record[4] = 0
	record[6] = 1
	for i in 15: crowded_records.append_array(record)
	record[6] = 5
	crowded_records.append_array(record)
	if not check(hud.apply_skill_records(211, 16, crowded_records) and hud.apply_skill_progress(5, 10, 1), "retain skills beyond display limit"): return
	if not await capture("magic-visible-limit"): return
	if not check(hud.apply_skill_removal(1), "remove one visible record"): return
	if not check(hud.apply_skill_progress(5, 0xffffffff, 1), "accept full DWORD experience"): return
	if not await capture("magic-visible-after-removal"): return
	click_at(Vector2(385, 152), true)
	click_at(Vector2(385, 152), false)
	if not await capture("magic-signed-experience"): return
	if OS.get_cmdline_user_args().has("--screenshots"):
		var limited := Image.load_from_file(ProjectSettings.globalize_path("res://../build/screenshots/magic-visible-limit.png"))
		var revealed := root.get_texture().get_image()
		if not check(limited.get_region(Rect2i(378, 144, 40, 30)).get_data() != revealed.get_region(Rect2i(378, 144, 40, 30)).get_data(), "removal reveals skill previously past the display limit"): return
	click_at(Vector2(636, 32), true)
	click_at(Vector2(636, 32), false)
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
	var group_sound: AudioStreamPlayer = hud.get_node("HudClickSound")
	group_sound.stop()
	click_at(Vector2(300, 174), true)
	if not check(not group_sound.playing, "group allowance press stays silent"): return
	click_at(Vector2(300, 174), false)
	if not check(group_sound.playing, "group allowance release plays original click"): return
	if not check(hud.hud_snapshot().group_allow and allowance_requests == [true], "group allowance request"): return
	if not await capture("group-members"): return
	var group_hover := InputEventMouseMotion.new()
	group_hover.position = Vector2(375, 322)
	Input.parse_input_event(group_hover)
	for frame in range(30): await process_frame
	if not await capture("group-add-member-hint"): return
	hud.notification(Node.NOTIFICATION_WM_MOUSE_EXIT)
	if not await capture("group-hint-after-mouse-exit"): return
	if OS.get_cmdline_user_args().has("--screenshots"):
		var image := root.get_texture().get_image()
		if not check(not image.get_pixel(408, 335).is_equal_approx(Color8(255, 255, 150)), "mouse exit clears group tooltip"): return
	var member_requests: Array = []
	hud.group_member_requested.connect(func(id: int, name: String): member_requests.append([id, name]))
	click_at(Vector2(375, 322), true)
	click_at(Vector2(375, 322), false)
	if not check(hud.hud_snapshot().group_request == 1021, "open add member dialog"): return
	for ch in "银杏剑客": key(0, ch.unicode_at(0))
	if not await capture("group-member-dialog"): return
	var member_input: LineEdit = hud.get_node("ChatInput")
	key(KEY_SPACE, 32)
	if not check(hud.hud_snapshot().group_request == 1021 and member_input.text == "银杏剑客 ", "space types into focused dialog edit"): return
	key(KEY_BACKSPACE)
	member_input.release_focus()
	if not await capture("group-dialog-keyboard-activation"): return
	key(KEY_SPACE)
	if not check(member_requests == [[1021, "银杏剑客"]], "space activates dialog when edit is unfocused"): return
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
	click_at(Vector2(490, 316), true)
	hud.notification(Node.NOTIFICATION_WM_WINDOW_FOCUS_OUT)
	click_at(Vector2(490, 316), false)
	if not check(hud.hud_snapshot().group_request == 1022, "focus loss cancels modal button press"): return
	group_sound.stop()
	click_at(Vector2(490, 316), true)
	if not check(not group_sound.playing, "modal cancel press stays silent"): return
	if not await capture("group-dialog-cancel-pressed"): return
	click_at(Vector2(490, 316), false)
	if not check(group_sound.playing and hud.hud_snapshot().group_request == 0 and member_requests.size() == 1,
		"modal cancel release plays original click without submitting"): return
	key(KEY_ESCAPE)
	await process_frame
	if not check(not hud.hud_snapshot().group_visible, "group escape closes"): return
	key(KEY_E)
	key(KEY_G)
	key(KEY_W)
	key(KEY_Q)
	if not await capture("hud-panel-stack"): return
	var stacked_bag_origin := Vector2(hud.hud_snapshot().inventory_position)
	var stacked_bag_grab := stacked_bag_origin + Vector2(12, 25)
	click_at(stacked_bag_grab, true)
	key(KEY_Q, 0, true)
	if not check(hud.hud_snapshot().group_request == 109, "open modal during inventory drag"): return
	click_at(stacked_bag_grab, false)
	key(KEY_TAB)
	key(KEY_ENTER)
	var interrupted_bag_motion := InputEventMouseMotion.new()
	interrupted_bag_motion.position = stacked_bag_grab - Vector2(20, 0)
	Input.parse_input_event(interrupted_bag_motion)
	if not check(hud.hud_snapshot().group_request == 0 and Vector2(hud.hud_snapshot().inventory_position) == stacked_bag_origin,
		"dismissing modal does not resume inventory drag"): return
	if not await capture("hud-inventory-drag-interrupted"): return
	click_at(stacked_bag_grab, true)
	var stacked_bag_motion := InputEventMouseMotion.new()
	stacked_bag_motion.position = stacked_bag_grab - Vector2(200, 0)
	Input.parse_input_event(stacked_bag_motion)
	click_at(stacked_bag_motion.position, false)
	if not check(Vector2(hud.hud_snapshot().inventory_position) == stacked_bag_origin - Vector2(200, 0),
		"dragging exposed bag border moves it over the status window"): return
	if not await capture("hud-inventory-raised-over-status"): return
	key(KEY_ESCAPE)
	if not check(not hud.hud_snapshot().inventory_visible and hud.hud_snapshot().status_visible, "drag raises inventory above status"): return
	key(KEY_ESCAPE)
	if not check(not hud.hud_snapshot().status_visible and hud.hud_snapshot().group_visible, "status closes second after inventory drag"): return
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
	if not check(not hud.apply_guild_overview_reply(773, "lo\r".to_ascii_buffer()), "continuation requires initial overview"): return
	if not check(hud.apply_guild_overview_reply(753, "Bichon\r\r0\r<Notice>\rHel".to_ascii_buffer()), "initial overview retains partial line"): return
	if not check(hud.apply_guild_overview_reply(773, "lo\rIgnored".to_ascii_buffer()), "773 completes initial partial line"): return
	click_at(Vector2(196, 410), true)
	click_at(Vector2(196, 410), false)
	if not check(hud.apply_guild_overview_reply(774, "Next\r".to_ascii_buffer()) and hud.hud_snapshot().guild_tab == 1, "774 updates overview without changing active tab"): return
	click_at(Vector2(145, 410), true)
	click_at(Vector2(145, 410), false)
	if not await capture("guild-overview-continuation"): return
	if OS.get_cmdline_user_args().has("--screenshots"):
		var heading_pixels := root.get_texture().get_image().get_region(Rect2i(137, 82, 514, 88))
		var matches := false
		for y in range(heading_pixels.get_height()):
			for x in range(heading_pixels.get_width()):
				var pixel := heading_pixels.get_pixel(x, y)
				if pixel.r8 == 255 and absi(pixel.g8 - 151) <= 1 and pixel.b8 == 0:
					matches = true
		if not check(matches, "guild overview section heading uses original orange COLORREF"): return
	hud.set_guild_overview("比奇行会", PackedStringArray(["欢迎加入比奇行会", "今晚集合练级"]), PackedStringArray(["今晚集合练级"]))
	var guild_hover := InputEventMouseMotion.new()
	guild_hover.position = Vector2(196, 410)
	Input.parse_input_event(guild_hover)
	for i in 30: await process_frame
	if not await capture("guild-button-hint"): return
	var guild_sound: AudioStreamPlayer = hud.get_node("HudClickSound")
	guild_sound.stop()
	click_at(Vector2(196, 410), true)
	if not check(not guild_sound.playing, "guild tab press stays silent"): return
	if not await capture("guild-member-tab-pressed"): return
	click_at(Vector2(196, 410), false)
	if not check(guild_sound.playing, "guild tab release plays original click"): return
	if not check(hud.hud_snapshot().guild_tab == 1, "guild members tab"): return
	var member_body := "#1/*Leader/Bichon/#2/*Member/Ginkgo/Incomplete".to_ascii_buffer()
	if not check(hud.apply_guild_members_reply(756, member_body), "756 parses guild member ranks"): return
	if not check(hud.apply_guild_members_reply(775, "#9/*Old/Obsolete/".to_ascii_buffer()), "775 replaces guild members"): return
	if not check(hud.apply_guild_members_reply(776, member_body), "776 replaces guild members"): return
	if not check(not hud.apply_guild_members_reply(776, ("x".repeat(260) + "/").to_ascii_buffer()), "oversized member token rejected"): return
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
	hud.post_system_notice("系统公告：比奇省欢迎各位旅人。", 0x96ffff, 0)
	if not check(hud.hud_snapshot().guild_notice_count == 3, "104 system text enters guild notices"): return
	hud.post_system_notice("比奇".repeat(32), 0x96ffff, 0)
	if not check(hud.hud_snapshot().guild_notice_count == 5, "system notice wraps at 514 pixels"): return
	if not await capture("guild-notices"): return
	click_at(Vector2(145, 410), true)
	click_at(Vector2(145, 410), false)
	if not check(hud.hud_snapshot().guild_tab == 0 and hud.hud_snapshot().guild_first == 0, "overview tab resets shared first row"): return
	var notice_history := PackedStringArray()
	for i in 79:
		notice_history.append("历史公告 %02d" % i)
	hud.set_guild_notices(notice_history)
	Input.parse_input_event(guild_wheel.duplicate())
	if not check(hud.hud_snapshot().guild_first == 1, "scroll guild overview before system notice"): return
	hud.post_system_notice("系统公告：通知列表已滚动。", 0x96ffff, 0)
	if not check(hud.hud_snapshot().guild_first == 0 and hud.hud_snapshot().guild_notice_count == 79, "trimming notices updates shared first row on overview tab"): return
	click_at(Vector2(246, 410), true)
	click_at(Vector2(246, 410), false)
	if not await capture("guild-notice-history-trim"): return
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
	var guild_wire_edits: Array = []
	hud.guild_edit_bytes_requested.connect(func(id: int, body: PackedByteArray): guild_wire_edits.append([id, body]))
	hud.guild_edit_requested.connect(func(id: int, body: String): guild_edits.append([id, body]))
	click_at(Vector2(400, 415), true)
	click_at(Vector2(400, 415), false)
	var editor: TextEdit = hud.get_node("GuildEdit")
	if not check(editor.visible and editor.has_focus() and editor.size == Vector2(468, 170), "guild announcement editor layout"): return
	if not check(editor.text == "今晚集合练级\n", "announcement prefill uses editable lines only"): return
	var editor_shortcuts: Array[int] = []
	var record_editor_shortcut := func(opcode: int): editor_shortcuts.append(opcode)
	hud.minimap_requested.connect(record_editor_shortcut)
	key(KEY_V)
	if not check(editor_shortcuts.is_empty() and editor.has_focus(), "focused guild editor blocks HUD shortcut"): return
	hud.minimap_requested.disconnect(record_editor_shortcut)
	var editor_origin := editor.position
	click_at(Vector2(150, 140), true)
	hud.notification(NOTIFICATION_APPLICATION_FOCUS_OUT)
	var editor_motion := InputEventMouseMotion.new()
	editor_motion.position = Vector2(190, 160)
	Input.parse_input_event(editor_motion)
	click_at(Vector2(190, 160), false)
	if not check(editor.position == editor_origin, "focus loss cancels guild editor drag"): return
	click_at(Vector2(575, 130), true)
	hud.notification(NOTIFICATION_APPLICATION_FOCUS_OUT)
	click_at(Vector2(575, 130), false)
	if not check(editor.visible and guild_edits.is_empty(), "focus loss cancels pressed submit"): return
	editor.text = "行会公告\n\n今晚集合"
	if not await capture("guild-announcement-editor"): return
	var editor_sound: AudioStreamPlayer = hud.get_node("HudClickSound")
	editor_sound.stop()
	click_at(Vector2(575, 130), true)
	if not check(not editor_sound.playing, "guild submit press stays silent"): return
	click_at(Vector2(575, 130), false)
	if not check(editor_sound.playing, "guild submit release plays original click"): return
	if not check(not editor.visible and guild_edits == [[1040, "行会公告\r \r今晚集合"]], "guild announcement submit"): return
	if not check(guild_wire_edits == [[1040, PackedByteArray([208, 208, 187, 225, 185, 171, 184, 230, 13, 32, 13, 189, 241, 205, 237, 188, 175, 186, 207])]], "announcement wire retains ANSI bytes and CR separators"): return
	click_at(Vector2(550, 415), true)
	click_at(Vector2(550, 415), false)
	if not check(editor.visible and editor.text == "#1 <Leader>\nBichon \n#2 <Member>\n", "guild member editor uses rank source"): return
	if not await capture("guild-member-editor"): return
	editor_sound.stop()
	click_at(Vector2(660, 328), true)
	click_at(Vector2(660, 328), false)
	if not check(editor_sound.playing, "guild close release plays original click"): return
	if not check(not editor.visible and guild_edits.size() == 1, "guild editor cancel"): return
	hud.set_guild_overview("Bichon", PackedStringArray(), PackedStringArray(["A".repeat(4010)]))
	click_at(Vector2(400, 415), true)
	click_at(Vector2(400, 415), false)
	if not check(editor.visible and editor.text.length() == 4011, "long prefill bypasses user edit limit"): return
	editor.text = editor.text.left(4005)
	if not await capture("guild-announcement-long-prefill"): return
	click_at(Vector2(575, 130), true)
	click_at(Vector2(575, 130), false)
	if not check(guild_edits.size() == 2 and guild_edits[1] == [1040, "A".repeat(3999)], "long prefill submits at most 3999 bytes"): return
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
	if OS.get_cmdline_user_args().has("--screenshots"):
		var options_image := root.get_texture().get_image()
		var caption_rects := [Rect2i(205, 60, 390, 15), Rect2i(411, 119, 156, 21),
			Rect2i(411, 179, 156, 21), Rect2i(411, 239, 156, 21),
			Rect2i(165, 152, 155, 15), Rect2i(165, 182, 155, 15)]
		for index in range(caption_rects.size()):
			var caption := options_image.get_region(caption_rects[index])
			var expected := Color8(250, 250, 248) if index == 0 else Color8(255, 150, 0)
			var glyph_pixels := 0
			for y in range(caption.get_height()):
				for x in range(caption.get_width()):
					if caption.get_pixel(x, y).is_equal_approx(expected): glyph_pixels += 1
			if not check(glyph_pixels > 20, "system options caption %d uses the original color and bounds" % index): return
	var display_options: Dictionary = hud.hud_snapshot()
	for position in [Vector2(335, 149), Vector2(335, 179), Vector2(580, 240)]:
		click_at(position, true)
		click_at(position, false)
	var changed_options: Dictionary = hud.hud_snapshot()
	for option in ["magic_light", "damage_numbers", "reverse_pan"]:
		if not check(changed_options[option] != display_options[option], "toggle system option " + option): return
	if not await capture("system-options-display-toggles"): return
	click_at(Vector2(596, 152), true)
	var music_drag := InputEventMouseMotion.new()
	music_drag.position = Vector2(523, 152)
	Input.parse_input_event(music_drag)
	hud.notification(Node.NOTIFICATION_WM_WINDOW_FOCUS_OUT)
	if not check(hud.hud_snapshot().music_level == -50, "focus loss retains last dragged music level"): return
	if not await capture("system-options-volume-after-focus-loss"): return
	var saved_during_drag: Node = load("res://main.tscn").instantiate()
	saved_during_drag.set_meta("options_path", options_file)
	root.add_child(saved_during_drag)
	var saved_level: int = saved_during_drag.hud_snapshot().music_level
	saved_during_drag.free()
	if not check(saved_level == -50, "in-range drag survives reload"): return
	# The old client allows an overshoot in the drag calculation, but reload validates [-100, 0].
	click_at(Vector2(512, 152), true)
	music_drag = music_drag.duplicate()
	music_drag.position = Vector2(618, 152)
	Input.parse_input_event(music_drag)
	click_at(music_drag.position, false)
	if not check(hud.hud_snapshot().music_level == 2, "music slider preserves the legacy right endpoint"): return
	if not await capture("system-options-volume-right-end"): return
	click_at(Vector2(595, 447), false)
	if not check(not hud.hud_snapshot().options_visible, "options close"): return
	var reloaded: Node = load("res://main.tscn").instantiate()
	reloaded.set_meta("options_path", options_file)
	root.add_child(reloaded)
	var persisted: Dictionary = reloaded.hud_snapshot()
	reloaded.free()
	DirAccess.remove_absolute(ProjectSettings.globalize_path(options_file))
	if not check(not persisted.music_enabled and persisted.music_level == 0, "system options reload from original INI keys"): return
	for option in ["magic_light", "damage_numbers", "reverse_pan"]:
		if not check(persisted[option] == changed_options[option], "reload system option " + option): return
	var portrait_hover := InputEventMouseMotion.new()
	if OS.get_cmdline_user_args().has("--screenshots"):
		var entry_region := Rect2i(535, 567, 40, 13)
		portrait_hover.position = Vector2(540, 580)
		Input.parse_input_event(portrait_hover)
		await process_frame
		await RenderingServer.frame_post_draw
		var hidden_entry := root.get_texture().get_image().get_region(entry_region).get_data()
		portrait_hover = portrait_hover.duplicate()
		portrait_hover.position.y = 579
		Input.parse_input_event(portrait_hover)
		await process_frame
		await RenderingServer.frame_post_draw
		var normal_entry := root.get_texture().get_image().get_region(entry_region).get_data()
		if not check(normal_entry != hidden_entry, "portrait entry draws at the last row inside its hover rectangle"): return
		click_at(Vector2(540, 570), true)
		if not await capture("hud-portrait-entry-pressed"): return
		var pressed_entry := root.get_texture().get_image().get_region(entry_region).get_data()
		if not check(pressed_entry != normal_entry, "portrait entry uses the pressed frame"): return
		click_at(Vector2(540, 580), false)
		if not check(not hud.hud_snapshot().portrait_visible, "release outside the portrait hover rectangle cancels activation"): return
		key(KEY_W)
		if not check(hud.hud_snapshot().inventory_visible, "open inventory to receive release"): return
		click_at(Vector2(540, 570), true)
		click_at(Vector2(580, 100), false)
		portrait_hover = portrait_hover.duplicate()
		portrait_hover.position = Vector2(540, 570)
		Input.parse_input_event(portrait_hover)
		if not await capture("hud-portrait-release-over-inventory"): return
		var released_entry := root.get_texture().get_image().get_region(entry_region).get_data()
		if not check(released_entry == normal_entry and not hud.hud_snapshot().portrait_visible,
			"release over inventory clears the portrait pressed frame"): return
		key(KEY_W)
		portrait_hover = portrait_hover.duplicate()
	portrait_hover.position = Vector2(540, 570)
	Input.parse_input_event(portrait_hover)
	for frame in range(20): await process_frame
	if not await capture("hud-portrait-entry-hint"): return
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
	var mount_close_hover := InputEventMouseMotion.new()
	mount_close_hover.position = Vector2(250, 288)
	Input.parse_input_event(mount_close_hover)
	for i in 30: await process_frame
	if not await capture("hud-mount-close-hint"): return
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
	key(KEY_S)
	if not check(hud.hud_snapshot().mount_visible and hud.hud_snapshot().mount_bounds == Rect2(40, 30, 276, 322), "mount reopen retains dragged position"): return
	if not await capture("hud-mount-reopened"): return
	click_at(Vector2(290, 318), true)
	click_at(Vector2(290, 318), false)
	if not check(not hud.hud_snapshot().mount_visible, "close button follows dragged mount window"): return
	click_at(Vector2(750, 490), true)
	click_at(Vector2(750, 490), false)
	if not check(hud.hud_snapshot().quest_visible and hud.hud_snapshot().quest_bounds == Rect2(0, 0, 340, 440), "quest HUD button and original bounds"): return
	if not await capture("hud-quest-window"): return
	var quest_close_hover := InputEventMouseMotion.new()
	quest_close_hover.position = Vector2(280, 117)
	Input.parse_input_event(quest_close_hover)
	for i in 30: await process_frame
	if not await capture("hud-quest-close-hint"): return
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
	click_at(Vector2(280, 90), true)
	click_at(Vector2(280, 90), false)
	if not check(quest_details == [1], "pending detail request is not repeated"): return
	if not check(hud.fail_quest_detail_request(1), "failed request allows retry"): return
	click_at(Vector2(280, 90), true)
	click_at(Vector2(280, 90), false)
	if not check(quest_details == [1, 1], "retry sends a fresh detail request"): return
	if not check(hud.set_quest_details(1, PackedStringArray()) and not hud.fail_quest_detail_request(1), "empty reply completes request"): return
	click_at(Vector2(280, 90), true)
	click_at(Vector2(280, 90), false)
	if not check(quest_details == [1, 1], "empty detail response is cached"): return
	if not await capture("hud-quest-empty-detail"): return
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
	if not check(hud.set_trade_item(0, 1, 301, 1, 98767, "Removed offer"), "populate local offer for removal"): return
	click_at(Vector2(70, 75), true)
	click_at(Vector2(70, 75), false)
	if not check(hud.hud_snapshot().trade_return_slot == 1, "select offer before removal"): return
	if not check(hud.set_trade_item(0, 1, -1, 0) and hud.hud_snapshot().trade_return_slot == -1,
		"removing selected offer clears the return preview"): return
	var offers_before := offers.size()
	var empty_bag_cell := trade_pickup + Vector2(0, 38)
	click_at(empty_bag_cell, true)
	click_at(empty_bag_cell, false)
	if not check(offers.size() == offers_before, "removed offer cannot emit a return request"): return
	if not await capture("hud-trade-selected-offer-removed"): return
	if not check(hud.add_inventory_item(301, 5, hud.hud_snapshot().inventory_row * 6 + 5, 98766, "Trade stack", 0, 3), "add stack for splitting"): return
	click_at(trade_pickup + Vector2(38, 0), true)
	click_at(trade_pickup + Vector2(38, 0), false)
	click_at(Vector2(70, 75), true)
	click_at(Vector2(70, 75), false)
	if not check(hud.hud_snapshot().group_request == 1307, "stack opens quantity dialog"): return
	if not check(not hud.apply_trade_offer_reply(676) and hud.hud_snapshot().held_inventory_icon == 301,
		"rejection without an outstanding offer preserves the item and quantity input"): return
	if not await capture("hud-trade-quantity-before-offer"): return
	key(0, 57)
	if not check(hud.apply_shop_purchase_reply(651, 1) and hud.hud_snapshot().group_request == 1437,
		"purchase alert replaces trade quantity dialog"): return
	if not check(not input.has_focus() and input.text.is_empty() and input.position == Vector2(185, 575),
		"replacement alert restores chat input bounds and clears quantity"): return
	if not await capture("hud-trade-quantity-replaced-by-alert"): return
	key(KEY_ENTER)
	click_at(Vector2(70, 75), true)
	click_at(Vector2(70, 75), false)
	if not check(hud.hud_snapshot().group_request == 1307, "trade quantity can reopen after replacement"): return
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
	if not check(hud.set_trade_item(0, 27, 1043, 2), "apply tall trade item above scrolled page"): return
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
	if not check(hud.hud_snapshot().group_request == 1029, "gold prompt opens before cancellation"): return
	key(0, "9".unicode_at(0))
	if not check(hud.apply_trade_cancel() and hud.hud_snapshot().group_request == 0, "trade cancellation closes gold prompt"): return
	if not check(gold_requests.is_empty() and not input.has_focus() and input.text.is_empty(), "cancelled gold input releases and clears chat field"): return
	if not await capture("hud-trade-gold-prompt-cancelled"): return
	if not check(hud.open_trade("比奇旅人"), "reopen trade for fresh gold input"): return
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
	if OS.get_cmdline_user_args().has("--screenshots"):
		var exit_frame := root.get_texture().get_image()
		if not check(exit_frame.get_pixel(220, 126) != Color8(72, 72, 72), "keyboard exit dialog draws immediately"): return
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
