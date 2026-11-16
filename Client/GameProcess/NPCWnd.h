#ifndef _CNPCWnd_H
#define _CNPCWnd_H

/*using namespace std;

*/
class CNPCWnd : public CGameWnd
{ 
public:
	INT						m_nNpcId;

protected:
	CWHWilImageData			m_xNPCImage;
	int						m_nStartLine;
	int						m_nMaxLine;

public:
	CNPCWnd();
	~CNPCWnd();
	VOID	CreateNPCChatWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove);
	VOID	ShowNPCChatWnd();

	__inline void	SetNPC(int nNPCID) { m_nNpcId = nNPCID; }
	void	ResetDialog();

public:
	VOID SetNpcEditBoxPos(VOID);
	VOID SetWndRectReduce(VOID);
	VOID SetWndRectExtend(VOID);
//  消息函数.
	BOOL	OnLButtonDown(POINT ptMouse);
	BOOL	OnLButtonUp(POINT ptMouse);
	VOID	OnMouseMove(POINT ptMouse);
	BOOL	OnMsgInputted(VOID);

	VOID	OnScrollDown();
	VOID	OnScrollUp();
	VOID	OnEnterKeyDown(VOID);

private:
	// Command Click 处理
	BOOL	ClickCommandProcess(CMTxtBtn* ClickedBtn);		// 综合下面这些内容
	BOOL	SendCommandToSrv(CHAR* szMsg);
	INT		m_nNpcMsgState;
	CMTxtBtn* m_pClickedTxtBtn;
	CMirMsgBox	m_xMsgBox;
/*	ClickPepairCommand(VOID);		// 修理(@REPAIR)
	ClickSellCommand(VOID);			// 出售(@SELL)
	ClickBuyCommand(VOID);			// 购买(@BUY)
	ClickBuildGuildCommand(VOID);	// 行会创建(@@BUILDGUILDNOW)
	ClickSpecialRepairCommand(VOID);// 特殊修理(@SPECIALREPAIR)
	ClickMakeCommand(VOID);			// 制作(@MAKE)
*/
};

#endif // _CNPCWnd_H