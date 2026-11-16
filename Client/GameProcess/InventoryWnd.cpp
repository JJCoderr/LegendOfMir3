/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/



#include "StdAfx.h"
#include "Common/LoginMessages.h"



/******************************************************************************************************************

	CInventoryWnd Class

*******************************************************************************************************************/

CInventoryWnd::CInventoryWnd()
{
	Init();
}


CInventoryWnd::~CInventoryWnd()
{
	Destroy();
}


VOID CInventoryWnd::Init()
{
	INT nCnt;
	CGameWnd::Init();
	m_nStartLineNum	= 0;
    m_closePressed=false;
    m_layout.Clear();
	ZeroMemory(m_rcInvenCell, sizeof(RECT) *_INVEN_MAX_CELL);
	ZeroMemory(m_rcBeltCell,  sizeof(RECT) *_BELT_MAX_CELL );
	ZeroMemory(m_stInventoryItem, sizeof(ITEMSET)*_MAX_INVEN_ITEM);
	ZeroMemory(m_stBeltItem,	  sizeof(ITEMSET)*_MAX_BELT_ITEM );

    m_scrollCaptured=false; m_scrollTick=0;

	for ( nCnt = 0; nCnt < _MAX_INVEN_BTN; nCnt++ )
	{
		m_xInventoryBtn[nCnt].Init();
	}
}


VOID CInventoryWnd::Destroy()
{
	INT nCnt;
	CGameWnd::Destroy();
	for ( nCnt = 0; nCnt < _MAX_INVEN_BTN; nCnt++ )
	{
		m_xInventoryBtn[nCnt].Destroy();
	}
	Init();
}


VOID CInventoryWnd::CreateInventoryWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove)
{
	// 窗口创建.
	CreateGameWnd(nID, pxWndImage, nFrameImgIdx, bCanMove, nStartX, nStartY, nWidth, nHeight);

	// 按钮创建.
	m_xInventoryBtn[_BTN_ID_INVENCLOSE	 ].CreateGameBtn(pxWndImage, 1221, 1222, nStartX+242, nStartY+423);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLUP	 ].CreateGameBtn(pxWndImage, 74, 75, nStartX+247, nStartY+ 20);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLDOWN].CreateGameBtn(pxWndImage, 74, 75, nStartX+247, nStartY+167);
	

	// 背包格子区域的 Set.
	for ( INT nYCnt = 0; nYCnt < _INVEN_CELL_YCNT; nYCnt++ )
	{
		for ( INT nXCnt = 0; nXCnt < _INVEN_CELL_XCNT; nXCnt++ )
		{
			SetRect(&m_rcInvenCell[nXCnt+nYCnt*_INVEN_CELL_XCNT], 
					_INVEN_CELL_XSTART+(nXCnt*_INVENTORY_CELL_WIDTH), 
					_INVEN_CELL_YSTART+(nYCnt*_INVENTORY_CELL_HEIGHT),
					_INVEN_CELL_XSTART+(nXCnt*_INVENTORY_CELL_WIDTH) +_INVENTORY_CELL_WIDTH, 
					_INVEN_CELL_YSTART+(nYCnt*_INVENTORY_CELL_HEIGHT)+_INVENTORY_CELL_HEIGHT);
		}
	}

	// 腰带单元格区域的 Set。
	for ( INT nCnt = 0; nCnt < _MAX_BELT_ITEM; nCnt++ )
	{
		SetRect(&m_rcBeltCell[nCnt], 
				_BELT_CELL_XSTART+(nCnt*_BELT_CELL_XGAP), 
				_BELT_CELL_YSTART,
				_BELT_CELL_XSTART+(nCnt*_BELT_CELL_XGAP)+_INVENTORY_CELL_WIDTH, 
				_BELT_CELL_YSTART+_INVENTORY_CELL_HEIGHT);
	}
}



VOID CInventoryWnd::SetInvenBtnInit()
{
	for( INT nCnt = 0; nCnt < _MAX_INVEN_BTN; nCnt++ )
	{
		m_xInventoryBtn[nCnt].SetBtnState(_BTN_STATE_NORMAL);
	}
}


VOID CInventoryWnd::ShowInventoryWnd()
{
	INT nCnt;

	// 窗口背景绘制.
    const int x=m_rcWnd.left,y=m_rcWnd.top;
    if (m_pxWndImage->NewSetIndex(1220)) {
        const auto& image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        g_xMainWnd.DrawWithImageForComp(x-(image.shWidth-282)/2,y-(image.shHeight-466)/2,
            image.shWidth,image.shHeight,(WORD*)m_pxWndImage->m_pbCurrImage);
    }
    if (m_pxWndImage->NewSetIndex(1225)) {
        const auto& image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        g_xMainWnd.DrawWithImageForComp(x+250,y+70+int(254.0f*m_nStartLineNum/91),
            image.shWidth,image.shHeight,(WORD*)m_pxWndImage->m_pbCurrImage);
    }
    HDC dc; auto* surface=g_xMainWnd.GetBackBuffer();
    if (SUCCEEDED(surface->GetDC(&dc))) {
        const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
        HFONT font=g_xMainWnd.CreateGameFont(face,9); HGDIOBJ previous=SelectObject(dc,font);
        SetBkMode(dc,TRANSPARENT);
        static CLoginMessages captions; static const bool loaded=captions.Load("./CmList.dat");
        const auto text=[&](const char* value,int left,int top,int width,int height,COLORREF color) {
            RECT area={x+left,y+top,x+left+width,y+top+height}; SetTextColor(dc,color);
            DrawTextA(dc,value,-1,&area,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        };
        text(captions.Get(1701).c_str(),12,20,74,16,0xf8dcfa);
        char weight[128]; const auto& ability=g_xGameProc.m_xMyHero.m_stAbility;
        wsprintfA(weight,"%s: %u / %u",captions.Get(2501).c_str(),ability.wWeight,ability.wMaxWeight);
        text(weight,95,21,126,16,0x0a0a0a); text(weight,94,20,126,16,0xf8c8c8);
        SelectObject(dc,previous); DeleteObject(font); surface->ReleaseDC(dc);
    }
    m_xInventoryBtn[_BTN_ID_INVENCLOSE].ChangeRect(x+242,y+423);
    const RECT clip={x+17,y+60,x+245,y+364};
    for (nCnt=0;nCnt<_MAX_INVEN_ITEM;++nCnt) {
        if (!m_stInventoryItem[nCnt].bSetted) continue;
        const int anchor=m_layout.anchors[nCnt];
        if (anchor<0) continue;
        const int itemX=x+17+(anchor%6)*38+(m_layout.widths[nCnt]-1)*19;
        const int itemY=y+60+(anchor/6-m_nStartLineNum)*38+(m_layout.heights[nCnt]-1)*19;
        m_stInventoryItem[nCnt].xItem.DrawItem(itemX,itemY,_ITEM_TYPE_INVENTORY,&clip);
    }
	for ( nCnt = 0; nCnt < _MAX_INVEN_BTN; nCnt++ )
	{
		m_xInventoryBtn[nCnt].ShowGameBtn();
	}

	// 显示物品的状态值。
	ShowInvenItemState();	
}




VOID CInventoryWnd::ShowInvenItemState()
{
    const int id=GetInvenNum(g_xGameProc.m_ptMousePos);
    if (id<0 || !m_stInventoryItem[id].bSetted) return;
    const int anchor=m_layout.anchors[id];
    m_stInventoryItem[id].xItem.ShowItemStatus(m_rcWnd.left+32+(anchor%6)*38,
        m_rcWnd.top+75+(anchor/6-m_nStartLineNum)*38);
}














/******************************************************************************************************************

	物品放入与取出相关, 背包位置的保存文件存盘及加载.(背包及腰带)

*******************************************************************************************************************/

BOOL CInventoryWnd::CheckItemPos(CHAR* szName)
{
	HANDLE	hFile;
	CHAR	szItemFile[MAX_PATH];
	INT		nCnt1, nCnt2;
	ITEMSET	stInvenItem[_MAX_INVEN_ITEM];						// 临时数据加载用变量.
	ITEMSET	stBeltItem[_MAX_BELT_ITEM];							
	ITEMSET	stCheckedInvenItem[_MAX_INVEN_ITEM];				// 保存已检查数据的变量.
	ITEMSET	stCehckedBeltItem[_MAX_BELT_ITEM];

	ZeroMemory(szItemFile, MAX_PATH);
	g_xMainWnd.StringPlus(szItemFile, ".\\Data\\", szName, ".itm", "");

	hFile = CreateFile(szItemFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	ZeroMemory(stInvenItem, sizeof(ITEMSET)*_MAX_INVEN_ITEM);
	ZeroMemory(stCheckedInvenItem, sizeof(ITEMSET)*_MAX_INVEN_ITEM);
	ZeroMemory(stBeltItem , sizeof(ITEMSET)*_MAX_BELT_ITEM );
	ZeroMemory(stCehckedBeltItem, sizeof(ITEMSET)*_MAX_BELT_ITEM);

	if ( hFile != INVALID_HANDLE_VALUE )
	{
		DWORD		dwReadLen;

		ReadFile(hFile, &stInvenItem, sizeof(ITEMSET)*_MAX_INVEN_ITEM, &dwReadLen, NULL);
		ReadFile(hFile, &stBeltItem , sizeof(ITEMSET)*_MAX_BELT_ITEM , &dwReadLen, NULL);

		// 将服务器接收的背包数据与客户端最后保存的腰带窗口数据进行比较，并分配到正确的位置。
		for ( nCnt1 = 0; nCnt1 < _MAX_BELT_ITEM; nCnt1++ )
		{
			if ( stBeltItem[nCnt1].bSetted )
			{
				for ( nCnt2 = 0; nCnt2 < _MAX_INVEN_ITEM; nCnt2++ )
				{
					if ( m_stInventoryItem[nCnt2].bSetted )
					{
						// 判断是否为相同物品.
						if ( stBeltItem[nCnt1].xItem.m_stItemInfo.nMakeIndex  != m_stInventoryItem[nCnt2].xItem.m_stItemInfo.nMakeIndex &&
							 !strcmp(stBeltItem[nCnt1].xItem.m_stItemInfo.stStdItem.szName, m_stInventoryItem[nCnt2].xItem.m_stItemInfo.stStdItem.szName) &&
							 (stBeltItem[nCnt1].xItem.m_stItemInfo.nDura    == m_stInventoryItem[nCnt2].xItem.m_stItemInfo.nDura   ) &&
							 (stBeltItem[nCnt1].xItem.m_stItemInfo.nDuraMax == m_stInventoryItem[nCnt2].xItem.m_stItemInfo.nDuraMax) )
						{
							stCehckedBeltItem[nCnt1] = m_stInventoryItem[nCnt2];
							ZeroMemory(&stBeltItem[nCnt1], sizeof(ITEMSET));
							DeleteInvenItem(nCnt2);
							break;
						}						   
					}
				}
			}
		} 

		memcpy(m_stBeltItem, stCehckedBeltItem, sizeof(ITEMSET)*_MAX_BELT_ITEM);

		// 背包.
		for ( nCnt1 = 0; nCnt1 < _MAX_INVEN_ITEM; nCnt1++ )
		{
			if ( stInvenItem[nCnt1].bSetted )
			{
				for ( nCnt2 = 0; nCnt2 < _MAX_INVEN_ITEM; nCnt2++ )
				{
					if ( m_stInventoryItem[nCnt2].bSetted )
					{
						// 判断是否为相同物品.
						if ( stInvenItem[nCnt1].xItem.m_stItemInfo.nMakeIndex != m_stInventoryItem[nCnt2].xItem.m_stItemInfo.nMakeIndex &&
							 !strcmp(stInvenItem[nCnt1].xItem.m_stItemInfo.stStdItem.szName, m_stInventoryItem[nCnt2].xItem.m_stItemInfo.stStdItem.szName) &&
							 (stInvenItem[nCnt1].xItem.m_stItemInfo.nDura	 == m_stInventoryItem[nCnt2].xItem.m_stItemInfo.nDura    ) &&
							 (stInvenItem[nCnt1].xItem.m_stItemInfo.nDuraMax == m_stInventoryItem[nCnt2].xItem.m_stItemInfo.nDuraMax ) )
						{
							stCheckedInvenItem[nCnt1] = m_stInventoryItem[nCnt2];
							ZeroMemory(&stInvenItem[nCnt1], sizeof(ITEMSET));
							DeleteInvenItem(nCnt2);
							break;
						}						
					}
				}
			}
		}

		for ( nCnt1 = 0; nCnt1 < _MAX_INVEN_ITEM; nCnt1++ )
		{
			if ( m_stInventoryItem[nCnt1].bSetted )
			{
				for ( nCnt2 = 0; nCnt2 < _MAX_INVEN_ITEM; nCnt2++ )
				{
					if ( !stCheckedInvenItem[nCnt2].bSetted )
					{
						stCheckedInvenItem[nCnt2] = m_stInventoryItem[nCnt1];
						break;
					}
				}
			}
		}
		memcpy(m_stInventoryItem, stCheckedInvenItem, sizeof(ITEMSET)*_MAX_INVEN_ITEM);
        RebuildLayout();

		CloseHandle(hFile);
		return TRUE;
	}

	return FALSE;
}


BOOL CInventoryWnd::SaveItemPos(CHAR* szName)
{
	HANDLE	hFile;
	CHAR	szItemFile[MAX_PATH];

	ZeroMemory(szItemFile, MAX_PATH);
	g_xMainWnd.StringPlus(szItemFile, ".\\Data\\", szName, ".itm", "");
	hFile = CreateFile(szItemFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if ( hFile != INVALID_HANDLE_VALUE )
	{
		DWORD		dwWriteLen;

		WriteFile(hFile, m_stInventoryItem, sizeof(ITEMSET)*_MAX_INVEN_ITEM, &dwWriteLen, NULL);
		WriteFile(hFile, m_stBeltItem,		sizeof(ITEMSET)*_MAX_BELT_ITEM , &dwWriteLen, NULL);

		CloseHandle(hFile);
		return TRUE;
	}
	return FALSE;
}


/******************************************************************************************************************

	函数名 : CInventoryWnd::GetInvenNum()

	作者 :
	编写日期 :

	目的   : 获取鼠标位置处当前背包的单元格编号。
	输入   : POINT ptMouse
	输出   : INT

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
int CInventoryWnd::SlotAt(POINT point) const
{
    const int x=point.x-m_rcWnd.left-17,y=point.y-m_rcWnd.top-60;
    if (x<0 || y<0 || x>=228 || y>=304) return -1;
    return x/38+6*(y/38+m_nStartLineNum);
}
INT CInventoryWnd::GetInvenNum(POINT point)
{
    const int slot=SlotAt(point);
    if (slot<0) return -1;
    if (m_layout.cells[slot]>=0) return m_layout.cells[slot];
    for (int id=0;id<_MAX_INVEN_ITEM;++id) if (!m_stInventoryItem[id].bSetted) return id;
    return -1;
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::GetBeltNum()

	作者 :
	创建日期 :

	目的   : 取得鼠标位置处当前腰带的格子编号.
	输入   : POINT ptMouse
	输出   : INT

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
INT CInventoryWnd::GetBeltNum(POINT ptMouse)
{
    return -1; // Quick items are handled by the HUD.
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::DeleteInventoryCurrItem()

	作者 :
	创建日期 :

	目的   : 删除背包中对应格子的物品.
	输入   : INT nInventoryNum
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::DeleteInvenItem(INT nInventoryNum)
{
	ZeroMemory(&m_stInventoryItem[nInventoryNum], sizeof(ITEMSET));
    m_layout.Remove(nInventoryNum);
}



VOID CInventoryWnd::DeleteInvenItem(INT nMakeIndex, CHAR* szName)
{
	for ( INT nCnt = 0; nCnt < _MAX_INVEN_ITEM; nCnt++ )
	{
		if ( m_stInventoryItem[nCnt].bSetted )
		{
			if ( m_stInventoryItem[nCnt].xItem.m_stItemInfo.nMakeIndex == nMakeIndex )
			{
				if ( !strcmp(m_stInventoryItem[nCnt].xItem.m_stItemInfo.stStdItem.szName, szName) )
				{
					DeleteInvenItem(nCnt);
					break;
				}
			}
		}		
	}
}


/******************************************************************************************************************

	函数名 : CInventoryWnd::DeleteInventoryAllItem()

	作者 : 
	编写日期 : 

	目的   : 清除背包中的所有物品.
	输出   : VOID 

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::DeleteAllInvenItem()
{
    m_layout.Clear();
	for ( INT nCnt = 0; nCnt < _MAX_INVEN_ITEM; nCnt++ )
	{
		ZeroMemory(&m_stInventoryItem[nCnt], sizeof(ITEMSET));
	}
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::DeleteCurrBeltItemList()

	作者 :
	编写日期 :

	目的   : 删除腰带对应格子的物品.
	输入   : INT nBeltInventoryNum
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::DeleteBeltItem(INT nBeltInventoryNum)
{
	ZeroMemory(&m_stBeltItem[nBeltInventoryNum], sizeof(ITEMSET));
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::DeleteAllBeltItemList()

	作者 : 
	编写日期 : 

	目的   : 删除腰带中的所有物品.
	输入   : INT nBeltInventoryNum
	输出   : VOID 

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::DeleteAllBeltItem()
{
	for ( INT nCnt = 0; nCnt < _MAX_BELT_ITEM; nCnt++ )
	{
		ZeroMemory(&m_stBeltItem[nCnt], sizeof(ITEMSET));
	}
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::AddInvenItem()

	作者 :
	编写日期 :

	目的   : 将物品放入背包。
	输入   : CItem xItem
	         INT nInvenNum
	         BOOL bUseInvenNum
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
bool CInventoryWnd::PlaceItem(int id,const CItem& item,const POINT* point)
{
    auto& image=g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY];
    if (!image.NewSetIndex(item.m_stItemInfo.stStdItem.wLooks)) return false;
    const int width=(image.m_lpstNewCurrWilImageInfo->shWidth+37)/38;
    const int height=(image.m_lpstNewCurrWilImageInfo->shHeight+37)/38;
    int x=-1,y=-1;
    if (point) {
        const int slot=SlotAt(*point);
        if (slot>=0) {
            x=slot%6; y=slot/6;
            if (width%2==0 && point->x>m_rcWnd.left+17+38*x+19) ++x;
            if (height%2==0 && point->y>m_rcWnd.top+60+38*y+19) ++y;
            x-=width/2; y-=height/2;
        }
    }
    return m_layout.Place(id,width,height,x,y);
}
void CInventoryWnd::RebuildLayout()
{
    m_layout.Clear();
    for (int id=0;id<_MAX_INVEN_ITEM;++id)
        if (m_stInventoryItem[id].bSetted) PlaceItem(id,m_stInventoryItem[id].xItem);
}
BOOL CInventoryWnd::AddInvenItem(CItem item,INT preferred,BOOL usePreferred)
{
    int id=usePreferred && preferred>=0 && preferred<_MAX_INVEN_ITEM && !m_stInventoryItem[preferred].bSetted ? preferred : -1;
    if (id<0) for (int i=0;i<_MAX_INVEN_ITEM;++i) if (!m_stInventoryItem[i].bSetted) { id=i; break; }
    if (id<0 || !PlaceItem(id,item)) return FALSE;
    m_stInventoryItem[id].bSetted=TRUE; m_stInventoryItem[id].xItem=item;
    return TRUE;
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::AddBeltItem()

	作者 :
	创建日期 :

	目的   : 把物品放入腰带.
	输入   : CItem xItem
	         INT nBeltNum
	         BOOL bUseBeltNum
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CInventoryWnd::AddBeltItem(CItem xItem, INT nBeltNum, BOOL bUseBeltNum)
{
	if ( bUseBeltNum )
	{
		if ( !m_stBeltItem[nBeltNum].bSetted )
		{
			m_stBeltItem[nBeltNum].bSetted = TRUE;
			m_stBeltItem[nBeltNum].xItem = xItem;

			return TRUE;
		}
	}

	for ( INT nCnt = 0; nCnt < _MAX_BELT_ITEM; nCnt++ )
	{
		if ( !m_stBeltItem[nCnt].bSetted )
		{
			m_stBeltItem[nCnt].bSetted = TRUE;
			m_stBeltItem[nCnt].xItem = xItem;

			return TRUE;
		}
	}
	return FALSE;
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::ExtractInvenItem()

	作者 :
	编写日期 :

	目的   : 从背包中取出当前编号的物品。
	输入   : CItem* pxItem
	         INT nInventoryNum
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CInventoryWnd::ExtractInvenItem(CItem* pxItem, INT nInventoryNum)
{
	if ( m_stInventoryItem[nInventoryNum].bSetted )
	{
		memcpy(pxItem, &m_stInventoryItem[nInventoryNum].xItem, sizeof(CItem));
		DeleteInvenItem(nInventoryNum);
				
		return TRUE;
	}
	return FALSE;
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::ExtractBeltItem()

	作者 :
	编写日期 :

	目的   : 从背包窗口中取出当前编号的物品.
	输入   : CItem* pxItem
	         INT nBeltInventoryNum
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CInventoryWnd::ExtractBeltItem(CItem* pxItem, INT nBeltInventoryNum)
{
	if ( m_stBeltItem[nBeltInventoryNum].bSetted )
	{
		memcpy(pxItem, &m_stBeltItem[nBeltInventoryNum].xItem, sizeof(CItem));
		DeleteBeltItem(nBeltInventoryNum);
				
		return TRUE;
	}
	return FALSE;
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::ChangeInvenWithCommon()

	作者 :
	编写日期 :

	目的   : 将背包中的物品与公共(鼠标)物品交换.
	输入   : INT nInventoryNum
	         LPCOMMONITEMSET pstCommonItemSet
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::ChangeInvenWithCommon(INT id,LPCOMMONITEMSET held,const POINT* point)
{
    if (!held->bSetted || !m_stInventoryItem[id].bSetted) return;
    const CBagLayout previous=m_layout;
    m_layout.Remove(id);
    if (!PlaceItem(id,held->xItem,point)) { m_layout=previous; return; }
    const CItem old=m_stInventoryItem[id].xItem;
    m_stInventoryItem[id].xItem=held->xItem;
    held->xItem=old; held->xItem.m_shCellNum=id;
    held->bWaitResponse=held->bIsEquipItem=held->bIsHideItem=held->bIsBeltItem=FALSE;
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::ChangeBeltWithCommon()

	作者 :
	日期 :

	目的   : 更换腰带物品与公共(鼠标)物品。
	输入   : INT nBeltInventoryNum
	         LPCOMMONITEMSET pstCommonItemSet
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::ChangeBeltWithCommon(INT nBeltInventoryNum, LPCOMMONITEMSET pstCommonItemSet)
{
	if ( pstCommonItemSet->bSetted )
	{
		CItem xBeltItem;
		if ( ExtractBeltItem(&xBeltItem, nBeltInventoryNum) )
		{
			AddBeltItem(pstCommonItemSet->xItem, nBeltInventoryNum, TRUE);

			pstCommonItemSet->bSetted			= TRUE;
			pstCommonItemSet->xItem				= xBeltItem;
			pstCommonItemSet->bWaitResponse		= FALSE;
			pstCommonItemSet->bIsEquipItem		= FALSE;
			pstCommonItemSet->bIsHideItem		= FALSE;
			pstCommonItemSet->bIsBeltItem		= TRUE;
			pstCommonItemSet->xItem.m_shCellNum = nBeltInventoryNum; 
		}
	}
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::SetCommonFromInven()

	作者 :
	创建日期 :

	目的   : 把背包中对应编号的物品设置为公共(鼠标)物品.
	输入   : INT nInventoryNum
	         LPCOMMONITEMSET pstCommonItemSet
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::SetCommonFromInven(INT nInventoryNum, LPCOMMONITEMSET pstCommonItemSet)
{
	CItem xInvenItem;
	if ( ExtractInvenItem(&xInvenItem, nInventoryNum) )
	{
		pstCommonItemSet->bSetted			= TRUE;
		pstCommonItemSet->xItem				= xInvenItem;
		pstCommonItemSet->bWaitResponse		= FALSE;
		pstCommonItemSet->bIsEquipItem		= FALSE;
		pstCommonItemSet->bIsHideItem		= FALSE;
		pstCommonItemSet->bIsBeltItem		= FALSE;
		pstCommonItemSet->xItem.m_shCellNum = nInventoryNum; 
	}
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::SetCommonFromBelt()

	作者 :
	编写日期 :

	目的   : 把腰带窗口中对应编号的物品设置为公共(鼠标)物品。
	输入   : INT nBeltInventoryNum
	         LPCOMMONITEMSET pstCommonItemSet
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::SetCommonFromBelt(INT nBeltInventoryNum, LPCOMMONITEMSET pstCommonItemSet)
{
	CItem xBeltItem;
	if ( ExtractBeltItem(&xBeltItem, nBeltInventoryNum) )
	{
		pstCommonItemSet->bSetted			= TRUE;
		pstCommonItemSet->xItem				= xBeltItem;
		pstCommonItemSet->bWaitResponse		= FALSE;
		pstCommonItemSet->bIsEquipItem		= FALSE;
		pstCommonItemSet->bIsHideItem		= FALSE;
		pstCommonItemSet->bIsBeltItem		= TRUE;
		pstCommonItemSet->xItem.m_shCellNum = nBeltInventoryNum; 
	}
}	



/******************************************************************************************************************

	函数名 : CInventoryWnd::SetInvenFromCommon()

	作者 :
	编写日期 :

	目的   : 将公共(鼠标)物品设置到背包的对应编号。
	输入   : INT nInventoryNum
	         LPCOMMONITEMSET pstCommonItemSet
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::SetInvenFromCommon(INT id,LPCOMMONITEMSET held,const POINT* point)
{
    if (!held->bSetted || !PlaceItem(id,held->xItem,point)) return;
    m_stInventoryItem[id].bSetted=TRUE; m_stInventoryItem[id].xItem=held->xItem;
    m_stInventoryItem[id].xItem.m_shCellNum=id;
    ZeroMemory(held,sizeof(COMMONITEMSET));
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::SetBeltFromCommon()

	作者 :
	编写日期 :

	目的   : 把公共(鼠标)物品设置到腰带窗口的对应编号上.
	输入   : INT nBeltInventoryNum
	         LPCOMMONITEMSET pstCommonItemSet
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::SetBeltFromCommon(INT nBeltInventoryNum, LPCOMMONITEMSET pstCommonItemSet)
{
	if ( pstCommonItemSet->bSetted )
	{
		pstCommonItemSet->xItem.m_shCellNum = nBeltInventoryNum;
		AddBeltItem(pstCommonItemSet->xItem, nBeltInventoryNum);

		ZeroMemory(pstCommonItemSet, sizeof(LPCOMMONITEMSET));
	}
}


VOID CInventoryWnd::AddNewBeltItem(LPCOMMONITEMSET pstCommonItemSet)
{
	INT nCnt;
	if ( pstCommonItemSet->bSetted && pstCommonItemSet->bIsBeltItem )
	{
		if ( /*pstCommonItemSet->xItem.m_stItemInfo.szMakeIndex[0] == 'G' && */ 
			 (pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode == 0 || pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode == 3) )
		{
			for ( nCnt = 0; nCnt < _MAX_INVEN_ITEM; nCnt++ )
			{
				if ( m_stInventoryItem[nCnt].bSetted && 
					 m_stInventoryItem[nCnt].xItem.m_stItemInfo.stStdItem.bStdMode == pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode &&
					 m_stInventoryItem[nCnt].xItem.m_stItemInfo.stStdItem.bShape == pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bShape && 
					 m_stInventoryItem[nCnt].xItem.m_stItemInfo.nDura == pstCommonItemSet->xItem.m_stItemInfo.nDura && 
					 m_stInventoryItem[nCnt].xItem.m_stItemInfo.nDuraMax == pstCommonItemSet->xItem.m_stItemInfo.nDuraMax )
					 
				{
					if ( !m_stBeltItem[pstCommonItemSet->xItem.m_shCellNum].bSetted )
					{
						m_stBeltItem[pstCommonItemSet->xItem.m_shCellNum].xItem = m_stInventoryItem[nCnt].xItem;
						m_stBeltItem[pstCommonItemSet->xItem.m_shCellNum].bSetted = TRUE;
						DeleteInvenItem(nCnt);
					}
					break;
				}
			}
		}			 
	}
}












/******************************************************************************************************************

	Message Function.(鼠标输入)

*******************************************************************************************************************/
BOOL CInventoryWnd::OnKeyUp(LPCOMMONITEMSET pstCommonItemSet, WPARAM wParam, LPARAM lParam)
{
	INT nBeltInventoryNum;

	nBeltInventoryNum = -1;
	
	switch ( wParam )
	{
	case '1': 	nBeltInventoryNum = 0;		break;
	case '2': 	nBeltInventoryNum = 1;		break;
	case '3': 	nBeltInventoryNum = 2;		break;
	case '4': 	nBeltInventoryNum = 3;		break;
	case '5': 	nBeltInventoryNum = 4;		break;
	case '6': 	nBeltInventoryNum = 5;		break;
	}

	if ( nBeltInventoryNum != -1 )
	{
		if ( m_stBeltItem[nBeltInventoryNum].bSetted )
		{
			if ( !pstCommonItemSet->bSetted )
			{				  
				SetCommonFromBelt(nBeltInventoryNum, pstCommonItemSet);
			}
			else if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
			{		
				if ( /*pstCommonItemSet->xItem.m_stItemInfo.szMakeIndex[0] == 'G' && */
					 (pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode == 0 || pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode == 3) )
					ChangeBeltWithCommon(nBeltInventoryNum, pstCommonItemSet);
			}
		}
		else
		{
			if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
			{				  
				if ( /*pstCommonItemSet->xItem.m_stItemInfo.szMakeIndex[0] == 'G' && */
					 (pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode == 0 || pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode == 3) )
					 SetBeltFromCommon(nBeltInventoryNum, pstCommonItemSet);
			}
		}

		if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
		{			
			pstCommonItemSet->bWaitResponse = TRUE;
			pstCommonItemSet->bIsHideItem	= TRUE;
			g_xClientSocket.SendItemIndex(CM_EAT, pstCommonItemSet->xItem.m_stItemInfo.nMakeIndex, pstCommonItemSet->xItem.m_stItemInfo.stStdItem.szName);
			return TRUE;
		}
	}

	return FALSE;
}


BOOL CInventoryWnd::OnLButtonDown(LPCOMMONITEMSET pstCommonItemSet, POINT ptMouse)
{
	INT nCnt, nInventoryNum, nBeltInventoryNum;
    m_closePressed=false;
    if (ScrollInput(0,ptMouse)) return TRUE;

	m_xInventoryBtn[_BTN_ID_INVENCLOSE	 ].ChangeRect(m_rcWnd.left+242, m_rcWnd.top+423);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLUP	 ].ChangeRect(m_rcWnd.left+247, m_rcWnd.top+ 20);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLDOWN].ChangeRect(m_rcWnd.left+247, m_rcWnd.top+167);


	for( nCnt = 0; nCnt < _MAX_INVEN_BTN; nCnt++ )
	{
		if ( m_xInventoryBtn[nCnt].OnLButtonDown(ptMouse) )		
        { m_closePressed=true; return TRUE; }
	}

	nInventoryNum	  = GetInvenNum(ptMouse);
	nBeltInventoryNum = GetBeltNum(ptMouse);

	if ( (nInventoryNum == -1 && nBeltInventoryNum == -1) )		m_bCanMove = TRUE;
	else													
	{
		if ( nInventoryNum != -1 )
		{
			if ( m_stInventoryItem[nInventoryNum].bSetted )		m_bCanMove = FALSE;
			else												m_bCanMove = TRUE;				
		}
		else if ( nBeltInventoryNum != -1 )
		{
			if ( m_stBeltItem[nInventoryNum].bSetted )			m_bCanMove = FALSE;
			else												m_bCanMove = TRUE;				
		}
	}
	
	return FALSE;
}



BOOL CInventoryWnd::OnLButtonUp(LPCOMMONITEMSET pstCommonItemSet, POINT ptMouse)
{
    if (ScrollInput(2,ptMouse)) return FALSE;
	INT nInventoryNum, nBeltInventoryNum;

	m_xInventoryBtn[_BTN_ID_INVENCLOSE	 ].ChangeRect(m_rcWnd.left+242, m_rcWnd.top+423);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLUP	 ].ChangeRect(m_rcWnd.left+247, m_rcWnd.top+ 20);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLDOWN].ChangeRect(m_rcWnd.left+247, m_rcWnd.top+167);

    const bool close=m_xInventoryBtn[_BTN_ID_INVENCLOSE].OnLButtonUp(ptMouse) && m_closePressed;
    m_closePressed=false;
    if (close) { g_xSound.PlayLobbyEffect("105.wav"); return TRUE; }

	// 滚动 Up, Down 处理.
//	m_xInventoryBtn[_BTN_ID_INVENSCRLUP	 ].OnLButtonUp(ptMouse);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLDOWN].OnLButtonUp(ptMouse);

	nInventoryNum	  = GetInvenNum(ptMouse);
	nBeltInventoryNum = GetBeltNum(ptMouse);

	// 通用(鼠标)物品栏中有来自装备窗口的物品，且不在等待服务器响应信号的状态时，关闭物品穿戴.
	if ( pstCommonItemSet->bSetted && pstCommonItemSet->bIsEquipItem && !pstCommonItemSet->bWaitResponse )
	{
		nInventoryNum = GetInvenNum(ptMouse);
		if ( nInventoryNum != -1 )
		{
			g_xClientSocket.SendTakeOnOffItem(CM_TAKEOFFITEM, pstCommonItemSet->xItem.m_shCellNum, pstCommonItemSet->xItem.m_stItemInfo.stStdItem.szName, pstCommonItemSet->xItem.m_stItemInfo.nMakeIndex);
			pstCommonItemSet->bWaitResponse	= TRUE;
			return FALSE;
		}
	}

	// 在背包格子区域内点击的情况.
	if ( nInventoryNum != -1 )
	{
		// 点击的背包中有物品时。
		if ( m_stInventoryItem[nInventoryNum].bSetted )
		{
			// 如果公共(鼠标)物品集为空，
			if ( !pstCommonItemSet->bSetted )
			{				  
				// 把物品附着在鼠标上.
				SetCommonFromInven(nInventoryNum, pstCommonItemSet);
			}
			// 存在公共(鼠标)物品集, 不在等待服务器应答信号的过程中, 且不是已穿戴物品时.
			else if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
			{				  
				ChangeInvenWithCommon(nInventoryNum, pstCommonItemSet, &ptMouse);
			}
		}
		// 点击的背包中没有物品时。
		else
		{
			// 存在公共(鼠标)物品集，且不在等待服务器响应信号时，
			if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
			{				  
				SetInvenFromCommon(nInventoryNum, pstCommonItemSet, &ptMouse);
			}
		}
	}

	if ( nBeltInventoryNum != -1 )
	{
		if ( m_stBeltItem[nBeltInventoryNum].bSetted )
		{
			if ( !pstCommonItemSet->bSetted )
			{				  
				SetCommonFromBelt(nBeltInventoryNum, pstCommonItemSet);
			}
			else if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
			{		
				if ( /*pstCommonItemSet->xItem.m_stItemInfo.szMakeIndex[0] == 'G' && */
					 (pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode == 0 || pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode == 3) )
					ChangeBeltWithCommon(nBeltInventoryNum, pstCommonItemSet);
			}
		}
		else
		{
			if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
			{				  
				if ( /*pstCommonItemSet->xItem.m_stItemInfo.szMakeIndex[0] == 'G' && */
					 (pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode == 0 || pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode == 3) )
					 SetBeltFromCommon(nBeltInventoryNum, pstCommonItemSet);
			}
		}
	}

	return FALSE;
}



BOOL CInventoryWnd::OnLButtonDoubleClick(LPCOMMONITEMSET pstCommonItemSet, POINT ptMouse)
{	
	INT nInventoryNum	  = GetInvenNum(ptMouse);
	INT nBeltInventoryNum = GetBeltNum(ptMouse);

	if ( nInventoryNum != -1 )
	{
		// 点击的背包中有物品时。
		if ( m_stInventoryItem[nInventoryNum].bSetted )
		{
			// 如果公共(鼠标)物品集为空，
			if ( !pstCommonItemSet->bSetted )
			{				  
				// 把物品附着在鼠标上.
				SetCommonFromInven(nInventoryNum, pstCommonItemSet);
			}
			// 存在公共(鼠标)物品集，且不在等待服务器响应信号时，
			else if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
			{				  
				ChangeInvenWithCommon(nInventoryNum, pstCommonItemSet, &ptMouse);
			}
		}
	}

	if ( nBeltInventoryNum != -1 )
	{
		if ( m_stBeltItem[nBeltInventoryNum].bSetted )
		{
			if ( !pstCommonItemSet->bSetted )
			{				  
				SetCommonFromBelt(nBeltInventoryNum, pstCommonItemSet);
			}
			else if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
			{				  
				ChangeBeltWithCommon(nBeltInventoryNum, pstCommonItemSet);
			} 
		}
	}

	if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
	{			
		pstCommonItemSet->bWaitResponse = TRUE;
		g_xClientSocket.SendItemIndex(CM_EAT, pstCommonItemSet->xItem.m_stItemInfo.nMakeIndex, pstCommonItemSet->xItem.m_stItemInfo.stStdItem.szName);
	}

	return TRUE;
}



VOID CInventoryWnd::OnMouseMove(POINT ptMouse)
{
    if (ScrollInput(1,ptMouse)) return;
	m_xInventoryBtn[_BTN_ID_INVENCLOSE	 ].ChangeRect(m_rcWnd.left+242, m_rcWnd.top+423);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLUP	 ].ChangeRect(m_rcWnd.left+247, m_rcWnd.top+ 20);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLDOWN].ChangeRect(m_rcWnd.left+247, m_rcWnd.top+167);

	for( INT nCnt = 0; nCnt < _MAX_INVEN_BTN; nCnt++ )
		m_xInventoryBtn[nCnt].OnMouseMove(ptMouse);
}



VOID CInventoryWnd::OnScrollDown()
{
	if ( m_nStartLineNum > 0 )												m_nStartLineNum--;
}



VOID CInventoryWnd::OnScrollUp()
{
//	if ( m_nStartLineNum < _INVEN_MAX_CELL_YCNT - _INVEN_CELL_YCNT )		m_nStartLineNum++;
	if ( m_nStartLineNum < (_INVEN_MAX_CELL_YCNT-_INVEN_CELL_YCNT) )
	{
		m_nStartLineNum++;
	}
}

// Preserve the original draw divisor 91 and input multiplier 92.
bool CInventoryWnd::ScrollInput(int kind, POINT point)
{
    const DWORD now=GetTickCount();
    if (kind==2) {
        m_scrollTick=now;
        const bool captured=m_scrollCaptured; m_scrollCaptured=false;
        if (captured && GetCapture()==g_xMainWnd.GetSafehWnd()) ReleaseCapture();
        return captured;
    }
    if (!m_pxWndImage->NewSetIndex(1225)) return false;
    const auto& image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
    const int left=m_rcWnd.left+250,top=m_rcWnd.top+70,bottom=top+image.shHeight+254;
    if (kind==1) {
        if (!m_scrollCaptured) return false;
        const float ratio=max(0.f,min(1.f,float(point.y-top)/(bottom-top)));
        m_nStartLineNum=int(ratio*92.f);
        return true;
    }
    if (point.x<left || point.x>=left+image.shWidth || point.y<top || point.y>=bottom) return false;
    if (now-m_scrollTick<=10) return true;
    m_scrollTick=now;
    const int thumb=top+int(254.0*(float(m_nStartLineNum)/91));
    if (point.y>=thumb && point.y<thumb+image.shHeight) {
        m_scrollCaptured=true; SetCapture(g_xMainWnd.GetSafehWnd()); return true;
    }
    float ratio=float(m_nStartLineNum)/91;
    ratio=max(0.f,min(1.f,ratio+(point.y<thumb+7 ? -8.f/92 : 8.f/92)));
    m_nStartLineNum=int(ratio*92.f);
    return true;
}
