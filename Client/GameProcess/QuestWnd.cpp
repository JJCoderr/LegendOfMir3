#include "StdAfx.h"
#include "Common/LegacyPanel.h"

void CQuestWnd::Init()
{
    CGameWnd::Init(); entries.clear();
    pressed=selected=expanded=descriptionParent=descriptionChild=-1;
    first=descriptionFirst=0; scrollCaptured=false;
}
void CQuestWnd::CreateQuestWnd(INT id,CWHWilImageData* image,INT frame,INT x,INT y,INT width,INT height,BOOL movable)
{
    CreateGameWnd(id,image,frame,movable,x,y,width,height);
}
int CQuestWnd::LastRow() const
{
    return max(0,int(entries.size())-1)+(expanded>=0?int(entries[expanded].children.size()):0);
}
int CQuestWnd::EntryAt(int row) const
{
    if (expanded>=0 && row>expanded) {
        const int count=int(entries[expanded].children.size());
        if (row<=expanded+count) return -1;
        row-=count;
    }
    return row>=0 && row<int(entries.size())?row:-1;
}
int CQuestWnd::ThumbY() const { return m_rcWnd.top+160+(LastRow()?182*first/LastRow():0); }
void CQuestWnd::Scroll(int delta) { first=max(0,min(LastRow(),first+delta)); }
const std::string& CQuestWnd::Description() const
{
    static const std::string empty;
    if (descriptionParent<0 || descriptionParent>=int(entries.size())) return empty;
    const auto& entry=entries[descriptionParent];
    if (descriptionChild<0) return entry.summary;
    return descriptionChild<int(entry.children.size())?entry.children[descriptionChild].summary:empty;
}
int CQuestWnd::ButtonAt(POINT point)
{
    for (int index=0;index<2;++index) {
        if (!m_pxWndImage->NewSetIndex(index?1425:1422)) continue;
        const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        const int x=m_rcWnd.left+267,y=m_rcWnd.top+(index?107:75);
        RECT box={x,y,x+info.shWidth,y+info.shHeight}; if (PtInRect(&box,point)) return index;
    }
    return -1;
}
bool CQuestWnd::ScrollInput(int kind,POINT point)
{
    if (kind==2) { const bool captured=scrollCaptured; scrollCaptured=false; return captured; }
    if (!m_pxWndImage->NewSetIndex(1225)) return false;
    const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
    const int top=m_rcWnd.top+160;
    if (kind==1 && scrollCaptured) {
        first=LastRow()*max(0,min(182+int(info.shHeight),int(point.y)-top))/(182+info.shHeight); return true;
    }
    if (kind!=0 || point.x<m_rcWnd.left+272 || point.x>=m_rcWnd.left+272+info.shWidth || point.y<top || point.y>=top+182+info.shHeight) return false;
    if (point.y>=ThumbY() && point.y<ThumbY()+info.shHeight) scrollCaptured=true;
    else Scroll(point.y<ThumbY()+7?-5:5);
    return true;
}
void CQuestWnd::Wheel(int delta,POINT point)
{
    if (!PtInRect(&m_rcWnd,point)) return;
    const RECT detail={m_rcWnd.left+80,m_rcWnd.top+310,m_rcWnd.left+250,m_rcWnd.top+380};
    if (PtInRect(&detail,point) && !Description().empty()) descriptionFirst=max(0,min(3,descriptionFirst+delta));
    if (point.y<=m_rcWnd.top+310) Scroll(delta);
}
void CQuestWnd::OnScrollDown() { Wheel(-1,g_xGameProc.m_ptMousePos); }
void CQuestWnd::OnScrollUp() { Wheel(1,g_xGameProc.m_ptMousePos); }
BOOL CQuestWnd::OnLButtonDown(POINT point)
{
    if (ScrollInput(0,point)) return TRUE;
    pressed=ButtonAt(point); if (pressed>=0) return TRUE;
    const RECT list={m_rcWnd.left+65,m_rcWnd.top+90,m_rcWnd.left+265,m_rcWnd.top+90+19*15};
    if (!PtInRect(&list,point)) return FALSE;
    const int row=first+(point.y-m_rcWnd.top-90)/15,index=EntryAt(row);
    if (index>=0) { selected=descriptionParent=index; descriptionChild=-1; descriptionFirst=0; }
    else if (expanded>=0 && row>expanded && row<=expanded+int(entries[expanded].children.size())) {
        selected=-1; descriptionParent=expanded; descriptionChild=row-expanded-1; descriptionFirst=0;
    }
    return TRUE;
}
BOOL CQuestWnd::OnLButtonUp(POINT point)
{
    if (ScrollInput(2,point)) return FALSE;
    const int hit=ButtonAt(point); const bool clicked=hit>=0 && hit==pressed; pressed=-1;
    if (clicked) {
        g_xSound.PlayLobbyEffect("105.wav");
        if (hit==1) return TRUE;
        if (selected>=0) { expanded=selected; Scroll(0); }
    }
    return FALSE;
}
void CQuestWnd::ShowQuestWnd()
{
    const int x=m_rcWnd.left,y=m_rcWnd.top;
    if (m_pxWndImage->NewSetIndex(1420)) {
        const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        DrawSelectionFrame(*m_pxWndImage,1420,x-(info.shWidth-340)/2,y-(info.shHeight-440)/2,SelectionBlend::Alpha,false,220);
    }
    DrawSelectionFrame(*m_pxWndImage,1225,x+272,ThumbY(),SelectionBlend::Normal,false);
    HDC dc; auto* surface=g_xMainWnd.GetBackBuffer(); if (FAILED(surface->GetDC(&dc))) return;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font=g_xMainWnd.CreateGameFont(face,9); HGDIOBJ old=SelectObject(dc,font); SetBkMode(dc,TRANSPARENT);
    const auto color=[](bool done,bool highlighted) { return highlighted?(done?0x191a32:0x1919c8):(done?0x191a64:0x191996); };
    const auto text=[&](const std::string& value,int left,int top,int width,COLORREF ink) {
        const int state=SaveDC(dc); IntersectClipRect(dc,x+left,y+top,x+left+width,y+top+15);
        SetTextColor(dc,ink); TextOutA(dc,x+left,y+top,value.data(),int(value.size())); RestoreDC(dc,state);
    };
    if (!entries.empty()) for (int row=0;row<19 && row<=LastRow()-first;++row) {
        const int line=first+row,index=EntryAt(line);
        if (index>=0) text(entries[index].title,65,90+15*row,200,color(entries[index].completed,index==selected));
        else {
            const int child=line-expanded-1; const auto& entry=entries[expanded].children[child];
            text(entry.title,75,90+15*row,190,color(entry.completed,descriptionParent==expanded && descriptionChild==child));
        }
    }
    const auto& description=Description(); size_t start=0,end=0; int line=0;
    while (end<description.size()) {
        end+=end+1<description.size() && IsDBCSLeadByteEx(CP_ACP,BYTE(description[end]))?2:1;
        SIZE extent={}; GetTextExtentPoint32A(dc,description.data()+start,int(end-start),&extent);
        if (extent.cx>=170 || end==description.size()) {
            if (line>=descriptionFirst && line<descriptionFirst+3) text(description.substr(start,end-start),84,347+15*(line-descriptionFirst),165,0x7d0000);
            ++line; start=end;
        }
    }
    SelectObject(dc,old); DeleteObject(font); surface->ReleaseDC(dc);
    for (int index=0;index<2;++index) DrawSelectionFrame(*m_pxWndImage,(index?1425:1422)+(pressed==index),x+267,y+(index?107:75),SelectionBlend::Normal,false);
}
