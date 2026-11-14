# 特殊近战批次：真实文本状态+鼠标输入，验证攻击包、动作、附魔色、声音及单次消费。
extends "weapon_enchantment.gd"

func run() -> void:
    Input.use_accumulated_input=false
    var cases := [[7,"PWR",Mir3HudView.CM_POWERHIT,130], [12,"LNG",Mir3HudView.CM_LONGHIT,132],
        [25,"WID",Mir3HudView.CM_WIDEHIT,133], [26,"FIR",Mir3HudView.CM_FIREHIT,137],
        [34,"RND",Mir3HudView.CM_RANDHIT,140], [35,"BKS",Mir3HudView.CM_BACKHIT,141],
        [103,"MAN",Mir3HudView.CM_WIDEHIT2,133]]
    for argument in OS.get_cmdline_user_args():
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
    quit()
