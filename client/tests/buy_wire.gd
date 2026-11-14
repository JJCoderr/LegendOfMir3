# 测试内容：买入回包的完整金额、缺货与其他失败、详情移除、独立物品入包和关窗后回包。
extends "inventory_wire.gd"

var buy_packets: Array[PackedByteArray] = []

func buy_click(position: Vector2) -> void:
	click_at(position,true)
	click_at(position,false)

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.client_packet_ready.connect(func(bytes: PackedByteArray): buy_packets.append(bytes))
	if not check(hud.apply_npc_dialogue(Mir3HudView.SM_MERCHANTSAY,91,"Merchant/<Buy/@buy>"),"开启商人对话"): return
	var goods := game_reply(Mir3HudView.SM_SENDGOODSLIST,91,1,0,0)
	goods.append_array(encode_scene_bytes("Potion/1/25/1/301/0/0/".to_ascii_buffer()))
	if not check(hud.apply_game_message(goods),"打开商品列表原包"): return
	buy_click(Vector2(130,419))
	var offers := game_reply(Mir3HudView.SM_SENDDETAILGOODSLIST,91,3,0,0)
	var payload := PackedByteArray()
	for id in [10001,10002,10003]:
		payload.append_array(item_record(id,"Potion",301))
		payload.append(47)
	offers.append_array(encode_scene_bytes(payload))
	# 47974D 以回复偏移为准，即使该页为空。
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_SENDDETAILGOODSLIST,91,0,20,0))
		and hud.hud_snapshot().shop_offers_count==0,"接受非请求偏移的空详情页"): return
	buy_click(Vector2(334,399))
	var page_header := PackedByteArray([91,0,0,0,Mir3HudView.CM_USERGETDETAILITEM&255,Mir3HudView.CM_USERGETDETAILITEM>>8,10,0,0,0,0,0])
	if not check(buy_packets.size()==2 and buy_packets.back().slice(2,18)==encode_scene_bytes(page_header),
		"上一页从回复指定的第二页发出偏移10请求"): return
	if not check(hud.apply_game_message(offers) and hud.hud_snapshot().shop_offers_count==3,"解析嵌套商品详情"): return
	var confirm := Vector2(378,399)
	buy_click(Vector2(368,280))
	buy_click(confirm)
	var expected := "#3".to_ascii_buffer()
	expected.append_array(encode_scene_bytes(PackedByteArray([91,0,0,0,Mir3HudView.CM_USERBUYITEM&255,Mir3HudView.CM_USERBUYITEM>>8,0x12,0x27,0,0,0,0])))
	expected.append_array(encode_scene_bytes("Potion".to_ascii_buffer()))
	expected.append(33)
	if not check(buy_packets.size()==3 and buy_packets[-1]==expected,"确认发送选中编号"): return
	var success := game_reply(Mir3HudView.SM_BUYITEM_SUCCESS,0x12345678,7,0,0)
	if not check(hud.apply_game_message(success),"识别号携带完整余额"): return
	var state: Dictionary = hud.hud_snapshot()
	if not check(state.inventory_gold==0x12345678 and state.shop_offers_count==2 and state.bag_records.is_empty(),"成功移除选中详情并等待独立背包包"): return
	if not check(hud.apply_game_message(packet(Mir3HudView.SM_ADDITEM,[item_record(10002,"Potion",301)])),"单独入包"): return
	buy_click(confirm)
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_BUYITEM_FAIL,2,1,0,0)) and hud.hud_snapshot().shop_offers_count==2 and hud.hud_snapshot().group_request==1438,"失败2保留详情"): return
	key(KEY_ENTER)
	buy_click(confirm)
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_BUYITEM_FAIL,1,2,0,0)) and hud.hud_snapshot().shop_offers_count==1 and hud.hud_snapshot().group_request==1437,"缺货失败移除详情"): return
	if not await capture("npc-buy-sold-out"): return
	key(KEY_ENTER)
	key(KEY_W)
	if not await capture("npc-buy-stock-and-bag"): return
	buy_click(Vector2(335,280))
	buy_click(Vector2(427,399))
	if not check(hud.hud_snapshot().shop_offer_loading,"关闭前已请求下一页"): return
	if not check(hud.apply_game_message(success) and hud.hud_snapshot().shop_offers_count==0
		and hud.hud_snapshot().shop_offer_loading,"等待下一页时仍移除原页面的选中条目"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_MERCHANTDLGCLOSE,91,0,0,0))
		and not hud.hud_snapshot().shop_offer_loading,"关闭回包清除交易页等待状态"): return
	if not check(not hud.apply_game_message(offers),"关闭后丢弃迟到的详情页"): return
	if not check(hud.apply_game_message(success) and hud.hud_snapshot().shop_offers_count==0,"关闭后回包不会恢复旧窗口"): return
	await process_frame
	hud.queue_free()
	await create_timer(0.1).timeout
	print("NPC_BUY_UI_OK")
	quit()
