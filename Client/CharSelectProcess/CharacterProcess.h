#include "Common/LoginNotice.h"
// CharacterProcess.h: interface for the CCharacterProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHARACTERPROCESS_H__70098477_5700_4CC3_88C4_5E7460A1F63A__INCLUDED_)
#define AFX_CHARACTERPROCESS_H__70098477_5700_4CC3_88C4_5E7460A1F63A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



//#define ANITICK				200


class CCharacterProcess: public CWHDefProcess  
{
public:
    bool EnterCharacter(const std::string& name) { return !IsWaiting() && !HasNotice() && !m_bCreatePending && !m_deletePending && m_xSelectChr.EnterCharacter(name); }
    bool DeleteCharacter(const std::string& name,const std::string& password);
    const char* DeletionResult() const { return m_deletionResult; }
    bool CreateCharacter(const std::string& name,int job,int gender);
    SelectionRoster Roster();
    const char* CreationResult() const { return m_creationResult; }
    bool DismissNotice();
    bool HasNotice() const { return m_xMsgBox.IsActive()!=FALSE; }
						CCharacterProcess();
	virtual				~CCharacterProcess();
	VOID				Init();
	VOID				Load(bool connect = true);
	bool PrepareReturn();
	void LoadReturn(bool connect = true);
	bool IsWaiting() const { return m_returnWaiting; }
	void Preview(const char* page);

// Message Function
	LRESULT				OnKeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT				OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT				OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT				OnSocketMessage(WPARAM wParam,LPARAM lParam);
	LRESULT				OnMouseOn(WPARAM wParam,LPARAM lParam);
	LRESULT				OnWindowMove(WPARAM wParam,LPARAM lParam);
	LRESULT				OnMouseMove(WPARAM wParam,LPARAM lParam);

	void				OnSocketMessageRecieve(char* pszMsg);

	virtual VOID		OnConnectToServer();
	virtual char*		OnMessageReceive(CHAR* pszMessage);

	virtual LRESULT DefMainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Socket
	BOOL				ConnectServer(SOCKET &s, SOCKADDR_IN* addr, UINT nMsgID, LPCTSTR lpServerIP, DWORD dwIP, int nPort, long lEvent);

// Process control
	VOID				DeleteProc();
	VOID				SetNextProc();

// Scene Renders
	VOID				RenderScene(INT nLoopTime);
	VOID				RenderCharCreate(INT nLoopTime);
	VOID				SetErrorMessage(INT nMsg,CHAR* szMgs);
	virtual VOID		ShowStatus(INT nStartX, INT nStartY);
	BOOL				MessageBoxProcess(LPARAM lParam);
	
private:
    friend struct CharacterInteractionPreview;
    friend struct RolePresentationPreview;
    void OpenCreation();
    void CloseCreationEditor();
    void ExitSelection();
    bool m_offline=false;
    const char* m_creationResult="idle";
    const char* m_deletionResult="idle";
    bool m_deletePending=false;
    std::string m_lastUrl;
    friend struct SelectionRosterPreview;
    friend struct SelectionDialogPreview;
    void ShowNotice(int message);
    void ApplyRoster(const SelectionRoster& roster);
    void BeginRosterWait();
    void RequestRoster();
    void AdvanceRoster(DWORD elapsed);
    DWORD m_selectionElapsed=SelectionRoster::Delay;
    std::optional<SelectionRoster> m_pendingRoster;
    int m_pendingNotice=0;
    void FinishNotice(HRESULT result);
    void SubmitCreation();
    friend struct MoviePreview;
	void OpenDeleteNotice();
	void DrawCreationDescription();
	CLoginMessages m_messages;
    int m_createPressed = -1;
	BOOL m_bCreatePending;
	RECT m_createBounds[2];
	CBackGround			m_xBGround;
	CSelectChr			m_xSelectChr;
	char m_selectionHost[256] = {};
	int m_selectionPort = 0;
	bool m_returnWaiting = false;
	WSADATA				m_wsd;
	CWHWilImageData		m_Image;
	CWHWilImageData		m_Image2;
	CWHWilImageData		m_tImage1;	// 临时
	int					m_Progress;
	SOCKET 				m_LoginSocket;
	SOCKADDR_IN			m_LoginAddress;
	UINT				m_ButtonID;
	POINT				m_ptMousePos;
	int					m_ConnectionState;		// 0: not login.
												// 1: login success.
												// 2: login fail etc.
	BOOL				m_fIsLButtonClicked;

	CPDLList<SelectionRoleInfo>	m_CList;				// 角色 Info 保存 List
	CPDLList<CHRRECT>	m_CRectList;			// 角色 Rect 保存 List
	// Buttons
	CMirButton			*m_pBtn;
	INT					m_nMessage;				// Error Message Box Index
	BYTE				m_SelectJob;			// 创建窗口中选中的职业
	BYTE				m_SelectSex;			// 创建窗口中选定的性别
	INT					m_SelectChr;			// 选择窗口中选中的角色编号
	INT					m_ChrCount;				// 通过查询得到的角色数量
	char				m_szChrName[16];
	CHAR				m_szErrorMsg[512];
	int					m_Distance;
    RoleAnimation m_creationAnimation[2];
	POINT				m_PosName;
	CLoginNotice		m_xMsgBox;
/*	CAvi				m_xCreateChrAvi;					// For Create Charector
	CAvi				m_xStartGameAvi;					// For Create Start Game*/
}; 

#endif // !defined(AFX_CHARACTERPROCESS_H__70098477_5700_4CC3_88C4_5E7460A1F63A__INCLUDED_)
