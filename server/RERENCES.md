# 服务端功能参考与实现选择

当前无自有服务端实现。按 AGENTS.md 记录客户端功能所依赖的服务端裁决路径，待迭代 3 对接。

## 掉落物与拾取（2026-11-14）

- LegendOfMir3_Src/LegendOfMir3_Server/GameSvr/ObjectPlayer.cpp：CM_PICKUP 分支先检查请求坐标等于人物当前坐标才 PickUp；RM_ITEMSHOW 将物品识别号、格子 x/y、looks 编成 SM_ITEMSHOW 610，并编码名称。客户端请求不能直接决定拾取成功。
- legendOfMir2_src/Mir3EIServerDev/Mir3server/GameSvr/ObjectPlayer.cpp：同样 CM_PICKUP 坐标校验再进入 PickUp，同源版本需在实服联调时核对字段排列。
- Zircon/ServerLibrary/Envir/SConnection.cs::Process(C.PickUp)：只在 Game 阶段交给 Player.PickUp(ObjectID)；Models/PlayerObject.cs::PickUp 检查 Dead，并按 PickUpRadius 搜索，属于不同玩法版本，不套用于 1.45。
- mir2x/server/src/playernet.cpp::net_CM_PICKUP：检查 mapUID、请求格等于当前格，否则 reportStand；有拾取锁与 PickUpError。可参考服务端裁决/失败通知分离，不移植包格式。
- 原版证据边界：本批未新增服务端 IDA 分析，沿用旧项目拾取输入路径记录。客户端只发 1001+坐标信号；背包更新、物品消失、重量/拥有权/金币/失败通知须由服务端真实回包驱动。

待办见 TODO.md；尚未启动测试服务端，不能把客户端测试注入物品当作端到端拾取通过。

## 地图切换（实现前记录，2026-11-14）

- 两套早期 C++ ObjectPlayer.cpp::RM_CHANGEMAP 都发送当前人物识别号、x/y、DayBright 与地图 series，六位编码地图文件名，再 SendMapName；文件名和显示名称不是一个字段。
- Zircon ServerLibrary/Models/PlayerObject.cs 地图变化发送 MapChanged(MapIndex, InstanceIndex)，清异图法术、按地图限制下马；位置消息独立。
- mir2x server/src/playerop.cpp::on_AM_MAPSWITCHTRIGGER 区分同图 requestSpaceMove 与异图 requestMapSwitch，由服务端请求流程裁决。
- 旧 mir3_1.45/gameplay.cpp::cross_exit 使用 MockServer::enter_region，不能据此实现生产服务端换图包。

当前只实现客户端已解码切换入口和资源/场景生命周期；真实角色识别号、网关帧、地图文件名/显示名分包、视野重建顺序、换图失败与实例生命周期需要实服联调。客户端不自行推断传送门或服务器落点。

## 环境亮度（实现前记录，2026-11-14）

- 两套早期 C++ ObjectPlayer.cpp::RM_DAYCHANGING 发送 SM_DAYCHANGING(0, m_btBright, DayBright(), 0)，与旧 1.45 客户端分析的第三参数读法不同；真实服务端版本必须验证，不能直接套字段。
- Zircon ServerLibrary/Models/PlayerObject.cs 入场数据含 SEnvir.DayTime，地图 LightSetting 另行控制照明；不是目标协议。
- mir2x 未找到可替代目标 46 昼夜包的证据。旧 mir3_1.45 MockServer 的 120 秒循环只是离线演示，不移植自动时间循环。

客户端按旧目标分析消费 46 第三参数；入场亮度由已解码 change_map 参数传入。真实服务端字段、地图初始设置、人物装备灯属性和特效光源仍待联调。

## 人物灯属性（实现前记录，2026-11-14）

- 两套早期 C++ ObjectPlayer.cpp 的 RM_CHANGELIGHT 发 654、wParam=m_btLight；与目标旧分析 807、第三参数高字节不同，联调前须核对。
- Zircon PlayerObject 对象数据以 Stats[Stat.Light] 提供 Light，客户端不自行裁定装备照明。
- mir2x 未找到可替代 807 的证据。旧项目的项链属性 2 是离线 fixture，不能作为物品数据库规则。

客户端接收 807，并新增 set_local_actor_identity 登记本地网络 ID；网络识别号与渲染 actor=0 区分。真实登录角色 ID、穿脱装备结果和灯属性包顺序仍待服务端联调。

## 地图消息联调接口（2026-11-14）

两套早期 C++ ObjectPlayer::SendNewMap 发送 51(角色 ID,x,y,MAKEWORD(DayBright,series)) 加六位编码文件编号；RM_CHANGEMAP 对应 634，SendMapName 另发 54 显示名称。Zircon 使用 MapIndex/InstanceIndex，mir2x 使用行动消息 mapUID，不直接兼容。目标客户端本批 IDA 确认 51/634 共用 0x42EE00、低字节亮度、高字节地图状态、54 独立名称字段。

客户端原包接口现可排队处理地图消息，并通过 map_change_finished 报告加载结果；真实网关连接、初次登录人物数据、地图状态效果和跨消息批次的视野重建顺序继续待联调。

## 换图左键延迟（2026-11-14）

目标客户端成功处理换图后设置 1500ms 普通左键按下延迟；早期 C++ 参考为 2300ms，采用目标 IDA 证据。该延迟不等于服务端传送保护或动作冷却，不向服务器虚构额外确认消息；Zircon 动作 NextActionTime 与 mir2x 事件状态门禁不能套用于此字段。服务端真实入场、消息顺序和角色保护逻辑继续待联调。

## 选项命令接口（2026-11-14）

目标原版 0x463500 的选项项 11/12/13/14/16/17/18 调用 0x47CA80 发送 CmList2123..2129，攻击模式项19发送 CmList1422。当前资源对应回生术、天地合一、交易、私聊、行会聊天、加入行会、全体聊天及攻击方式命令。客户端按已确认字段预测显示，不自行确认开关值或循环攻击模式。

本批复用 chat_requested 命令信号，新增已解码选项状态输入；原始确认消息和服务端状态映射未完成，不把 UI 点击当成服务器批准。

## 选项状态原包 1401（实现前记录，2026-11-14）

实时 IDA：0x4282C0 的 case1401 / 0x42D1DA..0x42D1E6 将 recognition 与第一 WORD 参数交给 0x464A00。掩码按窗口顺序为 40000000、80000000、08000000、04000000、02000000、01000000、00800000；最后一项是立即数（IDA 符号 dword_800000），不是读内存。模式直接保存 unsigned short。

原版只在某一确认位变化时修改该项显示，因此同值重复包不清掉预测。私聊/行会变化各翻转其拒绝变量并更新底栏图标；所有人聊天变化依旧值选择图标，再翻转 7678F4，并写入 7678F0（行会拒绝变量）：旧 all=0 时 guild=1，旧 all!=0 时 guild=0。反汇编 0x464CDF..0x464D5F 验证该交叉写入，不能把它当反编译错误擅自修正。依序先行会、后所有人聊天。

实现选择：原始 1401 接入已有分帧头解析，允许 recognition 最高位；只有变化项覆盖预测，严格保持聊天侧效应和执行顺序。已解码 apply_options_state 仍作为显式整体状态注入接口，不代替原版差量更新路径。完整网络会话与服务端联调仍未完成。

## 选项按钮状态（实现前记录，2026-11-14）

IDA 实时核验 0x41FAF0 绘制、0x41FC60 悬停、0x41FCA0 按下、0x41FCD0 释放、0x41FD70 换帧。ON 的普通/悬停底图/按下为1292/1293/1295，OFF 为1295/1294/1292；悬停先画底图再画普通图。旧客户端一直画1293/1294，实际是一直显示悬停。关闭按下1222，攻击方式按下1297。0x464010 按下遍历按钮，0x4640C0 移动调用全部按钮，state2不会被移动覆盖；0x463500 释放只测试落点，不要求此前按下。0x4641D0 清全部状态及滑块拖动，主界面无命中窗口的移动路径0x43CF40调用它。

本批补齐普通/悬停/按下帧、移动中按下态保留、窗口外清理与关闭/失焦清理，保留释放落点触发。原版窗口重叠分发和所有音量拖动边界仍需进一步同状态核验，不声称整个输入系统完成。
