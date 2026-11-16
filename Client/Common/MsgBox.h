#ifndef _MSGBOX_H_
#define _MSGBOX_H_
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// CMsgBox.h
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////////////////

#define CLICK_NONE			0
#define CLICK_OK			1
#define CLICK_YES			2
#define CLICK_NO			4
#define CLICK_CANCLE		8
#define HAVE_EDITOBOX		128

#define MSG_BTN_OK				1
#define MSG_BTN_YESNO			2
#define MSG_BTN_YESNOCANCLE		4
#define MSG_EDITEXIST			128

#define ASK_MAKE_BIG_GUILD			1		// 请填写要创建的行会名称.
#define ASK_MAKE_SMALL_GUILD		2		// 请输入要创建的行会名称。
#define ASK_OTHERSNAME				3		// 请输入对方行会名称.
#define ASK_DELMEMBER_NAME			4		// 请输入要从行会中排除的人员名称.
#define ASK_ADDMEMBER_NAME			5		// 请输入要添加为行会成员的人的名称.
#define ASK_DROP_GOLD				6		// 请输入要存放的金额.
#define ASK_DEAL_GOLD				7		// 请输入交易金额。
#define ASK_GROUP_ADD_MEMBER_NAME	8		// 请填写要加入队伍的人的姓名.
#define ASK_GROUP_DEL_MEMBER_NAME	9		// 请写下要退出队伍的人的姓名.
#define	ASK_GUILD_BREAK_ALLY		10		// 请输入要解除同盟的行会名称.
#define	ASK_YN_LOGOUT_GAME			11		// 要注销吗?
#define ASK_YN_QUIT_GAME			12		// 是否退出传奇2?
#define	ASK_YN_GUILD_ALLY			13		// 要进行结盟，对方行会必须处于 [可结盟] 状态，并且必须与对方行会会长面对面。
#define	ASK_YN_GAIN_SKILL			14		// 要学习吗?
#define	ASK_YN_USE_ITEM				15		// 是否使用？
#define ASK_WITHDRAWAL				16		// 请输入要查找的金额.
#define ASK_RECEIPT					17		// 请输入要寄存的金额.


class	CMsgBox
{
	//////////////////////////////
	// Function & Procedure
	//////////////////////////////
public:
	CMsgBox();			// Constructor
	~CMsgBox();			// Destructor
	// Function
	virtual BOOL	SetMsgBox(CHAR* szMsg,INT nType);				// Init Messages
	virtual BOOL	ShowMessageBox(VOID);							// Show Message box with Initialized
	virtual BOOL	ShowMessageBox(CHAR* szMsg,INT nType=0);		// Show	Message box with Initialization
	virtual BOOL	HideMessageBox(VOID);							// Hide Message box
	virtual BOOL	DestoryMessageBox(VOID);						// DestoryMessage box

	// Information
	BOOL	IsActive(VOID);
	BOOL	IsInited(VOID);

	// Message Process
	virtual HRESULT	OnButtonDown(LPARAM lParam, WPARAM wParam);		// Button Down
	virtual HRESULT	OnButtonUp(LPARAM lParam,WPARAM wParam);		// Button Up

	// Render Process
	virtual BOOL	RenderMessageBox(INT nLoopTIme);				// Render Function
private:
protected:
	__inline BOOL	IsInRect(int nPosX, int nPosY, RECT rc)	// 检查是否没有正确设置 Rect
	{	if (nPosX >= rc.left && nPosX <= rc.right && nPosY >= rc.top && nPosY <= rc.bottom)
			return TRUE;
		else 
			return FALSE;
	}
public:
	CHAR	m_szMsg[MAX_PATH];			// 要输出的字符
private:
protected:
	INT		m_nType;			// 0: OK(确认) ,1 : YES/NO ,2 : YES/NO/Cancle ,3 : EditBox & OK(确认) ...
	BOOL	m_bActive;			// Show or Hide ??
	BOOL	m_bInited;			// Initialized ?
	BOOL	m_bSet;				// Set ?
};

#endif  _MSGBOX_H_