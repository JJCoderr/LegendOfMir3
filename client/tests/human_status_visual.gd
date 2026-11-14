# 定向验收：人物所有图层状态颜色、优先级、受击保留状态及外观/死亡状态更新。
extends "local_struck.gd"

func status(actor: int, flags: int) -> void:
	hud.apply_game_message(game_reply(Mir3HudView.SM_ACTOR_STATUS,actor,flags&65535,(flags>>16)&65535,0))
	await settle(0.035)

func green_pixels(bounds: Rect2) -> int:
	await RenderingServer.frame_post_draw
	var image := root.get_texture().get_image()
	var count := 0
	for y in range(maxi(0,int(bounds.position.y)),mini(530,int(bounds.end.y))):
		for x in range(maxi(0,int(bounds.position.x)),mini(800,int(bounds.end.x))):
			var color := image.get_pixel(x,y)
			if color.g>0.15 and color.r<0.035 and color.b<0.035: count+=1
	return count

func run() -> void:
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.set_local_actor_identity(77)
	hud.set_character_vitals(100,100,0,0,0,100)
	hud.set_player_mount(1)
	hud.set_player_attachments(1,1)
	var origin: Vector2i=hud.hud_snapshot().player_cell
	hud.set_scene_human(5,origin+Vector2i(0,2),4,0,"状态颜色")
	hud.set_scene_human_attachments(5,1,1)
	await status(77,0x80000000)
	await status(5,0x80000000)
	if not check(await green_pixels(hud.hud_snapshot().player_bounds)>40,"本地骑乘身体实际显示绿色状态"): return
	if not check(await green_pixels(hud.hud_snapshot().scene_human_bounds[5])>20,"远端人物实际显示绿色状态"): return
	if not await capture("human-status-green"): return
	var masks := [0x00800000,0x04000000,0x20000000,0x08000000,0x40000000,0x80000000,0x00020000]
	var colors := [1,0,64511,38079,64690,2016,1]
	var combined := 0
	for i in masks.size():
		combined|=masks[i]
		await status(77,combined)
		if not check(hud.hud_snapshot().player_status_color==colors[i],"状态叠加按原版优先级选择"+str(i)): return
	if not await capture("human-status-translucent"): return
	await status(77,0x80000000)
	hud.apply_game_message(hit_reply(Mir3HudView.SM_STRUCK,80,20))
	await settle(0.04)
	if not check(hud.hud_snapshot().player_status_color==2016,"本地受击外观不清除已有状态"): return
	var hit := hit_reply(Mir3HudView.SM_HUMAN_STRUCK,80,20)
	# 重编码远端身份，正文仍为零状态位。
	var remote_hit := game_reply(Mir3HudView.SM_HUMAN_STRUCK,5,80,100,20)
	remote_hit.append_array(hit.slice(16))
	hud.apply_game_message(remote_hit)
	await settle(0.04)
	if not check(hud.hud_snapshot().scene_human_status_colors[5]==2016,"远端受击外观不清除已有状态"): return
	await settle(0.65)
	var feature := PackedByteArray()
	feature.resize(21)
	feature.encode_u32(4,0x04000002)
	hud.queue_scene_human_state(5,Mir3HudView.SM_TURN,origin+Vector2i(0,2),4,feature)
	await settle(0.04)
	if not check(hud.hud_snapshot().scene_human_status_colors[5]==0,"站立外观包更新状态颜色"): return
	var death := death_reply(Mir3HudView.SM_DEATH,origin)
	var death_feature := PackedByteArray()
	death_feature.resize(16)
	death_feature.encode_u32(4,0x40000000)
	death=death.slice(0,16)
	death.append_array(encode_scene_bytes(death_feature))
	hud.apply_game_message(death)
	await settle(0.04)
	if not check(hud.hud_snapshot().player_dead and hud.hud_snapshot().player_status_color==64690,"死亡外观包刷新尸体状态"): return
	if not await capture("human-status-corpse"): return
	hud.queue_free()
	await settle()
	print("HUMAN_STATUS_VISUAL_UI_OK")
	quit()
