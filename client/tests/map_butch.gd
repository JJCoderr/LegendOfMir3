# 测试内容：Alt 左键切割姿态、尸体距离判断、原版请求字段和远端蹲姿回包。
extends "map_harvest.gd"

func cut_at(pointer: Vector2) -> void:
	for pressed in [true,false]:
		var button := InputEventMouseButton.new()
		button.button_index=MOUSE_BUTTON_LEFT
		button.position=pointer
		button.pressed=pressed
		button.alt_pressed=true
		root.push_input(button,true)

func run() -> void:
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("options_path","user://map-butch-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	var origin: Vector2i=hud.hud_snapshot().player_cell
	var cell := origin
	for offset in [Vector2i(1,0),Vector2i(-1,0),Vector2i(0,1),Vector2i(0,-1)]:
		if hud.set_scene_human(5,origin+offset,4,1,"目标"):
			cell=origin+offset
			break
	if not check(cell!=origin and hud.set_scene_human_body(5,0,1),"创建相邻目标"): return
	hud.set_character_vitals(100,100,0,0,0,100)
	hud.set_local_actor_identity(77)
	await settle()
	var packets: Array[PackedByteArray]=[]
	hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(packet))
	cut_at(hud.hud_snapshot().scene_human_bounds[5].get_center())
	if not check(packets.size()==1 and decode_request(packets[0]).decode_u16(4)==Mir3HudView.CM_SITDOWN,
		"活体只触发蹲姿"): return
	await settle(1.0)
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_SITDOWN,5,cell.x,cell.y,4)),"远端蹲姿回包入队"): return
	await settle()
	if not check(hud.hud_snapshot().scene_human_actions[5].x==8,"远端进入动作8"): return
	await settle(1.0)
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_DEATH,5,cell.x,cell.y,4)),"接收尸体回包"): return
	await settle()
	cut_at(hud.hud_snapshot().scene_human_bounds[5].get_center())
	if not check(packets.size()==3 and hud.hud_snapshot().player_action==8,"相邻尸体先蹲再发割肉请求"): return
	var sit := decode_request(packets[1])
	var cut := decode_request(packets[2])
	if not check(sit.decode_u16(4)==Mir3HudView.CM_SITDOWN and cut.decode_u16(4)==Mir3HudView.CM_BUTCH
		and cut.decode_u32(0)==5 and cut.decode_u16(6)==origin.x and cut.decode_u16(8)==origin.y
		and cut.decode_u16(10)==sit.decode_u16(8),"核对割肉目标、角色坐标和朝向"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_SITDOWN,77,origin.x,origin.y,cut.decode_u16(10))),
		"本地蹲姿回包"): return
	hud.set_process(false)
	if not await capture("map-butch-crouch"): return
	hud.set_process(true)
	await settle(1.0)
	hud.remove_scene_actor(5)
	var feature := PackedByteArray()
	feature.resize(16)
	feature[0]=3
	feature[2]=20
	var stand := game_reply(Mir3HudView.SM_DIGUP,6,cell.x,cell.y,6)
	stand.append_array(encode_scene_bytes(feature))
	if not check(not hud.apply_game_message(stand.slice(0,27)),"拒绝正文截断的入场包"): return
	if not check(hud.apply_game_message(stand) and hud.hud_snapshot().scene_chicken_steps[6]==-1,
		"怪物20按原坐标入场并站立"): return
	await settle()
	var living: Rect2=hud.hud_snapshot().scene_chicken_bounds[6]
	if not check(living.has_area(),"活鸡在地图上绘制"): return
	if not await capture("map-butch-live-chicken"): return
	# 怪物的 SM_TURN 带新格坐标时触发行动1。重放往返两步，验证排队。
	var outward := game_reply(Mir3HudView.SM_TURN,6,origin.x,origin.y,6 if cell.x>origin.x else 2)
	outward.append_array(encode_scene_bytes(feature))
	var returning := game_reply(Mir3HudView.SM_TURN,6,cell.x,cell.y,2 if cell.x>origin.x else 6)
	returning.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(outward) and hud.apply_game_message(returning)
		and hud.hud_snapshot().scene_chicken_steps[6]==0
		and hud.hud_snapshot().scene_chicken_cells[6]==origin,"先走第一步，第二步留在队列"): return
	await settle(0.25)
	if not await capture("map-chicken-walk"): return
	await settle(0.75)
	if not check(hud.hud_snapshot().scene_chicken_cells[6]==cell and hud.hud_snapshot().scene_chicken_steps[6]>=0,
		"第一步结束后开始返回"): return
	await settle(0.8)
	if not check(hud.hud_snapshot().scene_chicken_cells[6]==cell and hud.hud_snapshot().scene_chicken_steps[6]==-1,
		"走完两步后恢复站立"): return
	var health := game_reply(Mir3HudView.SM_WALK,6,10,0,10)
	if not check(hud.apply_game_message(health) and hud.hud_snapshot().scene_health_values[6][2]==10
		and hud.hud_snapshot().scene_chicken_steps[6]==-1,"怪物11只更新血量"): return
	var miss := game_reply(Mir3HudView.SM_HUMAN_STRUCK,6,10,10,0)
	miss.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(miss) and hud.hud_snapshot().scene_chicken_struck[6]==-1,
		"零伤害不触发受击帧"): return
	if not check(hud.hud_snapshot().map_event_count==0,"零伤害不创建命中特效"): return
	await settle(1.0)
	var strike_feature := feature.duplicate()
	strike_feature[14]=1
	var strike := game_reply(Mir3HudView.SM_HUMAN_STRUCK,6,6,10,4)
	strike.append_array(encode_scene_bytes(strike_feature))
	if not check(hud.apply_game_message(strike) and hud.hud_snapshot().scene_chicken_struck[6]==0
		and hud.hud_snapshot().scene_health_values[6][0]==6,"有伤害时更新血量并播放受击"): return
	if not check(hud.hud_snapshot().map_event_frames.values().has(810),"正文变体1选择 MAGIC 401 和起始帧810"): return
	await settle(0.15)
	if not await capture("map-chicken-struck"): return
	await settle(0.7)
	if not check(hud.hud_snapshot().map_event_count==0,"命中特效播放结束后清理"): return
	if not check(hud.hud_snapshot().scene_chicken_struck[6]==-1,"受击动作结束"): return
	if not check(hud.hud_snapshot().scene_actor_sounds.get(6,{}).get("sample")==404,
		"受击动作播放原版鸡叫声"): return
	var death := game_reply(Mir3HudView.SM_HUMAN_STRUCK,6,0,10,6)
	death.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(death) and hud.hud_snapshot().scene_health_values[6][0]==0,
		"致死受击包触发死亡"): return
	await settle()
	var early: Rect2=hud.hud_snapshot().scene_chicken_bounds[6]
	await settle(1.65)
	if not check(hud.hud_snapshot().scene_actor_sounds.get(6,{}).get("sample")==405
		and hud.hud_snapshot().scene_actor_sounds[6].count==2,"死亡只播放资源表存在的样本"): return
	var corpse: Rect2=hud.hud_snapshot().scene_chicken_bounds[6]
	if not check(corpse.has_area() and corpse!=early,"鸡死亡动画停在尸体帧"): return
	cut_at(corpse.get_center())
	if not check(packets.size()==5 and decode_request(packets[4]).decode_u32(0)==6,"割肉请求以鸡尸体为目标"): return
	hud.set_process(false)
	if not await capture("map-butch-chicken"): return
	hud.set_process(true)
	await settle(1.0)
	var skeleton := game_reply(Mir3HudView.SM_SKELETON,6,cell.x,cell.y,6)
	skeleton.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(skeleton),"骨架回包替换鸡尸体"): return
	await settle()
	var bones: Rect2=hud.hud_snapshot().scene_chicken_bounds[6]
	if not check(bones.has_area() and bones!=corpse,"骨架使用独立资源帧"): return
	if not check(hud.apply_game_message(stand),"处理已死亡对象的站立更新"): return
	await settle()
	if not check(hud.hud_snapshot().scene_chicken_bounds[6]==bones,"站立更新不使骨架复活"): return
	cut_at(bones.get_center())
	if not check(packets.size()==6 and decode_request(packets[5]).decode_u16(4)==Mir3HudView.CM_SITDOWN,
		"骨架只触发蹲姿"): return
	if not await capture("map-butch-skeleton"): return
	hud.queue_free()
	await settle()
	print("MAP_BUTCH_UI_OK")
	quit()
