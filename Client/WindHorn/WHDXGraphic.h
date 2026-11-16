#ifndef _WINDHORN_DXGRAPHIC
#define	_WINDHORN_DXGRAPHIC



/******************************************************************************************************************

	CWHDXGraphicWindow Class Declaration

*******************************************************************************************************************/

typedef struct tagDXG_DISPLAYINFO 
{
	WORD			wWidth;
	WORD			wHeight;
	WORD			wBPP;
}DXG_DISPLAYINFO, *LPDXG_DISPLAYINFO;

typedef struct DXG_ENUM_DEVICEINFO
{
    // 3D 设备信息。
    TCHAR			szDeviceDesc[256];
    GUID*			pDeviceGUID;
    D3DDEVICEDESC7	ddDeviceDesc;
    BOOL			f3DHardware;
    // 绘制驱动信息。
	TCHAR			szDriverDesc[256];
    GUID*			pDriverGUID;
    DDCAPS			ddDriverCaps;
    DDCAPS			ddHELCaps;

    GUID            guidDevice;
    GUID            guidDriver;
    DDSURFACEDESC2* pddsdModes;
    DWORD           dwNumModes;
}DXG_ENUM_DEVICEINFO, *LPDXG_ENUM_DEVICEINFO;

typedef struct tagDXG_MASKINFO
{
	BYTE			bRCnt;
	BYTE			bGCnt;
	BYTE			bBCnt;
	BYTE			bRShift;
	BYTE			bGShift;
	BYTE			bBShift;
	DWORD			dwRMask;
	DWORD			dwGMask;
	DWORD			dwBMask;
}DXG_MASKINFO, *LPDXG_MASKINFO;


class CWHDXGraphicWindow : public CWHWindow
{
//1: Constuctor/Destructor
public:
	CWHDXGraphicWindow(WORD wWidth = _SCREEN_WIDTH, WORD wHeight = _SCREEN_HEIGHT, WORD wBpp = _SCREEN_BPP);
	~CWHDXGraphicWindow();

//2: Variables
protected:
	LPDIRECT3D7				m_pD3D;
    LPDIRECTDRAW7			m_pDD;
	LPDIRECT3DDEVICE7		m_pd3dDevice;
    LPDIRECTDRAWSURFACE7	m_pddsFrontBuffer;
    LPDIRECTDRAWSURFACE7	m_pddsBackBuffer;
	LPDIRECTDRAWSURFACE7	m_pddsZBuffer;
	LPDIRECTDRAWCLIPPER		m_lpcClipper;

///////<---------------------------------------------------------------------------------------------------------------------------
	HFONT					m_hDefGameFont;
///////<---------------------------------------------------------------------------------------------------------------------------

	BYTE					m_bScreenModeFlag;
    BYTE					m_bDeviceModeFlag;

public:
	DWORD					m_dwTextureTotal;	// 全部可使用的内存.
	DWORD					m_dwTextureFree;    // 当前剩余内存.
	DWORD					m_dwVideoTotal; 	// vram 全部内存.
	DWORD					m_dwVideoFree;      // 当前剩余的 vram 内存 .

	BOOL					m_bIsWindowActive;
	BOOL					m_bIsWindowReady;

    RECT					m_rcWindow;
	DXG_DISPLAYINFO			m_stDisplayInfo;
	DXG_MASKINFO			m_stBitsMaskInfo;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 用于分割进程的变量声明.
	static CWHDXGraphicWindow*	m_pxDXGWnd;
	CWHDefProcess*				m_pxDefProcess;

	VOID					(*RenderProcess)(int);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//3: Methods
protected:
	BYTE	FindDriverAndDevice();
	VOID	FreeDXGEnumModeResources();

	HRESULT CreateDXG();
	HRESULT	CreatePrimarySurface();
	HRESULT CreateZBuffer(GUID* pDeviceGUID);
	HRESULT Init3DDeviceObjects();
	HRESULT DestroyDXGObjects();

public:

    __inline LPDIRECTDRAW7			GetDirectDraw()     { return m_pDD;				}
    __inline LPDIRECT3D7			GetDirect3D()		{ return m_pD3D;			}
	__inline LPDIRECT3DDEVICE7		Get3DDevice()		{ return m_pd3dDevice;		}
    __inline LPDIRECTDRAWSURFACE7	GetFrontBuffer()    { return m_pddsFrontBuffer; }
    __inline LPDIRECTDRAWSURFACE7	GetBackBuffer()     { return m_pddsBackBuffer;	}
    __inline BYTE					GetDeviceMode()     { return m_bDeviceModeFlag; }
	__inline BYTE					GetScreenMode()		{ return m_bScreenModeFlag; }

    DXG_MASKINFO			GetRGBMaskInfoIN16Bits(LPDIRECTDRAWSURFACE7 pSurface);
	virtual VOID			UpdateBoundsWnd();

	HRESULT					ResetDXG(WORD wWidth, WORD wHeight, WORD wBPP, BYTE bScreenModeFlag, BYTE bDeviceModeFlag);
	BOOL					Create(HINSTANCE hInst, LPTSTR lpCaption = NULL, CHAR *pszMenuName = NULL, CHAR* pszIconName = NULL, BYTE bScreenModeFlag = _DXG_SCREENMODE_WINDOW, BYTE bDeviceModeFlag = _DXG_DEVICEMODE_PRIMARY);

///////<---------------------------------------------------------------------------------------------------------------------------
	HFONT					CreateGameFont(LPCSTR szFontName, INT nHeight, INT nWidth = 0, INT nWeight = FW_NORMAL, BOOL bItalic = FALSE, BOOL bULine = FALSE, BOOL bStrikeOut = FALSE, DWORD dwCharSet = HANGEUL_CHARSET);
	VOID					CreateDefFont();

	VOID					PutsHan(LPDIRECTDRAWSURFACE7 pSurface, INT nX, INT nY, COLORREF foreColor, COLORREF backColor, CHAR* szText, HFONT hFont = NULL);
	VOID					PutsHan(LPDIRECTDRAWSURFACE7 pSurface, RECT rc, COLORREF foreColor, COLORREF backColor, CHAR* szText, HFONT hFont = NULL);

	SIZE					GetStrLength(LPDIRECTDRAWSURFACE7 pSurface, HFONT hFont, CHAR* szFormat, ...);
///////<---------------------------------------------------------------------------------------------------------------------------

	BOOL 					StringDivide(INT nDivideWidth, INT& nDividedLine, CHAR* szSrc, CHAR* szResult);
//	BOOL					StringDivideLen(INT nDivideLen, INT& nDividedLine, CHAR* szSrc, CDLList<CHAR*>* m_pxpStr);

	VOID 					StringPlus(CHAR* szResult, CHAR* szSrc, ...);
	CHAR*					IntToStr(INT nNum);

	VOID					UsedAndFreeMemoryCheck();


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	WORD					ConvertColor24To16(COLORREF dwColor);
	// 渲染函数集合.
	// 1. GDI 用绘制函数.(将线、矩形等绘制到背面表面.)
	HRESULT					DrawWithGDI(RECT rc, LPDIRECTDRAWSURFACE7 pSurface, DWORD dwColor, BYTE bKind);
	// 2. WORD 型数据绘制函数.(按像素单位检查颜色键, 绘制到背景表面上.)
	BOOL					DrawWithImage(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc);
	// 3. WORD 型数据绘制函数。(按行不考虑颜色键，只在后台表面的裁剪内部区域绘制。)
	BOOL					DrawWithImagePerLineClipRgn(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight);
	// 4. Surface 型数据绘制函数。(考虑源的颜色键并绘制到后台表面。)
	BOOL					DrawWithSurface(INT nX, INT nY, INT nXSize, INT nYSize, LPDIRECTDRAWSURFACE7 pddsSrc);

	// wChooseColor1 用于染色衣服(0xC2), wChooseColor2(0xC3) 用于头发等(刀, 其他变化的颜色)的染色.
	// 5. 压缩(0 号)及染色的 WORD 型数据绘制函数.(绘制到后台表面.)
	BOOL					DrawWithImageForComp(INT nX, INT nY, RECT rcSrc, WORD* pwSrc, WORD wChooseColor1 = 0XFFFF, WORD wChooseColor2 = 0XFFFF);
	BOOL					DrawWithImageForComp(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wChooseColor1 = 0XFFFF, WORD wChooseColor2 = 0XFFFF);
	// 6. 压缩(0号)及染色后的 WORD 型数据绘制函数.(仅绘制在白色表面的裁剪内部区域.)
	BOOL					DrawWithImageForCompClipRgnBase(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight, WORD wChooseColor1 = 0XFFFF, WORD wChooseColor2 = 0XFFFF);
	__inline VOID			DrawWithImageForCompClipRgn(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight, WORD wChooseColor1 = 0XFFFF, WORD wChooseColor2 = 0XFFFF, BOOL bLighted = FALSE)
	{
		if ( bLighted )		DrawWithABlendCompDataWithLightedColor(nX, nY, nXSize, nYSize, pwSrc, wClipWidth, wClipHeight, wChooseColor1, wChooseColor2);
		else				DrawWithImageForCompClipRgnBase(nX, nY, nXSize, nYSize, pwSrc, wClipWidth, wClipHeight, wChooseColor1, wChooseColor2);
	}


	// 7. 绘制压缩(0 号)及染色后的 WORD 型数据的函数.(从源内存区域绘制到目标内存区域.)
	BOOL					DrawWithImageForCompMemToMem(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, INT nDstXSize, INT nDstYSize, WORD* pwDst, WORD wChooseColor1 = 0XFFFF, WORD wChooseColor2 = 0XFFFF);
	// 8. 绘制压缩(0 号)及染色后的 WORD 型数据的函数.(只把压缩数据源内存区域中的一部分矩形区域绘制到目地区域内存.)
	BOOL					DrawWithImageForCompToMem(RECT rcWanted, WORD* pwSrc, WORD wChooseColor1, WORD wChooseColor2, WORD* pwDst);
	BOOL					DrawWithImageForCompColorToMem(RECT rcWanted, WORD* pwSrc, WORD wColor, WORD* pwDst);
	// 9. 压缩(0号)及染色的 WORD 型数据的绘制函数。(把压缩数据源区域与目标区域的交叉矩形做 alpha 混合后绘制到后台表面。)
	BOOL					DrawWithABlendForIntersectCompData(INT nSrcX, INT nSrcY, 
															   INT nSrcXSize, INT nSrcYSize, WORD* pwSrc,
															   INT nDstX, INT nDstY,
															   INT nDstXSize, INT nDstYSize, WORD* pwDst, 
															   WORD wClipWidth, WORD wClipHeight,
															   BYTE bOpa = 50, BOOL bFocused = FALSE,
															   WORD wSrcChooseColor1 = 0XFFFF, WORD wSrcChooseColor2 = 0XFFFF,
															   WORD wDstChooseColor1 = 0XFFFF, WORD wDstChooseColor2 = 0XFFFF,
															   WORD wSrcColor = 0XFFFF, WORD wDstColor = 0XFFFF);
	// 10. 压缩(0 号)及染色的 WORD 型数据绘制函数。(将压缩数据的源区域与背景表面进行 alpha 混合。)
	BOOL					DrawWithABlendCompDataWithBackBuffer(INT nX, INT nY, 
																 INT nXSize, INT nYSize, WORD* pwSrc,
																 WORD wClipWidth, WORD wClipHeight,
																 WORD wChooseColor1 = 0XFFFF, WORD wChooseColor2 = 0XFFFF, BYTE bOpa = 50);
	// 11. 压缩(0 号)及染色后的 WORD 型数据绘制函数.(把压缩数据的源区域以想要的色彩、以光照状态(变亮)绘制到背景表面.)
	BOOL					DrawWithABlendCompDataWithLightedColor(INT nX, INT nY, 
																   INT nXSize, INT nYSize, WORD* pwSrc,
																   WORD wClipWidth, WORD wClipHeight,
																   WORD wChooseColor1 = 0XFFFF, WORD wChooseColor2 = 0XFFFF);
	// 阴影绘制函数.
	// 12. 压缩(0号)及染色后的 WORD 型数据绘制函数.(将压缩后数据源区域的影子以阿尔法混合绘制到白色表面上.影子的高度减半,宽度增加 YSize,倾斜地绘制到白色表面上.)
	BOOL					DrawWithShadowABlend(INT nX, INT nY, 
												 INT nXSize, INT nYSize, WORD* pwSrc,
												 WORD wClipWidth, WORD wClipHeight, WORD* pwShadowClrSrc,
												 BOOL bBlend = FALSE, BYTE bShadowType = 48, BYTE bOpa = 50);
	VOID					DrawWithShadowABlend(INT nX, INT nY, INT nXSize, INT nYSize, 
		                                         INT nPX, INT nPY, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight, WORD* pwShadowClrSrc,
												 BOOL bBlend = FALSE, BYTE bOpa = 50);
	// 颜色处理.
	// 13.
	BOOL					DrawWithImageForCompClipRgnColor(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight, WORD wColor, BOOL bFocused = FALSE, BOOL bBlend = FALSE);
	BOOL					DrawWithImageForCompClipRgnColor(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc, WORD wClipWidth, WORD wClipHeight, WORD wColor, WORD wChooseColor1 = 0XFFFF, WORD wChooseColor2 = 0XFFFF);
	// 14.
	BOOL					DrawWithImageForCompClipRgnGray(INT nX, INT nY, INT nXSize, INT nYSize, WORD* pwSrc , WORD wClipWidth, WORD wClipHeight, WORD wChooseColor1 = 0XFFFF, WORD wChooseColor2 = 0XFFFF);
	// 黑白处理.
	// 15.
	VOID					DrawWithGrayBackBuffer();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	HRESULT					Present();
	HRESULT					RestoreSurfaces();

//4: Message Map
public:
	virtual LRESULT MainWndProcDXG(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnSize(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnMove(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnSysKeyDown(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnGetMinMaxInfo(WPARAM wParam, LPARAM lParam);


protected:

	__inline virtual LRESULT MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{ 
		if ( m_pxDefProcess )	return m_pxDefProcess->DefMainWndProc(hWnd, uMsg, wParam, lParam);
		else					return MainWndProcDXG(hWnd, uMsg, wParam, lParam); 
	}

	virtual LRESULT	OnDestroy();
	virtual LRESULT	OnSetCursor();
};

CWHDXGraphicWindow*		GetDXGWindowPtr();

#endif //_WINDHORN_DXGRAPHIC
