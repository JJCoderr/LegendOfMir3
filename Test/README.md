# CSV 联调场景

Test.exe 是独立控制台程序，读取 CSV，逐行向 Client 或 Server 转发 JSON-RPC，检查响应并记录日志。Client/Server 不读取场景文件。CSV 的一行可以操作客户端，下一行立即查询服务端。

## 构建和运行

在仓库根目录运行：

```powershell
cmake --build build-eiserver --config Release --target Test --parallel 6
build-eiserver/Release/Test.exe Test/Inventory.csv --client-pid 1234 --server-pid 5678 --from 200 --to 400 --log out/item-segment.log --database out/runtime/mir3.sqlite3
```

- 第一个参数指定 CSV。
- `--client-pid` / `--server-pid`：以 `--debug-pipe` 启动的真实进程；只需提供所选步骤使用的目标端。
- `--from` / `--to`：包含边界的 **step 编号**，不是文件行号。省略时执行全部；空范围、逆序范围报错。
- `--log`：UTF-8 日志文件，自动创建父目录，每次运行覆盖；默认当前目录 `test.log`。
- `--timeout-ms`：单步骤 RPC/状态等待期限，默认 65000 毫秒。
- `--database`：Database 断言使用的 SQLite 数据库，只读打开。

中途开始会沿用进程当前状态，不会自动补跑前置步骤或恢复存档。Test.exe 不负责启动或关闭被测进程。

需要自动准备隔离环境并启动两端时，用辅助程序：

```powershell
python Test/launch_csv.py Test/Inventory.csv --log out/items.log
```

辅助程序使用标准库，读取同名 `Inventory.fixture.csv` 的 SQL 数据种子，创建独立 `out/rpc-*` 目录和资源链接，先执行服务端基础检查，再启动客户端并调用 Test.exe。默认程序目录 `build-eiserver/Release`，配置资源目录 `out/debug`，可用 `--bin-dir`、`--config-dir`、`--port-offset` 覆盖。也支持 `--from` / `--to`，但新启动的客户端没有已完成的业务前置状态。结束后只关闭自己启动的进程。

## CSV 格式

固定表头，step 必须为正数且严格递增，可以预留编号间隔：

```csv
step,name,target,action,params,expect
10,登录,Client,session.login,account=mirtest;password=test123,result.status=submitted
20,检查服务器,Server,state.get,,result.ready=true;result.services_ready=6
30,等待登录完成,Client,state.wait,,result.stage=server_list;result.notice=true
```

- target：`Client`、`Server`、`Database`。
- action：通常直接填写 RPC 方法名。
- params：`参数=值;参数=值`；空白表示无参数。
- expect：`响应路径=值;响应路径=值`，支持 `result.equipment.0.name` 等路径；留空表示只要求 RPC 正常返回。预期错误必须明确填写 `error.code`。
- 数字、true/false/null、数组按 JSON 值处理，其余普通文本按字符串处理。数字密码等需要保留字符串类型时使用 JSON 引号，再按 CSV 规则转义，例如单元格 `"password=""123456"""`。
- 含分号的字符串用 JSON 引号包围；含逗号、双引号或换行的 CSV 单元格用双引号包围，内部双引号写两次。支持 UTF-8 BOM、CRLF/LF 和多行单元格。

特殊 action：

- `state.wait`：反复查询该端 `state.get`，直到 expect 满足或超时。业务动作的 submitted 只代表提交，应另写状态/数据库断言确认结果。
- `raw`：params 是原始 JSON-RPC 文本，仅供格式错误、批处理、ID 精度等协议基础测试。
- `wire.expect`：Client 协议计数断言，params 例如 `message=CM_TAKEONITEM;direction=sent;at_least=1`。按源码协议名解析消息码，需要场景文件位于仓库内。
- `Database,query`：params 是只读 SQL，expect 如 `rows=[[0]]`。

`Inventory.csv` 是完整登录、物品、返回选角再进图的长场景，包含双端交替检查、协议计数及数据库断言。`Login.csv` 是独立登录场景，`RpcClient.csv` / `RpcServer.csv` 是基础协议测试。

日志包含时间、步骤编号、目标端、动作、实际响应、PASS/FAIL、总数和耗时。首个失败立即停止，退出码为 1；全部通过为 0。正常请求参数不写日志。辅助程序另存 runtime.log、server-preflight.log、result.json；场景执行到 protocol.get 时生成 protocol-coverage.json。服务端当前 state.get 反映服务和网络连接状态，具体业务持久化仍由数据库断言验证。

## 子系统划分

业务 CSV 统一放在根目录 `Test/`，以子系统名称开头；一个子系统可以按场景拆成多个脚本，不要求把所有用例塞入一个文件。每个脚本可以连续调用 Client、Server 和 Database，完成一次完整联调。

| 子系统 | CSV 脚本 | 测试范围 |
| --- | --- | --- |
| 登录 | `Login.csv` | 认证、选服、选角与进入地图 |
| 角色 | `Character.csv`、`CharacterDelete.csv` | 创建角色、删除角色及持久化 |
| 物品 | `Inventory.csv`、`InventoryDuplicates.csv`、`InventoryFull.csv`、`InventorySaveFailure.csv` | 背包、穿脱、使用、丢弃、拾取、保存与重进 |
| 技能 | `Skills.csv`、`SkillsCasting.csv`、`SkillsHealing.csv` | 技能列表、快捷键、保存恢复、施法回包、扣蓝与受伤治疗 |
| 怪物 | `MonstersSpawn.csv`、`MonstersLoot.csv`、`MonstersLootCount.csv`、`MonstersGold.csv`、`MonstersGoldCap.csv`、`MonstersEquipment.csv`、`MonstersEquipmentCount.csv`、`MonstersEquipmentFailure.csv` | 刷新、实体名称、普通物品与装备掉落拾取、金币更新与满额保留、装备入库失败保留 |
| 战斗 | `CombatMagic.csv`、`CombatExperience.csv` | 实体施法、伤害、死亡、无效目标拒绝、经验升级与战斗退出冷却 |
| RPC 基础 | `RpcClient.csv`、`RpcServer.csv` | 两端接口与错误响应 |
| 执行器 | `Runner/*.csv` | CSV 解析、执行区间、断言与失败处理 |

后续场景使用 `<子系统><场景>.csv`，例如 `ChatChannel.csv`、`ShopBuySell.csv`、`CombatDamage.csv`、`NpcDialog.csv`。这些名称是扩展约定，不表示对应场景已经实现或验证。

归属规则：按场景的主要验证目标划分子系统。例如击杀后检查掉落属于 Monsters，检查经验升级属于 Combat，检查治疗效果属于 Skills。登录和进图是这些场景的前置步骤，不必另拆为 Client/Server 两份脚本。每个 CSV 的步骤号独立，运行时先选文件，再用步骤范围选择片段。

编写规则：

- 每个业务脚本从准备好的初始状态开始，写清登录、进图等必要前置步骤，再执行本子系统的动作、结果断言、失败分支和持久化检查。
- 数据种子放在同目录的同名 `.fixture.csv` 中，例如 `Inventory.fixture.csv`；辅助启动程序按文件名自动配对。
- `step` 是脚本内稳定、严格递增的步骤编号；新增脚本建议以 10 为间隔，便于插入检查。既有脚本保留编号，避免改变区间参数的含义。
- 动作返回 `submitted` 后，另用 `state.wait` 或数据库断言验证实际结果；服务器 `ready` 只能证明服务可用，不能代替业务结果断言。
- 一条长场景允许在客户端动作后立即查询服务器；`--from` / `--to` 选择该文件的步骤区间，`--log` 指定本次日志。截取中间区间时，调用者必须先满足该区间的前置状态。

例如执行物品子系统的指定区间：

```powershell
build-eiserver/Release/Test.exe Test/Inventory.csv --client-pid 1234 --server-pid 5678 --database out/runtime/mir3.sqlite3 --from 200 --to 400 --log out/logs/inventory-200-400.log
```

执行器自身验证：`python Test/verify_runner.py`。Runner 目录含用于验证失败处理的预期失败 CSV，应通过该验证程序执行。

`Character.csv` 覆盖创建角色：非法职业/性别/名字拒绝、重名服务器回包、失败恢复、创建女性法师、角色列表刷新、客户端人数上限、初始物品数据库检查及新角色进图。删除角色使用 `CharacterDelete.csv`：密码拒绝、正常删除四张关联表、触发器故障回滚、其他账号数据保留及剩余角色进图。

删除请求为角色名/分区登录密码；当前账号后端使用 Mir2_Account 中的 FLD_PASSWORD。SQLite 新建脚本的技能表已与服务端逐技能记录格式对齐。已有旧版 FLD_LEVEL0..30 宽表数据库不会由 CREATE IF NOT EXISTS 自动迁移，需另行迁移后使用技能记录；本轮没有修改现有数据库。

`Skills.csv` 验证已学技能列表、名称/等级/修炼值、F12绑定与占用替换、非法绑定拒绝、退出保存及重新登录恢复。当前不包含施法效果、技能学习、升级或删除。

`SkillsCasting.csv` 经真实客户端与服务器验证火球术快捷键施法、两次扣蓝及回包、未绑定快捷键与蓝量不足拒绝。当前只覆盖地图坐标目标，不包含实体目标伤害、其他技能效果。`skill.cast` 参数为 `key`（1–12）、`x`、`y`（地图格坐标），复用客户端快捷键业务；`state.get` 的 `spell_reply_id` 记录已解析的施法回包技能，`cast_ready` 表示动作时序允许尝试施法，具体技能还需满足已绑定、蓝量等条件。等待断言失败期间，执行器仅在响应状态变化时输出 WAIT，便于定位回包和状态更新。

`SkillsHealing.csv` 配置真实怪物攻击，将角色 HP 从 16 降到 12，再通过客户端治疗术恢复到 16、MP 从 18 降到 12；断言受击、施法和血蓝回包及服务器存活。fixture 中怪物攻击间隔为 60 秒，行动间隔为 500 毫秒，避免治疗期间连续伤害。服务端恢复数据库怪物刷新；缺少或不完整的 `Map/searchTable.tbl` 使用按距离排列的默认 15 圈搜索偏移。暂未覆盖怪物死亡后刷新、掉落、所有怪物类型与技能目标伤害。

`MonstersSpawn.csv` 使用数据库怪物编号 3、1007 和一个不存在的编号 1，验证只生成目标怪物，并查询名称、执行受伤治疗链路。`TBL_MONGEN.FLD_MONINDEX` 引用 `TBL_MONSTER.FLD_INDEX`，不依赖查询结果行号。客户端 `state.get` 增加 `actors`（id/name/x/y/dead）和 `actor_count`；`actor.query_name(index)` 使用当前 actors 数组的零基下标，走与鼠标悬停相同的名称查询协议。实体增减可能改变数组下标，调用前应读取当前状态。

`CombatMagic.csv` 使用真实火球术击杀低血量怪物，检查 SM_STRUCK、SM_NOWDEATH、客户端 HP=0/dead=true 和扣蓝，拒绝不存在或已死亡的目标。`skill.cast` 可以传 `key=1;target=0`，target 是当前 actors 数组的零基下标；客户端从选中实体读取坐标、网络标识和外观，调用与键盘相同的施法入口。省略 target 时继续使用 x/y 坐标施法。actors 增加 hp/max_hp，未收到血量回包时 max_hp=0 表示血量尚未知。该场景验证致死伤害，不验证精确伤害公式、掉落、经验和重新刷新。

`CombatExperience.csv` 验证击杀奖励、一次奖励跨多级、战斗退出冷却、数据库保存及重登恢复。fixture 奖励 1000 经验，按当前服务端经验表从 1 级升到 4 级并剩余 100 经验。state.get 增加 level、experience、next_experience、return_ready；return_ready 复用现有退出冷却判断，脚本通过 state.wait 等待，不跳过冷却或修改时钟。本次保持原经验表及等级索引，未校验其数值与原版资源是否一致。

`MonstersLoot.csv` 验证击杀后普通药水掉落、真实行走到物品格、拾取、保存和重登恢复，同时检查不存在怪物的掉落记录与禁用掉落记录不会生成额外物品。`world.walk(x,y)` 只提交到相邻格的单步移动，复用地图通行检查、动作时序及现有移动协议；远距离请求拒绝。FLD_PROBABLY 使用 1/N 抽取，N<=0 禁用。本场景使用 N=1 确保结果可复现，尚未验证随机概率分布、金币和装备掉落、多个同类物品与背包满时的处理。

金币掉落使用 `MonstersGold.csv`，检查掉落、行走拾取、SM_GOLDCHANGED、数据库余额和重登恢复；`MonstersGoldCap.csv` 检查达到当前服务端 BAGGOLD=5000000 时金币留在地面、余额不变，重登仍可见。客户端 state.get 的 gold 是角色当前余额。金币数量为0的配置不生成金币，数量为1保证至少1金币；多条金币配置累加。本轮未验证金币随机数量分布、玩家主动丢钱或混合物品散落顺序，未进行图标视觉检查。

`MonstersEquipment.csv` 验证击杀掉落装备、行走拾取、生成数据库身份、穿戴、保存及重登恢复。fixture 预置当天未补零的编号 9 和 10，断言新装备使用数值上的下一编号 00011。`MonstersEquipmentFailure.csv` 用 SQLite 触发器拒绝装备 INSERT，检查背包与装备栏为空、数据库无新增记录，并在返回选角再进图后确认物品仍在地面。新掉落装备在数据库事务提交后才加入背包和发送拾取成功；失败恢复地面物品。当前验证使用 SQLite，尚未覆盖 ODBC 并发分配、当天编号耗尽或背包满的装备拾取。

Inventory 步骤编号调整：为补入真实行走与到达检查，原有步骤号统一乘以 10（原 20–40 对应 200–400）；新增 262/264 和 332/334 步行走到药水、装备掉落格。恢复地图搜索表后，主动丢弃物品散落在相邻格，不再假设物品位于角色脚下。调用该脚本的历史区间参数需按此换算。

`InventoryDuplicates.csv` 验证两瓶相同属性药水具有不同的客户端 ID；丢弃第二瓶、真实行走拾回、使用第二瓶时第一瓶仍在，保存重登后剩余数量为 1，再使用并保存后为 0。普通物品离开背包或被消耗后旧 ID 失效，拾回时分配新 ID；脚本应读取 state.get 获取当前 ID。装备仍按完整数据库身份映射。当前未验证多份相同符咒或火把的装备槽位语义。

掉落数量场景：`MonstersLootCount.csv` 验证同一配置生成两瓶药水、分别行走拾取、独立客户端 ID 和保存重登数量；`MonstersEquipmentCount.csv` 验证生成两件装备、分别入库为 00011/00012，穿戴一件后另一件留在背包，保存重登保留两件。两份 fixture 同时包含数量为 0 的记录，断言无额外掉落。非金币配置命中一次概率后生成 FLD_COUNT 件独立物品；金币保持原随机数量规则。尚未验证大批掉落、周围格子全部被占用或背包满时的处理。

背包边界：Client/Server 使用 Shared/Mir3InventoryLimits.h 的 60 格容量；服务端只计算未穿戴物品。state.get 提供 bag_count、bag_capacity。`InventoryFull.csv` 以 60 瓶药水和一件已穿戴装备验证满包拒绝拾取、地面物品保留、使用一瓶后拾取、完整保存及重登。尚未覆盖大尺寸物品造成的布局空间不足或满包卸装。

保存原子性：DBSvr 将角色、技能和普通物品写入同一事务，并持有数据库管理器锁，全部成功后才提交。`InventorySaveFailure.csv` 通过触发器拒绝普通物品 INSERT，验证原有两瓶药水和角色位置保留、重登恢复上一次快照；保存失败会写服务端日志，当前没有新增客户端保存失败通知。
