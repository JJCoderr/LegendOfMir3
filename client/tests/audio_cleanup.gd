# 定向验收：脚步/挥击声音仍在播放时退出场景，检查节点、音频流、播放实例及临时总线释放。
extends "melee_input.gd"

func watch_audio(node: Node, refs: Array[WeakRef]) -> int:
	var playing := 0
	if node is AudioStreamPlayer:
		if node.playing:
			playing+=1
			refs.append(weakref(node.get_stream_playback()))
		if node.stream!=null:
			refs.append(weakref(node.stream))
	for child in node.get_children():
		playing+=watch_audio(child,refs)
	return playing

func run() -> void:
	var buses := AudioServer.bus_count
	for kind in ["footstep","swing"]:
		hud=load("res://main.tscn").instantiate()
		hud.set_meta("offline_preview",true)
		hud.set_connection_enabled(false)
		root.add_child(hud)
		await settle()
		hud.set_character_vitals(100,100,0,0,0,100)
		var origin: Vector2i=hud.hud_snapshot().player_cell
		if kind=="footstep":
			hud.walk_to_map_cell(origin+Vector2i(0,1))
		else:
			var pointer := Vector2(origin*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(49,1)
			attack_click(pointer,true)
		var deadline := Time.get_ticks_msec()+1500
		while Time.get_ticks_msec()<deadline:
			await process_frame
			var state: Dictionary=hud.hud_snapshot()
			if (kind=="footstep" and state.last_movement_sound!=0) or (kind=="swing" and state.scene_actor_sounds.has(0)):
				break
		var refs: Array[WeakRef]=[]
		if not check(watch_audio(hud,refs)>0,"退出前确有声音播放："+kind): return
		if kind=="swing":
			if not await capture("audio-cleanup-active"): return
		var owner_ref: WeakRef = weakref(hud)
		hud.queue_free()
		deadline=Time.get_ticks_msec()+1000
		while Time.get_ticks_msec()<deadline:
			await process_frame
			if owner_ref.get_ref()==null and refs.all(func(ref: WeakRef): return ref.get_ref()==null): break
		if not check(owner_ref.get_ref()==null and refs.all(func(ref: WeakRef): return ref.get_ref()==null),
			"场景和音频流/播放实例全部释放："+kind): return
		if not check(AudioServer.bus_count==buses,"临时音频总线全部移除："+kind): return
	print("AUDIO_CLEANUP_UI_OK")
	quit()
