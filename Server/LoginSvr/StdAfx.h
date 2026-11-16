// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define UNICODE
#define _UNICODE
//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#pragma warning( disable : 4146 )

#include <winsock2.h>
#include <windows.h>
#include <tchar.h>
#include <crtdbg.h>
#include <commctrl.h>
#include <time.h>
#include <process.h>


#include "resource.h"

// 现代外观：主窗口尺寸/DPI 辅助（Server/Def/ModernUI.cpp）
#include "Def/ModernUI.h"

#include "Def/EnDecode.h"
#include "Def/ServerSockHandler.h"
#include "Def/Misc.h"
#include "Def/Protocol.h"
#include "Def/Queue.h"
#include "Def/List.h"

#include "LoginSvr.h"

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "comctl32.lib")

// **************************************************************************************

#define _LOGIN_SERVER_CLASS				_TEXT("LoginServerClass")
#define _LOGIN_SERVER_TITLE				_TEXT("Login Server - Legend of Mir II")
#define _LOGIN_SERVER_REGISTRY			_TEXT("Software\\LegendOfMir\\LoginSvr")
#define _LOGIN_SERVERLIST_REGISTRY		_TEXT("Software\\LegendOfMir\\LoginSvr\\Servers")

#define _STATUS_HEIGHT					22
#define _NUMOFMAX_STATUS_PARTS			7

// 主窗口逻辑尺寸（96 DPI 像素；实际显示按显示器 DPI 缩放，并限制在工作区内）
#define _MAINWND_WIDTH					940
#define _MAINWND_HEIGHT					600
										    
#define _IDM_GATECOMMSOCK_MSG			WM_USER + 1000
#define _IDM_CLIENTSOCK_MSG				_IDM_GATECOMMSOCK_MSG + 1

#define _IDW_TOOLBAR					_IDM_GATECOMMSOCK_MSG + 2
#define _IDW_STATUSBAR					_IDM_GATECOMMSOCK_MSG + 3

#define BTNS_SEP TBSTYLE_SEP // 0x0001   lost it

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
