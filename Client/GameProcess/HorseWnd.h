#pragma once
#include <string>

class CHorseWnd : public CGameWnd {
    POINT m_pointer={};
    bool m_hovered=false,m_hintExpanding=true;
    int m_hintHeight=6;
    std::string m_previousHint;
    void ShowHint();
public:
    int pressed=-1;
    BYTE shape=0;
    CHorseWnd() { Init(); }
    void Init() override;
    void Destroy() override { CGameWnd::Destroy(); }
    void CreateHorseWnd(INT id,CWHWilImageData* image,INT frame,INT x,INT y,INT width,INT height,BOOL movable);
    void ShowHorseWnd();
    int ButtonAt(POINT point);
    int Command(int button) const;
    int HintAt();
    void UpdatePointer(POINT point,bool visible) { m_pointer=point; m_hovered=visible; }
    BOOL OnLButtonDown(POINT point);
    BOOL OnLButtonUp(POINT point);
};
