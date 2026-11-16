/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/

#include "StdAfx.h"
#include "Common/LoginMessages.h"

static const int groupButtons[5][3] = {
    {1221,217,200},{1362,38,192},{1365,98,192},{1368,158,192},{1370,23,47}
};

BOOL g_fAllowGroup = TRUE;

CGroupWnd::CGroupWnd()
{
	Init();
}

CGroupWnd::~CGroupWnd()
{
	Destroy();
}

VOID CGroupWnd::CreateGroupWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove)
{
	CreateGameWnd(nID, pxWndImage, nFrameImgIdx, bCanMove, nStartX, nStartY, nWidth, nHeight);
}

VOID CGroupWnd::ShowGroupWnd()
{
    if (!m_bActive) return;
    const RECT area = GetGameWndRect();
    const auto draw = [&](int frame,int x,int y) {
        if (!m_pxWndImage->NewSetIndex(frame)) return;
        const auto& skin = *m_pxWndImage->m_lpstNewCurrWilImageInfo;
        g_xMainWnd.DrawWithImageForComp(x,y,skin.shWidth,skin.shHeight,(WORD*)m_pxWndImage->m_pbCurrImage);
    };
    if (m_pxWndImage->NewSetIndex(1360)) {
        const auto& skin = *m_pxWndImage->m_lpstNewCurrWilImageInfo;
        draw(1360,area.left-(skin.shWidth-258)/2,area.top-(skin.shHeight-244)/2);
    }
    for (int i=0;i<4;++i)
        if (i==0 || m_pressed==i)
            draw(groupButtons[i][0]+(m_pressed==i),area.left+groupButtons[i][1],area.top+groupButtons[i][2]);
    draw(g_fAllowGroup?1370:1371,area.left+23,area.top+47);
    auto* surface = g_xMainWnd.GetBackBuffer();
    HDC dc;
    if (FAILED(surface->GetDC(&dc))) return;
    const char face[] = {char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font = g_xMainWnd.CreateGameFont(face,9);
    HGDIOBJ previous = SelectObject(dc,font);
    SetBkMode(dc,TRANSPARENT);
    const auto text = [&](const char* value,int x,int y,int width,COLORREF color) {
        RECT clip = {area.left+x,area.top+y,area.left+x+width,area.top+y+15};
        SetTextColor(dc,color);
        ExtTextOutA(dc,clip.left,clip.top,ETO_CLIPPED,&clip,value,(UINT)strlen(value),NULL);
    };
    text(g_szCharName,52,15,180,0xdce6c8);
    static CLoginMessages captions;
    static const bool loaded = captions.Load("./CmList.dat");
    text(captions.Get(g_fAllowGroup?1603:1604).c_str(),110,52,140,0xdce6c8);
    m_xGroupList.MoveCurrentToTop();
    for (int i=0;i<m_xGroupList.GetCounter() && i<10;++i) {
        const CGroupMember* member = m_xGroupList.GetCurrentData();
        text(member->szMemberName,i%2?45:145,90+20*(i/2),100,0xffffff);
        m_xGroupList.MoveNextNode();
    }
    SelectObject(dc,previous);
    DeleteObject(font);
    surface->ReleaseDC(dc);
}

VOID CGroupWnd::Init()
{
	CGameWnd::Init();
}

VOID CGroupWnd::Destroy()
{
	CGameWnd::Destroy();
}

int CGroupWnd::ButtonAt(POINT point)
{
    for (int i=0;i<5;++i) {
        if (!m_pxWndImage->NewSetIndex(groupButtons[i][0])) continue;
        const auto& image = *m_pxWndImage->m_lpstNewCurrWilImageInfo;
        RECT hit = {m_rcWnd.left+groupButtons[i][1],m_rcWnd.top+groupButtons[i][2],
            m_rcWnd.left+groupButtons[i][1]+image.shWidth,m_rcWnd.top+groupButtons[i][2]+image.shHeight};
        if (PtInRect(&hit,point)) return i;
    }
    return -1;
}

BOOL CGroupWnd::OnLButtonUp(POINT point)
{
    const int hit = ButtonAt(point);
    const bool clicked = hit >= 0 && hit == m_pressed;
    m_pressed = -1;
    if (!clicked) return FALSE;
    g_xSound.PlayLobbyEffect("105.wav");
    if (hit == 0) return TRUE;
    if (hit == 4) {
        g_fAllowGroup = !g_fAllowGroup;
        g_xClientSocket.SendGroupMode(g_fAllowGroup);
    } else g_xGameProc.m_xInterface.OpenGroupDialog(hit);
    return FALSE;
}

BOOL CGroupWnd::OnLButtonDown(POINT point)
{
    m_pressed = ButtonAt(point);
    return m_pressed >= 0;
}

VOID CGroupWnd::OnMouseMove(POINT point)
{
}
