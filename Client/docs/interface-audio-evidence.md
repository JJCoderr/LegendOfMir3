# GUI音效播放链路验证

## 改动

- CMirSound::InitMirSound返回DirectSound Create的实际结果，原先只要窗口非空就报告成功。
- PlayLobbyEffect返回加载/播放成功状态，静音、无设备或文件不存在返回false；原有调用方无需改变，资源和播放时机保持不变。
- 增加只读InspectInterfaceAudio及CSBuffer::GetVolume，用DirectSound GetStatus/GetVolume核验实际缓冲区，不把配置字段或请求调用当作已播放。

## 验证

- 客户端干净构建87/87，全目标109/109通过。
- 预览212 interface-audio：15项全部1。实际DirectSound设备已创建；105.wav进入播放状态；正在播放的音效应用-2000/-4000音量；音乐数值保持独立；通过原生选项按钮静音停止及恢复播放；另一个原生选项按钮触发点击音效；不存在文件明确失败并停止前一个界面音效。
- 12个职业/性别/创建或选角语音逐个先停止，再调用真实PlayRoleVoice并读取播放状态，全部通过；物品使用音效通过。没有用上一个仍在播放的样本冒充下一样本成功。
- 开场Opening.wav在1200ms不播放、再1ms播放；循环缓冲实际音量更新为-1000；静音立即停止。
- 既有options-panel9项通过：滑块两端、两个通道独立、左右声道反转、INI恢复、DirectShow音乐播放/停止/恢复。两组共24个检查条件，语音覆盖12个资源样本。
- 截图out/interface-audio.png已检查，仍为CLIENT原生选项窗口。测试路径使用preview-audio-options.ini或无持久化路径，不改用户Mir3.ini。

## 范围

证明的是实际Windows音频后端的播放状态及音量，不是扬声器录音或听感鉴定。没有录制系统其他程序音频；声卡输出设备、扬声器物理输出、全场景音效混音仍需实机验收。
