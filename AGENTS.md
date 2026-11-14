# 光通传奇3
使用GODOT复刻光通传奇3 v1.45版本。 私服包括网络家园,传奇学院,宝藏传奇

## 参考开源代码
分析各个开源代码的游戏逻辑，并按模块功能记录。
优先参考Zircon等前面3个原版的游戏逻辑。 GODOT客户端则可以参考thexinchen/mir2x的已有实现。

1. https://github.com/tensafe/LegendOfMir3_Src ，应该是最初的C++版本
2. https://github.com/etorth/legendOfMir2_src ，lomcn，来源于https://www.lomcn.net/forum/threads/maybe-useful.94024/#post-1033272，20030515f client
3. https://github.com/Suprcode/Zircon， 韩版的传奇3复刻版，使用C#
4. https://github.com/etorth/mir2x, 一个C++版本，完成度还行。 https://github.com/thexinchen/mir2x，使用GODOT以及IMGUI更改过客户端。 客户端在https://github.com/etorth/CBWCQ3


## 约束
开始一个模块/功能前，需要使用IDA分析原版的实现。 按按模块/功能记录到客户端和服务端的RERENCES.md。 有不清晰的地方可以参考几个开源实现。 但优先原版。
需要归纳原版的游戏逻辑，资源索引，文件格式，物品数据等等开发需要的重要知识，并记录到docs的独立文件中。

1. 采用C++23标准，采用CMAKE管理工程。
2. client/server各维护一个TODO列表。 加入未完成的模块，功能，逻辑，方便后期联调时候跟踪。 将已完成的标上[x]。
3. 每个模块/功能需要独立的测试集。 再按游戏场景做集成测试。 测试脚本要在开头使用中文记录测试的内容。
4. 采用合理的实现颗粒度，完成相关功能测试后，再使用中文进行提交。 一次需要完成多点的功能，才启动测试，不用每次小改就测试和提交。

## 客户端
目标： 将原版的客户端改为GODOT实现。
1. GODOT做客户端，C++做游戏逻辑引擎，而不是GDScripts。 
2. 客户端支持WINDOWS,LINUX,ANDROID,IOS,WEB -- 先在WINDOWS下完成再考虑其他客户端。
3. 字体，坐标，大小，行为要与原版一致。 可以通过IDA进行分析。
4. 采用MVC框架，对话框采用struct定义显示的内容，以便做测试。 一个对话框的逻辑对应一个类，整个主界面逻辑都放到一个cpp文件中。
5. 只能解析资源文件，而不是解压。 客户端代码放入client。
6. 先完成游戏客户端的界面，但需要方便后期服务器端的接入（比如采用消息队列）。

## 迭代
0. 记录IDA对客户端和服务端的分析结果，以便后面借鉴。
1. 完成游戏主界面，以及各个对话框。 包括地图，背包，魔法，聊天，骑马，行会等等。
2. 地图和人物,NPS加载，基本游戏逻辑。行为与原版对齐。 以比奇省地图开始。
3. 启动测试服务器端，然后让实现的客户端匹配这个服务器端。
