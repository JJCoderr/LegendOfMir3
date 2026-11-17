#include <vector>
#include <string>
#ifndef _INTERFACE_H
#define _INTERFACE_H



#include "Common/LoginNotice.h"

class CInterface
{ 
public:
    std::string previousHint;
    int hintHeight=6;
    bool hintExpanding=true;
    int ButtonHintAt(POINT point,POINT& anchor);
    void ShowButtonHint(POINT point);
    bool m_primaryStat[2] = {true,true};
    bool m_chatExpanded = false;
    int m_chatMode = 0;
    bool m_chatModeChanged = false;
    bool m_chatRejected[3] = {};
    int m_chatReceptionHint[3] = {2637,2639,2643};
    int m_chatReceptionFrame[3] = {-1,-1,-1};
    void FocusChatMode();
    void ShowNotice(int message);
    void OpenNameDialog(int action);
    void FinishNotice(int result, bool mouse);
    void LayoutChat();
    struct ChatMessage { DWORD color, background; std::string text; };
    std::vector<ChatMessage> m_chatHistory;
    void ReflowChat();
    int VisibleButtons() const { return m_chatExpanded ? 23 : 16; }
    int ChatRows() const { return m_chatExpanded ? 14 : 5; }
    CTargetDisplay m_targetDisplay;
    CMinimap m_minimap;
    CWHWilImageData m_portraits;
	 CInterface();
	~CInterface();
public:
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// 左上角的 Message 管理。
//---------------------------------------------------------------------------------------------------------------//
	ClientSysMsg	m_xClientSysMsg;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Main Interface 处理变量.
//---------------------------------------------------------------------------------------------------------------//
	CWHWilImageData*	m_pxInterImg;					// 游戏中使用的界面图像.

	COMMONITEMSET		m_stCommonItem;					// 公共物品存储(鼠标物品).
	RECT				m_rcMain;						// 主界面整个区域。

	RECT				m_rcChat;						// 聊天窗口区域.
	INT					m_nFstLine;						// 聊天画面最上面一行所指向的聊天列表的编号索引。
	CDLList<CHAT>		m_xChat;						// 分割后的聊天数据。

	CDLList<INT>		m_xWndOrderList;				
	BOOL				m_bMoveFocusedWnd;

	SHORT				m_shBeltHeight;
	BYTE				m_bBeltState;

	CGameBtn			m_xInterBtn[_MAX_INTER_BTN];	// 按钮.

	CScrlBar			m_xScrlBar;

	BOOL				m_bShowKey;

	CLoginNotice		m_xMsgBox;						// 容器.
	INT					m_nIdxMsgBox;					// 容器.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// 由 Main Interface 管理的 Window.
//---------------------------------------------------------------------------------------------------------------//
	CInventoryWnd		m_xInventoryWnd;				//背包窗口.
    CSkillWnd m_xSkillWnd;
    CQuickWnd m_xQuickWnd;
    CPortraitWnd m_xPortraitWnd;
    bool m_portraitEntryPressed=false;
    bool PortraitEntryHit(POINT point);
    CGuildEditor m_xGuildEditor;
    CMailWnd m_xMailWnd;
	CStatusWnd			m_xStatusWnd;					//角色信息查看窗口(3个)。
	CStoreWnd			m_xStoreWnd;					//商店窗口.
	CExchangeWnd		m_xExchangeWnd;					//交易窗口.
	CGuildMasterWnd		m_xGuildMasterWnd;				//门主窗口.
	CGuildWnd			m_xGuildWnd;					//文元窗口.
	CGroupWnd			m_xGroupWnd;					//组窗口。
	CGroupPopWnd		m_xGroupPopWnd;					//组输入窗口.
	CChatPopWnd			m_xChatPopWnd;					//组输入窗口.
	CNPCWnd				m_xNPCWnd;						//NPC 对话窗口。
	CQuestWnd			m_xQuestWnd;					//任务窗口.
	COptionWnd			m_xOptionWnd;					//选项窗口.
	CHorseWnd			m_xHorseWnd;					//聊天框.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
public:
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// 初始化, 删除, 创建.
//---------------------------------------------------------------------------------------------------------------//
	VOID			Init();
	VOID			Destroy();
	VOID			CreateInterface(CImageHandler* pxImgHandler);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// 聊天消息相关函数.
//---------------------------------------------------------------------------------------------------------------//
	VOID			MsgAdd(DWORD dwFontColor, DWORD dwFontBackColor, CHAR* pszMsg = NULL);
	BOOL			MsgDelete();					// 消息删除。
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// 主界面相关函数.
//---------------------------------------------------------------------------------------------------------------//
	VOID			RenderInterface(INT nLoopTime, POINT ptMousePos);	// 在屏幕上显示界面相关部分。

	VOID			ShowWndList();
	VOID			ShowChatList();
	VOID			ShowBeltItem();
	VOID			ShowGameStatus();
	VOID			ShowMagicKey();
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// 窗口列表管理及状态值设置函数.
//---------------------------------------------------------------------------------------------------------------//
	INT				GetWindowInMousePos(POINT ptMouse);
	VOID			AddWindowToList(INT nID);
	VOID			DeleteWindowToList(INT nID);

	BOOL			WindowActivate(INT nID);
	VOID			WindowFocusChangedAndSetReadyMove(INT nID, POINT ptMouse);
	VOID			MoveTopWindow(POINT ptMouse);
	VOID			AllWindowsFocusOff();
	BOOL			IsTopWindow(INT nID);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Message Funtion.
//---------------------------------------------------------------------------------------------------------------//
	VOID			OnSysKeyDown();
	BOOL 			OnKeyDown(WPARAM wParam, LPARAM lParam);
	BOOL 			OnKeyUp(WPARAM wParam, LPARAM lParam);
	BOOL			OnLButtonDown(POINT ptMouse);
	BOOL			OnLButtonUp(POINT ptMouse);
	BOOL			OnMouseMove(POINT ptMouse);
	BOOL			OnScrollDown();
	BOOL			OnScrollUp();
	BOOL			OnLButtonDoubleClick(POINT ptMouse);
	LRESULT			OnMsgInputted(WPARAM wParam, LPARAM lParam);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
};



#endif // _INTERFACE_H



