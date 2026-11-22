/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/



#include "StdAfx.h"
#include "EffectRenderer.h"
#include "SpellReply.h"
#include "Common/Mir3HumanAppearance.h"
#include "MonsterMotion.h"
#include "Common/LegacyPanel.h"



/******************************************************************************************************************

	CActor Class

*******************************************************************************************************************/

CActor::CActor()
{
	InitActor();
}


CActor::~CActor()
{
	DestroyActor();
}

VOID CActor::InitActor()
{
    static std::uint64_t nextInstance=0;m_instanceSerial=++nextInstance;
	m_wOldPosX			= 0;
	m_wOldPosY			= 0;
	m_bOldDir			= 0;

	m_bOpenHealth		= FALSE;
	m_bWarMode			= FALSE;
	m_bCurrMtn			= _MT_MON_STAND;
	m_bCurrDir			= _DIRECTION_LIST_1;
	m_bMoveDir			= _DIRECTION_LIST_1;
	m_bEffectFrameCnt	= _DEFAULT_SPELLFRAME;
	m_bHPPercent		= 100;

	m_bMoveNextFrmCnt	= 5;
	m_bIsMoved			= FALSE;

	m_dwWarModeTime		= 0;
	m_wMAXHP			= 0;
    m_bProfession=255;m_dwPortraitVersion=0;
    m_damage=CDamageNumbers{};
	m_wHP				= 0;
	m_wMP				= 0;
	m_wPosX				= 0;
	m_wPosY				= 0;
	m_bLightSize		= 2;
	m_shShiftPixelX		= 0;
	m_shShiftPixelY		= 0;
	m_shShiftTileX		= 0;
	m_shShiftTileY		= 0;
	m_bMoveSpeed		= 0;
	m_shScrnPosX		= 0;
	m_shScrnPosY		= 0;
	m_wCurrDelay		= 0;
	m_dwFstFrame		= 0;
	m_dwEndFrame		= 1;
	m_dwCurrFrame		= 0;	
	m_wDelay			= 0;		
	m_pxActorImage		= NULL;
	m_bMsgHurryCheck	= FALSE;
	m_bUseEffect		= FALSE;
	m_bUseSwordEffect	= FALSE;
	m_bReverse			= FALSE;
	m_bIsDead			= FALSE;
	m_bABlendRev		= FALSE;
	m_dwCurrEffectFrame = 0;
	m_dwFstEffectFrame	= 0;
	m_dwEndEffectFrame	= 0;
	m_bEffectFrame		= 0;
	m_bBackStepFrame	= 0;
	m_bBackStepFrameCnt = 0;
	m_nState			= 0;
	m_nDividedChatLine	= 0;
	m_dwIdentity		= 0;
	m_wCurrChatDelay	= 0;
	m_wABlendCurrDelay	= 0;
	m_wABlendCurrDelay	= 0;

	m_bFstSoundPlayed   = FALSE;

	m_bAppearState		= _DIG_NORMAL;

	m_wStateClr			= _STATE_NOTUSED;
	m_dwNameClr			= RGB(255, 255, 255);

	m_bActorImgIdx		= _IMAGE_M_HUMAN;
	m_bEffectImgIdx		= _IMAGE_MAGIC;

	ZeroMemory(m_szName, 64);
    ZeroMemory(m_szGuildName,sizeof(m_szGuildName));
    ZeroMemory(m_szGuildRank,sizeof(m_szGuildRank));
	ZeroMemory(&m_stFeature,   sizeof(FEATURE));
	ZeroMemory(&m_stHitter,    sizeof(FEATURE));
	ZeroMemory(m_bLightRadius, sizeof(BYTE)*2);
	ZeroMemory(m_bLightColor , sizeof(BYTE)*2*3);
	ZeroMemory(m_szChatMsg, MAX_PATH);
	ZeroMemory(m_szChatMsgArg, sizeof(m_szChatMsgArg));
    ZeroMemory(m_speechWidths,sizeof(m_speechWidths));
	ZeroMemory(&m_rcActor, sizeof(RECT));
	ZeroMemory(&m_rcTargetRgn, sizeof(RECT));

	D3DVECTOR		vNorm(0, 0, -1);
	m_avBoard[0] = D3DVERTEX(D3DVECTOR(-0.5f, 0.5f, 0), vNorm, 0, 0);
	m_avBoard[1] = D3DVERTEX(D3DVECTOR(-0.5f,-0.5f, 0), vNorm, 0, 1);
	m_avBoard[2] = D3DVERTEX(D3DVECTOR( 0.5f, 0.5f, 0), vNorm, 1, 0);
	m_avBoard[3] = D3DVERTEX(D3DVECTOR( 0.5f,-0.5f, 0), vNorm, 1, 1);
}


VOID CActor::DestroyActor()
{
	INT			nCnt;
	SHORT		shLeftMsgCnt;
	LPPACKETMSG	lpPacketMsg;

	lpPacketMsg  = NULL;
	shLeftMsgCnt = m_xPacketQueue.GetCount();

	// 清除已堆积的数据包。
	if ( shLeftMsgCnt > 0 )
	{
		for ( nCnt = 0; nCnt < shLeftMsgCnt; nCnt++ )
		{
			lpPacketMsg = (LPPACKETMSG)m_xPacketQueue.PopQ();
			if ( lpPacketMsg )
			{
				SAFE_DELETE(lpPacketMsg);
			}
		}
	}

	// 初始化全部变量.
	InitActor();
}


WORD CActor::GetCharState()
{		
	WORD wCharState = 0XFFFF;

//	if( m_nState &   0X100000 )			wCharState = _STATE_SHIELDUSE;
	if( m_nState &   0X800000 )			wCharState = _STATE_ABLEND;
	if( m_nState &  0X4000000 )			wCharState = _STATE_GRAY;
	if( m_nState &  0X8000000 )			wCharState = _STATE_FUCHSIA;
	if( m_nState & 0X10000000 )			wCharState = _STATE_YELLOW;
	if( m_nState & 0X20000000 )			wCharState = _STATE_BLUE;
	if( m_nState & 0X40000000 )			wCharState = _STATE_RED;
	if( m_nState & 0X80000000 )			wCharState = _STATE_GREEN;

	m_wStateClr = _STATE_NOTUSED;

 	if ( m_wStateClr == _STATE_ABLEND || m_wStateClr == _STATE_SHIELDUSE )
		m_wStateClr = _STATE_NOTUSED;

	return wCharState;
}


VOID CActor::DrawWithEffected(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wChooseColor1, WORD wChooseColor2, BOOL bFocused, BYTE bOpa, WORD wState)
{
	switch ( wState )
	{
	case _STATE_RED:
	case _STATE_GREEN:
	case _STATE_BLUE:		
	case _STATE_YELLOW:
	case _STATE_FUCHSIA:	
	case _STATE_GRAY:
		if ( m_wABlendDelay )
			g_xMainWnd.DrawWithImageForCompClipRgnColor(nX, nY, nXSize, nYSize, pwSrc, _CLIP_WIDTH, _CLIP_HEIGHT, wState, bFocused, TRUE);
		else
			g_xMainWnd.DrawWithImageForCompClipRgnColor(nX, nY, nXSize, nYSize, pwSrc, _CLIP_WIDTH, _CLIP_HEIGHT, wState, bFocused, FALSE);
		break;
	case _STATE_ABLEND:
		{
			if ( bFocused )		bOpa -= 20;

			if ( bOpa < 0 && bOpa > 100 )		bOpa = 0;
			g_xMainWnd.DrawWithABlendCompDataWithBackBuffer(nX, nY, nXSize, nYSize, pwSrc, _CLIP_WIDTH, _CLIP_HEIGHT, wChooseColor1, wChooseColor2, bOpa);
		}
		break;
	case _STATE_SHIELDUSE:
	case _STATE_NOTUSED:
	default:
		{
		if ( !m_wABlendDelay )
			g_xMainWnd.DrawWithImageForCompClipRgn(nX, nY, nXSize, nYSize, pwSrc, _CLIP_WIDTH, _CLIP_HEIGHT, wChooseColor1, wChooseColor2, bFocused);
		else
			g_xMainWnd.DrawWithABlendCompDataWithBackBuffer(nX, nY, nXSize, nYSize, pwSrc, _CLIP_WIDTH, _CLIP_HEIGHT, wChooseColor1, wChooseColor2, bOpa);
		}
		break;
	}
}


BOOL CActor::Create(CImageHandler* pxImgHandler, FEATURE* pstFeature, BYTE bMtn, WORD bDir, WORD wPosX, WORD wPosY)
{
	// 参数应用及确认./////////////////////////////////////////////////////////////////////////////////
	if ( /*(pstFeature->bGender >= 0 && pstFeature->bGender < _MAX_GENDER) && */(bDir >= 0 && bDir < _MAX_DIRECTION) )
	{
		switch ( pstFeature->bGender )
		{
		case _GENDER_MAN:
			{
				if ( (pstFeature->bDress < 0 && pstFeature->bDress >= _MAX_HERO_KIND) ||
					 (bMtn < 0 && bMtn >= _MAX_HERO_MTN) )	
					return FALSE;
				else	
				{
					m_bActorImgIdx = _IMAGE_M_HUMAN;
					m_dwFstFrame = g_xSpriteInfo.m_stHeroSpr[bMtn].wFstFrm + pstFeature->bDress*_MAX_HERO_FRAME + bDir*10;
					m_dwEndFrame = m_dwFstFrame + g_xSpriteInfo.m_stHeroSpr[bMtn].wFrmCnt;
					m_wDelay = g_xSpriteInfo.m_stHeroSpr[bMtn].wDelay;


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					pstFeature->bHair = 2;
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


				}
			}
			break;
		case _GENDER_WOMAN:
			{
				if ( (pstFeature->bDress < 0 && pstFeature->bDress >= _MAX_HERO_KIND) ||
					 (bMtn < 0 && bMtn >= _MAX_HERO_MTN) )	
					return FALSE;
				else
				{
					m_bActorImgIdx = _IMAGE_WM_HUMAN;
					m_dwFstFrame = g_xSpriteInfo.m_stHeroSpr[bMtn].wFstFrm + pstFeature->bDress*_MAX_HERO_FRAME + bDir*10;
					m_dwEndFrame = m_dwFstFrame + g_xSpriteInfo.m_stHeroSpr[bMtn].wFrmCnt;
					m_wDelay = g_xSpriteInfo.m_stHeroSpr[bMtn].wDelay;


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					pstFeature->bHair = 2;
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


				}
			}
			break;
		case _GENDER_MON:
			{	
				if ( (pstFeature->bDress < 0 && pstFeature->bDress >= _MAX_MON_KIND) ||
					 (bMtn < 0 && bMtn >= _MAX_MON_MTN) )	
					return FALSE;
				else
				{
					g_xSpriteInfo.SetMonFrameInfo(pstFeature->bDress);
					// 原版 406750 按动作起始帧重判朝向，见 MonsterMotion.h。
					bDir = (BYTE)mir3::MonsterResourceDirection(pstFeature->bDress, g_xSpriteInfo.m_stMonSpr[bMtn].wFstFrm, bDir);
					m_bActorImgIdx = _IMAGE_MONSTER1 + (pstFeature->bDress / 10);
					m_dwFstFrame = g_xSpriteInfo.m_stMonSpr[bMtn].wFstFrm + (pstFeature->bDress%10)*_MAX_MON_FRAME + bDir*10;
					m_dwEndFrame = m_dwFstFrame + g_xSpriteInfo.m_stMonSpr[bMtn].wFrmCnt;
					m_wDelay = g_xSpriteInfo.m_stMonSpr[bMtn].wDelay;
				}
			}
			break;
		case _GENDER_NPC:
			{
				if ( (pstFeature->bDress < 0 && pstFeature->bDress >= _MAX_NPC_KIND) ||
					 (bMtn < 0 && bMtn >= _MAX_NPC_MTN) )	
					return FALSE;
				else
				{
					m_bActorImgIdx = _IMAGE_NPC;
					bDir++;		// 因为 bDir 也可能为 0。
					bDir = bDir/3;
					m_dwFstFrame = g_xSpriteInfo.m_stNPCSpr[bMtn].wFstFrm + pstFeature->bDress*_MAX_NPC_FRAME + bDir*10;
					m_dwEndFrame = m_dwFstFrame + g_xSpriteInfo.m_stNPCSpr[bMtn].wFrmCnt;
					m_wDelay = g_xSpriteInfo.m_stNPCSpr[bMtn].wDelay;
				}
			}
			break;
		default:
			return FALSE;
		}
		
		m_bCurrMtn		= bMtn;
		m_bCurrDir		= bDir;
		m_bMoveDir		= bDir;
		m_wPosX			= wPosX;
		m_wPosY			= wPosY;
		
		memcpy(&m_stFeature, pstFeature, sizeof(FEATURE));

		m_pxActorImage	= &(pxImgHandler->m_xImageList[m_bActorImgIdx]);
		m_dwCurrFrame	= m_dwFstFrame;
		m_bMoveDir		= m_bCurrDir;

		D3DVECTOR		vNorm(0, 0, -1);
		m_avBoard[0] = D3DVERTEX(D3DVECTOR(-0.5f, 0.5f, 0), vNorm, 0, 0);
		m_avBoard[1] = D3DVERTEX(D3DVECTOR(-0.5f,-0.5f, 0), vNorm, 0, 1);
		m_avBoard[2] = D3DVERTEX(D3DVECTOR( 0.5f, 0.5f, 0), vNorm, 1, 0);
		m_avBoard[3] = D3DVERTEX(D3DVECTOR( 0.5f,-0.5f, 0), vNorm, 1, 1);

		return TRUE;
	}
	
	return FALSE;
}



BOOL CActor::CheckFeatureValidate(FEATURE stFeature)
{
//	if ( (stFeature.bGender >= 0 && stFeature.bGender < _MAX_GENDER) )
	{
		switch ( stFeature.bGender )
		{
		case _GENDER_MAN:
		case _GENDER_WOMAN:
			if ( stFeature.bDress < 0 && stFeature.bDress >= _MAX_HERO_KIND )		return FALSE;
			break;
		case _GENDER_MON:
			{
				if ( stFeature.bDress < 0 && stFeature.bDress >= _MAX_MON_KIND )		return FALSE;
			}
			break;
		case _GENDER_NPC:
		default:
			return FALSE;
		}
	}
	return TRUE;
}



BOOL CActor::ChangeFeature(FEATURE stFeature)
{
	CImageHandler* pxImgHandler = &g_xGameProc.m_xImage;

	if ( /*stFeature.bGender >= 0 && stFeature.bGender < _MAX_GENDER && */CheckFeatureValidate(stFeature) )
	{
		switch ( stFeature.bGender )
		{
		case _GENDER_MAN:
			{
				if ( stFeature.bDress < 0 && stFeature.bDress >= _MAX_HERO_KIND )
					return FALSE;
				else	
				{
					m_bActorImgIdx = _IMAGE_M_HUMAN;
					m_dwFstFrame = g_xSpriteInfo.m_stHeroSpr[m_bCurrMtn].wFstFrm + stFeature.bDress*_MAX_HERO_FRAME + m_bCurrDir*10;
					m_dwEndFrame = m_dwFstFrame + g_xSpriteInfo.m_stHeroSpr[m_bCurrMtn].wFrmCnt;
					m_wDelay = g_xSpriteInfo.m_stHeroSpr[m_bCurrMtn].wDelay;



//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					// Preserve the hairstyle supplied by the server.
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!



				}
			}
			break;
		case _GENDER_WOMAN:
			{
				if ( stFeature.bDress < 0 && stFeature.bDress >= _MAX_HERO_KIND )
					return FALSE;
				else
				{
					m_bActorImgIdx = _IMAGE_WM_HUMAN;
					m_dwFstFrame = g_xSpriteInfo.m_stHeroSpr[m_bCurrMtn].wFstFrm + stFeature.bDress*_MAX_HERO_FRAME + m_bCurrDir*10;
					m_dwEndFrame = m_dwFstFrame + g_xSpriteInfo.m_stHeroSpr[m_bCurrMtn].wFrmCnt;
					m_wDelay = g_xSpriteInfo.m_stHeroSpr[m_bCurrMtn].wDelay;


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					// Preserve the hairstyle supplied by the server.
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!



				}
			}
			break;
		case _GENDER_MON:
			{
				if ( stFeature.bDress < 0 && stFeature.bDress >= _MAX_MON_KIND )
					return FALSE;
				else
				{
					g_xSpriteInfo.SetMonFrameInfo(stFeature.bDress);
					m_bActorImgIdx = _IMAGE_MONSTER1 + (stFeature.bDress/10);
					m_dwFstFrame = g_xSpriteInfo.m_stMonSpr[m_bCurrMtn].wFstFrm + (stFeature.bDress%10)*_MAX_MON_FRAME + m_bCurrDir*10;
					m_dwEndFrame = m_dwFstFrame + g_xSpriteInfo.m_stMonSpr[m_bCurrMtn].wFrmCnt;
					m_wDelay = g_xSpriteInfo.m_stMonSpr[m_bCurrMtn].wDelay;
				}
			}
			break;
		// NPC 的帧不固定, 因此单独应用。
		case _GENDER_NPC:
		default:
			return FALSE;
		}
		m_stFeature		= stFeature;
		m_pxActorImage	= &(pxImgHandler->m_xImageList[m_bActorImgIdx]);
		m_dwCurrFrame	= m_dwFstFrame;

		return TRUE;
	}
	
	return FALSE;
}


BOOL CActor::SetMotionFrame(BYTE bMtn, BYTE bDir)
{
	if ( /*(bMtn < 0 && bMtn >= _MAX_HERO_MTN) || */(bDir < 0 && bDir >= _MAX_DIRECTION) )
		return FALSE;

	switch ( m_stFeature.bGender )
	{
	case _GENDER_MAN:
		{
			m_dwFstFrame = g_xSpriteInfo.m_stHeroSpr[bMtn].wFstFrm + m_stFeature.bDress*_MAX_HERO_FRAME + bDir*10;
			m_dwEndFrame = m_dwFstFrame + g_xSpriteInfo.m_stHeroSpr[bMtn].wFrmCnt;
			m_wDelay = g_xSpriteInfo.m_stHeroSpr[bMtn].wDelay;
		}
		break;
	case _GENDER_WOMAN:
		{
			m_dwFstFrame = g_xSpriteInfo.m_stHeroSpr[bMtn].wFstFrm + m_stFeature.bDress*_MAX_HERO_FRAME + bDir*10;
			m_dwEndFrame = m_dwFstFrame + g_xSpriteInfo.m_stHeroSpr[bMtn].wFrmCnt;
			m_wDelay = g_xSpriteInfo.m_stHeroSpr[bMtn].wDelay;
		}
		break;
	case _GENDER_MON:
		{
			g_xSpriteInfo.SetMonFrameInfo(m_stFeature.bDress);
			// 原版 406750 按动作起始帧重判朝向，见 MonsterMotion.h。
			bDir = (BYTE)mir3::MonsterResourceDirection(m_stFeature.bDress, g_xSpriteInfo.m_stMonSpr[bMtn].wFstFrm, bDir);
			m_dwFstFrame = g_xSpriteInfo.m_stMonSpr[bMtn].wFstFrm + (m_stFeature.bDress%10)*_MAX_MON_FRAME + bDir*10;
			m_dwEndFrame = m_dwFstFrame + g_xSpriteInfo.m_stMonSpr[bMtn].wFrmCnt;
			m_wDelay = g_xSpriteInfo.m_stMonSpr[bMtn].wDelay;
		}
		break;
	case _GENDER_NPC:
		{
			bDir++;		// 因为 bDir 也可能为 0。
			bDir = bDir/3;
			m_dwFstFrame = g_xSpriteInfo.m_stNPCSpr[bMtn].wFstFrm + m_stFeature.bDress*_MAX_NPC_FRAME + bDir*10;
			m_dwEndFrame = m_dwFstFrame + g_xSpriteInfo.m_stNPCSpr[bMtn].wFrmCnt;
			m_wDelay = g_xSpriteInfo.m_stNPCSpr[bMtn].wDelay;
		}
		break;
	default:
		return FALSE;
	}

	m_bCurrMtn		= bMtn;
	m_bCurrDir		= bDir;
	m_bMoveDir		= bDir;
	m_dwCurrFrame	= m_dwFstFrame;
	m_wCurrDelay	= 0;

	if ( m_bCurrDir == _DIRECTION_LIST_8 )
		m_bMoveNextFrmCnt = 2;
	else
		m_bMoveNextFrmCnt = 5;


	return TRUE;
}


VOID CActor::PlayActSound()
{
	INT nWaveNum = -1;

	const INT nActorSndTbl[100] = 
	{
		 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,	//  0 -  9
		 -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,	// 10 - 19
		160, 161, 100, 101, 102, 163,   0, 162,  83,  80,	// 20 - 29
		  1,  10,  20,  21,  22,  23, 150,  24,  25,  26,	// 30 - 39
		 27,  30,  32,  31, 151,  34,  28,  18,  40,  50,	// 40 - 49
		 51,  52,  53, 152,  36,  72,  37,  38,  43,  44,	// 50 - 59
		 45,  48,  49,  90,  91,  70,  73, 140,  74, 120,	// 60 - 69
		121,  81,  82,  41,  42,  39, 110, 111, 112, 130,	// 70 - 79
		164,  46,  47,  61,  62,  63,  71,  -1,  -1, 170,	// 80 - 89
		171,  33,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,	// 90 - 99
	};

	// 表外外观没有专用音效，避免按外观编号越界读取。
	constexpr INT ActorSoundGroups = sizeof(nActorSndTbl)/sizeof(nActorSndTbl[0]);
	const INT nShape = m_stFeature.bDress;
	const INT nSoundGroup = ( nShape >= 0 && nShape < ActorSoundGroups ) ? nActorSndTbl[nShape] : -1;
	const auto GroupSample = [nSoundGroup](INT nOffset) { return nSoundGroup < 0 ? -1 : 200 + nSoundGroup*10 + nOffset; };

	if ( m_dwCurrFrame == m_dwFstFrame+1 && m_bCurrMtn == _MT_MON_APPEAR )			// 出现。
	{
		nWaveNum = GroupSample(0);
	}
	else if ( (m_dwCurrFrame == m_dwFstFrame+1) && (m_bCurrMtn == _MT_MON_STAND || _MT_MON_WALK) )		// 静止不动.
	{
		INT nRand = rand();

		if ( m_bFstSoundPlayed && !m_bIsDead )
		{
			if ( !(nRand%25) )
				nWaveNum = GroupSample(1);
			else
				nWaveNum = -1;
		}
		else
		{
			if ( !(nRand%3) && !m_bIsDead )
			{
				nWaveNum = GroupSample(1);
				m_bFstSoundPlayed = TRUE;
			}
			else
				nWaveNum = -1;
		}
	}
	else if ( m_dwCurrFrame == m_dwFstFrame+1 && m_bCurrMtn == _MT_MON_ATTACK_A )	// 普通攻击1.
	{
		nWaveNum = GroupSample(2);
	}
	else if ( m_dwCurrFrame == m_dwFstFrame+2 && m_bCurrMtn == _MT_MON_ATTACK_A )	// 普通攻击1.
	{
		nWaveNum = GroupSample(3);
	}
	else if ( m_dwCurrFrame == m_dwFstFrame+1 && m_bCurrMtn == _MT_MON_HITTED )
	{
		if ( m_stFeature.bGender < 2 )
		{
			switch ( m_stHitter.bWeapon )
			{
				case 21: case 24: // 短剑，非短剑.
				case 8: case 9: // 木剑, 阿里水木剑.
				case 18:  case 22:  case 23:  case 26:  case 27:  case 28:  case 30:  // 蛇矛剑. 青铜剑. 铁剑. 青音剑. 辟邪剑. 天灵. 哭声剑.
				case 1:   case 4:  case 11:  case 13:  case 14:  case 20:  case 25:  case 29:  case 31:   // 柳月刀. 墨青大刀. 六合刀. 军刀. 屠龙宝刀. 四角刀. 细尖刀. 锐刀. 招魂刀.
					nWaveNum = 70;
					break;
				case 5:  case 10:  case 12: // 三戟大斧. 青铜斧. 连子斧.
				case 15:  // 破雷震堂.
					nWaveNum = 71;
					break;
				case 2:  case 3:  case 6:  case 7: case 16:  case 17: case 19:  // 三叉戟. 天刑木. 红牙枪. 镐. 青马枪, 龙牙杖. 制魔棒
					nWaveNum = 72;
					break;
				default:
					nWaveNum = 73;	// 空手。
					break;
			}
		}
	}
	else if ( m_dwCurrFrame == m_dwFstFrame+2 && m_bCurrMtn == _MT_MON_HITTED )		// 受击.
	{
		nWaveNum = GroupSample(4);
	}
	else if ( m_dwCurrFrame == m_dwFstFrame+2 && m_bCurrMtn == _MT_MON_DIE )		// 死亡1.
	{
		nWaveNum = GroupSample(5);
	}
	else if ( m_dwCurrFrame == m_dwFstFrame+3 && m_bCurrMtn == _MT_MON_DIE )		// 死亡2.
	{
		nWaveNum = GroupSample(6);
	}

	if ( nWaveNum != -1 )
		g_xSound.PlayActorWav(m_wPosX, m_wPosY, g_xGameProc.m_xMyHero.m_wPosX, g_xGameProc.m_xMyHero.m_wPosY, nWaveNum);

}



VOID CActor::SetMoving()
{	
	WORD wFrmCnt  = m_dwEndFrame  - m_dwFstFrame;
	WORD wCurrFrm = m_dwCurrFrame - m_dwFstFrame;

	switch ( m_bMoveDir )
	{
	case _DIRECTION_LIST_1:
		m_shShiftPixelX =  0;

		if ( wCurrFrm < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelY = -(_CELL_HEIGHT/wFrmCnt * (wCurrFrm+1))*m_bMoveSpeed;
		else
			m_shShiftPixelY =  (_CELL_HEIGHT/wFrmCnt * (wFrmCnt - wCurrFrm -1))*m_bMoveSpeed;
		break;
	case _DIRECTION_LIST_2:
		if ( wCurrFrm < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelX =  (_CELL_WIDTH /wFrmCnt * (wCurrFrm+1))*m_bMoveSpeed;
		else
			m_shShiftPixelX = -(_CELL_WIDTH /wFrmCnt * (wFrmCnt - wCurrFrm -1))*m_bMoveSpeed;

		if ( wCurrFrm < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelY = -(_CELL_HEIGHT/wFrmCnt * (wCurrFrm+1))*m_bMoveSpeed;
		else
			m_shShiftPixelY =  (_CELL_HEIGHT/wFrmCnt * (wFrmCnt - wCurrFrm -1))*m_bMoveSpeed;
		break;
	case _DIRECTION_LIST_3:
		if ( wCurrFrm < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelX =  (_CELL_WIDTH /wFrmCnt * (wCurrFrm+1))*m_bMoveSpeed;
		else
			m_shShiftPixelX = -(_CELL_WIDTH /wFrmCnt * (wFrmCnt - wCurrFrm -1))*m_bMoveSpeed;

		m_shShiftPixelY = 0;
		break;
	case _DIRECTION_LIST_4:
		if ( wCurrFrm < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelX =  (_CELL_WIDTH /wFrmCnt * (wCurrFrm+1))*m_bMoveSpeed;
		else
			m_shShiftPixelX = -(_CELL_WIDTH /wFrmCnt * (wFrmCnt - wCurrFrm -1))*m_bMoveSpeed;

		if ( wCurrFrm < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelY =  (_CELL_HEIGHT/wFrmCnt * (wCurrFrm+1))*m_bMoveSpeed;
		else
			m_shShiftPixelY = -(_CELL_HEIGHT/wFrmCnt * (wFrmCnt - wCurrFrm -1))*m_bMoveSpeed;
		break;
	case _DIRECTION_LIST_5:
		m_shShiftPixelX =  0;

		if ( wCurrFrm < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelY =  (_CELL_HEIGHT/wFrmCnt * (wCurrFrm+1))*m_bMoveSpeed;
		else
			m_shShiftPixelY = -(_CELL_HEIGHT/wFrmCnt * (wFrmCnt - wCurrFrm -1))*m_bMoveSpeed;
		break;
	case _DIRECTION_LIST_6:
		if ( wCurrFrm < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelX = -(_CELL_WIDTH /wFrmCnt * (wCurrFrm+1))*m_bMoveSpeed;
		else
			m_shShiftPixelX =  (_CELL_WIDTH /wFrmCnt * (wFrmCnt - wCurrFrm -1))*m_bMoveSpeed;

		if ( wCurrFrm < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelY =  (_CELL_HEIGHT/wFrmCnt * (wCurrFrm+1))*m_bMoveSpeed;
		else
			m_shShiftPixelY = -(_CELL_HEIGHT/wFrmCnt * (wFrmCnt - wCurrFrm -1))*m_bMoveSpeed;
		break;
	case _DIRECTION_LIST_7:
		if ( wCurrFrm < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelX = -(_CELL_WIDTH /wFrmCnt * (wCurrFrm+1))*m_bMoveSpeed;
		else
			m_shShiftPixelX =  (_CELL_WIDTH /wFrmCnt * (wFrmCnt - wCurrFrm -1))*m_bMoveSpeed;

		m_shShiftPixelY =  0;
		break;
	case _DIRECTION_LIST_8:
		if ( wCurrFrm < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelX = -(_CELL_WIDTH /wFrmCnt * (wCurrFrm+1))*m_bMoveSpeed;
		else
			m_shShiftPixelX =  (_CELL_WIDTH /wFrmCnt * (wFrmCnt - wCurrFrm -1))*m_bMoveSpeed;

		if ( wCurrFrm < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelY = -(_CELL_HEIGHT/wFrmCnt * (wCurrFrm+1))*m_bMoveSpeed;
		else
			m_shShiftPixelY =  (_CELL_HEIGHT/wFrmCnt * (wFrmCnt - wCurrFrm -1))*m_bMoveSpeed;
		break;	
	}
}



VOID CActor::SetBackStepMoving()
{	
	switch ( m_bMoveDir )
	{
	case _DIRECTION_LIST_1:
		m_shShiftPixelX =  0;

		if ( m_bBackStepFrame < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelY = -(_CELL_HEIGHT/m_bBackStepFrameCnt * (m_bBackStepFrame+1))*m_bMoveSpeed;
		else
			m_shShiftPixelY =  (_CELL_HEIGHT/m_bBackStepFrameCnt * (m_bBackStepFrameCnt - m_bBackStepFrame -1))*m_bMoveSpeed;
		break;
	case _DIRECTION_LIST_2:
		if ( m_bBackStepFrame < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelX =  (_CELL_WIDTH /m_bBackStepFrameCnt * (m_bBackStepFrame+1))*m_bMoveSpeed;
		else
			m_shShiftPixelX = -(_CELL_WIDTH /m_bBackStepFrameCnt * (m_bBackStepFrameCnt - m_bBackStepFrame -1))*m_bMoveSpeed;

		if ( m_bBackStepFrame < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelY = -(_CELL_HEIGHT/m_bBackStepFrameCnt * (m_bBackStepFrame+1))*m_bMoveSpeed;
		else
			m_shShiftPixelY =  (_CELL_HEIGHT/m_bBackStepFrameCnt * (m_bBackStepFrameCnt - m_bBackStepFrame -1))*m_bMoveSpeed;
		break;
	case _DIRECTION_LIST_3:
		if ( m_bBackStepFrame < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelX =  (_CELL_WIDTH /m_bBackStepFrameCnt * (m_bBackStepFrame+1))*m_bMoveSpeed;
		else
			m_shShiftPixelX = -(_CELL_WIDTH /m_bBackStepFrameCnt * (m_bBackStepFrameCnt - m_bBackStepFrame -1))*m_bMoveSpeed;

		m_shShiftPixelY = 0;
		break;
	case _DIRECTION_LIST_4:
		if ( m_bBackStepFrame < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelX =  (_CELL_WIDTH /m_bBackStepFrameCnt * (m_bBackStepFrame+1))*m_bMoveSpeed;
		else
			m_shShiftPixelX = -(_CELL_WIDTH /m_bBackStepFrameCnt * (m_bBackStepFrameCnt - m_bBackStepFrame -1))*m_bMoveSpeed;

		if ( m_bBackStepFrame < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelY =  (_CELL_HEIGHT/m_bBackStepFrameCnt * (m_bBackStepFrame+1))*m_bMoveSpeed;
		else
			m_shShiftPixelY = -(_CELL_HEIGHT/m_bBackStepFrameCnt * (m_bBackStepFrameCnt - m_bBackStepFrame -1))*m_bMoveSpeed;
		break;
	case _DIRECTION_LIST_5:
		m_shShiftPixelX =  0;

		if ( m_bBackStepFrame < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelY =  (_CELL_HEIGHT/m_bBackStepFrameCnt * (m_bBackStepFrame+1))*m_bMoveSpeed;
		else
			m_shShiftPixelY = -(_CELL_HEIGHT/m_bBackStepFrameCnt * (m_bBackStepFrameCnt - m_bBackStepFrame -1))*m_bMoveSpeed;
		break;
	case _DIRECTION_LIST_6:
		if ( m_bBackStepFrame < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelX = -(_CELL_WIDTH /m_bBackStepFrameCnt * (m_bBackStepFrame+1))*m_bMoveSpeed;
		else
			m_shShiftPixelX =  (_CELL_WIDTH /m_bBackStepFrameCnt * (m_bBackStepFrameCnt - m_bBackStepFrame -1))*m_bMoveSpeed;

		if ( m_bBackStepFrame < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelY =  (_CELL_HEIGHT/m_bBackStepFrameCnt * (m_bBackStepFrame+1))*m_bMoveSpeed;
		else
			m_shShiftPixelY = -(_CELL_HEIGHT/m_bBackStepFrameCnt * (m_bBackStepFrameCnt - m_bBackStepFrame -1))*m_bMoveSpeed;
		break;
	case _DIRECTION_LIST_7:
		if ( m_bBackStepFrame < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelX = -(_CELL_WIDTH /m_bBackStepFrameCnt * (m_bBackStepFrame+1))*m_bMoveSpeed;
		else
			m_shShiftPixelX =  (_CELL_WIDTH /m_bBackStepFrameCnt * (m_bBackStepFrameCnt - m_bBackStepFrame -1))*m_bMoveSpeed;

		m_shShiftPixelY =  0;
		break;
	case _DIRECTION_LIST_8:
		if ( m_bBackStepFrame < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelX = -(_CELL_WIDTH /m_bBackStepFrameCnt * (m_bBackStepFrame+1))*m_bMoveSpeed;
		else
			m_shShiftPixelX =  (_CELL_WIDTH /m_bBackStepFrameCnt * (m_bBackStepFrameCnt - m_bBackStepFrame -1))*m_bMoveSpeed;

		if ( m_bBackStepFrame < (6-m_bMoveNextFrmCnt) )
			m_shShiftPixelY = -(_CELL_HEIGHT/m_bBackStepFrameCnt * (m_bBackStepFrame+1))*m_bMoveSpeed;
		else
			m_shShiftPixelY =  (_CELL_HEIGHT/m_bBackStepFrameCnt * (m_bBackStepFrameCnt - m_bBackStepFrame -1))*m_bMoveSpeed;
		break;	
	}
}


VOID CActor::SetMoved()
{
	switch ( m_bMoveDir )
	{
	case _DIRECTION_LIST_1:
		m_shShiftTileX =  0;
		m_shShiftTileY = -m_bMoveSpeed;
		break;
	case _DIRECTION_LIST_2:
		m_shShiftTileX =  m_bMoveSpeed;
		m_shShiftTileY = -m_bMoveSpeed;
		break;
	case _DIRECTION_LIST_3:
		m_shShiftTileX =  m_bMoveSpeed;
		m_shShiftTileY =  0;
		break;
	case _DIRECTION_LIST_4:
		m_shShiftTileX =  m_bMoveSpeed;
		m_shShiftTileY =  m_bMoveSpeed;
		break;
	case _DIRECTION_LIST_5:
		m_shShiftTileX =  0;
		m_shShiftTileY =  m_bMoveSpeed;
		break;
	case _DIRECTION_LIST_6:
		m_shShiftTileX = -m_bMoveSpeed;
		m_shShiftTileY =  m_bMoveSpeed;
		break;
	case _DIRECTION_LIST_7:
		m_shShiftTileX = -m_bMoveSpeed;
		m_shShiftTileY =  0;
		break;
	case _DIRECTION_LIST_8:
		m_shShiftTileX = -m_bMoveSpeed;
		m_shShiftTileY = -m_bMoveSpeed;
		break;
	}
	m_wPosX += m_shShiftTileX;
	m_wPosY += m_shShiftTileY;
	m_shShiftPixelX = 0;
	m_shShiftPixelY = 0;
}


/******************************************************************************************************************

	函数名 : CActor::OnCharDescPacket()

	作者 :
	编写日期 :

	目的   : SM_WALK, SM_DEATH, SM_TURN, SM_RUN, SM_DIGUP 消息中使用.
	输入   : LPPACKETMSG lpPacketMsg
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CActor::OnCharDescPacket(LPPACKETMSG lpPacketMsg)
{
	CHARDESC	stCharDesc;
	FEATURE		stFeature;
	fnDecode6BitBuf(lpPacketMsg->szEncodeData, (char*)&stCharDesc, sizeof(CHARDESC));

	memcpy(&stFeature, &stCharDesc.nFeature, sizeof(FEATURE));
	m_nState = stCharDesc.nStatus;

	if( m_nState & 0X2 )	m_bOpenHealth = TRUE;
	else					m_bOpenHealth = FALSE;

	ChangeFeature(stFeature);
}

bool CActor::ReceiveName(const char* encoded,BYTE color)
{
    constexpr size_t MaximumEncoded=1364;
    const size_t length=strlen(encoded);
    if(length>MaximumEncoded) return false;
    for(size_t i=0;i<length;++i) if(BYTE(encoded[i])<0x3c || BYTE(encoded[i])>0x7b) return false;
    char decoded[1024]={};
    decoded[fnDecode6BitBuf(const_cast<char*>(encoded),decoded,sizeof(decoded)-1)]=0;
    std::string remaining=decoded,lines[3];
    for(auto& line:lines) {
        const auto cr=remaining.find('\r');
        line=remaining.substr(0,cr);
        if(cr==std::string::npos) line=line.substr(0,line.find('/'));
        if(line.size()>=sizeof(m_szName)) return false;
        if(cr==std::string::npos) break;
        remaining.erase(0,cr+1);
    }
    strcpy(m_szName,lines[0].c_str());strcpy(m_szGuildName,lines[1].c_str());strcpy(m_szGuildRank,lines[2].c_str());
    m_dwNameClr=GetUserNameColor(color);
    if(this==&g_xGameProc.m_xMyHero) g_xGameProc.m_xInterface.m_xStatusWnd.m_identityTertiary=lines[2];
    return true;
}

VOID CActor::OnUserName(LPPACKETMSG lpPacketMsg)
{
    ReceiveName(lpPacketMsg->szEncodeData,LOBYTE(lpPacketMsg->stDefMsg.wParam));
}

VOID CActor::OnChangeNameClr(LPPACKETMSG lpPacketMsg)
{
	m_dwNameClr = GetUserNameColor(lpPacketMsg->stDefMsg.wParam);
}

VOID CActor::OnChangeLight(LPPACKETMSG lpPacketMsg)
{
	m_bLightSize = lpPacketMsg->stDefMsg.wParam;
}

VOID CActor::OnOpenHealth(LPPACKETMSG lpPacketMsg)
{
	m_bOpenHealth = TRUE;
}

VOID CActor::OnCloseHealth(LPPACKETMSG lpPacketMsg)
{
	m_bOpenHealth = FALSE;
}

VOID CActor::OnFeatureChanged(LPPACKETMSG lpPacketMsg)
{
	FEATURE stFeature;
	LONG nFeature	  = MAKELONG(lpPacketMsg->stDefMsg.wParam, lpPacketMsg->stDefMsg.wTag);

	memcpy(&stFeature, &nFeature, sizeof(LONG));

	ChangeFeature(stFeature);
}

VOID CActor::OnHealthSpellChanged(LPPACKETMSG lpPacketMsg)
{
    HealthDamage(m_wHP,lpPacketMsg->stDefMsg.wParam);
	m_wHP	 = lpPacketMsg->stDefMsg.wParam;
	m_wMP	 = lpPacketMsg->stDefMsg.wTag;
	m_wMAXHP = lpPacketMsg->stDefMsg.wSeries;
	FLOAT wHPRate = (FLOAT)((FLOAT)m_wHP/(FLOAT)m_wMAXHP);
	m_bHPPercent  = wHPRate*100;
}

VOID CActor::OnWalk(LPPACKETMSG lpPacketMsg)
{
	BYTE bDir;
//	m_wPosX = lpPacketMsg->stDefMsg.wParam;
//	m_wPosY = lpPacketMsg->stDefMsg.wTag;
	bDir		 = LOBYTE(lpPacketMsg->stDefMsg.wSeries);
	m_bLightSize = HIBYTE(lpPacketMsg->stDefMsg.wSeries);
	OnCharDescPacket(lpPacketMsg);
	SetMotionFrame(_MT_MON_WALK, bDir);
	m_bMoveSpeed = _SPEED_WALK;
	SetMoving();
}

VOID CActor::OnTurn(LPPACKETMSG lpPacketMsg)
{
	BYTE bDir;
	m_wPosX = lpPacketMsg->stDefMsg.wParam;
	m_wPosY = lpPacketMsg->stDefMsg.wTag;
	bDir		 = LOBYTE(lpPacketMsg->stDefMsg.wSeries);
	m_bLightSize = HIBYTE(lpPacketMsg->stDefMsg.wSeries);

	// 食人草.
	if ( m_stFeature.bDress == 31 )
		bDir = 0;

	OnCharDescPacket(lpPacketMsg);

	SetMotionFrame(_MT_MON_STAND, bDir);

	if( m_nState & 0X1 )		// 石化状态.
	{
		switch ( m_stFeature.bDress )
		{
		case 83: //走马神掌.
		case 84: //走马护法.
		case 85: //主魔王.
			SetMotionFrame(_MT_MON_APPEAR, 0);
			m_dwEndFrame = m_dwFstFrame + 1;
			m_bCurrMtn = _MT_MON_STAND;
		}
	}
}

VOID CActor::OnDigup(LPPACKETMSG lpPacketMsg)
{
	//MESSAGEBODYWL ????????
	BYTE bDir;
	m_wPosX      = lpPacketMsg->stDefMsg.wParam;
	m_wPosY		 = lpPacketMsg->stDefMsg.wTag;
	bDir		 = LOBYTE(lpPacketMsg->stDefMsg.wSeries);
	m_bLightSize = HIBYTE(lpPacketMsg->stDefMsg.wSeries);
	OnCharDescPacket(lpPacketMsg);
	m_bReverse = FALSE;

	switch ( m_stFeature.bDress )
	{
	case 3: 
		m_bReverse = TRUE;
		break;
	case 85: //主魔王.
		bDir = 0;
		break;
	case 31: //食人草.		
		bDir = 0;
		m_bReverse = TRUE;
		break;
	case 56:
		{
			CMagic* pxMagic;
			pxMagic = new CMagic;
			pxMagic->CreateMagic(_SKILL_SKELLETON, m_wPosX, m_wPosY, m_wPosX, m_wPosY, NULL, 0);
			g_xGameProc.m_xMagicList.AddNode(pxMagic);	
			break;
		}
/*	case 89:
		{
			CMagic* pxMagic;
			pxMagic = new CMagic;
			pxMagic->CreateMagic(_SKILL_SINSU, m_wPosX, m_wPosY, m_wPosX, m_wPosY, NULL, 0);
			g_xGameProc.m_xMagicList.AddNode(pxMagic);	
			break;
		}
*/	}

	m_bAppearState = _DIG_UP;
	SetMotionFrame(_MT_MON_APPEAR, bDir);
}

VOID CActor::OnDigDown(LPPACKETMSG lpPacketMsg)
{
	BYTE bDir;
	m_wPosX		 = lpPacketMsg->stDefMsg.wParam;
	m_wPosY		 = lpPacketMsg->stDefMsg.wTag;
	bDir		 = m_bCurrDir;
	m_bReverse	 = TRUE;

	switch ( m_stFeature.bDress )
	{
	case 3: 
		m_bReverse = FALSE;
		break;
	case 31: //食人草.		
		bDir = 0;
		m_bReverse = FALSE;
		break;
	}

	m_bAppearState = _DIG_DOWN;

	SetMotionFrame(_MT_MON_APPEAR, bDir);
}

VOID CActor::OnDeath(LPPACKETMSG lpPacketMsg)
{
    m_wHP=0;
    m_bHPPercent=0;
	BYTE bDir;
	m_wPosX = lpPacketMsg->stDefMsg.wParam;
	m_wPosY = lpPacketMsg->stDefMsg.wTag;
	bDir	= lpPacketMsg->stDefMsg.wSeries;
	OnCharDescPacket(lpPacketMsg);
	SetMotionFrame(_MT_MON_DIE, bDir);
	if ( lpPacketMsg->stDefMsg.wIdent == SM_DEATH )			
	{
		m_dwCurrFrame = m_dwEndFrame - 1;
		m_bIsDead = TRUE;
	}
	else 
	{
		// 为怪物时.
		if ( m_stFeature.bGender == _GENDER_MON )
		{
			CMagic* pxMagic;
			switch ( m_stFeature.bDress )
			{
			case 4:		// 石匠.
				pxMagic = new CMagic;
				pxMagic->CreateMagic(_MONMAGIC_EXPLODE1, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
				g_xGameProc.m_xMagicList.AddNode(pxMagic);
				break;
			case  5:		// 巴库加尔纳.
			case  6:		// 巴扎乌尔.
			case 11:		// 莫迪森.
			case 19:		// 雷迪加尔纳.
				pxMagic = new CMagic;
				pxMagic->CreateMagic(_MONMAGIC_SINGI_DIE, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
				g_xGameProc.m_xMagicList.AddNode(pxMagic);
/*				LoadEffect(&g_xGameProc.m_xImage, _MONMAGIC_DIE);
				m_bUseSwordEffect = TRUE;
*/				break;
			case 40:		// 稻草人.
				pxMagic = new CMagic;
				pxMagic->CreateMagic(_MONMAGIC_HUSU_DIE, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
				g_xGameProc.m_xMagicList.AddNode(pxMagic);
/*				LoadEffect(&g_xGameProc.m_xImage, _MONMAGIC_HUSU_DIE);
				m_bUseSwordEffect = TRUE;
*/				break;
			case 48:		// 僵尸.
				pxMagic = new CMagic;
				pxMagic->CreateMagic(_MONMAGIC_ZOMBIE_DIE, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
				g_xGameProc.m_xMagicList.AddNode(pxMagic);
/*				LoadEffect(&g_xGameProc.m_xImage, _MONMAGIC_ZOMBIE_DIE);
				m_bUseSwordEffect = TRUE;
*/				break;
			case 99:		// 巴奥达尔德.
				pxMagic = new CMagic;
				pxMagic->CreateMagic(_MONMAGIC_BAODIE, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
				g_xGameProc.m_xMagicList.AddNode(pxMagic);
				break;
			default :
				break;
			}
		}
	}	
}

VOID CActor::OnBackStep(LPPACKETMSG lpPacketMsg)
{
	BYTE bDir;
	bDir	= lpPacketMsg->stDefMsg.wSeries;
	m_wPosX = lpPacketMsg->stDefMsg.wParam;
	m_wPosY = lpPacketMsg->stDefMsg.wTag;
	SetMotionFrame(_MT_MON_WALK, bDir);
	if ( m_bCurrDir < 4 )			m_bMoveDir = m_bCurrDir + 4;
	else							m_bMoveDir = m_bCurrDir - 4;
	m_bMoveSpeed = _SPEED_WALK;
	m_bBackStepFrame	= 0;
	m_bBackStepFrameCnt = 6;
	SetBackStepMoving();
}

VOID CActor::OnStruck(LPPACKETMSG lpPacketMsg)
{
	MESSAGEBODYWL stMsgBodyWl;
	FEATURE		  stFeature;

	FLOAT wHPRate = (FLOAT)((FLOAT)lpPacketMsg->stDefMsg.wParam/(FLOAT)lpPacketMsg->stDefMsg.wTag);
	WORD  wDamage = lpPacketMsg->stDefMsg.wSeries;
    if(!m_bIsDead) AddDamage(-int(wDamage));
	m_wHP		  = lpPacketMsg->stDefMsg.wParam;
	m_wMAXHP	  = lpPacketMsg->stDefMsg.wTag;
	m_bHPPercent  = wHPRate*100;

	fnDecode6BitBuf(lpPacketMsg->szEncodeData, (char*)&stMsgBodyWl, sizeof(MESSAGEBODYWL));

	memcpy(&stFeature, &stMsgBodyWl.lParam1, sizeof(LONG));

	ChangeFeature(stFeature);
	SetMotionFrame(_MT_MON_HITTED, m_bCurrDir);

	// 为怪物时.
	if ( m_stHitter.bGender == _GENDER_MON )
	{
		CMagic* pxMagic;
		switch ( m_stHitter.bDress )
		{
		case 2:		// 凯法洛普.
			pxMagic = new CMagic;
			pxMagic->CreateMagic(_MONMAGIC_KEPAL, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
			g_xGameProc.m_xMagicList.AddNode(pxMagic);
			break;
		case 8:		// 甲胄蚂蚁.
		case 14:	// 兵蚁.
		case 16:	// 游荡蚁.
			pxMagic = new CMagic;
			pxMagic->CreateMagic(_MONMAGIC_GREATANT, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
			g_xGameProc.m_xMagicList.AddNode(pxMagic);
			break;
		case 67:	// 烛龙神.
			pxMagic = new CMagic;
			pxMagic->CreateMagic(_MONMAGIC_BIGGINE_CHAR, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
			g_xGameProc.m_xMagicList.AddNode(pxMagic);
			break;
		default :
			break;
		}
	}
	ZeroMemory(&m_stHitter, sizeof(FEATURE));
}


VOID CActor::OnLighting(LPPACKETMSG lpPacketMsg)
{
	WORD	wTargetX, wTargetY;
	INT		nTargetID;
	CMagic* pxMagic;

	m_wPosX      = lpPacketMsg->stDefMsg.wParam;
	m_wPosY		 = lpPacketMsg->stDefMsg.wTag;
	m_bCurrDir	 = lpPacketMsg->stDefMsg.wSeries;

	MESSAGEBODYWL stMsgBodyWl;
	fnDecode6BitBuf(lpPacketMsg->szEncodeData, (char*)&stMsgBodyWl, sizeof(MESSAGEBODYWL));
	wTargetX = stMsgBodyWl.lParam1;
	wTargetY = stMsgBodyWl.lParam2;
	nTargetID = MAKELONG(stMsgBodyWl.nTag1, stMsgBodyWl.nTag2);

	switch ( m_stFeature.bDress )
	{
	case 48:		// 僵尸. 8方向.
		pxMagic = new CMagic;
		pxMagic->CreateMagic(_MONMAGIC_ZOMBIE, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this, nTargetID);
		g_xGameProc.m_xMagicList.AddNode(pxMagic);
		SetMotionFrame(_MT_MON_ATTACK_A, m_bCurrDir);
		break;
	}
}


VOID CActor::OnFlyAxe(LPPACKETMSG lpPacketMsg)
{
	WORD	wTargetX, wTargetY;
	INT		nTargetID;
	CMagic* pxMagic;

	m_wPosX      = lpPacketMsg->stDefMsg.wParam;
	m_wPosY		 = lpPacketMsg->stDefMsg.wTag;
	m_bCurrDir	 = lpPacketMsg->stDefMsg.wSeries;

	MESSAGEBODYW stMsgBodyW;
	fnDecode6BitBuf(lpPacketMsg->szEncodeData, (char*)&stMsgBodyW, sizeof(MESSAGEBODYW));
	wTargetX = stMsgBodyW.wParam1;
	wTargetY = stMsgBodyW.wParam2;
	nTargetID = MAKELONG(stMsgBodyW.wTag1, stMsgBodyW.wTag2);

	switch ( m_stFeature.bDress )
	{
	case 33:				// 双斧骷髅.
		pxMagic = new CMagic;
		pxMagic->CreateMagic(_MONMAGIC_DUALAXE, m_wPosX, m_wPosY, wTargetX, wTargetY, this, nTargetID);
		g_xGameProc.m_xMagicList.AddNode(pxMagic);
		SetMotionFrame(_MT_MON_ATTACK_A, m_bCurrDir);
		break;
	case 46:				// 暗黑。
		pxMagic = new CMagic;
		pxMagic->CreateMagic(_MONMAGIC_CHIM, m_wPosX, m_wPosY, wTargetX, wTargetY, this, nTargetID);
		g_xGameProc.m_xMagicList.AddNode(pxMagic);
		SetMotionFrame(_MT_MON_ATTACK_A, m_bCurrDir);
		break;
	case 82:				// 魔弓士.
		pxMagic = new CMagic;
		pxMagic->CreateMagic(_MONMAGIC_MAARROW, m_wPosX, m_wPosY, wTargetX, wTargetY, this, nTargetID);
		g_xGameProc.m_xMagicList.AddNode(pxMagic);
		SetMotionFrame(_MT_MON_ATTACK_A, m_bCurrDir);
		break;
	}
}


VOID CActor::OnHit(LPPACKETMSG lpPacketMsg)
{
	BYTE bDir;
	bDir	= lpPacketMsg->stDefMsg.wSeries;
	m_wPosX = lpPacketMsg->stDefMsg.wParam;
	m_wPosY = lpPacketMsg->stDefMsg.wTag;
	SetMotionFrame(_MT_MON_ATTACK_A, bDir);

	// 为怪物时.
	if ( m_stFeature.bGender == _GENDER_MON )
	{
		CMagic* pxMagic;
		switch ( m_stFeature.bDress )
		{
		case 9:		// 罗恩.
			pxMagic = new CMagic;
			pxMagic->CreateMagic(_MONMAGIC_EXPLODE, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
			g_xGameProc.m_xMagicList.AddNode(pxMagic);
			break;
		case 5:		// 巴库加尔纳.
			pxMagic = new CMagic;
			pxMagic->CreateMagic(_MONMAGIC_BLACK1, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
			g_xGameProc.m_xMagicList.AddNode(pxMagic);
			break;
		case 19:		// 雷迪加尔纳.
			pxMagic = new CMagic;
			pxMagic->CreateMagic(_MONMAGIC_RED1, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
			g_xGameProc.m_xMagicList.AddNode(pxMagic);
			break;
		case 45:		// 牛面鬼王. 
			pxMagic = new CMagic;
			pxMagic->CreateMagic(_MONMAGIC_COWGHOST, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
			g_xGameProc.m_xMagicList.AddNode(pxMagic);
			break;
		case 90:		// 神兽.
			pxMagic = new CMagic;
			pxMagic->CreateMagic(_MONMAGIC_SINSU, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
			g_xGameProc.m_xMagicList.AddNode(pxMagic);
			break;
		case 43:		// 火焰牛面鬼.
			pxMagic = new CMagic;
			pxMagic->CreateMagic(_MONMAGIC_COWFLAME, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
			g_xGameProc.m_xMagicList.AddNode(pxMagic);
			break;
		case 85:		// 走马王。
			pxMagic = new CMagic;
			pxMagic->CreateMagic(_MONMAGIC_JUMAWANG, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
			g_xGameProc.m_xMagicList.AddNode(pxMagic);
			break;
		case 67:		// 烛龙神.
			pxMagic = new CMagic;
			pxMagic->CreateMagic(_MONMAGIC_BIGGINE_ATT, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
			g_xGameProc.m_xMagicList.AddNode(pxMagic);
			break;	
		case 75:		// 刺蛾.
			pxMagic = new CMagic;
			pxMagic->CreateMagic(_MONMAGIC_SSEGI, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
			g_xGameProc.m_xMagicList.AddNode(pxMagic);
			break;	
		case 94:		// 沙鱼.
			pxMagic = new CMagic;
			pxMagic->CreateMagic(_MONMAGIC_SANDFISH, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
			g_xGameProc.m_xMagicList.AddNode(pxMagic);
			break;	
		case 98:		// 上级怪物.
			m_dwCurrEffectFrame = 50;
			m_dwFstEffectFrame  = 50;
			m_dwEndEffectFrame  = 60;
			m_bEffectFrame		= 0;
			m_bEffectFrameCnt	= _DEFAULT_SPELLFRAME;
			LoadEffect(&g_xGameProc.m_xImage, _MONMAGIC_NUMAGUMGI, bDir);
			m_bUseSwordEffect = TRUE;
			break;	
		default :
			break;
		}
	}
}

VOID CActor::StruckMsgReassign()
{
	INT			nCnt;
	SHORT		shLeftMsgCnt;
	LPPACKETMSG	lpPacketMsg;

	lpPacketMsg  = NULL;
	shLeftMsgCnt = m_xPacketQueue.GetCount();

	if ( shLeftMsgCnt > 1 )
	{
		for ( nCnt = 0; nCnt < shLeftMsgCnt; nCnt++ )
		{
			lpPacketMsg = (LPPACKETMSG)m_xPacketQueue.PopQ();
			if ( lpPacketMsg )
			{
				if ( lpPacketMsg->stDefMsg.wIdent == SM_STRUCK )
				{
					SAFE_DELETE(lpPacketMsg);
				}
				else
				{
					m_xPacketQueue.PushQ((BYTE*)lpPacketMsg);
				}
			}
		}
	}
}


BOOL CActor::UpdatePacketState()
{
	LPPACKETMSG	lpPacketMsg = NULL;

	if ( m_bCurrMtn == _MT_MON_STAND )
	{
		SHORT shLeftMsgCnt = m_xPacketQueue.GetCount();

		if ( shLeftMsgCnt > 0 )
		{
			lpPacketMsg = (LPPACKETMSG)m_xPacketQueue.PopQ();

			if ( shLeftMsgCnt >= 3 )		m_bMsgHurryCheck = TRUE;
			else							m_bMsgHurryCheck = FALSE;

			if ( lpPacketMsg )
			{
				switch ( lpPacketMsg->stDefMsg.wIdent )
				{
					case SM_NOWDEATH:
					case SM_DEATH:				{	OnDeath(lpPacketMsg);				break;	}
					case SM_WALK:				{	OnWalk(lpPacketMsg);				break;	}
					case SM_TURN:				{	OnTurn(lpPacketMsg);				break;	}
					case SM_DIGUP:				{	OnDigup(lpPacketMsg);				break;	}
					case SM_DIGDOWN:			{	OnDigDown(lpPacketMsg);				break;	}
					case SM_FEATURECHANGED:		{	OnFeatureChanged(lpPacketMsg);		break;	}
					case SM_OPENHEALTH:			{	OnOpenHealth(lpPacketMsg);			break;	}
					case SM_CLOSEHEALTH:		{	OnCloseHealth(lpPacketMsg);			break;	}
					case SM_CHANGELIGHT:		{	OnChangeLight(lpPacketMsg);			break;	}
					case SM_CHANGENAMECOLOR:	{	OnChangeNameClr(lpPacketMsg);		break;	}
					case SM_USERNAME:			{	OnUserName(lpPacketMsg);			break;	}
					case SM_HEALTHSPELLCHANGED:	{	OnHealthSpellChanged(lpPacketMsg);	break;	}
					case SM_BACKSTEP:			{	OnBackStep(lpPacketMsg);			break;	}
					case SM_STRUCK:				{	OnStruck(lpPacketMsg);				break;	}
					case SM_HIT:				{	OnHit(lpPacketMsg);					break;	}
					case SM_FLYAXE:				{	OnFlyAxe(lpPacketMsg);				break;	}
					case SM_LIGHTING:			{	OnLighting(lpPacketMsg);			break;	}
					case SM_SKELETON:
						OnWalk(lpPacketMsg);
						break;
					default:
					{										
						break;	
					}
				}
			}
			SAFE_DELETE(lpPacketMsg);
			return TRUE;
		}
	}
	return FALSE;
}



BOOL CActor::LoadEffect(CImageHandler* pxImgHandler, WORD wEffectNum, BYTE bDir)
{
	WORD wFileType;
	LPEFFECTSPRINFO pstEffect = g_xSpriteInfo.GetEffectInfo(wEffectNum);

	if ( pstEffect )
	{
		m_bEffectImgIdx = pstEffect->wImgIdx;		

		m_dwFstEffectFrame = pstEffect->dwFstFrm;
		m_dwEndEffectFrame = pstEffect->dwEndFrm;

		if ( bDir )
		{
			m_dwFstEffectFrame = m_dwFstEffectFrame + bDir*10;
			m_dwEndEffectFrame = m_dwEndEffectFrame + bDir*10;
		}
		m_dwCurrEffectFrame = m_dwFstEffectFrame;
		m_bEffectFrame = 0;

		m_bLightRadius[0] = pstEffect->bLightRadius[0];
		m_bLightRadius[1] = pstEffect->bLightRadius[1];

		m_bLightColor[0][0]	= pstEffect->bLightColor[0][0];
		m_bLightColor[0][1]	= pstEffect->bLightColor[0][1];
		m_bLightColor[0][2]	= pstEffect->bLightColor[0][2];
		
		m_bLightColor[1][0]	= pstEffect->bLightColor[1][0];
		m_bLightColor[1][1]	= pstEffect->bLightColor[1][1];
		m_bLightColor[1][2]	= pstEffect->bLightColor[1][2];

		for ( INT nCnt = m_dwFstEffectFrame; nCnt < m_dwEndEffectFrame; nCnt++ )
		{
			if ( m_bEffectImgIdx == _IMAGE_MAGIC )
			{
				wFileType = _TEXTR_FILE_MAGIC;
			}
			else if ( m_bEffectImgIdx == _IMAGE_MONMAGIC )
			{
				wFileType = _TEXTR_FILE_MONMAGIC;
			}

			pxImgHandler->AddTextr(wFileType, m_bEffectImgIdx, nCnt);
		}
		return TRUE;

	}
	return FALSE;
}



BOOL CActor::DrawEffect()
{
	if ( g_xMainWnd.Get3DDevice() )
	{
		if( SUCCEEDED(g_xMainWnd.Get3DDevice()->BeginScene()) )
		{
			WORD	  wFileType;
			D3DVECTOR vTrans;
			D3DMATRIX matTrans;
			D3DMATRIX matScale;
			D3DMATRIX matRot;
			D3DMATRIX matWorld;
			D3DMATRIX matWorldOriginal;

			g_xMainWnd.Get3DDevice()->GetTransform(D3DTRANSFORMSTATE_WORLD, &matWorldOriginal);

			D3DMATERIAL7 mtrl;
			CWHWilImageData* pxWilImg;

			if ( m_bUseEffect || m_bUseSwordEffect )
			{
				BOOL bIndexSetted = FALSE;
				if ( m_bUseEffect )
				{
					pxWilImg = &g_xGameProc.m_xImage.m_xImageList[m_bEffectImgIdx];
					bIndexSetted = pxWilImg->NewSetIndex(m_dwCurrEffectFrame);
				}
				else
				{
					pxWilImg = &g_xGameProc.m_xImage.m_xImageList[m_bEffectImgIdx];
					m_dwCurrEffectFrame = m_dwFstEffectFrame + m_dwCurrFrame - m_dwFstFrame;
					bIndexSetted = pxWilImg->NewSetIndex(m_dwCurrEffectFrame);
				}

				if ( bIndexSetted )
				{
 					vTrans.x = (FLOAT)m_shScrnPosX+(FLOAT)pxWilImg->m_lpstNewCurrWilImageInfo->shWidth/2+pxWilImg->m_lpstNewCurrWilImageInfo->shPX-400; 
					vTrans.y = (FLOAT)-m_shScrnPosY-(FLOAT)pxWilImg->m_lpstNewCurrWilImageInfo->shHeight/2-pxWilImg->m_lpstNewCurrWilImageInfo->shPY+300; 
					vTrans.z = 0;
					D3DUtil_SetTranslateMatrix(matTrans, vTrans);
					D3DUtil_SetScaleMatrix(matScale, (FLOAT)pxWilImg->m_lpstNewCurrWilImageInfo->shWidth, (FLOAT)pxWilImg->m_lpstNewCurrWilImageInfo->shHeight, 0.0f);
					D3DMath_MatrixMultiply(matWorld, matScale, matTrans);
					g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld);

					if ( m_bEffectImgIdx == _IMAGE_MAGIC )
					{
						wFileType = _TEXTR_FILE_MAGIC;
					}
					else if ( m_bEffectImgIdx == _IMAGE_MONMAGIC )
					{
						wFileType = _TEXTR_FILE_MONMAGIC;
					}
					LPDIRECTDRAWSURFACE7 lpddsTextr = g_xGameProc.m_xImage.GetTextrImg(wFileType, m_bEffectImgIdx, m_dwCurrEffectFrame);
					g_xMainWnd.Get3DDevice()->SetTexture(0, lpddsTextr);
//					g_xMainWnd.Get3DDevice()->SetTexture(0, D3DWILTextr_GetSurface(pxWilImg->m_szWilFileName, m_dwCurrEffectFrame));

					D3DUtil_InitMaterial(mtrl, 
										 (FLOAT)255/255.0f,
										 (FLOAT)255/255.0f, 
										 (FLOAT)255/255.0f);
					mtrl.diffuse.a = 0/255.0f;
					g_xMainWnd.Get3DDevice()->SetMaterial(&mtrl);

					SetBlendRenderState(g_xMainWnd.Get3DDevice(), _BLEND_LIGHTINV, mtrl);
					g_xMainWnd.Get3DDevice()->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX, m_avBoard, 4, NULL);

					// 恢复原状.
					ZeroMemory(&mtrl, sizeof(mtrl));
					mtrl.diffuse.r = mtrl.diffuse.g = mtrl.diffuse.b = 0.1f;
					mtrl.ambient.r = mtrl.ambient.g = mtrl.ambient.b = 1.0f;
					g_xMainWnd.Get3DDevice()->SetMaterial(&mtrl);

					ResetBlendenderState(g_xMainWnd.Get3DDevice());
					g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorldOriginal);
				}


				if ( m_bUseEffect )
				{
					if ( m_bEffectFrame < m_bEffectFrameCnt / 2 )
					{
						m_bLightRadius[0] = m_bEffectFrame;
						m_bLightRadius[1] = m_bEffectFrame+1;
					}
					else
					{
						m_bLightRadius[0] = m_bEffectFrameCnt - m_bEffectFrame -1;
						m_bLightRadius[1] = m_bEffectFrameCnt - m_bEffectFrame;
					}
					
					if (g_xGameProc.m_xInterface.m_xOptionWnd.m_values[9]) g_xGameProc.m_xLightFog.SetLightRadiusWithRing(
													  m_shScrnPosX + _CELL_WIDTH/2,
							  						  m_shScrnPosY + _CELL_HEIGHT/2, 
													  m_bLightRadius[0], m_bLightColor[0][0], m_bLightColor[0][1], m_bLightColor[0][2], 
													  m_bLightRadius[1], m_bLightColor[1][0], m_bLightColor[1][1], m_bLightColor[1][2]);
				}
				else if ( m_bUseSwordEffect )
				{
					if ( m_dwEndEffectFrame - m_dwFstEffectFrame > 8 )
					{
						m_bLightRadius[0] = (m_dwCurrEffectFrame - m_dwFstEffectFrame)/2 + 2;
						m_bLightRadius[1] = (m_dwCurrEffectFrame - m_dwFstEffectFrame)/2 + 3;
					}
					else
					{
						m_bLightRadius[0] = m_dwCurrEffectFrame - m_dwFstEffectFrame + 2;
						m_bLightRadius[1] = m_dwCurrEffectFrame - m_dwFstEffectFrame + 3;
					}
					if (g_xGameProc.m_xInterface.m_xOptionWnd.m_values[9]) g_xGameProc.m_xLightFog.SetLightRadiusWithRing(
													  m_shScrnPosX + _CELL_WIDTH/2,
							  						  m_shScrnPosY + _CELL_HEIGHT/2, 
													  m_bLightRadius[0], m_bLightColor[0][0], m_bLightColor[0][1], m_bLightColor[0][2], 
													  m_bLightRadius[1], m_bLightColor[1][0], m_bLightColor[1][1], m_bLightColor[1][2]);
				}

			}

			g_xMainWnd.Get3DDevice()->EndScene();

			return S_OK;
		}
	}
	return E_FAIL;
}



VOID CActor::UpdateMotionState(INT nLoopTime, BOOL bIsMoveTime)
{
	m_wABlendCurrDelay += nLoopTime;

	if ( m_wABlendCurrDelay >= m_wABlendDelay )
	{
		m_wABlendCurrDelay = 0;
		m_wABlendDelay	   = 0;
		m_bABlendRev	   = FALSE;
	}

	if ( m_bCurrMtn == _MT_MON_DIE && m_dwCurrFrame >= m_dwEndFrame-1 )
	{
		m_bIsDead = TRUE;
	}

	if ( m_bIsDead )
	{
		SetMotionFrame(_MT_MON_DIE, m_bCurrDir);
		m_dwCurrFrame = m_dwEndFrame - 1;
		return;
	}

	if ( UpdateMove(bIsMoveTime) )
	{
		UpdatePacketState();
		return;
	}
	else
	{
		m_wCurrDelay	   += nLoopTime;

		// 慢速状态让怪物动作帧间隔翻倍，对照 Godot 分支 SceneChicken::slowed。
		const BOOL bSlowMonster = ( m_stFeature.bGender == _GENDER_MON ) && ( m_nState & _STATE_SLOWACTION );
		const WORD wMotionDelay = bSlowMonster ? WORD(m_wDelay * 2) : m_wDelay;

		if ( m_wCurrDelay > wMotionDelay )
		{
			m_wCurrDelay = 0;

			if ( m_dwCurrFrame < m_dwEndFrame )
			{
				m_dwCurrFrame++;

				PlayActSound();

				if ( m_bMsgHurryCheck )			
				{
					m_wDelay = m_wDelay/2;
					m_bMsgHurryCheck = FALSE;
				}
			}
		}

		UpdatePacketState();

		if ( m_dwCurrFrame >= m_dwEndFrame )
		{
			m_shShiftTileX = 0;
			m_shShiftTileY = 0;
			m_shShiftPixelX = 0;
			m_shShiftPixelY = 0;

			m_bReverse		= FALSE;
			m_dwCurrEffectFrame = 0;
			m_dwFstEffectFrame = 0;
			m_dwEndEffectFrame = 0;
			m_bEffectFrame = 0;
			m_bEffectFrameCnt = _DEFAULT_SPELLFRAME;
			m_bUseEffect = FALSE;
			m_bUseSwordEffect = FALSE;
			m_dwCurrFrame = m_dwFstFrame;

			SetMotionFrame(_MT_MON_STAND, m_bCurrDir);

			if( m_nState & 0X1 )		// 石化状态.
			{
				switch ( m_stFeature.bDress )
				{
				case 83: //走马神掌.
				case 84: //走马护法.
				case 85: //主魔王.
					SetMotionFrame(_MT_MON_APPEAR, 0);
					m_dwEndFrame = m_dwFstFrame + 1;
					m_bCurrMtn = _MT_MON_STAND;
				}
			}

			if ( m_bAppearState == _DIG_DOWN )
				m_bAppearState = _DIG_DOWNDEL;
		}
	}
}



BOOL CActor::UpdateMove(BOOL bIsMoveTime)
{
	if ( m_bCurrMtn == _MT_MON_WALK || m_bCurrMtn == _MT_PUSHBACK )
	{
		m_wCurrDelay = 0;

		if ( bIsMoveTime )
		{
			if ( m_bBackStepFrameCnt )
			{
				m_dwCurrFrame += 2;
			}
			else
			{
				m_dwCurrFrame++;

				if ( m_bMsgHurryCheck )			
				{
					m_dwCurrFrame++;
				}
			}

			if ( m_dwCurrFrame >= m_dwEndFrame-m_bMoveNextFrmCnt && !m_bIsMoved )
			{
				SetMoved();
				m_bIsMoved = TRUE;
			}
		}

		// 连续帧中要做的事。
		if ( m_dwCurrFrame >= m_dwEndFrame )
		{
			m_dwCurrFrame = m_dwEndFrame - 1;

			m_shShiftTileX = 0;
			m_shShiftTileY = 0;
			m_shShiftPixelX = 0;
			m_shShiftPixelY = 0;

			m_dwCurrEffectFrame = 0;
			m_dwFstEffectFrame = 0;
			m_dwEndEffectFrame = 0;
			m_bEffectFrame = 0;
			m_bUseEffect = FALSE;
			m_bUseSwordEffect = FALSE;
			m_dwCurrFrame = m_dwFstFrame;

			m_bBackStepFrame = 0;
			m_bBackStepFrameCnt = 0;

			m_bIsMoved = FALSE;

			SetMotionFrame(_MT_MON_STAND, m_bCurrDir);
		}
		else if ( m_dwCurrFrame < m_dwEndFrame )
		{
			if ( m_bCurrMtn != _MT_PUSHBACK )
				SetMoving();
			else
				SetBackStepMoving();
		}
		return TRUE;
	}
	return FALSE;
}



BOOL CActor::DrawActor(CMapHandler* pxMap, BOOL bFocused, BOOL bShadowAblended, BOOL bUseScrnPos, BOOL bDrawShadow)
{
	// 坐标处理。
	if ( bUseScrnPos )
	{
		m_shScrnPosX = (m_wPosX - pxMap->m_shStartViewTileX) * _CELL_WIDTH + _VIEW_CELL_X_START - pxMap->m_shViewOffsetX + m_shShiftPixelX;
		m_shScrnPosY = (m_wPosY - pxMap->m_shStartViewTileY) * _CELL_HEIGHT+ _VIEW_CELL_Y_START - pxMap->m_shViewOffsetY + m_shShiftPixelY;
	}

	BYTE	bShadowType;
	SHORT	shShadowPX;
	SHORT	shShadowPY;

	if ( m_pxActorImage )
	{
		SHORT shPX, shPY;
		DWORD dwCurrFrm;

		if ( !m_bReverse )
		{
			if ( m_bBackStepFrameCnt )		dwCurrFrm = m_dwEndFrame - (m_dwCurrFrame - m_dwFstFrame) -1;
			else							dwCurrFrm = m_dwCurrFrame;
		}
		else
		{
			if ( m_bBackStepFrameCnt )		dwCurrFrm = m_dwCurrFrame;
			else							dwCurrFrm = m_dwEndFrame - (m_dwCurrFrame - m_dwFstFrame) -1;
		}

		if ( m_pxActorImage->NewSetIndex(dwCurrFrm) )
		{
			shPX = m_pxActorImage->m_lpstNewCurrWilImageInfo->shPX;
			shPY = m_pxActorImage->m_lpstNewCurrWilImageInfo->shPY;

			bShadowType = m_pxActorImage->m_lpstNewCurrWilImageInfo->bShadow;
			shShadowPX  = m_pxActorImage->m_lpstNewCurrWilImageInfo->shShadowPX;
			shShadowPY  = m_pxActorImage->m_lpstNewCurrWilImageInfo->shShadowPY;

			SetRect(&m_rcActor, m_shScrnPosX + shPX, m_shScrnPosY + shPY, 
					m_shScrnPosX + shPX + m_pxActorImage->m_lpstNewCurrWilImageInfo->shWidth, 
					m_shScrnPosY + shPY + m_pxActorImage->m_lpstNewCurrWilImageInfo->shHeight);

			if ( ( m_rcActor.right - m_rcActor.left ) > _CELL_WIDTH + _TARGETRGN_GAPX )
			{
				m_rcTargetRgn.left  = m_rcActor.left + ( (m_rcActor.right - m_rcActor.left) - (_CELL_WIDTH + _TARGETRGN_GAPX) )/2;
				m_rcTargetRgn.right = m_rcActor.left + ( (m_rcActor.right - m_rcActor.left) + (_CELL_WIDTH + _TARGETRGN_GAPX) )/2;
			}
			else
			{
				m_rcTargetRgn.left  = m_rcActor.left;
				m_rcTargetRgn.right = m_rcActor.right;
			}

			if ( ( m_rcActor.bottom - m_rcActor.top ) > _CELL_HEIGHT + _TARGETRGN_GAPY )
			{
				m_rcTargetRgn.top    = m_rcActor.top + ( (m_rcActor.bottom - m_rcActor.top) - (_CELL_HEIGHT + _TARGETRGN_GAPY) )/2;
				m_rcTargetRgn.bottom = m_rcActor.top + ( (m_rcActor.bottom - m_rcActor.top) + (_CELL_HEIGHT + _TARGETRGN_GAPY) )/2;
			}
			else
			{
				m_rcTargetRgn.top    = m_rcActor.top;
				m_rcTargetRgn.bottom = m_rcActor.bottom;
			}


			INT nStartX1 = m_shScrnPosX + shShadowPX;
			INT nStartY1 = m_shScrnPosY + shShadowPY;

			BYTE bOpaRate = 70;
			WORD wState   = GetCharState();

			// 应用怪物影子文件。
			if ( bDrawShadow )
			{
				CWHWilImageData* pxSahdowImage;
				pxSahdowImage = &g_xGameProc.m_xImage.m_xImageList[m_bActorImgIdx+_MAX_MONSTER_IMAGE];
				if ( pxSahdowImage->NewSetIndex(dwCurrFrm) )
				{
					SHORT shShadowPX, shShadowPY;
					if ( m_wABlendDelay || wState==_STATE_ABLEND || bShadowAblended )
					{
						shShadowPX = pxSahdowImage->m_lpstNewCurrWilImageInfo->shPX;
						shShadowPY = pxSahdowImage->m_lpstNewCurrWilImageInfo->shPY;

						g_xMainWnd.DrawWithABlendCompDataWithBackBuffer(m_shScrnPosX+shShadowPX, m_shScrnPosY+shShadowPY, pxSahdowImage->m_lpstNewCurrWilImageInfo->shWidth,	pxSahdowImage->m_lpstNewCurrWilImageInfo->shHeight, (WORD*)pxSahdowImage->m_pbCurrImage,
																		_CLIP_WIDTH, _CLIP_HEIGHT, 0XFFFF, 0XFFFF, bOpaRate);
					}
					else
					{
						shShadowPX = pxSahdowImage->m_lpstNewCurrWilImageInfo->shPX;
						shShadowPY = pxSahdowImage->m_lpstNewCurrWilImageInfo->shPY;
						
						if ( !m_bIsDead )
							g_xMainWnd.DrawWithImageForCompClipRgn(m_shScrnPosX+shShadowPX, m_shScrnPosY+shShadowPY, pxSahdowImage->m_lpstNewCurrWilImageInfo->shWidth,	pxSahdowImage->m_lpstNewCurrWilImageInfo->shHeight, (WORD*)pxSahdowImage->m_pbCurrImage,
																   _CLIP_WIDTH, _CLIP_HEIGHT, 0XFFFF, 0XFFFF, bFocused);
					}
				}
				// 计算后绘制.
				else
				{
					// 绘制影子。
					if ( m_wABlendDelay || wState==_STATE_ABLEND || bShadowAblended )
					{
						if ( bOpaRate < 0 && bOpaRate > 100 )		bOpaRate  = 0;
						bShadowAblended = TRUE;
					}
					if ( !m_bIsDead )
					{
						g_xMainWnd.DrawWithShadowABlend(
												nStartX1, nStartY1, 
												m_pxActorImage->m_lpstNewCurrWilImageInfo->shWidth,							
												m_pxActorImage->m_lpstNewCurrWilImageInfo->shHeight,
												(WORD*)m_pxActorImage->m_pbCurrImage,
												_CLIP_WIDTH, _CLIP_HEIGHT, g_xGameProc.m_wShadowClr, bShadowAblended, bShadowType, bOpaRate);
					}
					else
					{
						g_xMainWnd.DrawWithShadowABlend(
												m_rcActor.left+3, m_rcActor.top+2, 
												m_pxActorImage->m_lpstNewCurrWilImageInfo->shWidth,							
												m_pxActorImage->m_lpstNewCurrWilImageInfo->shHeight,
												(WORD*)m_pxActorImage->m_pbCurrImage,
												_CLIP_WIDTH, _CLIP_HEIGHT, g_xGameProc.m_wShadowClr, bShadowAblended, 50, bOpaRate);
					}
				}
			}

			if ( m_wABlendDelay )
			{
				if ( !m_bABlendRev )
					bOpaRate = 100 - ( (m_wABlendCurrDelay * 100) / m_wABlendDelay );
				else
					bOpaRate = ( (m_wABlendCurrDelay * 100) / m_wABlendDelay );
			}

			if ( bOpaRate < 0 && bOpaRate > 100 )		bOpaRate  = 0;

			if ( bFocused )								
			{
				if ( !m_bABlendRev )
					bOpaRate -= 20;
				else
					bOpaRate += 20;
			}

			DrawWithEffected(m_rcActor.left, m_rcActor.top, m_pxActorImage->m_lpstNewCurrWilImageInfo->shWidth, m_pxActorImage->m_lpstNewCurrWilImageInfo->shHeight, (WORD*)m_pxActorImage->m_pbCurrImage, 
							 0XFFFF, 0XFFFF, bFocused, bOpaRate, wState);

			DrawEffect();

			return TRUE;
		}
	}
	return FALSE;
}



VOID CActor::DrawHPBar()
{
	if ( m_bOpenHealth )
	{
		FLOAT fHPPercent = (FLOAT)((FLOAT)m_bHPPercent/100);
		RECT rcHP = {0, 0, (36*fHPPercent), 4};
 		g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].NewSetIndex(2);
 		g_xMainWnd.DrawWithImageForComp(
						m_shScrnPosX+7,
						m_shScrnPosY-53,
						g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].m_lpstNewCurrWilImageInfo->shWidth,					
						g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].m_lpstNewCurrWilImageInfo->shHeight,
						(WORD*)g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].m_pbCurrImage);
		g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].NewSetIndex(3);
 		g_xMainWnd.DrawWithImageForComp(
						m_shScrnPosX+7,
						m_shScrnPosY-53,
						rcHP,
						(WORD*)g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].m_pbCurrImage);
	}
}



VOID CActor::DrawName()
{
    auto* surface=g_xMainWnd.GetBackBuffer();HDC dc;
    if(FAILED(surface->GetDC(&dc))) return;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font=g_xMainWnd.CreateGameFont(face,9);HGDIOBJ old=SelectObject(dc,font);
    SetBkMode(dc,TRANSPARENT);
    const char* lines[]={m_szName,m_szGuildName,m_szGuildRank};
    for(int line=0;line<3;++line) {
        if(!*lines[line]) continue;
        SIZE size={};GetTextExtentPoint32A(dc,lines[line],int(strlen(lines[line])),&size);
        const int x=m_shScrnPosX+(_CELL_WIDTH-size.cx)/2;
        const int y=m_shScrnPosY-30+line*15;
        const auto text=[&](int dx,int dy,COLORREF color) {
            RECT area={x+dx,y+dy,x+dx+size.cx,y+dy+15};SetTextColor(dc,color);
            DrawTextA(dc,lines[line],-1,&area,DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_NOPREFIX);
        };
        for(int dx:{-1,1}) for(int dy:{-1,1}) text(dx,dy,RGB(10,10,10));
        text(0,0,m_dwNameClr);
    }
    SelectObject(dc,old);DeleteObject(font);surface->ReleaseDC(dc);
}

bool CActor::ReceiveSpeech(const char* message)
{
    if(strlen(message)>=sizeof(m_szChatMsg)) return false;
    char rows[8][MAX_PATH]={};int widths[8]={};int count=*message?1:0;
    HDC dc=GetDC(g_xMainWnd.GetSafehWnd());
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font=g_xMainWnd.CreateGameFont(face,9);HGDIOBJ old=SelectObject(dc,font);
    std::string wrapped;int start=0,highBytes=0;
    for(int i=0;message[i];++i) {
        SIZE size={};GetTextExtentPoint32A(dc,message+start,i-start+1,&size);
        const bool high=static_cast<signed char>(message[i])<0;
        if(size.cx<=_CHAT_WIDTH) {if(high) ++highBytes;wrapped+=message[i];}
        else if(high && highBytes%2) {wrapped.back()='`';start=i-1;i-=2;--highBytes;++count;}
        else {wrapped+='`';start=i;--i;++count;}
    }
    if(count<=8) {
        sscanf(wrapped.c_str(),"%259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c",rows[0],rows[1],rows[2],rows[3],rows[4],rows[5],rows[6],rows[7]);
        for(int i=0;i<count;++i) {SIZE size={};GetTextExtentPoint32A(dc,rows[i],int(strlen(rows[i])),&size);widths[i]=size.cx;}
    }
    SelectObject(dc,old);DeleteObject(font);ReleaseDC(g_xMainWnd.GetSafehWnd(),dc);
    if(count>8) return false;
    strcpy(m_szChatMsg,message);memcpy(m_szChatMsgArg,rows,sizeof(rows));memcpy(m_speechWidths,widths,sizeof(widths));
    m_nDividedChatLine=count;m_wCurrChatDelay=0;return true;
}

RECT CActor::SpeechBounds() const
{
    if(!m_nDividedChatLine) return {};
    const int width=m_nDividedChatLine==1?m_speechWidths[0]:_CHAT_WIDTH;
    const int x=m_shScrnPosX+28-width/2,y=m_shScrnPosY-55;
    return {x,y-14*m_nDividedChatLine-4,x+width,y};
}

POINT CActor::SpeechPosition(int row) const
{
    int x=SpeechBounds().left;
    if(m_nDividedChatLine>1 && row==m_nDividedChatLine-1) x+=(_CHAT_WIDTH-m_speechWidths[row])/2;
    return {x,m_shScrnPosY-55+14*(row-m_nDividedChatLine)};
}

VOID CActor::ShowMessage(INT nLoopTime)
{
    if(!m_nDividedChatLine) return;
    m_wCurrChatDelay+=nLoopTime;
    if(m_wCurrChatDelay>3000) {
        m_wCurrChatDelay=0;m_nDividedChatLine=0;m_szChatMsg[0]=0;
        ZeroMemory(m_szChatMsgArg,sizeof(m_szChatMsgArg));ZeroMemory(m_speechWidths,sizeof(m_speechWidths));return;
    }
    DrawLegacyPanel(SpeechBounds(),RGB(80,60,40),120,0,false);
    auto* surface=g_xMainWnd.GetBackBuffer();HDC dc;
    if(FAILED(surface->GetDC(&dc))) return;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font=g_xMainWnd.CreateGameFont(face,9);HGDIOBJ old=SelectObject(dc,font);SetBkMode(dc,TRANSPARENT);
    for(int row=0;row<m_nDividedChatLine;++row) {
        const POINT point=SpeechPosition(row);const int length=int(strlen(m_szChatMsgArg[row]));
        SetTextColor(dc,RGB(10,10,10));TextOutA(dc,point.x+1,point.y+1,m_szChatMsgArg[row],length);
        SetTextColor(dc,RGB(255,255,255));TextOutA(dc,point.x,point.y,m_szChatMsgArg[row],length);
    }
    SelectObject(dc,old);DeleteObject(font);surface->ReleaseDC(dc);
}

/******************************************************************************************************************

	CNPC Class

*******************************************************************************************************************/

BOOL CNPC::DrawActor(CMapHandler* pxMap, BOOL bFocused, BOOL bShadowAblended, BOOL bUseScrnPos, BOOL bDrawShadow)
{
	// 坐标处理。
	m_shScrnPosX = (m_wPosX - pxMap->m_shStartViewTileX) * _CELL_WIDTH + _VIEW_CELL_X_START - pxMap->m_shViewOffsetX + m_shShiftPixelX;
	m_shScrnPosY = (m_wPosY - pxMap->m_shStartViewTileY) * _CELL_HEIGHT+ _VIEW_CELL_Y_START - pxMap->m_shViewOffsetY + m_shShiftPixelY;

	BYTE	bShadowType;
	SHORT	shShadowPX;
	SHORT	shShadowPY;

	if ( m_pxActorImage )
	{
		SHORT shPX, shPY;
		m_pxActorImage->NewSetIndex(m_dwCurrFrame);
		shPX = m_pxActorImage->m_lpstNewCurrWilImageInfo->shPX;
		shPY = m_pxActorImage->m_lpstNewCurrWilImageInfo->shPY;

		bShadowType = m_pxActorImage->m_lpstNewCurrWilImageInfo->bShadow;
		shShadowPX  = m_pxActorImage->m_lpstNewCurrWilImageInfo->shShadowPX;
		shShadowPY  = m_pxActorImage->m_lpstNewCurrWilImageInfo->shShadowPY;

		SetRect(&m_rcActor, m_shScrnPosX + shPX, m_shScrnPosY + shPY, 
				m_shScrnPosX + shPX + m_pxActorImage->m_lpstNewCurrWilImageInfo->shWidth, 
				m_shScrnPosY + shPY + m_pxActorImage->m_lpstNewCurrWilImageInfo->shHeight);

		if ( ( m_rcActor.right - m_rcActor.left ) > _CELL_WIDTH + _TARGETRGN_GAPX )
		{
			m_rcTargetRgn.left  = m_rcActor.left + ( (m_rcActor.right - m_rcActor.left) - (_CELL_WIDTH + _TARGETRGN_GAPX) )/2;
			m_rcTargetRgn.right = m_rcActor.left + ( (m_rcActor.right - m_rcActor.left) + (_CELL_WIDTH + _TARGETRGN_GAPX) )/2;
		}
		else
		{
			m_rcTargetRgn.left  = m_rcActor.left;
			m_rcTargetRgn.right = m_rcActor.right;
		}

		if ( ( m_rcActor.bottom - m_rcActor.top ) > _CELL_HEIGHT + _TARGETRGN_GAPY )
		{
			m_rcTargetRgn.top    = m_rcActor.top + ( (m_rcActor.bottom - m_rcActor.top) - (_CELL_HEIGHT + _TARGETRGN_GAPY) )/2;
			m_rcTargetRgn.bottom = m_rcActor.top + ( (m_rcActor.bottom - m_rcActor.top) + (_CELL_HEIGHT + _TARGETRGN_GAPY) )/2;
		}
		else
		{
			m_rcTargetRgn.top    = m_rcActor.top;
			m_rcTargetRgn.bottom = m_rcActor.bottom;
		}

		// 角色影子.
		INT nStartX1 = m_shScrnPosX + shShadowPX;
		INT nStartY1 = m_shScrnPosY + shShadowPY;

		BYTE bOpaRate = 70;
		WORD wState   = GetCharState();

		// 绘制影子。
		if ( m_wABlendDelay || wState==_STATE_ABLEND )
		{
			bShadowAblended = TRUE;
		}

		if ( bDrawShadow )
			g_xMainWnd.DrawWithShadowABlend(
									nStartX1, nStartY1,
									m_pxActorImage->m_lpstNewCurrWilImageInfo->shWidth,							
									m_pxActorImage->m_lpstNewCurrWilImageInfo->shHeight,
									(WORD*)m_pxActorImage->m_pbCurrImage,
									_CLIP_WIDTH, _CLIP_HEIGHT, g_xGameProc.m_wShadowClr, bShadowAblended, bShadowType, bOpaRate);

		if ( m_wABlendDelay )
		{
			if ( !m_bABlendRev )
				bOpaRate = 100 - ( (m_wABlendCurrDelay * 100) / m_wABlendDelay );
			else
				bOpaRate = ( (m_wABlendCurrDelay * 100) / m_wABlendDelay );
		}

		if ( bOpaRate < 0 && bOpaRate > 100 )		bOpaRate  = 0;

		if ( bFocused )								
		{
			if ( !m_bABlendRev )
				bOpaRate -= 20;
			else
				bOpaRate += 20;
		}

		DrawWithEffected(m_rcActor.left, m_rcActor.top, m_pxActorImage->m_lpstNewCurrWilImageInfo->shWidth, m_pxActorImage->m_lpstNewCurrWilImageInfo->shHeight, (WORD*)m_pxActorImage->m_pbCurrImage, 
			             0XFFFF, 0XFFFF, bFocused, bOpaRate, wState);

		return TRUE;
	}
	return FALSE;
}


VOID CNPC::OnTurn(LPPACKETMSG lpPacketMsg)
{
	BYTE bDir;
	m_wPosX = lpPacketMsg->stDefMsg.wParam;
	m_wPosY = lpPacketMsg->stDefMsg.wTag;
	bDir		 = LOBYTE(lpPacketMsg->stDefMsg.wSeries);
	m_bLightSize = HIBYTE(lpPacketMsg->stDefMsg.wSeries);
	OnCharDescPacket(lpPacketMsg);
	SetMotionFrame(_MT_NPC_STAND, bDir);
}


VOID CNPC::OnHit(LPPACKETMSG lpPacketMsg)
{
	BYTE bDir;
	bDir	= lpPacketMsg->stDefMsg.wSeries;
	m_wPosX = lpPacketMsg->stDefMsg.wParam;
	m_wPosY = lpPacketMsg->stDefMsg.wTag;
	SetMotionFrame(_MT_NPC_ACT01, bDir);
}

BOOL CNPC::UpdatePacketState()
{
//	if ( m_bCurrMtn == _MT_NPC_STAND )
	{
		LPPACKETMSG	lpPacketMsg;

		SHORT shLeftMsgCnt = m_xPacketQueue.GetCount();

		if ( shLeftMsgCnt > 0 )
		{
			lpPacketMsg = (LPPACKETMSG)m_xPacketQueue.PopQ();

			if ( shLeftMsgCnt >= 3 )			m_bMsgHurryCheck = TRUE;
			else								m_bMsgHurryCheck = FALSE;

			if ( lpPacketMsg )
			{
				switch ( lpPacketMsg->stDefMsg.wIdent )
				{
					case SM_OPENHEALTH:			{	OnOpenHealth(lpPacketMsg);			break;	}
					case SM_CLOSEHEALTH:		{	OnCloseHealth(lpPacketMsg);			break;	}
					case SM_CHANGELIGHT:		{	OnChangeLight(lpPacketMsg);			break;	}
					case SM_USERNAME:			{	OnUserName(lpPacketMsg);			break;	}
					case SM_CHANGENAMECOLOR:	{	OnChangeNameClr(lpPacketMsg);		break;	}
					case SM_HEALTHSPELLCHANGED:	{	OnHealthSpellChanged(lpPacketMsg);	break;	}
					case SM_TURN:				{	OnTurn(lpPacketMsg);				break;	}
					case SM_HIT:				{	OnHit(lpPacketMsg);					break;	}
					default:
					{										
						break;	
					}
				}
			}
			SAFE_DELETE(lpPacketMsg);
			return TRUE;
		}
	}
	return FALSE;
}



VOID CNPC::UpdateMotionState(INT nLoopTime, BOOL bIsMoveTime)
{
	m_wCurrDelay += nLoopTime;

	if ( m_wCurrDelay > m_wDelay )
	{
		m_wCurrDelay = 0;

		if ( m_dwCurrFrame < m_dwEndFrame )			m_dwCurrFrame++;
	}

	UpdatePacketState();

	if ( m_dwCurrFrame >= m_dwEndFrame )			m_dwCurrFrame = m_dwFstFrame;
}









/******************************************************************************************************************

	CHero Class

*******************************************************************************************************************/

CHero::CHero()
{
	InitActor();
}


CHero::~CHero()
{
	DestroyActor();
}


VOID CHero::InitActor()
{
    m_forcedMotion.reset();
    m_spellTint={};m_spellLightActive=false;m_spellFrameElapsed=0;
	CActor::InitActor();

	ZeroMemory(&m_stFeatureEx, sizeof(FEATUREEX));

	m_bWeaponImgIdx = 0;
	m_bHairImgIdx	= 0;	
	m_bHorseImgIdx	= 0;	
	m_bYedoCnt		= 0;
	m_bFireHitCnt	= 0;
	m_shHitSpeed	= 0;

	m_pxHairImage	= NULL;
	m_pxWeaponImage	= NULL;
	m_pxHorseImage	= NULL;

	m_bUseBanwol	= FALSE;
	m_bUseErgum		= FALSE;
	m_bIsMon		= FALSE;

	m_bShieldCurrFrm	= 0;
	m_wShieldCurrDelay	= 0;
	m_dwCurrHairFrame	= 0;
	m_dwCurrWeaponFrame	= 0;
	m_dwCurrHorseFrame	= 0;
	m_shCurrMagicID		= 0;

	ZeroMemory(&m_rcHair, sizeof(RECT));
	ZeroMemory(&m_rcWeapon, sizeof(RECT));
	ZeroMemory(&m_rcHorse, sizeof(RECT));
}


VOID CHero::DestroyActor()
{
	CActor::DestroyActor();
	InitActor();
}




BOOL CHero::Create(CImageHandler* pxImgHandler, BYTE bMtn, BYTE bDir, WORD wPosX, WORD wPosY, 
				   FEATURE* pstFeature, FEATUREEX* pstFeatureEx)
{
	if ( CActor::Create(pxImgHandler, pstFeature, bMtn, bDir, wPosX, wPosY) )
	{
		m_bIsMon		= FALSE;
		
//		memcpy(&m_stFeatureEx, pstFeatureEx, sizeof(FEATUREEX));

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		m_stFeatureEx.bHorse = 0;
		m_stFeatureEx.wDressColor = 0XFFFF;
		m_stFeatureEx.wHairColor = 0XFFFF;
        if(pstFeatureEx) m_stFeatureEx=*pstFeatureEx;
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


		if ( m_stFeature.bHair	  >= _MAX_HAIR )	m_stFeature.bHair	= _HAIR_NONE;
		if ( m_stFeature.bWeapon  >= _MAX_WEAPON )	m_stFeature.bWeapon	= _WEAPON_NONE;
		if ( m_stFeatureEx.bHorse >= _MAX_HORSE )	m_stFeatureEx.bHorse= _HORSE_NONE;

		m_bHorseImgIdx	= _IMAGE_HORSE;
		if ( m_stFeature.bGender == _GENDER_MAN )
		{
			m_bHairImgIdx	= _IMAGE_M_HAIR;

			if ( m_stFeature.bWeapon != _WEAPON_NONE )			m_bWeaponImgIdx	= _IMAGE_M_WEAPON1+(m_stFeature.bWeapon-1)/10;
		}
		else if ( m_stFeature.bGender == _GENDER_WOMAN )
		{
			m_bHairImgIdx	= _IMAGE_WM_HAIR;
			if ( m_stFeature.bWeapon != _WEAPON_NONE )			m_bWeaponImgIdx	= _IMAGE_WM_WEAPON1+(m_stFeature.bWeapon-1)/10;
		}
		else			return FALSE;

		if ( m_stFeature.bHair != _HAIR_NONE )		m_pxHairImage	= &pxImgHandler->m_xImageList[m_bHairImgIdx];
		else										m_pxHairImage	= NULL;

		if ( m_stFeature.bWeapon != _WEAPON_NONE )	m_pxWeaponImage	= &pxImgHandler->m_xImageList[m_bWeaponImgIdx];
		else										m_pxWeaponImage	= NULL;

		if ( m_stFeatureEx.bHorse != _HORSE_NONE )	m_pxHorseImage	= &pxImgHandler->m_xImageList[m_bHorseImgIdx];
		else										m_pxHorseImage	= NULL;
  
		return TRUE;
	}
	return FALSE;
}



VOID CHero::PlayActSound()
{
	INT nWaveNum = -1;

	if ( m_dwCurrFrame == m_dwFstFrame+1 && m_bCurrMtn == _MT_HITTED )
	{
		switch ( m_stHitter.bWeapon )
		{
			case 21: case 24: // 短剑，非短剑.
			case 8: case 9: // 木剑, 阿里水木剑.
			case 18:  case 22:  case 23:  case 26:  case 27:  case 28:  case 30:  // 蛇矛剑. 青铜剑. 铁剑. 青音剑. 辟邪剑. 天灵. 哭声剑.
			case 1:   case 4:  case 11:  case 13:  case 14:  case 20:  case 25:  case 29:  case 31:   // 柳月刀. 墨青大刀. 六合刀. 军刀. 屠龙宝刀. 四角刀. 细尖刀. 锐刀. 招魂刀.
				nWaveNum = 70;
				break;
			case 5:  case 10:  case 12: // 三戟大斧. 青铜斧. 连子斧.
			case 15:  // 破雷震堂.
				nWaveNum = 71;
				break;
			case 2:  case 3:  case 6:  case 7: case 16:  case 17: case 19:  // 三叉戟. 天刑木. 红牙枪. 镐. 青马枪, 龙牙杖. 制魔棒
				nWaveNum = 72;
				break;
			default:
				nWaveNum = 73;	// 空手。
				break;
		}
		if ( m_stFeature.bDress == 3 )		// 未甲胄.
			nWaveNum += 10;
	}
	else if ( m_dwCurrFrame == m_dwFstFrame+2 && m_bCurrMtn == _MT_HITTED )
	{
		if ( m_stFeature.bGender == _GENDER_MAN )	nWaveNum = 138;
		else										nWaveNum = 139;
	}
	else if ( m_dwCurrFrame == m_dwFstFrame+1 && m_bCurrMtn == _MT_DIE )
	{
		if ( m_stFeature.bGender == _GENDER_MAN )	nWaveNum = 144;
		else										nWaveNum = 145;

	}
	else if ( m_dwCurrFrame == m_dwFstFrame+1 && m_bCurrMtn >= _MT_ONEVSWING && m_bCurrMtn <= _MT_SPEARHSWING )
	{
		if ( m_bFireHitCnt == 2 )
		{
			nWaveNum = 137;
		}
		else if ( m_bYedoCnt == 2 )
		{
/*			if ( m_stFeature.bGender == _GENDER_MAN )
			{
				g_xSound.PlayActorWav(m_wPosX, m_wPosY, g_xGameProc.m_xMyHero.m_wPosX, g_xGameProc.m_xMyHero.m_wPosY, 130);
			}
			else
			{
				g_xSound.PlayActorWav(m_wPosX, m_wPosY, g_xGameProc.m_xMyHero.m_wPosX, g_xGameProc.m_xMyHero.m_wPosY, 131);
			}
*/			m_bYedoCnt = 0;
		}
		else if ( m_bUseBanwol )
		{
			nWaveNum = 133;
		}
		else if ( m_bUseErgum )
		{
			nWaveNum = 132;
		}
	}
	else if ( m_dwCurrFrame == m_dwFstFrame+2 && m_bCurrMtn >= _MT_ONEVSWING && m_bCurrMtn <= _MT_SPEARHSWING )
	{
		switch ( m_stFeature.bWeapon )
		{
		case 21: case 24: // 短剑，非短剑.
			nWaveNum = 50;
			break;
		case 8: case 9: // 木剑, 阿里水木剑.
			nWaveNum = 51;
			break;
		case 18:  case 22:  case 23:  case 26:  case 27:  case 28:  case 30:  // 蛇矛剑. 青铜剑. 铁剑. 青音剑. 辟邪剑. 天灵. 哭声剑.
			nWaveNum = 52;
			break;
		case 1:   case 4:  case 11:  case 13:  case 14:  case 20:  case 25:  case 29:  case 31:   // 柳月刀. 墨青大刀. 六合刀. 军刀. 屠龙宝刀. 四角刀. 细尖刀. 锐刀. 招魂刀.
			nWaveNum = 53;
			break;
		case 5:  case 10:  case 12: // 三戟大斧. 青铜斧. 连子斧.
			nWaveNum = 54;
			break;
		case 15:  // 破雷震堂.
			nWaveNum = 55;
			break;
		case 2:  case 3:  case 6:  case 7: case 16:  case 17: case 19:  // 三叉戟. 天刑木. 红牙枪. 镐. 青马枪, 龙牙杖. 制魔棒
			nWaveNum = 56;
			break;
		default:
			nWaveNum = 57;	// 空手。
			break;
		}
	}
	else if ( m_dwCurrFrame == m_dwFstFrame+1 && (m_bCurrMtn == _MT_SPELL1 || m_bCurrMtn == _MT_SPELL2) )
	{
		const INT nMagicSndTbl[31] = 
		{
			 0,  6,  8, 14, 15,  9,  0, 20, 21, 32,
			 1,  2, 16, 29, 24, 10, 17, 33, 19, 31,
			28, 22, 13, 21,  5,  0, 12, 11,  0, 18, 
			30,
		};
		if ( m_shCurrMagicID < 31 )
		{
			nWaveNum = nMagicSndTbl[m_shCurrMagicID]*10 + 10000;
		}
		if ( nWaveNum < 10010 )
			nWaveNum = -1;
	}

	if ( nWaveNum != -1 )
		g_xSound.PlayActorWav(m_wPosX, m_wPosY, g_xGameProc.m_xMyHero.m_wPosX, g_xGameProc.m_xMyHero.m_wPosY, nWaveNum);
}


VOID CHero::PlayMoveSound()
{
	if ( m_bCurrMtn == _MT_WALK || m_bCurrMtn == _MT_RUN || m_bCurrMtn == _MT_HORSEWALK || m_bCurrMtn == _MT_HORSERUN || m_bCurrMtn == _MT_MOODEPO || m_bCurrMtn == _MT_PUSHBACK )
	{
		if ( m_dwCurrFrame == m_dwEndFrame-5 || m_bBackStepFrame == m_dwEndFrame-5 )
			g_xSound.PlayActorWav(m_wPosX, m_wPosY, g_xGameProc.m_xMyHero.m_wPosX, g_xGameProc.m_xMyHero.m_wPosY, 1,100);
		if ( m_dwCurrFrame == m_dwEndFrame-2 || m_bBackStepFrame == m_bBackStepFrameCnt-2 )
			g_xSound.PlayActorWav(m_wPosX, m_wPosY, g_xGameProc.m_xMyHero.m_wPosX, g_xGameProc.m_xMyHero.m_wPosY, 1,100);
	}
}


VOID CHero::ShowShield()
{
	if ( m_nState & 0X100000 )
	{
		WORD wShieldFrm;
		if ( m_bCurrMtn == _MT_HITTED )			wShieldFrm = m_bShieldCurrFrm + 853;
		else									wShieldFrm = m_bShieldCurrFrm + 850;

		if ( g_xMainWnd.Get3DDevice() )
		{
			if( SUCCEEDED(g_xMainWnd.Get3DDevice()->BeginScene()) )
			{
				D3DVECTOR vTrans;
				D3DMATRIX matTrans;
				D3DMATRIX matScale;
				D3DMATRIX matRot;
				D3DMATRIX matWorld;
				D3DMATRIX matWorldOriginal;

				g_xMainWnd.Get3DDevice()->GetTransform(D3DTRANSFORMSTATE_WORLD, &matWorldOriginal);

				D3DMATERIAL7 mtrl;
				CWHWilImageData* pxWilImg;

				pxWilImg = &g_xGameProc.m_xImage.m_xImageList[_IMAGE_MAGIC];

				if ( pxWilImg->NewSetIndex(wShieldFrm) )
				{
//					if ( !D3DWILTextr_GetSurface(pxWilImg->m_szWilFileName, wShieldFrm) )
//						D3DWILTextr_Restore(pxWilImg->m_szWilFileName, wShieldFrm, g_xMainWnd.Get3DDevice());

 					vTrans.x = (FLOAT)m_shScrnPosX+(FLOAT)pxWilImg->m_lpstNewCurrWilImageInfo->shWidth/2+pxWilImg->m_lpstNewCurrWilImageInfo->shPX-400; 
					vTrans.y = (FLOAT)-m_shScrnPosY-(FLOAT)pxWilImg->m_lpstNewCurrWilImageInfo->shHeight/2-pxWilImg->m_lpstNewCurrWilImageInfo->shPY+300; 
					vTrans.z = 0;
					D3DUtil_SetTranslateMatrix(matTrans, vTrans);
					D3DUtil_SetScaleMatrix(matScale, (FLOAT)pxWilImg->m_lpstNewCurrWilImageInfo->shWidth, (FLOAT)pxWilImg->m_lpstNewCurrWilImageInfo->shHeight, 0.0f);
					D3DMath_MatrixMultiply(matWorld, matScale, matTrans);
					g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld);

//					g_xMainWnd.Get3DDevice()->SetTexture(0, D3DWILTextr_GetSurface(pxWilImg->m_szWilFileName, wShieldFrm));
					g_xGameProc.m_xImage.AddTextr(_TEXTR_FILE_MAGIC,_IMAGE_MAGIC, wShieldFrm);
					LPDIRECTDRAWSURFACE7 lpddsTextr = g_xGameProc.m_xImage.GetTextrImg(_TEXTR_FILE_MAGIC, _IMAGE_MAGIC, wShieldFrm);
					g_xMainWnd.Get3DDevice()->SetTexture(0, lpddsTextr);
					D3DUtil_InitMaterial(mtrl, 
										 (FLOAT)255/255.0f,
										 (FLOAT)255/255.0f, 
										 (FLOAT)255/255.0f);
					mtrl.diffuse.a = 0/255.0f;
					g_xMainWnd.Get3DDevice()->SetMaterial(&mtrl);

					SetBlendRenderState(g_xMainWnd.Get3DDevice(), _BLEND_LIGHTINV, mtrl);

					g_xMainWnd.Get3DDevice()->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX, m_avBoard, 4, NULL);

					// 恢复原状.
					ZeroMemory(&mtrl, sizeof(mtrl));
					mtrl.diffuse.r = mtrl.diffuse.g = mtrl.diffuse.b = 0.1f;
					mtrl.ambient.r = mtrl.ambient.g = mtrl.ambient.b = 1.0f;
					g_xMainWnd.Get3DDevice()->SetMaterial(&mtrl);

					ResetBlendenderState(g_xMainWnd.Get3DDevice());
					g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorldOriginal);
				}
				
				g_xMainWnd.Get3DDevice()->EndScene();
			}
		}
	}
}



BOOL CHero::UpdateMove(BOOL bIsMoveTime)
{
	if ( m_bCurrMtn == _MT_WALK || m_bCurrMtn == _MT_RUN || m_bCurrMtn == _MT_HORSEWALK || m_bCurrMtn == _MT_HORSERUN || m_bCurrMtn == _MT_MOODEPO || m_bCurrMtn == _MT_PUSHBACK )
	{
		m_wCurrDelay = 0;

		if ( bIsMoveTime )
		{
			if ( m_bCurrMtn == _MT_PUSHBACK )
			{
				m_bBackStepFrame += 2;
				if ( m_bBackStepFrame >= m_bBackStepFrameCnt )
				{
					m_dwCurrFrame++;
					m_bBackStepFrame = m_bBackStepFrameCnt;
				}
			}
			else if ( m_bCurrMtn == _MT_MOODEPO )
			{
				m_dwCurrFrame+=2;
				if ( m_dwCurrFrame >= m_dwEndFrame )		m_dwCurrFrame = m_dwEndFrame;
			}
			else
			{
				m_dwCurrFrame++;
				PlayMoveSound();
				if ( m_bMsgHurryCheck )			
				{
					m_dwCurrFrame++;
				}
			}

			if ( m_bCurrMtn == _MT_PUSHBACK )
			{
				if ( m_bBackStepFrame >= m_bBackStepFrameCnt-m_bMoveNextFrmCnt && !m_bIsMoved )
				{
					SetMoved();
					m_bIsMoved = TRUE;
				}
			}
			else
			{
				if ( m_dwCurrFrame >= m_dwEndFrame-m_bMoveNextFrmCnt && !m_bIsMoved )
				{
					SetMoved();
					m_bIsMoved = TRUE;
				}
			}
		}

		// 连续帧中要做的事。
		if ( m_dwCurrFrame >= m_dwEndFrame )
		{
			m_dwCurrFrame = m_dwEndFrame - 1;

			m_shShiftTileX = 0;
			m_shShiftTileY = 0;
			m_shShiftPixelX = 0;
			m_shShiftPixelY = 0;

			m_dwCurrEffectFrame = 0;
			m_dwFstEffectFrame = 0;
			m_dwEndEffectFrame = 0;
			m_bEffectFrame = 0;
			m_bEffectFrameCnt = _DEFAULT_SPELLFRAME;
			m_bUseEffect = FALSE;
			m_bUseSwordEffect = FALSE;
			m_dwCurrFrame = m_dwFstFrame;

			m_bBackStepFrame = 0;
			m_bBackStepFrameCnt = 0;

			m_bIsMoved = FALSE;

			if ( m_stFeatureEx.bHorse == _HORSE_NONE )
			{
				if ( m_bWarMode )
					SetMotionFrame(_MT_ATTACKMODE, m_bCurrDir);
				else
					SetMotionFrame(_MT_STAND, m_bCurrDir);
			}
			else
			{
				SetMotionFrame(_MT_HORSESTAND, m_bCurrDir);
			}
		}
		else if ( m_dwCurrFrame < m_dwEndFrame )
		{
			if ( m_bCurrMtn != _MT_PUSHBACK )
				SetMoving();
			else
				SetBackStepMoving();
		}
		return TRUE;
	}
	return FALSE;
}


VOID CHero::OnCharDescPacket(LPPACKETMSG lpPacketMsg)
{
	CHARDESC	stCharDesc;
	FEATURE		stFeature;
	fnDecode6BitBuf(lpPacketMsg->szEncodeData, (char*)&stCharDesc, sizeof(CHARDESC));

	memcpy(&stFeature, &stCharDesc.nFeature, sizeof(FEATURE));
	m_nState = stCharDesc.nStatus;

	if( m_nState & 0X2 )	m_bOpenHealth = TRUE;
	else					m_bOpenHealth = FALSE;

	ChangeFeature(stFeature, m_stFeatureEx);
}

VOID CHero::OnFeatureChanged(LPPACKETMSG lpPacketMsg)
{
	FEATURE stFeature;
	LONG nFeature	  = MAKELONG(lpPacketMsg->stDefMsg.wParam, lpPacketMsg->stDefMsg.wTag);

	memcpy(&stFeature, &nFeature, sizeof(LONG));

	ChangeFeature(stFeature, m_stFeatureEx);
}

VOID CHero::OnCharStatusChanged(LPPACKETMSG lpPacketMsg)
{
	m_shHitSpeed = lpPacketMsg->stDefMsg.wSeries;
	m_nState = MAKELONG(lpPacketMsg->stDefMsg.wParam, lpPacketMsg->stDefMsg.wTag);

	if( m_nState & 0X2 )	m_bOpenHealth = TRUE;
	else					m_bOpenHealth = FALSE;
}

void CHero::BeginForcedMotion(LPPACKETMSG packet)
{
    m_forcedMotion=mir3::ForcedMotion::begin(packet->stDefMsg.wIdent,LOBYTE(packet->stDefMsg.wSeries),
        packet->stDefMsg.wParam,packet->stDefMsg.wTag);
    m_bUseEffect=m_bUseSwordEffect=FALSE;m_spellLightActive=false;
    const auto& motion=*m_forcedMotion;
    m_wPosX=WORD(motion.origin[0]);m_wPosY=WORD(motion.origin[1]);
    SetMotionFrame(motion.opcode==mir3::ForcedMotion::Backstep?_MT_PUSHBACK:_MT_MOODEPO,BYTE(motion.facing));
    m_bMoveDir=BYTE(motion.direction());m_bMoveSpeed=_SPEED_WALK;m_bIsMoved=FALSE;
    const auto offset=motion.offset();m_shShiftPixelX=SHORT(offset[0]);m_shShiftPixelY=SHORT(offset[1]);
}
VOID CHero::OnRush(LPPACKETMSG packet) {BeginForcedMotion(packet);}
VOID CHero::OnBackStep(LPPACKETMSG packet) {BeginForcedMotion(packet);}
bool CHero::AdvanceForcedMotion(DWORD elapsed)
{
    if(!m_forcedMotion) return false;
    auto& motion=*m_forcedMotion;motion.advance(elapsed);
    const auto cell=motion.cell(),offset=motion.offset();
    const bool local=this==&g_xGameProc.m_xMyHero;
    const bool changed=m_wPosX!=cell[0] || m_wPosY!=cell[1];
    m_wPosX=WORD(cell[0]);m_wPosY=WORD(cell[1]);
    m_shShiftPixelX=SHORT(offset[0]);m_shShiftPixelY=SHORT(offset[1]);
    m_dwCurrFrame=m_dwFstFrame+(motion.opcode==mir3::ForcedMotion::Backstep?0:motion.frame);
    if(local) {
        auto& hero=g_xGameProc.m_xMyHero;
        if(changed || !hero.CheckMyPostion()) hero.AdjustMyPostion();
        hero.m_pxMap->m_shViewOffsetX=m_shShiftPixelX;hero.m_pxMap->m_shViewOffsetY=m_shShiftPixelY;
    }
    if(motion.finished()) {
        const bool returned=motion.returned();m_forcedMotion.reset();
        m_bMoveSpeed=0;m_bIsMoved=FALSE;
        m_shShiftPixelX=m_shShiftPixelY=m_shShiftTileX=m_shShiftTileY=0;
        SetMotionFrame(m_stFeatureEx.bHorse==_HORSE_NONE?_MT_ATTACKMODE:_MT_HORSESTAND,m_bCurrDir);
        if(local) {
            auto& hero=g_xGameProc.m_xMyHero;
            if(returned) {
                hero.m_wOldPosX=m_wPosX;hero.m_wOldPosY=m_wPosY;hero.m_bOldDir=m_bCurrDir;
                hero.ConfirmAction();
            }
            hero.m_bInputLock=hero.m_spellAwaitingReply;
            hero.AdjustMyPostion();
        }
    }
    return true;
}

VOID CHero::OnDeath(LPPACKETMSG lpPacketMsg)
{
    m_wHP=0;
    m_bHPPercent=0;
    m_forcedMotion.reset();
	BYTE bDir;
	m_wPosX = lpPacketMsg->stDefMsg.wParam;
	m_wPosY = lpPacketMsg->stDefMsg.wTag;
	bDir	= lpPacketMsg->stDefMsg.wSeries;
	OnCharDescPacket(lpPacketMsg);
	SetMotionFrame(_MT_DIE, bDir);
	if ( lpPacketMsg->stDefMsg.wIdent == SM_DEATH )	
	{
		m_dwCurrFrame = m_dwEndFrame - 1;
		m_bIsDead = TRUE;
	}
}

VOID CHero::OnWalk(LPPACKETMSG lpPacketMsg)
{
	BYTE		bDir;
	CHARDESC	stCharDesc;
	FEATURE		stFeature;
//	FEATUREEX	stFeatureEx;
//	m_wPosX = lpPacketMsg->stDefMsg.wParam;
//	m_wPosY = lpPacketMsg->stDefMsg.wTag;
	bDir		 = LOBYTE(lpPacketMsg->stDefMsg.wSeries);
	m_bLightSize = HIBYTE(lpPacketMsg->stDefMsg.wSeries);

	fnDecode6BitBuf(lpPacketMsg->szEncodeData, (char*)&stCharDesc, sizeof(CHARDESC));
//	fnDecode6BitBuf(&lpPacketMsg->szEncodeData[_FEATURESIZE*2], (char*)&stFeatureEx, sizeof(FEATUREEX));

	memcpy(&stFeature, &stCharDesc.nFeature, sizeof(FEATURE));
	m_nState = stCharDesc.nStatus;

	if( m_nState & 0X2 )	m_bOpenHealth = TRUE;
	else					m_bOpenHealth = FALSE;

	ChangeFeature(stFeature, m_stFeatureEx);

	if ( m_stFeatureEx.bHorse == _HORSE_NONE )		SetMotionFrame(_MT_WALK, bDir);
	else											SetMotionFrame(_MT_HORSEWALK, bDir);
						
	m_bMoveSpeed = _SPEED_WALK;
	SetMoving();
}

VOID CHero::OnRun(LPPACKETMSG lpPacketMsg)
{
	BYTE		bDir;
	CHARDESC	stCharDesc;
	FEATURE		stFeature;
//	FEATUREEX	stFeatureEx;
//	m_wPosX = lpPacketMsg->stDefMsg.wParam;
//	m_wPosY = lpPacketMsg->stDefMsg.wTag;
	bDir		 = LOBYTE(lpPacketMsg->stDefMsg.wSeries);
	m_bLightSize = HIBYTE(lpPacketMsg->stDefMsg.wSeries);

	fnDecode6BitBuf(lpPacketMsg->szEncodeData, (char*)&stCharDesc, sizeof(CHARDESC));
//	fnDecode6BitBuf(&lpPacketMsg->szEncodeData[_CHARDESCSIZE], (char*)&stFeatureEx, sizeof(FEATUREEX));

	memcpy(&stFeature, &stCharDesc.nFeature, sizeof(FEATURE));
	m_nState = stCharDesc.nStatus;

	if( m_nState & 0X2 )	m_bOpenHealth = TRUE;
	else					m_bOpenHealth = FALSE;

	ChangeFeature(stFeature, m_stFeatureEx);

	if ( m_stFeatureEx.bHorse == _HORSE_NONE )	
	{
		SetMotionFrame(_MT_RUN, bDir);
		m_bMoveSpeed = _SPEED_RUN;
	}
	else
	{
		SetMotionFrame(_MT_HORSERUN, bDir);
		m_bMoveSpeed = _SPEED_HORSERUN;
	}

	SetMoving();
}


VOID CHero::OnTurn(LPPACKETMSG lpPacketMsg)
{
	BYTE		bDir;
	CHARDESC	stCharDesc;
	FEATURE		stFeature;
	INT			nPos;
	m_wPosX = lpPacketMsg->stDefMsg.wParam;
	m_wPosY = lpPacketMsg->stDefMsg.wTag;
	bDir		 = LOBYTE(lpPacketMsg->stDefMsg.wSeries);
	m_bLightSize = HIBYTE(lpPacketMsg->stDefMsg.wSeries);

	fnDecode6BitBuf(lpPacketMsg->szEncodeData, (char*)&stCharDesc, sizeof(CHARDESC));
    if(strlen(lpPacketMsg->szEncodeData)>=_CHARDESCSIZE+_FEATURESIZEEX)
        fnDecode6BitBuf(&lpPacketMsg->szEncodeData[_CHARDESCSIZE], (char*)&m_stFeatureEx , sizeof(FEATUREEX));

	if (strlen(lpPacketMsg->szEncodeData) > _CHARDESCSIZE+_FEATURESIZEEX)
	{
		nPos = fnDecode6BitBuf(&lpPacketMsg->szEncodeData[_CHARDESCSIZE +_FEATURESIZEEX], m_szName, sizeof(m_szName)-1);
		m_szName[nPos] = '\0';

		CHAR* szClr = strrchr(m_szName, '/');

		if ( szClr )
		{
			m_dwNameClr = GetUserNameColor(atoi(szClr+1));
			*szClr = '\0';
		}
	}

	memcpy(&stFeature, &stCharDesc.nFeature, sizeof(FEATURE));
	m_nState = stCharDesc.nStatus;

	if( m_nState & 0X2 )	m_bOpenHealth = TRUE;
	else					m_bOpenHealth = FALSE;

	if ( !m_bIsMon )
	{
		ChangeFeature(stFeature, m_stFeatureEx);

		if ( m_stFeatureEx.bHorse == _HORSE_NONE )		SetMotionFrame(_MT_STAND, bDir);
		else											SetMotionFrame(_MT_HORSESTAND, bDir);
	}
	else
	{
		SetMotionFrame(_MT_MON_STAND, bDir);
	}
}

VOID CHero::OnStruck(LPPACKETMSG lpPacketMsg)
{
	MESSAGEBODYWL stMsgBodyWl;
	FEATURE		  stFeature;

    std::optional<CMir3HumanAppearance> appearance;
    if(lpPacketMsg->stDefMsg.wIdent==1309) {
        BYTE body[16];if(strlen(lpPacketMsg->szEncodeData)!=22 || fnDecode6BitBuf(lpPacketMsg->szEncodeData,(char*)body,sizeof(body))!=16) return;
        appearance=CMir3HumanAppearance::Parse(body,sizeof(body),true);if(!appearance) return;
        m_bProfession=appearance->profession;m_nState=appearance->flags;
    }
	FLOAT wHPRate	 = (FLOAT)((FLOAT)lpPacketMsg->stDefMsg.wParam/(FLOAT)lpPacketMsg->stDefMsg.wTag);
	WORD  wDamage	 = lpPacketMsg->stDefMsg.wSeries;
    if(!m_bIsDead) AddDamage(-int(wDamage));
	m_wHP			 = lpPacketMsg->stDefMsg.wParam;
	m_wMAXHP		 = lpPacketMsg->stDefMsg.wTag;
	m_bHPPercent     = wHPRate*100;
	m_bShieldCurrFrm = 0;

	fnDecode6BitBuf(lpPacketMsg->szEncodeData, (char*)&stMsgBodyWl, sizeof(MESSAGEBODYWL));
	memcpy(&stFeature, &stMsgBodyWl.lParam1, sizeof(LONG));
    if(appearance) stFeature={appearance->type,appearance->weapon,appearance->clothing,appearance->hair};

	if ( !m_bIsMon )
	{
		ChangeFeature(stFeature, m_stFeatureEx);

		if ( m_stFeatureEx.bHorse == _HORSE_NONE )		SetMotionFrame(_MT_HITTED, m_bCurrDir);
		else											SetMotionFrame(_MT_HORSEHIT, m_bCurrDir);
	}
	else
	{
		SetMotionFrame(_MT_MON_HITTED, m_bCurrDir);
	}

	// 为怪物时.
	if ( m_stHitter.bGender == _GENDER_MON )
	{
		CMagic* pxMagic;
		switch ( m_stHitter.bDress )
		{
		case 2:		// 凯法洛普.
			pxMagic = new CMagic;
			pxMagic->CreateMagic(_MONMAGIC_KEPAL, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this, 0);
			g_xGameProc.m_xMagicList.AddNode(pxMagic);
			break;
		case 8:		// 甲胄蚂蚁.
			pxMagic = new CMagic;
			pxMagic->CreateMagic(_MONMAGIC_GREATANT, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
			g_xGameProc.m_xMagicList.AddNode(pxMagic);
			break;
		case 14:	// 兵蚁.
			pxMagic = new CMagic;
			pxMagic->CreateMagic(_MONMAGIC_ANT, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
			g_xGameProc.m_xMagicList.AddNode(pxMagic);
			break;
		case 16:	// 游荡蚁.
			pxMagic = new CMagic;
			pxMagic->CreateMagic(_MONMAGIC_WORKANT, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
			g_xGameProc.m_xMagicList.AddNode(pxMagic);
			break;
		case 67:	// 烛龙神.
			pxMagic = new CMagic;
			pxMagic->CreateMagic(_MONMAGIC_BIGGINE_CHAR, m_wPosX, m_wPosY, m_wPosX, m_wPosY, this);
			g_xGameProc.m_xMagicList.AddNode(pxMagic);
			break;
		default :
			break;
		}
	}
	ZeroMemory(&m_stHitter, sizeof(FEATURE));
}

VOID CHero::OnHit(LPPACKETMSG lpPacketMsg)
{
	BYTE bDir;
	bDir	= lpPacketMsg->stDefMsg.wSeries;
	m_wPosX = lpPacketMsg->stDefMsg.wParam;
	m_wPosY = lpPacketMsg->stDefMsg.wTag;

	if ( !m_bIsMon )
	{
		m_dwCurrEffectFrame = 50;
		m_dwFstEffectFrame  = 50;
		m_dwEndEffectFrame  = 60;
		m_bEffectFrame		= 0;
		m_bEffectFrameCnt	= _DEFAULT_SPELLFRAME;
		m_bWarMode			= TRUE;

		if ( lpPacketMsg->stDefMsg.wIdent == SM_WIDEHIT )
		{
			SetMotionFrame(_MT_ONEHSWING, bDir);
			LoadEffect(&g_xGameProc.m_xImage, _SKILL_BANWOL, bDir);
			m_bUseSwordEffect = TRUE;
		}
		else if ( lpPacketMsg->stDefMsg.wIdent == SM_POWERHIT )
		{
			SetMotionFrame(_MT_ONEVSWING, bDir);
			LoadEffect(&g_xGameProc.m_xImage, _SKILL_YEDO, bDir);
			m_bUseSwordEffect = TRUE;
		}
		else if ( lpPacketMsg->stDefMsg.wIdent == SM_FIREHIT )
		{
			SetMotionFrame(_MT_ONEVSWING, bDir);
			LoadEffect(&g_xGameProc.m_xImage, _SKILL_FIRESWORD, bDir);
			m_bUseSwordEffect = TRUE;
		}
		else if ( lpPacketMsg->stDefMsg.wIdent == SM_LONGHIT )
		{
			SetMotionFrame(_MT_ONEVSWING, bDir);
			LoadEffect(&g_xGameProc.m_xImage, _SKILL_ERGUM, bDir);
			m_bUseSwordEffect = TRUE;
		}
		else
		{
			bDir = LOBYTE(lpPacketMsg->stDefMsg.wSeries);
			BYTE bAttackMode = HIBYTE(lpPacketMsg->stDefMsg.wSeries);
			SetMotionFrame(_MT_ONEVSWING, bDir);
		}
	}
}

void CHero::OnMagicList(LPPACKETMSG packet)
{
    const auto reply=SpellWire::Reply::Parse(packet->stDefMsg,packet->szEncodeData);
    if(!reply || !reply->multiple) return;
    m_shCurrMagicID=reply->skill;
    if(reply->skill!=WORD(mir3::SkillId::original_113)) return;
    for(DWORD identity:reply->targets) {
        auto& game=g_xGameProc;
        CActor* target=identity==game.m_xMyHero.m_dwIdentity?&game.m_xMyHero:game.FindActor(int(identity));
        if(!identity || !target) continue;
        game.StartNativeSpell(*this,reply->skill,POINT{target->m_wPosX,target->m_wPosY},identity);
    }
}

VOID CHero::OnMagicFire(LPPACKETMSG lpPacketMsg)
{
    const auto reply=SpellWire::Reply::Parse(lpPacketMsg->stDefMsg,lpPacketMsg->szEncodeData);
    if(!reply || reply->multiple) return;
    m_lastSpellReplyId=reply->skill;
    POINT ptPos={reply->x,reply->y};
    const INT nMagicTargetID=INT(reply->targets.front());
    // 413150 ignores these replies without changing the actor's current spell.
    switch(static_cast<mir3::SkillId>(reply->skill)) {
    case mir3::SkillId::original_104: case mir3::SkillId::spacemove:
    case mir3::SkillId::sinsu: case mir3::SkillId::firewind:
    case mir3::SkillId::cloak: case mir3::SkillId::skelleton:
    case mir3::SkillId::original_78: case mir3::SkillId::original_64:
    case mir3::SkillId::original_105: case mir3::SkillId::original_121: return;
    case mir3::SkillId::earthfire: case mir3::SkillId::holyshield:
    case mir3::SkillId::original_63:
        m_shCurrMagicID=reply->skill;return;
    default: break;
    }
    const POINT source={m_wPosX,m_wPosY};
    switch(static_cast<mir3::SkillId>(reply->skill)) {
    case mir3::SkillId::fire:
        g_xGameProc.StartGroundSpell(mir3::MagicEffectId::fire_wave,source,ptPos,m_bCurrDir);break;
    case mir3::SkillId::original_53:
        g_xGameProc.StartGroundSpell(mir3::MagicEffectId::ice_wave,source,ptPos,m_bCurrDir);break;
    case mir3::SkillId::original_110:
        g_xGameProc.StartGroundSpell(mir3::MagicEffectId::three_rays,source,ptPos,m_bCurrDir);break;
    default:
        g_xGameProc.StartNativeSpell(*this,reply->skill,ptPos,DWORD(nMagicTargetID));
        if(reply->skill==WORD(mir3::SkillId::shootlighten))
            g_xGameProc.StartGroundSpell(mir3::MagicEffectId::lightning_trail,source,ptPos,m_bCurrDir);
        break;
    }
    m_shCurrMagicID=reply->skill;
}

BOOL CHero::DrawEffect()
{
    if(m_bCurrMtn!=_MT_SPELL1 && m_bCurrMtn!=_MT_SPELL2) return CActor::DrawEffect();
    const WORD skill=WORD(m_shCurrMagicID);
    const auto* resource=mir3::EffectCatalog::shared().find(mir3::EffectCatalog::Section::spell,skill);
    if(resource && m_bUseEffect) {
        const int direction=mir3::SpellRoutes::get(skill).directional_effect?m_bCurrDir:0;
        if(m_spellLightActive && resource->outer_radius) {
            const BYTE outer=BYTE(m_bEffectFrame>=(m_bEffectFrameCnt>>1)?m_bEffectFrameCnt-m_bEffectFrame:m_bEffectFrame+1);
            const BYTE inner=BYTE(outer-1);
            if(g_xGameProc.m_xInterface.m_xOptionWnd.m_values[9])
                g_xGameProc.m_xLightFog.SetLightRadiusWithRing(m_shScrnPosX+_CELL_WIDTH/2,m_shScrnPosY+_CELL_HEIGHT/2,
                    inner,resource->inner_color[0],resource->inner_color[1],resource->inner_color[2],
                    outer,resource->outer_color[0],resource->outer_color[1],resource->outer_color[2]);
            if(!outer) m_spellLightActive=false;
        }
        auto tinted=*resource;tinted.tint=m_spellTint;
        NativeEffects::Draw(tinted,int(resource->first)+direction*10+m_bEffectFrame,m_shScrnPosX,m_shScrnPosY);
    }
    return TRUE;
}

void CHero::BeginSpellAction(WORD skill,BYTE facing,BYTE action)
{
    m_spellFrameElapsed=0;
    const auto& route=mir3::SpellRoutes::get(skill);
    m_shCurrMagicID=skill;
    m_bWarMode=TRUE;
    m_dwFstEffectFrame=m_dwEndEffectFrame=m_dwCurrEffectFrame=0;
    m_bEffectFrame=0;
    const auto* resource=mir3::EffectCatalog::shared().find(mir3::EffectCatalog::Section::spell,skill);
    m_bUseEffect=resource!=nullptr;m_spellLightActive=resource!=nullptr;
    m_spellTint=resource?resource->tint:mir3::SpellColor::Rgb{};
    if(resource && this==&g_xGameProc.m_xMyHero) {
        std::array<std::uint16_t,AbilityWire::ElementCount> attack={};
        const BYTE* extension=g_xGameProc.m_xInterface.m_xStatusWnd.m_abilityExtension;
        for(int i=0;i<AbilityWire::ElementCount;++i) {
            const int offset=AbilityWire::AttackElements-AbilityWire::Extension+2*i;
            attack[i]=WORD(extension[offset])|(WORD(extension[offset+1])<<8);
        }
        m_spellTint=mir3::SpellColor::strongest(attack,m_spellTint);
    }
    if(resource) {
        m_dwFstEffectFrame=resource->first+(route.directional_effect?facing*10:0);
        m_dwCurrEffectFrame=m_dwFstEffectFrame;m_dwEndEffectFrame=resource->end+(route.directional_effect?facing*10:0);
        m_bLightRadius[0]=resource->inner_radius;m_bLightRadius[1]=resource->outer_radius;
        for(int channel=0;channel<3;++channel) {
            m_bLightColor[0][channel]=resource->inner_color[channel];
            m_bLightColor[1][channel]=resource->outer_color[channel];
        }
    }
    m_bEffectFrameCnt=BYTE(route.action_limit);
    SetMotionFrame(action,facing);
    constexpr WORD NormalSpellInterval=75;
    m_wDelay=route.action_interval?WORD(route.action_interval):NormalSpellInterval;
    if(mir3::EffectCatalog::shared().find(mir3::EffectCatalog::Section::spell,skill)) {
        const int sound=10*(int(skill)+1000);
        g_xGameProc.PlaySpellSound(sound,{m_wPosX,m_wPosY});
    }
}

bool CHero::AdvanceSpellAnimation(DWORD elapsed)
{
    if(m_bCurrMtn!=_MT_SPELL1 && m_bCurrMtn!=_MT_SPELL2) return false;
    m_wCurrDelay=0;m_spellFrameElapsed+=elapsed;
    const DWORD interval=DWORD(m_wDelay)*((DWORD(m_nState)&_STATE_SLOWACTION)?2:1);
    if(m_dwCurrFrame<m_dwEndFrame && LONG(m_spellFrameElapsed)>LONG(interval)) {
        m_spellFrameElapsed=0;++m_dwCurrFrame;++m_dwCurrEffectFrame;++m_bEffectFrame;
        if(this!=&g_xGameProc.m_xMyHero && m_bMsgHurryCheck) {
            m_wDelay/=2;m_bMsgHurryCheck=FALSE;
        }
        ApplySpellPhase();
    }
    return true;
}

void CHero::ApplySpellPhase()
{
    if(m_bCurrMtn==_MT_SPELL2) {
        if(m_dwCurrFrame>=m_dwEndFrame-1 && m_bEffectFrame<=BYTE(m_bEffectFrameCnt-2)) {
            m_wDelay=70;m_dwCurrFrame=m_dwEndFrame-2;
        }
    } else if(m_bCurrMtn==_MT_SPELL1) {
        if(m_dwCurrFrame>=m_dwEndFrame-3 && m_dwCurrFrame<m_dwEndFrame-2) {
            if(m_bEffectFrame<BYTE(m_bEffectFrameCnt-4)) m_dwCurrFrame=m_dwEndFrame-4;
        } else if(m_dwCurrFrame>=m_dwEndFrame-2 && m_dwCurrFrame<m_dwEndFrame-1) m_wDelay=200;
        else if(m_dwCurrFrame>=m_dwEndFrame-1) m_wDelay=80;
    }
}

VOID CHero::OnSpell(LPPACKETMSG lpPacketMsg)
{
    const WORD skill=WORD(atoi(lpPacketMsg->szEncodeData));
    const BYTE facing=g_xGameProc.m_xMap.CalcDirection8(m_wPosX,m_wPosY,
        lpPacketMsg->stDefMsg.wParam,lpPacketMsg->stDefMsg.wTag);
    BeginSpellAction(skill,facing,BYTE(mir3::SpellRoutes::remote_action(skill)));
}


VOID CHero::OnButch(LPPACKETMSG lpPacketMsg)
{
	BYTE bDir;
	m_wPosX = lpPacketMsg->stDefMsg.wParam;
	m_wPosY = lpPacketMsg->stDefMsg.wTag;
	bDir	= lpPacketMsg->stDefMsg.wSeries;
	SetMotionFrame(_MT_CUT, bDir);
}


BOOL CHero::UpdatePacketState()
{
	LPPACKETMSG	lpPacketMsg = NULL;


	if ( m_bCurrMtn == _MT_STAND || m_bCurrMtn == _MT_ATTACKMODE || m_bCurrMtn == _MT_HORSESTAND )
	{
		LPPACKETMSG	lpPacketMsg;
		SHORT shLeftMsgCnt = m_xPacketQueue.GetCount();

		if ( shLeftMsgCnt > 0 )
		{
			lpPacketMsg = (LPPACKETMSG)m_xPacketQueue.PopQ();

			if ( shLeftMsgCnt >= 3 )		m_bMsgHurryCheck = TRUE;
			else							m_bMsgHurryCheck = FALSE;

			if ( lpPacketMsg )
			{
				switch ( lpPacketMsg->stDefMsg.wIdent )
				{
					case SM_SITDOWN:
					case SM_BUTCH:				{	OnButch(lpPacketMsg);				break;	}
					case SM_FEATURECHANGED:		{	OnFeatureChanged(lpPacketMsg);		break;	}
					case SM_CHARSTATUSCHANGE:	{	OnCharStatusChanged(lpPacketMsg);	break;	}
					case SM_OPENHEALTH:			{	OnOpenHealth(lpPacketMsg);			break;	}
					case SM_CLOSEHEALTH:		{	OnCloseHealth(lpPacketMsg);			break;	}
					case SM_CHANGELIGHT:		{	OnChangeLight(lpPacketMsg);			break;	}
					case SM_USERNAME:			{	OnUserName(lpPacketMsg);			break;	}
					case SM_CHANGENAMECOLOR:	{	OnChangeNameClr(lpPacketMsg);		break;	}
					case SM_HEALTHSPELLCHANGED:	{	OnHealthSpellChanged(lpPacketMsg);	break;	}
					case mir3::ForcedMotion::Bump:
					case SM_RUSH:				{	OnRush(lpPacketMsg);				break;	}
					case SM_BACKSTEP:			{	OnBackStep(lpPacketMsg);			break;	}
					case SM_NOWDEATH:
					case SM_DEATH:				{	OnDeath(lpPacketMsg);				break;	}
					case SM_WALK:				{	OnWalk(lpPacketMsg);				break;	}
					case SM_RUN:				{	OnRun(lpPacketMsg);					break;	}
					case SM_TURN:				{	OnTurn(lpPacketMsg);				break;	}
					case 1309:
					case SM_STRUCK:				{	OnStruck(lpPacketMsg);				break;	}
					case SM_HIT:
					case SM_FIREHIT:
					case SM_LONGHIT:
					case SM_POWERHIT:
					case SM_WIDEHIT:			{	OnHit(lpPacketMsg);					break;	}
					case SM_MAGICFIRE:			{	OnMagicFire(lpPacketMsg);			break;	}
                    case SpellWire::MultipleReply: {OnMagicList(lpPacketMsg);break;}
					case SM_SPELL:				{	OnSpell(lpPacketMsg);				break;	}
					default:
					{										
						break;	
					}
				}
			}

			SAFE_DELETE(lpPacketMsg);
			return TRUE;
		}
	}
	return FALSE;
}



VOID CHero::UpdateMotionState(INT nLoopTime, BOOL bIsMoveTime)
{
	if ( m_bWarMode ) 
		m_dwWarModeTime += nLoopTime;

	m_wABlendCurrDelay += nLoopTime;

	if ( m_wABlendCurrDelay >= m_wABlendDelay )
	{
		m_wABlendCurrDelay = 0;
		m_wABlendDelay  = 0;
		m_bABlendRev	= FALSE;
	}

	if ( m_bCurrMtn == _MT_DIE && m_dwCurrFrame >= m_dwEndFrame-1 )
	{
		m_bIsDead = TRUE;
	}

	if ( m_bIsDead )
	{
		SetMotionFrame(_MT_DIE, m_bCurrDir);
		m_dwCurrFrame = m_dwEndFrame - 1;
		return;
	}

    if(AdvanceForcedMotion(DWORD(nLoopTime))) {UpdatePacketState();return;}
	if ( UpdateMove(bIsMoveTime) )
	{
		UpdatePacketState();
		return;
	}
	else
	{
		m_wCurrDelay += nLoopTime;

		m_wShieldCurrDelay += nLoopTime;
		if ( m_wShieldCurrDelay > 150 )  
		{
			m_bShieldCurrFrm++;
			m_wShieldCurrDelay = 0;
			if ( m_bShieldCurrFrm > 2 )
				m_bShieldCurrFrm = 0;
		}

		if ( !AdvanceSpellAnimation(DWORD(nLoopTime)) && m_wCurrDelay > m_wDelay )
		{
			m_wCurrDelay = 0;

			if ( m_dwCurrFrame < m_dwEndFrame )
			{
				m_dwCurrFrame++;

				PlayActSound();

				if ( (m_bCurrMtn == _MT_SPELL2 || m_bCurrMtn == _MT_SPELL1) )
				{
					m_dwCurrEffectFrame++;
					m_bEffectFrame++;
				}

				if ( m_bMsgHurryCheck )			
				{
					m_wDelay = m_wDelay/2;
					m_bMsgHurryCheck = FALSE;
				}
			}
		}

		UpdatePacketState();
		
		if ( m_dwWarModeTime > _WARMODE_TIME )
		{
			m_dwWarModeTime = 0;
			m_bWarMode		= FALSE;
		}


        if ( m_dwCurrFrame >= m_dwEndFrame )
		{
			m_dwCurrEffectFrame = 0;
			m_dwFstEffectFrame = 0;
			m_dwEndEffectFrame = 0;
			m_bEffectFrame = 0;
			m_bEffectFrameCnt = _DEFAULT_SPELLFRAME;
			m_bUseEffect = FALSE;
			m_bUseSwordEffect = FALSE;
			m_dwCurrFrame = m_dwFstFrame;

			if ( m_stFeatureEx.bHorse == _HORSE_NONE )
			{
				if ( m_bWarMode )
					SetMotionFrame(_MT_ATTACKMODE, m_bCurrDir);
				else
					SetMotionFrame(_MT_STAND, m_bCurrDir);
			}
			else
			{
				SetMotionFrame(_MT_HORSESTAND, m_bCurrDir);
			}
		}

	}
}



/******************************************************************************************************************

	函数名 : CHero::ChangeFeature()

	作者 :
	编写日期 :

	目的   : BYTE bGender, BYTE bWear, BYTE bHair, BYTE bWeapon;
	输入   : WORD wActor
	         BYTE bHair
	         BYTE bWeapon
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CHero::ChangeFeature(FEATURE stFeature, FEATUREEX stFeatureEx)
{
	if ( CActor::ChangeFeature(stFeature) )
	{
		CImageHandler* pxImgHandler = &g_xGameProc.m_xImage;

		m_stFeatureEx = stFeatureEx;




//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//		m_stFeatureEx.bHorse = 0;
//		m_stFeatureEx.wDressColor = 0XFFFF;
//		m_stFeatureEx.wHairColor = 0XFFFF;
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!






		if ( m_stFeature.bHair	  >= _MAX_HAIR )	m_stFeature.bHair	= _HAIR_NONE;
		if ( m_stFeature.bWeapon  >= _MAX_WEAPON )	m_stFeature.bWeapon	= _WEAPON_NONE;
		if ( m_stFeatureEx.bHorse >= _MAX_HORSE )	m_stFeatureEx.bHorse= _HORSE_NONE;

		m_bHorseImgIdx	= _IMAGE_HORSE;
		if ( m_stFeature.bGender == _GENDER_MAN )
		{
			m_bHairImgIdx	= _IMAGE_M_HAIR;
			if ( m_stFeature.bWeapon != _WEAPON_NONE )			m_bWeaponImgIdx	= _IMAGE_M_WEAPON1+(m_stFeature.bWeapon-1)/10;
		}
		else if ( m_stFeature.bGender == _GENDER_WOMAN )
		{
			m_bHairImgIdx	= _IMAGE_WM_HAIR;
			if ( m_stFeature.bWeapon != _WEAPON_NONE )			m_bWeaponImgIdx	= _IMAGE_WM_WEAPON1+(m_stFeature.bWeapon-1)/10;
		}
		else			return FALSE;

		if ( m_stFeature.bHair != _HAIR_NONE )		m_pxHairImage	= &pxImgHandler->m_xImageList[m_bHairImgIdx];
		else										m_pxHairImage	= NULL;

		if ( m_stFeature.bWeapon != _WEAPON_NONE )	m_pxWeaponImage	= &pxImgHandler->m_xImageList[m_bWeaponImgIdx];
		else										m_pxWeaponImage	= NULL;

		if ( m_stFeatureEx.bHorse != _HORSE_NONE )	m_pxHorseImage	= &pxImgHandler->m_xImageList[m_bHorseImgIdx];
		else										m_pxHorseImage	= NULL;

		return TRUE;
	}
	return FALSE;
}



BOOL CHero::DrawActor(CMapHandler* pxMap, BOOL bFocused, BOOL bShadowAblended, BOOL bUseScrnPos, BOOL bDrawShadow)
{
	// 坐标处理。
	if ( bUseScrnPos )
	{
		m_shScrnPosX = (m_wPosX - pxMap->m_shStartViewTileX) * _CELL_WIDTH + _VIEW_CELL_X_START - pxMap->m_shViewOffsetX + m_shShiftPixelX;
		m_shScrnPosY = (m_wPosY - pxMap->m_shStartViewTileY) * _CELL_HEIGHT+ _VIEW_CELL_Y_START - pxMap->m_shViewOffsetY + m_shShiftPixelY;
	}

	BYTE	bShadowType;
	SHORT	shShadowPX;
	SHORT	shShadowPY;

	SHORT	shPX, shPY;

	//---------------------------------------------------------------------------------------------------------------//
	// 各图像的设置及坐标区域检查。
	//---------------------------------------------------------------------------------------------------------------//
	// 马.
	if ( m_stFeatureEx.bHorse != _HORSE_NONE && m_pxHorseImage != NULL )
	{
		if ( m_bCurrMtn >= _START_HORSE_MTN )
		{		
			m_dwCurrHorseFrame = (m_stFeatureEx.bHorse*_MAX_HORSE_FRAME-_MAX_HORSE_FRAME)+(m_dwCurrFrame-(_MAX_HERO_FRAME*m_stFeature.bDress)-_START_HORSE_FRAME);
			if ( !m_pxHorseImage->NewSetIndex(m_dwCurrHorseFrame) )
				return FALSE;
			shPX = m_pxHorseImage->m_lpstNewCurrWilImageInfo->shPX;
			shPY = m_pxHorseImage->m_lpstNewCurrWilImageInfo->shPY;
			SetRect(&m_rcHorse, m_shScrnPosX + shPX, m_shScrnPosY + shPY,
					m_shScrnPosX + shPX + m_pxHorseImage->m_lpstNewCurrWilImageInfo->shWidth, 
					m_shScrnPosX + shPY + m_pxHorseImage->m_lpstNewCurrWilImageInfo->shHeight);
		}
	} 

	// 武器。
	if ( m_stFeature.bWeapon != _WEAPON_NONE && m_pxWeaponImage != NULL )
	{
		if ( m_bCurrMtn < _MAX_WEAPON_MTN )
		{	
			m_dwCurrWeaponFrame = (((m_stFeature.bWeapon-1)%10)*_MAX_WEAPON_FRAME)+(m_dwCurrFrame-(_MAX_HERO_FRAME*m_stFeature.bDress));
			if ( !m_pxWeaponImage->NewSetIndex(m_dwCurrWeaponFrame) )
				return FALSE;

			shPX = m_pxWeaponImage->m_lpstNewCurrWilImageInfo->shPX;
			shPY = m_pxWeaponImage->m_lpstNewCurrWilImageInfo->shPY;
			SetRect(&m_rcWeapon, m_shScrnPosX + shPX, m_shScrnPosY + shPY, 
					m_shScrnPosX + shPX + m_pxWeaponImage->m_lpstNewCurrWilImageInfo->shWidth, 
					m_shScrnPosY + shPY + m_pxWeaponImage->m_lpstNewCurrWilImageInfo->shHeight);
		}
	}

	// 角色.
	if ( m_pxActorImage )
	{
		if ( !m_pxActorImage->NewSetIndex(m_dwCurrFrame) )
			return FALSE;
		shPX = m_pxActorImage->m_lpstNewCurrWilImageInfo->shPX;
		shPY = m_pxActorImage->m_lpstNewCurrWilImageInfo->shPY;
		bShadowType = m_pxActorImage->m_lpstNewCurrWilImageInfo->bShadow;
		shShadowPX  = m_pxActorImage->m_lpstNewCurrWilImageInfo->shShadowPX;
		shShadowPY  = m_pxActorImage->m_lpstNewCurrWilImageInfo->shShadowPY;

		SetRect(&m_rcActor, m_shScrnPosX + shPX, m_shScrnPosY + shPY, 
				m_shScrnPosX + shPX + m_pxActorImage->m_lpstNewCurrWilImageInfo->shWidth, 
				m_shScrnPosY + shPY + m_pxActorImage->m_lpstNewCurrWilImageInfo->shHeight);

		if ( ( m_rcActor.right - m_rcActor.left ) > _CELL_WIDTH + _TARGETRGN_GAPX )
		{
			m_rcTargetRgn.left  = m_rcActor.left + ( (m_rcActor.right - m_rcActor.left) - (_CELL_WIDTH + _TARGETRGN_GAPX) )/2;
			m_rcTargetRgn.right = m_rcActor.left + ( (m_rcActor.right - m_rcActor.left) + (_CELL_WIDTH + _TARGETRGN_GAPX) )/2;
		}
		else
		{
			m_rcTargetRgn.left  = m_rcActor.left;
			m_rcTargetRgn.right = m_rcActor.right;
		}

		if ( ( m_rcActor.bottom - m_rcActor.top ) > _CELL_HEIGHT + _TARGETRGN_GAPY )
		{
			m_rcTargetRgn.top    = m_rcActor.top + ( (m_rcActor.bottom - m_rcActor.top) - (_CELL_HEIGHT + _TARGETRGN_GAPY) )/2;
			m_rcTargetRgn.bottom = m_rcActor.top + ( (m_rcActor.bottom - m_rcActor.top) + (_CELL_HEIGHT + _TARGETRGN_GAPY) )/2;
		}
		else
		{
			m_rcTargetRgn.top    = m_rcActor.top;
			m_rcTargetRgn.bottom = m_rcActor.bottom;
		}
	}

	// 头部.
	if ( m_stFeature.bHair != _HAIR_NONE && m_pxHairImage != NULL )
	{
		m_dwCurrHairFrame = (m_stFeature.bHair*_MAX_HERO_FRAME-_MAX_HERO_FRAME)+(m_dwCurrFrame-(_MAX_HERO_FRAME*m_stFeature.bDress));
		if ( !m_pxHairImage->NewSetIndex(m_dwCurrHairFrame) )
			return FALSE;
		shPX = m_pxHairImage->m_lpstNewCurrWilImageInfo->shPX;
		shPY = m_pxHairImage->m_lpstNewCurrWilImageInfo->shPY;
		SetRect(&m_rcHair, m_shScrnPosX + shPX, m_shScrnPosY + shPY, 
					m_shScrnPosX + shPX + m_pxHairImage->m_lpstNewCurrWilImageInfo->shWidth, 
					m_shScrnPosY + shPY + m_pxHairImage->m_lpstNewCurrWilImageInfo->shHeight);
	}

	//---------------------------------------------------------------------------------------------------------------//
	// 绘制影子.
	//---------------------------------------------------------------------------------------------------------------//
	BYTE bOpaRate = 70;
	WORD wState   = GetCharState();

	// 武器。
	if ( m_stFeature.bWeapon != _WEAPON_NONE && m_pxWeaponImage != NULL )
	{
		if ( m_bCurrMtn < _MAX_WEAPON_MTN )
		{
			if ( bDrawShadow )
			{
				if ( !m_bIsDead )
				{
					g_xMainWnd.DrawWithShadowABlend(
									m_rcWeapon.left, m_rcWeapon.top,
									m_pxWeaponImage->m_lpstNewCurrWilImageInfo->shWidth,
									m_pxWeaponImage->m_lpstNewCurrWilImageInfo->shHeight,
									m_pxWeaponImage->m_lpstNewCurrWilImageInfo->shPX,
									m_pxWeaponImage->m_lpstNewCurrWilImageInfo->shPY,
									(WORD*)m_pxWeaponImage->m_pbCurrImage, 						
									_CLIP_WIDTH, _CLIP_HEIGHT, g_xGameProc.m_wShadowClr, bShadowAblended, bOpaRate);
				}
/*				else
				{
					g_xMainWnd.DrawWithShadowABlend(
									m_rcWeapon.left+3, m_rcWeapon.top-2,
									m_pxWeaponImage->m_lpstNewCurrWilImageInfo->shWidth,
									m_pxWeaponImage->m_lpstNewCurrWilImageInfo->shHeight,
									m_pxWeaponImage->m_lpstNewCurrWilImageInfo->shPX,
									m_pxWeaponImage->m_lpstNewCurrWilImageInfo->shPY,
									(WORD*)m_pxWeaponImage->m_pbCurrImage, 						
									_CLIP_WIDTH, _CLIP_HEIGHT, g_xGameProc.m_wShadowClr, bShadowAblended, bOpaRate);
				}
*/			}
		}
	}

	// 马.
	if ( m_stFeatureEx.bHorse != _HORSE_NONE && m_pxHorseImage != NULL )
	{
		if ( m_bCurrMtn >= _START_HORSE_MTN )
		{	
			if ( bDrawShadow )
			{
				if ( !m_bIsDead )
				{
					g_xMainWnd.DrawWithShadowABlend(
									m_rcHorse.left, m_rcHorse.top,
									m_pxHorseImage->m_lpstNewCurrWilImageInfo->shWidth,
									m_pxHorseImage->m_lpstNewCurrWilImageInfo->shHeight,
									m_pxHorseImage->m_lpstNewCurrWilImageInfo->shPX,
									m_pxHorseImage->m_lpstNewCurrWilImageInfo->shPY,
									(WORD*)m_pxHorseImage->m_pbCurrImage, 						
									_CLIP_WIDTH, _CLIP_HEIGHT, g_xGameProc.m_wShadowClr, bShadowAblended, bOpaRate);
				}
				else
				{
					g_xMainWnd.DrawWithShadowABlend(
									m_rcHorse.left+3, m_rcHorse.top+2,
									m_pxHorseImage->m_lpstNewCurrWilImageInfo->shWidth,
									m_pxHorseImage->m_lpstNewCurrWilImageInfo->shHeight,
									m_pxHorseImage->m_lpstNewCurrWilImageInfo->shPX,
									m_pxHorseImage->m_lpstNewCurrWilImageInfo->shPY,
									(WORD*)m_pxHorseImage->m_pbCurrImage, 						
									_CLIP_WIDTH, _CLIP_HEIGHT, g_xGameProc.m_wShadowClr, bShadowAblended, bOpaRate);
				}
			}
		}
	}

	// 角色.
	if ( m_pxActorImage )
	{
		INT nStartX1 = m_shScrnPosX + shShadowPX;
		INT nStartY1 = m_shScrnPosY + shShadowPY;

		if ( bDrawShadow )
		{
			if ( !m_bIsDead )
			{
				g_xMainWnd.DrawWithShadowABlend(
									nStartX1, nStartY1,
									m_pxActorImage->m_lpstNewCurrWilImageInfo->shWidth,
									m_pxActorImage->m_lpstNewCurrWilImageInfo->shHeight,
									(WORD*)m_pxActorImage->m_pbCurrImage,
									_CLIP_WIDTH, _CLIP_HEIGHT, g_xGameProc.m_wShadowClr, bShadowAblended, bShadowType, bOpaRate);
			}
			else
			{
				g_xMainWnd.DrawWithShadowABlend(
									m_rcActor.left+3, m_rcActor.top+2,
									m_pxActorImage->m_lpstNewCurrWilImageInfo->shWidth,
									m_pxActorImage->m_lpstNewCurrWilImageInfo->shHeight,
									(WORD*)m_pxActorImage->m_pbCurrImage,
									_CLIP_WIDTH, _CLIP_HEIGHT, g_xGameProc.m_wShadowClr, bShadowAblended, 50, bOpaRate);
			}
		}
	}

	// 头部不绘制阴影。
	if ( m_wABlendDelay )		
	{
		bShadowAblended = TRUE;

		if ( m_wABlendDelay )	
		{
			if ( !m_bABlendRev )
				bOpaRate = 100 - ( (m_wABlendCurrDelay * 100) / m_wABlendDelay );
			else
				bOpaRate = ( (m_wABlendCurrDelay * 100) / m_wABlendDelay );
		}

		if ( bFocused )								
		{
			if ( !m_bABlendRev )
				bOpaRate -= 20;
			else
				bOpaRate += 20;
		}

		if ( bOpaRate < 0 && bOpaRate > 100 )		bOpaRate  = 0;
	}
	else if ( wState==_STATE_ABLEND )			bShadowAblended = TRUE;

	//---------------------------------------------------------------------------------------------------------------//
	// 绘制图像.
	//---------------------------------------------------------------------------------------------------------------//
	// 马.
	if ( m_stFeatureEx.bHorse != _HORSE_NONE && m_pxHorseImage != NULL )
	{
		if ( m_bCurrMtn >= _START_HORSE_MTN )
		{			
			DrawWithEffected(m_rcHorse.left, m_rcHorse.top, m_pxHorseImage->m_lpstNewCurrWilImageInfo->shWidth, m_pxHorseImage->m_lpstNewCurrWilImageInfo->shHeight, (WORD*)m_pxHorseImage->m_pbCurrImage, 
				             0XFFFF, 0XFFFF, bFocused, bOpaRate, wState);
		}
	}

	// 需要先绘制武器的方向.
	if ( m_stFeature.bWeapon != _WEAPON_NONE && m_pxWeaponImage != NULL && g_xSpriteInfo.m_bWOrder[(m_dwCurrFrame-(_MAX_HERO_FRAME*m_stFeature.bDress))] )
	{
		if ( m_bCurrMtn < _MAX_WEAPON_MTN )
		{			
			DrawWithEffected(m_rcWeapon.left, m_rcWeapon.top, m_pxWeaponImage->m_lpstNewCurrWilImageInfo->shWidth, m_pxWeaponImage->m_lpstNewCurrWilImageInfo->shHeight, (WORD*)m_pxWeaponImage->m_pbCurrImage,
							 0XFFFF, 0XFFFF, bFocused, bOpaRate, wState);
/* 			g_xMainWnd.DrawWithImageForCompClipRgn(
							m_rcWeapon.left, m_rcWeapon.top,
							m_pxWeaponImage->m_lpstNewCurrWilImageInfo->shWidth,
							m_pxWeaponImage->m_lpstNewCurrWilImageInfo->shHeight,
							(WORD*)m_pxWeaponImage->m_pbCurrImage, _CLIP_WIDTH, _CLIP_HEIGHT, bFocused);
*/		}
	}

	// 角色.
	if ( m_pxActorImage != NULL )
	{
		DrawWithEffected(m_rcActor.left, m_rcActor.top, m_pxActorImage->m_lpstNewCurrWilImageInfo->shWidth, m_pxActorImage->m_lpstNewCurrWilImageInfo->shHeight, (WORD*)m_pxActorImage->m_pbCurrImage,
				         m_stFeatureEx.wDressColor, m_stFeatureEx.wHairColor, bFocused, bOpaRate, wState);
	}

	// 头部.
 	if ( m_stFeature.bHair != _HAIR_NONE && m_pxHairImage != NULL )
	{
		DrawWithEffected(m_rcHair.left, m_rcHair.top, m_pxHairImage->m_lpstNewCurrWilImageInfo->shWidth, m_pxHairImage->m_lpstNewCurrWilImageInfo->shHeight, (WORD*)m_pxHairImage->m_pbCurrImage, 
				         m_stFeatureEx.wDressColor, m_stFeatureEx.wHairColor, bFocused, bOpaRate, wState);
	}			

	// 之后需要绘制武器的方向.
	if ( m_stFeature.bWeapon != _WEAPON_NONE && m_pxWeaponImage != NULL && !g_xSpriteInfo.m_bWOrder[(m_dwCurrFrame-(_MAX_HERO_FRAME*m_stFeature.bDress))] ) 
	{
		if ( m_bCurrMtn < _MAX_WEAPON_MTN )
		{			
 			DrawWithEffected(m_rcWeapon.left, m_rcWeapon.top, m_pxWeaponImage->m_lpstNewCurrWilImageInfo->shWidth, m_pxWeaponImage->m_lpstNewCurrWilImageInfo->shHeight, (WORD*)m_pxWeaponImage->m_pbCurrImage,
							 0XFFFF, 0XFFFF, bFocused, /*bOpaRate*/20, wState);
/*			g_xMainWnd.DrawWithImageForCompClipRgn(
						m_rcWeapon.left, m_rcWeapon.top,
						m_pxWeaponImage->m_lpstNewCurrWilImageInfo->shWidth,
						m_pxWeaponImage->m_lpstNewCurrWilImageInfo->shHeight,
						(WORD*)m_pxWeaponImage->m_pbCurrImage, _CLIP_WIDTH, _CLIP_HEIGHT, bFocused);
*/		}
	}

	DrawEffect();
	ShowShield();

    // Scene lights are collected after all actors, independently of shadow rendering.
	return TRUE;
}

























/******************************************************************************************************************

	CMyHero Class

*******************************************************************************************************************/

CMyHero::CMyHero()
{
	InitActor();
}


CMyHero::~CMyHero()
{
	DestroyActor();
}


VOID CMyHero::InitActor()
{
    m_spellTiming={};m_actionClock={};m_pendingAction.reset();m_spellAwaitingReply=false;m_spellReplyElapsed=0;m_lastSpellReplyId=0;
	CHero::InitActor();

	m_wOldPosX			= 0;
	m_wOldPosY			= 0;
	m_bOldDir			= 0;
	m_dwWarModeTime		= 0;
	m_dwMotionLockTime	= 0;

	m_wMagicDelayTime	= 0;
	m_wMagicPKDelayTime	= 0;

	m_bJob				= 0;
	m_nGlod				= 0;

	m_dwLastHitTime		= 0;
	m_dwLastSpellTime	= timeGetTime();
	m_dwLastMagicTime	= timeGetTime();
	m_dwLastStruckTime  = timeGetTime();
	m_dwLastPKStruckTime= timeGetTime();
	m_dwLastRushTime	= timeGetTime();
	m_dwLastFireHitTime = timeGetTime();

	m_bCanRun			= FALSE;
	m_bInputLock		= FALSE;
	m_bMotionLock		= FALSE;
	m_bWarMode			= FALSE;

	m_pxMap				= NULL;

	m_bAttackMode		= _MT_ONEHSWING;

	ZeroMemory(&m_stAbility, sizeof(ACTORABILITY));
	ZeroMemory(&m_stSubAbility, sizeof(ACTORSUBABILITY));
}

VOID CMyHero::DestroyActor()
{
	INT			nCnt;
	SHORT		shLeftMsgCnt;
	LPPACKETMSG	lpPacketMsg;

	CHero::DestroyActor();

	lpPacketMsg  = NULL;
	shLeftMsgCnt = m_xPriorPacketQueue.GetCount();

	// 清除已堆积的数据包。
	if ( shLeftMsgCnt > 0 )
	{
		for ( nCnt = 0; nCnt < shLeftMsgCnt; nCnt++ )
		{
			lpPacketMsg = (LPPACKETMSG)m_xPriorPacketQueue.PopQ();
			if ( lpPacketMsg )
			{
				SAFE_DELETE(lpPacketMsg);
			}
		}
	}

	InitActor();
}

BOOL CMyHero::Create(CImageHandler* pxImgHandler, BYTE bMtn, BYTE bDir, WORD wPosX, WORD wPosY, 
				   FEATURE* pstFeature, FEATUREEX* pstFeatureEx)
{
	ZeroMemory(&m_stAbility, sizeof(ACTORABILITY));
	ZeroMemory(&m_stSubAbility, sizeof(ACTORSUBABILITY));
	
	if ( !(CHero::Create(pxImgHandler, bMtn, bDir, wPosX, wPosY, pstFeature, pstFeatureEx)) )
		return FALSE;

	return TRUE;
}



VOID CMyHero::SetMapHandler(CMapHandler* pxMap)
{
	m_pxMap = pxMap;
}


VOID CMyHero::OnHealthSpellChanged(LPPACKETMSG lpPacketMsg)
{
    HealthDamage(m_stAbility.wHP,lpPacketMsg->stDefMsg.wParam);
	m_stAbility.wHP	   = lpPacketMsg->stDefMsg.wParam;
	m_stAbility.wMaxHP = lpPacketMsg->stDefMsg.wSeries;
	m_stAbility.wMP    = lpPacketMsg->stDefMsg.wTag;
    m_wHP=m_stAbility.wHP;m_wMP=m_stAbility.wMP;m_wMAXHP=m_stAbility.wMaxHP;

	if ( m_stAbility.wHP <= 0 )			m_stAbility.wHP = m_bHPPercent = 0;
	else
	{
		FLOAT wHPRate= (FLOAT)((FLOAT)m_stAbility.wHP/(FLOAT)m_stAbility.wMaxHP);
		m_bHPPercent = wHPRate * 100;
	}
}


BOOL CMyHero::UpdatePacketState()
{
    if(m_pendingAction) return FALSE;
	LPPACKETMSG	lpPacketMsg = NULL;

	SHORT shLeftMsgCnt = m_xPriorPacketQueue.GetCount();

	if ( shLeftMsgCnt > 0 )
	{
		lpPacketMsg = (LPPACKETMSG)m_xPriorPacketQueue.PopQ();
		if ( lpPacketMsg )
		{
			if ( lpPacketMsg->stDefMsg.wIdent == SM_NOWDEATH || lpPacketMsg->stDefMsg.wIdent == SM_DEATH )
			{
				OnDeath(lpPacketMsg);
				SAFE_DELETE(lpPacketMsg);
				return TRUE;
			}
		}
	}

	if ( m_bCurrMtn == _MT_STAND || m_bCurrMtn == _MT_ATTACKMODE || m_bCurrMtn == _MT_HORSESTAND )
	{
		LPPACKETMSG	lpPacketMsg;

		SHORT shLeftMsgCnt = m_xPacketQueue.GetCount();

		if ( shLeftMsgCnt > 0 )
		{
			lpPacketMsg = (LPPACKETMSG)m_xPacketQueue.PopQ();

			if ( shLeftMsgCnt >= 3 )		m_bMsgHurryCheck = TRUE;
			else							m_bMsgHurryCheck = FALSE;
			
			if ( lpPacketMsg )
			{
				switch ( lpPacketMsg->stDefMsg.wIdent )
				{
					case 1309:
					case SM_STRUCK:
					{
						m_bInputLock = TRUE;
						m_stAbility.wHP	   = lpPacketMsg->stDefMsg.wParam;
						m_stAbility.wMaxHP = lpPacketMsg->stDefMsg.wTag;
						OnStruck(lpPacketMsg);

						if ( m_dwNameClr == RGB(255, 0, 0) )
						{
							m_dwLastPKStruckTime = timeGetTime();
						}
						m_dwLastStruckTime = timeGetTime();
                        g_xGameProc.m_exitGuard.ticks[ExitFlow::Struck]=GetTickCount();
						break;
					}
                    case mir3::ForcedMotion::Bump:
					case SM_RUSH:
					{
						m_bInputLock = TRUE;
						m_wOldPosX	 = m_wPosX;
						m_wOldPosY	 = m_wPosY;
						m_bOldDir	 = m_bCurrDir;
						OnRush(lpPacketMsg);
						if ( !CheckMyPostion() )			AdjustMyPostion();
						AdvanceForcedMotion(0);
						break;
					}
					case SM_BACKSTEP:
					{
						m_bInputLock = TRUE;
						m_wOldPosX = m_wPosX;
						m_wOldPosY = m_wPosY;
						m_bOldDir  = m_bCurrDir;
						OnBackStep(lpPacketMsg);

						if ( !CheckMyPostion() )			AdjustMyPostion();
						AdvanceForcedMotion(0);
						break;
					}
					case SM_FEATURECHANGED:		{	OnFeatureChanged(lpPacketMsg);		break;	}
					case SM_OPENHEALTH:			{	OnOpenHealth(lpPacketMsg);			break;	}
					case SM_CLOSEHEALTH:		{	OnCloseHealth(lpPacketMsg);			break;	}
					case SM_CHANGELIGHT:		{	OnChangeLight(lpPacketMsg);			break;	}
					case SM_USERNAME:			{	OnUserName(lpPacketMsg);			break;	}
					case SM_CHANGENAMECOLOR:	{	OnChangeNameClr(lpPacketMsg);		break;	}
					case SM_CHARSTATUSCHANGE:	{	OnCharStatusChanged(lpPacketMsg);	break;	}
					case SM_MAGICFIRE:			{	OnMagicFire(lpPacketMsg);			break;	}
                    case SpellWire::MultipleReply: {OnMagicList(lpPacketMsg);break;}
					case SM_HEALTHSPELLCHANGED:	{	OnHealthSpellChanged(lpPacketMsg);	break;	}
					default:
					{										
						break;	
					}
				}				
			}
			SAFE_DELETE(lpPacketMsg);

			return TRUE;
		}
	}
	return FALSE;
}



void CMyHero::ConfirmAction()
{
    m_spellAwaitingReply=false;m_spellReplyElapsed=0;m_bMotionLock=FALSE;
    if(!m_bIsDead && (m_bCurrMtn==_MT_STAND || m_bCurrMtn==_MT_HORSESTAND || m_bCurrMtn==_MT_ATTACKMODE))
        m_bInputLock=FALSE;
}

BOOL CMyHero::SetOldPosition()
{
    if(m_bIsDead || m_bCurrMtn==_MT_DIE) return FALSE;
    m_pendingAction.reset();m_forcedMotion.reset();
    m_spellAwaitingReply=false;m_spellReplyElapsed=0;
    m_spellLightActive=false;m_bUseEffect=m_bUseSwordEffect=FALSE;
    m_dwCurrEffectFrame=m_dwFstEffectFrame=m_dwEndEffectFrame=0;
    m_bEffectFrame=0;m_dwMotionLockTime=0;
	m_wPosX	   = m_wOldPosX;
	m_wPosY	   = m_wOldPosY;
	m_bCurrDir = m_bOldDir;

	if ( m_stFeatureEx.bHorse == _HORSE_NONE )
	{
		if ( SetMotionFrame(_MT_STAND, m_bCurrDir) )
		{
			AdjustMyPostion();
			m_bMotionLock = m_bInputLock = FALSE;
			return TRUE;
		}
	}
	else
	{
		if ( SetMotionFrame(_MT_HORSESTAND, m_bCurrDir) )
		{
			AdjustMyPostion();
			m_bMotionLock = m_bInputLock = FALSE;
			return TRUE;
		}
	}
	return FALSE;
}



BOOL CMyHero::CheckMyPostion()
{
	if ( m_wPosX != m_pxMap->m_shStartViewTileX + _GAPX_TILE_CHAR_MAP ||
		 m_wPosY != m_pxMap->m_shStartViewTileY + _GAPY_TILE_CHAR_MAP )
	{
		return FALSE;
	}
	return TRUE;
}



VOID CMyHero::AdjustMyPostion()
{
	m_pxMap->SetStartViewTile(m_wPosX-_GAPX_TILE_CHAR_MAP, m_wPosY-_GAPY_TILE_CHAR_MAP);
	m_pxMap->LoadNewMapBuffer();
}


VOID CMyHero::SetMotionState(BYTE motion,BYTE direction,INT target,BOOL targetDead,LPPOINT position,SHORT skill)
{
    if(m_pendingAction) return;
    const bool movement=motion==_MT_WALK || motion==_MT_RUN || motion==_MT_HORSEWALK || motion==_MT_HORSERUN;
    const bool attack=motion==_MT_ONEHSWING;
    const bool turn=(motion==_MT_STAND || motion==_MT_HORSESTAND) && direction!=m_bCurrDir;
    if(g_xGameProc.InputDelayed() && (movement || attack || turn || motion==_MT_CUT)) return;
    if(!m_bIsMon && (movement || attack)) {
        if(!position || m_bIsDead || m_spellAwaitingReply || (movement && m_xPacketQueue.GetCount())) return;
        if(attack && !CanNextHit()) return;
        m_pendingAction=PendingLocalAction{motion,direction,target,targetDead,*position,{m_wPosX,m_wPosY},skill,{},0};
        AdvancePendingAction();return;
    }
    ExecuteMotionState(motion,direction,target,targetDead,position,skill);
}
DWORD CMyHero::AttackInterval() const
{
    const int discount=(std::min)(800,(std::min)(370,int(m_stAbility.bLevel)*14)+60*int(m_shHitSpeed));
    return DWORD(1400-discount+(m_stAbility.bHandWeight>m_stAbility.bMaxHandWeight?1500:0));
}
void CMyHero::AdvancePendingAction()
{
    if(!m_pendingAction) return;
    auto& pending=*m_pendingAction;
    if(!m_pxMap || !m_pxMap->m_pstCellInfo || m_wPosX!=pending.origin.x || m_wPosY!=pending.origin.y ||
        m_bIsDead || (m_stAbility.wMaxHP && !m_stAbility.wHP)) {m_pendingAction.reset();return;}
    const DWORD now=GetTickCount();
    const bool movement=pending.motion!=_MT_ONEHSWING;
    if(!pending.waitingSince) {
        if(movement) {
            if(!m_actionClock.admit_move(now)) return;
            if(!m_spellTiming.movement_ready(now)) {m_pendingAction.reset();return;}
        }
        pending.waitingSince=now;pending.wait=m_actionClock.shared_wait(now,AttackInterval());
    }
    if(mir3::LocalActionClock::remaining(now,*pending.waitingSince,pending.wait)) return;
    auto action=pending;m_pendingAction.reset();
    if(movement) m_actionClock.moved=now;
    else m_actionClock.attacked=now;
    ExecuteMotionState(action.motion,action.direction,action.target,action.targetDead,&action.destination,action.skill);
}
VOID CMyHero::ExecuteMotionState(BYTE bMtn,BYTE bDir,INT nMouseTargetID,BOOL bIsDead,LPPOINT lpptPos,SHORT shSkill)
{
	if ( !m_bIsMon )
	{
		switch ( bMtn )
		{
			case _MT_WALK:
			case _MT_HORSEWALK:
			{
				if ( lpptPos )
				{
					POINT ptNext;

					for ( INT nCnt = 0; nCnt < _MAX_DIRECTION; nCnt++ )
					{
						m_pxMap->GetNextTileCanMove(lpptPos->x, lpptPos->y, nCnt, 1, &ptNext);
						if ( !m_pxMap->IsDoorOpen(ptNext.x, ptNext.y) )
						{
							g_xClientSocket.SendOpenDoor(ptNext.x, ptNext.y, m_pxMap->GetDoor(ptNext.x, ptNext.y));
							break;
						}
					}

					g_xClientSocket.SendActMsg(CM_WALK, lpptPos->x, lpptPos->y, bDir);
					m_bMotionLock = m_bInputLock = TRUE;
					m_wOldPosX = m_wPosX;
					m_wOldPosY = m_wPosY;
					m_bOldDir  = m_bCurrDir;

					SetMotionFrame(bMtn, bDir);
					m_bMoveSpeed = _SPEED_WALK;

					m_pxMap->ScrollMap(m_bMoveDir, m_dwCurrFrame-m_dwFstFrame, m_bMoveSpeed);
				}
				break;
			}
			case _MT_RUN:
			case _MT_HORSERUN:
			{
				if ( lpptPos )
				{
					POINT ptNext;
					POINT ptStart;

					if ( bMtn == _MT_RUN )
						m_bMoveSpeed = _SPEED_RUN;
					else
						m_bMoveSpeed = _SPEED_HORSERUN;
					
					m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, bDir, 1, &ptStart);
					for ( INT nCnt = 0; nCnt < _MAX_DIRECTION; nCnt++ )
					{
						for ( INT nSpeedCnt = 0; nSpeedCnt < m_bMoveSpeed; nSpeedCnt++ )
						{
							m_pxMap->GetNextTileCanMove(ptStart.x, ptStart.y, nCnt, nSpeedCnt+1, &ptNext);
							if ( !m_pxMap->IsDoorOpen(ptNext.x, ptNext.y) )
							{
								g_xClientSocket.SendOpenDoor(ptNext.x, ptNext.y, m_pxMap->GetDoor(ptNext.x, ptNext.y));
								break;
							}
						}
					}

					g_xClientSocket.SendActMsg(CM_RUN, lpptPos->x, lpptPos->y, bDir);
					m_bMotionLock = m_bInputLock = TRUE;
					m_wOldPosX = m_wPosX;
					m_wOldPosY = m_wPosY;
					m_bOldDir  = m_bCurrDir;

					SetMotionFrame(bMtn, bDir);

					m_pxMap->ScrollMap(m_bMoveDir, m_dwCurrFrame-m_dwFstFrame, m_bMoveSpeed);			
				}
				break;
			}
			case _MT_ONEHSWING:
			{
				if ( m_bAttackMode == _MT_ONEHSWING || m_bAttackMode == _MT_ONEVSWING )
				{
					BYTE bAttackStyle = rand()%2;
					if ( bAttackStyle )			m_bAttackMode = _MT_ONEVSWING;
					else						m_bAttackMode = _MT_ONEHSWING;

					m_bAttackMode = _MT_ONEVSWING;
				}			

				if ( lpptPos )
				{
					m_bMotionLock	= m_bInputLock = TRUE;
					m_wOldPosX		= m_wPosX;
					m_wOldPosY		= m_wPosY;
					m_bOldDir		= m_bCurrDir;
					m_bWarMode		= TRUE;
					m_dwWarModeTime	= 0;

					if ( m_bUseErgum && g_xGameProc.TargetInLongAttack(bDir) )
					{
						SetMotionFrame(_MT_ONEVSWING, bDir);
						LoadEffect(&g_xGameProc.m_xImage, _SKILL_ERGUM, bDir);
						m_bUseSwordEffect = TRUE;
						g_xClientSocket.SendActMsg(CM_LONGHIT, lpptPos->x, lpptPos->y, bDir);
					}
					else if ( m_bFireHitCnt == 1 && m_stAbility.wMP > 7 )
					{
						SetMotionFrame(_MT_ONEVSWING, bDir);
						LoadEffect(&g_xGameProc.m_xImage, _SKILL_FIRESWORD, bDir);
						m_bUseSwordEffect = TRUE;
						g_xClientSocket.SendActMsg(CM_FIREHIT, lpptPos->x, lpptPos->y, bDir);
						m_bFireHitCnt = 2;
					}
					else if ( m_bYedoCnt == 1 )
					{
						SetMotionFrame(_MT_ONEVSWING, bDir);
						LoadEffect(&g_xGameProc.m_xImage, _SKILL_YEDO, bDir);
						m_bUseSwordEffect = TRUE;
						g_xClientSocket.SendActMsg(CM_POWERHIT, lpptPos->x, lpptPos->y, bDir);
						m_bYedoCnt = 2;
					}
					else if ( m_bUseBanwol && m_stAbility.wMP > 3 )
					{
						SetMotionFrame(_MT_ONEHSWING, bDir);
						LoadEffect(&g_xGameProc.m_xImage, _SKILL_BANWOL, bDir);
						m_bUseSwordEffect = TRUE;
						g_xClientSocket.SendActMsg(CM_WIDEHIT, lpptPos->x, lpptPos->y, bDir);
					}
					else
					{
						SetMotionFrame(m_bAttackMode, bDir);
						if ( m_bAttackMode == _MT_WHEELWIND )
						{
							LoadEffect(&g_xGameProc.m_xImage, _SKILL_JUMPSHOT, bDir);
							m_bUseSwordEffect = TRUE;
						}
						else if ( m_bAttackMode == _MT_RANDSWING )
						{
							LoadEffect(&g_xGameProc.m_xImage, _SKILL_RANDSWING, bDir);
							m_bUseSwordEffect = TRUE;
						}
						WORD wAttackStyle;
						if ( m_bAttackMode != _MT_ONEVSWING && m_bAttackMode != _MT_ONEHSWING )
							wAttackStyle = _MT_ONEVSWING;
						else
						{
							wAttackStyle = m_bAttackMode;
						}
						g_xClientSocket.SendHitMsg(CM_HIT, lpptPos->x, lpptPos->y, bDir, wAttackStyle);
					}
				}
				break;
			}
			case _MT_MOODEPO:
            case _MT_BACKDROPKICK:
			{
				if ( lpptPos )		
				{
					m_wOldPosX=m_wPosX;m_wOldPosY=m_wPosY;m_bOldDir=m_bCurrDir;
                    BeginSpellReply();
                    g_xClientSocket.SendSpellMsg(shSkill,lpptPos->x,lpptPos->y,0);
                    if(bMtn==_MT_BACKDROPKICK) {
                        SetMotionFrame(bMtn,bDir);m_bUseEffect=FALSE;m_spellLightActive=false;
                        constexpr int SpecialSpellSound=142;
                        g_xSound.PlayActorWav(m_wPosX,m_wPosY,g_xGameProc.m_xMyHero.m_wPosX,g_xGameProc.m_xMyHero.m_wPosY,SpecialSpellSound);
                    }
					// 记录无怠步的时间值.
					m_dwLastRushTime = timeGetTime();
					m_bMotionLock = m_bInputLock = TRUE;
					m_bWarMode	= TRUE;
				}

				break;
			}
            case _MT_SPELL2:
            case _MT_SPELL1:
                if(lpptPos) {
                    m_wOldPosX=m_wPosX;m_wOldPosY=m_wPosY;m_bOldDir=m_bCurrDir;
                    BeginSpellReply();
                    g_xClientSocket.SendSpellMsg(shSkill,lpptPos->x,lpptPos->y,nMouseTargetID);
                    m_bMotionLock=m_bInputLock=TRUE;
                    m_dwMotionLockTime=0;
                    BeginSpellAction(WORD(shSkill),bDir,bMtn);
                }
                break;
			case _MT_CUT:
			{
				m_bInputLock = TRUE;
				m_bMotionLock = TRUE;
				m_wOldPosX = m_wPosX;
				m_wOldPosY = m_wPosY;
				m_bOldDir  = m_bCurrDir;
				SetMotionFrame(bMtn, bDir);

				g_xClientSocket.SendActMsg(CM_SITDOWN, m_wPosX, m_wPosY, bDir);
				if ( nMouseTargetID && bIsDead )
				{
					g_xClientSocket.SendButchAnimal(m_wPosX, m_wPosY, bDir, nMouseTargetID);
				}
				break;
			}
			case _MT_HORSESTAND:
			case _MT_STAND:
			{
				if ( bDir != m_bCurrDir )	
				{
					g_xClientSocket.SendActMsg(CM_TRUN, m_wPosX, m_wPosY, bDir);
					m_bMotionLock = TRUE;
					m_bInputLock = TRUE;
					m_wOldPosX = m_wPosX;
					m_wOldPosY = m_wPosY;
					m_bOldDir  = m_bCurrDir;
					SetMotionFrame(bMtn, bDir);
				}
				break;
			}
		}
	}
	else
	{
		switch ( bMtn )
		{
			case _MT_HORSESTAND:
			case _MT_STAND:
			{
				bMtn = _MT_MON_STAND;
				if ( bDir != m_bCurrDir )			g_xClientSocket.SendActMsg(CM_TRUN, m_wPosX, m_wPosY, bDir);
	//			m_bMotionLock = TRUE;
				m_wOldPosX = m_wPosX;
				m_wOldPosY = m_wPosY;
				m_bOldDir  = m_bCurrDir;
				SetMotionFrame(bMtn, bDir);
				break;
			}
			case _MT_WALK:
			case _MT_HORSEWALK:
			{
				bMtn = _MT_MON_WALK;
				g_xClientSocket.SendActMsg(CM_WALK, m_wPosX, m_wPosY, bDir);
				m_bMotionLock = m_bInputLock = TRUE;
				m_wOldPosX = m_wPosX;
				m_wOldPosY = m_wPosY;
				m_bOldDir  = m_bCurrDir;

				SetMotionFrame(bMtn, bDir);
				m_bMoveSpeed = _SPEED_WALK;
				m_pxMap->ScrollMap(m_bMoveDir, m_dwCurrFrame-m_dwFstFrame, m_bMoveSpeed);
				break;
			}
			case _MT_ONEHSWING:
			{
				bMtn = _MT_MON_ATTACK_A;
				m_bMotionLock	= m_bInputLock = TRUE;
				m_wOldPosX		= m_wPosX;
				m_wOldPosY		= m_wPosY;
				m_bOldDir		= m_bCurrDir;
				m_bWarMode		= TRUE;
				m_dwWarModeTime	= 0;

				SetMotionFrame(_MT_MON_ATTACK_A, bDir);
				g_xClientSocket.SendHitMsg(CM_HIT, lpptPos->x, lpptPos->y, bDir, 0);
				break;
			}
		}
	}

	m_dwMotionLockTime	= 0;
}



VOID CMyHero::UpdateMotionState(INT nLoopTime, BOOL bIsMoveTime)
{
    AdvancePendingAction();
	if ( m_bMotionLock ) 
		m_dwMotionLockTime += nLoopTime;

    if(m_spellAwaitingReply) {
        if(std::uint64_t(m_spellReplyElapsed)+DWORD(nLoopTime)>_MOTION_LOCKTIME) ConfirmAction();
        else m_spellReplyElapsed+=DWORD(nLoopTime);
    }
	if ( m_bWarMode ) 
		m_dwWarModeTime += nLoopTime;

	m_wABlendCurrDelay += nLoopTime;
	if ( m_wABlendCurrDelay >= m_wABlendDelay )
	{
		m_wABlendCurrDelay = 0;
		m_wABlendDelay	   = 0;
		m_bABlendRev	   = FALSE;
	}

	if ( m_bCurrMtn == _MT_DIE && m_dwCurrFrame >= m_dwEndFrame-1 )
	{
		m_bIsDead = TRUE;
	}

	if ( m_bIsDead )
	{
		if ( m_bIsMon )
			SetMotionFrame(_MT_MON_DIE, m_bCurrDir);
		else
			SetMotionFrame(_MT_DIE, m_bCurrDir);

		m_dwCurrFrame = m_dwEndFrame - 1;
		m_bInputLock  = TRUE;
		return;
	}


    if(AdvanceForcedMotion(DWORD(nLoopTime))) {UpdatePacketState();return;}
	if ( UpdateMove(bIsMoveTime) )
	{
		UpdatePacketState();
		return;
	}
	else
	{
		if ( !m_bIsMon )
		{
			m_wCurrDelay += nLoopTime;

			m_wShieldCurrDelay += nLoopTime;

			if ( m_wShieldCurrDelay > 150 )  
			{
				m_bShieldCurrFrm++;
				m_wShieldCurrDelay = 0;
				if ( m_bShieldCurrFrm > 2 )
					m_bShieldCurrFrm = 0;
			}

			if ( !AdvanceSpellAnimation(DWORD(nLoopTime)) && m_wCurrDelay > m_wDelay )
			{
				m_wCurrDelay  = 0;

				if ( m_dwCurrFrame < m_dwEndFrame )
				{
					m_dwCurrFrame++;

					PlayActSound();

					if ( (m_bCurrMtn == _MT_SPELL2 || m_bCurrMtn == _MT_SPELL1) )
					{
						m_dwCurrEffectFrame++;
						m_bEffectFrame++;
					}
				}
			}

			UpdatePacketState();

			// 连续帧中要做的事。
			if ( m_dwMotionLockTime > _MOTION_LOCKTIME )
			{
				m_dwMotionLockTime = 0;
				m_bMotionLock	   = FALSE;
//				SetOldPosition();
			}

			if ( m_dwWarModeTime > _WARMODE_TIME )
			{
				m_dwWarModeTime = 0;
				m_bWarMode		= FALSE;
			}


        if ( m_dwCurrFrame >= m_dwEndFrame )
			{
				switch ( m_bCurrMtn )
				{
				case _MT_CUT:
					{
						m_bInputLock  = FALSE;
						m_bMotionLock = FALSE;
						m_dwCurrFrame = m_dwFstFrame;
						if ( m_stFeatureEx.bHorse == _HORSE_NONE )		SetMotionFrame(_MT_STAND, m_bCurrDir);
						else											SetMotionFrame(_MT_HORSESTAND, m_bCurrDir);
					}
					break;
				case _MT_STAND:
				case _MT_HORSESTAND:
					{
						m_bInputLock  = m_spellAwaitingReply;
						m_bMotionLock = m_spellAwaitingReply;
						m_bUseEffect = FALSE;
						m_bUseSwordEffect = FALSE;
						m_dwCurrFrame = m_dwFstFrame;
						break;
					}
				case _MT_ATTACKMODE:
					{
						if ( m_stFeatureEx.bHorse != _HORSE_NONE )
                            SetMotionFrame(_MT_HORSESTAND, m_bCurrDir);
                        else if ( !m_bWarMode )
							SetMotionFrame(_MT_STAND, m_bCurrDir);
						else
						{
							m_bInputLock  = m_spellAwaitingReply;
							m_bMotionLock = m_spellAwaitingReply;
							m_dwCurrFrame = m_dwFstFrame;
						}
						break;
					}
				default:
					{
                        if(m_spellAwaitingReply && (m_bCurrMtn==_MT_SPELL1 || m_bCurrMtn==_MT_SPELL2 || m_bCurrMtn==_MT_BACKDROPKICK)) {
                            m_bUseEffect=FALSE;m_spellLightActive=false;
                            SetMotionFrame(_MT_ATTACKMODE,m_bCurrDir);
                            m_bMotionLock=m_bInputLock=TRUE;
                            break;
                        }
						m_dwCurrEffectFrame = 0;
						m_dwFstEffectFrame = 0;
						m_dwEndEffectFrame = 0;
						m_bEffectFrame = 0;
						m_bEffectFrameCnt = _DEFAULT_SPELLFRAME;
						m_bUseEffect = FALSE;
						m_bUseSwordEffect = FALSE;
						if ( !m_bMotionLock )
						{				
							m_bInputLock = FALSE;

							if ( m_stFeatureEx.bHorse == _HORSE_NONE )
							{
								if ( m_bWarMode )
									SetMotionFrame(_MT_ATTACKMODE, m_bCurrDir);
								else
									SetMotionFrame(_MT_STAND, m_bCurrDir);
							}
							else
							{
								SetMotionFrame(_MT_HORSESTAND, m_bCurrDir);
							}
						}
						else
						{
							m_dwCurrFrame = m_dwEndFrame-1;
						}
					}
					break;
				}
			}

			if ( m_bCurrMtn == _MT_STAND || m_bCurrMtn == _MT_HORSESTAND )
			{
				POINT ptMouse;
				GetCursorPos(&ptMouse);
				ScreenToClient(g_xMainWnd.GetSafehWnd(), &ptMouse);
				ptMouse = mir3::ClientToCanvas(g_xMainWnd.GetSafehWnd(), ptMouse,
				    g_xMainWnd.m_stDisplayInfo.wWidth, g_xMainWnd.m_stDisplayInfo.wHeight);
				m_bCanRun = FALSE;

				if ( HIBYTE(GetKeyState(VK_RBUTTON)) || ( HIBYTE(GetKeyState(VK_LBUTTON)) && HIBYTE(GetKeyState(VK_CONTROL))) )
				{
					OnRButtonDown(ptMouse);
				}
				else if ( HIBYTE(GetKeyState(VK_LBUTTON)) )
				{
					OnLButtonDown(ptMouse);

/*					LPARAM lParam = MAKELPARAM((WORD)ptMouse.x, (WORD)ptMouse.y);
					WPARAM wParam = 0;
					g_xGameProc.OnLButtonDown(wParam, lParam);
*/				}
			}
		}
		else
		{
			m_wCurrDelay += nLoopTime;
			m_wShieldCurrDelay += nLoopTime;
			m_wABlendCurrDelay += nLoopTime;

			if ( m_wShieldCurrDelay > 150 )  
			{
				m_bShieldCurrFrm++;
				m_wShieldCurrDelay = 0;
				if ( m_bShieldCurrFrm > 2 )
					m_bShieldCurrFrm = 0;
			}

			if ( m_wCurrDelay > m_wDelay )
			{
				m_wCurrDelay  = 0;

				if ( m_dwCurrFrame < m_dwEndFrame )
				{
					m_dwCurrFrame++;
				}
			}

			UpdatePacketState();

			// 连续帧中要做的事。
			if ( m_dwMotionLockTime > _MOTION_LOCKTIME )
			{
				m_dwMotionLockTime = 0;
				m_bMotionLock	   = FALSE;
//				SetOldPosition();
			}

			if ( m_dwWarModeTime > _WARMODE_TIME )
			{
				m_dwWarModeTime = 0;
				m_bWarMode		= FALSE;
			}


			if ( m_dwCurrFrame >= m_dwEndFrame )
			{
				switch ( m_bCurrMtn )
				{
				case _MT_MON_STAND:
					{
						m_bInputLock  = FALSE;
						m_bMotionLock = FALSE;
						m_bUseEffect = FALSE;
						m_bUseSwordEffect = FALSE;
						m_dwCurrFrame = m_dwFstFrame;
						break;
					}
				default:
					{
						m_dwCurrEffectFrame = 0;
						m_dwFstEffectFrame = 0;
						m_dwEndEffectFrame = 0;
						m_bEffectFrame = 0;
						m_bEffectFrameCnt = _DEFAULT_SPELLFRAME;
						m_bUseEffect = FALSE;
						m_bUseSwordEffect = FALSE;
						if ( !m_bMotionLock )
						{				
							m_bInputLock = FALSE;
							SetMotionFrame(_MT_MON_STAND, m_bCurrDir);
						}
						else
						{
							m_dwCurrFrame = m_dwEndFrame-1;
						}
					}
					break;
				}
			}

			if ( m_bCurrMtn == _MT_MON_STAND )
			{
				POINT ptMouse;
				GetCursorPos(&ptMouse);
				ScreenToClient(g_xMainWnd.GetSafehWnd(), &ptMouse);
				ptMouse = mir3::ClientToCanvas(g_xMainWnd.GetSafehWnd(), ptMouse,
				    g_xMainWnd.m_stDisplayInfo.wWidth, g_xMainWnd.m_stDisplayInfo.wHeight);
				m_bCanRun = FALSE;

				if ( HIBYTE(GetKeyState(VK_RBUTTON)) || ( HIBYTE(GetKeyState(VK_LBUTTON)) && HIBYTE(GetKeyState(VK_CONTROL))) )
				{
					OnRButtonDown(ptMouse);
				}
				else if ( HIBYTE(GetKeyState(VK_LBUTTON)) )
				{
					OnLButtonDown(ptMouse);
/*					LPARAM lParam = MAKELPARAM((WORD)ptMouse.x, (WORD)ptMouse.y);
					WPARAM wParam = 0;
					g_xGameProc.OnLButtonDown(wParam, lParam);
*/				}
			}
		}
	}
}




BOOL CMyHero::UpdateMove(BOOL bIsMoveTime)
{
	if ( !m_bIsMon )
	{
		if ( m_bCurrMtn == _MT_WALK || m_bCurrMtn == _MT_RUN || m_bCurrMtn == _MT_HORSEWALK || m_bCurrMtn == _MT_HORSERUN || m_bCurrMtn == _MT_MOODEPO || m_bCurrMtn == _MT_PUSHBACK)
		{
			m_wCurrDelay = 0;

			if ( bIsMoveTime )
			{
				if ( m_bCurrMtn == _MT_PUSHBACK )
				{
					m_bBackStepFrame += 2;
					if ( m_bBackStepFrame >= m_bBackStepFrameCnt )
					{
						m_dwCurrFrame++;
						m_bBackStepFrame = m_bBackStepFrameCnt;
					}
				}
				else if ( m_bCurrMtn == _MT_MOODEPO )
				{
					m_dwCurrFrame += 2;
					if ( m_dwCurrFrame >= m_dwEndFrame )
					{
						m_dwCurrFrame = m_dwEndFrame;
					}
				}
				else
				{
					m_dwCurrFrame++;

					PlayMoveSound();
/*					if ( m_bMsgHurryCheck )			
					{
						m_dwCurrFrame++;
					}
*/				}

				if ( m_bCurrMtn == _MT_PUSHBACK )
				{
					if ( m_bBackStepFrame >= m_bBackStepFrameCnt-m_bMoveNextFrmCnt && !m_bIsMoved )
					{
						SetMoved();
						m_bIsMoved = TRUE;
					}
				}
				else
				{
					if ( m_dwCurrFrame >= m_dwEndFrame-m_bMoveNextFrmCnt && !m_bIsMoved )
					{
						SetMoved();
						m_bIsMoved = TRUE;
					}
				}
			}

			// 连续帧中要做的事。
			if ( m_dwCurrFrame >= m_dwEndFrame )
			{
				m_dwCurrFrame = m_dwEndFrame-1;
				m_bCanRun = FALSE;

				switch ( m_bCurrMtn )
				{
				case _MT_MOODEPO:
					{
						if ( m_bInputLock )
						{
							m_bIsMoved = FALSE;
							m_bInputLock = FALSE;
							m_pxMap->SetMovedTileBuffer(m_shShiftTileX, m_shShiftTileY);
							m_bMoveSpeed	= 0;
							m_shShiftPixelX	= 0;
							m_shShiftPixelY	= 0;
							m_shShiftTileX	= 0;
							m_shShiftTileY	= 0;
							// 移动后确认坐标。
							if ( CheckMyPostion() == FALSE )
								AdjustMyPostion();

							if ( m_bWarMode )
								SetMotionFrame(_MT_ATTACKMODE, m_bCurrDir);
							else
								SetMotionFrame(_MT_STAND, m_bCurrDir);
						}
						break;
					}
				case _MT_PUSHBACK:
					{
						if ( m_bInputLock )
						{
							m_bIsMoved = FALSE;
							m_bInputLock = FALSE;
							m_pxMap->SetMovedTileBuffer(m_shShiftTileX, m_shShiftTileY);
							m_bMoveSpeed	= 0;
							m_shShiftPixelX	= 0;
							m_shShiftPixelY	= 0;
							m_shShiftTileX	= 0;
							m_shShiftTileY	= 0;

							m_bBackStepFrame = 0;
							m_bBackStepFrameCnt = 0;
							// 移动后确认坐标。
							if ( CheckMyPostion() == FALSE )
								AdjustMyPostion();

							if ( m_bWarMode )
								SetMotionFrame(_MT_ATTACKMODE, m_bCurrDir);
							else
								SetMotionFrame(_MT_STAND, m_bCurrDir);
						}
						break;
					}
				case _MT_WALK:
				case _MT_HORSEWALK:
					{
						if ( !m_bMotionLock && m_bInputLock )
						{
							m_bIsMoved = FALSE;
							m_bInputLock = FALSE;
							m_pxMap->SetMovedTileBuffer(m_shShiftTileX, m_shShiftTileY);
							m_bMoveSpeed	= 0;
							m_shShiftPixelX	= 0;
							m_shShiftPixelY	= 0;
							m_shShiftTileX	= 0;
							m_shShiftTileY	= 0;

							if ( m_bCurrMtn == _MT_WALK && m_stFeatureEx.bHorse == _HORSE_NONE )
							{
								if ( m_bWarMode )
								{
									SetMotionFrame(_MT_ATTACKMODE, m_bCurrDir);
								}
								else		SetMotionFrame(_MT_STAND, m_bCurrDir);
							}
							else	SetMotionFrame(_MT_HORSESTAND, m_bCurrDir);

							// 移动后确认坐标。
							if ( CheckMyPostion() == FALSE )
								AdjustMyPostion();
							POINT ptMouse;
							GetCursorPos(&ptMouse);
							ScreenToClient(g_xMainWnd.GetSafehWnd(), &ptMouse);
							ptMouse = mir3::ClientToCanvas(g_xMainWnd.GetSafehWnd(), ptMouse,
							    g_xMainWnd.m_stDisplayInfo.wWidth, g_xMainWnd.m_stDisplayInfo.wHeight);
							if ( HIBYTE(GetKeyState(VK_RBUTTON)) || ( HIBYTE(GetKeyState(VK_LBUTTON)) && HIBYTE(GetKeyState(VK_CONTROL))) )
							{
								m_bCanRun = TRUE;
								OnRButtonDown(ptMouse);
							}
							else if ( HIBYTE(GetKeyState(VK_LBUTTON)) )
							{
								OnLButtonDown(ptMouse);
							}
						}
						break;
					}
				case _MT_RUN:
				case _MT_HORSERUN:
					{
						if ( !m_bMotionLock && m_bInputLock )
						{				
							m_bIsMoved = FALSE;
							m_bInputLock = FALSE;
							m_pxMap->SetMovedTileBuffer(m_shShiftTileX, m_shShiftTileX);
							m_bMoveSpeed	= 0;
							m_shShiftPixelX	= 0;
							m_shShiftPixelY	= 0;
							m_shShiftTileX	= 0;
							m_shShiftTileY	= 0;

							if ( m_bCurrMtn == _MT_RUN && m_stFeatureEx.bHorse == _HORSE_NONE )
							{
								if ( m_bWarMode )
								{
									SetMotionFrame(_MT_ATTACKMODE, m_bCurrDir);
								}
								else	SetMotionFrame(_MT_STAND, m_bCurrDir);
							}
							else	SetMotionFrame(_MT_HORSESTAND, m_bCurrDir);

							if ( CheckMyPostion() == FALSE )
								AdjustMyPostion();
							POINT ptMouse;
							GetCursorPos(&ptMouse);
							ScreenToClient(g_xMainWnd.GetSafehWnd(), &ptMouse);
							ptMouse = mir3::ClientToCanvas(g_xMainWnd.GetSafehWnd(), ptMouse,
							    g_xMainWnd.m_stDisplayInfo.wWidth, g_xMainWnd.m_stDisplayInfo.wHeight);

							if ( HIBYTE(GetKeyState(VK_RBUTTON)) || ( HIBYTE(GetKeyState(VK_LBUTTON)) && HIBYTE(GetKeyState(VK_CONTROL))) )
							{
								m_bCanRun = TRUE;
								OnRButtonDown(ptMouse);
							}
							else if ( HIBYTE(GetKeyState(VK_LBUTTON)) )
							{
								OnLButtonDown(ptMouse);
							}
						}
						break;
					}
				}
			}
			else
			{
				switch ( m_bCurrMtn )
				{
				case _MT_WALK:
				case _MT_RUN:
				case _MT_HORSEWALK:
				case _MT_HORSERUN:
				case _MT_MOODEPO:
					m_pxMap->ScrollMap(m_bMoveDir, m_dwCurrFrame-m_dwFstFrame, m_bMoveSpeed);			
//					m_pxMap->ScrollMap(m_bMoveDir, m_wCurrDelay, m_wDelay*(m_dwEndFrame-m_dwFstFrame), m_bMoveSpeed);
					break;
				case _MT_PUSHBACK:
					m_pxMap->ScrollMap(m_bMoveDir, m_bBackStepFrame, m_bMoveSpeed);			
					break;
				}
			}

			// 连续帧中要做的事。
			if ( m_dwMotionLockTime > _MOTION_LOCKTIME )
			{
				m_dwMotionLockTime = 0;
				m_bMotionLock	   = FALSE;
//				SetOldPosition();
			}

			if ( m_dwWarModeTime > _WARMODE_TIME )
			{
				m_dwWarModeTime = 0;
				m_bWarMode		= FALSE;
			}

			return TRUE;
		}
	}
	else
	{
		if ( m_bCurrMtn == _MT_MON_WALK )
		{
			m_wCurrDelay = 0;

			if ( bIsMoveTime )
			{
				m_dwCurrFrame++;

				if ( m_dwCurrFrame >= m_dwEndFrame-m_bMoveNextFrmCnt && !m_bIsMoved )
				{
					SetMoved();
					m_bIsMoved = TRUE;
				}
			}

			if ( m_dwCurrFrame >= m_dwEndFrame )
			{
				m_dwCurrFrame = m_dwEndFrame-1;
				m_bCanRun = FALSE;
				switch ( m_bCurrMtn )
				{
				case _MT_MON_WALK:
					{
						if ( !m_bMotionLock && m_bInputLock )
						{
							m_bIsMoved = FALSE;
							m_bInputLock = FALSE;
							m_pxMap->SetMovedTileBuffer(m_shShiftTileX, m_shShiftTileY);
							m_bMoveSpeed	= 0;
							m_shShiftPixelX	= 0;
							m_shShiftPixelY	= 0;
							m_shShiftTileX	= 0;
							m_shShiftTileY	= 0;
							
							SetMotionFrame(_MT_MON_STAND, m_bCurrDir);

							// 移动后确认坐标。
							if ( CheckMyPostion() == FALSE )
								AdjustMyPostion();
							POINT ptMouse;
							GetCursorPos(&ptMouse);
							ScreenToClient(g_xMainWnd.GetSafehWnd(), &ptMouse);
							ptMouse = mir3::ClientToCanvas(g_xMainWnd.GetSafehWnd(), ptMouse,
							    g_xMainWnd.m_stDisplayInfo.wWidth, g_xMainWnd.m_stDisplayInfo.wHeight);
							if ( HIBYTE(GetKeyState(VK_LBUTTON)) )
							{
								OnLButtonDown(ptMouse);
							}
						}
						break;
					}
				}
			}
			else
			{
				switch ( m_bCurrMtn )
				{
				case _MT_MON_WALK:
					m_pxMap->ScrollMap(m_bMoveDir, m_dwCurrFrame-m_dwFstFrame, m_bMoveSpeed);			
					break;
				}
			}

			// 连续帧中要做的事。
			if ( m_dwMotionLockTime > _MOTION_LOCKTIME )
			{
				m_dwMotionLockTime = 0;
				m_bMotionLock	   = FALSE;
//				SetOldPosition();
			}

			if ( m_dwWarModeTime > _WARMODE_TIME )
			{
				m_dwWarModeTime = 0;
				m_bWarMode		= FALSE;
			}

			return TRUE;
		}
	}

	return FALSE;
}





BOOL CMyHero::DrawActor(BOOL bFocused, BOOL bShadowAblended, BOOL bUseScrnPos, BOOL bDrawShadow)
{
	// 角色坐标固定.
	m_shScrnPosX = _CHAR_CENTER_XPOS - 24; 
	m_shScrnPosY = _CHAR_CENTER_YPOS - 16;

	if ( m_bIsMon )
	{
		if ( CActor::DrawActor(m_pxMap, bFocused, bShadowAblended, bUseScrnPos, bDrawShadow) )
			return TRUE;
	}
	else
	{
		if ( CHero::DrawActor(m_pxMap, bFocused, bShadowAblended, bUseScrnPos, bDrawShadow) )
			return TRUE;
	}

	return FALSE;
}



BYTE CMyHero::CalcDirection(INT nTargetTileX, INT nTargetTileY)
{
	INT nWidth	= nTargetTileX - m_wPosX;
	INT nHeight = nTargetTileY - m_wPosY;

	FLOAT	rLineLength, rBottomInTriangle;
	INT		nDimension;
	BYTE	bDir;
	rLineLength = (FLOAT)sqrt((double)(nWidth*nWidth+nHeight*nHeight));
	
	// 默认.
	// 7  0  1          
	// 6     2
	// 5  4  3
	// 首先分为 4 个象限(以 90 度为基准), 并对象限应用 cos 值。
	( nWidth >= 0 ) ?
		( nHeight <  0 ? (rBottomInTriangle=(FLOAT)-nHeight, nDimension=0) : (rBottomInTriangle=(FLOAT) nWidth, nDimension=2) ):
		( nHeight >= 0 ? (rBottomInTriangle=(FLOAT) nHeight, nDimension=4) : (rBottomInTriangle=(FLOAT)-nWidth, nDimension=6) );
	// 6(cos45)  0(cos 0)  0(cos45)
	// 4(cos90)  2(cos 0)  2(cos 0)
	// 4(cos45)  2(cos90)  2(cos45)

	FLOAT rCosVal = rBottomInTriangle/rLineLength;
	
//	cos(0), cos(pi/8), cos(pi/4), cos(pi/2)
	CONST FLOAT rCosVal8[4] = { 1.0f, 0.923880f, 0.707107f, 0.382683f };
	
	// 把各个象限分成 3 个区域，再重新调整区域。
	bDir = 0;
	for ( INT nCnt = 0; nCnt < 4; nCnt++ )
	{
		if( rCosVal <= rCosVal8[nCnt] )
			bDir = nDimension+(nCnt+1)/2;
		else break;
	}
	
	if( bDir >= 8 )					bDir = 0;
	
	return bDir;

}



POINT CMyHero::GetPosMouseToTile(INT nXPos, INT nYPos)
{
	POINT ptTilePos;

	if ( m_pxMap )
	{
		ptTilePos.x = m_pxMap->m_shStartViewTileX + (nXPos - _VIEW_CELL_X_START) / _CELL_WIDTH;
		ptTilePos.y = m_pxMap->m_shStartViewTileY + (nYPos - _VIEW_CELL_Y_START) / _CELL_HEIGHT;
	}

	return ptTilePos;
}


BOOL CMyHero::CanNextHit()
{
    const DWORD now=GetTickCount();
    if(DWORD(now-m_dwLastHitTime)<=AttackInterval()) return FALSE;
    m_dwLastHitTime=now;
    g_xGameProc.m_exitGuard.ticks[ExitFlow::Attack]=now;
    return TRUE;
}


BOOL CMyHero::CanWalk()
{
    return m_spellTiming.movement_ready(GetTickCount());
}


BOOL CMyHero::CanRun()
{
   if ( m_stAbility.wHP < _RUN_MINHEALTH )
      return FALSE;

   if ( (timeGetTime() - m_dwLastPKStruckTime < 3000) || !m_spellTiming.movement_ready(GetTickCount()) )
      return FALSE;

   return TRUE;
}





//---------------------------------------------------------------------------------------------------------------//
//  用户输入函数.
//---------------------------------------------------------------------------------------------------------------//

BOOL CMyHero::OnLButtonDown(POINT ptMouse, INT nTargetID, BOOL bIsDead, POINT* lpptTargetPos)
{
    if(g_xGameProc.InputDelayed()) return FALSE;
	POINT ptTargetTile;
	POINT ptMouseTilePos;
	BYTE  bDir;

	if ( g_xGameProc.m_xInterface.OnLButtonDown(ptMouse) )
		return FALSE;

	if ( m_wABlendDelay )
		return FALSE;

	if ( !m_bMotionLock && !m_bInputLock )
	{
		SHORT shLeftMsgCnt = m_xPacketQueue.GetCount();
		if ( shLeftMsgCnt > 0 )
		{
			UpdatePacketState();
			return FALSE;
		}
		
        if(nTargetID!=-1 && lpptTargetPos && !HIBYTE(GetKeyState(VK_MENU))) {
            CActor* target=g_xGameProc.FindActor(nTargetID);
            if(target && target->m_stFeature.bGender<2 && !bIsDead &&
               !g_xGameProc.m_xInterface.m_xOptionWnd.CanAttackHuman(target->m_dwNameClr,HIBYTE(GetKeyState(VK_SHIFT))!=0)) return FALSE;
        }
		ptMouseTilePos = GetPosMouseToTile(ptMouse.x, ptMouse.y);
		bDir		   = CalcDirection(ptMouseTilePos.x, ptMouseTilePos.y);

		if ( m_wPosX == ptMouseTilePos.x && m_wPosY == ptMouseTilePos.y )
			bDir = m_bCurrDir;

		INT nLength;
		if ( lpptTargetPos ) 
			nLength = (INT)sqrt((double)(m_wPosX-lpptTargetPos->x)*(m_wPosX-lpptTargetPos->x) + (m_wPosY-lpptTargetPos->y)*(m_wPosY-lpptTargetPos->y));
		else
			nLength = (INT)sqrt((double)(m_wPosX-ptMouseTilePos.x)*(m_wPosX-ptMouseTilePos.x) + (m_wPosY-ptMouseTilePos.y)*(m_wPosY-ptMouseTilePos.y));


		// 1. SHIFT.(强制攻击)
		if ( HIBYTE(GetKeyState(VK_SHIFT)) )
		{
			if ( m_stFeatureEx.bHorse == _HORSE_NONE )
			{
				if ( m_bUseErgum && g_xGameProc.TargetInLongAttack(bDir) )
				{
					if ( m_stFeatureEx.bHorse == _HORSE_NONE && !bIsDead )
					{
						// 进行攻击.
						ptTargetTile.x = m_wPosX;
						ptTargetTile.y = m_wPosY;
						SetMotionState(_MT_ONEHSWING, bDir, nTargetID, bIsDead, &ptTargetTile);
					}
				}
				else
				{
					ptTargetTile.x = m_wPosX;
					ptTargetTile.y = m_wPosY;
					SetMotionState(_MT_ONEHSWING, bDir, nTargetID, bIsDead, &ptTargetTile);				
				}
			}
		}
		// 2. ALT.(切)
		else if ( HIBYTE(GetKeyState(VK_MENU)) )
		{
			SetMotionState(_MT_CUT, bDir, nTargetID, bIsDead, &ptTargetTile);
		}
		/*
		// 3. 点击的图块是否是我角色所在的位置?
		else if ()
		{
			// 是否有物品?
			if ()
			{
				// 拾取物品.
			}
		}*/
		// 4. 点击的坐标是否在 Actor 区域之内?
		else if ( nTargetID != -1 && lpptTargetPos )
		{
			//1. 目标图块的位置是否在 1 图块以内?
			if ( nLength < 2 && nLength > 0 ) 
			{
				if ( m_stFeatureEx.bHorse == _HORSE_NONE && !bIsDead )
				{
					// 进行攻击.
                    if(auto* target=g_xGameProc.FindActor(nTargetID)) AttackAdjacent(*target);

				}
			}
			// 能向 Actor 所在的方向移动吗？(地图属性检查) 
			else if ( m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, bDir, 1, &ptTargetTile) && nLength > 0)
			{
				// 移动1图块.
				if ( m_stFeatureEx.bHorse == _HORSE_NONE )		SetMotionState(_MT_WALK, bDir, nTargetID, bIsDead, &ptTargetTile);
				else											SetMotionState(_MT_HORSEWALK, bDir, nTargetID, bIsDead, &ptTargetTile);
			}
			else
			{
				if ( m_stFeatureEx.bHorse == _HORSE_NONE )		SetMotionState(_MT_STAND, bDir);
				else											SetMotionState(_MT_HORSESTAND, bDir);

			}
			/*// Actor 是否 NPC?
			if ()
			{
				// 打开对话框或商店窗口.
			}
			// Actor 是否可攻击?
			else if()
			{
				// Actor 的位置是否在 1 图块内?
				if ()
				{
					// 进行攻击.
				}
				// 能否向 Actor 所在方向移动?(地图属性检查)
				else if ()
				{
					// 移动 1 图块.
				}
			}*/
		}
		// 6. 当前能否向鼠标方向移动?(地图属性检查)
		else if ( m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, bDir, 1, &ptTargetTile) && nLength > 0 )
		{
			// 移动1图块.
			if ( m_stFeatureEx.bHorse == _HORSE_NONE )			SetMotionState(_MT_WALK, bDir, nTargetID, bIsDead, &ptTargetTile);
			else												SetMotionState(_MT_HORSEWALK, bDir, nTargetID, bIsDead, &ptTargetTile);

		}
		// 7. 若无法移动，则在周围找出可移动的图块，并找出与目标图块最接近的图块.
		else if ( !m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, bDir, 1, &ptTargetTile) )
		{
			INT nSelectedDir = -1;
			INT nDistance;
			INT nSelectedDistance = 100;
			for ( INT nCnt = 0; nCnt < _MAX_DIRECTION; nCnt++ )
			{
				if ( m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, nCnt, 1, &ptTargetTile) )
				{
					nDistance = (INT)(sqrt((double)(ptTargetTile.x-ptMouseTilePos.x)*(ptTargetTile.x-ptMouseTilePos.x) + 
									  (ptTargetTile.y-ptMouseTilePos.y)*(ptTargetTile.y-ptMouseTilePos.y)));
					if ( nDistance <= nSelectedDistance )
					{
						nSelectedDistance = nDistance;
						nSelectedDir = nCnt;
					}
				}
			}

			if ( nSelectedDir != -1 && nDistance > 0 )
			{
				// 移动1图块.
				m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, nSelectedDir, 1, &ptTargetTile);
				if ( m_stFeatureEx.bHorse == _HORSE_NONE )		SetMotionState(_MT_WALK, nSelectedDir, nTargetID, bIsDead, &ptTargetTile);
				else											SetMotionState(_MT_HORSEWALK, nSelectedDir, nTargetID, bIsDead, &ptTargetTile);
			}
			else
			{
				if ( m_stFeatureEx.bHorse == _HORSE_NONE )		SetMotionState(_MT_STAND, bDir);
				else											SetMotionState(_MT_HORSESTAND, bDir);
			}
		}
		else
		{
			if ( m_stFeatureEx.bHorse == _HORSE_NONE )		SetMotionState(_MT_STAND, bDir);
			else											SetMotionState(_MT_HORSESTAND, bDir);
		}
	}

	return FALSE;
}


BOOL CMyHero::OnRButtonDown(POINT ptMouse)
{
    if(g_xGameProc.InputDelayed()) return FALSE;
	POINT ptTargetTile;
	POINT ptMouseTilePos;
	BYTE  bDir;

	if ( m_wABlendDelay )
		return FALSE;


	if ( !m_bMotionLock && !m_bInputLock )
	{
		SHORT shLeftMsgCnt = m_xPacketQueue.GetCount();
		if ( shLeftMsgCnt > 0 )
		{
			UpdatePacketState();
			return FALSE;
		}
		
		ptMouseTilePos = GetPosMouseToTile(ptMouse.x, ptMouse.y);
		bDir		   = CalcDirection(ptMouseTilePos.x, ptMouseTilePos.y);			

		//1. 目标图块的位置是否在 1 图块以内?
		if ( (INT)(sqrt((double)(m_wPosX-ptMouseTilePos.x)*(m_wPosX-ptMouseTilePos.x) + (m_wPosY-ptMouseTilePos.y)*(m_wPosY-ptMouseTilePos.y)))
			 < 2 ) 
		{
			// 改变方向.
			if ( m_stFeatureEx.bHorse == _HORSE_NONE )		SetMotionState(_MT_STAND, bDir);
			else											SetMotionState(_MT_HORSESTAND, bDir);
		}
		//2. 移动.
		else
		{
			// 可以奔跑时.
			if ( m_bCanRun )
			{
				m_bCanRun = FALSE;

				if ( m_stFeatureEx.bHorse != _HORSE_NONE )
				{
					// 朝向目的地图块的方向时，第 1、2、3 个图块位置没有障碍物。
					if ( m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, bDir, 1, &ptTargetTile) && 
						 m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, bDir, 2, &ptTargetTile) &&
						 m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, bDir, 3, &ptTargetTile) )
					{
						// 移动 3 个图块。
						m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, bDir, 3, &ptTargetTile);
						SetMotionState(_MT_HORSERUN, bDir, 0, FALSE, &ptTargetTile);
					}
					// 朝目标图块方向第 1 个图块的位置没有障碍物时.
					else if ( m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, bDir, 1, &ptTargetTile) )
					{
						SetMotionState(_MT_HORSEWALK, bDir, 0, FALSE, &ptTargetTile);
					}
					// 朝目的地图块的方向, 第 1 个图块的位置上有障碍物时.
					else
					{
						INT nSelectedDir = -1;
						INT nDistance;
						INT nSelectedDistance = 100;
						for ( INT nCnt = 0; nCnt < _MAX_DIRECTION; nCnt++ )
						{
							if ( m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, nCnt, 1, &ptTargetTile) )
							{
								nDistance = (INT)(sqrt((double)(ptTargetTile.x-ptMouseTilePos.x)*(ptTargetTile.x-ptMouseTilePos.x) + 
												  (ptTargetTile.y-ptMouseTilePos.y)*(ptTargetTile.y-ptMouseTilePos.y)));
								if ( nDistance <= nSelectedDistance )
								{
									nSelectedDistance = nDistance;
									nSelectedDir = nCnt;
								}
							}
						}
						if ( nSelectedDir != -1 )
						{
							// 移动1图块.
							m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, nSelectedDir, 1, &ptTargetTile);
							SetMotionState(_MT_HORSEWALK, nSelectedDir, 0, FALSE, &ptTargetTile);
						}
						else
						{
							SetMotionState(_MT_HORSESTAND, nSelectedDir);
						}
					}
				}
				// 没有骑马时。
				else
				{
					// 朝目标图块方向第 1, 2 图块位置没有障碍物时.
					if ( m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, bDir, 1, &ptTargetTile) && 
						 m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, bDir, 2, &ptTargetTile) )
					{
						// 移动 2 个图块。
						m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, bDir, 2, &ptTargetTile);
						SetMotionState(_MT_RUN, bDir, 0, FALSE, &ptTargetTile);
					}
					// 朝目标图块方向第 1 个图块的位置没有障碍物时.
					else if ( m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, bDir, 1, &ptTargetTile) )
					{
						SetMotionState(_MT_WALK, bDir, 0, FALSE, &ptTargetTile);
					}
					// 朝目的地图块的方向, 第 1 个图块的位置上有障碍物时.
					else
					{
						INT nSelectedDir = -1;
						INT nDistance;
						INT nSelectedDistance = 100;
						for ( INT nCnt = 0; nCnt < _MAX_DIRECTION; nCnt++ )
						{
							if ( m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, nCnt, 1, &ptTargetTile) )
							{
								nDistance = (INT)(sqrt((double)(ptTargetTile.x-ptMouseTilePos.x)*(ptTargetTile.x-ptMouseTilePos.x) + 
												  (ptTargetTile.y-ptMouseTilePos.y)*(ptTargetTile.y-ptMouseTilePos.y)));
								if ( nDistance <= nSelectedDistance )
								{
									nSelectedDistance = nDistance;
									nSelectedDir = nCnt;
								}
							}
						}
						if ( nSelectedDir != -1 )
						{
							// 移动1图块.
							m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, nSelectedDir, 1, &ptTargetTile);
							SetMotionState(_MT_WALK, nSelectedDir, 0, FALSE, &ptTargetTile);
						}
						else
						{
							SetMotionState(_MT_STAND, nSelectedDir);
						}
					}
				}
			}
			else
			{
				// 朝目标图块方向第 1 个图块的位置没有障碍物时.
				if ( m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, bDir, 1, &ptTargetTile) )
				{
					if ( m_stFeatureEx.bHorse == _HORSE_NONE )		SetMotionState(_MT_WALK, bDir, 0, FALSE, &ptTargetTile);
					else											SetMotionState(_MT_HORSEWALK, bDir, 0, FALSE, &ptTargetTile);

				}
				// 朝目的地图块的方向, 第 1 个图块的位置上有障碍物时.
				else
				{
					INT nSelectedDir = -1;
					INT nDistance;
					INT nSelectedDistance = 100;
					for ( INT nCnt = 0; nCnt < _MAX_DIRECTION; nCnt++ )
					{
						if ( m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, nCnt, 1, &ptTargetTile) )
						{
							nDistance = (INT)(sqrt((double)(ptTargetTile.x-ptMouseTilePos.x)*(ptTargetTile.x-ptMouseTilePos.x) + 
											  (ptTargetTile.y-ptMouseTilePos.y)*(ptTargetTile.y-ptMouseTilePos.y)));
							if ( nDistance <= nSelectedDistance )
							{
								nSelectedDistance = nDistance;
								nSelectedDir = nCnt;
							}
						}
					}
					if ( nSelectedDir != -1 )
					{
						// 移动1图块.
						m_pxMap->GetNextTileCanMove(m_wPosX, m_wPosY, nSelectedDir, 1, &ptTargetTile);
						if ( m_stFeatureEx.bHorse == _HORSE_NONE )	SetMotionState(_MT_WALK, nSelectedDir, 0, FALSE, &ptTargetTile);
						else										SetMotionState(_MT_HORSEWALK, nSelectedDir, 0, FALSE, &ptTargetTile);
					}
					else
					{
						if ( m_stFeatureEx.bHorse == _HORSE_NONE )	SetMotionState(_MT_STAND, bDir);
						else										SetMotionState(_MT_HORSESTAND, bDir);
					}
				}
			}
		}
	}

	return FALSE;
}


VOID CMyHero::SetMagic(LPCLIENTMAGICRCD pstMagic, BYTE bKeyNum, BYTE bDir, INT nTargetID, FEATURE stTargetFeature, POINT ptTargetPos)
{
    if(g_xGameProc.InputDelayed()) return;
    if(m_pendingAction) return;
    using namespace mir3;
    const DWORD now=GetTickCount();
    const auto& route=SpellRoutes::get(m_shCurrMagicID);
    int followupTarget=nTargetID;
    if(!m_bMotionLock && !m_bInputLock) {
        if(route.action==SpellAction::directed || route.action==SpellAction::area) {
            if(route.self_target) {ptTargetPos={m_wPosX,m_wPosY};nTargetID=m_dwIdentity;followupTarget=nTargetID;}
            m_spellTiming.begin_animated(now,DWORD(pstMagic->stStdMagic.nDelayTime));
            SetMotionState(BYTE(route.action),bDir,nTargetID,TRUE,&ptTargetPos,m_shCurrMagicID);
        } else if(route.action==SpellAction::rush || route.action==SpellAction::special) {
            auto& last=route.action==SpellAction::rush?m_spellTiming.rush_at:m_spellTiming.special_at;
            if(SpellTiming::elapsed_after(now,last,3000)) {
                last=now;
                POINT direction={bDir,0};
                SetMotionState(BYTE(route.action),bDir,0,FALSE,&direction,m_shCurrMagicID);
            }
        }
    }
    switch(route.direct) {
    case DirectSpell::fire_sword:
    case DirectSpell::skill35:
    case DirectSpell::skill34: {
        auto& last=route.direct==DirectSpell::fire_sword?m_spellTiming.fire_sword_at:
            route.direct==DirectSpell::skill35?m_spellTiming.skill35_at:m_spellTiming.skill34_at;
        if(SpellTiming::elapsed_after(now,last,10000)) {
            g_xClientSocket.SendSpellMsg(m_shCurrMagicID,0,0,nTargetID);last=now;
        }
        break;
    }
    case DirectSpell::rapid:
        if(SpellTiming::elapsed_after(now,m_spellTiming.cast_at,200)) {
            m_spellTiming.cast_at=now;m_spellTiming.casting_delay=0;
            g_xClientSocket.SendSpellMsg(m_shCurrMagicID,0,0,nTargetID);
        }
        break;
    case DirectSpell::predicted:
        if(SpellTiming::elapsed_after(now,m_spellTiming.cast_at,1000)) {
            m_spellTiming.cast_at=now;
            m_wOldPosX=m_wPosX;m_wOldPosY=m_wPosY;m_bOldDir=m_bCurrDir;
            BeginSpellReply();
            g_xClientSocket.SendSpellMsg(m_shCurrMagicID,0,0,nTargetID);
            m_bMotionLock=m_bInputLock=TRUE;m_dwMotionLockTime=0;
        }
        break;
    default:break;
    }
    if(route.restrict_followup) {
        m_spellTiming.restricted_at=now;
        g_xGameProc.m_exitGuard.ticks[ExitFlow::Spell]=now;
    }
    m_spellTiming.finish_target(DWORD(followupTarget),stTargetFeature.bGender,rand(),rand());
    // Existing movement code is migrated with the action/reply lifecycle in this batch.
    m_dwLastSpellTime=m_spellTiming.cast_at;
    m_wMagicDelayTime=WORD(m_spellTiming.casting_delay);
    m_wMagicPKDelayTime=WORD(m_spellTiming.movement_delay);
}


bool CMyHero::AttackReady() const {
    return !g_xGameProc.InputDelayed() && !m_pendingAction && !m_bMotionLock && !m_bInputLock &&
        !m_bIsDead && !m_wABlendDelay && !m_spellAwaitingReply && !m_xPacketQueue.GetCount() &&
        m_stFeatureEx.bHorse==_HORSE_NONE && DWORD(GetTickCount()-m_dwLastHitTime)>AttackInterval();
}

bool CMyHero::AttackAdjacent(CActor& target) {
    if(!AttackReady() || target.m_bIsDead) return false;
    const int dx=abs(int(target.m_wPosX)-m_wPosX),dy=abs(int(target.m_wPosY)-m_wPosY);
    if((!dx && !dy) || dx>1 || dy>1) return false;
    if(target.m_stFeature.bGender<2 &&
        !g_xGameProc.m_xInterface.m_xOptionWnd.CanAttackHuman(target.m_dwNameClr,false)) return false;
    POINT position{m_wPosX,m_wPosY};
    SetMotionState(_MT_ONEHSWING,CalcDirection(target.m_wPosX,target.m_wPosY),target.m_dwIdentity,FALSE,&position);
    return true;
}

bool CMyHero::WalkAdjacent(POINT destination) {
    if(g_xGameProc.InputDelayed() || m_pendingAction || m_bMotionLock || m_bInputLock || m_bIsDead ||
        !m_spellTiming.movement_ready(GetTickCount())) return false;
    const int dx=abs(destination.x-m_wPosX),dy=abs(destination.y-m_wPosY);
    if((!dx && !dy) || dx>1 || dy>1) return false;
    const BYTE direction=CalcDirection(destination.x,destination.y);
    POINT next{};
    if(!m_pxMap->GetNextTileCanMove(m_wPosX,m_wPosY,direction,1,&next)) return false;
    SetMotionState(m_stFeatureEx.bHorse==_HORSE_NONE?_MT_WALK:_MT_HORSEWALK,direction,0,FALSE,&next);
    return true;
}

bool CMyHero::CastShortcut(int slot,POINT target,int targetId,FEATURE feature,bool repeat) {
    if(slot<1 || slot>SkillWire::ShortcutCount || g_xGameProc.InputDelayed() || m_pendingAction ||
        m_stFeatureEx.bHorse!=_HORSE_NONE ||
        !m_spellTiming.key_ready(GetTickCount(),slot,repeat)) return false;
    auto* skill=g_xGameProc.m_xInterface.m_xSkillWnd.GetMagicByShortcut(slot);
    if(!skill) return false;
    const auto& route=mir3::SpellRoutes::get(skill->stStdMagic.wMagicID);
    if(route.direct==mir3::DirectSpell::none && (m_bMotionLock || m_bInputLock)) return false;
    if(!route.bypass_mana && (10+skill->stStdMagic.bDefSpell)>m_stAbility.wMP) {
        constexpr int InsufficientManaCaption=1000;
        static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
        g_xGameProc.m_xInterface.m_xClientSysMsg.AddSysMsg(const_cast<char*>(captions.Get(InsufficientManaCaption).c_str()));
        return false;
    }
    m_shCurrMagicID=skill->stStdMagic.wMagicID;
    SetMagic(skill,CSkillWnd::LegacyKey(slot),CalcDirection(target.x,target.y),targetId,feature,target);
    return true;
}

BOOL CMyHero::OnKeyDown(WPARAM wParam, LPARAM lParam, POINT ptMouse, POINT ptTargetPos, INT nMosueTargetID, FEATURE stTargetFeature)
{
	BYTE bMagicKey;
	POINT				ptTargetTile, ptMouseTilePos;

    const int skillSlot=CSkillWnd::PhysicalKeySlot(wParam);
    if(skillSlot && g_xGameProc.InputDelayed()) return FALSE;
    bMagicKey=skillSlot?CSkillWnd::LegacyKey(skillSlot):0;
    ZeroMemory(&ptTargetTile,sizeof(POINT));
    switch(wParam) {
	case VK_SPACE:
		{
			static INT nMode = 0;
			nMode++;
			switch ( nMode )
			{
			case 0:		m_bAttackMode = _MT_ONEHSWING;		break;
			case 1:		m_bAttackMode = _MT_ONEVSWING;		break;
			case 2:		m_bAttackMode = _MT_TWOVSWING;		break;
			case 3:		m_bAttackMode = _MT_TWOHSWING;		break;
			case 4:		m_bAttackMode = _MT_SPEARVSWING;	break;
			case 5:		m_bAttackMode = _MT_SPEARHSWING;	break;
			case 6:		m_bAttackMode = _MT_WHEELWIND;		break;
			case 7:		m_bAttackMode = _MT_RANDSWING;		break;
			case 8:		m_bAttackMode = _MT_BACKDROPKICK;	break;
			case 9:		m_bAttackMode = _MT_ONEHSWING;		break;
			}
			if ( nMode > 8 )		nMode = 0;			
			break;
		}
	}

    if(bMagicKey) {
        ptMouseTilePos=nMosueTargetID?ptTargetPos:GetPosMouseToTile(ptMouse.x,ptMouse.y);
        CastShortcut(skillSlot,ptMouseTilePos,nMosueTargetID,stTargetFeature,(lParam & (1L<<30))!=0);
    }

	return TRUE;
}

void CActor::AddDamage(int value)
{
    m_damage.enabled=g_xGameProc.m_xInterface.m_xOptionWnd.m_values[10];m_damage.Push(value);
}
void CActor::HealthDamage(int before,int after)
{
    m_damage.enabled=g_xGameProc.m_xInterface.m_xOptionWnd.m_values[10];m_damage.Changed(before,after);
}
void CActor::DrawDamage()
{
    m_damage.enabled=g_xGameProc.m_xInterface.m_xOptionWnd.m_values[10];
    for(const auto& glyph:m_damage.Draw(m_shScrnPosX,m_shScrnPosY,m_stFeature.bGender<2))
        DrawSelectionFrame(*g_xGameProc.m_xInterface.m_pxInterImg,glyph.frame,glyph.x,glyph.y,SelectionBlend::Normal,false);
}
