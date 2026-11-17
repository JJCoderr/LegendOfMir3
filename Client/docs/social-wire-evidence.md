# 行会主页、成员及组队消息接入

依据 Godot br_godot_test_261115 的 guild/guild_overview.hpp、guild_members.hpp、protocol/group_message.hpp、guild_notice.hpp，以及docs/evidence/guild-wire.md、group-wire.md。原版主要入口433A30、433DA0、434000、4282C0；复用其纯C++解析规则，使用当前CLIENT原生窗口。

## 行会

753解析名称、权限、公告/敌对/同盟段并打开CGuildWnd，替换原来误开旧GuildMaster窗口的空分支。773/774保留持久累积及原版编辑行重复追加行为；756/775/776独立重建成员显示和职位编辑内容，管理员显示职位序号。坏输入先拒绝，不覆盖旧数据。初始化清理累积状态。

F键与HUD原有1036主页查询一致，行会主页/成员按钮分别查询1036/1037，通知历史按钮不发查询。增加成员成功先查询1037再提示1474，删除成功只查询。失败提示按有符号recognition映射，未知原因忽略；复用原生单确认框和音效。

## 组队

660允许组队；666仅清成员，保留窗口与允许状态；667逐个读取以/结束的名字，非终结尾字段忽略，名字上限31字节，失败整包不改变列表。661/664/665恢复失败提示。会话初始化清成员及允许状态，与Godot清理行为一致。

## 验证

清理构建后首次因测试访问旧消息框字段失败；改用CLoginNotice只读Text接口后全部七目标构建通过。15项社交接收/请求检查（包含15种行会原因、10种组队原因的组合检查）加行会编辑17项、组队创建1项、技能12项，共45项全部通过。F键、成员按钮、成功回执刷新通过实际SendPacket本地UDP回环验证零参数1036/1037报文。

195 guild-wire、196 guild-wire-members、197 guild-wire-editor、198 guild-wire-error、199 group-wire、200 group-wire-error，六张out/*wire*.png原生截图已核对。保留现有字体、颜色、尺寸和按钮音效。

本批不是完整实服验证。行会通知历史页m_pages[2]仍缺正常游戏填充来源；Godot post_system_notice仅找到登录欢迎调用，不能未经证据把所有聊天都追加进去。称号/身份信息同步、聊天命令触发的完整管理业务及实服流程仍需后续验证。


## 行会聊天历史（2026-11-17）

- 原版分析库：Mir3-unpacked-analysis.exe.i64，输入 SHA256 08f68b794bfb28803d1132c02b26d2595dba309be5b2b475ae0cdfe6a752f8aa；本轮 IDA 会话 1a4e6f32。
- 42F380 是消息接收入口；仅消息 104（SM_GUILDMESSAGE）调用 4331C0。聊天背景使用色表 4、文字使用色表 0；普通聊天仍读取 header 的低字节颜色。100 等其他消息不进入行会历史。
- 4331C0：宋体 9pt、514 像素 ANSI 字节前缀测宽；延续 Godot chat_history.cpp 的高位字节回退与 sscanf 分行，最多五行。每行追加前若历史行数减共享滚动位置等于 18，则位置加一；追加后达到 80 行即删除最早行，因此最多保留 79 行。433900 允许向下滚到最后一行，不是最后一整屏。
- 42F380 限制编码消息长度 1024；4366D0 对聊天区限制解码文本 200 字节，长消息仍进入行会历史。接收缓冲额外留终止符，人物气泡仅接收其固定数组能容纳的文本。
- 使用 CLIENT 原有行会窗口、字体创建与声音路径；页签点击仍播放 105.wav。本次没有添加原版不存在的消息到达音效。音效扬声器输出未录音验证。
- 干净构建 87/87，全目标构建 111/111，通过；最终增量构建通过。预览 guild-history 经真实 OnSocketMessageRecieve 分发验证 11 项全部为 1，guild-wire 15 项及 guild-editor 17 项全部为 1。
- 截图：out/guild-history.png；预览号 201。截图内容为构造协议消息，尚未连接真实游戏服验证。


## 人物页行会身份（2026-11-17）

- Godot hud_view.cpp 的 set_character_identity / 人物页已完成四字段排版，CLIENT 原有 m_identityTertiary / m_identityBadge 之前仅由预览赋值。旧消息750把行会和职务拼成单行，且无分隔符时不清空。
- 原版 4282C0 的 case750（42B859..42B970）不检查 nRecog，直接更新本地身份。先用 strchr(0x2f2a) 找 `*`；这是单字节查找，实际匹配星号；其后为称号，且原版删除星号前一个字节（通常为 `/`）。然后以首个 `/` 分割行会与职务。无分隔符清空这两项，无星号清空称号。
- 原版行会与职务合计限制40个ANSI字节，截断职务；称号原始目标数组60字节。CLIENT拒绝超过40字节的行会名或超过59字节的称号，避免复现原版越界；合法数据保持原版字节截断，不改成Unicode字符计数。
- CStatusWnd::ReceiveGuildIdentity 已由实际 SM_CHANGEGUILDNAME 分发调用，使用原有原生文字/图标。无额外音效（原版该消息分支没有音效调用）。消息42的角色名称流程未在本批改动，仍需另行核对。
- clean87/87、全目标109/109通过；identity8 + history11 + ability13 = 32项检查全部1。预览202 guild-identity、203 guild-identity-cleared，经 OnSocketMessageRecieve 注入消息。截图 out/guild-identity.png、out/guild-identity-cleared.png 已检查；真实服务器尚未联调。
