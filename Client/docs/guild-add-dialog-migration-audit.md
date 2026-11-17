# 聊天命令添加行会成员弹窗

对照Mir3/br_godot_test_261115的chat_submission.hpp::submit及hud_view.cpp::chat_requested、open_group_dialog、group_dialog_result。消息1425为完整命令，1426为输入提示（首个%s替换当前行会名），1427为非会长提示；确认请求使用现有CM_GUILDADDMEMBER=1038。

输入命令在普通SendSay及调试命令处理之前被本地分派。会长打开原生Yes/No输入弹窗，普通成员显示原生OK提示。沿用通用弹窗的位置、字体、资源、90字节ANSI限制、Tab/Enter及Escape消费规则。确认发送一次1038、四个参数为零、正文为输入角色名；取消和打开均不发网络请求。没有另外请求成员列表，与参考一致。

新增Common/ChatCommandIds.h命名三个消息资源。没有添加Godot运行依赖或独立控件。鼠标音效沿用现有FinishNotice点击105.wav；键盘确认不新增声音。

预览325 guild-add-member、326 guild-add-denied、327 guild-add-member-filled；下一328。回环UDP读取真实SendPacket帧，验证权限/打开不发包、完整行会提示、焦点和字节限制、Escape、取消、确认单包及关闭后重复Enter无请求，共12项。

七目标构建out/guild-add-final-build.log成功。本批12+行会编辑17+模态27+聊天命令23+快捷21+键盘34+音效15=149项通过。最后只加强重复Enter的夹具检查，重建并重跑12项和三张新增图；相关回归对应相同生产实现。九图位于out/guild-add，三个新状态均查看；所有自启预览停止。真实服务器新增成员结果尚未联调。

另发现坐骑命令延迟缺口：参考map_input_delay.hpp的mount_command为300ms，HUD左键、地面拾取/丢弃、技能/动作均查询active；当前CLIENT的m_wInputDelayTime只有计时赋值与推进，未在输入路径查询。不能只赋300就宣称完成，需后续连同输入拦截及现有换图延迟一起核对。本批没有改动该路径。
