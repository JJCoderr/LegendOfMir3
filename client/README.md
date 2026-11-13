# 客户端

Godot 4.5+ GDExtension，C++23，CMake。按功能从 `mir3_1.45` 提取并验证，不依赖其 `world.gd`、`extension.cpp` 或游戏逻辑总模块。

## Windows 构建

```powershell
cmake -S client -B client/build -G "Visual Studio 18 2026" -A x64 `
  -DMIR3_RESOURCE_ROOT="G:/游戏/宝藏传奇3客户端" `
  -DMIR3_GODOT_CPP="G:/mir3/mir3_1.45/third_party/godot-cpp"
cmake --build client/build --config Release --parallel
ctest --test-dir client/build -C Release --output-on-failure
Godot_v4.7.2-stable_win64_console.exe --headless --editor --path client/godot --import --quit
Godot_v4.7.2-stable_win64_console.exe --path client/godot
```

`MIR3_GODOT_CPP` 使用已有 godot-cpp 源码，本机版本 `e83fd0904c13356ed1d4c3d09f8bb9132bdc6b77`。构建产物位于 `build` 和 `godot/bin`，已忽略。

`MIR3_RESOURCE_ROOT` 必须指向原客户端目录。CMake 生成本机专用的 `godot/resource_path.cfg`，也可直接修改该配置后重启客户端。配置不提交，资源不复制、不导出；WIL/WIX 只在运行时读取并在内存中解码。

## 功能边界

- `src/hud`：主界面显示数据、原版布局和输入控制器，不依赖 Godot。
- `src/resources`：原包读取和像素解码。
- `src/text`：从旧实现单独提取的 Windows GDI 字体渲染。
- `src/chat`：聊天双宽度历史、原版 ANSI/GDI 换行、滚动条、ANSI 输入编辑和小组/行会模式。
- `src/godot`：C++ Godot 视图和扩展注册，无 GDScript 游戏逻辑。
- `tests`：原包、布局边界和行为验证。
- `docs/evidence/hud`：旧项目的 HUD 取证材料；其中“已实现”描述指旧项目，不能当作本项目的完成记录。

主界面正在迁移，当前包含底栏、血球/计量条绘制及提示、默认职业头像、等级底图、属性文字与切换、原包按钮提示、按钮状态及聊天区域展开收起。没有服务器数据时字段保持空值；灰色区域不是已实现的地图。其余按钮只发出 `hud_action` 信号，尚未接入对话框。等级底图选择字段沿用旧项目的全零初值，其生产路径仍未知。字体、DirectDraw 混色和原版同状态画面尚未完成像素级比对，不能声称整个主界面一致。

若 CMake 找到 Godot，CTest 也会执行项目导入与无界面交互冒烟；可通过 `MIR3_GODOT_EXECUTABLE` 指定可执行文件。`tests/hud_smoke.gd` 仅是测试驱动，通过 Godot 输入分发检查 C++ 控制器，不含客户端游戏逻辑。

每次提交前必须生成并展示当前游戏截图。运行带渲染的测试（不能加 `--headless`）：

```powershell
Godot_v4.7.2-stable_win64_console.exe --path client/godot --script ../tests/hud_smoke.gd -- --screenshots
```

截图保存于 `client/build/screenshots/hud-chat-compact.png` 和 `hud-chat-expanded.png`，尺寸 800×600，来自实际 Godot 视口。测试自动注入聊天数据，检查后退出；不使用 Computer Use，不拆包导出资源。截图展示的是已迁移模块，不能代替原版同状态画面对比。后续新增对话框时需加入对应测试状态和截图。

聊天记录复用旧版 `native_text.cpp` 的 `0x4366D0/0x48C03A` 路径：普通/展开分别按 341/429 像素测量 ANSI 字节前缀，最多保留 79 行；原版跨列表索引更新和普通滚动条按下更新展开索引的行为均保留。绘制采用 TextOutA 顶端原点，不用居中 DrawText；滚动条使用帧 1207，普通/展开行程 43/203。

聊天输入按 `0x47B880` 的 12px/400 字重、CMList1074 字体及 90 ANSI 字节限制；按 `0x43EF10` 布置为普通 `(185,575,344,15)` / 展开 `(185,534,432,15)`。中文输入和选区替换在 C++ 中限制，不以 Unicode 字符数替代字节数。Enter/空格聚焦，Escape 保留草稿并离开，Enter 提交后清空并离开。按钮 16/17 替换拒绝/喊话前缀，18/19 切换互斥的小组/行会模式，进入输入时填 `!!` / `!~`。

输入通过 `chat_requested(text)` 发出待发送请求，尚未连接 3030 网络消息，不制造服务端回显。控件按旧项目使用 Godot LineEdit，已验证位置、尺寸和编辑边界，但光标、选区、IME 与字体像素不是原生 Win32 EDIT 的完整复现，仍需对齐。测试通过真实 Godot 输入事件覆盖中文输入、提交、Escape、ANSI 上限、小组前缀；新增截图 `hud-chat-input.png`、`hud-chat-group-input.png`。

三个拒绝按钮按旧分析 `0x43C130` case20..22，读取各自拒绝状态后设置预测图案/提示，通过同一 `chat_requested` 发送 CMList1070/1071/1073 原文（包括1073末尾空格）。初始提示2637/2639/2643，预测拒绝使用1338/1340/1342与2638/2640/2644，预测允许清除动态叠图。`update_chat_rejection(channel,rejected)` 仅是外部状态入口：原版字段写入/回包路径未知，点击不自行切换状态，也不在本地过滤消息。截图 `hud-chat-reception.png`、`hud-chat-reception-allow.png` 覆盖两组分支，测试注入状态不代表网络应答已完成。频道切换提示使用旧分析的白色文字、COLORREF 0x80FF 背景。

后续按主界面、背包、人物、魔法、聊天、组队、行会及其余对话框分别提取模型、控制器、视图与证据，一项验证完成后再处理下一项。测试与验证使用命令行，不使用 Computer Use。
