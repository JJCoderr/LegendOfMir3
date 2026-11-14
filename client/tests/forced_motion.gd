# 定向验收：四种原包排队等待、冲刺/碰撞折返/后退/强制骑乘走步、参数拒绝及无本地反向发包。
extends "melee_input.gd"

func motion_reply(opcode: int, actor: int, cell: Vector2i, facing: int, mount: int=0) -> PackedByteArray:
	var body := PackedByteArray()
	body.resize(16)
	var extra := PackedByteArray([mount,0,0,0,0])
	var packet := game_reply(opcode,actor,cell.x,cell.y,facing|(44<<8))
	packet.append_array(encode_scene_bytes(body))
	packet.append_array(encode_scene_bytes(extra))
	return packet

func run() -> void:
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.set_local_actor_identity(77)
	hud.set_character_vitals(100,100,0,0,0,100)
	var origin: Vector2i=hud.hud_snapshot().player_cell
	var packets: Array[PackedByteArray]=[]
	hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(packet))
	var pointer := Vector2(origin*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(49,1)
	attack_click(pointer,true)
	if not check(packets.size()==1,"先发起本地攻击"): return
	if not check(not hud.apply_game_message(motion_reply(Mir3HudView.SM_RUSH,77,origin,8)),"拒绝非法方向"): return
	if not check(not hud.apply_game_message(motion_reply(Mir3HudView.SM_RUSH,77,Vector2i(65535,65535),4)),"拒绝越界落点"): return
	var cases := [
		motion_reply(Mir3HudView.SM_RUSH,77,origin+Vector2i(0,2),4),
		motion_reply(Mir3HudView.SM_BUMP,77,origin+Vector2i(1,2),2),
		motion_reply(Mir3HudView.SM_BACKSTEP,77,origin+Vector2i(0,1),4),
		motion_reply(Mir3HudView.SM_REVIVE_REFRESH,999,origin+Vector2i(0,2),4,1)]
	for packet in cases:
		if not check(hud.apply_game_message(packet),"完整服务器位移原包入队"): return
	if not check(hud.hud_snapshot().forced_motion_queue==4 and hud.hud_snapshot().walk_frame==-1
		and hud.hud_snapshot().player_action==9,"服务器位移等待当前挥击结束"): return
	var seen := {}
	var saw_return := false
	var deadline := Time.get_ticks_msec()+4000
	while Time.get_ticks_msec()<deadline:
		await process_frame
		var state: Dictionary=hud.hud_snapshot()
		var opcode: int=state.forced_motion_opcode
		if opcode!=0:
			seen[opcode]=true
		if opcode==Mir3HudView.SM_RUSH and state.walk_frame==2 and not seen.has("rush-shot"):
			seen["rush-shot"]=true
			if not await capture("forced-motion-rush"): return
		if opcode==Mir3HudView.SM_BACKSTEP and state.walk_frame==2 and not seen.has("back-shot"):
			seen["back-shot"]=true
			if not check(state.player_facing==4 and state.map_step_destination==origin+Vector2i(0,1),"后退保留南向而向北移动"): return
			if not await capture("forced-motion-backstep"): return
		if opcode==0 and state.player_action==7:
			saw_return=state.player_cell==origin+Vector2i(0,2)
		if seen.has(Mir3HudView.SM_REVIVE_REFRESH) and opcode==0 and state.forced_motion_queue==0:
			break
	var state: Dictionary=hud.hud_snapshot()
	if not check(seen.has(Mir3HudView.SM_RUSH) and seen.has(Mir3HudView.SM_BUMP)
		and seen.has(Mir3HudView.SM_BACKSTEP) and seen.has(Mir3HudView.SM_REVIVE_REFRESH)
		and saw_return,"依次执行四类位移且碰撞退回推导起点"): return
	if not check(state.player_cell==origin+Vector2i(0,2) and state.player_mount==1
		and not state.action_reply_pending and packets.size()==1,"强制骑乘走步落点正确、解除确认且不反发动作请求"): return
	if not await capture("forced-motion-arrival"): return
	hud.queue_free()
	await settle()
	print("FORCED_MOTION_UI_OK")
	quit()
