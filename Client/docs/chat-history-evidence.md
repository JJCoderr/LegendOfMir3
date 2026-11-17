# 原生聊天历史与双模式显示

## 实现依据

- Godot br_godot_test_261115 的 client/src/chat/chat_history.cpp（ChatHistory::post），原版4366D0、43682E、436902、436991。
- 每条消息最多200 ANSI字节；宋体9pt按ANSI字节前缀测宽，普通341px、展开429px；中文高位字节使用原版奇偶回退规则，反引号用sscanf分行并消耗分隔符后的空白。普通最多5行，展开最多3行。
- 两种模式分别保存行历史及滚动位置。追加期间，展开位置在两个循环中均按差值14判断自动跟随；普通在普通循环按差值5判断。最后统一淘汰旧行，达到80即删除首行，所以每模式最多79行。
- CLIENT过去保留50条消息，每次消息到达及布局变化都重新换行并跳至底部。本批改为两套行缓存，ReflowChat仅把当前缓存投影到现有原生绘制链表。m_chatHistory作为最近消息记录供现有检查和查询使用，不参与重新换行。
- 模式切换保存各自位置；查看旧消息时新消息不会强制跳到底部。向下滚动可到最后一行，与原版滚动条范围一致。
- 沿用原生HUD、字体、颜色、按钮及点击音效；没有添加消息到达音效。

## 验证

- 客户端干净构建87/87；全目标构建111/111。
- 新增chat-history12项全部1：中文完整性、颜色保留、两种宽度、分隔符、200字节边界、容量、自动跟随、旧位置保留、双模式恢复。
- 回归guild-history11、progression14、scene-identity10全部1；总计47项。
- 预览205 chat-history、206 chat-history-expanded。截图out/chat-history-compact.png、out/chat-history-expanded.png均已查看，使用CLIENT原生界面。
- 当前验证为本地消息注入与原生渲染；真实游戏服收发、扬声器录音尚未验证。


## 角色聊天气泡（2026-11-17）

- 对照Godot hud/speech_bubble.hpp、chat/chat_history.cpp::wrap_speech、hud_view.cpp::draw_scene_speech；原版入口42F380，计时40FAC1。
- 普通说话40和怪物说话1501生成气泡，其他频道只写聊天。恢复1501消息分发以及NPC/怪物ShowMessage绘制；未知角色不创建实体或气泡。
- 原生宋体9pt、144px ANSI字节前缀测宽、最多8行，每条最多259字节；超限不替换当前气泡。ReceiveSpeech先完成分行再提交，空消息清空。
- 单行背景宽度取文字测量，中心偏移x+28，顶部y-73；多行背景x-44、宽144、底部y-55，高14*行数+4。最后一行居中。白字及右下1px深灰阴影。
- 背景沿用CLIENT DrawLegacyPanel，与Godot预乘矩形公式一致：RGB(80,60,40)、alpha120。取代旧重复D3D状态设置，支持已有RGB565剪裁。
- 时长严格大于3000ms后清空，等于3000仍可见；新消息计时归零。无新增音效，原版此分支没有播放调用。
- clean87/87、all109/109；speech14+chat-history12+guild-history11共37项全1。预览207 speech-single、208 speech-multiline、209 speech-eight；三张同名PNG位于out，均已查看。
- 截图使用固定屏幕坐标和实际消息分发；尚未验证真实游戏服、移动角色与地图遮挡的整体验证。
