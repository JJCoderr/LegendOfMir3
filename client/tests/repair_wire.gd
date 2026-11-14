# 测试内容：修理开窗、完整报价与付款包、无效报价、失败重试、耐久金币和取消。
extends "inventory_wire.gd"

var packets: Array[PackedByteArray] = []

func repair_click(position: Vector2) -> void:
	click_at(position,true)
	click_at(position,false)

func repair_request(opcode: int, sequence: int) -> PackedByteArray:
	var bytes := ("#%d" % sequence).to_ascii_buffer()
	bytes.append_array(encode_scene_bytes(PackedByteArray([91,0,0,0,opcode&255,opcode>>8,0x78,0x56,0x34,0x12,0,0])))
	bytes.append_array(encode_scene_bytes("Sword".to_ascii_buffer()))
	bytes.append(33)
	return bytes

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.client_packet_ready.connect(func(bytes: PackedByteArray): packets.append(bytes))
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[item_record(0x12345678,"Sword",1043,1,5,0)])),"加载原包物品"): return
	var open := game_reply(Mir3HudView.SM_OPEN_REPAIR,91,0,0,0)
	if not check(hud.apply_game_message(open),"接收商人修理入口"): return
	var row: Dictionary = hud.hud_snapshot().bag_records[0]
	var slot: int = row.anchor
	var cell := Vector2(518+25+slot%6*38,70+int(slot/6)*38)
	var confirm := Vector2(720,405)
	repair_click(cell)
	repair_click(confirm)
	if not check(packets.size()==1 and packets[0]==repair_request(Mir3HudView.CM_MERCHANTQUERYREPAIRCOST,1),"等待报价期间只发1024"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_REPAIR_QUOTE,0,0,0,0)),"零报价"): return
	repair_click(confirm)
	if not check(packets.size()==1,"零报价不能付款"): return
	repair_click(cell)
	var quote := game_reply(Mir3HudView.SM_REPAIR_QUOTE,120,0,0,0)
	if not check(hud.apply_game_message(quote),"有效报价"): return
	if not await capture("npc-repair-quote"): return
	repair_click(confirm)
	repair_click(confirm)
	if not check(packets.size()==3 and packets[-1]==repair_request(Mir3HudView.CM_USERREPAIRITEM,3),"确认只发送一次1023"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_REPAIR_FAIL,0,0,0,0)) and hud.hud_snapshot().group_request==1457,"失败提示"): return
	key(KEY_ENTER)
	repair_click(cell)
	if not check(hud.apply_game_message(quote),"失败可重新报价"): return
	repair_click(confirm)
	var success := game_reply(Mir3HudView.SM_REPAIR_OK,880,5500,6000,0)
	if not check(hud.apply_game_message(success),"修理成功"): return
	var state: Dictionary = hud.hud_snapshot()
	if not check(state.inventory_gold==880 and state.bag_records[0].durability==5500,"成功同步金币耐久"): return
	if not await capture("npc-repair-success"): return
	repair_click(cell)
	repair_click(Vector2(768,430))
	if not check(hud.apply_game_message(quote),"关闭后处理迟到报价"): return
	if not check(hud.apply_game_message(open),"重新开窗"): return
	var sent := packets.size()
	repair_click(confirm)
	if not check(packets.size()==sent,"重新开窗没有旧报价"): return
	hud.queue_free()
	await create_timer(0.1).timeout
	print("NPC_REPAIR_UI_OK")
	quit()
