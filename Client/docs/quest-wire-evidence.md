# 原生任务窗口的数据接入

本批复用 Godot 分支 br_godot_test_261115 的 client/src/protocol/quest_wire.hpp 解析规则，移植到 QuestWire.h；窗口、绘制、输入和音效均由现有 CLIENT 实现，不依赖 Godot。

## 原版依据

MIR3.EXE：46D330 全量任务、46DC30 新任务、46D570 子任务、46D7F0 正文；46CFE0 展开时查询未加载的子项，46D310 打开未初始化的列表时查询。反编译记录见 out/quest-receive-original.c、out/quest-expand-original.c。

| 消息 | 用途 | 参数 |
| --- | --- | --- |
| 1046 | 查询列表 | 全零 |
| 1048 | 查询子任务 | wSeries 父任务 ID |
| 1049 | 查询正文 | recognition 子任务 ID，wSeries 父任务 ID |
| 1301 | 替换列表 | wParam 数量 |
| 1302 | 新增任务 | wSeries 数量 |
| 1303 | 新增子任务 | wSeries 数量，正文首字段父 ID |
| 1310 | 替换子任务 | wParam 数量，wSeries 父 ID |
| 1311 | 更新正文 | wParam 子 ID，wSeries 父 ID |

1301 正文为斜线分隔的 ID/标题/子数量/完成标志/摘要，父任务按有符号 ID 稳定排序。全量替换清除旧任务、子缓存和选择；空列表保持未初始化。增量遇首个重复项停止；1303 的重复检查使用头部 wParam 指定父项，与原版及 Godot 一致。迟到正文仅更新对应缓存，不切换当前选择。整段解析成功后才修改列表。

原版通知使用 Cm1433/Cm1432、调色板索引 2/3（白字红底）；正文按 160 像素换行。现有宋体、原图资源、按钮音效及拖动滚动操作沿用。

## 验证

完整 Client 清理构建与全部七个目标构建通过。新增接收/查询/缓存/增量/错误输入检查 15 项，任务交互 14 项和相关邮件、寄售、投石车、地图回归 141 项，共 170 项全部通过。

预览：187 quest-wire、188 quest-wire-detail、189 quest-wire-increment。截图 out/quest-wire-list.png、out/quest-wire-detail.png、out/quest-wire-increment.png 已人工核对。

验证覆盖原生接收函数的六位编码正文与查询头部，不代表完整加密网络包注入或真实服务器流程已完成。
