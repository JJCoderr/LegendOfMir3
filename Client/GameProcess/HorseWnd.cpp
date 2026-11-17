#include "StdAfx.h"
#include "Common/LegacyPanel.h"
#include "Common/LoginMessages.h"

namespace {
const int buttons[5][4]={{1221,236,279,1052},{1402,19,232,4100},{1405,77,232,4101},{1408,134,232,4102},{1410,191,232,4103}};
}
void CHorseWnd::Init()
{
    CGameWnd::Init(); pressed=-1; shape=0; m_hovered=false;
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
        g_xGameProc.StartInputDelay(InputDelay::Mount);
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
}
