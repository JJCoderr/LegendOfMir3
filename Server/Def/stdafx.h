// =============================================================================
//  Server/Def/stdafx.h  —— Def 静态库私有预编译头
//
//  历史上 Def/*.cpp 是直接以源码形式挂进每个工程的，因此 #include "stdafx.h"
//  实际解析到的是【各工程自己的】StdAfx.h（顺带拿到了各工程的 resource.h、
//  UNICODE 设置等）。Def 收敛为静态库后只编译一次，必须自备一份，并且不能
//  依赖任何单个工程的 StdAfx.h / resource.h，否则库就没法共用。
//
//  注意：Def/Protocol.h、Def/EnDecode.h、Def/dbmgr.h 都含 TCHAR 成员/参数，
//  TCHAR 宽度变了 ABI 就变了（协议结构体大小都会变）。所以本库固定按 UNICODE
//  /_UNICODE 编译（见 Def/CMakeLists.txt），与各 Server 工程 StdAfx.h 中
//  #define UNICODE / _UNICODE 的设置保持一致，P/Invoke 语义不允许各工程各自为政。
// =============================================================================
#ifndef _MIR3_DEF_STDAFX_H
#define _MIR3_DEF_STDAFX_H

#pragma once

// winsock2.h 必须先于 windows.h
#include <winsock2.h>
#include <windows.h>
#include <tchar.h>
#include <commctrl.h>

#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 原先各工程的 StdAfx.h 顺带包含了这些 Def 头文件，库内源码（EnDecode.cpp、
// ServerSockHandler.cpp、dbmgr.cpp 等）依赖其中的 _LPTDEFAULTMESSAGE、TCHAR
// 协议结构体、memlen 等声明。改为库自己包含，源码里的 #include "stdafx.h" 不变。
#include "EnDecode.h"
#include "Misc.h"
#include "Protocol.h"
#include "ServerSockHandler.h"

// -----------------------------------------------------------------------------
//  宿主工程（exe）必须提供的日志出口。
//
//  各 Server 工程都在自己的 *.h 里实现了同名函数（DBSvr.h / GameSvr.h /
//  GameGate.h / LoginGate.h / LoginSvr.h / SelGate.h）：
//      void InsertLogMsg(UINT nID);       // 从 exe 字符串表取文本
//      void InsertLogMsg(LPTSTR lpszMsg);
//  这里声明 Def 需要的最小契约，链接期由宿主解析。目前只有
//  ServerSockHandler.cpp 用到（连接失败提示）。
// -----------------------------------------------------------------------------
void InsertLogMsg(LPTSTR lpszMsg);

#endif // _MIR3_DEF_STDAFX_H
