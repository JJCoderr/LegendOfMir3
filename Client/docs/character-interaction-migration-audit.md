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


## 2026-11-17 姓名输入内容与字节限制

大厅整批截图中姓名为空，不能证明原生EDIT内容可见。新增307 creation-name-ascii、308 creation-name-chinese、309 creation-name-restored三种填入内容截图，显示英文、中文以及错误返回/重新打开后重新输入。实际查看out/creation-name三图，姓名位于参考(288,405)、75×13位置，无密码掩码。

新增检查发现CharacterProcess未处理EN_CHANGE，原有EM_LIMITTEXT按字符计数，允许超13字节中文姓名。现接入既有ChatEdit.Changed/LimitAnsiEdit，按参考13字节截取且保留完整中文和编辑后缀。删除确认沿用其自身90字节限制。测试中EM_SETSEL按字符位置而非ANSI字节计数，已修正该夹具。

12项新检查：可见/焦点、限制值、ASCII上限、中文完整边界、中文替换、超限插入保留后缀、恰好插入保留后缀、错误隐藏、错误恢复空文本焦点、职业/性别切换保留姓名、重开清空。加既有交互15、角色表现22、选角对话框23，共72检查通过。七目标构建通过out/creation-name-final.log，六页新鲜截图在out/creation-name，自启预览均停止。下一编号310。真实服务器创建请求仍未联调。


## 2026-11-17 创建与删除原生编辑框键盘转发

参考hud_view.cpp 6112附近删除框使用Tab切换、Enter/Space执行当前按钮；7043附近创建姓名与删除输入保持编辑焦点。CLIENT原先沿用聊天转发，Escape/Tab夺焦，删除空格作为文本插入；创建空姓名Enter还隐藏输入框。ChatEditProc现按_CHAR_SEL_PROC转发这些控制键到CharacterProcess，保留编辑焦点，删除弹窗Space由提示框处理。普通姓名字符继续交给EDIT。新增HasNotice只读查询避免暴露私有提示框。

创建姓名报告增加7项真实EDIT消息检查：创建Escape/Tab焦点、空Enter、删除Escape/Tab焦点、Space取消当前按钮、Enter取消当前按钮。共姓名19+角色交互15+选角弹窗23+登录27+游戏模态27=111检查通过。七目标构建out/selection-editor-build.log通过，6页截图在out/selection-editor；创建中文与删除确认图已查看，所有自启预览停止。无新增编号，下一310。真实创建/删除服务端请求联调仍未开展。


## 2026-11-17 关闭创建页的输入状态清理

参考close_character_creation清空输入、释放焦点、隐藏并恢复普通字节限制。CLIENT取消和SM_NEWCHR_SUCCESS原先仅隐藏EDIT，遗留姓名和13字节限制。新增CloseCreationEditor并接入这两条实际返回路径：清空EDIT与旧输入缓存、取消密码掩码、恢复90字节、隐藏并交回主窗焦点、清除按下状态。

新增316 creation-return-cancelled、317 creation-return-created（成功为离线注入回复），下一318。11项检查覆盖两路径清理、重开13字节、收到列表后保持隐藏、再次创建恢复输入。加姓名19、交互15、选角弹窗23、列表22，共90检查通过。七目标构建creation-return-final.log通过。六页新鲜截图在out/creation-return，取消/成功均显示返回选角的等待页且无残留编辑器；两张已查看。最后仅补列表回复测试，重跑返回模式，生产实现未再变化。所有自启预览停止；不代表服务器创建成功或原版返回影片已验收。
