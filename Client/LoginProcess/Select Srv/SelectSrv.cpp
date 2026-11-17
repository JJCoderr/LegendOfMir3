#include "stdafx.h"
#include "../LoginVisual.h"

CSelectSrv::CSelectSrv():CBMWnd(), m_ppxSrvBtn(NULL), m_nSrvCount(0), m_pressed(-1), m_hover(-1)
{

}

CSelectSrv::~CSelectSrv()
{
    for (int i = 0; i < m_nSrvCount; ++i) delete m_ppxSrvBtn[i];
    delete[] m_ppxSrvBtn;
}

VOID CSelectSrv::Create(CWHWilImageData* image, CPDLList<ServerList>* servers)
{
    for (int i = 0; i < m_nSrvCount; ++i) delete m_ppxSrvBtn[i];
    delete[] m_ppxSrvBtn;
    m_nSrvCount = servers->GetCounter();
    m_ppxSrvBtn = new _PSERVERBUTTON[m_nSrvCount];
    m_loads.assign(m_nSrvCount,0);m_messages.Load("./CmList.dat");
    m_pressed = m_hover = -1;
    HDC dc = GetDC(g_xMainWnd.GetSafehWnd());
    HFONT font = g_xMainWnd.CreateGameFont("ËÎÌו", 9);
    HGDIOBJ previous = SelectObject(dc, font);
    int width = 0, height = 0;
    servers->MoveCurrentToTop();
    for (int i = 0; i < m_nSrvCount; ++i) {
        m_ppxSrvBtn[i] = new _SERVERBUTTON{};
        strcpy(m_ppxSrvBtn[i]->szName, servers->GetCurrentData()->Name);
        strcpy(m_ppxSrvBtn[i]->szCaption, servers->GetCurrentData()->Caption);
        SIZE size = {};
        GetTextExtentPoint32(dc, m_ppxSrvBtn[i]->szCaption, strlen(m_ppxSrvBtn[i]->szCaption), &size);
        width = max(width, size.cx); height = max(height, size.cy);
        servers->MoveNextNode();
    }
    SelectObject(dc, previous); DeleteObject(font); ReleaseDC(g_xMainWnd.GetSafehWnd(), dc);
    const int rowHeight = height + 20;
    SetRect(&m_rcWnd, 20, 60, 20 + width + 90, 80 + rowHeight * m_nSrvCount);
    for (int i = 0; i < m_nSrvCount; ++i)
        SetRect(&m_ppxSrvBtn[i]->rcBtn, 20, 70 + i * rowHeight, m_rcWnd.right, 70 + (i + 1) * rowHeight);
}

INT CSelectSrv::Hit(INT x, INT y) const
{
    POINT point = {x, y};
    for (int i = 0; i < m_nSrvCount; ++i) if (PtInRect(&m_ppxSrvBtn[i]->rcBtn, point)) return i;
    return -1;
}

HRESULT CSelectSrv::OnKeyDown(WPARAM, LPARAM) { return 0; }
HRESULT CSelectSrv::OnButtonDown(WPARAM, LPARAM position)
{
    m_pressed = Hit(LOWORD(position), HIWORD(position)); return 0;
}
HRESULT CSelectSrv::OnButtonDown(POINT point) { return OnButtonDown(0, MAKELPARAM(point.x, point.y)); }
HRESULT CSelectSrv::OnButtonUp(WPARAM, LPARAM position)
{
    const int pressed = m_pressed; m_pressed = -1;
    if (pressed >= 0 && pressed == Hit(LOWORD(position), HIWORD(position)))
        return pressed+1;
    return 0;
}
HRESULT CSelectSrv::OnButtonUp(POINT point) { return OnButtonUp(0, MAKELPARAM(point.x, point.y)); }
LRESULT CSelectSrv::OnMouseMove(WPARAM, LPARAM position)
{
    m_hover = Hit(LOWORD(position), HIWORD(position)); return 0;
}


VOID CSelectSrv::Render(INT)
{
    if (!m_fIsActive) return;
    DrawLegacyPanel(m_rcWnd,RGB(100,100,100),120,RGB(150,150,150));
    for (int i = 0; i < m_nSrvCount; ++i) {
        const bool pressed=m_pressed==i,hover=m_pressed<0 && m_hover==i;
        const RECT bounds=m_ppxSrvBtn[i]->rcBtn;
        if(pressed || hover) DrawLegacyPanel(bounds,pressed?RGB(255,100,100):RGB(100,100,100),50,
            pressed?RGB(255,100,100):RGB(100,100,200));
        std::string caption=m_ppxSrvBtn[i]->szCaption;
        if(m_loads[i]>=1 && m_loads[i]<=3) caption+=" ("+m_messages.Get(LoginLayout::LoadCaption+m_loads[i])+")";
        LoginVisual::Text(caption,bounds.left+20,bounds.top+10,pressed?0x6464ff:hover?0xff0000:0x1e1e1e);
    }
    for(int i=0;i<3;++i) LoginVisual::Text(m_messages.Get(LoginLayout::HelpCaption+i),40+200*i,440,3328200);
}
bool CSelectSrv::ApplyLoads(const char* text)
{
    auto next=m_loads;std::string body=text;size_t start=0;
    for(size_t end;(end=body.find(';',start))!=std::string::npos;start=end+1) {
        const std::string entry=body.substr(start,end-start);const auto open=entry.find('('),close=entry.find(')');
        if(open==std::string::npos || close==std::string::npos || close<=open+1) return false;
        const std::string value=entry.substr(open+1,close-open-1);char* tail=nullptr;
        const long status=strtol(value.c_str(),&tail,10);if(tail!=value.c_str()+value.size()) return false;
        const std::string name=entry.substr(0,open);
        for(int i=0;i<m_nSrvCount;++i) if(name==m_ppxSrvBtn[i]->szName) next[i]=int(status);
    }
    m_loads.swap(next);return true;
}
