# 客户端功能参考与实现选择

按当前 AGENTS.md 的文件名 RERENCES.md 维护。工程索引见 ../reference/ANALYSIS_INDEX.md；先前名称/发言/离场对照记录在 ../reference/HELP.md，历史模块后续逐项归并。

## 地图掉落物与拾取（实现前记录，2026-11-14）

- 原版既有分析：旧 mir3_1.45/gameplay.cpp 的 press_pointer / update_pointer 记录旧 Mir3.dat 0x67B26D..0x67B2B5：原地左键可发拾取，即使无客户端掉落物缓存；空闲、无路径/攻击等状态检查，持续左键 >0.3 秒后再请求。此证据来自旧二进制，目标 1.45 的完整地面绘制/文字混色尚未有本批新 IDA 证据。
- LegendOfMir3_Src：客户端 GameProcess/GameProc.cpp 的 SM_ITEMSHOW/SM_ITEMHIDE 使用 610/611，识别号去重、坐标和 looks 来自三个参数、名字六位解码；Ground.wil 由 Common/ImageHandler.cpp 映射。绘制按 48×32 格中心减图片半宽/高，忽略图片偏移，先影子后图；DropItemShow 仅鼠标所在格的首物品显示名称，中心 x+24、y+4，字顶 y-4，RGB(255,255,125)。Common/ClientSocket.cpp::SendPickUp 发 1001，recognition=0、当前 x/y、series=0。
- legendOfMir2_src：Mir3EIClientDev/Mir3EIClient/GameProcess/GameProc.cpp 的 610/611 和 DropItemShow 同源；Common/ImageHandler.cpp 同样 Ground.wil，库编号不同，不按编号移植。
- Zircon：Client/Models/ItemObject.cs 使用物品堆代表、库内 drawIndex、格中心绘制；MouseOver 返回 false，名称独立定位。Client/Scenes/GameScene.cs 拾取按键受 250ms 时间限制。其对象/物品堆与网络结构不同，不借用编号或冷却替代原版。
- mir2x：client/src/processrunnet.cpp::on_SM_GROUNDITEMIDLIST 按 mapUID 和格子整批替换物品 ID 列表，协议不同；不把其列表替换语义当成 610 单对象新增。

选择：以两套早期 C++ 的 610/611、Ground.wil、格中心和名称坐标作当前可追溯实现；以旧原版分析接原地左键拾取信号，保持服务器确认后才移除物品/加背包。保留多个物品同格，不将物品纳入移动阻挡。原版精确透明影子、悬停提亮与地图变更清理语义需进一步核验，不能据参考代码声称像素一致。

## 地图切换与场景重置（实现前记录，2026-11-14）

- 原版既有分析/旧 Godot：mir3_1.45/gameplay.cpp::cross_exit 先验证目标地图和出生格，再交换地图，清路径、指针、攻击和对话；其 MockServer 不可作为真实网络协议证据。注释记录 0x42EE00 消费 51/634 的环境亮度低字节，本批不推断完整包结构。
- LegendOfMir3_Src 客户端 GameProcess/GameProc.cpp::LoadMapChanged 加载地图并清可见格缓存；SM_CHANGEMAP 分支重置本地站立、坐标、视点，关闭 NPC 窗口，还包含雾色与 2300ms 输入/混色延迟。
- legendOfMir2_src 的 Mir3EIClient/GameProcess/GameProc.cpp 同源 LoadMapChanged；地图数据和显示格缓存分离。不能仅靠该函数声称远端对象队列已全部清理。
- Zircon Client/Envir/CConnection.cs::Process(S.MapChanged) 切 MapInfo/InstanceInfo，UserLocation 独立纠正位置；Client/Scenes/Views/MapControl.cs 管理对象集合清理。实例与协议结构不同。
- mir2x client/src/processrun.cpp::loadMap 停音效、清地面物品、按 mapUID 加载地图；相同地图资源可供不同实例复用，不能靠显示名称判断是否需要重置。

选择：提供独立的已解码 change_map(map_id, name, spawn) 入口，以旧 Godot 的先校验后切换为基础，复用现有资源解析器和人物移除逻辑；每次成功切换（包括同图重入）清远端对象、掉落、路径、临时提示、NPC 对话和小地图内容。失败不改变原场景。资源仍从配置根目录读取，地图编号只接受文件名，不接受路径。显示名称更新仍是独立接口。原版雾色、输入锁/淡入和真实换图包接入继续待办，不声称该接口已完整复刻网络换图。

验证：Release 构建；Godot --map-change --screenshots 单场景覆盖不存在/路径型地图编号、越界及阻挡落点失败保留现场、0→01→0 真地图切换、同图同名重入、小地图失效/重请求、旧移除队列不能删除新图同 ID 人物。截图 map-change-before/arrival/return.png 位于 build/screenshots。未跑无关场景。

## 环境亮度与场景照明（实现前记录，2026-11-14）

- 旧 mir3_1.45/gameplay.cpp::apply_ambient_mode / receive_ambient_update / update：既有 0x42EE00、0x429213、0x425E10 分析，入场模式 0/1/2 对应 255/15/85；46 使用第三参数，50ms 增减 0.5，入场未知模式为 10、渐变未知目标为 0。保留这一区别，不混用早期参考字段。
- 同项目 spell_light.hpp（0x448C30、0x40EB70）及 extension.cpp::scene_lighting：18×19 顶点、48×32 格、灯中心偏移 +8/+29、半径与颜色线性衰减、饱和相加、三角形对角线；gameplay.cpp::map_light 使用地图格 +12 的灯字及颜色/半径表。world.gd::_draw_world_spell_light 在场景与覆盖文字之间进行 RGB 相乘。本批按功能迁移 C++，不引入旧 MockServer 或 GDScript 业务。
- LegendOfMir3_Src GameProc.cpp::SM_DAYCHANGING 使用 wParam/wTag，并设置 50ms 定时器；换图雾色 240/10/50，与目标旧分析 255/15/85 不同。
- legendOfMir2_src Mir3EIClient/GameProcess/GameProc.cpp 同源 SM_DAYCHANGING/ChangeLightColor；不能作为目标 1.45 参数布局的替代证据。
- Zircon Client/Scenes/Views/MapControl.cs::Light.UpdateAmbientLight 使用独立照明层，Night=15、Twilight=100、Light=255，默认乘 DayTime；只参考场景/界面分层，不沿用其暮色值。
- mir2x client/src 的 processrun 绘制与 light 搜索未定位对应原版昼夜网格实现；无可复用证据，不以特效类 LightBolt 的名字推断环境算法。

选择：独立 C++ 环境状态与灯网格模型，支持 46 原始消息渐变、change_map 入场模式、静态地图灯及默认人物灯；GPU 使用场景 RGB 乘法，HUD/名称/对话在照明后绘制。装备灯属性与法术光源仍需接消息/特效模块，不能标记完整夜景已还原。

## 人物灯属性更新（实现前记录，2026-11-14）

- 旧 mir3_1.45/gameplay.hpp::receive_player_light 记录 0x415AD0 将 807 分发至 0x4117D0，从 record+11 复制到 actor descriptor+20，即第三参数高字节；spell_light.hpp::actor 给出本地/远端半径差异。MockServer 项链属性 2 仅是测试数据，不能自动从装备图标推断。
- LegendOfMir3_Src Actor.cpp::OnChangeLight 和 legendOfMir2_src Mir3EIClient/Actor.cpp 同名函数均读 SM_CHANGELIGHT=654 的 wParam。原版分析优先，不将 654 当成 807 的别名。
- Zircon Client/Models/PlayerObject.cs 从对象信息取 Light，ServerLibrary/Models/PlayerObject.cs 从 Stats[Stat.Light] 填入对象信息；参考“服务端决定灯属性”的边界，不照搬消息格式。
- mir2x client/src/hero.cpp 未找到目标人物灯属性消息对应实现，不能以名字相似代码推断灯属性。

选择：显式登记本地网络 ID；807 无消息体，本地匹配 ID 更新持久灯属性，远端仅对已存在人物入队，随原有人物队列消费，不挪动人物坐标/方向。移除远端人物即丢弃灯属性和排队更新；换图保留本地装备灯属性。死亡远端不发光，属性保存供复活使用。队列/死亡细节仍需与目标运行时进一步对照，本批无新增 IDA 证据。

## 地图原始消息 51/634/54（实现前记录，2026-11-14）

本批通过 IDA HTTP API 恢复分析库并实时核验；输入分析镜像 SHA256=08f68b794bfb28803d1132c02b26d2595dba309be5b2b475ae0cdfe6a752f8aa。
- 目标 0x4282C0：51/634 跳转公共队列分支 LABEL_876；54 六位解码至 this+96，限制 260 字节并加 NUL，随后返回，没有换图或对话清理。
- 0x42EC70 的 switch 将 51/634 同时分发到 0x42EE00；此分支在普通人物 ID 判断之前，不能套用远端人物存在性校验。
- 0x42EE00：record+6/+8 是落点；record+12 是编码地图文件名；0x42EF9D 读取 +10 低字节设置环境亮度。+11 高字节另外控制地图状态（1/2/3），不能混入亮度。复位本地站立，刷新视点与小地图并清临时列表。1500ms 字段已观察，但语义与输入锁需要继续核验，不采用早期 C++ 的 2300ms 常量替代。
- 两套早期 C++ GameProc 的 OnSvrMsgNewMap/SM_CHANGEMAP、SM_MAPDESCRIPTION 同样分文件名与显示名；ObjectPlayer::SendNewMap/SendMapName 分别发文件编号和显示文字。
- Zircon CConnection::Process(MapChanged) 使用 MapIndex/InstanceIndex，UserLocation 独立；不同协议不照搬。
- mir2x processrunnet.cpp 用本地主角 action 的 mapUID 变化触发 loadMap，清强制动作与阻塞状态；与目标公共地图消息不同。

选择：51/634 只解码并排队，在进程更新中调用既有安全切图入口，完成信号区分排队成功与资源加载成功；54 仅改 HUD 名称。保留亮度低字节与地图状态高字节，后者视觉效果仍待实现。非法编码/路径型编号拒绝；资源缺失或落点非法不破坏当前场景。本批不声称完成 TCP、所有公共消息顺序或初次登录角色构建。

## 换图左键延迟（实现前记录，2026-11-14）

优先采用本批原版实时 IDA 证据（分析镜像哈希见 map-wire.md）：0x42EE30 向 this+0x437BD8 写 1500，+0x437BDC 清零；0x4242C6..0x4242E6 累加帧时间，elapsed >= duration 时将两字段清零。此计时位于 0x42EC70 地图队列消费之前，因此新换图当帧不扣除此前时间。

0x4277D0 明确将 WM_LBUTTONDOWN=0x201 路由到 0x4264D0；0x42655A 在普通左键处理前检查 duration 非零则返回。前面的模态窗口处理不受该检查影响。右键、键盘与左键释放是其他处理函数，本批扫描未见它们读取该 duration；不扩大成全输入锁。普通左键路径包括地图移动、拾取、NPC 与后续界面点击。

参考：两套早期 C++ GameProc::RenderScene 同样累计并使用 >= 解锁，但换图写 2300ms；目标 1.45 的 1500 优先。Zircon MapControl 的 NextActionTime/ActionQueue 是动作限制，不能作为这个鼠标门禁的依据；mir2x ProcessRun::processEvent 先分发主 UI，再按角色状态处理地图事件，没有可替代目标该字段的证据。

选择：成功换图启动 1500ms 左键按下延迟，在公共地图队列前推进；失败不重启。保留模态确认、释放清理、键盘和右键的现有路径。已有直接寻路接口属于已解码/测试接口，不伪装成鼠标按下。淡入及其他写入该字段的场景（如 300/500/1000ms）仍需分别分析。

## 选项对话框缺项（实现前记录，2026-11-14）

本批实时 IDA：0x462000 初始化按钮，0x462850 每次绘制重定位，0x463500 区分本地选项与命令按钮，0x4644F0 读取 INI，0x4641F0 保存。优先使用绘制时坐标，不能使用构造时临时坐标。背景 GameInter1290；普通开关 1293/1294，攻击模式按钮 1296，关闭 1221。

0x462850 确认左列 2105..2114 位于 +(20,62+30*i)，开关 +(186,55+30*i)；右列除现有音量还包括 2115/2103/攻击模式2116..2120/2130，位于 y=59/239/269/300。空白不是原版故意预留。

0x463500：本地 ShowMon、AttackUser、MagicLight、BloodNum、UserPic 翻转并打印各自提示；ShadowBlend 翻转保存不打印提示。服务器项发送 CmList2123..2129，攻击模式发1422；点击依据已确认字段给出预测按钮帧，不直接改变已确认字段，不自行循环攻击模式。0x4644F0 默认 ShowMon/AttackUser/UserPic=1，ShadowBlend=0。

选择：补齐所有这些文字和控件、命令分流、服务器状态独立输入接口、本地持久化及提示；ShowMon 连接现有小地图标记，UserPic 连接目标自定义头像显示。强制攻击与完整影子渲染属于尚未完成的战斗/绘制模块，本批保存选项但不能声称这些业务效果完成。服务器原始状态包及聊天拒绝字段关系需继续核验，不能直接套用其他版本。

## 选项状态原包 1401（实现前记录，2026-11-14）

实时 IDA：0x4282C0 的 case1401 / 0x42D1DA..0x42D1E6 将 recognition 与第一 WORD 参数交给 0x464A00。掩码按窗口顺序为 40000000、80000000、08000000、04000000、02000000、01000000、00800000；最后一项是立即数（IDA 符号 dword_800000），不是读内存。模式直接保存 unsigned short。

原版只在某一确认位变化时修改该项显示，因此同值重复包不清掉预测。私聊/行会变化各翻转其拒绝变量并更新底栏图标；所有人聊天变化依旧值选择图标，再翻转 7678F4，并写入 7678F0（行会拒绝变量）：旧 all=0 时 guild=1，旧 all!=0 时 guild=0。反汇编 0x464CDF..0x464D5F 验证该交叉写入，不能把它当反编译错误擅自修正。依序先行会、后所有人聊天。

实现选择：原始 1401 接入已有分帧头解析，允许 recognition 最高位；只有变化项覆盖预测，严格保持聊天侧效应和执行顺序。已解码 apply_options_state 仍作为显式整体状态注入接口，不代替原版差量更新路径。完整网络会话与服务端联调仍未完成。

## 选项按钮状态（实现前记录，2026-11-14）

IDA 实时核验 0x41FAF0 绘制、0x41FC60 悬停、0x41FCA0 按下、0x41FCD0 释放、0x41FD70 换帧。ON 的普通/悬停底图/按下为1292/1293/1295，OFF 为1295/1294/1292；悬停先画底图再画普通图。旧客户端一直画1293/1294，实际是一直显示悬停。关闭按下1222，攻击方式按下1297。0x464010 按下遍历按钮，0x4640C0 移动调用全部按钮，state2不会被移动覆盖；0x463500 释放只测试落点，不要求此前按下。0x4641D0 清全部状态及滑块拖动，主界面无命中窗口的移动路径0x43CF40调用它。

本批补齐普通/悬停/按下帧、移动中按下态保留、窗口外清理与关闭/失焦清理，保留释放落点触发。原版窗口重叠分发和所有音量拖动边界仍需进一步同状态核验，不声称整个输入系统完成。

## 退出游戏与退出程序（原版分析，2026-11-14）

IDA 0x43C130 底栏3/4分别经0x421F60门禁后打开109/101；文字2636/1065。0x4256B0处理确认：101（case e）再次检查门禁，调用0x47BE10(1009)，调用0x43EC60，再写主对象+0x43843C=0、+0x438438=2；109（case m）再次检查，使用1077并发送WM_DESTROY。0x47BE10构造recognition与三个参数均0的消息头，无正文。当前Godot的101仅发exit_confirmed且默认处理为空，尚未发送此请求。

0x421F60汇编明确：GetTickCount无符号差值针对+0x36AE88/+0x36AE84/+0x36AE7C都必须严格大于8000；任何一项<=8000时，只有全局byte_7C1E98==19能放行。拒绝时输出CmList1407，调色板前景2、背景3。不可把阈值改为>=8000，不能只检查打开框而忽略确认时的二次检查。

三个字段对应本地人物子对象+0x628E0/+0x628DC/+0x628D4；0x415510初始化为0，写入点分别0x415D09、0x419017、0x417D2F。字段的准确动作语义、byte_7C1E98身份及退出状态2的消费路径仍需追踪。本批只建立独立退出窗口定向测试和证据文档，不用推测补齐网络和场景切换。

## 退出1009发送接入（实现前补充，2026-11-14）

本批实时复核0x47BE10：调用0x47D8B0以1009和五个0构造头，再经通用0x47C730发送；不存在用户名正文。0x43EC60调用CreateFile/WriteFile写三段本地缓存（143520、1200、18672字节），不是页面切换。将现有NPC发送编码提取为共享C++发送器，使1009与NPC请求使用同一1..9循环序号；NPC兼容信号保留，并增加通用client_packet_ready供会话层接入。当前仍没有实际TCP连接；门禁事件来源、缓存结构和退出状态消费者待继续补齐。

## 退出门禁接入（实现前记录，2026-11-14）

补证：0x4B9878+本地人物偏移3179944+动作偏移120=0x7C1E98，豁免字节是本地人物动作19（死亡动作）。0x415AD0处理31/1309后更新人物+0x628E0，0x415F90的动作11经过0x417C40更新+0x628D4；0x418D80指定技能编号分支更新+0x628DC。故不能用普通走路计时替代这些字段。

实现：独立ExitGuard保持三个原始DWORD计时字段及动作字节；所有差值按uint32回绕，严格>8000，动作19直接放行。主界面打开101/109及确认时再次检查，拒绝显示1407/调色板2,3。提供set_local_exit_state解码后字段接口，三个字段按628E0、628DC、628D4顺序，使用Windows GetTickCount同一时钟。当前本地战斗消息分发仍未完整实现，此接口不能标为真实动作同步完成。

## 退出过渡和目标页面（实现前分析，2026-11-14）

通过按o_displ实际位移搜索，找到0x423D50读438438/43843C；IDA格式化为loc_438437+1导致此前字符串搜索漏掉。状态2执行255−elapsed×0.1整屏RGB乘法，严格elapsed>2500后调用0x421E80清理游戏场景、切场景2并初始化0x485040，显示尺寸640×480。目标是角色选择页，读取Interface1c帧50及角色记录，不是账号登录框。0x492788模式6为SRC=ZERO、DEST=SRCCOLOR；不能用普通alpha近似。详细状态、资源、清理和待恢复页面分支见client/docs/evidence/exit-transition.md（服务端相对路径为../client/docs/evidence/exit-transition.md）。本节仅证据归档，运行时尚未接入。

## 退出过渡与角色选择初始页接入（实现前补证，2026-11-14）

0x484E10重置角色记录、内部状态0和+26B4=0；故初始页显示Interface1c50背景及52/54/56/58四个普通按钮，不显示两个GameInter附加按钮。0x485040末尾显示CmList200、类型2提示；0x420540类型2使用GameInter1248且禁用全部按钮，显式位置155,150。因此连接尚未完成时应保留无按钮连接提示，不允许底层角色按钮触发。该初始页可以先恢复，角色列表回包及创建/删除/选角分支另行核对。

本批按0x423D50/0x421E80接入2500ms整屏乘法过渡及离场，切换640×480初始角色选择页；提供会话关闭与角色选择连接通知，尚未接实际TCP。保留外部资源路径读取，不复制Interface1c资源。缓存文件结构和登录/角色列表协议不在本批声称完成。
