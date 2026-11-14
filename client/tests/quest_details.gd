# 测试内容：父任务展开、子列表与描述原包、迟到正文归属、空正文重试及缓存复用。
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
	hud.queue_free()
	await create_timer(0.1).timeout
	print("QUEST_DETAILS_UI_OK")
	quit()
