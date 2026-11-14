# 测试内容：角色退出保存原版布局文件、背包重排恢复与服务器数量优先、同名快捷匹配、跨角色隔离和损坏文件回退。
extends "inventory_wire.gd"

var storage: String
func spawn_hud(role: String) -> void:
	root.size=Vector2i(800,600)
	root.content_scale_size=Vector2i(800,600)
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("item_layout_directory",storage)
	hud.set_meta("options_path",storage+"/Config.ini")
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.set_character_identity(role,"","","")
func release_item(point: Vector2) -> void:
	var deadline := Time.get_ticks_msec()+320
	while Time.get_ticks_msec()<deadline: await process_frame
	click_at(point,false)
func run() -> void:
	Input.use_accumulated_input=false
	storage="user://item-layout-%d" % OS.get_process_id()
	await spawn_hud("布局甲")
	hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[item_record(100,"Potion",301,3,0,3)]))
	key(KEY_W)
	key(KEY_Z)
	var origin := Vector2(hud.hud_snapshot().inventory_position)
	await release_item(origin+Vector2(36,79))
	await release_item(Vector2(109,438))
	await release_item(origin+Vector2(36,79))
	await release_item(origin+Vector2(36+2*38,79+3*38))
	if not check(hud.hud_snapshot().bag_records[0].anchor==20 and hud.hud_snapshot().quick_records[0].count==3,"保存前背包移动与快捷绑定"): return
	await settle()
	if not await capture("item-layout-before"): return
	key(KEY_X,0,true)
	key(KEY_ENTER)
	var path := storage+"/布局甲.itms"
	if not check(FileAccess.file_exists(path) and FileAccess.get_file_as_bytes(path).size()==163392,"确认退出立即保存原版长度文件"): return
	var deadline := Time.get_ticks_msec()+4500
	while hud.hud_snapshot().client_page==3 and Time.get_ticks_msec()<deadline: await process_frame
	if not check(hud.hud_snapshot().client_page==2,"保存后进入角色选择"): return
	hud.queue_free()
	await create_timer(0.1).timeout
	await spawn_hud("布局甲")
	hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[item_record(999,"New",301,1,1,0),item_record(100,"Potion",301,9,0,3),item_record(300,"Potion",301,11,0,3)]))
	var rows: Array = hud.hud_snapshot().bag_records
	if not check(rows.size()==3 and rows[0].id==100 and rows[0].anchor==20 and rows[0].count==9,"旧格位与服务器新数量"): return
	if not check(hud.hud_snapshot().quick_records[0].id==300 and hud.hud_snapshot().quick_records[0].count==11,"快捷栏按同名最后一条服务器记录恢复"): return
	key(KEY_W)
	key(KEY_Z)
	await settle()
	if not await capture("item-layout-restored"): return
	hud.open_login()
	hud.queue_free()
	await create_timer(0.1).timeout
	await spawn_hud("布局乙")
	hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[item_record(100,"Potion",301,5,0,3)]))
	if not check(hud.hud_snapshot().bag_records[0].anchor==0 and hud.hud_snapshot().quick_records.is_empty(),"不同角色不继承旧布局"): return
	hud.queue_free()
	await create_timer(0.1).timeout
	if not check(FileAccess.get_file_as_bytes(storage+"/布局乙.itms").size()==163392,"关闭游戏保存独立角色布局"): return
	var corrupt := FileAccess.open(path,FileAccess.WRITE)
	corrupt.store_buffer(PackedByteArray([1,2,3]))
	corrupt.close()
	await spawn_hud("布局甲")
	hud.apply_game_message(packet(Mir3HudView.SM_BAGITEMS,[item_record(100,"Potion",301,7,0,3)]))
	if not check(hud.hud_snapshot().bag_records[0].anchor==0 and hud.hud_snapshot().bag_records[0].count==7 and hud.hud_snapshot().quick_records.is_empty(),"截断布局不破坏新背包"): return
	hud.open_login()
	hud.queue_free()
	await create_timer(0.1).timeout
	print("ITEM_LAYOUT_UI_OK")
	quit()
