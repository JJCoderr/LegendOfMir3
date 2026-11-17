# 实服属性同步与游戏服连接生命周期（2026-11-17）

## 修复与依据

原生客户端 `ReceiveAbility` 已按 Godot/MIR3 的 63 字节能力记录解析，但游戏服仍直接编码旧 `CObjectAbility`（生命、经验、攻防的排列不同）。`Server/Def/Mir3Ability.h` 显式生成目标记录，普通能力刷新和升级后的能力刷新共用该转换。金币、职业仍由原消息头发送。

本阶段映射等级、五组攻防范围、生命/魔法及上限、32 位经验/升级经验、负重、穿戴与手持重量。不把旧五元素范围误当作原版七元素扩展；未支持的扩展保持零。依据仍见 `ability-wire-evidence.md`，客户端绘制、字体、颜色和解析未改动。

实测部署时又发现 DBSvr 在游戏服连接关闭后直接释放对象，未从 `g_xServerList` 删除；选角仍取列表首项，导致角色准备记录发往失效连接，表现为黑屏等待。修复如下：

- 用同一读写锁保护连接加入、移除和选角发送，关闭时先从列表移除并失效 socket。
- 每个排队数据库请求持有连接引用；接收结束与队列处理结束分别释放，避免断连后排队请求访问已释放对象。
- 数据库处理期间持读锁，防止发送时 socket 被关闭并复用；已断开连接保留 INVALID_SOCKET，原有持久化请求仍可完成，不向新连接误发。
- 队列完成或入队失败时释放其附加数据；没有更改登录数据库内容。

## 验证

- `Server/Tools/test_mir3_ability.cpp` 固定 63 字节黄金记录：覆盖旧结构顺序、范围高低字节、32 位值、重量、空值及扩展清零。MSVC 编译执行通过。
- 七目标构建通过；DBSvr 的全部 CPP 在头文件布局变化后重新编译。日志 `out/ability-session-build.log`。
- DBSvr 新启动后，先连接并关闭三次模拟游戏服，再启动真实游戏服，实际客户端完成登录、选角、入场，验证旧连接不再占据候选列表。
- `python Server/Tools/test_db_connection_lifetime.py`：20 次断连、160 个排队的不存在角色读取请求，最终读请求仍正常返回 DBR_FAIL。只读取，不创建或修改角色。
- `python Server/Tools/test_gate_user_index.py`：前批越界索引回归仍通过。
- 实际人物页显示等级 1、生命/魔法上限及真实负重；背包负重由此前 0/0 恢复为 3/50。不是离线预览或测试页注入。
- 客户端已关闭，六服务仍在运行，未观察到本轮新崩溃。未实际升级角色；升级分支已改为同一转换，运行时升级全流程仍待验证。

截图：

- `out/native-live-01/live-ability-status.png`：实际人物页。
- `out/native-live-01/live-ability-bag-weight.png`：人物页与背包负重。

## 部署记录及边界

- 更新 GameSvr、DBSvr；为了连接恢复，重启 DBSvr、SelGate、GameSvr、GameGate，原 LoginSvr、LoginGate 保持运行。
- 本轮 GameSvr SHA256：`F6619ED21DEBE8CCC3E92060C2CD37E26EE66A2F5A083BEB0948D6CD4DC43621`。
- 本轮 DBSvr SHA256：`CA54A3362E8C2E9C47C9F487D31C6436F90B691BBED89F6214181142B3B9ECC7`。
- 备份在 `out/native-live-01/server-backup`。第一次覆盖遇到退出中的程序仍占用文件，之后等待进程退出、重新复制并核对哈希；未用旧程序冒充本轮验证。
- 最终进程：DBSvr 34264、SelGate 23520、GameSvr 17436、GameGate 34804；测试客户端 31952 已退出。后续操作必须重新查询 PID。
- 背包物品仍为空：旧服务器的 12 字节物品实例标识、标准物品记录，与客户端 MIR3 100 字节物品记录及数值实例 ID 不同，尚未完成双向映射，不能只把图标显示当作物品系统完成。
- 七元素扩展、实际升级、复杂多账号并发、GameGate 自动重连和 ProcessLogin ready-list 游标问题仍待后续处理。总体 GUI 与实服目标继续保留。
