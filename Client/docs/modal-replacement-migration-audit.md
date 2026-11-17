# 通用弹窗替换与取消状态

参考Godot hud_view.cpp::open_group_dialog在打开新弹窗之前调用finish_group_dialog(false)。原生PrepareModal原先只清理鼠标捕获和隐藏编辑框，覆盖旧输入后可能残留交易pendingSlot、NPC prompt和寄售confirmationPending。

现在PrepareModal在旧MsgBox活动时先FinishNotice(CLICK_NO,false)，再清理指针和焦点。复用已有各类取消处理，清空旧输入和密码设置；false确保自动替换不播放确认点击声。已经关闭的弹窗不重复取消；已发送的交易金币、寄售请求继续等待服务器，邮件和寄售草稿、手持物品保留。

寄售Activate调整为先PrepareModal，再设置confirmation和新对话框编号。旧顺序提前写入新编号，若直接统一取消会误把旧窗口当成新寄售确认处理。没有新增类字段或新的控件。

新增335 modal-replacement-alert、336 modal-replacement-input，下一337。18项检查覆盖交易取消、提示编号、旧文本、焦点、NPC密码/命令、输入框掩码和字节限制、赠金确认、草稿、寄售取消后调用Confirm不会发送、寄售替换旧交易编号、已发送请求不取消、手持物品保留；完整替换序列回环UDP无请求。

测试夹具首次直接访问寄售私有字段导致编译失败，改为通过Confirm及requests/waiting观察取消后的行为，没有为测试暴露私有字段。七目标增量构建out/modal-replacement-build.log成功。

本批18+模态27+指针9+寄售22+邮件20+交易54+NPC密码10+金币18+堆叠丢弃28+音效15=221项全部通过。voiceCount=12仅为统计，不计断言。11张运行截图在out/modal-replacement，两个新增替换状态原尺寸查看。自启预览均停止。真实服务器并发提示/响应组合尚待联调，不以离线检查代替整体GUI或登录验收。
