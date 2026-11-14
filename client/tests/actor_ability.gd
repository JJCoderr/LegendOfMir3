# 测试内容：人物能力原包更新底栏、生命条、状态页元素与金币；重复更新、非法包原子拒绝、换角色清理和数值截图。
extends "login_smoke.gd"

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("options_path","user://ability-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	hud.set_character_identity("属性验证","","","")
	var packet := game_reply(Mir3HudView.SM_ABILITY,0xfedcba98,2,0,0)
	packet.append_array(PackedByteArray([70, 108, 68, 67, 60, 108, 92, 70, 67, 76, 80, 77, 61, 109, 89, 90, 60, 83, 92, 60, 121, 60, 67, 108, 60, 72, 63, 80, 60, 76, 62, 60, 66, 92, 84, 60, 85, 60, 63, 118, 60, 60, 120, 90, 61, 108, 108, 61, 60, 108, 60, 60, 60, 92, 80, 63, 92, 60, 79, 123, 61, 76, 96, 66, 62, 92, 64, 60, 91, 116, 63, 123, 60, 108, 76, 62, 60, 67, 118, 61, 123, 92, 80, 66]))
	if not check(hud.apply_game_message(packet),"能力消息接入游戏分发"): return
	var state: Dictionary = hud.hud_snapshot()
	if not check(state.inventory_gold==0xfedcba98 and state.character_level==43 and state.character_hp==Vector2i(350,500) and state.character_mp==Vector2i(120,240),"金币和生命魔法同步"): return
	if not check(state.character_experience==120000 and state.character_next_experience==400000 and state.character_weight==Vector2i(100,250),"经验重量同步"): return
	if not check(state.scene_health_values[0]==PackedInt32Array([350,120,500]),"本地人物生命条同步"): return
	if not check(not hud.apply_game_message(packet.slice(0,-1)) and hud.hud_snapshot().character_hp==state.character_hp,"截断包不改变已显示值"): return
	if not check(hud.apply_game_message(packet),"重复能力包保持幂等"): return
	await settle()
	if not await capture("actor-ability-hud"): return
	key(KEY_Q)
	await settle()
	if not check(hud.hud_snapshot().status_visible,"状态窗口已打开"): return
	if not await capture("actor-ability-status"): return
	if not check(hud.hud_snapshot().scene_damage_frames.is_empty(),"属性更新不产生伤害跳字"): return
	key(KEY_Q)
	key(KEY_X,0,true)
	key(KEY_ENTER)
	var deadline := Time.get_ticks_msec()+4000
	while hud.hud_snapshot().client_page==3 and Time.get_ticks_msec()<deadline:
		await process_frame
	if not check(hud.set_selection_session("account",-123) and hud.request_selection_list(),"进入换角色场景"): return
	if not check(hud.apply_selection_message(selection_packet(1,"Hero/2/x/10/0")),"加载新角色列表"): return
	await settle(0.55)
	if not check(hud.apply_selection_message(plain_reply(Mir3HudView.SM_STARTPLAY,0,"127.0.0.1/7200/")),"新角色开始游戏"): return
	await settle()
	state=hud.hud_snapshot()
	if not check(state.client_page==3 and state.inventory_gold==0 and state.character_hp==Vector2i.ZERO and state.character_experience==0 and state.character_weight==Vector2i.ZERO and state.scene_health_values.is_empty(),"新角色入场清除旧属性"): return
	if not check(hud.apply_game_message(packet) and hud.hud_snapshot().character_hp==Vector2i(350,500),"入场后重新接收能力值"): return
	print("ACTOR_ABILITY_UI_OK")
	quit()
