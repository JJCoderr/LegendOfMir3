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

## 本地动作确认批次

4282C0的+前缀控制包转427C50；GOOD清等待，FAIL调用415E60恢复发起位置与方向。已接入ActionPrediction、输入门禁、8秒严格超时及换图清理。action_reply.gd故意不自动确认，测试预测位移、等待门禁、拒绝回滚、确认不截断动画及地图清理；模型检查包含8000/8001边界。local_action_clock.gd和melee_cooldown.gd回归通过。旧melee_input/map_harvest用例也显式模拟GOOD，未启动长场景。首轮固定0.16秒断言过早，改为等待实际位移状态后通过。截图action-reply-rollback.png。实服及二进制纠偏仍待完成。

## 本地服务器位移批次

按415AD0/411C00/411D40/411EA0/4120A0及417540接通6、7、9、1321。1321仍沿用既有SM_REVIVE_REFRESH标识，但本地实际行为是带外观的强制走步，识别字不作为本地ID门禁。原包必须16字节外观加5字节扩展分段编码；6/7保留骑乘，9/1321应用扩展骑乘。服务器强制位移不发送本地动作包，7相对帧4退回推导起点并战斗站立。模型forced-motion、Godot forced_motion与action_reply通过；初次退出2项ObjectDB提示在verbose定向复查中未复现，记录到TODO。实服及完整状态位仍待后续。

## 本地角色死亡批次

本地32/34已按415AD0/412000进入与位移共用的local_actor_messages队列。player_life保存尸体状态，动作19末帧将HP置零，死亡时保留骑乘字段但不画马。53本地身份转场景健康索引0；回血不解除死亡，尸体下一更新继续HP归零。1316不是本地复活，1321死后刷新推导锚点/外观但保持尸体不走动；50重新入场即使身份相同也重置死亡和残留动作。共享HumanLife模型检查及local_life、forced_motion一次集中验收通过，约9秒；位移用例退出再次出现2项ObjectDB提示，未宣称已修复。


2026-11-14：完成Godot音频退出清理，脚步及角色声音停止后解除流引用、切回Master并移除专用总线。播放中退出弱引用验收与原forced_motion复现用例通过，未出现ObjectDB泄漏。仅执行相关短用例；全场景测试留到全部功能完成后。证据见client/docs/evidence/audio-cleanup.md。


## 本地角色受击

- [x] 31/1309完整原包、本地FIFO、血量与伤害提示、徒步/骑乘动作、等级节拍、命中特效与来源音效、1309复制31、旧31折叠、500ms过滤及致死例外、尸体保持；local_struck及local_life定向验收通过，截图local-struck-standing/mounted.png。证据见client/docs/evidence/local-struck.md。
- [ ] 实服时序、原版同状态像素和全部命中特效变体覆盖；完整场景测试待全部模块完成。


## 采集蹲姿回包

- [x] 637与12远端FIFO、坐标/朝向/蹲姿、正文忽略、骑乘及尸体分支；本地两类回包仅排队消费，不打断移动或代替GOOD。纠正旧文档虚表偏移及637特效归属错误；butch_reply独立验收约5秒通过，截图butch-reply-crouch.png，证据见client/docs/evidence/butch-reply.md。
- [ ] 实服采集产物与原版同状态像素验收；637本身不负责生成物品或武器特效。


## 人物状态与即时灯光消息

- [x] 801/807/808本地及远端状态DWORD、int16速度FIFO；bit1血条校验；1100低字节即时灯光和1101即时血条校验刷新，纠正807旧灯光误判。独立actor_status、普攻门禁回归及scene-light模型通过；截图actor-status-bar-restored/local-light.png。证据见client/docs/evidence/actor-status.md。
- [ ] 其余状态位消费者、外观包生产者、实服和逐像素对照；G06仍未全部完成。


## 人物状态颜色

- [x] 人物身体/头发/武器/坐骑状态颜色、七种优先级、8/32半透明、原WIL区域染色；本地入场/位移/死亡及远端普通外观状态同步，受击保留已有状态。human_status_visual像素与消息断言、actor_status回归通过，截图human-status-green/translucent/corpse.png。证据见client/docs/evidence/human-status-visual.md。
- [ ] 其他状态位玩法限制、人物阴影、特殊装备光效及原版逐像素对照。
