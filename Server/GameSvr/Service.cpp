// GameSvr.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
BOOL jRegGetKeyBuf(LPCTSTR, LPCTSTR, LPBYTE, DWORD);

#define _BMP_CX						16
#define _BMP_CY						16

// **************************************************************************************


BOOL	jRegGetKey(LPCTSTR pSubKeyName, LPCTSTR pValueName, LPBYTE pValue);


static WSADATA	g_wsd;

void __cbDBMsg( char *pState, int nNativeCode, char *pDesc )
{
	static TCHAR szState[256], szDesc[2048];
	static TCHAR szMsg[2048];

	MultiByteToWideChar( CP_ACP, 0, pState, -1, szState, sizeof( szState ) / sizeof( TCHAR ) );
	MultiByteToWideChar( CP_ACP, 0, pDesc, -1, szDesc, sizeof( szDesc ) / sizeof( TCHAR ) );

	wsprintf( szMsg, _T("ODBC MsgID: %s(%d)"), szState, nNativeCode );
	InsertLogMsg( szMsg );

	wsprintf( szMsg, _T("%s"), szDesc );
	InsertLogMsg( szMsg );
}

#include "stdafx.h"
BOOL jRegGetKeyBuf(LPCTSTR, LPCTSTR, LPBYTE, DWORD);

LPARAM OnClientSockMsg(WPARAM wParam, LPARAM lParam);
LPARAM OnLogSvrSockMsg(WPARAM wParam, LPARAM lParam);

BOOL	jRegSetKey(LPCTSTR pSubKeyName, LPCTSTR pValueName, DWORD dwFlags, LPBYTE pValue, DWORD nValueSize);
BOOL	jRegGetKey(LPCTSTR pSubKeyName, LPCTSTR pValueName, LPBYTE pValue);


void InitMonsterGenInfo();
void InitMagicInfo();
void InitMonRaceInfo();
void InitStdItemSpecial();
void InitStdItemEtcInfo();
void InitMerchantInfo();
void InitMoveMapEventInfo();
CMapInfo* InitMapInfo(int nServerIndex);

void InitAdminCommandList();
void UnInitAdminCommandList();
void LoadMap(CMapInfo* pMapInfo);


#include "../Service/Logging.inl"
CMapInfo* InitDataInDatabase()
{
	int nServerIndex;

	InitMagicInfo();
	InitMonRaceInfo();
	InitMonsterGenInfo();
	InitStdItemSpecial();
	InitStdItemEtcInfo();
	InitMerchantInfo();
	InitMoveMapEventInfo();

	if (!jRegGetKey(_GAME_SERVER_REGISTRY, _TEXT("ServerNumber"), (LPBYTE)&nServerIndex))
		return FALSE;

	return InitMapInfo(nServerIndex);
}
UINT WINAPI InitializingServer(LPVOID lpParameter)
{
	TCHAR		wszPath[128];
	TCHAR		wszFullPath[256];

	ZeroMemory(wszPath, sizeof(wszPath));
	ZeroMemory(wszFullPath, sizeof(wszFullPath));

	ZeroMemory(g_szGoldName, sizeof(g_szGoldName));
	LoadString(g_hInst, IDS_GOLD, wszPath, sizeof(wszPath)/sizeof(TCHAR));
	WideCharToMultiByte(CP_ACP, 0, wszPath, -1, g_szGoldName, sizeof(g_szGoldName), NULL, NULL);

	jRegGetKeyBuf(_GAME_SERVER_REGISTRY, _TEXT("MapFileLoc"), (LPBYTE)wszPath, sizeof(wszPath));

	lstrcpy(wszFullPath, wszPath);
	lstrcat(wszFullPath, _TEXT("\\"));
	lstrcat(wszFullPath, _TEXT("searchTable.tbl"));

    LoadSearchTable(wszFullPath);

	CMapInfo* pMapInfo = (CMapInfo*)lpParameter;

	for (int i = 0; i < g_nNumOfMapInfo; i++)
		LoadMap(&pMapInfo[i]);

	delete [] pMapInfo;
	pMapInfo = NULL;

	InitAdminCommandList();

	BYTE	btInstalled;

	if (!jRegGetKey(_GAME_SERVER_REGISTRY, _TEXT("Installed"), (LPBYTE)&btInstalled))
		return FALSE;

	DWORD	dwIP = 0;
	TCHAR	szPort[24];
	int		nPort = 0;

	jRegGetKey(_GAME_SERVER_REGISTRY, _TEXT("DBServerIP"), (LPBYTE)&dwIP);
	jRegGetKey(_GAME_SERVER_REGISTRY, _TEXT("DBServerPort"), (LPBYTE)&nPort);
	_itow(nPort, szPort, 10);

	ConnectToServer(g_csock, &g_caddr, _IDM_CLIENTSOCK_MSG, NULL, dwIP, nPort, FD_CONNECT|FD_READ|FD_CLOSE);
//	ConnectToServer(g_clsock, &g_claddr, _IDM_LOGSVRSOCK_MSG, NULL, dwIP, 5500, FD_CONNECT|FD_READ|FD_CLOSE);

	return 0L;
}
void UnInitializingServer()
{
	if (g_pMagicInfo)
	{
		delete [] g_pMagicInfo;
		g_pMagicInfo = NULL;
	}

	if (g_pMonGenInfo)
	{
		delete [] g_pMonGenInfo;
		g_pMonGenInfo = NULL;
	}

	if (g_pMonRaceInfo)
	{
		delete [] g_pMonRaceInfo;
		g_pMonRaceInfo = NULL;
	}

	if (g_pStdItemSpecial)
	{
		delete [] g_pStdItemSpecial;
		g_pStdItemSpecial = NULL;
	}

	if (g_pStdItemEtc)
	{
		delete [] g_pStdItemEtc;
		g_pStdItemEtc = NULL;
	}

	if (g_pMerchantInfo)
	{
		delete [] g_pMerchantInfo;
		g_pMerchantInfo = NULL;
	}

	if (g_pMoveMapEventInfo)
	{
		delete [] g_pMoveMapEventInfo;
		g_pMoveMapEventInfo = NULL;
	}
}
bool Initialize(HINSTANCE instance, HWND dispatcher) {
    g_hInst=instance;g_hMainWnd=dispatcher;
if (WSAStartup(MAKEWORD(2, 2), &g_wsd) != 0)
		return (FALSE);

	srand((unsigned)time(NULL));

	BYTE	btInstalled;

	if (!jRegGetKey(_GAME_SERVER_REGISTRY, _TEXT("Installed"), (LPBYTE)&btInstalled))
		return FALSE;

	TCHAR	wszDatabase[256];
	char	szDatabase[256];

	ZeroMemory(wszDatabase, sizeof(wszDatabase));
	ZeroMemory(szDatabase, sizeof(szDatabase));

	jRegGetKeyBuf(_GAME_SERVER_REGISTRY, _TEXT("Device"), (LPBYTE)wszDatabase, sizeof(wszDatabase));
	WideCharToMultiByte(CP_ACP, 0, wszDatabase, -1, szDatabase, sizeof(szDatabase), NULL, NULL);

	g_MirDB.SetDiagRec( __cbDBMsg );
	g_MirDB.Init();

	g_pConnCommon	= g_MirDB.CreateConnection( "Mir2_Common", "sa", "prg" );
	g_pConnGame		= g_MirDB.CreateConnection( szDatabase, "sa", "prg" );

	return g_pConnCommon && g_pConnGame;

}
void Start() {

			g_fTerminated = FALSE;

			CMapInfo* pMapInfo = InitDataInDatabase();

			UINT			dwThreadIDForMsg = 0;
			unsigned long	hThreadForMsg = 0;

			hThreadForMsg = _beginthreadex(NULL, 0, InitializingServer, pMapInfo, 0, &dwThreadIDForMsg);

			ReportServiceState(TRUE);

			return;

}
void Stop() {

			g_fTerminated = TRUE;

			ReportServiceState(FALSE);

			UnInitializingServer();
			UnInitAdminCommandList();

			return;

}
LRESULT CALLBACK Dispatch(HWND, UINT message, WPARAM w, LPARAM l) {
switch(message) {
#ifdef _SOCKET_ASYNC_IO
case _IDM_SERVERSOCK_MSG: return OnServerSockMsg(w,l);
#endif
case _IDM_CLIENTSOCK_MSG: return OnClientSockMsg(w,l);
case _IDM_LOGSVRSOCK_MSG: return OnLogSvrSockMsg(w,l);
}
return 0;
}
