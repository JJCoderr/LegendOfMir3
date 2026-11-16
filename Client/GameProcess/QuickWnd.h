#pragma once
class CQuickWnd : public CGameWnd {
public:
    struct Slot { bool present=false; int icon=0,count=0,stackMode=0; };
    Slot slots[6]={};
    bool closePressed=false,backgroundPressed=false;
    void Init() override;
    int SlotAt(POINT point) const;
    bool CloseHit(POINT point);
    void ShowQuickWnd();
    BOOL OnLButtonDown(POINT point);
    BOOL OnLButtonUp(POINT point);
};
