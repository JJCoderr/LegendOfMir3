# 定向验收：本地受击排队、生命同步、未命中、骑乘、1309复制及死亡保持。
extends "local_life.gd"

func hit_reply(opcode: int, hp: int, damage: int, variant: int=0, source: int=0) -> PackedByteArray:
	var body := PackedByteArray()
	body.resize(16)
	body[0]=1
	body.encode_u32(8,source)
	body[14]=variant&255
	body[15]=variant>>8
	var packet := game_reply(opcode,77,hp,100,damage)
	packet.append_array(encode_scene_bytes(body))
	return packet

func wait_action(action: int, limit: int=2000) -> bool:
	var deadline := Time.get_ticks_msec()+limit
	while Time.get_ticks_msec()<deadline:
		if hud.hud_snapshot().player_action==action: return true
		await process_frame
	return check(false,"等待动作："+str(action))

func run() -> void:
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.set_local_actor_identity(77)
	hud.set_character_vitals(100,100,0,0,0,100)
	hud.set_character_progress(40,0,0,0,100)
	var origin: Vector2i=hud.hud_snapshot().player_cell
	hud.walk_to_map_cell(origin+Vector2i(0,1))
	await settle(0.04)
	if not check(hud.apply_game_message(hit_reply(Mir3HudView.SM_STRUCK,80,20)),"31完整外观受击包接收"): return
	if not check(hud.hud_snapshot().character_hp.x==100 and hud.hud_snapshot().forced_motion_queue==1,"移动结束前不提前扣血或打断"): return
	if not await wait_action(15): return
	var hit_started := Time.get_ticks_msec()
	if not check(hud.hud_snapshot().character_hp==Vector2i(80,100) and hud.hud_snapshot().player_cell==origin+Vector2i(0,1),"出队同步HP且不把HP误当格坐标"): return
	if not await capture("local-struck-standing"): return
	while hud.hud_snapshot().player_action==15 and Time.get_ticks_msec()-hit_started<1000:
		await process_frame
	if not check(Time.get_ticks_msec()-hit_started<450,"40级受击按80ms推进而非默认200ms"): return
	hud.apply_game_message("+GOOD".to_ascii_buffer())
	if not check(hud.apply_game_message(hit_reply(Mir3HudView.SM_STRUCK,80,0)),"零伤害仍接收"): return
	await settle(0.04)
	if not check(hud.hud_snapshot().player_action!=15,"未命中不启动受击动作"): return
	hud.set_player_mount(1)
	hud.apply_game_message(hit_reply(Mir3HudView.SM_HUMAN_STRUCK,65,15))
	if not await wait_action(32): return
	if not check(hud.hud_snapshot().forced_motion_queue==1,"1309追加31到队尾"): return
	if not await capture("local-struck-mounted"): return
	await settle(0.6)
	if not check(hud.hud_snapshot().forced_motion_queue==0 and hud.hud_snapshot().character_hp.x==65,"复制受击完成且血量使用绝对值"): return
	# 队列为空时，复制的31可能仍在播放同一个骑乘受击动作。
	if not await wait_action(-1): return
	# 当前动作期间连续收包：第三份31到达时折叠前两份，只保留最新动作。
	hud.apply_game_message(hit_reply(Mir3HudView.SM_STRUCK,64,1))
	if not await wait_action(32): return
	hud.apply_game_message(hit_reply(Mir3HudView.SM_STRUCK,63,1))
	hud.apply_game_message(hit_reply(Mir3HudView.SM_STRUCK,62,1))
	hud.apply_game_message(hit_reply(Mir3HudView.SM_STRUCK,61,1))
	if not check(hud.hud_snapshot().forced_motion_queue==1 and hud.hud_snapshot().character_hp.x==62,"折叠旧31同步血量并保留最新包"): return
	await settle(0.6)
	if not check(hud.hud_snapshot().character_hp.x==61,"最新受击出队更新血量"): return
	if not await wait_action(-1): return
	# 特殊受击启动500ms过滤；后续非致死31只同步HP，不追加动画。
	hud.apply_game_message(hit_reply(Mir3HudView.SM_STRUCK,50,15,1))
	if not await wait_action(32): return
	hud.apply_game_message(hit_reply(Mir3HudView.SM_STRUCK,45,5))
	if not check(hud.hud_snapshot().character_hp.x==45 and hud.hud_snapshot().forced_motion_queue==0,"500ms内非致死31只更新HP"): return
	hud.apply_game_message(hit_reply(Mir3HudView.SM_STRUCK,0,45))
	if not check(hud.hud_snapshot().forced_motion_queue==1,"致死血量不被500ms过滤吞掉"): return
	await settle(0.6)
	hud.apply_game_message(death_reply(Mir3HudView.SM_DEATH,hud.hud_snapshot().player_cell))
	await settle(0.05)
	var cell: Vector2i=hud.hud_snapshot().player_cell
	var attacker := game_reply(Mir3HudView.SM_TURN,6,cell.x,cell.y,4)
	var monster_feature := PackedByteArray()
	monster_feature.resize(16)
	monster_feature[0]=3
	monster_feature[2]=2
	attacker.append_array(encode_scene_bytes(monster_feature))
	if not check(hud.apply_game_message(attacker),"建立本地受击的怪物来源"): return
	hud.apply_game_message(hit_reply(Mir3HudView.SM_HUMAN_STRUCK,20,5,0,6))
	await settle(0.06)
	if not check(hud.hud_snapshot().player_dead and hud.hud_snapshot().character_hp.x==0,"尸体受击保持死亡并归零生命"): return
	if not check(hud.hud_snapshot().scene_effects.any(func(effect): return effect.skill==500 and effect.target_actor==77),
		"本地尸体播放跟随自身的怪物来源特效"): return
	if not check(not hud.apply_game_message(game_reply(Mir3HudView.SM_STRUCK,77,10,100,1)),"缺外观受击包拒绝"): return
	hud.queue_free()
	await settle()
	print("LOCAL_STRUCK_UI_OK")
	quit()
