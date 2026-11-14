# 测试内容：持续地面事件显示、画面像素、重复编号、隐藏、类型9驻留与退场及切图清理。
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
	var layered := show_event(456,9,origin)
	if not check(hud.apply_game_message(layered) and hud.hud_snapshot().map_event_frames[456]==90,"类型9从首帧启动"): return
	hud.set_process(true)
	await create_timer(0.95).timeout
	hud.set_process(false)
	if not check(hud.hud_snapshot().map_event_frames[456]==100,"类型9驻留帧"): return
	if not await capture("map-event-layered"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_HIDEEVENT,456,0,0,0))
		and hud.hud_snapshot().map_event_count==2,"隐藏保留退出动画"): return
	hud.set_process(true)
	await create_timer(0.33).timeout
	hud.set_process(false)
	if not check(hud.hud_snapshot().map_event_particle_count>0,"退场段在可走格产生粒子"): return
	if not await capture("map-event-layered-particles"): return
	hud.set_process(true)
	await create_timer(0.62).timeout
	hud.set_process(false)
	if not check(not hud.hud_snapshot().map_event_frames.has(456),"退出动画结束释放类型9"): return
	if not check(hud.hud_snapshot().map_event_mark_count==1,"尾段留下地表印记"): return
	if not check(hud.apply_game_message(show_event(700,3,origin+Vector2i(-2,1)))
		and hud.apply_game_message(show_event(701,6,origin+Vector2i(2,1))),"创建原版地表图"): return
	hud.set_process(true)
	await create_timer(0.06).timeout
	hud.set_process(false)
	if not check(hud.hud_snapshot().map_event_frames[700]==230 and hud.hud_snapshot().map_event_frames[701]==210,"类型与ProgUse图号"): return
	if not await capture("map-events-ground"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_HIDEEVENT,701,0,0,0))
		and hud.hud_snapshot().map_event_frames.has(701),"隐藏后等待地图更新"): return
	hud.set_process(true)
	await create_timer(0.06).timeout
	hud.set_process(false)
	if not check(not hud.hud_snapshot().map_event_frames.has(701),"下一次地图更新清除地表图"): return
	var hide_ice := game_reply(Mir3HudView.SM_HIDEEVENT,123,0,0,0)
	if not check(hud.apply_game_message(hide_ice),"隐藏触发残留"): return
	var tail: int = hud.hud_snapshot().map_event_frames[123]
	if not check(tail>=2450 and tail<=2490 and (tail-2450)%10==0
		and hud.hud_snapshot().map_ground_event_count==2,"生成原版残留动画和地表图"): return
	if not check(hud.apply_game_message(hide_ice) and hud.hud_snapshot().map_event_frames[123]==tail
		and hud.hud_snapshot().map_ground_event_count==2,"重复隐藏保留原残留"): return
	hud.set_process(true)
	await create_timer(0.2).timeout
	hud.set_process(false)
	if not check(hud.hud_snapshot().map_event_frames[123]>tail,"残留动画独立推进"): return
	if not await capture("map-event-ice-aftermath"): return
	if not check(hud.change_map("0","比奇省",origin) and hud.hud_snapshot().map_event_count==0 and hud.hud_snapshot().map_event_mark_count==0 and hud.hud_snapshot().map_event_particle_count==0,"切图移除剩余效果"): return
	hud.queue_free()
	await create_timer(0.1).timeout
	print("MAP_EVENT_UI_OK")
	quit()
