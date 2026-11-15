# 怪物系统集中接入状态

当前按用户最新策略推进：以Zircon游戏逻辑为主体，匹配1.45服务端，再装入1.45客户端外观、动作和特效。宝藏传奇3.exe及其模块用于必要差异核对，不再以持续逆向或类数量扩张作为主线。全部怪物完成后集中验证，通过后终止本次迭代。

收尾以完整链路为准：参考怪物状态/行为进入MonsterObject体系；1.45服务端怪物数据与场景消息驱动相应对象；1.45资源呈现出生、移动、攻击、受击、死亡及特殊动作。当前server/TODO.md仍注明尚无自有服务端实现，不能把offline_preview的AI互打当成1.45服务联调。现有server-monsters.json保存服务端怪物名称与shape证据，client/src/protocol及Monster.cpp中的场景消息分发作为适配入口；具体服务器连接与完整数据字段仍需接入。

## 全量范围

`client/docs/evidence/monster-protocol/reference-inventory.json`记录104个参考专用类及141个AI工厂分支，包含原始初始化参数。`inventory_reference.py`仅做源码清单提取，不是完成性测试。各类world_status/baozang_status仍为待审计，不能根据声明数量标记完成。

104个参考专用类型现在均有C++声明及实现入口；缺失类清单为空仅证明类型覆盖，不证明每个方法、继承结构、工厂初始化或世界行为已完整。继续逐项收拢剩余状态结构体、基础服务与消息。

后续按完整行为组合推进：

- 守卫/城堡：核心规则已移入，ConquestWorld连接对象、玩家、行会归属、阻挡物及消息；仍需真实世界实例和协议呈现接入。
- 宠物/替身：Companion、MirrorImage、Puppet、Shinsu，主人生命周期、技能/属性、消息外观和队列归属。
- Boss召唤/场地法术：余下Boss、ArachnidGrazer及HalloweenMonster，连同已移植类的召唤工厂、毒云/火墙、死亡与掉落接入。
- 已移植普通/施法类：统一时钟、寻敌、移动、伤害、状态、消息与清理，不再逐类建立孤立演示。

## 结构调整（按最新要求优先直接移植）

先移植Zircon全量实现，再集中补世界接入与宝藏比对；不再先扩展适配框架，不额外增加防御性校验。MonsterObjectState存放共用位置、朝向、目标、时钟及基础状态，MonsterObject实现基础攻击/追击；每个怪物独立XxxState结构体，Xxx逻辑类继承MonsterObject或对应参考父类及自身状态。无新增字段的派生类型保留空状态结构体。头文件仅声明，实现集中Monster.cpp。

已迁入共用基类的首组包括Sama元素守卫/黑蓝白/Phoenix、NumaMage、EscortCommander、PinkBat、Monkey、FieryDancer、BanyaLeftGuard、UmaKing、SkeletonAxeThrower、TerracottaBoss、FlameDemon、CorrosivePoisonSpitter、OxFeralGeneral、BanyoWarrior、SpittingSpider及其相关派生类。其余现有类仍需拆出状态并收归基类；清单新增state_struct字段，不能把已有类数当成架构迁移完成。

Shinsu按SpittingSpider继承，创建后一秒显现、出生两秒动作锁，目标刷新十秒变身期限；MirrorImage保留十五秒期限、主人死亡/消失分支、1.5秒施法及四种视觉均使用冰伤的参考行为；Puppet保留五秒引爆、受击/有目标时置零HP、主人死亡时不自动引爆、死亡后800ms将魔法列表和暗石属性快照排入主人队列。三类已写核心逻辑，玩家外观包复制及去除Cloak/Transparency、主人世界绑定尚待接入。Companion已移植主要AI、拾取/背包/喂食/技能逻辑，数据库绑定和外观/物品消息仍待接入。

新增参考实现均采用独立State及对应继承类：ArachnidGrazer保留0..2初始朝向、600ms孵化时读取当前目标、方向+3偏移出生；DragonLord保留十五秒子体维护、远处非Boss清理、数量差直接传入SpawnMinions、40%选定中心及二格范围重复命中、共享远程目标列表。MonasteryBoss保留首次死亡召唤和风属性GreenSludgeBall，参考注释“1/15生成两只”仍由SpawnMinions配置解释，不在怪物类硬编码。JinhwanSpirit死亡按SpawnChance抽样后召唤20+随机10子体。

ArchLichTaedu按血量七阶段每次Process仅降一阶段，ActionTime累加一秒后显形召唤；狂暴变体半血一次近距离传送、每秒1/7概率贴目标、下一次近战双倍伤害。LordNiJae保留CarnivorousPlant父类，十格范围攻击每目标500+距离×48延迟，命中后分别独立抽绿毒与麻痹；狂暴变体每三十秒清理十格外子体并补足五个。CarnivorousPlant同步迁入MonsterObject，显隐动作锁使用共用AttackClock。

HalloweenMonster保留出生经验/掉率×10、金币×30、目标背后攻击再归位、300ms后按野怪当前HP直击，先累积其经验并启动一分钟野怪攻击冷却，再按1/5概率随机传送。死亡在非安全地图抽取300级以下Boss或倒序搬移符合条件野怪。当前经验累积字段使用long double，最终仍需对接项目统一数值类型，不能宣称已与C# decimal精度等价。以上是源码移植状态；工厂完整参数、公共召唤规则、世界注册和消息消费尚未全量闭合，不作为可用性或宝藏比对证据。

最后五类参考核心已移植：FrostLordHwa按服务器星期映射亲和，每分钟传送视野目标并顺序施加Abyss/Silenced/Red/WraithGrip，三格外贴身传送成功才更新五秒时限，怪物目标直接置零HP；PachontheChaosbringer八格外传送、两格内DragonRepulse，保留else-if和不额外检查CanAttack的参考顺序；BanyoLordGuzak二十秒净化、四十五秒补足五个子体。JinchonDevil继承SpittingSpider，三格直线/近战与死亡云使用MonsterObject共用方法，云时长配置保留Min+Random。

MonsterObject共用DragonRepulse方法保留六秒Buff、首Tick500ms、随后每秒一次、五格曼哈顿距离筛选、倒序遍历、400ms延迟伤害及直推失败后两侧尝试。Buff到期/清除仍由统一Buff服务接入，不能将这些回调视为已经运行。

Companion包含30格背包/4装备格、属性叠加/重量/外观更新、主人背后召回、拾取寻路与占格规则、空闲和聊天计时、等级使用限制、自动喂食、3/5/7/10/11/13/15级技能抽取、碎片类型过滤、职业筛选、堆叠检查与合并、任务/货币/经验物品处理。保留源码CanGainItems堆叠满足后退出整个checks循环、重量逐项比较、不因没有匹配台词跳过聊天冷却等细节。Owner/DB ItemInfo、等级表和移动计时由调用方数据绑定，物品/技能/外观包仍须接入实际协议；非法存档槽的原参考日志尚需接日志服务。不能将类型清单归零作为全量行为验收。

## 统一运行入口（正在接入，未完成）

Monster.cpp中的zircon::Controller按RuntimeAI创建实际专用类型，保留AI 20恐惧参数、34九格射程、84 RangeChance=1。当前支持23个不需额外召唤/治疗/毒配置的工厂分支（包括93 EscortCommander）；未知AI明确拒绝，不能默认为普通怪物。141个工厂分支的其余配置仍须逐项映射。

HUD新增bind_reference_monster，绑定已存在的离线场景怪物；advance_monster_ai调用统一运行入口。普通移动/攻击进入现有SceneChicken动作队列，地图清理与对象删除移除控制器。Controller统一攻击时钟和动作等待，按每对象逆插入顺序投递到期攻击，死亡不清空已发出攻击。消息中的共享目标/位置列表在填充结束后才转为输出。

离线HUD帧循环已自动推进绑定对象（reference_auto_tick可关闭以便最终集中验收）；不是仅依赖测试调用advance_monster_ai。configure_reference_combat配置HP/MP、命中/敏捷、AC/MR、抗性、暴击、吸血、护盾比例和离线队伍。双方均配置后，延迟命中执行Monster.cpp的CombatState，更新血条、受击/死亡动作和吸血治疗。默认同队，不自动把普通怪物互相视为敌人。目标保持到不再合格，受击后记录反击目标；位置在命中/选范围时读取当前场景状态。

CombatState按参考保留物理命中/AC、魔法MR、正负抗性不同分母、受击范围门控在防御抽样后、红毒/转生倍率、护盾时长、暴击、上位盾吸收不溢出、护身戒指MP抵扣、过量伤害返回及吸血严格大于1才治疗。核心已包含部分状态字段，但HUD配置尚未暴露红毒/转生/上位盾的完整Buff生命周期。

运行入口已修正同帧状态读取：场景怪物移动立即更新cell，外部旧目标快照不再重复占住原位置；筛选实时排除死亡/埋藏对象、按MaxViewRange丢弃远处目标，延迟投递杀死目标后再次检查目标资格。受击显示记录按现有消费者约定使用HP/最大HP/伤害三字段，不再误写MP。Map.Load仅为可通行地形创建Cell，因此法术格查询保留walkable门控；完整格对象索引仍待接入。

**尚未闭合，不能作为完整战斗验收：** 未配置双方战斗属性的请求明确返回resolved=false，不伪造HP。毒投递、反伤、宠物技能经验、经验归属、奖励/掉落、死亡召唤、复活与Buff触发还未进入完整链；magic_requests尚未映射宝藏版视觉消息。当前离线目标列表不是完整世界对象索引，格对象空列表语义、地图边界、同格阻挡与视野配置仍需接入真实地图服务。首次寻敌暂按距离，需补参考ProperSearch及真实敌我权限，离线team仅为场景配置。该入口不用于联网场景，不将参考AI强加到服务器消息。

## 守卫与攻城规则（参考移植，尚未接完实际世界）

Guard按红名/赎罪、野怪被动属性及宠物主人判断目标；300ms后对怪物取命中时HP并清经验归属，直接调用Attacked，不经过普通Attack的命中/AC计算。攻击先在目标背后广播，再发送守卫原位置转向。保持不可移动、始终阻挡、免伤、天蓝名称和不休眠。

CastleObject维护城堡与行会归属，1001/1002/1003工厂分别创建旗、门、城防守卫。CastleGuard射程15，400+48×距离延迟投递；参与名单为空时可发起攻击，但不能承受玩家攻击，保留参考的不对称条件。CastleGate按FaceImage的三组五格偏移创建BlockingObject，非攻城时本行会玩家四格内自动开门，十秒后且四格内无任何玩家才关闭。开关、修复、死亡分别发送动作及阻挡显隐；外观血量等级保留Math.Round中点取偶，受击前检查方向；不额外加入参考没有的攻城权限限制。

CastleFlag保留30秒占领、附近其他行会重置、未参战玩家提前返回、目标丢失取消、Seconds分量倒计时，以及夺旗后的归属/外观事件。CastleLord保留八方向三格攻击、三格直线逐格首目标、十五秒死亡云（500ms后生成、4000ms后触发）和受击强制基础伤害1；死亡时要求合格EXPOwner，否则连base.Die也不执行。归属转移后的清目标、Ping/重生Boss、延长战争至少十五分钟、全体玩家Buff由finish_lord_capture世界接口执行，尚未接入具体服务。

EscortCommander已接普通Controller及HUD绑定：十格雷电术、两格概率雷暴、远程攻击后移动及再次检查攻击时钟。雷暴伤害预取一次，500ms后按同一数值投递所有目标。城堡ConquestWorld不得以离线team代替行会/战争数据；寻敌、世界注册、掉落、旗帜颜色信息、广播消费和宝藏协议仍待接入，不能据这些类存在宣称游戏链完整。

## 宝藏版证据进展

已恢复IDA工作会话并只读分析，无游戏启动或测试：

- king-entry-evidence.json：KingMir3-analysis.dll导出GetCharInfo/GetCharInfoEx/SetCharInfo/GetMsgProcess/RegMsgProcess。
- king-dispatch-evidence.json：RegMsgProcess(0x4A8EFC)保存回调到0x4BE854；GetMsgProcess(0x4A92B8)处理输入+16字符串并向对象+64的容器去重添加。不能仅凭函数名认定为怪物消息分发。
- king-character-evidence.json：GetCharInfo路径经过流、16字节转换与数据编码；SetCharInfo遍历PE导出。尚无证据将这些函数当作怪物属性逻辑。
- king-callback-evidence.json：沿0x4B9970定位sub_4A9350，线程枚举进程/模块、检查模块内容后调用注册回调。它不是已证实的怪物消息分发入口，不据此映射怪物行为。
- launcher-descriptor-evidence.json：重新打开已有分析数据库后，0x41750C描述符明确指向Mymir3.dll及ModifyGame，0x417524经DllFunctionCall解析；调用点0x4A1E14/0x4A1F92各传两个参数。这证明启动器包含该调用路径，尚不能证明当前运行时已成功加载和调用。
- baozang-module-identities.json：原始启动器与分析副本SHA256一致；当前MyMir3.dll与已分析副本一致。已有monster-baozang-mymir3-survey.json列出的导出仅为COM注册/类工厂接口，并无ModifyGame，不能把该文件直接当作所需游戏补丁模块。原目录Mir3.exe与Mir3.dat哈希一致；后续沿宝藏启动器生成/启动路径定位实际游戏映像及模块解析，不用旧版Mir3.exe替代最终依据。

请求和原始结果均在client/docs/evidence/monster-protocol，query_baozang.py强制显式会话，不回退Mir3.exe。上述是模块调查证据，不是特殊怪物行为已对照的结论。

本阶段未构建或启动测试。统一验证必须覆盖工厂默认/变体、出生/唤醒、移动、攻击、延迟命中、受击/状态、死亡/复活/召唤/清理以及宝藏消息映射后才能标记完整完成。
