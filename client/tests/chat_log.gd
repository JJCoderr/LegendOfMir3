# 测试内容：原版聊天日志通道次数、真实开关提交、GBK与CRLF追加、私聊本地回显不记日志、编码边界及异常包、文件打开失败不阻断显示。
extends "chat_submission.gd"

func incoming(opcode: int, text: PackedByteArray) -> PackedByteArray:
	var result := game_reply(opcode,999,2,0,0)
	result.append_array(encode_scene_bytes(text))
	return result
func run() -> void:
	Input.use_accumulated_input=false
	var directory := "user://chat-log-%d" % OS.get_process_id()
	DirAccess.make_dir_recursive_absolute(ProjectSettings.globalize_path(directory))
	var path := directory+"/Chat.txt"
	var seed := FileAccess.open(path,FileAccess.WRITE)
	seed.store_buffer("old\r\n".to_ascii_buffer())
	seed.close()
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("chat_log_path",path)
	hud.set_meta("options_path",directory+"/Config.ini")
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	var expected := "old\r\n".to_ascii_buffer()
	var text := PackedByteArray([0xbc,0xaf,0xba,0xcf])
	var line := text.duplicate()
	line.append_array(PackedByteArray([13,10]))
	var channels := [Mir3HudView.SM_HEAR,Mir3HudView.SM_SYSMESSAGE,Mir3HudView.SM_GROUPMESSAGE,Mir3HudView.SM_CRY,Mir3HudView.SM_WHISPER,Mir3HudView.SM_GUILDMESSAGE,Mir3HudView.SM_MONSTERSAY]
	for enabled in [false,true]:
		if enabled: submit("@chatsave")
		for opcode in channels:
			if not check(hud.apply_game_message(incoming(opcode,text)),"接受聊天通道原包"): return
			var copies := 0
			if enabled and opcode!=Mir3HudView.SM_SYSMESSAGE: copies+=1
			if opcode!=Mir3HudView.SM_GUILDMESSAGE: copies+=1
			for i in range(copies): expected.append_array(line)
			if not check(FileAccess.get_file_as_bytes(path)==expected,"原版通道次数和追加字节 %d %s" % [opcode,enabled]): return
			if not check(hud.hud_snapshot().chat_last_text=="集合","收到消息正常显示"): return
	submit("/Local hello")
	submit("#phrase")
	if not check(FileAccess.get_file_as_bytes(path)==expected,"本地私聊回显和短语不写入接收日志"): return
	submit("@chatsave")
	hud.apply_game_message(incoming(Mir3HudView.SM_WHISPER,"after-off".to_ascii_buffer()))
	expected.append_array("after-off\r\n".to_ascii_buffer())
	if not check(FileAccess.get_file_as_bytes(path)==expected,"关闭后非行会仍追加一次"): return
	hud.apply_game_message(incoming(Mir3HudView.SM_SYSMESSAGE,PackedByteArray([65,0,66])))
	expected.append_array("A\r\n".to_ascii_buffer())
	var bad := incoming(Mir3HudView.SM_SYSMESSAGE,"broken".to_ascii_buffer())
	bad[-1]=0
	if not check(not hud.apply_game_message(bad) and FileAccess.get_file_as_bytes(path)==expected,"畸形编码不写文件"): return
	var longest := "x".repeat(768).to_ascii_buffer()
	if not check(hud.apply_game_message(incoming(Mir3HudView.SM_SYSMESSAGE,longest)),"编码正文1024字节边界"): return
	expected.append_array(longest)
	expected.append_array(PackedByteArray([13,10]))
	if not check(not hud.apply_game_message(incoming(Mir3HudView.SM_SYSMESSAGE,"x".repeat(769).to_ascii_buffer())) and FileAccess.get_file_as_bytes(path)==expected,"超过原版编码长度不显示不写文件"): return
	hud.apply_game_message(incoming(Mir3HudView.SM_SYSMESSAGE,"Chat log received.".to_ascii_buffer()))
	await settle()
	if not await capture("chat-log-received"): return
	hud.open_login()
	hud.queue_free()
	await create_timer(0.1).timeout
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("chat_log_path",directory)
	hud.set_meta("options_path",directory+"/Other.ini")
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	if not check(hud.apply_game_message(incoming(Mir3HudView.SM_SYSMESSAGE,"Still visible".to_ascii_buffer())) and hud.hud_snapshot().chat_last_text=="Still visible","日志路径是目录时仍显示消息"): return
	hud.queue_free()
	await create_timer(0.1).timeout
	print("CHAT_LOG_UI_OK")
	quit()
