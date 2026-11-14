# 定向验收：人物所有图层状态颜色、优先级、受击保留状态及外观/死亡状态更新。
extends "local_struck.gd"

func status(actor: int, flags: int) -> void:
	hud.apply_game_message(game_reply(Mir3HudView.SM_ACTOR_STATUS,actor,flags&65535,(flags>>16)&65535,0))
	await settle(0.035)

func green_pixels(bounds: Rect2) -> int:
	await RenderingServer.frame_post_draw
	var image := root.get_texture().get_image()
	var count := 0
	for y in range(maxi(0,int(bounds.position.y)),mini(530,int(bounds.end.y))):
		for x in range(maxi(0,int(bounds.position.x)),mini(800,int(bounds.end.x))):
			var color := image.get_pixel(x,y)
			if color.g>0.15 and color.r<0.035 and color.b<0.035: count+=1
	return count

func run() -> void:
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.set_local_actor_identity(77)
	hud.set_character_vitals(100,100,0,0,0,100)
	hud.set_player_mount(1)
	hud.set_player_attachments(1,1)
	var origin: Vector2i=hud.hud_snapshot().player_cell
	hud.set_scene_human(5,origin+Vector2i(0,2),4,0,"状态颜色")
	hud.set_scene_human_attachments(5,1,1)
	await status(77,0x80000000)
	await status(5,0x80000000)
	if not check(await green_pixels(hud.hud_snapshot().player_bounds)>40,"本地骑乘身体实际显示绿色状态"): return
	if not check(await green_pixels(hud.hud_snapshot().scene_human_bounds[5])>20,"远端人物实际显示绿色状态"): return
	if not await capture("human-status-green"): return
	var masks := [0x00800000,0x04000000,0x20000000,0x08000000,0x40000000,0x80000000,0x00020000]
	var colors := [1,0,64511,38079,64690,2016,1]
	var combined := 0
	for i in masks.size():
		combined|=masks[i]
		await status(77,combined)
		if not check(hud.hud_snapshot().player_status_color==colors[i],"状态叠加按原版优先级选择"+str(i)): return
	if not await capture("human-status-translucent"): return
	await status(77,0x80000000)
	hud.apply_game_message(hit_reply(Mir3HudView.SM_STRUCK,80,20))
	await settle(0.04)
	if not check(hud.hud_snapshot().player_status_color==2016,"本地受击外观不清除已有状态"): return
	var attacker_cell := origin+Vector2i(0,2)
	var attacker := game_reply(Mir3HudView.SM_TURN,6,attacker_cell.x,attacker_cell.y,4)
	var attacker_feature := PackedByteArray()
	attacker_feature.resize(16)
	attacker_feature[0]=3
	attacker_feature[2]=2
	attacker.append_array(encode_scene_bytes(attacker_feature))
	if not check(hud.apply_game_message(attacker),"建立外观2攻击者"): return
	var hit_feature := PackedByteArray()
	hit_feature.resize(16)
	hit_feature[0]=1
	hit_feature.encode_u32(8,6)
	# 重编码远端身份，正文仍为零状态位。
	var remote_hit := game_reply(Mir3HudView.SM_HUMAN_STRUCK,5,80,100,20)
	remote_hit.append_array(encode_scene_bytes(hit_feature))
	hud.apply_game_message(remote_hit)
	await settle(0.04)
	if not check(hud.hud_snapshot().scene_human_status_colors[5]==2016,"远端受击外观不清除已有状态"): return
	if not check(hud.hud_snapshot().scene_human_pending[5]==1,"1309 受击后将完整31追加到队尾"): return
	var attached := func(effect): return effect.skill==500 and effect.target_actor==5
	if not check(hud.hud_snapshot().scene_effects.any(attached),"外观2附加500特效跟随受击人物"): return
	var hit_deadline := Time.get_ticks_msec()+2500
	while Time.get_ticks_msec()<hit_deadline:
		var hit_state: Dictionary=hud.hud_snapshot()
		if hit_state.scene_human_pending[5]==0 and hit_state.scene_human_actions[5].x==-1: break
		await process_frame
	if not check(hud.hud_snapshot().scene_human_pending[5]==0 and hud.hud_snapshot().scene_human_actions[5].x==-1,
		"两条受击消息完成后才能刷新站立"): return
	if not check(hud.remove_scene_actor(6),"移除原攻击者"): return
	var test_source_reset := false
	for source in [6,77,6]:
		hit_feature.encode_u32(8,source)
		var packet := game_reply(Mir3HudView.SM_STRUCK,5,70,100,0)
		packet.append_array(encode_scene_bytes(hit_feature))
		var before: int=hud.hud_snapshot().scene_effects.filter(attached).size()
		if not check(hud.apply_game_message(packet),"接收缓存来源测试包"): return
		var deadline := Time.get_ticks_msec()+1000
		while hud.hud_snapshot().scene_human_pending[5]>0 and Time.get_ticks_msec()<deadline:
			await process_frame
		if not check(hud.hud_snapshot().scene_human_pending[5]==0,"缓存来源测试包出队"): return
		var after: int=hud.hud_snapshot().scene_effects.filter(attached).size()
		if not check(after==(before+1 if source==6 and not test_source_reset else before),
			"离场来源保留特效，已知人物来源覆盖缓存"): return
		if source==77: test_source_reset=true
	# 4127E8 的其余分支必须载入真实特效帧，并跟随受击者。
	for mapping in [[67,523],[104,542],[150,574]]:
		var spawn := game_reply(Mir3HudView.SM_TURN,6,attacker_cell.x,attacker_cell.y,4)
		attacker_feature[2]=mapping[0]
		spawn.append_array(encode_scene_bytes(attacker_feature))
		if not check(hud.apply_game_message(spawn),"建立特效攻击者 %d" % mapping[0]): return
		hit_feature.encode_u32(8,6)
		var hit_packet := game_reply(Mir3HudView.SM_STRUCK,5,70,100,0)
		hit_packet.append_array(encode_scene_bytes(hit_feature))
		if not check(hud.apply_game_message(hit_packet),"投递特效受击包"): return
		var visible_effect := func(effect): return effect.skill==mapping[1] and effect.target_actor==5 and effect.sprite_visible
		var effect_deadline := Time.get_ticks_msec()+1000
		while not hud.hud_snapshot().scene_effects.any(visible_effect) and Time.get_ticks_msec()<effect_deadline:
			await process_frame
		if not check(hud.hud_snapshot().scene_effects.any(visible_effect),"特效 %d 真实帧可见" % mapping[1]): return
		if not await capture("human-hit-effect-%d" % mapping[1]): return
		if not check(hud.remove_scene_actor(6),"清理特效攻击者"): return
	var feature := PackedByteArray()
	feature.resize(21)
	feature.encode_u32(4,0x04000002)
	hud.queue_scene_human_state(5,Mir3HudView.SM_TURN,origin+Vector2i(0,2),4,feature)
	await settle(0.04)
	if not check(hud.hud_snapshot().scene_human_status_colors[5]==0,"站立外观包更新状态颜色"): return
	var remote_death := game_reply(Mir3HudView.SM_DEATH,5,attacker_cell.x,attacker_cell.y,4)
	if not check(hud.apply_game_message(remote_death),"已有远端人物进入尸体状态"): return
	await settle(0.06)
	if not check(hud.hud_snapshot().scene_human_life[5].dead,"远端尸体已建立"): return
	var late_hit := game_reply(Mir3HudView.SM_STRUCK,5,13,99,7)
	var source_effect := func(effect): return effect.skill==574 and effect.target_actor==5
	var before_late_hit: int=hud.hud_snapshot().scene_effects.filter(source_effect).size()
	hit_feature.encode_u16(14,1)
	late_hit.append_array(encode_scene_bytes(hit_feature))
	if not check(hud.apply_game_message(late_hit),"尸体接收迟到完整受击包"): return
	await settle(0.06)
	var corpse_state: Dictionary=hud.hud_snapshot()
	if not check(corpse_state.scene_health_values[5][0]==13 and corpse_state.scene_health_values[5][2]==99,
		"迟到包同步尸体生命数据"): return
	if not check(corpse_state.scene_human_life[5].dead and corpse_state.scene_human_life[5].frame==9
		and not corpse_state.scene_effects.any(func(effect): return effect.skill==401 and effect.target_actor==5),
		"迟到受击保留尸体末帧且不放普通受击特效"): return
	if not check(corpse_state.scene_effects.filter(source_effect).size()>before_late_hit,
		"尸体仍按缓存攻击来源播放574"): return
	# 迟到包也会用已解析的新攻击者替换缓存。
	var new_source := game_reply(Mir3HudView.SM_TURN,6,attacker_cell.x,attacker_cell.y,4)
	var new_feature := PackedByteArray()
	new_feature.resize(16)
	new_feature[0]=3
	new_feature.encode_u16(2,2)
	new_source.append_array(encode_scene_bytes(new_feature))
	if not check(hud.apply_game_message(new_source),"建立迟到受击的新怪物来源"): return
	hit_feature.encode_u32(8,6)
	var new_late_hit := game_reply(Mir3HudView.SM_STRUCK,5,12,99,7)
	new_late_hit.append_array(encode_scene_bytes(hit_feature))
	if not check(hud.apply_game_message(new_late_hit),"尸体接收新来源受击"): return
	await settle(0.06)
	if not check(hud.hud_snapshot().scene_effects.any(func(effect):
		return effect.skill==500 and effect.target_actor==5),"尸体从574切换到新来源的500"): return
	var last_cell := attacker_cell+Vector2i(1,0)
	if not check(hud.apply_scene_human_stand(5,last_cell,4) and hud.remove_scene_actor(5),
		"在新位置移除仍有命中特效的人物"): return
	if not check(hud.hud_snapshot().scene_effects.any(func(effect):
		return effect.skill==500 and effect.target_actor==0 and effect.cell==last_cell),
		"人物离场后500在最后一格继续播放"): return
	var death := death_reply(Mir3HudView.SM_DEATH,origin)
	var death_feature := PackedByteArray()
	death_feature.resize(16)
	death_feature.encode_u32(4,0x40000000)
	death=death.slice(0,16)
	death.append_array(encode_scene_bytes(death_feature))
	hud.apply_game_message(death)
	await settle(0.04)
	if not check(hud.hud_snapshot().player_dead and hud.hud_snapshot().player_status_color==64690,"死亡外观包刷新尸体状态"): return
	if not await capture("human-status-corpse"): return
	hud.queue_free()
	await settle()
	print("HUMAN_STATUS_VISUAL_UI_OK")
	quit()
