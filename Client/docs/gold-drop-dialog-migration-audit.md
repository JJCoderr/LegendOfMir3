# 原生金币拾取与丢弃金额框

依据现有 Godot hud_view.cpp 的 inventory_pointer、世界左键、held_gold 绘制与 group_dialog_result 102。使用当前 Inventory.wil 第124帧、CmList.dat 1415及原生通用输入控件；没有引入 Godot 运行依赖。

背包金币区域释放鼠标拿起金币，播放106.wav；持有金币时不能拿起背包物品。点击地面释放持有状态并打开金额框；输入延迟期间保留持有状态。沿用原生字体、皮肤、90字节输入限制和通用确认音效规则。行会编辑窗口优先于背包金币鼠标处理。

确认按参考 strtol 解析，金额必须大于零且小于当前余额；保留数字前缀行为，拒绝全部余额。发送1016，nRecog为金额，其余参数为零；取消不发包。余额等待653回复更新，不预扣。统一 HasInputDialog 分类，同时修复添加行会成员1038在旧输入提交入口的遗漏。

预览330 gold-held、331 gold-drop-dialog、332 gold-drop-filled；下一333。18项金币检查包括真实回环UDP单包、取消/非法值不发包、原生Enter与旧入口、余额回复、输入延迟、物品互斥及图标资源。行会13、通用模态27检查通过；另回归交易金额、邮件赠金、寄售价格、坐骑延迟、背包和音效，共11张截图位于out/gold-drop。背包静态报告moved与placement值不作为通过断言。

新增Interface成员后先clean Client（gold-drop-clean-build.log），七目标构建gold-drop-build.log成功。最终gold-drop-final-build.log增量构建成功。截图复核发现预览漏载Inventory.wil，补载并新增资源检查；最终重跑三个金币状态、行会输入及模态回归。三种金币图已查看，所有自启预览进程已停止。

这是原生GUI及离线协议验证，真实服务器金币落地/拾取仍待联调。堆叠物品丢弃数量框104尚未由本批实现；不将金币流程等同完整背包系统完成。
