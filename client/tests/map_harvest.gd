# 测试内容：本地挖矿输入与发包、回包驱动的地表变化、远端动作及无标记回归。
extends "map_events.gd"

func shift_click(pointer: Vector2, pressed: bool) -> void:
	var button := InputEventMouseButton.new()
	button.button_index=MOUSE_BUTTON_LEFT
	button.position=pointer
	button.pressed=pressed
	button.shift_pressed=true
	root.push_input(button,true)

func decode_request(packet: PackedByteArray) -> PackedByteArray:
	var bytes := PackedByteArray()
	var buffer := 0
	var bits := 0
	for i in range(2,packet.size()-1):
		buffer=(buffer<<6)|(packet[i]-60)
		bits+=6
		if bits>=8:
			bits-=8
			bytes.append((buffer>>bits)&255)
			buffer&=(1<<bits)-1
	return bytes

func run() -> void:
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	var origin: Vector2i=hud.hud_snapshot().player_cell
	var packets: Array[PackedByteArray]=[]
	hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(packet))
	if not check(hud.set_local_actor_identity(77) and hud.set_player_attachments(0,7)
		and hud.set_character_vitals(100,100,0,0,0,100),"本地角色装备矿镐"): return
	if not check(hud.apply_game_message(show_event(701,3,origin)),"在角色所在格布置地表事件"): return
	var pointer := Vector2(origin*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(49,1)
	shift_click(pointer,true)
	shift_click(pointer,false)
	if not check(packets.size()==1 and hud.hud_snapshot().player_action==10,"Shift 左键只发送一次请求并开始原地挥击"): return
	var request := decode_request(packets[0])
	if not check(request.size()==12 and request.decode_u16(4)==3015
		and request.decode_u16(0)==origin.x and request.decode_u16(2)==origin.y
		and request.decode_u16(8)==2,"挖矿请求携带角色坐标和向东朝向"): return
	await create_timer(0.7).timeout
	if not check(hud.hud_snapshot().map_event_frames[701]==230,"没有标记回包时不推进地表"): return
	shift_click(pointer,true)
	shift_click(pointer,false)
	var reply := game_reply(Mir3HudView.SM_HIT2,77,origin.x,origin.y,2)
	reply.append(1)
	if not check(packets.size()==2 and hud.apply_game_message(reply)
		and hud.hud_snapshot().map_event_frames[701]==230,"收到标记后仍等待动作帧"): return
	for i in range(80):
		await process_frame
		if hud.hud_snapshot().map_event_frames[701]==231: break
	if not check(hud.hud_snapshot().map_event_frames[701]==231 and hud.hud_snapshot().player_cell==origin
		and hud.hud_snapshot().map_event_count==2,"本地第3帧播放特效并推进地表"): return
	hud.set_process(false)
	if not await capture("map-harvest-local"): return
	hud.set_process(true)
	hud.apply_game_message(game_reply(Mir3HudView.SM_HIDEEVENT,701,0,0,0))
	await create_timer(0.6).timeout
	var cell := origin
	for offset in [Vector2i(-2,1),Vector2i(2,1),Vector2i(-1,1),Vector2i(1,1)]:
		if hud.set_scene_human(5,origin+offset,4,1,"矿工"):
			cell=origin+offset
			break
	if not check(cell!=origin,"可见可走格创建矿工"): return
	if not check(hud.apply_game_message(show_event(700,3,cell)),"创建地表事件"): return
	var attack := game_reply(Mir3HudView.SM_HIT2,5,cell.x,cell.y,4)
	if not check(hud.apply_game_message(attack),"无标记攻击原包"): return
	await create_timer(1.0).timeout
	if not check(hud.hud_snapshot().map_event_frames[700]==230,"普通攻击保持图号"): return
	attack.append(1)
	if not check(hud.apply_game_message(attack) and hud.hud_snapshot().map_event_frames[700]==230,"入队尚未改变地表"): return
	for i in range(90):
		await process_frame
		if hud.hud_snapshot().map_event_frames[700]==231: break
	var state: Dictionary=hud.hud_snapshot()
	if not check(state.map_event_frames[700]==231 and state.map_event_count==2,"动作帧推进地表并触发335"): return
	hud.set_process(false)
	if not await capture("map-harvest-impact"): return
	hud.set_process(true)
	await create_timer(0.8).timeout
	if not check(hud.hud_snapshot().map_event_count==1 and hud.hud_snapshot().map_event_frames[700]==231,"一次动作只推进一次且短效果清理"): return
	hud.queue_free()
	await create_timer(0.1).timeout
	print("MAP_HARVEST_UI_OK")
	quit()
