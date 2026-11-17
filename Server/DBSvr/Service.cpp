// DBSvr.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
BOOL jRegGetKeyBuf(LPCTSTR, LPCTSTR, LPBYTE, DWORD);
#include "../def/dbmgr.h"
#include "TableList.h"

#define _BMP_CX				16
#define _BMP_CY				16

// **************************************************************************************


BOOL	jRegSetKey(LPCTSTR pSubKeyName, LPCTSTR pValueName, DWORD dwFlags, LPBYTE pValue, DWORD nValueSize);
BOOL	jRegGetKey(LPCTSTR pSubKeyName, LPCTSTR pValueName, LPBYTE pValue);



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



// **************************************************************************************


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


#include "../Service/Logging.inl"
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
bool Initialize(HINSTANCE instance, HWND dispatcher) {
    g_hInst=instance;g_hMainWnd=dispatcher;
if (WSAStartup(MAKEWORD(2, 2), &g_wsd) != 0)
		return (FALSE);

	BYTE	btInstalled;

	if (!jRegGetKey(_DB_SERVER_REGISTRY, _TEXT("Installed"), (LPBYTE)&btInstalled))
		return FALSE;

	TCHAR	wszDatabase[256];
	char	szDatabase[256];

	ZeroMemory(wszDatabase, sizeof(wszDatabase));
	ZeroMemory(szDatabase, sizeof(szDatabase));

	jRegGetKeyBuf(_DB_SERVER_REGISTRY, _TEXT("Device"), (LPBYTE)wszDatabase, sizeof(wszDatabase));
	WideCharToMultiByte(CP_ACP, 0, wszDatabase, -1, szDatabase, sizeof(szDatabase), NULL, NULL);

	// 数据库连不上时只记日志，不能顺手崩掉：
	// 后面再拿空连接去查表 / 释放，就是"点取消（关配置框）之后程序自己退出"的那种崩溃。
	if ( !GetDBManager()->Init( InsertLogMsg, szDatabase, "sa", "prg" ) )
	{
		InsertLogMsg( _T("数据库连接失败：请用 Configuration 检查数据库设置（ODBC DSN 名称 / 账号 / 密码）。") );

		return FALSE;
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
void Start() {
int nPort;

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

			ReportServiceState(TRUE);

			return;

}
void Stop() {

			StopServiceThreads();

			ReportServiceState(FALSE);

			return;

}
LRESULT CALLBACK Dispatch(HWND, UINT message, WPARAM w, LPARAM l) {
switch(message) {
case _IDM_GATECOMMSOCK_MSG: return OnGateCommSockMsg(w,l);
}
return 0;
}
