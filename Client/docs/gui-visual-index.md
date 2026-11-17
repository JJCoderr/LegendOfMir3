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
