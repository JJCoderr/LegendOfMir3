#pragma once
#include <string>
#include <vector>
#include "QuestWire.h"

class CQuestWnd : public CGameWnd {
public:
    struct Child { std::string title,summary; bool completed=false; int id=0; };
    struct Entry : Child { std::vector<Child> children; int childCount=0; bool childrenLoaded=false; };
    std::vector<Entry> entries;
    bool listInitialized=false;
    std::vector<_TDEFAULTMESSAGE> requests;
    bool Receive(const _TDEFAULTMESSAGE& header,const char* encoded);
    void Query();
    void Expand();
    void ResetView();
    void SendRequest(WORD opcode,int id=0,WORD parent=0);
    int pressed=-1,selected=-1,expanded=-1,first=0;
    int descriptionParent=-1,descriptionChild=-1,descriptionFirst=0;
    bool scrollCaptured=false;
    CQuestWnd() { Init(); }
    void Init() override;
    void Destroy() override;
    void CreateQuestWnd(INT id,CWHWilImageData* image,INT frame,INT x,INT y,INT width,INT height,BOOL movable);
    void ShowQuestWnd();
    int LastRow() const;
    int EntryAt(int row) const;
    int ThumbY() const;
    const std::string& Description() const;
    int ButtonAt(POINT point);
    bool ScrollInput(int kind,POINT point);
    void Scroll(int delta);
    void Wheel(int delta,POINT point);
    BOOL OnLButtonDown(POINT point);
    BOOL OnLButtonUp(POINT point);
    void OnMouseMove(POINT point) { ScrollInput(1,point); }
    void OnScrollDown();
    void OnScrollUp();
};
