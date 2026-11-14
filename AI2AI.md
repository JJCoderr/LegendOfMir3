# 开发交接记录

更新日期：2026-11-14。

## 目标与工作约束

- 使用 Godot 显示、C++23 实现客户端业务，CMake 管理工程，代码位于 `client`；GDScript 用于测试。
- 原版客户端行为为依据。开始功能实现前，先整理已有 IDA 证据及模块伪代码到 `client/docs/evidence`，只分析缺失或矛盾处；不复用旧 Godot 工程的业务实现。
- `reference` 开源实现按功能辅助核实协议和逻辑，不能替代目标版本证据。协议使用命名定义，不在业务代码中硬编码消息号。
- 外部资源目录为 `G:\游戏\宝藏传奇3客户端`，通过配置读取，禁止复制、解压或修改原资源。
- 一次闭环一个完整功能，再运行受影响范围的独立测试及必要场景验证。测试脚本开头用中文注释测试内容，不记录准备条件和执行流程。
- 提交前将真实游戏截图内嵌展示在对话中，提交信息使用中文。不使用 Computer Use。
- `AGENTS.md` 当前有用户修改，不能覆盖或纳入自己的提交。

## 当前进度与证据入口

- `7b1aca7`：建立全模块 TODO 与 IDA 入口清单。`client/TODO.md` 总表包含 60 项最终验收任务，历史局部完成记录不能代表整个模块完成。
- `client/docs/evidence/client-module-inventory.md`：原版模块控制流、窗口编号与函数入口、已知限制。
- `client/docs/evidence/client-entry-inventory.json`：已分析入口的分支与调用证据，不代表遍历整个二进制。
- `client/docs/evidence/client-message-inventory.md`：196 项消息登记，其中 56 项尚未命名；仍需补查隐含分支和独立分派入口。
- `client/docs/evidence/window-dispatch-instructions.json`：窗口分派汇编证据。窗口编号 0–22 中的 5、10、17 进入默认分支，不能直接认定为独立窗口。
- 已确认缺口包括窗口 19/20 寄售、窗口 21 信件与送礼；窗口 18 的正式用途仍待核实，不能猜作普通小地图。
- `client/docs/Game.md`、`client/RERENCES.md`、`server/RERENCES.md` 和两端 TODO 按功能持续更新。

## 证据复用和下一步

IDA 数据库名为 `mir3-map-review`，本机 MCP 地址为 `http://127.0.0.1:13337/mcp`。原版哈希记录在 `client/docs/evidence/map-wire.md`。复用结论前核对目标版本；本地临时反编译缓存位于 `client/build/ida-inventory`，缓存不属于已提交证据，重要结论必须归档到 docs。

任务窗口 C05 尚未形成网络闭环。已缓存 `0046D330.txt`、`0046DC30.txt`、`0046D570.txt`、`0046D7F0.txt`，对应 1301、1302、1310、1311 的原版处理入口。后续先读取这些缓存、整理字段和状态转换伪代码，再实现命名消息与列表、详情、成败恢复；不能把现有 `set_quest_entries/details` 显示接口视为完成。

天气的上一实现批次为 `5c36a98`，已完成雪/雾及昼夜字段修正，证据见 `client/docs/evidence/weather.md`。天气 3 的缓存为 `client/build/weather3-ida.txt` 与 `weather3-setup-ida.txt`，尚未实现。

## 验证入口

- Windows 构建：`cmake --build client/build --config Release --target mir3_client hud_checks --parallel`。
- 原生检查程序：`client/build/Release/hud_checks.exe`，按功能选取已有开关。
- Godot 定向测试：使用 Godot console 的 `--path client/godot --script ../tests/<功能脚本>.gd -- --screenshots`。
- 真实视口截图位于 `client/build/screenshots`，在对话中使用绝对路径图片展示。历史截图仅说明历史批次，不能作为新代码测试结果。
- 模拟回包、环回连接与截图通过均不能替代真实服务器联调或原版同状态像素验收；未知项保留在 TODO。

## 普攻频率门禁批次

已接通417C40准入门禁，见client/src/hud/melee_cooldown.hpp。速度由本地801/807/808参数3按有符号WORD读取，等级及手持重量取现有角色模型。普通近战与挖矿共用最近获准时间，换角色或退出游戏清零。hud_checks --melee-cooldown及melee_cooldown.gd一次集中验收通过，总耗时约6秒。截图melee-cooldown-attack.png。现有近战/采矿脚本的点击等待调整到原版间隔；未启动这些长脚本。后续仍需完成419B40共用动作等待及服务端反馈，G01保持未完成。

## 本地共用动作等待批次

已实现LocalActionClock与单个待执行动作：移动600毫秒准入后进入共用等待，普攻/矿镐通过频率门禁后进入共用等待。419B40参数在进入等待时固定，时间条件到期只执行一次，移动包使用准入时间，攻击包使用实际出招时间。独立模型local-action-clock及melee-cooldown通过；Godot local_action_clock.gd及melee_cooldown.gd通过。跑动用例最初误选北侧障碍，读取原始0.map核实后改用南侧连续可走格，仅重跑相关短用例。截图local-action-walk.png和local-action-run.png。技能移动延迟、服务器纠偏与状态字段剩余项见TODO。
