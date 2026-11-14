# 共用施法批次：独立原版元数据期望；按编号验证请求、起手、范围、资源、声光及清理。
extends "skill_casting.gd"

func sound_matches(expected: Dictionary) -> bool:
    var found: bool=hud.hud_snapshot().scene_actor_sounds.values().any(func(a): return a.sample==int(expected.id))
    return found==bool(expected.exists)

func run() -> void:
    var fixture_path: String= get_script().resource_path.get_base_dir().path_join("skill_common_cases.json")
    var fixture: Dictionary=JSON.parse_string(FileAccess.get_file_as_string(fixture_path))
    var cases: Array=fixture.cases
    for argument in OS.get_cmdline_user_args():
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
    quit()
