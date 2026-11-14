# 定向验收：攻击后移动等待、等待期间画面继续刷新、目的格锁定、移动发包时间和换图取消。
extends "melee_input.gd"

func packet_tick(packet: PackedByteArray) -> int:
	return packet.decode_u16(6)|(packet.decode_u16(10)<<16)

func run() -> void:
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	# 本用例只检查动作本身；模拟服务器逐次确认。
	hud.client_packet_ready.connect(func(_packet: PackedByteArray): hud.apply_game_message("+GOOD".to_ascii_buffer()))
	await settle()
	hud.set_local_actor_identity(77)
	hud.set_character_vitals(100,100,0,0,0,100)
	hud.set_character_progress(1,0,0,0,100)
	hud.set_character_load_bonuses(Vector2i.ZERO,Vector2i(1,1),Vector2i.ZERO)
	var origin: Vector2i=hud.hud_snapshot().player_cell
	var packets: Array[PackedByteArray]=[]
	var sent_at: Array[int]=[]
	hud.client_packet_ready.connect(func(packet: PackedByteArray):
		packets.append(decode_request(packet))
		sent_at.append(hud.hud_snapshot().exit_clock))
	var pointer := Vector2(origin*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(1,33)
	attack_click(pointer,true)
	if not check(packets.size()==1 and packets[0].decode_u16(4)==Mir3HudView.CM_HIT,"强攻发包"): return
	await settle(0.7)
	if not check(hud.walk_to_map_cell(origin+Vector2i(0,1)),"请求相邻格移动"): return
	await settle(0.05)
	if not check(hud.hud_snapshot().local_action_pending and hud.hud_snapshot().walk_frame==-1
		and packets.size()==1,"攻击动画完成但共用等待未结束"): return
	if not check(not hud.walk_to_map_cell(origin+Vector2i(1,0)),"等待请求不被后续目的格覆盖"): return
	var rendered_frames := 0
	var deadline := Time.get_ticks_msec()+1500
	while packets.size()<2 and Time.get_ticks_msec()<deadline:
		await process_frame
		rendered_frames+=1
	if not check(packets.size()==2 and rendered_frames>1,"等待期间继续绘制且只执行一次"): return
	var move := packets[1]
	if not check(move.decode_u16(4)==Mir3HudView.CM_WALK and move.decode_u16(0)==origin.x
		and move.decode_u16(2)==origin.y+1 and move.decode_u16(8)==4,"移动包携带固定目的格和朝向"): return
	var actual_delay := (sent_at[1]-packet_tick(packets[0]))&0xffffffff
	if not check(actual_delay>=1109 and actual_delay<1600,"移动遵守攻击间隔80%的边界"): return
	if not check(((sent_at[1]-packet_tick(move))&0xffffffff)>=150,"移动包保留准入时间而非最终发送时间"): return
	await settle(0.12)
	if not await capture("local-action-walk"): return
	await settle(0.65)
	if not check(hud.hud_snapshot().player_cell==origin+Vector2i(0,1)
		and packets.size()==2,"到达目的格且不重复发包"): return
	# 同一可走位置按住右键：第一步走，之后跑，核验两个请求使用不同命令。
	var run := InputEventMouseButton.new()
	# 原始0.map在(475,361)向北第三格不可通行；南侧连续五格可走。
	run.position=Vector2(390,400)
	run.button_index=MOUSE_BUTTON_RIGHT
	run.pressed=true
	root.push_input(run,true)
	if not check(packets.size()==3 and packets.back().decode_u16(4)==Mir3HudView.CM_WALK,"右键首步发送走请求"): return
	deadline=Time.get_ticks_msec()+1800
	while not hud.hud_snapshot().map_running and Time.get_ticks_msec()<deadline:
		await process_frame
	run=run.duplicate()
	run.pressed=false
	root.push_input(run,true)
	if not check(hud.hud_snapshot().map_running and packets.size()==4
		and packets.back().decode_u16(4)==Mir3HudView.CM_RUN,"持续右键发送跑请求：cell=%s, frame=%s, pending=%s, packets=%s" % [
		hud.hud_snapshot().player_cell,hud.hud_snapshot().walk_frame,hud.hud_snapshot().local_action_pending,
		packets.map(func(p: PackedByteArray): return [p.decode_u16(4),p.decode_u16(0),p.decode_u16(2)])]): return
	if not check(((packet_tick(packets[3])-packet_tick(packets[2]))&0xffffffff)>=600,"走跑准入至少相隔600毫秒"): return
	if not await capture("local-action-run"): return
	await settle(0.7)
	# 新一轮攻击后超重，使移动等待足够长，验证换图清理已接受动作。
	var cell: Vector2i=hud.hud_snapshot().player_cell
	pointer=Vector2(cell*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(1,33)
	attack_click(pointer,true)
	if not check(packets.size()==5,"第二轮攻击"): return
	hud.set_character_load_bonuses(Vector2i.ZERO,Vector2i(2,1),Vector2i.ZERO)
	await settle(0.7)
	if not check(hud.walk_to_map_cell(origin),"请求返程"): return
	await settle(0.05)
	if not check(hud.hud_snapshot().local_action_pending,"超重使移动进入共用等待"): return
	if not check(hud.change_map("0","比奇省",origin),"换图取消旧动作"): return
	if not check(not hud.hud_snapshot().local_action_pending,"换图后没有遗留请求"): return
	await settle(0.1)
	if not check(packets.size()==5,"换图未发送旧移动包"): return
	hud.queue_free()
	await settle()
	print("LOCAL_ACTION_CLOCK_UI_OK")
	quit()
