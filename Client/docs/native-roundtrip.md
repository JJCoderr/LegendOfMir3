# 原生客户端联调回归

在 Windows 的仓库根目录运行：

```powershell
python Client/Tools/test_native_roundtrip.py
```

默认从 `out/debug` 读取 EIServer.exe、Mir2Ex.exe、EIServer.ini、Mir3.ini，以及 Data/Map/Sound 资源链接。需要可运行原生客户端的桌面环境。可用 `--bin-dir` 和 `--config-dir` 指定目录，`--port-offset` 调整七个测试端口（默认加 20000）。使用标准开发 INI 的节名，客户端窗口尺寸应保持默认 DPI 缩放。

测试通过 Windows 消息操作真实 Mir2Ex 窗口及其输入框；不截图、不用模拟协议客户端替代游戏客户端。验证六服务就绪、登录、选服、选角、移动、返回选角、再次进图并再次移动，以及退出客户端并重启重登后的坐标。通过可选 JSON 状态和 SQLite 实际记录交叉核对，检查窗口尺寸、目标端口、进程存活和快照时效。

每轮在 `out/native-roundtrip-*` 中新建 SQLite 与配置，Data/Map/Sound 使用目录链接。配置中的数据库路径与服务间连接明确指向隔离环境。EIServer 始终运行指定 bin-dir 中的同一个程序路径，避免随机复制服务器路径；Mir2Ex 启动时切换到自身目录，因此客户端仍复制到隔离目录。不会修改开发目录的配置或数据库。

终端输出每阶段状态，最后打印 PASS；服务日志保留在该轮目录的 server.log。失败返回非零并保留现场文件。清理只结束本轮创建的进程，服务端最后使用进程终止收尾，因此本测试不证明服务端协调停服、在线保存或长期稳定性。也不覆盖装备、背包、战斗和视觉样式完整验收。

退出保存检查使用确认退出前的新鲜客户端快照，等待 SQLite 坐标精确匹配后才重登。第二次进图会再次移动，确保本次保存位置与先前返回选角的记录不同，避免把上一次保存当作本次保存完成。

鼠标坐标换算另有独立检查，不需要游戏资源：

```powershell
cmake --build build-eiserver --config Release --target InputCoordinatesCheck
./build-eiserver/Release/InputCoordinatesCheck.exe
```

检查真实 Win32 客户区在 100%、150% 和横纵不同缩放下的坐标换算，以及窗口外负坐标。窗口消息与角色持续按键轮询共用该换算；该独立坐标检查不模拟物理按键持续按住的完整游戏行为。

长按输入回归：

```powershell
python Client/Tools/test_native_roundtrip.py --hold-mouse
```

此模式需要可交互桌面，并会短暂激活客户端、移动指针并注入左键按住事件约 1.8 秒，随后释放并恢复指针位置。检测到其他鼠标键已按下或无法取得前台焦点时失败退出，不将该情况视为通过。运行时请暂停鼠标操作。断言角色至少连续向东走两格且纵坐标不变，再执行返回选角、再次移动、保存和重登检查。
