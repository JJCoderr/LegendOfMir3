# 定向验收：46号武器男女资源、动作帧跟随、蓝色场景光及换装/骑乘清理。
extends "forced_motion.gd"

func render_frame() -> void:
	await process_frame
	await RenderingServer.frame_post_draw

func run() -> void:
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.set_character_vitals(100,100,0,0,0,100)
	var origin: Vector2i=hud.hud_snapshot().player_cell
	hud.change_map("0","比奇省",origin,1)
	hud.set_scene_human(5,origin+Vector2i(0,2),4,0,"武器光效")
	hud.set_scene_human_body(5,1,0)
	hud.set_player_attachments(1,1)
	hud.set_scene_human_attachments(5,1,1)
	await render_frame()
	var baseline: PackedColorArray=hud.hud_snapshot().scene_light_samples
	hud.set_player_attachments(1,46)
	hud.set_scene_human_attachments(5,1,46)
	await render_frame()
	var state: Dictionary=hud.hud_snapshot()
	if not check(state.weapon_glow_frames.size()==2 and state.weapon_glow_light_count==2,"男女特殊武器各有实际光效帧及光源"): return
	if not check(state.weapon_glow_frames.any(func(frame): return frame>=1900 and frame<2000) and state.weapon_glow_frames.any(func(frame): return frame>=3900 and frame<4000),"男女使用不同原版帧组"): return
	var bluer := 0
	for i in baseline.size():
		var after: Color=state.scene_light_samples[i]
		if after.b>baseline[i].b+0.02: bluer+=1
	if not check(bluer>8,"武器蓝光实际改变照明网格"): return
	if not await capture("weapon-glow-male-female"): return
	hud.apply_scene_human_action(5,Mir3HudView.SM_HIT,4,0)
	await settle(0.12)
	if not check(hud.hud_snapshot().weapon_glow_frames.any(func(frame): return frame>=4620 and frame<4700),"武器光效随攻击帧推进"): return
	if not await capture("weapon-glow-attack"): return
	hud.set_player_mount(1)
	hud.set_scene_human_attachments(5,1,1)
	await render_frame()
	if not check(hud.hud_snapshot().weapon_glow_light_count==0 and hud.hud_snapshot().weapon_glow_frames.is_empty(),"骑乘动作和换装移除光效及光源"): return
	hud.set_player_mount(0)
	await render_frame()
	if not check(hud.hud_snapshot().weapon_glow_light_count==1,"下马恢复对应武器光源"): return
	hud.set_player_attachments(1,1)
	await render_frame()
	if not check(hud.hud_snapshot().weapon_glow_light_count==0,"最后一件特殊武器移除后无残留"): return
	hud.queue_free()
	await settle()
	print("WEAPON_GLOW_UI_OK")
	quit()
