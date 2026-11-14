# 测试内容：骑乘原包低字节、上马/下马外观及方向、移动中确认与路径清理、命令300ms延迟、右键恢复、技能入口限制、非法头拒绝和离场清理。
extends "login_smoke.gd"

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("map_spawn_cell",Vector2i(413,410))
	hud.set_meta("options_path","user://mount-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	if not check(hud.set_player_body(0,1),"原人物衣服资源"): return
	hud.set_character_identity("骑乘验证","","","")
	await settle()
	var origin: Vector2i = hud.hud_snapshot().player_cell
	var spells: Array[int] = []
	hud.spell_key_requested.connect(func(slot: int, id: int, pointer: Vector2): spells.append(id))
	if not check(hud.set_learned_skill(1,2,1),"绑定技能快捷键"): return
	var motion := InputEventMouseMotion.new()
	motion.position=Vector2(400,200)
	Input.parse_input_event(motion.duplicate())
	key(KEY_F1)
	if not check(spells==[1],"步行技能入口可用"): return
	spells.clear()
	var packets: Array[PackedByteArray] = []
	hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(packet))
	key(KEY_S)
	click_at(Vector2(25,240),true)
	click_at(Vector2(25,240),false)
	if not check(packets.size()==1 and hud.hud_snapshot().map_left_delay_ms==300 and hud.hud_snapshot().player_mount==0,"骑马命令启动延迟且不提前上马"): return
	local_movement_click(Vector2i(1,0),true)
	local_movement_click(Vector2i(1,0),false)
	if not check(hud.hud_snapshot().walk_frame==-1,"300ms内右键不启动移动"): return
	key(KEY_S)
	if not check(not hud.hud_snapshot().mount_visible,"延迟保留窗口快捷键"): return
	Input.parse_input_event(motion.duplicate())
	key(KEY_F1)
	if not check(spells.is_empty(),"动作延迟阻止技能请求"): return
	var packet := game_reply(Mir3HudView.SM_RIDEHORSE,0xabcdef01,65535,65535,65535)
	if not check(hud.apply_game_message(packet),"骑乘原包忽略无关参数"): return
	var state: Dictionary = hud.hud_snapshot()
	if not check(state.mount_shape==1 and state.player_mount==1 and state.player_idle_frame==0 and state.walk_frame==-1,"按钮及人物同步坐骑并从待机首帧开始"): return
	await settle(0.35)
	if not check(hud.hud_snapshot().map_left_delay_ms==0,"命令延迟期满"): return
	key(KEY_F1)
	if not check(spells.is_empty(),"骑乘时不释放技能"): return
	if not await capture("mount-wire-mounted"): return
	local_movement_click(Vector2i(1,0),true)
	local_movement_click(Vector2i(1,0),false)
	if not check(hud.hud_snapshot().walk_frame>=0,"期满右键恢复移动"): return
	var before: Dictionary = hud.hud_snapshot()
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_RIDEHORSE,0x12340000,1,2,3)),"移动中下马回包"): return
	state=hud.hud_snapshot()
	if not check(state.player_mount==0 and state.mount_shape==0 and state.walk_frame==-1 and state.player_cell==before.player_cell and state.player_facing==before.player_facing and state.route_destination==Vector2i(-1,-1),"下马保留当前格与方向并清移动路径"): return
	Input.parse_input_event(motion.duplicate())
	key(KEY_F1)
	if not check(spells==[1],"下马后技能入口恢复"): return
	if not check(not hud.apply_game_message(packet.slice(0,15)) and hud.hud_snapshot().player_mount==0,"截断头不改状态"): return
	if not check(hud.walk_to_map_cell(state.player_cell+Vector2i(0,1)),"建立待处理路径"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_RIDEHORSE,3,0,0,0)) and hud.hud_snapshot().route_destination==Vector2i(-1,-1),"确认清理尚未开始的路径"): return
	await settle()
	if not await capture("mount-wire-third-horse"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_RIDEHORSE,0,0,0,0)),"恢复步行"): return
	await settle()
	if not await capture("mount-wire-dismounted"): return
	if not check(hud.open_login() and hud.hud_snapshot().mount_shape==0 and hud.hud_snapshot().player_mount==0 and hud.hud_snapshot().map_left_delay_ms==0,"离场清除骑乘和动作延迟"): return
	print("MOUNT_WIRE_UI_OK")
	quit()
