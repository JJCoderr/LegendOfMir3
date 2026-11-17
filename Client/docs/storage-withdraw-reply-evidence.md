# 仓库取回结果同步

基线 bd2e386。StoreWnd::Receive 原先只使用 705 的 wParam 更新仓库列表，忽略 wTag，因此取回合并到已有背包堆叠时，背包和快捷栏仍显示旧数量。

依据 Godot br_godot_test_261115 的 client/src/godot/hud_view.cpp 中 SM_STORAGE_WITHDRAW_OK/FAIL/BAG_FULL 分支（5254 附近）：先消费当前 pending instance；成功且 wTag 非零时 update_bag_stack_count(instance,wTag)，随后用 wParam 更新或删除仓库项。wTag 为零不清空背包，关闭窗口/无待取回时忽略成功。失败仍显示原提示。

本批将此行为接入 CLIENT 既有 UpdateStackCount，保留其背包、快捷引用、鼠标持有及数量增加音效逻辑；705..707 拒绝带附加内容的异常消息。仓库列表删除后的页码改用已有 OffersPerPage 常量。

预览 storage-withdraw-reply（219）通过 GameProc 编码消息入口检查异常载荷、部分取回、重复回包、全部取回、零数量、失败、背包满、鼠标持有、关闭窗口后回包，共 9 项。回归 storage-list 23 项、service-deposit-reply 16 项、item-use-chain 26 项，合计 74 项通过；七目标增量构建通过。截图 out/storage-withdraw-reply.png 已查看，背包与快捷栏均显示 15，仓库列表仍保留剩余物品。

验证范围为原生客户端本地收发与状态更新；真实服务器仓库存储及数据库事务尚未验证。
