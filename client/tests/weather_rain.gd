# 测试内容：即时雨效、落下与水花实际像素、@snow 独立性、换图保留、停雨恢复和离场清理。
extends "weather.gd"

func wait_for_particle(field: String) -> bool:
	for i in range(300):
		await advance_frames(1)
		var position: Vector2 = hud.hud_snapshot().get(field, Vector2(-1,-1))
		if Rect2(20,30,760,390).has_point(position):
			return true
	return check(false, "可见场景缺少 " + field)

func check_pixels(field: String, screenshot: String) -> bool:
	if not await wait_for_particle(field): return false
	if not await capture(screenshot): return false
	await RenderingServer.frame_post_draw
	var position: Vector2 = hud.hud_snapshot()[field]
	var area := Rect2i(Vector2i(position)-Vector2i(17,17),Vector2i(34,34))
	var drawn := root.get_texture().get_image().get_region(area)
	var layer := hud.find_child("RainWeather",true,false)
	layer.hide()
	await process_frame
	await RenderingServer.frame_post_draw
	var plain := root.get_texture().get_image().get_region(area)
	layer.show()

	var changed := 0
	var difference := Image.create(area.size.x,area.size.y,false,Image.FORMAT_RGB8)
	for y in range(area.size.y):
		for x in range(area.size.x):
			var before := plain.get_pixel(x,y)
			var after := drawn.get_pixel(x,y)
			if before == after: continue
			changed += 1
			difference.set_pixel(x,y,Color(absf(after.r-before.r),absf(after.g-before.g),absf(after.b-before.b))*4.0)
	if not check(changed>0,screenshot + " 在对应场景坐标绘制原包纹理"): return false
	print("%s: position=%s, changed pixels=%d" % [screenshot,position,changed])
	if not OS.get_cmdline_user_args().has("--screenshots"): return true

	var size := area.size*8
	var comparison := Image.create(size.x*3+16,size.y,false,Image.FORMAT_RGB8)
	comparison.fill(Color(0.19,0.19,0.19))
	var panels: Array[Image] = [plain,drawn,difference]
	for i in range(panels.size()):
		var panel := panels[i]
		panel.convert(Image.FORMAT_RGB8)
		panel.resize(size.x,size.y,Image.INTERPOLATE_NEAREST)
		comparison.blit_rect(panel,Rect2i(Vector2i.ZERO,size),Vector2i(i*(size.x+8),0))
	var path := ProjectSettings.globalize_path("res://../build/screenshots/%s-detail.png" % screenshot)
	return check(comparison.save_png(path)==OK,"保存关闭、开启和像素差分对照")

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("options_path","user://weather-rain-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.set_process(false)
	hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(packet))
	var key_bytes := PackedByteArray([0,0,0,0])
	submit("@snow")
	var state: Dictionary = hud.hud_snapshot()
	if not check(not state.snow_enabled and packets.size()==1 and state.rain_resources_loaded and state.snow_resource_loaded,"雪雾关闭且原包雨滴水花资源完整"): return
	if not check(hud.apply_scene_message(weather_packet(3),key_bytes),"接收即时雨效"): return
	if not await check_pixels("rain_first_position","weather-rain"): return
	if not await check_pixels("rain_splash_position","weather-rain-splash"): return
	state=hud.hud_snapshot()
	var cell: Vector2i = state.player_cell
	var map_message := game_reply(Mir3HudView.SM_CHANGEMAP,0,cell.x,cell.y,2<<8)
	map_message.append_array(encode_scene_bytes("0".to_ascii_buffer()))
	if not check(hud.apply_scene_message(map_message,key_bytes),"换图天气入队"): return
	await advance_frames(2)
	state=hud.hud_snapshot()
	if not check(state.map_series==2 and state.weather_other and state.weather_fog and not state.snow_enabled and state.rain_particle_count>0,"换图至雾天保留雨效"): return
	if not check(hud.apply_scene_message(weather_packet(0),key_bytes),"即时晴天清除雨效标志"): return
	var count: int = hud.hud_snapshot().rain_particle_count
	await advance_frames(8)
	state=hud.hud_snapshot()
	if not check(not state.weather_other and state.rain_particle_count==count,"停雨冻结粒子"): return
	if not check(hud.apply_scene_message(weather_packet(3),key_bytes) and hud.hud_snapshot().rain_particle_count==count,"恢复原粒子"): return
	hud.open_login()
	await settle()
	state=hud.hud_snapshot()
	if not check(not state.weather_other and state.rain_particle_count==0,"离场清理雨效"): return
	hud.queue_free()
	await create_timer(0.1).timeout
	print("RAIN_UI_OK")
	quit()
