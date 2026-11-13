# Zircon 功能分析

本地分析日期：2026-11-14；基准提交：`04d8cd34445b329ba69f5e027f5d4c45e35eac7d`。

C# 传奇3实现，优先参考功能状态、对话框职责和客户端/服务器边界。现有 docs 已有详细导航，本文件只记录本次复刻所需的入口和差异。

## 按功能定位

| 功能 | 首先读取 | 需要确认的内容 |
| --- | --- | --- |
| 场景与控件 | [GameScene.cs](Zircon/Client/Scenes/GameScene.cs)、[DXControl.cs](Zircon/Client/Controls/DXControl.cs) | 场景拥有窗口；父子关系、可见性、事件及缓存归控件 |
| 背包 | [InventoryDialog.cs](Zircon/Client/Scenes/Views/InventoryDialog.cs)、[DXItemGrid.cs](Zircon/Client/Controls/DXItemGrid.cs) | Grid、货币、选中条目、整理/出售请求 |
| 人物与魔法 | [CharacterDialog.cs](Zircon/Client/Scenes/Views/CharacterDialog.cs)、[MagicDialog.cs](Zircon/Client/Scenes/Views/MagicDialog.cs)、[MagicBarDialog.cs](Zircon/Client/Scenes/Views/MagicBarDialog.cs) | 属性和技能窗口分别定位 |
| 聊天与组队 | [ChatTab.cs](Zircon/Client/Scenes/Views/ChatTab.cs)、[ChatTextBox.cs](Zircon/Client/Scenes/Views/ChatTextBox.cs)、[GroupDialog.cs](Zircon/Client/Scenes/Views/GroupDialog.cs) | 输入与历史、组队状态分开 |
| 行会与任务 | [GuildDialog.cs](Zircon/Client/Scenes/Views/GuildDialog.cs)、[QuestDialog.cs](Zircon/Client/Scenes/Views/QuestDialog.cs) | 分页、权限、任务选择 |
| NPC | [NPCDialog.cs](Zircon/Client/Scenes/Views/NPCDialog.cs)、[NPCObject.cs](Zircon/Client/Models/NPCObject.cs) | 场景对象与对话窗口分开 |
| 地图与小地图 | [MapControl.cs](Zircon/Client/Scenes/Views/MapControl.cs)、[MiniMapDialog.cs](Zircon/Client/Scenes/Views/MiniMapDialog.cs) | 地图绘制输入与小地图窗口 |
| 人物与骑乘 | [PlayerObject.cs](Zircon/Client/Models/PlayerObject.cs)、[UserObject.cs](Zircon/Client/Models/UserObject.cs)、[HorseTameDialog.cs](Zircon/Client/Scenes/Views/HorseTameDialog.cs) | 远端人物、本地玩家、驯马窗口不是同一职责 |
| 网络与服务器 | [ClientPackets.cs](Zircon/LibraryCore/Network/ClientPackets.cs)、[ServerPackets.cs](Zircon/LibraryCore/Network/ServerPackets.cs)、[PlayerObject.cs](Zircon/ServerLibrary/Models/PlayerObject.cs) | 请求和权威应答成对追踪 |

## 已核对的规则与差异

`InventoryDialog` 继承 DXImageControl，拥有 DXItemGrid、多个货币标签、关闭/整理/丢弃/出售按钮；操作通过 CEnvir.Enqueue 发出。可参考条目选择与请求组织，不能直接照搬当前窗口坐标和新增货币布局到 1.45。

`PlayerObject.HorseFrame` 使用 `DrawFrame + ((int)Horse - 1) * 5000`。这与 mir3_1.45 的 ActorAppearance::horse_frame 使用 400 帧马匹分组明显不同。因此骑乘的状态流程可以交叉参考，帧号必须使用目标资源与 1.45 规则。

## 后续查询路线

先读 [CLIENT_UI](Zircon/docs/CLIENT_UI.md)，只打开目标 Views 文件和所用控件。地图与人物转到 [CLIENT_RUNTIME](Zircon/docs/CLIENT_RUNTIME.md)，请求/应答转到 [NETWORKING](Zircon/docs/NETWORKING.md)。客户端与服务器都有 PlayerObject，路径必须写全。不要照搬现代协议的包编号、序列化字段顺序或资源库索引。


## 使用与验证边界

本轮为源码导航与代表性路径分析，没有构建或运行该参考工程。表中入口已核对存在；不能据此宣称整套逻辑与 1.45 一致。读取目标函数及直接调用者后，再迁移到 `client` 的对应功能模块。只验证本次影响的显示和行为，相关功能成组完成后统一截图与提交。
