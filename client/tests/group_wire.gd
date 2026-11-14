# 测试内容：组队允许开关、创建及增删成员请求、失败提示、成员列表替换、坏包保留及取消组队，验证实际视口。
extends "login_smoke.gd"

func roster(body: String) -> PackedByteArray:
	var packet := game_reply(Mir3HudView.SM_GROUPMEMBERS,0,0,0,0)
	packet.append_array(encode_scene_bytes(body.to_ascii_buffer()))
	return packet

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("options_path","user://group-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	hud.set_character_identity("组队验证","","","")
	await settle()
	var packets: Array[PackedByteArray] = []
	hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(packet))
	key(KEY_G)
	if not check(hud.hud_snapshot().group_visible,"打开组队窗口"): return
	for enabled in [1,0]:
		click_at(Vector2(300,174),true)
		click_at(Vector2(300,174),false)
		var expected := ("#"+str(packets.size())).to_ascii_buffer()
		expected.append_array(encode_scene_bytes(PackedByteArray([0,0,0,0,Mir3HudView.CM_GROUPMODE&255,Mir3HudView.CM_GROUPMODE>>8,enabled,0,0,0,0,0])))
		expected.append(33)
		if not check(packets[-1]==expected and hud.hud_snapshot().group_allow==bool(enabled),"允许开关完整请求"): return
	var editor: LineEdit = hud.get_node("ChatInput")
	var operations := [Mir3HudView.CM_CREATEGROUP,Mir3HudView.CM_ADDGROUPMEMBER,Mir3HudView.CM_DELGROUPMEMBER]
	for i in range(3):
		click_at(Vector2(315+i*60,322),true)
		click_at(Vector2(315+i*60,322),false)
		if not check(hud.hud_snapshot().group_request==operations[i],"对应组队输入框"): return
		editor.text="Alice"
		key(KEY_ENTER)
		if not check(packets.size()==i+3 and packets[-1]==expected_selection_request(operations[i],i+3,"Alice"),"成员请求正文与序号"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_CREATEGROUP_OK,0,0,0,0)) and hud.hud_snapshot().group_allow,"创建成功允许组队"): return
	if not check(hud.apply_game_message(roster("Leader/Alice/Bob/")) and hud.hud_snapshot().group_members==PackedStringArray(["Leader","Alice","Bob"]),"成员原包刷新"): return
	await settle()
	if not await capture("group-wire-members"): return
	var bad := roster("Invalid/")
	bad[16]=255
	if not check(not hud.apply_game_message(bad) and hud.hud_snapshot().group_members.size()==3,"坏包不清空名单"): return
	for failure in [[Mir3HudView.SM_CREATEGROUP_FAIL,-1,1495],[Mir3HudView.SM_ADDGROUPMEMBER_FAIL,-5,1502],[Mir3HudView.SM_DELGROUPMEMBER_FAIL,-3,1503]]:
		if not check(hud.apply_game_message(game_reply(failure[0],failure[1],0,0,0)) and hud.hud_snapshot().group_request==failure[2],"失败提示资源映射"): return
		if failure[2]==1502:
			await settle()
			if not await capture("group-wire-error"): return
		key(KEY_ENTER)
		if not check(hud.hud_snapshot().group_request==0 and packets.size()==5,"提示关闭不产生请求"): return
	if not check(hud.apply_game_message(roster("Replacement/tail")) and hud.hud_snapshot().group_members==PackedStringArray(["Replacement"]),"列表替换及未终结字段忽略"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_GROUPCANCEL,0,0,0,0)) and hud.hud_snapshot().group_members.is_empty() and hud.hud_snapshot().group_visible and hud.hud_snapshot().group_allow,"取消只清空名单"): return
	print("GROUP_WIRE_UI_OK")
	quit()
