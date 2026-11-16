/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/
#include "StdAfx.h"


struct WILTextureContainer
{
    WILTextureContainer* m_pNext;

    TCHAR   m_strWilFileName[80];
	INT		m_nIdx;
	WORD*	m_pwData;				// 仅在以 Surface 加载时才定义值. 除此之外始终为 NULL.<---???
    DWORD   m_dwFlags;
    DWORD   m_dwWidth;
    DWORD   m_dwHeight;
    LPDIRECTDRAWSURFACE7 m_pddsSurface;

public:
//  HRESULT LoadImageData();
    HRESULT Restore(LPDIRECT3DDEVICE7 pd3dDevice);
    HRESULT CopyDataToSurface();
	HRESULT CopyDataToSurfaceToSurface();

    WILTextureContainer(TCHAR* strWilFileName, INT nIndex, DWORD dwFlags);
    ~WILTextureContainer();
};
static WILTextureContainer* g_ptcWILTextureList = NULL;


class CD3DWILTextureManager
{
public:
    CD3DWILTextureManager () {}
    ~CD3DWILTextureManager() { if ( g_ptcWILTextureList ) delete g_ptcWILTextureList; }
};

CD3DWILTextureManager g_StaticWILTextureEngine;


struct TEXTURESEARCHINFO
{
    DWORD dwDesiredBPP;
    BOOL  bUseAlpha;
    BOOL  bUsePalette;
    BOOL  bFoundGoodFormat;

    DDPIXELFORMAT* pddpf;
};


static HRESULT CALLBACK TextureSearchCallback(DDPIXELFORMAT* pddpf, VOID* param)
{
    if ( NULL==pddpf || NULL==param )
        return DDENUMRET_OK;

    TEXTURESEARCHINFO* ptsi = (TEXTURESEARCHINFO*)param;

    if ( pddpf->dwFlags & (DDPF_LUMINANCE|DDPF_BUMPLUMINANCE|DDPF_BUMPDUDV) )
        return DDENUMRET_OK;

    if ( ptsi->bUsePalette )
    {
        if( !(pddpf->dwFlags & DDPF_PALETTEINDEXED8) )
            return DDENUMRET_OK;
        memcpy(ptsi->pddpf, pddpf, sizeof(DDPIXELFORMAT));
        ptsi->bFoundGoodFormat = TRUE;
        return DDENUMRET_CANCEL;
    }

    if ( pddpf->dwRGBBitCount < 16 )
        return DDENUMRET_OK;

    if ( pddpf->dwFourCC != 0 )
        return DDENUMRET_OK;

    if( pddpf->dwRGBAlphaBitMask == 0x0000f000 )
        return DDENUMRET_OK;

    if ( (ptsi->bUseAlpha==TRUE) && !(pddpf->dwFlags&DDPF_ALPHAPIXELS) )
        return DDENUMRET_OK;
    if ( (ptsi->bUseAlpha==FALSE) && (pddpf->dwFlags&DDPF_ALPHAPIXELS) )
        return DDENUMRET_OK;

    if ( pddpf->dwRGBBitCount == ptsi->dwDesiredBPP )
    {
        memcpy(ptsi->pddpf, pddpf, sizeof(DDPIXELFORMAT));
        ptsi->bFoundGoodFormat = TRUE;
        return DDENUMRET_CANCEL;
    }

    return DDENUMRET_OK;
}


static WILTextureContainer* FindWilTexture(TCHAR* strWilFileName, INT nIdx)
{
    WILTextureContainer* ptcWILTexture = g_ptcWILTextureList;

    while ( ptcWILTexture )
    {
        if ( !lstrcmpi(strWilFileName, ptcWILTexture->m_strWilFileName) &&
			 nIdx == ptcWILTexture->m_nIdx )
		{
            return ptcWILTexture;
		}
        ptcWILTexture = ptcWILTexture->m_pNext;
    }

    return NULL;
}


BOOL D3DWILTextr_IsTextrExisted(TCHAR* strWilFileName, INT nIdx)
{
    if ( NULL != FindWilTexture(strWilFileName, nIdx) )
        return TRUE;
	return FALSE;
}


WILTextureContainer::WILTextureContainer(TCHAR* strWilFileName, INT nIndex, DWORD dwFlags)
{
    lstrcpy(m_strWilFileName, strWilFileName);
	m_nIdx				= nIndex;
    m_dwWidth			= 0;
    m_dwHeight			= 0;
    m_dwFlags			= dwFlags;
    m_pddsSurface		= NULL;
	m_pwData			= NULL;
    m_pNext				= g_ptcWILTextureList;
    g_ptcWILTextureList	= this;
}


WILTextureContainer::~WILTextureContainer()
{
    SAFE_RELEASE( m_pddsSurface );

    if ( g_ptcWILTextureList == this )
        g_ptcWILTextureList = m_pNext;
    else
    {
        for ( WILTextureContainer* ptc = g_ptcWILTextureList; ptc; ptc=ptc->m_pNext )
            if ( ptc->m_pNext == this )
                ptc->m_pNext = m_pNext;
    }
}


HRESULT WILTextureContainer::Restore(LPDIRECT3DDEVICE7 pd3dDevice)
{
    SAFE_RELEASE(m_pddsSurface);

    if( NULL == pd3dDevice )
        return DDERR_INVALIDPARAMS;

    D3DDEVICEDESC7 ddDesc;
    if ( FAILED(pd3dDevice->GetCaps(&ddDesc)) )
        return E_FAIL;

    DDSURFACEDESC2 ddsd;
    D3DUtil_InitSurfaceDesc( ddsd );
    ddsd.dwFlags         = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|
                           /*DDSD_PIXELFORMAT|*/DDSD_TEXTURESTAGE;
    ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
    ddsd.dwTextureStage  = 0;
    ddsd.dwWidth         = m_dwWidth;
    ddsd.dwHeight        = m_dwHeight;

    if ( ddDesc.deviceGUID == IID_IDirect3DHALDevice || ddDesc.deviceGUID == IID_IDirect3DTnLHalDevice )
        ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
    else
        ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

	// Setting Power of 2
    if ( ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2 )
    {
        for ( ddsd.dwWidth =1; m_dwWidth>ddsd.dwWidth;   ddsd.dwWidth<<=1 );
        for ( ddsd.dwHeight=1; m_dwHeight>ddsd.dwHeight; ddsd.dwHeight<<=1 );
    }

    DWORD dwMaxWidth  = ddDesc.dwMaxTextureWidth;
    DWORD dwMaxHeight = ddDesc.dwMaxTextureHeight;
    ddsd.dwWidth  = min(ddsd.dwWidth,  (dwMaxWidth  ? dwMaxWidth  : 256));
    ddsd.dwHeight = min(ddsd.dwHeight, (dwMaxHeight ? dwMaxHeight : 256));

	// 如果不支持矩形的纹理...
    if ( ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY )
    {
        if ( ddsd.dwWidth > ddsd.dwHeight ) ddsd.dwHeight = ddsd.dwWidth;
        else                                ddsd.dwWidth  = ddsd.dwHeight;
    }


	// 纹理像素格式设置...
    TEXTURESEARCHINFO tsi;
    tsi.bFoundGoodFormat = FALSE;
    tsi.pddpf            = &ddsd.ddpfPixelFormat;
    tsi.dwDesiredBPP     = 16;
    tsi.bUsePalette      = ( 16 <= 8 );
    tsi.bUseAlpha        = FALSE;
    tsi.dwDesiredBPP	 = 16;

    pd3dDevice->EnumTextureFormats( TextureSearchCallback, &tsi );

    if( FALSE == tsi.bFoundGoodFormat )
    {
		return E_FAIL;
    }

    LPDIRECTDRAW7        pDD;
    LPDIRECTDRAWSURFACE7 pddsRender;
    pd3dDevice->GetRenderTarget(&pddsRender);
    pddsRender->GetDDInterface((VOID**)&pDD);
    pddsRender->Release();

    HRESULT hr = pDD->CreateSurface(&ddsd, &m_pddsSurface, NULL);

    pDD->Release();

    if( FAILED(hr) )
        return hr;

	if ( ddsd.dwWidth == m_dwWidth && ddsd.dwHeight == m_dwHeight )
		CopyDataToSurface();
	else
		CopyDataToSurfaceToSurface();

	// 设置颜色键。
	DDSetColorKey(m_pddsSurface, RGB(0, 0, 0));

    return S_OK;
}


HRESULT WILTextureContainer::CopyDataToSurfaceToSurface()
{
	LPDIRECTDRAW7 pDD;
    m_pddsSurface->GetDDInterface((VOID**)&pDD);

    DDSURFACEDESC2 ddsd;
    ddsd.dwSize = sizeof(ddsd);
    m_pddsSurface->GetSurfaceDesc(&ddsd);
    ddsd.dwFlags         = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|//DDSD_PIXELFORMAT|
                           DDSD_TEXTURESTAGE;
    ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE|DDSCAPS_SYSTEMMEMORY;
    ddsd.ddsCaps.dwCaps2 = 0L;
    ddsd.dwWidth         = m_dwWidth;
    ddsd.dwHeight        = m_dwHeight;

    LPDIRECTDRAWSURFACE7 pddsTempSurface;
    HRESULT hr;
    if ( FAILED(hr = pDD->CreateSurface(&ddsd, &pddsTempSurface, NULL) ) )
    {
        pDD->Release();
        return NULL;
    }

    while ( pddsTempSurface->Lock(NULL, &ddsd, 0, 0) == DDERR_WASSTILLDRAWING );

    DWORD lPitch = ddsd.lPitch;
    WORD* pwData = (WORD*)ddsd.lpSurface;






	RECT			rc;
	SetRect(&rc, 0, 0, m_dwWidth, m_dwHeight);
			
	WORD* pwdDst = pwData;

	INT nWidthStart	= 0;
	INT nWidthEnd	= 0;
	INT nCurrWidth  = 0;
	INT nCntCopyWord = 0;
	INT nYCnt =0;
	INT nLastWidth = 0;

	// y 轴裁剪。
	for ( nYCnt=0 ; nYCnt < rc.top ; nYCnt++ )
	{
		nWidthEnd += m_pwData[nWidthStart];
		nWidthStart++;
		nWidthEnd++;
		nWidthStart	= nWidthEnd;
	}

	// 只决定对 y 轴实际进行循环的 Count.
	for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
	{
		// 获取一行的长度.(单位为字)
		nWidthEnd += m_pwData[nWidthStart];
		nWidthStart++;
		
		// 按行的长度输出到屏幕。
		for ( INT x = nWidthStart; x < nWidthEnd ; )
		{
			if ( m_pwData[x] == 0xC0 )
			{
				x++;
				nCntCopyWord = m_pwData[x];
				x++;
				nCurrWidth += nCntCopyWord;
			}
			else if ( m_pwData[x] == 0xC1 )
			{
				x++;
				nCntCopyWord = m_pwData[x];
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
						memcpy(&pwdDst[((nYCnt) * (ddsd.lPitch >> 1)) + (rc.left)], &m_pwData[x], sizeof(WORD)*(nCurrWidth-rc.left));
						x += (nCurrWidth-rc.left);
					}
					// 以 rc.right 点为基准，复制区域被截断的情况.
					else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
					{
						memcpy(&pwdDst[((nYCnt) * (ddsd.lPitch >> 1)) + (nLastWidth)], &m_pwData[x], sizeof(WORD)*(rc.right-nLastWidth));
						x += nCntCopyWord;
					}
					else
					{
						memcpy(&pwdDst[((nYCnt) * (ddsd.lPitch >> 1)) + (nLastWidth)], &m_pwData[x], sizeof(WORD)*nCntCopyWord);
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



    


	pddsTempSurface->Unlock(0);
    m_pddsSurface->Blt(NULL, pddsTempSurface, NULL, DDBLT_WAIT, NULL);
    pddsTempSurface->Release();
    pDD->Release();

    return S_OK;
}


HRESULT WILTextureContainer::CopyDataToSurface()
{
	/*  
	LPDIRECTDRAW7 pDD;
    m_pddsSurface->GetDDInterface((VOID**)&pDD);

    DDSURFACEDESC2 ddsd;
    ddsd.dwSize = sizeof(ddsd);
    m_pddsSurface->GetSurfaceDesc(&ddsd);
    ddsd.dwFlags         = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|//DDSD_PIXELFORMAT|
                           DDSD_TEXTURESTAGE;
    ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE|DDSCAPS_SYSTEMMEMORY;
    ddsd.ddsCaps.dwCaps2 = 0L;
    ddsd.dwWidth         = m_dwWidth;
    ddsd.dwHeight        = m_dwHeight;

    LPDIRECTDRAWSURFACE7 pddsTempSurface;
    HRESULT hr;
    if ( FAILED(hr = pDD->CreateSurface(&ddsd, &pddsTempSurface, NULL) ) )
    {
        pDD->Release();
        return NULL;
    }

    while ( pddsTempSurface->Lock(NULL, &ddsd, 0, 0) == DDERR_WASSTILLDRAWING );

    DWORD lPitch = ddsd.lPitch;
    WORD* pwData = (WORD*)ddsd.lpSurface;
	memcpy(pwData, m_pwData, sizeof(WORD)*m_dwWidth*m_dwHeight);

    pddsTempSurface->Unlock(0);

    m_pddsSurface->Blt(NULL, pddsTempSurface, NULL, DDBLT_WAIT, NULL);

    pddsTempSurface->Release();
    pDD->Release();
	*/

	RECT			rc;
	DDSURFACEDESC2	ddsd;
	
	SetRect(&rc, 0, 0, m_dwWidth, m_dwHeight);

	ddsd.dwSize	= sizeof(DDSURFACEDESC2);
	ddsd.lpSurface = NULL;
			
	m_pddsSurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
	if ( !ddsd.lpSurface )			
		return NULL;

	WORD* pwdDst;

	pwdDst = (WORD*)ddsd.lpSurface;

	INT nWidthStart	= 0;
	INT nWidthEnd	= 0;
	INT nCurrWidth  = 0;
	INT nCntCopyWord = 0;
	INT nYCnt =0;
	INT nLastWidth = 0;

	// y 轴裁剪。
	for ( nYCnt=0 ; nYCnt < rc.top ; nYCnt++ )
	{
		nWidthEnd += m_pwData[nWidthStart];
		nWidthStart++;
		nWidthEnd++;
		nWidthStart	= nWidthEnd;
	}

	// 只决定对 y 轴实际进行循环的 Count.
	for ( nYCnt=rc.top ; nYCnt < rc.bottom ; nYCnt++ )
	{
		// 获取一行的长度.(单位为字)
		nWidthEnd += m_pwData[nWidthStart];
		nWidthStart++;
		
		// 按行的长度输出到屏幕。
		for ( INT x = nWidthStart; x < nWidthEnd ; )
		{
			if ( m_pwData[x] == 0xC0 )
			{
				x++;
				nCntCopyWord = m_pwData[x];
				x++;
				nCurrWidth += nCntCopyWord;
			}
			else if ( m_pwData[x] == 0xC1 )
			{
				x++;
				nCntCopyWord = m_pwData[x];
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
						memcpy(&pwdDst[((nYCnt) * (ddsd.lPitch >> 1)) + (rc.left)], &m_pwData[x], sizeof(WORD)*(nCurrWidth-rc.left));
						x += (nCurrWidth-rc.left);
					}
					// 以 rc.right 点为基准，复制区域被截断的情况.
					else if ( nLastWidth <= rc.right && rc.right < nCurrWidth )
					{
						memcpy(&pwdDst[((nYCnt) * (ddsd.lPitch >> 1)) + (nLastWidth)], &m_pwData[x], sizeof(WORD)*(rc.right-nLastWidth));
						x += nCntCopyWord;
					}
					else
					{
						memcpy(&pwdDst[((nYCnt) * (ddsd.lPitch >> 1)) + (nLastWidth)], &m_pwData[x], sizeof(WORD)*nCntCopyWord);
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

	m_pddsSurface->Unlock(NULL);
    return S_OK;
}


/*
HRESULT D3DTextr_CreateWILTextureFromFile(TCHAR* strWilFileName, INT nIndex, DWORD dwFlags)
{
    if ( NULL == strWilFileName )
        return E_INVALIDARG;

    if ( NULL != FindWilTexture(strWilFileName, nIndex) )
        return S_OK;

    WILTextureContainer* ptcWILTexture = new WILTextureContainer(strWilFileName, dwFlags);
    if ( NULL == ptcWILTexture )
        return E_OUTOFMEMORY;

    if ( FAILED(ptcWILTexture->LoadImageData()) )
    {
        delete ptcWILTexture;
        return E_FAIL;
    }

//  ptcWILTexture->m_dwWidth  = ;
//  ptcWILTexture->m_dwHeight = ;

    return S_OK;
}
*/


BOOL D3DWILTextr_CreateEmptyTexture(TCHAR* strWilFileName, INT nIndex, 
									   DWORD dwWidth, DWORD dwHeight, WORD* pwData, DWORD dwFlags)
{
    if ( NULL == strWilFileName )
        return FALSE;

    if ( NULL != FindWilTexture(strWilFileName, nIndex) )
        return FALSE;

    WILTextureContainer* ptcWILTexture = new WILTextureContainer(strWilFileName, nIndex, dwFlags);

    if ( NULL == ptcWILTexture )
        return FALSE;

    ptcWILTexture->m_dwWidth  = dwWidth;
    ptcWILTexture->m_dwHeight = dwHeight;
	ptcWILTexture->m_pwData	  = pwData;

    return TRUE;
}


HRESULT D3DWILTextr_Restore(TCHAR* strWilFileName, INT nIndex, LPDIRECT3DDEVICE7 pd3dDevice)
{
    WILTextureContainer* ptcWILTexture = FindWilTexture(strWilFileName, nIndex);
    if ( NULL == ptcWILTexture )
        return DDERR_NOTFOUND;

    return ptcWILTexture->Restore(pd3dDevice);
}


LPDIRECTDRAWSURFACE7 D3DWILTextr_RestoreEx(TCHAR* strWilFileName, INT nIndex, LPDIRECT3DDEVICE7 pd3dDevice)
{
    WILTextureContainer* ptcWILTexture = FindWilTexture(strWilFileName, nIndex);

	if ( ptcWILTexture )
	{
		if ( ptcWILTexture->m_pddsSurface )
		{
			return ptcWILTexture->m_pddsSurface;
		}
		else
		{
			if ( (ptcWILTexture->Restore(pd3dDevice) == S_OK) )
			{
				return ptcWILTexture->m_pddsSurface;
			}
			else
				return NULL;
		}
	}

	return NULL;
		
}


HRESULT D3DWILTextr_RestoreAllTextures(LPDIRECT3DDEVICE7 pd3dDevice)
{
    WILTextureContainer* ptcWILTexture = g_ptcWILTextureList;

    while ( ptcWILTexture && ptcWILTexture->m_pddsSurface )
    {
        D3DWILTextr_Restore(ptcWILTexture->m_strWilFileName, ptcWILTexture->m_nIdx, pd3dDevice);
        ptcWILTexture = ptcWILTexture->m_pNext;
    }

    return S_OK;
}


HRESULT D3DWILTextr_Invalidate(TCHAR* strWilFileName, INT nIndex)
{
    WILTextureContainer* ptcWILTexture = FindWilTexture(strWilFileName, nIndex);
    if ( NULL == ptcWILTexture )
        return DDERR_NOTFOUND;

    SAFE_RELEASE(ptcWILTexture->m_pddsSurface);

    return S_OK;
}


HRESULT D3DWILTextr_InvalidateAllTextures()
{
    WILTextureContainer* ptcWILTexture = g_ptcWILTextureList;

    while( ptcWILTexture )
    {
        SAFE_RELEASE(ptcWILTexture->m_pddsSurface);
        ptcWILTexture = ptcWILTexture->m_pNext;
    }

    return S_OK;
}

HRESULT D3DWILTextr_DestroyAllTextures()
{
    WILTextureContainer* ptcWILTexture = g_ptcWILTextureList;
	WILTextureContainer* ptcWILMextTexture = NULL;

    while( ptcWILTexture )
    {
        ptcWILMextTexture = ptcWILTexture->m_pNext;

		SAFE_DELETE(ptcWILTexture);

		ptcWILTexture = ptcWILMextTexture;
		
    }
    return S_OK;
}

HRESULT D3DWILTextr_DestroyTexture(TCHAR* strWilFileName, INT nIndex)
{
    WILTextureContainer* ptcWILTexture = FindWilTexture(strWilFileName, nIndex);

    SAFE_DELETE(ptcWILTexture);

    return S_OK;
}


LPDIRECTDRAWSURFACE7 D3DWILTextr_GetSurface(TCHAR* strWilFileName, INT nIndex)
{
    WILTextureContainer* ptcWILTexture = FindWilTexture(strWilFileName, nIndex);

    return ptcWILTexture ? ptcWILTexture->m_pddsSurface : NULL;
}


/*
POINT D3DWILTextr_GetImageSize(TCHAR* strWilFileName, INT nIndex)
{
	POINT ptSize = {0 , 0};
    WILTextureContainer* ptcWILTexture = FindWilTexture(strWilFileName, nIndex);

	if ( ptcWILTexture )
	{
		ptSize.x = ptcWILTexture->m_dwWidth; 
		ptSize.y = ptcWILTexture->m_dwHeight;
	}
    return ptSize;
}
*/

INT	GetRandomNum(INT nFrom, INT nTo)
{
	INT nRandNum = 0;

	INT nRand = ((INT)rand()-(INT)rand());

	if ( nRand < 0 )
		nRand = -nRand;

	nRandNum = nRand % (nTo-nFrom+1) + nFrom;

	return nRandNum;
}

