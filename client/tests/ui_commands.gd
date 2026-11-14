# 测试内容：系统选项、聊天接收开关和骑马按钮命令的完整GBK原包、共享序号、重复请求及确认状态，验证按下/释放和原文空格。
extends "login_smoke.gd"

const GBK := {
	"@拒绝回生术": [64, 190, 220, 190, 248, 187, 216, 201, 250, 202, 245],
	"@拒绝天地合一": [64, 190, 220, 190, 248, 204, 236, 181, 216, 186, 207, 210, 187],
	"@拒绝交易": [64, 190, 220, 190, 248, 189, 187, 210, 215],
	"@拒绝私聊": [64, 190, 220, 190, 248, 203, 189, 193, 196],
	"@拒绝行会聊天": [64, 190, 220, 190, 248, 208, 208, 187, 225, 193, 196, 204, 236],
	"@加入行会": [64, 188, 211, 200, 235, 208, 208, 187, 225],
	"@拒绝所有人聊天": [64, 190, 220, 190, 248, 203, 249, 211, 208, 200, 203, 193, 196, 204, 236],
	"@攻击方式": [64, 185, 165, 187, 247, 183, 189, 202, 189],
	"@拒绝悄悄话": [64, 190, 220, 190, 248, 199, 196, 199, 196, 187, 176],
	"@拒绝所有人聊天 ": [64, 190, 220, 190, 248, 203, 249, 211, 208, 200, 203, 193, 196, 204, 236, 32],
	"@骑马": [64, 198, 239, 194, 237],
	"@下马": [64, 207, 194, 194, 237],
	"@包马": [64, 176, 252, 194, 237],
}

var packets: Array[PackedByteArray] = []
var commands: Array[String] = []
func click(pos: Vector2) -> void:
	click_at(pos,true)
	click_at(pos,false)
func expect(command: String) -> bool:
	var expected := ("#"+str((packets.size()-1)%9+1)).to_ascii_buffer()
	expected.append_array(encode_scene_bytes(PackedByteArray([0,0,0,0,Mir3HudView.CM_SAY&255,Mir3HudView.CM_SAY>>8,0,0,0,0,0,0])))
	expected.append_array(encode_scene_bytes(PackedByteArray(GBK[command])))
	expected.append(33)
	return check(not packets.is_empty() and packets[-1]==expected and commands[-1]==command and commands.size()==packets.size(),"命令原文、GBK正文及共享序号："+command)

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("options_path","user://commands-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(packet))
	hud.chat_requested.connect(func(command: String): commands.append(command))
	key(KEY_N)
	var option_commands := ["@拒绝回生术","@拒绝天地合一","@拒绝交易","@拒绝私聊","@拒绝行会聊天","@加入行会","@拒绝所有人聊天"]
	for i in range(7):
		var pos := Vector2(336,210+30*i) if i<6 else Vector2(580,90)
		var before := packets.size()
		click_at(pos,true)
		if not check(packets.size()==before,"选项按下不发包"): return
		click_at(pos,false)
		if not expect(option_commands[i]): return
		if not check(hud.hud_snapshot().option_confirmed[i]==0 and hud.hud_snapshot().option_displayed[i]==1,"请求仅改变预测"): return
	click(Vector2(336,210))
	if not expect(option_commands[0]): return
	if not check(hud.hud_snapshot().option_displayed[0]==1,"重复请求不反转确认状态"): return
	click(Vector2(580,300))
	if not expect("@攻击方式"): return
	if not check(hud.hud_snapshot().option_attack_mode==0,"攻击模式等待确认"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CLIENT_OPTIONS,0xcf800000,4,0,0)),"选项原包确认"): return
	if not check(hud.hud_snapshot().option_confirmed==PackedByteArray([1,1,1,1,1,1,1]) and hud.hud_snapshot().option_attack_mode==4,"确认原包更新状态"): return
	await settle()
	if not await capture("ui-commands-options"): return
	key(KEY_N)
	click_at(Vector2(620,473),false)
	var reception := ["@拒绝悄悄话","@拒绝行会聊天","@拒绝所有人聊天 "]
	for i in range(3):
		click(Vector2(350+40*i,560))
		if not expect(reception[i]): return
	await settle()
	if not await capture("ui-commands-reception"): return
	key(KEY_S)
	var before := packets.size()
	click(Vector2(85,240))
	if not check(packets.size()==before,"步行时下马不发送"): return
	click(Vector2(25,240))
	if not expect("@骑马"): return
	await settle(0.31)
	if not check(hud.hud_snapshot().mount_shape==0,"骑马请求等待确认"): return
	if not check(hud.apply_mount_reply(Mir3HudView.SM_RIDEHORSE,2),"已解码骑乘确认入口"): return
	before=packets.size()
	click(Vector2(25,240))
	if not check(packets.size()==before,"骑乘时上马不发送"): return
	for i in range(3):
		click(Vector2(85+57*i,240))
		if not expect(["@下马","@包马","@下马"][i]): return
		await settle(0.31)
	if not check(hud.hud_snapshot().mount_shape==2,"请求不能伪造下马结果"): return
	await settle()
	if not await capture("ui-commands-mount"): return
	print("UI_COMMANDS_OK")
	quit()
