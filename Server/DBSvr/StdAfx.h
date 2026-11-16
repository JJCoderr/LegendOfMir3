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


#include <winsock2.h>
#include <windows.h>
#include <tchar.h>
#ifdef _DEBUG
#include <crtdbg.h>
#endif

// ORZ:
#pragma warning( disable : 4146 )
#pragma warning(disable:4786)

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
#include "Def/TableColumn.h"
#include "Def/Queue.h"
#include "Def/List.h"

#include "DBSvr.h"

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "comctl32.lib")

// **************************************************************************************

#define _DB_SERVER_CLASS				_TEXT("DatabaseServerClass")
#define _DB_SERVER_TITLE				_TEXT("Database Server - Legend of Mir II")
#define _DB_SERVER_REGISTRY				_TEXT("Software\\LegendOfMir\\DatabaseSvr")
#define _DB_SERVERLIST_REGISTRY			_TEXT("Software\\LegendOfMir\\DatabaseSvr\\Servers")

#define _STATUS_HEIGHT					22
#define _NUMOFMAX_STATUS_PARTS			4

// 主窗口逻辑尺寸（96 DPI 像素；实际显示按显示器 DPI 缩放，并限制在工作区内）
#define _MAINWND_WIDTH					940
#define _MAINWND_HEIGHT					580
										    
#define _IDM_GATECOMMSOCK_MSG			WM_USER + 1000

#define _WM_LOGMSG						WM_USER + 1004	// 工作线程投递给主线程的日志消息
#define _WM_SESSIONCOUNT				WM_USER + 1005	// 工作线程投递的连接数

#define _IDW_TOOLBAR					_IDM_GATECOMMSOCK_MSG + 2
#define _IDW_STATUSBAR					_IDM_GATECOMMSOCK_MSG + 3

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
