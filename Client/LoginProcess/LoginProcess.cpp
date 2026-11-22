// LoginProcess.cpp: implementation of the CLoginProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define SETBOX(ret,x,y,w,h)			{ ret.left = x;ret.top = y;ret.right = w;ret.bottom.h;}

#define MESSAGE_NUM_LOGIN	0
#define SRVSELBTN_LEFT		110
#define SRVSELBTN_TOP		120

extern CClientSocket g_xClientSocket;

CLoginProcess::CLoginProcess()
{

	m_hBrush = CreateSolidBrush(RGB(0, 0, 0));
	Init();
}

VOID CLoginProcess::Init()
{
	m_fIsConnected				= CONNECT_FAIL;
	m_Progress					= PRG_PATCH;
	m_UserState					= LGM_INPUT_ID;
	m_fIsLButtonClicked			= FALSE;
	m_nAnimationCounter			= 0;
	ReadIniFileData();
}

VOID CLoginProcess::ReadIniFileData(VOID)
{
	CHAR	szFullPathFileName[MAX_PATH];
	CHAR	szTemp[MAX_PATH];
	INT		nServerCount;
	INT		nLoop;

	GetCurrentDirectory(1024,szFullPathFileName);
	strcat(szFullPathFileName+strlen(szFullPathFileName),"\\\0");
 	strcat(szFullPathFileName,MIR2EI_INI_FILE_NAME);

	// Get Server IP & PORT
	GetPrivateProfileString( MIR2EI_INI_SECTION, LOGIN_SETVER_NAME, LOGIN_GATE_SERVER_IP, m_szServerIP, 16, szFullPathFileName );
	m_nServerPort	=	GetPrivateProfileInt( MIR2EI_INI_SECTION, LOGIN_SERVER_PORT, LOGIN_SERVER_PORT_DATA, szFullPathFileName );
	// Get ServerCounter
	nServerCount	=	GetPrivateProfileInt( MIR2EI_SRV_SECTION, SERVER_COUNT, 0, szFullPathFileName);
	// Read ServerList From Ini files
	m_SList.ClearAllNodes();
	for( nLoop = 0; nLoop < nServerCount; nLoop++ )
	{
		PServerList	SList = new ServerList;		
		sprintf( szTemp, "%s%d%s", "server", nLoop+1, "caption");
		GetPrivateProfileString( MIR2EI_SRV_SECTION,(LPCTSTR) szTemp, NULL, SList->Caption , MAX_PATH, szFullPathFileName );		
		sprintf( szTemp, "%s%d%s", "server", nLoop+1, "name");
		GetPrivateProfileString( MIR2EI_SRV_SECTION,(LPCTSTR) szTemp, NULL, SList->Name , MAX_PATH, szFullPathFileName );
		m_SList.AddNode(SList);
	}
}

CLoginProcess::~CLoginProcess()
{
	DeleteObject(m_hBrush);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//	Load
//////////////////////////////////////////////////////////////////////////////////////////////////////
void CLoginProcess::Load(void)
{	
//	int i;
	int nSrvCount;

	m_Image.NewLoad(IMAGE_INTERFACE_1, TRUE, TRUE);
	m_Image2.NewLoad(IMAGE_INTERFACE_2, TRUE, TRUE);

	ZeroMemory(g_xChatEditBox.m_szInputMsg,sizeof(g_xChatEditBox.m_szInputMsg));
	pPatch = new CPatch;
	pPatch->InitPatch();

//	m_xMsgBox.Load(&m_Image);							// 容器		2D
	m_xMsgBox.CreateWnd(&m_Image2,TRUE,219,110,362,180);	// 容器		3D
//	g_xChatEditBox.SetLimitText(g_LoginInputState[0].nSize);
//	g_xChatEditBox.m_dwFontColor = RGB(125,125,125);
	SendMessage(g_xChatEditBox.GetSafehWnd(),EM_SETPASSWORDCHAR,NULL,0);

	// Create Server Select Btn
	nSrvCount = m_SList.GetCounter();
	m_xAvi.Create(&m_Image);
	m_xLogin.Create(&m_Image);
	m_xSelectSrv.Create(&m_Image,&m_SList);

}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//	Message
//////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CLoginProcess::DefMainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch ( uMsg )
	{
		case WM_CTLCOLOREDIT:
		{
			if ((HWND)lParam == g_xChatEditBox.GetSafehWnd())
			{
				SetBkColor((HDC)wParam, RGB(0, 0, 0));
				SetTextColor((HDC)wParam, RGB(255, 255, 255));

				UnrealizeObject(m_hBrush);
				
				POINT pt;

				SetBrushOrgEx((HDC)wParam, 0, 0, &pt);
				return (LRESULT)m_hBrush;
			}
			break;
		}
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
			UpdateWindow(g_xMainWnd.GetSafehWnd());
			OnKeyDown(wParam, lParam);
			break;
		case WM_SYSKEYDOWN:
		{
			if ( wParam == VK_RETURN )
			{
				g_xMainWnd.OnSysKeyDown(wParam, lParam);
			}
			return 0L;
		}
		case WM_MOVE:
		{
			OnWindowMove(wParam,lParam);
			return 0L;
		}
		case WM_MOUSEMOVE:
			OnMouseMove(wParam,lParam);
			break;
		case _WM_USER_MSG_INPUTTED:
		{
			switch(m_Progress)
			{
				case PRG_LOGIN:
				case PRG_NEW_ACCOUNT:
					OnKeyDown((WPARAM)VK_RETURN,NULL);
					break;
				case PRG_CHANGE_PASS:
					break;
			}
		}
		//case WM_PAINT:
//		{
			//if (m_Progress == PRG_PATCH)
	//		{
//				RenderPatch(0);
				//pPatch->DrawProgressImage();
			//	return 0l;
		//	}
	//		if (m_Progress == PRG_QUIT)
//			{
				//SendMessage(g_xMainWnd.GetSafehWnd(), WM_DESTROY, NULL, NULL);
 //       pPatch->DrawProgressImage();
	//			return 0l;
	//		}
		//	break;
	//	}
	}
	return CWHDefProcess::DefMainWndProc(hWnd, uMsg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//	Event
//////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CLoginProcess::OnKeyDown(WPARAM wParam,LPARAM lParam)
{
	if(!m_xMsgBox.IsActive())
	{
		switch(m_Progress)
		{
			case PRG_LOGIN:// LOGIN 的情况下
			{
				m_xLogin.OnKeyDown(wParam,lParam);
				break;
			}
			case PRG_NEW_ACCOUNT:
			{
				if (wParam == VK_RETURN || wParam == VK_TAB)
				{
					SetFocusBefore();
					m_UserState = (m_UserState+1) > 16 ? 0 : m_UserState + 1;
					SetFocusAfter();
				}
				break;
			}
		}
	}
	return 0L;
}

LRESULT CLoginProcess::OnLButtonDown( WPARAM wParam, LPARAM lParam )
{	
//	RECT	tRect;
//	int i;
	if( m_xMsgBox.IsActive() )
	{
		POINT ptMouse;
		ptMouse.x = LOWORD( lParam );
		ptMouse.y = HIWORD( lParam );		
		m_xMsgBox.OnButtonDown( ptMouse );
	}
	else
	{
		switch( m_Progress )
		{
			case PRG_LOGIN:
			{
				// Input Area Click
				m_xLogin.OnButtonDown( wParam, lParam );
				break;
			}
			case PRG_SERVER_SELE:
			{
				m_xSelectSrv.OnButtonDown( wParam, lParam );
				break;
			}
			case PRG_NEW_ACCOUNT:
			{
				break;
			}
		}
	}
	return 0L;
}

LRESULT CLoginProcess::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{	
//	int i;
//	INT MsgNum;
	POINT	ptMouse;

	ptMouse.x = LOWORD(lParam);
	ptMouse.y = HIWORD(lParam);

	if(m_fIsConnected && !m_xMsgBox.IsActive())
	{
		m_fIsLButtonClicked = FALSE;
		switch(m_Progress)
		{
			case PRG_LOGIN:
			{
				m_xLogin.OnButtonUp(wParam, lParam);
				break;
			}// case
			case PRG_SERVER_SELE:
			{
				m_xSelectSrv.OnButtonUp(wParam, lParam);
				break;
			}
			case PRG_NEW_ACCOUNT:
			{
/*				for( i = BTN_NEW_OK_ID ; i <= BTN_NEW_QUIT_ID ; i++ )
				{
					if ( m_pBtn[i].CheckMouseOn( LOWORD( lParam ), HIWORD( lParam ) ) )
					{
						switch( m_pBtn[i].m_nButtonID )
						{
						case BTN_NEW_OK:
							{
								int Count;
								BOOL IsNotFilled = FALSE;

								Count = (sizeof(g_LoginInputState) / sizeof(INPUTSTATE));

								SetFocusBefore();

								for(int i = LGM_INPUT_USER_ID ; i <= LGM_INPUT_ANSWER_2 ; i++)
								{
									if(strlen(g_LoginInputState[i-8].szData) == 0 )
									{
										IsNotFilled= TRUE;
										break;
									}
								}
								
								if(IsNotFilled)
								{
									MsgNum = ERROR_STR_NEW_ACCOUNT_5;
								}
								else
								{
									if(strcmp( g_LoginInputState[3].szData, g_LoginInputState[4].szData) != 0 )
									{	
										MsgNum=ERROR_STR_NEW_ACCOUNT_2;							
									}
									else
									{
										if(!CheckSSNO(g_LoginInputState[6].szData))
										{
											MsgNum=ERROR_STR_NEW_ACCOUNT_3;
										}
										else
										{
											
											if(!CheckEMail(g_LoginInputState[14].szData))
											{
												MsgNum=ERROR_STR_NEW_ACCOUNT_4;
											}
											else
											{
												//OnNewAccount();
												break;
											}
										}
									}
								}
								SetErrorMessage(MsgNum,m_szErrorMsg);
								m_xMsgBox.ShowMessageBox(m_szErrorMsg, 1, _WNDIMGINDX_MSGBOX1);
								break;
							}
						case BTN_NEW_CANCLE:
							{
								m_Progress = PRG_LOGIN;
								m_UserState = LGM_INPUT_ID;
								ZeroMemory(g_xChatEditBox.m_szInputMsg,sizeof(g_xChatEditBox.m_szInputMsg));
								SetWindowText(g_xChatEditBox.GetSafehWnd(),NULL);
								SetFocusAfter();
								break;
							}
						case BTN_NEW_QUIT:
							{
								m_Progress = PRG_LOGIN;
								m_UserState = LGM_INPUT_ID;
								ZeroMemory(g_xChatEditBox.m_szInputMsg,sizeof(g_xChatEditBox.m_szInputMsg));
								SetWindowText(g_xChatEditBox.GetSafehWnd(),NULL);
								SetFocusAfter();
								break;
							}
						}
					}
					m_pBtn[i].m_nState = BUTTON_STATE_UP;
				}*/
			break;
			}
		}
	}
	else if(m_xMsgBox.IsActive())
			if(m_xMsgBox.OnButtonUp(ptMouse)==1)
				m_xMsgBox.HideMessageBox();

	return 0L;
}

LRESULT CLoginProcess::OnMouseMove(WPARAM wParam, LPARAM lParam)
{	
//	int i;
	m_MousePos.x = LOWORD(lParam);
	m_MousePos.y = HIWORD(lParam);
//	g_xMainWnd.OnSetCursor(IDC_ARROW);
	if(m_xMsgBox.IsActive())
	{
		m_xMsgBox.MoveWnd(m_MousePos);	// Moon
		return 0l;
	}

	m_xLogin.OnMouseMove( wParam, lParam);
	m_xSelectSrv.OnMouseMove( wParam, lParam);
/*
	if(m_fIsConnected && !m_fIsLButtonClicked)
	{
		for( i = BTN_NEW_ID ; i <= BTN_NEW_QUIT_ID ; i ++)
		{
			if (m_pBtn[i].CheckMouseOn(LOWORD(lParam), HIWORD(lParam)))
				m_pBtn[i].m_nState = BUTTON_STATE_ON;
			else
				m_pBtn[i].m_nState = BUTTON_STATE_UP;
		}
	}*/
	return 0L;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
//	Socket
//////////////////////////////////////////////////////////////////////////////////////////////////////
VOID CLoginProcess::OnConnectToServer()
{
	SetFocus(g_xChatEditBox.GetSafehWnd());

	m_fIsConnected	= CONNECT_SUCCESS;			// Connection Success
}

char* CLoginProcess::OnMessageReceive(CHAR* pszMessage)
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

void CLoginProcess::OnSocketMessageRecieve(char *pszMsg)
{
	INT	nMsgNum;
	_TDEFAULTMESSAGE	tdm;

	fnDecodeMessage(&tdm, pszMsg);

	switch(tdm.wIdent)
	{
		// ID Not Found
		case SM_ID_NOTFOUND:
		{
			m_Progress	= PRG_LOGIN;
			SetErrorMessage(23,m_szErrorMsg);
			m_xMsgBox.ShowMessageBox(m_szErrorMsg, 1, _WNDIMGINDX_MSGBOX1);
			break;
		}
		// Incorrect Password
		case SM_PASSWD_FAIL:
		{
			m_Progress	= PRG_LOGIN;
			
			if(tdm.nRecog==(-2))
				nMsgNum = ERROR_STR_LOGIN_3;
			else
				nMsgNum = ERROR_STR_LOGIN_2;
			SetErrorMessage(nMsgNum,m_szErrorMsg);
			m_xMsgBox.ShowMessageBox(m_szErrorMsg, 1, _WNDIMGINDX_MSGBOX1);
			break;
		}

		case SM_CERTIFICATION_FAIL:
		{
			SetErrorMessage(FATAL_ERROR_STR,m_szErrorMsg);
			m_xMsgBox.ShowMessageBox(m_szErrorMsg, 1, _WNDIMGINDX_MSGBOX1);			// 勇气
			break;
		}

		case SM_PASSOK_SELECTSERVER:
		{
			m_xSelectSrv.OpenWnd();
			m_Progress = PRG_SERVER_SELE;
			Clear(RGB(0,0,0));
//			g_xClientSocket.OnSelectServer(NULL);

			break;
		}

		case SM_NEWID_SUCCESS:	
		{
			m_Progress = PRG_LOGIN;
			m_UserState = LGM_INPUT_ID;
			ZeroMemory(g_xChatEditBox.m_szInputMsg,sizeof(g_xChatEditBox.m_szInputMsg));
			SetFocusAfter();

			SetErrorMessage(NEW_ACCOUNT_WELL_DONE,m_szErrorMsg);
			m_xMsgBox.ShowMessageBox(m_szErrorMsg, 1, _WNDIMGINDX_MSGBOX1);
			break;
		}
		// New ID Create Fail
		case SM_NEWID_FAIL:
		{
			m_Progress	= PRG_NEW_ACCOUNT;

			switch(tdm.nRecog)
			{
			case 1:
				nMsgNum=ERROR_STR_NEW_ACCOUNT_1;
				break;
			case 2:
				nMsgNum=ERROR_STR_NEW_ACCOUNT_6;
				break;
			case 3:
				nMsgNum=ERROR_STR_NEW_ACCOUNT_7;
				break;
			}
			SetErrorMessage(nMsgNum,m_szErrorMsg);
			m_xMsgBox.ShowMessageBox(m_szErrorMsg, 1, _WNDIMGINDX_MSGBOX1);
			break;
		}
		// Change Password Success
		case SM_CHGPASSWD_SUCCESS:
		{
			m_Progress	= PRG_LOGIN;
			break;
		}
		// Change Passsword Fail
		case SM_CHGPASSWD_FAIL:
		{
			m_Progress	= PRG_CHANGE_PASS;	
			break;	
		}

		case SM_SELECTSERVER_OK:
		{
			char szDecodeMsg[256];
			char *pszIP = szDecodeMsg, *pszPort, *pszIdenty;

			ZeroMemory(szDecodeMsg, sizeof(szDecodeMsg));

			fnDecode6BitBuf((pszMsg + DEFBLOCKSIZE), szDecodeMsg, sizeof(szDecodeMsg));

			if (pszPort = strchr(szDecodeMsg, '/')){
				*pszPort = '\0';
				pszPort++;

				if (pszIdenty = strchr(pszPort, '/')){
					*pszIdenty = '\0';
					pszIdenty++;

					strcpy(g_szServerIP, pszIP);
					g_nServerPort = atoi(pszPort);

					m_nAnimationCounter = 0;
					
					g_nCertifyCode = tdm.nRecog;
					g_xClientSocket.DisconnectToServer();

					Clear(RGB(0,0,0));
					m_Progress	= PRG_TO_SELECT_CHR;
				}
			}
			break;
		}
		// Connection Lost(close Connection)
		case	SM_OUTOFCONNECTION:
		{
			m_Progress	= PRG_QUIT;									// Quit for Some Reason		程序退出
			SendMessage(g_xMainWnd.GetSafehWnd(), WM_DESTROY, NULL, NULL);
			break;
		}
	}
/*	
	switch(tdm.wIdent)
	{
		case	SM_NEEDUPDATE_ACCOUNT:	// Need to account update
			m_Progress	= PRG_NEED_ACCOUNT_CHANGE;	// Need Account change
			break;
		case	SM_UPDATEID_SUCCESS:	// account update success
			m_Progress	= PRG_LOGIN;		// Login
			break;
		case	SM_UPDATEID_FAIL:		// account update Fail
			m_Progress	= PRG_NEED_ACCOUNT_CHANGE;	// Login
			break;
	}
*/
}

VOID CLoginProcess::SetFocusBefore(VOID)
{
	INT nTemp;

	nTemp = (( m_Progress == PRG_LOGIN) ? m_UserState - 1 : m_UserState + 2 );

	ZeroMemory(g_xChatEditBox.m_szInputMsg,sizeof(g_xChatEditBox.m_szInputMsg));
	ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
	g_xChatEditBox.SetSelectAll();
}

VOID CLoginProcess::SetFocusAfter(VOID)
{
	CHAR	cChr;
	INT		nTemp;

	ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_SHOW);

	nTemp = (( m_Progress == PRG_LOGIN) ? m_UserState - 1 : m_UserState + 2 );


	if( ( m_UserState != LGM_INPUT_PASSWORD ) &&
		( m_UserState != LGM_INPUT_USER_PASSWORD ) &&
		( m_UserState != LGM_INPUT_REPASS ) )
		cChr = NULL;
	else
		cChr = '*';

	SendMessage(g_xChatEditBox.GetSafehWnd(),EM_SETPASSWORDCHAR,(WPARAM)cChr,0);

	SetFocus(g_xChatEditBox.GetSafehWnd());
	g_xChatEditBox.SetSelectAll();
}

VOID CLoginProcess::SetNextProc()
{
	DeleteProc();
	g_xClientSocket.m_pxDefProc = g_xMainWnd.m_pxDefProcess = &g_xChrSelProc;
	g_xChrSelProc.Load();
	g_bProcState = _CHAR_SEL_PROC;
}

VOID CLoginProcess::DeleteProc()
{
	m_xAvi.StopAllAvis();
	m_Image2.Destroy();
	m_Image.Destroy();
}

LRESULT CLoginProcess::OnWindowMove(WPARAM wParam,LPARAM lParam)
{
	ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
	g_xMainWnd.OnMove(wParam, lParam);
	return 0L;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//	Rander
//////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CLoginProcess::CheckSSNO(char* szSSNo)
{	// 未完成
	int		nLen = 0, nYear = 0;
	long	nSum = 0, nCheckSum = 0;
	char	szDate[7], szYear[5], szSex[5] = "1234";
	int		nTemp;
	int		Counter=1;
	int		i;
	CHAR*	pszLowCode = NULL;
	CHAR	sztSSNo[15];

	strcpy(sztSSNo,szSSNo);

	pszLowCode = strchr(sztSSNo,'-');
	if(pszLowCode==NULL)
		return FALSE;								// 居民登记号上没有 - 则按错误处理
	pszLowCode ++;

	nLen = strlen( sztSSNo );
	if(nLen != 14)
		return FALSE;
	
	// 性别代码确认
	if((*pszLowCode - '0') < 0 || (*pszLowCode - '0') > 4 ) 
		return FALSE;								// 性别代码不匹配

	strncpy(szDate,sztSSNo,6);
	strncpy(szYear,sztSSNo,2);
	nYear= atoi(szYear);
	nCheckSum = *( sztSSNo + nLen - 1 ) - '0';

	memcpy(pszLowCode-1,pszLowCode,strlen(pszLowCode));

	for( i = 0 ; i < nLen - 1 ; i++ )
	{
		if(szSSNo[i]=='-')
			return FALSE;
		nTemp = (sztSSNo[i]-'0')*(Counter>=9? Counter=2:++Counter);
		nSum += nTemp;
	}

	nSum = nSum % 11;
	nSum = 11 - nSum;
	if( nCheckSum != nSum )
		return FALSE;
	return TRUE;
}

BOOL CLoginProcess::DateCehck( char* szDate )
{
	char szYear[5],szMonth[3],szDay[3];
	int nYear,nMonth,nDay;
	int tDay;

	if ( strlen( szDate ) != 8)
		return FALSE;
	
	strncpy( szYear, szDate, 4 );
	strncpy( szMonth, szDate + 4, 2 );
	strncpy( szDay, szDate + 6, 2 );

	nYear	= atoi( szYear );
	nMonth	= atoi( szMonth );
	nDay	= atoi( szDay );

	// Check Month's days
	switch( nMonth )
	{
		case 1:	case 3:	case 5:	case 7: case 8: case 10:case 12:
			tDay = 31;
			break;
		case 4: case 6: case 9: case 11:
			tDay = 30;
			break;
		case 2:
			tDay = IsLeapYear( nYear ) ? 29 : 28;
			break;
		default :
			return FALSE;
	}

	if( nDay < 1 || nDay > tDay ) return FALSE;

	return TRUE;
}

BOOL CLoginProcess::IsLeapYear( int nYear )
{
	if ( ( nYear % 4 ) == 0){

		if ( ( nYear % 100 ) == 0){

			if ( ( nYear % 400 ) == 0)
				return TRUE;
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CLoginProcess::CheckEMail( char* szEMail )	// E Mail Check
{
	CHAR* pszTemp = NULL;

	// '@' 不能出现在最前面。'@' 必须始终包含。
	pszTemp = strchr( szEMail , '@');
	if( ( ( pszTemp - szEMail ) == 0 ) ||
		( pszTemp == NULL ) )
		return FALSE; 

	// '@' 不能有一个以上.
	pszTemp = strchr( pszTemp + 1 ,'@');
	if( pszTemp != NULL ) return FALSE; 

	// '.' 不能出现在最前面. '.' 必须至少包含一个.
	pszTemp = strchr(szEMail,'.');
	if( ( ( pszTemp - szEMail ) == 0 ) ||
		( pszTemp == NULL ) )
		return FALSE; 
	
	return TRUE;
}

BOOL CLoginProcess::StringDivideLen( INT nDivideLen, INT& nDividedLine, CHAR* szSrc,CDLList<CHAR*>* m_pxpStr )
{
	CHAR	szTmpCheck[MAX_PATH];
	CHAR	szResult[MAX_PATH];
	INT		nCnt; 
	
	nDividedLine = 1;

	INT nStartLen	= 0;
	INT nEndLen		= 0;
	INT nResultLen	= 0;
	INT nWordCheck	= 0;

	if ( szSrc[0] != NULL ){
		ZeroMemory(szResult,MAX_PATH);

		for ( nCnt = 0; nCnt < (INT) strlen( szSrc ); nCnt++){
			ZeroMemory(szTmpCheck, MAX_PATH);
			nEndLen = nCnt+1;

			// 读取截至当前部分的字符串。
			memcpy(&szTmpCheck, &szSrc[nStartLen], nEndLen-nStartLen);
			INT nsLen;
			nsLen = strlen(szTmpCheck);

			// 当前已读取的部分超出允许宽度时。
			if (nsLen > nDivideLen ){
				// 当前如果是 2 字节字符. 
				if ( szSrc[nEndLen-1] < 0 ){
					// 如果当前前面的字符不是 2 字节字符
					if ( !(nWordCheck%2) ){
						CHAR* pszNewLine;
						nStartLen += strlen(szTmpCheck)-1;
						nCnt--;
						pszNewLine = new CHAR[nResultLen+1];
						memcpy(pszNewLine,szResult,nResultLen);
						pszNewLine[nResultLen]=NULL;
						m_pxpStr->AddNode(pszNewLine);
						nResultLen = 0;
						nDividedLine++;
					}
					else{
						CHAR* pszNewLine;
						nStartLen += strlen(szTmpCheck)-2;
						nCnt -= 2;
						pszNewLine = new CHAR[nResultLen];
						memcpy(pszNewLine,szResult,nResultLen-1);
						pszNewLine[nResultLen-1]=NULL;
						m_pxpStr->AddNode(pszNewLine);
						nResultLen = 0;
						nDividedLine++;
						nWordCheck--;
					}
				}
				// 1字节字符. 
				// 只需考虑到比当前位置往前一个字节的部分即可。
				else{
					CHAR* pszNewLine;
					nStartLen += strlen(szTmpCheck)-1;
					nCnt--;
					pszNewLine = new CHAR[nResultLen+1];
					memcpy(pszNewLine,szResult,nResultLen);
					pszNewLine[nResultLen]=NULL;
					m_pxpStr->AddNode(pszNewLine);
					nResultLen=0;
					nDividedLine++;
				}
			}
			// 强制换行     如果用 \n 则 Error ????
			else if(szSrc[nEndLen-1]=='\n'){
				CHAR* pszNewLine;
				nStartLen += strlen(szTmpCheck)-1;
				pszNewLine = new CHAR[nResultLen+1];
				memcpy(pszNewLine,szResult,nResultLen);
				pszNewLine[nResultLen]=NULL;
				m_pxpStr->AddNode(pszNewLine);
				nResultLen=0;
				nDividedLine++;
			}
			else{
				if ( szSrc[nEndLen-1] < 0 )
					nWordCheck++;

				szResult[nResultLen] = szSrc[nEndLen-1];
				nResultLen++;
			}
		}
		if( nResultLen != 0 ){
			CHAR* pszNewLine;
			pszNewLine = new CHAR[nResultLen+1];
			memcpy(pszNewLine,szResult,nResultLen);
			pszNewLine[nResultLen]=NULL;
			m_pxpStr->AddNode(pszNewLine);
			nDividedLine++;
			nResultLen=0;
		}
		return TRUE;
	}
	return FALSE;
}

VOID CLoginProcess::SetErrorMessage(INT nMsg, CHAR* szMgs)
{
	HINSTANCE hLib;

	hLib = LoadLibrary(MESSAGE_DLL_1); // Load Dll
	
	if ( nMsg!=99 && nMsg!=0 )
		LoadString(hLib, nMsg+MESSAGE_NUM_LOGIN, m_szErrorMsg, 512);	
	else if( nMsg!=0 )	
		LoadString(hLib, 9999, m_szErrorMsg, 512);
	else
		szMgs[0]=NULL;

	FreeLibrary(hLib);
}

