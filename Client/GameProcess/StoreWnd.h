#pragma once
#include <string>
#include <vector>
class CStoreWnd : public CGameWnd {
    int Hit(POINT point);
    bool Scroll(int kind,POINT point);
    void Layout();
    void Request(int opcode,const std::string& name,DWORD instance=0,WORD quantity=0);
public:
    struct Good { std::string name; int variants=0,price=0,stock=0,icon=0,requirementType=0,requirement=0; };
    struct Offer { std::string name; int icon=0,price=0; DWORD id=0; };
    std::vector<Good> goods;
    std::vector<Offer> offers;
    int actor=0,first=0,highlighted=-1,selected=-1,offerIndex=-1,page=0,bulk=-1,pressed=-1;
    bool details=false,loading=false,scrollCaptured=false;
    double fraction=0;
    void Init() override;
    void Reset();
    void SetNPC(int id) { actor=id; }
    void CreateStoreWnd(INT id,CWHWilImageData* image,INT,INT,INT,INT,INT,BOOL);
    void ShowStoreWnd();
    bool SetGoods(int id,const char* text,int count);
    void SetOffers(std::vector<Offer> list,int offset);
    bool SetDetailRecords(const char* body,int count,int offset);
    void OpenCategory();
    void CloseOffers();
    void BuyOffer();
    void BuyBulk(const char* quantity);
    BOOL OnLButtonDown(POINT point);
    BOOL OnLButtonUp(POINT point);
    void OnDoubleClick(POINT point);
    void OnMouseMove(POINT point) { Scroll(1,point); }
    void OnScrollDown();
    void OnScrollUp();
    void Receive(const _TDEFAULTMESSAGE& header,const char* body);
};
