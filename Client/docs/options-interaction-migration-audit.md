# 选项页释放行为与拖动验收

依据 Mir3/br_godot_test_261115 的 hud_view.cpp options_pointer：普通控件在释放命中时执行动作；窗口或音量滑块拖动结束只结束捕获。mount_pointer 则要求同按钮按下/释放配对，不能把两种窗口统一成同一种行为。

## 实现

COptionWnd 普通开关/关闭/服务器选项按释放位置执行，无须同按钮按下。CInterface 在选项窗口拖动结束时先清理拖动/按钮状态并消费释放，避免释放到开关时误切换；点击控件明确取消窗口拖动状态。音量滑块既有捕获链路保持。

骑乘布局、四按钮及关闭按钮、按钮配对、拖动和提示按当前原生代码与参考核对，本批没有修改骑乘实现。未执行真实骑乘命令。

## 验证

全七目标增量构建成功，out/options-interaction-build.log 和 options-interaction-final.log。测试二进制 out/mud3-01/Mir2Ex-options-review.exe 来自 build-ninja-sqlite/Mir2Ex.exe。

新增10项输入检查，经实际 Interface 入口验证释放切换、按下A释放B、音效恢复、局部开关、窗外释放、背景拖动、拖动释放不切换、按钮不拖窗、正常配对及关闭。音量/保存9、服务器状态6、标记5、阴影4、光照6、伤害9、攻击5、音效15、骑乘13，总82项全通过。旧标记用例错误地期待孤立释放不生效，本批按参考修正预期，保留后续关闭/保存/恢复检查。

预览300 options-interaction；复用既有 options-panel/options-*-on/off、interface-audio、mount-panel。下一301。实际截图 out/options-interaction.png、out/mount-panel.png 已查看。预览进程已停止；配置写入仅已有 preview-*.ini 夹具，未做真实网关联调。

## 后续

本批证明常规释放和拖动路径，尚未证明所有选项多次交叉按下的中间按钮帧均与参考完全一致。整体GUI仍按 gui-completion-audit.md推进，不能将本批82项当作全页面终验。
