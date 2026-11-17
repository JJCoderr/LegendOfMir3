# 大厅影片原生迁移

2026-11-17，基线5dec543。使用CLIENT的DirectDraw后缓冲及DirectSound，不引入Godot运行时。

依据Mir3 br_godot_test_261115的resources/legacy_movie.hpp/.cpp、tests/movie_checks.cpp和godot/hud_view.cpp的start_login_movie/advance_login_movie/open_configured_login/apply_selection_message。解码器直接沿用其独立VFW实现。

## 行为

- actor_resource_root是root/Data，wemade.dat开场；CreateChr.dat创建正放、取消或创建成功反放；StartGame.dat收到SM_STARTPLAY后开始。旧根目录ei_login.avi/ei_logo.avi不再使用。
- 反放首先显示0帧，第一个定时点取count-2，直到1帧，count-1个帧间隔后结束；PCM音轨仍正向播放。8位AVI无符号PCM直接交给DirectSound，不做Godot音轨转换。
- 影片以640×480呈现，RGB565后缓冲显示，音轨使用音效启用状态与音量。结束/关闭释放音频和影片；重复Start不重复创建缓冲。保留CreateChr.wav/StartGame.wav独立效果。
- 缺失或无法解码影片进入对应页面；输入在播放期间不可跳页或操作旧提示框。正常登录Load启动开场，已有offline登录预览保持静态可验收。重新Load选角重置背景状态。
- 移除旧F1/F2调试跳页；取消、创建成功及SM_STARTPLAY衔接到相应影片。真实服务器操作未在本批执行。

## 验收

Client清理构建90/90、全七目标113/113通过，最后复核后的4/4增量通过。日志out/lobby-movie-clean-final.log、out/lobby-movie-all.log、out/lobby-movie-final.log。首次clean暴露Create误加override，已删除并重新clean通过。

影片33、登录27、转场12、分组33、音效15、连接20、退出30、返回选角7，共177项检查通过。影片检查覆盖正反时序、色序和上下翻转、真实DirectSound播放/音量/静音/释放、创建取消/成功、入场消息、返回页面和重载。最终相关影片与退出回归重跑通过。

预览268..271：movie-forward、movie-reverse、movie-opening-return、movie-creation-return；下一272。运行程序out/mud3-01/Mir2Ex-movie-review.exe来自build-ninja-sqlite/Mir2Ex.exe。生成的movie-preview-fixture.avi只在out运行目录，不写外部资源Data链接。所有本批预览进程已停止。

截图out/movie-{forward,reverse,opening-return,creation-return}.png均640×480，已目视检查。forward/reverse是明确标注SYNTHETIC AVI / PCM TEST的合成4帧影片，100ms顶部RGB分别82/0/0与123/0/0，底部蓝色；原始80/120红值经过RGB565量化。opening-return和creation-return是实际原生页面。

## 限制与后续

当前外部资源递归查找没有wemade.dat/CreateChr.dat/StartGame.dat或相同名字AVI，因此原始影片画面/编解码器兼容性与原片音轨尚不能实测；本批验证的是参考帧序、合成解码和缺失文件回退，未声称原版逐像素一致。未修改资源或真实网关数据。

下一批继续审计角色创建/删除/选择的消息文案和等待提示：当前SM_NEWCHR_FAIL、SM_STARTFAIL仍含旧英文文本，需按CmList及Godot参考迁移。真实登录流程留待GUI完整后联调。
