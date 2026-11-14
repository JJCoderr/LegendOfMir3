# 服务端功能参考与实现选择

当前无自有服务端实现。按 AGENTS.md 记录客户端功能所依赖的服务端裁决路径，待迭代 3 对接。

## 掉落物与拾取（2026-11-14）

- LegendOfMir3_Src/LegendOfMir3_Server/GameSvr/ObjectPlayer.cpp：CM_PICKUP 分支先检查请求坐标等于人物当前坐标才 PickUp；RM_ITEMSHOW 将物品识别号、格子 x/y、looks 编成 SM_ITEMSHOW 610，并编码名称。客户端请求不能直接决定拾取成功。
- legendOfMir2_src/Mir3EIServerDev/Mir3server/GameSvr/ObjectPlayer.cpp：同样 CM_PICKUP 坐标校验再进入 PickUp，同源版本需在实服联调时核对字段排列。
- Zircon/ServerLibrary/Envir/SConnection.cs::Process(C.PickUp)：只在 Game 阶段交给 Player.PickUp(ObjectID)；Models/PlayerObject.cs::PickUp 检查 Dead，并按 PickUpRadius 搜索，属于不同玩法版本，不套用于 1.45。
- mir2x/server/src/playernet.cpp::net_CM_PICKUP：检查 mapUID、请求格等于当前格，否则 reportStand；有拾取锁与 PickUpError。可参考服务端裁决/失败通知分离，不移植包格式。
- 原版证据边界：本批未新增服务端 IDA 分析，沿用旧项目拾取输入路径记录。客户端只发 1001+坐标信号；背包更新、物品消失、重量/拥有权/金币/失败通知须由服务端真实回包驱动。

待办见 TODO.md；尚未启动测试服务端，不能把客户端测试注入物品当作端到端拾取通过。
