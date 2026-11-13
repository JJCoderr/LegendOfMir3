# Test harness only. Client behavior lives in C++.
extends SceneTree

var hud: Node
var actions: Array[int] = []

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

func run() -> void:
	hud = load("res://main.tscn").instantiate()
	root.size = Vector2i(800, 600)
	Input.use_accumulated_input = false
	root.add_child(hud)
	hud.hud_action.connect(func(button: int): actions.append(button))
	await process_frame
	if not check(hud.hud_snapshot().ready, "original resource loading"): return
	click_at(Vector2(0, 0), true)
	click_at(Vector2(620, 473), false)
	await process_frame
	if not check(hud.hud_snapshot().expanded and actions == [14], "release-inside expansion"): return
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
	print("HUD_SMOKE_OK: resources, expansion, collapse, stat switching, action dispatch")
	quit(0)
