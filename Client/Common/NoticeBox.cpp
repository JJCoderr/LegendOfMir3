#include "stdafx.h"

BOOL CNoticeBox::Load(CWHWilImageData* source) { image=source;return image!=nullptr; }
BOOL CNoticeBox::HideMessageBox() { active=false;pressed=false;dragging=false;return FALSE; }
BOOL CNoticeBox::DestoryMessageBox() { HideMessageBox();lines.clear();image=nullptr;return FALSE; }

void CNoticeBox::LayoutText(const char* text)
{
    constexpr int Width=324,MaximumRows=21;
    lines.clear();
    HDC dc=GetDC(g_xMainWnd.GetSafehWnd());
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font=g_xMainWnd.CreateGameFont(face,9);HGDIOBJ old=SelectObject(dc,font);
    TEXTMETRICA metrics={};GetTextMetricsA(dc,&metrics);lineHeight=metrics.tmHeight+3;
    SIZE size={};GetTextExtentPoint32A(dc,text,int(strlen(text)),&size);
    if(size.cx<=Width) lines.push_back(text);
    else {
        std::string row,previous;
        for(size_t i=0;text[i] && lines.size()<MaximumRows;++i) {
            if(text[i]=='\n' || text[i]==27) {
                // 4815D0 retains the previous buffer on consecutive separators.
                lines.push_back(row.empty()?previous:row);previous=lines.back();row.clear();continue;
            }
            row+=text[i];
            if(static_cast<signed char>(text[i])<0 && text[i+1]) row+=text[++i];
            GetTextExtentPoint32A(dc,row.data(),int(row.size()),&size);
            if(size.cx>=Width) {lines.push_back(row);previous=row;row.clear();}
        }
        if(!row.empty() && lines.size()<MaximumRows) lines.push_back(row);
    }
    SelectObject(dc,old);DeleteObject(font);ReleaseDC(g_xMainWnd.GetSafehWnd(),dc);
}

BOOL CNoticeBox::ShowMessageBox(char* text,int requestedFrame)
{
    if(active) return TRUE;
    if(!image || !text || !*text || strlen(text)>8191 || !image->NewSetIndex(requestedFrame)) return FALSE;
    const auto& info=*image->m_lpstNewCurrWilImageInfo;
    const int width=g_xMainWnd.m_rcWindow.right-g_xMainWnd.m_rcWindow.left;
    const int height=g_xMainWnd.m_rcWindow.bottom-g_xMainWnd.m_rcWindow.top;
    bounds={(width-info.shWidth)/2,(height-info.shHeight)/2,(width+info.shWidth)/2,(height+info.shHeight)/2};
    frame=requestedFrame;LayoutText(text);pressed=dragging=false;active=true;return TRUE;
}

RECT CNoticeBox::ButtonBounds()
{
    if(!image || !image->NewSetIndex(AcceptFrame)) return {};
    const auto& info=*image->m_lpstNewCurrWilImageInfo;
    return {bounds.left+131,bounds.top+429,bounds.left+131+info.shWidth,bounds.top+429+info.shHeight};
}
HRESULT CNoticeBox::OnButtonDown(POINT point)
{
    if(!active) return 0;
    const RECT button=ButtonBounds();pressed=PtInRect(&button,point)!=0;
    if(pressed) return 1;
    dragging=PtInRect(&bounds,point)!=0;grab={point.x-bounds.left,point.y-bounds.top};return 0;
}
HRESULT CNoticeBox::OnButtonUp(POINT point)
{
    const RECT button=ButtonBounds();const bool accepted=active && pressed && PtInRect(&button,point);
    pressed=dragging=false;
    if(accepted) {g_xSound.PlayLobbyEffect("105.wav");g_xClientSocket.SendNoticeOK();}
    return accepted;
}
bool CNoticeBox::OnKeyDown(WPARAM key)
{
    if(!active || key!=VK_RETURN) return false;
    g_xClientSocket.SendNoticeOK();return true;
}
void CNoticeBox::MoveWnd(POINT point)
{
    if(active && dragging) OffsetRect(&bounds,point.x-grab.x-bounds.left,point.y-grab.y-bounds.top);
}
BOOL CNoticeBox::RenderMessageBox(INT)
{
    if(!active || !image) return TRUE;
    const auto draw=[&](int index,int x,int y) {
        if(!image->NewSetIndex(index)) return;
        const auto& info=*image->m_lpstNewCurrWilImageInfo;
        g_xMainWnd.DrawWithImageForCompClipRgn(x,y,info.shWidth,info.shHeight,(WORD*)image->m_pbCurrImage,800,600);
    };
    draw(frame,bounds.left,bounds.top);
    auto* surface=g_xMainWnd.GetBackBuffer();HDC dc;
    if(SUCCEEDED(surface->GetDC(&dc))) {
        const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
        HFONT font=g_xMainWnd.CreateGameFont(face,9);HGDIOBJ old=SelectObject(dc,font);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,RGB(255,255,255));
        for(size_t row=0;row<lines.size();++row) TextOutA(dc,bounds.left+50,bounds.top+115+int(row)*lineHeight,lines[row].data(),int(lines[row].size()));
        SelectObject(dc,old);DeleteObject(font);surface->ReleaseDC(dc);
    }
    draw(pressed?AcceptPressedFrame:AcceptFrame,bounds.left+131,bounds.top+429);
    return FALSE;
}
