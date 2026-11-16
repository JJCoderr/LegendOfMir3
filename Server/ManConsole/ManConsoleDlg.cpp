// ManConsoleDlg.cpp : implementation file
//

#include "stdafx.h"

#include "ManConsole.h"

// **************************************************************************************
//
//			CAboutDlg dialog used for App About
//
// **************************************************************************************

INT_PTR CALLBACK AboutDlgProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			return TRUE;
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
					return EndDialog(hWndDlg, LOWORD(wParam));
			}

			break;
		}
	}

	return FALSE;
}

// **************************************************************************************
//
//			CManConsoleDlg dialog
//
// **************************************************************************************

INT_PTR CALLBACK ManConsoleDlgProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			g_hMainWnd = hWndDlg;

			HICON hIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE|LR_SHARED);

			SendMessage(hWndDlg, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIcon);
			SendMessage(hWndDlg, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIcon);

			// Add "About..." menu item to system menu.

			// IDM_ABOUTBOX must be in the system command range.
			HMENU hSysMenu = GetSystemMenu(hWndDlg, FALSE);

			if (hSysMenu != NULL)
			{
				TCHAR szAboutMenu[64];

				LoadString(g_hInst, IDS_ABOUTBOX, szAboutMenu, sizeof(szAboutMenu) / sizeof(TCHAR));

				if (szAboutMenu[0] != _T('\0'))
				{
					AppendMenu(hSysMenu, MF_SEPARATOR, 0, NULL);
					AppendMenu(hSysMenu, MF_STRING, IDM_ABOUTBOX, szAboutMenu);
				}
			}

			g_hLogMsgWnd = GetDlgItem(hWndDlg, IDC_LIST1);
			g_hCommandWnd = GetDlgItem(hWndDlg, IDC_EDIT1);

			ListView_SetExtendedListViewStyle(g_hLogMsgWnd, LVS_EX_FULLROWSELECT);

			LV_COLUMN	lvc;
			TCHAR		szText[64];

			lvc.mask	= LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.fmt		= LVCFMT_LEFT;
			lvc.cx		= 150;
			lvc.pszText	= szText;

			for (int i = 0; i < 3; i++)
			{
				lvc.iSubItem = i;
				lvc.cx = (i == 2) ? 400 : 150;

				LoadString(g_hInst, IDS_LVS_LABEL1 + i, szText, sizeof(szText) / sizeof(TCHAR));

				ListView_InsertColumn(g_hLogMsgWnd, i, &lvc);
			}

			Mir3FitLogColumns(g_hLogMsgWnd, hWndDlg);

			return TRUE;
		}
		case WM_SYSCOMMAND:
		{
			if ((wParam & 0xFFF0) == IDM_ABOUTBOX)
			{
				DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWndDlg, (DLGPROC)AboutDlgProc);

				return TRUE;
			}

			break;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
					EndDialog(hWndDlg, LOWORD(wParam));
					return TRUE;
			}

			break;
		}
		case WM_CLOSE:
		{
			EndDialog(hWndDlg, IDCANCEL);

			return TRUE;
		}
	}

	return FALSE;
}
