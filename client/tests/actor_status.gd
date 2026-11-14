# 定向验收：801/807/808状态与速度排队、1100灯光即时更新、1101血条校验恢复。
extends "forced_motion.gd"

func run() -> void:
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.set_local_actor_identity(77)
	hud.set_character_vitals(100,100,0,0,0,100)
	var origin: Vector2i=hud.hud_snapshot().player_cell
	hud.set_scene_human(5,origin+Vector2i(0,2),4,0,"状态验收")
	hud.apply_game_message(game_reply(Mir3HudView.SM_HEALTHSPELLCHANGED,5,80,0,100))
	hud.apply_game_message(game_reply(Mir3HudView.SM_HEALTH_PERCENT,5,0,0,0))
	hud.apply_scene_human_action(5,Mir3HudView.SM_SITDOWN,4,0)
	hud.walk_to_map_cell(origin+Vector2i(0,1))
	await settle(0.03)
	for actor in [77,5]:
		if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_ACTOR_STATUS,actor,2,0,65535)),"807状态速度入队"): return
	if not check(hud.hud_snapshot().melee_speed==0 and hud.hud_snapshot().scene_human_status[5].y==0,"动作中不提前更新速度"): return
	for actor in [77,5]:
		if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_ACTOR_LIGHT,actor,258,65535,65535)),"1100即时灯光更新"): return
	if not check(hud.hud_snapshot().player_light_attribute==2 and hud.hud_snapshot().scene_human_lights[5]==2,"灯光取参数0低字节而非参数2"): return
	await settle(0.95)
	if not check(hud.hud_snapshot().melee_speed==-1 and hud.hud_snapshot().player_status_flags==2 and hud.hud_snapshot().scene_human_status[5]==Vector2i(2,-1),"队列出队同步DWORD状态和有符号速度"): return
	if not check(hud.hud_snapshot().player_light_attribute==2 and hud.hud_snapshot().scene_human_lights[5]==2,"807不污染灯光"): return
	hud.apply_game_message(game_reply(Mir3HudView.SM_801,5,0,0,10))
	hud.apply_game_message(game_reply(Mir3HudView.SM_808,77,0,0,10))
	await settle(0.05)
	if not check(not hud.hud_snapshot().scene_health_bounds.has(5),"清除状态bit1只清当前校验值，血条隐藏"): return
	hud.apply_game_message(game_reply(Mir3HudView.SM_ACTOR_BAR_REFRESH,5,0,0,0))
	await settle(0.04)
	if not check(hud.hud_snapshot().scene_health_bounds.has(5),"1101同步当前与保存校验值恢复血条"): return
	if not await capture("actor-status-bar-restored"): return
	if not check(hud.hud_snapshot().melee_speed==10 and hud.hud_snapshot().scene_human_status[5].y==10,"801/808同一状态入口"): return
	hud.change_map("01","地图01",Vector2i(312,310),1)
	await settle(0.04)
	if not check(hud.hud_snapshot().player_light_attribute==2,"换图保留本地灯光"): return
	if not await capture("actor-status-local-light"): return
	hud.set_local_actor_identity(78)
	if not check(hud.hud_snapshot().player_status_flags==0 and hud.hud_snapshot().melee_speed==0 and hud.hud_snapshot().player_light_attribute==0,"新身份清理角色状态"): return
	hud.queue_free()
	await settle()
	print("ACTOR_STATUS_UI_OK")
	quit()
