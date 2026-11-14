# 测试内容：持续地面事件显示、画面像素、重复编号、隐藏和切图清理。
extends "login_smoke.gd"

func show_event(id: int, kind: int, cell: Vector2i) -> PackedByteArray:
	var message := game_reply(Mir3HudView.SM_SHOWEVENT,id,kind,cell.x,cell.y)
	message.append_array(encode_scene_bytes(PackedByteArray([0,0,0,0])))
	return message

func run() -> void:
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	var origin: Vector2i = hud.hud_snapshot().player_cell
	var fire := show_event(0x87654321,4,origin+Vector2i(-2,0))
	var ice := show_event(123,5,origin+Vector2i(2,0))
	hud.set_process(false)
	if not check(hud.apply_game_message(fire) and hud.apply_game_message(ice),"原包创建两种持续事件"): return
	if not check(hud.apply_game_message(fire) and hud.hud_snapshot().map_event_count==2,"重发不新建事件"): return
	if not check(not hud.apply_game_message(fire.slice(0,18)) and hud.hud_snapshot().map_event_count==2,"短正文不改变事件"): return
	if not await capture("map-events-active"): return
	await RenderingServer.frame_post_draw
	var active := root.get_texture().get_image().get_region(Rect2i(220,150,310,120))
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_HIDEEVENT,0x87654321,0,0,0)) and hud.hud_snapshot().map_event_count==1,"按编号清除持续地面事件"): return
	await process_frame
	await RenderingServer.frame_post_draw
	if not check(active.get_data()!=root.get_texture().get_image().get_region(Rect2i(220,150,310,120)).get_data(),"隐藏改变真实地图像素"): return
	if not check(hud.change_map("0","比奇省",origin) and hud.hud_snapshot().map_event_count==0,"切图移除剩余效果"): return
	hud.queue_free()
	await create_timer(0.1).timeout
	print("MAP_EVENT_UI_OK")
	quit()
