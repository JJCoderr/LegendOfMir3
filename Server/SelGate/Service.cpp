// SelGate.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

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

//list<CUserInfo> TUserInfoList;
//CUserInfo TUserInfoList[1500];



int				g_nStatusPartsWidths[_NUMOFMAX_STATUS_PARTS] = { 10, 10, 10, -1 };

// **************************************************************************************


#include "stdafx.h"

LPARAM OnServerSockMsg(WPARAM wParam, LPARAM lParam);
LPARAM OnClientSockMsg(WPARAM wParam, LPARAM lParam);

BOOL	jRegSetKey(LPCTSTR pSubKeyName, LPCTSTR pValueName, DWORD dwFlags, LPBYTE pValue, DWORD nValueSize);
BOOL	jRegGetKey(LPCTSTR pSubKeyName, LPCTSTR pValueName, LPBYTE pValue);


VOID WINAPI OnTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

extern HINSTANCE				g_hInst;

extern HWND						g_hMainWnd;
extern HWND						g_hLogMsgWnd;
extern HWND						g_hToolBar;
extern HWND						g_hStatusBar;

extern HANDLE					g_hMsgThread;

#ifndef _SOCKET_ASYNC_IO
extern HANDLE					g_hIOCP;
extern HANDLE					g_hAcceptThread;
#endif

SOCKET			g_ssock = INVALID_SOCKET;
SOCKADDR_IN		g_saddr;

SOCKET			g_csock = INVALID_SOCKET;
SOCKADDR_IN		g_caddr;

BOOL			g_fTerminated = FALSE;



#include "../Service/Logging.inl"

bool Initialize(HINSTANCE instance, HWND dispatcher) {
    g_hInst=instance;g_hMainWnd=dispatcher;
if (WSAStartup(MAKEWORD(2, 2), &g_wsd) != 0)
		return (FALSE);

	//
	BYTE	btInstalled;

	if (!jRegGetKey(_SELGATE_SERVER_REGISTRY, _TEXT("Installed"), (LPBYTE)&btInstalled))
		return FALSE;

	return TRUE;

}
void Start() {

			DWORD	dwIP = 0;
			int		nPort = 0;

			g_fTerminated = FALSE;

			if (!jRegGetKey(_SELGATE_SERVER_REGISTRY, _TEXT("LocalPort"), (LPBYTE)&nPort))
				nPort = 7100;

			InitServerSocket(g_ssock, &g_saddr, _IDM_SERVERSOCK_MSG, nPort, 2);

			jRegGetKey(_SELGATE_SERVER_REGISTRY, _TEXT("RemoteIP"), (LPBYTE)&dwIP);

			if (!jRegGetKey(_SELGATE_SERVER_REGISTRY, _TEXT("RemotePort"), (LPBYTE)&nPort))
				nPort = 5100;

			ConnectToServer(g_csock, &g_caddr, _IDM_CLIENTSOCK_MSG, NULL, dwIP, nPort, FD_CONNECT|FD_READ|FD_CLOSE);










			InsertLogMsg(IDS_STARTSERVICE);



			return;

}
void Stop() {

			g_fTerminated = TRUE;

/*			if (g_hAcceptThread != INVALID_HANDLE_VALUE)
			{
				TerminateThread(g_hAcceptThread, 0);
				WaitForSingleObject(g_hAcceptThread, INFINITE);
				CloseHandle(g_hAcceptThread);
				g_hAcceptThread = INVALID_HANDLE_VALUE;
			}

			if (g_hMsgThread != INVALID_HANDLE_VALUE)
			{
				TerminateThread(g_hMsgThread, 0);
				WaitForSingleObject(g_hMsgThread, INFINITE);
				CloseHandle(g_hMsgThread);
				g_hMsgThread = INVALID_HANDLE_VALUE;
			}
*/
			ClearSocket(g_ssock);
			ClearSocket(g_csock);

			CloseHandle(g_hIOCP);










			InsertLogMsg(IDS_STOPSERVICE);



			return;

}
LRESULT CALLBACK Dispatch(HWND, UINT message, WPARAM w, LPARAM l) {
switch(message) {
#ifdef _SOCKET_ASYNC_IO
case _IDM_SERVERSOCK_MSG: return OnServerSockMsg(w,l);
#endif
case _IDM_CLIENTSOCK_MSG: return OnClientSockMsg(w,l);
}
return 0;
}
