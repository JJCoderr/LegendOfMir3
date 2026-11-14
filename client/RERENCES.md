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
