# 角色动画、语音与按钮呈现

参考 Mir3/br_godot_test_261115 的 character_selection.hpp SelectionRole、create_character.hpp CreateCharacterPage，以及 hud_view.cpp 6749..6780 的推进/命中矩形/声音时机。原生实现仍使用 DirectDraw 和 DirectSound。

## 修正

新增共享 RoleAnimation，选角与创建页使用同一30组帧数表。累计16位时间严格大于120ms才推进一次，丢弃余量；大帧间隔不跳多帧。选中/取消动作分别转为选中循环/待机。创建页两种性别分别计时，未选角色身体保持首帧，但阴影使用其持续推进的动画帧。切换性别只重置新选角色；切换职业或重开页面重置两者。

创建语音在选中角色每轮首帧推进时播放；选角语音只在选择动作首帧推进时播放。移除点击时直接发声，首次进入和翻页的选择动画也能触发语音。命中矩形在推进时取下一资源帧，在动作切换和静态重绘时保持，匹配参考的更新顺序。

选角按钮在角色绘制之后绘制，增加参考的半透明棕色悬停背景和边框；拖动离开仍保留按下图，悬停矩形按当前鼠标位置更新。创建按钮点击声保持上一批已接入的行为。

## 验收

clean90/90、七目标109/109通过，日志 out/role-presentation-{clean,all}.log。运行 out/mud3-01/Mir2Ex-role-review.exe，复制自 build-ninja-sqlite/Mir2Ex.exe。

新增22项检查：120/121ms、单次大间隔、16位回绕、选中/取消转态、六种创建循环/语音触发、两个角色时钟、性别和职业重置范围、矩形更新、悬停/拖离状态。加交互15、列表22、提示23、影片33、音效15，共130项全部通过。原生音效设备/加载/音量开关回归通过，不把程序检查等同于人工听音。

预览292..299：role-{male,female}-{warrior,mage,tao}，role-selection-hover/pressed；下一300。截图 out/role-*.png，六状态实图拼接 out/role-creation-six.png，均已目视核查。首个战士截图还显示鼠标所在职业按钮提示，是实际控件输出。所有预览进程已停止，未连接真实服务器。

整体范围见 gui-completion-audit.md；本批不宣称所有页面或原版影片已完成验收。
