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


## 使用与验证边界

本轮为源码导航与代表性路径分析，没有构建或运行该参考工程。表中入口已核对存在；不能据此宣称整套逻辑与 1.45 一致。读取目标函数及直接调用者后，再迁移到 `client` 的对应功能模块。只验证本次影响的显示和行为，相关功能成组完成后统一截图与提交。
