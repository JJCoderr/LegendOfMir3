/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/



#include "StdAfx.h"
#include "Common/LoginMessages.h"

static const int guildButtons[9][3] = {
 {1221,554,401},{1301,39,385},{1304,89,385},{1307,139,385},
 {1310,289,385},{1313,339,385},{1316,389,385},{1320,439,385},{1323,489,385}
};


CGuildWnd::CGuildWnd()
{
	Init();
}

CGuildWnd::~CGuildWnd()
{
	Destroy();
}

VOID CGuildWnd::Init()
{
	CGameWnd::Init();
    m_tab=0; m_pressed=-1; m_first=0;
    m_name.clear();
    for (int i=0;i<3;++i) m_pages[i].clear();
}

VOID CGuildWnd::Destroy()
{
	CGameWnd::Destroy();
}

VOID CGuildWnd::CreateGuildWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove)
{
	CreateGameWnd(nID, pxWndImage, nFrameImgIdx, bCanMove, nStartX, nStartY, nWidth, nHeight);
}

VOID CGuildWnd::ShowGuildWnd()
{
    if (!m_bActive) return;
    const int x=m_rcWnd.left,y=m_rcWnd.top;
    const auto draw = [&](int frame,int left,int top) {
        if (!m_pxWndImage->NewSetIndex(frame)) return;
        const auto& image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        g_xMainWnd.DrawWithImageForComp(left,top,image.shWidth,image.shHeight,(WORD*)m_pxWndImage->m_pbCurrImage);
    };
    if (m_pxWndImage->NewSetIndex(1300)) {
        const auto& skin=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        draw(1300,x-(skin.shWidth-594)/2,y-(skin.shHeight-444)/2);
    }
    const auto& lines=m_pages[m_tab];
    const float rate=lines.size()>1 ? float(m_first)/(lines.size()-1) : 0;
    draw(1225,x+557,y+60+int(258*rate));
    for (int i=0;i<9;++i) draw(guildButtons[i][0]+(m_pressed==i),x+guildButtons[i][1],y+guildButtons[i][2]);
    auto* surface=g_xMainWnd.GetBackBuffer();
    HDC dc;
    if (FAILED(surface->GetDC(&dc))) return;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font=g_xMainWnd.CreateGameFont(face,9);
    HGDIOBJ previous=SelectObject(dc,font);
    SetBkMode(dc,TRANSPARENT);
    const auto text = [&](const std::string& value,int left,int top,int width,int height,COLORREF color,bool center) {
        RECT area={x+left,y+top,x+left+width,y+top+height};
        SetTextColor(dc,color);
        DrawTextA(dc,value.c_str(),(int)value.size(),&area,DT_SINGLELINE|(center?DT_CENTER|DT_VCENTER:DT_LEFT|DT_TOP));
    };
    static CLoginMessages captions;
    static const bool loaded=captions.Load("./CmList.dat");
    text(m_name,340,10,213,24,0x96c8ff,true);
    text(captions.Get(m_tab==0?4010:m_tab==1?4009:4008),45,10,112,25,0x96c8ff,true);
    TEXTMETRICA metrics; GetTextMetricsA(dc,&metrics);
    for (int row=0;row<18 && m_first+row<(int)lines.size();++row) {
        const std::string& value=lines[m_first+row];
        const int top=60+(metrics.tmHeight+5)*row;
        if (m_tab==2) {
            text(value,34,top-1,514,metrics.tmHeight,0x0a140a,false);
            text(value,35,top,514,metrics.tmHeight,0x00ff00,false);
        } else {
            const bool heading=m_tab==1 ? value==captions.Get(4007) :
                value==captions.Get(4004)||value==captions.Get(4005)||value==captions.Get(4006);
            text(value,35,top,514,metrics.tmHeight,heading?38655:0xffffff,false);
        }
    }
    SelectObject(dc,previous); DeleteObject(font); surface->ReleaseDC(dc);
}

int CGuildWnd::ButtonAt(POINT point)
{
    for (int i=0;i<9;++i) {
        if (!m_pxWndImage->NewSetIndex(guildButtons[i][0])) continue;
        const auto& image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        RECT area={m_rcWnd.left+guildButtons[i][1],m_rcWnd.top+guildButtons[i][2],
            m_rcWnd.left+guildButtons[i][1]+image.shWidth,m_rcWnd.top+guildButtons[i][2]+image.shHeight};
        if (PtInRect(&area,point)) return i;
    }
    return -1;
}
BOOL CGuildWnd::OnLButtonUp(POINT point)
{
    const int hit=ButtonAt(point);
    const bool clicked=hit>=0 && hit==m_pressed;
    m_pressed=-1;
    if (!clicked) return FALSE;
    g_xSound.PlayLobbyEffect("105.wav");
    if (hit>=1 && hit<=3) { m_tab=hit-1; m_first=0; }
    return hit==0;
}
BOOL CGuildWnd::OnLButtonDown(POINT point)
{
    m_pressed=ButtonAt(point);
    return m_pressed>=0;
}
VOID CGuildWnd::OnMouseMove(POINT point) {}
VOID CGuildWnd::OnScrollDown() { m_first=max(0,m_first-1); }
VOID CGuildWnd::OnScrollUp() { m_first=min(max(0,int(m_pages[m_tab].size())-18),m_first+1); }
