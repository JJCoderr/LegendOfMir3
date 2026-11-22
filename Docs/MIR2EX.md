# 传奇 3 服务端与客户端源码导读

本文依据本地 master 提交 `3184c28` 的源码与构建配置。下文的端口为该版本代码中的监听值或缺省值，流程说明来自静态分析。末尾索引覆盖 Server 和 Client 两个目录中 Git 跟踪的全部文件。

## 一名玩家从登录到进入地图

| 阶段 | 客户端连接 | 网关连接 | 业务归属 |
| --- | --- | --- | --- |
| 登录、选区服 | LoginGate（默认 7000） | LoginSvr（5500） | LoginSvr 查询账号、校验密码，读取区服列表并返回选角入口 |
| 查角、建角、删角、选角 | SelGate（默认 7100） | DBSvr（5100） | DBSvr 操作角色资料，选角时预载角色并返回游戏网关地址 |
| 进入地图、游戏交互 | GameGate（默认 7200） | GameSvr（默认 5000） | GameSvr 建立玩家对象并运行世界逻辑，通过 DBSvr（6000）读写角色记录 |

三个阶段使用各自的客户端连接。网关维护客户端会话与后端连接的对应关系。选服和选角回包告诉客户端下一次连接的地址，因此后台服务的监听端口、网关的转发目标、下发给客户端的入口地址是三项独立配置。

1. **登录与选服。** 客户端登录场景经 LoginGate 发出 `CM_IDPASSWORD`。LoginSvr 从账号表取记录，成功时返回 `SM_PASSOK_SELECTSERVER`。收到 `CM_SELECTSERVER` 后，它按区服编号从启动时加载的 `TBL_SERVERINFO` 列表取地址，编码进 `SM_SELECTSERVER_OK`。当前客户端按 `IP/端口/标识` 拆包，保存地址并断开登录连接。这里有一处字段差异值得联调时核实：服务端将认证值放在消息头的 `wParam`，客户端读取 `nRecog`。[服务端组包](Server/LoginSvr/GateInfo.cpp) · [客户端解析](Client/LoginProcess/LoginProcess.cpp)
2. **角色管理。** 客户端通过 SelGate 发送 `CM_QUERYCHR`、`CM_NEWCHR`、`CM_DELCHR` 或 `CM_SELCHR`。DBSvr 的网关线程从队列中解码消息并调用对应处理函数。选角处理从 `TBL_GAMEGATEINFO` 载入的列表中选取连接计数最小的网关，并取已连接游戏服列表的首项发送角色预载数据。返回的 `SM_STARTPLAY` 携带游戏入口，客户端按 `IP/端口` 解析。[请求分派](Server/DBSvr/ProcessGateMsg.cpp) · [选角与预载](Server/DBSvr/GateInfo.cpp) · [客户端解析](Client/CharSelectProcess/CharacterProcess.cpp)
3. **角色进入世界。** GameGate 用 `GM_OPEN`、`GM_DATA`、`GM_CLOSE` 向 GameSvr 表示会话建立、数据到达和断开。GameSvr 从客户端入场报文取账号和角色名，登录线程据此匹配待加载记录，将角色、技能和物品挂到玩家对象上，转入 `USERMODE_PLAYGAME`。此后玩家线程驱动动作，怪物和 NPC 线程分别推进世界状态。角色存档经 `DB_SAVEHUMANRCD` 交给 DBSvr 写库。[网关协议入口](Server/GameSvr/SockMsg_GateComm.cpp) · [加载角色](Server/GameSvr/ProcessLogin.cpp) · [会话与存档](Server/GameSvr/UserInfo.cpp)
4. **客户端显示。** Mir2Ex 将网络收发集中在 ClientSocket。登录场景处理区服回包，角色场景处理角色列表与入场回包，游戏场景管理地图、角色及操作界面。地图缓冲、角色绘制和背包交互分别由 MapHandler、Actor 和 InventoryWnd 承担，底层图像和绘图表面来自 WindHorn。[收发入口](Client/Common/ClientSocket.cpp) · [游戏场景](Client/GameProcess/GameProc.cpp) · [图形库](Client/WindHorn/CMakeLists.txt)

## 子工程职责与工作原理

| 子工程或目录 | 产物与职责 | 工作方式 |
| --- | --- | --- |
| LoginGate | 登录网关 EXE | 接受客户端 TCP 连接，维护会话，将账号阶段的消息与回复在客户端和 LoginSvr 间转发。 |
| LoginSvr | 账号服务 EXE | 直接经 ODBC 访问账号和服务器资料，按消息号执行注册、登录、选服处理。状态窗口读取内存中的连接与区服列表。 |
| SelGate | 角色网关 EXE | 接受选角连接并连接 DBSvr。前端会话经同一网关后端连接传送，回包按会话送回。 |
| DBSvr | 角色数据库服务 EXE | 两条入口分别接收 SelGate 的角色操作和 GameSvr 的角色读写。网关消息与数据库任务由不同线程消费。 |
| GameGate | 游戏网关 EXE | 维护游戏会话，拆解客户端游戏包，附加会话信息发给 GameSvr，再按会话转发下行数据。 |
| GameSvr | 世界逻辑服务 EXE | 启动时读取地图和游戏基础表，连接 DBSvr。数据库服务连接成功后启动登录、玩家、怪物、NPC 处理线程并监听 GameGate。 |
| Def | 公共静态库 | 向服务端目标提供消息结构、6 位编解码、网络、注册表、ODBC 和 Win32 界面公共实现。 |
| AdminCmd | GM 命令资源 DLL | GameSvr 动态加载 DLL，按资源 ID 读取命令名称组成查找表。聊天命令解析和实际执行逻辑都在 GameSvr。 |
| ManConsole | 管理控制台 EXE | 建立日志列表和命令输入框，目录内提供套接字封装。当前对话框只处理初始化、关于和关闭，命令提交尚未接入。 |
| VirtualClient | 手工协议调试 EXE | 启动时连接登录网关并显示收到的原始数据。源码保留发包函数，当前登录与注册对话框的确认按钮只关闭窗口。 |
| Tools | EncodeSize、test_encode | 前者计算角色记录的编码长度，后者将样例消息头编码再解码，并打印结果。 |
| SQL_Scripts | SQL Server 建库与建表脚本 | 包含账号、角色、地图、怪物和物品等表的历史定义。`create_db.sql` 会删除同名旧库再创建数据库。 |
| Client | Mir2Ex.exe | Win32 主程序管理登录、选角和游戏场景，调用网络、声音、视频和界面模块。 |
| Client/WindHorn | WindHorn 静态库 | 封装窗口消息、DirectX 绘图设备、表面与 WIL 图像资源，链接到 Mir2Ex。 |

构建目标和实际编译单元以 [服务端 CMake](Server/CMakeLists.txt)、[客户端 CMake](Client/CMakeLists.txt) 和 [WindHorn CMake](Client/WindHorn/CMakeLists.txt) 为准。

## 通信协议

### 四条链路的报文格式

| 链路 | 线上格式与处理方式 |
| --- | --- |
| 客户端 → 网关 | 常规帧为 `#同步号＋编码头＋可选编码正文!`。SendPacket 的同步号在 1 至 9 间循环，消息头和正文分别编码。进入游戏的首包单独编码 `**账号/角色名/认证值/版本/1`。 |
| LoginGate ↔ LoginSvr、SelGate ↔ DBSvr | 网关用 `%O套接字号/IP$` 打开会话，用 `%X套接字号$` 通知关闭，用 `%A套接字号/客户端帧$` 转发业务。普通下行格式为 `%套接字号/#编码消息!$`，网关按套接字号转发内层帧。接收线程留存未完整到达的尾段，等下一次读取后拼接。 |
| GameGate ↔ GameSvr | 使用二进制 `_TMSGHEADER`，按顺序存放标记、套接字号、网关用户索引、GM 消息号、服务端用户索引、临时字段和正文长度。常规上行标记为 `0xAA55AA55`，`GM_DATA` 携带已解码的业务头及可选正文。建连、关闭和接收确认各有独立消息。 |
| GameSvr ↔ DBSvr | 双向采用 `#认证值/编码头＋编码正文＋6字符尾部!`。尾部编码自 32 位值，低字为认证值异或 `0xAA`，高字为编码头、正文和尾部的总长度。DBSvr 将请求入队处理，角色读取和造物等操作通过 DBR 消息返回结果。 |

6 位编码对每组 6 位加 `0x3C` 后输出。默认消息头在内存中依次为 `nRecog`（32 位）、`wIdent`、`wParam`、`wTag`、`wSeries`（各 16 位），共 12 字节，编码后为 16 个字符。字段含义随消息号变化。相同编号也可用于不同链路，例如角色查询和内部角色读取均使用 100。[头结构](Server/Def/EnDecode.h) · [编码算法](Server/Def/EnDecode.cpp) · [客户端收发](Client/Common/ClientSocket.cpp) · [选角网关转发](Server/SelGate/ServerSockMsg.cpp) · [游戏网关封包](Server/GameGate/ThreadFuncForMsg.cpp) · [游戏服发包](Server/GameSvr/ProcessLogin.cpp) · [数据库服回包](Server/DBSvr/ProcessDBMsg.cpp)

### 业务消息与两端差异

| 操作 | 发送内容 | 接收端行为 |
| --- | --- | --- |
| 登录 | CM_IDPASSWORD 发送账号/密码 | LoginSvr 区分账号不存在、密码不符和认证失败，成功后返回区服列表。 |
| 选服 | CM_SELECTSERVER 用 wParam 传区服编号 | SM_SELECTSERVER_OK 携带地址串。客户端期待其中有两个斜杠。服务端把认证值放入 wParam，客户端从 nRecog 读取。 |
| 查角 | CM_QUERYCHR 发送账号/认证值 | DBSvr 取斜杠**之后**的部分作为登录账号查询。成功回包以 wParam 表示角色数，附角色记录数组。成功分支只接受 1 或 2 条记录。 |
| 建角、删角 | CM_NEWCHR 发送名称/发型/职业/性别文本，CM_DELCHR 只发角色名 | DBSvr 对两条消息均按固定布局的 `_TCREATECHR` 解码，两端布局存在差异。 |
| 选角 | CM_SELCHR 发送账号/角色名 | DBSvr 按斜杠拆分，预载角色并以 SM_STARTPLAY 下发游戏入口。 |
| 场景移动 | 玩家发送 CM_WALK | GameSvr 向观察者发送 SM_WALK。头中携带对象标识、横纵坐标、方向与光照，正文含外观和状态，人物另附扩展外观。 |
| 聊天 | CM_SAY 的正文为编码文本 | GameGate 解码文本并调用过滤函数。GameSvr 处理聊天或命令，客户端按 SM_HEAR 等消息更新显示。 |

这些差异来自当前两端调用点的静态对照。与原版 V1.45 的互通性尚待运行验证。[客户端发包](Client/Common/ClientSocket.cpp) · [角色消息分派](Server/DBSvr/ProcessGateMsg.cpp) · [查询与选角](Server/DBSvr/GateInfo.cpp) · [世界消息生成](Server/GameSvr/ObjectPlayer.cpp)

### 消息编号对照

下表汇总 [客户端登录与选角协议](Client/Common/Protocol.h)、[客户端游戏消息](Client/Define.h) 和 [服务端协议](Server/Def/Protocol.h) 中直接定义为数字的消息宏。横线表示该侧所查头文件未声明该名称。表中编号说明源码声明，业务处理情况见上文。

**客户端上行**

| 消息 | 客户端 | 服务端 |
| --- | ---: | ---: |
| `CM_QUERYUSERNAME` | 80 | 80 |
| `CM_QUERYBAGITEMS` | 81 | 81 |
| `CM_QUERYCHR` | 100 | 100 |
| `CM_NEWCHR` | 101 | 101 |
| `CM_DELCHR` | 102 | 102 |
| `CM_SELCHR` | 103 | 103 |
| `CM_SELECTSERVER` | 104 | 104 |
| `CM_DROPITEM` | 1000 | 1000 |
| `CM_PICKUP` | 1001 | 1001 |
| `CM_OPENDOOR` | 1002 | — |
| `CM_TAKEONITEM` | 1003 | 1003 |
| `CM_TAKEOFFITEM` | 1004 | 1004 |
| `CM_EAT` | 1006 | 1006 |
| `CM_BUTCH` | 1007 | 1007 |
| `CM_MAGICKEYCHANGE` | 1008 | 1008 |
| `CM_CLICKNPC` | 1010 | — |
| `CM_MERCHANTDLGSELECT` | 1011 | — |
| `CM_MERCHANTQUERYSELLPRICE` | 1012 | — |
| `CM_USERSELLITEM` | 1013 | — |
| `CM_USERBUYITEM` | 1014 | — |
| `CM_USERGETDETAILITEM` | 1015 | — |
| `CM_LOGINNOTICEOK` | 1018 | — |
| `CM_GROUPMODE` | 1019 | — |
| `CM_CREATEGROUP` | 1020 | — |
| `CM_ADDGROUPMEMBER` | 1021 | — |
| `CM_DELGROUPMEMBER` | 1022 | — |
| `CM_USERREPAIRITEM` | 1023 | — |
| `CM_MERCHANTQUERYREPAIRCOST` | 1024 | — |
| `CM_USERSTORAGEITEM` | 1031 | — |
| `CM_USERTAKEBACKSTORAGEITEM` | 1032 | — |
| `CM_USERMAKEDRUGITEM` | 1034 | — |
| `CM_OPENGUILDDLG` | 1035 | — |
| `CM_GUILDHOME` | 1036 | — |
| `CM_GUILDMEMBERLIST` | 1037 | — |
| `CM_GUILDADDMEMBER` | 1038 | — |
| `CM_GUILDDELMEMBER` | 1039 | — |
| `CM_GUILDUPDATENOTICE` | 1040 | — |
| `CM_GUILDUPDATERANKINFO` | 1041 | — |
| `CM_GUILDMAKEALLY` | 1044 | — |
| `CM_GUILDBREAKALLY` | 1045 | — |
| `CM_PROTOCOL` | 2000 | 2000 |
| `CM_IDPASSWORD` | 2001 | 2001 |
| `CM_ADDNEWUSER` | 2002 | 2002 |
| `CM_CHANGEPASSWORD` | 2003 | 2003 |
| `CM_UPDATEUSER` | 2004 | 2004 |
| `CM_TRUN` | 3010 | — |
| `CM_TURN` | — | 3010 |
| `CM_WALK` | 3011 | 3011 |
| `CM_SITDOWN` | 3012 | 3012 |
| `CM_RUN` | 3013 | 3013 |
| `CM_HIT` | 3014 | 3014 |
| `CM_HEAVYHIT` | — | 3015 |
| `CM_BIGHIT` | — | 3016 |
| `CM_SPELL` | 3017 | 3017 |
| `CM_POWERHIT` | 3018 | 3018 |
| `CM_LONGHIT` | 3019 | 3019 |
| `CM_WIDEHIT` | 3024 | 3024 |
| `CM_FIREHIT` | 3025 | 3025 |
| `CM_SAY` | 3030 | 3030 |
| `CM_RIDE` | 3031 | 3031 |

**客户端下行**

| 消息 | 客户端 | 服务端 |
| --- | ---: | ---: |
| `SM_RUSH` | 6 | 6 |
| `SM_FIREHIT` | 8 | 8 |
| `SM_BACKSTEP` | 9 | 9 |
| `SM_TURN` | 10 | 10 |
| `SM_WALK` | 11 | 11 |
| `SM_SITDOWN` | 12 | 12 |
| `SM_RUN` | 13 | 13 |
| `SM_HIT` | 14 | 14 |
| `SM_SPELL` | 17 | 17 |
| `SM_POWERHIT` | 18 | 18 |
| `SM_LONGHIT` | 19 | 19 |
| `SM_DIGUP` | 20 | 20 |
| `SM_DIGDOWN` | 21 | 21 |
| `SM_FLYAXE` | 22 | 22 |
| `SM_LIGHTING` | 23 | 23 |
| `SM_WIDEHIT` | 24 | 24 |
| `SM_DISAPPEAR` | 30 | 30 |
| `SM_STRUCK` | 31 | 31 |
| `SM_DEATH` | 32 | 32 |
| `SM_SKELETON` | 33 | 33 |
| `SM_NOWDEATH` | 34 | 34 |
| `SM_HEAR` | 40 | 40 |
| `SM_FEATURECHANGED` | 41 | 41 |
| `SM_USERNAME` | 42 | 42 |
| `SM_WINEXP` | 44 | 44 |
| `SM_LEVELUP` | 45 | 45 |
| `SM_DAYCHANGING` | 46 | 46 |
| `SM_LOGON` | 50 | 50 |
| `SM_NEWMAP` | 51 | 51 |
| `SM_ABILITY` | 52 | 52 |
| `SM_HEALTHSPELLCHANGED` | 53 | 53 |
| `SM_MAPDESCRIPTION` | 54 | 54 |
| `SM_SYSMESSAGE` | 100 | 100 |
| `SM_GROUPMESSAGE` | 101 | 101 |
| `SM_CRY` | 102 | 102 |
| `SM_WHISPER` | 103 | 103 |
| `SM_GUILDMESSAGE` | 104 | 104 |
| `SM_SPELL2` | — | 117 |
| `SM_ADDITEM` | 200 | 200 |
| `SM_BAGITEMS` | 201 | 201 |
| `SM_DELITEM` | 202 | — |
| `SM_DELITEMS` | 203 | — |
| `SM_ADDMAGIC` | 210 | 210 |
| `SM_SENDMYMAGIC` | 211 | 211 |
| `SM_VERSION_AVAILABLE` | 500 | — |
| `SM_CERTIFICATION_FAIL` | 501 | 501 |
| `SM_ID_NOTFOUND` | 502 | 502 |
| `SM_PASSWD_FAIL` | 503 | 503 |
| `SM_NEWID_SUCCESS` | 504 | 504 |
| `SM_NEWID_FAIL` | 505 | 505 |
| `SM_CHGPASSWD_SUCCESS` | 506 | — |
| `SM_CHGPASSWD_FAIL` | 507 | — |
| `SM_QUERYCHR` | 520 | 520 |
| `SM_NEWCHR_SUCCESS` | 521 | 521 |
| `SM_NEWCHR_FAIL` | 522 | 522 |
| `SM_DELCHR_SUCCESS` | 523 | 523 |
| `SM_DELCHR_FAIL` | 524 | 524 |
| `SM_STARTPLAY` | 525 | 525 |
| `SM_STARTFAIL` | 526 | 526 |
| `SM_QUERYCHR_FAIL` | 527 | 527 |
| `SM_OUTOFCONNECTION` | 528 | — |
| `SM_PASSOK_SELECTSERVER` | 529 | 529 |
| `SM_SELECTSERVER_OK` | 530 | 530 |
| `SM_NEEDUPDATE_ACCOUNT` | 531 | — |
| `SM_UPDATEID_SUCCESS` | 532 | — |
| `SM_UPDATEID_FAIL` | 533 | — |
| `SM_DROPITEM_SCCESS` | 600 | — |
| `SM_DROPITEM_SUCCESS` | — | 600 |
| `SM_DROPITEM_FAIL` | 601 | 601 |
| `SM_ITEMSHOW` | 610 | 610 |
| `SM_ITEMHIDE` | 611 | 611 |
| `SM_DOOROPEN` | 612 | 612 |
| `SM_OPENDOOR_OK` | 612 | — |
| `SM_OPENDOOR_LOCK` | 613 | — |
| `SM_CLOSEDOOR` | 614 | — |
| `SM_TAKEON_OK` | 615 | 615 |
| `SM_TAKEON_FAIL` | 616 | 616 |
| `SM_TAKEOFF_OK` | 619 | 619 |
| `SM_TAKEOFF_FAIL` | 620 | 620 |
| `SM_SENDUSEITEMS` | 621 | 621 |
| `SM_WEIGHTCHANGED` | 622 | 622 |
| `SM_CLEAROBJECT` | 633 | — |
| `SM_CLEAROBJECTS` | — | 633 |
| `SM_CHANGEMAP` | 634 | 634 |
| `SM_EAT_OK` | 635 | 635 |
| `SM_EAT_FAIL` | 636 | 636 |
| `SM_BUTCH` | 637 | 637 |
| `SM_MAGICFIRE` | 638 | 638 |
| `SM_MAGIC_LVEXP` | 640 | 640 |
| `SM_DURACHANGE` | 642 | 642 |
| `SM_MERCHANTSAY` | 643 | 643 |
| `SM_MERCHANTDLGCLOSE` | 644 | — |
| `SM_SENDGOODSLIST` | 645 | — |
| `SM_SENDUSERSELL` | 646 | — |
| `SM_SENDBUYPRICE` | 647 | — |
| `SM_USERSELLITEM_OK` | 648 | — |
| `SM_USERSELLITEM_FAIL` | 649 | — |
| `SM_BUYITEM_SUCCESS` | 650 | — |
| `SM_BUYITEM_FAIL` | 651 | — |
| `SM_SENDDETAILGOODSLIST` | 652 | — |
| `SM_GOLDCHANGE` | 653 | — |
| `SM_GOLDCHANGED` | 653 | 653 |
| `SM_CHANGELIGHT` | 654 | 654 |
| `SM_CHANGENAMECOLOR` | 656 | 656 |
| `SM_CHARSTATUSCHANGE` | 657 | — |
| `SM_CHARSTATUSCHANGED` | — | 657 |
| `SM_SENDNOTICE` | 658 | — |
| `SM_CREATEGROUP_OK` | 660 | — |
| `SM_CREATEGROUP_FAIL` | 661 | — |
| `SM_GROUPCANCEL` | 666 | — |
| `SM_GROUPMEMBERS` | 667 | — |
| `SM_AREASTATE` | 708 | — |
| `SM_CHANGEGUILDNAME` | 750 | — |
| `SM_SUBABILITY` | 752 | 752 |
| `SM_OPENGUILDDLG` | 753 | — |
| `SM_OPENGUILDDLG_FAIL` | 754 | — |
| `SM_SENDGUILDHOME` | 755 | — |
| `SM_SENDGUILDMEMBERLIST` | 756 | — |
| `SM_GUILDADDMEMBER_OK` | 757 | — |
| `SM_GUILDADDMEMBER_FAIL` | 758 | — |
| `SM_GUILDDELMEMBER_OK` | 759 | — |
| `SM_GUILDDELMEMBER_FAIL` | 760 | — |
| `SM_GUILDRANKUPDATE_FAIL` | 761 | — |
| `SM_BUILDGUILD_OK` | 762 | — |
| `SM_BUILDGUILD_FAIL` | 763 | — |
| `SM_GUILDMAKEALLY_OK` | 768 | — |
| `SM_GUILDMAKEALLY_FAIL` | 769 | — |
| `SM_GUILDBREAKALLY_OK` | 770 | — |
| `SM_GUILDBREAKALLY_FAIL` | 771 | — |
| `SM_DLGMSG` | 772 | — |
| `SM_SHOWEVENT` | 804 | 804 |
| `SM_HIDEEVENT` | 805 | 805 |
| `SM_OPENHEALTH` | 1100 | 1100 |
| `SM_CLOSEHEALTH` | 1101 | 1101 |
| `SM_CHANGEFACE` | 1104 | 1104 |
| `SM_RIDEHORSE` | 1300 | — |
| `SM_ITEMUPDATE` | — | 1500 |
| `SM_MONSTERSAY` | 1501 | 1501 |

**游戏网关内部消息**

| 消息 | 客户端 | 服务端 |
| --- | ---: | ---: |
| `GM_OPEN` | 1 | 1 |
| `GM_CLOSE` | 2 | 2 |
| `GM_CHECKSERVER` | 3 | 3 |
| `GM_CHECKCLIENT` | 4 | 4 |
| `GM_DATA` | 5 | 5 |
| `GM_SERVERUSERINDEX` | 6 | 6 |
| `GM_RECEIVE_OK` | 7 | 7 |
| `GM_TEST` | 20 | 20 |

**数据库请求**

| 消息 | 客户端 | 服务端 |
| --- | ---: | ---: |
| `DB_LOADHUMANRCD` | — | 100 |
| `DB_SAVEHUMANRCD` | — | 101 |
| `DB_MAKEITEMRCD` | — | 150 |
| `DB_ITEMTHROW` | — | 151 |
| `DB_MAKEITEMRCD2` | — | 152 |

**数据库回复**

| 消息 | 客户端 | 服务端 |
| --- | ---: | ---: |
| `DBR_LOADHUMANRCD` | — | 1100 |
| `DBR_LOADHUMANRCD2` | — | 1101 |
| `DBR_MAKEITEMRCD` | — | 1500 |
| `DBR_MAKEITEMRCD2` | — | 1501 |
| `DBR_FAIL` | — | 2000 |

## 文件职责索引

每行链接到一个仓库文件。源码行附有代表性符号，便于在编辑器中定位。标题为文件所在目录。

### Client/AVI

| 文件 | 职责与定位 |
| --- | --- |
| [Avi.cpp](Client/AVI/Avi.cpp) | 打开、播放及释放 AVI 视频 · `CAvi`、`InitAvi`、`ReleaseAvi` |
| [Avi.h](Client/AVI/Avi.h) | 定义或声明相关类型与接口：打开、播放及释放 AVI 视频 · `CAvi` |

### Client

| 文件 | 职责与定位 |
| --- | --- |
| [CMakeLists.txt](Client/CMakeLists.txt) | 声明 Client 的构建目标、源文件和编译设置 |

### Client/CharSelectProcess/BackGround

| 文件 | 职责与定位 |
| --- | --- |
| [BackGround.cpp](Client/CharSelectProcess/BackGround/BackGround.cpp) | 绘制选角背景并处理该层的输入 · `CBackGround`、`Create`、`OnKeyDown` |
| [BackGround.h](Client/CharSelectProcess/BackGround/BackGround.h) | 定义或声明相关类型与接口：绘制选角背景并处理该层的输入 · `CBackGround` |

### Client/CharSelectProcess

| 文件 | 职责与定位 |
| --- | --- |
| [CharacterProcess.cpp](Client/CharSelectProcess/CharacterProcess.cpp) | 调度角色选择与创建界面并处理角色服务回包 · `CCharacterProcess`、`Init`、`Load` |
| [CharacterProcess.h](Client/CharSelectProcess/CharacterProcess.h) | 定义或声明相关类型与接口：调度角色选择与创建界面并处理角色服务回包 · `CCharacterProcess` |

### Client/CharSelectProcess/Common

| 文件 | 职责与定位 |
| --- | --- |
| [define.h](Client/CharSelectProcess/Common/define.h) | 定义当前模块使用的常量和资源标识 |

### Client/CharSelectProcess/CreateChr

| 文件 | 职责与定位 |
| --- | --- |
| [CreateChr.cpp](Client/CharSelectProcess/CreateChr/CreateChr.cpp) | 处理创建角色界面的输入与选择 · `CCreateChr`、`Create`、`OnKeyDown` |
| [CreateChr.h](Client/CharSelectProcess/CreateChr/CreateChr.h) | 定义或声明相关类型与接口：处理创建角色界面的输入与选择 · `CCreateChr` |

### Client/CharSelectProcess/SelectChr

| 文件 | 职责与定位 |
| --- | --- |
| [SelectChr.cpp](Client/CharSelectProcess/SelectChr/SelectChr.cpp) | 显示可选角色并处理角色选择和交互 · `CSelectChr`、`Create`、`OpenWnd` |
| [SelectChr.h](Client/CharSelectProcess/SelectChr/SelectChr.h) | 定义或声明相关类型与接口：显示可选角色并处理角色选择和交互 · `CSelectChr` |

### Client/Common

| 文件 | 职责与定位 |
| --- | --- |
| [Button.H](Client/Common/Button.H) | 定义或声明相关类型与接口：管理文本按钮集合与点击命中测试 · `CTextButton` |
| [Button.cpp](Client/Common/Button.cpp) | 管理文本按钮集合与点击命中测试 · `CTextButton`、`AddButton`、`ClickCheck` |
| [ChatEditBox.cpp](Client/Common/ChatEditBox.cpp) | 创建和控制聊天输入编辑框 · `CChatEdit`、`DestroyDialog`、`ChatEditDlgProc` |
| [ChatEditBox.h](Client/Common/ChatEditBox.h) | 定义或声明相关类型与接口：创建和控制聊天输入编辑框 · `CChatEdit` |
| [ClientSocket.cpp](Client/Common/ClientSocket.cpp) | 建立客户端连接，编码上行请求并收发协议数据 · `CClientSocket`、`GetINetAddr`、`ConnectToServer` |
| [ClientSocket.h](Client/Common/ClientSocket.h) | 定义或声明相关类型与接口：建立客户端连接，编码上行请求并收发协议数据 · `CClientSocket` |
| [DLinkedList.cpp](Client/Common/DLinkedList.cpp) | 引入链表模板头，模板主体位于同名头文件 · `CDLList` |
| [DLinkedList.h](Client/Common/DLinkedList.h) | 定义或声明相关类型与接口：提供 CDLList 双向链表模板 · `CDLList` |
| [DblList.cpp](Client/Common/DblList.cpp) | 引入链表模板头，模板主体位于同名头文件 · `CPDLList` |
| [DblList.h](Client/Common/DblList.h) | 定义或声明相关类型与接口：提供 CPDLList 双向链表模板 · `CPDLList` |
| [EnDecode.cpp](Client/Common/EnDecode.cpp) | 在协议消息结构和 6 位文本编码之间转换 · `fnEncode6BitBuf`、`fnDecode6BitBuf`、`fnEncodeMessage` |
| [EnDecode.h](Client/Common/EnDecode.h) | 定义或声明相关类型与接口：在协议消息结构和 6 位文本编码之间转换 |
| [ImageHandler.cpp](Client/Common/ImageHandler.cpp) | 统一装载图像档案并管理纹理 · `CImageHandler`、`LoadAllImage`、`InitAllImage` |
| [ImageHandler.h](Client/Common/ImageHandler.h) | 定义或声明相关类型与接口：统一装载图像档案并管理纹理 · `CImageHandler` |
| [MirButton.cpp](Client/Common/MirButton.cpp) | 管理图像按钮状态、悬停及点击判断 · `CMirButton`、`CheckMouseOn`、`CheckClick` |
| [MirButton.h](Client/Common/MirButton.h) | 定义或声明相关类型与接口：管理图像按钮状态、悬停及点击判断 · `CMirButton` |
| [MirMsgBox.cpp](Client/Common/MirMsgBox.cpp) | 管理游戏图像消息框的装载和显示 · `CMirMsgBox`、`Load`、`ShowMessageBox` |
| [MirMsgBox.h](Client/Common/MirMsgBox.h) | 定义或声明相关类型与接口：管理游戏图像消息框的装载和显示 · `CMirMsgBox` |
| [MirMsgBox3D.cpp](Client/Common/MirMsgBox3D.cpp) | 创建和显示使用 3D 界面的游戏消息框 · `CMirMsgBox3D`、`Load`、`CreateWnd` |
| [MirMsgBox3D.h](Client/Common/MirMsgBox3D.h) | 定义或声明相关类型与接口：创建和显示使用 3D 界面的游戏消息框 · `CMirMsgBox3D` |
| [MsgBox.cpp](Client/Common/MsgBox.cpp) | 管理通用提示框的内容、显示与隐藏 · `CMsgBox`、`SetMsgBox`、`ShowMessageBox` |
| [MsgBox.h](Client/Common/MsgBox.h) | 定义或声明相关类型与接口：管理通用提示框的内容、显示与隐藏 · `CMsgBox` |
| [NPCTxtAnalysis.cpp](Client/Common/NPCTxtAnalysis.cpp) | 解析 NPC 对话文本及可交互片段 · `CNPCTxtAnalysis`、`InitAnalysis`、`FreeAllMsg` |
| [NPCTxtAnalysis.h](Client/Common/NPCTxtAnalysis.h) | 定义或声明相关类型与接口：解析 NPC 对话文本及可交互片段 · `CNPCTxtAnalysis` |
| [NoticeBox.cpp](Client/Common/NoticeBox.cpp) | 装载和显示公告框 · `CNoticeBox`、`Load`、`DestoryMessageBox` |
| [NoticeBox.h](Client/Common/NoticeBox.h) | 定义或声明相关类型与接口：装载和显示公告框 · `CNoticeBox` |
| [Protocol.h](Client/Common/Protocol.h) | 定义或声明相关类型与接口：定义协议消息编号和网络传输记录结构 |
| [Queue.cpp](Client/Common/Queue.cpp) | 提供消息入队和出队操作 · `CWHQueue`、`PushQ`、`PopQ` |
| [Queue.h](Client/Common/Queue.h) | 定义或声明相关类型与接口：提供消息入队和出队操作 · `CWHQueue` |
| [Typedeftxt.h](Client/Common/Typedeftxt.h) | 定义或声明相关类型与接口：定义文字按钮的颜色、文本和布局记录 |

### Client

| 文件 | 职责与定位 |
| --- | --- |
| [Define.h](Client/Define.h) | 定义当前模块使用的常量和资源标识 |
| [Extern.h](Client/Extern.h) | 声明客户端跨模块共享的全局对象和状态 |
| [GFun.cpp](Client/GFun.cpp) | 提供跨场景使用的结构和辅助函数 · `GetChatColor`、`GetUserNameColor` |
| [GFun.h](Client/GFun.h) | 定义或声明相关类型与接口：提供跨场景使用的结构和辅助函数 |

### Client/GameProcess

| 文件 | 职责与定位 |
| --- | --- |
| [Actor.cpp](Client/GameProcess/Actor.cpp) | 管理场景角色的动作、状态、外观与绘制 · `CActor`、`InitActor`、`DestroyActor` |
| [Actor.h](Client/GameProcess/Actor.h) | 定义或声明相关类型与接口：管理场景角色的动作、状态、外观与绘制 · `CActor`、`CNPC`、`CHero` |
| [ChatPopWnd.cpp](Client/GameProcess/ChatPopWnd.cpp) | 管理弹出聊天窗口与消息列表 · `CChatPopWnd`、`Init`、`Destroy` |
| [ChatPopWnd.h](Client/GameProcess/ChatPopWnd.h) | 定义或声明相关类型与接口：管理弹出聊天窗口与消息列表 · `CChatPopWnd` |
| [ClientSysMsg.cpp](Client/GameProcess/ClientSysMsg.cpp) | 追加、缓存并绘制客户端系统提示 · `ClientSysMsg`、`Init`、`Destroy` |
| [ClientSysMsg.h](Client/GameProcess/ClientSysMsg.h) | 定义或声明相关类型与接口：追加、缓存并绘制客户端系统提示 · `ClientSysMsg` |
| [ExchangeWnd.cpp](Client/GameProcess/ExchangeWnd.cpp) | 创建、绘制交易窗口并处理交互 · `CExchangeWnd`、`Init`、`Destroy` |
| [ExchangeWnd.h](Client/GameProcess/ExchangeWnd.h) | 定义或声明相关类型与接口：创建、绘制交易窗口并处理交互 · `CExchangeWnd` |
| [GameBtn.cpp](Client/GameProcess/GameBtn.cpp) | 管理游戏界面按钮的创建与显示 · `CreateGameBtn`、`Init`、`Destroy` |
| [GameBtn.h](Client/GameProcess/GameBtn.h) | 定义或声明相关类型与接口：管理游戏界面按钮的创建与显示 · `CGameBtn`、`CMsgBoxBtn`、`CScrlBar` |
| [GameProc.cpp](Client/GameProcess/GameProc.cpp) | 调度游戏场景，处理游戏消息、地图切换和绘制 · `CGameProcess`、`Init`、`DeleteProc` |
| [GameProc.h](Client/GameProcess/GameProc.h) | 定义或声明相关类型与接口：调度游戏场景，处理游戏消息、地图切换和绘制 · `CGameProcess` |
| [GameWnd.cpp](Client/GameProcess/GameWnd.cpp) | 管理游戏主界面窗口及交互 · `CreateGameWnd`、`Init`、`Destroy` |
| [GameWnd.h](Client/GameProcess/GameWnd.h) | 定义或声明相关类型与接口：管理游戏主界面窗口及交互 · `CGameWnd` |
| [GroupPopWnd.cpp](Client/GameProcess/GroupPopWnd.cpp) | 管理组队弹出窗口 · `CGroupPopWnd`、`CreateGroupPopWnd`、`Init` |
| [GroupPopWnd.h](Client/GameProcess/GroupPopWnd.h) | 定义或声明相关类型与接口：管理组队弹出窗口 · `CGroupPopWnd` |
| [GroupWnd.cpp](Client/GameProcess/GroupWnd.cpp) | 管理队伍界面及成员显示 · `CGroupWnd`、`CreateGroupWnd`、`ShowGroupWnd` |
| [GroupWnd.h](Client/GameProcess/GroupWnd.h) | 定义或声明相关类型与接口：管理队伍界面及成员显示 · `CGroupMember`、`CGroupWnd` |
| [GuildMasterWnd.cpp](Client/GameProcess/GuildMasterWnd.cpp) | 管理行会会长操作界面 · `CGuildMasterWnd`、`Init`、`Destroy` |
| [GuildMasterWnd.h](Client/GameProcess/GuildMasterWnd.h) | 定义或声明相关类型与接口：管理行会会长操作界面 · `CGuildMasterWnd` |
| [GuildWnd.cpp](Client/GameProcess/GuildWnd.cpp) | 管理行会信息窗口 · `CGuildWnd`、`Init`、`Destroy` |
| [GuildWnd.h](Client/GameProcess/GuildWnd.h) | 定义或声明相关类型与接口：管理行会信息窗口 · `CGuildWnd` |
| [HorseWnd.cpp](Client/GameProcess/HorseWnd.cpp) | 管理坐骑信息窗口 · `CHorseWnd`、`Init`、`Destroy` |
| [HorseWnd.h](Client/GameProcess/HorseWnd.h) | 定义或声明相关类型与接口：管理坐骑信息窗口 · `CHorseWnd` |
| [Interface.cpp](Client/GameProcess/Interface.cpp) | 组织游戏界面的子窗口并分派交互 · `CInterface`、`Init`、`Destroy` |
| [Interface.h](Client/GameProcess/Interface.h) | 定义或声明相关类型与接口：组织游戏界面的子窗口并分派交互 · `CInterface` |
| [InventoryWnd.cpp](Client/GameProcess/InventoryWnd.cpp) | 绘制背包格及其物品并处理输入 · `CInventoryWnd`、`Init`、`Destroy` |
| [InventoryWnd.h](Client/GameProcess/InventoryWnd.h) | 定义或声明相关类型与接口：绘制背包格及其物品并处理输入 · `CInventoryWnd` |
| [Item.cpp](Client/GameProcess/Item.cpp) | 设置并绘制物品图像及状态说明 · `CItem`、`SetItem`、`DrawItem` |
| [Item.h](Client/GameProcess/Item.h) | 定义或声明相关类型与接口：设置并绘制物品图像及状态说明 · `CItem` |
| [LightFog.cpp](Client/GameProcess/LightFog.cpp) | 计算屏幕光照格和光色 · `CLightFog`、`Init`、`ChangeLightColor` |
| [LightFog.h](Client/GameProcess/LightFog.h) | 定义或声明相关类型与接口：计算屏幕光照格和光色 · `CLightFog` |
| [Magic.cpp](Client/GameProcess/Magic.cpp) | 创建和绘制法术动画及光效 · `CMagic`、`CreateMagic`、`DrawLight` |
| [Magic.h](Client/GameProcess/Magic.h) | 定义或声明相关类型与接口：创建和绘制法术动画及光效 · `CMagic`、`CMagicStream`、`CElecMagic` |
| [MapHandler.cpp](Client/GameProcess/MapHandler.cpp) | 装载地图文件并更新可见区域的地图缓冲 · `CMapHandler`、`InitMapHandler`、`LoadMapHandler` |
| [MapHandler.h](Client/GameProcess/MapHandler.h) | 定义或声明相关类型与接口：装载地图文件并更新可见区域的地图缓冲 · `CMapHandler` |
| [NPCWnd.cpp](Client/GameProcess/NPCWnd.cpp) | 绘制 NPC 对话窗口并处理鼠标选择 · `CheckText`、`AddScriptLine`、`DevideScript` |
| [NPCWnd.h](Client/GameProcess/NPCWnd.h) | 定义或声明相关类型与接口：绘制 NPC 对话窗口并处理鼠标选择 · `CNPCWnd` |
| [OptionWnd.cpp](Client/GameProcess/OptionWnd.cpp) | 创建并绘制游戏设置窗口 · `COptionWnd`、`Init`、`Destroy` |
| [OptionWnd.h](Client/GameProcess/OptionWnd.h) | 定义或声明相关类型与接口：创建并绘制游戏设置窗口 · `COptionWnd` |
| [Particle.cpp](Client/GameProcess/Particle.cpp) | 管理粒子系统的初始化和运动更新 · `CParticleSystem`、`InitSystem`、`SetupSystem` |
| [Particle.h](Client/GameProcess/Particle.h) | 定义或声明相关类型与接口：管理粒子系统的初始化和运动更新 · `CParticle`、`CParticleSystem`、`CSnow` |
| [PlayerChar.cpp](Client/GameProcess/PlayerChar.cpp) | 保留旧玩家角色实现文件 · `CPlayerChar`（当前构建未编入） |
| [PlayerChar.h](Client/GameProcess/PlayerChar.h) | 定义或声明相关类型与接口：保留旧玩家角色实现文件 · `CPlayerChar` |
| [QuestWnd.cpp](Client/GameProcess/QuestWnd.cpp) | 创建并绘制任务窗口 · `CQuestWnd`、`Init`、`Destroy` |
| [QuestWnd.h](Client/GameProcess/QuestWnd.h) | 定义或声明相关类型与接口：创建并绘制任务窗口 · `CQuestWnd` |
| [SprDfn.cpp](Client/GameProcess/SprDfn.cpp) | 管理角色精灵动作与特效定义 · `CSprite`、`InitSpriteInfo`、`DeleteSpriteInfo` |
| [SprDfn.h](Client/GameProcess/SprDfn.h) | 定义或声明相关类型与接口：管理角色精灵动作与特效定义 · `CSprite` |
| [SpriteInfo.cpp](Client/GameProcess/SpriteInfo.cpp) | 设置人物、怪物精灵的动作资料 · `CSpriteInfo`、`Init`、`SetSpriteInfo`（当前构建未编入） |
| [SpriteInfo.h](Client/GameProcess/SpriteInfo.h) | 定义或声明相关类型与接口：设置人物、怪物精灵的动作资料 · `CSpriteInfo` |
| [StatusWnd.cpp](Client/GameProcess/StatusWnd.cpp) | 绘制人物属性和状态面板 · `CStatusWnd`、`Init`、`Destory` |
| [StatusWnd.h](Client/GameProcess/StatusWnd.h) | 定义或声明相关类型与接口：绘制人物属性和状态面板 · `CStatusWnd` |
| [StoreWnd.cpp](Client/GameProcess/StoreWnd.cpp) | 绘制商店窗口并处理商品交互 · `CStoreWnd`、`CreateStoreWnd`、`ShowStoreWnd` |
| [StoreWnd.h](Client/GameProcess/StoreWnd.h) | 定义或声明相关类型与接口：绘制商店窗口并处理商品交互 · `CStoreWnd` |
| [Water.cpp](Client/GameProcess/Water.cpp) | 保留水面效果使用的网格与颜色计算 · `CWater`、`Init`、`ChangeLightColor`（当前构建未编入） |
| [Water.h](Client/GameProcess/Water.h) | 定义或声明相关类型与接口：保留水面效果使用的网格与颜色计算 · `CWater` |
| [WeatherEffect.cpp](Client/GameProcess/WeatherEffect.cpp) | 更新并绘制雾等天气效果 · `CMist`、`Create`、`Init` |
| [WeatherEffect.h](Client/GameProcess/WeatherEffect.h) | 定义或声明相关类型与接口：更新并绘制雾等天气效果 · `CMist`、`CAirWave` |

### Client/LoginProcess/Change Pwd

| 文件 | 职责与定位 |
| --- | --- |
| [ChgPwd.cpp](Client/LoginProcess/Change%20Pwd/ChgPwd.cpp) | 保留改密页面目录中的旧登录界面代码 · `CLogin`、`Create`、`OnKeyDown`（当前构建未编入） |
| [ChgPwd.h](Client/LoginProcess/Change%20Pwd/ChgPwd.h) | 定义或声明相关类型与接口：保留改密页面目录中的旧登录界面代码 · `CLogin` |

### Client/LoginProcess/Common

| 文件 | 职责与定位 |
| --- | --- |
| [BMWnd.cpp](Client/LoginProcess/Common/BMWnd.cpp) | 提供位图界面的打开、关闭与输入接口 · `CBMWnd`、`OpenWnd`、`CloseWnd` |
| [BMWnd.h](Client/LoginProcess/Common/BMWnd.h) | 定义或声明相关类型与接口：提供位图界面的打开、关闭与输入接口 · `CBMWnd` |
| [define.h](Client/LoginProcess/Common/define.h) | 定义当前模块使用的常量和资源标识 |
| [typedef.h](Client/LoginProcess/Common/typedef.h) | 定义或声明相关类型与接口：定义当前模块使用的数据结构和常量 |

### Client/LoginProcess/Intro

| 文件 | 职责与定位 |
| --- | --- |
| [LoginAvi.cpp](Client/LoginProcess/Intro/LoginAvi.cpp) | 控制登录片头视频及跳过操作 · `CLoginAvi`、`Create`、`SetRenderAviState` |
| [LoginAvi.h](Client/LoginProcess/Intro/LoginAvi.h) | 定义或声明相关类型与接口：控制登录片头视频及跳过操作 · `CLoginAvi` |

### Client/LoginProcess/Login

| 文件 | 职责与定位 |
| --- | --- |
| [Login.cpp](Client/LoginProcess/Login/Login.cpp) | 处理账号密码输入与登录按钮 · `CLogin`、`Create`、`OnKeyDown` |
| [Login.h](Client/LoginProcess/Login/Login.h) | 定义或声明相关类型与接口：处理账号密码输入与登录按钮 · `CLogin` |

### Client/LoginProcess

| 文件 | 职责与定位 |
| --- | --- |
| [LoginProcess.cpp](Client/LoginProcess/LoginProcess.cpp) | 读取连接配置，调度登录页面并处理登录、选服回包 · `CLoginProcess`、`Init`、`ReadIniFileData` |
| [LoginProcess.h](Client/LoginProcess/LoginProcess.h) | 定义或声明相关类型与接口：读取连接配置，调度登录页面并处理登录、选服回包 · `CLoginProcess` |

### Client/LoginProcess/New Account

| 文件 | 职责与定位 |
| --- | --- |
| [NewAccount.cpp](Client/LoginProcess/New%20Account/NewAccount.cpp) | 管理注册表单和输入操作 · `CNewAccount`、`Create`、`OnKeyDown` |
| [NewAccount.h](Client/LoginProcess/New%20Account/NewAccount.h) | 定义或声明相关类型与接口：管理注册表单和输入操作 · `CNewAccount` |

### Client/LoginProcess

| 文件 | 职责与定位 |
| --- | --- |
| [Patch.cpp](Client/LoginProcess/Patch.cpp) | 管理网络补丁检查、连接和下载流程 · `CPatch`、`Progress`、`OpenInet` |
| [Patch.h](Client/LoginProcess/Patch.h) | 定义或声明相关类型与接口：管理网络补丁检查、连接和下载流程 · `CPatch` |

### Client/LoginProcess/Select Srv

| 文件 | 职责与定位 |
| --- | --- |
| [SelectSrv.cpp](Client/LoginProcess/Select%20Srv/SelectSrv.cpp) | 显示区服列表并处理选服输入 · `CSelectSrv`、`Create`、`OnKeyDown` |
| [SelectSrv.h](Client/LoginProcess/Select%20Srv/SelectSrv.h) | 定义或声明相关类型与接口：显示区服列表并处理选服输入 · `CSelectSrv` |

### Client

| 文件 | 职责与定位 |
| --- | --- |
| [Mir2Ei.ini](Client/Mir2Ei.ini) | 客户端本地 INI 配置 |
| [Mir2Ex.ICO](Client/Mir2Ex.ICO) | 窗口图标资源 |
| [Mir2Ex.cpp](Client/Mir2Ex.cpp) | 客户端 WinMain、窗口和场景生命周期入口 · `WinMain` |
| [ReadMe.txt](Client/ReadMe.txt) | 保留原工程说明及文件布局说明 |
| [Resource.rc](Client/Resource.rc) | 声明程序的 Win32 界面、图标及字符串等资源 |

### Client/ServerSelectProcess

| 文件 | 职责与定位 |
| --- | --- |
| [SvrSelProc.cpp](Client/ServerSelectProcess/SvrSelProc.cpp) | 保留独立选服场景的加载、绘制和切换实现 · `CSvrSelProcess`、`Load`、`DeleteProc`（当前构建未编入） |
| [SvrSelProc.h](Client/ServerSelectProcess/SvrSelProc.h) | 定义或声明相关类型与接口：保留独立选服场景的加载、绘制和切换实现 · `CSvrSelProcess` |

### Client/Sound

| 文件 | 职责与定位 |
| --- | --- |
| [BMMP3.cpp](Client/Sound/BMMP3.cpp) | 管理 MP3 文件的装载与播放 · `CBMMp3`、`InitBMMp3`、`LoadMp3` |
| [BMMP3.h](Client/Sound/BMMP3.h) | 定义或声明相关类型与接口：管理 MP3 文件的装载与播放 · `CBMMp3` |
| [MirSound.cpp](Client/Sound/MirSound.cpp) | 根据声音来源位置计算距离、声像并调度音效 · `CMirSound`、`CalsDistanceX`、`CalsDistanceY` |
| [MirSound.h](Client/Sound/MirSound.h) | 定义或声明相关类型与接口：根据声音来源位置计算距离、声像并调度音效 · `CMirSound` |
| [SBuffer.cpp](Client/Sound/SBuffer.cpp) | 装载声音数据并管理播放缓冲区 · `CSBuffer`、`Release`、`Load` |
| [SBuffer.h](Client/Sound/SBuffer.h) | 定义或声明相关类型与接口：装载声音数据并管理播放缓冲区 · `CSBuffer` |
| [Sound.cpp](Client/Sound/Sound.cpp) | 管理声音设备、格式与播放操作 · `CSound`、`GetCaps`、`ContinuousPlay` |
| [Sound.h](Client/Sound/Sound.h) | 定义或声明相关类型与接口：管理声音设备、格式与播放操作 · `CSound` |
| [TypeDef.h](Client/Sound/TypeDef.h) | 定义或声明相关类型与接口：定义当前模块使用的数据结构和常量 |

### Client

| 文件 | 职责与定位 |
| --- | --- |
| [StdAfx.cpp](Client/StdAfx.cpp) | 引入工程公共头，保留预编译头翻译单元 |
| [StdAfx.h](Client/StdAfx.h) | 汇集本工程使用的系统头、公共类型和编译宏 |

### Client/WindHorn

| 文件 | 职责与定位 |
| --- | --- |
| [CMakeLists.txt](Client/WindHorn/CMakeLists.txt) | 声明 WindHorn 的构建目标、源文件和编译设置 |

### Client/WindHorn/D3DCommon

| 文件 | 职责与定位 |
| --- | --- |
| [d3dmath.cpp](Client/WindHorn/D3DCommon/d3dmath.cpp) | 提供 Direct3D 向量、矩阵等数学运算 · `D3DMath_MatrixMultiply`、`D3DMath_MatrixInvert`、`D3DMath_VectorMatrixMultiply` |
| [d3dmath.h](Client/WindHorn/D3DCommon/d3dmath.h) | 定义或声明相关类型与接口：提供 Direct3D 向量、矩阵等数学运算 |
| [d3dtextr.cpp](Client/WindHorn/D3DCommon/d3dtextr.cpp) | 管理贴图装载、创建和设备恢复 · `TextureSearchCallback`、`FindTexture`、`TextureContainer` |
| [d3dtextr.h](Client/WindHorn/D3DCommon/d3dtextr.h) | 定义或声明相关类型与接口：管理贴图装载、创建和设备恢复 |
| [d3dutil.cpp](Client/WindHorn/D3DCommon/d3dutil.cpp) | 提供 Direct3D 材质、矩阵和绘制状态辅助函数 · `D3DUtil_GetDXSDKMediaPath`、`D3DUtil_InitSurfaceDesc`、`D3DUtil_InitMaterial` |
| [d3dutil.h](Client/WindHorn/D3DCommon/d3dutil.h) | 定义或声明相关类型与接口：提供 Direct3D 材质、矩阵和绘制状态辅助函数 |
| [ddutil.cpp](Client/WindHorn/D3DCommon/ddutil.cpp) | 提供 DirectDraw 位图及表面操作辅助函数 · `SurfaceDataToFile`、`FileToSurfaceData`、`LoadBitmap8BitsInSurface` |
| [ddutil.h](Client/WindHorn/D3DCommon/ddutil.h) | 定义或声明相关类型与接口：提供 DirectDraw 位图及表面操作辅助函数 |

### Client/WindHorn

| 文件 | 职责与定位 |
| --- | --- |
| [Readme.txt](Client/WindHorn/Readme.txt) | 保留原工程说明及文件布局说明 |
| [RegHandler.cpp](Client/WindHorn/RegHandler.cpp) | 封装客户端注册表访问 · `jRegSetKey`、`jRegGetKey`、`jRegEnumKey` |
| [RegHandler.h](Client/WindHorn/RegHandler.h) | 定义或声明相关类型与接口：封装客户端注册表访问 |
| [StdAfx.cpp](Client/WindHorn/StdAfx.cpp) | 引入工程公共头，保留预编译头翻译单元 |
| [StdAfx.h](Client/WindHorn/StdAfx.h) | 汇集本工程使用的系统头、公共类型和编译宏 |
| [WH3DInit.cpp](Client/WindHorn/WH3DInit.cpp) | 保留 Direct3D 设备对象初始化实现 · `C3DInit`、`InitDeviceObjects`（当前构建未编入） |
| [WH3DInit.h](Client/WindHorn/WH3DInit.h) | 定义或声明相关类型与接口：保留 Direct3D 设备对象初始化实现 · `C3DInit` |
| [WHDXGraphic.cpp](Client/WindHorn/WHDXGraphic.cpp) | 管理 DirectX 图形窗口、显示设备和绘制环境 · `GetDXGWindowPtr`、`CWHDXGraphicWindow`、`UsedAndFreeMemoryCheck` |
| [WHDXGraphic.h](Client/WindHorn/WHDXGraphic.h) | 定义或声明相关类型与接口：管理 DirectX 图形窗口、显示设备和绘制环境 · `CWHDXGraphicWindow` |
| [WHDefProcess.cpp](Client/WindHorn/WHDefProcess.cpp) | 提供场景处理基类、清屏与默认窗口消息处理 · `CWHDefProcess`、`Clear`、`ShowStatus` |
| [WHDefProcess.h](Client/WindHorn/WHDefProcess.h) | 定义或声明相关类型与接口：提供场景处理基类、清屏与默认窗口消息处理 · `CWHDefProcess` |
| [WHEngine.h](Client/WindHorn/WHEngine.h) | 定义或声明相关类型与接口：汇集 WindHorn 对外提供的引擎接口 |
| [WHImage.cpp](Client/WindHorn/WHImage.cpp) | 加载位图、转换像素格式和优化图像 · `CWHImage`、`Convert24To16`、`LoadBitmap` |
| [WHImage.h](Client/WindHorn/WHImage.h) | 定义或声明相关类型与接口：加载位图、转换像素格式和优化图像 · `CWHImage` |
| [WHSurface.cpp](Client/WindHorn/WHSurface.cpp) | 创建离屏绘图表面并复制图像 · `ReleaseSurface`、`CreateOffScreenSurface`、`CopyImageToSurface` |
| [WHSurface.h](Client/WindHorn/WHSurface.h) | 定义或声明相关类型与接口：创建离屏绘图表面并复制图像 · `CWHSurface` |
| [WHWilImage.cpp](Client/WindHorn/WHWilImage.cpp) | 读取 WIL 图像档案，按索引装载帧 · `CWHWilImageData`、`Init`、`Destroy` |
| [WHWilImage.h](Client/WindHorn/WHWilImage.h) | 定义或声明相关类型与接口：读取 WIL 图像档案，按索引装载帧 · `CWHWilImageData` |
| [WHWilTexture.cpp](Client/WindHorn/WHWilTexture.cpp) | 将 WIL 图像数据复制到可绘制的纹理表面 · `TextureSearchCallback`、`FindWilTexture`、`D3DWILTextr_IsTextrExisted` |
| [WHWilTexture.h](Client/WindHorn/WHWilTexture.h) | 定义或声明相关类型与接口：将 WIL 图像数据复制到可绘制的纹理表面 |
| [WHWindow.cpp](Client/WindHorn/WHWindow.cpp) | 注册和创建 Win32 窗口并分派窗口消息 · `CWHApp`、`MainWndProc`、`WndProc` |
| [WHWindow.h](Client/WindHorn/WHWindow.h) | 定义或声明相关类型与接口：注册和创建 Win32 窗口并分派窗口消息 · `CWHApp`、`CWHWindow` |

### Client

| 文件 | 职责与定位 |
| --- | --- |
| [harrow.cur](Client/harrow.cur) | 客户端鼠标光标资源 |
| [resource.h](Client/resource.h) | 为资源脚本和窗口消息处理提供控件、菜单和字符串编号 |

### Server/AdminCmd

| 文件 | 职责与定位 |
| --- | --- |
| [AdminCmd.cpp](Server/AdminCmd/AdminCmd.cpp) | DLL 加载入口，命令名称来自同工程字符串资源 · `DllMain` |
| [AdminCmd.rc](Server/AdminCmd/AdminCmd.rc) | 提供 GM 命令名的字符串资源表 |
| [CMakeLists.txt](Server/AdminCmd/CMakeLists.txt) | 声明 AdminCmd 的构建目标、源文件和编译设置 |
| [ReadMe.txt](Server/AdminCmd/ReadMe.txt) | 保留原工程说明及文件布局说明 |
| [StdAfx.cpp](Server/AdminCmd/StdAfx.cpp) | 引入工程公共头，保留预编译头翻译单元 |
| [StdAfx.h](Server/AdminCmd/StdAfx.h) | 汇集本工程使用的系统头、公共类型和编译宏 |
| [resource.h](Server/AdminCmd/resource.h) | 为资源脚本和窗口消息处理提供控件、菜单和字符串编号 |

### Server

| 文件 | 职责与定位 |
| --- | --- |
| [CMakeLists.txt](Server/CMakeLists.txt) | 汇集各服务端目标并链接公共 Def 库 |

### Server/DBSvr

| 文件 | 职责与定位 |
| --- | --- |
| [CMakeLists.txt](Server/DBSvr/CMakeLists.txt) | 声明 DBSvr 的构建目标、源文件和编译设置 |
| [ConfigDlgFunc.cpp](Server/DBSvr/ConfigDlgFunc.cpp) | 从注册表读取配置，处理配置对话框的编辑和保存 · `ConfigDlgFunc` |
| [DBSvr.cpp](Server/DBSvr/DBSvr.cpp) | 角色数据库服务的进程初始化、数据库连接和日志窗口 · `WinMain`、`InitApplication`、`InitInstance` |
| [DBSvr.h](Server/DBSvr/DBSvr.h) | 声明本工程的连接、会话及处理接口 · `CServerInfo`、`CUserInfo`、`CGateInfo` |
| [DBSvr.rc](Server/DBSvr/DBSvr.rc) | 声明程序的 Win32 界面、图标及字符串等资源 |
| [GameGateSockMsg.cpp](Server/DBSvr/GameGateSockMsg.cpp) | 预留翻译单元，仅包含公共头 |
| [GateCommSockMsg.cpp](Server/DBSvr/GateCommSockMsg.cpp) | 接受网关连接，拆分会话控制消息和业务数据 · `UpdateStatusBarGateSession`、`InitGateCommSocket`、`OnGateCommSockMsg` |
| [GateInfo.cpp](Server/DBSvr/GateInfo.cpp) | 处理角色查询、创建、删除、选角和预载请求 · `ReceiveOpenUser`、`ReceiveCloseUser`、`ReceiveSendUser` |
| [GlobalUserList.cpp](Server/DBSvr/GlobalUserList.cpp) | 按角色名维护在线用户与服务器地址的索引 · `CGlobalUserList`、`Insert`、`Search` |
| [GlobalUserList.h](Server/DBSvr/GlobalUserList.h) | 定义或声明相关类型与接口：按角色名维护在线用户与服务器地址的索引 · `CGlobalUserList`、`CUserInfo` |
| [MainWndProc.cpp](Server/DBSvr/MainWndProc.cpp) | 分派主窗口消息，处理服务启停、配置和网络事件 · `SwitchMenuItem`、`LoadCharacterRecords`、`StopServiceThreads` |
| [ProcessDBMsg.cpp](Server/DBSvr/ProcessDBMsg.cpp) | 读写角色、技能和物品记录，分派游戏服的数据库请求 · `SendSocket`、`GetHorseRcd`、`GetHumanGenItemRcd` |
| [ProcessDBMsg.h](Server/DBSvr/ProcessDBMsg.h) | 声明 MakeNewItem 数据库操作接口 |
| [ProcessGateMsg.cpp](Server/DBSvr/ProcessGateMsg.cpp) | 解码选角请求，分派查角、建角、删角和选角 · `ProcessGateMsg` |
| [ProcessServerMsg.cpp](Server/DBSvr/ProcessServerMsg.cpp) | 预留翻译单元，仅包含公共头 |
| [ReadMe.txt](Server/DBSvr/ReadMe.txt) | 保留原工程说明及文件布局说明 |

### Server/DBSvr/Res

| 文件 | 职责与定位 |
| --- | --- |
| [MIR2.ICO](Server/DBSvr/Res/MIR2.ICO) | 窗口图标资源 |
| [TOOLBAR.BMP](Server/DBSvr/Res/TOOLBAR.BMP) | 工具栏按钮位图 |

### Server/DBSvr

| 文件 | 职责与定位 |
| --- | --- |
| [ServerCommSockMsg.cpp](Server/DBSvr/ServerCommSockMsg.cpp) | 接受游戏服连接，通过 IOCP 收包并排队提交数据库任务 · `UpdateStatusBarSession`、`AcceptThread`、`ServerWorkerThread` |
| [StdAfx.cpp](Server/DBSvr/StdAfx.cpp) | 引入工程公共头，保留预编译头翻译单元 |
| [StdAfx.h](Server/DBSvr/StdAfx.h) | 汇集本工程使用的系统头、公共类型和编译宏 |
| [TableList.cpp](Server/DBSvr/TableList.cpp) | 从数据库载入出生点，按地图名查找记录 · `CTblStartPoint`、`Init`、`Get` |
| [TableList.h](Server/DBSvr/TableList.h) | 定义或声明相关类型与接口：从数据库载入出生点，按地图名查找记录 · `CTblStartPoint` |
| [resource.h](Server/DBSvr/resource.h) | 为资源脚本和窗口消息处理提供控件、菜单和字符串编号 |

### Server/Def

| 文件 | 职责与定位 |
| --- | --- |
| [CMakeLists.txt](Server/Def/CMakeLists.txt) | 声明 Def 的构建目标、源文件和编译设置 |
| [DynamicArray.cpp](Server/Def/DynamicArray.cpp) | 管理可按槽位挂接和移除的数据 · `CWHDynamicArray`、`AttachData`、`DettachData`（当前构建未编入） |
| [DynamicArray.h](Server/Def/DynamicArray.h) | 定义或声明相关类型与接口：管理可按槽位挂接和移除的数据 |
| [EnDecode.cpp](Server/Def/EnDecode.cpp) | 在协议消息结构和 6 位文本编码之间转换 · `fnEncode6BitBufW`、`fnDecode6BitBufW`、`fnEncodeMessageW` |
| [EnDecode.h](Server/Def/EnDecode.h) | 定义或声明相关类型与接口：在协议消息结构和 6 位文本编码之间转换 |
| [List.cpp](Server/Def/List.cpp) | 提供链表节点及插入、遍历、移除操作 · `CWHList`、`AddNewNode`、`Clear`（当前构建未编入） |
| [List.h](Server/Def/List.h) | 定义或声明相关类型与接口：提供链表节点及插入、遍历、移除操作 |
| [Misc.cpp](Server/Def/Misc.cpp) | 提供窗口居中、时间转换和字符串长度等公共函数 · `CenterDialog`、`FileTimeToQuadWord`、`QuadTimeToFileTime` |
| [Misc.h](Server/Def/Misc.h) | 定义或声明相关类型与接口：提供窗口居中、时间转换和字符串长度等公共函数 |
| [ModernUI.cpp](Server/Def/ModernUI.cpp) | 按 DPI 调整窗口尺寸和日志列宽 · `Mir3QueryDpi`、`Mir3DpiScale`、`Mir3SizeMainWindow` |
| [ModernUI.h](Server/Def/ModernUI.h) | 定义或声明相关类型与接口：按 DPI 调整窗口尺寸和日志列宽 |
| [Protocol.h](Server/Def/Protocol.h) | 定义或声明相关类型与接口：定义协议消息编号和网络传输记录结构 |
| [Queue.cpp](Server/Def/Queue.cpp) | 提供消息入队和出队操作 · `CWHQueue`、`PushQ`、`PopQ`（当前构建未编入） |
| [Queue.h](Server/Def/Queue.h) | 定义或声明相关类型与接口：提供消息入队和出队操作 · `CWHQueue` |
| [RegstryHandler.cpp](Server/Def/RegstryHandler.cpp) | 读写、枚举服务配置注册表项 · `jRegSetKey`、`jRegOpenKeyRead`、`jRegGetKey` |
| [ServerSockHandler.cpp](Server/Def/ServerSockHandler.cpp) | 建立监听和主动连接，准备 IOCP 及异步接收 · `InitThread`、`InitServerSocket`、`ClearSocket` |
| [ServerSockHandler.h](Server/Def/ServerSockHandler.h) | 定义或声明相关类型与接口：建立监听和主动连接，准备 IOCP 及异步接收 |
| [StaticArray.h](Server/Def/StaticArray.h) | 定义或声明相关类型与接口：管理固定容量对象池并查找空闲槽位 · `CStaticArray` |
| [TableColumn.h](Server/Def/TableColumn.h) | 定义或声明相关类型与接口：集中定义数据库字段名 |

### Server/Def/_OrzEx

| 文件 | 职责与定位 |
| --- | --- |
| [bstree.h](Server/Def/_OrzEx/bstree.h) | 定义或声明相关类型与接口：二叉搜索树的节点、插入和遍历 · `CBsTreeNode`、`CBsTree` |
| [database.cpp](Server/Def/_OrzEx/database.cpp) | 封装 ODBC 环境、连接和结果集 · `CDatabase`、`Init`、`Uninit` |
| [database.h](Server/Def/_OrzEx/database.h) | 定义或声明相关类型与接口：封装 ODBC 环境、连接和结果集 · `CDatabase`、`CConnection`、`CRecordset` |
| [datatype.h](Server/Def/_OrzEx/datatype.h) | 定义或声明相关类型与接口：定义基础整数类型别名 |
| [error.cpp](Server/Def/_OrzEx/error.cpp) | 保存错误信息并提供错误处理接口 · `CError`、`GetMsg` |
| [error.h](Server/Def/_OrzEx/error.h) | 定义或声明相关类型与接口：保存错误信息并提供错误处理接口 · `CError` |
| [fsa.h](Server/Def/_OrzEx/fsa.h) | 定义或声明相关类型与接口：管理固定大小内存块的分配和回收 · `CFixedSizeAllocator`、`CMemBlock` |
| [indexmap.h](Server/Def/_OrzEx/indexmap.h) | 定义或声明相关类型与接口：提供按索引登记和查找对象的映射 · `CIndexMap` |
| [list.h](Server/Def/_OrzEx/list.h) | 定义通用链表模板及节点操作 · `CListNode`、`CList` |
| [map.h](Server/Def/_OrzEx/map.h) | 定义或声明相关类型与接口：提供基于散列表的键值查找 · `CMap` |
| [pqueue.h](Server/Def/_OrzEx/pqueue.h) | 定义或声明相关类型与接口：按比较函数维护优先队列 · `CPriorityQueue` |
| [prime.cpp](Server/Def/_OrzEx/prime.cpp) | 判断素数并寻找临近素数 · `IsPrime`、`RoundUp`、`RoundDown` |
| [prime.h](Server/Def/_OrzEx/prime.h) | 定义或声明相关类型与接口：判断素数并寻找临近素数 · `CPrime` |
| [queue.h](Server/Def/_OrzEx/queue.h) | 定义或声明相关类型与接口：提供消息入队和出队操作 · `CQueue` |
| [stack.h](Server/Def/_OrzEx/stack.h) | 定义或声明相关类型与接口：提供基于链表的栈 · `CStack` |
| [streambf.h](Server/Def/_OrzEx/streambf.h) | 定义或声明相关类型与接口：管理可增长的字节流缓冲区 · `CStreamBuffer` |
| [stringex.cpp](Server/Def/_OrzEx/stringex.cpp) | 管理可扩展字符串的分配、赋值和追加 · `bstr`、`alloc`、`expand` |
| [stringex.h](Server/Def/_OrzEx/stringex.h) | 定义或声明相关类型与接口：管理可扩展字符串的分配、赋值和追加 · `bstr` |
| [syncobj.cpp](Server/Def/_OrzEx/syncobj.cpp) | 封装临界区和锁操作 · `CCriticalSection`、`Lock`、`Unlock` |
| [syncobj.h](Server/Def/_OrzEx/syncobj.h) | 定义或声明相关类型与接口：封装临界区和锁操作 · `CCriticalSection` |
| [util.cpp](Server/Def/_OrzEx/util.cpp) | 提供十六进制与整数等格式转换 · `_hextodec`、`_hexdectochar`、`_dectohex` |
| [util.h](Server/Def/_OrzEx/util.h) | 定义或声明相关类型与接口：提供十六进制与整数等格式转换 |

### Server/Def

| 文件 | 职责与定位 |
| --- | --- |
| [dbmgr.cpp](Server/Def/dbmgr.cpp) | 管理 ODBC 连接、记录集和数据库错误回调 · `CDBManager`、`Init`、`CreateRecordset` |
| [dbmgr.h](Server/Def/dbmgr.h) | 定义或声明相关类型与接口：管理 ODBC 连接、记录集和数据库错误回调 · `CDBManager`、`CQueryManager` |
| [stdafx.h](Server/Def/stdafx.h) | 汇集本工程使用的系统头、公共类型和编译宏 |

### Server/GameGate

| 文件 | 职责与定位 |
| --- | --- |
| [Abusive.cpp](Server/GameGate/Abusive.cpp) | 加载聊天过滤词表并替换命中的文本 · `CWHAbusive`、`LoadAbusiveList`、`ChangeAbusiveText` |
| [Abusive.h](Server/GameGate/Abusive.h) | 定义或声明相关类型与接口：加载聊天过滤词表并替换命中的文本 · `CWHAbusive` |
| [CMakeLists.txt](Server/GameGate/CMakeLists.txt) | 声明 GameGate 的构建目标、源文件和编译设置 |
| [ClientSockMsg.cpp](Server/GameGate/ClientSockMsg.cpp) | 处理网关主动连接的后端套接字，将回包送往客户端会话 · `ProcMakeSocketStr`、`ProcReceiveBuffer`、`InitServerThreadForMsg` |
| [ConfigDlgFunc.cpp](Server/GameGate/ConfigDlgFunc.cpp) | 从注册表读取配置，处理配置对话框的编辑和保存 · `ConfigDlgFunc` |
| [GameGate.cpp](Server/GameGate/GameGate.cpp) | 游戏网关的窗口、网络环境和会话初始化 · `WinMain`、`InitApplication`、`InitInstance` |
| [GameGate.h](Server/GameGate/GameGate.h) | 声明本工程的客户端会话及异步接收结构 · `CSessionInfo` |
| [GameGate.rc](Server/GameGate/GameGate.rc) | 声明程序的 Win32 界面、图标及字符串等资源 |
| [MainWndProc.cpp](Server/GameGate/MainWndProc.cpp) | 分派主窗口消息，处理服务启停、配置和网络事件 · `SwitchMenuItem`、`OnCommand`、`MainWndProc` |

### Server/GameGate/RES

| 文件 | 职责与定位 |
| --- | --- |
| [MIR2.ICO](Server/GameGate/RES/MIR2.ICO) | 窗口图标资源 |
| [TOOLBAR.BMP](Server/GameGate/RES/TOOLBAR.BMP) | 工具栏按钮位图 |

### Server/GameGate

| 文件 | 职责与定位 |
| --- | --- |
| [RESOURCE.H](Server/GameGate/RESOURCE.H) | 为资源脚本和窗口消息处理提供控件、菜单和字符串编号 |
| [ReadMe.txt](Server/GameGate/ReadMe.txt) | 保留原工程说明及文件布局说明 |
| [ServerSockMsg.cpp](Server/GameGate/ServerSockMsg.cpp) | 接受外部客户端连接，维护会话并将接收数据交给网关处理 · `UpdateStatusBar`、`SendSocketMsgS`、`AcceptThread` |
| [StdAfx.cpp](Server/GameGate/StdAfx.cpp) | 引入工程公共头，保留预编译头翻译单元 |
| [StdAfx.h](Server/GameGate/StdAfx.h) | 汇集本工程使用的系统头、公共类型和编译宏 |
| [ThreadFuncForComm.cpp](Server/GameGate/ThreadFuncForComm.cpp) | 定时处理连接状态和后端重连 · `OnTimerProc` |
| [ThreadFuncForMsg.cpp](Server/GameGate/ThreadFuncForMsg.cpp) | 从上行队列拆包并构造 GM_DATA，处理聊天正文 · `ThreadFuncForMsg` |

### Server/GameSvr

| 文件 | 职责与定位 |
| --- | --- |
| [CMakeLists.txt](Server/GameSvr/CMakeLists.txt) | 声明 GameSvr 的构建目标、源文件和编译设置 |
| [ConfigDlgFunc.cpp](Server/GameSvr/ConfigDlgFunc.cpp) | 从注册表读取配置，处理配置对话框的编辑和保存 · `ConfigDlgFunc` |
| [DataHandler.h](Server/GameSvr/DataHandler.h) | 定义或声明相关类型与接口：定义地图文件头、瓦片和地图单元数据 · `CMapCellInfo`、`CMirMap` |
| [FrontEngine.h](Server/GameSvr/FrontEngine.h) | 定义或声明相关类型与接口：管理登录等待记录和角色加载接口 · `CReadyUserInfo`、`CReadyUserInfo2` |
| [GameSvr.cpp](Server/GameSvr/GameSvr.cpp) | 游戏服务的应用程序入口、数据库连接和窗口初始化 · `__cbDBMsg`、`LogMsgWndProc`、`WinMain` |
| [GameSvr.h](Server/GameSvr/GameSvr.h) | 声明本工程的连接、会话及处理接口 · `CGateInfo`、`CUserInfo` |
| [GameSvr.rc](Server/GameSvr/GameSvr.rc) | 声明程序的 Win32 界面、图标及字符串等资源 |
| [GateInfo.cpp](Server/GameSvr/GateInfo.cpp) | 维护网关上的游戏会话，处理玩家连接及网关检查 · `CGateInfo`、`SendGateCheck`、`OpenNewUser` |
| [Global.cpp](Server/GameSvr/Global.cpp) | 集中管理在线用户、玩家对象池及共享世界数据 |
| [Global.h](Server/GameSvr/Global.h) | 定义或声明相关类型与接口：集中管理在线用户、玩家对象池及共享世界数据 |
| [Handler_CommonDB.cpp](Server/GameSvr/Handler_CommonDB.cpp) | 从数据库载入地图、怪物生成、商人、技能和物品基础资料 · `InitMapInfo`、`InitStdItemEtcInfo`、`InitStdItemSpecial` |
| [Handler_Item.cpp](Server/GameSvr/Handler_Item.cpp) | 计算物品升级属性并生成标准物品记录 · `GetUpgrade`、`GetStandardItem`、`CStdItemSpecial` |
| [Handler_Magic.cpp](Server/GameSvr/Handler_Magic.cpp) | 计算魔法威力及技能升级条件 · `GetPower13`、`CheckMagicLevelup` |
| [Handler_Map.cpp](Server/GameSvr/Handler_Map.cpp) | 加载地图并管理格子、对象和通行状态 · `CMapCellInfo`、`LoadMap`、`GetMap` |
| [ItemHandler.h](Server/GameSvr/ItemHandler.h) | 定义或声明相关类型与接口：定义地图物品、物品实例及标准物品类型 · `CMapItem`、`CItem`、`CStdItem` |
| [MagicHandler.h](Server/GameSvr/MagicHandler.h) | 定义或声明相关类型与接口：定义技能资料及威力、升级计算接口 · `CMagicInfo` |
| [MainWndProc.cpp](Server/GameSvr/MainWndProc.cpp) | 分派主窗口消息，处理服务启停、配置和网络事件 · `SwitchMenuItem`、`InitDataInDatabase`、`InitializingServer` |
| [ObjectEngine.cpp](Server/GameSvr/ObjectEngine.cpp) | 实现角色对象共有的定位、目标选择和动作逻辑 · `CCharObject`、`SelectTarget`、`GetFrontPosition` |
| [ObjectEngine.h](Server/GameSvr/ObjectEngine.h) | 定义或声明相关类型与接口：实现角色对象共有的定位、目标选择和动作逻辑 · `CVisibleObject`、`CVisibleEvent`、`CVisibleMapItem` |
| [ObjectEvent.cpp](Server/GameSvr/ObjectEvent.cpp) | 管理地图事件的创建、运行和关闭 · `Create`、`Close`、`Run` |
| [ObjectEvent.h](Server/GameSvr/ObjectEvent.h) | 定义或声明相关类型与接口：管理地图事件的创建、运行和关闭 · `CEvent`、`CFireBurnEvent`、`CHolyCurtainEvent` |
| [ObjectMerchant.cpp](Server/GameSvr/ObjectMerchant.cpp) | 驱动商人对象的运行逻辑 · `CMerchantObject`、`RunRace` |
| [ObjectMonster.cpp](Server/GameSvr/ObjectMonster.cpp) | 管理怪物的目标判断、行为和状态 · `CMonsterObject`、`KickException`、`MakeClone` |
| [ObjectMonster.h](Server/GameSvr/ObjectMonster.h) | 定义或声明相关类型与接口：管理怪物的目标判断、行为和状态 · `CHolySeizeInfo`、`CMonsterObject`、`CGenneralMonObject` |
| [ObjectNPC.cpp](Server/GameSvr/ObjectNPC.cpp) | 初始化 NPC 的运行间隔与光照等属性 · `CNPCObject` |
| [ObjectNPC.h](Server/GameSvr/ObjectNPC.h) | 声明 NPC 与商人对象类型及行为接口 · `CNPCObject`、`CMerchantObject` |
| [ObjectPlayer.cpp](Server/GameSvr/ObjectPlayer.cpp) | 管理玩家对象、动作、消息处理和状态变化 · `Constructor`、`IsEmpty`、`CPlayerObject` |
| [ObjectPlayer.h](Server/GameSvr/ObjectPlayer.h) | 定义或声明相关类型与接口：管理玩家对象、动作、消息处理和状态变化 · `CPlayerObject` |
| [ObjectPlayerAdmin.cpp](Server/GameSvr/ObjectPlayerAdmin.cpp) | 实现传送、造物、召怪等管理员操作 · `CmdChangeItemPrefix`、`CmdRandomSpaceMove`、`CmdFreeSpaceMove` |
| [ObjectPlayerAdminSaid.cpp](Server/GameSvr/ObjectPlayerAdminSaid.cpp) | 从 AdminCmd 加载命令名并解析、分派 @ 命令 · `InitAdminCommandList`、`UnInitAdminCommandList`、`ProcessForAdminSaid` |
| [ObjectPlayerClone.cpp](Server/GameSvr/ObjectPlayerClone.cpp) | 管理玩家分身的创建与更新 · `Create`、`Operate` |
| [ObjectPlayerClone.h](Server/GameSvr/ObjectPlayerClone.h) | 定义或声明相关类型与接口：管理玩家分身的创建与更新 · `CPlayerCloneObject` |
| [ObjectPlayerSaid.cpp](Server/GameSvr/ObjectPlayerSaid.cpp) | 处理玩家聊天输入和聊天命令 · `InitUserCommandList`、`UnInitUserCommandList`、`ProcessForUserSaid` |
| [ObjectScripter.cpp](Server/GameSvr/ObjectScripter.cpp) | 管理脚本驱动的玩家对象及其运行 · `Create`、`Operate` |
| [ObjectScripter.h](Server/GameSvr/ObjectScripter.h) | 定义或声明相关类型与接口：管理脚本驱动的玩家对象及其运行 · `CScripterObject` |
| [ProcessEvents.cpp](Server/GameSvr/ProcessEvents.cpp) | 遍历并更新地图事件 · `ProcessEvents` |
| [ProcessLogin.cpp](Server/GameSvr/ProcessLogin.cpp) | 匹配进入游戏的会话与预载角色，构建玩家对象 · `SendRDBSocket`、`LoadHumanFromDB`、`MakeItemToDB` |
| [ProcessMerchants.cpp](Server/GameSvr/ProcessMerchants.cpp) | 调度商人对象执行周期行为 · `ProcessMerchants` |
| [ProcessMonster.cpp](Server/GameSvr/ProcessMonster.cpp) | 调度怪物生成、刷新和行为更新 · `AddCreature`、`RegenMonster`、`GetZenTime` |
| [ProcessNPC.cpp](Server/GameSvr/ProcessNPC.cpp) | 周期调度 NPC、商人和地图事件 · `ProcessNPC` |
| [ProcessUserHuman.cpp](Server/GameSvr/ProcessUserHuman.cpp) | 遍历在线玩家，驱动会话和角色更新 · `GetUserHuman`、`CryCry`、`ProcessUserHuman` |
| [ProcessUserMsg.cpp](Server/GameSvr/ProcessUserMsg.cpp) | 保留消息文本宏，当前文件没有用户消息分派实现 |
| [ReadMe.txt](Server/GameSvr/ReadMe.txt) | 保留原工程说明及文件布局说明 |

### Server/GameSvr/Res

| 文件 | 职责与定位 |
| --- | --- |
| [MIR2.ICO](Server/GameSvr/Res/MIR2.ICO) | 窗口图标资源 |
| [toolbar.bmp](Server/GameSvr/Res/toolbar.bmp) | 工具栏按钮位图 |

### Server/GameSvr

| 文件 | 职责与定位 |
| --- | --- |
| [SockMsg_DBSrv.cpp](Server/GameSvr/SockMsg_DBSrv.cpp) | 处理数据库连接与角色、物品回包，连接成功后启动游戏线程及网关监听 · `ProcReceiveBuffer`、`OnClientSockMsg` |
| [SockMsg_GateComm.cpp](Server/GameSvr/SockMsg_GateComm.cpp) | 接受游戏网关连接，处理会话打开、关闭和游戏数据 · `UpdateStatusBarSession`、`UpdateStatusBarUsers`、`AcceptThread` |
| [SockMsg_LogSvr.cpp](Server/GameSvr/SockMsg_LogSvr.cpp) | 保留日志服务套接字事件入口，启动路径中的连接调用已注释 · `OnLogSvrSockMsg` |
| [StdAfx.cpp](Server/GameSvr/StdAfx.cpp) | 引入工程公共头，保留预编译头翻译单元 |
| [StdAfx.h](Server/GameSvr/StdAfx.h) | 汇集本工程使用的系统头、公共类型和编译宏 |
| [Tables.h](Server/GameSvr/Tables.h) | 定义或声明相关类型与接口：定义怪物、刷新点等游戏基础资料的内存结构 · `CMonItem`、`CMonRaceInfo`、`CMonsterGenInfo` |
| [ThreadFuncForComm.cpp](Server/GameSvr/ThreadFuncForComm.cpp) | 定时处理连接状态和后端重连 · `OnTimerProc` |
| [UserInfo.cpp](Server/GameSvr/UserInfo.cpp) | 管理在线会话、认证报文、玩家操作与角色存档请求 · `CUserInfo`、`IsEmpty`、`AddNewItem` |
| [capture.txt](Server/GameSvr/capture.txt) | 保留历史消息字段的文本样本 |
| [resource.h](Server/GameSvr/resource.h) | 为资源脚本和窗口消息处理提供控件、菜单和字符串编号 |

### Server/LoginGate

| 文件 | 职责与定位 |
| --- | --- |
| [CMakeLists.txt](Server/LoginGate/CMakeLists.txt) | 声明 LoginGate 的构建目标、源文件和编译设置 |
| [ClientSockMsg.cpp](Server/LoginGate/ClientSockMsg.cpp) | 处理网关主动连接的后端套接字，将回包送往客户端会话 · `OnClientSockMsg`、`SendExToServer` |
| [ConfigDlgFunc.cpp](Server/LoginGate/ConfigDlgFunc.cpp) | 从注册表读取配置，处理配置对话框的编辑和保存 · `ConfigDlgFunc` |
| [IOCPFunc.cpp](Server/LoginGate/IOCPFunc.cpp) | 创建完成端口工作线程的旧实现，工程使用公共网络代码 · `CheckAvailableIOCP`（当前构建未编入） |
| [IOCPFunc.h](Server/LoginGate/IOCPFunc.h) | 定义或声明相关类型与接口：创建完成端口工作线程的旧实现，工程使用公共网络代码 |
| [LoginGate.cpp](Server/LoginGate/LoginGate.cpp) | 登录网关的应用程序入口和网络初始化 · `WinMain`、`InitApplication`、`InitInstance` |
| [LoginGate.h](Server/LoginGate/LoginGate.h) | 声明本工程的客户端会话及异步接收结构 · `CSessionInfo` |
| [LoginGate.rc](Server/LoginGate/LoginGate.rc) | 声明程序的 Win32 界面、图标及字符串等资源 |
| [MainWndProc.cpp](Server/LoginGate/MainWndProc.cpp) | 分派主窗口消息，处理服务启停、配置和网络事件 · `SetFontColor`、`SetBackColor`、`OnCommand` |
| [RESOURCE.H](Server/LoginGate/RESOURCE.H) | 为资源脚本和窗口消息处理提供控件、菜单和字符串编号 |
| [ReadMe.txt](Server/LoginGate/ReadMe.txt) | 保留原工程说明及文件布局说明 |

### Server/LoginGate/Res

| 文件 | 职责与定位 |
| --- | --- |
| [CONNECT.ICO](Server/LoginGate/Res/CONNECT.ICO) | 窗口图标资源 |
| [ICON1.ICO](Server/LoginGate/Res/ICON1.ICO) | 窗口图标资源 |
| [MIR2.ICO](Server/LoginGate/Res/MIR2.ICO) | 窗口图标资源 |
| [TOOLBAR.BMP](Server/LoginGate/Res/TOOLBAR.BMP) | 工具栏按钮位图 |

### Server/LoginGate

| 文件 | 职责与定位 |
| --- | --- |
| [ServerSockMsg.cpp](Server/LoginGate/ServerSockMsg.cpp) | 接受外部客户端连接，维护会话并将接收数据交给网关处理 · `UpdateStatusBar`、`AcceptThread`、`CloseSession` |
| [StdAfx.cpp](Server/LoginGate/StdAfx.cpp) | 引入工程公共头，保留预编译头翻译单元 |
| [StdAfx.h](Server/LoginGate/StdAfx.h) | 汇集本工程使用的系统头、公共类型和编译宏 |
| [ThreadFuncForComm.cpp](Server/LoginGate/ThreadFuncForComm.cpp) | 定时处理连接状态和后端重连 · `OnTimerProc` |
| [ThreadFuncForMsg.cpp](Server/LoginGate/ThreadFuncForMsg.cpp) | 消费后端回包队列，按会话标识交付客户端 · `ThreadFuncForMsg` |

### Server/LoginSvr

| 文件 | 职责与定位 |
| --- | --- |
| [AddSvrListFunc.cpp](Server/LoginSvr/AddSvrListFunc.cpp) | 处理添加服务器条目的对话框 · `AddSvrListProc` |
| [CMakeLists.txt](Server/LoginSvr/CMakeLists.txt) | 声明 LoginSvr 的构建目标、源文件和编译设置 |
| [ConfigDlgFunc.cpp](Server/LoginSvr/ConfigDlgFunc.cpp) | 从注册表读取配置，处理配置对话框的编辑和保存 · `ConfigDlgFunc` |
| [GateCommSockMsg.cpp](Server/LoginSvr/GateCommSockMsg.cpp) | 接受网关连接，拆分会话控制消息和业务数据 · `UpdateStatusBar`、`InitServerThreadForMsg`、`AcceptThread` |
| [GateInfo.cpp](Server/LoginSvr/GateInfo.cpp) | 管理登录会话，处理账号注册、登录校验及区服选择 · `CompareDBString`、`GetCertification`、`Close` |
| [LoginSvr.cpp](Server/LoginSvr/LoginSvr.cpp) | 账号服务的应用程序入口、数据库和界面初始化 · `WinMain`、`InitApplication`、`InitInstance` |
| [LoginSvr.h](Server/LoginSvr/LoginSvr.h) | 声明本工程的连接、会话及处理接口 · `CUserInfo`、`CGateInfo` |
| [LoginSvr.rc](Server/LoginSvr/LoginSvr.rc) | 声明程序的 Win32 界面、图标及字符串等资源 |
| [MainWndProc.cpp](Server/LoginSvr/MainWndProc.cpp) | 分派主窗口消息，处理服务启停、配置和网络事件 · `SwitchMenuItem`、`CreateConfigProperties`、`LoadAccountRecords` |
| [ReadMe.txt](Server/LoginSvr/ReadMe.txt) | 保留原工程说明及文件布局说明 |

### Server/LoginSvr/Res

| 文件 | 职责与定位 |
| --- | --- |
| [MIR2.ICO](Server/LoginSvr/Res/MIR2.ICO) | 窗口图标资源 |
| [toolbar.bmp](Server/LoginSvr/Res/toolbar.bmp) | 工具栏按钮位图 |

### Server/LoginSvr

| 文件 | 职责与定位 |
| --- | --- |
| [ServerListProc.cpp](Server/LoginSvr/ServerListProc.cpp) | 管理登录服务配置中的服务器列表 · `InitListView`、`ServerListProc` |
| [StatusDlg.cpp](Server/LoginSvr/StatusDlg.cpp) | 显示登录服务监听、网关连接及游戏服务器列表的内存快照 · `InsertStatusRow`、`AdjustStatusColumns`、`InitStatusColumns` |
| [StdAfx.cpp](Server/LoginSvr/StdAfx.cpp) | 引入工程公共头，保留预编译头翻译单元 |
| [StdAfx.h](Server/LoginSvr/StdAfx.h) | 汇集本工程使用的系统头、公共类型和编译宏 |
| [ThreadFuncForMsg.cpp](Server/LoginSvr/ThreadFuncForMsg.cpp) | 从各网关队列取包，按消息号派发账号与选服请求 · `ThreadFuncForMsg` |
| [resource.h](Server/LoginSvr/resource.h) | 为资源脚本和窗口消息处理提供控件、菜单和字符串编号 |

### Server/ManConsole

| 文件 | 职责与定位 |
| --- | --- |
| [CMakeLists.txt](Server/ManConsole/CMakeLists.txt) | 声明 ManConsole 的构建目标、源文件和编译设置 |
| [ConsoleSocket.cpp](Server/ManConsole/ConsoleSocket.cpp) | 封装套接字创建、连接、监听和收发 · `CConsoleSocket`、`Create`、`Connect` |
| [ConsoleSocket.h](Server/ManConsole/ConsoleSocket.h) | 定义或声明相关类型与接口：封装套接字创建、连接、监听和收发 · `CConsoleSocket` |
| [ManConsole.cpp](Server/ManConsole/ManConsole.cpp) | 管理控制台的对话框入口和日志输出 · `WinMain`、`InitInstance`、`AddNewLogMsg` |
| [ManConsole.h](Server/ManConsole/ManConsole.h) | 声明本工程的日志和窗口接口 |
| [ManConsole.rc](Server/ManConsole/ManConsole.rc) | 声明程序的 Win32 界面、图标及字符串等资源 |
| [ManConsoleDlg.cpp](Server/ManConsole/ManConsoleDlg.cpp) | 初始化日志列表与命令编辑框，处理关于和关闭对话框 · `AboutDlgProc`、`ManConsoleDlgProc` |
| [ReadMe.txt](Server/ManConsole/ReadMe.txt) | 保留原工程说明及文件布局说明 |
| [StdAfx.cpp](Server/ManConsole/StdAfx.cpp) | 引入工程公共头，保留预编译头翻译单元 |
| [StdAfx.h](Server/ManConsole/StdAfx.h) | 汇集本工程使用的系统头、公共类型和编译宏 |

### Server/ManConsole/res

| 文件 | 职责与定位 |
| --- | --- |
| [ManConsole.ico](Server/ManConsole/res/ManConsole.ico) | 窗口图标资源 |

### Server/ManConsole

| 文件 | 职责与定位 |
| --- | --- |
| [resource.h](Server/ManConsole/resource.h) | 为资源脚本和窗口消息处理提供控件、菜单和字符串编号 |

### Server/SQL_Scripts

| 文件 | 职责与定位 |
| --- | --- |
| [create_db.sql](Server/SQL_Scripts/create_db.sql) | 重建游戏数据库并指定数据文件、日志文件 |
| [create_tbl.sql](Server/SQL_Scripts/create_tbl.sql) | 集中创建账号、怪物及物品等多张业务表 |
| [create_tbl_account.sql](Server/SQL_Scripts/create_tbl_account.sql) | 建立 TBL_ACCOUNT 表，用于账号资料 |
| [create_tbl_accountadd.sql](Server/SQL_Scripts/create_tbl_accountadd.sql) | 建立 TBL_ACCOUNTADD 表，用于账号附加资料 |
| [create_tbl_character.sql](Server/SQL_Scripts/create_tbl_character.sql) | 建立 TBL_CHARACTER 表，用于角色主记录 |
| [create_tbl_character_item.sql](Server/SQL_Scripts/create_tbl_character_item.sql) | 建立 TBL_CHARACTER_ITEM 表，用于角色物品实例 |
| [create_tbl_character_magic.sql](Server/SQL_Scripts/create_tbl_character_magic.sql) | 建立 TBL_CHARACTER_MAGIC 表，用于角色技能记录 |
| [create_tbl_guard.sql](Server/SQL_Scripts/create_tbl_guard.sql) | 建立 TBL_GUARD 表，用于守卫资料 |
| [create_tbl_horse.sql](Server/SQL_Scripts/create_tbl_horse.sql) | 建立 TBL_HORSE 表，用于坐骑资料 |
| [create_tbl_magic.sql](Server/SQL_Scripts/create_tbl_magic.sql) | 建立 TBL_MAGIC 表，用于技能定义 |
| [create_tbl_mapinfo.sql](Server/SQL_Scripts/create_tbl_mapinfo.sql) | 建立 TBL_MAPINFO 表，用于地图参数 |
| [create_tbl_merchant.sql](Server/SQL_Scripts/create_tbl_merchant.sql) | 建立 TBL_MERCHANT 表，用于商人资料 |
| [create_tbl_mongen.sql](Server/SQL_Scripts/create_tbl_mongen.sql) | 建立 TBL_MONGEN 表，用于怪物刷新配置 |
| [create_tbl_monster.sql](Server/SQL_Scripts/create_tbl_monster.sql) | 建立 TBL_MONSTER 表，用于怪物定义 |
| [create_tbl_movemapevnt.sql](Server/SQL_Scripts/create_tbl_movemapevnt.sql) | 建立 TBL_MOVEMAPEVENT 表，用于跨地图移动事件 |
| [create_tbl_npc.sql](Server/SQL_Scripts/create_tbl_npc.sql) | 建立 TBL_NPC 表，用于NPC 资料 |
| [create_tbl_serverinfo.sql](Server/SQL_Scripts/create_tbl_serverinfo.sql) | 建立 TBL_SERVERINFO 表，用于区服及连接地址 |
| [create_tbl_startpnt.sql](Server/SQL_Scripts/create_tbl_startpnt.sql) | 建立 TBL_STARTPOINT 表，用于地图出生点 |
| [create_tbl_stditem.sql](Server/SQL_Scripts/create_tbl_stditem.sql) | 建立 TBL_STDITEMWEAPON 表，用于武器模板 |
| [create_tbl_stditem_Accessory.sql](Server/SQL_Scripts/create_tbl_stditem_Accessory.sql) | 建立 TBL_STDITEM_ACCESSORY 表，用于首饰模板 |
| [create_tbl_stditem_etc.sql](Server/SQL_Scripts/create_tbl_stditem_etc.sql) | 建立 TBL_STDITEM_ETC 表，用于杂项物品模板 |
| [create_tbl_stditemarmor.sql](Server/SQL_Scripts/create_tbl_stditemarmor.sql) | 建立 TBL_STDITEMARMOR 表，用于防具模板 |
| [create_tbl_stditemweapon.sql](Server/SQL_Scripts/create_tbl_stditemweapon.sql) | 建立 TBL_STDITEMWEAPON 表，用于武器模板 |
| [create_tbl_useritem.sql](Server/SQL_Scripts/create_tbl_useritem.sql) | 建立 TBL_USERITEM 表，用于玩家物品记录 |
| [create_usr.sql](Server/SQL_Scripts/create_usr.sql) | 创建游戏数据库相关用户 |

### Server/SelGate

| 文件 | 职责与定位 |
| --- | --- |
| [CMakeLists.txt](Server/SelGate/CMakeLists.txt) | 声明 SelGate 的构建目标、源文件和编译设置 |
| [ClientSockMsg.cpp](Server/SelGate/ClientSockMsg.cpp) | 处理网关主动连接的后端套接字，将回包送往客户端会话 · `OnClientSockMsg`、`SendExToServer` |
| [ConfigDlgFunc.cpp](Server/SelGate/ConfigDlgFunc.cpp) | 从注册表读取配置，处理配置对话框的编辑和保存 · `ConfigDlgFunc` |
| [IOCPFunc.cpp](Server/SelGate/IOCPFunc.cpp) | 创建完成端口工作线程的旧实现，工程使用公共网络代码 · `CheckAvailableIOCP`（当前构建未编入） |
| [IOCPFunc.h](Server/SelGate/IOCPFunc.h) | 定义或声明相关类型与接口：创建完成端口工作线程的旧实现，工程使用公共网络代码 |
| [LoginGate.cpp](Server/SelGate/LoginGate.cpp) | 保留旧登录网关入口，SelGate 的构建清单使用 SelGate.cpp · `WinMain`、`InitApplication`、`InitInstance` |
| [MainWndProc.cpp](Server/SelGate/MainWndProc.cpp) | 分派主窗口消息，处理服务启停、配置和网络事件 · `SetFontColor`、`SetBackColor`、`OnCommand` |
| [RESOURCE.H](Server/SelGate/RESOURCE.H) | 为资源脚本和窗口消息处理提供控件、菜单和字符串编号 |
| [ReadMe.txt](Server/SelGate/ReadMe.txt) | 保留原工程说明及文件布局说明 |

### Server/SelGate/Res

| 文件 | 职责与定位 |
| --- | --- |
| [MIR2.ICO](Server/SelGate/Res/MIR2.ICO) | 窗口图标资源 |
| [TOOLBAR.BMP](Server/SelGate/Res/TOOLBAR.BMP) | 工具栏按钮位图 |

### Server/SelGate

| 文件 | 职责与定位 |
| --- | --- |
| [SelGate.cpp](Server/SelGate/SelGate.cpp) | 选角网关的应用程序入口和网络初始化 · `WinMain`、`InitApplication`、`InitInstance` |
| [SelGate.h](Server/SelGate/SelGate.h) | 声明本工程的客户端会话及异步接收结构 · `CSessionInfo` |
| [SelGate.rc](Server/SelGate/SelGate.rc) | 声明程序的 Win32 界面、图标及字符串等资源 |
| [ServerSockMsg.cpp](Server/SelGate/ServerSockMsg.cpp) | 接受外部客户端连接，维护会话并将接收数据交给网关处理 · `UpdateStatusBar`、`AcceptThread`、`CloseSession` |
| [StdAfx.cpp](Server/SelGate/StdAfx.cpp) | 引入工程公共头，保留预编译头翻译单元 |
| [StdAfx.h](Server/SelGate/StdAfx.h) | 汇集本工程使用的系统头、公共类型和编译宏 |
| [ThreadFuncForComm.cpp](Server/SelGate/ThreadFuncForComm.cpp) | 定时处理连接状态和后端重连 · `OnTimerProc` |
| [ThreadFuncForMsg.cpp](Server/SelGate/ThreadFuncForMsg.cpp) | 消费后端回包队列，按会话标识交付客户端 · `ThreadFuncForMsg` |

### Server/Tools

| 文件 | 职责与定位 |
| --- | --- |
| [CMakeLists.txt](Server/Tools/CMakeLists.txt) | 声明 Tools 的构建目标、源文件和编译设置 |
| [EncodeSize.cpp](Server/Tools/EncodeSize.cpp) | 输出角色记录经过 6 位编码后的字节数 · `fnEncode6BitBufA`、`main` |
| [test_encode.cpp](Server/Tools/test_encode.cpp) | 对默认消息头执行编码、解码并打印字段和结构大小 · `memlen`、`fnEncode6BitBufA`、`fnDecode6BitBufA` |

### Server/VirtualClient

| 文件 | 职责与定位 |
| --- | --- |
| [CMakeLists.txt](Server/VirtualClient/CMakeLists.txt) | 声明 VirtualClient 的构建目标、源文件和编译设置 |
| [DlgFunc.cpp](Server/VirtualClient/DlgFunc.cpp) | 提供组包发送函数及登录、注册对话框，当前确认按钮只关闭窗口 · `SendPacket`、`LoginFunc`、`NewUserFunc` |
| [InitClient.cpp](Server/VirtualClient/InitClient.cpp) | 建立与登录网关的异步连接，接收消息并写入日志 · `ConnectServer`、`OnSockMsg` |
| [InitClient.h](Server/VirtualClient/InitClient.h) | 空头文件，占位 |
| [RESOURCE.H](Server/VirtualClient/RESOURCE.H) | 为资源脚本和窗口消息处理提供控件、菜单和字符串编号 |
| [ReadMe.txt](Server/VirtualClient/ReadMe.txt) | 保留原工程说明及文件布局说明 |
| [StdAfx.cpp](Server/VirtualClient/StdAfx.cpp) | 引入工程公共头，保留预编译头翻译单元 |
| [StdAfx.h](Server/VirtualClient/StdAfx.h) | 汇集本工程使用的系统头、公共类型和编译宏 |
| [VirtualClient.cpp](Server/VirtualClient/VirtualClient.cpp) | 虚拟客户端入口，连接登录网关并响应登录、注册菜单 · `AddNewLogMsg`、`InsertLogMsg`、`WinMain` |
| [VirtualClient.rc](Server/VirtualClient/VirtualClient.rc) | 声明程序的 Win32 界面、图标及字符串等资源 |
