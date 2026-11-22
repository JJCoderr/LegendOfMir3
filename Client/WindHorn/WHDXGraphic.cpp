/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/


#include "stdafx.h"
#include "WHEngine.h"


/******************************************************************************************************************

	CWHDXGraphicWindow Functions

*******************************************************************************************************************/


static BYTE						g_bNumDevices = 0;
static DXG_ENUM_DEVICEINFO		g_stDXGEnumDeviceInfo[_MAX_DEVICES];

static INT						DXGModesSortCallback(const VOID* arg1, const VOID* arg2);
static BOOL		WINAPI			DXGDriverEnumCallbackEx(GUID* pGUID, LPSTR szDesc, LPSTR szName, LPVOID pContext, HMONITOR hm);
static HRESULT	WINAPI			DXGEnumDisplayModeCallback(DDSURFACEDESC2* pddsd, VOID* pParentInfo);
static HRESULT	WINAPI			DXG3DDeviceEnumCallback(TCHAR* szDesc, TCHAR* szName, D3DDEVICEDESC7* pDesc, VOID* pParentInfo);
static HRESULT	WINAPI			DXGEnumZBufferFormatsCallback(DDPIXELFORMAT* pddpf, VOID* pContext);



CWHDXGraphicWindow* CWHDXGraphicWindow::m_pxDXGWnd = NULL;

CWHDXGraphicWindow* GetDXGWindowPtr()
{
	return CWHDXGraphicWindow::m_pxDXGWnd;
}


CWHDXGraphicWindow::CWHDXGraphicWindow(WORD wWidth, WORD wHeight, WORD wBpp)
{
	m_pDD				= NULL;
	m_pD3D				= NULL;
	m_hDefGameFont		= NULL;
	m_pd3dDevice		= NULL;
	m_pddsZBuffer		= NULL;
    m_pddsFrontBuffer	= NULL;
    m_pddsBackBuffer	= NULL;
//	为了编辑窗口的输出而设置裁剪器。
	m_lpcClipper		= NULL;

	m_stDisplayInfo.wWidth	= wWidth;
	m_stDisplayInfo.wHeight	= wHeight;
	m_stDisplayInfo.wBPP	= wBpp;
	m_bScreenModeFlag		= _DXG_SCREENMODE_WINDOW;
	m_bDeviceModeFlag		= _DXG_DEVICEMODE_PRIMARY;

	SetRect(&m_rcWindow, 0, 0, 0, 0);
	ZeroMemory(&m_stBitsMaskInfo, sizeof(DXG_MASKINFO));

	m_pxDXGWnd = this;

	m_bIsWindowActive = FALSE;
	m_bIsWindowReady = FALSE;

	m_dwTextureTotal	= 0;
	m_dwTextureFree		= 0;
	m_dwVideoTotal		= 0;
	m_dwVideoFree		= 0;

	m_pxDefProcess = NULL;
}


VOID CWHDXGraphicWindow::UsedAndFreeMemoryCheck()
{
	DDSCAPS2     ddsCaps2; 

	ZeroMemory(&ddsCaps2, sizeof(ddsCaps2));

	ddsCaps2.dwCaps = DDSCAPS_TEXTURE;
	m_pDD->GetAvailableVidMem(&ddsCaps2, &m_dwTextureTotal, &m_dwTextureFree); 

	ZeroMemory(&ddsCaps2, sizeof(ddsCaps2));

	ddsCaps2.dwCaps = DDSCAPS_VIDEOMEMORY;
	m_pDD->GetAvailableVidMem(&ddsCaps2, &m_dwVideoTotal, &m_dwVideoFree); 
} 


CWHDXGraphicWindow::~CWHDXGraphicWindow()
{
	FreeDXGEnumModeResources();
    DestroyDXGObjects();

	m_pxDefProcess = NULL;
	m_pxDXGWnd = NULL;
}


BOOL CWHDXGraphicWindow::Create(HINSTANCE hInst, LPTSTR lpCaption, CHAR *pszMenuName, CHAR* pszIconName, BYTE bScreenModeFlag, BYTE bDeviceModeFlag)
{
	DWORD	dwStyle;

	if ( !CWHWindow::Create(hInst, lpCaption, pszMenuName, pszIconName) )
	{
		MessageBox(m_hWnd, TEXT("[CWHDXGraphicWindow::Create]") TEXT("Window create failed."), "MirDXG", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	m_bScreenModeFlag		= bScreenModeFlag;
	m_bDeviceModeFlag		= bDeviceModeFlag;

	if ( m_bScreenModeFlag & _DXG_SCREENMODE_WINDOW )
	{
		dwStyle  = GetWindowStyle(m_hWnd);
		dwStyle &= ~WS_POPUP;
		dwStyle |= WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME;
		SetWindowLong(m_hWnd, GWL_STYLE, dwStyle);
	}
	else
	{
		dwStyle  = GetWindowStyle(m_hWnd);
		dwStyle &= ~dwStyle;
		dwStyle |= WS_POPUP;
		SetWindowLong(m_hWnd, GWL_STYLE, dwStyle);
	}

	HRESULT	hr;
	if ( hr = SUCCEEDED(DirectDrawEnumerateEx(DXGDriverEnumCallbackEx, NULL,
											  DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_DETACHEDSECONDARYDEVICES | DDENUM_NONDISPLAYDEVICES)) )
	{
		if ( hr = SUCCEEDED(CreateDXG()) )
		{
			m_bIsWindowReady = TRUE;
			CreateDefFont();
			return TRUE;
		}
		else
		{
			MessageBox(m_hWnd, TEXT("[CWHDXGraphicWindow::Create]") TEXT("DirectGraphic create failed."), "MirDXG", MB_ICONERROR | MB_OK);
			return FALSE;
		}
	}

	return FALSE;
}


namespace
{
    constexpr int WindowAspectWidth = 4;
    constexpr int WindowAspectHeight = 3;
    constexpr int MinimumClientWidth = 1024;
    constexpr int MinimumWindowScale = MinimumClientWidth / WindowAspectWidth;
    constexpr int MinimumClientHeight = MinimumWindowScale * WindowAspectHeight;

    RECT WindowFrame(HWND window)
    {
        RECT frame = {};
        AdjustWindowRectEx(&frame, GetWindowStyle(window), GetMenu(window) != NULL, GetWindowExStyle(window));
        return frame;
    }
}

LRESULT CWHDXGraphicWindow::MainWndProcDXG(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch (uMsg)
	{
		case WM_SYSKEYDOWN:
			return OnSysKeyDown(wParam, lParam);

		case WM_MOVE:
			return OnMove(wParam, lParam);

		case WM_SIZE:
			return OnSize(wParam, lParam);

		case WM_DESTROY:
			return OnDestroy();

        case WM_SETCURSOR:
			return OnSetCursor();   

        case WM_GETMINMAXINFO:
            if (GetWindowStyle(hWnd) & WS_THICKFRAME)
            {
                const RECT frame = WindowFrame(hWnd);
                auto& limits = *reinterpret_cast<MINMAXINFO*>(lParam);
                limits.ptMinTrackSize.x = MinimumClientWidth + frame.right - frame.left;
                limits.ptMinTrackSize.y = MinimumClientHeight + frame.bottom - frame.top;
                return 0;
            }
            break;

        case WM_ENTERSIZEMOVE:
            m_resizeStartWidth = m_rcWindow.right - m_rcWindow.left;
            break;

        case WM_SIZING:
            ConstrainWindowSize(hWnd, wParam, *reinterpret_cast<RECT*>(lParam));
            return TRUE;

        case WM_EXITSIZEMOVE:
            if (m_resizeStartWidth != m_rcWindow.right - m_rcWindow.left)
                m_userWindowScale = (m_rcWindow.right - m_rcWindow.left) / WindowAspectWidth;
            m_resizeStartWidth = 0;
            break;

        case WM_ERASEBKGND:
            if (m_resizeStartWidth)
                return TRUE;
            break;

        case WM_PAINT:
            if (m_resizeStartWidth)
            {
                PAINTSTRUCT paint;
                BeginPaint(hWnd, &paint);
                Present();
                EndPaint(hWnd, &paint);
                return 0;
            }
            break;

		default:
			break;
	}

    return CWHWindow::MainWndProc(hWnd, uMsg, wParam, lParam);
}


void CWHDXGraphicWindow::ConstrainWindowSize(HWND window, WPARAM edge, RECT& bounds)
{
    const RECT frame = WindowFrame(window);
    const int frameWidth = frame.right - frame.left;
    const int frameHeight = frame.bottom - frame.top;
    const int width = bounds.right - bounds.left - frameWidth;
    const int height = bounds.bottom - bounds.top - frameHeight;

    int scale;
    if (edge == WMSZ_LEFT || edge == WMSZ_RIGHT)
        scale = MulDiv(width, 1, WindowAspectWidth);
    else if (edge == WMSZ_TOP || edge == WMSZ_BOTTOM)
        scale = MulDiv(height, 1, WindowAspectHeight);
    else
    {
        // For corners, use both mouse axes by projecting onto the aspect ratio.
        constexpr int aspectLengthSquared = WindowAspectWidth * WindowAspectWidth +
                                            WindowAspectHeight * WindowAspectHeight;
        scale = MulDiv(width * WindowAspectWidth + height * WindowAspectHeight, 1, aspectLengthSquared);
    }
    scale = max(scale, MinimumWindowScale);
    const int outerWidth = scale * WindowAspectWidth + frameWidth;
    const int outerHeight = scale * WindowAspectHeight + frameHeight;

    if (edge == WMSZ_LEFT || edge == WMSZ_TOPLEFT || edge == WMSZ_BOTTOMLEFT)
        bounds.left = bounds.right - outerWidth;
    else
        bounds.right = bounds.left + outerWidth;

    if (edge == WMSZ_TOP || edge == WMSZ_TOPLEFT || edge == WMSZ_TOPRIGHT)
        bounds.top = bounds.bottom - outerHeight;
    else
        bounds.bottom = bounds.top + outerHeight;
}


LRESULT CWHDXGraphicWindow::OnSetCursor()
{
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    return 0L;
}



LRESULT CWHDXGraphicWindow::OnSize(WPARAM wParam, LPARAM lParam)
{
    if ( SIZE_MAXHIDE == wParam || SIZE_MINIMIZED == wParam )
        m_bIsWindowActive = FALSE;
    else
        m_bIsWindowActive = TRUE;

	UpdateBoundsWnd();
 	return 0L;
}



LRESULT CWHDXGraphicWindow::OnMove(WPARAM wParam, LPARAM lParam)
{
	UpdateBoundsWnd();
	return 0L;
}


LRESULT CWHDXGraphicWindow::OnDestroy()
{
	return CWHWindow::OnDestroy();
}


LRESULT CWHDXGraphicWindow::OnSysKeyDown(WPARAM wParam, LPARAM lParam)
{
	if ( wParam == VK_RETURN )
	{
		m_bIsWindowReady = FALSE;

 		D3DTextr_InvalidateAllTextures();

		D3DWILTextr_InvalidateAllTextures();
		D3DWILTextr_DestroyAllTextures();

		if ( m_bScreenModeFlag & _DXG_SCREENMODE_WINDOW )
		{
			if ( FAILED(ResetDXG(_SCREEN_WIDTH, _SCREEN_HEIGHT, _SCREEN_BPP, _DXG_SCREENMODE_FULLSCREEN, _DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D)) )
			{
				MessageBox(m_hWnd, TEXT("DirectDraw RESETUP failed!!!") TEXT("The Program will now exit."), TEXT("MirDXG"), MB_ICONERROR | MB_OK);
			}
		}
		else
		{
			if ( FAILED(ResetDXG(_SCREEN_WIDTH, _SCREEN_HEIGHT, _SCREEN_BPP, _DXG_SCREENMODE_WINDOW, _DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D)) )
			{
				MessageBox(m_hWnd, TEXT("DirectDraw RESETUP failed!!!") TEXT("The Program will now exit."), TEXT("MirDXG"), MB_ICONERROR | MB_OK);
			}
		}

		D3DTextr_RestoreAllTextures(m_pd3dDevice);

		m_bIsWindowReady = TRUE;
	}

	return 0L;
}


BYTE CWHDXGraphicWindow::FindDriverAndDevice()
{
	BYTE bNum = 0;
	CHAR bCnt;
	 
	if ( m_bDeviceModeFlag & _DXG_DEVICEMODE_PRIMARY )
	{
		for ( bCnt = 0; bCnt < g_bNumDevices; bCnt++ )
		{
			if ( !g_stDXGEnumDeviceInfo[bCnt].pDriverGUID )
			{
				if ( g_stDXGEnumDeviceInfo[bCnt].f3DHardware )			return bCnt;
				else													bNum = bCnt;
			}
		}
	}
	else
	{
		for ( bCnt = g_bNumDevices-1; bCnt >= 0; bCnt-- )
		{
			// 视频卡有 2 个以上
			if( g_bNumDevices > 2 )
			{
				if ( g_stDXGEnumDeviceInfo[0].dwNumModes != g_stDXGEnumDeviceInfo[bCnt].dwNumModes )
				{
					if ( g_stDXGEnumDeviceInfo[bCnt].f3DHardware )		return bCnt;
					else											
					{
						bNum = bCnt;
						break;
					}
				}
			}
			else
			{
				if ( g_stDXGEnumDeviceInfo[bCnt].f3DHardware )			return bCnt;
				else													bNum = bCnt;
			}
		}
	}

	return bNum;
}


HRESULT CWHDXGraphicWindow::DestroyDXGObjects()
{
	HRESULT	hr;
	LONG	lDDRefCnt  = 0L;
    LONG	lD3DRefCnt = 0L;

	if ( !(m_bScreenModeFlag & _DXG_SCREENMODE_WINDOW) )
	{
		if ( FAILED(hr = m_pDD->RestoreDisplayMode()) )
			return E_FAIL;
	}
    if ( m_pDD )
	{
		if ( FAILED(hr = m_pDD->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL)) )
			return E_FAIL;
	}

    if ( m_pd3dDevice )
	{
        if ( 0 < ( lD3DRefCnt = m_pd3dDevice->Release() ) )
			return E_FAIL;
	}
    m_pd3dDevice = NULL;

    SAFE_RELEASE(m_pddsBackBuffer);
    SAFE_RELEASE(m_pddsFrontBuffer);
    SAFE_RELEASE(m_pddsZBuffer);
//	为了编辑窗口的输出而设置裁剪器。
	SAFE_RELEASE(m_lpcClipper);
    SAFE_RELEASE(m_pD3D);

    if ( m_pDD )
    {
        if ( 0 < ( lDDRefCnt = m_pDD->Release() ) )
			INT nAAAAAA = 0;
//			return E_FAIL;
    }
    m_pDD = NULL;

	if ( m_hDefGameFont != NULL )
	{
		DeleteObject(m_hDefGameFont);
		m_hDefGameFont = NULL; 
	}

	return S_OK;
//    return ( lDDRefCnt==0 && lD3DRefCnt==0 ) ? S_OK : E_FAIL;
}


VOID CWHDXGraphicWindow::FreeDXGEnumModeResources()
{	
    for ( BYTE bCnt = 0; bCnt < g_bNumDevices; bCnt++ )
    {
        SAFE_DELETE( g_stDXGEnumDeviceInfo[bCnt].pddsdModes );
    }
}


HRESULT CWHDXGraphicWindow::CreateDXG() 
{
    HRESULT hr;

	BYTE bSelecedDevice = FindDriverAndDevice();

	if ( SUCCEEDED(hr = DirectDrawCreateEx(g_stDXGEnumDeviceInfo[bSelecedDevice].pDriverGUID, (VOID**)&m_pDD, IID_IDirectDraw7, NULL)) ) 
	{
		if ( m_bScreenModeFlag & _DXG_SCREENMODE_WINDOW )
		{
			if ( FAILED(hr = m_pDD->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL)) )
				return E_FAIL;
		}
		else
		{
			if ( FAILED(hr = m_pDD->SetCooperativeLevel(m_hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT)) )
				return E_FAIL;

			if ( FAILED(hr = m_pDD->SetDisplayMode(m_stDisplayInfo.wWidth, m_stDisplayInfo.wHeight, m_stDisplayInfo.wBPP, 0, 0)) )
				return E_FAIL;		
		}

		if ( FAILED(hr = CreatePrimarySurface()) )
		{
			MessageBox(m_hWnd, TEXT("[CWHDXGraphicWindow::CreateDXG]") TEXT("Primary surface create failed."), TEXT("MirDXG"), MB_ICONERROR | MB_OK);
			return E_FAIL;
		}

		UpdateBoundsWnd();

		if ( m_bDeviceModeFlag & _DXG_DEVICEMODE_D3D )
		{
			if ( FAILED( hr = m_pDD->QueryInterface(IID_IDirect3D7, (VOID**)&m_pD3D)) )		
				return E_FAIL;

			if ( FAILED(hr = m_pD3D->CreateDevice(*g_stDXGEnumDeviceInfo[bSelecedDevice].pDeviceGUID, m_pddsBackBuffer, &m_pd3dDevice)) )
				return E_FAIL;

			D3DVIEWPORT7 d3dVP = { 0, 0, m_stDisplayInfo.wWidth, m_stDisplayInfo.wHeight, 0.0f, 1.0f };

			if ( FAILED( hr = m_pd3dDevice->SetViewport(&d3dVP)) )							return hr;

			if ( m_bDeviceModeFlag & _DXG_DEVICEMODE_ZBUFFER )
			{
				if ( FAILED(hr = CreateZBuffer(g_stDXGEnumDeviceInfo[bSelecedDevice].pDeviceGUID)) )
				{
					MessageBox(m_hWnd, TEXT("[CWHDXGraphicWindow::CreateDXG]") TEXT("ZBuffer create failed."), TEXT("MirDXG"), MB_ICONERROR | MB_OK);
					return E_FAIL;
				}
			}
		}

		m_stBitsMaskInfo = GetRGBMaskInfoIN16Bits(m_pddsBackBuffer);
		Init3DDeviceObjects();

		return S_OK;
	}

	return hr;
}


HRESULT CWHDXGraphicWindow::ResetDXG(WORD wWidth, WORD wHeight, WORD wBPP, BYTE bScreenModeFlag, BYTE bDeviceModeFlag)
{
    HRESULT hr;
    DWORD	dwStyle;

	if ( m_bDeviceModeFlag == bDeviceModeFlag && m_bScreenModeFlag == bScreenModeFlag && 
		 m_stDisplayInfo.wWidth == wWidth && m_stDisplayInfo.wHeight == wHeight && m_stDisplayInfo.wBPP == wBPP )
		return E_FAIL;													    

    if ( FAILED(DestroyDXGObjects()) )
	{
		MessageBox(m_hWnd, TEXT("[CWHDXGraphicWindow::ResetDXG]") TEXT("DirectGraphic reset failed."), "MirDXG", MB_ICONERROR | MB_OK);
		return E_FAIL;
	}

	m_stDisplayInfo.wWidth	= wWidth;
	m_stDisplayInfo.wHeight	= wHeight;
	m_stDisplayInfo.wBPP	= wBPP;
	m_bScreenModeFlag		= bScreenModeFlag;
	m_bDeviceModeFlag		= bDeviceModeFlag;

	if ( m_bScreenModeFlag & _DXG_SCREENMODE_WINDOW )
	{
		dwStyle  = GetWindowStyle(m_hWnd);
		dwStyle &= ~WS_POPUP;
		dwStyle |= WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME;
		SetWindowLong(m_hWnd, GWL_STYLE, dwStyle);
	}
	else
	{
		dwStyle  = GetWindowStyle(m_hWnd);
		dwStyle &= ~dwStyle;
		dwStyle |= WS_POPUP;
		SetWindowLong(m_hWnd, GWL_STYLE, dwStyle);
	}

	if ( hr = FAILED(CreateDXG()) )
	{
		MessageBox(m_hWnd, TEXT("[CWHDXGraphicWindow::ResetDXG]") TEXT("DirectGraphic create failed."), "MirDXG", MB_ICONERROR | MB_OK);
		return E_FAIL;
	}

	CreateDefFont();

	return S_OK;
}


HRESULT CWHDXGraphicWindow::CreatePrimarySurface()
{		
    DDSURFACEDESC2	ddsd;
    HRESULT			hr;

	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	
	if ( m_bScreenModeFlag & _DXG_SCREENMODE_WINDOW )
	{
		RECT				rcWork;
		RECT				rcTemp;
		
		// 设置客户端区域大小的部分与窗口的创建有关.
		// 获取屏幕分辨率即客户端矩形区域的大小。
		// Use the same DPI-scaled window baseline for every scene.
		HDC windowDC = GetDC(m_hWnd);
		const int dpi = GetDeviceCaps(windowDC, LOGPIXELSX);
		ReleaseDC(m_hWnd, windowDC);
		const int defaultScale = MulDiv(MinimumWindowScale, dpi, USER_DEFAULT_SCREEN_DPI);
		const int defaultWidth = defaultScale * WindowAspectWidth;
		const int defaultHeight = defaultScale * WindowAspectHeight;
		// Explicit Display overrides remain physical pixel sizes.
		const int windowWidth = GetPrivateProfileInt("Display", "WindowWidth", defaultWidth, ".\\Mir3.ini");
		const int windowHeight = GetPrivateProfileInt("Display", "WindowHeight", defaultHeight, ".\\Mir3.ini");
        const int requestedScale = max(
            windowWidth / WindowAspectWidth + (windowWidth % WindowAspectWidth > 0),
            windowHeight / WindowAspectHeight + (windowHeight % WindowAspectHeight > 0));
        const int scale = max(MinimumWindowScale, m_userWindowScale ? m_userWindowScale : requestedScale);
        SetRect(&rcTemp, 0, 0, scale * WindowAspectWidth, scale * WindowAspectHeight);

		// 从客户端矩形区域大小转换为整个窗口区域的大小.
        const RECT frame = WindowFrame(m_hWnd);
        rcTemp.right += frame.right - frame.left;
        rcTemp.bottom += frame.bottom - frame.top;

		// 用窗口的整个区域大小设置窗口。但起始坐标被忽略。
		SetWindowPos(m_hWnd, NULL, 0, 0, rcTemp.right-rcTemp.left, rcTemp.bottom-rcTemp.top,
					 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

		// 是为了设置 HWND_NOTOPMOST。
		SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);

		// 获取桌面区域的大小..
		MONITORINFO monitor = {};
		monitor.cbSize = sizeof(monitor);
		GetMonitorInfo(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST), &monitor);
		rcWork = monitor.rcWork;
		GetWindowRect(m_hWnd, &rcTemp);
		const LONG windowWidthPixels = rcTemp.right - rcTemp.left;
		const LONG windowHeightPixels = rcTemp.bottom - rcTemp.top;
		rcTemp.left = rcWork.left + max(0L, (rcWork.right - rcWork.left - windowWidthPixels) / 2);
		rcTemp.top = rcWork.top + max(0L, (rcWork.bottom - rcWork.top - windowHeightPixels) / 2);

		// 设置窗口的起始坐标.
		SetWindowPos(m_hWnd, NULL, rcTemp.left, rcTemp.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    
		ddsd.dwFlags        = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

		if ( FAILED(hr = m_pDD->CreateSurface(&ddsd, &m_pddsFrontBuffer, NULL)) )			return E_FAIL;

		ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

		if ( m_bDeviceModeFlag & _DXG_DEVICEMODE_D3D )
			ddsd.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE;

		// All sprite decoders and software drawing routines write RGB565 pixels.
		ddsd.dwFlags |= DDSD_PIXELFORMAT;
		ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
		ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
		ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
		ddsd.ddpfPixelFormat.dwRBitMask = 0xf800;
		ddsd.ddpfPixelFormat.dwGBitMask = 0x07e0;
		ddsd.ddpfPixelFormat.dwBBitMask = 0x001f;
		ddsd.dwWidth    = m_stDisplayInfo.wWidth;
		ddsd.dwHeight   = m_stDisplayInfo.wHeight;

		if ( FAILED(hr = m_pDD->CreateSurface(&ddsd, &m_pddsBackBuffer, NULL)) )			return E_FAIL;
	}
	else
	{
		ddsd.dwFlags           = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps    = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount = 1;
		if ( m_bDeviceModeFlag & _DXG_DEVICEMODE_D3D )
			ddsd.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE;

		if ( FAILED(hr = m_pDD->CreateSurface(&ddsd, &m_pddsFrontBuffer, NULL)) )			return E_FAIL;

		DDSCAPS2 ddscaps;
		ZeroMemory(&ddscaps, sizeof(ddscaps));
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;

		if ( FAILED(hr = m_pddsFrontBuffer->GetAttachedSurface(&ddscaps, &m_pddsBackBuffer)) )
			return E_FAIL;
	}

//	为了编辑窗口输出而设置裁剪器。////////////////////////////////////////////////////////////////
	if ( FAILED(hr = m_pDD->CreateClipper(0, &m_lpcClipper, NULL)) )						return E_FAIL;
	if ( FAILED(hr = m_lpcClipper->SetHWnd(0, m_hWnd)) )
	{
		m_lpcClipper->Release();
		return E_FAIL;
	}
	if ( FAILED(hr = m_pddsFrontBuffer->SetClipper(m_lpcClipper)) )
	{
		m_lpcClipper->Release();
		return E_FAIL;
	}
    m_pDD->FlipToGDISurface();
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	return S_OK;
}


VOID CWHDXGraphicWindow::UpdateBoundsWnd()
{
	if ( m_bScreenModeFlag & _DXG_SCREENMODE_WINDOW )
	{
        GetClientRect(m_hWnd, &m_rcWindow);
        ClientToScreen(m_hWnd, (POINT*)&m_rcWindow);
        ClientToScreen(m_hWnd, (POINT*)&m_rcWindow+1);
    }
    else
    {
        SetRect(&m_rcWindow, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    }
}


HRESULT CWHDXGraphicWindow::CreateZBuffer(GUID* pDeviceGUID)
{
    HRESULT hr;

    D3DDEVICEDESC7 ddDesc;
    m_pd3dDevice->GetCaps(&ddDesc);
    if (ddDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR)
        return S_OK;

    DDSURFACEDESC2 ddsd;
	ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(ddsd);
    m_pddsBackBuffer->GetSurfaceDesc(&ddsd);

    ddsd.dwFlags				= DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
    ddsd.ddsCaps.dwCaps			= DDSCAPS_ZBUFFER;
    ddsd.ddpfPixelFormat.dwSize = 0;

    if ( FAILED(hr = m_pD3D->EnumZBufferFormats(*pDeviceGUID, DXGEnumZBufferFormatsCallback, (VOID*)&ddsd.ddpfPixelFormat)) )
	        return E_FAIL;

    if ( 0 == ddsd.ddpfPixelFormat.dwSize )
    {
        ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
        if ( FAILED(hr = m_pD3D->EnumZBufferFormats(*pDeviceGUID, DXGEnumZBufferFormatsCallback,  (VOID*)&ddsd.ddpfPixelFormat)) )
	        return E_FAIL;
        if ( 0 == ddsd.ddpfPixelFormat.dwSize )
	        return E_FAIL;
    }

    if ( FAILED(hr = m_pDD->CreateSurface(&ddsd, &m_pddsZBuffer, NULL)) )		return E_FAIL;
    if ( FAILED(hr = m_pddsBackBuffer->AddAttachedSurface(m_pddsZBuffer)) )     return E_FAIL;

    if ( FAILED(hr = m_pd3dDevice->SetRenderTarget(m_pddsBackBuffer, 0L)) )
    {
        return E_FAIL;
    }

    return S_OK;
}


HRESULT CWHDXGraphicWindow::Init3DDeviceObjects()
{
    // Set the transform matrices
    D3DMATRIX matWorld, matView, matProj;

	// 设置适合当前 800*600 屏幕的 3D 坐标系。
 	D3DVECTOR vEyePt    = D3DVECTOR(0, 0, -300.0f);
    D3DVECTOR vLookatPt = D3DVECTOR(0, 0, 0);
    D3DVECTOR vUpVec    = D3DVECTOR(0, 1, 0);
    D3DUtil_SetIdentityMatrix(matWorld);
    D3DUtil_SetViewMatrix(matView, vEyePt, vLookatPt, vUpVec);
    D3DUtil_SetProjectionMatrix(matProj, g_PI/2, float(600.0f/800.0f), -1.0f, 1.0f);

	// 设置 MATRIAL 的初始化.
    D3DMATERIAL7 mtrl;
	ZeroMemory(&mtrl, sizeof(mtrl));
	mtrl.diffuse.r = mtrl.diffuse.g = mtrl.diffuse.b = 0.1f;
	mtrl.ambient.r = mtrl.ambient.g = mtrl.ambient.b = 1.0f;
    m_pd3dDevice->SetMaterial(&mtrl);

	if ( !m_pd3dDevice )
		return E_FAIL;

    m_pd3dDevice->SetTransform(D3DTRANSFORMSTATE_WORLD,      &matWorld);
    m_pd3dDevice->SetTransform(D3DTRANSFORMSTATE_VIEW,       &matView);
    m_pd3dDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj);

	// RenderState 的初始值设置.
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_AMBIENT, 0xFFFFFFFF);
    m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE); 
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE); 	
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_FALSE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
//	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME);
//	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);
	
    // TextureStageState 的初始值设置。
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE); 
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_POINT); 
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_NONE);

    return S_OK;
}


HFONT CWHDXGraphicWindow::CreateGameFont(LPCSTR szFontName, INT nHeight, INT nWidth, INT nWeight, BOOL bItalic, BOOL bULine, BOOL bStrikeOut, DWORD dwCharSet)
{
	// Godot raster_text uses the original client's 96-DPI logical font size.
	const INT nYsize = -MulDiv(nHeight, 96, 72);

	return CreateFont(nYsize, nWidth, 0, 0, nWeight, bItalic, bULine, bStrikeOut, dwCharSet, 
					  OUT_DEFAULT_PRECIS,	CLIP_DEFAULT_PRECIS, 
					  DEFAULT_QUALITY, DEFAULT_PITCH | FF_ROMAN, szFontName);

}
VOID CWHDXGraphicWindow::CreateDefFont()
{
	// 如果默认字体已创建, 则 Clear 以便可以重新创建。
	if ( m_hDefGameFont != NULL )
	{
		DeleteObject(m_hDefGameFont); 
		m_hDefGameFont = NULL;
	}
//is font name
	m_hDefGameFont	= CreateGameFont("宋体", 9);
}



VOID CWHDXGraphicWindow::StringPlus(CHAR* szResult, CHAR* szSrc, ...)
{
	INT nCnt = 0;

	va_list vaMarker;
	CHAR*	pszString;


//	ZeroMemory(szResult, strlen(szResult));
	va_start(vaMarker, szSrc);
	pszString = szSrc;
	
	while ( TRUE )
	{		
		strcpy(&szResult[strlen(szResult)], pszString);
		if ( !strcmp(pszString,  "") )		break;
		pszString = (CHAR*)va_arg(vaMarker, CHAR*);
	}

	va_end(vaMarker);
}


CHAR* CWHDXGraphicWindow::IntToStr(INT nNum)
{
	static CHAR szResult[MAX_PATH];
	ZeroMemory(szResult, MAX_PATH);
	_itoa(nNum, szResult, 10);
	return szResult;
}




VOID CWHDXGraphicWindow::PutsHan(LPDIRECTDRAWSURFACE7 pSurface, INT nX, INT nY, COLORREF foreColor, COLORREF backColor, CHAR* szText, HFONT hFont)
{
	HDC		hDC;
	HFONT	hOldFont;

    if (NULL == pSurface)
		pSurface = m_pddsBackBuffer;
			
    if ( NULL == pSurface )
		return;

	if ( NULL == hFont )
		hFont = m_hDefGameFont;

	HRESULT hr = pSurface->GetDC(&hDC);

	if (backColor == RGB( 0, 0, 0 ))		SetBkMode(hDC, TRANSPARENT);	
	else									SetBkColor(hDC, backColor);

	hOldFont = (HFONT)SelectObject(hDC, hFont);

	SetTextColor(hDC, foreColor);

	TextOut(hDC, nX, nY, szText, strlen(szText));

	SelectObject(hDC, hOldFont);

	if ( hFont != m_hDefGameFont )
		DeleteObject(hFont);
	
	pSurface->ReleaseDC(hDC);
}


VOID CWHDXGraphicWindow::PutsHan(LPDIRECTDRAWSURFACE7 pSurface, RECT rc, COLORREF foreColor, COLORREF backColor, CHAR* szText, HFONT hFont)
{
	HDC		hDC;
	HFONT	hOldFont;

    if (NULL == pSurface)
		pSurface = m_pddsBackBuffer;
			
    if ( NULL == pSurface )
		return;

	if ( NULL == hFont )
		hFont = m_hDefGameFont;

	HRESULT hr = pSurface->GetDC(&hDC);

	if (backColor == RGB( 0, 0, 0 ))		SetBkMode(hDC, TRANSPARENT);	
	else									SetBkColor(hDC, backColor);

	hOldFont = (HFONT)SelectObject(hDC, hFont);

	SetTextColor(hDC, foreColor);

	DrawText(hDC, szText, -1, &rc, DT_CENTER|DT_SINGLELINE);

	SelectObject(hDC, hOldFont);

	if ( hFont != m_hDefGameFont )
		DeleteObject(hFont);
	
	pSurface->ReleaseDC(hDC);
}


SIZE CWHDXGraphicWindow::GetStrLength(LPDIRECTDRAWSURFACE7 pSurface, HFONT hFont, CHAR* szFormat, ...)
{
	HDC		hDC;
	va_list arg;
    CHAR	szBuf[1024];
	SIZE	sSize;
	
	sSize.cx = 0;
	sSize.cy = 0;

	HFONT	hOldFont;

	if ( NULL == pSurface )
		pSurface = m_pddsBackBuffer;
			
    if ( NULL == pSurface )					return sSize;

	if ( NULL == hFont )
		hFont = m_hDefGameFont;

	HRESULT hr = pSurface->GetDC(&hDC);

	va_start(arg, szFormat);
    vsprintf(szBuf, szFormat, arg);
    va_end(arg);

	hOldFont = (HFONT)SelectObject(hDC, hFont);

	GetTextExtentPoint32(hDC,szBuf,strlen(szBuf),&sSize);

	SelectObject(hDC, hOldFont);
	
	pSurface->ReleaseDC(hDC);

	return sSize;
}


/*BOOL CWHDXGraphicWindow::StringDivideLen(INT nDivideLen, INT& nDividedLine, CHAR* szSrc,CDLList<CHAR*>* m_pxpStr)
{
	CHAR szTmpCheck[MAX_PATH];
	CHAR szResult[MAX_PATH];

	nDividedLine = 1;

	INT nStartLen	= 0;
	INT nEndLen		= 0;
	INT nResultLen	= 0;
	INT nWordCheck	= 0;

	if ( szSrc[0] != NULL )
	{
		ZeroMemory(szResult,MAX_PATH);

		for ( INT nCnt = 0; nCnt < (INT)strlen(szSrc); nCnt++)
		{
			ZeroMemory(szTmpCheck, MAX_PATH);
			nEndLen = nCnt+1;

			// 读取到当前部分的字符串。
			memcpy(&szTmpCheck, &szSrc[nStartLen], nEndLen-nStartLen);
			INT nsLen;
			nsLen = strlen(szTmpCheck);

			// 当已读取到的部分超过允许宽度时。
			if (nsLen > nDivideLen )
			{
				// 当前若是 2 字节字符。
				if ( szSrc[nEndLen-1] < 0 )
				{
					// 若当前前面的字符不是 2 字节字符
					if ( !(nWordCheck%2) )
					{
						nStartLen += strlen(szTmpCheck)-1;
						nCnt--;
						CHAR* pszNewLine;
						pszNewLine = new CHAR[nResultLen+1];
						memcpy(pszNewLine,szResult,nResultLen);
						pszNewLine[nResultLen]=NULL;
						m_pxpStr->AddNode(pszNewLine);
						nResultLen = 0;
						nDividedLine++;
					}
					else
					{
						nStartLen += strlen(szTmpCheck)-2;
						nCnt -= 2;
						CHAR* pszNewLine;
						pszNewLine = new CHAR[nResultLen];
						memcpy(pszNewLine,szResult,nResultLen-1);
						pszNewLine[nResultLen-1]=NULL;
						m_pxpStr->AddNode(pszNewLine);
						nResultLen = 0;
						nDividedLine++;
						nWordCheck--;
					}
				}
				// 1 字节字符。
				// 只需考虑到比当前位置往前一个字节的部分即可。
				else
				{
					nStartLen += strlen(szTmpCheck)-1;
					nCnt--;
					CHAR* pszNewLine;
					pszNewLine = new CHAR[nResultLen+1];
					memcpy(pszNewLine,szResult,nResultLen);
					pszNewLine[nResultLen]=NULL;
					m_pxpStr->AddNode(pszNewLine);
					nResultLen=0;
					nDividedLine++;
				}
			}
			else if(szSrc[nEndLen-1]=='\n')	// 强制换行     用 \n 的话  Error ????
			{
				nStartLen += strlen(szTmpCheck)-1;
				CHAR* pszNewLine;
				pszNewLine = new CHAR[nResultLen+1];
				memcpy(pszNewLine,szResult,nResultLen);
				pszNewLine[nResultLen]=NULL;
				m_pxpStr->AddNode(pszNewLine);
				nResultLen=0;
				nDividedLine++;
			}
			else
			{
				if ( szSrc[nEndLen-1] < 0 )
					nWordCheck++;

				szResult[nResultLen] = szSrc[nEndLen-1];
				nResultLen++;
			}
		}
		if(nResultLen!=0)
		{
			CHAR* pszNewLine;
			pszNewLine = new CHAR[nResultLen+1];
			memcpy(pszNewLine,szResult,nResultLen);
			pszNewLine[nResultLen]=NULL;
			m_pxpStr->AddNode(pszNewLine);
			nDividedLine++;
			nResultLen=0;
		}
		return TRUE;
	}
	return FALSE;
}
*/


BOOL CWHDXGraphicWindow::StringDivide(INT nDivideWidth, INT& nDividedLine, CHAR* szSrc, CHAR* szResult)
{
	CHAR szTmpCheck[MAX_PATH];
	
	nDividedLine = 1;

	INT nStartLen	= 0;
	INT nEndLen		= 0;
	INT nResultLen	= 0;

	INT nWordCheck	= 0;

	if ( szSrc[0] != NULL )
	{
		ZeroMemory(szResult, MAX_PATH);

		for ( INT nCnt = 0; nCnt < (INT)strlen(szSrc); nCnt++)
		{
			ZeroMemory(szTmpCheck, MAX_PATH);

			nEndLen = nCnt+1;

			// 如果字体大小超过最大字体大小，则调整为最大值.
			if ( nEndLen >= MAX_PATH )
			{
				szResult[MAX_PATH-1] = NULL;
				break;
			}

			// 读取截至当前部分的字符串。
			memcpy(&szTmpCheck, &szSrc[nStartLen], nEndLen-nStartLen);

			SIZE sizeLen;
			sizeLen = GetStrLength(NULL, NULL, szTmpCheck);

			// 当前已读取的部分超出允许宽度时。
			if ( sizeLen.cx > nDivideWidth )
			{
				// 当前如果是 2 字节字符. 
				if ( szSrc[nEndLen-1] < 0 )
				{
					// 如果当前前面的字符不是 2 字节字符
					if ( !(nWordCheck%2) )
					{
						nStartLen += strlen(szTmpCheck)-1;
						nCnt--;
						szResult[nResultLen] = '`';
						nResultLen++;
						nDividedLine++;
					}
					else
					{
						nStartLen += strlen(szTmpCheck)-2;
						nCnt -= 2;
						szResult[nResultLen-1] = '`';
						nDividedLine++;
						nWordCheck--;
					}
				}
				// 1字节字符. 
				// 只需考虑到比当前位置往前一个字节的部分即可。
				else
				{
					nStartLen += strlen(szTmpCheck)-1;
					nCnt--;
					szResult[nResultLen] = '`';
					nResultLen++;
					nDividedLine++;
				}
			}
			else
			{
				if ( szSrc[nEndLen-1] < 0 )
					nWordCheck++;

				szResult[nResultLen] = szSrc[nEndLen-1];
				nResultLen++;
			}
		}
		return TRUE;
	}
	return FALSE;

}


DXG_MASKINFO CWHDXGraphicWindow::GetRGBMaskInfoIN16Bits(LPDIRECTDRAWSURFACE7 pSurface)
{	
	DDPIXELFORMAT ddpf;
	DXG_MASKINFO stRGBMaskInfo;

	ZeroMemory(&stRGBMaskInfo, sizeof(DXG_MASKINFO));

	if ( pSurface == NULL )		pSurface = m_pddsBackBuffer;

    if ( pSurface )
	{
		memset(&ddpf, 0, sizeof(ddpf));
		ddpf.dwSize = sizeof(ddpf);
		ddpf.dwFlags = DDPF_RGB;
		pSurface->GetPixelFormat(&ddpf);

		stRGBMaskInfo.dwRMask = ddpf.dwRBitMask;
		stRGBMaskInfo.dwGMask = ddpf.dwGBitMask;
		stRGBMaskInfo.dwBMask = ddpf.dwBBitMask;

		DWORD dwCnt;
		DWORD dwMask;
		for ( dwCnt = 0, dwMask = ddpf.dwRBitMask; !( dwMask & 1 ); dwCnt++, dwMask >>= 1 );
		stRGBMaskInfo.bRShift	= (BYTE) dwCnt;
		for ( dwCnt = 0; ( ( dwMask>>dwCnt ) & 1 ); dwCnt++ );
		stRGBMaskInfo.bRCnt		= (BYTE) dwCnt;

		for ( dwCnt = 0, dwMask = ddpf.dwGBitMask; !( dwMask & 1 ); dwCnt++, dwMask >>= 1 );
		stRGBMaskInfo.bGShift	= (BYTE) dwCnt;
		for ( dwCnt = 0; ( ( dwMask>>dwCnt ) & 1 ); dwCnt++ );
		stRGBMaskInfo.bGCnt		= (BYTE) dwCnt;

		for ( dwCnt = 0, dwMask = ddpf.dwBBitMask; !( dwMask & 1 ); dwCnt++, dwMask >>= 1 );
		stRGBMaskInfo.bBShift	= (BYTE) dwCnt;
		for ( dwCnt = 0; ( ( dwMask>>dwCnt ) & 1 ); dwCnt++ );
		stRGBMaskInfo.bBCnt		= (BYTE) dwCnt;
	}

	return stRGBMaskInfo;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 绘制函数集合.

WORD CWHDXGraphicWindow::ConvertColor24To16(COLORREF dwColor)
{
	WORD wRet;

	INT nRed  = RGB_GETRED(dwColor);
	INT nGreen = RGB_GETGREEN(dwColor);
	INT nBlue   = RGB_GETBLUE(dwColor);
	
	nRed = (nRed*(1<<m_stBitsMaskInfo.bRCnt))/(1<<8);
	nGreen = (nGreen*(1<<m_stBitsMaskInfo.bGCnt))/(1<<8);
	nBlue = (nBlue*(1<<m_stBitsMaskInfo.bBCnt))/(1<<8);

	wRet = (nRed << m_stBitsMaskInfo.bRShift) | (nGreen << m_stBitsMaskInfo.bGShift) | (nBlue << m_stBitsMaskInfo.bBShift); 
	return wRet;
}


HRESULT CWHDXGraphicWindow::DrawWithGDI(RECT rc, LPDIRECTDRAWSURFACE7 pSurface, DWORD dwColor, BYTE bKind)
{
	POINT	pt;
	HDC		hDC;
	HPEN	hPen, hOldPen;
	HBRUSH	hBrush, hOldBrush;
	HRESULT hr;

	if ( pSurface == NULL )
	{
		pSurface = m_pddsBackBuffer;
	}

	if ( pSurface == NULL )
		return E_FAIL;

	if ( FAILED(hr = pSurface->GetDC(&hDC)) )
	{
		return E_FAIL;
	}

	switch ( bKind )
	{
	case 0:
		{
			hPen = CreatePen(PS_SOLID, NULL, dwColor);
		
			hOldPen = (HPEN)SelectObject(hDC, hPen);

			MoveToEx(hDC, rc.left, rc.top, &pt);
			LineTo(hDC, rc.right, rc.bottom);

			SelectObject(hDC, hOldPen);
			DeleteObject(hPen);
		}
		break;
	case 1:
		{
			hBrush = CreateSolidBrush(dwColor);

			hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);

			FrameRect(hDC, &rc, hBrush);

			SelectObject(hDC, hOldBrush);
			DeleteObject(hBrush);
		}
		break;
	case 2:
		{
			hBrush = CreateSolidBrush(dwColor);

			hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);

			FillRect(hDC, &rc, hBrush);

			SelectObject(hDC, hOldBrush);
			DeleteObject(hBrush);
		}
		break;
	default:
		break;
	}

	pSurface->ReleaseDC(hDC);

	return S_OK;
}


/******************************************************************************************************************

	函数名 : CWHDXGraphicWindow::DrawSprite()

	作者 :
	编写日期 :

	目的   : 常规绘制. 逐像素绘制. 屏幕区域裁剪, 源区域裁剪
	输入   : INT nX
	         INT nY
	         INT nXSize
	         INT nYSize
	         WORD* pwSrc
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CWHDXGraphicWindow::DrawWithImage(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc)
{
	RECT			rc;
	DDSURFACEDESC2	ddsd;

	INT	nWidth		= nXSize;
	INT	nHeight		= nYSize;
	INT	nXOffset	= 0;
	INT	nYOffset	= 0;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= m_stDisplayInfo.wWidth -1;
	INT	nEndY		= m_stDisplayInfo.wHeight-1;

	if ( m_pddsBackBuffer != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset = nStartX - nX;
			nWidth	 = nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth	 = nEndX - nX - nXOffset + 1;
		
		if ( nY < nStartY )
		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset + nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset + nHeight;

			ddsd.dwSize	= sizeof(DDSURFACEDESC2);
			ddsd.lpSurface = NULL;
			
			m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
			if ( !ddsd.lpSurface )		return FALSE;
			WORD* pwdDst;
			pwdDst = (WORD*)ddsd.lpSurface;

			for ( INT y=rc.top ; y < rc.bottom ; y++ )
			{
				for ( INT x=rc.left ; x < rc.right ; x++ )
				{
					if ( pwSrc[y * nXSize + x] != 0 )
					{
						pwdDst[((y+nY) * (ddsd.lPitch >> 1)) + (nX+x)] = pwSrc[y * nXSize + x];
					}
				}
			}

			m_pddsBackBuffer->Unlock(NULL);
		}
		return TRUE;
	}
	return FALSE;
}


BOOL CWHDXGraphicWindow::DrawWithImagePerLineClipRgn(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight, WORD wSrcWidth)
{
	RECT			rc;
	DDSURFACEDESC2	ddsd;

	INT	nWidth		= nXSize;
	INT	nHeight		= nYSize;
	INT	nXOffset	= 0;
	INT	nYOffset	= 0;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= wClipWidth  - 1;
	INT	nEndY		= wClipHeight - 1;

	if ( m_pddsBackBuffer != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset = nStartX - nX;
			nWidth	 = nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth	 = nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )
		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset + nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset + nHeight;

			ddsd.dwSize	= sizeof(DDSURFACEDESC2);
			ddsd.lpSurface = NULL;
			
			m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
			if ( !ddsd.lpSurface )			return FALSE;
			WORD* pwdDst;
			pwdDst = (WORD*)ddsd.lpSurface;

			for ( INT y=rc.top ; y < rc.bottom ; y++ )
			{
				memcpy(&pwdDst[((y+nY) * (ddsd.lPitch >> 1)) + (nX + rc.left)], &pwSrc[y * wSrcWidth + rc.left], (rc.right-rc.left)*2);
			}

			m_pddsBackBuffer->Unlock(NULL);
		}
		return TRUE;
	}
	return FALSE;
}



BOOL CWHDXGraphicWindow::DrawWithSurface(INT nX, INT nY, INT nXSize, INT nYSize, LPDIRECTDRAWSURFACE7 pddsSrc)
{
	RECT	rc;
	RECT	rcDst;
	INT		nXOffset	= 0;
	INT		nYOffset	= 0;
	INT		nWidth		= nXSize;
	INT		nHeight		= nYSize;
	INT		nStartX		= 0;
	INT		nStartY		= 0;

	INT		nEndX		= m_stDisplayInfo.wWidth -1;
	INT		nEndY		= m_stDisplayInfo.wHeight-1;;


	if ( m_pddsBackBuffer != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset	= nStartX - nX;
			nWidth		= nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth		= nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )
		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}		
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		 = nXOffset;
			rc.right	 = nXOffset+nWidth;
			rc.top		 = nYOffset;
			rc.bottom	 = nYOffset+nHeight;

			rcDst.left	 = rc.left+nX;
			rcDst.top	 = rc.top+nY;
			rcDst.right  = rc.right+nX;
			rcDst.bottom = rc.bottom+nY;

			m_pddsBackBuffer->Blt(&rcDst, pddsSrc, &rc, DDBLT_WAIT|DDBLT_KEYSRC, NULL);
		}
		return TRUE;
	}
	return FALSE;
}


BOOL CWHDXGraphicWindow::DrawWithImageForCompMemToMem(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, INT nDstXSize, INT nDstYSize, WORD* pwDst, WORD wChooseColor1, WORD wChooseColor2)
{
	RECT			rc;

	INT	nWidth		= nXSize;
	INT	nHeight		= nYSize;
	INT	nXOffset	= 0;
	INT	nYOffset	= 0;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= nDstXSize - 1;
	INT	nEndY		= nDstYSize - 1;

	if ( pwDst != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset	= nStartX - nX;
			nWidth		= nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth		= nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}		
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset+nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset+nHeight;

			INT nWidthStart	= 0;
			INT nWidthEnd	= 0;
			INT nCurrWidth  = 0;
			INT nCntCopyWord = 0;
			INT nYCnt =0;
			INT nLastWidth = 0;

			FLOAT rBlueRate, rGreenRate, bRedRate;
			BYTE  bRedSrc, bGreenSrc, bBlueSrc;
			BYTE  bRedWantedColor, bGreenWantedColor, bBlueWantedColor;
			WORD  wPixel;

			// y 轴裁剪。
			for ( nYCnt=0 ; nYCnt < rc.top ; nYCnt++ )
			{
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;
				nWidthEnd++;
				nWidthStart	= nWidthEnd;
			}

			// 只决定对 y 轴实际进行循环的 Count.
			for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
			{
				// 获取一行的长度.(单位为字)
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;

				// 按行的长度输出到屏幕。
				for ( INT x = nWidthStart; x < nWidthEnd ; )
				{
					if ( pwSrc[x] == 0xC0 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;
						nCurrWidth += nCntCopyWord;
					}
					else if ( pwSrc[x] == 0xC1 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// 以 rc.left 为基准,复制区域被截取的情况.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								memcpy(&pwDst[((nYCnt+nY) * nDstXSize) + (rc.left+nX)], &pwSrc[x], sizeof(WORD)*(nCurrWidth-rc.left));
								x += (nCurrWidth-rc.left);
							}
							// 以 rc.right 点为基准，复制区域被截断的情况.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								memcpy(&pwDst[((nYCnt+nY) * nDstXSize) + (nLastWidth+nX)], &pwSrc[x], sizeof(WORD)*(rc.right-nLastWidth));
								x += nCntCopyWord;
							}
							else
							{
								memcpy(&pwDst[((nYCnt+nY) * nDstXSize) + (nLastWidth+nX)], &pwSrc[x], sizeof(WORD)*nCntCopyWord);
								x += nCntCopyWord;
							}
						}
					}
					else if ( pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
					{
						WORD wDyingKind, wChooseColor;
						wDyingKind = pwSrc[x];
						switch ( wDyingKind )
						{
						case 0xC2: 
							wChooseColor = wChooseColor1;
							break;
						case 0xC3: 
							wChooseColor = wChooseColor2;
							break;
						}
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// 以 rc.left 为基准,复制区域被截取的情况.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								for ( INT nCheck = 0; nCheck < nCurrWidth-rc.left; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwDst[((nYCnt+nY) * nDstXSize) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																							    (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																								(bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += (nCurrWidth-rc.left);
							}
							// 以 rc.right 点为基准，复制区域被截断的情况.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for ( INT nCheck = 0; nCheck < rc.right-nLastWidth; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwDst[((nYCnt+nY) * nDstXSize) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																								(bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																								(bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
							else
							{
								for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwDst[((nYCnt+nY) * nDstXSize) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																								(bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																								(bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
						}
					}	
				}
				// 将本行的结尾移到下一行的开头.
				nWidthEnd++;

				nWidthStart	= nWidthEnd;
				nCurrWidth = 0;
			}
		}

		m_pddsBackBuffer->Unlock(NULL);
		return TRUE;
	}
	return FALSE;
}


BOOL CWHDXGraphicWindow::DrawWithImageForComp(INT nX, INT nY, RECT rcSrc, WORD* pwSrc, WORD wChooseColor1, WORD wChooseColor2)
{
	RECT			rc;
	DDSURFACEDESC2	ddsd;

	INT	nWidth		= rcSrc.right-rcSrc.left;
	INT	nHeight		= rcSrc.bottom-rcSrc.top;
	INT	nXOffset	= rcSrc.left;
	INT	nYOffset	= rcSrc.top;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= m_stDisplayInfo.wWidth -1;
	INT	nEndY		= m_stDisplayInfo.wHeight-1;

	if ( m_pddsBackBuffer != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset += nStartX - nX;
			nWidth	 = rcSrc.right - nXOffset;
		}
		if ( (nX+rcSrc.right-rcSrc.left-1) > nEndX )
			nWidth		= nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )	
		{ 
			nYOffset += nStartY - nY;
			nHeight	 = rcSrc.bottom - nYOffset;
		}		
		if ( (nY+rcSrc.bottom-rcSrc.top-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;


		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset+nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset+nHeight;

			ddsd.dwSize	= sizeof(DDSURFACEDESC2);
			ddsd.lpSurface = NULL;
			
			m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
			if ( !ddsd.lpSurface )			return FALSE;
			WORD* pwdDst;
			pwdDst = (WORD*)ddsd.lpSurface;

			INT nWidthStart	= 0;
			INT nWidthEnd	= 0;
			INT nCurrWidth  = 0;
			INT nCntCopyWord = 0;
			INT nYCnt =0;
			INT nLastWidth = 0;

			FLOAT rBlueRate, rGreenRate, bRedRate;
			BYTE  bRedSrc, bGreenSrc, bBlueSrc;
			BYTE  bRedWantedColor, bGreenWantedColor, bBlueWantedColor;
			WORD  wPixel;

			// y 轴裁剪。
			for ( nYCnt=0 ; nYCnt < rc.top ; nYCnt++ )
			{
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;
				nWidthEnd++;
				nWidthStart	= nWidthEnd;
			}

			// 只决定对 y 轴实际进行循环的 Count.
			for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
			{
				// 获取一行的长度.(单位为字)
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;

				// 按行的长度输出到屏幕。
				for ( INT x = nWidthStart; x < nWidthEnd ; )
				{
					if ( pwSrc[x] == 0xC0 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;
						nCurrWidth += nCntCopyWord;
					}
					else if ( pwSrc[x] == 0xC1 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// 以 rc.left 为基准,复制区域被截取的情况.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								memcpy(&pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (rc.left+nX)], &pwSrc[x], sizeof(WORD)*(nCurrWidth-rc.left));
								x += (nCurrWidth-rc.left);
							}
							// 以 rc.right 点为基准，复制区域被截断的情况.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								memcpy(&pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX)], &pwSrc[x], sizeof(WORD)*(rc.right-nLastWidth));
								x += nCntCopyWord;
							}
							else
							{
								memcpy(&pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX)], &pwSrc[x], sizeof(WORD)*nCntCopyWord);
								x += nCntCopyWord;
							}
						}
					}
					else if ( pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
					{
						WORD wDyingKind, wChooseColor;
						wDyingKind = pwSrc[x];
						switch ( wDyingKind )
						{
						case 0xC2: 
							wChooseColor = wChooseColor1;
							break;
						case 0xC3: 
							wChooseColor = wChooseColor2;
							break;
						}
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// 以 rc.left 为基准,复制区域被截取的情况.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								for ( INT nCheck = 0; nCheck < nCurrWidth-rc.left; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += (nCurrWidth-rc.left);
							}
							// 以 rc.right 点为基准，复制区域被截断的情况.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for ( INT nCheck = 0; nCheck < rc.right-nLastWidth; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
							else
							{
								for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																									 	  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
						}
					}	
				}
				// 将本行的结尾移到下一行的开头.
				nWidthEnd++;

				nWidthStart	= nWidthEnd;
				nCurrWidth = 0;
			}
		}

		m_pddsBackBuffer->Unlock(NULL);
		return TRUE;
	}
	return FALSE;
}


BOOL CWHDXGraphicWindow::DrawWithImageForComp(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wChooseColor1, WORD wChooseColor2)
{
	RECT			rc;
	DDSURFACEDESC2	ddsd;

	INT	nWidth		= nXSize;
	INT	nHeight		= nYSize;
	INT	nXOffset	= 0;
	INT	nYOffset	= 0;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= m_stDisplayInfo.wWidth -1;
	INT	nEndY		= m_stDisplayInfo.wHeight-1;

	if ( m_pddsBackBuffer != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset	= nStartX - nX;
			nWidth		= nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth		= nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )	
		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}		
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset+nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset+nHeight;

			ddsd.dwSize	= sizeof(DDSURFACEDESC2);
			ddsd.lpSurface = NULL;
			
			m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
			if ( !ddsd.lpSurface )			return FALSE;
			WORD* pwdDst;
			pwdDst = (WORD*)ddsd.lpSurface;

			INT nWidthStart	= 0;
			INT nWidthEnd	= 0;
			INT nCurrWidth  = 0;
			INT nCntCopyWord = 0;
			INT nYCnt =0;
			INT nLastWidth = 0;

			FLOAT rBlueRate, rGreenRate, bRedRate;
			BYTE  bRedSrc, bGreenSrc, bBlueSrc;
			BYTE  bRedWantedColor, bGreenWantedColor, bBlueWantedColor;
			WORD  wPixel;

			// y 轴裁剪。
			for ( nYCnt=0 ; nYCnt < rc.top ; nYCnt++ )
			{
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;
				nWidthEnd++;
				nWidthStart	= nWidthEnd;
			}

			// 只决定对 y 轴实际进行循环的 Count.
			for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
			{
				// 获取一行的长度.(单位为字)
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;

				// 按行的长度输出到屏幕。
				for ( INT x = nWidthStart; x < nWidthEnd ; )
				{
					if ( pwSrc[x] == 0xC0 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;
						nCurrWidth += nCntCopyWord;
					}
					else if ( pwSrc[x] == 0xC1 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// 以 rc.left 为基准,复制区域被截取的情况.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								memcpy(&pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (rc.left+nX)], &pwSrc[x], sizeof(WORD)*(nCurrWidth-rc.left));
								x += (nCurrWidth-rc.left);
							}
							// 以 rc.right 点为基准，复制区域被截断的情况.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								memcpy(&pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX)], &pwSrc[x], sizeof(WORD)*(rc.right-nLastWidth));
								x += nCntCopyWord;
							}
							else
							{
								memcpy(&pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX)], &pwSrc[x], sizeof(WORD)*nCntCopyWord);
								x += nCntCopyWord;
							}
						}
					}
					else if ( pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
					{
						WORD wDyingKind, wChooseColor;
						wDyingKind = pwSrc[x];
						switch ( wDyingKind )
						{
						case 0xC2: 
							wChooseColor = wChooseColor1;
							break;
						case 0xC3: 
							wChooseColor = wChooseColor2;
							break;
						}
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// 以 rc.left 为基准,复制区域被截取的情况.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								for ( INT nCheck = 0; nCheck < nCurrWidth-rc.left; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += (nCurrWidth-rc.left);
							}
							// 以 rc.right 点为基准，复制区域被截断的情况.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for ( INT nCheck = 0; nCheck < rc.right-nLastWidth; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
							else
							{
								for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																									 	  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
						}
					}	
				}
				// 将本行的结尾移到下一行的开头.
				nWidthEnd++;

				nWidthStart	= nWidthEnd;
				nCurrWidth = 0;
			}
		}

		m_pddsBackBuffer->Unlock(NULL);
		return TRUE;
	}
	return FALSE;
}


BOOL CWHDXGraphicWindow::DrawWithImageForCompClipRgnBase(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight, WORD wChooseColor1, WORD wChooseColor2)
{
	RECT			rc;
	DDSURFACEDESC2	ddsd;

	INT	nWidth		= nXSize;
	INT	nHeight		= nYSize;
	INT	nXOffset	= 0;
	INT	nYOffset	= 0;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= wClipWidth -1;
	INT	nEndY		= wClipHeight-1;

	if ( m_pddsBackBuffer != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset	= nStartX - nX;
			nWidth		= nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth		= nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )	
		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}		
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset+nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset+nHeight;

			ddsd.dwSize	= sizeof(DDSURFACEDESC2);
			ddsd.lpSurface = NULL;
			
			m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
			if ( !ddsd.lpSurface )			return FALSE;
			WORD* pwdDst;
			pwdDst = (WORD*)ddsd.lpSurface;

			INT nWidthStart	= 0;
			INT nWidthEnd	= 0;
			INT nCurrWidth  = 0;
			INT nCntCopyWord = 0;
			INT nYCnt =0;
			INT nLastWidth = 0;

			FLOAT rBlueRate, rGreenRate, bRedRate;
			BYTE  bRedSrc, bGreenSrc, bBlueSrc;
			BYTE  bRedWantedColor, bGreenWantedColor, bBlueWantedColor;
			WORD  wPixel;

			// y 轴裁剪。
  			for ( nYCnt=0 ; nYCnt < rc.top ; nYCnt++ )
			{
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;
				nWidthEnd++;
				nWidthStart	= nWidthEnd;
			}

			// 只决定对 y 轴实际进行循环的 Count.
			for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
			{
				// 获取一行的长度.(单位为字)
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;

				// 按行的长度输出到屏幕。
				for ( INT x = nWidthStart; x < nWidthEnd ; )
				{
					if ( pwSrc[x] == 0xC0 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;
						nCurrWidth += nCntCopyWord;
					}
					else if ( pwSrc[x] == 0xC1 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// 以 rc.left 为基准,复制区域被截取的情况.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								memcpy(&pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (rc.left+nX)], &pwSrc[x], sizeof(WORD)*(nCurrWidth-rc.left));
								x += (nCurrWidth-rc.left);
							}
							// 以 rc.right 点为基准，复制区域被截断的情况.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								memcpy(&pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX)], &pwSrc[x], sizeof(WORD)*(rc.right-nLastWidth));
								x += nCntCopyWord;
							}
							else
							{
								memcpy(&pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX)], &pwSrc[x], sizeof(WORD)*nCntCopyWord);
								x += nCntCopyWord;
							}
						}
					}
					else if ( pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
					{
						WORD wDyingKind, wChooseColor;
						wDyingKind = pwSrc[x];
						switch ( wDyingKind )
						{
						case 0xC2: 
							wChooseColor = wChooseColor1;
							break;
						case 0xC3: 
							wChooseColor = wChooseColor2;
							break;
						}
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// 以 rc.left 为基准,复制区域被截取的情况.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								for ( INT nCheck = 0; nCheck < nCurrWidth-rc.left; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																									 	  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += (nCurrWidth-rc.left);
							}
							// 以 rc.right 点为基准，复制区域被截断的情况.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for ( INT nCheck = 0; nCheck < rc.right-nLastWidth; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																									 	  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
							else
							{
								for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																									 	  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
						}
					}	
				}
				// 将本行的结尾移到下一行的开头.
				nWidthEnd++;

				nWidthStart	= nWidthEnd;
				nCurrWidth = 0;
			}
		}

		m_pddsBackBuffer->Unlock(NULL);
		return TRUE;
	}
	return FALSE;
}


BOOL CWHDXGraphicWindow::DrawWithABlendCompDataWithBackBuffer(INT nX, INT nY, 
															  INT nXSize, INT nYSize, WORD* pwSrc,
															  WORD wClipWidth, WORD wClipHeight,
															  WORD wChooseColor1, WORD wChooseColor2, BYTE bOpa)
{
	RECT			rc;
	DDSURFACEDESC2	ddsd;

	INT	nWidth		= nXSize;
	INT	nHeight		= nYSize;
	INT	nXOffset	= 0;
	INT	nYOffset	= 0;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= wClipWidth -1;
	INT	nEndY		= wClipHeight-1;

	WORD wDyingKind, wChooseColor;

	if ( m_pddsBackBuffer != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset	= nStartX - nX;
			nWidth		= nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth		= nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )	
		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}		
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset+nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset+nHeight;

			ddsd.dwSize	= sizeof(DDSURFACEDESC2);
			ddsd.lpSurface = NULL;
			
			m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
			if ( !ddsd.lpSurface )			return FALSE;
			WORD* pwdDst;
			pwdDst = (WORD*)ddsd.lpSurface;

			INT nWidthStart	= 0;
			INT nWidthEnd	= 0;
			INT nCurrWidth  = 0;
			INT nCntCopyWord = 0;
			INT nYCnt =0;
			INT nLastWidth = 0;

			BYTE bRedDst, bGreenDst, bBlueDst;
			BYTE bRedSrc, bGreenSrc, bBlueSrc;
			BYTE bRedWantedColor, bGreenWantedColor, bBlueWantedColor;
			WORD wPixel;
			FLOAT rBlueRate, rGreenRate, bRedRate;

			// y 轴裁剪。
  			for ( nYCnt=0 ; nYCnt < rc.top ; nYCnt++ )
			{
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;
				nWidthEnd++;
				nWidthStart	= nWidthEnd;
			}

			// 只决定对 y 轴实际进行循环的 Count.
			for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
			{
				// 获取一行的长度.(单位为字)
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;

				// 按行的长度输出到屏幕。
				for ( INT x = nWidthStart; x < nWidthEnd ; )
				{
					if ( pwSrc[x] == 0xC0 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;
						nCurrWidth += nCntCopyWord;
					}
					else if ( pwSrc[x] == 0xC1 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// 以 rc.left 为基准,复制区域被截取的情况.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								for ( INT nCheck = 0; nCheck < nCurrWidth-rc.left; nCheck++ )
								{
									wPixel	   = pwSrc[x+nCheck];
									bBlueSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	   = pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (rc.left+nX+nCheck)];
									bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									bBlueDst   = (BYTE)((bOpa*(bBlueDst -bBlueSrc )+100*bBlueSrc  ) / 100);
									bGreenDst  = (BYTE)((bOpa*(bGreenDst-bGreenSrc)+100*bGreenSrc ) / 100);
									bRedDst    = (BYTE)((bOpa*(bRedDst  -bRedSrc  )+100*bRedSrc   ) / 100);

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (rc.left+nX+nCheck)] = ((bRedDst  <<m_stBitsMaskInfo.bRShift) |
																									   (bGreenDst<<m_stBitsMaskInfo.bGShift) |
																									   (bBlueDst <<m_stBitsMaskInfo.bBShift));										
								}
								x += (nCurrWidth-rc.left);
							}
							// 以 rc.right 点为基准，复制区域被截断的情况.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for ( INT nCheck = 0; nCheck < rc.right-nLastWidth; nCheck++ )
								{
									wPixel	   = pwSrc[x+nCheck];
									bBlueSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	   = pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)];
									bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									bBlueDst   = (BYTE)((bOpa*(bBlueDst -bBlueSrc )+100*bBlueSrc  ) / 100);
									bGreenDst  = (BYTE)((bOpa*(bGreenDst-bGreenSrc)+100*bGreenSrc ) / 100);
									bRedDst    = (BYTE)((bOpa*(bRedDst  -bRedSrc  )+100*bRedSrc   ) / 100);

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bRedDst  <<m_stBitsMaskInfo.bRShift) |
																									      (bGreenDst<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueDst <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
							else
							{
								for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
								{
									wPixel	   = pwSrc[x+nCheck];
									bBlueSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	   = pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)];
									bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									bBlueDst   = (BYTE)((bOpa*(bBlueDst -bBlueSrc )+100*bBlueSrc  ) / 100);
									bGreenDst  = (BYTE)((bOpa*(bGreenDst-bGreenSrc)+100*bGreenSrc ) / 100);
									bRedDst    = (BYTE)((bOpa*(bRedDst  -bRedSrc  )+100*bRedSrc   ) / 100);

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bRedDst  <<m_stBitsMaskInfo.bRShift) |
																									      (bGreenDst<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueDst <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
						}
					}
					else if ( pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
					{
						wDyingKind = pwSrc[x];
						switch ( wDyingKind )
						{
						case 0xC2: 
							wChooseColor = wChooseColor1;
							break;
						case 0xC3: 
							wChooseColor = wChooseColor2;
							break;
						}
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// 以 rc.left 为基准,复制区域被截取的情况.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								for ( INT nCheck = 0; nCheck < nCurrWidth-rc.left; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);


									wPixel  = pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (rc.left+nX+nCheck)];
									bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									bBlueDst   = (BYTE)((bOpa*(bBlueDst -bBlueWantedColor )+100*bBlueWantedColor ) / 100);
									bGreenDst  = (BYTE)((bOpa*(bGreenDst-bGreenWantedColor)+100*bGreenWantedColor) / 100);
									bRedDst    = (BYTE)((bOpa*(bRedDst  -bRedWantedColor  )+100*bRedWantedColor  ) / 100);

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (rc.left+nX+nCheck)] = ((bRedDst  <<m_stBitsMaskInfo.bRShift) |
																									   (bGreenDst<<m_stBitsMaskInfo.bGShift) |
																									   (bBlueDst <<m_stBitsMaskInfo.bBShift));
								}
								x += (nCurrWidth-rc.left);
							}
							// 以 rc.right 点为基准，复制区域被截断的情况.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for ( INT nCheck = 0; nCheck < rc.right-nLastWidth; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);


									wPixel  = pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)];
									bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									bBlueDst   = (BYTE)((bOpa*(bBlueDst -bBlueWantedColor )+100*bBlueWantedColor ) / 100);
									bGreenDst  = (BYTE)((bOpa*(bGreenDst-bGreenWantedColor)+100*bGreenWantedColor) / 100);
									bRedDst    = (BYTE)((bOpa*(bRedDst  -bRedWantedColor  )+100*bRedWantedColor  ) / 100);

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bRedDst  <<m_stBitsMaskInfo.bRShift) |
																									      (bGreenDst<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueDst <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
							else
							{
								for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);


									wPixel  = pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)];
									bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									bBlueDst   = (BYTE)((bOpa*(bBlueDst -bBlueWantedColor )+100*bBlueWantedColor ) / 100);
									bGreenDst  = (BYTE)((bOpa*(bGreenDst-bGreenWantedColor)+100*bGreenWantedColor) / 100);
									bRedDst    = (BYTE)((bOpa*(bRedDst  -bRedWantedColor  )+100*bRedWantedColor  ) / 100);

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bRedDst  <<m_stBitsMaskInfo.bRShift) |
																									      (bGreenDst<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueDst <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
						}
					}	
				}
				// 将本行的结尾移到下一行的开头.
				nWidthEnd++;

				nWidthStart	= nWidthEnd;
				nCurrWidth = 0;
			}
		}

		m_pddsBackBuffer->Unlock(NULL);
		return TRUE;
	}
	return FALSE;
}


// nX, nY 是中心点的坐标.
BOOL CWHDXGraphicWindow::DrawWithShadowABlend(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight,
											  WORD* pwShadowClrSrc, BOOL bBlend, BYTE bShadowType, BYTE bOpa)

{
	// 不绘制的阴影.
	if ( bShadowType == 48 )
		return FALSE;

	RECT			rc;
	DDSURFACEDESC2	ddsd;

	INT	nWidth		= nXSize;
	INT	nHeight		= nYSize;
	INT	nXOffset	= 0;
	INT	nYOffset	= 0;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= wClipWidth -1;
	INT	nEndY		= wClipHeight-1;


	if ( m_pddsBackBuffer != NULL )
	{
		rc.left		= 0;
		rc.right	= nWidth;
		rc.top		= 0;
		rc.bottom	= nHeight;

		ddsd.dwSize	= sizeof(DDSURFACEDESC2);
		ddsd.lpSurface = NULL;
			
		m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
		if ( !ddsd.lpSurface )			return FALSE;
		WORD* pwdDst;
		pwdDst = (WORD*)ddsd.lpSurface;

		INT nWidthStart	= 0;
		INT nWidthEnd	= 0;
		INT nCurrWidth  = 0;
		INT nCntCopyWord = 0;
		INT nYCnt =0;
		INT nLastWidth = 0;
		INT nDstXPos = 0;
		INT nDstYPos = 0;

		WORD wPixel;
		BYTE bBlueWantedColor, bGreenWantedColor, bRedWantedColor, bBlueDst, bGreenDst, bRedDst;
		RECT rcScrnShadow;
		WORD wSahdowColor = pwShadowClrSrc[0];

		if ( bShadowType == 49 )
		{
			SetRect(&rcScrnShadow, nX, nY, nX+nHeight/2+nWidth, nY+nHeight/2);
		}
		else if ( bShadowType == 50 )
		{
			SetRect(&rcScrnShadow, nX, nY, nX+nWidth, nY+nHeight);
		}

		// 只决定对 y 轴实际进行循环的 Count.
		for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
		{
			// 获取一行的长度.(单位为字)
			nWidthEnd += pwSrc[nWidthStart];
			nWidthStart++;

			if ( bShadowType == 49 )
			{
				nDstYPos = nYCnt-nYCnt/2+nY;
			}
			else if ( bShadowType == 50 )
			{
				nDstYPos = nYCnt+nY;
			}

			if ( nDstYPos < 0 || nDstYPos >= wClipHeight || (bShadowType==49 && nYCnt%2) )
			{
				// 将本行的结尾移到下一行的开头.
				nWidthEnd++;
				nWidthStart	= nWidthEnd;
				nCurrWidth = 0;
				continue;
			}

			// 按行的长度输出到屏幕。
			for ( INT x = nWidthStart; x < nWidthEnd ; )
			{
				if ( pwSrc[x] == 0xC0 )
				{
					x++;
					nCntCopyWord = pwSrc[x];
					x++;
					nCurrWidth += nCntCopyWord;
				}
				else if ( pwSrc[x] == 0xC1 || pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
				{
					x++;
					nCntCopyWord = pwSrc[x];
					x++;

					nLastWidth = nCurrWidth;
					nCurrWidth += nCntCopyWord;

					if ( bShadowType == 49 )
					{
						nDstXPos = (nLastWidth+nX+(rc.bottom-nYCnt)/2);
					}
					else if ( bShadowType == 50 )
					{
						nDstXPos = (nLastWidth+nX);
					}

					if ( !bBlend ) 
					{
						// 左侧碰撞时.
						if ( nDstXPos < 0 && nDstXPos+nCntCopyWord >= 0 )
						{
							for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
							{
								if ( nDstXPos+nCheck >= 0 && nDstXPos+nCheck < wClipWidth )
								{
									pwdDst[(nDstYPos * (ddsd.lPitch >> 1)) + nDstXPos+nCheck] = wSahdowColor;

								}
							}
							x += nCntCopyWord;
						}
						// 在右侧卡住时。
						else if ( nDstXPos < wClipWidth && nDstXPos+nCntCopyWord >= wClipWidth )
						{
							for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
							{
								if ( nDstXPos+nCheck >= 0 && nDstXPos+nCheck < wClipWidth )
								{
									pwdDst[(nDstYPos * (ddsd.lPitch >> 1)) + nDstXPos+nCheck] = wSahdowColor;

								}
							}
							x += nCntCopyWord;
						}
						// 未被碰到时.
						else if ( nDstXPos >= 0 && nDstXPos+nCntCopyWord < wClipWidth )
						{
							memcpy(&pwdDst[(nDstYPos * (ddsd.lPitch >> 1)) + nDstXPos], pwShadowClrSrc, sizeof(WORD)*nCntCopyWord);
//							memset(&pwdDst[(nDstYPos * (ddsd.lPitch >> 1)) + nDstXPos], wSahdowColor, sizeof(WORD)*nCntCopyWord);
							x += nCntCopyWord;
						}
						else
						{
							x += nCntCopyWord;
						}
					}
					else
					{
						// 左侧碰撞时.
						if ( nDstXPos < 0 && nDstXPos+nCntCopyWord >= 0 )
						{
							for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
							{
								if ( nDstXPos+nCheck >= 0 && nDstXPos+nCheck < wClipWidth )
								{
//									pwdDst[(nDstYPos * (ddsd.lPitch >> 1)) + nDstXPos+nCheck] = wSahdowColor;
									wPixel = wSahdowColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel  = pwdDst[(nDstYPos * (ddsd.lPitch >> 1)) + nDstXPos+nCheck];
									bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									bBlueDst   = (BYTE)((bOpa*(bBlueDst -bBlueWantedColor)+100*bBlueWantedColor  ) / 100);
									bGreenDst  = (BYTE)((bOpa*(bGreenDst-bGreenWantedColor)+100*bGreenWantedColor) / 100);
									bRedDst    = (BYTE)((bOpa*(bRedDst  -bRedWantedColor)+100*bRedWantedColor    ) / 100);

									pwdDst[(nDstYPos * (ddsd.lPitch >> 1)) + nDstXPos+nCheck] = ((bRedDst  <<m_stBitsMaskInfo.bRShift) |
																						  		 (bGreenDst<<m_stBitsMaskInfo.bGShift) |
																								 (bBlueDst <<m_stBitsMaskInfo.bBShift));							
								}
							}
							x += nCntCopyWord;
						}
						// 在右侧卡住时。
						else if ( nDstXPos < wClipWidth && nDstXPos+nCntCopyWord >= wClipWidth )
						{
							for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
							{
								if ( nDstXPos+nCheck >= 0 && nDstXPos+nCheck < wClipWidth )
								{
//									pwdDst[(nDstYPos * (ddsd.lPitch >> 1)) + nDstXPos+nCheck] = wSahdowColor;

									WORD wPixel = wSahdowColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel  = pwdDst[(nDstYPos * (ddsd.lPitch >> 1)) + nDstXPos+nCheck];
									bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									bBlueDst   = (BYTE)((bOpa*(bBlueDst -bBlueWantedColor)+100*bBlueWantedColor  ) / 100);
									bGreenDst  = (BYTE)((bOpa*(bGreenDst-bGreenWantedColor)+100*bGreenWantedColor) / 100);
									bRedDst    = (BYTE)((bOpa*(bRedDst  -bRedWantedColor)+100*bRedWantedColor    ) / 100);

									pwdDst[(nDstYPos * (ddsd.lPitch >> 1)) + nDstXPos+nCheck] = ((bRedDst  <<m_stBitsMaskInfo.bRShift) |
																						  		 (bGreenDst<<m_stBitsMaskInfo.bGShift) |
																								 (bBlueDst <<m_stBitsMaskInfo.bBShift));							

								}
							}
							x += nCntCopyWord;
						}
						// 未被碰到时.
						else if ( nDstXPos >= 0 && nDstXPos+nCntCopyWord < wClipWidth )
						{
	//						memset(&pwdDst[(nDstYPos * (ddsd.lPitch >> 1)) + nDstXPos], wSahdowColor, sizeof(WORD)*nCntCopyWord);
	//						x += nCntCopyWord;

							for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
							{
	//							pwdDst[(nDstYPos * (ddsd.lPitch >> 1)) + nDstXPos+nCheck] = wSahdowColor;

								wPixel = wSahdowColor;
								bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
								bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
								bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

								wPixel  = pwdDst[(nDstYPos * (ddsd.lPitch >> 1)) + nDstXPos+nCheck];
								bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
								bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
								bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

								bBlueDst   = (BYTE)((bOpa*(bBlueDst -bBlueWantedColor)+100*bBlueWantedColor  ) / 100);
								bGreenDst  = (BYTE)((bOpa*(bGreenDst-bGreenWantedColor)+100*bGreenWantedColor) / 100);
								bRedDst    = (BYTE)((bOpa*(bRedDst  -bRedWantedColor)+100*bRedWantedColor    ) / 100);

								pwdDst[(nDstYPos * (ddsd.lPitch >> 1)) + nDstXPos+nCheck] = ((bRedDst  <<m_stBitsMaskInfo.bRShift) |
																							 (bGreenDst<<m_stBitsMaskInfo.bGShift) |
																							 (bBlueDst <<m_stBitsMaskInfo.bBShift));							
							}
							x += nCntCopyWord;
						}
						else
						{
							x += nCntCopyWord;
						}
					}
				}
			}
			// 将本行的结尾移到下一行的开头.
			nWidthEnd++;

			nWidthStart	= nWidthEnd;
			nCurrWidth = 0;
		}
			
		m_pddsBackBuffer->Unlock(NULL);
		return TRUE;
	}
	return FALSE;
}



VOID CWHDXGraphicWindow::DrawWithShadowABlend(INT nX, INT nY, INT nXSize, INT nYSize, INT nPX, INT nPY, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight,
											  WORD* pwShadowClrSrc, BOOL bBlend, BYTE bOpa)
{
	DDSURFACEDESC2	ddsd;

#define _HALF_CELL_WIDTH	24
#define _HALF_CELL_HEIGHT	16

//  求屏幕(800*600)上阴影绘制的大小.
	
	// 不是画面(800*600)中的单元格起始坐标，而是实际的中心点坐标(单元格正中央点。)
	INT		nScrnCenterX = nX - nPX + 24;//_HALF_CELL_WIDTH;
	INT		nScrnCenterY = nY - nPY + 16;//_HALF_CELL_HEIGHT;
	INT		nRealCenterX = nX - nScrnCenterX;
	INT		nRealCenterY = nY - nScrnCenterY;

	INT		nShadowCenterX = nX;
	INT		nShadowCenterY = nY-nRealCenterY/2;

	RECT	rcSrc		 = {0, 0, nXSize, nYSize};

	BYTE bRedDst, bGreenDst, bBlueDst;
	BYTE bRedWantedColor, bGreenWantedColor, bBlueWantedColor;
	WORD wPixel;
	WORD wSahdowColor = pwShadowClrSrc[0];

	if ( m_pddsBackBuffer != NULL )
	{
		ddsd.dwSize	= sizeof(DDSURFACEDESC2);
		ddsd.lpSurface = NULL;

		m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
		if ( !ddsd.lpSurface )			return;
		WORD* pwdDst = NULL;
		pwdDst = (WORD*)ddsd.lpSurface;

		if ( pwdDst )
		{
			INT nWidthStart	 = 0;
			INT nWidthEnd	 = 0;
			INT nCurrWidth   = 0;
			INT nLastWidth	 = 0;
			INT nCntCopyWord = 0;
			INT nYCnt		 = 0;

			INT nDstYPos = nShadowCenterY;

			for ( nYCnt=rcSrc.top ; nYCnt < rcSrc.bottom ; nYCnt++ )
			{
				if ( nDstYPos >= wClipHeight )
				{
					m_pddsBackBuffer->Unlock(NULL);
					return;
				}
				// y 轴裁剪以及逐行跳过.
				if ( nYCnt%2 == 0 ||  nDstYPos < 0 )
				{
					nWidthEnd += pwSrc[nWidthStart];
					nWidthStart++;
					nWidthEnd++;
					nWidthStart	= nWidthEnd;

					nDstYPos++;
				}
				else
				{
					// 获取一行的长度.(单位为字)
					nWidthEnd += pwSrc[nWidthStart];
					nWidthStart++;

					// 按行的长度输出到屏幕。
					for ( INT x = nWidthStart; x < nWidthEnd ; )
					{
						if ( pwSrc[x] == 0xC0 )
						{		
							x++;
							nCntCopyWord = pwSrc[x];
							x++;
							nCurrWidth += nCntCopyWord;
						}
						else
						{
							x++;
							nCntCopyWord = pwSrc[x];
							x++;
							nLastWidth = nCurrWidth;
							nCurrWidth += nCntCopyWord;

							if ( bBlend )
							{
								if ((nShadowCenterX+nLastWidth+((-nRealCenterY/2)-nYCnt/2)) < 0 && (nShadowCenterX+nCurrWidth+((-nRealCenterY/2)-nYCnt/2)) >= 0 )
								{
								}
								else if ((nShadowCenterX+nLastWidth+((-nRealCenterY/2)-nYCnt/2)) < wClipWidth && (nShadowCenterX+nCurrWidth+((-nRealCenterY/2)-nYCnt/2)) >= wClipWidth )
								{
								}
								else if ( (nShadowCenterX+nCurrWidth+((-nRealCenterY/2)-nYCnt/2)) < 0 )
								{
								}
								else if ( (nShadowCenterX+nCurrWidth+((-nRealCenterY/2)-nYCnt/2)) >= wClipWidth )
								{
								}
								else
								{
									for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
									{
										wPixel = wSahdowColor;
										bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
										bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
										bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

										wPixel  = pwdDst[nDstYPos*(ddsd.lPitch >> 1) + (nShadowCenterX+nLastWidth+((-nRealCenterY/2)-nYCnt/2)) + nCheck];
										bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
										bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
										bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

										bBlueDst   = (BYTE)((bOpa*(bBlueDst -bBlueWantedColor)+100*bBlueWantedColor  ) / 100);
										bGreenDst  = (BYTE)((bOpa*(bGreenDst-bGreenWantedColor)+100*bGreenWantedColor) / 100);
										bRedDst    = (BYTE)((bOpa*(bRedDst  -bRedWantedColor)+100*bRedWantedColor    ) / 100);

										pwdDst[nDstYPos*(ddsd.lPitch >> 1) + (nShadowCenterX+nLastWidth+((-nRealCenterY/2)-nYCnt/2)+nCheck)] = ((bRedDst  <<m_stBitsMaskInfo.bRShift) |
																																				(bGreenDst<<m_stBitsMaskInfo.bGShift) |
																																				(bBlueDst <<m_stBitsMaskInfo.bBShift));							}
								}
							}
							else
							{
								if ((nShadowCenterX+nLastWidth+((-nRealCenterY/2)-nYCnt/2)) < 0 && (nShadowCenterX+nCurrWidth+((-nRealCenterY/2)-nYCnt/2)) >= 0 )
								{
								}
								else if ((nShadowCenterX+nLastWidth+((-nRealCenterY/2)-nYCnt/2)) < wClipWidth && (nShadowCenterX+nCurrWidth+((-nRealCenterY/2)-nYCnt/2)) >= wClipWidth )
								{
								}
								else if ( (nShadowCenterX+nCurrWidth+((-nRealCenterY/2)-nYCnt/2)) < 0 )
								{
								}
								else if ( (nShadowCenterX+nCurrWidth+((-nRealCenterY/2)-nYCnt/2)) >= wClipWidth )
								{
								}
								else
								{
//									memset(&pwdDst[nDstYPos*(ddsd.lPitch >> 1) + (nShadowCenterX+nLastWidth+((-nRealCenterY/2)-nYCnt/2))], wSahdowColor, sizeof(WORD)*nCntCopyWord);
									memcpy(&pwdDst[nDstYPos*(ddsd.lPitch >> 1) + (nShadowCenterX+nLastWidth+((-nRealCenterY/2)-nYCnt/2))], pwShadowClrSrc, sizeof(WORD)*nCntCopyWord);
								}
							}
							x += nCntCopyWord;
						}
					}
					// 将本行的结尾移到下一行的开头.
					nWidthEnd++;

					nWidthStart	= nWidthEnd;
					nCurrWidth = 0;
				}
			}
			m_pddsBackBuffer->Unlock(NULL);
		}
	}
}


BOOL CWHDXGraphicWindow::DrawWithABlendCompDataWithLightedColor(INT nX, INT nY, 
															    INT nXSize, INT nYSize, WORD* pwSrc,
															    WORD wClipWidth, WORD wClipHeight,
															    WORD wChooseColor1, WORD wChooseColor2)
{
	RECT			rc;
	DDSURFACEDESC2	ddsd;

	INT	nWidth		= nXSize;
	INT	nHeight		= nYSize;
	INT	nXOffset	= 0;
	INT	nYOffset	= 0;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= wClipWidth -1;
	INT	nEndY		= wClipHeight-1;

	FLOAT fBright = 0.3f;

	if ( m_pddsBackBuffer != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset	= nStartX - nX;
			nWidth		= nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth		= nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )	
		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}		
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset+nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset+nHeight;

			ddsd.dwSize	= sizeof(DDSURFACEDESC2);
			ddsd.lpSurface = NULL;
			
			m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
			if ( !ddsd.lpSurface )			return FALSE;
			WORD* pwdDst;
			pwdDst = (WORD*)ddsd.lpSurface;

			INT nWidthStart	= 0;
			INT nWidthEnd	= 0;
			INT nCurrWidth  = 0;
			INT nCntCopyWord = 0;
			INT nYCnt =0;
			INT nLastWidth = 0;

			BYTE  bRedWantedColor, bGreenWantedColor, bBlueWantedColor;
			BYTE bRedSrc, bGreenSrc, bBlueSrc;
			WORD wPixel;
			WORD wDyingKind, wChooseColor;
			FLOAT rBlueRate, rGreenRate, bRedRate;

			// y 轴裁剪。
  			for ( nYCnt=0 ; nYCnt < rc.top ; nYCnt++ )
			{
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;
				nWidthEnd++;
				nWidthStart	= nWidthEnd;
			}

			// 只决定对 y 轴实际进行循环的 Count.
			for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
			{
				// 获取一行的长度.(单位为字)
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;

				// 按行的长度输出到屏幕。
				for ( INT x = nWidthStart; x < nWidthEnd ; )
				{
					if ( pwSrc[x] == 0xC0 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;
						nCurrWidth += nCntCopyWord;
					}
					else if ( pwSrc[x] == 0xC1 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// 以 rc.left 为基准,复制区域被截取的情况.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								for ( INT nCheck = 0; nCheck < nCurrWidth-rc.left; nCheck++ )
								{
									wPixel = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueSrc = (BYTE)((FLOAT)bBlueSrc + ((FLOAT)bBlueSrc*rBlueRate*fBright));
									bGreenSrc = (BYTE)((FLOAT)bGreenSrc + ((FLOAT)bGreenSrc*rGreenRate*fBright));
									bRedSrc = (BYTE)((FLOAT)bRedSrc + ((FLOAT)bRedSrc*bRedRate*fBright));

									if ( bBlueSrc > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueSrc = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenSrc > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenSrc = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedSrc > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedSrc = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (rc.left+nX+nCheck)] = ((bBlueSrc <<m_stBitsMaskInfo.bBShift) |
																									   (bGreenSrc<<m_stBitsMaskInfo.bGShift) |
																									   (bRedSrc  <<m_stBitsMaskInfo.bRShift));			
								}
								x += (nCurrWidth-rc.left);
							}
							// 以 rc.right 点为基准，复制区域被截断的情况.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for ( INT nCheck = 0; nCheck < rc.right-nLastWidth; nCheck++ )
								{
									wPixel = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueSrc = (BYTE)((FLOAT)bBlueSrc + ((FLOAT)bBlueSrc*rBlueRate*fBright));
									bGreenSrc = (BYTE)((FLOAT)bGreenSrc + ((FLOAT)bGreenSrc*rGreenRate*fBright));
									bRedSrc = (BYTE)((FLOAT)bRedSrc + ((FLOAT)bRedSrc*bRedRate*fBright));

									if ( bBlueSrc > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueSrc = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenSrc > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenSrc = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedSrc > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedSrc = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bBlueSrc <<m_stBitsMaskInfo.bRShift) |
																									      (bGreenSrc<<m_stBitsMaskInfo.bGShift) |
																										  (bRedSrc  <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
							else
							{
								for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
								{
									wPixel = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueSrc = (BYTE)((FLOAT)bBlueSrc + ((FLOAT)bBlueSrc*rBlueRate*fBright));
									bGreenSrc = (BYTE)((FLOAT)bGreenSrc + ((FLOAT)bGreenSrc*rGreenRate*fBright));
									bRedSrc = (BYTE)((FLOAT)bRedSrc + ((FLOAT)bRedSrc*bRedRate*fBright));

									if ( bBlueSrc > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueSrc = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenSrc > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenSrc = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedSrc > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedSrc = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bBlueSrc <<m_stBitsMaskInfo.bBShift) |
																									      (bGreenSrc<<m_stBitsMaskInfo.bGShift) |
																										  (bRedSrc  <<m_stBitsMaskInfo.bRShift));			
								}
								x += nCntCopyWord;
							}
						}
					}
					else if ( pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
					{
						wDyingKind = pwSrc[x];
						switch ( wDyingKind )
						{
						case 0xC2: 
							wChooseColor = wChooseColor1;
							break;
						case 0xC3: 
							wChooseColor = wChooseColor2;
							break;
						}
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// 以 rc.left 为基准,复制区域被截取的情况.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								for ( INT nCheck = 0; nCheck < nCurrWidth-rc.left; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									bBlueSrc = bBlueWantedColor;
									bGreenSrc = bGreenWantedColor;
									bRedSrc = bRedWantedColor;

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueSrc = (BYTE)((FLOAT)bBlueSrc + ((FLOAT)bBlueSrc*rBlueRate*fBright));
									bGreenSrc = (BYTE)((FLOAT)bGreenSrc + ((FLOAT)bGreenSrc*rGreenRate*fBright));
									bRedSrc = (BYTE)((FLOAT)bRedSrc + ((FLOAT)bRedSrc*bRedRate*fBright));

									if ( bBlueSrc > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueSrc = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenSrc > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenSrc = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedSrc > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedSrc = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (rc.left+nX+nCheck)] = ((bBlueSrc <<m_stBitsMaskInfo.bBShift) |
																									   (bGreenSrc<<m_stBitsMaskInfo.bGShift) |
																									   (bRedSrc  <<m_stBitsMaskInfo.bRShift));			
								}
								x += (nCurrWidth-rc.left);
							}
							// 以 rc.right 点为基准，复制区域被截断的情况.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for ( INT nCheck = 0; nCheck < rc.right-nLastWidth; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									bBlueSrc = bBlueWantedColor;
									bGreenSrc = bGreenWantedColor;
									bRedSrc = bRedWantedColor;

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueSrc = (BYTE)((FLOAT)bBlueSrc + ((FLOAT)bBlueSrc*rBlueRate*fBright));
									bGreenSrc = (BYTE)((FLOAT)bGreenSrc + ((FLOAT)bGreenSrc*rGreenRate*fBright ));
									bRedSrc = (BYTE)((FLOAT)bRedSrc + ((FLOAT)bRedSrc*bRedRate*fBright));

									if ( bBlueSrc > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueSrc = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenSrc > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenSrc = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedSrc > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedSrc = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bBlueSrc <<m_stBitsMaskInfo.bRShift) |
																									      (bGreenSrc<<m_stBitsMaskInfo.bGShift) |
																										  (bRedSrc  <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
							else
							{
								for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									bBlueSrc = bBlueWantedColor;
									bGreenSrc = bGreenWantedColor;
									bRedSrc = bRedWantedColor;

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueSrc = (BYTE)((FLOAT)bBlueSrc + ((FLOAT)bBlueSrc*rBlueRate*fBright));
									bGreenSrc = (BYTE)((FLOAT)bGreenSrc + ((FLOAT)bGreenSrc*rGreenRate*fBright));
									bRedSrc = (BYTE)((FLOAT)bRedSrc + ((FLOAT)bRedSrc*bRedRate*fBright));

									if ( bBlueSrc > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueSrc = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenSrc > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenSrc = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedSrc > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedSrc = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bBlueSrc <<m_stBitsMaskInfo.bBShift) |
																									      (bGreenSrc<<m_stBitsMaskInfo.bGShift) |
																										  (bRedSrc  <<m_stBitsMaskInfo.bRShift));			
								}
							x += nCntCopyWord;
							}
						}
					}	
				}
				// 将本行的结尾移到下一行的开头.
				nWidthEnd++;

				nWidthStart	= nWidthEnd;
				nCurrWidth = 0;
			}
		}

		m_pddsBackBuffer->Unlock(NULL);
		return TRUE;
	}
	return FALSE;
}



BOOL CWHDXGraphicWindow::DrawWithImageForCompClipRgnColor(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight, WORD wColor, BOOL bFocused, BOOL bBlend)
{
	RECT			rc;
	DDSURFACEDESC2	ddsd;

	INT	nWidth		= nXSize;
	INT	nHeight		= nYSize;
	INT	nXOffset	= 0;
	INT	nYOffset	= 0;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= wClipWidth -1;
	INT	nEndY		= wClipHeight-1;

	if ( m_pddsBackBuffer != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset	= nStartX - nX;
			nWidth		= nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth		= nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )	
		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}		
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset+nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset+nHeight;

			ddsd.dwSize	= sizeof(DDSURFACEDESC2);
			ddsd.lpSurface = NULL;
			
			m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
			if ( !ddsd.lpSurface )			return FALSE;
			WORD* pwdDst;
			pwdDst = (WORD*)ddsd.lpSurface;

			INT nWidthStart	= 0;
			INT nWidthEnd	= 0;
			INT nCurrWidth  = 0;
			INT nCntCopyWord = 0;
			INT nYCnt =0;
			INT nLastWidth = 0;

			BYTE  bRedStateColor,  bGreenStateColor,  bBlueStateColor;
			BYTE  bRedSrc, bGreenSrc, bBlueSrc;
			BYTE  bBlueDst, bGreenDst, bRedDst;
			WORD  wPixel;
			FLOAT rBlueRate, rGreenRate, rRedRate;
			FLOAT rbLightRate = 0.0f;

			// y 轴裁剪。
  			for ( nYCnt=0 ; nYCnt < rc.top ; nYCnt++ )
			{
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;
				nWidthEnd++;
				nWidthStart	= nWidthEnd;
			}

			// 只决定对 y 轴实际进行循环的 Count.
			for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
			{
				// 获取一行的长度.(单位为字)
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;

				// 按行的长度输出到屏幕。
				for ( INT x = nWidthStart; x < nWidthEnd ; )
				{
					if ( pwSrc[x] == 0xC0 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;
						nCurrWidth += nCntCopyWord;
					}
					else if ( pwSrc[x] == 0xC1 || pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// 以 rc.left 为基准,复制区域被截取的情况.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								for ( INT nCheck = 0; nCheck < nCurrWidth-rc.left; nCheck++ )
								{
									wPixel = wColor;
									bBlueStateColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenStateColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedStateColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									// 是否 Focus.
									if ( bFocused )			rbLightRate = 0.6f;
									else					rbLightRate = 0.0f;

									if ( !wColor )	
									{
										BYTE bTemp = (BYTE)(bBlueSrc+(bGreenSrc>>1)+bRedSrc)/3;
										bTemp = bTemp + (BYTE)(bTemp*rbLightRate);
										if ( bTemp > 31 )	bTemp = 31;
										bBlueStateColor = bRedStateColor = bTemp;
										bGreenStateColor = bTemp<<1;
									}
									else
									{
										rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
										rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
										rRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

										bBlueStateColor  = ( (BYTE)((FLOAT)bBlueStateColor *rBlueRate ) + (BYTE)((FLOAT)bBlueStateColor *rBlueRate *rbLightRate) );
										bGreenStateColor = ( (BYTE)((FLOAT)bGreenStateColor*rGreenRate) + (BYTE)((FLOAT)bGreenStateColor*rGreenRate*rbLightRate) );
										bRedStateColor   = ( (BYTE)((FLOAT)bRedStateColor  *rRedRate  ) + (BYTE)((FLOAT)bRedStateColor  *rRedRate  *rbLightRate) );					

										if ( bBlueStateColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
											bBlueStateColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
										if ( bGreenStateColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
											bGreenStateColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
										if ( bRedStateColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
											bRedStateColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);
									}

									if ( bBlend )
									{
										wPixel	   = pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)];
										bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
										bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
										bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

										bBlueDst   = (BYTE)((50*(bBlueDst -bBlueStateColor )+100*bBlueStateColor  ) / 100);
										bGreenDst  = (BYTE)((50*(bGreenDst-bGreenStateColor)+100*bGreenStateColor ) / 100);
										bRedDst    = (BYTE)((50*(bRedDst  -bRedStateColor  )+100*bRedStateColor   ) / 100);
									}
									else
									{
										bBlueDst   = bBlueStateColor;
										bGreenDst  = bGreenStateColor;
										bRedDst    = bRedStateColor;
									}

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (rc.left+nX+nCheck)] = ((bBlueDst <<m_stBitsMaskInfo.bBShift) |
																									   (bGreenDst<<m_stBitsMaskInfo.bGShift) |
																									   (bRedDst  <<m_stBitsMaskInfo.bRShift));			
								}
								x += (nCurrWidth-rc.left);
							}
							// 以 rc.right 点为基准，复制区域被截断的情况.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for ( INT nCheck = 0; nCheck < rc.right-nLastWidth; nCheck++ )
								{
									wPixel = wColor;
									bBlueStateColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenStateColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedStateColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									// 是否 Focus.
									if ( bFocused )			rbLightRate = 0.6f;
									else					rbLightRate = 0.0f;

									if ( !wColor )	
									{
										BYTE bTemp = (BYTE)(bBlueSrc+(bGreenSrc>>1)+bRedSrc)/3;
										bTemp = bTemp + (BYTE)(bTemp*rbLightRate);
										if ( bTemp > 31 )	bTemp = 31;
										bBlueStateColor = bRedStateColor = bTemp;
										bGreenStateColor = bTemp<<1;
									}
									else
									{
										rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
										rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
										rRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

										bBlueStateColor  = ( (BYTE)((FLOAT)bBlueStateColor *rBlueRate ) + (BYTE)((FLOAT)bBlueStateColor *rBlueRate *rbLightRate) );
										bGreenStateColor = ( (BYTE)((FLOAT)bGreenStateColor*rGreenRate) + (BYTE)((FLOAT)bGreenStateColor*rGreenRate*rbLightRate) );
										bRedStateColor   = ( (BYTE)((FLOAT)bRedStateColor  *rRedRate  ) + (BYTE)((FLOAT)bRedStateColor  *rRedRate  *rbLightRate) );					

										if ( bBlueStateColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
											bBlueStateColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
										if ( bGreenStateColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
											bGreenStateColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
										if ( bRedStateColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
											bRedStateColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);
									}

									if ( bBlend )
									{
										wPixel	   = pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)];
										bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
										bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
										bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

										bBlueDst   = (BYTE)((50*(bBlueDst -bBlueStateColor )+100*bBlueStateColor  ) / 100);
										bGreenDst  = (BYTE)((50*(bGreenDst-bGreenStateColor)+100*bGreenStateColor ) / 100);
										bRedDst    = (BYTE)((50*(bRedDst  -bRedStateColor  )+100*bRedStateColor   ) / 100);
									}
									else
									{
										bBlueDst   = bBlueStateColor;
										bGreenDst  = bGreenStateColor;
										bRedDst    = bRedStateColor;
									}

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bBlueDst <<m_stBitsMaskInfo.bRShift) |
																									      (bGreenDst<<m_stBitsMaskInfo.bGShift) |
																										  (bRedDst  <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
							else
							{
								for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
								{
									wPixel = wColor;
									bBlueStateColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenStateColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedStateColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									// 是否 Focus.
									if ( bFocused )			rbLightRate = 0.6f;
									else					rbLightRate = 0.0f;

									if ( !wColor )	
									{
										BYTE bTemp = (BYTE)(bBlueSrc+(bGreenSrc>>1)+bRedSrc)/3;
										bTemp = bTemp + (BYTE)(bTemp*rbLightRate);
										if ( bTemp > 31 )	bTemp = 31;
										bBlueStateColor = bRedStateColor = bTemp;
										bGreenStateColor = bTemp<<1;
									}
									else
									{
										rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
										rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
										rRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

										bBlueStateColor  = ( (BYTE)((FLOAT)bBlueStateColor *rBlueRate ) + (BYTE)((FLOAT)bBlueStateColor *rBlueRate *rbLightRate) );
										bGreenStateColor = ( (BYTE)((FLOAT)bGreenStateColor*rGreenRate) + (BYTE)((FLOAT)bGreenStateColor*rGreenRate*rbLightRate) );
										bRedStateColor   = ( (BYTE)((FLOAT)bRedStateColor  *rRedRate  ) + (BYTE)((FLOAT)bRedStateColor  *rRedRate  *rbLightRate) );					

										if ( bBlueStateColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
											bBlueStateColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
										if ( bGreenStateColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
											bGreenStateColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
										if ( bRedStateColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
											bRedStateColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);
									}

									if ( bBlend )
									{
										wPixel	   = pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)];
										bBlueDst   = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
										bGreenDst  = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
										bRedDst    = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

										bBlueDst   = (BYTE)((50*(bBlueDst -bBlueStateColor )+100*bBlueStateColor  ) / 100);
										bGreenDst  = (BYTE)((50*(bGreenDst-bGreenStateColor)+100*bGreenStateColor ) / 100);
										bRedDst    = (BYTE)((50*(bRedDst  -bRedStateColor  )+100*bRedStateColor   ) / 100);
									}
									else
									{
										bBlueDst   = bBlueStateColor;
										bGreenDst  = bGreenStateColor;
										bRedDst    = bRedStateColor;
									}

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bBlueDst <<m_stBitsMaskInfo.bBShift) |
																									      (bGreenDst<<m_stBitsMaskInfo.bGShift) |
																										  (bRedDst  <<m_stBitsMaskInfo.bRShift));			
								}
								x += nCntCopyWord;
							}
						}
					}
				}
				// 将本行的结尾移到下一行的开头.
				nWidthEnd++;

				nWidthStart	= nWidthEnd;
				nCurrWidth = 0;
			}
		}

		m_pddsBackBuffer->Unlock(NULL);
		return TRUE;
	}
	return FALSE;
}



BOOL CWHDXGraphicWindow::DrawWithImageForCompClipRgnColor(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight,WORD wColor , WORD wChooseColor1, WORD wChooseColor2)
{
	RECT			rc;
	DDSURFACEDESC2	ddsd;

	INT	nWidth		= nXSize;
	INT	nHeight		= nYSize;
	INT	nXOffset	= 0;
	INT	nYOffset	= 0;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= wClipWidth -1;
	INT	nEndY		= wClipHeight-1;

	INT FiveMiddle	= 0xf;
	INT SixMiddle	= 0x1f;

	BYTE bsBlueColor,bsGreenColor,bsRedColor;
	BYTE bdBlueColor,bdGreenColor,bdRedColor;

	bdBlueColor  = (BYTE)((wColor & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
	bdGreenColor = (BYTE)((wColor & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
	bdRedColor   = (BYTE)((wColor & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

	if ( m_pddsBackBuffer != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset	= nStartX - nX;
			nWidth		= nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth		= nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )	
		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}		
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset+nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset+nHeight;

			ddsd.dwSize	= sizeof(DDSURFACEDESC2);
			ddsd.lpSurface = NULL;
			
			m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
			if ( !ddsd.lpSurface )			return FALSE;
			WORD* pwdDst;
			pwdDst = (WORD*)ddsd.lpSurface;

			INT nWidthStart	= 0;
			INT nWidthEnd	= 0;
			INT nCurrWidth  = 0;
			INT nCntCopyWord = 0;
			INT nYCnt =0;
			INT nLastWidth = 0;

			FLOAT rBlueRate, rGreenRate, bRedRate;
			BYTE  bRedSrc, bGreenSrc, bBlueSrc;
			BYTE  bRedWantedColor, bGreenWantedColor, bBlueWantedColor;
			WORD  wPixel;

			// y 轴裁剪。
			for ( nYCnt=0 ; nYCnt < rc.top ; nYCnt++ )
			{
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;
				nWidthEnd++;
				nWidthStart	= nWidthEnd;
			}

			// 只决定对 y 轴实际进行循环的 Count.
			for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
			{
				// 获取一行的长度.(单位为字)
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;

				// 按行的长度输出到屏幕。
				for ( INT x = nWidthStart; x < nWidthEnd ; )
				{
					if ( pwSrc[x] == 0xC0 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;
						nCurrWidth += nCntCopyWord;
					}
					else if ( pwSrc[x] == 0xC1 )
					{
						x++;
						nCntCopyWord = pwSrc[x];	// 个数
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;		// 不合情理的情况
						}
						else
						{
							// 以 rc.left 为基准,复制区域被截取的情况.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);

								for(WORD i = 0 ; i < (sizeof(WORD)*(nCurrWidth-rc.left)/2);i++)
								{
									bsBlueColor  = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bsGreenColor = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bsRedColor   = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									BYTE	temp;

									temp = (bsRedColor+(bsGreenColor>>1)+bsBlueColor)/3;

									bsBlueColor = temp>0 ? (temp&bdBlueColor) : 0;
									bsGreenColor = temp>0 ? (temp&(bdGreenColor<<1)) : 0;
									bsRedColor = temp>0 ? (temp&bdRedColor) :  0;

									if ( bsBlueColor >= 32 )  bsBlueColor = 31;
									if ( bsGreenColor >= 64 )  bsGreenColor = 63;
									if ( bsRedColor >= 32 )  bsRedColor = 31;

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (rc.left+nX) + i] = (bsBlueColor<<m_stBitsMaskInfo.bBShift) |  (bsGreenColor<<m_stBitsMaskInfo.bGShift) | (bsRedColor<<m_stBitsMaskInfo.bRShift);
								}

								x += (nCurrWidth-rc.left);
							}
							// 以 rc.right 点为基准，复制区域被截断的情况.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for(WORD i = 0 ; i < (sizeof(WORD)*(rc.right-nLastWidth));i++)
								{
									bsBlueColor  = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bsGreenColor = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bsRedColor   = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);
									
									BYTE	temp;

									temp = (bsRedColor+(bsGreenColor>>1)+bsBlueColor)/3;

									bsBlueColor = temp>0 ? (temp&bdBlueColor) : 0;
									bsGreenColor = temp>0 ? (temp&(bdGreenColor<<1)) : 0;
									bsRedColor = temp>0 ? (temp&bdRedColor) :  0;

									if ( bsBlueColor >= 32 )  bsBlueColor = 31;
									if ( bsGreenColor >= 64 )  bsGreenColor = 63;
									if ( bsRedColor >= 32 )  bsRedColor = 31;
									
									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX) + i] = (bsBlueColor<<m_stBitsMaskInfo.bBShift) |  (bsGreenColor<<m_stBitsMaskInfo.bGShift) | (bsRedColor<<m_stBitsMaskInfo.bRShift);
								}
//								memcpy(&pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX)], &pwSrc[x], sizeof(WORD)*(rc.right-nLastWidth));
								x += nCntCopyWord;
							}
							else
							{
								for(WORD i = 0 ; i < (sizeof(WORD)*nCntCopyWord/2);i++)
								{
									bsBlueColor  = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bsGreenColor = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bsRedColor   = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									BYTE	temp;

									temp = (bsRedColor+(bsGreenColor>>1)+bsBlueColor)/3;

									bsBlueColor = temp>0 ? (temp&bdBlueColor) : 0;
									bsGreenColor = temp>0 ? (temp&(bdGreenColor<<1)) : 0;
									bsRedColor = temp>0 ? (temp&bdRedColor) :  0;

									if ( bsBlueColor >= 32 )  bsBlueColor = 31;
									if ( bsGreenColor >= 64 )  bsGreenColor = 63;
									if ( bsRedColor >= 32 )  bsRedColor = 31;

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX) + i] = (bsBlueColor<<m_stBitsMaskInfo.bBShift) |  (bsGreenColor<<m_stBitsMaskInfo.bGShift) | (bsRedColor<<m_stBitsMaskInfo.bRShift);
								}
								x += nCntCopyWord;
							}
						}
					}
					else if ( pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
					{
						WORD wDyingKind, wChooseColor;
						wDyingKind = pwSrc[x];
						switch ( wDyingKind )
						{
						case 0xC2: 
							wChooseColor = wChooseColor1;
							break;
						case 0xC3: 
							wChooseColor = wChooseColor2;
							break;
						}
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// 以 rc.left 为基准,复制区域被截取的情况.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								for ( INT nCheck = 0; nCheck < nCurrWidth-rc.left; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									BYTE	temp;

									temp = (bsRedColor+(bGreenWantedColor>>1)+bBlueWantedColor)/3;

									bBlueWantedColor = temp>0 ? (temp&bdBlueColor) : 0;
									bGreenWantedColor = temp>0 ? (temp&(bdGreenColor<<1)) : 0;
									bRedWantedColor = temp>0 ? (temp&bdRedColor) :  0;

									if ( bBlueWantedColor >= 32 )  bBlueWantedColor = 31;
									if ( bGreenWantedColor >= 64 )  bGreenWantedColor = 63;
									if ( bRedWantedColor >= 32 )  bRedWantedColor = 31;

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += (nCurrWidth-rc.left);
							}
							// 以 rc.right 点为基准，复制区域被截断的情况.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for ( INT nCheck = 0; nCheck < rc.right-nLastWidth; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									BYTE	temp;

									temp = (bsRedColor+(bGreenWantedColor>>1)+bBlueWantedColor)/3;

									bBlueWantedColor = temp>0 ? (temp&bdBlueColor) : 0;
									bGreenWantedColor = temp>0 ? (temp&(bdGreenColor<<1)) : 0;
									bRedWantedColor = temp>0 ? (temp&bdRedColor) :  0;

									if ( bBlueWantedColor >= 32 )  bBlueWantedColor = 31;
									if ( bGreenWantedColor >= 64 )  bGreenWantedColor = 63;
									if ( bRedWantedColor >= 32 )  bRedWantedColor = 31;

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
							else
							{
								for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									BYTE	temp;

									temp = (bsRedColor+(bGreenWantedColor>>1)+bBlueWantedColor)/3;

									bBlueWantedColor = temp>0 ? (temp&bdBlueColor) : 0;
									bGreenWantedColor = temp>0 ? (temp&(bdGreenColor<<1)) : 0;
									bRedWantedColor = temp>0 ? (temp&bdRedColor) :  0;

									if ( bBlueWantedColor >= 32 )  bBlueWantedColor = 31;
									if ( bGreenWantedColor >= 64 )  bGreenWantedColor = 63;
									if ( bRedWantedColor >= 32 )  bRedWantedColor = 31;

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																									 	  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
						}
					}	
				}
				// 将本行的结尾移到下一行的开头.
				nWidthEnd++;

				nWidthStart	= nWidthEnd;
				nCurrWidth = 0;
			}
		}

		m_pddsBackBuffer->Unlock(NULL);
		return TRUE;
	}
	return FALSE;
}



BOOL CWHDXGraphicWindow::DrawWithImageForCompClipRgnGray(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc , WORD wClipWidth, WORD wClipHeight, WORD wChooseColor1, WORD wChooseColor2)
{
	RECT			rc;
	DDSURFACEDESC2	ddsd;

	INT	nWidth		= nXSize;
	INT	nHeight		= nYSize;
	INT	nXOffset	= 0;
	INT	nYOffset	= 0;
	INT	nStartX		= 0;
	INT	nStartY		= 0;
	INT	nEndX		= wClipWidth -1;
	INT	nEndY		= wClipHeight-1;

	INT FiveMiddle	= 0xf;
	INT SixMiddle	= 0x1f;

	BYTE bsBlueColor,bsGreenColor,bsRedColor;
	BYTE bdBlueColor,bdGreenColor,bdRedColor;

	bdBlueColor  = 0x0f;
	bdGreenColor = 0x1f;
	bdRedColor   = 0x0f;

	if ( m_pddsBackBuffer != NULL )
	{
		if (nX < nStartX )
		{ 
			nXOffset	= nStartX - nX;
			nWidth		= nXSize - nXOffset;
		}
		if ( (nX+nXSize-1) > nEndX )
			nWidth		= nEndX - nX - nXOffset + 1;		
		if ( nY < nStartY )	
		{ 
			nYOffset	= nStartY - nY;
			nHeight		= nYSize - nYOffset;
		}		
		if ( (nY+nYSize-1) > nEndY )
			nHeight	= nEndY - nY - nYOffset + 1;

		if ( (nWidth > 0) && (nHeight > 0) )
		{
			rc.left		= nXOffset;
			rc.right	= nXOffset+nWidth;
			rc.top		= nYOffset;
			rc.bottom	= nYOffset+nHeight;

			ddsd.dwSize	= sizeof(DDSURFACEDESC2);
			ddsd.lpSurface = NULL;
			
			m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
			if ( !ddsd.lpSurface )			return FALSE;
			WORD* pwdDst;
			pwdDst = (WORD*)ddsd.lpSurface;

			INT nWidthStart	= 0;
			INT nWidthEnd	= 0;
			INT nCurrWidth  = 0;
			INT nCntCopyWord = 0;
			INT nYCnt =0;
			INT nLastWidth = 0;

			FLOAT rBlueRate, rGreenRate, bRedRate;
			BYTE  bRedSrc, bGreenSrc, bBlueSrc;
			BYTE  bRedWantedColor, bGreenWantedColor, bBlueWantedColor;
			WORD  wPixel;

			// y 轴裁剪。
			for ( nYCnt=0 ; nYCnt < rc.top ; nYCnt++ )
			{
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;
				nWidthEnd++;
				nWidthStart	= nWidthEnd;
			}

			// 只决定对 y 轴实际进行循环的 Count.
			for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
			{
				// 获取一行的长度.(单位为字)
				nWidthEnd += pwSrc[nWidthStart];
				nWidthStart++;

				// 按行的长度输出到屏幕。
				for ( INT x = nWidthStart; x < nWidthEnd ; )
				{
					if ( pwSrc[x] == 0xC0 )
					{
						x++;
						nCntCopyWord = pwSrc[x];
						x++;
						nCurrWidth += nCntCopyWord;
					}
					else if ( pwSrc[x] == 0xC1 )
					{
						x++;
						nCntCopyWord = pwSrc[x];	// 个数
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;		// 不合情理的情况
						}
						else
						{
							// 以 rc.left 为基准,复制区域被截取的情况.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);

								for(WORD i = 0 ; i < (sizeof(WORD)*(nCurrWidth-rc.left)/2);i++)
								{
									bsBlueColor  = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bsGreenColor = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bsRedColor   = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									// Draw Gray 									
									BYTE	temp;

									temp = (bsRedColor+bsGreenColor/2+bsBlueColor)/3;

									bsBlueColor = temp>0 ? (temp) : 0;
									bsGreenColor = temp>0 ? (temp<<1) : 0;
									bsRedColor = temp>0 ? (temp) :  0;									

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (rc.left+nX) + i] = (bsBlueColor<<m_stBitsMaskInfo.bBShift) |  (bsGreenColor<<m_stBitsMaskInfo.bGShift) | (bsRedColor<<m_stBitsMaskInfo.bRShift);
								}

								x += (nCurrWidth-rc.left);
							}
							// 以 rc.right 点为基准，复制区域被截断的情况.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for(WORD i = 0 ; i < (sizeof(WORD)*(rc.right-nLastWidth));i++)
								{
									bsBlueColor  = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bsGreenColor = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bsRedColor   = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									// Draw Gray 									

									BYTE	temp;

									temp = (bsRedColor+bsGreenColor/2+bsBlueColor)/3;

									bsBlueColor = temp>0 ? (temp) : 0;
									bsGreenColor = temp>0 ? (temp<<1) : 0;
									bsRedColor = temp>0 ? (temp) :  0;

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX) + i] = (bsBlueColor<<m_stBitsMaskInfo.bBShift) |  (bsGreenColor<<m_stBitsMaskInfo.bGShift) | (bsRedColor<<m_stBitsMaskInfo.bRShift);
								}
//								memcpy(&pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX)], &pwSrc[x], sizeof(WORD)*(rc.right-nLastWidth));
								x += nCntCopyWord;
							}
							else
							{
								for(WORD i = 0 ; i < (sizeof(WORD)*nCntCopyWord/2);i++)
								{
									bsBlueColor  = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bsGreenColor = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bsRedColor   = (BYTE)((pwSrc[x+i] & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);
									// Draw Gray 
									BYTE	temp;

									temp = (bsRedColor+bsGreenColor/2+bsBlueColor)/3;

									bsBlueColor = temp>0 ? (temp) : 0;
									bsGreenColor = temp>0 ? (temp<<1) : 0;
									bsRedColor = temp>0 ? (temp) :  0;

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX) + i] = (bsBlueColor<<m_stBitsMaskInfo.bBShift) |  (bsGreenColor<<m_stBitsMaskInfo.bGShift) | (bsRedColor<<m_stBitsMaskInfo.bRShift);
								}
								x += nCntCopyWord;
							}
						}
					}
					else if ( pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
					{
						WORD wDyingKind, wChooseColor;
						wDyingKind = pwSrc[x];
						switch ( wDyingKind )
						{
						case 0xC2: 
							wChooseColor = wChooseColor1;
							break;
						case 0xC3: 
							wChooseColor = wChooseColor2;
							break;
						}
						x++;
						nCntCopyWord = pwSrc[x];
						x++;

						nLastWidth = nCurrWidth;
						nCurrWidth += nCntCopyWord;

						if ( rc.left > nCurrWidth || rc.right < nLastWidth )
						{
							x += nCntCopyWord;
						}
						else
						{
							// 以 rc.left 为基准,复制区域被截取的情况.
							if ( nLastWidth < rc.left && rc.left <= nCurrWidth )
							{
								x += (rc.left-nLastWidth);
								for ( INT nCheck = 0; nCheck < nCurrWidth-rc.left; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									// Draw Gray 
									BYTE	temp;
									temp = (bRedWantedColor+bGreenWantedColor/2+bBlueWantedColor)/3;

									bBlueWantedColor = temp>0 ? (temp) : 0;
									bGreenWantedColor = temp>0 ? (temp<<1) : 0;
									bRedWantedColor = temp>0 ? (temp) :  0;

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += (nCurrWidth-rc.left);
							}
							// 以 rc.right 点为基准，复制区域被截断的情况.
							else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
							{
								for ( INT nCheck = 0; nCheck < rc.right-nLastWidth; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

									// Draw Gray 
									BYTE	temp;
									temp = (bRedWantedColor+bGreenWantedColor/2+bBlueWantedColor)/3;

									bBlueWantedColor = temp>0 ? (temp) : 0;
									bGreenWantedColor = temp>0 ? (temp<<1) : 0;
									bRedWantedColor = temp>0 ? (temp) :  0;

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																										  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
							else
							{
								for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
								{
									wPixel	  = wChooseColor;
									bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									wPixel	  = pwSrc[x+nCheck];
									bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
									bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
									bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

									rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
									rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
									bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

									bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
									bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
									bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

									if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
										bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
									if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
										bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
									if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
										bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);
									
									// Draw Gray 
									BYTE	temp;
									temp = (bRedWantedColor+bGreenWantedColor/2+bBlueWantedColor)/3;

									bBlueWantedColor = temp>0 ? (temp) : 0;
									bGreenWantedColor = temp>0 ? (temp<<1) : 0;
									bRedWantedColor = temp>0 ? (temp) :  0;

									pwdDst[((nYCnt+nY) * (ddsd.lPitch >> 1)) + (nLastWidth+nX+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																										  (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																									 	  (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
								}
								x += nCntCopyWord;
							}
						}
					}	
				}
				// 将本行的结尾移到下一行的开头.
				nWidthEnd++;

				nWidthStart	= nWidthEnd;
				nCurrWidth = 0;
			}
		}

		m_pddsBackBuffer->Unlock(NULL);
		return TRUE;
	}
	return FALSE;
}


VOID CWHDXGraphicWindow::DrawWithGrayBackBuffer()
{
	DDSURFACEDESC2	ddsd;
	RECT			rc = {0, 0, 800, 500};

	if ( m_pddsBackBuffer == NULL )			return;

	ddsd.dwSize	= sizeof(DDSURFACEDESC2);
	ddsd.lpSurface = NULL;
		
	m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
	if ( !ddsd.lpSurface )		return;
	WORD *pwdSrc, *pwdDst;
	BYTE bsBlueColor, bsGreenColor, bsRedColor;
	pwdDst = (WORD*)ddsd.lpSurface;
	pwdSrc = (WORD*)ddsd.lpSurface;

	for ( INT y=rc.top ; y < rc.bottom ; y++ )
	{
		for ( INT x=rc.left ; x < rc.right ; x++ )
		{
			bsBlueColor  = (BYTE)((pwdSrc[y * (ddsd.lPitch >> 1) + x] & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
			bsGreenColor = (BYTE)((pwdSrc[y * (ddsd.lPitch >> 1) + x] & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
			bsRedColor   = (BYTE)((pwdSrc[y * (ddsd.lPitch >> 1) + x] & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

			// Draw Gray 									
			BYTE	temp;

			temp = (bsRedColor+bsGreenColor/2+bsBlueColor)/3;

			bsBlueColor  = temp>0 ? (temp)    : 0;
			bsGreenColor = temp>0 ? (temp<<1) : 0;
			bsRedColor   = temp>0 ? (temp)    : 0;									

			pwdDst[y * (ddsd.lPitch >> 1) + x] = (bsBlueColor<<m_stBitsMaskInfo.bBShift) |  (bsGreenColor<<m_stBitsMaskInfo.bGShift) | (bsRedColor<<m_stBitsMaskInfo.bRShift);
		}
	}

	m_pddsBackBuffer->Unlock(NULL);
}



/******************************************************************************************************************

	函数名 : CWHDXGraphicWindow::DrawWithImageForCompClipRgnToMem()

	作者 :
	编写日期 :

	目的   : 将压缩的源图像按所需大小解压后的状态复制到传入的内存(pwDst)中.

	输入   : INT nStartX
	         INT nStartY
	         INT nWantedXSize
	         INT nWantedYSize
	         WORD* pwSrc
	         WORD wClipWidth
	         WORD wClipHeight
	         WORD wChooseColor1
	         WORD wChooseColor2
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CWHDXGraphicWindow::DrawWithImageForCompColorToMem(RECT rcWanted, WORD* pwSrc, WORD wColor, WORD* pwDst)
{
	if ( rcWanted.left < 0 || rcWanted.right < 0 || rcWanted.top < 0 || rcWanted.bottom < 0 ||
		 rcWanted.right - rcWanted.left < 0 || rcWanted.bottom - rcWanted.top < 0 || pwDst == NULL )
		 return FALSE;

	INT nWidthStart		= 0;
	INT nWidthEnd		= 0;
	INT nCurrWidth		= 0;
	INT nCntCopyWord	= 0;
	INT nYCnt			= 0;
	INT nLastWidth		= 0;

	// y 轴裁剪。
  	for ( nYCnt=0 ; nYCnt < rcWanted.top ; nYCnt++ )
	{
 		nWidthEnd += pwSrc[nWidthStart];
		nWidthStart++;
		nWidthEnd++;
		nWidthStart	= nWidthEnd;
	}

	INT nNewCurrWidth = 0;

	FLOAT rBlueRate, rGreenRate, rRedRate;
	BYTE  bRedSrc, bGreenSrc, bBlueSrc;
	BYTE  bRedStateColor, bGreenStateColor, bBlueStateColor;
	WORD  wPixel;

	// 只决定对 y 轴实际进行循环的 Count.
	for ( nYCnt=rcWanted.top ; nYCnt < rcWanted.bottom ; nYCnt++ )
	{
		// 获取一行的长度.(单位为字)
		nWidthEnd += pwSrc[nWidthStart];
		nWidthStart++;

		// 按行的长度输出到屏幕。
		for ( INT x = nWidthStart; x < nWidthEnd ; )
		{
			if ( pwSrc[x] == 0xC0 )
			{
				x++;
				nCntCopyWord = pwSrc[x];
				x++;
				nLastWidth = nCurrWidth;
				nCurrWidth += nCntCopyWord;

				// 以 rcWanted.left 点为基准, 复制区域发生重叠的情况.
				if ( nLastWidth <= rcWanted.left && nCurrWidth >= rcWanted.right )
				{
					memset(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left))  + nNewCurrWidth], 0, sizeof(WORD)*(rcWanted.right-rcWanted.left));
					nNewCurrWidth += rcWanted.right-rcWanted.left;
				}
				else if ( nLastWidth < rcWanted.left && nCurrWidth > rcWanted.left )
				{
					memset(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + nNewCurrWidth], 0, sizeof(WORD)*(nCurrWidth-rcWanted.left));
					nNewCurrWidth += nCurrWidth-rcWanted.left;
				}
				else if ( nLastWidth >= rcWanted.left && nCurrWidth <= rcWanted.right )
				{
					memset(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left))  + nNewCurrWidth], 0, sizeof(WORD)*(nCntCopyWord));
					nNewCurrWidth += nCntCopyWord;
				}
				// 以 rcWanted.right 点为基准, 复制区域被涉及的情况.
				else if ( nLastWidth < rcWanted.right && nCurrWidth > rcWanted.right )
				{
					memset(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + nNewCurrWidth], 0, sizeof(WORD)*(rcWanted.right-nLastWidth));
					nNewCurrWidth += rcWanted.right-nLastWidth;
				}
			}
			else if ( pwSrc[x] == 0xC1 || pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
			{
				x++;
				nCntCopyWord = pwSrc[x];
				x++;

				nLastWidth = nCurrWidth;
				nCurrWidth += nCntCopyWord;

				if ( rcWanted.left > nCurrWidth || rcWanted.right < nLastWidth )
				{
					x += nCntCopyWord;
				}
				else
				{
					// 以 rcWanted.left 点为基准, 复制区域发生重叠的情况.
					if ( nLastWidth <= rcWanted.left && nCurrWidth >= rcWanted.right )
					{
						x += (rcWanted.left-nLastWidth);
						for ( INT nCheck = 0; nCheck < rcWanted.right-rcWanted.left; nCheck++ )
						{
							wPixel = wColor;
							bBlueStateColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenStateColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedStateColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							wPixel = pwSrc[x+nCheck];
							bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							if ( !wColor )	
							{
								BYTE bTemp = (BYTE)(bBlueSrc+(bGreenSrc>>1)+bRedSrc)/3;
								if ( bTemp > 31 )	bTemp = 31;
								bBlueStateColor = bRedStateColor = bTemp;
								bGreenStateColor = bTemp<<1;
							}
							else
							{
								rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
								rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
								rRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

								bBlueStateColor  = (BYTE)((FLOAT)bBlueStateColor *rBlueRate );
								bGreenStateColor = (BYTE)((FLOAT)bGreenStateColor*rGreenRate);
								bRedStateColor   = (BYTE)((FLOAT)bRedStateColor  *rRedRate  );

								if ( bBlueStateColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
									bBlueStateColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
								if ( bGreenStateColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
									bGreenStateColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
								if ( bRedStateColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
									bRedStateColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);
							}

							if ( !bRedStateColor && !bGreenStateColor && !bBlueStateColor )		bRedStateColor = bGreenStateColor = bBlueStateColor = 1;

							pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + (nNewCurrWidth+nCheck)] = ((bRedStateColor  <<m_stBitsMaskInfo.bRShift) |
																													   (bGreenStateColor<<m_stBitsMaskInfo.bGShift) |
																													   (bBlueStateColor <<m_stBitsMaskInfo.bBShift));			
						}
						nNewCurrWidth += rcWanted.right-rcWanted.left;
						x += (nCurrWidth-rcWanted.left);
					}
					else if ( nLastWidth < rcWanted.left && nCurrWidth > rcWanted.left )
					{
						x += (rcWanted.left-nLastWidth);
						for ( INT nCheck = 0; nCheck < nCurrWidth-rcWanted.left; nCheck++ )
						{
							wPixel = wColor;
							bBlueStateColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenStateColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedStateColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							wPixel = pwSrc[x+nCheck];
							bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							if ( !wColor )	
							{
								BYTE bTemp = (BYTE)(bBlueSrc+(bGreenSrc>>1)+bRedSrc)/3;
								if ( bTemp > 31 )	bTemp = 31;
								bBlueStateColor = bRedStateColor = bTemp;
								bGreenStateColor = bTemp<<1;
							}
							else
							{
								rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
								rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
								rRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

								bBlueStateColor  = (BYTE)((FLOAT)bBlueStateColor *rBlueRate );
								bGreenStateColor = (BYTE)((FLOAT)bGreenStateColor*rGreenRate);
								bRedStateColor   = (BYTE)((FLOAT)bRedStateColor  *rRedRate  );

								if ( bBlueStateColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
									bBlueStateColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
								if ( bGreenStateColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
									bGreenStateColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
								if ( bRedStateColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
									bRedStateColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);
							}

							if ( !bRedStateColor && !bGreenStateColor && !bBlueStateColor )		bRedStateColor = bGreenStateColor = bBlueStateColor = 1;

							pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + (nNewCurrWidth+nCheck)] = ((bRedStateColor  <<m_stBitsMaskInfo.bRShift) |
																													   (bGreenStateColor<<m_stBitsMaskInfo.bGShift) |
																													   (bBlueStateColor <<m_stBitsMaskInfo.bBShift));										
						}
						nNewCurrWidth += nCurrWidth-rcWanted.left;
						x += (nCurrWidth-rcWanted.left);
					}
					else if ( nLastWidth >= rcWanted.left && nCurrWidth <= rcWanted.right )
					{
						for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
						{
							wPixel = wColor;
							bBlueStateColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenStateColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedStateColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							wPixel = pwSrc[x+nCheck];
							bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							if ( !wColor )	
							{
								BYTE bTemp = (BYTE)(bBlueSrc+(bGreenSrc>>1)+bRedSrc)/3;
								if ( bTemp > 31 )	bTemp = 31;
								bBlueStateColor = bRedStateColor = bTemp;
								bGreenStateColor = bTemp<<1;
							}
							else
							{
								if ( nCheck == 17 )
									int b = 0;

								rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
								rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
								rRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

								bBlueStateColor  = (BYTE)((FLOAT)bBlueStateColor *rBlueRate );
								bGreenStateColor = (BYTE)((FLOAT)bGreenStateColor*rGreenRate);
								bRedStateColor   = (BYTE)((FLOAT)bRedStateColor  *rRedRate  );

								if ( bBlueStateColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
									bBlueStateColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
								if ( bGreenStateColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
									bGreenStateColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
								if ( bRedStateColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
									bRedStateColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);
							}

							if ( !bRedStateColor && !bGreenStateColor && !bBlueStateColor )		bRedStateColor = bGreenStateColor = bBlueStateColor = 1;

							pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + (nNewCurrWidth+nCheck)] = ((bRedStateColor  <<m_stBitsMaskInfo.bRShift) |
																													   (bGreenStateColor<<m_stBitsMaskInfo.bGShift) |
																													   (bBlueStateColor <<m_stBitsMaskInfo.bBShift));			
						}
						nNewCurrWidth += nCntCopyWord;
						x += nCntCopyWord;
					}
					// 以 rcWanted.right 点为基准, 复制区域被涉及的情况.
					else if ( nLastWidth < rcWanted.right && nCurrWidth > rcWanted.right )
					{
						for ( INT nCheck = 0; nCheck < rcWanted.right-nLastWidth; nCheck++ )
						{
							wPixel = wColor;
							bBlueStateColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenStateColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedStateColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							wPixel = pwSrc[x+nCheck];
							bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							if ( !wColor )	
							{
								BYTE bTemp = (BYTE)(bBlueSrc+(bGreenSrc>>1)+bRedSrc)/3;
								if ( bTemp > 31 )	bTemp = 31;
								bBlueStateColor = bRedStateColor = bTemp;
								bGreenStateColor = bTemp<<1;
							}
							else
							{
								rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
								rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
								rRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

								bBlueStateColor  = (BYTE)((FLOAT)bBlueStateColor *rBlueRate );
								bGreenStateColor = (BYTE)((FLOAT)bGreenStateColor*rGreenRate);
								bRedStateColor   = (BYTE)((FLOAT)bRedStateColor  *rRedRate  );

								if ( bBlueStateColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
									bBlueStateColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
								if ( bGreenStateColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
									bGreenStateColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
								if ( bRedStateColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
									bRedStateColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);
							}

							if ( !bRedStateColor && !bGreenStateColor && !bBlueStateColor )		bRedStateColor = bGreenStateColor = bBlueStateColor = 1;

							pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + (nNewCurrWidth+nCheck)] = ((bRedStateColor  <<m_stBitsMaskInfo.bRShift) |
																													   (bGreenStateColor<<m_stBitsMaskInfo.bGShift) |
																													   (bBlueStateColor <<m_stBitsMaskInfo.bBShift));			
						}
						nNewCurrWidth += rcWanted.right-nLastWidth;
						x += nCntCopyWord;
					}
					else
					{
						x += nCntCopyWord;
					}
				}
			}	
		}
		// 将本行的结尾移到下一行的开头.
		nWidthEnd++;

		nWidthStart	= nWidthEnd;
		nCurrWidth = 0;
		nNewCurrWidth = 0;
	}
	return TRUE;
}




BOOL CWHDXGraphicWindow::DrawWithImageForCompToMem(RECT rcWanted, WORD* pwSrc, WORD wChooseColor1, WORD wChooseColor2, WORD* pwDst)
{
	if ( rcWanted.left < 0 || rcWanted.right < 0 || rcWanted.top < 0 || rcWanted.bottom < 0 ||
		 rcWanted.right - rcWanted.left < 0 || rcWanted.bottom - rcWanted.top < 0 || pwDst == NULL )
		 return FALSE;

	INT nWidthStart		= 0;
	INT nWidthEnd		= 0;
	INT nCurrWidth		= 0;
	INT nCntCopyWord	= 0;
	INT nYCnt			= 0;
	INT nLastWidth		= 0;

	// y 轴裁剪。
  	for ( nYCnt=0 ; nYCnt < rcWanted.top ; nYCnt++ )
	{
 		nWidthEnd += pwSrc[nWidthStart];
		nWidthStart++;
		nWidthEnd++;
		nWidthStart	= nWidthEnd;
	}

	INT nNewCurrWidth = 0;

	FLOAT rBlueRate, rGreenRate, bRedRate;
	BYTE  bRedSrc, bGreenSrc, bBlueSrc;
	BYTE  bRedWantedColor, bGreenWantedColor, bBlueWantedColor;
	WORD  wPixel;

	// 只决定对 y 轴实际进行循环的 Count.
	for ( nYCnt=rcWanted.top ; nYCnt < rcWanted.bottom ; nYCnt++ )
	{
		// 获取一行的长度.(单位为字)
		nWidthEnd += pwSrc[nWidthStart];
		nWidthStart++;

		// 按行的长度输出到屏幕。
		for ( INT x = nWidthStart; x < nWidthEnd ; )
		{
			if ( pwSrc[x] == 0xC0 )
			{
				x++;
				nCntCopyWord = pwSrc[x];
				x++;
				nLastWidth = nCurrWidth;
				nCurrWidth += nCntCopyWord;

				// 以 rcWanted.left 点为基准, 复制区域发生重叠的情况.
				if ( nLastWidth <= rcWanted.left && nCurrWidth >= rcWanted.right )
				{
					memset(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left))  + nNewCurrWidth], 0, sizeof(WORD)*(rcWanted.right-rcWanted.left));
					nNewCurrWidth += rcWanted.right-rcWanted.left;
				}
				else if ( nLastWidth < rcWanted.left && nCurrWidth > rcWanted.left )
				{
					memset(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + nNewCurrWidth], 0, sizeof(WORD)*(nCurrWidth-rcWanted.left));
					nNewCurrWidth += nCurrWidth-rcWanted.left;
				}
				else if ( nLastWidth >= rcWanted.left && nCurrWidth <= rcWanted.right )
				{
					memset(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left))  + nNewCurrWidth], 0, sizeof(WORD)*(nCntCopyWord));
					nNewCurrWidth += nCntCopyWord;
				}
				// 以 rcWanted.right 点为基准, 复制区域被涉及的情况.
				else if ( nLastWidth < rcWanted.right && nCurrWidth > rcWanted.right )
				{
					memset(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + nNewCurrWidth], 0, sizeof(WORD)*(rcWanted.right-nLastWidth));
					nNewCurrWidth += rcWanted.right-nLastWidth;
				}
			}
			else if ( pwSrc[x] == 0xC1 )
			{
				x++;
				nCntCopyWord = pwSrc[x];
				x++;

				nLastWidth = nCurrWidth;
				nCurrWidth += nCntCopyWord;

				if ( rcWanted.left > nCurrWidth || rcWanted.right < nLastWidth )
				{
					x += nCntCopyWord;
				}
				else
				{
					// 以 rcWanted.left 点为基准, 复制区域发生重叠的情况.
					if ( nLastWidth <= rcWanted.left && nCurrWidth >= rcWanted.right )
					{
						x += (rcWanted.left-nLastWidth);
						memcpy(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + nNewCurrWidth], &pwSrc[x], sizeof(WORD)*(rcWanted.right-rcWanted.left));
						nNewCurrWidth += rcWanted.right-rcWanted.left;
						x += (nCurrWidth-rcWanted.left);
					}
					else if ( nLastWidth < rcWanted.left && nCurrWidth > rcWanted.left )
					{
						x += (rcWanted.left-nLastWidth);
						memcpy(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + nNewCurrWidth], &pwSrc[x], sizeof(WORD)*(nCurrWidth-rcWanted.left));
						nNewCurrWidth += nCurrWidth-rcWanted.left;
						x += (nCurrWidth-rcWanted.left);
					}
					else if ( nLastWidth >= rcWanted.left && nCurrWidth <= rcWanted.right )
					{
						memcpy(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + nNewCurrWidth], &pwSrc[x], sizeof(WORD)*nCntCopyWord);
						nNewCurrWidth += nCntCopyWord;
						x += nCntCopyWord;
					}
					// 以 rcWanted.right 点为基准, 复制区域被涉及的情况.
					else if ( nLastWidth < rcWanted.right && nCurrWidth > rcWanted.right )
					{
						memcpy(&pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + nNewCurrWidth], &pwSrc[x], sizeof(WORD)*(rcWanted.right-nLastWidth));
						nNewCurrWidth += rcWanted.right-nLastWidth;
						x += nCntCopyWord;
					}
					else
					{
						x += nCntCopyWord;
					}
				}
			}
			else if ( pwSrc[x] == 0xC2 || pwSrc[x] == 0xC3 )
			{
				WORD wDyingKind, wChooseColor;
				wDyingKind = pwSrc[x];
				switch ( wDyingKind )
				{
				case 0xC2: 
					wChooseColor = wChooseColor1;
					break;
				case 0xC3: 
					wChooseColor = wChooseColor2;
					break;
				}
				x++;
				nCntCopyWord = pwSrc[x];
				x++;

				nLastWidth = nCurrWidth;
				nCurrWidth += nCntCopyWord;

				if ( rcWanted.left > nCurrWidth || rcWanted.right < nLastWidth )
				{
					x += nCntCopyWord;
				}
				else
				{
					// 以 rcWanted.left 点为基准, 复制区域发生重叠的情况.
					if ( nLastWidth <= rcWanted.left && nCurrWidth >= rcWanted.right )
					{
						x += (rcWanted.left-nLastWidth);
						for ( INT nCheck = 0; nCheck < rcWanted.right-rcWanted.left; nCheck++ )
						{
							wPixel	  = wChooseColor;
							bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							wPixel	  = pwSrc[x+nCheck];
							bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
							rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
							bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

							bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
							bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
							bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

							if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
								bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
							if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
								bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
							if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
								bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

							pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + (nNewCurrWidth+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																													   (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																													   (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
						}
						nNewCurrWidth += rcWanted.right-rcWanted.left;
						x += (nCurrWidth-rcWanted.left);
					}
					else if ( nLastWidth < rcWanted.left && nCurrWidth > rcWanted.left )
					{
						x += (rcWanted.left-nLastWidth);
						for ( INT nCheck = 0; nCheck < nCurrWidth-rcWanted.left; nCheck++ )
						{
							wPixel	  = wChooseColor;
							bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							wPixel	  = pwSrc[x+nCheck];
							bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
							rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
							bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

							bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
							bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
							bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

							if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
								bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
							if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
								bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
							if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
								bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

							pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + (nNewCurrWidth+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																													   (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																													   (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));										
						}
						nNewCurrWidth += nCurrWidth-rcWanted.left;
						x += (nCurrWidth-rcWanted.left);
					}
					else if ( nLastWidth >= rcWanted.left && nCurrWidth <= rcWanted.right )
					{
						for ( INT nCheck = 0; nCheck < nCntCopyWord; nCheck++ )
						{
							wPixel	  = wChooseColor;
							bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							wPixel	  = pwSrc[x+nCheck];
							bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
							rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
							bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

							bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
							bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
							bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

							if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
								bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
							if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
								bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
							if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
								bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

							pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + (nNewCurrWidth+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																													   (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																													   (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
						}
						nNewCurrWidth += nCntCopyWord;
						x += nCntCopyWord;
					}
					// 以 rcWanted.right 点为基准, 复制区域被涉及的情况.
					else if ( nLastWidth < rcWanted.right && nCurrWidth > rcWanted.right )
					{
						for ( INT nCheck = 0; nCheck < rcWanted.right-nLastWidth; nCheck++ )
						{
							wPixel	  = wChooseColor;
							bBlueWantedColor  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenWantedColor = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedWantedColor   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							wPixel	  = pwSrc[x+nCheck];
							bBlueSrc  = (BYTE)((wPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
							bGreenSrc = (BYTE)((wPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
							bRedSrc   = (BYTE)((wPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

							rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
							rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
							bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

							bBlueWantedColor = (BYTE)(((FLOAT)bBlueWantedColor*rBlueRate));
							bGreenWantedColor = (BYTE)(((FLOAT)bGreenWantedColor*rGreenRate));
							bRedWantedColor = (BYTE)(((FLOAT)bRedWantedColor*bRedRate));

							if ( bBlueWantedColor > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
								bBlueWantedColor = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
							if ( bGreenWantedColor > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
								bGreenWantedColor = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
							if ( bRedWantedColor > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
								bRedWantedColor = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);


							pwDst[((nYCnt-rcWanted.top) * (rcWanted.right-rcWanted.left)) + (nNewCurrWidth+nCheck)] = ((bRedWantedColor  <<m_stBitsMaskInfo.bRShift) |
																													   (bGreenWantedColor<<m_stBitsMaskInfo.bGShift) |
																													   (bBlueWantedColor <<m_stBitsMaskInfo.bBShift));			
						}
						nNewCurrWidth += rcWanted.right-nLastWidth;
						x += nCntCopyWord;
					}
					else
					{
						x += nCntCopyWord;
					}
				}
			}	
		}
		// 将本行的结尾移到下一行的开头.
		nWidthEnd++;

		nWidthStart	= nWidthEnd;
		nCurrWidth = 0;
		nNewCurrWidth = 0;
	}
	return TRUE;
}




/******************************************************************************************************************

	函数名 : CWHDXGraphicWindow::DrawWithABlendForIntersectCompData()

	作者 :
	编写日期 :

	目的   :
	输入   : INT nX					: 后台缓冲区上的绘制起始位置.
	         INT nY
	         INT nSrcXSize			: 原始图像的大小.
	         INT nSrcYSize
	         WORD* pwSrc			: 原始图像的数据.
	         INT nDstXSize			: 目标图像的大小.
	         INT nDstYSize
	         WORD* pwDst			: 目标图像的数据.
	         WORD wSrcChooseColor1	: 原始图像的染色图像1 颜色.
	         WORD wSrcChooseColor2
	         WORD wDstChooseColor1	: 目标图像的染色图像1 颜色.
	         WORD wDstChooseColor2
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CWHDXGraphicWindow::DrawWithABlendForIntersectCompData(INT nSrcX, INT nSrcY, 
															INT nSrcXSize, INT nSrcYSize, WORD* pwSrc,
															INT nDstX, INT nDstY,
															INT nDstXSize, INT nDstYSize, WORD* pwDst,
															WORD wClipWidth, WORD wClipHeight,
															BYTE bOpa, BOOL bFocused,
															WORD wSrcChooseColor1, WORD wSrcChooseColor2, 
															WORD wDstChooseColor1, WORD wDstChooseColor2,
															WORD wSrcColor, WORD wDstColor)
{
	DDSURFACEDESC2	ddsd;
	RECT rcSrc, rcDst;
	RECT rcSrcIntersect = { 0, 0, 0, 0 };
	RECT rcDstIntersect = { 0, 0, 0, 0 };
	WORD* pwdDst = NULL;

	INT	nSrcWidth	= nSrcXSize;
	INT	nSrcHeight	= nSrcYSize;
	INT	nXSrcOffset	= 0;
	INT	nYSrcOffset	= 0;

	INT	nDstWidth	= nDstXSize;
	INT	nDstHeight	= nDstYSize;
	INT	nXDstOffset	= 0;
	INT	nYDstOffset	= 0;

	INT	nEndX		= wClipWidth -1;
	INT	nEndY		= wClipHeight-1;
	INT	nStartX		= 0;
	INT	nStartY		= 0;

	WORD* pwDstSave = NULL;
	WORD* pwSrcSave = NULL;

	if ( m_pddsBackBuffer != NULL )
	{
		// Source 的屏幕裁剪区域.
		if ( nSrcX < nStartX )
		{ 
			nXSrcOffset	= nStartX - nSrcX;
			nSrcWidth	= nSrcXSize - nXSrcOffset;
		}
		if ( (nSrcX+nSrcXSize-1) > nEndX )
			nSrcWidth	= nEndX - nSrcX - nXSrcOffset + 1;		
		if ( nSrcY < nStartY )	
		{ 
			nYSrcOffset	= nStartY - nSrcY;
			nSrcHeight	= nSrcYSize - nYSrcOffset;
		}		
		if ( (nSrcY+nSrcYSize-1) > nEndY )
			nSrcHeight	= nEndY - nSrcY - nYSrcOffset + 1;

		if ( (nSrcWidth > 0) && (nSrcHeight > 0) )
		{
			// Source 区域的设置。
			rcSrc.left		= nXSrcOffset;
			rcSrc.right		= nXSrcOffset+nSrcWidth;
			rcSrc.top		= nYSrcOffset;
			rcSrc.bottom	= nYSrcOffset+nSrcHeight;

			// Destination 的屏幕裁剪区域.
			if ( nDstX < nStartX )
			{ 
				nXDstOffset	= nStartX - nDstX;
				nDstWidth	= nDstXSize - nXDstOffset;
			}
			if ( (nDstX+nDstXSize-1) > nEndX )
				nDstWidth	= nEndX - nDstX - nXDstOffset + 1;		
			if ( nDstY < nStartY )	
			{ 
				nYDstOffset	= nStartY - nDstY;
				nDstHeight	= nDstYSize - nYDstOffset;
			}		
			if ( (nDstY+nDstYSize-1) > nEndY )
				nDstHeight	= nEndY - nDstY - nYDstOffset + 1;

			if ( (nDstWidth > 0) && (nDstHeight > 0) )
			{
				// Destination 区域的设置.
				rcDst.left		= nXDstOffset;
				rcDst.right		= nXDstOffset+nDstWidth;
				rcDst.top		= nYDstOffset;
				rcDst.bottom	= nYDstOffset+nDstHeight;

				// 求两个图像的重叠区域。
				RECT rcTemp;

				// 求出相对于源区域的目地区域.
				rcTemp.left = rcDst.left + (nDstX-nSrcX);
				rcTemp.top = rcDst.top + (nDstY-nSrcY);
				rcTemp.right = rcDst.right + (nDstX-nSrcX);
				rcTemp.bottom = rcDst.bottom + (nDstY-nSrcY);

				// 求以源矩形区域为基准的两个矩形的相交矩形.
				IntersectRect(&rcSrcIntersect, &rcSrc, &rcTemp);

				// 求出相对于目标区域的源区域.
				rcTemp.left = rcSrc.left + (nSrcX-nDstX);
				rcTemp.top = rcSrc.top + (nSrcY-nDstY);
				rcTemp.right = rcSrc.right + (nSrcX-nDstX);
				rcTemp.bottom = rcSrc.bottom + (nSrcY-nDstY);

				// 求以目标矩形区域为基准的两个矩形的相交矩形。
				IntersectRect(&rcDstIntersect, &rcTemp, &rcDst);

				ddsd.dwSize	= sizeof(DDSURFACEDESC2);
				ddsd.lpSurface = NULL;
				m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
				if ( !ddsd.lpSurface )			return FALSE;
				pwdDst = (WORD*)ddsd.lpSurface;

				pwSrcSave = new WORD[(rcSrcIntersect.right-rcSrcIntersect.left)*(rcSrcIntersect.bottom-rcSrcIntersect.top)];
				pwDstSave = new WORD[(rcDstIntersect.right-rcDstIntersect.left)*(rcDstIntersect.bottom-rcDstIntersect.top)];

				if ( wSrcColor == 0XFFFF && wDstColor == 0XFFFF )
				{
					DrawWithImageForCompToMem(rcSrcIntersect, (WORD*)pwSrc, wSrcChooseColor1, wSrcChooseColor2, pwSrcSave);
					DrawWithImageForCompToMem(rcDstIntersect, (WORD*)pwDst, wDstChooseColor1, wDstChooseColor2, pwDstSave);
				}
				else if ( wSrcColor != 0XFFFF && wDstColor == 0XFFFF )
				{
					DrawWithImageForCompColorToMem(rcSrcIntersect, (WORD*)pwSrc, wSrcColor, pwSrcSave);
					DrawWithImageForCompToMem(rcDstIntersect, (WORD*)pwDst, wDstChooseColor1, wDstChooseColor2, pwDstSave);
				}
				else if ( wSrcColor == 0XFFFF && wDstColor != 0XFFFF )
				{
					DrawWithImageForCompToMem(rcSrcIntersect, (WORD*)pwSrc, wSrcChooseColor1, wSrcChooseColor2, pwSrcSave);
					DrawWithImageForCompColorToMem(rcDstIntersect, (WORD*)pwDst, wDstColor, pwDstSave);
				}
				else
				{
					DrawWithImageForCompColorToMem(rcSrcIntersect, (WORD*)pwSrc, wSrcColor, pwSrcSave);
					DrawWithImageForCompColorToMem(rcDstIntersect, (WORD*)pwDst, wDstColor, pwDstSave);
				}

				BYTE bRedDst, bGreenDst, bBlueDst;
				BYTE bRedSrc, bGreenSrc, bBlueSrc;
				WORD wSrcPixel, wDstPixel;
				FLOAT rBlueRate, rGreenRate, bRedRate;

				for ( INT nYCnt = 0; nYCnt < rcSrcIntersect.bottom - rcSrcIntersect.top; nYCnt++ )
					for ( INT nXCnt = 0; nXCnt < rcSrcIntersect.right - rcSrcIntersect.left; nXCnt++ )
				{
					wSrcPixel = pwSrcSave[nYCnt*(rcSrcIntersect.right - rcSrcIntersect.left) + nXCnt];
					wDstPixel = pwDstSave[nYCnt*(rcDstIntersect.right - rcDstIntersect.left) + nXCnt];
					if ( wSrcPixel != 0 && wDstPixel != 0 )
					{
						bBlueSrc  = (BYTE)((wSrcPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
						bGreenSrc = (BYTE)((wSrcPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
						bRedSrc   = (BYTE)((wSrcPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

						if ( bFocused )
						{
							rBlueRate = (FLOAT)((FLOAT)bBlueSrc / (FLOAT)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift));
							rGreenRate = (FLOAT)((FLOAT)bGreenSrc / (FLOAT)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift));
							bRedRate = (FLOAT)((FLOAT)bRedSrc / (FLOAT)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift));

							bBlueSrc = (BYTE)((FLOAT)bBlueSrc + ((FLOAT)bBlueSrc*rBlueRate));
							bGreenSrc = (BYTE)((FLOAT)bGreenSrc + ((FLOAT)bGreenSrc*rGreenRate));
							bRedSrc = (BYTE)((FLOAT)bRedSrc + ((FLOAT)bRedSrc*bRedRate));

							if ( bBlueSrc > (m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift) )
								bBlueSrc = (BYTE)(m_stBitsMaskInfo.dwBMask>>m_stBitsMaskInfo.bBShift);
							if ( bGreenSrc > (m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift) )
								bGreenSrc = (BYTE)(m_stBitsMaskInfo.dwGMask>>m_stBitsMaskInfo.bGShift);
							if ( bRedSrc > (m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift) )
								bRedSrc = (BYTE)(m_stBitsMaskInfo.dwRMask>>m_stBitsMaskInfo.bRShift);

							// 获得焦点时提高 Opacity 值.
							bOpa = 40;
						}

						bBlueDst  = (BYTE)((wDstPixel & m_stBitsMaskInfo.dwBMask) >> m_stBitsMaskInfo.bBShift);
						bGreenDst = (BYTE)((wDstPixel & m_stBitsMaskInfo.dwGMask) >> m_stBitsMaskInfo.bGShift);
						bRedDst   = (BYTE)((wDstPixel & m_stBitsMaskInfo.dwRMask) >> m_stBitsMaskInfo.bRShift);

						bBlueDst  = (BYTE)((bOpa*(bBlueDst -bBlueSrc )+100*bBlueSrc ) / 100);
						bGreenDst = (BYTE)((bOpa*(bGreenDst-bGreenSrc)+100*bGreenSrc) / 100);
						bRedDst   = (BYTE)((bOpa*(bRedDst  -bRedSrc  )+100*bRedSrc  ) / 100);

						pwdDst[((nYCnt+nSrcY+rcSrcIntersect.top) * (ddsd.lPitch >> 1)) + (rcSrcIntersect.left+nSrcX+nXCnt)] = 
												((bRedDst  <<m_stBitsMaskInfo.bRShift) |
												 (bGreenDst<<m_stBitsMaskInfo.bGShift) |
												 (bBlueDst <<m_stBitsMaskInfo.bBShift));
					}
				}

				SAFE_DELETE(pwSrcSave);
				SAFE_DELETE(pwDstSave);
				m_pddsBackBuffer->Unlock(NULL);
				return TRUE;
			}
		}
	}
	return FALSE;
}





HRESULT CWHDXGraphicWindow::Present()
{
	HRESULT hr;

	if ( NULL == m_pddsFrontBuffer || NULL == m_pddsBackBuffer )
	{
		return E_POINTER;
	}

	while ( TRUE )
	{
//		if ( m_bScreenModeFlag & _DXG_SCREENMODE_WINDOW )
//		{
//		 为了编辑窗口的输出而设置裁剪器.
			m_pddsFrontBuffer->SetClipper(m_lpcClipper);
			hr = m_pddsFrontBuffer->Blt(&m_rcWindow, m_pddsBackBuffer, NULL, DDBLT_WAIT, NULL);
/*		}
		else
		{
			hr = m_pddsFrontBuffer->Flip(NULL, DDFLIP_WAIT );
		}
*/
		if(hr == DDERR_SURFACELOST)
		{
			m_pddsFrontBuffer->Restore();
			m_pddsBackBuffer->Restore();
		}

		if(hr != DDERR_WASSTILLDRAWING)
			return hr;
	} 
}



HRESULT CWHDXGraphicWindow::RestoreSurfaces()
{
    HRESULT hr;

	if ( !m_pDD )
		return E_FAIL;

    if ( FAILED(hr = m_pDD->RestoreAllSurfaces()) )
        return hr;

    return S_OK;
}













/******************************************************************************************************************

  Callback Function

*******************************************************************************************************************/


static BOOL WINAPI DXGDriverEnumCallbackEx(GUID *pGUID, LPSTR szDesc, LPSTR szName, LPVOID pContext, HMONITOR hm)
{
    LPDIRECTDRAW7			pDD	 = NULL;
    LPDIRECT3D7				pD3D = NULL;
	DXG_ENUM_DEVICEINFO		stDXGEnumDeviceInfo;
    HRESULT					hr;

    if ( FAILED(hr = DirectDrawCreateEx(pGUID, (VOID**) &pDD, IID_IDirectDraw7, NULL)) )
	{
		MessageBox(NULL, TEXT("[DXGDriverEnumCallbackEx]") TEXT("Can't create DDraw during enumeration!"), "MirDXG", MB_ICONERROR | MB_OK);
        return D3DENUMRET_OK;
	}

    if ( FAILED(hr = pDD->QueryInterface(IID_IDirect3D7, (VOID**) &pD3D)) )
    {
		SAFE_RELEASE(pDD);
		MessageBox(NULL, TEXT("[DXGDriverEnumCallbackEx]") TEXT("Can't query IDirect3D7 during enumeration!"), "MirDXG", MB_ICONERROR | MB_OK);
        return D3DENUMRET_OK;
    }

    ZeroMemory(&stDXGEnumDeviceInfo, sizeof(DXG_ENUM_DEVICEINFO));
    lstrcpyn(stDXGEnumDeviceInfo.szDriverDesc, szDesc, 255);

    stDXGEnumDeviceInfo.ddDriverCaps.dwSize = sizeof(DDCAPS);
    stDXGEnumDeviceInfo.ddHELCaps.dwSize    = sizeof(DDCAPS);
    pDD->GetCaps(&stDXGEnumDeviceInfo.ddDriverCaps, &stDXGEnumDeviceInfo.ddHELCaps);

    if ( pGUID )
    {
        stDXGEnumDeviceInfo.guidDriver  = (*pGUID);
        stDXGEnumDeviceInfo.pDriverGUID = &stDXGEnumDeviceInfo.guidDriver;
    }

    if ( FAILED(hr = pDD->EnumDisplayModes(0, NULL, &stDXGEnumDeviceInfo, DXGEnumDisplayModeCallback)) )
	{
		MessageBox(NULL, TEXT("[DXGDriverEnumCallbackEx]") TEXT("Can't enumerate displaymode!"), "MirDXG", MB_ICONERROR | MB_OK);
	}

    qsort(stDXGEnumDeviceInfo.pddsdModes, stDXGEnumDeviceInfo.dwNumModes, sizeof(DDSURFACEDESC2), DXGModesSortCallback);

    if ( FAILED(hr = pD3D->EnumDevices(DXG3DDeviceEnumCallback, &stDXGEnumDeviceInfo)) )
	{
		MessageBox(NULL, TEXT("[DXGDriverEnumCallbackEx]") TEXT("Can't enumerate device!"), "MirDXG", MB_ICONERROR | MB_OK);
	}

    SAFE_DELETE(stDXGEnumDeviceInfo.pddsdModes);
	SAFE_RELEASE(pD3D);
	SAFE_RELEASE(pDD);

	return DDENUMRET_OK;
}


static HRESULT WINAPI DXGEnumDisplayModeCallback(DDSURFACEDESC2* pddsd, VOID* pParentInfo)
{
    DXG_ENUM_DEVICEINFO* pDevice = (DXG_ENUM_DEVICEINFO*) pParentInfo;

    DDSURFACEDESC2* pddsdNewModes = new DDSURFACEDESC2[pDevice->dwNumModes+1];
    memcpy(pddsdNewModes, pDevice->pddsdModes, pDevice->dwNumModes * sizeof(DDSURFACEDESC2));
    SAFE_DELETE(pDevice->pddsdModes);
    pDevice->pddsdModes = pddsdNewModes;

    pDevice->pddsdModes[pDevice->dwNumModes++] = (*pddsd);

    return DDENUMRET_OK;
}


static int DXGModesSortCallback(const VOID* arg1, const VOID* arg2)
{
    DDSURFACEDESC2* p1 = (DDSURFACEDESC2*)arg1;
    DDSURFACEDESC2* p2 = (DDSURFACEDESC2*)arg2;

    if ( p1->dwWidth < p2->dwWidth )        return -1;
    if ( p1->dwWidth > p2->dwWidth )        return +1;

    if ( p1->dwHeight < p2->dwHeight )      return -1;
    if ( p1->dwHeight > p2->dwHeight )      return +1;

    if ( p1->ddpfPixelFormat.dwRGBBitCount < p2->ddpfPixelFormat.dwRGBBitCount )        return -1;
    if ( p1->ddpfPixelFormat.dwRGBBitCount > p2->ddpfPixelFormat.dwRGBBitCount )        return +1;

    return 0;
}


static HRESULT WINAPI DXG3DDeviceEnumCallback(TCHAR* szDesc, TCHAR* szName, D3DDEVICEDESC7* pDesc, VOID* pParentInfo)
{
    DXG_ENUM_DEVICEINFO* pDriverInfo = (DXG_ENUM_DEVICEINFO*)pParentInfo;
    DXG_ENUM_DEVICEINFO* pDeviceInfo = &g_stDXGEnumDeviceInfo[g_bNumDevices];
    ZeroMemory(pDeviceInfo, sizeof(DXG_ENUM_DEVICEINFO));

    pDeviceInfo->f3DHardware = pDesc->dwDevCaps & D3DDEVCAPS_HWRASTERIZATION;
    memcpy(&pDeviceInfo->ddDeviceDesc, pDesc, sizeof(D3DDEVICEDESC7));

    pDeviceInfo->ddDriverCaps       = pDriverInfo->ddDriverCaps;
    pDeviceInfo->ddHELCaps          = pDriverInfo->ddHELCaps;
    pDeviceInfo->guidDevice         = pDesc->deviceGUID;
    pDeviceInfo->pDeviceGUID        = &pDeviceInfo->guidDevice;
    pDeviceInfo->pddsdModes         = new DDSURFACEDESC2[pDriverInfo->dwNumModes];

    if ( pDriverInfo->pDriverGUID )
    {
        pDeviceInfo->guidDriver  = pDriverInfo->guidDriver;
        pDeviceInfo->pDriverGUID = &pDeviceInfo->guidDriver;
    }
    else
    {
        pDeviceInfo->pDriverGUID = NULL;
    }
    lstrcpyn(pDeviceInfo->szDriverDesc, pDriverInfo->szDriverDesc, 255);
    lstrcpyn(pDeviceInfo->szDeviceDesc, szName, 255);

    for ( DWORD dwCnt=0; dwCnt<pDriverInfo->dwNumModes; dwCnt++ )
    {
        DDSURFACEDESC2 ddsdMode = pDriverInfo->pddsdModes[dwCnt];
        DWORD dwRenderDepths    = pDeviceInfo->ddDeviceDesc.dwDeviceRenderBitDepth;
        DWORD dwDepth           = ddsdMode.ddpfPixelFormat.dwRGBBitCount;

        if ( ( ( dwDepth == 32 ) && ( dwRenderDepths & DDBD_32 ) ) ||
             ( ( dwDepth == 24 ) && ( dwRenderDepths & DDBD_24 ) ) ||
             ( ( dwDepth == 16 ) && ( dwRenderDepths & DDBD_16 ) ) )
        {
            pDeviceInfo->pddsdModes[pDeviceInfo->dwNumModes++] = ddsdMode;
        }
    }

    if ( 0 == pDeviceInfo->dwNumModes )
        return D3DENUMRET_OK;

    g_bNumDevices++;
    if ( NULL != pDeviceInfo->pDriverGUID && FALSE == pDeviceInfo->f3DHardware )
            return D3DENUMRET_OK;

    return D3DENUMRET_OK;
}


static HRESULT WINAPI DXGEnumZBufferFormatsCallback(DDPIXELFORMAT* pddpf, VOID* pContext)
{
    DDPIXELFORMAT* pddpfOut = (DDPIXELFORMAT*) pContext;

    if ( pddpfOut->dwRGBBitCount == pddpf->dwRGBBitCount )
    {
        (*pddpfOut) = (*pddpf);
        return D3DENUMRET_CANCEL;
    }

    return D3DENUMRET_OK;
}