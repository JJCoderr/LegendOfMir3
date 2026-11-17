# 物品使用链路迁移审查与实现

审查基线 a257359。以下断点是修改前的状态；本批已迁移本地客户端输入、请求及回包链路，验证记录见文末。

## 已确认的断点

1. Interface::OnKeyUp 仍调用 InventoryWnd::OnKeyUp，后者读取 m_stBeltItem；新 CQuickWnd::slots 是独立的六格引用，因此新栏位绑定无法驱动这条数字键使用路径。
2. Interface::OnLButtonDoubleClick 未分派快捷栏，仅分派背包等窗口。QuickWnd 目前仅实现拿起、绑定和交换。
3. GameProc 的 SM_EAT_OK 调用旧 AddNewBeltItem 后清空公共持有；SM_EAT_FAIL 根据 bIsBeltItem 返回旧 m_stBeltItem。没有按新 heldSlot 还原引用，也没有处理堆叠模式 3 的服务器剩余数量。
4. ClientSocket::SendItemIndex 将 wParam/wTag 均置零；Godot 使用请求将 GetTickCount 同义的毫秒 tick 分成低/高 WORD 传入 CM_EAT。
5. 背包旧双击路径未实现 Godot release_action 的 300ms 释放间隔和药物 2000ms/特殊图标 1000ms 共用冷却。
6. GameProc::OnKeyUp 没有服务器公告模态过滤，旧物品数字键路径可能在公告打开时执行；迁移按键到 OnKeyDown 后应一起覆盖公告、通用对话框和行会编辑模态。

## 参考实现和行为

读取 Godot br_godot_test_261115：client/src/inventory/quick_items.hpp、inventory_grid.hpp；client/src/godot/hud_view.cpp 的 activate_quick（7421）、quick_pointer、数字键分派（7685）和 apply_inventory_use_reply（7897 附近）。这些是迁移参考，不需要 Godot 运行时。

- 数字 1~6 在按下时使用；Shift+1/2 保留聊天输入行为。Z 保留现有开关。
- 药物 std_mode 0/3 的间隔严格大于 2000ms；mode 0 且 icon 20/21/70 时可严格大于 1000ms；非药物不受该冷却。时间使用 DWORD 差值。
- 快捷栏引用不拥有背包物品；使用前取出引用到公共持有，不能删除背包实体或把引用重复插入背包。
- 交易提交/取回和装备待确认时不发使用请求。请求中设置 pending，禁止重复发送。
- 数字键指向空槽且手持可绑定物品时，先移除同名旧引用再执行绑定；提示 1067/1068/1069 与使用失败提示 1051/1066 分别输出。
- CM_EAT 带 instance、名称、tick 低 WORD/高 WORD。
- 回包必须有待用物品，重复回包不重复提示/音效。快捷来源失败恢复原引用；原位占用时寻找空位，无空位不丢失待用状态。
- stack_mode 3 且原数量>1时，采用服务器剩余量更新同 instance 背包副本及持有，再恢复引用；原数量<=1时删除匹配 instance+name 的实体和同名引用。不能本地直接减一。
- 成功提示 1464，快捷来源另有 1467/1468（剩余量）；模式0使用108音效，模式1/2使用107。正常背包来源的剩余量同步也要更新快捷引用。

## 本批实现范围

统一原生物品使用状态和冷却，接入数字键、快捷栏鼠标、背包释放/双击、实际 CM_EAT 编码及 OK/FAIL 回包；移除本链路对旧 belt 数据的依赖。本批按此范围集中实现、构建和测试。

验证必须覆盖：六格绑定后按键实际请求、隐藏栏按键、Shift、冷却边界与时间回绕、重复按键/回包、失败恢复、堆叠剩余量与末件、交易/装备/模态阻断、背包与快捷栏共享冷却、请求 tick 字段以及声音。截图至少展示绑定、等待/恢复及更新数量后的原生界面。现有 quick-panel/binding/persistence 和 item-use-confirmed 仅覆盖部分行为，不能作为上述端到端链路完成证明。

## 实现与验证

CInventoryWnd 统一维护 lastUse、lastRelease 和 pending 状态，RequestUse 编码 CM_EAT 的 instance/name/tick；ApplyUseReply 区分背包实体与快捷引用，处理失败恢复、模式 3 剩余量、末件和重复回包。QuickWnd::Activate 迁移数字键/鼠标激活，保留绑定及相关提示；Interface 将数字键迁移到按下，抬起不再使用旧 belt。背包通过第二次释放的时间间隔决定使用，Windows 双击通知不再额外发包。失败模式 51 打开原提示 1414。资源和冷却常量集中于 ItemUseIds.h。

新预览 item-use-chain / item-use-restored（213/214）以实际 ClientSocket UDP 回环捕获请求，再通过 GameProc 的编码消息入口投递回包，验证 26 项：报文字段、待确认重复请求、数量同步、重复回包、冷却边界、失败、末件、DWORD 回绕、快速药物边界、真实按键/抬起/重复键、交易与装备阻断、鼠标激活、背包请求/回复/共享冷却/失败、快捷栏全满保留及空位恢复、模态、Shift、背包真实释放、使用成功后的 DirectSound 播放状态。

既有 quick-empty、quick-binding、item-use-confirmed、interface-audio 覆盖快捷栏界面 16 项、绑定 12 项、持久化 7 项、旧使用提示 5 项、音效 15 项。新旧合计 81 个布尔检查；音效用例另覆盖 12 个角色语音。首次新增音效断言误将 HRESULT 的 S_OK(0) 当作 false，修正为 SUCCEEDED 后重新验证。清理构建 87/87 与七目标构建通过；最终增量构建用于断言修正。

截图：out/item-use-chain.png 展示成功后背包/快捷栏均为 3；out/item-use-restored.png 展示失败后均恢复 8。本地截图和 UDP 回环不能证明真实服务器物品消费、数据库扣减、掉线重连或整套在线流程完成；这些仍属于后续服务器接入。
