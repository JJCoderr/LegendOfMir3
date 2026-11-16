// 最大按钮数

#include "stdafx.h"
#define MAX_TXT_BUTTON_COUNT	5

/////////////////////////////////////////////////////////////////////////
//	CTextButton : 构造函数
/////////////////////////////////////////////////////////////////////////
CTextButton::CTextButton()
{
	m_pTxtBtn.ClearAllNodes();
	m_nTxtHeight = 0;
	m_nMaxLine = 0;
	m_bIsReady = FALSE;
	m_IsUseSubTxt = FALSE;
}

/////////////////////////////////////////////////////////////////////////
//	~CTextButton : 析构函数
/////////////////////////////////////////////////////////////////////////
CTextButton::~CTextButton()
{
	ReleaseButtons();
}

/////////////////////////////////////////////////////////////////////////
//	AddButton : 添加 Text 按钮.
/////////////////////////////////////////////////////////////////////////
// pszTxt: 屏幕上显示的 Text
// xRect: 被识别为按钮的 Rect 区域
// nLinePos: 绘制位置的 Line Count
// pszParam: 被点击时使用的 Parameter 值
/////////////////////////////////////////////////////////////////////////
BOOL	CTextButton::AddButton(CHAR *pszTxt,RECT xRect,INT nLinePos,CHAR *pszParam,COLORREF nColor,COLORREF cColor)
{
	if(m_bIsReady)
	{
		SIZE tSize;
		CMTxtBtn* tCMTxtBtn;
		tCMTxtBtn = new  CMTxtBtn;
		tSize = g_xMainWnd.GetStrLength(NULL,NULL,pszTxt);
		m_nTxtHeight  = tSize.cy;
		strcpy(tCMTxtBtn->Txt,pszTxt);
		tCMTxtBtn->xRect = xRect;
		tCMTxtBtn->nLinePos = nLinePos;
		strcpy(tCMTxtBtn->Param,pszParam);
		tCMTxtBtn->bIsClicked = FALSE;
		tCMTxtBtn->nColor = nColor;
		tCMTxtBtn->cColor = cColor;
		m_pTxtBtn.AddNode(tCMTxtBtn);
		return TRUE;
	}
	else 
		return FALSE;
}

BOOL	CTextButton::AddButton(CHAR *pszTxt,CHAR *pszTxt2,RECT xRect,INT nLinePos,CHAR *pszParam,COLORREF nColor,COLORREF cColor)
{
	if(m_bIsReady)
	{
		SIZE tSize;
		CMTxtBtn* tCMTxtBtn;
		tCMTxtBtn = new  CMTxtBtn;
		tSize = g_xMainWnd.GetStrLength(NULL,NULL,pszTxt);
		m_nTxtHeight  = tSize.cy;

		strcpy(tCMTxtBtn->Txt,pszTxt);
		strcpy(tCMTxtBtn->sTxt,pszTxt2);

		tCMTxtBtn->xRect = xRect;
		tCMTxtBtn->nLinePos = nLinePos;

		strcpy(tCMTxtBtn->Param,pszParam);
		tCMTxtBtn->bIsClicked = FALSE;
		tCMTxtBtn->nColor = nColor;
		tCMTxtBtn->cColor = cColor;

		m_pTxtBtn.AddNode(tCMTxtBtn);
		m_IsUseSubTxt = TRUE;
		return TRUE;
	}
	else 
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////
//	ClickCheck : 进行由 Button Click 引起的 Click Checking。
/////////////////////////////////////////////////////////////////////////
// xPos: 要获取实际鼠标坐标
// xWndPos: 将取得实际 Text 开始写入位置的坐标
// CurrentTopLine: 当前 Window 上显示的最上一行
/////////////////////////////////////////////////////////////////////////
CMTxtBtn*	CTextButton::ClickCheck(POINT xWndPos,POINT	xPos,INT CurrentTopLine)
{
	if(m_bIsReady)
	{
		m_pTxtBtn.MoveCurrentToTop();
		for(int i = 0; i<m_pTxtBtn.ListLength();i++)
		{
			if((m_pTxtBtn.GetCurrentData()->nLinePos >= CurrentTopLine) && (m_pTxtBtn.GetCurrentData()->nLinePos <= CurrentTopLine+m_nMaxLine))
			{
				if(IsCurrsorOnThisButton(m_pTxtBtn.GetCurrentData(),xWndPos,xPos,CurrentTopLine))
				{
					m_pTxtBtn.GetCurrentData()->bIsClicked = !m_pTxtBtn.GetCurrentData()->bIsClicked;
					return m_pTxtBtn.GetCurrentData();
				}
			}
			m_pTxtBtn.MoveNextNode();
		}
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////
//	ReleaseButtons : 丢弃当前已捕获的所有按钮.
/////////////////////////////////////////////////////////////////////////
BOOL CTextButton::ReleaseButtons(VOID)
{
	if(m_bIsReady)
	{
		m_pTxtBtn.ClearAllNodes();
//		m_bIsReady = FALSE;
		m_nTxtHeight = 0;
		return TRUE;
	}
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////////////
//	InitTxtBtn : 为使本类可以使用而进行准备。
/////////////////////////////////////////////////////////////////////////
// nTextHeight: 实际 Text 的 Height  
// WndRect: 窗口的大小  
// nMaxLine: 可输出的最大行数
/////////////////////////////////////////////////////////////////////////
BOOL CTextButton::InitTxtBtn(INT nMaxLine,POINT StartPos,INT nLineGap)
{
	m_bIsReady = TRUE;
	m_nMaxLine = nMaxLine;
	m_xStartPos = StartPos;
	m_nLineGap = nLineGap;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////
//	GetButton : 获取该 Button 的指针。
/////////////////////////////////////////////////////////////////////////
// nIndex : Index
/////////////////////////////////////////////////////////////////////////
CMTxtBtn* CTextButton::GetButton(INT nIndex)
{
	if(nIndex<0 || nIndex>m_pTxtBtn.GetCounter()) return NULL;
	m_pTxtBtn.MoveCurrentToTop();
	m_pTxtBtn.MoveNode(nIndex);
	return m_pTxtBtn.GetCurrentData();
}

/////////////////////////////////////////////////////////////////////////
//	GetButtonCount : 获取 Button 的数量.
/////////////////////////////////////////////////////////////////////////
INT CTextButton::GetButtonCount(VOID)
{
	return m_pTxtBtn.GetCounter();
}

VOID CTextButton::ClickFreeAllBtn(VOID)
{
	m_pTxtBtn.MoveCurrentToTop();
	for(int i = 0 ; i < m_pTxtBtn.GetCounter() ; i++)
	{
		m_pTxtBtn.GetCurrentData()->bIsClicked = FALSE;
		m_pTxtBtn.MoveNextNode();
	}
}

/////////////////////////////////////////////////////////////////////////
//	ClickCheck : 进行由 Button Click 引起的 Click Checking。
/////////////////////////////////////////////////////////////////////////
// xPos: 要获取实际鼠标坐标
// xWndPos: 将取得实际 Text 开始写入位置的坐标
// CurrentTopLine: 当前 Window 上显示的最上一行
/////////////////////////////////////////////////////////////////////////


CMTxtBtn* CTextButton::ButtonDownCheck(POINT xWndPos,POINT xPos,INT CurrentTopLine)
{
///	CMTxtBtn*	xResult=NULL;
/*	int			nTempX,nTempY;
	int			StdPosX,StdPosY;	// 在移动的窗口内计算基准点。*/
	
	if(m_bIsReady)
	{
		m_pTxtBtn.MoveCurrentToTop();
		for(int i = 0; i<m_pTxtBtn.ListLength();i++)
		{
			if((m_pTxtBtn.GetCurrentData()->nLinePos >= CurrentTopLine) && (m_pTxtBtn.GetCurrentData()->nLinePos <= CurrentTopLine+m_nMaxLine))
			{
				if(IsCurrsorOnThisButton(m_pTxtBtn.GetCurrentData(),xWndPos,xPos,CurrentTopLine))
				{
					m_pTxtBtn.GetCurrentData()->bIsClicked = TRUE; 
					return m_pTxtBtn.GetCurrentData();
				}
				m_pTxtBtn.GetCurrentData()->bIsClicked = FALSE;
			}
			m_pTxtBtn.MoveNextNode();
		}
	}
	return NULL;
}


CMTxtBtn* CTextButton::ButtonUpCheck(POINT xWndPos,POINT xPos,INT CurrentTopLine)
{	
	if(m_bIsReady)
	{
		m_pTxtBtn.MoveCurrentToTop();
		for(int i = 0; i<m_pTxtBtn.ListLength();i++)
		{
			if((m_pTxtBtn.GetCurrentData()->nLinePos >= CurrentTopLine) && (m_pTxtBtn.GetCurrentData()->nLinePos <= CurrentTopLine+m_nMaxLine))
			{
				if(IsCurrsorOnThisButton(m_pTxtBtn.GetCurrentData(),xWndPos,xPos,CurrentTopLine))
				{
					m_pTxtBtn.GetCurrentData()->bIsClicked = FALSE;   
					return m_pTxtBtn.GetCurrentData();
				}
			}
			m_pTxtBtn.MoveNextNode();
		}
	}
	return NULL;
}



BOOL CTextButton::IsCurrsorOnThisButton(CMTxtBtn* pBtn,POINT xWndPos,POINT xPos,INT CurrentTopLine)
{
	int			nTempX,nTempY;
	int			StdPosX,StdPosY;																			// 在移动的窗口内计算基准点.
	
	StdPosY = xWndPos.y + (pBtn->nLinePos - CurrentTopLine)*(m_nTxtHeight+m_nLineGap)+m_xStartPos.y;		// 获取可变位置处的 Top。
	StdPosX = xWndPos.x + pBtn->xRect.left+m_xStartPos.x;													// 求可变位置上的 Left.
	nTempX = StdPosX	+ pBtn->xRect.right;
	nTempY = StdPosY	+ pBtn->xRect.bottom;
			
	if((xPos.x>StdPosX)&&(xPos.x<nTempX))
	{
		if((xPos.y>StdPosY)&&(xPos.y<nTempY))
		{
			return TRUE;
		}
	}
	return FALSE;
}