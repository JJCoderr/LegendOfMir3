# 测试内容：原资源升级特效、本地等级经验和提示、远端升级隔离、无效分支、并行效果、末帧光尾、移动中升级、骑乘恢复及结束回收。
extends "login_smoke.gd"

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("map_spawn_cell",Vector2i(413,410))
	hud.set_meta("options_path","user://ability-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	hud.set_character_identity("升级验证","","","")
	var packet := game_reply(Mir3HudView.SM_ABILITY,0xfedcba98,2,0,0)
	packet.append_array(PackedByteArray([70, 108, 68, 67, 60, 108, 92, 70, 67, 76, 80, 77, 61, 109, 89, 90, 60, 83, 92, 60, 121, 60, 67, 108, 60, 72, 63, 80, 60, 76, 62, 60, 66, 92, 84, 60, 85, 60, 63, 118, 60, 60, 120, 90, 61, 108, 108, 61, 60, 108, 60, 60, 60, 92, 80, 63, 92, 60, 79, 123, 61, 76, 96, 66, 62, 92, 64, 60, 91, 116, 63, 123, 60, 108, 76, 62, 60, 67, 118, 61, 123, 92, 80, 66]))
	if not check(hud.apply_game_message(packet),"能力消息接入游戏分发"): return
	var origin: Vector2i = hud.hud_snapshot().player_cell
	if not check(hud.set_scene_human(5,origin+Vector2i(-2,1),4,1,"升级旅人"),"加入远端人物"): return
	var before: int = hud.hud_snapshot().chat_compact_count
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_LEVELUP,210000,0x012c,0,0)),"本地升级消息"): return
	var state: Dictionary = hud.hud_snapshot()
	if not check(state.character_level==44 and state.character_experience==210000 and state.character_next_experience==400000 and state.player_level_action,"本地低字节等级和升级动作"): return
	if not check(state.level_effects.size()==1 and state.level_effects[0].frame==2020 and state.chat_compact_count==before+1,"原资源首帧及一次提示"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_LEVELUP,5,99,0,1)),"远端升级消息"): return
	state=hud.hud_snapshot()
	if not check(state.level_effects.size()==2 and state.character_level==44 and state.character_experience==210000 and state.chat_compact_count==before+1,"远端效果不改本地属性提示"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_LEVELUP,999,99,0,1)) and hud.apply_game_message(game_reply(Mir3HudView.SM_LEVELUP,1,1,0,2)) and hud.hud_snapshot().level_effects.size()==2,"未知人物及分支忽略"): return
	await settle(0.3)
	if not await capture("level-up-effects"): return
	var saw_tail := false
	var deadline := Time.get_ticks_msec()+3000
	while Time.get_ticks_msec()<deadline:
		state=hud.hud_snapshot()
		for effect in state.level_effects:
			if effect.phase==1 and effect.frame==2029: saw_tail=true
		if state.level_effects.is_empty(): break
		await process_frame
	if not check(saw_tail and hud.hud_snapshot().level_effects.is_empty(),"观察末帧光尾后回收"): return
	key(KEY_Q)
	await settle()
	if not await capture("level-up-status"): return
	key(KEY_Q)
	if not check(hud.set_player_body(0,1) and hud.set_player_mount(1) and hud.set_scene_human_mount(5,1),"本地和远端骑乘"): return
	if not check(hud.walk_to_map_cell(origin+Vector2i(0,1)),"开始骑乘移动"): return
	await settle(0.1)
	if not check(hud.hud_snapshot().walk_frame>=0,"升级发生在移动中"): return
	var upgrade_cell: Vector2i = hud.hud_snapshot().player_cell
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_LEVELUP,215000,44,0,0)) and hud.apply_game_message(game_reply(Mir3HudView.SM_LEVELUP,5,44,0,1)),"移动与骑乘升级"): return
	state=hud.hud_snapshot()
	if not check(state.walk_frame==-1 and state.player_mount==1 and state.scene_human_mounts[5]==1 and state.level_effects[0].cell==upgrade_cell,"升级切换动作但保留坐骑"): return
	await settle(0.7)
	state=hud.hud_snapshot()
	if not check(not state.player_level_action and state.scene_human_actions[5].x==-1 and state.player_mount==1 and state.scene_human_mounts[5]==1,"动作结束恢复骑乘"): return
	if not await capture("level-up-mounted"): return
	if not check(hud.walk_to_map_cell(upgrade_cell+Vector2i(0,1)),"升级后继续移动"): return
	await settle(0.08)
	state=hud.hud_snapshot()
	if not check(not state.level_effects.is_empty() and state.level_effects[0].cell==upgrade_cell,"升级特效保留发生地点"): return
	if not check(hud.set_player_mount(0),"恢复步行"): return
	if not check(hud.change_map("0","比奇省",origin,1),"夜间照明场景"): return
	await settle()
	var baseline: PackedColorArray = hud.hud_snapshot().scene_light_samples
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_LEVELUP,220000,45,0,0)),"夜间升级"): return
	await settle(0.15)
	var lit: PackedColorArray = hud.hud_snapshot().scene_light_samples
	var brighter := 0
	for i in range(lit.size()):
		if lit[i].get_luminance()>baseline[i].get_luminance()+0.01: brighter+=1
	if not check(brighter>0,"升级照明确实改变场景网格"): return
	if not await capture("level-up-night"): return
	if not check(hud.change_map("0","比奇省",origin,1) and hud.hud_snapshot().level_effects.is_empty(),"切图清除未结束特效"): return
	print("LEVEL_UP_UI_OK")
	quit()
