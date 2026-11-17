// LoginGate.cpp : Defines the entry point for the application.
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

BOOL			g_fTerminated = FALSE;

static WSADATA	g_wsd;



// **************************************************************************************


#include "stdafx.h"

LPARAM OnServerSockMsg(WPARAM wParam, LPARAM lParam);
LPARAM OnClientSockMsg(WPARAM wParam, LPARAM lParam);

BOOL	jRegSetKey(LPCTSTR pSubKeyName, LPCTSTR pValueName, DWORD dwFlags, LPBYTE pValue, DWORD nValueSize);
BOOL	jRegGetKey(LPCTSTR pSubKeyName, LPCTSTR pValueName, LPBYTE pValue);


extern HINSTANCE		g_hInst;

extern HWND				g_hMainWnd;
extern HWND				g_hLogMsgWnd;
extern HWND				g_hToolBar;
extern HWND				g_hStatusBar;

extern HANDLE			g_hThreadForComm;

SOCKET			g_ssock = INVALID_SOCKET;
SOCKADDR_IN		g_saddr;

SOCKET			g_csock = INVALID_SOCKET;
SOCKADDR_IN		g_caddr;

extern CWHAbusive	g_xAbusive;


#include "../Service/Logging.inl"

bool Initialize(HINSTANCE instance, HWND dispatcher) {
    g_hInst=instance;g_hMainWnd=dispatcher;
if (WSAStartup(MAKEWORD(2, 2), &g_wsd) != 0)
		return (FALSE);

	BYTE	btInstalled;

	if (!jRegGetKey(_GAMEGATE_SERVER_REGISTRY, _TEXT("Installed"), (LPBYTE)&btInstalled))
		return FALSE;

	return TRUE;

}
void Start() {

			DWORD	dwIP = 0;
			int		nPort = 0;

			g_fTerminated = FALSE;

			if (!jRegGetKey(_GAMEGATE_SERVER_REGISTRY, _TEXT("LocalPort"), (LPBYTE)&nPort))
				nPort = 7200;

			InitServerSocket(g_ssock, &g_saddr, _IDM_SERVERSOCK_MSG, nPort, 2);

			jRegGetKey(_GAMEGATE_SERVER_REGISTRY, _TEXT("RemoteIP"), (LPBYTE)&dwIP);

			if (!jRegGetKey(_GAMEGATE_SERVER_REGISTRY, _TEXT("RemotePort"), (LPBYTE)&nPort))
				nPort = 5000;

			ConnectToServer(g_csock, &g_caddr, _IDM_CLIENTSOCK_MSG, NULL, dwIP, nPort, FD_CONNECT|FD_READ|FD_CLOSE);

			ReportServiceState(TRUE);



			g_xAbusive.LoadAbusiveList();

			return;

}
void Stop() {

			g_fTerminated = TRUE;

			ClearSocket(g_ssock);
			ClearSocket(g_csock);

			ReportServiceState(FALSE);



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
