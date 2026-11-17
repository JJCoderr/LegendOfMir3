# 原生施法完整批次审查（实施中）

基线d7c70bb。此文件记录下一批的实证范围；不表示施法完成，不单独启动测试或提交。总体目标仍为原生CLIENT GUI及服务器接入。

## 本次核对结果

- Actor.cpp CMyHero::OnKeyDown仍以timeGetTime和500+m_wMagicDelayTime准入，没有SpellTiming的1000延迟及F10/F12重复入口语义；MP提示仍为英文，25/103没有豁免。
- SkillWnd.h使用pack(1)的74字节CLIENTMAGICRCD；延迟nDelayTime位于记录49，门槛bDefSpell位于53。SkillWnd::Receive完整memcpy原记录，字段已经保留。不要照抄Godot早期文档“丢字段”的旧结论，也无需另建技能记录副本。
- ClientSocket::SendSpellMsg的3017头字段位置已经正确，但SendPacket正文为NULL；原版要求GetTickCount DWORD按有符号十进制正文编码，沿用公共序号流。不能只补正文便宣称完整施法。
- SetMagic仍为旧编号switch；普通、冲刺、直接技能及等待确认分支需整体对照SpellRoutes。上一批新ExitFlow限制尾部已经是原版集合，迁移时应共用同一路由，避免两份技能集合逐渐分歧。
- CHero::OnSpell已经直接atoi读取17消息正文，并按目标格算方向；缺口在动作/方向资源表和动画帧规则，不能再次六位解码正文。当前SINSU按旧分支加载有方向效果，仍需分别检查远端动作选择。
- OnMagicFire处理638但仍为旧少量技能分支：9只产生5段，而原版9/53为9段；10走旧CElecMagic；特殊73/74/110/113/150及1410不能靠统一CMagic替代。
- CMyHero::CanWalk返回“处于施法延迟”的布尔值，搜索Actor.cpp只发现定义未发现调用；不能仅改这个函数就声称移动限制接通。需追到SetMotionState及鼠标行走/奔跑实际入口。

## 参考来源及迁移顺序

已读取Mir3仓库br_godot_test_261115的client/docs/evidence/skill-casting.md、magic/spell_routes.hpp、magic/spell_timing.hpp、protocol/spell_request.hpp。文档包含多次开发中与后续完成记录，须以当前代码和各专项范围为准，不能以开头旧状态或末尾局部通过替代整体验收。

1. 共用路由/时钟和原记录字段，统一实际按键/骑乘/模态/MP/重复入口；补3017正文与原文提示。
2. 本地动作预测、位置/朝向保存、等待确认、移动限制和失败/超时恢复，保持同角色换图与离场生命周期区别。
3. 起手SPELL资源、动画独立帧、方向、声音；远端17的FIFO与本地动作区别。
4. 638/1410效果分派及MAGIC/EXPLOSION资源、追踪/固定落点、特殊阶段、声音光尾和目标丢失；使用当前CLIENT绘制与声音设施，不引入Godot。
5. 功能闭环后一次集中构建与定向验证：实际键盘请求、协议正文/回绕、MP和间隔边界、GOOD/FAIL/BUMP恢复、效果/音效/清理，展示实际画面。随后中文提交并更新AI2AI。

下一预览仍237；本次尚未添加预览、运行测试或修改运行时代码。


## 当前实施状态（未构建、未验收）

已移入无Godot依赖的SpellRoutes/SpellTiming，ExitFlow限制技能集合改为共用路由。SendSpellMsg补GetTickCount位模式的有符号十进制正文；SkillWnd增加49/53偏移编译断言。CMyHero新增独立时钟并在InitActor清零，SetMagic迁移普通/冲刺/特殊/直接/预测路由，OnKeyDown改1000时序、骑乘限制、25/103 MP豁免和CmList1000提示。对照当前hud_view.cpp纠正预测直接请求目标格为0,0且保留casting_delay；自身目标技能的尾部限制使用改写后的自身编号。

尚未完成同批动作/效果/移动/失败生命周期，不得据此发版：SetMotionState仍为旧起手方向与效果限制，动作18需要补动画/声音，冲刺/特殊保存和恢复需核对；新时钟暂镜像至旧移动字段，须迁移实际移动入口后整理。下一步读取godot/hud_view.cpp request_skill_cast完整上下文、human_actions/action_prediction和native当前MotionLock/动作推进，再实现起手/恢复。没有启动测试或提交，预览仍237。


### 起手动作继续迁移（未验收）

新增CHero::BeginSpellAction供本地与远端17共用，统一方向性、action_limit和104首帧间隔，远端用remote_action而非旧switch（30改动作3）。本地普通请求先保存旧位置/方向再发送，重置确认累计。缺少起手贴图时清效果帧并禁绘，但动作/法术帧推进不再依赖m_bUseEffect，避免空资源使时序提前结束。尚未替换动作2/3阶段循环阈值和阶段间隔。

查明关键资源缺口：实际g_xSpriteInfo是SprDfn.h/.cpp的CSprite，不是未使用旧SpriteInfo.cpp的CSpriteInfo；GetEffectInfo仍查固定m_stEffectSpr，不能直接把新150编号路由当资源迁移完成。CHero::PlayActSound的最终PlayActorWav被注释，旧技能声音查表亦不符合10*(id+1000)。原始MInfo解析已有LevelEffects.cpp局部例子；Godot完整资源模型为resources/effect_catalog.hpp，三节SPELL/MAGIC/EXPLOSION使用19列，CRLF、逗号处转换、末无逗号字段不转换。下一步迁移此表并映射资源库与混合，再接起手/命中音效；不要简单启用旧整张声音表。

本次仍未启动构建/测试/提交，继续作为同一个完整功能批次。


### 原资源目录和起手时序继续实现（未验收）

EffectCatalog.h已从Godot纯C++资源模型移植，改用现有LoadLegacyText读取MInfo.dat，保留三节、计数、19列、CRLF及逗号转换规则。BeginSpellAction按实际SPELL记录存在性触发10*(skill+1000)声音，通过CLIENT PlayActorWav，不启用旧被注释声音表。仅资源目录与起手音效已接入，实际贴图仍通过旧LoadEffect，不能声称资源绘制已完成。

统一ApplySpellPhase用于本地/远端动作2/3：默认75ms，104用40ms；动作3在法术帧<=limit-2时回卷并用70ms，动作2使用limit-4与200/80ms阶段。独立法术帧不因角色动作回卷而倒退。远端追帧折半和状态速度标记仍需核对。

查明CLIENT旧m_xImageList只有61项，而原MInfo资源库有137/149等，不能直接把原library写入数组下标。下一步需独立资源库映射/持有，参考Godot hud_view.cpp约4613的映射及现有LevelEffects独立MagicEx加载；避免未经转换使用原编号越界。本批仍未构建/测试/提交。


### 原生资源绘制接入（未验收）

EffectRenderer.h新增按原库编号到Magic/Mon-4/MonMagic/MagicEx/MonMagicEx1..4/MagicEx2的独立WIL持有，未扩大或误索引旧61项数组。参考Godot prepare_effect_library与level_effect.gdshader实现RGB565逐像素模式0..6、颜色乘法/补色和透明度；现有WIL压缩透明段跳过。CHero虚拟DrawEffect在动作2/3绘制原SPELL帧（first+方向*10+独立法术帧），仍处于人物身体之后的原调用位置；其余动作保留原路径。BeginSpellAction直接读取MInfo起手记录，不再通过旧LoadEffect查询静态表，起手声仍按资源存在触发。

剩余：元素色覆盖、起手光源收缩/清理、远端追帧时序、移动门槛/预测恢复、638/1410及特殊子类/命中音效。图片缓存持有生命周期、缺失资源反复加载及真实场景像素/声音需同批验证。未构建、未测试、未提交；不能把新增绘制路径当已验收。


### 元素色及光源继续实现（未验收）

SpellColor.h复用参考模型；BeginSpellAction本地读取StatusWnd保存的能力扩展原包，按AttackElements-Extension偏移取七个完整WORD，严格大于才替换最大值，保留同值时较早元素；不使用GUI只显示低BYTE的m_elementValues。远端保持资源原色，未虚构远端元素数据。起手时冻结颜色，InitActor清理颜色与光源状态。

CHero::DrawEffect在取贴图前按独立法术帧和limit推进光源半径，半程后收缩，外半径0后停止，遵守现有光照开关；不存在贴图不阻止光源处理。资源颜色副本传给混色绘制，不修改共享目录。

PlayActorWav审查发现听者坐标错误使用MAKEPOS(Chr,Dx,Sy)，忽略传入Dy，导致垂直方向距离/声像丢失。本批改为Dx,Dy，与既有施法起手传入的本地角色坐标一致，待音效整体验证。

仍未构建/测试/提交；接下来继续移动/预测、638/1410与特殊效果，最终集中验收。


### 移动准入与失败回退继续实现（未验收）

LocalActionClock纯C++模型已移入，并在InitActor复位。SetMotionState实际四类步行/跑步入口先调用600ms admit_move，再检查完整DWORD SpellTiming::movement_ready；被施法延迟拒绝时保留已更新的移动准入时间。shared_wait与待执行动作队列尚未接入，不能宣称整个417D50/419B40已完成。

冲刺23与特殊18发包前保存旧位置/朝向；18开始原动作并播放142，23继续等待服务器实际位移。预测直接请求重置确认累计。现有+GOOD只清m_bMotionLock，+FAIL调用SetOldPosition；本批在失败回退处加死亡豁免及施法贴图/光源/累计清理，避免死亡复站和残留起手。仍需处理idle循环过早清输入锁、确认后动画继续、8秒超时与BUMP排队，不能以当前状态作为已验收恢复。

参考已读hud/action_prediction.hpp与local_action_clock.hpp，原ActionPrediction保存origin且GOOD后保留，FAIL死亡时不恢复，等待严格>8000释放；不要因GOOD删除保存位置。未构建、未测试、未提交。


### 施法确认等待继续接入（未验收）

普通起手、23/18及预测直接请求调用BeginSpellReply，独立elapsed累计而不借用旧motion累计；严格>8000才超时。站立/战斗待机循环保留未确认施法锁，动作2/3/18视觉结束则进入战斗待机但仍等待，避免循环重播。GOOD经ConfirmAction只解确认锁，若已待机则解输入锁，未结束起手继续播放；原保存位置未丢弃。FAIL回退清等待；InitActor复位。独立累计避免其他动作重置旧motion时间而永不超时。

仍需检查服务器BUMP/强制位移时输入锁与待机生命周期、远端FIFO/追帧及638/1410效果。未运行测试，不把以上实现状态当行为证明；同批实现完后集中构建和截图。


### 法术回包解码与队列容量（未验收）

SpellReply.h适配参考protocol/spell_reply.hpp：638独立解码4字节目标，技能取wSeries低BYTE；1410最多1024字节十进制列表，保留空项、符号、重复和最后一段，遇NUL停止，转换限制有符号32位。检查编码字符区间，拒绝超限；638已接入CHero::OnMagicFire，非法正文不再带未初始化目标进入旧效果创建。1410解析暂未接消费，待113列表效果与其他编号抑制规则一起连接，不能用通用效果替代。

发现PACKETMSG原正文MAX_PATH*4=1040，小于1024字节六位编码上限1366。已定义ActorPacketBodyCapacity=1367（含NUL），实际队列入队前拒绝超过容量的正文，避免原lstrcpy溢出。此结构改动须在整批验证时干净重建，不能混用旧对象文件。当前未构建/测试/提交。


### 飞行/命中模型迁移准备（尚未接入实例）

MagicRules.h/SpellLight.h复用参考纯C++规则，分别保留repeat/at_target/track_target/light、同格拒绝、无目标超时和光尾类别。SpellFlight.h从hud_view.cpp的spell_initial_velocity及44B7E0推进代码提取世界像素模型：初速横轴优先double/纵轴优先float商再截断；跟踪时500/主轴距离为整数商；按原float常量0.0011111111380159855推进，逐轴向零取整后比较接近与15像素命中界限。

模型只返回几何命中，尚未连接CMagic或新效果实例；3秒无目标分组、60秒终止、延迟、EXPLOSION资源切换保留帧累计、目标引用生命周期及声音必须由实例统一执行，不得单用这个模型宣称飞行效果完成。原CMagic使用WORD lifetime和旧静态表，直接复用其UpdateMagic会丢失上述差异；下一步需按这些证据实现实例生命周期并连接GameProc绘制/清理。未构建/测试/提交。


### 通用效果生命周期模型（尚未接入场景）

SpellPlayback.h接合EffectCatalog、MagicRules、SpellFlight和SpellLight。按当前参考advance_map_events顺序保存总年龄/帧累计/延迟：总年龄严格>60000结束；等待时清帧累计，解除时使用本次完整delta；严格超过资源间隔只推进一帧。飞行无目标特定组>=3000终止；几何命中换EXPLOSION、清方向偏移、保留帧/光源时钟，返回impact供场景决定目标仍绑定时的命中声。

实例绑定目标失效后永久解除，不接受后来同编号替代对象；调用方仍须提供稳定实例身份，而非只按网络id查询。单次动画结束转光尾，无光则释放；光尾按技能减色（保留BYTE 0/1回绕）或50ms等效半径收缩。地表、73/74和150必须专用推进，不套此通用类。

模型还未注册GameProc更新/绘制、未接638/1410实际创建和目标实例身份，故不代表运行效果已迁移。下一步需实现场景持有、绑定失效、同图换图/退出清理及声音分派，再补特殊子类。仍未构建/测试/提交。


### 场景实例与1410接入（未验收）

GameProc新增NativeSpell持有、Start/Advance/Draw，接入既有法术更新/绘制阶段及所有levelEffects清理点。CActor InitActor分配独立64位实例序号，目标用网络id与实例序号共同核对，失效后SpellPlayback永久解除绑定，避免对象地址复用和同id重建。1410已接本地/远端FIFO，解析后更新当前技能，仅113逐项查找真实目标创建通用飞行效果，未知目标跳过、重复项保留。不是直接调用旧CMagic。

创建/命中声音使用原编号10001/10002分支，命中只在目标仍绑定时播放；绘制用独立原WIL映射，清理随切图/退出执行。尚有明确未完成处：当前目标位置按格坐标，需改为人物实际行走像素；屏幕原点使用SHORT GetScrnPos(0,0)对大地图可能溢出，应改用当前可见格差值；光照提交阶段与持有音效句柄生命周期须核对。638完整分支及地表/73/74/150仍未接新实例，不能验收或发布。未构建/测试/提交。


### 坐标与光照阶段修正（未验收）

AdvanceNativeSpells移到本地与远端动作推进之后；远端目标用tile*cell+ShiftPixel，避免只追格中心；本地依据固定屏幕锚点和当前地图滚动反算世界像素，不依赖上一帧绘制更新的SHORT屏幕坐标。绘制原点直接用32位int按MapHandler公式计算，移除GetScrnPos(0,0)的SHORT大地图溢出。

发现ShowLightFog在旧新法术贴图层之前，新实例此前在DrawNativeSpells才提交光源会太晚。已拆LightNativeSpells并放在ShowLightFog之前；贴图继续在原法术层，光尾无需贴图仍可提交光。尚须实际大地图/移动追踪/光源帧验证；新生成效果同帧年龄、独立音效回收及特殊分派仍待处理。没有构建测试提交。


### 新实例帧边界及73/74接入（未验收）

NativeSpell使用单调serial，RenderScene动作推进前记录边界；本帧FIFO新创建实例不在同帧Advance消耗寿命，已有实例仍使用动作更新后的目标像素。清列表不重用serial，避免切图同帧新实例越过边界。

StagedSpell.h移植参考纯模型并连接638的73/74创建：73锚施法者、74锚目标，方向按16方向右移；74创建(100,100)、首次推进(0,10)，固定1050门槛、移动1000ms但回卷优先；73原sin/cos轨迹。EXPLOSION切换保留帧累计/光源状态，阶段结束转共用光尾；不调用通用60秒/飞行推进。创建音效使用原目标格或源格，不误用74暂态像素偏移反算的格。

还未进行构建/图形验证；638其余特殊分支、150粒子和地表、独立声音回收、移动等待及强制位移仍未完成，不提交此中间状态。


### 638忽略/状态分支与地表模型（未验收）

OnMagicFire对照play_spell_reply补全完全忽略的104/21/30/8/18/17/78/64/105/121，不更新当前技能；22/16/63只更新编号。除尚待整体迁移的9/10/53/110地表相关路径外，其他单目标638统一进入StartNativeSpell，不再使用旧CMagic；150的通用前置效果已走新入口，但双轨粒子仍未完成。

新增GroundSpell.h纯模型：9段150ms，336为9段30ms；110按原8方向三射线表、每线5段、保留重叠起点。还保留fire1915前跳帧/1925后可走地形80ms、fire/ice残留阈值及提前结束阈值，返回残留/冰粒子/闪电粒子事件供场景消费。目前此模型尚未接入场景；旧9/10/53/110创建仍暂存，不能称地表效果已迁移。下一步应连接地形可走性、残留地表、余烬和粒子，再整体替换旧分支。未构建、测试或提交。


### 地表波、残留与共享粒子池接入（未验收）

638的9/53/110改走StartGroundSpell，10先通用自身效果再336九段尾迹，旧CMagicStream/CElecMagic回包分支已删除。NativeSpell增加地表与余烬播放，地形仅使用地图边界和GetTileAttribute，不查询角色占位。地表按独立延迟累计（释放延迟取余量）、先光后动画、严格超过间隔推进一次，保留地表光尾最后一帧；336只发射粒子、不画普通WIL。

残留火痕220、冰痕260/261按2秒存活，放在地图底层和角色前绘制；火焰另生成330..334余烬，循环10秒后每次推进减色5。余烬创建延后到遍历结束避免vector迭代失效，但资源随机编号在原残留事件位置预先选择；新增实例不扣同帧寿命。所有退出/切图清理入口统一ClearNativeSpells，清除法术、痕迹、粒子。

移植SpellParticle/SpellParticlePool/SpellOrbit纯模型；目前冰与雷电发射接入三个各1000槽池，粒子使用屏幕坐标并在事件推进末统一更新。新增SpellSpriteRenderer从原ProgUse解码520..523/530与地表痕迹，按RGB565实现粒子ONE/INVSRCALPHA、光点ONE/INVSRCCOLOR及痕迹180/255透明度。尚未做实际截图/像素对照，不能宣称渲染已验收。150双轨发射尚未接入（只有模型），音效独立句柄和移动预测仍待完成。整批未构建、测试或提交。


### 150双轨及独立法术声音接入（未验收）

StartNativeSpell对150先生成普通火球1（目标0），再按同格拒绝生成两条150，间隔0/30ms，反相Orbit；允许原粒子资源空帧/零interval，命中EXPLOSION同样保留这一例外。轨迹只改显示/发射坐标，不改基础飞行；命中停止发射、保持独立光尾，不画150普通WIL。共享资源最后偏移按效果推进顺序记录供Orbit发射使用。新增SpellDirection纯函数替换通用/73/74/150的旧MapHandler方向计算，使用世界像素距离及原float余弦阈值。

CMirSound增加原50缓存*10声道模型，独立CSBuffer按需加载（m_ID设置正确以保证播放/析构），空闲声道从头播放，占满不抢占，缓存年龄>1000ms才可被替换。SpellSoundVoices/SoundPosition复用参考模型，独立音量与原屏幕格量化衰减/声像，支持反转/静音；起手、MAGIC、地表、余烬和命中已转入此路径。飞行效果记录音效key/sample，仅前5声道跟随基础飞行格坐标；失效/替换sample不会误改另一个声音，结束效果不截断音效，切图/退出ClearNativeSpells统一释放声音池。还修正旧ChgPlayingSet听者Y误用Sy。

上述代码均未编译/运行验证。后续重点：移动shared_wait与BUMP/FIFO、普通飞行与原帧/光顺序、声音实际文件路径/失败加载回收、渲染像素与大地图坐标，以及完整预览测试和截图。不能以此前音效/退出测试覆盖本批。


### 共享动作等待接入（未验收）

SetMotionState拆为请求入口和ExecuteMotionState实际执行。步行/跑步/骑乘移动及普通攻击保存PendingLocalAction（起点、目标、方向、技能、等待时刻）；普通攻击先通过原等级/速度/手持重量冷却，再进入共享等待。移动600ms准入未到保留请求；准入后若施法movement_ready拒绝则消费请求，保留准入时间。shared_wait按实际攻击时间的0.8倍、实际移动后480ms和初始800ms计算一次，等待完成先清请求再执行，防止重入。实际执行才写moved/attacked。

等待时不允许另一输入/技能覆盖请求，地图失效、角色坐标变化或死亡会取消。FAIL在未死亡时清待执行请求并回退。普通攻击时钟改为GetTickCount并初始0，与施法/共享等待统一；CanRun不再读截断WORD施法延迟，CanWalk按movement_ready返回。去掉本地/远端FIFO在动作2末段提前消费的旧例外，施法结束才消费后续消息。

尚未构建或运行；BUMP7/强制位移回弹及生命周期仍待补齐，动作队列与现有按键连发/移动确认/死亡优先队列须在整批集中回归。不能把当前接入当已验证功能。


### 强制位移及首次干净编译

ForcedMotion.h接入本地/远端CHero：RUSH6、BUMP7、BACKSTEP9以服务器终点反算起点，独立严格>95ms一次推进2帧，BUMP在帧4返回起点，其他帧6结束。位移按原方向7提交阈值及整数偏移计算；本地同步地图起点/滚动，回弹重置确认保存位置，普通强制位移保留仍待确认的施法。起手光/图层在强制位移开始清除，死亡和FAIL清强制状态。旧UpdateMove不再推进这些实例。

本批首次干净编译已完成：out/spell-client-clean.log末尾[87/87] Linking CXX executable Mir2Ex.exe，命令exit0，未见error。此前“未构建”的历史记录由此覆盖；尚未进行本批运行验收/截图，也未提交。需继续检查原生人体动作资源帧、强制消息外观/队列细节、共用播放帧/光顺序、位移/确认/死亡边界，以及独立声音实际播放。下步新增完整施法预览与报告（可从237开始），集中跑整批运行回归后提交；编译成功不等于原版行为一致。


### 首轮运行验收与特效截图

新增SpellPreview.h及237/238/239（--preview=spell-ground/spell-orbit/spell-staged）。通过实际MInfo/ProgUse资源、OnMagicFire638/OnMagicList1410入口执行33项检查，spell-check.txt全部1；覆盖基础时序/回绕、共享等待计算、强制位移模型边界、回包解析与忽略/仅更新编号、通用/地表/双轨分派、重复目标、粒子数量和清理。尚未覆盖真实3017发包、原生动作队列/强制位移与实际音频声道播放，不得以33项推导整个施法已验收。

7目标构建out/spell-all.log [111/111]通过。发现当前Ninja对头文件单独修改未自动触发重编译（spell-preview-layout.log显示no work to do）；预览布局头改后通过更新Mir2Ex.cpp时间戳重新构建[2/2]，确认新截图HUD位置[4,8]，不误用旧exe。未来头文件布局变化仍须干净重建，所有头改要明确确认实际对象重编译。

实际预览out/spell-ground.png、spell-orbit.png、spell-staged.png已查看：原生火焰/冰波、150双轨粒子、73/74分阶段图像正常显示在画面内。灰背景是离线特效审查场景，不是完整地图/人物战斗验收，光照和混合仍需对照原版。运行程序out/mud3-01/Mir2Ex-spell-review.exe；进程均已结束。整批仍未提交，下一步扩大动作/网络/音频/渲染验证并修复差异。


### 真实发包及本地恢复回归

SpellPreview扩展到49项全部通过（out/mud3-01/spell-check.txt）。使用本机UDP接收器捕获实际SendPacket序列化输出：3017头的坐标、目标DWORD拆分、技能字段、外层序号及有符号DWORD十进制tick正文；通过真实SetMagic验证普通起手、102预测直接请求和12快速分支，不只测试纯模型。验证+GOOD保留动画、+FAIL恢复GOOD前保存的原点、8000/8001超时边界、死亡不回滚和原生OnRush/AdvanceForcedMotion的BUMP往返。

首跑GOOD/FAIL两项失败源于测试输入遗漏“+”；OnSocketMessageRecieve要求+GOOD/+FAIL，修正测试数据后重跑全49项通过，未修改生产确认逻辑来迎合错误测试。out/spell-actions-rebuild.log [2/2]重新链接通过。仍缺实际法术音频声道状态/音量回归、完整动作队列与渲染原版对照，故整批未提交；没有活动进程。


### 实际DirectSound验收与已有功能回归

新增CSBuffer::GetPan与CMirSound::InspectSpellAudio读取真实DirectSound状态，而不是模型变量。首跑揭示生产路径错误：PlaySpellWav在根目录检查SoundList文件名，CSBuffer::Load实际拼接Sound\，导致所有声音提前返回。已将存在性检查改为同一Sound\路径，保持加载器接收裸文件名。修复后11项实际音频检查通过：播放中、位置音量/声像、反转、选项音量、sample失配保护、位置更新、十声道分配、满载不抢占、静音、重新启用、场景清理。总计60/60，out/spell-audio-path.log重新构建成功。

用同一Mir2Ex-spell-review.exe重跑现有快捷键34、交易54、装备25、界面音效15均全过，连同新施法60共188项。所有预览进程已结束，diff --check通过。仍需对照渲染混合与完整人物动作帧/队列及真实地图场景，整批未提交；不要用这188项替代上述尚未覆盖项。


### 施法人动作时钟对照修复

对照参考human_actions.hpp发现原生仍在没有推进帧时调用ApplySpellPhase，且计时使用WORD并忽略0x08000000减速状态。新增CHero::AdvanceSpellAnimation，独立DWORD累计，严格超过（状态修正后）间隔仅推进一次；只有推进帧时才应用中段重复/尾段间隔，远端追帧在首帧后将基础间隔减半一次，本地不继承远端追帧。普通动作仍沿用既有路径。BeginSpellAction重置独立累计。

干净编译out/spell-animation-clean.log [87/87]通过；新增7项真实CHero/CMyHero时钟检查通过（动作2完整序列、动作3技能11的13帧序列、75ms边界、减速150ms、>65535累计不回绕、远端37ms追帧）。施法报告现67/67。此次修改后尚未重新跑此前128项旧回归，不能将上一版188项宣称为此版195项；整批仍未提交。后续继续实际队列/地图与渲染像素对照。


### 实际地图数据及原生动作队列

新增9项回归通过，报告76/76。加载运行目录的真实01.map，图形视口暂放在地图外以便独立检查地图数据/动作逻辑，不宣称完整地图渲染已验证。原生SetMotionState/AdvancePendingAction测试600ms准入保留请求、共享等待不提前发包、原点变化取消、施法拒绝保留准入时刻、等待到期发送真实CM_WALK并消费请求/记移动时钟。通过CHero回包队列验证施法动作不提前消费638，动作结束后释放FIFO并更新技能编号。

out/spell-queue-build.log七目标构建[112/112]通过，运行Mir2Ex-spell-review.exe，程序已退出。尚需混合渲染与实际人物/地图视觉审查，以及最终同版旧功能回归，整批未提交。


### 施法整批收尾验证（2026-11-17）

原生离线预览增加 `--preview=spell-map`，实际加载01.map与地图/男性人物WIL，通过CGameProcess::RenderScene绘制人物施法、火墙、冰刺、150飞行粒子及原生HUD。截图 `out/spell-map.png` 已实际检查并展示；人物处于树下，保留原生树木遮挡关系。该画面为原生800×600回缓冲截图，不是Godot画面，也不宣称已与MIR3.EXE逐像素相同。

加载全套旧库时发现末尾interfacec2.wil/wix在当前MIR3资源目录不存在，会弹出错误。离线场景改为显式加载需要的地图和人物库；生产LoadAllImage的旧资源清单问题留待真实进场联调处理，未通过复制伪造资源绕过。

最终同一可执行文件 `out/mud3-01/Mir2Ex-spell-review.exe`：施法76、快捷键34、交易54、装备25、界面音效15，全204项通过；退出守卫/发送/等待30项及回选角本地TCP7项通过，总241项。七目标构建成功；最终预览调整构建 `out/spell-map-build.log` [2/2]实际重新编译链接，基于此前七目标完整构建 `out/spell-queue-build.log` [112/112]。git空白检查通过。退出预览已执行，但未将单独程序退出状态额外计入断言数。

本批以“原生施法动作、特效与空间音效”完整功能统一提交。仍待真实服务器联调、MIR3逐像素与听感验收；原生地图旧资源清单、天气与特效共享随机序列仍有已知差异。整个GUI与服务端接入目标尚未完成。


### 正式进场资源加载修复（2026-11-17）

已修复上一批发现的interfacec2缺失提示：CImageHandler::LoadAllImage保留旧索引布局，将不再使用的_IMAGE_INTEREX槽设为nullptr，仅加载真实资源。CGameWnd::SetGameWndActive移除旧三维纹理登记，当前ShowGameWnd使用m_pxWndImage直接二维绘制；其余三维引用位于旧注释代码中。未伪造或复制替代资源。

spell-map预览改用生产LoadAllImage，而非只加载场景子集；60个实际库加载成功，1个废弃槽为空，61项断言通过。实际01.map+人物施法+HUD画面生成out/scene-resources.png，已检查。相同二进制的窗口/快捷键34项通过，共95项。七目标增量构建out/scene-resource-build.log [4/4]成功，git diff --check通过。程序已退出。此证据覆盖正式资源加载和原生窗口开关，不等于真实登录/服务器进场流程已通过。
