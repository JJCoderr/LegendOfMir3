# 测试内容：父任务展开、子列表与描述原包、迟到正文归属、空正文重试及缓存复用、增量、内联描述及完成态全量刷新。
extends "quest_wire.gd"

func press_quest(point: Vector2) -> void:
	click_at(point,true)
	click_at(point,false)

func expected_query(opcode: int, sequence: int, parent: int, child: int = 0) -> PackedByteArray:
	var packet := ("#%d" % sequence).to_ascii_buffer()
	packet.append_array(encode_scene_bytes(PackedByteArray([child,0,0,0,opcode&255,opcode>>8,0,0,0,0,parent,0])))
	packet.append(33)
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
	var list := quest_reply(Mir3HudView.SM_QUEST_LIST,"30/Border patrol/2/0/Visit the gate/".to_ascii_buffer(),1,0)
	if not check(hud.apply_game_message(list),"接收父任务和简介"): return
	press_quest(Vector2(76,96))
	if not check(hud.hud_snapshot().quest_description=="Visit the gate","选中父任务显示简介"): return
	press_quest(Vector2(279,87))
	if not check(packets.size()==2 and packets[-1]==expected_query(Mir3HudView.CM_QUERYQUESTCHILDREN,2,30),"展开请求1048使用父编号"): return
	var children := quest_reply(Mir3HudView.SM_QUEST_CHILDREN,"21/Talk to the guard/20/Survey the gate/".to_ascii_buffer(),2,30)
	if not check(hud.apply_game_message(children) and hud.hud_snapshot().quest_detail_rows==2,"展开显示两条子任务"): return
	press_quest(Vector2(90,113))
	if not check(packets.size()==3 and packets[-1]==expected_query(Mir3HudView.CM_QUERYQUESTCHILDTEXT,3,30,21),"点击首条子任务发送1049"): return
	press_quest(Vector2(90,113))
	if not check(packets.size()==4 and packets[-1]==expected_query(Mir3HudView.CM_QUERYQUESTCHILDTEXT,4,30,21),"等待时允许再次点击查询"): return
	var late := quest_reply(Mir3HudView.SM_QUEST_CHILD_TEXT,"Inspect the north gate.".to_ascii_buffer(),20,30)
	if not check(hud.apply_game_message(late) and hud.hud_snapshot().quest_description=="","其他子任务正文不会覆盖当前选择"): return
	var selected := quest_reply(Mir3HudView.SM_QUEST_CHILD_TEXT,"Speak with the guard at the eastern gate.".to_ascii_buffer(),21,30)
	if not check(hud.apply_game_message(selected) and hud.hud_snapshot().quest_description.begins_with("Speak with"),"匹配子编号后显示正文"): return
	if not await capture("quest-wire-child-description"): return
	press_quest(Vector2(90,128))
	if not check(hud.hud_snapshot().quest_description=="Inspect the north gate." and packets.size()==4,"点击其他子任务显示已缓存正文"): return
	var increment := quest_reply(Mir3HudView.SM_QUEST_CHILD_ADDED,
		"30/22/Report to the captain/Return to the city/23/Collect supplies//".to_ascii_buffer(),30,2)
	if not check(hud.apply_game_message(increment) and hud.hud_snapshot().quest_detail_rows==4,"追加子任务及内联正文"): return
	if not check(hud.hud_snapshot().quest_description=="Inspect the north gate.","增量保留选中正文"): return
	press_quest(Vector2(90,143))
	if not check(hud.hud_snapshot().quest_description=="Return to the city" and packets.size()==4,"内联正文无需再查询"): return
	var duplicate := quest_reply(Mir3HudView.SM_QUEST_CHILD_ADDED,
		"30/22/Repeat//24/Unreachable//".to_ascii_buffer(),30,2)
	if not check(hud.apply_game_message(duplicate) and hud.hud_snapshot().quest_detail_rows==4,"首个重复编号停止追加"): return
	var truncated := quest_reply(Mir3HudView.SM_QUEST_CHILD_ADDED,"30/25/Incomplete".to_ascii_buffer(),30,1)
	if not check(not hud.apply_game_message(truncated) and hud.hud_snapshot().quest_detail_rows==4,"截断增量保持已有子任务"): return
	if not await capture("quest-wire-child-increment"): return
	var completed := quest_reply(Mir3HudView.SM_QUEST_LIST,
		"30/Border patrol/2/1/Patrol complete/".to_ascii_buffer(),1,0)
	if not check(hud.apply_game_message(completed),"原包全量完成状态"): return
	var refreshed: Dictionary = hud.hud_snapshot()
	if not check(refreshed.quest_completed==PackedInt32Array([1]) and refreshed.quest_detail_rows==0
		and refreshed.quest_description=="","完成更新清理旧展开和正文"): return
	press_quest(Vector2(76,96))
	press_quest(Vector2(279,87))
	if not check(packets[-1]==expected_query(Mir3HudView.CM_QUERYQUESTCHILDREN,5,30),"刷新后重新取得子列表"): return
	if not check(hud.apply_game_message(children),"重新接收完成任务的子标题"): return
	press_quest(Vector2(90,113))
	if not check(hud.hud_snapshot().quest_selected==-1,"选中子项取消父标题高亮"): return
	if not check(hud.apply_game_message(selected),"重新取得完成任务的正文"): return
	if not await capture("quest-wire-completed"): return
	hud.queue_free()
	await create_timer(0.1).timeout
	print("QUEST_DETAILS_UI_OK")
	quit()
