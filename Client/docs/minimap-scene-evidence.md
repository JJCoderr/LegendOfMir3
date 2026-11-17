# 地图场景标记接入

## 来源与原版差异

复用Godot分支br_godot_test_261115的client/src/hud/minimap.hpp坐标模型和既有原生CMinimap。Godot将type_five_actors黄色集合受show_mon控制，原生旧代码又命名为monsters且只在预览赋值；以MIR3实际绘制分支为准修正分类。

- 45D0B0普通小地图：类型5始终黄色2×2；显示选项启用且对象+400432为0时，类型3红色2×2、类型0/1白色2×2。1312位置记录按类别分绿/橙且进行近距过滤。
- 45DAD0头像地图：NPC黄色单点，其他存活怪物/玩家仍红/白2×2，受相同显示选项控制。原版按full地图先除2再进行坐标转换，复用既有PortraitMarker。
- 45E660投石车：NPC黄色单点，可选存活怪物红色2×2，没有白色玩家分支。位置记录另按绿/橙处理，排除受控对象。
- 4093B0处理消息32/33时将对象+400432置1，4094A0复活路径清0；对应当前CActor::m_bIsDead。当前代码NPC种类常量为_GENDER_NPC=50，与原版内部类型5的数值不同，按语义映射，不能直接比较5。

只读原版导出out/minimap-scene-markers.c、out/minimap-actor-flag-writes.txt、out/extension-map.txt。

## 实现

CheckMappedData每帧开始清场景标记，在对象动作更新及删除处理后、视野裁剪前将有效CActor的位置/类型/死亡状态交给CMinimap::AddActor。NPC始终收集，存活怪物和玩家分别进入集合；绘制时按选项决定是否显示后两类，避免角色死亡后残留旧点。

1312位置缓存复用既有CSiegeWnd，过期清理后CopyMapMarkers填充普通小地图与头像地图的绿/橙集合。投石车直接使用该缓存，移除旧绿/橙集合重复绘制。选项值在RenderInterface起始同步，使头像地图和窗口链也在本帧应用设置。

## 验证与边界

clean Client及全部七目标构建通过（out/build-scene-markers-clean.log、out/build-scene-markers-all.log）。183/184/185/186预览分别scene-map、scene-portrait、scene-siege、scene-map-hidden；6项分类/位置更新/清空检查与既有135项回归全1。四张截图已检查，并对地图内容区域的精确红/白/黄像素分别断言，12个颜色计数全部符合预期，报告out/scene-marker-pixels.json。

截图为调用同一标记收集接口的离线场景数据；尚未用实服的对象出生、死亡、移动和1312消息做完整端到端测试。地图纹理透明度和部分标记边界的逐像素一致性仍需继续审计。
