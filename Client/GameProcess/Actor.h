#include "SpellTiming.h"
#include "LocalActionClock.h"
#include "ForcedMotion.h"
#include <optional>
#include "SpellColor.h"
#ifndef _CACTOR_H
#define _CACTOR_H
#include "DamageNumbers.h"



/******************************************************************************************************************

	CActor Class

*******************************************************************************************************************/
#pragma pack(1)
typedef struct tagMESSAGEBODYWL
{
	LONG	lParam1;
	LONG	lParam2;
	LONG	nTag1;			// 攻击者的 ID.(SM_STRUCK 时).
	LONG	nTag2;
}MESSAGEBODYWL, *LPMESSAGEBODYWL;
typedef struct tagMESSAGEBODYW
{
	WORD	wParam1;
	WORD	wParam2;
	WORD	wTag1;			// 攻击者的 ID.(SM_STRUCK 时).
	WORD	wTag2;
}MESSAGEBODYW, *LPMESSAGEBODYW;

typedef struct tagCHARDESC
{
	LONG	nFeature;
	LONG	nStatus;
}CHARDESC, *LPCHARDESC;

typedef struct tagFEATURE
{
	BYTE	bGender; 
	BYTE	bWeapon;
	BYTE	bDress; 
	BYTE	bHair;
}FEATURE, *LPFEATURE;
#pragma pack(8)



class CActor
{
private:
public:
    std::uint64_t m_instanceSerial=0;
    BYTE m_bProfession=255;
    DWORD m_dwPortraitVersion=0;
    CDamageNumbers m_damage;
    void AddDamage(int value);
    void HealthDamage(int before,int after);
    void DrawDamage();
	DWORD				m_dwIdentity;					// ID Index From Server.
	CHAR				m_szName[64];					// 角色或怪物的名称.
	CHAR				m_szGuildName[64];
    char m_szGuildRank[64];
    bool ReceiveName(const char* encoded,BYTE color);

	FEATURE				m_stFeature;					// 决定角色的外观.
	BYTE				m_bActorImgIdx;					// 当前 Actor 所拥有的图像结构体的编号。
	BYTE				m_bEffectImgIdx;				// 当前 Actor 所拥有的图像结构体的编号。
	CWHWilImageData*	m_pxActorImage;					// 用于获取图像的指针.
	RECT				m_rcActor;						// 当前屏幕上的 Actor 的矩形区域。
	RECT				m_rcTargetRgn;					// 当前 Actor 的目标区域.

	DWORD				m_dwFstFrame;					// 当前动作的起始帧.
	DWORD				m_dwEndFrame;					// 当前动作的最后一帧.
	WORD				m_wDelay;						// 当前动作的延迟时间.

	BYTE				m_bCurrMtn;						// 当前动作.
	BYTE				m_bCurrDir;						// 当前方向.
	BYTE				m_bMoveDir;						// 当前方向.
	DWORD				m_dwCurrFrame;					// 当前帧.
	WORD				m_wCurrDelay;					// 当前延迟时间。

	WORD				m_wOldPosX;						// 当前动作发生之前的图块 X 坐标.
	WORD				m_wOldPosY;
	BYTE				m_bOldDir;						// 当前动作发生之前的方向.
	WORD				m_wPosX;						// 图块 X坐标.
	WORD				m_wPosY;						// 图块 Y 坐标。
	SHORT				m_shShiftPixelX;				// 从当前自身图块沿 X 轴移动的像素距离.
	SHORT				m_shShiftPixelY;				// 当前从自身所在图块沿 Y 轴移动的像素距离。
	SHORT				m_shShiftTileX;					// 从自身当前图块沿 X 轴移动的图块距离。
	SHORT				m_shShiftTileY;					// 从当前自身图块沿 Y 轴移动的图块距离.
	SHORT				m_shScrnPosX;					// 未考虑中心点的屏幕 X 坐标.
	SHORT				m_shScrnPosY;					// 未考虑中心点的屏幕 Y 坐标.
	BYTE				m_bMoveSpeed;					// 移动速度.

	CWHQueue			m_xPacketQueue;					// 保存 Actor 待处理消息的队列.
	BOOL				m_bMsgHurryCheck;				// 消息堆积 2 个以上时，每次跳过 1 帧.

	BYTE				m_bBackStepFrame;
	BYTE				m_bBackStepFrameCnt;

	DWORD				m_dwCurrEffectFrame;			// 角色 Effect 当前帧.
	DWORD				m_dwFstEffectFrame;				// 角色 Effect 起始帧.
	DWORD				m_dwEndEffectFrame;				// 角色 Effect 最后一帧。
	BYTE				m_bEffectFrame;
	BYTE				m_bEffectFrameCnt;
	BYTE				m_bLightRadius[2];				// 光源宽度。
	BYTE				m_bLightColor [2][3];			// 光源颜色.

	BOOL				m_bUseEffect;					// 使用特效帧。
	BOOL				m_bUseSwordEffect;				// 剑法效果使用.

	BOOL				m_bWarMode;
	DWORD				m_dwWarModeTime;

	BYTE				m_bHPPercent;
	WORD				m_wMAXHP;
	WORD				m_wHP;
	WORD				m_wMP;
	BOOL				m_bOpenHealth;

	BYTE				m_bLightSize;					// Actor 周围的光源大小。

	D3DVERTEX			m_avBoard[4];

	WORD				m_wABlendDelay;
	WORD				m_wABlendCurrDelay;
	BOOL				m_bABlendRev;

	BOOL				m_bReverse;

	INT					m_nState;
	WORD				m_wStateClr;

	DWORD				m_dwNameClr;

	BOOL				m_bIsDead;
    bool m_drawnThisFrame=false;

	FEATURE				m_stHitter;

	BYTE				m_bAppearState;

	BYTE				m_bMoveNextFrmCnt;
	BOOL				m_bIsMoved;

	BOOL				m_bFstSoundPlayed;

	INT					m_nDividedChatLine;
	DWORD				m_wCurrChatDelay;
	CHAR				m_szChatMsg[MAX_PATH];			// 聊天消息。
	CHAR				m_szChatMsgArg[8][MAX_PATH];	// 将聊天消息分为 5 个字符串。

public:
//---------------------------------------------------------------------------------------------------------------//
// 构造函数与析构函数.
	CActor();
	~CActor();

	virtual VOID InitActor();
	virtual VOID DestroyActor();
//---------------------------------------------------------------------------------------------------------------//

//---------------------------------------------------------------------------------------------------------------//
// 创建.
	virtual BOOL Create(CImageHandler* pxImgHandler, FEATURE* pstFeature, BYTE bMtn, WORD bDir, WORD wPosX, WORD wPosY);
//---------------------------------------------------------------------------------------------------------------//

	BOOL LoadEffect(CImageHandler* pxImgHandler, WORD wEffectNum, BYTE bDir = 0);
	VOID DrawWithEffected(INT nx, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wChooseColor1 = 0XFFFF, WORD wChooseColor2 = 0XFFFF, BOOL bFocused = FALSE, BYTE bOpa = 50, WORD wState = _STATE_NOTUSED);
//---------------------------------------------------------------------------------------------------------------//
// 帧设置相关.
	BOOL	CheckFeatureValidate(FEATURE stFeature);
	BOOL	ChangeFeature(FEATURE stFeature);
	virtual BOOL SetMotionFrame(BYTE bMtn, BYTE bDir);
//---------------------------------------------------------------------------------------------------------------//

//---------------------------------------------------------------------------------------------------------------//
// 角色移动.
	VOID SetMoving();
	VOID SetBackStepMoving();
	VOID SetMoved();
	WORD GetCharState();
//---------------------------------------------------------------------------------------------------------------//

	bool ReceiveSpeech(const char* message);
    RECT SpeechBounds() const;
    POINT SpeechPosition(int row) const;
    int m_speechWidths[8]={};
	VOID ShowMessage(INT nLoopTime);
	VOID StruckMsgReassign();
	
	virtual VOID PlayActSound();

//---------------------------------------------------------------------------------------------------------------//
// 动作进行.
	virtual BOOL UpdatePacketState();
	virtual VOID UpdateMotionState(INT nLoopTime, BOOL bIsMoveTime);
	virtual BOOL UpdateMove(BOOL bIsMoveTime);

// 按类型应用数据包状态.
	virtual VOID OnCharDescPacket(LPPACKETMSG lpPacketMsg);
	virtual VOID OnUserName(LPPACKETMSG lpPacketMsg);
	virtual VOID OnChangeNameClr(LPPACKETMSG lpPacketMsg);
	virtual VOID OnChangeLight(LPPACKETMSG lpPacketMsg);
	virtual VOID OnOpenHealth(LPPACKETMSG lpPacketMsg);
	virtual VOID OnCloseHealth(LPPACKETMSG lpPacketMsg);
	virtual VOID OnFeatureChanged(LPPACKETMSG lpPacketMsg);
	virtual VOID OnHealthSpellChanged(LPPACKETMSG lpPacketMsg);
	virtual VOID OnWalk(LPPACKETMSG lpPacketMsg);
	virtual VOID OnTurn(LPPACKETMSG lpPacketMsg);
	virtual VOID OnDigup(LPPACKETMSG lpPacketMsg);
	virtual VOID OnDigDown(LPPACKETMSG lpPacketMsg);
	virtual VOID OnDeath(LPPACKETMSG lpPacketMsg);
	virtual VOID OnBackStep(LPPACKETMSG lpPacketMsg);
	virtual VOID OnStruck(LPPACKETMSG lpPacketMsg);
	virtual VOID OnHit(LPPACKETMSG lpPacketMsg);
	virtual VOID OnFlyAxe(LPPACKETMSG lpPacketMsg);
	virtual VOID OnLighting(LPPACKETMSG lpPacketMsg);
//---------------------------------------------------------------------------------------------------------------//

//---------------------------------------------------------------------------------------------------------------//
// Rendering.
	virtual BOOL DrawActor(CMapHandler* pxMap, BOOL bFocused = FALSE, BOOL bShadowAblended = FALSE, 
		                   BOOL bUseScrnPos = TRUE, BOOL bDrawShadow = TRUE);
	virtual VOID DrawHPBar();
	virtual VOID DrawName();

	virtual BOOL DrawEffect();
//---------------------------------------------------------------------------------------------------------------//
protected:
public:
};








/******************************************************************************************************************

	CNPC Class

*******************************************************************************************************************/
class CNPC : public CActor
{
private:
private:
public:
	virtual VOID OnHit(LPPACKETMSG lpPacketMsg);
	virtual VOID OnTurn(LPPACKETMSG lpPacketMsg);
	virtual BOOL UpdatePacketState();
	virtual VOID UpdateMotionState(INT nLoopTime, BOOL bIsMoveTime);
	virtual BOOL DrawActor(CMapHandler* pxMap, BOOL bFocused = FALSE, BOOL bShadowAblended = FALSE, 
		                   BOOL bUseScrnPos = TRUE, BOOL bDrawShadow = TRUE);
};








/******************************************************************************************************************

	CHero Class

*******************************************************************************************************************/
#pragma pack(1)
typedef struct tagFEATUREEX
{
	BYTE		bHorse;
	WORD		wHairColor;
	WORD		wDressColor;
}FEATUREEX, *LPFEATUREEX;
static_assert(sizeof(FEATUREEX)==5, "Appearance extension wire size");
#pragma pack(8)

class CHero : public CActor
{
private:
public:
    std::optional<mir3::ForcedMotion> m_forcedMotion;
    void BeginForcedMotion(LPPACKETMSG packet);
    bool AdvanceForcedMotion(DWORD elapsed);
    DWORD m_spellFrameElapsed=0;
    WORD m_lastSpellReplyId=0;
    bool AdvanceSpellAnimation(DWORD elapsed);

	FEATUREEX			m_stFeatureEx;

	BYTE				m_bWeaponImgIdx;				// 当前 Actor 武器所持有的图像结构体编号.
	BYTE				m_bHairImgIdx;					// 当前 Actor 头部所具有的图像结构体的编号。
	BYTE				m_bHorseImgIdx;					// 当前 Actor 坐骑所具有的图像结构体的编号。

	BYTE				m_bYedoCnt;
	BYTE				m_bFireHitCnt;
	BOOL				m_bUseBanwol;
	BOOL				m_bUseErgum;

	RECT				m_rcHair;
	RECT				m_rcWeapon;
	RECT				m_rcHorse;

	CWHWilImageData*	m_pxHairImage;		
	CWHWilImageData*	m_pxWeaponImage;
	CWHWilImageData*	m_pxHorseImage;

	DWORD				m_dwCurrHairFrame;
	DWORD				m_dwCurrWeaponFrame;
	DWORD				m_dwCurrHorseFrame;

	BYTE				m_bShieldCurrFrm;
	WORD				m_wShieldCurrDelay;

	SHORT				m_shHitSpeed;

	BOOL				m_bIsMon;

	SHORT				m_shCurrMagicID;

protected:
public:
	CHero();
	~CHero();

	virtual VOID InitActor();
	virtual VOID DestroyActor();

	VOID ShowShield();
	VOID PlayMoveSound();

	virtual VOID PlayActSound();
	virtual BOOL ChangeFeature(FEATURE stFeature, FEATUREEX stFeatureEx);
	virtual BOOL Create(CImageHandler* pxImgHandler, BYTE bMtn, BYTE bDir, WORD wPosX, WORD wPosY, FEATURE* pstFeature, FEATUREEX* pstFeatureEx);
	virtual BOOL UpdatePacketState();
	virtual VOID UpdateMotionState(INT nLoopTime, BOOL bIsMoveTime);
	virtual BOOL UpdateMove(BOOL bIsMoveTime);
	virtual VOID OnRun(LPPACKETMSG lpPacketMsg);
	virtual VOID OnRush(LPPACKETMSG lpPacketMsg);
	virtual VOID OnMagicFire(LPPACKETMSG lpPacketMsg);
    void OnMagicList(LPPACKETMSG packet);
	mir3::SpellColor::Rgb m_spellTint={};
    bool m_spellLightActive=false;
    BOOL DrawEffect() override;
    void BeginSpellAction(WORD skill,BYTE facing,BYTE action);
    void ApplySpellPhase();
	virtual VOID OnSpell(LPPACKETMSG lpPacketMsg);
	virtual VOID OnCharDescPacket(LPPACKETMSG lpPacketMsg);
	virtual VOID OnFeatureChanged(LPPACKETMSG lpPacketMsg);
	virtual VOID OnCharStatusChanged(LPPACKETMSG lpPacketMsg);
	virtual VOID OnBackStep(LPPACKETMSG lpPacketMsg);
	virtual VOID OnDeath(LPPACKETMSG lpPacketMsg);
	virtual VOID OnWalk(LPPACKETMSG lpPacketMsg);
	virtual VOID OnTurn(LPPACKETMSG lpPacketMsg);
	virtual VOID OnStruck(LPPACKETMSG lpPacketMsg);
	virtual VOID OnHit(LPPACKETMSG lpPacketMsg);
	virtual VOID OnButch(LPPACKETMSG lpPacketMsg);
	virtual BOOL DrawActor(CMapHandler* pxMap, BOOL bFocused = FALSE, BOOL bShadowAblended = FALSE, 
		                   BOOL bUseScrnPos = TRUE, BOOL bDrawShadow = TRUE);
};










/******************************************************************************************************************

	CMyHero Class

*******************************************************************************************************************/
#pragma pack(1)
typedef struct tagACTORABILITY
{
    WORD	wAC;
    WORD	wMAC;
	WORD	wDC;
    WORD	wMC;
    WORD	wSC;
	WORD	wHP;
	WORD	wMP;
    WORD	wMaxHP;
    WORD	wMaxMP;
	WORD	wWeight;
 	BYTE	bExpCount;
	BYTE	bExpMaxCount;
	BYTE	bLevel;
	BYTE	bReserved1;
	BYTE	bWearWeight;
	BYTE	bMaxWearWeight;
    BYTE	bHandWeight;
    BYTE	bMaxHandWeight;

	DWORD	dwExp;
	DWORD	dwMaxExp;
	WORD	wMaxWeight;
}ACTORABILITY, *LPACTORABILITY;

typedef struct tagACTORSUBABILITY
{
	WORD wAntiMagic;		// 魔法回避。
	BYTE bHitPoint;			// 准确.
	BYTE bSpeedPoint;		// 敏捷.
	BYTE bAntiPoison;		// 中毒回避.
	BYTE bPoisonRecover;	// 中毒恢复.
	BYTE bHealthRecover;	// 体力恢复。
	BYTE bSpellRecover;		// 魔法力恢复.
}ACTORSUBABILITY, *LPACTORSUBABILITY;
#pragma pack(8)

class CMyHero : public CHero
{
public:
    mir3::SpellTiming m_spellTiming;
    mir3::LocalActionClock m_actionClock;
    struct PendingLocalAction {
        BYTE motion,direction;
        int target;BOOL targetDead;
        POINT destination,origin;SHORT skill;
        std::optional<DWORD> waitingSince;
        DWORD wait=0;
    };
    std::optional<PendingLocalAction> m_pendingAction;
    DWORD AttackInterval() const;
    void AdvancePendingAction();
    void ExecuteMotionState(BYTE motion,BYTE direction,INT target,BOOL targetDead,LPPOINT position,SHORT skill);
    bool m_spellAwaitingReply=false;
    DWORD m_spellReplyElapsed=0;
    void BeginSpellReply() {m_spellAwaitingReply=true;m_spellReplyElapsed=0;}
    void ConfirmAction();
private:
public:
//---------------------------------------------------------------------------------------------------------------//
//  角色校正用变量.
	DWORD			m_dwMotionLockTime;
	BOOL			m_bInputLock;				// 角色动作执行中不接受任何输入.
	BOOL			m_bMotionLock;				// 角色动作设定后，在服务器发来动作确认之前不接受任何动作.
//---------------------------------------------------------------------------------------------------------------//

	CMapHandler*	m_pxMap;
	BOOL			m_bCanRun;
	BYTE			m_bAttackMode;
	ACTORABILITY    m_stAbility;
	ACTORSUBABILITY m_stSubAbility;
	BYTE		    m_bJob;
	UINT			m_nGlod;

	CWHQueue		m_xPriorPacketQueue;					// 保存 Actor 待处理消息的队列.

//---------------------------------------------------------------------------------------------------------------//
//  角色 Delay 相关变量。
	DWORD			m_dwLastHitTime;						// 刀击命中时的 tick 保存值。
	DWORD			m_dwLastSpellTime;						// 魔法生效时的 tick 保存值。
	DWORD			m_dwLastMagicTime;						// 炎火诀施放后保存的 tick 值。
	DWORD			m_dwLastStruckTime;						// 受击时的 tick 保存值.
	DWORD			m_dwLastPKStruckTime;
	DWORD			m_dwLastRushTime;						// 无胎步施放后的 tick 保存值.
	DWORD			m_dwLastFireHitTime;					// 炎火诀施放后保存的 tick 值。

	WORD			m_wMagicPKDelayTime;					// 用魔法攻击人物时的惩罚 Delay.(适用于行走和奔跑。)
	WORD			m_wMagicDelayTime;						// 施放魔法后,到下一个魔法进入之前的 Delay.
//---------------------------------------------------------------------------------------------------------------//

protected:
public:
	CMyHero();
	~CMyHero();

	virtual VOID InitActor();
	virtual VOID DestroyActor();

	VOID  SetMotionState(BYTE bMtn, BYTE bDir, INT nMouseTargetID = NULL, BOOL bIsDead = FALSE, LPPOINT lpptPos = NULL, SHORT shSkill = -1);
	VOID  SetMapHandler(CMapHandler* pxMap);
    bool AttackReady() const;
    bool AttackAdjacent(CActor& target);
    bool WalkAdjacent(POINT destination);
    bool CastShortcut(int slot,POINT target,int targetId,FEATURE feature,bool repeat=false);
	VOID  SetMagic(LPCLIENTMAGICRCD	pstMagic, BYTE bKeyNum, BYTE bDir, INT nTargetID, FEATURE stTargetFeature, POINT ptTargetPos);
	BOOL  SetOldPosition();
	BOOL  CheckMyPostion();
	VOID  AdjustMyPostion();
	BYTE  CalcDirection(INT nTargetTileX, INT nTargetTileY);
	POINT GetPosMouseToTile(INT nXPos, INT nYPos);

	virtual BOOL Create(CImageHandler* pxImgHandler, BYTE bMtn, BYTE bDir, WORD wPosX, WORD wPosY, FEATURE* pstFeature, FEATUREEX* pstFeatureEx);
	virtual VOID OnHealthSpellChanged(LPPACKETMSG lpPacketMsg);
	virtual BOOL UpdatePacketState();
	virtual VOID UpdateMotionState(INT nLoopTime, BOOL bIsMoveTime);
	virtual BOOL UpdateMove(BOOL bIsMoveTime);
	virtual BOOL DrawActor(BOOL bFocused = FALSE, BOOL bShadowAblended = FALSE, 
		                   BOOL bUseScrnPos = FALSE, BOOL bDrawShadow = TRUE);

	__inline virtual BOOL ChangeFeature(FEATURE stFeature, FEATUREEX stFeatureEx)
	{
		if ( m_bIsMon )		
		{
			if ( CActor::ChangeFeature(stFeature) )					return TRUE;
		}
		else	
		{
			if ( CHero::ChangeFeature(stFeature, stFeatureEx) )		return TRUE;
		}

		return FALSE;
	}

//---------------------------------------------------------------------------------------------------------------//
//  角色 Delay 相关函数。
	BOOL CanNextHit();									// 现在是否可以挥刀攻击。随等级和装备重量而变化。
	BOOL CanWalk();										// 与 Delay 相关联,角色是否可以移动的判断.
	BOOL CanRun();										// 与 Delay 相关联,角色是否可以移动的判断.
//---------------------------------------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------------------------------------//
//  用户输入函数.
	BOOL OnLButtonDown(POINT ptMouse, INT nTargetID = -1, BOOL bIsDead = FALSE, POINT* lpptTargetPos = NULL);
	BOOL OnRButtonDown(POINT ptMouse);
	BOOL OnKeyDown(WPARAM wParam, LPARAM lParam, POINT ptMouse, POINT ptTargetPos, INT nMosueTargetID, FEATURE stTargetFeature);
//---------------------------------------------------------------------------------------------------------------//
};








#endif // _CACTOR_H