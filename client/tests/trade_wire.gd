# 测试内容：交易完整请求编码、分堆剩余与新编号、增删失败恢复、同名去重、双方金币、取消返还、完成独立交付、十二件限制及原包提示。
extends "inventory_wire.gd"

var sent: Array[PackedByteArray] = []
func expected(opcode: int, id: int = 0, quantity: int = 0, name: String = "") -> PackedByteArray:
	var raw := PackedByteArray([id&255,(id>>8)&255,(id>>16)&255,(id>>24)&255,opcode&255,opcode>>8,0,0,0,0,quantity&255,quantity>>8])
	var result := ("#"+str((sent.size()-1)%9+1)).to_ascii_buffer()
	result.append_array(encode_scene_bytes(raw))
	result.append_array(encode_scene_bytes(name.to_ascii_buffer()))
	result.append(33)
	return result
func open_packet() -> PackedByteArray:
	var result := game_reply(Mir3HudView.SM_TRADE_OPEN,0,0,0,0)
	result.append_array(encode_scene_bytes("Partner".to_ascii_buffer()))
	return result
func peer_packet(opcode: int, id: int, name: String = "Peer") -> PackedByteArray:
	return packet(opcode,[item_record(id,name,301,1,1,0)])
func release(point: Vector2) -> void:
	await create_timer(0.31).timeout
	click_at(point,false)
func offer_first(split: bool = false) -> void:
	await release(Vector2(554,79))
	await release(Vector2(32,75))
	if split:
		key(0,50)
		key(KEY_ENTER)
func take_back() -> void:
	await release(Vector2(32,75))
	await release(Vector2(592,79))

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("options_path","user://trade-wire-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.client_packet_ready.connect(func(bytes: PackedByteArray): sent.append(bytes))
	hud.set_map_location("比奇省",337,267)
	hud.set_trade_context(2,Vector2i(338,267),1,false,"Partner")
	key(KEY_C)
	if not check(sent[-1]==expected(Mir3HudView.CM_TRADE_REQUEST,0,0,"Partner"),"交易请求完整原包"): return
	if not check(hud.apply_game_message(open_packet()) and hud.hud_snapshot().trade_visible and hud.hud_snapshot().inventory_visible,"开交易并打开背包"): return
	hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[item_record(100,"Stack",301,5,1,3)]))
	await offer_first(true)
	if not check(sent[-1]==expected(Mir3HudView.CM_TRADE_ADD_ITEM,100,2,"Stack"),"分堆数量在参数三"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_TRADE_ADD_OK,200,3,0,0)),"675剩余数量确认"): return
	var state: Dictionary = hud.hud_snapshot()
	if not check(state.bag_records[0].id==100 and state.bag_records[0].count==3 and state.trade_records[0].id==200 and state.trade_records[0].count==2,"保留原编号剩余与交易新编号"): return
	if not check(hud.apply_game_message(open_packet()) and hud.hud_snapshot().trade_records==state.trade_records,"重复开窗保留交易"): return
	await take_back()
	if not check(sent[-1]==expected(Mir3HudView.CM_TRADE_REMOVE_ITEM,200,0,"Stack") and hud.apply_game_message(game_reply(Mir3HudView.SM_TRADE_REMOVE_FAIL,0,0,0,0)),"取回失败回到交易区"): return
	await take_back()
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_TRADE_REMOVE_OK,0,0,0,0)) and hud.hud_snapshot().trade_records.is_empty() and hud.hud_snapshot().bag_records[0].count==3,"堆叠同名存在时不重复插入或累加"): return
	hud.apply_game_message(packet(Mir3HudView.SM_UPDATEITEM,[item_record(100,"Stack",301,5,1,3)]))
	if not check(hud.hud_snapshot().bag_records[0].count==5,"独立物品消息更新权威数量"): return
	hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[item_record(300,"Ordinary",301,1,1,0)]))
	await offer_first()
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_TRADE_ADD_FAIL,0,0,0,0)) and hud.hud_snapshot().bag_records.size()==1,"提交失败恢复"): return
	await offer_first()
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_TRADE_ADD_OK,0,0,0,0)) and hud.hud_snapshot().bag_records.is_empty(),"整件提交"): return
	if not check(hud.apply_game_message(peer_packet(Mir3HudView.SM_TRADE_PEER_ADD,500)),"对方物品加入"): return
	var before: Array = hud.hud_snapshot().trade_records
	var bad := peer_packet(Mir3HudView.SM_TRADE_PEER_ADD,501)
	if not check(not hud.apply_game_message(bad.slice(0,bad.size()-2)) and hud.hud_snapshot().trade_records==before,"畸形物品不部分变更"): return
	hud.apply_game_message(peer_packet(Mir3HudView.SM_TRADE_PEER_REMOVE,500,"Other"))
	if not check(hud.hud_snapshot().trade_records==before,"删除同时匹配编号与名称"): return
	hud.apply_game_message(peer_packet(Mir3HudView.SM_TRADE_PEER_REMOVE,500))
	hud.apply_game_message(peer_packet(Mir3HudView.SM_TRADE_PEER_ADD,500))
	hud.apply_game_message(game_reply(Mir3HudView.SM_GOLDCHANGED,1000,0,0,0))
	click_at(Vector2(81,288),true)
	click_at(Vector2(81,288),false)
	key(0,50)
	key(0,48)
	key(0,48)
	key(KEY_ENTER)
	if not check(sent[-1]==expected(Mir3HudView.CM_TRADE_GOLD,200),"金币完整请求"): return
	hud.apply_game_message(game_reply(Mir3HudView.SM_TRADE_PEER_GOLD,300,0,0,0))
	if not check(hud.hud_snapshot().trade_gold_pending,"对方金币不结束本方待确认"): return
	hud.apply_game_message(game_reply(Mir3HudView.SM_TRADE_GOLD,200,800,0,0))
	if not check(hud.hud_snapshot().inventory_gold==800 and hud.hud_snapshot().trade_gold_remote==300,"双方金币与余额"): return
	await settle()
	if not await capture("trade-wire-offers"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_TRADE_CANCELLED,0,0,0,0)) and hud.hud_snapshot().inventory_gold==1000 and hud.hud_snapshot().bag_records.size()==1 and not hud.hud_snapshot().trade_visible,"681返还非堆叠物品和本方金币"): return
	hud.apply_game_message(open_packet())
	hud.apply_game_message(peer_packet(Mir3HudView.SM_TRADE_PEER_ADD,600))
	click_at(Vector2(238,285),true)
	click_at(Vector2(238,285),false)
	if not check(sent[-1]==expected(Mir3HudView.CM_TRADE_CONFIRM) and hud.hud_snapshot().trade_confirmed,"确认完整原包"): return
	hud.apply_game_message(packet(Mir3HudView.SM_ADDITEM,[item_record(600,"Peer",301,1,1,0)]))
	var delivered: Array = hud.hud_snapshot().bag_records
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_TRADE_COMPLETE,99999,0,0,0)) and hud.hud_snapshot().bag_records==delivered and hud.hud_snapshot().inventory_gold==1000 and not hud.hud_snapshot().trade_visible,"687不复制对方物品或误用recognition余额"): return
	await settle()
	if not await capture("trade-wire-completed"): return
	hud.apply_game_message(open_packet())
	for i in range(12):
		if not check(hud.apply_game_message(peer_packet(Mir3HudView.SM_TRADE_PEER_ADD,700+i)),"十二条记录容量"): return
	if not check(not hud.apply_game_message(peer_packet(Mir3HudView.SM_TRADE_PEER_ADD,800)) and hud.hud_snapshot().trade_item_counts[1]==12,"空格尚多也禁止第十三件"): return
	hud.apply_game_message(game_reply(Mir3HudView.SM_TRADE_COMPLETE,0,0,0,0))
	hud.apply_game_message(open_packet())
	hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[item_record(900,"Cancel stack",301,5,1,3)]))
	await offer_first(true)
	hud.apply_game_message(game_reply(Mir3HudView.SM_TRADE_ADD_OK,901,3,0,0))
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_TRADE_CANCELLED,0,0,0,0)) and hud.hud_snapshot().bag_records.size()==1 and hud.hud_snapshot().bag_records[0].count==3,"取消已提交堆叠不模拟新增，等待独立权威更新"): return
	hud.apply_game_message(open_packet())
	await offer_first(true)
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_TRADE_CANCELLED,0,0,0,0)) and hud.hud_snapshot().bag_records.size()==1 and hud.hud_snapshot().bag_records[0].count==3 and hud.hud_snapshot().held_inventory_icon==-1,"等待增加确认时取消恢复暂存物品"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_TRADE_OPEN_FAIL,0,0,0,0)) and hud.hud_snapshot().group_request==1434,"交易失败原提示"): return
	key(KEY_ENTER)
	hud.queue_free()
	await create_timer(0.1).timeout
	print("TRADE_WIRE_UI_OK")
	quit()
