# 测试内容：真实聊天输入发送及GBK编码、私聊回显、短语本地保存与重载、命令标志和延迟、行会加人权限及对话框焦点、界面命令不重复发送。
extends "ui_commands.gd"

func submit(value: String) -> void:
	var edit: LineEdit = hud.get_node("ChatInput")
	edit.grab_focus()
	edit.text=value
	edit.caret_column=value.length()
	key(KEY_ENTER)
func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	var storage := "user://chat-submit-%d" % OS.get_process_id()
	hud.set_meta("item_layout_directory",storage)
	hud.set_meta("options_path",storage+"/Config.ini")
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.set_character_identity("聊天甲","","","")
	hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(packet))
	hud.chat_requested.connect(func(command: String): commands.append(command))
	submit("hello")
	if not check(packets.size()==1 and packets[-1]==expected_selection_request(Mir3HudView.CM_SAY,1,"hello") and hud.hud_snapshot().get("chat_last_text","")!="hello","普通发送不伪造回显"): return
	submit("/Bob hello")
	if not check(packets.size()==2 and hud.hud_snapshot().chat_last_text=="/Bob hello" and hud.hud_snapshot().private_chat_prefix=="/Bob " and hud.hud_snapshot().private_chat_contacts==PackedStringArray(["Bob "]),"私聊回显及前缀"): return
	submit("#集合")
	if not check(packets.size()==2 and hud.hud_snapshot().quick_chat_phrases==PackedStringArray(["集合"]),"中文快捷短语本地保存不发送"): return
	if not check(FileAccess.get_file_as_bytes(storage+"/聊天甲().mlt")==PackedByteArray([0xbc,0xaf,0xba,0xcf,47]) and FileAccess.get_file_as_bytes(storage+"/聊天甲().slt")=="Bob /".to_ascii_buffer(),"原版GBK及斜线分隔文件"): return
	submit("#集合")
	if not check(hud.hud_snapshot().quick_chat_phrases.size()==1,"快捷短语去重"): return
	hud.set_character_identity("聊天乙","","","")
	if not check(hud.hud_snapshot().quick_chat_phrases.is_empty(),"切换角色不串用历史"): return
	hud.set_character_identity("聊天甲","","","")
	if not check(hud.hud_snapshot().quick_chat_phrases==PackedStringArray(["集合"]) and hud.hud_snapshot().private_chat_contacts==PackedStringArray(["Bob "]),"原版历史重新加载"): return
	submit("@chatsave")
	submit("@snow")
	if not check(packets.size()==4 and hud.hud_snapshot().chat_save_enabled and not hud.hud_snapshot().snow_enabled,"本地开关仍发送命令"): return
	submit("@骑马")
	var expected := ("#5").to_ascii_buffer()
	expected.append_array(encode_scene_bytes(PackedByteArray([0,0,0,0,Mir3HudView.CM_SAY&255,Mir3HudView.CM_SAY>>8,0,0,0,0,0,0])))
	expected.append_array(encode_scene_bytes(PackedByteArray(GBK["@骑马"])))
	expected.append(33)
	if not check(packets[-1]==expected and hud.hud_snapshot().map_left_delay_ms==300,"中文命令原包及共享延迟"): return
	submit("@添加成员")
	if not check(packets.size()==5 and hud.hud_snapshot().group_request==1427,"无行会权限不发聊天包"): return
	key(KEY_ENTER)
	hud.set_guild_overview("测试行会",PackedStringArray(),PackedStringArray())
	hud.set_guild_manager(true)
	submit("@添加成员")
	if not check(hud.hud_snapshot().group_request==Mir3HudView.CM_GUILDADDMEMBER and hud.get_node("ChatInput").has_focus(),"姓名框保留新焦点"): return
	await settle()
	if not await capture("chat-submission-guild-add"): return
	hud.get_node("ChatInput").text="NewMember"
	key(KEY_ENTER)
	if not check(packets.size()==6 and packets[-1]==expected_selection_request(Mir3HudView.CM_GUILDADDMEMBER,6,"NewMember"),"确认只发送命名加人协议"): return
	key(KEY_N)
	await create_timer(0.35).timeout
	click(Vector2(336,270))
	if not check(packets.size()==7,"界面按钮仍只发送一次"): return
	key(KEY_N)
	await settle()
	if not await capture("chat-submission-private"): return
	hud.open_login()
	hud.queue_free()
	await create_timer(0.1).timeout
	print("CHAT_SUBMISSION_UI_OK")
	quit()
