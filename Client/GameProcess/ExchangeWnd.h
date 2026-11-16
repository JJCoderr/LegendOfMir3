#pragma once
#include <string>
#include <array>

class CExchangeWnd : public CGameWnd {
public:
    static constexpr int RecordBytes=100, SlotsPerSide=200, GridColumns=5, CellSize=38, GridWidth=190, GridTop=56, LocalGridLeft=14, RemoteGridLeft=264;
    struct Offer { bool present=false; int icon=0,width=1,height=1,count=1; unsigned makeIndex=0; std::string name; int stdMode=-1; BYTE record31=0; std::array<BYTE,RecordBytes> record={}; };
    Offer offers[2][200]={};
    std::string partner;
    unsigned gold[2]={};
    int first[2]={},captured=-1,pendingSlot=-1,returnSlot=-1;
    bool confirmed=false,confirmPressed=false,goldPending=false,returnPending=false,headerPressed=false;
    CExchangeWnd() { Init(); }
    void Init() override;
    void Destroy() override { CGameWnd::Destroy(); }
    void CreateExchangeWnd(INT id,CWHWilImageData* image,INT frame,INT x,INT y,INT width,INT height,BOOL movable);
    void ShowExchangeWnd();
    bool ConfirmHit(POINT point);
    bool ScrollInput(int kind,POINT point);
    static std::string Amount(unsigned value);
    bool SubmitGold(const char* text,unsigned balance);
    int OfferAt(POINT point,int side=0) const;
    bool SetOfferRecord(int side,int slot,const BYTE* record);
    bool RequestReturn();
    bool CancelReturn();
    void ShowHeld(POINT point);
    BOOL OnLButtonDown(POINT point);
    BOOL OnLButtonUp(POINT point);
    void OnMouseMove(POINT point) { ScrollInput(1,point); }
};
