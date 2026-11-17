
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
