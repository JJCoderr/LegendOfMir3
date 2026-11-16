#include "StdAfx.h"
#include "Common/LegacyPanel.h"
#include <cstdlib>

void CExchangeWnd::Init()
{
    CGameWnd::Init(); partner.clear();
    for (int side=0;side<2;++side) {
        gold[side]=0; first[side]=0;
        for (auto& item:offers[side]) item=Offer{};
    }
    captured=pendingSlot=returnSlot=-1; confirmed=confirmPressed=goldPending=returnPending=headerPressed=false;
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
bool CExchangeWnd::SubmitGold(const char* text,unsigned balance)
{
    if (!m_bActive || confirmed || goldPending || gold[0]) return false;
    if (!*text) return false;
    for (const char* digit=text;*digit;++digit) if (*digit<'0' || *digit>'9') return false;
    const unsigned long long parsed=std::strtoull(text,NULL,10);
    if (!parsed || parsed>balance) return false;
    const unsigned amount=unsigned(parsed);
    goldPending=true;
    _TDEFAULTMESSAGE message;
    // Mir3 CM_TRADE_GOLD carries the amount in recognition; balance stays server-owned.
    fnMakeDefMessage(&message,1029,INT(amount),0,0,0); g_xClientSocket.SendPacket(&message,NULL);
    return true;
}
int CExchangeWnd::OfferAt(POINT point) const
{
    const int x=point.x-m_rcWnd.left-14,y=point.y-m_rcWnd.top-56;
    if (x<0 || x>=190 || y<0 || y>=190) return -1;
    const int column=x/38,row=first[0]+y/38;
    for (int slot=0;slot<200;++slot) {
        const auto& item=offers[0][slot];
        if (item.present && item.makeIndex && column>=slot%5 && column<slot%5+item.width &&
            row>=slot/5 && row<slot/5+item.height) return slot;
    }
    return -1;
}
bool CExchangeWnd::RequestReturn()
{
    if (!m_bActive || returnSlot<0 || returnPending) return false;
    auto& item=offers[0][returnSlot];
    returnPending=true;
    g_xClientSocket.SendItemIndex(1027,INT(item.makeIndex),const_cast<char*>(item.name.c_str()));
    return true;
}
bool CExchangeWnd::CancelReturn()
{
    if (!m_bActive || returnSlot<0 || returnPending || pendingSlot>=0) return false;
    const auto& item=offers[0][returnSlot];
    g_xSound.PlayItemEffect(item.stdMode,item.record31,item.name.c_str());
    returnSlot=-1;
    return true;
}
void CExchangeWnd::ShowHeld(POINT point)
{
    if (!m_bActive || returnSlot<0 || point.x<0 || point.x>=800 || point.y<0 || point.y>=600) return;
    auto& icons=g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY];
    const auto& item=offers[0][returnSlot];
    if (!icons.NewSetIndex(item.icon)) return;
    const auto info=*icons.m_lpstNewCurrWilImageInfo;
    DrawSelectionFrame(icons,item.icon,point.x-(info.shWidth+1)/2,point.y-(info.shHeight+1)/2,SelectionBlend::Normal,false);
}
BOOL CExchangeWnd::OnLButtonDown(POINT point)
{
    headerPressed=false;
    if (ScrollInput(0,point)) return TRUE;
    confirmPressed=ConfirmHit(point);
    headerPressed=!confirmPressed && point.y<m_rcWnd.top+56;
    return confirmPressed || point.y>=m_rcWnd.top+56;
}
BOOL CExchangeWnd::OnLButtonUp(POINT point)
{
    if (ScrollInput(2,point)) return FALSE;
    const bool wasCaptured=headerPressed || confirmPressed; headerPressed=false;
    const bool activate=confirmPressed && ConfirmHit(point) && !confirmed && !goldPending && pendingSlot<0 && returnSlot<0;
    confirmPressed=false;
    if (activate) {
        confirmed=true;
        // Mir3 protocol/message_ids.inc: CM_TRADE_CONFIRM, no payload.
        g_xClientSocket.SendQueryMsg(1030);
    }
    if (wasCaptured) return FALSE;
    const RECT grid={m_rcWnd.left+14,m_rcWnd.top+56,m_rcWnd.left+204,m_rcWnd.top+246};
    if (!confirmed && pendingSlot<0 && !returnPending && !g_xGameProc.m_xInterface.m_stCommonItem.bSetted && PtInRect(&grid,point))
        returnSlot=OfferAt(point);
    const RECT amount={m_rcWnd.left+34,m_rcWnd.top+270,m_rcWnd.left+156,m_rcWnd.top+304};
    if (!confirmed && !goldPending && pendingSlot<0 && returnSlot<0 && !gold[0] &&
        !g_xGameProc.m_xInterface.m_stCommonItem.bSetted && g_xGameProc.m_xMyHero.m_nGlod && PtInRect(&amount,point))
        g_xGameProc.m_xInterface.OpenNameDialog(6);
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
