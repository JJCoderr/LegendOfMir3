# 测试内容：服务器组与账号页显示、输入切换、错误恢复、选服交接、角色创建和删除、开始游戏及真实视口截图。
extends "hud_smoke.gd"

func plain_reply(opcode: int, recognition: int = 0, body: String = "") -> PackedByteArray:
	var packet := encode_scene_bytes(PackedByteArray([recognition&255,(recognition>>8)&255,(recognition>>16)&255,(recognition>>24)&255,opcode&255,(opcode>>8)&255,0,0,0,0,0,0]))
	packet.append_array(encode_scene_bytes(body.to_ascii_buffer()))
	return packet

func game_reply(opcode: int, actor: int, x: int, y: int, param: int) -> PackedByteArray:
	return encode_scene_bytes(PackedByteArray([opcode&255,x&255,y&255,param&255,(opcode>>8)&255,actor&255,(x>>8)&255,(actor>>8)&255,(y>>8)&255,(actor>>16)&255,(param>>8)&255,(actor>>24)&255]))

func settle(seconds: float = 0.08) -> void:
	await create_timer(seconds).timeout
	await process_frame

func run() -> void:
	Input.use_accumulated_input = false
	hud = load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("options_path","user://login-check-%d.ini" % OS.get_process_id())
	hud.set_meta("show_terrain",true)
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	var packets: Array[PackedByteArray] = []
	hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(packet))
	if not check(hud.open_server_groups(),"外部资源服务器组入口"): return
	await settle()
	if not await capture("login-server-groups"): return
	click_at(Vector2(710,330),true)
	if not check(hud.hud_snapshot().server_group_selected==0,"选择上海服务器组"): return
	await settle()
	if not await capture("login-server-group-selected"): return
	click_at(Vector2(80,77),true)
	await settle()
	if not check(hud.hud_snapshot().client_page==1 and hud.hud_snapshot().login_server_count==1,"服务器行交接账号配置"): return
	var urls: Array[String] = []
	hud.login_url_requested.connect(func(url: String): urls.append(url))
	click_at(Vector2(150,390),false)
	click_at(Vector2(290,390),false)
	if not check(urls==["http://reg.mir3.com.cn","http://reg.mir3.com.cn"],"注册与密码按钮采用选组配置网址"): return
	if not check(hud.open_login(),"账号入口读取外部配置"): return
	await settle()
	var editor: LineEdit
	for child in hud.get_children():
		if child is LineEdit: editor=child
	if not check(editor!=null and editor.visible,"账号编辑器显示"): return
	editor.text="account"
	key(KEY_TAB)
	if not check(hud.hud_snapshot().login_field==1 and editor.secret,"Tab切换密码掩码"): return
	editor.text="secret"
	key(KEY_ENTER)
	if not check(packets.size()==1 and packets[0]==expected_selection_request(Mir3HudView.CM_IDPASSWORD,1,"account/secret"),"登录请求正文"): return
	if not check(hud.apply_login_message(plain_reply(Mir3HudView.SM_PASSWD_FAIL,-1)),"密码失败回包"): return
	await settle()
	if not await capture("login-password-error",Vector2i(640,480)): return
	key(KEY_ESCAPE)
	if not check(hud.hud_snapshot().login_notice==113,"Esc不能关闭提示"): return
	key(KEY_ENTER)
	if not check(editor.visible and editor.text.is_empty() and editor.secret,"错误恢复聚焦并清空密码"): return
	await settle()
	if not await capture("login-credentials",Vector2i(640,480)): return
	if not check(hud.apply_login_message(plain_reply(Mir3HudView.SM_PASSOK_SELECTSERVER)),"选服阶段"): return
	await settle()
	if not await capture("login-server-list",Vector2i(640,480)): return
	if not check(hud.apply_login_message(plain_reply(Mir3HudView.SM_SELECTSERVER_OK,-123,"127.0.0.1/7100/")),"选服端点交接"): return
	await settle(2.1)
	if not check(hud.hud_snapshot().client_page==2,"淡出后进入角色页"): return
	if not check(hud.apply_selection_message(selection_packet(0,"")),"空角色列表"): return
	await settle(0.55)
	if not check(hud.hud_snapshot().selection_notice==219,"空角色列表提示"): return
	key(KEY_ENTER)
	click_at(Vector2(450,100),false)
	await settle()
	if not check(hud.hud_snapshot().creation_visible,"创建角色影片结束进入创建页"): return
	click_at(Vector2(355,425),false)
	if not check(hud.hud_snapshot().creation_profession==2,"切换道士职业"): return
	editor.text="Hero"
	var before := packets.size()
	key(KEY_ENTER)
	if not check(packets.size()==before+1,"创建角色请求发出"): return
	await settle()
	if not await capture("login-create-character",Vector2i(640,480)): return
	if not check(hud.apply_selection_message(plain_reply(Mir3HudView.SM_NEWCHR_FAIL,2)) and hud.hud_snapshot().selection_notice==224,"重名提示"): return
	key(KEY_ENTER)
	if not check(editor.visible and editor.text.is_empty(),"重名后编辑器清空"): return
	if not check(hud.apply_selection_message(plain_reply(Mir3HudView.SM_NEWCHR_SUCCESS)),"创建成功返回选角"): return
	await settle()
	if not check(hud.apply_selection_message(selection_packet(1,"Hero/2/x/10/0")),"创建后刷新角色列表"): return
	await settle(0.55)
	click_at(Vector2(90,250),false)
	if not check(hud.hud_snapshot().selection_delete_visible,"删除确认框"): return
	key(KEY_ESCAPE)
	if not check(hud.hud_snapshot().selection_delete_visible,"删除框忽略Esc"): return
	key(KEY_TAB)
	if not check(hud.hud_snapshot().selection_delete_selected==2,"Tab选择取消"): return
	before=packets.size()
	key(KEY_SPACE)
	if not check(not hud.hud_snapshot().selection_delete_visible and packets.size()==before,"空格执行取消且不发包"): return
	click_at(Vector2(90,250),false)
	editor.text="Hero"
	await settle()
	if not await capture("login-delete-character",Vector2i(640,480)): return
	before=packets.size()
	key(KEY_ENTER)
	if not check(packets.size()==before+2,"确认删除后发送删除及查询"): return
	if not check(hud.apply_selection_message(selection_packet(1,"Hero/2/x/10/0")),"恢复选角列表"): return
	await settle(0.55)
	if not await capture("login-character-selection",Vector2i(640,480)): return
	click_at(Vector2(280,60),false)
	if not check(hud.apply_selection_message(plain_reply(Mir3HudView.SM_STARTPLAY,0,"127.0.0.1/7200/")),"开始游戏端点"): return
	await settle()
	if not check(hud.hud_snapshot().client_page==3,"开始影片后进入游戏阶段"): return
	var arrival := game_reply(Mir3HudView.SM_NEWMAP,100,413,410,0)
	arrival.append_array(encode_scene_bytes("0".to_ascii_buffer()))
	if not check(hud.apply_game_message(arrival) and hud.hud_snapshot().map_id=="0", "首次地图在后续人物包前完成加载"): return
	var actor := game_reply(Mir3HudView.SM_LOGON,100,413,410,4)
	actor.append_array(encode_scene_bytes(PackedByteArray([0,0,1,128,0,0,0,0,0,0,0,0,0,0,0,0])))
	actor.append_array(encode_scene_bytes(PackedByteArray([0,0,0,0,0])))
	actor.append_array(encode_scene_bytes(PackedByteArray([0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0])))
	before=packets.size()
	if not check(hud.apply_game_message(actor) and hud.hud_snapshot().local_actor_identity==100 and hud.hud_snapshot().player_body==Vector2i(0,1) and packets.size()==before+1,"本地人物外观绑定及背包查询"): return
	await settle()
	if not await capture("login-game-arrival"): return
	print("LOGIN_UI_OK")
	quit()
