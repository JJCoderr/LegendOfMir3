# 原生技能列表及快捷栏接入

依据 Godot br_godot_test_261115 的 protocol/skill_wire.hpp、magic/skill_panel.hpp、hud_view.cpp的apply_skill_records/progress/removal和docs/evidence/skill-wire.md。原版入口为44F4F0全量、44F730新增、44F670删除、44F870熟练度、44F8E0按键冲突处理。

原生CLIENTMAGICRCD经static_assert确认为74字节：当前修炼DWORD0、按键4、等级5、技能ID WORD6、名称8、三门槛DWORD31/35/39。现逐条验证99字节六位编码并解码，列表以/结束每条；计数用wSeries低字节，计数之后余文忽略。新增只接受单条记录。坏记录、缺分隔、零ID及非法编码在替换前拒绝，保留已有列表和快捷栏。

全量重建列表、关闭按键菜单并选择首个非空类别，空列表保留原类别。640使用等级低字节、wTag/wSeries组合完整DWORD修炼；212删除首条匹配，关闭该技能菜单并切至非空页。修炼改用无符号字段及显示，避免高位数值变负。

独立12槽快捷表按原版保存技能ID低字节。全量支持1..9/A..C，新增按atoi路径，新增A不填快捷表；全量及删除不主动清表，退出角色清空。技能窗口记录上的快捷标记与HUD/按键菜单的快捷表分开。绑定冲突沿用取消后设置的1008报文，同时更新快捷表。实际施法仍沿用当前CLIENT的GetMagicByKey查找，不把本批视为施法系统迁移完成。

完整Client清理构建87/87及全部七目标构建通过。新增12项+既有技能布局21、菜单10、说明7、快捷栏3、能力13、升级15项，共81项通过；说明行数13、12个图标与标记帧另外确认。193 skill-wire、194 skill-wire-progress截图out/skill-wire-list.png、skill-wire-progress.png已核对。

测试使用独立74字节记录编码后调用实际接收入口；没有执行完整加密连接或实服施法/消耗/战斗验证。原生字体、皮肤、输入与已有按钮音效继续复用。
