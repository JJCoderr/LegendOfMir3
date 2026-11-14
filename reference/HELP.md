# 按模块记录实现选择

本文件是功能级选择记录，工程概览和既有分析索引见 [ANALYSIS_INDEX.md](ANALYSIS_INDEX.md)。优先以目标 1.45 的既有 IDA 结果确定精确行为，再使用开源实现核对架构和补足信息；不同版本字段或表现不能直接混用。此文件随当前工作区新增要求建立，历史模块的逐项汇总仍需补齐。

## 场景名称、发言与离场（2026-11-14）

### 1.45 既有 IDA 分析

旧项目 name_update.hpp / actor_name.hpp 记录 0x408450 的名称三行与尾段斜杠处理、0x401590 名称颜色表；chat_message.hpp 记录 0x401430 聊天调色板及头 +6/+8 低字节分别作为前景/背景，104 固定绿色/黑底；speech_bubble.hpp 记录 0x42F380 仅 40/1501 更新气泡，其他聊天仅入日志，0x40FAC1 的严格 >3000 ms 超时。本批复用这些已分析结论，没有新增 IDA 运行，不能将参考源码结论标作原版新实测。

### LegendOfMir3_Src

[GameProc.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/GameProc.cpp) 的 OnSvrMsgHear：六位解码后先 MsgAdd，仅 HEAR/MONSTERSAY 更新演员气泡；ProcessDefaultPacket：30/633 走独立默认消息队列，先清对象包队列，再删除对应对象/全对象。[Define.h](LegendOfMir3_Src/LegendOfMir3_Client/Define.h) 提供 30、42、633、656、1501 等号。[GFun.cpp](LegendOfMir3_Src/LegendOfMir3_Client/GFun.cpp) 聊天色表及 GameProc 的前背景参数顺序与目标旧分析不同，不直接采用。

### legendOfMir2_src（目录内容为 Mir3EI）

[GameProc.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/GameProc.cpp) 的 OnSvrMsgHear、ProcessDefaultPacket 保持日志/气泡双路径及默认队列删除；[copy_Actor.cpp](legendOfMir2_src/Mir3EIClientDev/Mir3EIClient/GameProcess/copy_Actor.cpp) 的 OnUserName 解码名字，OnChangeNameClr 单改颜色。该版本使用 m_pMyHero，气泡拷贝有 MAX_PATH-1 限制；不能据此覆盖 1.45 三行名称解析。

### Zircon

[CConnection.cs](Zircon/Client/Envir/CConnection.cs) 的 Process(S.Chat) 使用结构化包：OverheadOnly 控制日志，仅 Normal 且 ObjectID>0 发给对象 Chat；Process(S.ObjectNameColour) 直接更新颜色；Process(S.ObjectRemove) 清目标、鼠标对象、魔法对象、焦点与相关 NPC/怪物窗口，再删除对象。采用其“删除时关联状态一起清理”的校核思路，不移植消息号、OverheadOnly 和分类规则。

### mir2x

[processrunnet.cpp](mir2x/client/src/processrunnet.cpp) 的 on_SM_PLAYERSAY 查 Hero 添加气泡，非本人消息写日志；on_SM_OFFLINE 检查 mapUID 后从 m_coList 擦除。该仓库当前 C++/ImGui/OpenGL 的网络格式与目标不同，只用于核对 UID 生命周期和日志/气泡分流，不套用本地消息抑制政策。

### 实现选择与边界

采用 1.45 的名称、聊天颜色及气泡规则；30/633 按前两个 C++ 参考的默认消息队列思路延迟到场景更新清理，调用已有对象移除函数统一删除覆盖层、音频、目标和动作队列。保留本地玩家、聊天历史；人物缺席的合法聊天仍写日志但不创建对象。中文载荷先按当前 ANSI 资源编码转 UTF-8，不能直接当 UTF-8 读取。此入口处理已经分帧的消息，尚无 TCP/服务器联调；30/633 的具体原版阶段时序仍需后续原版实测校准。
