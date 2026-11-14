# 测试内容：背包使用完整原包与计时参数、等待确认锁定、成功数量及提示、失败恢复与特殊弹框、末件移除、金币禁用和栗子名称规范化。
extends "inventory_wire.gd"

func expected_use(id: int, tick: int, name: PackedByteArray, sequence: int) -> PackedByteArray:
	var raw := PackedByteArray([id&255,(id>>8)&255,(id>>16)&255,(id>>24)&255,Mir3HudView.CM_EAT&255,Mir3HudView.CM_EAT>>8,tick&255,(tick>>8)&255,0,0,(tick>>16)&255,(tick>>24)&255])
	var bytes := ("#"+str(sequence)).to_ascii_buffer()
	bytes.append_array(encode_scene_bytes(raw))
	bytes.append_array(encode_scene_bytes(name))
	bytes.append(33)
	return bytes

func named_record(name: PackedByteArray) -> PackedByteArray:
	var raw := PackedByteArray()
	raw.resize(100)
	for i in range(name.size()): raw[i]=name[i]
	raw[30]=1
	raw[36]=301&255
	raw[37]=301>>8
	raw[62]=55
	raw[71]=1
	return encode_scene_bytes(raw)

func use_first() -> void:
	await create_timer(0.31).timeout
	var point := Vector2(hud.hud_snapshot().inventory_position)+Vector2(36,79)
	click_at(point,false)
	click_at(point,false)

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("options_path","user://inventory-use-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	var packets: Array[PackedByteArray] = []
	var ticks: Array[int] = []
	hud.client_packet_ready.connect(func(bytes: PackedByteArray): packets.append(bytes))
	hud.inventory_use_requested.connect(func(_id: int, tick: int, _name: String): ticks.append(tick))
	hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[item_record(0x89abcdef,"Stack",301,3,1,3)]))
	key(KEY_W)
	await use_first()
	if not check(hud.hud_snapshot().inventory_use_pending and packets==[expected_use(0x89abcdef,ticks[0],"Stack".to_ascii_buffer(),1)],"使用完整原包及计时拆分"): return
	click_at(Vector2(hud.hud_snapshot().inventory_position)+Vector2(36,79),false)
	if not check(packets.size()==1,"等待确认期间不重复发送"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_EAT_OK,999,2,123,456)),"成功取参数一数量"): return
	if not check(hud.hud_snapshot().bag_records[0].count==2 and hud.hud_snapshot().chat_last_text=="使用Stack物品。","剩余数量和原资源提示"): return
	await settle()
	if not await capture("inventory-use-success"): return
	await use_first()
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_EAT_FAIL,0,0,0,0)) and hud.hud_snapshot().bag_records[0].count==2,"失败不消耗数量"): return
	hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[item_record(9,"Unreadable",301,1,51,0)]))
	await use_first()
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_EAT_FAIL,0,0,0,0)) and hud.hud_snapshot().group_request==1414 and hud.hud_snapshot().bag_records[0].id==9,"模式51失败恢复并弹框"): return
	await settle()
	if not await capture("inventory-use-failure"): return
	key(KEY_ENTER)
	if not check(hud.hud_snapshot().group_request==0,"确认关闭特殊失败提示"): return
	hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[item_record(10,"Last",301,1,1,3)]))
	await use_first()
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_EAT_OK,0,9,0,0)) and hud.hud_snapshot().bag_records.is_empty(),"末件按原持有数量删除"): return
	if not check(not hud.apply_game_message(game_reply(Mir3HudView.SM_EAT_OK,0,9,0,0)),"重复确认不重复提示"): return
	var chestnut := PackedByteArray([0xc0,0xf5,0xd7,0xd3,49])
	hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[named_record(chestnut)]))
	await use_first()
	if not check(packets[4]==expected_use(55,ticks[4],chestnut,5),"中文名称按原GBK发送"): return
	hud.apply_game_message(game_reply(Mir3HudView.SM_EAT_OK,0,0,0,0))
	if not check(hud.hud_snapshot().chat_last_text=="使用栗子物品。","栗子数字后缀提示规范化"): return
	hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[named_record(PackedByteArray([0xbd,0xf0,0xb1,0xd2]))]))
	await use_first()
	if not check(packets.size()==5 and not hud.hud_snapshot().inventory_use_pending and hud.hud_snapshot().held_inventory_icon==301,"金币禁止使用且保留持有"): return
	hud.queue_free()
	await create_timer(0.1).timeout
	print("INVENTORY_USE_UI_OK")
	quit()
