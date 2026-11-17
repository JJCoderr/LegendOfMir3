# GUI资源版本盘点（2026-11-17）

本次为只读资源盘点，没有替换/重打包素材，没有运行压缩包中的程序。检查范围是G:/传奇3工作区（rg默认不跟随目录链接），并单独读取以下压缩包目录；不声称搜索了其他磁盘或所有嵌套归档。

## 当前素材

实际文件：G:/传奇3/宝藏传奇3/Data/GameInter.wil。工作区仅找到这一套按该文件名命名的解压素材。运行目录使用现有链接。

SHA256：`4975920dd39f832fb90c050a038dab3177eef93071951c6b76da242ba3de1df4`。

| 帧 | 宽×高 |
|---|---|
| 1240 | 360×268 |
| 1241 | 80×32 |
| 1242 | 80×32 |
| 1243 | 80×32 |
| 1244 | 80×32 |
| 1245 | 80×32 |
| 1246 | 80×32 |
| 1248 | 328×162 |
| 1250 | 344×462 |
| 1251 | 84×30 |
| 1252 | 84×30 |

公告1250为344×462，确认按钮1251/1252为84×30。out/notice-source-frames.png已查看；该拼图缩小了底板，仅用于资源识别，不是原尺寸页面截图。原版函数的x+50、324像素正文宽度与这套底板存在已记录的视觉疑点，不能凭素材缩略图决定缩窄排版。

## 压缩包目录

- 传奇学院1.45传奇3经典复刻版无插件.rar：tar列出2610条，内容为Mir3Server服务端，未列出GameInter、目标影片或AVI。
- mir2x/mir2x_res.zip：490624条，只匹配到wemade/CreateChr/StartGame/ToCreateChr的MP3音乐，没有同名影片或GameInter文件；不同资源命名体系不能据此认定每张纹理都不存在等价内容。
- 宝藏传奇3/Data/Wood/DATA.zip：4项，M-Hum.wil/wix、WM-Hum.wil/wix，为人物素材。
- 宝湾传奇3服务器端.7z：7-Zip只读目录检索未匹配到GameInter、三个目标影片或AVI。

证据：out/client-archive-files.txt、out/server-archive-files.txt、out/gui-resource-archive-audit.json、out/gui-resource-audit.json。原版wemade.dat/CreateChr.dat/StartGame.dat在本次检查范围仍未找到。没有把MP3当作影片替代品。

已向用户询问是否有另一份原版客户端目录；答复前保留缺失和版本匹配问题，不修改既有资源。其他GUI核查仍可继续，整体目标保持active。
