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


## 使用与验证边界

本轮为源码导航与代表性路径分析，没有构建或运行该参考工程。表中入口已核对存在；不能据此宣称整套逻辑与 1.45 一致。读取目标函数及直接调用者后，再迁移到 `client` 的对应功能模块。只验证本次影响的显示和行为，相关功能成组完成后统一截图与提交。
