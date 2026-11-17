#include "StdAfx.h"
#include "Common/AnsiInput.h"
#include "Common/LegacyPanel.h"
#include "Common/LoginMessages.h"

void CGuildEditor::Destroy()
{
    if (edit) DestroyWindow(edit); edit=NULL;
    if (font) DeleteObject(font); font=NULL;
    if (background) DeleteObject(background); background=NULL;
    fontHeight=0; dragging=false; pressed=-1; CGameWnd::Destroy();
}
void CGuildEditor::LayoutEdit()
{
    const auto& display=g_xMainWnd.m_rcWindow;
    const int width=display.right-display.left,height=display.bottom-display.top;
    if(width<=0 || height<=0) return;
    const int x=display.left+MulDiv(m_rcWnd.left+46,width,800),y=display.top+MulDiv(m_rcWnd.top+54,height,600);
    const int editWidth=MulDiv(468,width,800),editHeight=MulDiv(170,height,600);
    RECT previous; GetWindowRect(edit,&previous);
    if (previous.right-previous.left!=editWidth || previous.bottom-previous.top!=editHeight) {
        if (background) DeleteObject(background); background=NULL;
    }
    if (previous.left!=x || previous.top!=y || previous.right!=x+editWidth || previous.bottom!=y+editHeight)
        MoveWindow(edit,x,y,editWidth,editHeight,TRUE);
    const int pixels=MulDiv(12,height,600);
    if (fontHeight!=pixels) {
        static CLoginMessages captions; static const bool loaded=captions.Load("./CmList.dat");
        HFONT next=CreateFontA(-pixels,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,captions.Get(1074).c_str());
        SendMessage(edit,WM_SETFONT,(WPARAM)next,TRUE); if (font) DeleteObject(font); font=next; fontHeight=pixels;
        if (background) DeleteObject(background); background=NULL;
    }
}
void CGuildEditor::Open(bool notice)
{
    auto& guild=g_xGameProc.m_xInterface.m_xGuildWnd;
    if (!notice && guild.m_pages[1].empty()) { g_xGameProc.m_xInterface.ShowNotice(4017); return; }
    if (!edit) {
        edit=CreateWindowExA(0,"EDIT","",WS_POPUP|ES_LEFT|ES_MULTILINE|ES_AUTOVSCROLL|ES_WANTRETURN,0,0,468,170,
            g_xMainWnd.GetSafehWnd(),NULL,GetModuleHandle(NULL),NULL);
        SendMessage(edit,EM_LIMITTEXT,0x7ffffffe,0); SendMessage(edit,EM_SETMARGINS,EC_LEFTMARGIN|EC_RIGHTMARGIN,0);
    }
    announcement=notice; pressed=-1; dragging=false; m_bActive=TRUE;
    std::string text; for (const auto& line:guild.m_editable[notice?0:1]) text+=line+"\r\n";
    updating=true; ansi=text; SetWindowTextA(edit,text.c_str()); updating=false; LayoutEdit(); ShowWindow(edit,SW_SHOW); SetFocus(edit);
}
std::string CGuildEditor::AcceptEdit(const std::string& previous,const std::string& next)
{ return LimitAnsiEdit(previous,next,4000); }
void CGuildEditor::Changed()
{
    if (updating || !edit) return;
    std::string text(GetWindowTextLengthA(edit)+1,'\0'); GetWindowTextA(edit,&text[0],int(text.size())); text.resize(strlen(text.c_str()));
    ansi=AcceptEdit(ansi,text);
    if (ansi!=text) { updating=true; SetWindowTextA(edit,ansi.c_str()); updating=false; }
}
int CGuildEditor::ButtonAt(POINT point)
{
    for (int i=0;i<2;++i) {
        if (!m_pxWndImage->NewSetIndex(i?1327:1221)) continue;
        const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        const int x=m_rcWnd.left+(i?461:546),y=m_rcWnd.top+(i?12:212);
        const RECT hit={x,y,x+info.shWidth,y+info.shHeight}; if (PtInRect(&hit,point)) return i;
    }
    return -1;
}
std::string CGuildEditor::Payload(const std::string& input)
{
    const std::string text=input.substr(0,3999); std::string body;
    for (size_t start=0;start<text.size();) {
        const auto end=text.find('\n',start); const std::string line=text.substr(start,end==std::string::npos?end:end-start);
        if (end!=std::string::npos && !line.empty() && line[0]=='\r') body+=" \r"; else body+=line;
        if (end==std::string::npos) break; start=end+1;
    }
    return body;
}
void CGuildEditor::Close(bool save)
{
    Changed();
    m_bActive=FALSE; pressed=-1; dragging=false; ShowWindow(edit,SW_HIDE); SetFocus(g_xMainWnd.GetSafehWnd());
    if (save) {
        const auto body=Payload(ansi);
        if (!body.empty() && body.size()<=3999) {
            _TDEFAULTMESSAGE message; fnMakeDefMessage(&message,announcement?1040:1041,0,0,0,0);
            g_xClientSocket.SendPacket(&message,const_cast<char*>(body.c_str()));
        }
    }
}
void CGuildEditor::Pointer(int kind,POINT point)
{
    if (kind==1) { if (dragging) { MoveGameWnd(point); LayoutEdit(); } return; }
    const int hit=ButtonAt(point);
    if (kind==0) {
        const RECT input={m_rcWnd.left+46,m_rcWnd.top+54,m_rcWnd.left+514,m_rcWnd.top+224};
        pressed=hit; dragging=hit<0 && PtInRect(&m_rcWnd,point) && !PtInRect(&input,point);
        if (dragging) { m_bFocused=TRUE; SetReadyMove(point); }
    } else {
        const bool activate=hit>=0 && hit==pressed; dragging=false; pressed=-1;
        if (activate) { g_xSound.PlayLobbyEffect("105.wav"); Close(hit==1); }
    }
}
HBRUSH CGuildEditor::EditColor(HDC dc)
{
    SetBkMode(dc,TRANSPARENT); SetTextColor(dc,RGB(224,224,224)); SetBrushOrgEx(dc,0,0,NULL);
    return background?background:(HBRUSH)GetStockObject(BLACK_BRUSH);
}
void CGuildEditor::Show()
{
    if (!m_bActive) { if(edit) ShowWindow(edit,SW_HIDE); return; }
    if(g_xGameProc.m_xInterface.m_xMsgBox.IsActive() || g_xGameProc.m_xNotice.IsActive()) {
        ShowWindow(edit,SW_HIDE);
        if(GetFocus()==edit) SetFocus(g_xMainWnd.GetSafehWnd());
    } else if(!IsWindowVisible(edit)) {
        ShowWindow(edit,SW_SHOWNOACTIVATE);SetFocus(edit);
    }
    const int x=m_rcWnd.left,y=m_rcWnd.top;
    if (m_pxWndImage->NewSetIndex(1326)) {
        const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        DrawSelectionFrame(*m_pxWndImage,1326,x-(info.shWidth-584)/2,y-(info.shHeight-252)/2,SelectionBlend::Normal,false);
    }
    LayoutEdit();
    HDC dc; auto* surface=g_xMainWnd.GetBackBuffer(); if (FAILED(surface->GetDC(&dc))) return;
    if (!background) {
        RECT bounds; GetClientRect(edit,&bounds); HDC memory=CreateCompatibleDC(dc);
        HBITMAP bitmap=CreateCompatibleBitmap(dc,bounds.right,bounds.bottom); auto old=SelectObject(memory,bitmap);
        StretchBlt(memory,0,0,bounds.right,bounds.bottom,dc,x+46,y+54,468,170,SRCCOPY);
        background=CreatePatternBrush(bitmap); SelectObject(memory,old); DeleteObject(bitmap); DeleteDC(memory); InvalidateRect(edit,NULL,TRUE);
    }
    static CLoginMessages captions; static const bool loaded=captions.Load("./CmList.dat");
    HFONT title=g_xMainWnd.CreateGameFont(captions.Get(1074).c_str(),9); auto old=SelectObject(dc,title); SetBkMode(dc,TRANSPARENT);
    const auto text=[&](int id,int left,int top,COLORREF color) {
        RECT area={x+left,y+top,x+left+420,y+top+16}; SetTextColor(dc,color);
        DrawTextA(dc,captions.Get(id).c_str(),-1,&area,DT_LEFT|DT_SINGLELINE|DT_TOP);
    };
    text(announcement?1903:1900,70,20,0x96c8ff);
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0}; HFONT normal=g_xMainWnd.CreateGameFont(face,9); SelectObject(dc,normal);
    if (announcement) { text(1904,154,21,0x323232); text(1904,154,22,0x96c8ff); }
    else { text(1901,154,14,0x96c8ff); text(1902,154,28,0x96c8ff); }
    SelectObject(dc,old); DeleteObject(title); DeleteObject(normal); surface->ReleaseDC(dc);
    DrawSelectionFrame(*m_pxWndImage,1221+(pressed==0),x+546,y+212,SelectionBlend::Normal,false);
    DrawSelectionFrame(*m_pxWndImage,1327+(pressed==1),x+461,y+12,SelectionBlend::Normal,false);
}
