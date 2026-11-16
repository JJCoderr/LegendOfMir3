/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/



#ifndef _CMAGIC_H
#define _CMAGIC_H



/******************************************************************************************************************

	CMagic Class
	

*******************************************************************************************************************/
class CMagic
{
private:
protected:
public:
	BYTE			m_bActiveState;							// 魔法是否激活.(魔法消失时使用.)
	BOOL			m_bRepeat;								// 动画是否重复。
	BOOL			m_bFixed;								// 魔法是否移动.
	BYTE			m_bMagicNum;							// 魔法编号.

	CActor*			m_pxOwner;								// 魔法发射主体。
	CActor*			m_pxTarget;								// 魔法发射对象.
	CMapHandler*	m_pxMap;								// 地图引用指针.

	// 坐标信息。
	SHORT	m_shScrnX, m_shScrnY;							// 魔法屏幕坐标。
	FLOAT	m_fFloatScrnX, m_fFloatScrnY;					// 魔法屏幕坐标。
	SHORT	m_shTileX, m_shTileY;							// 魔法图块坐标.

	SHORT	m_shFireScrnX, m_shFireScrnY;					// 魔法生成屏幕坐标.	
	SHORT	m_shFireTileX, m_shFireTileY;					// 魔法生成图块坐标。

	SHORT	m_shTargetScrnX, m_shTargetScrnY;				// 目标点的屏幕坐标。
	SHORT	m_shTargetTileX, m_shTargetTileY;				// 目标点的图块坐标.(可能会变化.)

	// 渲染信息.
	WORD	m_wCurrDelay;									// 延迟时间保存值.
	WORD	m_wCurrLightDelay;
	WORD	m_wFrmDelay;									// 用于翻帧的延迟时间值.
	WORD	m_wMagicLife;									// 魔法正在执行的时间。(单位 ms)


	BYTE	m_bLightRadius[2];								// 光源宽度。
	BYTE	m_bLightColor[2][3];							// 光源颜色.
	BYTE	m_bMagicColor[3];								// 颜色。

	DWORD	m_dwFstFrame;									// 当前魔法的起始帧.
	DWORD	m_dwEndFrame;									// 当前魔法的最后一帧.
	DWORD	m_dwCurrFrame;									// 帧保存值.

	BYTE	m_bBlendType;									// 决定以何种形态绘制魔法。
	BYTE	m_bOpacity;										// 0 - 255 级. 0 为完全透明(光照形态), 255 为不透明.
	BYTE	m_bSwingCnt;									// 设置魔法摇动的条件.

	FLOAT	m_fGradient;
	INT		m_nShiftX;
	INT		m_nShiftY;
	INT		m_nDistanceX;
	INT		m_nDistanceY;
	INT		m_nPrevDistanceX;
	INT		m_nPrevDistanceY;

	BYTE	m_bDir16;
	BYTE	m_bCurrSwing;

	BOOL	m_bShowLight;

	D3DVERTEX			m_avMagic[4];
	CWHWilImageData*	m_pxMagicImg;

	INT		m_nDuplicateNum; 

	LPDIRECTDRAWSURFACE7 m_lpddsMagicTextr[60];

public:
	CMagic();
	~CMagic();

	virtual BOOL CreateMagic(BYTE bMagicNum, SHORT shFireTileX, SHORT shFireTileY, 
		                     SHORT shTargetTileX, SHORT shTargetTileY, CActor* pxOwner = NULL, INT nTarget = 0);


	virtual VOID DestoryMagic();
	virtual BOOL UpdateMagic(INT nLoopTime);
	virtual BOOL DrawMagic();
	virtual BOOL DrawLight(CLightFog* xLightFog, INT nLoopTime);
};


// _SKILL_FIRE, _SKILL_SHOOTLIGHTEN 专用。
class CMagicStream : public CMagic
{
private:
	WORD	m_wMagicStart;
	POINT	m_ptTileGap;
public:
	BOOL CreateMagic(BYTE bMagicNum, SHORT shFireTileX, SHORT shFireTileY, 
		             SHORT shTargetTileX, SHORT shTargetTileY, CActor* pxOwner, INT nTarget, WORD wStartMagicTime, POINT ptTileGap);
	BOOL UpdateMagic(INT nLoopTime);
	BOOL DrawMagic();
};


class CElecMagic : public CMagic
{
private:
	SHORT	m_shEPosX, m_shEPosY;
	BOOL	m_bArrived;

	CElec m_xElec;

public:
	virtual BOOL CreateMagic(BYTE bMagicNum, SHORT shFireTileX, SHORT shFireTileY, 
		                     SHORT shTargetTileX, SHORT shTargetTileY, CActor* pxOwner = NULL, INT nTarget = 0);
	virtual BOOL UpdateMagic(INT nLoopTime);
	virtual BOOL DrawMagic();
};



// _SKILL_HOLYSHIELD, _SKILL_EARTHFIRE 专用。
class CRepeatMagic : public CMagic
{
private:
public:
	DWORD	m_dwMagiLifeTotal;
	INT		m_nEventID;					// 由事件管理.

	_inline CRepeatMagic()
	{
		m_nEventID		  = 0;
		m_dwMagiLifeTotal = 0;
	}

	BOOL CreateMagic(INT nEventID, BYTE bMagicNum, SHORT shFireTileX, SHORT shFireTileY, 
	                 SHORT shTargetTileX, SHORT shTargetTileY, DWORD dwMagicLifeTotal, CActor* pxOwner = NULL, INT nTarget = 0);
	BOOL UpdateMagic(INT nLoopTime);
	BOOL DrawMagic();
};


#endif // _CMAGIC_H
