/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/



#include "StdAfx.h"



/******************************************************************************************************************

	CInventoryWnd Class

*******************************************************************************************************************/
/******************************************************************************************************************

	函数名 : CInventoryWnd::CInventoryWnd()

	编写者 :
	编写日期 :

	目的   :
	输出   :

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
CInventoryWnd::CInventoryWnd()
{
	Init();
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::~CInventoryWnd()

	作者 : 
	编写日期 : 

	目的   : 
	输出   : 

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
CInventoryWnd::~CInventoryWnd()
{
	Destroy();
}


VOID CInventoryWnd::Init()
{
	INT nCnt;
	CGameWnd::Init();
	m_nStartLineNum	= 0;
	ZeroMemory(m_rcInvenCell, sizeof(RECT) *_INVEN_MAX_CELL);
	ZeroMemory(m_rcBeltCell,  sizeof(RECT) *_BELT_MAX_CELL );
	ZeroMemory(m_stInventoryItem, sizeof(ITEMSET)*_MAX_INVEN_ITEM);
	ZeroMemory(m_stBeltItem,	  sizeof(ITEMSET)*_MAX_BELT_ITEM );

	m_xInvenScrlBar.Init();

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


/******************************************************************************************************************

	函数名 : CInventoryWnd::CreateInventoryWnd()

	作者 :
	编写日期 :

	目的   :
	输入   : INT nID
	         CWHWilImageData* pxWndImage
	         INT nFrameImgIdx
	         INT nStartX
	         INT nStartY
	         INT nWidth
	         INT nHeight
	         BOOL bCanMove
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::CreateInventoryWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove)
{
	// 窗口创建.
	CreateGameWnd(nID, pxWndImage, nFrameImgIdx, bCanMove, nStartX, nStartY, nWidth, nHeight);

	// 按钮创建.
	m_xInventoryBtn[_BTN_ID_INVENCLOSE	 ].CreateGameBtn(pxWndImage, 280, 281, nStartX+255, nStartY+291);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLUP	 ].CreateGameBtn(pxWndImage, 74, 75, nStartX+247, nStartY+ 20);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLDOWN].CreateGameBtn(pxWndImage, 74, 75, nStartX+247, nStartY+167);
	
	m_xInvenScrlBar.CreateScrlBar(pxWndImage, 270, _INVEN_MAX_CELL_YCNT, 10, 218, 10);

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



/******************************************************************************************************************

	函数名 : CInventoryWnd::SetInvenBtnInit()

	作者 :
	编写日期 :

	目的   :
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::SetInvenBtnInit()
{
	for( INT nCnt = 0; nCnt < _MAX_INVEN_BTN; nCnt++ )
	{
		m_xInventoryBtn[nCnt].SetBtnState(_BTN_STATE_NORMAL);
	}
}


/******************************************************************************************************************

	函数名 : CInventoryWnd::ShowInventoryWnd()

	作者 :
	编写日期 :

	目的   :
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::ShowInventoryWnd()
{
	INT nCnt;

	// 窗口背景绘制.
	ShowGameWnd();


	RECT rcTitle = {m_rcWnd.left+31, m_rcWnd.top+27, m_rcWnd.left+133, m_rcWnd.top+43};

	g_xMainWnd.PutsHan(NULL, rcTitle, RGB(200, 200, 255), RGB(0, 0, 0), "[Inventory]", g_xMainWnd.CreateGameFont("Arial", 10, 0, FW_BOLD));

	// 滚动条.
	m_xInvenScrlBar.ShowScrlBar(m_rcWnd.left+254, m_rcWnd.top-165, m_nStartLineNum, 12);
	
	// 绘制物品栏物品.
	for ( nCnt = 0; nCnt < _MAX_INVEN_ITEM; nCnt++ )
	{
		if ( m_stInventoryItem[nCnt].bSetted )
		{
			INT nStartX, nStartY;

			if ( nCnt >= m_nStartLineNum*_INVEN_CELL_XCNT && nCnt < (m_nStartLineNum+_INVEN_CELL_YCNT)*_INVEN_CELL_XCNT ) 
			{
				nStartX = m_rcWnd.left+m_rcInvenCell[nCnt-(m_nStartLineNum*_INVEN_CELL_XCNT)].left;
				nStartY = m_rcWnd.top +m_rcInvenCell[nCnt-(m_nStartLineNum*_INVEN_CELL_XCNT)].top;

//				m_stInventoryItem[nCnt].xItem.m_stItemInfo.stStdItem.wLooks = 4;
				m_stInventoryItem[nCnt].xItem.DrawItem(nStartX, nStartY);

//				RECT rc = {nStartX, nStartY, nStartX+_INVENTORY_CELL_WIDTH, nStartY+_INVENTORY_CELL_HEIGHT};
//				g_xMainWnd.DrawWithGDI(rc, NULL, RGB(255, 0, 255), 1);
			}
		}
	}

	// 腰带物品绘制。
	for ( nCnt = 0; nCnt < _MAX_BELT_ITEM; nCnt++ )
	{
		if ( m_stBeltItem[nCnt].bSetted )
		{
			INT nStartX, nStartY;

			nStartX = m_rcWnd.left+m_rcBeltCell[nCnt].left;
			nStartY = m_rcWnd.top+m_rcBeltCell[nCnt].top;

			m_stBeltItem[nCnt].xItem.DrawItem(nStartX, nStartY);
		}
	}

	// 绘制背包窗口按钮.
	for ( nCnt = 0; nCnt < _MAX_INVEN_BTN; nCnt++ )
	{
		m_xInventoryBtn[nCnt].ShowGameBtn();
	}

	// 显示物品的状态值。
	ShowInvenItemState();	
}




/******************************************************************************************************************

	函数名 : CInventoryWnd::ShowInvenItemState()

	作者 :
	编写日期 :

	目的   :
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::ShowInvenItemState()
{
/*	INT nInventoryNum = GetInvenNum(g_xGameProc.m_ptMousePos);
	if ( nInventoryNum != -1 )
	{
		if ( m_stInventoryItem[nInventoryNum].bSetted )
		{
			m_stInventoryItem[nInventoryNum].xItem.ShowItemStatus(m_rcWnd.left+95, m_rcWnd.top+250);
		}
	}
*/
	INT nInventoryNum = GetInvenNum(g_xGameProc.m_ptMousePos);
	if ( nInventoryNum != -1 )
	{
		if ( m_stInventoryItem[nInventoryNum].bSetted )
		{
			INT nStartX, nStartY;

			if ( nInventoryNum >= m_nStartLineNum*_INVEN_CELL_XCNT && nInventoryNum < (m_nStartLineNum+_INVEN_CELL_YCNT)*_INVEN_CELL_XCNT ) 
			{
				nStartX = m_rcWnd.left+m_rcInvenCell[nInventoryNum-(m_nStartLineNum*_INVEN_CELL_XCNT)].left;
				nStartY = m_rcWnd.top +m_rcInvenCell[nInventoryNum-(m_nStartLineNum*_INVEN_CELL_XCNT)].top;

				m_stInventoryItem[nInventoryNum].xItem.ShowItemStatus(nStartX+15, nStartY+15);
			}
		}
	}

	INT nBeltInventoryNum = GetBeltNum(g_xGameProc.m_ptMousePos);
	if ( nBeltInventoryNum != -1 )
	{
		if ( m_stBeltItem[nBeltInventoryNum].bSetted )
		{
			m_stBeltItem[nBeltInventoryNum].xItem.ShowItemStatus(m_rcWnd.left+95, m_rcWnd.top+250);
		}		
	}
}














/******************************************************************************************************************

	物品放入与取出相关, 背包位置的保存文件存盘及加载.(背包及腰带)

*******************************************************************************************************************/
/******************************************************************************************************************

	函数名 : CInventoryWnd::CheckItemPos()

	作者 :
	编写日期 :

	目的   :
	输入   : CHAR* szName
	输出   : BOOL

	[日期][修改者] : 修改内容

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

		CloseHandle(hFile);
		return TRUE;
	}

	return FALSE;
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::SaveItemPos()

	作者 :
	编写日期 :

	目的   :
	输入   : CHAR* szName
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
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
INT CInventoryWnd::GetInvenNum(POINT ptMouse)
{
	RECT	rc;
	for ( INT nCnt = 0; nCnt < _INVEN_MAX_CELL; nCnt++ )
	{
		SetRect(&rc, m_rcWnd.left+m_rcInvenCell[nCnt].left, m_rcWnd.top+m_rcInvenCell[nCnt].top, 
			    m_rcWnd.left+m_rcInvenCell[nCnt].left+_INVENTORY_CELL_WIDTH, m_rcWnd.top+m_rcInvenCell[nCnt].top+_INVENTORY_CELL_HEIGHT);
		if ( PtInRect(&rc, ptMouse) )
		{
			return (nCnt+m_nStartLineNum*_INVEN_CELL_XCNT);
		}
	}

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
	RECT	rc;
	for ( INT nCnt = 0; nCnt < _MAX_BELT_ITEM; nCnt++ )
	{
		SetRect(&rc, m_rcWnd.left+m_rcBeltCell[nCnt].left, m_rcWnd.top+m_rcBeltCell[nCnt].top, 
			    m_rcWnd.left+m_rcBeltCell[nCnt].left+_INVENTORY_CELL_WIDTH, m_rcWnd.top+m_rcBeltCell[nCnt].top+_INVENTORY_CELL_HEIGHT);
		if ( PtInRect(&rc, ptMouse) )
		{
			return (nCnt);
		}
	}

	return -1;
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
BOOL CInventoryWnd::AddInvenItem(CItem xItem, INT nInvenNum, BOOL bUseInvenNum)
{
	if ( bUseInvenNum )
	{
		if ( !m_stInventoryItem[nInvenNum].bSetted )
		{
			m_stInventoryItem[nInvenNum].bSetted = TRUE;
			m_stInventoryItem[nInvenNum].xItem = xItem;

			return TRUE;
		}
	}

	for ( INT nCnt = 0; nCnt < _MAX_INVEN_ITEM; nCnt++ )
	{
		if ( !m_stInventoryItem[nCnt].bSetted )
		{
			m_stInventoryItem[nCnt].bSetted = TRUE;
			m_stInventoryItem[nCnt].xItem = xItem;

			return TRUE;
		}
	}

	return FALSE;
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
VOID CInventoryWnd::ChangeInvenWithCommon(INT nInventoryNum, LPCOMMONITEMSET pstCommonItemSet)
{
	if ( pstCommonItemSet->bSetted )
	{
		CItem xInvenItem;
		if ( ExtractInvenItem(&xInvenItem, nInventoryNum) )
		{
			AddInvenItem(pstCommonItemSet->xItem, nInventoryNum, TRUE);

			pstCommonItemSet->bSetted			= TRUE;
			pstCommonItemSet->xItem				= xInvenItem;
			pstCommonItemSet->bWaitResponse		= FALSE;
			pstCommonItemSet->bIsEquipItem		= FALSE;
			pstCommonItemSet->bIsHideItem		= FALSE;
			pstCommonItemSet->bIsBeltItem		= FALSE;
			pstCommonItemSet->xItem.m_shCellNum = nInventoryNum; 
		}
	}
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
VOID CInventoryWnd::SetInvenFromCommon(INT nInventoryNum, LPCOMMONITEMSET pstCommonItemSet)
{
	if ( pstCommonItemSet->bSetted )
	{
		pstCommonItemSet->xItem.m_shCellNum = nInventoryNum;
		AddInvenItem(pstCommonItemSet->xItem, nInventoryNum, TRUE);

		ZeroMemory(pstCommonItemSet, sizeof(COMMONITEMSET));
	}
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


/******************************************************************************************************************

	函数名 : CInventoryWnd::OnLButtonDown()

	作者 : 
	日期 : 

	目的   : 
	输入   : POINT ptMouse
	输出   : BOOL 

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CInventoryWnd::OnLButtonDown(LPCOMMONITEMSET pstCommonItemSet, POINT ptMouse)
{
	INT nCnt, nInventoryNum, nBeltInventoryNum;

	m_xInventoryBtn[_BTN_ID_INVENCLOSE	 ].ChangeRect(m_rcWnd.left+255, m_rcWnd.top+291);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLUP	 ].ChangeRect(m_rcWnd.left+247, m_rcWnd.top+ 20);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLDOWN].ChangeRect(m_rcWnd.left+247, m_rcWnd.top+167);


	if ( m_xInvenScrlBar.OnLButtonDown(ptMouse) )
	{
		FLOAT	fScrlRate;

		fScrlRate	= m_xInvenScrlBar.GetScrlRate();
		m_nStartLineNum = 11*fScrlRate;

		return FALSE;
	}


	for( nCnt = 0; nCnt < _MAX_INVEN_BTN; nCnt++ )
	{
		if ( m_xInventoryBtn[nCnt].OnLButtonDown(ptMouse) )		
			return TRUE;
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



/******************************************************************************************************************

	函数名 : CInventoryWnd::OnLButtonUp()

	作者 :
	编写日期 :

	目的   :
	输入   : LPCOMMONITEMSET pstCommonItemSet
	         POINT ptMouse
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CInventoryWnd::OnLButtonUp(LPCOMMONITEMSET pstCommonItemSet, POINT ptMouse)
{
	INT nInventoryNum, nBeltInventoryNum;

	m_xInventoryBtn[_BTN_ID_INVENCLOSE	 ].ChangeRect(m_rcWnd.left+255, m_rcWnd.top+291);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLUP	 ].ChangeRect(m_rcWnd.left+247, m_rcWnd.top+ 20);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLDOWN].ChangeRect(m_rcWnd.left+247, m_rcWnd.top+167);

	if ( m_xInvenScrlBar.OnLButtonUp(ptMouse) )
	{
		return TRUE;
	}

	// 关闭背包窗口.
	if ( m_xInventoryBtn[_BTN_ID_INVENCLOSE].OnLButtonUp(ptMouse) )
		return TRUE;

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
				ChangeInvenWithCommon(nInventoryNum, pstCommonItemSet);
			}
		}
		// 点击的背包中没有物品时。
		else
		{
			// 存在公共(鼠标)物品集，且不在等待服务器响应信号时，
			if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
			{				  
				SetInvenFromCommon(nInventoryNum, pstCommonItemSet);
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



/******************************************************************************************************************

	函数名 : CInventoryWnd::OnLButtonDoubleClick()

	作者 :
	编写日期 :

	目的   :
	输入   : LPCOMMONITEMSET pstCommonItemSet
	         POINT ptMouse
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
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
				ChangeInvenWithCommon(nInventoryNum, pstCommonItemSet);
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



/******************************************************************************************************************

	函数名 : CInventoryWnd::OnMouseMove()

	作者 :
	编写日期 :

	目的   :
	输入   : POINT ptMouse
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::OnMouseMove(POINT ptMouse)
{
	m_xInventoryBtn[_BTN_ID_INVENCLOSE	 ].ChangeRect(m_rcWnd.left+255, m_rcWnd.top+291);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLUP	 ].ChangeRect(m_rcWnd.left+247, m_rcWnd.top+ 20);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLDOWN].ChangeRect(m_rcWnd.left+247, m_rcWnd.top+167);

	for( INT nCnt = 0; nCnt < _MAX_INVEN_BTN; nCnt++ )
		m_xInventoryBtn[nCnt].OnMouseMove(ptMouse);
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::OnScrollDown()

	作者 :
	编写日期 :

	目的   :
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::OnScrollDown()
{
	if ( m_nStartLineNum > 0 )												m_nStartLineNum--;
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::OnScrollUp()

	作者 :
	编写日期 :

	目的   :
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::OnScrollUp()
{
//	if ( m_nStartLineNum < _INVEN_MAX_CELL_YCNT - _INVEN_CELL_YCNT )		m_nStartLineNum++;
	if ( m_nStartLineNum < (_INVEN_MAX_CELL_YCNT-1) )
	{
		m_nStartLineNum++;
	}
}
