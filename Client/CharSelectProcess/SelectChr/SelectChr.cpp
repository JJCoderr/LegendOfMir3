#include "stdafx.h"
#include "LoginProcess/LoginLayout.h"
#include "../SelectionDialog.h"
#include "Common/LegacyPanel.h"

#define CHR_ANI_COUNT			21
#define ANIMATION_TICK			200
#define BLEND_TICK				50


CSelectChr::CSelectChr()
{
	m_pBtn = NULL;
	m_nSelectChr = 0;
	m_nRenderState = 0;
	m_nAnimationCount = 0;
	m_nRoll = 0;
	m_nBlend = -999;
	m_fIsButtonDown	= FALSE;
}

CSelectChr::~CSelectChr()
{
	if(m_pBtn!=NULL)
	{
		for (int i = 0; i < 4; ++i) delete m_pBtn[i];
		delete[] m_pBtn;
	}
}

VOID CSelectChr::Create(CWHWilImageData* pxImage,CPDLList<SelectionRoleInfo>* pxList, CPDLList<CHRRECT>* pxrcList,INT nChrCount)
{
	INT	nLoop;
	m_messages.Load(".\\CmList.dat");
    m_pointer={-1,-1};m_fIsButtonDown=FALSE;
    m_page = 0;
    m_pagePressed = -1;
    m_pageImages.Destroy();
    m_pageImages.NewLoad(".\\Data\\GameInter.wil", TRUE, TRUE);
	// Buttons
	BUTTONINFO CharProcBtnInfo[] ={
									{ IMG_IDX_SELECT_CREATE,	SELECT_CHAR_NEW_BTN_POS_X,		SELECT_CHAR_NEW_BTN_POS_Y,		0,					0},	// Create New Charector Button
									{ IMG_IDX_SELECT_DELETE,	SELECT_CHAR_ERASE_BTN_POS_X,	SELECT_CHAR_ERASE_BTN_POS_Y,	0,					0},	// Erase Charector Button
									{ IMG_IDX_SELECT_EXIT,		SELECT_CHAR_EXIT_BTN_POS_X,		SELECT_CHAR_EXIT_BTN_POS_Y,		0,					0},	// Exit Button
									{ IMG_IDX_SELECT_START,		SELECT_CHAR_START_BTN_POS_X,	SELECT_CHAR_START_BTN_POS_Y,	0,					0},	// Start Button									
									};

	if (m_pBtn) {
		for (int i = 0; i < 4; ++i) delete m_pBtn[i];
		delete[] m_pBtn;
	}
	m_nSelectChr = 0;
	m_pBtn = new CMirButton*[sizeof(CharProcBtnInfo)/sizeof(BUTTONINFO)];

	m_pxImage			= pxImage;
	m_pxSList			= pxList;
	m_pxrcList			= pxrcList;
	m_nChrCount			= nChrCount;
	m_nSelectChr = nChrCount ? 0 : SELECT_NONE;
    for (int i = 0; i < SelectionRoster::Capacity; ++i) {
        m_roleAnimation[i] = RoleAnimation{};
        m_roleAnimation[i].action = i % 2 == 0 ? 1 : 0;
    }
	
	m_xLightFog.Init(RGB(125,125,125));

	for( nLoop = 0 ; nLoop < (sizeof(CharProcBtnInfo)/sizeof(BUTTONINFO)) ; nLoop++)
	{
		m_pBtn[nLoop] = new CMirButton;
		m_pxImage->NewSetIndex(CharProcBtnInfo[nLoop].nID);
		CharProcBtnInfo[nLoop].right = m_pxImage->m_lpstNewCurrWilImageInfo->shWidth;
		CharProcBtnInfo[nLoop].bottom = m_pxImage->m_lpstNewCurrWilImageInfo->shHeight;
		m_pBtn[nLoop]->SetBtn(&CharProcBtnInfo[nLoop]);
	}
}

VOID CSelectChr::OpenWnd(VOID)
{
	m_fIsActive = TRUE;
	m_nRenderState = 9;
	m_nBlend = 100;
}
HRESULT	CSelectChr::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

int CSelectChr::PageHit(LPARAM position)
{
    if (m_nChrCount <= 2) return -1;
    const int target = 1 - m_page, x = target ? 550 : 500;
    if (!m_pageImages.NewSetIndex(target ? 1153 : 1150)) return -1;
    const auto& info = *m_pageImages.m_lpstNewCurrWilImageInfo;
    RECT bounds = {x,400,x + info.shWidth,400 + info.shHeight};
    POINT point = {LOWORD(position),HIWORD(position)};
    return PtInRect(&bounds, point) ? target : -1;
}

HRESULT CSelectChr::OnButtonDown(WPARAM wParam, LPARAM lParam)
{
    m_pointer={LOWORD(lParam),HIWORD(lParam)};
    m_pagePressed = m_fIsActive ? PageHit(lParam) : -1;
	if(m_fIsActive)
		for (int i = ID_SEL_CHR_NEW_BTN; i <= ID_SEL_CHR_START_BTN; i++)
		{
			if (IsInRect(m_pBtn[i]->m_Rect,LOWORD(lParam), HIWORD(lParam)))
			{
				m_pBtn[i]->m_nState = BUTTON_STATE_DOWN;
				m_fIsButtonDown = TRUE;
			}
			else
				m_pBtn[i]->m_nState = BUTTON_STATE_UP;
		}

    if(m_fIsActive) {
        m_pxrcList->MoveCurrentToTop();
        POINT point={LOWORD(lParam),HIWORD(lParam)};
        for(int i=0;i<m_pxrcList->ListLength();++i) {
            const auto& roleBounds=*m_pxrcList->GetCurrentData();
            RECT bounds={roleBounds.nLeft,roleBounds.nTop,
                roleBounds.nLeft+roleBounds.nWidth,roleBounds.nTop+roleBounds.nHeight};
            if(i/2==m_page && PtInRect(&bounds,point)) {
                if(m_nSelectChr!=i) {
                    if(m_nSelectChr>=0) {
                        m_roleAnimation[m_nSelectChr]=RoleAnimation{};
                        m_roleAnimation[m_nSelectChr].action=3;
                    }
                    m_roleAnimation[i]=RoleAnimation{};
                    m_roleAnimation[i].action=1;
                    m_nSelectChr=i;

                }
                break;
            }
            m_pxrcList->MoveNextNode();
        }
    }

	return 0;
}

HRESULT CSelectChr::OnButtonDown(POINT ptMouse)
{
	m_fIsButtonDown = TRUE;
	return 0;
}

HRESULT CSelectChr::OnButtonUp(WPARAM wParam, LPARAM lParam)
{
    m_pointer={LOWORD(lParam),HIWORD(lParam)};
    m_fIsButtonDown = FALSE;
    const int target = PageHit(lParam);
    if (m_fIsActive && target >= 0) {
        g_xSound.PlayLobbyEffect(LoginLayout::ClickSound);
        m_roleAnimation[m_nSelectChr] = RoleAnimation{};
        m_page = target;
        m_nSelectChr = target * 2;
        m_roleAnimation[m_nSelectChr] = RoleAnimation{};
        m_roleAnimation[m_nSelectChr].action = 1;
        m_pagePressed = -1;
        m_fIsButtonDown = FALSE;
        return 0;
    }
    m_pagePressed = -1;
	INT		i;
	if(m_fIsActive)
	{
		for(i = ID_SEL_CHR_NEW_BTN; i <= ID_SEL_CHR_START_BTN; i++)
		{	m_pBtn[i]->m_nState = BUTTON_STATE_UP;
			if (IsInRect(m_pBtn[i]->m_Rect,LOWORD(lParam), HIWORD(lParam)))
			{
                g_xSound.PlayLobbyEffect(LoginLayout::ClickSound);
                switch(i)
				{	
					case ID_SEL_CHR_NEW_BTN:
					{
						if(m_nChrCount<SelectionDialog::CreationLimit)
						{	m_nBlend = 0;
							m_nRenderState = 11;
							return RETURN_SELECT_NEW_BTN;
						}
						else
							return RETURN_SELECT_NEW_ERR;
						break;
					}
					case ID_SEL_CHR_DEL_BTN:
					{
						if(m_nChrCount!=0)
						{
							return RETURN_SELECT_DEL_BTN;
						}
						else
							return RETURN_SELECT_DEL_ERR;
						break;
					}
					case ID_SEL_CHR_EXIT_BTN:
					{
						return RETURN_SELECT_EXIT_BTN;
					}
					case ID_SEL_CHR_START_BTN:
					{
						if(m_nSelectChr != SELECT_NONE)
						{	m_nBlend = 0;
							m_pxSList->MoveCurrentToTop();
							m_pxSList->MoveNode(m_nSelectChr);
							EnterCharacter(m_pxSList->GetCurrentData()->ChrName);
							return RETURN_SELECT_START_BTN;
						}
						else
						{
							return RETURN_SELECT_START_ERR;
						}
						break;
					}
				}
			}
		}


	}

	m_fIsButtonDown = FALSE;

	return 0;
}

HRESULT CSelectChr::OnButtonUp(POINT ptMouse)
{

	return 0;
}

LRESULT	CSelectChr::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    m_pointer={LOWORD(lParam),HIWORD(lParam)};
	INT	i;
	BOOL	fOnButton = FALSE;
	if(m_fIsActive&&(!m_fIsButtonDown))
	{
		for (i = ID_SEL_CHR_NEW_BTN; i <= ID_SEL_CHR_START_BTN; i++)
		{
			if (IsInRect(m_pBtn[i]->m_Rect,LOWORD(lParam), HIWORD(lParam)))
			{
				m_pBtn[i]->m_nState = BUTTON_STATE_DOWN;
				fOnButton = TRUE;
			}
			else
				m_pBtn[i]->m_nState = BUTTON_STATE_UP;
		}

/*		if(fOnButton)
		{
			HCURSOR	hCursor;
			hCursor = LoadCursor(g_xMainWnd.GetSafehInstance(),MAKEINTRESOURCE(IDC_HAND1));
			SetCursor(hCursor);
			DestroyCursor(hCursor); 
		}
		else
			g_xMainWnd.OnSetCursor(IDC_ARROW);
*/
	}
	return 0;
}

VOID CSelectChr::Render(INT	nLoopTime)
{
	static DWORD	dwTicks = 0;
	int i;
	INT	nPx,nPy;
	POINT	ptPos[]	=	{ 
						{310,310},	// One
						{300,270},	// Two-One
						{360,310},	// Two-Two
						{250,270},	// Three-One
						{320,310},	// Three-Two
						{390,340} 
						};// Three-Three	

	if(m_fIsActive)
	{
		m_xLightFog.ClearSavedLightColor();


		dwTicks +=nLoopTime;

		if(dwTicks > BLEND_TICK)
		{
			m_nBlend = m_nBlend +((m_nRenderState != 0)? dwTicks / BLEND_TICK *(m_nRenderState != 9 ? 1 : -1): 0);
			m_nRoll = (m_nRoll+ 2)%100;
			if(dwTicks > ANIMATION_TICK)
			{
				m_nAnimationCount++;
				m_nAnimationCount%=CHR_ANI_COUNT;
				dwTicks = 0;
			}
		}

        m_pxSList->MoveCurrentToTop();
        m_pxrcList->MoveCurrentToTop();
        for (i = 0; i < m_nChrCount; ++i) {
            const SelectionRoleInfo* role = m_pxSList->GetCurrentData();
            const int type = role->Gender + 2 * role->Class;
            if (i / 2 == m_page && type >= 0 && type < 6) {
                auto& animation = m_roleAnimation[i];
                const int x=m_nChrCount==1?300:(i%2?350:250),y=i%2?250:210;
                if(animation.Due(nLoopTime) && m_pxImage->NewSetIndex(animation.ImageFrame(role->Class,role->Gender)+1)) {
                    const auto& info=*m_pxImage->m_lpstNewCurrWilImageInfo;
                    CHRRECT* bounds=m_pxrcList->GetCurrentData();
                    bounds->nLeft=x+info.shPX;bounds->nTop=y+info.shPY;
                    bounds->nWidth=info.shWidth;bounds->nHeight=info.shHeight;
                }
                if(animation.Advance(nLoopTime,role->Class,role->Gender))
                    g_xSound.PlayRoleVoice(false,role->Class,role->Gender);
                const int frame=animation.ImageFrame(role->Class,role->Gender);
                DrawSelectionFrame(*m_pxImage,frame+20,x,y,SelectionBlend::Shadow);
                DrawSelectionFrame(*m_pxImage,frame,x,y,animation.action?SelectionBlend::Normal:SelectionBlend::Inactive);
                DrawSelectionFrame(*m_pxImage,frame+40,x,y,SelectionBlend::Additive);
            }
            m_pxSList->MoveNextNode();
            m_pxrcList->MoveNextNode();
        }
		m_xLightFog.ShowLightFog();

		// The resource background contains no button labels; draw every state.
		for (i = ID_SEL_CHR_NEW_BTN; i <= ID_SEL_CHR_START_BTN; i++)
		{
			const int frame = m_pBtn[i]->m_nButtonID +
				(m_fIsButtonDown && m_pBtn[i]->m_nState == BUTTON_STATE_DOWN ? 0 : 1);
			if (m_pxImage->NewSetIndex(frame))
				g_xMainWnd.DrawWithImageForComp(m_pBtn[i]->m_Rect.left, m_pBtn[i]->m_Rect.top,
					m_pxImage->m_lpstNewCurrWilImageInfo->shWidth,
					m_pxImage->m_lpstNewCurrWilImageInfo->shHeight,
					(WORD*)m_pxImage->m_pbCurrImage);
            if(PtInRect(&m_pBtn[i]->m_Rect,m_pointer))
                DrawLegacyPanel(m_pBtn[i]->m_Rect,RGB(150,100,50),50,RGB(200,150,100));
		}

        if (m_nChrCount > 2) {
            for (int target = 0; target < 2; ++target) {
                const int base = target ? 1153 : 1150;
                const int frame = base + (target == m_page ? 2 : m_pagePressed == target ? 1 : 0);
                if (m_pageImages.NewSetIndex(frame)) {
                    const auto& info = *m_pageImages.m_lpstNewCurrWilImageInfo;
                    g_xMainWnd.DrawWithImageForComp(target ? 550 : 500, 400,
                        info.shWidth, info.shHeight, (WORD*)m_pageImages.m_pbCurrImage);
                }
            }
        }
		ShowChrInfo();
		if((m_nBlend>=100 && m_nRenderState!=9)||(m_nBlend<=0 && m_nRenderState==9))
		{
			m_nBlend = -1;
			m_nRenderState = m_nRenderState + 10;
		}

	}
}

VOID CSelectChr::ShowChrInfo(VOID)
{
    if (m_nSelectChr < 0 || m_nSelectChr >= m_nChrCount) return;
    m_pxSList->MoveCurrentToTop();
    m_pxSList->MoveNode(m_nSelectChr);
    const SelectionRoleInfo& role = *m_pxSList->GetCurrentData();
    lstrcpynA(g_szCharName, role.ChrName, sizeof(g_szCharName));
    const std::string name = m_messages.Get(206) + "   " + role.ChrName;
    const std::string level = m_messages.Get(207) + "   " + std::to_string(role.Level);
    const std::string job = m_messages.Get(208 + role.Class);
    const std::string& face = m_messages.Get(1074);
    HFONT font = g_xMainWnd.CreateGameFont(face.c_str(), 9, 0, FW_BOLD);
    HDC dc = GetDC(g_xMainWnd.GetSafehWnd());
    HGDIOBJ old = SelectObject(dc, font);
    SIZE nameSize = {}, jobSize = {};
    GetTextExtentPoint32A(dc, name.c_str(), (int)name.size(), &nameSize);
    GetTextExtentPoint32A(dc, job.c_str(), (int)job.size(), &jobSize);
    SelectObject(dc, old); ReleaseDC(g_xMainWnd.GetSafehWnd(), dc); DeleteObject(font);
    const int width = max(nameSize.cx, jobSize.cx) + 20;
    RECT box = {79,108,79+width,178};
    DrawLegacyPanel(box, RGB(50,50,25), 100, RGB(100,100,80));
    g_xMainWnd.PutsHan(NULL,89,118,0x96c8ff,0,(char*)name.c_str(),g_xMainWnd.CreateGameFont(face.c_str(),9,0,FW_BOLD));
    g_xMainWnd.PutsHan(NULL,89,138,0x96f096,0,(char*)level.c_str(),g_xMainWnd.CreateGameFont(face.c_str(),9,0,FW_BOLD));
    g_xMainWnd.PutsHan(NULL,89,158,0xafffff,0,(char*)job.c_str(),g_xMainWnd.CreateGameFont(face.c_str(),9,0,FW_BOLD));
}

VOID CSelectChr::CloseWnd(VOID)
{
	m_fIsActive = FALSE;
}

bool CSelectChr::EnterCharacter(const std::string& name)
{
    if (!m_fIsActive) return false;
    m_pxSList->MoveCurrentToTop();
    for (int i = 0; i < m_nChrCount; ++i) {
        const auto role = m_pxSList->GetCurrentData();
        if (name == role->ChrName) {
            m_nSelectChr = i;
            m_nBlend = 0;
            lstrcpynA(g_szCharName, role->ChrName, sizeof(g_szCharName));
            g_xClientSocket.OnSelChar(role->ChrName);
            return true;
        }
        m_pxSList->MoveNextNode();
    }
    return false;
}
