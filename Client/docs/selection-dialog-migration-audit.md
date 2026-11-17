# 角色创建、删除和选角提示框

2026-11-17，基线23b30bd。依据Mir3 br_godot_test_261115的hud/character_selection.hpp、create_character.hpp，以及hud_view.cpp的creation_input/close_selection_notice/open_selection_delete/finish_selection_delete/apply_selection_message/selection_input。

## 本批实现

原生CLoginNotice/GameInter提示框，继续使用既有字体、颜色和固定位置。CmList消息集中在SelectionDialog：219空角色、220查询失败、221入场失败、222断开、223名称过长、224重名、225名称非法、226角色上限、227无可删角色、228删除确认、230无可开始角色、232创建失败默认。SM_NEWCHR_FAIL读取nRecog，2对应224，3对应226，其余232，替换原英文和错误wParam读取。

名称13字节上限，按参考ANSI双字节遍历，合法双字节尾字节即使等于反斜线也不误拒绝。空名称不提交。创建入口两角色上限按参考执行；不表示列表四槽解析已完成。

普通提示Enter/空格关闭，Esc保持；关闭创建错误会清空名称并恢复编辑器焦点。角色提示点击释放落在按钮即可关闭，其他页面仍保留原按下/释放匹配语义。删除默认选是、Tab切换、按下按钮更新键盘选择。复用90字节输入框，关闭时清空并隐藏；确认内容先读取后清理，CM_DELCHR发送name/confirmation并请求刷新，回复成功/失败也刷新列表。列表刷新关闭旧提示或显示219。

删除发送格式按Godot参考接入，仅在127.0.0.1回环验收；当前旧DBSvr与该格式兼容性待真实协议联调，不能宣称真实删除已通。没有执行真实角色创建/删除或连接配置中的外网。

## 验收

Client clean90/90、全七目标109/109、最终3/3增量构建通过。日志out/selection-dialog-clean.log、out/selection-dialog-all.log、out/selection-dialog-final.log。可执行out/mud3-01/Mir2Ex-selection-dialog-review.exe来自build-ninja-sqlite/Mir2Ex.exe。

专项23、影片33、登录27、转场12、分组33、音效15、连接20、退出30、返回7，共200检查通过。最终列表清理修正后重跑专项、影片、退出回归。回环实际解码CM_DELCHR证明确认文字未丢失。未额外新增提示音，沿用参考行为及已有按钮/大厅音频。

预览272..283：character-name-exists/full/create-failed/name-invalid/query-failed/start-failed/disconnected/delete-confirm/delete-missing/start-missing/name-long/empty。下一284。每个真实截图out/character-*.png；三张2×2汇总out/character-dialogs-1.png至3.png按上述顺序。空角色预览使用实际空列表，名称过长提示在创建页。汇总图只拼接截图，未改UI内容。已目视检查全部12状态。所有本批预览进程停止。

## 下一步

仍需核对选角首次进入及刷新列表的等待状态/500ms延迟：参考在500ms前暂存列表，再显示角色或空提示。当前原生支持旧packed三角色列表，参考支持文本四槽，需迁移/兼容并核对翻页。不能将这批提示框完成当作整个角色页面/协议已完成。真实服务器流程仍按用户要求待GUI之后统一调试。
