# 光通传奇3
使用GODOT复刻光通传奇3 v1.45版本。 私服包括网络家园,传奇学院,宝藏传奇

## 参考开源代码
除了通过IDA分析客户端/服务器端的逻辑之外，还可以参考reference中的几个开源实现。
参考代码放在reference目录下。 优先参考Zircon等前面3个原版的游戏逻辑。 GODOT客户端则可以参考thexinchen/mir2x的已有实现。

先在reference目录下，生成各个工程的分析文档，后面参考时候可以使用，就不用每次都重新分析。 
按照工程再细分客户端，服务器端，各个游戏逻辑，网络协议，网关，游戏资源(数据库，物品，地图，加密/解密)等等模块。
仔细分析游戏逻辑，并按功能模块记录。

1. https://github.com/tensafe/LegendOfMir3_Src ，应该是最初的C++版本
2. https://github.com/etorth/legendOfMir2_src ，lomcn，来源于https://www.lomcn.net/forum/threads/maybe-useful.94024/#post-1033272，20030515f client
3. https://github.com/Suprcode/Zircon， 韩版的传奇3复刻版，使用C#
4. https://github.com/etorth/mir2x, 一个C++版本，完成度还行。 https://github.com/thexinchen/mir2x，使用GODOT以及IMGUI更改过客户端。 客户端在https://github.com/etorth/CBWCQ3


## 约束
1. GODOT做客户端，C++做游戏逻辑引擎，而不是GDScripts。 采用C++23标准，采用CMAKE管理工程。
2. 支持WINDOWS,LINUX,ANDROID,IOS,WEB -- 先在WINDOWS下完成再考虑其他客户端。
3. 游戏逻辑要与1.45客户端一致。 通过IDA分析原版客户端Mir3.exe和服务器端，分析原版游戏逻辑，再翻译。字体，坐标，大小，行为要与原本一致
4. 采用MVC框架，对话框采用struct定义显示的内容，以便做测试。 一个对话框的逻辑对应一个类，整个主界面逻辑都放到一个cpp文件中。
5. 先完成游戏客户端的界面。如果后续逻辑需要消息等，则完成对应的游戏服务器端。
6. 只能解析资源文件，而不是解压。 客户端代码放入client。
7. 完成一个功能就使用中文提交。一次需要完成多点的功能，才启动测试，不用每次小改就测试和提交
8. 需要按照大的功能模块来制作独立的测试集。然后再按大的游戏场景做集成测试。 
9. client/server各维护一个TODO列表，将未完成的功能，逻辑，需要加入，方便后期联调时候跟踪。 已完成的通过[x]标明完成。

## 迭代
0. 完成参考代码的分析，IDA对客户端和服务端，以便后面借鉴。
1. 完成游戏主界面，以及各个对话框。 包括地图，背包，魔法，聊天，骑马，行会等等。
2. 地图和人物,NPS加载，基本游戏逻辑。行为与原版对齐。 以比奇省地图开始。
3. 启动测试服务器端，然后让实现的客户端匹配这个服务器端。
