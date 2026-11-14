# 测试内容：出售报价、整数数量确认、取消、失败重试、部分出售与完整移除。
extends "inventory_wire.gd"

var sell_packets: Array[PackedByteArray] = []

func sell_click(position: Vector2) -> void:
	click_at(position,true)
	click_at(position,false)

func request_bytes(opcode: int, sequence: int, quantity: int) -> PackedByteArray:
	var bytes := ("#%d" % sequence).to_ascii_buffer()
	bytes.append_array(encode_scene_bytes(PackedByteArray([91,0,0,0,opcode&255,opcode>>8,33,0,0,0,quantity,0])))
	bytes.append_array(encode_scene_bytes("Potion".to_ascii_buffer()))
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
	hud.client_packet_ready.connect(func(bytes: PackedByteArray): sell_packets.append(bytes))
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[item_record(33,"Potion",301,5)])),"加载五件药品"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_SENDUSERSELL,91,0,0,0)),"商人开启出售"): return
	var row: Dictionary = hud.hud_snapshot().bag_records[0]
	var slot: int = row.anchor
	var cell := Vector2(543+slot%6*38,70+int(slot/6)*38)
	var confirm := Vector2(720,405)
	sell_click(cell)
	if not check(sell_packets==[request_bytes(Mir3HudView.CM_MERCHANTQUERYSELLPRICE,1,0)],"完整报价请求"): return
	var quote := game_reply(Mir3HudView.SM_SENDBUYPRICE,120,0,0,0)
	if not check(hud.apply_game_message(quote),"接收报价"): return
	sell_click(confirm)
	if not check(hud.hud_snapshot().group_request==1705,"堆叠打开数量框"): return
	if not await capture("npc-sell-quantity"): return
	key(KEY_ESCAPE)
	if not check(sell_packets.size()==1,"取消不发送出售"): return
	sell_click(confirm)
	# 4256B0 允许数字前的空白，读取数字前缀后再校验数量。
	hud.get_node("ChatInput").text=" 2件"
	key(KEY_ENTER)
	if not check(sell_packets[-1]==request_bytes(Mir3HudView.CM_USERSELLITEM,2,2),"第三字段发送数量"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_USERSELLITEM_FAIL,0,0,0,0)),"失败回包"): return
	key(KEY_ENTER)
	sell_click(cell)
	if not check(hud.apply_game_message(quote),"失败后重新报价"): return
	sell_click(confirm)
	hud.get_node("ChatInput").text="2"
	key(KEY_ENTER)
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_SELL_STACK_OK,240,3,2,0)),"部分出售回包"): return
	var state: Dictionary = hud.hud_snapshot()
	if not check(state.inventory_gold==240 and state.bag_records[0].count==3,"剩余数量与金币同步"): return
	sell_click(cell)
	if not check(hud.apply_game_message(quote),"剩余物品报价"): return
	sell_click(confirm)
	hud.get_node("ChatInput").text="*"
	key(KEY_ENTER)
	if not check(sell_packets[-1]==request_bytes(Mir3HudView.CM_USERSELLITEM,6,3),"星号发送全部剩余"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_SELL_STACK_OK,600,0,3,0)),"出售全部"): return
	if not check(hud.hud_snapshot().bag_records.is_empty() and hud.hud_snapshot().inventory_gold==600,"移除原物品并更新金币"): return
	if not await capture("npc-sell-success"): return
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[item_record(33,"Potion",301,5)]))
		and hud.apply_game_message(game_reply(Mir3HudView.SM_MERCHANTDLGCLOSE,91,0,0,0)),"关窗前恢复背包夹具"): return
	for reply in [[Mir3HudView.SM_USERSELLITEM_OK,660],[Mir3HudView.SM_SELL_STACK_OK,720]]:
		if not check(hud.apply_game_message(game_reply(reply[0],reply[1],2,1,0))
			and hud.hud_snapshot().inventory_gold==reply[1] and hud.hud_snapshot().bag_records[0].count==5,
			"关窗后出售成功同步余额并保留物品"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_USERSELLITEM_FAIL,0,0,0,0))
		and hud.hud_snapshot().group_request==1455,"关窗后普通出售失败仍提示"): return
	key(KEY_ENTER)
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[item_record(33,"Potion",301,1,0,0)])),
		"加载普通可绑定物品"): return
	key(KEY_W)
	key(KEY_Z)
	click_at(Vector2(543,70),false)
	await settle(0.35)
	click_at(Vector2(hud.hud_snapshot().quick_position)+Vector2(36,25),false)
	if not check(hud.hud_snapshot().quick_records.size()==1,"绑定普通物品到快捷栏"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_SENDUSERSELL,91,0,0,0)),"重新打开出售"): return
	sell_click(cell)
	if not check(hud.apply_game_message(quote),"收到普通物品报价"): return
	sell_click(confirm)
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_USERSELLITEM_OK,780,0,0,0))
		and hud.hud_snapshot().bag_records.is_empty() and hud.hud_snapshot().quick_records.size()==1,
		"普通出售回包清背包并保留快捷引用"): return
	await process_frame
	hud.queue_free()
	await create_timer(0.1).timeout
	print("NPC_SELL_UI_OK")
	quit()
