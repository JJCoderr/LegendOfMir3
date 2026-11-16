/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/


#include "StdAfx.h"
#include "Common/LegacyPanel.h"
#include "Common/LoginMessages.h"


CItem::CItem()
{
	m_shCellNum	= 0;
	m_bAttr		= 0;
	ZeroMemory(&m_stItemInfo, sizeof(CLIENTITEMRCD));
}


CItem::~CItem()
{
}


bool CItem::SetMir3Item(const BYTE* record,SHORT cell,bool allowEmptyName)
{
    if(!allowEmptyName && !record[0]) return false;
    const auto word=[&](int offset) {return WORD(record[offset]|(WORD(record[offset+1])<<8));};
    CLIENTITEMRCD legacy={};
    memcpy(legacy.stStdItem.szName,record,sizeof(legacy.stStdItem.szName)-1);
    legacy.stStdItem.bStdMode=record[30];legacy.stStdItem.bShape=record[31];
    legacy.stStdItem.wLooks=word(36);
    legacy.nMakeIndex=DWORD(word(62))|(DWORD(word(64))<<16);
    legacy.nDura=word(66);legacy.nDuraMax=word(68);
    SetItem(legacy,cell);
    memcpy(m_serverRecord,record,sizeof(m_serverRecord));memcpy(m_serverName,record,30);m_serverName[30]=0;
    m_stackMode=record[61];m_count=word(71);return true;
}

BOOL CItem::SetItem(CLIENTITEMRCD stItemInfo, SHORT shInventoryNum)
{
	m_stItemInfo = stItemInfo;
	m_shCellNum = shInventoryNum;
    m_bAttr=_U_UNEQUIP; m_count=1; m_stackMode=0;
    ZeroMemory(m_serverRecord,sizeof(m_serverRecord));ZeroMemory(m_serverName,sizeof(m_serverName));
/*
a:武器
b:衣服
c:戒指 手镯 项链
g:其他
*/
/*
#define _U_DRESS					0
#define _U_WEAPON					1
#define _U_RIGHTHAND				2
#define _U_NECKLACE					3
#define _U_HELMET					4
#define _U_ARMRINGL					5
#define _U_ARMRINGR					6            
#define _U_RINGL					7 
#define _U_RINGR					8 
#define _U_CHARM					9
#define _U_UNEQUIP					100
*/

	switch ( m_stItemInfo.stStdItem.bStdMode )
	{
	// 武器。
	case 5:		
	case 6:
    case 54: case 55: case 56:
		m_bAttr = _U_WEAPON;
		break;
	// 衣服.
	case 10:
	case 11:
		m_bAttr = _U_DRESS;
		break;
	case 15:
		m_bAttr = _U_HELMET;
		break;
	case 19:
	case 20:
	case 21:
    case 57:
		m_bAttr = _U_NECKLACE;
		break;
	case 22:
	case 23:
		m_bAttr = _U_RINGL;
		break;
	case 24:
	case 26:
		m_bAttr = _U_ARMRINGL;
		break;
	// 蜡烛, 火把。
	case 30:
		m_bAttr = _U_RIGHTHAND;
		break;
	// 毒粉、符咒。
	case 25:
		m_bAttr = _U_CHARM;
		break;
	}

    if (m_stItemInfo.stStdItem.bStdMode==53) m_bAttr=10;
    if (m_stItemInfo.stStdItem.bStdMode==60) m_bAttr=11;
	return TRUE;
}


BOOL CItem::DrawItem(INT nX, INT nY, BYTE bItemType, const RECT* clip)
{
	CWHWilImageData*	pxItemImg;	
	BOOL bDrawItem = FALSE;
	switch ( bItemType )
	{
	case _ITEM_TYPE_INVENTORY:
		pxItemImg = &g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY];
		bDrawItem = pxItemImg->NewSetIndex(m_stItemInfo.stStdItem.wLooks);
		if ( bDrawItem )
		{
			nX = nX+_INVENTORY_CELL_WIDTH/2-pxItemImg->m_lpstNewCurrWilImageInfo->shWidth/2;
			nY = nY+_INVENTORY_CELL_HEIGHT/2-pxItemImg->m_lpstNewCurrWilImageInfo->shHeight/2;
		}
		break;
	case _ITEM_TYPE_EQUIP:
		pxItemImg = &g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_EQUIP];
		bDrawItem = pxItemImg->NewSetIndex(m_stItemInfo.stStdItem.wLooks);
		if ( bDrawItem )
		{
			nX += g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_EQUIP].m_lpstNewCurrWilImageInfo->shPX;
			nY += g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_EQUIP].m_lpstNewCurrWilImageInfo->shPY;
		}
		break;
	}
		
    if (bDrawItem && clip) {
        const auto& image=*pxItemImg->m_lpstNewCurrWilImageInfo;
        RECT bounds={nX,nY,nX+image.shWidth,nY+image.shHeight},visible;
        if (!IntersectRect(&visible,&bounds,clip)) return TRUE;
        DrawSelectionFrame(*pxItemImg,m_stItemInfo.stStdItem.wLooks,nX,nY,SelectionBlend::Normal,false,0,&visible);
        return TRUE;
    }
	if ( bDrawItem )
	{
 		g_xMainWnd.DrawWithImageForCompClipRgn(
						nX,
						nY,
						pxItemImg->m_lpstNewCurrWilImageInfo->shWidth,
						pxItemImg->m_lpstNewCurrWilImageInfo->shHeight,
						(WORD*)pxItemImg->m_pbCurrImage, _CLIP_WIDTH, 600);
	}
	return TRUE;
}


VOID CItem::ShowItemStatus(INT nX, INT nY)
{
    if(m_serverName[0]) {
        switch(m_stItemInfo.stStdItem.bStdMode) {
        case 15:case 19:case 20:case 21:case 22:case 23:case 24:case 26:case 57:case 53:break;
        default:ShowGeneralHint(nX,nY);return;
        }
    }
	CHAR	szLineBuf[3][MAX_PATH];
	CHAR	szName[MAX_PATH];
	SIZE	sizeLen;
	BOOL	bUseAble = TRUE;
	const INT nLineGap = 14;

	ZeroMemory(szName, MAX_PATH);
	ZeroMemory(szLineBuf, 3*MAX_PATH);

	if ( m_stItemInfo.stStdItem.szName )
	{
		switch ( m_stItemInfo.stStdItem.bStdMode )
		{
		// 男衣, 女衣.
		case 10:
		case 11:
			{
				bUseAble = FALSE;

				// 第一行。
				if ( m_stItemInfo.stStdItem.szName[20] )
					g_xMainWnd.StringPlus(szName, &m_stItemInfo.stStdItem.szName[20], " ", &m_stItemInfo.stStdItem.szName[0], "");
				else
					strcpy(szName, m_stItemInfo.stStdItem.szName);
				g_xMainWnd.PutsHan(NULL, nX, nY, RGB(250, 250, 0), RGB(0, 0, 0), szName);
				sizeLen = g_xMainWnd.GetStrLength(NULL, NULL, szName);

				sprintf(szLineBuf[0], " Weight %d Durability %d/%d", (m_stItemInfo.stStdItem.bWeight), (m_stItemInfo.nDura/1000), (m_stItemInfo.nDuraMax)/1000);

				g_xMainWnd.PutsHan(NULL, nX+sizeLen.cx, nY, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[0]);
				

				// 第二行.
				if ( m_stItemInfo.stStdItem.wAC > 0 )
					sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Defense %d-%d ", LOBYTE(m_stItemInfo.stStdItem.wAC), HIBYTE(m_stItemInfo.stStdItem.wAC));

				if ( m_stItemInfo.stStdItem.wMAC > 0 )
					sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Magic Resist %d-%d ", LOBYTE(m_stItemInfo.stStdItem.wMAC), HIBYTE(m_stItemInfo.stStdItem.wMAC));

				if ( m_stItemInfo.stStdItem.wDC > 0 )
					sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Destruction %d-%d ", LOBYTE(m_stItemInfo.stStdItem.wDC), HIBYTE(m_stItemInfo.stStdItem.wDC));

				if ( m_stItemInfo.stStdItem.wMC > 0 )
					sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Magic %d-%d ", LOBYTE(m_stItemInfo.stStdItem.wMC), HIBYTE(m_stItemInfo.stStdItem.wMC));

				if ( m_stItemInfo.stStdItem.wSC > 0 )
					sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Tao Power %d-%d ", LOBYTE(m_stItemInfo.stStdItem.wSC), HIBYTE(m_stItemInfo.stStdItem.wSC));

				g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[1]);

				// 第三行.
				switch ( m_stItemInfo.stStdItem.bNeed )
				{
				case 0:
					{
						if ( g_xGameProc.m_xMyHero.m_stAbility.bLevel >= m_stItemInfo.stStdItem.bNeedLevel )
						   bUseAble = TRUE;
						sprintf(&szLineBuf[2][strlen(szLineBuf[2])], "Req. Level %d ", m_stItemInfo.stStdItem.bNeedLevel);
					}
					break;
				case 1:
					{
						if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.wDC) >= m_stItemInfo.stStdItem.bNeedLevel )
						   bUseAble = TRUE;
						sprintf(&szLineBuf[2][strlen(szLineBuf[2])], "Req. Destruction %d ", m_stItemInfo.stStdItem.bNeedLevel);
					}
					break;
				case 2:
					{
						if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.wMC) >= m_stItemInfo.stStdItem.bNeedLevel )
						   bUseAble = TRUE;
						sprintf(&szLineBuf[2][strlen(szLineBuf[2])], "Req. Magic %d ", m_stItemInfo.stStdItem.bNeedLevel);
					}
					break;
				case 3:
					{
						if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.wSC) >= m_stItemInfo.stStdItem.bNeedLevel )
						   bUseAble = TRUE;
						sprintf(&szLineBuf[2][strlen(szLineBuf[2])], "Req. Tao Power %d ", m_stItemInfo.stStdItem.bNeedLevel);
					}
					break;
				}

				if ( bUseAble )
					g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap*2, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[2]);
				else
					g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap*2, RGB(250, 0, 0), RGB(0, 0, 0), szLineBuf[2]);
			}
			break;
		// 武器。
		case 5:
		case 6:
			{
				bUseAble = FALSE;

				// 第一行。
				if ( (m_stItemInfo.stStdItem.bNeedIdentify & 0X01) != 0 )			
				{
					if ( m_stItemInfo.stStdItem.szName[20] )
						g_xMainWnd.StringPlus(szName, "(*)", &m_stItemInfo.stStdItem.szName[20], " ", &m_stItemInfo.stStdItem.szName[0], "");
					else
						strcpy(szName, m_stItemInfo.stStdItem.szName);
				}
				else																
				{
					if ( m_stItemInfo.stStdItem.szName[20] )
						g_xMainWnd.StringPlus(szName, &m_stItemInfo.stStdItem.szName[20], " ", &m_stItemInfo.stStdItem.szName[0], "");
					else
						strcpy(szName, m_stItemInfo.stStdItem.szName);
				}

				g_xMainWnd.PutsHan(NULL, nX, nY, RGB(250, 250, 0), RGB(0, 0, 0), szName);
				sizeLen = g_xMainWnd.GetStrLength(NULL, NULL, szName);

				sprintf(szLineBuf[0], " Weight %d Durability %d/%d", (m_stItemInfo.stStdItem.bWeight), (m_stItemInfo.nDura/1000), (m_stItemInfo.nDuraMax)/1000);
				g_xMainWnd.PutsHan(NULL, nX+sizeLen.cx, nY, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[0]);

				// 第二行.
				if ( m_stItemInfo.stStdItem.wDC > 0 )
					sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Destruction %d-%d ", LOBYTE(m_stItemInfo.stStdItem.wDC), HIBYTE(m_stItemInfo.stStdItem.wDC));

				if ( m_stItemInfo.stStdItem.wMC > 0 )
					sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Magic %d-%d ", LOBYTE(m_stItemInfo.stStdItem.wMC), HIBYTE(m_stItemInfo.stStdItem.wMC));

				if ( m_stItemInfo.stStdItem.wSC > 0 )
					sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Tao Power %d-%d ", LOBYTE(m_stItemInfo.stStdItem.wSC), HIBYTE(m_stItemInfo.stStdItem.wSC));

				if ( m_stItemInfo.stStdItem.sSpecialPower > 0 )
					sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Power %d ", m_stItemInfo.stStdItem.sSpecialPower);

				g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[1]);

				// 第三行.
				if ( HIBYTE(m_stItemInfo.stStdItem.wAC) > 0 )
					sprintf(&szLineBuf[2][strlen(szLineBuf[2])], "Accuracy +%d ", HIBYTE(m_stItemInfo.stStdItem.wAC));

				if ( HIBYTE(m_stItemInfo.stStdItem.wMAC) > 0 )
				{
					if ( HIBYTE(m_stItemInfo.stStdItem.wMAC) > 10 )
						sprintf(&szLineBuf[2][strlen(szLineBuf[2])], "Attack Speed +%d ", HIBYTE(m_stItemInfo.stStdItem.wMAC)-10);
					else
						sprintf(&szLineBuf[2][strlen(szLineBuf[2])], "Attack Speed -%d ", HIBYTE(m_stItemInfo.stStdItem.wMAC));
				}

				if ( LOBYTE(m_stItemInfo.stStdItem.wAC) > 0 )
					sprintf(&szLineBuf[2][strlen(szLineBuf[2])], "Luck +%d ", LOBYTE(m_stItemInfo.stStdItem.wAC));

				if ( LOBYTE(m_stItemInfo.stStdItem.wMAC) > 0 )
					sprintf(&szLineBuf[2][strlen(szLineBuf[2])], "Curse +%d ", LOBYTE(m_stItemInfo.stStdItem.wMAC));

				switch ( m_stItemInfo.stStdItem.bNeed )
				{
				case 0:
					{
						if ( g_xGameProc.m_xMyHero.m_stAbility.bLevel >= m_stItemInfo.stStdItem.bNeedLevel )
						   bUseAble = TRUE;
						sprintf(&szLineBuf[2][strlen(szLineBuf[2])], "Req. Level %d ", m_stItemInfo.stStdItem.bNeedLevel);
					}
					break;
				case 1:
					{
						if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.wDC) >= m_stItemInfo.stStdItem.bNeedLevel )
						   bUseAble = TRUE;
						sprintf(&szLineBuf[2][strlen(szLineBuf[2])], "Req. Destruction %d ", m_stItemInfo.stStdItem.bNeedLevel);
					}
					break;
				case 2:
					{
						if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.wMC) >= m_stItemInfo.stStdItem.bNeedLevel )
						   bUseAble = TRUE;
						sprintf(&szLineBuf[2][strlen(szLineBuf[2])], "Req. Magic %d ", m_stItemInfo.stStdItem.bNeedLevel);
					}
					break;
				case 3:
					{
						if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.wSC) >= m_stItemInfo.stStdItem.bNeedLevel )
						   bUseAble = TRUE;
						sprintf(&szLineBuf[2][strlen(szLineBuf[2])], "Req. Tao Power %d ", m_stItemInfo.stStdItem.bNeedLevel);
					}
					break;
				}

				if ( bUseAble )
					g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap*2, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[2]);
				else
					g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap*2, RGB(250, 0, 0), RGB(0, 0, 0), szLineBuf[2]);

			}
			break;			
        case 15:	//帽子,头盔
        case 19:	//项链
		case 20:
		case 21:  
        case 22:	//戒指
		case 23:  
        case 24:	//手镯
		case 26:  
			{
				bUseAble = FALSE;

				// 第一行。
				if ( m_stItemInfo.stStdItem.szName[20] )
					g_xMainWnd.StringPlus(szName, &m_stItemInfo.stStdItem.szName[20], " ", &m_stItemInfo.stStdItem.szName[0], "");
				else
					strcpy(szName, m_stItemInfo.stStdItem.szName);
				g_xMainWnd.PutsHan(NULL, nX, nY, RGB(250, 250, 0), RGB(0, 0, 0), szName);
				sizeLen = g_xMainWnd.GetStrLength(NULL, NULL, szName);

				sprintf(szLineBuf[0], " Weight %d Durability %d/%d", (m_stItemInfo.stStdItem.bWeight), (m_stItemInfo.nDura/1000), (m_stItemInfo.nDuraMax)/1000);

				g_xMainWnd.PutsHan(NULL, nX+sizeLen.cx, nY, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[0]);

				// 第二行.
				// 项链.
				if ( m_stItemInfo.stStdItem.bStdMode == 19 && m_stItemInfo.stStdItem.bStdMode == 20 )
				{
					if ( m_stItemInfo.stStdItem.wAC > 0 )
						sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Magic Evasion +%d ", HIBYTE(m_stItemInfo.stStdItem.wAC));

					if ( LOBYTE(m_stItemInfo.stStdItem.wMAC) > 0 )
						sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Curse +%d ", LOBYTE(m_stItemInfo.stStdItem.wMAC));

					if ( HIBYTE(m_stItemInfo.stStdItem.wMAC) > 0 )
						sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Luck +%d ", HIBYTE(m_stItemInfo.stStdItem.wMAC));		
				}				
				// 戒指.
				if ( m_stItemInfo.stStdItem.bStdMode == 22 && m_stItemInfo.stStdItem.bStdMode == 23 )
				{
					if ( HIBYTE(m_stItemInfo.stStdItem.wMAC) > 0 )
						sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Poison Recovery +%d ", HIBYTE(m_stItemInfo.stStdItem.wMAC));

					if ( LOBYTE(m_stItemInfo.stStdItem.wAC) > 0 )
						sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Attack Speed +%d ", LOBYTE(m_stItemInfo.stStdItem.wAC));
					
					if ( LOBYTE(m_stItemInfo.stStdItem.wMAC) > 0 )
						sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Attack Speed -%d ", LOBYTE(m_stItemInfo.stStdItem.wMAC));
				}
				// 手镯.
				else if ( m_stItemInfo.stStdItem.bStdMode == 24 && m_stItemInfo.stStdItem.bStdMode == 26 )
				{
					if ( m_stItemInfo.stStdItem.wAC > 0 )
						sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Accuracy +%d ", HIBYTE(m_stItemInfo.stStdItem.wAC));
			
					if ( m_stItemInfo.stStdItem.wMAC > 0 )
						sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Agility +%d ", HIBYTE(m_stItemInfo.stStdItem.wMAC));
				}
				else
				{

					if ( m_stItemInfo.stStdItem.wAC > 0 )
						sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Defense %d-%d ", LOBYTE(m_stItemInfo.stStdItem.wAC), HIBYTE(m_stItemInfo.stStdItem.wAC));
					
					if ( m_stItemInfo.stStdItem.wMAC > 0 )
						sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Magic Resist %d-%d ", LOBYTE(m_stItemInfo.stStdItem.wMAC), HIBYTE(m_stItemInfo.stStdItem.wMAC));
				}

				if ( m_stItemInfo.stStdItem.wDC > 0 )
					sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Destruction %d-%d ", LOBYTE(m_stItemInfo.stStdItem.wDC), HIBYTE(m_stItemInfo.stStdItem.wDC));

				if ( m_stItemInfo.stStdItem.wMC > 0 )
					sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Magic %d-%d ", LOBYTE(m_stItemInfo.stStdItem.wMC), HIBYTE(m_stItemInfo.stStdItem.wMC));

				if ( m_stItemInfo.stStdItem.wSC > 0 )
					sprintf(&szLineBuf[1][strlen(szLineBuf[1])], "Tao Power %d-%d ", LOBYTE(m_stItemInfo.stStdItem.wSC), HIBYTE(m_stItemInfo.stStdItem.wSC));

				g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[1]);

				// 第三行。
				switch ( m_stItemInfo.stStdItem.bNeed )
				{
				case 0:
					{
						if ( g_xGameProc.m_xMyHero.m_stAbility.bLevel >= m_stItemInfo.stStdItem.bNeedLevel )
						   bUseAble = TRUE;
						sprintf(&szLineBuf[2][strlen(szLineBuf[2])], "Req. Level %d ", m_stItemInfo.stStdItem.bNeedLevel);
					}
					break;
				case 1:
					{
						if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.wDC) >= m_stItemInfo.stStdItem.bNeedLevel )
						   bUseAble = TRUE;
						sprintf(&szLineBuf[2][strlen(szLineBuf[2])], "Req. Destruction %d ", m_stItemInfo.stStdItem.bNeedLevel);
					}
					break;
				case 2:
					{
						if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.wMC) >= m_stItemInfo.stStdItem.bNeedLevel )
						   bUseAble = TRUE;
						sprintf(&szLineBuf[2][strlen(szLineBuf[2])], "Req. Magic %d ", m_stItemInfo.stStdItem.bNeedLevel);
					}
					break;
				case 3:
					{
						if ( HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.wSC) >= m_stItemInfo.stStdItem.bNeedLevel )
						   bUseAble = TRUE;
						sprintf(&szLineBuf[2][strlen(szLineBuf[2])], "Req. Tao Power %d ", m_stItemInfo.stStdItem.bNeedLevel);
					}
					break;
				}

				if ( bUseAble )
					g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap*2, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[2]);
				else
					g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap*2, RGB(250, 0, 0), RGB(0, 0, 0), szLineBuf[2]);
			}
			break;

/*		case 25:	//撒播的毒粉
		case 30:	//蜡烛,火把
		case 40:	//肉块
		case 42:
		case 43:
*/		default:
			{
				bUseAble = FALSE;

				// 第一行.
				g_xMainWnd.PutsHan(NULL, nX, nY, RGB(250, 250, 0), RGB(0, 0, 0), m_stItemInfo.stStdItem.szName);
				sizeLen = g_xMainWnd.GetStrLength(NULL, NULL, m_stItemInfo.stStdItem.szName);

				switch ( m_stItemInfo.stStdItem.bStdMode )
				{
				case 0:			// 药水。
					if ( m_stItemInfo.stStdItem.wAC > 0 )
						sprintf(&szLineBuf[0][strlen(szLineBuf[0])], " +%dHP ", m_stItemInfo.stStdItem.wAC);

					if ( m_stItemInfo.stStdItem.wMAC > 0 )
						sprintf(&szLineBuf[0][strlen(szLineBuf[0])], " +%dMP ", m_stItemInfo.stStdItem.wMAC);

					sprintf(&szLineBuf[0][strlen(szLineBuf[0])], " Weight %d ", m_stItemInfo.stStdItem.bWeight);

					g_xMainWnd.PutsHan(NULL, nX+sizeLen.cx, nY, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[0]);

					break;
				case 1:			// 干肉.
				case 2:			// 饭团。
					break;
				case 3:			// 特殊药水 OR 各种全书。
					sprintf(&szLineBuf[0][strlen(szLineBuf[0])], " Weight %d ", m_stItemInfo.stStdItem.bWeight);
					g_xMainWnd.PutsHan(NULL, nX+sizeLen.cx, nY, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[0]);
					break;
				case 4:			// 各种书籍.
					sprintf(&szLineBuf[0][strlen(szLineBuf[0])], " Weight %d ", m_stItemInfo.stStdItem.bWeight);
					g_xMainWnd.PutsHan(NULL, nX+sizeLen.cx, nY, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[0]);

					switch ( m_stItemInfo.stStdItem.bShape )
					{
					case 0:
						{
							// 第二行.
							sprintf(&szLineBuf[1][strlen(szLineBuf[1])], " Warrior Manual ");
							g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[1]);

							sprintf(&szLineBuf[2][strlen(szLineBuf[2])], " Req. Level %d ", m_stItemInfo.stStdItem.wDuraMax);

							if ( (g_xGameProc.m_xMyHero.m_bJob == _JOB_JUNSA) && (g_xGameProc.m_xMyHero.m_stAbility.bLevel >= m_stItemInfo.stStdItem.wDuraMax) )
							   bUseAble = TRUE;
							if ( bUseAble )
								g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap*2, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[2]);
							else
								g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap*2, RGB(250, 0, 0), RGB(0, 0, 0), szLineBuf[2]);
						}
						break;
					case 1:
						{
							// 第二行.
							sprintf(&szLineBuf[1][strlen(szLineBuf[1])], " Taoist Spell Book ");
							g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[1]);

							sprintf(&szLineBuf[2][strlen(szLineBuf[2])], " Req. Level %d ", m_stItemInfo.stStdItem.wDuraMax);

							if ( (g_xGameProc.m_xMyHero.m_bJob == _JOB_SULSA) && (g_xGameProc.m_xMyHero.m_stAbility.bLevel >= m_stItemInfo.stStdItem.wDuraMax) )
							   bUseAble = TRUE;

							if ( bUseAble )
								g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap*2, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[2]);
							else
								g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap*2, RGB(250, 0, 0), RGB(0, 0, 0), szLineBuf[2]);
						}
						break;
					case 2:
						{
							sprintf(&szLineBuf[1][strlen(szLineBuf[1])], " Taoist Manual ");
							g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[1]);

							sprintf(&szLineBuf[2][strlen(szLineBuf[2])], " Req. Level %d ", m_stItemInfo.stStdItem.wDuraMax);

							if ( (g_xGameProc.m_xMyHero.m_bJob == _JOB_DOSA) && (g_xGameProc.m_xMyHero.m_stAbility.bLevel >= m_stItemInfo.stStdItem.wDuraMax) )
							   bUseAble = TRUE;

							if ( bUseAble )
								g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap*2, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[2]);
							else
								g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap*2, RGB(250, 0, 0), RGB(0, 0, 0), szLineBuf[2]);
						}
						break;
					}
					break;						
				case 25:			// 各种毒粉。
					// 第一行.
					sprintf(&szLineBuf[1][strlen(szLineBuf[1])], " Weight %d ", m_stItemInfo.stStdItem.bWeight);
					g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[1]);

					// 第二行.
					sprintf(&szLineBuf[2][strlen(szLineBuf[2])], " Uses %d/%d ", m_stItemInfo.nDura, m_stItemInfo.nDuraMax);
					g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap*2, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[2]);
					break;
				case 30:			// 火把和蜡烛.
					// 第二行.
					sprintf(&szLineBuf[1][strlen(szLineBuf[1])], " Weight %d Durability %d/%d ", m_stItemInfo.stStdItem.bWeight, m_stItemInfo.nDura/1000, m_stItemInfo.nDuraMax/1000);
					g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[1]);
					break;
				case 31:			// 各种打包系列。
					// 第一行.
					sprintf(&szLineBuf[1][strlen(szLineBuf[1])], " Weight %d ", m_stItemInfo.stStdItem.bWeight);
					g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[1]);
					break;
				case 40:		// 各种肉类(肉块)
					// 第二行.
					sprintf(&szLineBuf[1][strlen(szLineBuf[1])], " Weight %d Quality %d/%d ", m_stItemInfo.stStdItem.bWeight, m_stItemInfo.nDura/1000, m_stItemInfo.nDuraMax/1000);
					g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[1]);
					break;
				case 42:			// 各种药材.
					// 第二行.
					sprintf(&szLineBuf[1][strlen(szLineBuf[1])], " Weight %d Herb ", m_stItemInfo.stStdItem.bWeight);
					g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[1]);
					break;
				case 43:		// 各种矿石.
					// 第二行.
					sprintf(&szLineBuf[1][strlen(szLineBuf[1])], " Weight %d Purity %d ", m_stItemInfo.stStdItem.bWeight, m_stItemInfo.nDura/1000);
					g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[1]);
					break;
				case 44:		// 商店里没有的特殊物品.
					break;
				case 45:		// 掉落在地面时发生变化的功能性物品.
					break;
				case 46:		// 没有功能的单纯物品。
				default:
					// 第二行.
					sprintf(&szLineBuf[1][strlen(szLineBuf[1])], " Weight %d ", m_stItemInfo.stStdItem.bWeight);
					g_xMainWnd.PutsHan(NULL, nX, nY+nLineGap, RGB(250, 250, 250), RGB(0, 0, 0), szLineBuf[1]);
					break;
				}
			}
			break;
		}				
	}
}


std::vector<CItem::HintLine> CItem::GeneralHint(int profession,int level,bool shop) const
{
    static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
    const auto word=[&](int offset) {return unsigned(m_serverRecord[offset])|(unsigned(m_serverRecord[offset+1])<<8);};
    std::vector<HintLine> lines;
    std::string title=captions.Get(2515);
    if(m_serverRecord[73]) {
        int size=0;while(size<27 && m_serverRecord[73+size]) ++size;
        title.append((const char*)m_serverRecord+73,size);title+=" ";title+=Name();
    } else {
        title+=Name();
        for(int i=0;i<10;++i) if(Name()==captions.Get(1409)+std::to_string(i)) {title=captions.Get(2510);break;}
    }
    lines.push_back({title,64255});
    const int stack=m_serverRecord[61],count=word(71),weight=stack==2?count/10:stack>2?count*m_serverRecord[32]:m_serverRecord[32];
    lines.push_back({captions.Get(2501)+": "+std::to_string(weight),0xffffff});
    if(stack>=2) lines.push_back({captions.Get(2503)+": "+std::to_string(count),0xffffff});
    const int mode=m_serverRecord[30],shape=m_serverRecord[31];
    const auto rounded=[&](int offset,int unit) {return int(double(word(offset))/unit+0.5);};
    const auto label=[&](int id,int value) {return captions.Get(id)+": "+std::to_string(value);};
    const auto durability=[&](int id,int unit,DWORD color) {lines.push_back({label(id,rounded(66,unit))+"/"+std::to_string(rounded(shop?38:68,unit)),color});};
    switch(mode) {
    case 0:
        if(word(40)) lines.push_back({captions.Get(1058)+": +"+std::to_string(word(40))+"HP ",0xffffff});
        if(word(42)) lines.push_back({captions.Get(1059)+": +"+std::to_string(word(42))+"MP ",0xffffff});
        break;
    case 3:
        if(shape==13 || shape==15) {const int required=m_serverRecord[40];lines.push_back({required?label(5140,required):captions.Get(2516),level>=required?0xffffffu:255u});}
        break;
    case 4:case 51:
        if(shape<3) {
            lines.push_back({captions.Get(5170+shape),0xffffff});
            const int required=word(38);
            lines.push_back({required?label(5140,required):captions.Get(5140)+": "+captions.Get(5182),profession==shape && level>=required?0xffffffu:255u});
        }
        break;
    case 25:durability(2511,100,16777088);break;
    case 30:durability(2500,1000,rounded(66,1000)>1?16777088:3289855);break;
    case 40:durability(2512,1000,16777088);break;
    case 42:lines.push_back({captions.Get(2513),0xffffff});break;
    case 43:lines.push_back({label(2514,rounded(66,1000)),16777088});break;
    case 60: {
        const int bonus=static_cast<signed char>(m_serverRecord[86]);
        std::string text=label(5177,m_serverRecord[40])+"-"+std::to_string(m_serverRecord[41]+max(0,bonus));
        if(bonus>0) text+=" (+"+std::to_string(bonus)+")";
        lines.push_back({text,bonus>0?13172630u:0xffffffu});break;
    }
    }
    return lines;
}
void CItem::ShowGeneralHint(int x,int y) const
{
    const auto& hero=g_xGameProc.m_xMyHero;
    const int mode=m_stItemInfo.stStdItem.bStdMode;
    const auto lines=mode==5 || mode==6 || mode==54 || mode==55 || mode==56
        ?WeaponHint(hero.m_stAbility.bLevel,HIBYTE(hero.m_stAbility.wDC),g_xGameProc.m_xInterface.m_xStatusWnd.m_elementValues[0])
        :mode==10 || mode==11
        ?ArmorHint(hero.m_stAbility.bLevel,HIBYTE(hero.m_stAbility.wDC),g_xGameProc.m_xInterface.m_xStatusWnd.m_elementValues[0])
        :GeneralHint(hero.m_bJob,hero.m_stAbility.bLevel);auto* surface=g_xMainWnd.GetBackBuffer();HDC dc;
    if(FAILED(surface->GetDC(&dc))) return;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};HFONT font=g_xMainWnd.CreateGameFont(face,9);
    auto previous=SelectObject(dc,font);int width=0;
    for(const auto& line:lines) {SIZE size={};GetTextExtentPoint32A(dc,line.text.c_str(),int(line.text.size()),&size);width=max(width,int(size.cx));}
    SelectObject(dc,previous);surface->ReleaseDC(dc);
    RECT box={x-5,y-5,x+width+5,y+int(lines.size())*15+2};
    if(box.right>800) OffsetRect(&box,800-box.right,0);
    DrawLegacyPanel(box,RGB(90,90,45),230,3315350);
    if(SUCCEEDED(surface->GetDC(&dc))) {
        previous=SelectObject(dc,font);SetBkMode(dc,TRANSPARENT);
        int lineY=box.top+5;
        for(const auto& line:lines) {SetTextColor(dc,line.color);TextOutA(dc,box.left+5,lineY,line.text.c_str(),int(line.text.size()));lineY+=15;}
        SelectObject(dc,previous);surface->ReleaseDC(dc);
    }
    DeleteObject(font);
}

std::vector<CItem::HintLine> CItem::ArmorHint(int level,int attack,const BYTE* elements,bool shop) const
{
    static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
    const auto* r=m_serverRecord;
    const auto word=[&](int at) {return int(r[at])|(int(r[at+1])<<8);};
    const auto signedByte=[&](int at) {return int(static_cast<signed char>(r[at]));};
    const auto label=[&](int id) {return captions.Get(id)+": ";};
    std::string title=captions.Get(2515);
    if(r[73]) {int length=0;while(length<27 && r[73+length]) ++length;title.append((const char*)r+73,length);title+=" ";}
    title+=Name();std::vector<HintLine> lines={{title,64255}};
    const int current=int(double(word(66))/1000+0.5),maximum=int(double(word(shop?38:68))/1000+0.5);
    lines.push_back({label(2500)+std::to_string(current)+"/"+std::to_string(maximum),current>1?16777088u:3289855u});
    lines.push_back({label(2501)+std::to_string(r[32]),0xffffff});
    const auto range=[&](int message,int offset,int extra,bool trailing=false) {
        const int bonus=signedByte(extra);
        std::string text=label(message)+std::to_string(r[offset])+"-"+std::to_string(r[offset+1]+max(0,bonus));
        if(bonus>0) text+=" (+"+std::to_string(bonus)+")";else if(trailing) text+=" ";
        lines.push_back({text,bonus>0?13172630u:0xffffffu});
    };
    if(word(44)+signedByte(88)>0) range(1061,44,88,true);
    range(5177,40,86);
    if(word(42)+signedByte(87)>0) range(1406,42,87);
    if(word(47)+signedByte(89)>0) range(5100+r[46],47,89,true);
    for(int i=0;i<2;++i) if(signedByte(94+i)>0) lines.push_back({label(2518+i)+"+"+std::to_string(10*signedByte(94+i)),13172630});
    if(signedByte(97)>0 || r[49]) {
        std::string text=captions.Get(5178)+":";int element=r[49];
        if(element!=signedByte(97) && element) text+=" "+captions.Get(5160+element)+"+"+std::to_string(word(50));
        if(signedByte(97)>0) element=r[97];
        if(signedByte(98)>0) text+=" "+captions.Get(5160+element)+"+"+std::to_string(signedByte(98)+(element==r[49]?word(50):0));
        lines.push_back({text,65408});
    }
    for(int kind=0;kind<2;++kind) {
        const int base=signedByte(52+kind),mask=signedByte(91+kind);
        if(mask<=0 && !r[52+kind]) continue;
        std::string text=captions.Get(5179+kind)+":";bool any=false;
        for(int element=1;element<=7;++element) {
            const bool bit=(mask&(1<<(element-1)))!=0,normal=base==element;
            if(!bit && !normal) continue;
            any=true;text+=" "+captions.Get(5160+element)+"x"+std::to_string(bit && normal?3:2);
        }
        if(any) lines.push_back({text,kind?13172630u:65408u});
    }
    if(signedByte(53)>0 && signedByte(53)<7) lines.push_back({captions.Get(5179)+":"+captions.Get(5160+r[53])+"x2",13172630});
    if(signedByte(97)>0) lines.push_back({label(5160+signedByte(97))+std::to_string(r[50])+"-"+std::to_string(r[51]+signedByte(98))+" (+"+std::to_string(signedByte(98))+")",65408});
    lines.push_back(RequirementHint(level,attack,elements));
    return lines;
}

CItem::HintLine CItem::RequirementHint(int level,int attack,const BYTE* elements) const
{
    static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
    const auto* r=m_serverRecord;const auto label=[&](int id) {return captions.Get(id)+": ";};
    const int kind=r[54],required=r[55];bool allowed=false;std::string condition;
    if(kind==0) {
        const int needed=required>100?required-100:required;allowed=level>=needed;
        condition=required?label(5140)+std::to_string(needed):captions.Get(2516);
        if(required>100) condition+=" ("+captions.Get(2517)+")";
    } else if(kind<=11) {
        condition=captions.Get(5140+kind)+std::to_string(required)+" ";
        if(kind==1) allowed=attack>=required;
        else if(kind>=5) allowed=elements[kind-5]>=required;
        else {
            allowed=true;const int first=kind==3?4:0,last=kind==2?4:7;
            for(int i=first;i<last;++i) if(elements[i]<required) allowed=false;
        }
    }
    return {condition,!allowed?255u:kind==0 && (required==0 || required>100)?51200u:0xffffffu};
}

std::vector<CItem::HintLine> CItem::WeaponHint(int level,int attack,const BYTE* elements,bool shop) const
{
    static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
    const auto* r=m_serverRecord;const auto word=[&](int at) {return int(r[at])|(int(r[at+1])<<8);};
    const auto signedByte=[&](int at) {return int(static_cast<signed char>(r[at]));};
    const auto label=[&](int id) {return captions.Get(id)+": ";};
    const bool special=r[30]==54 || r[30]==55;
    std::string title=captions.Get(2515)+(r[96]?" (*)":" ");
    if(r[73]) {int length=0;while(length<27 && r[73+length]) ++length;title.append((const char*)r+73,length);title+=" ";}
    title+=Name();std::vector<HintLine> lines={{title,64255}};
    const int current=int(double(word(66))/(special?100:1000)+0.5),maximum=int(double(word(shop?38:68))/1000+0.5);
    lines.push_back({label(special?2503:2500)+std::to_string(current)+(special?"":"/"+std::to_string(maximum)),current>1?16777088u:3289855u});
    lines.push_back({label(2501)+std::to_string(r[32]),0xffffff});
    const auto range=[&](int id,int offset,int extra) {
        const int bonus=signedByte(extra);if(word(offset)+bonus<=0) return;
        std::string text=label(id)+std::to_string(r[offset])+"-"+std::to_string(r[offset+1]+max(0,bonus));
        text+=bonus>0?" (+"+std::to_string(bonus)+")":" ";lines.push_back({text,bonus>0?13172630u:0xffffffu});
    };
    range(1061,44,86);range(5100+r[46],47,87);
    if(signedByte(97)>0 || r[49]) {
        const int element=signedByte(97)>0?r[97]:r[49];
        lines.push_back({label(5178)+captions.Get(5160+element)+"+"+std::to_string(word(50)+max(0,signedByte(98))),65408});
    }
    const auto value=[&](int id,int base,int bonus,bool plus) {
        std::string text=label(id)+(plus?"+":"")+std::to_string(base+max(0,bonus));
        if(bonus>0) text+=" (+"+std::to_string(bonus)+")";
        lines.push_back({text,bonus>0?13172630u:0xffffffu});
    };
    if(signedByte(34)+signedByte(93)>0) value(2504,signedByte(34),signedByte(93),false);
    if(r[41]+signedByte(91)>0) value(1053,r[41]<100?r[41]:100-r[41],signedByte(91),false);
    const int speed=r[43]+signedByte(92),extra=signedByte(92);
    if(speed>0) {
        std::string text=label(2505);
        if(speed<=10) {text+="-"+std::to_string(r[43]+max(0,extra));if(extra>0) text+=" (-"+std::to_string(extra)+")";}
        else {text+="+"+std::to_string(r[43]+max(0,extra)-10);if(extra>0) text+=" (+"+std::to_string(extra-10)+")";}
        lines.push_back({text,extra>0?13172630u:0xffffffu});
    }
    if(r[40]+signedByte(89)>0) value(2506,r[40],signedByte(89),true);
    if(r[42]+signedByte(90)>0) value(2507,r[42],signedByte(90),true);
    if(!special) {
        // Original 4B52A4 thresholds; each step corresponds to 60000 experience.
        static const int thresholds[]={0,3,6,9,13,17,21,25,30,35,40,45,51,57,63,69,76};
        int index=0;while(index<16 && r[70]>=thresholds[index+1]) ++index;
        if(index==16) lines.push_back({captions.Get(2508)+" : MAX",0xffffff});
        else {
            lines.push_back({captions.Get(2508)+" : "+std::to_string(index+1),0xffffff});
            const unsigned progress=word(71)+60000u*r[70]-60000u*thresholds[index];
            char percent[64];sprintf(percent,"%.2f %%",100.0*progress/(60000*(thresholds[index+1]-thresholds[index])));
            lines.push_back({captions.Get(2509)+" : "+percent,0xffffff});
        }
    }
    lines.push_back(RequirementHint(level,attack,elements));return lines;
}
