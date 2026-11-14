# 测试内容：怪物资源、动作队列、死亡与割肉请求，以及远端蹲姿回包。
extends "map_harvest.gd"

var showcase_title: Label
var showcase_progress: Label
var showcase_report: Array = []
var showcase_index := 0
var showcase_total := 0
var showcase_rng := RandomNumberGenerator.new()
var hit_skills: Array[int] = []
var hit_skill_bag: Array[int] = []
var last_hit_skill := -1
const SHOWCASE_ORIGIN := Vector2i(425,370)

func run() -> void:
	var scenario := "gallery"
	var selected: Array[int] = []
	for arg in OS.get_cmdline_user_args():
		if arg.begins_with("--monster-scenario="): scenario=arg.get_slice("=",1)
		if arg.begins_with("--shapes="):
			for value in arg.get_slice("=",1).split(","): selected.append(int(value))
	if scenario=="regression":
		await run_regression()
		return
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("showcase_monster_names",true)
	hud.set_meta("map_spawn_cell",SHOWCASE_ORIGIN)
	hud.set_meta("options_path","user://monsters-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	var layer := CanvasLayer.new()
	layer.layer=100
	root.add_child(layer)
	var panel := PanelContainer.new()
	var backdrop := StyleBoxFlat.new()
	backdrop.bg_color=Color(0,0,0,0.40)
	panel.add_theme_stylebox_override("panel",backdrop)
	panel.size=Vector2(520,30)
	panel.position=Vector2(8,8)
	panel.mouse_filter=Control.MOUSE_FILTER_IGNORE
	layer.add_child(panel)
	var row := HBoxContainer.new()
	row.mouse_filter=Control.MOUSE_FILTER_IGNORE
	panel.add_child(row)
	var font := SystemFont.new()
	font.font_names=PackedStringArray(["Microsoft YaHei","Noto Sans CJK SC","sans-serif"])
	showcase_progress=Label.new()
	showcase_title=Label.new()
	var input: Control=hud.get_node("ChatInput")
	for label in [showcase_progress,showcase_title]:
		label.mouse_filter=Control.MOUSE_FILTER_IGNORE
		label.add_theme_font_override("font",font)
		label.add_theme_font_size_override("font_size",16)
		label.add_theme_color_override("font_color",input.get_theme_color("font_color"))
		row.add_child(label)
	await settle()
	showcase_rng.randomize()
	hud.set_local_actor_identity(77)
	# 从技能全集数据选择实际有施放资源的项目，不把被动/空资源伪装成魔法。
	var skill_catalog: Dictionary=JSON.parse_string(FileAccess.get_file_as_string("res://../tests/skill_common_cases.json"))
	for skill in skill_catalog.cases:
		if skill.reply=="effect" and skill.magic!=null and int(skill.magic[1])>int(skill.magic[0]):
			hit_skills.append(int(skill.id))
	# 独立专项验证的技能（普通技能表以外）。
	for skill in [1,9,10,53,73,110,113,150]:
		if not skill in hit_skills: hit_skills.append(skill)
	print("MONSTER_HIT_SKILL_POOL ",hit_skills)
	if scenario=="maps":
		await showcase_maps()
		return
	var catalog: Dictionary=JSON.parse_string(FileAccess.get_file_as_string("res://../tests/monster_cases.json"))
	for item in catalog.monsters:
		if (selected.is_empty() or int(item.shape) in selected) and not item.alias: showcase_total+=1
	for item in catalog.monsters:
		if not selected.is_empty() and not int(item.shape) in selected: continue
		if not item.alias: showcase_index+=1
		if not await showcase_monster(item): return
	var output := ProjectSettings.globalize_path("res://../build/monster-report.json")
	var file := FileAccess.open(output,FileAccess.WRITE)
	file.store_string(JSON.stringify(showcase_report,"\t"))
	file.close()
	print("MONSTER_GALLERY_OK count=",showcase_report.size()," report=",output)
	hud.queue_free()
	layer.queue_free()
	await settle()
	quit()

func monster_packet(opcode: int, actor: int, cell: Vector2i, facing: int, shape: int, hit_variant: int = 0, status: int = 0) -> PackedByteArray:
	var feature := PackedByteArray()
	feature.resize(16)
	feature[0]=3
	feature.encode_u16(2,shape)
	feature.encode_u32(4,status)
	feature.encode_u16(14,hit_variant)
	var packet := game_reply(opcode,actor,cell.x,cell.y,facing)
	packet.append_array(encode_scene_bytes(feature))
	return packet


func showcase_maps() -> void:
	var selected_map := ""
	var start_index := 1
	for arg in OS.get_cmdline_user_args():
		if arg.begins_with("--monster-map="): selected_map=arg.get_slice("=",1)
		if arg.begins_with("--start-index="): start_index=maxi(1,int(arg.get_slice("=",1)))
	var catalog: Dictionary=JSON.parse_string(FileAccess.get_file_as_string("res://../tests/monster_maps.json"))
	var groups: Array = []
	# 按显式区域归并大图、楼层、侧室和首领房，外观在区域内去重。
	var families := {}
	for entry in catalog.maps:
		var family: String=entry.family
		if not families.has(family):
			families[family]=groups.size()
			groups.append({"maps":[],"name":entry.get("family_name",entry.name),"monsters":[],"shapes":[],"unavailable":[]})
		var group: Dictionary=groups[families[family]]
		group.maps.append(entry.map)
		for monster in entry.monsters:
			if not monster.available:
				group.unavailable.append({"map":entry.map,"name":monster.name,"shape":monster.shape,
					"spawn_source":monster.spawn_source,"reason":"unresolved_shape" if monster.shape==null else "unsupported_shape"})
				continue
			if int(monster.shape) in group.shapes: continue
			group.shapes.append(int(monster.shape))
			group.monsters.append(monster)
	if not selected_map.is_empty():
		groups=groups.filter(func(group): return selected_map in group.maps)
	showcase_total=groups.size()
	for group in groups:
		showcase_index+=1
		if showcase_index<start_index: continue
		print("MONSTER_MAP_BEGIN index=",showcase_index," total=",showcase_total," map=",group.maps," name=",group.name)
		if not await showcase_map_group(group): return
	print("MONSTER_MAPS_OBSERVED groups=",groups.size()," unresolved_names=",catalog.mapping_unresolved.size())
	quit()

func showcase_map_group(group: Dictionary) -> bool:
	if not check(hud.change_map("0","比奇省",SHOWCASE_ORIGIN,1),"清理上一组怪物与特效"): return false
	if not check(hud.hud_snapshot().scene_monster_shapes.is_empty(),"切组清除旧怪物"): return false
	# 每轮地图独立随机；不沿用上一轮剩余的技能顺序。
	showcase_rng.randomize()
	hit_skill_bag.clear()
	var report := {"family":group.name,"maps":group.maps,"rng_state":showcase_rng.state,"actors":[],"missing":group.unavailable.duplicate(true)}
	showcase_report.append(report)
	var item := {"name":group.name,"shape":-1}
	var actors: Array = []
	var occupied: Array[Vector2i] = []
	var camera: Vector2i=hud.hud_snapshot().map_camera_pixel
	var index := 0
	for monster in group.monsters:
		var cell := SHOWCASE_ORIGIN
		var actor := 7000+index
		index+=1
		var shape := int(monster.shape)
		var behavior: Dictionary=hud.monster_behavior_profile(shape)
		var placed := false
		for attempt in range(80):
			var pixel := Vector2i(showcase_rng.randi_range(90,690),showcase_rng.randi_range(210,370))+camera
			cell=Vector2i(pixel.x/48,pixel.y/32)
			if cell in occupied or cell==SHOWCASE_ORIGIN: continue
			if hud.apply_game_message(monster_packet(behavior.birth_opcode,actor,cell,showcase_rng.randi_range(0,7),shape,0,behavior.birth_status)):
				placed=true
				occupied.append(cell)
				break
		if placed:
			var record := {"actor":actor,"shape":shape,"cell":cell,"name":monster.name,"walked":false,"behavior":behavior}
			actors.append(record)
			report.actors.append(record)
			if not check(hud.hud_snapshot().scene_monster_shapes.get(actor)==shape,"入场保留数据库外观"): return false
			hud.apply_game_message(game_reply(Mir3HudView.SM_HEALTHSPELLCHANGED,actor,100,0,100))
			var name_packet := game_reply(Mir3HudView.SM_USERNAME,actor,255,0,0)
			name_packet.append_array(encode_scene_bytes(PackedByteArray(monster.name_bytes)))
			if not check(hud.apply_game_message(name_packet),"原版怪物姓名包"): return false
		else:
			report.missing.append({"shape":shape,"name":monster.name,"reason":"no_valid_entry_in_80_attempts"})
			print("MONSTER_MAP_UNAVAILABLE map=",group.maps," shape=",shape," cell=",cell)
	show_phase(item,"同时出生 · %d种外观 · 仓库南侧展示场" % actors.size())
	await settle(1.2)
	if not await capture("monster-map-%s-birth" % group.maps[0]): return false
	var waking: Array=actors.filter(func(actor): return int(actor.behavior.wake_opcode)!=0)
	if not waking.is_empty():
		show_phase(item,"特殊出生 / 苏醒")
		for actor in waking:
			if not check(hud.hud_snapshot().scene_monster_dormant[actor.actor],"C++入场休眠姿势 shape=%d profile=%s" % [actor.shape,actor.behavior]): return false
			if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CHARSTATUSCHANGE,actor.actor,0,0,0)),"服务器解除休眠状态"): return false
			if not check(hud.apply_game_message(monster_packet(actor.behavior.wake_opcode,actor.actor,actor.cell,showcase_rng.randi_range(0,7),actor.shape)),"特殊苏醒消息"): return false
		await settle(0.3)
		if not await capture("monster-map-%s-awake" % group.maps[0]): return false
		var deadline := Time.get_ticks_msec()+5000
		while waking.any(func(actor): return hud.hud_snapshot().scene_monster_burrow_frames[actor.actor]>=0) and Time.get_ticks_msec()<deadline:
			await process_frame
		if not check(waking.all(func(actor): return hud.hud_snapshot().scene_monster_burrow_frames[actor.actor]==-1 and not hud.hud_snapshot().scene_monster_dormant[actor.actor]),"C++特殊出生完成后恢复活动"): return false
	show_phase(item,"行走")
	# 独立随机游走；包接收端验证阻挡，展示层限制可视范围与占位。
	var directions := [Vector2i(0,-1),Vector2i(1,-1),Vector2i(1,0),Vector2i(1,1),Vector2i(0,1),Vector2i(-1,1),Vector2i(-1,0),Vector2i(-1,-1)]
	for step in range(4):
		for actor in actors:
			var cell: Vector2i=hud.hud_snapshot().scene_chicken_cells[actor.actor]
			var visible_directions: Array=actor.behavior.visible_walk_directions
			if visible_directions.is_empty():
				actor.walk_unavailable_reason="incomplete_walk_or_idle_resources"
				continue
			for attempt in range(8):
				var facing: int=visible_directions[showcase_rng.randi_range(0,visible_directions.size()-1)]
				var destination: Vector2i=cell+directions[facing]
				var pixel := Vector2(destination*Vector2i(48,32)-camera)
				if not Rect2(75,185,625,205).has_point(pixel) or destination in occupied: continue
				if hud.apply_game_message(monster_packet(Mir3HudView.SM_WALK,actor.actor,destination,facing,actor.shape)):
					occupied.erase(cell)
					actor.cell=hud.hud_snapshot().scene_chicken_cells[actor.actor]
					actor.walked=actor.walked or actor.cell!=cell
					occupied.append(actor.cell)
					break
		await settle(0.8)
	if not await capture("monster-map-%s-walk" % group.maps[0]): return false
	show_phase(item,"攻击其他目标（含自身魔法）")
	for i in range(actors.size()):
		var actor: Dictionary=actors[i]
		var target: Dictionary=actors[(i+showcase_rng.randi_range(1,maxi(1,actors.size()-1)))%actors.size()]
		var target_cell: Vector2i=target.cell if actors.size()>1 else SHOWCASE_ORIGIN
		var delta: Vector2i=target_cell-actor.cell
		var facing := directions.find(Vector2i(signi(delta.x),signi(delta.y)))
		if facing<0: facing=4
		if int(actor.behavior.attack_opcode)==Mir3HudView.SM_FLYAXE:
			var body := PackedByteArray()
			body.resize(8)
			body.encode_u16(0,target_cell.x)
			body.encode_u16(2,target_cell.y)
			body.encode_u32(4,target.actor if actors.size()>1 else 0)
			var packet := game_reply(Mir3HudView.SM_FLYAXE,actor.actor,actor.cell.x,actor.cell.y,facing)
			packet.append_array(encode_scene_bytes(body))
			actor.attack_accepted=hud.apply_game_message(packet)
		else:
			actor.attack_accepted=hud.apply_game_message(game_reply(Mir3HudView.SM_HIT,actor.actor,actor.cell.x,actor.cell.y,facing))
	await settle(0.5)
	if not await capture("monster-map-%s-attack" % group.maps[0]): return false
	await settle(2.3)
	show_phase(item,"被攻击")
	# 本地图技能池整轮无重复；跨地图相邻也不重复。
	for actor in actors:
		if hit_skill_bag.is_empty():
			hit_skill_bag.assign(hit_skills)
			for i in range(hit_skill_bag.size()-1,0,-1):
				var j := showcase_rng.randi_range(0,i)
				var saved := hit_skill_bag[i]
				hit_skill_bag[i]=hit_skill_bag[j]
				hit_skill_bag[j]=saved
			if hit_skill_bag.back()==last_hit_skill:
				var saved := hit_skill_bag[0]
				hit_skill_bag[0]=hit_skill_bag.back()
				hit_skill_bag[hit_skill_bag.size()-1]=saved
		actor.hit_skill=hit_skill_bag.pop_back()
		last_hit_skill=actor.hit_skill
		var magic := game_reply(Mir3HudView.SM_MAGICFIRE,77,actor.cell.x,actor.cell.y,actor.hit_skill)
		var target_body := PackedByteArray()
		target_body.resize(4)
		target_body.encode_u32(0,actor.actor)
		magic.append_array(encode_scene_bytes(target_body))
		if not check(hud.apply_game_message(magic),"随机技能回包入队"): return false
		actor.struck_accepted=hud.apply_game_message(monster_packet(Mir3HudView.SM_STRUCK,actor.actor,Vector2i(70,100),30,actor.shape))
		actor.struck_at=Time.get_ticks_msec()
		actor.death_delay=showcase_rng.randf_range(0.3,2.0)
		actor.death_at=actor.struck_at+int(actor.death_delay*1000.0)
	await settle(0.2)
	if not await capture("monster-map-%s-struck" % group.maps[0]): return false
	for actor in actors:
		if actor.struck_accepted:
			if not check(hud.hud_snapshot().scene_health_values[actor.actor][0]==70,"受击后生命为70"): return false
	show_phase(item,"随机死亡及死亡魔法 → 尸体驻留")
	var death_order: Array=actors.duplicate()
	death_order.sort_custom(func(a,b): return a.death_at<b.death_at)
	for actor in death_order:
		# 以本怪物受击为起点，两秒内发送死亡，不叠加独立死亡阶段等待。
		while Time.get_ticks_msec()<actor.death_at:
			await process_frame
		actor.death_accepted=hud.apply_game_message(monster_packet(Mir3HudView.SM_NOWDEATH,actor.actor,actor.cell,4,actor.shape))
		actor.death_sent_after_ms=Time.get_ticks_msec()-actor.struck_at
	await settle(0.35)
	if not await capture("monster-map-%s-death" % group.maps[0]): return false
	await settle(3.0)
	for actor in actors:
		actor.death_frame=hud.hud_snapshot().scene_monster_death_frames[actor.actor]
		if actor.death_accepted and not check(actor.death_frame>=0,"接受死亡后保留尸体动作"): return false
	var report_file := FileAccess.open("res://../build/monster-map-report.json",FileAccess.WRITE)
	if not check(report_file!=null,"写入逐系列验收记录"): return false
	report_file.store_string(JSON.stringify(showcase_report,"\t"))
	report_file.close()
	return true

func show_phase(item: Dictionary, phase: String) -> void:
	showcase_progress.text="(%d/%d)" % [showcase_index,showcase_total]
	showcase_title.text="%s · %s" % [item.name,phase]
	print("MONSTER_PHASE shape=",item.shape," phase=",phase)

func showcase_monster(item: Dictionary) -> bool:
	# 通过协议驱动真实场景；资源库别名不伪装为独立怪物。
	var result := {"shape":item.shape,"name":item.name,"stages":[]}
	showcase_report.append(result)
	if item.alias:
		result["status"]="resource_alias"
		return true
	var shape := int(item.shape)
	var actor := 6000+shape
	var cell := SHOWCASE_ORIGIN+Vector2i(2,3)
	if not check(hud.change_map("0","比奇省",SHOWCASE_ORIGIN,1),"准备仓库南侧怪物展示空地"): return false
	show_phase(item,"出生 / 站立")
	if not hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,cell,4,shape)):
		result["status"]="entry_resource_unavailable"
		show_phase(item,"无法创建：外观资源或初始化条件不满足（未计通过）")
		await settle(0.35)
		return true
	if not check(hud.hud_snapshot().scene_monster_shapes.get(actor)==shape,"出生外观与协议一致"): return false
	result.stages.append("birth")
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_HEALTHSPELLCHANGED,actor,100,0,100)),"初始化生命100"): return false
	await settle(0.4)
	if not await capture("monster-%d-birth" % shape): return false
	for facing in range(8):
		show_phase(item,"站立方向 %d / 7" % facing)
		if not hud.apply_game_message(monster_packet(Mir3HudView.SM_WALK,actor,cell,facing,shape)):
			result.stages.append("direction_%d_resource_unavailable" % facing)
		await settle(0.12)
	show_phase(item,"行走")
	var destination := cell+Vector2i(1,0)
	if hud.apply_game_message(monster_packet(Mir3HudView.SM_WALK,actor,destination,2,shape)):
		await settle(0.9)
		if not check(hud.hud_snapshot().scene_chicken_cells[actor]==destination,"移动到协议目标格"): return false
		cell=destination
		result.stages.append("walk")
	else: result.stages.append("walk_resource_unavailable")
	show_phase(item,"攻击")
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_HIT,actor,cell.x,cell.y,4)),"攻击包接收"): return false
	if not check(hud.hud_snapshot().scene_monster_attack_frames[actor]>=0,"攻击动作启动"): return false
	result.stages.append("attack")
	await settle(0.2)
	if not await capture("monster-%d-attack" % shape): return false
	await settle(1.6)
	show_phase(item,"被攻击：生命100 → 70，伤害30")
	var hit := monster_packet(Mir3HudView.SM_STRUCK,actor,Vector2i(70,100),30,shape)
	if hud.apply_game_message(hit):
		result.stages.append("struck")
		await settle(0.1)
		if not check(hud.hud_snapshot().scene_health_values[actor][0]==70,"受击更新当前生命70"): return false
		if not await capture("monster-%d-struck" % shape): return false
		await settle(0.65)
	else: result.stages.append("struck_resource_unavailable")
	show_phase(item,"死亡 → 尸体驻留")
	if hud.apply_game_message(monster_packet(Mir3HudView.SM_NOWDEATH,actor,cell,4,shape)):
		await settle(3.5)
		if not check(hud.hud_snapshot().scene_monster_death_frames[actor]>=0,"死亡状态保留"): return false
		result.stages.append("death")
	else: result.stages.append("death_resource_unavailable")
	if not await capture("monster-%d-lifecycle" % shape): return false
	hud.remove_scene_actor(actor)
	if not check(not hud.hud_snapshot().scene_monster_shapes.has(actor),"离场释放怪物对象"): return false
	result.stages.append("remove")
	result["status"]="observed"
	return true

func cut_at(pointer: Vector2) -> void:
	for pressed in [true,false]:
		var button := InputEventMouseButton.new()
		button.button_index=MOUSE_BUTTON_LEFT
		button.position=pointer
		button.pressed=pressed
		button.alt_pressed=true
		root.push_input(button,true)

func wait_for_monster_step(actor: int, destination: Vector2i) -> bool:
	var deadline := Time.get_ticks_msec()+2500
	while Time.get_ticks_msec()<deadline:
		var snapshot: Dictionary = hud.hud_snapshot()
		if snapshot.scene_chicken_cells.get(actor) == destination and snapshot.scene_chicken_steps.get(actor, -1) >= 0:
			return true
		await process_frame
	return false

func run_regression() -> void:
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
	# 动作回包不解除预测等待；使用真实的 +GOOD 确认再进行下一次操作。
	if not check(hud.apply_game_message("+GOOD/200".to_ascii_buffer()),"确认首次本地蹲姿"): return
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
	if not check(hud.apply_game_message("+GOOD/200".to_ascii_buffer()),"确认人物尸体割肉动作"): return
	hud.set_process(false)
	if not await capture("map-butch-crouch"): return
	hud.set_process(true)
	await settle(1.0)
	hud.remove_scene_actor(5)
	var feature := PackedByteArray()
	feature.resize(16)
	feature[0]=3
	feature[2]=20
	var stand := game_reply(Mir3HudView.SM_TURN,6,cell.x,cell.y,6)
	stand.append_array(encode_scene_bytes(feature))
	if not check(not hud.apply_game_message(stand.slice(0,27)),"拒绝正文截断的入场包"): return
	if not check(hud.apply_game_message(stand) and hud.hud_snapshot().scene_chicken_steps[6]==-1,
		"怪物20按原坐标入场并站立"): return
	await settle()
	var living: Rect2=hud.hud_snapshot().scene_chicken_bounds[6]
	if not check(living.has_area(),"活鸡在地图上绘制"): return
	if not await capture("map-butch-live-chicken"): return
	var call_deadline := Time.get_ticks_msec()+10000
	while hud.hud_snapshot().scene_actor_sounds.get(6,{}).get("sample")!=401 and Time.get_ticks_msec()<call_deadline:
		await process_frame
	if not check(hud.hud_snapshot().scene_actor_sounds.get(6,{}).get("sample")==401,
		"站立动作触发原版鸡叫声"): return
	if not await capture("map-chicken-idle-call"): return
	# 怪物的 SM_WALK 带新格坐标时触发行动1。重放往返两步，验证排队。
	var outward := game_reply(Mir3HudView.SM_BACKSTEP,6,origin.x,origin.y,4)
	outward.append_array(encode_scene_bytes(feature))
	var returning := game_reply(Mir3HudView.SM_WALK,6,cell.x,cell.y,4)
	returning.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(outward) and hud.apply_game_message(returning)
		and hud.hud_snapshot().scene_chicken_steps[6]==0
		and hud.hud_snapshot().scene_monster_walk_poses[6]==5
		and hud.hud_snapshot().scene_chicken_cells[6]==origin,"先面向原侧后退，随后移动留在队列"): return
	var step_deadline := Time.get_ticks_msec()+1000
	while Time.get_ticks_msec()<step_deadline:
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
	step_deadline=Time.get_ticks_msec()+2500
	while Time.get_ticks_msec()<step_deadline:
		await process_frame
		if hud.hud_snapshot().scene_chicken_cells[6]==cell: break
	if not check(hud.hud_snapshot().scene_chicken_steps[6]==0,"追帧动作仍从首帧开始"): return
	step_deadline=Time.get_ticks_msec()+1000
	while Time.get_ticks_msec()<step_deadline:
		await process_frame
		if hud.hud_snapshot().scene_chicken_steps[6]>0: break
	if not check(hud.hud_snapshot().scene_chicken_steps[6]==2,"积压时首个移动节拍前进两帧"): return
	if not await capture("map-chicken-queue"): return
	await settle(2.2)
	if not check(hud.hud_snapshot().scene_chicken_steps[6]==-1,"连续动作全部结束"): return
	# 行走期间的受击应先排队，后续移动须等受击动作结束。
	# 怪物31号更新生命并进入受击动作。
	var queued_hit := game_reply(Mir3HudView.SM_STRUCK,6,9,10,1)
	queued_hit.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(outward) and hud.apply_game_message(queued_hit)
		and hud.apply_game_message(returning) and hud.hud_snapshot().scene_chicken_struck[6]==-1,
		"受击不抢占正在播放的后退"): return
	step_deadline=Time.get_ticks_msec()+2500
	while Time.get_ticks_msec()<step_deadline:
		await process_frame
		if hud.hud_snapshot().scene_chicken_struck[6]>=0: break
	if not check(hud.hud_snapshot().scene_chicken_struck[6]>=0 and hud.hud_snapshot().scene_chicken_cells[6]==origin
		and hud.hud_snapshot().scene_health_values[6][0]==9,"后退完成才应用受击与生命更新"): return
	if not await capture("map-monster-queued-hit"): return
	await settle(1.6)
	if not check(hud.hud_snapshot().scene_chicken_cells[6]==cell and hud.hud_snapshot().scene_chicken_steps[6]==-1,
		"受击完成后才执行返回"): return
	if not await check_relay_queue(origin,cell): return
	var health := game_reply(Mir3HudView.SM_HEALTHSPELLCHANGED,6,10,3,10)
	if not check(hud.apply_game_message(health) and hud.hud_snapshot().scene_health_values[6][2]==10
		and hud.hud_snapshot().scene_chicken_steps[6]==-1,"怪物53即时更新血量"): return
	var register_bar := game_reply(Mir3HudView.SM_HEALTH_PERCENT,6,0,0,0)
	if not check(hud.apply_game_message(register_bar),"用原包登记怪物血条"): return
	var health_delta := game_reply(Mir3HudView.SM_HEALTHSPELLCHANGED,6,5,2,10)
	if not check(hud.apply_game_message(health_delta) and hud.hud_snapshot().scene_health_values[6]==PackedInt32Array([5,2,10]),
		"生命和第二数值字段随回包更新"): return
	# 伤害字按绘制次数老化；首个完成绘制帧验收，避免高FPS下80ms已经超过40帧。
	await RenderingServer.frame_post_draw
	var health_snapshot: Dictionary=hud.hud_snapshot()
	if not check(health_snapshot.scene_health_bounds.get(6,Rect2()).size.x==17,
		"注册血条显示半血：%s" % [health_snapshot.scene_health_bounds]): return
	if not check(health_snapshot.scene_damage_frames.has(6),
		"绘制生命差值 enabled=%s frames=%s" % [health_snapshot.damage_numbers,health_snapshot.scene_damage_frames]): return
	if not await capture("map-chicken-health"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CHARSTATUSCHANGE,6,2,0,0)),"怪物状态设置血条校验值"): return
	await settle()
	if not check(hud.hud_snapshot().scene_health_bounds.has(6),"匹配时仍显示血条"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CHARSTATUSCHANGE,6,0,0,0)),"清除怪物状态位"): return
	await settle()
	if not check(not hud.hud_snapshot().scene_health_bounds.has(6),"校验值不匹配时隐藏血条"): return
	if not await capture("map-chicken-health-hidden"): return
	# 外观包中的状态位也经过血条校验，并随动作队列应用。
	var marked := feature.duplicate()
	marked[4]=2
	var marked_turn := game_reply(Mir3HudView.SM_WALK,6,origin.x,origin.y,0)
	marked_turn.append_array(encode_scene_bytes(marked))
	var queued_status := game_reply(Mir3HudView.SM_CHARSTATUSCHANGE,6,0,0,0)
	var queued_health := game_reply(Mir3HudView.SM_HEALTHSPELLCHANGED,6,4,1,10)
	if not check(hud.apply_game_message(marked_turn) and hud.apply_game_message(queued_status)
		and hud.apply_game_message(queued_health) and hud.apply_game_message(returning),"发送移动、清位、血量与回程"): return
	await settle(0.2)
	if not check(not hud.hud_snapshot().scene_health_bounds.has(6)
		and hud.hud_snapshot().scene_health_values[6][0]==4,"行走中657与53即时清位和更新生命"): return
	if not await capture("map-chicken-health-moving"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_FEATURECHANGED,6,3,21,0)),
		"怪物41用头部外观DWORD立即更换为鹿"): return
	await settle()
	if not check(not hud.hud_snapshot().scene_health_bounds.has(6)
		and hud.hud_snapshot().scene_monster_shapes[6]==21
		and hud.hud_snapshot().scene_chicken_steps[6]>=0,"41在当前动作结束前更换外观"): return
	await settle(1.5)
	if not check(not hud.hud_snapshot().scene_health_bounds.has(6)
		and hud.hud_snapshot().scene_health_values[6]==PackedInt32Array([4,1,10]),"回程前应用清位与新生命"): return
	await settle(1.0)
	var miss := game_reply(Mir3HudView.SM_STRUCK,6,10,10,0)
	miss.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(miss) and hud.hud_snapshot().scene_chicken_struck[6]==-1,
		"零伤害不触发受击帧"): return
	if not check(hud.hud_snapshot().map_event_count==0,"零伤害不创建命中特效"): return
	await settle(1.0)
	var strike_feature := feature.duplicate()
	strike_feature[14]=1
	var strike := game_reply(Mir3HudView.SM_STRUCK,6,6,10,4)
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
	var death := game_reply(Mir3HudView.SM_STRUCK,6,0,10,6)
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
	if not check(hud.apply_game_message("+GOOD/200".to_ascii_buffer()),"确认鸡尸体割肉动作"): return
	hud.set_process(false)
	if not await capture("map-butch-chicken"): return
	hud.set_process(true)
	await settle(1.0)
	# 33号创建骨架尸体，动作10，不是站立或钻地动作8。
	var skeleton := monster_packet(Mir3HudView.SM_SKELETON,60,cell,6,20)
	if not check(hud.apply_game_message(skeleton)
		and hud.hud_snapshot().scene_monster_death_frames[60]>=0
		and hud.hud_snapshot().scene_monster_burrow_frames[60]==-1,"33号建立骨架尸体"): return
	if not check(hud.remove_scene_actor(60),"清理首次入场的尸体"): return
	# 原尸体仍可用于独立验证超距割肉。
	var before_deer := packets.size()
	# 形状21与鸡共用动作规则，在 Mon-3 取下一组图像。
	hud.remove_scene_actor(6)
	await settle(0.9)
	feature[2]=21
	var deer := game_reply(Mir3HudView.SM_TURN,7,cell.x,cell.y,6)
	deer.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(deer),"鹿按原版外观入场"): return
	await settle()
	if not check(hud.hud_snapshot().scene_chicken_bounds[7].size.y>living.size.y,"鹿使用自己的站立图"): return
	if not await capture("map-butch-deer-live"): return
	var deer_death := game_reply(Mir3HudView.SM_NOWDEATH,7,cell.x,cell.y,6)
	deer_death.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(deer_death),"鹿接收34号死亡动作"): return
	await settle(1.65)
	cut_at(hud.hud_snapshot().scene_chicken_bounds[7].get_center())
	if not check(packets.size()==before_deer+2 and decode_request(packets.back()).decode_u32(0)==7,
		"蹲下后发送以鹿尸体为目标的割肉请求"): return
	if not check(hud.apply_game_message("+GOOD/200".to_ascii_buffer()),"确认鹿尸体割肉动作"): return
	if not await capture("map-butch-deer"): return
	# 夜间对比同格、同朝向的活鹿，并验证离场清除光源。
	if not check(hud.change_map("0","比奇省",origin,1),"进入夜间比奇"): return
	await settle()
	var night: PackedColorArray=hud.hud_snapshot().scene_light_samples
	var lit_deer := game_reply(Mir3HudView.SM_TURN,7,cell.x,cell.y,6+8*256)
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
	var oma := game_reply(Mir3HudView.SM_801,8,cell.x,cell.y,6)
	oma.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(oma),"原始入场包创建半兽人"): return
	if not check(hud.hud_snapshot().scene_actor_sounds.get(8,{}).get("sample")==109,"801 触发现身音效"): return
	await settle()
	var oma_live: Rect2=hud.hud_snapshot().scene_chicken_bounds[8]
	if not check(oma_live.has_area() and oma_live.size.y>living.size.y,"使用半兽人的原版站立资源"): return
	if not await capture("map-oma-entry"): return
	var oma_death := game_reply(Mir3HudView.SM_NOWDEATH,8,cell.x,cell.y,6)
	oma_death.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(oma_death),"34号启动半兽人死亡"): return
	await settle(1.5)
	var oma_body: Rect2=hud.hud_snapshot().scene_chicken_bounds[8]
	var before_oma := packets.size()
	cut_at(oma_body.get_center())
	if not check(oma_body.has_area() and oma_body!=oma_live and packets.size()==before_oma+1
		and decode_request(packets.back()).decode_u16(4)==Mir3HudView.CM_SITDOWN,"半兽人尸体不发送割肉请求"): return
	if not check(hud.apply_game_message("+GOOD/200".to_ascii_buffer()),"确认非割肉尸体蹲姿"): return
	hud.remove_scene_actor(8)
	await settle(0.9)
	# 猪在 Mon-12 的第十组，验证另一资源库及割肉请求。
	feature[2]=119
	var pig := game_reply(Mir3HudView.SM_TURN,9,cell.x,cell.y,6)
	pig.append_array(encode_scene_bytes(feature))
	# GBK「猪」，斜杠后的十进制文本经atoi转为名称颜色索引。
	pig.append_array(encode_scene_bytes(PackedByteArray([0xd6,0xed,47,50,52,57])))
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
	# 656 更新名字颜色，42 同时更新名字正文和颜色。
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CHANGENAMECOLOR,9,255,0,0)),
		"656更新怪物名色"): return
	await settle()
	if not check(hud.hud_snapshot().displayed_name_actor==9
		and hud.hud_snapshot().displayed_name_color==0xffffff,"656保留名称并切换为白色"): return
	var pig_name := game_reply(Mir3HudView.SM_USERNAME,9,249,0,0)
	pig_name.append_array(encode_scene_bytes(PackedByteArray([0xd6,0xed])))
	if not check(hud.apply_game_message(pig_name),"42更新怪物名称正文"): return
	await settle()
	if not check(hud.hud_snapshot().displayed_name_color==255,"42应用名字颜色"): return
	# 怪物41号包刷新原对象外观，先跨库切到半兽人再切回猪。
	var transform_feature := feature.duplicate()
	transform_feature[2]=23
	var transform := game_reply(Mir3HudView.SM_FEATURECHANGED,9,3,23,0)
	if not check(hud.apply_game_message(transform),"用怪物41号包跨库更换外观"): return
	await settle()
	if not check(hud.hud_snapshot().scene_chicken_cells[9]==cell
		and hud.hud_snapshot().scene_chicken_bounds[9]!=pig_live,"同一对象在原格更换图像"): return
	if not await capture("map-monster-feature-change"): return
	transform=game_reply(Mir3HudView.SM_STRUCK,9,8,10,2)
	transform.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(transform) and hud.hud_snapshot().scene_chicken_struck[9]==0
		and hud.hud_snapshot().scene_health_values[9][0]==8,"受击包恢复猪外观、更新生命并进入受击动作"): return
	if not await capture("map-pig-transformed-hit"): return
	await settle(0.8)
	var pig_walk := game_reply(Mir3HudView.SM_WALK,9,origin.x,origin.y,0)
	pig_walk.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(pig_walk) and hud.hud_snapshot().scene_chicken_steps[9]==0,"猪进入行走动作"): return
	var oma_back := game_reply(Mir3HudView.SM_BACKSTEP,9,cell.x,cell.y,0)
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
	var oma_appearance := game_reply(Mir3HudView.SM_FEATURECHANGED,9,3,23,0)
	if not check(hud.apply_game_message(oma_appearance) and hud.hud_snapshot().scene_monster_shapes[9]==23,
		"死亡前对象显示半兽人"): return
	var pig_death := game_reply(Mir3HudView.SM_NOWDEATH,9,cell.x,cell.y,6)
	pig_death.append_array(encode_scene_bytes(feature))
	var final_walk := game_reply(Mir3HudView.SM_WALK,9,origin.x,origin.y,0)
	final_walk.append_array(encode_scene_bytes(transform_feature))
	if not check(hud.apply_game_message(final_walk) and hud.apply_game_message(pig_death)
		and hud.hud_snapshot().scene_monster_shapes[9]==23,"死亡回包等待当前行动播放"): return
	await settle(2.2)
	if not check(hud.hud_snapshot().scene_monster_shapes[9]==119 and hud.hud_snapshot().scene_chicken_cells[9]==cell
		and hud.hud_snapshot().scene_chicken_steps[9]==-1,"当前行动结束后应用猪尸体"): return
	cut_at(hud.hud_snapshot().scene_chicken_bounds[9].get_center())
	if not check(packets.size()==before_oma+3 and decode_request(packets.back()).decode_u32(0)==9,
		"猪尸体发送对应编号的割肉请求"): return
	if not check(hud.apply_game_message("+GOOD/200".to_ascii_buffer()),"确认猪尸体割肉动作"): return
	if not await capture("map-pig-death-transform"): return
	hud.remove_scene_actor(9)
	feature[2]=31
	# 形状31按原版固定读取方向0资源，即使入场包指定方向6。
	var emergence := game_reply(Mir3HudView.SM_TURN,10,cell.x,cell.y,6)
	emergence.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(emergence),"形状31从 Mon-4 入场"): return
	await settle()
	if not check(hud.hud_snapshot().scene_chicken_bounds[10].has_area(),"钻地前可见"): return
	# 先以猪的资源播放移动，到达后通过受击包换成具备钻地图的形状31。
	var moving_pig := game_reply(Mir3HudView.SM_WALK,10,origin.x,origin.y,0)
	var pig_feature := feature.duplicate()
	pig_feature[2]=119
	moving_pig.append_array(encode_scene_bytes(pig_feature))
	var burrow_shape := game_reply(Mir3HudView.SM_STRUCK,10,10,10,0)
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
	if not check(not hud.hud_snapshot().scene_monster_shapes.has(10),"钻地末帧结束后删除对象"): return
	if not check(hud.apply_game_message(emergence),"同一对象重新出土"): return
	await settle()
	if not check(hud.hud_snapshot().scene_chicken_bounds[10].has_area(),"出土恢复可见"): return
	# 钻回期间排队的消息随旧对象删除，不得凭旧队列复活对象。
	var reentry := monster_packet(Mir3HudView.SM_ACTOR_STATUS,10,origin,0,31)
	if not check(hud.apply_game_message(burrow) and hud.apply_game_message(reentry),"钻回期间收到入场消息"): return
	await settle(1.5)
	if not check(not hud.hud_snapshot().scene_monster_shapes.has(10),"钻回结束释放旧对象及其队列"): return
	if not check(hud.apply_game_message(reentry),"旧对象释放后新入场"): return
	await settle()
	if not check(hud.hud_snapshot().scene_chicken_cells[10]==origin,"新入场使用新的位置"): return
	hud.remove_scene_actor(10)
	# 骷髅按通用索引读取 Mon-4 第三组，覆盖入场、移动及死亡。
	feature[2]=32
	var skeleton_entry := game_reply(Mir3HudView.SM_TURN,11,cell.x,cell.y,6)
	skeleton_entry.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(skeleton_entry),"形状32按通用索引入场"): return
	await settle()
	if not check(hud.hud_snapshot().scene_chicken_bounds[11].has_area(),"骷髅站立帧可见"): return
	var skeleton_walk := game_reply(Mir3HudView.SM_WALK,11,origin.x,origin.y,0)
	skeleton_walk.append_array(encode_scene_bytes(feature))
	var skeleton_burrow := monster_packet(Mir3HudView.SM_DEATH,11,cell,6,32)
	if not check(hud.apply_game_message(skeleton_walk) and hud.apply_game_message(skeleton_burrow)
		and hud.hud_snapshot().scene_chicken_steps[11]==0,"骷髅行走时死亡包进入队列"): return
	if not await capture("map-skeleton-walk"): return
	step_deadline=Time.get_ticks_msec()+2500
	while Time.get_ticks_msec()<step_deadline:
		if hud.hud_snapshot().scene_monster_death_frames[11]>=0: break
		await process_frame
	if not check(hud.hud_snapshot().scene_chicken_steps[11]==-1
		and hud.hud_snapshot().scene_chicken_cells[11]==cell
		and hud.hud_snapshot().scene_monster_burrow_frames[11]==-1
		and hud.hud_snapshot().scene_monster_death_frames[11]>=0,"32号在移动结束后直接进入尸体末帧"): return
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
	var approach := game_reply(Mir3HudView.SM_WALK,14,origin.x,origin.y,0)
	approach.append_array(encode_scene_bytes(feature))
	var attack := game_reply(Mir3HudView.SM_HIT,14,origin.x,origin.y,0)
	var attack_retreat := game_reply(Mir3HudView.SM_WALK,14,cell.x,cell.y,4)
	attack_retreat.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(fighter) and hud.apply_game_message(approach)
		and hud.apply_game_message(attack) and hud.apply_game_message(attack_retreat)
		and hud.hud_snapshot().scene_monster_attack_frames[14]==-1,"攻击等待移动完成"): return
	step_deadline=Time.get_ticks_msec()+2500
	while Time.get_ticks_msec()<step_deadline:
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
		var corpse_packet := game_reply(Mir3HudView.SM_NOWDEATH,actor,position.x,position.y,0)
		corpse_packet.append_array(encode_scene_bytes(feature))
		if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,position,0,fixture[1])) and hud.apply_game_message(corpse_packet)
			and hud.hud_snapshot().scene_monster_death_frames[actor]==0,"34号在已存在对象上启动死亡动作"): return
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
		and hud.apply_game_message(game_reply(Mir3HudView.SM_HIT,17,cell.x,cell.y,0))
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
	var ranged := game_reply(Mir3HudView.SM_FLYAXE,18,cell.x,cell.y,0)
	ranged.append_array(encode_scene_bytes(target))
	if not check(hud.apply_game_message(ranged_actor) and hud.apply_game_message(ranged)
		and hud.hud_snapshot().scene_monster_attack_frames[18]==0,"远程攻击解析目标并播放攻击动作"): return
	await settle(0.4)
	if not await capture("map-monster-ranged-attack"): return
	# 9号消息让怪物播放起身动作，并清理死亡状态。
	var rise := game_reply(Mir3HudView.SM_27,15,cell.x,cell.y,0)
	feature[2]=83
	rise.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(rise)
		and hud.hud_snapshot().scene_monster_death_frames[15]==-1
		and hud.hud_snapshot().scene_monster_burrow_frames[15]==0,"起身后恢复活体状态并开始动作8"): return
	if not await capture("map-monster-rise"): return
	# 1316清除死亡状态并恢复站立。
	var forced_step := game_reply(Mir3HudView.SM_REVIVE,14,cell.x,cell.y,0)
	feature[2]=23
	forced_step.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(forced_step)
		and hud.hud_snapshot().scene_chicken_steps[14]==-1,"1316恢复站立而不强制行走"): return
	# 形状83携带状态位1入场时固定显示动作8首帧。
	feature[2]=83
	feature[4]=1
	var hidden_entry := game_reply(Mir3HudView.SM_TURN,19,cell.x,cell.y,6)
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
		and hud.apply_game_message(game_reply(Mir3HudView.SM_HIT,20,origin.x,origin.y,0))
		and hud.apply_game_message(game_reply(Mir3HudView.SM_1317,20,cell.x,cell.y,6)),
		"普通攻击和1317依次处理"): return
	if not check(hud.hud_snapshot().scene_monster_attack_actions[20]==6
		and hud.hud_snapshot().map_event_frames.values().has(1390),"形状141的动作6附加565号特效"): return
	if not await capture("map-monster-action-six"): return
	step_deadline=Time.get_ticks_msec()+2500
	while Time.get_ticks_msec()<step_deadline:
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
		[25, Mir3HudView.SM_ACTOR_STATUS], [26, Mir3HudView.SM_808]]:
		var actor: int = fixture[0]
		var opcode: int = fixture[1]
		var entry := game_reply(opcode,actor,cell.x,cell.y,0)
		entry.append_array(encode_scene_bytes(feature))
		if not check(hud.apply_game_message(entry)
			and hud.hud_snapshot().scene_monster_shapes.get(actor)==23,
			"首次收到 %d 时按外观创建怪物" % opcode): return
	await settle()
	var initial_actors: Dictionary = hud.hud_snapshot()
	if not check(initial_actors.scene_chicken_steps[21]>=0
		and initial_actors.scene_chicken_steps[22]>=0
		and initial_actors.scene_chicken_steps[23]==-1,"首次消息分别应用后退、行走和站立"): return
	var status_update := game_reply(Mir3HudView.SM_CHARSTATUSCHANGE,24,0,0,0)
	feature[2]=21
	status_update.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(status_update)
		and hud.hud_snapshot().scene_monster_shapes[24]==23,"已有对象的状态包忽略外观正文"): return
	if not await check_rise_health(cell,feature): return
	if not await check_special_effect(origin,cell,feature): return
	for actor in [21,22,24,25,26,27,28]:
		hud.remove_scene_actor(actor)
	if not await check_status_blend(): return
	if not await check_zuma_lifecycle(cell): return
	if not await check_zuma_ai_scene(): return
	if not await check_monster_pursuit(): return
	if not await check_revive_transition(): return
	await finish()

func finish() -> void:
	hud.queue_free()
	# 停止播放由音频线程异步回收，退出前留出混音线程处理释放请求的时间。
	await settle(0.35)
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
		"pursuit":
			if not await check_monster_pursuit(): return
		"zuma-ai":
			if not await check_zuma_ai_scene(): return
		"revive-transition":
			if not await check_revive_transition(): return
		"empty-actions":
			if not await check_empty_actions(): return
		"reentry":
			if not await check_monster_reentry(): return
		"identity":
			if not await check_monster_identity(): return
		"relay":
			if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,6,cell,0,20)),"建立队列专项怪物"): return
			if not await check_relay_queue(origin,cell): return
		"special-follow":
			if not await check_special_follow(): return
		"special-origin":
			if not await check_special_origin(): return
		"special-branches":
			if not await check_special_branches(): return
		"burrow-removal":
			if not await check_burrow_removal(cell): return
		"burrow":
			if not await check_burrow_lifecycle(cell): return
		"wire":
			if not await check_corrected_wire(cell): return
		"zuma":
			if not await check_zuma_lifecycle(cell): return
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

func check_monster_pursuit() -> bool:
	# 夹具只提供目标观察与推进时钟，移动选择和攻击冷却全部在C++。
	if not check(hud.change_map("0","比奇省",SHOWCASE_ORIGIN,1),"追踪AI清场"): return false
	var cell := SHOWCASE_ORIGIN+Vector2i(0,2)
	var actor := 9981
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,cell,0,83)),"建立已苏醒卫士"): return false
	var target := {"id":500,"cell":cell+Vector2i(3,0)}
	var result: Dictionary=hud.advance_monster_ai(1001,[target])
	if not check(result.moved==[actor] and hud.hud_snapshot().scene_chicken_cells[actor]==cell+Vector2i(1,0),"C++向目标方向迈第一步"): return false
	result=hud.advance_monster_ai(601,[target])
	if not check(result.moved.is_empty() and result.attacked.is_empty(),"画面动作未完成时离线快进不堆积动作"): return false
	await settle(0.8)
	result=hud.advance_monster_ai(601,[target])
	if not check(result.moved==[actor] and hud.hud_snapshot().scene_chicken_cells[actor]==cell+Vector2i(2,0),"C++追踪第二步"): return false
	await settle(0.8)
	result=hud.advance_monster_ai(601,[target])
	if not check(result.attacked==[actor] and result.moved.is_empty() and hud.hud_snapshot().scene_monster_attack_frames[actor]>=0 and hud.hud_snapshot().scene_monster_facings[actor]==2,"邻格转朝目标并进入攻击动画"): return false
	if not await capture("monster-ai-pursuit-attack"): return false
	await settle(0.9)
	result=hud.advance_monster_ai(601,[target])
	if not check(result.attacked.is_empty() and result.moved.is_empty(),"攻击间隔内原地等待，不进入目标格"): return false
	result=hud.advance_monster_ai(601,[target])
	if not check(result.attacked==[actor],"攻击间隔结束重新攻击"): return false
	await settle(0.9)
	target.alive=false
	result=hud.advance_monster_ai(601,[target])
	if not check(result.attacked.is_empty() and result.moved.is_empty(),"无效目标停止追踪和攻击"): return false
	# 教主外观苏醒强制朝北，但服务端逻辑朝西仍用于前方召唤。
	if not check(hud.change_map("0","比奇省",SHOWCASE_ORIGIN,1),"方向分离清场"): return false
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,9982,cell,6,85,0,1)),"朝西教主入场"): return false
	result=hud.advance_monster_ai(601,[{"id":501,"cell":cell}])
	if not check(result.woke==[9982],"朝西教主苏醒"): return false
	await settle(2.6)
	if not check(hud.hud_snapshot().scene_monster_facings[9982]==0,"原版苏醒显示方向0"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_HEALTHSPELLCHANGED,9982,99,0,100)),"教主触发召唤"): return false
	result=hud.advance_monster_ai(2601,[])
	if not check(not result.spawned.is_empty() and result.spawned.all(func(slave): return slave.cell==cell+Vector2i(-1,0)),"召唤使用独立逻辑方向，不被客户端朝北姿势覆盖"): return false
	print("MONSTER_PURSUIT_OK logic_facing_preserved=true")
	return true

func check_zuma_ai_scene() -> bool:
	# GD只提供场景和观察对象，距离、联动、暂停、召唤均由C++计算。
	if not check(hud.change_map("0","比奇省",SHOWCASE_ORIGIN,1),"祖玛AI场景清场"): return false
	var cell := SHOWCASE_ORIGIN+Vector2i(0,2)
	var actors := [{"id":9971,"shape":83,"cell":cell},{"id":9972,"shape":84,"cell":cell+Vector2i(5,0)},{"id":9973,"shape":85,"cell":cell+Vector2i(0,3)}]
	for actor in actors:
		if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor.id,actor.cell,0,actor.shape,0,1)),"AI石像入场"): return false
	var target := {"cell":cell+Vector2i(-2,-2)}
	var result: Dictionary=hud.advance_monster_ai(600,[target])
	if not check(result.woke.is_empty(),"出生正常步速门限采用严格大于"): return false
	result=hud.advance_monster_ai(1,[target])
	if not check(result.woke.has(9971) and result.woke.has(9972) and not result.woke.has(9973) and result.rejected.is_empty(),"C++邻近唤醒、范围内卫士联动、教主不联动"): return false
	await settle(0.25)
	if not check(hud.hud_snapshot().scene_monster_burrow_frames[9971]>=0 and hud.hud_snapshot().scene_monster_burrow_frames[9972]>=0 and hud.hud_snapshot().scene_monster_dormant[9973],"AI输出进入原版动画状态机"): return false
	if not await capture("monster-zuma-ai-group-wake"): return false
	await settle(0.8)
	result=hud.advance_monster_ai(2600,[])
	if not check(not result.ready.has(9971),"触发卫士暂停边界"): return false
	result=hud.advance_monster_ai(1,[])
	if not check(result.ready.has(9971),"触发卫士结束暂停允许正常行动"): return false
	result=hud.advance_monster_ai(1,[{"cell":actors[2].cell}])
	if not check(result.woke==[9973],"教主独立靠近才唤醒"): return false
	await settle(0.3)
	if not check(hud.hud_snapshot().scene_monster_burrow_frames[9973]>=0 and hud.hud_snapshot().scene_monster_facings[9973]==0,"教主唤醒走C++固定方向长动画"): return false
	if not await capture("monster-zuma-ai-king-wake"): return false
	await settle(2.3)
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_HEALTHSPELLCHANGED,9973,99,0,100)),"教主生命输入"): return false
	result=hud.advance_monster_ai(2601,[])
	if not check(result.spawned.size()>=6 and result.spawned.size()<=11 and result.spawn_failures.is_empty(),"C++教主实际生成一波6–11只随从"): return false
	var summoned: Array=result.spawned.duplicate(true)
	for wave in range(4):
		result=hud.advance_monster_ai(601,[])
		if not check(result.spawn_failures.is_empty(),"有效前方格召唤成功"): return false
		summoned.append_array(result.spawned)
	if not check(summoned.size()==30 and hud.hud_snapshot().scene_monster_shapes.size()==33,"五波到达30存活随从上限"): return false
	var prototype_shapes := {143:82,145:83,148:84,126:75}
	var identities: Array=[]
	for request in summoned:
		if not check(request.owner==9973 and prototype_shapes.get(request.prototype)==request.shape and request.actor not in identities,"每只随从使用独立对象ID及明确原型映射"): return false
		identities.append(request.actor)
		if not check(hud.hud_snapshot().scene_monster_shapes.get(request.actor)==request.shape and request.cell==actors[2].cell+Vector2i(0,-1),"实际实体按教主前方格入场"): return false
		if not check(hud.hud_snapshot().scene_monster_dormant[request.actor]==(request.shape in [83,84]),"召唤石像保留特殊出生状态"): return false
	if not await capture("monster-zuma-ai-summons"): return false
	# 离场及死亡都释放存活配额；尸体可以留在场景，不应被计作活随从。
	for index in range(10): hud.remove_scene_actor(summoned[index].actor)
	for index in range(10,13):
		var slave: Dictionary=summoned[index]
		if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_DEATH,slave.actor,slave.cell,0,slave.shape)),"随从成为尸体"): return false
	result=hud.advance_monster_ai(601,[])
	if not check(result.spawned.is_empty(),"危险等级耗尽不自动继续召唤"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_HEALTHSPELLCHANGED,9973,100,0,100)),"教主满血重置输入"): return false
	hud.advance_monster_ai(601,[])
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_HEALTHSPELLCHANGED,9973,99,0,100)),"教主再次受击"): return false
	result=hud.advance_monster_ai(601,[])
	if not check(result.spawned.size()>=6 and result.spawned.size()<=11 and result.spawn_failures.is_empty(),"死去和离场随从释放名额后可再次生成"): return false
	hud.remove_scene_actor(9971)
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,9971,cell,0,83,0,1)),"同编号石像重建"): return false
	result=hud.advance_monster_ai(601,[target])
	if not check(result.woke.has(9971),"重建对象重置AI，不继承上一个对象已唤醒状态"): return false
	print("MONSTER_ZUMA_AI_SCENE_OK resolved_prototypes=4 live_limit=30")
	return true

func check_revive_transition() -> bool:
	# 1316应等待当前死亡动画，然后从出队时开始600ms显现。
	if not check(hud.change_map("0","比奇省",SHOWCASE_ORIGIN,1),"复活过渡清场"): return false
	var cell := SHOWCASE_ORIGIN+Vector2i(0,2)
	var actor := 9960
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,cell,0,23)),"建立复活对象"): return false
	# 42E9A0首次建怪开始300ms显现，后续普通站立不能重置。
	if not check(hud.hud_snapshot().scene_monster_appearance_elapsed_ms[actor]==0,"首次入场立即开始显现"): return false
	if not check(hud.hud_snapshot().scene_monster_appearance_weight[actor]==0,"未悬停时入场权重从0开始"): return false
	hud.set_hovered_target(actor,3,23,false,0,0,0,0,cell)
	if not check(hud.hud_snapshot().scene_monster_appearance_weight[actor]==5,"悬停将入场权重提高5"): return false
	hud.set_hovered_target(0,0,0,false,0,0,0,0,Vector2i.ZERO)
	await settle(0.12)
	var arrival: float=hud.hud_snapshot().scene_monster_appearance_elapsed_ms[actor]
	if not check(arrival>0 and arrival<300,"首次入场显现正在推进"): return false
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,cell,0,23)),"显现期间普通站立"): return false
	if not check(hud.hud_snapshot().scene_monster_appearance_elapsed_ms[actor]==arrival,"普通站立不重置显现"): return false
	await settle(0.21)
	if not check(hud.hud_snapshot().scene_monster_appearance_elapsed_ms[actor]==-1,"首次显现在300ms结束"): return false
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_NOWDEATH,actor,cell,0,23)),"播放死亡"): return false
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_REVIVE,actor,cell,0,23)),"复活排队"): return false
	await settle(0.2)
	if not check(hud.hud_snapshot().scene_monster_revive_elapsed_ms[actor]==-1 and hud.hud_snapshot().scene_monster_death_frames[actor]>=0,"队列中复活不能提前启动过渡"): return false
	var deadline := Time.get_ticks_msec()+2500
	while hud.hud_snapshot().scene_monster_revive_elapsed_ms[actor]<0 and Time.get_ticks_msec()<deadline:
		await process_frame
	if not check(hud.hud_snapshot().scene_monster_revive_elapsed_ms[actor]>=0 and hud.hud_snapshot().scene_monster_death_frames[actor]==-1,"出队恢复站立并开始过渡"): return false
	await settle(0.18)
	hud.set_process(false)
	var elapsed: float=hud.hud_snapshot().scene_monster_revive_elapsed_ms[actor]
	if not check(elapsed>=150 and elapsed<600,"复活中段保留计时"): return false
	if not await capture("monster-revive-transition-mid"): return false
	hud.set_process(true)
	await settle(0.65)
	if not check(hud.hud_snapshot().scene_monster_revive_elapsed_ms[actor]==-1,"600ms结束恢复普通绘制"): return false
	if not await capture("monster-revive-transition-end"): return false
	# 暂停动作时钟，用同一帧的怪物像素核对常态悬停亮化。
	hud.set_process(false)
	await process_frame
	await RenderingServer.frame_post_draw
	var normal := root.get_texture().get_image()
	var bounds: Rect2 = hud.hud_snapshot().scene_chicken_bounds[actor]
	hud.set_hovered_target(actor,3,23,false,0,0,0,0,cell)
	await process_frame
	await RenderingServer.frame_post_draw
	var highlighted := root.get_texture().get_image()
	var brighter := 0
	for y in range(maxi(0,int(bounds.position.y)),mini(530,int(bounds.end.y))):
		for x in range(maxi(0,int(bounds.position.x)),mini(800,int(bounds.end.x))):
			var original := normal.get_pixel(x,y)
			var bright := highlighted.get_pixel(x,y)
			if bright.r>original.r or bright.g>original.g or bright.b>original.b: brighter+=1
	if not check(brighter>0,"普通悬停绘制提高怪物区域像素亮度"): return false
	if not await capture("monster-hover-highlight"): return false
	hud.set_hovered_target(0,0,0,false,0,0,0,0,Vector2i.ZERO)
	for status in [0x0400,0x0800,0x0002]:
		if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CHARSTATUSCHANGE,actor,0,status,0)),"设置待比较的怪物状态色"): return false
		await process_frame
		await RenderingServer.frame_post_draw
		normal=root.get_texture().get_image()
		hud.set_hovered_target(actor,3,23,false,0,0,0,0,cell)
		await process_frame
		await RenderingServer.frame_post_draw
		highlighted=root.get_texture().get_image()
		brighter=0
		for y in range(maxi(0,int(bounds.position.y)),mini(530,int(bounds.end.y))):
			for x in range(maxi(0,int(bounds.position.x)),mini(800,int(bounds.end.x))):
				var original := normal.get_pixel(x,y)
				var bright := highlighted.get_pixel(x,y)
				if bright.r>original.r or bright.g>original.g or bright.b>original.b: brighter+=1
		if not check((brighter==0) if status==0x0002 else (brighter>0),"灰阶与状态色悬停亮化，半透明保留原权重"): return false
		hud.set_hovered_target(0,0,0,false,0,0,0,0,Vector2i.ZERO)
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CHARSTATUSCHANGE,actor,0,0,0)),"清除状态色"): return false
	hud.set_process(true)
	# 立即重复复活重新计时，普通站立包不清除正在进行的过渡。
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_REVIVE,actor,cell,0,23)),"重复复活"): return false
	if not check(hud.hud_snapshot().scene_monster_revive_elapsed_ms[actor]==0,"重复复活重置计时"): return false
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,cell,0,23)),"过渡中站立"): return false
	if not check(hud.hud_snapshot().scene_monster_revive_elapsed_ms[actor]==0,"站立不取消过渡"): return false
	# 40ECB0 将超过31的悬停权重重置为0。
	var hover_deadline := Time.get_ticks_msec()+1500
	while hud.hud_snapshot().scene_monster_revive_elapsed_ms[actor]>=0 and hud.hud_snapshot().scene_monster_revive_elapsed_ms[actor]<525 and Time.get_ticks_msec()<hover_deadline:
		await process_frame
	var before_hover: int=hud.hud_snapshot().scene_monster_appearance_weight[actor]
	if not check(before_hover>=27 and before_hover<=31,"复活后段未悬停权重达到越界前范围"): return false
	hud.set_hovered_target(actor,3,23,false,0,0,0,0,cell)
	if not check(hud.hud_snapshot().scene_monster_appearance_weight[actor]==0,"悬停权重越界归零"): return false
	hud.set_hovered_target(0,0,0,false,0,0,0,0,Vector2i.ZERO)
	await settle(0.12)
	print("MONSTER_REVIVE_TRANSITION_OK duration_ms=600")
	return true

func check_empty_actions() -> bool:
	if not check(hud.change_map("0","比奇省",SHOWCASE_ORIGIN,1),"空帧专项清场"): return false
	var cell := SHOWCASE_ORIGIN+Vector2i(0,2)
	for shape in [5,186]:
		var profile: Dictionary=hud.monster_behavior_profile(shape)
		if not check(profile.visible_walk_directions.size()==(0 if shape==5 else 8),"C++按资源提供可见行走方向，不将缺帧当作固定AI"): return false
		var actor: int=9950+shape
		if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,cell,0,shape)),"建立空帧资源对象"): return false
		if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_STRUCK,actor,Vector2i(70,100),30,shape)),"空帧不拒绝受击及生命更新 shape=%d" % shape): return false
		if not check(hud.hud_snapshot().scene_health_values[actor][0]==70,"受击生命更新"): return false
		await settle(0.5)
		if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_NOWDEATH,actor,cell,4,shape)),"死亡消息不因空帧拒绝"): return false
		await settle(1.7)
		if not check(hud.hud_snapshot().scene_monster_death_frames[actor]==9,"保留原版10帧死亡，不缩短到9帧"): return false
		if not check(not hud.hud_snapshot().scene_chicken_bounds[actor].has_area(),"空尸体帧不伪造可见图像"): return false
		hud.remove_scene_actor(actor)
	# 602和610都在800毫秒后解除绘制延迟。
	for fixture in [[184,602],[186,610]]:
		var shape: int=fixture[0]
		var skill: int=fixture[1]
		var frame_offset := 40 if skill==602 else 0
		var actor := 9950+shape
		if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,cell,0,shape))
			and hud.apply_game_message(monster_packet(Mir3HudView.SM_NOWDEATH,actor,cell,4,shape)),
			"建立死亡特效测试对象"): return false
		if not check(hud.hud_snapshot().scene_effects.any(func(effect):
			return (effect.skill==skill and effect.delay==800 and not effect.sprite_visible
				and effect.frame==effect.first+frame_offset)),
			"死亡特效在800毫秒等待开始时隐藏"): return false
		var deadline := Time.get_ticks_msec()+1800
		while Time.get_ticks_msec()<deadline:
			if hud.hud_snapshot().scene_effects.any(func(effect):
				return effect.skill==skill and effect.delay==0 and effect.sprite_visible):
				break
			await process_frame
		if not check(hud.hud_snapshot().scene_effects.any(func(effect):
			return effect.skill==skill and effect.delay==0 and effect.sprite_visible),
			"死亡特效到期后显示带纹理的帧"): return false
		hud.remove_scene_actor(actor)
	# 缺少完整站立帧的外观也可以从32和33首次入场。
	var changed_actor := 10131
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,changed_actor,cell,0,20))
		and hud.apply_game_message(game_reply(Mir3HudView.SM_FEATURECHANGED,changed_actor,3,182,0))
		and hud.hud_snapshot().scene_monster_shapes[changed_actor]==182,
		"41号外观更新不依赖完整站立帧"): return false
	hud.remove_scene_actor(changed_actor)
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,changed_actor,cell,2,182))
		and hud.hud_snapshot().scene_monster_shapes[changed_actor]==182,
		"缺少站立帧的外观也可首次站立入场"): return false
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_NOWDEATH,changed_actor,cell,2,182))
		and hud.hud_snapshot().scene_monster_death_frames[changed_actor]==0,
		"缺帧对象仍处理后续死亡消息"): return false
	hud.remove_scene_actor(changed_actor)
	for fixture in [[Mir3HudView.SM_DEATH,9],[Mir3HudView.SM_SKELETON,0]]:
		var actor := 10132
		if not check(hud.apply_game_message(monster_packet(fixture[0],actor,cell,2,182)),
			"从尸体消息创建外观182"): return false
		var corpse: Dictionary=hud.hud_snapshot()
		if not check(corpse.scene_monster_shapes[actor]==182 and corpse.scene_monster_facings[actor]==2
			and corpse.scene_monster_death_frames[actor]==fixture[1],"首次入场直接应用尸体姿势"): return false
		hud.remove_scene_actor(actor)
	# 182与185共用605，但分别按死亡包朝向选取不同的帧段。
	for fixture in [[182,2],[185,6]]:
		var shape: int=fixture[0]
		var facing: int=fixture[1]
		var actor := 9950+shape
		var effect_cell := cell+Vector2i(facing/2,0)
		if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,effect_cell,0,20)),
			"建立接收外观刷新和死亡的对象"): return false
		if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_NOWDEATH,actor,effect_cell,facing,shape)),
			"死亡包刷新外观 %d" % shape): return false
		if not check(hud.hud_snapshot().scene_effects.any(func(effect):
			return (effect.skill==605 and effect.cell==effect_cell and effect.delay==0
				and effect.frame==effect.first+10*facing)),
			"605从死亡包朝向选择起始帧"): return false
		hud.remove_scene_actor(actor)
	print("MONSTER_EMPTY_ACTIONS_OK shapes=5,186")
	return true

func check_monster_reentry() -> bool:
	if not check(hud.change_map("0","比奇省",SHOWCASE_ORIGIN,1),"重入专项清场"): return false
	var cell := SHOWCASE_ORIGIN+Vector2i(0,2)
	var entry := monster_packet(Mir3HudView.SM_TURN,9901,cell,6,21)
	if not check(hud.apply_game_message(entry),"建立旧对象"): return false
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_DEATH,9901,cell,6,21)),"旧对象成为尸体"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_DISAPPEAR,9901,0,0,0)) and hud.apply_game_message(entry),"同批删除后重新入场"): return false
	await settle()
	if not check(hud.hud_snapshot().scene_monster_shapes.has(9901) and hud.hud_snapshot().scene_monster_death_frames[9901]==-1,"删除阶段先于新入场，新对象存活"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CLEAROBJECTS,0,0,0,0)) and hud.apply_game_message(entry),"同批清场后重新入场"): return false
	await settle()
	if not check(hud.hud_snapshot().scene_monster_shapes.has(9901),"清场后新对象保留"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_DISAPPEAR,9901,0,0,0)),"最终离场"): return false
	await settle()
	if not check(not hud.hud_snapshot().scene_monster_shapes.has(9901),"最终离场释放对象"): return false
	# 29与803同30进入离场队列，下一帧直接释放普通怪物。
	for opcode in [Mir3HudView.SM_29, Mir3HudView.SM_803]:
		if not check(hud.apply_game_message(entry),"建立离场消息测试对象"): return false
		if not check(hud.apply_game_message(game_reply(opcode,9901,0,0,0)),"接受同队列离场消息 %d" % opcode): return false
		if not check(hud.hud_snapshot().scene_monster_shapes.has(9901),"离场消息先入队"): return false
		await process_frame
		await process_frame
		if not check(not hud.hud_snapshot().scene_monster_shapes.has(9901),"普通离场在下一帧释放对象"): return false
	print("MONSTER_REENTRY_OK")
	return true

func check_monster_identity() -> bool:
	if not check(hud.change_map("0","比奇省",SHOWCASE_ORIGIN,1),"姓名专项清场"): return false
	var cell := SHOWCASE_ORIGIN+Vector2i(0,2)
	var entry := monster_packet(Mir3HudView.SM_TURN,9900,cell,6,119)
	entry.append_array(encode_scene_bytes(PackedByteArray([0xd6,0xed,47,50,52,57])))
	if not check(hud.apply_game_message(entry),"入场姓名携带十进制颜色249"): return false
	await settle()
	var hover := InputEventMouseMotion.new()
	hover.position=hud.hud_snapshot().scene_chicken_bounds[9900].get_center()
	root.push_input(hover,true)
	await settle()
	if not check(hud.hud_snapshot().displayed_name_actor==9900 and hud.hud_snapshot().displayed_name_color==255,"入场解析249为原版红色"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CHANGENAMECOLOR,9900,255,0,0)),"656改名色"): return false
	await settle()
	if not check(hud.hud_snapshot().displayed_name_color==0xffffff,"656更新白色"): return false
	var name_packet := game_reply(Mir3HudView.SM_USERNAME,9900,249,0,0)
	name_packet.append_array(encode_scene_bytes(PackedByteArray([0xd6,0xed])))
	if not check(hud.apply_game_message(name_packet),"42更新名字与颜色"): return false
	await settle()
	if not check(hud.hud_snapshot().displayed_name_color==255,"42应用颜色"): return false
	# 名称缺少颜色后缀时，站立和现身刷新沿用当前颜色。
	for opcode in [Mir3HudView.SM_TURN,Mir3HudView.SM_801,Mir3HudView.SM_ACTOR_STATUS,Mir3HudView.SM_808]:
		var refresh := monster_packet(opcode,9900,cell,6,119)
		var next_name := "Monster%d" % opcode
		refresh.append_array(encode_scene_bytes(next_name.to_ascii_buffer()))
		if not check(hud.apply_game_message(refresh),"接收无颜色后缀的名称"): return false
		await settle()
		var state: Dictionary=hud.hud_snapshot()
		if not check(state.displayed_name_text==next_name and state.displayed_name_color==255,
			"刷新名称时保留已有颜色"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_FEATURECHANGED,9900,3,23,0)) and hud.hud_snapshot().scene_monster_shapes[9900]==23,"41从消息头DWORD跨外观变形"): return false
	print("MONSTER_IDENTITY_OK")
	return true

func check_relay_queue(origin: Vector2i, cell: Vector2i) -> bool:
	# 1309出队时先受击，再把31追加队尾；已经等待的回程应先于复制受击。
	var outward := monster_packet(Mir3HudView.SM_WALK,6,origin,0,20)
	var returning := monster_packet(Mir3HudView.SM_WALK,6,cell,4,20)
	var relay := monster_packet(Mir3HudView.SM_HUMAN_STRUCK,6,Vector2i(9,10),1,20)
	if not check(hud.apply_game_message(outward) and hud.apply_game_message(relay) and hud.apply_game_message(returning),"移动、1309、回程依次入队"): return false
	if not check(hud.hud_snapshot().scene_chicken_struck[6]==-1,"1309不抢占移动"): return false
	var deadline := Time.get_ticks_msec()+2500
	while hud.hud_snapshot().scene_chicken_struck[6]<0 and Time.get_ticks_msec()<deadline: await settle(0.01)
	if not check(hud.hud_snapshot().scene_chicken_struck[6]>=0 and hud.hud_snapshot().scene_chicken_cells[6]==origin and hud.hud_snapshot().scene_health_values[6][0]==9,"1309在原位置受击并更新生命"): return false
	if not check(await wait_for_monster_step(6,cell),"回程在复制31之前执行"): return false
	deadline=Time.get_ticks_msec()+2500
	while hud.hud_snapshot().scene_chicken_struck[6]<0 and Time.get_ticks_msec()<deadline: await settle(0.01)
	if not check(hud.hud_snapshot().scene_chicken_struck[6]>=0 and hud.hud_snapshot().scene_chicken_cells[6]==cell,"复制31在回程终点再次受击，不后退"): return false
	await settle(0.5)
	if not check(hud.hud_snapshot().scene_chicken_steps[6]==-1 and hud.hud_snapshot().scene_chicken_struck[6]==-1,"复制受击结束后站立"): return false
	var old_hit := monster_packet(Mir3HudView.SM_STRUCK,6,Vector2i(7,10),1,20)
	if not check(hud.apply_game_message(outward) and hud.apply_game_message(old_hit)
		and hud.apply_game_message(returning),"移动期间积压完整31及回程"): return false
	if not check(hud.hud_snapshot().scene_health_values[6][0]==9,"只有一条待处理消息时保留31"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CHARSTATUSCHANGE,6,0,0x0400,0)),
		"接收657即时状态"): return false
	var immediate: Dictionary=hud.hud_snapshot()
	if not check(immediate.scene_monster_status_colors[6]==0 and immediate.scene_health_values[6][0]==9,
		"657立即更新灰阶状态，保留排队31的生命值"): return false
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,6,cell,4,20)),
		"新站立消息触发折叠"): return false
	if not check(hud.hud_snapshot().scene_health_values[6][0]==7 and hud.hud_snapshot().scene_chicken_struck[6]==-1,
		"折叠完整31只同步生命"): return false
	if not check(await wait_for_monster_step(6,cell),"折叠保留已排队的回程"): return false
	deadline=Time.get_ticks_msec()+1500
	while hud.hud_snapshot().scene_chicken_steps[6]>=0 and Time.get_ticks_msec()<deadline:
		await process_frame
	if not check(hud.hud_snapshot().scene_chicken_steps[6]==-1,"回程动作结束后交还测试场景"): return false
	print("MONSTER_RELAY_QUEUE_OK")
	return true

func check_burrow_removal(cell: Vector2i) -> bool:
	# 42E780: 正在钻回的对象先完成动作，移除包清队列但不截断动画。
	var actor := 9603
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,cell,0,3)),"移除专项建立对象"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_DIGDOWN,actor,cell.x,cell.y,0)),"开始钻回"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_DISAPPEAR,actor,0,0,0)),"钻回期间收到移除"): return false
	await settle(0.2)
	if not check(hud.hud_snapshot().scene_monster_shapes.has(actor) and hud.hud_snapshot().scene_monster_burrow_frames[actor]>=0,"移除不截断钻回动画"): return false
	await settle(1.4)
	if not check(not hud.hud_snapshot().scene_monster_shapes.has(actor),"动画完成删除对象"): return false
	print("MONSTER_BURROW_REMOVAL_OK")
	return true

func check_burrow_lifecycle(cell: Vector2i) -> bool:
	# 20号倒播不代表隐藏；21号完成删除。门对象由原版单独保留开关姿势。
	if not check(hud.change_map("0","比奇省",SHOWCASE_ORIGIN,1),"钻地专项清理场景"): return false
	cell=SHOWCASE_ORIGIN+Vector2i(0,2)
	var selected: Array[int]=[]
	for arg in OS.get_cmdline_user_args():
		if arg.begins_with("--shapes="):
			for value in arg.get_slice("=",1).split(","): selected.append(int(value))
	for shape in [3,4,31,49,56,83,84,85,89,90,530,531,532,533,538]:
		if not selected.is_empty() and not shape in selected: continue
		var actor: int=9500+shape
		var gate: bool=shape>=530
		var wing := cell+Vector2i(-1,0) if shape in [530,531] else cell+Vector2i(1,1)
		if gate:
			if not check(hud.walk_to_map_cell(wing) and hud.hud_snapshot().route_destination==wing,
				"地形本身不阻挡门翼测试格"): return false
			hud.walk_to_map_cell(SHOWCASE_ORIGIN)
		if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_DIGUP,actor,cell,0 if shape==538 else 6,shape)),"20号创建并钻出 shape=%d" % shape): return false
		if not check(hud.hud_snapshot().scene_monster_reverse_burrows[actor]==(shape in [3,31]),"钻出方向使用原版倒序分支"): return false
		if gate:
			# 408720 选门的开启动作时仍保持关闭朝向，直到 40E947。
			var opens_early: bool=hud.walk_to_map_cell(cell) and hud.hud_snapshot().route_destination==cell
			if not check(not opens_early,"开门动画开始时中心格仍挡路"): return false
			hud.walk_to_map_cell(SHOWCASE_ORIGIN)
		await settle(2.7 if shape==85 else 1.5)
		if not check(hud.hud_snapshot().scene_monster_shapes.has(actor) and hud.hud_snapshot().scene_monster_burrow_frames[actor]==-1,"钻出完成保留对象 shape=%d" % shape): return false
		if gate and not check(hud.hud_snapshot().scene_monster_facings[actor]==7,"门打开后固定方向7"): return false
		if gate:
			if not check(hud.walk_to_map_cell(cell) and hud.hud_snapshot().route_destination==cell,
				"打开的怪物门可走过中心格"): return false
			hud.walk_to_map_cell(SHOWCASE_ORIGIN)
		if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_DIGDOWN,actor,cell.x,cell.y,0)),"21号钻回"): return false
		if not check(hud.hud_snapshot().scene_monster_reverse_burrows[actor]==(not shape in [3,31]),"钻回倒序与钻出相反"): return false
		if gate:
			# 408BE8 仅对 530–533 立即设为方向 0，538 在动画结束才切换。
			var still_open: bool=hud.walk_to_map_cell(cell) and hud.hud_snapshot().route_destination==cell
			if not check(still_open==(shape==538),"关门动画开始时按原版外观决定中心碰撞"): return false
			hud.walk_to_map_cell(SHOWCASE_ORIGIN)
		await settle(2.7 if shape==85 else 1.5)
		if gate:
			if not check(hud.hud_snapshot().scene_monster_shapes.has(actor) and hud.hud_snapshot().scene_monster_facings[actor]==0,"门关闭后保留并固定方向0"): return false
			if not check(hud.walk_to_map_cell(wing) and hud.hud_snapshot().route_destination!=wing,
				"关闭的怪物门门翼阻挡目标格"): return false
			hud.walk_to_map_cell(SHOWCASE_ORIGIN)
			hud.remove_scene_actor(actor)
		elif not check(not hud.hud_snapshot().scene_monster_shapes.has(actor),"钻回结束删除对象及资源 shape=%d" % shape): return false
		print("MONSTER_BURROW_CASE_OK shape=",shape)
	print("MONSTER_BURROW_LIFECYCLE_OK")
	return true

func check_corrected_wire(cell: Vector2i) -> bool:
	# 按4AF3D4虚表验证消息语义，不能复用旧偏移的期望值。
	var actor := 9085
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,cell,0,85)),"10号站立入场"): return false
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_DIGUP,actor,cell,0,85)),"20号启动教主苏醒"): return false
	# 苏醒期间排入携带灰阶位的尸体消息，状态要到出队才更新。
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_DEATH,actor,cell,0,85,0,0x04000000)),"32号尸体排队"): return false
	if not check(hud.hud_snapshot().scene_monster_status_colors[actor]==65535 and hud.hud_snapshot().scene_monster_death_frames[actor]==-1,"排队不提前改变状态和死亡姿势"): return false
	await settle(2.8)
	if not check(hud.hud_snapshot().scene_monster_status_colors[actor]==0 and hud.hud_snapshot().scene_monster_death_frames[actor]>=0,"出队时应用尸体及状态"): return false
	# 10号刷新尸体的位置和外观，1316才清除死亡标志。
	var corpse_cell := cell+Vector2i(0,1)
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,corpse_cell,6,20)),
		"10号刷新已有尸体"): return false
	var corpse: Dictionary=hud.hud_snapshot()
	if not check(corpse.scene_chicken_cells[actor]==corpse_cell and corpse.scene_monster_facings[actor]==6
		and corpse.scene_monster_shapes[actor]==20 and corpse.scene_monster_death_frames[actor]==9
		and corpse.scene_monster_status_colors[actor]==65535,"站立包更新尸体但保留死亡末帧"): return false
	# 已死亡对象换成不同帧数的外观后，34号同样取新动作的末帧。
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_NOWDEATH,actor,cell,4,30)),
		"34号刷新已有尸体"): return false
	corpse=hud.hud_snapshot()
	if not check(corpse.scene_chicken_cells[actor]==cell and corpse.scene_monster_shapes[actor]==30
		and corpse.scene_monster_death_frames[actor]==3,"34号更新尸体后取新外观的死亡末帧"): return false
	# 比奇障碍测试中，475,360 西邻的格子不可行走。
	var blocked_cell := Vector2i(474,360)
	if not check(not hud.set_scene_chicken(actor+1,blocked_cell),"确认复活目标格不可行走"): return false
	# 首次站立和钻出均按服务器坐标创建怪物。
	for opcode in [Mir3HudView.SM_TURN,Mir3HudView.SM_DIGUP]:
		if not check(hud.apply_game_message(monster_packet(opcode,actor+1,blocked_cell,6,85)),
			"首次入场接受地图内阻挡格"): return false
		var arrival: Dictionary=hud.hud_snapshot()
		if not check(arrival.scene_chicken_cells[actor+1]==blocked_cell
			and arrival.scene_monster_burrow_frames[actor+1]==(0 if opcode==Mir3HudView.SM_DIGUP else -1),
			"新对象在包坐标选择站立或钻出动作"): return false
		hud.remove_scene_actor(actor+1)
	# 13号允许创建，但普通怪物队列不读取它的方向和光照。
	var run := game_reply(Mir3HudView.SM_RUN,actor+1,blocked_cell.x,blocked_cell.y,255|(12<<8))
	var run_feature := PackedByteArray()
	run_feature.resize(16)
	run_feature[0]=3
	run_feature[2]=20
	run.append_array(encode_scene_bytes(run_feature))
	if not check(hud.apply_game_message(run),"13号用构造参数建立普通怪物"): return false
	var standing: Dictionary=hud.hud_snapshot()
	if not check(standing.scene_chicken_cells[actor+1]==blocked_cell and standing.scene_monster_facings[actor+1]==0
		and standing.scene_monster_lights[actor+1]==0 and standing.scene_chicken_steps[actor+1]==-1,
		"13号新怪物保留构造时的站立、朝向和光照"): return false
	hud.remove_scene_actor(actor+1)
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_REVIVE,actor,blocked_cell,4,85)),"1316号按服务器坐标复活"): return false
	var revived: Dictionary=hud.hud_snapshot()
	if not check(revived.scene_chicken_cells[actor]==blocked_cell and revived.scene_monster_death_frames[actor]==-1
		and revived.scene_chicken_steps[actor]==-1,"复活清除尸体并在目标格站立"): return false
	# 两种移动都服从服务器指定的阻挡格，后退仍播放倒序移动。
	for opcode in [Mir3HudView.SM_WALK,Mir3HudView.SM_BACKSTEP]:
		if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,Vector2i(475,360),6,20))
			and hud.apply_game_message(monster_packet(opcode,actor,blocked_cell,6,20)),"接受指向阻挡格的移动包"): return false
		var moving: Dictionary=hud.hud_snapshot()
		if not check(moving.scene_chicken_cells[actor]==blocked_cell and moving.scene_chicken_steps[actor]==0
			and moving.scene_monster_walk_poses[actor]==(5 if opcode==Mir3HudView.SM_BACKSTEP else 0),
			"服务器移动进入阻挡格并选择对应动作帧"): return false
		await settle(1.0)
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CHARSTATUSCHANGE,actor,0,0x0400,0)) and hud.hud_snapshot().scene_monster_status_colors[actor]==0,"657即时更新状态"): return false
	hud.remove_scene_actor(actor)
	for transfer in [[Mir3HudView.SM_801,321],[Mir3HudView.SM_ACTOR_STATUS,0],[Mir3HudView.SM_808,323]]:
		if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,cell,0,85))
			and hud.apply_game_message(monster_packet(Mir3HudView.SM_DIGUP,actor,cell,0,85)),"建立正在苏醒的怪物"): return false
		var destination := cell+Vector2i(0,1)
		var feature := PackedByteArray()
		feature.resize(16)
		feature[0]=3
		feature[2]=23
		feature.encode_u32(4,0x04000000)
		var packet := game_reply(transfer[0],actor,destination.x,destination.y,4|(11<<8))
		packet.append_array(encode_scene_bytes(feature))
		if not check(hud.apply_game_message(packet),"现身消息在苏醒期间排队"): return false
		var state: Dictionary=hud.hud_snapshot()
		if not check(state.scene_monster_shapes[actor]==85 and state.scene_monster_lights[actor]==0
			and state.scene_chicken_cells[actor]==cell,"排队期间保留旧外观、光属性和坐标"): return false
		# 654立即取第一参数低字节，队列里的现身包稍后再应用自己的光照。
		if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CHANGELIGHT,actor,0x0107,0,0)),
			"苏醒期间接收654"): return false
		state=hud.hud_snapshot()
		if not check(state.scene_monster_lights[actor]==7 and state.scene_monster_shapes[actor]==85
			and state.scene_monster_burrow_frames[actor]>=0,"654立即更新光照且不打断苏醒"): return false
		var deadline := Time.get_ticks_msec()+3600
		while hud.hud_snapshot().scene_monster_shapes[actor]==85 and Time.get_ticks_msec()<deadline:
			await process_frame
		state=hud.hud_snapshot()
		if not check(state.scene_monster_shapes[actor]==23 and state.scene_monster_lights[actor]==11
			and state.scene_monster_facings[actor]==4 and state.scene_chicken_cells[actor]==destination
			and state.scene_monster_status_colors[actor]==0,"出队时应用完整现身外观和状态"): return false
		if transfer[1]!=0:
			if not check(state.scene_effects.any(func(effect): return effect.skill==transfer[1]),"出队触发对应现身特效"): return false
		if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_DEATH,actor,destination,4,23)),
			"建立已有尸体"): return false
		feature[2]=20
		var corpse_transfer := game_reply(transfer[0],actor,cell.x,cell.y,6|(12<<8))
		corpse_transfer.append_array(encode_scene_bytes(feature))
		if not check(hud.apply_game_message(corpse_transfer),"尸体接收现身刷新"): return false
		state=hud.hud_snapshot()
		if not check(state.scene_monster_shapes[actor]==20 and state.scene_monster_lights[actor]==12
			and state.scene_chicken_cells[actor]==cell and state.scene_monster_facings[actor]==6
			and state.scene_monster_death_frames[actor]==9,"现身刷新尸体外观和位置并取死亡末帧"): return false
		# 407CC0 先选站立动作，40DA8A 随后把骸骨切换为普通尸体末帧。
		if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_SKELETON,actor,cell,6,20))
			and hud.hud_snapshot().scene_monster_death_frames[actor]==0,"33号先选择骸骨姿势"): return false
		if not check(hud.apply_game_message(corpse_transfer)
			and hud.hud_snapshot().scene_monster_death_frames[actor]==9,"骸骨收到现身刷新后选择普通尸体末帧"): return false
		hud.remove_scene_actor(actor)
	# 离场特效排在走路之后，并取出队时的怪物坐标。
	for fixture in [[Mir3HudView.SM_800,322],[Mir3HudView.SM_806,320]]:
		var destination := cell+Vector2i(0,1)
		var skill: int=fixture[1]
		if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,cell,4,20))
			and hud.apply_game_message(monster_packet(Mir3HudView.SM_WALK,actor,destination,4,20))
			and hud.apply_game_message(game_reply(fixture[0],actor,cell.x,cell.y,0)),
			"移动期间接收离场包"): return false
		if not check(not hud.hud_snapshot().scene_effects.any(func(effect): return effect.skill==skill),
			"离场特效等待移动结束"): return false
		var deadline := Time.get_ticks_msec()+2000
		while Time.get_ticks_msec()<deadline:
			if hud.hud_snapshot().scene_effects.any(func(effect): return effect.skill==skill): break
			await process_frame
		var departed: Dictionary=hud.hud_snapshot()
		if not check(departed.scene_effects.any(func(effect): return effect.skill==skill and effect.cell==destination)
			and departed.scene_chicken_cells.get(actor)==destination,
			"离场特效使用对象坐标且对象仍在场景中"): return false
		hud.remove_scene_actor(actor)
	# 1317使用出队时的对象朝向，消息中的坐标和方向不参与动作选择。
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,cell,4,10))
		and hud.apply_game_message(monster_packet(Mir3HudView.SM_WALK,actor,cell+Vector2i(0,1),4,10))
		and hud.apply_game_message(game_reply(Mir3HudView.SM_1317,actor,cell.x,cell.y,6)),
		"移动期间接收1317"): return false
	if not check(hud.hud_snapshot().scene_monster_attack_frames[actor]==-1,"1317等待走路完成"): return false
	var action_deadline := Time.get_ticks_msec()+2000
	while hud.hud_snapshot().scene_monster_attack_frames[actor]<0 and Time.get_ticks_msec()<action_deadline:
		await process_frame
	var action_state: Dictionary=hud.hud_snapshot()
	if not check(action_state.scene_monster_attack_actions[actor]==6 and action_state.scene_monster_facings[actor]==4
		and action_state.scene_chicken_cells[actor]==cell+Vector2i(0,1)
		and action_state.scene_effects.any(func(effect): return effect.skill==501 and effect.cell==cell+Vector2i(0,1)),
		"1317以当前坐标和朝向播放动作6与501"): return false
	hud.remove_scene_actor(actor)
	# 4097CF把受击对象传给400系列特效，特效记录应指向该对象。
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,cell,4,20))
		and hud.apply_game_message(monster_packet(Mir3HudView.SM_STRUCK,actor,Vector2i(5,10),2,20,16)),
		"怪物接收带命中特效的31号包"): return false
	if not check(hud.hud_snapshot().scene_effects.any(func(effect):
		return effect.skill==416 and effect.target_actor==actor and effect.cell==cell),
		"416号命中特效绑定受击怪物"): return false
	var hit_destination := cell+Vector2i(1,0)
	if not check(hud.set_scene_chicken(actor,hit_destination),
		"命中特效期间直接更新场景对象位置"): return false
	var hit_deadline := Time.get_ticks_msec()+1500
	var followed := false
	while Time.get_ticks_msec()<hit_deadline:
		await process_frame
		followed=hud.hud_snapshot().scene_effects.any(func(effect):
			return effect.skill==416 and effect.target_actor==actor and effect.world_position==Vector2(hit_destination*Vector2i(48,32)))
		if followed: break
	if not check(followed,"416更新到受击怪物的新坐标"): return false
	hud.remove_scene_actor(actor)
	if not check(hud.hud_snapshot().scene_effects.any(func(effect):
		return effect.skill==416 and effect.target_actor==0 and effect.cell==hit_destination),
		"目标离场后416在最后一格继续播放"): return false
	# 14先写包坐标，再按包朝向选择普通攻击动作。
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,cell,4,20))
		and hud.apply_game_message(game_reply(Mir3HudView.SM_HIT,actor,hit_destination.x,hit_destination.y,2)),
		"接收不同位置的普通攻击包"): return false
	var attack_state: Dictionary=hud.hud_snapshot()
	if not check(attack_state.scene_chicken_cells[actor]==hit_destination
		and attack_state.scene_monster_facings[actor]==2 and attack_state.scene_monster_attack_actions[actor]==2
		and attack_state.scene_monster_attack_frames[actor]==0,"14应用包坐标和朝向并启动攻击"): return false
	hud.remove_scene_actor(actor)
	# 22使用包头起点，正文单独提供目标坐标和目标ID。
	var launch := cell+Vector2i(1,0)
	var target := launch+Vector2i(0,3)
	var target_body := PackedByteArray()
	target_body.resize(8)
	target_body.encode_u16(0,target.x)
	target_body.encode_u16(2,target.y)
	target_body.encode_u32(4,actor+1)
	var ranged := game_reply(Mir3HudView.SM_FLYAXE,actor,launch.x,launch.y,4)
	ranged.append_array(encode_scene_bytes(target_body))
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,cell,2,7))
		and hud.apply_game_message(ranged),"接收带独立起点和目标的22号包"): return false
	var ranged_state: Dictionary=hud.hud_snapshot()
	if not check(ranged_state.scene_chicken_cells[actor]==launch and ranged_state.scene_monster_facings[actor]==4
		and ranged_state.scene_effects.any(func(effect):
			return (effect.skill==505 and not effect.projectile and effect.delay==500
				and effect.target_actor==actor+1 and effect.cell==target)),
		"22应用起点和朝向，505使用正文目标位置和ID"): return false
	hud.remove_scene_actor(actor)
	# 520用格坐标选择八方向帧，弹道起点仍换算为像素。
	target=launch+Vector2i(2,1)
	target_body.encode_u16(0,target.x)
	target_body.encode_u16(2,target.y)
	ranged=game_reply(Mir3HudView.SM_FLYAXE,actor,launch.x,launch.y,4)
	ranged.append_array(encode_scene_bytes(target_body))
	var last_target_cell := launch+Vector2i(4,2)
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,cell,2,33))
		and hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor+1,last_target_cell,2,20))
		and hud.apply_game_message(ranged),"接收外观33的远程攻击"): return false
	if not check(hud.hud_snapshot().scene_effects.any(func(effect):
		return (effect.skill==520 and effect.projectile and effect.delay==365 and effect.frame_offset==30
			and effect.world_position==Vector2(launch*Vector2i(48,32)))),
		"520从包头位置发射并使用格坐标方向"): return false
	hud.remove_scene_actor(actor)
	# 520没有爆炸资源，改用具有EXPLOSION条目的573验证落点。
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,cell,2,145))
		and hud.apply_game_message(ranged),"创建具有爆炸资源的573投射物"): return false
	hud.remove_scene_actor(actor+1)
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor+1,target,2,20)),
		"离场目标的编号在另一格重用"): return false
	await process_frame
	if not check(hud.hud_snapshot().scene_effects.any(func(effect):
		return (effect.skill==573 and effect.projectile and effect.target_actor==0
			and effect.destination==Vector2(last_target_cell*Vector2i(48,32)))),
		"573解除追踪并保留目标离场前的格子"): return false
	var impact_deadline := Time.get_ticks_msec()+1500
	var impact: Dictionary={}
	while Time.get_ticks_msec()<impact_deadline:
		await process_frame
		var impacts: Array=hud.hud_snapshot().scene_effects.filter(func(effect):
			return effect.skill==573 and not effect.projectile and effect.cell==last_target_cell
		)
		if not impacts.is_empty():
			impact=impacts[0]
			break
	if not check(not impact.is_empty() and impact.frame==2330 and impact.first==2330
		and impact.frame_offset==0 and impact.animation_elapsed_ms>0,
		"573在最后目标格显示爆炸首帧并继承动画计时"): return false
	# 520缺少EXPLOSION记录，抵达后销毁飞行对象。
	var cleanup_deadline := Time.get_ticks_msec()+1500
	while Time.get_ticks_msec()<cleanup_deadline:
		if not hud.hud_snapshot().scene_effects.any(func(effect): return effect.skill==520): break
		await process_frame
	if not check(not hud.hud_snapshot().scene_effects.any(func(effect): return effect.skill==520),
		"520抵达后结束且没有生成替代爆炸"): return false
	hud.remove_scene_actor(actor+1)
	hud.remove_scene_actor(actor)
	# 纵轴占优时原版先把速度比例存为单精度，纵轴截断为499。
	target_body.encode_u16(2,launch.y+11)
	ranged=game_reply(Mir3HudView.SM_FLYAXE,actor,launch.x,launch.y,4)
	ranged.append_array(encode_scene_bytes(target_body))
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,cell,2,33))
		and hud.apply_game_message(ranged),"接收纵轴占优的远程攻击"): return false
	if not check(hud.hud_snapshot().scene_effects.any(func(effect):
		return effect.skill==520 and effect.get("velocity")==Vector2(136,499)),
		"520速度使用原版单精度比例"): return false
	hud.remove_scene_actor(actor)
	# 外观169的九发595各用自己的目标格选择方向。
	ranged=game_reply(Mir3HudView.SM_FLYAXE,actor,launch.x,launch.y,3)
	ranged.append_array(encode_scene_bytes(target_body))
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,cell,2,169))
		and hud.apply_game_message(ranged),"接收外观169的扇形攻击"): return false
	var fan: Array=hud.hud_snapshot().scene_effects.filter(func(effect): return effect.skill==595)
	if not check(fan.size()==9 and fan.any(func(effect): return effect.delay==0 and effect.frame_offset==30)
		and fan.any(func(effect): return effect.delay==560),"595逐发按目标格选方向并分时发射"): return false
	hud.remove_scene_actor(actor)
	print("MONSTER_CORRECTED_WIRE_OK")
	return true

func check_zuma_lifecycle(cell: Vector2i) -> bool:
	# 仅重放服务器消息；全部特殊帧数、方向与状态恢复由C++处理。
	if not check(hud.change_map("0","比奇省",SHOWCASE_ORIGIN,1),"特殊怪物验收使用仓库空地"): return false
	cell=SHOWCASE_ORIGIN+Vector2i(0,2)
	for shape in [83,84,85]:
		var actor: int = 9000+shape
		if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,cell,6,shape,0,1)),"祖玛休眠入场"): return false
		await settle(0.3)
		if not check(hud.hud_snapshot().scene_monster_dormant[actor],"特征位1冻结休眠首帧"): return false
		if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CHARSTATUSCHANGE,actor,0,0,0)),"清除休眠位"): return false
		if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_DIGUP,actor,cell,6,shape)),"21号开始苏醒"): return false
		await settle(0.35)
		if not check(hud.hud_snapshot().scene_monster_burrow_frames[actor]>=1 and not hud.hud_snapshot().scene_monster_dormant[actor],"苏醒按原版帧钟推进"): return false
		if not await capture("monster-special-%d-awake" % shape): return false
		await settle(0.6)
		if shape==85 and not check(hud.hud_snapshot().scene_monster_burrow_frames[actor]>=0,"教主20帧苏醒不能按普通6帧提前结束"): return false
		await settle(2.0)
		if not check(hud.hud_snapshot().scene_monster_burrow_frames[actor]==-1 and not hud.hud_snapshot().scene_monster_dormant[actor],"苏醒完成转正常待机"): return false
		if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CHARSTATUSCHANGE,actor,1,0,0)),"重新设置休眠位"): return false
		if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_HIT,actor,cell.x,cell.y,4)),"播放攻击后检查状态恢复"): return false
		await settle(1.1)
		if not check(hud.hud_snapshot().scene_monster_dormant[actor],"动作结束重新应用休眠状态"): return false
		hud.remove_scene_actor(actor)
	print("MONSTER_ZUMA_LIFECYCLE_OK shapes=83,84,85")
	return true


func check_rise_health(cell: Vector2i, feature: PackedByteArray) -> bool:
	# 34号死亡清空填充但保留生命值，27号起身，53号恢复血量显示。
	feature[2]=30
	feature[3]=0
	feature[4]=0
	var bar_actor := game_reply(Mir3HudView.SM_TURN,27,cell.x,cell.y,0)
	bar_actor.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(bar_actor)
		and hud.apply_game_message(game_reply(Mir3HudView.SM_HEALTHSPELLCHANGED,27,5,0,10))
		and hud.apply_game_message(game_reply(Mir3HudView.SM_HEALTH_PERCENT,27,0,0,0)),
		"建立带半血血条的怪物"): return false
	await settle()
	if not check(hud.hud_snapshot().scene_health_bounds.get(27,Rect2()).size.x==17,"死亡前显示半血填充"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_OPENHEALTH,27,0,0,0)),
		"死亡前记录血条开启时间戳"): return false
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_NOWDEATH,27,cell,0,30)),
		"死亡动作开始"): return false
	var rise_cell := cell+Vector2i(0,1)
	var bar_rise := game_reply(Mir3HudView.SM_27,27,rise_cell.x,rise_cell.y,6|(12<<8))
	feature[2]=83
	bar_rise.append_array(encode_scene_bytes(feature))
	if not check(hud.apply_game_message(bar_rise),"死亡期间排队起身"): return false
	var queued: Dictionary=hud.hud_snapshot()
	if not check(queued.scene_chicken_cells[27]==cell and queued.scene_monster_shapes[27]==30
		and queued.scene_monster_lights[27]==0,"起身出队前保留旧坐标、外观和光照"): return false
	var rise_deadline := Time.get_ticks_msec()+3500
	while Time.get_ticks_msec()<rise_deadline:
		var state: Dictionary = hud.hud_snapshot()
		if state.scene_monster_death_frames[27]==-1 and state.scene_monster_burrow_frames[27]==-1: break
		await process_frame
	await settle()
	var revived: Dictionary = hud.hud_snapshot()
	if not check(revived.scene_chicken_cells[27]==rise_cell and revived.scene_monster_shapes[27]==83
		and revived.scene_monster_facings[27]==6 and revived.scene_monster_lights[27]==12,
		"起身出队时应用坐标、外观、朝向和光照"): return false
	var outside := game_reply(Mir3HudView.SM_27,27,65535,65535,0)
	outside.append_array(encode_scene_bytes(feature))
	if not check(not hud.apply_game_message(outside) and hud.hud_snapshot().scene_chicken_cells[27]==rise_cell,
		"越界起身包不移动现有怪物"): return false
	if not check(revived.scene_monster_death_frames[27]==-1 and revived.scene_monster_burrow_frames[27]==-1
		and revived.scene_health_values[27][0]==5 and revived.scene_health_bounds.get(27,Rect2()).size.x==0,
		"起身后保留生命值并显示空血条"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_HEALTHSPELLCHANGED,27,5,0,10)),
		"起身后接收血量回包"): return false
	await settle()
	# 起身正文未设置状态位2。53恢复血量填充，但不会重置已保存的时间戳。
	if not check(not hud.hud_snapshot().scene_health_bounds.has(27)
		and hud.hud_snapshot().scene_health_values[27][0]==5,"53更新生命但血条继续隐藏"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_OPENHEALTH,27,0,0,0)),
		"起身后重新同步开启时间戳"): return false
	await settle()
	if not check(hud.hud_snapshot().scene_health_bounds.get(27,Rect2()).size.x==17,"开启后显示已更新的半血填充"): return false
	# 1100保存当前时间戳，1101只清活动时间戳，使血条隐藏。
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_OPENHEALTH,27,0,0,0))
		and hud.apply_game_message(game_reply(Mir3HudView.SM_CLOSEHEALTH,27,0,0,0)),"接收血条开关消息"): return false
	await process_frame
	await process_frame
	if not check(not hud.hud_snapshot().scene_health_bounds.has(27),"1101隐藏已显示的血条"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_OPENHEALTH,27,0,0,0)),"1100重新开启血条"): return false
	await process_frame
	await process_frame
	if not check(hud.hud_snapshot().scene_health_bounds.get(27,Rect2()).size.x==17,"重新显示原有半血填充"): return false
	# 53从零开始写入生命时无飘字，后续非零生命差值产生恢复飘字。
	var damage_actor := 10253
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,damage_actor,cell,4,20))
		and hud.apply_game_message(game_reply(Mir3HudView.SM_HEALTHSPELLCHANGED,damage_actor,5,0,10)),
		"53首次记录生命值"): return false
	await RenderingServer.frame_post_draw
	if not check(not hud.hud_snapshot().scene_damage_frames.has(damage_actor),"零生命到非零无飘字"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_HEALTHSPELLCHANGED,damage_actor,8,0,10)),
		"53增加已有生命值"): return false
	await RenderingServer.frame_post_draw
	var damage: Dictionary=hud.hud_snapshot()
	if not check(damage.scene_damage_frames.has(damage_actor)==damage.damage_numbers,
		"非零生命差值按飘字选项显示"): return false
	if damage.damage_numbers:
		var before: Vector2i=damage.scene_damage_positions[damage_actor]
		if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,damage_actor,cell+Vector2i(1,0),4,20)),
			"飘字期间更新怪物位置"): return false
		await RenderingServer.frame_post_draw
		var after: Vector2i=hud.hud_snapshot().scene_damage_positions.get(damage_actor,Vector2i.ZERO)
		if not check(after.x>=before.x+48 and after.x<=before.x+51,"恢复飘字跟随怪物横移一格"): return false
	hud.remove_scene_actor(damage_actor)
	# 409691 在死亡动作4上只更新生命，不为后续受击新增飘字。
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_DEATH,damage_actor,cell,4,20))
		and hud.apply_game_message(monster_packet(Mir3HudView.SM_STRUCK,damage_actor,Vector2i(1,10),3,30)),
		"尸体接收受击血量记录"): return false
	await RenderingServer.frame_post_draw
	var dead_hit: Dictionary=hud.hud_snapshot()
	if not check(dead_hit.scene_health_values[damage_actor][0]==1 and not dead_hit.scene_damage_frames.has(damage_actor),
		"死亡动作上更新生命但无新飘字"): return false
	if not check(dead_hit.scene_monster_shapes[damage_actor]==30 and dead_hit.scene_monster_death_frames[damage_actor]==3,
		"31号刷新尸体外观并取新动作末帧"): return false
	# 33先选动作10，零伤害保留骸骨，非零伤害经动作3回到尸体末帧。
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_SKELETON,damage_actor,cell,4,20))
		and hud.apply_game_message(monster_packet(Mir3HudView.SM_STRUCK,damage_actor,Vector2i(0,10),0,30)),
		"骸骨接收零伤害受击包"): return false
	var skeleton: Dictionary=hud.hud_snapshot()
	if not check(skeleton.scene_monster_shapes[damage_actor]==30 and skeleton.scene_monster_death_frames[damage_actor]==0,
		"零伤害更新外观并保留骸骨"): return false
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_STRUCK,damage_actor,Vector2i(0,10),3,30))
		and hud.hud_snapshot().scene_monster_death_frames[damage_actor]==3,
		"非零伤害将骸骨切为普通尸体末帧"): return false
	hud.remove_scene_actor(damage_actor)
	# 53保存百分比低字节，绘制再用原版浮点常量截断宽度。
	for fixture in [[10,10,35],[300,100,15],[92,5,17],[5,0,0]]:
		if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_HEALTHSPELLCHANGED,27,
			fixture[0],0,fixture[1])),"接收临界血量回包"): return false
		await RenderingServer.frame_post_draw
		if not check(hud.hud_snapshot().scene_health_bounds.get(27,Rect2()).size.x==fixture[2],
			"血条宽度符合百分比低字节和截断规则"): return false
	return true

func check_special_follow() -> bool:
	if not check(hud.change_map("0","比奇省",SHOWCASE_ORIGIN,1),"跟随专项清场"): return false
	var cell := SHOWCASE_ORIGIN+Vector2i(0,2)
	var target := cell+Vector2i(0,2)
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,9800,cell,0,23)) and hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,9801,target,0,20)),"建立施法者与跟随目标"): return false
	var body := PackedByteArray()
	body.resize(16)
	body.encode_u32(0,target.x)
	body.encode_u32(4,target.y)
	body.encode_u32(8,9801)
	body.encode_u32(12,26)
	var packet := game_reply(Mir3HudView.SM_LIGHTING,9800,cell.x,cell.y,0)
	packet.append_array(encode_scene_bytes(body))
	if not check(hud.apply_game_message(packet),"26分支绑定目标"): return false
	await settle(0.72)
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_WALK,9801,target+Vector2i(0,1),4,20)),"移动被跟随目标"): return false
	await settle(0.15)
	var effects: Array=hud.hud_snapshot().scene_effects.filter(func(effect): return effect.skill==120)
	if not check(effects.size()==1,"跟随效果仍存活"): return false
	if not check(effects[0].target_actor==9801 and Vector2(effects[0].world_position).distance_to(Vector2(hud.hud_snapshot().scene_chicken_pixels[9801]))<10,"效果跟随目标的移动像素坐标"): return false
	var last_cell: Vector2i=hud.hud_snapshot().scene_chicken_cells[9801]
	var last_position := Vector2(last_cell*Vector2i(48,32))
	if not check(hud.remove_scene_actor(9801),"移除目标"): return false
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,9801,cell,0,20)),"同编号重新出现"): return false
	await settle(0.04)
	effects=hud.hud_snapshot().scene_effects.filter(func(effect): return effect.skill==120)
	if not check(effects.size()==1 and effects[0].target_actor==0 and effects[0].world_position==last_position,"目标删除后解绑并保留最后位置，不补绑同编号对象"): return false
	var npc_cell := target+Vector2i(1,0)
	var npc_feature := PackedByteArray()
	npc_feature.resize(16)
	npc_feature[0]=5
	npc_feature[2]=11
	# 42E9A0 接受移动包创建 NPC，42EB3D 固定传动作 0 和方向 0。
	var walk_entry := game_reply(Mir3HudView.SM_WALK,9802,npc_cell.x,npc_cell.y,255)
	walk_entry.append_array(encode_scene_bytes(npc_feature))
	if not check(hud.apply_game_message(walk_entry),"类型5可由移动包建立"): return false
	await RenderingServer.frame_post_draw
	if not check(hud.hud_snapshot().scene_npc_bounds[9802].size==Vector2(40,76)
		and hud.remove_scene_actor(9802),"移动包仍按方向0绘制站立帧"): return false
	var npc_entry := game_reply(Mir3HudView.SM_TURN,9802,npc_cell.x,npc_cell.y,0)
	npc_entry.append_array(encode_scene_bytes(npc_feature))
	if not check(hud.apply_game_message(npc_entry),"由站立包创建跟随特效的NPC目标"): return false
	await RenderingServer.frame_post_draw
	# NPC.wix 第1100帧为40×76，第110帧为28×66。
	if not check(hud.hud_snapshot().scene_npc_bounds[9802].size==Vector2(40,76),
		"类型5首次站立按100帧外观步长定位原资源"): return false
	if not check(hud.hud_snapshot().scene_npc_standing[9802]==Vector2i(4,300)
		and hud.apply_game_message(npc_entry)
		and hud.hud_snapshot().scene_npc_standing[9802]==Vector2i(10,300),
		"类型5首次站立使用动作0，刷新使用动作1"): return false
	# 4102D4 复活重启显现，普通站立刷新沿用当前进度。
	var npc_revive := game_reply(Mir3HudView.SM_REVIVE,9802,npc_cell.x,npc_cell.y,0)
	npc_revive.append_array(encode_scene_bytes(npc_feature))
	if not check(hud.apply_game_message(npc_revive)
		and hud.hud_snapshot().scene_npc_arrival_ms[9802]==0,"NPC复活重启显现"): return false
	var revive_deadline := Time.get_ticks_msec()+1800
	var revive_elapsed: float=0
	while Time.get_ticks_msec()<revive_deadline:
		await process_frame
		revive_elapsed=hud.hud_snapshot().scene_npc_arrival_ms[9802]
		if revive_elapsed<0 or revive_elapsed>=300: break
	if not check(revive_elapsed>=300 and revive_elapsed<600 and hud.apply_game_message(npc_entry)
		and hud.hud_snapshot().scene_npc_arrival_ms[9802]==revive_elapsed,
		"NPC复活显现超过300毫秒，普通刷新保留进度"): return false
	while Time.get_ticks_msec()<revive_deadline:
		await process_frame
		if hud.hud_snapshot().scene_npc_arrival_ms[9802]<0: break
	if not check(hud.hud_snapshot().scene_npc_arrival_ms[9802]<0,"NPC复活显现结束"): return false
	body.encode_u32(8,9802)
	packet=game_reply(Mir3HudView.SM_LIGHTING,9800,cell.x,cell.y,0)
	packet.append_array(encode_scene_bytes(body))
	if not check(hud.apply_game_message(packet),"接收以NPC为目标的26分支"): return false
	var deadline := Time.get_ticks_msec()+1800
	while Time.get_ticks_msec()<deadline:
		if hud.hud_snapshot().scene_effects.any(func(effect): return effect.skill==120 and effect.target_actor==9802): break
		await process_frame
	if not check(hud.hud_snapshot().scene_effects.any(func(effect): return effect.skill==120 and effect.target_actor==9802),
		"120绑定NPC目标"): return false
	if not check(hud.remove_scene_actor(9802) and hud.hud_snapshot().scene_effects.any(func(effect):
		return effect.skill==120 and effect.target_actor==0 and effect.cell==npc_cell),
		"NPC离场后120定位到最后一格"): return false
	# 外观99在当前NPC资源中缺帧，仍应创建可被移除的场景对象。
	npc_feature[2]=99
	npc_entry=game_reply(Mir3HudView.SM_TURN,9802,npc_cell.x,npc_cell.y,7)
	npc_entry.append_array(encode_scene_bytes(npc_feature))
	if not check(hud.apply_game_message(npc_entry),"缺帧NPC仍接受站立包"): return false
	await RenderingServer.frame_post_draw
	if not check(not hud.hud_snapshot().scene_npc_bounds[9802].has_area() and hud.remove_scene_actor(9802),
		"缺帧NPC无绘制范围但可移除"): return false
	npc_feature[0]=4
	npc_feature[1]=1
	npc_entry=game_reply(Mir3HudView.SM_TURN,9802,npc_cell.x,npc_cell.y,7)
	npc_entry.append_array(encode_scene_bytes(npc_feature))
	if not check(hud.apply_game_message(npc_entry),"类型4使用Flag资源入场"): return false
	await RenderingServer.frame_post_draw
	var arrival_ms: float=hud.hud_snapshot().scene_npc_arrival_ms[9802]
	if not check(arrival_ms>0 and arrival_ms<300 and hud.apply_game_message(npc_entry),
		"显现期间接收同一旗帜站立包"): return false
	if not check(hud.hud_snapshot().scene_npc_arrival_ms[9802]==arrival_ms,"重复站立保留显现计时"): return false
	await RenderingServer.frame_post_draw
	var flag_bounds: Rect2=hud.hud_snapshot().scene_npc_bounds[9802]
	if not check(flag_bounds.has_area(),"类型4绘制Flag原资源帧"): return false
	# 4102C2 从站立方向高字节取灯属性，4255E2 把旗帜灯加入场景。
	var flag_light_before: PackedColorArray=hud.hud_snapshot().scene_light_samples
	var lit_flag := game_reply(Mir3HudView.SM_TURN,9802,npc_cell.x,npc_cell.y,7+8*256)
	lit_flag.append_array(encode_scene_bytes(npc_feature))
	if not check(hud.apply_game_message(lit_flag),"旗帜刷新灯属性"): return false
	await settle()
	if not check(hud.hud_snapshot().scene_light_samples!=flag_light_before,"旗帜刷新改变夜间光照网格"): return false
	var flag_light_after: PackedColorArray=hud.hud_snapshot().scene_light_samples
	# 4247AF 的血条绘制分派跳过 NPC，1100 和 1101 只改校验计时。
	for opcode in [Mir3HudView.SM_OPENHEALTH,Mir3HudView.SM_CLOSEHEALTH]:
		if not check(hud.apply_game_message(game_reply(opcode,9802,0,0,0))
			and hud.apply_game_message(game_reply(Mir3HudView.SM_HEALTH_PERCENT,9802,50,0,100)),
			"旗帜接受血条消息"): return false
		await settle()
		if not check(not hud.hud_snapshot().scene_health_bounds.has(9802)
			and hud.hud_snapshot().scene_light_samples==flag_light_after,
			"血条消息不绘制旗帜血条，也不改变灯光"): return false
	var hover := InputEventMouseMotion.new()
	hover.position=flag_bounds.get_center()
	root.push_input(hover,true)
	# 41030D 读取刷新包的名称尾段，缺少斜杠时沿用原颜色。
	for label in ["Flag/249","Renamed"]:
		var renamed: PackedByteArray=npc_entry.duplicate()
		renamed.append_array(encode_scene_bytes(label.to_ascii_buffer()))
		if not check(hud.apply_game_message(renamed),"接收旗帜名称刷新"): return false
		await settle()
		var state: Dictionary=hud.hud_snapshot()
		if not check(state.displayed_name_actor==9802 and state.displayed_name_text==label.get_slice("/",0)
			and state.displayed_name_color==255,"悬停显示更新的旗帜名称和原版红色"): return false
	if not check(hud.remove_scene_actor(9802),"移除旗帜"): return false
	# 46F3B0 接收旗帜外观加77，外观10映射到10帧的特例87。
	for fixture in [[1,Vector2i(4,300)],[10,Vector2i(10,150)]]:
		npc_feature[1]=fixture[0]
		npc_entry=game_reply(Mir3HudView.SM_TURN,9802,npc_cell.x,npc_cell.y,0)
		npc_entry.append_array(encode_scene_bytes(npc_feature))
		if not check(hud.apply_game_message(npc_entry)
			and hud.hud_snapshot().scene_npc_standing[9802]==fixture[1],
			"旗帜外观选择原版站立帧数和间隔"): return false
		if not check(hud.hud_snapshot().scene_npc_standing_frames[9802]==0,"入场从首帧开始"): return false
		# 放大一次处理的delta，使它跨越多个动画间隔。
		var old_scale := Engine.time_scale
		Engine.time_scale=1000.0
		for attempt in range(3):
			await RenderingServer.frame_post_draw
			if hud.hud_snapshot().scene_npc_standing_frames[9802]!=0: break
		Engine.time_scale=old_scale
		if not check(hud.hud_snapshot().scene_npc_standing_frames[9802]==1,
			"一次长帧只推进一帧"): return false
		# 类型4在406750将动作归零，重新选取站立首帧。
		if not check(hud.apply_game_message(npc_entry)
			and hud.hud_snapshot().scene_npc_standing_frames[9802]==0,
			"旗帜重复站立回到首帧"): return false
		hud.remove_scene_actor(9802)
	npc_feature[0]=5
	npc_feature[2]=87
	npc_entry=game_reply(Mir3HudView.SM_TURN,9802,npc_cell.x,npc_cell.y,0)
	npc_entry.append_array(encode_scene_bytes(npc_feature))
	if not check(hud.apply_game_message(npc_entry) and hud.apply_game_message(npc_entry),
		"外观87首次入场并刷新"): return false
	if not check(hud.hud_snapshot().scene_effects.any(func(effect):
		return effect.skill==607 and effect.attached_actor==9802),"607附着于NPC"): return false
	npc_entry=game_reply(Mir3HudView.SM_TURN,9802,npc_cell.x,npc_cell.y,5)
	npc_entry.append_array(encode_scene_bytes(npc_feature))
	if not check(hud.apply_game_message(npc_entry),"改变NPC刷新方向"): return false
	var overlays: Array=hud.hud_snapshot().scene_effects.filter(func(effect):
		return effect.skill==607 and effect.attached_actor==9802)
	if not check(overlays.size()==1 and overlays[0].frame_offset==20,
		"607替换上次实例并使用方向2的帧段"): return false
	npc_feature[2]=11
	npc_entry=game_reply(Mir3HudView.SM_TURN,9802,npc_cell.x,npc_cell.y,0)
	npc_entry.append_array(encode_scene_bytes(npc_feature))
	if not check(hud.apply_game_message(npc_entry) and not hud.hud_snapshot().scene_effects.any(func(effect):
		return effect.skill==607 and effect.attached_actor==9802),"切换普通外观时清理607"): return false
	npc_feature[2]=87
	npc_entry=game_reply(Mir3HudView.SM_TURN,9802,npc_cell.x,npc_cell.y,0)
	npc_entry.append_array(encode_scene_bytes(npc_feature))
	if not check(hud.apply_game_message(npc_entry) and hud.hud_snapshot().scene_effects.any(func(effect):
		return effect.skill==607 and effect.attached_actor==9802),"切回外观87时重建607"): return false
	if not check(hud.remove_scene_actor(9802) and not hud.hud_snapshot().scene_effects.any(func(effect):
		return effect.skill==607 and effect.attached_actor==9802),"移除NPC时清理附着607"): return false
	print("MONSTER_SPECIAL_FOLLOW_OK branch=26")
	return true

func check_special_origin() -> bool:
	# 409DB0分支26反传坐标，但4495D0的120为定点模式，44B7E0使用终点。
	if not check(hud.change_map("0","比奇省",SHOWCASE_ORIGIN,1),"特殊落点清场"): return false
	var cell := SHOWCASE_ORIGIN+Vector2i(0,2)
	var target := cell+Vector2i(0,3)
	if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,9800,cell,0,23)),"建立施法者"): return false
	var body := PackedByteArray()
	body.resize(16)
	body.encode_u32(0,target.x)
	body.encode_u32(4,target.y)
	body.encode_u32(12,26)
	var packet := game_reply(Mir3HudView.SM_LIGHTING,9800,cell.x,cell.y,4)
	packet.append_array(encode_scene_bytes(body))
	if not check(hud.apply_game_message(packet),"26号分支接受"): return false
	var effects: Array=hud.hud_snapshot().scene_effects
	var matching: Array=effects.filter(func(effect): return effect.skill==120)
	if not check(matching.size()==1,"登记120号独立效果"): return false
	var effect: Dictionary=matching[0]
	if not check(effect.cell==cell and not effect.projectile and effect.delay==700 and effect.frame_offset==0,"无目标对象时在施法者终点定点播放，延时700ms，方向0"): return false
	print("MONSTER_SPECIAL_ORIGIN_OK branch=26")
	return true

func check_special_branches() -> bool:
	# 409DB0逐分支资源登记；此处验证分派，不代替弹道/延时/像素验收。
	var resources := [515,538,539,543,23,9,576,538,555,590,590,581,579,524,592,584,576,588,587,597,603,606,2,29,53,120]
	var missing: Array[int]=[]
	for index in range(resources.size()):
		if not check(hud.change_map("0","比奇省",SHOWCASE_ORIGIN,1),"分支间清理特效"): return false
		var cell := SHOWCASE_ORIGIN+Vector2i(0,2)
		var actor := 9700+index
		if not check(hud.apply_game_message(monster_packet(Mir3HudView.SM_TURN,actor,cell,0,13 if index==1 else 23)),"特殊攻击建立施法者"): return false
		var body := PackedByteArray()
		body.resize(16)
		body.encode_u32(0,cell.x)
		body.encode_u32(4,cell.y+4)
		body.encode_u32(12,index+1)
		var packet := game_reply(Mir3HudView.SM_LIGHTING,actor,cell.x,cell.y,0)
		packet.append_array(encode_scene_bytes(body))
		if not check(hud.apply_game_message(packet),"23分支%d接受" % (index+1)): return false
		var effects: Array=hud.hud_snapshot().scene_effects
		if not effects.any(func(effect): return effect.skill==resources[index]):
			missing.append(index+1)
			print("MONSTER_SPECIAL_BRANCH_MISSING branch=",index+1," resource=",resources[index])
			continue
		print("MONSTER_SPECIAL_BRANCH_OK branch=",index+1," resource=",resources[index])
	if not check(missing.is_empty(),"特殊攻击未登记资源的分支：%s" % [missing]): return false
	print("MONSTER_SPECIAL_BRANCHES_OK count=26")
	return true

func check_special_effect(origin: Vector2i, cell: Vector2i, feature: PackedByteArray) -> bool:
	# 23参与动作队列，657只更新状态；原版40D8C0调用+108。
	feature[2]=23
	var entry := monster_packet(Mir3HudView.SM_TURN,28,cell,0,23)
	var step := monster_packet(Mir3HudView.SM_WALK,28,origin,0,23)
	var body := PackedByteArray()
	body.resize(16)
	body.encode_u32(12,1)
	var attack := game_reply(Mir3HudView.SM_LIGHTING,28,origin.x,origin.y,0)
	attack.append_array(encode_scene_bytes(body))
	if not check(hud.apply_game_message(entry) and hud.apply_game_message(step),"建立移动中的怪物"): return false
	if not check(hud.apply_game_message(attack) and hud.hud_snapshot().scene_chicken_steps[28]>=0
		and hud.hud_snapshot().scene_monster_attack_actions[28]==0,"23入队不打断当前移动"): return false
	var deadline := Time.get_ticks_msec()+2000
	while hud.hud_snapshot().scene_monster_attack_actions[28]!=2 and Time.get_ticks_msec()<deadline:
		await settle(0.02)
	if not check(hud.hud_snapshot().scene_chicken_steps[28]==-1 and hud.hud_snapshot().scene_monster_attack_actions[28]==2,"移动完成后启动特殊攻击"): return false
	await settle(1.5)
	# 分支4只转向，不采纳消息头的位置。
	body.encode_u32(0,cell.x)
	body.encode_u32(4,cell.y)
	body.encode_u32(12,4)
	var fixed_target := game_reply(Mir3HudView.SM_LIGHTING,28,cell.x,cell.y,2)
	fixed_target.append_array(encode_scene_bytes(body))
	if not check(hud.apply_game_message(fixed_target) and hud.hud_snapshot().scene_chicken_cells[28]==origin
		and hud.hud_snapshot().scene_monster_facings[28]==2,"23分支4保留施法者位置并转向"): return false
	body.encode_u32(4,cell.y+5)
	body.encode_u32(12,25)
	var wave := game_reply(Mir3HudView.SM_LIGHTING,27,cell.x,cell.y,0)
	wave.append_array(encode_scene_bytes(body))
	if not check(hud.apply_game_message(wave),"23分支25启动地面波"): return false
	var delays: Dictionary=hud.hud_snapshot().map_ground_wave_delays
	if not check(delays.size()==9,"地面波布置九格"): return false
	for index in range(9):
		if not check(delays.get(cell+Vector2i(0,index+1),-1)==150*index,"地面波逐格延迟150毫秒"): return false
	await settle(0.6)
	if not await capture("map-monster-ground-wave"): return false
	print("MONSTER_SPECIAL_QUEUE_OK branches=1,4,25")
	return true

func check_status_blend() -> bool:
	# 657的高WORD依次设置灰阶、减速色与半透明，核对重叠位优先级。
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CHARSTATUSCHANGE,23,0,0x0400,0))
		and hud.hud_snapshot().scene_monster_status_colors[23]==0,"灰阶状态选择模式0"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CHARSTATUSCHANGE,23,0,0x0c00,0))
		and hud.hud_snapshot().scene_monster_status_colors[23]==38079,"减速色覆盖灰阶"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CHARSTATUSCHANGE,23,0,0x0c02,0))
		and hud.hud_snapshot().scene_monster_status_colors[23]==1,"半透明状态优先于颜色状态"): return false
	if not await capture("map-monster-status-blend"): return false
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CHARSTATUSCHANGE,23,0,0,0))
		and hud.hud_snapshot().scene_monster_status_colors[23]==65535,"清除状态后恢复普通显示"): return false
	return true
