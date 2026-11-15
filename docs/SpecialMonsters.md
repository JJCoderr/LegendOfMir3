# 特殊怪物逐项清单

怪物行为最终以宝藏传奇3.exe为准，Mir3.exe作为辅助对照；宝藏版加载模块中的行为也需追踪。先用参考工程建立Monster.cpp，再逐项按宝藏版修正。shape是客户端外观，可由多个怪物共用；参考工程中的race、AI和MonsterImage是不同字段。既有Mir3.exe证据不自动等于宝藏版结论。

## 范围与逐项进度

按用户最新要求，Monster.cpp初版包含参考工程中的全部怪物，包括Zircon专有类型，取代此前暂缓Zircon专有怪物的范围。类、race与外观是三个索引，不能相加作为怪物总数。下方1.45候选仅作定位线索；表中“—”表示尚无映射，不代表排除实现。先完整移植参考行为，再以宝藏版逐项比对修正。所有怪物实现完成后统一构建与验证，过程中不频繁启动测试。

完成标准包括行为触发、客户端消息与表现、目标失效、死亡及重新入场，并分别记录模型、图形与服务端验证。动画专项通过不等于整只特殊怪物完成。

| 旧版专用分支 | 已验证范围 | 下一项 |
| --- | --- | --- |
| CAnimalObject | 独立 AnimalMonster 无主动搜敌；离线移动/阻挡、受击追踪、显式属性近战扣血/550ms受击/致死、石化与清理专项通过；客户端割肉请求与尸体回归 | 其余状态及主人跟随世界接入、肉量初始化、原服属性与联网结算、EXE对照 |
| CStickObject | 隐藏出生、四格钻出、双轴远离钻回、固定位置近战；持久实体再次出现及死亡/换图清理图形专项 | 原服数据库间隔、伤害结算与联网时序 |
| CSandMobObject | 独立 SandMobMonster：整数生命阈值、三秒、双轴距离、提前返回及隐藏时间刷新模型通过 | 隐藏分支未直接调用 ComeOut 的原版行为、外观映射及世界接入 |
| CRonObject | 独立 RonMonster：五格触发、一格查询、逐对象攻击回调及空范围动作模型通过 | 世界查询与伤害接入、外观映射及 EXE 对照 |
| CScultureObject（两个 race） | 祖玛邻近及群体唤醒模型和图形专项 | 原服属性、伤害及 race 顺序 |
| CBeeQueen | 独立 BeeQueenMonster；延迟投递和回收模型；显式外观夹具生成真实子体、目标继承、追踪及清理专项通过 | 母体/子体数据库映射、子体实际 race 行为和 EXE 对照 |
| CScultureKingObject | 唤醒、召唤限额、实体生成和追踪专项 | 原服属性、伤害与联网时序 |
| CDualAxeObject（默认、6、3） | 三种外观射程、连射、恢复模型及图形回归 | 原服伤害与攻击间隔 |
| CLightingZombiObject | 独立 LightningZombieMonster：四格后退请求、近两格随机提前走基类、五格雷电、独立冷却及参考同格向下模型通过 | 基类移动、雷电穿透伤害、外观映射及 EXE 对照 |
| CRockManObject | 独立 RockManMonster：八格显现、1/8 秒重搜和失去目标隐藏模型通过 | 外观核对、世界接入及 EXE 对照 |
| CMinorNumaObject | 独立 MinorNumaMonster：死亡门控、1/8秒重搜及基类顺序 | 世界选敌与外观候选176核对、EXE对照 |
| CWhiteSkeleton | 独立 WhiteSkeletonMonster：首次出现方向 5、隐藏解除及 DWORD 时间模型 | 世界首次出现与 AI 联测、EXE 对照 |
| CElfMonster | 独立 ElfMonster：首次出现、主人/自身目标触发克隆、失败移除及死亡两秒模型 | 世界克隆属性与归属、EXE 对照 |
| CElfWarriorMonster | 独立 ElfWarriorMonster：无目标退回、成功才发钻回/换形、失败也移除且继续基类模型 | 世界形态替换、属性和消息标识、EXE 对照 |
| CSpitSpiderMonster | 独立 SpitSpiderMonster：显式方向喷吐、4×4扫描、闪避及连续减伤结算 | 近距离未初始化方向、转置索引和异常地图分支核对，世界接入及EXE对照 |
| COmaObject（三个 race） | 独立 OmaMonster：死亡/幽灵门控、1/8秒重搜及基类顺序 | 三个工厂配置、世界接入与EXE对照 |

2026-11-15：食人花显隐与固定位置近战按参考提交 `5140eef5e03dd9388851423171ccbc80c8556848` 实现。600 ms 行动间隔、1200 ms 攻击间隔是可配置离线夹具值。`monsters ai`、`monsters regression --monster-case plant-ai` 与完整 `monsters regression` 通过；包括显示删除后同一实体保留生命再次出现、死亡停止、显式删除和换图清理。日志 `client/build/monster-plant-ai.log`、`client/build/monster-regression-plant.log`。伤害与原服配置尚未验收。

怪物实现统一入口为 [Monster.cpp](../client/src/world/Monster.cpp)。先以参考工程建立初版，各怪物采用独立类型，再根据 EXE 证据逐项修正。相关头文件只声明接口和状态；Godot HUD 转发调用。文件分别编译纯 C++ 核心和 Godot 适配部分，纯模型测试无需启动 Godot。现有通用 AI 类型到逐怪物独立类型的整理仍待继续。

`CBeeQueen` 的“蜂后”称呼只是英文类名直译。原头文件 CP949 注释为 `비막원충`，`_MONSTER_BEE` 注释为 `비막충`，值分别为 124、125。`Operate` 用 125 索引数据库，工厂随后按该行 `wRaceIndex` 选择行为；缺少数据行时不能断定该子体的 race 或外观。世界专项显式配置 73/74 作为显示夹具，子体暂用普通追踪，不将其视为正式映射。证据见 [reference-bee-prototypes.json](../client/docs/evidence/monster-protocol/reference-bee-prototypes.json)。

## 原版客户端已定位的特殊分支

共 75 个外观。范围为已审计的近战、远程、受击、死亡和显隐分支，其他按外观分派的条件仍在复核。地址和完整验收字段见[行为文档](MonsterBehavior.md)与[原版审计](../client/docs/evidence/monster-protocol/special-audit.json)。

| shape | 名称 | 已定位分支 |
| --- | --- | --- |
| 2 | 变异章鱼 | 受击附加效果 |
| 3 | 沙鬼（参考） | 现身／钻回 |
| 4 | 石巨人 | 死亡附加效果、现身／钻回 |
| 5 | 冥界之门 | 近战附加效果、死亡附加效果 |
| 6 | 轻装士兵 | 死亡附加效果 |
| 7 | 蚂蚁治疗者 | 22 号远程 |
| 9 | 践踏者 | 近战附加效果 |
| 19 | 血色死灵法师 | 近战附加效果、22 号远程、死亡附加效果 |
| 31 | 食人花 | 现身／钻回 |
| 32 | 骷髅 | 死亡附加效果 |
| 33 | 掷斧骷髅 | 22 号远程、死亡附加效果 |
| 34 | 骷髅斧兵 | 死亡附加效果 |
| 35 | 骷髅战士 | 死亡附加效果 |
| 36 | 骷髅领主 | 死亡附加效果 |
| 37 | 洞穴蛆虫 | 近战附加效果 |
| 40 | 稻草人 | 死亡附加效果 |
| 43 | 沃玛喷火者 | 近战附加效果 |
| 45 | 沃玛教主 | 近战附加效果 |
| 46 | 棘背暗蜥 | 22 号远程 |
| 47 | 粪怪 | 近战附加效果 |
| 48 | 幽灵术士 | 死亡附加效果 |
| 49 | 幽灵法师 | 现身／钻回 |
| 56 | 白骨 | 现身／钻回 |
| 67 | 邪恶钳虫领主 | 近战附加效果、受击附加效果 |
| 75 | 楔蛾 | 近战附加效果 |
| 82 | 祖玛弓箭手 | 22 号远程 |
| 83 | 祖玛狂信徒 | 现身／钻回 |
| 84 | 祖玛卫士 | 现身／钻回 |
| 85 | 祖玛教主 | 近战附加效果、现身／钻回 |
| 86 | 弓箭守卫 | 22 号远程 |
| 89 | 神兽（参考） | 现身／钻回 |
| 90 | 神兽 | 近战附加效果、现身／钻回 |
| 94 | 沙鲨 | 近战附加效果 |
| 95 | 诅咒仙人掌 | 22 号远程 |
| 96 | 针蚁 | 22 号远程 |
| 97 | 风怒女巫 | 近战附加效果 |
| 98 | 诺玛教主+（参考） | 近战附加效果 |
| 99 | 幻影士兵 | 死亡附加效果 |
| 101 | 蝙蝠蜘蛛 | 死亡附加效果 |
| 104 | 赤月恶魔 | 受击附加效果 |
| 105 | 幼虫 | 死亡附加效果 |
| 113 | 森林守卫 | 近战附加效果 |
| 120 | 混沌使者破天 | 近战附加效果、死亡附加效果 |
| 124 | 玲花（参考） | 22 号远程，双效果专项通过 |
| 125 | 咸蛋／柴三郎等（参考） | 22 号远程，双效果专项通过 |
| 130 | 般若右护卫 | 近战附加效果、死亡附加效果 |
| 131 | 般若左护卫 | 近战附加效果、死亡附加效果 |
| 132 | 腐败食尸鬼 | 近战附加效果 |
| 138 | 腐烂食尸鬼 | 死亡附加效果 |
| 139 | 邪恶帝王 | 近战附加效果 |
| 140 | 骷髅队长 | 死亡附加效果 |
| 141 | 太斗巫妖 | 近战附加效果 |
| 142 | 骷髅士兵 | 死亡附加效果 |
| 143 | 骷髅刀兵 | 死亡附加效果 |
| 144 | 骷髅弓手 | 22 号远程、死亡附加效果 |
| 145 | 变异跳蚤 | 22 号远程 |
| 146 | 紫色跳蚤 | 22 号远程 |
| 150 | 利牙兽 | 近战附加效果、受击附加效果 |
| 152 | 怒龙神（参考） | 近战附加效果 |
| 156 | 食人狂信徒 | 22 号远程 |
| 159 | 火焰狮鹫 | 死亡附加效果 |
| 160 | 石狮鹫 | 死亡附加效果 |
| 161 | 变异队长 | 死亡附加效果、受击附加效果 |
| 164 | 真天魔神 | 近战附加效果、死亡附加效果 |
| 167 | 投石车（参考） | 22 号远程 |
| 169 | 弩车（参考） | 22 号远程 |
| 183 | 诺玛投石兵 | 22 号远程，定点落石与光源专项通过 |
| 184 | 诺玛大法师 | 死亡附加效果 |
| 185 | 诺玛皇家卫士 | 死亡附加效果 |
| 186 | 诺玛统帅 | 近战附加效果、死亡附加效果 |
| 530 | 沙巴克南门 | 现身／钻回 |
| 531 | 沙巴克北门 | 现身／钻回 |
| 532 | 沙巴克东门 | 现身／钻回 |
| 533 | 沙巴克西门 | 现身／钻回 |
| 538 | 诺玛城门（参考） | 现身／钻回 |

## LegendOfMir3_Src 专用创建分支

按 AddCreature 的 21 个非默认 race 分支逐项登记。shape 栏是原版外观或候选映射，候选仍需数据库或实机核对。
[ProcessMonster.cpp](https://github.com/tensafe/LegendOfMir3_Src/blob/5140eef5e03dd9388851423171ccbc80c8556848/LegendOfMir3_Server/GameSvr/ProcessMonster.cpp) · [ObjectMonster.cpp](https://github.com/tensafe/LegendOfMir3_Src/blob/5140eef5e03dd9388851423171ccbc80c8556848/LegendOfMir3_Server/GameSvr/ObjectMonster.cpp)

| race | 专用实现 | 参考机制 | 1.45 shape |
| --- | --- | --- | --- |
| `_ANIMAL_DEER` | `CAnimalObject` | 被动动物与割肉 | 21 |
| `_MONSTER_STICK` | `CStickObject` | 隐藏、近敌钻出与离开时钻回 | 31 |
| `_MONSTER_SANDMOB` | `CSandMobObject` | 隐藏与现身计时 | 待核对 |
| `_MONSTER_RON` | `CRonObject` | 专用运行与攻击 | 待核对 |
| `_MONSTER_SCULTURE1` | `CScultureObject` | 石化、邻近及群体唤醒 | 83／84，race 顺序待核 |
| `_MONSTER_SCULTURE2` | `CScultureObject` | 石化、邻近及群体唤醒 | 83／84，race 顺序待核 |
| `_MONSTER_BEEQUEEN` | `CBeeQueen` | 有目标时播放攻击，500 ms 后产蜂，排程时检查随从数小于 15 | 待核对 |
| `_MONSTER_SCULTUREKING` | `CScultureKingObject` | 石化唤醒、按生命条件召唤 | 85 |
| `_MONSTER_DUALAXE` | `CDualAxeObject` | 远程掷斧 | 33 |
| `_MONSTER_MAGUNGSA` | `CDualAxeObject(6)` | 远程弓矢连射 | 82 |
| `_MONSTER_THONEDARK` | `CDualAxeObject(3)` | 远程尖刺连射 | 46 |
| `_MONSTER_LIGHTZOMBI` | `CLightingZombiObject` | 闪电攻击 | 待核对 |
| `_MONSTER_ROCKMAN` | `CRockManObject` | 近敌出现、丢失目标后隐藏 | 3，语义候选 |
| `_MONSTER_MINORNUMA` | `CMinorNumaObject` | 小诺玛专用行为 | 176，名称候选 |
| `_MONSTER_WHITESKELETON` | `CWhiteSkeleton` | 首次出现与固定朝向 | 56 |
| `_MONSTER_ELFMONSTER` | `CElfMonster` | 召唤物出现、触敌换形态 | 89，形态对应待核 |
| `_MONSTER_ELFWARRIOR` | `CElfWarriorMonster` | 神兽战斗形态 | 90，形态对应待核 |
| `_MONSTER_SPITSPIDER` | `CSpitSpiderMonster` | 喷吐攻击 | 25，语义候选 |
| `_MONSTER_OMA` | `COmaObject` | 半兽人系列共享行为 | 22／23，映射待核 |
| `_MONSTER_OMAKNIGHT` | `COmaObject` | 半兽人系列共享行为 | 24，语义候选 |
| `_MONSTER_OMAWARRIOR` | `COmaObject` | 半兽人系列共享行为 | 27，语义候选 |

## Zircon 专用服务端实现

列出 Monsters 目录全部 104 个类文件，包含共享基类及辅助对象。AI 编号来自 GetMonster 中直接返回该类的分支。1.45 候选按已登记的同名同资源组匹配，破折号表示此次未建立外观映射。
[服务端工厂](../reference/Zircon/ServerLibrary/Models/MonsterObject.cs#L127) · [外观清单](Monsters.md)

| 类 | 工厂 AI | 1.45 外观候选 |
| --- | --- | --- |
| [Affliction](../reference/Zircon/ServerLibrary/Models/Monsters/Affliction.cs) | — | — |
| [ArachnidGrazer](../reference/Zircon/ServerLibrary/Models/Monsters/ArachnidGrazer.cs) | 17 | 106 凝视蜘蛛 |
| [ArchLichTaedu](../reference/Zircon/ServerLibrary/Models/Monsters/ArchLichTaedu.cs) | 43 | 141 太斗巫妖 |
| [BanyaLeftGuard](../reference/Zircon/ServerLibrary/Models/Monsters/BanyaLeftGuard.cs) | 38 | 131 般若左护卫 |
| [BanyoCaptain](../reference/Zircon/ServerLibrary/Models/Monsters/BanyoCaptain.cs) | 72 | — |
| [BanyoLordGuzak](../reference/Zircon/ServerLibrary/Models/Monsters/BanyoLordGuzak.cs) | 74 | — |
| [BanyoWarrior](../reference/Zircon/ServerLibrary/Models/Monsters/BanyoWarrior.cs) | 71、117 | — |
| [BlasterMutantFlea](../reference/Zircon/ServerLibrary/Models/Monsters/BlasterMutantFlea.cs) | 55 | 147 爆破变异跳蚤 |
| [BlockingObject](../reference/Zircon/ServerLibrary/Models/Monsters/BlockingObject.cs) | — | — |
| [CarnivorousPlant](../reference/Zircon/ServerLibrary/Models/Monsters/CarnivorousPlant.cs) | 5、125 | 31 食人花 |
| [CastleFlag](../reference/Zircon/ServerLibrary/Models/Monsters/CastleFlag.cs) | 1001 | — |
| [CastleGate](../reference/Zircon/ServerLibrary/Models/Monsters/CastleGate.cs) | 1002 | — |
| [CastleGuard](../reference/Zircon/ServerLibrary/Models/Monsters/CastleGuard.cs) | 1003 | — |
| [CastleLord](../reference/Zircon/ServerLibrary/Models/Monsters/CastleLord.cs) | — | — |
| [CastleObject](../reference/Zircon/ServerLibrary/Models/Monsters/CastleObject.cs) | — | — |
| [ChaosKnight](../reference/Zircon/ServerLibrary/Models/Monsters/ChaosKnight.cs) | 60 | 10 混沌骑士 |
| [ChristmasMonster](../reference/Zircon/ServerLibrary/Models/Monsters/ChristmasMonster.cs) | — | — |
| [Companion](../reference/Zircon/ServerLibrary/Models/Monsters/Companion.cs) | — | — |
| [CorrosivePoisonSpitter](../reference/Zircon/ServerLibrary/Models/Monsters/CorrosivePoisonSpitter.cs) | 56、57 | 93 腐蚀喷毒兽 |
| [CrimsonNecromancer](../reference/Zircon/ServerLibrary/Models/Monsters/CrimsonNecromancer.cs) | 59 | 19 血色死灵法师 |
| [DepartedMonster](../reference/Zircon/ServerLibrary/Models/Monsters/DepartedMonster.cs) | 75、76 | — |
| [Doll](../reference/Zircon/ServerLibrary/Models/Monsters/Doll.cs) | 128 | — |
| [DoomClaw](../reference/Zircon/ServerLibrary/Models/Monsters/DoomClaw.cs) | 120 | — |
| [DragonLord](../reference/Zircon/ServerLibrary/Models/Monsters/DragonLord.cs) | 102 | — |
| [DragonQueen](../reference/Zircon/ServerLibrary/Models/Monsters/DragonQueen.cs) | 101 | — |
| [EmperorSaWoo](../reference/Zircon/ServerLibrary/Models/Monsters/EmperorSaWoo.cs) | 41、85 | 139 邪恶帝王 |
| [EnragedArchLichTaedu](../reference/Zircon/ServerLibrary/Models/Monsters/EnragedArchLichTaedu.cs) | 91 | — |
| [EnragedLordNiJae](../reference/Zircon/ServerLibrary/Models/Monsters/EnragedLordNiJae.cs) | 77 | — |
| [EscortCommander](../reference/Zircon/ServerLibrary/Models/Monsters/EscortCommander.cs) | 93 | — |
| [EvilElephant](../reference/Zircon/ServerLibrary/Models/Monsters/EvilElephant.cs) | 25 | 158 邪恶巨象 |
| [FerociousIceTiger](../reference/Zircon/ServerLibrary/Models/Monsters/FerociousIceTiger.cs) | 104 | — |
| [FieryDancer](../reference/Zircon/ServerLibrary/Models/Monsters/FieryDancer.cs) | 94、95 | — |
| [FlameDemon](../reference/Zircon/ServerLibrary/Models/Monsters/FlameDemon.cs) | 83、86 | — |
| [FrostLordHwa](../reference/Zircon/ServerLibrary/Models/Monsters/FrostLordHwa.cs) | 70 | — |
| [GhostMage](../reference/Zircon/ServerLibrary/Models/Monsters/GhostMage.cs) | 10、27 | 49 幽灵法师 |
| [GhostSorcerer](../reference/Zircon/ServerLibrary/Models/Monsters/GhostSorcerer.cs) | 9 | 48 幽灵术士 |
| [GiantLizard](../reference/Zircon/ServerLibrary/Models/Monsters/GiantLizard.cs) | 33、49、50、54、79、92、105、106 | — |
| [GoruArcher](../reference/Zircon/ServerLibrary/Models/Monsters/GoruArcher.cs) | 89 | — |
| [GoruSpearman](../reference/Zircon/ServerLibrary/Models/Monsters/GoruSpearman.cs) | 88 | — |
| [Guard](../reference/Zircon/ServerLibrary/Models/Monsters/Guard.cs) | -1 | 26 守卫 |
| [HalloweenMonster](../reference/Zircon/ServerLibrary/Models/Monsters/HalloweenMonster.cs) | — | — |
| [HealerAnt](../reference/Zircon/ServerLibrary/Models/Monsters/HealerAnt.cs) | 12 | 7 蚂蚁治疗者 |
| [IcyGoddess](../reference/Zircon/ServerLibrary/Models/Monsters/IcyGoddess.cs) | 65 | 170 冰雪女神 |
| [IcySpiritGeneral](../reference/Zircon/ServerLibrary/Models/Monsters/IcySpiritGeneral.cs) | 67 | — |
| [IcySpiritWarrior](../reference/Zircon/ServerLibrary/Models/Monsters/IcySpiritWarrior.cs) | 66 | — |
| [InfernalSoldier](../reference/Zircon/ServerLibrary/Models/Monsters/InfernalSoldier.cs) | 103 | — |
| [JinamStoneGate](../reference/Zircon/ServerLibrary/Models/Monsters/JinamStoneGate.cs) | 69 | — |
| [JinchonDevil](../reference/Zircon/ServerLibrary/Models/Monsters/JinchonDevil.cs) | 78、127 | 164 真天魔神 |
| [JinhwanSpirit](../reference/Zircon/ServerLibrary/Models/Monsters/JinhwanSpirit.cs) | 99 | — |
| [Larva](../reference/Zircon/ServerLibrary/Models/Monsters/Larva.cs) | 18、123 | 105 幼虫 |
| [LordNiJae](../reference/Zircon/ServerLibrary/Models/Monsters/LordNiJae.cs) | 13 | 67 邪恶钳虫领主 |
| [MirrorImage](../reference/Zircon/ServerLibrary/Models/Monsters/MirrorImage.cs) | — | — |
| [MonasteryBoss](../reference/Zircon/ServerLibrary/Models/Monsters/MonasteryBoss.cs) | 126 | — |
| [Monkey](../reference/Zircon/ServerLibrary/Models/Monsters/Monkey.cs) | 23、24、64 | 154 猴子 |
| [MoonFeralWarrior](../reference/Zircon/ServerLibrary/Models/Monsters/MoonFeralWarrior.cs) | 81 | — |
| [MutantFlea](../reference/Zircon/ServerLibrary/Models/Monsters/MutantFlea.cs) | 51 | 145 变异跳蚤 |
| [NetherworldGate](../reference/Zircon/ServerLibrary/Models/Monsters/NetherworldGate.cs) | 30 | 5 冥界之门 |
| [NumaHighMage](../reference/Zircon/ServerLibrary/Models/Monsters/NumaHighMage.cs) | 62 | 184 诺玛大法师 |
| [NumaMage](../reference/Zircon/ServerLibrary/Models/Monsters/NumaMage.cs) | 26、36 | 13 诺玛法师 |
| [NumaStoneThrower](../reference/Zircon/ServerLibrary/Models/Monsters/NumaStoneThrower.cs) | 63 | 183 诺玛投石兵 |
| [NumaWarlord](../reference/Zircon/ServerLibrary/Models/Monsters/NumaWarlord.cs) | — | 186 诺玛统帅 |
| [OmaMage](../reference/Zircon/ServerLibrary/Models/Monsters/OmaMage.cs) | 118 | — |
| [OmaWarlord](../reference/Zircon/ServerLibrary/Models/Monsters/OmaWarlord.cs) | 87、90 | 27 半兽统领 |
| [OxFeralGeneral](../reference/Zircon/ServerLibrary/Models/Monsters/OxFeralGeneral.cs) | 82 | — |
| [PachontheChaosbringer](../reference/Zircon/ServerLibrary/Models/Monsters/PachontheChaosbringer.cs) | 61 | 120 混沌使者破天 |
| [PinkBat](../reference/Zircon/ServerLibrary/Models/Monsters/PinkBat.cs) | 121 | — |
| [Puppet](../reference/Zircon/ServerLibrary/Models/Monsters/Puppet.cs) | — | — |
| [QuartzTree](../reference/Zircon/ServerLibrary/Models/Monsters/QuartzTree.cs) | 124 | — |
| [QuartzTurtleSub](../reference/Zircon/ServerLibrary/Models/Monsters/QuartzTurtleSub.cs) | 122 | — |
| [QueenOfDawn](../reference/Zircon/ServerLibrary/Models/Monsters/QueenOfDawn.cs) | 96 | — |
| [RazorTusk](../reference/Zircon/ServerLibrary/Models/Monsters/RazorTusk.cs) | 45 | 150 利牙兽 |
| [RedMoonTheFallen](../reference/Zircon/ServerLibrary/Models/Monsters/RedMoonTheFallen.cs) | 19 | 104 赤月恶魔 |
| [SamaBlack](../reference/Zircon/ServerLibrary/Models/Monsters/SamaBlack.cs) | 112 | — |
| [SamaBlue](../reference/Zircon/ServerLibrary/Models/Monsters/SamaBlue.cs) | 113 | — |
| [SamaFireGuardian](../reference/Zircon/ServerLibrary/Models/Monsters/SamaFireGuardian.cs) | 107 | — |
| [SamaIceGuardian](../reference/Zircon/ServerLibrary/Models/Monsters/SamaIceGuardian.cs) | 108 | — |
| [SamaLightningGuardian](../reference/Zircon/ServerLibrary/Models/Monsters/SamaLightningGuardian.cs) | 109 | — |
| [SamaPhoenix](../reference/Zircon/ServerLibrary/Models/Monsters/SamaPhoenix.cs) | 111 | — |
| [SamaProphet](../reference/Zircon/ServerLibrary/Models/Monsters/SamaProphet.cs) | 115 | — |
| [SamaScorcer](../reference/Zircon/ServerLibrary/Models/Monsters/SamaScorcer.cs) | 116 | — |
| [SamaWhite](../reference/Zircon/ServerLibrary/Models/Monsters/SamaWhite.cs) | 114 | — |
| [SamaWindGuardian](../reference/Zircon/ServerLibrary/Models/Monsters/SamaWindGuardian.cs) | 110 | — |
| [Shinsu](../reference/Zircon/ServerLibrary/Models/Monsters/Shinsu.cs) | 53 | 89 神兽（参考） |
| [SkeletonAxeThrower](../reference/Zircon/ServerLibrary/Models/Monsters/SkeletonAxeThrower.cs) | 7、20、29、34 | 33 掷斧骷髅 |
| [SonicLizard](../reference/Zircon/ServerLibrary/Models/Monsters/SonicLizard.cs) | 31、48、97 | — |
| [SpittingSpider](../reference/Zircon/ServerLibrary/Models/Monsters/SpittingSpider.cs) | 6、14、42、46、47 | 25 吐液蜘蛛 |
| [Stomper](../reference/Zircon/ServerLibrary/Models/Monsters/Stomper.cs) | 58 | 9 践踏者 |
| [SunFeralWarrior](../reference/Zircon/ServerLibrary/Models/Monsters/SunFeralWarrior.cs) | 80 | — |
| [Terracotta](../reference/Zircon/ServerLibrary/Models/Monsters/Terracotta.cs) | 131、132 | — |
| [TerracottaBoss](../reference/Zircon/ServerLibrary/Models/Monsters/TerracottaBoss.cs) | 134 | — |
| [TerracottaSub](../reference/Zircon/ServerLibrary/Models/Monsters/TerracottaSub.cs) | 133 | — |
| [Tornado](../reference/Zircon/ServerLibrary/Models/Monsters/Tornado.cs) | — | — |
| [TreeMonster](../reference/Zircon/ServerLibrary/Models/Monsters/TreeMonster.cs) | 4 | — |
| [UmaKing](../reference/Zircon/ServerLibrary/Models/Monsters/UmaKing.cs) | 16 | 45 沃玛教主 |
| [UndeadSoul](../reference/Zircon/ServerLibrary/Models/Monsters/UndeadSoul.cs) | 130 | — |
| [VoraciousGhost](../reference/Zircon/ServerLibrary/Models/Monsters/VoraciousGhost.cs) | 11 | 50 贪食鬼 |
| [Warewolf](../reference/Zircon/ServerLibrary/Models/Monsters/Warewolf.cs) | 68 | — |
| [WedgeMothLarva](../reference/Zircon/ServerLibrary/Models/Monsters/WedgeMothLarva.cs) | 44 | 73 楔蛾幼虫 |
| [WhiteBone](../reference/Zircon/ServerLibrary/Models/Monsters/WhiteBone.cs) | 52 | 56 白骨 |
| [WindfurySorcerer](../reference/Zircon/ServerLibrary/Models/Monsters/WindfurySorcerer.cs) | 28 | — |
| [WingedHorror](../reference/Zircon/ServerLibrary/Models/Monsters/WingedHorror.cs) | 84 | — |
| [YumgonWitch](../reference/Zircon/ServerLibrary/Models/Monsters/YumgonWitch.cs) | 98、100 | — |
| [ZumaGuardian](../reference/Zircon/ServerLibrary/Models/Monsters/ZumaGuardian.cs) | 21 | 84 祖玛卫士 |
| [ZumaKing](../reference/Zircon/ServerLibrary/Models/Monsters/ZumaKing.cs) | 22 | 85 祖玛教主 |

## Zircon 客户端专用动作与效果

列出 SetAnimation、DrawBody、CreateProjectile 和 SetAction 中按 MonsterImage 分派的外观。基础外观装载和共享帧表见外观清单。

| MonsterImage | 专用入口 | 1.45 外观候选 |
| --- | --- | --- |
| AntHealer | CreateProjectile | 7 蚂蚁治疗者 |
| AntNeedler | CreateProjectile | 96 针蚁 |
| ArchLichTaedu | CreateProjectile、SetAction | 141 太斗巫妖 |
| ArcherGuard | CreateProjectile | 86 弓箭守卫 |
| BanyaLeftGuard | SetAction | 131 般若左护卫 |
| BanyaRightGuard | SetAction | 130 般若右护卫 |
| BlasterMutantFlea | CreateProjectile | 147 爆破变异跳蚤 |
| BlueBlasterMutantFlea | CreateProjectile | 148 蓝色爆破跳蚤 |
| BoneArcher | CreateProjectile、SetAction | 144 骷髅弓手 |
| BoneBladesman | SetAction | 143 骷髅刀兵 |
| BoneCaptain | SetAction | 140 骷髅队长 |
| BoneSoldier | SetAction | 142 骷髅士兵 |
| CannibalFanatic | CreateProjectile | 156 食人狂信徒 |
| CarnivorousPlant | SetAction | 31 食人花 |
| CaveMaggot | SetAction | 37 洞穴蛆虫 |
| ChiwooGeneral | CreateProjectile | — |
| CrazedLizard | CreateProjectile | — |
| CursedCactus | CreateProjectile | 95 诅咒仙人掌 |
| DecayingGhoul | SetAction | 132 腐败食尸鬼 |
| DragonLord | CreateProjectile | — |
| DragonQueen | SetAction | — |
| EmeraldDancer | SetAction | — |
| EmperorSaWoo | CreateProjectile、SetAction | 139 邪恶帝王 |
| EvilElephant | CreateProjectile | 158 邪恶巨象 |
| FerociousIceTiger | SetAction | — |
| FieryDancer | SetAction | — |
| FlameGriffin | CreateProjectile | 159 火焰狮鹫 |
| GhostKnight | CreateProjectile | — |
| GhostSorcerer | SetAction | 48 幽灵术士 |
| GiantLizard | CreateProjectile | — |
| IcyGoddess | CreateProjectile | 170 冰雪女神 |
| IcyRanger | CreateProjectile | — |
| IcySpiritGeneral | CreateProjectile | — |
| IcySpiritSoldier | SetAction | — |
| IcySpiritWarrior | CreateProjectile | — |
| InfernalSoldier | SetAction | — |
| JinchonDevil | SetAction | 164 真天魔神 |
| LordNiJae | SetAction | 67 邪恶钳虫领主 |
| MaWarlord | SetAction | — |
| MonasteryMon4 | SetAction | — |
| Monkey | CreateProjectile | 154 猴子 |
| MutantCaptain | CreateProjectile | 161 变异队长 |
| MutantFlea | CreateProjectile | 145 变异跳蚤 |
| NewMob1 | SetAction | — |
| NewMob10 | SetAction | — |
| NewMob3 | SetAction | — |
| NewMob4 | SetAction | — |
| NewMob5 | SetAction | — |
| NewMob6 | SetAction | — |
| NewMob8 | SetAction | — |
| NumaCavalry | CreateProjectile | 180 诺玛骑兵 |
| NumaRoyalGuard | CreateProjectile | 185 诺玛皇家卫士 |
| NumaStoneThrower | CreateProjectile | 183 诺玛投石兵 |
| NumaWarlord | SetAction | 186 诺玛统帅 |
| OYoungBeast | SetAction | — |
| PachonTheChaosBringer | SetAction | 120 混沌使者破天 |
| QueenOfDawn | SetAction | — |
| RazorTusk | CreateProjectile、SetAction | 150 利牙兽 |
| RedMoonTheFallen | CreateProjectile | 104 赤月恶魔 |
| RottingGhoul | SetAction | 138 腐烂食尸鬼 |
| SamaCursedFlameMage | CreateProjectile | — |
| SandShark | CreateProjectile | 94 沙鲨 |
| Scarecrow | SetAction | 40 稻草人 |
| Shinsu | SetAction | 89 神兽（参考） |
| Skeleton | SetAction | 32 骷髅 |
| SkeletonAxeThrower | CreateProjectile、SetAction | 33 掷斧骷髅 |
| SkeletonLord | SetAction | 36 骷髅领主 |
| SkeletonWarrior | SetAction | 35 骷髅战士 |
| SonicLizard | CreateProjectile | — |
| SpinedDarkLizard | CreateProjectile | 46 棘背暗蜥 |
| Stomper | SetAction | 9 践踏者 |
| StoneGolem | SetAction | 4 石巨人 |
| StoneGriffin | CreateProjectile | 160 石狮鹫 |
| UmaFlameThrower | SetAction | 43 沃玛喷火者 |
| UmaKing | SetAction | 45 沃玛教主 |
| Werewolf | SetAction | — |
| WindfurySorceress | CreateProjectile | 97 风怒女巫 |
| YumgonWitch | CreateProjectile | — |
| ZumaFanatic | SetAction | 83 祖玛狂信徒 |
| ZumaGuardian | SetAction | 84 祖玛卫士 |
| ZumaKing | SetAction | 85 祖玛教主 |
| ZumaSharpShooter | CreateProjectile | 82 祖玛弓箭手 |

## 全参考移植进度（待集中验证）

- Zircon WhiteBone：独立类型已移植延迟可见、召唤效果顺序及方向；世界对象登记仍由适配接口提供。
- Zircon Warewolf：独立类型已移植十秒施法冷却、五向八格冰线、侧格半伤、逐对象伤害抽样、575–1100ms排程及施法后基类处理。允许范围重叠重复命中，不擅自去重。
- Zircon WedgeMothLarva：独立类型已移植固定位置、同图射程、600ms延迟生成、当前目标继承、投递时随从上限及基类处理顺序；共享AttackClock保留减速/锁链/Neutralize调整。
- 本批只完成参考专用实现，尚未构建/测试、完整世界适配或宝藏版对照；不标记整只怪物完成。

- Zircon CarnivorousPlant：独立显隐门控、3秒检查、显现500ms格刷新/1秒动作延迟、隐藏满血清毒、3秒后移除及4秒隐藏间隔已移植。
- Zircon ZumaGuardian：隐藏免疫受击/中毒、三格触发/七格群体唤醒、目标继承和参考受击布尔参数转发已移植。
- Zircon TreeMonster / QuartzTree：被动不可移动/攻击、固定传入1伤害及参数转发；石英树低于四分之一HP尝试一次子Boss、每60秒清理异图或30格外随从并补充至10、五格随机出生与禁用恢复已移植。
- Zircon GhostMage / GhostSorcerer：五格触发显现、僵尸洞生命周期参数、400ms普通/随机属性攻击；同图八方向六格直线、主目标优先及其余格首合格对象命中已移植。
- 本批六类仍处于参考实现阶段，世界回调适配、构建与统一验证、宝藏版证据比对均未完成。

- Zircon VoraciousGhost：初始化0–3次复活，经验返回剩余复活次数对应的2幂分母供世界精确除法、仅零次可掉落；死亡后3–7秒复活，尸体截止时间附加配置时长，按死亡次数减半生命。
- Zircon UndeadSoul：500ms延迟显现、方向左下、出生效果与1秒动作等待独立移植，不混用WhiteBone的一秒显现。
- Zircon SkeletonAxeThrower：同图七格射程、六格内退避、八方向旋转尝试、恐惧概率/时长与400+距离×48ms弹道命中已移植。
- Zircon SonicLizard：同图三格八方向直线；移动前后均检查攻击，同格随机脱离；近格3/4近战否则直线，400ms到期才抽取伤害和当前属性。
- Zircon Stomper：普通攻击广播后，对周围一格每个目标独立抽伤，400ms排程。
- 本批五类未构建/测试，仍待全参考覆盖、世界适配与宝藏传奇3.exe最终对照。

- Zircon WindfurySorcerer：远程动作、攻击时钟和400ms单体命中已移植。
- Zircon NumaStoneThrower：继承掷斧骷髅移动/恐惧，目标周围两格逐个抽伤，所有目标按主目标距离计算弹道延迟。
- Zircon HealerAnt：友方受伤且无Heal才选中，直接ProperSearch；到期仅检查对象登记与死亡，使用届时自身治疗/上限添加永久、不可见、不暂停、1秒周期Heal。
- Zircon YumgonWitch / DragonQueen：十格射程，移动前随机攻击/移动后再次判定，近三格概率范围攻击；龙后范围属性跟随AttackElement且忽略setter，每15秒清理异图或15格外随从并补至10，十格随机出生，死亡原位生成DragonLord。
- Zircon GoruArcher：半血及以下一次逃离传送(7,12)、每秒1/7概率扫描目标周围八格传送，成功选格后标记下一击双倍；普通400ms单体攻击后清除Bonus。
- 六类本批未构建/测试，等待全参考覆盖后统一验证和宝藏版对照；共享世界回调仍需完整接入。

- Zircon SpittingSpider：独立两格八方向直线攻击，排除同格/异图，主目标直接排程、其他格首合格对象，400ms命中。保留与旧C++喷吐实现的版本差异。
- Zircon RazorTusk：同图三格范围，移动前2/3攻击机会、移动后再次判断；近格普通动作/远格单体投射，400ms到期读取伤害及属性。
- Zircon DepartedMonster：死亡基类之后随机五种属性/傀儡效果，两格范围逐对象排程，再为每个存活玩家请求4个随从；幼体在自身格生成。
- Zircon JinamStoneGate：绿色名称、移动/攻击/伤害/毒免疫，出生公告、二十分钟过期及SpawnInfo计数清理；三秒检查，目标副本地图缺失则等六十秒，倒序选择视野内非安全区活玩家并随机传入巢穴。
- 本批四类未构建/测试；世界回调、消息绑定与宝藏版对照仍待全部参考移植后集中完成。

- Zircon FerociousIceTiger：同图三格射程；每击先抽旋转随机数，旋转状态持续至位置变化；三格群攻400ms、玩家战士减20%，固定600/640ms动作/攻击时钟；非旋转向前三格中心范围半伤600ms。
- Zircon QueenOfDawn：每严格超过一分钟将全图合格目标随机传送；有SpawnInfo时每严格超过一秒1/10近身传送，记录但不消费Bonus；4/5单体、1/5自身两格群攻，均400ms。
- Zircon MutantFlea：七格射程，邻格近战400ms，否则远程400+距离×48ms。BlasterMutantFlea：五格射程，始终使用远程动作与距离延迟。
- 本批四类仍待完整世界适配和宝藏版对照；遵照要求未构建或测试。

- Zircon UmaKing：移动前概率远攻、移动后再次检查攻击，普通攻击400ms；远攻按1/3群体雷电球、2/3群体雷电术选择。
- Zircon WingedHorror：继承UmaKing追击，三种远攻等概率；十格闪电射线包含方向偏移0至8，保留首方向重复命中及侧格半伤。
- Zircon EmperorSaWoo：移动前1/3远攻、移动后1/5远攻；远攻1/3群体旋风，否则目标周围两格400ms逐对象伤害。
- Zircon OmaMage：继承掷斧骷髅追击，等概率雷电球/雷电术/一格雷电波；500ms命中，保留传入伤害为零时重新抽样规则。
- 群体魔法保留广播后填充共享目标/位置列表、空对象列表与未分配对象列表的随机差异，以及逐对象随机顺序。四类实现均集中Monster.cpp，尚未完成世界接入、构建测试和宝藏传奇3.exe最终比对。

- Zircon NumaMage / PinkBat：独立类型共用相同追击流程，移动前1/2尝试远攻、移动后1/5选择远攻；十格魔法范围。前者雷电500ms，后者幻影火球500+距离×48ms；远攻范围判断保留源码未额外检查同图/同格的行为。
- Zircon Monkey：同图非同格两格射程，移动前1/2攻击、移动后再次攻击；邻格普通动作、远格远程动作，均400ms。
- Zircon NumaHighMage：继承Monkey移动流程，改为同图非同格六格八方向射程；邻格单体、远格沿线逐格首合格对象，主目标所在格直接使用主目标，400ms命中。
- Zircon FieryDancer：十格射程与移动前后攻击；4/5单体，1/5主目标周围两格范围，逐对象抽伤，400ms命中。
- Zircon GiantLizard：七格远攻仅在严格超过远攻冷却且允许对玩家远攻时可用，否则退至一格；移动前后无随机攻击，远攻设置独立冷却，400ms到期读取DC和当前属性。
- 本批六类仍为参考实现，世界接入及宝藏版最终对照待完成；按要求未构建或运行测试。

- Zircon FlameDemon：八格内在施法计数耗尽或严格超过五秒时施放十二格火线；方向区间使用Min/Max，普通攻击先递减计数。火线逐有效中心格显示效果、中心全伤/侧格半伤，随后执行包含恐惧逃离的基类追击。
- Zircon CorrosivePoisonSpitter：目标超过三格且严格超过冷却时扫描目标周围八格；不要求CanAttack、不抽随机数，找到非空且Movements为null的格后传送并设置五秒冷却，找不到格不刷新冷却。
- Zircon GoruSpearman：继承喷吐蜘蛛的两格直线射程，但攻击覆盖为400ms单体；半血一次逃离(7,12)，每秒1/7近身传送成功选格后下一击双倍并清标记。受击沿用源码省略canStruck导致默认true的行为。
- Zircon EvilElephant：5/6普通单体、1/6自身周围一格范围，400ms到期抽DC及当前属性；范围攻击造成正伤害后才抽SC施加归属自身的绿毒，2秒一次共10次。毒的世界适配须使用当前怪物作为Owner。
- Zircon BanyaLeftGuard：移动前1/2远攻尝试、移动后1/5远攻，十格内火球为火属性，500+距离×48ms命中。
- 本批五类专用实现位于Monster.cpp，未构建或运行测试；仍待世界适配及宝藏传奇3.exe最终比对。射线共享函数现同时供WingedHorror和FlameDemon使用，保留两者效果位置列表差异。

- Zircon Larva：基类Process之后若活着且没有目标则SetHP(0)，攻击直接SetHP(0)；基类Die之后为自身Range内各对象独立抽DC，800ms延迟爆发。
- Zircon Terracotta：初始不可见，攻击/阻挡/被看见受Visible限制；同格先随机脱离，允许相位且目标四格外时1/45尝试隐藏，否则追击。两格内显现后立即返回，显现设置500ms格时钟/1秒动作等待；隐藏设置五秒冷却、900ms后不可见、1.2秒移动/动作等待。隐身时移动/动作间隔20ms，不恢复HP或清毒。
- Zircon TerracottaSub：启用CanPhase，继承相位流程；等概率普通扇形两圈三方向逐格攻击，或无技能类型魔法动作后攻击前方一格中心的一格范围，均400ms独立抽DC。
- Zircon TerracottaBoss：十一格总射程、两格八方向近战范围；远于近战时移动。攻击先更新时钟，远程或近战4/5触发偏转+1方向十二格暗线，保留LineAoE再次更新时钟；其余攻击前方三格中心的两格范围，使用MC而非DC，400ms命中。
- 本批四类仅完成参考专用逻辑，未构建或运行测试，完整世界接入与宝藏传奇3.exe对照仍待完成。

- Zircon ZumaKing：继承祖玛卫士隐藏/群体唤醒，关闭避让火墙；唤醒动作等待两秒。出生设为七阶段，每次Process至多下降一个血量阶段并请求SpawnMinions(4,8,Target)。移动前后远攻选择均1/5，远攻1/3十字火墙、2/3五向十二格火线。
- 共享MonsterFireWall：先广播并更新攻击时钟，再从自身二十格内目标随机选中心；依中心/上/下/左/右顺序排程500ms，保留原地图格与空格。投递时倒序清理该格FireWall及Tempest法术对象，然后生成归属施法者的15次、2秒周期火墙；世界适配需绑定Owner。
- Zircon QuartzTurtleSub：基类处理后死亡退出，否则每20秒请求补随从至10；随从生成位置为自身十格随机位置，不附加石英树的远离回收规则。
- 本批两类及火墙规则均集中Monster.cpp，尚待世界接入、统一构建验证和宝藏传奇3.exe对照，未启动测试。

- Zircon NumaWarlord：两格挥击，前方一格中心的一格范围，广播完整受击列表并逐对象排程400ms。距离大于四格优先传送，起手检查随机可用格、500ms后重新检查目标并重新随机选格，六秒冷却，传送两个效果标志均false。
- NumaWarlord拉拽：距离大于两格且不超过五格、十秒冷却到期时获取五格目标快照；空集合不消耗冷却。400ms后逐个重查可攻击/同图/距离，以max(1,DC/2)攻击，再重查死亡/同图并读取当前位置，拉向自身最多三格且停在一格外；不要求此次攻击造成正伤害。
- Zircon OmaWarlord：普通一格近战，半血及以下仅一次逃离(7,12)；超过一秒且可攻击、目标非邻格时1/7近身传送，下一次400ms单体攻击双倍并清标记；继承普通恐惧逃离流程及受击canStruck默认true的参数转发。
- 本批两类已加入Monster.cpp，世界回调接入、完整验证和宝藏版对照仍未完成；未启动测试。

- Zircon SunFeralWarrior：继承BanyaLeftGuard追击；第一次正伤害后先清CanSpawn再请求SpawnMinions(2,6,null)，不额外添加死亡过滤。十格内等概率火球（500+距离×48ms）或目标周围一格火风暴（500ms逐对象抽DC），均火属性。
- Zircon MoonFeralWarrior：继承NumaMage追击/雷电，额外每秒1/7概率近身传送；Bonus仅由400ms普通近战双倍并清除，雷电不消费。NumaMage攻击改为虚派发以保留子类覆盖。
- Zircon OxFeralGeneral：八格内施法计数耗尽或严格超过五秒时重设三次，等概率冰/火/雷/风五向十二格射线，之后执行普通基类追击；普通攻击递减计数。雷/风显示射线端点，冰/火逐有效中心格显示效果。
- Zircon BanyoWarrior：普通追击与每秒1/7近身传送，400ms单体攻击只有Bonus和DoubleDamage同时为真才双倍；无论是否倍伤，攻击后均清除Bonus。默认DoubleDamage为false。
- 本批四类实现集中Monster.cpp，未构建/运行测试；世界接入与宝藏版最终对照仍待完成。

- Zircon BanyoCaptain：可攻击且严格超过三秒施法间隔时，1/5净化、4/5雷电；雷电按Chebyshev距离不超过2/超过2/超过4分别为100/150/200固定伤害，500ms命中，不抽DC、不额外限制十格射程。施法后继续普通基类追击；近战先抽DC再1/10翻倍，400ms命中。
- MonsterPurification：500ms投递时要求目标存在、已登记且活着；友方倒序清除非寄生毒，计数仍含保留的寄生毒，并登记MagicWeakness/DefensiveBlow移除；敌方等级不高于施法者直接通过，否则42%通过，移除源码列出的23类增益。两种关系判断独立，最后统一倒序移除Buff，不新增距离或同图检查。
- 本批BanyoCaptain及净化规则集中Monster.cpp，世界状态/Buff枚举映射和延迟动作接入待完成；未构建或启动测试，最终行为仍需宝藏版证据对照。

- Zircon SamaBlack / SamaBlue / SamaWhite：独立类型，近战同图非同格两格；移动前后按RangeChance（默认5）选择远攻。各自等概率五种远攻：单体、五向十格射线、目标两格范围、目标三格守卫技能、目标三格强化技能。分别使用冰/雷/风属性；冰刃具有距离×48ms飞行时间，单体雷电/旋风仅500ms。
- 三类强化技能在广播前抽一次DC×2并供整组目标使用；如果结果为0，共享AttackAoE按源码逐对象重新抽DC。普通范围技能逐对象抽DC，500ms投递；射线保留中心全伤/侧格半伤和按步数延迟，不去重。
- Zircon SamaIceGuardian / SamaLightningGuardian / SamaWindGuardian：独立实现同类追击和前四种远攻，不包含上述强化技能。SamaFireGuardian使用AdamantineFireBall、ScortchedEarth、FireStorm和SamaGuardianFire；不把ScortchedEarth替换为MonsterScortchedEarth。
- 七类均未给远攻额外添加十格门控，射线长度与是否允许起手分开，保留参考行为。专用实现全部在Monster.cpp，世界接入、统一验证与宝藏版最终比对尚未完成；未启动构建或测试。

- Zircon SamaPhoenix：两格近战，按RangeChance选择远攻；五种火系攻击为AdamantineFireBall、五向十格ScortchedEarth、两格FireStorm、三格SamaGuardianFire、三格SamaPhoenixFire。最后一种预抽DC×2，保留零值重新抽样。
- Zircon SamaScorcer：三格近战；地图中存在BloodStone怪物即返回0伤害，不添加距离/死亡过滤。可攻击时先抽1/5概率，再检查目标是否超过五格，满足则传送目标到自身两格随机点，随后执行普通基类追击；不可攻击时不移动。
- Zircon SamaProphet：出生请求一个随从，在自身左下五格中心的四格范围生成；每十秒扫描地图，召回首个超过ViewRange-3的SamaSorcerer到自身左下两格中心的一格范围。地图中存在BloodStone或SamaSorcerer即免伤，转发受击时canStruck保持源码默认true。
- Prophet可攻击时先1/5尝试拉近五格外目标，再直接使用十五格目标中心范围魔法，不通过十二格InAttackRange门控或普通追击。三种效果分别对应火、冰、雷，SamaProphetWind明确使用Ice属性，均500ms逐目标抽DC。
- 本批三类参考实现集中Monster.cpp，未构建/运行测试；地图对象标志、传送、随从与世界接入及宝藏版对照尚未完成。

- Zircon Doll：构造后500ms显现，初始左下方向，不可移动/攻击；出生先基类、诅咒效果、1秒动作等待，再登记诅咒目标（允许重复及空目标，适配须保留列表语义）。基类Process之后分别检查施法者、诅咒目标、严格寿命到期，三个条件各自SetHP(0)。激活只登记一次，停用不执行操作；死亡尸体1秒，离场只移除首个匹配诅咒登记。
- Doll受击：仅当攻击者与诅咒目标同图且十格内时转发原伤害及布尔参数，元素/攻击者由适配回调保留，自身恒返回0。玩家/怪物信息包名称为目标名字加“ Doll”，其他对象不返回信息包。
- Zircon BlockingObject：不可移动/攻击/行走，不漫游或搜索；可见时阻挡状态取父对象，受击原样转发父对象。死亡只隐藏，不调用基类Die；无地图时也先更新Visible，有地图时显现先AddAllObjects再Show，名称为单个空格。
- 本批两类专用实现集中Monster.cpp，诅咒全局列表、父对象引用和信息包适配仍待接入；未构建或运行测试，宝藏版对照未完成。

- Zircon ChristmasMonster：不可移动/攻击，ExtraExperienceRate初始10；受击向基类固定传入1并禁用暴击，其余参数保留。名字默认AliceBlue，Shock优先Peru，之后才检查Rage红色。
- ChristmasMonster死亡：先1/15倒序召集十五格随机点（排除宠物、卫士、圣诞怪、死亡、无法移动、有目标及300级以上），再用经验归属者的十八格目标列表筛选非Boss活怪；已有不同经验归属者或圣诞怪跳过。先提升经验倍率到两者最大、设置归属，再SetHP(0)，最后才调用自身基类Die。
- Zircon Tornado：不阻挡、不可推动；出生两秒动作等待并Show，寿命严格超过VisibleTime时Die，尸体700ms。每秒且有玩家看见时漫游，当前格有其他阻挡对象且可攻击则先对一格范围排程100ms伤害，再4/5向当前方向走、1/5随机转向。
- Tornado攻击过滤：排除自身、未登记、死亡、不可见、卫士/城主及物品/NPC/法术；玩家额外排除GM及双方安全区，怪物无需安全区判断。未知对象类型按参考抛出未实现错误；伤害全部排程后才更新攻击时钟。
- 本批两类专用实现集中Monster.cpp；活动怪物地图/经验回调、Tornado世界过滤和统一验证仍待完成，未构建或运行测试，最终以宝藏版比对为准。

- Zircon NetherworldGate：独立于JinamStoneGate，初始朝上、绿色名称、伤害及毒免疫、禁止移动/攻击/搜索。出生公告及二十分钟寿命；每三秒解析同Instance/InstanceSequence的MysteryShip地图，缺图改等六十秒，区域点列表仅null时创建。倒序传送视野内非安全区活玩家及怪物，排除自身和Guard；到期先减出生计数、关闭公告、清SpawnInfo再Despawn。激活只登记一次，DeActivate为空。
- Zircon Affliction：攻击SetHP(0)，死亡先基类Die再为自身一格目标逐个抽DC并排程800ms；不套用Larva失去目标时自毁的规则。
- Zircon InfernalSoldier：出生基类后设置两秒动作等待并Show；七格远攻受严格冷却和CanPvPRange控制，否则仅一格。移动前后均检查攻击，近远动作不同，远攻刷新独立RangeTime；400ms到期才读取DC及当前属性，近远投递相同。
- 本批三类专用实现集中Monster.cpp，世界接入、统一构建验证和宝藏传奇3.exe比对仍未完成；未启动测试。

- Zircon IcySpiritWarrior：八格同图非同格射程，移动前后均尝试攻击；等概率远程动作后400ms Function或目标一格MonsterIceStorm（冰属性500ms）。源码自身未覆盖Function投递，MonsterObject及MapObject基类也不处理Function，因此该分支仅有动作，暂不补造伤害，待宝藏版裁定。
- Zircon IcySpiritGeneral：同样起手/移动流程，但400ms Function会在到期抽DC，以Dark攻击；结果大于0后扣除min(目标当前MP,伤害结果)，使用伤害后的当前MP。冰风暴分支不套用扣MP逻辑。
- Zircon IcyGoddess：继承CarnivorousPlant显隐/恢复机制，八格射程；可见时移动前后尝试攻击，邻格400ms普通攻击，远格400+距离×48ms远程攻击，起手抽DC及AttackElement，不强制冰属性。
- 本批三类参考实现集中Monster.cpp，未构建或运行测试；世界显隐时钟、Function队列和宝藏版最终对照尚待完成。

- Zircon CrimsonNecromancer：不先检查Target；可攻击且严格超过十秒冷却时先刷新冷却，再查询自身三格对象。非空时朝首对象、广播无目标列表远攻、更新攻击时钟，立即为所有目标添加十秒MagicWeakness（不可见、不暂停、零tick、无Stats），再执行普通追击；空集合也消耗减益冷却。
- Zircon DoomClaw：出生两秒等待/Show、推移免疫、十格外攻击者无伤害。AI死亡退出，否则先恢复、检查可攻击，查询右五格/下五格/右下五格中心列表并更新时钟；1/20全视野波浪，否则按三个列表数量加权选招。
- DoomClaw左右区域只决定选择，钳击/横扫使用middleTargets；每侧9/10钳击、1/10横扫。所有招式逐目标抽DC，玩家战士/法师/道士/刺客分别减40%/30%/60%/20%，400ms命中；波浪排程后立即向右下推15，右扫向左下推5，左扫向右上推5。喷吐先排程再广播空Locations，其他招式先广播；各招式再次更新时钟。无目标仍保留Random.Next(0)语义（返回0）。
- 本批两类专用逻辑集中Monster.cpp，未构建或运行测试；职业/消息/地图适配与宝藏版最终比对尚未完成。

- Zircon ChaosKnight：同图非同格一格近战范围且可攻击、严格超过CastTime时设置二十秒冷却并释放毒云，随后继续普通基类追击。毒云先朝目标广播空目标/位置列表并更新攻击时钟，再立即遍历自身两格地图格生成法术；仅中心格可见，DisplayLocation均为自身位置，TickCount=1、TickFrequency=20秒、Power=20、Owner为自身。不附加延迟或清理既有毒云。
- Zircon RedMoonTheFallen：禁止移动、初始向上；攻击射程只判断同图，不排除同格。攻击不重设方向，先广播共享空目标列表再更新时钟，之后填充自身四十格目标ID并逐对象抽DC，500ms命中。
- 两类实现及毒云生成规则集中Monster.cpp，法术实体/消息世界适配和宝藏版最终比对仍待完成；未构建或启动测试。
