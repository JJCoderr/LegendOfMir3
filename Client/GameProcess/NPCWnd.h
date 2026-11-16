#pragma once
#include <string>
#include <vector>
class CNPCWnd : public CGameWnd {
    std::vector<DWORD> background;
    int cachedStrips=0,arrowFrame=0;
    POINT pointer={};
    bool hovered=false;
    void DrawBackground();
    bool Hit(int frame,int x,int y,POINT point);
public:
    struct Row { std::string text,command; int x,line,width,height; COLORREF color; int option; };
    std::vector<Row> rows;
    int m_nNpcId=0,promptActor=0,first=0,extraRows=0,scrollLimit=0,pressed=-1,arrowPressed=0;
    bool canClose=true,closePressed=false,scrollEnabled=false,downLit=false;
    DWORD lastAction=0;
    std::string prompt,promptCommand;
    void CreateNPCChatWnd(INT id,CWHWilImageData* image,INT frame,INT x,INT y,INT width,INT height,BOOL movable);
    void Init() override { CGameWnd::Init(); ResetDialog(); background.clear(); cachedStrips=0; lastAction=0; }
    void ResetDialog();
    void SetScript(int actor,const char* script);
    void SetNPC(int id) { m_nNpcId=id; }
    int Height() const { return 204+20*max(1,extraRows); }
    RECT TextBounds(const Row& row) const;
    int OptionAt(POINT point) const;
    void ShowNPCChatWnd();
    BOOL OnLButtonDown(POINT point);
    BOOL OnLButtonUp(POINT point);
    void OnMouseMove(POINT point) { pointer=point; }
    void UpdatePointer(POINT point,bool visible) { pointer=point; hovered=visible; }
    void OnScrollDown() { if (scrollEnabled) first=max(0,first-1); }
    void OnScrollUp() { if (scrollEnabled) first=min(scrollLimit,first+1); }
    BOOL OnMsgInputted() { return FALSE; }
    void SubmitPrompt(const char* text);
};
