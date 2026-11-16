#pragma once
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

class CSkillWnd : public CGameWnd {
public:
    RECT m_keyMenu={};
    int m_keyMenuTarget=0;
    bool OpenKeyMenu(POINT point);
    void ShowKeyMenu();
    void BindMagic(int id,BYTE key);
    static BYTE LegacyKey(int key) { return BYTE(key<=9?'0'+key:'A'+key-10); }
    static int KeyNumber(BYTE key) { return key>='1' && key<='9'?key-'0':key>='A' && key<='C'?key-'A'+10:0; }
    BYTE m_bMyMagicCnt=0;
    LPCLIENTMAGICRCD m_pstMyMagic=nullptr;
    int m_skillTab=0, m_skillScroll=0, m_skillPressed=-1;
    int m_skillSelected[8]={-1,-1,-1,-1,-1,-1,-1,-1};
    bool m_skillScrollCaptured=false;
    DWORD m_skillScrollTime=0;
    ~CSkillWnd() { Destroy(); }
    void Init() override;
    void Destroy() override;
    void SelectNonempty();
    void ShowSkillWnd();
    int SkillButtonAt(POINT point);
    int SkillAt(POINT point);
    bool SkillScrollInput(int kind,POINT point);
    BOOL SetMagicKey(POINT point,BYTE key);
    LPCLIENTMAGICRCD GetMagicByKey(BYTE key);
    BOOL OnKeyDown(WPARAM key,LPARAM flags,POINT point);
    BOOL OnLButtonDown(POINT point);
    BOOL OnLButtonUp(POINT point);
    void OnMouseMove(POINT point) { SkillScrollInput(1,point); }
    void OnScrollDown();
    void OnScrollUp();
};
