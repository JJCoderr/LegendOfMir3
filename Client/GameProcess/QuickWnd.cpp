#include "StdAfx.h"
#include "Common/LegacyPanel.h"
#include <cmath>

void CQuickWnd::Init()
{
    CGameWnd::Init(); closePressed=backgroundPressed=false;
    for (auto& slot:slots) slot=Slot{};
}
int CQuickWnd::SlotAt(POINT point) const
{
    for (int i=0;i<6;++i) {
        const RECT hit={m_rcWnd.left+17+42*i,m_rcWnd.top+6,m_rcWnd.left+55+42*i,m_rcWnd.top+44};
        if (PtInRect(&hit,point)) return i;
    }
    return -1;
}
bool CQuickWnd::CloseHit(POINT point)
{
    if (!m_pxWndImage->NewSetIndex(1211)) return false;
    const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
    const RECT hit={m_rcWnd.left+264,m_rcWnd.top+32,m_rcWnd.left+264+info.shWidth,m_rcWnd.top+32+info.shHeight};
    return PtInRect(&hit,point)!=FALSE;
}
BOOL CQuickWnd::OnLButtonDown(POINT point)
{
    closePressed=CloseHit(point); backgroundPressed=!closePressed && SlotAt(point)<0;
    return !backgroundPressed;
}
BOOL CQuickWnd::OnLButtonUp(POINT point)
{
    const bool close=closePressed && CloseHit(point),captured=backgroundPressed || closePressed;
    closePressed=backgroundPressed=false;
    if (!captured && SlotAt(point)>=0) g_xGameProc.m_xInterface.m_xExchangeWnd.CancelReturn();
    return close;
}
void CQuickWnd::ShowQuickWnd()
{
    const int x=m_rcWnd.left,y=m_rcWnd.top;
    if (m_pxWndImage->NewSetIndex(1210)) {
        const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        DrawSelectionFrame(*m_pxWndImage,1210,x-(info.shWidth-280)/2,y-(info.shHeight-48)/2,SelectionBlend::Normal,false);
    }
    DrawSelectionFrame(*m_pxWndImage,closePressed?1212:1211,x+264,y+32,SelectionBlend::Normal,false);
    auto& icons=g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY];
    for (int i=0;i<6;++i) if (slots[i].present && icons.NewSetIndex(slots[i].icon)) {
        const auto info=*icons.m_lpstNewCurrWilImageInfo;
        DrawSelectionFrame(icons,slots[i].icon,x+17+42*i+19-info.shWidth/2,y+25-info.shHeight/2,SelectionBlend::Normal,false);
    }
    HDC dc; auto* surface=g_xMainWnd.GetBackBuffer(); if (FAILED(surface->GetDC(&dc))) return;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font=g_xMainWnd.CreateGameFont(face,9); HGDIOBJ old=SelectObject(dc,font); SetBkMode(dc,TRANSPARENT); SetTextColor(dc,0xffffff);
    for (int i=0;i<6;++i) if (slots[i].present && slots[i].stackMode>=2 && slots[i].count>0) {
        const int offset=int(5*(5-std::log10(double(slots[i].count))));
        const std::string text=std::to_string(slots[i].count);
        const RECT box={x+17+42*i+offset,y+29,x+55+42*i+offset,y+45};
        const int saved=SaveDC(dc); IntersectClipRect(dc,box.left,box.top,box.right,box.bottom);
        TextOutA(dc,box.left,box.top,text.data(),int(text.size())); RestoreDC(dc,saved);
    }
    SelectObject(dc,old); DeleteObject(font); surface->ReleaseDC(dc);
}
