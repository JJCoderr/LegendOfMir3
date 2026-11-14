# 定向验收：本地死亡排队、骑乘死亡外观与尸体保持、死后输入/拒绝/回血限制、32直接尸体和同身份重新入场。
extends "forced_motion.gd"

func death_reply(opcode: int, cell: Vector2i, gender: int=1) -> PackedByteArray:
	var body := PackedByteArray()
	body.resize(16)
	body[0]=gender
	var packet := game_reply(opcode,77,cell.x,cell.y,4)
	packet.append_array(encode_scene_bytes(body))
	return packet

func run() -> void:
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.set_local_actor_identity(77)
	hud.set_character_vitals(100,100,0,0,0,100)
	hud.set_player_mount(1)
	var origin: Vector2i=hud.hud_snapshot().player_cell
	var destination := origin+Vector2i(0,1)
	var packets: Array[PackedByteArray]=[]
	hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(packet))
	hud.walk_to_map_cell(destination)
	await settle(0.05)
	if not check(hud.apply_game_message(death_reply(Mir3HudView.SM_NOWDEATH,destination)),"完整死亡外观包入队"): return
	if not check(not hud.hud_snapshot().player_dead and hud.hud_snapshot().forced_motion_queue==1,"移动过程中死亡包等待出队"): return
	var deadline := Time.get_ticks_msec()+2500
	while not hud.hud_snapshot().player_dying and Time.get_ticks_msec()<deadline:
		await process_frame
	if not check(hud.hud_snapshot().player_dying and hud.hud_snapshot().player_action==19,"启动本地死亡动作19"): return
	await settle(0.35)
	if not await capture("local-life-dying"): return
	if not check(not hud.walk_to_map_cell(origin),"死亡中禁止本地移动"): return
	hud.apply_game_message("+FAIL".to_ascii_buffer())
	if not check(hud.hud_snapshot().player_dead and hud.hud_snapshot().player_cell==destination,"FAIL不能将死亡角色回滚站立"): return
	deadline=Time.get_ticks_msec()+2000
	while hud.hud_snapshot().player_dying and Time.get_ticks_msec()<deadline:
		await process_frame
	if not check(hud.hud_snapshot().player_dead and not hud.hud_snapshot().player_dying
		and hud.hud_snapshot().character_hp.x==0,"死亡结束保持尸体并HP归零"): return
	if not await capture("local-life-corpse"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_HEALTHSPELLCHANGED,77,20,5,100))
		and hud.hud_snapshot().character_hp==Vector2i(20,100) and hud.hud_snapshot().player_dead,"本地身份回血更新HUD但不伪造复活"): return
	hud.apply_game_message(motion_reply(Mir3HudView.SM_REVIVE_REFRESH,77,origin,4))
	await settle(0.1)
	if not check(hud.hud_snapshot().player_dead and hud.hud_snapshot().player_cell==origin-Vector2i(0,1)
		and hud.hud_snapshot().walk_frame==-1 and hud.hud_snapshot().character_hp.x==0,
		"强制走步只刷新尸体锚点，保持死亡且不播放行走"): return
	var logon := game_reply(Mir3HudView.SM_LOGON,77,origin.x,origin.y,4)
	var feature := PackedByteArray()
	feature.resize(16)
	logon.append_array(encode_scene_bytes(feature))
	logon.append_array(encode_scene_bytes(PackedByteArray([0,0,0,0,0])))
	logon.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(logon) and not hud.hud_snapshot().player_dead
		and hud.hud_snapshot().player_cell==origin,"同一身份重新入场清除死亡和残留动作"): return
	hud.apply_game_message(game_reply(Mir3HudView.SM_HEALTHSPELLCHANGED,77,100,10,100))
	if not await capture("local-life-reentered"): return
	# 424994 在每帧检查坐标，服务器直接重定位也会关闭对话。
	if not check(hud.apply_npc_dialogue(Mir3HudView.SM_MERCHANTSAY,2,"/对话"),"打开NPC对话"): return
	var corpse_cell := origin+Vector2i(6,0)
	hud.apply_game_message(death_reply(Mir3HudView.SM_DEATH,corpse_cell,0))
	await settle(0.05)
	if not check(hud.hud_snapshot().player_dead and not hud.hud_snapshot().player_dying,"32直接固定尸体"): return
	if not check(hud.hud_snapshot().player_cell==corpse_cell and hud.hud_snapshot().npc_dialogue_actor==0,
		"跨出五格的服务器重定位关闭NPC对话"): return
	hud.apply_game_message(death_reply(Mir3HudView.SM_NOWDEATH,origin,0))
	await settle(0.05)
	if not check(not hud.hud_snapshot().player_dying,"重复死亡不重播倒地"): return
	hud.change_map("0","比奇省",origin)
	if not check(hud.hud_snapshot().player_dead,"普通切图不冒充复活"): return
	# 只允许首个移动和重新入场背包查询，服务器死亡过程不生成客户端动作。
	if not check(packets.size()==2,"死亡和恢复回包不生成额外移动攻击请求"): return
	hud.queue_free()
	await settle()
	print("LOCAL_LIFE_UI_OK")
	quit()
