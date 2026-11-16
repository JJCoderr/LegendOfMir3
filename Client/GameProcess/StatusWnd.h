#ifndef _CSTATUSWND_H
#define _CSTATUSWND_H
#include <string>
#include "SkillLayout.h"



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
    bool m_nativeClosePressed=false;
    bool NativeCloseHit(POINT point);
    void ShowNativeStatus();
    void ShowNativeHint();
    POINT m_nativePointer={};
    bool m_showNativeHint=false;
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
    RECT m_statusPosition={0,0,328,466}, m_skillPosition={348,0,708,488};
    int m_skillTab=0, m_skillScroll=0, m_skillPressed=-1;
    int m_skillSelected[8]={-1,-1,-1,-1,-1,-1,-1,-1};
    bool m_skillScrollCaptured=false;
    DWORD m_skillScrollTime=0;
    int SkillButtonAt(POINT point);
    int SkillAt(POINT point);
    bool SkillScrollInput(int kind,POINT point);
    static int EquipmentPreviewFrame(int mode,int slot);
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

	__inline VOID SetStatusWndType(BYTE bType)
	{
        if ((m_bType==_TYPE_SETMAGIC)!=(bType==_TYPE_SETMAGIC)) {
            if (m_bType==_TYPE_SETMAGIC) { m_skillPosition=m_rcWnd; m_rcWnd=m_statusPosition; }
            else { m_statusPosition=m_rcWnd; m_rcWnd=m_skillPosition; }
            m_skillPressed=-1; m_skillScrollCaptured=false;
        }
        if (bType==_TYPE_SETMAGIC && m_bMyMagicCnt) {
            bool populated=false;
            for (int i=0;i<m_bMyMagicCnt;++i) if (SkillLayout::category(m_pstMyMagic[i].stStdMagic.wMagicID)==m_skillTab) populated=true;
            if (!populated) {
                int first=7;
                for (int i=0;i<m_bMyMagicCnt;++i) first=min(first,SkillLayout::category(m_pstMyMagic[i].stStdMagic.wMagicID));
                m_skillTab=first;
            }
        }
        m_bType = bType;
		switch ( m_bType )
		{
        case _TYPE_EQUIP:
        case _TYPE_STATUS:
            m_nFrameImgIdx=1280; m_nWidth=328; m_nHeight=466;
            m_rcWnd.right=m_rcWnd.left+328; m_rcWnd.bottom=m_rcWnd.top+466;
            break;
		case _TYPE_SETMAGIC:
			m_nFrameImgIdx=1620; m_nWidth=360; m_nHeight=488;
            m_rcWnd.right=m_rcWnd.left+360; m_rcWnd.bottom=m_rcWnd.top+488;
			break;
		}
	}
	__inline BYTE GetStatusWndType()
	{
		return m_bType;
	}

	VOID	CreateStatusWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove);
	VOID	ShowStatusWnd();
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


#endif // _CSTATUSWND_H