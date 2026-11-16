#include "stdafx.h"
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

VOID CSelectChr::Create(CWHWilImageData* pxImage,CPDLList<CHRINFO>* pxList, CPDLList<CHRRECT>* pxrcList,INT nChrCount)
{
	INT	nLoop;
	m_messages.Load(".\\CmList.dat");
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
    for (int i = 0; i < 3; ++i) {
        m_roleAnimation[i] = RoleAnimation{};
        m_roleAnimation[i].action = i == 0 ? 1 : 0;
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

HRESULT CSelectChr::OnButtonDown(WPARAM wParam, LPARAM lParam)
{
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

	return 0;
}

HRESULT CSelectChr::OnButtonDown(POINT ptMouse)
{
	m_fIsButtonDown = TRUE;
	return 0;
}

HRESULT	CSelectChr::OnButtonUp(WPARAM wParam, LPARAM lParam)
{
	INT		i;
	int		Px,Py;
	if(m_fIsActive)
	{
		for(i = ID_SEL_CHR_NEW_BTN; i <= ID_SEL_CHR_START_BTN; i++)
		{	m_pBtn[i]->m_nState = BUTTON_STATE_UP;
			if (IsInRect(m_pBtn[i]->m_Rect,LOWORD(lParam), HIWORD(lParam)))
			{	switch(i)
				{	
					case ID_SEL_CHR_NEW_BTN:
					{
						if(m_nChrCount<3)
						{	m_nBlend = 0;
							m_nRenderState = 1;
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
						SendMessage(g_xMainWnd.GetSafehWnd(), WM_DESTROY, NULL, NULL);
						return 0L;
					}
					case ID_SEL_CHR_START_BTN:
					{
						if(m_nSelectChr != SELECT_NONE)
						{	m_nBlend = 0;
							m_pxSList->MoveCurrentToTop();
							m_pxSList->MoveNode(m_nSelectChr);
							g_xClientSocket.OnSelChar(m_pxSList->GetCurrentData()->ChrName);
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

		m_pxrcList->MoveCurrentToTop();
		m_pxSList->MoveCurrentToTop();
		for(i=0; i < m_pxrcList->ListLength(); i++)
		{
			Px = m_pxrcList->GetCurrentData()->nLeft - LOWORD(lParam);
			Py = m_pxrcList->GetCurrentData()->nTop - HIWORD(lParam);
			if(Px < 0 && ((Px +m_pxrcList->GetCurrentData()->nWidth)>0))
			{
				if(Py<0 && ((Py+m_pxrcList->GetCurrentData()->nHeight)>0))
				{
                    if (m_nSelectChr != i) {
                        if (m_nSelectChr >= 0) {
                            m_roleAnimation[m_nSelectChr] = RoleAnimation{};
                            m_roleAnimation[m_nSelectChr].action = 3;
                        }
                        m_roleAnimation[i] = RoleAnimation{};
                        m_roleAnimation[i].action = 1;
                        m_nSelectChr = i;
                        m_pxSList->MoveCurrentToTop();
                        m_pxSList->MoveNode(i);
                        const CHRINFO& role = *m_pxSList->GetCurrentData();
                        g_xSound.PlayRoleVoice(false, role.Class, role.Gender);
                    }

				}
			}
			m_pxrcList->MoveNextNode();
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
		}

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

        // Frame groups/anchors follow Mir3's CharacterSelectionPage.
        static const int lastFrames[30] = {
            10,19,11,7,17, 10,10,10,11,15, 10,11,10,10,14,
            10,17,10,8,16, 10,16,19,11,16, 10,16,10,9,14
        };
        m_pxSList->MoveCurrentToTop();
        m_pxrcList->MoveCurrentToTop();
        for (i = 0; i < m_nChrCount; ++i) {
            const CHRINFO* role = m_pxSList->GetCurrentData();
            const int type = role->Gender + 2 * role->Class;
            if (type >= 0 && type < 6) {
                auto& animation = m_roleAnimation[i];
                animation.elapsed += nLoopTime;
                if (animation.elapsed > 120) {
                    animation.elapsed = 0;
                    if (++animation.frame > lastFrames[animation.action + 5 * type]) {
                        animation.frame = 0;
                        if (animation.action == 1) animation.action = 2;
                        else if (animation.action == 3) animation.action = 0;
                    }
                }
                const int frame = 200 + 60 * (animation.action + 5 * type) + animation.frame;
                const int x = m_nChrCount == 1 ? 300 : (i % 2 ? 350 : 250);
                const int y = i % 2 ? 250 : 210;
                DrawSelectionFrame(*m_pxImage, frame + 20, x, y, SelectionBlend::Shadow);
                const RECT rect = DrawSelectionFrame(*m_pxImage, frame, x, y,
                    animation.action ? SelectionBlend::Normal : SelectionBlend::Inactive);
                DrawSelectionFrame(*m_pxImage, frame + 40, x, y, SelectionBlend::Additive);
                CHRRECT* bounds = m_pxrcList->GetCurrentData();
                bounds->nLeft = rect.left; bounds->nTop = rect.top;
                bounds->nWidth = rect.right - rect.left; bounds->nHeight = rect.bottom - rect.top;
            }
            m_pxSList->MoveNextNode();
            m_pxrcList->MoveNextNode();
        }
		m_xLightFog.ShowLightFog();

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
    const CHRINFO& role = *m_pxSList->GetCurrentData();
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
