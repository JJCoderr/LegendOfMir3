# 原生客户端联调回归

在 Windows 的仓库根目录运行：

```powershell
python Client/Tools/test_native_roundtrip.py
```

默认从 `out/debug` 读取 EIServer.exe、Mir2Ex.exe、EIServer.ini、Mir3.ini，以及 Data/Map/Sound 资源链接。需要可运行原生客户端的桌面环境。可用 `--bin-dir` 和 `--config-dir` 指定目录，`--port-offset` 调整七个测试端口（默认加 20000）。使用标准开发 INI 的节名，客户端窗口尺寸应保持默认 DPI 缩放。

测试通过 Windows 消息操作真实 Mir2Ex 窗口及其输入框；不截图、不用模拟协议客户端替代游戏客户端。验证六服务就绪、登录、选服、选角、移动、返回选角、再次进图，以及退出客户端并重启重登后的坐标。通过可选 JSON 状态和 SQLite 实际记录交叉核对，检查窗口尺寸、目标端口、进程存活和快照时效。

每轮在 `out/native-roundtrip-*` 中新建 SQLite 与配置，Data/Map/Sound 使用目录链接。配置中的数据库路径与服务间连接明确指向隔离环境。EIServer 始终运行指定 bin-dir 中的同一个程序路径，避免随机复制服务器路径；Mir2Ex 启动时切换到自身目录，因此客户端仍复制到隔离目录。不会修改开发目录的配置或数据库。

终端输出每阶段状态，最后打印 PASS；服务日志保留在该轮目录的 server.log。失败返回非零并保留现场文件。清理只结束本轮创建的进程，服务端最后使用进程终止收尾，因此本测试不证明服务端协调停服、在线保存或长期稳定性。也不覆盖装备、背包、战斗和视觉样式完整验收。
