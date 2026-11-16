// =============================================================================
//  Server/Def/ModernUI.cpp  —— 主窗口尺寸 / DPI / 列表列宽 辅助实现
// =============================================================================
#include "stdafx.h"
#include "ModernUI.h"

// GetDpiForWindow 只在 Windows 10 1607+ 才有导出，这里动态解析，
// 老系统（Win7/8/8.1）自动回退到 GetDeviceCaps。
typedef UINT (WINAPI *PFN_GETDPIFORWINDOW)(HWND);

static UINT Mir3QueryDpi(HWND hWnd)
{
	static PFN_GETDPIFORWINDOW s_pfnGetDpiForWindow = NULL;
	static BOOL                s_bResolved          = FALSE;

	if (!s_bResolved)
	{
		HMODULE hUser32 = GetModuleHandle(_T("user32.dll"));

		if (hUser32 != NULL)
			s_pfnGetDpiForWindow = (PFN_GETDPIFORWINDOW)GetProcAddress(hUser32, "GetDpiForWindow");

		s_bResolved = TRUE;
	}

	if (s_pfnGetDpiForWindow != NULL && hWnd != NULL)
	{
		UINT nDpi = s_pfnGetDpiForWindow(hWnd);

		if (nDpi != 0)
			return nDpi;
	}

	HDC  hDC  = GetDC(NULL);
	UINT nDpi = 96;

	if (hDC != NULL)
	{
		nDpi = (UINT)GetDeviceCaps(hDC, LOGPIXELSX);

		ReleaseDC(NULL, hDC);
	}

	return (nDpi != 0) ? nDpi : 96;
}

// **************************************************************************************

int Mir3DpiScale(HWND hWnd, int nLogical)
{
	if (nLogical <= 0)
		return 0;

	return MulDiv(nLogical, (int)Mir3QueryDpi(hWnd), 96);
}

// **************************************************************************************

void Mir3SizeMainWindow(HWND hWnd, int nLogicalWidth, int nLogicalHeight)
{
	if (hWnd == NULL || nLogicalWidth <= 0 || nLogicalHeight <= 0)
		return;

	int cx = Mir3DpiScale(hWnd, nLogicalWidth);
	int cy = Mir3DpiScale(hWnd, nLogicalHeight);

	HMONITOR    hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi;

	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);

	if (hMonitor != NULL && GetMonitorInfo(hMonitor, &mi))
	{
		int nWorkWidth  = mi.rcWork.right - mi.rcWork.left;
		int nWorkHeight = mi.rcWork.bottom - mi.rcWork.top;

		// 工作区里留出边距，避免贴边、被任务栏或屏幕边缘挡住
		if (cx > nWorkWidth  * 92 / 100) cx = nWorkWidth  * 92 / 100;
		if (cy > nWorkHeight * 88 / 100) cy = nWorkHeight * 88 / 100;

		int x = mi.rcWork.left + (nWorkWidth  - cx) / 2;
		int y = mi.rcWork.top  + (nWorkHeight - cy) / 2;

		SetWindowPos(hWnd, NULL, x, y, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else
	{
		SetWindowPos(hWnd, NULL, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

// **************************************************************************************

void Mir3FitLogColumns(HWND hList, HWND hWndForDpi)
{
	if (hList == NULL)
		return;

	// 日期 / 时间两列沿用原来的固定宽度（96 DPI 下都是 100，够放下 "9999/99/99" 和 "99:99:99"）
	int nDateWidth = Mir3DpiScale(hWndForDpi, 100);
	int nTimeWidth = Mir3DpiScale(hWndForDpi, 100);

	ListView_SetColumnWidth(hList, 0, nDateWidth);
	ListView_SetColumnWidth(hList, 1, nTimeWidth);

	RECT rcClient;

	GetClientRect(hList, &rcClient);

	// 剩余宽度全部给 Message 列；扣掉纵向滚动条与边框，避免冒出横向滚动条
	int nMinWidth = Mir3DpiScale(hWndForDpi, 240);
	int nMsgWidth = (rcClient.right - rcClient.left) - nDateWidth - nTimeWidth
					- GetSystemMetrics(SM_CXVSCROLL) - Mir3DpiScale(hWndForDpi, 6);

	if (nMsgWidth < nMinWidth)
		nMsgWidth = nMinWidth;

	ListView_SetColumnWidth(hList, 2, nMsgWidth);
}
