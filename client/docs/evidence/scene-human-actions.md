# 场景人物攻击与受击

本批按功能移植 `G:/mir3/mir3_1.45/human_actions.hpp` 的动作表、武器分组与有限时钟，采用 `remote_actor_action.hpp` 已分析的 opcode 映射。原版依据为 0x46EFA0 / 0x4872D0 动作表、0x4702F0 武器攻击选择、0x416E70 有限动作推进、0x412500 受击和 0x412C80 攻击。既有身体索引与分层绘制沿用 LegendOfMir3_Src 的 Actor.cpp / SpriteInfo.cpp 参考结论。

`apply_scene_human_action(actor, opcode, facing)` 接受 14、15、1306、1307、1308、1319、1309。武器攻击分为动作 9、10、13；普通受击 15，骑马受击 32，受击保持原方向。身体、头发和武器共用动作帧索引，保留染色和 Weapon.ord 次序。资源按需从配置目录 WIL 读取并缓存，不导出或复制。结束恢复普通或骑马站立。站立/移动更新取消有限动作。

接口仅为解码后的场景显示入口。移动中动作返回 false，调用者需保留消息；尚未实现网络消息队列。骑马攻击不接受；技能特效、攻击音效、命中判定、状态减速标志不在本批接入范围。特殊动作目前仅显示人体动作，不能据此认为技能完整实现。

验证：Release CMake 构建；`hud_checks.exe --scene-actions` 检查严格时间边界、每更新最多推进一帧、有限动作结束、武器分组及骑马帧索引；Godot `hud_smoke.gd -- --scene-actions --screenshots` 独立场景检查三种武器动作、特殊映射、非法输入、方向保持和骑马恢复。实际视口截图为 `bichon-human-attack.png`、`bichon-rider-hit.png`。未运行无关对话框测试。
