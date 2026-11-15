# C18 原版怪物消息复核证据

## 当前目标版本与执行顺序

用户最新指定：Monster.cpp先覆盖参考工程全部怪物，再以 `G:/游戏/宝藏传奇3客户端/宝藏传奇3.exe` 为最终依据逐项比对。Mir3.exe仅为辅助；缺少某怪物不能据此排除。宝藏启动器及其实际加载的游戏镜像、KingMir3.dll等模块必须按真实调用链追踪，不能把启动器导入视图当作完整怪物实现。本文下方已有Mir3.exe地址与结论保留其原版本标签，未逐项对照前不提升为宝藏版结论。按用户要求，所有怪物完成后统一构建与验证，期间不频繁启动测试。

实现位置更新：现有怪物行为与游戏逻辑统一在 [Monster.cpp](../../../src/world/Monster.cpp)，原 `hud_monster_wire.cpp` 已合并。参考工程先建立初版，随后逐怪物对照 EXE；本次集中没有将尚未确认的参考参数提升为 EXE 证据。Release 构建、`monsters ai` 和完整 `monsters regression` 通过，日志分别为 `client/build/monster-consolidation-build.log`、`monster-consolidation-model.log`、`monster-consolidation-regression.log`。

2026-11-15，IDA会话mir3-monster-review。survey.json记录镜像身份，imports.json记录标准导入枚举为空，不能据此声称没有网络/文件功能；未进行IAT修复，动态API覆盖不在本次消息分派分析结论内。相关限制继承[既有导入视图](../feature-audit/E-imports.md)。

collect.py已更正为只读提取32个函数和4AF3D4虚表。4AF3D0属于前一链表节点类型，此前基于该错误起点的消息映射全部作废。40579B/40581D引用4AF3D4；+16=406750、+36=407B70与动作/外观调用签名相符。export.json为未截断结果，单函数.c便于复核；raw.json是工具执行回执。反编译中的FILE类型为错误类型恢复，内部消息按偏移解释，不能当作CRT文件对象。

结论见[行为规格](../../../../docs/MonsterBehavior.md)。本轮完整枚举对象队列分派；特殊攻击的部分视觉参数、对象删除的全局生命周期和动态时序仍不能宣称全部验证。

远程分支复核使用会话 `mir3-ranged-review`。目标为宝藏传奇3客户端目录中的基础 `Mir3.exe`，磁盘 SHA-256 为 `FDCE0CF98757D64C18EB08F9346842E7028B93070872CD4C50A0BD116FD47195`。IDA survey 确认 32 位解包镜像 SHA-256 为 `08f68b794bfb28803d1132c02b26d2595dba309be5b2b475ae0cdfe6a752f8aa`，基址 `0x400000`，入口 `0x4A14EC`。标准导入视图仍为空，与现有 E-imports 一致。

在相同镜像中用 Hex-Rays 查看 `0x42E9A0`、`0x4282C0`、`0x40D8C0`、`0x40B0A0`、`0x4495D0` 和 `0x44B7E0`，并读取 `0x4AF3D4 + 104` 的 DWORD。此次导出位于本机 `client/build/monster-ranged-ida.json`。仓库中已有的 [0x40B0A0 反编译](0x40b0a0.c) 可复核分支 26、39、75 的 MAGIC 编号、延迟和动作切换。

## 宝藏传奇3.exe 的导入视图

IDA 会话 `mir3-baozang-monsters` 从安装目录独立打开 SHA-256 为 `98344D358C7381EA0609FAE61DF9C5B66F44285062BB597837309AA8FDB629FF` 的 32 位 EXE，基址 `0x400000`，入口 `0x406E38`。导入主要是 VB6 运行时。文件类包括 `__vbaFileOpen` 和 `rtcFileCopy`，进程类包含 `rtcShell`（`0x401210`）。survey 的网络、加密、注册表分类为空。完整导入枚举位于本机 `client/build/monster-baozang-survey.json`。
