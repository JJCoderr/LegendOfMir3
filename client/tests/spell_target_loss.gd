# 失去目标：原版44B7E0保留3秒无目标飞行，38号例外转静默命中；不能补绑新角色。
extends "spell_impact.gd"
func run() -> void:
    var ids := [5,13,37,38,39,40,41,67]
    for arg in OS.get_cmdline_user_args():
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
    print("SPELL_TARGET_LOSS_UI_OK ",ids);quit()
