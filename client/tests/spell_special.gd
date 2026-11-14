# 特殊路径补证：74完整阶段/固定目标偏移；36/104/107/108确认、拒绝和强制纠偏。
extends "skill_casting.gd"

func run() -> void:
    var ids := [74,36,104,107,108]
    for argument in OS.get_cmdline_user_args():
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
    print("SPELL_SPECIAL_UI_OK "+str(ids));quit()
