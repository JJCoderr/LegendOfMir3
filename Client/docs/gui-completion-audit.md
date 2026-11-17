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
