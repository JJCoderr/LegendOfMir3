#ifndef _CITEM_H
#define	_CITEM_H
#include <string>
#include <vector>


#pragma pack(1)
typedef struct tagSTANDARDITEM
{
	CHAR			szName[15];

	BYTE			bStdMode;          
	BYTE			bShape;            
	BYTE			bWeight;           
	BYTE			bAniCount;
	
	BYTE			sSpecialPower;           

	BYTE			bNeedIdentify;

	WORD			wLooks;            
	WORD			wDuraMax;

	WORD			wAC;			
	WORD			wMAC;			

	WORD			wDC;			
	WORD			wMC;			

	WORD			wSC;			
	BYTE			bNeed;             
	BYTE			bNeedLevel;        

	int				nPrice;
}STANDARDITEM, *LPSTANDARDITEM;

typedef struct tagCLIENTITEMRCD
{
	STANDARDITEM	stStdItem;
	int				nMakeIndex;
	WORD			nDura;
	WORD			nDuraMax;
}CLIENTITEMRCD, *LPCLIENTITEMRCD;

typedef struct tagGROUNDITEM
{
	INT				nRecog;
	SHORT			shTileX;
	SHORT			shTileY;
	WORD			wLooks;
	CHAR			szItemName[40];
}GROUNDITEM, *LPGROUNDITEM;
#pragma pack(8)


class CItem
{ 
public:
	CItem();
	~CItem();

private:
public:
	BYTE			m_bAttr;
	SHORT			m_shCellNum;
	CLIENTITEMRCD	m_stItemInfo;
    struct HintLine { std::string text; DWORD color; };
    std::vector<HintLine> GeneralHint(int profession,int level,bool shop=false) const;
    void ShowGeneralHint(int x,int y) const;
    std::vector<HintLine> ArmorHint(int level,int attack,const BYTE* elements,bool shop=false) const;
    HintLine RequirementHint(int level,int attack,const BYTE* elements) const;
    std::vector<HintLine> WeaponHint(int level,int attack,const BYTE* elements,bool shop=false) const;
    WORD m_count=1;
    BYTE m_stackMode=0;
    BYTE m_serverRecord[100]={};
    char m_serverName[31]={};
    const char* Name() const { return m_serverName[0]?m_serverName:m_stItemInfo.stStdItem.szName; }
    bool SetMir3Item(const BYTE* record,SHORT cell,bool allowEmptyName=false);
    void SetCount(WORD count) {m_count=count;m_serverRecord[71]=BYTE(count);m_serverRecord[72]=BYTE(count>>8); }

protected:
public:
	BOOL		SetItem(CLIENTITEMRCD stItemInfo, SHORT shInventoryNum);
	BOOL		DrawItem(INT nX, INT nY, BYTE bItemType = _ITEM_TYPE_INVENTORY, const RECT* clip = nullptr);
	VOID		ShowItemStatus(INT nX, INT nY);
public:
};



#endif //_CITEM_H