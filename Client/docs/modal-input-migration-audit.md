# 模态提示优先级与原生编辑框遮挡

依据参考 hud_view.cpp 输入分派：group_dialog_input 先于 guild_editor_input 和其他面板输入。当前CLIENT此前在OnLButtonUp/OnMouseMove中有多个面板捕获分支早于模态判断，任务滚动、音量滑块、快捷栏/骑乘按钮会抢走提示点击；OnLButtonDown/OnKeyDown还把行会编辑器排在提示之前。

本批在鼠标按下/释放/移动入口优先处理提示，键盘也先处理提示。提示期间原有背景捕获保持但不更新，关闭后可继续完成释放，与参考的分派顺序一致。不是向背景面板注入取消或误提交操作。

实际截图额外发现原生行会EDIT窗口覆盖提示正文和按钮。ShowNotice立即隐藏行会EDIT并把焦点交回主窗；GuildEditor.Show按提示是否活动同步可见性，提示关闭后重新显示、恢复焦点，保留草稿文本。行会底板仍绘在提示下方。邮件/寄售已有单独编辑器遮挡逻辑，本批没有改写它们。

## 验证

七目标增量构建通过，out/modal-input-build.log、modal-input-final.log。产物out/mud3-01/Mir2Ex-modal-review.exe来自build-ninja-sqlite/Mir2Ex.exe。

新增14项检查：任务滚动/滚轮冻结、跨捕获点击确认、关闭提示后释放、音量不变、行会拖动冻结、原生编辑器隐藏和焦点、保留草稿恢复、键盘确认、快捷/骑乘捕获不抢事件。任务14、行会编辑17、选项10、音效15、键盘34、退出30，总134项全部通过。所有预览进程停止，无真实服务器操作。

预览301 modal-scroll、302 modal-audio、303 modal-guild-editor；下一304。三张实际截图out/modal-{scroll,audio,guild-editor}.png已查看。提示文本使用已有CmList4000作为离线叠放夹具，不代表任务页或选项页真实业务会发出行会权限错误。

整体GUI仍未终验。52页索引记录的是43d7c75二进制，本次修正截图单独记录，不覆盖该索引的版本证据。继续补其他边界状态与协议前置核查。
