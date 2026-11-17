#pragma once
class CQuickWnd : public CGameWnd {
public:
    struct Slot { bool present=false; int icon=0,count=0,stackMode=0; CItem item; };
    Slot slots[6]={};
    Slot heldReference;
    int heldSlot=-1;
    int Release(int slot);
    void Activate(int slot,DWORD now,bool mouse=false);
    static void Notice(int resource,int slot,const char* name);
    void ClearHeldReference();
    int Find(const char* name) const;
    static Slot FromItem(const CItem& item);
    bool closePressed=false,backgroundPressed=false;
    void Init() override;
    int SlotAt(POINT point) const;
    bool CloseHit(POINT point);
    void ShowQuickWnd();
    BOOL OnLButtonDown(POINT point);
    BOOL OnLButtonUp(POINT point);
};
