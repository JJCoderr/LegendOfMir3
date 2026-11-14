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
