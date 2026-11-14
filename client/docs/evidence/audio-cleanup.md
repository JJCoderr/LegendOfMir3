# Godot声音资源退出清理

本批修复Godot适配层的资源生命周期，不改变原版声音触发、编号、音量或时序。原版音效依据复用scene-human-life、map-events及声音位置相关专题。

2026-11-14，forced_motion.gd的verbose退出日志明确列出AudioStreamWAV、AudioStreamPlaybackWAV各1个引用，以及Mir3Footsteps实例专用总线StringName残留。此前只看到2个ObjectDB提示且偶发不能复现，本次已获取具体类型和所属总线。

现有_exit_tree对背景/界面声音执行stop及set_stream空引用，但脚步分支只stop就移除总线；remove_actor_sound同样仅stop后queue_free。统一退出顺序：

```text
停止播放器 -> 解除播放器的音频流引用 -> 转回Master总线
移除该实例专属总线 -> 场景树/延迟队列销毁播放器节点
```

独立audio_cleanup.gd覆盖播放脚步及挥击过程中释放HUD，用WeakRef检查节点、音频流和播放实例全部失效，并检查AudioServer总线数量恢复。该测试直接建立场景并触发相关声音，不走登录或整图场景。

验收：Release构建成功；audio_cleanup.gd输出AUDIO_CLEANUP_UI_OK；原复现用例forced_motion.gd输出FORCED_MOTION_UI_OK。两份verbose日志均无ObjectDB泄漏或孤立StringName提示。驱动初始化仍打印NVAPI_EXECUTABLE_ALREADY_IN_USE，未影响用例通过。截图：client/build/screenshots/audio-cleanup-active.png。未执行全场景测试。
