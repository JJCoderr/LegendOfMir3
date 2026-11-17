# CSV 联调场景

Test.exe 是独立控制台程序，读取 CSV，逐行向 Client 或 Server 转发 JSON-RPC，检查响应并记录日志。Client/Server 不读取场景文件。CSV 的一行可以操作客户端，下一行立即查询服务端。

## 构建和运行

在仓库根目录运行：

```powershell
cmake --build build-eiserver --config Release --target Test --parallel 6
build-eiserver/Release/Test.exe Test/Inventory/Inventory.csv --client-pid 1234 --server-pid 5678 --from 200 --to 400 --log out/item-segment.log --database out/runtime/mir3.sqlite3
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
python Test/launch_csv.py Test/Inventory/Inventory.csv --log out/items.log
```

辅助程序使用标准库，读取脚本同目录的同名 `Inventory.fixture.csv` 的 SQL 数据种子，创建独立 `out/rpc-*` 目录和资源链接，先执行服务端基础检查，再启动客户端并调用 Test.exe。默认程序目录 `build-eiserver/Release`，配置资源目录 `out/debug`，可用 `--bin-dir`、`--config-dir`、`--port-offset` 覆盖。也支持 `--from` / `--to` 和透传给 Test.exe 的 `--timeout-ms`，但新启动的客户端没有已完成的业务前置状态。结束后只关闭自己启动的进程。

## CSV 格式

固定表头，step 必须为正数且严格递增，可以预留编号间隔：

```csv
step,name,target,action,params,expect
10,登录,Client,session.login,account=mirtest;password=test123,result.status=submitted
20,检查服务器,Server,state.get,,result.ready=true;result.services_ready=6
30,等待登录完成,Client,state.wait,,result.stage=server_list;result.notice=true
```

- target：`Client`、`Client2`、`Client3`、`Server`、`Database`、`Runner`。Client2/Client3 是额外的真实客户端，用 `--client2-pid` / `--client3-pid` 指定进程。
- action：通常直接填写 RPC 方法名。
- params：`参数=值;参数=值`；空白表示无参数。
- expect：`响应路径=值;响应路径=值`，支持 `result.equipment.0.name` 等路径；留空表示只要求 RPC 正常返回。预期错误必须明确填写 `error.code`。
- 数字、true/false/null、数组按 JSON 值处理，其余普通文本按字符串处理。数字密码等需要保留字符串类型时使用 JSON 引号，再按 CSV 规则转义，例如单元格 `"password=""123456"""`。
- 含分号的字符串用 JSON 引号包围；含逗号、双引号或换行的 CSV 单元格用双引号包围，内部双引号写两次。支持 UTF-8 BOM、CRLF/LF 和多行单元格。

特殊 action：

- `Runner,wait`：params 为 `milliseconds=31000`，执行器等待真实时间，再继续下一步；响应含 `result.status=waited` 和 `result.elapsed_ms`。仅接受1至4294967294的整数毫秒，不需要客户端/服务器PID；不受RPC超时参数控制。用于补货等真实定时行为，不修改游戏时钟。

- `state.wait`：反复查询该端 `state.get`，直到 expect 满足或超时。业务动作的 submitted 只代表提交，应另写状态/数据库断言确认结果。
- `raw`：params 是原始 JSON-RPC 文本，仅供格式错误、批处理、ID 精度等协议基础测试。
- `wire.expect`：Client 协议计数断言，params 例如 `message=CM_TAKEONITEM;direction=sent;at_least=1`。按源码协议名解析消息码，需要场景文件位于仓库内。
- `Database,query`：params 是只读 SQL，expect 如 `rows=[[0]]`。

`Inventory/Inventory.csv` 是完整登录、物品、返回选角再进图的长场景，包含双端交替检查、协议计数及数据库断言。`Login/Login.csv` 是独立登录场景，`Rpc/RpcClient.csv` / `Rpc/RpcServer.csv` 是基础协议测试。

日志包含时间、步骤编号、目标端、动作、实际响应、PASS/FAIL、总数和耗时。首个失败立即停止，退出码为 1；全部通过为 0。正常请求参数不写日志。辅助程序另存 runtime.log、server-preflight.log、result.json；场景执行到 protocol.get 时生成 protocol-coverage.json。服务端当前 state.get 反映服务和网络连接状态，具体业务持久化仍由数据库断言验证。

商店价格按参考服 x87 运算顺序实现，具体指令、舍入边界与范围限制见 [商店价格核对](Shop/native-price-audit.md)。

## 子系统划分

业务 CSV 统一放在根目录 `Test/<子系统>/`，文件名以子系统名称开头；一个子系统可以按场景拆成多个脚本，不要求把所有用例塞入一个文件。每个脚本可以连续调用 Client、Server 和 Database，完成一次完整联调。

| 子系统 | CSV 脚本 | 测试范围 |
| --- | --- | --- |
| 登录 | `Login/Login.csv` | 认证、选服、选角与进入地图 |
| 角色 | `Character/Character.csv`、`Character/CharacterDelete.csv` | 创建角色、删除角色及持久化 |
| 物品 | `Inventory/Inventory.csv`、`Inventory/InventoryDuplicates.csv`、`Inventory/InventoryFull.csv`、`Inventory/InventoryFullUnequip.csv`、`Inventory/InventorySaveFailure.csv` | 背包、穿脱、使用、丢弃、拾取、保存与重进 |
| 技能 | `Skills/Skills.csv`、`Skills/SkillsCasting.csv`、`Skills/SkillsHealing.csv` | 技能列表、快捷键、保存恢复、施法回包、扣蓝与受伤治疗 |
| 怪物 | `Monsters/MonstersSpawn.csv`、`Monsters/MonstersRespawn.csv`、`Monsters/MonstersCorpse.csv`、`Monsters/MonstersLoot.csv`、`Monsters/MonstersLootCount.csv`、`Monsters/MonstersGold.csv`、`Monsters/MonstersGoldCap.csv`、`Monsters/MonstersEquipment.csv`、`Monsters/MonstersEquipmentCount.csv`、`Monsters/MonstersEquipmentFailure.csv` | 刷新、实体名称、普通物品与装备掉落拾取、金币更新与满额保留、装备入库失败保留 |
| 战斗 | `Combat/CombatMagic.csv`、`Combat/CombatExperience.csv` | 实体施法、伤害、死亡、无效目标拒绝、经验升级与战斗退出冷却 |
| 聊天 | `Chat/ChatLocal.csv`、`Chat/ChatShout.csv`、`Chat/ChatShoutLevel.csv`、`Chat/ChatShoutRange.csv`、`Chat/ChatShoutRangeY.csv`、`Chat/ChatWhisper.csv`、`Chat/ChatWhisperReject.csv`、`Chat/ChatWhisperIsolation.csv` | 普通聊天、中文编码、喊话等级及冷却拒绝 |
| NPC | `Npc/NpcSpawn.csv`、`Npc/NpcDialog.csv`、`Npc/NpcLevel*.csv`、`Npc/NpcImports.csv`、`Npc/NpcContext.csv` | 商人生成、NPC 类型、名称同步、基础对话及选项跳转 |
| 商店 | `Shop/Shop*.csv` | 普通物品购买、金币/库存/背包同步、失败保留、保存重登 |
| RPC 基础 | `Rpc/RpcClient.csv`、`Rpc/RpcServer.csv` | 两端接口与错误响应 |
| 执行器 | `Runner/*.csv` | CSV 解析、执行区间、断言与失败处理 |

后续场景使用 `<子系统>/<子系统><场景>.csv`，例如 `Chat/ChatChannel.csv`、`Shop/ShopBuySell.csv`、`Combat/CombatDamage.csv`、`Npc/NpcDialog.csv`。这些名称是扩展约定，不表示对应场景已经实现或验证。

归属规则：按场景的主要验证目标划分子系统。例如击杀后检查掉落属于 Monsters，检查经验升级属于 Combat，检查治疗效果属于 Skills。登录和进图是这些场景的前置步骤，不必另拆为 Client/Server 两份脚本。每个 CSV 的步骤号独立，运行时先选文件，再用步骤范围选择片段。

编写规则：

- 每个业务脚本从准备好的初始状态开始，写清登录、进图等必要前置步骤，再执行本子系统的动作、结果断言、失败分支和持久化检查。
- 数据种子放在同目录的同名 `.fixture.csv` 中，例如 `Inventory/Inventory.fixture.csv`；辅助启动程序按文件名自动配对。
- `step` 是脚本内稳定、严格递增的步骤编号；新增脚本建议以 10 为间隔，便于插入检查。既有脚本保留编号，避免改变区间参数的含义。
- 动作返回 `submitted` 后，另用 `state.wait` 或数据库断言验证实际结果；服务器 `ready` 只能证明服务可用，不能代替业务结果断言。
- 一条长场景允许在客户端动作后立即查询服务器；`--from` / `--to` 选择该文件的步骤区间，`--log` 指定本次日志。截取中间区间时，调用者必须先满足该区间的前置状态。

例如执行物品子系统的指定区间：

```powershell
build-eiserver/Release/Test.exe Test/Inventory/Inventory.csv --client-pid 1234 --server-pid 5678 --database out/runtime/mir3.sqlite3 --from 200 --to 400 --log out/logs/inventory-200-400.log
```

执行器自身验证：`python Test/verify_runner.py`。Runner 目录含用于验证失败处理的预期失败 CSV，应通过该验证程序执行。

`Character/Character.csv` 覆盖创建角色：非法职业/性别/名字拒绝、重名服务器回包、失败恢复、创建女性法师、角色列表刷新、客户端人数上限、初始物品数据库检查及新角色进图。删除角色使用 `Character/CharacterDelete.csv`：密码拒绝、正常删除四张关联表、触发器故障回滚、其他账号数据保留及剩余角色进图。

删除请求为角色名/分区登录密码；当前账号后端使用 Mir2_Account 中的 FLD_PASSWORD。SQLite 新建脚本的技能表已与服务端逐技能记录格式对齐。已有旧版 FLD_LEVEL0..30 宽表数据库不会由 CREATE IF NOT EXISTS 自动迁移，需另行迁移后使用技能记录；本轮没有修改现有数据库。

`Skills/Skills.csv` 验证已学技能列表、名称/等级/修炼值、F12绑定与占用替换、非法绑定拒绝、退出保存及重新登录恢复。当前不包含施法效果、技能学习、升级或删除。

`Skills/SkillsCasting.csv` 经真实客户端与服务器验证火球术快捷键施法、两次扣蓝及回包、未绑定快捷键与蓝量不足拒绝。当前只覆盖地图坐标目标，不包含实体目标伤害、其他技能效果。`skill.cast` 参数为 `key`（1–12）、`x`、`y`（地图格坐标），复用客户端快捷键业务；`state.get` 的 `spell_reply_id` 记录已解析的施法回包技能，`cast_ready` 表示动作时序允许尝试施法，具体技能还需满足已绑定、蓝量等条件。等待断言失败期间，执行器仅在响应状态变化时输出 WAIT，便于定位回包和状态更新。

`Skills/SkillsHealing.csv` 配置真实怪物攻击，将角色 HP 从 16 降到 12，再通过客户端治疗术恢复到 16、MP 从 18 降到 12；断言受击、施法和血蓝回包及服务器存活。fixture 中怪物攻击间隔为 60 秒，行动间隔为 500 毫秒，避免治疗期间连续伤害。服务端恢复数据库怪物刷新；缺少或不完整的 `Map/searchTable.tbl` 使用按距离排列的默认 15 圈搜索偏移。暂未覆盖怪物死亡后刷新、掉落、所有怪物类型与技能目标伤害。

`Monsters/MonstersSpawn.csv` 使用数据库怪物编号 3、1007 和一个不存在的编号 1，验证只生成目标怪物，并查询名称、执行受伤治疗链路。`TBL_MONGEN.FLD_MONINDEX` 引用 `TBL_MONSTER.FLD_INDEX`，不依赖查询结果行号。客户端 `state.get` 增加 `actors`（id/name/x/y/dead）和 `actor_count`；`actor.query_name(index)` 使用当前 actors 数组的零基下标，走与鼠标悬停相同的名称查询协议。实体增减可能改变数组下标，调用前应读取当前状态。

`Combat/CombatMagic.csv` 使用真实火球术击杀低血量怪物，检查 SM_STRUCK、SM_NOWDEATH、客户端 HP=0/dead=true 和扣蓝，拒绝不存在或已死亡的目标。`skill.cast` 可以传 `key=1;target=0`，target 是当前 actors 数组的零基下标；客户端从选中实体读取坐标、网络标识和外观，调用与键盘相同的施法入口。省略 target 时继续使用 x/y 坐标施法。actors 增加 hp/max_hp，未收到血量回包时 max_hp=0 表示血量尚未知。该场景验证致死伤害，不验证精确伤害公式、掉落、经验和重新刷新。

`Combat/CombatExperience.csv` 验证击杀奖励、一次奖励跨多级、战斗退出冷却、数据库保存及重登恢复。fixture 奖励 1000 经验，按当前服务端经验表从 1 级升到 4 级并剩余 100 经验。state.get 增加 level、experience、next_experience、return_ready；return_ready 复用现有退出冷却判断，脚本通过 state.wait 等待，不跳过冷却或修改时钟。本次保持原经验表及等级索引，未校验其数值与原版资源是否一致。

`Monsters/MonstersLoot.csv` 验证击杀后普通药水掉落、真实行走到物品格、拾取、保存和重登恢复，同时检查不存在怪物的掉落记录与禁用掉落记录不会生成额外物品。`world.walk(x,y)` 只提交到相邻格的单步移动，复用地图通行检查、动作时序及现有移动协议；远距离请求拒绝。FLD_PROBABLY 使用 1/N 抽取，N<=0 禁用。本场景使用 N=1 确保结果可复现，尚未验证随机概率分布、金币和装备掉落、多个同类物品与背包满时的处理。

金币掉落使用 `Monsters/MonstersGold.csv`，检查掉落、行走拾取、SM_GOLDCHANGED、数据库余额和重登恢复；`Monsters/MonstersGoldCap.csv` 检查达到当前服务端 BAGGOLD=5000000 时金币留在地面、余额不变，重登仍可见。客户端 state.get 的 gold 是角色当前余额。金币数量为0的配置不生成金币，数量为1保证至少1金币；多条金币配置累加。本轮未验证金币随机数量分布、玩家主动丢钱或混合物品散落顺序，未进行图标视觉检查。

`Monsters/MonstersEquipment.csv` 验证击杀掉落装备、行走拾取、生成数据库身份、穿戴、保存及重登恢复。fixture 预置当天未补零的编号 9 和 10，断言新装备使用数值上的下一编号 00011。`Monsters/MonstersEquipmentFailure.csv` 用 SQLite 触发器拒绝装备 INSERT，检查背包与装备栏为空、数据库无新增记录，并在返回选角再进图后确认物品仍在地面。新掉落装备在数据库事务提交后才加入背包和发送拾取成功；失败恢复地面物品。当前验证使用 SQLite，尚未覆盖 ODBC 并发分配、当天编号耗尽或背包满的装备拾取。

Inventory 步骤编号调整：为补入真实行走与到达检查，原有步骤号统一乘以 10（原 20–40 对应 200–400）；新增 262/264 和 332/334 步行走到药水、装备掉落格。恢复地图搜索表后，主动丢弃物品散落在相邻格，不再假设物品位于角色脚下。调用该脚本的历史区间参数需按此换算。

`Inventory/InventoryDuplicates.csv` 验证两瓶相同属性药水具有不同的客户端 ID；丢弃第二瓶、真实行走拾回、使用第二瓶时第一瓶仍在，保存重登后剩余数量为 1，再使用并保存后为 0。普通物品离开背包或被消耗后旧 ID 失效，拾回时分配新 ID；脚本应读取 state.get 获取当前 ID。装备仍按完整数据库身份映射。当前未验证多份相同符咒或火把的装备槽位语义。

掉落数量场景：`Monsters/MonstersLootCount.csv` 验证同一配置生成两瓶药水、分别行走拾取、独立客户端 ID 和保存重登数量；`Monsters/MonstersEquipmentCount.csv` 验证生成两件装备、分别入库为 00011/00012，穿戴一件后另一件留在背包，保存重登保留两件。两份 fixture 同时包含数量为 0 的记录，断言无额外掉落。非金币配置命中一次概率后生成 FLD_COUNT 件独立物品；金币保持原随机数量规则。尚未验证大批掉落、周围格子全部被占用或背包满时的处理。

背包边界：Client/Server 使用 Shared/Mir3InventoryLimits.h 的 60 格容量；服务端只计算未穿戴物品。state.get 提供 bag_count、bag_capacity。`Inventory/InventoryFull.csv` 以 60 瓶药水和一件已穿戴装备验证满包拒绝拾取、地面物品保留、使用一瓶后拾取、完整保存及重登。尚未覆盖大尺寸物品造成的布局空间不足。

保存原子性：DBSvr 将角色、技能和普通物品写入同一事务，并持有数据库管理器锁，全部成功后才提交。`Inventory/InventorySaveFailure.csv` 通过触发器拒绝普通物品 INSERT，验证原有两瓶药水和角色位置保留、重登恢复上一次快照；保存失败会写服务端日志，当前没有新增客户端保存失败通知。

`Inventory/InventoryFullUnequip.csv` 验证满包卸装收到服务端 SM_TAKEOFF_FAIL，装备、外观和背包不变；使用一瓶药水后卸装成功，护甲进入背包，再穿戴腾出空位并拾取地面物品，最后保存重登验证。服务端卸装必须在清除装备槽前检查背包容量。

`Monsters/MonstersRespawn.csv` 使用 FLD_GENTIME=1（分钟）的真实刷新配置，击杀后等待新怪物出现、查询名称并再次击杀，再检查第二次补充刷新。刷新数量只统计未死亡且未进入 Ghost 状态的怪物；尸体和延迟释放对象仍按原时序保留，不阻塞刷新名额。场景约需两分钟，使用 state.wait 等待，不修改时钟。本场景验证两轮刷新与战斗，不覆盖尸体最终消失、Ghost 对象释放或长期运行稳定性。

`Monsters/MonstersCorpse.csv` 在击杀和一分钟补充刷新后等待原尸体消失，检查 SM_DISAPPEAR，随后攻击剩余的新怪物，确认移除旧实体未影响新实体。使用真实三分钟尸体时序；运行命令为 `python Test/launch_csv.py Test/Monsters/MonstersCorpse.csv --timeout-ms 190000 --log out/monsters-corpse.log`。超时参数只调整测试等待期限，不改变游戏计时。本场景不验证额外三分钟 Ghost 对象保留后的内存释放。

`Chat/ChatLocal.csv` 通过 chat.send(text) 调用客户端与回车相同的 OnMsgInputted 入口，检查 CM_SAY/SM_HEAR 以及实际聊天历史中的英文、中文回显。state.get 增加 chat_count（历史条数）、chat_last（最后一条文本，空历史为 null）。RPC 只接受非空单行文本，按本地编码最多 255 字节；超长、参数缺失、空字符串和换行输入返回 -32602。当前仅验证普通聊天回显，不代表多人广播、私聊、喊话、组队与行会聊天已完成。

喊话场景：`Chat/ChatShout.csv` 用 8 级角色发送 ! 前缀消息，验证 SM_CRY 与喊话文本；立即再次喊话后发送普通聊天作为处理完成的依据，断言喊话只收到 1 次、普通消息使用 SM_HEAR。`Chat/ChatShoutLevel.csv` 用 7 级角色验证等级不足提示和零喊话广播。当前验证发送者自身收到的广播及冷却内拒绝，未覆盖冷却结束后的再次喊话或多客户端距离边界。

双客户端私聊：`python Test/launch_csv.py Test/Chat/ChatWhisper.csv --clients 2 --log out/chat-whisper.log`。启动程序管理两个真实客户端；CSV 用 Client/Client2 交替登录不同账号、发送和断言消息，Test.exe 分别连接两个 PID 的 Client 管道。场景验证中文私聊、对方回复、错误角色名反馈，以及向已在线角色名加后缀发送时不会误投。普通聊天作为后续处理依据，随后精确检查私聊回包次数。仍未覆盖私聊拒收设置、跨服务器私聊或第三个旁观者的隔离。

私聊拒收：`python Test/launch_csv.py Test/Chat/ChatWhisperReject.csv --clients 2 --log out/chat-whisper-reject.log`。通过 chat.toggle_reception(channel=0) 复用聊天栏按钮入口，从 CmList.dat 读取 @拒绝悄悄话（1070）命令切换服务器拒收状态；服务器也支持选项入口的 @拒绝私聊（2126）。服务器回传客户端选项消息，state.get 的 chat_rejected 数组反映客户端实际确认状态。测试检查拒收提示、零私聊投递、普通聊天不受影响及恢复接收。拒收状态目前仅在本次角色会话内有效，未实现持久化、按名单拒收或其他聊天频道开关。

测试资源：辅助启动程序为 CmList.dat 与 SoundList.wwl 创建硬链接，优先使用 --config-dir 下的文件，缺失时从其 Data 资源目录的同级目录查找。chat.toggle_reception 的频道编号对应现有按钮：0 私聊、1 行会、2 所有人聊天；非法编号拒绝，缺少命令资源时也拒绝。当前服务器仅接通私聊拒收，行会及所有人聊天的开关仍待实现，不能将提交成功当作状态已确认。

私聊旁观隔离：`python Test/launch_csv.py Test/Chat/ChatWhisperIsolation.csv --clients 3 --log out/chat-whisper-isolation.log`。三个不同账号同图在线，检查指定收件人收到私聊及回复，而旁观者仅收到两次普通广播、私聊回包为 0。以之后的普通广播确认队列已处理，再精确断言两种消息计数，避免只检查最后一条文本而漏掉误投。

喊话距离：`Chat/ChatShoutRange.csv` / `Chat/ChatShoutRangeY.csv` 均用 `--clients 3` 运行，分别在横向和纵向放置距离 49 格、50 格的玩家。检查 49 格玩家收到 SM_CRY，50 格玩家的 SM_CRY/SM_HEAR 均为 0；之后发给边界玩家的私聊确认处理已完成，也证明私聊不受此范围限制。验证的是现有服务端同地图、两轴差值均小于 50 的规则；未校验该数值是否与原版服务器完全一致，也未覆盖跨地图和死亡角色过滤。

`Npc/NpcSpawn.csv` 通过 TBL_MERCHANT 加载真实商人，检查 actors[].npc、坐标、名称请求与回包。使用稀疏商人 ID，并放置一条不存在地图的记录，确认不会生成额外实体。服务端与客户端共用 MIR3_ACTOR_NPC 类型常量；旧服务端的类型 2 与客户端 NPC 类型 50 不一致。此场景尚未覆盖 NPC 点击、对话、选项或商店交易。

`Npc/NpcDialog.csv` 通过真实 npc.click / npc.select 入口验证 CM_CLICKNPC、CM_MERCHANTDLGSELECT 和 SM_MERCHANTSAY。state.get 的 npc_dialog 包含 open、id、rows 和选项冷却 ready；rows 来自客户端实际接收并解析的对话。33 步覆盖两页跳转、无效实体/选项拒绝、未支持条件页不回传，以及后续正常页面可用。

`Npc/NpcOptions.csv` 验证服务端只接受最近一次成功返回页面中的选项。隐藏标签、颜色控制标签内部的伪选项、前页过期选项和关闭后的请求均不触发页面；拒绝后合法选项仍可使用。`@main` / `@exit` 是当前NPC上下文中的特殊入口，不必显示在页面里；关闭后必须重新点击NPC。页面执行失败时保留原选项，切换NPC或关闭时清空，导入脚本中的选项随实际返回页面更新。

`npc.request(index,command)` 是专供协议校验的客户端RPC：index取当前actors数组下标，command最多255个本地编码字节；调用真实SendNPCMessage发送CM_MERCHANTDLGSELECT，故意不经过UI选项列表与冷却判断，便于测试服务端拒绝路径。通常游戏流程仍使用 `npc.click` / `npc.select`。测试用后续聊天回包作为同连接处理屏障，再核对页面回包数，避免把尚未处理误判为拒绝。

参考依据：`Mud3/Eiserver.exe`（SHA-256见Shop/native-price-audit.md）中的 `0x4E11C0` 检查角色当前对话选项列表，对 `@main` / `@exit` 放行；其他不存在的标签不执行页面。本轮未实现输入型 `@@` 参数、完整任务指令或出售/修理业务。


场景可附带同名 `.files` 目录，例如 `Npc/NpcDialog.files/Envir/Market_Def/NpcProbe-01.txt`。启动器将其中的相对目录结构复制到隔离运行目录。CSV 仍只存测试动作与断言，NPC 文本由服务端实际读取。

当前数据库商人的脚本路径为 `Envir/Market_Def/<FLD_SCRIPT或NPC名称>-<地图名>.txt`（本地代码页），支持 `[@main]` / `[@选项]`、`#SAY`、反斜杠换行及 `<文字/@选项>`。已支持 #IF/checklevel、#ELSESAY、#ACT/#ELSEACT 下的 goto 和 break；已接入 #CALL/#INCLUDE；数据库已支持独立脚本名映射；尚未直接导入参考服 merchant.txt，也未接入完整交易和任务指令。遇到未支持的条件或需要执行的指令时拒绝整个请求，不发送半段对话。本阶段不是完整旧脚本引擎。

`Npc/NpcLevelLow.csv`、`Npc/NpcLevelBoundary.csv`、`Npc/NpcLevelHigh.csv` 分别使用 4、5、6 级角色，验证 `checklevel 5` 门槛及同一 #IF 中多个条件取 AND。脚本通过 #ACT/#ELSEACT 的 goto 调用页面，返回后继续执行，break 结束当前页面；#SAY/#ELSESAY 仅输出命中的分支。当前 checklevel 参数仅支持非负整数，判断为角色等级大于等于参数。

每个等级场景还验证循环跳转、缺失标签、非数字等级参数不产生对话回包，之后正常页面仍可访问。单次请求最多执行 4096 行、嵌套 32 层，避免错误脚本占住游戏线程。以上是当前实现的联调覆盖；完整参考服任务脚本及全部指令语义仍待迁移核对。

`Npc/NpcImports.csv` 覆盖跨文件 #CALL、嵌套调用、#INCLUDE 中文文本、导入文件中的后续选项、跳回商人标签、缺失文件/标签、循环调用和文本段包含执行指令的拒绝。调用上下文属于角色当前交互的商人；失败请求不更新文件列表，也不发送部分文本。`Npc/NpcContext.csv` 在两个商人之间切换，双方都有 @next，验证不会沿用另一商人的导入脚本。

当前路径规则：`#CALL [\Library\dialog.txt] @start` 从 `Envir/QuestDiary/Library/dialog.txt` 加载；`#INCLUDE [..\Convert_Def\Library\text.txt] @welcome` 相对 `Envir/Market_Def` 解析。引用文件使用服务器本地代码页，文本标签的独立 `{` / `}` 包裹行不发送给客户端。#INCLUDE 读取文本段，不执行其内的 #ACT 等指令。共享执行行数和嵌套限制跨文件生效。完整任务状态、变量替换、所有引用路径变体及旧脚本语义仍未全部实现或验证。

`Npc/NpcMapping.csv` 使用 `TBL_MERCHANT.FLD_SCRIPT='StoryGuide'`，读取 `Envir/Market_Def/StoryGuide-01.txt`，NPC 显示名仍为 NpcProbe；同时放置同显示名的干扰脚本，验证配置优先。FLD_SCRIPT 填脚本基础名，不含 `-地图.txt` 后缀。空值或缺少此列时使用 NPC 名称；显式配置的文件不存在时不自动回退。`Npc/NpcMissingScript.csv` 验证后一种情况。

`Npc/NpcLegacySchema.csv` 在启动前删除 FLD_SCRIPT 列，验证旧表仍能加载和对话。新建 SQLite/SQL Server 表定义已增加该列；不会自动修改现有数据库。已有 SQLite 数据库若需配置独立脚本名，可执行一次 `ALTER TABLE TBL_MERCHANT ADD COLUMN FLD_SCRIPT TEXT NOT NULL DEFAULT '';`。

`@exit` 是当前商人的退出选项：服务端发送 SM_MERCHANTDLGCLOSE，并清除角色的对话脚本上下文。映射和旧表场景均验证关闭后窗口/行/身份重置、关闭后的选项拒绝、重新点击打开，以及两次关闭的精确回包计数。实际构建及联调使用 SQLite，SQL Server 新建表定义尚未在 ODBC 环境执行验证。

商店基础链路：商人 `@buy` 页面成功显示后，服务端读取同一脚本 `[Goods]` 中的 `物品名 数量 补货小时` 行，按 TBL_STDITEM_ETC 的名称匹配普通物品，以数据库 FLD_PRICE 和脚本头部 % 倍率计算单价后发送 SM_SENDGOODSLIST。`shop.buy(index,quantity)` 复用客户端商品分类与 BuyBulk 入口，实际发送 CM_USERBUYITEM；state.get 的 shop 来自真实商品列表与通知窗口。

`ShopBuy.csv` 验证两件同名药品购买、金币不足拒绝、关闭商店、数据库保存和重登恢复。`ShopStock.csv` 验证超库存拒绝、买空及买空后的再次拒绝；`ShopCapacity.csv` 使用59格背包购买2件，`ShopWeight.csv` 使用超负重商品，检查整笔拒绝、金币/库存/物品不变。购买仅在当前商人已打开商店且角色同地图、在视野内时处理。扣款前检查完整数量对应的库存、容量、负重和总价；成功后更新金币、独立物品实例、负重及购买者商品列表。notice.dismiss 可通过既有确认入口关闭普通提示，不提交输入或确认交易对话框。

本阶段支持普通物品初始库存及定时补货，库存尚未跨服务器重启持久化，已支持装备明细与单件出售，但尚未实现装备修理或所有在线顾客的即时库存广播。物品和金币沿用角色保存事务，已验证正常保存重登；尚未验证购买过程中的服务器崩溃恢复。

`Shop/ShopPrice.csv` 使用标准价11和 `%150`，报价单价18，购买2件扣36；验证余额4的保存重登。单价按参考服 `base / 100 * percent * 1.1` 的 x87 扩展精度顺序计算，再取最近整数，报价和扣款共用单价。`ShopLowPrice.csv` 验证 `%0` 被忽略，`ShopInvalidPrice.csv` 验证非数字倍率保留默认100；`ShopPriceBelowMinimum.csv` / `ShopPriceMinimum.csv` 验证54/55边界；`ShopPriceRetained.csv` 验证无效、过低和溢出的倍率不覆盖先前有效值。`ShopZeroPrice.csv` 验证零基础价可以报价但不能购买。`ShopPriceRounding.csv` 验证基础价5/15/25/35对应单价6/17/28/38及按单价乘数量扣款。

`python Test/launch_csv.py Test/Shop/ShopSharedStock.csv --clients 2 --log out/shop-shared-stock.log` 启动两个真实角色，双方看到同一初始库存后交替购买。第一位买2件后，第二位按旧列表购买2件被服务器拒绝；重新打开商店获得真实剩余1件并买走；第一位的过期购买也被拒绝。保存后检查总物品数3与双方余额78/89。该场景证明交替请求使用共享服务端库存，不是并发压力测试；尚未向其他顾客主动广播库存变化。

`Shop/ShopRestock.csv` 运行两个真实31秒等待：0小时商品先买空再补满、部分售出再恢复目标数量，1小时商品在两轮内维持剩余库存；随后购买补货商品并验证金币与8件物品保存。后台每30秒检查，商品间隔按小时换算；补货不会主动广播，重新进入购买页面获取当前库存。购买、商品列表和补货共享每个商人的互斥锁，避免NPC线程与玩家线程同时修改库存。

参考服 `0x4CCB4C` 每超过30000毫秒调用补货函数 `0x4CD3EC`；后者在商品间隔到期时更新计时、按目标与现存差值增加或减少库存。当前普通商品直接恢复目标数。使用64位单调计时避免32位溢出；负补货小时不作为有效商品加载。初始库存仍在首次打开商店时加载；未接入原服库存文件、出售商品回收或重启持久化。本场景未等待完整一小时，不能作为一小时间隔到期的实测证据。

本轮补货测试还定位并修复了旧NPC调度问题：CCharObject未初始化m_nViewRange，商人首次SearchViewRange可能扫描巨大的坐标区间、占住NPC线程。基类现在使用已有_RANGE_X初始化默认视野；玩家和怪物各自的显式视野配置仍生效。

普通物品单件出售已接入服务端，参考服收购类型、半价取整和消息字段见 [出售协议核对](Shop/native-sale-audit.md)。

`shop.quote(id)` 通过真实背包询价入口选择实例，`shop.sell` 确认已报价物品；`state.get.merchant_service` 返回actor、mode、item、price和phase。`shop.sale_request(index,id,name)` 为协议测试入口，可发送正常UI无法产生的失效实例请求，服务端始终重新验证当前出售上下文、地图距离、物品归属、名称和未装备状态。

`ShopSell.csv` / `ShopSellDefault.csv` 验证%150/%100下收购价同为6（基础价11）、指定第二件同名物品、重复请求拒绝、原属性回购、保存重登；`ShopSellNewStock.csv` 验证收购物品不在[Goods]中时创建回购分类。`ShopSellRoundDown/Even/RoundUp.csv` 覆盖9/13/15的半价取偶数。`ShopSellGoldCap.csv`、`ShopSellRejectedType.csv` 验证金币上限和类型拒绝不丢物品；`ShopSellPowderLow/PowderBoundary/CandleLow.csv` 验证类型25/30的4000耐久门槛（这些场景标准耐久为0，未覆盖耐久折价计算）。

本阶段支持现有独立实例的普通物品单件出售，已支持普通物品耐久折价和实例回购，装备及附加属性计价现已接入，尚未支持数量式堆叠出售；标准价只对已覆盖的普通物品报价路径验证。耐久非零的收购物品以原记录进入库存，回购保留原属性；库存列表不主动广播，仍随重新打开商店刷新。未配置[Goods]的收购物品没有定时补货目标，尚未实现原服库存文件保存及超额回收清理。

`ShopSellContext.csv` 通过真实发包验证尚未打开出售模式、重新点击NPC导致旧出售上下文失效两种拒绝，之后重新打开出售仍可正常成交。


出售边界场景：`Shop/ShopSellZeroDurability.csv` 验证普通物品耐久为0仍可成交，但不加入商人库存；随后买完原有3件，检查余额52、背包4件及保存重登，数据库中售出的零耐久记录为0。此规则不覆盖25/30类型的最低品质限制。

`Shop/ShopSellZeroQuote.csv` 使用基础价1（半价取偶数为0），验证界面进入Unavailable且正常shop.sell拒绝，再用shop.sale_request发送真实协议探测。服务端接受零价成交，重复同实例请求拒绝，商人库存增加1，保存重登后金币100、背包1件。探测绕过客户端Paying阶段，因此不以当前客户端背包即时变化判断成功，而以成功/失败回包、商人库存、数据库及重登结果交叉验证；不改变正常UI的零报价限制。


普通物品标准耐久：`TBL_STDITEM_ETC.FLD_DURAMAX`独立于药效FLD_VAL1/FLD_VAL2，按16位字段发送；旧表没有此列时取0。SQLite/SQL Server新建表已加入该列，已有SQLite表可执行 `ALTER TABLE TBL_STDITEM_ETC ADD COLUMN FLD_DURAMAX INTEGER NOT NULL DEFAULT 0;`。本轮未自动迁移已有数据库，也未执行SQL Server实测。

客户端state.get的bag/equipment/held_item物品记录增加`mode`、`standard_durability`、`durability`、`max_durability`，来自实际接收的物品属性。`Inventory/InventoryStandardDurability.csv`验证标准耐久1/65535与实例耐久8000、4000/8000互相独立；真实怪物造成伤害后使用药水，确认按药效恢复满血，再保存重登检查剩余物品。`Inventory/InventoryLegacyStandard.csv`删除新列后运行同样流程，确认旧数据库回退标准耐久0而不影响药效或实例属性。

普通物品生成现已使用非零标准耐久，标准耐久为0时保留旧初值规则。12字节实例键增加G~扩展格式，支持完整16位字段；新旧存档兼容及业务覆盖见[普通物品实例键](Inventory/general-item-key.md)。耐久报价仍待接入。

### 普通物品实例与耐久价格

`shop.details(index)` 打开商品分类详情，`shop.page(page)` 请求从0开始的页码，`shop.buy_offer(index)` 购买当前页指定实例。均复用客户端商店入口。`state.get` 的 `shop.offers` 返回实例ID、名称、价格、当前耐久；另有 `offer_count/details/loading/page`。`shop.buy_request(index,id,name)` 仅用于过期实例等协议拒绝测试。

每页最多10件；刷新保留未成交实例ID，补货创建新ID。出售和回购均按真实耐久计算，类型40肉、43矿石采用各自预处理。FLD_STACKMODE 缺列或 NULL 时使用旧类型推断，显式0表示不堆叠。批量路径目前仍生成独立记录，不代表已支持真实数量堆叠。

ShopDurability/MeatPrice/MeatPremium/OrePrice/OreMinimum/MaximumLoss/PowderPrice 验证不同计价、成交和保存重登；ShopStackPriceBypass 验证堆叠模式跳过折价；ShopDetailPages/Shared/Restock/Capacity/Weight/Gold 验证分页、争买、补货及失败保留。双客户端 Shared 场景需 `--clients 2`。价格详情使用16位协议字段，超出范围拒绝；装备附加属性计价现已接入；原服溢出行为未模拟。详见 [耐久计价核对](Shop/native-durability-audit.md)。

`Inventory/InventoryEquipmentAttributes.csv` 为装备的14个附加属性设置各不相同的值，经过真实客户端丢弃、拾取、保存与重登后检查数据库逐项保留，并检查当前/最大耐久。它覆盖了加载遗漏 FLD_VALUE14、转移 SQL 把 FLD_VALUE5 写成 VALUE6 的回归；不代表装备出售或全部附加属性显示已完成。

装备回包状态新增 `upgrades`（14字节原始附加属性）、`base_defence`、`base_magic_defence`、`base_attack`，均读取客户端实际收到的100字节记录。InventoryEquipmentAttributes / InventoryWeaponAttributes 在背包、穿脱、丢弃拾取及重登时断言这些字段；武器模式5/6的强度附加值最多发送5，数据库原值不变。基础属性与附加属性分开发送，避免客户端提示把已合并值再次当作基础值。此处尚未覆盖所有魔法/元素字段或角色战斗能力中的附加值应用。

`InventoryEquipmentDropFailure.csv` 用 SQLite 触发器拒绝装备转移更新；`InventoryEquipmentDropIgnored.csv` 用 RAISE(IGNORE) 使更新成功执行但影响零行。两者都验证连续两次丢弃收到失败回包、原背包实例恢复、无地面广播、数据库所有权保留和保存重登不复制装备。服务端在地面物品广播前检查持久化结果，失败则移除临时地面记录；普通物品仍沿用角色保存路径。这里不代表服务器崩溃时内存地面库存能够恢复。

### 装备商店

`ShopEquipment.csv`、`ShopWeapon.csv`、`ShopAccessory.csv` 覆盖护甲、武器、首饰的耐久与附加属性报价、指定实例出售、保留原身份回购、新库存生成独立身份、过期请求拒绝和保存重登。商人 [Goods] 同时查找普通物品表与装备标准表；装备库存使用单件详情，收购仍要求 +类型。

`ShopEquipmentSaleFailure`、`ShopEquipmentRebuyFailure`、`ShopEquipmentCreateFailure` 注入数据库故障，验证不转移金币、不丢失背包/库存及不生成多余身份。`ShopEquipmentShared` 使用 `--clients 2` 验证争买；`ShopEquipmentRestock` 等待真实补货，验证新库存ID和旧ID拒绝。

装备计价的附加量为前三个无符号附加字节之和，武器类型5/6还加第7字节超过10的部分；先执行整数基础价/5的加价，再计算最大耐久缩放与损耗。原始14项附加值完整保留，客户端强度裁剪只发生在发送副本。

装备数据库身份/所有权与成交后金币现已在同一事务内提交；提交失败不改变内存身份、背包、库存或金币。商人库存尚未跨服务器重启保存，新装备目前按标准耐久生成且附加值为0，未接入随机极品生成及修理业务；完整原服交易限制仍需继续核对。

装备成交事务增加 `ShopEquipmentSaleGoldFailure`、`ShopEquipmentRebuyGoldFailure`、`ShopEquipmentCreateGoldFailure`、`ShopEquipmentCreateGoldIgnored`，分别在物品已写入事务后拒绝金币更新，检查物品与金币同时回滚；最后一项覆盖更新影响零行而不报SQL异常的路径。正常装备场景也在角色保存前读取数据库金币。

重启验证运行 `python Test/verify_equipment_recovery.py --log-dir out/equipment-recovery`。脚本在出售、回购、新购及两种混合交易完成后截断CSV，先强制终止测试服务器，再终止客户端，避免断线保存；随后复用同一数据库运行对应 `ShopEquipmentRecovery*.csv`，检查金币、物品数量、独立身份与归属。恢复CSV需要此前的运行状态，不能当作独立新环境场景。

辅助启动器新增 `--resume-runtime out/rpc-xxx`：复用一个已停止的隔离环境，跳过SQL种子和资源覆盖，仍启动并清理自己的进程。恢复时使用该目录原有客户端与配置，以及 `--bin-dir` 中的服务器/Test.exe；保持两端版本一致。

这只验证装备成交的数据库事务和进程重启恢复，不代表整个游戏世界已具备统一崩溃恢复：商人库存未持久化，其他未保存的普通物品、地图及角色状态仍需统一处理。SQL Server事务路径尚未实测。

混合交易：`ShopMixedTrade` 先购买普通物品再购买装备，`ShopMixedSale` 先购买普通物品再出售装备。装备成交事务现在同时替换该角色的普通物品快照，使用 GeneralItemKey 编码当前属性，保留同属性重复记录及16位耐久，并检查其他角色数据不变。

`ShopMixedDeleteFailure/DeleteIgnored/InsertFailure/InsertIgnored` 覆盖快照删除/插入报错和忽略操作，失败后装备、金币及旧普通物品快照一起回滚。删除后查询剩余记录，防止忽略删除后继续插入形成重复；插入必须影响一行。

`verify_equipment_recovery.py` 现执行五种交易恢复，混合场景分别使用 `ShopMixedRecovery` / `ShopMixedSaleRecovery`。普通物品交易本身仍沿用原有保存时机；本次同步的是装备成交时的普通物品和金币快照，尚未覆盖全部角色属性、装备栏引用或商人库存持久化。


### 选角回包与连续返回

`Login/LoginReturnShared.csv` 使用两个真实客户端，各执行三轮返回选角和重新进图，逐轮检查账号对应的角色列表，以及 CM_QUERYCHR / SM_QUERYCHR 精确次数。执行：

```powershell
python Test/launch_csv.py Test/Login/LoginReturnShared.csv --clients 2 --timeout-ms 15000 --log out/login-return-shared.log
```

网关 TCP 分片由 `Test/Login/GateFrames.cpp` 验证：遍历每个分割位置、逐字节输入、带 NUL 填充的连续多包、控制包和残包续接。构建 `test_gate_frames` 后运行 `build-eiserver/Release/test_gate_frames.exe`。测试直接调用选角网关使用的拆包实现；真实 CSV 验证两端流程，但不保证操作系统实际生成某种 TCP 分片。


真实网关分片验证：`python Test/verify_gate_stream.py --log-dir out/login-fragments`。该辅助程序创建隔离运行目录，在 LoginSvr→LoginGate、DBSvr→SelGate 回包方向加入本地 TCP 代理，每次发送一个字节并保留 NUL 填充，然后执行现有 Login CSV。日志目录包含 `test.log` 和 `launcher.log`；退出码保留场景失败结果。TCP 接收端仍可能合并相邻写入，所有精确分割位置由 GateFrames 单元测试覆盖。

双客户端分片回归使用：

```powershell
python Test/verify_gate_stream.py --scenario Test/Login/LoginReturnShared.csv --clients 2 --log-dir out/login-return-fragments
```

代理只改变测试环境的传输分段，不修改游戏时钟、业务回包或角色数据。默认单步骤等待 15 秒，包含正常页面过渡动画；不宜将完整登录场景的等待期限缩短到 2 秒。


分片方向可用 `--direction requests|replies|both` 选择，默认 `replies`。`requests` 检查网关到 LoginSvr/DBSvr 的请求分片，`both` 同时分片请求和回包。程序分别输出每条链路的请求/回包单字节写入次数，并要求所选方向实际传输过数据。例如：

```powershell
python Test/verify_gate_stream.py --direction both --scenario Test/Character/CharacterDelete.csv --log-dir out/character-delete-fragments
```
