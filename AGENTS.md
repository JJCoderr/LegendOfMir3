# 光通传奇3
使用GODOT复刻光通传奇3 v1.45版本。 私服包括网络家园,传奇学院,宝藏传奇

## 参考开源代码
1. https://github.com/tensafe/LegendOfMir3_Src ，应该是最初的C++版本
2. https://github.com/etorth/legendOfMir2_src ，lomcn，来源于https://www.lomcn.net/forum/threads/maybe-useful.94024/#post-1033272，20030515f client
3. https://github.com/Suprcode/Zircon， 韩版的传奇3复刻版，使用C#
4. https://github.com/etorth/mir2x, 一个C++版本，完成度还行。 https://github.com/thexinchen/mir2x，使用GODOT以及IMGUI更改过客户端。 客户端在https://github.com/etorth/CBWCQ3


## 约束
1. GODOT做客户端，C++做游戏逻辑引擎，而不是GDScripts。 采用C++23标准。
2. 支持WINDOWS,LINUX,ANDROID,IOS,WEB -- 先在WINDOWS下完成再考虑其他客户端。
3. 游戏逻辑要与1.45客户端一致。 通过IDA分析原版客户端和服务器端，分析原版游戏逻辑，再翻译。
4. 采用MVC框架，对话框采用struct定义显示的内容，以便做测试。
5. 先完成游戏客户端的界面。如果后续逻辑需要消息等，则完成对应的游戏服务器端。
6. 客户端只能解析资源文件，而不是解压。
7. 完成一个功能就使用中文提交。

## 迭代
1. 完成游戏主界面，以及各个对话框。 包括地图，背包，魔法，聊天，骑马，行会等等。 --这部分可以参考thexinchen/mir2x的GODOT客户端。 完成一个对话框后再下一个。
2. 地图和人物,NPS加载，基本游戏逻辑。行为与原版对齐。 以比奇省地图开始。
