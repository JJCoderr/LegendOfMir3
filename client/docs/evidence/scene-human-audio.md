# 场景人物动作音效

复用旧项目 HumanActions 的 0x410B20 / 0x410DF0 帧进入规则：攻击第 2 帧按武器类型选择挥动声；普通受击第 1 帧按攻击来源武器和受击者衣服选择碰撞声，第 2 帧按性别选 138/139；动作 17/16 第 1 帧选 140/141。远端走路和骑马移动接入既有脚步、马蹄规则。骑马受击没有冒用普通受击音效。

apply_scene_human_action 增加可选 hit_weapon（默认 0），表达解码后的攻击来源武器。原有三参数调用兼容。每个人物独立播放通道，避免覆盖本地脚步；人物移除或离开场景释放通道。使用既有 SoundPosition 屏幕位置衰减和 pan 换算，音效音量、关闭和反转声道选项生效。当前每人物一个 AudioStreamPlayer，连续不同声音会替换本人物前一个流；尚未对齐原版 DirectSound 的同源多实例重叠行为，也未做录音波形对比。

新增 resources/sound_list.hpp：根据 reference/LegendOfMir3_Src/LegendOfMir3_Client/Sound/TypeDef.h 的 pack(1) 布局解析 SoundList.wwl（58 字节头、16 字节记录、2 字节 ID + 14 字节描述），参照 MirSound.cpp ReadWavFileList 截断 wav 后残留字符。特殊动作 140 对应 m34-1.wav，141 对应 m35-1.wav。当前用户资源缺少后一个文件，按缺资源保持无声，不创建替代资源。

验证：Release CMake 构建；hud_checks --scene-actions 覆盖时间边界、武器/衣服/性别音效分组和索引截断；Godot --scene-audio 检查实际 WAV 播放、特殊动作索引、缺文件不误播、静音、远端脚步/马蹄和移除清理；--scene-actions 验证新增音效未破坏动作恢复与场景退出。截图 bichon-human-action-sound.png、bichon-remote-hoof-sound.png 来自实际 800×600 视口。未运行无关对话框测试。
