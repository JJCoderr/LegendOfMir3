/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/



#include "StdAfx.h"
#include <charconv>
#include "Common/LoginMessages.h"
#include "Common/LegacyPanel.h"



/******************************************************************************************************************

	CStatusWnd Class

*******************************************************************************************************************/

CStatusWnd::CStatusWnd()
{
	Init();
}


CStatusWnd::~CStatusWnd()
{
	Destory();
}


VOID CStatusWnd::Init()
{
	CGameWnd::Init();

    m_nativeClosePressed=false;
    ZeroMemory(m_elementValues,sizeof(m_elementValues)); m_showNativeHint=false;
    m_identityTertiary.clear(); m_identityBadge.clear();
	ZeroMemory(&m_rcCell, sizeof(RECT)*_EQUIP_MAX_CELL);
	ZeroMemory(&m_stEquipItem, sizeof(ITEMSET)*_EQUIP_MAX_CELL);
	ZeroMemory(&m_stTakeOnItemSet, sizeof(CItem));
	ZeroMemory(&m_szEquipItemIdx, sizeof(m_szEquipItemIdx));



}

VOID CStatusWnd::Destory()
{
	CGameWnd::Destroy();
	Init();
}


VOID CStatusWnd::CreateStatusWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove)
{
	CreateGameWnd(nID, pxWndImage, nFrameImgIdx, bCanMove, nStartX, nStartY, nWidth, nHeight);
    const int slots[12][4]={{95,114,50,90},{40,70,50,95},{90,265,38,38},{170,88,38,38},
        {95,70,38,34},{12,155,38,38},{170,155,38,38},{12,195,38,38},{170,195,38,38},{130,265,38,38},{12,255,38,38},{170,265,38,38}};
    for (int i=0;i<12;++i) SetRect(&m_rcCell[i],slots[i][0],slots[i][1],slots[i][0]+slots[i][2],slots[i][1]+slots[i][3]);

	// 魔法窗口初始化.

	// 魔法变量初始化.
}



VOID CStatusWnd::ShowStatusWnd()
{
    ShowNativeStatus();
}













/******************************************************************************************************************

	角色装备窗口相关.

*******************************************************************************************************************/

VOID CStatusWnd::ShowCharEquipWnd()
{
	INT nStartX, nStartY;

	ShowGameWnd();

	// 区分是男性还是女性.
	if ( g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].NewSetIndex(g_xGameProc.m_xMyHero.m_stFeature.bGender) )
	{
		g_xMainWnd.DrawWithImageForComp(
						m_rcWnd.left+_EQUIP_CHAR_X+g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].m_lpstNewCurrWilImageInfo->shPX, 
						m_rcWnd.top+_EQUIP_CHAR_Y+g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].m_lpstNewCurrWilImageInfo->shPY, 
						g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].m_lpstNewCurrWilImageInfo->shWidth, 
						g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].m_lpstNewCurrWilImageInfo->shHeight,
						(WORD*)(g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].m_pbCurrImage));
	}

	// 物品绘制.
	for ( INT nCnt = 0; nCnt < _EQUIP_MAX_CELL; nCnt++ )
	{
		if ( m_stEquipItem[nCnt].bSetted )
		{
//			m_stEquipItem[nCnt].xItem.m_stItemInfo.stStdItem.wLooks = 1;
			if ( nCnt!=_U_HELMET && nCnt!=_U_WEAPON && nCnt!=_U_DRESS )
			{
				nStartX = m_rcWnd.left+m_rcCell[nCnt].left;
				nStartY = m_rcWnd.top+m_rcCell[nCnt].top;

				m_stEquipItem[nCnt].xItem.DrawItem(nStartX, nStartY);
			}
			else
			{
				nStartX = m_rcWnd.left+_EQUIP_CHAR_X;
				nStartY = m_rcWnd.top+_EQUIP_CHAR_Y;

				m_stEquipItem[nCnt].xItem.DrawItem(nStartX, nStartY, _ITEM_TYPE_EQUIP);
			}

		}
	}

	ShowEquipItemState();
}


VOID CStatusWnd::ShowEquipItemState()
{
	if (!m_showNativeHint || g_xGameProc.m_xInterface.m_stCommonItem.bSetted) return;
	INT nEquipNum = GetEquipNum(m_nativePointer);

	if ( nEquipNum != -1 )
	{
		if ( m_stEquipItem[nEquipNum].bSetted )
		{
			m_stEquipItem[nEquipNum].xItem.ShowItemStatus(m_rcWnd.left+37, m_rcWnd.top+288);
		}		
	}
}



INT CStatusWnd::GetEquipNum(POINT ptMouse)
{
	RECT rc;
	for ( INT nCnt = 0; nCnt < _EQUIP_MAX_CELL; nCnt++ )
	{
		SetRect(&rc, m_rcWnd.left+m_rcCell[nCnt].left, m_rcWnd.top+m_rcCell[nCnt].top, 
			    m_rcWnd.left+m_rcCell[nCnt].left+(m_rcCell[nCnt].right-m_rcCell[nCnt].left), m_rcWnd.top+m_rcCell[nCnt].top+(m_rcCell[nCnt].bottom-m_rcCell[nCnt].top));
		if ( PtInRect(&rc, ptMouse) )
		{
			return nCnt;
		}
	}

	return -1;
}



BOOL CStatusWnd::CheckEquipItem(CItem* item,BYTE slot,POINT point)
{
    if (GetEquipNum(point)!=slot) return FALSE;
    if (item->m_stItemInfo.stStdItem.bStdMode==25) return slot==5 || slot==9;
    if (item->m_bAttr==5 || item->m_bAttr==6) return slot==5 || slot==6;
    if (item->m_bAttr==7 || item->m_bAttr==8) return slot==7 || slot==8;
    return item->m_bAttr==slot;
}



VOID CStatusWnd::DeleteEquipItem(INT nEquipNum)
{
	ZeroMemory(&m_stEquipItem[nEquipNum], sizeof(ITEMSET));
}



VOID CStatusWnd::DeleteEquipItem(INT nMakeIndex, CHAR* szName)
{
	for ( INT nCnt = 0; nCnt < _EQUIP_MAX_CELL; nCnt++ )
	{
		if ( m_stEquipItem[nCnt].bSetted )
		{
			if ( m_stEquipItem[nCnt].xItem.m_stItemInfo.nMakeIndex == nMakeIndex )
			{
				if ( !strcmp(m_stEquipItem[nCnt].xItem.m_stItemInfo.stStdItem.szName, szName) )
				{
					DeleteEquipItem(nCnt);
					break;
				}
			}
		}		
	}
}



VOID CStatusWnd::DeleteAllEquipItem()
{
	for ( INT nCnt = 0; nCnt < _EQUIP_MAX_CELL; nCnt++ )
	{
		ZeroMemory(&m_stEquipItem[nCnt], sizeof(ITEMSET));
	}
}



BOOL CStatusWnd::AddEquipItem(CItem xItem, INT nEquipNum, BOOL bUseEquipNum)
{
	if ( bUseEquipNum )
	{
		if ( !m_stEquipItem[nEquipNum].bSetted )
		{
			m_stEquipItem[nEquipNum].bSetted = TRUE;
			m_stEquipItem[nEquipNum].xItem = xItem;
			m_stEquipItem[nEquipNum].xItem.m_shCellNum = nEquipNum;

			return TRUE;
		}
	}

	for ( INT nCnt = 0; nCnt < _EQUIP_MAX_CELL; nCnt++ )
	{
		if ( !m_stEquipItem[nCnt].bSetted )
		{
			m_stEquipItem[nCnt].bSetted = TRUE;
			m_stEquipItem[nCnt].xItem = xItem;
			m_stEquipItem[nCnt].xItem.m_shCellNum = nCnt;

			return TRUE;
		}
	}
	return FALSE;
}



BOOL CStatusWnd::ExtractEquipItem(CItem* pxItem, INT nEquipNum)
{
	if ( m_stEquipItem[nEquipNum].bSetted )
	{
		memcpy(pxItem, &m_stEquipItem[nEquipNum].xItem, sizeof(CItem));
		DeleteEquipItem(nEquipNum);
				
		return TRUE;
	}
	return FALSE;
}





VOID CStatusWnd::ChangeEquipWithCommon(INT nEquipNum, LPCOMMONITEMSET pstCommonItemSet)
{
	if ( pstCommonItemSet->bSetted )
	{
		CItem xEquipItem;
		if ( ExtractEquipItem(&xEquipItem, nEquipNum) )
		{
			AddEquipItem(pstCommonItemSet->xItem, nEquipNum, TRUE);

			pstCommonItemSet->bSetted			= TRUE;
			pstCommonItemSet->xItem				= xEquipItem;
			pstCommonItemSet->bWaitResponse		= FALSE;
			pstCommonItemSet->bIsEquipItem		= TRUE;
			pstCommonItemSet->bIsHideItem		= FALSE;
			pstCommonItemSet->bIsBeltItem		= FALSE;
			pstCommonItemSet->xItem.m_shCellNum = nEquipNum; 
		}
	}
}




VOID CStatusWnd::SetCommonFromEquip(INT nEquipNum, LPCOMMONITEMSET pstCommonItemSet)
{
	CItem xEquipItem;
	if ( ExtractEquipItem(&xEquipItem, nEquipNum) )
	{
		pstCommonItemSet->bSetted			= TRUE;
		pstCommonItemSet->xItem				= xEquipItem;
		pstCommonItemSet->bWaitResponse		= FALSE;
		pstCommonItemSet->bIsEquipItem		= TRUE;
		pstCommonItemSet->bIsHideItem		= FALSE;
		pstCommonItemSet->bIsBeltItem		= FALSE;
		pstCommonItemSet->xItem.m_shCellNum = nEquipNum; 
	}	
}



VOID CStatusWnd::SetEquipFromCommon(INT nEquipNum, LPCOMMONITEMSET pstCommonItemSet)
{
	if ( pstCommonItemSet->bSetted )
	{
		pstCommonItemSet->xItem.m_shCellNum = nEquipNum;
		AddEquipItem(pstCommonItemSet->xItem, nEquipNum);
		ZeroMemory(pstCommonItemSet, sizeof(COMMONITEMSET));
	}
}



VOID CStatusWnd::SetTakeOnFromEquip(INT nEquipNum)
{
	if ( m_stEquipItem[nEquipNum].bSetted )
	{
		m_stTakeOnItemSet.xItem	  = m_stEquipItem[nEquipNum].xItem;
		m_stTakeOnItemSet.bSetted = TRUE;
		m_stTakeOnItemSet.xItem.m_shCellNum = nEquipNum;
		ZeroMemory(&m_stEquipItem[nEquipNum], sizeof(ITEMSET));	
	}
}



VOID CStatusWnd::SetTakeOnFromCommon(INT nEquipNum, LPCOMMONITEMSET pstCommonItemSet)
{
	if ( pstCommonItemSet->bSetted )
	{
		m_stTakeOnItemSet.xItem   = pstCommonItemSet->xItem;
		m_stTakeOnItemSet.bSetted = TRUE;
		m_stTakeOnItemSet.xItem.m_shCellNum = nEquipNum;
		ZeroMemory(pstCommonItemSet, sizeof(COMMONITEMSET));	
	}
}



VOID CStatusWnd::SetEquipFromTakeOn()
{
	if ( m_stTakeOnItemSet.bSetted )
	{
		AddEquipItem(m_stTakeOnItemSet.xItem, m_stTakeOnItemSet.xItem.m_shCellNum, TRUE);
		ZeroMemory(&m_stTakeOnItemSet, sizeof(ITEMSET));
	}
}





















/******************************************************************************************************************

	角色状态信息窗口相关.

*******************************************************************************************************************/

VOID CStatusWnd::ShowCharStatus()
{
	CHAR	szBuff[MAX_PATH];

	POINT	ptStart = {m_rcWnd.left+255+15, m_rcWnd.top+67};

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "LEVEL", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d", g_xGameProc.m_xMyHero.m_stAbility.bLevel);
	g_xMainWnd.PutsHan(NULL, ptStart.x+53, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "HP", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d / %d", g_xGameProc.m_xMyHero.m_stAbility.wHP, g_xGameProc.m_xMyHero.m_stAbility.wMaxHP);
	g_xMainWnd.PutsHan(NULL, ptStart.x+53, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "MP", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d / %d", g_xGameProc.m_xMyHero.m_stAbility.wMP, g_xGameProc.m_xMyHero.m_stAbility.wMaxMP);
	g_xMainWnd.PutsHan(NULL, ptStart.x+53, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);
	
	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "AC", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d-%d", LOBYTE(g_xGameProc.m_xMyHero.m_stAbility.wAC), HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.wAC));
	g_xMainWnd.PutsHan(NULL, ptStart.x+53, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "MAC", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d-%d", LOBYTE(g_xGameProc.m_xMyHero.m_stAbility.wMAC), HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.wMAC));
	g_xMainWnd.PutsHan(NULL, ptStart.x+53, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "DC", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d-%d", LOBYTE(g_xGameProc.m_xMyHero.m_stAbility.wDC), HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.wDC));
	g_xMainWnd.PutsHan(NULL, ptStart.x+53, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "MC", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d-%d", LOBYTE(g_xGameProc.m_xMyHero.m_stAbility.wMC), HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.wMC));
	g_xMainWnd.PutsHan(NULL, ptStart.x+53, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "SC", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d-%d", LOBYTE(g_xGameProc.m_xMyHero.m_stAbility.wSC), HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.wSC));
	g_xMainWnd.PutsHan(NULL, ptStart.x+53, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.x = m_rcWnd.left+255+145;
	ptStart.y = m_rcWnd.top+67;

	FLOAT fExpRate = (FLOAT)((FLOAT)g_xGameProc.m_xMyHero.m_stAbility.dwExp/(FLOAT)g_xGameProc.m_xMyHero.m_stAbility.dwMaxExp);
	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "EXP", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%.2f%s", fExpRate*100, "%%");
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "Bag Weight", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d / %d", g_xGameProc.m_xMyHero.m_stAbility.wWeight, g_xGameProc.m_xMyHero.m_stAbility.wMaxWeight);
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "Equip Weight", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d / %d", g_xGameProc.m_xMyHero.m_stAbility.bWearWeight, g_xGameProc.m_xMyHero.m_stAbility.bMaxWearWeight);
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "Two-Hand Weight", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d / %d", g_xGameProc.m_xMyHero.m_stAbility.bHandWeight, g_xGameProc.m_xMyHero.m_stAbility.bMaxHandWeight);
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "Accuracy", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "+%d%", g_xGameProc.m_xMyHero.m_stSubAbility.bHitPoint);
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "Agility", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "+%d%", g_xGameProc.m_xMyHero.m_stSubAbility.bSpeedPoint);
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;
	
	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "Magic Evasion", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "+%d%", g_xGameProc.m_xMyHero.m_stSubAbility.wAntiMagic*10);
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;
	
	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "Poison Resist", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "+%d%", g_xGameProc.m_xMyHero.m_stSubAbility.bAntiPoison*10);
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;
	
	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "Poison Recovery", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "+%d%", g_xGameProc.m_xMyHero.m_stSubAbility.bPoisonRecover*10);
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;
	
	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "HP Recovery", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "+%d%", g_xGameProc.m_xMyHero.m_stSubAbility.bHealthRecover*10);
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;
	
	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "MP Recovery", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "+%d%", g_xGameProc.m_xMyHero.m_stSubAbility.bSpellRecover*10);
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);
}














/******************************************************************************************************************

	角色魔法窗口相关.

*******************************************************************************************************************/
// Native skill-tree renderer and pointer controls are below.




























/******************************************************************************************************************

	Message Function.

*******************************************************************************************************************/



BOOL CStatusWnd::OnLButtonUp(LPCOMMONITEMSET pstCommonItemSet, POINT ptMouse)
{
	INT nEquipNum;

    const bool closePressed=m_nativeClosePressed;
    const bool close=closePressed && NativeCloseHit(ptMouse); m_nativeClosePressed=false;
    if (close) { g_xSound.PlayLobbyEffect("105.wav"); return TRUE; }
    if (closePressed) return FALSE;

    if (g_xGameProc.m_xInterface.m_xExchangeWnd.GetGameWndActive() && g_xGameProc.m_xInterface.m_xExchangeWnd.returnSlot>=0) return FALSE;

		nEquipNum = GetEquipNum(ptMouse);
		// 鼠标位于装备窗口区域时.
		if ( nEquipNum != -1)
		{
			// 点击的装备窗口中有物品时.
			if ( m_stEquipItem[nEquipNum].bSetted )
			{
				// 如果公共(鼠标)物品集为空，
				if ( !pstCommonItemSet->bSetted )
				{				  
					// 把物品挂在鼠标上时, 在背包窗口点击的瞬间会发送 CM_TAKEOFFITEM 消息.
					SetCommonFromEquip(nEquipNum, pstCommonItemSet);
				}
				// 存在公共(鼠标)物品集，且不在等待服务器响应信号时，
				// 把物品预先设置到公共物品 m_stTakeOnItemSet 中, 再把当前点击的装备窗口物品设置为公共物品后,
				// 发送 CM_TAKEONITEM 消息.
				else if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse )
				{
					// 检查发送前的区域是否相互匹配(戒指是否放进了戒指位置).
					if ( CheckEquipItem(&pstCommonItemSet->xItem, nEquipNum, ptMouse) )
					{
						SetTakeOnFromCommon(nEquipNum, pstCommonItemSet);
						SetCommonFromEquip(nEquipNum, pstCommonItemSet);					
						g_xClientSocket.SendTakeOnOffItem(CM_TAKEONITEM, nEquipNum, m_stTakeOnItemSet.xItem.m_stItemInfo.stStdItem.szName, m_stTakeOnItemSet.xItem.m_stItemInfo.nMakeIndex);
						pstCommonItemSet->bWaitResponse	= TRUE;
					}
				}
			}
			// 点击的装备窗口中没有物品时.
			else
			{
				// 存在公共(鼠标)物品集, 且不是在等待服务器响应信号时, 就把公共物品设置到 m_stTakeOnItemSet 中.
				if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse )
				{	
					if ( CheckEquipItem(&pstCommonItemSet->xItem, nEquipNum, ptMouse) )
					{
						// 无需向服务器发送消息。卸下物品后又重新穿戴的情况。
						if ( pstCommonItemSet->bIsEquipItem )
						{
							SetEquipFromCommon(pstCommonItemSet->xItem.m_shCellNum, pstCommonItemSet);
							ZeroMemory(pstCommonItemSet, sizeof(COMMONITEMSET));
						}
						else
						{
							SetTakeOnFromCommon(nEquipNum, pstCommonItemSet);
							g_xClientSocket.SendTakeOnOffItem(CM_TAKEONITEM, nEquipNum, m_stTakeOnItemSet.xItem.m_stItemInfo.stStdItem.szName, m_stTakeOnItemSet.xItem.m_stItemInfo.nMakeIndex);
							pstCommonItemSet->bWaitResponse	= TRUE;
						}
					}
				}
			}
		}


	return FALSE;
}





BOOL CStatusWnd::OnLButtonDown(LPCOMMONITEMSET pstCommonItemSet, POINT ptMouse)
{
	INT nEquipNum;

    m_nativeClosePressed=NativeCloseHit(ptMouse);
    if (m_nativeClosePressed) return TRUE;
	nEquipNum = GetEquipNum(ptMouse);

	if ( nEquipNum == -1 )		m_bCanMove = TRUE;
	else						
	{
		if ( m_stEquipItem[nEquipNum].bSetted )		m_bCanMove = FALSE;
		else										m_bCanMove = TRUE;				
	}

	return FALSE;
}






bool CStatusWnd::NativeCloseHit(POINT point)
{
    if (!m_pxWndImage->NewSetIndex(1221)) return false;
    const auto& image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
    RECT area={m_rcWnd.left+289,m_rcWnd.top+14,m_rcWnd.left+289+image.shWidth,m_rcWnd.top+14+image.shHeight};
    return PtInRect(&area,point)!=FALSE;
}
void CStatusWnd::ShowNativeStatus()
{
    const int x=m_rcWnd.left,y=m_rcWnd.top;
    if (m_pxWndImage->NewSetIndex(1280)) {
        const auto& image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        g_xMainWnd.DrawWithImageForComp(x-(image.shWidth-328)/2,y-(image.shHeight-466)/2,
            image.shWidth,image.shHeight,(WORD*)m_pxWndImage->m_pbCurrImage);
    }
    const auto& held=g_xGameProc.m_xInterface.m_stCommonItem;
    if (m_showNativeHint && held.bSetted) {
        const int slot=GetEquipNum(m_nativePointer);
        const int mode=held.xItem.m_stItemInfo.stStdItem.bStdMode;
        const int frame=EquipmentPreviewFrame(mode,slot);
        if (frame>=0) DrawSelectionFrame(*m_pxWndImage,frame,x+m_rcCell[slot].left-2,
            y+m_rcCell[slot].top-(mode==53?21:2),SelectionBlend::Additive,false,20);
    }
    auto& hero=g_xGameProc.m_xMyHero;
    auto& body=g_xGameProc.m_xInterface.m_portraits;
    if (body.NewSetIndex(hero.m_stFeature.bGender)) {
        const auto& image=*body.m_lpstNewCurrWilImageInfo;
        g_xMainWnd.DrawWithImageForComp(x+82+image.shPX,y+199+image.shPY,image.shWidth,image.shHeight,(WORD*)body.m_pbCurrImage);
    }
    for (int slot=0;slot<_EQUIP_MAX_CELL;++slot) if (m_stEquipItem[slot].bSetted) {
        if (slot==0 || slot==1 || slot==4) m_stEquipItem[slot].xItem.DrawItem(x+82,y+199,_ITEM_TYPE_EQUIP);
        else m_stEquipItem[slot].xItem.DrawItem(x+m_rcCell[slot].left,y+m_rcCell[slot].top);
    }
    if (m_pxWndImage->NewSetIndex(m_nativeClosePressed?1222:1221)) {
        const auto& image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        g_xMainWnd.DrawWithImageForComp(x+289,y+14,image.shWidth,image.shHeight,(WORD*)m_pxWndImage->m_pbCurrImage);
    }
    for (int row=0;row<3;++row) for (int column=6;column>=0;--column) {
        if (ElementValue(row,column)<=0 || !m_pxWndImage->NewSetIndex((row==2?1520:1510)+column)) continue;
        const auto& image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        g_xMainWnd.DrawWithImageForComp(x+63+37*column,y+375+30*row,image.shWidth,image.shHeight,(WORD*)m_pxWndImage->m_pbCurrImage);
    }
    if (!m_identityBadge.empty() && m_pxWndImage->NewSetIndex(1298)) {
        const auto& image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        g_xMainWnd.DrawWithImageForComp(x+181,y+67,image.shWidth,image.shHeight,(WORD*)m_pxWndImage->m_pbCurrImage);
    }
    HDC dc; auto* surface=g_xMainWnd.GetBackBuffer(); if (FAILED(surface->GetDC(&dc))) return;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font=g_xMainWnd.CreateGameFont(face,9); HGDIOBJ previous=SelectObject(dc,font); SetBkMode(dc,TRANSPARENT);
    static CLoginMessages captions; static const bool loaded=captions.Load("./CmList.dat");
    const auto text=[&](const char* value,int left,int top,int width,int height,COLORREF color=0xfafafa,bool center=true) {
        RECT area={x+left,y+top,x+left+width,y+top+height}; SetTextColor(dc,color);
        DrawTextA(dc,value,-1,&area,DT_VCENTER|DT_SINGLELINE|(center?DT_CENTER:DT_LEFT));
    };
    const auto& a=hero.m_stAbility; char value[64];
    wsprintfA(value,"%u",a.bLevel); text(value,245,91,67,15);
    sprintf(value,"%.2f%%",a.dwMaxExp?100.0*a.dwExp/a.dwMaxExp:0.0); text(value,245,115,67,15,0x0000ff);
    const unsigned vitals[5][2]={{a.wHP,a.wMaxHP},{a.wMP,a.wMaxMP},{a.wWeight,a.wMaxWeight},
        {a.bWearWeight,a.bMaxWearWeight},{a.bHandWeight,a.bMaxHandWeight}};
    for (int row=0;row<5;++row) {
        for (int part=0;part<2;++part) { wsprintfA(value,"%u",vitals[row][part]); text(value,part?282:245,139+24*row,30,15); }
        text("/",276,140+24*row,6,15,0xfafafa,false);
    }
    const int labels[5][3]={{11,320,58},{13,349,58},{118,349,60},{118,320,60},{218,349,60}};
    const int values[5][3]={{58,320,58},{60,349,58},{163,349,60},{163,320,60},{270,349,60}};
    const int messages[5]={1061,5101,5102,1060,1406}; const WORD ranges[5]={a.wDC,a.wMC,a.wSC,a.wAC,a.wMAC};
    for (int i=0;i<5;++i) {
        text(captions.Get(messages[i]).c_str(),labels[i][0],labels[i][1],labels[i][2],15);
        wsprintfA(value,"%u-%u",LOBYTE(ranges[i]),HIBYTE(ranges[i])); text(value,values[i][0],values[i][1],values[i][2],15);
    }
    wsprintfA(value,"+%u%%",hero.m_stSubAbility.bHitPoint); text(value,245,259,67,15);
    wsprintfA(value,"+%u%%",hero.m_stSubAbility.bSpeedPoint); text(value,245,283,67,15);
    text(captions.Get(5178).c_str(),11,377,50,15); text(captions.Get(5179).c_str(),11,408,50,15);
    text(captions.Get(5180).c_str(),12,438,50,15);
    for (int row=0;row<3;++row) for (int column=6;column>=0;--column) {
        const int element=ElementValue(row,column); if (element<=0) continue;
        wsprintfA(value,row==0?"+%d":"x%d",element);
        text(value,81+37*column,row==0?379:row==1?408:439,20,15);
    }
    const char nameFace[]={char(0xb1),char(0xbc),char(0xb8),char(0xb2),0};
    HFONT nameFont=g_xMainWnd.CreateGameFont(nameFace,9,0,FW_BOLD); SelectObject(dc,nameFont);
    const bool secondary=hero.m_szGuildName[0]!=0;
    text(hero.m_szName,100,secondary?14:30,130,20,0xdcffdc);
    SelectObject(dc,font);
    if (secondary) {
        text(hero.m_szGuildName,82,29,172,20,0xb4faff);
        text(m_identityTertiary.c_str(),82,43,172,20,0x96c8ff);
    }
    if (!m_identityBadge.empty()) text(m_identityBadge.c_str(),200,66,105,15,0xeb3ef4);
    SelectObject(dc,previous); DeleteObject(nameFont); DeleteObject(font); surface->ReleaseDC(dc);
    ShowNativeHint();
    ShowEquipItemState();
}

void CStatusWnd::ShowNativeHint()
{
    if (!m_showNativeHint) return;
    static CLoginMessages captions; static const bool loaded=captions.Load("./CmList.dat");
    const int x=m_rcWnd.left,y=m_rcWnd.top;
    std::string caption; RECT bounds={}; bool measured=false;
    for (int row=0;row<3;++row) for (int column=0;column<7;++column) {
        const RECT hit={x+62+37*column,y+382+30*row,x+98+37*column,y+401+30*row};
        if (ElementValue(row,column)>0 && PtInRect(&hit,m_nativePointer)) {
            caption=captions.Get(5161+column);
            SetRect(&bounds,x+72+37*column,y+372+30*row,x+133+37*column,y+391+30*row);
        }
    }
    const int messages[9]={2400,2401,-1,-1,2404,2402,2403,1053,1054};
    for (int row=0;row<9;++row) {
        const RECT hit={x+210,y+84+24*row,x+288,y+102+24*row};
        if (PtInRect(&hit,m_nativePointer)) {
            caption=row==2?"HP":row==3?"MP":captions.Get(messages[row]);
            SetRect(&bounds,x+200,y+69+24*row,x+200,y+87+24*row); measured=true;
        }
    }
    if (caption.empty()) return;
    auto* surface=g_xMainWnd.GetBackBuffer(); HDC dc;
    if (FAILED(surface->GetDC(&dc))) return;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font=g_xMainWnd.CreateGameFont(face,9); HGDIOBJ previous=SelectObject(dc,font);
    if (measured) { SIZE size; GetTextExtentPoint32A(dc,caption.c_str(),int(caption.size()),&size); bounds.right+=size.cx; }
    SelectObject(dc,previous); surface->ReleaseDC(dc);
    DrawLegacyPanel(bounds,RGB(80,60,40),150,RGB(100,75,50));
    if (SUCCEEDED(surface->GetDC(&dc))) {
        previous=SelectObject(dc,font); SetBkMode(dc,TRANSPARENT); SetTextColor(dc,RGB(255,255,255));
        DrawTextA(dc,caption.c_str(),-1,&bounds,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        SelectObject(dc,previous); surface->ReleaseDC(dc);
    }
    DeleteObject(font);
}

int CStatusWnd::EquipmentPreviewFrame(int mode,int slot)
{
    switch (mode) {
    case 19: case 20: case 21: case 57: return slot==3?1235:-1;
    case 22: case 23: return slot==7 || slot==8?1235:-1;
    case 24: case 26: return slot==5 || slot==6?1235:-1;
    case 25: return slot==9?1235:-1;
    case 30: return slot==2?1235:-1;
    case 53: return slot==10?1236:-1;
    case 60: return slot==11?1235:-1;
    default: return -1;
    }
}

bool CStatusWnd::ReceiveEquipmentRecords(const char* encoded)
{
    ITEMSET next[_EQUIP_MAX_CELL];std::copy(std::begin(m_stEquipItem),std::end(m_stEquipItem),std::begin(next));
    int count=0;
    while(*encoded) {
        if(++count>_EQUIP_MAX_CELL) return false;
        const char* slash=strchr(encoded,'/');if(!slash) return false;
        int slot=-1;const auto parsed=std::from_chars(encoded,slash,slot);
        if(parsed.ec!=std::errc{} || parsed.ptr!=slash || slot<0 || slot>=_EQUIP_MAX_CELL) return false;
        encoded=slash+1;const char* end=strchr(encoded,'/');if(!end) return false;
        const std::string block(encoded,end);CItem item;
        if(block.size()==134) {
            BYTE raw[100];
            if(fnDecode6BitBuf(const_cast<char*>(block.c_str()),(char*)raw,sizeof(raw))!=100 || !item.SetMir3Item(raw,slot,true)) return false;
        } else if(block.size()==(sizeof(CLIENTITEMRCD)*8+5)/6) {
            CLIENTITEMRCD legacy={};
            if(fnDecode6BitBuf(const_cast<char*>(block.c_str()),(char*)&legacy,sizeof(legacy))!=sizeof(legacy)) return false;
            item.SetItem(legacy,slot);
        } else return false;
        if(!g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewSetIndex(item.m_stItemInfo.stStdItem.wLooks)) return false;
        next[slot].bSetted=TRUE;next[slot].xItem=item;encoded=end+1;
    }
    std::copy(std::begin(next),std::end(next),std::begin(m_stEquipItem));return true;
}

void CStatusWnd::UpdateDurability(WORD slot,LONG current,WORD maximum)
{
    if(slot>=_EQUIP_MAX_CELL || !m_stEquipItem[slot].bSetted) return;
    auto& item=m_stEquipItem[slot].xItem;
    item.m_stItemInfo.nDura=WORD(current);item.m_stItemInfo.nDuraMax=maximum;
    item.m_serverRecord[66]=BYTE(current);item.m_serverRecord[67]=BYTE(WORD(current)>>8);
    item.m_serverRecord[68]=BYTE(maximum);item.m_serverRecord[69]=BYTE(maximum>>8);
    const int mode=item.m_stItemInfo.stStdItem.bStdMode,unit=mode==54 || mode==55?100:1000;
    if(int(double(current)/unit+0.5)<=0) {
        static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
        std::string message=captions.Get(1470);const auto at=message.find("%s");
        if(at!=std::string::npos) message.replace(at,2,item.Name());
        g_xGameProc.m_xInterface.MsgAdd(0xffffff,33023,const_cast<char*>(message.c_str()));
    }
}
