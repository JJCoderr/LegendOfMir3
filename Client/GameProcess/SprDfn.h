#ifndef _SPRITEDEF_H
#define _SPRITEDEF_H



/******************************************************************************************************************

	Actor Sprite Frame Define

*******************************************************************************************************************/

//~~~ Hero 与怪物、NPC 用的帧定义. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct tagSPRFRMDEF
{
	WORD		wFstFrm;						// 动作的起始帧。
	WORD		wFrmCnt;						// 动作的帧数.
	WORD		wDelay;							// 动作的延迟时间.
}SPRFRMDEF, *LPSPRFRMDEF;


/******************************************************************************************************************

	Effect Sprite Frame Define

*******************************************************************************************************************/

//~~~ 用于魔法的帧定义. ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct tagEFFECTSPRINFO
{
	DWORD		dwFstFrm;
	DWORD		dwEndFrm;
	WORD		wDelay;
	WORD		wImgIdx;
	WORD		wEffectIdx;
	BYTE		bLightRadius[2];				// 光源宽度。
	BYTE		bLightColor [2][3];				// 光源颜色.
	BYTE		bMagicColor [3];				// 魔法颜色。
	BYTE		bBlendType;
	BYTE		bOpa;
	BYTE		bSwingCnt;
}EFFECTSPRINFO, *LPEFFECTSPRINFO;


class CSprite
{
public:
	EFFECTSPRINFO		m_stEffectSpr[_MAX_EFFECT];			// 魔法的施放及剑法效果.
	EFFECTSPRINFO		m_stMagicSpr[_MAX_MAGIC];			// 魔法效果.
	EFFECTSPRINFO		m_stExplosionSpr[_MAX_EXPLOSION];	// 魔法爆炸效果.

	EFFECTSPRINFO		m_stMonEffectSpr[_MAX_EFFECT];			// 魔法的施放及剑法效果.
	EFFECTSPRINFO		m_stMonMagicSpr[_MAX_MAGIC];			// 魔法效果.
	EFFECTSPRINFO		m_stMonExplosionSpr[_MAX_EXPLOSION];	// 魔法爆炸效果.


	SPRFRMDEF			m_stHeroSpr[_MAX_HERO_MTN];			// Hero 的动作帧定义。
	SPRFRMDEF			m_stMonSpr [_MAX_MON_MTN];			// Monster 的动作帧定义.
	SPRFRMDEF			m_stNPCSpr [_MAX_MON_MTN];			// Monster 的动作帧定义.

	BYTE				m_bWOrder[_MAX_HERO_REALFRAME];	

public:
	CSprite();
	~CSprite();

	VOID InitSpriteInfo();
	VOID DeleteSpriteInfo();

	VOID SetInfo();

	VOID SetSprDfn(LPSPRFRMDEF pstSprDfn, WORD wFstFrm, WORD wFrmCnt, WORD wDelay);

	VOID SetEffectInfo(LPEFFECTSPRINFO pstEffect, DWORD dwFstFrm, DWORD dwEndFrm, WORD wDelay, WORD wImgIdx, WORD wEffectIdx,
					   BYTE bSLightRadius, BYTE bLLightRadius, 
					   BYTE bSLightRColor, BYTE bSLightGColor, BYTE bSLightBColor,
					   BYTE bLLightRColor, BYTE bLLightGColor, BYTE bLLightBColor, 
					   BYTE bMagicRColor = 255, BYTE bMagicGColor = 255, BYTE bMagicBColor = 255, 
					   BYTE bBlendType = _BLEND_LIGHTINV, BYTE bOpa = 0, BYTE bSwingCnt = 0);

	LPEFFECTSPRINFO GetEffectInfo(WORD wEffectNum);
	LPEFFECTSPRINFO GetExplosionInfo(WORD wEffectNum);
	LPEFFECTSPRINFO GetMagicInfo(WORD wEffectNum);

	VOID			SetMonFrameInfo(WORD wMonNum);

	BOOL			SetWeaponOrder(CHAR* szFileName);
};



#endif //_SPRITEDEF_H



















