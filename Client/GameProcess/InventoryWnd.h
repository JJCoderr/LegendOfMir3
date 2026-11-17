#ifndef _INVENTORYWND_H
#define _INVENTORYWND_H
#include "BagLayout.h"


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
    bool m_closePressed=false;
    CBagLayout m_layout;
    bool PlaceItem(int id,const CItem& item,const POINT* point=nullptr);
    bool Placement(const CItem& item,const POINT* point,int& x,int& y,int& width,int& height);
    POINT m_placementPointer={};
    bool m_showPlacement=false;
	INT			m_nStartLineNum;					// 背包窗口起始行的编号. (共 10 行, 屏幕上最多可看到 4 行.)
	RECT		m_rcInvenCell[_INVEN_MAX_CELL];		// 背包窗口各单元格的区域。
	RECT		m_rcBeltCell[_BELT_MAX_CELL];

	CGameBtn	m_xInventoryBtn[_MAX_INVEN_BTN];

    DWORD m_scrollTick=0;

public:
    DWORD m_lastUse=0,m_lastRelease=0;
    bool m_usePending=false;
    static bool UseAllowed(const CItem& item,DWORD elapsed);
    bool UseBlocked() const;
    bool RequestUse(DWORD now);
    bool ApplyUseReply(WORD opcode,WORD remaining);
    bool ClosePressed() const { return m_closePressed; }
	ITEMSET		m_stInventoryItem[_MAX_INVEN_ITEM];
	ITEMSET		m_stBeltItem[_MAX_BELT_ITEM];

public:
    enum class ServicePhase { Choosing, Quoting, Quoted, Unavailable, Paying };
    struct Service { int actor=0,mode=0,item=0; DWORD price=0; std::string name; ServicePhase phase=ServicePhase::Choosing; bool pressed=false,stack=false; } service;
    void OpenService(int actor,int mode);
    void SelectServiceItem(int index);
    void QuoteService(DWORD price,bool selling);
    void ConfirmService(int quantity=0);
    void SubmitServiceQuantity(const char* text);
    void ClearService();
    bool ServiceButton(POINT point);
    bool m_scrollCaptured=false;
    bool ScrollInput(int kind, POINT point);
    int ScrollRow() const { return m_nStartLineNum; }
    int SlotAt(POINT point) const;
    bool PreviewPlacement(const CItem& item,POINT point,RECT& visible,bool& fits);
    void UpdatePlacementPointer(POINT point,bool visible) { m_placementPointer=point; m_showPlacement=visible; }
    int ItemAnchor(int id) const { return m_layout.anchors[id]; }
	CInventoryWnd();
	~CInventoryWnd();

	virtual VOID Init();
	virtual VOID Destroy();

	VOID	CreateInventoryWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove);
	VOID	ShowInventoryWnd();
	VOID	ShowItemStatus(LPCLIENTITEMRCD pstItemInfo);
	VOID	ShowInvenItemState();
	VOID	SetInvenBtnInit();

public:
	BOOL	CheckItemPos(CHAR* szName);
	BOOL	SaveItemPos(CHAR* szName);
    void UpdateGold(UINT balance);
    void UpdateStackCount(DWORD instance,WORD quantity);
    bool ReceiveBagRecords(const char* encoded,int count);
    bool ReceiveItemRecord(WORD opcode,const char* encoded);
    bool SaveLayoutFile(const char* path);
    bool LoadLayoutFile(const char* path);

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

	VOID	ChangeInvenWithCommon(INT nInventoryNum, LPCOMMONITEMSET pstCommonItemSet, const POINT* point=nullptr);
	VOID	ChangeBeltWithCommon(INT nBeltInventoryNum, LPCOMMONITEMSET pstCommonItemSet);

	VOID	SetCommonFromInven(INT nInventoryNum, LPCOMMONITEMSET pstCommonItemSet);
	VOID	SetCommonFromBelt(INT nBeltInventoryNum, LPCOMMONITEMSET pstCommonItemSet);

	VOID	SetInvenFromCommon(INT nInventoryNum, LPCOMMONITEMSET pstCommonItemSet, const POINT* point=nullptr);
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



#endif // _INVENTORYWND_H