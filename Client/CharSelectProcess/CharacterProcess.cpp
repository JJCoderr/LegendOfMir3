// CharacterProcess.cpp: implementation of the CCharacterProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LoginProcess/LoginLayout.h"
#include "Common/LegacyPanel.h"
#include <vector>
#include "SelectionDialog.h"

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
    for(auto& role:m_creationAnimation) role.Start(RoleAnimation::Create);
	ZeroMemory(m_createBounds, sizeof(m_createBounds));
	m_SelectJob = 0;
	m_SelectSex = 0;
	m_bCreatePending = FALSE;
	m_Progress=PRG_CHAR_SELE;
	m_ConnectionState=CONNECT_FAIL;
	ZeroMemory(m_szErrorMsg,512);
//	m_pBtn = new CMirButton[sizeof(g_CharProcBtnInfo)/sizeof(BUTTONINFO)];
}

VOID CCharacterProcess::Load(bool connect)
{
    m_offline=!connect;m_lastUrl.clear();
	WSAStartup(MAKEWORD(2, 2), &m_wsd);
	m_Image.NewLoad(".\\Data\\interface1c.wil", TRUE, TRUE);
	m_Image2.NewLoad(".\\Data\\GameInter.wil", TRUE, TRUE);
	// 临时
	//
	m_ConnectionState = CONNECT_FAIL;

	if (connect) {
        lstrcpynA(m_selectionHost,g_szServerIP,sizeof(m_selectionHost));
        m_selectionPort=g_nServerPort;
        g_xClientSocket.ConnectToServer(g_xMainWnd.GetSafehWnd(),g_szServerIP,g_nServerPort);
    }
	m_messages.Load(".\\CmList.dat");

	g_xChatEditBox.SetLimitText(13);
	SendMessage(g_xChatEditBox.GetSafehWnd(),EM_SETPASSWORDCHAR,NULL,0);

	m_returnWaiting = false;m_selectionElapsed=SelectionRoster::Delay;m_pendingRoster.reset();m_pendingNotice=0;
    m_xMsgBox.HideMessageBox();
    m_xMsgBox.SetOrigin(140,110);
    m_CList.ClearAllNodes();m_CRectList.ClearAllNodes();
    m_xSelectChr.CloseWnd();m_xSelectChr.m_nRenderState=0;
	m_ChrCount =0;
	m_nMessage = 0;
//	m_xMsgBox.Load(&m_Image2);
	m_xMsgBox.CreateWnd(&m_Image2,TRUE,219,110,362,180);	// 勇气
	m_Progress	= PRG_CHAR_SELE;							// Character Selection
	m_xBGround.Create(&m_Image);
	m_xBGround.OpenWnd();
    m_xSelectChr.Create(&m_Image,&m_CList,&m_CRectList,0);m_xSelectChr.OpenWnd();
    if(connect) BeginRosterWait();
}	

bool CCharacterProcess::PrepareReturn()
{
    CWHWilImageData selection,notice;
    if(!selection.NewLoad("./Data/interface1c.wil",TRUE,TRUE) ||
       !notice.NewLoad("./Data/GameInter.wil",TRUE,TRUE)) return false;
    constexpr int SelectionBackground=50,SelectionLastFrame=58;
    for(int frame=SelectionBackground;frame<=SelectionLastFrame;++frame)
        if(!selection.NewSetIndex(frame)) return false;
    return notice.NewSetIndex(CLoginNotice::WaitingFrame)!=FALSE;
}

void CCharacterProcess::LoadReturn(bool connect)
{
    lstrcpynA(g_szServerIP,m_selectionHost,sizeof(g_szServerIP));
    g_nServerPort=m_selectionPort;
    Load(connect);
    BeginRosterWait();
}

void CCharacterProcess::Preview(const char* page)
{
    Load(false);
    auto* role = new SelectionRoleInfo{};
    lstrcpyA(role->ChrName, "MirPreview");
    role->Level = 12;
    m_CList.AddNode(role);
    m_CRectList.AddNode(new CHRRECT{});
    m_ChrCount = 1;
    if (strcmp(page, "select-three") == 0) {
        for (int i = 1; i < 3; ++i) {
            auto* extra = new SelectionRoleInfo{};
            lstrcpyA(extra->ChrName, i == 1 ? "MagePreview" : "TaoPreview");
            extra->Class = BYTE(i);
            extra->Level = 12;
            m_CList.AddNode(extra);
            m_CRectList.AddNode(new CHRRECT{});
            ++m_ChrCount;
        }
    }
    m_xSelectChr.Create(&m_Image, &m_CList, &m_CRectList, m_ChrCount);
    m_xSelectChr.OpenWnd();
    if (strcmp(page, "create") == 0) {
        m_xBGround.SetRenderState(_RENDER_STILL_2);
        OpenCreation();
    } else if (strcmp(page, "delete") == 0) {
        OpenDeleteNotice();
    }
}

void CCharacterProcess::OpenDeleteNotice()
{
    if(m_xSelectChr.m_nSelectChr<0 || m_xSelectChr.m_nSelectChr>=m_ChrCount) {ShowNotice(SelectionDialog::DeleteMissing);return;}
    m_nMessage = SelectionDialog::DeleteConfirmation;
    m_xMsgBox.ShowMessageBox(m_messages.Get(SelectionDialog::DeleteConfirmation).c_str(), MSG_BTN_YESNO, 0);
    HWND editor = g_xChatEditBox.GetSafehWnd();
    g_xChatEditBox.SetLimitText(90);
    SetWindowTextA(editor, "");
    g_xChatEditBox.MoveScaled(g_xMainWnd.m_rcWindow.left + 164,
        g_xMainWnd.m_rcWindow.top + 338, 309, 15, TRUE);
    ShowWindow(editor, SW_SHOW);
    SetFocus(editor);
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
    m_xBGround.AllStopAvi();m_pendingRoster.reset();m_pendingNotice=0;
	
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
    case WM_COMMAND:
        if((HWND)lParam==g_xChatEditBox.GetSafehWnd() && HIWORD(wParam)==EN_CHANGE) {
            g_xChatEditBox.Changed();return 0;
        }
        break;
    case WM_CTLCOLOREDIT:
        if ((HWND)lParam == g_xChatEditBox.GetSafehWnd()) {
            SetBkColor((HDC)wParam, RGB(0, 0, 0));
            SetTextColor((HDC)wParam, RGB(255, 255, 255));
            return (LRESULT)GetStockObject(BLACK_BRUSH);
        }
        break;
		case WM_LBUTTONDOWN:
			OnLButtonDown(wParam, lParam);
			break;
		case WM_LBUTTONUP:
			OnLButtonUp(wParam, lParam);
			break;
		case ID_SOCKCLIENT_EVENT_MSG:
			g_xClientSocket.OnSocketMessage(wParam, lParam);
			break;
        case _WM_USER_MSG_INPUTTED:
            SetWindowTextA(g_xChatEditBox.GetSafehWnd(),g_xChatEditBox.m_szInputMsg);
            OnKeyDown(VK_RETURN,0);return 0;
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
    if(m_returnWaiting || m_Progress==PRG_SEL_TO_CREA || m_Progress==PRG_CREA_TO_SEL || m_Progress==PRG_SEL_TO_GAME) return 0;
    if(m_xMsgBox.IsActive()) FinishNotice(m_xMsgBox.OnKeyDown(wParam));
    else if(m_Progress==PRG_CREATE_CHAR && wParam==VK_RETURN) SubmitCreation();
    return 0L;
}

LRESULT CCharacterProcess::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    if(m_Progress==PRG_SEL_TO_CREA || m_Progress==PRG_CREA_TO_SEL || m_Progress==PRG_SEL_TO_GAME) return 0;
    m_ptMousePos = POINT{LOWORD(lParam), HIWORD(lParam)};
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
    if(m_Progress==PRG_SEL_TO_CREA || m_Progress==PRG_CREA_TO_SEL || m_Progress==PRG_SEL_TO_GAME) return 0;
    m_createPressed = -1;
    m_ptMousePos = POINT{LOWORD(lParam), HIWORD(lParam)};
    if (m_Progress == PRG_CREATE_CHAR && !m_xMsgBox.IsActive() && !m_bCreatePending) {
            for (int gender = 0; gender < 2; ++gender)
                if (PtInRect(&m_createBounds[gender], m_ptMousePos) && gender != m_SelectSex) {
                    m_SelectSex = (BYTE)gender;
                    m_creationAnimation[gender].Start(RoleAnimation::Create);
                }

        for (int i = 0; i < 5; ++i) {
            const auto& c = createControls[i];
            RECT r = {c.x,c.y,c.x+c.width,c.y+c.height};
            if (PtInRect(&r,m_ptMousePos)) m_createPressed = i;
        }
    }
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
    if(m_Progress==PRG_SEL_TO_CREA || m_Progress==PRG_CREA_TO_SEL || m_Progress==PRG_SEL_TO_GAME) return 0;
	INT	nRecieve;
    m_createPressed = -1;
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
					case RETURN_SELECT_EXIT_BTN:
                        ExitSelection();break;
					case RETURN_SELECT_NEW_ERR:
					{
                        ShowNotice(SelectionDialog::Full);
						break;
					}
					case RETURN_SELECT_DEL_BTN:
					{
						OpenDeleteNotice();
						break;
					}
					case RETURN_SELECT_DEL_ERR:
					{
                        ShowNotice(SelectionDialog::DeleteMissing);
						break;
					}
					case RETURN_SELECT_START_BTN:
					{
						break;
					}
					case RETURN_SELECT_START_ERR:
					{
                        ShowNotice(SelectionDialog::StartMissing);
						break;
					}
				}
				break;
			}

        case PRG_CREATE_CHAR:
        {
            if (m_bCreatePending) break;
            RECT editorFocus={294,404,371,419};
            if(PtInRect(&editorFocus,m_ptMousePos)) SetFocus(g_xChatEditBox.GetSafehWnd());
            for (int i = 0; i < 5; ++i) {
                const CreateControl& control = createControls[i];
                RECT rect = {control.x, control.y, control.x + control.width, control.y + control.height};
                if (!PtInRect(&rect, m_ptMousePos)) continue;
                g_xSound.PlayLobbyEffect(LoginLayout::ClickSound);
                if (i == 1) {
                    g_xSound.UpdateLobbyAudio(CMirSound::LobbyPage::None,0);
                    m_Progress = PRG_CREA_TO_SEL;
                    m_xBGround.SetRenderState(_RENDER_AVI_OUT);
                    ShowWindow(g_xChatEditBox.GetSafehWnd(),SW_HIDE);
                    RequestRoster();
                } else if (i >= 2) {
                    m_SelectJob = (BYTE)(i - 2);
                    for(auto& role:m_creationAnimation) role.Start(RoleAnimation::Create);
                } else {
                    SubmitCreation();
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
            char body[256]={};
            const int decoded=fnDecode6BitBuf(pszMsg+DEFBLOCKSIZE,body,sizeof(body));
            if(decoded<0 || decoded>sizeof(body)) break;
            SelectionRoster roster;
            const bool legacy=tdm.nRecog==0 && tdm.wParam!=0;
            if(!(legacy?roster.ParseLegacy(tdm.wParam,body,decoded):roster.Parse(unsigned(tdm.nRecog),std::string_view(body,decoded)))) break;
            m_pendingNotice=0;
            if(m_selectionElapsed<SelectionRoster::Delay) m_pendingRoster=roster;
            else ApplyRoster(roster);
            break;
        }
		case	SM_NEWCHR_SUCCESS:		// New character Create Success
		{
			m_bCreatePending = FALSE;
            g_xSound.UpdateLobbyAudio(CMirSound::LobbyPage::None,0);
            m_Progress = PRG_CREA_TO_SEL;
            m_xBGround.SetRenderState(_RENDER_AVI_OUT);
            ShowWindow(g_xChatEditBox.GetSafehWnd(),SW_HIDE);
            RequestRoster();
			break;
		}
        case SM_NEWCHR_FAIL:
            m_bCreatePending=FALSE;m_Progress=PRG_CREATE_CHAR;
            ShowNotice(SelectionDialog::CreationFailure(unsigned(tdm.nRecog)));break;
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
            g_xSound.UpdateLobbyAudio(CMirSound::LobbyPage::None,0);
            g_xSound.PlayLobbyEffect("StartGame.wav");
            m_xBGround.SetRenderState(_RENDER_AVI_START);
            m_Progress=PRG_SEL_TO_GAME;m_xSelectChr.CloseWnd();
            ShowWindow(g_xChatEditBox.GetSafehWnd(),SW_HIDE);
            break;
        }
        case SM_STARTFAIL: ShowNotice(SelectionDialog::StartFailed);break;
        case SM_QUERYCHR_FAIL: ShowNotice(SelectionDialog::QueryFailed);break;
        case SM_OUTOFCONNECTION: ShowNotice(SelectionDialog::Disconnected);break;
        case SM_DELCHR_SUCCESS:
        case SM_DELCHR_FAIL: RequestRoster();break;

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
		RequestRoster();
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

		RequestRoster();
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
    if(m_Progress==PRG_CHAR_SELE || m_Progress==PRG_CREATE_CHAR) AdvanceRoster(DWORD(max(0,nLoopTime)));
    using Page = CMirSound::LobbyPage;
    g_xSound.UpdateLobbyAudio(m_Progress == PRG_CHAR_SELE ? Page::Selection :
        m_Progress == PRG_CREATE_CHAR ? Page::Creation : Page::None, nLoopTime);
	m_xBGround.Render(nLoopTime);
	switch(m_Progress)
	{
		case PRG_CHAR_SELE:
		{
			if (!m_xMsgBox.IsActive() || m_nMessage != SelectionDialog::DeleteConfirmation)
				ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
			switch(m_xSelectChr.m_nRenderState)
			{
				case 11:	// Zoom in
				{
                    g_xSound.UpdateLobbyAudio(Page::None, 0);
                    g_xSound.PlayLobbyEffect("CreateChr.wav");
					m_xBGround.SetRenderState(_RENDER_AVI_IN);
					m_Progress = PRG_SEL_TO_CREA;
					m_xSelectChr.CloseWnd();
					break;
				}
				case 12:	// Zoom Out
				{
                    g_xSound.UpdateLobbyAudio(Page::None, 0);
                    g_xSound.PlayLobbyEffect("StartGame.wav");
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
			m_xSelectChr.Render(m_returnWaiting?0:nLoopTime);
			break;
		}
		case PRG_SEL_TO_CREA:
			ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
			if(m_xBGround.GetRenderState()==_RENDER_STILL_2)
				OpenCreation();
			break;
		case PRG_CREATE_CHAR:
			ShowWindow(g_xChatEditBox.GetSafehWnd(), m_xMsgBox.IsActive() ? SW_HIDE : SW_SHOW);
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
	if(m_Progress!=PRG_SEL_TO_CREA && m_Progress!=PRG_CREA_TO_SEL && m_Progress!=PRG_SEL_TO_GAME) m_xMsgBox.RenderMessageBox(0);
}

VOID CCharacterProcess::ShowStatus(INT nStartX, INT nStartY)
{
/*	TCHAR			szBuff[MAX_PATH];
	sprintf(szBuff, "Character Procedure");*/

	CWHDefProcess::ShowStatus(nStartX, nStartY+20);
}


// Mir3 hud_view.cpp: creation description wraps at 430 logical pixels.
void CCharacterProcess::DrawCreationDescription()
{
    const std::string& description = m_messages.Get(216 + m_SelectJob);
    std::vector<std::string> lines;
    HWND hwnd = g_xMainWnd.GetSafehWnd();
    HDC dc = GetDC(hwnd);
    const char songti[] = {char(0xcb), char(0xce), char(0xcc), char(0xe5), 0};
    HFONT font = g_xMainWnd.CreateGameFont(songti, 9, 0, FW_NORMAL);
    HGDIOBJ previous = SelectObject(dc, font);
    const char* start = description.c_str();
    for (const char* end = start; *end;) {
        end = CharNextA(end);
        SIZE extent = {};
        GetTextExtentPoint32A(dc, start, int(end - start), &extent);
        if (extent.cx >= 430 || !*end) {
            lines.emplace_back(start, end);
            start = end;
        }
    }
    SelectObject(dc, previous);
    ReleaseDC(hwnd, dc);
    DeleteObject(font);

    RECT box = {100, 15, 550, 35 + 18 * LONG(lines.size() + 1)};
    DrawLegacyPanel(box, RGB(50, 50, 50), 50, RGB(150, 150, 150));
    std::string title = m_messages.Get(211 + m_SelectSex) + m_messages.Get(213 + m_SelectJob);
    const std::string& face = m_messages.Get(1074);
    g_xMainWnd.PutsHan(NULL, 110, 25, 0x96c8fa, 0, &title[0],
        g_xMainWnd.CreateGameFont(face.c_str(), 10, 0, FW_BOLD));
    int y = 50;
    for (auto& line : lines) {
        g_xMainWnd.PutsHan(NULL, 110, y, 16775930, 0, &line[0]);
        y += 18;
    }
}

VOID CCharacterProcess::RenderCharCreate(INT nLoopTime)
{
    g_xChatEditBox.MoveScaled(
        g_xMainWnd.m_rcWindow.left + POS_NAME_X,
        g_xMainWnd.m_rcWindow.top + POS_NAME_Y, 75, 13, TRUE);
    DrawCreationDescription();
    // Mir3 br_godot_test_261115: CreateCharacterPage positions/action 4.
    static const POINT positions[6] = {{110,110},{110,120},{110,120},{400,160},{420,115},{425,118}};

    for (int gender = 0; gender < 2; ++gender) {
        const int role = m_SelectJob + 3 * gender;
        auto& animation=m_creationAnimation[gender];
        const auto& anchor=positions[role];
        if(animation.Due(nLoopTime) && m_Image.NewSetIndex(animation.ImageFrame(m_SelectJob,gender)+1)) {
            const auto& info=*m_Image.m_lpstNewCurrWilImageInfo;
            const int x=anchor.x+info.shPX,y=anchor.y+info.shPY;
            m_createBounds[gender]={x,y,x+info.shWidth,y+info.shHeight};
        }
        const bool voice=animation.Advance(nLoopTime,m_SelectJob,gender);
        const bool selected=gender==m_SelectSex;
        if(selected && voice) g_xSound.PlayRoleVoice(true,m_SelectJob,gender);
        const int frame=animation.ImageFrame(m_SelectJob,gender);
        DrawSelectionFrame(m_Image,frame+20,anchor.x,anchor.y,SelectionBlend::Shadow);
        DrawSelectionFrame(m_Image,selected?frame:animation.FirstFrame(m_SelectJob,gender),anchor.x,anchor.y,
            selected?SelectionBlend::Normal:SelectionBlend::Inactive);
        if (selected)
            DrawSelectionFrame(m_Image, frame + 40, anchor.x, anchor.y, SelectionBlend::Additive);
    }
    DrawSelectionFrame(m_Image, 82, 201, 434, SelectionBlend::Shadow, false);
    if (m_Image.NewSetIndex(81))
        g_xMainWnd.DrawWithImageForComp(247,384,m_Image.m_lpstNewCurrWilImageInfo->shWidth,
            m_Image.m_lpstNewCurrWilImageInfo->shHeight,(WORD*)m_Image.m_pbCurrImage);
    RECT editor = {287,404,364,419};
    DrawLegacyPanel(editor, 0, 255, RGB(120,90,60));
    for (int i = 0; i < 5; ++i) {
        const CreateControl& control = createControls[i];
        RECT bounds = {control.x,control.y,control.x+control.width,control.y+control.height};
        const bool hover = !m_xMsgBox.IsActive() && PtInRect(&bounds,m_ptMousePos);
        const int state = m_createPressed == i ? 2 : hover ? 1 : 0;
        const int frame = control.frame + (i == 4 && state == 2 ? 1 : state);
        if (m_Image.NewSetIndex(frame))
            g_xMainWnd.DrawWithImageForComp(control.x, control.y,
                m_Image.m_lpstNewCurrWilImageInfo->shWidth,
                m_Image.m_lpstNewCurrWilImageInfo->shHeight, (WORD*)m_Image.m_pbCurrImage);
    }
    // Control order is confirm/cancel/jobs; reference captions are jobs/confirm/cancel.
    static const POINT anchors[5] = {{469,458},{510,458},{289,438},{332,438},{376,438}};
    static const int captions[5] = {204,205,201,202,203};
    if (!m_xMsgBox.IsActive()) for (int i = 0; i < 5; ++i) {
        const auto& c = createControls[i];
        RECT hit = {c.x,c.y,c.x+c.width,c.y+c.height};
        if (!PtInRect(&hit,m_ptMousePos)) continue;
        const auto& caption = m_messages.Get(captions[i]);
        const int x = anchors[i].x + 10, y = anchors[i].y + 2;
        RECT box = {x,y,x + int(caption.size()) * 6,y + 15};
        DrawLegacyPanel(box,RGB(125,125,75),50,RGB(180,180,120));
        static const POINT offsets[4] = {{1,2},{3,2},{2,1},{2,3}};
        for (const auto& offset : offsets)
            g_xMainWnd.PutsHan(NULL,x+offset.x,y+offset.y,1644825,0,(char*)caption.c_str());
        g_xMainWnd.PutsHan(NULL,x+2,y+2,11862015,0,(char*)caption.c_str());
        break;
    }
}

void CCharacterProcess::BeginRosterWait() {
    if(m_nMessage==SelectionDialog::DeleteConfirmation && m_xMsgBox.IsActive()) FinishNotice(CLICK_NO);
    m_selectionElapsed=0;m_pendingRoster.reset();m_pendingNotice=0;m_returnWaiting=true;m_nMessage=0;
    m_xMsgBox.SetOrigin(155,150,false);
    m_xMsgBox.ShowMessageBox(m_messages.Get(ExitFlow::ConnectingCaption).c_str(),CLoginNotice::Waiting,CLoginNotice::WaitingFrame);
    ShowWindow(g_xChatEditBox.GetSafehWnd(),SW_HIDE);SetFocus(g_xMainWnd.GetSafehWnd());
}
void CCharacterProcess::RequestRoster() {BeginRosterWait();g_xClientSocket.OnQueryChar();}
void CCharacterProcess::AdvanceRoster(DWORD elapsed) {
    m_selectionElapsed+=min(elapsed,DWORD(SelectionRoster::Delay)-m_selectionElapsed);
    if(m_pendingRoster && m_selectionElapsed>=SelectionRoster::Delay) {
        const auto roster=*m_pendingRoster;const int notice=m_pendingNotice;
        m_pendingRoster.reset();m_pendingNotice=0;ApplyRoster(roster);
        if(notice) ShowNotice(notice);
    }
}
void CCharacterProcess::ApplyRoster(const SelectionRoster& roster) {
    if(m_returnWaiting) {m_returnWaiting=false;m_xMsgBox.HideMessageBox();m_xMsgBox.SetOrigin(140,110);}
    if(m_nMessage==SelectionDialog::DeleteConfirmation && m_xMsgBox.IsActive()) FinishNotice(CLICK_NO);
    m_CList.ClearAllNodes();m_CRectList.ClearAllNodes();
    m_ChrCount=roster.count;m_SelectChr=m_ChrCount?SELECT_FIRST:SELECT_NONE;
    for(int i=0;i<m_ChrCount;++i) {
        m_CList.AddNode(new SelectionRoleInfo(roster.roles[i]));m_CRectList.AddNode(new CHRRECT{});
    }
    m_xSelectChr.Create(&m_Image,&m_CList,&m_CRectList,m_ChrCount);m_xSelectChr.OpenWnd();
    if(!m_ChrCount) ShowNotice(SelectionDialog::Empty);
    else {m_xMsgBox.HideMessageBox();m_nMessage=0;}
}
void CCharacterProcess::ShowNotice(int message) {
    if(m_pendingRoster && m_selectionElapsed<SelectionRoster::Delay) {m_pendingNotice=message;return;}
    if(m_nMessage==SelectionDialog::DeleteConfirmation && m_xMsgBox.IsActive()) FinishNotice(CLICK_NO);
    m_returnWaiting=false;m_nMessage=message;m_xMsgBox.SetOrigin(140,110);
    m_xMsgBox.ShowMessageBox(m_messages.Get(message).c_str(),MSG_BTN_OK,0);
    ShowWindow(g_xChatEditBox.GetSafehWnd(),SW_HIDE);SetFocus(g_xMainWnd.GetSafehWnd());
}
void CCharacterProcess::SubmitCreation() {
    if(m_bCreatePending || m_xMsgBox.IsActive()) return;
    char name[256]={};GetWindowTextA(g_xChatEditBox.GetSafehWnd(),name,sizeof(name));
    if(!name[0]) return;
    if(const int error=SelectionDialog::NameError(name)) {ShowNotice(error);return;}
    if(!g_szUserID[0]) return;
    lstrcpynA(m_szChrName,name,sizeof(m_szChrName));m_bCreatePending=TRUE;
    g_xClientSocket.OnNewChar(m_szChrName,0,m_SelectJob,m_SelectSex);
}
void CCharacterProcess::FinishNotice(HRESULT result) {
    if(result!=CLICK_OK && result!=CLICK_YES && result!=CLICK_NO) return;
    const bool deleting=m_nMessage==SelectionDialog::DeleteConfirmation;
    char confirmation[SelectionDialog::ConfirmationBytes+1]={};
    if(deleting) GetWindowTextA(g_xChatEditBox.GetSafehWnd(),confirmation,sizeof(confirmation));
    m_xMsgBox.HideMessageBox();m_nMessage=0;
    if(deleting) {
        SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"");ShowWindow(g_xChatEditBox.GetSafehWnd(),SW_HIDE);
        g_xChatEditBox.SetLimitText(SelectionDialog::NameBytes);
        if(result==CLICK_YES && m_xSelectChr.m_nSelectChr>=0 && m_xSelectChr.m_nSelectChr<m_ChrCount && g_szUserID[0]) {
            m_CList.MoveCurrentToTop();m_CList.MoveNode(m_xSelectChr.m_nSelectChr);
            g_xClientSocket.OnDelChar(m_CList.GetCurrentData()->ChrName,confirmation);
            RequestRoster();
        }
    } else if(m_Progress==PRG_CREATE_CHAR) {
        SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"");ShowWindow(g_xChatEditBox.GetSafehWnd(),SW_SHOW);
        SetFocus(g_xChatEditBox.GetSafehWnd());g_xChatEditBox.SetSelectAll();
    }
}
VOID CCharacterProcess::SetErrorMessage(INT message,CHAR* text) {
    lstrcpynA(text,m_messages.Get(message).c_str(),512);
}
BOOL CCharacterProcess::MessageBoxProcess(LPARAM parameter) {
    FinishNotice(m_xMsgBox.OnButtonUp(POINT{LOWORD(parameter),HIWORD(parameter)},true));return TRUE;
}

void CCharacterProcess::OpenCreation()
{
    m_Progress=PRG_CREATE_CHAR;
    m_SelectJob=0;m_createPressed=-1;m_bCreatePending=FALSE;
    for(auto& role:m_creationAnimation) role.Start(RoleAnimation::Create);
    ZeroMemory(m_createBounds,sizeof(m_createBounds));
    m_xMsgBox.HideMessageBox();m_nMessage=0;
    g_xChatEditBox.SetLimitText(SelectionDialog::NameBytes);
    SendMessage(g_xChatEditBox.GetSafehWnd(),EM_SETPASSWORDCHAR,0,0);
    g_xChatEditBox.MoveScaled(g_xMainWnd.m_rcWindow.left+288,g_xMainWnd.m_rcWindow.top+405,75,13,TRUE);
    SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"");
    ShowWindow(g_xChatEditBox.GetSafehWnd(),SW_SHOW);
    SetFocus(g_xChatEditBox.GetSafehWnd());
}

void CCharacterProcess::ExitSelection()
{
    m_lastUrl=m_messages.Get(LoginLayout::ExitUrl);
    if(m_offline) return;
    g_xSound.UpdateLobbyAudio(CMirSound::LobbyPage::None,0);
    if(!m_lastUrl.empty()) ShellExecuteA(g_xMainWnd.GetSafehWnd(),"open",m_lastUrl.c_str(),nullptr,nullptr,SW_SHOWNORMAL);
    PostMessage(g_xMainWnd.GetSafehWnd(),WM_CLOSE,0,0);
}
