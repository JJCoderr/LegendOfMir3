# 地图消息 51 / 634 / 54

## 目标证据

2026-11-14 使用 ida-reverse 技能，通过本地 HTTP API 恢复现有分析库并反编译；未使用 Computer Use。输入镜像为 Mir3-unpacked-analysis.exe，IDA 记录 SHA256 为 `08f68b794bfb28803d1132c02b26d2595dba309be5b2b475ae0cdfe6a752f8aa`。

- 0x4282C0：51 与 634 进入公共消息队列分支；54 调用六位解码器，将名称写入 this+96，容量参数 260，加 NUL 后返回。
- 0x42EC70/0x42EDDD：51、634 共用 0x42EE00，此分支先于普通人物 ID 匹配。
- 0x42EEC3 / 0x42EEE1：record+6 / +8 为 x / y；0x42EF2A 从 record+12 解码地图文件名；0x42EF55 设置以人物格减 12 为基准的视点，0x42EF61 载入地图。
- 0x42EF9D 读取 record+10 低字节：0→255、1→15、2→85，其余→10；record+11 高字节在 0x42EFEE..0x42F03F 处理 1/2/3 的地图状态，不能当成亮度。
- 0x42EE30 写入 1500ms 字段；本批后续已追踪为普通左键按下延迟，见 map-input.md。两个早期参考版本的换图代码使用 2300ms，不能直接移植该常量。
- 0x42F082 之后清理临时列表；名称消息 54 没有这些清理操作。

两套早期 C++ 的 SendNewMap / RM_CHANGEMAP / SendMapName 同样区分地图文件编号和地图显示名称；Zircon 使用 MapIndex/InstanceIndex、mir2x 使用本地主角 action.mapUID，协议不可混用。完整对照见 ../../RERENCES.md。

## 客户端接口与字段

`apply_scene_message(encoded,key)` 接收已分帧的编码头（16 字节）与可选六位编码消息体。当前无 TCP/网关解帧实现。

| 消息 | 参数 0 | 参数 1 | 参数 2 | 消息体 | 处理 |
| --- | --- | --- | --- | --- | --- |
| 51 / 634 | x | y | 低字节亮度，高字节地图状态 | 地图文件编号 | 排队切图 |
| 54 | 忽略 | 忽略 | 忽略 | ANSI 显示名称 | 只更新名称 |

51/634 的 recognition 不用作远端人物存在性判断，也不自动绑定本地人物身份。文件编号按 NUL 终止，当前允许 1..64 个 ASCII 字母、数字、下划线或短横线；资源路径为配置根目录下 Map/<编号>.map。54 按 NUL 截取后最多 259 个 ANSI 字节；转换为 UTF-8 供 HUD 显示，空名称允许，不关闭对话或刷新小地图。

地图请求在 `_process` 中按已排队顺序交给 change_map。地图文件缺失或出生格非法时保留当前场景；`apply_scene_message` 返回 true 只表示编码和结构正确、消息已入队。最终结果由 `map_change_finished(opcode,map_id,success)` 通知。成功后保存高字节 map_series；高字节1/2的雪雾效果已在weather.md恢复，天气3仍待恢复。54 独立更新 model.map_name，切图不会以文件编号覆盖已收到的显示名。

公共地图队列当前独立于已有即时对象入口；后续必须统一真实接收批次中的入场/离场/人物创建顺序。不能把本接口测试视作完整公共消息调度、初次登录人物创建或服务器视野联调完成。

## 验证

- Release 构建。
- `hud_checks.exe --map-wire`：51/634 字段拆分、NUL 截断、合法文件编号和长度边界、路径拒绝、54 不当作切图。
- Godot `--map-wire --screenshots`：51 延迟切图、随后 54 中文名称保留、亮度与状态高低字节、失败完成通知及原场景保留、634 返回比奇、名称更新不移除新人物/NPC/对话、超长名称拒绝。
- `--map-change --screenshots` 定向回归既有切图入口。
- 截图：client/build/screenshots/map-wire-arrival.png、map-wire-description.png。脚本开头中文注释仅列测试内容，不记录准备条件和执行流程。

1500ms 普通左键延迟已实现，见 map-input.md；地图状态视觉、淡入、正式网关解帧及完整接收顺序仍待完成。原版错误包可能存在越界风险，本实现拒绝超长名称，不复刻内存破坏行为。
