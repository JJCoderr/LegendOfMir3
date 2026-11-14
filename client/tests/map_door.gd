# 测试内容：原地图门请求、锁门反馈、开关画面、通行及换图复位。
extends "login_smoke.gd"

var door_packets: Array[PackedByteArray] = []

func tap(target: Vector2i) -> void:
	var position := Vector2(target*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(5,5)
	click_at(position,true)
	click_at(position,false)

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	var door := Vector2i(436,438)
	var start := Vector2i(436,439)
	if not check(hud.change_map("0","比奇省",start),"读取原地图真实门格"): return
	await create_timer(1.65).timeout
	hud.client_packet_ready.connect(func(bytes: PackedByteArray): door_packets.append(bytes))
	if not await capture("map-door-closed"): return
	tap(door)
	var expected := "#1".to_ascii_buffer()
	var op := Mir3HudView.CM_OPENDOOR
	expected.append_array(encode_scene_bytes(PackedByteArray([60,0,0,0,op&255,op>>8,180,1,182,1,0,0])))
	expected.append(33)
	if not check(door_packets.size()==1 and door_packets[0]==expected and hud.hud_snapshot().walk_frame<0,"关闭门发送完整请求且挡路 %s != %s, frame %s" % [door_packets,expected,hud.hud_snapshot().walk_frame]): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_OPENDOOR_LOCK,0,door.x,door.y,0))
		and not hud.hud_snapshot().chat_last_text.is_empty(),"锁门提示"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_OPENDOOR_OK,0,door.x,door.y,0)),"原包开门"): return
	if not await capture("map-door-open"): return
	tap(door)
	if not check(hud.hud_snapshot().map_step_destination==door and hud.hud_snapshot().walk_frame>=0,"打开后可以跨门"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CLOSEDOOR,0,door.x,door.y,0)),"原包关门"): return
	var before_reentry := door_packets.size()
	if not check(hud.change_map("0","比奇省",start),"切图恢复门初态"): return
	await create_timer(1.65).timeout
	tap(door)
	if not check(door_packets.size()==before_reentry+1 and hud.hud_snapshot().walk_frame<0,"换图后关闭门再次挡路"): return
	# 41603D 在走步前检查目的格的邻门，并先发开门请求。
	var beside := Vector2i(437,439)
	var before_walk := door_packets.size()
	tap(beside)
	var deadline := Time.get_ticks_msec()+1000
	while door_packets.size()<before_walk+2 and Time.get_ticks_msec()<deadline:
		await process_frame
	if not check(door_packets.size()==before_walk+2 and door_packets[before_walk].slice(2,18)==expected.slice(2,18)
		and hud.hud_snapshot().map_step_destination==beside,"走到门旁时先发开门请求再走步"): return
	var run_start := Vector2i(438,440)
	if not check(hud.change_map("0","比奇省",run_start),"设置跑步开门测试位置"): return
	hud.set_character_vitals(30,30,20,20,0,0)
	await settle(1.65)
	door_packets.clear()
	var right := InputEventMouseButton.new()
	right.button_index=MOUSE_BUTTON_RIGHT
	right.position=Vector2(Vector2i(434,440)*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(5,5)
	right.pressed=true
	root.push_input(right,true)
	if not check(door_packets.size()==1 and hud.apply_game_message("+GOOD".to_ascii_buffer()),"确认跑步前的首个走步"): return
	deadline=Time.get_ticks_msec()+2000
	while not hud.hud_snapshot().map_running and Time.get_ticks_msec()<deadline:
		await process_frame
	right=right.duplicate()
	right.pressed=false
	root.push_input(right,true)
	if not check(hud.hud_snapshot().map_running and door_packets.size()==3
		and door_packets[1].slice(2,18)==expected.slice(2,18),
		"跑步前请求首步北方两格的关闭门，跑步=%s，发包=%s" % [hud.hud_snapshot().map_running,door_packets]): return
	hud.queue_free()
	await create_timer(0.1).timeout
	print("MAP_DOOR_UI_OK")
	quit()
