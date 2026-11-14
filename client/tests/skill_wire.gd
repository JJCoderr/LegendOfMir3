# 测试内容：技能原始分段列表、增量追加、修炼等级、删除及快捷键保留、冲突清除后绑定的原包顺序、畸形包原子拒绝和离场清理。
extends "login_smoke.gd"

func record(id: int, key_code: int, level: int = 1) -> PackedByteArray:
	var raw := PackedByteArray()
	raw.resize(74)
	raw[0]=42
	raw[4]=key_code
	raw[5]=level
	raw[6]=id&255
	raw[7]=(id>>8)&255
	var name := "Magic".to_ascii_buffer()
	for i in range(name.size()): raw[8+i]=name[i]
	raw[31]=100
	raw[35]=200
	raw[39]=44
	raw[40]=1
	return encode_scene_bytes(raw)
func records_packet(opcode: int, records: Array[PackedByteArray]) -> PackedByteArray:
	var packet := game_reply(opcode,0,0,0,0xab00|records.size())
	for row in records:
		packet.append_array(row)
		if opcode==Mir3HudView.SM_SENDMYMAGIC: packet.append(47)
	return packet
func binding_packet(id: int, key_code: int, sequence: int) -> PackedByteArray:
	var packet := ("#"+str(sequence)).to_ascii_buffer()
	packet.append_array(encode_scene_bytes(PackedByteArray([id&255,(id>>8)&255,0,0,Mir3HudView.CM_MAGICKEYCHANGE&255,Mir3HudView.CM_MAGICKEYCHANGE>>8,key_code,0,0,0,0,0])))
	packet.append(33)
	return packet

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("options_path","user://skill-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	var packets: Array[PackedByteArray] = []
	hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(packet))
	var full := records_packet(Mir3HudView.SM_SENDMYMAGIC,[record(1,49),record(5,0)])
	if not check(hud.apply_game_message(full) and hud.hud_snapshot().learned_skills.size()==2 and hud.hud_snapshot().skill_bindings[1]==1,"分段原包列表及绑定"): return
	key(KEY_E)
	var motion := InputEventMouseMotion.new()
	motion.position=Vector2(385,152)
	Input.parse_input_event(motion)
	key(KEY_F1)
	key(KEY_F12)
	if not check(packets==[binding_packet(1,0,1),binding_packet(5,49,2),binding_packet(5,67,3)],"冲突清除再绑定、F12编码及序号"): return
	if not check(hud.hud_snapshot().skill_bindings[1]==0 and hud.hud_snapshot().skill_bindings[12]==5,"本地快捷键同步"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_MAGIC_LVEXP,5,0xff02,0xcdef,0x89ab)),"等级修炼原包"): return
	var progress: Dictionary = hud.hud_snapshot().learned_skills[1]
	if not check(progress.level==2 and progress.experience==0x89abcdef and progress.required==300,"等级低字节与32位经验组合"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_MAGIC_LVEXP,5,2,215,0)),"正常修炼值显示"): return
	await settle()
	if not await capture("skill-wire-progress"): return
	if not check(hud.apply_game_message(records_packet(Mir3HudView.SM_ADDMAGIC,[record(261,65)])),"新增高位技能ID"): return
	if not check(hud.hud_snapshot().learned_skills.size()==3 and hud.hud_snapshot().skill_bindings[10]==0,"新增A键标签不按全量规则建立绑定"): return
	var bad := full.slice(0,full.size()-2)
	if not check(not hud.apply_game_message(bad) and hud.hud_snapshot().learned_skills.size()==3,"坏列表不部分替换"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_DELMAGIC,5,0,0,0)) and hud.hud_snapshot().learned_skills.size()==2 and hud.hud_snapshot().skill_bindings[12]==5,"删除保留原版快捷表"): return
	if not check(hud.apply_game_message(records_packet(Mir3HudView.SM_SENDMYMAGIC,[record(1,49),record(5,67)])),"重新全量列表"): return
	await settle()
	if not await capture("skill-wire-list"): return
	if not check(hud.apply_game_message(records_packet(Mir3HudView.SM_SENDMYMAGIC,[])) and hud.hud_snapshot().learned_skills.is_empty() and hud.hud_snapshot().skill_bindings[12]==5,"空全量清列表但不擅改快捷表"): return
	if not check(hud.open_login() and hud.hud_snapshot().learned_skills.is_empty() and hud.hud_snapshot().skill_bindings==PackedInt32Array([0,0,0,0,0,0,0,0,0,0,0,0,0]),"换会话清除技能与快捷表"): return
	print("SKILL_WIRE_UI_OK")
	quit()
