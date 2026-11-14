# 测试内容：打开任务窗口发送原版查询、全量排序与完成标记、增量及重复、坏包拒绝与离场清理。
extends "chat_submission.gd"

func quest_reply(opcode: int, body: PackedByteArray, first: int, third: int) -> PackedByteArray:
	var packet := game_reply(opcode,0,first,0,third)
	packet.append_array(encode_scene_bytes(body))
	return packet

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(packet))
	key(KEY_D)
	if not check(hud.hud_snapshot().quest_visible and packets.size()==1 and packets[0]==expected_selection_request(Mir3HudView.CM_QUERYQUESTLIST,1,""),"开窗发送1046原包"): return
	var added := quest_reply(Mir3HudView.SM_QUEST_ADDED,"18/Delivery".to_ascii_buffer(),0,1)
	if not check(hud.apply_game_message(added) and packets.size()==2 and hud.hud_snapshot().quest_titles.is_empty(),"无全量时追加消息触发查询"): return

	var list := "30/".to_ascii_buffer()
	list.append_array(PackedByteArray([0xb1,0xc8,0xc6,0xe6,0xc8,0xce,0xce,0xf1]))
	list.append_array("/2/0//10/Patrol/0/1".to_ascii_buffer())
	if not check(hud.apply_game_message(quest_reply(Mir3HudView.SM_QUEST_LIST,list,2,0)),"接收带GBK标题的全量"): return
	var state: Dictionary = hud.hud_snapshot()
	if not check(state.quest_titles==PackedStringArray(["Patrol","比奇任务"]) and state.quest_completed==PackedInt32Array([1,0]),"按编号排序及独立完成标记"): return
	key(KEY_D)
	key(KEY_D)
	if not check(packets.size()==2,"已初始化再次打开不查询"): return
	if not check(hud.apply_game_message(added) and hud.apply_game_message(added),"增量与重复消息"): return
	if not check(hud.hud_snapshot().quest_titles==PackedStringArray(["Patrol","Delivery","比奇任务"]),"追加排序且无重复项"): return
	if not check(not hud.apply_game_message(quest_reply(Mir3HudView.SM_QUEST_LIST,list,3,0)) and hud.hud_snapshot().quest_titles.size()==3,"截断全量不破坏现有列表"): return
	if not await capture("quest-wire-list"): return
	if not check(hud.apply_game_message(quest_reply(Mir3HudView.SM_QUEST_LIST,PackedByteArray(),0,0)) and hud.hud_snapshot().quest_titles.is_empty(),"空全量清除任务"): return
	if not check(hud.apply_game_message(quest_reply(Mir3HudView.SM_QUEST_LIST,list,2,0)),"离场前恢复非空列表"): return
	hud.open_login()
	if not check(not hud.hud_snapshot().quest_visible and hud.hud_snapshot().quest_titles.is_empty() and not hud.apply_game_message(added),"离场关闭并拒绝迟到消息"): return
	hud.queue_free()
	await create_timer(0.1).timeout
	print("QUEST_LIST_UI_OK")
	quit()
