# LegendOfMir3_Src 功能分析

本地分析日期：2026-11-14；基准提交：`5140eef5e03dd9388851423171ccbc80c8556848`。

传统 C++ 客户端，可优先用于 1.45 窗口、资源索引和行为交叉核对。客户端、服务器分别位于 LegendOfMir3_Client、LegendOfMir3_Server。

## 按功能定位

| 功能 | 首先读取 | 需要确认的内容 |
| --- | --- | --- |
| 主界面与输入 | [Interface.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/Interface.cpp)、[GameProc.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/GameProc.cpp)、[GameWnd.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/GameWnd.cpp)、[GameBtn.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/GameBtn.cpp) | 窗口所有者、按钮状态、事件消费和拖动坐标 |
| 背包与物品 | [InventoryWnd.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/InventoryWnd.cpp)、[Item.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/Item.cpp) | 格子坐标、拖放、物品显示和请求边界 |
| 人物与魔法 | [StatusWnd.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/StatusWnd.cpp)、[Magic.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/Magic.cpp)、[PlayerChar.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/PlayerChar.cpp) | 属性窗口与角色动作分开；魔法特效不等于技能列表 |
| 聊天与组队 | [ChatPopWnd.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/ChatPopWnd.cpp)、[GroupWnd.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/GroupWnd.cpp)、[GroupPopWnd.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/GroupPopWnd.cpp) | 展开/弹窗、组员列表、点击动作 |
| 行会 | [GuildWnd.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/GuildWnd.cpp)、[GuildMasterWnd.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/GuildMasterWnd.cpp) | 普通成员和管理者窗口分开 |
| NPC与商店 | [NPCWnd.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/NPCWnd.cpp)、[StoreWnd.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/StoreWnd.cpp) | 对话解析、菜单选择、商店条目 |
| 交易与任务 | [ExchangeWnd.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/ExchangeWnd.cpp)、[QuestWnd.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/QuestWnd.cpp) | 双方交易状态、任务文本及选择 |
| 骑马与选项 | [HorseWnd.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/HorseWnd.cpp)、[OptionWnd.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/OptionWnd.cpp) | 按钮图案、按下/释放、配置读取 |
| 地图与角色 | [MapHandler.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/MapHandler.cpp)、[Actor.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/Actor.cpp)、[SpriteInfo.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/SpriteInfo.cpp) | 地图加载、帧索引、方向、遮挡和动作表 |

## 已核对的规则与迁移方向

- `HorseWnd.cpp` 的 `CHorseWnd::CreateHorseWnd` 使用 630/631 按钮帧，位置为窗口起点加 (567,413)；`OnMouseMove` 再按窗口当前位置更新按钮矩形。迁移时同时核对位置更新和命中区域，不只复制贴图坐标。
- `CHorseWnd::OnLButtonDown/OnLButtonUp` 只是按钮事件入口，不能把它视为服务端骑乘成功。继续沿窗口所有者的调用处追踪请求和回包。
- `Actor.cpp` 用服装编号乘 `_MAX_HERO_FRAME` 再加动作起始帧和方向偏移。先看 `SpriteInfo.cpp` 的动作定义，再决定本工程的动作帧，不能套现代版本的帧距。

## 交叉核对顺序

窗口先读表内专用 cpp/h，再找 Interface/GameProc 的调用点；只有涉及请求时才追消息分发。地图先读 MapHandler，再读 Actor；不要把整个 GameProc 搬进新客户端。旧注释编码可能不是 UTF-8，分析时以符号和语句为准，不批量转码参考仓库。

`Actor.cpp` 的绘制顺序已核对：Weapon.ord 为真时先武器，再身体、头发，为假时最后绘制武器。身体与头发使用服装/头发颜色，武器传入 0xFFFF。本工程人物分层与染色据此交叉验证。


## 服务器、网关和资源模块

| 模块 | 源码入口 | 查询和复用边界 |
| --- | --- | --- |
| 服务器入口 | [GameSvr.cpp](LegendOfMir3_Src/LegendOfMir3_Server/GameSvr/GameSvr.cpp)、[ObjectEngine.cpp](LegendOfMir3_Src/LegendOfMir3_Server/GameSvr/ObjectEngine.cpp) | 先定位主循环和对象处理；UI 线程与消息线程不能等同 |
| 玩家、怪物、NPC | [ObjectPlayer.cpp](LegendOfMir3_Src/LegendOfMir3_Server/GameSvr/ObjectPlayer.cpp)、[ObjectMonster.cpp](LegendOfMir3_Src/LegendOfMir3_Server/GameSvr/ObjectMonster.cpp)、[ObjectNPC.cpp](LegendOfMir3_Src/LegendOfMir3_Server/GameSvr/ObjectNPC.cpp) | 属性与动作归服务器对象；客户端只表示已接收状态 |
| 商店、脚本与事件 | [ObjectMerchant.cpp](LegendOfMir3_Src/LegendOfMir3_Server/GameSvr/ObjectMerchant.cpp)、[ObjectScripter.cpp](LegendOfMir3_Src/LegendOfMir3_Server/GameSvr/ObjectScripter.cpp)、[ProcessEvents.cpp](LegendOfMir3_Src/LegendOfMir3_Server/GameSvr/ProcessEvents.cpp) | NPC 对话命令继续追服务端条件、执行结果和回包 |
| 消息处理 | [ProcessUserMsg.cpp](LegendOfMir3_Src/LegendOfMir3_Server/GameSvr/ProcessUserMsg.cpp)、[SockMsg_GateComm.cpp](LegendOfMir3_Src/LegendOfMir3_Server/GameSvr/SockMsg_GateComm.cpp)、[Protocol.h](LegendOfMir3_Src/LegendOfMir3_Server/Def/Protocol.h) | 先找请求定义，再到分发和功能处理；分清外层网关头与游戏消息头 |
| 登录与选角网关 | [ClientSockMsg.cpp](LegendOfMir3_Src/LegendOfMir3_Server/LoginGate/ClientSockMsg.cpp)、[ClientSockMsg.cpp](LegendOfMir3_Src/LegendOfMir3_Server/SelGate/ClientSockMsg.cpp)、[ServerListProc.cpp](LegendOfMir3_Src/LegendOfMir3_Server/LoginSvr/ServerListProc.cpp) | 连接接入与服务器列表；配置/端口以目标版本为准 |
| 游戏网关 | [ClientSockMsg.cpp](LegendOfMir3_Src/LegendOfMir3_Server/GameGate/ClientSockMsg.cpp)、[ServerSockMsg.cpp](LegendOfMir3_Src/LegendOfMir3_Server/GameGate/ServerSockMsg.cpp)、[ThreadFuncForMsg.cpp](LegendOfMir3_Src/LegendOfMir3_Server/GameGate/ThreadFuncForMsg.cpp) | 会话映射、消息缓冲、客户端侧/服务器侧方向分开 |
| 数据库 | [ProcessDBMsg.cpp](LegendOfMir3_Src/LegendOfMir3_Server/DBSvr/ProcessDBMsg.cpp)、[TableList.cpp](LegendOfMir3_Src/LegendOfMir3_Server/DBSvr/TableList.cpp)、[dbmgr.cpp](LegendOfMir3_Src/LegendOfMir3_Server/Def/dbmgr.cpp)、[SQL_Scripts](LegendOfMir3_Src/LegendOfMir3_Server/SQL_Scripts) | SQL 初始化脚本与数据库操作一起核对，不能只复制物品表结构 |
| 物品、魔法数据 | [Handler_Item.cpp](LegendOfMir3_Src/LegendOfMir3_Server/GameSvr/Handler_Item.cpp)、[Handler_Magic.cpp](LegendOfMir3_Src/LegendOfMir3_Server/GameSvr/Handler_Magic.cpp)、[Tables.h](LegendOfMir3_Src/LegendOfMir3_Server/GameSvr/Tables.h) | 加载定义、数据结构与运行时实例分开 |
| 地图 | [Handler_Map.cpp](LegendOfMir3_Src/LegendOfMir3_Server/GameSvr/Handler_Map.cpp)、[MapHandler.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/MapHandler.cpp) | 服务端可通行/对象管理与客户端地图绘制分别检查 |
| 图像资源 | [ImageHandler.cpp](LegendOfMir3_Src/LegendOfMir3_Client/Common/ImageHandler.cpp)、[WHWilImage.cpp](LegendOfMir3_Src/LegendOfMir3_Client/WindHorn/WHWilImage.cpp) | 库名/编号映射、WIL/WIX 索引和图像解码；资源留在原目录 |
| 编码/解码 | [EnDecode.cpp](LegendOfMir3_Src/LegendOfMir3_Server/Def/EnDecode.cpp)、[EnDecode.h](LegendOfMir3_Src/LegendOfMir3_Server/Def/EnDecode.h) | 6-bit 编码与真正加密不同；不要替换成标准 Base64 |

### 已核对的消息边界

`Def/EnDecode.cpp` 的 fnEncode6BitBufW 将六位值加 0x3c 输出；fnDecode6BitBufW 做对应还原。它是文本传输编码，不能单凭文件名叫作加密算法。迁移时需按 ANSI/Unicode 分支、字节长度与截断行为分别核对。

`GameGate/ClientSockMsg.cpp` 的 ProcMakeSocketStr 从 _TMSGHEADER 取 wUserGateIndex，再在 g_UserInfoArray 查 CSessionInfo 并使用该会话 SendBuffer。网关会话索引不是角色 ID；重做网络接入时必须维持两者边界。

### 具体任务的读取顺序

背包动作：客户端 InventoryWnd/Item → GameProc 消息发送 → Protocol.h → ProcessUserMsg → ObjectPlayer/Handler_Item → 应答更新。NPC 动作：NPCWnd → 用户消息分发 → ObjectNPC/ObjectMerchant/ObjectScripter。地图切换：客户端 MapHandler 和服务端 Handler_Map 分开验证。以上是定向追踪入口，尚未证明每一个消息编号和字段都与宝藏 1.45 一致。

## 使用与验证边界

本轮为源码导航与代表性路径分析，没有构建或运行该参考工程。表中入口已核对存在；不能据此宣称整套逻辑与 1.45 一致。读取目标函数及直接调用者后，再迁移到 `client` 的对应功能模块。只验证本次影响的显示和行为，相关功能成组完成后统一截图与提交。
