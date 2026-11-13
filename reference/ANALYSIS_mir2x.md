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


## 使用与验证边界

本轮为源码导航与代表性路径分析，没有构建或运行该参考工程。表中入口已核对存在；不能据此宣称整套逻辑与 1.45 一致。读取目标函数及直接调用者后，再迁移到 `client` 的对应功能模块。只验证本次影响的显示和行为，相关功能成组完成后统一截图与提交。
