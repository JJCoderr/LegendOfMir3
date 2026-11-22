#pragma once

// MsgBox
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

// MirMsgBox
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// CMirMsgBox.h
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////////////////

class	CMirMsgBox : public CMsgBox
{
	//////////////////////////////
	// Function & Procedure
	//////////////////////////////
public:
	CMirMsgBox();	// Constructor
	~CMirMsgBox();	// Destructor
	// Function
	BOOL	ShowMessageBox(CHAR* szMgs,INT nType,INT nImgIndex);	// Show	Message box with Initialization
	BOOL	ShowMessageBox(CHAR* szMgs,INT nType);	// Show	Message box with Initialization
	BOOL	StringDivideLen(INT nDivideLen, INT& nDividedLine, CHAR* szSrc,CDLList<CHAR*>* m_pxpStr);
	BOOL	HideMessageBox(VOID);									// Hide Message box
	BOOL	DestoryMessageBox(VOID);								// DestoryMessage box
	BOOL	Load(CWHWilImageData* pxBtnImage);
	// Message Process
	virtual HRESULT	OnButtonDown(POINT ptMouse);		// Button Down
	virtual HRESULT	OnButtonUp(POINT ptMouse);		// Button Up

	// Render Process
	virtual BOOL	RenderMessageBox(INT nLoopTIme);							// Render Function
private:
protected:
	BOOL	SetBoxImage(INT nImageIndex,INT nType);
	//////////////////////////////
	// Variables
	//////////////////////////////
public:
private:
protected:
	CDLList<CHAR*>	m_xMsg;		// 将被输出的字符
	INT		m_nImgIndex;
	POINT	m_xBoxPos;
	SHORT	m_shWidth;
	SHORT	m_shHeight;
	WORD*	m_wpData;
	INT		m_nStart;
	INT		m_nEnd;
	CWHWilImageData* m_pxBtnImage;
	CMsgBoxBtn	m_xMsgBtn[4];	// 0 : OK  1: YES  2:NO  3: Cancle
};

// MirMsgBox3D
class	CMirMsgBox3D : public CMirMsgBox
{
public:
	CMirMsgBox3D();
	~CMirMsgBox3D();

	virtual BOOL	Load(CWHWilImageData* pxBtnImage);
	virtual BOOL	DestoryMessageBox(VOID);								// DestoryMessage box

	BOOL			CreateWnd(CWHWilImageData* pxWndImage, BOOL bCanMove, INT nStartX, INT nStartY, INT nWidth, INT nHeight);

	virtual	BOOL	ShowMessageBox(CHAR* szMgs,INT nType,INT nImgIndex);	// Show	Message box with Initialization
	virtual BOOL	HideMessageBox(VOID);									// Hide Message box

	virtual BOOL	OnKeyDown(WPARAM wParam,LPARAM lParam);

	virtual HRESULT	OnButtonDown(POINT ptMouse);		// Button Down
	virtual HRESULT	OnButtonUp(POINT ptMouse);		// Button Up

	VOID			MoveWnd(POINT ptMouse);

	// Render
	virtual BOOL	RenderMessageBox(INT nLoopTIme);
	BOOL			RenderBox(VOID);
private:
	INT						m_nID;
	INT						m_nFrameImgIdx;
	CWHWilImageData*		m_pxWndImage;
	BOOL					m_bCanMove;
	BOOL					m_bFocused;
	POINT					m_bGrabbedMousePos;
	// 3D测试
	D3DVERTEX		m_avMagic[4];
	RECT			m_rcWnd;
	RECT			m_rcImgWnd;
	INT				m_nCurrImageIdx;

	INT				m_nWidth;
	INT				m_nHeight;
protected:
};

// NoticeBox
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// CMirMsgBox.h
//////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////////////////
class	CNoticeBox
{
public:
	CNoticeBox();	// Constructor
	~CNoticeBox();	// Destructor

	// Function
	BOOL	IsActive(VOID){return m_bActive;}
	BOOL	IsInited(VOID){return m_bSet;}

	BOOL	StringDivideLen(INT nDivideLen, INT& nDividedLine, CHAR* szSrc,CDLList<CHAR*>* m_pxpStr);

	virtual BOOL	Load(CWHWilImageData* pxBtnImage);
	virtual BOOL	SetBoxImage(INT nImageIndex);
	virtual BOOL	SetMsg(CHAR* szMsg);
	virtual BOOL	ShowMessage(CHAR* szMsg);
	virtual BOOL	ShowMessageBox(CHAR* szMsg,INT nImgIndex);		// Show	Message box with Initialization
	virtual BOOL	HideMessageBox(VOID);							// Hide Message box
	virtual BOOL	DestoryMessageBox(VOID);						// DestoryMessage box

	// Message Process
	virtual HRESULT	OnButtonDown(POINT ptMouse);					// Button Down
	virtual HRESULT	OnButtonUp(POINT ptMouse);						// Button Up
			VOID	MoveWnd(POINT ptMouse);							// Move Window

	// Render Process
	virtual BOOL	RenderMessageBox(INT nLoopTIme);				// Render Function

private:
	RECT	m_rcWnd;
	POINT	m_bGrabbedMousePos;
	BOOL	m_bActive;			// Show or Hide ??
	BOOL	m_bInited;			// Initialized ?
	BOOL	m_bSet;				// Set ?
	CHAR	m_szMsg[8196];
	WORD*	m_wpData;
	SHORT	m_shWidth;
	SHORT	m_shHeight;
	BOOL	m_bIsSelected;

	CDLList<CHAR*>		m_xMsg;		// 将被输出的字符
	CGameBtn			m_xMsgBtn;
	CWHWilImageData*	m_pxBtnImage;
protected:
	__inline BOOL	IsInRect(int nPosX, int nPosY, RECT rc)	// 检查是否没有正确设置 Rect
	{	if (nPosX >= rc.left && nPosX <= rc.right && nPosY >= rc.top && nPosY <= rc.bottom)
			return TRUE;
		else
			return FALSE;
	}
};

// GameWnd
struct tagCOMMONITEMSET;

class CGameWnd
{
public:
	INT				m_nID;					// 窗口管理器用于区分此窗口的标识符。
	RECT			m_rcImgWnd;				// 窗口图像的区域.
	RECT			m_rcWnd;				// 窗口的屏幕区域.

	INT				 m_nFrameImgIdx;		// 图像相关变量.
	CWHWilImageData* m_pxWndImage;

	BOOL			m_bActive;				// 窗口是否激活。
	BOOL			m_bFocused;
	BOOL			m_bMoving;				// 窗口是否正在移动.
	BOOL			m_bCanMove;				// 窗口是否固定.

	INT				m_nWidth;
	INT				m_nHeight;

	POINT			m_bGrabbedMousePos;
	// 3D测试
	D3DVERTEX		m_avBoard[4];

public:
	__inline RECT GetGameWndRect()
	{
		return m_rcWnd;
	}
	__inline BOOL GetGameWndActive()
	{
		return m_bActive;
	}

	__inline CGameWnd()
	{
		Init();
	}

	~CGameWnd()
	{
		Destroy();
	}

	virtual VOID Init();
	virtual VOID Destroy();
	virtual VOID ShowGameWnd();
	virtual void ShowPanel() = 0;
	virtual BOOL OnLButtonDown(tagCOMMONITEMSET* item, POINT mouse) = 0;
	virtual BOOL OnLButtonUp(tagCOMMONITEMSET* item, POINT mouse) = 0;
	virtual void OnMouseMove(POINT mouse) = 0;


	VOID ResetWndImg(INT nIdx);
	VOID CreateGameWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, BOOL bCanMove, INT nStartX, INT nStartY, INT nWidth = 0, INT nHeight = 0);
	VOID DestroyGameWnd();
	VOID SetGameWndActive(BOOL bActive);
	VOID SetGameWndFocused(BOOL bFocused);
	VOID SetReadyMove(POINT ptMouse);
	VOID MoveGameWnd(POINT ptMouse);
};

// InventoryWnd
#pragma pack(1)
typedef struct tagITEMSET
{
	BOOL	bSetted;
	CItem	xItem;
}ITEMSET, *LPITEMSET;
typedef struct tagCOMMONITEMSET
{
	BOOL	bSetted;					// 物品是否已设置.
	BOOL	bWaitResponse;				// 当前物品是否已被吃掉或穿戴,正在等待服务器响应.
	BOOL	bIsEquipItem;				// 当前物品是否来自装备窗口.
	BOOL	bIsBeltItem;				// 当前物品是否来自腰带窗口.
	BOOL	bIsHideItem;				// 是否显示鼠标状态下的物品?。
	CItem	xItem;						// 物品.
}COMMONITEMSET, *LPCOMMONITEMSET;
#pragma pack(8)


class CInventoryWnd : public CGameWnd
{
protected:
	INT			m_nStartLineNum;					// 背包窗口起始行的编号. (共 10 行, 屏幕上最多可看到 4 行.)
	RECT		m_rcInvenCell[_INVEN_MAX_CELL];		// 背包窗口各单元格的区域。
	RECT		m_rcBeltCell[_BELT_MAX_CELL];

	CGameBtn	m_xInventoryBtn[_MAX_INVEN_BTN];

	CScrlBar	m_xInvenScrlBar;

public:
	ITEMSET		m_stInventoryItem[_MAX_INVEN_ITEM];
	ITEMSET		m_stBeltItem[_MAX_BELT_ITEM];

public:
	CInventoryWnd();
	~CInventoryWnd();

	virtual VOID Init();
	virtual VOID Destroy();

	VOID	CreateInventoryWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove);
	VOID	ShowPanel() override;
	VOID	ShowItemStatus(LPCLIENTITEMRCD pstItemInfo);
	VOID	ShowInvenItemState();
	VOID	SetInvenBtnInit();

public:
	BOOL	CheckItemPos(CHAR* szName);
	BOOL	SaveItemPos(CHAR* szName);

	VOID	DeleteBeltItem(INT nBeltInventoryNum);
	VOID	DeleteInvenItem(INT nInventoryNum);
	VOID	DeleteInvenItem(INT nMakeIndex, CHAR* szName);
	VOID	DeleteAllBeltItem();
	VOID	DeleteAllInvenItem();

	BOOL	AddInvenItem(CItem xItem, INT nInvenNum, BOOL bUseInvenNum = TRUE);
	BOOL	AddBeltItem(CItem xItem, INT nBeltNum, BOOL bUseBeltNum = TRUE);

	INT		GetInvenNum(POINT ptMouse);
	INT		GetBeltNum(POINT ptMouse);

	BOOL	ExtractInvenItem(CItem* pxItem, INT nInventoryNum);
	BOOL	ExtractBeltItem(CItem* pxItem, INT nBeltInventoryNum);

	VOID	ChangeInvenWithCommon(INT nInventoryNum, LPCOMMONITEMSET pstCommonItemSet);
	VOID	ChangeBeltWithCommon(INT nBeltInventoryNum, LPCOMMONITEMSET pstCommonItemSet);

	VOID	SetCommonFromInven(INT nInventoryNum, LPCOMMONITEMSET pstCommonItemSet);
	VOID	SetCommonFromBelt(INT nBeltInventoryNum, LPCOMMONITEMSET pstCommonItemSet);

	VOID	SetInvenFromCommon(INT nInventoryNum, LPCOMMONITEMSET pstCommonItemSet);
	VOID	SetBeltFromCommon(INT nBeltInventoryNum, LPCOMMONITEMSET pstCommonItemSet);

	VOID	AddNewBeltItem(LPCOMMONITEMSET pstCommonItemSet);


	BOOL	OnKeyUp(LPCOMMONITEMSET pstCommonItemSet, WPARAM wParam, LPARAM lParam);
	BOOL	OnLButtonDoubleClick(LPCOMMONITEMSET pstCommonItemSet, POINT ptMouse);
	BOOL	OnLButtonDown(LPCOMMONITEMSET pstCommonItemSet, POINT ptMouse);
	BOOL	OnLButtonUp(LPCOMMONITEMSET pstCommonItemSet, POINT ptMouse);
	VOID	OnMouseMove(POINT ptMouse);
	VOID	OnScrollDown();
	VOID	OnScrollUp();
};

// StatusWnd
#pragma pack(1)
typedef struct tagSTANDARDMAGIC
{
	WORD	wMagicID;
	char	szMagicName[13];
	BYTE	bEffectType;
	BYTE	bEffect;
	WORD	wSpell;
	WORD	wMinPower;
	BYTE	btNeed[4];
	INT		nTrain[4];
	BYTE	btMaxTrainLevel;
	BYTE	btJob;
	INT		nDelayTime;
	BYTE	bDefSpell;
	BYTE	btDefMinPower;
	WORD	wMaxPower;
	BYTE	btDefMaxPower;
	char	szDesc[16];
}STANDARDMAGIC, *LPSTANDARDMAGIC;

typedef struct tagCLIENTMAGICRCD
{
	INT				nCurrTrain;
	CHAR			bUseKey;
	BYTE			bLevel;
	STANDARDMAGIC	stStdMagic;
}CLIENTMAGICRCD, *LPCLIENTMAGICRCD;
#pragma pack(8)



class CStatusWnd : public CGameWnd
{
private:
protected:
	BYTE				m_bType;						// 0 : 装备窗口. 1 : 角色状态窗口.  2 : 角色魔法设置窗口。
	CGameBtn			m_xStatusBtn[_MAX_STAUTS_BTN];

	RECT				m_rcCell[_EQUIP_MAX_CELL];		// 背包窗口各单元格的区域。

	RECT				m_rcMagicCell[_MAGIC_MAX_CELL];	// 魔法设置窗口各单元格的区域.
	INT					m_nStartLineNum;				// 魔法窗口首行的编号。(可以按魔法列表中包含的个数进行滚动..)
	BYTE				m_bMagicIdxTable[_MAX_MAGIC_SKILL];
	CGameBtn			m_xMagicScrlBtn[2];


public:
	CHAR				m_szEquipItemIdx[_EQUIP_MAX_CELL][12];
	ITEMSET				m_stEquipItem[_EQUIP_MAX_CELL];
	ITEMSET				m_stTakeOnItemSet;				// 将待装备的物品临时保存, 直到收到是否穿戴的消息.

	BYTE				m_bMyMagicCnt;
	LPCLIENTMAGICRCD	m_pstMyMagic;

public:
	CStatusWnd();
	~CStatusWnd();

	virtual VOID Init();
	virtual VOID Destory();

	__inline VOID SetStatusWndType(BYTE bType)
	{
		m_bType = bType;
		switch ( m_bType )
		{
		case _TYPE_EQUIP:
			ResetWndImg(_WNDIMGIDX_ITEMSET);
			break;
		case _TYPE_STATUS:
			ResetWndImg(_WNDIMGIDX_STATUS);
			m_rcWnd.left -= 250;
			m_rcWnd.right-= 250;
			break;
		case _TYPE_SETMAGIC:
			ResetWndImg(_WNDIMGIDX_SKILLSET);
			break;
		}
	}
	__inline BYTE GetStatusWndType()
	{
		return m_bType;
	}

	VOID	CreateStatusWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove);
	VOID	ShowPanel() override;
	VOID	SetStatusBtnInit();

public:
	VOID	ShowCharEquipWnd();
	VOID	ShowEquipItemState();
	BOOL	CheckEquipItem(CItem* pxItem, BYTE bEquipCell, POINT ptMouse);
	INT		GetEquipNum(POINT ptMouse);
	BOOL	CheckIsItemExisted(INT nEquipNum);
	VOID	DeleteEquipItem(INT nEquipNum);
	VOID	DeleteEquipItem(INT nMakeIndex, CHAR* szName);
	VOID	DeleteAllEquipItem();
	BOOL	AddEquipItem(CItem xItem, INT nEquipNum, BOOL bUseEquipNum = TRUE);
	BOOL	ExtractEquipItem(CItem* pxItem, INT nEquipNum);
	VOID	ChangeEquipWithCommon(INT nEquipNum, LPCOMMONITEMSET pstCommonItemSet);
	VOID	SetEquipFromCommon(INT nEquipNum, LPCOMMONITEMSET pstCommonItemSet);
	VOID	SetCommonFromEquip(INT nEquipNum, LPCOMMONITEMSET pstCommonItemSet);
	VOID	SetTakeOnFromEquip(INT nEquipNum);
	VOID	SetTakeOnFromCommon(INT nEquipNum, LPCOMMONITEMSET pstCommonItemSet);
	VOID	SetEquipFromTakeOn();

	VOID	ShowCharStatus();

	VOID			 ShowCharSetMagicWnd();
	INT				 SetMagicKey(POINT ptMouse, BYTE bKey);
	LPCLIENTMAGICRCD GetMagicByKey(BYTE bKey);

	BOOL	OnLButtonDown(LPCOMMONITEMSET pstItemSet, POINT ptMouse);
	BOOL	OnLButtonUp(LPCOMMONITEMSET pstItemSet, POINT ptMouse);
	BOOL	OnKeyDown(WPARAM wParam, LPARAM lParam, POINT ptMouse);
	VOID	OnMouseMove(POINT ptMouse);
	VOID	OnScrollDown();
	VOID	OnScrollUp();
};



class CButtonPanel : public CGameWnd
{
	CGameBtn button;
	POINT moveOffset;
public:
	CButtonPanel() { Init(); }
	~CButtonPanel() { Destroy(); }
	void Init() override { CGameWnd::Init(); button.Init(); }
	void Create(INT id, CWHWilImageData* image, INT background, INT frame,
	            INT x, INT y, INT moveX, INT moveY);
	void ShowPanel() { ShowGameWnd(); button.ShowGameBtn(); }
	void SetStatusBtnInit() { button.SetBtnState(_BTN_STATE_NORMAL); }
	BOOL OnLButtonDown(LPCOMMONITEMSET, POINT mouse) override { return button.OnLButtonDown(mouse) != FALSE; }
	BOOL OnLButtonUp(LPCOMMONITEMSET, POINT mouse) override { return button.OnLButtonUp(mouse) != FALSE; }
	void OnMouseMove(POINT mouse);
};

// GuildWnd
class CGuildWnd : public CGameWnd
{
private:
protected:
	CGameBtn				m_xGuildBtn[5];

public:
	CGuildWnd();
	~CGuildWnd();


	VOID	CreateGuildWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove);
	VOID	ShowPanel() override;
public:
//  消息函数.
	BOOL	OnLButtonDown(LPCOMMONITEMSET, POINT ptMouse) override;
	BOOL	OnLButtonUp(LPCOMMONITEMSET, POINT ptMouse) override;
	VOID	OnMouseMove(POINT ptMouse);

	VOID	OnScrollDown();
	VOID	OnScrollUp();
};

// GroupWnd
class CGroupMember
{
public:
	char		szMemberName[32];
};

class CGroupWnd : public CGameWnd
{
public:
	CPDLList<CGroupMember>	m_xGroupList;

public:
	CGroupWnd();
	~CGroupWnd();

	VOID	CreateGroupWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove);
	VOID	ShowPanel() override;

public:
//  消息函数.
	BOOL	OnLButtonDown(LPCOMMONITEMSET, POINT ptMouse) override;
	BOOL	OnLButtonUp(LPCOMMONITEMSET, POINT ptMouse) override;
	VOID	OnMouseMove(POINT ptMouse);

	VOID	OnScrollDown();
	VOID	OnScrollUp();
};

// GroupPopWnd
class CGroupPopWnd : public CGameWnd
{
private:
protected:
	CGameBtn				m_xGroupPopBtn[5];

public:
	CGroupPopWnd();
	~CGroupPopWnd();
	VOID	CreateGroupPopWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove);
	VOID	ShowPanel() override;


public:
//  消息函数.
	BOOL	OnLButtonDown(LPCOMMONITEMSET, POINT ptMouse) override;
	BOOL	OnLButtonUp(LPCOMMONITEMSET, POINT ptMouse) override;
	VOID	OnMouseMove(POINT ptMouse);

	VOID	OnScrollDown();
	VOID	OnScrollUp();
};

// ChatPopWnd
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
	VOID	ShowPanel() override;
	VOID	SetStatusBtnInit();
public:
//  消息函数.
	BOOL	OnLButtonDown(LPCOMMONITEMSET, POINT ptMouse) override;
	BOOL	OnLButtonUp(LPCOMMONITEMSET, POINT ptMouse) override;
	VOID	OnMouseMove(POINT ptMouse);

	VOID	OnScrollDown();
	VOID	OnScrollUp();
};




// NPCWnd
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
	VOID	ShowPanel() override;

	__inline void	SetNPC(int nNPCID) { m_nNpcId = nNPCID; }
	void	ResetDialog();

public:
	VOID SetNpcEditBoxPos(VOID);
	VOID SetWndRectReduce(VOID);
	VOID SetWndRectExtend(VOID);
//  消息函数.
	BOOL	OnLButtonDown(LPCOMMONITEMSET, POINT ptMouse) override;
	BOOL	OnLButtonUp(LPCOMMONITEMSET, POINT ptMouse) override;
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

// StoreWnd
using namespace std;


typedef struct tagQUALITYINFO
{
	string	stlszQuality;
	string	stlszPrice;
}QUALITYINFO, *LPQUALITYINFO;

typedef struct tagARTICLEINFO
{
	string					stlszArticle;
	CDLList<QUALITYINFO>	m_xQualityList;
}ARTICLEINFO, *LPARTICLEINFO;


class CStoreWnd : public CGameWnd
{
private:
	INT						m_nIsReadyToDisplay;
	INT						m_nCurrentItemTop;
	INT						m_nCurrentKindTop;
	INT						m_nMaxKindLine;
	INT						m_nMaxItemLine;
	INT						m_nNpcID;

	CTextButton				m_xKindBtn;
	CTextButton				m_xItemLstBtn;

	RECT					m_xKindRect;
	RECT					m_xItemRect;

	CPDLList<CLIENTITEMRCD>	m_xItems;

	BOOL					m_bSelectedKind;
	BOOL					m_bSelectedItem;
protected:
	BYTE					m_bState;					// 0 : Sell. 1 : Buy.  2 : Repair   3 : Special Repair
	CGameBtn				m_xStoreBtn[5];
	CDLList<ARTICLEINFO>	m_stArticleList;

	SHORT					m_shArticleStart;
	SHORT					m_shQualityStart;

public:
	__inline VOID SetNPC(INT Id)
	{
		m_nNpcID = Id;
	}

	inline	BOOL			IsInRect(POINT tPos,RECT tRect)
	{
		INT	nTemp;
		RECT twRect;
		twRect = GetGameWndRect();
		nTemp = tRect.left - (tPos.x - twRect.left);
		if(nTemp<=0)
		{
			nTemp = nTemp + (tRect.right-tRect.left);
			if(nTemp>=0)
			{
				nTemp = tRect.top - (tPos.y - twRect.top);
				if(nTemp<=0)
				{
					nTemp = nTemp + (tRect.bottom - tRect.top);
					if(nTemp>=0)
					{
						return TRUE;
					}
				}
			}
		}
		return FALSE;
	};
	CStoreWnd();
	~CStoreWnd();
	VOID	CreateStoreWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove);
	VOID	ShowPanel() override;

	BOOL	SetBuyKinds(CHAR* msg,INT Cound);
	BOOL	SetBuyItemLst(CHAR* Msg,INT Cound);
	BOOL	SetSellKinds(CInventoryWnd*,CHAR Kind,INT nStdMode);
	BOOL	SetSellItemLst(CInventoryWnd*,CHAR Kind,BYTE nStdMode,CHAR bKind);
public:
//  消息函数.
	BOOL	OnLButtonDown(LPCOMMONITEMSET, POINT ptMouse) override;
	BOOL	OnLButtonUp(LPCOMMONITEMSET, POINT ptMouse) override;
	VOID	OnMouseMove(POINT ptMouse);

	VOID	OnScrollDown();
	VOID	OnScrollUp();

	CWHQueue			m_xPacketQueue;					// 保存需要在 NPC Chat 中处理的消息的队列.
	BOOL	OnUpdateQueue(VOID);

private:
	VOID	SendDeTailGoodsList(INT nIndex,CHAR* szItemName);
	VOID	SendButThisITme(CMTxtBtn* tBtn);

};

// Interface
class CInterface
{
	CGameWnd* FindWindowById(INT id);
public:
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
	CWHWilImageData		m_xInterImgEx;					// 界面相关图像。

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

	CMirMsgBox			m_xMsgBox;						// 容器.
	INT					m_nIdxMsgBox;					// 容器.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// 由 Main Interface 管理的 Window.
//---------------------------------------------------------------------------------------------------------------//
	CInventoryWnd		m_xInventoryWnd;				//背包窗口.
	CStatusWnd			m_xStatusWnd;					//角色信息查看窗口(3个)。
	CStoreWnd			m_xStoreWnd;					//商店窗口.
	CButtonPanel m_xExchangeWnd;					//交易窗口.
	CButtonPanel m_xGuildMasterWnd;				//门主窗口.
	CGuildWnd			m_xGuildWnd;					//文元窗口.
	CGroupWnd			m_xGroupWnd;					//组窗口。
	CGroupPopWnd		m_xGroupPopWnd;					//组输入窗口.
	CChatPopWnd			m_xChatPopWnd;					//组输入窗口.
	CNPCWnd				m_xNPCWnd;						//NPC 对话窗口。
	CButtonPanel m_xQuestWnd;					//任务窗口.
	CButtonPanel m_xOptionWnd;					//选项窗口.
	CButtonPanel m_xHorseWnd;					//聊天框.
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

// BMWnd
class CBMWnd
{
	/////////////////////////////////////////////////////////////////////////////////////////
	// Functions
public:
	CBMWnd();
	~CBMWnd();

	VOID	SetFocused(VOID);

	virtual VOID	OpenWnd(VOID);
	virtual VOID	CloseWnd(VOID);

	VOID	SetWndRect(RECT rc)		{m_rcWnd = rc;}
	BOOL	IsWndActive(VOID)		{return m_fIsActive;}
	BOOL	IsWndFocused(VOID)		{return m_fIsFocused;}

	virtual HRESULT	OnButtonDown(WPARAM wParam, LPARAM lParam);
	virtual HRESULT OnButtonDown(POINT ptMouse);
	virtual HRESULT	OnButtonUp(WPARAM wParam, LPARAM lParam);
	virtual HRESULT OnButtonUp(POINT ptMouse);
	virtual HRESULT OnKeyDown(WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnMouseMove(WPARAM wParam, LPARAM lParam);

	virtual	VOID	Render(INT	nLoopsTime);
private:
protected:
	__inline	BOOL	IsInRect(RECT rc,INT nX,INT nY)
	{
		if((rc.left > nX) || (rc.right < nX) || (rc.top > nY) || (rc.bottom < nY) ) return FALSE;
		return TRUE;
	}
	__inline	BOOL	IsInRect(RECT rc,POINT pt)
	{
		return IsInRect(rc, pt.x, pt.y);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	// Variable
public:
private:
protected:
	BOOL	m_fIsActive;
	BOOL	m_fIsFocused;
	RECT	m_rcWnd;
};

// LoginAvi
// LoginProcess.h: interface for the CLoginProcess class.
//
//////////////////////////////////////////////////////////////////////



class CLoginAvi : public CBMWnd
{
	/////////////////////////////////////////////////////////////////////////////////////////
	// Functions
public:
	CLoginAvi();
	~CLoginAvi();

	VOID	SetRenderAviState(INT nState);

	virtual VOID		Create(CWHWilImageData* pxImage);

	VOID				StopAllAvis(VOID);

	virtual HRESULT		OnButtonDown(WPARAM wParam, LPARAM lParam);
	using CBMWnd::OnButtonDown;
	virtual HRESULT		OnButtonUp(WPARAM wParam, LPARAM lParam);
	using CBMWnd::OnButtonUp;

	virtual	VOID		Render(INT	nLoopTime);
private:
protected:
	VOID				SetFocusBefore(VOID);
	VOID				SetFocusAfter(VOID);
	/////////////////////////////////////////////////////////////////////////////////////////
	// Variable
public:
private:
	INT					m_nRenderAviState;
protected:
	CWHWilImageData		*m_pxImage;
	CAvi				m_xAvi;
	CAvi				m_xLogoAvi;
	RECT				m_rcSrc;
};

// Login
// LoginProcess.h: interface for the CLoginProcess class.
//
//////////////////////////////////////////////////////////////////////



class CLogin : public CBMWnd
{
	/////////////////////////////////////////////////////////////////////////////////////////
	// Functions
public:
	CLogin();
	~CLogin();

	virtual VOID		Create(CWHWilImageData* pxImage);

	virtual HRESULT		OnButtonDown(WPARAM wParam, LPARAM lParam);
	virtual HRESULT		OnButtonDown(POINT ptMouse);
	virtual HRESULT		OnButtonUp(WPARAM wParam, LPARAM lParam);
	using CBMWnd::OnButtonUp;
	virtual HRESULT		OnKeyDown(WPARAM wParam, LPARAM lParam);
	virtual LRESULT		OnMouseMove(WPARAM wParam, LPARAM lParam);
	virtual	VOID		Render(INT	nLoopTime);
private:
protected:
	VOID				SetFocusBefore(VOID);
	VOID				SetFocusAfter(VOID);
	/////////////////////////////////////////////////////////////////////////////////////////
	// Variable
public:
private:
	INPUTSTATE			m_xInputState[2];
	CMirButton			m_xButtons[4];
	INT					m_nUserState;
	BOOL				m_fIsButtonDown;
protected:
	CWHWilImageData		*m_pxImage;
	CMirButton			*m_pBtn;
	BOOL				m_fShowIDPASS;
};

// NewAccount
// LoginProcess.h: interface for the CNewAccountProcess class.
//
//////////////////////////////////////////////////////////////////////



class CNewAccount : public CBMWnd
{
	/////////////////////////////////////////////////////////////////////////////////////////
	// Functions
public:
	CNewAccount();
	~CNewAccount();

	virtual VOID		Create(CWHWilImageData* pxImage);

	virtual HRESULT		OnButtonDown(WPARAM wParam, LPARAM lParam);
	virtual HRESULT		OnButtonDown(POINT ptMouse);
	virtual HRESULT		OnButtonUp(WPARAM wParam, LPARAM lParam);
	using CBMWnd::OnButtonUp;

	virtual	VOID		Render(INT	nLoopTime);
private:
protected:
	VOID				SetFocusBefore(VOID);
	VOID				SetFocusAfter(VOID);
	/////////////////////////////////////////////////////////////////////////////////////////
	// Variable
public:
private:
	INPUTSTATE			m_xInputState[2];
	_LOGINIMAGEINFO		m_xInputImg[4];			// 图像删除后
	CMirButton			m_xButtons[3];
	INT					m_nUserState;
	BOOL				m_fIsButtonDown;
protected:
	CWHWilImageData		*m_pxImage;
	CMirButton			*m_pBtn;
};

// SelectSrv
// LoginProcess.h: interface for the CLoginProcess class.
//
//////////////////////////////////////////////////////////////////////



class CSelectSrv : public CBMWnd
{
	/////////////////////////////////////////////////////////////////////////////////////////
	// Functions
public:
	CSelectSrv();
	~CSelectSrv();

	virtual VOID		Create(CWHWilImageData* pxImage,CPDLList<ServerList>* pxSList);

	virtual HRESULT		OnButtonDown(WPARAM wParam, LPARAM lParam);
	virtual HRESULT		OnButtonDown(POINT ptMouse);
	virtual HRESULT		OnButtonUp(WPARAM wParam, LPARAM lParam);
	using CBMWnd::OnButtonUp;
	virtual LRESULT		OnMouseMove(WPARAM wParam, LPARAM lParam);

	virtual	VOID		Render(INT	nLoopTime);
	VOID				Draw3DBorder(RECT rcWnd, COLORREF dwColor, INT nAlpha);
private:
protected:
	/////////////////////////////////////////////////////////////////////////////////////////
	// Variable
public:
private:
	BOOL					m_fIsButtonDown;
protected:
	CWHWilImageData			*m_pxImage;
	_PPSERVERBUTTON			m_ppxSrvBtn;
	INT						m_nSrvCount;
	D3DVERTEX				m_avBillBoard[4];
};

// BackGround
// LoginProcess.h: interface for the CBackGroundProcess class.
//
//////////////////////////////////////////////////////////////////////



class CBackGround : public CBMWnd
{
	/////////////////////////////////////////////////////////////////////////////////////////
	// Functions
public:
	CBackGround();
	~CBackGround();

	virtual VOID		Create(CWHWilImageData* pxImage);

	VOID				SetRenderState(INT nState);
	INT					GetRenderState(VOID);
	VOID				AllStopAvi(VOID);
	virtual HRESULT		OnButtonDown(WPARAM wParam, LPARAM lParam);
	using CBMWnd::OnButtonDown;
	virtual HRESULT		OnButtonUp(WPARAM wParam, LPARAM lParam);
	using CBMWnd::OnButtonUp;

	virtual	VOID			Render(INT	nLoopTime);
private:
protected:
	/////////////////////////////////////////////////////////////////////////////////////////
	// Variable
public:
	CAvi				m_xCreateChrAvi;					// For Create Charector
	CAvi				m_xStartGameAvi;					// For Create Start Game
	INT					m_nRenderState;
private:
protected:
	CWHWilImageData		*m_pxImage;
};

// CreateChr
// LoginProcess.h: interface for the CCreateChrProcess class.
//
//////////////////////////////////////////////////////////////////////



class CCreateChr : public CBMWnd
{
	/////////////////////////////////////////////////////////////////////////////////////////
	// Functions
public:
	CCreateChr();
	~CCreateChr();

	virtual VOID		Create(CWHWilImageData* pxImage);

	virtual HRESULT		OnButtonDown(WPARAM wParam, LPARAM lParam);
	virtual HRESULT		OnButtonDown(POINT ptMouse);
	virtual HRESULT		OnButtonUp(WPARAM wParam, LPARAM lParam);
	using CBMWnd::OnButtonUp;
	virtual HRESULT		OnKeyDown(WPARAM wParam, LPARAM lParam);

	virtual	VOID		Render(INT	nLoopTime);
private:
protected:
	/////////////////////////////////////////////////////////////////////////////////////////
	// Variable
public:
private:
	CMirButton			m_xButtons[4];
	BOOL				m_fIsButtonDown;
protected:
	CWHWilImageData		*m_pxImage;
	INT					m_nSelectGender;
	INT					m_nSelectJob;
};

// SelectChr
// LoginProcess.h: interface for the CSelectChrProcess class.
//
//////////////////////////////////////////////////////////////////////



class CSelectChr : public CBMWnd
{
	/////////////////////////////////////////////////////////////////////////////////////////
	// Functions
public:
	CSelectChr();
	~CSelectChr();

	virtual VOID		Create(CWHWilImageData* pxImage,CPDLList<CHRINFO>* pxList, CPDLList<CHRRECT>* pxrcList,INT nChrCount);

	virtual HRESULT		OnButtonDown(WPARAM wParam, LPARAM lParam);
	virtual HRESULT		OnButtonDown(POINT ptMouse);
	virtual HRESULT		OnButtonUp(WPARAM wParam, LPARAM lParam);
	using CBMWnd::OnButtonUp;
	virtual LRESULT		OnMouseMove(WPARAM wParam, LPARAM lParam);
	virtual VOID		OpenWnd(VOID);
	virtual VOID		CloseWnd(VOID);

	VOID				ShowChrInfo(VOID);
	virtual	VOID		Render(INT	nLoopTime);
private:
protected:
	/////////////////////////////////////////////////////////////////////////////////////////
	// Variable
public:
	INT					m_nRenderState;
	INT					m_nSelectChr;
private:
protected:
	CLightFog			m_xLightFog;
	CPDLList<CHRINFO>*	m_pxSList;
	CPDLList<CHRRECT>*	m_pxrcList;
	CWHWilImageData		*m_pxImage;
	CMirButton			**m_pBtn;
	POINT				m_PosName;
	BOOL				m_fIsButtonDown;
	INT					m_nChrCount;
	INT					m_nAnimationCount;
	INT					m_nBlend;
	INT					m_nRoll;
};
