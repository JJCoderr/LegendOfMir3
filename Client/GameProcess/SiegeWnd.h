#pragma once
#include "SiegeLayout.h"
#include <vector>
#include <map>
class CSiegeWnd : public CGameWnd {
public:
    struct Request { SiegeLayout::Action action; POINT target; CItem item; unsigned quantity=0; };
    bool controlled=false,objectPresent=false,captured=false,dragging=false,entryPressed=false;
    DWORD objectId=0;
    int firstValue=0,secondValue=0,style=0,pressed=-1;
    POINT objectCell={},target={};
    std::vector<Request> requests;
    void Create(CWHWilImageData* image);
    void ReceiveControl(DWORD values,WORD rangeStyle,const char* description);
    void ReceiveStatus(DWORD values);
    void ReceiveObject(DWORD id,POINT cell);
    void ClearObjects();
    void ReleaseControl();
    void Destroy() override;
    void Close(bool clearTarget);
    void Show();
    void Pointer(int kind,POINT point);
    int ButtonAt(POINT point) const;
    int HintAt(POINT point,POINT& anchor) const;
    void Activate(SiegeLayout::Action action);
    bool BeginLoad();
    bool SubmitLoad(const char* text);
    POINT MapOrigin() const;
    RECT EntryBounds() const;
    void ShowEntry();
private:
    std::map<DWORD,POINT> objects;
};
