# 测试内容：经验总量及比例、原资源聊天提示和颜色、附加提示、武器修炼累加/覆盖/零值、截断拒绝与换角色清理。
extends "login_smoke.gd"

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("options_path","user://ability-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	hud.set_character_identity("经验验证","","","")
	var packet := game_reply(Mir3HudView.SM_ABILITY,0xfedcba98,2,0,0)
	packet.append_array(PackedByteArray([70, 108, 68, 67, 60, 108, 92, 70, 67, 76, 80, 77, 61, 109, 89, 90, 60, 83, 92, 60, 121, 60, 67, 108, 60, 72, 63, 80, 60, 76, 62, 60, 66, 92, 84, 60, 85, 60, 63, 118, 60, 60, 120, 90, 61, 108, 108, 61, 60, 108, 60, 60, 60, 92, 80, 63, 92, 60, 79, 123, 61, 76, 96, 66, 62, 92, 64, 60, 91, 116, 63, 123, 60, 108, 76, 62, 60, 67, 118, 61, 123, 92, 80, 66]))
	if not check(hud.apply_game_message(packet),"能力消息接入游戏分发"): return
	var before: int = hud.hud_snapshot().chat_compact_count
	var gained := game_reply(Mir3HudView.SM_WINEXP,160000,40000,0,0)
	if not check(hud.apply_game_message(gained),"获得经验消息"): return
	var state: Dictionary = hud.hud_snapshot()
	if not check(state.character_experience==160000 and state.character_next_experience==400000 and state.character_level==43,"更新总经验并保留等级上限"): return
	if not check(state.chat_compact_count==before+1 and "40000" in state.chat_last_text and "%d" not in state.chat_last_text and state.chat_last_foreground==0xffffff and state.chat_last_background==255,"原资源提示与原版颜色"): return
	if not check(not hud.apply_game_message(gained.slice(0,15)) and hud.hud_snapshot().chat_compact_count==before+1,"截断消息不更新也不追加提示"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_WINEXP,180000,20000,7,0xff00)),"经验与武器修炼及附加通知"): return
	state=hud.hud_snapshot()
	if not check(state.weapon_training==7 and state.chat_compact_count==before+3 and "20000" in state.chat_last_text and "7" in state.chat_last_text,"附加提示和正文按序各一次"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_WINEXP,190000,10000,3,0)) and hud.hud_snapshot().weapon_training==10,"修炼累加"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_WINEXP,200000,10000,2,1)) and hud.hud_snapshot().weapon_training==2,"修炼覆盖"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_WINEXP,200000,0,0,1)) and hud.hud_snapshot().weapon_training==2,"零修炼忽略覆盖标志"): return
	key(KEY_Q)
	await settle()
	if not await capture("actor-experience-status"): return
	if not check(hud.apply_game_message(packet) and hud.hud_snapshot().weapon_training==2,"能力包不覆盖武器修炼"): return
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
	if not check(hud.hud_snapshot().weapon_training==0 and hud.hud_snapshot().character_experience==0,"新角色清除旧经验和修炼"): return
	print("ACTOR_EXPERIENCE_UI_OK")
	quit()
