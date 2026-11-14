# 测试内容：Alt 左键切割姿态、尸体距离判断、原版请求字段和远端蹲姿回包。
extends "map_harvest.gd"

func cut_at(pointer: Vector2) -> void:
	for pressed in [true,false]:
		var button := InputEventMouseButton.new()
		button.button_index=MOUSE_BUTTON_LEFT
		button.position=pointer
		button.pressed=pressed
		button.alt_pressed=true
		root.push_input(button,true)

func run() -> void:
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	var origin: Vector2i=hud.hud_snapshot().player_cell
	var cell := origin
	for offset in [Vector2i(1,0),Vector2i(-1,0),Vector2i(0,1),Vector2i(0,-1)]:
		if hud.set_scene_human(5,origin+offset,4,1,"目标"):
			cell=origin+offset
			break
	if not check(cell!=origin and hud.set_scene_human_body(5,0,1),"创建相邻目标"): return
	hud.set_character_vitals(100,100,0,0,0,100)
	hud.set_local_actor_identity(77)
	await settle()
	var packets: Array[PackedByteArray]=[]
	hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(packet))
	cut_at(hud.hud_snapshot().scene_human_bounds[5].get_center())
	if not check(packets.size()==1 and decode_request(packets[0]).decode_u16(4)==Mir3HudView.CM_SITDOWN,
		"活体只触发蹲姿"): return
	await settle(1.0)
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_SITDOWN,5,cell.x,cell.y,4)),"远端蹲姿回包入队"): return
	await settle()
	if not check(hud.hud_snapshot().scene_human_actions[5].x==8,"远端进入动作8"): return
	await settle(1.0)
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_DEATH,5,cell.x,cell.y,4)),"接收尸体回包"): return
	await settle()
	cut_at(hud.hud_snapshot().scene_human_bounds[5].get_center())
	if not check(packets.size()==3 and hud.hud_snapshot().player_action==8,"相邻尸体先蹲再发割肉请求"): return
	var sit := decode_request(packets[1])
	var cut := decode_request(packets[2])
	if not check(sit.decode_u16(4)==Mir3HudView.CM_SITDOWN and cut.decode_u16(4)==Mir3HudView.CM_BUTCH
		and cut.decode_u32(0)==5 and cut.decode_u16(6)==origin.x and cut.decode_u16(8)==origin.y
		and cut.decode_u16(10)==sit.decode_u16(8),"核对割肉目标、角色坐标和朝向"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_SITDOWN,77,origin.x,origin.y,cut.decode_u16(10))),
		"本地蹲姿回包"): return
	hud.set_process(false)
	if not await capture("map-butch-crouch"): return
	hud.queue_free()
	await settle()
	print("MAP_BUTCH_UI_OK")
	quit()
