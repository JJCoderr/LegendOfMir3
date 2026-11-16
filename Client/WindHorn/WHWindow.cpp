/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/



#include "stdafx.h"
#include "WHEngine.h"

static CWHApp* g_pWHApp = NULL;

/******************************************************************************************************************

	CWHApp Functions.

*******************************************************************************************************************/

/******************************************************************************************************************

	函数名 : CWHApp::CWHApp()

	作者 : 
	编写日期 : 

	目的   : 
	输出   : 

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
CWHApp::CWHApp()
{
	g_pWHApp	= this;
	m_hInstance = NULL;
}

/******************************************************************************************************************

	函数名 : CWHApp::~CWHApp()

	作者 :
	编写日期 :

	目的   :
	输出   :

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
CWHApp::~CWHApp()
{
}

/******************************************************************************************************************

	函数名 : CWHApp::MainWndProc()

	作者 :
	日期 :

	目的   :
	输入   : HWND hWnd
	         UINT uMsg
	         WPARAM wParam
	         LPARAM lParam
	输出   : LRESULT

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
LRESULT CWHApp::MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/******************************************************************************************************************

	函数名 : WndProc()

	作者 :
	编写日期 :

	目的   :
	输入   : HWND hWnd
	         UINT uMsg
	         WPARAM wParam
	         LPARAM lParam
	输出   : LRESULT CALLBACK

	[日期][修改人] : 修改内容

*******************************************************************************************************************/
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return g_pWHApp->MainWndProc(hWnd, uMsg, wParam, lParam);
}










/******************************************************************************************************************

	CWHWindow Functions.

*******************************************************************************************************************/

/******************************************************************************************************************

	函数名 : CWHWindow::CWHWindow()

	编写者 :
	编写日期 :

	目的   :
	输出   :

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
CWHWindow::CWHWindow()
{
	m_hWnd = NULL;
}

/******************************************************************************************************************

	函数名 : CWHWindow::~CWHWindow()

	作者 :
	编写日期 :

	目的   :
	输出   :

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
CWHWindow::~CWHWindow()
{
}

/******************************************************************************************************************

	函数名 : CWHWindow::RegisterClass()

	作者 :
	编写日期 :

	目的   :
	输入   : CHAR *pszMenuName
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CWHWindow::RegisterClass(CHAR* pszMenuName, CHAR* pszIconName)
{
    WNDCLASSEX wc;

    wc.cbSize			= sizeof(wc);
    wc.lpszClassName	= _WINDHORN_CLASS;
    wc.lpfnWndProc		= WndProc;
    wc.style			= CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
    wc.hInstance		= m_hInstance;
    wc.hIcon			= LoadIcon(m_hInstance, pszIconName);
    wc.hIconSm			= LoadIcon(m_hInstance, pszIconName);
    wc.hCursor			= LoadCursor(m_hInstance, IDC_ARROW);
    wc.hbrBackground	= (HBRUSH)(GRAY_BRUSH);//(COLOR_WINDOW + 1);
    wc.lpszMenuName		= pszMenuName;
    wc.cbClsExtra		= 0;
    wc.cbWndExtra		= 0;

    if ( RegisterClassEx(&wc) == 0 )
	{
        return FALSE;
	}

	return TRUE;
}

/******************************************************************************************************************

	函数名 : CWHWindow::Create()

	作者 :
	编写日期 :

	目的   :
	输入   : LPTSTR lpCaption
	         CHAR *pszMenuName
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CWHWindow::Create(HINSTANCE hInst, LPTSTR lpCaption, CHAR *pszMenuName, CHAR* pszIconName)
{
	SethInstance(hInst);
	if ( !RegisterClass(pszMenuName, pszIconName) ) 
	{
		MessageBox(NULL, TEXT("[CWHWindow::Create]") TEXT("Register class failed."), "MirDXG", MB_ICONERROR | MB_OK);
		return FALSE;
	}

    DWORD	dwFrameWidth    = GetSystemMetrics(SM_CXSIZEFRAME);
    DWORD	dwFrameHeight   = GetSystemMetrics(SM_CYSIZEFRAME);
	DWORD	dwMenuHeight;
	
	if ( pszMenuName != NULL )		
		dwMenuHeight = GetSystemMetrics(SM_CYMENU);
	else
		dwMenuHeight = 0;

    DWORD dwCaptionHeight = GetSystemMetrics(SM_CYCAPTION);
    DWORD dwWindowWidth   = _SCREEN_WIDTH  + dwFrameWidth * 2;
    DWORD dwWindowHeight  = _SCREEN_HEIGHT + dwFrameHeight * 2 + dwMenuHeight + dwCaptionHeight;

    DWORD dwStyle = WS_POPUPWINDOW/*WS_OVERLAPPEDWINDOW*/ & ~WS_MAXIMIZEBOX;

    m_hWnd = CreateWindowEx(NULL, _WINDHORN_CLASS, lpCaption,
						    dwStyle, CW_USEDEFAULT, CW_USEDEFAULT,
  						    dwWindowWidth, dwWindowHeight, NULL, NULL, m_hInstance, NULL);

	if ( m_hWnd == NULL )
    	return FALSE;

//  ShowWindow(m_hWnd, SW_SHOW);
//  UpdateWindow(m_hWnd);

	return TRUE;
}

/******************************************************************************************************************

	函数名 : CWHWindow::MainWndProc()

	作者 :
	编写日期 :

	目的   :
	输入   : HWND hWnd
	         UINT uMsg
	         WPARAM wParam
	         LPARAM lParam
	输出   : LRESULT

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
LRESULT CWHWindow::MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch (uMsg)
	{
		case WM_DESTROY:
			return OnDestroy();
	}

    return CWHApp::MainWndProc(hWnd, uMsg, wParam, lParam);
}

/******************************************************************************************************************

	函数名 : CWHWindow::OnDestroy()

	作者 :
	编写日期 :

	目的   :
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
LRESULT CWHWindow::OnDestroy()
{
	PostQuitMessage(NULL);
	return 0L;
}
