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
	for offset in [Vector2i(0,1)]:
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
	for i in range(4200):
		if hud.hud_snapshot().scene_actor_sounds.get(6,{}).get("sample")==401: break
		await process_frame
	if not check(hud.hud_snapshot().scene_actor_sounds.get(6,{}).get("sample")==401,
		"站立动作触发原版鸡叫声"): return
	if not await capture("map-chicken-idle-call"): return
	# 怪物的 SM_TURN 带新格坐标时触发行动1。重放往返两步，验证排队。
	var outward := game_reply(Mir3HudView.SM_STRUCK,6,origin.x,origin.y,4)
	outward.append_array(encode_scene_bytes(feature))
	var returning := game_reply(Mir3HudView.SM_TURN,6,cell.x,cell.y,4)
	returning.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(outward) and hud.apply_game_message(returning)
		and hud.hud_snapshot().scene_chicken_steps[6]==0
		and hud.hud_snapshot().scene_chicken_cells[6]==origin,"先面向原侧后退，随后移动留在队列"): return
	await settle(0.25)
	var retreat: Dictionary=hud.hud_snapshot()
	var frame: int=retreat.scene_chicken_steps[6]
	if not check(frame>=1 and retreat.scene_chicken_pixels[6]==cell*Vector2i(48,32)-Vector2i(0,int(32.0*(frame+1)/6)),
		"纵向推退按原版整数除法计算世界像素"): return
	if not await capture("map-chicken-backstep"): return
	await settle(0.75)
	if not check(hud.hud_snapshot().scene_chicken_cells[6]==cell and hud.hud_snapshot().scene_chicken_steps[6]>=0,
		"第一步结束后开始返回"): return
	await settle(0.8)
	if not check(hud.hud_snapshot().scene_chicken_cells[6]==cell and hud.hud_snapshot().scene_chicken_steps[6]==-1,
		"走完两步后恢复站立"): return
	# 第一段动作结束时还有三条消息，第二段首次换帧后按半间隔播放。
	if not check(hud.apply_game_message(outward) and hud.apply_game_message(returning)
		and hud.apply_game_message(outward) and hud.apply_game_message(returning),"连续入队四步"): return
	for i in range(60):
		await process_frame
		if hud.hud_snapshot().scene_chicken_cells[6]==cell: break
	if not check(hud.hud_snapshot().scene_chicken_steps[6]==0,"追帧动作仍从首帧开始"): return
	for i in range(6): await process_frame
	if not check(hud.hud_snapshot().scene_chicken_steps[6]==0,"首帧仍使用原始间隔"): return
	for i in range(9): await process_frame
	if not check(hud.hud_snapshot().scene_chicken_steps[6]>=2,"从第二帧起消化积压"): return
	if not await capture("map-chicken-queue"): return
	await settle(2.2)
	if not check(hud.hud_snapshot().scene_chicken_steps[6]==-1,"连续动作全部结束"): return
	var health := game_reply(Mir3HudView.SM_WALK,6,10,3,10)
	if not check(hud.apply_game_message(health) and hud.hud_snapshot().scene_health_values[6][2]==10
		and hud.hud_snapshot().scene_chicken_steps[6]==-1,"怪物11只更新血量"): return
	var register_bar := game_reply(Mir3HudView.SM_HEALTH_PERCENT,6,0,0,0)
	if not check(hud.apply_game_message(register_bar),"用原包登记怪物血条"): return
	var health_delta := game_reply(Mir3HudView.SM_WALK,6,5,2,10)
	if not check(hud.apply_game_message(health_delta) and hud.hud_snapshot().scene_health_values[6]==PackedInt32Array([5,2,10]),
		"生命和第二数值字段随回包更新"): return
	await settle()
	if not check(hud.hud_snapshot().scene_health_bounds[6].size.x==17 and hud.hud_snapshot().scene_damage_frames.has(6),
		"注册血条显示半血并绘制生命差值"): return
	if not await capture("map-chicken-health"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_801,6,2,0,0)),"怪物状态设置血条校验值"): return
	await settle()
	if not check(hud.hud_snapshot().scene_health_bounds.has(6),"匹配时仍显示血条"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_801,6,0,0,0)),"清除怪物状态位"): return
	await settle()
	if not check(not hud.hud_snapshot().scene_health_bounds.has(6),"校验值不匹配时隐藏血条"): return
	if not await capture("map-chicken-health-hidden"): return
	# 外观包中的状态位也经过血条校验，并随动作队列应用。
	var marked := feature.duplicate()
	marked[4]=2
	var marked_turn := game_reply(Mir3HudView.SM_TURN,6,origin.x,origin.y,0)
	marked_turn.append_array(encode_scene_bytes(marked))
	if not check(hud.apply_game_message(marked_turn) and hud.apply_game_message(returning),"发送带状态的移动与清位回程"): return
	await settle(0.2)
	if not check(hud.hud_snapshot().scene_health_bounds.has(6),"移动更新恢复血条，排队回程尚未清位"): return
	if not await capture("map-chicken-health-moving"): return
	await settle(1.5)
	if not check(not hud.hud_snapshot().scene_health_bounds.has(6),"回程外观清位后隐藏血条"): return
	await settle(1.0)
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
	var sounds_before_death: int=hud.hud_snapshot().scene_actor_sounds[6].count
	var death := game_reply(Mir3HudView.SM_HUMAN_STRUCK,6,0,10,6)
	death.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(death) and hud.hud_snapshot().scene_health_values[6][0]==0,
		"致死受击包触发死亡"): return
	await settle()
	var early: Rect2=hud.hud_snapshot().scene_chicken_bounds[6]
	await settle(1.65)
	if not check(hud.hud_snapshot().scene_actor_sounds.get(6,{}).get("sample")==405
		and hud.hud_snapshot().scene_actor_sounds[6].count==sounds_before_death+1,"死亡只播放资源表存在的样本"): return
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
	# 形状21与鸡共用动作规则，在 Mon-3 取下一组图像。
	hud.remove_scene_actor(6)
	await settle(0.9)
	feature[2]=21
	var deer := game_reply(Mir3HudView.SM_DIGUP,7,cell.x,cell.y,6)
	deer.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(deer),"鹿按原版外观入场"): return
	await settle()
	if not check(hud.hud_snapshot().scene_chicken_bounds[7].size.y>living.size.y,"鹿使用自己的站立图"): return
	if not await capture("map-butch-deer-live"): return
	var deer_death := game_reply(Mir3HudView.SM_DEATH,7,cell.x,cell.y,6)
	deer_death.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(deer_death),"鹿直接显示尸体末帧"): return
	await settle()
	cut_at(hud.hud_snapshot().scene_chicken_bounds[7].get_center())
	if not check(packets.size()==8 and decode_request(packets[7]).decode_u32(0)==7,"鹿尸体成为割肉目标"): return
	if not await capture("map-butch-deer"): return
	# 夜间对比同格、同朝向的活鹿，并验证离场清除光源。
	if not check(hud.change_map("0","比奇省",origin,1),"进入夜间比奇"): return
	await settle()
	var night: PackedColorArray=hud.hud_snapshot().scene_light_samples
	var lit_deer := game_reply(Mir3HudView.SM_DIGUP,7,cell.x,cell.y,6+8*256)
	lit_deer.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(lit_deer),"怪物入场包携带光照高字节"): return
	await settle()
	var lit: PackedColorArray=hud.hud_snapshot().scene_light_samples
	if not check(lit!=night,"活鹿改变夜间光照网格"): return
	if not await capture("map-deer-light"): return
	if not check(hud.apply_game_message(deer_death),"夜间鹿死亡"): return
	await settle()
	if not check(hud.hud_snapshot().scene_light_samples==night,"鹿死亡后移除活动光源"): return
	if not await capture("map-deer-light-out"): return
	# 同一批网络消息内重用对象编号。新鹿应在处理旧离场后留在图中。
	var gone := game_reply(Mir3HudView.SM_DISAPPEAR,7,0,0,0)
	if not check(hud.apply_game_message(gone) and hud.apply_game_message(lit_deer),
		"同帧先离场再入场"): return
	await settle()
	if not check(hud.hud_snapshot().scene_chicken_bounds.has(7)
		and hud.hud_snapshot().scene_light_samples==lit,"编号重用后活鹿及光源保留"): return
	var clear := game_reply(Mir3HudView.SM_CLEAROBJECTS,0,0,0,0)
	if not check(hud.apply_game_message(clear) and hud.apply_game_message(lit_deer),"同帧先清场再入场"): return
	await settle()
	if not check(hud.hud_snapshot().scene_light_samples==lit,"清场之后的新对象保留"): return
	if not await capture("map-deer-reentry"): return
	if not check(hud.apply_game_message(gone),"再次离场"): return
	await settle()
	if not check(not hud.hud_snapshot().scene_chicken_bounds.has(7)
		and hud.hud_snapshot().scene_light_samples==night,"最终离场移除对象和光源"): return
	# 形状23使用 Mon-3 第四组资源，非动物尸体只触发蹲姿。
	if not check(hud.change_map("0","比奇省",origin,0),"回到白天验证普通怪物"): return
	await settle(1.6)
	feature[2]=23
	var oma := game_reply(Mir3HudView.SM_DIGUP,8,cell.x,cell.y,6)
	oma.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(oma),"原始入场包创建半兽人"): return
	await settle()
	var oma_live: Rect2=hud.hud_snapshot().scene_chicken_bounds[8]
	if not check(oma_live.has_area() and oma_live.size.y>living.size.y,"使用半兽人的原版站立资源"): return
	if not await capture("map-oma-entry"): return
	var oma_death := game_reply(Mir3HudView.SM_DEATH,8,cell.x,cell.y,6)
	oma_death.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(oma_death),"显示半兽人尸体末帧"): return
	await settle()
	var oma_body: Rect2=hud.hud_snapshot().scene_chicken_bounds[8]
	var before_oma := packets.size()
	cut_at(oma_body.get_center())
	if not check(oma_body.has_area() and oma_body!=oma_live and packets.size()==before_oma+1
		and decode_request(packets.back()).decode_u16(4)==Mir3HudView.CM_SITDOWN,"半兽人尸体不发送割肉请求"): return
	hud.remove_scene_actor(8)
	await settle(0.9)
	# 猪在 Mon-12 的第十组，验证另一资源库及割肉请求。
	feature[2]=119
	var pig := game_reply(Mir3HudView.SM_DIGUP,9,cell.x,cell.y,6)
	pig.append_array(encode_scene_bytes(feature))
	# GBK「猪」，斜杠后的单字节是原版名称颜色索引。
	pig.append_array(encode_scene_bytes(PackedByteArray([0xd6,0xed,47,249])))
	if not check(hud.apply_game_message(pig),"跨资源库加载猪"): return
	await settle()
	var pig_live: Rect2=hud.hud_snapshot().scene_chicken_bounds[9]
	if not check(pig_live.has_area() and pig_live!=oma_live,"猪显示独立原版资源"): return
	var hover_pig := InputEventMouseMotion.new()
	hover_pig.position=pig_live.get_center()
	root.push_input(hover_pig,true)
	await settle()
	if not check(hud.hud_snapshot().displayed_name_actor==9 and hud.hud_snapshot().displayed_name_color==255,
		"悬停显示入场包中的名称颜色"): return
	if not await capture("map-pig-name"): return
	# 41号包只刷新原对象外观，先跨库切到半兽人再切回猪。
	var transform_feature := feature.duplicate()
	transform_feature[2]=23
	var transform := game_reply(Mir3HudView.SM_FEATURECHANGED,9,0,0,0)
	transform.append_array(encode_scene_bytes(transform_feature))
	if not check(hud.apply_game_message(transform),"用41号包跨库更换怪物外观"): return
	await settle()
	if not check(hud.hud_snapshot().scene_chicken_cells[9]==cell
		and hud.hud_snapshot().scene_chicken_bounds[9]!=pig_live,"同一对象在原格更换图像"): return
	if not await capture("map-monster-feature-change"): return
	transform=game_reply(Mir3HudView.SM_HUMAN_STRUCK,9,8,10,2)
	transform.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(transform) and hud.hud_snapshot().scene_chicken_struck[9]==0
		and hud.hud_snapshot().scene_health_values[9][0]==8,"受击包恢复猪外观、更新生命并进入受击动作"): return
	if not await capture("map-pig-transformed-hit"): return
	await settle(0.8)
	var pig_walk := game_reply(Mir3HudView.SM_TURN,9,origin.x,origin.y,0)
	pig_walk.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(pig_walk) and hud.hud_snapshot().scene_chicken_steps[9]==0,"猪进入行走动作"): return
	if not await capture("map-pig-walk"): return
	await settle(0.9)
	var pig_death := game_reply(Mir3HudView.SM_DEATH,9,cell.x,cell.y,6)
	pig_death.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(pig_death),"显示猪尸体"): return
	await settle()
	cut_at(hud.hud_snapshot().scene_chicken_bounds[9].get_center())
	if not check(packets.size()==before_oma+3 and decode_request(packets.back()).decode_u32(0)==9,
		"猪尸体发送对应编号的割肉请求"): return
	if not await capture("map-butch-pig"): return
	hud.queue_free()
	await settle()
	print("MAP_BUTCH_UI_OK")
	quit()
