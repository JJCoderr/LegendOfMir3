#ifndef _MAGIC_HANDLER
#define _MAGIC_HANDLER
#include "../../Shared/Mir3SkillIds.h"

#define _SKILL_BANWOL static_cast<int>(::mir3::SkillId::banwol)			// 半月.
#define _SKILL_AMYOUNSUL static_cast<int>(::mir3::SkillId::amyounsul)			// 暗然术.
#define _SKILL_FIREWIND static_cast<int>(::mir3::SkillId::firewind)			// 火焰风.
#define _SKILL_HANGMAJINBUB static_cast<int>(::mir3::SkillId::hangmajinbub)			// 降魔阵法.
#define _SKILL_DEJIWONHO static_cast<int>(::mir3::SkillId::dejiwonho)			// 大地援护.
#define _SKILL_FIRE static_cast<int>(::mir3::SkillId::fire)			// 盐社长.
#define _SKILL_MOOTEBO static_cast<int>(::mir3::SkillId::mootebo)			// 武太步。

#define _SKILL_TAMMING static_cast<int>(::mir3::SkillId::tamming)			// 雷魂击.
#define _SKILL_SPACEMOVE static_cast<int>(::mir3::SkillId::spacemove)			// 我空行法.
#define _SKILL_KILLUNDEAD static_cast<int>(::mir3::SkillId::killundead)			// 狮子轮回.

#define _SKILL_FIREBALL static_cast<int>(::mir3::SkillId::fireball)			// 火焰场.
#define _SKILL_HEALLING static_cast<int>(::mir3::SkillId::healling)			// 恢复术.
#define _SKILL_HOLYSHIELD static_cast<int>(::mir3::SkillId::holyshield)			// 结界.
#define _SKILL_BIGHEALLING static_cast<int>(::mir3::SkillId::bighealling)			// 大恢复.
#define _SKILL_LIGHTFLOWER static_cast<int>(::mir3::SkillId::lightflower)			// 雷雪火.
#define _SKILL_SHOOTLIGHTEN static_cast<int>(::mir3::SkillId::shootlighten)			// 雷印.
#define _SKILL_SKELLETON static_cast<int>(::mir3::SkillId::skelleton)			// 白骨召唤术.
#define _SKILL_SNOWWIND static_cast<int>(::mir3::SkillId::snowwind)			// 冰旋风.
#define _SKILL_BIGCLOAK static_cast<int>(::mir3::SkillId::bigcloak)			// 大隐身.
#define _SKILL_SHIELD static_cast<int>(::mir3::SkillId::shield)			// 咒术之幕.
#define _SKILL_SHOWHP static_cast<int>(::mir3::SkillId::showhp)			// 探气破延.
#define _SKILL_EATTHFIRE static_cast<int>(::mir3::SkillId::earthfire)			// 地炎术。
#define _SKILL_FIRECHARM static_cast<int>(::mir3::SkillId::firecharm)			// 爆杀系.
#define _SKILL_FIREBOOM static_cast<int>(::mir3::SkillId::fireboom)			// 爆裂波.
#define _SKILL_FIREBALL2 static_cast<int>(::mir3::SkillId::fireball2)			// 金刚火焰掌.
#define _SKILL_YEDO static_cast<int>(::mir3::SkillId::yedo)			// 锐刀.
#define _SKILL_ERGUM static_cast<int>(::mir3::SkillId::ergum)			// 御剑术.
#define _SKILL_LIGHTENING static_cast<int>(::mir3::SkillId::lightening)			// 强击.
#define _SKILL_FIRESWORD static_cast<int>(::mir3::SkillId::firesword)			// 焰火诀.
#define _SKILL_CLOAK static_cast<int>(::mir3::SkillId::cloak)			// 隐身.
#define _SKILL_SINSU static_cast<int>(::mir3::SkillId::sinsu)			// 神兽召唤.
#define _SKILL_ILKWANG static_cast<int>(::mir3::SkillId::ilkwang)			// 日光剑法.
#define _SKILL_ONESWORD static_cast<int>(::mir3::SkillId::onesword)			// 外手剑法。

class CMagicInfo
{
public:
	int		nIndex;
	char	szName[13];
	
	short	sEffectType;
	short	sEffect;
	short	sSpell;
	short	sPower;
	short	sMaxPower;
	short	sDefSpell;
	short	sDefPower;
	short	sDefMaxPower;
	
	short	sJob;

	short	sNeed[3];
	int		nTrain[3];
	
	short	sDelay;

public:
	int		GetPower13(int nPwr, int nLevel);
	BOOL	CheckMagicLevelup(_LPTHUMANMAGICRCD lptMagicRcd);

	__inline int MPow() 
		{ if ((sDefMaxPower - sDefPower) > 0) return sDefPower + (rand() % (sDefMaxPower - sDefPower)); 
			else return sDefPower; }
	__inline int GetSpellPoint(int nLevel)
		{ return ROUND(sSpell / (3/*MaxTrainLevel*/ + 1) * (nLevel + 1)) + sDefSpell; }
	__inline int GetPower(int nPwr, int nLevel)
		{ if ((sDefMaxPower - sDefPower) > 0) return ROUND(nPwr / (3/*MaxTrainLevel*/ + 1) * (nLevel + 1)) + (sDefPower + (rand() % (sDefMaxPower - sDefPower))); 
			else return ROUND(nPwr / (3/*MaxTrainLevel*/ + 1) * (nLevel + 1)) + sDefPower; }	
};

#endif
