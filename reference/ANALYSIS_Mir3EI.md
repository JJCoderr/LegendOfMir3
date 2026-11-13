# legendOfMir2_src 功能分析

本地分析日期：2026-11-14；基准提交：`e7c47bb2375ddcc74e9f480e613aad79e66a7c08`。

传统 C++ 客户端，可优先用于 1.45 窗口、资源索引和行为交叉核对。仓库名虽然带 Mir2，实际客户端位于 Mir3EIClientDev；不是 Delphi 项目。

## 按功能定位

| 功能 | 首先读取 | 需要确认的内容 |
| --- | --- | --- |
| 主界面与输入 | [Interface.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/Interface.cpp)、[GameProc.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/GameProc.cpp)、[GameWnd.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/GameWnd.cpp)、[GameBtn.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/GameBtn.cpp) | 窗口所有者、按钮状态、事件消费和拖动坐标 |
| 背包与物品 | [InventoryWnd.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/InventoryWnd.cpp)、[Item.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/Item.cpp) | 格子坐标、拖放、物品显示和请求边界 |
| 人物与魔法 | [StatusWnd.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/StatusWnd.cpp)、[Magic.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/Magic.cpp)、[PlayerChar.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/PlayerChar.cpp) | 属性窗口与角色动作分开；魔法特效不等于技能列表 |
| 聊天与组队 | [ChatPopWnd.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/ChatPopWnd.cpp)、[GroupWnd.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/GroupWnd.cpp)、[GroupPopWnd.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/GroupPopWnd.cpp) | 展开/弹窗、组员列表、点击动作 |
| 行会 | [GuildWnd.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/GuildWnd.cpp)、[GuildMasterWnd.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/GuildMasterWnd.cpp) | 普通成员和管理者窗口分开 |
| NPC与商店 | [NPCWnd.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/NPCWnd.cpp)、[StoreWnd.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/StoreWnd.cpp) | 对话解析、菜单选择、商店条目 |
| 交易与任务 | [ExchangeWnd.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/ExchangeWnd.cpp)、[QuestWnd.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/QuestWnd.cpp) | 双方交易状态、任务文本及选择 |
| 骑马与选项 | [HorseWnd.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/HorseWnd.cpp)、[OptionWnd.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/OptionWnd.cpp) | 按钮图案、按下/释放、配置读取 |
| 地图与角色 | [MapHandler.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/MapHandler.cpp)、[Actor.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/Actor.cpp)、[SpriteInfo.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/SpriteInfo.cpp) | 地图加载、帧索引、方向、遮挡和动作表 |
| 快捷栏与消息头 | [BeltWnd.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/BeltWnd.cpp)、[DecodeMessage.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/DecodeMessage.cpp) | DecodeMessageHead 使用外部解码符号；不可直接替换当前协议 |

## 已核对的规则与迁移方向

- `HorseWnd.cpp` 的 `CHorseWnd::CreateHorseWnd` 使用 630/631 按钮帧，位置为窗口起点加 (567,413)；`OnMouseMove` 再按窗口当前位置更新按钮矩形。迁移时同时核对位置更新和命中区域，不只复制贴图坐标。
- `CHorseWnd::OnLButtonDown/OnLButtonUp` 只是按钮事件入口，不能把它视为服务端骑乘成功。继续沿窗口所有者的调用处追踪请求和回包。
- `Actor.cpp` 用服装编号乘 `_MAX_HERO_FRAME` 再加动作起始帧和方向偏移。先看 `SpriteInfo.cpp` 的动作定义，再决定本工程的动作帧，不能套现代版本的帧距。

## 交叉核对顺序

窗口先读表内专用 cpp/h，再找 Interface/GameProc 的调用点；只有涉及请求时才追消息分发。地图先读 MapHandler，再读 Actor；不要把整个 GameProc 搬进新客户端。旧注释编码可能不是 UTF-8，分析时以符号和语句为准，不批量转码参考仓库。

`copy_Actor.cpp` 与 `Actor.cpp` 同时存在；优先查工程文件的编译项，不默认 copy 文件有效。`DecodeMessage.cpp` 含 `sub_42F760`、`SetWord` 外部符号，必须一并确认依赖和协议版本。


## 服务器、网关和资源模块

| 模块 | 源码入口 | 查询和复用边界 |
| --- | --- | --- |
| 服务器入口 | [GameSvr.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/GameSvr/GameSvr.cpp)、[ObjectEngine.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/GameSvr/ObjectEngine.cpp) | 先定位主循环和对象处理；UI 线程与消息线程不能等同 |
| 玩家、怪物、NPC | [ObjectPlayer.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/GameSvr/ObjectPlayer.cpp)、[ObjectMonster.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/GameSvr/ObjectMonster.cpp)、[ObjectNPC.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/GameSvr/ObjectNPC.cpp) | 属性与动作归服务器对象；客户端只表示已接收状态 |
| 商店、脚本与事件 | [ObjectMerchant.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/GameSvr/ObjectMerchant.cpp)、[ObjectScripter.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/GameSvr/ObjectScripter.cpp)、[ProcessEvents.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/GameSvr/ProcessEvents.cpp) | NPC 对话命令继续追服务端条件、执行结果和回包 |
| 消息处理 | [ProcessUserMsg.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/GameSvr/ProcessUserMsg.cpp)、[SockMsg_GateComm.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/GameSvr/SockMsg_GateComm.cpp)、[Protocol.h](legendOfMir2_src/Mir3EIServerDev/Mir3server/Def/Protocol.h) | 先找请求定义，再到分发和功能处理；分清外层网关头与游戏消息头 |
| 登录与选角网关 | [ClientSockMsg.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/LoginGate/ClientSockMsg.cpp)、[ClientSockMsg.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/SelGate/ClientSockMsg.cpp)、[ServerListProc.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/LoginSvr/ServerListProc.cpp) | 连接接入与服务器列表；配置/端口以目标版本为准 |
| 游戏网关 | [ClientSockMsg.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/GameGate/ClientSockMsg.cpp)、[ServerSockMsg.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/GameGate/ServerSockMsg.cpp)、[ThreadFuncForMsg.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/GameGate/ThreadFuncForMsg.cpp) | 会话映射、消息缓冲、客户端侧/服务器侧方向分开 |
| 数据库 | [ProcessDBMsg.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/DBSvr/ProcessDBMsg.cpp)、[TableList.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/DBSvr/TableList.cpp)、[dbmgr.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/Def/dbmgr.cpp)、[SQL_Scripts](legendOfMir2_src/Mir3EIServerDev/SQL_Scripts) | SQL 初始化脚本与数据库操作一起核对，不能只复制物品表结构 |
| 物品、魔法数据 | [Handler_Item.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/GameSvr/Handler_Item.cpp)、[Handler_Magic.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/GameSvr/Handler_Magic.cpp)、[Tables.h](legendOfMir2_src/Mir3EIServerDev/Mir3server/GameSvr/Tables.h) | 加载定义、数据结构与运行时实例分开 |
| 地图 | [Handler_Map.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/GameSvr/Handler_Map.cpp)、[MapHandler.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/MapHandler.cpp) | 服务端可通行/对象管理与客户端地图绘制分别检查 |
| 图像资源 | [ImageHandler.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/Common/ImageHandler.cpp)、[WHWilImage.cpp](legendOfMir2_src/Mir3EIClientDev/WindHorn/WHWilImage.cpp) | 库名/编号映射、WIL/WIX 索引和图像解码；资源留在原目录 |
| 编码/解码 | [EnDecode.cpp](legendOfMir2_src/Mir3EIServerDev/Mir3server/Def/EnDecode.cpp)、[EnDecode.h](legendOfMir2_src/Mir3EIServerDev/Mir3server/Def/EnDecode.h) | 6-bit 编码与真正加密不同；不要替换成标准 Base64 |

### 已核对的消息边界

`Def/EnDecode.cpp` 的 fnEncode6BitBufW 将六位值加 0x3c 输出；fnDecode6BitBufW 做对应还原。它是文本传输编码，不能单凭文件名叫作加密算法。迁移时需按 ANSI/Unicode 分支、字节长度与截断行为分别核对。

`GameGate/ClientSockMsg.cpp` 的 ProcMakeSocketStr 从 _TMSGHEADER 取 wUserGateIndex，再在 g_UserInfoArray 查 CSessionInfo 并使用该会话 SendBuffer。网关会话索引不是角色 ID；重做网络接入时必须维持两者边界。

### 具体任务的读取顺序

背包动作：客户端 InventoryWnd/Item → GameProc 消息发送 → Protocol.h → ProcessUserMsg → ObjectPlayer/Handler_Item → 应答更新。NPC 动作：NPCWnd → 用户消息分发 → ObjectNPC/ObjectMerchant/ObjectScripter。地图切换：客户端 MapHandler 和服务端 Handler_Map 分开验证。以上是定向追踪入口，尚未证明每一个消息编号和字段都与宝藏 1.45 一致。

## 使用与验证边界

本轮为源码导航与代表性路径分析，没有构建或运行该参考工程。表中入口已核对存在；不能据此宣称整套逻辑与 1.45 一致。读取目标函数及直接调用者后，再迁移到 `client` 的对应功能模块。只验证本次影响的显示和行为，相关功能成组完成后统一截图与提交。
