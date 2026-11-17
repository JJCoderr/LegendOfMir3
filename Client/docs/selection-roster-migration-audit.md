# 选角列表、等待和翻页

2026-11-17，基线a7876dc。依据Mir3 br_godot_test_261115的character_selection.hpp及hud_view.cpp enter_character_selection/request_selection_list/apply_selection_message/_process/draw_character_selection。

## 实现

SelectionRoster独立页面结构，四槽、24字节ANSI姓名；不改CHRINFO旧23字节及_TCREATECHR旧43字节创建报文。参考文本列表取nRecog数量、每项name/job/unused/level/gender，最多四项，保留原BYTE转换及字段语义；无效数据不覆盖旧列表。nRecog为0且wParam非0时兼容旧packed列表（最多三条），空列表统一处理。

CCharacterProcess首次进入或请求刷新时显示GameInter1248@(155,150)、CmList200文字，关闭编辑器并阻止页面操作，保留旧列表静止显示。请求后500ms之前有效列表暂存，499ms不应用，500ms应用；没有有效回复一直等待。暂存列表之后到达的错误也一并延迟呈现。影片播放期间暂停页面计时；离线静态预览可直接提供完整数据。

CSelectChr扩展四槽动画，初始各页首角色选中动画；每页两人。翻页释放命中即可切页，切页后选择该页首角色；16位动画累积与参考一致。后续列表重建回到第一页。24字节姓名使用独立25字节槽，选角请求改用std::string拼接，消除原32字节账号/姓名拼接缓冲溢出。

删除/创建返回的列表刷新走统一等待入口；之前删除确认验收更新为确认后进入等待而非无提示状态。旧创建报文保持43字节，真实服务器协议兼容性仍待后续联调。

## 验收与截图

Client clean90/90、全七目标109/109通过；日志out/selection-roster-clean.log和out/selection-roster-all.log。运行产物out/mud3-01/Mir2Ex-roster-review.exe来自build-ninja-sqlite/Mir2Ex.exe。

列表22、提示23、影片33、登录27、转场12、分组33、音效15、连接20、退出30、返回7，共222检查全部通过。包括四槽文本、24/25字节边界、损坏列表不覆盖、整数溢出/字节转换、旧23字节、真实消息解码、499/500ms、等待禁输入、两页切换、延迟错误、无回复保持等待与空列表。所有本批预览进程停止，未连接真实网关。

预览284..288：selection-roster-first/waiting/second/long-name/empty。下一289。截图out/selection-roster-*.png为原生后缓冲640×480，列表名称是明确测试数据First/Second/Third/Fourth及24位数字。已目视检查等待、两页及长姓名；四状态汇总out/selection-roster-pages.png只拼接实际截图。

## 后续

未声称原版同帧逐像素一致或真实服务端流程已通。继续核对角色页按钮/鼠标选中、退出网址与创建编辑器初始化等剩余交互；然后做GUI整体差异清单，再安排真实网关联调。原片缺失限制仍见lobby-movie-migration-audit.md。
