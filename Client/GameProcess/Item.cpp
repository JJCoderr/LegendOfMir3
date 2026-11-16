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
    if(m_serverName[0] && m_stItemInfo.stStdItem.bStdMode==0) {ShowPotionHint(nX,nY);return;}
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


std::vector<CItem::HintLine> CItem::PotionHint() const
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
    if(word(40)) lines.push_back({captions.Get(1058)+": +"+std::to_string(word(40))+"HP ",0xffffff});
    if(word(42)) lines.push_back({captions.Get(1059)+": +"+std::to_string(word(42))+"MP ",0xffffff});
    return lines;
}
void CItem::ShowPotionHint(int x,int y) const
{
    const auto lines=PotionHint();auto* surface=g_xMainWnd.GetBackBuffer();HDC dc;
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
