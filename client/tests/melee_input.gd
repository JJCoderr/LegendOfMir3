# 测试点击相邻活怪物后播放普攻并发送带角色格和朝向的3014请求。
extends "map_harvest.gd"

func attack_click(position: Vector2, force: bool = false) -> void:
	for pressed in [true,false]:
		var event := InputEventMouseButton.new()
		event.button_index=MOUSE_BUTTON_LEFT
		event.position=position
		event.shift_pressed=force
		event.pressed=pressed
		root.push_input(event,true)

func run() -> void:
	var gate_case := OS.get_cmdline_user_args().has("--player-gate")
	var options_path := "user://melee-%d.ini" % OS.get_process_id()
	var config := ConfigFile.new()
	config.set_value("Options","AttackUser",0 if gate_case else 1)
	if not check(config.save(options_path)==OK,"准备独立攻击选项"): return
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("options_path",options_path)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	# 本用例只检查动作本身；模拟服务器逐次确认。
	hud.client_packet_ready.connect(func(_packet: PackedByteArray): hud.apply_game_message("+GOOD".to_ascii_buffer()))
	await settle()
	hud.set_character_vitals(100,100,0,0,0,100)
	if gate_case:
		await verify_player_gate()
		return
	var origin: Vector2i=hud.hud_snapshot().player_cell
	var cell := origin+Vector2i(0,1)
	var feature := PackedByteArray()
	feature.resize(16)
	feature[0]=3
	feature[2]=20
	var entry := game_reply(Mir3HudView.SM_TURN,6,cell.x,cell.y,0)
	entry.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(entry),"建立相邻怪物"): return
	await settle()
	var bounds: Rect2=hud.hud_snapshot().scene_chicken_bounds[6]
	if not check(bounds.has_area(),"目标进入视野"): return
	var attacks: Array[PackedByteArray]=[]
	hud.client_packet_ready.connect(func(packet: PackedByteArray):
		var decoded := decode_request(packet)
		if decoded.decode_u16(4)==Mir3HudView.CM_HIT: attacks.append(decoded))
	for pressed in [true,false]:
		var event := InputEventMouseButton.new()
		event.button_index=MOUSE_BUTTON_LEFT
		event.position=bounds.get_center()
		event.pressed=pressed
		root.push_input(event,true)
	if not check(attacks.size()==1,"相邻目标触发一次普攻"): return
	if not check(attacks[0].decode_u16(0)==origin.x and attacks[0].decode_u16(2)==origin.y
		and attacks[0].decode_u16(8)==4,"普攻带角色所在格且面向南侧目标"): return
	if not check(hud.hud_snapshot().player_action==9,"空手播放动作9"): return
	if not await capture("melee-input"): return
	await settle(1.5)
	if not check(hud.remove_scene_actor(6) and hud.set_scene_human(7,cell,0,0,"玩家"),
		"在相邻格建立玩家目标"): return
	await settle()
	var player_bounds: Rect2=hud.hud_snapshot().scene_human_bounds[7]
	if not check(player_bounds.has_area(),"玩家目标进入视野"): return
	var mouse := InputEventMouseButton.new()
	mouse.button_index=MOUSE_BUTTON_LEFT
	mouse.position=player_bounds.get_center()
	mouse.pressed=true
	root.push_input(mouse,true)
	if not check(attacks.size()==2,"启用 AttackUser 时普通左键可攻击玩家"): return
	mouse=mouse.duplicate()
	mouse.pressed=false
	root.push_input(mouse,true)
	if not await capture("melee-player-input"): return
	hud.queue_free()
	await settle()
	print("MELEE_INPUT_OK")
	quit()

func verify_player_gate() -> void:
	var origin: Vector2i=hud.hud_snapshot().player_cell
	if not check(not hud.hud_snapshot().option_force_attack,"AttackUser已关闭"): return
	if not check(hud.set_scene_human(7,origin+Vector2i(0,1),0,0,"玩家"),"建立白名玩家"): return
	await settle()
	var packets: Array[PackedByteArray]=[]
	hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(decode_request(packet)))
	var bounds: Rect2=hud.hud_snapshot().scene_human_bounds[7]
	attack_click(bounds.get_center())
	if not check(packets.is_empty() and hud.hud_snapshot().player_cell==origin,
		"关闭选项后普通点击白名玩家不发动作请求"): return
	attack_click(bounds.get_center(),true)
	if not check(packets.size()==1 and packets[0].decode_u16(4)==Mir3HudView.CM_HIT,
		"Shift允许对白名玩家强攻"): return
	await settle(1.5)
	for color in [125,69]:
		if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CHANGENAMECOLOR,7,color,0,0)),
			"接收名称颜色回包"): return
		var before := packets.size()
		attack_click(bounds.get_center())
		if not check(packets.size()==before+1 and packets.back().decode_u16(4)==Mir3HudView.CM_HIT,
			"原版名称颜色例外允许普通攻击"): return
		await settle(1.5)
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CHANGENAMECOLOR,7,255,0,0)),
		"恢复白名"): return
	var before := packets.size()
	attack_click(bounds.get_center())
	if not check(packets.size()==before,"颜色恢复后重新禁止普通攻击"): return
	if not await capture("melee-player-gate"): return
	hud.queue_free()
	await settle()
	print("MELEE_PLAYER_GATE_OK")
	quit()
