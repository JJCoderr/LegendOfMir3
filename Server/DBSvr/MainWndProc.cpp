#include "stdafx.h"
#include "../def/dbmgr.h"

LPARAM OnServerSockMsg(WPARAM wParam, LPARAM lParam);
LPARAM OnClientSockMsg(WPARAM wParam, LPARAM lParam);

BOOL		jRegSetKey(LPCTSTR pSubKeyName, LPCTSTR pValueName, DWORD dwFlags, LPBYTE pValue, DWORD nValueSize);
BOOL		jRegGetKey(LPCTSTR pSubKeyName, LPCTSTR pValueName, LPBYTE pValue);

UINT WINAPI ProcessDBMsg(LPVOID lpParameter);
UINT WINAPI ProcessGateMsg(LPVOID lpParameter);

BOOL		InitGateCommSocket(SOCKET &s, SOCKADDR_IN* addr, UINT nMsgID, int nPort, long lEvent);
LPARAM		OnGateCommSockMsg(WPARAM wParam, LPARAM lParam);

BOOL CALLBACK ConfigDlgFunc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

extern HINSTANCE		g_hInst;

extern HWND				g_hMainWnd;
extern HWND				g_hLogMsgWnd;
extern HWND				g_hToolBar;
extern HWND				g_hStatusBar;

BOOL					g_fTerminated = FALSE;

// 关闭确认框是否正在显示：用来挡掉重复的 WM_CLOSE（连点标题栏 X 不会再叠第二个框）
BOOL					g_fCloseConfirm = FALSE;

// 上一个确认框关掉的时刻：连点 X 时后一个 WM_CLOSE 可能排在队列里，等框关了才被处理，
// 只用 g_fCloseConfirm 挡不住，再加一个很短的时间窗。
DWORD					g_dwLastCloseTick = 0;

// 两个服务线程的句柄：退出前要先让它们自己跳出循环，再拆 socket / WSA
HANDLE					g_hDBMsgThread = NULL;
HANDLE					g_hGateMsgThread = NULL;

SOCKET					g_ssock = INVALID_SOCKET;
SOCKADDR_IN				g_saddr;

SOCKET					g_gssock = INVALID_SOCKET;
SOCKADDR_IN				g_gsaddr;


CWHList<GAMESERVERINFO*>	g_xGameServerList;

void SwitchMenuItem(BOOL fFlag)
{
	HMENU hMainMenu = GetMenu(g_hMainWnd);
	HMENU hMenu = GetSubMenu(hMainMenu, 0);

	if (fFlag)
	{
		EnableMenuItem(hMenu, IDM_STARTSERVICE, MF_GRAYED|MF_BYCOMMAND);
		EnableMenuItem(hMenu, IDM_STOPSERVICE, MF_ENABLED|MF_BYCOMMAND);

		SendMessage(g_hToolBar, TB_SETSTATE, (WPARAM)IDM_STARTSERVICE, (LPARAM)MAKELONG(TBSTATE_INDETERMINATE, 0));
		SendMessage(g_hToolBar, TB_SETSTATE, (WPARAM)IDM_STOPSERVICE, (LPARAM)MAKELONG(TBSTATE_ENABLED, 0));

		InsertLogMsg(IDS_STARTSERVICE);

		SendMessage(g_hStatusBar, SB_SETTEXT, MAKEWORD(0, 0), (LPARAM)_T("Ready"));
	}
	else
	{
		EnableMenuItem(hMenu, IDM_STARTSERVICE, MF_ENABLED|MF_BYCOMMAND);
		EnableMenuItem(hMenu, IDM_STOPSERVICE, MF_GRAYED|MF_BYCOMMAND);

		SendMessage(g_hToolBar, TB_SETSTATE, (WPARAM)IDM_STARTSERVICE, (LPARAM)MAKELONG(TBSTATE_ENABLED, 0));
		SendMessage(g_hToolBar, TB_SETSTATE, (WPARAM)IDM_STOPSERVICE, (LPARAM)MAKELONG(TBSTATE_INDETERMINATE, 0));

		InsertLogMsg(IDS_STOPSERVICE);
	}
}

void LoadCharacterRecords()
{
	InsertLogMsg(IDS_LOADACCOUNTRECORDS);

	CRecordset *pRec = GetDBManager()->CreateRecordset();

	if ( pRec == NULL )
	{
		InsertLogMsg( _T("无法读取 TBL_GAMEGATEINFO：数据库还没有连上。") );

		return;
	}

	if ( pRec->Execute( "SELECT * FROM TBL_GAMEGATEINFO" ) )
	{
		GAMESERVERINFO *pServerInfo;

		while ( pRec->Fetch() )
		{
			pServerInfo = new GAMESERVERINFO;
			if ( !pServerInfo )
				break;

			pServerInfo->index = atoi( pRec->Get( "FLD_SERVERIDX" ) );
			strcpy( pServerInfo->name, pRec->Get( "FLD_SERVERNAME" ) );
			strcpy( pServerInfo->ip,   pRec->Get( "FLD_SERVERIP" ) );
			pServerInfo->connCnt = 0;

			g_xGameServerList.AddNewNode( pServerInfo );
		}
	}
	GetDBManager()->DestroyRecordset( pRec );
}

// 停服务：先置退出标志让各工作线程自己跳出循环，再等它们结束。
// 不等就直接拆 socket、WSACleanup、让进程退出，是"关程序卡很久、最后被强制结束"的来源：
// 线程还停在 ODBC / socket 调用里，主线程却已经在销毁这些句柄了。
static void StopServiceThreads()
{
	g_fTerminated = TRUE;

	HANDLE	hThreads[2] = { g_hDBMsgThread, g_hGateMsgThread };

	for ( int i = 0; i < 2; i++ )
	{
		if ( hThreads[i] == NULL )
			continue;

		WaitForSingleObject( hThreads[i], 3000 );
		CloseHandle( hThreads[i] );
	}

	g_hDBMsgThread	 = NULL;
	g_hGateMsgThread = NULL;
}

void OnCommand(WPARAM wParam, LPARAM lParam)
{
	int nPort;

	switch (LOWORD(wParam))
	{
		case IDM_STARTSERVICE:
		{
			if ( g_hDBMsgThread || g_hGateMsgThread )		// 服务已经在跑了
				return;

			g_fTerminated = FALSE;

			if (!jRegGetKey(_DB_SERVER_REGISTRY, _TEXT("LocalPort"), (LPBYTE)&nPort))
				nPort = 6000;
	
			LoadCharacterRecords();

			InitServerSocket(g_ssock, &g_saddr, _IDM_GATECOMMSOCK_MSG, nPort, 1);
			if (!jRegGetKey(_DB_SERVER_REGISTRY, _TEXT("GatePort"), (LPBYTE)&nPort)) nPort = 5100;
			InitGateCommSocket(g_gssock, &g_gsaddr, _IDM_GATECOMMSOCK_MSG, nPort, FD_ACCEPT|FD_READ|FD_CLOSE);

			UINT	dwThreadIDForMsg = 0;

			// 两个句柄分开存：关程序时要等它们退出（原来的写法把第一个句柄覆盖掉了）
			g_hDBMsgThread	 = (HANDLE)_beginthreadex(NULL, 0, ProcessDBMsg, NULL, 0, &dwThreadIDForMsg);
			g_hGateMsgThread = (HANDLE)_beginthreadex(NULL, 0, ProcessGateMsg, NULL, 0, &dwThreadIDForMsg);

			SwitchMenuItem(TRUE);

			return;
		}
		case IDM_STOPSERVICE:
		{
			StopServiceThreads();

			SwitchMenuItem(FALSE);

			return;
		}
		case IDM_CONFIG:
		{
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_CONFIGDLG), NULL, (DLGPROC)ConfigDlgFunc);

			return;
		}
		case IDM_EXIT:
		{
			// 菜单里的 Exit 走和标题栏 X 一样的确认流程：点取消就什么都不做
			PostMessage(g_hMainWnd, WM_CLOSE, 0, 0);

			return;
		}
	}
}

// **************************************************************************************
//
//			
//
// **************************************************************************************

LPARAM APIENTRY MainWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch (nMsg)
	{
		case _IDM_GATECOMMSOCK_MSG:
			return OnGateCommSockMsg(wParam, lParam);
		case _WM_LOGMSG:
		{
			// 工作线程投递过来的日志：这里在主线程，可以安全地写 ListView
			LPTSTR	pszMsg = (LPTSTR)lParam;

			if (pszMsg)
			{
				LogText(pszMsg);
				LocalFree(pszMsg);
			}

			return 0L;
		}
		case _WM_SESSIONCOUNT:
		{
			// 连接数由工作线程投递过来，这里在主线程刷状态栏（工作线程不等主线程）
			TCHAR	szText[32];

			wsprintf(szText, _TEXT("%d Sessions"), (int)wParam);

			SendMessage(g_hStatusBar, SB_SETTEXT, MAKEWORD(3, 0), (LPARAM)szText);

			return 0L;
		}
		case WM_COMMAND:
			OnCommand(wParam, lParam);
			break;
		case WM_SIZE:
		{
			if (g_hToolBar && g_hMainWnd && g_hStatusBar) 
			{
				RECT rcToolBar, rcMain, rcStatusBar;

				GetWindowRect(g_hToolBar, &rcToolBar);
				GetClientRect(g_hMainWnd, &rcMain);
				GetWindowRect(g_hStatusBar, &rcStatusBar);

				MoveWindow(g_hToolBar, 0, 0, LOWORD(lParam), (rcToolBar.bottom - rcToolBar.top), TRUE);
				MoveWindow(g_hStatusBar, 0, rcMain.bottom - (rcStatusBar.bottom - rcStatusBar.top), 
								LOWORD(lParam), (rcStatusBar.bottom - rcStatusBar.top), TRUE);
				MoveWindow(g_hLogMsgWnd, 0, (rcToolBar.bottom - rcToolBar.top) - 2, (rcMain.right - rcMain.left), 
								(rcMain.bottom - rcMain.top) - (rcToolBar.bottom - rcToolBar.top) - (rcStatusBar.bottom - rcStatusBar.top) + 2, 
								TRUE);

				Mir3FitLogColumns(g_hLogMsgWnd, g_hMainWnd);

				int	nStatusPartsWidths[_NUMOFMAX_STATUS_PARTS];
				int nCnt = 0;

				for (int i = _NUMOFMAX_STATUS_PARTS - 1; i >= 0 ; i--)
					nStatusPartsWidths[nCnt++] = (rcMain.right - rcMain.left) - (90 * i);

				nStatusPartsWidths[_NUMOFMAX_STATUS_PARTS - 1] = -1;

			SendMessage(g_hStatusBar, SB_SETPARTS, _NUMOFMAX_STATUS_PARTS, (LPARAM)nStatusPartsWidths);
			}

			break;
		}
		case WM_CLOSE:
		{
			DWORD	dwNow = GetTickCount();

			// 已经在确认中，或者刚关掉上一个确认框（连点 X）：
			// 直接忽略，不要再叠模态框——那种嵌套看起来就是"卡住"。
			if (g_fCloseConfirm || (dwNow - g_dwLastCloseTick) < 500)
				return 0L;

			TCHAR	szMsg[128] = { 0 };
			TCHAR	szTitle[128] = { 0 };
			int		nRet;

			// LoadString 的第三个参数是字符数，不是字节数
			LoadString(g_hInst, IDS_PROGRAM_QUIT, szMsg, sizeof(szMsg) / sizeof(TCHAR));
			LoadString(g_hInst, IDS_PROGRAM_TITLE, szTitle, sizeof(szTitle) / sizeof(TCHAR));

			g_fCloseConfirm = TRUE;

			// 默认按钮放在"否"：回车/ESC 都不会把服务器关掉
			nRet = MessageBox(g_hMainWnd, szMsg, szTitle, MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2);

			g_fCloseConfirm		= FALSE;
			g_dwLastCloseTick	= GetTickCount();

			// 取消：什么都不做。
			// 不置 g_fTerminated、不关 socket、不 WSACleanup、不 PostQuitMessage，
			// 也不能让 DefWindowProc 去 DestroyWindow —— 服务器继续正常跑。
			if (nRet != IDYES)
				return 0L;

			// 确认退出：先停线程，再拆网络和 COM，最后退出消息循环
			StopServiceThreads();

			ClearSocket(g_ssock);
			ClearSocket(g_gssock);

			WSACleanup();

			CoUninitialize();

			PostQuitMessage(0);

			return 0L;
		}
	}

	return (DefWindowProc(hWnd, nMsg, wParam, lParam));
}
