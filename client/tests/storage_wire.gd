# 测试内容：仓库堆叠存入、满仓重试、列表解码、本地翻页、取回以及关闭。
extends "inventory_wire.gd"

var storage_packets: Array[PackedByteArray] = []

func tap(point: Vector2) -> void:
	click_at(point,true)
	click_at(point,false)

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.client_packet_ready.connect(func(bytes: PackedByteArray): storage_packets.append(bytes))
	var open := game_reply(Mir3HudView.SM_STORAGE_OPEN,91,0,0,0)
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[item_record(33,"Potion",301,5)])) and hud.apply_game_message(open),"开仓存入"): return
	var slot: int = hud.hud_snapshot().bag_records[0].anchor
	var cell := Vector2(543+slot%6*38,70+int(slot/6)*38)
	tap(cell)
	tap(Vector2(720,405))
	if not check(hud.hud_snapshot().group_request==1706,"堆叠存入数量框"): return
	hud.get_node("ChatInput").text="2"
	key(KEY_ENTER)
	if not check(storage_packets.size()==1,"提交存入"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_STORAGE_FULL,0,0,0,0)) and hud.hud_snapshot().group_request==1449,"满仓提示"): return
	key(KEY_ENTER)
	tap(cell)
	tap(Vector2(720,405))
	hud.get_node("ChatInput").text="2"
	key(KEY_ENTER)
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_STORAGE_DEPOSIT_OK,0,3,2,0)) and hud.hud_snapshot().bag_records[0].count==3,"存入成功保持剩余物品"): return
	var storage := game_reply(Mir3HudView.SM_STORAGE_LIST,91,0,0,13)
	for id in range(13):
		storage.append_array(item_record(300+id if id<12 else 33,"Potion",301,3))
		storage.append(47)
	if not check(hud.apply_game_message(storage) and hud.hud_snapshot().shop_offers_count==13,"原包仓库记录"): return
	if not await capture("npc-storage-list"): return
	tap(Vector2(135,399))
	tap(Vector2(35,280))
	tap(Vector2(87,399))
	if not check(hud.hud_snapshot().group_request==1707 and storage_packets.size()==2,"本地翻页与取回数量"): return
	hud.get_node("ChatInput").text="1"
	key(KEY_ENTER)
	var expected := "#3".to_ascii_buffer()
	var op := Mir3HudView.CM_USERTAKEBACKSTORAGEITEM
	expected.append_array(encode_scene_bytes(PackedByteArray([91,0,0,0,op&255,op>>8,33,0,0,0,1,0])))
	expected.append_array(encode_scene_bytes("Potion".to_ascii_buffer()))
	expected.append(33)
	if not check(storage_packets.size()==3 and storage_packets[-1]==expected,"第二页物品完整取回包"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_STORAGE_WITHDRAW_OK,0,2,4,0)) and hud.hud_snapshot().shop_offers_count==13 and hud.hud_snapshot().bag_records[0].count==4,"部分取回同步两侧数量"): return
	tap(Vector2(87,399))
	if not check(hud.hud_snapshot().group_request==1707,"剩余两件可继续取回"): return
	hud.get_node("ChatInput").text="*"
	key(KEY_ENTER)
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_STORAGE_WITHDRAW_OK,0,0,6,0)) and hud.hud_snapshot().shop_offers_count==12,"取回全部移除仓库项"): return
	if not await capture("npc-storage-withdraw"): return
	hud.open_login()
	if not check(hud.hud_snapshot().shop_offers_count==0,"离场清理仓库"): return
	hud.queue_free()
	await create_timer(0.1).timeout
	print("NPC_STORAGE_UI_OK")
	quit()
