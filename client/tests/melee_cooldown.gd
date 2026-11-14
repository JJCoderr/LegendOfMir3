# 定向验收：直接建立相邻怪物，检查动画结束后仍限频、速度回包、超重与挖矿共用门禁。
extends "melee_input.gd"

func run() -> void:
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.set_local_actor_identity(77)
	hud.set_character_vitals(100,100,0,0,0,100)
	hud.set_character_progress(1,0,0,0,100)
	hud.set_character_load_bonuses(Vector2i.ZERO,Vector2i(1,1),Vector2i.ZERO)
	var origin: Vector2i=hud.hud_snapshot().player_cell
	var cell := origin+Vector2i(0,1)
	var feature := PackedByteArray()
	feature.resize(16)
	feature[0]=3
	feature[2]=20
	var entry := game_reply(Mir3HudView.SM_TURN,6,cell.x,cell.y,0)
	entry.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(entry),"直接建立相邻怪物"): return
	await settle()
	var bounds: Rect2=hud.hud_snapshot().scene_chicken_bounds[6]
	var packets: Array[PackedByteArray]=[]
	hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(decode_request(packet)))
	attack_click(bounds.get_center())
	if not check(packets.size()==1 and packets.back().decode_u16(4)==Mir3HudView.CM_HIT,
		"首次普攻发包"): return
	await settle(0.9)
	attack_click(bounds.get_center())
	if not check(packets.size()==1,"动画结束后仍受1386毫秒门禁限制"): return
	await settle(0.55)
	attack_click(bounds.get_center())
	if not check(packets.size()==2,"期满恢复且拒绝点击未延长冷却"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_801,77,0,0,65535))
		and hud.hud_snapshot().melee_speed==-1 and hud.hud_snapshot().melee_interval_ms==1446,
		"原包速度WORD按负数解释"): return
	hud.set_character_load_bonuses(Vector2i.ZERO,Vector2i(2,1),Vector2i.ZERO)
	if not check(hud.hud_snapshot().melee_interval_ms==2946,"超重增加1500毫秒"): return
	await settle(1.5)
	attack_click(bounds.get_center())
	if not check(packets.size()==2,"超重时原间隔结束仍不可攻击"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_808,77,0,0,100)),"更新速度回包"): return
	hud.set_character_load_bonuses(Vector2i.ZERO,Vector2i(1,1),Vector2i.ZERO)
	attack_click(bounds.get_center())
	if not check(packets.size()==3 and hud.hud_snapshot().melee_interval_ms==600,"实时属性解除限频"): return
	await settle(0.1)
	if not await capture("melee-cooldown-attack"): return
	await settle(0.7)
	hud.remove_scene_actor(6)
	hud.set_player_attachments(0,7)
	var pointer := Vector2(origin*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(49,1)
	attack_click(pointer,true)
	if not check(packets.size()==4 and packets.back().decode_u16(4)==Mir3HudView.CM_FORCEHIT,
		"矿镐动作使用同一门禁并发送挖矿请求"): return
	hud.apply_game_message(game_reply(Mir3HudView.SM_801,77,0,0,0))
	await settle(0.9)
	attack_click(pointer,true)
	if not check(packets.size()==4,"挖矿动画结束也不能提前重发"): return
	hud.queue_free()
	await settle()
	print("MELEE_COOLDOWN_UI_OK")
	quit()
