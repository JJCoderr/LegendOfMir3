# 原生账号登录 GUI 迁移

## 范围与依据

参考 Mir3 仓库 `br_godot_test_261115` 的 `hud_view.cpp`（draw_login、login_input、draw_login_announcement）、`login_flow.hpp`、`login_announcement.hpp` 和 `docs/evidence/login-flow.md`。

本批完成 640×480 账号输入页、登录公告、普通错误框、登录后的服务器列表和计费提示。使用 CLIENT 的 CBMWnd、CChatEdit、CLoginNotice、DirectDraw 背景和 GDI 文字，不引入 Godot。

原始证据 404D80 及 Godot 实现均表明“创建账号”“修改密码”读取 Initial/Param2、Param3 并打开网址后退出，不是显示旧版客户端自带的注册表单。本批按此接线；离线验收只检查请求，不打开网址或退出进程。

800×600 LoginInterface/SelLogin.ini 服务器分组入口是另一个页面，仍待下一批迁移。真实登录连接、530 转场、后续选角/入场的服务器兼容性不在本批验收范围。

## 实现

- Interface1c 背景 0 位于 (0,60)，输入底图 2 位于 (100,436)。四个按钮使用 10/12/14/16 及其按下帧；登录悬停色块使用 ONE/INVSRCALPHA 对应的原生像素混合。
- 账号与密码输入控件为 (128,440,99,14)、(326,440,99,14)，24 字节上限。全局账号缓冲区扩为 25 字节，容纳终止符。密码只以星号绘制，验收报告不写账号密码。
- Tab/Enter 与鼠标提交遵循参考状态：Enter 在条件满足时提交，鼠标登录可提交空字段；普通错误提示关闭后清空密码、保留账号、聚焦密码。Esc 不关闭该提示框。
- Notice.ntc 使用既有加密文本解码；支持字号、粗体、前后景调色板、224 像素换行、每源行最多 10 行。行间距 14，累计严格超过 100 ms 移动 1 像素，过长单帧不计时。
- 服务器列表保留按下与释放同一行才选择，修正边框和悬停颜色，增加负载后缀、底部说明；529 更新负载和计费提示，528 显示普通提示。
- 账号页沿用 Opening.wav 延迟与循环播放；四个按钮接入 105.wav 点击音效。网址离开时停止大厅音频。
- 离线预览复用同一套控件和消息处理，固定 640×480；截图捕获按实际后台缓冲区尺寸合成原生编辑框。

## 验收

预览入口：`--preview=login`、`login-hover`、`login-pressed`、`login-error`、`login-servers`、`login-billing`。

最终 Client 清理构建 87/87、七目标构建 113/113 通过；27 项原生登录检查全部通过，覆盖实际输入控件的字节限制及 Enter 路由、错误恢复、网址配置、DirectSound 播放状态、负载更新及选择取消、公告样式和滚动边界。共用键盘路由 34 项、界面音效 15 项回归通过，共 76 项。最终运行程序为 out/mud3-01/Mir2Ex-login-review.exe；六张截图保存在 out/login*.png。

截图为原生 CLIENT 的外部资源实际渲染。未进行 MIR3.EXE 同帧逐像素对照；不能据此宣称完整客户端或真实登录流程已完成。
