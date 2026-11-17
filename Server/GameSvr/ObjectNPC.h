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
    struct StockItem { DWORD id; _TGENERALITEMRCD record; };
    DWORD m_nextStockId = 1;
    struct Good {
        int standard, stock, price, targetStock;
        ULONGLONG refillInterval, lastRefill;
        std::vector<_TGENERALITEMRCD> returnedItems;
        std::vector<StockItem> items;
    };
    std::vector<Good> m_goods;
    bool m_goodsLoaded = false;
    int m_pricePercent = 100;
    std::vector<int> m_purchaseTypes;
    bool LoadGoods(); // Caller holds m_goodsMutex.
    void Sale(CPlayerObject* player, _LPTGENERALITEMRCD item, bool commit, int quantity);
    std::mutex m_goodsMutex;
    ULONGLONG m_lastRefillCheck = GetTickCount64();
    void ShowGoods(CPlayerObject* player);
    void SendGoods(CPlayerObject* player); // Caller holds m_goodsMutex.
    void FillStock(Good& good); // Caller holds m_goodsMutex.
    void Details(CPlayerObject* player, const char* name, DWORD offset);
    void Buy(CPlayerObject* player, const char* name, int quantity, DWORD instance);

	virtual WORD	GetThisCharColor() { return _CHAT_COLOR3; }
	virtual void	GetCharName(char *pszCharName) { strcpy(pszCharName, m_szName); }
};
