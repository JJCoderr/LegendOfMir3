#include "StdAfx.h"
#include "Common/LegacyPanel.h"

void CExchangeWnd::Init()
{
    CGameWnd::Init(); partner.clear();
    for (int side=0;side<2;++side) {
        gold[side]=0; first[side]=0;
        for (auto& item:offers[side]) item=Offer{};
    }
    captured=pendingSlot=returnSlot=-1; confirmed=confirmPressed=goldPending=false;
}
void CExchangeWnd::CreateExchangeWnd(INT id,CWHWilImageData* image,INT frame,INT x,INT y,INT width,INT height,BOOL movable)
{
    CreateGameWnd(id,image,frame,movable,x,y,width,height);
}
std::string CExchangeWnd::Amount(unsigned value)
{
    std::string result=std::to_string(value);
    for (int i=int(result.size())-3;i>0;i-=3) result.insert(i,1,',');
    return result;
}
bool CExchangeWnd::ConfirmHit(POINT point)
{
    if (!m_pxWndImage->NewSetIndex(1394)) return false;
    const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
    RECT hit={m_rcWnd.left+226,m_rcWnd.top+279,m_rcWnd.left+226+info.shWidth,m_rcWnd.top+279+info.shHeight};
    return PtInRect(&hit,point)!=FALSE;
}
bool CExchangeWnd::ScrollInput(int kind,POINT point)
{
    if (kind==2) { const bool was=captured>=0; captured=-1; return was; }
    if (!m_pxWndImage->NewSetIndex(1225)) return false;
    const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
    const int x=point.x-m_rcWnd.left,y=point.y-m_rcWnd.top;
    if (kind==1 && captured>=0) {
        first[captured]=int(max(0.f,min(1.f,float(y-58)/(158+info.shHeight)))*33); return true;
    }
    if (kind!=0 || y<58 || y>=58+158+info.shHeight) return false;
    const int track[2]={206,457};
    for (int side=0;side<2;++side) {
        if (x<track[side] || x>=track[side]+info.shWidth) continue;
        const int thumb=58+first[side]*158/33;
        if (y>=thumb && y<thumb+info.shHeight) captured=side;
        else first[side]=max(0,min(33,first[side]+(y<thumb?-6:6)));
        return true;
    }
    return false;
}
BOOL CExchangeWnd::OnLButtonDown(POINT point)
{
    if (ScrollInput(0,point)) return TRUE;
    confirmPressed=ConfirmHit(point);
    return confirmPressed || point.y>=m_rcWnd.top+56;
}
BOOL CExchangeWnd::OnLButtonUp(POINT point)
{
    if (ScrollInput(2,point)) return FALSE;
    const bool activate=confirmPressed && ConfirmHit(point) && !confirmed && !goldPending && pendingSlot<0 && returnSlot<0;
    confirmPressed=false;
    if (activate) {
        confirmed=true;
        // Mir3 protocol/message_ids.inc: CM_TRADE_CONFIRM, no payload.
        g_xClientSocket.SendQueryMsg(1030);
    }
    return FALSE;
}
void CExchangeWnd::ShowExchangeWnd()
{
    const int x=m_rcWnd.left,y=m_rcWnd.top;
    if (m_pxWndImage->NewSetIndex(1390)) {
        const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        DrawSelectionFrame(*m_pxWndImage,1390,x-(info.shWidth-494)/2,y-(info.shHeight-330)/2,SelectionBlend::Normal,false);
    }
    auto& icons=g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY];
    for (int side=0;side<2;++side) {
        const int left=x+(side?264:14); RECT clip={left,y+56,left+190,y+246};
        DrawSelectionFrame(*m_pxWndImage,1225,x+(side?457:206),y+58+first[side]*158/33,SelectionBlend::Normal,false);
        for (int slot=0;slot<5*(first[side]+5);++slot) {
            const auto& item=offers[side][slot]; if (!item.present || slot/5+item.height<=first[side] || !icons.NewSetIndex(item.icon)) continue;
            const auto info=*icons.m_lpstNewCurrWilImageInfo;
            const int px=left+38*(slot%5)+(38*item.width-info.shWidth)/2;
            const int py=y+56+38*(slot/5-first[side])+(38*item.height-info.shHeight)/2;
            DrawSelectionFrame(icons,item.icon,px,py,side==0 && slot==returnSlot?SelectionBlend::Alpha:SelectionBlend::Normal,false,128,&clip);
        }
    }
    HDC dc; auto* surface=g_xMainWnd.GetBackBuffer(); if (FAILED(surface->GetDC(&dc))) return;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font=g_xMainWnd.CreateGameFont(face,9); HGDIOBJ old=SelectObject(dc,font); SetBkMode(dc,TRANSPARENT);
    const auto text=[&](const std::string& value,int left,int top,int width,int height,COLORREF color,bool centered) {
        RECT box={x+left,y+top,x+left+width,y+top+height}; SetTextColor(dc,color);
        if (centered) DrawTextA(dc,value.c_str(),-1,&box,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        else { const int saved=SaveDC(dc); IntersectClipRect(dc,box.left,box.top,box.right,box.bottom); TextOutA(dc,box.left,box.top,value.data(),int(value.size())); RestoreDC(dc,saved); }
    };
    text(g_xGameProc.m_xMyHero.m_szName,0,0,228,30,0xfac8c8,true); text(partner,247,0,120,30,0xfac8c8,true);
    for (int side=0;side<2;++side) {
        const int left=side?264:14;
        text(Amount(gold[side]),side?350:70,289,side?85:113,25,0xfac8c8,true);
        for (int slot=0;slot<5*(first[side]+5);++slot) {
            const auto& item=offers[side][slot]; if (!item.present || item.count<=1) continue;
            const int px=left+38*(slot%5)+22,py=56+38*(slot/5-first[side])+23;
            if (px>=left && px+16<=left+190 && py>=56 && py+15<=246) text(std::to_string(item.count),px,py,16,15,0xffffff,false);
        }
    }
    SelectObject(dc,old); DeleteObject(font); surface->ReleaseDC(dc);
    DrawSelectionFrame(*m_pxWndImage,confirmed?1394:confirmPressed?1396:1392,x+226,y+279,SelectionBlend::Normal,false);
}
