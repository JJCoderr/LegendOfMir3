// CharacterProcess.cpp: implementation of the CCharacterProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

// Coordinates are shared by drawing and hit testing for the supplied UI frames.
struct CreateControl { int frame, x, y, width, height; };
static const CreateControl createControls[] = {
    {85, 450, 444, 28, 28}, {88, 491, 444, 28, 28},
    {91, 266, 419, 40, 38}, {94, 308, 419, 40, 38}, {97, 352, 419, 40, 38}
};


////////////////////////////////////////////////
////////////////////////////////////////////////


// Positio of Edit box
#define POS_NAME_X						288
#define POS_NAME_Y						405

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCharacterProcess::CCharacterProcess()
{
	Init();
}

CCharacterProcess::~CCharacterProcess()
{
//	if(m_pBtn)
//	{
//		delete[] m_pBtn;
//		m_pBtn = NULL;
//	}
}

VOID CCharacterProcess::Init()
{
	m_Tick = 0;
	ZeroMemory(m_createBounds, sizeof(m_createBounds));
	m_SelectJob = 0;
	m_SelectSex = 0;
	m_bCreatePending = FALSE;
	m_Progress=PRG_CHAR_SELE;
	m_ConnectionState=CONNECT_FAIL;
	ZeroMemory(m_szErrorMsg,512);
//	m_pBtn = new CMirButton[sizeof(g_CharProcBtnInfo)/sizeof(BUTTONINFO)];
}

VOID CCharacterProcess::Load()
{
	WSAStartup(MAKEWORD(2, 2), &m_wsd);
	m_Image.NewLoad(".\\Data\\interface1c.wil", TRUE, TRUE);
	m_Image2.NewLoad(".\\Data\\GameInter.wil", TRUE, TRUE);
	// 临时
	//
	m_ConnectionState = CONNECT_FAIL;

	g_xClientSocket.ConnectToServer(g_xMainWnd.GetSafehWnd(), g_szServerIP, g_nServerPort);

	g_xChatEditBox.SetLimitText(13);
	SendMessage(g_xChatEditBox.GetSafehWnd(),EM_SETPASSWORDCHAR,NULL,0);

	m_ChrCount =0;
	m_nMessage = 0;
//	m_xMsgBox.Load(&m_Image2);
	m_xMsgBox.CreateWnd(&m_Image2,TRUE,219,110,362,180);	// 勇气
	m_Progress	= PRG_CHAR_SELE;							// Character Selection
	m_xBGround.Create(&m_Image);
	m_xBGround.OpenWnd();
}	

VOID CCharacterProcess::SetNextProc()
{
	D3DMATRIX	matProj;
	DeleteProc();
 	g_xClientSocket.m_pxDefProc = g_xMainWnd.m_pxDefProcess = &g_xGameProc;
	g_xGameProc.Load();
	g_xChatEditBox.SetLimitText(90);
	ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
	D3DUtil_SetProjectionMatrix(matProj, g_PI/2, float(600.0f/800.0f), -1.0f, 1.0f);
    g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj);
	g_bProcState = _GAME_PROC; 
}

VOID CCharacterProcess::DeleteProc()
{
	
	WSACleanup();
	m_Image.Destroy();
	m_Image2.Destroy();
	// 临时
	m_CRectList.ClearAllNodes();
	m_CList.ClearAllNodes();
}


LRESULT CCharacterProcess::DefMainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch ( uMsg )
	{
		case WM_LBUTTONDOWN:
			OnLButtonDown(wParam, lParam);
			break;
		case WM_LBUTTONUP:
			OnLButtonUp(wParam, lParam);
			break;
		case ID_SOCKCLIENT_EVENT_MSG:
			g_xClientSocket.OnSocketMessage(wParam, lParam);
			break;
		case WM_KEYDOWN:
			OnKeyDown(wParam, lParam);
			break;
		case WM_MOUSEON:
			OnMouseOn(wParam,lParam);
			break;
		case WM_MOVE:
			OnWindowMove(wParam,lParam);
			break;
		case WM_MOUSEMOVE:
			OnMouseMove(wParam,lParam);
			break;
		default:
			break;
	}
	return CWHDefProcess::DefMainWndProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CCharacterProcess::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	switch ( wParam )
	{
	case VK_F1:
		m_Progress=PRG_CHAR_SELE;
		break;
	case VK_F2:
		m_Progress=PRG_CREATE_CHAR;
		break;
	}	
	return 0L;
}

LRESULT CCharacterProcess::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	int		i;
	BOOL	fOnButton = FALSE;

	if(m_xMsgBox.IsActive())
	{
		POINT  tPos;
		tPos.x = LOWORD(lParam);
		tPos.y = HIWORD(lParam);
		m_xMsgBox.MoveWnd(tPos);	// Moon
		return 0l;
	}


	if(!m_fIsLButtonClicked)//m_fIsConnected && !m_fIsLButtonClicked)
	{
		switch(m_Progress)
		{
			case PRG_CHAR_SELE:
				m_xSelectChr.OnMouseMove(wParam, lParam);
				break;
			case PRG_CREATE_CHAR:
			{
				for(i = 5; i < 9; i++)
				{
/*					if (m_pBtn[i].CheckMouseOn(LOWORD(lParam), HIWORD(lParam)))
					{
						m_pBtn[i].m_nState = BUTTON_STATE_ON;
						fOnButton = TRUE;
					}
					else
						m_pBtn[i].m_nState = BUTTON_STATE_UP;*/
				}

				break;
			}
		}
		if(fOnButton)
		{
			HCURSOR	hCursor;
			hCursor = LoadCursor(g_xMainWnd.GetSafehInstance(),MAKEINTRESOURCE(IDC_ARROW));
			SetCursor(hCursor);
			DestroyCursor(hCursor); 
		}
//		else
//			g_xMainWnd.OnSetCursor(IDC_ARROW);
	}
	return 0L;
}

LRESULT CCharacterProcess::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{	
	m_fIsLButtonClicked = FALSE;
	if(m_xMsgBox.IsActive())
	{
		POINT ptMouse;
		ptMouse.x = LOWORD(lParam);
		ptMouse.y = HIWORD(lParam);

		m_xMsgBox.OnButtonDown(ptMouse);
	}
	else
	{
		if(m_Progress==PRG_CHAR_SELE)
			m_xSelectChr.OnButtonDown(wParam, lParam);
		if (m_Progress == PRG_CREATE_CHAR)
		{
			for (int i = ID_CREATE_CHR_OK_BTN; i <= ID_CREATE_CHR_TAO_BTN; i++)
			{
/*				if (m_pBtn[i].CheckMouseOn(LOWORD(lParam), HIWORD(lParam)))
				{
					m_pBtn[i].m_nState = BUTTON_STATE_DOWN;
					m_fIsLButtonClicked = TRUE;
				}
				else
					m_pBtn[i].m_nState = BUTTON_STATE_UP;
*/			}
		}
	}
	return 0L;
}

LRESULT CCharacterProcess::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{	
	INT	nRecieve;
	m_ptMousePos.x = LOWORD(lParam);
	m_ptMousePos.y = HIWORD(lParam);

	m_fIsLButtonClicked = FALSE;
	if(m_xMsgBox.IsActive())	// Message Box Process
	{
		MessageBoxProcess(lParam);
	}
	else
	{
		switch(m_Progress)
		{
		case PRG_CHAR_SELE:
			{
				nRecieve = m_xSelectChr.OnButtonUp(wParam, lParam);
				switch(nRecieve)
				{
					case RETURN_SELECT_NEW_BTN:
					{
						break;
					}
					case RETURN_SELECT_NEW_ERR:
					{
						m_nMessage = 3;
						SetErrorMessage(m_nMessage,m_szErrorMsg);
						m_xMsgBox.ShowMessageBox(m_szErrorMsg,MSG_BTN_OK,_WNDIMGINDX_MSGBOX1);
						break;
					}
					case RETURN_SELECT_DEL_BTN:
					{
						m_nMessage = 5;
						SetErrorMessage(m_nMessage,m_szErrorMsg);
						m_xMsgBox.ShowMessageBox(m_szErrorMsg,MSG_BTN_YESNO,_WNDIMGINDX_MSGBOX1);
						break;
					}
					case RETURN_SELECT_DEL_ERR:
					{
						m_nMessage = 8;
						SetErrorMessage(m_nMessage,m_szErrorMsg);
						m_xMsgBox.ShowMessageBox(m_szErrorMsg,MSG_BTN_OK,_WNDIMGINDX_MSGBOX1);
						break;
					}
					case RETURN_SELECT_START_BTN:
					{
						break;
					}
					case RETURN_SELECT_START_ERR:
					{
						m_nMessage = 7;
						SetErrorMessage(m_nMessage,m_szErrorMsg);
						m_xMsgBox.ShowMessageBox(m_szErrorMsg,MSG_BTN_OK,_WNDIMGINDX_MSGBOX1);
						break;
					}
				}
				break;
			}

        case PRG_CREATE_CHAR:
        {
            if (m_bCreatePending) break;
            for (int gender = 0; gender < 2; ++gender)
                if (PtInRect(&m_createBounds[gender], m_ptMousePos)) m_SelectSex = (BYTE)gender;
            for (int i = 0; i < 5; ++i) {
                const CreateControl& control = createControls[i];
                RECT rect = {control.x, control.y, control.x + control.width, control.y + control.height};
                if (!PtInRect(&rect, m_ptMousePos)) continue;
                if (i == 1) {
                    m_Progress = PRG_CHAR_SELE;
                    m_xBGround.SetRenderState(_RENDER_STILL_1);
                    m_xSelectChr.OpenWnd();
                } else if (i >= 2) {
                    m_SelectJob = (BYTE)(i - 2);
                } else {
                    GetWindowTextA(g_xChatEditBox.GetSafehWnd(), m_szChrName, sizeof(m_szChrName));
                    if (!m_szChrName[0]) break;
                    if (strpbrk(m_szChrName, " /@?'\"\\.,:;`~!#$%^&*()-_+=|[{}]<>")) {
                        m_nMessage = 0;
                        strcpy(m_szErrorMsg, "Please use letters, numbers or Chinese characters for the name.");
                        m_xMsgBox.ShowMessageBox(m_szErrorMsg, MSG_BTN_OK, 1240);
                        break;
                    }
                    m_bCreatePending = TRUE;
                    g_xClientSocket.OnNewChar(m_szChrName, 0, m_SelectJob, m_SelectSex);
                }
                break;
            }
            break;
        }

		}
	}
	return 0L;
}

LRESULT CCharacterProcess::OnMouseOn(WPARAM wParam, LPARAM lParam)
{	
/*	UINT	t_ID;
	t_ID=(UINT)wParam;
	m_ButtonID = t_ID;		// Upon Mouse*/
	return 0L;
}

LRESULT CCharacterProcess::OnWindowMove(WPARAM wParam,LPARAM lParam)
{

	ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
	g_xMainWnd.OnMove(wParam, lParam);
	return 0L;
}

// **************************************************************************************
//
//		Socket
//
// **************************************************************************************

VOID CCharacterProcess::OnConnectToServer()
{
	m_ConnectionState = CONNECT_SUCCESS;// Connection Success
	
	g_xClientSocket.OnQueryChar();
}

char* CCharacterProcess::OnMessageReceive(CHAR* pszMessage)
{
	char	*pszFirst = pszMessage;
	char	*pszEnd;
	
	while (pszEnd = strchr(pszFirst, '!'))
	{
		*pszEnd = '\0';

		OnSocketMessageRecieve(pszFirst + 1);

		if (*(pszEnd + 1) == '#')
			pszFirst = pszEnd + 1;
		else
			return NULL;
	}

	return pszFirst;
}

void CCharacterProcess::OnSocketMessageRecieve(char* pszMsg)
{
	_TDEFAULTMESSAGE	tdm;
	INT	nChrPos[] = { 350, 280, 350, 250, 320, 390 };

	fnDecodeMessage(&tdm, pszMsg);

	switch(tdm.wIdent)
	{
        case SM_QUERYCHR:
        {
            CHRINFO records[3] = {};
            static_assert(sizeof(CHRINFO) == 23, "Character list wire layout changed");
            if (tdm.wParam > 3) break;
            const int count = tdm.wParam;
            const int decoded = fnDecode6BitBuf(pszMsg + DEFBLOCKSIZE, (char*)records, sizeof(records));
            if (decoded != count * sizeof(CHRINFO)) break;
            m_CList.ClearAllNodes();
            m_CRectList.ClearAllNodes();
            m_ChrCount = count;
            m_SelectChr = count ? SELECT_FIRST : SELECT_NONE;
            for (int i = 0; i < count; ++i) {
                records[i].ChrName[sizeof(records[i].ChrName) - 1] = 0;
                CHRINFO* info = new CHRINFO(records[i]);
                m_CList.AddNode(info);
                CHRRECT* rect = new CHRRECT;
                rect->nLeft = 200 + i * 100;
                rect->nTop = 200;
                rect->nWidth = 90;
                rect->nHeight = 140;
                m_CRectList.AddNode(rect);
            }
            m_xSelectChr.Create(&m_Image, &m_CList, &m_CRectList, count);
            m_xSelectChr.OpenWnd();
            break;
        }
		case	SM_NEWCHR_SUCCESS:		// New character Create Success
		{
			m_bCreatePending = FALSE;
			m_Progress = PRG_CHAR_SELE;
			m_xBGround.SetRenderState(_RENDER_STILL_1);
			g_xClientSocket.OnQueryChar();
			break;
		}
        case SM_NEWCHR_FAIL:
            m_bCreatePending = FALSE;
            m_Progress = PRG_CREATE_CHAR;
            m_nMessage = 0;
            sprintf(m_szErrorMsg, "Character creation failed (code %u).", tdm.wParam);
            m_xMsgBox.ShowMessageBox(m_szErrorMsg, MSG_BTN_OK, 1240);
            break;
        case SM_STARTPLAY:
        {
            char endpoint[256] = {};
            fnDecode6BitBuf(pszMsg + DEFBLOCKSIZE, endpoint, sizeof(endpoint)-1);
            char* port = strchr(endpoint, '/');
            if (port) *port++ = 0;
            lstrcpynA(g_szServerIP, endpoint, sizeof(g_szServerIP));
            g_nServerPort = port ? atoi(port) :
                GetPrivateProfileIntA("Initial","GameGatePort",7200,".\\Mir3.ini");
            g_xClientSocket.DisconnectToServer();
            m_xSelectChr.m_nRenderState = 2;
            break;
        }
        case SM_STARTFAIL:
            m_nMessage = 0;
            strcpy(m_szErrorMsg,"Game server is unavailable. Please try again.");
            m_xMsgBox.ShowMessageBox(m_szErrorMsg,MSG_BTN_OK,1240);
            break;

	}
/*	_TDEFAULTMESSAGE	tdm;
	char				szDecodeMsg[256];
	char				szTmpDecodeMsg[512];

	ZeroMemory(szTmpDecodeMsg, 512);
	strcpy(szTmpDecodeMsg, pszMsg);

	char *pszData = &szTmpDecodeMsg[1];

	fnDecodeMessage(&tdm, pszData);

	if(lstrlen(szTmpDecodeMsg) >= DEFBLOCKSIZE + 1)
		fnDecode6BitBuf((szTmpDecodeMsg + DEFBLOCKSIZE + 1), szDecodeMsg, sizeof(szDecodeMsg));

	switch(tdm.wIdent)
	{
	case	SM_QUERYCHR:			// Getting character List
		{
			pCHRINFO ChrInfo;
			pCHRRECT tChRect;
			m_CRectList.ClearAllNodes();
			m_CList.ClearAllNodes();
			if(tdm.wParam!=0)	// 角色存在时
			{
				ChrInfo = new CHRINFO[tdm.wParam]; // 按收到的个数创建角色信息.
				m_SelectChr = SELECT_FIRST;
				m_ChrCount = tdm.wParam;
				fnDecode6BitBuf((szTmpDecodeMsg + DEFBLOCKSIZE + 1), (char*)ChrInfo, sizeof(CHRINFO)*(tdm.wParam));
				pCHRINFO  tChInfo;
				for(int i = 0 ;i<tdm.wParam;i++)
				{
					// 角色不存在时?
					// 角色 Info
					tChInfo = new CHRINFO;
					strcpy(tChInfo->ChrName,ChrInfo[i].ChrName);
					tChInfo->Class=ChrInfo[i].Class;
					tChInfo->Gender=ChrInfo[i].Gender;
					tChInfo->Level=ChrInfo[i].Level;
					m_CList.AddNode(tChInfo);

					// 角色 Rect 需要修改  // 临时
					tChRect = new CHRRECT;

					m_tImage1.NewSetIndex((2-(INT)(tChInfo->Class)));

					switch(tdm.wParam)
					{
					case 1:
						tChRect->nLeft = 350;
						break;
					case 2:
						{
							switch(i)
							{
							case 0:
								tChRect->nLeft = 280;
								break;
							case 1:
								tChRect->nLeft = 420;
								break;
							}
							break;
						}
					case 3:
						{
							switch(i)
							{
							case 0:
								tChRect->nLeft = 180;
								break;
							case 1:
								tChRect->nLeft = 350;
								break;
							case 2:
								tChRect->nLeft  = 480;
								break;
							}
							break;
						}
					}
					
					tChRect->nTop  = 250+((2-tChInfo->Class)/2)*30;
					tChRect->nWidth = m_tImage1.m_lpstNewCurrWilImageInfo->shWidth;
					tChRect->nHeight = m_tImage1.m_lpstNewCurrWilImageInfo->shHeight;
					m_CRectList.AddNode(tChRect);
				}
				delete ChrInfo;
			}
			else
			{	// 角色不存在时是?
				m_SelectChr = SELECT_NONE;
			}
			break;
		}
	case	SM_NEWCHR_SUCCESS:		// New character Create Success
		{
		m_Progress = PRG_CREA_TO_SEL;
		g_xClientSocket.OnQueryChar();
		break;
		}
	case	SM_NEWCHR_FAIL:			// New character Create Fail
		{
			m_nMessage = tdm.wParam;		// 传递 Message Parameter
			SetErrorMessage(m_nMessage,m_szErrorMsg);
			m_xMsgBox.ShowMessageBox(m_szErrorMsg,1,_WNDIMGINDX_MSGBOX1);
			m_Progress	= PRG_CREATE_CHAR;	// Create New Character
		}
		break; 
	case	SM_DELCHR_SUCCESS:		// Delete character Success

		g_xClientSocket.OnQueryChar();
		m_Progress	= PRG_CHAR_SELE;	// Character Selection

		break;
	case	SM_DELCHR_FAIL:			// Delete character Fail
		{
			m_nMessage = tdm.wParam;
			SetErrorMessage(m_nMessage,m_szErrorMsg);
			m_xMsgBox.ShowMessageBox(m_szErrorMsg,1,_WNDIMGINDX_MSGBOX1);
			// 1. 删除不存在的角色时
			// 2. 未知 Error
			// 3. 
			m_Progress	= PRG_CHAR_SELE;	// Character Selection
		}
		break;
	case	SM_STARTPLAY:			// Start Game
		{
			int Count;
			Count = strlen(szDecodeMsg);
			for(int i = 0 ; i< Count ; i++)
			{
				if(szDecodeMsg[i]==':')
				{
					strncpy(g_szServerIP,szDecodeMsg,i);
					g_nServerPort = atoi(szDecodeMsg+i+1);
				}
			}
			g_xClientSocket.DisconnectToServer();		// 断开与 Char sever 的连接
			m_Progress=PRG_SEL_TO_GAME;
			break;
		}
	case	SM_STARTFAIL:
		{	// Fail to start game
			m_Progress	= PRG_QUIT;	// Quit for Some Reason
			SendMessage(g_xMainWnd.GetSafehWnd(), WM_DESTROY, NULL, NULL);
		}
		break;
	case	SM_QUERYCHR_FAIL:
		{	// Getting character List Fail
			m_Progress	= PRG_QUIT;	// Quit for Some Reason
			SendMessage(g_xMainWnd.GetSafehWnd(), WM_DESTROY, NULL, NULL);
		}
		break;
	case	SM_OUTOFCONNECTION:
		{	// Connection Lost(close Connection)
			m_Progress	= PRG_QUIT;	// Quit for Some Reason
			SendMessage(g_xMainWnd.GetSafehWnd(), WM_DESTROY, NULL, NULL);
		}
		break;
	}
	return 0L; */
}
		/**/
VOID CCharacterProcess::RenderScene(INT nLoopTime)
{
	m_xBGround.Render(nLoopTime);
	switch(m_Progress)
	{
		case PRG_CHAR_SELE:
		{
			ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
			switch(m_xSelectChr.m_nRenderState)
			{
				case 11:	// Zoom in
				{
					m_xBGround.SetRenderState(_RENDER_AVI_IN);
					m_Progress = PRG_SEL_TO_CREA;
					m_xSelectChr.CloseWnd();
					break;
				}
				case 12:	// Zoom Out
				{
					m_xBGround.SetRenderState(_RENDER_AVI_START);	
					m_Progress = PRG_SEL_TO_GAME;
					m_xSelectChr.CloseWnd();
					break;
				}
				case 19:	// Select;
				{
					m_xSelectChr.m_nRenderState = 0;
				}
			}
			m_xSelectChr.Render(nLoopTime);
			break;
		}
		case PRG_SEL_TO_CREA:
			ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
			if(m_xBGround.GetRenderState()==_RENDER_STILL_2)
				m_Progress = PRG_CREATE_CHAR;
			break;
		case PRG_CREATE_CHAR:
			ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_SHOW);
			RenderCharCreate(nLoopTime);
			break;
		case PRG_CREA_TO_SEL:
			ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
			if(m_xBGround.GetRenderState() == _RENDER_STILL_1)
			{	m_Progress = PRG_CHAR_SELE;
				m_xSelectChr.OpenWnd();
			}
			break;
		case PRG_SEL_TO_GAME:
			ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
			if(m_xBGround.GetRenderState()==_RENDER_STARTGAME)
			{	m_Progress	= PRG_PLAY_GAME;				// Play Game
				SetNextProc();
			}
			break;
	}
	m_xMsgBox.RenderMessageBox(0);
}

VOID CCharacterProcess::ShowStatus(INT nStartX, INT nStartY)
{
/*	TCHAR			szBuff[MAX_PATH];
	sprintf(szBuff, "Character Procedure");*/

	CWHDefProcess::ShowStatus(nStartX, nStartY+20);
}


VOID CCharacterProcess::RenderCharCreate(INT nLoopTime)
{
    MoveWindow(g_xChatEditBox.GetSafehWnd(),
        g_xMainWnd.m_rcWindow.left + POS_NAME_X,
        g_xMainWnd.m_rcWindow.top + POS_NAME_Y, 75, 13, TRUE);
    // Mir3 br_godot_test_261115: CreateCharacterPage positions/action 4.
    static const POINT positions[6] = {{110,110},{110,120},{110,120},{400,160},{420,115},{425,118}};
    static const int lastFrame[6] = {17,14,16,15,16,14};
    m_Tick += nLoopTime;
    for (int gender = 0; gender < 2; ++gender) {
        const int role = m_SelectJob + 3 * gender;
        const int first = 440 + 600 * m_SelectJob + 300 * gender;
        const int frame = first + (gender == m_SelectSex ? (m_Tick / 121) % (lastFrame[role] + 1) : 0);
        if (!m_Image.NewSetIndex(frame)) continue;
        const NEWWILIMAGEINFO& info = *m_Image.m_lpstNewCurrWilImageInfo;
        const int x = positions[role].x + info.shPX, y = positions[role].y + info.shPY;
        SetRect(&m_createBounds[gender], x, y, x + info.shWidth, y + info.shHeight);
        g_xMainWnd.DrawWithImageForComp(x, y, info.shWidth, info.shHeight, (WORD*)m_Image.m_pbCurrImage);
    }
    if (m_Image.NewSetIndex(81))
        g_xMainWnd.DrawWithImageForComp(247,384,m_Image.m_lpstNewCurrWilImageInfo->shWidth,
            m_Image.m_lpstNewCurrWilImageInfo->shHeight,(WORD*)m_Image.m_pbCurrImage);
    for (int i = 0; i < 5; ++i) {
        const CreateControl& control = createControls[i];
        const int frame = control.frame + (i >= 2 && m_SelectJob == i - 2 ? 1 : 0);
        if (m_Image.NewSetIndex(frame))
            g_xMainWnd.DrawWithImageForComp(control.x, control.y,
                m_Image.m_lpstNewCurrWilImageInfo->shWidth,
                m_Image.m_lpstNewCurrWilImageInfo->shHeight, (WORD*)m_Image.m_pbCurrImage);
    }
}

VOID CCharacterProcess::SetErrorMessage(INT nMsg,CHAR* szMgs)
{
	HINSTANCE hLib;
	hLib = LoadLibrary(MESSAGE_DLL_1); // Load Dll
	if(nMsg!=99 && nMsg!=0)
	{
		LoadString(hLib,(nMsg+199),szMgs,512);
	}
	else 
	{
		if(nMsg!=0)
			LoadString(hLib,9999,szMgs,512);
		else
			ZeroMemory(szMgs,512);
	}
	FreeLibrary(hLib);
}


BOOL CCharacterProcess::MessageBoxProcess(LPARAM lParam)
{
	HRESULT hResult;
	POINT	ptMouse;
	ptMouse.x = LOWORD(lParam);
	ptMouse.y = HIWORD(lParam);
	hResult  = m_xMsgBox.OnButtonUp(ptMouse);
	if((hResult&HAVE_EDITOBOX)==HAVE_EDITOBOX)
	{
		// edit box exist
		switch(hResult^HAVE_EDITOBOX)
		{
		case CLICK_OK:
			{
				m_xMsgBox.HideMessageBox();
/*					switch(m_nMessage)
				{
				}*/
				break;
			}
		case CLICK_YES:
			{
				m_xMsgBox.HideMessageBox();
/*					switch(m_nMessage)
				{
				}*/
				break;
			}
		case CLICK_NO:
			{
				m_xMsgBox.HideMessageBox();
/*					switch(m_nMessage)
				{
				}*/
				break;
			}
		case CLICK_CANCLE:
			{
				m_xMsgBox.HideMessageBox();
/*					switch(m_nMessage)
				{
				}*/
				break;
			}
		}
	}
	else
	{
		// edit box not exist
		switch(hResult)
		{
		case CLICK_OK:
			{
				m_xMsgBox.HideMessageBox();
				switch(m_nMessage)
				{
				case 103:	//
					{
						PostQuitMessage(NULL);
						m_nMessage = 0;
						break;
					}
				}
				break;
			}
		case CLICK_YES:
			{
				m_xMsgBox.HideMessageBox();
				switch(m_nMessage)
				{
					case 5:
					{
						m_CList.MoveCurrentToTop();
						m_CList.MoveNode(m_xSelectChr.m_nSelectChr);
						g_xClientSocket.OnDelChar(m_CList.GetCurrentData()->ChrName);

						g_xClientSocket.OnQueryChar();

						m_ChrCount =0 ;
						break;
					}
				}
				break;
			}
		case CLICK_NO:
			{
				m_xMsgBox.HideMessageBox();
/*					switch(m_nMessage)
				{
				}*/
				break;
			}
		case CLICK_CANCLE:
			{
				m_xMsgBox.HideMessageBox();
/*					switch(m_nMessage)
				{
				}*/
				break;
			}
		}
	}
	return TRUE;
}
