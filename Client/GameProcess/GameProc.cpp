#include "StdAfx.h"
#include "Common/ChatShortcuts.h"
#include "Common/ChatLog.h"
#include "Common/ChatCommandIds.h"
#include "EffectRenderer.h"
#include "SpellSpriteRenderer.h"
#include "WeatherRenderer.h"
#include "TradeWire.h"
#include "MerchantServiceIds.h"
#include "MailWire.h"
#include "AbilityWire.h"
#include "Common/Mir3HumanAppearance.h"
#include "Common/LoginMessages.h"
#include "Common/LegacyPanel.h"


extern CWHQueue	PacketQ;

char	szBuff[8192];
int		nBuff = 0;



/******************************************************************************************************************

	游戏操作相关函数.

*******************************************************************************************************************/

CGameProcess::CGameProcess()
{
	Init();
    m_weather.random_state=GetTickCount();
}


CGameProcess::~CGameProcess()
{
	DeleteProc();
}


VOID CGameProcess::Init()
{
    m_sceneAmbient={};
    m_weather.select(0);m_weather.particles={};m_weather.active=0;m_weather.clear_rain();
    m_exitGuard={};m_exitTransition={};
	m_bTileView				= FALSE;
	m_bObj1View				= FALSE;
	m_bObj2View				= FALSE;
	m_bCellView				= FALSE;
	m_bStatusView			= FALSE;
	m_bStopPtView			= FALSE;
	m_bRender				= FALSE;
	m_bShadowABlend			= FALSE;

	m_bShowFPS				= FALSE;

	ZeroMemory(&m_ptMousePos, sizeof(POINT));
	ZeroMemory(&m_ptCurrTile, sizeof(POINT));
	ZeroMemory(m_stMemMappedID, sizeof(MEMMAPPEDID)*_VIEW_CELL_X_COUNT* _VIEW_CELL_Y_COUNT);

	m_bDayState				= _DAYSTATE_DAY;
	m_bFogState				= _FOGSTATE_DAY;

	m_pxMouseTargetActor	= NULL;		// 当前鼠标所指位置的目标 Actor.
	m_pxMouseOldTargetActor	= NULL;		// 已保存的目标 Actor.
	m_pxSavedTargetActor	= NULL;

	m_fTileLightRed			= 255;
	m_fTileLightGreen		= 230;
	m_fTileLightBlue		= 180;

	m_fDarkRate				= 255.0f;
	m_wInputDelayTime		= 0;
	m_wCurrInputDelayTime	= 0;

	m_wNPCWndPosX			= 0;
	m_wNPCWndPosY			= 0;

	ZeroMemory(m_wShadowClr, 800);

	m_bShowShine			= FALSE;	
	m_dwLastShineTick		= timeGetTime();
	m_wShineCurrDelay		= 0;
	m_bCurrShineFrame		= 0;
	ZeroMemory(m_lpddsShineTextr, sizeof(LPDIRECTDRAWSURFACE7)*10);
 	D3DVECTOR vNorm(0, 0, -1);
	m_avBillboard[0]  = D3DVERTEX(D3DVECTOR(-0.5f, 0.5f, 0), vNorm, 0, 0);
	m_avBillboard[1]  = D3DVERTEX(D3DVECTOR(-0.5f,-0.5f, 0), vNorm, 0, 1);
	m_avBillboard[2]  = D3DVERTEX(D3DVECTOR( 0.5f, 0.5f, 0), vNorm, 1, 0);
	m_avBillboard[3]  = D3DVERTEX(D3DVECTOR( 0.5f,-0.5f, 0), vNorm, 1, 1);

	ZeroMemory(m_szMapName, MAX_PATH);

	m_xImage.InitAllImage();
	m_xInterface.Init();
	m_xMyHero.InitActor();
	m_xMap.InitMapHandler();
	m_xLightFog.Init(RGB(255, 255, 255));
	m_xFlyingTail.InitSystem();
	m_xSmoke.InitSystem();


//	m_xElec.InitElec();

	m_hBrush = CreateSolidBrush(RGB(200, 150, 100));
}


VOID CGameProcess::DeleteProc()
{
	INT			nCnt;
	SHORT		shLeftMsgCnt;
	LPPACKETMSG	lpPacketMsg;

	lpPacketMsg  = NULL;
	shLeftMsgCnt = m_xWaitPacketQueue.GetCount();

	// 清除已堆积的数据包。
	if ( shLeftMsgCnt > 0 )
	{
		for ( nCnt = 0; nCnt < shLeftMsgCnt; nCnt++ )
		{
			lpPacketMsg = (LPPACKETMSG)m_xWaitPacketQueue.PopQ();
			if ( lpPacketMsg )
			{
				SAFE_DELETE(lpPacketMsg);
			}
		}
	}

	lpPacketMsg  = NULL;
	shLeftMsgCnt = m_xWaitDefaultPacketQueue.GetCount();

	// 清除已堆积的数据包。
	if ( shLeftMsgCnt > 0 )
	{
		for ( nCnt = 0; nCnt < shLeftMsgCnt; nCnt++ )
		{
			lpPacketMsg = (LPPACKETMSG)m_xWaitDefaultPacketQueue.PopQ();
			if ( lpPacketMsg )
			{
				SAFE_DELETE(lpPacketMsg);
			}
		}
	}

	D3DWILTextr_InvalidateAllTextures();
	D3DWILTextr_DestroyAllTextures();

	DeleteObject(m_hBrush);
	m_xImage.DeleteAllImage();
	m_xMap.DestroyMapHandler();
	m_xInterface.Destroy();
	m_xMyHero.DestroyActor();


	m_xActorList.ClearAllNodes();
	m_xMagicList.ClearAllNodes();
    m_levelEffects.Clear();ClearNativeSpells();m_weaponTraining=0;
	m_xGroundMagicList.ClearAllNodes();
	m_stMapItemList.ClearAllNodes();

	g_xClientSocket.DisconnectToServer();
	Init();
}

VOID CGameProcess::Load()
{
	INT nCnt;
	g_xMainWnd.ResetDXG(800, 600, _SCREEN_BPP, _DXG_SCREENMODE_WINDOW, _DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D);
//	g_xMainWnd.ResetDXG(800, 600, _SCREEN_BPP, _DXG_SCREENMODE_FULLSCREEN, _DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D);

/*	CHAR szSetup[MAX_PATH];
	ZeroMemory(szSetup, MAX_PATH);
	jRegGetKey(_T("Software\\WEMADE Entertainment\\传奇2"), _T("setup path"), (LPBYTE)szSetup);
	for ( nCnt = strlen(szSetup); nCnt >= 0; nCnt--)
	{
		if ( szSetup[nCnt] == '\\' )
		{
			szSetup[nCnt+1] = NULL;
			break;
		}
	}
*/
	Init();
	m_xLightFog.Init(RGB(255, 255, 255));
	m_xImage.LoadAllImage();
	m_xInterface.CreateInterface(&m_xImage);
	m_xNotice.Load(&m_xImage.m_xImageList[_IMAGE_INTER]);

	g_xClientSocket.ConnectToServer(g_xMainWnd.GetSafehWnd(), g_szServerIP, g_nServerPort);

/*	for ( nCnt = 0; nCnt < m_xImage.m_xImageList[_IMAGE_MAGIC].m_stNewWixImgaeInfo.nIndexCount; nCnt++)
	{
		if ( m_xImage.m_xImageList[_IMAGE_MAGIC].NewSetIndex(nCnt) )
		{
			D3DWILTextr_CreateEmptyTexture(m_xImage.m_xImageList[_IMAGE_MAGIC].m_szWilFileName, nCnt, 
										   m_xImage.m_xImageList[_IMAGE_MAGIC].m_lpstNewCurrWilImageInfo->shWidth,
										   m_xImage.m_xImageList[_IMAGE_MAGIC].m_lpstNewCurrWilImageInfo->shHeight,
										   (WORD*)m_xImage.m_xImageList[_IMAGE_MAGIC].m_pbCurrImage, D3DTEXTR_TRANSPARENTBLACK);
		}
	}
	for ( nCnt = 0; nCnt < m_xImage.m_xImageList[_IMAGE_MONMAGIC].m_stNewWixImgaeInfo.nIndexCount; nCnt++)
	{
		if ( m_xImage.m_xImageList[_IMAGE_MONMAGIC].NewSetIndex(nCnt) )
		{
			D3DWILTextr_CreateEmptyTexture(m_xImage.m_xImageList[_IMAGE_MONMAGIC].m_szWilFileName, nCnt, 
										   m_xImage.m_xImageList[_IMAGE_MONMAGIC].m_lpstNewCurrWilImageInfo->shWidth,
										   m_xImage.m_xImageList[_IMAGE_MONMAGIC].m_lpstNewCurrWilImageInfo->shHeight,
										   (WORD*)m_xImage.m_xImageList[_IMAGE_MONMAGIC].m_pbCurrImage, D3DTEXTR_TRANSPARENTBLACK);
		}
	}
*/
/*	// 物品闪烁纹理.
	for ( nCnt = 0; nCnt < 10; nCnt++)
	{
		if ( m_xImage.m_xImageList[_IMAGE_PROGUSE].NewSetIndex(nCnt+40) )
		{
			D3DWILTextr_CreateEmptyTexture(m_xImage.m_xImageList[_IMAGE_PROGUSE].m_szWilFileName, nCnt+40,
										   m_xImage.m_xImageList[_IMAGE_PROGUSE].m_lpstNewCurrWilImageInfo->shWidth,
										   m_xImage.m_xImageList[_IMAGE_PROGUSE].m_lpstNewCurrWilImageInfo->shHeight,
										   (WORD*)m_xImage.m_xImageList[_IMAGE_PROGUSE].m_pbCurrImage, D3DTEXTR_TRANSPARENTBLACK);
			m_lpddsShineTextr[nCnt] = D3DWILTextr_RestoreEx(m_xImage.m_xImageList[_IMAGE_PROGUSE].m_szWilFileName, nCnt+40, g_xMainWnd.Get3DDevice());
		}
	}
*/



	m_xFlyingTail.InitSystem();
	m_xFlyingTail.SetupSystem();

	m_xSmoke.InitSystem();
	m_xSmoke.SetupSystem();	

//	m_xElec.InitElec();
//	m_xElec.SetupElec();	

	ZeroMemory(m_stMemMappedID, sizeof(MEMMAPPEDID)*_VIEW_CELL_X_COUNT* _VIEW_CELL_Y_COUNT);

	SendMessage(g_xChatEditBox.GetSafehWnd(),EM_SETPASSWORDCHAR, NULL, NULL);
	g_xChatEditBox.SetLimitText(90);

    SetShadowBlend(m_xInterface.m_xOptionWnd.m_values[4]);

	m_xMap.LoadMapImage(&m_xImage);

	SetTimer(g_xMainWnd.GetSafehWnd(), WM_USER + 5000, 10, NULL); // Temp
}	


/******************************************************************************************************************

	函数名 : CGameProcess::LoadMapChanged()

	作者 :
	编写日期 :

	目的   : 地图变更时内存的释放与创建,以及数据加载.
	输入   : CHAR* szMap
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CGameProcess::LoadMapChanged(CHAR* szMap)
{
	m_xMap.LoadMapData(szMap);
    m_xInterface.m_minimap.MapChanged();
    m_xInterface.m_xSiegeWnd.ClearObjects();
    m_levelEffects.Clear();ClearNativeSpells();m_stMapItemList.ClearAllNodes();
    m_xInterface.m_targetDisplay.Clear();

	ZeroMemory(m_stMemMappedID, sizeof(MEMMAPPEDID)*_VIEW_CELL_X_COUNT* _VIEW_CELL_Y_COUNT);
}


VOID CGameProcess::SetNextProc()
{
	DeleteProc();

	g_xClientSocket.m_pxDefProc = g_xMainWnd.m_pxDefProcess = &g_xLoginProc;
	g_xLoginProc.Load();
	g_bProcState = _LOGIN_PROC;

/* 	g_xMainWnd.m_pxDefProcess = &g_xChrSelProc;
	g_xChrSelProc.Load();
	g_bProcState = _CHAR_SEL_PROC; 
	g_xChrSelProc.OnBeforeSelectChar();
*/
}

 
BOOL CGameProcess::TargetInLongAttack(BYTE bDir)
{
	INT		nCnt;
	POINT	ptLongTarget;
	if ( !m_xMap.GetNextTileCanMove(m_xMyHero.m_wPosX, m_xMyHero.m_wPosY, bDir, 2, &ptLongTarget) )
	{
		CActor*	pxActor;
		m_xActorList.MoveCurrentToTop();
		for ( nCnt = 0; nCnt < m_xActorList.GetCounter(); nCnt++)
		{
			pxActor = m_xActorList.GetCurrentData();

			if ( pxActor->m_wPosX == ptLongTarget.x && pxActor->m_wPosY == ptLongTarget.y )
			{
				return TRUE;
			}
			
			m_xActorList.MoveNextNode();
		}
	}

	return FALSE;
}


VOID CGameProcess::CheckMappedData(INT nLoopTime, BOOL bIsMoveTime)
{
    m_xInterface.m_minimap.ClearActors();
    m_xInterface.m_xSiegeWnd.ExpireObjects(GetTickCount());
    m_xInterface.m_xSiegeWnd.CopyMapMarkers(m_xInterface.m_minimap);
	INT		nCnt, nObjCnt;
	BOOL	bIsSaveTargetExisted = FALSE;
	BOOL	bFocused;

	ZeroMemory(m_stMemMappedID, sizeof(MEMMAPPEDID)*_VIEW_CELL_X_COUNT* _VIEW_CELL_Y_COUNT);

	CActor* pxActor = NULL;
	CActor* pxDeadTargetActor = NULL;

	m_xActorList.MoveCurrentToTop();
	for ( nCnt = 0; nCnt < m_xActorList.GetCounter(); )
	{
		pxActor = m_xActorList.GetCurrentData();

		if ( !pxActor )
			continue;

		switch ( pxActor->m_stFeature.bGender )
		{
		case _GENDER_MAN:
		case _GENDER_WOMAN:
			{
				CHero* pxHero;
				pxHero = (CHero*)pxActor;
				pxHero->UpdateMotionState(nLoopTime, bIsMoveTime);
				m_xActorList.MoveNextNode();
				nCnt++;
			}
			break;
		case _GENDER_NPC:
			{
				CNPC* pxNPC;
				pxNPC = (CNPC*)pxActor;
				pxNPC->UpdateMotionState(nLoopTime, bIsMoveTime);
				m_xActorList.MoveNextNode();
				nCnt++;
			}
			break;
		case _GENDER_MON:
			{
				pxActor->UpdateMotionState(nLoopTime, bIsMoveTime);

				if ( pxActor->m_bAppearState == _DIG_DOWNDEL )
				{
					m_xActorList.DeleteCurrentNodeEx();
					delete((CActor*)pxActor);
					pxActor = NULL;
				}
				else
				{
					m_xActorList.MoveNextNode();
					nCnt++;
				}
			}
			break;
		}

		if ( !pxActor )
			continue;

        m_xInterface.m_minimap.AddActor(pxActor->m_stFeature.bGender,pxActor->m_bIsDead!=FALSE,POINT{pxActor->m_wPosX,pxActor->m_wPosY});

		if ( pxActor->m_wPosX >= m_xMap.m_shStartViewTileX && pxActor->m_wPosX < m_xMap.m_shStartViewTileX + _VIEW_CELL_X_COUNT && 
			 pxActor->m_wPosY >= m_xMap.m_shStartViewTileY && pxActor->m_wPosY < m_xMap.m_shStartViewTileY + _VIEW_CELL_Y_COUNT )				 
		{
			for ( nObjCnt = 0; nObjCnt < _MAX_CELL_OBJ; nObjCnt++ )
			{
				if ( m_stMemMappedID[pxActor->m_wPosY - m_xMap.m_shStartViewTileY][pxActor->m_wPosX - m_xMap.m_shStartViewTileX].pxActor[nObjCnt] == NULL )
				{
					bFocused = FALSE;
					if ( pxActor == m_pxSavedTargetActor )
					{
						bIsSaveTargetExisted = TRUE;
					}

					// 传入图像的起始坐标、焦点状态、阴影混合状态。
					if ( !m_pxMouseTargetActor /*&& !pxActor->m_bIsDead*/ )
					{
						if ( PtInRect(&pxActor->m_rcActor, m_ptMousePos) )
						{
							m_pxMouseTargetActor = pxActor;
							bFocused = TRUE;
						}
					}
					else
					{
						if ( PtInRect(&pxActor->m_rcTargetRgn, m_ptMousePos) )
						{
							if ( !pxActor->m_bIsDead )
							{
								m_pxMouseTargetActor = pxActor;
								bFocused = TRUE;
							}
							else
							{
								pxDeadTargetActor = pxActor;
								bFocused = TRUE;
							}
						}
					}	

					// 把死亡的 Actor 向前拉.
					if ( pxActor->m_bIsDead )
					{
						CActor* pxTempActor[_MAX_CELL_OBJ];
						ZeroMemory(pxTempActor, sizeof(CActor*)*_MAX_CELL_OBJ);

						pxTempActor[0] = pxActor;
						memcpy(&pxTempActor[1], m_stMemMappedID[pxActor->m_wPosY - m_xMap.m_shStartViewTileY][pxActor->m_wPosX - m_xMap.m_shStartViewTileX].pxActor, sizeof(CActor*)*(_MAX_CELL_OBJ-1));
						memcpy(m_stMemMappedID[pxActor->m_wPosY - m_xMap.m_shStartViewTileY][pxActor->m_wPosX - m_xMap.m_shStartViewTileX].pxActor, pxTempActor, sizeof(CActor*)*(_MAX_CELL_OBJ));

/*						switch ( pxActor->m_stFeature.bGender )
						{
						case _GENDER_MAN:
						case _GENDER_WOMAN:
							{
								CHero* pxHero;
								pxHero = (CHero*)pxActor;
								pxHero->m_drawnThisFrame |= pxHero->DrawActor(&m_xMap, bFocused);
							}
							break;
						case _GENDER_NPC:
							{
								CNPC* pxNPC;
								pxNPC = (CNPC*)pxActor;
								pxNPC->m_drawnThisFrame |= pxNPC->DrawActor(&m_xMap, bFocused);
							}
							break;
						case _GENDER_MON:
							{
								pxActor->m_drawnThisFrame |= pxActor->DrawActor(&m_xMap, bFocused);
							}
							break;
						}
*/
					}
					else
						m_stMemMappedID[pxActor->m_wPosY - m_xMap.m_shStartViewTileY][pxActor->m_wPosX - m_xMap.m_shStartViewTileX].pxActor[nObjCnt] = pxActor;

					break;
				}
			}
		}
	} 

	if ( !bIsSaveTargetExisted )		m_pxSavedTargetActor = NULL;

	if ( !m_pxMouseTargetActor && pxDeadTargetActor )
	{
		m_pxMouseTargetActor = pxDeadTargetActor;
	}

	CMagic* pxMagic;
	m_xGroundMagicList.MoveCurrentToTop();
	for ( nCnt = 0; nCnt < m_xGroundMagicList.GetCounter(); )
	{
		pxMagic = m_xGroundMagicList.GetCurrentData();

		if ( pxMagic->m_bMagicNum == _SKILL_HOLYSHIELD || pxMagic->m_bMagicNum == _SKILL_EARTHFIRE )
		{
			CRepeatMagic* pxRepeatMagic = (CRepeatMagic*)pxMagic;

			pxRepeatMagic->UpdateMagic(nLoopTime);

			if ( pxRepeatMagic->m_bActiveState == _MAGIC_FINISH )
			{
				m_xGroundMagicList.DeleteCurrentNodeEx();
				delete((CRepeatMagic*)pxRepeatMagic);
				pxRepeatMagic = NULL;
			}
			else
			{
				if ( pxMagic->m_shTargetTileX >= m_xMap.m_shStartViewTileX && pxMagic->m_shTargetTileX < m_xMap.m_shStartViewTileX  + _VIEW_CELL_X_COUNT && 
					 pxMagic->m_shTargetTileY >= m_xMap.m_shStartViewTileY && pxMagic->m_shTargetTileY < m_xMap.m_shStartViewTileY  + _VIEW_CELL_Y_COUNT )
				{
					for ( nObjCnt = 0; nObjCnt < _MAX_CELL_OBJ; nObjCnt++ )
					{
						if ( m_stMemMappedID[pxMagic->m_shTargetTileY - m_xMap.m_shStartViewTileY][pxMagic->m_shTargetTileX - m_xMap.m_shStartViewTileX].pxMagic[nObjCnt] == NULL )
						{
 							m_stMemMappedID[pxMagic->m_shTargetTileY - m_xMap.m_shStartViewTileY][pxMagic->m_shTargetTileX - m_xMap.m_shStartViewTileX].pxMagic[nObjCnt] = pxMagic;
							break;
						}
					}
				}

				m_xGroundMagicList.MoveNextNode();
				nCnt++;
			}
		}
		else if ( pxMagic->m_bMagicNum == _SKILL_FIRE )
		{
			CMagicStream* pxMagicStream = (CMagicStream*)pxMagic;

			pxMagicStream->UpdateMagic(nLoopTime);

			if ( pxMagicStream->m_bActiveState == _MAGIC_FINISH )
			{
				m_xGroundMagicList.DeleteCurrentNodeEx();
				delete((CMagicStream*)pxMagicStream);
				pxMagicStream = NULL;
			}
			else
			{
				if ( pxMagic->m_shTargetTileX >= m_xMap.m_shStartViewTileX && pxMagic->m_shTargetTileX < m_xMap.m_shStartViewTileX  + _VIEW_CELL_X_COUNT && 
					 pxMagic->m_shTargetTileY >= m_xMap.m_shStartViewTileY && pxMagic->m_shTargetTileY < m_xMap.m_shStartViewTileY  + _VIEW_CELL_Y_COUNT )
				{
					for ( nObjCnt = 0; nObjCnt < _MAX_CELL_OBJ; nObjCnt++ )
					{
						if ( m_stMemMappedID[pxMagic->m_shTargetTileY - m_xMap.m_shStartViewTileY][pxMagic->m_shTargetTileX - m_xMap.m_shStartViewTileX].pxMagic[nObjCnt] == NULL )
						{
							m_stMemMappedID[pxMagic->m_shTargetTileY - m_xMap.m_shStartViewTileY][pxMagic->m_shTargetTileX - m_xMap.m_shStartViewTileX].pxMagic[nObjCnt] = pxMagic;
							break;
						}
					}
				}

				m_xGroundMagicList.MoveNextNode();
				nCnt++;
			}
		}
	}		
	
	LPGROUNDITEM pstItem;
	m_stMapItemList.MoveCurrentToTop();
	for ( nCnt = 0; nCnt < m_stMapItemList.GetCounter(); nCnt++ )
	{
		pstItem = m_stMapItemList.GetCurrentData();

		if ( pstItem->shTileX >= m_xMap.m_shStartViewTileX && pstItem->shTileX < m_xMap.m_shStartViewTileX  + _VIEW_CELL_X_COUNT && 
			 pstItem->shTileY >= m_xMap.m_shStartViewTileY && pstItem->shTileY < m_xMap.m_shStartViewTileY  + _VIEW_CELL_Y_COUNT )
		{
			for ( nObjCnt = 0; nObjCnt < GroundItemDisplay::CellCapacity; nObjCnt++ )
			{
				if ( m_stMemMappedID[pstItem->shTileY - m_xMap.m_shStartViewTileY][pstItem->shTileX - m_xMap.m_shStartViewTileX].pstItem[nObjCnt] == NULL )
				{
					m_stMemMappedID[pstItem->shTileY - m_xMap.m_shStartViewTileY][pstItem->shTileX - m_xMap.m_shStartViewTileX].pstItem[nObjCnt] = pstItem;
					break;
				}
			}
		}

		m_stMapItemList.MoveNextNode();
	}
}


VOID CGameProcess::AutoTargeting()
{
	static BYTE bLastDir = _DIRECTION_LIST_1;

	if ( !m_xMyHero.m_bMotionLock && !m_xMyHero.m_bInputLock && m_pxSavedTargetActor )
	{
		SHORT shLeftMsgCnt = m_xMyHero.m_xPacketQueue.GetCount();
		if ( shLeftMsgCnt > 0 )
		{
			if ( m_xMyHero.m_stFeatureEx.bHorse == _HORSE_NONE )	m_xMyHero.SetMotionState(_MT_STAND,      m_xMyHero.m_bCurrDir);
			else													m_xMyHero.SetMotionState(_MT_HORSESTAND, m_xMyHero.m_bCurrDir);
			m_xMyHero.UpdatePacketState();
			return;
		}

		POINT ptTargetTile = {0, 0};
		BYTE bDir = m_xMyHero.CalcDirection(m_pxSavedTargetActor->m_wPosX, m_pxSavedTargetActor->m_wPosY);

		if ( m_pxSavedTargetActor->m_bIsDead )
			return;

		if ( (m_pxSavedTargetActor->m_stFeature.bGender == _GENDER_MON && m_pxSavedTargetActor->m_bCurrMtn == _MT_MON_DIE) ||
			 (m_pxSavedTargetActor->m_stFeature.bGender <= _GENDER_WOMAN && m_pxSavedTargetActor->m_bCurrMtn == _MT_DIE) )
			 return;


		if ( (INT)(sqrt((double)(m_xMyHero.m_wPosX-m_pxSavedTargetActor->m_wPosX)*(m_xMyHero.m_wPosX-m_pxSavedTargetActor->m_wPosX) + 	(m_xMyHero.m_wPosY-m_pxSavedTargetActor->m_wPosY)*(m_xMyHero.m_wPosY-m_pxSavedTargetActor->m_wPosY)))
			      < 2 )
		{
			if ( m_xMyHero.m_stFeatureEx.bHorse == _HORSE_NONE && !m_pxSavedTargetActor->m_bIsDead )
			{
				// 进行攻击.
				ptTargetTile.x = m_xMyHero.m_wPosX;
				ptTargetTile.y = m_xMyHero.m_wPosY;
				m_xMyHero.SetMotionState(_MT_ONEHSWING, bDir, m_pxSavedTargetActor->m_dwIdentity, m_pxSavedTargetActor->m_bIsDead, &ptTargetTile);
			}
		}
		// 能向 Actor 所在的方向移动吗？(地图属性检查) 
		else if ( m_xMap.GetNextTileCanMove(m_xMyHero.m_wPosX, m_xMyHero.m_wPosY, bDir, 1, &ptTargetTile) )
		{
			// 移动1图块.
			if ( m_xMyHero.m_stFeatureEx.bHorse == _HORSE_NONE )
			{
				m_xMyHero.SetMotionState(_MT_WALK, bDir);
			}
			else
			{
				m_xMyHero.SetMotionState(_MT_HORSEWALK, bDir);
			}
			bLastDir = bDir;
		}
		else if ( !m_xMap.GetNextTileCanMove(m_xMyHero.m_wPosX, m_xMyHero.m_wPosY, bDir, 1, &ptTargetTile) )
		{
			BYTE	bSelectedDirList[8] = {0, 0, 0, 0, 0, 0, 0, 0};
			INT		nSeclectedDirCnt = -1;
			INT		nLastDistance = 100;
			INT		nCurrDistance;
			BYTE	bInvDir;
			BYTE	bSelectedDir = 255;

			if ( bLastDir < 4 )		bInvDir = bLastDir + 4;
			else					bInvDir = bLastDir - 4;

			for ( INT nCnt = 0; nCnt < _MAX_DIRECTION; nCnt++ )
			{
				if ( m_xMap.GetNextTileCanMove(m_xMyHero.m_wPosX, m_xMyHero.m_wPosY, nCnt, 1, &ptTargetTile) )
				{
					nCurrDistance = (INT)(sqrt((double)(ptTargetTile.x-m_pxSavedTargetActor->m_wPosX)*(ptTargetTile.x-m_pxSavedTargetActor->m_wPosX) + (ptTargetTile.y-m_pxSavedTargetActor->m_wPosY)*(ptTargetTile.y-m_pxSavedTargetActor->m_wPosY)));
					if ( nCurrDistance <= nLastDistance && bInvDir != nCnt )
					{
						nLastDistance = nCurrDistance;
						nSeclectedDirCnt++;
						bSelectedDirList[nSeclectedDirCnt] = nCnt;
					}
				}
			}

			BYTE bDirOrderList[8][8] = 
			{
				{0, 7, 1, 6, 2, 5, 3, 4},
				{1, 0, 2, 7, 3, 6, 4, 5},
				{2, 1, 3, 0, 4, 7, 5, 6},
				{3, 2, 4, 1, 5, 0, 6, 7},
				{4, 5, 3, 6, 2, 7, 1, 0},
				{5, 6, 4, 7, 3, 0, 2, 1},
				{6, 7, 5, 0, 4, 1, 3, 2},
				{7, 0, 6, 1, 5, 2, 4, 3},
			};

			BYTE bOrderCnt = 0;

			for ( nCnt = 0; nCnt < nSeclectedDirCnt; nCnt++ )
			{
				while ( bSelectedDirList[nCnt] != bDirOrderList[bDir][bOrderCnt] || bOrderCnt < 8 )	bOrderCnt++;
				if ( bOrderCnt >= 8 )			bOrderCnt = 0;
				else					
				{	bSelectedDir = bLastDir = bSelectedDirList[nCnt];	break;	}
			}

			if ( bSelectedDir == 255 )		bSelectedDir = bLastDir = bSelectedDirList[GetRandomNum(0, nSeclectedDirCnt+1)];
	

			if ( nSeclectedDirCnt != -1 )
			{
				// 移动1图块.
				m_xMap.GetNextTileCanMove(m_xMyHero.m_wPosX, m_xMyHero.m_wPosY, bSelectedDir, 1, &ptTargetTile);
				if ( m_xMyHero.m_stFeatureEx.bHorse == _HORSE_NONE )
				{
					m_xMyHero.SetMotionState(_MT_WALK, bSelectedDir);
				}
				else
				{
					m_xMyHero.SetMotionState(_MT_HORSEWALK, bSelectedDir);
				}
			}
			else
			{
				if ( m_xMyHero.m_stFeatureEx.bHorse == _HORSE_NONE )
				{
					m_xMyHero.SetMotionState(_MT_STAND, bDir);
				}
				else
				{
					m_xMyHero.SetMotionState(_MT_HORSESTAND, bDir);
				}
			}
		}
		else
		{
			if ( m_xMyHero.m_stFeatureEx.bHorse == _HORSE_NONE )
			{
				m_xMyHero.SetMotionState(_MT_STAND, bDir);
			}
			else
			{
				m_xMyHero.SetMotionState(_MT_HORSESTAND, bDir);
			}
		}
	}
}


VOID CGameProcess::DropItemShow()
{
    if(m_xNotice.IsActive() || !m_xInterface.SpellPointerAvailable()) return;
    static CLoginMessages messages;static bool loaded=messages.Load("CmList.dat");
    const auto& coin=messages.Get(GroundItemDisplay::CoinNameMessage);
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font=g_xMainWnd.CreateGameFont(face,9);
    HDC dc=GetDC(g_xMainWnd.GetSafehWnd());HGDIOBJ previous=SelectObject(dc,font);
    struct Label {RECT bounds;std::string text;};std::vector<Label> labels;
    int y=(m_ptCurrTile.y-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_shViewOffsetY-4;
    m_stMapItemList.MoveCurrentToTop();
    for(int index=0;index<m_stMapItemList.GetCounter();++index) {
        const auto* item=m_stMapItemList.GetCurrentData();m_stMapItemList.MoveNextNode();
        if(item->shTileX!=m_ptCurrTile.x || item->shTileY!=m_ptCurrTile.y) continue;
        SIZE size={};GetTextExtentPoint32A(dc,item->szItemName,int(strlen(item->szItemName)),&size);
        const int x=(item->shTileX-m_xMap.m_shStartViewTileX)*_CELL_WIDTH+_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX+24-size.cx/2;
        const bool variant=GroundItemDisplay::CoinVariant(item->szItemName,coin);
        labels.push_back({{x,y,x+size.cx,y+15},variant?coin:std::string(item->szItemName)});
        if(variant || labels.size()==GroundItemDisplay::CellCapacity) break;
        y-=16;
    }
    SelectObject(dc,previous);ReleaseDC(g_xMainWnd.GetSafehWnd(),dc);
    for(const auto& label:labels) DrawLegacyPanel(label.bounds,RGB(80,60,40),150,0,false);
    if(SUCCEEDED(g_xMainWnd.GetBackBuffer()->GetDC(&dc))) {
        previous=SelectObject(dc,font);SetBkMode(dc,TRANSPARENT);
        for(const auto& label:labels) {
            SetTextColor(dc,RGB(0,0,0));TextOutA(dc,label.bounds.left-1,label.bounds.top+2,label.text.c_str(),int(label.text.size()));
            SetTextColor(dc,RGB(255,255,125));TextOutA(dc,label.bounds.left,label.bounds.top+1,label.text.c_str(),int(label.text.size()));
        }
        SelectObject(dc,previous);g_xMainWnd.GetBackBuffer()->ReleaseDC(dc);
    }
    DeleteObject(font);
}

void CGameProcess::LightSceneMap()
{
    if(!m_xMap.m_pstCellInfo) return;
    constexpr int radii[]={4,8,12,20};
    constexpr int colors[][3]={{173,198,214},{90,140,165},{41,57,115},{222,198,99},{41,115,49},
        {255,255,255},{255,255,255},{255,255,255},{255,255,255},{255,255,255}};
    const int cameraX=m_xMap.m_shStartViewTileX*_CELL_WIDTH+m_xMap.m_shViewOffsetX-_VIEW_CELL_X_START;
    const int cameraY=m_xMap.m_shStartViewTileY*_CELL_HEIGHT+m_xMap.m_shViewOffsetY-_VIEW_CELL_Y_START;
    for(int y=max(0,cameraY/32-20);y<=min(int(m_xMap.m_stMapFileHeader.shHeight)-1,cameraY/32+38);++y)
        for(int x=max(0,cameraX/48-20);x<=min(int(m_xMap.m_stMapFileHeader.shWidth)-1,cameraX/48+37);++x) {
            const WORD packed=m_xMap.m_pstCellInfo[y+x*m_xMap.m_stMapFileHeader.shHeight].wLigntNEvent;
            const int color=(packed>>4)&255;
            if(!packed || color>=10) continue;
            m_xLightFog.SetLightRadiusWithRing(x*48-cameraX,y*32-cameraY,0,0,0,0,
                radii[packed>>14],colors[color][0],colors[color][1],colors[color][2]);
        }
}

void CGameProcess::LightSceneActors()
{
    const auto add=[&](const mir3::SceneLight& light) {
        m_xLightFog.SetLightRadiusWithRing(light.x,light.y,light.inner_radius,
            light.inner[0],light.inner[1],light.inner[2],light.outer_radius_cells,
            light.outer[0],light.outer[1],light.outer[2]);
    };
    add(mir3::SceneLight::actor(m_xMyHero.m_shScrnPosX,m_xMyHero.m_shScrnPosY,m_xMyHero.m_bLightSize,true));
    const RECT viewport={0,0,800,600};
    std::vector<CActor*> actors;
    m_xActorList.MoveCurrentToTop();
    for(int i=0;i<m_xActorList.GetCounter();++i) {
        auto* actor=m_xActorList.GetCurrentData();m_xActorList.MoveNextNode();
        if(actor->m_drawnThisFrame) actors.push_back(actor);
    }
    const auto rank=[](CActor* actor) {return actor->m_stFeature.bGender==_GENDER_NPC?1:actor->m_stFeature.bGender==_GENDER_MON?2:0;};
    std::sort(actors.begin(),actors.end(),[&](CActor* a,CActor* b) {
        return rank(a)!=rank(b)?rank(a)<rank(b):int(a->m_dwIdentity)<int(b->m_dwIdentity);
    });
    for(auto* actor:actors) {
        const auto gender=actor->m_stFeature.bGender;
        if(gender!=_GENDER_MAN && gender!=_GENDER_WOMAN && gender!=_GENDER_NPC && gender!=_GENDER_MON) continue;
        if(gender==_GENDER_MON && actor->m_bAppearState==_DIG_DOWNDEL) continue;
        if(gender!=_GENDER_NPC && actor->m_bIsDead) continue;
        RECT visible={};if(!IntersectRect(&visible,&viewport,&actor->m_rcActor)) continue;
        add(mir3::SceneLight::actor(actor->m_shScrnPosX+24,actor->m_shScrnPosY+16,actor->m_bLightSize,false));
    }
}

void CGameProcess::EnterSceneAmbient(WORD mode)
{
    m_sceneAmbient.enter(mode);m_bFogState=m_sceneAmbient.mode;
    m_fDarkRate=float(m_sceneAmbient.level());
    m_xLightFog.ChangeLightColor(RGB(int(m_fDarkRate),int(m_fDarkRate),int(m_fDarkRate)));
}

void CGameProcess::AdvanceSceneAmbient(DWORD elapsed)
{
    m_sceneAmbient.advance(elapsed);m_fDarkRate=float(m_sceneAmbient.level());
    m_xLightFog.ChangeLightColor(RGB(int(m_fDarkRate),int(m_fDarkRate),int(m_fDarkRate)));
}

VOID CGameProcess::RenderScene(INT nLoopTime)
{
    if(m_exitTransition.Advance(DWORD(nLoopTime))) {ReturnToSelection();return;}
	BOOL		 bIsMoveTime	= FALSE;
	static WORD	 wMoveTime		= 0;
	static DWORD dwReceiveTime	= 0;

	wMoveTime			  += nLoopTime;
	m_wCurrInputDelayTime += nLoopTime;

	if ( nLoopTime < 10000 )		dwReceiveTime += nLoopTime;
	if ( m_wCurrInputDelayTime >= m_wInputDelayTime )
	{
		m_wCurrInputDelayTime	= 0;
		m_wInputDelayTime		= 0;
	}
	if ( wMoveTime > 100 )
	{
		bIsMoveTime = TRUE;
		wMoveTime	= 0;
	}
	if ( !m_bRender && dwReceiveTime > 100000 )
	{
		MessageBox(g_xMainWnd.GetSafehWnd(), "Failed to receive logon information. The program will exit.", "", MB_OK);
		PostQuitMessage(NULL);
		dwReceiveTime = 0;
	}

	if ( m_bRender )
	{
        m_ptCurrTile.x=m_xMap.m_shStartViewTileX+(m_ptMousePos.x-_VIEW_CELL_X_START+m_xMap.m_shViewOffsetX)/_CELL_WIDTH;
        m_ptCurrTile.y=m_xMap.m_shStartViewTileY+(m_ptMousePos.y-_VIEW_CELL_Y_START+m_xMap.m_shViewOffsetY)/_CELL_HEIGHT;
        m_weather.advance(nLoopTime,m_weatherEnabled);
        AdvanceSceneAmbient(DWORD(nLoopTime));
		m_xLightFog.ClearSavedLightColor();
        m_xActorList.MoveCurrentToTop();
        for(int i=0;i<m_xActorList.GetCounter();++i) {
            m_xActorList.GetCurrentData()->m_drawnThisFrame=false;m_xActorList.MoveNextNode();
        }
        const auto nativeSpellBoundary=m_nextNativeSpell;
        m_levelEffects.Advance(nLoopTime);

		m_xMyHero.UpdateMotionState(nLoopTime, bIsMoveTime);
        LightSceneMap();
		// 绘制地图.
//		Clear(0);
		g_xMainWnd.DrawWithImagePerLineClipRgn(
			m_xMap.m_rcView.left+_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX,
			m_xMap.m_rcView.top+_VIEW_CELL_Y_START-m_xMap.m_shViewOffsetY,
			m_xMap.m_rcView.right-m_xMap.m_rcView.left,
			m_xMap.m_rcView.bottom-m_xMap.m_rcView.top,
			m_xMap.m_wSavedMapBuffer, _CLIP_WIDTH, _CLIP_HEIGHT);

		m_pxMouseTargetActor = NULL;
        m_stMapItemList.MoveCurrentToTop();
        for(int index=0;index<m_stMapItemList.GetCounter();) {
            auto* item=m_stMapItemList.GetCurrentData();
            if(!GroundItemDisplay::Nearby(int(item->shTileX)-m_xMyHero.m_wPosX,int(item->shTileY)-m_xMyHero.m_wPosY)) {
                m_stMapItemList.DeleteCurrentNodeEx();delete item;continue;
            }
            item->sparkle.Advance(GetTickCount(),DWORD(nLoopTime));
            m_stMapItemList.MoveNextNode();++index;
        }
		CheckMappedData(nLoopTime, bIsMoveTime);
        AdvanceNativeSpells(DWORD(nLoopTime),nativeSpellBoundary);DrawSpellMarks();
		AutoTargeting();

		RenderObject(nLoopTime);
		RenderMapTileGrid();		//For Debug	

		CMagic* pxMagic;
		m_xMagicList.MoveCurrentToTop();
		for ( INT nCnt = 0; nCnt < m_xMagicList.GetCounter(); )
		{
			pxMagic = m_xMagicList.GetCurrentData();

			if ( pxMagic->m_bMagicNum == _SKILL_SHOOTLIGHTEN )
			{
				CElecMagic* pxElecMagic = (CElecMagic*)pxMagic;;

				pxElecMagic->UpdateMagic(nLoopTime);

				if ( pxElecMagic->m_bActiveState == _MAGIC_FINISH )
				{
					m_xMagicList.DeleteCurrentNodeEx();
					delete((CElecMagic*)pxElecMagic);
					pxElecMagic = NULL;
				}
				else
				{
					m_xMagicList.MoveNextNode();
					nCnt++;
				}
			}
			else
			{
				pxMagic->UpdateMagic(nLoopTime);
				if ( pxMagic->m_bActiveState == _MAGIC_FINISH )
				{
					m_xMagicList.DeleteCurrentNodeEx();
					delete((CMagic*)pxMagic);
					pxMagic = NULL;
				}
				else
				{
					m_xMagicList.MoveNextNode();
					nCnt++;
				}
			}
		}


        NativeWeather::Draw(m_weather,m_weatherEnabled);

		m_xFlyingTail.UpdateSystem(nLoopTime);
		m_xFlyingTail.RenderSystem(g_xMainWnd.Get3DDevice());

		m_xSmoke.UpdateSystem(nLoopTime);
		m_xSmoke.RenderSystem(g_xMainWnd.Get3DDevice());		

//		m_xElec.UpdateElec(400, 300, 0, 0);
//		m_xElec.RenderElec(g_xMainWnd.Get3DDevice());


        m_levelEffects.Light();LightNativeSpells();LightSceneActors();
		m_xLightFog.ShowLightFog();

		m_xMyHero.ShowMessage(nLoopTime);
		m_xMyHero.DrawHPBar();
        m_xMyHero.DrawDamage();

		if ( m_xActorList.GetCounter() != 0 )
		{
			m_xActorList.MoveCurrentToTop();
			CActor* pxActor;
			for ( INT nCnt = 0; nCnt < m_xActorList.GetCounter(); nCnt++ )
			{
				pxActor = m_xActorList.GetCurrentData();
                pxActor->DrawDamage();

				switch ( pxActor->m_stFeature.bGender )
				{
				case _GENDER_MAN:
				case _GENDER_WOMAN:
					{
						CHero*	pxHero;
						pxHero = (CHero*)pxActor;
						pxHero->ShowMessage(nLoopTime);
						pxHero->DrawHPBar();
					}
					break;
				case _GENDER_NPC:
				case _GENDER_MON:
					{
						pxActor->ShowMessage(nLoopTime);
						pxActor->DrawHPBar();
					}
					break;
				}

				m_xActorList.MoveNextNode();
			}
		}

		DropItemShow();
        m_levelEffects.Draw();DrawNativeSpells();DrawSpellParticles();
        m_stMapItemList.MoveCurrentToTop();
        for(int index=0;index<m_stMapItemList.GetCounter();++index) {
            const auto* item=m_stMapItemList.GetCurrentData();
            if(item->sparkle.frame>=0) {
                const int x=(item->shTileX-m_xMap.m_shStartViewTileX)*_CELL_WIDTH+_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX+8;
                const int y=(item->shTileY-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_shViewOffsetY;
                DrawSelectionFrame(m_xImage.m_xImageList[_IMAGE_PROGUSE],GroundItemDisplay::SparkleFirst+item->sparkle.frame,x,y,SelectionBlend::Additive,false,1);
            }
            m_stMapItemList.MoveNextNode();
        }
//		RenderShine(nLoopTime);

		if ( m_xMagicList.GetCounter() != 0 )
		{
			m_xMagicList.MoveCurrentToTop();
			CMagic* pxMagic;

			for ( INT nCnt = 0; nCnt < m_xMagicList.GetCounter(); nCnt++ )
			{
				pxMagic = m_xMagicList.GetCurrentData();

				if ( pxMagic->m_bMagicNum == _SKILL_SHOOTLIGHTEN )
				{
					CElecMagic* pxElecMagic = (CElecMagic*)pxMagic;
					pxElecMagic->DrawMagic();
				}
				else			
					pxMagic->DrawMagic();
				m_xMagicList.MoveNextNode();
			}
		}	

		if ( m_pxMouseTargetActor )
		{
			if ( m_pxMouseOldTargetActor != m_pxMouseTargetActor )
			{
				if ( !m_pxMouseTargetActor->m_szName[0] )
					g_xClientSocket.SendQueryName(m_pxMouseTargetActor->m_dwIdentity, m_pxMouseTargetActor->m_wPosX, m_pxMouseTargetActor->m_wPosY);
			}

			m_pxMouseOldTargetActor = m_pxMouseTargetActor;
			m_pxMouseTargetActor->DrawName();
//m_pxMouseTargetActor->DrawHPBar();
		}

		if ( m_wNPCWndPosX && m_wNPCWndPosY && ( abs(m_wNPCWndPosX-m_xMyHero.m_wPosX) > 5 || abs(m_wNPCWndPosY-m_xMyHero.m_wPosY) > 5) )
		{
			CloseNPCWindow();
		}

        const bool worldPointer=m_ptMousePos.y<(m_xInterface.m_chatExpanded?305:470) && m_xInterface.GetWindowInMousePos(m_ptMousePos)==-1 &&
            !m_xInterface.m_xMsgBox.IsActive() && !m_xInterface.m_xGuildEditor.m_bActive;
        UpdateTargetDisplay(worldPointer?m_pxMouseTargetActor:nullptr,GetTickCount());
		m_xInterface.RenderInterface(nLoopTime, m_ptMousePos);

		ShowStatus(0, 0);
	}
	else
		Clear(0);

//g_xMainWnd.DrawWithGrayBackBuffer();
	m_xNotice.RenderMessageBox(nLoopTime);
    DrawExitTransition();
}


VOID CGameProcess::CloseNPCWindow()
{
    if (m_xInterface.m_xInventoryWnd.m_bActive && m_xInterface.m_xInventoryWnd.service.actor) m_xInterface.WindowActivate(_WND_ID_INVENTORY);
    if (m_xInterface.m_xStoreWnd.GetGameWndActive()) m_xInterface.WindowActivate(_WND_ID_STORE);
	if ( m_xInterface.m_xNPCWnd.GetGameWndActive() )
	{
		m_xInterface.DeleteWindowToList(_WND_ID_NPCCHAT);
		m_xInterface.m_xNPCWnd.SetGameWndActive(FALSE);
        if (m_xInterface.m_xMsgBox.IsActive() && m_xInterface.m_nIdxMsgBox==7)
            m_xInterface.FinishNotice(CLICK_NO,false);
        m_xInterface.m_xNPCWnd.ResetDialog();

		m_wNPCWndPosX = m_wNPCWndPosY = 0;
	}
}



BOOL CGameProcess::RenderShine(INT nLoopTime)
{
	DWORD dwCurrTick = timeGetTime();

	if ( dwCurrTick - m_dwLastShineTick > 5000 )
	{
		m_bShowShine = TRUE;
		m_dwLastShineTick = dwCurrTick;
		m_wShineCurrDelay = 0;
		m_bCurrShineFrame = 0;
	}

	m_wShineCurrDelay += nLoopTime;
	if ( m_wShineCurrDelay > 120 )
	{
		m_bCurrShineFrame++;
		m_wShineCurrDelay = 0;
		if ( m_bCurrShineFrame > 9 )
		{
			m_bCurrShineFrame = 0;
			m_bShowShine = FALSE;
		}
	}

	if ( g_xMainWnd.Get3DDevice() && m_bShowShine )
	{
		if( SUCCEEDED(g_xMainWnd.Get3DDevice()->BeginScene()) )
		{
			D3DVECTOR vTrans;
			D3DMATRIX matTrans;
			D3DMATRIX matScale;
			D3DMATRIX matRot;
			D3DMATRIX matWorld;
			D3DMATRIX matTempWorld;
			D3DMATRIX matWorldOriginal;

			g_xMainWnd.Get3DDevice()->GetTransform(D3DTRANSFORMSTATE_WORLD, &matWorldOriginal);

			D3DMATERIAL7 mtrl;

			CWHWilImageData* pxShineImg = &m_xImage.m_xImageList[_IMAGE_PROGUSE];
			if ( pxShineImg->NewSetIndex(m_bCurrShineFrame+40) )
			{
				if ( m_stMapItemList.GetCounter() != 0 )
				{
					m_stMapItemList.MoveCurrentToTop();
					LPGROUNDITEM pstItem;

					for ( INT nCnt = 0; nCnt < m_stMapItemList.GetCounter(); nCnt++ )
					{
						pstItem = m_stMapItemList.GetCurrentData();

/*						INT nCenterX = (pstItem->shTileX-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX;
						INT nCenterY = (pstItem->shTileY-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_shViewOffsetY;
*/						INT nCenterX = (pstItem->shTileX-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX+_CELL_WIDTH /2;
						INT nCenterY = (pstItem->shTileY-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_shViewOffsetY+_CELL_HEIGHT/2;

						g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorldOriginal);
						vTrans.x = (FLOAT) nCenterX-400; 
						vTrans.y = (FLOAT)-nCenterY+300; 
						vTrans.z = 0;

						D3DUtil_SetTranslateMatrix(matTrans, vTrans);
						D3DUtil_SetScaleMatrix(matScale, (FLOAT)pxShineImg->m_lpstNewCurrWilImageInfo->shWidth, (FLOAT)pxShineImg->m_lpstNewCurrWilImageInfo->shHeight, 0.0f);
						D3DMath_MatrixMultiply(matWorld, matScale, matTrans);
						g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld);

						if ( m_lpddsShineTextr[m_bCurrShineFrame] )
							g_xMainWnd.Get3DDevice()->SetTexture(0, m_lpddsShineTextr[m_bCurrShineFrame]);
						D3DUtil_InitMaterial(mtrl, (FLOAT)255/255.0f, (FLOAT)255/255.0f, (FLOAT)255/255.0f);
						mtrl.diffuse.a = 1/255.0f;
						g_xMainWnd.Get3DDevice()->SetMaterial(&mtrl);
						SetBlendRenderState(g_xMainWnd.Get3DDevice(), _BLEND_LIGHTINV, mtrl);
						g_xMainWnd.Get3DDevice()->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX, m_avBillboard, 4, NULL);

						// 恢复原状.
						ZeroMemory(&mtrl, sizeof(mtrl));
						mtrl.diffuse.r = mtrl.diffuse.g = mtrl.diffuse.b = 0.1f;
						mtrl.ambient.r = mtrl.ambient.g = mtrl.ambient.b = 1.0f;
						g_xMainWnd.Get3DDevice()->SetMaterial(&mtrl);

						ResetBlendenderState(g_xMainWnd.Get3DDevice());
						g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorldOriginal);

						m_stMapItemList.MoveNextNode();
					}
				}				
			}
			g_xMainWnd.Get3DDevice()->EndScene();
			return S_OK;
		}
	}
	return E_FAIL;


}



VOID CGameProcess::RenderObject(INT nLoopTime)
{
	INT	 nStartX, nStartY, nObjFileIdx, nImgIdx, nArrayNum, nObjCnt;
	BOOL bFocused = FALSE;
	BOOL bLongObj = FALSE;
	BOOL bBlend	  = FALSE;
	BOOL bDrawShadow = TRUE;

	m_xMap.SetAniTileFrame(nLoopTime);

// 绘制 48*32 图块型对象。
	for ( INT nYCnt = m_xMap.m_shStartViewTileY-4; nYCnt < m_xMap.m_shStartViewTileY+_VIEW_CELL_Y_COUNT+6; nYCnt++ )
	{
		for ( INT nXCnt = m_xMap.m_shStartViewTileX-4; nXCnt < m_xMap.m_shStartViewTileX+_VIEW_CELL_X_COUNT+6; nXCnt++ )
		{
			if ( nYCnt >= m_xMap.m_stMapFileHeader.shHeight )
				break;

			if ( nXCnt >= m_xMap.m_stMapFileHeader.shWidth || nXCnt < 0 || nYCnt < 0 )
				continue; 

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Test : 为了光源，再多取 -4 到 +5 的区域。

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			nArrayNum = nYCnt + nXCnt*m_xMap.m_stMapFileHeader.shHeight;

			// 存在对象时.
			if ( m_xMap.m_pstCellInfo[nArrayNum].bFileIdx != 255 )
			{
				// 存在对象1时.
				if ( m_xMap.m_pstCellInfo[nArrayNum].wObj1 != 65535 )
				{
					nObjFileIdx = m_xMap.m_pstCellInfo[nArrayNum].bObjectFile1;
					nImgIdx	= m_xMap.m_pstCellInfo[nArrayNum].wObj1;

					if ( nObjFileIdx > 2 && nObjFileIdx < 14 )
					{
						if ( m_xMap.m_pxTileImg[nObjFileIdx]->NewSetIndex(nImgIdx) )
						{
							if ( m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shWidth == 48 &&
  								 m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight == 32 )
							{
								bBlend = FALSE;
								
								nImgIdx += m_xMap.GetDoorImgIdx(nXCnt, nYCnt);

								// 如果是动画对象.
								if ( m_xMap.m_pstCellInfo[nArrayNum].bObj1Ani != 255 )
								{
									BYTE  bTickType;
									SHORT shAniCnt;
									
									bTickType = (m_xMap.m_pstCellInfo[nArrayNum].bObj1Ani & 0X70) >> 4;
									shAniCnt = m_xMap.m_pstCellInfo[nArrayNum].bObj1Ani & 0X0F;
									
									if ( (m_xMap.m_pstCellInfo[nArrayNum].bObj1Ani & 0X80) >> 7 )
										bBlend = TRUE;

									nImgIdx += m_xMap.m_bAniTileFrame[bTickType][shAniCnt];
									m_xMap.m_pxTileImg[nObjFileIdx]->NewSetIndex(nImgIdx);
								}
						
								if ( !bBlend )
									g_xMainWnd.DrawWithImageForCompClipRgn(
									(nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX,
									(nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight+_CELL_HEIGHT-m_xMap.m_shViewOffsetY,
									m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shWidth,
									m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight,
									(WORD*)m_xMap.m_pxTileImg[nObjFileIdx]->m_pbCurrImage,
									_CLIP_WIDTH, _CLIP_HEIGHT);
								else
								{
									m_xImage.AddTextr(_TEXTR_FILE_MAP, nObjFileIdx, nImgIdx);
									LPDIRECTDRAWSURFACE7 lpddsTextr = m_xImage.GetTextrImg(_TEXTR_FILE_MAP, nObjFileIdx, nImgIdx);

									nStartX = (nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX;
									nStartY = -(nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight+_CELL_HEIGHT-m_xMap.m_shViewOffsetY;

									D3DMATERIAL7 mtrl;
									D3DVECTOR	vecTrans(nStartX, nStartY, 0);
									D3DVECTOR	vecScale(m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shWidth, m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight, 1);

									D3DUtil_InitMaterial(mtrl, (FLOAT)255/255.0f, (FLOAT)255/255.0f, (FLOAT)255/255.0f);
									mtrl.diffuse.a = 1.0f/255.0f;
									g_xMainWnd.Get3DDevice()->SetMaterial(&mtrl);
									m_xImage.DrawBillBoard(g_xMainWnd.Get3DDevice(), vecTrans, vecScale, mtrl, lpddsTextr);
								}
							}
						}
					}
				}
				// 存在对象2时。
				if ( m_xMap.m_pstCellInfo[nArrayNum].wObj2 != 65535 )
				{
					nObjFileIdx = m_xMap.m_pstCellInfo[nArrayNum].bObjectFile2;
					nImgIdx		= m_xMap.m_pstCellInfo[nArrayNum].wObj2;

					if ( nObjFileIdx > 2 && nObjFileIdx < 14 )
					{
						if ( m_xMap.m_pxTileImg[nObjFileIdx]->NewSetIndex(nImgIdx) )
						{
							if ( m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shWidth == 48 &&
								 m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight == 32 )
							{
								bBlend = FALSE;
								
								nImgIdx += m_xMap.GetDoorImgIdx(nXCnt, nYCnt);

								// 如果是动画对象.
								if ( m_xMap.m_pstCellInfo[nArrayNum].bObj2Ani != 255 )
								{
									BYTE  bTickType;
									SHORT shAniCnt;
									
									bTickType = (m_xMap.m_pstCellInfo[nArrayNum].bObj2Ani & 0X70) >> 4;
									shAniCnt = m_xMap.m_pstCellInfo[nArrayNum].bObj2Ani & 0X0F;
									
									if ( (m_xMap.m_pstCellInfo[nArrayNum].bObj2Ani & 0X80) >> 7 )
										bBlend = TRUE;

									nImgIdx += m_xMap.m_bAniTileFrame[bTickType][shAniCnt];
									m_xMap.m_pxTileImg[nObjFileIdx]->NewSetIndex(nImgIdx);
								}
								
								if ( !bBlend )
									g_xMainWnd.DrawWithImageForCompClipRgn(
									(nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX,
									(nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight+_CELL_HEIGHT-m_xMap.m_shViewOffsetY,
									m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shWidth,
									m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight,
									(WORD*)m_xMap.m_pxTileImg[nObjFileIdx]->m_pbCurrImage,
									_CLIP_WIDTH, _CLIP_HEIGHT);
								else
								{
									m_xImage.AddTextr(_TEXTR_FILE_MAP, nObjFileIdx, nImgIdx);
									LPDIRECTDRAWSURFACE7 lpddsTextr = m_xImage.GetTextrImg(_TEXTR_FILE_MAP, nObjFileIdx, nImgIdx);

									nStartX = (nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX;
									nStartY = -(nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight+_CELL_HEIGHT-m_xMap.m_shViewOffsetY;

									D3DMATERIAL7 mtrl;
									D3DVECTOR	vecTrans(nStartX, nStartY, 0);
									D3DVECTOR	vecScale(m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shWidth, m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight, 1);

									D3DUtil_InitMaterial(mtrl, (FLOAT)255/255.0f, (FLOAT)255/255.0f, (FLOAT)255/255.0f);
									mtrl.diffuse.a = 1.0f/255.0f;
									g_xMainWnd.Get3DDevice()->SetMaterial(&mtrl);
									m_xImage.DrawBillBoard(g_xMainWnd.Get3DDevice(), vecTrans, vecScale, mtrl, lpddsTextr);
								}
							}
						}
					}
				}
			}
		}
	}	

	// 绘制长度不是 48*32 的图块型对象.
	for ( nYCnt = m_xMap.m_shStartViewTileY; nYCnt < m_xMap.m_shStartViewTileY+_VIEW_CELL_Y_COUNT_FOR_OBJ; nYCnt++ )
	{
		for ( INT nXCnt = m_xMap.m_shStartViewTileX; nXCnt < m_xMap.m_shStartViewTileX+_VIEW_CELL_X_COUNT; nXCnt++ )
		{
			if ( nYCnt >= m_xMap.m_stMapFileHeader.shHeight )
				break;
			if ( nXCnt >= m_xMap.m_stMapFileHeader.shWidth || nXCnt < 0 || nYCnt < 0 )
				continue; 

			bLongObj = FALSE;

			nArrayNum = nYCnt + nXCnt*m_xMap.m_stMapFileHeader.shHeight;

			// 存在对象时.
			if ( m_xMap.m_pstCellInfo[nArrayNum].bFileIdx != 255 )
			{
				// 存在对象1时.
				if ( m_xMap.m_pstCellInfo[nArrayNum].wObj1 != 65535 )
				{
					nObjFileIdx = m_xMap.m_pstCellInfo[nArrayNum].bObjectFile1;
					nImgIdx	= m_xMap.m_pstCellInfo[nArrayNum].wObj1;	
	
					if ( nObjFileIdx > 2 && nObjFileIdx < 14 )
					{
						if ( m_xMap.m_pxTileImg[nObjFileIdx]->NewSetIndex(nImgIdx) )
						{
							if ( !(m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shWidth == 48 &&
								   m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight == 32) )
							{
								bBlend = FALSE;
								
								nImgIdx += m_xMap.GetDoorImgIdx(nXCnt, nYCnt);

								// 如果是动画对象.
								if ( m_xMap.m_pstCellInfo[nArrayNum].bObj1Ani != 255 )
								{
									BYTE  bTickType;
									SHORT shAniCnt;
									
									bTickType = (m_xMap.m_pstCellInfo[nArrayNum].bObj1Ani & 0X70) >> 4;
									shAniCnt = m_xMap.m_pstCellInfo[nArrayNum].bObj1Ani & 0X0F;
									
									if ( (m_xMap.m_pstCellInfo[nArrayNum].bObj1Ani & 0X80) >> 7 )
										bBlend = TRUE;

									nImgIdx += m_xMap.m_bAniTileFrame[bTickType][shAniCnt];
									m_xMap.m_pxTileImg[nObjFileIdx]->NewSetIndex(nImgIdx);
								}
								
								if ( !bBlend )
									g_xMainWnd.DrawWithImageForCompClipRgn(
									(nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX,
									(nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight+_CELL_HEIGHT-m_xMap.m_shViewOffsetY,
									m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shWidth,
									m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight,
									(WORD*)m_xMap.m_pxTileImg[nObjFileIdx]->m_pbCurrImage,
									_CLIP_WIDTH, _CLIP_HEIGHT);
								else
								{
									m_xImage.AddTextr(_TEXTR_FILE_MAP, nObjFileIdx, nImgIdx);
									LPDIRECTDRAWSURFACE7 lpddsTextr = m_xImage.GetTextrImg(_TEXTR_FILE_MAP, nObjFileIdx, nImgIdx);

									nStartX = (nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX+m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shPX;
									nStartY = (nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight+_CELL_HEIGHT-m_xMap.m_shViewOffsetY+m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shPY;

									D3DMATERIAL7 mtrl;
									D3DVECTOR	vecTrans(nStartX, nStartY, 0);
									D3DVECTOR	vecScale(m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shWidth, m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight, 1);

									D3DUtil_InitMaterial(mtrl, (FLOAT)255/255.0f, (FLOAT)255/255.0f, (FLOAT)255/255.0f);
									mtrl.diffuse.a = 0.0f/255.0f;
									g_xMainWnd.Get3DDevice()->SetMaterial(&mtrl);
									m_xImage.DrawBillBoard(g_xMainWnd.Get3DDevice(), vecTrans, vecScale, mtrl, lpddsTextr);
								}
	
								bLongObj = TRUE;
							}
						}
					}
				}
				// 存在对象2时。
				if ( m_xMap.m_pstCellInfo[nArrayNum].wObj2 != 65535 )
				{
					nObjFileIdx = m_xMap.m_pstCellInfo[nArrayNum].bObjectFile2;
					nImgIdx		= m_xMap.m_pstCellInfo[nArrayNum].wObj2;

					if ( nObjFileIdx > 2 && nObjFileIdx < 14 )
					{
						if ( m_xMap.m_pxTileImg[nObjFileIdx]->NewSetIndex(nImgIdx) )
						{
							if ( !(m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shWidth == 48 &&
								 m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight == 32) )
							{
								bBlend = FALSE;
								
								nImgIdx += m_xMap.GetDoorImgIdx(nXCnt, nYCnt);

								// 如果是动画对象.
								if ( m_xMap.m_pstCellInfo[nArrayNum].bObj2Ani != 255 )
								{
									BYTE  bTickType;
									SHORT shAniCnt;
									
									bTickType = (m_xMap.m_pstCellInfo[nArrayNum].bObj2Ani & 0X70) >> 4;
									shAniCnt = m_xMap.m_pstCellInfo[nArrayNum].bObj2Ani & 0X0F;
									
//									if ( (m_xMap.m_pstCellInfo[nArrayNum].bObj2Ani & 0X80) >> 7 )
//										bBlend = TRUE;

									nImgIdx += m_xMap.m_bAniTileFrame[bTickType][shAniCnt];
									m_xMap.m_pxTileImg[nObjFileIdx]->NewSetIndex(nImgIdx);
								}

								if ( !bBlend )
									g_xMainWnd.DrawWithImageForCompClipRgn(
												(nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX,
												(nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight+_CELL_HEIGHT-m_xMap.m_shViewOffsetY,
												m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shWidth,
												m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight,
												(WORD*)m_xMap.m_pxTileImg[nObjFileIdx]->m_pbCurrImage,
												_CLIP_WIDTH, _CLIP_HEIGHT);
								else
								{
									m_xImage.AddTextr(_TEXTR_FILE_MAP, nObjFileIdx, nImgIdx);
									LPDIRECTDRAWSURFACE7 lpddsTextr = m_xImage.GetTextrImg(_TEXTR_FILE_MAP, nObjFileIdx, nImgIdx);

									nStartX = (nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX;
									nStartY = -(nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight+_CELL_HEIGHT-m_xMap.m_shViewOffsetY;

									D3DMATERIAL7 mtrl;
									D3DVECTOR	vecTrans(nStartX, nStartY, 0);
									D3DVECTOR	vecScale(m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shWidth, m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight, 1);

									D3DUtil_InitMaterial(mtrl, (FLOAT)255/255.0f, (FLOAT)255/255.0f, (FLOAT)255/255.0f);
									mtrl.diffuse.a = 1.0f/255.0f;
									g_xMainWnd.Get3DDevice()->SetMaterial(&mtrl);
									m_xImage.DrawBillBoard(g_xMainWnd.Get3DDevice(), vecTrans, vecScale, mtrl, lpddsTextr);
								}
								
   								bLongObj = TRUE;
							}
						}
					}
				}
			}

//			m_xMap.DrawOpenDoor(nXCnt, nYCnt);

			INT	 nMapViewX		= 0;
			INT	 nMapViewY		= 0;
			BOOL bIsMapViewTime = FALSE;
			if ( nXCnt >= m_xMap.m_shStartViewTileX && nXCnt < m_xMap.m_shStartViewTileX + _VIEW_CELL_X_COUNT && 
				 nYCnt >= m_xMap.m_shStartViewTileY && nYCnt < m_xMap.m_shStartViewTileY + _VIEW_CELL_Y_COUNT )
			{
				bIsMapViewTime = TRUE;
				nMapViewX = nXCnt - m_xMap.m_shStartViewTileX;
				nMapViewY = nYCnt - m_xMap.m_shStartViewTileY;
			}
	
			// 绘制地面物品。
			if ( bIsMapViewTime )
			{
				for ( nObjCnt = 0; nObjCnt < GroundItemDisplay::CellCapacity; nObjCnt++ )
				{
					LPGROUNDITEM pstItem = m_stMemMappedID[nMapViewY][nMapViewX].pstItem[nObjCnt];
					if ( pstItem )
					{
						CWHWilImageData* pxItemImg = &g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_GROUND];
						if ( pxItemImg->NewSetIndex(pstItem->wLooks) )
						{
							nStartX = (pstItem->shTileX-m_xMap.m_shStartViewTileX)*_CELL_WIDTH+_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX;
							nStartY = (pstItem->shTileY-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_shViewOffsetY;
							INT nX  = nStartX+_CELL_WIDTH/2-pxItemImg->m_lpstNewCurrWilImageInfo->shWidth/2;
							INT nY  = nStartY+_CELL_HEIGHT/2-pxItemImg->m_lpstNewCurrWilImageInfo->shHeight/2;


                            const bool highlighted=nXCnt==m_ptCurrTile.x && nYCnt==m_ptCurrTile.y && !m_xNotice.IsActive() && m_xInterface.SpellPointerAvailable();
                            DrawSelectionFrame(*pxItemImg,pstItem->wLooks,nX,nY,highlighted?SelectionBlend::ItemHighlight:SelectionBlend::Normal,false);


						}
					}
					else
						break;
				}
			}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 绘制自己的角色.
			if ( nXCnt == m_xMyHero.m_wPosX && nYCnt == m_xMyHero.m_wPosY )
			{
				// 传入图像的起始坐标、焦点状态、阴影混合状态。
				if ( PtInRect(&m_xMyHero.m_rcActor, m_ptMousePos) && !m_pxMouseTargetActor )
				{
					bFocused			 = TRUE;
					m_pxMouseTargetActor = &m_xMyHero;
				}
				else
					bFocused			 = FALSE;

				bDrawShadow = TRUE;
				if ( m_bShadowABlend )
					bDrawShadow = FALSE;

				m_xMyHero.DrawActor(bFocused, m_bShadowABlend, FALSE, bDrawShadow);
			} 

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 绘制其他角色。
			if ( bIsMapViewTime )
			{
				for ( nObjCnt = 0; nObjCnt < _MAX_CELL_OBJ; nObjCnt++ )
				{
					CActor* pxActor = m_stMemMappedID[nMapViewY][nMapViewX].pxActor[nObjCnt];

					if ( pxActor )
					{
						BYTE bOpa = 50;

						if ( pxActor->m_bIsDead )
							bOpa = 30;

						if ( pxActor == m_pxMouseTargetActor )
							bFocused = TRUE;
						else
							bFocused = FALSE;

						bDrawShadow = TRUE;
						if ( pxActor == m_pxMouseTargetActor && m_bShadowABlend )
							bDrawShadow = FALSE;

						switch ( pxActor->m_stFeature.bGender )
						{
						case _GENDER_MAN:
						case _GENDER_WOMAN:
							{
								CHero* pxHero;
								pxHero = (CHero*)pxActor;

								pxHero->m_drawnThisFrame |= pxHero->DrawActor(&m_xMap, bFocused, m_bShadowABlend, TRUE, bDrawShadow);
							}
							break;
						case _GENDER_NPC:
							{
								CNPC* pxNPC;
								pxNPC = (CNPC*)pxActor;

								pxNPC->m_drawnThisFrame |= pxNPC->DrawActor(&m_xMap, bFocused, m_bShadowABlend, TRUE, bDrawShadow);
							}
							break;
						case _GENDER_MON:
							{
								pxActor->m_drawnThisFrame |= pxActor->DrawActor(&m_xMap, bFocused, m_bShadowABlend, TRUE, bDrawShadow);

/*								// 马匹时.
								RECT rc, rcIntersect;
//								if ( bLongObj && pxActor->m_wPosY <= nYCnt && pxActor->m_stFeature.bDress > 114 && pxActor->m_stFeature.bDress < 118 )
								{
									SetRect(&rc,
									(nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX,
									(nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight+_CELL_HEIGHT-m_xMap.m_shViewOffsetY,
									m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shWidth  + (nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX,
									m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight + (nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight+_CELL_HEIGHT-m_xMap.m_shViewOffsetY);

									// 马与背景.
									if ( IntersectRect(&rcIntersect, &rc, &pxActor->m_rcActor) )
									{
										m_xImage.m_xImageList[pxActor->m_bActorImgIdx].NewSetIndex(pxActor->m_dwCurrFrame);
 										g_xMainWnd.DrawWithABlendForIntersectCompData(
														pxActor->m_rcActor.left,
														pxActor->m_rcActor.top,
														m_xImage.m_xImageList[pxActor->m_bActorImgIdx].m_lpstNewCurrWilImageInfo->shWidth,
														m_xImage.m_xImageList[pxActor->m_bActorImgIdx].m_lpstNewCurrWilImageInfo->shHeight,
														(WORD*)m_xImage.m_xImageList[pxActor->m_bActorImgIdx].m_pbCurrImage,
														rc.left,
														rc.top,
														m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shWidth,
														m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight,
														(WORD*)m_xMap.m_pxTileImg[nObjFileIdx]->m_pbCurrImage,
														_CLIP_WIDTH, _CLIP_HEIGHT, 50, bFocused, 0XFFFF, 0XFFFF, 0XFFFF, 0XFFFF, m_xMyHero.m_wStateClr);
									}
								}
*/							}
							break;
						}
						
						bFocused = FALSE;
					}
					else
						break;
				}
			}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if ( bIsMapViewTime )
			{
				for ( nObjCnt = 0; nObjCnt < _MAX_CELL_OBJ; nObjCnt++ )
				{
					CMagic* pxMagic = m_stMemMappedID[nMapViewY][nMapViewX].pxMagic[nObjCnt];
					if ( pxMagic )
					{
						if ( pxMagic->m_bMagicNum == _SKILL_HOLYSHIELD || pxMagic->m_bMagicNum == _SKILL_EARTHFIRE )
						{
							CRepeatMagic* pxRepeatMagic = (CRepeatMagic*)pxMagic;

							pxRepeatMagic->DrawMagic();
						}
						else if ( pxMagic->m_bMagicNum == _SKILL_FIRE )
						{
							CMagicStream* pxMagicStream = (CMagicStream*)pxMagic;

							pxMagicStream->DrawMagic();
						}
					}
					else
						break;
				}
			}	
		}
	}		


	bDrawShadow = FALSE;
	if ( m_bShadowABlend )
		bDrawShadow = TRUE;

	m_xMyHero.m_wABlendDelay	 = 10;
	m_xMyHero.m_wABlendCurrDelay = 5;

	m_xMyHero.DrawActor(FALSE, m_bShadowABlend, FALSE, bDrawShadow);
	
	m_xMyHero.m_wABlendDelay	 = 0;
	m_xMyHero.m_wABlendCurrDelay = 0;

	if ( m_pxMouseTargetActor && m_pxMouseTargetActor != &m_xMyHero /*&& !m_pxMouseTargetActor->m_bIsDead*/ )
	{
		m_pxMouseTargetActor->m_wABlendDelay	 = 10;
		m_pxMouseTargetActor->m_wABlendCurrDelay = 4;

		m_pxMouseTargetActor->DrawActor(&m_xMap, FALSE, m_bShadowABlend, FALSE, bDrawShadow);
		
		m_pxMouseTargetActor->m_wABlendDelay	 = 0;
		m_pxMouseTargetActor->m_wABlendCurrDelay = 0;
	}
}









/******************************************************************************************************************

	消息相关函数.

*******************************************************************************************************************/
LRESULT CGameProcess::OnDestroy(WPARAM wParam, LPARAM lParam)
{
	m_xInterface.m_xInventoryWnd.SaveItemPos(m_xMyHero.m_szName);
	return 0L;
}

LRESULT CGameProcess::OnTimer(WPARAM wParam, LPARAM lParam)
{
    if(wParam==_TIMER_ID) {
        KillTimer(g_xMainWnd.GetSafehWnd(),_TIMER_ID);
    }
	else	// Temp
	{
		if (PacketQ.GetCount())
		{
			char *pszPacket = (char*)PacketQ.PopQ();

			if (pszPacket)
			{
				strcpy(&szBuff[nBuff], pszPacket);

				char *pszRemain = OnMessageReceive(szBuff);

				if (pszRemain)
				{
					nBuff = strlen(pszRemain);
					memcpy(szBuff, pszRemain, nBuff);
					szBuff[nBuff] = '\0';
				}
				else
				{
					nBuff = 0;
					ZeroMemory(szBuff, sizeof(szBuff));
				}

				delete [] pszPacket;
			}
		}

		ProcessPacket();
		ProcessDefaultPacket();
	}

	return 0L;
}


LRESULT CGameProcess::OnLButtonDoubleClick(WPARAM wParam, LPARAM lParam)
{
    if(m_xNotice.IsActive()) return 0L;
	m_ptMousePos.x = LOWORD(lParam);
	m_ptMousePos.y = HIWORD(lParam);

	m_xInterface.OnLButtonDoubleClick(m_ptMousePos);

	return 0L;
}


LRESULT CGameProcess::OnWheel(WPARAM wParam, LPARAM lParam)
{
    if(m_xNotice.IsActive()) return 0L;
	INT nDelta;
	nDelta = (short)HIWORD(wParam);
	if ( nDelta > 0 )
		m_xInterface.OnScrollDown();
	else if ( nDelta < 0 )
		m_xInterface.OnScrollUp();

	return 0L;
}


LRESULT CGameProcess::OnSysKeyDown(WPARAM wParam, LPARAM lParam)
{
    if((lParam & (1L<<29)) && (wParam=='Q' || wParam=='X')) {
        if(!(lParam & (1L<<30)) && !m_xNotice.IsActive() && !m_xInterface.m_xMsgBox.IsActive() && !m_xInterface.m_xGuildEditor.m_bActive)
            m_xInterface.OpenExitDialog(wParam=='Q'?ExitFlow::ProgramDialog:ExitFlow::CharacterDialog);
        return 0L;
    }
	if ( wParam == VK_RETURN )
	{
		m_bRender = FALSE;
		g_xMainWnd.OnSysKeyDown(wParam, lParam);
		SetFocus(g_xChatEditBox.GetSafehWnd());
		SetFocus(g_xMainWnd.GetSafehWnd());

		for ( INT nCnt = 0; nCnt < 10; nCnt++)
		{
			if ( m_xImage.m_xImageList[_IMAGE_PROGUSE].NewSetIndex(nCnt+40) )
			{
				D3DWILTextr_CreateEmptyTexture(m_xImage.m_xImageList[_IMAGE_PROGUSE].m_szWilFileName, nCnt+40, 
											   m_xImage.m_xImageList[_IMAGE_PROGUSE].m_lpstNewCurrWilImageInfo->shWidth,
											   m_xImage.m_xImageList[_IMAGE_PROGUSE].m_lpstNewCurrWilImageInfo->shHeight,
											   (WORD*)m_xImage.m_xImageList[_IMAGE_PROGUSE].m_pbCurrImage, D3DTEXTR_TRANSPARENTBLACK);
				m_lpddsShineTextr[nCnt] = D3DWILTextr_RestoreEx(m_xImage.m_xImageList[_IMAGE_PROGUSE].m_szWilFileName, nCnt+40, g_xMainWnd.Get3DDevice());
			}
		}
		m_xImage.DelAllTextrFileList();

		m_bRender = TRUE;
	}

	if ( wParam == VK_F10 )
	{
		return OnKeyDown(wParam,lParam);
	}
	return 0L;
}


LRESULT CGameProcess::OnSysKeyUp(WPARAM wParam, LPARAM lParam)
{
	return 0L;
}


LRESULT CGameProcess::OnMove(WPARAM wParam, LPARAM lParam)
{
    g_xMainWnd.OnMove(wParam,lParam);
    g_xChatEditBox.Relocate();
    return 0L;
}


LRESULT CGameProcess::OnMouseMove(WPARAM wParam, LPARAM lParam)
{	
	m_ptMousePos.x = LOWORD(lParam);
	m_ptMousePos.y = HIWORD(lParam);
	m_ptCurrTile.x = m_xMap.m_shStartViewTileX + (m_ptMousePos.x - _VIEW_CELL_X_START + m_xMap.m_shViewOffsetX) / TILE_WIDTH;
	m_ptCurrTile.y = m_xMap.m_shStartViewTileY + (m_ptMousePos.y - _VIEW_CELL_Y_START + m_xMap.m_shViewOffsetY) / TILE_HEIGHT;

	if(m_xNotice.IsActive())
	{
		m_xNotice.MoveWnd(m_ptMousePos);
	}
	else
	{
		m_xInterface.OnMouseMove(m_ptMousePos);
	}
	return 0L;
}


LRESULT CGameProcess::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{	
	POINT ptTaretTilePos;

	// 游戏数据是否已加载...
	if(m_xNotice.IsActive())
	{
        m_ptMousePos=POINT{LOWORD(lParam),HIWORD(lParam)};
		m_xNotice.OnButtonDown(m_ptMousePos);
        return 0L;
	}

	if ( !m_bRender )			return 0L;

	m_pxSavedTargetActor = NULL;

	m_ptMousePos.x = LOWORD(lParam);
	m_ptMousePos.y = HIWORD(lParam);
	m_ptCurrTile.x = m_xMap.m_shStartViewTileX + (m_ptMousePos.x - _VIEW_CELL_X_START + m_xMap.m_shViewOffsetX) / TILE_WIDTH;
	m_ptCurrTile.y = m_xMap.m_shStartViewTileY + (m_ptMousePos.y - _VIEW_CELL_Y_START + m_xMap.m_shViewOffsetY) / TILE_HEIGHT;

	if ( !m_xInterface.OnLButtonDown(m_ptMousePos) )
	{
		// 界面中没有要做的事情时.

		// 1. 掉落物品。
		if ( m_xInterface.m_stCommonItem.bSetted && !m_xInterface.m_stCommonItem.bWaitResponse && !m_xInterface.m_stCommonItem.bIsEquipItem && m_xInterface.m_xQuickWnd.heldSlot<0 && m_xInterface.m_xExchangeWnd.pendingSlot<0 )
		{
			g_xClientSocket.SendItemIndex(CM_DROPITEM, m_xInterface.m_stCommonItem.xItem.m_stItemInfo.nMakeIndex, m_xInterface.m_stCommonItem.xItem.m_stItemInfo.stStdItem.szName);
			m_xInterface.m_stCommonItem.bWaitResponse = TRUE;
			return 0L;
		}

		// 2. 拾取地图物品.
		if ( m_stMapItemList.GetCounter() != 0 )
		{
			m_stMapItemList.MoveCurrentToTop();
			LPGROUNDITEM pstItem;

			for ( INT nCnt = 0; nCnt < m_stMapItemList.GetCounter(); nCnt++ )
			{
				pstItem = m_stMapItemList.GetCurrentData();

				if ( pstItem->shTileX == m_ptCurrTile.x && pstItem->shTileY == m_ptCurrTile.y && pstItem->shTileX == m_xMyHero.m_wPosX && pstItem->shTileY == m_xMyHero.m_wPosY )
				{
					g_xClientSocket.SendPickUp(pstItem->shTileX, pstItem->shTileY);
					return 0L;
				}
				m_stMapItemList.MoveNextNode();
			}
		}

		if ( m_pxMouseTargetActor )
		{
			if ( !m_pxMouseTargetActor->m_bIsDead )
			{
				// 确定攻击目标 Actor。
				m_pxSavedTargetActor = m_pxMouseTargetActor;
			}

			ptTaretTilePos.x = m_pxMouseTargetActor->m_wPosX;
			ptTaretTilePos.y = m_pxMouseTargetActor->m_wPosY;
			if ( m_pxMouseTargetActor->m_stFeature.bGender == _GENDER_NPC )
			{
				g_xClientSocket.SendNPCClick(m_pxMouseTargetActor->m_dwIdentity);
				m_pxSavedTargetActor = NULL;
			}
			else
			{
				m_xMyHero.OnLButtonDown(m_ptMousePos, m_pxMouseTargetActor->m_dwIdentity, m_pxMouseTargetActor->m_bIsDead, &ptTaretTilePos);
			}
		}
		else
		{
			m_xMyHero.OnLButtonDown(m_ptMousePos);
		}
	}

	return 0L;
}


LRESULT CGameProcess::OnRButtonDown(WPARAM wParam, LPARAM lParam)
{
    if(m_xNotice.IsActive()) return 0L;
	m_ptMousePos.x = LOWORD(lParam);
	m_ptMousePos.y = HIWORD(lParam);
	m_ptCurrTile.x = m_xMap.m_shStartViewTileX + (m_ptMousePos.x - _VIEW_CELL_X_START + m_xMap.m_shViewOffsetX) / TILE_WIDTH;
	m_ptCurrTile.y = m_xMap.m_shStartViewTileY + (m_ptMousePos.y - _VIEW_CELL_Y_START + m_xMap.m_shViewOffsetY) / TILE_HEIGHT;

	m_pxSavedTargetActor = NULL;

	m_xMyHero.OnRButtonDown(m_ptMousePos);

	return 0L;
}


LRESULT CGameProcess::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	m_ptMousePos.x = LOWORD(lParam);
	m_ptMousePos.y = HIWORD(lParam);
	m_ptCurrTile.x = m_xMap.m_shStartViewTileX + (m_ptMousePos.x - _VIEW_CELL_X_START + m_xMap.m_shViewOffsetX) / TILE_WIDTH;
	m_ptCurrTile.y = m_xMap.m_shStartViewTileY + (m_ptMousePos.y - _VIEW_CELL_Y_START + m_xMap.m_shViewOffsetY) / TILE_HEIGHT;
	if(m_xNotice.IsActive())
	{
		if(m_xNotice.OnButtonUp(m_ptMousePos))
		{
			// OK Button Click
			m_xNotice.HideMessageBox();
			Clear(RGB(0,0,0));
			// Start Game From Here
		}
	}
	else
		m_xInterface.OnLButtonUp(m_ptMousePos);

	return 0L;
}


LRESULT CGameProcess::OnRButtonUp(WPARAM wParam, LPARAM lParam)
{
    if(m_xNotice.IsActive()) return 0L;
	m_ptMousePos.x = LOWORD(lParam);
	m_ptMousePos.y = HIWORD(lParam);
	m_ptCurrTile.x = m_xMap.m_shStartViewTileX + (m_ptMousePos.x - _VIEW_CELL_X_START + m_xMap.m_shViewOffsetX) / TILE_WIDTH;
	m_ptCurrTile.y = m_xMap.m_shStartViewTileY + (m_ptMousePos.y - _VIEW_CELL_Y_START + m_xMap.m_shViewOffsetY) / TILE_HEIGHT;

	return 0L;
}


LRESULT CGameProcess::OnKeyUp(WPARAM wParam, LPARAM lParam)
{
	m_xInterface.OnKeyUp(wParam, lParam);
	return 0L;
}


LRESULT CGameProcess::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    if(m_xNotice.IsActive()) {
        if(m_xNotice.OnKeyDown(wParam)) m_xNotice.HideMessageBox();
        return 0L;
    }
    if(m_xInterface.OnKeyDown(wParam,lParam)) return 0L;
	INT		nTargetID;
	POINT	ptTaretTilePos;
	FEATURE stFeature;

	D3DMATRIX matTrans;
	D3DMATRIX matScale;
	D3DMATRIX matRot;
	D3DMATRIX matView;
	D3DMATRIX matTemp;
	FLOAT	  fRot = 0.0f;

	D3DVECTOR vTrans  = D3DVECTOR(0.0f, 0.0f, 0.0f);
	D3DVECTOR vRotate = D3DVECTOR(0.0f, 0.0f, 0.0f);
	D3DVECTOR vScale  = D3DVECTOR(1.0f, 1.0f, 1.0f);

	g_xMainWnd.Get3DDevice()->GetTransform(D3DTRANSFORMSTATE_VIEW, &matView);

	if ( m_pxMouseTargetActor )	
	{
		nTargetID			= m_pxMouseTargetActor->m_dwIdentity;
		ptTaretTilePos.x	= m_pxMouseTargetActor->m_wPosX;
		ptTaretTilePos.y	= m_pxMouseTargetActor->m_wPosY;
		stFeature			= m_pxMouseTargetActor->m_stFeature;
	}
	else
	{
		nTargetID			= 0;
		ptTaretTilePos.x	= 0;
		ptTaretTilePos.y	= 0;
		ZeroMemory(&stFeature, sizeof(FEATURE));
	}

	m_xMyHero.OnKeyDown(wParam, lParam, m_ptMousePos, ptTaretTilePos, nTargetID, stFeature);

	switch ( wParam )
	{
	case 'L':
		g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME);
		break;
	case 'M':
		g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
		break;
	case 'N':
		{
			SetNextProc();
			break;
		}
	case '6':
		{
/*			g_xSound.PlayActorWav(m_ptCurrTile.x, m_ptCurrTile.y, m_xMyHero.m_wPosX, m_xMyHero.m_wPosY, 1);
*/			break;
		}
	case '7':
		{
/*			g_xSound.StopSound(1);
*/			break;
		}
	case VK_SPACE:
	case VK_RETURN:
		{
            m_xInterface.FocusChatMode();
		}
		break;
    case 'U':
        EnterSceneAmbient((m_sceneAmbient.mode+1)%3);
        break;
	case VK_LEFT:
		{
			vTrans.x = -2.0f;
			D3DUtil_SetTranslateMatrix(matTrans, vTrans);
			D3DMath_MatrixMultiply(matTemp, matView, matTrans);
			g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_VIEW, &matTemp);
		}
		break;
	case VK_RIGHT:
		{
			vTrans.x = 2.0f;
			D3DUtil_SetTranslateMatrix(matTrans, vTrans);
			D3DMath_MatrixMultiply(matTemp, matView, matTrans);
			g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_VIEW, &matTemp);
		}
		break;
	case VK_UP:
		{
			vTrans.y = -2.0f;
			D3DUtil_SetTranslateMatrix(matTrans, vTrans);
			D3DMath_MatrixMultiply(matTemp, matView, matTrans);
			g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_VIEW, &matTemp);
		}
		break;
	case VK_DOWN:
		{
			vTrans.y = 2.0f;
			D3DUtil_SetTranslateMatrix(matTrans, vTrans);
			D3DMath_MatrixMultiply(matTemp, matView, matTrans);
			g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_VIEW, &matTemp);
		}
		break;
	case VK_INSERT:
		vScale.x = 1.01f;
		vScale.y = 1.01f;
		vScale.z = 1.00f;
		D3DUtil_SetScaleMatrix(matScale, vScale.x, vScale.y, vScale.z);
		D3DMath_MatrixMultiply(matTemp, matView, matScale);
		g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_VIEW, &matTemp);
		break;
	case VK_DELETE:
		vScale.x = 0.99f;
		vScale.y = 0.99f;
		vScale.z = 1.00f;
		D3DUtil_SetScaleMatrix(matScale, vScale.x, vScale.y, vScale.z);
		D3DMath_MatrixMultiply(matTemp, matView, matScale);
		g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_VIEW, &matTemp);
		break;
	case VK_NUMPAD1:
		fRot = 0.01f;
		D3DUtil_SetRotateYMatrix(matRot, fRot);
		D3DMath_MatrixMultiply(matTemp, matView, matRot);
		g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_VIEW, &matTemp);
		break;
	case VK_NUMPAD7:
		fRot = -0.01f;
		D3DUtil_SetRotateYMatrix(matRot, fRot);
		D3DMath_MatrixMultiply(matTemp, matView, matRot);
		g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_VIEW, &matTemp);
		break;
	case VK_NUMPAD4:
		fRot = 0.01f;
		D3DUtil_SetRotateZMatrix(matRot, fRot);
		D3DMath_MatrixMultiply(matTemp, matView, matRot);
		g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_VIEW, &matTemp);
		break;
	case VK_NUMPAD6:
		fRot = -0.01f;
		D3DUtil_SetRotateZMatrix(matRot, fRot);
		D3DMath_MatrixMultiply(matTemp, matView, matRot);
		g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_VIEW, &matTemp);
		break;
	case VK_NUMPAD2:
		fRot = 0.01f;
		D3DUtil_SetRotateXMatrix(matRot, fRot);
		D3DMath_MatrixMultiply(matTemp, matView, matRot);
		g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_VIEW, &matTemp);
		break;
	case VK_NUMPAD8:
		fRot = -0.01f;
		D3DUtil_SetRotateXMatrix(matRot, fRot);
		D3DMath_MatrixMultiply(matTemp, matView, matRot);
		g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_VIEW, &matTemp);
		break;
	}			

	return 0L;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 用户自定义消息相关函数。
LRESULT CGameProcess::OnMsgInputted(WPARAM wParam, LPARAM lParam)
{
	if ( m_xInterface.OnMsgInputted(wParam, lParam) )
	{
		return 0L;
	}

	if ( g_xChatEditBox.m_szInputMsg[0] == NULL )
		return 0L;
    static CLoginMessages chatCaptions;static const bool chatCaptionsLoaded=chatCaptions.Load("./CmList.dat");
    if(chatCaptions.Get(ChatCommandIds::AddGuildMember)==g_xChatEditBox.m_szInputMsg) {
        g_xChatEditBox.m_szInputMsg[0]=0;
        if(m_xInterface.m_xGuildWnd.m_manager) m_xInterface.OpenNameDialog(CM_GUILDADDMEMBER);
        else m_xInterface.ShowNotice(ChatCommandIds::AddGuildMemberDenied);
        return 0;
    }
    if(ChatShortcuts().Remember(g_xChatEditBox.m_szInputMsg)) {
        ZeroMemory(g_xChatEditBox.m_szInputMsg,sizeof(g_xChatEditBox.m_szInputMsg));return 0;
    }

	// 命令消息.
	if ( wParam )
	{
		CommandAnalyze();
	}

	CHAR	szCommand[MAX_PATH];
	CHAR	szArg1[MAX_PATH];
	CHAR	szArg2[MAX_PATH];
	CHAR	szArg3[MAX_PATH];
	CHAR	szArg4[MAX_PATH];
	ZeroMemory(szCommand, MAX_PATH);
	ZeroMemory(szArg1, MAX_PATH);
	ZeroMemory(szArg2, MAX_PATH);
	ZeroMemory(szArg3, MAX_PATH);
	ZeroMemory(szArg4, MAX_PATH);
	sscanf(g_xChatEditBox.m_szInputMsg, "%[^ ]%*c %[^ ]%*c %[^ ]%*c %[^ ]%*c %[^ ]%*c", szCommand, szArg1, szArg2, szArg3, szArg4);
	INT nArg1;	INT nArg2;	INT nArg3;	INT nArg4;
	nArg1 = nArg2 = nArg3 = nArg4 = 0;
	if ( !strcmp(szCommand, "@dye") )
	{
		nArg1 = atoi(szArg1);
		nArg2 = atoi(szArg2);
		nArg3 = atoi(szArg3);
		
		WORD wColor = g_xMainWnd.ConvertColor24To16(RGB(nArg3, nArg2, nArg1));

		sprintf(g_xChatEditBox.m_szInputMsg, "@dye %d", wColor );
		m_xMyHero.m_stFeatureEx.wDressColor = wColor;
	}	
	else if ( !strcmp(szCommand, "@speed") )
	{
		nArg1 = atoi(szArg1);
		m_xMyHero.m_shHitSpeed = nArg1;
	}	
	else if ( !strcmp(szCommand, "@npc") )
	{
		static int nNPC = 0;
		CActor* pxActor = NULL;

		nArg1 = atoi(szArg1);
		pxActor = new CNPC;
		CNPC* pxNPC = (CNPC*)pxActor;

		pxNPC->m_dwIdentity = nNPC;
		FEATURE stFeature;
		stFeature.bGender = _GENDER_NPC;
		stFeature.bDress = nArg1;
		stFeature.bHair = 0;
		stFeature.bWeapon = 0;

		if ( pxNPC->Create(&m_xImage, &stFeature, _MT_STAND, GetRandomNum(0, 7), m_xMyHero.m_wPosX, m_xMyHero.m_wPosY-1) )
			m_xActorList.AddNode(pxActor);

		pxNPC->m_wABlendCurrDelay = 0;
		pxNPC->m_wABlendDelay	  = 0;
		nNPC++;
	}	
	else if ( !strcmp(szCommand, "@light") )
	{
		m_fTileLightRed = atoi(szArg1);
		m_fTileLightGreen = atoi(szArg2);
		m_fTileLightBlue = atoi(szArg3);
	}	
	else if ( !strcmp(szCommand, "@timeshift") )
	{
		m_bDayState = atoi(szArg1);
		m_bFogState = atoi(szArg2);
        m_sceneAmbient.update(WORD(atoi(szArg2)));
	}	
/*	else if ( !strcmp(szCommand, "@刷怪点") )
	{
		CHAR szTmp[MAX_PATH];
		nArg2 = atoi(szArg2);
		sprintf(szTmp, "%s %d %d %s %d", szCommand, m_ptCurrTile.x, m_ptCurrTile.y, szArg1, nArg2);
		strcpy(g_xChatEditBox.m_szInputMsg, szTmp);
	}*/	
	else if ( !strcmp(szCommand, "@shadow") )
	{
        SetShadowBlend(!m_bShadowABlend);
        m_xInterface.m_xOptionWnd.SaveAudioOptions();

	}	
	else if ( !strcmp(szCommand, "@morph") )
	{		
		static FEATURE stLastCahrFeature;
		FEATURE stFeature;
		ZeroMemory(&stFeature, sizeof(FEATURE));

		if ( !m_xMyHero.m_bIsMon )	stLastCahrFeature = m_xMyHero.m_stFeature;

		if ( !strcmp(szArg1, "None") )
		{
			m_xMyHero.m_bIsMon = FALSE;
			stFeature = stLastCahrFeature;
			m_xMyHero.ChangeFeature(stFeature, m_xMyHero.m_stFeatureEx);
		}
		else
		{
			m_xMyHero.m_bIsMon = TRUE;
			stFeature.bGender = _GENDER_MON;
			stFeature.bDress = atoi(szArg1);
		}
		
		if ( !m_xMyHero.ChangeFeature(stFeature, m_xMyHero.m_stFeatureEx) )
		{
			strcpy(g_xChatEditBox.m_szInputMsg, "This monster cannot be transformed.");
		}
		else
		{
			m_xMyHero.m_stFeature = stFeature;
		}
	}	
	else if ( !strcmp(szCommand, "@hairdye") )
	{
		nArg1 = atoi(szArg1);
		nArg2 = atoi(szArg2);
		nArg3 = atoi(szArg3);
		
		WORD wColor = g_xMainWnd.ConvertColor24To16(RGB(nArg3, nArg2, nArg1));

		sprintf(g_xChatEditBox.m_szInputMsg, "@hairdye %d", wColor );
		m_xMyHero.m_stFeatureEx.wHairColor = wColor;
	}	
	else if ( !strcmp(szCommand, "@hairstyle") )
	{
		nArg1 = atoi(szArg1);

		FEATURE stFeature = m_xMyHero.m_stFeature;
		stFeature.bHair = nArg1;
		if ( stFeature.bHair > 12 )
			stFeature.bHair = _HAIR_NONE;

		m_xMyHero.ChangeFeature(stFeature, m_xMyHero.m_stFeatureEx);
	}	

// 普通消息.
	g_xClientSocket.SendSay(g_xChatEditBox.m_szInputMsg);

	if ( g_xChatEditBox.m_szInputMsg[0] == '/' )
	{
		DWORD dwBack = GetChatColor(_CHAT_COLOR1);
		DWORD dwFont = GetChatColor(_CHAT_COLOR6);
		m_xInterface.MsgAdd(dwFont, dwBack, g_xChatEditBox.m_szInputMsg);
	}

	ZeroMemory(g_xChatEditBox.m_szInputMsg, sizeof(g_xChatEditBox.m_szInputMsg));

	return 0L;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////









/******************************************************************************************************************

	其他函数及调试用函数。

*******************************************************************************************************************/
VOID CGameProcess::ShowStatus(INT nStartX, INT nStartY)
{
	if ( m_bStatusView )
	{
		TCHAR			szBuff[MAX_PATH];

		sprintf(szBuff, "Current tile x:%d, y:%d", m_ptCurrTile.x, m_ptCurrTile.y);
		g_xMainWnd.PutsHan(g_xMainWnd.GetBackBuffer(), nStartX, nStartY+10, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

		sprintf(szBuff, "Server No. : %s", g_szServerIP);
		g_xMainWnd.PutsHan(g_xMainWnd.GetBackBuffer(), nStartX, nStartY+25, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

		sprintf(szBuff, "Mouse x:%d, y:%d", m_ptMousePos.x, m_ptMousePos.y);
		g_xMainWnd.PutsHan(g_xMainWnd.GetBackBuffer(), nStartX, nStartY+40, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

		sprintf(szBuff, "MYIDNUM:%x, %d", m_xMyHero.m_dwIdentity, m_xWaitPacketQueue.GetCount());
		g_xMainWnd.PutsHan(g_xMainWnd.GetBackBuffer(), nStartX, nStartY+55, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

		m_xActorList.MoveCurrentToTop();
		CActor* pxActor;
		for ( INT nCnt = 0; nCnt < m_xActorList.GetCounter(); nCnt++ )
		{
			pxActor = m_xActorList.GetCurrentData();

			sprintf(szBuff, "IDNUM:%d, X:%d, Y:%d, MSGCNT:%d", pxActor->m_dwIdentity, pxActor->m_wPosX, pxActor->m_wPosY, pxActor->m_xPacketQueue.GetCount());
			g_xMainWnd.PutsHan(g_xMainWnd.GetBackBuffer(), pxActor->m_shScrnPosX, pxActor->m_shScrnPosY, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);
			m_xActorList.MoveNextNode();

		}

		g_xMainWnd.UsedAndFreeMemoryCheck();
		sprintf(szBuff, "Texture total:%d, free:%d, used:%d", g_xMainWnd.m_dwTextureTotal, g_xMainWnd.m_dwTextureFree, g_xMainWnd.m_dwTextureTotal-g_xMainWnd.m_dwTextureFree);
		g_xMainWnd.PutsHan(g_xMainWnd.GetBackBuffer(), nStartX, nStartY+70, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

		sprintf(szBuff, "Video RAM total:%d, free:%d, used:%d", g_xMainWnd.m_dwVideoTotal, g_xMainWnd.m_dwVideoFree, g_xMainWnd.m_dwVideoTotal-g_xMainWnd.m_dwVideoFree);
		g_xMainWnd.PutsHan(g_xMainWnd.GetBackBuffer(), nStartX, nStartY+85, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);
	}

	if ( m_bShowFPS )
		CWHDefProcess::ShowStatus(750, 0);
}



VOID CGameProcess::CommandAnalyze()
{
	CHAR	szCommand[MAX_PATH];
	CHAR	szArg1[MAX_PATH];
	CHAR	szArg2[MAX_PATH];
	CHAR	szArg3[MAX_PATH];
	CHAR	szArg4[MAX_PATH];

	ZeroMemory(szCommand, MAX_PATH);
	ZeroMemory(szArg1, MAX_PATH);
	ZeroMemory(szArg2, MAX_PATH);
	ZeroMemory(szArg3, MAX_PATH);
	ZeroMemory(szArg4, MAX_PATH);

	sscanf(g_xChatEditBox.m_szInputMsg, "%[^ ]%*c %[^ ]%*c %[^ ]%*c %[^ ]%*c %[^ ]%*c", szCommand, szArg1, szArg2, szArg3, szArg4);

	INT nArg1;	INT nArg2;	INT nArg3;	INT nArg4;
	nArg1 = nArg2 = nArg3 = nArg4 = 0;

	if ( !strcmp(szCommand, "@grid") )
	{
		nArg1 = atoi(szArg1);
		switch ( nArg1 )
		{
		case 0:
			m_bCellView= !m_bCellView;
			break;
		case 1:
			m_bTileView = !m_bTileView;
			break;
		case 2:
			m_bObj1View = !m_bObj1View;
			break;
		case 3:
			m_bObj2View = !m_bObj2View;
			break;
		case 4:
			m_bStopPtView = !m_bStopPtView;
			break;
		}
	}
	else if(!strcmp(g_xChatEditBox.m_szInputMsg,"@snow")) { m_weatherEnabled=!m_weatherEnabled; }
    else if(!strcmp(g_xChatEditBox.m_szInputMsg,"@chatsave")) { NativeChatLog().enabled=!NativeChatLog().enabled; }
	else if ( !strcmp(szCommand, "@fps") )
	{
		m_bShowFPS = !m_bShowFPS;
	}
	else if ( !strcmp(szCommand, "@show") )
	{
		if ( !strcmp(szArg1, "snow") )
		{
			m_weather.snow=!m_weather.snow;
		}
		else if ( !strcmp(szArg1, "rain") )
		{
			m_weather.other=!m_weather.other;
		}
		else if ( !strcmp(szArg1, "fog2") )
		{
			m_weather.fog=!m_weather.fog;
		}
		else if ( !strcmp(szArg1, "state") )
		{
			m_bStatusView = !m_bStatusView;
		}
	}
/*	else if ( !strcmp(szCommand, "@移动") )
	{
		nArg1 = atoi(szArg1);
		nArg2 = atoi(szArg2);

		m_xMyHero.m_wPosX = nArg1+11;
		m_xMyHero.m_wPosY = nArg2+11;
		m_xMap.SetStartViewTile(nArg1, nArg2);
		m_xMap.LoadNewMapBuffer();
	}
	else if ( !strcmp(szCommand, "@自由移动") )
	{
		m_bRender = FALSE;
		nArg2 = atoi(szArg2);
		nArg3 = atoi(szArg3);

		m_xMyHero.m_wPosX = nArg2+11;
		m_xMyHero.m_wPosY = nArg3+11;
		m_xMap.SetStartViewTile(nArg2, nArg3);
		LoadMapChanged(szArg1);
		Sleep(100);
		m_bRender = TRUE;
	}*/	
	else if ( !strcmp(szCommand, "@fix") )
	{
		if ( !strcmp(szArg1, "delay") )
		{
			nArg2 = atoi(szArg2);
			m_xMyHero.m_wDelay	= nArg2;
		}
	}	
}


VOID CGameProcess::RenderMapTileGrid()
{
	WORD	bCanMove = FALSE;
	INT		nObjCnt1 = 0;
	INT		nObjCnt2 = 0;

	INT		nObjFileIdx;
	INT		nFileIdx;
	INT		nImgIdx;

	RECT	rc;

	if ( m_bCellView || m_bTileView || m_bObj1View || m_bObj2View || m_bStopPtView )
	{
		for ( INT nXCnt = m_xMap.m_shStartViewTileX; nXCnt < m_xMap.m_shStartViewTileX+_VIEW_CELL_X_COUNT; nXCnt++ )
			for ( INT nYCnt = m_xMap.m_shStartViewTileY; nYCnt < m_xMap.m_shStartViewTileY+_VIEW_CELL_Y_COUNT_FOR_OBJ; nYCnt++ )
		{
			if ( nXCnt >= m_xMap.m_stMapFileHeader.shWidth || nYCnt >= m_xMap.m_stMapFileHeader.shHeight || nXCnt < 0 || nYCnt < 0 )
				continue; 

			INT nArrayNum = nYCnt + nXCnt*m_xMap.m_stMapFileHeader.shHeight;

			// 显示 Cell 网格.
			if ( m_bCellView )
			{
				SetRect(&rc,
					   (nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX,
					   (nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_shViewOffsetY,
					   (nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START+_CELL_WIDTH -m_xMap.m_shViewOffsetX,
					   (nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START+_CELL_HEIGHT-m_xMap.m_shViewOffsetY);
				g_xMainWnd.DrawWithGDI(rc, NULL, RGB(0, 255, 255), 1);
			}

			if ( m_bTileView )
			{
				// 显示图块网格。
 				if( !( nXCnt % 2 ) && !( nYCnt % 2 ) && nYCnt < m_xMap.m_shStartViewTileY+_VIEW_CELL_Y_COUNT )
 				{
					nFileIdx= m_xMap.m_pstTileInfo[(nYCnt/2) + (nXCnt/2)*m_xMap.m_stMapFileHeader.shHeight/2].bFileIdx;
					nImgIdx = m_xMap.m_pstTileInfo[(nYCnt/2) + (nXCnt/2)*m_xMap.m_stMapFileHeader.shHeight/2].wTileIdx;

					if( nFileIdx != 255 && nImgIdx != 65535 )
					{
						m_xMap.m_pxTileImg[nFileIdx]->NewSetIndex(nImgIdx);

						SetRect(&rc,
							   (nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX,
							   (nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_shViewOffsetY,
							   (nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START+m_xMap.m_pxTileImg[nFileIdx]->m_lpstNewCurrWilImageInfo->shWidth -m_xMap.m_shViewOffsetX,
							   (nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START+m_xMap.m_pxTileImg[nFileIdx]->m_lpstNewCurrWilImageInfo->shHeight-m_xMap.m_shViewOffsetY);
						g_xMainWnd.DrawWithGDI(rc, NULL, RGB(255, 0, 0), 1);
					}
				}
			}

			if ( m_bObj1View )
			{
				// 显示 Obj1 图块网格.
				if ( m_xMap.m_pstCellInfo[nArrayNum].bFileIdx != 255 )
				{
					if ( m_xMap.m_pstCellInfo[nArrayNum].wObj1 != 255 )
					{
						nObjFileIdx = m_xMap.m_pstCellInfo[nArrayNum].bObjectFile1;
						nImgIdx		= m_xMap.m_pstCellInfo[nArrayNum].wObj1;

						if ( nObjFileIdx > 2 && nObjFileIdx < 14 )
						{
							m_xMap.m_pxTileImg[nObjFileIdx]->NewSetIndex(nImgIdx);
							SetRect(&rc,
								   (nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX,
								   (nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight+_CELL_HEIGHT-m_xMap.m_shViewOffsetY,
								   (nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START+m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shWidth -m_xMap.m_shViewOffsetX,
								   (nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START+m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight-m_xMap.m_shViewOffsetY);

//							if ( PtInRect(&rc, m_ptMousePos) )
							{
								nObjCnt1++;
								g_xMainWnd.DrawWithGDI(rc, NULL, RGB(255, 0, 255), 1);

								TCHAR szBuff[MAX_PATH];
								sprintf(szBuff, "[%d]%d%d", nObjCnt1, nXCnt, nYCnt);
								g_xMainWnd.PutsHan(g_xMainWnd.GetBackBuffer(), 
												   (nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX,
												   (nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight+_CELL_HEIGHT-m_xMap.m_shViewOffsetY,
												   RGB(255, 0, 255), RGB(0, 0, 0), szBuff);
							}
						}
					}
				}
			}
			
			if ( m_bObj2View )
			{
				// 显示 Obj2 图块网格。
				if ( m_xMap.m_pstCellInfo[nArrayNum].bFileIdx != 255 )
				{
					if ( m_xMap.m_pstCellInfo[nArrayNum].wObj2 != 65535 /*&& m_shObjTileViewState == 2*/ )
					{
						nObjFileIdx = m_xMap.m_pstCellInfo[nArrayNum].bObjectFile2;
						nImgIdx		= m_xMap.m_pstCellInfo[nArrayNum].wObj2;

						if ( nObjFileIdx > 2 && nObjFileIdx < 14 )
						{
							m_xMap.m_pxTileImg[nObjFileIdx]->NewSetIndex(nImgIdx);
							SetRect(&rc,
								   (nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX,
								   (nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight+_CELL_HEIGHT-m_xMap.m_shViewOffsetY,
								   (nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START+m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shWidth -m_xMap.m_shViewOffsetX,
								   (nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START+_CELL_HEIGHT-m_xMap.m_shViewOffsetY);

//							if ( PtInRect(&rc, m_ptMousePos) )
							{
								nObjCnt2++;
								g_xMainWnd.DrawWithGDI(rc, NULL, RGB(0, 255, 255), 1);

								TCHAR szBuff[MAX_PATH];
								sprintf(szBuff, "[%d]%d%d", nObjCnt2, nXCnt, nYCnt);
								g_xMainWnd.PutsHan(g_xMainWnd.GetBackBuffer(), 
												   (nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX,
												   (nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_pxTileImg[nObjFileIdx]->m_lpstNewCurrWilImageInfo->shHeight+_CELL_HEIGHT-m_xMap.m_shViewOffsetY,
												   RGB(0, 255, 255), RGB(0, 0, 0), szBuff);
							}
						}
					}
				}
			}

			if ( m_bStopPtView )
			{
				bCanMove  = m_xMap.GetTileAttribute(nXCnt, nYCnt);
				if ( bCanMove != _CAN_WALK && m_bStopPtView )
				{
					SetRect(&rc,
						   (nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX,
						   (nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_shViewOffsetY,
						   (nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START+_CELL_WIDTH -m_xMap.m_shViewOffsetX,
						   (nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START+_CELL_HEIGHT-m_xMap.m_shViewOffsetY);
					g_xMainWnd.DrawWithGDI(rc, NULL, RGB(255, 255, 0), 1);
				}
			}

/*			if ( ( m_xMap.m_pstCellInfo[nArrayNum].wLigntNEvent & 0X07 ) == 2 )
			{
				SetRect(&rc,
					   (nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX,
					   (nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_shViewOffsetY,
					   (nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START+_CELL_WIDTH -m_xMap.m_shViewOffsetX,
					   (nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START+_CELL_HEIGHT-m_xMap.m_shViewOffsetY);
				g_xMainWnd.DrawWithGDI(rc, NULL, RGB(255, 255, 255), 2);
			}
*/
		}
	}
	for ( INT nXCnt = m_xMap.m_shStartViewTileX; nXCnt < m_xMap.m_shStartViewTileX+_VIEW_CELL_X_COUNT; nXCnt++ )
		for ( INT nYCnt = m_xMap.m_shStartViewTileY; nYCnt < m_xMap.m_shStartViewTileY+_VIEW_CELL_Y_COUNT_FOR_OBJ; nYCnt++ )
	{
		if ( nXCnt >= m_xMap.m_stMapFileHeader.shWidth || nYCnt >= m_xMap.m_stMapFileHeader.shHeight || nXCnt < 0 || nYCnt < 0 )
			continue; 

//		if ( m_xMap.GetDoor(nXCnt, nYCnt) )
		if ( !m_xMap.IsDoorOpen(nXCnt, nYCnt) )
		{
			SetRect(&rc,
				   (nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START-m_xMap.m_shViewOffsetX,
				   (nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_shViewOffsetY,
				   (nXCnt-m_xMap.m_shStartViewTileX)*_CELL_WIDTH +_VIEW_CELL_X_START+_CELL_WIDTH -m_xMap.m_shViewOffsetX,
				   (nYCnt-m_xMap.m_shStartViewTileY)*_CELL_HEIGHT+_VIEW_CELL_Y_START+_CELL_HEIGHT-m_xMap.m_shViewOffsetY);
			g_xMainWnd.DrawWithGDI(rc, NULL, RGB(255, 255, 255), 1);
		}
	}
}


LRESULT CGameProcess::DefMainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if(uMsg==WM_ACTIVATEAPP && !wParam) ChatShortcuts().controlDown=ChatShortcuts().altDown=false;
    if(m_exitTransition.active && ((uMsg>=WM_KEYFIRST && uMsg<=WM_KEYLAST) ||
       (uMsg>=WM_MOUSEFIRST && uMsg<=WM_MOUSELAST) || uMsg==WM_COMMAND)) return 0L;
    if ((m_xInterface.m_xMsgBox.IsActive() || m_xInterface.m_xGuildEditor.m_bActive) &&
        (uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP || uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP))
        return 0L;
    if((uMsg==WM_KEYDOWN || uMsg==WM_KEYUP || uMsg==WM_SYSKEYDOWN || uMsg==WM_SYSKEYUP) &&
       !m_xInterface.m_xMsgBox.IsActive() && !m_xInterface.m_xGuildEditor.m_bActive &&
       ChatShortcuts().Input(uMsg,wParam)) return 0;
	switch ( uMsg )
	{
        case WM_SIZE: {
            const LRESULT result=g_xMainWnd.OnSize(wParam,lParam);
            g_xChatEditBox.Relocate();return result;
        }
        case WM_COMMAND:
            if ((HWND)lParam==g_xChatEditBox.GetSafehWnd() && HIWORD(wParam)==EN_CHANGE) { g_xChatEditBox.Changed(); return 0; }
            if ((HWND)lParam==m_xInterface.m_xGuildEditor.edit && HIWORD(wParam)==EN_CHANGE) { m_xInterface.m_xGuildEditor.Changed(); return 0; }
            break;
		case WM_CTLCOLOREDIT:
		{
            for(auto* panel:{&m_xInterface.m_xConsignmentWnd,&m_xInterface.m_xConsignmentRegistration}) for(HWND edit:panel->edits) if(edit && edit==(HWND)lParam) return (LRESULT)panel->EditColor(edit,(HDC)wParam);
            if ((HWND)lParam==m_xInterface.m_xMailWnd.edits[0] || (HWND)lParam==m_xInterface.m_xMailWnd.edits[1]) return (LRESULT)m_xInterface.m_xMailWnd.EditColor((HWND)lParam,(HDC)wParam);
            if ((HWND)lParam==m_xInterface.m_xGuildEditor.edit) return (LRESULT)m_xInterface.m_xGuildEditor.EditColor((HDC)wParam);
			if ((HWND)lParam == g_xChatEditBox.GetSafehWnd())
			{
                SetBkColor((HDC)wParam,RGB(0,0,0));
                SetTextColor((HDC)wParam,RGB(255,255,255));
                return (LRESULT)GetStockObject(BLACK_BRUSH);
			}

			break;
		}
		case WM_DESTROY:
			OnDestroy(wParam, lParam);
			break;
		case WM_TIMER:
			OnTimer(wParam, lParam);
			break;
		case WM_MOUSEMOVE:
			OnMouseMove(wParam, lParam);
			break;
		case WM_LBUTTONDBLCLK:
			OnLButtonDoubleClick(wParam, lParam);
			break;

		case WM_LBUTTONDOWN:
			OnLButtonDown(wParam, lParam);
			break;
		case WM_RBUTTONDOWN:
			OnRButtonDown(wParam, lParam);
			break;
		case WM_KEYDOWN:
			OnKeyDown(wParam, lParam);
			break;
		case WM_KEYUP:
			OnKeyUp(wParam, lParam);
			break;
		case WM_LBUTTONUP:
			OnLButtonUp(wParam, lParam);
			break;
		case WM_RBUTTONUP:
			OnRButtonUp(wParam, lParam);
			break;
		case WM_MOVE:
			OnMove(wParam, lParam);
			break;
		case WM_SYSKEYDOWN:
			return OnSysKeyDown(wParam, lParam);
		case WM_SYSKEYUP:
			return OnSysKeyUp(wParam, lParam);
		case WM_MOUSEWHEEL:
			return OnWheel(wParam,lParam);
		case _WM_USER_MSG_INPUTTED:
			return OnMsgInputted(wParam, lParam);
		case ID_SOCKCLIENT_EVENT_MSG:
			g_xClientSocket.OnSocketMessage(wParam, lParam);
			break;
		default:
			break;
	}

	return CWHDefProcess::DefMainWndProc(hWnd, uMsg, wParam, lParam);
}

VOID CGameProcess::OnConnectToServer()
{
	g_xClientSocket.SendRunLogin(g_szUserID, g_szCharName, g_nCertifyCode, g_nVersion);
}

char* CGameProcess::OnMessageReceive(CHAR* pszMessage)
{
	char	*pszFirst;
	char	*pszEnd;
	int		nCount = 0;

	if (*pszMessage == '*')
	{
		pszFirst = pszMessage + 1;
		send(g_xClientSocket.m_sockClient, "*", 1, 0);
	}
	else
		pszFirst = pszMessage;

	while (pszEnd = strchr(pszFirst, '!'))
	{
		*pszEnd++ = '\0';

		OnSocketMessageRecieve(pszFirst + 1);

		if (*pszEnd == '*')
		{
			send(g_xClientSocket.m_sockClient, "*", 1, 0);
			pszEnd++;
		}

		if (*pszEnd == '#')
			pszFirst = pszEnd;
		else
			return NULL;
	}

	return pszFirst;
}

void CGameProcess::OnProcPacketNotEncode(char *pszMsg)
{
	static char pszGood[]	= "GOOD";
	static char pszFail[]	= "FAIL";
	static char pszLng[]	= "LNG";
	static char pszUlng[]	= "ULNG";
	static char pszWid[]	= "WID";
	static char pszUWid[]	= "UWID";
	static char pszFir[]	= "FIR";
	static char pszUFir[]	= "UFIR";
	static char pszUPwr[]	= "PWR";

	if ( memcmp(pszMsg, pszGood, strlen(pszGood)) == 0 )
		m_xMyHero.ConfirmAction();
 	else if( memcmp(pszMsg, pszFail, strlen(pszFail)) == 0 )
		m_xMyHero.SetOldPosition();
 	else if( memcmp(pszMsg, pszLng, strlen(pszLng)) == 0 )
		m_xMyHero.m_bUseErgum  = TRUE;
 	else if( memcmp(pszMsg, pszUlng, strlen(pszUlng)) == 0 )
		m_xMyHero.m_bUseErgum  = FALSE;
 	else if( memcmp(pszMsg, pszWid, strlen(pszWid)) == 0 )
		m_xMyHero.m_bUseBanwol = TRUE;
 	else if( memcmp(pszMsg, pszUWid, strlen(pszUWid)) == 0 )
		m_xMyHero.m_bUseBanwol = FALSE;
 	else if( memcmp(pszMsg, pszFir, strlen(pszFir)) == 0 )
		m_xMyHero.m_bFireHitCnt  = 1;
 	else if( memcmp(pszMsg, pszUFir, strlen(pszUFir)) == 0 )
		m_xMyHero.m_bFireHitCnt  = 0;
 	else if( memcmp(pszMsg, pszUPwr, strlen(pszUPwr)) == 0 )
		m_xMyHero.m_bYedoCnt  = 1;
}

CActor*	CGameProcess::FindActor(int nID) // gadget
{
	CActor* pxActor = NULL;

	m_xActorList.MoveCurrentToTop();
	
	for ( INT nCnt = 0; nCnt < m_xActorList.GetCounter(); nCnt++ )
	{
		pxActor = m_xActorList.GetCurrentData();

		if ( pxActor->m_dwIdentity == nID )
			return pxActor;
	
		m_xActorList.MoveNextNode();
	}

	return NULL;
}

void CGameProcess::OnSocketMessageRecieve(char *pszMsg)
{
	if (*pszMsg == '+')
		OnProcPacketNotEncode(pszMsg + 1);
	else
	{
		int					nPos = 0;
		_TDEFAULTMESSAGE	tdm;

		fnDecodeMessage(&tdm, pszMsg);

		switch (tdm.wIdent)
		{
            case 1402: // MIR3 portrait upload acknowledgement
                if(tdm.nRecog) {
                    m_xInterface.m_xPortraitWnd.SaveOwnVersion(DWORD(tdm.nRecog));
                    static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
                    std::string message=captions.Get(1519);m_xInterface.MsgAdd(0xffffff,0xff,const_cast<char*>(message.c_str()));
                }
                break;
            case 1403: // MIR3 player portrait reply
            {
                if(!tdm.wParam && !tdm.wTag) {
                    if(DWORD(tdm.nRecog)==m_xMyHero.m_dwIdentity) {m_xInterface.m_xPortraitWnd.ApplyOwnPhoto(nullptr,0);m_xInterface.m_xPortraitWnd.SaveOwnCache({});m_xInterface.m_xPortraitWnd.SaveOwnVersion(0);}
                    m_xInterface.m_targetDisplay.ApplyPortrait(DWORD(tdm.nRecog),nullptr,0);
                }
                else {
                    BYTE jpeg[4097];const int length=fnDecode6BitBuf(pszMsg+DEFBLOCKSIZE,(char*)jpeg,sizeof(jpeg));
                    if(DWORD(tdm.nRecog)==m_xMyHero.m_dwIdentity && m_xInterface.m_xPortraitWnd.ApplyOwnPhoto(jpeg,length))
                    {
                        if(m_xInterface.m_xPortraitWnd.SaveOwnCache(std::vector<BYTE>(jpeg,jpeg+length)))
                            m_xInterface.m_xPortraitWnd.SaveOwnVersion(MAKELONG(tdm.wParam,tdm.wTag));
                    }
                    m_xInterface.m_targetDisplay.ApplyPortrait(DWORD(tdm.nRecog),jpeg,length);
                }
                break;
            }
            case 1304: // MIR3 target attributes
            {
                BYTE body[11]={};
                const int length=fnDecode6BitBuf(pszMsg+DEFBLOCKSIZE,(char*)body,sizeof(body));
                m_xInterface.m_targetDisplay.ApplyAttributes(DWORD(tdm.nRecog),tdm.wParam,tdm.wTag,body,length);
                break;
            }
            case MailWire::NotificationMessage:
                m_xInterface.m_xMailWnd.notified=true;break;
            case QuestWire::ListMessage:case QuestWire::AddedMessage:case QuestWire::ChildAddedMessage:
            case QuestWire::ChildrenMessage:case QuestWire::TextMessage:
                m_xInterface.m_xQuestWnd.Receive(tdm,pszMsg+DEFBLOCKSIZE);break;
            case ConsignmentWire::ListMessage:
                m_xInterface.m_xConsignmentWnd.ReceiveList(pszMsg+DEFBLOCKSIZE,tdm.wSeries,tdm.wParam,tdm.wTag);
                if(!m_xInterface.m_xConsignmentWnd.m_bActive) {
                    m_xInterface.WindowActivate(ConsignmentLayout::BrowserId);
                    m_xInterface.m_xConsignmentWnd.sessionId=tdm.nRecog;
                }
                break;
            case ConsignmentWire::WaitingMessage:
                m_xInterface.m_xConsignmentWnd.waiting=tdm.nRecog==0;
                break;
            case MailWire::ListMessage:
                if(tdm.wParam==MailWire::Inbox || tdm.wParam==MailWire::Outbox)
                    m_xInterface.m_xMailWnd.ReceiveList(tdm.wParam==MailWire::Outbox,pszMsg+DEFBLOCKSIZE,tdm.wSeries);
                break;
            case MailWire::BodyMessage:
                if(tdm.wParam==MailWire::Inbox || tdm.wParam==MailWire::Outbox) {
                    BYTE record[MailWire::BodyBytes]={};
                    const int length=fnDecode6BitBuf(pszMsg+DEFBLOCKSIZE,(char*)record,sizeof(record));
                    m_xInterface.m_xMailWnd.ReceiveBody(tdm.wParam==MailWire::Outbox,record,length);
                }
                break;
            case SiegeLayout::ObjectPositionMessage:
                m_xInterface.m_xSiegeWnd.ReceiveObject(DWORD(tdm.nRecog),POINT{tdm.wParam,tdm.wTag},tdm.wSeries);break;
            case SiegeLayout::ControlMessage:
            {
                char body[8192]={};
                const int length=fnDecode6BitBuf(pszMsg+DEFBLOCKSIZE,body,sizeof(body)-1);
                body[length]=0;
                m_xInterface.m_xSiegeWnd.ReceiveControl(DWORD(tdm.nRecog),tdm.wParam,body);
                break;
            }
            case SiegeLayout::StatusMessage:
                m_xInterface.m_xSiegeWnd.ReceiveStatus(DWORD(tdm.nRecog));break;
            case SiegeLayout::ReleaseMessage:
                m_xInterface.m_xSiegeWnd.ReleaseControl();break;
            case 710: // MIR3 minimap reply, one-based archive index
                m_xInterface.m_minimap.Receive(tdm.wParam,GetTickCount());
                break;
            case 711: // MIR3 minimap unavailable
                m_xInterface.m_minimap.Unavailable(GetTickCount());
                break;
            case 1401: // MIR3 SM_CLIENT_OPTIONS
                m_xInterface.m_xOptionWnd.ReceiveServerOptions(DWORD(tdm.nRecog),tdm.wParam);
                break;
			case SM_SENDNOTICE:
			{
				char szDecodeMsg[8193];

				nPos = fnDecode6BitBuf((pszMsg + DEFBLOCKSIZE), szDecodeMsg, sizeof(szDecodeMsg)-1);
				szDecodeMsg[nPos] = '\0';
				m_xInterface.m_xMsgBox.HideMessageBox();
                m_xNotice.ShowMessageBox(szDecodeMsg,CNoticeBox::Frame);

				break;
			}
			case SM_NEWMAP:
				OnSvrMsgNewMap(&tdm, (pszMsg + DEFBLOCKSIZE));
				break;
			case SM_LOGON:
				OnSvrMsgLogon(&tdm, (pszMsg + DEFBLOCKSIZE));
				break;
			case SM_MAPDESCRIPTION:
				nPos = fnDecode6BitBuf((pszMsg + DEFBLOCKSIZE), (char*)m_szMapName, sizeof(m_szMapName));
				m_szMapName[nPos] = '\0';
				break;
            case SM_ABILITY:
                ReceiveAbility(tdm,pszMsg+DEFBLOCKSIZE);
                break;
            case SM_WINEXP:
            case SM_SUBABILITY:
            case SM_LEVELUP:
            case SM_WEIGHTCHANGED:
                ReceiveProgression(tdm);
                break;
            case SM_SENDMYMAGIC:
            case SM_ADDMAGIC:
            case SM_MAGIC_LVEXP:
            case SkillWire::DeleteMessage:
                m_xInterface.m_xSkillWnd.Receive(tdm,pszMsg+DEFBLOCKSIZE);
                break;
            case SM_BAGITEMS:
                if(m_xInterface.m_xInventoryWnd.ReceiveBagRecords(pszMsg+DEFBLOCKSIZE,tdm.wSeries))
                    m_xInterface.m_xInventoryWnd.CheckItemPos(m_xMyHero.m_szName);
                break;
            case SM_SENDUSEITEMS:
                m_xInterface.m_xStatusWnd.ReceiveEquipmentRecords(pszMsg+DEFBLOCKSIZE);
                break;
            case 1313: // MIR3 SM_INVENTITEM: header-only stack quantity update.
                if(!pszMsg[DEFBLOCKSIZE]) m_xInterface.m_xInventoryWnd.UpdateStackCount(tdm.nRecog,tdm.wParam);
                break;
			case SM_ADDITEM:
			{
                if(strlen(pszMsg+DEFBLOCKSIZE)==134) {
                    m_xInterface.m_xInventoryWnd.ReceiveItemRecord(tdm.wIdent,pszMsg+DEFBLOCKSIZE);
                    break;
                }
                if(strlen(pszMsg+DEFBLOCKSIZE)!=(sizeof(CLIENTITEMRCD)*8+5)/6) break;
				CLIENTITEMRCD	stReceivedItem;

				char *pszItem = (pszMsg + DEFBLOCKSIZE);

				fnDecode6BitBuf(pszItem, (char*)&stReceivedItem, sizeof(CLIENTITEMRCD));
				CItem xItem;
				xItem.SetItem(stReceivedItem, 0);
				m_xInterface.m_xInventoryWnd.AddInvenItem(xItem, 0, FALSE);

				CHAR szSysMsg[MAX_PATH];
				sprintf(szSysMsg, "You got %s.", xItem.m_stItemInfo.stStdItem.szName);
				m_xInterface.m_xClientSysMsg.AddSysMsg(szSysMsg);
				break;
			}
			case SM_DELITEM:
			{
                if(strlen(pszMsg+DEFBLOCKSIZE)==134) {
                    m_xInterface.m_xInventoryWnd.ReceiveItemRecord(tdm.wIdent,pszMsg+DEFBLOCKSIZE);
                    break;
                }
                if(strlen(pszMsg+DEFBLOCKSIZE)!=(sizeof(CLIENTITEMRCD)*8+5)/6) break;
				CLIENTITEMRCD	stReceivedItem;

				char *pszItem = (pszMsg + DEFBLOCKSIZE);

				fnDecode6BitBuf(pszItem, (char*)&stReceivedItem, sizeof(CLIENTITEMRCD));

				m_xInterface.m_xInventoryWnd.DeleteInvenItem(stReceivedItem.nMakeIndex, stReceivedItem.stStdItem.szName);
				m_xInterface.m_xStatusWnd.DeleteEquipItem(stReceivedItem.nMakeIndex, stReceivedItem.stStdItem.szName);

				CHAR szSysMsg[MAX_PATH];
				sprintf(szSysMsg, "%s item was deleted.", stReceivedItem.stStdItem.szName);
				m_xInterface.m_xClientSysMsg.AddSysMsg(szSysMsg);
				break;
			}
			case SM_DELITEMS: // 203 is SM_UPDATEITEM in MIR3.
			{
                if(!strchr(pszMsg+DEFBLOCKSIZE,'/')) {
                    m_xInterface.m_xInventoryWnd.ReceiveItemRecord(tdm.wIdent,pszMsg+DEFBLOCKSIZE);
                    break;
                }
				CHAR szSysMsg[MAX_PATH];

				char szItemName[64];
				char szItemIndex[32];
				char nItemIndex;

				char *pszItem  = (pszMsg + DEFBLOCKSIZE);
				char *pszItemIndex;
				int  nCount = 0;

				do
				{
					if (pszItemIndex = strchr(pszItem, '/'))
					{
						*pszItemIndex++ = '\0';

						nPos = fnDecode6BitBuf(pszItem, (char*)&szItemName, sizeof(szItemName));
						szItemName[nPos] = '\0';

						if (pszItem = strchr(pszItemIndex, '/'))
						{
							*pszItem++ = '\0';

							fnDecode6BitBuf(pszItem, (char*)&szItemIndex, sizeof(szItemIndex));
							szItemIndex[nPos] = '\0';

							nItemIndex = atoi(szItemIndex);

							m_xInterface.m_xInventoryWnd.DeleteInvenItem(nItemIndex, szItemName);
							m_xInterface.m_xStatusWnd.DeleteEquipItem(nItemIndex, szItemName);

							sprintf(szSysMsg, "%s item was deleted.", szItemName);
							m_xInterface.m_xClientSysMsg.AddSysMsg(szSysMsg);

							nCount++;
						}
					}
				} while (nCount < tdm.wSeries && pszItem);

				break;
			}
            case SM_EAT_OK:
            case SM_EAT_FAIL:
                m_xInterface.m_xInventoryWnd.ApplyUseReply(tdm.wIdent,tdm.wParam);
                break;
            case SM_TAKEON_OK:
            case SM_TAKEON_FAIL:
            case SM_TAKEOFF_OK:
            case SM_TAKEOFF_FAIL:
                if(m_xInterface.m_xStatusWnd.ApplyEquipmentReply(tdm.wIdent) && (tdm.wIdent==SM_TAKEON_OK || tdm.wIdent==SM_TAKEOFF_OK)) {
                    FEATURE appearance;memcpy(&appearance,&tdm.nRecog,sizeof(LONG));
                    m_xMyHero.ChangeFeature(appearance,m_xMyHero.m_stFeatureEx);
                }
                break;
            case SM_ITEMSHOW:
            {
                if(!m_bRender || tdm.nRecog<=0 || tdm.wParam>=m_xMap.m_stMapFileHeader.shWidth ||
                    tdm.wTag>=m_xMap.m_stMapFileHeader.shHeight) break;
                const char* encoded=pszMsg+DEFBLOCKSIZE;const size_t length=strlen(encoded);
                if(length>ActorPacketBodyCapacity-1) break;
                bool valid=true;
                for(size_t i=0;i<length;++i) if(BYTE(encoded[i])<0x3c || BYTE(encoded[i])>0x7b) {valid=false;break;}
                if(!valid) break;
                char decoded[MaximumActorPayloadBytes+1]={};
                const int count=fnDecode6BitBuf(const_cast<char*>(encoded),decoded,sizeof(decoded)-1);
                if(count<0 || count>MaximumActorPayloadBytes) break;
                decoded[count]=0;std::string name(decoded);int stackMode=0;
                const auto slash=name.find('/');
                if(slash!=std::string::npos) {stackMode=strtol(name.c_str(),nullptr,10);name.erase(0,slash+1);}
                if(name.size()>GroundItemDisplay::NameLimit) break;
                bool exists=false;m_stMapItemList.MoveCurrentToTop();
                for(int index=0;index<m_stMapItemList.GetCounter();++index) {
                    if(m_stMapItemList.GetCurrentData()->nRecog==tdm.nRecog) {exists=true;break;}
                    m_stMapItemList.MoveNextNode();
                }
                if(exists) break;
                auto* item=new GROUNDITEM{};item->nRecog=tdm.nRecog;item->shTileX=tdm.wParam;item->shTileY=tdm.wTag;
                item->wLooks=tdm.wSeries;item->stackMode=stackMode;memcpy(item->szItemName,name.c_str(),name.size()+1);
                const int first=rand();item->sparkle.Begin(GetTickCount(),first,rand());
                m_stMapItemList.AddNode(item);break;
            }
			case SM_ITEMHIDE:
			{
                if(pszMsg[DEFBLOCKSIZE]) break;
				LPGROUNDITEM pstItem;

				m_stMapItemList.MoveCurrentToTop();
				
				for ( INT nCnt = 0; nCnt < m_stMapItemList.GetCounter(); nCnt++ )
				{
					pstItem = m_stMapItemList.GetCurrentData();												
					
					if ( pstItem->nRecog == tdm.nRecog )
					{
						m_stMapItemList.DeleteCurrentNodeEx();
						SAFE_DELETE(pstItem);
						break;
					}
				
					m_stMapItemList.MoveNextNode();
				}	

				break;
			}
			case SM_SHOWEVENT:
			{
				BOOL bExisted = FALSE;

				_TSHORTMSSEAGE stShortMsg;
				
				fnDecode6BitBuf((pszMsg + DEFBLOCKSIZE), (char*)&stShortMsg, sizeof(_TSHORTMSSEAGE));

				if ( m_xGroundMagicList.GetCounter() != 0 )
				{
					m_xGroundMagicList.MoveCurrentToTop();
					CMagic* pxMagic;

					for ( INT nCnt = 0; nCnt < m_xGroundMagicList.GetCounter(); nCnt++ )
					{
						pxMagic = m_xGroundMagicList.GetCurrentData();

						switch ( pxMagic->m_bMagicNum )
						{
							case _SKILL_EARTHFIRE: 
							case _SKILL_HOLYSHIELD:
							{
								CRepeatMagic* pxRepeatMagic = (CRepeatMagic*)pxMagic;
								if ( pxRepeatMagic->m_nEventID == tdm.nRecog )
									bExisted = TRUE;

								break;
							}
						}

						m_xGroundMagicList.MoveNextNode();
					}
				}	

				// 创建.
				if ( !bExisted )
				{
					BYTE bEffectNum = 0;

					if ( tdm.wParam == _ET_FIRE )
					{
						bEffectNum = _SKILL_EARTHFIRE;
						CRepeatMagic* pxRepeatMagic;
						pxRepeatMagic = new CRepeatMagic;
						pxRepeatMagic->CreateMagic(tdm.nRecog, bEffectNum, 0, 0, tdm.wTag, tdm.wSeries, 0XFFFFFFFF, NULL, NULL);

						pxRepeatMagic->m_dwCurrFrame += GetRandomNum(1, 8);

						g_xGameProc.m_xGroundMagicList.AddNode(pxRepeatMagic);

					}
					else if ( tdm.wParam == _ET_HOLYCURTAIN )
					{
						bEffectNum = _SKILL_HOLYSHIELD;
						CRepeatMagic* pxRepeatMagic;
						pxRepeatMagic = new CRepeatMagic;
						pxRepeatMagic->CreateMagic(tdm.nRecog, bEffectNum, 0, 0, tdm.wTag, tdm.wSeries, 0XFFFFFFFF, NULL, NULL);

						pxRepeatMagic->m_dwCurrFrame += GetRandomNum(1, 8);

						g_xGameProc.m_xGroundMagicList.AddNode(pxRepeatMagic);
					}
				}							
				break;
			}
			case SM_HIDEEVENT:
			{
				if ( m_xGroundMagicList.GetCounter() != 0 )
				{
					m_xGroundMagicList.MoveCurrentToTop();
					CMagic* pxMagic;

					for ( INT nCnt = 0; nCnt < m_xGroundMagicList.GetCounter(); nCnt++ )
					{
						pxMagic = m_xGroundMagicList.GetCurrentData();
						CRepeatMagic* pxRepeatMagic = NULL;

						switch ( pxMagic->m_bMagicNum )
						{
							case _SKILL_EARTHFIRE: 
							case _SKILL_HOLYSHIELD:
							{
								pxRepeatMagic = (CRepeatMagic*)pxMagic;
								break;
							}
						}
						if ( pxRepeatMagic && pxRepeatMagic->m_nEventID == tdm.nRecog )
						{
							pxRepeatMagic->m_dwMagiLifeTotal = 0;
							break;
						}

						m_xGroundMagicList.MoveNextNode();
					}
				}	
				break;
			}
            case SM_DAYCHANGING:
            {
                if(pszMsg[DEFBLOCKSIZE]) break;
                m_bDayState=BYTE(tdm.wParam);m_bFogState=BYTE(tdm.wTag);
                m_sceneAmbient.select_base(BYTE(tdm.wParam),m_weather.random());
                m_sceneAmbient.update(tdm.wTag);
                m_weather.select(tdm.wSeries);
                break;
            }
			case SM_AREASTATE:
			{
				break;
			}
			case SM_HEAR:
			case SM_SYSMESSAGE:
			case SM_GUILDMESSAGE:
			case SM_CRY:
			case SM_WHISPER:
			case SM_GROUPMESSAGE:
			case SM_MONSTERSAY:
				OnSvrMsgHear(&tdm, (pszMsg + DEFBLOCKSIZE));
				break;
			case SM_DISAPPEAR:
			case SM_CLEAROBJECT:
			{
				_TDEFAULTMESSAGE*	lpstDefMsg;
                m_xInterface.m_targetDisplay.Remove(DWORD(tdm.nRecog));
				lpstDefMsg	= new _TDEFAULTMESSAGE;

				memcpy(lpstDefMsg, &tdm, sizeof(_TDEFAULTMESSAGE));
				
				m_xWaitDefaultPacketQueue.PushQ((BYTE*)lpstDefMsg);
				break;
			}
            case SM_MERCHANTSAY:
            {
                char decoded[8193];
                const int length=fnDecode6BitBuf(pszMsg+DEFBLOCKSIZE,decoded,sizeof(decoded)-1);
                decoded[length]=0;
                const char* script=strchr(decoded,'/'); if (!script) break;
                m_wNPCWndPosX=m_xMyHero.m_wPosX; m_wNPCWndPosY=m_xMyHero.m_wPosY;
                m_xInterface.m_xNPCWnd.SetScript(tdm.nRecog,script+1);
                if (!m_xInterface.m_xNPCWnd.GetGameWndActive()) m_xInterface.WindowActivate(_WND_ID_NPCCHAT);
                break;
            }
			case SM_MERCHANTDLGCLOSE:
			{
				CloseNPCWindow();
				break;
			}
            case SM_SENDUSERSELL:
            case SM_OPEN_REPAIR:
            case SM_STORAGE_OPEN:
                if (tdm.nRecog) m_xInterface.m_xInventoryWnd.OpenService(tdm.nRecog,tdm.wIdent==SM_STORAGE_OPEN?3:tdm.wIdent==SM_SENDUSERSELL?2:1);
                break;
            case TradeWire::Open: case TradeWire::OpenFail:
            case TradeWire::AddOk: case TradeWire::AddFail:
            case TradeWire::RemoveOk: case TradeWire::RemoveFail:
            case TradeWire::Cancelled: case TradeWire::PeerAdd: case TradeWire::PeerRemove:
            case TradeWire::LocalGold: case TradeWire::PeerGold: case TradeWire::Complete:
                m_xInterface.m_xExchangeWnd.Receive(tdm,pszMsg+DEFBLOCKSIZE);
                break;
            case SM_USERSELLITEM_OK:
            case SM_USERSELLITEM_FAIL:
            case MerchantService::StackSaleOk:
            case MerchantService::StackSaleFail:
            case MerchantService::RepairOk:
            case MerchantService::RepairFail:
            case MerchantService::DepositOk:
            case MerchantService::StorageFull:
            case MerchantService::DepositFail:
                if(!pszMsg[DEFBLOCKSIZE]) m_xInterface.m_xInventoryWnd.ReceiveService(tdm);
                break;
            case SM_SENDBUYPRICE:
            case SM_REPAIR_QUOTE:
                m_xInterface.m_xInventoryWnd.QuoteService(DWORD(tdm.nRecog),tdm.wIdent==SM_SENDBUYPRICE);
                break;
            case SM_SENDGOODSLIST:
			case SM_SENDDETAILGOODSLIST:
			case SM_BUYITEM_FAIL:
            case SM_BUYITEM_SUCCESS:
            case SM_STORAGE_LIST:
            case SM_STORAGE_WITHDRAW_OK:
            case SM_STORAGE_WITHDRAW_FAIL:
            case SM_STORAGE_BAG_FULL:
			
			{	// 商店窗口处理例程
                m_xInterface.m_xStoreWnd.Receive(tdm,pszMsg+DEFBLOCKSIZE);
				break;
			}
			case SM_DROPITEM_SCCESS: 
			{
				CHAR	pszItemName[15];
				if (m_xInterface.m_stCommonItem.bWaitResponse )
				{
					if ( m_xInterface.m_stCommonItem.bSetted && m_xInterface.m_stCommonItem.xItem.m_stItemInfo.nMakeIndex == tdm.nRecog )
					{
						nPos = fnDecode6BitBuf((pszMsg + DEFBLOCKSIZE), (char*)pszItemName, sizeof(pszItemName));
						pszItemName[nPos] = '\0';

						if ( !strcmp(pszItemName, m_xInterface.m_stCommonItem.xItem.m_stItemInfo.stStdItem.szName) )
						{
							ZeroMemory(&m_xInterface.m_stCommonItem, sizeof(COMMONITEMSET));
						}
					}
					m_xInterface.m_stCommonItem.bWaitResponse = FALSE;
				}
				break;
			}
			case SM_DROPITEM_FAIL:
			{
				// 重新放回背包.
				CHAR	pszItemName[15];
				if (m_xInterface.m_stCommonItem.bWaitResponse )
				{
					if ( m_xInterface.m_stCommonItem.bSetted && m_xInterface.m_stCommonItem.xItem.m_stItemInfo.nMakeIndex == tdm.nRecog )
					{
						nPos = fnDecode6BitBuf((pszMsg + DEFBLOCKSIZE), (char*)pszItemName, sizeof(pszItemName));
						pszItemName[nPos] = '\0';

						if ( !strcmp(pszItemName, m_xInterface.m_stCommonItem.xItem.m_stItemInfo.stStdItem.szName) )
						{
							if ( !m_xInterface.m_stCommonItem.bIsBeltItem )
								m_xInterface.m_xInventoryWnd.AddInvenItem(m_xInterface.m_stCommonItem.xItem, m_xInterface.m_stCommonItem.xItem.m_shCellNum, TRUE);
							else
								m_xInterface.m_xInventoryWnd.AddBeltItem(m_xInterface.m_stCommonItem.xItem, m_xInterface.m_stCommonItem.xItem.m_shCellNum, TRUE);
							ZeroMemory(&m_xInterface.m_stCommonItem, sizeof(COMMONITEMSET));
						}
					}
					m_xInterface.m_stCommonItem.bWaitResponse = FALSE;
				}
                break;
			}
            case SM_DURACHANGE:
                m_xInterface.m_xStatusWnd.UpdateDurability(tdm.wParam,tdm.nRecog,tdm.wTag);
                break;
			case SM_GOLDCHANGE:
			{
                m_xInterface.m_xInventoryWnd.UpdateGold(UINT(tdm.nRecog));
				break;
			}
			case SM_RIDEHORSE:
			{
				m_xMyHero.m_stFeatureEx.bHorse = tdm.nRecog;
				m_xMyHero.ChangeFeature(m_xMyHero.m_stFeature, m_xMyHero.m_stFeatureEx);

				if ( m_xMyHero.m_stFeatureEx.bHorse != _HORSE_NONE )
					m_xMyHero.SetMotionFrame(_MT_HORSESTAND, m_xMyHero.m_bCurrDir);
				else
					m_xMyHero.SetMotionFrame(_MT_STAND, m_xMyHero.m_bCurrDir);

				break;
			}
			case SM_OPENDOOR_OK:
			{
				INT nIdx = m_xMap.GetDoor(tdm.wParam, tdm.wTag);
				if ( nIdx )
				{
					m_xMap.OpenDoor(tdm.wParam, tdm.wTag, nIdx);
				}
				break;
			}
			case SM_OPENDOOR_LOCK:
			{
				m_xInterface.m_xClientSysMsg.AddSysMsg("The door is locked.");
				break;
			}
			case SM_CLOSEDOOR:
			{
				INT nIdx = m_xMap.GetDoor(tdm.wParam, tdm.wTag);
				if ( nIdx )
				{
					m_xMap.CloseDoor(tdm.wParam, tdm.wTag, nIdx);
				}
				break;
			}
			case SM_HEALTHSPELLCHANGED: // gadget
			{
				if ( tdm.nRecog == m_xMyHero.m_dwIdentity )
				{
                    m_xMyHero.HealthDamage(m_xMyHero.m_wHP,tdm.wParam);
					m_xMyHero.m_wHP			= tdm.wParam;
					m_xMyHero.m_wMP			= tdm.wTag;
					m_xMyHero.m_wMAXHP		= tdm.wSeries;
					FLOAT wHPRate			= (FLOAT)((FLOAT)m_xMyHero.m_wHP/(FLOAT)m_xMyHero.m_wMAXHP);
					m_xMyHero.m_bHPPercent  = wHPRate * 100;
#ifdef _DEBUG
					_RPT2(_CRT_WARN, "HP:%d, MAXHP:%d\n", tdm.wParam, tdm.wSeries);
#endif
				}
				else
				{
					CActor* pxActor = FindActor(tdm.nRecog);

					if (pxActor)
					{
                        pxActor->HealthDamage(pxActor->m_wHP,tdm.wParam);
						pxActor->m_wHP			= tdm.wParam;
						pxActor->m_wMP			= tdm.wTag;
						pxActor->m_wMAXHP		= tdm.wSeries;
						FLOAT wHPRate			= (FLOAT)((FLOAT)pxActor->m_wHP/(FLOAT)pxActor->m_wMAXHP);
						pxActor->m_bHPPercent	= wHPRate * 100;
					}
				}

				break;
			}
            case SM_CREATEGROUP_OK:case SM_CREATEGROUP_FAIL:case SM_GROUPCANCEL:case SM_GROUPMEMBERS:
            case CGroupWnd::AddMemberFailed:case CGroupWnd::DeleteMemberFailed:
                m_xInterface.m_xGroupWnd.Receive(tdm,pszMsg+DEFBLOCKSIZE);break;
            case SM_OPENGUILDDLG:case SM_SENDGUILDMEMBERLIST:
            case GuildWire::SM_GUILD_OVERVIEW:case GuildWire::SM_GUILD_OVERVIEW_UPDATE:
            case GuildWire::SM_GUILD_MEMBERS_UPDATE:case GuildWire::SM_GUILD_MEMBERS_REFRESH:
                m_xInterface.m_xGuildWnd.Receive(tdm,pszMsg+DEFBLOCKSIZE);break;
            case SM_OPENGUILDDLG_FAIL:case SM_GUILDADDMEMBER_OK:case SM_GUILDADDMEMBER_FAIL:
            case SM_GUILDDELMEMBER_OK:case SM_GUILDDELMEMBER_FAIL:case SM_GUILDRANKUPDATE_FAIL:
            case SM_BUILDGUILD_OK:case SM_BUILDGUILD_FAIL:case SM_GUILDMAKEALLY_OK:case SM_GUILDMAKEALLY_FAIL:
            case SM_GUILDBREAKALLY_OK:case SM_GUILDBREAKALLY_FAIL:
                m_xInterface.m_xGuildWnd.ReceiveNotice(tdm);break;
			/*case SM_DOOROPEN:
			{
				BOOL bIsOpen = stDefMsg.nRecog;
				BYTE bDoorIdx = stDefMsg.wParam;

				m_xMap.SetDoorState(bDoorIdx, bIsOpen);
				break;
			}*/
            case SM_USERNAME:
            {
                CActor* actor=DWORD(tdm.nRecog)==m_xMyHero.m_dwIdentity?&m_xMyHero:FindActor(tdm.nRecog);
                if(actor) actor->ReceiveName(pszMsg+DEFBLOCKSIZE,LOBYTE(tdm.wParam));
                break;
            }
            case SM_CHANGEGUILDNAME:
                m_xInterface.m_xStatusWnd.ReceiveGuildIdentity(pszMsg+DEFBLOCKSIZE);
                break;
            case 1309: // MIR3 human hit requires an existing human, never creates an actor from HP coordinates.
            {
                CActor* actor=DWORD(tdm.nRecog)==m_xMyHero.m_dwIdentity?&m_xMyHero:FindActor(tdm.nRecog);
                if(!actor || actor->m_stFeature.bGender>1) break;
                BYTE body[16];if(strlen(pszMsg+DEFBLOCKSIZE)!=22 || fnDecode6BitBuf(pszMsg+DEFBLOCKSIZE,(char*)body,sizeof(body))!=16 || !CMir3HumanAppearance::Parse(body,sizeof(body),true)) break;
            }
            // Fall through to the existing actor message queue.
			default:
			{
                if(strlen(pszMsg+DEFBLOCKSIZE)>=ActorPacketBodyCapacity) break;
				LPPACKETMSG	lpPacketMsg	= new PACKETMSG;

				lpPacketMsg->stDefMsg			= tdm;
				lpPacketMsg->szEncodeData[0]	= NULL;

				if ( tdm.wIdent != SM_HIT && tdm.wIdent != SM_BACKSTEP && tdm.wIdent != SM_RUSH )
 					lstrcpy(lpPacketMsg->szEncodeData, (pszMsg + DEFBLOCKSIZE));

 				m_xWaitPacketQueue.PushQ((BYTE*)lpPacketMsg);

				break;
			}
		}
	}
}

VOID CGameProcess::ProcessDefaultPacket()
{
	_TDEFAULTMESSAGE*	lpstDefMsg;

	for (int i = 0; i < m_xWaitDefaultPacketQueue.GetCount(); i++)
	{
		lpstDefMsg = (_TDEFAULTMESSAGE*)m_xWaitDefaultPacketQueue.PopQ();

		if ( lpstDefMsg )
		{
			if ( lpstDefMsg->wIdent == SM_DISAPPEAR )
			{
				CActor* pxActor = NULL;

				m_xActorList.MoveCurrentToTop();
				for ( INT nCnt = 0; nCnt < m_xActorList.GetCounter(); nCnt++ )
				{
					pxActor = m_xActorList.GetCurrentData();
					
					if ( pxActor->m_dwIdentity == lpstDefMsg->nRecog )
					{
						INT nPacketCnt = pxActor->m_xPacketQueue.GetCount();
						for ( INT nLoopCnt = 0; nLoopCnt < nPacketCnt; nLoopCnt++ )
						{
							LPPACKETMSG	lpObjPacketMsg = (LPPACKETMSG)pxActor->m_xPacketQueue.PopQ();
							SAFE_DELETE(lpObjPacketMsg);
						}

						switch ( pxActor->m_stFeature.bGender )
						{
						case _GENDER_MAN:
						case _GENDER_WOMAN:
							{
								m_xActorList.DeleteCurrentNodeEx();
								CHero* pxHero = (CHero*)pxActor;
								delete((CHero*)pxHero);
								pxHero = NULL;
							}
							break;
						case _GENDER_NPC:
							{
								m_xActorList.DeleteCurrentNodeEx();
								CNPC* pxNPC = (CNPC*)pxActor;
								delete((CNPC*)pxNPC);
								pxNPC = NULL;
							}
							break;
						case _GENDER_MON:
							{
								m_xActorList.DeleteCurrentNodeEx();
								delete(pxActor);
								pxActor = NULL;
							}
							break;
						}						
						break;		//跳出 for 循环。
					}					
					m_xActorList.MoveNextNode();

				}
			}
			else if ( lpstDefMsg->wIdent == SM_CLEAROBJECT )
			{
				CActor* pxActor = NULL;

				m_xActorList.MoveCurrentToTop();
				for ( INT nCnt = 0; nCnt < m_xActorList.GetCounter(); )
				{
					pxActor = m_xActorList.GetCurrentData();
					
					INT nPacketCnt = pxActor->m_xPacketQueue.GetCount();
					for ( INT nLoopCnt = 0; nLoopCnt < nPacketCnt; nLoopCnt++ )
					{
						LPPACKETMSG	lpObjPacketMsg = (LPPACKETMSG)pxActor->m_xPacketQueue.PopQ();
						SAFE_DELETE(lpObjPacketMsg);
					}

					switch ( pxActor->m_stFeature.bGender )
					{
					case _GENDER_MAN:
					case _GENDER_WOMAN:
						{
							m_xActorList.DeleteCurrentNodeEx();
							CHero* pxHero = (CHero*)pxActor;
							delete((CHero*)pxHero);
							pxHero = NULL;
						}
						break;
					case _GENDER_NPC:
						{
							m_xActorList.DeleteCurrentNodeEx();
							CNPC* pxNPC = (CNPC*)pxActor;
							delete((CNPC*)pxNPC);
							pxNPC = NULL;
						}
						break;
					case _GENDER_MON:
						{
							m_xActorList.DeleteCurrentNodeEx();
							delete(pxActor);
							pxActor = NULL;
						}
						break;
					}
//					m_xActorList.MoveNextNode();
				}					
			}
		}
		SAFE_DELETE(lpstDefMsg);
	}						
}


VOID CGameProcess::ProcessPacket()
{
	LPPACKETMSG	lpPacketMsg;

	for (int i = 0; i < m_xWaitPacketQueue.GetCount(); i++)
	{
		BOOL bExist = FALSE;
		CActor* pxActor = NULL;

		lpPacketMsg = (LPPACKETMSG)m_xWaitPacketQueue.PopQ();

		if ( lpPacketMsg )
		{
			if ( lpPacketMsg->stDefMsg.nRecog == m_xMyHero.m_dwIdentity )
			{
				if ( lpPacketMsg->stDefMsg.wIdent == SM_NOWDEATH || lpPacketMsg->stDefMsg.wIdent == SM_DEATH )
				{
					m_xMyHero.m_xPriorPacketQueue.PushQ((BYTE*)lpPacketMsg);
				}
				else if ( lpPacketMsg->stDefMsg.wIdent == SM_CHANGEMAP )
				{
					DWORD dwFogColor;

					m_xMyHero.SetMotionFrame(_MT_STAND, m_xMyHero.m_bCurrDir);
					m_xMyHero.m_wPosX = lpPacketMsg->stDefMsg.wParam;	// + GAPX_TILE_CHAR_MAP;
					m_xMyHero.m_wPosY = lpPacketMsg->stDefMsg.wTag;		// + GAPY_TILE_CHAR_MAP;
					m_xMap.SetStartViewTile(m_xMyHero.m_wPosX - _GAPX_TILE_CHAR_MAP, m_xMyHero.m_wPosY - _GAPY_TILE_CHAR_MAP);

					char szDecodeMsg[MAX_PATH];
					int nPos = fnDecode6BitBuf(lpPacketMsg->szEncodeData, szDecodeMsg, sizeof(szDecodeMsg));
					szDecodeMsg[nPos] = '\0';

					LoadMapChanged(szDecodeMsg);

                    EnterSceneAmbient(LOBYTE(lpPacketMsg->stDefMsg.wSeries));
                    m_weather.select(HIBYTE(lpPacketMsg->stDefMsg.wSeries),true);

					// gadget			
					m_wInputDelayTime = 2300;
					m_wCurrInputDelayTime = 0;
					m_xMyHero.m_wABlendCurrDelay = 0;
					m_xMyHero.m_wABlendDelay = 2300;

					CloseNPCWindow();
				}
				else
				{
					if ( lpPacketMsg->stDefMsg.wIdent == SM_STRUCK || lpPacketMsg->stDefMsg.wIdent == 1309 )
					{
//						m_xMyHero.StruckMsgReassign();

						if ( lpPacketMsg->stDefMsg.wIdent == SM_STRUCK && timeGetTime() - m_xMyHero.m_dwLastStruckTime < 500 )
                        {
                            m_xMyHero.m_wHP = m_xMyHero.m_stAbility.wHP = lpPacketMsg->stDefMsg.wParam;
                            m_xMyHero.m_wMAXHP = m_xMyHero.m_stAbility.wMaxHP = lpPacketMsg->stDefMsg.wTag;
                            m_xMyHero.m_bHPPercent = lpPacketMsg->stDefMsg.wTag ? 100u*lpPacketMsg->stDefMsg.wParam/lpPacketMsg->stDefMsg.wTag : 0;
                            if (lpPacketMsg->stDefMsg.wParam) {
                                SAFE_DELETE(lpPacketMsg);
                                continue;
                            }
                        }

						MESSAGEBODYWL stMsgBodyWl;
						fnDecode6BitBuf(lpPacketMsg->szEncodeData, (char*)&stMsgBodyWl, sizeof(MESSAGEBODYWL));

						for ( INT nCnt = 0; nCnt < m_xActorList.GetCounter(); nCnt++ )
						{
							CActor* pxGaheja = m_xActorList.GetCurrentData();

							if ( pxGaheja->m_dwIdentity == stMsgBodyWl.nTag1 )
							{
								m_xMyHero.m_stHitter = pxGaheja->m_stFeature;
								break;
							}

							m_xActorList.MoveNextNode();
						}						
					}					
					
					m_xMyHero.StruckMsgReassign();
					m_xMyHero.m_xPacketQueue.PushQ((BYTE*)lpPacketMsg);
				}
			}
			else
			{
				FEATURE		stFeature;
//				FEATUREEX	stFeatureEx;

 				fnDecode6BitBuf(lpPacketMsg->szEncodeData, (char*)&stFeature, sizeof(FEATURE));

				m_xActorList.MoveCurrentToTop();
				
				for ( INT nCnt = 0; nCnt < m_xActorList.GetCounter(); nCnt++ )
				{
					pxActor = m_xActorList.GetCurrentData();
			
					if ( pxActor->m_dwIdentity == lpPacketMsg->stDefMsg.nRecog )
					{
						bExist = TRUE;
						break;
					}
				
					m_xActorList.MoveNextNode();
				}
				if ( !bExist )
				{
					if ( stFeature.bGender == _GENDER_MAN || stFeature.bGender == _GENDER_WOMAN ) 
					{
						pxActor = new CHero;
						CHero* pxHero = (CHero*)pxActor;

						pxHero->m_dwIdentity = lpPacketMsg->stDefMsg.nRecog;

						if ( pxHero->Create(&m_xImage, _MT_STAND, _DIRECTION_LIST_1, lpPacketMsg->stDefMsg.wParam, lpPacketMsg->stDefMsg.wTag, &stFeature, NULL) )
						{
							m_xActorList.AddNode(pxActor);
							pxHero->m_wABlendCurrDelay = 0;
							pxHero->m_wABlendDelay  = 1500;
						}
					}
					else if ( stFeature.bGender == _GENDER_NPC )
					{
						pxActor = new CNPC;
						CNPC* pxNPC = (CNPC*)pxActor;

						pxNPC->m_dwIdentity = lpPacketMsg->stDefMsg.nRecog;

						if ( pxNPC->Create(&m_xImage, &stFeature, _MT_STAND, _DIRECTION_LIST_1, lpPacketMsg->stDefMsg.wParam, lpPacketMsg->stDefMsg.wTag) )
							m_xActorList.AddNode(pxActor);

						pxNPC->m_wABlendCurrDelay = 0;
						pxNPC->m_wABlendDelay	  = 0;
					}
					else
//					else if ( stFeature.bGender == _GENDER_MON )
					{
						stFeature.bGender	= _GENDER_MON;

						// 需要应用.
//						if ( stFeature.bWeapon == 0 ) // 默认.
//						if ( stFeature.bWeapon == 1 ) // SKELLEON.

						pxActor = new CActor;
						pxActor->m_dwIdentity = lpPacketMsg->stDefMsg.nRecog;

						if ( pxActor->Create(&m_xImage, &stFeature, _MT_STAND, _DIRECTION_LIST_1, lpPacketMsg->stDefMsg.wParam, lpPacketMsg->stDefMsg.wTag) )
						{
							m_xActorList.AddNode(pxActor);
							pxActor->m_wABlendCurrDelay = 0;
							pxActor->m_wABlendDelay		= 150;
						}
					}
				}

				if ( pxActor )				
				{
					if ( lpPacketMsg->stDefMsg.wIdent == SM_STRUCK || lpPacketMsg->stDefMsg.wIdent == 1309 )
					{
//						pxActor->StruckMsgReassign();

						MESSAGEBODYWL stMsgBodyWl;
						fnDecode6BitBuf(lpPacketMsg->szEncodeData, (char*)&stMsgBodyWl, sizeof(MESSAGEBODYWL));

						for ( INT nCnt = 0; nCnt < m_xActorList.GetCounter(); nCnt++ )
						{
							CActor* pxGaheja = m_xActorList.GetCurrentData();

							if ( pxGaheja->m_dwIdentity == stMsgBodyWl.nTag1 )
							{
								pxActor->m_stHitter = pxGaheja->m_stFeature;
								break;
							}

							m_xActorList.MoveNextNode();
						}
						
					}

					pxActor->StruckMsgReassign();
					pxActor->m_xPacketQueue.PushQ((BYTE*)lpPacketMsg);
				}
				else							
					SAFE_DELETE(lpPacketMsg);
			}
		}
	}
}

// **************************************************************************************
//
//		 Packet Handling
//
// **************************************************************************************

VOID CGameProcess::OnSvrMsgNewMap(_TDEFAULTMESSAGE	*ptdm, char *pszMapName)
{
	DWORD dwFogColor;
	char szMapName[32];

	m_xMyHero.m_wPosX = ptdm->wParam;
	m_xMyHero.m_wPosY = ptdm->wTag;

	m_bDayState = LOBYTE(ptdm->wSeries);

	int nPos = fnDecode6BitBuf(pszMapName, (char*)szMapName, sizeof(szMapName));
	szMapName[nPos] = '\0';

	m_xMap.SetStartViewTile(m_xMyHero.m_wPosX - _GAPY_TILE_CHAR_MAP, m_xMyHero.m_wPosY - _GAPY_TILE_CHAR_MAP);
	m_xMap.LoadMapData(szMapName);
    m_xInterface.m_minimap.MapChanged();
    m_xInterface.m_xSiegeWnd.ClearObjects();
    m_levelEffects.Clear();ClearNativeSpells();m_stMapItemList.ClearAllNodes();
    m_xInterface.m_targetDisplay.Clear();

    EnterSceneAmbient(LOBYTE(ptdm->wSeries));
    m_weather.select(HIBYTE(ptdm->wSeries),true);

/*	g_xSound.PlayBkMusicMp3(TRUE, 0);
*/
	m_bRender = TRUE;
}

VOID CGameProcess::OnSvrMsgLogon(_TDEFAULTMESSAGE	*ptdm, char *pszMsg)
{
	MESSAGEBODYWL wl;

	m_xMyHero.m_dwIdentity	= ptdm->nRecog;
	m_xMyHero.m_bLightSize	= HIBYTE(ptdm->wSeries);

	BYTE bDir = LOBYTE(ptdm->wSeries);

	if ( bDir > 8 )
		bDir = 0;

	// 附加代码
	fnDecode6BitBuf(pszMsg, (char*)&wl, sizeof(wl));

//	if ( stFeatureEx.bHorse == _HORSE_NONE )
		m_xMyHero.Create(&m_xImage, _MT_STAND, bDir, ptdm->wParam/*x*/, ptdm->wTag/*y*/, (FEATURE *)&wl.lParam1, NULL);
//	else
//		m_xMyHero.Create(&m_xImage, _MT_HORSESTAND, bDir, m_xMyHero.m_wPosX, m_xMyHero.m_wPosY, stFeature, stFeatureEx);

	strcpy(g_xGameProc.m_xMyHero.m_szName, g_szCharName);
    ChatShortcuts().SetIdentity(g_szCharName,g_xClientSocket.selectedServerName);
    NativeChatLog().path="./Chat.txt";
	
	m_xMyHero.SetMapHandler(&m_xMap);

	CMagic* pxMagic;
	pxMagic = new CMagic;
	pxMagic->CreateMagic(_SKILL_SPACEMOVE2, m_xMyHero.m_wPosX, m_xMyHero.m_wPosY, m_xMyHero.m_wPosX, m_xMyHero.m_wPosY, NULL, m_xMyHero.m_dwIdentity);
	m_xMagicList.AddNode(pxMagic);	
	
	m_wInputDelayTime				= 2300;
	m_wCurrInputDelayTime			= 0;
	m_xMyHero.m_wABlendCurrDelay	= 0;
	m_xMyHero.m_wABlendDelay		= 2300;

	g_xClientSocket.SendQueryMsg(CM_QUERYBAGITEMS);
}

VOID CGameProcess::OnSvrMsgHear(_TDEFAULTMESSAGE *ptdm, char *pszMsg)
{
//	DWORD dwFontBackClr = m_xInterface.m_dwFontClrDfnList[ptdm->wParam];
//	DWORD dwFontClr		= m_xInterface.m_dwFontClrDfnList[ptdm->wTag];

	constexpr size_t MaximumEncodedChat=1024;
    if(strlen(pszMsg)>MaximumEncodedChat) return;
    char szDecodeMsg[MaximumEncodedChat+1];

	int nPos = fnDecode6BitBuf(pszMsg, szDecodeMsg, MaximumEncodedChat);
	szDecodeMsg[nPos] = '\0';
    NativeChatLog().Append(ptdm->wIdent,szDecodeMsg);

	const bool guildMessage=ptdm->wIdent==SM_GUILDMESSAGE;
    DWORD dwBack = GetChatColor(guildMessage?_CHAT_COLOR1:LOBYTE(ptdm->wTag));
	DWORD dwFont = GetChatColor(guildMessage?_CHAT_COLOR5:LOBYTE(ptdm->wParam));
    if(guildMessage) m_xInterface.m_xGuildWnd.AppendHistory(szDecodeMsg);
    constexpr int MaximumChatBytes=200;
    if(nPos<=MaximumChatBytes) m_xInterface.MsgAdd(dwFont, dwBack, szDecodeMsg);

    if(ptdm->wIdent==SM_HEAR || ptdm->wIdent==SM_MONSTERSAY) {
        CActor* actor=DWORD(ptdm->nRecog)==m_xMyHero.m_dwIdentity?&m_xMyHero:FindActor(ptdm->nRecog);
        if(actor) actor->ReceiveSpeech(szDecodeMsg);
    }
}

void CGameProcess::SetShadowBlend(bool enabled)
{
    m_bShadowABlend=enabled;m_xInterface.m_xOptionWnd.m_values[4]=enabled;
    const WORD color=g_xMainWnd.ConvertColor24To16(enabled?RGB(0,0,0):RGB(20,20,20));
    for(int i=0;i<800;++i) m_wShadowClr[i]=color;
}

void CGameProcess::UpdateTargetDisplay(CActor* actor,DWORD now)
{
    auto& target=m_xInterface.m_targetDisplay;
    if(!actor || actor->m_bIsDead || actor->m_stFeature.bGender==_GENDER_NPC) {target.Focus(0,now);return;}
    const BYTE type=actor->m_stFeature.bGender;
    if(type<=_GENDER_WOMAN) {
        // Remote legacy actors have no profession field; use the reference unknown-job frame.
        target.SetHuman(actor->m_dwIdentity,type,actor->m_bProfession!=255?actor->m_bProfession:actor==&m_xMyHero?m_xMyHero.m_bJob:255,now);
        target.QueryPortrait(actor->m_szName);
    } else if(type==_GENDER_MON) {
        const bool changed=target.ChangesTo(actor->m_dwIdentity);
        target.SetMonster(actor->m_dwIdentity,type,actor->m_stFeature.bDress,false,0,
            actor->m_wHP,actor->m_wMAXHP,actor->m_bHPPercent,now);
        if(changed && target.attributes[actor->m_dwIdentity].icon==-1) {
            _TDEFAULTMESSAGE packet;fnMakeDefMessage(&packet,1047,actor->m_dwIdentity,actor->m_wPosX,actor->m_wPosY,0);
            g_xClientSocket.SendPacket(&packet,NULL);
        }
    } else target.Focus(0,now);
}

// Update the native status page and HUD from the same validated MIR3 record.
bool CGameProcess::ReceiveAbility(const _TDEFAULTMESSAGE& header,const char* encoded)
{
    using namespace AbilityWire;
    if(header.wIdent!=SM_ABILITY || LOBYTE(header.wParam)>MaximumProfession || strlen(encoded)!=EncodedSize) return false;
    BYTE bytes[Size+1]={};
    if(fnDecode6BitBuf(const_cast<char*>(encoded),reinterpret_cast<char*>(bytes),sizeof(bytes))!=Size) return false;
    const auto word=[&](int at) {return MAKEWORD(bytes[at],bytes[at+1]);};
    const auto dword=[&](int at) {return DWORD(word(at))|(DWORD(word(at+2))<<16);};
    ACTORABILITY ability={};
    ability.bLevel=bytes[Level];ability.wAC=word(Defence);ability.wMAC=word(MagicDefence);
    ability.wDC=word(Attack);ability.wMC=word(Magic);ability.wSC=word(Spirit);
    ability.wHP=word(Health);ability.wMP=word(Mana);ability.wMaxHP=word(MaxHealth);ability.wMaxMP=word(MaxMana);
    ability.dwExp=dword(Experience);ability.dwMaxExp=dword(NextExperience);
    ability.wWeight=word(Weight);ability.wMaxWeight=word(MaxWeight);
    ability.bWearWeight=bytes[Wear];ability.bMaxWearWeight=bytes[MaxWear];
    ability.bHandWeight=bytes[Hand];ability.bMaxHandWeight=bytes[MaxHand];
    m_xMyHero.m_stAbility=ability;m_xMyHero.m_nGlod=DWORD(header.nRecog);m_xMyHero.m_bJob=LOBYTE(header.wParam);
    m_xMyHero.m_wHP=ability.wHP;m_xMyHero.m_wMAXHP=ability.wMaxHP;m_xMyHero.m_wMP=ability.wMP;
    m_xMyHero.m_bHPPercent=ability.wMaxHP?BYTE(min(100u,unsigned(ability.wHP)*100/ability.wMaxHP)):0;
    auto& status=m_xInterface.m_xStatusWnd;
    memcpy(status.m_abilityExtension,bytes+Extension,ExtensionSize);
    status.m_abilityHeaderOptions[0]=LOBYTE(header.wTag);
    status.m_abilityHeaderOptions[1]=BYTE(HIBYTE(header.wTag)*16);
    status.m_abilityHeaderOptions[2]=BYTE(LOBYTE(header.wSeries)*16);
    status.m_abilityHeaderOptions[3]=BYTE(HIBYTE(header.wSeries)*16);
    for(int i=0;i<ElementCount;++i) {
        status.m_elementValues[0][i]=bytes[AttackElements+2*i];
        status.m_elementValues[1][i]=bytes[StrengthElements+i];
        status.m_elementValues[2][i]=bytes[WeaknessElements+i];
    }
    return true;
}

bool CGameProcess::ReceiveProgression(const _TDEFAULTMESSAGE& header)
{
    static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
    const auto post=[&](const std::string& text,int background=_CHAT_COLOR4) {
        m_xInterface.MsgAdd(GetChatColor(_CHAT_COLOR3),GetChatColor(background),const_cast<char*>(text.c_str()));
    };
    auto& ability=m_xMyHero.m_stAbility;
    switch(header.wIdent) {
    case SM_WINEXP: {
        ability.dwExp=DWORD(header.nRecog);
        if(header.wTag) m_weaponTraining=LOBYTE(header.wSeries)?header.wTag:WORD(m_weaponTraining+header.wTag);
        if(HIBYTE(header.wSeries)) post(captions.Get(Progression::ExtraCaption));
        std::string text=captions.Get(header.wTag?Progression::TrainingCaption:Progression::ExperienceCaption);
        size_t begin=0;const unsigned values[]={header.wParam,header.wTag};
        for(int i=0;i<(header.wTag?2:1);++i) {
            const auto at=text.find("%d",begin);if(at==text.npos) break;
            const auto value=std::to_string(values[i]);text.replace(at,2,value);begin=at+value.size();
        }
        post(text);return true;
    }
    case SM_SUBABILITY: {
        auto& value=m_xMyHero.m_stSubAbility;
        value.wAntiMagic=LOBYTE(header.nRecog);value.bHitPoint=LOBYTE(header.wParam);value.bSpeedPoint=HIBYTE(header.wParam);
        value.bAntiPoison=LOBYTE(header.wTag);value.bPoisonRecover=HIBYTE(header.wTag);
        value.bHealthRecover=LOBYTE(header.wSeries);value.bSpellRecover=HIBYTE(header.wSeries);return true;
    }
    case SM_WEIGHTCHANGED:
        ability.wWeight=WORD(header.nRecog);ability.bWearWeight=LOBYTE(header.wParam);ability.bHandWeight=LOBYTE(header.wTag);return true;
    case SM_LEVELUP: {
        CActor* actor=nullptr;
        if(header.wSeries==0) {ability.dwExp=DWORD(header.nRecog);ability.bLevel=LOBYTE(header.wParam);actor=&m_xMyHero;}
        else if(header.wSeries==1) {
            m_xActorList.MoveCurrentToTop();
            for(int i=0;i<m_xActorList.GetCounter();++i) {
                auto* candidate=m_xActorList.GetCurrentData();m_xActorList.MoveNextNode();
                if(candidate->m_dwIdentity==DWORD(header.nRecog) && (candidate->m_stFeature.bGender==_GENDER_MAN || candidate->m_stFeature.bGender==_GENDER_WOMAN)) {actor=candidate;break;}
            }
        } else return true;
        if(!actor) return true;
        actor->SetMotionFrame(_MT_ATTACKMODE,Progression::LevelDirection);
        m_levelEffects.Start({actor->m_wPosX,actor->m_wPosY});
        if(header.wSeries==0) post(captions.Get(Progression::LevelCaption),_CHAT_COLOR8);
        return true;
    }
    default:return false;
    }
}


bool CGameProcess::CheckExitAllowed(DWORD now)
{
    if(m_exitGuard.Allowed(now,m_xMyHero.m_bCurrMtn)) return true;
    static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
    m_xInterface.MsgAdd(GetChatColor(_CHAT_COLOR3),GetChatColor(_CHAT_COLOR4),const_cast<char*>(captions.Get(ExitFlow::BlockedCaption).c_str()));
    return false;
}
bool CGameProcess::ConfirmExit(int action,DWORD now)
{
    if(g_bProcState!=_GAME_PROC || m_exitTransition.active || (action!=ExitFlow::ProgramDialog && action!=ExitFlow::CharacterDialog) || !CheckExitAllowed(now)) return false;
    if(action==ExitFlow::ProgramDialog) {SendMessage(g_xMainWnd.GetSafehWnd(),WM_DESTROY,0,0);return true;}
    if(!g_xChrSelProc.PrepareReturn()) return false;
    g_xClientSocket.SendQueryMsg(ExitFlow::Logout);
    m_xInterface.m_xInventoryWnd.SaveItemPos(m_xMyHero.m_szName);
    m_exitTransition.Begin();return true;
}
void CGameProcess::DrawExitTransition()
{
    if(!m_exitTransition.active) return;
    DDSURFACEDESC2 surface={};surface.dwSize=sizeof(surface);auto* buffer=g_xMainWnd.GetBackBuffer();
    if(FAILED(buffer->Lock(nullptr,&surface,DDLOCK_WAIT,nullptr))) return;
    const float brightness=m_exitTransition.Brightness();
    for(DWORD y=0;y<surface.dwHeight;++y) {
        WORD* row=(WORD*)((BYTE*)surface.lpSurface+y*surface.lPitch);
        for(DWORD x=0;x<surface.dwWidth;++x) {const WORD p=row[x];row[x]=WORD((int((p>>11)*brightness)<<11)|(int(((p>>5)&63)*brightness)<<5)|int((p&31)*brightness));}
    }
    buffer->Unlock(nullptr);
}
void CGameProcess::ReturnToSelection()
{
    g_xSound.StopSceneAudio();
    while(PacketQ.GetCount()) delete[] (char*)PacketQ.PopQ();
    nBuff=0;szBuff[0]=0;
    DeleteProc();
    g_xClientSocket.m_pxDefProc=g_xMainWnd.m_pxDefProcess=&g_xChrSelProc;g_bProcState=_CHAR_SEL_PROC;
    g_xMainWnd.ResetDXG(640,480,_SCREEN_BPP,_DXG_SCREENMODE_WINDOW,_DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D);
    g_xChrSelProc.LoadReturn();
}


bool CGameProcess::StartNativeSpell(CActor& source,WORD skill,POINT target,DWORD targetId)
{
    const float deltaX=float((target.x-source.m_wPosX)*_CELL_WIDTH);
    const float deltaY=float((target.y-source.m_wPosY)*_CELL_HEIGHT);
    if(skill==WORD(mir3::SkillId::original_150)) {
        const bool fireball=StartNativeSpell(source,WORD(mir3::SkillId::fireball),target,0);
        if(deltaX==0 && deltaY==0) return fireball;
        const auto* resource=mir3::EffectCatalog::shared().find(mir3::EffectCatalog::Section::magic,skill);
        if(!resource || !NativeEffects::Image(resource->library)) return fireball;
        const int direction=mir3::SpellDirection(deltaX,deltaY);
        constexpr DWORD SecondOrbitDelay=30;
        for(int i=0;i<2;++i) {
            NativeSpell effect;effect.serial=m_nextNativeSpell++;
            effect.orbit=mir3::SpellOrbit{0,i!=0};effect.orbitDirection=unsigned(direction);
            effect.playback.begin(mir3::SkillId::original_150,*resource,
                {float(source.m_wPosX*_CELL_WIDTH),float(source.m_wPosY*_CELL_HEIGHT)},
                {float(target.x*_CELL_WIDTH),float(target.y*_CELL_HEIGHT)},direction,false,i*SecondOrbitDelay);
            effect.playback.particle_only=true;
            effect.soundSample=int(skill)*10+10001;
            effect.soundVoice=PlaySpellSound(effect.soundSample,{source.m_wPosX,source.m_wPosY});
            m_nativeSpells.push_back(effect);
        }
        return true;
    }
    const auto* resource=mir3::EffectCatalog::shared().find(mir3::EffectCatalog::Section::magic,skill);
    if(!resource || resource->end<=resource->first || !resource->interval || !NativeEffects::Image(resource->library)) return false;
    const auto rules=mir3::MagicRules::for_skill(static_cast<mir3::SkillId>(skill));
    if(rules.reject_same_cell && source.m_wPosX==target.x && source.m_wPosY==target.y) return false;
    CActor* actor=targetId==m_xMyHero.m_dwIdentity?&m_xMyHero:FindActor(int(targetId));
    if(!rules.track_target || !targetId) actor=nullptr;
    const int direction=rules.direction==mir3::MagicDirection::fixed?0:
        mir3::SpellDirection(deltaX,deltaY,rules.direction==mir3::MagicDirection::eight);
    NativeSpell effect;effect.serial=m_nextNativeSpell++;
    effect.target=actor?targetId:0;effect.targetInstance=actor?actor->m_instanceSerial:0;
    effect.playback.begin(static_cast<mir3::SkillId>(skill),*resource,
        {float(source.m_wPosX*_CELL_WIDTH),float(source.m_wPosY*_CELL_HEIGHT)},
        {float(target.x*_CELL_WIDTH),float(target.y*_CELL_HEIGHT)},direction,actor!=nullptr);
    if(skill==WORD(mir3::SkillId::original_73) || skill==WORD(mir3::SkillId::original_74)) {
        const bool sourceAnchor=skill==WORD(mir3::SkillId::original_73);
        effect.anchor=sourceAnchor?mir3::SpellFlight::Point{float(source.m_wPosX*_CELL_WIDTH),float(source.m_wPosY*_CELL_HEIGHT)}:
            mir3::SpellFlight::Point{float(target.x*_CELL_WIDTH),float(target.y*_CELL_HEIGHT)};
        const unsigned facing=unsigned(mir3::SpellDirection(deltaX,deltaY))>>1;
        effect.staged.emplace(static_cast<mir3::SkillId>(skill),facing,
            mir3::StagedSpell::Frames{int(resource->first),int(resource->end),resource->interval});
        effect.playback.bound=false;effect.playback.frame_offset=0;
        effect.playback.position={effect.anchor[0]+effect.staged->offset[0],effect.anchor[1]+effect.staged->offset[1]};
    }
    const POINT soundCell=rules.at_target && !rules.use_source_as_target?target:POINT{source.m_wPosX,source.m_wPosY};
    effect.soundSample=int(skill)*10+(effect.playback.flying?10001:10002);
    effect.soundVoice=PlaySpellSound(effect.soundSample,soundCell);
    m_nativeSpells.push_back(effect);
    return true;
}
void CGameProcess::ClearNativeSpells()
{
    m_nativeSpells.clear();m_spellMarks.clear();m_spellParticles.clear();g_xSound.ClearSpellVoices();
}
void CGameProcess::StartGroundSpell(mir3::MagicEffectId id,POINT source,POINT target,int facing)
{
    const auto* resource=mir3::EffectCatalog::shared().find(mir3::EffectCatalog::Section::magic,WORD(id));
    const bool particles=id==mir3::MagicEffectId::lightning_trail;
    if(!resource || (!particles && (resource->end<=resource->first || !resource->interval)) || !NativeEffects::Image(resource->library)) return;
    for(const auto& segment:mir3::GroundSpell::segments(id,{int(source.x),int(source.y)},{int(target.x),int(target.y)},facing)) {
        NativeSpell effect;effect.serial=m_nextNativeSpell++;effect.groundWave=true;
        effect.cell={segment.cell[0],segment.cell[1]};
        auto& playback=effect.playback;
        playback.resource=resource;playback.skill=static_cast<mir3::SkillId>(id);
        playback.frame=int(resource->first);playback.delay=segment.delay;
        playback.position={float(effect.cell.x*_CELL_WIDTH),float(effect.cell.y*_CELL_HEIGHT)};
        playback.light=resource->outer_radius!=0;
        m_nativeSpells.push_back(effect);
        PlaySpellSound(int(id)*10+10002,effect.cell);
    }
}
void CGameProcess::StartSpellEmber(POINT cell,int id)
{
    const auto* resource=mir3::EffectCatalog::shared().find(mir3::EffectCatalog::Section::magic,WORD(id));
    if(!resource || resource->end<=resource->first || !resource->interval || !NativeEffects::Image(resource->library)) return;
    NativeSpell effect;effect.serial=m_nextNativeSpell++;effect.ember=true;effect.cell=cell;
    effect.remaining=SpellEmberLifetime;
    auto& playback=effect.playback;playback.resource=resource;
    playback.frame=int(resource->first);playback.one_shot=false;
    playback.position={float(cell.x*_CELL_WIDTH),float(cell.y*_CELL_HEIGHT)};
    m_nativeSpells.push_back(effect);
    PlaySpellSound(id*10+10002,cell);
}
void CGameProcess::AdvanceNativeSpells(DWORD elapsed,std::uint64_t before)
{
    for(auto& mark:m_spellMarks) mark.age+=elapsed;
    m_spellMarks.erase(std::remove_if(m_spellMarks.begin(),m_spellMarks.end(),[](const SpellMark& mark){return mark.age>SpellMarkLifetime;}),m_spellMarks.end());
    std::vector<std::pair<POINT,int>> embers;
    const int originX=_VIEW_CELL_X_START-m_xMap.m_shStartViewTileX*_CELL_WIDTH-m_xMap.m_shViewOffsetX;
    const int originY=_VIEW_CELL_Y_START-m_xMap.m_shStartViewTileY*_CELL_HEIGHT-m_xMap.m_shViewOffsetY;
    for(auto& effect:m_nativeSpells) {
        if(effect.serial>=before) continue;
        const auto rememberFrame=[&]() {
            const auto& playback=effect.playback;
            if(effect.orbit || playback.finished || playback.delay || (playback.tail && !effect.groundWave) ||
                (effect.groundWave && playback.resource->id==WORD(mir3::MagicEffectId::lightning_trail))) return;
            auto* image=NativeEffects::Image(playback.resource->library);
            if(image && image->NewSetIndex(playback.frame))
                m_spellLibraryOffsets[playback.resource->library]={image->m_lpstNewCurrWilImageInfo->shPX,image->m_lpstNewCurrWilImageInfo->shPY};
        };
        if(effect.groundWave || effect.ember) {
            auto& playback=effect.playback;playback.elapsed+=elapsed;
            if(effect.ember) {
                effect.remaining=(std::max)(0,effect.remaining-int(elapsed));
                if(!effect.remaining) {
                    effect.tint=(std::max)(0,effect.tint-5);
                    if(!effect.tint) {playback.stop();continue;}
                }
            }
            DWORD lightElapsed=elapsed;
            if(playback.delay) {
                if(playback.elapsed<playback.delay) continue;
                playback.elapsed-=playback.delay;playback.delay=0;
                lightElapsed=playback.elapsed;
            }
            // Ground light fades before animation advancement, unlike common projectiles.
            if(playback.advance_light(lightElapsed,true)==mir3::SpellPlayback::Change::finished) continue;
            const auto id=static_cast<mir3::MagicEffectId>(playback.resource->id);
            const bool walkable=effect.cell.x>=0 && effect.cell.y>=0 &&
                effect.cell.x<m_xMap.m_stMapFileHeader.shWidth && effect.cell.y<m_xMap.m_stMapFileHeader.shHeight &&
                m_xMap.GetTileAttribute(effect.cell.x,effect.cell.y)==_CAN_WALK;
            const int interval=effect.groundWave?mir3::GroundSpell::interval(id,playback.frame,playback.resource->interval,walkable):playback.resource->interval;
            if(!playback.tail && playback.elapsed>DWORD(interval)) {
                playback.elapsed=0;
                if(effect.ember) {
                    if(++playback.frame>=int(playback.resource->end)) playback.frame=int(playback.resource->first);
                    rememberFrame();continue;
                }
                const auto change=mir3::GroundSpell::next(id,playback.frame,int(playback.resource->first),int(playback.resource->end),walkable);
                const float x=float(effect.cell.x*_CELL_WIDTH+originX+_CELL_WIDTH/2);
                const float y=float(effect.cell.y*_CELL_HEIGHT+originY);
                if(change.lightning) m_spellParticles.emit_lightning({x,y-_CELL_HEIGHT/2},m_weather);
                if(change.residue) {
                    const bool fire=id==mir3::MagicEffectId::fire_wave;
                    if(fire) embers.push_back({effect.cell,m_weather.between(FirstEmber,LastEmber)});
                    m_spellMarks.push_back({effect.cell,fire?FireMark:IceMark+m_weather.between(0,1),elapsed});
                }
                if(change.ice_particles) m_spellParticles.emit_ice({x,y},m_weather);
                if(change.finished) {
                    if(!playback.light) playback.stop();
                    else {playback.tail=true;--playback.frame;}
                }
            }
            rememberFrame();continue;
        }
        if(effect.staged) {
            auto& playback=effect.playback;auto& staged=*effect.staged;
            if(!playback.tail) {
                const auto change=staged.advance(elapsed);
                if(change==mir3::StagedSpell::Change::load_explosion) {
                    const auto* resource=mir3::EffectCatalog::shared().find(mir3::EffectCatalog::Section::explosion,WORD(playback.skill));
                    if(!resource || resource->end<=resource->first || !resource->interval || !NativeEffects::Image(resource->library)) {playback.stop();continue;}
                    playback.resource=resource;
                    staged.load_explosion({int(resource->first),int(resource->end),resource->interval});
                }
                playback.frame=staged.frame;
                playback.position={effect.anchor[0]+staged.offset[0],effect.anchor[1]+staged.offset[1]};
                if(change==mir3::StagedSpell::Change::finished) {playback.tail=true;--playback.frame;}
            }
            playback.advance_light(elapsed);rememberFrame();continue;
        }
        std::optional<mir3::SpellFlight::Point> target;
        if(effect.playback.bound) {
            CActor* actor=effect.target==m_xMyHero.m_dwIdentity?&m_xMyHero:FindActor(int(effect.target));
            if(actor && actor->m_instanceSerial==effect.targetInstance) {
                if(actor==&m_xMyHero) {
                    // Local rendering is screen-centered while the camera scrolls.
                    target=mir3::SpellFlight::Point{
                        float(m_xMap.m_shStartViewTileX*_CELL_WIDTH+m_xMap.m_shViewOffsetX-_VIEW_CELL_X_START+_CHAR_CENTER_XPOS-_CELL_WIDTH/2),
                        float(m_xMap.m_shStartViewTileY*_CELL_HEIGHT+m_xMap.m_shViewOffsetY-_VIEW_CELL_Y_START+_CHAR_CENTER_YPOS-_CELL_HEIGHT/2)};
                } else target=mir3::SpellFlight::Point{
                    float(actor->m_wPosX*_CELL_WIDTH+actor->m_shShiftPixelX),
                    float(actor->m_wPosY*_CELL_HEIGHT+actor->m_shShiftPixelY)};
            }
        }
        const auto change=effect.playback.advance(elapsed,target,mir3::EffectCatalog::shared());
        if(effect.soundVoice && effect.playback.flying && !effect.playback.finished && !effect.playback.delay &&
            (mir3::SpellSoundVoices::key(0,0)-effect.soundVoice)%mir3::SpellSoundVoices::voices<5) {
            // The original follows only the first five voices; positions are quantized to cells.
            const auto& position=effect.playback.flight.position;
            const int cellX=int(position[0]/_CELL_WIDTH),cellY=int(position[1]/_CELL_HEIGHT);
            g_xSound.PositionSpellWav(effect.soundVoice,effect.soundSample,cellX*_CELL_WIDTH+originX,cellY*_CELL_HEIGHT+originY);
        }
        if(effect.orbit && !effect.playback.finished && !effect.playback.delay && effect.playback.flying && !effect.playback.tail && elapsed) {
            auto& playback=effect.playback;
            const auto pixel=effect.orbit->advance(playback.flight.position[0],playback.flight.position[1],effect.orbitDirection);
            playback.position={float(pixel[0]),float(pixel[1])};
            const auto offset=m_spellLibraryOffsets[playback.resource->library];
            const auto emitter=mir3::SpellOrbit::emitter({pixel[0]+originX,pixel[1]+originY},effect.orbitDirection,offset.x,offset.y);
            m_spellParticles.emit({float(emitter[0]),float(emitter[1])},m_weather);
        }
        rememberFrame();
        if(change==mir3::SpellPlayback::Change::impact && effect.playback.bound) {
            const auto& position=effect.playback.position;
            PlaySpellSound(int(effect.playback.skill)*10+10002,{int(position[0])/_CELL_WIDTH,int(position[1])/_CELL_HEIGHT});
        }
    }
    for(const auto& ember:embers) StartSpellEmber(ember.first,ember.second);
    if(elapsed) m_spellParticles.advance(int(elapsed),m_weather);
    g_xSound.RefreshSpellVoices();
    m_nativeSpells.erase(std::remove_if(m_nativeSpells.begin(),m_nativeSpells.end(),[](const NativeSpell& effect){return effect.playback.finished;}),m_nativeSpells.end());
}
void CGameProcess::DrawNativeSpells()
{
    const int originX=_VIEW_CELL_X_START-m_xMap.m_shStartViewTileX*_CELL_WIDTH-m_xMap.m_shViewOffsetX;
    const int originY=_VIEW_CELL_Y_START-m_xMap.m_shStartViewTileY*_CELL_HEIGHT-m_xMap.m_shViewOffsetY;
    for(const auto& entry:m_nativeSpells) {
        const auto& effect=entry.playback;
        if(effect.finished || effect.delay || entry.orbit) continue;
        const int x=int(effect.position[0])+originX,y=int(effect.position[1])+originY;
        if(entry.groundWave && effect.resource->id==WORD(mir3::MagicEffectId::lightning_trail)) continue;
        if(!effect.tail || entry.groundWave) {
            auto resource=*effect.resource;
            if(entry.ember) resource.tint={BYTE(entry.tint),BYTE(entry.tint),BYTE(entry.tint)};
            NativeEffects::Draw(resource,effect.frame,x,y);
        }

    }
}

void CGameProcess::LightNativeSpells()
{
    if(!m_xInterface.m_xOptionWnd.m_values[9]) return;
    const int originX=_VIEW_CELL_X_START-m_xMap.m_shStartViewTileX*_CELL_WIDTH-m_xMap.m_shViewOffsetX;
    const int originY=_VIEW_CELL_Y_START-m_xMap.m_shStartViewTileY*_CELL_HEIGHT-m_xMap.m_shViewOffsetY;
    for(const auto& entry:m_nativeSpells) {
        const auto& effect=entry.playback;
        if(effect.finished || effect.delay) continue;
        const int x=int(effect.position[0])+originX,y=int(effect.position[1])+originY;
        if(effect.light) {
            const auto& r=*effect.resource;
            m_xLightFog.SetLightRadiusWithRing(x+_CELL_WIDTH/2,y+_CELL_HEIGHT/2,
                int(r.inner_radius)+effect.light_phase-effect.light_shrink,
                mir3::SpellLight::channel(r.inner_color[0],effect.light_fade),mir3::SpellLight::channel(r.inner_color[1],effect.light_fade),mir3::SpellLight::channel(r.inner_color[2],effect.light_fade),
                int(r.outer_radius)+effect.light_phase-effect.light_shrink,
                mir3::SpellLight::channel(r.outer_color[0],effect.light_fade),mir3::SpellLight::channel(r.outer_color[1],effect.light_fade),mir3::SpellLight::channel(r.outer_color[2],effect.light_fade));
        }
    }
}

void CGameProcess::DrawSpellMarks()
{
    if(m_spellMarks.empty()) return;
    DDSURFACEDESC2 surface={};surface.dwSize=sizeof(surface);auto* buffer=g_xMainWnd.GetBackBuffer();
    if(FAILED(buffer->Lock(nullptr,&surface,DDLOCK_WAIT,nullptr))) return;
    const int originX=_VIEW_CELL_X_START-m_xMap.m_shStartViewTileX*_CELL_WIDTH-m_xMap.m_shViewOffsetX;
    const int originY=_VIEW_CELL_Y_START-m_xMap.m_shStartViewTileY*_CELL_HEIGHT-m_xMap.m_shViewOffsetY;
    for(const auto& mark:m_spellMarks) {
        if(!mark.age) continue;
        const auto* sprite=NativeSpellSprites::Frame(mark.frame);if(!sprite) continue;
        NativeSpellSprites::Draw(surface,*sprite,float(mark.cell.x*_CELL_WIDTH+originX+sprite->offsetX),
            float(mark.cell.y*_CELL_HEIGHT+originY+sprite->offsetY),float(sprite->width),float(sprite->height),
            {255,255,255},NativeSpellSprites::Blend::mark);
    }
    buffer->Unlock(nullptr);
}
void CGameProcess::DrawSpellParticles()
{
    if(!m_spellParticles.count()) return;
    DDSURFACEDESC2 surface={};surface.dwSize=sizeof(surface);auto* buffer=g_xMainWnd.GetBackBuffer();
    if(FAILED(buffer->Lock(nullptr,&surface,DDLOCK_WAIT,nullptr))) return;
    for(const auto* slots:{&m_spellParticles.sparks,&m_spellParticles.glows,&m_spellParticles.fragments})
        for(const auto& slot:*slots) {
            if(!slot || slot->size<=0) continue;
            const auto& particle=*slot;
            const auto* sprite=NativeSpellSprites::Frame(particle.resource_frame());if(!sprite) continue;
            NativeSpellSprites::Draw(surface,*sprite,particle.position[0]-particle.size*.5f,
                particle.position[1]-particle.size*.5f,particle.size,particle.size,particle.color,
                particle.glow()?NativeSpellSprites::Blend::glow:NativeSpellSprites::Blend::spark);
        }
    buffer->Unlock(nullptr);
}

std::int64_t CGameProcess::PlaySpellSound(int sample,POINT cell)
{
    const int x=cell.x*_CELL_WIDTH+_VIEW_CELL_X_START-m_xMap.m_shStartViewTileX*_CELL_WIDTH-m_xMap.m_shViewOffsetX;
    const int y=cell.y*_CELL_HEIGHT+_VIEW_CELL_Y_START-m_xMap.m_shStartViewTileY*_CELL_HEIGHT-m_xMap.m_shViewOffsetY;
    return g_xSound.PlaySpellWav(sample,x,y);
}
