// ManConsole.h : main header file for the MANCONSOLE application
//

#if !defined(AFX_MANCONSOLE_H__F8877CC8_9C56_4B4D_970F_49791465A3D9__INCLUDED_)
#define AFX_MANCONSOLE_H__F8877CC8_9C56_4B4D_970F_49791465A3D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _WINDOWS_
	#error include 'stdafx.h' before including this file
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
//
//			Global Variables Definition
//
// **************************************************************************************

extern HINSTANCE	g_hInst;			// Application instance
extern HWND			g_hMainWnd;			// Main dialog handle
extern HWND			g_hLogMsgWnd;		// Log list view
extern HWND			g_hCommandWnd;		// Command edit box

/////////////////////////////////////////////////////////////////////////////
//
//			Log message helpers
//
// **************************************************************************************

int		AddNewLogMsg();
void	InsertLogMsg(LPCTSTR lpszMsg);

/////////////////////////////////////////////////////////////////////////////
//
//			Dialog procedures
//
// **************************************************************************************

INT_PTR CALLBACK ManConsoleDlgProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutDlgProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MANCONSOLE_H__F8877CC8_9C56_4B4D_970F_49791465A3D9__INCLUDED_)
