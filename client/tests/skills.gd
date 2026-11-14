# 技能统一验收实现：由 Python 入口传入场景计划，所有场景共用一个进程。
extends "forced_motion.gd"


var active_arguments := PackedStringArray()
var suite_music: AudioStreamPlayer
var music_sources: Array[AudioStreamPlayer] = []
var suite_failed := false

func skill_arguments() -> PackedStringArray:
    return active_arguments

# 后续消息专项：不替服务端计算技能结果，只验客户端收到结果后能完整呈现和清理。
var outcome_origin := Vector2i()
var outcome_target := Vector2i()

func outcome_ids(available: Array) -> Array:
    for argument in skill_arguments():
        if argument.begins_with("--skill=") or argument.begins_with("--skills="):
            var values := argument.split("=",true,1)[1].split(",")
            if not check(Array(values).all(func(v): return v.is_valid_int() and int(v) in available), "后续效果技能筛选有效"): return []
            return Array(values).map(func(v): return int(v))
    return available

func prepare_outcome(skill: int) -> bool:
    Input.use_accumulated_input = false
    hud = load("res://main.tscn").instantiate()
    hud.set_meta("offline_preview", true)
    hud.set_meta("options_path", "user://skill-outcome-%d.ini" % OS.get_process_id())
    hud.set_connection_enabled(false)
    root.add_child(hud)
    await settle()
    hud.set_local_actor_identity(77)
    hud.set_character_vitals(100,100,200,200,0,100)
    outcome_origin = hud.hud_snapshot().player_cell + Vector2i(0,2)
    outcome_target = outcome_origin + Vector2i(0,4)
    if not check(hud.change_map("0","比奇省",outcome_origin,1), "准备后续效果场景"): return false
    var learned := game_reply(Mir3HudView.SM_SENDMYMAGIC,0,0,0,1)
    learned.append_array(skill_record(skill,49)); learned.append(47)
    if not check(hud.apply_game_message(learned), "学习技能并绑定F1"): return false
    var packets: Array[PackedByteArray] = []
    hud.client_packet_ready.connect(func(packet: PackedByteArray):
        packets.append(packet)
        hud.apply_game_message("+GOOD".to_ascii_buffer()))
    if not check(await wait_until(func(s): return s.map_left_delay_ms==0), "输入门控结束"): return false
    key(KEY_F1)
    if not check(packets.size()==1 and decode_request(packets[0]).decode_u16(8)==skill, "施法请求技能编号正确"): return false
    if not check(await wait_until(func(s): return s.player_action==-1,4000), "起手结束"): return false
    if not check(hud.apply_game_message(magic_reply(skill,outcome_target)), "接收技能效果回包"): return false
    return true

# 17普通骷髅、30神兽、105高级召唤：外观由服务器入场包决定，105不凭名称推断专属外观。
func run_summon() -> void:
    for skill in outcome_ids([17,30,105]):
        if not await prepare_outcome(skill): return
        var shape: int = 90 if skill==30 else 32
        const PET := 130
        if not check(not hud.hud_snapshot().scene_chicken_cells.has(PET), "施法回包不能凭空生成召唤物"): return
        var feature := PackedByteArray(); feature.resize(16)
        feature[0]=3; feature.encode_u16(2,shape)
        var entry := game_reply(Mir3HudView.SM_DIGUP,PET,outcome_target.x,outcome_target.y,0)
        entry.append_array(encode_scene_bytes(feature))
        if not check(hud.apply_game_message(entry), "召唤物通过入场协议创建"): return
        if not check(await wait_until(func(s): return s.scene_chicken_bounds.has(PET) and s.scene_chicken_bounds[PET].has_area()), "召唤物原资源身体可见"): return
        if not check(hud.hud_snapshot().scene_monster_shapes[PET]==shape and hud.hud_snapshot().scene_chicken_cells[PET]==outcome_target, "外观和位置来自入场包"): return
        var count: int = hud.hud_snapshot().scene_chicken_cells.size()
        if not check(hud.apply_game_message(entry), "重复入场包"): return
        await settle()
        if not check(hud.hud_snapshot().scene_chicken_cells.size()==count, "重复入场不创建第二只召唤物"): return
        if not await capture("skill-summon-%d" % skill): return
        var moved := outcome_target + Vector2i(2,0)
        var turn := game_reply(Mir3HudView.SM_TURN,PET,moved.x,moved.y,2)
        turn.append_array(encode_scene_bytes(feature))
        if not check(hud.apply_game_message(turn), "召唤物后续移动原包"): return
        if not check(await wait_until(func(s): return s.scene_chicken_cells[PET]==moved and s.scene_chicken_steps[PET]==-1), "召唤物移动到服务器落点"): return
        if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_DISAPPEAR,PET,0,0,0)), "召唤物离场原包"): return
        if not check(await wait_until(func(s): return not s.scene_chicken_cells.has(PET)), "离场释放召唤物"): return
        if not check(hud.apply_game_message(entry), "同编号重新入场"): return
        if not check(await wait_until(func(s): return s.scene_chicken_cells.has(PET)), "重新入场恢复召唤物"): return
        if not check(hud.change_map("0","比奇省",outcome_origin,1) and not hud.hud_snapshot().scene_chicken_cells.has(PET), "换图释放召唤物"): return
        hud.queue_free(); await settle()
        print("SKILL_SUMMON_UI_OK ",skill)

# 困魔咒/火墙：普通技能回包不创建持续地面效果，实际由SHOWEVENT/HIDEEVENT控制。
func run_persistent() -> void:
    for skill in outcome_ids([16,22]):
        if not await prepare_outcome(skill): return
        const EVENT := 730
        if not check(hud.hud_snapshot().map_event_count==0, "技能回包不代替持续事件"): return
        var event := game_reply(Mir3HudView.SM_SHOWEVENT,EVENT,4 if skill==16 else 5,outcome_target.x,outcome_target.y)
        event.append_array(encode_scene_bytes(PackedByteArray([0,0,0,0])))
        if not check(hud.apply_game_message(event), "服务器创建持续地面事件"): return
        if not check(await wait_until(func(s): return s.map_event_frames.has(EVENT)), "持续效果进入帧推进"): return
        if not check(hud.apply_game_message(event) and hud.hud_snapshot().map_event_count==1, "重复编号不叠加地面效果"): return
        await settle(0.8)
        var frame: int= hud.hud_snapshot().map_event_frames[EVENT]
        if not check(frame >= (640 if skill==16 else 920) and frame < (650 if skill==16 else 925), "持续循环使用对应技能帧段"): return
        if not await capture("skill-persistent-%d" % skill): return
        if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_HIDEEVENT,EVENT,0,0,0)), "服务器停止持续事件"): return
        if skill==16:
            if not check(await wait_until(func(s): return not s.map_event_frames.has(EVENT)), "困魔咒停止后消失"): return
        else:
            frame=hud.hud_snapshot().map_event_frames[EVENT]
            if not check(frame>=2450 and frame<=2490 and hud.hud_snapshot().map_ground_event_count==1, "火墙停止后保留余烬而非立即删除"): return
            if not await capture("skill-persistent-22-embers"): return
        if not check(hud.change_map("0","比奇省",outcome_origin,1) and hud.hud_snapshot().map_event_count==0 and hud.hud_snapshot().map_ground_event_count==0, "换图清除持续效果及余烬"): return
        hud.queue_free(); await settle()
        print("SKILL_PERSISTENT_UI_OK ",skill)

# 抗拒火环的受击者由强制位移包驱动；传送/换位由服务器地图落点驱动，施法成功不自行位移。
func run_displacement() -> void:
    for skill in outcome_ids([8,21,104]):
        if not await prepare_outcome(skill): return
        if not check(hud.hud_snapshot().player_cell==outcome_origin, "施法确认不自行改写位置"): return
        if skill==8:
            # 接收端退步：结果由服务器落点决定，不在测试中替服务器选择受击对象。
            var destination := outcome_origin+Vector2i(0,1)
            if not check(hud.apply_game_message(motion_reply(Mir3HudView.SM_BACKSTEP,77,destination,4)), "接收端退步回包"): return
            if not check(await wait_until(func(s): return s.player_cell==destination and s.forced_motion_opcode==0 and s.walk_frame==-1), "退步结束并保持服务器落点"): return
        else:
            var packet := game_reply(Mir3HudView.SM_CHANGEMAP,77,outcome_target.x,outcome_target.y,1)
            packet.append_array(encode_scene_bytes("0".to_ascii_buffer()))
            if not check(hud.apply_game_message(packet), "接收服务器传送落点"): return
            if not check(await wait_until(func(s): return s.player_cell==outcome_target and s.map_id=="0"), "地图内传送到指定格"): return
            if not check(hud.hud_snapshot().spell_effects.is_empty() and not hud.hud_snapshot().action_reply_pending, "传送清理原施法和等待状态"): return
        if not await capture("skill-displacement-%d" % skill): return
        hud.queue_free(); await settle()
        print("SKILL_DISPLACEMENT_UI_OK ",skill)

func check(condition: bool, label: String) -> bool:
    if not condition:
        suite_failed = true
        push_error("技能验收失败：" + label)
        quit(1)
    return condition

func preserve_music(node: Node) -> void:
    # HUD 换图仍照常处理自己的音乐；验收会话独立播放同一原始流，避免重播干扰听感。
    if node is AudioStreamPlayer and node.name == "MapMusic":
        node.bus = "SkillTestMapMusic"
        music_sources.append(node)

func sync_suite_music() -> void:
    if suite_music.stream != null:
        return
    for source in music_sources:
        if is_instance_valid(source) and source.stream != null and source.playing:
            suite_music.stream = source.stream
            suite_music.volume_db = source.volume_db
            suite_music.play()
            print("SKILL_SUITE_MUSIC_STARTED")
            return

func run() -> void:
    var plan: Array = []
    for argument in OS.get_cmdline_user_args():
        if argument.begins_with("--plan="):
            var decoded = JSON.parse_string(argument.trim_prefix("--plan="))
            if not check(decoded is Array, "Python 场景计划格式有效"): return
            plan = decoded
    if not check(not plan.is_empty(), "请通过 Python 统一入口启动技能测试"): return
    var bus := AudioServer.bus_count
    AudioServer.add_bus()
    AudioServer.set_bus_name(bus, "SkillTestMapMusic")
    AudioServer.set_bus_mute(bus, true)
    suite_music = AudioStreamPlayer.new()
    root.add_child(suite_music)
    node_added.connect(preserve_music)
    process_frame.connect(sync_suite_music)
    for item in plan:
        var method := "run_" + str(item["case"]).replace("-", "_")
        if not check(has_method(method), "有效技能场景：" + method): return
        active_arguments = PackedStringArray(item["args"])
        # 技能绑定的换会话断言会切换到 640×480 登录页；下一场景恢复地图视口。
        root.content_scale_size = Vector2i(800,600)
        root.size = Vector2i(800,600)
        selected_skill = 0
        actions.clear()
        requests.clear()
        print("SKILL_SCENARIO_START " + str(item["case"]) + " " + str(active_arguments))
        await call(method)
        if suite_failed: return
        # 旧独立脚本部分依赖进程退出释放 HUD；合并后必须在场景边界主动释放。
        if is_instance_valid(hud):
            hud.queue_free()
            await settle()
        music_sources.clear()
        if suite_music.stream != null:
            if not check(suite_music.playing, "跨场景背景音乐持续播放"): return
            print("SKILL_SUITE_MUSIC_POSITION " + str(suite_music.get_playback_position()))
        print("SKILL_SCENARIO_OK " + str(item["case"]))
    process_frame.disconnect(sync_suite_music)
    node_added.disconnect(preserve_music)
    suite_music.queue_free()
    AudioServer.remove_bus(AudioServer.get_bus_index("SkillTestMapMusic"))
    # 等待延迟释放完成再结束 SceneTree，避免会话音乐对象留到引擎强制退出。
    await settle()
    print("SKILLS_COLLECTION_OK scenarios=" + str(plan.size()))
    quit()

# 场景：bindings
# 测试内容：技能原始分段列表、增量追加、修炼等级、删除及快捷键保留、冲突清除后绑定的原包顺序、畸形包原子拒绝和离场清理。

func record(id: int, key_code: int, level: int = 1) -> PackedByteArray:
    var raw := PackedByteArray()
    raw.resize(74)
    raw[0]=42
    raw[4]=key_code
    raw[5]=level
    raw[6]=id&255
    raw[7]=(id>>8)&255
    var name := "Magic".to_ascii_buffer()
    for i in range(name.size()): raw[8+i]=name[i]
    raw[31]=100
    raw[35]=200
    raw[39]=44
    raw[40]=1
    return encode_scene_bytes(raw)
func records_packet(opcode: int, records: Array[PackedByteArray]) -> PackedByteArray:
    var packet := game_reply(opcode,0,0,0,0xab00|records.size())
    for row in records:
        packet.append_array(row)
        if opcode==Mir3HudView.SM_SENDMYMAGIC: packet.append(47)
    return packet
func binding_packet(id: int, key_code: int, sequence: int) -> PackedByteArray:
    var packet := ("#"+str(sequence)).to_ascii_buffer()
    packet.append_array(encode_scene_bytes(PackedByteArray([id&255,(id>>8)&255,0,0,Mir3HudView.CM_MAGICKEYCHANGE&255,Mir3HudView.CM_MAGICKEYCHANGE>>8,key_code,0,0,0,0,0])))
    packet.append(33)
    return packet

func run_bindings() -> void:
    Input.use_accumulated_input=false
    hud=load("res://main.tscn").instantiate()
    hud.set_meta("offline_preview",true)
    hud.set_meta("show_terrain",true)
    hud.set_meta("options_path","user://skill-%d.ini" % OS.get_process_id())
    hud.set_connection_enabled(false)
    root.add_child(hud)
    await settle()
    var packets: Array[PackedByteArray] = []
    hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(packet))
    var full := records_packet(Mir3HudView.SM_SENDMYMAGIC,[record(1,49),record(5,0)])
    if not check(hud.apply_game_message(full) and hud.hud_snapshot().learned_skills.size()==2 and hud.hud_snapshot().skill_bindings[1]==1,"分段原包列表及绑定"): return
    key(KEY_E)
    var motion := InputEventMouseMotion.new()
    motion.position=Vector2(385,152)
    Input.parse_input_event(motion)
    key(KEY_F1)
    key(KEY_F12)
    if not check(packets==[binding_packet(1,0,1),binding_packet(5,49,2),binding_packet(5,67,3)],"冲突清除再绑定、F12编码及序号"): return
    if not check(hud.hud_snapshot().skill_bindings[1]==0 and hud.hud_snapshot().skill_bindings[12]==5,"本地快捷键同步"): return
    if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_MAGIC_LVEXP,5,0xff02,0xcdef,0x89ab)),"等级修炼原包"): return
    var progress: Dictionary = hud.hud_snapshot().learned_skills[1]
    if not check(progress.level==2 and progress.experience==0x89abcdef and progress.required==300,"等级低字节与32位经验组合"): return
    if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_MAGIC_LVEXP,5,2,215,0)),"正常修炼值显示"): return
    await settle()
    if not await capture("skill-wire-progress"): return
    click_at(Vector2(385,152),true)
    click_at(Vector2(385,152),false)
    var popup := InputEventMouseButton.new()
    popup.position=Vector2(385,152)
    popup.button_index=MOUSE_BUTTON_LEFT
    popup.pressed=true
    popup.double_click=true
    root.push_input(popup,true)
    click_at(Vector2(385,152),false)
    if not await capture("skill-wire-key-menu"): return
    if not check(hud.apply_game_message(records_packet(Mir3HudView.SM_ADDMAGIC,[record(261,65)])),"新增高位技能ID"): return
    if not check(hud.hud_snapshot().learned_skills.size()==3 and hud.hud_snapshot().skill_bindings[10]==0,"新增A键标签不按全量规则建立绑定"): return
    var bad := full.slice(0,full.size()-2)
    if not check(not hud.apply_game_message(bad) and hud.hud_snapshot().learned_skills.size()==3,"坏列表不部分替换"): return
    if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_DELMAGIC,5,0,0,0)) and hud.hud_snapshot().learned_skills.size()==2 and hud.hud_snapshot().skill_bindings[12]==5,"删除保留原版快捷表"): return
    if not check(hud.apply_game_message(records_packet(Mir3HudView.SM_SENDMYMAGIC,[record(1,49),record(5,67)])),"重新全量列表"): return
    click_at(Vector2(435,202),true)
    click_at(Vector2(435,202),false)
    if not check(packets.size()==3 and hud.hud_snapshot().skill_bindings[12]==5,"删除和重同步使旧快捷键菜单失效"): return
    click_at(Vector2(636,32),true)
    click_at(Vector2(636,32),false)
    if not check(hud.hud_snapshot().skill_tab==7,"可切换技能分类页签"): return
    click_at(Vector2(397,32),true)
    click_at(Vector2(397,32),false)
    motion=InputEventMouseMotion.new()
    motion.position=Vector2(385,152)
    root.push_input(motion,true)
    await settle()
    if not await capture("skill-wire-list"): return
    if not check(hud.apply_game_message(records_packet(Mir3HudView.SM_SENDMYMAGIC,[])) and hud.hud_snapshot().learned_skills.is_empty() and hud.hud_snapshot().skill_bindings[12]==5,"空全量清列表但不擅改快捷表"): return
    if not check(hud.open_login() and hud.hud_snapshot().learned_skills.is_empty() and hud.hud_snapshot().skill_bindings==PackedInt32Array([0,0,0,0,0,0,0,0,0,0,0,0,0]),"换会话清除技能与快捷表"): return
    print("SKILL_WIRE_UI_OK")
    return


# 场景：casting
# 技能定向验收：直接进入地图，不执行登录、选角或其他模块长流程。

enum Skill { FIREBALL=1, RUSH=27, STAGED=73, THREE_RAYS=110, MULTIPLE=113, ORBIT=150 }

# 不带筛选运行全部用例；未知编号必须失败，避免空跑误报通过。
const TESTED_SKILLS := [1,27,73,110,113,150,408,409,410,411,412,413,414]
var selected_skill := 0

func selected(id: int) -> bool:
    return selected_skill==0 or selected_skill==id

func repeat_key(code: int) -> void:
    var event := InputEventKey.new()
    event.keycode=code
    event.physical_keycode=code
    event.pressed=true
    event.echo=true
    root.push_input(event)

func skill_record(id: int, binding: int) -> PackedByteArray:
    var raw := PackedByteArray()
    raw.resize(74)
    raw[4]=binding
    raw[5]=1
    raw.encode_u16(6,id)
    raw.encode_u32(49,0)
    raw[53]=10
    return encode_scene_bytes(raw)

func magic_reply(id: int, cell: Vector2i, target: int=0, actor: int=77) -> PackedByteArray:
    var packet := game_reply(Mir3HudView.SM_MAGICFIRE,actor,cell.x,cell.y,id)
    var body := PackedByteArray()
    body.resize(4)
    body.encode_u32(0,target)
    packet.append_array(encode_scene_bytes(body))
    return packet

func wait_until(predicate: Callable, duration: int=2000) -> bool:
    var deadline := Time.get_ticks_msec()+duration
    while Time.get_ticks_msec()<deadline:
        await process_frame
        if predicate.call(hud.hud_snapshot()): return true
    return false

func run_casting() -> void:
    for argument in skill_arguments():
        if argument.begins_with("--skill="):
            var value := argument.trim_prefix("--skill=")
            if not check(value.is_valid_int() and int(value) in TESTED_SKILLS and selected_skill==0,
                "--skill 必须是一个已有专项用例的编号："+str(TESTED_SKILLS)): return
            selected_skill=int(value)
    print("SKILL_SELECTION: "+("ALL" if selected_skill==0 else str(selected_skill)))
    Input.use_accumulated_input=false
    hud=load("res://main.tscn").instantiate()
    hud.set_meta("offline_preview",true)
    hud.set_meta("options_path","user://skill-casting-%d.ini" % OS.get_process_id())
    hud.set_connection_enabled(false)
    root.add_child(hud)
    await settle()
    hud.set_local_actor_identity(77)
    hud.set_character_vitals(100,100,200,200,0,100)
    # 避开默认出生格的建筑立柱，便于目视核对人物手部起手贴图。
    var origin: Vector2i=hud.hud_snapshot().player_cell+Vector2i(0,2)
    if not check(hud.change_map("0","比奇省",origin,255),"准备无遮挡施法格"): return
    if not check(await wait_until(func(s): return s.map_left_delay_ms==0),"等待换图输入门控结束"): return
    var remote := origin+Vector2i(0,2)
    var target := origin+Vector2i(4,0)
    var full := game_reply(Mir3HudView.SM_SENDMYMAGIC,0,0,0,3)
    full.append_array(skill_record(Skill.FIREBALL,49))
    full.append(47)
    full.append_array(skill_record(Skill.RUSH,65))
    full.append(47)
    full.append_array(skill_record(Skill.FIREBALL,66))
    full.append(47)
    if not check(hud.apply_game_message(full),"准备F1技能原包"): return
    var packets: Array[PackedByteArray]=[]
    hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(packet))
    var mouse := InputEventMouseMotion.new()
    mouse.position=Vector2(target*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(1,1)
    root.push_input(mouse,true)
    if selected(Skill.FIREBALL):
        hud.set_character_vitals(100,100,19,200,0,100)
        key(KEY_F1)
        if not check(packets.is_empty(),"MP低于门槛时不发施法请求"): return
        hud.set_character_vitals(100,100,20,200,0,100)
        key(KEY_F1)
        if not check(packets.size()==1 and hud.hud_snapshot().player_action==2,"F1仅发一个请求并进入定向起手"): return
        var request := decode_request(packets[0])
        if not check(request.decode_u16(4)==Mir3HudView.CM_SPELL and request.decode_u16(8)==Skill.FIREBALL,
            "请求使用3017且技能编号位置正确"): return
        key(KEY_F1)
        if not check(packets.size()==1,"冷却内重复按键不发包"): return
        # 等待进入起手中段，避免截到原资源刚出现的微弱首帧。
        if not check(await wait_until(func(s): return s.player_spell_frame>=3 and not s.drawn_human_spells.is_empty()),
            "本地起手实际绘制原资源帧"): return
        if not await capture("skill-casting-start"): return
        if not check(hud.apply_game_message("+FAIL".to_ascii_buffer()) and hud.hud_snapshot().player_cell==origin,
            "失败回包恢复预测原点"): return
    if selected(Skill.ORBIT):
        # 直接注入技能效果，单独验收表现，不重复等待每个技能的快捷键冷却。
        if not check(hud.apply_game_message(magic_reply(Skill.ORBIT,target)),"150回包进入队列"): return
        if not check(await wait_until(func(s): return s.spell_particle_count>0),"150产生可见粒子"): return
        await settle(0.12)
        if not await capture("skill-casting-orbit"): return
        if not check(await wait_until(func(s): return s.spell_particle_count==0,2500),"发射结束后粒子按寿命释放"): return
    if selected(Skill.STAGED):
        hud.change_map("0","比奇省",origin,1)
        if not check(hud.hud_snapshot().map_event_count==0 and hud.hud_snapshot().spell_particle_count==0,
            "换图清理技能事件与粒子"): return
        if not check(hud.apply_game_message(magic_reply(Skill.STAGED,target)),"73回包进入队列"): return
        if not check(await wait_until(func(s): return s.spell_effects.any(func(e): return e.skill==Skill.STAGED and e.stage==1)),
            "73独立进入移动阶段"): return
        if not await capture("skill-casting-staged"): return
    if selected(Skill.THREE_RAYS):
        hud.change_map("0","比奇省",origin,1)
        if not check(hud.apply_game_message(magic_reply(Skill.THREE_RAYS,target)),"110回包进入队列"): return
        if not check(await wait_until(func(s): return s.map_event_count==15),"三射线保留15段及重叠起点"): return
        if not check(hud.hud_snapshot().spell_effects.all(func(e): return e.ground_wave and e.emits_light),
            "三射线各段启用原资源光源"): return
        await settle(0.2)
        if not await capture("skill-casting-rays"): return
    if selected(Skill.FIREBALL):
        hud.change_map("0","比奇省",origin,1)
        if not check(hud.hud_snapshot().map_event_count==0,"地表技能换图无残留"): return
        # 远端起手与效果按同一人物队列执行；638必须等待起手动作完成。
        if not check(hud.set_scene_human(88,remote,0,0,"施法验收"),"准备远端施法者"): return
        var start := game_reply(Mir3HudView.SM_SPELL,88,target.x,target.y,0)
        start.append_array(str(Skill.FIREBALL).to_ascii_buffer())
        if not check(hud.apply_game_message(start),"远端原始十进制起手入队"): return
        if not check(await wait_until(func(s): return s.scene_human_actions[88].x==2),"远端启动定向动作"): return
        if not check(hud.hud_snapshot().scene_human_cells[88]==remote,"起手目标坐标不移动施法者"): return
        if not check(hud.apply_game_message(magic_reply(Skill.FIREBALL,origin,77,88)),"远端638效果入队"): return
        if not check(hud.hud_snapshot().map_event_count==0 and hud.hud_snapshot().scene_human_pending[88]==1,
            "638等待当前远端起手结束"): return
        await settle(0.2)
        if not check(not hud.hud_snapshot().drawn_human_spells.is_empty(),"远端起手实际绘制原资源帧"): return
        if not await capture("skill-casting-remote"): return
        if not check(await wait_until(func(s): return s.spell_effects.any(func(e): return e.skill==Skill.FIREBALL)),
            "远端起手结束后生成投射物"): return
    if selected(Skill.MULTIPLE):
        # 1410保留重复目标，跳过不在场目标，不将目标文本当二进制编号。
        hud.change_map("0","比奇省",origin,1)
        if not check(hud.set_scene_human(88,remote,0,0,"多目标验收"),"准备多目标人物"): return
        var multiple := game_reply(Mir3HudView.SM_MAGICFIRE_LIST,77,target.x,target.y,Skill.MULTIPLE)
        multiple.append_array(encode_scene_bytes("88/999999/88".to_ascii_buffer()))
        if not check(hud.apply_game_message(multiple),"1410目标列表入队"): return
        if not check(await wait_until(func(s): return s.spell_effects.size()==2),"重复目标保留两发、未知目标跳过"): return
        if not await capture("skill-casting-multiple"): return
    if selected(Skill.RUSH):
        hud.change_map("0","比奇省",origin,1)
        if not check(await wait_until(func(s): return s.map_left_delay_ms==0),"等待冲撞前换图限制结束"): return
        root.push_input(mouse,true)
        var packets_before := packets.size()
        repeat_key(KEY_F11)
        if not check(packets.size()==packets_before,"F11持续按键不进入施法"): return
        repeat_key(KEY_F10)
        if not check(packets.size()==packets_before+1 and hud.hud_snapshot().action_reply_pending and hud.hud_snapshot().walk_frame==-1
            and hud.hud_snapshot().player_cell==origin,"F10持续按键发冲撞请求并等待服务器位移"): return
        var rush_request := decode_request(packets.back())
        if not check(rush_request.decode_u32(0)==2 and rush_request.decode_u16(8)==Skill.RUSH
            and rush_request.decode_u16(6)==0 and rush_request.decode_u16(10)==0,"冲撞3017只发送朝向，不发送鼠标格或目标编号"): return
        var landing := origin+Vector2i(1,0)
        if not check(hud.apply_game_message("+GOOD".to_ascii_buffer()) and
            hud.apply_game_message(motion_reply(Mir3HudView.SM_RUSH,77,landing,2)),"服务器确认和冲撞位移入队"): return
        if not check(await wait_until(func(s): return s.forced_motion_opcode==Mir3HudView.SM_RUSH),"服务器回包启动冲撞动作"): return
        if not await capture("skill-casting-rush"): return
        if not check(await wait_until(func(s): return s.forced_motion_opcode==0 and s.player_cell==landing and not s.action_reply_pending)
            and packets.size()==packets_before+1,"冲撞落点、解除等待且不重复发包"): return
    if selected_skill==0 or selected_skill>=408:
        # 近战回包的高字节携带元素，七系附魔沿人物动作帧绘制。
        hud.change_map("0","比奇省",origin,1)
        if not check(hud.set_scene_human(88,remote,0,0,"附魔验收"),"准备附魔人物"): return
        if not check(hud.set_scene_human_attachments(88,0,1),"准备附魔武器外观"): return
        var elemental_colors := [Color8(240,180,100),Color8(100,140,190),Color8(50,70,200),
            Color8(140,200,150),Color8(200,200,200),Color8(160,100,180),Color8(200,220,130)]
        for element in range(7):
            if not selected(408+element): continue
            if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_HIT,88,remote.x,remote.y,2|(element<<8))),
                "七系附魔原包入队"): return
            if not check(await wait_until(func(s): return s.drawn_human_spells.any(func(e):
                return e.weapon and e.skill==408+element and e.relative_frame>=3 and e.tint.is_equal_approx(elemental_colors[element]))),
                "附魔效果编号、七系颜色及动作中段绘制正确"): return
            if element==2 or selected_skill!=0:
                if not await capture("skill-casting-enchantment"): return
            if not check(await wait_until(func(s): return s.scene_human_actions[88].x==-1),"附魔动作结束"): return
    if selected(Skill.FIREBALL):
        # 完整等待八秒，确认施法等待超时后可恢复输入，动画结束不能提前解锁。
        if not check(await wait_until(func(s): return s.map_left_delay_ms==0),"超时验收换图门控结束"): return
        hud.change_map("0","比奇省",origin,1)
        if not check(await wait_until(func(s): return s.map_left_delay_ms==0),"独立超时用例门控结束"): return
        root.push_input(mouse,true)
        key(KEY_F1)
        if not check(hud.hud_snapshot().action_reply_pending,"施法进入等待确认"): return
        await settle(1.3)
        if not check(hud.hud_snapshot().action_reply_pending,"起手结束仍等待服务器"): return
        if not check(await wait_until(func(s): return not s.action_reply_pending,7500),"八秒超时解除施法等待"): return
        if not await capture("skill-casting-timeout"): return
        hud.apply_game_message("+FAIL".to_ascii_buffer())
        if not check(hud.hud_snapshot().player_cell==origin,"迟到拒绝恢复保存的施法原点"): return
    hud.queue_free()
    await settle()
    print("SKILL_CASTING_UI_OK selection="+("ALL" if selected_skill==0 else str(selected_skill)))
    return


# 场景：enchantment
# 本地武器附魔：装备原包来源、特殊覆盖色、普通元素、无强度关闭及动作结束清理。

func weapon_packet(element: int, power: int, special: int) -> PackedByteArray:
    var raw := PackedByteArray()
    raw.resize(100)
    raw[0]=83 # Sword
    raw[30]=5
    raw.encode_u16(36,1043)
    raw[49]=element
    raw.encode_u16(50,power)
    raw.encode_u32(62,101)
    raw[97]=special
    var packet := game_reply(Mir3HudView.SM_SENDUSEITEMS,0,0,0,1)
    packet.append_array("1/".to_ascii_buffer())
    packet.append_array(encode_scene_bytes(raw))
    packet.append(47)
    return packet

func run_enchantment() -> void:
    Input.use_accumulated_input=false
    var ids := [408,409,410,411,412,413,414]
    for argument in skill_arguments():
        if argument.begins_with("--skill=") or argument.begins_with("--skills="):
            var values := argument.split("=",true,1)[1].split(",")
            if not check(Array(values).all(func(n): return n.is_valid_int() and int(n) in ids),"本地附魔编号必须在408至414"): return
            ids=Array(values).map(func(n): return int(n))
    hud=load("res://main.tscn").instantiate()
    hud.set_meta("offline_preview",true)
    hud.set_meta("options_path","user://weapon-enchantment-%d.ini" % OS.get_process_id())
    hud.set_connection_enabled(false)
    root.add_child(hud)
    await settle()
    hud.set_local_actor_identity(77)
    hud.set_character_vitals(100,100,200,200,0,100)
    var origin: Vector2i=hud.hud_snapshot().player_cell+Vector2i(0,2)
    hud.change_map("0","比奇省",origin,1)
    if not check(await wait_until(func(s): return s.map_left_delay_ms==0),"等待附魔地图门控"): return
    hud.client_packet_ready.connect(func(_packet): hud.apply_game_message("+GOOD".to_ascii_buffer()))
    var colors := [Color8(240,180,100),Color8(100,140,190),Color8(50,70,200),
        Color8(140,200,150),Color8(200,200,200),Color8(160,100,180),Color8(200,220,130)]
    for effect in ids:
        var element: int=effect-407
        # 普通附魔、特殊附魔覆盖不同底色、特殊字段有符号负值退回普通附魔。
        for values in [[element,1,0],[element%7+1,0,element],[element,256,128]]:
            if not check(hud.apply_game_message(weapon_packet(values[0],values[1],values[2])),"注入武器槽原包"): return
            var pointer := Vector2((origin+Vector2i(1,0))*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(1,1)
            attack_click(pointer,true)
            if not check(await wait_until(func(s): return s.drawn_human_spells.any(func(e):
                return e.weapon and e.skill==effect and e.relative_frame>=3 and e.tint.is_equal_approx(colors[element-1]))),
                "本地附魔读取装备字段并按优先级选色"): return
            if values[2]==element and not await capture("skill-local-enchantment-%d" % effect): return
            if values[2]==element:
                # 附魔挥击后排队的碰撞折返不能沿用上一次武器贴图。
                if not check(hud.apply_game_message(motion_reply(Mir3HudView.SM_BUMP,77,origin+Vector2i(1,0),2)),"附魔期间碰撞回包入队"): return
                if not check(await wait_until(func(s): return s.player_action==7 and s.forced_motion_opcode==0),"附魔后进入碰撞折返动作"): return
                await process_frame
                if not check(hud.hud_snapshot().drawn_human_spells.is_empty(),"碰撞折返不残留武器附魔"): return
            if not check(await wait_until(func(s): return s.player_action==-1),"等待本地挥击结束"): return
            await settle(1.5) # 等级1的普通挥击准入间隔接近1400ms，不能过早再次点击。
    # 有元素但强度WORD为0时，不产生普通附魔；上一次附魔不得残留。
    if not check(hud.apply_game_message(weapon_packet(3,0,0)),"清除附魔强度"): return
    attack_click(Vector2((origin+Vector2i(1,0))*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(1,1),true)
    if not check(await wait_until(func(s): return s.player_action>=0),"无附魔时仍有挥击动作"): return
    await settle(0.2)
    if not check(hud.hud_snapshot().drawn_human_spells.is_empty(),"普通强度为零不残留附魔"): return
    hud.queue_free()
    await settle()
    print("LOCAL_WEAPON_ENCHANTMENT_OK "+str(ids))
    return


# 场景：enchantment-visual
# 同帧像素验收：冻结挥击后比较真实着色器与仅返回背景的基线，避免只看绘制元数据。

func run_enchantment_visual() -> void:
    var ids := [408,409,410,411,412,413,414]
    for arg in skill_arguments():
        if arg.begins_with("--skill="):
            var value := arg.trim_prefix("--skill=")
            if not check(value.is_valid_int() and int(value) in ids,"像素验收附魔编号有效"): return
            ids=[int(value)]
    Input.use_accumulated_input=false
    hud=load("res://main.tscn").instantiate()
    hud.set_meta("offline_preview",true)
    hud.set_meta("options_path","user://enchant-pixels-%d.ini" % OS.get_process_id())
    hud.set_connection_enabled(false)
    root.add_child(hud);await settle()
    hud.set_local_actor_identity(77)
    hud.set_character_vitals(100,100,200,200,0,100)
    var origin: Vector2i=hud.hud_snapshot().player_cell+Vector2i(0,2)
    hud.client_packet_ready.connect(func(_p): hud.apply_game_message("+GOOD".to_ascii_buffer()))
    var shader: Shader=load("res://level_effect.gdshader")
    var original: String=shader.code
    var colors := [Vector3(240,180,100),Vector3(100,140,190),Vector3(50,70,200),Vector3(140,200,150),Vector3(200,200,200),Vector3(160,100,180),Vector3(200,220,130)]
    for id in ids:
        hud.change_map("0","比奇省",origin,1)
        await wait_until(func(s): return s.map_left_delay_ms==0)
        hud.apply_game_message(weapon_packet(1,0,id-407))
        attack_click(Vector2((origin+Vector2i(1,0))*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(1,1),true)
        var effect: Dictionary={}
        for i in range(120):
            await RenderingServer.frame_post_draw
            var visible: Array=hud.hud_snapshot().drawn_human_spells.filter(func(e): return e.weapon and e.skill==id and e.relative_frame==3)
            if not visible.is_empty():
                effect=visible[0];break
        if not check(not effect.is_empty(),"取得实际第三动作帧"): return
        hud.set_process(false)
        var actual: Image=root.get_texture().get_image()
        var area := Rect2i(effect.bounds).intersection(Rect2i(0,0,800,600))
        # 仅在测试进程暂换基线；恢复原着色器后才断言，不修改磁盘着色器或生产数据。
        shader.code="shader_type canvas_item; render_mode unshaded,blend_disabled; uniform sampler2D background: hint_screen_texture,repeat_disable,filter_nearest; void fragment(){if(texture(TEXTURE,UV).a==0.0) discard;COLOR=vec4(texture(background,SCREEN_UV).rgb,1.0);}"
        hud.queue_redraw();await process_frame;await RenderingServer.frame_post_draw
        var baseline: Image=root.get_texture().get_image()
        shader.code=original
        hud.queue_redraw();await process_frame;await RenderingServer.frame_post_draw
        hud.set_process(true)
        var changed := 0
        var energy := Vector3.ZERO
        for y in range(area.position.y,area.end.y):
            for x in range(area.position.x,area.end.x):
                var a := actual.get_pixel(x,y)
                var b := baseline.get_pixel(x,y)
                var delta := Vector3(a.r-b.r,a.g-b.g,a.b-b.b)
                if delta.length()>0.02:
                    changed+=1
                    energy+=Vector3(maxf(delta.x,0)/maxf(1-b.r,0.05),maxf(delta.y,0)/maxf(1-b.g,0.05),maxf(delta.z,0)/maxf(1-b.b,0.05))
        var normalized := energy/maxf(energy.x+energy.y+energy.z,0.001)
        var palette: Vector3=colors[id-408]
        var expected := palette/(palette.x+palette.y+palette.z)
        print("ENCHANT_PIXELS ",id," changed=",changed," rgb=",normalized," expected=",expected)
        if not check(changed>=100,"附魔至少改变100个实际视口像素"): return
        if not check((normalized-expected).length()<0.15,"实际混合色保持预期色系（含素材和光照容差）"): return
        if skill_arguments().has("--screenshots"):
            var path := ProjectSettings.globalize_path("res://../build/screenshots/skill-enchantment-pixels-%d.png" % id)
            if not check(actual.save_png(path)==OK,"保存未改写着色器的原始截图"): return
            print("SCREENSHOT: ",path)
        if not check(await wait_until(func(s): return s.player_action==-1),"挥击结束"): return
        await settle(1.5)
    hud.queue_free();await settle()
    print("ENCHANTMENT_PIXELS_UI_OK ",ids);return


# 场景：correction
func run_correction() -> void:
    hud=load("res://main.tscn").instantiate()
    hud.set_meta("offline_preview",true)
    hud.set_connection_enabled(false)
    root.add_child(hud)
    await settle()
    hud.set_local_actor_identity(77)
    hud.set_character_vitals(100,100,0,0,0,100)
    var origin: Vector2i=hud.hud_snapshot().player_cell
    var packets: Array[PackedByteArray]=[]
    hud.client_packet_ready.connect(func(packet: PackedByteArray): packets.append(packet))
    var pointer := Vector2(origin*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(49,1)
    attack_click(pointer,true)
    if not check(packets.size()==1,"先发起本地攻击"): return
    if not check(not hud.apply_game_message(motion_reply(Mir3HudView.SM_RUSH,77,origin,8)),"拒绝非法方向"): return
    if not check(not hud.apply_game_message(motion_reply(Mir3HudView.SM_RUSH,77,Vector2i(65535,65535),4)),"拒绝越界落点"): return
    var cases := [
        motion_reply(Mir3HudView.SM_RUSH,77,origin+Vector2i(0,2),4),
        motion_reply(Mir3HudView.SM_BUMP,77,origin+Vector2i(1,2),2),
        motion_reply(Mir3HudView.SM_BACKSTEP,77,origin+Vector2i(0,1),4),
        motion_reply(Mir3HudView.SM_REVIVE_REFRESH,999,origin+Vector2i(0,2),4,1)]
    for packet in cases:
        if not check(hud.apply_game_message(packet),"完整服务器位移原包入队"): return
    if not check(hud.hud_snapshot().forced_motion_queue==4 and hud.hud_snapshot().walk_frame==-1
        and hud.hud_snapshot().player_action==9,"服务器位移等待当前挥击结束"): return
    var seen := {}
    var saw_return := false
    var deadline := Time.get_ticks_msec()+4000
    while Time.get_ticks_msec()<deadline:
        await process_frame
        var state: Dictionary=hud.hud_snapshot()
        var opcode: int=state.forced_motion_opcode
        if opcode!=0:
            seen[opcode]=true
        if opcode==Mir3HudView.SM_RUSH and state.walk_frame==2 and not seen.has("rush-shot"):
            seen["rush-shot"]=true
            if not await capture("forced-motion-rush"): return
        if opcode==Mir3HudView.SM_BACKSTEP and state.walk_frame==2 and not seen.has("back-shot"):
            seen["back-shot"]=true
            if not check(state.player_facing==4 and state.map_step_destination==origin+Vector2i(0,1),"后退保留南向而向北移动"): return
            if not await capture("forced-motion-backstep"): return
        if opcode==0 and state.player_action==7:
            saw_return=state.player_cell==origin+Vector2i(0,2)
        if seen.has(Mir3HudView.SM_REVIVE_REFRESH) and opcode==0 and state.forced_motion_queue==0:
            break
    var state: Dictionary=hud.hud_snapshot()
    if not check(seen.has(Mir3HudView.SM_RUSH) and seen.has(Mir3HudView.SM_BUMP)
        and seen.has(Mir3HudView.SM_BACKSTEP) and seen.has(Mir3HudView.SM_REVIVE_REFRESH)
        and saw_return,"依次执行四类位移且碰撞退回推导起点"): return
    if not check(state.player_cell==origin+Vector2i(0,2) and state.player_mount==1
        and not state.action_reply_pending and packets.size()==1,"强制骑乘走步落点正确、解除确认且不反发动作请求"): return
    if not await capture("forced-motion-arrival"): return
    hud.queue_free()
    await settle()
    print("FORCED_MOTION_UI_OK")
    return



# 场景：cast-correction
# 施法纠偏组合：碰撞排队、折返落点、确认不截断起手、拒绝清理及换图后迟到拒绝。

func begin_fireball(origin: Vector2i) -> bool:
    if not hud.change_map("0","比奇省",origin,1): return false
    if not await wait_until(func(s): return s.map_left_delay_ms==0): return false
    var pointer := InputEventMouseMotion.new()
    pointer.position=Vector2((origin+Vector2i(4,0))*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(1,1)
    root.push_input(pointer,true)
    key(KEY_F1)
    return hud.hud_snapshot().action_reply_pending and hud.hud_snapshot().player_action==2

func run_cast_correction() -> void:
    Input.use_accumulated_input=false
    hud=load("res://main.tscn").instantiate()
    hud.set_meta("offline_preview",true)
    hud.set_meta("options_path","user://spell-correction-%d.ini" % OS.get_process_id())
    hud.set_connection_enabled(false)
    root.add_child(hud)
    await settle()
    hud.set_local_actor_identity(77)
    hud.set_character_vitals(100,100,200,200,0,100)
    var packet := game_reply(Mir3HudView.SM_SENDMYMAGIC,0,0,0,1)
    packet.append_array(skill_record(Skill.FIREBALL,49))
    packet.append(47)
    if not check(hud.apply_game_message(packet),"准备纠偏用火球技能"): return
    var origin: Vector2i=hud.hud_snapshot().player_cell+Vector2i(0,2)
    var packets: Array[PackedByteArray]=[]
    hud.client_packet_ready.connect(func(value): packets.append(value))
    if not check(await begin_fireball(origin),"开始等待确认的施法"): return
    if not check(hud.apply_game_message(motion_reply(Mir3HudView.SM_BUMP,77,origin+Vector2i(1,0),2)),"施法期间碰撞原包入队"): return
    var state: Dictionary=hud.hud_snapshot()
    if not check(state.forced_motion_queue==1 and state.forced_motion_opcode==0 and state.player_action==2,
        "碰撞等待当前施法起手，不立即移动"): return
    if not check(await wait_until(func(s): return s.forced_motion_opcode==Mir3HudView.SM_BUMP),"起手结束后开始服务器碰撞"): return
    if not check(await wait_until(func(s): return s.forced_motion_opcode==0 and s.player_action==7),"碰撞结束折返受阻动作"): return
    state=hud.hud_snapshot()
    if not check(state.player_cell==origin and not state.action_reply_pending and state.drawn_human_spells.is_empty()
        and packets.size()==1,"折返推导原点、解除等待、起手特效无残留且不反发请求"): return
    if not await capture("skill-correction-bump"): return
    if not check(await begin_fireball(origin),"重新准备确认与拒绝组合"): return
    if not check(hud.apply_game_message("+GOOD".to_ascii_buffer()) and not hud.hud_snapshot().action_reply_pending
        and hud.hud_snapshot().player_action==2,"确认解除等待但不截断施法动作"): return
    if not check(hud.apply_game_message("+FAIL".to_ascii_buffer()) and hud.hud_snapshot().player_cell==origin
        and hud.hud_snapshot().player_action==-1,"后续拒绝恢复起手前状态"): return
    await process_frame
    if not check(hud.hud_snapshot().drawn_human_spells.is_empty(),"拒绝后清除已绘制起手"): return
    if not check(await begin_fireball(origin),"开始跨地图拒绝场景"): return
    var new_origin := origin+Vector2i(0,2)
    if not check(hud.change_map("0","比奇省",new_origin,1) and hud.apply_game_message("+FAIL".to_ascii_buffer()),"换图后接收旧拒绝"): return
    state=hud.hud_snapshot()
    if not check(state.player_cell==new_origin and not state.action_reply_pending and state.player_action==-1
        and state.forced_motion_queue==0 and state.map_event_count==0,"旧拒绝不能跨地图回滚，动作与效果清空"): return
    if not await capture("skill-correction-map"): return
    hud.queue_free()
    await settle()
    print("SPELL_CORRECTION_UI_OK")
    return


# 场景：ranges
# 范围专项：9/53九段地表波、10九段闪电尾迹；逐格、延迟、光源与创建音效。

enum RangeSkill { FIRE_WAVE=9, LIGHTNING=10, ICE_WAVE=53 }

func run_ranges() -> void:
    var ids := [RangeSkill.FIRE_WAVE,RangeSkill.LIGHTNING,RangeSkill.ICE_WAVE]
    for argument in skill_arguments():
        if argument.begins_with("--skill="):
            var value := argument.trim_prefix("--skill=")
            if not check(value.is_valid_int() and int(value) in ids,"范围子集只接受9、10、53"): return
            ids=[int(value)]
    hud=load("res://main.tscn").instantiate()
    hud.set_meta("offline_preview",true)
    hud.set_meta("options_path","user://spell-ranges-%d.ini" % OS.get_process_id())
    hud.set_connection_enabled(false)
    root.add_child(hud)
    await settle()
    hud.set_local_actor_identity(77)
    var origin: Vector2i=hud.hud_snapshot().player_cell+Vector2i(0,2)
    for skill in ids:
        # 正东和接近正北的斜目标验证方向归一化，目标远近不截断九段表现。
        for offset in [Vector2i(4,0),Vector2i(1,-5)]:
            if not check(hud.change_map("0","比奇省",origin,1),"清理上个范围用例"): return
            if not check(hud.apply_game_message(magic_reply(skill,origin+offset)),"范围技能回包入队"): return
            if not check(await wait_until(func(s): return s.spell_effects.filter(func(e): return e.ground_wave).size()==9),
                "创建九段地表效果 skill=%d offset=%s states=%s" % [skill,offset,hud.hud_snapshot().spell_effects]): return
            var state: Dictionary=hud.hud_snapshot()
            var waves: Array=state.spell_effects.filter(func(e): return e.ground_wave)
            var step := Vector2i(1,0) if offset.y==0 else Vector2i(0,-1)
            var effect: int=336 if skill==RangeSkill.LIGHTNING else skill
            var interval: int=30 if skill==RangeSkill.LIGHTNING else 150
            var waiting_segment := false
            for i in range(9):
                var matches := waves.filter(func(e): return e.cell==origin+step*(i+1))
                if not check(matches.size()==1 and matches[0].skill==effect and matches[0].emits_light,
                    "九段逐格范围、资源编号与光源"): return
                # delay字段保存初始延迟，触发后归零，并非逐帧递减的剩余时间。
                if matches[0].delay>0:
                    waiting_segment=true
                    if not check(matches[0].delay==interval*i,"未触发段保留150ms或30ms间隔"): return
                elif not check(not waiting_segment,"九段按距离顺序触发，不能越过等待中的前段"): return
            # 原SoundList：10092为空，10102/13362无记录；只有10532映射m53-3.wav。
            if skill==RangeSkill.ICE_WAVE:
                if not check(state.scene_actor_sounds.values().any(func(s): return s.sample==10532),
                    "冰波创建使用原表m53-3.wav"): return
            elif not check(state.scene_actor_sounds.is_empty(),"空或缺失的创建音效记录保持静音"): return
            await settle(0.25)
            if skill==RangeSkill.LIGHTNING and not check(hud.hud_snapshot().spell_particle_count>0,"336零间隔尾迹触发可见粒子"): return
            if skill==RangeSkill.ICE_WAVE:
                if not check(await wait_until(func(s): return s.map_event_particle_count>0),"冰波发射共享光点与碎片"): return
            if offset.y==0 and not await capture("skill-range-%d" % skill): return
    hud.change_map("0","比奇省",origin,1)
    if not check(hud.hud_snapshot().spell_effects.is_empty() and hud.hud_snapshot().spell_particle_count==0,"范围技能离场清空"): return
    hud.queue_free()
    await settle()
    print("SPELL_RANGES_UI_OK "+str(ids))
    return


# 场景：common
# 共用施法批次：独立原版元数据期望；按编号验证请求、起手、范围、资源、声光及清理。

func sound_matches(expected: Dictionary) -> bool:
    var found: bool=hud.hud_snapshot().scene_actor_sounds.values().any(func(a): return a.sample==int(expected.id))
    return found==bool(expected.exists)

func run_common() -> void:
    var fixture_path: String= get_script().resource_path.get_base_dir().path_join("skill_common_cases.json")
    var fixture: Dictionary=JSON.parse_string(FileAccess.get_file_as_string(fixture_path))
    var cases: Array=fixture.cases
    for argument in skill_arguments():
        if argument.begins_with("--skill=") or argument.begins_with("--skills="):
            var values := argument.split("=",true,1)[1].split(",")
            var ids: Array=Array(values).map(func(n): return int(n))
            if not check(Array(values).all(func(n): return n.is_valid_int()) and ids.all(func(n): return cases.any(func(c): return int(c.id)==n)),"共用批次技能编号有效"): return
            cases=cases.filter(func(c): return int(c.id) in ids)
    Input.use_accumulated_input=false
    hud=load("res://main.tscn").instantiate()
    hud.set_meta("offline_preview",true)
    hud.set_meta("options_path","user://spell-common-%d.ini" % OS.get_process_id())
    hud.set_connection_enabled(false)
    root.add_child(hud)
    await settle()
    hud.set_local_actor_identity(77)
    hud.set_character_vitals(100,100,200,200,0,100)
    var origin: Vector2i=hud.hud_snapshot().player_cell+Vector2i(0,2)
    var target := origin+Vector2i(0,2)
    var packets: Array[PackedByteArray]=[]
    hud.client_packet_ready.connect(func(packet: PackedByteArray):
        packets.append(packet)
        hud.apply_game_message("+GOOD".to_ascii_buffer()))
    for c in cases:
        var id := int(c.id)
        print("COMMON_CASE_START %d" % id)
        if not check(hud.change_map("0","比奇省",origin,1),"准备独立技能状态"): return
        if not check(hud.set_scene_human(89,target,0,0,"法术目标"),"准备可跟随目标"): return
        var full := game_reply(Mir3HudView.SM_SENDMYMAGIC,0,0,0,1)
        full.append_array(skill_record(id,49));full.append(47)
        if not check(hud.apply_game_message(full),"F1技能记录原包"): return
        if not check(await wait_until(func(s): return s.map_left_delay_ms==0),"换图输入门控解除"): return
        var mouse := InputEventMouseMotion.new()
        mouse.position=Vector2(target*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(1,1)
        root.push_input(mouse,true)
        packets.clear();key(KEY_F1)
        if not check(packets.size()==1,"技能%d只发一个请求" % id): return
        var request := decode_request(packets[0])
        if not check(request.decode_u16(4)==Mir3HudView.CM_SPELL and request.decode_u16(8)==id,"技能请求编号与原版字段"): return
        if not check(hud.hud_snapshot().player_action==int(c.action),"技能%d本地动作路由" % id): return
        if id in [31,122] and not check(Vector2i(request.decode_u16(0),request.decode_u16(2))==origin,"自身目标例外"): return
        if c.spell!=null and int(c.action) in [2,3]:
            if c.spell_image:
                if not check(await wait_until(func(s): return s.drawn_human_spells.any(func(e): return e.skill==id and e.relative_frame>=1)),"技能%d起手实际贴图" % id): return
                var drawn: Dictionary=hud.hud_snapshot().drawn_human_spells.filter(func(e): return e.skill==id)[0]
                var offset := 40 if int(c.action)==2 else 0
                if not check(drawn.frame==int(c.spell[0])+offset+drawn.relative_frame,"起手资源首帧、方向步长与相对帧"): return
            else:
                await settle(0.1)
                if not check(hud.hud_snapshot().drawn_human_spells.is_empty(),"原WIX空帧不伪造起手贴图"): return
            if not check(sound_matches(c.spell_sound),"技能%d起手声音与原表/文件一致" % id): return
            if not check(hud.hud_snapshot().human_spell_light_count>0,"起手光源存在"): return
        if not check(await wait_until(func(s): return s.player_action==-1,4000),"起手动作正常结束"): return
        if not check(hud.apply_game_message(magic_reply(id,target,89)),"638目标原包"): return
        var expected_effect: bool=c.reply=="effect" and c.magic!=null and int(c.magic[1])>int(c.magic[0]) and int(c.magic[2])>0
        if expected_effect:
            if not check(await wait_until(func(s): return s.spell_effects.any(func(e): return e.skill==id)),"技能%d创建施放效果" % id): return
            var effect: Dictionary=hud.hud_snapshot().spell_effects.filter(func(e): return e.skill==id)[0]
            if not check(effect.library==int(c.magic[3]) and effect.first==int(c.magic[0]) and effect.end==int(c.magic[1]) and effect.interval==int(c.magic[2]),"逐号资源库、帧段和帧间隔"): return
            if id!=74:
                if not check(effect.projectile==bool(c.projectile) and effect.repeat==bool(c.repeat),"飞行/定点与循环规则"): return
                if not check(effect.target_actor==(89 if c.track else 0),"目标绑定规则"): return
                if not check(effect.cell==(origin if c.projectile else target),"单体范围落在施法者或目标格"): return
            if not check(effect.emits_light==(bool(c.light) and int(c.magic[6])>0),"逐号光源启用规则"): return
            if not check(sound_matches(c.magic_sound),"技能%d施放音效与原表/文件一致" % id): return
            if cases.size()==1 or id in [2,5,11,14,47,74,85,102,111,122]:
                await settle(0.08)
                if not await capture("skill-common-%d" % id): return
            if c.projectile and c.explosion!=null:
                if not check(await wait_until(func(s): return s.spell_effects.any(func(e): return e.skill==id and e.first==int(c.explosion[0])),3000),"技能%d转入原命中帧段" % id): return
        else:
            await settle(0.05)
            if not check(hud.hud_snapshot().spell_effects.is_empty(),"被忽略/无MAGIC资源的回包不凭空生成效果"): return
        # 远端起手复用同资源，但30号明确使用动作3。
        if not check(hud.set_scene_human(88,origin+Vector2i(0,-1),0,0,"远端施法者"),"准备远端人物"): return
        var start := game_reply(Mir3HudView.SM_SPELL,88,target.x,target.y,0)
        start.append_array(str(id).to_ascii_buffer())
        if not check(hud.apply_game_message(start),"远端十进制技能起手"): return
        var remote_action := 2 if int(c.action)==2 and id!=30 else 3
        if not check(await wait_until(func(s): return s.scene_human_actions[88].x==remote_action),"远端逐技能动作路由"): return
        print("COMMON_CASE_OK %d" % id)
    if not check(hud.change_map("0","比奇省",origin,1),"批次离场换图成功"): return
    if not hud.hud_snapshot().scene_actor_sounds.is_empty():
        print("COMMON_EXIT_REMAINING_SOUNDS "+str(hud.hud_snapshot().scene_actor_sounds))
    if not check(hud.hud_snapshot().spell_effects.is_empty() and hud.hud_snapshot().scene_actor_sounds.is_empty(),"批次离场释放技能效果与声音"): return
    hud.queue_free()
    await settle()
    print("SPELL_COMMON_UI_OK "+str(cases.map(func(c): return int(c.id))))
    return


# 场景：passive
# 3/4号客户端职责：原版418D80无主动施法分支；保留学习/修炼显示，普通攻击不转技能。

func run_passive() -> void:
    var ids := [3,4]
    for arg in skill_arguments():
        if arg.begins_with("--skill="):
            var value := arg.trim_prefix("--skill=")
            if not check(value.is_valid_int() and int(value) in ids,"被动技能编号有效"): return
            ids=[int(value)]
    Input.use_accumulated_input=false
    hud=load("res://main.tscn").instantiate()
    hud.set_meta("offline_preview",true)
    hud.set_meta("options_path","user://passive-skills-%d.ini" % OS.get_process_id())
    hud.set_connection_enabled(false)
    root.add_child(hud)
    await settle()
    hud.set_local_actor_identity(77)
    hud.set_character_vitals(100,100,200,200,0,100)
    var origin: Vector2i=hud.hud_snapshot().player_cell+Vector2i(0,2)
    var packets: Array[PackedByteArray]=[]
    hud.client_packet_ready.connect(func(p: PackedByteArray):
        packets.append(p)
        hud.apply_game_message("+GOOD".to_ascii_buffer()))
    for id in ids:
        hud.change_map("0","比奇省",origin,1)
        var full := game_reply(Mir3HudView.SM_SENDMYMAGIC,0,0,0,1)
        full.append_array(skill_record(id,49));full.append(47)
        if not check(hud.apply_game_message(full),"被动技能学习原包"): return
        if not check(hud.hud_snapshot().skill_bindings[1]==id and hud.hud_snapshot().learned_skills.size()==1,"被动记录与快捷表保留"): return
        if not check(hud.apply_game_message(game_reply(Mir3HudView.SM_MAGIC_LVEXP,id,2,215,0)),"被动修炼更新"): return
        var learned: Dictionary=hud.hud_snapshot().learned_skills[0]
        if not check(learned.id==id and learned.level==2 and learned.experience==215,"修炼值显示来自服务器原包"): return
        if not check(await wait_until(func(s): return s.map_left_delay_ms==0),"等待地图门控"): return
        var pointer := Vector2((origin+Vector2i(1,0))*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(1,1)
        var motion := InputEventMouseMotion.new()
        motion.position=pointer;root.push_input(motion,true)
        # 前项普通挥击可能仍留声音计数，核对本次没有新增声音。
        var sounds_before: int=hud.hud_snapshot().scene_actor_sounds.values().reduce(func(n,a): return n+a.count,0)
        packets.clear();key(KEY_F1)
        await settle(0.25)
        if not check(packets.is_empty() and hud.hud_snapshot().player_action==-1,"被动快捷键不发主动请求或动作"): return
        if not check(hud.hud_snapshot().spell_effects.is_empty() and hud.hud_snapshot().drawn_human_spells.is_empty() and hud.hud_snapshot().human_spell_light_count==0 and hud.hud_snapshot().scene_actor_sounds.values().reduce(func(n,a): return n+a.count,0)==sounds_before,"被动不生成法术贴图声光"): return
        attack_click(pointer,true)
        if not check(await wait_until(func(_s): return not packets.is_empty()),"被动记录不阻止普通挥击"): return
        if not check(decode_request(packets[0]).decode_u16(4)==Mir3HudView.CM_HIT,"普通攻击仍用CM_HIT，不伪造被动伤害结算"): return
        if not check(await wait_until(func(s): return s.player_action==-1),"普通动作结束"): return
        key(KEY_E)
        await settle()
        if not await capture("skill-passive-%d" % id): return
        key(KEY_E)
        print("PASSIVE_CASE_OK %d" % id)
    hud.queue_free();await settle()
    print("SPELL_PASSIVE_UI_OK "+str(ids));return


# 场景：special
# 特殊路径补证：74完整阶段/固定目标偏移；36/104/107/108确认、拒绝和强制纠偏。

func run_special() -> void:
    var ids := [74,36,104,107,108]
    for argument in skill_arguments():
        if argument.begins_with("--skill=") or argument.begins_with("--skills="):
            var values := argument.split("=",true,1)[1].split(",")
            if not check(Array(values).all(func(n): return n.is_valid_int() and int(n) in ids),"特殊技能筛选有效"): return
            ids=Array(values).map(func(n): return int(n))
    Input.use_accumulated_input=false
    hud=load("res://main.tscn").instantiate()
    hud.set_meta("offline_preview",true)
    hud.set_meta("options_path","user://spell-special-%d.ini" % OS.get_process_id())
    hud.set_connection_enabled(false)
    root.add_child(hud);await settle()
    hud.set_local_actor_identity(77)
    hud.set_character_vitals(100,100,200,200,0,100)
    var origin: Vector2i=hud.hud_snapshot().player_cell+Vector2i(0,2)
    var target := origin+Vector2i(0,2)
    var packets: Array[PackedByteArray]=[]
    hud.client_packet_ready.connect(func(p: PackedByteArray): packets.append(p))
    for id in ids:
        hud.change_map("0","比奇省",origin,1)
        if id==74:
            if not check(hud.set_scene_human(89,target,0,0,"74目标"),"准备阶段目标"): return
            if not check(hud.apply_game_message(magic_reply(id,target,89)),"74回包入队"): return
            if not check(await wait_until(func(s): return s.spell_effects.any(func(e): return e.skill==74 and e.stage==1)),"74进入移动阶段"): return
            var moving: Dictionary=hud.hud_snapshot().spell_effects.filter(func(e): return e.skill==74)[0]
            if not check(moving.cell==target and moving.stage_offset==Vector2i(0,10) and moving.first==1980 and moving.end==1995,"74固定目标格和10像素偏移、原帧段"): return
            if not check(hud.set_scene_human(89,target+Vector2i(0,2),0,0,"移动目标"),"确认74目标实际换格"): return
            await settle(0.2)
            moving=hud.hud_snapshot().spell_effects.filter(func(e): return e.skill==74)[0]
            if not check(moving.cell==target and moving.stage_offset==Vector2i(0,10),"74不跟随目标移动或套用73轨迹"): return
            if not await capture("skill-special-74-moving"): return
            if not check(await wait_until(func(s): return s.spell_effects.any(func(e): return e.skill==74 and e.stage==2),2000),"74转入爆炸"): return
            var impact: Dictionary=hud.hud_snapshot().spell_effects.filter(func(e): return e.skill==74)[0]
            if not check(impact.first==2000 and impact.end==2010 and impact.interval==100 and impact.stage_offset==Vector2i(0,10),"74爆炸资源与位置保持"): return
            if not await capture("skill-special-74-impact"): return
            if not check(await wait_until(func(s): return s.spell_effects.is_empty(),5000),"74爆炸及光尾最终释放"): return
        else:
            var packet := game_reply(Mir3HudView.SM_SENDMYMAGIC,0,0,0,1)
            packet.append_array(skill_record(id,49));packet.append(47)
            if not check(hud.apply_game_message(packet),"特殊技能原记录"): return
            if not check(await wait_until(func(s): return s.map_left_delay_ms==0),"等待地图门控"): return
            var pointer := InputEventMouseMotion.new()
            pointer.position=Vector2(target*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(1,1)
            root.push_input(pointer,true)
            packets.clear();key(KEY_F1)
            var action := 18 if id==36 else (2 if id==108 else 3)
            if not check(packets.size()==1 and hud.hud_snapshot().player_action==action and hud.hud_snapshot().action_reply_pending,"特殊动作并等待确认"): return
            if not check(hud.hud_snapshot().player_cell==origin,"无服务器位移回包前不自行换位"): return
            if not check(hud.apply_game_message("+GOOD".to_ascii_buffer()) and hud.hud_snapshot().player_action==action and not hud.hud_snapshot().action_reply_pending,"GOOD不截断特殊起手"): return
            if not check(hud.apply_game_message(motion_reply(Mir3HudView.SM_BUMP,77,origin+Vector2i(1,0),2)),"特殊起手期间碰撞入队"): return
            if not check(hud.hud_snapshot().forced_motion_queue==1 and hud.hud_snapshot().player_action==action,"碰撞等待当前动作完成"): return
            if not check(await wait_until(func(s): return s.forced_motion_opcode==Mir3HudView.SM_BUMP,4000),"特殊起手完成后推进碰撞"): return
            if not check(await wait_until(func(s): return s.forced_motion_opcode==0 and s.player_action==7),"碰撞折返"): return
            if not check(hud.hud_snapshot().player_cell==origin and hud.hud_snapshot().drawn_human_spells.is_empty() and packets.size()==1,"纠偏落点与贴图清理，不反发请求"): return
            if not check(hud.apply_game_message("+FAIL".to_ascii_buffer()) and hud.hud_snapshot().player_cell==origin,"纠偏后的拒绝不错误移动"): return
            if not await capture("skill-special-%d-correction" % id): return
        print("SPECIAL_CASE_OK %d" % id)
    hud.queue_free();await settle()
    print("SPELL_SPECIAL_UI_OK "+str(ids));return


# 场景：impact
# 命中生命周期：不重复起手；逐号检查跟随/固定落点、命中声音一次、爆炸帧和最终释放。

func voice_count(sample: int) -> int:
    return hud.hud_snapshot().scene_actor_sounds.values().filter(func(a): return a.sample==sample).reduce(func(n,a): return n+a.count,0)

func run_impact() -> void:
    var path: String=get_script().resource_path.get_base_dir().path_join("skill_common_cases.json")
    var fixture: Dictionary=JSON.parse_string(FileAccess.get_file_as_string(path))
    var cases: Array=fixture.cases.filter(func(c): return c.projectile and c.explosion!=null)
    for argument in skill_arguments():
        if argument.begins_with("--skill=") or argument.begins_with("--skills="):
            var values := argument.split("=",true,1)[1].split(",")
            if not check(Array(values).all(func(n): return n.is_valid_int() and cases.any(func(c): return int(c.id)==int(n))),"命中技能筛选有效"): return
            var ids: Array=Array(values).map(func(n): return int(n))
            cases=cases.filter(func(c): return int(c.id) in ids)
    hud=load("res://main.tscn").instantiate()
    hud.set_meta("offline_preview",true)
    hud.set_meta("options_path","user://spell-impact-%d.ini" % OS.get_process_id())
    hud.set_connection_enabled(false)
    root.add_child(hud);await settle()
    hud.set_local_actor_identity(77)
    var origin: Vector2i=hud.hud_snapshot().player_cell+Vector2i(0,2)
    var target := origin+Vector2i(0,4)
    var moved := target+Vector2i(2,0)
    for c in cases:
        var id := int(c.id)
        print("IMPACT_CASE_START ",id)
        hud.change_map("0","比奇省",origin,1)
        if not check(hud.set_scene_human(89,target,0,0,"命中目标"),"准备目标"): return
        var sample := int(c.explosion_sound.id)
        var before := voice_count(sample)
        if not check(hud.apply_game_message(magic_reply(id,target,89)),"创建弹道原包"): return
        if not check(await wait_until(func(s): return s.spell_effects.any(func(e): return e.skill==id and e.projectile)),"弹道实际进入飞行"): return
        if not check(hud.set_scene_human(89,moved,0,0,"移动后的目标"),"确认目标移动成功"): return
        if not check(await wait_until(func(s): return s.spell_effects.any(func(e): return e.skill==id and not e.projectile and e.first==int(c.explosion[0])),3000),"飞行切换独立命中资源"): return
        var impact: Dictionary=hud.hud_snapshot().spell_effects.filter(func(e): return e.skill==id)[0]
        if not check(impact.cell==(moved if c.track else target) and impact.target_actor==(89 if c.track else 0),"逐技能跟随或固定落点"): return
        if not check(impact.library==int(c.explosion[3]) and impact.end==int(c.explosion[1]) and impact.interval==int(c.explosion[2]) and not impact.repeat,"独立爆炸库/末帧/间隔及单次播放"): return
        var expected := 1 if c.track and c.explosion_sound.exists else 0
        if not check(voice_count(sample)-before==expected,"命中声音遵守跟随引用及实际原文件"): return
        if cases.size()==1 or id in [5,67,95]:
            if not await capture("skill-impact-%d" % id): return
        if not check(await wait_until(func(s): return s.spell_effects.any(func(e): return e.skill==id and e.light_tail),4000),"爆炸完成后进入光尾"): return
        if not check(voice_count(sample)-before==expected,"命中声音不随爆炸帧重复"): return
        if not check(await wait_until(func(s): return s.spell_effects.is_empty(),5000),"光尾释放，不遗留事件"): return
        print("IMPACT_CASE_OK ",id)
    hud.queue_free();await settle()
    print("SPELL_IMPACT_UI_OK ",cases.map(func(c): return int(c.id)));return


# 场景：target-loss
# 失去目标：原版44B7E0保留3秒无目标飞行，38号例外转静默命中；不能补绑新角色。
func run_target_loss() -> void:
    var ids := [5,13,37,38,39,40,41,67]
    for arg in skill_arguments():
        if arg.begins_with("--skill=") or arg.begins_with("--skills="):
            var values := arg.split("=",true,1)[1].split(",")
            if not check(Array(values).all(func(n): return n.is_valid_int() and int(n) in ids),"失去目标子集有效"): return
            ids=Array(values).map(func(n): return int(n))
    hud=load("res://main.tscn").instantiate()
    hud.set_meta("offline_preview",true)
    hud.set_meta("options_path","user://spell-target-loss-%d.ini" % OS.get_process_id())
    hud.set_connection_enabled(false)
    root.add_child(hud);await settle()
    hud.set_local_actor_identity(77)
    var origin: Vector2i=hud.hud_snapshot().player_cell+Vector2i(0,2)
    var target := origin+Vector2i(0,4)
    for id in ids:
        hud.change_map("0","比奇省",origin,1)
        if not check(hud.set_scene_human(89,target,0,0,"将消失的目标"),"确认原目标创建"): return
        var sample: int=id*10+10002
        var before := voice_count(sample)
        hud.apply_game_message(magic_reply(id,target,89))
        if not check(await wait_until(func(s): return s.spell_effects.any(func(e): return e.skill==id and e.projectile)),"目标丢失前已有弹道"): return
        if not check(hud.remove_scene_actor(89),"移除原目标"): return
        # 同编号新对象出现不能被失去引用的旧弹道重新绑定。
        if not check(hud.set_scene_human(89,target+Vector2i(2,0),0,0,"重新出现的编号"),"确认同编号目标重新创建"): return
        if id==38:
            if not check(await wait_until(func(s): return s.spell_effects.any(func(e): return e.skill==id and not e.projectile)),"38不使用三秒空飞，转静默命中"): return
        else:
            await settle(2.4)
            if not check(hud.hud_snapshot().spell_effects.any(func(e): return e.skill==id and e.projectile and e.target_actor==0),"未满三秒保持空飞且不补绑目标"): return
        if not check(voice_count(sample)==before,"丢失引用后不播放命中声"): return
        if not check(await wait_until(func(s): return s.spell_effects.is_empty(),5000 if id==38 else 1600),"无目标效果最终释放"): return
        if not check(voice_count(sample)==before,"释放时不补发命中声"): return
        print("TARGET_LOSS_CASE_OK ",id)
    hud.queue_free();await settle()
    print("SPELL_TARGET_LOSS_UI_OK ",ids);return


# 场景：stationary
# 定点/抑制分支：实际跟随位置、失去引用、单次音效、光尾释放和1410非113分发。
func run_stationary() -> void:
    var path: String=get_script().resource_path.get_base_dir().path_join("skill_common_cases.json")
    var fixture: Dictionary=JSON.parse_string(FileAccess.get_file_as_string(path))
    var cases: Array=fixture.cases.filter(func(c): return not c.projectile and not int(c.id) in [36,74,104,107,108])
    for arg in skill_arguments():
        if arg.begins_with("--skill=") or arg.begins_with("--skills="):
            var values := arg.split("=",true,1)[1].split(",")
            if not check(Array(values).all(func(n): return n.is_valid_int() and cases.any(func(c): return int(c.id)==int(n))),"定点子集筛选有效"): return
            var ids: Array=Array(values).map(func(n): return int(n))
            cases=cases.filter(func(c): return int(c.id) in ids)
    hud=load("res://main.tscn").instantiate()
    hud.set_meta("offline_preview",true)
    hud.set_meta("options_path","user://spell-stationary-%d.ini" % OS.get_process_id())
    hud.set_connection_enabled(false)
    root.add_child(hud);await settle()
    hud.set_local_actor_identity(77)
    var origin: Vector2i=hud.hud_snapshot().player_cell+Vector2i(0,2)
    var target := origin+Vector2i(0,4)
    var moved := target+Vector2i(2,0)
    for c in cases:
        var id := int(c.id)
        print("STATIONARY_CASE_START ",id)
        hud.change_map("0","比奇省",origin,1)
        if not check(hud.set_scene_human(89,target,0,0,"定点目标"),"目标格可站立"): return
        var before: int=hud.hud_snapshot().active_spell
        var sample := int(c.magic_sound.id)
        var sounds := voice_count(sample)
        if not check(hud.apply_game_message(magic_reply(id,target,89)),"638定点回包"): return
        var creates: bool=c.reply=="effect" and c.magic!=null
        if creates:
            if not check(await wait_until(func(s): return s.spell_effects.any(func(e): return e.skill==id)),"定点效果存在"): return
            if not check(hud.hud_snapshot().active_spell==id,"创建更新当前编号"): return
            if not check(hud.set_scene_human(89,moved,0,0,"移动后的定点目标"),"移动格可站立"): return
            var expected := Vector2((moved if c.track else target)*Vector2i(48,32))
            if not check(await wait_until(func(s): return s.spell_effects.any(func(e): return e.skill==id and e.world_position.is_equal_approx(expected))),"逐号实际跟随或固定世界位置"): return
            hud.remove_scene_actor(89)
            if not check(hud.set_scene_human(89,origin,0,0,"新同编号对象"),"确认同编号对象重新创建"): return
            await process_frame
            var effect: Dictionary=hud.hud_snapshot().spell_effects.filter(func(e): return e.skill==id)[0]
            if not check(effect.target_actor==0 and effect.world_position.is_equal_approx(expected),"丢失后保持最后位置，不重新绑定"): return
            if not check(voice_count(sample)-sounds==(1 if c.magic_sound.exists else 0),"定点音效次数与原文件匹配"): return
            if cases.size()==1 or id in [2,47,102]:
                if not await capture("skill-stationary-%d" % id): return
            if c.light and int(c.magic[6])>0:
                if not check(await wait_until(func(s): return s.spell_effects.any(func(e): return e.skill==id and e.light_tail),5000),"定点动画完成后进入光尾"): return
            if not check(await wait_until(func(s): return s.spell_effects.is_empty(),7000),"定点及光尾释放，禁光技能也正常结束"): return
            if not check(voice_count(sample)-sounds==(1 if c.magic_sound.exists else 0),"定点声音不按帧重复"): return
        else:
            await settle(0.12)
            if not check(hud.hud_snapshot().spell_effects.is_empty() and voice_count(sample)==sounds,"抑制分支不创建贴图或声音"): return
            if not check(hud.hud_snapshot().active_spell==(before if c.reply=="ignore" else id),"完全忽略与只更新编号区别"): return
        # 4138C0：非113多目标消息只更新编号，不用目标列表伪造效果。
        var multiple := game_reply(Mir3HudView.SM_MAGICFIRE_LIST,77,target.x,target.y,id)
        multiple.append_array(encode_scene_bytes("89/89/99999".to_ascii_buffer()))
        if not check(hud.apply_game_message(multiple),"非113多目标原包"): return
        await settle(0.08)
        if not check(hud.hud_snapshot().active_spell==id and hud.hud_snapshot().spell_effects.is_empty(),"非113列表仅更新编号，无新增效果"): return
        print("STATIONARY_CASE_OK ",id)
    hud.queue_free();await settle()
    print("SPELL_STATIONARY_UI_OK ",cases.map(func(c): return int(c.id)));return


# 场景：melee
# 特殊近战批次：真实文本状态+鼠标输入，验证攻击包、动作、附魔色、声音及单次消费。

func run_melee() -> void:
    Input.use_accumulated_input=false
    var cases := [[7,"PWR",Mir3HudView.CM_POWERHIT,130], [12,"LNG",Mir3HudView.CM_LONGHIT,132],
        [25,"WID",Mir3HudView.CM_WIDEHIT,133], [26,"FIR",Mir3HudView.CM_FIREHIT,137],
        [34,"RND",Mir3HudView.CM_RANDHIT,140], [35,"BKS",Mir3HudView.CM_BACKHIT,141],
        [103,"MAN",Mir3HudView.CM_WIDEHIT2,133]]
    for argument in skill_arguments():
        if argument.begins_with("--skill="):
            var value := argument.trim_prefix("--skill=")
            if not check(value.is_valid_int() and int(value) in [7,12,25,26,34,35,103],"近战子集编号有效"): return
            cases=cases.filter(func(c): return c[0]==int(value))
        elif argument.begins_with("--skills="):
            var selected := argument.trim_prefix("--skills=").split(",")
            if not check(selected.size()>0 and Array(selected).all(func(n): return n.is_valid_int() and int(n) in [7,12,25,26,34,35,103]),"近战批次集合有效"): return
            cases=cases.filter(func(c): return c[0] in Array(selected).map(func(n): return int(n)))
    hud=load("res://main.tscn").instantiate()
    hud.set_meta("offline_preview",true)
    hud.set_meta("options_path","user://melee-spells-%d.ini" % OS.get_process_id())
    hud.set_connection_enabled(false)
    root.add_child(hud)
    await settle()
    hud.set_local_actor_identity(77)
    hud.set_character_vitals(100,100,200,200,0,100)
    var origin: Vector2i=hud.hud_snapshot().player_cell+Vector2i(0,2)
    var packets: Array[PackedByteArray]=[]
    hud.client_packet_ready.connect(func(packet: PackedByteArray):
        packets.append(packet)
        hud.apply_game_message("+GOOD".to_ascii_buffer()))
    for c in cases:
        hud.change_map("0","比奇省",origin,1)
        for command in ["UFIR","UBKS","URND","ULNG","UWID","UMAN"]:
            if not check(hud.apply_game_message(("+"+command).to_ascii_buffer()),"关闭上项状态"): return
        if not check(hud.apply_game_message(weapon_packet(1,0,3)),"装备特殊蓝色武器覆盖底色"): return
        if c[0]==12:
            if not check(hud.set_scene_human(88,origin+Vector2i(0,2),0,0,"刺杀隔格目标"),"准备刺杀第二格人物"): return
        if not check(await wait_until(func(s): return s.map_left_delay_ms==0),"等待地图输入门控"): return
        if not check(hud.apply_game_message(("+"+c[1]).to_ascii_buffer()),"近战状态原包"): return
        packets.clear()
        var step := Vector2i(0,1) if c[0]==12 else Vector2i(1,0)
        var pointer := Vector2((origin+step)*Vector2i(48,32)-hud.hud_snapshot().map_camera_pixel)+Vector2(1,1)
        attack_click(pointer,true)
        if not check(await wait_until(func(_s): return not packets.is_empty()),"真实点击产生近战请求"): return
        if not check(decode_request(packets[0]).decode_u16(4)==c[2],"攻击包编号与技能一致：%d" % c[0]): return
        if not check(await wait_until(func(s): return s.drawn_human_spells.any(func(e):
            return e.weapon and e.skill==c[0] and e.relative_frame>=3 and e.tint.is_equal_approx(Color8(50,70,200)))),"特殊近战贴图采用武器覆盖色：%d" % c[0]): return
        # 原SoundList的141指向m35-1.wav，但原安装目录与当前资源均缺该文件，保持静音。
        var has_sound: bool=hud.hud_snapshot().scene_actor_sounds.values().any(func(a): return a.sample==c[3])
        if not check(has_sound==(c[0]!=35),"第一帧音效遵守实际原资源，35缺文件保持静音"): return
        if not await capture("skill-melee-%d" % c[0]): return
        if not check(await wait_until(func(s): return s.player_action==-1),"特殊近战动作结束"): return
        # 蓄力类消费一次，持续开关通过关闭消息撤销。
        if c[0] in [12,25,103]:
            if not check(hud.apply_game_message(("+U"+c[1]).to_ascii_buffer()),"关闭持续近战技能"): return
        await settle(1.5)
        packets.clear()
        attack_click(pointer,true)
        if not check(await wait_until(func(_s): return not packets.is_empty()),"再次挥击"): return
        if not check(decode_request(packets[0]).decode_u16(4)==Mir3HudView.CM_HIT,"消费或关闭后恢复普通请求"): return
        if not check(await wait_until(func(s): return s.player_action==-1),"普通动作结束"): return
        # 同批验证远端原包的元素色和动作第一帧音效，不能只验证本地预测。
        var remote := origin+Vector2i(0,-1)
        var opcodes := {7:Mir3HudView.SM_POWERHIT,12:Mir3HudView.SM_LONGHIT,25:Mir3HudView.SM_WIDEHIT,
            26:Mir3HudView.SM_FIREHIT,34:Mir3HudView.SM_HUMAN_ACTION17,35:Mir3HudView.SM_HUMAN_ACTION16,103:Mir3HudView.SM_WIDEHIT2}
        if not check(hud.set_scene_human(89,remote,0,0,"近战远端验收"),"准备远端人物"): return
        var before: int=hud.hud_snapshot().scene_actor_sounds.values().filter(func(a): return a.sample==c[3]).reduce(func(n,a): return n+a.count,0)
        if not check(hud.apply_game_message(game_reply(opcodes[c[0]],89,remote.x,remote.y,2|(2<<8))),"远端近战原包入队"): return
        if not check(await wait_until(func(s): return s.drawn_human_spells.any(func(e):
            return e.weapon and e.skill==c[0] and e.relative_frame>=3 and e.tint.is_equal_approx(Color8(50,70,200)))),"远端元素色来自回包"): return
        var after: int=hud.hud_snapshot().scene_actor_sounds.values().filter(func(a): return a.sample==c[3]).reduce(func(n,a): return n+a.count,0)
        if not check((after>before) if c[0]!=35 else (after==before),"远端声音遵守原声音表与缺文件行为"): return
        if not check(await wait_until(func(s): return s.scene_human_actions[89].x==-1),"远端动作完成"): return
        print("MELEE_CASE_OK %d" % c[0])
    hud.queue_free()
    await settle()
    print("MELEE_SPELLS_UI_OK "+str(cases.map(func(c): return c[0])))
    return


# 场景：sound
# 技能音效专项：同种并发、异种互不覆盖、声道上限及换图释放。

func run_sound() -> void:
    var ids := [1,53]
    for argument in skill_arguments():
        if argument.begins_with("--skill="):
            var value := argument.trim_prefix("--skill=")
            if not check(value.is_valid_int() and int(value) in ids,"音效子集只接受1、53"): return
            ids=[int(value)]
    hud=load("res://main.tscn").instantiate()
    hud.set_meta("offline_preview",true)
    hud.set_meta("options_path","user://spell-sound-%d.ini" % OS.get_process_id())
    hud.set_connection_enabled(false)
    root.add_child(hud)
    await settle()
    hud.set_local_actor_identity(77)
    var origin: Vector2i=hud.hud_snapshot().player_cell+Vector2i(0,2)
    hud.change_map("0","比奇省",origin,1)
    var buses_before := AudioServer.bus_count
    for skill in ids:
        if not check(hud.apply_game_message(magic_reply(skill,origin+Vector2i(5,0))),"音效技能原包入队"): return
        var sample: int=10011 if skill==1 else 10532
        if not check(await wait_until(func(s): return s.scene_actor_sounds.values().any(func(a): return a.sample==sample)),"创建对应原音效声道"): return
        var sounds: Array=hud.hud_snapshot().scene_actor_sounds.values().filter(func(a): return a.sample==sample)
        if skill==53 and not check(sounds.size()==9,"九段冰波同时拥有九个独立声道"): return
        if skill==1:
            var initial_pan: int=sounds[0].pan
            if not check(await wait_until(func(s): return s.scene_actor_sounds.values().any(func(a): return a.sample==sample and a.pan<initial_pan),500),
                "向东飞行的火球更新原声道声像"): return
        if not check(hud.apply_game_message(magic_reply(skill,origin+Vector2i(4,0))),"追加同种技能"): return
        await settle(0.1)
        sounds=hud.hud_snapshot().scene_actor_sounds.values().filter(func(a): return a.sample==sample)
        if not check(sounds.size()>=2 and sounds.size()<=10,"同种技能并发且不超过十声道"): return
    if ids.size()==2:
        var sounds: Array=hud.hud_snapshot().scene_actor_sounds.values()
        if not check(sounds.any(func(a): return a.sample==10011) and sounds.any(func(a): return a.sample==10532),
            "火球与冰波资源同时存在，不替换另一声音"): return
    if not await capture("skill-sound-concurrent"): return
    hud.change_map("0","比奇省",origin,1)
    await process_frame
    if not check(hud.hud_snapshot().scene_actor_sounds.is_empty() and AudioServer.bus_count==buses_before,"换图释放全部技能声道及音频总线"): return
    hud.queue_free()
    await settle()
    print("SPELL_SOUND_UI_OK "+str(ids))
    return
