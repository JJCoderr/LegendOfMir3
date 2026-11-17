# 服务器公告的原生编辑框隔离与居中

核对SM_SENDNOTICE=658入口发现它仅HideMessageBox，绕过了通用弹窗取消；邮件/寄售的SyncEditors和行会Show只检查普通MsgBox，下一帧会重新显示原生EDIT。原生EDIT由Windows合成，无法靠DirectDraw最后绘制公告来遮挡。

658入口改用PrepareModal：取消未提交输入、清理指针捕获、隐藏各编辑框、焦点移至主窗。邮件/寄售SyncEditors检查公告活动状态，行会Show也持续隐藏编辑框；公告关闭后按原页面布局恢复，草稿不清空。公告期间新到达的输入弹窗保留内容但暂缓显示，m_noticeInputDeferred仅在这一场景恢复编辑框和焦点；其他原因隐藏的输入不被每帧强行显示。共用EDIT在公告活动期间消费键盘消息，回车仅确认公告，不同时提交背后的金额请求。

截图复查发现另一实际问题：公告ShowMessageBox用窗口客户区尺寸居中，但最终按固定800×600游戏画布绘制。在较大窗口中公告会偏到右下角，按钮超出画面。新增CanvasWidth/CanvasHeight，按逻辑画布居中；150%窗口尺寸字段变化不影响公告逻辑Bounds。此项是坐标空间修复，不改变原版324px换行、字体、资源或拖动规则。尚未验证跨显示器DPI切换。

新增337 notice-mail-hidden、338 notice-guild-hidden、339 notice-input-deferred、340 notice-input-restored，下一341。23项检查通过真实658分发、实际RenderInterface/原生EDIT、回环UDP验证：旧交易数量取消、捕获/焦点释放、邮件/寄售/行会隐藏与草稿恢复、空公告取消旧输入、迟到输入即时与逐帧隐藏、隐藏EDIT键盘无请求、回车只发一次1018全零参数、延后输入恢复、其他隐藏输入不复显、公告画布居中与窗口缩放不偏移。

首次夹具调用GuildEditor.Close缺少参数，已修正为Close(false)。新增Interface字段后clean Client 90/90通过（notice-editor-clean-build.log）；最终七目标112/112通过（notice-editor-final-build.log）。最终重新运行14个预览：23新+公告11+隔离5+通用模态27+替换18+面板18+行会17+窗口移动缩放6+键盘34+音效15=174项通过。voiceCount统计不计入。

out/notice-editor共14张新鲜PNG；四新增状态均原尺寸查看，自启进程全部停止。review_gui.ps1 all新增四个入口，未来完整套件132；本批只跑上述14状态，没有把旧128图重新标成当前132图。

公告长文本越出资源内框的既有问题没有因此解决，仍见server-notice-evidence.md。真实服务器并发公告/输入响应、完整登录和听感仍待联调。
