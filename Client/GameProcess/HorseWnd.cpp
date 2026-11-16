#include "StdAfx.h"
#include "Common/LegacyPanel.h"
#include "Common/LoginMessages.h"

namespace {
const int buttons[5][4]={{1221,236,279,1052},{1402,19,232,4100},{1405,77,232,4101},{1408,134,232,4102},{1410,191,232,4103}};
}
void CHorseWnd::Init()
{
    CGameWnd::Init(); pressed=-1; shape=0; m_hovered=false;
    m_hintExpanding=true; m_hintHeight=6; m_previousHint.clear();
}
void CHorseWnd::CreateHorseWnd(INT id,CWHWilImageData* image,INT frame,INT x,INT y,INT width,INT height,BOOL movable)
{
    CreateGameWnd(id,image,frame,movable,x,y,width,height);
}
int CHorseWnd::ButtonAt(POINT point)
{
    for (int i=0;i<5;++i) {
        if (!m_pxWndImage->NewSetIndex(buttons[i][0])) continue;
        const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        const int x=m_rcWnd.left+buttons[i][1],y=m_rcWnd.top+buttons[i][2];
        const RECT hit={x,y,x+info.shWidth,y+info.shHeight}; if (PtInRect(&hit,point)) return i;
    }
    return -1;
}
int CHorseWnd::Command(int button) const
{
    switch (button) {
    case 1:return shape?0:4106;
    case 2:return shape?4105:0;
    case 3:return 4104;
    case 4:return 4105;
    default:return 0;
    }
}
BOOL CHorseWnd::OnLButtonDown(POINT point) { pressed=ButtonAt(point); return pressed>=0; }
BOOL CHorseWnd::OnLButtonUp(POINT point)
{
    const int hit=ButtonAt(point); const bool clicked=hit>=0 && hit==pressed; pressed=-1;
    if (!clicked) return FALSE;
    g_xSound.PlayLobbyEffect("105.wav");
    if (hit==0) return TRUE;
    if (const int id=Command(hit)) {
        static CLoginMessages captions; static const bool loaded=captions.Load("./CmList.dat");
        std::string command=captions.Get(id); g_xClientSocket.SendSay(const_cast<char*>(command.c_str()));
    }
    return FALSE;
}
int CHorseWnd::HintAt()
{
    const int hit=m_hovered?ButtonAt(m_pointer):-1;
    return hit>=0 && hit!=pressed?buttons[hit][3]:0;
}
void CHorseWnd::ShowHorseWnd()
{
    if (m_pxWndImage->NewSetIndex(1400)) {
        const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        DrawSelectionFrame(*m_pxWndImage,1400,m_rcWnd.left-(info.shWidth-276)/2,m_rcWnd.top-(info.shHeight-322)/2,SelectionBlend::Alpha,false,220);
    }
    for (int i=0;i<5;++i) DrawSelectionFrame(*m_pxWndImage,buttons[i][0]+(pressed==i),m_rcWnd.left+buttons[i][1],m_rcWnd.top+buttons[i][2],SelectionBlend::Normal,false);
    ShowHint();
}
void CHorseWnd::ShowHint()
{
    const int hint=HintAt(); if (!hint) return;
    static CLoginMessages captions; static const bool loaded=captions.Load("./CmList.dat");
    const auto& caption=captions.Get(hint); if (caption.empty()) return;
    const int button=ButtonAt(m_pointer); const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
    const int anchorX=m_rcWnd.left+buttons[button][1]+info.shWidth,anchorY=m_rcWnd.top+buttons[button][2]+info.shHeight;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0}; HFONT font=g_xMainWnd.CreateGameFont(face,9);
    HDC measure=CreateCompatibleDC(NULL); HGDIOBJ old=SelectObject(measure,font); SIZE size={};
    GetTextExtentPoint32A(measure,caption.data(),int(caption.size()),&size); SelectObject(measure,old); DeleteDC(measure);
    if (m_previousHint!=caption) { m_previousHint=caption; m_hintExpanding=true; }
    const int width=size.cx+12,fullHeight=size.cy+4;
    const int height=m_hintExpanding?min(++m_hintHeight,fullHeight):fullHeight;
    if (m_hintExpanding && m_hintHeight>=fullHeight) { m_hintExpanding=false; m_hintHeight=6; }
    const int left=min(anchorX,800-width); RECT box={left,anchorY-height,left+width,anchorY};
    DrawLegacyPanel(box,RGB(255,255,150),255,RGB(0,0,0));
    HDC dc; auto* surface=g_xMainWnd.GetBackBuffer();
    if (SUCCEEDED(surface->GetDC(&dc))) {
        old=SelectObject(dc,font); SetBkMode(dc,TRANSPARENT); SetTextColor(dc,0);
        DrawTextA(dc,caption.data(),int(caption.size()),&box,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        SelectObject(dc,old); surface->ReleaseDC(dc);
    }
    DeleteObject(font);
}
