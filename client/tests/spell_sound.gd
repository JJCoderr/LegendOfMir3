# 技能音效专项：同种并发、异种互不覆盖、声道上限及换图释放。
extends "skill_casting.gd"

func run() -> void:
	var ids := [1,53]
	for argument in OS.get_cmdline_user_args():
		if argument.begins_with("--skill="):
			var value := argument.trim_prefix("--skill=")
			if not check(value.is_valid_int() and int(value) in ids,"音效子集只接受1、53"): return
			ids=[int(value)]
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("options_path","user://spell-sound-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.set_local_actor_identity(77)
	var origin: Vector2i=hud.hud_snapshot().player_cell+Vector2i(0,2)
	hud.change_map("0","比奇省",origin,1)
	var buses_before := AudioServer.bus_count
	for skill in ids:
		if not check(hud.apply_game_message(magic_reply(skill,origin+Vector2i(5,0))),"音效技能原包入队"): return
		var sample: int=10011 if skill==1 else 10532
		if not check(await wait_until(func(s): return s.scene_actor_sounds.values().any(func(a): return a.sample==sample)),"创建对应原音效声道"): return
		var sounds: Array=hud.hud_snapshot().scene_actor_sounds.values().filter(func(a): return a.sample==sample)
		if skill==53 and not check(sounds.size()==9,"九段冰波同时拥有九个独立声道"): return
		if skill==1:
			var initial_pan: int=sounds[0].pan
			if not check(await wait_until(func(s): return s.scene_actor_sounds.values().any(func(a): return a.sample==sample and a.pan<initial_pan),500),
				"向东飞行的火球更新原声道声像"): return
		if not check(hud.apply_game_message(magic_reply(skill,origin+Vector2i(4,0))),"追加同种技能"): return
		await settle(0.1)
		sounds=hud.hud_snapshot().scene_actor_sounds.values().filter(func(a): return a.sample==sample)
		if not check(sounds.size()>=2 and sounds.size()<=10,"同种技能并发且不超过十声道"): return
	if ids.size()==2:
		var sounds: Array=hud.hud_snapshot().scene_actor_sounds.values()
		if not check(sounds.any(func(a): return a.sample==10011) and sounds.any(func(a): return a.sample==10532),
			"火球与冰波资源同时存在，不替换另一声音"): return
	if not await capture("skill-sound-concurrent"): return
	hud.change_map("0","比奇省",origin,1)
	await process_frame
	if not check(hud.hud_snapshot().scene_actor_sounds.is_empty() and AudioServer.bus_count==buses_before,"换图释放全部技能声道及音频总线"): return
	hud.queue_free()
	await settle()
	print("SPELL_SOUND_UI_OK "+str(ids))
	quit()
