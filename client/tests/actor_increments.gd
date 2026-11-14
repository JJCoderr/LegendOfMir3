# 测试内容：负重增量保留上限，附加能力百分比同步、完整能力包互不覆盖、截断拒绝、零值更新及换角色清理。
extends "login_smoke.gd"

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("options_path","user://ability-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	hud.set_character_identity("增量验证","","","")
	var packet := game_reply(Mir3HudView.SM_ABILITY,0xfedcba98,2,0,0)
	packet.append_array(PackedByteArray([70, 108, 68, 67, 60, 108, 92, 70, 67, 76, 80, 77, 61, 109, 89, 90, 60, 83, 92, 60, 121, 60, 67, 108, 60, 72, 63, 80, 60, 76, 62, 60, 66, 92, 84, 60, 85, 60, 63, 118, 60, 60, 120, 90, 61, 108, 108, 61, 60, 108, 60, 60, 60, 92, 80, 63, 92, 60, 79, 123, 61, 76, 96, 66, 62, 92, 64, 60, 91, 116, 63, 123, 60, 108, 76, 62, 60, 67, 118, 61, 123, 92, 80, 66]))
	if not check(hud.apply_game_message(packet),"能力消息接入游戏分发"): return
	var sub := game_reply(Mir3HudView.SM_SUBABILITY,0xabcdefab,0x8007,0xfe81,0x09ff)
	var weight := game_reply(Mir3HudView.SM_WEIGHTCHANGED,0xabcd00fa,0xff19,0xff09,0xffff)
	if not check(hud.apply_game_message(sub) and hud.apply_game_message(weight),"接收附加能力和负重"): return
	var state: Dictionary = hud.hud_snapshot()
	if not check(state.character_weight==Vector2i(250,250) and state.character_wear_weight==Vector2i(25,30) and state.character_hand_weight==Vector2i(9,12),"低位当前负重同步并保留上限"): return
	if not check(state.character_bonuses==Vector2i(7,128) and state.character_hp==Vector2i(350,500) and state.inventory_gold==0xfedcba98,"附加能力不改生命金币"): return
	if not check(not hud.apply_game_message(sub.slice(0,15)) and hud.hud_snapshot().character_bonuses==Vector2i(7,128),"截断头原子拒绝"): return
	if not check(hud.apply_game_message(packet) and hud.hud_snapshot().character_bonuses==Vector2i(7,128),"完整能力消息保留附加能力"): return
	if not check(hud.apply_game_message(weight),"完整能力后再次更新负重"): return
	key(KEY_Q)
	await settle()
	if not check(hud.hud_snapshot().status_visible,"打开状态窗口"): return
	if not await capture("actor-increments-status"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_SUBABILITY,0,0,0,0)) and hud.apply_game_message(game_reply(Mir3HudView.SM_WEIGHTCHANGED,0,0,0,0)),"接收清零增量"): return
	state=hud.hud_snapshot()
	if not check(state.character_bonuses==Vector2i.ZERO and state.character_weight==Vector2i(0,250) and state.character_wear_weight==Vector2i(0,30),"清零不清负重上限"): return
	if not check(hud.apply_game_message(sub),"重新设置附加能力"): return
	key(KEY_Q)
	key(KEY_X,0,true)
	key(KEY_ENTER)
	var deadline := Time.get_ticks_msec()+4000
	while hud.hud_snapshot().client_page==3 and Time.get_ticks_msec()<deadline:
		await process_frame
	if not check(hud.set_selection_session("account",-123) and hud.request_selection_list(),"换角色"): return
	if not check(hud.apply_selection_message(selection_packet(1,"Hero/2/x/10/0")),"加载角色"): return
	await settle(0.55)
	if not check(hud.apply_selection_message(plain_reply(Mir3HudView.SM_STARTPLAY,0,"127.0.0.1/7200/")),"新角色入场"): return
	await settle()
	if not check(hud.hud_snapshot().character_bonuses==Vector2i.ZERO,"新角色清除旧附加能力"): return
	print("ACTOR_INCREMENTS_UI_OK")
	quit()
