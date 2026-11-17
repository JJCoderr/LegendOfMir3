# 装备穿脱事务审查与迁移

审查基线 590a742。以下断点为修改前的情况；本批已迁移原生事务，验证记录见文末。

## 当前 CLIENT 断点

- StatusWnd::OnLButtonUp 通过 m_stTakeOnItemSet 和公共 bWaitResponse 发送穿上请求；背包独立发送脱下请求。没有统一的请求种类/目标槽/pending 状态，难以拒绝与当前请求不匹配的回包。
- GameProc 的 615/616/619/620 分支未先判断待确认请求，重复或无请求回包也可能改变持有物品和外观。
- 穿上/脱下成功使用 ZeroMemory(common,sizeof(ITEMSET))，清理长度与 COMMONITEMSET 不符，留下部分字段。失败路径 AddInvenItem 返回值被忽略，背包放不下时仍清除 incoming。
- 现有空位还原分支与已占用槽分支不同，装备来源拖回已占用槽可误走穿上请求；缺少完整的来源判定。
- 当前装备请求传旧 szName，未使用 CItem::Name() 的完整名称。
- 缺少原版 300ms 装备快速操作和 1124 请求；缺少原版 2413 拒绝提示。
- 装备回包未按 incoming/held 播放物品音效。当前已有页面和槽位规则测试不能证明完整事务安全。

## 已读取的参考

Godot br_godot_test_261115：client/src/status/equipment.hpp 完整 EquipmentState；client/src/godot/hud_view.cpp 的 status_pointer、emit_equipment_request、apply_equipment_reply、apply_game_message 中装备回包外观分支；client/src/protocol/message_ids.inc 中 CM_EQUIPMENT_QUICK_ACTION=1124。

参考原版注释定位：476570 拿起装备，476690 保存 incoming，476740 放回原槽，4766F0 穿上确认，476630 失败还原，476980 快速操作，476793/4767D5 更新释放时间。

## 完整迁移批次

1. 在 StatusWnd 保存明确的 pending 请求种类和目标槽；incoming 沿用已有原生 CItem，公共持有保留来源。装备操作和物品使用互斥，交易待确认/取回时禁止发起。
2. 拿起装备只移到公共持有；装备来源放回空槽时还原原槽，不发请求；指向已占用槽不发穿上。背包来源的替换先保存 incoming，把旧装备移到公共持有，再发送1003。背包接收装备来源时发送1004。
3. 请求使用完整名称。1003/1004：recognition=instance，wParam=slot；1124：recognition=slot，无名称，不建立普通穿脱 pending。
4. 释放间隔严格小于300ms时，模式5/6/10/11/15/53提示2413后继续普通释放；其他类型只允许装备来源且匹配目标槽的快速动作。时间按 DWORD 差值处理。
5. 回包匹配 pending 种类。穿上成功将 incoming 恢复到请求槽（冲突找空位），旧装备由独立200消息返回背包；失败恢复旧装备并自动放回 incoming，背包无空间时保留公共持有。脱下成功只清持有，等待独立200；失败放回装备槽。不能自行给背包多加一份。
6. 完成时清空全部 COMMONITEMSET 状态，重复/错类回包无副作用；只有接受的成功回包更新角色外观。按参考播放 incoming/held 音效。
7. 同步 InventoryWnd::UseBlocked 等现有互斥逻辑，不能仅靠 incoming.bSetted 判断脱下 pending。

## 集中验收范围

完整请求编码（含长名称）、空槽穿上、替换成功/失败、脱下成功/失败、错类/重复回包、背包满失败恢复、槽位冲突、非法装备位、物品使用/交易互斥、快速操作及300ms边界、独立200补回物品而不重复、声音。通过实际GameProc消息入口而非只直接调用新方法。页面截图至少展示穿戴完成、替换失败恢复和脱下后的原生状态。本批按上述整套事务集中实现、测试和提交。

## 实现和验证结果

StatusWnd 新增明确的 m_equipmentRequest/Slot/Release，ReleaseEquipment 统一普通与快速操作，RequestTakeOff 由背包原生释放入口调用；ApplyEquipmentReply 匹配请求后恢复物品、完整清理状态并播放音效。GameProc 只有接受成功回包才改变外观。InventoryWnd 将装备来源排除出消耗物品双击路径，使用互斥也检查脱下 pending。修正 StatusWnd 初始化 incoming 的清理长度。

槽位全部占用时保留 pending 和物品；有空位后可继续处理。穿上失败且背包无空间时保留普通鼠标持有，不丢 incoming。沿用当前 CItem/ITEMSET，无 Godot 运行时。

预览 220/221/222：equipment-transaction、equipment-rollback、equipment-takeoff。实际 ClientSocket UDP 回环验证 1003/1004/1124 的名称、instance 和 slot 字段，通过 GameProc 编码入口投递成功/失败/错类/重复回包及独立200物品增加。共25项检查，包括满背包、占满装备槽、冲突恢复、非法槽、快速边界、2413提示、两类事务互斥和实际DirectSound播放。三预览均通过final_state检查。截图out/equipment-transaction.png、equipment-rollback.png、equipment-takeoff.png用于显示测试物品状态，不作为真实装备美术或在线服务器验收。

回归 item-use-chain 26项、service-repair-reply 16项、status-equipment 5项，合计72项检查通过。清理构建87/87、七目标111/111通过，最终截图状态断言增量构建通过。还修正了新装备与既有商人长名称测试夹具：原文字实际31字符，现使用30字符以符合m_serverName容量；商人回归重新通过。

仍未验证真实服务器穿脱/属性重算和外观同步；仅保留既有成功外观更新接口，不据此宣称完整在线角色显示一致。
