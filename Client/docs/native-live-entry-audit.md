# 原生客户端真实登录入场联调（2026-11-17）

## 范围及结果

使用 e5e8de2 原生客户端、本机六服务、既有测试账号及 MirProbe01 角色，实际操作服务器组、账号登录、计费提示、选服、选角、入场和地图点击移动。没有使用 Godot 运行时，也没有以离线预览代替服务器验证。

最终一轮成功显示地图 01、人物和原生 HUD，坐标从 459,261 移动至 462,266；技能页和背包面板可打开。客户端退出后 GameSvr 仍运行。背包为空、属性显示仍不正确，不能据此认定物品及属性协议已对接完成。

## 本阶段修复

1. GameGate 接受连接后立即 Recv，可能在 GM_SERVERUSERINDEX 返回前转发登录数据。旧游戏服崩溃转储 `GameSvr.exe.2652.dmp` 的故障 RVA 为 `0x19c9c`，指令访问用户数组，索引寄存器值 `0xaa88` 超过数组容量 10000。改为收到匹配 socket 的索引回复后才开始接收，客户端提前发送的数据由 TCP 缓冲保留。
2. GameSvr 的 GM_DATA、GM_CLOSE 在访问用户数组前检查索引范围。独立连接发送两种越界包后再发送检查包，服务仍在同一连接上正确回复。
3. 第一轮修复后的入场又暴露运行库缺失初始物品定义：新转储 `GameSvr.exe.27260.dmp` RVA `0x14f2e` 对应背包序列化的标准物品虚调用；运行库两张标准物品表均为零条。先用 SQLite backup 保存运行库，再应用仓库既有 `sqlite_minimal.sql`，补齐 7、33、34 和普通物品 0。未修改这些测试物品的定义，也未宣称其为原版数值。

IDA 技能启动检查可复用服务，但开库脚本找不到 IDA 安装目录；本次定位实际使用本机 pefile/capstone 和 minidump 异常上下文解析，未声称获得 IDA 反编译结果。

## 环境隔离和部署

- 测试客户端目录 `out/native-live-01`，独立 INI 指向 127.0.0.1；Data/Map 仅链接不可变资源，Sound 只读复用。没有改写原始素材或原运行目录的客户端服务器列表。
- 更新并重启了 GameGate、GameSvr；其余 DBSvr、LoginSvr、LoginGate、SelGate 保持运行。
- 原程序及数据库备份：`out/native-live-01/server-backup`。恢复数据库不能在服务运行时直接覆盖，应先停服务并确认是否需要保留后续数据。
- 最终 GameSvr PID 28188、GameGate PID 31804；测试客户端 PID 32304 已通过窗口关闭退出。PID 只记录本次会话，不作为后续操作依据。

## 验证及截图

- 七目标构建成功：`out/entry-handshake-build.log`，实际重编译/链接本批 GameGate、GameSvr。
- `python Server/Tools/test_gate_user_index.py`：越界 GM_DATA、GM_CLOSE 后检查响应通过。
- `python Server/Tools/test_sqlite_schema.py`：模式重复应用及自定义数据保留测试通过。
- SQLite `integrity_check` 为 `ok`；最终六服务运行、游戏服到数据库及游戏网关到游戏服连接正常。
- `git -c core.whitespace=cr-at-eol diff --check` 通过。

实际窗口截图（保留在本机 out，不纳入代码库）：

| 截图 | 证据 |
| --- | --- |
| `out/native-live-01/live-selection.png` | 真实服务器返回角色 |
| `out/native-live-01/live-entry-seeded.png` | 补齐测试定义后的实际地图/HUD |
| `out/native-live-01/live-skills.png` | 真实入场后的技能页 |
| `out/native-live-01/live-bag-empty.png` | 背包面板可打开，物品尚未显示 |
| `out/native-live-01/live-map-moving.png` | 实际地图移动后的截图 |

`live-entry-fixed.png` 是补齐物品定义前的短暂地图首帧，不能用作稳定入场证据；`live-entry-result.png`、`live-entry-timeout.png` 是修复前失败记录。

## 尚未完成

- 服务器旧物品记录与客户端 MIR3 记录格式对接；属性、背包负重/物品显示。
- GameGate 断线自动重连：旧 FD_CLOSE 分支被注释，游戏服崩溃后本次通过重启网关恢复。
- 多账号并发、重复登录、创建/删除角色及持久化退出的完整回归；ProcessLogin 的 ready-list 游标赋值问题需独立修复验证。
- 原版缺失电影仍以现有资源为准；长公告溢出、全部 GUI 原版逐像素一致性和全部音效实听未获本次验证。
