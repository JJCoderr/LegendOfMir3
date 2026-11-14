# 原版聊天日志输出

## 实时原版证据

IDA数据库mir3-map-review。字符串4B5770为Chat.txt，两处引用均位于42F380。该函数先检查编码正文strlen<=1024，47D780解码，再按C字符串首个NUL终止。文件以ab方式追加，格式为原ANSI文本加CRLF，没有时间戳、角色前缀、BOM或Unicode转码。

存在两个独立写入分支：应用+72开启且opcode不为SM_SYSMESSAGE时写一次；随后opcode不是SM_GUILDMESSAGE时无条件再写一次。不能将两处分支合并成通常理解的“开启才记录”。实际次数如下：

| 收到通道 | @chatsave关闭 | @chatsave开启 |
| --- | ---: | ---: |
| SM_SYSMESSAGE | 1 | 1 |
| SM_GUILDMESSAGE | 0 | 1 |
| SM_HEAR / SM_GROUPMESSAGE / SM_CRY / SM_WHISPER / SM_MONSTERSAY | 1 | 2 |

聊天窗口只显示一次。40/1501另外更新人物头顶发言；104另走行会文本分支。文件打开失败仍继续显示消息。427400精确识别@chatsave，切换+72后仍发送CM_SAY，命令本身没有直接日志输出。

## 实现

ChatLog模型按现有集中协议常量计算追加次数，并生成原字节CRLF文本。HUD只从通过校验的原始接收聊天包调用，不从通用post_chat调用，因此本地提示、私聊本地回显和#短语都不会额外写日志。开启时保留原版重复行。

CMake MIR3_CHAT_LOG_PATH默认user://Chat.txt，生成storage/chat_log_path。文件独立于原资源目录，可配置绝对路径或Godot用户路径；空路径关闭文件输出。已有文件从尾部追加，不截断。普通offline_preview默认禁用输出，定向测试显式指定独立临时路径。

按原版检查编码正文<=1024（有效六位编码最多对应768个解码字节）；非人物聊天不再误用259字节人物缓冲上限。人物头顶40/1501仍保留已有259字节安全限制，避免复现原版对人物缓冲的不安全复制；这一差异不能视为原版越界行为的实现。

## 验证

hud_checks --chat-log：全部通道开关组合、重复行、GBK、CRLF、空文本、NUL及非接收消息排除。

chat_log.gd：已有文件追加、真实输入@chatsave、所有通道逐次字节比较、中文原始字节、本地私聊和短语不写文件、关闭后仍按原分支追加、畸形包拒绝、1024字节编码边界，以及日志路径不可写时消息仍显示。截图chat-log-received.png来自实际Godot窗口。

受影响模块仅验证chat_submission.gd；全游戏场景集成继续留待所有模块完成。雪花消费者、实服聊天权限/时序和原版同状态对照仍未完成。
