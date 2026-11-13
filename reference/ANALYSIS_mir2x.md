# mir2x 功能分析

本地分析日期：2026-11-14；基准提交：`16a0c24d236f7d1387dd365b059652fb05b23813`。

本地 submodule 指向 thexinchen/mir2x。当前检出版本的客户端为 C++、ImGui/OpenGL；client/CMakeLists.txt 链接 imgui、miniaudio、stb，未发现 project.godot/tscn 工程。不能把此版本标为已确认的 Godot 客户端。

## 按功能定位

| 功能 | 首先读取 | 需要确认的内容 |
| --- | --- | --- |
| 主界面 | [ImMainUI.cpp](mir2x/client/src/ImMainUI.cpp)、[processrun.cpp](mir2x/client/src/processrun.cpp) | UI布局与运行场景 |
| 背包 | [ImInventoryBoard.cpp](mir2x/client/src/ImInventoryBoard.cpp)、[invpack.cpp](mir2x/client/src/invpack.cpp) | 格子布局与物品容器 |
| 人物、组队与行会 | [ImPlayerStateBoard.cpp](mir2x/client/src/ImPlayerStateBoard.cpp)、[ImTeamStateBoard.cpp](mir2x/client/src/ImTeamStateBoard.cpp)、[ImGuildBoard.cpp](mir2x/client/src/ImGuildBoard.cpp) | 按窗口取功能，避免整体引入 |
| 骑乘、任务、小地图 | [ImHorseBoard.cpp](mir2x/client/src/ImHorseBoard.cpp)、[ImQuestStateBoard.cpp](mir2x/client/src/ImQuestStateBoard.cpp)、[ImMiniMapBoard.cpp](mir2x/client/src/ImMiniMapBoard.cpp) | 独立窗口入口 |
| NPC | [ImNPCChatBoard.cpp](mir2x/client/src/gui/ImNPCChatBoard.cpp)、[clientnpc.cpp](mir2x/client/src/clientnpc.cpp) | 文本对话和场景对象分离 |
| 移动与网络 | [myhero.cpp](mir2x/client/src/myhero.cpp)、[hero.cpp](mir2x/client/src/hero.cpp)、[processrunnet.cpp](mir2x/client/src/processrunnet.cpp) | 本地人物、通用人物、消息分发 |
| 绘制适配 | [gldevice.cpp](mir2x/client/src/gldevice.cpp)、[gui_texture.cpp](mir2x/client/src/gui_texture.cpp)、[CMakeLists.txt](mir2x/client/CMakeLists.txt) | 当前图形后端及依赖 |

## 已核对的实现和使用边界

`ImInventoryBoard.cpp` 的 overlayButton 使用 ImGui::InvisibleButton 做命中，按 IsItemHovered/IsItemActive 选 hover/down 图像，再 AddImage 绘制。可参考按钮状态和视觉绑定；迁移到 Godot 时仍由 C++ 控制器负责行为，不引入 ImGui 作为本工程窗口框架。

`ProcessRun` 提供 draw/processEvent 以及地图 tile、object 绘制入口；适合参考场景职责划分。人物资源命名、包格式、协议和地图参数不能据此认定与传奇3 1.45 相同。

`client/CMakeLists.txt` 的安装流程会处理资源。本项目要求原包就地读取、路径配置，因此不复用它的资源转换/安装流程。Godot 绑定和 1.45 特有规则继续从 G:/mir3/mir3_1.45 按功能提取；若需要其他 Godot 分支，先明确分支和提交后再分析。


## 服务器、协议、数据与资源模块

| 模块 | 源码入口 | 查询和复用边界 |
| --- | --- | --- |
| 服务器入口与消息调度 | [main.cpp](mir2x/server/src/main.cpp)、[server.cpp](mir2x/server/src/server.cpp)、[actorpod.cpp](mir2x/server/src/actorpod.cpp)、[dispatcher.cpp](mir2x/server/src/dispatcher.cpp) | actor 消息调度模型，不直接照搬到当前客户端 UI |
| 玩家和规则 | [player.cpp](mir2x/server/src/player.cpp)、[playerop.cpp](mir2x/server/src/playerop.cpp)、[playernet.cpp](mir2x/server/src/playernet.cpp) | 状态操作、网络处理分文件 |
| 怪物、NPC、任务 | [monster.cpp](mir2x/server/src/monster.cpp)、[npchar.cpp](mir2x/server/src/npchar.cpp)、[quest.cpp](mir2x/server/src/quest.cpp) | 玩法与脚本依赖按对象定位 |
| 地图 | [servermap.cpp](mir2x/server/src/servermap.cpp)、[mir2xmapdata.hpp](mir2x/common/src/mir2xmapdata.hpp)、[maprecord.hpp](mir2x/common/src/maprecord.hpp) | 运行时地图与静态定义、格式分开 |
| 网络/接入 | [netdriver.cpp](mir2x/server/src/netdriver.cpp)、[channel.cpp](mir2x/server/src/channel.cpp)、[clientmsg.hpp](mir2x/common/src/clientmsg.hpp)、[servermsg.hpp](mir2x/common/src/servermsg.hpp) | CMType/SM 消息与连接通道；并非传统三网关协议 |
| 数据库与物品 | [playerdb.cpp](mir2x/server/src/playerdb.cpp)、[dbpod.hpp](mir2x/server/src/dbpod.hpp)、[itemrecord.hpp](mir2x/common/src/itemrecord.hpp)、[sditem.hpp](mir2x/common/src/sditem.hpp) | 持久化入口、物品定义和传输表示分别定位 |
| 图像和资源库 | [wilimagepackage.cpp](mir2x/common/src/wilimagepackage.cpp)、[zsdb.cpp](mir2x/common/src/zsdb.cpp)、[installres](mir2x/tools/installres) | WIL 读取与转换安装工具不同，本项目只参考解析路径 |
| 加密与压缩 | [aesf.hpp](mir2x/common/src/aesf.hpp)、[aesf.cpp](mir2x/common/src/aesf.cpp)、[zcompf.cpp](mir2x/common/src/zcompf.cpp) | AES 接口存在不代表整个链路使用它，先查调用者再下结论 |

### 已核对与尚待追踪的边界

clientmsg.hpp 的 CMType 使用 uint8_t 枚举，含 CM_LOGIN、CM_QUERYCHAR、CM_ACTION 等消息。这不等于原版 1.45 的消息号；移植功能时应以本工程目标协议建立明确适配。

aesf.hpp 提供 AES::encrypt/decrypt 接口；zcompf.cpp 是另一压缩模块。尚未逐条追踪所有网络和资源调用点，不能声称全部包经过 AES 或同一种压缩。

查物品问题先看 playernet/playerop/playerdb 和 itemrecord/sditem；查地图问题先看 servermap 与客户端 ProcessRun；查连接问题先看 netdriver/channel。tools 中 mapconverter、各类 wil2png 和 installres 是转换工具，不能作为本项目资源复制或解包的理由。

## 使用与验证边界

本轮为源码导航与代表性路径分析，没有构建或运行该参考工程。表中入口已核对存在；不能据此宣称整套逻辑与 1.45 一致。读取目标函数及直接调用者后，再迁移到 `client` 的对应功能模块。只验证本次影响的显示和行为，相关功能成组完成后统一截图与提交。
