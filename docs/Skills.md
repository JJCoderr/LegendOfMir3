# MIR3 1.45 技能清单

对应 [Features.md 的 C22/C23](../Features.md)。以宝藏客户端安装包中的 1.45 分析样本为基准。本轮在该样本的 IDA 数据库复核了 0x418D80 的施法分派及 0x44FCF0 的技能记录查找入口。

- 收录 83 个原版施法编号，逐项核对了 0x418DBA 和 0x418E0F 两张跳转表的目标地址。7 号攻杀剑术由近战分支另行确认，详见下文。3、4 仍待核对原版战斗行为。
- 中文名称沿用安装包 Magic.exp 的段标题，150 为行为暂名。28 号已按早期客户端的 SHOWHP 与相同的 870–890、890–900 帧段定位，中文名取自 King_Magic。其他未命名编号仍待同资源或同机制的证据。英文栏优先使用可对应的 Zircon 枚举名称，其他已知标识取早期客户端。缺少参考英文名称时留空。客户端运行时从服务器技能记录取得显示名。
- MIcon 是按 2×(ID−1) 计算的索引。效果来自 MInfo 的 SPELL、MAGIC、EXPLOSION 段，按效果 ID 查找。帧段为从 0 开始的方向 0 索引，区间右端不含。破折号表示该段无记录，[0,0) 表示资源表明确登记了零帧。法术可学习性由服务器数据决定。

## 技能总表

其他版本与待确认条目在「客户端版本」列标明来源。与 1.45 对应的 Zircon 技能并入同一行，保留 Zircon ID 和原始英文标识。中文名称采用能与技能机制对应的国服官方名称。候选原版编号与新版特效证据放在核对依据中。

### 1.45 技能

| 1.45 技能 ID | 中文名称 | 英文名称 / 标识 | Zircon ID | MIcon 索引 | SPELL 起手 | MAGIC 施放效果 | EXPLOSION 命中效果 | 核对依据 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 火球术 | FireBall | 201 | 0 | Magic[1820,1828) / 100ms | Magic[420,425) / 20ms | Magic[580,590) / 100ms | 动作 2。Magic:1820/580 |
| 2 | 治愈术 | Heal | 300 | 2 | Magic[600,610) / 100ms | Magic[610,620) / 100ms | — | 动作 3。Magic:610 |
| 5 | 大火球 | fireball2 | — | 8 | Magic[1560,1569) / 100ms | Magic[1640,1646) / 10ms | Magic[1800,1810) / 100ms | 动作 2 |
| 6 | 施毒术 | PoisonDust | 302 | 10 | Magic[60,70) / 100ms | Magic[70,88) / 100ms | — | 动作 3。Magic:60 |
| 7 | 攻杀剑术 | Slaying | 102 | 12 | Magic[1350,1356) / 100ms | — | — | 攻击触发，CM_POWERHIT 3018。原版攻击请求 3018 与回包 18 均加载效果 7，Zircon 使用同一 Magic 1350 帧段。 |
| 8 | 抗拒火环 | Repulsion | 205 | 14 | Magic[90,98) / 100ms | — | — | 动作 3。Magic:90 |
| 9 | 地域火 | ScortchedEarth | 212 | 16 | Magic[1820,1828) / 100ms | Magic[1900,1960) / 50ms | — | 动作 2。Magic:1820/1900 |
| 10 | 疾光电影 | LightningBeam | 213 | 18 | Magic[1970,1978) / 80ms | MagicEx[1180,1186) / 60ms | — | 动作 2。Magic:1970+MagicEx:1180 |
| 11 | 雷电术 | ThunderBolt | 209 | 20 | Magic[1430,1442) / 100ms | Magic[1450,1455) / 100ms | — | 动作 3。Magic:1430，单体雷击 |
| 12 | 刺杀剑术 | Thrusting | 103 | 22 | Magic[320,326) / 100ms | — | — | 直接请求。隔格直线剑击 |
| 13 | 灵魂火符 | ExplosiveTalisman | 303 | 24 | Magic[2080,2086) / 100ms | Magic[980,983) / 100ms | Magic[1140,1150) / 100ms | 动作 2。Magic:2080/1140 |
| 14 | 幽灵盾 | MagicResistance | 306 | 26 | Magic[2080,2086) / 100ms | Magic[980,983) / 100ms | Magic[200,210) / 130ms | 动作 2。Magic:2080/200，魔防 |
| 15 | 神圣战甲术 | Resilience | 309 | 28 | Magic[2080,2086) / 100ms | Magic[980,983) / 100ms | Magic[170,180) / 130ms | 动作 2。Magic:2080/170，物防 |
| 16 | 困魔咒 | TrapOctagon | 310 | 30 | Magic[630,640) / 100ms | Magic[640,650) / 100ms | — | 动作 3。Magic:630，困住目标 |
| 17 | 召唤骷髅 | SummonSkeleton | 332 | 32 | Magic[740,750) / 100ms | Magic[750,760) / 155ms | — | 动作 3。普通骷髅召唤 |
| 18 | 隐身术 | Invisibility | 305 | 34 | Magic[810,820) / 100ms | — | — | 动作 3。Magic:810，自身隐身 |
| 19 | 集体隐身术 | MassInvisibility | 307 | 36 | Magic[2080,2086) / 100ms | Magic[980,983) / 100ms | Magic[820,830) / 100ms | 动作 2。Magic:2080/820 |
| 20 | 诱惑之光 | ElectricShock | 206 | 38 | Magic[0,9) / 100ms | Magic[10,30) / 50ms | — | 动作 3。Magic:0/10，驯化 |
| 21 | 瞬间移动 | Teleportation | 207 | 40 | Magic[2360,2370) / 100ms | — | — | 动作 3。法师地图内传送 |
| 22 | 火墙 | FireWall | 216 | 42 | Magic[910,920) / 100ms | Magic[920,925) / 150ms | — | 动作 3。Magic:910，地面火墙 |
| 23 | 爆裂火焰 | FireStorm | 220 | 44 | Magic[940,950) / 100ms | Magic[950,960) / 100ms | — | 动作 3。Magic:940/950 |
| 24 | 地狱雷光 | LightningWave | 221 | 46 | Magic[1430,1442) / 100ms | MagicEx[980,988) / 100ms | — | 动作 3。Magic:1430+MagicEx:980 |
| 25 | 半月弯刀 | HalfMoon | 104 | 48 | Magic[230,236) / 100ms | — | — | 直接请求。Magic:230，弧形剑击 |
| 26 | 烈火剑法 | FlamingSword | 106 | 50 | Magic[1470,1476) / 100ms | — | — | 直接请求。Magic:1470 |
| 27 | 野蛮冲撞 | ShoulderDash | 105 | 52 | — | — | — | 动作 23。冲撞位移，原版动作23 |
| 28 | 心灵启示（参考） | showhp | — | 54 | Magic[870,890) / 100ms | Magic[890,900) / 100ms | — | 动作 3 |
| 29 | 群体治愈术 | MassHeal | 313 | 56 | Magic[660,670) / 100ms | Magic[670,680) / 100ms | — | 动作 3。Magic:660/670 |
| 30 | 召唤神兽 | SummonShinsu | 333 | 58 | Magic[2080,2086) / 100ms | Magic[2590,2609) / 100ms | — | 动作 2。Magic:2590，神兽召唤 |
| 31 | 魔法盾 | MagicShield | 219 | 60 | Magic[830,849) / 100ms | Magic[840,850) / 100ms | — | 动作 3。Magic:830，自身护盾 |
| 32 | 圣言术 | ExpelUndead | 217 | 62 | Magic[130,140) / 100ms | Magic[140,160) / 75ms | — | 动作 3。Magic:130/140 |
| 33 | 冰咆哮 | IceStorm | 222 | 64 | Magic[770,780) / 100ms | Magic[780,790) / 100ms | — | 动作 3。Magic:770/780 |
| 34 | 莲月剑法 | BladeStorm | 108 | 66 | MagicEx[1780,1790) / 100ms | — | — | 直接请求。MagicEx:1780 |
| 35 | 翔空剑法 | DragonRise | 107 | 68 | Magic[2180,2190) / 100ms | — | — | 直接请求。Magic:2185落在原版2180–2190段 |
| 36 | 空拳刀法 | spacemove3 | — | 70 | — | — | — | 动作 18 |
| 37 | 月魂断玉 | EvilSlayer | 304 | 72 | Magic[3250,3256) / 100ms | Magic[3330,3336) / 50ms | Magic[3340,3350) / 100ms | 动作 2。Magic:3250/3340 |
| 38 | 月魂灵波 | GreaterEvilSlayer | 308 | 74 | Magic[3360,3366) / 100ms | Magic[3440,3446) / 20ms | Magic[3450,3460) / 100ms | 动作 2。Magic:3360/3450 |
| 39 | 冰月神掌 | IceBolt | 203 | 76 | Magic[2620,2626) / 100ms | Magic[2700,2703) / 50ms | Magic[2860,2870) / 100ms | 动作 2。Magic:2860，冰系单体弹道 |
| 40 | 冰月震天 | IceBlades | 210 | 78 | Magic[2880,2886) / 100ms | Magic[2960,2966) / 20ms | Magic[2970,2980) / 100ms | 动作 2。Magic:2880/2970 |
| 41 | 霹雳掌 | LightningBall | 202 | 80 | Magic[2990,2996) / 100ms | Magic[3070,3076) / 50ms | Magic[3230,3240) / 100ms | 动作 2。Magic:2990/3230 |
| 47 | 未命名 | — | — | 92 | 库91[940,950) / 100ms | MagicEx[1060,1080) / 50ms | — | 动作 3 |
| 48 | 未命名 | — | — | 94 | MagicEx[760,770) / 100ms | MagicEx[770,776) / 100ms | — | 动作 3 |
| 53 | 冰沙掌 | FrozenEarth | 214 | 104 | MagicEx[0,10) / 150ms | MagicEx[90,130) / 20ms | — | 动作 2。MagicEx:0/90，冰系地面波 |
| 63 | 未命名 | — | — | 124 | MagicEx[950,960) / 100ms | MagicEx[960,966) / 100ms | — | 动作 3 |
| 64 | 未命名 | — | — | 126 | — | — | — | 动作 3 |
| 66 | 未命名 | — | — | 130 | MagicEx[1000,1008) / 100ms | MagicEx[1010,1020) / 100ms | — | 动作 3 |
| 67 | 风掌 | GustBlast | 204 | 132 | MagicEx[350,357) / 100ms | MagicEx[430,435) / 80ms | MagicEx[590,600) / 100ms | 动作 2。MagicEx:350/590 |
| 72 | 龙卷风 | DragonTornado | 223 | 142 | MagicEx[1030,1040) / 100ms | MagicEx[1040,1056) / 80ms | — | 动作 3。MagicEx:1030/1040 |
| 73 | 风震天 | Cyclone | 211 | 144 | MagicEx[1970,1980) / 100ms | MagicEx[1990,1995) / 80ms | MagicEx[2000,2010) / 100ms | 动作 3。MagicEx:1970/1990/2000 |
| 74 | 击风 | — | — | 146 | MagicEx[1970,1980) / 100ms | MagicEx[1980,1995) / 80ms | MagicEx[2000,2010) / 100ms | 动作 3 |
| 77 | 回生术 | Resurrection | 315 | 152 | MagicEx[310,320) / 100ms | MagicEx[320,330) / 130ms | — | 动作 3。MagicEx:310/320 |
| 78 | 未命名 | — | — | 154 | MagicEx[170,174) / 100ms | — | — | 动作 3 |
| 85 | 未命名 | — | — | 168 | Magic[2080,2086) / 100ms | Magic[980,983) / 100ms | MagicEx[610,620) / 130ms | 动作 2 |
| 86 | 未命名 | — | — | 170 | Magic[2080,2086) / 100ms | Magic[980,983) / 100ms | MagicEx[640,650) / 130ms | 动作 2 |
| 87 | 未命名 | — | — | 172 | Magic[2080,2086) / 100ms | Magic[980,983) / 100ms | MagicEx[670,680) / 130ms | 动作 2 |
| 88 | 未命名 | — | — | 174 | Magic[2080,2086) / 100ms | Magic[980,983) / 100ms | MagicEx[700,710) / 130ms | 动作 2 |
| 89 | 强魔震法 | ElementalSuperiority | 312 | 176 | Magic[2080,2086) / 100ms | Magic[980,983) / 100ms | MagicEx[1870,1880) / 130ms | 动作 2。Magic:2080+MagicEx:1870 |
| 90 | 未命名 | — | — | 178 | Magic[2080,2086) / 100ms | Magic[980,983) / 100ms | MagicEx[190,200) / 130ms | 动作 2 |
| 91 | 未命名 | — | — | 180 | Magic[2080,2086) / 100ms | Magic[980,983) / 100ms | MagicEx[220,230) / 130ms | 动作 2 |
| 92 | 未命名 | — | — | 182 | Magic[2080,2086) / 100ms | Magic[980,983) / 100ms | MagicEx[250,260) / 130ms | 动作 2 |
| 93 | 未命名 | — | — | 184 | Magic[2080,2086) / 100ms | Magic[980,983) / 100ms | MagicEx[280,290) / 130ms | 动作 2 |
| 94 | 猛虎强势 | — | — | 186 | Magic[2080,2086) / 100ms | Magic[980,983) / 100ms | MagicEx[140,150) / 130ms | 动作 2 |
| 95 | 未命名 | — | — | 188 | Magic[2080,2086) / 100ms | Magic[980,983) / 100ms | MagicEx[140,160) / 100ms | 动作 2 |
| 96 | 未命名 | — | — | 190 | Magic[2080,2086) / 100ms | Magic[980,983) / 100ms | MagicEx[140,160) / 100ms | 动作 2 |
| 97 | 未命名 | — | — | 192 | Magic[2080,2086) / 100ms | Magic[980,983) / 100ms | MagicEx[140,160) / 100ms | 动作 2 |
| 98 | 未命名 | — | — | 194 | Magic[2080,2086) / 100ms | Magic[980,983) / 100ms | MagicEx[140,160) / 100ms | 动作 2 |
| 99 | 未命名 | — | — | 196 | Magic[2080,2086) / 100ms | Magic[980,983) / 100ms | MagicEx[140,160) / 100ms | 动作 2 |
| 100 | 未命名 | — | — | 198 | Magic[2080,2086) / 100ms | Magic[980,983) / 100ms | MagicEx[140,160) / 100ms | 动作 2 |
| 101 | 未命名 | — | — | 200 | Magic[2080,2086) / 100ms | Magic[980,983) / 100ms | MagicEx[140,160) / 100ms | 动作 2 |
| 102 | 铁布衫 | Defiance | 111 | 202 | — | MagicEx2[40,50) / 80ms | — | 直接请求并等待确认。MagicEx2:40，自身防御 |
| 103 | 十方斩 | — | — | 204 | MagicEx2[520,526) / 100ms | — | — | 直接请求 |
| 104 | 异形换位 | — | — | 206 | Magic[2360,2370) / 50ms | — | — | 动作 3 |
| 105 | 超强召唤骷髅 | SummonJinSkeleton | 334 | 208 | Magic[740,750) / 100ms | Magic[750,760) / 155ms | — | 动作 3。强化骷髅召唤 |
| 106 | 破血狂杀 | Might | 113 | 210 | — | MagicEx2[60,70) / 80ms | — | 直接请求并等待确认。MagicEx2:60，战士自身增益 |
| 107 | 乾坤大挪移 | Interchange | 110 | 212 | MagicEx2[0,10) / 50ms | MagicEx2[0,10) / 50ms | — | 动作 3。MagicEx2:0 |
| 108 | 斗转星移 | Beckon | 112 | 214 | MagicEx2[580,590) / 100ms | — | — | 动作 2。MagicEx2:580 |
| 110 | 魄冰刺 | — | — | 218 | MagicEx[0,10) / 150ms | MagicEx[90,130) / 20ms | — | 动作 2 |
| 111 | 怒神霹雳 | ChainLightning | 225 | 220 | Magic[1430,1442) / 100ms | MagicEx2[470,480) / 100ms | — | 动作 3。Magic:1430+MagicEx2:470 |
| 112 | 凝血离魂 | — | — | 222 | MagicEx2[80,90) / 100ms | MagicEx2[80,90) / 100ms | — | 直接请求并等待确认 |
| 113 | 焰天火雨 | — | — | 224 | Magic[1560,1569) / 100ms | Magic[1640,1646) / 10ms | Magic[1800,1810) / 100ms | 动作 2 |
| 120 | 云寂术 | Purification | 316 | 238 | MagicEx2[220,230) / 100ms | MagicEx2[230,240) / 100ms | — | 动作 3。MagicEx2:220/230 |
| 121 | 妙影无踪 | Transparency | 317 | 240 | MagicEx2[430,437) / 100ms | — | — | 动作 3。MagicEx2:430 |
| 122 | 阴阳法环 | CelestialLight | 318 | 242 | MagicEx2[280,288) / 100ms | MagicEx2[290,299) / 100ms | — | 动作 3。MagicEx2:280/290 |
| 123 | 移花接玉 | StrengthOfFaith | 335 | 244 | MagicEx2[360,370) / 100ms | MagicEx2[370,380) / 100ms | — | 动作 3。MagicEx2:360/370 |
| 150 | 环绕火焰（行为暂名） | — | — | 298 | Magic[1820,1828) / 100ms | Magic[0,0) / 0ms | Magic[0,0) / 0ms | 动作 2 |

### 其他版本新增技能

| 1.45 技能 ID | 中文名称 | 英文名称 / 标识 | Zircon ID | MIcon 索引 | SPELL 起手 | MAGIC 施放效果 | EXPLOSION 命中效果 | 核对依据 | 客户端版本 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| — | 柳舞（译名） | WillowDance | 401 | — | — | — | — | 新增 | Zircon |
| — | 藤舞（译名） | VineTreeDance | 402 | — | — | — | — | 新增 | Zircon |
| — | 基本刺杀术 | Discipline | 403 | — | — | — | — | 新增。国服名称依据：刺客被动命中，与 Zircon Discipline.GetPassiveStats 对应。 | Zircon |
| — | 毒云（译名） | PoisonousCloud | 404 | — | — | — | — | 新增 | Zircon |
| — | 盛放（译名） | FullBloom | 405 | — | — | — | — | 新增 | Zircon |
| — | 潜行术 | Cloak | 406 | — | — | — | — | 新增 | Zircon |
| — | 白莲（译名） | WhiteLotus | 407 | — | — | — | — | 新增 | Zircon |
| — | 满月灾厄（译名） | CalamityOfFullMoon | 408 | — | — | — | — | 新增 | Zircon |
| — | 幽魂之握（译名） | WraithGrip | 409 | — | — | — | — | 新增 | Zircon |
| — | 红莲（译名） | RedLotus | 410 | — | — | — | — | 新增 | Zircon |
| — | 地狱烈焰（译名） | HellFire | 411 | — | — | — | — | 新增 | Zircon |
| — | 血誓（译名） | PledgeOfBlood | 412 | — | — | — | — | 新增 | Zircon |
| — | 裂爪（译名） | Rake | 413 | — | — | — | — | 新增 | Zircon |
| — | 野蔷薇（译名） | SweetBrier | 414 | — | — | — | — | 新增 | Zircon |
| — | 召唤傀儡（译名） | SummonPuppet | 415 | — | — | — | — | 新增 | Zircon |
| — | 业报（译名） | Karma | 416 | — | — | — | — | 新增 | Zircon |
| — | 亡者之触（译名） | TouchOfTheDeparted | 417 | — | — | — | — | 新增 | Zircon |
| — | 残月（译名） | WaningMoon | 418 | — | — | — | — | 新增 | Zircon |
| — | 鬼步（译名） | GhostWalk | 419 | — | — | — | — | 新增 | Zircon |
| — | 元素傀儡（译名） | ElementalPuppet | 420 | — | — | — | — | 新增 | Zircon |
| — | 回春（译名） | Rejuvenation | 421 | — | — | — | — | 新增 | Zircon |
| — | 决意（译名） | Resolution | 422 | — | — | — | — | 新增 | Zircon |
| — | 四季轮转（译名） | ChangeOfSeasons | 423 | — | — | — | — | 新增 | Zircon |
| — | 解放（译名） | Release | 424 | — | — | — | — | 新增 | Zircon |
| — | 焰溅（译名） | FlameSplash | 425 | — | — | — | — | 新增 | Zircon |
| — | 血花（译名） | BloodyFlower | 426 | — | — | — | — | 新增 | Zircon |
| — | 新生（译名） | TheNewBeginning | 427 | — | — | — | — | 新增 | Zircon |
| — | 燕舞（译名） | DanceOfSwallow | 428 | — | — | — | — | 新增 | Zircon |
| — | 暗转（译名） | DarkConversion | 429 | — | — | — | — | 新增 | Zircon |
| — | 龙拒（译名） | DragonRepulse | 430 | — | — | — | — | 新增 | Zircon |
| — | 魔降（译名） | AdventOfDemon | 431 | — | — | — | — | 新增 | Zircon |
| — | 恶魔降临（译名） | AdventOfDevil | 432 | — | — | — | — | 新增 | Zircon |
| — | 深渊（译名） | Abyss | 433 | — | — | — | — | 新增 | Zircon |
| — | 闪光（译名） | FlashOfLight | 434 | — | — | — | — | 新增 | Zircon |
| — | 隐匿（译名） | Stealth | 435 | — | — | — | — | 新增 | Zircon |
| — | 闪避（译名） | Evasion | 436 | — | — | — | — | 新增 | Zircon |
| — | 狂风（译名） | RagingWind | 437 | — | — | — | — | 新增 | Zircon |
| — | 屠戮（译名） | Massacre | 439 | — | — | — | — | 新增 | Zircon |
| — | 影术（译名） | ArtOfShadows | 440 | — | — | — | — | 新增 | Zircon |
| — | 龙血（译名） | DragonBlood | 441 | — | — | — | — | 新增 | Zircon |
| — | 致命一击（译名） | FatalBlow | 442 | — | — | — | — | 新增 | Zircon |
| — | 背水一战（译名） | LastStand | 443 | — | — | — | — | 新增 | Zircon |
| — | 魔力燃烧（译名） | MagicCombustion | 444 | — | — | — | — | 新增 | Zircon |
| — | 活力（译名） | Vitality | 445 | — | — | — | — | 新增 | Zircon |
| — | 锁链（译名） | Chain | 446 | — | — | — | — | 新增 | Zircon |
| — | 专注（译名） | Concentration | 447 | — | — | — | — | 新增 | Zircon |
| — | 双持技（译名） | DualWeaponSkills | 448 | — | — | — | — | 新增 | Zircon |
| — | 封锁（译名） | Containment | 449 | — | — | — | — | 新增 | Zircon |
| — | 龙波（译名） | DragonWave | 450 | — | — | — | — | 新增 | Zircon |
| — | 出血（译名） | Hemorrhage | 451 | — | — | — | — | 新增 | Zircon |
| — | 燃火（译名） | BurningFire | 452 | — | — | — | — | 新增 | Zircon |
| — | 火链（译名） | ChainOfFire | 453 | — | — | — | — | 新增 | Zircon |
| — | 烈焰匕首（译名） | FlamingDaggers | 454 | — | — | — | — | 新增 | Zircon |
| — | 撕裂（译名） | Shredding | 455 | — | — | — | — | 新增 | Zircon |
| — | 四轮（译名） | FourWheels | 456 | — | — | — | — | 新增 | Zircon |
| — | 新月（译名） | CrescentMoon | 457 | — | — | — | — | 新增 | Zircon |

### 待确认技能

| 1.45 技能 ID | 中文名称 | 英文名称 / 标识 | Zircon ID | MIcon 索引 | SPELL 起手 | MAGIC 施放效果 | EXPLOSION 命中效果 | 核对依据 | 客户端版本 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| — | 基本剑术 | ONESWORD | — | — | — | — | — | 候选编号 3。安装包与早期客户端 | 参考客户端／安装包 |
| — | 精神力战法 | ILKWANG | — | — | — | — | — | 候选编号 4。安装包与早期客户端 | 参考客户端／安装包 |
| — | 假武功 | — | — | — | — | — | — | 候选编号 199。安装包 Magic.exp | 宝藏安装包，版本待确认 |
| — | 耐力（译名） | Endurance | 116 | — | — | — | — | 待确认。新增特效使用 MagicEx3。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 群体牵引（译名） | MassBeckon | 123 | — | — | — | — | 待确认。新增特效使用 MagicEx5。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 地震猛击（译名） | SeismicSlam | 124 | — | — | — | — | 待确认。新增特效使用 MagicEx5。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 无敌（译名） | Invincibility | 125 | — | — | — | — | 待确认。新增特效使用 MagicEx5。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 碎浪（译名） | CrushingWave | 126 | — | — | — | — | 待确认。新增特效使用 MagicEx6。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 防御打击（译名） | DefensiveBlow | 130 | — | — | — | — | 待确认。新增特效使用 MagicEx7。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 太天剑（译名） | TaecheonSword | 135 | — | — | — | — | 待确认。新增特效使用 MagicEx5。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 火焰剑（译名） | FireSword | 136 | — | — | — | — | 待确认。新增特效使用 MagicEx5。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 雷击（译名） | LightningStrike | 236 | — | — | — | — | 待确认。新增特效使用 MagicEx6。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 冰雨（译名） | IceRain | 238 | — | — | — | — | 待确认。新增特效使用 MagicEx7。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 冻伤（译名） | FrostBite | 239 | — | — | — | — | 待确认。新增特效使用 MagicEx5。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 天火之怒 | Asteroid | 240 | — | — | — | — | 待确认。新增特效使用 MagicEx5。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 龙卷风（译名） | Tornado | 242 | — | — | — | — | 待确认。新增特效使用 MagicEx5。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 冰霜光环（译名） | IceAura | 243 | — | — | — | — | 待确认。新增特效使用 MagicEx5。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 冰龙（译名） | IceDragon | 244 | — | — | — | — | 待确认。新增特效使用 MagicEx5。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 破冰（译名） | IceBreaker | 245 | — | — | — | — | 待确认。新增特效使用 MagicEx5。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 冻龙（译名） | FrozenDragon | 246 | — | — | — | — | 待确认。新增特效使用 MagicEx5。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 诅咒人偶（译名） | CursedDoll | 323 | — | — | — | — | 待确认。新增特效使用 MagicEx3。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 灵魂共鸣（译名） | SoulResonance | 325 | — | — | — | — | 待确认。新增特效使用 MagicEx7。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 降魔伏虎 | Parasite | 326 | — | — | — | — | 待确认。新增特效使用 MagicEx5。未找到能定位 1.45 技能 ID 的证据。国服名称依据：单体寄生效果，与 Zircon Parasite.ApplyPoison 对应。 | Zircon |
| — | 中和（译名） | Neutralize | 340 | — | — | — | — | 待确认。新增特效使用 MagicEx7。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 暗魂牢（译名） | DarkSoulPrison | 342 | — | — | — | — | 待确认。新增特效使用 MagicEx6。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 灼光（译名） | SearingLight | 343 | — | — | — | — | 待确认。新增特效使用 MagicEx3。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 尸爆（译名） | CorpseExploder | 345 | — | — | — | — | 待确认。新增特效使用 MagicEx7。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 缚身符（译名） | BindingTalisman | 347 | — | — | — | — | 待确认。新增特效使用 MagicEx5。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 脑力风暴（译名） | BrainStorm | 348 | — | — | — | — | 待确认。新增特效使用 MagicEx5。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 天穹（译名） | HeavenlySky | 349 | — | — | — | — | 待确认。新增特效使用 MagicEx5。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 毒雾（译名） | PoisonCloud | 350 | — | — | — | — | 待确认。新增特效使用 MagicEx5。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 升龙击（译名） | RisingStrike | 701 | — | — | — | — | 待确认。新增特效使用 MagicEx9。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 基本剑术 | Swordsmanship | 100 | — | — | — | — | 待确认。候选 3。参考名称和原版图标槽吻合，原版伤害或命中路径仍待确认。国服名称依据：战士被动命中。 | Zircon |
| — | 运气术 | PotionMastery | 101 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 毁灭冲击（译名） | DestructiveSurge | 109 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 迅刃（译名） | SwiftBlade | 114 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 突袭（译名） | Assault | 115 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 移花接木 | ReflectDamage | 117 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 泰山压顶 | Fetter | 118 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 强化毁灭冲击（译名） | AugmentDestructiveSurge | 119 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 强化防御（译名） | AugmentDefiance | 120 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 强化伤害反弹（译名） | AugmentReflectDamage | 121 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 强化药剂精通（译名） | AugmentPotionMastery | 122 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 防御精通（译名） | DefensiveMastery | 127 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 物理免疫（译名） | PhysicalImmunity | 128 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 魔法免疫（译名） | MagicImmunity | 129 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 元素剑（译名） | ElementalSwords | 131 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 手里剑 | Shuriken | 132 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。国服名称依据：单体投掷攻击。 | Zircon |
| — | 百裂拳（译名） | HundredFist | 133 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 进攻打击（译名） | OffensiveBlow | 134 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 火焰之术（译名） | FlameArt | 137 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 精钢火球（译名） | AdamantineFireBall | 208 | — | — | — | — | 待确认。候选 5，需与 FireBounce 区分。 | Zircon |
| — | 击风（译名） | BlowEarth | 215 | — | — | — | — | 待确认。候选 74，起手和命中与 73 复用。 | Zircon |
| — | 地脉操控（译名） | GeoManipulation | 218 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 魄冰刺 | GreaterFrozenEarth | 224 | — | — | — | — | 待确认。候选 110，需验证原版三射线。 | Zircon |
| — | 流星火雨（译名） | MeteorShower | 226 | — | — | — | — | 待确认。候选 113，待核对原版弹道与攻击范围。 | Zircon |
| — | 凝血离魂 | Renounce | 227 | — | — | — | — | 待确认。候选 112，MagicEx2 80 吻合，待核对技能机制。 | Zircon |
| — | 暴风（译名） | Tempest | 228 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 天罚（译名） | JudgementOfHeaven | 229 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 霹雷（译名） | ThunderStrike | 230 | — | — | — | — | 待确认。与 11 复用 Magic 1450，待核对独立行为。 | Zircon |
| — | 弹射火球（译名） | FireBounce | 231 | — | — | — | — | 待确认。原版 5 和 113 复用 Magic 1560／1800。 | Zircon |
| — | 元素飓风（译名） | ElementalHurricane | 232 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 高阶魔法盾（译名） | SuperiorMagicShield | 233 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 燃烧（译名） | Burning | 234 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 感电（译名） | Shocked | 235 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 分身术 | MirrorImage | 237 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 风暴（译名） | Storm | 241 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 天人合一（译名） | UnityWithNature | 247 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 精神力战法 | SpiritSword | 301 | — | — | — | — | 待确认。候选 4。参考名称和原版图标槽吻合，原版道士被动效果仍待确认。国服名称依据：道士被动命中。 | Zircon |
| — | 空拳刀法 | CombatKick | 311 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 嗜血（译名） | BloodLust | 314 | — | — | — | — | 待确认。候选 94 或 95–101，共用 MagicEx 140。 | Zircon |
| — | 养生术 | EmpoweredHealing | 319 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 吸血（译名） | LifeSteal | 320 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 改良爆裂火符（译名） | ImprovedExplosiveTalisman | 321 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 强化施毒术（译名） | AugmentPoisonDust | 322 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 雷踢（译名） | ThunderKick | 324 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 通灵术（译名） | Spiritualism | 327 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 强化爆裂火符（译名） | AugmentExplosiveTalisman | 328 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 强化驱邪术（译名） | AugmentEvilSlayer | 329 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 强化云寂术（译名） | AugmentPurification | 330 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 强化回生术（译名） | AugmentResurrection | 331 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 焰魔召唤术 | SummonDemonicCreature | 336 | — | — | — | — | 待确认。与 17／105 复用部分召唤帧，召唤对象不同。国服名称依据：召唤 InfernalSoldier。 | Zircon |
| — | 魔焰强解术 | DemonExplosion | 337 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。国服名称依据：消耗已召唤的 InfernalSoldier。 | Zircon |
| — | 感染（译名） | Infection | 338 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 魔物恢复（译名） | DemonicRecovery | 339 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 强化中和（译名） | AugmentNeutralize | 341 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 强化天光（译名） | AugmentCelestialLight | 344 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 召唤亡者（译名） | SummonDead | 346 | — | — | — | — | 待确认。与 17／105 复用部分召唤帧，待核对召唤对象。 | Zircon |
| — | 至高治愈（译名） | SupremeHealing | 351 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 未使用（译名） | Unused | 438 | — | — | — | — | 待确认。枚举预留项。 | Zircon |
| — | 法力灼烧（译名） | ManaBurn | 458 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 怪物地狱火（译名） | MonsterScortchedEarth | 501 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 怪物冰咆哮（译名） | MonsterIceStorm | 502 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 怪物死亡毒云（译名） | MonsterDeathCloud | 503 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 怪物雷暴（译名） | MonsterThunderStorm | 504 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 萨玛守卫之火（译名） | SamaGuardianFire | 505 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 萨玛守卫之冰（译名） | SamaGuardianIce | 506 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 萨玛守卫之雷（译名） | SamaGuardianLightning | 507 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 萨玛守卫之风（译名） | SamaGuardianWind | 508 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 萨玛凤凰之火（译名） | SamaPhoenixFire | 509 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 萨玛黑冰（译名） | SamaBlackIce | 510 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 萨玛蓝雷（译名） | SamaBlueLightning | 511 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 萨玛白风（译名） | SamaWhiteWind | 512 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 萨玛先知之火（译名） | SamaProphetFire | 513 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 萨玛先知之雷（译名） | SamaProphetLightning | 514 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 萨玛先知之风（译名） | SamaProphetWind | 515 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 末日巨钳左夹（译名） | DoomClawLeftPinch | 520 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 末日巨钳左扫（译名） | DoomClawLeftSwipe | 521 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 末日巨钳右夹（译名） | DoomClawRightPinch | 522 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 末日巨钳右扫（译名） | DoomClawRightSwipe | 523 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 末日巨钳波动（译名） | DoomClawWave | 524 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 末日巨钳喷吐（译名） | DoomClawSpit | 525 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 粉色火球（译名） | PinkFireBall | 530 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 绿色泥浆弹（译名） | GreenSludgeBall | 540 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |
| — | 龙冲（译名） | DragonCharge | 700 | — | — | — | — | 待确认。未找到能定位 1.45 技能 ID 的证据。 | Zircon |

## 主要动作与帧定位

| 动作 / 阶段 | 英文名称 | 资源及索引规则 | 主要行为 |
| --- | --- | --- | --- |
| 定向起手（人物动作2） | Directed Cast | 人物身体基础帧160，方向步长10；特效查SPELL[技能ID] | 身体共5帧；中段保持时特效帧继续增加，不能只按身体帧选图。 |
| 范围起手（人物动作3） | Area Cast | 人物身体基础帧240，方向步长10；特效查SPELL[技能ID] | 身体共5帧；末段等待技能起手帧完成。默认限制10，技能28/11/78分别20/13/14，104初始间隔40ms。 |
| 特殊动作18 | Special Cast | 人物身体基础帧1440＋方向×10，共10帧 | 技能36；音效索引142。 |
| 冲撞动作23 | Rush | 人物身体基础帧1840＋方向×10，共6帧 | 技能27请求不提前播放位移，等待服务器动作消息。 |
| 飞行 / 追踪 | Travel / Homing | MAGIC[技能ID]基础帧＋方向组偏移＋相对帧 | 常规16方向、部分8方向或固定方向；追踪目标、速度及超时按技能分支。不能将所有MAGIC都解释为飞弹。 |
| 命中 / 爆炸 | Impact / Explosion | EXPLOSION[技能ID]基础帧＋对应方向偏移＋相对帧 | 命中时转换资源；保留该效果寿命和光源状态。 |
| 固定 / 附着效果 | Fixed / Attached Effect | MAGIC或SPELL记录；位置为指定格或跟随对象 | 治愈、护盾等表现以回包和技能分支选择，不仅由资源段名称判断。 |
| 地面波与三射线 | Ground Wave / Three Rays | 9、53、110的MAGIC记录 | 9/53沿路径分段，110形成三条射线；10另使用MAGIC 336地表尾迹。 |
| 分阶段效果 | Staged Effect | 73/74：MAGIC → EXPLOSION | 起始、偏移运动、爆炸分开推进；不能按普通直线飞弹处理。 |
| 双环绕粒子 | Orbiting Flame | 150：MAGIC/EXPLOSION零帧；ProgUse 520、530为粒子纹理 | 普通火球加两组错峰发射器，每次5火星＋1光点；零帧记录不是资源错误。 |
| 结束光尾 / 清理 | Light Tail / Cleanup | MInfo半径、RGB及光源字段；无独立图像索引 | 贴图结束后可继续衰减光源；离场清理效果、粒子和目标引用。 |

方向性起手的图号为 `SPELL.first＋方向×10＋起手相对帧`；非方向性技能方向偏移为0。普通法术飞行采用 `MAGIC.first＋方向×(end−first)＋相对帧`，固定方向分支偏移0；特殊子类另按各自规则。人物身体库由性别、衣服等外观决定，以上160/240/1440/1840是动作相对基址，不是Magic.wil索引。原版动作2/3的默认75ms会被技能阶段和追帧逻辑修改。

资源库号：81=`Magic`，129=`MonMagic`，137=`MagicEx`，138=`MonMagicEx`，140=`MonMagicEx2`，143=`MonMagicEx3`，148=`MonMagicEx4`，149=`MagicEx2`，均配套WIL/WIX。音效起手索引为 `10000＋技能ID×10`，通过声音表解析文件。

## 样本与证据

- [IDA 样本指纹](../client/docs/evidence/feature-audit/README.md)记录了解包分析映像。本轮核对其 SHA-256 为 08f68b794bfb28803d1132c02b26d2595dba309be5b2b475ae0cdfe6a752f8aa。
- 0x418D80 内的分派决定本表收录的主动施法编号和本地路径。逐组编号和动作见[施法证据](../client/docs/evidence/skill-casting.md)及[路由表](../client/src/magic/spell_routes.hpp)。
- 本轮核对 Magic.exp 的 SHA-256 为 a0e430524fcacd27795fd604317d6ca1cb2460104b7b5cb114bfa6eb2ebec1a2，MInfo.dat 为 ed46fe33682180fbc23796797ba04f4db9928daf4eadb6937d79803d957a3b0b。

- 28 号参考命名见 [早期客户端 Define.h](../reference/LegendOfMir3_Src/LegendOfMir3_Client/Define.h)、[SprDfn.cpp](../reference/LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/SprDfn.cpp)和 [King_Magic.csv](../reference/mir2x/readme/sql2csv/King_Magic.csv)。

## 7 号攻杀剑术的战斗证据

- 本地攻击由 0x415F90 的近战路径选择挥击。0x416715 将效果编号 7 传给 0x40DB10，0x416743 将 3018（CM_POWERHIT）传给发包函数。
- 远端回包由 0x412C80 处理。0x412DB0 判断消息为 18（SM_POWERHIT），0x412DE2 同样传入效果编号 7。Magic.wix 的 1350–1355 六个索引均非零。
- 早期客户端在 SM_POWERHIT 分支使用 YEDO，Zircon 的 Slaying 使用 Magic 1350 起播。证据覆盖客户端近战表现与协议，触发概率和伤害计算仍需 1.45 服务端数据确认。[早期客户端 Actor.cpp](../reference/LegendOfMir3_Src/LegendOfMir3_Client/GameProcess/Actor.cpp) · [Zircon MapObject.cs](../reference/Zircon/Client/Models/MapObject.cs)

## 待核对的原版编号

早期客户端把 3、4 定义为 ONESWORD、ILKWANG。1.45 的 MIcon.wix 在对应索引 4、6 有非零偏移，两项在主动施法入口走默认分支。普通攻击路径选择挥击动作并发送 3014，尚未找到直接按 3、4 查找记录或计算增益的证据。199 号「假武功」见宝藏安装包 Magic.exp。[早期客户端 Define.h](../reference/LegendOfMir3_Src/LegendOfMir3_Client/Define.h) · [King_Magic.csv](../reference/mir2x/readme/sql2csv/King_Magic.csv)

Zircon 编号与英文标识见 [Enum.cs](../reference/Zircon/LibraryCore/Enum.cs)，行为和特效见[职业技能实现](../reference/Zircon/ServerLibrary/Models/Magics)与 [MapObject.cs](../reference/Zircon/Client/Models/MapObject.cs)。

## 国服名称来源

按官方中文名和效果说明对照 Zircon 实现。标注「译名」的中文名称沿用参考译法。此处仅核对名称，客户端版本和 1.45 对应状态沿用表中记录。[盛趣《传奇3》官方技能说明](https://mir3.web.sdo.com/web7/news_con.html?id=297641)

本轮补核的名称及对应依据如下。

- PotionMastery 对应「运气术」。使用药品时按技能强度提高恢复量。
- EmpoweredHealing 对应「养生术」。强化治愈术的单次恢复量与恢复上限。
- Renounce 对应「凝血离魂」。降低自身生命值上限并提高自然魔法。
- GreaterFrozenEarth 对应「魄冰刺」。沿三个方向施放冰系攻击。
- Asteroid 对应「天火之怒」。陨石落点结算范围伤害并生成地面火焰。

国服说明公开页提供名称与技能描述，部分条目尚未取得最新实服显示名的独立证据。上表保留未能建立对应关系的既有译名。

### 国服客户端 Data 核对

- 本地国服安装目录的 Data/SkillDesc.txt 经 Base64 解码后按 GB18030 读取。每条说明以中文技能名为键。本轮按实现交叉核对了移花接木（反弹受击伤害）、泰山压顶（近身范围减速）、分身术（生成自身镜像）和潜行术（进入刺客隐匿状态）。可按英文标识在 [Zircon 职业技能实现](../reference/Zircon/ServerLibrary/Models/Magics)中定位。
- Data/FORCESKILL.dat 是带 BOM 的 UTF-8 心法配置，能读到减伤等级、消耗和秘籍等属性。Data/RES_MONMAGIC.xml 的 MAGNAME 属于怪物招式编号，例如 60106 是「撕裂术」。这两份数据尚未建立与 Zircon 玩家或怪物技能枚举的编号对应关系。
- 对 Mir3.exe 和 46 个 DLL 检查了上述文件名及怪物表名的 ASCII、UTF-16LE 字符串，未命中直接引用。根目录 DLL 的可见版本信息包括音频、DirectX 和防钓鱼组件，其他 DLL 多位于 GPK 或 SDOA 下。具体哪个模块读取或提供运行时显示名尚未定位。

### 新版客户端技能说明读取路径

新版 MIR3.EXE 的入口 0x4B4B78 引用 Mir3.dat，路径引用位于 0x4B4C61。Mir3.dat 本身是 UPX 压缩的 PE。对离线解包副本检查发现，0x6A7C17 与 0x6A7C1C 拼接 SkillDesc 和 .txt，0x6A7C63 读取文件，0x6A7C77 转换内容，0x6A7CBA 跳过分号注释，0x6A7CCD 写入客户端列表。实际文件经 Base64 和 GB18030 解码后以中文名为键。另有 0x716EE1 引用 Data/SkillDesc.dat，本轮证据对应的是安装目录实际存在的 SkillDesc.txt。

CombatKick 的实现先尝试推开面前目标，再结算攻击，与 SkillDesc.txt 的「空拳刀法」说明对应。本轮补的是国服中文名，表中 311 与 1.45 编号的对应仍未验证。[Zircon CombatKick.cs](../reference/Zircon/ServerLibrary/Models/Magics/Taoist/CombatKick.cs)

资源读取函数位于解包后的 Mir3.dat，地址以该 PE 的 IDA 加载地址为准。此前对安装目录 46 个 DLL 的明文检索未找到这些资源名。上述数据路径已有游戏模块内的调用证据。
