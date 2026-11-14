# 客户端功能参考与实现选择

按当前 AGENTS.md 的文件名 RERENCES.md 维护。工程索引见 ../reference/ANALYSIS_INDEX.md；先前名称/发言/离场对照记录在 ../reference/HELP.md，历史模块后续逐项归并。

## 地图掉落物与拾取（实现前记录，2026-11-14）

- 原版既有分析：旧 mir3_1.45/gameplay.cpp 的 press_pointer / update_pointer 记录旧 Mir3.dat 0x67B26D..0x67B2B5：原地左键可发拾取，即使无客户端掉落物缓存；空闲、无路径/攻击等状态检查，持续左键 >0.3 秒后再请求。此证据来自旧二进制，目标 1.45 的完整地面绘制/文字混色尚未有本批新 IDA 证据。
- LegendOfMir3_Src：客户端 GameProcess/GameProc.cpp 的 SM_ITEMSHOW/SM_ITEMHIDE 使用 610/611，识别号去重、坐标和 looks 来自三个参数、名字六位解码；Ground.wil 由 Common/ImageHandler.cpp 映射。绘制按 48×32 格中心减图片半宽/高，忽略图片偏移，先影子后图；DropItemShow 仅鼠标所在格的首物品显示名称，中心 x+24、y+4，字顶 y-4，RGB(255,255,125)。Common/ClientSocket.cpp::SendPickUp 发 1001，recognition=0、当前 x/y、series=0。
- legendOfMir2_src：Mir3EIClientDev/Mir3EIClient/GameProcess/GameProc.cpp 的 610/611 和 DropItemShow 同源；Common/ImageHandler.cpp 同样 Ground.wil，库编号不同，不按编号移植。
- Zircon：Client/Models/ItemObject.cs 使用物品堆代表、库内 drawIndex、格中心绘制；MouseOver 返回 false，名称独立定位。Client/Scenes/GameScene.cs 拾取按键受 250ms 时间限制。其对象/物品堆与网络结构不同，不借用编号或冷却替代原版。
- mir2x：client/src/processrunnet.cpp::on_SM_GROUNDITEMIDLIST 按 mapUID 和格子整批替换物品 ID 列表，协议不同；不把其列表替换语义当成 610 单对象新增。

选择：以两套早期 C++ 的 610/611、Ground.wil、格中心和名称坐标作当前可追溯实现；以旧原版分析接原地左键拾取信号，保持服务器确认后才移除物品/加背包。保留多个物品同格，不将物品纳入移动阻挡。原版精确透明影子、悬停提亮与地图变更清理语义需进一步核验，不能据参考代码声称像素一致。
