# 测试点击相邻活怪物后播放普攻并发送带角色格和朝向的3014请求。
extends "map_harvest.gd"

func run() -> void:
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.set_character_vitals(100,100,0,0,0,100)
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
	await settle(1.0)
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
