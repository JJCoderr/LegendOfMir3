// ManConsole.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include "ManConsole.h"

// **************************************************************************************

INT	InitInstance(HINSTANCE hInstance);

// **************************************************************************************
//
//			Global Variables Definition
//
// **************************************************************************************

HINSTANCE	g_hInst = NULL;				// Application instance
HWND		g_hMainWnd = NULL;			// Main dialog handle
HWND		g_hLogMsgWnd = NULL;		// Log list view
HWND		g_hCommandWnd = NULL;		// Command edit box

static WSADATA	g_wsd;

// **************************************************************************************

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (WSAStartup(MAKEWORD(2, 2), &g_wsd) != 0)
	{
		TCHAR szMsg[256];

		LoadString(hInstance, IDP_SOCKETS_INIT_FAILED, szMsg, sizeof(szMsg) / sizeof(TCHAR));
		MessageBox(NULL, szMsg, _MANCONSOLE_TITLE, MB_OK|MB_ICONERROR);

		return (-1);
	}

	int nResponse = InitInstance(hInstance);

	WSACleanup();

	return (nResponse);
}

// **************************************************************************************
//
//			
//
// **************************************************************************************

INT InitInstance(HINSTANCE hInstance)
{
	INITCOMMONCONTROLSEX	icex;

	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES;

	InitCommonControlsEx(&icex);

	g_hInst = hInstance;

	// The main window is the dialog template itself, so the application
	// is driven by the dialog's own message loop.
	return (INT)DialogBox(g_hInst, MAKEINTRESOURCE(IDD_MANCONSOLE_DIALOG), NULL, (DLGPROC)ManConsoleDlgProc);
}

// **************************************************************************************
//
//			
//
// **************************************************************************************

int AddNewLogMsg()
{
	LV_ITEM		lvi;
	TCHAR		szText[64];

	int nCount = ListView_GetItemCount(g_hLogMsgWnd);

	if (nCount >= 100)
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

void InsertLogMsg(LPCTSTR lpszMsg)
{
	int nCount = AddNewLogMsg();

	ListView_SetItemText(g_hLogMsgWnd, nCount, 2, (LPTSTR)lpszMsg);

#ifdef _DEBUG
	_RPT1(_CRT_WARN, "%ls\n", lpszMsg);
#endif
}
