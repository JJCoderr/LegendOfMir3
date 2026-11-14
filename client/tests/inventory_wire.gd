# 测试内容：背包原始列表替换、物品增删更新、编号与名称联合匹配、计数与耐久区分、异常包原子拒绝及原版提示显示。
extends "login_smoke.gd"

func item_record(id: int, name: String, icon: int, count: int = 1, mode: int = 0, stack: int = 3, ground_look: int = 0) -> PackedByteArray:
	var raw := PackedByteArray()
	raw.resize(100)
	var bytes := name.to_ascii_buffer()
	for i in range(mini(bytes.size(),29)): raw[i]=bytes[i]
	raw[30]=mode
	raw[36]=icon&255
	raw[37]=(icon>>8)&255
	raw.encode_u16(48,ground_look)
	raw[61]=stack
	for i in range(4): raw[62+i]=(id>>(8*i))&255
	raw[66]=0x34
	raw[67]=0x12
	raw[71]=count&255
	raw[72]=(count>>8)&255
	return encode_scene_bytes(raw)

func packet(opcode: int, records: Array[PackedByteArray]) -> PackedByteArray:
	var bytes := game_reply(opcode,0,0,0,records.size())
	for row in records:
		bytes.append_array(row)
		if opcode==Mir3HudView.SM_BAGITEMS: bytes.append(47)
	return bytes

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("options_path","user://inventory-wire-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	var first := item_record(0x89abcdef,"Potion",301,265)
	var sword := item_record(23,"Sword",1043,1,5,0)
	var full := packet(Mir3HudView.SM_BAGITEMS,[first,sword])
	if not check(hud.apply_game_message(full),"完整背包原包"): return
	var rows: Array = hud.hud_snapshot().bag_records
	if not check(rows.size()==2 and rows[0].id==0x89abcdef and rows[0].count==265 and rows[0].durability==0x1234 and rows[0].anchor==0,"记录偏移、无符号编号与首个可用位置"): return
	key(KEY_W)
	await settle()
	if not check(hud.hud_snapshot().inventory_visible,"打开背包"): return
	if not await capture("inventory-wire-list"): return
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_ADDITEM,[item_record(88,"Potion",301,9)])),"增加同名不同编号物品"): return
	if not check(hud.hud_snapshot().bag_records.size()==3 and hud.hud_snapshot().chat_last_text.contains("Potion") and hud.hud_snapshot().chat_last_text.contains("9"),"新增数量提示"): return
	if not check(hud.hud_snapshot().chat_last_foreground==0xb4ffb4 and hud.hud_snapshot().chat_last_background==0,
		"获得物品提示使用原版系统消息配色"): return
	var before: Array = hud.hud_snapshot().bag_records
	if not check(not hud.apply_game_message(full.slice(0,full.size()-2)) and hud.hud_snapshot().bag_records==before,"截断列表不部分替换"): return
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_UPDATEITEM,[item_record(88,"Other",301,99)])) and hud.hud_snapshot().bag_records==before,"编号相同名称不同不更新"): return
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_UPDATEITEM,[item_record(88,"Potion",301,18)])),"更新已有物品"): return
	rows=hud.hud_snapshot().bag_records
	if not check(rows.size()==3 and rows[2].count==18 and rows[0].count==265,"只更新联合匹配记录"): return
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_DELITEM,[item_record(88,"Other",301)])) and hud.hud_snapshot().bag_records.size()==3,"删除同编号异名不命中"): return
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_DELITEM,[first])) and hud.hud_snapshot().bag_records.size()==2,"删除原物品"): return
	if not check(hud.hud_snapshot().chat_last_text.contains("Potion")
		and hud.hud_snapshot().chat_last_foreground==0xb4ffb4 and hud.hud_snapshot().chat_last_background==0,
		"移除物品提示使用原版系统消息配色"): return
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_UPDATEITEM,[item_record(88,"Potion",301,18)])),"更新重新寻找空位"): return
	rows=hud.hud_snapshot().bag_records
	if not check(rows[0].id==88 and rows[0].anchor==0,"更新重新排列至第一个空槽和空格"): return
	await settle()
	if not await capture("inventory-wire-updated"): return
	var held_cell := Vector2(543,70)
	click_at(held_cell,false)
	if not check(hud.hud_snapshot().held_inventory_icon==301,"拿起待更新的药品堆叠"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_INVENTITEM,88,21,0,0)),"接收手持物品数量更新"): return
	# 等过双击使用窗口，再按普通放回路径验收。
	await settle(0.35)
	click_at(held_cell,false)
	if not check(hud.hud_snapshot().bag_records[0].id==88 and hud.hud_snapshot().bag_records[0].count==21,
		"放回背包时保留回包更新后的手持数量"): return
	var player: Vector2i=hud.hud_snapshot().player_cell
	var camera: Vector2i=hud.hud_snapshot().map_camera_pixel
	var self_point := Vector2(player*Vector2i(48,32)-camera+Vector2i(24,16))
	var drop_packets: Array[PackedByteArray] = []
	hud.client_packet_ready.connect(func(bytes: PackedByteArray): drop_packets.append(bytes))
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[item_record(88,"Potion",301,1,0,0)])),
		"建立普通扔出物品夹具"): return
	for attempt in range(2):
		await settle(0.35)
		click_at(held_cell,false)
		click_at(self_point,true)
		click_at(self_point,false)
		if not check(drop_packets.size()==attempt+1 and hud.hud_snapshot().held_inventory_icon<0,
			"点击地面发送一次扔出请求并清空光标"): return
		var head := encode_scene_bytes(PackedByteArray([88,0,0,0,233,3]))
		var body := encode_scene_bytes("Potion".to_ascii_buffer())
		if not check(drop_packets.back().slice(2,10)==head and drop_packets.back().slice(18,-1)==body,
			"验证1001请求的物品编号和名称"): return
		if attempt==0:
			var failed := game_reply(Mir3HudView.SM_DROPITEM_FAIL,88,0,0,0)
			failed.append_array(body)
			if not check(hud.apply_game_message(failed) and hud.hud_snapshot().bag_records.size()==1,
				"601恢复原物品"): return
		elif not check(hud.apply_game_message(game_reply(Mir3HudView.SM_DROPITEM_SUCCESS,88,0,0,0))
			and hud.hud_snapshot().bag_records.is_empty(),"600提交背包移除"): return
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[item_record(88,"Potion",301,150)])),
		"加载150件堆叠"): return
	await settle(0.35)
	click_at(held_cell,false)
	click_at(self_point,true)
	click_at(self_point,false)
	if not check(hud.hud_snapshot().group_request==104,"多件堆叠打开扔出数量框"): return
	hud.get_node("ChatInput").text="150"
	key(KEY_ENTER)
	var stack_head := encode_scene_bytes(PackedByteArray([88,0,0,0,77,4,0,0,0,0,100,0]))
	if not check(drop_packets.size()==3 and drop_packets.back().slice(2,18)==stack_head
		and hud.hud_snapshot().bag_records[0].count==150,"1101最多扔出100件并等待数量更新"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_INVENTITEM,88,50,0,0))
		and hud.hud_snapshot().bag_records[0].count==50,"1313同步堆叠剩余50件"): return
	await settle(0.35)
	click_at(held_cell,false)
	click_at(self_point,true)
	click_at(self_point,false)
	hud.get_node("ChatInput").text="*"
	key(KEY_ENTER)
	stack_head=encode_scene_bytes(PackedByteArray([88,0,0,0,77,4,0,0,0,0,50,0]))
	if not check(drop_packets.size()==4 and drop_packets.back().slice(2,18)==stack_head
		and hud.apply_game_message(game_reply(Mir3HudView.SM_DROPITEM_SUCCESS,88,0,0,0))
		and hud.hud_snapshot().bag_records.is_empty(),"扔出全部剩余堆叠"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_GOLDCHANGED,1000,0,0,0)),"同步初始金币"): return
	for amount in [1000,300]:
		click_at(Vector2(hud.hud_snapshot().inventory_position)+Vector2(32,400),false)
		if not check(hud.hud_snapshot().held_inventory_icon==124,"点取背包金币图标"): return
		click_at(self_point,true)
		click_at(self_point,false)
		if not check(hud.hud_snapshot().group_request==102,"打开扔出金币金额框"): return
		hud.get_node("ChatInput").text=str(amount)
		key(KEY_ENTER)
		if amount==1000 and not check(drop_packets.size()==4,"金额等于余额时拒绝扔出"): return
	var gold_head := encode_scene_bytes(PackedByteArray([44,1,0,0,248,3,0,0,0,0,0,0]))
	if not check(drop_packets.size()==5 and drop_packets.back().slice(2,18)==gold_head
		and hud.hud_snapshot().inventory_gold==1000,"1016携带金额且本地余额等待回包"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_GOLDCHANGED,700,0,0,0))
		and hud.hud_snapshot().inventory_gold==700,"653同步扔出后的余额"): return
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[])) and hud.hud_snapshot().bag_records.is_empty(),"空列表清除背包"): return
	key(KEY_W)
	var full_bag: Array[PackedByteArray] = []
	for index in range(46): full_bag.append(item_record(1000+index,"Potion",301,1,0,3,1))
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,full_bag)),"装满46条背包记录"): return
	var pickups: Array[int] = []
	hud.pickup_requested.connect(func(op: int, _cell: Vector2i): pickups.append(op))
	for fixture in [["Potion",1,0],["3/Potion",2,0],["3/Potion",1,1],["gold",1,2]]:
		var drop := game_reply(Mir3HudView.SM_ITEMSHOW,103,player.x,player.y,fixture[1])
		var label: PackedByteArray = PackedByteArray([0xbd,0xf0,0xb1,0xd2]) if fixture[0]=="gold" else fixture[0].to_ascii_buffer()
		drop.append_array(encode_scene_bytes(label))
		if not check(hud.apply_game_message(drop),"放置满背包拾取夹具"): return
		click_at(self_point,true)
		click_at(self_point,false)
		if not check(pickups.size()==fixture[2],"满背包允许拾取金币和图号匹配的堆叠"): return
		if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_ITEMHIDE,103,0,0,0)),"移除拾取夹具"): return
	hud.queue_free()
	await create_timer(0.1).timeout
	print("INVENTORY_WIRE_UI_OK")
	quit()
