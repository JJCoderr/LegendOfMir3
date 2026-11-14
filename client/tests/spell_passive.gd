# 3/4号客户端职责：原版418D80无主动施法分支；保留学习/修炼显示，普通攻击不转技能。
extends "skill_casting.gd"

func run() -> void:
    var ids := [3,4]
    for arg in OS.get_cmdline_user_args():
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
    print("SPELL_PASSIVE_UI_OK "+str(ids));quit()
