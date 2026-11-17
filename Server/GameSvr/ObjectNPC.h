
/*
*/

class CNPCObject : public CCharObject
{
public:
	DWORD		m_dwRunTime;
	DWORD		m_dwRunNextTick;

	CNPCObject();

	virtual void	GetCharName(char *pszCharName) { return; }
};

/*
*/

class CMerchantObject : public CCharObject
{
public:
	DWORD		m_dwRunTime;
	DWORD		m_dwRunNextTick;

	int			m_nIndex;

	CMerchantObject();

	void RunRace();
    void Say(CPlayerObject* player, const char* section);

	virtual WORD	GetThisCharColor() { return _CHAT_COLOR3; }
	virtual void	GetCharName(char *pszCharName) { strcpy(pszCharName, m_szName); }
};
