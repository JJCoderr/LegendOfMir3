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
- `src/chat`：聊天双宽度历史、原版 ANSI/GDI 换行、滚动条状态；输入框和发送尚待接入。
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

后续按主界面、背包、人物、魔法、聊天、组队、行会及其余对话框分别提取模型、控制器、视图与证据，一项验证完成后再处理下一项。测试与验证使用命令行，不使用 Computer Use。
