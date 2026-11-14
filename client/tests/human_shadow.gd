# 定向验收：身体资源投影、武器奇数行投影、坐骑独立影、尸体平影及半透明选项。
extends "local_life.gd"

func rendered() -> Image:
	await process_frame
	await RenderingServer.frame_post_draw
	return root.get_texture().get_image()

func run() -> void:
	var bodies: Array[Image]=[]
	var region := Rect2()
	for blend in [0,1]:
		var path := "user://human-shadow-%d-%d.ini" % [OS.get_process_id(),blend]
		var config := ConfigFile.new()
		config.set_value("Options","ShadowBlend",blend)
		config.save(path)
		hud=load("res://main.tscn").instantiate()
		hud.set_meta("offline_preview",true)
		hud.set_meta("options_path",path)
		hud.set_connection_enabled(false)
		root.add_child(hud)
		await settle(0.02)
		hud.set_process(false)
		hud.set_local_actor_identity(77)
		bodies.append(await rendered())
		var shadows: Array=hud.hud_snapshot().human_shadow_bounds
		if not check(shadows.size()==1,"身体使用原资源阴影模式"): return
		region=shadows[0]
		hud.set_player_attachments(1,1)
		await rendered()
		if not check(hud.hud_snapshot().human_shadow_bounds.size()==2,"武器增加独立投影"): return
		if not await capture("human-shadow-weapon-"+str(blend)): return
		hud.set_player_mount(1)
		await rendered()
		if not check(hud.hud_snapshot().human_shadow_bounds.size()>=1,"骑乘读取独立阴影帧"): return
		if not await capture("human-shadow-mounted-"+str(blend)): return
		hud.apply_game_message(death_reply(Mir3HudView.SM_DEATH,hud.hud_snapshot().player_cell,0))
		hud.set_process(true)
		await settle(0.03)
		hud.set_process(false)
		await rendered()
		if not check(hud.hud_snapshot().player_dead and hud.hud_snapshot().human_shadow_bounds.size()==1,"尸体只保留平影，无武器或坐骑影"): return
		if not await capture("human-shadow-corpse-"+str(blend)): return
		hud.queue_free()
		await settle(0.03)
	var changed := 0
	for y in range(maxi(0,int(region.position.y)),mini(530,int(region.end.y))):
		for x in range(maxi(0,int(region.position.x)),mini(800,int(region.end.x))):
			if bodies[0].get_pixel(x,y).get_luminance()<0.01 and bodies[1].get_pixel(x,y).get_luminance()>0.03:
				changed+=1
	if not check(changed>30,"半透明阴影真实像素保留背景，区别于实心影"): return
	print("HUMAN_SHADOW_UI_OK")
	quit()
