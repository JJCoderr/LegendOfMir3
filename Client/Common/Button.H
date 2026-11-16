#ifndef _CTEXTBUTTON_H
#define _CTEXTBUTTON_H

class	CTextButton
{
public:
	VOID ClickFreeAllBtn(VOID);
	INT GetButtonCount(VOID);
	CMTxtBtn* GetButton(INT nIndex);
	CTextButton();												// Constructor
	virtual	~CTextButton();										// Destructor
	BOOL	AddButton(CHAR *pszTxt,RECT xRect,INT nLinePos,CHAR *pszParam,COLORREF nColor,COLORREF cColor);	// Add Button to class
	BOOL	AddButton(CHAR *pszTxt,CHAR *pszTxt2,RECT xRect,INT nLinePos,CHAR *pszParam,COLORREF nColor,COLORREF cColor);	// Add Button to class

	CMTxtBtn*	ClickCheck(POINT xWndPos,POINT	xPos,INT CurrentTopLine);	// xWndPos: 实际 Text 绘制的最后端点
	CMTxtBtn*	ButtonDownCheck(POINT xWndPos,POINT xPos,INT CurrentTopLine);
	CMTxtBtn*	ButtonUpCheck(POINT xWndPos,POINT xPos,INT CurrentTopLine);

	BOOL		IsCurrsorOnThisButton(CMTxtBtn* pBtn,POINT xWndPos,POINT xPos,INT CurrentTopLine);

	BOOL InitTxtBtn(INT nMaxLine,POINT nStartPos,INT LineGap);
//	BOOL InitTxtBtn(INT nMaxLine);	
	CPDLList<CMTxtBtn> m_pTxtBtn; 
	BOOL		ReleaseButtons(VOID);	
private:
	BOOL		m_bIsReady;
	RECT		m_rtWndRect;			// 窗口的 Rect，仅文字显示的部分
	INT			m_nTxtHeight;			// 每个 Text 的 Height
	INT			m_nMaxLine;				// 窗口中显示的最大 Line 数
	INT			m_nLineGap;
	POINT		m_xStartPos;
	BOOL		m_IsUseSubTxt;
};

#endif _CTEXTBUTTON_H