# 原生连接失败与重试界面

2026-11-17。依据 Mir3 的 br_godot_test_261115 分支 client/src/godot/hud_view.cpp poll_connection、client_connection.hpp，以及 client/docs/evidence/login-flow.md 的 47C430/47C580 记录。

使用 CLIENT 原生 Win32 MessageBoxA，CmList 132 文案、Legend Of Mir 3 标题、MB_YESNO。系统对话框字体、按钮和边框由当前 Windows 主题决定；不声称与旧 Windows 同帧像素相同。该提示没有新增自定义音效，已有大厅音乐与按钮音频回归通过。

连接或异步读取错误进入提示。选是后冻结场景更新并禁用父窗口，等待 2000 ms 后重连同一地址，恢复输入焦点。选否向主窗口发送 WM_CLOSE。无错误 FD_CLOSE 保持参考版正常断开行为，不触发重试。主动断开取消等待，旧/无效 socket 通知被忽略。登录页不再拦截为旧中文游戏提示。

预览 262..264：connection-error、selection-connection-error、connection-checks。前两者只连接预览创建的 127.0.0.1 监听端点；第三者覆盖真实回环拒绝、重连、读错误与正常关闭。无真实网关登录。

验收：Client clean 88/88、全七目标构建通过，最终增量构建通过。连接20、账号27、分组33、音效15、退出流程30、返回选角7，共132项通过。报告位于 out/mud3-01/*-check.txt。执行程序由 build-ninja-sqlite/Mir2Ex.exe 复制到 out/mud3-01/Mir2Ex-connection-review.exe。

首轮回环等待1秒不足，改为最多5秒等待异步事件（产品重试仍2秒）。复验期间曾误复制旧运行目录 exe，旧报告不能作为最终结果；改从构建产物复制并删除旧报告后，全部检查通过。

真实弹窗截图 out/selection-connection-error-1.jpg，背景 out/selection-connection-error-0.jpg。Computer Use 可读取真实按钮及文字，但点击返回坐标超出窗口范围，未完成真实按钮点击验收；不以其冒充通过。取消和确认分支由上述回环检查验证。仅停止本批启动的进程。

下一批仍需审计登录530转场、全屏淡入淡出、wemade.dat与缺失影片行为；真实服务器流程待GUI完成后统一联调。
