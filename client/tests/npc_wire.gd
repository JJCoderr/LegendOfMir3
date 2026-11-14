# 测试内容：NPC固定原包向量的四字节密钥、对话识别、非法头拒绝及关闭回包。
extends "hud_smoke.gd"

func run() -> void:
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("options_path","user://npc-wire-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	var keys := PackedByteArray([17,34,51,68])
	var packet := "`\\<<<=IB<<<<<<<<lHzwm^{NfhGniZKFhhJpdwyXKGbvlkGNfXVsGp=^YS`zS?o<vwzfGp=aZBapK\\".to_ascii_buffer()
	if not check(not hud.apply_npc_message(packet.slice(0,15),keys),"截断消息头不改变对话"): return
	if not check(hud.apply_npc_message(packet,keys) and hud.hud_snapshot().npc_dialogue_actor==2,"固定原包按正确密钥还原NPC身份"): return
	var close := scene_packet(Mir3HudView.SM_MERCHANTDLGCLOSE,2,0,0,0)
	if not check(hud.apply_npc_message(close,PackedByteArray([18,52,86,120])) and hud.hud_snapshot().npc_dialogue_actor==0,"共享场景头关闭NPC对话"): return
	print("NPC_WIRE_OK")
	quit()
