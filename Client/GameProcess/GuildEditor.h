#pragma once
#include <string>
class CGuildEditor : public CGameWnd {
    HFONT font=NULL;
    HBRUSH background=NULL;
    int fontHeight=0;
    bool updating=false;
    std::string ansi;
    void LayoutEdit();
public:
    HWND edit=NULL;
    bool announcement=true,dragging=false;
    int pressed=-1;
    void Open(bool notice);
    void Changed();
    static std::string AcceptEdit(const std::string& previous,const std::string& next);
    void Destroy() override;
    void Show();
    void Pointer(int kind,POINT point);
    int ButtonAt(POINT point);
    void Close(bool save);
    HBRUSH EditColor(HDC dc);
    static std::string Payload(const std::string& input);
};
