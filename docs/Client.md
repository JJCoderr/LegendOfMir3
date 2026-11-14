# 原版客户端行为与Godot实现记录

按当前AGENTS.md维护客户端逻辑。既有逻辑记录保留在[Game.md](Game.md)，各子系统的地址证据、伪代码与验收范围见[客户端TODO](../client/TODO.md)及[模块索引](../client/docs/evidence/client-module-inventory.md)。

本地动作相关专题：[频率及共用等待](../client/docs/evidence/map-input.md)、[服务器确认与拒绝](../client/docs/evidence/action-reply.md)、[服务器位移](../client/docs/evidence/forced-motion.md)、[死亡及重新入场](../client/docs/evidence/local-life.md)。

Godot资源生命周期：[音频退出清理](../client/docs/evidence/audio-cleanup.md)。这类修复只处理适配层资源所有权，不改变已确认的原版音效行为。

本地战斗反馈：[受击队列、血量与动作](../client/docs/evidence/local-struck.md)。

采集动作：[12/637回包与本地/远端差异](../client/docs/evidence/butch-reply.md)。

角色状态消息：[801/807/808排队更新与1100/1101即时入口](../client/docs/evidence/actor-status.md)。

人物状态表现：[颜色优先级、图层混合与外观状态同步](../client/docs/evidence/human-status-visual.md)。
