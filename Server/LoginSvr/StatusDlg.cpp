// =============================================================================
//  Server/LoginSvr/StatusDlg.cpp
//
//      Service -> Game Server Connection Status
//
//      对话框里只读"内存中的运行状态快照"：
//          * 登录服务器自己的监听 socket 是否在工作（启动服务后 5500 在听）
//          * 已经连上来的 Login Gate（对端地址、端口、该网关上的客户端数）
//          * 数据库 TBL_SERVERINFO 里注册的游戏服务器（编号 / 名称 / 下发 IP / 选择次数）
//
//      这里不发任何网络连接、也不查数据库，所以弹窗和刷新都是毫秒级，
//      不会出现"点一下卡很久"。
// =============================================================================
#include "stdafx.h"

extern HINSTANCE					g_hInst;
extern HWND							g_hMainWnd;
extern SOCKET						g_gcSock;
extern CWHList<CGateInfo*>			g_xGateList;
extern CWHList<GAMESERVERINFO*>		g_xGameServerList;

#define _STATUS_COL_TYPE			0
#define _STATUS_COL_NAME			1
#define _STATUS_COL_ADDR			2
#define _STATUS_COL_PORT			3
#define _STATUS_COL_STATE			4
#define _STATUS_COL_COUNT			5

#define _STATUS_MAX_ROWS			512
#define _STATUS_LISTEN_PORT			5500

// **************************************************************************************

static int InsertStatusRow(HWND hList, int nItem, LPCTSTR pszType, LPCTSTR pszName,
						   LPCTSTR pszAddr, LPCTSTR pszPort, LPCTSTR pszState)
{
	LV_ITEM		lvi;
	int			nRow;

	ZeroMemory(&lvi, sizeof(lvi));

	lvi.mask		= LVIF_TEXT;
	lvi.iItem		= nItem;
	lvi.iSubItem	= 0;
	lvi.pszText		= (LPTSTR)pszType;

	nRow = ListView_InsertItem(hList, &lvi);

	if (nRow < 0)
		return nItem;

	ListView_SetItemText(hList, nRow, _STATUS_COL_NAME,  (LPTSTR)pszName);
	ListView_SetItemText(hList, nRow, _STATUS_COL_ADDR,  (LPTSTR)pszAddr);
	ListView_SetItemText(hList, nRow, _STATUS_COL_PORT,  (LPTSTR)pszPort);
	ListView_SetItemText(hList, nRow, _STATUS_COL_STATE, (LPTSTR)pszState);

	return nRow + 1;
}

// **************************************************************************************

static void AdjustStatusColumns(HWND hWndDlg)
{
	HWND		hList = GetDlgItem(hWndDlg, IDC_STATUS_LIST);
	RECT		rcClient;
	int			nTotal, nType, nName, nAddr, nPort, nState;

	if (!hList)
		return;

	GetClientRect(hList, &rcClient);

	nTotal = rcClient.right - rcClient.left;

	if (nTotal <= 0)
		return;

	nType	= Mir3DpiScale(hWndDlg, 90);
	nName	= Mir3DpiScale(hWndDlg, 150);
	nAddr	= Mir3DpiScale(hWndDlg, 110);
	nPort	= Mir3DpiScale(hWndDlg, 60);

	nState	= nTotal - nType - nName - nAddr - nPort
			  - GetSystemMetrics(SM_CXVSCROLL) - Mir3DpiScale(hWndDlg, 6);

	if (nState < Mir3DpiScale(hWndDlg, 90))
		nState = Mir3DpiScale(hWndDlg, 90);

	ListView_SetColumnWidth(hList, _STATUS_COL_TYPE,  nType);
	ListView_SetColumnWidth(hList, _STATUS_COL_NAME,  nName);
	ListView_SetColumnWidth(hList, _STATUS_COL_ADDR,  nAddr);
	ListView_SetColumnWidth(hList, _STATUS_COL_PORT,  nPort);
	ListView_SetColumnWidth(hList, _STATUS_COL_STATE, nState);
}

// **************************************************************************************

static void InitStatusColumns(HWND hWndDlg)
{
	HWND		hList = GetDlgItem(hWndDlg, IDC_STATUS_LIST);
	LV_COLUMN	lvc;
	TCHAR		szText[64];

	if (!hList)
		return;

	ZeroMemory(&lvc, sizeof(lvc));

	lvc.mask	= LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt		= LVCFMT_LEFT;
	lvc.cx		= Mir3DpiScale(hWndDlg, 90);
	lvc.pszText	= szText;

	for (int i = 0; i < _STATUS_COL_COUNT; i++)
	{
		lvc.iSubItem = i;

		LoadString(g_hInst, IDS_STATUS_COL1 + i, szText, sizeof(szText) / sizeof(TCHAR));

		ListView_InsertColumn(hList, i, &lvc);
	}

	AdjustStatusColumns(hWndDlg);
}

// **************************************************************************************

static int RefreshStatus(HWND hWndDlg)
{
	HWND	hList		= GetDlgItem(hWndDlg, IDC_STATUS_LIST);
	HWND	hSummary	= GetDlgItem(hWndDlg, IDC_STATUS_SUMMARY);

	TCHAR	szType[32];
	TCHAR	szName[64];
	TCHAR	szAddr[64];
	TCHAR	szPort[16];
	TCHAR	szState[64];
	TCHAR	szFmt[128];
	TCHAR	szSummary[256];

	int		nItem		= 0;
	int		nGates		= 0;
	int		nServers	= 0;

	if (!hList || !hSummary)
		return 0;

	SendMessage(hList, WM_SETREDRAW, (WPARAM)FALSE, 0L);
	ListView_DeleteAllItems(hList);

	// ---------------------------------------------------------------- 登录服务器自身

	lstrcpy(szType, _TEXT("Login Server"));
	lstrcpy(szName, _TEXT("Local listener"));
	lstrcpy(szAddr, _TEXT("0.0.0.0"));
	wsprintf(szPort, _TEXT("%d"), _STATUS_LISTEN_PORT);

	if (g_gcSock != INVALID_SOCKET)
		LoadString(g_hInst, IDS_STATUS_LISTENING, szState, sizeof(szState) / sizeof(TCHAR));
	else
		LoadString(g_hInst, IDS_STATUS_STOPPED, szState, sizeof(szState) / sizeof(TCHAR));

	nItem = InsertStatusRow(hList, nItem, szType, szName, szAddr, szPort, szState);

	// ---------------------------------------------------------------- 已连接的 Login Gate
	//
	// g_xGateList 由 IO 完成端口线程维护，这里只是快速扫一遍。
	// 万一刚好撞上节点释放，用 SEH 兜住，宁可少显示一行也不让界面崩掉。

	__try
	{
		PLISTNODE	pNode	= g_xGateList.GetHead();
		int			nGuard	= 0;

		while (pNode && nGuard++ < _STATUS_MAX_ROWS)
		{
			CGateInfo *pGate = g_xGateList.GetData(pNode);

			if (pGate)
			{
				SOCKADDR_IN	addr;
				int			nLen = sizeof(addr);

				ZeroMemory(&addr, sizeof(addr));

				if (getpeername(pGate->sock, (SOCKADDR *)&addr, &nLen) == 0)
					wsprintf(szAddr, _TEXT("%s"), inet_ntoa(addr.sin_addr));
				else
					lstrcpy(szAddr, _TEXT("-"));

				wsprintf(szPort, _TEXT("%d"), (int)ntohs(addr.sin_port));

				wsprintf(szType, _TEXT("Login Gate"));
				wsprintf(szName, _TEXT("Gate #%d"), ++nGates);
				wsprintf(szState, _TEXT("%d user(s)"), (int)pGate->xUserInfoList.GetCount());

				nItem = InsertStatusRow(hList, nItem, szType, szName, szAddr, szPort, szState);
			}

			pNode = g_xGateList.GetNext(pNode);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}

	// ---------------------------------------------------------------- 游戏服务器列表
	//
	// g_xGameServerList 由启动服务时的 LoadAccountRecords 线程从 TBL_SERVERINFO 填好，
	// 之后只读，这里直接拿来显示。

	__try
	{
		PLISTNODE	pNode	= g_xGameServerList.GetHead();
		int			nGuard	= 0;

		while (pNode && nGuard++ < _STATUS_MAX_ROWS)
		{
			GAMESERVERINFO *pInfo = g_xGameServerList.GetData(pNode);

			if (pInfo)
			{
				MultiByteToWideChar(CP_ACP, 0, pInfo->name, -1, szName, sizeof(szName) / sizeof(TCHAR));
				MultiByteToWideChar(CP_ACP, 0, pInfo->ip,   -1, szAddr, sizeof(szAddr) / sizeof(TCHAR));

				lstrcpy(szType, _TEXT("Game Server"));
				lstrcpy(szPort, _TEXT("-"));
				wsprintf(szState, _TEXT("#%d, %d selection(s)"), pInfo->index, pInfo->connCnt);

				nItem = InsertStatusRow(hList, nItem, szType, szName, szAddr, szPort, szState);

				nServers++;
			}

			pNode = g_xGameServerList.GetNext(pNode);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}

	SendMessage(hList, WM_SETREDRAW, (WPARAM)TRUE, 0L);
	InvalidateRect(hList, NULL, TRUE);

	// ---------------------------------------------------------------- 概要文字

	LoadString(g_hInst, IDS_STATUS_SUMMARY_FMT, szFmt, sizeof(szFmt) / sizeof(TCHAR));

	wsprintf(szSummary, szFmt, nGates, nServers);

	SetWindowText(hSummary, szSummary);

	return nItem;
}

// **************************************************************************************

BOOL CALLBACK ServerStatusDlgProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			CenterDialog(g_hMainWnd, hWndDlg);

			InitStatusColumns(hWndDlg);
			RefreshStatus(hWndDlg);

			return TRUE;
		}
		case WM_SIZE:
			AdjustStatusColumns(hWndDlg);
			break;
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_STATUS_REFRESH:
					RefreshStatus(hWndDlg);
					return TRUE;
				case IDOK:
				case IDCANCEL:
					EndDialog(hWndDlg, LOWORD(wParam));
					return TRUE;
			}

			break;
		}
	}

	return FALSE;
}

// **************************************************************************************

void ShowServerStatusDlg(HWND hParentWnd)
{
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_SERVERSTAT),
			  (hParentWnd ? hParentWnd : g_hMainWnd), (DLGPROC)ServerStatusDlgProc);
}
