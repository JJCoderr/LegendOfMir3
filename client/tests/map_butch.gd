# 测试内容：怪物资源、动作队列、死亡与割肉请求，以及远端蹲姿回包。
extends "map_harvest.gd"

func cut_at(pointer: Vector2) -> void:
	for pressed in [true,false]:
		var button := InputEventMouseButton.new()
		button.button_index=MOUSE_BUTTON_LEFT
		button.position=pointer
		button.pressed=pressed
		button.alt_pressed=true
		root.push_input(button,true)

func wait_for_monster_step(actor: int, destination: Vector2i) -> bool:
	for i in range(120):
		var snapshot: Dictionary = hud.hud_snapshot()
		if snapshot.scene_chicken_cells.get(actor) == destination and snapshot.scene_chicken_steps.get(actor, -1) >= 0:
			return true
		await process_frame
	return false

func run() -> void:
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("options_path","user://map-butch-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	for arg in OS.get_cmdline_user_args():
		if arg.begins_with("--monster-case="):
			await run_targeted(arg.get_slice("=",1))
			return
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
		and hud.hud_snapshot().scene_monster_walk_poses[6]==5
		and hud.hud_snapshot().scene_chicken_cells[6]==origin,"先面向原侧后退，随后移动留在队列"): return
	for i in range(20):
		if hud.hud_snapshot().scene_chicken_steps[6]>0: break
		await process_frame
	var retreat: Dictionary=hud.hud_snapshot()
	var frame: int=retreat.scene_chicken_steps[6]
	if not check(frame>=1 and retreat.scene_monster_walk_poses[6]==5-frame
		and retreat.scene_chicken_pixels[6]==origin*Vector2i(48,32)+Vector2i(0,5*(5-frame)),
		"纵向推退切换到目的格锚点后按原版整数步长计算"): return
	if not await capture("map-chicken-backstep"): return
	if not check(await wait_for_monster_step(6,cell),"第一步结束后开始返回"): return
	await settle(0.8)
	if not check(hud.hud_snapshot().scene_chicken_cells[6]==cell and hud.hud_snapshot().scene_chicken_steps[6]==-1,
		"走完两步后恢复站立"): return
	# 第一段动作结束时还有三条消息，第二段在首个移动节拍前进两帧。
	if not check(hud.apply_game_message(outward) and hud.apply_game_message(returning)
		and hud.apply_game_message(outward) and hud.apply_game_message(returning),"连续入队四步"): return
	for i in range(60):
		await process_frame
		if hud.hud_snapshot().scene_chicken_cells[6]==cell: break
	if not check(hud.hud_snapshot().scene_chicken_steps[6]==0,"追帧动作仍从首帧开始"): return
	for i in range(15):
		await process_frame
		if hud.hud_snapshot().scene_chicken_steps[6]>0: break
	if not check(hud.hud_snapshot().scene_chicken_steps[6]==2,"积压时首个移动节拍前进两帧"): return
	if not await capture("map-chicken-queue"): return
	await settle(2.2)
	if not check(hud.hud_snapshot().scene_chicken_steps[6]==-1,"连续动作全部结束"): return
	# 行走期间的受击应先排队，后续移动须等受击动作结束。
	# 怪物的14号消息走血量及受击分支，人物仍使用各自的14号攻击逻辑。
	var queued_hit := game_reply(Mir3HudView.SM_HIT,6,9,10,1)
	queued_hit.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(outward) and hud.apply_game_message(queued_hit)
		and hud.apply_game_message(returning) and hud.hud_snapshot().scene_chicken_struck[6]==-1,
		"受击不抢占正在播放的后退"): return
	for i in range(120):
		await process_frame
		if hud.hud_snapshot().scene_chicken_struck[6]>=0: break
	if not check(hud.hud_snapshot().scene_chicken_struck[6]>=0 and hud.hud_snapshot().scene_chicken_cells[6]==origin
		and hud.hud_snapshot().scene_health_values[6][0]==9,"后退完成才应用受击与生命更新"): return
	if not await capture("map-monster-queued-hit"): return
	await settle(1.6)
	if not check(hud.hud_snapshot().scene_chicken_cells[6]==cell and hud.hud_snapshot().scene_chicken_steps[6]==-1,
		"受击完成后才执行返回"): return
	# 怪物1309在队列消费时用原始坐标和朝向执行后退。
	var relay := game_reply(Mir3HudView.SM_HUMAN_STRUCK,6,origin.x,origin.y,4)
	relay.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(relay) and hud.apply_game_message(returning)
		and hud.hud_snapshot().scene_chicken_steps[6]==-1,"1309与回程依次进入队列"): return
	if not check(await wait_for_monster_step(6,origin),"按原坐标执行后退"): return
	if not await capture("map-chicken-relayed-backstep"): return
	if not check(await wait_for_monster_step(6,cell),"已排队的回程先于复制消息执行"): return
	if not check(await wait_for_monster_step(6,origin),"同一对象消费复制后的31号后退"): return
	if not check(hud.apply_game_message(returning),"第二次后退后恢复位置"): return
	await settle(1.7)
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
	var queued_status := game_reply(Mir3HudView.SM_801,6,0,0,0)
	var queued_health := game_reply(Mir3HudView.SM_WALK,6,4,1,10)
	if not check(hud.apply_game_message(marked_turn) and hud.apply_game_message(queued_status)
		and hud.apply_game_message(queued_health) and hud.apply_game_message(returning),"发送移动、清位、血量与回程"): return
	await settle(0.2)
	if not check(hud.hud_snapshot().scene_health_bounds.has(6)
		and hud.hud_snapshot().scene_health_values[6][0]==5,"行走中保留原生命，排队状态尚未清位"): return
	if not await capture("map-chicken-health-moving"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_FEATURECHANGED,6,0,0,0)),
		"怪物41直接清除当前血条校验值"): return
	await settle()
	if not check(not hud.hud_snapshot().scene_health_bounds.has(6)
		and hud.hud_snapshot().scene_chicken_steps[6]>=0,"41在当前动作结束前生效"): return
	await settle(1.5)
	if not check(not hud.hud_snapshot().scene_health_bounds.has(6)
		and hud.hud_snapshot().scene_health_values[6]==PackedInt32Array([4,1,10]),"回程前应用清位与新生命"): return
	await settle(1.0)
	var miss := game_reply(Mir3HudView.SM_HIT,6,10,10,0)
	miss.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(miss) and hud.hud_snapshot().scene_chicken_struck[6]==-1,
		"零伤害不触发受击帧"): return
	if not check(hud.hud_snapshot().map_event_count==0,"零伤害不创建命中特效"): return
	await settle(1.0)
	var strike_feature := feature.duplicate()
	strike_feature[14]=1
	var strike := game_reply(Mir3HudView.SM_HIT,6,6,10,4)
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
	var death := game_reply(Mir3HudView.SM_HIT,6,0,10,6)
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
	# 33 creates a standing object before the reposted message starts action 8.
	var skeleton := game_reply(Mir3HudView.SM_SKELETON,60,cell.x,cell.y,6)
	skeleton.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(skeleton)
		and hud.hud_snapshot().scene_monster_death_frames[60]==-1
		and hud.hud_snapshot().scene_monster_burrow_frames[60]==-1,"33号先建立站立对象"): return
	await settle()
	if not check(hud.hud_snapshot().scene_monster_burrow_frames[60]>=0,"重投后开始动作8"): return
	if not check(hud.remove_scene_actor(60),"清理首次入场的怪物"): return
	# 原尸体仍可用于独立验证超距割肉。
	var before_deer := packets.size()
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
	var deer_death := game_reply(Mir3HudView.SM_27,7,cell.x,cell.y,6)
	deer_death.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(deer_death),"鹿接收27号死亡动作"): return
	await settle(1.65)
	cut_at(hud.hud_snapshot().scene_chicken_bounds[7].get_center())
	if not check(packets.size()==before_deer+2 and decode_request(packets.back()).decode_u32(0)==7,
		"蹲下后发送以鹿尸体为目标的割肉请求"): return
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
	var oma := game_reply(Mir3HudView.SM_800,8,cell.x,cell.y,6)
	oma.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(oma),"原始入场包创建半兽人"): return
	if not check(hud.hud_snapshot().scene_actor_sounds.get(8,{}).get("sample")==109,"800 触发现身音效"): return
	await settle()
	var oma_live: Rect2=hud.hud_snapshot().scene_chicken_bounds[8]
	if not check(oma_live.has_area() and oma_live.size.y>living.size.y,"使用半兽人的原版站立资源"): return
	if not await capture("map-oma-entry"): return
	var oma_death := game_reply(Mir3HudView.SM_27,8,cell.x,cell.y,6)
	oma_death.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(oma_death),"27号启动半兽人死亡"): return
	await settle(1.5)
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
	# 654 更新名字颜色，656 同时更新名字正文和颜色。
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CHANGELIGHT,9,255,0,0)),
		"654更新怪物名色"): return
	await settle()
	if not check(hud.hud_snapshot().displayed_name_actor==9
		and hud.hud_snapshot().displayed_name_color==0xffffff,"654保留名称并切换为白色"): return
	var pig_name := game_reply(Mir3HudView.SM_CHANGENAMECOLOR,9,249,0,0)
	pig_name.append_array(encode_scene_bytes(PackedByteArray([0xd6,0xed])))
	if not check(hud.apply_game_message(pig_name),"656更新怪物名称正文"): return
	await settle()
	if not check(hud.hud_snapshot().displayed_name_color==255,"656应用名字颜色"): return
	# 怪物42号包刷新原对象外观，先跨库切到半兽人再切回猪。
	var transform_feature := feature.duplicate()
	transform_feature[2]=23
	var transform := game_reply(Mir3HudView.SM_USERNAME,9,0,0,0)
	transform.append_array(encode_scene_bytes(transform_feature))
	if not check(hud.apply_game_message(transform),"用怪物42号包跨库更换外观"): return
	await settle()
	if not check(hud.hud_snapshot().scene_chicken_cells[9]==cell
		and hud.hud_snapshot().scene_chicken_bounds[9]!=pig_live,"同一对象在原格更换图像"): return
	if not await capture("map-monster-feature-change"): return
	transform=game_reply(Mir3HudView.SM_HIT,9,8,10,2)
	transform.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(transform) and hud.hud_snapshot().scene_chicken_struck[9]==0
		and hud.hud_snapshot().scene_health_values[9][0]==8,"受击包恢复猪外观、更新生命并进入受击动作"): return
	if not await capture("map-pig-transformed-hit"): return
	await settle(0.8)
	var pig_walk := game_reply(Mir3HudView.SM_TURN,9,origin.x,origin.y,0)
	pig_walk.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(pig_walk) and hud.hud_snapshot().scene_chicken_steps[9]==0,"猪进入行走动作"): return
	var oma_back := game_reply(Mir3HudView.SM_STRUCK,9,cell.x,cell.y,0)
	oma_back.append_array(encode_scene_bytes(transform_feature))
	var pig_turn := game_reply(Mir3HudView.SM_TURN,9,cell.x,cell.y,6)
	pig_turn.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(oma_back) and hud.apply_game_message(pig_turn)
		and hud.hud_snapshot().scene_monster_shapes[9]==119,"排队后退不提前改变猪外观"): return
	if not await capture("map-pig-walk"): return
	if not check(await wait_for_monster_step(9,cell)
		and hud.hud_snapshot().scene_monster_shapes[9]==23,"后退开始时切换到半兽人外观"): return
	if not await capture("map-monster-transform-backstep"): return
	await settle(0.9)
	if not check(hud.hud_snapshot().scene_monster_shapes[9]==119 and hud.hud_snapshot().scene_chicken_steps[9]==-1,
		"后退结束后，同格转向恢复猪外观"): return
	var oma_appearance := game_reply(Mir3HudView.SM_USERNAME,9,0,0,0)
	oma_appearance.append_array(encode_scene_bytes(transform_feature))
	if not check(hud.apply_game_message(oma_appearance) and hud.hud_snapshot().scene_monster_shapes[9]==23,
		"死亡前对象显示半兽人"): return
	var pig_death := game_reply(Mir3HudView.SM_27,9,cell.x,cell.y,6)
	pig_death.append_array(encode_scene_bytes(feature))
	var final_walk := game_reply(Mir3HudView.SM_TURN,9,origin.x,origin.y,0)
	final_walk.append_array(encode_scene_bytes(transform_feature))
	if not check(hud.apply_game_message(final_walk) and hud.apply_game_message(pig_death)
		and hud.hud_snapshot().scene_monster_shapes[9]==23,"死亡回包等待当前行动播放"): return
	await settle(2.2)
	if not check(hud.hud_snapshot().scene_monster_shapes[9]==119 and hud.hud_snapshot().scene_chicken_cells[9]==cell
		and hud.hud_snapshot().scene_chicken_steps[9]==-1,"当前行动结束后应用猪尸体"): return
	cut_at(hud.hud_snapshot().scene_chicken_bounds[9].get_center())
	if not check(packets.size()==before_oma+3 and decode_request(packets.back()).decode_u32(0)==9,
		"猪尸体发送对应编号的割肉请求"): return
	if not await capture("map-pig-death-transform"): return
	hud.remove_scene_actor(9)
	feature[2]=31
	# 形状31按原版固定读取方向0资源，即使入场包指定方向6。
	var emergence := game_reply(Mir3HudView.SM_DIGUP,10,cell.x,cell.y,6)
	emergence.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(emergence),"形状31从 Mon-4 入场"): return
	await settle()
	if not check(hud.hud_snapshot().scene_chicken_bounds[10].has_area(),"钻地前可见"): return
	# 先以猪的资源播放移动，到达后通过受击包换成具备钻地图的形状31。
	var moving_pig := game_reply(Mir3HudView.SM_TURN,10,origin.x,origin.y,0)
	var pig_feature := feature.duplicate()
	pig_feature[2]=119
	moving_pig.append_array(encode_scene_bytes(pig_feature))
	var burrow_shape := game_reply(Mir3HudView.SM_HIT,10,10,10,0)
	burrow_shape.append_array(encode_scene_bytes(feature))
	var burrow := game_reply(Mir3HudView.SM_DIGDOWN,10,cell.x,cell.y,0)
	if not check(hud.apply_game_message(moving_pig) and hud.apply_game_message(burrow_shape)
		and hud.apply_game_message(burrow) and hud.hud_snapshot().scene_monster_burrow_frames[10]==-1,
		"钻地按顺序等待移动与外观刷新"): return
	await settle(1.0)
	if not check(hud.hud_snapshot().scene_monster_burrow_frames[10]>=0
		and hud.hud_snapshot().scene_chicken_bounds[10].has_area(),"钻地过程保留动作图像"): return
	if not await capture("map-monster-burrow"): return
	await settle(1.3)
	if not check(not hud.hud_snapshot().scene_chicken_bounds[10].has_area(),"钻地末帧结束后隐藏"): return
	if not check(hud.apply_game_message(emergence),"同一对象重新出土"): return
	await settle()
	if not check(hud.hud_snapshot().scene_chicken_bounds[10].has_area(),"出土恢复可见"): return
	# 连续到达的钻地、现身和再次钻地必须保持原始顺序。
	var reentry := game_reply(Mir3HudView.SM_806,10,origin.x,origin.y,0)
	reentry.append_array(encode_scene_bytes(feature))
	var second_burrow := game_reply(Mir3HudView.SM_DIGDOWN,10,origin.x,origin.y,0)
	if not check(hud.apply_game_message(burrow) and hud.apply_game_message(reentry)
		and hud.apply_game_message(second_burrow),"连续接收钻地、现身和再次钻地"): return
	await settle(0.3)
	if not check(hud.hud_snapshot().scene_chicken_cells[10]==cell
		and hud.hud_snapshot().scene_monster_burrow_frames[10]>=0
		and hud.hud_snapshot().scene_actor_sounds.get(10,{}).get("sample")!=109,"现身音效等待第一段钻地"): return
	await settle(1.15)
	if not check(hud.hud_snapshot().scene_chicken_cells[10]==origin
		and hud.hud_snapshot().scene_monster_burrow_frames[10]>=0,"现身后仍会消费下一条钻地"): return
	if not await capture("map-monster-queued-reentry"): return
	await settle(1.4)
	if not check(not hud.hud_snapshot().scene_chicken_bounds[10].has_area(),"再次钻地结束后隐藏"): return
	# 骷髅按通用索引读取 Mon-4 第三组，覆盖入场、移动及死亡。
	feature[2]=32
	var skeleton_entry := game_reply(Mir3HudView.SM_DIGUP,11,cell.x,cell.y,6)
	skeleton_entry.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(skeleton_entry),"形状32按通用索引入场"): return
	await settle()
	if not check(hud.hud_snapshot().scene_chicken_bounds[11].has_area(),"骷髅站立帧可见"): return
	var skeleton_walk := game_reply(Mir3HudView.SM_TURN,11,origin.x,origin.y,0)
	skeleton_walk.append_array(encode_scene_bytes(feature))
	var skeleton_burrow := game_reply(Mir3HudView.SM_DEATH,11,cell.x,cell.y,6)
	if not check(hud.apply_game_message(skeleton_walk) and hud.apply_game_message(skeleton_burrow)
		and hud.hud_snapshot().scene_chicken_steps[11]==0,"骷髅行走时死亡包进入队列"): return
	if not await capture("map-skeleton-walk"): return
	for i in range(120):
		if hud.hud_snapshot().scene_monster_burrow_frames[11]>=0: break
		await process_frame
	if not check(hud.hud_snapshot().scene_chicken_steps[11]==-1
		and hud.hud_snapshot().scene_chicken_cells[11]==cell
		and hud.hud_snapshot().scene_monster_burrow_frames[11]>=0
		and hud.hud_snapshot().scene_monster_death_frames[11]==-1,"已有骷髅在32号消息下启动动作8"): return
	# 从原版档案读取两类怪物的站立动画，形状2000覆盖高位资源分组。
	for fixture in [[12, 200, cell], [13, 2000, origin]]:
		var actor: int = fixture[0]
		var shape: int = fixture[1]
		var position: Vector2i = fixture[2]
		feature[2]=shape & 255
		feature[3]=shape >> 8
		var entry := game_reply(Mir3HudView.SM_TURN,actor,position.x,position.y,0)
		entry.append_array(encode_scene_bytes(feature))
		if not check(hud.apply_game_message(entry),"特殊怪物档案入场，形状 %d" % shape): return
	await settle()
	var resource_snapshot: Dictionary = hud.hud_snapshot()
	for actor in [12, 13]:
		if not check(resource_snapshot.scene_monster_shapes[actor]==(200 if actor==12 else 2000)
			and resource_snapshot.scene_chicken_bounds[actor].has_area(),"特殊怪物站立帧可见"): return
	if not await capture("map-monster-resource-families"): return
	# 2001 与 2000 共用 DMon-1 第0组，末位只改变 C2 的染色。
	feature[2]=2001 & 255
	feature[3]=2001 >> 8
	var dyed_entry := game_reply(Mir3HudView.SM_TURN,13,origin.x,origin.y,0)
	dyed_entry.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(dyed_entry)
		and hud.hud_snapshot().scene_monster_shapes[13]==2001
		and hud.hud_snapshot().scene_monster_body_dyes[13]==0x6339,"扩展形状末位选取原版颜色"): return
	if not await capture("map-monster-expanded-dye"): return
	# 普通攻击等待物件移动完成，攻击结束后才能消费后续移动。
	feature[2]=23
	feature[3]=0
	var fighter := game_reply(Mir3HudView.SM_TURN,14,cell.x,cell.y,6)
	fighter.append_array(encode_scene_bytes(feature))
	var approach := game_reply(Mir3HudView.SM_TURN,14,origin.x,origin.y,0)
	approach.append_array(encode_scene_bytes(feature))
	var attack := game_reply(Mir3HudView.SM_FLYAXE,14,origin.x,origin.y,0)
	var attack_retreat := game_reply(Mir3HudView.SM_TURN,14,cell.x,cell.y,4)
	attack_retreat.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(fighter) and hud.apply_game_message(approach)
		and hud.apply_game_message(attack) and hud.apply_game_message(attack_retreat)
		and hud.hud_snapshot().scene_monster_attack_frames[14]==-1,"攻击等待移动完成"): return
	for i in range(120):
		if hud.hud_snapshot().scene_monster_attack_frames[14]>=0: break
		await process_frame
	if not check(hud.hud_snapshot().scene_monster_attack_frames[14]>=0
		and hud.hud_snapshot().scene_chicken_cells[14]==origin,"攻击期间保留原地和后续移动"): return
	if not await capture("map-monster-basic-attack"): return
	await settle(1.6)
	if not check(hud.hud_snapshot().scene_monster_attack_frames[14]==-1
		and hud.hud_snapshot().scene_chicken_cells[14]==cell,"攻击结束后执行后续移动"): return
	# 形状30以第4帧停尸，形状104以第20帧停尸。
	for fixture in [[15, 30, cell], [16, 104, origin]]:
		var actor: int = fixture[0]
		var position: Vector2i = fixture[2]
		feature[2]=fixture[1]
		var corpse_packet := game_reply(Mir3HudView.SM_27,actor,position.x,position.y,0)
		corpse_packet.append_array(encode_scene_bytes(feature))
		if not check(hud.apply_game_message(corpse_packet)
			and hud.hud_snapshot().scene_monster_death_frames[actor]==0,"27号从首帧开始死亡动作"): return
	await settle(2.6)
	if not check(hud.hud_snapshot().scene_monster_death_frames[15]==3
		and hud.hud_snapshot().scene_monster_death_frames[16]==19
		and hud.hud_snapshot().scene_chicken_bounds[15].has_area()
		and hud.hud_snapshot().scene_chicken_bounds[16].has_area(),"两种尸体资源可见"): return
	if not await capture("map-monster-death-frames"): return
	# 形状9的22号攻击创建 MonMagic 的501号独立特效。
	feature[2]=9
	var caster := game_reply(Mir3HudView.SM_TURN,17,cell.x,cell.y,0)
	caster.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(caster)
		and hud.apply_game_message(game_reply(Mir3HudView.SM_FLYAXE,17,cell.x,cell.y,0))
		and hud.hud_snapshot().map_event_frames.values().has(300),"怪物专用攻击从501号首帧播放"): return
	await settle()
	if not await capture("map-monster-attack-effect"): return
	# 23号消息按小端解码目标格，形状33发射520号弹道。
	feature[2]=33
	var ranged_actor := game_reply(Mir3HudView.SM_TURN,18,cell.x,cell.y,0)
	ranged_actor.append_array(encode_scene_bytes(feature))
	var target := PackedByteArray()
	target.resize(8)
	target.encode_u16(0,origin.x)
	target.encode_u16(2,origin.y)
	var ranged := game_reply(Mir3HudView.SM_LIGHTING,18,cell.x,cell.y,0)
	ranged.append_array(encode_scene_bytes(target))
	if not check(hud.apply_game_message(ranged_actor) and hud.apply_game_message(ranged)
		and hud.hud_snapshot().scene_monster_attack_frames[18]==0,"远程攻击解析目标并播放攻击动作"): return
	await settle(0.4)
	if not await capture("map-monster-ranged-attack"): return
	# 9号消息让怪物播放起身动作，并清理死亡状态。
	var rise := game_reply(Mir3HudView.SM_BACKSTEP,15,cell.x,cell.y,0)
	feature[2]=83
	rise.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(rise)
		and hud.hud_snapshot().scene_monster_death_frames[15]==-1
		and hud.hud_snapshot().scene_monster_burrow_frames[15]==0,"起身后恢复活体状态并开始动作8"): return
	if not await capture("map-monster-rise"): return
	# 已存在的活怪物接收1316时，即使目标格相同也要走一步。
	var forced_step := game_reply(Mir3HudView.SM_REVIVE,14,cell.x,cell.y,0)
	feature[2]=23
	forced_step.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(forced_step)
		and hud.hud_snapshot().scene_chicken_steps[14]==0,"1316同格移动也启动动作1"): return
	# 形状83携带状态位1入场时固定显示动作8首帧。
	feature[2]=83
	feature[4]=1
	var hidden_entry := game_reply(Mir3HudView.SM_DIGUP,19,cell.x,cell.y,6)
	hidden_entry.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(hidden_entry),"钻地形状以固定姿势入场"): return
	await settle()
	var hidden_bounds: Rect2=hud.hud_snapshot().scene_chicken_bounds[19]
	await settle(0.4)
	if not check(hidden_bounds.has_area() and hud.hud_snapshot().scene_chicken_bounds[19]==hidden_bounds,
		"固定姿势不随待机时钟换帧"): return
	await settle(0.85)
	if not check(hud.hud_snapshot().scene_monster_burrow_frames[15]==-1
		and hud.hud_snapshot().scene_chicken_bounds[15].has_area(),"起身结束后恢复可见站立"): return
	# 形状141的22号攻击和1317都切换到动作6，并附加 SPELL 565。
	feature[2]=141
	feature[4]=0
	var action_six_actor := game_reply(Mir3HudView.SM_TURN,20,origin.x,origin.y,0)
	action_six_actor.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(action_six_actor)
		and hud.apply_game_message(game_reply(Mir3HudView.SM_FLYAXE,20,origin.x,origin.y,0))
		and hud.apply_game_message(game_reply(Mir3HudView.SM_1317,20,cell.x,cell.y,6)),
		"普通攻击和1317依次处理"): return
	if not check(hud.hud_snapshot().scene_monster_attack_actions[20]==6
		and hud.hud_snapshot().map_event_frames.values().has(1390),"形状141的动作6附加565号特效"): return
	if not await capture("map-monster-action-six"): return
	for i in range(120):
		await process_frame
		if hud.hud_snapshot().scene_monster_attack_frames[20]==0: break
	if not check(hud.hud_snapshot().scene_monster_attack_actions[20]==6
		and hud.hud_snapshot().scene_monster_attack_frames[20]==0
		and hud.hud_snapshot().scene_chicken_cells[20]==origin,"1317重新开始动作6并保留位置"): return
	# 同编号对象可在同一帧重现，旧附身效果应在离场时立即清理。
	if not check(hud.remove_scene_actor(20)
		and not hud.hud_snapshot().map_event_frames.values().has(1390),
		"离场立即清除动作6附身效果"): return
	if not check(hud.apply_game_message(action_six_actor)
		and not hud.hud_snapshot().map_event_frames.values().has(1390),
		"同编号重入不继承旧对象附身效果"): return
	# 42E9A0 允许这些消息在携带外观时首次创建类型3对象。
	feature[2]=23
	feature[3]=0
	feature[4]=0
	for fixture in [[21, Mir3HudView.SM_BACKSTEP], [22, Mir3HudView.SM_WALK],
		[23, Mir3HudView.SM_RUN], [24, Mir3HudView.SM_801],
		[25, Mir3HudView.SM_ACTOR_LIGHT], [26, Mir3HudView.SM_808]]:
		var actor: int = fixture[0]
		var opcode: int = fixture[1]
		var entry := game_reply(opcode,actor,cell.x,cell.y,0)
		entry.append_array(encode_scene_bytes(feature))
		if not check(hud.apply_game_message(entry)
			and hud.hud_snapshot().scene_monster_shapes.get(actor)==23,
			"首次收到 %d 时按外观创建怪物" % opcode): return
	await settle()
	var initial_actors: Dictionary = hud.hud_snapshot()
	if not check(initial_actors.scene_monster_burrow_frames[21]>=0
		and initial_actors.scene_health_values[22]==PackedInt32Array([cell.x,cell.y,0])
		and initial_actors.scene_chicken_steps[23]==-1,"首次消息出队后分别应用起身、血量和站立"): return
	var status_update := game_reply(Mir3HudView.SM_808,24,0,0,0)
	feature[2]=21
	status_update.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(status_update)
		and hud.hud_snapshot().scene_monster_shapes[24]==23,"已有对象的状态包忽略外观正文"): return
	if not await check_rise_health(cell,feature): return
	if not await check_special_effect(origin,cell,feature): return
	for actor in [21,22,24,25,26,27,28]:
		hud.remove_scene_actor(actor)
	if not await check_status_blend(): return
	await finish()

func finish() -> void:
	hud.queue_free()
	await settle()
	print("MAP_BUTCH_UI_OK")
	quit()

func run_targeted(case: String) -> void:
	var origin: Vector2i = hud.hud_snapshot().player_cell
	var cell := origin+Vector2i(0,1)
	var feature := PackedByteArray()
	feature.resize(16)
	feature[0]=3
	feature[2]=23
	match case:
		"rise-health":
			if not await check_rise_health(cell,feature): return
		"special-effect":
			var entry := game_reply(Mir3HudView.SM_TURN,27,cell.x,cell.y,0)
			entry.append_array(encode_scene_bytes(feature))
			if not check(hud.apply_game_message(entry),"建立定向回放怪物"): return
			if not await check_special_effect(origin,cell,feature): return
		"status-blend":
			var entry := game_reply(Mir3HudView.SM_TURN,23,cell.x,cell.y,0)
			entry.append_array(encode_scene_bytes(feature))
			if not check(hud.apply_game_message(entry),"建立定向回放怪物"): return
			if not await check_status_blend(): return
		_:
			check(false,"未知怪物测试用例 %s" % case)
			return
	await finish()


func check_rise_health(cell: Vector2i, feature: PackedByteArray) -> bool:
	# 27号清空填充但保留生命值，9号起身后由下一条血量回包恢复填充。
	feature[2]=30
	feature[3]=0
	feature[4]=0
	var bar_actor := game_reply(Mir3HudView.SM_TURN,27,cell.x,cell.y,0)
	bar_actor.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(bar_actor)
		and hud.apply_game_message(game_reply(Mir3HudView.SM_WALK,27,5,0,10))
		and hud.apply_game_message(game_reply(Mir3HudView.SM_HEALTH_PERCENT,27,0,0,0)),
		"建立带半血血条的怪物"): return false
	await settle()
	if not check(hud.hud_snapshot().scene_health_bounds[27].size.x==17,"死亡前显示半血填充"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_27,27,cell.x,cell.y,0)),
		"死亡动作开始"): return false
	var bar_rise := game_reply(Mir3HudView.SM_BACKSTEP,27,cell.x,cell.y,0)
	feature[2]=83
	bar_rise.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(bar_rise),"死亡期间排队起身"): return false
	var rise_deadline := Time.get_ticks_msec()+3500
	while Time.get_ticks_msec()<rise_deadline:
		var state: Dictionary = hud.hud_snapshot()
		if state.scene_monster_death_frames[27]==-1 and state.scene_monster_burrow_frames[27]==-1: break
		await process_frame
	await settle()
	var revived: Dictionary = hud.hud_snapshot()
	if not check(revived.scene_monster_death_frames[27]==-1 and revived.scene_monster_burrow_frames[27]==-1
		and revived.scene_health_values[27][0]==5 and revived.scene_health_bounds[27].size.x==0,
		"起身后保留生命值并显示空血条"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_WALK,27,5,0,10)),
		"起身后接收血量回包"): return false
	await settle()
	if not check(hud.hud_snapshot().scene_health_bounds[27].size.x==17,"血量回包恢复半血填充"): return false
	return true

func check_special_effect(origin: Vector2i, cell: Vector2i, feature: PackedByteArray) -> bool:
	# 657分支1立即切换攻击，越过当前移动但保留已排队消息。
	feature[2]=23
	var interrupt_entry := game_reply(Mir3HudView.SM_TURN,28,cell.x,cell.y,0)
	interrupt_entry.append_array(encode_scene_bytes(feature))
	var interrupt_step := game_reply(Mir3HudView.SM_TURN,28,origin.x,origin.y,0)
	interrupt_step.append_array(encode_scene_bytes(feature))
	var interrupt_body := PackedByteArray()
	interrupt_body.resize(16)
	interrupt_body.encode_u32(12,1)
	var interrupt := game_reply(Mir3HudView.SM_CHARSTATUSCHANGE,28,origin.x,origin.y,0)
	interrupt.append_array(encode_scene_bytes(interrupt_body))
	if not check(hud.apply_game_message(interrupt_entry) and hud.apply_game_message(interrupt_step)
		and hud.hud_snapshot().scene_chicken_steps[28]==0,"开始待打断的移动"): return false
	if not check(hud.apply_game_message(interrupt)
		and hud.hud_snapshot().scene_chicken_steps[28]==-1
		and hud.hud_snapshot().scene_monster_attack_actions[28]==2,
		"657立即打断移动并启动攻击"): return false
	# 分支4只转向，保留怪物所在格，即使消息头带了另一个位置。
	interrupt_body.encode_u32(0,cell.x)
	interrupt_body.encode_u32(4,cell.y)
	interrupt_body.encode_u32(12,4)
	var fixed_target := game_reply(Mir3HudView.SM_CHARSTATUSCHANGE,28,cell.x,cell.y,2)
	fixed_target.append_array(encode_scene_bytes(interrupt_body))
	if not check(hud.apply_game_message(fixed_target)
		and hud.hud_snapshot().scene_chicken_cells[28]==origin,
		"657分支4保留施法者所在格"): return false
	# 657分支25向南依次布置九格，发射间隔为150毫秒。
	var wave_body := PackedByteArray()
	wave_body.resize(16)
	wave_body.encode_u32(0,cell.x)
	wave_body.encode_u32(4,cell.y+5)
	wave_body.encode_u32(12,25)
	var wave := game_reply(Mir3HudView.SM_CHARSTATUSCHANGE,27,cell.x,cell.y,0)
	wave.append_array(encode_scene_bytes(wave_body))
	if not check(hud.apply_game_message(wave),"657分支25启动地面波"): return false
	var wave_delays: Dictionary = hud.hud_snapshot().map_ground_wave_delays
	if not check(wave_delays.size()==9,"地面波布置九格"): return false
	for index in range(9):
		if not check(wave_delays.get(cell+Vector2i(0,index+1),-1)==150*index,
			"第%d格使用原版启动延时" % (index+1)): return false
	await settle(0.6)
	if not await capture("map-monster-ground-wave"): return false
	return true

func check_status_blend() -> bool:
	# 801的高WORD依次设置灰阶、减速色与半透明，核对重叠位优先级。
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_801,23,0,0x0400,0))
		and hud.hud_snapshot().scene_monster_status_colors[23]==0,"灰阶状态选择模式0"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_801,23,0,0x0c00,0))
		and hud.hud_snapshot().scene_monster_status_colors[23]==38079,"减速色覆盖灰阶"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_801,23,0,0x0c02,0))
		and hud.hud_snapshot().scene_monster_status_colors[23]==1,"半透明状态优先于颜色状态"): return false
	if not await capture("map-monster-status-blend"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_801,23,0,0,0))
		and hud.hud_snapshot().scene_monster_status_colors[23]==65535,"清除状态后恢复普通显示"): return false
	return true
