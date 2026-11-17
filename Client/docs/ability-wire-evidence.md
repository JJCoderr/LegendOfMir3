# 角色能力同步至原生状态页和 HUD

依据 Godot br_godot_test_261115 的 protocol/actor_ability.hpp、docs/evidence/actor-ability.md，以及其中保存的 MIR3 42F5A0、4390E0、4710F0 分析，恢复 SM_ABILITY 的63字节记录。此前直接写入旧 ACTORABILITY，字段顺序不相同。

AbilityWire.h 命名线上偏移；ReceiveAbility 验证消息、职业低字节和编码/解码长度后，显式赋值原生结构：等级0；五组攻防范围1..10；HP/MP及上限11..18；经验19/23；负重27/29；穿戴和手持31..34。金币取recognition完整32位。同步本地人物HP/MP/MaxHP及血条百分比，不调用受击伤害逻辑。

元素攻击取36+2*i，强元素49+i、弱元素56+i；后两行仍按有符号BYTE显示。扩展28字节和原版四个附加头字段保留，未给未知字段臆造业务作用。状态窗口初始化清理新缓存。

完整Client清理构建及七目标构建通过。190 ability-wire 使用独立固定记录验证13项：接收、范围、生命、32位字段、负重、元素、原始扩展、截断、职业、消息类型、失败不改变状态、替换和清理；相关装备/状态/任务/邮件63项回归通过，共76项。截图out/ability-wire.png已检查。

测试经过六位编码正文和原生接收入口，尚未执行完整加密连接或实服流程。本仓库Server/GameSvr/ObjectPlayer.cpp的RM_ABILITY及相关分支仍直接编码CObjectAbility；其字段顺序和元素数与63字节原版记录不同，后续服务端接入必须显式序列化，不可用旧结构尺寸冒充兼容。

2026-11-17 更新：服务端基础63字节能力记录现已显式序列化，并通过实际登录人物页/负重截图验证；七元素扩展仍未映射。详见 native-ability-session-audit.md。
