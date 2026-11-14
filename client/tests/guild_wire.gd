# 测试内容：行会打开与分页查询、主页续包和成员替换、管理权限、公告及封号提交、删除和解除联盟请求、回包提示与场景清理。
extends "login_smoke.gd"

var packets: Array[PackedByteArray] = []
func reply(opcode: int, body: String) -> PackedByteArray:
	var packet := game_reply(opcode,0,0,0,0)
	packet.append_array(encode_scene_bytes(body.to_ascii_buffer()))
	return packet
func click(pos: Vector2) -> void:
	click_at(pos,true)
	click_at(pos,false)
func last_request(opcode: int, body: String = "") -> bool:
	return check(not packets.is_empty() and packets[-1]==expected_selection_request(opcode,(packets.size()-1)%9+1,body),"行会完整请求及序号")

func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("options_path","user://guild-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(packet))
	key(KEY_F)
	if not last_request(Mir3HudView.CM_GUILDHOME): return
	if not check(not hud.hud_snapshot().guild_visible,"查询等待回包"): return
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_OPENGUILDDLG_FAIL,0,0,0,0)) and hud.hud_snapshot().group_request==1473,"未入会提示"): return
	key(KEY_ENTER)
	if not check(not hud.apply_game_message(reply(Mir3HudView.SM_GUILD_OVERVIEW,"orphan\r")),"续包要求初始主页"): return
	if not check(hud.apply_game_message(reply(Mir3HudView.SM_OPENGUILDDLG,"Bichon\r\r0\r<Notice>\rWelcome\rHel")),"主页打开"): return
	if not check(hud.hud_snapshot().guild_visible and not hud.hud_snapshot().guild_manager,"成员权限"): return
	click(Vector2(450,415))
	if not check(hud.hud_snapshot().group_request==4000,"无管理权提示"): return
	key(KEY_ENTER)
	if not check(hud.apply_game_message(reply(Mir3HudView.SM_OPENGUILDDLG,"Bichon\r\r1\r<Notice>\rWelcome\rHel")),"管理主页"): return
	if not check(hud.apply_game_message(reply(Mir3HudView.SM_GUILD_OVERVIEW,"lo\rIgnored")) and hud.hud_snapshot().guild_overview[-1]=="Hello","原版续包缓冲"): return
	click(Vector2(196,410))
	if not last_request(Mir3HudView.CM_GUILDMEMBERLIST): return
	if not check(hud.apply_game_message(reply(Mir3HudView.SM_GUILD_OVERVIEW_UPDATE,"Next\r")) and hud.hud_snapshot().guild_tab==1,"续包保留当前页"): return
	for opcode in [Mir3HudView.SM_SENDGUILDMEMBERLIST,Mir3HudView.SM_GUILD_MEMBERS_UPDATE,Mir3HudView.SM_GUILD_MEMBERS_REFRESH]:
		if not check(hud.apply_game_message(reply(opcode,"#1/*Leader/Hero/#2/*Member/Alice/Bob/")),"成员原包"): return
	if not check(hud.hud_snapshot().guild_members.size()==7 and hud.hud_snapshot().guild_members[-1]=="Alice Bob ","名单替换不追加"): return
	var bad := reply(Mir3HudView.SM_SENDGUILDMEMBERLIST,"Bad/")
	bad[16]=255
	if not check(not hud.apply_game_message(bad) and hud.hud_snapshot().guild_members.size()==7,"非法编码保留名单"): return
	await settle()
	if not await capture("guild-wire-members"): return
	click(Vector2(145,410))
	if not last_request(Mir3HudView.CM_GUILDHOME): return
	click(Vector2(400,415))
	var editor: TextEdit = hud.get_node("GuildEdit")
	if not check(editor.visible,"公告编辑器"): return
	editor.text="Welcome\n\nTonight"
	await settle()
	if not await capture("guild-wire-editor"): return
	click(Vector2(575,130))
	if not last_request(Mir3HudView.CM_GUILDUPDATENOTICE,"Welcome\r \rTonight"): return
	click(Vector2(550,415))
	editor.text="#1 <Leader>\nHero\n#2 <Member>\nAlice"
	click(Vector2(575,130))
	if not last_request(Mir3HudView.CM_GUILDUPDATERANKINFO,"#1 <Leader>\rHero\r#2 <Member>\rAlice"): return
	click(Vector2(450,415))
	hud.get_node("ChatInput").text="Alice"
	key(KEY_ENTER)
	if not last_request(Mir3HudView.CM_GUILDMEMBERLIST): return
	if not check(packets[-2]==expected_selection_request(Mir3HudView.CM_GUILDDELMEMBER,(packets.size()-2)%9+1,"Alice"),"删除后查询顺序"): return
	var before := packets.size()
	click(Vector2(597,414))
	hud.get_node("ChatInput").text="Ally"
	key(KEY_ENTER)
	if not check(packets.size()==before+2,"解除联盟两条请求"): return
	if not last_request(Mir3HudView.CM_GUILDHOME): return
	var expected := ("#"+str((packets.size()-2)%9+1)).to_ascii_buffer()
	expected.append_array(encode_scene_bytes(PackedByteArray([0,0,0,0,Mir3HudView.CM_SAY&255,Mir3HudView.CM_SAY>>8,0,0,0,0,0,0])))
	expected.append_array(encode_scene_bytes(PackedByteArray([198,198,187,181,189,225,195,203,32,65,108,108,121])))
	expected.append(33)
	if not check(packets[-2]==expected,"解除联盟使用原中文命令与GBK正文"): return
	for opcode in [Mir3HudView.SM_GUILDADDMEMBER_OK,Mir3HudView.SM_GUILDDELMEMBER_OK]:
		if not check(hud.apply_game_message(game_reply(opcode,0,0,0,0)),"成员成功回包"): return
		if not last_request(Mir3HudView.CM_GUILDMEMBERLIST): return
		if opcode==Mir3HudView.SM_GUILDADDMEMBER_OK: key(KEY_ENTER)
	if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_GUILDRANKUPDATE_FAIL,-2,0,0,0)) and hud.hud_snapshot().group_request==1483,"封号失败提示"): return
	await settle()
	if not await capture("guild-wire-error"): return
	key(KEY_ENTER)
	if not check(hud.open_login(),"返回登录页清理游戏会话"): return
	if not check(hud.hud_snapshot().guild_members.is_empty() and hud.hud_snapshot().guild_overview.is_empty() and not hud.hud_snapshot().guild_manager,"离场清空行会会话"): return
	if not check(not hud.apply_game_message(reply(Mir3HudView.SM_GUILD_OVERVIEW,"Old\r")),"离场拒绝旧续包"): return
	print("GUILD_WIRE_UI_OK")
	quit()
