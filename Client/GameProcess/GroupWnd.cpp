/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/

#include "StdAfx.h"
#include "Common/LoginMessages.h"

typedef struct tagINTERFACE_BUTTON
{
public:
	int		m_nLeft;
	int		m_nTop;
	int		m_nWidth;
	int		m_nHeight;
	BYTE	m_btCommand;
} INTERFACE_BUTTON;

#define GROUPBTN_ALLOWGROUP		1
#define GROUPBTN_CREATEGROUP	2
#define GROUPBTN_ADDMEMBER		3
#define GROUPBTN_DELMEMBER		4

INTERFACE_BUTTON GroupWndButton[] = {	{ 21, 57, 27, 28, GROUPBTN_ALLOWGROUP },
										{ 27, 200, 71, 13, GROUPBTN_CREATEGROUP }, 
										{ 91, 200, 77, 16, GROUPBTN_ADDMEMBER }, 
										{ 170, 200, 73, 16, GROUPBTN_DELMEMBER } 
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
    draw(1221,area.left+217,area.top+200);
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

BOOL CGroupWnd::OnLButtonUp(POINT ptMouse)
{
	RECT tRect = GetGameWndRect();

	for (int i = 0; i < sizeof(GroupWndButton)/sizeof(INTERFACE_BUTTON); i++)
	{
		if (tRect.left + GroupWndButton[i].m_nLeft < ptMouse.x && tRect.left + GroupWndButton[i].m_nLeft + GroupWndButton[i].m_nWidth > ptMouse.x &&
			tRect.top + GroupWndButton[i].m_nTop < ptMouse.y && tRect.top + GroupWndButton[i].m_nTop + GroupWndButton[i].m_nHeight > ptMouse.y)
		{
			switch (GroupWndButton[i].m_btCommand)
			{
				case GROUPBTN_ALLOWGROUP:
					(g_fAllowGroup ? g_fAllowGroup = FALSE : g_fAllowGroup = TRUE);
					g_xClientSocket.SendGroupMode(g_fAllowGroup);
					break;
				case GROUPBTN_CREATEGROUP:
					g_xClientSocket.SendCreateGroup("mandul");
					break;
				case GROUPBTN_ADDMEMBER:
					g_xClientSocket.SendAddGroupMember("mandul");
					break;
				case GROUPBTN_DELMEMBER:
					g_xClientSocket.SendDelGroupMember("mandul");
					break;
			}
		}
	}

	return FALSE;
}

BOOL CGroupWnd::OnLButtonDown(POINT ptMouse)
{
	return FALSE;
}

VOID CGroupWnd::OnMouseMove(POINT ptMouse)
{
}
