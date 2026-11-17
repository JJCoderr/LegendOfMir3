# HUD 快捷栏入口核对

原版依据：Godot br_godot_test_261115 的 client/docs/evidence/feature-audit/0043C130.c，HUD action 10（1190/1191）无窗口动作；action 15（1205）调用 sub_43AB90(this,16)。Godot hud_action 仅定义、发射和测试连接，没有生产连接补上该入口。

进一步核实原分析库（输入 SHA256 08f68b794bfb28803d1132c02b26d2595dba309be5b2b475ae0cdfe6a752f8aa）：434AC0 在偏移 358156 构造 419D50；435820 调用 419EE0 创建窗口 16，背景 1210，初始位置 (73,413)，逻辑大小 280×48。419F80 绘制六格物品及关闭按钮 1211/1212；41A3D0 处理 Z 和数字键。因此 CLIENT 旧名 HELP 实际对应快捷物品栏，并非帮助页。此次原版输出保存在 out/help-original.c 和 out/quickbar-original-items.c。

修改：HELP 按钮接入已有 _WND_ID_QUICK/CQuickWnd，与 Z 键共用窗口开关。POPUP 保留按钮状态及通用点击音效，取消错误的旧聊天弹窗。复用既有物品绑定和持久化，不增加第二套数据。

验证：out/build-game.cmd 七目标增量构建通过。同一构建运行 quick-empty、quick-items、quick-binding、interface-audio；界面 16 项、绑定 12 项、持久化 7 项、音效 15 项，共 50 个布尔检查通过，另含 12 个角色语音。新增真实 HUD 鼠标打开/关闭和不误开旧弹窗检查。out/quick-items.png 已查看，六格栏位于 HUD 上方，显示物品和数量。此批为本地原生控件验证，不代表在线物品使用流程已完成。
