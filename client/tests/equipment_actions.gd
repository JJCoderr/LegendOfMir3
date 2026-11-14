# 测试内容：穿戴与脱下完整请求编码、失败恢复、成功外观更新、独立新增包返还不重复，以及错序和重复确认拒绝。
extends "equipment_wire.gd"

func expected_request(opcode: int, id: int, slot: int, name: String, sequence: int) -> PackedByteArray:
	var raw := PackedByteArray([id&255,(id>>8)&255,(id>>16)&255,(id>>24)&255,opcode&255,opcode>>8,slot,0,0,0,0,0])
	var result := ("#"+str(sequence)).to_ascii_buffer()
	result.append_array(encode_scene_bytes(raw))
	result.append_array(encode_scene_bytes(name.to_ascii_buffer()))
	result.append(33)
	return result

func release_at(point: Vector2) -> void:
	await create_timer(0.31).timeout
	click_at(point,false)

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("options_path","user://equipment-actions-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	var packets: Array[PackedByteArray] = []
	hud.client_packet_ready.connect(func(bytes: PackedByteArray): packets.append(bytes))
	var old_item := item_record(100,"Old necklace",1520,1,20,0)
	var new_item := item_record(0x89abcdef,"New necklace",1520,1,20,0)
	hud.apply_game_message(equipment_packet([[3,old_item]]))
	hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[new_item]))
	key(KEY_Q)
	key(KEY_W)
	await settle()
	var bag_point := Vector2(hud.hud_snapshot().inventory_position)+Vector2(36,79)
	var equipment_point := Vector2(hud.hud_snapshot().status_position)+Vector2(185,100)
	await release_at(bag_point)
	await release_at(equipment_point)
	if not check(hud.hud_snapshot().equipment_pending and packets==[expected_request(Mir3HudView.CM_TAKEONITEM,0x89abcdef,3,"New necklace",1)],"穿戴完整原包，编号DWORD、槽位和正文"): return
	if not check(not hud.apply_game_message(game_reply(Mir3HudView.SM_TAKEOFF_OK,0,0,0,0)) and hud.hud_snapshot().equipment_pending,"错序确认不结束事务"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_TAKEON_FAIL,0,0,0,0)) and hud.hud_snapshot().bag_records[0].id==0x89abcdef and hud.hud_snapshot().equipment_records[0].id==100,"穿戴失败恢复新旧物品"): return
	await release_at(bag_point)
	await release_at(equipment_point)
	if not check(packets[1]==expected_request(Mir3HudView.CM_TAKEONITEM,0x89abcdef,3,"New necklace",2),"重试共享序号"): return
	hud.apply_game_message(packet(Mir3HudView.SM_ADDITEM,[old_item]))
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_TAKEON_OK,0x00010100,0,0,0)),"穿戴成功"): return
	if not check(hud.hud_snapshot().bag_records.size()==1 and hud.hud_snapshot().equipment_records[0].id==0x89abcdef and hud.hud_snapshot().player_attachments.y==1 and hud.hud_snapshot().player_body==Vector2i(0,1),"新增包先到不重复返还，成功更新外观"): return
	await settle()
	if not await capture("equipment-actions-equipped"): return
	await release_at(equipment_point)
	await release_at(bag_point+Vector2(38,0))
	if not check(packets[2]==expected_request(Mir3HudView.CM_TAKEOFFITEM,0x89abcdef,3,"New necklace",3) and hud.apply_game_message(game_reply(Mir3HudView.SM_TAKEOFF_FAIL,0,0,0,0)),"脱下请求与失败恢复"): return
	if not check(hud.hud_snapshot().equipment_records[0].id==0x89abcdef and hud.hud_snapshot().held_inventory_icon==-1,"失败放回装备槽"): return
	await release_at(equipment_point)
	await release_at(bag_point+Vector2(38,0))
	if not check(packets[3]==expected_request(Mir3HudView.CM_TAKEOFFITEM,0x89abcdef,3,"New necklace",4) and hud.apply_game_message(game_reply(Mir3HudView.SM_TAKEOFF_OK,0x00010000,0,0,0)),"脱下成功"): return
	if not check(hud.hud_snapshot().bag_records.size()==1 and hud.hud_snapshot().equipment_records.is_empty() and hud.hud_snapshot().player_attachments.y==0,"成功只清持有物品和更新外观，等待独立返还"): return
	if not check(not hud.apply_game_message(game_reply(Mir3HudView.SM_TAKEOFF_OK,0,0,0,0)),"重复确认拒绝"): return
	hud.apply_game_message(packet(Mir3HudView.SM_ADDITEM,[new_item]))
	if not check(hud.hud_snapshot().bag_records.size()==2 and packets.size()==4,"独立新增包只返还一次，无额外请求"): return
	await settle()
	if not await capture("equipment-actions-returned"): return
	hud.apply_game_message(equipment_packet([[3,old_item]]))
	await release_at(equipment_point)
	click_at(equipment_point,false)
	if not check(packets.size()==5 and packets[4]==expected_request(Mir3HudView.CM_EQUIPMENT_QUICK_ACTION,3,0,"",5) and not hud.hud_snapshot().equipment_pending,"装备快速操作用recognition槽位及空正文"): return
	hud.queue_free()
	await create_timer(0.1).timeout
	print("EQUIPMENT_ACTIONS_UI_OK")
	quit()
