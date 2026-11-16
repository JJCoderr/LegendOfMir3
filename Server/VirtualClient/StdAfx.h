// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <tchar.h>
#include <winsock2.h>
#include <windows.h>
#include <commctrl.h>
#include <time.h>
#include <crtdbg.h>

#include "Def/Misc.h"
#include "Def/EnDecode.h"
#include "Def/Protocol.h"

// 现代外观：主窗口尺寸/DPI 辅助（Server/Def/ModernUI.cpp）
#include "Def/ModernUI.h"

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "comctl32.lib")

#define _IDM_SOCKMSG			WM_USER + 1000

// 主窗口逻辑尺寸（96 DPI 像素；实际显示按显示器 DPI 缩放，并限制在工作区内）
#define _MAINWND_WIDTH					760
#define _MAINWND_HEIGHT					480

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
