# 角色页交互迁移验收

参考 Mir3/br_godot_test_261115 的 hud_view.cpp selection_input、creation_input、show_character_creation。使用 CLIENT 原生 DirectDraw、编辑控件和 DirectSound。

选角改为鼠标按下选中，命中矩形包含左上边界，重叠时采用首个角色，重复选择保留动画。按钮/翻页仍在释放命中时响应，点击声使用 LoginLayout::ClickSound。新建角色取消原来的渐变等待，下一绘制帧直接进入已迁移的创建影片。退出使用 CmList1077 配置网址并关闭窗口；离线模式只记录网址，不打开浏览器、不退出。

创建页统一入口：重置职业为战士、清空姓名、恢复13字节非密码编辑器及焦点，清理等待/按键/动画状态，保留上次性别。性别在鼠标按下时切换；职业在释放时切换，重复点击同职业也重启动画。补齐创建按钮点击声及姓名框右侧范围点击恢复焦点。

## 验收

客户端 clean90/90、全部七目标109/109、最终增量构建通过。日志 out/character-interaction-{clean,all,final}.log；运行产物 out/mud3-01/Mir2Ex-interaction-review.exe 来自 build-ninja-sqlite/Mir2Ex.exe。

交互15、列表22、提示23、影片33、登录27、淡出12、音效15，共147项通过。音效报告另有 voiceCount=12，为实际语音数而非布尔断言。最终矩形命中简化后复验交互15+列表22。预览进程均已停止；没有连接真实网关或实际执行退出网址。

预览289..291：character-interaction-select/create/female，下一292。实际后缓冲截图 out/character-interaction-{select,create,female}.png；已目视检查选角、男战士创建及女道士创建状态。截图使用测试角色。

## 剩余边界

整体 GUI 仍需汇总各页面差异，未宣称与 MIR3.EXE 逐像素完全一致。原版三部影片缺失仍见 lobby-movie-migration-audit.md；真实登录/创建/删除/进游戏协议联调留待 GUI 验收后。
