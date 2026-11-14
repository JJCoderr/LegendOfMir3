# 范围专项：9/53九段地表波、10九段闪电尾迹；逐格、延迟、光源与创建音效。
extends "skill_casting.gd"

enum RangeSkill { FIRE_WAVE=9, LIGHTNING=10, ICE_WAVE=53 }

func run() -> void:
	var ids := [RangeSkill.FIRE_WAVE,RangeSkill.LIGHTNING,RangeSkill.ICE_WAVE]
	for argument in OS.get_cmdline_user_args():
		if argument.begins_with("--skill="):
			var value := argument.trim_prefix("--skill=")
			if not check(value.is_valid_int() and int(value) in ids,"范围子集只接受9、10、53"): return
			ids=[int(value)]
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("options_path","user://spell-ranges-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.set_local_actor_identity(77)
	var origin: Vector2i=hud.hud_snapshot().player_cell+Vector2i(0,2)
	for skill in ids:
		# 正东和接近正北的斜目标验证方向归一化，目标远近不截断九段表现。
		for offset in [Vector2i(4,0),Vector2i(1,-5)]:
			if not check(hud.change_map("0","比奇省",origin,1),"清理上个范围用例"): return
			if not check(hud.apply_game_message(magic_reply(skill,origin+offset)),"范围技能回包入队"): return
			if not check(await wait_until(func(s): return s.spell_effects.filter(func(e): return e.ground_wave).size()==9),
				"创建九段地表效果 skill=%d offset=%s states=%s" % [skill,offset,hud.hud_snapshot().spell_effects]): return
			var state: Dictionary=hud.hud_snapshot()
			var waves: Array=state.spell_effects.filter(func(e): return e.ground_wave)
			var step := Vector2i(1,0) if offset.y==0 else Vector2i(0,-1)
			var effect: int=336 if skill==RangeSkill.LIGHTNING else skill
			var interval: int=30 if skill==RangeSkill.LIGHTNING else 150
			var waiting_segment := false
			for i in range(9):
				var matches := waves.filter(func(e): return e.cell==origin+step*(i+1))
				if not check(matches.size()==1 and matches[0].skill==effect and matches[0].emits_light,
					"九段逐格范围、资源编号与光源"): return
				# delay字段保存初始延迟，触发后归零，并非逐帧递减的剩余时间。
				if matches[0].delay>0:
					waiting_segment=true
					if not check(matches[0].delay==interval*i,"未触发段保留150ms或30ms间隔"): return
				elif not check(not waiting_segment,"九段按距离顺序触发，不能越过等待中的前段"): return
			# 原SoundList：10092为空，10102/13362无记录；只有10532映射m53-3.wav。
			if skill==RangeSkill.ICE_WAVE:
				if not check(state.scene_actor_sounds.values().any(func(s): return s.sample==10532),
					"冰波创建使用原表m53-3.wav"): return
			elif not check(state.scene_actor_sounds.is_empty(),"空或缺失的创建音效记录保持静音"): return
			await settle(0.25)
			if skill==RangeSkill.LIGHTNING and not check(hud.hud_snapshot().spell_particle_count>0,"336零间隔尾迹触发可见粒子"): return
			if skill==RangeSkill.ICE_WAVE:
				if not check(await wait_until(func(s): return s.map_event_particle_count>0),"冰波发射共享光点与碎片"): return
			if offset.y==0 and not await capture("skill-range-%d" % skill): return
	hud.change_map("0","比奇省",origin,1)
	if not check(hud.hud_snapshot().spell_effects.is_empty() and hud.hud_snapshot().spell_particle_count==0,"范围技能离场清空"): return
	hud.queue_free()
	await settle()
	print("SPELL_RANGES_UI_OK "+str(ids))
	quit()
