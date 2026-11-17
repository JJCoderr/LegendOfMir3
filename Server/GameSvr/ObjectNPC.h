#include <mutex>

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
    struct Good { int standard, stock, price, targetStock; ULONGLONG refillInterval, lastRefill; };
    std::vector<Good> m_goods;
    bool m_goodsLoaded = false;
    std::mutex m_goodsMutex;
    ULONGLONG m_lastRefillCheck = GetTickCount64();
    void ShowGoods(CPlayerObject* player);
    void SendGoods(CPlayerObject* player); // Caller holds m_goodsMutex.
    void Buy(CPlayerObject* player, const char* name, int quantity);

	virtual WORD	GetThisCharColor() { return _CHAT_COLOR3; }
	virtual void	GetCharName(char *pszCharName) { strcpy(pszCharName, m_szName); }
};
