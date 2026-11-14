# 定向验收：12/637远端蹲姿FIFO、忽略正文、骑乘/尸体差异与本地只消费不打断。
extends "forced_motion.gd"

func wait_remote(action: int) -> bool:
	var deadline := Time.get_ticks_msec()+1800
	while Time.get_ticks_msec()<deadline:
		if hud.hud_snapshot().scene_human_actions[5].x==action: return true
		await process_frame
	return check(false,"等待远端动作"+str(action))

func run() -> void:
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.set_local_actor_identity(77)
	hud.set_character_vitals(100,100,0,0,0,100)
	var origin: Vector2i=hud.hud_snapshot().player_cell
	var target := origin+Vector2i(0,2)
	if not check(hud.set_scene_human(5,origin+Vector2i(0,1),4,0,"采集回包"),"建立远端人物"): return
	hud.apply_game_message(game_reply(Mir3HudView.SM_WALK,5,target.x,target.y,4))
	await settle(0.04)
	var reply := game_reply(Mir3HudView.SM_BUTCH,5,target.x,target.y,2)
	reply.append_array(PackedByteArray([1,2,3,4]))
	if not check(hud.apply_game_message(reply),"637正文不参与解析"): return
	if not check(hud.hud_snapshot().scene_human_movement[5]>=0 and hud.hud_snapshot().scene_human_pending[5]==1,"637等待移动结束"): return
	if not await wait_remote(8): return
	if not check(hud.hud_snapshot().scene_human_cells[5]==target and hud.hud_snapshot().scene_human_facings[5]==2,"637采用包坐标与朝向"): return
	if not check(hud.hud_snapshot().map_event_count==0,"637不虚构武器命中特效"): return
	if not await capture("butch-reply-crouch"): return
	await settle(0.9)
	hud.set_scene_human_mount(5,1)
	hud.apply_game_message(game_reply(Mir3HudView.SM_SITDOWN,5,target.x,target.y,6))
	if not await wait_remote(8): return
	if not check(hud.hud_snapshot().scene_human_mounts[5]==1,"12保留骑乘字段且不拒绝蹲姿消息"): return
	await settle(0.9)
	hud.apply_scene_human_life(5,Mir3HudView.SM_DEATH,target,6)
	var corpse_cell := target+Vector2i(0,1)
	hud.apply_game_message(game_reply(Mir3HudView.SM_BUTCH,5,corpse_cell.x,corpse_cell.y,4))
	await settle(0.04)
	if not check(hud.hud_snapshot().scene_human_life[5].dead and hud.hud_snapshot().scene_human_cells[5]==corpse_cell,"尸体更新锚点但不复活"): return
	hud.walk_to_map_cell(origin+Vector2i(0,1))
	await settle(0.04)
	hud.apply_game_message(game_reply(Mir3HudView.SM_SITDOWN,77,target.x,target.y,2))
	hud.apply_game_message(game_reply(Mir3HudView.SM_BUTCH,77,target.x,target.y,6))
	if not check(hud.hud_snapshot().walk_frame>=0 and hud.hud_snapshot().forced_motion_queue==2,"本地12/637进入FIFO且不打断移动"): return
	await settle(0.8)
	if not check(hud.hud_snapshot().player_cell==origin+Vector2i(0,1) and hud.hud_snapshot().player_action!=8 and hud.hud_snapshot().forced_motion_queue==0,"本地消费后不改坐标、不播放蹲姿"): return
	if not check(hud.hud_snapshot().action_reply_pending,"12/637不代替GOOD确认"): return
	if not check(not hud.apply_game_message(game_reply(Mir3HudView.SM_BUTCH,5,target.x,target.y,8)),"远端非法朝向拒绝"): return
	hud.queue_free()
	await settle()
	print("BUTCH_REPLY_UI_OK")
	quit()
