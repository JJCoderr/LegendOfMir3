# 人物消息队列与连续动作

依据旧项目 actor_message_queue.hpp / extension.cpp 中的 0x481A30、0x413F90、0x414220、0x411480 已分析路径，新增 C++ SceneHumanQueue 和 queue_scene_human_message 解码后入口。每人物 FIFO 最多 99999 条，支持当前已实现的站立、走跑、攻击和受击操作。无效消息不改变队列，人物移除连同队列销毁。

每次开始动作前读取积压数，至少三条则追帧：移动每次进两帧，攻击/受击在首次推进后间隔减半。移动按场景共用 >95 ms 节拍，每更新最多一拍；减速位 0x08000000 跳过奇数拍，有限动作时钟使用双倍间隔。跨越提交帧也会提交目标格，避免追帧跳过第 1 帧后不落点。移动结束下次更新才消费下条；有限动作结束同次更新可消费一条。1307 按原版保持人物方向。直接 apply_* 入口仍用于即时状态设置，正常顺序播放应使用 queue_*。

本批没有接网络层：原始消息的头/体解码、外观随包更新、HP 消息折叠、死亡/复活等完整协议队列仍待实现，已列入 client/TODO.md。队列只承载当前支持的解码后字段；不能用本测试声称真实服务器联调完成。

定向验证：Release CMake 构建；hud_checks --scene-queue（FIFO、积压阈值、容量、非法输入、追帧和减速时间边界）；Godot --scene-queue（移动→攻击→受击→站立串联、0/2/4 追帧、方向/落点、无效输入、移除/ID复用），截图 bichon-queued-attack.png 和 bichon-queued-hit.png。移动节拍改动另执行相关 --scene-human-step --scene-human-mount 场景，动作声音执行 --scene-audio。
