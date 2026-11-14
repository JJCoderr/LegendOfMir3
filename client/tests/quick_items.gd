# 测试内容：快捷栏原版布局与开关、拖放引用防复制、数字键及双击完整请求、冷却锁定、使用数量与失败恢复、交易关联和离场清理。
extends "inventory_use.gd"

func wait_use_cooldown() -> void:
	var deadline := Time.get_ticks_msec()+2050
	while Time.get_ticks_msec()<deadline: await process_frame
func drop_at(point: Vector2) -> void:
	await create_timer(0.31).timeout
	click_at(point,false)
func bag_point() -> Vector2:
	return Vector2(hud.hud_snapshot().inventory_position)+Vector2(36,79)
func quick_point(slot: int) -> Vector2:
	return Vector2(hud.hud_snapshot().quick_position)+Vector2(36+slot*42,25)
func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("options_path","user://quick-items-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	var packets: Array[PackedByteArray] = []
	var ticks: Array[int] = []
	hud.client_packet_ready.connect(func(bytes: PackedByteArray): packets.append(bytes))
	hud.inventory_use_requested.connect(func(_id: int, tick: int, _name: String): ticks.append(tick))
	hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[item_record(100,"Potion",301,3,0,3)]))
	key(KEY_W)
	key(KEY_Z)
	if not check(hud.hud_snapshot().quick_visible and hud.hud_snapshot().quick_position==Vector2i(73,413),"原版位置及Z开关"): return
	await drop_at(bag_point())
	await drop_at(quick_point(0))
	if not check(hud.hud_snapshot().quick_records.size()==1 and hud.hud_snapshot().bag_records.size()==1 and hud.hud_snapshot().held_inventory_icon<0,"绑定只增加快捷引用"): return
	click_at(Vector2(quick_point(0).x,415),true)
	var motion := InputEventMouseMotion.new()
	motion.position=Vector2(quick_point(0).x+20,395)
	motion.global_position=motion.position
	Input.parse_input_event(motion)
	click_at(motion.position,false)
	if not check(hud.hud_snapshot().quick_position==Vector2i(93,393),"快捷栏背景拖动"): return
	var close_point := Vector2(hud.hud_snapshot().quick_position)+Vector2(270,38)
	click_at(close_point,true)
	click_at(close_point,false)
	if not check(not hud.hud_snapshot().quick_visible,"关闭按钮"): return
	key(KEY_Z)
	await settle()
	if not await capture("quick-items-bound"): return
	key(KEY_1)
	if not check(packets.size()==1 and packets[0]==expected_use(100,ticks[0],"Potion".to_ascii_buffer(),1) and hud.hud_snapshot().inventory_use_pending,"数字键完整使用请求"): return
	key(KEY_1)
	if not check(packets.size()==1,"等待回包不重复发送"): return
	hud.apply_game_message(game_reply(Mir3HudView.SM_EAT_OK,0,2,0,0))
	if not check(hud.hud_snapshot().bag_records[0].count==2 and hud.hud_snapshot().quick_records[0].count==2 and hud.hud_snapshot().held_quick_slot<0,"成功同步背包和快捷数量"): return
	key(KEY_1)
	if not check(packets.size()==1,"共享药品冷却"): return
	await wait_use_cooldown()
	key(KEY_1)
	if not check(packets.size()==2 and hud.hud_snapshot().inventory_use_pending,"冷却后再次数字键使用"): return
	hud.apply_game_message(game_reply(Mir3HudView.SM_EAT_FAIL,0,0,0,0))
	if not check(hud.hud_snapshot().quick_records[0].count==2 and hud.hud_snapshot().bag_records.size()==1,"失败恢复快捷引用不复制背包"): return
	await drop_at(quick_point(0))
	await drop_at(bag_point())
	if not check(hud.hud_snapshot().quick_records.is_empty() and hud.hud_snapshot().bag_records.size()==1 and hud.hud_snapshot().held_inventory_icon<0,"拖回占用背包格只清引用"): return
	await drop_at(bag_point())
	await drop_at(quick_point(0))
	await wait_use_cooldown()
	await drop_at(quick_point(0))
	if not check(hud.hud_snapshot().held_quick_slot==0,"双击前持有快捷引用"): return
	var double_click := InputEventMouseButton.new()
	double_click.button_index=MOUSE_BUTTON_LEFT
	double_click.position=quick_point(0)
	double_click.global_position=quick_point(0)
	double_click.pressed=true
	double_click.double_click=true
	Input.parse_input_event(double_click)
	click_at(quick_point(0),false)
	if not check(packets.size()==3 and hud.hud_snapshot().inventory_use_pending,"鼠标双击使用持有的快捷引用"): return
	hud.apply_game_message(game_reply(Mir3HudView.SM_EAT_OK,0,1,0,0))
	await settle()
	if not await capture("quick-items-used"): return
	key(KEY_Z)
	if not check(not hud.hud_snapshot().quick_visible,"隐藏快捷栏"): return
	await wait_use_cooldown()
	key(KEY_1)
	hud.apply_game_message(game_reply(Mir3HudView.SM_EAT_OK,0,0,0,0))
	if not check(hud.hud_snapshot().quick_records.is_empty() and hud.hud_snapshot().bag_records.is_empty(),"隐藏时数字键可用且末件清除"): return
	hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[item_record(200,"TradePotion",301,1,0,3)]))
	key(KEY_Z)
	await drop_at(bag_point())
	await drop_at(quick_point(0))
	hud.open_trade("Partner")
	await drop_at(bag_point())
	await drop_at(Vector2(32,75))
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_TRADE_ADD_OK,0,0,0,0)) and hud.hud_snapshot().quick_records.is_empty(),"完整交易提交清除快捷引用"): return
	hud.apply_game_message(game_reply(Mir3HudView.SM_TRADE_COMPLETE,0,0,0,0))
	hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[item_record(300,"SplitPotion",301,3,0,3)]))
	await drop_at(bag_point())
	await drop_at(quick_point(0))
	hud.open_trade("Partner")
	await drop_at(bag_point())
	await drop_at(Vector2(32,75))
	key(0,50)
	key(KEY_ENTER)
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_TRADE_ADD_OK,301,1,0,0)) and hud.hud_snapshot().quick_records[0].count==1,"分堆提交同步快捷剩余数量"): return
	hud.apply_game_message(packet(Mir3HudView.SM_DELITEM,[item_record(300,"SplitPotion",301,1,0,3)]))
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_TRADE_COMPLETE,0,0,0,0)) and hud.hud_snapshot().quick_records.is_empty(),"完成交易时背包已无相同图标则清快捷引用"): return
	hud.open_login()
	if not check(hud.hud_snapshot().quick_records.is_empty() and not hud.hud_snapshot().quick_visible and hud.hud_snapshot().held_quick_slot<0,"离场清空快捷状态"): return
	hud.queue_free()
	await create_timer(0.1).timeout
	print("QUICK_ITEMS_UI_OK")
	quit()
