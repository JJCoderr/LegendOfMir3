# 测试内容：返回登录和角色选择时清理背包、装备、持有及待确认物品、交易双方物品和金币、商店数据，并拒绝旧角色的迟到确认。
extends "equipment_wire.gd"

func clean_items(state: Dictionary) -> bool:
	return state.bag_records.is_empty() and state.equipment_records.is_empty() and state.held_inventory_icon==-1 and not state.inventory_use_pending and not state.equipment_pending and state.inventory_gold==0 and state.trade_gold_local==0 and state.trade_gold_remote==0 and state.trade_item_counts==PackedInt32Array([0,0]) and state.trade_partner.is_empty() and not state.trade_visible and not state.trade_confirmed and state.trade_return_slot==-1 and not state.trade_gold_pending and state.shop_goods_count==0 and state.shop_offers_count==0 and not state.shop_offer_loading and state.npc_dialogue_actor==0 and state.inventory_row==0

func run() -> void:
	Input.use_accumulated_input=false
	for scenario in range(4):
		hud=load("res://main.tscn").instantiate()
		hud.set_meta("offline_preview",true)
		hud.set_meta("show_terrain",scenario==0 or scenario==3)
		hud.set_meta("options_path","user://item-session-%d-%d.ini" % [OS.get_process_id(),scenario])
		hud.set_connection_enabled(false)
		root.add_child(hud)
		await settle()
		hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[item_record(100,"Bag",301,3,1,3),item_record(101,"Necklace",1520,1,20,0)]))
		hud.apply_game_message(equipment_packet([[1,item_record(102,"Sword",1043,1,5,0)]]))
		hud.apply_game_message(game_reply(Mir3HudView.SM_GOLDCHANGED,12345,0,0,0))
		key(KEY_W)
		await create_timer(0.31).timeout
		var point := Vector2(hud.hud_snapshot().inventory_position)+Vector2(36,79)
		if scenario==0:
			click_at(point,false)
			click_at(point,false)
			if not check(hud.hud_snapshot().inventory_use_pending,"构造等待使用确认"): return
		elif scenario==1:
			key(KEY_Q)
			click_at(point+Vector2(38,0),false)
			await create_timer(0.31).timeout
			click_at(Vector2(hud.hud_snapshot().status_position)+Vector2(185,100),false)
			if not check(hud.hud_snapshot().equipment_pending,"构造等待穿戴确认"): return
		elif scenario==2:
			if not check(hud.open_trade("Old partner") and hud.set_trade_item(0,0,301,1,201,"Local",1,0) and hud.set_trade_item(1,0,301,1,202,"Remote",1,0) and hud.set_trade_gold(500,600),"构造交易双方物品和金币"): return
			if not check(hud.apply_npc_dialogue(Mir3HudView.SM_MERCHANTSAY,2,"Merchant/Shop") and hud.apply_shop_goods(2,1,"Goods/1/100/2/"),"构造商店列表"): return
		else:
			click_at(point,false)
			if not check(hud.hud_snapshot().held_inventory_icon==301,"构造普通持有物品"): return
		if scenario==0:
			await settle()
			if not await capture("item-session-before-exit"): return
		if scenario==3:
			key(KEY_X,0,true)
			key(KEY_ENTER)
			var deadline := Time.get_ticks_msec()+4500
			while hud.hud_snapshot().client_page==3 and Time.get_ticks_msec()<deadline: await process_frame
			if not check(hud.hud_snapshot().client_page==2,"退出过渡进入角色选择"): return
		else:
			if not check(hud.open_login(),"返回账号登录"): return
		if not check(clean_items(hud.hud_snapshot()),"完整清理物品会话 %d" % scenario): return
		if not check(not hud.apply_inventory_use_reply(Mir3HudView.SM_EAT_OK,2) and not hud.apply_equipment_reply(Mir3HudView.SM_TAKEON_OK) and not hud.apply_trade_complete(9999),"迟到确认不能复活旧角色物品"): return
		if not check(not hud.apply_game_message(packet(Mir3HudView.SM_ADDITEM,[item_record(999,"Late",301)])) and clean_items(hud.hud_snapshot()),"非游戏页拒绝旧物品原包"): return
		if scenario==3:
			await settle()
			if not await capture("item-session-selection",Vector2i(640,480)): return
		hud.queue_free()
		await create_timer(0.1).timeout
	print("ITEM_SESSION_UI_OK")
	quit()
