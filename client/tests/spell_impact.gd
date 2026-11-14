# 命中生命周期：不重复起手；逐号检查跟随/固定落点、命中声音一次、爆炸帧和最终释放。
extends "skill_casting.gd"

func voice_count(sample: int) -> int:
    return hud.hud_snapshot().scene_actor_sounds.values().filter(func(a): return a.sample==sample).reduce(func(n,a): return n+a.count,0)

func run() -> void:
    var path: String=get_script().resource_path.get_base_dir().path_join("skill_common_cases.json")
    var fixture: Dictionary=JSON.parse_string(FileAccess.get_file_as_string(path))
    var cases: Array=fixture.cases.filter(func(c): return c.projectile and c.explosion!=null)
    for argument in OS.get_cmdline_user_args():
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
    print("SPELL_IMPACT_UI_OK ",cases.map(func(c): return int(c.id)));quit()
