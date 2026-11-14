# 技能定向验收：直接进入地图，不执行登录、选角或其他模块长流程。
extends "forced_motion.gd"

enum Skill { FIREBALL=1, RUSH=27, STAGED=73, THREE_RAYS=110, MULTIPLE=113, ORBIT=150 }

func repeat_key(code: int) -> void:
	var event := InputEventKey.new()
	event.keycode=code
	event.physical_keycode=code
	event.pressed=true
	event.echo=true
	root.push_input(event)

func skill_record(id: int, binding: int) -> PackedByteArray:
	var raw := PackedByteArray()
	raw.resize(74)
	raw[4]=binding
	raw[5]=1
	raw.encode_u16(6,id)
	raw.encode_u32(49,0)
	raw[53]=10
	return encode_scene_bytes(raw)

func magic_reply(id: int, cell: Vector2i, target: int=0, actor: int=77) -> PackedByteArray:
	var packet := game_reply(Mir3HudView.SM_MAGICFIRE,actor,cell.x,cell.y,id)
	var body := PackedByteArray()
	body.resize(4)
	body.encode_u32(0,target)
	packet.append_array(encode_scene_bytes(body))
	return packet

func wait_until(predicate: Callable, duration: int=2000) -> bool:
	var deadline := Time.get_ticks_msec()+duration
	while Time.get_ticks_msec()<deadline:
		await process_frame
		if predicate.call(hud.hud_snapshot()): return true
	return false

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("options_path","user://skill-casting-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.set_local_actor_identity(77)
	hud.set_character_vitals(100,100,200,200,0,100)
	# 避开默认出生格的建筑立柱，便于目视核对人物手部起手贴图。
	var origin: Vector2i=hud.hud_snapshot().player_cell+Vector2i(0,2)
	if not check(hud.change_map("0","比奇省",origin,255),"准备无遮挡施法格"): return
	if not check(await wait_until(func(s): return s.map_left_delay_ms==0),"等待换图输入门控结束"): return
	var target := origin+Vector2i(4,0)
	var full := game_reply(Mir3HudView.SM_SENDMYMAGIC,0,0,0,3)
	full.append_array(skill_record(Skill.FIREBALL,49))
	full.append(47)
	full.append_array(skill_record(Skill.RUSH,65))
	full.append(47)
	full.append_array(skill_record(Skill.FIREBALL,66))
	full.append(47)
	if not check(hud.apply_game_message(full),"准备F1技能原包"): return
	var packets: Array[PackedByteArray]=[]
	hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(packet))
	var mouse := InputEventMouseMotion.new()
	mouse.position=Vector2(target*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(1,1)
	root.push_input(mouse,true)
	hud.set_character_vitals(100,100,19,200,0,100)
	key(KEY_F1)
	if not check(packets.is_empty(),"MP低于门槛时不发施法请求"): return
	hud.set_character_vitals(100,100,20,200,0,100)
	key(KEY_F1)
	if not check(packets.size()==1 and hud.hud_snapshot().player_action==2,"F1仅发一个请求并进入定向起手"): return
	var request := decode_request(packets[0])
	if not check(request.decode_u16(4)==Mir3HudView.CM_SPELL and request.decode_u16(8)==Skill.FIREBALL,
		"请求使用3017且技能编号位置正确"): return
	key(KEY_F1)
	if not check(packets.size()==1,"冷却内重复按键不发包"): return
	# 等待进入起手中段，避免截到原资源刚出现的微弱首帧。
	if not check(await wait_until(func(s): return s.player_spell_frame>=3 and not s.drawn_human_spells.is_empty()),
		"本地起手实际绘制原资源帧"): return
	if not await capture("skill-casting-start"): return
	if not check(hud.apply_game_message("+FAIL".to_ascii_buffer()) and hud.hud_snapshot().player_cell==origin,
		"失败回包恢复预测原点"): return
	# 直接注入技能效果，单独验收表现，不重复等待每个技能的快捷键冷却。
	if not check(hud.apply_game_message(magic_reply(Skill.ORBIT,target)),"150回包进入队列"): return
	if not check(await wait_until(func(s): return s.spell_particle_count>0),"150产生可见粒子"): return
	await settle(0.12)
	if not await capture("skill-casting-orbit"): return
	if not check(await wait_until(func(s): return s.spell_particle_count==0,2500),"发射结束后粒子按寿命释放"): return
	hud.change_map("0","比奇省",origin,1)
	if not check(hud.hud_snapshot().map_event_count==0 and hud.hud_snapshot().spell_particle_count==0,
		"换图清理技能事件与粒子"): return
	if not check(hud.apply_game_message(magic_reply(Skill.STAGED,target)),"73回包进入队列"): return
	if not check(await wait_until(func(s): return s.spell_effects.any(func(e): return e.skill==Skill.STAGED and e.stage==1)),
		"73独立进入移动阶段"): return
	if not await capture("skill-casting-staged"): return
	hud.change_map("0","比奇省",origin,1)
	if not check(hud.apply_game_message(magic_reply(Skill.THREE_RAYS,target)),"110回包进入队列"): return
	if not check(await wait_until(func(s): return s.map_event_count==15),"三射线保留15段及重叠起点"): return
	if not check(hud.hud_snapshot().spell_effects.all(func(e): return e.ground_wave and e.emits_light),
		"三射线各段启用原资源光源"): return
	await settle(0.2)
	if not await capture("skill-casting-rays"): return
	hud.change_map("0","比奇省",origin,1)
	if not check(hud.hud_snapshot().map_event_count==0,"地表技能换图无残留"): return
	# 远端起手与效果按同一人物队列执行；638必须等待起手动作完成。
	var remote := origin+Vector2i(0,2)
	if not check(hud.set_scene_human(88,remote,0,0,"施法验收"),"准备远端施法者"): return
	var start := game_reply(Mir3HudView.SM_SPELL,88,target.x,target.y,0)
	start.append_array(str(Skill.FIREBALL).to_ascii_buffer())
	if not check(hud.apply_game_message(start),"远端原始十进制起手入队"): return
	if not check(await wait_until(func(s): return s.scene_human_actions[88].x==2),"远端启动定向动作"): return
	if not check(hud.hud_snapshot().scene_human_cells[88]==remote,"起手目标坐标不移动施法者"): return
	if not check(hud.apply_game_message(magic_reply(Skill.FIREBALL,origin,77,88)),"远端638效果入队"): return
	if not check(hud.hud_snapshot().map_event_count==0 and hud.hud_snapshot().scene_human_pending[88]==1,
		"638等待当前远端起手结束"): return
	await settle(0.2)
	if not check(not hud.hud_snapshot().drawn_human_spells.is_empty(),"远端起手实际绘制原资源帧"): return
	if not await capture("skill-casting-remote"): return
	if not check(await wait_until(func(s): return s.spell_effects.any(func(e): return e.skill==Skill.FIREBALL)),
		"远端起手结束后生成投射物"): return
	# 1410保留重复目标，跳过不在场目标，不将目标文本当二进制编号。
	hud.change_map("0","比奇省",origin,1)
	if not check(hud.set_scene_human(88,remote,0,0,"多目标验收"),"准备多目标人物"): return
	var multiple := game_reply(Mir3HudView.SM_MAGICFIRE_LIST,77,target.x,target.y,Skill.MULTIPLE)
	multiple.append_array(encode_scene_bytes("88/999999/88".to_ascii_buffer()))
	if not check(hud.apply_game_message(multiple),"1410目标列表入队"): return
	if not check(await wait_until(func(s): return s.spell_effects.size()==2),"重复目标保留两发、未知目标跳过"): return
	if not await capture("skill-casting-multiple"): return
	hud.change_map("0","比奇省",origin,1)
	if not check(await wait_until(func(s): return s.map_left_delay_ms==0),"等待冲撞前换图限制结束"): return
	root.push_input(mouse,true)
	repeat_key(KEY_F11)
	if not check(packets.size()==1,"F11持续按键不进入施法"): return
	repeat_key(KEY_F10)
	if not check(packets.size()==2 and hud.hud_snapshot().action_reply_pending and hud.hud_snapshot().walk_frame==-1
		and hud.hud_snapshot().player_cell==origin,"F10持续按键发冲撞请求并等待服务器位移"): return
	var rush_request := decode_request(packets.back())
	if not check(rush_request.decode_u32(0)==2 and rush_request.decode_u16(8)==Skill.RUSH
		and rush_request.decode_u16(6)==0 and rush_request.decode_u16(10)==0,"冲撞3017只发送朝向，不发送鼠标格或目标编号"): return
	var landing := origin+Vector2i(1,0)
	if not check(hud.apply_game_message("+GOOD".to_ascii_buffer()) and
		hud.apply_game_message(motion_reply(Mir3HudView.SM_RUSH,77,landing,2)),"服务器确认和冲撞位移入队"): return
	if not check(await wait_until(func(s): return s.forced_motion_opcode==Mir3HudView.SM_RUSH),"服务器回包启动冲撞动作"): return
	if not await capture("skill-casting-rush"): return
	if not check(await wait_until(func(s): return s.forced_motion_opcode==0 and s.player_cell==landing and not s.action_reply_pending)
		and packets.size()==2,"冲撞落点、解除等待且不重复发包"): return
	# 近战回包的高字节携带元素，七系附魔沿人物动作帧绘制。
	hud.change_map("0","比奇省",origin,1)
	if not check(hud.set_scene_human(88,remote,0,0,"附魔验收"),"准备附魔人物"): return
	if not check(hud.set_scene_human_attachments(88,0,1),"准备附魔武器外观"): return
	var elemental_colors := [Color8(240,180,100),Color8(100,140,190),Color8(50,70,200),
		Color8(140,200,150),Color8(200,200,200),Color8(160,100,180),Color8(200,220,130)]
	for element in range(7):
		if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_HIT,88,remote.x,remote.y,2|(element<<8))),
			"七系附魔原包入队"): return
		if not check(await wait_until(func(s): return s.drawn_human_spells.any(func(e):
			return e.weapon and e.skill==408+element and e.relative_frame>=3 and e.tint.is_equal_approx(elemental_colors[element]))),
			"附魔效果编号、七系颜色及动作中段绘制正确"): return
		if element==2:
			if not await capture("skill-casting-enchantment"): return
		if not check(await wait_until(func(s): return s.scene_human_actions[88].x==-1),"附魔动作结束"): return
	# 完整等待八秒，确认施法等待超时后可恢复输入，动画结束不能提前解锁。
	if not check(await wait_until(func(s): return s.map_left_delay_ms==0),"超时验收换图门控结束"): return
	root.push_input(mouse,true)
	key(KEY_F1)
	if not check(hud.hud_snapshot().action_reply_pending,"施法进入等待确认"): return
	await settle(1.3)
	if not check(hud.hud_snapshot().action_reply_pending,"起手结束仍等待服务器"): return
	if not check(await wait_until(func(s): return not s.action_reply_pending,7500),"八秒超时解除施法等待"): return
	if not await capture("skill-casting-timeout"): return
	hud.apply_game_message("+FAIL".to_ascii_buffer())
	if not check(hud.hud_snapshot().player_cell==origin,"迟到拒绝恢复保存的施法原点"): return
	hud.queue_free()
	await settle()
	print("SKILL_CASTING_UI_OK")
	quit()
