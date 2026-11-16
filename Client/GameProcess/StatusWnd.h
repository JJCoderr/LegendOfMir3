#ifndef _CSTATUSWND_H
#define _CSTATUSWND_H
#include <string>







class CStatusWnd : public CGameWnd
{ 
private:
protected:
    bool m_nativeClosePressed=false;
    bool NativeCloseHit(POINT point);
    void ShowNativeStatus();
    void ShowNativeHint();
    POINT m_nativePointer={};
    bool m_showNativeHint=false;

	RECT				m_rcCell[_EQUIP_MAX_CELL];		// 背包窗口各单元格的区域。



public:
    bool ClosePressed() const { return m_nativeClosePressed; }
	CHAR				m_szEquipItemIdx[_EQUIP_MAX_CELL][12];
	ITEMSET				m_stEquipItem[_EQUIP_MAX_CELL];
	ITEMSET				m_stTakeOnItemSet;				// 将待装备的物品临时保存, 直到收到是否穿戴的消息.


public:
    static int EquipmentPreviewFrame(int mode,int slot);
    bool ReceiveEquipmentRecords(const char* encoded);
    void UpdateDurability(WORD slot,LONG current,WORD maximum);
    std::string m_identityTertiary, m_identityBadge;
    BYTE m_elementValues[3][7]={};
    int ElementValue(int row,int column) const {
        const int value=m_elementValues[row][column];
        return row==0 || value<128 ? value : value-256;
    }
    void UpdatePointer(POINT point,bool visible) { m_nativePointer=point; m_showNativeHint=visible; }
	CStatusWnd();
	~CStatusWnd();

	virtual VOID Init();
	virtual VOID Destory();

	VOID	CreateStatusWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove);
	VOID	ShowStatusWnd();

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


	BOOL	OnLButtonDown(LPCOMMONITEMSET pstItemSet, POINT ptMouse);
	BOOL	OnLButtonUp(LPCOMMONITEMSET pstItemSet, POINT ptMouse);
};


#endif // _CSTATUSWND_H