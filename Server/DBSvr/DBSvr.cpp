// DBSvr.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "../def/dbmgr.h"
#include "TableList.h"

#define _BMP_CX				16
#define _BMP_CY				16

// **************************************************************************************

BOOL	InitApplication(HANDLE hInstance);
BOOL	InitInstance(HANDLE hInstance, int nCmdShow);
LPARAM	APIENTRY MainWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

BOOL	jRegSetKey(LPCTSTR pSubKeyName, LPCTSTR pValueName, DWORD dwFlags, LPBYTE pValue, DWORD nValueSize);
BOOL	jRegGetKey(LPCTSTR pSubKeyName, LPCTSTR pValueName, LPBYTE pValue);

BOOL	CALLBACK ConfigDlgFunc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

void	CreateConfigProperties();

// **************************************************************************************
//
//			Global Variables Definition
//
// **************************************************************************************

HINSTANCE		g_hInst = NULL;				// Application instance
HWND			g_hMainWnd = NULL;			// Main window handle
HWND			g_hLogMsgWnd = NULL;
HWND			g_hToolBar = NULL;
HWND			g_hStatusBar = NULL;

// 创建主窗口的线程 ID：界面（ListView / 状态栏）只能在它上面动
DWORD			g_dwMainThreadId = 0;

static WSADATA	g_wsd;

TBBUTTON tbButtons[] = 
{
	{ 0, IDM_STARTSERVICE,	TBSTATE_ENABLED,	TBSTYLE_BUTTON, 0L, 0},
	{ 1, IDM_STOPSERVICE,	TBSTATE_ENABLED,	TBSTYLE_BUTTON, 0L, 0}
};

// **************************************************************************************

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;

	g_dwMainThreadId = GetCurrentThreadId();

//	if (CheckAvailableIOCP())
//	{
		if (!InitApplication(hInstance))
			return (FALSE);

		if (!InitInstance(hInstance, nCmdShow))
			return (FALSE);

		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
/*	}
	else
	{
		TCHAR szMsg[1024];

		LoadString(hInstance, IDS_NOTWINNT, szMsg, sizeof(szMsg));

		MessageBox(NULL, szMsg, _LOGINGATE_SERVER_TITLE, MB_OK|MB_ICONINFORMATION);
		
		return -1;
	}
*/
    return (msg.wParam);
}

// **************************************************************************************
//
//			
//
// **************************************************************************************

BOOL InitApplication(HANDLE hInstance)
{
    WNDCLASS  wc;

    wc.style            = CS_GLOBALCLASS|CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc      = (WNDPROC)MainWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hIcon            = LoadIcon((HINSTANCE)hInstance, MAKEINTRESOURCE(IDI_MIR2));
    wc.hInstance        = (HINSTANCE)hInstance;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName     = MAKEINTRESOURCE(IDR_MAINMENU);
    wc.lpszClassName    = _DB_SERVER_CLASS;

	return RegisterClass(&wc);
}

// **************************************************************************************
//
//			
//
// **************************************************************************************

BOOL InitInstance(HANDLE hInstance, int nCmdShow)
{
	g_hInst = (HINSTANCE)hInstance;
	
	OleInitialize(NULL);

	INITCOMMONCONTROLSEX	icex;

	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES | ICC_INTERNET_CLASSES | ICC_TAB_CLASSES;

	InitCommonControlsEx(&icex);

    g_hMainWnd = CreateWindowEx(0, _DB_SERVER_CLASS, _DB_SERVER_TITLE, 
							WS_OVERLAPPEDWINDOW|WS_VISIBLE,
							CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,                 
							NULL, NULL, (HINSTANCE)hInstance, NULL);

	// [现代外观] 先用逻辑尺寸把窗口摆好，避免首次显示时按 CW_USEDEFAULT 撑满屏幕
	Mir3SizeMainWindow(g_hMainWnd, _MAINWND_WIDTH, _MAINWND_HEIGHT);

	g_hToolBar = CreateToolbarEx(g_hMainWnd, WS_CHILD|CCS_TOP|WS_VISIBLE|WS_BORDER,
									_IDW_TOOLBAR, sizeof(tbButtons) / sizeof(TBBUTTON), (HINSTANCE)hInstance, IDB_TOOLBAR,
									(LPCTBBUTTON)&tbButtons, sizeof(tbButtons) / sizeof(TBBUTTON),
									_BMP_CX, _BMP_CY, _BMP_CX, _BMP_CY, sizeof(TBBUTTON));

	RECT rcMainWnd, rcToolBar, rcStatusBar;

	GetClientRect(g_hMainWnd, &rcMainWnd);
	GetWindowRect(g_hToolBar, &rcToolBar);

	g_hStatusBar = CreateWindowEx(0L, STATUSCLASSNAME, _T(""), WS_CHILD|WS_BORDER|WS_VISIBLE|SBS_SIZEGRIP,
									0, rcMainWnd.bottom - _STATUS_HEIGHT, (rcMainWnd.right - rcMainWnd.left), _STATUS_HEIGHT, g_hMainWnd, (HMENU)_IDW_STATUSBAR, g_hInst, NULL);

	int	nStatusPartsWidths[_NUMOFMAX_STATUS_PARTS];
	int nCnt = 0;
	
	for (int i = _NUMOFMAX_STATUS_PARTS - 1; i >= 0; i--)
		nStatusPartsWidths[nCnt++] = (rcMainWnd.right - rcMainWnd.left) - (90 * i);

	nStatusPartsWidths[_NUMOFMAX_STATUS_PARTS - 1] = -1;

	SendMessage(g_hStatusBar, SB_SETPARTS, _NUMOFMAX_STATUS_PARTS, (LPARAM)nStatusPartsWidths);
	SendMessage(g_hStatusBar, SB_SETTEXT, MAKEWORD(1, 0), (LPARAM)_T("Not Connected"));

	GetWindowRect(g_hStatusBar, &rcStatusBar);

    g_hLogMsgWnd = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, _TEXT(""), 
							WS_CHILD|WS_VISIBLE|WS_BORDER|LVS_REPORT|LVS_EDITLABELS,
							0, (rcToolBar.bottom - rcToolBar.top) - 2, (rcMainWnd.right - rcMainWnd.left), 
							(rcMainWnd.bottom - rcMainWnd.top) - (rcToolBar.bottom - rcToolBar.top)- (rcStatusBar.bottom - rcStatusBar.top) + 2,
							g_hMainWnd, NULL, (HINSTANCE)hInstance, NULL);

	ListView_SetExtendedListViewStyleEx(g_hLogMsgWnd, 0, LVS_EX_FULLROWSELECT);

	LV_COLUMN	lvc;
	TCHAR		szText[64];

	lvc.mask	= LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt		= LVCFMT_LEFT;
	lvc.cx		= 100;
	lvc.pszText	= szText;

	for (i = 0; i < 3; i++)
	{
		lvc.iSubItem = i;
		LoadString((HINSTANCE)hInstance, IDS_LVS_LABEL1 + i, szText, sizeof(szText));
		
		ListView_InsertColumn(g_hLogMsgWnd, i, &lvc);
	}
	Mir3FitLogColumns(g_hLogMsgWnd, g_hMainWnd);

	SendMessage(g_hToolBar, TB_SETSTATE, (WPARAM)IDM_STOPSERVICE, (LPARAM)MAKELONG(TBSTATE_INDETERMINATE, 0));

	ShowWindow(g_hMainWnd, SW_SHOW);
	UpdateWindow(g_hMainWnd);
	// [现代外观] 按 96 DPI 逻辑尺寸设定主窗口大小并居中：CW_USEDEFAULT 的默认尺寸太大。
	Mir3SizeMainWindow(g_hMainWnd, _MAINWND_WIDTH, _MAINWND_HEIGHT);

	// 尺寸确定后再按实际客户区重新布局一次：工具栏/状态栏/日志列表跟上新大小，
	// Message 列按真实宽度重算，避免列宽按旧的默认窗口尺寸算出来。
	RECT rcLayout;

	GetClientRect(g_hMainWnd, &rcLayout);
	SendMessage(g_hMainWnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rcLayout.right, rcLayout.bottom));

	if (WSAStartup(MAKEWORD(2, 2), &g_wsd) != 0)
		return (FALSE);

	BYTE	btInstalled;

	if (!jRegGetKey(_DB_SERVER_REGISTRY, _TEXT("Installed"), (LPBYTE)&btInstalled))
		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_CONFIGDLG), NULL, (DLGPROC)ConfigDlgFunc);

	TCHAR	wszDatabase[256];
	char	szDatabase[256];

	ZeroMemory(wszDatabase, sizeof(wszDatabase));
	ZeroMemory(szDatabase, sizeof(szDatabase));

	jRegGetKey(_DB_SERVER_REGISTRY, _TEXT("Device"), (LPBYTE)wszDatabase);
	WideCharToMultiByte(CP_ACP, 0, wszDatabase, -1, szDatabase, sizeof(szDatabase), NULL, NULL);

	// 数据库连不上时只记日志，不能顺手崩掉：
	// 后面再拿空连接去查表 / 释放，就是"点取消（关配置框）之后程序自己退出"的那种崩溃。
	if ( !GetDBManager()->Init( InsertLogMsg, szDatabase, "sa", "prg" ) )
	{
		InsertLogMsg( _T("数据库连接失败：请用 Configuration 检查数据库设置（ODBC DSN 名称 / 账号 / 密码）。") );

		return TRUE;
	}

	// 读取表
	CConnection *pConn = GetDBManager()->m_dbMain.CreateConnection( "Mir2_Common", "sa", "prg" );
	if ( pConn )
	{
		if ( !GetTblStartPoint()->Init( pConn ) )
			InsertLogMsg( _T("failed to read TBL_STARTPOINT table\n") );

		GetDBManager()->m_dbMain.DestroyConnection( pConn );
	}
	else
		InsertLogMsg( _T("读取 TBL_STARTPOINT 失败：连不上 Mir2_Common 数据库。") );

	return TRUE;
}

// **************************************************************************************
//
//			日志输出
//
// **************************************************************************************
//
// 日志列表（ListView）只有主线程能碰。
//
// 以前各个工作线程（ProcessDBMsg / ProcessGateMsg / AcceptThread / IOCP worker）
// 都是直接调 ListView_* 跨线程改列表的：主线程一忙（比如停在关闭确认框里、
// 或者在等 ODBC 返回），工作线程就跟着阻塞在 SendMessage 上，两边一起卡住。
//
// 现在改成：工作线程只把文本 Post 给主线程，立刻返回；主线程在窗口过程
// (_WM_LOGMSG) 里统一写列表。谁也不会互相等。
// **************************************************************************************

int AddNewLogMsg()
{
	LV_ITEM		lvi;
	TCHAR		szText[64];

	int nCount = ListView_GetItemCount(g_hLogMsgWnd);

	if (nCount >= 50)
	{
		ListView_DeleteItem(g_hLogMsgWnd, 0);
		nCount--;
	}

	lvi.mask		= LVIF_TEXT;
	lvi.iItem		= nCount;
	lvi.iSubItem	= 0;
	
	_tstrdate(szText);

	lvi.pszText = szText;
	
	ListView_InsertItem(g_hLogMsgWnd, &lvi);

	_tstrtime(szText);

	ListView_SetItemText(g_hLogMsgWnd, nCount, 1, szText);

	return nCount;
}

// 主线程专用：把一条文本追加到日志列表
static void AppendLogText(LPCTSTR pszMsg)
{
	if (g_hLogMsgWnd == NULL || pszMsg == NULL)
		return;

	int	nCount	= AddNewLogMsg();
	int	nLen	= lstrlen(pszMsg);

	if (nLen > 256)
		nLen = 256;

	TCHAR	szText[260];

	lstrcpyn(szText, pszMsg, nLen + 1);

	ListView_SetItemText(g_hLogMsgWnd, nCount, 2, szText);
	ListView_Scroll(g_hLogMsgWnd, 0, 8);
}

// 任意线程：主线程直接写列表，其它线程投递给主线程
void LogText(LPCTSTR pszMsg)
{
	if (pszMsg == NULL)
		return;

	if (GetCurrentThreadId() == g_dwMainThreadId)
	{
		AppendLogText(pszMsg);
		return;
	}

	size_t	nBytes	= (lstrlen(pszMsg) + 1) * sizeof(TCHAR);
	LPTSTR	pszCopy	= (LPTSTR)LocalAlloc(LPTR, nBytes);

	if (pszCopy == NULL)
		return;

	memcpy(pszCopy, pszMsg, nBytes);

	if (!PostMessage(g_hMainWnd, _WM_LOGMSG, (WPARAM)0, (LPARAM)pszCopy))
		LocalFree(pszCopy);
}

void InsertLogMsg(UINT nID)
{
	TCHAR	szText[256];

	if (LoadString(g_hInst, nID, szText, sizeof(szText) / sizeof(TCHAR)) == 0)
		return;

	LogText(szText);
}

void InsertLogMsg(LPTSTR lpszMsg)
{
	LogText(lpszMsg);
}

void InsertLogMsgParam(UINT nID, void *pParam, BYTE btFlags)
{
	TCHAR	szText[128];
	TCHAR	szMsg[256];

	if (LoadString(g_hInst, nID, szText, sizeof(szText) / sizeof(TCHAR)) == 0)
		return;

	switch (btFlags)
	{
		case LOGPARAM_STR:
			_stprintf(szMsg, szText, (LPTSTR)pParam);
			break;
		case LOGPARAM_INT:
			_stprintf(szMsg, szText, *(int *)pParam);
			break;
		default:
			return;
	}

	LogText(szMsg);
}
