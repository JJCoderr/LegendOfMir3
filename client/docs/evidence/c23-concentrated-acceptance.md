# C23 集中验收记录（2026-11-14）

通过 Python 总入口启动 `--suite skills --screenshots`，使用 Windows Godot 4.7.2 Compatibility 渲染与原 WIL 资源。没有处理 ASPack，也没有修改服务端。

首次全集日志：`client/build/c23-full-acceptance.log`。模型、绑定、施法（含实际八秒超时）、七系附魔、七系像素可见性、强制移动、施法纠偏、范围均通过。共用路径的 68 个技能逐项通过，但批次最后换图清理断言失败，因此首次全集退出码为 1，不能记作全集成功。

## 发现与修复

只运行 `--case common --skills 36,123` 即复现。换图后剩余 `scene_actor_sounds[0]` 为 sample=142、count=1、playing=false；不是正在播放的法术效果，而是本地动作声音记录未释放。证据：`client/build/c23-exit-repro.log`。

`change_map` 已清除法术声音，并在移除远端人物时释放各自声音；本地人物不走移除流程。补上 `remove_actor_sound(0)`，同时为共用测试的最后换图添加返回值断言及失败声音诊断。Release 构建成功，同一双技能复现用例修复后通过，记录为 `client/build/c23-exit-fixed.log`。

## 未执行部分续跑

随后运行 `--case passive --case special --case impact --case target-loss --case stationary --case melee --case sound --screenshots`，退出码 0，日志 `client/build/c23-full-remaining.log`：

- 被动 3/4、特殊 5 项通过。
- 命中生命周期 28 项、目标丢失 8 项、定点生命周期 35 项通过。
- 近战 7 项、声道模型及图形声音专项通过。

此前通过且未受影响的技能未重复运行。此次证据由首次运行、失败场景定向复测、剩余子集续跑组成；没有产生修复后一次无中断的 `TEST_SUITE_ALL_OK`，最终全集门槛仍保留。原版同状态视觉/声音对照以及 47/48 运行时资源改写核对也仍未完成。

截图位于 `client/build/screenshots`：`skill-enchantment-pixels-410.png`、`skill-impact-67.png`、`skill-casting-timeout.png` 等为本次实际视口捕获；截图展示呈现结果，超时/纠偏结论依据运行断言，不靠静态截图证明。
