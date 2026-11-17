# 当前 GUI 整批截图索引

源码 43d7c75bebbb3a01d7643a48a24accc68451b644，二进制 SHA256 3E5E2E2323A2012A1185DC60C1232A36188A358EBBE6BC20437BA34363F3E264。

输出 out/gui-review-current/index.html：52张原生800×600截图、9张原尺寸拼图、manifest.json（时间/源码/二进制）及本批reports。每张BMP必须在对应预览启动后更新，不用旧截图充数。所有预览进程均已停止，未连接真实网关。

## 页面覆盖

1. `hud-expanded` → `out/gui-review-current/hud-expanded.png`
2. `inventory-panel` → `out/gui-review-current/inventory-panel.png`
3. `status-panel` → `out/gui-review-current/status-panel.png`
4. `status-elements` → `out/gui-review-current/status-elements.png`
5. `skills-panel` → `out/gui-review-current/skills-panel.png`
6. `quick-items` → `out/gui-review-current/quick-items.png`
7. `skill-key-menu` → `out/gui-review-current/skill-key-menu.png`
8. `skill-description` → `out/gui-review-current/skill-description.png`
9. `group-panel` → `out/gui-review-current/group-panel.png`
10. `group-create` → `out/gui-review-current/group-create.png`
11. `group-invite` → `out/gui-review-current/group-invite.png`
12. `group-remove` → `out/gui-review-current/group-remove.png`
13. `guild-overview` → `out/gui-review-current/guild-overview.png`
14. `guild-members` → `out/gui-review-current/guild-members.png`
15. `guild-notice` → `out/gui-review-current/guild-notice.png`
16. `guild-denied` → `out/gui-review-current/guild-denied.png`
17. `guild-notice-editor` → `out/gui-review-current/guild-notice-editor.png`
18. `guild-members-editor` → `out/gui-review-current/guild-members-editor.png`
19. `options-interaction` → `out/gui-review-current/options-interaction.png`
20. `mount-panel` → `out/gui-review-current/mount-panel.png`
21. `quest-empty` → `out/gui-review-current/quest-empty.png`
22. `quest-list` → `out/gui-review-current/quest-list.png`
23. `trade-empty` → `out/gui-review-current/trade-empty.png`
24. `trade-list` → `out/gui-review-current/trade-list.png`
25. `trade-confirmed` → `out/gui-review-current/trade-confirmed.png`
26. `trade-gold` → `out/gui-review-current/trade-gold.png`
27. `npc-dialogue` → `out/gui-review-current/npc-dialogue.png`
28. `npc-input` → `out/gui-review-current/npc-input.png`
29. `npc-password` → `out/gui-review-current/npc-password.png`
30. `shop-goods` → `out/gui-review-current/shop-goods.png`
31. `shop-offers` → `out/gui-review-current/shop-offers.png`
32. `shop-quantity` → `out/gui-review-current/shop-quantity.png`
33. `storage-list` → `out/gui-review-current/storage-list.png`
34. `storage-quantity` → `out/gui-review-current/storage-quantity.png`
35. `merchant-repair` → `out/gui-review-current/merchant-repair.png`
36. `merchant-sell` → `out/gui-review-current/merchant-sell.png`
37. `mail-inbox` → `out/gui-review-current/mail-inbox.png`
38. `mail-read` → `out/gui-review-current/mail-read.png`
39. `mail-compose` → `out/gui-review-current/mail-compose.png`
40. `mail-gift` → `out/gui-review-current/mail-gift.png`
41. `consignment-empty` → `out/gui-review-current/consignment-empty.png`
42. `consignment-list` → `out/gui-review-current/consignment-list.png`
43. `consignment-registration` → `out/gui-review-current/consignment-registration.png`
44. `consignment-confirm` → `out/gui-review-current/consignment-confirm.png`
45. `portrait-empty` → `out/gui-review-current/portrait-empty.png`
46. `portrait-preview` → `out/gui-review-current/portrait-preview.png`
47. `siege` → `out/gui-review-current/siege.png`
48. `siege-load` → `out/gui-review-current/siege-load.png`
49. `exit-program` → `out/gui-review-current/exit-program.png`
50. `exit-character` → `out/gui-review-current/exit-character.png`
51. `minimap-small` → `out/gui-review-current/minimap-small.png`
52. `minimap-large` → `out/gui-review-current/minimap-large.png`

## 核查结果与边界

全部52图已通过9张原尺寸拼图逐张查看：背景皮肤、主要控件、中文文本、编辑器、弹窗及底层面板叠放均呈现。组队/行会绘制与参考draw_group/draw_group_dialog/draw_guild的资源、坐标、颜色核对，未在这些静态状态发现需修正差异。没有生产运行时代码变更，不重复编译；采用前批七目标构建成功的当前产物并核验哈希。

任务长说明覆盖列表、HUD提示消息、重复邮件正文、头像彩色方格均为预览夹具状态。inventory-layout-check的moved/placement_green/placement_red=0属于静态inventory-panel未执行摆放，不作为失败，也不盲目累计全部报告通过数。

参考分支未跟踪PNG/BMP/JPG基准图。本轮查漏不等于与MIR3.EXE同输入同帧像素比对通过。52图仍未覆盖所有错误/禁用/滚动/拖动中间态。原版影片缺失与真实服务器联调边界不变。

## 重现

仓库根目录运行 `./Client/Tools/review_gui.ps1`，再运行 `python Client/Tools/gui_gallery.py out/gui-review-current`。依赖现有运行资源、已构建CLIENT和Pillow。HTML可按预览名筛选并点击原图；已验证52图文件和链接存在，未声称浏览器交互已验收。

下一批补齐错误/禁用/滚动/拖动状态，检查字体、输入、声音。下一新预览ID仍301。


## 大厅页面整批索引（2026-11-17）

源码 4786674d8b8d9a5dc910b26b77bcb11562de5208，二进制SHA256 7A793E0016CD1F59EA04A6EDDAFC3DD344FA5C76F0E92C1BF54C194C68B9DEB7。输出out/gui-review-lobby/index.html，38张当前截图、7张原尺寸拼图。全部图片逐批查看，无整页缺失或弹窗文字不可见现象；不代表逐像素对齐验收。

运行方式：`Client/Tools/review_gui.ps1 -Suite lobby -Output out/gui-review-lobby`，然后 `python Client/Tools/gui_gallery.py out/gui-review-lobby`。默认game套件仍为原52页。

服务器分组7、登录6、角色列表5、六外观及选角按钮8、选角/创建错误及删除确认12，共38页。分组33、登录27、角色表现22、角色列表22、选角对话框23，共127项检查通过。预览进程全部停止。

创建姓名框初始为空，OpenCreation与参考均为(288,405)、75×13；空白截图不能代替输入焦点与输入内容的完整验收。影片资源缺失问题仍在；真实登录链路未验证。

- `server-groups` → `out/gui-review-lobby/server-groups.png`
- `server-group-hover` → `out/gui-review-lobby/server-group-hover.png`
- `server-group-selected` → `out/gui-review-lobby/server-group-selected.png`
- `server-group-row` → `out/gui-review-lobby/server-group-row.png`
- `server-group-scroll` → `out/gui-review-lobby/server-group-scroll.png`
- `server-group-exit` → `out/gui-review-lobby/server-group-exit.png`
- `server-group-login` → `out/gui-review-lobby/server-group-login.png`
- `login` → `out/gui-review-lobby/login.png`
- `login-hover` → `out/gui-review-lobby/login-hover.png`
- `login-pressed` → `out/gui-review-lobby/login-pressed.png`
- `login-error` → `out/gui-review-lobby/login-error.png`
- `login-servers` → `out/gui-review-lobby/login-servers.png`
- `login-billing` → `out/gui-review-lobby/login-billing.png`
- `selection-roster-first` → `out/gui-review-lobby/selection-roster-first.png`
- `selection-roster-waiting` → `out/gui-review-lobby/selection-roster-waiting.png`
- `selection-roster-second` → `out/gui-review-lobby/selection-roster-second.png`
- `selection-roster-long-name` → `out/gui-review-lobby/selection-roster-long-name.png`
- `selection-roster-empty` → `out/gui-review-lobby/selection-roster-empty.png`
- `role-male-warrior` → `out/gui-review-lobby/role-male-warrior.png`
- `role-male-mage` → `out/gui-review-lobby/role-male-mage.png`
- `role-male-tao` → `out/gui-review-lobby/role-male-tao.png`
- `role-female-warrior` → `out/gui-review-lobby/role-female-warrior.png`
- `role-female-mage` → `out/gui-review-lobby/role-female-mage.png`
- `role-female-tao` → `out/gui-review-lobby/role-female-tao.png`
- `role-selection-hover` → `out/gui-review-lobby/role-selection-hover.png`
- `role-selection-pressed` → `out/gui-review-lobby/role-selection-pressed.png`
- `character-name-exists` → `out/gui-review-lobby/character-name-exists.png`
- `character-full` → `out/gui-review-lobby/character-full.png`
- `character-create-failed` → `out/gui-review-lobby/character-create-failed.png`
- `character-name-invalid` → `out/gui-review-lobby/character-name-invalid.png`
- `character-query-failed` → `out/gui-review-lobby/character-query-failed.png`
- `character-start-failed` → `out/gui-review-lobby/character-start-failed.png`
- `character-disconnected` → `out/gui-review-lobby/character-disconnected.png`
- `character-delete-confirm` → `out/gui-review-lobby/character-delete-confirm.png`
- `character-delete-missing` → `out/gui-review-lobby/character-delete-missing.png`
- `character-start-missing` → `out/gui-review-lobby/character-start-missing.png`
- `character-name-long` → `out/gui-review-lobby/character-name-long.png`
- `character-empty` → `out/gui-review-lobby/character-empty.png`


## 2026-11-17 同版本整批截图与资源口径

用户确认没有其他原版目录，后续以当前原版资源为准。无需继续等待额外目录；现有资源包仍可按证据核查。未找到的三段影片保留缺失记录，不使用同名MP3代替。

review_gui.ps1新增all套件：52个游戏内状态、38个大厅状态和20个输入/弹窗边界状态。完整运行110/110，生成out/gui-review-integrated/index.html、110张原尺寸PNG、19张联系表及42份本次新鲜报告。manifest记录生产版本724bc52b58f113516f52214fe45418f448bd6b5b和二进制SHA256 1802597130EF08E8226A968C0B198DAF912CE62044604638310ABBB7569DE636。

已核对报告；inventory-layout静态模式的moved/placement_green/placement_red为0沿用既有夹具含义，不累计成通过数。其余报告中的断言均为1，样本数量/字体/尺寸等为数据字段。检查110个唯一模式及PNG齐全；抽查中文创建原图、返回列表联系表19，未声称本轮逐张视觉验收全部110图。旧批次视觉检查记录保留。

本批只更改采集工具和文档，沿用已干净构建的客户端，不重复编译；没有新增生产功能。截图套件不是全部交互测试，音效沿用上一批15项验证，真实服务器流程、原版影片、公告长文本资源适配及跨显示器DPI/全屏仍未完整验收。


## 2026-11-17 整批截图视觉复核结果

基于同一724bc52二进制，已浏览联系表01–18，联系表19在上一批已查看；覆盖manifest中全部110个状态。07查看原尺寸联系表，其余01–18以1000px宽缩略图检查整体布局；创建中文输入另有原尺寸查看。逐状态记录保存在out/gui-review-integrated/visual-review.json，可按模式定位联系表。

- 108个状态未观察到新的明显布局异常；这是联系表层面的检查，不证明字体像素、颜色数值、命中区域或真实协议正确。
- server-notice和server-notice-moved两图仍存在长行文字越出背景右框，与server-notice-evidence.md记载一致，不计为视觉通过。
- 任务页长说明覆盖列表、聊天末行换行、邮件重复正文、头像彩色方格及编辑恢复页保留背包是既有夹具内容；本次未改为更好看的假截图。
- 不从图片推断音效、影片、缩放/DPI、拖动或服务器操作通过。52/38旧基线保留来源，但当前静态布局浏览证据以本批为准。

下一步需解决公告在当前资源上的排版验收依据，并针对具体页面做参考坐标/字体核对；不以110图齐全替代最终GUI验收。用户已明确无额外原版目录，不再等待该问题的用户回复。


## 联系人/快捷短语新增四页

详见chat-shortcuts-migration-audit.md。截图位于out/chat-shortcuts：chat-contacts.png、chat-phrases.png、chat-contacts-expanded.png、chat-phrases-expanded.png。普通/展开布局及选中行已查看。此四页是110图基线以外的新增原生页面，不将旧manifest改为包含尚未重跑的页面。


## 聊天回显与调色板两页

新增out/chat-commands/chat-private-echo.png及chat-palette.png，均查看；原生私聊淡蓝字黑底与八色列表。见chat-commands-migration-audit.md，未改写此前110图manifest。


## 添加行会成员三状态

out/guild-add/guild-add-member.png、guild-add-denied.png、guild-add-member-filled.png均查看，覆盖会长空输入、非会长提示和中文角色名。详见guild-add-dialog-migration-audit.md。


## 坐骑延迟交互截图

out/input-delay/mount-input-delay.png、mount-delay-dialog.png：坐骑命令后的面板、延迟期间仍可操作的通用输入弹窗。计时边界以input-delay-migration-audit.md及报告验证，静态截图不代替时间测试。


## 金币和堆叠丢弃金额/数量框

out/gold-drop的gold-held、gold-drop-dialog、gold-drop-filled，以及out/item-drop的item-drop-dialog、item-drop-filled均已有实际运行PNG并查看。后者包含弹窗上方手持图标。参见gold-drop-dialog-migration-audit.md与item-drop-dialog-migration-audit.md；没有改写旧110图基线。
