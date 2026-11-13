# 移动、物品和 NPC 处理链分析

分析日期：2026-11-14。参考提交见各工程分析文档。以下区分已经阅读的处理函数、可复用的状态边界和仍需追踪的路径；不是对整个服务端的完成声明。

## 移动：请求与位置校正

### Zircon

[SConnection.cs:427](Zircon/ServerLibrary/Envir/SConnection.cs#L427) 检查 Game 阶段和方向范围，转交 Player.Move。

[PlayerObject.cs:14625](Zircon/ServerLibrary/Models/PlayerObject.cs#L14625) 检查 ActionTime/MoveTime、CanMove、距离 1–3；三格移动要求骑乘。逐格取 CurrentMap.GetCell 并检查 IsBlocking，不能只检查目的格。拒绝时发 S.UserLocation；动作时机未到时可入 DelayedAction，已有等待包则返回位置。

[CConnection.cs:815](Zircon/Client/Envir/CConnection.cs#L815) 和 [CConnection.cs:1002](Zircon/Client/Envir/CConnection.cs#L1002) 是本地校正与对象移动的不同接收入口。迁移时保持本地预测、服务端权威位置和远端动画分离，不把一个移动成功测试当作全部同步行为的证明。

### 传统 Mir3 C++

[Protocol.h:70](LegendOfMir3_Src/LegendOfMir3_Server/Def/Protocol.h#L70) 定义 CM_WALK=3011，CM_RUN=3013。[ObjectPlayer.cpp:2305](LegendOfMir3_Src/LegendOfMir3_Server/GameSvr/ObjectPlayer.cpp#L2305) 中可见死亡检查、WalkTo/RunTo、GOOD/FAIL 和时间值回复；被注释的 WalkXY/RunXY 不能当作当前执行路径。

[ObjectPlayer.cpp:2305](legendOfMir2_src/Mir3EIServerDev/Mir3server/GameSvr/ObjectPlayer.cpp#L2305) 在 EI 树中有对应分支。两树均有 UserInfo.cpp 的同名消息分支；尚未核实所有编译开关与调用者，因此不能仅靠两处 switch 相似就断言两条路径同时生效。

[Actor.cpp:1476](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/Actor.cpp#L1476) 是对象消息处理入口，和发给本地玩家的 GOOD/FAIL 不是同一层。后续实现联网移动需要分别确认确认包、场景广播及失败回滚。

### mir2x

[playernet.cpp:11](mir2x/server/src/playernet.cpp#L11) 校验请求 UID/mapUID 后按动作类型分发，移动转到 [player.cpp:904](mir2x/server/src/player.cpp#L904)。该函数按 estimateHop 处理当前起点或相邻起点，requestMove 成功后 dbUpdateMapGLoc，失败 reportStand。它不是服务端全路径寻路；不能把客户端整条路径直接交给此函数。

## 物品：暂存、失败与解锁

### Zircon

[SConnection.cs:541](Zircon/ServerLibrary/Envir/SConnection.cs#L541) → [PlayerObject.cs:7429](Zircon/ServerLibrary/Models/PlayerObject.cs#L7429)：先建立并排队 S.ItemMove 结果对象，再检查死亡、相同起终格、来源/目标容器、安全区和容量等条件。结果对象先入队不等于先判成功，应继续观察 Success 的赋值和连接发送时机。

[CConnection.cs:2111](Zircon/Client/Envir/CConnection.cs#L2111) 由 GridType 定位来源/目标 DXItemCell；先解除两端 Locked，再检查 p.Success。失败分支也必须解除锁定。成功后才继续容器变更和快捷栏关联维护。可借鉴这种请求中状态，不能照搬现代容器种类、货币和协议。

### 传统 Mir3 C++

[ObjectPlayer.cpp:2414](LegendOfMir3_Src/LegendOfMir3_Server/GameSvr/ObjectPlayer.cpp#L2414) 解码物品标识并按首字节 G 分流一般物品/其他装备处理。TAKEOFF 也解码标识后调用对应方法。包体不是直接传一个当前 UI 格子指针。

[GameProc.cpp:3151](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/GameProc.cpp#L3151) 更新 FEATURE/ChangeFeature，再 SetEquipFromTakeOn；失败分支把待穿戴物品放回背包并处理 common item。TAKEOFF 的成功/失败也有不同恢复路径。背包、装备和手持暂存物品是三份状态，不能点击后直接销毁来源对象。

EI 的对应服务端入口：[ObjectPlayer.cpp:1412](legendOfMir2_src/Mir3EIServerDev/Mir3server/GameSvr/ObjectPlayer.cpp#L1412)。这里只确认入口存在与装备职责；未断言两树所有穿戴条件完全一致。

### mir2x

[playernet.cpp:1075](mir2x/server/src/playernet.cpp#L1075) 按 itemID/seqID/count 取物品定义，分别尝试 consumeBook/consumePotion，只有 consumed 为真才 removeInventoryItem。其他类型分支尚未实现，不能将此参考函数当成完整物品使用方案。

客户端更新入口为 [processrunnet.cpp:687](mir2x/client/src/processrunnet.cpp#L687) 与 [processrunnet.cpp:710](mir2x/client/src/processrunnet.cpp#L710)。后续需要核对序号与数量更新，不能只刷新图标。

## NPC：当前页面与服务端权限

### Zircon

[SConnection.cs:608](Zircon/ServerLibrary/Envir/SConnection.cs#L608) 把对象 ID 交给 [PlayerObject.cs:10174](Zircon/ServerLibrary/Models/PlayerObject.cs#L10174)：死亡拒绝，清旧 NPC/页面状态，在 CurrentMap.NPCs 中匹配对象并检查距离，然后由 NPCObject 进入 EntryPage。

[PlayerObject.cs:10192](Zircon/ServerLibrary/Models/PlayerObject.cs#L10192) 要求存活、当前 NPC 和 NPCPage 存在，只接受当前页 Buttons 中的 buttonID 且 DestinationPage 非空。客户端不应自行发任意目标页面并认为跳转成功。

[CConnection.cs:2797](Zircon/Client/Envir/CConnection.cs#L2797) 将应答交 NPCBox.Response；页面显示与发送请求分离。此处没有把响应文法等同于 1.45 的文本菜单文法。

### 传统 Mir3 C++

[GameProc.cpp:3402](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/GameProc.cpp#L3402) 先 ResetDialog，记录本地玩家坐标，然后 6-bit 解码消息体并处理斜线分隔。可用于检查当前对话替换、移动关闭和菜单解析职责。服务端 NPC 请求分发到脚本执行的完整调用链本轮尚未证实；应从 ObjectNPC/ObjectMerchant/ObjectScripter 的具体调用者继续追踪，不能以目录导航替代完成证据。

### mir2x

[playernet.cpp:215](mir2x/server/src/playernet.cpp#L215) 把客户端事件转成 AM_NPCEVENT，并附上服务端玩家 X/Y/mapUID、事件路径和可选 value。这是 actor 间事件，NPC 脚本的权限校验需要继续查接收者，不能把转发视为已校验。

[processrunnet.cpp:625](mir2x/client/src/processrunnet.cpp#L625) 解序列化 npcUID/eventPath/xmlLayout，调用 loadXML，显示对话并隐藏购买窗口。该 XML 布局与原版文本菜单不同，只复用窗口互斥和上下文归属思路。

## 落到本工程时的验证项

| 一组改动 | 最小相关验证 |
| --- | --- |
| 移动/骑乘 | 行走与跑步、跨越阻挡格、失败位置校正、骑乘距离、远端动画结束 |
| 物品操作 | 成功迁移、服务端拒绝、两端解锁、手持物品恢复、外观随确认更新 |
| NPC 对话 | 当前对象和页面、非法按钮、切换 NPC、页面替换、关闭与回包 |

上述为后续实现检查清单，本轮没有运行这些游戏测试。已验证的是文档源码引用与明确阅读的分支；未闭合的调用链已在对应小节说明。客户端当前截图不能证明服务器或协议完成。
