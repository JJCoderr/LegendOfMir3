# GUI 完整性核查清单

基线 bf93924 后的源码盘点。目标不变：CLIENT 自身控件，先与 Mir3/br_godot_test_261115 已完成页面一致，以 MIR3.EXE 为最终基准，包含字体、色彩、大小、交互和音效；逐页截图；GUI 后再做真实登录及服务器接入。

这里的“入口存在”仅表示当前代码可定位，不是全部状态已验证。AI2AI及早期证据包含历史待办，必须以当前实现核对，不能直接累计为未完成项或完成项。

| 页面/功能族 | 当前原生入口 | 可追溯证据/后续核查 |
|---|---|---|
| 服务器分组、账号、公告、服务器列表、计费提示 | ServerSelectProcess、LoginProcess | server-groups/login-gui/login-transition-migration-audit；需整体视觉基准 |
| 开场、创建、入场影片 | AVI/NativeMovie、CharSelectProcess/BackGround | lobby-movie-migration-audit；原版三个影片文件缺失，只有合成解码验证 |
| 选角、四槽分页、等待、创建六外观、删除和错误提示 | CharacterProcess、SelectChr、Common/LoginNotice | selection-roster/selection-dialog/character-interaction/role-presentation-migration-audit |
| 主HUD、聊天展开/收起、频道、拒收、输入、快捷物品 | GameProcess/Interface、ChatPopWnd、QuickWnd | hud-quick-entry、chat-history、keyboard-routing；需按当前总入口核对覆盖 |
| 人物、装备、背包、物品说明、数量输入 | StatusWnd、InventoryWnd、Common控件 | equipment-transaction、item-use、amount-dialog；历史预览不能代替新整体截图 |
| 技能树、列表、说明、绑定菜单、顶部快捷栏 | SkillWnd、Interface、GameProc | skill-wire、keyboard-routing、skill-casting |
| 组队、邀请、成员操作、行会、公告和职务编辑 | GroupWnd、GroupPopWnd、GuildWnd、GuildMasterWnd | social-wire；仍需整理所有弹窗到可视清单 |
| NPC对话、商店、购买/出售/修理、仓库 | NPCWnd、StoreWnd | merchant-service-reply、storage-withdraw-reply、物品事务证据 |
| 双方交易、金币和物品、确认/取消/结果 | ExchangeWnd | trade-transaction-audit |
| 邮件列表/正文/写信/附件/金额、寄售查询/登记 | MailWnd、ConsignmentWnd | mail-wire、consignment-wire、amount-dialog |
| 选项、音量、行为开关、骑乘 | OptionWnd、HorseWnd | interface-audio及AI2AI既有批次；需补统一状态清单 |
| 任务目录/正文/滚动 | QuestWnd | quest-wire |
| 自定义头像、文件选择、目标头像 | PortraitWnd、Interface、头像缓存实现 | AI2AI头像批次；需复核空列表/加载失败/滚动视觉 |
| 小地图、大地图、标记、攻城控制/装填 | MiniMap、SiegeWnd | minimap-scene、window18；旧文档“人物怪物未完成”需与现有Show中actors过滤重核 |
| 游戏退出、返回角色、冷却/等待/失败 | GameProc、ExitFlow、CharacterProcess | exit-flow、connection-retry |
| 地面物品、名称、气泡、血条、场景光照和天气 | GameProc、Actor及场景绘制 | ground-items、scene-lighting、技能/场景证据；动态效果应按时序验证 |

## 尚未成立的完成条件

1. 已建立52个主要页面/弹窗的当前版本视觉索引（gui-visual-index.md）；仍未覆盖全部错误、禁用、滚动和拖动中间态，不能据此断言GUI全完成。
2. 字体、颜色、尺寸须逐页对照参考；本轮角色动画审计发现旧总时间跳帧、语音时机与悬停背景差异，已在同批修正，证明旧截图与通过测试不足以作为终验。
3. 三部原版影片仍缺资源，合成片不能代替原片验收。
4. 真实登录/创建/删除/进游戏兼容协议与完整操作尚需后续集中联调；已有旧回环/服务器证据不代表当前二进制端到端通过。

下一批基于52图索引补齐错误、禁用、滚动与拖动状态，检查字体、输入和声音，再处理真实协议链路。目标保持 active。


大厅38页新增统一视觉索引，见gui-visual-index.md；基线4786674，区别于游戏52页旧基线。38页/7拼图已查看，127离线检查通过。创建姓名框空白状态虽与源码位置相符，仍需补填入内容后的视觉证据；全部GUI与真实流程完成条件仍未成立。

姓名输入补验已完成一批：out/creation-name三张填入内容截图已查看，并修复创建页EN_CHANGE遗漏导致的中文超字节限制；12项新检查及60项相关回归通过。见character-interaction-migration-audit.md。


## 2026-11-17 窗口移动与输入框布局

原GameProc.OnMove把共用编辑框强制放回旧聊天位置，Login/Character.OnWindowMove直接隐藏编辑框。改为ChatEdit.Relocate：保留当前编辑框相对客户区的逻辑位置/尺寸，更新窗口客户区后重新定位，不改变文本、焦点或可见性。初建WM_MOVE早于EDIT创建时直接返回。没有新增类数据成员。

Common/InputMovePreview.h复用实际SetWindowPos移动/恢复主窗的检查：比较前后编辑框相对位置、尺寸、文本、焦点、可见性。覆盖登录、创建、删除、普通聊天、展开聊天、数量弹窗，以及隐藏编辑框不意外显示。新318 input-window-moved，下一319。

构建七目标通过out/input-window-move-final.log。登录35+创建21+游戏移动4+键盘34+模态27=121项通过。最后只补初建空句柄处理并重跑登录/游戏移动，其他正常编辑框回归对应相同路径。6页新鲜截图out/input-window-move，数量弹窗图已查看。所有自启预览停止。

此批验证窗口平移；不据此声称任意DPI切换、最小化恢复或全屏切换已验收。影片/公告资源匹配问题仍等待额外原版目录信息。
