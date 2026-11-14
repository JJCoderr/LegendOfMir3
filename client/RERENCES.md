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
