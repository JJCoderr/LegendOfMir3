# 测试内容：背包原始列表替换、物品增删更新、编号与名称联合匹配、计数与耐久区分、异常包原子拒绝及原版提示显示。
extends "login_smoke.gd"

func item_record(id: int, name: String, icon: int, count: int = 1, mode: int = 0, stack: int = 3) -> PackedByteArray:
	var raw := PackedByteArray()
	raw.resize(100)
	var bytes := name.to_ascii_buffer()
	for i in range(mini(bytes.size(),29)): raw[i]=bytes[i]
	raw[30]=mode
	raw[36]=icon&255
	raw[37]=(icon>>8)&255
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
	var before: Array = hud.hud_snapshot().bag_records
	if not check(not hud.apply_game_message(full.slice(0,full.size()-2)) and hud.hud_snapshot().bag_records==before,"截断列表不部分替换"): return
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_UPDATEITEM,[item_record(88,"Other",301,99)])) and hud.hud_snapshot().bag_records==before,"编号相同名称不同不更新"): return
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_UPDATEITEM,[item_record(88,"Potion",301,18)])),"更新已有物品"): return
	rows=hud.hud_snapshot().bag_records
	if not check(rows.size()==3 and rows[2].count==18 and rows[0].count==265,"只更新联合匹配记录"): return
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_DELITEM,[item_record(88,"Other",301)])) and hud.hud_snapshot().bag_records.size()==3,"删除同编号异名不命中"): return
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_DELITEM,[first])) and hud.hud_snapshot().bag_records.size()==2,"删除原物品"): return
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_UPDATEITEM,[item_record(88,"Potion",301,18)])),"更新重新寻找空位"): return
	rows=hud.hud_snapshot().bag_records
	if not check(rows[0].id==88 and rows[0].anchor==0,"更新重新排列至第一个空槽和空格"): return
	await settle()
	if not await capture("inventory-wire-updated"): return
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[])) and hud.hud_snapshot().bag_records.is_empty(),"空列表清除背包"): return
	print("INVENTORY_WIRE_UI_OK")
	quit()
