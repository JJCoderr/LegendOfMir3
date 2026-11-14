# 测试内容：真实TCP登录请求、半包与心跳、选角连接交接和自动查询、游戏认证首包以及正常断开。
extends "login_smoke.gd"

func accept_peer(server: TCPServer) -> StreamPeerTCP:
	var deadline := Time.get_ticks_msec()+3000
	while not server.is_connection_available() and Time.get_ticks_msec()<deadline:
		await process_frame
	return server.take_connection() if server.is_connection_available() else null

func receive_packet(peer: StreamPeerTCP, end: int = 33) -> PackedByteArray:
	var result := PackedByteArray()
	var deadline := Time.get_ticks_msec()+3000
	while Time.get_ticks_msec()<deadline:
		peer.poll()
		if peer.get_available_bytes()>0:
			var data := peer.get_data(peer.get_available_bytes())
			if data[0]!=OK: return PackedByteArray()
			result.append_array(data[1])
			if result.has(end): return result
		await process_frame
	return result

func framed(payload: PackedByteArray) -> PackedByteArray:
	var result := PackedByteArray([35])
	result.append_array(payload)
	result.append(33)
	return result

func run() -> void:
	var config := ConfigFile.new()
	if not check(config.load("res://resource_path.cfg")==OK,"读取资源路径配置"): return
	var resource_root: String = config.get_value("resources","root")
	var settings := FileAccess.get_file_as_bytes(resource_root.path_join("Mir3.ini")).get_string_from_ascii()
	var host := ""
	var port := 0
	for line in settings.split("\n"):
		var entry := line.strip_edges()
		if entry.begins_with("ServerAddr="): host=entry.trim_prefix("ServerAddr=")
		if entry.begins_with("Param1="): port=int(entry.trim_prefix("Param1="))
	if not check(host=="127.0.0.1" and port>0,"仅连接配置中的本地测试网关"): return
	var gateway := TCPServer.new()
	var selection_server := TCPServer.new()
	var game_server := TCPServer.new()
	if not check(gateway.listen(port,"127.0.0.1")==OK and selection_server.listen(17124,"127.0.0.1")==OK and game_server.listen(17125,"127.0.0.1")==OK,"本地三阶段监听"): return
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("options_path","user://login-tcp-%d.ini" % OS.get_process_id())
	root.add_child(hud)
	if not check(hud.open_login(),"打开账号页并连接"): return
	var peer := await accept_peer(gateway)
	if not check(peer!=null,"真实TCP网关连接"): return
	await settle()
	var editor: LineEdit
	for child in hud.get_children():
		if child is LineEdit: editor=child
	editor.text="account"
	key(KEY_TAB)
	editor.text="secret"
	key(KEY_ENTER)
	var received := await receive_packet(peer)
	if not check(received==expected_selection_request(Mir3HudView.CM_IDPASSWORD,1,"account/secret"),"登录包通过TCP发出"): return
	var failure := framed(plain_reply(Mir3HudView.SM_PASSWD_FAIL,-1))
	peer.put_data(failure.slice(0,7))
	await settle()
	if not check(hud.hud_snapshot().login_notice==0,"半包不提前分发"): return
	var tail := failure.slice(7)
	tail.append(42)
	peer.put_data(tail)
	received=await receive_packet(peer,42)
	if not check(received==PackedByteArray([42]) and hud.hud_snapshot().login_notice==113,"尾包完成并回复心跳"): return
	key(KEY_ENTER)
	var replies := framed(plain_reply(Mir3HudView.SM_PASSOK_SELECTSERVER))
	replies.append_array(framed(plain_reply(Mir3HudView.SM_SELECTSERVER_OK,-123,"127.0.0.1/17124/")))
	peer.put_data(replies)
	var selection := await accept_peer(selection_server)
	if not check(selection!=null,"选服淡出后建立选角连接"): return
	received=await receive_packet(selection)
	if not check(received==expected_selection_request(Mir3HudView.CM_QUERYCHR,1,"account/-123"),"选角连接自动查询且序号重置"): return
	selection.put_data(framed(selection_packet(1,"Hero/0/x/10/0")))
	await settle(0.6)
	click_at(Vector2(280,60),false)
	received=await receive_packet(selection)
	if not check(received==expected_selection_request(Mir3HudView.CM_SELCHR,2,"account/Hero"),"开始游戏请求通过选角连接"): return
	selection.put_data(framed(plain_reply(Mir3HudView.SM_STARTPLAY,0,"127.0.0.1/17125/")))
	var game := await accept_peer(game_server)
	if not check(game!=null,"游戏连接建立"): return
	received=await receive_packet(game)
	var session := -123
	var auth := "**account/Hero/%d/%d/%d/1" % [int((session^0x3eb2c5cc)&0xffffffff)-0x100000000,int((0x0131a4f0^session^0x5580af27)&0xffffffff)-0x100000000,int(0x0131a4f0^0xfa0280af)-0x100000000]
	var expected := "#1".to_ascii_buffer()
	expected.append_array(encode_scene_bytes(auth.to_ascii_buffer()))
	expected.append(33)
	if not check(received==expected,"无普通消息头的游戏认证首包"): return
	game.disconnect_from_host()
	await settle()
	hud.set_connection_enabled(false)
	gateway.stop()
	selection_server.stop()
	game_server.stop()
	print("LOGIN_TCP_OK")
	quit()
