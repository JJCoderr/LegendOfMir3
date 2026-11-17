# 原生技能与面板键盘路由

基线feafc67。依据Godot br_godot_test_261115的hud_view.cpp：_unhandled_key_input、bind_skill_at_cursor、bind_skill、bound_skill、spell_pointer_available；hud/hud_layout.hpp；tests/hud_smoke.gd的技能树、F12、低字节ID和遮挡用例。本批是GUI输入链路迁移，不是完整法术与战斗验收。

## 完成行为

- F1–F12统一映射1–9/A–C。F9/F10/F11不再切换背包/人物/技能窗，F10通过WM_SYSKEYDOWN进入同一分派；聊天编辑框补齐F9–F12及系统F10转发。
- 鼠标在技能窗口时绑定快捷键；按未学习的树格也清除对应键的旧绑定，滚动后的坐标生效。窗口空白处消费按键，覆盖窗口与模态框阻止绑定/施法穿透。绑定冲突实际发送1008取消后设置。只有旧快捷表仍指向本技能时才清旧位置。
- 角色入口按独立12槽低BYTE技能表查找实际记录；全量/删除保留的陈旧绑定若找不到记录，不执行技能。移除旧A键意外落入F1的分支。保留GetMagicByKey供记录查询，实际角色键盘入口使用GetMagicByShortcut。
- 场景键盘入口避开HUD、展开/收起聊天框、所有活动面板和模态框；小地图打开时整个场景阻止技能键。普通按键自动重复被消费；F10/F12仅在场景路径继续交给既有施法处理，重复不会改绑定。
- E/Q/W/G/N分别切换技能、人物、背包、组队和选项，R切换聊天展开；保留已有B/C/D/F/J/P/S/T/V/Y/Z输入。Escape先退商店详情再关闭商店，普通面板按最上层关闭，交易窗口保留等待服务器状态。
- GameProc在GUI消费按键后立即返回，避免N键打开选项后仍执行旧调试SetNextProc退出场景。没有给原本静默的键盘切换增添点击音；按钮与物品音效通过既有音效回归。

## 验证

客户端清理87/87、七目标109/109和最终增量构建通过。预览228 keyboard-skill-binding、229 keyboard-skill-shortcuts、230 keyboard-skill-covered；下一231。运行out/mud3-01/Mir2Ex-keyboard-final.exe。

34项新增检查包括实际UDP捕获1008、12个物理键、F10系统路径、聊天框转发、冲突/未学/滚动/遮挡/重复、低字节及删除记录查找、HUD与小地图限制、面板快捷键以及Escape商店和交易分支。角色调用处的完整施法动作/资源/回包不在这34项范围内。

集中回归：技能布局21、菜单10、说明7、顶部快捷栏3、技能协议12；交易54、物品使用26、快捷栏16；邮件15+滚动3+附件4+金额4；寄售16+金额1；音效15。加新增34项，共241项全过。说明13行、快捷栏12图标/12标记、音效12声部为另行元数据，不计入布尔检查数。

截图out/keyboard-skill-binding.png、keyboard-skill-shortcuts.png、keyboard-skill-covered.png。遮挡夹具将技能窗移动到NPC固定左上角区域之后，实际绘制与命中范围一致；不通过单独伪造NPC矩形证明遮挡。截图显示真实CLIENT控件、MIcon/GameInter资源和已有字体。

## 后续完整功能

已审查Godot docs/evidence/skill-casting.md、protocol/spell_request.hpp、magic/spell_routes.hpp、magic/spell_timing.hpp及request_skill_cast：原生Actor::SetMagic仍有旧路由/500ms门槛/旧提示，完整施法需要迁移动作预测、MP门槛、3017带有符号tick正文、特殊技能路由、移动限制、638/1410效果及失败恢复。不能把本批GUI按键通过当作法术已经正确。

退出/返回角色的Alt+Q、Alt+X及战斗冷却、原提示和网络退出链路仍需完整批次核对。原生骑乘和场景切换输入限制也需与实际技能请求一起验证。全客户端与服务器接入目标继续未完成。
