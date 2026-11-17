# 原生服务器分组入口

## 依据与范围

参考 Mir3 仓库 `br_godot_test_261115`：`hud/server_groups.hpp`、`hud_view.cpp` 的 `open_server_groups`、`draw_server_groups`、`server_groups_input` 和 `open_configured_login`；原始位置证据见该仓库 `docs/evidence/login-flow.md` 的 488F20、4891C0、489BD0、48A270、48A5D0、48A900。

本批把原先未参与构建的 ServerSelectProcess 空壳实现为原生 800×600 分组入口，并接到现有 640×480 账号页。仍使用 CLIENT 的窗口过程、DirectDraw 和 GDI，不使用 Godot。

## 行为

- 读取外部 SelLogin.ini：ServerGroup 的标签、标题、地图位置，以及各 ServerGroupN 的显示名、登录配置节、更新配置节。每组、每组条目最多 40，初始不自动选择。
- 通过 Windows INI API 读取并缓存登录配置；选择只在内存交接网关、Param2/Param3 网址和 ServerList，不回写 Mir3.ini 或外部资源。更新配置节保留为元数据，本批不实现补丁下载。
- 默认入口先尝试分组页；缺少配置或必要图帧时回到已有 Mir3.ini 账号入口。
- 使用 LoginInterface 原始图帧：背景 0、退出 3/4、列表 7/8、地图标记 11/12/13、滚动块 15。所有固定图片忽略资源偏移；可选光效帧 1 按原偏移加法绘制。
- 当前提供的 LoginInterface 第 1 帧索引为 0，按参考版允许缺失，不伪造光效图。
- 地图标记按下即选择，列表条目按下即进入；每页最多 20 条可见，滚轮一次一条，分页十条，滚动/拖动范围到最后一项。滚动按下节流严格超过 10 ms；退出按钮按释放位置决定退出。
- 列表、标题用 9 号宋体居中；地图标签四方向黑色描边。坐标、色值和滚动比例取参考实现。
- 分组页停止大厅音频；参考输入处理没有额外的分组点击音，本批不添加新声音。进入账号页后继续既有 Opening.wav 和账号按钮音效。
- ClientSocket 接受内存中的服务器名称，即使它不在 Mir3.ini 内。原已有的 Mir3.ini ServerNIndex 兼容分支仍保留，真实登录协议兼容性留待后续联调。

## 验证入口

`--preview=server-groups`、`server-group-hover`、`server-group-selected`、`server-group-row`、`server-group-scroll`、`server-group-exit`、`server-group-login`。

预览使用真实资源与原生事件处理；长列表状态使用标注为 Preview server 的 40 行内存数据。账号交接预览不连接 SelLogin.ini 中的外部地址，也不打开网址。

七目标构建通过（out/server-groups-all.log，112/112）；最终复核后增量构建通过（out/server-groups-final.log）。分组页 33 项、账号页 27 项、界面音效 15 项共 75 项检查全部通过。默认启动另经桌面窗口观察确认分组页，拥有者 PID 的 TCP 连接数为 0。运行前后 Mir3.ini、运行目录及原资源目录 SelLogin.ini 的 SHA-256 均未改变。

最终程序：out/mud3-01/Mir2Ex-server-groups-review.exe。七张截图：out/server-groups.png、server-group-hover.png、server-group-selected.png、server-group-row.png、server-group-scroll.png、server-group-exit.png、server-group-login.png。前六张 800×600，交接后的账号服务器列表为 640×480。本批不证明真实登录、更新下载、账号/角色服务器闭环或 MIR3.EXE 同帧逐像素一致。
