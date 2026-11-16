#include "StdAfx.h"
#include "Common/LegacyPanel.h"
#include <cmath>
#include "Common/LoginMessages.h"

void CQuickWnd::Init()
{
    CGameWnd::Init(); closePressed=backgroundPressed=false; heldSlot=-1; heldReference=Slot{};
    for (auto& slot:slots) slot=Slot{};
}
CQuickWnd::Slot CQuickWnd::FromItem(const CItem& item)
{
    Slot result; result.present=true; result.icon=item.m_stItemInfo.stStdItem.wLooks;
    result.count=item.m_count; result.stackMode=item.m_stackMode; result.item=item; return result;
}
int CQuickWnd::Find(const char* name) const
{
    for (int i=0;i<6;++i) if (slots[i].present && !strcmp(slots[i].item.m_stItemInfo.stStdItem.szName,name)) return i;
    return -1;
}
void CQuickWnd::ClearHeldReference()
{
    if (heldSlot<0) return;
    ZeroMemory(&g_xGameProc.m_xInterface.m_stCommonItem,sizeof(COMMONITEMSET));
    heldSlot=-1; heldReference=Slot{};
}
int CQuickWnd::Release(int slot)
{
    auto& ui=g_xGameProc.m_xInterface; auto& held=ui.m_stCommonItem;
    if (slot<0 || slot>=6 || held.bWaitResponse) return -1;
    const auto take=[&](const Slot& source) {
        heldReference=source; heldSlot=slot;
        ZeroMemory(&held,sizeof(held)); held.bSetted=held.bIsBeltItem=TRUE;
        held.xItem=source.item; held.xItem.m_shCellNum=slot;
    };
    if (!held.bSetted) {
        if (!slots[slot].present) return -1;
        take(slots[slot]); slots[slot]=Slot{}; return 0;
    }
    static CLoginMessages captions; static const bool loaded=captions.Load("./CmList.dat");
    const auto& record=held.xItem.m_stItemInfo;
    if (held.bIsEquipItem || !strcmp(record.stStdItem.szName,captions.Get(1050).c_str()) ||
        (record.stStdItem.bStdMode!=0 && record.stStdItem.bStdMode!=3)) return -1;
    const bool fromQuick=heldSlot>=0;
    const Slot incoming=fromQuick?heldReference:FromItem(held.xItem);
    if (slots[slot].present) {
        if (!strcmp(slots[slot].item.m_stItemInfo.stStdItem.szName,record.stStdItem.szName)) return 1069;
        if (!fromQuick && !ui.m_xInventoryWnd.AddInvenItem(held.xItem,-1,FALSE)) return -1;
        const Slot previous=slots[slot]; slots[slot]=incoming; take(previous); return 0;
    }
    if (Find(record.stStdItem.szName)>=0) return 1067;
    if (!fromQuick && !ui.m_xInventoryWnd.AddInvenItem(held.xItem,slot,TRUE)) return -1;
    slots[slot]=incoming;
    ZeroMemory(&held,sizeof(held)); heldSlot=-1; heldReference=Slot{};
    return fromQuick?0:1068;
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
    const int slot=SlotAt(point);
    if (!captured && slot>=0) {
        auto& ui=g_xGameProc.m_xInterface; auto& trade=ui.m_xExchangeWnd;
        if (trade.pendingSlot<0 && trade.returnSlot>=0) trade.CancelReturn();
        else if (trade.pendingSlot<0) {
            const CItem sound=ui.m_stCommonItem.bSetted?ui.m_stCommonItem.xItem:slots[slot].item;
            const int result=Release(slot);
            if (result>0) {
                static CLoginMessages captions; static const bool loaded=captions.Load("./CmList.dat");
                std::string text=captions.Get(result);
                auto at=text.find("%s"); if (at!=std::string::npos) text.replace(at,2,sound.m_stItemInfo.stStdItem.szName);
                at=text.find("%d"); if (at!=std::string::npos) text.replace(at,2,std::to_string(slot+1));
                ui.MsgAdd(11861940,0,const_cast<char*>(text.c_str()));
            }
            if (result==0 || result==1068) g_xSound.PlayItemEffect(sound.m_stItemInfo.stStdItem.bStdMode,0,sound.m_stItemInfo.stStdItem.szName);
        }
    }
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
