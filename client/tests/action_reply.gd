# 定向验收：真实输入发包后等待确认，拒绝恢复格坐标/朝向/相机，确认不打断动画，换图清除回滚状态。
extends "melee_input.gd"

func run() -> void:
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.set_local_actor_identity(77)
	hud.set_character_vitals(100,100,0,0,0,100)
	var state: Dictionary=hud.hud_snapshot()
	var origin: Vector2i=state.player_cell
	var camera: Vector2i=state.map_camera_pixel
	var facing: int=state.player_facing
	var packets: Array[PackedByteArray]=[]
	hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(decode_request(packet)))
	if not check(hud.walk_to_map_cell(origin+Vector2i(0,1)),"发起走路"): return
	var deadline := Time.get_ticks_msec()+1000
	while hud.hud_snapshot().player_cell==origin and Time.get_ticks_msec()<deadline:
		await process_frame
	state=hud.hud_snapshot()
	if not check(packets.size()==1 and state.action_reply_pending and state.player_cell!=origin,
		"本地预测前进一步并等待服务器：包=%s, 等待=%s, 位置=%s, 帧=%s" % [packets.size(),state.action_reply_pending,state.player_cell,state.walk_frame]): return
	if not check(not hud.walk_to_map_cell(origin),"确认前不能提交下一次移动"): return
	if not check(hud.apply_game_message("+FAIL/100".to_ascii_buffer()),"接收拒绝文本包"): return
	state=hud.hud_snapshot()
	if not check(state.player_cell==origin and state.player_facing==facing and state.map_camera_pixel==camera
		and state.walk_frame==-1 and not state.action_reply_pending,"拒绝恢复位置方向相机并终止位移"): return
	if not await capture("action-reply-rollback"): return
	await settle(0.55)
	if not check(hud.walk_to_map_cell(origin+Vector2i(0,1)),"拒绝后允许再次移动"): return
	await settle(0.15)
	state=hud.hud_snapshot()
	var frame: int=state.walk_frame
	if not check(hud.apply_game_message("+GOOD/200".to_ascii_buffer()),"接收确认文本包"): return
	state=hud.hud_snapshot()
	if not check(not state.action_reply_pending and state.walk_frame==frame and frame>=0,"确认不截断移动动画"): return
	await settle(0.6)
	var cell: Vector2i=hud.hud_snapshot().player_cell
	var pointer := Vector2(cell*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(49,1)
	attack_click(pointer,true)
	if not check(packets.size()==3 and hud.hud_snapshot().action_reply_pending
		and hud.hud_snapshot().player_facing==2,"普攻同样保存回滚状态并转向东"): return
	await settle(0.7)
	attack_click(pointer,true)
	if not check(packets.size()==3,"动画结束也不能越过确认门禁"): return
	hud.apply_game_message("+FAIL".to_ascii_buffer())
	if not check(hud.hud_snapshot().player_facing==4 and hud.hud_snapshot().player_action==-1,"攻击拒绝恢复出招前朝向并站立"): return
	if not check(hud.change_map("0","比奇省",origin),"换图清除回滚位置"): return
	hud.apply_game_message("+FAIL/300".to_ascii_buffer())
	if not check(hud.hud_snapshot().player_cell==origin and not hud.hud_snapshot().action_reply_pending,"旧拒绝包不能拉回旧地图位置"): return
	if not check(not hud.apply_game_message("+UNKNOWN".to_ascii_buffer()),"未知控制包不冒充确认"): return
	hud.queue_free()
	await settle()
	print("ACTION_REPLY_UI_OK")
	quit()
