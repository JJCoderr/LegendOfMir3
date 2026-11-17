# 商人服务结果回包迁移

基线 cb75497 的出售、修理和仓库存入已有页面、报价及提交，但 GameProc 未分派结果消息，服务会停留在 Paying。本批统一迁移到 InventoryWnd::ReceiveService。

## 依据

Godot br_godot_test_261115：client/src/protocol/message_ids.inc、client/src/godot/hud_view.cpp 中 apply_game_message 的服务分支，以及 update_bag_stack_count。参考注释对应原版 42AA6F（普通出售只清背包）、42AB37（修理先更新余额）、42AB9A（修理失败提示）。消息/提示/耐久偏移集中 MerchantServiceIds.h。

- 648/1314 出售成功，先更新余额；有对应 Paying 服务时才更新物品和提示。1314 采用 wParam 剩余量、wTag 出售量；0 剩余删除实体与同名快捷引用。648 保留原版只清背包槽的行为，不擅自清快捷引用。
- 649 总是显示 1455；1315 不增加通用失败弹框。对应出售状态恢复 Choosing、报价清零。
- 669 修理先更新余额，有对应服务时更新匹配 instance+完整名称的物品耐久和服务器记录 66..69；670 显示 1457。成功提示 1456，完成后恢复可选择状态。
- 701 存入按 wParam 更新剩余量，wTag 选择 1447/1448 提示；702/703 分别提示 1449/1450，保留物品。存入结果不会误改金币余额。
- 所有新消息要求仅含头部；附加内容不改变状态。迟到成功可更新余额，但不再重复删除或更新服务物品。失败提示按原版可在窗口关闭后出现。
- UpdateStackCount 增加明确的 removeWhenEmpty 参数；既有 1313 数量为零仍保留物品，只有服务路径请求清零删除。
- 服务选择、数量输入和提交匹配统一使用 CItem::Name()，支持 30 字节名称，不再混用旧短名称字段。

## 验证

清理客户端构建 87/87 通过，七目标构建通过。预览 215..218（service-repair-reply、service-sell-reply、service-deposit-reply、service-failure-reply）通过 GameProc 编码消息入口验证 16 项：异常载荷、部分/全部/普通出售、重复回包、两类出售失败、耐久更新、修理失败、部分/全部存入、仓库满、存入失败、跨服务回包、迟到成功/失败。夹具使用 30 字节完整物品名。

回归 merchant-repair 17 项、merchant-sell-quantity 14 项、item-stack-reply 5 项、item-use-chain 26 项，共 78 个布尔检查全通过。使用链路回归仍覆盖实际 DirectSound 播放。截图 out/service-{repair,sell,deposit,failure}-reply.png 均已查看。

此批验证客户端状态和界面；没有证明在线服务器的买卖交易、仓库存储或数据库持久化完成。后续仍需真实服务器接入。
