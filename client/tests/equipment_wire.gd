# 测试内容：装备槽位原包覆盖与保留、重复槽位、畸形包原子拒绝、普通和特殊武器耐久取整提示、删除与更新重排及装备窗口显示。
extends "inventory_wire.gd"

func equipment_packet(entries: Array) -> PackedByteArray:
	var bytes := game_reply(Mir3HudView.SM_SENDUSEITEMS,0,0,0,65535)
	for entry in entries:
		bytes.append_array((str(entry[0])+"/").to_ascii_buffer())
		bytes.append_array(entry[1])
		bytes.append(47)
	return bytes

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("options_path","user://equipment-wire-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	var sword := item_record(101,"Sword",1043,1,5,0)
	var necklace := item_record(102,"Necklace",1520,1,20,0)
	var full := equipment_packet([[1,sword],[3,necklace]])
	if not check(hud.apply_game_message(full) and hud.hud_snapshot().equipment_records.size()==2,"装备列表忽略头计数，按正文槽位覆盖"): return
	var rows: Array = hud.hud_snapshot().equipment_records
	if not check(rows[0].slot==1 and rows[0].id==101 and rows[1].slot==3,"装备槽位与编号"): return
	key(KEY_Q)
	await settle()
	if not check(hud.hud_snapshot().status_visible,"打开人物装备窗口"): return
	if not await capture("equipment-wire-list"): return
	if not check(hud.apply_game_message(equipment_packet([])) and hud.hud_snapshot().equipment_records==rows,"空装备列表不清除现有槽位"): return
	if not check(not hud.apply_game_message(full.slice(0,full.size()-1)) and hud.hud_snapshot().equipment_records==rows,"畸形装备列表原子拒绝"): return
	if not check(hud.apply_game_message(equipment_packet([[3,item_record(103,"New",1520,1,20,0)],[3,necklace]])) and hud.hud_snapshot().equipment_records==rows,"重复槽位以后者覆盖，其它槽位保留"): return
	var notices: int = hud.hud_snapshot().chat_compact_count
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_DURACHANGE,500,1,6000,0)) and hud.hud_snapshot().chat_compact_count==notices,"普通装备500不警告"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_DURACHANGE,499,1,6000,0)),"普通装备低耐久"): return
	rows=hud.hud_snapshot().equipment_records
	if not check(rows[0].durability==499 and rows[0].max_durability==6000 and hud.hud_snapshot().chat_compact_count==notices+1 and hud.hud_snapshot().chat_last_text.contains("Sword"),"耐久字段及原资源警告"): return
	await settle()
	if not await capture("equipment-wire-durability"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_DURACHANGE,65536,1,7000,0)) and hud.hud_snapshot().equipment_records[0].durability==0 and hud.hud_snapshot().chat_compact_count==notices+1,"完整recognition判断，不使用截断后的耐久"): return
	if not check(hud.apply_game_message(equipment_packet([[1,item_record(101,"Sword",1043,1,54,0)]])),"特殊武器记录"): return
	hud.apply_game_message(game_reply(Mir3HudView.SM_DURACHANGE,50,1,6000,0))
	if not check(hud.hud_snapshot().chat_compact_count==notices+1,"特殊武器50不警告"): return
	hud.apply_game_message(game_reply(Mir3HudView.SM_DURACHANGE,49,1,6000,0))
	if not check(hud.hud_snapshot().chat_compact_count==notices+2,"特殊武器49警告"): return
	rows=hud.hud_snapshot().equipment_records
	hud.apply_game_message(game_reply(Mir3HudView.SM_DURACHANGE,0,12,6000,0))
	hud.apply_game_message(game_reply(Mir3HudView.SM_DURACHANGE,0,11,6000,0))
	if not check(hud.hud_snapshot().equipment_records==rows and hud.hud_snapshot().chat_compact_count==notices+2,"无效槽位和空槽忽略"): return
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_UPDATEITEM,[necklace])) and hud.hud_snapshot().equipment_records[0].slot==0 and hud.hud_snapshot().equipment_records[0].id==102,"203更新装备使用第一个空槽"): return
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_DELITEM,[necklace])) and hud.hud_snapshot().equipment_records.size()==1,"202删除匹配装备"): return
	print("EQUIPMENT_WIRE_UI_OK")
	quit()
