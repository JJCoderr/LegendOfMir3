# 原生退出与返回选角完整批次

基线83ec303。以下缺口为实施前审查记录；本批已完成原生退出、过渡、清理及选角重连，统一构建、验证和提交。预览231..236，下一237。

## 实施前原生缺口

- Interface仅处理HUD按钮3（_BTN_ID_EXIT），用内部动作12显示2636；按钮4（_BTN_ID_LOGOUT）没有分派。GameProc::OnSysKeyDown只有Alt+Enter与F10，Alt+Q/X未接入。
- FinishNotice动作12在确认时才检查三个旧时间字段>10000，失败显示英文；成功直接SendMessage(WM_DESTROY)。打开时未检查，原生没有返回选角的1009与过渡。
- GameProc::SetNextProc是DeleteProc后加载登录页，不能复用为返回角色。DeleteProc清场景并关闭旧socket；OnDestroy只保存物品位置。
- CharacterProcess::Load(connect)会从g_szServerIP/g_nServerPort连接选角；收到SM_STARTPLAY又覆盖同一全局变量为游戏网关。返回选角必须保留前一个选角端点，不能连接当前游戏端点。
- CharacterProcess::Load清计数但没有显示CmList200连接等待框；真实列表由SM_QUERYCHR构建，不能把当前人物填成假列表。当前页有Load(false)离线加载入口。
- GameProc::RenderScene末尾在HUD和m_xNotice之后尚无全屏退出过渡。仅改变地图照明不足以符合原版。

## 原版与Godot证据

读取Godot br_godot_test_261115的docs/evidence/exit-flow.md、exit-guard.md、exit-transition.md；hud/exit_guard.hpp、hud/exit_transition.hpp；hud_view.cpp的check_exit_allowed、open/finish_group_dialog、handle_exit_confirmation、prepare/enter_character_selection和clear_game_scene；tests/hud_smoke.gd的exit-dialog/request/guard/transition用例。

- 43C130：按钮3为109、2636；按钮4为101、1065。快捷键Alt+Q/X分别对应109/101。通用确认框使用Tab/Enter/Space，取消不执行，Escape沿用原对话框阻断行为。
- 421F60：GetTickCount的DWORD差值，三个字段均严格>8000才放行；恰好8000拒绝，8001放行；动作19（死亡）豁免。打开与确认均检查，确认阶段拒绝后框已关闭。拒绝文字1407、前景调色板2、背景3。
- 原版三个字段为本地人物+628E0/+628DC/+628D4，初值0；写入来自415AD0处理31/1309、418D80的指定技能尾部、415F90动作11经417C40准入。普通移动不是这些写入来源。
- 101确认：共享发送器发1009，recognition/三个参数均0、无正文；保存缓存；状态2过渡。109确认：关闭程序。不能把1009当NPC请求或另建序号流。
- 423D50：每帧累加毫秒，<=2500仍绘制；亮度255-elapsed*float(0.1)，2500约为5，严格>2500才离场。覆盖800x600，SRCBLEND=ZERO/DESTBLEND=SRCCOLOR整屏乘法，位置在HUD和对话框之后。
- 421E80/4217E0：清场景、停止声音、关闭旧连接、恢复保存的选角地址，切640x480后初始化选角。485040使用Interface1c和GameInter，连接等待文字200/帧1248/坐标155,150/无按钮；连接成功再查询真实角色列表。

## 不能直接照抄的参考实现边界

Godot的exit_guard仅在set_local_exit_state注入以及clear_game_scene复位，搜索未发现战斗路径自动写入；原证据exit-guard.md也明确说明自动同步尚未完成。原生已有m_dwLastPKStruckTime（受击时还受名字颜色条件限制）、m_dwLastMagicTime（旧技能集合）、m_dwLastHitTime（旧近战间隔），不能不核对语义便全部当成原版字段。

Godot的SpellTiming::restricted_at是628DC的已命名模型，request_skill_cast通过SpellRoutes::restrict_followup尾部更新；它与ExitGuard::ticks目前独立。原生Actor::SetMagic仍为旧技能分支，后续应在完整技能编号证据支持下统一限制时间，保持GetTickCount同一时钟和DWORD回绕。

Godot退出过渡测试使用连接通知与注入状态，不证明真实TCP断开、选角网关重连或重新进入游戏。原生验证需把这些边界分开，不能用截图或空白角色页冒充完成登录流程。

## 本批实现与整批验证范围

1. 命名退出动作、提示、1009、8秒/2.5秒常量；统一HUD与Alt入口、打开/确认二次限制、取消与音效；保留原宋体/颜色/尺寸布局。
2. 明确并接入三类本地时间来源及死亡状态；初始化为0，测试每项8000/8001、回绕、确认期间新动作、死亡豁免，远端动作不得更新本地限制。
3. 缓存选角端点；在确认时保存物品布局并发1009；原生全屏乘法渐暗，严格2500/2501边界；离场只执行一次，清队列/旧HUD/持有物品/快捷状态并停音。
4. 复用原生CharacterProcess展示640x480初始等待页，阻止等待时旧游戏输入和旧socket事件；关闭游戏连接后连接保存的选角端点、重置发送序号，再按现有真实列表消息更新。检查旧socket事件不得误关闭新socket。
5. 同批验证实际编码请求、取消零发包、HUD两入口与Alt系统消息、二次拒绝、渐暗HUD/地图像素、清理和连接顺序。程序退出测试用独立子进程，不能终止工作环境其他进程。
6. 展示程序退出确认、返回选角确认、战斗拒绝、过渡及选角等待页截图。真实账户登录/创建/选角/删除/再入场仍按原目标后续服务器联调，不提前声称完成。


## 实现及验证结果

- ExitFlow统一101/109、字幕、1009与时间边界；HUD按钮和Alt+Q/X共用原生确认框，打开/确认双检查，鼠标确认/取消沿用105.wav。退出109经WM_DESTROY；101保存背包位置并通过公共发送器发送零字段无正文1009。
- 独立GetTickCount退出时钟接入本地CMyHero受击31/1309、CanNextHit准入和SetMagic指定技能尾部，不复用旧timeGetTime移动冷却；Mir3SkillIds保留Godot命名。完整技能发送路由仍未迁移，不能把此项等同完整战斗联机验收。
- RenderScene在HUD/公告后执行RGB565整屏乘法，严格超过2500ms调用ReturnToSelection一次；过渡阻止游戏输入，离场停止场景音效/音乐、清原始包及场景并销毁旧界面。随后恢复保存的选角端点、640x480和无按钮1248等待框。合法SM_QUERYCHR才解除等待，不构造当前人物假列表。
- 断开前取消WSA异步通知并清已排队事件，防止系统复用socket句柄时误处理旧事件；接收入口过滤非当前socket。
- 干净构建87/87、Client及六个服务器/网关目标109/109、最终增量构建通过。
- 退出30项：三时钟8000/8001、死亡19/非死亡18、DWORD回绕、2500/2501及一次性收尾、指定技能集、Alt入口、模态键盘、确认二次检查、资源、UDP实际1009及取消零发包、重复/旧页拒绝和无按钮输入。
- 返回7项：真实本机TCP重连恢复保存端口、清场景、等待页、陈旧socket忽略、公共发送器重置后查询角色、非法列表维持等待、合法空角色列表关闭等待。公共发送器连接时序号状态重置为1，发包前递增，因此首个线上查询序号为2；首轮测试错误期待1，按实际共享发送器规则修正后全部通过。
- 回归技能键盘34、交易54、装备25、界面音效15，共128项；加退出/返回37项为165项，全部通过。另运行select-three/create/delete页面，独立exit-quit进程确认后自然退出。
- 运行文件：out/mud3-01/Mir2Ex-exit-verified.exe。报告：out/exit-regression。截图：out/exit-program.png、exit-character.png、exit-blocked.png、exit-fade.png、exit-waiting.png。

本机TCP夹具验证客户端切页和协议边界，不代表真实账号认证、服务端角色状态保存、战斗退出限制及再次入场端到端完成。这些继续按总体目标在服务器联调阶段处理。
