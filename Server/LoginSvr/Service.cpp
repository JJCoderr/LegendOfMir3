// LoginSvr.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "../def/dbmgr.h"

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

static WSADATA	g_wsd;


// **************************************************************************************


#include "stdafx.h"
#include <stdio.h>
#include "../def/dbmgr.h"


BOOL			jRegSetKey(LPCTSTR pSubKeyName, LPCTSTR pValueName, DWORD dwFlags, LPBYTE pValue, DWORD nValueSize);
BOOL			jRegGetKey(LPCTSTR pSubKeyName, LPCTSTR pValueName, LPBYTE pValue);


LPARAM			OnGateCommSockMsg(WPARAM wParam, LPARAM lParam);

BOOL			InitServerThreadForMsg();


extern CWHList< GAMESERVERINFO * >	g_xGameServerList;
extern CWHList< GATESERVERINFO * >	g_xGateServerList;
extern char							g_szServerList[1024];

extern HINSTANCE		g_hInst;

extern HWND				g_hMainWnd;
extern HWND				g_hLogMsgWnd;
extern HWND				g_hToolBar;
extern HWND				g_hStatusBar;

extern unsigned long	g_hThreadForMsg;

SOCKET					g_gcSock = INVALID_SOCKET;
SOCKADDR_IN				g_gcAddr;

BOOL					g_fTerminated = FALSE;


#include "../Service/Logging.inl"
UINT WINAPI LoadAccountRecords(LPVOID lpParameter)
{
	InsertLogMsg(IDS_LOADACCOUNTRECORDS);

	// 数据库没连上时 CreateRecordset() 返回 NULL（CDBManager 的空指针保护会直接
	// 返回 NULL）。这里必须先判断：拿 NULL 去 Execute() 就是 0xC0000005，
	// 表现就是点 Start Service 之后进程直接消失。
	CRecordset *pRec = GetDBManager()->CreateRecordset();

	if ( pRec == NULL )
	{
		InsertLogMsg( IDS_DBCONNECTFAIL );
		return 0L;
	}

	pRec->Execute( "UPDATE TBL_ACCOUNT SET FLD_CERTIFICATION=0 WHERE FLD_CERTIFICATION >= 30" );
	GetDBManager()->DestroyRecordset( pRec );
	// ----------------------------------------------------------------------------------------

	GAMESERVERINFO *pServerInfo;

	pRec = GetDBManager()->CreateRecordset();
	if ( pRec && pRec->Execute( "SELECT * FROM TBL_SERVERINFO" ) )
	{
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
	if ( pRec )
		GetDBManager()->DestroyRecordset( pRec );

	char szTmp[64];
	for ( PLISTNODE pNode = g_xGameServerList.GetHead(); pNode; pNode = g_xGameServerList.GetNext( pNode ) )
	{
		pServerInfo = g_xGameServerList.GetData( pNode );

		sprintf( szTmp, "%d,%s,", pServerInfo->index, pServerInfo->name );
		strcat( g_szServerList, szTmp );
	}
	// ----------------------------------------------------------------------------------------

	InitServerThreadForMsg();

	int port = 5500;
	jRegGetKey(_LOGIN_SERVER_REGISTRY, _TEXT("LocalPort"), (LPBYTE)&port);
	if (InitServerSocket(g_gcSock, &g_gcAddr, _IDM_GATECOMMSOCK_MSG, port, 1))
		ReportServiceState(TRUE);

	return 0L;
}
bool Initialize(HINSTANCE instance, HWND dispatcher) {
    g_hInst=instance;g_hMainWnd=dispatcher;
if (WSAStartup(MAKEWORD(2, 2), &g_wsd) != 0)
		return (FALSE);

	if (!GetDBManager()->Init( InsertLogMsg, "Mir2_Account", "sa", "prg" )) return false;

	//
//	BYTE	btInstalled;

//	if (!jRegGetKey(_LOGIN_SERVER_REGISTRY, _TEXT("Installed"), (LPBYTE)&btInstalled))
//		CreateConfigProperties();



	return TRUE;

}
void Start() {

			g_fTerminated = FALSE;

			UINT			dwThreadIDForMsg = 0;
			unsigned long	hThreadForMsg = 0;

			hThreadForMsg = _beginthreadex(NULL, 0, LoadAccountRecords, NULL, 0, &dwThreadIDForMsg);

			return;

}
void Stop() {

			ClearSocket(g_gcSock);

			ReportServiceState(FALSE);

			return;

}
LRESULT CALLBACK Dispatch(HWND, UINT message, WPARAM w, LPARAM l) {
switch(message) {

}
return 0;
}
