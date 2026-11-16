#ifndef _MAGIC_HANDLER
#define _MAGIC_HANDLER

#define _SKILL_BANWOL				0			// °ëÔÂ.
#define _SKILL_AMYOUNSUL			1			// °µÈ»Êõ.
#define _SKILL_FIREWIND				2			// »ðÑæ·ç.
#define _SKILL_HANGMAJINBUB			3			// ½µÄ§Õó·¨.
#define _SKILL_DEJIWONHO			4			// ´óµØÔ®»¤.
#define _SKILL_FIRE					5			// ÑÎÉç³¤.
#define _SKILL_MOOTEBO				6			// ÎäÌ«²½¡£

#define _SKILL_TAMMING				7			// À×»ê»÷.
#define _SKILL_SPACEMOVE			8			// ÎÒ¿ÕÐÐ·¨.
#define _SKILL_KILLUNDEAD			9			// Ê¨×ÓÂÖ»Ø.

#define _SKILL_FIREBALL				10			// »ðÑæ³¡.
#define _SKILL_HEALLING				11			// »Ö¸´Êõ.
#define _SKILL_HOLYSHIELD			12			// ½á½ç.
#define _SKILL_BIGHEALLING			13			// ´ó»Ö¸´.
#define _SKILL_LIGHTFLOWER			14			// À×Ñ©»ð.
#define _SKILL_SHOOTLIGHTEN			15			// À×Ó¡.
#define _SKILL_SKELLETON			16			// °×¹ÇÕÙ»½Êõ.
#define _SKILL_SNOWWIND				17			// ±ùÐý·ç.
#define _SKILL_BIGCLOAK				18			// ´óÒþÉí.
#define _SKILL_SHIELD				19			// ÖäÊõÖ®Ä».
#define _SKILL_SHOWHP				20			// Ì½ÆøÆÆÑÓ.
#define _SKILL_EATTHFIRE			21			// µØÑ×Êõ¡£
#define _SKILL_FIRECHARM			22			// ±¬É±Ïµ.
#define _SKILL_FIREBOOM				23			// ±¬ÁÑ²¨.
#define _SKILL_FIREBALL2			24			// ½ð¸Õ»ðÑæÕÆ.
#define _SKILL_YEDO					25			// Èñµ¶.
#define _SKILL_ERGUM				26			// Óù½£Êõ.
#define _SKILL_LIGHTENING			27			// Ç¿»÷.
#define _SKILL_FIRESWORD			28			// Ñæ»ð¾÷.
#define _SKILL_CLOAK				29			// ÒþÉí.
#define _SKILL_SINSU				30			// ÉñÊÞÕÙ»½.
#define _SKILL_ILKWANG				31			// ÈÕ¹â½£·¨.
#define _SKILL_ONESWORD				32			// ÍâÊÖ½£·¨¡£

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
	BOOL	CheckMagicLevelup(CCharObject* pCharObject, _LPTHUMANMAGICRCD lptMagicRcd);

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
