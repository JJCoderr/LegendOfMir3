#pragma once
#include <string>

class CExchangeWnd : public CGameWnd {
public:
    struct Offer { bool present=false; int icon=0,width=1,height=1,count=1; };
    Offer offers[2][200]={};
    std::string partner;
    unsigned gold[2]={};
    int first[2]={},captured=-1,pendingSlot=-1,returnSlot=-1;
    bool confirmed=false,confirmPressed=false,goldPending=false;
    CExchangeWnd() { Init(); }
    void Init() override;
    void Destroy() override { CGameWnd::Destroy(); }
    void CreateExchangeWnd(INT id,CWHWilImageData* image,INT frame,INT x,INT y,INT width,INT height,BOOL movable);
    void ShowExchangeWnd();
    bool ConfirmHit(POINT point);
    bool ScrollInput(int kind,POINT point);
    static std::string Amount(unsigned value);
    BOOL OnLButtonDown(POINT point);
    BOOL OnLButtonUp(POINT point);
    void OnMouseMove(POINT point) { ScrollInput(1,point); }
};
