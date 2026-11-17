
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

### 原生任务页面首轮迁移
- 用1420/alpha220卷轴底图替换旧任务窗口，逻辑0,0/340x440；1422/1425展开与关闭按钮位于267,75/107，1225滚动块位于272,160，行程182。按钮匹配按下/释放后播放105.wav。
- 新增原生任务/子任务展示记录，按参考选中、完成颜色与宋体9pt绘制19行；展开后映射子行，选中父/子任务显示三行说明。说明按170px保留越界字符换行、165px显示裁剪，说明滚轮范围0..3，列表滚轮独立。
- 接通窗口层级、拖动、滚动捕获及窗口外释放；D键和现有HUD任务按钮切换窗口，重复D消息不反复切换。空列表正常显示，窗口外滚轮不修改列表。
- Client全量78步及最终全部构建目标通过；quest-empty验证基础开关/关闭/拖动，quest-list填入22条离线任务与2条子任务，14项交互检查全1。已检查out/quest-empty.png和out/quest-list.png。
- 明确未完成：任务列表/子任务/说明服务器请求与响应尚未接入，截图内容为中文离线样例。参考draw_quest的19行列表会与347起说明区重叠，当前按该布局保留，后续需与MIR3.EXE实机再次核对，不能把页面声称为最终像素验收。

### 原生坐骑页面首轮迁移
- 按Godot MountPanel迁移0,0/276x322面板，1400底图alpha220，1221关闭与1402/1405/1408/1410四个操作按钮采用参考坐标。替换原旧窗口630关闭按钮路径；S键及现有HUD入口切换，拖动/置顶/窗口外释放接入原生管理，按钮匹配释放播放105.wav。
- CmList命令映射按shape状态：步行按钮1=4106、骑乘按钮2=4105，按钮3/4=4104/4105；调用现有SendSay。shape服务器更新、骑乘后的输入延时和实际命令响应尚未适配验证，预览仅检查映射，不发出操作命令。
- 接入1052/4100..4103悬停提示，宋体9pt、黑框黄底黑字，复现参考逐帧高度展开和右边界位置规则；按下或被遮挡时隐藏。
- Client全量78步及最终全部构建目标通过。mount-panel/mount-hint预览13项检查全1，覆盖S开关/重复消息、按钮命中、无按下释放、状态命令映射、提示与遮挡、外部释放、窗口拖动/关闭。已检查out/mount-panel.png和out/mount-hint.png。坐骑列表/属性与服务端骑乘仍未完成。

### 原生交易页面首轮迁移
- 交易窗口使用1390底图、0,0/494x330；双方5x5可见网格分别14,56和264,56，按多格尺寸居中绘制真实Inventory图标并裁剪，数量文字和千分位金币按Godot draw_trade坐标/宋体9pt/颜色显示。每侧保留200锚点展示槽，尚未实现交易物品提交逻辑或12条协议记录限制。
- 独立滚动条1225位于206/457,58，行程158，首行0..33，轨道点击6行；捕获与外部释放接入Interface，仅顶部56px区域可拖动窗口。
- 确认按钮1392/1396/1394对应普通/按下/已确认；无配对按下、金币/提交/退回待处理时不确认。正常确认沿用SendQueryMsg发1030（参考CM_TRADE_CONFIRM），未增加参考不存在的按钮音效。服务器交易请求、数据更新、退回、取消和结束仍未接入。
- Client全量78步及最终全部目标构建通过。trade-empty/list/confirmed三个离线预览10项检查均通过，包含左右独立滚动、金额、待处理限制、外部释放及拖动；已检查out/trade-empty.png、out/trade-list.png、out/trade-confirmed.png。预览没有连接服务器，确认只证明本地状态/发送调用，不证明真实交易成功。
- 后续：交易金币/数量对话框、物品添加/退回、服务器状态机、确认提示及完整交易取消路径。

### 原生交易金币输入对话框
- 交易窗口本方金币区34,270/122x34接入现有原生输入对话框，采用1240底图、CmList 1304提示、宋体及既有输入框坐标；复用YES/NO匹配点击与105.wav音效、回车提交。按Godot参考保留Esc吞掉行为，NO取消隐藏输入框并恢复聊天框位置。
- SubmitGold严格接受非零十进制金额，拒绝符号、空格、尾随字符和超出余额；当前工具链不支持from_chars，使用逐字符检查加strtoull，溢出结果超过32位余额而被拒绝。确认/已有金币/等待响应时禁止再提交。
- 金币通过1029（CM_TRADE_GOLD）的recognition字段提交，仅置待响应状态，不提前更改背包或交易余额。服务器确认/失败响应及待处理清理仍未接入，不能声称真实交易成功。
- Client及全部目标最终构建通过；trade-gold预览8项全1，覆盖打开、Esc阻断、NO取消、非法金额、全额、重复提交阻断、回车提交和模态滚轮；交易基础10项仍全1。已检查实际DirectDraw及原生EDIT合成截图out/trade-gold.png。音效调用已复用，实际听感未验证。

### 交易物品退回的原生交互
- 交易展示记录补入makeIndex/name，按本方5列、多格尺寸、滚动首行定位选中物品；无实例编号/对方栏不拾取，点击本方空格清除选择。被选中源图使用已有alpha128路径，鼠标图标在窗口/对话框/快捷栏之后按真实尺寸居中绘制。
- 放到背包网格后调用SendItemIndex发送1027（CM_TRADE_REMOVE_ITEM），置returnPending并保留交易物品/鼠标图，等待响应；重复请求与待处理期间换选被阻止。交易持有期间阻止装备拾取、背包双击/快捷使用与背包空格拖动，避免同时持有两件不同来源物品。
- 修正确认按钮外部释放或窗口标题拖动释放误触金币区/物品区的问题，头部捕获释放统一终止窗口移动；关闭窗口清除按下/捕获状态。
- Client清理后完整构建及最终全部目标通过。trade-return离线预览11项全部为1，覆盖多格/滚动命中、选择/清除、对方栏、缺编号、确认阻断、捕获释放、背包请求和重复/待处理状态；trade-gold 8项、trade-list 10项及inventory-panel回归通过。已查看out/trade-return.png，展示半透明原图、跟随鼠标物品及背包。
- 尚未完成：服务器交易物品记录导入与退回成功/失败响应、背包插入、回复后物品音效、快捷栏取消持有；当前仅验证原生UI及发送调用，未验证服务器收到请求。数量输入与物品加入仍待后续迁移。

### 原生六格快捷物品栏首轮迁移
- 新增CQuickWnd及窗口ID15，采用Godot的73,413/280x48独立窗口、1210底图、1211/1212关闭帧，六格起点17,6、间隔42、命中38x38。真实Inventory图标按尺寸居中，堆叠数量按参考log10偏移/宋体9pt白色绘制。
- 接通原生窗口绘制、置顶、命中、拖动、捕获释放和Z开关，忽略重复按键；格子和关闭按钮阻止窗口拖动，背景允许拖动，关闭需匹配按下释放。参考快捷栏关闭没有按钮音，本实现不额外增加。
- 交易来源物品在快捷格释放时仅取消持有，保持交易槽且不绑定快捷栏；退回请求等待中禁止取消。新增CMirSound的物品类型到108/111..118映射，24/26按CmList2641/2642名称匹配，31按原记录字节31的106阈值选择；通过SoundList.wwl解析文件并复用现有音量/禁音控制播放。
- Client清理后79步构建和最终全部目标通过。quick-empty/items两个预览13项检查全1，含开关/重复、六格边界/空隙、关闭/外部释放、背景拖动、格子禁止拖动、交易持有取消/等待、完整音效分类映射。九个映射WAV均存在且可解码（44100Hz）；未验证实际扬声器听感。已检查out/quick-empty.png、out/quick-items.png。
- 尚未完成：普通背包物品的快捷引用绑定/交换/取回、双击/数字键使用、数量更新及持久化。slots为原生显示记录，当前截图使用离线样例，不声称快捷栏完整业务已完成；旧m_stBeltItem协议记录还需迁移到参考的引用语义。

### 快捷栏与背包的引用绑定交互
- CQuickWnd槽记录补入旧客户端CItem，heldSlot/heldReference区分快捷引用与实物。按Godot QuickItems::release实现药品/卷轴（std_mode 0/3）绑定、取回引用、移动和交换；绑定前先把背包来源实物插回背包，插入失败时保留原状态。
- 同名目标返回1069、其他格已有同名返回1067、新绑定返回1068，使用CmList名称/格号替换并以参考11861940文字色显示。成功取回、移动、交换/绑定播放对应物品音效；失败不播放。
- 快捷引用拖回背包只清除引用和鼠标持有，不插入实物；禁止快捷引用走旧背包双击/数字键使用路径，地面丢弃条件排除快捷引用，以免把副本当实物发送。装备来源、等待回复、非允许类型不绑定。
- Client清理后79步及最终全部目标构建通过。quick-binding 12项全1，验证绑定后实物数量不变、引用取回/移动、同名拒绝、交换、拖回背包删除引用、等待/装备/类型限制、真实鼠标路由、满背包回滚；quick-items 13项、trade-return 11项和inventory-panel检查均通过。已查看out/quick-binding.png，展示背包两件实物与其快捷引用及中文绑定提示。
- 旧CLIENTITEMRCD目前没有完整MIR3堆叠记录，FromItem暂不推断堆叠，数量保持1、不绘制数量标签。后续仍需完整记录导入、快捷使用/服务器回复、删除与数量同步、.itms保存恢复迁移；本阶段不声称快捷使用已可用。

### 快捷引用接入.itms布局持久化
- SaveLayoutFile的六个快捷记录改读取CQuickWnd.slots，保持46*3120+1200+6*3112（163392字节）原格式及每个快捷记录+8的身份字段位置；已拿到鼠标的快捷引用不在slots中，不额外保存。
- LoadLayoutFile按Godot ItemLayout规则以名称匹配当前背包列表，取最后一个匹配记录，通过FromItem恢复新原生快捷槽。旧文件中的编号/耐久/属性不覆盖当前记录；无匹配和空标记保持现有引用状态，沿用参考规则。旧m_stBeltItem不再作为新快捷栏的持久化数据源。
- 接入现有GameProc OnDestroy保存、SM_BAGITEMS收到列表后CheckItemPos恢复入口；本轮只验证文件/原生状态，并未验证联网退出重登的完整时序。
- 全部目标构建通过；quick-persistence七项全1（文件大小与槽标记、恢复、最新记录/最后同名匹配、损坏文件不变、缺失物品不生成、鼠标引用不保存），quick-binding十二项及inventory-panel回归通过。已检查out/quick-restored.png，背包包含两个同名实例以验证最后匹配规则。
- 后续仍需完整MIR3物品/堆叠协议、快捷使用与服务器更新、实际退出重登验收；现有旧背包容量60而.itms仅46条的限制仍在，超过46条保存会返回失败。

### 原生行会公告与职位编辑对话框
- 新增CGuildEditor，使用CLIENT原生多行EDIT而非Godot控件，逻辑107,110/584x252、1326底图、1221/1222关闭、1327/1328保存；标题1903/1900使用CmList1074的9pt字体，说明1904或1901/1902使用宋体9pt与参考坐标/颜色。编辑区域46,54/468x170，原生字体12px随显示比例缩放；背景采样底图生成画刷，保留木纹。
- 行会按钮4/7在管理员权限下打开公告/职位编辑，成员页为空时显示4017。新增m_editable[2]保存原始可编辑行，避免误用展示用排版文本；该数据尚未接入服务器。原生编辑器模态拦截底层点击/滚轮/快捷键/右键，支持背景拖动和编辑框同步定位；关闭/保存匹配释放播放105.wav。
- 通过EN_CHANGE执行参考4000 ANSI字节限制，保留未修改前后缀且不截断DBCS字符；Enter保留CRLF。提交截取前3999字节、去LF并按参考转换空行为“空格+CR”，非空时分别发送1040/1041。保存只关闭窗口和发请求，不提前声称服务器修改成功。
- 新增原生EDIT截图合成，兼容已有聊天输入框；Client清理后80步及最终全部目标构建通过。公告与职位两种预览15项全部为1，含权限/空成员、开启/换行、模态、拖动、匹配释放、取消、4000限制、DBCS纯函数与真实控件插入、真实回车、载荷格式及保存关闭；既有行会滚动3项与快捷栏13项回归通过。已检查out/guild-notice-editor.png、out/guild-members-editor.png。
- 仍待：行会公告/职位原始数据和管理员状态的服务器消息接入、更新响应及实际联网保存；原生输入框长文本滚动/选择及Godot默认TextEdit字体颜色与MIR3实机的最终像素对照尚未全面验收。截图内容为离线中文样例。

### 行会多行编辑器横向缩放修正
- LayoutEdit不再每帧无条件MoveWindow；仅位置/尺寸变化时移动，减少原生控件重复重绘。编辑框宽或高改变时释放旧木纹画刷，下一次Show按新尺寸重建；此前仅字体高度变化才重建，横向缩放会使用错误的旧宽度纹理。
- 在原生公告预览中补充60行文本滚动检查及仅缩小显示宽度到3/4的验证；将实际画刷填充结果与从DirectDraw底图按新尺寸缩放的结果作网格像素比较，验证不止控件大小。新增两项均通过，既有15项仍通过，最终全部构建目标通过。显示尺寸随后恢复，未写入真实配置。

### WIL 5000版帧头偏移修复
- NPC拼接底图出现20像素间隔横向接缝，定位为NewSetIndex固定使用17字节帧头；现有GameInter/Inventory/MIcon为5000版，实际帧头21字节。按Godot WilArchive规则对5000/6000增加4字节，保留17版路径；6000版索引及完整资源支持未验证。
- 用独立Python解码GameInter并按386宽、1353/1351/1352拼接和RGB565 tint180/alpha220计算，最终NPC滚动截图59931个背景像素与参考一致，差异0（out/npc-composite-pixels.txt）。此前原生裁剪/非裁剪互比不能证明读取偏移正确。
- 背包四边裁剪、快捷绑定12项、技能说明8项、行会编辑17项、人物布局、选项音量及交易金币8项回归通过；已查看修复后的NPC、背包和行会实际截图。

### 原生NPC对话与输入提示
- 以Godot已完成实现为参考重写CNPCWnd，仍使用CLIENT DirectDraw、GDI宋体9pt和原生EDIT。386宽、204+20*条数高度（1..13条），1353/1351/1352拼接底图只混合一次；1221/1222关闭及1355..1358滚动箭头采用原资源坐标。
- 解析反斜杠换行、FCOLOR的17色、NOTCLOSE及<label/command>，保留同NPC关闭限制、新NPC重置；按ANSI/DBCS测量换行、22行滚动及参考按钮命中规则。链接黄色、按下红色、悬停下划线，保留1秒选项节流和释放激活语义。旧裸分配解析器被移除，末尾纯文本与不闭合标记不再导致旧循环问题。
- SM_MERCHANTSAY解码后交给SetScript，普通选项发1011；@@选项复用原生输入对话框，@@buildguildnow使用CmList2000提示，@@*显示密码遮罩。输入提交组合命令、CR及用户文本，保存发起提示的NPC编号；关闭NPC同步取消输入框并清除密码遮罩。YES/NO复用105.wav，参考无显式音效的NPC关闭/箭头不另加声音。
- Client清理构建及全部109步目标构建通过；四种实际预览均22项全1，覆盖解析/颜色、关闭限制、命中/按下/节流/释放、边界与滚动、输入取消、NPC编号捕获、密码遮罩及关闭清理。已查看out/npc-dialogue.png、npc-scrolled.png、npc-input.png、npc-notclose.png；截图为离线样例。
- 尚未完成：NPC与商店/仓库的层级和联动、@@特定密码长度规则、商店业务及实际服务器请求/回复验收；普通输入当前沿用90字节上限。音效已调用，未做扬声器听感检查。当前不宣称NPC系统或总体GUI全部完成。

### 原生商店列表、明细与批量购买界面
- 重写CStoreWnd，移除旧文字按钮/裸分配列表，使用CLIENT DirectDraw/GDI和Godot参考的1260/1261/1262底图。列表位于0,225/292x224，四行商品、42像素间隔、独立图标/文字命中区；明细宽488、4x3格，批量宽516。保留原尺寸Inventory图标、宋体9pt名称/价格/需求、参考8pt明细价格字体、颜色及高亮混合。
- 接入1225滚动条（85像素行程、参考5/商品数轨道步长）、滚轮、跨窗口拖动释放、配对按钮点击、双击分类/购买、明细翻页待响应及关闭返回列表；NPC关闭同步关闭商店及数量输入。批量数量复用原生1240输入框、CmList2302和105.wav确认/取消音效，按参考空输入为1及十进制前缀转换，接受1..65535。商店其他按钮参考未显式播放声音，未另加。
- 商品按七字段解析，校验NPC编号及计数；明细按两层六位编码和100字节记录提取名称、36处图标、62处32位实例编号、68处价格，页码取回复offset/10。请求使用实例编号拆分到wParam/wTag、数量到wSeries。购买成功/失败按参考更新余额/移除选中明细或显示1437..1439提示。
- 删除原有PACKETMSG约1KB正文复制及商店队列，改由GameProc直接调用Receive；不再把wIdent当作商品数，也不在失败/明细消息到达时重新打开关闭的商店。真实网络头部/服务端格式与物品入包仍须后续联调，本轮不声称购买已联网可用。
- Client清理构建80步及全部目标构建通过，四种预览各25项全1：解析/拒绝无效数据、布局、配对释放/间隙、滚轮/拖动、分类、明细选中/翻页阻断、批量输入/取消及NPC关闭联动、100字节记录解析、超过1KB的两层编码明细回复和模拟购买成功余额更新。NPC输入22项及交易金币8项回归全1。已查看out/shop-goods.png、shop-offers.png、shop-bulk.png、shop-quantity.png；数据为离线展示样例，未验证实际扬声器听感。
- 后续仍待：仓库/出售/修理等商人服务页面、商店与其他窗口的全局层级、悬浮物品详情、数量请求的实际服务器接收、购买失败/入包的联网验收及最终MIR3实机像素核对。旧仅四字段商品与旧CLIENTITEMRCD明细不再作为本窗口输入，按MIR3/Godot七字段/100字节记录为目标，服务端适配留在联网阶段。

### 原生仓库列表、翻页与取回数量对话框
- CStoreWnd新增仓库模式，沿用Godot的1261资源、逻辑-292,234/488x205裁剪布局，显示右侧十二格；格子22,266起、38间距、37x37命中，页码从1起，总页数至少1，仓库不绘制商品价格。按钮平移-292，翻页本地每页12条并清除选中项；空格、空列表及末页不能误选。
- Offer补入count/stackMode，提取100字节记录71处数量和61处堆叠标记；共享明细记录解码逻辑。704仓库列表采用直接分隔的六位编码记录（不同于商店明细外层再编码），数量来自wSeries；打开后列表刷新保留原NPC编号及待取回实例。
- 原生action9数量框使用CmList2301、1240底图、原生EDIT，确认/取消复用105.wav。堆叠数量大于1打开输入，支持*全部及参考十进制前缀，超出堆叠/非正数拒绝；非堆叠双击/确认直接发送1032，等待回复不重复发送或翻页。关闭仓库/NPC同步取消数量输入。
- 接入705/706/707显示状态：成功更新仓库剩余数量或移除已取完记录并收缩页码，失败清除等待并显示1445/1446；尚未把wTag携带的背包堆叠数量合入旧CLIENTITEMRCD，新增背包物品也依赖后续完整物品消息。当前不能声称联网取回完成。
- Client清理80步、全部目标109步及最终增量构建通过。三个仓库预览各23项全1，涵盖格子边界、配对释放/翻页、空列表、关闭、数量取消/范围/*、待回复/刷新、模拟部分和完整取回、704记录解析/拒绝、非堆叠双击及失败提示；商店25项与NPC22项回归通过。已检查out/storage-list.png、storage-page.png、storage-quantity.png，离线样例截图；实际音效听感未验收。
- 后续仍待仓库存入服务界面、物品详情悬停、完整背包堆叠数据、真实服务器存取流程与最后的MIR3实机像素验收。

### 原生背包商人服务界面首轮迁移
- 按Godot merchant_service复用原生背包：服务打开时定位519,0并置顶，标题CmList1702修理/1703出售/1704保管，按钮1227/1229/1231及按下前一帧位于198,401，非零单件报价使用1452、70,423/85x14及0xF8DCFA；保留原生背包底图、字体和关闭105.wav音效。参考服务选择/确认没有显式按钮音，未额外增加。
- Service明确Choosing/Quoting/Quoted/Unavailable/Paying状态；服务期间点击背包物品只记录身份并询价，不拖出实物。选择修理/出售发1024/1012，仓库存入直接可确认；报价0禁止确认，允许后续报价替换。匹配按下释放才确认，等待询价/付款时不更换物品，确认前再次匹配编号和名称，物品消失则撤销报价。
- 646/668/700消息打开对应服务，647/671更新报价；单件确认发送1013/1023/1031（以Define.h常量为准），实例编号拆分wParam/wTag。关闭背包清除服务，关闭NPC同步隐藏服务背包，收到仓库列表清除服务状态。尚未处理服务成功/失败后的库存、耐久和余额同步，真实联网流程未验收。
- Client清理80步、全部目标111步构建通过，merchant-repair/sell/deposit三个预览各17项全1，覆盖开启定位、选择不移除物品、询价等待、错类型报价、不可用与有效报价、配对释放、支付阻断、缺失物品、存入、关闭清理。仓库23项、快捷绑定12项及普通背包回归通过；已查看out/merchant-repair.png、merchant-sell.png、merchant-deposit.png，物品及报价为离线样例。
- 后续必须补齐：完整MIR3堆叠元数据、出售/存入数量对话框（1705/1706）、1451每件报价、响应更新和提示、鼠标持有与快捷物品使用的全面服务交互、实际扬声器音效与MIR3实机像素验收。当前旧CLIENTITEMRCD不含完整堆叠信息，单件路径不代表完整商人业务完成。

### 原生出售和存入数量输入框
- CItem新增原生m_count/m_stackMode元数据，默认及旧SetItem路径为1/0，不改变CLIENTITEMRCD协议结构；通过现有CItem复制保留数量，快捷栏FromItem改读取这两个字段。完整100字节MIR3背包记录的导入仍待迁移，本轮不从旧记录字段猜测堆叠。
- 商人服务确认时重新查找当前物品；出售/存入的stackMode>=2且count>1打开原生action10/11数量框，使用CmList1705/1706、1240底图、原生EDIT及105.wav确认/取消。支持*读取当前全部数量、十进制前缀、1..65535及实际堆叠上限；存入单个堆叠物品发送数量1。发送只转Paying，不提前扣减实物。
- 堆叠出售显示1451每件报价，其他使用1452；背包数量按参考锚点+18,+22、20x16白色宋体9pt绘制，仅完整位于可见网格时显示。新ClearService统一关闭数量输入并清空服务，用于窗口关闭、服务替换和进入仓库列表。
- Client清理80步及全部111步构建通过，出售/存入两个预览各14项全1，验证旧物品默认、快捷引用及持有/放回复制、打开取消、非法和超量输入、十进制前缀、*全部、物品消失、服务替换/关闭/仓库联动。既有商人17项、仓库23项、快捷绑定12项、布局持久化7项及背包四边裁剪回归全1。已查看out/merchant-sell-quantity.png与merchant-deposit-quantity.png，数量为离线样例。
- 仍需：完整物品消息导入和堆叠更新、出售/存入响应后的数量同步/消息提示、快捷引用随服务器更新、实际服务器收到请求与音效听感验收；本轮检查没有证明联网交易完成。

### NPC仓库密码输入规则校正
- 对照Godot补齐@@*特殊命令的字节上限：enter_stgpasswd/Input_stgpasswd/Change_stgpasswd/confirm_passwd识别大小写保持参考，前缀下划线位置<10时解析其前数字，非正/非法回到90；下划线位置>=10或不存在采用10。其他@@*仍遮罩但默认90，普通输入不遮罩。
- 抽出既有行会编辑器的ANSI/DBCS插入限长函数到Common/AnsiInput.h，NPC原生EDIT在EN_CHANGE中使用，保留未改动前后缀且不截断中文双字节字符；行会编辑器继续使用4000限额。结束输入恢复90字节默认限额并清除密码遮罩。
- 修正上一阶段仅捕获NPC编号仍会向旧NPC提交的问题：确认必须与当前NPC编号相同，确认/取消均消费并清理提示状态；按参考允许空答案。原生回车对NPC输入直接走FinishNotice，从控件动态取完整答案，移除固定90/260字节取值造成的截断。
- Client清理80步及全部111步构建通过；npc-password预览10项全1，覆盖各前缀/回退规则、密码/长度属性、真实中文插入与后缀保留、取消重置、NPC变更拒绝、空答案、300字节输入及原生回车、普通输入恢复。NPC原有22项、行会17项、出售数量14项、交易金币8项回归全1；已查看out/npc-password.png，截图使用测试密码。
- 尚未验证真实服务器密码流程及超长命令的传输上限：当前SendPacket仍使用旧8KB编码正文缓冲，GUI允许的极大脚本前缀长度不能据此宣称可完整发送。后续联网阶段需核对包长度规则和传输实现；本轮不改变此缓冲。

### NPC与商店窗口层级统一
- 按Godot draw顺序把NPC/商店从普通激活顺序中分离：普通面板、技能快捷栏、NPC、商店、行会编辑器、消息对话框；修正背包后来置顶盖住NPC/商店以及技能快捷栏晚于模态框绘制的问题。鼠标命中先检查商店、再NPC，底层窗口不再抢点击；行会编辑模态期间也禁用底层悬停提示。
- NPC和商店滚轮改按指针实际覆盖的窗口分派，移除旧NPC仅按激活顺序滚动的分支，NPC滚动清除按下/箭头状态；普通面板剩余滚轮分派仍沿用旧IsTopWindow，后续可继续按Godot核对。
- 全部目标增量构建通过；npc-shop-layering离线场景11项全1，含背包最后激活时NPC/商店优先命中、可见背包区域命中、真实点击路由、覆盖区滚轮、商店关闭后露出NPC和模态阻断。将背包最后激活与NPC/商店最后激活两种顺序分别清屏并实际渲染，432x449（193968）个RGB565像素全部一致。已查看out/npc-shop-layering.png，特意重叠三窗口展示半透明NPC上层与商店最上层。
- 商店25项、仓库23项、出售数量14项、NPC22项及技能按键菜单10项回归通过。该对比证明激活顺序不再改变覆盖关系，不等同于整体MIR3像素验收；地图/小地图、目标信息和完整物品悬浮层仍待后续迁移与核对。

### 原生头像选择器首轮迁移
- 使用CLIENT原生CGameWnd、DirectDraw和GDI实现580x404头像选择器，复用2090底图、1966滚动条、1221/1222关闭及2092/2094上传/默认按钮；按Godot坐标绘制十二行文件名、选中/悬停底色、宋体9pt说明和85x104 JPEG预览。通过OLE解码JPEG，未引入Godot运行时。
- HUD头像区域显示2096/2097入口，折叠状态可打开；窗口加入普通面板层，NPC/商店及模态窗口保持上层。接入鼠标配对释放、跨窗口滚动捕获、滚轮、Escape关闭以及105.wav点击音效。
- 枚举资源工作目录下不超过4095字节的普通JPG文件，选中后解码预览；上传重新读取并检查格式/大小，错误提示4714/4716，成功请求限制一秒一次；恢复默认产生空请求。保留参考滚动到底可能出现空页的行为，关闭清除列表和预览但保留上传时刻。
- Client清理81步、全目标112步构建通过；portrait-empty/preview/scrolled三个实际客户端预览各21项全1，NPC/商店层级11项、NPC密码10项及行会编辑17项回归通过。已检查out/portrait-empty.png、portrait-preview.png、portrait-scrolled.png，JPEG为离线测试图案。
- 尚未完成：头像上传请求与服务器协议连接、上传结果和其他玩家头像更新、入口4715悬浮提示、实际音效听感及MIR3实机像素验收。requested/requestPending仅承接GUI操作，不代表已上传服务器。

### 原生头像按钮悬浮提示
- 补入头像入口4715、上传4700、恢复默认4701及关闭1052提示；沿用Godot按钮右下角锚点、屏幕右边界约束、宋体9pt黑字、淡黄底黑边以及6像素起逐帧增长的高度规则。提示文字限制在当前展开矩形内。
- 按真实窗口命中处理遮挡，按钮按下隐藏自身提示；展开聊天栏隐藏头像入口提示，消息框/行会编辑器活动时阻断头像提示。绘制放在普通窗口及模态处理后，鼠标持有物仍保留原顺序。
- Client清理81步、全部111步和最终增量构建通过；portrait-hint七项检查全1，头像选择器原21项回归全1。已查看out/portrait-hint.png，预览先推进24帧再截图，显示完整上传提示。
- 后续需要统一全HUD按钮提示的共享动画状态（当前头像和坐骑分别持有），补齐其余普通面板提示；头像服务器请求、实际声音和MIR3实机像素核对仍待后续阶段。

### 普通面板按钮提示与共享动画
- CInterface统一持有提示文本、展开状态和高度，集中绘制头像、坐骑及新增背包关闭/人物关闭/任务详细与关闭提示；移除头像和坐骑重复绘制代码。按Godot使用1052/2200等原始CmList，实际窗口命中决定提示所属，移动窗口后锚点跟随，模态窗口阻断。
- 修正Windows min宏对++hintHeight重复求值的问题，自增移到表达式外，保证每帧由6开始只增加1像素；文字仍裁剪在展开框内。新增提示共用动画状态，变更文本保留尚在增长的高度。
- 修复背包/人物关闭按钮在窗口外释放时未清除按下状态的问题；界面捕获释放，关闭手势结束后直接返回，避免松开到格子上继续拾取/穿戴物品。保留原105.wav配对关闭音效。
- Client清理81步、全部112步及最终增量构建通过。inventory-hint/status-button-hint/quest-hint各七项全1，覆盖命中、按下隐藏、窗口外释放、移动锚点、模态、首帧7像素和完全展开；头像提示七项、坐骑十三项、商人数量十四项回归全1。已检查out/inventory-hint.png、status-button-hint.png、quest-hint.png。测试释放点改为450,450，旧790,450实际位于背包关闭按钮内，不是有效的窗口外测试点。
- 仍待其余面板按钮提示、物品完整悬浮层、地图/小地图等GUI及后续服务器流程；截图采用离线样例，不代表真实联网数据或整体MIR3像素验收。

### 组队、行会、技能、交易与NPC按钮提示
- 将组队四个按钮（1052/1600/1601/1602）、行会九个按钮（1052及4008..4015）、技能关闭及八系（1052/5050..5057）、交易确认（1300/已确认1303）和NPC关闭1052接入CInterface共享提示绘制；采用Godot对应帧、偏移和按下隐藏规则。NPC禁止关闭时不提示不可见的关闭按钮，商店覆盖区仍不透出下层提示。
- 组队与行会新增按钮按下后的全局释放分派，移出窗口释放也清除按下状态；不改变原配对激活、输入框和点击音效逻辑。
- Client清理81步、全部111步构建通过。group-hint/guild-hint/skill-button-hint/trade-hint/npc-close-hint五个离线预览各五项全1，逐个覆盖该窗口按钮命中、按下隐藏、移出释放、模态以及交易确认/禁止关闭状态；组队创建原生输入回归通过、行会编辑17项和技能快捷菜单10项全1。
- 已查看out/group-hint.png、guild-hint.png、skill-button-hint.png、trade-hint.png、npc-close-hint.png。截图为实际CLIENT离线渲染。HUD主按钮及聊天按钮提示、小地图、完整物品提示与服务器数据联动仍待后续阶段。

### 主界面与聊天栏按钮提示
- 接入主HUD及展开聊天栏共23个按钮提示，使用实际命中矩形右下角作为锚点、共享宋体9pt淡黄提示及增长动画。显式映射旧CLIENT编号7..15与Godot/MIR3顺序，避免技能、任务、背包等提示错配。
- 组队/行会聊天按钮随当前频道显示2618/2631和2619/2630；三类消息接收按钮新增独立提示状态，初始2637/2639/2643，点击按参考依据当前服务器拒收标志选择基础编号或+1。按钮不自行改变服务器拒收状态，服务器选项同步仍待联网阶段。
- 按下时隐藏提示，收起聊天栏后隐藏其七个按钮提示；已有面板/商店/NPC命中和模态框优先阻断底层HUD，头像入口继续优先处理。
- Client清理81步、全部111步构建通过。hud-button-hint/chat-mode-hint/chat-reception-hint三个预览各九项全1，遍历23按钮文案和锚点、按下状态、频道切换、拒收两种状态、收起隐藏、模态与背包实际覆盖区域；组队聊天输入、头像提示七项及普通面板提示七项回归通过。
- 已检查out/hud-button-hint.png、chat-mode-hint.png、chat-reception-hint.png。后续仍需小地图与目标显示、完整物品提示、真实服务器选项同步及实际音效验收；当前离线状态验证不代表联网聊天接收完成。

### 组队/行会按住移动的回归修正与小地图迁移准备
- 检查小地图输入路由时发现df74539的文本替换同时命中了OnMouseMove中的行会滚动分支，误插入组队/行会OnLButtonUp。修正为移动期间只消费事件、保留按下状态，真正释放仍由OnLButtonUp分派。
- 既有remaining-hint预览增加真实OnMouseMove后再检查按下隐藏，并继续窗口外释放；全部目标增量构建通过，组队/行会/技能/交易/NPC五个预览各五项全1。该检查补上上一阶段仅按下/释放未覆盖移动事件的缺口。
- 小地图迁移证据：Godot client/src/hud/minimap.hpp及hud_view.cpp的draw_minimap/minimap_pointer/apply_minimap_reply。Mmap/Fmmap本地资源均5000版本压缩WIL，分别309/40帧。map_number-1小于1000读Mmap，否则读Fmmap的id-1000；请求1033。当前CLIENT无小地图窗口，仅保留HUD按钮编号1。
- 待实现规格：128/256裁剪，出生默认右上角；坐标x*3/2,y跟随主角并约束源矩形；图像150/255调暗，透明时alpha175，灰边；1486/1487/1488装饰与1482/1483、1484/1485按钮；提示1800..1803；Ctrl按下记录偏移，移动仍在旧区域且Ctrl按住时才拖动，捕获释放；按钮按下即切换。小地图绘制高于普通面板、低于NPC/商店，四像素玩家闪烁框及三种标记按参考裁剪。不可声称小地图GUI已完成，目前仅完成参考和资源核对。

### 原生小地图首轮实现
- 新增CMinimap，使用CLIENT现有WIL读取与DirectDraw绘制，加载Mmap/Fmmap；map_number-1小于1000选择Mmap，否则选择Fmmap的id-1000。按Godot实现128/256源图裁剪、x坐标3/2换算、跟随人物并限制源矩形、150/255调暗、alpha175透明模式、灰边与1482..1488按钮/装饰。
- 小地图绘制位于普通面板之上、NPC/商店之下，命中顺序一致；接入按钮1800..1803共享提示、Ctrl拖动与释放捕获、按下切换大小/透明，V开关、T大小、Y透明。HUD入口保留既有105.wav点击声；参考小地图内部切换未单独播放声音。覆盖区阻断背包点击、滚轮和双击。
- HUD入口100ms、V键1000ms冷却依回复时间计算；打开发送1033请求并等Receive后显示，关闭不重新请求。Unavailable取消等待/显示并添加1458消息，未请求的Receive只载入资源不自行显示。本轮尚未把实际服务器回复分派至Receive/Unavailable，不能宣称联网小地图完成。
- 人物4像素闪烁轮廓、黄色type5/绿色/橙色标记按参考范围与尺寸裁剪；ShowMon读取原生选项7。标记容器已实现，当前只由离线样例填入，真实附近角色分类/更新尚待整合；主角位置已读取现有英雄坐标。头像内地图/J键模式尚未实现。
- Client清理82步、全目标112步及最终增量构建通过。minimap-small/large/transparent/dragged四种实际预览各17项全1，最终键盘/覆盖/滚轮/双击五项全1；普通面板提示七项、NPC商店层级十一项及HUD按钮九项回归全1。使用参考测试位置337,267，源矩形441,203，实际截图黄色713,47、绿色755,80、橙色770,72及远处799,79裁剪像素全部匹配RGB565预期，证据out/minimap-marker-pixels.txt。
- 已检查out/minimap-small.png、minimap-large.png、minimap-transparent.png、minimap-dragged.png，使用原版比奇地图资源，人物/附近标记为离线样例。后续需服务器地图回复、切图生命周期、实际角色标记、头像地图，以及MIR3实机整体像素与音效听感验收。

### 头像区域内的原生地图模式
- CMinimap补入portraitMode/pending/visible独立状态，共用1033请求和Mmap/Fmmap回复；J键或折叠HUD头像区域释放切换（上传入口优先），1000ms回复冷却，等待期间不继续显示普通头像。Unavailable清除显示/等待但保留模式。
- 原生WIL帧一次解码为RGB565像素缓存。Fmmap裁剪256x256并半尺寸显示，Mmap裁剪180x130再绘制128x128，按参考近邻采样、150/255调暗、黑底及内侧灰边；小图不足256时保留530/485起点规则。地图在HUD底图之前绘制，展开聊天栏时隐藏，避免地图越过头像装饰边缘。
- 绿色/橙色附近标记使用参考先整数除2、再3/2转换的坐标规则，玩家4x4内侧轮廓沿用累计800ms闪烁；主角与附近标记仍复用原生小地图容器，实际附近角色填充待联网阶段。
- 使用现有Config.ini的Options/InformationMode保存模式，预览默认禁用用户配置写入；读取后重新初始化保留待请求状态并发送1033。预览持久化只写preview-information-mode.ini。提取共用Request消除两个地图模式重复发包代码。
- Client清理82步、全部111步构建通过；portrait-map-full/normal/expanded三个预览各十项全1，覆盖独立请求/回复、源区域与标记坐标、冷却、关闭、入口拖出释放、J键、持久化和Unavailable。最终重新初始化持久化回归、右上角小地图17项、头像提示七项、HUD提示九项通过。
- 已检查out/portrait-map-full.png、portrait-map-normal.png、portrait-map-expanded.png。按Godot样例坐标验证玩家572,532起的4x4轮廓（含周围空白）、绿色583,542及橙色591,538，RGB565像素全部匹配，证据out/portrait-map-pixels.txt。实际服务器地图回复、切图生命周期、附近角色更新及整体MIR3像素验收尚未完成。

### 目标头像、血条与怪物属性面板
- 新增CTargetDisplay，读取MonImg与现有ProgUse资源，迁移Godot target_portrait.hpp规则：怪物type3，排除127/153及excluded，普通形态低8位、530..533/2000以上保留完整编号，资源缺失回退1999；普通玩家使用职业/性别650..655，未知职业回退0。特殊形态省略徽章和属性面板。
- 实现目标身份切换/800ms保留/移除及按actor缓存属性。10字节属性体读取有符号图标/修正值，保留参考两轮排序（不是完整数值降序），迟到属性不切换当前显示目标。该状态API尚未连接旧Actor悬停字段或服务器1304回复，当前通过离线场景驱动。
- 血量数字使用宋体9pt、0xe6e6e6及532/562/565,570矩形；current为0时两端显示问号。660边框与661填充位于533,582/534,583，保留0.01f精度导致50%为41像素的规则。怪物属性底框615,487/105x85，RGB50透明230、RGB120边框，原始元素图标与正负百分比按参考绘制。
- 原生接口渲染目标头像/徽章、血条及属性面板；展开聊天栏时隐藏。未额外添加声音，参考目标悬浮本身不播放点击音效。自定义玩家JPEG头像、真实角色职业/完整形态字段、属性请求/响应和实际目标生命周期接线仍待后续阶段。
- Client清理83步、全部111步及最终增量构建通过；目标预览十三项全1，含有效头像、800ms边界、排除/缺图回退、特殊/玩家/未知职业、属性体拒绝/缓存/参考顺序、移除及未知血量。头像内地图十项、右上角小地图十七项、HUD提示九项回归通过。
- 已检查out/target-monster.png、target-attributes.png、target-human.png、target-unknown.png、target-special.png。特殊531在当前资源中缺图，截图按规则显示1999问号；其余人物、血量和属性均为离线样例。仍不能据此声称真实服务器目标显示已完成。

### 目标玩家自定义JPEG头像与照片选项
- 抽出Common/PortraitJpeg.h的原生OLE JPEG解码，头像选择器与目标照片共享；保留JPEG签名检查，选择器本地文件限制4095字节，目标回复按Godot允许4096字节。目标照片仅接受当前已显示玩家身份，空数据清除，损坏/超量/错误身份拒绝并保留原图。
- 同一玩家更新职业/性别保留照片，切换玩家或怪物清除旧照片；采用534,479/84x103目标照片矩形。把自身头像与目标头像移至HUD底图之前，边框覆盖顺序与Godot一致，照片关闭时回退标准职业头像。
- 补齐此前尚未执行的选项20：点击切换UserPic，发布CmList2132/2131白字/33023底色聊天提示，沿用105.wav选项点击声，并通过现有Config.ini选项读写路径保存/恢复UserPic。预览仅写preview-photo-options.ini。
- Client清理83步、全部112步构建通过。target-photo/target-photo-disabled各十一项全1，包括JPEG加载、错误身份/损坏数据、4096边界/4097拒绝、同目标保留/换目标清除、空回复、怪物拒绝、真实选项点击及配置恢复；头像选择器21项、音频设置9项、头像地图10项、目标面板13项回归通过。
- 已检查out/target-photo.png和target-photo-disabled.png，照片为离线测试图案；两张截图像素差异边界为534,487..616,581，位于HUD头像开口内，区域外不变。真实头像查询/回复路由、当前悬停角色驱动及服务器上传仍待接入；选项7等剩余本地开关还需继续从Godot迁移，当前不能宣称全部选项业务完成。

### 设置页的小地图怪物标记开关
- 补齐选项7的真实点击切换，复用照片选项的本地切换路径，显示CmList2646/2647白字/33023底色提示，保留原105.wav点击音效。ShowMon默认开启，并通过现有Options配置读写路径保存/恢复。
- 原生小地图已经读取选项7，本轮闭合设置按钮到黄色type5标记绘制的路径；绿色/橙色附近角色不受此开关影响。实际角色标记数据源仍待接入。
- 全部目标增量构建通过；options-markers-on/off各五项全1，含非配对释放、开关、默认值与配置恢复。音频设置九项和目标照片十一项回归全1。已检查out/options-markers-on.png及options-markers-off.png，实际像素验证713,47黄色按开关消失，755,80绿色及770,72橙色保持一致。
- 预览只写preview-marker-options.ini。设置页的强制攻击、魔法光、伤害数字、阴影与服务器控制开关仍需继续迁移并接入对应运行状态，尚未完成整页业务验收。

### 设置页阴影混合开关接入原生渲染
- 选项4接入GameProc现有m_bShadowABlend及800项m_wShadowClr表；提取SetShadowBlend统一设置页、启动恢复与原@shadow命令，开启使用黑色混合源，关闭使用原RGB20阴影源。修正进入游戏时固定重置灰色表覆盖保存状态的问题。
- ShadowBlend默认关闭，随现有Config.ini Options读写路径保存；选项点击沿用105.wav，按Godot不额外发布阴影切换聊天提示。原调试命令同步更新设置页并保存，避免两处状态分离。
- Client清理83步、其余目标109步构建通过。options-shadow-on/off各四项全1，验证实际按下/释放、渲染标志与全部800项颜色、默认值及保存恢复；音频九项和小地图标记五项回归通过。已检查out/options-shadow-on.png和options-shadow-off.png，预览只写preview-shadow-options.ini。
- 本轮验证证明按钮到现有角色渲染控制变量/颜色表的接线，未在真实地图上逐像素验收角色阴影。魔法光、伤害数字、强制攻击及服务器控制选项仍待相应路径迁移。

### 设置页魔法光开关接入照明网格
- 选项9新增实际点击、CmList1518/1517提示和MagicLight配置保存/恢复，默认开启，与Godot一致；保留105.wav点击音效。
- CMagic::DrawLight在原光照提交位置检查选项，关闭仅跳过照明网格写入，仍执行光效摇摆/衰减/结束处理；角色施法与剑技的两处SetLightRadiusWithRing也受同一选项控制。普通人物/地图静态照明不受此开关影响。
- 全目标增量构建通过。options-light-on/off各六项全1：调用真实CMagic::DrawLight和CLightFog网格，开启有光、关闭无光、恢复有光，关闭时_LIGHT_ACTIVE仍能正常结束；同时验证默认值和保存恢复。音频九项、阴影四项回归通过。
- 已检查out/options-light-on.png和options-light-off.png，预览只写preview-light-options.ini。该测试验证照明网格和设置状态，尚未在完整地图/实际施法场景核对最终屏幕光照效果；伤害数字、强制攻击和服务器选项继续待迁移。

### 原生体力变化数字与设置开关
- 新增DamageNumbers，按Godot参考使用GameInter原帧：小额受击1790/1800、大额1830/1840、回血1810/1821、零伤害1850 MISS；保留八槽游标回绕、每次绘制斜向漂移及age 0..40寿命。人物/怪物采用不同高度偏移。
- 接入原生Actor/CHero受击、血量变化和GameProc直接血量消息路径；死亡角色受击不加入数字。渲染在人物血条附近执行，使用现有DirectDraw，不依赖Godot。
- 设置10接入BloodNum默认开启、真实点击及配置保存恢复，沿用105.wav和CmList2122/2121聊天提示。关闭时不新增/绘制数字，保留参考暂停年龄行为。
- Client清理83步与全目标构建通过。options-damage-on/off各九项全1，覆盖血量差、无变化/初始血量、关闭、默认/持久化、帧样式、寿命和回绕；魔法光六项、音频九项回归通过。预览仅写preview-damage-options.ini。
- 已检查并展示out/options-damage-on.png、options-damage-off.png，顶部依次为-9、-99、-100、-65535、+25、MISS原生资源样例。仍需真实服务器战斗验证消息时序、世界坐标锚点与画面遮挡，不以离线样例宣称战斗显示全部完成。

### 强制攻击模式接入玩家点击判定
- 选项8使用AttackUser默认开启，接入实际按钮切换、CmList1515/1516提示、105.wav点击及配置保存恢复。参考Godot step_toward_pointer：关闭时普通点击玩家受名字颜色限制，255/27631例外；Shift继续绕过限制，Alt挖取分支不受该限制影响。
- CMyHero::OnLButtonDown在行走/攻击处理前读取实际目标Actor身份与名字颜色，调用设置页共享CanAttackHuman判定；怪物不走玩家颜色限制。未把开关替换为持续按Shift，保留正常追近目标行为。
- Client清理83步、全目标111步构建通过；options-attack-on/off各五项全1，验证按钮、白名限制、颜色与Shift例外、默认值和配置恢复。体力数字九项回归通过。预览只写preview-attack-options.ini。
- 已检查out/options-attack-on.png和options-attack-off.png；当前证明配置与客户端判定接线，尚未通过真实服务器玩家战斗验证完整动作/伤害结果。服务器控制的设置按钮仍待接入。

### 设置页服务器开关与1401确认消息
- 补齐选项11..14/16..18的CmList2123..2129命令，攻击模式19发送1422命令，沿用原生SendSay/CM_SAY与105.wav点击音效。服务器开关独立保存confirmed状态，点击显示!confirmed；重复点击不盲目反转确认状态。
- 按Godot SystemOptions::receive接入SM_CLIENT_OPTIONS(1401)：nRecog七个掩码40000000/80000000/08000000/04000000/02000000/01000000/00800000，wParam攻击模式。仅确认值变化时覆盖预测显示并释放对应按下状态；重复确认保留预测帧。服务器状态不写本地INI。
- 同步主界面私聊/行会/所有聊天接收图标和提示，保留参考所有聊天同时更新行会拒绝标记的行为。
- Client清理83步、全部111步构建通过；options-server-on/off各六项全1，含真实按钮预测、相同确认、七个命令编号、通过fnEncodeMessage和GameProc实际接收入口的1401确认、接收按钮同步、重复消息；强制攻击五项回归通过。
- 已检查out/options-server-on.png和options-server-off.png，展示七开关开启/关闭及攻击模式4/0。该阶段为原生客户端协议接线与离线编码消息验证，尚未证明现有游戏服务器的命令/回复兼容；实际服务器交互与完整GUI验收仍未完成。

### 小地图回复路由与切图刷新
- 根据Godot仓库client/docs/evidence/feature-audit/004282C0.c的case710，地图编号来自消息wParam（WORD1(v358)，紧随opcode），接入CLIENT接收分支710到Minimap::Receive；711接入Unavailable并显示CmList1458。编号仍按一基映射Mmap/Fmmap。
- 新增MapChanged，按Godot reset_map_overlays清除旧图尺寸/像素/标记/冷却/拖动；保留右上角已打开或请求中的意图，头像地图按portraitMode恢复请求。两种地图都隐藏至新回复，合并发送一次1033请求。接入LoadMapChanged及OnSvrMsgNewMap的实际地图加载路径。
- Client清理83步及全目标111步构建通过。minimap-server-reply/unavailable各六项全1，使用fnEncodeMessage经GameProc实际接收入口验证首次打开、清空、无效编号、换图恢复、不可用、关闭状态；原小地图十七项回归通过。
- 已检查out/minimap-server-reply.png、minimap-server-unavailable.png，分别显示右上角/头像地图及地图不可用状态。成功截图底部保留此前不可用测试产生的提示，属于测试序列历史。仍需真实服务器1033/710交互验证，附近玩家/怪物标记数据源与完整场景尚待接入；不据此宣称地图系统完成。

### 目标属性回复与移除清理
- 接入原生GameProc的1304目标属性回复：nRecog目标、wParam分类、wTag状态、10字节解码体，与Godot保存的004282C0.c case1304字段一致。复用TargetDisplay已有有符号修正值和缓存规则，未知/截短回复不改变面板，已缓存旧目标的迟到回复不切换当前目标。
- 抽出TargetDisplay::Clear用于切图时清空显示身份、照片、属性及血量，保留已加载头像资源。SM_DISAPPEAR/SM_CLEAROBJECT在接收时移除目标显示和对应缓存；当前目标照片同时释放。
- Client清理83步、全部111步及最终增量构建通过。target-server-attributes/removed各七项全1，经实际编码/接收入口验证字段、未知目标、截短体、迟到回复、消失、清空和最终显示；目标照片十一项回归通过。
- 已检查out/target-server-attributes.png及target-server-removed.png。悬停目标仍由预览指定，原生旧Actor缺少部分MIR3完整形态/职业字段；真实悬停适配、属性请求、照片查询回复及服务器联调仍待继续，不能视为目标显示端到端完成。

### 原生Actor驱动目标显示
- 新增GameProc::UpdateTargetDisplay，在实际RenderScene完成鼠标目标选择后、HUD绘制前更新目标；普通/展开HUD、窗口和模态界面遮挡时按无目标处理。死亡对象和NPC不作为头像目标，沿用Focus原800ms规则。
- 当前旧Actor怪物使用已参与原生怪物绘制的bDress外形及HP/MAXHP/HPPercent，目标变化且属性未缓存时发送1047(actor,x,y,0)。玩家使用性别；自身使用已有m_bJob，远程旧Actor缺职业字段时按参考未知职业255回退帧0。完整WORD外形、怪物特殊状态和远程职业仍需MIR3角色协议适配，不猜用武器/头发字段。
- Client清理83步、全部111步及最终增量构建通过。target-native-hover/expired各七项全1，以真实CActor对象调用生产UpdateTargetDisplay验证外形/血量、更新、800ms边界、死亡/NPC排除和未知职业；属性消息七项回归通过。
- 已检查out/target-native-hover.png及target-native-expired.png。悬停截图未注入属性回复，因此属性底框为空。此阶段证明Actor到目标显示的接线，尚未在完整地图上验收真实鼠标命中、1047网络请求/1304响应或远程玩家照片查询。

### 玩家目标照片查询去重
- 根据Godot set_hovered_human与CM_TARGET_PORTRAIT=1152，新增TargetDisplay::QueryPortrait并接入实际玩家悬停更新。使用现有SendPacket发送1152/零头字段及ANSI玩家名字，目标身份保留在本地显示上下文。实际服务器报文兼容仍待抓包联调。
- 同一目标同一名字不重复发送；空名字等待名字回复，改名/切目标/移除后返回/切图清空允许重新查询。照片数据清除与查询名缓存分开，空照片结果不会导致每帧重发。
- Client清理83步和全部112步构建通过；target-portrait-query十项全1，照片显示十一项回归通过。已检查out/target-portrait-query.png，未注入JPEG回复，显示标准职业头像。该离线检查验证查询去重状态，不证明网络请求成功。
- 照片回复路由以及头像上传/恢复默认的完整报文尚未接入。Godot仅暴露portrait_upload_requested事件，当前没有足够报文证据，保留待发送数据，后续查证原版/服务器定义后继续实现。

### 原版头像上传协议与原生发送
- 使用ida-reverse读取既有解包数据库，输入镜像SHA256为08f68b794bfb28803d1132c02b26d2595dba309be5b2b475ae0cdfe6a752f8aa，与Godot feature-audit一致。镜像位于C:/Users/czx/AppData/Local/Temp/mir3-assess-205da311dc994a8bbd59010e1c6a9ce2/Mir3-unpacked-analysis.exe，当前IDA会话e3ded018。启动器宝藏传奇3.exe与加壳Mir3.exe并非这些函数地址的分析对象。
- 0x47D3A0确认1152查询头字段全零、正文名字；0x47D360确认1151上传头字段全零、正文按显式长度编码。0x41ECC0确认JPEG<=4095、1000ms限制及恢复默认发送空数据。0x47C730确认非零长度不走strlen。只读证据导出out/portrait-query-original.c、portrait-nearby-original.c、portrait-upload-original.c。
- 新增PortraitWnd::SendRequest，在原生Interface渲染入口消费已验证的待发送请求；1151上传使用requested.size，默认恢复nullptr/0，消费后不重复发送。保留原按钮105.wav、JPEG检查和节流。
- Client清理83步、全部111步构建通过。头像预览新增本机UDP回环捕获实际SendPacket输出：消息1151、全零头、JPEG逐字节一致、空默认报文和一次性消费均通过（jpeg=1/reset=1）；原头像选择二十一项回归全1。回环仅用于离线字节验证，不代表真实TCP服务器接受。
- 已检查out/portrait-upload-verified.png。原版还把照片编码写入角色.fac缓存并更新自身头像，此本地缓存/自身照片以及照片回复路由、实际服务器存储仍未完成，不能宣称头像业务已全部完成。

### 1403目标照片回复接线
- 依据Godot保存的004282C0.c case1403：nRecog目标身份，wParam/wTag为32位版本标记，正文JPEG最多4096字节；远程目标版本为零走无照片分支。接入原生GameProc，对当前玩家目标调用ApplyPortrait，零版本清除照片并回退标准头像。
- 解码缓冲4097字节，用已有ApplyPortrait的4096上限拒绝超量数据；错误身份和损坏JPEG保持当前照片，迟到回复不覆盖新目标。不把版本字段误当JPEG长度。
- 全目标增量构建通过。target-photo-reply/cleared各七项全1，全部经fnEncodeMessage/fnEncode6BitBuf和GameProc真实接收入口，覆盖显示、错误身份、坏JPEG、清零、4096边界、4097拒绝和迟到回复；查询去重十项回归通过。
- 已检查out/target-photo-reply.png和target-photo-cleared.png，照片为既有测试图案。原版还保存按服务器/角色区分的.fac文件和照片版本配置；当前只接当前目标显示，离开目标后的缓存、自身照片和真实TCP服务器交互仍待完成。

### 自身照片即时显示与回复更新
- 通过既有IDA会话e3ded018核对原版0x4381B0..0x4381EE：自身照片矩形为534,486到616,580（82x94），区别于目标玩家照片84x103。证据out/portrait-render-original.c。
- PortraitWnd增加独立自身IPicture，上传1151后使用已验证JPEG即时更新，恢复默认清空；关闭选择窗口不丢失自身照片，Interface销毁/对象析构释放。1403自身身份回复同步该照片，目标照片处理仍保留。
- 原生HUD头像区域在无头像地图时优先绘制自身照片，否则绘制标准职业头像；其上仍按现有顺序绘制目标头像与HUD边框。未把自身照片错误套用目标照片的尺寸或UserPic开关。
- Client清理83步、全目标111步构建通过；own-photo-uploaded/default各六项全1，覆盖发送后即时更新、关闭窗口保留、坏JPEG保留、恢复默认、自身编码回复及清零；上传回环JPEG/空报文回归通过。已检查out/own-photo-uploaded.png和own-photo-default.png。
- 本轮自身照片为内存状态；原版按服务器/角色区分的.fac缓存、版本配置、登录恢复和真实服务器存储尚待完成，仍不能宣称整个头像流程验收完成。

### 原版FAC缓存格式核实与编解码
- IDA会话e3ded018核对0x43F060/0x43F220/0x43F3B0：UserPic.ini的UserPic节使用服务器名_角色名键，Data/服务器名_角色名.fac存照片；版本不符或解码失败时节流重新查询。0x43F3B0设置固定密钥-1901381136/-1612834157，再调用0x47D560解码。证据out/portrait-cache-load-original.c、portrait-cache-decode-original.txt。
- 新增Common/PortraitCache.h实现8字节头、长度编码、四轮异或及32位校验，与原版0x47D4C0/0x47D560一致。限制有效载荷1..4096，验证长度与校验后解码，拒绝损坏缓存。
- VS x86 C++17独立编译测试通过；从同一解包IDB提取两个原版函数机器码，在本机Unicorn中执行，与新代码1/2/255/256/4095/4096字节样例逐字节对照，编码和解码全部一致；原生测试同时覆盖损坏校验、损坏长度和大小边界。记录out/portrait-codec-machine.json、portrait-cache-check.txt、test-portrait-cache.cpp。Unicorn仅安装到忽略目录out/portrait-emulator。
- 本阶段没有新增页面或改变截图。缓存编解码尚未接入文件读写；下一阶段仍需保存服务器选择名、建立服务器/角色缓存上下文、接入上传/1403版本保存和登录恢复。完整GUI与服务器目标保持未完成。

### 自身FAC文件保存与角色隔离恢复
- ClientSocket成功选服请求保留selectedServerName；Interface创建时按该名字和g_szCharName建立Data/服务器_角色.fac上下文，加载FAC后经原生JPEG验证恢复自身照片。空身份或含路径分隔/非法文件名字符时禁用文件上下文，避免角色名字变为文件路径。
- 上传后保存原版编码FAC，恢复默认删除对应文件；有效自身1403照片回复同步保存，零版本清除缓存。没有改变原版图片资源或引入Godot。缓存文件I/O失败不阻断内存照片显示。
- Client清理83步及全目标112步构建通过。own-cache-restored/default各七项全1，真实文件读写覆盖恢复、跨角色/跨服隔离、非法路径拒绝、删除默认、损坏拒绝及重新保存；自身照片六项回归通过。测试仅写preview-photo-cache目录。已检查out/own-cache-restored.png和own-cache-default.png。
- 当前进入游戏恢复本地有效缓存，尚未按原版UserPic.ini版本与服务器角色版本比较；远程目标缓存、自动重查、真实登录过程与服务器存储仍待完成。不得把离线SetCacheContext恢复测试描述为已通过完整登录联调。

### 服务器头像上传确认与版本记录
- 依据原版004282C0.c case1402及0x42D2DE对dword_823A30的写入，1402非零nRecog为上传确认版本，显示CmList1519白字红底提示并记录UserPic.ini/UserPic/服务器_角色。零确认按原版忽略，不把本地SendRequest作为服务器成功。
- PortraitWnd缓存上下文增加独立版本文件路径，SaveOwnVersion以原版有符号十进制文本保存32位值；自身1403有效JPEG缓存保存成功后记录MAKELONG(wParam,wTag)，零版本清除自身缓存并记录0。
- Client清理83步、全目标111步构建通过。portrait-upload-confirmed五项全1，经真实编码消息入口验证确认、零忽略、高位32位值、角色隔离和0记录；本地缓存七项、自身照片六项回归通过。测试INI位于preview-photo-version/preview-photo-cache独立目录。
- 已检查out/portrait-upload-confirmed.png，三条成功提示来自测试序列中三次非零模拟确认，不是实际服务器上传结果。缓存版本现在已记录，但加载时与服务器版本比较、失败节流重查、远程玩家版本缓存及真实登录联调仍未完成。

### 中文头像文件与缓存身份的DBCS路径修正
- 发现PortraitWnd原字节级find_first_of会把GBK汉字尾字节0x5C当作反斜杠，导致合法中文JPG无法读取，中文服务器/角色缓存上下文失效。新增Common/AnsiInput.h的IsAnsiFileComponent，按CP_ACP双字节边界检查单字节非法路径字符，替换两处校验。
- 全目标增量构建通过。在实际GetACP=936环境使用U+4E57（GBK 81 5C）命名JPG、服务器及角色，真实文件列出、上传读取、FAC保存恢复和真实分隔符拒绝四项全1。上传回环两项、缓存七项回归通过。测试仅写preview-dbcs-photo目录。
- 已检查out/portrait-chinese-filename.png，中文文件成功选中并显示JPEG预览及自身照片。此修正处理中文资源/身份路径兼容，不代表头像版本比较、远程缓存或登录联调已完成。

### FAC恢复版本门槛与扩展角色字段核查
- 原版0x411700从16字节角色描述读取偏移8的照片版本并写actor+400400，版本改变置actor+400416。其他移动/动作处理也写该字段。CLIENT仍使用旧CHARDESC及FEATUREEX，不能直接把其偏移8解释为MIR3照片版本。证据out/actor-photo-version-original.c；完整角色消息适配仍待后续。
- SetCacheContext现在先读取UserPic.ini非零确认版本，再经RestoreOwnVersion验证期望版本一致后解码FAC；缺失记录/0/不匹配均清空照片并回退默认。修正上一阶段无版本记录也恢复本地未确认上传的行为。RestoreOwnVersion保留供后续服务器角色版本驱动；当前创建界面仍以本地已确认版本恢复，不声称已验证最新服务器版本。
- Client清理83步及全目标构建通过。portrait-cache-version-match/zero各五项全1，覆盖未确认、已确认、版本不符、匹配和0；缓存七项、中文文件四项回归通过。预览缓存保存测试补上模拟确认版本，符合新的恢复规则。
- 已检查out/portrait-cache-version-match.png、portrait-cache-version-zero.png。远程缓存、扩展角色协议、自动节流重查及真实服务器登录联调仍未完成。

### MIR3人物描述解码器
- 新增Common/Mir3HumanAppearance.h，对齐Godot human_appearance.hpp：16字节基础描述/21字节含扩展描述，性别0..1、衣服0..8、武器>=50回退0、头发低6位且>=30回退0、职业取高2位，坐骑扩展仅0..3有效。普通移动缺扩展保留坐骑，其他非受击缺扩展默认0。
- 区分偏移8上下文：普通描述为照片版本，SM_STRUCK/HUMAN_STRUCK为hit_source，后者绝不能覆盖照片版本；依据Godot hud_view.cpp对应消息处理及原版0x411700。扩展5字节在网络层单独编码（22+7字符），不能把整段直接当连续21字节编码。
- VS x86 C++17独立字段/边界八项检查通过；另以C++20直接包含Godot原human_appearance.hpp，对性别3种/衣服10种/武器256种/头发256种/受击2种共3,932,160组合差分，接受范围、标准化外形、职业和坐骑全部一致。测试out/test-mir3-human.cpp及compare-mir3-human.cpp。
- 本阶段仅完成协议解码基础，没有改动页面，无新增截图。尚未接原生Actor创建、移动/受击队列或完整角色渲染，不能声称远程职业/照片版本已在实际游戏生效；下一步需统一迁移旧CHARDESC/FEATUREEX消息边界，避免按长度猜测造成旧协议误判。

### MIR3专用人物受击消息接入
- 1309仅接受已有的人物及完整16字节描述，经现有人物队列更新HP、受击动作、职业与外形；偏移8仍作为攻击来源，不写照片版本。目标头像优先采用已收到的职业。旧31消息保留既有处理，普通出现/移动等共享编号尚未迁移。
- 移除ChangeFeature中强制发型2的旧调试覆盖；Client单独启用C++17以使用人物描述optional字段，服务器编译标准不变。
- 新增--preview=human-hit-server，走OnSocketMessageRecieve、ProcessPacket、CMyHero队列与目标显示。unknown/truncated/queued/health/appearance/photo/motion/portrait八项全1；截图out/human-hit-server.png已检查。截图为离线HUD，未包含世界人物渲染，不代表真实服务器联调完成。
- Client清理构建83步及全目标112步通过，diff --check通过。受击沿用旧动作与音效路径，尚未听音验证；连续受击节流及完整MIR3动作队列仍需对照Godot继续迁移。

### 连续受击血量同步修复
- 对照Godot process_local_actor_message及42EC70路径，1309不再进入旧500ms丢弃分支；31在节流期仍更新HUD和Actor血量，非致死才省略入队，致死保留入队。远程1309补入攻击来源外形查找。
- human-hit-server增加重复1309、节流31血量同步及致死31保留三项，连同原八项共11项全1。全目标增量构建通过，diff --check通过。截图out/human-hit-repeat.png。
- 本次修复消息丢失，不代表完整特殊受击已对齐：hit_variant驱动的特效、计时、1309复制普通受击、折叠计数及死亡动作仍待统一迁移。

### 使用物品成功回复音效
- 对照Godot apply_use_reply，SM_EAT_OK在清理m_stCommonItem之前调用原生PlayUseItemEffect：类型0播放SoundList样本108，类型1/2播放107，其余不播放；失败回复不调用，无待用物品的重复回复不播放。沿用PlayLobbyEffect的音效开关和音量。
- Client清理83步、全目标111步构建通过；--preview=item-use-confirmed走实际成功回复处理，consumed和duplicate_empty两项通过。107/108资源均可读，为44.1kHz单声道PCM，时长1.503/1.043秒；未进行回环录音或听感验收。
- 已检查out/item-use-confirmed.png（既有背包界面）。完整物品堆叠回复、快捷栏数量、使用提示以及真实服务器联调尚未完成；本次不宣称整个使用物品流程完成。

### 使用物品确认提示
- SM_EAT_OK有待用物品时，读取CmList1464并替换物品名，按Godot规则将CmList1409加单个数字0..9的名称归一化；使用chat_palette(2)/chat_palette(7)对应0xffffff/33023，经原生MsgAdd显示。无待用物品的重复回复不产生提示或音效。
- 原生item-use-confirmed预览覆盖消费状态、重复回复、归一化提示、前景/背景颜色及单次提示，共五项全1。增量构建和diff --check通过；已检查out/item-use-feedback.png，中文提示显示在原生聊天区域。
- 快捷栏剩余数量提示与MIR3堆叠回复尚未迁移，不以该确认提示证明物品使用端到端完成。

### 服务器物品数量更新与快捷栏同步
- 接入MIR3 SM_INVENTITEM=1313，仅接受无正文消息，按nRecog实例编号/wParam数量调用InventoryWnd::UpdateStackCount。对齐Godot update_bag_stack_count默认分支：更新非快捷栏引用的鼠标持有物品、背包物品及同名快捷栏引用；数量0保留物品，未知实例不创建物品。
- 数量增加显示CmList1707的物品名和增加量，前景11861940/背景0，并调用现有物品音效；减少/相等不重复提示。现有旧物品记录缺少完整MIR3 raw[31]，音效该参数仍沿用0，完整记录迁移后需补齐。
- --preview=item-stack-reply经过实际编码消息入口，增加/减少/未知/零保留/鼠标持有五项全1；增量构建及diff --check通过。已检查out/item-stack-reply.png，背包和快捷栏均显示30，聊天显示增加15和23的两条提示。
- 本阶段接数量回复，不代表MIR3完整物品入包/使用/出售/拆分协议已完成。

### MIR3单物品入包记录接入
- SM_ADDITEM新增134字符/100字节MIR3记录路径，依据Godot InventoryRecord：名称0..29、类型30、音效/形态31、图标36、堆叠模式61、实例62、耐久66/68、数量71。完整100字节保存在CItem，Name保留最长30字节，旧CLIENTITEMRCD仍仅接受其精确编码长度。截断MIR3记录不再误按旧格式入包。
- 新格式入包使用原生网格放置、CmList1465/1461/1463提示及真实raw[31]物品音效。快捷栏比较/提示及使用成功提示采用完整Name；数量更新同时写raw[71..72]并同步快捷栏副本。
- Client清理83步及全目标112步构建通过。item-record-reply七项全1：截断、字段、30字节名称、原始记录/耐久、16位数量同步、快捷栏查找、空名称。已检查out/item-record-reply.png，背包及快捷栏显示300，聊天保留完整30字节名称。diff --check通过。
- 目前仅SM_ADDITEM新格式入口，SM_BAGITEMS全量、删除/更新、装备记录、属性提示及布局文件仍需迁移；旧结构中的属性投影和名称长度不足尚未全面替换，不宣称完整MIR3物品系统兼容。

### 全量背包MIR3记录接入
- SM_BAGITEMS=201迁移到ReceiveBagRecords，按wSeries读取斜杠分隔记录，接受100字节MIR3记录及精确长度旧CLIENTITEMRCD。全部记录与图标尺寸/网格位置准备成功后才提交，不完整回复保留原背包；合法空列表清空背包。继续调用现有CheckItemPos恢复布局。
- 原生--preview=bag-records-reply六项全1：双记录加载、缺记录保留、缺末尾分隔符保留、超容量保留、完整替换、空列表清空。增量构建及diff --check通过；已检查out/bag-records-reply.png，两件物品数量12/34分别显示。
- 与Godot相比，当前对多余正文采用拒绝处理，避免残留记录混入。布局持久化仍使用旧字段比较，完整30字节名称匹配、快捷栏恢复及删除/更新消息继续待迁移。

### MIR3物品删除与更新
- 200/202/203的100字节记录共用ReceiveItemRecord，按实例编号和完整名称匹配。更新采用副本布局先移除再放置，未知实例不新增背包物品，无效图标/放置失败保留旧状态；删除同时清理对应背包和装备。203装备更新按Godot放到第一个空装备槽。
- 203与旧SM_DELITEMS冲突：无斜杠正文按MIR3单记录更新处理，带斜杠仍走旧批量删除。202旧记录也检查精确长度。新增/更新沿用物品音效，新增/删除显示参考CmList提示。
- --preview=item-update-reply六项全1：更新、无效图标保留、未知实例、完整名称匹配、装备重放、删除。单物品入包七项回归通过。增量构建及diff --check通过；已检查out/item-update-reply.png，最终数量60。
- 完整装备初始回复、属性提示、布局文件及真实服务器交互仍未完成，快捷栏在200/202/203保持Godot当前同样不主动重建的行为。

### MIR3装备记录初始化
- SM_SENDUSEITEMS改由StatusWnd::ReceiveEquipmentRecords处理，解析槽位/记录/序列，支持0..11槽位和100字节MIR3记录，保留精确长度旧记录兼容。按Godot局部覆盖已给槽位，空正文不清空装备；全部记录及图标校验成功再提交。移除旧固定九次循环和未检查atoi槽位索引。
- SetMir3Item增加装备上下文允许空名称参数，默认背包仍拒绝空名，与EquipmentWire和InventoryWire区别一致。对超12条或额外无效正文采用拒绝处理。
- Client清理83步及全目标109步构建通过。--preview=equipment-records-reply七项全1：载入含第11槽、完整名称/原始字段、非法槽位事务保留、截断保留、空回复保留、非法数字、局部覆盖。diff --check通过。已检查out/equipment-records-reply.png，药瓶图标是槽位验证夹具，并非真实装备资源验收。
- 装备属性提示、耐久回复的MIR3字段映射、完整角色外观及真实服务器联调仍待继续。

### 装备耐久回复与耗尽提示
- SM_DURACHANGE通过StatusWnd::UpdateDurability更新有效槽位：当前耐久取recognition低WORD，最大耐久取wTag，同步raw[66..69]。提示判断使用完整有符号recognition除单位并加0.5截断，类型54/55单位100，其余1000，对齐Godot EquipmentDurability。CmList1470使用完整名称、白字/33023背景。
- 修复SM_DROPITEM_FAIL缺break误进入耐久分支的问题。未知/空装备槽不改数据。
- --preview=equipment-durability九项全1，覆盖普通/特殊阈值、原始字节、负值、超过WORD范围、空/越界槽位及丢弃失败不误改耐久。增量构建与diff --check通过；已检查out/equipment-durability.png，三条耗尽提示来自三个边界测试。
- 当前证明回复与提示接线，不代表装备属性悬浮框、真实耐久消耗与服务器流程已全部验收。

### MIR3完整名称布局持久化
- 核查Godot参考当前没有完整物品属性悬浮层，不能据其虚构属性格式；该部分后续需继续原版取证。本轮先修复已明确的布局链：LayoutIdentity对MIR3物品保存完整100字节记录，LayoutName按完整30字节名称匹配，旧物品维持原格式投影。
- --preview=item-layout-full-name六项全1，使用前29字节相同、末字节不同的两个名称，重排服务器列表后恢复原位置，快捷栏准确引用对应物品；保留新数量99而非磁盘5，文件原始记录逐字节一致。
- 增量构建及diff --check通过；旧inventory-panel布局回归通过。已检查out/item-layout-full-name.png，预览只写preview-item-layout-full.itms。完整属性悬浮层与真实登录/布局恢复仍待联调。

### 原版药品悬浮属性框
- 原IDA会话e3ded018失效，经技能open.ps1重开同一Mir3-unpacked-analysis.exe.i64，当前会话6f444c1e。定位4423F0属性分派：武器444050、防具4424E0、饰品444F40、特殊53为443490、普通447E70；448770绘制。证据out/item-tooltip-{dispatch,general,weapon,armor,render}.c和item-tooltip-original.c。
- 原物品对象记录起点this+4；普通物品标题CmList2515+名称/前缀raw73，栗子数字名称用2510；类型0药品HP/MP来自WORD40/42，CmList1058/1059。重量raw32，堆叠2重量count/10，堆叠>2重量count*raw32；数量WORD71，CmList2501/2503。标题64255，其余白字。
- CItem对MIR3类型0走PotionHint/ShowPotionHint原生GDI/DirectDraw提示：15像素行距，边距5/底部2，原背景RGB100/100/50 alpha230经现有预乘绘制为90/90/45，边框3315350，右侧超800整体左移。字体沿用原生界面宋体9；尚未与运行中的原版逐像素核对。
- Client清理83步、全目标109步构建通过；普通/堆叠两预览各三项全1，覆盖HP/MP、stack2重量和数量、stack3重量。已检查out/potion-hint.png和potion-stack-hint.png。仅药品类型0已迁移，其他类型属性、商店附带图标提示及完整原版视觉验收继续待处理。

### 普通物品悬浮属性扩展
- 根据原版447E70将PotionHint扩展为GeneralHint，复用ShowGeneralHint绘制。普通分派覆盖类型3(shape13/15)等级要求、4/51职业及等级、25使用次数(耐久/100四舍五入)、30耐久/低耐久色、40品质、42固定说明、43纯度、60属性范围及有符号raw86加成。保留药品、堆叠重量与前缀标题。武器/防具/饰品/53仍留原处理待后续迁移。
- 要求字段：类型3等级raw40；技能书职业raw31、等级WORD38，职业/等级不符为255红字；次数/耐久当前WORD66、最大WORD68，shop参数可改用WORD38但商店悬浮尚未接线。类型60基本范围raw40/41，加成raw86>0采用13172630，否则白字。
- 增量构建和diff --check通过；book/charm/stone预览七项全1，覆盖等级边界、职业、次数舍入、低耐久色、正/负加成；药品三项回归通过。已检查out/book-hint.png、charm-hint.png、stone-hint.png。图标为离线夹具，不代表物品图标对应验收；其余装备属性与原版逐像素核对继续待完成。

### 防具原版悬浮属性
- 按原版4424E0完整移植类型10/11属性行至ArmorHint，复用原生提示绘制。包括标题、耐久/低耐久色、重量、破坏/防御/魔防/魔法范围及有符号加成、HP/MP附加、攻击元素、强弱元素位图叠加、原版额外元素行及12类穿戴条件。保持原版含重复元素行的显示顺序，不自行合并。
- raw映射：基础范围40/42/44/47；加成86/87/88/89；附加HP/MP94/95；攻击元素49+WORD50与97/98；强弱52/53与91/92；要求54/55。等级>100按减100显示及满足时51200绿色，普通不满足255红色。防具需求的角色元素采用现有StatusWnd攻击元素数组，该数组实际服务器数据源仍待完整迁移。
- 增量构建、diff --check通过；普通与扩展防具预览检查基础范围/加成色、低耐久、等级边界、特殊等级、组合元素要求，扩展另核对14行及强弱x3/附加范围，七项全1。已检查out/armor-hint.png和armor-elements-hint.png。图标为夹具，未宣称完整资源/原版逐像素一致。
- 武器、饰品、类型53与商店带图标提示仍需继续迁移。

### 武器原版悬浮属性
- 按444050接入类型5/6/54/55/56的WeaponHint；保留raw96的(*)标题标记、前缀空格、普通耐久与54/55的百单位数量、重量、破坏/魔法/攻击元素、强度、准确、速度、幸运/诅咒和穿戴要求。与防具抽出RequirementHint复用，未改变要求规则。
- 武器加成与防具位置不同：破坏raw86、魔法87、强度93、准确91、速度92、幸运89、诅咒90；强度基础signed raw34，准确raw41>=100时用100-raw41，速度超过10用total-10并保留原版(+bonus-10)格式。
- 从IDA当前6f444c1e读取4B52A4等级阈值0/3/6/9/13/17/21/25/30/35/40/45/51/57/63/69/76；4B5214经验表等于阈值步长*60000，4AF464倍率100。raw70和WORD71组成经验，达到76显示MAX，否则保留两位百分比。
- 增量构建及diff --check通过；武器八项全1（标题、双攻击范围、特殊速度、负准确、经验、MAX、特殊数量），防具七项回归通过。首次2秒预览等待不足，改4秒后报告/截图正常；后续不要以短观察窗口误判失败。已检查out/weapon-hint.png与weapon-special-hint.png。饰品、类型53及原版逐像素验收仍待后续。

### 头盔与类型26原版属性提示
- 按原版444F40接入15/26与默认57分支，新增AccessoryHint；名称、耐久、重量抽为EquipmentBaseHint与防具共享。15/26省略零防御/魔防，攻击元素保持原分支，强弱元素普通x1、叠加x2；魔法范围在元素之后，末尾复用穿戴要求。57仅公共破坏/魔法及要求，不进入头盔元素分支。
- 增量构建成功；helmet-hint与accessory-26-hint运行截图已检查，六项检查omitted/defense/elements/order/shared/variant全1；armor-hint七项回归全1。截图out/helmet-hint.png、out/accessory-26-hint.png。每次预览等待4秒，仅终止自身进程。
- 饰品19..24、特殊53、装备窗口悬浮接线、商店提示与原版逐像素验收仍待完成；本轮未宣称服务器完整流程通过。

### 人物装备页属性悬浮接线
- ShowNativeStatus末尾接回ShowEquipItemState，采用UpdatePointer提供的当前悬停窗口判定和坐标；手持物品时不显示装备属性。复用各物品原生属性分派与绘制，沿用CLIENT原有left+37/top+288锚点，该锚点尚待与MIR3运行画面对照。
- 增量构建与diff --check通过，equipment-hover-hint预览已实际截图检查，装备槽5的类型26属性在人物页显示。截图out/equipment-hover-hint.png；夹具图标不代表正式物品资源对应关系。

### 类型19项链原版属性
- 按out/item-tooltip-original.c原版444F40的case0x13及LABEL98接入AccessoryHint。魔法躲避raw41+signed86以十倍百分比显示，诅咒raw42，幸运raw43+signed87；基础/附加攻击元素49/50/97/98及独立91/92，强弱52/53仅1..6显示x1，末尾魔法与要求共享原路径。保留负加成仅参与是否显示判定、不减基础显示值的原版规则。
- 增量构建、diff --check通过；necklace-hint三项stats/elements/boundary全1，覆盖百分比、加成色、元素组合、负加成和元素7不显示。已检查out/necklace-hint.png。20..24与53仍待迁移，尚未完成原版逐像素验收。

### 类型20准确敏捷饰品属性
- 对照444F40 case0x14，类型20准确raw41+signed86(Cm1053)、敏捷raw43+signed87(Cm1054)，均整数；与19共享相同的攻击元素及强弱x1规则，19仍独立保留百分比躲避/诅咒/幸运。复用现有value辅助避免重复元素代码。
- 增量构建与diff --check通过；accuracy-necklace-hint与necklace-hint三项stats/elements/boundary各全1，覆盖正负加成、行数顺序、元素组合与范围。已检查out/accuracy-necklace-hint.png。类型21..24及53仍未迁移，完整GUI/音效/服务器流程继续。

### 饰品完整属性分支统一补齐
- 按444F40补齐21/22/23/24；至此其15/19..24/26/57普通提示分支均走原生AccessoryHint。21为体力/魔法恢复百分比，22防御/魔防范围，23回避中毒/中毒恢复百分比(无加成保留尾空格)，21/23在元素后追加独立正负速度；24准确/敏捷及强弱位图x1/x2。比较模式a3标记仍未接入，不宣称整体属性系统完成。
- 新AccessoryHintIds.h集中命名该函数物品类型、记录偏移、文案、颜色；范围、标量、基础攻击元素、强弱位图分别复用小型局部函数。常量名称按CmList实显修正为中毒回避/恢复，避免误称生命恢复。
- 按用户新要求合并21..24为一个功能批次统一验证：增量构建通过，四个新增预览各五项scalar/elements/speed/signed/trailing全1；helmet/accessory26/necklace/accuracy-necklace回归通过。四张截图out/{vital-accessory,defense-accessory,recovery-accessory,accuracy-bracelet}-hint.png已检查。夹具0/0耐久用于呈现低耐久色，图标不作资源对应验收。
- 用户AGENTS.md修改不纳入提交；仅本轮代码diff检查。类型53、商店带图标提示、原版逐像素校准和真实服务器流程继续待完成。

### 进行中：特殊53与商店提示
- IDA旧会话worker不可达，经技能open.ps1重新打开原分析库，当前38a4174b；py_eval导出443490到out/item-tooltip-special.c(14046字符，stderr空)。
- 类型53已写入AccessoryHint独立分支但尚未构建/运行验证、未提交：共同标题耐久重量；WORD44+signed86(Cm2502)、byte47+signed87(Cm1063)、byte48+signed88(Cm1064)；强元素位图x1/x2，HP94/MP95正值*10，弱元素位图，末尾RequirementHint。复用elementMasks(first,end)保持行顺序；MIR3所有模式均可走ShowGeneralHint，删除已无分支的空switch。
- 下一步先加入53的边界/行序预览，再连同商店提示完整功能统一测试提交。StoreWnd::Offer当前只保留名称图标价格ID数量堆叠，丢弃完整记录；ShowStoreWnd未绘制悬浮提示，需先保留完整记录再接线，不能仅调用现有旧属性。用户要求大功能批次测试/提交。

### 特殊装备与商店仓库属性提示完成接线
- 53分支按443490完成舒适(WORD44+signed86)、负重能力(byte47+signed87)、手负重能力(byte48+signed88)、强元素、HP/MP附加、弱元素、要求的原版行序；命名常量按实际CmList含义修正。删除MIR3分派的空switch。
- StoreWnd::Offer保留完整记录，解析允许完整30字节名称；商店/仓库明细悬停创建CItem并复用ShowGeneralHint，模态框、编辑器、手持物品、非当前悬停窗口和加载状态不显示。商店shop=true最大耐久取38，仓库false取68。无记录的旧摘要夹具不伪造属性。
- ShowGeneralHint增加shop/showIcon参数，按448770扩展图标宽度并居中放置，文本右移；沿用现有原生DrawSelectionFrame。商品提示锚点暂为格子左上角，图标原版染色/530裁剪及空行高度细节、完整逐像素对照仍需校准。
- Client清理83步、全目标109步构建通过；特殊/商店/仓库三预览七项attributes/order/maximum/signed/parsed/rejected/full_name全1，旧shop-offers和storage-list已运行回归。已检查out/special-equipment-hint.png、shop-item-hint.png、storage-item-hint.png。截图证明商店2/10与仓库2/5及带图标属性；图标为离线夹具。

### 物品提示位置与图标规则原版校准
- IDA38a4174b导出448770全部9个调用方，证据out/tooltip-caller-0x*.c；440B30经440220/440130确认背包6列38格映射，提示为鼠标+10/+10。4784F0经4786A0确认商店/仓库4列12格及页码，提示同为鼠标+10/+10，仅模式10/11启用图标；之前所有商品显示图标的接线已纠正。
- ItemHintStyle.h集中指针偏移、模式、边距、行高和裁剪尺寸；ShowGeneralHint按448770仅非空文字计高，保持绘制行推进，图标裁剪800x530。库存提示从格子锚点改跟随鼠标，商店/仓库亦同步。装备页旧锚点仍待确认，不能仅凭12槽调用方推断所属页面。
- 增量构建通过；特殊、商店、仓库、防具商品四种预览七项属性/解析回归全1，已检查对应PNG；防具夹具独立设置正常破坏范围，不复用特殊容量导致44-3。图标染色仍待48FA0F核查，未宣称完整原版像素一致。

### 进行中：原版提示图标分区染色与提亮
- 38a4174b导出48FA0F完整函数out/item-icon-original.c，HexRays丢失x87运算，进一步读取汇编确认4AFA60系数1.5；C1通道floor(c+1.5*c*c/max)，C2/C3先按颜色通道乘积/max截断再相同提亮，最后clamp。48C2DB确认RGB565打包；提示调用448770以raw70/71/72为C2颜色，C3白色。与Godot wil_pixels.hpp普通分区染色共用基础含义，但原版此路径额外非线性提亮。
- 已写入LegacyPanel.h独立SelectionBlend::ItemHighlight/HighlightItemPixel，默认参数保持其他调用不变；仅ShowGeneralHint带图标路径使用。尚未构建测试/截图/提交，下一步应以原机器码或充分像素用例验证计算、找含C2/C3的真实资源验证，之后再提交这一完整绘制功能。

### 提示图标分区染色与提亮验证完成
- out/verify-item-icon.py通过Unicorn运行原48FA0F及其浮点转换机器码，模拟DirectDraw Lock/Unlock，C1/C2/C3各穷举65536种像素、六种染色(黑白红绿蓝混合)，共1179648结果全部符合HighlightItemPixel整数计算，out/item-icon-machine-check.txt all_match=1。机器码快照out/item-icon-code.bin由IDA38a4174b原库只读导出。
- 原生渲染加入独立ItemHighlight模式，C1白色、C2第一染色、C3第二染色；只有提示图标启用，其余现有Blend默认参数不变。RGB颜色来自记录70/71/72。
- 增量构建、代码diff检查通过；扫描真实Inventory.wil找到含C2的frame372，红蓝预览已检查(out/shop-red-armor-hint.png、shop-blue-armor-hint.png)。两图差异仅(335,366)-(357,399)图标区域，像素计数红图红通道、蓝图蓝通道；旧shop-item-hint回归运行。图片缩略显示不明显时以实际像素数据确认，不能仅凭肉眼判成同色。
- 该批完成提示图标绘制，人物装备锚点、其余GUI完整逐像素/交互验收和真实服务器流程继续。

### 进行中：装备与交易提示统一接线
- 依据Godot client/docs/evidence/client-module-inventory.md及window-dispatch-instructions.json确认436D14对象2AB4C(人物装备)调用4761E0，使用槽绝对left/top+15；此前47B0E0实际为ID7行会成员窗口，不能用于人物装备。已将StatusWnd固定left+37/top+288改为对应装备槽+15。
- 交易41D640使用鼠标+10，当前ExchangeWnd仅存摘要、无完整属性记录，现新增Offer.record及SetOfferRecord，OfferAt支持双方独立滚动、保留默认本方以兼容取回操作；ShowExchangeWnd双方悬停复用原生属性。手持/待取回/模态/非顶层窗口均隐藏。
- 以上修改尚未编译测试或提交。下一步加入双方、多格/滚动、完整名称/字段保持、模态与取回遮挡预览，统一构建验证后提交。检查发现当前CLIENT交易服务端回复没有对应ExchangeWnd数据写入，SetOfferRecord仅准备GUI数据入口，后续真实协议接线仍需实现，勿宣称已有实服交易闭环。

### 装备与交易提示验证完成
- Client清理83步、全目标109步构建通过。trade-local/remote/held三预览八项local/remote/name/retained/multi/scroll/edge/invalid全1；原trade-list十项布局/交互回归全1，已检查双方提示与取回隐藏截图。
- 装备首次截图未见提示，加入预览状态报告后复核window=status=1、slot5、held0、present1；新截图out/equipment-hover-audited.png正确显示在槽位+15。额外两次预览BMP SHA256均5595CE706CF0BE2542D57EC000C9FC5BC5CD8DE5C772D77EC8EDD02BA3C34755；首次差异原因未确定，不宣称已修复未知根因。报告equipment-hover-check.txt保留供复核。
- 新入口SetOfferRecord仅供完整物品GUI数据使用，尚无真实交易回复调用；当前证明GUI数据、定位和绘制，不代表实服交易闭环。

### 进行中：信件完整窗口
- Godot参考分支无mail/letter实现，不能声称沿用完成页面；按原版458640确认0收件/1阅读/2写信/3送礼/4发件。38a4174b导出out/mail-original-{0x457f70,0x457ab0,0x458640,0x4586f0,0x458c20,0x459490,0x459670,0x459b80}.c。457540构造HexRays返回None，已转查实际控件初始化457F70。
- 新MailLayout.h记录17按钮的原始帧/文案/初始坐标、五状态、七行24高列表、编辑4000字节、各框位置；仅定义，尚未运行/截图/提交。注意绘制各状态会重定位Refresh/Delete等按钮，不能始终套用初始化坐标；按钮帧同normal/pressed是原始初始化，交互后状态变化仍待跟踪。
- 编辑器分别this1618/1636两个Win32 EDIT，CreateWindow样式-2147483580，body Compose(21,84)-(394,225)、Gift(21,84)-(214,225)，Recipient(68,62)-(216,77)；关闭键400,267；页签x183/228/273/318/363,y14。文字颜色Title16309498/Text16448250。
- 下一步恢复页面基底资源及五状态绘制、点击处理/窗口入口和音效，完整功能后统一构建/测试/截图/中文提交；不按单页拆提交。当前尚无新页面完成。

### 信件窗口实现继续（未完成/未提交）
- 原HUD初始化4365EC..43660F确认通过457F50创建窗口ID21，资源0x7A8=1960、尺寸0x1B8x0x136=440x310、原点0,0。之前457F70无直接xref因457F50入口包装，不能据此误判缺调用。
- 新MailWnd.h/.cpp定义CMailWnd五状态、消息列表/草稿数据、可见按钮及位置、背景和GDI文字绘制；尚未加入CMake/Interface，尚未构建测试。页签/编辑框/滚动/选中与标记绘制/发送请求/音效/窗口入口仍待完成。该骨架不算页面完成，不能用之宣称可用信件功能。
- 补充导出out/mail-event-0x*.c(45AC50/45B420/45B6B0等)，45A110无法反编译需反汇编。按钮控制下标264Delete、444Refresh、624Outbox、804Gift、984Compose、1164Read、1344Inbox；45AC50是鼠标释放/页签切换路径，按调用时机后续核对。收件重定位Refresh286/Delete326，发件Refresh246/Cancel286/Delete326，y256。
- 邮件布局初始化直接确认；列表发送方43..137、主题141..362,y65+24row；Read2010(1,51)、Compose2030(1,51)、Gift2050(1,50)、Outbox2070(1,51)。部分原版顶栏选中图标y使用this[6]+14(疑似原版x/y误用)，目前骨架尚未处理，不自行声称像素一致。

### 信件窗口继续实现（未提交，尚未统一测试）
- 按用户要求以完整功能作为测试/提交单位；本轮未启动构建或提交。
- MailWnd 已接入 CMake、StdAfx、Interface 生命周期/窗口顺序/点击拖动/滚轮/双击/Escape，GameProc 编辑框着色和预览 EDIT 合成。Mail 窗口 ID 使用命名常量 `_WND_ID_MAIL=21`。
- 增加收件/发件选择区分、正文 loaded 标志（空正文有效）、上/下一封、回复、复选标记、列表图标、滚动条、成对点击捕获、缩放字体。
- 未完成：真实入口快捷键需原版证据；正文按原版 12px 行高分行/滚动；送礼物品/金币完整交互；按钮悬浮提示；原生 EDIT 遮挡和焦点处理；待发动作数据快照；五页预览、统一构建回归、截图、中文提交。当前 pendingAction 仅界面请求占位，不能宣称邮件协议已接通。
- 原版依据见 out/mail-original-* 与 out/mail-event-*。注意 Read 顶部标签直接切换读取页，双击列表才请求/打开具体邮件。滚动范围原版允许第一行到最后一条（不是 count-visibleRows）。
- 编码补充：Client/StdAfx.h、Client/CMakeLists.txt 是 UTF-8；Interface/GameProc/Mir2Ex/Define 等仍为 CP936。编辑前逐文件识别，保持原有换行。

### 信件五页 GUI 批次（2026-11-17）
- 新增 MailWnd/MailLayout，接入原生 CGameWnd/DirectDraw、Interface 窗口列表、鼠标捕获/拖动/滚轮/双击、按钮提示及关闭；无 Godot 运行时。原版五状态、17按钮资源、背景、七行列表/图标/复选框，写信/送礼使用 Win32 EDIT。
- 正文依据 45BC70/45BEF0：355px 宽，CR 段落各至多11行；显示11行、12px行距、四向黑描边。收发件选择和读取来源分开；loaded 标识支持空正文。写信/送礼草稿独立；取消清空但停留本页（45A909/45A975）；关闭清空正文（45C450）。
- EDIT 跟随窗口缩放/移动，Tab切换、Esc关闭，其他窗口/模态覆盖时隐藏并绘制静态内容。字体宋体9号。预览发现高DPI窗口再缩回800x600会导致编辑框字形有采样差异，邮件预览固定800x600实际客户区，保留真实应用缩放逻辑。
- 送礼选择保留完整100字节物品记录并返回背包（45AAD3..45AB32），不提前扣物品/金币；金额输入复用原生对话框，使用命名 action=13，校验余额/数字；按钮105.wav、放入物品调用已有物品音效。未做音频回放听测。
- 所有网络动作保留带消息ID/收发方向/收件人/正文/物品/金币的不可变 Request；真实收发、删除、撤回协议适配仍未接通，不能把离线GUI检查当作服务器成功。原版未发现独立邮件快捷键，未擅自添加；当前由 WindowActivate(_WND_ID_MAIL) 和预览进入，游戏实际入口仍需随服务器/NPC流程确认。
- IDA 会话已换 d0afb8c0；新增证据 out/mail-more-*.c、mail-press.asm、mail-entry-refs.asm。45A110仍无法反编译，使用反汇编核对取消/送礼操作。
- 构建：clean Client 84/84，通过；随后 Client/GameSvr/LoginSvr/DBSvr/LoginGate/SelGate/GameGate 全部通过。out/build-mail-review.log、build-mail-final.log、build-mail-cleanup.log。
- 统一交互检查：out/mud3-01/mail-gui-check.txt 的15项全1；mail-gift-check.txt 的4项全1。覆盖窗口路由、编辑框/Tab/Esc、独立草稿、取消、请求快照、发件空正文、前后封/正文滚动/列表末端、复选、遮挡、物品保留、金币校验。
- 预览编号154..161（Inbox/Read/Compose/Gift/Outbox/Checks/Gold/GiftItem），下一空闲162。截图 out/mail-{inbox,read,compose,gift,outbox,gift-item,gift-gold}-final.png，均真实原生渲染。
- 尚需后续原版精校：页签选中重复绘制的x/y疑似原版bug；正文分段排版与原版482240逐像素比较；金币附件tooltip和送礼大图裁切/提示位置；服务端收发/删除/撤回及实际入口。此批完成页面/编辑和交互框架，不能宣称整个客户端GUI或邮件端到端完成。

### 信件排版、滚动与附件提示校准（2026-11-17）
- IDA 原会话 worker 不可达；通过 bundled open.ps1 重新打开为 ed4a8612。导出 out/mail-layout-0x482240.c、0x41fed0.c、0x41fe80.asm、0x420200.asm；420180 可反编译确认拖动比例按完整轨道长度计算。
- 482240 是先测量已累积字节宽度再追加字节，因此行可略超355px；原先预测下一个完整字符导致提前换行。现保留原版高字节配对及半字符回退、每CR段至多11行；空CR段不添加空白行；取消额外355px裁切，以保留原版末字。正文仍采用当前原生GDI字体，不声称所有字节组合已做机器码穷举。
- 滚动条空白处按7/count翻页，按滑块不跳动，拖动以完整轨道作比例；使用原版float中间结果，例如10项第一下页结果6，已在原生运行中验证。
- 金币附件提示按458C20固定90x20、CmList1050、颜色64250/边框3315350；礼物图标绘制与tooltip共享GiftBounds，提示锚点为图标+25,+22。保留459670超高图标绝对y=78、裁130的原版行为。
- 合并验证：build-mail-fidelity-final.log全部目标构建通过；mail-gui-check15项、mail-gift-check4项、mail-scroll-check3项全1。图像：out/mail-body-boundary.png、mail-gold-hint.png、mail-gift-hint.png，已逐张检查。
- 预览新增162正文边界、163金币提示、164礼物提示，下一空闲165。邮件实际入口及协议仍待接入；页签重复绘制坐标疑似原版bug、图标边框的混合顺序等尚不能称逐像素完全一致。

### 寄售完整功能批次启动（未提交）
- 当前工作树起点7f42b05清洁；上一轮属于已验证并提交的进展。本轮转向缺失窗口19/20，不把已有邮件界面当作整个GUI完成。
- Godot br_godot_test_261115仅client-module-inventory有寄售证据，无已实现寄售页面；按MIR3实现原生控件。IDA会话ed4a8612仍可用。
- 导出out/consignment-{0x453f90,0x454270,0x454400,0x454410,0x4546f0,0x456120,0x456590,0x456770,0x4567a0,0x456960,0x4572b0}.c。
- 初始化43659C：19号1890背景，x60y20，720x440；4365C4：20号1910背景，x0y10，408x310。已新建ConsignmentLayout.h命名资源/标题/按钮/矩形。尚未实现或接入窗口，不算页面完成；尚未测试、截图、提交。
- 查询页13行，绘制阶段改为(14,84)-(468,102)，行距19；查询框最终(268,363)-(405,380)/(268,393)-(405,410)。按钮9个，资源1221/1222、1908/1906/1904/1900/1898/1896/1894。
- 登记页为多行描述EDIT，4000字节，(12,83)-(230,195)；价格独立点击区域(19,235)-(172,282)，文本61,250..150,265；物品区247,47..395,203。开启窗口20会带出背包，关闭恢复聊天编辑框。
- 下一步：逐分支读取4546F0列表列/选择详情绘制、输入事件与搜索/分类；查登记456960后续鼠标/价格确认；完成两窗口与确认/价格输入后统一构建回归和截图，不零散提交布局头文件。

### 寄售查询、登记及确认界面完成（2026-11-17）
- 本批以原版19/20号窗口实现 CConsignmentWnd，复用客户端原生 CGameWnd、DirectDraw 和 Win32 EDIT；没有引入 Godot 运行时。窗口、按钮、消息和资源集中在 ConsignmentLayout.h。
- 查询页13行，字段按 CmList4409..4412为服务器/物品名称/价格/出售人；右上显示卖家与日期。搜索双EDIT、翻页/刷新请求、选择详情、等待动画、物品提示和拖动/关闭接入统一窗口路由。
- 登记页自动带出背包，描述原生多行EDIT；价格输入、购买确认、取消登记确认、登记确认复用原生消息框。选择物品保留完整记录并返还背包，确认请求使用快照，离线不扣物品或金币。按钮105.wav、物品放入音效已接入，未做人工听测。
- 描述换行依48C03A复刻字节宽度、双字节回退和段落逻辑。关闭查询窗口依456120清空列表、选择、页码和等待状态。登记关闭清正文和物品/价格，取消选择保留描述。
- 证据 out/consignment-0x*.c、consignment-0x455360.asm、consignment-wrap.c。455BA0/455BD0为选择行增减，尚未确认键盘/滚轮调用入口；未擅自映射快捷键。价格上限当前为命名有符号32位上限，协议接入时需确认原版范围。
- 完成后统一验证：clean Client85/85通过，全部七目标通过；最终增量 build-consignment-final.log通过。寄售16项检查全1；邮件15+4+3项回归全1。已审查复用实现与资源命名。
- 原生预览165..173（空列表、列表、登记空物品、登记物品、价格、购买、取消登记、登记确认、检查），下一空闲174。out/consignment-{empty,list,registration,item,price,buy,cancel,confirm}-final.png共8张已逐张检查；预览保持800x600实际客户区。
- 边界：真实NPC/服务器打开入口及寄售协议仍未接入，Request仅为待发送操作；不可称购买/登记端到端成功。原版共用聊天EDIT而当前独立搜索EDIT，图标混合顺序、全部逐像素对照仍待精校。整体GUI/登录与服务器目标继续进行。

### 18号窗口恢复进展（未完成、未提交）
- 上一批寄售已提交f9d9c67且工作树起点清洁，本轮恢复剩余窗口18。原版具有攻击/等待/移动和装填提示，判断为器械控制窗口，正式标题仍未发现。
- 完整实现依据保存Client/docs/window18-evidence.md；导出out/extension-functions.txt、extension-map.txt、extension-dispatch.c、extension-window-toggle.c。IDA ed4a8612当前可用。
- 已确认背景1590、100/100位置、194x285尺寸、4按钮最终坐标，范围样式、地图非等比缩放和1120/1121/1122操作。1352是投射物特效，不是UI状态；1350/1351更新两数值和对象，1353关闭。
- 尚需追踪：4304装填动作112后续、310/311物品字段语义、入口和两数值含义、颜色DWORD0x64C8F8/0x6464C8、46EBA0不可达分支的汇编。然后实现整套原生界面与对话框，统一验证和截图后提交。本轮无新增完成页面，不展示伪造截图，不做碎片提交。

- 继续核对：43DDE0查询P键，HUD+0x72BC非0才切换18；HUD+0x67F8按钮也可切换。46E987 push offset 0x64C8F8证明颜色是立即数，并非读取该地址。46EC17..46EC54确认射程分支内重复内框检测确实不可达。
- 新增未接线SiegeLayout.h：完整窗口/按钮布局与命名资源/消息、原版float坐标映射及内外射程命中。尚未完整功能，未启动碎片测试/提交。下一步实现窗口和地图缩放/标记；全局装填动作112未在420000..425000反编译的case中找到，应追踪消息框输入消费者而非继续猜相邻packet函数。

- 已实现CSiegeWnd原生皮肤/数值/按钮/射程、按下选目标、按钮释放配对、拖动、P键状态限制及窗口管理接线。攻击/等待/移动保留Request快照，尚未发包。类布局已变，完整功能验证前需clean Client。
- CMinimap新增SiegeSource/ShowSiege复用已解码像素。45E370证据补充：普通地图源180x130，全地图256x256，再输出180x150；避免误用统一256裁剪。当前只绘制地图底图，动态标记和装填对话框待补；未启动构建、测试或提交。
- 下一步：完成装填消费者追踪、标记与预览174起，再统一构建/回归/截图。新增文件SiegeWnd.h/.cpp和SiegeLayout.h尚未提交，代码未编译，不把接线当功能验证。

- 装填消费者已找到：420A10经Windows消息2030→4277D0→4256B0的case字符'p'(112)。导出out/extension-load-consumer.c，关键425CDE..425D58。空输入全量，否则atoi，超过持有count或<=0不发送。47C390为1123、recognition makeIndex、param1数量、正文物品名；4259C0归还背包并清持有。
- 310/311为commonItem+64，对应item+36的wLooks，已命名FirstAmmoIcon/SecondAmmoIcon。新增原生LoadDialog=18（与窗口ID不同命名空间用途），接入OpenNameDialog/FinishNotice/OnMsgInputted；Request记录完整物品和数量，归回背包但未发送服务器。取消保留持有。
- 尚未构建；下一步动态标记/预览174起及整批验证，检查BeginLoad优先于按钮动作符合46EA50。

- 已补地图标记与目标1599绘制，尚需预览校验颜色、边界和原版邻近过滤。当前仍未构建，不能称窗口完成；下轮先检查新增代码，再增加174起预览与交互检查完成整批验证。

### 器械原生GUI批次验证完成（2026-11-17）
- 完整功能统一构建：clean Client因Minimap像素表达式缺括号失败一次，改为分通道计算后build-siege-final.log所有七目标通过；皮肤定位/预览物品图库修正后build-siege-review.log通过。
- 皮肤1590有额外透明画布，绘制按194x285窗口居中，第一次截图错位已修正。装填测试归包失败来自预览未加载Inventory.wil，已修复fixture。
- siege-gui-check10项全1：窗口路由、射程外圈选择/内圈排除、攻击快照、移动清目标、数量弹框、取消保留、数量边界、空输入全部归包、P键关闭。寄售16项、邮件15+4+3项回归全1。
- 新预览174=siege、175=siege-load、176=siege-checks；下一空闲177。out/siege.png、out/siege-load.png已检查，分别为窗口和装填对话框真实截图。按钮音效接入，未人工听测。
- 提交前已审查复用小地图像素、命名资源/消息与原生弹框。范围：GUI离线功能完成，真实1350/1351/1353接收、场景对象位置/HUD器械按钮以及1120..1123发包仍待接线；两数值含义、标记邻近过滤/透明度和原版逐像素精校仍未全部确认，不把GUI检查当作实服攻城成功。

### 器械实际状态接线（进行中、未提交）
- 970febb之后干净工作树开始。新增GameProc消息分派：1350控制（recognition低/高WORD两数值，wParam样式，正文首TAB后对象ID）、1351状态、1353清控制并关闭。1350不自动打开窗口，符合原版。
- 4280E0查找的是独立20字节对象位置列表，不是Actor列表。4282C0的1312以recognition为ID、wParam/wTag为坐标、wSeries为类别并更新时间戳。当前新增命名ObjectPositionMessage=1312和CSiegeWnd位置缓存，支持位置先于/后于控制，换图清缓存。真实原版列表老化删除规则及HUD按钮仍待核对。
- 本批尚未构建/测试/提交，CSiegeWnd增加std::map成员，下一次统一验证必须clean Client。下一步补HUD控制入口与状态顺序/换图/释放回归；不要把上批离线10项当作新分派已验证。

- HUD入口证据：435820创建1570/1572、CmList2645，x=757，y=HUD顶-20；HUD顶按601-1160资源高度求值，避免猜450/430。438000仅控制标志1绘制；43CCE3点击切换窗口18。
- 已加入CSiegeWnd EntryBounds/ShowEntry与按下释放配对，主界面绘制窗口列表前画入口，遮挡/模态时不响应。接口类新增entryPressed；仍未整批构建测试。下一步补入口提示、服务器模拟消息顺序和HUD点击预览，核对原版对象缓存老化后统一验证提交。

### 器械控制状态与HUD入口验证（2026-11-17）
- 完成1312对象位置缓存、1350控制、1351状态、1353释放的实际GameProc分派，以及换图清缓存；P键已有控制条件，HUD1570/1572按钮与CmList2645提示接入。
- clean Client86/86通过；build-siege-state-all.log七目标通过。siege-state-check9项全1，既有siege-gui-check10项全1。测试直接调用状态处理函数及窗口鼠标路由，并未注入真实加密网络包，也未运行实服器械流程。
- 截图out/siege-state.png含右下HUD器械入口；out/siege-load-state.png装填框；out/siege-checks-state.png提示状态。主窗口截图已核对，提示截图暴露底边文字被裁切，后续需统一核对ButtonHint边界。
- 静态1312原版还保存类别和更新时间；当前窗口位置缓存只保留ID/坐标，原版过期删除规则尚未找到，不能宣称动态列表生命周期完全等价。攻击/移动/等待/装填仍仅记录Request，后续网络动作接线继续。

### 提示截图时机校正（2026-11-17）
- 上轮将器械HUD提示首帧误判为裁切；核对ShowButtonHint确认原版展开动画，hintHeight从6逐帧增至文字高度。运行时动画正常，问题是预览首次Render即保存BMP。
- 现仅调整CHudPreviewProcess截图时机：存在按钮提示且仍展开时延迟保存，直到展开完成；无提示/模态预览照常。首次编译因Windows interface宏冲突失败，变量改为previewUi后build-hint-capture.log全部目标通过。
- 三种原生预览验证：siege-checks、hud-button-hint、inventory-hint。panel-hint-check7项、hud-button-hint-check9项全1，保留首帧/展开动画测试；截图out/{siege-checks,hud-button-hint,inventory-hint}-expanded.png已检查。
- 展开后的CmList2645明确为“投石车窗口(Ctrl+P, P)”，窗口18可正式命名投石车；先前“器械控制”是证据不足时暂称。当前代码P入口已实现，Ctrl修饰未额外拦截，仍需原版完整按键优先级核对。
- 本次为预览验收修正，暂不做单独小提交，随下一完整GUI/流程批次提交；当前Mir2Ex.cpp和AI2AI.md有未提交修改。

### 邮件通知和回复接线启动
- 新增Client/docs/mail-wire-evidence.md保存证据。1358仅点亮HUD邮件标志；邮件按钮2072/2074与投石车入口同位置，后绘制，但43C130释放未找到邮件开关，不能臆造点击行为。
- 1356的wParam0/2选择收发列表，wSeries条数；原编码正文逐段按/切分，再各解码80字节。1357固定613字节正文缓存，正文偏移112。列表id0、WORD4/6、人员8、第二列33已确认，其他字段/打开路径待核对。
- 本轮为具体接线证据进展，邮件行为未改、无新增完成页面；上轮预览截图等待展开修正仍未提交，将与后续完整批次合并。

- 邮件字段恢复补充：45B420复制header+64日期，人员8(25字节)、标题33(31)、日期64(16)。body613中金币8、物品12(100)、正文112(501)。新增MailWire.h及ReceiveList/ReceiveBody，GameProc1356/1357分派已写入，尚未编译/测试。
- ReceiveList按/分段调用旧char*解码API（std::string.data），要求80字节；ReceiveBody匹配当前选中并OpenSelected进入阅读。还需处理正文先于列表缓存、未知ID回复、方向切换与通知生命周期，不能仅凭当前接线视为完整。下一步统一实现这些状态及预览177起，再完整构建验证。

- 已补receivedBodies按(sent,id)缓存，正文先到也能保留，刷新列表恢复对应正文；切到Compose/Gift或关闭窗口时延迟回复不切阅读页。缓存当前Destroy清理，关闭保留，实际原版关闭缓存策略待结合45C450进一步确认。
- 1358通知分派与2072 HUD提示绘制接入，覆盖顺序在投石车之后，提示4622。未伪造点击开关，释放路径仍按原证据处理。
- 新增预览177 mail-wire、178 mail-wire-read、179 mail-notification，以及10项解码/缓存/方向/边界/页面状态检查。当前尚未编译运行，下轮先clean Client（类布局增加map/bool），再统一全部目标和预览回归，不把写入测试代码视为通过。

### 邮件回复与通知批次验收（2026-11-17）
- clean Client86/86通过，build-mail-wire-all.log全部七目标通过。mail-wire-check10项全1；旧邮件15+4+3、投石车10+9、寄售16项回归全1。记录解析测试对80字节列表先编码再按原/分隔解码，613字节正文直接传接收函数；尚未注入完整加密网络包，不算实服验证。
- 预览177/178/179分别mail-wire、mail-wire-read、mail-notification；下一空闲180。out/mail-wire-list.png、mail-wire-read.png、mail-notification.png逐张检查，人物/标题/日期/正文/金币附件标记与来信提示可见。
- 本批合并此前预览截图等待提示展开修正和投石车名称证据更新；未另做碎片提交。资源/报文/记录偏移统一MailWire命名，复用原生邮件页面和消息解码。
- 边界仍明确：通知图标按原证据只显示，不新增未经确认的点击打开；邮件查询、发送、删除、撤回请求尚未发送；服务器开放入口与真实事务仍待接入。缓存Close保留/Destroy清空，原版所有删除/撤回失效细节待恢复。整个客户端GUI/登录/服务端目标未完成。

### 邮件动作发送适配进行中
- f213824后开始：恢复查询1137/读取1138/删除1139/撤回1142，方向0/2在wParam，邮件ID recognition，撤回方向参数0。新增CMailWnd::BuildRequest与在线SendPacket，保留Request快照、等待服务器更新，不本地删信；尚未构建验证。
- 普通信件1141固定596字节，recipient0/subject64/body95；标题首CR段，长于30则前28+Cm4613。送礼按钮45A9D9后未找到直接发送调用，需继续查控制路径，不能套用普通发送。
- 下一步补普通发送编码/DBCS边界和报文验证，确认删除/撤回回执缓存失效；统一完整批次构建/截图提交。当前仅适配已确认4类头部，发送Compose/Gift仍待实现。

- 原版送礼发送按钮命中后45A9DE直接跳默认退出，无网络调用，证据out/mail-gift-send.asm；不臆造送礼协议。普通编写页调用45C100发送1141。
- 已新增BuildCompose固定596字节及在线发送：recipient0限24、subject64限30（长标题28+4613）、body95限500，复用GuildEditor::Payload，保留DBCS边界。类未新增成员，完整功能后增量构建即可；目前尚未构建测试。下一步报文/中文边界/换行测试及删除撤回缓存失效核对，统一提交。

邮件发送批次验证：客户端及六个服务器目标构建通过（out/build-mail-send-all.log）；新增发送10项及现有邮件/投石车/寄售67项检查全部通过。普通发送1141已适配固定596字节，送礼按钮原版为无操作，未伪造协议；实服回执仍未联调。

### 寄售列表及页面状态接入（2026-11-17）
- 1355恢复312字节记录，各段编码以/分隔；字段server0、id16、seller20、price72、date76、item91(100)、description191(121)。证据Client/docs/consignment-wire-evidence.md。
- CConsignmentWnd::ReceiveList接已有原生页面，首条默认选中，清列表/等待状态，count>0更新页码；1355仅未打开时开窗口并保存recognition会话ID，1400按recognition==0设置等待动画。发送操作尚未适配。
- clean Client通过，全七目标构建通过；新增10项列表解析/状态检查、既有77项相关回归全1。截图out/consignment-wire.png已核对；未验证完整加密包或实服。预览180 consignment-wire，下一空闲181。

### 寄售操作发送适配（2026-11-17）
- 已按原版接1130登记、1131/1132翻页、1133购买、1134刷新、1135撤销、1136查询。recognition为会话ID，商品ID低高WORD；购买正文物品名，查询CR分隔条件，登记正文名/实例/价格/说明，堆叠数量放WORD。
- 浏览器开登记时复制会话标识；Request保存不可变会话快照，确认期间列表变化不换目标。登记确认调用后关闭窗口/清会话，不本地扣背包；说明转换CRLF，安全上限123字节，DBCS不截断。
- clean Client和全部七目标构建通过，12项发送检查+原有87项回归全1；追加快照和登记关闭断言通过。截图out/consignment-send-confirm.png已检查。
- 原版1秒重复操作时限、完整加密网络注入和实服事务回执仍未完成；发送适配并非交易成功。证据Client/docs/consignment-wire-evidence.md；没有新增预览ID，180继续包含接收与发送检查。

### 寄售键盘与操作节奏（2026-11-17）
- 455A30方向键上下选择0..12，左右1131/1132，删除此前客户端页码边界拦截；455930/455670空行也命中/高亮，但空行无交易请求。
- 查询独立searchTick，翻页/刷新/交易actionTick，DWORD差严格>1000；取消确认不消耗间隔，购买确认记录时间，撤销确认再次校验。Close/Destroy清计时。主界面模态之后、顶部窗口处理方向键，EDIT光标不受影响。
- 原生实现及交互批次clean Client+全部七目标通过，新增18项和既有99项回归全1。截图out/consignment-keyboard.png已检查；预览181 consignment-input，下一空闲182。实服与完整加密包回执仍待联调，不宣称全GUI目标完成。

### 投石车动作、地图请求与对象标记（2026-11-17）
- 接入1120目标坐标打包、1121移动清目标、1122等待、1123装填（实例ID/数量/名称），复用实际SendPacket；开窗请求1033，不激活普通小地图pending。P/HUD/统一开窗均生效，关闭不请求。
- 1312位置缓存保存wSeries类别，投石车地图绘制缓存中的其他对象，0绿色/非0橙色，按地图源半宽高筛选；仍待恢复对象过期及人物/怪物标记条件。没有以完整动态地图完成为名结束目标。
- clean后WaitMessage命名冲突已修复，全部七目标重建通过。新增7项含真实SendPacket本地UDP回环捕获+既有117项回归全1。截图out/siege-send.png、out/siege-load-send.png已检查；不代表实服操作成功。
- 继续使用174/175/176预览，未新增ID；下一空闲仍182。证据Client/docs/window18-evidence.md与out/siege-load-and-objects.txt。

### 地图对象位置生命周期（2026-11-17）
- 428110汇编确认DWORD时间差>=5000删除1312位置记录，调用点4249A3；每条记录+16更新时间由接收分支刷新。证据out/map-object-expiry-confirmed.asm。
- CSiegeWnd缓存保存updated，每帧RenderInterface调用ExpireObjects，不依赖窗口可见性；只移除记录/取消objectPresent，不解除controlled，也不清数值和目标。新位置恢复；换图清缓存沿用。
- 初次增量链接命中旧GameProc目标依赖，clean后全部七目标重建通过；11项新增+124项回归全1。182 siege-expired截图out/siege-expired.png已检查，地图和射程消失、控件保留。下一预览空闲183。
- 人物/怪物地图标记、全量GUI细节及实服联调仍待完成；目标保持原范围。

### 三种地图场景标记接入（2026-11-17）
- 对照Godot minimap.hpp与MIR3 45D0B0/45DAD0/45E660：修复此前将NPC黄色点误作monsters且仅在预览赋值的问题。CheckMappedData每帧动作/删除后收集NPC、存活怪物、存活玩家，红/白受选项控制，黄NPC始终显示；投石车不画白玩家。
- 1312缓存过期后CopyMapMarkers同步普通/头像地图绿橙集合，投石车移除重复绘制。选项在RenderInterface开头同步，避免头像图读旧值。
- clean Client及全部七目标构建通过；6项状态+既有135项回归全1；四截图精确红/白/黄12个像素计数通过。文档Client/docs/minimap-scene-evidence.md。尚未用实服场景对象执行端到端验证。
- 新预览183 scene-map、184 scene-portrait、185 scene-siege、186 scene-map-hidden；下一空闲187。截图out/scene-map.png、scene-portrait.png、scene-siege.png、scene-map-hidden.png已检查。

### 任务列表、子任务及正文接入（2026-11-17）
- 从 Godot quest_wire.hpp 移植独立解析器；原生 QuestWnd 接1301/1302/1303/1310/1311，开窗/展开/空正文点击查询1046/1048/1049。全量替换清旧数据，增量按原版重复停止，迟到正文不切换选择；Destroy清缓存。
- MIR3 46CFE0/46D310再次确认查询条件；正文160像素换行，通知白字红底符合401430调色板。详见Client/docs/quest-wire-evidence.md。
- clean Client及全部七目标构建通过；新增15项+既有任务14项+相关141项，共170项全1。三截图out/quest-wire-list.png、quest-wire-detail.png、quest-wire-increment.png已检查。尚未验证完整加密包或实服流程。
- 预览187/188/189，下一空闲190。全GUI与登录服务端目标仍未完成。
- 下一批金额对话框待修：原版4256B0寄售价与邮件金币上限100000000；目前寄售MaximumPrice是0x7fffffff，邮件仅余额限制。原版输入0保留旧金额；证据out/extension-load-consumer.c，需整体核对交互后统一构建提交。

### 金额输入对话框行为修正（2026-11-17）
- 原版4256B0的r/v分支确认寄售和邮件金额上限100000000；零保留旧值。邮件不按余额限制，增加giftGoldConfirmed独立控制显示；取消输入清标志，关闭/取消送礼/换物品清状态。
- 459670 +10364控制金额显示，确认零也显示0。详见Client/docs/amount-dialog-evidence.md。沿用原生控件/资源/音效。
- clean Client与全部七目标构建通过；金额5项+相关80项回归共85项全1。两截图out/amount-consignment-price.png、amount-mail-gift-gold.png已核对。
- 没有新增预览ID，下一空闲190。实服登录和全GUI仍未完成，送礼按钮没有已证实可接入的发送协议。

### 状态页与HUD能力记录接入（2026-11-17）
- 审计发现SM_ABILITY仍直接解码旧ACTORABILITY，元素只有预览赋值。现按Godot actor_ability.hpp及MIR3 42F5A0/4390E0/4710F0接63字节记录，显式更新原生属性、金币、职业、生命/血条及元素。
- 新AbilityWire.h命名字段；原始扩展28字节与4头选项保留，StatusWnd::Init清空。强弱元素signed显示保持，不触发伤害跳字。
- clean Client及七目标构建通过；新增13项+相关63项共76项全1。190 ability-wire截图out/ability-wire.png已核对；下一空闲191。
- 详见Client/docs/ability-wire-evidence.md。Server/GameSvr/ObjectPlayer.cpp两处仍直接发送CObjectAbility，顺序及五元素与原版不同，后续服务端阶段必须显式序列化；没有将当前截图当作实服联通。全GUI与登录目标保持进行中。

### 属性增量及原生升级反馈（2026-11-17）
- ReceiveProgression统一接经验44、升级45、负重及附加属性；经验按1459/1460/1513中文白字红底，武器修炼WORD按累加/覆盖/零不改，完整能力消息不覆盖修炼值。
- 升级wSeries0本地、1远端人物、其他忽略；动作7/方向4，本地1469通知。修正本地骑乘动作结束错误回站立。
- 新LevelEffects.h/.cpp从MInfo.dat读取422/库137，加载MagicEx.wil，原地多实例、严格帧间隔、末帧光尾和回收。Native DrawSelectionFrame增加Screen混合与RGB染色；照明接CLightFog，换图/退出清理。
- 原音效14222已请求，当前SoundList.wwl没有映射，无法验收可听输出。运行目录缺失MInfo已从原资源复制；部署需此文件。
- clean Client87/87及全部七目标通过；新增15项+相关60项共75项全1。191 progression、192 level-tail截图out/progression.png、level-tail.png已检查；下一空闲193。
- 文档Client/docs/progression-evidence.md。预览特效固定两屏幕位置便于检查，实服实际地图投影；夜间实景/全部移动边界/实服时序仍待验证，目标未完成。

### 原生技能列表与快捷栏协议接入（2026-11-17）
- 替换GameProc旧裸解码/增添分支：CSkillWnd::Receive接211/210/212/640，固定74字节独立分段，计数低BYTE，先验证再替换，尾余文规则按Godot/MIR3。
- 独立12槽低BYTE技能ID表用于HUD/按键菜单，保留全量/删除不清表、新增A不绑定差异；退出角色清理。BindMagic同步表并沿用1008先取消后设置。熟练度DWORD完整显示，不再变负。
- clean Client87/87与七目标构建通过；12新增+69相关回归共81项全1，图标/标记各12帧存在。193 skill-wire、194 skill-wire-progress截图out/skill-wire-list.png、skill-wire-progress.png已核对；下一空闲195。
- 文档Client/docs/skill-wire-evidence.md。实际施法GetMagicByKey仍沿用当前CLIENT查记录；完整加密连接、服务器技能记录及施法/消耗/战斗不在本批验证范围，目标继续。

### 行会主页成员与组队数据接入（2026-11-17）
- 753原先误开旧GuildMaster；现在CGuildWnd解析名称/权限/公告敌对同盟并打开原生行会页。773/774续包和756/775/776成员按Godot纯C++模型移植，保留累积重复行为与编辑内容。
- 补F键1036、主页/成员按钮1036/1037、成员成功后查询及原版失败原因提示；组队660/661/664/665/666/667接允许/名单/失败提示，解析失败原子保留，初始化清成员和允许状态。
- 首次clean构建测试用了旧消息框m_szMsg导致失败，补CLoginNotice::Text只读接口后七目标构建通过。15项新检查（含25种原因组合）+30回归共45项全1；3种查询通过实际SendPacket UDP回环。
- 195..200六预览guild-wire、guild-wire-members、guild-wire-editor、guild-wire-error、group-wire、group-wire-error，六截图out/guild-wire-{overview,members,editor,error}.png及out/group-wire-{members,error}.png已检查；下一空闲201。
- 详见Client/docs/social-wire-evidence.md。待办：行会历史页m_pages[2]仍只预览填充，需原版追踪其追加入口，不能猜所有聊天都属于历史；身份职位/徽记实际赋值也未接。实服及登录仍待，目标保持完整。


### 2026-11-17 行会聊天历史完整接入
- 原版 42F380 -> 4331C0 证明只有 SM_GUILDMESSAGE104 写历史；不要把所有系统消息写进去。CLIENT AppendHistory 已接入实际消息分发，沿用 Godot 的 514px 宋体9pt字节换行、最多5行、79行历史、共享滚动位置逻辑。修正向下滚动上限为最后一行（433900）。
- 消息编码上限1024；聊天显示上限200解码字节，长通知仍保留历史；气泡固定数组边界已限制。页签105.wav沿用，无新增到达音效。
- clean87/87、all111/111、最终增量通过；guild-history11、social-wire15、guild-editor17共43项全部1。截图out/guild-history.png已检查。新预览201，下一个空闲202。
- IDA重开会话1a4e6f32；open.ps1需进程环境IDADIR=C:/Program Files/IDA Professional 9.1。分析库C:/Users/czx/AppData/Local/Temp/mir3-assess-205da311dc994a8bbd59010e1c6a9ce2/Mir3-unpacked-analysis.exe.i64。
- 本批仅GUI数据链路与本地协议注入，不代表真实服务器联调完成。身份行会名称/职务数据等仍待审计。


### 2026-11-17 人物页行会身份同步
- 新增StatusWnd.ReceiveGuildIdentity，由SM_CHANGEGUILDNAME750接入；分别更新hero.m_szGuildName、m_identityTertiary职务、m_identityBadge称号，缺失时清除。Godot只提供四字段排版，协议依据原版42B859..42B970。
- 星号前一字节按原版丢弃，通常数据为行会/职务/*称号；称号独立存在。行会与职务合计40 ANSI字节，截断职务；拒绝行会>40、称号>59的越界输入。750忽略nRecog更新本地；消息42旧流程仍待核对。
- clean87/87、all109/109通过；identity8+history11+ability13共32项全1。两张截图out/guild-identity.png及guild-identity-cleared.png已查看。预览202/203，下一个空闲204。实际游戏服与扬声器输出未验证。


### 2026-11-17 三行角色名称与聊天色修正
- 完成SM_USERNAME42的Godot SceneName三行CR解析；末段斜杠截断、每行63字节、空字段清除。CActor.ReceiveName统一直接消息与队列，新增m_szGuildRank；本地同步Status职务，保留独立称号；750同步actor职务。Actor.Init清空三行。
- 场景DrawName按Godot宋体9pt、格中心、-30/-15/0偏移、四角描边；尚未实测动态地图中的定位，只验证固定格坐标原生渲染。
- 重要纠错：前一批行会历史的聊天区颜色顺序错误。现按Godot post_chat与原版调用，wParam前景/wTag背景；104固定绿字黑底。早期color测试预期错误已纠正，out/guild-history.png更新并查看。
- clean87/87、all109/109；scene-name10+history11+identity8+ability13共42全1。截图out/scene-identity.png已检查。预览204，下一个空闲205。真实服务器及动态地图仍未验证。


### 2026-11-17 原生聊天双模式历史
- Interface.MsgAdd移植Godot ChatHistory::post：200 ANSI字节上限，341/429px字节换行，两套行缓存各79行、独立滚动；保留原版expanded索引在两个插入循环中都检查的行为。ReflowChat改为当前缓存投影，切换模式不重排历史、不强制到底。
- m_chatHistory保留最近消息供已有查询/测试使用，显示来自m_chatLines[2]；m_chatFirst[2]/m_chatLayoutMode跟踪各模式位置。用户拖动仍更新m_nFstLine，在新消息/布局变化时同步。普通向下滚动上限改为最后一行。
- clean87/87、all111/111。chat12+guild-history11+progression14+scene-identity10共47项全1。截图out/chat-history-compact.png及chat-history-expanded.png已查看。预览205/206，下一个空闲207。详情Client/docs/chat-history-evidence.md。
- 仍未实服收发验证；气泡换行/绘制、旧独立ChatPop窗口等需继续按原版审计。


### 2026-11-17 原生聊天气泡
- Actor.ReceiveSpeech取代ChatMsgAdd，移植Godot144px/宋体9pt字节换行与8行上限，259字节超限或>8行保持旧气泡。空消息清空；ShowMessage严格>3000ms过期。新增SpeechBounds/SpeechPosition用于实际绘制和几何检查。
- DrawLegacyPanel绘制RGB80/60/40 alpha120预乘底色，白字右下深灰阴影，多行末行居中。删去旧重复D3D背景代码。Actor数组5改8，清理按sizeof；已干净构建。
- GameProc启用SM_MONSTERSAY1501，统一FindActor分发40/1501；恢复NPC/怪物ShowMessage。不存在角色不生成实体。
- clean87/87、all109/109；speech14+chat12+guild-history11共37项全1。截图out/speech-single.png、speech-multiline.png、speech-eight.png已查看。预览207/208/209，下一个空闲210。
- 尚未实服或移动地图中验证；继续GUI完整性审计，勿据此标记全目标完成。


### 2026-11-17 服务器公告原生迁移（视觉差异待核对）
- CNoticeBox从旧资源0改为原版1250/按钮1251、1252，位置131,429；以宋体9pt按4815D0宽324、文字50,115排版，最多21行。移除旧StringDivideLen分配链表及失效背景指针，逐帧重取资源。
- 658关闭通用提示再显示；活动公告不覆盖；确认由按下发送改为释放或回车1018全零，鼠标点击105.wav。补齐键盘/右键/滚轮/双击模态拦截。回环UDP验证请求。
- 首次clean因测试CM_NOTICE未定义失败，改为CM_LOGINNOTICEOK后all111/111及最终增量通过。notice11+identity8+chat12共31项全1。
- 重要待办：截图out/server-notice.png、server-notice-moved.png中长行溢出当前资源内框；原版反编译确实为324px且x+50，暂保留该规则。下一批需核对原版实际运行画面/资源版本，不可宣称公告视觉完全一致。预览210/211，下一个空闲212。
- IDA旧会话失效，脚本重开同一库当前78ac321b。详情Client/docs/server-notice-evidence.md。


### 2026-11-17 公告长行溢出复核
- 已纠正资源来源：实际ImageHandler第14项GameInter.wil；1250尺寸344x462，1251/1252为84x30。Interface1c的1250为56x52，先前文档来源记错。
- 原版4815D0确为x+50/宽324px；48BF93直接GetTextExtentPoint32A，48BCAF直接TextOutA，没有内框剪裁或缩放。因此当前素材下的溢出不是遗漏裁剪；保留已确认原版行为，不凭视觉擅改宽度。证据out/server-notice-original-render.c。
- 资源与原版完整运行画面的最终对应关系仍未实机验收；未改运行代码、未重复测试。复核并入上一公告功能提交。
- 补充只读IDA会话950bec4e为VB启动器、a507abb4为MyMir3.dll，不是公告实现来源；主参考仍78ac321b。


### 2026-11-17 GUI音效实际后端验证
- InitMirSound改为返回DirectSound创建结果；PlayLobbyEffect改为bool，静音/无设备/缺文件/加载播放失败可识别。旧调用方资源及触发时机不变。
- CSBuffer.GetVolume与CMirSound.InspectInterfaceAudio只读DirectSound GetVolume/GetStatus，用于实际缓冲验证，不是假调用计数。
- 新预览212 interface-audio15项全1，覆盖按钮105、实际音量、静音恢复、缺资源、12职业性别语音、物品使用、Opening延迟1200+1/循环音量/停止；options-panel9项回归全1（含实际MP3播放/停止/恢复）。共24条件，12语音样本。
- clean87/87、all109/109通过；out/interface-audio.png已查看。下一个空闲213。尚未扬声器录音或物理输出验收，不能说已听过实际音效。详情Client/docs/interface-audio-evidence.md。


## HUD 快捷栏入口修复（2026-11-17）

原版 HUD action 15（1205，CLIENT 旧名 HELP）实际打开窗口 16，即现有 CQuickWnd；已接通按钮。action 10（1190）原版无窗口动作，已删除旧 CHATPOP 误打开。原版证据、快捷栏与音效 50 项集中回归见 Client/docs/hud-quick-entry-evidence.md；七目标增量构建通过，截图 out/quick-items.png。下一 preview ID 仍为 213，本批复用既有预览。目标未完成，在线流程仍待后续验证。


## 后续物品使用迁移断点（2026-11-17，待实现）

已核对 a257359 后的新快捷栏与旧数字键/SM_EAT 回包不连通，另缺共享冷却、请求 tick 和堆叠剩余量处理。具体源码依据、参考 Godot 方法、整批实现及验收范围见 Client/docs/item-use-migration-audit.md。该审查不表示使用功能完成；尚未修改运行时代码，未启动新测试，待完整功能实现后统一中文提交。


## 原生物品使用链路迁移（2026-11-17）

已落实上一节断点：InventoryWnd 统一冷却、请求与 OK/FAIL 回包；QuickWnd 使用新六格引用，数字键在按下时处理；背包按释放间隔处理使用，旧抬键 belt 使用路径停用。CM_EAT 携带 tick，模式 3 使用服务器剩余量，失败和重复回包不丢引用/不重复反馈。ItemUseIds.h 保存新增资源与冷却常量。预览 213/214 为 item-use-chain/restored，下一 ID 215；截图 out/item-use-chain.png、out/item-use-restored.png。详见 Client/docs/item-use-migration-audit.md。真实服务器消费及持久化仍未验证，目标继续。


## 商人服务结果回包（2026-11-17）

已补全出售648/649/1314/1315、修理669/670、存入701/702/703的GameProc分派和InventoryWnd结果处理，结束Paying、同步余额/耐久/剩余量并显示原提示。服务使用完整物品名；UpdateStackCount新增removeWhenEmpty，原1313零数量语义不变。普通出售保留Godot/原版只删除背包槽的行为。MerchantServiceIds.h集中新增常量。清理87/87及七目标构建通过，78项集中检查通过；截图out/service-repair-reply.png、service-sell-reply.png、service-deposit-reply.png、service-failure-reply.png已查看。证据Client/docs/merchant-service-reply-evidence.md。预览215..218，下一219。在线服务器交易及存储仍待验证，目标未完成。


## 仓库取回结果同步（2026-11-17）

StoreWnd::Receive补上705 wTag非零时的背包/快捷栏/鼠标持有数量同步，继续用wParam更新仓库剩余量；705..707异常附加载荷不消费pending。与Godot原分支一致，零wTag不清背包。预览219 storage-withdraw-reply，下一220；74项集中回归及七目标增量构建通过，截图out/storage-withdraw-reply.png已查看。详见Client/docs/storage-withdraw-reply-evidence.md。真实服务器仓库事务仍待验证。


## 装备穿脱待迁移审查（2026-11-17）

590a742后核对发现装备615/616/619/620仍为旧事务：缺少pending种类/回包匹配，sizeof(ITEMSET)清COMMONITEMSET，满背包失败返回值被忽略；缺1124快速操作和回包音效。已读取Godot EquipmentState及原版地址注释，完整行为和下一批验收见Client/docs/equipment-transaction-audit.md。本轮仅新增审查依据，未修改运行时代码，未测试/提交，下一preview仍220。


## 原生装备穿脱事务迁移（2026-11-17）

已完成上一节审查的统一事务：StatusWnd记录穿上/脱下pending，普通与1124快速操作、错类/重复回包拒绝、失败恢复和音效；满背包返回物品保留鼠标持有，满装备槽保留pending。GameProc仅接受成功后更新外观，背包脱下通过RequestTakeOff，物品使用互斥覆盖脱下pending。长名称夹具修正为30字符。清理87/87、七目标111/111及72项集中检查通过，三种final_state通过。预览220..222，下一223；截图out/equipment-transaction.png、equipment-rollback.png、equipment-takeoff.png。详见Client/docs/equipment-transaction-audit.md。真实服务器穿脱和属性重算仍未验证，目标未完成。


## 交易完整流程审查（2026-11-17，待实现）

核对8ff102a发现ExchangeWnd仍只有页面与部分操作，pendingSlot没有放入路径，GameProc无673..687交易结果分派，HUD直接开本地窗。已读Godot完整交易模型/收发与取消完成逻辑，详细迁移范围和消息字段见Client/docs/trade-transaction-audit.md。下一步应整批迁移交易，避免只补一个回包。当前仅新增审查文档，无运行时变更/测试/提交，下一preview223。


## 原生交易完整流程（2026-11-17）

已完成上述审查对应的一整批：TradeWire.h命名常量、C键/HUD正前方玩家请求、673..687分派、放入/数量拆堆、取回、金币、确认/取消与失败恢复，音效及快捷引用同步。RestoreItems批量原子恢复，满背包保留pending；交易pending期间补齐背包按下/抬起与场景丢弃互斥。关闭数量/金额弹框统一清理，修正公会管理关闭按钮误切换交易窗口。不要按旧1025 CANCEL别名臆造取消报文。

清理87/87、七目标109/109及修正后增量构建通过；新交易54项+相关回归108项=162项全过。实际UDP请求与GameProc编码回复、DirectSound状态均检查，真实双客户端联机尚未验证。截图out/trade-transaction.png、trade-quantity.png、trade-gold-input.png、trade-open-error.png、trade-cancelled.png已查看并展示；运行exe为out/mud3-01/Mir2Ex-trade-verified.exe。详见Client/docs/trade-transaction-audit.md。预览223..227，下一228。完整目标继续保持active。


## 原生技能与面板键盘路由（2026-11-17）

feafc67后完成GUI输入批次：F1..F12全绑定、F10系统路径、聊天编辑转发、未学习格清冲突、滚动/遮挡/模态保护；角色入口从GetMagicByKey改用独立低BYTE快捷表GetMagicByShortcut，陈旧表无对应记录不施放，移除A落入F1。E/Q/W/G/N与R及Escape按Godot处理，F9/F10/F11不再开旧窗口。GameProc消费GUI按键立即返回，避免N触发旧SetNextProc。技能布局旧预览切换改用E/Q。

清理87/87、七目标109/109及最终增量通过；新增34+回归207=241项通过，含UDP1008和DirectSound既有音效回归。预览228..230，下一231；截图out/keyboard-skill-binding.png、keyboard-skill-shortcuts.png、keyboard-skill-covered.png。NPC固定左上角，遮挡夹具应移动技能窗而非单改NPC命中矩形。详见Client/docs/keyboard-routing-evidence.md。

下一完整批次可核对退出/返回角色对话框与Alt+Q/X、战斗冷却和网络退出；技能路由、3017正文/时序及法术效果仍是旧Actor实现，已读取Godot skill-casting证据及request_skill_cast/SpellTiming/SpellRoutes，不能声称技能系统完成。全目标保持active。


## 原生退出与返回选角完整链路（2026-11-17）

基线83ec303，本批统一完成HUD退出/返回及Alt+Q/X、101/109确认框、8秒打开/确认双检查、死亡19豁免、1009公共发送器、严格>2500ms整屏乘法渐暗、清原始包/旧场景/声音、恢复保存选角地址及640x480无按钮1248等待框。收到合法SM_QUERYCHR才关闭等待；没有用本地英雄伪造角色。独立退出时钟连接CMyHero受击、攻击准入和指定技能尾部，完整法术路由仍待迁移。

取消socket异步事件并清队列后关闭，过滤旧socket事件，避免Windows句柄复用误关闭新连接。真实本机TCP夹具验证保存端口恢复、重连查询及非法/合法列表；公共发送器连接重置状态1，首次发送递增为线上序号2。

清理87/87、七目标109/109及最终增量通过；退出30+重连7+键盘34/交易54/装备25/音效15回归=165项全部通过。程序退出用独立子进程自然结束；select-three/create/delete也运行检查。运行文件out/mud3-01/Mir2Ex-exit-verified.exe，报告out/exit-regression，截图out/exit-{program,character,blocked,fade,waiting}.png。预览231..236，下一237。详见Client/docs/exit-flow-migration-audit.md。

完整目标保持active。真实账号登录、战斗、角色保存和再次进入游戏尚未联机验收；后续继续GUI缺口及技能路由批次，不能把本机TCP夹具当真实服务器登录流程完成。


## 施法完整批次审查（2026-11-17，继续实施）

d7c70bb后核对当前源代码与Godot施法证据。重要纠正：原生74字节技能记录已经完整保留49延迟/53 MP门槛，不需要新增记录解析；3017头已正确但缺有符号tick正文。旧500ms准入、MP豁免/提示、动作分派、638/1410效果和恢复必须整批迁移。OnSpell原本直接atoi正文正确；CanWalk只发现定义，不能改未调用函数冒充移动限制。详见Client/docs/skill-casting-migration-audit.md。本次记录实证，不单独测试/提交；下一预览237，总目标active。

施法批次已有未验证运行时代码：SpellRoutes.h/SpellTiming.h复用Godot纯C++模型；ExitFlow共用route限制；3017时间正文、49/53偏移断言、CMyHero实际按键/SetMagic初步接入。尚未构建或测试、未提交，不能当功能完成。起手方向/效果帧、特殊18动画声音、预测恢复和实际移动门槛仍待同批实现；细节见skill-casting-migration-audit.md末节。参考hud_view正确路径client/src/godot/hud_view.cpp。

施法继续：BeginSpellAction已统一本地/远端路由、方向、限制帧和104间隔，远端30为动作3；空贴图不再阻断法术帧推进。实际资源入口是SprDfn.cpp CSprite静态表，不是SpriteInfo.cpp；CHero最终声音调用被注释。接下来须用MInfo三节资源替换/衔接，参考Godot resources/effect_catalog.hpp和原生LevelEffects.cpp解码，不能启用旧技能声音映射冒充正确音效。仍未构建/测试/提交。

施法继续：EffectCatalog.h移植MInfo三节纯C++解析，BeginSpellAction按SPELL记录触发原编号起手音效；ApplySpellPhase统一75/40起手、70/200/80阶段和循环阈值。未验收。重要：旧m_xImageList只有61项，原资源库137/149不能直接索引；实际LoadEffect仍旧表，须独立库映射与绘制后再验收。详见施法审查末节，仍不测试/提交。

施法绘制继续：EffectRenderer.h独立映射原库81/91/129/137/138/140/143/148/149，RGB565实现参考shader混合模式；CHero虚拟DrawEffect在原人物绘制位置画SPELL，BeginSpellAction已改查MInfo而非旧静态表。尚未验收；元素色/光源/生命周期、实际移动和预测恢复、638/1410仍待做。未测试或提交，预览仍237。

施法继续：SpellColor使用能力扩展完整WORD（不是GUI低BYTE）选最强元素，平局早元素，起手冻结颜色；光源独立贴图扩缩并在外半径0停止，InitActor清理。发现PlayActorWav将听者Y误取源Sy，已改Dy，需同批验证声像。所有施法改动仍未构建/测试/提交；继续回包/恢复和效果批次。

施法恢复继续：实际SetMotionState步行/奔跑接600ms移动准入后检查SpellTiming完整DWORD限制，LocalActionClock复位。23/18保存原坐标方向，18起动作/142音效；预测直接请求清确认累计；FAIL SetOldPosition加死亡豁免并清施法光/贴图。shared_wait待执行队列、idle提前解锁、GOOD/FAIL/8秒/BUMP闭环仍待完成，不能验收。仍无构建测试提交。

施法等待继续：BeginSpellReply覆盖普通/23/18/预测直接请求，独立elapsed严格>8000释放；idle保留等待锁、视觉结束进战斗待机不再循环起手。GOOD ConfirmAction保留动画/保存位置且待机立即解锁；FAIL与Init清等待。未验证，仍须BUMP强制位移/FIFO及638/1410同批完成，无测试提交。

施法回包继续：SpellReply.h实现638 DWORD/1410十进制列表解码，638已接OnMagicFire，1410尚待完整113效果消费。PACKETMSG正文1040装不下1024字节编码，已改1367含NUL并在默认入队拒绝超长，整批必须干净编译。仍未构建测试提交，下一237。

施法效果继续：新增MagicRules/SpellLight参考规则和SpellFlight世界像素模型（初速精度差异、跟踪整数商、原float步长、15像素/接近判定）。尚未接实例；下一步须处理延迟/3秒/60秒/爆炸切换保留累计/目标引用及音效后接绘制，不能继续套旧CMagic WORD生命周期。仍未测试提交。

施法效果继续：SpellPlayback.h已串起通用延迟/帧累计/60秒/无目标3秒/EXPLOSION保留累计/光尾模型，返回impact供声音消费。尚未接GameProc场景实例与638/1410，调用方必须使用稳定目标实例避免同id重绑；地表/73/74/150单独推进。仍未验证或提交。

施法场景继续：GameProc NativeSpell持有/更新/绘制/清理已接，目标使用CActor独立实例序号防同id重绑；1410本地/远端FIFO已接，113逐实际目标创建，重复保留，其余只改技能。未验收：跟踪尚按格而非行走像素；GetScrnPos(0,0) SHORT原点需换可见格差；光源阶段/声音句柄/638特殊分派仍待完成。未构建测试提交。

施法坐标继续：新实例在actor更新后推进，远端取tile+ShiftPixel，本地用相机+固定锚点反算；32位原点替代SHORT地图0格换算。LightNativeSpells拆到ShowLightFog之前，贴图仍原法术层。尚未验证；同帧新实例年龄/声音回收/638特殊分派继续未完。

施法特殊效果继续：新实例serial边界避免同帧提前老化；73/74 StagedSpell已接638三阶段/原轨迹/爆炸保留时钟及光尾，跳过通用60秒，声音用原格而非暂态偏移格。未测试构建提交，150/地表/其余638及恢复仍未齐。


施法批次继续（未验收）：638已补全完全忽略及仅更新编号的分支，非地表单目标接入新NativeSpell；9/10/53/110尚保留旧路径。新增GroundSpell.h实现原地表几何及帧阈值，尚待场景消费与残留/粒子接入。详见skill-casting-migration-audit.md末节。本批仍未构建/测试/提交，按完整功能集中验收，不误用此前退出批次截图证明本批。


地表施法实现继续：638旧地表创建已替换成NativeSpell地表专用分支；新增痕迹/余烬、共享冰雷粒子池、原ProgUse缩放混合绘制与集中清理。GroundSpell已接场景，先前“尚未接入”记录由此覆盖。SpellOrbit模型已移植但150双轨尚未接入；音效句柄/移动预测和整批干净构建、回归、截图仍待完成。未测试提交，详见迁移审计末节。


施法批次：150双轨现已接入（普通火球+0/30ms反相粒子），SpellDirection改原像素阈值。独立50*10法术声音缓存已接起手/效果/地表/余烬/命中与前5声道跟随、静音/音量/切图释放；旧ChgPlayingSet听者Y也已修正。仍未构建/测试/提交；待移动等待/BUMP等剩余逻辑与整批预览验收。最新改动含Client/Sound/MirSound.h及SpellSoundVoices.h/SoundPosition.h；下次必须干净重建，不复用旧布局对象。


共享动作等待已接原生SetMotionState/AdvancePendingAction：移动准入600ms后施法限制，再按攻击0.8倍/移动480ms共享等待；普通攻击只在入队时做冷却，执行时不二次消费。请求保存起点并在失效/FAIL取消，实际执行写moved/attacked。去掉施法末段提前消费FIFO的旧分支。仍未测试提交；下一步补BUMP7与强制位移生命周期，再对完整施法批次干净编译、运行回归和截图。


施法批次首次干净编译成功：out/spell-client-clean.log [87/87]链接Mir2Ex.exe，exit0。新增ForcedMotion.h接RUSH6/BUMP7/BACKSTEP9，本地/远端严格>95ms推进2帧，BUMP帧4回起点；客户端独立强制位移替代旧UpdateMove。仍未运行新功能验收或截图、未提交。下步新增237起的完整施法预览/回归报告；重点检查模型和实际638/1410、渲染、音效、动作锁/死亡与强制位移。没有仍在运行的构建session。


首轮施法运行验收完成33/33（out/mud3-01/spell-check.txt），新预览237地表/238双轨/239分阶段；真实截图out/spell-{ground,orbit,staged}.png已查看，HUD坐标[4,8]。7目标[111/111]构建通过。注意Ninja未追踪本次SpellPreview.h单独修改，已touch Mir2Ex.cpp重编译[2/2]并重新截图；后续头改必须确认实际重编译/必要时clean。尚缺真实发包、原生动作队列/强制位移、实际声音和像素原版对照等验证，未提交。没有正在运行的编译/预览。


施法验收扩到49/49：本机UDP捕获实际3017序列化（头、序号、signed tick）、真实SetMagic普通/102/12分支、+GOOD/+FAIL/死亡/8000ms边界，以及原生BUMP返回。首次测试漏“+”导致两项失败，已修正测试输入并重跑全通过，生产逻辑未为测试改动。未提交；下步实际音频声道/音量、更多动作队列和渲染对照。所有构建与预览均已结束。


实际法术音频测试发现并修复根目录存在性检查错误（CSBuffer实际Sound\目录）；新增真实GetPan/InspectSpellAudio，11项DS检查通过，新施法共60/60。旧快捷键34+交易54+装备25+界面音效15也全过，总188。runtime仍Mir2Ex-spell-review.exe，音频编译log spell-audio-path.log。未提交；还须渲染原版/完整人物动作与地图队列审查。无运行中进程。


施法动作时钟修复：AdvanceSpellAnimation独立DWORD，严格阈值，阶段调整只在实际推进帧执行，状态0x08000000双间隔，远端首帧追帧减半；新7项动作实际方法测试通过，当前67/67。spell-animation-clean.log干净87构建通过。此前旧128项回归是上一版，勿误报当前195全通过；此版尚须旧回归、实际队列/地图与混合像素对照，未提交。无活动进程。


原生动作队列新增9项通过，当前76/76。使用真实01.map数据（离屏视口，只验证数据/队列），检查pending准入/共享等待/起点取消/施法拒绝/真实CM_WALK发送与消费，以及施法期间638 FIFO保留、结束后消费。spell-queue-build.log七目标112通过。未提交；剩余混合/人物地图视觉审查和最终同版旧回归。无运行进程。


### 施法整批收尾验证（2026-11-17）

原生离线预览增加 `--preview=spell-map`，实际加载01.map与地图/男性人物WIL，通过CGameProcess::RenderScene绘制人物施法、火墙、冰刺、150飞行粒子及原生HUD。截图 `out/spell-map.png` 已实际检查并展示；人物处于树下，保留原生树木遮挡关系。该画面为原生800×600回缓冲截图，不是Godot画面，也不宣称已与MIR3.EXE逐像素相同。

加载全套旧库时发现末尾interfacec2.wil/wix在当前MIR3资源目录不存在，会弹出错误。离线场景改为显式加载需要的地图和人物库；生产LoadAllImage的旧资源清单问题留待真实进场联调处理，未通过复制伪造资源绕过。

最终同一可执行文件 `out/mud3-01/Mir2Ex-spell-review.exe`：施法76、快捷键34、交易54、装备25、界面音效15，全204项通过；退出守卫/发送/等待30项及回选角本地TCP7项通过，总241项。七目标构建成功；最终预览调整构建 `out/spell-map-build.log` [2/2]实际重新编译链接，基于此前七目标完整构建 `out/spell-queue-build.log` [112/112]。git空白检查通过。退出预览已执行，但未将单独程序退出状态额外计入断言数。

本批以“原生施法动作、特效与空间音效”完整功能统一提交。仍待真实服务器联调、MIR3逐像素与听感验收；原生地图旧资源清单、天气与特效共享随机序列仍有已知差异。整个GUI与服务端接入目标尚未完成。


### 正式进场资源加载修复（2026-11-17）

已修复上一批发现的interfacec2缺失提示：CImageHandler::LoadAllImage保留旧索引布局，将不再使用的_IMAGE_INTEREX槽设为nullptr，仅加载真实资源。CGameWnd::SetGameWndActive移除旧三维纹理登记，当前ShowGameWnd使用m_pxWndImage直接二维绘制；其余三维引用位于旧注释代码中。未伪造或复制替代资源。

spell-map预览改用生产LoadAllImage，而非只加载场景子集；60个实际库加载成功，1个废弃槽为空，61项断言通过。实际01.map+人物施法+HUD画面生成out/scene-resources.png，已检查。相同二进制的窗口/快捷键34项通过，共95项。七目标增量构建out/scene-resource-build.log [4/4]成功，git diff --check通过。程序已退出。此证据覆盖正式资源加载和原生窗口开关，不等于真实登录/服务器进场流程已通过。


### 地面物品GUI完整批次审查（2026-11-17）

基线5524af8，已核对Godot ground_items.hpp/hud_view.cpp与原生GameProc：当前缺100件单格容量、名称堆叠/金币变体、闪光周期、严格接收与范围清理；仍绘旧阴影。证据及批次验收见Client/docs/ground-items-migration-audit.md。本次仅保存可执行迁移依据，未改运行代码、未测试或单独提交。下一预览ID241，后续完成整批再验证提交。


### 地面物品GUI整批完成（2026-11-17）

已实现消息校验/stack_mode、独立100容量、堆叠名称/金币名称、原高亮/无旧阴影、40..49闪光与DWORD时钟、24格半开剔除及换图释放，鼠标格补滚动偏移并每帧更新。干净87/87、七目标111/111构建及同版228项检查通过；截图out/ground-items.png、out/ground-coin.png已展示。预览241/242，下一243。证据Client/docs/ground-items-migration-audit.md。没有真实服务器拾取或MIR3同帧像素验收，完整目标仍active。


### 场景光照迁移进行中（2026-11-17）

基线2098af4。已接SceneAmbient进入/消息/逐帧渐变，改18x19光照网格与Ring采样/颜色/每帧对角线；核对地面闪光shader后修正为Additive alpha1。人物灯、地图灯顺序/范围、天气共享随机及旧调试入口仍待完成，未构建测试提交，下一预览243。详见Client/docs/scene-lighting-migration-audit.md，不能把当前未验收代码宣称完成。


场景光照继续：地图灯接真实packed颜色/半径与扩大采集范围；人/NPC/怪物灯集中收集，不再依赖阴影开关；升级灯移到人施法灯之后，旧U/@timeshift同步SceneAmbient。天气完整模型与雪雨雾绘制已读但尚未接入，人物灯本帧可见性/顺序仍需核对。未构建测试提交，详见scene-lighting-migration-audit末节。


### 昼夜光照与雨雪雾集中验收（2026-11-17）

已完成SceneAmbient、18x19采样网格、地图及人物/NPC/怪物灯、天气46/地图消息接入，共享天气/施法PRNG，原生天气Quad/快照/混合，清除旧天气实例。本帧绘制标志与分类+ID灯光顺序已接；地面闪光alpha1补正。最终clean87/87、all109/109与同版257项检查全通过。截图out/lighting-{day,night,dawn}.png及out/weather-{snow,rain,fog}.png已查看。预览243..248，下一249。证据Client/docs/scene-lighting-migration-audit.md记载首轮断言/包含错误和修复。真实服务器、原版逐像素、近战特殊灯序仍未验收，整个目标继续active。


## 2026-11-17：原生账号登录 GUI 完整批次

- 完成 640×480 账号页、Notice.ntc 公告、登录错误框、登录后服务器列表和计费提示；复用 CBMWnd/CChatEdit/CLoginNotice，未使用 Godot 实现。详见 Client/docs/login-gui-migration-audit.md。
- 纠正审计中的初步判断：账号输入坐标原本已对应 MIR3；缺口是 24 字节限制、状态顺序、公告、按钮状态/网址行为和服务器列表反馈。Godot 的注册/修改密码按钮打开 Initial/Param2、Param3 网址并退出，没有独立表单，不应移植旧版注册表单。
- CLogin 采用原生输入控件；Enter 和点击提交差异、提示清空密码、Esc 保持提示。账号全局缓冲区增至 25 字节；普通提示使用既有 GameInter 1240/1243。网址预览只记录请求，不联网或打开浏览器。
- 公告复用 LoadLegacyText，GDI 字号/粗体和原调色板；严格 >100 ms 滚动。服务器列表使用原生像素面板和宋体，529 负载/计费、528 提示，保留同一行按下释放才选择。按钮音效 105.wav，Opening.wav 延迟循环沿用已完成音频层。
- 最终 Client 清理构建 87/87、全七目标 113/113 通过；登录 27、键盘路由 34、音效 15 共 76 检查通过。out/mud3-01/Mir2Ex-login-review.exe，日志 out/login-gui-clean-final.log、out/login-gui-all-final.log。
- 预览编号 249..254（LoginPreview::First/Last）：login、login-hover、login-pressed、login-error、login-servers、login-billing；六张真实原生截图 out/login*.png。原生截图合成已改用实际后台宽高，兼容 640×480 和既有 800×600。
- 下一批应优先迁移 800×600 LoginInterface/SelLogin.ini 服务器分组入口（现有 ServerSelectProcess 仍需核对）。真实登录连接/530 转场和选角/入场联调仍待 GUI 后进行；未声称 MIR3.EXE 同帧逐像素一致。


## 2026-11-17：原生服务器分组入口完整批次

- ServerSelectProcess 原为空壳且未编译。本批加入 Client/CMakeLists.txt 并实现为默认 800×600 入口；配置/必需资源缺失时回退既有 Mir3.ini 账号页。
- 依据 Mir3 br_godot_test_261115 的 ServerGroups/hud_view 和 login-flow 证据，用 CLIENT 原生窗口事件、DirectDraw、GDI 显示 LoginInterface 0/3/4/7/8/11/12/13/15。当前外部帧 1 索引为零，选中光效按参考版可缺失。
- SelLogin.ini 通过 Win32 INI 读取，缓存每组的登录配置；初始 selected=-1，最多 40 组/40 条，20 行可见，滚轮每次一项、分页十项、拖动到 count-1，严格 >10 ms 节流。标签四向描边，标题/服务器名宋体 9 号，按下即选组/进入。
- Common/LoginConfiguration.h 传递网关、账号/改密 URL 和服务器列表。CLoginProcess::Load(bool,const LoginConfiguration*) 接收内存配置并重置提示框/画布；不修改 Mir3.ini。ClientSocket::OnSelectServer 对未出现在 Mir3.ini 的内存服务器也发送选择；旧 ServerNIndex 兼容分支仍保留，网络兼容性尚待真实联调。
- 分组页停止大厅音频，参考版无额外分组点击声；进入账号页沿用已完成的 Opening.wav、105.wav 音效。更新配置节只是元数据，未实现更新下载。
- 编号 255..261（ServerGroupsPreview::First/Last）：server-groups、server-group-hover、server-group-selected、server-group-row、server-group-scroll、server-group-exit、server-group-login。真实资源截图 out/server-group*.png；scroll 是明确标注 Preview server 的 40 行内存验收数据，未写回配置。
- 七目标构建 112/112 成功，最后代码复核后的增量构建亦通过。原生检查分组 33 + 账号 27 + 音效 15 共 75 全通过。默认无参数启动另外通过 CUA 观察分组页，PID TCP 连接数为零；原目录及运行目录 SelLogin.ini、运行目录 Mir3.ini 哈希不变。运行程序 out/mud3-01/Mir2Ex-server-groups-review.exe；日志 out/server-groups-all.log、out/server-groups-final.log。
- 本轮没有执行真实网关登录，也没有打开配置中的外部网址。原始外部配置仍指向旧公网地址；离线预览 m_offline=true。
- computer-use 的旧 sky.get_window_state 返回了与请求不符的旧 Mir2ExGuiPreview 窗口对象，未据此交互；改用 cua_repl.getApp(实际 inventory ID) 完成只读截图确认并发送 completion_summary。正常启动测试 PID 已停止，未触碰原有六个服务器进程。
- 下一步审计登录/选角的连接失败、重试及转场界面；现有 LoginProcess FD_CLOSE/FD_CONNECT 错误提示仍是旧文本流程，Intro/LoginAvi 仍用 ei_login.avi/ei_logo.avi，需对照参考 wemade.dat 和缺失影片行为。继续 GUI 优先，真实服务器流程随后统一调试。


## 2026-11-17：连接失败与重试完整批次

原生 MessageBoxA/CmList132/MB_YESNO，2秒同地址重连、禁用输入和冻结场景、否退出、正常关闭不重试、旧socket隔离。ConnectionRetry/ConnectionPreview 新增，ClientSocket布局改变后已clean构建，七目标通过。连接20+账号27+分组33+音效15+退出30+返回7=132检查全部通过。预览262..264，下一265。真实提示截图 out/selection-connection-error-1.jpg；桌面点击坐标越界，未计作UI点击验收。详见 Client/docs/connection-retry-migration-audit.md。测试exe务必从 build-ninja-sqlite/Mir2Ex.exe 复制，勿用运行目录旧Mir2Ex.exe；最终报告已重新生成。转场/影片及真实服务器流程仍待完成，目标保持active。


## 2026-11-17：登录到选角淡出完整批次

完成SM_SELECTSERVER_OK后2000ms全画布乘色淡出，服务器列表/帮助/提示一起变暗，禁用输入，保留Opening.wav，交接到选角仅一次且不提交明亮背景帧。移除旧静态scroll计数，Load/响应重置。SetNextProc离线模式Load(false)。预览265..267及截图out/login-transition-{start,half,end}.png，下一268。全七目标构建通过；转场12+账号27+分组33+音效15+连接20=107检查通过，最终防闪帧修正后转场复验。详见Client/docs/login-transition-migration-audit.md。影片wemade.dat/CreateChr.dat/StartGame.dat及内嵌音频尚未迁移，不要把旧LoginAvi静态背景当成完成；真实网关联调仍待GUI后进行。


## 2026-11-17：大厅影片迁移进行中（未验收）

基线5dec543，工作区已迁移Godot参考的独立LegacyMovie解码器并增加NativeMovie原生DirectDraw/DirectSound呈现，修改CLoginAvi开场与BackGround创建/入场/返回影片入口。正确路径为Data/*.dat；当前外部资源无这三部影片。角色返回取消/成功改反放，SM_STARTPLAY直接入场影片，删除F1/F2跳页。尚未构建测试提交；成员布局已变，后续必须clean Client再all7。下一预览268，需合成影片测试和截图，详细待办在Client/docs/lobby-movie-migration-audit.md。不能将现有代码宣称已完成；整个批次完成后再测试提交。


## 2026-11-17：大厅影片批次验收完成

接上前节未验收修改，原生LegacyMovie/NativeMovie已完成VFW解码、DirectDraw RGB565、DirectSound PCM；Data目录三个影片入口与正反时序/音效接入。创建取消/成功返回反放、SM_STARTPLAY直接入场、移除F1/F2跳页、影片禁鼠标旧提示操作，重载背景重置及关闭释放。clean90/90、all113/113、复核4/4成功；177检查通过（影片33/登录27/转场12/分组33/音效15/连接20/退出30/返回7）。预览268..271，下一272，截图out/movie-*.png。原版三部影片不存在；forward/reverse为标注合成测试影片，不能冒充原片。详细证据Client/docs/lobby-movie-migration-audit.md。下一步角色创建/删除/选择提示审计，SM_NEWCHR_FAIL/SM_STARTFAIL旧英文是已知缺口。真实服务器联调仍待GUI完成，目标保持active。


## 2026-11-17：角色提示框完整批次

SelectionDialog集中CmList219..232，创建失败按nRecog映射224/226/232，名称13字节与ANSI双字节校验，两角色创建上限；Enter/空格关闭、Esc保持，关闭创建提示清名恢复焦点。删除确认90字节输入先读后清，键盘Tab/Enter和release-only按钮，CM_DELCHR改name/confirmation并回环验证；真实DBSvr兼容性尚待联调。空列表219及非空刷新清提示。clean90/all109/final3构建通过，200检查全通过。预览272..283，下一284，截图out/character-*.png及character-dialogs-1..3.png。详见Client/docs/selection-dialog-migration-audit.md。后续优先选角等待/500ms暂存、文本四槽与旧packed三槽兼容及翻页；GUI整体未完成、真实服务器尚未联调。


## 2026-11-17：选角列表等待与四槽完整批次

新增SelectionRoster/SelectionRoleInfo独立GUI结构，支持nRecog文本四槽/24字节姓名，并保留wParam旧packed三槽、23字节CHRINFO/43字节创建包不变。进入/刷新显示CmList200与1248等待面板；500ms暂存列表及错误，等待冻结输入/角色动画，影片暂停计时。四槽分页与WORD动画时间，释放命中翻页。OnSelChar改动态字符串避免账号24+姓名24溢出原32缓冲。clean90/all109通过，222检查全通过。预览284..288，下一289；截图out/selection-roster-*.png与selection-roster-pages.png。详见Client/docs/selection-roster-migration-audit.md。真实网关未联调，下一核对角色按钮/选择/退出网址/创建编辑器初始化，再整理GUI整体差异。


## 2026-11-17：角色页交互完整批次

选角/性别改按下选中，矩形左上边界包含且角色重叠取首项；新建取消旧渐变等待，按钮/分页/创建补点击音；退出CmList1077网址+关闭（offline仅记录）。OpenCreation统一职业/姓名/13字节非密码/焦点/动画/等待初始化，保留性别，同职业重复点击重启动画。clean90/all109/final增量通过，147检查通过（交互15/列表22/提示23/影片33/登录27/转场12/音效15），最后命中简化复验37。预览289..291，下一292；截图out/character-interaction-{select,create,female}.png。详见Client/docs/character-interaction-migration-audit.md。所有测试进程已停止，真实网关未联调；接下来汇总GUI整体差异和剩余页面，目标保持active。


## 2026-11-17：角色呈现、动画与语音完整批次

共享RoleAnimation取代创建总时间除法和选角重复帧表，WORD时钟严格>120ms推进一次；双性别独立动画/阴影，选择仅重置新性别，职业重置两者。语音改首帧推进时发声，包含初次选角/翻页/创建循环；命中矩形取推进下一帧且静态绘制不覆盖。选角按钮增加棕色悬停覆盖并移到角色之后绘制。clean90/all109通过，新增22+交互15+列表22+提示23+影片33+音效15=130检查通过。预览292..299，下一300；截图out/role-*.png及role-creation-six.png。详见role-presentation-migration-audit；gui-completion-audit新增完整功能族索引及未成立终验条件。所有预览停止、无真实网关联调；下一游戏内面板/弹窗视觉索引和字体布局复核。


## 2026-11-17：选项页释放与窗口拖动批次

核对options_pointer/mount_pointer，选项普通开关/关闭改释放命中即执行，骑乘保留配对。Interface选项拖窗结束只消费释放；控件按下清除拖窗标记。预览300 options-interaction增加10项实际Interface输入检查；修正旧markers测试错误的孤立释放无效预期。增量七目标构建通过，输入10/音量9/服务端选项6/标记5/阴影4/光照6/伤害9/攻击5/音效15/骑乘13=82全过。截图out/options-interaction.png、mount-panel.png；详见options-interaction-migration-audit.md。下一301；多次交叉按下中间帧及其余面板终验仍待整体索引推进，真实网关未联调。


## 2026-11-17：52页游戏内GUI整批视觉索引

新增Tools/review_gui.ps1、gui_gallery.py；当前43d7c75产物逐进程重新截52个主要页面/弹窗，检测BMP时间，不混旧图。out/gui-review-current/index.html含52PNG/9原尺寸拼图/manifest版本SHA256/本批报告。9拼图均已查看，组队/行会源码静态绘制核对无新差异。未改生产代码，无须重编译。详情gui-visual-index.md；inventory-layout三项0属未执行摆放状态，不计失败，不盲目统计全报告通过。所有预览停止、无真实网关联调。下一301；继续错误/禁用/滚动/拖动状态及字体声音核对。


## 2026-11-17：模态提示跨面板输入与EDIT遮挡完整批次

Interface鼠标按下/抬起/移动优先模态，键盘模态先于行会编辑器，修复任务/音量/快捷/骑乘捕获抢事件。实图发现GuildEditor原生EDIT盖住提示，ShowNotice即时隐藏并交焦点主窗，Editor.Show按模态同步隐藏/恢复焦点且保留草稿。七目标增量通过；新增14+任务14+行会17+选项10+音效15+键盘34+退出30=134全过。预览301..303，下一304；out/modal-{scroll,audio,guild-editor}.png均查看，测试文本CmList4000仅离线夹具。详见modal-input-migration-audit.md。进程全停止，真实网关未操作，52页旧版本索引不覆盖。


## 2026-11-17 模态弹窗生命周期修正
- 纠正1968a17仅冻结后台捕获的推断：Godot open_group_dialog 先 cancel_pointer_interactions。CLIENT新增 PrepareModal，接普通提示、输入、退出、寄售确认；取消GUI临时拖动/按下并释放焦点/捕获，保留业务状态和草稿。
- Escape按参考吞键不关闭；Tab选取消再Enter关闭。新增测试初始错误预期已修正。
- 七目标构建通过；模态21+任务14+行会17+选项10+音效15+键盘34+退出30=141检查通过。11页截图 out/modal-lifecycle；最后仅调整测试后重跑模态。三个叠放截图已查看。所有自启预览已停。
- 文档 Client/docs/modal-input-migration-audit.md 已纠正旧说明。下一预览304。整体GUI仍需继续核查，原电影资源仍缺，真实服务器流程留后。


## 2026-11-17 原生模态输入框键盘转发
- ChatEditProc游戏模态时保留Escape/功能键/Tab焦点，Enter按当前选中按钮执行；沿用Interface的重复键过滤。普通字符仍走EDIT，系统键不泄漏后台。
- 新增6项真实EDIT消息检查，包含Tab后取消不改变邮件金币。七目标构建通过；模态27+键盘34+NPC22+密码10+建组5+交易金币8+布局10=116检查通过。
- 9页截图/报告out/modal-keyboard，NPC密码/建组图已查看；最终只增加测试重跑模态。预览全部停止。继续完整GUI核查，真实登录流程未开始。下一预览编号304。


## 2026-11-17 游戏模态鼠标取消和按钮中间态
- 按参考group_dialog_input：移出取消按下，移回不恢复；鼠标按下不改键盘选择。LoginNotice新增移动处理/可选鼠标选择参数，仅游戏入口传false；登录选角默认行为保留。
- 七目标构建通过。鼠标9+模态27+密码10+退出30+音效15=91检查通过，9页预览out/modal-pointer。按下图与移回图已查看，按钮像素差验证成功。
- 窗口初显会产生鼠标移动清除按下，截图夹具已在捕获前重放事件；末次只改夹具重跑三态。自启预览全停。
- 新编号304/305/306按下/移出/移回，下一307。整体GUI目标继续，未开展真实登录联调。


## 2026-11-17 大厅38页统一视觉索引
- review_gui.ps1支持-Suite lobby，默认game52页保留。大厅38页(分组7/登录6/角色列表5/外观及选择8/错误删除12)全部鲜截图，out/gui-review-lobby/index.html及7张拼图已查看。
- 使用4786674已有生产二进制，本批只改工具/文档无需重编译。分组33+登录27+角色表现22+列表22+对话框23=127离线检查通过，所有自启预览已停。
- 创建姓名框空白：源码和参考均(288,405)75×13且show；应补填入内容后的视觉核验，不能仅用空白截图算输入验收。原影片资源缺失仍在。下一编号307。GUI目标继续，真实链路未验证。


## 2026-11-17 创建姓名输入完整状态补验
- 发现创建页只EM_LIMITTEXT按字符限长，未接EN_CHANGE字节限制。CharacterProcess接入ChatEdit.Changed，姓名13字节中文完整边界，删除确认依原90字节。
- CharacterInteractionPreview新增307英文/308中文/309恢复后填入姓名，下一310。实际三图已查看：out/creation-name。测试EM_SETSEL按字符位置，已修正原测试的字节位置误解。
- 新姓名12+交互15+角色表现22+选角弹窗23=72检查通过；七目标构建out/creation-name-final.log通过，6页截图，自启预览全停。真实创建/登录尚未联调。


## 2026-11-17 创建与删除编辑框键盘行为
- ChatEditProc按_CHAR_SEL_PROC统一转发控制键，创建空Enter不隐藏框、Escape/Tab不夺焦；删除Space执行选中按钮。CharacterProcess增加HasNotice只读查询。
- 新增7项原生EDIT消息检查，姓名19+角色交互15+选角弹窗23+登录27+游戏模态27=111通过。七目标构建out/selection-editor-build.log；6页截图out/selection-editor，中文创建/删除确认已查看。自启预览全停。
- 无新预览编号，下一310。整体GUI仍在核查，真实服务器创建/删除未联调。


## 2026-11-17 游戏共用输入字节边界
- GameProc原EN_CHANGE仅处理NPC输入；现统一共用ChatEdit的Changed，实现普通90字节限制，保留中文边界与编辑后缀，NPC自定义限制不变。
- 新310聊天中文/311组队中文/312行会成员移除中文截图；下一313。out/game-input-boundary三图已查看。行会夹具最初误用组队action3，已改4并重跑，生产代码未再改。
- 输入11+建组5+行会编辑17+音效15+键盘34+模态27+NPC密码10=119检查通过。七目标构建game-input-boundary-final.log通过，9页新鲜截图，自启预览全停。数字框测试只验证容量，非业务接受中文数值。


## 2026-11-17 邮件寄售独立编辑框补验
- 两类独立EDIT已用ReadEditors/LimitAnsiEdit，未改生产逻辑。新增中文边界、独立草稿切换、模态遮挡恢复10检查。
- 新313邮件恢复/314赠送恢复/315寄售说明恢复，下一316。out/panel-editor三张中文截图已查看；邮件右侧背包来自此前寄售打开的背景状态。
- 七目标构建panel-editor-build.log通过；共98检查通过(新10/模态27/邮件15/赠送4/滚动3/金额4/寄售16/寄售输入18/价格1)。六页截图，自启预览全停。线上交易尚未联调。


## 2026-11-17 登录提示鼠标释放边界
- GameInter 1241..1246为80×32，现有命中矩形正确。参考login_input在按钮内释放关闭，原CLIENT错误要求成对点击；登录改传releaseOnly=true，游戏仍保留成对/移出取消。
- 新7边界与恢复检查；登录34+选角23+游戏鼠标9=66通过。七目标构建login-notice-pointer-build.log，5页新鲜图out/login-notice-pointer，错误/计费两图已查看。自启预览全停。
- 无新编号，下一316。真实登录链路仍未联调。


## 2026-11-17 创建返回选角输入状态
- 新CloseCreationEditor接取消和SM_NEWCHR_SUCCESS，清文本/缓存/密码、恢复90字节、释放焦点并隐藏，清按钮按下；符合参考close_character_creation。
- 新316取消返回/317离线成功返回，下一318。返回11+姓名19+交互15+选角23+列表22=90检查通过。七目标构建creation-return-final.log通过，6页截图out/creation-return；两张等待页已查看，无残留姓名。
- 最后仅补列表回复测试重跑返回模式。自启预览全停。成功回复为离线注入，不代表真实创建或原影片验证。


## 2026-11-17 服务器公告输入隔离
- 修复GameProc.OnLButtonDown公告后未返回且用旧坐标；公告按下现用当前消息坐标并立即返回，防止持物丢弃等后台动作。Alt+Q/X不叠加退出框。
- 新5项回环UDP检查，公告11+隔离5+退出30+键盘34+模态27+音效15=122通过。七目标构建server-notice-isolation-build.log，6页图out/notice-isolation，默认/拖动公告已查看，自启预览全停。
- 公告324px排版溢出当前344px底板是既有原版证据/资源对应未决问题，已复核server-notice-evidence.md，未擅改算法。下一编号318，真实游戏服尚未联调。


## 2026-11-17 GUI素材归档盘点
- 只读盘点工作区GameInter与四归档：经典复刻RAR实际Mir3Server2610条；mir2x资源ZIP490624条只有同名MP3；Wood/DATA.zip4项人物WIL/WIX；服务器7z无目标素材命中。
- 仍仅找到一套GameInter，哈希/帧尺寸已写Client/docs/gui-resource-inventory.md，原始证据out/gui-resource-audit.json等。公告原帧缩略图notice-source-frames.png已查看。未改资源或生产代码，因此未重复构建。
- 已异步询问用户其他原版客户端本机路径，尚未得到答复。原影片缺失、公告版本对应仍未解决；可继续其他GUI核查，目标active。下一预览318。


## 2026-11-17 窗口移动时输入布局
- ChatEdit.Relocate按此前客户区计算输入框逻辑位置，游戏/登录/选角WM_MOVE统一重新定位；修复旧聊天位置重置和隐藏输入。空句柄初始化阶段返回，无类布局变化。
- Common/InputMovePreview.h实际SetWindowPos验证相对位置/尺寸/内容/焦点/可见性，覆盖登录创建删除及三游戏输入态。新318 input-window-moved，下一319。
- 七目标构建input-window-move-final.log；登录35+创建21+移动4+键盘34+模态27=121通过，6页图out/input-window-move，数量输入已查看。最后空句柄守卫后重跑登录/游戏启动，自启预览全停。
- 未验证DPI/最小化/全屏切换。原影片和公告资源目录问题仍待用户答复，其他GUI可继续。


## 2026-11-17 输入布局缩放最小化恢复
- ChatEdit增加逻辑矩形/有效标记，MoveScaled记录布局，Relocate无参按当前客户区重排；零尺寸不创建无效字体。三个页面接WM_SIZE，WM_MOVE沿用Relocate。
- 有类布局变化，执行clean客户端；首次WindHorn库引用客户端失败已撤销，改为页面入口后七目标input-resize-build.log成功。WindHorn源文件无最终改动。
- 登录37+创建23+移动缩放6+键盘34+模态27+选角23+音效15=165检查通过。7图out/input-resize，中文创建/数量输入恢复图已查看，自启预览全停。
- 跨显示器DPI和全屏切换尚未实测；无新编号，下一319。资源目录问题仍待答复。


## 2026-11-17 同版本整批截图与资源口径

用户确认没有其他原版目录，后续以当前原版资源为准。无需继续等待额外目录；现有资源包仍可按证据核查。未找到的三段影片保留缺失记录，不使用同名MP3代替。

review_gui.ps1新增all套件：52个游戏内状态、38个大厅状态和20个输入/弹窗边界状态。完整运行110/110，生成out/gui-review-integrated/index.html、110张原尺寸PNG、19张联系表及42份本次新鲜报告。manifest记录生产版本724bc52b58f113516f52214fe45418f448bd6b5b和二进制SHA256 1802597130EF08E8226A968C0B198DAF912CE62044604638310ABBB7569DE636。

已核对报告；inventory-layout静态模式的moved/placement_green/placement_red为0沿用既有夹具含义，不累计成通过数。其余报告中的断言均为1，样本数量/字体/尺寸等为数据字段。检查110个唯一模式及PNG齐全；抽查中文创建原图、返回列表联系表19，未声称本轮逐张视觉验收全部110图。旧批次视觉检查记录保留。

本批只更改采集工具和文档，沿用已干净构建的客户端，不重复编译；没有新增生产功能。截图套件不是全部交互测试，音效沿用上一批15项验证，真实服务器流程、原版影片、公告长文本资源适配及跨显示器DPI/全屏仍未完整验收。


## 2026-11-17 整批GUI视觉复核
- 当前110图的19联系表已全部浏览（19上一批，其他本批）；108态无新的明显布局异常，公告默认/移动两态的长文字右框溢出仍可见。out/gui-review-integrated/visual-review.json逐模式记录结果及证据层级。
- 这是缩略图整体布局检查，不是像素一致性、交互或音效新测试。未改生产代码、未重建。详见gui-visual-index.md末节。
- 已检查邮件/寄售/行会编辑框布局源码：均有各自LayoutEditors/LayoutEdit，不能仅根据主窗只调用ChatEdit.Relocate就认定其他编辑框失效；尚未实测这些窗口缩放，不作结论。
- 原资源口径已确认，不再等待额外目录；公告排版及逐页字体/坐标参考核验仍需继续。下一预览319。


## 2026-11-17 面板编辑框最小化与缩放恢复

邮件、寄售及行会编辑框每帧执行各自布局，最小化后客户区为零仍会重设矩形和字体。新增真实主窗最小化/恢复、移动加缩放、恢复原尺寸检查；修改前邮件收件人/正文/送礼正文、寄售物品查询/卖家查询/登记说明六项失败。现在三个Layout入口在零尺寸期间保留现有矩形和字体，恢复后照常按当前客户区排布。

截图复查另外发现：已关闭的行会原生编辑框在后续主窗反复最小化/恢复后重新可见，叠在邮件窗口上。CGuildEditor.Show在非活动状态同步隐藏原生EDIT，补充关闭后恢复不可见检查；最终截图已确认残留消失。未修改截图过滤规则掩盖问题。

CheckPanelEditorLayout覆盖七类编辑框，检查最小化不改矩形/字体句柄、放大后位置/尺寸/字体按比例变化（容许1像素舍入）、恢复后精确矩形、文本及可见性。行会通过正常Show布局，邮件/寄售通过正常SyncEditors布局。未据此宣称跨显示器DPI或全屏已验证。

最终七目标构建out/panel-layout-final-build.log成功。面板18+行会17+共用输入6+模态27+键盘34+音效15=117项检查通过，voiceCount计数另计。九页新鲜截图在out/panel-layout；邮件恢复、寄售恢复、行会公告原图已查看。全部自启预览停止。无类布局更改，无新预览编号，下一319。


## 2026-11-17 原生联系人与快捷短语列表
- 源码核对发现Godot chat_submission与draw_chat_shortcuts尚未移植。新增Common/ChatShortcuts.h，实现在ChatEditBox.cpp：原生列表绘制、Ctrl/Alt数字和上下选择、十条去重FIFO、#本地记短语、/私聊记联系人、ANSI slt/mlt按角色服务器读写。GameProc登录身份载入，Interface初始化清理并绘制。无类布局更改。
- 四个新预览319–322；下一323。测试临时文件不进入Data联接目录。旧私聊回显颜色及chatsave/snow本地分派仍待审计，不能把整个聊天系统视为完成。
- 构建chat-shortcuts-final-build.log七目标成功。快捷21+历史12+中文11+模态27+键盘34+音效15=120检查；九页out/chat-shortcuts，四新增图已查看。详见chat-shortcuts-migration-audit.md。所有自启预览停止后提交。


## 2026-11-17 聊天命令日志及原版颜色
- 新Common/ChatLog.h实现原版两个独立日志追加分支，GameProc真实登录启用./Chat.txt，Interface.Init清空路径。开关@chatsave；@snow从首token比较改为整条精确匹配。
- 修复GetChatColor索引5/6以及私聊回显前景/背景；参照scene_chat.hpp调色板。
- 新323私聊/324调色板，下一325。七目标最终chat-commands-final-build.log成功；143项检查（23本批+120相关回归），最后仅修截图夹具滚动位置，重跑23及两图。out/chat-commands八页；私聊/调色板已查看，自启进程全部停止。
- 详见chat-commands-migration-audit.md。行会添加成员命令、坐骑命令延迟尚待对照；真实服务器链路仍未整体联调。


## 2026-11-17 添加行会成员命令弹窗
- 补Godot ChatSubmission的1425命令分派：会长OpenNameDialog(CM_GUILDADDMEMBER)，非会长ShowNotice1427。1426提示替换行会名，原生输入沿用90字节，FinishNotice确认发送1038全零参数及姓名，无列表附加请求。
- 新325/326/327输入、拒绝、中文填入截图out/guild-add；下一328。新增回环12检查及相关回归共149通过，最终七目标guild-add-final-build.log成功；最后只加强重复Enter夹具，重跑12和三图。详见guild-add-dialog-migration-audit.md。
- 发现m_wInputDelayTime仅计时未拦截输入，参考map_input_delay的300ms坐骑延迟需要完整接入HUD左键/地图动作/技能，而不是只赋值；留待下一批。真实服务器流程仍未整体完成。


## 2026-11-17 坐骑及换图输入延迟
- 新GameProcess/InputDelay.h常量300/1500与三个命令ID。GameProc复用原字段增加开始/推进/active方法；大帧不WORD绕回。两处换图输入2300改1500，人物独立混合2300保留。
- 输入命令/坐骑按钮启动300；GameProc和Interface左键、角色左右持续输入、新移动/攻击/挖取/转向、技能入口阻止新请求。通用弹窗优先，既有动作与服务端动画继续。
- 新328/329，下一330；out/input-delay十图及九报告，新19+相关215=234通过；七目标input-delay-final-build.log成功。测试字段初次命名错误已修cast_at；两新增图查看，自启预览停。详见input-delay-migration-audit.md。
- 仍需真实服骑乘/换图组合体验和整体登录链路；未将离线通过等同端到端完成。


## 2026-11-17 金币拾取及丢弃金额框
- 新GoldDrop.h命名102弹窗、1415提示、1016请求、124图标。背包释放拿金币并播放106.wav，地面打开原生金额输入，延迟阻止打开，回复更新余额。统一HasInputDialog修复行会1038旧提交入口。
- 新330–332，下一333。新增Interface成员已clean Client；七目标gold-drop-build.log成功，最终增量gold-drop-final-build.log成功。金币18、行会13、模态27通过，另完成金额/背包/音效/延迟相关回归。out/gold-drop共11图，三种金币状态查看。
- 截图发现离线预览漏载Inventory.wil，已补齐并增加资源断言；最终重跑五种状态。详见gold-drop-dialog-migration-audit.md。真实服落地金币与堆叠丢弃104仍待后续工作。


## 2026-11-17 堆叠丢弃数量框及普通丢弃回复
- 新ItemDrop.h和Inventory RequestDrop/SubmitDropQuantity/ReceiveDrop及独立pendingDrop。104/1416输入，1101数量请求，上限100，部分回袋等1313；普通1000带时钟，600头成功清状态，601匹配失败回袋/回手。
- 手持图标移至弹窗和提示之后，修复数量框遮挡。新增333/334，下一335。新类字段已clean重编；首次夹具BYTE参数错误修复后七目标item-drop-build.log成功，最终item-drop-final-build.log成功。
- 新28及相关170共198项通过；out/item-drop十二图，两个新状态查看。最后仅1313常量命名整理，无行为更改。自启进程停。详见item-drop-dialog-migration-audit.md。真实服丢弃/落地/拾取与整体登录流程仍待联调。


## 2026-11-17 通用弹窗替换取消状态
- PrepareModal先对活动旧弹窗FinishNotice(CLICK_NO,false)，再重置指针/焦点。寄售Activate先清理旧弹窗后再设置新编号和confirmation，避免误清新请求。
- 新335/336，下一337；18项替换检查及相关回归共221通过，七目标modal-replacement-build.log成功。首次夹具访问私有字段编译错误已改行为观察，无新增类布局。
- out/modal-replacement十一图，两个替换状态查看；详见modal-replacement-migration-audit.md。自启进程停止；真实服组合响应仍待联调。


## 2026-11-17 当前GUI统一128状态基线
- review_gui.ps1 all加入319–336的18个近期状态，128/128新鲜截图成功。生产4f65f9f，SHA256 7B7399A73B9633966F9E6D0575C10D9889BBBEC18FAF2BB3BCD2D53A8FF19368；out/gui-review-native-final有HTML、manifest、visual-review和22联系表，全部联系表浏览。
- 126状态未见新明显布局异常，2公告状态保留既有长文本溢出；49报告无新断言失败，统计/静态字段另记。非逐像素终验或真实服联调。
- 本批仅工具/文档改动，不重建生产二进制。截图自启进程全部停止，六个既有服务器未改动。下一预览编号仍337。


## 2026-11-17 服务器公告编辑框隔离及逻辑居中
- 658使用PrepareModal取消旧输入；邮件/寄售SyncEditors和GuildEditor.Show持续隐藏公告背后EDIT。迟到输入通过m_noticeInputDeferred恢复，其他隐藏原因不复显；共用EDIT回车只确认公告。
- 截图发现公告按物理客户区居中却画到800×600画布，较大窗口按钮落到画面外。改用NoticeBox命名画布尺寸；未改324文字宽度/字体/皮肤。
- 新337–340，下一341。新Interface字段clean90通过；最终七目标112通过。14状态、174断言通过，out/notice-editor四新图查看，自启进程停。详见notice-editor-migration-audit.md。
- all采集清单未来132，旧128基线保留，未虚称全套重跑。真实服及长文本溢出仍未完成。


## 2026-11-17 原生客户端真实入场握手修复
- 独立 out/native-live-01 客户端通过真实本机登录/选服/选角。首次入场 GameSvr 因 GameGate 在索引回复前转发数据越界崩溃；改为 GM_SERVERUSERINDEX 后 Recv，并校验 socket，游戏服 GM_DATA/GM_CLOSE 拒绝越界索引。
- 随后发现运行库未应用既有初始物品定义，背包序列化再次崩溃。SQLite backup 后应用既有 sqlite_minimal.sql，保留账户角色及自定义数据。原程序/库备份在 out/native-live-01/server-backup。
- 最终真实地图01、人物/HUD、点击移动通过，技能/背包面板可开，退出客户端后服务仍运行。背包空白和属性不正确仍待旧服务端/MIR3记录格式对接，不能声称背包完成。
- 七目标构建通过；新增真实TCP越界索引探测及既有SQLite测试通过。截图/失败转储证据/部署细节见 Client/docs/native-live-entry-audit.md。测试客户端已退出，六服务保留运行，GameSvr28188/GameGate31804为本次PID。
- GameGate崩溃后不自动重连，已手动重启恢复；ProcessLogin ready-list游标赋值也待独立验证修复。未动GUI预览编号，下一341。


## 2026-11-17 实服能力记录和数据库连接生命周期
- GameSvr 的 RM_ABILITY / RM_LEVELUP 不再直接编码旧 CObjectAbility，使用 Server/Def/Mir3Ability.h 显式生成63字节原版记录。基础属性、32位经验及重量映射；旧五元素没有七元素映射证据，扩展保留零。黄金字节测试通过，未改客户端GUI/解析。
- 重启实测发现 DBSvr 断连释放对象却未移除 g_xServerList，选角命中失效对象而黑屏。加读写锁保护列表和选角发送、断连移除；排队DB请求引用保活，完成后释放引用及附加数据，关闭socket时防复用误发。
- 全DBSvr CPP重新编译，七目标通过 out/ability-session-build.log。三次模拟游戏服断连后真实登录入场成功；20次断连/160个排队只读请求测试通过；前批非法索引测试通过。
- 人物页等级1和各基本数值、背包负重3/50已实服截图，out/native-live-01/live-ability-status.png / live-ability-bag-weight.png。物品仍空，旧12字节实例ID与MIR3数值ID/100字节物品记录需双向适配。
- 部署GameSvr/DBSvr并重启相关四服务；原LoginSvr/LoginGate保留。最终DB34264/SelGate23520/GameSvr17436/GameGate34804，测试客户端31952已退出。详见Client/docs/native-ability-session-audit.md；下一预览仍341。


## 2026-11-17 基础物品记录与实例请求适配
- 新 Mir3Item.h 100字节字段映射、每角色连接12字节旧标识与数值ID双向表；背包/装备/新增包统一，穿戴/卸装/使用/普通丢弃校验拥有关系和名称后还原旧标识。
- GameSvr 类布局变化已全34 CPP重编，七目标通过；100字节黄金记录及ID测试、能力测试、真实非法索引回归通过。最终 out/item-wire-return-build.log。
- 实服三件物品显示、首轮盔甲和武器穿戴/重量更新有截图。卸装原来缺SM_ADDITEM，已补成功后的返包（包括替换），但最终实服回归未闭环，不能宣称背包完成。
- 重登盔甲未恢复到可选中的装备栏；测试角色HP下降死亡原因未知；跨面板快速点击出现面板移动。后续先追踪装备初始621及角色存活，再验证卸装/使用，勿重复宣称通过。详见 Client/docs/native-item-wire-audit.md。
- 现有最小物品定义Looks均0，三图标为红药水是测试资料占位；未修改资源。会话ID不跨登录持久，魔法属性/堆叠/落地/拾取/满包未验收。
- 截图 out/native-live-01/live-item-bag.png、live-item-equipped.png、live-item-relogin-pending.png。GameSvr33016/GameGate15264运行，测试客户端33232已退出。预览编号仍341。


## 2026-11-17 EIServer 合并构建和新调试目录
- MIR3_BUILD_EISERVER 默认 ON；六服务编译为隔离 DLL 并嵌入 EIServer.exe，同一进程的独立消息线程运行。启动会释放模块到临时目录；目前临时模块文件需后续清理。关闭任一服务窗口结束整个集成进程，尚未实现协调停服保存流程。
- 模块默认 --auto-start；服务配置、SQLite DSN 和起始地图统一读取 EIServer.ini。OFF 保留独立 EXE 构建；DBSvr LocalPort/GatePort、LoginSvr LocalPort 可配置。
- 合并版与独立版 Release 构建通过。新 out/debug 仅放 EIServer.exe、Mir2Ex.exe、配置和初始化 SQLite；Data/Map/Sound 为指向 G:/传奇3/宝藏传奇3 的 junction。旧 out 已不存在，旧测试角色未迁移。
- 新目录实测七个端口5000/5100/5500/6000/7000/7100/7200同属PID33888，登录、选服、角色列表通过；测试进程已结束。客户端编译通过，未做本轮GUI验收。


## 2026-11-17 EIServer 静态库与窗口分离
- 替换嵌入DLL宿主：EIServer.h/cpp为控制台入口，直接链接DBSvr/LoginSvr/GameSvr/LoginGate/SelGate/GameGate六个同名静态库，启动按监听就绪顺序推进，统一读取旁边EISERVER.ini。AdminCmd文字并入宿主字符串资源，不再加载业务DLL。
- 每个服务明确列出LIB源码，初始化及启动逻辑移至Service.cpp；排除原入口、MainWndProc、配置/状态对话框及ModernUI。重复全局符号通过生成的namespace编译单元隔离，不使用DLL。dumpbin检查六库均无WinMain、MainWndProc、CreateWindow、DialogBox、ChooseColor、PropertySheet符号。
- Window目标使用独立ServiceWindow.cpp薄界面调用对应LIB；MIR3_BUILD_EISERVER=OFF构建独立窗口程序，ON默认构建控制台。网络兼容层由宿主创建message-only接收对象，不是可见窗口；单一消息循环调度。
- out/debug保留两EXE与配置数据库，Data/Map/Sound目录链接不变。控制台实测PID21804七端口同属一个进程、EnumWindows无顶层窗口、登录/选服/角色列表通过。Server/Tools/test_eiserver.py可复跑（测试账号mirtest/test123，需默认端口）。
- 本轮验证启动及登录，控制台退出仍由进程终止收尾；协调停服、在线角色落库和独立窗口交互尚未验收，不应宣称通过。此前物品/网关并发的未提交改动保持独立。


## 2026-11-17 EIServer 文件布局与配置清理
- 按最终约定取消Server/EIServer子目录，入口为Server/EIServer.h、Server/EIServer.cpp，配置为Server/EIServer.ini。公共静态库构建模板、头文件、资源脚本及独立窗口入口移至Server/Service；Server/CMakeLists.txt构建控制台入口。
- 逐项比对确认原Server/server.ini.example全部节和键值已包含于EIServer.ini，删除该示例文件。历史记录中复制server.ini.example的运行方式已废止，当前六服务统一使用运行目录中的EIServer.ini；保留其中SQLite与Game配置。
- 已重新构建EIServer与LoginGateWindow并更新out/debug；最新PID25980测试七监听同进程、无顶层窗口、登录/选服/角色列表通过。目录不存在检查通过。


## 2026-11-17 Server 独立编译 EIServer
- cmake -S Server 原先缺少 C/RC 支持，生成阶段无法确定 SQLite 的链接语言。Server/CMakeLists.txt 现在声明 C/C++/RC，并在独立配置时默认启用 EIServer 和旧源码所需的 MSVC 兼容选项。
- Visual Studio 18 Win32 Release 已验证 Server 单独构建和仓库根目录构建。


## 2026-11-17 EIServer 运行复验
- 从仓库根目录重新构建 Win32 Release，使用独立临时运行目录和新初始化的 SQLite 验证。六服务自动启动，七个监听端口同属 PID 23120，无顶层服务窗口，登录、选服和角色列表均通过。
- Server/CMakeLists.txt 在 f6a38c6 后还有本地编辑，本次验证使用当前工作区，未将已有本地编辑纳入提交。


## 2026-11-17 客户端窗口尺寸按 Godot 页面规格
- 以 G:/传奇3/Mir3/client/src/godot/hud_view.cpp 的页面切换为依据，服务器分组/游戏用800x600，登录/选角用640x480。Win32 默认客户区取当前画布尺寸，以物理像素计算，Mir3.ini Display 的显式宽高仍能覆盖。移除 LoginVisual 的第二次窗口缩放。
- Win32 Release 构建成功，144 DPI 下用实际客户端和 GetClientRect 验证登录640x480、服务器分组800x600，已更新 out/debug/Mir2Ex.exe。选角与在线进图尺寸未实测，字体颜色未改。IDA 旧会话失效，未从原版二进制确认尺寸常量。


## 2026-11-17 EIServer 构建产物启动目录
- 复现 Release 目录直接启动因 EXE 旁缺 EIServer.ini 退出 1。CMake 为构建版提供 out/debug 路径，程序优先使用 EXE 旁的配置，仅缺失时回退至有配置的联调目录。初始化失败输出服务名和运行路径。
- Release 构建通过，Exe 从构建目录启动后六服务就绪、七端口监听。MIR3_DEBUG_STATE_DIR 启用的服务器状态快照也随本批接入。


## 2026-11-17 原生客户端登录进图无截图验证
- Client/EIServer Win32 Release 构建通过。MIR3_DEBUG_STATE_DIR 可选输出客户端场景、连接端口、客户区尺寸、登录阶段/提示框和角色地图坐标及 HP/MP；未启用时不写文件。
- out/native-live-check.py 在独立 SQLite 与资源目录链接下使用 25000/25100/25500/26000/27000/27100/27200 端口，以真实 Mir2Ex 窗口输入登录、确认计费提示、选服、选角进入游戏。记录见 out/native-live-5hib8t9g；登录/选角640x480、游戏800x600，map=01 x=459 y=261 hp=18 mp=13。没有用协议模拟代替客户端，也没有截图。
- 构建目录的 EIServer PID34664 复验仍在运行，开发端口5000/5100/5500/6000/7000/7100/7200持续监听，保留供联调。测试实例已退出。
- 这证明启动及进入游戏链路；长期稳定性、协调停服、完整字体颜色与其他业务系统仍需后续验证。强制终止后状态文件可能残留，读取时必须同时检查 PID 与更新时间。


## 2026-11-17 修正高 DPI 下客户端整体窗口偏小
- 用户确认整体窗口偏小。此前按物理像素640x480/800x600验收不足：公共manifest声明system DPI aware，当前桌面144 DPI/150%，不会由Windows自动放大。
- CreatePrimarySurface将默认客户区按系统DPI/96放大，内部画布保持640x480/800x600；Display WindowWidth/WindowHeight显式配置仍表示物理像素。没有修改公共manifest或服务窗口。
- Win32 Release构建通过。真实客户端隔离测试 out/native-live-btta88ru 实测登录/选角960x720、游戏1200x900，缩放后鼠标输入正常；移动459,261到460,261，Alt+Q确认正常退出0，SQLite保存460,261，第二次登录恢复同坐标且再次正常退出0。out/debug/Mir2Ex.exe已更新。
- 状态增加notice/input_delayed：前一次移动失败因进图后的输入等待未解除，测试已按状态等待。
- IDA启动服务可复用，但open仍因未找到IDA安装目录失败；bootstrap ida返回missing-definition。原版运行曾观测到DPI unaware，不能据此宣称已完成原版各页面尺寸反编译验证。


## 2026-11-17 窗口DPI最终约定
- 用户确认保留DPI放大，但不能写死。已撤销本轮尚未提交的原尺寸回退，继续使用9bdb779中读取系统DPI并MulDiv计算窗口的实现。Release重建通过，out/debug/Mir2Ex.exe与构建产物SHA256一致。
- 本轮原尺寸临时测试通过返回选角，但恢复坐标460,262与第一次移动快照460,261不一致，原因未确认；不能把该次测试记为全通过。隔离实例已退出。后续需区分移动中快照与最终保存坐标。开发服务器34664保留运行。


## 2026-11-17 EIServer 控制台日志
- 六个静态库的InsertLogMsg与宿主启动消息统一经过Service/ConsoleLog.h，格式[HH:MM:SS.mmm] [服务名] 消息。线程间串行输出完整行；控制台WriteConsoleW，重定向WriteFile输出UTF-8。未增加原始包体/账号密码转储。
- Release构建通过。out/native-live-5u3_51lk真实客户端登录、移动、返回选角、再次进图及重新启动客户端登录全部通过，退出码0；退出前客户端位置、SQLite、重进位置均460,261。日志44行均匹配统一格式并能严格UTF-8解码。
- 旧开发服务器34664在无客户端连接时通过Ctrl+C结束。out/debug/EIServer.exe已更新并用可见控制台启动PID33900；控制台IsWindowVisible=1，读取控制台缓冲区确认All services ready；快照六服务就绪、七默认端口正常。未重定向其标准输出，日志直接显示。
- 此前单次坐标差一格未复现，尚不能定性为已修复。临时联调脚本仍在out/native-return-check.py，整理为正式脚本尚未完成。


## 2026-11-17 协议正文日志与可见终端
- LoginSvr/DBSvr业务边界及GameGate客户端收发边界增加MSG RX/TX日志，含socket、id、recog/param/tag/series及正文。六位正文解码后输出，控制字节/二进制高字节转义，不能整体解码的记录输出wire十六进制预览；正文最多512字节并标记截断。登录/注册/改密码/更新账号请求遮盖正文。Debug MessageLog默认1，设置0关闭（重启生效）。
- 按用户要求移除CMake MIR3_DEV_RUNTIME_DIR，直接使用进程当前工作目录./中的EIServer.ini，不再回退开发机out路径。
- Release构建成功，真实客户端日志out/native-live-oo8w1t1z验证2001遮盖、104正文Mir3Local及双向消息；该轮返回选角前数据库坐标460,262与客户端460,261不同，已保留失败证据，不宣称全流程通过。
- 曾经ShowWindow/IsWindowVisible为1但窗口矩形0x0，不足以证明终端可见。最终通过wt.exe创建真实Windows Terminal窗口，标题EIServer - EIServer.exe，窗口矩形(322,329)-(1490,953)，已前置。当前服务器PID31528，七默认监听端口正常。运行目录out/debug，窗口由cmd /k保留便于看到启动错误。


## 2026-11-17 进图角色后方特效彩色失真修复
- 用户指出角色后方入场特效五颜六色。SM_LOGON创建_SKILL_SPACEMOVE2，旧表指向Magic.wil110..128，其中实际有效帧110..119，资源解码显示蓝白传送光柱。
- 像素回读复现：WHWilTexture枚举16位格式，但CreateSurface的DDSD_PIXELFORMAT被注释，实际返回32位00ff0000/0000ff00/000000ff；上传仍按WORD逐像素复制，导致颜色/布局错乱。恢复纹理及中间面的格式标志，明确选择RGB565，上传前清零透明区域，并支持C1/C2/C3有色游程。
- 增加--check-arrival-textures：使用真实DirectDraw纹理与独立WIL解码像素比较，不截图。修复前10帧rgb565=0；修复后10帧rgb565=1、透明像素正确、mismatch=0，退出0。报告out/debug/arrival-texture-check.txt。
- Release构建成功，out/debug/Mir2Ex.exe已更新。out/arrival-live-check.py隔离实测登录进图并在入场播放后保持存活，目录out/native-live-siyddt4d。未完成原版入场特效全调用链反编译，不把资源核对写成完整IDA验收。


## 2026-11-17 修复返回选角过渡仍发送移动
- out/native-live-oo8w1t1z日志明确显示1009退出消息后仍有3011走路请求，导致客户端快照460,261与数据库460,262不同。DefMainWndProc只拦截窗口消息，但角色UpdateMotionState仍轮询鼠标，且动作队列可能尚有待发操作。
- InputDelayed同时判断退出过渡，统一阻止鼠标轮询和动作入口；ConfirmExit在发送Logout前清空待发动作。保持场景过渡动画和网络接收。
- Release构建通过。退出回归32项全通过，包括新增加的pending取消、轮询输入零发包。真实客户端out/native-live-_1yoqbtd验证移动、返回选角、重进与重登，客户端/SQLite位置均460,261；日志1009到选角重新连接之间无3011/3013。out/debug/Mir2Ex.exe已更新。

## 2026-11-17 EIServer 消息循环日志
- Run() 打印每条 GetMessage 返回的 MSG：hwnd、message、wParam、lParam、time、pt，包括 WM_TIMER 和 WM_QUIT；GetMessage 失败打印错误码并返回失败，WM_QUIT 记录并返回退出码。
- Release EIServer 构建通过，已更新 out/debug/EIServer.exe。实际启动六个服务就绪，注入 WM_APP 的 0x1234/0x5678 参数可见，WM_QUIT 正常记录且退出码为 0；日志 out/debug/msg-loop-check.log。

## 2026-11-17 将真实客户端联调回归纳入源码
- 新增 Client/Tools/test_native_roundtrip.py 与 Client/docs/native-roundtrip.md，覆盖原生窗口登录、移动、返回选角、再次进图、退出重启后的坐标持久化，不截图、不用协议客户端替代。可选构建/配置目录和端口偏移，验证进程存活及状态文件时效。
- EIServer从固定bin-dir运行，cwd指向每轮隔离目录；客户端会主动切换到自身EXE目录，必须复制客户端到隔离目录，否则会读开发配置。已增加输入账号前的端口断言。服务端数据库、互联端口与地址显式覆盖到隔离环境，资源为目录链接。
- 默认端口偏移20000回归 out/native-roundtrip-qj1cixoz 通过：两次客户端退出0，坐标460,261一致。
- 配置键改为显式覆盖后，以偏移21000回归 out/native-roundtrip-8gezfrph：六服务、登录/移动/返回选角/重进通过，但重启恢复断言失败。第一次返回前461,261，DB和重进一致；重进后23:19:19.244又收到3011请求，下一次登录462,261，脚本退出时读到461,261。需检查真实按键轮询/重进动作和断开落库时序，不能宣称完整回归通过。脚本保留失败断言，未放宽标准。测试实例均已退出。

## 2026-11-17 修正重登回归的落库等待条件
- 旧脚本在第二次进图退出后只等数据库坐标不等于初始459,261；第一次返回选角已满足该条件，因此会把上次保存当作本次保存完成。改为读取确认退出前新鲜客户端快照，等待数据库精确匹配该位置，再重启客户端校验。
- 回归增加第二次进图后主动移动，让两次保存位置不同，保留精确坐标断言。偏移21000下 out/native-roundtrip-n82sf7y6 全通过；加强用例 out/native-roundtrip-mx02yn1o 也通过，首次保存460,261，第二次退出前/数据库/重登均461,261，两次客户端退出0。
- 先前8gezfrph的非预期3011来源仍未证明，不把脚本修正写成客户端移动修复。静态排查另发现Actor.cpp五处GetCursorPos+ScreenToClient后直接交给逻辑坐标动作入口，而WHDXGraphic.h窗口消息路径会按客户区/画布缩放；DPI下轮询坐标不一致待实现与验证，本轮未改。

## 2026-11-17 统一鼠标轮询与消息坐标
- Actor.cpp五处持续按键路径此前只做ScreenToClient，直接把DPI放大后的物理坐标传给逻辑动作入口。新增WindHorn/InputCoordinates.h，窗口鼠标消息与五处轮询统一按实时客户区/画布比例换算；保留负坐标，不写死DPI比例。
- Release Client/InputCoordinatesCheck构建通过，独立真实窗口坐标检查5例失败0（150%、100%、横纵不同缩放、窗口外坐标）。out/debug/Mir2Ex.exe已更新。
- 真实客户端回归out/native-roundtrip-fe8lntuz完整通过：登录/选角960x720、游戏1200x900；首次移动保存460,261，重进再移动后退出前/数据库/重登461,261，两个客户端正常退出0。该回归使用窗口消息输入，尚未验证物理长按完整行为，不据此声称先前额外3011的根因已确认。

## 2026-11-17 验证真实长按输入
- 原生联调脚本新增--hold-mouse可选模式：确认鼠标未被按住、激活客户端并等待前台焦点，按当前客户区计算屏幕位置，注入左键持续按住1.8秒；finally释放并恢复指针。至少向东走两格且纵坐标不变才继续保存重登验收。
- 前几轮因无法激活窗口/检测到鼠标已按住而在注入前退出，不能视为游戏失败或测试通过。用户明确回复现在可以测试后，out/native-roundtrip-1w5cb80z全流程通过。
- 150% DPI下游戏1200x900，长按459,261到462,261；返回选角DB与重进一致，重进再次移动到463,261，确认退出前/SQLite/重登均463,261，两个客户端退出0。鼠标已释放，测试进程均结束。此前非预期移动不能在缺少输入现场证据时归因于客户端。

## 2026-11-17 原生盔甲穿脱回归与音效崩溃修复
- 增加可选物品状态输出Tools/ItemDebugState.h：背包/装备实例ID名称槽位、手持物品、装备请求、面板位置。--equipment隔离用例通过真实面板三次穿戴两次卸装，检查物品守恒、HP/MP、面板位置、返回选角和重登恢复。
- 首轮h_2pmnqk与复现i81t_yl8均在收到穿戴成功后访问冲突。启用构建目录/MAP后第二次故障RVA0x123998位于PlayItemEffect内联的音效索引查找；SoundList.wwl缺失时m_pWavListHeader为空，设置恢复m_bPlay后SeekFileName仍解引用。新增空索引检查，并把读取从全局构造移至InitMirSound（main已切换到EXE目录）；CreateFile按句柄判断成功，移除错误的GetLastError判断。
- Release构建通过，已更新out/debug/Mir2Ex.exe。缺索引lwu97zw2、存在索引a5o6bjev全回归通过，两次客户端退出0，hp18/mp13稳定，面板坐标不变；装备重进/重登恢复，数据库仅一条B26111700001且FLD_DRESS_ID一致。out/debug/SoundList.wwl为资源原文件硬链接；测试可选硬链接该索引。
- 尚不代表属性消息计数、替换装备、武器、消耗品和满背包全覆盖。旧GameGate锁及ObjectPlayer修改仍保留未提交，本轮没有把其独立变更混入。

## 2026-11-17 修正测试布衣图标及外观协议
- 用户指出拿药水往人物身上装。原测试盔甲Looks=0（Inventory帧0为红药）。直接解码资源确认男布衣Inventory/Equip帧940、女布衣950；Shape=1对应M-Hum第3000帧起的衣服段。初始化脚本迁移同名且Looks仍0的测试盔甲，out/debug数据库33/34各更新一行；没有修改实际自定义物品。资源检查器补V5000/V6000的21字节图像头支持。
- 属性验证增加防御2–5、魔防1–3的测试值及客户端实际属性快照。此前mh0ff3en只验证属性通过，换成非零Shape后6nohe26k暴露GetFeatureToLong字节顺序不匹配：服务器gender/wear/hair/weapon，客户端gender/weapon/dress/hair。统一输出顺序；18ie4n1z又暴露SM_LOGON直接编码旧结构体，改为同一个GetFeatureToLong编码。
- Client/EIServer Release构建通过并更新out/debug。最终t8e0ld87真实客户端三穿两脱、移动、返回选角/重进和重启重登全部通过；物品looks940/shape1，穿戴dress1、卸装dress0，重进重登仍dress1；防御/魔防按穿脱变化，hp18/mp13稳定，最终位置461,261一致。女布衣本轮只做资源/SQL核对，未做女性角色实服验收。
- sqlite schema两项测试通过，覆盖初始化、旧测试定义迁移和保留自定义外观。未把先前HP/MP、网关锁等未提交改动混入本提交。


## 2026-11-17 网关发送队列与女性角色整流程验证
- 按完整模块批量构建和验证，协议边界优先用自动断言，真实客户端只做整流程验收，避免逐点截图和重复登录。
- GameGate发送缓存改为有上限的动态字节队列；生产、发送、关闭和复用共用会话锁。部分发送只移除已发送字节，WSAEWOULDBLOCK保留队列，致命错误或容量超限断开连接。
- 正长度消息正文的memlen包含末尾NUL，网关必须去掉后再拼接结束符；负长度已编码消息保持实际长度。最初遗漏该约定导致5ir4rv5m进图失败，修正并补正文/空正文/已编码消息测试。旧实现覆盖的是NUL，并非丢失正文最后一个字节。
- Release EIServer与test_client_send_buffer构建通过；2万帧共420000字节TCP传输逐字节一致，受控部分发送和暂不可发送保留余量测试通过。out/debug/EIServer.exe已更新。生产仍使用同步发送，本轮不宣称解决慢连接阻塞。
- 原生脚本增加--gender female，使用女布衣950/Shape1。8hee9xfc完整通过三穿两脱、属性变化、移动、退选重进及退出重登；重登dress1、防御2–5/魔防1–3、坐标461,261，客户端退出0。此前bxhppgx3女性回归同样通过。测试使用真实客户端窗口消息与状态断言，无截图。


## 2026-11-18 消耗品成功与失败原生联调
- 状态输出新增背包物品cell（实际布局锚点）和bag_scroll_row。数组slot不是显示格子，盔甲占多格；首轮knaxq3_e按slot点击误选盔甲，协议日志显示两次均CM_EAT盔甲，不是药水服务失败。
- 原生脚本--consumable可与--equipment组合：隔离数据库放入DBSvr既有通用物品键G00080008000，测试定义命名Test potion并沿用红药图标0；不更改开发数据库。按实际锚点双击，检查服务端SM_EAT_FAIL后盔甲返包、SM_EAT_OK后药水消失、其他物品守恒与面板稳定；退出保存后DB通用物品为空，重登无药水。
- Client Release构建通过并更新out/debug/Mir2Ex.exe。3pjf0_jp完整通过消耗/失败返包、盔甲三穿两脱、退选重进及客户端重启重登，最终位置461,262与数据库一致，两个客户端退出0。日志out/item-consumable-test.log。没有验证受伤回血量、快捷栏消耗、堆叠、满背包或掉落拾取，不据此宣称整个物品系统完成。


## 2026-11-18 JSON-RPC架构按用户确认落地
- 最新结构：Server/Def/JsonRpc.h/.cpp提供共用命名管道与JSON-RPC 2.0协议；Client/ClientRpc.h/.cpp处理客户端方法与响应；服务端方法直接在Server/EIServer.cpp；Client/Test与Server/Test只保存JSON测试集。没有Test.h/.cpp或Server/Rpc文件。ClientRpc命名避开Windows SDK rpc.h。
- 仅显式--debug-pipe启用，端点为\\.\pipe\Mir3.Client.<PID> / Mir3.Server.<PID>。主线程轮询非阻塞管道，每行一个UTF-8 JSON消息；支持请求ID、通知、批量请求及标准错误码。目前只有state.get，尚未接入登录、背包等业务命令和事件序号等待，不可称为全部协议适配完成。
- JSON解析使用nlohmann/json v3.11.3单头文件（MIT许可在文件头），来源https://github.com/nlohmann/json/releases/tag/v3.11.3，已校验官方SHA256 9bea4c8066ef4a1c206b2be5a36302f8926f7fdc6087af5d20b417d0cf103ea6。只在独立JsonRpc.cpp包含，不能放入六个服务命名空间封装源列表。
- Client/EIServer Release构建通过。Client/Tools/run_rpc_tests.py以真实进程--pid运行JSON用例；客户端及服务器各6项通过（状态、未知方法、参数类型、请求格式、解析错误、批量通知），无登录点击/截图。初版测试器读写同一同步句柄并发阻塞，改为先写再读已通过。
- 丢弃拾取检查发现通用物品被当装备记录处理及DB失败未返地图；ObjectPlayer.cpp有本轮修复但尚未通过真实业务RPC验证，未混入本提交，未部署新EIServer到out/debug。


## 2026-11-18 精简JSON解析依赖
- 按用户要求移除919975字节/24765行的nlohmann json.hpp，换成PicoJSON v1.3.0单头文件26102字节/1010行（BSD-2-Clause许可保留在文件头），体积减少97.2%。来源https://github.com/kazuho/picojson/blob/v1.3.0/picojson.h；SHA256 5ddf7276d04926da7be243e7af49258e78cc27278ee9097ba45b942c7a6b5f9d。
- JSON-RPC架构与方法保持不变；启用64位整数保证大整数请求ID精度，检查解析结束位置拒绝尾随内容。业务实现仍与解析库隔离。
- Client/EIServer Release构建通过，真实两端各9项JSON测试全部通过，新增9007199254740993 ID、Unicode代理对/转义字符和尾随非法数据。构建日志out/jsonrpc-small-build.log。


## 2026-11-18 真实客户端登录RPC与事件等待
- ClientRpc新增session.login、session.select_server、session.enter、notice.dismiss；调用原登录动作、选服发送及选角业务方法，实际经过原网络编码/服务器/客户端接收，未用坐标输入或模拟网络客户端。抽出选服和进入角色方法供GUI与RPC共享；选择角色时立即同步g_szCharName，避免依赖渲染帧写回。
- 主线程记录带递增序号的state/submitted事件，环形保存256条，events.wait接受after/timeout_ms；等待期间主循环继续处理网络和渲染，超时返回-32001，历史过期-32003，未来序号拒绝。操作返回submitted，不声称服务器已成功。当前事件尚未包含逐条网络收发包，后续协议覆盖还需补齐。
- Client/Test/login.json覆盖密码错误及恢复、正常登录、通知确认、无效服务器/角色拒绝、选服、选角、进图，共13步；通用测试器增加await_state，经事件长轮询等待，不再按固定sleep或重复登录探测业务状态。
- Client/EIServer Release构建通过。使用现有隔离fixture 3pjf0_jp，真实两端9项服务端RPC基础测试与13步客户端业务测试全部通过，单轮约5秒；日志out/rpc-login-runtime.log。测试进程结束，没有操作鼠标/截图。尚需独立fixture自动创建、全协议清单、角色创建删除及后续业务测试集；没有把局部通过当完整适配。
- 已读取用户更新AGENTS.md：全部协议先经JSONRPC连通，最后统一UI调试；保留该用户修改未混入本提交。


## 2026-11-18 独立JSON测试运行目录
- run_rpc_tests.py新增--launch：从指定bin-dir/config-dir创建全新out/rpc-*运行目录，专用端口、独立SQLite、Data/Map/Sound目录链接及SoundList硬链接；只复制客户端二进制，EIServer二进制路径稳定。JSON测试集fixture数组定义参数化SQL种子，不再借用旧native-roundtrip目录。
- 服务端EIServer.cpp新增state.wait_ready，主循环继续启动六个服务时RPC延迟响应；服务器JSON用例等待全部ready后再启动客户端。RPC读响应超时覆盖服务启动60秒上限。
- 一条命令：python Client/Tools/run_rpc_tests.py Client/Test/login.json --launch。默认二进制build-eiserver/Release、资源配置out/debug，支持覆盖目录和端口偏移；--pid仍支持附着既有调试进程。结果及错误写入fixture/result.json，日志runtime.log，finally仅清理本次子进程。
- EIServer Release重新构建通过；全新rpc-2_mwya81独立运行通过服务端10项和客户端13步，结果passed，运行5.165秒（不含目录准备）；已核对三个资源目录为Junction。未宣称非登录协议已完成，后续继续JSON业务覆盖。


## 2026-11-18 物品业务RPC、拾取修复及持久化闭环
- ClientRpc接入item.equip/unequip/use/drop/pickup、游戏公告关闭和session.return。装备规则与RequestTakeOn从鼠标处理抽为共用业务方法；RPC使用既有背包提取、穿脱请求、消耗、丢弃、拾取网络入口。状态增加ground_items/use_pending/drop_pending。没有用坐标输入或截图。
- 修复通用物品拾取：按G标识恢复GENERALITEMRCD并放入通用物品列表，正确转换发送记录，避免按装备结构读取并错误访问装备表。装备拾取DB更新失败则放回地图；成功更新重量并返回成功。数据库失败回退分支本轮仅代码核对，未做故障注入。
- 同批纳入此前保留的HP/MP修复：属性重算保留当前HP/MP并限制新上限，登录因旧记录没有当前HP/MP字段而明确初始化。正常穿脱及重进hp18/mp13稳定；尚未验证受伤状态重算或战斗回血量。
- JSON测试支持include复用登录流程、继承fixture和database_expect，动作回执后更新事件游标，await_state先检查当前状态再等新事件，避免漏掉已完成状态。入场公告并非每次存在，关闭操作可重复调用。
- Client/Test/items.json完整覆盖错误槽位返包/不存在物品拒绝、穿脱、药水落地拾取使用、装备落地拾取、重新穿戴、退选保存及重进。Client/EIServer Release构建通过；rpc-6cpl29oa的51项断言全部通过，运行10.809秒。数据库确认药水0条、盔甲1条、FLD_DRESS_ID=B26111800001。
- 尚未覆盖堆叠、快捷栏、满背包、交易/商店、跨角色拾取、地图重启落地保存；不能将本批当整个物品或全部协议适配完成。早先equipment-pointer临时日志与其他无关改动未混入本提交。


## 2026-11-18 实际收发证据与协议覆盖清单
- 仅RPC启用时记录真实SendPacket完成/失败以及登录、选角、游戏解码入口的消息码和长度；不记录账号密码正文。事件含packet.sent/received/send_failed，protocol.get提供会话累计计数，避免环形事件历史过期后丢失覆盖统计。收到消息并不等于业务解析成功，业务仍由状态/数据库断言证明。
- JSON测试wire_expect按协议符号断言真实收发次数；登录与物品共新增21项消息断言。测试器每轮输出protocol-coverage.json，汇总Client/Common/Protocol.h、Client/Define.h、Server/Def/Protocol.h中声明的CM/SM消息与源位置/别名，列出未观察项和未知实际消息。范围明确不含其他命名空间常量及源文件直接数字opcode，尚不能叫全部协议最终清单。
- Client Release构建通过。rpc-lpyhxjr0完整72项通过，运行10.408秒；199个已声明方向/消息码组合，本轮观察38个。新增注册/改密、创建删除角色、移动战斗、组队、技能等仍在缺口清单，后续按完整场景补齐。


## 2026-11-18 根目录统一测试编排
- 所有 JSON 测试集迁移至 Test/Client、Test/Server，通用执行器和 fixture/协议统计工具从 Client/Tools 迁至 Test。Client/Server 仅承担 RPC 入口和业务响应。
- 新入口：python Test/run_rpc_tests.py Test/Client/items.json --launch。自动启动及 --pid 附着均输出 out/rpc-*/test.log 和 result.json；客户端另有协议统计，自动启动保留 runtime.log。Test/README.md 说明用法和边界。
- 真实两端回归 rpc-w212xy2b：72 项全部通过，10.882 秒；附着不存在 PID 的失败路径 rpc-5mjj41jc：日志和 failed 结果正确落盘，退出码 1。仅迁移 Python/JSON，无需重编 C++。


## 2026-11-18 按子系统组织CSV与原生Test.exe
- 用户改为CSV场景，新增根CMake下Test控制台目标（Test/Test.cpp）；Test.exe接收CSV、--client-pid/--server-pid、--from/--to（含边界步骤号）、--log、--timeout-ms及只读--database。每行自由切换Client/Server，日志记录响应与断言差异，失败即停并返回1。
- 业务按子系统放Test/Login.csv、Inventory.csv；基础协议为RpcClient.csv/RpcServer.csv，测试种子同名.fixture.csv。移除四个旧JSON测试集与Python RPC执行器；launch_csv.py只负责隔离fixture、启动两端及调用原生执行器，保留协议覆盖报告。
- CSV支持UTF-8 BOM、引号转义、逗号和跨行单元格；参数/预期用key=value分号列表。支持state.wait、wire.expect、Database query和基础协议raw。现有服务端state.get仅证明服务/网络状态，业务结果仍检查客户端状态及数据库。
- PicoJSON通用相等比较会把int64转double并改写值；执行器采用精确递归比较，新增相邻大整数拒绝验证，未修改第三方头文件。
- Test Release构建通过；真实全场景rpc-4v69c9op含67步骤+10服务端检查全部通过，场景9.329秒，日志out/csv-validation/items-final.log。客户端基础协议也通过；8项执行器验证覆盖CSV字段、起止范围、空/逆序范围、精度和失败停止。按子系统重命名后rpc-mz7a0_87再次验证指定第6步及自定义日志成功。
- 角色创建删除未在本轮修改：用户要求优先改CSV执行器。后续继续各子系统协议闭环，不做截图调试，也未宣称整体适配完成。


## 2026-11-18 角色创建RPC与Character场景
- CreateCharacter抽为UI/RPC共用业务入口，新增character.create及选角notice.dismiss；state.get暴露characters和creation_result。待创建回包时禁止进入角色，沿用现有客户端两角色上限；服务端旧三角色上限仍未统一，本轮未绕过客户端验证服务端满员分支。
- 修复SM_NEWCHR_FAIL：集成DBSvr通过wParam报告旧错误码，客户端此前只读nRecog；兼容原有nRecog映射，正确识别重名并恢复后续创建。
- Test/Character.csv及同名fixture新增完整创建场景：非法职业/性别/名字、重名请求回包、恢复、女性法师创建、列表刷新、客户端上限、数据库角色/初始装备/药水及真实进图。Client Release构建通过；rpc-leoyrz4q的34步骤与10服务端基础检查全部通过，场景3.796秒，日志out/csv-validation/character-final.log。
- 删除仍未实现：OnDelChar发送name/confirmation，DBSvr却解为_TCREATECHR且忽略SQL失败，后续需要核对确认语义和账号绑定后修复；本轮只完成创建闭环，不宣称角色子系统或全部协议完成。


## 2026-11-18 删除角色协议、密码校验与事务回滚
- 从原版宝藏传奇3/CmList.dat按现有LoadLegacyText算法解码，#228明确要求游戏分区密码；Godot只转发name/confirmation，不能把确认文本当角色名。
- DBSvr按name/password解析CM_DELCHR，选角查询将账号关联当前连接，打开/关闭连接清理关联；删除通过Mir2_Account的FLD_PASSWORD校验，并按账号+角色名限定SQL。账号关联本身不等于完成登录凭证认证，其他旧认证链路未在本轮重构。
- 新增CConnection事务接口（SQLite/ODBC），删除角色、普通物品、装备和技能在同一数据库锁/事务中完成；失败回滚并发SM_DELCHR_FAIL，不再无条件成功。ODBC分支编译通过但未做运行验证。
- Client UI/RPC共用DeleteCharacter，character.delete接收name/password；状态暴露deletion_result，待回包锁定创建/删除/进图，收到成功或失败后才请求列表。CM_DELCHR日志体脱敏，最终日志3个删除请求均脱敏且无test123正文。
- 故障注入测试在删除装备时RAISE ABORT，验证已删角色与普通物品回滚、技能值完整、其他账号角色保留。Test/CharacterDelete.csv和fixture覆盖38步骤，加10服务端基础检查全部通过，rpc-m2xs0xwv耗时7.052秒（场景4.813秒），日志out/csv-validation/character-delete-verified.log。
- 首轮删除检查通过后，保留角色进图触发真实EIServer退出：sqlite_minimal的技能宽表与GetHumanMagicRcd/SaveHumanMagicRcd逐技能行格式不符。已将新建脚本对齐FLD_MAGICID/LEVEL/USEKEY/CURRTRAIN及复合主键，带技能行进图复测通过。未迁移任何已有数据库；旧宽表数据迁移、技能语义全链路仍需继续。
- 尚未覆盖多会话同时在线删除、名字复用冷却、账号认证全链路；不能视为角色/技能系统全面完成。


## 2026-11-18 技能列表与快捷键持久化
- ClientState暴露skills/skill_shortcuts；skill.bind调用现有BindMagic，先校验已学ID和0..12槽位，仍走CM_MAGICKEYCHANGE真实网络。
- 服务端旧_TCLIENTMAGICRCD布局不符合客户端74字节MIR3记录；新增GameSvr/MagicWire.h统一转换全量列表和新增技能包，包含名称、需求、修炼、职业等字段并零初始化。GetMagicInfo索引快速路径补范围判断，列表转换按定义ID查找而非盲按数组下标；未知定义不再读越界数据。
- 首轮Skills场景证明UI本地绑定改变但DB保持旧值：ProcessUserMessage默认分派丢弃wParam快捷键且把技能ID放错参数。为CM_MAGICKEYCHANGE明确传nRecog技能ID及wParam键值，复测保存/重进一致。
- Skills.csv及fixture使用两条技能定义和学习记录，覆盖已学列表、非法ID/键位、F12绑定/占用冲突清理、DB级别/键值/修炼断言和重进。Client/EIServer Release构建通过；rpc-8y5uoicq的29步骤+10服务端基础检查全部通过，场景8.890秒，日志out/csv-validation/skills-final.log。
- 未验证新增技能包实际学习流程、施法、升级、移除或大批量列表；服务端MagicHandler的旧技能常量与客户端Mir3SkillIds存在编号差异，施法效果映射仍需核对，不能以本轮列表/持久化通过宣称整个技能适配完成。

## CSV 子系统组织约定

业务脚本统一放在根 Test，按子系统及场景命名；同一场景可交替调用 Client/Server/Database。Test/README.md 已列出登录、角色、物品、技能及 RPC/Runner 基础检查，并说明同名 fixture、稳定 step 编号、区间前置状态和自定义日志。已检查七个现有子系统脚本的表头、步骤编号、目标端及业务 fixture 配对；此次只完善组织说明，未修改业务实现。

## 技能施法协议适配

新增 Test/SkillsCasting.csv 及同名 fixture，skill.cast(key,x,y) 复用 CMyHero::CastShortcut 与键盘施法入口；RPC 报告已解析的 spell_reply_id 和动作时序 cast_ready。技能编号抽到 Shared/Mir3SkillIds.h，客户端原路径保留转发头，服务端 _SKILL_ 常量映射到同一 MIR3 枚举。服务端 SpellXY 按 CM_SPELL 的技能 ID 查找已学记录，并修正目标标识高低字组合。SM_HEALTHSPELLCHANGED 原来只更新旧 HP/MP 字段，现统一调用 OnHealthSpellChanged 同步能力字段和旧字段，修复回包扣蓝但客户端仍显示旧 MP 的问题。保留直接技能在动作锁期间的原有分发路径。

Client/EIServer/Test 构建通过；最终施法 CSV 26 步通过，真实两次 CM_SPELL/SM_MAGICFIRE/SM_HEALTHSPELLCHANGED，MP 18→12→6，未绑定与低蓝拒绝。日志 out/csv-validation/skills-casting-verified.log。回归 Inventory 67 步、Skills 29 步通过，各自运行前 RpcServer 10 项通过；verify_runner.py 的 8 项通过。Test.exe 的 state.wait 仅在响应变化时输出 WAIT。实体目标伤害、其他技能效果和旧数据库技能表迁移仍未验证/完成，不能把本次火球术坐标场景视为全部战斗适配完成。

## 怪物受击与治疗术联调

新增 Test/SkillsHealing.csv / fixture，真实怪物攻击使 HP 16→12，客户端治疗术使 HP 12→16、MP 18→12，断言 SM_STRUCK、SM_MAGICFIRE、SM_HEALTHSPELLCHANGED 和服务端健康。恢复 ProcessMonster 中整段被注释的数据库刷新循环，跳过未配置刷新组的轮询；校验刷新地图/怪物数组索引。修复怪物名称错误使用行为类型二次索引种族数组。当前资源没有 searchTable.tbl，新增 LoadSearchTable：有效文件仍加载，缺失/短读时生成 15 圈共 960 个由近到远的搜索偏移，统一服务和独立窗口入口。

恢复搜索后复现 EIServer 0xc0000005：怪物攻击误用 m_pUserInfo->GetWeapon，已将武器损耗计算限定到具有玩家记录的角色。去掉临时 MONDBG 诊断。EIServer 构建通过；SkillsHealing 最终 24 步通过（out/csv-validation/skills-healing-final.log，9016ms），SkillsCasting 回归 26 步通过（out/csv-validation/healing-casting-regression.log），两次 RpcServer 各 10 步通过。没有截图或物理输入测试。怪物死亡后刷新、掉落、各行为类型、实体技能伤害尚需后续验证；当前刷新配置仍使用旧服务端怪物数组索引语义，稀疏数据库 ID 映射尚未处理。

## 怪物数据库编号与实体名称查询

怪物定义保存 TBL_MONSTER.FLD_INDEX 为 databaseId，保留 nIndex 的运行时数组下标语义；先加载定义，再将刷新配置 FLD_MONINDEX 按数据库编号解析一次为运行时下标，缺失编号标记 -1 并跳过。新增 MonstersSpawn.csv/fixture，数据库编号 3、1007，刷新 1007；额外不存在编号 1 不得误刷数组第 1 项。Client state.get 新增 actors(id/name/x/y/dead) 和 actor_count；actor.query_name(index) 使用当前可见数组下标调用现有 SendQueryName，验证 CM_QUERYUSERNAME/SM_USERNAME，不依赖鼠标悬停。怪物初次进入视野名字为空符合当前客户端按需查询路径，本次没有改动名称协议格式。

Client/EIServer 构建通过。MonstersSpawn 29 步通过（out/csv-validation/monster-name.log，9203ms，runtime rpc-q861_yka）：仅一个实体、名称 HealingProbe、怪物受击 HP16→12、治疗 HP12→16/MP18→12、无效实体下标拒绝；原编号0的 SkillsHealing 24步回归通过（out/csv-validation/monster-id-zero-regression.log，9000ms），两次 RpcServer 各10步通过。未验证怪物死亡刷新/掉落、特殊召唤怪物硬编码索引和实体目标伤害。

## 实体目标火球术与死亡协议

skill.cast 新增 target 参数（当前 actors 下标），从真实客户端实体取得坐标、网络标识和 FEATURE，复用 CastShortcut；不存在/已死亡目标拒绝。actors 状态增加 hp/max_hp，max_hp=0 表示尚未收到血量。服务器 SpellXY 对非空目标使用地图对象校验，RM_DELAYMAGIC 生效前重新校验目标在原目标坐标容差内；修复 CMirMap::IsValidObject 将比较误写成赋值，原来只要找到任意移动对象就通过。

新增 CombatMagic.csv / fixture：低血量怪物收到真实火球术后 HP=0、dead=true、MP18→12，验证 CM_SPELL/SM_STRUCK/SM_NOWDEATH，死亡和不存在目标拒绝及服务器健康。首次死亡断言期望 SM_DEATH 失败，实际抓到的是 SM_NOWDEATH(34)，按已有服务端 Die 路径修正场景断言，没有篡改死亡协议实现。Client/EIServer 构建通过；最终战斗24步通过（out/csv-validation/combat-magic-final.log，8125ms），坐标施法回归26步通过（out/csv-validation/combat-coordinate-regression.log，7313ms），两次 RpcServer各10步通过。尚未验证非致死精确伤害公式、怪物经验/掉落/重新刷新或无效原始网络标识攻击；没有界面截图操作。

## 战斗经验、跨级与保存恢复

state.get 新增 level/experience/next_experience/return_ready，后者复用 ExitFlow::Guard 的真实战斗冷却和退出转换状态。新增 CombatExperience.csv/fixture，击杀奖励1000经验；WinExp 原先 if 只升级一次，改为按新等级阈值循环扣除经验，限制在 MAXLEVEL 表内并排除零阈值。保持原 NEEDEXPS 和等级索引，没有更改经验平衡。

Client/EIServer 构建通过。首次单级奖励250已验证2级剩50，但立即退出被正常战斗冷却拒绝；脚本补上拒绝断言和 return_ready 等待。最终跨级场景33步通过（out/csv-validation/combat-experience-multilevel.log，runtime rpc-q6af1hmr，15766ms）：1级→4级剩100，冷却前拒绝退出，冷却后保存数据库(4,100)，重登恢复及SM_WINEXP/SM_LEVELUP。CombatMagic增加小额经验断言(1级、1经验)，24步回归通过（out/csv-validation/experience-small-regression.log，8109ms）；各自RpcServer10步通过。等级上限、经验表与原版数值一致性、组队经验和召唤物归属尚未验证。

## 普通怪物掉落、行走拾取与保存

InitMonItemInfo 按数据库怪物编号映射掉落记录，跳过不存在定义，修正原来把FLD_MONSTER当数组下标及<=越界判断。原函数本来由InitMonRaceInfo末尾调用，本轮排查时新增的重复启动调用已移除，最终未改变初始化调用次数。ScatterBagItems补上m_xTGenItemRcd普通物品；MonGetRandomItems启用1/N概率(N<=0禁用)，装备实例只在装备分支分配，消除普通物品/金币分支无用的装备分配。

客户端新增WalkAdjacent和world.walk(x,y)，检查相邻格、通行和动作时序，调用既有SetMotionState发真实CM_WALK；脚本没有改坐标或绕过服务器。MonstersLoot.csv/fixture验证稀疏怪物1007掉落药水、禁用概率0与不存在怪物1的记录无额外物品、远距离走路拒绝、走到散落格(459,260)、拾取、DB物品数1/位置一致、重登恢复。Client/EIServer构建通过（中途旧测试进程占用EXE导致一次LNK1168，在该测试终止后重新链接成功）；最终39步+RpcServer10步通过，out/csv-validation/monster-loot-final.log，runtime rpc-4ceovflh，场景16859ms。随机概率分布、FLD_COUNT多份同类物品、金币/装备掉落及背包满仍待完善验证，不能据此声称掉落系统全部完成。

## 怪物金币掉落与钱包上限

ScatterBagItems新增金币地图物品：使用现有金币名称/金币图标编号、CMapItem.nCount金额和RM_ITEMSHOW，成功放入地图后清零怪物持有金币。修复金币配置数量0导致rand()%0及数量1恒为0，正数量至少1金币，多条配置累加。Client state.get新增gold，来自客户端实际余额m_nGlod。

Client/EIServer构建通过。MonstersGold.csv40步通过（out/csv-validation/monster-gold.log，rpc-risgzv86，16719ms）：余额100→101，SM_GOLDCHANGED、DB与重登一致，同时配置数量0未崩溃也未额外掉钱。MonstersGoldCap.csv39步通过（out/csv-validation/monster-gold-cap.log，16703ms）：5000000满额钱包不增加，金币不消失，保存和重登后仍在地面；两次RpcServer各10步通过。未验证玩家主动丢钱、概率/数量分布、多种物品混合散落、金币图标视觉及数值溢出极端配置。装备掉落、多份同类物品和死亡刷新仍需后续完成。

### CSV 子系统归属约定
- Test/README.md 按主要验证目标维护脚本索引；跨客户端、服务端与数据库的步骤保留在同一场景。补齐金币场景与战斗经验范围。
- Test.exe 的文件、步骤区间和日志参数保持不变；verify_runner.py 的 8 项检查通过。

### 怪物装备拾取事务与回归
- 新装备拾取改为同步创建数据库身份，事务提交后加入背包；失败恢复地面物品。移除原 DB_MAKEITEMRCD2 指针大小编码与提前移除物品的路径。当天编号按数值取最大值，兼容未补零记录。
- MonstersEquipment.csv 41 步通过（含 9/10 -> 00011、穿戴与重登）；MonstersEquipmentFailure.csv 39 步通过，触发器拒绝 INSERT 后地面装备保留且数据库无新增。
- Inventory 回归发现恢复搜索表后掉落位置改为相邻格，已补入真实行走；原步骤号乘以 10，新增到达检查，71 步通过。日志位于 out/csv-validation/monster-equipment.log、monster-equipment-failure.log、equipment-inventory-regression.log。EIServer Release 构建通过；尚未验证 ODBC 并发分配。

### 相同普通物品的独立实例
- 普通物品的旧 G 键描述属性，两瓶相同药水会相同；客户端 ID 现结合活跃记录分配，使用/丢弃按请求 ID 定位具体记录。消耗或丢弃使旧 ID 失效，拾回分配新 ID，避免旧请求操作另一瓶。普通物品发送直接使用真实记录，不再构造临时副本。
- InventoryDuplicates.csv 39 步通过：两瓶相同药水、指定第二瓶丢弃拾回并使用、第一瓶保留、数据库数量 1 -> 重登 -> 使用 -> 0。Inventory 71 步、MonstersLoot 39 步、MonstersEquipment 41 步回归通过；EIServer Release 构建通过。日志位于 out/csv-validation/inventory-duplicates.log 和 duplicates-*-regression.log。
- 相同符咒/火把的装备槽位仍使用旧属性键，尚未验证多份相同物品的装备行为。

### 多件怪物掉落
- 非金币掉落按 FLD_COUNT 生成独立物品；数量 0 不掉落，每条配置只抽取一次概率，金币保留原数量规则。整理 MonGetRandomItems，去掉重复清零和失效注释。
- MonstersLootCount.csv 43 步通过：两瓶药水分散掉落、逐件行走拾取、独立 ID、数据库数量 2 与重登恢复。MonstersEquipmentCount.csv 45 步通过：两件装备分配 00011/00012，穿戴一件、背包保留另一件并保存恢复。两场景包含数量 0 的负向配置。MonstersGold 40 步回归通过，EIServer Release 构建通过。日志 out/csv-validation/monster-loot-count.log、monster-equipment-count.log、count-gold-regression.log。

### 背包容量与整笔存档
- Client/Server 共用 Shared/Mir3InventoryLimits.h 的 60 格容量，服务端容量判断排除已穿戴物品，AddNewItem 同样使用该判断；RPC 状态新增 bag_count/bag_capacity。InventoryFull 46 步通过：60 瓶 + 已穿戴护甲、满包拒绝拾取、消费腾位后拾取、保存重登数量不变。
- 满包测试暴露 DBSvr 保存无事务，数据库检查曾读到只写入 21 瓶的中间状态。角色/技能/普通物品现在同一事务保存，持管理器锁，任何写入失败回滚；按消息声明数量解析记录。InventorySaveFailure 34 步通过，触发器拒绝 INSERT 后药水和角色坐标回滚到先前存档。失败只记录服务端日志，尚无客户端保存失败通知。
- Client/EIServer Release 构建通过；Inventory 71 步、Skills 29 步回归通过。日志 out/csv-validation/inventory-full.log、inventory-save-failure.log、full-inventory-regression.log、save-skills-regression.log。大尺寸物品布局空间与满包卸装尚未验证。

### 满包卸装
- ServerGetTakeOffItem 在清除装备槽前检查 IsEnoughBag；满包通过现有 SM_TAKEOFF_FAIL 回包让客户端恢复穿戴状态。
- InventoryFullUnequip.csv 52 步通过：满包卸装失败回包、外观/装备不变、消费一瓶后卸装成功、重新穿戴后拾取、保存重登数量正确。EIServer Release 构建通过，日志 out/csv-validation/inventory-full-unequip.log。

### 怪物按存活数量补充刷新
- ProcessMonster 刷新名额改为仅统计未死亡、非 Ghost 对象；仍保留原尸体及延迟释放时序，避免这些对象占用刷新名额。
- MonstersRespawn.csv 使用真实 FLD_GENTIME=1 分钟配置，首杀后补充刷新、查询新实体名称、再击杀、第二轮补充刷新，30 步通过，耗时 118984ms。EIServer Release 构建通过，日志 out/csv-validation/monsters-respawn.log。本场景不覆盖尸体最终消失、Ghost 释放与长期稳定性。
