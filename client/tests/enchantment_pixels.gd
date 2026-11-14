# 同帧像素验收：冻结挥击后比较真实着色器与仅返回背景的基线，避免只看绘制元数据。
extends "weapon_enchantment.gd"

func run() -> void:
    var ids := [408,409,410,411,412,413,414]
    for arg in OS.get_cmdline_user_args():
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
        if OS.get_cmdline_user_args().has("--screenshots"):
            var path := ProjectSettings.globalize_path("res://../build/screenshots/skill-enchantment-pixels-%d.png" % id)
            if not check(actual.save_png(path)==OK,"保存未改写着色器的原始截图"): return
            print("SCREENSHOT: ",path)
        if not check(await wait_until(func(s): return s.player_action==-1),"挥击结束"): return
        await settle(1.5)
    hud.queue_free();await settle()
    print("ENCHANTMENT_PIXELS_UI_OK ",ids);quit()
