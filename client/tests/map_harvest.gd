# 测试内容：带标记攻击在动作帧3更新同格地表，无标记攻击保持原状。
extends "map_events.gd"

func run() -> void:
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	var origin: Vector2i=hud.hud_snapshot().player_cell
	var cell := origin
	for offset in [Vector2i(-2,1),Vector2i(2,1),Vector2i(-1,1),Vector2i(1,1)]:
		if hud.set_scene_human(5,origin+offset,4,1,"矿工"):
			cell=origin+offset
			break
	if not check(cell!=origin,"可见可走格创建矿工"): return
	if not check(hud.apply_game_message(show_event(700,3,cell)),"创建地表事件"): return
	var attack := game_reply(Mir3HudView.SM_HIT2,5,cell.x,cell.y,4)
	if not check(hud.apply_game_message(attack),"无标记攻击原包"): return
	await create_timer(1.0).timeout
	if not check(hud.hud_snapshot().map_event_frames[700]==230,"普通攻击保持图号"): return
	attack.append(1)
	if not check(hud.apply_game_message(attack) and hud.hud_snapshot().map_event_frames[700]==230,"入队尚未改变地表"): return
	for i in range(90):
		await process_frame
		if hud.hud_snapshot().map_event_frames[700]==231: break
	var state: Dictionary=hud.hud_snapshot()
	if not check(state.map_event_frames[700]==231 and state.map_event_count==2,"动作帧推进地表并触发335"): return
	hud.set_process(false)
	if not await capture("map-harvest-impact"): return
	hud.set_process(true)
	await create_timer(0.8).timeout
	if not check(hud.hud_snapshot().map_event_count==1 and hud.hud_snapshot().map_event_frames[700]==231,"一次动作只推进一次且短效果清理"): return
	hud.queue_free()
	await create_timer(0.1).timeout
	print("MAP_HARVEST_UI_OK")
	quit()
