#ifndef _CCHATPOPWND_H
#define _CCHATPOPWND_H

typedef struct tagCHATSTRING
{
	DWORD	dwFontColor;
	DWORD	dwFontBackColor;
	string	strChat;
}CHATSTRING, *LPCHATSTRING;


class CChatPopWnd : public CGameWnd
{ 
private:
protected:
	CDLList<CHATSTRING> m_xstrDividedChat;		// 分割后的聊天数据。
	
	CGameBtn		m_xChatPopBtn;
	RECT			m_rcChatPopFrame;			// 聊天区域.
	INT				m_nCurrStartChatLine;		// 聊天画面最上面一行所指向的聊天列表的编号索引。
	INT				m_nCanScrlCnt;

public:
	RECT			m_rcEditBoxFrame;			

public:
	CChatPopWnd();
	~CChatPopWnd();

	virtual VOID Init();
	virtual VOID Destroy();

	BOOL	MsgAdd(DWORD dwFontColor, DWORD dwFontBackColor, CHAR* szMsg = NULL);

	VOID	CreateChatPopWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove);
	VOID	ShowChatPopWnd();
	VOID	SetStatusBtnInit();
public:
//  消息函数.
	BOOL	OnLButtonDown(POINT ptMouse);
	BOOL	OnLButtonUp(POINT ptMouse);
	VOID	OnMouseMove(POINT ptMouse);

	VOID	OnScrollDown();
	VOID	OnScrollUp();
};


#endif // _CCHATPOPWND_H
