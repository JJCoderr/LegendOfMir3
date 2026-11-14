# 施法纠偏组合：碰撞排队、折返落点、确认不截断起手、拒绝清理及换图后迟到拒绝。
extends "skill_casting.gd"

func begin_fireball(origin: Vector2i) -> bool:
	if not hud.change_map("0","比奇省",origin,1): return false
	if not await wait_until(func(s): return s.map_left_delay_ms==0): return false
	var pointer := InputEventMouseMotion.new()
	pointer.position=Vector2((origin+Vector2i(4,0))*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(1,1)
	root.push_input(pointer,true)
	key(KEY_F1)
	return hud.hud_snapshot().action_reply_pending and hud.hud_snapshot().player_action==2

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("options_path","user://spell-correction-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.set_local_actor_identity(77)
	hud.set_character_vitals(100,100,200,200,0,100)
	var packet := game_reply(Mir3HudView.SM_SENDMYMAGIC,0,0,0,1)
	packet.append_array(skill_record(Skill.FIREBALL,49))
	packet.append(47)
	if not check(hud.apply_game_message(packet),"准备纠偏用火球技能"): return
	var origin: Vector2i=hud.hud_snapshot().player_cell+Vector2i(0,2)
	var packets: Array[PackedByteArray]=[]
	hud.client_packet_ready.connect(func(value): packets.append(value))
	if not check(await begin_fireball(origin),"开始等待确认的施法"): return
	if not check(hud.apply_game_message(motion_reply(Mir3HudView.SM_BUMP,77,origin+Vector2i(1,0),2)),"施法期间碰撞原包入队"): return
	var state: Dictionary=hud.hud_snapshot()
	if not check(state.forced_motion_queue==1 and state.forced_motion_opcode==0 and state.player_action==2,
		"碰撞等待当前施法起手，不立即移动"): return
	if not check(await wait_until(func(s): return s.forced_motion_opcode==Mir3HudView.SM_BUMP),"起手结束后开始服务器碰撞"): return
	if not check(await wait_until(func(s): return s.forced_motion_opcode==0 and s.player_action==7),"碰撞结束折返受阻动作"): return
	state=hud.hud_snapshot()
	if not check(state.player_cell==origin and not state.action_reply_pending and state.drawn_human_spells.is_empty()
		and packets.size()==1,"折返推导原点、解除等待、起手特效无残留且不反发请求"): return
	if not await capture("skill-correction-bump"): return
	if not check(await begin_fireball(origin),"重新准备确认与拒绝组合"): return
	if not check(hud.apply_game_message("+GOOD".to_ascii_buffer()) and not hud.hud_snapshot().action_reply_pending
		and hud.hud_snapshot().player_action==2,"确认解除等待但不截断施法动作"): return
	if not check(hud.apply_game_message("+FAIL".to_ascii_buffer()) and hud.hud_snapshot().player_cell==origin
		and hud.hud_snapshot().player_action==-1,"后续拒绝恢复起手前状态"): return
	await process_frame
	if not check(hud.hud_snapshot().drawn_human_spells.is_empty(),"拒绝后清除已绘制起手"): return
	if not check(await begin_fireball(origin),"开始跨地图拒绝场景"): return
	var new_origin := origin+Vector2i(0,2)
	if not check(hud.change_map("0","比奇省",new_origin,1) and hud.apply_game_message("+FAIL".to_ascii_buffer()),"换图后接收旧拒绝"): return
	state=hud.hud_snapshot()
	if not check(state.player_cell==new_origin and not state.action_reply_pending and state.player_action==-1
		and state.forced_motion_queue==0 and state.map_event_count==0,"旧拒绝不能跨地图回滚，动作与效果清空"): return
	if not await capture("skill-correction-map"): return
	hud.queue_free()
	await settle()
	print("SPELL_CORRECTION_UI_OK")
	quit()
