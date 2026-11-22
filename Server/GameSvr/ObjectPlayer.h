

// ORZ:
#include "../def/staticArray.h"
#include "../def/_orzex/syncobj.h"

class CPlayerObject : public CCharObject, public CIntLock, CStaticArray< CPlayerObject >::IArrayData
{
public:
	bool					m_bEmpty;
	int						m_nArrIndex;
	BOOL					m_fIsAlive;

	BYTE					m_btBright;
	
	BOOL					m_fAdminMode;

	DWORD					m_dwLastCryTime;
    DWORD m_clientOptions;
    CCharObject* m_dialogMerchant;
    CCharObject* m_shopMerchant;
    CCharObject* m_sellMerchant;
    std::vector<std::string> m_npcScriptFiles;
    std::vector<std::string> m_npcOptions;

	CWHList<CCharObject*>	m_xSlaveObjList;

	CPlayerObject*			m_pGroupOwner;					// 队长（自己建队时指向自己）
	CWHList<CPlayerObject*>	m_xGroupMembers;				// 队伍成员，由队长维护


public:
	void		Constructor();
	bool		IsEmpty();

	CPlayerObject();
	CPlayerObject(CUserInfo* pUserInfo);

	void		Initialize();
	void		SendGroupMembers();

	virtual void		Operate();
	BOOL		DoRideHorse(int nDir, int fRideFlag);

	void		MakeFeature();

	void		HasLevelUp(int nPrevLevel);

	void		WeightChanged();

	void		RecalcHitSpeed();
	void		RecalcAbilitys();
	void		RecalcLevelAbilitys();

	void		SendBagItems();
    void SendEquipmentItems();
	void		SendMyMagics();

	void		SendMapName();
	void		SendNewMap();
	void		SendAddItem(_LPTUSERITEMRCD lpTItemRcd);
    void SendAddGeneralItem(_LPTGENERALITEMRCD item);

	BOOL		EatItem(char *pszMakeIndex);
	BOOL		ReadBook(char *pszMakeIndex);
	int			GetMagicID(char *pszMakeIndex);
	BOOL		ServerGetEatItem(int nItemIndex, char *pszMakeIndex, DWORD instance = 0);
	BOOL		PickUp();

	void		Whisper(char *pszChar, char *pszMsg);

	int			UpdateItemToDB(_LPTUSERITEMRCD lpMakeItemRcd, int nAction);
    bool TradeItemToDB(_LPTUSERITEMRCD item, int action, int gold);
private:
    int WriteItemRecord(_LPTUSERITEMRCD item, int action); // Caller owns the transaction.
public:

	BOOL		CheckTakeOnItem(WORD wWhere, _LPTUSERITEMRCD lpTItemRcd);
	void		ServerGetTakeOnGenItem(WORD wWhere, char *pszItemIndex);
	void		ServerGetTakeOnItem(WORD wWhere, char *pszItemIndex);
	void		ServerGetTakeOffItem(WORD wWhere, char *pszItemIndex);
	void		ServerGetButch(CCharObject* pCharObject, int nX, int nY, int nDir);

	void		ServerGetMagicKeyChange(int nMagicID, int nUseKey);

	BYTE		DayBright();

	int			CalcGetExp(int nTagLevel, int nTagHP);
	void		WinExp(int nExp);
	void		GainExp(int nExp);
	void		AddBodyLuck(double fLuck);

	BOOL		ProcessForUserSaid(char *pszData);

	void		TrainSkill(_LPTHUMANMAGICRCD lptMagicRcd, int nTrain);
	BOOL		DoSpell(_LPTHUMANMAGICRCD	lptMagicRcd, int TargetX, int TargetY, CCharObject* pTargetObject);
	BOOL		SpellXY(int nMagicID, int nTargetX, int nTargetY, int nTargetObj);
	
	BOOL		MakeSlave(int nMonRace, int nX, int nY, int nLevel, int nMax, DWORD dwRoyaltySec);

	virtual BOOL	IsProperTarget(CCharObject* pCharObject);

	virtual void	Run();
	virtual WORD	GetThisCharColor();
	virtual void	GetCharName(char *pszCharName);

	// For Administrator
	BOOL		m_fIsCapture;
	FILE		*m_hCaptureFile;

protected:
	BOOL		ProcessForAdminSaid(char *pszMsg);

	void		CmdRandomSpaceMove(char *pszParam1);
	void		CmdFreeSpaceMove(char *pszParam1, char *pszParam2, char *pszParam3);
	void		CmdCallMakeMonster(char *pszParam1, char *pszParam2);
	void		CmdCallMakeMonsterXY(char *pszParam1, char *pszParam2, char *pszParam3, char *pszParam4);
	void		CmdMakeItem(char *pszParam1, char *pszParam2);
	void		CmdDyeingHair(char *pszParam1);
	void		CmdDyeingWear(char *pszParam1);
	void		CmdHairStyle(char *pszParam1);
	BOOL		CmdChangeJob(char *pszParam1);
	void		CmdMakeFullSkill(char *pszParam1, char *pszParam2);
	void		CmdSendMonsterLevelInfos();
	void		CmdChangeItemPrefix(char *pszParam1, char *pszParam2);
	void		CmdCallMakeSlaveMonster(char *pszParam1, char *pszParam2);
};
