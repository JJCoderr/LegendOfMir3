/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/

#ifndef D3DWILTEXTR_H
#define D3DWILTEXTR_H
#include <ddraw.h>
#include <d3d.h>

#define D3DTEXTR_TRANSPARENTWHITE 0x00000001
#define D3DTEXTR_TRANSPARENTBLACK 0x00000002

LPDIRECTDRAWSURFACE7 D3DWILTextr_GetSurface(TCHAR* strWilFileName, INT nIndex);
HRESULT D3DWILTextr_Invalidate(TCHAR* strWilFileName, INT nIndex);
HRESULT D3DWILTextr_Restore(TCHAR* strWilFileName, INT nIndex, LPDIRECT3DDEVICE7 pd3dDevice);
LPDIRECTDRAWSURFACE7 D3DWILTextr_RestoreEx(TCHAR* strWilFileName, INT nIndex, LPDIRECT3DDEVICE7 pd3dDevice);
HRESULT D3DWILTextr_InvalidateAllTextures();
HRESULT D3DWILTextr_DestroyAllTextures();
HRESULT D3DWILTextr_RestoreAllTextures(LPDIRECT3DDEVICE7 pd3dDevice);
//HRESULT D3DWILTextr_CreateTextureFromFile(TCHAR* strWilFileName, INT nIndex, DWORD dwFlags=0L);
BOOL	D3DWILTextr_CreateEmptyTexture(TCHAR* strWilFileName, INT nIndex, 
									   DWORD dwWidth, DWORD dwHeight, WORD* pwData, DWORD dwFlags);
HRESULT D3DWILTextr_DestroyTexture(TCHAR* strWilFileName, INT nIndex);
VOID    D3DWILTextr_SetTexturePath(TCHAR* strTexturePath);
BOOL    D3DWILTextr_IsTextrExisted(TCHAR* strWilFileName, INT nIdx);
INT		GetRandomNum(INT nFrom, INT nTo);

//POINT	D3DWILTextr_GetImageSize(TCHAR* strWilFileName, INT nIndex);


// 管理用作纹理的所有文件.
/*
    WILTextureContainer* m_pNext;

    TCHAR   m_strWilFileName[80];
	INT		m_nIdx;
    DWORD   m_dwFlags;
    DWORD   m_dwWidth;
    DWORD   m_dwHeight;
    LPDIRECTDRAWSURFACE7 m_pddsSurface;
*/
// [概述]  使用文件映射访问数据.(使用 WIL 文件与 WIX 文件)
//    ---> CMappedWilFile 类中的 m_strWilFileName, m_nIdx, m_dwWidth, 
//		   m_dwHeight, 图像数据地址, 创建用于纹理的表面.


#endif // D3DWILTEXTR_H
