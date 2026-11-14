# 定点/抑制分支：实际跟随位置、失去引用、单次音效、光尾释放和1410非113分发。
extends "spell_impact.gd"
func run() -> void:
    var path: String=get_script().resource_path.get_base_dir().path_join("skill_common_cases.json")
    var fixture: Dictionary=JSON.parse_string(FileAccess.get_file_as_string(path))
    var cases: Array=fixture.cases.filter(func(c): return not c.projectile and not int(c.id) in [36,74,104,107,108])
    for arg in OS.get_cmdline_user_args():
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
    print("SPELL_STATIONARY_UI_OK ",cases.map(func(c): return int(c.id)));quit()
