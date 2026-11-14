# 测试内容：Ctrl/Alt历史菜单、数字与上下键插入、独立选中状态、越界与空表、普通/展开布局、松键隐藏、模态框隔离和插入不发包。
extends "chat_submission.gd"

func modifier(code: int, pressed: bool) -> void:
	var event := InputEventKey.new()
	event.keycode=code
	event.physical_keycode=code
	event.pressed=pressed
	event.ctrl_pressed=code==KEY_CTRL and pressed
	event.alt_pressed=code==KEY_ALT and pressed
	Input.parse_input_event(event)
func choose(code: int, ctrl: bool, alt: bool = false) -> void:
	var event := InputEventKey.new()
	event.keycode=code
	event.physical_keycode=code
	event.pressed=true
	event.ctrl_pressed=ctrl
	event.alt_pressed=alt
	Input.parse_input_event(event)
	event=event.duplicate()
	event.pressed=false
	Input.parse_input_event(event)
func run() -> void:
	Input.use_accumulated_input=false
	hud=load("res://main.tscn").instantiate()
	hud.set_meta("offline_preview",true)
	hud.set_meta("show_terrain",true)
	hud.set_meta("options_path","user://chat-shortcuts-%d.ini" % OS.get_process_id())
	hud.set_connection_enabled(false)
	root.add_child(hud)
	await settle()
	hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(packet))
	var edit: LineEdit = hud.get_node("ChatInput")
	edit.grab_focus()
	modifier(KEY_CTRL,true)
	choose(KEY_DOWN,true)
	if not check(not hud.hud_snapshot().chat_contacts_visible and edit.text.is_empty() and packets.is_empty(),"空列表不显示不插入"): return
	modifier(KEY_CTRL,false)
	for recipient in ["One","Two","Three"]: submit("/"+recipient+" hello")
	var long_phrase := "长".repeat(40)
	for i in range(10): submit("#"+long_phrase if i==9 else "#集合地点%d" % (i+1))
	var before := packets.size()
	edit.grab_focus()
	modifier(KEY_CTRL,true)
	choose(KEY_2,true)
	if not check(hud.hud_snapshot().chat_contacts_visible and edit.text=="/Two " and edit.caret_column==5 and hud.hud_snapshot().chat_contact_selected==1,"Ctrl数字选择收件人与尾部光标"): return
	choose(KEY_9,true)
	if not check(edit.text=="/Two ","不存在序号保留旧选中项"): return
	choose(KEY_UP,true)
	choose(KEY_UP,true)
	if not check(edit.text=="/One " and hud.hud_snapshot().chat_contact_selected==0,"向上不越界"): return
	await settle()
	if not await capture("chat-shortcuts-contacts"): return
	modifier(KEY_CTRL,false)
	if not check(not hud.hud_snapshot().chat_contacts_visible and edit.text=="/One ","松开Ctrl隐藏但保留文本"): return
	modifier(KEY_ALT,true)
	choose(KEY_0,false,true)
	if not check(hud.hud_snapshot().chat_phrases_visible and edit.text==long_phrase and hud.hud_snapshot().chat_phrase_selected==9,"Alt零选择第十短语"): return
	choose(KEY_DOWN,false,true)
	choose(KEY_UP,false,true)
	if not check(edit.text=="集合地点9" and hud.hud_snapshot().chat_contact_selected==0 and packets.size()==before,"菜单只填入文本且两组选中独立"): return
	modifier(KEY_ALT,false)
	key(KEY_ENTER)
	if not check(packets.size()==before+1,"只有确认发送短语"): return
	key(KEY_R)
	edit.grab_focus()
	modifier(KEY_ALT,true)
	await settle()
	if not await capture("chat-shortcuts-phrases-expanded"): return
	modifier(KEY_ALT,false)
	key(KEY_ESCAPE)
	modifier(KEY_CTRL,true)
	if not check(not hud.hud_snapshot().chat_contacts_visible,"失去输入焦点不显示菜单"): return
	modifier(KEY_CTRL,false)
	hud.set_guild_manager(true)
	submit("@添加成员")
	modifier(KEY_CTRL,true)
	choose(KEY_1,true)
	if not check(hud.hud_snapshot().group_request==Mir3HudView.CM_GUILDADDMEMBER and not hud.hud_snapshot().chat_contacts_visible and edit.text.is_empty(),"姓名模态框不被历史插入覆盖"): return
	modifier(KEY_CTRL,false)
	hud.open_login()
	hud.queue_free()
	await create_timer(0.1).timeout
	print("CHAT_SHORTCUTS_UI_OK")
	quit()
