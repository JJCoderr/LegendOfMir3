
## 界面现代外观（Manifest）

清单文件放在仓库根目录：[app.manifest](app.manifest)，内容为 Common Controls 6.0 主题外观、
DPI 感知、Windows 7~11 兼容性、longPathAware。

各程序直接在自己的资源脚本里嵌入它（没有额外的公共 .rc）：

```rc
// Server\DBSvr\DBSvr.rc
#ifndef RT_MANIFEST
#define RT_MANIFEST 24
#endif
1 RT_MANIFEST "..\\..\\app.manifest"     // Client\Resource.rc 用 "..\\app.manifest"
```

CMake 里只需要关掉链接器自动生成的清单：

```cmake
target_link_options(GameSvr PRIVATE "/MANIFEST:NO")
```

`/MANIFEST:NO` 是为了避免同一个 exe 里出现两份 `RT_MANIFEST`（资源 ID 都是 1）。
`Server/Tools` 下的两个控制台工具没有 .rc，保持链接器默认清单。

清单内容生效后：

* 按钮、列表框、状态栏、工具栏、滚动条等控件改成当前 Windows 主题（不再是 Win95 灰色凸起）；
* 高 DPI 下不再被系统位图拉伸，界面清晰；
* 对话框里的 `FONT 9, "Arial"` 已统一改成 `FONT 9, "MS Shell Dlg"`（按系统语言映射到雅黑/Segoe UI），
  对话框加了 `DS_CENTER`，打开时居中显示。

验证清单是否嵌入：

```bat
mt.exe -inputresource:GameSvr.exe;#1 -out:manifest.xml


## 窗口尺寸

原来各程序用 `CW_USEDEFAULT` 让系统决定宽高，在 2K/4K 屏上会开到 1792×1008 这种超大面积。
现在统一走 `Server/Def/ModernUI.h` 的 `Mir3SizeMainWindow()`：

* 按 96 DPI 逻辑尺寸 → 再按显示器 DPI 放大；
* 限制在当前显示器工作区内（留边距）并居中。

各程序的实际尺寸就是自己 `StdAfx.h` 里的 `_MAINWND_WIDTH` / `_MAINWND_HEIGHT`
（96 DPI 逻辑像素），`InitInstance()` 中传给 `Mir3SizeMainWindow()`；要改哪个程序就改哪个程序的常量：

| 程序 | 窗口 |
| --- | --- |
| GameSvr | 1040 × 640 |
| DBSvr / LoginSvr | 940 × 580 / 940 × 600 |
| GameGate / LoginGate / SelGate | 900 × 560 |
| VirtualClient | 760 × 480 |
| ManConsole | 对话框模板 320 × 200（DLU），居中 |
| Client(Mir2Ex) | 游戏窗口 800 × 600，由 `WindHorn` 决定，未改动 |

## 2026-11-16 DBSvr 关闭确认框：点「取消」后卡一下然后程序退出

### 症状
`DBSvr.exe` 点标题栏 X → 弹确认框 → 点「取消」→ 界面卡一下 → 进程自己退出。

### 根因（三条叠在一起）
1. `Server/DBSvr/MainWndProc.cpp` 的 `WM_CLOSE` 里 `MessageBox` 是**同步模态**的，
   没有重入保护：连点 X / 关闭请求排队时会在前一个框关掉之后再弹一个框，
   表现为"点了取消又来一个框、界面卡住"。
2. 各工作线程（`ProcessDBMsg` / `ProcessGateMsg` / `AcceptThread` / IOCP worker）
   **直接跨线程操作界面**：`InsertLogMsg*` 写 ListView，`UpdateStatusBarSession`
   发 `SB_SETTEXT`。都是同步 `SendMessage`，主线程一停在模态框里，工作线程跟着卡。
3. 确认退出时**没有先停线程**就去 `ClearSocket` / `WSACleanup` / `PostQuitMessage`，
   进程退出时 CRT 还要析构 `static CDBManager`（析构里跑 ODBC 的
   `SQLFreeHandle`），而 DB 线程可能仍在那条连接上干活 → 长时间卡住/被杀掉。
   另外 `ProcessGateMsg` 是死循环，`g_fTerminated` 时也不退出，等线程永远等不到。

顺带修掉的崩溃（`cdb` 实测栈：`DBSvr!CConnection::Uninit+0x11`，`eax=0`）：
`DBSvr.cpp` 里 `GetDBManager()->m_dbMain.CreateConnection()` 失败返回 NULL 后
直接 `DestroyConnection(pConn)`，`CDatabase::DestroyConnection` 没有 NULL 判断
→ 0xC0000005，进程直接没。数据库没配好时"关掉配置框就退出"就是这条。

### 改动
| 文件 | 改动 |
| --- | --- |
| `Server/DBSvr/MainWndProc.cpp` | `WM_CLOSE`：`g_fCloseConfirm` 重入保护 + 500ms 抑制窗（连点 X 不叠框）；默认按钮改成「否」(`MB_DEFBUTTON2`)；**取消分支只 `return 0`**——不置 `g_fTerminated`、不关 socket、不 `WSACleanup`、不 `PostQuitMessage`、不走 `DefWindowProc`；确认退出时先 `StopServiceThreads()`（等线程退出，3s 上限）再拆 socket/WSA/COM。新增 `_WM_LOGMSG` / `_WM_SESSIONCOUNT` 分支。菜单 `IDM_EXIT` 改走 `WM_CLOSE` 同一流程。`LoadCharacterRecords()` 加 `pRec == NULL` 保护 |
| `Server/DBSvr/DBSvr.cpp` | 日志改为「工作线程 Post 文本 → 主线程写 ListView」（`LogText` / `AppendLogText`），工作线程不再阻塞；新增 `g_dwMainThreadId`；`InitInstance` 数据库连接失败只记日志不崩（原来会拿空连接去查表/释放） |
| `Server/DBSvr/ServerCommSockMsg.cpp` | `UpdateStatusBarSession` 改 `PostMessage(g_hMainWnd, _WM_SESSIONCOUNT, n, 0)` |
| `Server/DBSvr/ProcessGateMsg.cpp` | 循环里检查 `g_fTerminated`，否则服务停了线程也不退 |
| `Server/DBSvr/StdAfx.h` / `DBSvr.h` | 新增 `_WM_LOGMSG`(WM_USER+1004) / `_WM_SESSIONCOUNT`(WM_USER+1005)、`LogText()`、`g_dwMainThreadId` |
| `Server/Def/_OrzEx/database.cpp` | `DestroyConnection` / `DestroyRecordset` 加 NULL 保护 |
| `Server/Def/dbmgr.cpp` | 构造函数初始化 `m_pConn` / `m_pfnLog`；`CreateRecordset` / `DestroyRecordset` / `__cbDBMsg` 加空指针保护 |

### 验证（本机无可用的 SQL Server，DSN 缺失也算一种边界）
自动化脚本对 Debug/Release 各跑一轮（Win32 窗口消息 + `BM_CLICK` 驱动）：

```
服务启动: 6000在听=True 5100在听=True 响应=True
连点 X 未叠框 (ok)
取消后: 存活=是 窗口在=True 响应=True 残留框=0 6000在听=True 5100在听=True
点是: 退出 code=0 耗时=78ms 端口已释放=True
```

即：取消后进程继续运行、界面可响应、服务端口仍在监听；确认退出 100ms 内干净退出。

### 注意
- 源码保持 **GBK + CRLF**（工程没有 `/utf-8`，`_MBCS`），改文件时别写成 UTF-8。
- `cmake --build build-vs18-check --target DBSvr --config Release|Debug`

## 2026-11-16 LoginSvr 菜单「Game Server Connection Status」点了没反应 / Configuration 卡死

### 症状
- 服务菜单 `Service -> &Game Server Connection Status`（`IDM_SERVERSTAT`）点下去像卡住：没有窗口、没有提示。
- 同一个菜单里的 `Configuration`（`IDM_CONFIG`）也有过点开后长时间无响应。

### 根因（四条）
1. **`IDM_SERVERSTAT` 根本没实现**。`MainWndProc.cpp` 的 `OnCommand()` 只处理
   Start/Stop/FontColor/BackColor/Config，这个命令落到 switch 外面，
   既不弹窗也不报错 —— 表现出来就是"点了没反应，像卡住"。
2. **`Configuration` 的服务器列表枚举是死循环陷阱**。`ServerListProc.cpp::InitListView()`
   原来写的是 `while (jRegEnumKey(...) != ERROR_NO_MORE_ITEMS)`，而 `jRegEnumKey()`
   在键打不开时返回 `-1`（不是 `ERROR_NO_MORE_ITEMS`）。只要
   `HKLM\Software\LegendOfMir\LoginSvr\Servers` 不存在、或者 32/64 位注册表视图错位，
   循环永远退不出去，每轮还往 ListView 里插一条空记录 → UI 线程彻底卡死。
3. `jRegGetKey()` 是"先查大小再按该大小写"的写法，调用方传的是 `TCHAR szTitle[64]`，
   注册表里的值一长就把调用方的栈缓冲写爆（枚举时每个子键都会读 Title）。
4. 无数据库时点 `Start Service` 直接 `0xC0000005`：`LoadAccountRecords()`
   没判断 `GetDBManager()->CreateRecordset()` 返回 `NULL`（DB 没连上时它就是 NULL），
   拿着空指针去 `Execute()`。进程两秒左右消失，看起来也像"卡一下然后没了"。

### 改动
| 文件 | 改动 |
| --- | --- |
| `Server/LoginSvr/StatusDlg.cpp`（新增） | `Service -> Game Server Connection Status` 的实现：对话框列出①登录服务器监听状态（`g_gcSock` + 5500）②已连接的 Login Gate（`getpeername` 对端地址、端口、该网关上客户端数）③`TBL_SERVERINFO` 里的游戏服务器（编号/名称/下发 IP/选服次数）。**只读内存快照**，不发网络请求、不查数据库，所以弹出与刷新都是毫秒级；遍历运行期链表用 `__try/__except` 兜底，撞上并发释放也只是少显示一行。 |
| `Server/LoginSvr/MainWndProc.cpp` | `OnCommand()` 补 `case IDM_SERVERSTAT` → `ShowServerStatusDlg(g_hMainWnd)`；`LoadAccountRecords()` 增加 `CreateRecordset() == NULL` 早退（记 `IDS_DBCONNECTFAIL`，不再建监听）；`SELECT TBL_SERVERINFO` 与 `DestroyRecordset` 加空指针判断。 |
| `Server/LoginSvr/ServerListProc.cpp` | 枚举改成 `for (nLoop < 512)` + `!= ERROR_SUCCESS` 才继续，`ERROR_SUCCESS` 之外一律 `break`；Title 改用带缓冲区大小的 `jRegGetKeyBuf()`。 |
| `Server/Def/RegstryHandler.cpp` | 新增 `jRegOpenKeyRead()`：默认视图打不开就依次试 `KEY_WOW64_64KEY` / `KEY_WOW64_32KEY`（regedit 写 64 位视图、32 位程序读 Wow6432Node 的错位就靠这个兜）；`jRegEnumKey()` 打不开键时返回 `ERROR_NO_MORE_ITEMS` 而不是 `-1`；新增 `jRegGetKeyBuf(subkey, value, buf, size)`，超长值只截断不越界。 |
| `Server/LoginSvr/GateInfo.cpp` | `MakeNewUser` / `ReceiveServerMsg` / `ProcAddUser` / `ProcLogin` 四处 DB 调用补 `NULL` 检查：数据库不可用时回 `SM_NEWID_FAIL` / `SM_ID_NOTFOUND`，不再空指针往 `Execute()` 里冲。 |
| `Server/LoginSvr/LoginSvr.rc`、`resource.h` | 新增 `IDD_SERVERSTAT`（106）对话框、`IDC_STATUS_LIST/REFRESH/SUMMARY`（1017–1019）、`IDS_STATUS_*`（25–32）与 `IDS_DBCONNECTFAIL`（33）。 |
| `Server/LoginSvr/CMakeLists.txt` | 加入 `StatusDlg.cpp`。 |

### 验证（本机无 SQL Server，正好覆盖"DB 连不上"这条边界）
Win32 消息驱动（`WM_COMMAND` + `EnumWindows` 轮询窗口出现时刻，测的是"点菜单到窗口出现"的真实延迟）：

```
===== Release =====
[1] Connect status : 25.3 ms, found=True, rows=1, summary='0 login gate(s) connected, 0 game server(s) registered.'
[2] Configuration  : 17.8 ms, found=True
[3] Start Service  : alive=True
[4] Connect status : 27.0 ms, found=True, rows=1, summary='0 login gate(s) connected, 0 game server(s) registered.'
[5] Stop Service   : alive=True

===== Debug =====
[1] Connect status : 12.3 ms, found=True, rows=1, ...
[2] Configuration  : 16.3 ms, found=True
[3] Start Service  : alive=True
[4] Connect status : 30.6 ms, found=True, rows=1, ...
[5] Stop Service   : alive=True
```

修之前同一脚本跑到 `Start Service` 后 2.1s 进程就退出（`exit code 0xC0000005`，WER `APPCRASH LoginSvr.exe 0x5b4f`）；
修之后 Start/Stop/两个菜单全流程进程存活、界面即时响应。
`rows=1` 是 "Login Server / Local listener / 0.0.0.0 / 5500 / Stopped" 这一行——DB 不可用时服务确实没起来，
所以显示 Stopped；有数据库时还会多出每个 Login Gate 与每个 Game Server 的行。

### 注意
- 源码保持 **GBK + CRLF**；新增的 `StatusDlg.cpp` 也是 GBK。
- 改 `Server/Def/*` 会影响所有 Server 端程序（Def 是静态库），本次只加函数 + 改错误返回值，不改签名。
- `cmake --build build-vs18-check --target LoginSvr --config Release|Debug`

## 2026-11-16 V1.45 资源适配盘点

- Server 使用 HKLM\Software\LegendOfMir 下的服务配置和 SQL Server ODBC。LoginSvr 使用 Mir2_Account；GameSvr 使用 Mir2_Common 及 GameServer\Device；DBSvr 使用 DatabaseSvr\Device。Mud3/Setup 和 Envir 不能直接作为该工程的配置目录；需要转换世界数据到 TBL_*，脚本另行适配。现有 SQL 脚本含 DROP TABLE，且未找到 TBL_GAMEGATEINFO、TBL_MONITEM、TBL_CHARACTER_HORSE、TBL_CHARACTER_GENITEM 的建表语句。
- Client 运行时读取工作目录下的 Mir3.ini（仓库样例叫 Mir2Ei.ini），另读 Data/*.wil 与同名 .wix、Map/*.map、SoundList.wwl、weapon.ord 和音频。宝藏传奇3/Data 有 99 对 WIL/WIX，但缺角色选择及游戏界面引用的 Interfacec2.wil/wix；a.wil 是 Mir2Ex.cpp 中另一个引用，须按实际构建入口核对。
- 宝湾传奇3服务器端.7z 的主要目录为 Mud3/；工作区中的宝藏传奇3/是另一套客户端资源。磁盘 Mud3/Map 与宝藏传奇3/Map 有 598 张同名地图，其中 597 张字节相同，仅 z014.map 不同。样本 0、01、3、d001 的长度及字节模式符合 28 字节头、3 字节 tile、14 字节 cell；当前服务端和客户端分别按 10 和 12 字节 cell 读，不能直接混用。先核实 14 字节字段与语义，再让两端共用解码器，用 01.map 验证显示、碰撞和坐标。
- 本次是源码与资源的只读盘点，未启动服务或导入数据库。

## 2026-11-16 01.map 适配进行中（尚未联调通过）

- Shared/Map145.h 新增共享格子读取，支持 14 字节记录和旧 10/12 字节记录。客户端使用独立对象图包编号；字段映射仍需画面验证，尤其门和保留字节。
- out/mud3-01 包含 Mud3 的 01.map、Mir3.ini，并通过目录联接引用宝藏传奇3的 Data/Sound，另复制 SoundList.wwl 和 Weapon.ord。Interfacec2 仍缺失。
- 用户指定使用 Mud3/GSQL。GSql.exe 已启动，sqlservr.exe 存活，但未监听 1433，日志未更新，数据库尚未建立。此前 Express 安装尝试以 2227437582 失败，已停止该路线。
- 当前 VS 18 实例 isComplete=false、isLaunchable=false。直接 MSBuild 报缺失 Microsoft.VCToolsVersion.VC.14.52.props，CMake 也无法识别完整实例。此次代码尚未编译验证，登录、建角、进图均未完成。

### 后续诊断
- VS Installer 进程 18384 正在执行更新，UI 显示下载 1.64/2.85 GB（56%）、安装 969/1147（71%），并非需要重新安装另一个 VS。当前 MSVC 目录已被更新过程移除，等待更新完成后重配 CMake。
- GSQL 引擎版本 8.00.2039（SQL Server 2000 SP4），进程 18760 只有一个持续运行线程，累计 CPU 超过 4 分钟，无新增 SQL 日志、无 1433 监听。尚未查明循环位置，未改业务库。IDA 启动脚本 exec session 40375 仍在运行，后续应先检查其状态。
- 两端地图读取增加文件头读取长度、偶数尺寸及完整文件长度校验，避免损坏文件按任意尺寸分配。01.map 确认为 600x600，5310028 字节，与 14 字节格子布局一致。编译及图层语义验证仍待进行。

## SQLite 可选后端（实现已加入，编译待验证）

- 用户明确要求增加 SQLite 后端。Server/Def/_OrzEx/database_sqlite.cpp 复用 CDatabase/CConnection/CRecordset 接口；sqlite: 路径或工作目录 database.ini 的 [SQLite] DSN映射启用 SQLite，其他连接继续 ODBC。示例 Server/database.ini.example。数据库文件由连接创建，但业务表需独立初始化，示例不是完整数据库部署。
- SQLite 3.54.1 官方 amalgamation 已下载并通过官方 SHA3-256 校验，源码及出处在 Server/Def/sqlite；CMake 静态链接。连接使用 FULLMUTEX、WAL、foreign_keys 和 5000ms busy timeout，文本在 CP_ACP/UTF-8 间转换。查询拒绝多语句，复用 recordset 时清理旧状态，NULL 返回空字符串保持现有 Get 使用方式。
- 业务 INSERT TBL 和查询构建器改为 SQL Server/SQLite 都支持的 INSERT INTO。添加 test_database_sqlite 覆盖约束错误、行数、NULL、重复 Fetch、事务回滚、中文和长字段。
- CMake build-sqlite 配置仍因 VS 正在更新、无完整实例而失败，测试尚未执行。登录、建角、进图尚未验证。

### SQLite 最小库落地
- Server/SQL_Scripts/sqlite_minimal.sql 创建 26 张表，补齐当前 GetHumanRcd/Init* 查询字段和角色物品逐条记录布局。out/mud3-01/mir3.sqlite3 已执行两次，integrity_check=ok；实际建角 INSERT 和角色加载字段查询在事务内验证后回滚。仅证明数据库结构，未证明服务端流程。
- 本地测试账户 mirtest/test123，服名 Mir3Local。database.ini 将 Mir2_Account/Mir2_Common/Mir2_Game 映射到同一个本地库；[Game] StartMap=01。DBSvr 建角读取该配置，缺出生点返回失败；旧默认地图4保留。
- SQLite 字符串缓冲补零余量，兼容旧代码固定长度 memcpy 读取物品编码；管理员字段改用 atoi 避免字符0误判。
- VS 更新已安装部分 14.52.37025 include/lib，但 cl.exe/vcvars 尚未就绪，仍不能配置构建。下一步待工具链恢复运行 test_database_sqlite，继续补道具种子和缺失 Interfacec2。

### 构建恢复与 SQLite 测试通过
- vcvars32.bat + Ninja 已可用，构建目录 build-ninja-sqlite。out/build-sqlite.cmd 构建测试，out/build-game.cmd 构建 Client/GameSvr/LoginSvr/DBSvr/LoginGate/SelGate/GameGate，均已成功。VS 生成器仍受安装状态影响。
- test_database_sqlite.exe 输出 SQLite backend smoke checks passed，覆盖中文、2048 字节长字段、NULL、约束、回滚和结果复用。
- 修复 Shared/Map145.h 先于 stdafx 引入 windows.h 导致 winsock/winsock2 重定义的问题。
- 宝藏传奇3 WIX 实测文件长度为 28+count*4，原读取器按24字节头读取，首条错读额外字段。WHWilImage::NewLoad 已按文件长度兼容24/28字节头并验证计数，NewSetIndex 先查边界再访问索引。
- Interface1c/GameInter 在145、169、254、255处多为空，Inventory同帧号是小物品图标，不能靠改名替代 Interfacec2。后续需预览并匹配正确图包及帧号。尚未启动端到端流程。

## SQLite backend migration commit

- Added SQLite 3.54.1 as a bundled static library, with the upstream URL and verified SHA3-256 in Server/Def/sqlite/SOURCE.txt.
- CDatabase/CConnection/CRecordset accept sqlite: paths or DSN mappings from the working directory's database.ini. Other connections use the existing ODBC implementation.
- Copy Server/database.ini.example to the runtime working directory. Initialize mir3.sqlite3 with Server/SQL_Scripts/sqlite_minimal.sql using SQLite's CLI or Python sqlite3.executescript. This is a minimal local test schema, not an import of the Mud3 business database.
- The example maps Mir2_Account, Mir2_Common and Mir2_Game to one file and selects StartMap=01. The local seed account is mirtest/test123. DBA/game-server Device must select Mir2_Game.
- Added SQL-compatible INSERT INTO statements and numeric admin flag parsing. StartMap defaults to the previous map 4 when absent; missing spawn configuration returns a character creation failure.
- Tests passed: query reuse, constraints, affected rows, NULL values, transaction rollback, Chinese text, long fields, and file persistence across reopening.
- Built Client, GameSvr, LoginSvr, DBSvr, LoginGate, SelGate and GameGate via vcvars32 + Ninja. End-to-end login, character creation and map entry remain unverified. Map/resource changes and service INI migration are separate work.

## 2026-11-16 INI and runtime integration evidence

- SQLite backend committed separately as e12f077. INI, map and resource changes remain uncommitted.
- Server/Def/RegstryHandler.cpp now stores legacy configuration calls in working-directory server.ini, without registry access. Server/server.ini.example supplies the local topology. Sized string reads replace Device/MapFileLoc reads; LoginSvr server-list IP reads now pass addresses correctly. test_server_config.exe passed, including Chinese strings, IPv4, enumeration, missing/invalid values and short buffers.
- Deployed builds to out/mud3-01 and started DBSvr, LoginSvr, GameSvr and three gates through their Service menus. All three gates reported Connected; ports 5000, 5100, 5500, 6000, 7000, 7100, 7200 were listening. GameSvr logged Read map file (Map\01.map) info and connection to DBSvr. This proves server map loading, not client rendering/collision.
- GameSvr startup revealed missing FLD_INDEX in TBL_STDITEM and TBL_MONSTER. Added both columns to sqlite_minimal.sql and ALTERed the local runtime database; integrity_check=ok. Server/Tools/test_sqlite_schema.py executes the 17 actual GameSvr startup SELECT strings against a fresh database after two schema applications; passed. Running GameSvr has not yet been restarted to verify clean startup logs.
- Mir2Ex was launched, but desktop control was stopped with the physical Escape key before client state could be inspected. Do not claim login/character creation/map entry success. Subsequent work used source/files only.
- Fixed WIL loader initialization clearing the smaller NEWWIX structure with the larger legacy WIX size, filename clearing size, array delete mismatches, and the successful NewLoad path leaking its WIX handle. Client rebuilt successfully; running client still uses the previous build.
- Interfacec2 is still absent. Existing Interface1c, GameInter, Inventory are not proven frame-compatible substitutes. A scan of all asset packs found no UI frame near 362x180 (message-box requested size); this does not identify a replacement. Resource adaptation and end-to-end flow remain unfinished.

### Resource-driven UI adaptation
- User explicitly requested that the client follow the supplied resource artwork. Added Client/Tools/inspect_wil.py to list/decode V1.45 frames and export contact sheets (Pillow optional). out/ui-resource-contact.png and out/interface-buttons.png visually verified Interface1c layout.
- Verified Interface1c: 50 selection background; 51/52 create, 53/54 delete, 55/56 start, 57/58 exit; 80 creation background; 85-87 confirm, 88-90 cancel, 91-99 three job icon states. Corrected selection constants and both creation-background definitions. Selection buttons now always render, using the second frame for hover/pressed. Previously labels were only drawn when pressed despite absent labels in background.
- Login images 10/11,12/13,14/15,16/17 are two-state pairs; corrected frame selection to avoid adjacent-button images. Initialized login input state before Render indexes m_xInputState[state-1]; moved login button left to remove overlap with account button. Client builds successfully. Not yet deployed to the running client or visually exercised.
- Critical unfinished behavior: CCharacterProcess::OnLButtonUp PRG_CREATE_CHAR has commented-out creation controls and currently returns to selection on any click. RenderCharCreate uses missing 100-105 frames. Must restore resource-aligned controls and invoke CClientSocket::OnNewChar, handle server response. Do not mistake frame mapping/build success for functional character creation.

## Server INI configuration migration

- Service configuration now reads/writes server.ini in the process working directory via the shared legacy configuration entry points; registry settings are no longer read or written by this implementation.
- Copy Server/server.ini.example to server.ini in the runtime working directory. Database backend/DSN mappings remain in database.ini. Existing registry values are not automatically imported.
- Device and MapFileLoc reads use explicit buffer sizes. LoginSvr server-list IP reads pass writable addresses.
- test_server_config covers IPv4 round trips, Chinese text, short buffers, child-section enumeration and missing/invalid values; rebuilt and passed before commit.
- Map decoding, client resources and SQLite schema follow-up fixes are separate changes. End-to-end login, character creation and map entry remain unverified.

### Character creation flow repair in progress
- Confirmed CM_NEWCHR protocol mismatch: ClientSocket sent slash text, DBSvr decoded packed _TCREATECHR (43 bytes). SendPacket now accepts an optional explicit body length, OnNewChar sends the zero-initialized character/account structure with a compile-time size assertion. Other callers retain string-length behavior.
- Fixed DBSvr account character limit query to filter FLD_LOGINID and Execute && Fetch (previously filtered name and skipped Fetch with ||).
- Creation UI now shares frame/position definitions for drawing and hit-testing: confirm85 at432,420; cancel88 at472,420; jobs91/94/97 at260/310/360,365. Confirm sends OnNewChar, blocks repeated submission until response; cancel returns without sending. Missing preview100-105 is no longer dereferenced. This is unfinished: actual character art, gender selection, failure feedback and full layout still need adaptation.
- Success response returns to selection and queries records; failure releases pending state. Zero-character response now creates/opens selection UI; previously it skipped initialization entirely. Query refresh clears previous lists. Selection button allocations are released before recreation.
- Client/DBSvr rebuilt after packet and handler changes; final selection allocation cleanup still needs rebuild. No live login/create/map-entry verification. Running EXEs remain older builds.

### Authoritative Godot reference supplied by user
- User specified Mir3's Godot branch layout/configuration is correct. Reference repository G:/传奇3/Mir3, branch br_godot_test_261115, read with git show (no checkout/change to that repository). Use client/src/godot/hud_view.cpp, client/src/hud/create_character.hpp, character_selection.hpp and resources/login_config.hpp as layout/config sources. Do not continue inventing positions from sheets.
- Applied creation coordinates from hud_view.cpp: profession buttons266/308/352,419; confirm450,444; cancel491,444; panel81 at247,384; editor288,405 size75x13, limit13 bytes. Added two gender previews with reference positions and action4 animation (first frame440+600*job+300*gender); clicking rendered bounds selects gender. Body drawing is present but shadow/inactive/light blends, description panel, tooltip/hover/pressed details are still missing. Build passed.
- Reference login coordinates (not yet applied): buttons10/12/14/16 at459,436 /139,379 /279,379 /439,379; ID128,440 and password326,440. Notices use GameInter1240 at140,110 and1243/1244 at277,298. This gives a concrete replacement path for missing Interfacec2; do not alias unrelated Inventory frames.
- Independently found and repaired role-list protocol: DBSvr used certificate part of account/certification as account and rejected count0 and3. Now queries the account, returns packed records with count in wParam; client decodes same23-byte records (max3). This legacy C++ server protocol intentionally remains distinct from Godot/original text protocol; user requested layout/config reference, not replacing the game client with Godot. Full interoperability not verified.
- Need finish selection rendering using reference action frame formula and positions. Existing old frame150-based rendering is still incorrect; newly created role hitboxes are provisional. Empty list Start guard fixed by SELECT_NONE.

### Godot-reference layout continuation
- Login.cpp now uses reference button/editor positions; credentials stay visible alongside actions, login click submits entered values. Editor/text origins aligned without old +5/+7 offsets. Login tab/enter behavior still needs runtime verification and reference parity review.
- Selection button positions now match hud_view.cpp (440,93;79,243;259,49;28,438). Replaced obsolete frame150 animation with reference 200+60*(action+5*(gender+2*job)), idle action0 and selected loop action2. Bounds update from actual rendered WIL dimensions/offsets. Third-character paging, transition action1/3, shadows and additive overlays remain unfinished; currently no pagination.
- Decoded GameInter1240..1248 and verified notice background1240 (360x268), YES1241/1242, OK1243/1244, NO1245/1246. Reference login notice at140,110, OK at277,298. Existing CMirMsgBox3D still uses old254/255 and Interfacec2/Inventory references: must replace with correct notice resource and layout before end-to-end UI test.
- Latest selection/layout build passed (out/selection-layout-build.log). Changes not deployed; no live login/create/map proof yet.

### Login/selection notices now use supplied resources
- Added Common/LoginNotice.h/.cpp for fixed 640x480 login/selection notices. Background GameInter1240 at140,110 and OK1243/1244 at277,298 follow the Godot reference. YES/NO uses1241/1242 and1245/1246; its two-button coordinates still need checking against reference deletion-dialog semantics. Message text wraps at ANSI character boundaries.
- LoginProcess and CharacterProcess now use CLoginNotice and load GameInter into m_Image2 instead of Inventory/Interfacec2. Other game UI still references Interfacec2 and must be adapted separately; do not claim all missing-resource dependencies resolved.
- Added visible creation-failure feedback with DBSvr wParam reason and invalid-name feedback. Name symbols now reject the reference forbidden ASCII set.
- Verified every login/selection control frame, seven notice frames, and all frames of six action4 creation animations exist in supplied WIL/WIX. Client built successfully (out/notice-final-build.log). Runtime deployment, visual parity, login/create/entry remain unverified.

### Live login and server selection verified
- Connected by TCP to local LoginGate7000, sent actual CM_IDPASSWORD2001 for mirtest/test123 with native six-bit codec. Received header(0,529,1,0,0), body1,Mir3Local,NUL. Then sent CM_SELECTSERVER104 with wParam1 over the same connection; received header(0,530,32,0,0), body127.0.0.1 NUL. This proves real login/selection through running gate/LoginSvr/SQLite, not client UI or creation/entry.
- Fixed ClientSocket::OnSelectServer hardcoded ktest/wParam0. It matches configured ServerNName, uses ServerNIndex (default N), sends numeric index to current C++ LoginSvr. Fixed client SM_SELECTSERVER_OK to support the legacy IP-only body/certification wParam; SelectGatePort comes from Mir3.ini Initial (default7100). Existing slash endpoint parsing retained.
- Initialized LoginSvr ProcSelectServer pServerIP to NULL. Client/LoginSvr rebuild passed (out/select-endpoint-build.log). Runtime EXEs still earlier versions; DBSvr needs redeployment for the new creation/list protocol. Do not assume network tests exercised un-deployed edits.

### 2026-11-17 live creation and startup evidence
- Removed unconditional debug startup in Mir2Ex.cpp that loaded login then character then game and a.wil. Normal startup now stays in login. Built and deployed Client+DBSvr to out/mud3-01, replacing only our test processes. Process Mir2Ex21288 was responding; UI state/visual login not yet verified.
- Restarted DBSvr service via menu and SelGate service after DB restart; SelGate UI confirmed Ready/Connected. GameSvr did not reconnect (port6000 had listener only) and must be restarted/reconnected before entry test.
- out/probe_character.py is the actual socket test used: login2001 ->529; select104 ->530; query100 ->520 count0; create101 packed43-byte record ->521; query100 ->520 count1. SQLite confirms mirtest/MirProbe01 level1, job0, gender0, map01 at459,261. Do not repeatedly create new characters; script reuses existing test character. This proves actual network creation and persistence, not UI operation or map entry.
- Computer Use was stopped again by physical Escape while locating the client window; further work used source/files only.
- Found entry mismatch: DBSvr SM_STARTPLAY sends plain IP, client required slash/port. Added fallback Initial/GameGatePort (7200) while retaining slash parsing. DBSvr now returns SM_STARTFAIL if no game gate or connected game server, instead of dereferencing absent/uninitialized server. Client displays failure. Built successfully (out/entry-build.log), these last entry changes not deployed.

### Staged commits requested by user
- f147438: server character-flow fixes, missing SQLite startup columns and schema regression test. SQLite C++ smoke test and all 17 schema startup queries pass; live creation evidence is recorded above.
- 1014d20: shared Mud3 14-byte map decoding and corresponding client/server consumers. Compiled; GameSvr loaded01.map. Client visual layers/collision still need runtime confirmation.
- Client resource/UI/protocol/startup adaptations are committed as a separate integration milestone. Godot reference layouts are partly applied; WIL reader fixes and inspection tooling included. Not a completed client release: game UI still references Interfacec2, three-role paging and some visual effects remain unfinished, live graphical login/map entry not verified.
- Prior INI migration commit abbed0d already exists and is preserved. No remote push performed.

### Starter item definitions (local test fixtures)
- Allocate standard item tables by MAX(FLD_INDEX)+1 and load rows at their real IDs; initialize item fields so sparse slots are deterministic. Seed weapon7, armor33/34 and general item0 only when absent. Values are explicit local integration fixtures, not imported Mud3 data.
- Schema repeat/custom-definition preservation test passed; GameSvr rebuilt (out/starter-items-build.log). Runtime database has not been updated and entry is not validated.
- User reports a small black client rectangle. Priority now is client startup and Godot-reference UI/flow parity before further game-entry integration.

### Client startup black rectangle repaired
- Root cause: InitPatch resized window479x359, but RenderScene never called RenderPatch and required connection success before drawing; no connection was initiated. Load now enters credentials directly, opens static background, connects, and renders independently of connection. Initial framebuffer640x480; EXE-relative resource/config paths re-read at Load. Missing movie playback/group-selection parity is still outstanding.
- Live CUA screenshot confirmed login background/buttons/editor are visible. First run exposed RGB565 sprite writes into desktop32-bit backbuffer (half-width/purple image). Explicit RGB565 offscreen pixel format fixes actual live rendering. Also fixed erroneous hr==CreatePrimarySurface comparison.
- Tab no longer submits from password; render no longer calls focus-changing SetFocusBefore each frame. Removed editor WM_PAINT self-invalidation loop so default EDIT painting completes. Build passed; last editor change requires deployment/visual regression.
- User input detected during live input testing; stopped desktop control. Graphical authentication/creation/map entry still not proven. Current deployed binary includes startup and RGB565 fixes, not last editor repaint fix.
- Updated user AGENTS prioritizes matching Mir3/Godot login configuration/layout/flow before deeper authentication/creation/selection/deletion integration. User AGENTS edits remain unstaged.

### Login configuration/list/input parity continuation
- Replaced legacy multi-column server buttons with Godot-reference text-measured single-column bounds (20,60 outer,20,70 rows; measured text +90 width,+20 row height). Fixed count+1 extra button, repeated Create cleanup, per-frame font leak, and require matching press/release to submit. Captions/names now MAX_PATH matching INI read sizes. Draw3DBorder still supplies legacy filled rectangles; exact reference outlines/status captions remain pending.
- Login notices hide editor and support Enter/Escape/Space dismissal; returning to credentials clears password and restores password focus. Offline login page buttons no longer depend on connected flag. Built Client successfully (out/login-parity-build.log); these changes are not deployed or visually verified yet.
- Automatic approval rejected attempted registration/password external URL-launch code (blocked by policy, no detailed reason). That change was not written; buttons remain incomplete. No external URL was opened.
- Reference message text comes from CmList.dat (resources/messages.cpp in Mir3 branch), not Mirmg.dll. Current legacy DLL captions are still wrong/missing. Next implement CmList reader and correct login message IDs. Server group page from SelLogin.ini/LoginInterface, login announcements/movies, full character visual parity, game HUD/bag/chat remain outstanding per user AGENTS.

### Native CmList.dat captions
- Added CLoginMessages loader following Mir3 resources/messages.cpp: encrypted length/checksum, in-memory decode, string-key caption table, transactional load. Login failure IDs now112/113/114/116/117/119 instead of old Mirmg.dll IDs; absent captions show numeric diagnostic rather than empty text. Remaining obsolete registration/password-change message IDs are not yet mapped.
- Client build passed (out/login-messages-build.log). Compiled native Client/Tools/test_login_messages.cpp and ran against original Baozang CmList.dat: LOGIN_MESSAGES_OK for real required caption IDs, corruption/truncation rejection and previous-table preservation. Temporary corrupt fixture is out/corrupt-cmlist.dat; original resources unchanged.
- Copied CmList.dat into out/mud3-01. Running EXE has not been replaced during user interaction; latest source/list/notices/message-reader changes still need deployment and live UI verification.
- User AGENTS now also enumerates later game UI, bag, chat, NPC, skills and combat backlog. Preserve these requirements; do not claim complete game from current login work.

### Live graphical login evidence and requested window scaling
- Deployed c511a4b and tested via CUA: mirtest input retained across Tab, badpass produced resource notice, dismiss restored empty password editor, test123 reached single Mir3Local server row, click reached role list with MirProbe01. This is actual graphical login/selection, not only socket evidence. Enter notice dismissal needs further checking (first observation unchanged, mouse close verified on later snapshot).
- Clicking Create crashed process. Windows Application event1000 records exception0xc0000094 (integer divide by zero), RVA0x38ef4. Background unconditionally starts absent CreateChr.avi; CAvi ReadDatas divides by uninitialized/zero FPS. Must fix failed movie handling. No graphical creation or map-entry success yet.
- User requires larger window matching Godot. Reference project viewport800x600 (per-page login coordinates640x480). Separate presentation from logical backbuffer: Display/WindowWidth=800,WindowHeight=600 by default, multiplied by system DPI. Mouse messages map back to scene coordinates; shared MoveScaled positions/sizes editor and font, all editor callers updated. Server-row D3D projection uses logical height. Build passed (out/client-window-size-build.log), deployed and launched. Physical Escape stopped CUA before new-size verification; do not claim visual confirmation.

### Missing character-transition movies
- Character background now uses reference CreateChr.dat/StartGame.dat names and checks Create return before Start; missing movies advance to the corresponding static/entry state. CAvi tracks successful load so Start/Draw cannot consume uninitialized FPS after failed load. Client compiled (out/missing-movie-build.log). Runtime crash fix not yet exercised.

### Goal replaced: GUI parity first, business login later
- User replaced active objective with complete client GUI, all pages/dialogs, matching MIR3.EXE via completed Godot branch; fonts/colors/sizes included. Do not continue server/login protocol debugging until GUI work is ready. User also requires centered initialization and noted absent titlebar.
- Windowed style now has caption/system menu/minimize; title changed to Chinese Mir3. Presentation dimensions remain configurable800x600 logical display units with system DPI scaling. Position is centered inside nearest monitor work area using full outer window dimensions.
- Godot native_text.cpp normal/dialog font uses Songti9pt, DEFAULT_CHARSET and DPI-unaware96-DPI rasterization. Updated default render font and logical font sizing to match (presentation scaling happens once), editor uses matching face without WS_EX_CLIENTEDGE. Selected-server rendering now uses default font; fixed ownership issue discovered because PutsHan deletes supplied nondefault fonts.
- Selection deletion YES/NO positions corrected to reference181/374,298. Character information panel, animation blends/paging, group selector, announcements, full game HUD/dialogs still incomplete.
- Client build passes out/client-font-window-build.log. Latest title/font/centering/dialog changes are not deployed or visually verified; prior physical Escape stopped desktop use. Running process28644 remains prior viewport build. User AGENTS edits are separate, do not stage accidentally.

### 2026-11-17 原生选角、建角和删除确认界面
- 依据 Mir3 br_godot_test_261115 的 hud_view.cpp、selection_pixels.hpp 和 native_text.cpp，移植角色信息框、建角职业说明、角色阴影/灰度/加色层和删除确认对话框。渲染仍使用原有 WIL、RGB565 DirectDraw 与 Windows 编辑控件。
- 用 --preview=select、--preview=create、--preview=delete 离线运行同一客户端的页面代码，截图位于 out/gui-select.png、out/gui-create.png、out/gui-delete.png。客户端编译通过。选角分页、动画状态、游戏 HUD 和真实删角仍有待验证或实现。

### 原生页面音频接入进展
- 恢复 CBMMp3 的 DirectShow 初始化与真实加载失败返回；循环播放复用现有媒体图，先释放事件参数再处理停止，避免访问已释放事件接口。
- 接入登录 Opening.wav（1200ms）、选角 SelChr.mp3 和建角 CreateChr.mp3（1000ms）延迟播放，换页停止旧声音；建角职业/性别切换接入原资源语音。仍使用 CLIENT 自身声音模块。
- Client 编译通过；建角离线预览在超过音频延迟后仍存活。尚未进行音频回环录制或听感验收，选角语音、转场音效和游戏内音效仍需继续接入。

### 选角与转场音效
- 选中不同角色时按职业/性别播放对应选择语音；进入建角、进入游戏的转场分别播放 CreateChr.wav、StartGame.wav。
- 修正 MP3 Stop 销毁媒体图导致无法重播的问题，以及 IsPlaying 错报初始化状态的问题。编译通过；尚待声音输出与交互验收。

### 选角动画状态对齐
- 采用 Godot CharacterSelectionPage 的 30 组动画长度与 120ms 阈值，角色拥有独立帧计时；选中使用 action1→2，取消使用 action3→0，灰度随动作状态切换。
- 编译通过并运行选角、建角、删除确认三个离线页面；截图更新至 out/gui-*.png。完整点击切换与三角色分页仍需继续验收。

### 原生选角分页
- 增加 GameInter1150–1155 翻页按钮及按下/禁用状态，采用参考坐标500/550,400；绘制和命中均限于当前页两名角色，避免第三名角色覆盖第一页。
- 增加 --preview=select-three 三角色离线预览。编译通过，分页交互与截图验收尚待完成；本次不宣称页面验收完成。

### 分页验证与资源生命周期
- 三角色预览进程6080运行且响应，但 computer-use两次激活窗口均返回 failed to activate captured window，未完成点击翻页验证。没有把捕获到的桌面图片当作客户端验收证据。
- 检查发现 NewLoad 不释放旧映射，刷新角色列表会重新加载分页资源；在重新加载前显式 Destroy，避免映射和索引泄漏。编译通过。

### 提示框文字排版对齐
- 按 Godot native_text.cpp 使用同一 GDI DrawText 标志：长文本按字体宽度自动换行、整个文本块垂直居中，短文本单行居中；删除确认使用278×75，普通提示使用308×95逻辑像素区域，并裁剪至区域。
- 移除固定50字节截行和固定18像素行距。Client编译及diff检查通过；最新提示框运行截图仍待验证，不宣称视觉验收完成。

### 建角按钮状态
- 按参考输入与帧表补齐普通、悬停、按下三态；道士按钮第三态仍使用98帧。职业选中状态不再误作悬停帧。
- 编译通过；鼠标交互截图和提示文字仍待完成。

### 建角提示与动画重置
- 原生绘制五个按钮的 CmList201–205 悬停提示，位置、底色、边框与四向文字阴影对齐 Godot hud_view.cpp。
- 切换职业/性别时重置动画计时；非法名字提示改用 CmList225，去掉硬编码英文。编译通过，鼠标悬停截图尚待验收。

### 游戏HUD迁移开始
- 核对 Godot HudLayout：主框架为GameInter1160，位置0,601-height。原CLIENT仍使用旧50帧，现已修正主框架资源与底边。
- 这仅是HUD迁移第一步，现有按钮80–115、血魔球、聊天框、背包与其他子窗仍是旧布局；Interfacec2依赖尚存。尚不可宣称游戏主界面完成，需继续逐项移植并提供运行截图。
- Client构建通过，日志out/hud-frame-build.log。

### HUD主按钮资源映射
- 保留CLIENT事件编号，按功能映射交易、小地图、技能快捷栏、退出、注销、组队、行会、技能、聊天、任务、选项、坐骑、背包、角色共14个按钮的位置与资源帧。
- 对齐参考基础五按钮无悬停图、其余按钮正常/按下帧配对。现有腰带与帮助两个按钮尚未适配聊天展开/攻击模式，命中区域覆写与子窗口仍待迁移。
- Client构建通过；未宣称整体HUD视觉验收完成。

### HUD按钮独立命中区域
- CGameBtn新增可选命中矩形，图像绘制仍使用原坐标；九个已映射HUD按钮使用Godot HudLayout命中覆写。
- 因按钮类布局改变，执行Client clean-first完整77步重编译通过（out/hud-clean-build.log），避免旧对象混用。视觉/点击仍待运行验收。

### HUD血魔球与计量条
- 依据HudModel/HudLayout改为1163–1167资源与33/73,8及152/165,26偏移；低于24级战士右半球共享HP，负重先绘制、经验后绘制。
- 使用真实能力上限计算裁剪，零上限显示空值；保留原生DirectDraw压缩帧裁剪。编译通过。等级/属性/头像/聊天和运行截图仍待后续完成。

### HUD等级和坐标文字
- 对齐等级位置393,466、8pt参考字体与颜色；坐标区域6,575,133,17，加入ONE/INVSRCALPHA底色和四向文字阴影，采用垂直/水平居中。
- 原头像及属性图标仍待迁移，聊天区域尚未完成。Client构建通过，运行截图未验收。

### HUD属性图标及切换
- 移植1500–1503属性图标、数值位置与文字颜色；点击左侧切换防御/魔防，右侧切换物攻/魔攻（道士为道术），仅在未被子窗口覆盖时响应。
- 去除该区域旧64–66职业图标与120–124属性图标。头像需独立肖像资源加载，尚未完成。Client clean-first完整77步构建通过，截图及交互仍待验收。

### 默认职业头像接入
- 确认ProgUse650–655六帧均存在且为84×94；按职业×2+性别映射默认头像，使用参考HUD偏移534,20，沿用原生WIL加载和DirectDraw绘制。
- 显式管理头像资源生命周期，Client完整77步重编译通过。自定义FAC头像、目标头像与HUD整体运行截图仍未完成。

### 收起聊天区域
- 按Godot布局加入GameInter1161聊天框178,448，五行文本区域189,492起、宽341，输入框185,575尺寸344×15。
- 修复ShowChatList将前景色与背景色反传的问题。Client完整77步重编译通过。展开聊天、滚动条、频道按钮和运行截图仍待完成。

### 展开聊天框初步接入
- 原腰带按钮位置改为参考聊天展开按钮619,472/305，切换1161/1162、五行/十四行、输入框和等级标签位置，并绘制展开区域半透明底色。
- Client完整77步重编译通过。已有历史文本仍按加入时宽度存储，展开/收起重排、频道按钮、滚动条、重新聚焦时输入框定位和运行截图仍需继续完善。

### 聊天历史重排
- 保留最近50条原始消息，按当前341/429像素宽度与实际宋体度量重新换行；替代固定五段sscanf解析，避免中文拆字及长消息仅保留五行。
- 展开/收起重排历史，聊天弹窗关闭后输入框保持对应布局。完整77步重编译通过；滚动位置保持、滚动条适配及运行视觉验收仍待完成。

### 聊天滚动条资源与行程
- 原生CScrlBar增加聊天专用绘制入口：1207帧，x619，收起top507/travel43、展开top342/travel203，保持背包旧滚动条行为。
- 空历史不再生成负行号，单行历史不会除零。Client及服务器构建通过；拖动、翻页、滚轮与截屏验收待继续。

### 原生聊天输入修正
- 滚动条滑块按下返回已消费，避免点击继续传递给地图。
- 聊天滚轮向末尾滚动限制为最后完整可见页，使用当前展开/收起行数。
- WM_MOUSEWHEEL 从高位有符号值读取方向，零增量不滚动。
- Client 编译链接通过；本阶段是输入修正，尚未完成 HUD 运行截图验收。

### 离线 HUD 预览入口与运行阻碍
- 新增 --preview=hud 与 --preview=hud-expanded，复用正式 CInterface 渲染、鼠标和滚轮处理，使用固定角色数值及20条聊天消息，不连接服务器或运行地图逻辑。
- Client 编译链接通过；启动 out/mud3-01/Mir2ExHudPreview.exe 后，通过窗口可访问性确认停在 Cannot find .\Data\interfacec2.wix file。这是正式 CreateInterface 仍保留的旧素材依赖，不能通过伪造或重命名素材掩盖。
- Computer Use 对该进程激活失败，截图只显示蓝色背景，不是有效 HUD 截图。当前 HUD 仍未完成运行验收。下一步迁移 Interfacec2 使用者（公会、队伍弹窗、通用提示），再验证两个预览布局。

### 游戏弹窗素材迁移（第一部分）
- 公会底图改用 GameInter1300，逻辑区域参考 Godot 为102,22,594,444；队伍输入弹窗底图改用1240，位置220,112。两者现有内容仍为空壳，尚未完成窗口。
- HUD 退出确认改用 CLoginNotice 的 GameInter1240/1241/1245等帧，新增可设置绘制/命中/文本统一偏移，游戏位置220,112；登录/选角默认位置保持不变。
- Client 全量77步编译链接通过。运行时发现 NPCWnd.cpp:247 也依赖 m_xInterImgEx；移除旧素材加载会使按钮初始化读空指针，预览进程已退出。因此恢复旧加载，待迁移 NPC 带编辑框对话框后再移除，不能宣称 HUD 已正常启动。
- 本阶段尚无有效新页面截图。

### HUD 实际渲染截图与旧依赖清理
- 进一步核对：NPC 的四个 ShowMessageBox 调用全部在 ClickCommandProcess 的块注释内，SetNpcEditBoxPos 也未启用。移除无人使用的 Load，删除 CInterface 的 m_xInterImgEx 字段及生命周期调用，彻底移除该启动路径的 Interfacec2 加载。NPC 输入功能仍待移植，不宣称已完成。
- 离线 HUD 预览在实际 CInterface 渲染后，将 DirectDraw RGB565 后台缓冲区保存为24位BMP（hud-collapsed.bmp/hud-expanded.bmp），只影响预览模式。解决桌面捕获只能得到蓝色背景的问题。
- 全量 Client 77步编译链接通过。新构建进程23216、30444分别以两种布局运行，窗口标题传奇3且 Responding=True；已检查两张800x600实际渲染截图：out/hud-collapsed.png、out/hud-expanded.png，且已向用户展示。
- 可见问题：旧日夜图标和帮助按钮仍需替换，展开频道按钮尚未接事件，聊天字体/换行还需对齐。截图是当前实现检查依据，不代表HUD完成。

### HUD 旧控件清理
- 移除旧GameInter130–133日夜覆盖图，避免覆盖传奇三右侧功能按钮。
- 原HELP槽位改为参考中的攻击模式按钮1205，位置150,469，命中24x15；按下帧与普通帧相同且不绘制悬停层。攻击模式业务事件尚未接入，不宣称功能已完成。
- 构建通过，新预览进程28920正常响应；已检查实际渲染截图 out/hud-icons-collapsed.png。

### 聊天文本绘制对齐
- ShowChatList 显式选用与 ReflowChat 相同的宋体9磅字体，不再依赖全局默认字体；一次取得后台DC绘制所有可见行。
- 按 Godot chat_lines：背景色0透明，非0仅填充实际文字测量区域；前景颜色保持消息值，裁剪到当前聊天区域，行距15。
- Client 编译链接通过；新预览进程14380正常响应，已检查 out/hud-font-expanded.png 实际渲染截图。整体HUD及频道交互仍未完成。

### HUD 点击音效
- Godot hud_view.cpp 加载 Sound/105.wav 作为 HUD 点击音效；原生主按钮有效释放与退出确认按钮返回有效结果时复用现有 PlayLobbyEffect 播放105.wav，沿用 m_bPlay、DirectSound 和 m_lWavMaxVol。
- 已验证运行目录样本存在且可解析，Client 构建通过。尚未做回环录音或听感验证，不宣称已验证实际扬声器输出。
- 展开聊天七按钮的控制映射已定位（16/17输入前缀、18/19本地模式、20–22服务器命令），尚未实现。

### 展开聊天按钮控件
- 原生按钮数组扩为23槽，16–22对应Godot1330/1333/1335/1337/1338/1340/1342，按参考坐标创建。绘制和鼠标处理只遍历当前可见16/23槽；初始化与释放覆盖全部槽。
- 16/17接入输入快捷动作，分别读取CmList1072和写入!，显示并聚焦原生聊天编辑框，光标放在末尾。其余五个按钮目前只有控件按压与点击音效，模式/接收状态命令仍需继续接入。
- 修改数组大小后完成Client全量编译，并完成后续增量构建；本阶段没有新的完整页面交付或交互截图验收。

### 组队与行会聊天模式
- 原生18/19按钮按Godot ChatMode切换普通/组队/行会，重复点击当前模式回普通，读取2626/2633/2635提示并用ffffff前景、80ff背景加入历史。
- 模式变化后按1334/1335、1336/1337绘制状态覆盖帧，按下帧同步当前状态。
- 游戏回车/空格打开聊天调用FocusChatMode，组队预填!!，行会预填!~；普通模式保留输入。删除旧的打开编辑框时直接调用OnMsgInputted，避免发送旧缓冲。
- Client全量及后续增量构建通过；模式运行交互和截图尚待验证，接收状态三个按钮仍未接命令。

### 聊天模式运行验证
- 新增 --preview=hud-group / --preview=hud-guild，通过实际CInterface按下/释放处理器完成选择、取消、重新选择，再保存后台渲染图；读取原生编辑框文字记录结果。
- 两份运行报告均 selected=1、toggled_off=1，组队prefix=!!，行会prefix=!~。
- 截图检查发现模式提示缺失，定位并修复新增caption路径的C++反斜杠转义错误；重新构建运行后中文提示可见。
- 已检查 out/hud-group.png 与 out/hud-guild.png；截图只含DirectDraw后台画面，子窗口编辑框前缀由GetWindowText报告验证，不宣称截图包含编辑框。

### 消息接收按钮命令
- 原生20/21/22按钮读取CmList1070/1071/1073并直接调用SendSay，匹配Godot send_chat_command，不经过输入框本地命令处理。
- 依据参考ChatReceptionButtons，点击仅从外部接收状态推导1338/1340/1342覆盖层；不在点击时切换rejected状态，也不伪造服务器确认。增加三个接收状态槽，初始化false；服务器options更新接入仍待协议阶段完成。
- Client全量77步编译链接通过，尚未验证服务端接收或状态回包；GUI整体仍未完成。

### 退出确认框中文文案与截图
- 移除HUD退出按钮对Ats.dll字符串12的加载，改用CmList2636，匹配Godot请求109的退出提示。
- CLoginNotice增加确认文本宽度选择；游戏退出框在220,112，正文采用相对23,84,308,95；登录/选角保留原紧凑确认布局。
- 增加 --preview=hud-exit，通过实际退出按钮按下/释放打开确认框。Client全量77步构建通过，已检查 out/hud-exit.png，中文提示及YES/NO位置可见；退出限制、确认退出及注销协议仍需单独联调。

### 退出确认框模态输入验证
- 确认框活动期间拦截底层鼠标移动、滚轮、双击、按键释放；GameProc提前转交键盘并拦截右键和系统快捷键，防止继续执行地图/聊天快捷操作。Esc取消确认框。
- 离线退出预览通过实际处理器检查滚轮保持聊天位置、NO取消、Esc取消、重新打开。报告hud-exit-check.txt四项均为1；构建通过。
- 键盘左右选择/回车确认和实际退出限制尚待对齐验证。

### 确认框键盘规则校正
- 按Godot group_dialog_input：默认YES，Tab在YES/NO切换，Enter/Space返回选中结果；重复按键不触发。Esc仅拦截，不取消，纠正上一阶段误加行为。
- 提取FinishNotice共用确认结果处理，鼠标播放点击声，键盘不播放（与参考区分）。
- Client全量77步编译通过；退出预览报告wheel_blocked=1、cancelled=1、escape_blocked=1、keyboard_cancelled=1、reopened=1，验证Tab+Enter可取消。真实YES退出仍需退出限制联调。

### 组队页面显示迁移
- 组队逻辑区域272,123,258,244，GameInter1360居中绘制；关闭1221与允许组队1370/1371按Godot位置显示。
- 名称改为角色名，宋体9磅；成员按参考两列顺序、20px行距显示，允许状态读取1603/1604。
- 新增 --preview=group-panel，注入六名示例成员，复用实际窗口列表绘制；构建通过，已检查 out/group-panel.png。
- 旧按钮命中和创建/邀请/移除仍待迁移，尤其旧代码硬编码mandul不可视为完整功能；下一步实现输入对话框并替换这些测试动作。

### 组队按钮与成员输入对话框
- 替换旧命中表：1221关闭、1362创建、1365邀请、1368移除、1370允许，使用参考坐标和帧尺寸；按下/释放同按钮才执行并播放105.wav。移除硬编码mandul。
- 三种成员动作打开GameInter1240原生确认框，正文读取1605/1606，复用ChatEdit子窗口，位置244,340、309x15、90字节限制；YES读取实际输入调用对应已有发送函数，NO隐藏并恢复聊天编辑框位置，回车通过OnMsgInputted处理。
- 增加三个离线预览，实际按钮触发后截图out/group-create.png、group-invite.png、group-remove.png，均已检查；全量Client及后续构建通过。截图不包含原生编辑子窗口，编辑框外观、取消恢复及服务端处理仍待进一步验证。

### 原生组队输入框字体与取消验证
- ChatEdit创建和缩放重建字体均读取CmList1074，逻辑12px/400字重，默认EM_LIMITTEXT由95改为90，与参考ChatInputSpec一致。
- 三种成员对话框运行验证均读取到PreviewMember、limit=90、cancel_restored=1；当前150%显示缩放下实际框464x23，字体高度-18，对应逻辑309x15与12px。
- 检查通过实际HWND/WM_GETFONT/GetWindowText完成，取消后确认框关闭、编辑框隐藏且内容清空；未发送创建/邀请/移除请求。构建通过。输入框透明背景和完整窗口截图仍需进一步核对。

### 预览包含原生编辑框
- 预览截图在DirectDraw后台图像上，按实际窗口位置缩放合入WM_PRINT返回的真实原生EDIT绘制，不再遗漏独立编辑窗口；截图不含光标闪烁。
- 完整截图暴露旧WM_CTLCOLOREDIT画刷覆盖底图，改为黑底白字、BLACK_BRUSH，消除旧背景色；与Godot透明背景的细节一致性仍待后续核对。
- 构建通过，已检查 out/group-create-edit.png，原生输入框与底图槽位对齐。

### 公会页面三页签迁移
- 原空壳公会窗口改为GameInter1300居中底图，九按钮按GuildLayout坐标与按下帧绘制；真实按下/释放支持关闭和公告/成员/行会聊天三个页签切换及点击音效。
- 标题使用CmList4010/4009/4008、96c8ff色；正文按参考行距与标题色，聊天页绿色及阴影，1225滚动滑块，滚轮接入当前公会页。
- Client全量77步构建通过，已检查out/guild-overview.png、guild-members.png、guild-notice.png（三者分别公告、成员目录、行会聊天），示例内容仅用于离线预览。
- 公会服务端数据、管理动作、拖动滚动条、权限提示和工具提示尚未完成，不能宣称公会功能完成。

### 公会滚动条交互
- 按GuildScroll移植1225滑块：起点相对557,60，行程258，10ms轨道点击节流，翻页比例19/count，拖动以count-1映射。
- 拖动时通过主窗口捕获鼠标，Interface优先转发窗口外移动和释放；释放解除捕获。
- 运行40行数据验证报告guild-scroll-check.txt：end=1、released=1、page=1，分别验证末行39、窗口外释放、轨道一步到18。全量Client及后续构建通过。

### 公会管理权限提示
- 按GuildLayout::denial，非管理者点击4–8号管理按钮显示CmList4000原文，使用GameInter1240与1243单确认按钮。公会管理身份初始化false，仍待服务器数据提供真实值。
- 新增通用ShowNotice路径，结果处理关闭提示框并区分鼠标音效与键盘确认。
- Client全量77步编译通过；--preview=guild-denied通过实际管理按钮打开提示，已检查out/guild-denied.png，正文和单按钮位置正确。管理者分支功能尚未完成。

### 公会成员移除与解除同盟对话框
- 管理者按钮5/8分别打开原生输入确认框，提示为公会名+CmList4001及CmList4002；复用成员输入框取消恢复逻辑。
- YES移除发送CM_GUILDDELMEMBER后请求CM_GUILDMEMBERLIST；解除同盟发送CmList4016+名称的聊天命令后请求GuildHome，按Godot顺序。服务端处理尚未验证。
- Client全量77步构建通过；离线管理者预览经真实按钮打开两框，已检查out/guild-remove.png和out/guild-break-ally.png，含真实原生输入框绘制。预览未确认或发送管理请求。

### 设置页面布局迁移
- 设置页改为GameInter1290，位置145,30尺寸510x468；中文标题/标签读取2100–2130对应文案，开关1292/1295、悬停1293/1294、滑块1291、攻击切换1296/1297、关闭1221/1222。
- 音乐/音效开关接入现有声音系统。其余设置动作、滑块拖动、配置持久化和服务端状态尚未完成。
- 截图发现设置页底部被HUD覆盖，调整RenderInterface为HUD后绘制窗口、窗口后绘制确认框，已重新检查out/options-panel.png。
- Client全量77步及后续构建通过，新增--preview=options-panel。

### 设置页音量滑块与声音参数
- 按Godot AudioOptions移植音乐/音效滑块拖动、严格轨道边界与9像素步进；主窗口捕获鼠标，Interface转发窗口外移动及释放。
- 新增独立音量入口，映射40*level，分别更新音乐和音效音量；现有界面点击/开场音效同步更新。参考右端可产生level=2，DirectSound不支持正增益，因此音频输出参数钳制到0dB，控件保留参考位置。
- 全量Client及后续全项目构建完成。原生预览options-audio-check.txt为minimum=1 maximum_released=1 independent=1，已检查out/options-audio-sliders.png。验证涉及控件交互与声音系统参数，未验证实际音频输出，未覆盖所有正在播放的场景音效。
- 配置持久化、其余设置动作和服务器状态同步仍待完成。

### 声道反转开关
- 设置页音响左右频道按钮接入CMirSound：反转2D声像及3D横向坐标，作用于后续播放/更新的定位音效。滑块按下补充参考105.wav音效。
- Client全量77步构建通过。原生预览报告reverse_enabled=1、reverse_restored=1，已有三个音量检查仍通过；已检查out/options-reverse-pan.png。该验证证明按钮与声音状态联动，未完成实际左右声道输出监听或录制。

### 音频设置保存与恢复
- 原生设置页采用Config.ini的[Options]，沿用BGM、EffectSound、ReversePan、BGMLevel、EffectSoundLevel五项；启动时声音系统初始化后加载，窗口重建时再次加载，点击/拖动时保存。音量越界时按Godot规则恢复0。
- 所有--preview模式默认禁用玩家配置读写；音频配置验证只使用preview-audio-options.ini。保存-清除内存状态-重新加载后，验证三开关、两音量及声音系统参数一致，settings_restored=1，既有五项交互检查仍通过。已检查out/options-audio-config.png。
- Client全量77步及最终全项目构建通过。实际声音输出、正在播放声音的开关即时停止行为及其余系统设置仍需继续完善。

### 音频开关即时停止
- 关闭音乐停止已加载的DirectShow播放并取消淡入淡出定时器；重新开启沿用原循环标志恢复播放。关闭音效停止角色、界面、开场、公共与已启用的环境音缓冲，环境音播放入口同时检查音效开关，防止定时器重新播放。
- 全量Client及最终全项目构建通过。预览加载实际Sound/SelChr.mp3，播放接口与状态验证music_started=1、music_stopped=1、music_resumed=1，原有六项音量/配置检查仍通过。该验证未录制扬声器输出；地图音乐切换和场景音效仍待联调。

### 背包页面布局首轮迁移
- 背包采用GameInter1220，逻辑518,0/282x466，底图居中；标题CmList1701、负重2501采用宋体9pt及参考颜色/阴影；关闭1221/1222位于242,423，滑块1225按参考初始位置绘制。
- 可见格由5x5改6x8，起点17,60，38像素格；旧腰带内容不再绘制/命中于背包页面，HUD仍保留原快捷物品绘制。关闭需要匹配按下和释放并播放105.wav。
- Client全量77步构建通过，原生预览inventory-layout-check.txt验证grid=1、release_without_press=1、close_clicked=1（已修正格内测试点为762，并清理本任务旧预览进程后重新运行确认）；已检查out/inventory-panel.png。
- 此阶段不是完整背包：物品数组仍为旧60项，超过现有容量的格子不命中；参考100行的锚点布局、多格物品、滚动拖动/轨道输入、金币与商店操作、布局文件迁移和服务器物品数据尚未完成。

### 背包滚动条交互迁移
- 移除背包旧CScrlBar实例，移植InventoryScroll的1225命中、254行程、绘制除数91/输入乘数92、8行轨道比例步进与10ms节流。拖动捕获主窗口鼠标，Interface优先转发窗口外移动和释放。
- Client全量77步构建通过；重新运行原生预览，inventory-layout-check.txt五项均为1：格子命中、无按下释放、关闭按钮、滚动末行92、窗口外捕获释放。轨道翻页已移植但尚未独立运行验证。
- 多格物品、100行位置存储、金币与物品服务器数据仍未完成，当前超过旧60项存储的格子仍返回未命中。

### 背包真实物品绘制与裁剪
- 核对Godot InventoryGrid：46个物品记录与6x100布局格是独立结构，当前旧60记录按格索引的实现仍需迁移，不能直接扩容替代。
- CItem::DrawItem增加可选裁剪矩形，使用现有压缩图像源区域绘制；背包以17,60/228x304可见区域裁剪图标，遍历物品计算相对滚动位置，允许边缘部分可见图像参与绘制。其他调用维持原绘制方式。
- Client全量77步编译通过；原生预览加载Inventory.wil并加入6个真实图标，icon_samples=6，原有五项交互检查全部为1；已检查out/inventory-icons.png。此次截图验证资源与绘制路径，尚未独立覆盖大图标边缘裁剪、多格占位及碰撞。

### 背包独立锚点与多格占位
- 新增原生CBagLayout，将记录索引与6x100位置格分离，按图标宽高向上取整38px建立占位，支持指定落点、碰撞后首个可容纳位置、移除释放占位。仍保留旧60条记录容量，46条参考容量及布局文件格式迁移待下一阶段。
- Add/删除/拾取/放置/交换接入布局，点击占用区域解析记录；物品绘制居中于完整占位并裁剪到背包，提示框位置跟随锚点。放置失败保留手持物品，交换失败恢复原占位。旧itm恢复后重建布局，但当前不会保存新锚点，重启位置保持尚未完成。
- Client全量77步及最终全项目构建通过。真实Inventory图标75验证multi_hit=1、picked=1、moved=1：滚动20行后放置，再移回首页（不是第20页）。布局碰撞/越界/无空间及移除检查collision=1、freed=1；原五项UI检查仍通过。已检查out/inventory-multicell.png。
- 多格放置预览、堆叠数量、完整服务器记录适配与布局持久化仍待实现。

### 背包itms位置保存与恢复
- 替换旧itm序列化路径为Data/<角色名>.itms：46个3120字节记录区、600个int16占位、6个3112字节快捷项，共163392字节；只写当前旧记录能提供的姓名、MakeIndex、耐久身份及尺寸/占位字段，其余缓存字段归零，不声称完整100字节MIR3服务器记录已适配。旧itm文件保留，未迁移旧文件。
- 读取按身份匹配当前物品，校验占位范围、单锚点、完整矩形及尺寸，不覆盖当前物品属性；未匹配物品按首个可容纳位置放置。记录顺序与磁盘保存顺序无关。SM_BAGITEMS接收完成后调用恢复，退出沿用保存入口；真实服务端重登录时序尚未验证。
- Client全量77步构建通过。独立preview-item-layout.itms保存7个物品后反序重建，再读取，layout_restored=1、server_records_preserved=1；原有UI/多格检查全部通过，已检查out/inventory-restored.png。
- 当前内存仍旧60条容量，文件最多46条；超过46条会拒绝保存，不会静默截断。容量/完整记录与快捷栏语义仍待一起适配。

### 背包放置占位预览
- 手持物品进入背包时绘制完整多格占位，按参考绿色RGB(50,100,50)/红色RGB(100,50,50)、alpha=1的预乘混合，并裁剪至背包可见区域；预览和实际放置共用Placement坐标计算。
- 在绘制窗口列表前确定鼠标命中的最上层窗口，模态提示/其它窗口遮挡时不绘制占位；手持图标调整到窗口之后、确认框之前绘制，修复被背包底图遮挡。手持时隐藏旧物品提示文字。
- Client全量77步及最终全项目构建通过，原生预览placement_green=1、placement_red=1，既有UI/多格/保存恢复检查全部通过；已检查out/inventory-place.png和out/inventory-conflict.png。新增--preview=inventory-place/inventory-conflict。

### 背包金币显示与变化反馈
- 金币读取现有m_nGlod，按参考72,402/77x17、CmList1074字体10pt粗体、COLORREF 0x64c8f8居中显示无符号余额。
- SM_GOLDCHANGE转入UpdateGold，按32位回绕差值显示正增量CmList1471提示（白字/33023背景），并播放音效106；已检查SoundList.wwl的106映射为106.wav。初始角色金币消息继续使用原初始化入口。
- Client全量77步构建通过；离线UpdateGold验证gold_unsigned=1、gold_updated=1，既有背包检查全部通过；已检查out/inventory-gold.png，显示123456789测试金额及回绕增量提示。未验证真实服务器金币消息和实际音频输出。

### 角色状态合并页面首轮迁移
- 普通装备/状态视图统一使用GameInter1280、0,0/328x466；基础属性按StatusLayout坐标、宋体9pt及参考色绘制，人物名按参考字体粗体，关闭1221/1222位于289,14并要求匹配按下/释放，点击105音效。技能旧入口暂保留，未声明已迁移。
- 现有10个装备槽按EquipmentState前10槽更新命中位置；纸娃娃装备相对82,199绘制，普通饰品按格中心绘制。人物底图复用Interface已加载的ProgUse资源，修正预览资源实例未加载导致的缺失。
- Client全量77步与后续全项目构建通过；--preview=status-panel验证release_without_press=1、close_clicked=1、slots=1；已检查out/status-panel.png，人物底图和基础属性可见。
- 扩展装备槽10/11、元素数值、头衔/公会身份、悬停提示、装备高亮和完整穿脱流程仍待完成；当前身体帧沿用旧性别字段，完整appearance映射仍待协议适配。

### 状态页元素数值与悬停提示
- 接入3x7原始元素字节，攻击行无符号、强弱行有符号，只有正值显示图标及+N/xN；位置、1510/1520系列图标与5161起元素提示按StatusLayout。实际服务器元素消息尚未接入，此阶段数据由离线预览填充。
- 增加等级/经验/HP/MP/负重等9行提示，采用参考文本测量、背景预乘混合和边框；鼠标命中窗口在绘制列表前统一计算，模态/上层窗口遮挡时不显示。
- Client全量77步编译通过；--preview=status-elements与status-tooltip检查signed_elements=1，原三个状态交互检查仍通过；已检查out/status-elements.png和out/status-tooltip.png，分别展示火元素和HP提示。

### 状态页公会身份与徽记
- 按参考身份布局：公会名存在时角色名上移16像素，公会/职位分别82,29与82,43；徽记1298位于181,67，名称200,66/105x15，沿用参考文字色及字体。公会名使用当前英雄现有字段；职位与徽记新增展示字段，实际服务器赋值尚未接入。
- Client全量77步编译通过，--preview=status-identity展示中文测试角色、公会、掌门和荣誉勇士；已检查out/status-identity.png，四项既有状态检查均为1。

### 状态页扩展装备槽与拖放高亮
- 原生装备数组扩展为12槽，新增10/11槽参考坐标；物品分类补齐54/55/56武器、57项链、53与60扩展类，未知类不再默认当衣服。符类25允许槽5或9，但仅槽9高亮，与参考接受/显示规则保持区分。
- 按EquipmentState选择1235/1236高亮帧，位置使用槽左上偏移；复用压缩纹理绘制，新增源alpha=20的ONE/INVSRCALPHA混合参数，原调用默认0保持原加法混合。窗口遮挡时不显示高亮，手持图标保持最上层。
- Client全量77步编译通过；--preview=status-equipment验证equipment_rules=1（含符类区别、扩展槽命中），既有四项状态检查仍通过；已检查out/status-equipment.png。预览未发出穿脱请求，完整服务器装备列表与穿脱响应仍需适配。

### 技能页迁移前快捷键解析修正
- 已核对Godot SkillPanel与draw_skills：独立360x488窗口、八分类技能树、1620底图、1622起树帧、滚动范围0..190、技能图标2*ID-2。当前原生技能页面仍是旧状态窗口内列表，尚未完成页面迁移。
- 修复GetMagicByKey未匹配时返回最后一条技能的问题；现在仅返回实际绑定记录，未绑定、0键与空列表返回NULL，现有Actor调用据此跳过施法。
- 原生离线预览直接调用生产方法，bound_first_last=1、unbound_and_zero=1、empty_list=1；既有五项状态页检查均通过。Client及六个服务端目标构建通过。本阶段没有完成新页面，不将旧状态页截图作为技能页成果。

### 原生技能树页面首轮迁移
- 从Godot SkillLayout迁移八类技能分类、15槽树坐标到原生SkillLayout.h；技能页采用1620底图、1622起技能树、1640起页签、1672滚动块，逻辑348,0/360x488。原生StatusWnd绘制替代旧列表，未引入Godot运行时。
- 技能图标使用MIcon的2*ID-2帧；当前旧协议F1..F8字符绑定映射至1660起标记。等级使用参考8pt字体/0x64fafa，名称和Level/Exp使用宋体9pt与参考文字布局；支持选中边框、15项显示限制及无树槽回退位置。
- 页签与关闭要求匹配按下/释放并播放105.wav；滚轮0..190、滚动块189绘制分母/190输入范围与参考一致，拖动和按钮释放路由覆盖窗口外。状态/技能各自记住位置，打开时选择有技能的首个分类。
- 截图揭示旧RECT源区绘图接口会叠加源偏移；技能树改用DrawSelectionFrame新增屏幕裁剪参数，实现正确滚动和边缘裁剪。该发现亦意味着此前背包旧源区裁剪路径需要后续复查，不能视为所有多格边缘场景已验证。
- Client全量77步及最终全部目标构建通过；原生skills-panel/skills-scrolled预览13项检查全1，覆盖选中、页签、上下限、拖动捕获/外部释放、关闭、初始位置和两类位置恢复。状态页5项及快捷键3项回归通过。已检查out/skills-panel.png、out/skills-scrolled.png；测试技能数据为离线样例，未验证服务器技能消息或实际音频输出。
- 待办：技能与状态仍共用窗口管理ID，尚不能同时打开；需拆独立技能窗口，补齐参考12个快捷键及快捷键面板、提示/协议适配与实际施法。旧技能列表初始化字段待拆分时移除。

### 技能页拆分独立原生窗口
- 新建CSkillWnd与窗口ID14，独立拥有技能记录、分类、滚动、选中和按钮状态；从StatusWnd移除技能模式、旧列表初始化、类型切换及失效按钮。两页可同时打开并各自保存当前位置。
- 接通Interface绘制顺序、命中、聚焦、拖动、滚轮、按键、鼠标捕获与关闭；HUD技能按钮/F11切换技能，角色按钮/F10切换状态。点击技能页签置顶但不启动拖动。技能页关闭和窗口外释放不影响状态页。
- 旧技能消息和Actor快捷键查找改引用独立技能窗口，技能数组生命周期统一delete[]；保持旧CLIENTMAGICRCD线格式，尚未完成MIR3完整技能协议适配，不声称真实服务端施法验证。
- Client全量78步及最终全部构建目标通过。status-and-skills与skills-scrolled原生预览21项全部为1，新增验证双窗命中、独立开关/关闭、拖动位置恢复、页签置顶不拖动；状态页5项和快捷键3项回归通过。已检查out/status-and-skills.png与更新后的out/skills-scrolled.png。
- 待办：参考12快捷键及快捷键选择面板、技能提示、完整服务器技能消息和施法；另外前阶段发现的背包源区域裁剪偏移仍待修正。

### 背包多格物品边缘裁剪修正
- CItem裁剪路径改为复用带屏幕裁剪的DrawSelectionFrame，保留物品原始屏幕位置，不再把源区域偏移重复加到目标坐标。普通未裁剪调用保持原路径。
- 新增inventory-clip-top/bottom原生预览：真实Inventory图标75（68x66）分别位于首行滚动一行后、锚点44跨越底边。截图out/inventory-clip-top.png、out/inventory-clip-bottom.png已检查，内容局限于可见网格。
- 在800x600实际DirectDraw缓冲上比较CItem完整绘制与四种裁剪绘制：裁剪内逐像素匹配完整图像，裁剪外匹配原背景，并要求可见非背景像素。修复前left=0/top=0，修复后left/right/top/bottom全1；原背包布局、拾取移动、保存恢复、放置高亮和金币检查均通过，全部构建目标通过。此检查覆盖所选真实图标，不等同于全资源扫描。

### 技能12槽快捷键选择面板
- 按Godot skill_menu_input/draw_skill_key_menu实现原生双击弹出160x120、4x3面板，40px槽、1660..1671标记、RGB(100,100,50)/alpha150预乘背景，显示各槽已绑定技能图标。保留参考前10个未滚动槽命中、以当前选中槽确定目标及原有位置计算规则。
- 选键后保持面板，外部按下或关闭技能窗口清除面板；窗口命中覆盖弹出区域，面板在技能窗口外并覆盖HUD时优先处理点击，同时遵守上层窗口命中。
- 统一BindMagic冲突清除与赋值，扩展显示/选择编码1..9/A/B/C（12槽）；仍使用旧CM_MAGICKEYCHANGE发送入口。物理F9..F12目前仍有窗口快捷键冲突，12键实际施法和完整协议尚未完成。快捷键面板在参考中未播放按钮音，不额外增加；页签/关闭沿用105.wav。
- Client全量78步及最终全部目标构建通过，skill-key-menu原生预览10项全1：打开、禁止拖动、F12冲突、保持打开、F10重绑、编码、外部关闭、整窗关闭、窗口外命中、覆盖HUD点击；既有技能窗口21项仍全1。已检查out/skill-key-menu.png。绑定检查为离线内存/调用路径，未证明服务端收包或实际施法。

### 原生技能说明提示框
- 提取现有CmList.dat解码为LoadLegacyText共用函数，保持长度/校验和/XOR算法；技能说明读取同格式Magic.exp，按#编号分段、跳过注释，解码仅在内存。运行目录补入现有宝藏传奇3/Magic.exp用于验证。
- 按Godot wrap_skill_description移植165px宋体9pt换行、原字节/行上限及标题标识，缓存各技能行；提示背景RGB(100,100,50)/alpha150，标题参考10pt粗体与四色阴影，正文白色、熟练度红色。悬停位置、右边位置分支及最大等级文字沿用参考。
- 在技能窗口绘制层中接入提示，鼠标未命中顶层技能窗口、滚动捕获或快捷键面板打开时隐藏；按树槽查说明，未学习技能也可展示文件说明。真实Magic.exp的ID1为火球术，修正原离线样例名称以免与说明矛盾；完整技能ID/资源/协议跨版本一致性仍待服务器适配时核对。
- Client全量78步及最终全部目标构建通过；skill-description原生预览得到13行，悬停、165px换行、标题、遮挡标志、滚动/菜单隐藏、CmList复用读取均通过，既有技能窗口21项仍通过。已检查out/skill-description.png。未声称所有技能、屏幕边缘或真实服务器数据都已验证。

### 顶部12槽技能快捷栏
- 替换旧ShowMagicKey八个70起图标，按Godot draw_skill_shortcuts绘制12槽：x=40*(index+index/4)，F4/F8后留40px空隙；绑定图标使用MIcon的999+(ID&255)，位置y2/alpha200；1450起标记位于x-13,y-11，RGB与alpha均150。
- 原生压缩纹理绘制新增Alpha混合及灰度tint参数，保持其他混合模式不变；对应参考画布shader对COLOR.rgb乘alpha后的混合。沿用HUD技能快捷栏按钮，接入B键显示开关并忽略长按重复消息。
- Client全量78步及最终全部目标构建通过。skill-shortcuts离线预览shown/repeat_ignored/hidden全1，12图标与12标记帧均存在，既有技能窗口21项仍通过。已检查out/skill-shortcuts.png。
- 这里只完成快捷栏显示和开关；物理F9..F12冲突、独立低字节绑定表的服务器消息更新及实际施法仍未验证/完成。
