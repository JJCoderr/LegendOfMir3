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
	var input: LineEdit = hud.get_node("ChatInput")
	if not check(input.position == Vector2(185, 575) and input.size == Vector2(344, 15), "compact input bounds"): return
	key(KEY_ENTER)
	await process_frame
	if not check(input.has_focus(), "enter focuses input"): return
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
	print("HUD_SMOKE_OK: resources, expansion, collapse, stat switching, action dispatch")
	quit(0)
