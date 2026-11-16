#include "StdAfx.h"
#include "Common/LegacyPanel.h"
#include "Common/LoginMessages.h"
#include <olectl.h>

CPortraitWnd::~CPortraitWnd() { Close(); }
void CPortraitWnd::Close()
{
    if (preview) preview->Release(); preview=nullptr;
    names.clear(); first=0; selected=hovered=pressed=-1; scrolling=false;
}
void CPortraitWnd::Open()
{
    Close(); WIN32_FIND_DATAA data; HANDLE search=FindFirstFileA((directory+"/*.jpg").c_str(),&data);
    if (search!=INVALID_HANDLE_VALUE) {
        do { if (!(data.dwFileAttributes&(FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_REPARSE_POINT)) && !data.nFileSizeHigh && data.nFileSizeLow<=4095) names.emplace_back(data.cFileName); }
        while (FindNextFileA(search,&data)); FindClose(search);
    }
}
int CPortraitWnd::ReadSelected(std::vector<BYTE>& bytes,IPicture** picture)
{
    if (selected<0 || selected>=int(names.size())) return 4716;
    const auto& name=names[selected]; if (name.find_first_of("/\\:")!=name.npos) return 4716;
    FILE* file=fopen((directory+"/"+name).c_str(),"rb"); if (!file) return 4716;
    fseek(file,0,SEEK_END); const long size=ftell(file); rewind(file);
    if (size>4095) { fclose(file); return 4714; }
    if (size<=0) { fclose(file); return 4716; }
    bytes.resize(size); const bool read=fread(bytes.data(),1,bytes.size(),file)==bytes.size(); fclose(file); if (!read) return 4716;
    // OleLoadPicture accepts other formats too; this panel is specifically JPEG.
    if (bytes.size()<2 || bytes[0]!=0xff || bytes[1]!=0xd8) return 4716;
    HGLOBAL memory=GlobalAlloc(GMEM_MOVEABLE,bytes.size()); void* destination=GlobalLock(memory);
    memcpy(destination,bytes.data(),bytes.size()); GlobalUnlock(memory);
    IStream* stream=nullptr; if (FAILED(CreateStreamOnHGlobal(memory,TRUE,&stream))) { GlobalFree(memory); return 4716; }
    const HRESULT status=OleLoadPicture(stream,LONG(bytes.size()),FALSE,IID_IPicture,(void**)picture); stream->Release();
    return SUCCEEDED(status)?0:4716;
}
void CPortraitWnd::LoadPreview()
{
    if (preview) preview->Release(); preview=nullptr; std::vector<BYTE> bytes; ReadSelected(bytes,&preview);
}
void CPortraitWnd::Request(int button,DWORD tick)
{
    if (button==2) { requested.clear(); requestPending=true; return; }
    if (selected<0) return;
    std::vector<BYTE> bytes; IPicture* picture=nullptr; const int error=ReadSelected(bytes,&picture);
    if (error) { g_xGameProc.m_xInterface.ShowNotice(error); return; } picture->Release();
    if (tick-lastUpload<1000) return;
    lastUpload=tick; requested=std::move(bytes); requestPending=true;
}
int CPortraitWnd::RowAt(POINT point)
{
    for (int i=0;i<12;++i) { const RECT row={26,27+24*i,264,48+24*i}; if (PtInRect(&row,point)) return i; }
    return -1;
}
int CPortraitWnd::ButtonAt(POINT point)
{
    const int buttons[][3]={{1221,539,364},{2092,59,345},{2094,179,345}};
    for (int i=0;i<3;++i) if (m_pxWndImage->NewSetIndex(buttons[i][0])) {
        const auto& info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        const RECT box={buttons[i][1],buttons[i][2],buttons[i][1]+info.shWidth,buttons[i][2]+info.shHeight};
        if (PtInRect(&box,point)) return i;
    }
    return -1;
}
int CPortraitWnd::ThumbY() const
{
    const float ratio=names.size()>1?float(double(first)/(names.size()-1)):0; return 30+int(274*double(ratio));
}
void CPortraitWnd::Wheel(int direction)
{ if (direction>0 && first>0) --first; if (direction<0 && first<int(names.size())-1) ++first; }
bool CPortraitWnd::Scroll(int kind,POINT point)
{
    const DWORD now=GetTickCount();
    if (kind==2) { const bool used=scrolling; scrolling=false; scrollTick=now; return used; }
    if (!m_pxWndImage->NewSetIndex(1966)) return false;
    const auto& info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
    const auto drag=[&](float ratio) { first=int(names.size()*double(max(0.f,min(1.f,ratio)))); };
    if (kind==1) { if (!scrolling) return false; drag(float(double(point.y-30)/(274+info.shHeight))); return true; }
    const RECT track={276,30,276+info.shWidth,304+info.shHeight};
    if (kind!=0 || !PtInRect(&track,point)) return false;
    if (now-scrollTick<=10) return true; scrollTick=now;
    const int top=ThumbY(); const float ratio=names.size()>1?float(double(first)/(names.size()-1)):0;
    if (point.y>=top && point.y<top+info.shHeight) { scrolling=true; drag(ratio); return true; }
    const float page=names.empty()?0:float(12.0/names.size()); drag(ratio+(point.y<top+7?-page:page)); return true;
}
BOOL CPortraitWnd::Pointer(int kind,POINT point)
{
    if (Scroll(kind,point)) return FALSE;
    const int hit=ButtonAt(point),row=RowAt(point);
    if (kind==1 && row>=0) hovered=row;
    if (kind==0) { pressed=hit; if (hit<0 && row>=0 && first+row<int(names.size())) { selected=first+row; LoadPreview(); } }
    if (kind==2) {
        const bool activated=hit>=0 && hit==pressed; pressed=-1;
        if (activated) { g_xSound.PlayLobbyEffect("105.wav"); if (!hit) return TRUE; Request(hit,GetTickCount()); }
    }
    return FALSE;
}
void CPortraitWnd::Show()
{
    if (m_pxWndImage->NewSetIndex(2090)) {
        const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        DrawSelectionFrame(*m_pxWndImage,2090,-(info.shWidth-580)/2,-(info.shHeight-404)/2,SelectionBlend::Normal,false);
    }
    for (int i=0;i<12;++i) {
        const RECT row={26,27+24*i,264,48+24*i};
        if (i==hovered) DrawLegacyPanel(row,RGB(30,30,30),180,0,false);
        if (selected>=0 && i==selected-first) DrawLegacyPanel(row,RGB(35,114,156),180,0,false);
    }
    DrawSelectionFrame(*m_pxWndImage,1966,276,ThumbY(),SelectionBlend::Normal,false);
    DrawSelectionFrame(*m_pxWndImage,pressed==0?1222:1221,539,364,SelectionBlend::Normal,false);
    DrawSelectionFrame(*m_pxWndImage,2092,59,345,SelectionBlend::Normal,false);
    DrawSelectionFrame(*m_pxWndImage,2094,179,345,SelectionBlend::Normal,false);
    HDC dc; auto* surface=g_xMainWnd.GetBackBuffer(); if (FAILED(surface->GetDC(&dc))) return;
    if (preview) { LONG width,height; preview->get_Width(&width); preview->get_Height(&height); preview->Render(dc,390,50,85,104,0,height,width,-height,NULL); }
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0}; HFONT font=g_xMainWnd.CreateGameFont(face,9);
    auto old=SelectObject(dc,font); SetBkMode(dc,TRANSPARENT); SetTextColor(dc,0xfafafa);
    static CLoginMessages captions; static const bool loaded=captions.Load("./CmList.dat");
    const auto text=[&](const std::string& value,int x,int y,int width,int height) {
        const int saved=SaveDC(dc); IntersectClipRect(dc,x,y,x+width,y+height); TextOutA(dc,x,y,value.data(),int(value.size())); RestoreDC(dc,saved);
    };
    for (int i=0;i<12;++i) {
        if (first+i<int(names.size())) text(names[first+i],26,32+24*i,238,21);
        text(captions.Get(4702+i),320,165+15*i,260,15);
    }
    SelectObject(dc,old); DeleteObject(font); surface->ReleaseDC(dc);
}
