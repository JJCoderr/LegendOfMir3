# 本地武器附魔：装备原包来源、特殊覆盖色、普通元素、无强度关闭及动作结束清理。
extends "skill_casting.gd"

func weapon_packet(element: int, power: int, special: int) -> PackedByteArray:
	var raw := PackedByteArray()
	raw.resize(100)
	raw[0]=83 # Sword
	raw[30]=5
	raw.encode_u16(36,1043)
	raw[49]=element
	raw.encode_u16(50,power)
	raw.encode_u32(62,101)
	raw[97]=special
	var packet := game_reply(Mir3HudView.SM_SENDUSEITEMS,0,0,0,1)
	packet.append_array("1/".to_ascii_buffer())
	packet.append_array(encode_scene_bytes(raw))
	packet.append(47)
	return packet

func run() -> void:
	Input.use_accumulated_input=false
	var ids := [408,409,410,411,412,413,414]
	for argument in OS.get_cmdline_user_args():
		if argument.begins_with("--skill="):
			var value := argument.trim_prefix("--skill=")
			if not check(value.is_valid_int() and int(value) in ids,"本地附魔编号必须在408至414"): return
			ids=[int(value)]
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("options_path","user://weapon-enchantment-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.set_local_actor_identity(77)
	hud.set_character_vitals(100,100,200,200,0,100)
	var origin: Vector2i=hud.hud_snapshot().player_cell+Vector2i(0,2)
	hud.change_map("0","比奇省",origin,1)
	if not check(await wait_until(func(s): return s.map_left_delay_ms==0),"等待附魔地图门控"): return
	hud.client_packet_ready.connect(func(_packet): hud.apply_game_message("+GOOD".to_ascii_buffer()))
	var colors := [Color8(240,180,100),Color8(100,140,190),Color8(50,70,200),
		Color8(140,200,150),Color8(200,200,200),Color8(160,100,180),Color8(200,220,130)]
	for effect in ids:
		var element: int=effect-407
		# 普通附魔、特殊附魔覆盖不同底色、特殊字段有符号负值退回普通附魔。
		for values in [[element,1,0],[element%7+1,0,element],[element,256,128]]:
			if not check(hud.apply_game_message(weapon_packet(values[0],values[1],values[2])),"注入武器槽原包"): return
			var pointer := Vector2((origin+Vector2i(1,0))*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(1,1)
			attack_click(pointer,true)
			if not check(await wait_until(func(s): return s.drawn_human_spells.any(func(e):
				return e.weapon and e.skill==effect and e.relative_frame>=3 and e.tint.is_equal_approx(colors[element-1]))),
				"本地附魔读取装备字段并按优先级选色"): return
			if values[2]==element and not await capture("skill-local-enchantment-%d" % effect): return
			if values[2]==element:
				# 附魔挥击后排队的碰撞折返不能沿用上一次武器贴图。
				if not check(hud.apply_game_message(motion_reply(Mir3HudView.SM_BUMP,77,origin+Vector2i(1,0),2)),"附魔期间碰撞回包入队"): return
				if not check(await wait_until(func(s): return s.player_action==7 and s.forced_motion_opcode==0),"附魔后进入碰撞折返动作"): return
				await process_frame
				if not check(hud.hud_snapshot().drawn_human_spells.is_empty(),"碰撞折返不残留武器附魔"): return
			if not check(await wait_until(func(s): return s.player_action==-1),"等待本地挥击结束"): return
			await settle(1.5) # 等级1的普通挥击准入间隔接近1400ms，不能过早再次点击。
	# 有元素但强度WORD为0时，不产生普通附魔；上一次附魔不得残留。
	if not check(hud.apply_game_message(weapon_packet(3,0,0)),"清除附魔强度"): return
	attack_click(Vector2((origin+Vector2i(1,0))*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(1,1),true)
	if not check(await wait_until(func(s): return s.player_action>=0),"无附魔时仍有挥击动作"): return
	await settle(0.2)
	if not check(hud.hud_snapshot().drawn_human_spells.is_empty(),"普通强度为零不残留附魔"): return
	hud.queue_free()
	await settle()
	print("LOCAL_WEAPON_ENCHANTMENT_OK "+str(ids))
	quit()
