#include "StdAfx.h"
#include "SvrSelProc.h"
#include "ServerGroupLayout.h"
#include "LoginProcess/LoginVisual.h"
using namespace ServerGroupLayout;

bool CSvrSelProcess::Load(bool offline)
{
    ServerGroups groups;
    if(!groups.Load("./SelLogin.ini") || GetFileAttributesA("./Data/LoginInterface.wil")==INVALID_FILE_ATTRIBUTES ||
        GetFileAttributesA("./Data/LoginInterface.wix")==INVALID_FILE_ATTRIBUTES) return false;
    m_image.Destroy();
    if(!m_image.NewLoad("./Data/LoginInterface.wil",TRUE,TRUE)) return false;
    for(int frame:{Background,Exit,ExitDown,Row,RowHover,Marker,MarkerHover,MarkerSelected,Thumb})
        if(!m_image.NewSetIndex(frame)) {m_image.Destroy();return false;}
    m_groups=std::move(groups);m_offline=offline;m_inLogin=false;m_exitRequested=false;
    m_pointer={-1,-1};m_exitPressed=false;m_dragging=false;m_scrollTick=GetTickCount();
    LoginVisual::ResizeCanvas(Width,Height);
    ShowWindow(g_xChatEditBox.GetSafehWnd(),SW_HIDE);SetFocus(g_xMainWnd.GetSafehWnd());
    g_xSound.UpdateLobbyAudio(CMirSound::LobbyPage::None,0);
    return true;
}
void CSvrSelProcess::DeleteProc() {m_image.Destroy();m_groups={};}
RECT CSvrSelProcess::FrameBounds(int frame,int x,int y)
{
    if(!m_image.NewSetIndex(frame)) return {};
    const auto& size=*m_image.m_lpstNewCurrWilImageInfo;
    return {x,y,x+size.shWidth,y+size.shHeight};
}
RECT CSvrSelProcess::ScrollThumb()
{
    const int count=m_groups.selected<0?0:int(m_groups.groups[m_groups.selected].servers.size());
    const float ratio=count>1?float(m_groups.first)/float(count-1):0;
    return FrameBounds(Thumb,ScrollX,ScrollY+int(ScrollTravel*ratio));
}
void CSvrSelProcess::DrawText(const std::string& text,RECT box,COLORREF color)
{
    HDC dc;auto* surface=g_xMainWnd.GetBackBuffer();if(FAILED(surface->GetDC(&dc))) return;
    HFONT font=LoginVisual::Font();const auto previous=SelectObject(dc,font);
    SetTextColor(dc,color);SetBkMode(dc,TRANSPARENT);
    ::DrawTextA(dc,text.c_str(),int(text.size()),&box,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    SelectObject(dc,previous);DeleteObject(font);surface->ReleaseDC(dc);
}
void CSvrSelProcess::DrawLabel(const ServerGroups::Group& group)
{
    HDC dc;auto* surface=g_xMainWnd.GetBackBuffer();if(FAILED(surface->GetDC(&dc))) return;
    HFONT font=LoginVisual::Font();const auto previous=SelectObject(dc,font);SetBkMode(dc,TRANSPARENT);
    const POINT offsets[]={{-1,0},{1,0},{0,-1},{0,1},{0,0}};
    for(int i=0;i<5;++i) {
        const int x=group.x+5+offsets[i].x,y=group.y+25+offsets[i].y;
        const int saved=SaveDC(dc);IntersectClipRect(dc,x,y,x+260,y+16);
        SetTextColor(dc,i==4?LabelColor:0);TextOutA(dc,x,y,group.label.c_str(),int(group.label.size()));RestoreDC(dc,saved);
    }
    SelectObject(dc,previous);DeleteObject(font);surface->ReleaseDC(dc);
}
void CSvrSelProcess::RenderScene(INT elapsed)
{
    if(m_inLogin) {g_xLoginProc.RenderScene(elapsed);return;}
    Clear(0);
    DrawSelectionFrame(m_image,Background,0,0,SelectionBlend::Normal,false);
    DrawSelectionFrame(m_image,m_exitPressed?ExitDown:Exit,ExitX,ExitY,SelectionBlend::Normal,false);
    for(int i=0;i<int(m_groups.groups.size());++i) {
        const auto& group=m_groups.groups[i];if(!group.x && !group.y) continue;
        DrawLabel(group);
        const bool selected=m_groups.selected==i;
        RECT hover={group.x,group.y,group.x+33,group.y+21};
        const int frame=selected?MarkerSelected:PtInRect(&hover,m_pointer)?MarkerHover:Marker;
        DrawSelectionFrame(m_image,frame,group.x-(selected?3:0),group.y-(selected?3:0),SelectionBlend::Normal,false);
    }
    if(m_groups.selected>=0) {
        const auto& group=m_groups.groups[m_groups.selected];
        DrawText(group.title,{28,20,200,35},TitleColor);
        for(int row=0;row<ServerGroups::VisibleRows && m_groups.first+row<int(group.servers.size());++row) {
            const auto& server=group.servers[m_groups.first+row];if(server.caption=="empty") continue;
            RECT box=RowBounds(row);
            DrawSelectionFrame(m_image,PtInRect(&box,m_pointer)?RowHover:Row,box.left+7,box.top-5,SelectionBlend::Normal,false);
            DrawText(server.caption,box,TitleColor);
        }
    }
    const RECT thumb=ScrollThumb();
    DrawSelectionFrame(m_image,Thumb,thumb.left,thumb.top,SelectionBlend::Normal,false);
    if(m_groups.selected>=0) {
        const auto& group=m_groups.groups[m_groups.selected];
        DrawSelectionFrame(m_image,Glow,group.x-55,group.y-120,SelectionBlend::Additive,true);
    }
}
void CSvrSelProcess::Drag(int y)
{
    if(m_groups.selected<0) return;
    const int count=int(m_groups.groups[m_groups.selected].servers.size());
    const RECT thumb=FrameBounds(Thumb,0,0);const float span=float(ScrollTravel+thumb.bottom);
    const float ratio=(std::max)(0.f,(std::min)(1.f,(y-ScrollY)/span));
    m_groups.first=(std::max)(0,(std::min)(count-1,int(count*ratio)));
}
void CSvrSelProcess::Move(POINT point) {m_pointer=point;if(m_dragging) Drag(point.y);}
void CSvrSelProcess::Enter(const LoginConfiguration& config)
{
    m_dragging=false;ReleaseCapture();m_inLogin=true;
    if(!m_offline) {
        g_xClientSocket.m_pxDefProc=g_xMainWnd.m_pxDefProcess=&g_xLoginProc;
        g_bProcState=_LOGIN_PROC;
    }
    g_xLoginProc.Load(!m_offline,&config);
    m_image.Destroy();
}
void CSvrSelProcess::Down(POINT point,DWORD tick)
{
    m_pointer=point;
    for(int i=0;i<int(m_groups.groups.size());++i) {
        const auto& group=m_groups.groups[i];const RECT hit={group.x,group.y,group.x+40,group.y+40};
        if(PtInRect(&hit,point)) m_groups.Choose(i);
    }
    if(m_groups.selected>=0) {
        for(int row=0;row<ServerGroups::VisibleRows;++row) {
            const RECT box=RowBounds(row);
            if(PtInRect(&box,point)) if(const auto config=m_groups.Login(row)) {Enter(*config);return;}
        }
        const RECT thumb=ScrollThumb();const RECT track={ScrollX,ScrollY,thumb.right,ScrollY+ScrollTravel+thumb.bottom-thumb.top};
        const int count=int(m_groups.groups[m_groups.selected].servers.size());
        if(PtInRect(&track,point) && tick-m_scrollTick>ScrollDelay) {
            if(PtInRect(&thumb,point)) {m_dragging=true;SetCapture(g_xMainWnd.GetSafehWnd());}
            else if(count) {
                const float ratio=count>1?float(m_groups.first)/(count-1):0;
                const float next=(std::max)(0.f,(std::min)(1.f,ratio+(point.y<thumb.top+15?-float(ScrollPage):float(ScrollPage))/count));
                m_groups.first=(std::min)(count-1,int(count*next));
            }
            m_scrollTick=tick;
        }
    }
    const RECT exit=FrameBounds(Exit,ExitX,ExitY);m_exitPressed=PtInRect(&exit,point)!=FALSE;
}
void CSvrSelProcess::Up(POINT point)
{
    m_pointer=point;m_dragging=false;m_exitPressed=false;ReleaseCapture();
    const RECT exit=FrameBounds(Exit,ExitX,ExitY);
    if(PtInRect(&exit,point)) {
        m_exitRequested=true;
        if(!m_offline) PostMessage(g_xMainWnd.GetSafehWnd(),WM_CLOSE,0,0);
    }
}
LRESULT CSvrSelProcess::DefMainWndProc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(m_inLogin) return g_xLoginProc.DefMainWndProc(window,message,wp,lp);
    const POINT point={(short)LOWORD(lp),(short)HIWORD(lp)};
    switch(message) {
    case WM_MOUSEMOVE:Move(point);return 0;
    case WM_LBUTTONDOWN:Down(point,GetTickCount());return 0;
    case WM_LBUTTONUP:Up(point);return 0;
    case WM_MOUSEWHEEL:m_groups.Scroll((short)HIWORD(wp)>0?-1:1);return 0;
    case WM_CAPTURECHANGED:m_dragging=false;return 0;
    case WM_MOVE:g_xMainWnd.OnMove(wp,lp);return 0;
    }
    return CWHDefProcess::DefMainWndProc(window,message,wp,lp);
}
