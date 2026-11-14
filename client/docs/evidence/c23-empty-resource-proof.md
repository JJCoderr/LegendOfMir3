# C23：47/48 原安装目录空帧核对

2026-11-14 直接读取 `G:/游戏/宝藏传奇3客户端` 与 `G:/mir3/mir3_1.45/resources` 中的资源文件，只解析索引，不导出素材。两目录的下列文件逐字节相同：

| 文件 | 字节数 | SHA256 |
|---|---:|---|
| MInfo.dat | 46874 | ed46fe33682180fbc23796797ba04f4db9928daf4eadb6937d79803d957a3b0b |
| SoundList.wwl | 21274 | 2e9f91f730f455f174d88918d2625cc3d174e0b3e5a186dd44bc9105911416c5 |
| Data/Mon-4.wix | 40024 | 2b8ebeed26e86a153870fa72ca99f66ca5f281649f4d6baf8461432a8ef55dfd |
| Data/Mon-4.wil | 12343624 | cc4075a1b48d205bebf923395fa2abb318b317c24c2bfdfa76a6f8349b19246f |
| Data/MagicEx.WIX | 8148 | 540e7d07e96298dfe4d72d253d6278d6c3e7cd4d36d930d43a62c06586866d30 |
| Data/MagicEx.wil | 23152617 | 7e2b3f1cd3a8932243fca9b0536c6f47a4c2e168ddfee68ca40d692d1244b55d |

从 WIL 偏移 22 读取小端版本字段：Mon-4 为 17，MagicEx 为 5000；对应 WIX 索引头分别为 24、28 字节。在 `header + frame * 4` 读取小端 DWORD，以下每一项均为零，两目录结果一致。

| 技能 | 阶段 | 库 | 帧区间（含两端） | 空索引数 |
|---|---|---|---|---:|
| 47 | SPELL | Mon-4 | 940–949 | 10 |
| 47 | MAGIC | MagicEx | 1060–1079 | 20 |
| 48 | SPELL | MagicEx | 760–769 | 10 |
| 48 | MAGIC | MagicEx | 770–775 | 6 |

原始读取记录保存在忽略目录 `client/build/c23-original-resource-proof.json`。此前 common、stationary 的生命周期验收记录仍有效，本次没有重复运行已通过技能。

结论：这些帧在原安装目录的磁盘资源中就不存在，可排除复制遗漏。MInfo/SoundList 相同证明输入文件一致，不代表已独立验证运行时加载内容。尚未证明宝藏传奇3.exe 或其加载模块不会在运行时替换资源、索引或技能表；该项与原版同状态视觉对照继续保留，不能据此将 C23 标记完成。
