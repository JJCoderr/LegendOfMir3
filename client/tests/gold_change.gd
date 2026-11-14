# 测试内容：金币增减和重复回包、原提示颜色、106音效触发、截断拒绝、全DWORD余额、背包/交易同步及升级提示共用颜色。
extends "login_smoke.gd"

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("options_path","user://ability-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	hud.set_character_identity("金币验证","","","")
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_GOLDCHANGED,100,0,0,0)),"初始余额"): return
	var state: Dictionary = hud.hud_snapshot()
	var before: int = state.chat_compact_count
	var sounds: int = state.gold_sound_count
	var update := game_reply(Mir3HudView.SM_GOLDCHANGED,350,65535,65535,65535)
	if not check(hud.apply_game_message(update),"金币增加"): return
	state=hud.hud_snapshot()
	if not check(state.inventory_gold==350 and state.chat_compact_count==before+1 and "250" in state.chat_last_text and state.chat_last_foreground==0xffffff and state.chat_last_background==33023 and state.gold_sound_count==sounds+1,"余额提示及非位置音效"): return
	if not check(hud.apply_game_message(update) and hud.hud_snapshot().chat_compact_count==before+1 and hud.hud_snapshot().gold_sound_count==sounds+2,"重复回包播放音效但无重复收入提示"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_GOLDCHANGED,300,0,0,0)) and hud.hud_snapshot().inventory_gold==300 and hud.hud_snapshot().chat_compact_count==before+1,"支出不显示收入"): return
	if not check(not hud.apply_game_message(update.slice(0,15)) and hud.hud_snapshot().inventory_gold==300 and hud.hud_snapshot().gold_sound_count==sounds+3,"截断包不更新或播放"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_GOLDCHANGED,0xffffffff,0,0,0)) and hud.hud_snapshot().inventory_gold==0xffffffff,"余额不受人物ID上限影响"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_GOLDCHANGED,0,0,0,0)) and "1" in hud.hud_snapshot().chat_last_text,"原版差值回绕"): return
	if not check(hud.open_trade("比奇旅人"),"打开交易和背包"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_GOLDCHANGED,123456,0,0,0)),"窗口打开期间更新余额"): return
	state=hud.hud_snapshot()
	if not check(state.inventory_gold==123456 and state.inventory_visible and state.trade_visible,"交易背包共用余额"): return
	await settle()
	if not await capture("gold-change-trade"): return
	if not check(hud.apply_trade_cancel(),"关闭交易"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_LEVELUP,0,1,0,0)),"共用升级提示"): return
	state=hud.hud_snapshot()
	if not check(state.chat_last_foreground==0xffffff and state.chat_last_background==33023,"升级也使用41C120实际颜色"): return
	print("GOLD_CHANGE_UI_OK")
	quit()
