# 测试内容：天气消息字段、雪与雾原资源、命令暂停恢复及发送、切图天气重建、无效包不变更状态、昼夜参数独立及真实渲染截图。
extends "chat_submission.gd"

func weather_packet(kind: int, day: int = 0) -> PackedByteArray:
	return game_reply(Mir3HudView.SM_DAYCHANGING,0,0,day,kind)

func advance_frames(count: int) -> void:
	hud.set_process(true)
	for i in range(count): await process_frame
	hud.set_process(false)

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("options_path","user://weather-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.set_process(false)
	hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(packet))
	var key_bytes := PackedByteArray([0,0,0,0])
	var state: Dictionary = hud.hud_snapshot()
	if not check(state.snow_enabled and state.snow_resource_loaded and state.fog_resource_loaded,"默认开启及外部ProgUse资源"): return
	if not await capture("weather-clear"): return
	if not check(hud.apply_scene_message(weather_packet(1),key_bytes),"原版雪天天气消息"): return
	# 只推进天气样例；数值边界由独立C++测试覆盖。
	await advance_frames(220)
	state=hud.hud_snapshot()
	if not check(state.weather_snow and state.weather_fog and state.snow_particle_count>0 and state.scene_ambient_mode==0,"雪雾启动且不把天气当夜晚"): return
	for i in range(400):
		var position: Vector2 = hud.hud_snapshot().snow_first_position
		if Rect2(10,10,780,430).has_point(position): break
		await advance_frames(2)
	await RenderingServer.frame_post_draw
	var snow_image := root.get_texture().get_image()
	var snow_position: Vector2 = hud.hud_snapshot().snow_first_position
	if not check(Rect2(10,10,780,430).has_point(snow_position),"雪花进入可见区域"): return
	var sample := Rect2i(Vector2i(snow_position)-Vector2i(8,8),Vector2i(16,16))
	hud.get_node("SnowWeather").hide()
	await process_frame
	await RenderingServer.frame_post_draw
	var fog_image := root.get_texture().get_image()
	if not check(snow_image.get_region(sample).get_data()!=fog_image.get_region(sample).get_data(),"粒子位置产生真实雪纹理像素"): return
	hud.get_node("SnowWeather").show()
	if not await capture("weather-snow-fog"): return
	submit("@snow")
	var paused: Dictionary = hud.hud_snapshot()
	await advance_frames(20)
	state=hud.hud_snapshot()
	if not check(not state.snow_enabled and state.snow_first_age==paused.snow_first_age and state.weather_fog_frame==paused.weather_fog_frame and packets.size()==1,"关闭冻结并仍发CM_SAY"): return
	if not await capture("weather-paused"): return
	submit("@snow")
	await advance_frames(2)
	if not check(hud.hud_snapshot().snow_first_age>paused.snow_first_age and packets.size()==2,"恢复而非重建"): return
	var bad := weather_packet(2)
	bad.append(60)
	if not check(not hud.apply_scene_message(bad,key_bytes) and hud.hud_snapshot().weather_snow,"畸形天气包不修改状态"): return
	if not check(hud.apply_scene_message(weather_packet(2),key_bytes) and not hud.hud_snapshot().weather_snow and hud.hud_snapshot().weather_fog,"仅雾天气"): return
	await advance_frames(2)
	if not await capture("weather-fog-only"): return
	if not check(hud.apply_scene_message(weather_packet(257),key_bytes) and not hud.hud_snapshot().weather_fog,"天气使用完整WORD"): return
	if not check(hud.apply_scene_message(weather_packet(0,1),key_bytes) and hud.hud_snapshot().scene_ambient_mode==1 and hud.hud_snapshot().scene_ambient_fading,"第二参数昼夜渐变"): return
	var before: int = hud.hud_snapshot().scene_ambient
	hud.set_process(true)
	await settle(0.6)
	hud.set_process(false)
	if not check(hud.hud_snapshot().scene_ambient<before,"昼夜仍按50毫秒推进"): return
	state=hud.hud_snapshot()
	var cell: Vector2i = state.player_cell
	var map_message := game_reply(Mir3HudView.SM_CHANGEMAP,0,cell.x,cell.y,1<<8)
	map_message.append_array(encode_scene_bytes("0".to_ascii_buffer()))
	if not check(hud.apply_scene_message(map_message,key_bytes),"切图天气入队"): return
	await advance_frames(2)
	if not check(hud.hud_snapshot().weather_snow and hud.hud_snapshot().snow_particle_count==1 and hud.hud_snapshot().map_series==1,"切图高字节重建雪粒子"): return
	hud.open_login()
	await settle()
	if not check(not hud.hud_snapshot().weather_snow and hud.hud_snapshot().snow_particle_count==0,"退出清理天气"): return
	hud.queue_free()
	await create_timer(0.1).timeout
	print("WEATHER_UI_OK")
	quit()
