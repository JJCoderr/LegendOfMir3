// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__D4BEEACE_E0B0_4C9B_BE8D_FF8618EF253F__INCLUDED_)
#define AFX_STDAFX_H__D4BEEACE_E0B0_4C9B_BE8D_FF8618EF253F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define UNICODE
#define _UNICODE

#include <winsock2.h>
#include <windows.h>
#include <tchar.h>
#ifdef _DEBUG
#include <crtdbg.h>
#endif
#include <commctrl.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include "resource.h"

// 现代外观：主窗口尺寸/DPI/列表列宽辅助（Server/Def/ModernUI.cpp）
#include "Def/ModernUI.h"

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "comctl32.lib")

// **************************************************************************************

#define _MANCONSOLE_TITLE			_TEXT("Legend Of Mir II - Administrator Management Console")

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__D4BEEACE_E0B0_4C9B_BE8D_FF8618EF253F__INCLUDED_)
