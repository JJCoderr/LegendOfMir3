# 登录到选角的原生淡出

2026-11-17，基线703b9a7。参考 Mir3 br_godot_test_261115 的 client/src/hud/login_flow.hpp brightness/advance 和 hud_view.cpp draw_login/draw_exit_transition/apply_login_message。

收到 SM_SELECTSERVER_OK 后，隐藏输入框、断开旧连接，保留服务器列表画面；全画布按参考公式在2000毫秒内乘色淡出。原生RGB565后缓冲绘制，包含列表、帮助文字及提示框。按键和鼠标按钮在转场期间不改变页面。大厅Opening.wav在服务器列表及淡出期间延续，到选角由既有音频层切换。

移除RenderScroll的静态累积滚动计数，复用每次Load/成功响应重置的成员计时器。1999ms保持登录，2000ms交接一次，超大帧时间饱和；完成交接后不再Present登录背景，避免明亮闪帧。SetNextProc沿用既有选角加载，offline预览不发起网关连接。原来的旧DBSvr单IP响应兼容分支保留，真实协议尚待联调。

预览265..267：login-transition-start、login-transition-half、login-transition-end；固定0/1000/1999毫秒，实际后缓冲截图out/login-transition-{start,half,end}.png。下一编号268。

构建全七目标通过，日志out/login-transition-build.log、out/login-transition-final.log。没有新增类字段，不需布局迁移；头文件依赖涉及的Mir2Ex/LoginProcess均已重新编译。运行产物out/mud3-01/Mir2Ex-transition-review.exe来自build-ninja-sqlite/Mir2Ex.exe。

验收：转场12、账号27、分组33、音效15、连接20，共107项通过；最终防闪帧修改后再次执行转场12项。全帧像素检查半暗图每通道不大于起始，结束图全黑；三张截图均已目视检查。仅使用离线预览及回环连接，不进行真实服务器登录。测试进程均已停止。

未完成：wemade.dat开场、CreateChr.dat正反向与StartGame.dat影片及内嵌音频需独立迁移核对。现有LoginAvi仍使用旧ei_login.avi/ei_logo.avi并在Load指定静态背景，不能视作完整影片实现。未声称MIR3.EXE同帧逐像素一致，真实网关及选角流程留待GUI完成。
