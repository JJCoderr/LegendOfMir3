// Mir2Ex dialogs and panels.
#include "StdAfx.h"

// MsgBox
//#include "MsgBox.h"


CMsgBox::CMsgBox()	// Constructor
{
	m_nType = 0;
	m_bActive = FALSE;
	m_bInited = FALSE;
	m_bSet = FALSE;
}

CMsgBox::~CMsgBox()	// Destructor
{

}
	// Function

/*
*/
BOOL CMsgBox::SetMsgBox(CHAR* szMsg,INT nType)				// Init Messages
{
	INT nLength;
	nLength = strlen(szMsg);
	if(nLength!=0)
	{
		ZeroMemory(m_szMsg,MAX_PATH);
		strcpy(m_szMsg,szMsg);
		m_nType = nType;
		m_bInited = TRUE;
	}
	else
		m_bInited = FALSE;
	return m_bInited;
}

BOOL CMsgBox::ShowMessageBox(VOID)							// Show Message box with Initialized
{
	m_bActive = TRUE;
	return m_bActive;
}

BOOL CMsgBox::ShowMessageBox(CHAR* szMsg,INT nType)		// Show	Message box with Initialization
{
	if(SetMsgBox(szMsg,nType))
		return ShowMessageBox();
	else
		m_bActive = FALSE;
	return m_bActive;
}

BOOL CMsgBox::HideMessageBox()							// Hide Message box
{
	m_bActive = FALSE;
	return TRUE;
}

BOOL CMsgBox::DestoryMessageBox(VOID)						// DestoryMessage box
{
	m_nType = 0;
	m_bActive = FALSE;
	m_bInited = FALSE;
	return TRUE;
}

HRESULT CMsgBox::OnButtonDown(LPARAM lParam, WPARAM wParam)		// Button Down
{
	HRESULT	hResult = 0;

	return hResult;
}

HRESULT	CMsgBox::OnButtonUp(LPARAM lParam,WPARAM wParam)		// Button Up
{
	HRESULT	hResult = 0;

	return hResult;
/*
#define HAVE_NO_EDITBOX		256
*/
}

BOOL CMsgBox::IsActive(VOID)
{
	return m_bActive;
}
BOOL CMsgBox::IsInited(VOID)
{
	return m_bInited;
}

	// Render Process
BOOL CMsgBox::RenderMessageBox(INT nLoopTIme)							// Render Function
{
	if(m_bInited&&m_bActive)
	{
		// Draw Text With Dlg Image
		return TRUE;
	}
	return FALSE;
}

#undef HAVE_NO_EDITBOX

// MirMsgBox
#define WRITESTRLENTH	50


CMirMsgBox::CMirMsgBox():CMsgBox()	// Constructor
{
	m_xBoxPos.x = 0;
	m_xBoxPos.y = 0;
	m_nImgIndex = 0;
}

BOOL CMirMsgBox::Load(CWHWilImageData* pxBtnImage=NULL)
{
	if(pxBtnImage!=NULL)
	{
		// Init Button;
		m_xMsgBtn[0].CreateGameBtn(pxBtnImage,241,243,0,0);	// OK
		m_xMsgBtn[1].CreateGameBtn(pxBtnImage,244,246,0,0);	// YES
		m_xMsgBtn[2].CreateGameBtn(pxBtnImage,247,249,0,0);	// NO
		m_xMsgBtn[3].CreateGameBtn(pxBtnImage,250,252,0,0);	// Cancle
		m_pxBtnImage = pxBtnImage;
		m_bSet = TRUE;
	}
	else
	{
		m_bSet = FALSE;
	}
	return m_bSet;
}

CMirMsgBox::~CMirMsgBox()	// Destructor
{
}

// Function
/*
*/

BOOL CMirMsgBox::ShowMessageBox(CHAR* szMsg,INT nType) // Show	Message box with Initialization
{
	return ShowMessageBox(szMsg,nType,239);
}

BOOL CMirMsgBox::ShowMessageBox(CHAR* szMsg,INT nType,INT nImgIndex) // Show	Message box with Initialization
{
	INT Lenth;
	if(!m_bActive)
	{
		if(SetBoxImage(nImgIndex,nType))
		{
			if(CMsgBox::ShowMessageBox(szMsg,nType))
			{
				// 在 m_xMsg 中存入 String
				m_xMsg.ClearAllNodes();
				StringDivideLen(WRITESTRLENTH,Lenth, m_szMsg,&m_xMsg);

				// Text Setting

				// Button Setting
				switch(m_nType)
				{
				case MSG_BTN_YESNO:
				case MSG_BTN_YESNO|MSG_EDITEXIST:
					{
						m_nStart=1;
						m_nEnd = 3;

						m_xMsgBtn[1].ChangeRect(m_xBoxPos.x+78,m_xBoxPos.y+134);
						m_xMsgBtn[2].ChangeRect(m_xBoxPos.x+210,m_xBoxPos.y+134);

						break;
					}
				case MSG_BTN_YESNOCANCLE:
				case MSG_BTN_YESNOCANCLE|MSG_EDITEXIST:
					{
						m_nStart=1;
						m_nEnd = 4;

						m_xMsgBtn[1].ChangeRect(m_xBoxPos.x+54,m_xBoxPos.y+134);
						m_xMsgBtn[2].ChangeRect(m_xBoxPos.x+144,m_xBoxPos.y+134);
						m_xMsgBtn[3].ChangeRect(m_xBoxPos.x+234,m_xBoxPos.y+134);
						break;
					}
				case MSG_BTN_OK:
					{
						m_nStart=0;
						m_nEnd = 1;

						m_xMsgBtn[0].ChangeRect(m_xBoxPos.x+144,m_xBoxPos.y+134);
						break;
					}
				case MSG_BTN_OK|MSG_EDITEXIST:
				default:
					{
						m_nStart=0;
						m_nEnd = 1;

						m_xMsgBtn[0].ChangeRect(m_xBoxPos.x+260,m_xBoxPos.y+134);
						break;
					}
				}
			}
			else
			{
				m_bActive = FALSE;
			}
		}
		else
		{
			m_bActive = FALSE;
			m_bInited = FALSE;
		}
	}
	return m_bActive;
}

BOOL CMirMsgBox::SetBoxImage(INT nImageIndex,INT nType)
{
	POINT xSize,xPos;
	if(m_bSet)
	{
		if((nType&MSG_EDITEXIST)==MSG_EDITEXIST)
		{
			m_pxBtnImage->NewSetIndex(nImageIndex+1);
		}
		else
		{
			m_pxBtnImage->NewSetIndex(nImageIndex);
		}
		m_wpData = (WORD*)m_pxBtnImage->m_pbCurrImage;
		m_shWidth = m_pxBtnImage->m_lpstNewCurrWilImageInfo->shWidth;
		m_shHeight = m_pxBtnImage->m_lpstNewCurrWilImageInfo->shHeight;
		xSize.x = g_xMainWnd.m_rcWindow.right - g_xMainWnd.m_rcWindow.left;
		xSize.y = g_xMainWnd.m_rcWindow.bottom - g_xMainWnd.m_rcWindow.top;
		xPos.x = (xSize.x - m_shWidth)/2;
		xPos.y = (xSize.y - m_shHeight*2)/2;
		m_xBoxPos = xPos;
	}
	return m_bSet;
}

BOOL CMirMsgBox::HideMessageBox(VOID)							// Hide Message box
{
	m_bActive = FALSE;
	return FALSE;
}

BOOL CMirMsgBox::DestoryMessageBox(VOID)						// DestoryMessage box
{
	m_bActive = FALSE;
	m_bInited = FALSE;
	m_bSet = FALSE;
	m_xMsg.ClearAllNodes();
	return FALSE;
}

// Message Process
HRESULT CMirMsgBox::OnButtonDown(POINT tpMouse)		// Button Down
{
	HRESULT hResult=0;
	for(int i = m_nStart; i< m_nEnd; i++)
	{
		m_xMsgBtn[i].OnLButtonDown(tpMouse);
	}
	return hResult;
}

HRESULT	CMirMsgBox::OnButtonUp(POINT tpMouse)			// Button Up
{
	HRESULT hResult=0;
	for(int i = m_nStart; i< m_nEnd; i++)
	{
		if(m_xMsgBtn[i].OnLButtonUp(tpMouse))
		{
			hResult =(HRESULT)pow(2,i);
		}
	}
	if(hResult!=0)
	{
		hResult = hResult | ((m_nType&MSG_EDITEXIST)==MSG_EDITEXIST ? HAVE_EDITOBOX:0);
	}
	return hResult;
}

BOOL CMirMsgBox::StringDivideLen(INT nDivideLen, INT& nDividedLine, CHAR* szSrc,CDLList<CHAR*>* m_pxpStr)
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

			// 读取截至当前部分的字符串。
			memcpy(&szTmpCheck, &szSrc[nStartLen], nEndLen-nStartLen);
			INT nsLen;
			nsLen = strlen(szTmpCheck);

			// 当前已读取的部分超出允许宽度时。
			if (nsLen > nDivideLen )
			{
				// 当前如果是 2 字节字符.
				if ( szSrc[nEndLen-1] < 0 )
				{
					// 如果当前前面的字符不是 2 字节字符
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
				// 1字节字符.
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
			else if(szSrc[nEndLen-1]=='\n')	// 强制换行     如果用 \n 则 Error ????
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

// Render Process
BOOL CMirMsgBox::RenderMessageBox(INT nLoopTIme)							// Render Function
{
	INT	nCount;
//	POINT	xPos;
//	POINT	xSize;

	if(m_bActive&&m_bInited&&m_bSet)
	{	// 只在真正需要绘制时才绘制
		// 确认要绘制的图像区域
		g_xMainWnd.DrawWithImageForCompClipRgn(m_xBoxPos.x,m_xBoxPos.y,m_shWidth,m_shHeight,m_wpData,800,600);

		POINT	xTxtPos;
		nCount = m_xMsg.GetCounter();
		m_xMsg.MoveCurrentToTop();

		xTxtPos.x = m_xBoxPos.x +24;
		switch(nCount)
		{
		case 1:	// 要绘制的线只有 1 条时
			{
				xTxtPos.y = 68;
				INT n = strlen(m_szMsg);
				if(n<WRITESTRLENTH)
				{
					xTxtPos.x= xTxtPos.x + (WRITESTRLENTH-n)*2;
				}
				break;
			}
		case 2:	// 要绘制的行数为 2 个时
			{
				xTxtPos.y = 61;
				break;
			}
		case 3:	// 要绘制的行数为 2 个时
			{
				xTxtPos.y = 54;
				break;
			}
		case 4:	// 要绘制的行数为 2 个时
			{
				xTxtPos.y = 47;
				break;
			}
		case 5:	// 要绘制的行数为 2 个时
			{
				xTxtPos.y = 40;
				break;
			}
		case 6:	// 要绘制的行数为 2 个时
			{
				xTxtPos.y = 33;
				break;
			}
		default: // 当要输出的行数为 3 行以上时
			{
				xTxtPos.y = 26;
				break;
			}
		}

		for(int i = 0 ; i < nCount ; i++)
		{
			g_xMainWnd.PutsHan(NULL,xTxtPos.x,m_xBoxPos.y+xTxtPos.y,RGB(255,255,255),RGB(0,0,0),*m_xMsg.GetCurrentData());
			xTxtPos.y = xTxtPos.y + 16;
			m_xMsg.MoveNextNode();
		}
		// Draw Button

		for(i = m_nStart; i< m_nEnd; i++)
		{
			m_xMsgBtn[i].ShowGameBtn();
		}
		return FALSE;
	}
	return TRUE;
}
/*
*/

#undef WRITESTRLENTH

// MirMsgBox3D
#define WRITESTRLENTH	50

CMirMsgBox3D::CMirMsgBox3D()
{
	m_bCanMove = TRUE;
	m_bFocused = FALSE;
}

CMirMsgBox3D::~CMirMsgBox3D()
{

}

BOOL CMirMsgBox3D::Load(CWHWilImageData* pxImage)
{
	CMirMsgBox::Load(pxImage);
	m_pxWndImage = pxImage;
	D3DWILTextr_Invalidate(pxImage->m_szWilFileName,_WNDIMGINDX_MSGBOX1);
	D3DWILTextr_DestroyTexture(pxImage->m_szWilFileName,_WNDIMGINDX_MSGBOX1);

	pxImage->NewSetIndex(_WNDIMGINDX_MSGBOX1);
	D3DWILTextr_CreateEmptyTexture(pxImage->m_szWilFileName,_WNDIMGINDX_MSGBOX1,pxImage->m_lpstNewCurrWilImageInfo->shWidth,
									pxImage->m_lpstNewCurrWilImageInfo->shHeight,(WORD*)pxImage->m_pbCurrImage,D3DTEXTR_TRANSPARENTBLACK);

	D3DWILTextr_Invalidate(pxImage->m_szWilFileName,_WNDIMGINDX_MSGBOX2);
	D3DWILTextr_DestroyTexture(pxImage->m_szWilFileName,_WNDIMGINDX_MSGBOX2);
	pxImage->NewSetIndex(_WNDIMGINDX_MSGBOX2);
	D3DWILTextr_CreateEmptyTexture(pxImage->m_szWilFileName,_WNDIMGINDX_MSGBOX2,pxImage->m_lpstNewCurrWilImageInfo->shWidth,
									pxImage->m_lpstNewCurrWilImageInfo->shHeight,(WORD*)pxImage->m_pbCurrImage,D3DTEXTR_TRANSPARENTBLACK);
	return TRUE;
}

BOOL CMirMsgBox3D::CreateWnd(CWHWilImageData* pxWndImage, BOOL bCanMove, INT nStartX, INT nStartY, INT nWidth, INT nHeight)
{
	m_nID			= _WND_ID_MESSAGEBOX;
	m_pxWndImage	= pxWndImage;
	m_bCanMove		= bCanMove;

	Load(pxWndImage);

	m_pxWndImage->NewSetIndex(_WNDIMGINDX_MSGBOX1);
	SetRect(&m_rcImgWnd, nStartX, nStartY,
			nStartX+m_pxWndImage->m_lpstNewCurrWilImageInfo->shWidth,
			nStartY+m_pxWndImage->m_lpstNewCurrWilImageInfo->shHeight);

	( ! nWidth )  ?	m_nWidth = m_rcImgWnd.right -m_rcImgWnd.left: m_nWidth  = nWidth;
	( ! nHeight ) ?	m_nHeight =m_rcImgWnd.bottom-m_rcImgWnd.top : m_nHeight = nHeight;

	SetRect(&m_rcWnd, nStartX, nStartY, nStartX+m_nWidth, nStartY+m_nHeight);

	D3DVECTOR vNorm(0, 0, -1);
    m_avMagic[0]  = D3DVERTEX(D3DVECTOR(-0.5f, 0.5f, 0), vNorm, 0, 0);
    m_avMagic[1]  = D3DVERTEX(D3DVECTOR(-0.5f,-0.5f, 0), vNorm, 0, 1);
    m_avMagic[2]  = D3DVERTEX(D3DVECTOR( 0.5f, 0.5f, 0), vNorm, 1, 0);
    m_avMagic[3]  = D3DVERTEX(D3DVECTOR( 0.5f,-0.5f, 0), vNorm, 1, 1);

	return FALSE;
}

BOOL CMirMsgBox3D::ShowMessageBox(CHAR* szMgs,INT nType,INT nImgIndex)	// Show	Message box with Initialization
{
	m_nFrameImgIdx = (nType&128) ? _WNDIMGINDX_MSGBOX2:_WNDIMGINDX_MSGBOX1;

	D3DWILTextr_Invalidate(m_pxWndImage->m_szWilFileName, m_nFrameImgIdx);
	D3DWILTextr_Restore(m_pxWndImage->m_szWilFileName, m_nFrameImgIdx, g_xMainWnd.Get3DDevice());

	CMirMsgBox::ShowMessageBox(szMgs,nType,nImgIndex);
	m_bActive = TRUE;
	return TRUE;
}

BOOL CMirMsgBox3D::HideMessageBox(VOID)									// Hide Message box
{
	m_bActive = FALSE;
	return FALSE;
}

BOOL CMirMsgBox3D::DestoryMessageBox(VOID)								// DestoryMessage box
{
	m_bActive = FALSE;
	m_bInited = FALSE;
	m_bSet = FALSE;
	m_xMsg.ClearAllNodes();
	return TRUE;
}

BOOL CMirMsgBox3D::OnKeyDown(WPARAM wParam,LPARAM lParam)
{
	return TRUE;
}

BOOL CMirMsgBox3D::RenderMessageBox(INT nLoopTIme)
{
	INT	nCount;

	RenderBox();

	if(m_bActive&&m_bInited&&m_bSet)
	{
		// 只在真正需要绘制时才绘制
		// 确认要绘制的图像区域
		POINT	xTxtPos;

		m_xBoxPos.x = m_rcWnd.left;
		m_xBoxPos.y = m_rcWnd.top;

		nCount = m_xMsg.GetCounter();
		m_xMsg.MoveCurrentToTop();

		xTxtPos.x = m_xBoxPos.x +24;
		switch(nCount)
		{
		case 1:	// 要绘制的线只有 1 条时
			{
				xTxtPos.y = 68;
				INT n = strlen(m_szMsg);
				if(n<WRITESTRLENTH)
				{
					xTxtPos.x= xTxtPos.x + (WRITESTRLENTH-n)*2;
				}
				break;
			}
		case 2:	// 要绘制的行数为 2 个时
			{
				xTxtPos.y = 61;
				break;
			}
		case 3:	// 打印的行数为 3 个时
			{
				xTxtPos.y = 54;
				break;
			}
		case 4:	// 要绘制的行数为 4 个时
			{
				xTxtPos.y = 47;
				break;
			}
		case 5:	// 要绘制的行数为 5 个时
			{
				xTxtPos.y = 40;
				break;
			}
		case 6:	// 要绘制的行数为 6 个时
			{
				xTxtPos.y = 33;
				break;
			}
		default: // 要绘制的行在 7 个以上时
			{
				xTxtPos.y = 26;
				break;
			}
		}

		for(int i = 0 ; i < nCount ; i++)
		{
			g_xMainWnd.PutsHan(NULL,xTxtPos.x,m_xBoxPos.y+xTxtPos.y,RGB(255,255,255),RGB(0,0,0),*m_xMsg.GetCurrentData());
			xTxtPos.y = xTxtPos.y + 16;
			m_xMsg.MoveNextNode();
		}

		// Draw Button
		switch(m_nType)
		{
		case MSG_BTN_YESNO:
		case MSG_BTN_YESNO|MSG_EDITEXIST:
			{
				m_nStart=1;
				m_nEnd = 3;

				m_xMsgBtn[1].ChangeRect(m_xBoxPos.x+78,m_xBoxPos.y+134);
				m_xMsgBtn[2].ChangeRect(m_xBoxPos.x+210,m_xBoxPos.y+134);

				break;
			}
		case MSG_BTN_YESNOCANCLE:
		case MSG_BTN_YESNOCANCLE|MSG_EDITEXIST:
			{
				m_nStart=1;
				m_nEnd = 4;

				m_xMsgBtn[1].ChangeRect(m_xBoxPos.x+54,m_xBoxPos.y+134);
				m_xMsgBtn[2].ChangeRect(m_xBoxPos.x+144,m_xBoxPos.y+134);
				m_xMsgBtn[3].ChangeRect(m_xBoxPos.x+234,m_xBoxPos.y+134);
				break;
			}
		case MSG_BTN_OK:
			{
				m_nStart=0;
				m_nEnd = 1;

				m_xMsgBtn[0].ChangeRect(m_xBoxPos.x+144,m_xBoxPos.y+134);
				break;
			}
		case MSG_BTN_OK|MSG_EDITEXIST:
		default:
			{
				m_nStart=0;
				m_nEnd = 1;

				m_xMsgBtn[0].ChangeRect(m_xBoxPos.x+260,m_xBoxPos.y+134);
				break;
			}
		}
		for(i = m_nStart; i< m_nEnd; i++)
		{
			m_xMsgBtn[i].ShowGameBtn();
		}
		return FALSE;
	}
	return TRUE;
}

BOOL CMirMsgBox3D::RenderBox(VOID)
{
	if ( m_bActive )
	{
		if ( g_xMainWnd.Get3DDevice() )
		{
			D3DVECTOR vTrans;
			D3DMATRIX matTrans;
			D3DMATRIX matScale;
			D3DMATRIX matWorld;
			D3DMATRIX matWorldOriginal;
			D3DMATERIAL7 mtrl;
			if( SUCCEEDED(g_xMainWnd.Get3DDevice()->BeginScene()))
			{
				g_xMainWnd.Get3DDevice()->GetTransform(D3DTRANSFORMSTATE_WORLD, &matWorldOriginal);

				m_pxWndImage->NewSetIndex(m_nFrameImgIdx);

				vTrans.x = (FLOAT)m_rcWnd.left+(FLOAT)(m_rcImgWnd.right-m_rcImgWnd.left)/2-400;
				vTrans.y = (FLOAT)-m_rcWnd.top-(FLOAT)(m_rcImgWnd.bottom-m_rcImgWnd.top)/2+300;
				vTrans.z = 0;
				D3DUtil_SetTranslateMatrix(matTrans, vTrans);
				D3DUtil_SetScaleMatrix(matScale, (FLOAT)m_rcImgWnd.right-m_rcImgWnd.left, (FLOAT)m_rcImgWnd.bottom-m_rcImgWnd.top, 0.0f);
				D3DMath_MatrixMultiply(matWorld, matScale, matTrans);
				g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld);

				if ( m_bFocused )
				{
					g_xMainWnd.Get3DDevice()->SetTexture(0, D3DWILTextr_GetSurface(m_pxWndImage->m_szWilFileName, m_nFrameImgIdx));
					D3DUtil_InitMaterial(mtrl,
										 (FLOAT)255/255.0f,
										 (FLOAT)255/255.0f,
										 (FLOAT)255/255.0f);
					mtrl.diffuse.a = 200/255.0f;
					g_xMainWnd.Get3DDevice()->SetMaterial(&mtrl);
				}
				else
				{
					g_xMainWnd.Get3DDevice()->SetTexture(0, D3DWILTextr_GetSurface(m_pxWndImage->m_szWilFileName, m_nFrameImgIdx));
					D3DUtil_InitMaterial(mtrl,
										 (FLOAT)255/255.0f,
										 (FLOAT)255/255.0f,
										 (FLOAT)255/255.0f);
					mtrl.diffuse.a = 170/255.0f;
					g_xMainWnd.Get3DDevice()->SetMaterial(&mtrl);
				}

				g_xMainWnd.Get3DDevice()->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
				g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE);
				g_xMainWnd.Get3DDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				g_xMainWnd.Get3DDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
				g_xMainWnd.Get3DDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

				g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
				g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);

				g_xMainWnd.Get3DDevice()->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX, m_avMagic, 4, NULL);

				g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
				g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
				g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_SRCBLEND , D3DBLEND_ONE);
				g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
				g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorldOriginal);

				g_xMainWnd.Get3DDevice()->EndScene();
			}
		}
	}
	return TRUE;
}

VOID CMirMsgBox3D::MoveWnd(POINT ptMouse)		// Move Window
{
	if ( m_bActive && m_bFocused && m_bCanMove )
	{
		SetRect(&m_rcWnd, ptMouse.x-m_bGrabbedMousePos.x, ptMouse.y-m_bGrabbedMousePos.y,
				ptMouse.x-m_bGrabbedMousePos.x+m_nWidth, ptMouse.y-m_bGrabbedMousePos.y+m_nHeight);
	}
}

HRESULT	CMirMsgBox3D::OnButtonDown(POINT tpMouse)		// Button Down
{
	if(CMirMsgBox::OnButtonDown(tpMouse)==1) return 0;

	if(IsInRect(tpMouse.x,tpMouse.y,m_rcWnd))
	{
		m_bFocused = TRUE;

		if ( m_bActive && m_bFocused && m_bCanMove )		// Get the gaps between Mouse and Window
		{
			m_bGrabbedMousePos.x = tpMouse.x - m_rcWnd.left;
			m_bGrabbedMousePos.y = tpMouse.y - m_rcWnd.top;
		}
	}
	return 0;
}

HRESULT	CMirMsgBox3D::OnButtonUp(POINT tpMouse)		// Button Up
{
	m_bFocused = FALSE;
	return CMirMsgBox::OnButtonUp(tpMouse);
}

#undef WRITESTRLENTH

// NoticeBox
#define WRITESTRLENTH 50

#define BTN_POS_X	165
#define BTN_POS_Y	483

#define _IDX_UP_BTN_IMG			1
#define _IDX_ON_BTN_IMG			2
#define _IDX_DOWN_BTN_IMG		3


CNoticeBox::CNoticeBox()
{
	m_bActive = FALSE;
	m_bInited = FALSE;
	m_bIsSelected = FALSE;
	m_bSet = FALSE;
	ZeroMemory(&m_rcWnd,sizeof(RECT));
}

BOOL CNoticeBox::Load(CWHWilImageData* pxBtnImage=NULL)
{
	if(pxBtnImage!=NULL)
	{
		// Init Button;
		m_bSet			=	TRUE;
		m_pxBtnImage	=	pxBtnImage;
		m_xMsgBtn.CreateGameBtn(pxBtnImage, _IDX_ON_BTN_IMG, _IDX_DOWN_BTN_IMG, BTN_POS_X,BTN_POS_Y);
	}
	else
	{
		m_bSet = FALSE;
	}
	return m_bSet;
}

BOOL CNoticeBox::DestoryMessageBox(VOID)						// DestoryMessage box
{
	m_bActive = FALSE;
	m_bInited = FALSE;
	m_bSet = FALSE;
	m_xMsg.ClearAllNodes();
	return FALSE;
}

BOOL CNoticeBox::HideMessageBox(VOID)							// Hide Message box
{
	m_bActive = FALSE;
	return FALSE;
}

CNoticeBox::~CNoticeBox()										// Destructor
{
}

BOOL CNoticeBox::SetBoxImage(INT nImageIndex)
{
	POINT xSize,xPos;
	if(m_bSet)
	{
		m_pxBtnImage->NewSetIndex(nImageIndex);
		m_wpData = (WORD*)m_pxBtnImage->m_pbCurrImage;
		m_shWidth = m_pxBtnImage->m_lpstNewCurrWilImageInfo->shWidth;
		m_shHeight = m_pxBtnImage->m_lpstNewCurrWilImageInfo->shHeight;
		xSize.x = g_xMainWnd.m_rcWindow.right - g_xMainWnd.m_rcWindow.left;
		xSize.y = g_xMainWnd.m_rcWindow.bottom - g_xMainWnd.m_rcWindow.top;
		xPos.x = (xSize.x - m_shWidth)/2;
		xPos.y = (xSize.y - m_shHeight)/2;

		SetRect(&m_rcWnd, xPos.x, xPos.y, xPos.x + m_shWidth, xPos.y + m_shHeight);
	}
	return m_bSet;
}

BOOL CNoticeBox::ShowMessageBox(CHAR* szMsg,INT nImgIndex) // Show	Message box with Initialization
{
	INT Lenth;
	if(!m_bActive)
	{
		if(SetBoxImage(nImgIndex))
		{
			if(ShowMessage(szMsg))
			{
				m_xMsg.ClearAllNodes();
				StringDivideLen(WRITESTRLENTH,Lenth, m_szMsg,&m_xMsg);

				m_xMsgBtn.ChangeRect(m_rcWnd.left + BTN_POS_X, m_rcWnd.top + BTN_POS_Y);
			}
			else
			{
				m_bActive = FALSE;
			}
		}
		else
		{
			m_bActive = FALSE;
			m_bInited = FALSE;
		}
	}
	return m_bActive;
}

// Message Process
HRESULT CNoticeBox::OnButtonDown(POINT tpMouse)		// Button Down
{
	HRESULT hResult=0;

	if(m_xMsgBtn.OnLButtonDown(tpMouse))
	{
		g_xClientSocket.SendNoticeOK();
		hResult = 1;
	}

	if(!hResult)
	{
		if(IsInRect(tpMouse.x,tpMouse.y,m_rcWnd))
		{
			m_bGrabbedMousePos.x  = tpMouse.x - m_rcWnd.left;
			m_bGrabbedMousePos.y  = tpMouse.y - m_rcWnd.top;
			m_bIsSelected = TRUE;
			hResult = 0;
		}
	}

	return hResult;
}

HRESULT	CNoticeBox::OnButtonUp(POINT tpMouse)			// Button Up
{
	m_bIsSelected = FALSE;
	HRESULT hResult=0;
	if(m_xMsgBtn.OnLButtonUp(tpMouse))
		hResult = 1; // Here
	return hResult;
}

// Render Process
BOOL CNoticeBox::RenderMessageBox(INT nLoopTIme)							// Render Function
{
	INT	nCount;

	if(m_bActive&&m_bInited&&m_bSet)
	{
		g_xMainWnd.DrawWithImageForCompClipRgn( m_rcWnd.left, m_rcWnd.top, m_shWidth, m_shHeight, m_wpData, 800, 600 );

		POINT	xTxtPos;
		nCount = m_xMsg.GetCounter();
		m_xMsg.MoveCurrentToTop();

		xTxtPos.x = m_rcWnd.left + 40;
		xTxtPos.y = m_rcWnd.top	 + 20;

		for(int i = 0 ; i < nCount ; i++)
		{
			g_xMainWnd.PutsHan(NULL,xTxtPos.x,xTxtPos.y,RGB(255,255,255),RGB(0,0,0),*m_xMsg.GetCurrentData());
			xTxtPos.y = xTxtPos.y + 16;	// Line Skip

			m_xMsg.MoveNextNode();
		}
		// Draw Button
		m_xMsgBtn.ShowGameBtn();

		return FALSE;
	}
	return TRUE;
}

VOID CNoticeBox::MoveWnd(POINT ptMouse)					// Move Window
{
	if ( m_bActive && m_bIsSelected)
	{
		SetRect(&m_rcWnd, ptMouse.x-m_bGrabbedMousePos.x, ptMouse.y-m_bGrabbedMousePos.y,
				ptMouse.x-m_bGrabbedMousePos.x+m_shWidth, ptMouse.y-m_bGrabbedMousePos.y+m_shHeight);
		m_xMsgBtn.ChangeRect(m_rcWnd.left + BTN_POS_X, m_rcWnd.top + BTN_POS_Y);
	}
	else
	{
		m_xMsgBtn.OnMouseMove(ptMouse);
	}
}


BOOL CNoticeBox::ShowMessage(CHAR* szMsg)		// Show	Message box with Initialization
{
	if(SetMsg(szMsg))
		m_bActive = TRUE;
	else
		m_bActive = FALSE;

	return m_bActive;
}

BOOL CNoticeBox::SetMsg(CHAR* szMsg)				// Init Messages
{
	INT nLength;
	nLength = strlen(szMsg);
	if(nLength!=0)
	{
		ZeroMemory(m_szMsg,MAX_PATH);
		strcpy(m_szMsg,szMsg);
		m_bInited = TRUE;
	}
	else
		m_bInited = FALSE;

	return m_bInited;
}

BOOL CNoticeBox::StringDivideLen(INT nDivideLen, INT& nDividedLine, CHAR* szSrc,CDLList<CHAR*>* m_pxpStr)
{
	CHAR* szTmpCheck;
	CHAR szResult[MAX_PATH];

	INT nStartLen	= 0;
	INT nEndLen		= 0;
	INT nResultLen	= 0;
	INT nWordCheck	= 0;

	nDividedLine = 1;

	if ( szSrc[0] != NULL )
	{
		ZeroMemory(szResult,MAX_PATH);

		for ( INT nCnt = 0; nCnt < (INT)strlen(szSrc); nCnt++)
		{
			nEndLen = nCnt+1;

			// 读取截至当前部分的字符串。
			szTmpCheck = szSrc+nStartLen;
			INT nsLen;
			nsLen = strlen(szTmpCheck);

			// 当前已读取的部分超出允许宽度时。
			if (nsLen > nDivideLen )
			{
				// 当前如果是 2 字节字符.
				if ( szSrc[nEndLen-1] < 0 )
				{
					// 如果当前前面的字符不是 2 字节字符
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
				// 1字节字符.
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
			else if(szSrc[nEndLen-1] == '\n' || szSrc[nEndLen-1] == 0x1B)	// 强制换行     如果用 \n 则 Error ????

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

#undef WRITESTRLENTH
#undef BTN_POS_X
#undef BTN_POS_Y
#undef _IDX_UP_BTN_IMG
#undef _IDX_ON_BTN_IMG
#undef _IDX_DOWN_BTN_IMG

// GameWnd
/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/





VOID CGameWnd::CreateGameWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, BOOL bCanMove, INT nStartX, INT nStartY, INT nWidth, INT nHeight)
{
	m_nID			= nID;
	m_nFrameImgIdx	= nFrameImgIdx;
	m_pxWndImage	= pxWndImage;
	m_bCanMove		= bCanMove;

	if ( m_pxWndImage->NewSetIndex(m_nFrameImgIdx) )
	{
		SetRect(&m_rcImgWnd, nStartX, nStartY,
				nStartX+m_pxWndImage->m_lpstNewCurrWilImageInfo->shWidth,
				nStartY+m_pxWndImage->m_lpstNewCurrWilImageInfo->shHeight);
	}

	( ! nWidth )  ?	m_nWidth = m_rcImgWnd.right -m_rcImgWnd.left: m_nWidth  = nWidth;
	( ! nHeight ) ?	m_nHeight =m_rcImgWnd.bottom-m_rcImgWnd.top : m_nHeight = nHeight;

	SetRect(&m_rcWnd, nStartX, nStartY, nStartX+m_nWidth, nStartY+m_nHeight);

	// 3D测试
	D3DVECTOR vNorm(0, 0, -1);
    m_avBoard[0]  = D3DVERTEX(D3DVECTOR(-0.5f, 0.5f, 0), vNorm, 0, 0);
    m_avBoard[1]  = D3DVERTEX(D3DVECTOR(-0.5f,-0.5f, 0), vNorm, 0, 1);
    m_avBoard[2]  = D3DVERTEX(D3DVECTOR( 0.5f, 0.5f, 0), vNorm, 1, 0);
    m_avBoard[3]  = D3DVERTEX(D3DVECTOR( 0.5f,-0.5f, 0), vNorm, 1, 1);
}


VOID CGameWnd::Init()
{
	m_nID			= 0;
	m_nFrameImgIdx	= 0;
	m_nWidth		= 0;
	m_nHeight		= 0;
	m_pxWndImage	= NULL;
	m_bActive		= FALSE;
	m_bFocused		= FALSE;
	m_bMoving		= FALSE;
	m_bCanMove		= FALSE;
	SetRect(&m_rcWnd, 0, 0, 0, 0);
	SetRect(&m_rcImgWnd, 0, 0, 0, 0);
	ZeroMemory(&m_bGrabbedMousePos, sizeof(POINT));
	D3DVECTOR		vNorm(0, 0, -1);
	m_avBoard[0] = D3DVERTEX(D3DVECTOR(-0.5f, 0.5f, 0), vNorm, 0, 0);
	m_avBoard[1] = D3DVERTEX(D3DVECTOR(-0.5f,-0.5f, 0), vNorm, 0, 1);
	m_avBoard[2] = D3DVERTEX(D3DVECTOR( 0.5f, 0.5f, 0), vNorm, 1, 0);
	m_avBoard[3] = D3DVERTEX(D3DVECTOR( 0.5f,-0.5f, 0), vNorm, 1, 1);
}


VOID CGameWnd::Destroy()
{
	Init();
}


VOID CGameWnd::ShowGameWnd()
{
	if ( m_bActive )
	{
		if ( m_pxWndImage->NewSetIndex(m_nFrameImgIdx) )
		{
			g_xMainWnd.DrawWithImageForCompClipRgn(m_rcWnd.left, m_rcWnd.top, m_pxWndImage->m_lpstNewCurrWilImageInfo->shWidth, m_pxWndImage->m_lpstNewCurrWilImageInfo->shHeight,
												   (WORD*)m_pxWndImage->m_pbCurrImage, _CLIP_WIDTH, 600);
		}

/*		if ( g_xMainWnd.Get3DDevice() )
		{
			D3DVECTOR vTrans;
			D3DMATRIX matTrans;
			D3DMATRIX matScale;
			D3DMATRIX matWorld;
			D3DMATRIX matWorldOriginal;
			D3DMATERIAL7 mtrl;
			if( SUCCEEDED(g_xMainWnd.Get3DDevice()->BeginScene()) )
			{
				g_xMainWnd.Get3DDevice()->GetTransform(D3DTRANSFORMSTATE_WORLD, &matWorldOriginal);

				m_pxWndImage->NewSetIndex(m_nFrameImgIdx);

				vTrans.x = (FLOAT)m_rcWnd.left+(FLOAT)(m_rcImgWnd.right-m_rcImgWnd.left)/2-400;
				vTrans.y = (FLOAT)-m_rcWnd.top-(FLOAT)(m_rcImgWnd.bottom-m_rcImgWnd.top)/2+300;
				vTrans.z = 0;
				D3DUtil_SetTranslateMatrix(matTrans, vTrans);
				D3DUtil_SetScaleMatrix(matScale, (FLOAT)m_rcImgWnd.right-m_rcImgWnd.left, (FLOAT)m_rcImgWnd.bottom-m_rcImgWnd.top, 0.0f);
				D3DMath_MatrixMultiply(matWorld, matScale, matTrans);
				g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld);

				LPDIRECTDRAWSURFACE7 lpddsTextr = g_xGameProc.m_xImage.GetTextrImg(_TEXTR_FILE_INTER, _IMAGE_INTEREX, m_nFrameImgIdx);
				g_xMainWnd.Get3DDevice()->SetTexture(0, lpddsTextr);

//				g_xMainWnd.Get3DDevice()->SetTexture(0, D3DWILTextr_GetSurface(m_pxWndImage->m_szWilFileName, m_nFrameImgIdx));
				D3DUtil_InitMaterial(mtrl,
									 (FLOAT)255/255.0f,
									 (FLOAT)255/255.0f,
									 (FLOAT)255/255.0f);
				if ( m_bFocused )
				{
					mtrl.diffuse.a = 200/255.0f;
				}
				else
				{
					mtrl.diffuse.a = 170/255.0f;
				}
				g_xMainWnd.Get3DDevice()->SetMaterial(&mtrl);


				g_xMainWnd.Get3DDevice()->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
				g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE);
				g_xMainWnd.Get3DDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				g_xMainWnd.Get3DDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
				g_xMainWnd.Get3DDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

				g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
				g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);

				g_xMainWnd.Get3DDevice()->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX, m_avBoard, 4, NULL);

				g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
				g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
				g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_SRCBLEND , D3DBLEND_ONE);
				g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
				g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorldOriginal);

				g_xMainWnd.Get3DDevice()->EndScene();
			}
		}
*/	}
}


VOID CGameWnd::ResetWndImg(INT nIdx)
{
//	D3DWILTextr_Invalidate(m_pxWndImage->m_szWilFileName, m_nFrameImgIdx);
	if ( m_pxWndImage->NewSetIndex(nIdx) )
	{
		m_nFrameImgIdx = nIdx;
		SetRect(&m_rcImgWnd, m_rcWnd.left, m_rcWnd.top,
				m_rcWnd.left+m_pxWndImage->m_lpstNewCurrWilImageInfo->shWidth,
				m_rcWnd.top+m_pxWndImage->m_lpstNewCurrWilImageInfo->shHeight);

		m_nWidth = m_rcImgWnd.right-m_rcWnd.left;
		m_nHeight =m_rcImgWnd.bottom-m_rcWnd.top;

		m_rcWnd = m_rcImgWnd;
	}

//	g_xGameProc.m_xImage.AddTextr(_TEXTR_FILE_INTER, _IMAGE_INTEREX, m_nFrameImgIdx);
//	D3DWILTextr_Restore(m_pxWndImage->m_szWilFileName, m_nFrameImgIdx, g_xMainWnd.Get3DDevice());
}

// 激活窗口。
VOID CGameWnd::SetGameWndActive(BOOL bActive)
{
	m_bActive = bActive;

	g_xGameProc.m_xImage.AddTextr(_TEXTR_FILE_INTER, _IMAGE_INTEREX, m_nFrameImgIdx);

/*	if ( m_bActive )
	{
		D3DWILTextr_Restore(m_pxWndImage->m_szWilFileName, m_nFrameImgIdx, g_xMainWnd.Get3DDevice());
	}
	else
	{
		D3DWILTextr_Invalidate(m_pxWndImage->m_szWilFileName, m_nFrameImgIdx);
	}
*/
}

// 使窗口获得焦点。
VOID CGameWnd::SetGameWndFocused(BOOL bFocused)
{
	m_bFocused = bFocused;
}

VOID CGameWnd::MoveGameWnd(POINT ptMouse)
{
	if ( m_bActive && m_bFocused && m_bCanMove )
	{
		SetRect(&m_rcWnd, ptMouse.x-m_bGrabbedMousePos.x, ptMouse.y-m_bGrabbedMousePos.y,
				ptMouse.x-m_bGrabbedMousePos.x+m_nWidth, ptMouse.y-m_bGrabbedMousePos.y+m_nHeight);
	}
}

// 窗口开始移动时取得与鼠标的 Gap.
VOID CGameWnd::SetReadyMove(POINT ptMouse)
{
	if ( m_bActive && m_bFocused && m_bCanMove )
	{
		m_bGrabbedMousePos.x = ptMouse.x - m_rcWnd.left;
		m_bGrabbedMousePos.y = ptMouse.y - m_rcWnd.top;
	}
}

// InventoryWnd
/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/






/******************************************************************************************************************

	CInventoryWnd Class

*******************************************************************************************************************/

CInventoryWnd::CInventoryWnd()
{
	Init();
}


CInventoryWnd::~CInventoryWnd()
{
	Destroy();
}


VOID CInventoryWnd::Init()
{
	INT nCnt;
	CGameWnd::Init();
	m_nStartLineNum	= 0;
	ZeroMemory(m_rcInvenCell, sizeof(RECT) *_INVEN_MAX_CELL);
	ZeroMemory(m_rcBeltCell,  sizeof(RECT) *_BELT_MAX_CELL );
	ZeroMemory(m_stInventoryItem, sizeof(ITEMSET)*_MAX_INVEN_ITEM);
	ZeroMemory(m_stBeltItem,	  sizeof(ITEMSET)*_MAX_BELT_ITEM );

	m_xInvenScrlBar.Init();

	for ( nCnt = 0; nCnt < _MAX_INVEN_BTN; nCnt++ )
	{
		m_xInventoryBtn[nCnt].Init();
	}
}


VOID CInventoryWnd::Destroy()
{
	INT nCnt;
	CGameWnd::Destroy();
	for ( nCnt = 0; nCnt < _MAX_INVEN_BTN; nCnt++ )
	{
		m_xInventoryBtn[nCnt].Destroy();
	}
	Init();
}


VOID CInventoryWnd::CreateInventoryWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove)
{
	// 窗口创建.
	CreateGameWnd(nID, pxWndImage, nFrameImgIdx, bCanMove, nStartX, nStartY, nWidth, nHeight);

	// 按钮创建.
	m_xInventoryBtn[_BTN_ID_INVENCLOSE	 ].CreateGameBtn(pxWndImage, 280, 281, nStartX+255, nStartY+291);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLUP	 ].CreateGameBtn(pxWndImage, 74, 75, nStartX+247, nStartY+ 20);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLDOWN].CreateGameBtn(pxWndImage, 74, 75, nStartX+247, nStartY+167);

	m_xInvenScrlBar.CreateScrlBar(pxWndImage, 270, _INVEN_MAX_CELL_YCNT, 10, 218, 10);

	// 背包格子区域的 Set.
	for ( INT nYCnt = 0; nYCnt < _INVEN_CELL_YCNT; nYCnt++ )
	{
		for ( INT nXCnt = 0; nXCnt < _INVEN_CELL_XCNT; nXCnt++ )
		{
			SetRect(&m_rcInvenCell[nXCnt+nYCnt*_INVEN_CELL_XCNT],
					_INVEN_CELL_XSTART+(nXCnt*_INVENTORY_CELL_WIDTH),
					_INVEN_CELL_YSTART+(nYCnt*_INVENTORY_CELL_HEIGHT),
					_INVEN_CELL_XSTART+(nXCnt*_INVENTORY_CELL_WIDTH) +_INVENTORY_CELL_WIDTH,
					_INVEN_CELL_YSTART+(nYCnt*_INVENTORY_CELL_HEIGHT)+_INVENTORY_CELL_HEIGHT);
		}
	}

	// 腰带单元格区域的 Set。
	for ( INT nCnt = 0; nCnt < _MAX_BELT_ITEM; nCnt++ )
	{
		SetRect(&m_rcBeltCell[nCnt],
				_BELT_CELL_XSTART+(nCnt*_BELT_CELL_XGAP),
				_BELT_CELL_YSTART,
				_BELT_CELL_XSTART+(nCnt*_BELT_CELL_XGAP)+_INVENTORY_CELL_WIDTH,
				_BELT_CELL_YSTART+_INVENTORY_CELL_HEIGHT);
	}
}



VOID CInventoryWnd::SetInvenBtnInit()
{
	for( INT nCnt = 0; nCnt < _MAX_INVEN_BTN; nCnt++ )
	{
		m_xInventoryBtn[nCnt].SetBtnState(_BTN_STATE_NORMAL);
	}
}


VOID CInventoryWnd::ShowPanel()
{
	INT nCnt;

	// 窗口背景绘制.
	ShowGameWnd();


	RECT rcTitle = {m_rcWnd.left+31, m_rcWnd.top+27, m_rcWnd.left+133, m_rcWnd.top+43};

	g_xMainWnd.PutsHan(NULL, rcTitle, RGB(200, 200, 255), RGB(0, 0, 0), "[Inventory]", g_xMainWnd.CreateGameFont("Arial", 10, 0, FW_BOLD));

	// 滚动条.
	m_xInvenScrlBar.ShowScrlBar(m_rcWnd.left+254, m_rcWnd.top-165, m_nStartLineNum, 12);

	// 绘制物品栏物品.
	for ( nCnt = 0; nCnt < _MAX_INVEN_ITEM; nCnt++ )
	{
		if ( m_stInventoryItem[nCnt].bSetted )
		{
			INT nStartX, nStartY;

			if ( nCnt >= m_nStartLineNum*_INVEN_CELL_XCNT && nCnt < (m_nStartLineNum+_INVEN_CELL_YCNT)*_INVEN_CELL_XCNT )
			{
				nStartX = m_rcWnd.left+m_rcInvenCell[nCnt-(m_nStartLineNum*_INVEN_CELL_XCNT)].left;
				nStartY = m_rcWnd.top +m_rcInvenCell[nCnt-(m_nStartLineNum*_INVEN_CELL_XCNT)].top;

//				m_stInventoryItem[nCnt].xItem.m_stItemInfo.stStdItem.wLooks = 4;
				m_stInventoryItem[nCnt].xItem.DrawItem(nStartX, nStartY);

//				RECT rc = {nStartX, nStartY, nStartX+_INVENTORY_CELL_WIDTH, nStartY+_INVENTORY_CELL_HEIGHT};
//				g_xMainWnd.DrawWithGDI(rc, NULL, RGB(255, 0, 255), 1);
			}
		}
	}

	// 腰带物品绘制。
	for ( nCnt = 0; nCnt < _MAX_BELT_ITEM; nCnt++ )
	{
		if ( m_stBeltItem[nCnt].bSetted )
		{
			INT nStartX, nStartY;

			nStartX = m_rcWnd.left+m_rcBeltCell[nCnt].left;
			nStartY = m_rcWnd.top+m_rcBeltCell[nCnt].top;

			m_stBeltItem[nCnt].xItem.DrawItem(nStartX, nStartY);
		}
	}

	// 绘制背包窗口按钮.
	for ( nCnt = 0; nCnt < _MAX_INVEN_BTN; nCnt++ )
	{
		m_xInventoryBtn[nCnt].ShowGameBtn();
	}

	// 显示物品的状态值。
	ShowInvenItemState();
}




VOID CInventoryWnd::ShowInvenItemState()
{
/*	INT nInventoryNum = GetInvenNum(g_xGameProc.m_ptMousePos);
	if ( nInventoryNum != -1 )
	{
		if ( m_stInventoryItem[nInventoryNum].bSetted )
		{
			m_stInventoryItem[nInventoryNum].xItem.ShowItemStatus(m_rcWnd.left+95, m_rcWnd.top+250);
		}
	}
*/
	INT nInventoryNum = GetInvenNum(g_xGameProc.m_ptMousePos);
	if ( nInventoryNum != -1 )
	{
		if ( m_stInventoryItem[nInventoryNum].bSetted )
		{
			INT nStartX, nStartY;

			if ( nInventoryNum >= m_nStartLineNum*_INVEN_CELL_XCNT && nInventoryNum < (m_nStartLineNum+_INVEN_CELL_YCNT)*_INVEN_CELL_XCNT )
			{
				nStartX = m_rcWnd.left+m_rcInvenCell[nInventoryNum-(m_nStartLineNum*_INVEN_CELL_XCNT)].left;
				nStartY = m_rcWnd.top +m_rcInvenCell[nInventoryNum-(m_nStartLineNum*_INVEN_CELL_XCNT)].top;

				m_stInventoryItem[nInventoryNum].xItem.ShowItemStatus(nStartX+15, nStartY+15);
			}
		}
	}

	INT nBeltInventoryNum = GetBeltNum(g_xGameProc.m_ptMousePos);
	if ( nBeltInventoryNum != -1 )
	{
		if ( m_stBeltItem[nBeltInventoryNum].bSetted )
		{
			m_stBeltItem[nBeltInventoryNum].xItem.ShowItemStatus(m_rcWnd.left+95, m_rcWnd.top+250);
		}
	}
}














/******************************************************************************************************************

	物品放入与取出相关, 背包位置的保存文件存盘及加载.(背包及腰带)

*******************************************************************************************************************/

BOOL CInventoryWnd::CheckItemPos(CHAR* szName)
{
	HANDLE	hFile;
	CHAR	szItemFile[MAX_PATH];
	INT		nCnt1, nCnt2;
	ITEMSET	stInvenItem[_MAX_INVEN_ITEM];						// 临时数据加载用变量.
	ITEMSET	stBeltItem[_MAX_BELT_ITEM];
	ITEMSET	stCheckedInvenItem[_MAX_INVEN_ITEM];				// 保存已检查数据的变量.
	ITEMSET	stCehckedBeltItem[_MAX_BELT_ITEM];

	ZeroMemory(szItemFile, MAX_PATH);
	g_xMainWnd.StringPlus(szItemFile, ".\\Data\\", szName, ".itm", "");

	hFile = CreateFile(szItemFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	ZeroMemory(stInvenItem, sizeof(ITEMSET)*_MAX_INVEN_ITEM);
	ZeroMemory(stCheckedInvenItem, sizeof(ITEMSET)*_MAX_INVEN_ITEM);
	ZeroMemory(stBeltItem , sizeof(ITEMSET)*_MAX_BELT_ITEM );
	ZeroMemory(stCehckedBeltItem, sizeof(ITEMSET)*_MAX_BELT_ITEM);

	if ( hFile != INVALID_HANDLE_VALUE )
	{
		DWORD		dwReadLen;

		ReadFile(hFile, &stInvenItem, sizeof(ITEMSET)*_MAX_INVEN_ITEM, &dwReadLen, NULL);
		ReadFile(hFile, &stBeltItem , sizeof(ITEMSET)*_MAX_BELT_ITEM , &dwReadLen, NULL);

		// 将服务器接收的背包数据与客户端最后保存的腰带窗口数据进行比较，并分配到正确的位置。
		for ( nCnt1 = 0; nCnt1 < _MAX_BELT_ITEM; nCnt1++ )
		{
			if ( stBeltItem[nCnt1].bSetted )
			{
				for ( nCnt2 = 0; nCnt2 < _MAX_INVEN_ITEM; nCnt2++ )
				{
					if ( m_stInventoryItem[nCnt2].bSetted )
					{
						// 判断是否为相同物品.
						if ( stBeltItem[nCnt1].xItem.m_stItemInfo.nMakeIndex  != m_stInventoryItem[nCnt2].xItem.m_stItemInfo.nMakeIndex &&
							 !strcmp(stBeltItem[nCnt1].xItem.m_stItemInfo.stStdItem.szName, m_stInventoryItem[nCnt2].xItem.m_stItemInfo.stStdItem.szName) &&
							 (stBeltItem[nCnt1].xItem.m_stItemInfo.nDura    == m_stInventoryItem[nCnt2].xItem.m_stItemInfo.nDura   ) &&
							 (stBeltItem[nCnt1].xItem.m_stItemInfo.nDuraMax == m_stInventoryItem[nCnt2].xItem.m_stItemInfo.nDuraMax) )
						{
							stCehckedBeltItem[nCnt1] = m_stInventoryItem[nCnt2];
							ZeroMemory(&stBeltItem[nCnt1], sizeof(ITEMSET));
							DeleteInvenItem(nCnt2);
							break;
						}
					}
				}
			}
		}

		memcpy(m_stBeltItem, stCehckedBeltItem, sizeof(ITEMSET)*_MAX_BELT_ITEM);

		// 背包.
		for ( nCnt1 = 0; nCnt1 < _MAX_INVEN_ITEM; nCnt1++ )
		{
			if ( stInvenItem[nCnt1].bSetted )
			{
				for ( nCnt2 = 0; nCnt2 < _MAX_INVEN_ITEM; nCnt2++ )
				{
					if ( m_stInventoryItem[nCnt2].bSetted )
					{
						// 判断是否为相同物品.
						if ( stInvenItem[nCnt1].xItem.m_stItemInfo.nMakeIndex != m_stInventoryItem[nCnt2].xItem.m_stItemInfo.nMakeIndex &&
							 !strcmp(stInvenItem[nCnt1].xItem.m_stItemInfo.stStdItem.szName, m_stInventoryItem[nCnt2].xItem.m_stItemInfo.stStdItem.szName) &&
							 (stInvenItem[nCnt1].xItem.m_stItemInfo.nDura	 == m_stInventoryItem[nCnt2].xItem.m_stItemInfo.nDura    ) &&
							 (stInvenItem[nCnt1].xItem.m_stItemInfo.nDuraMax == m_stInventoryItem[nCnt2].xItem.m_stItemInfo.nDuraMax ) )
						{
							stCheckedInvenItem[nCnt1] = m_stInventoryItem[nCnt2];
							ZeroMemory(&stInvenItem[nCnt1], sizeof(ITEMSET));
							DeleteInvenItem(nCnt2);
							break;
						}
					}
				}
			}
		}

		for ( nCnt1 = 0; nCnt1 < _MAX_INVEN_ITEM; nCnt1++ )
		{
			if ( m_stInventoryItem[nCnt1].bSetted )
			{
				for ( nCnt2 = 0; nCnt2 < _MAX_INVEN_ITEM; nCnt2++ )
				{
					if ( !stCheckedInvenItem[nCnt2].bSetted )
					{
						stCheckedInvenItem[nCnt2] = m_stInventoryItem[nCnt1];
						break;
					}
				}
			}
		}
		memcpy(m_stInventoryItem, stCheckedInvenItem, sizeof(ITEMSET)*_MAX_INVEN_ITEM);

		CloseHandle(hFile);
		return TRUE;
	}

	return FALSE;
}


BOOL CInventoryWnd::SaveItemPos(CHAR* szName)
{
	HANDLE	hFile;
	CHAR	szItemFile[MAX_PATH];

	ZeroMemory(szItemFile, MAX_PATH);
	g_xMainWnd.StringPlus(szItemFile, ".\\Data\\", szName, ".itm", "");
	hFile = CreateFile(szItemFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if ( hFile != INVALID_HANDLE_VALUE )
	{
		DWORD		dwWriteLen;

		WriteFile(hFile, m_stInventoryItem, sizeof(ITEMSET)*_MAX_INVEN_ITEM, &dwWriteLen, NULL);
		WriteFile(hFile, m_stBeltItem,		sizeof(ITEMSET)*_MAX_BELT_ITEM , &dwWriteLen, NULL);

		CloseHandle(hFile);
		return TRUE;
	}
	return FALSE;
}


/******************************************************************************************************************

	函数名 : CInventoryWnd::GetInvenNum()

	作者 :
	编写日期 :

	目的   : 获取鼠标位置处当前背包的单元格编号。
	输入   : POINT ptMouse
	输出   : INT

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
INT CInventoryWnd::GetInvenNum(POINT ptMouse)
{
	RECT	rc;
	for ( INT nCnt = 0; nCnt < _INVEN_MAX_CELL; nCnt++ )
	{
		SetRect(&rc, m_rcWnd.left+m_rcInvenCell[nCnt].left, m_rcWnd.top+m_rcInvenCell[nCnt].top,
			    m_rcWnd.left+m_rcInvenCell[nCnt].left+_INVENTORY_CELL_WIDTH, m_rcWnd.top+m_rcInvenCell[nCnt].top+_INVENTORY_CELL_HEIGHT);
		if ( PtInRect(&rc, ptMouse) )
		{
			return (nCnt+m_nStartLineNum*_INVEN_CELL_XCNT);
		}
	}

	return -1;
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::GetBeltNum()

	作者 :
	创建日期 :

	目的   : 取得鼠标位置处当前腰带的格子编号.
	输入   : POINT ptMouse
	输出   : INT

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
INT CInventoryWnd::GetBeltNum(POINT ptMouse)
{
	RECT	rc;
	for ( INT nCnt = 0; nCnt < _MAX_BELT_ITEM; nCnt++ )
	{
		SetRect(&rc, m_rcWnd.left+m_rcBeltCell[nCnt].left, m_rcWnd.top+m_rcBeltCell[nCnt].top,
			    m_rcWnd.left+m_rcBeltCell[nCnt].left+_INVENTORY_CELL_WIDTH, m_rcWnd.top+m_rcBeltCell[nCnt].top+_INVENTORY_CELL_HEIGHT);
		if ( PtInRect(&rc, ptMouse) )
		{
			return (nCnt);
		}
	}

	return -1;
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::DeleteInventoryCurrItem()

	作者 :
	创建日期 :

	目的   : 删除背包中对应格子的物品.
	输入   : INT nInventoryNum
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::DeleteInvenItem(INT nInventoryNum)
{
	ZeroMemory(&m_stInventoryItem[nInventoryNum], sizeof(ITEMSET));
}



VOID CInventoryWnd::DeleteInvenItem(INT nMakeIndex, CHAR* szName)
{
	for ( INT nCnt = 0; nCnt < _MAX_INVEN_ITEM; nCnt++ )
	{
		if ( m_stInventoryItem[nCnt].bSetted )
		{
			if ( m_stInventoryItem[nCnt].xItem.m_stItemInfo.nMakeIndex == nMakeIndex )
			{
				if ( !strcmp(m_stInventoryItem[nCnt].xItem.m_stItemInfo.stStdItem.szName, szName) )
				{
					DeleteInvenItem(nCnt);
					break;
				}
			}
		}
	}
}


/******************************************************************************************************************

	函数名 : CInventoryWnd::DeleteInventoryAllItem()

	作者 :
	编写日期 :

	目的   : 清除背包中的所有物品.
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::DeleteAllInvenItem()
{
	for ( INT nCnt = 0; nCnt < _MAX_INVEN_ITEM; nCnt++ )
	{
		ZeroMemory(&m_stInventoryItem[nCnt], sizeof(ITEMSET));
	}
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::DeleteCurrBeltItemList()

	作者 :
	编写日期 :

	目的   : 删除腰带对应格子的物品.
	输入   : INT nBeltInventoryNum
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::DeleteBeltItem(INT nBeltInventoryNum)
{
	ZeroMemory(&m_stBeltItem[nBeltInventoryNum], sizeof(ITEMSET));
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::DeleteAllBeltItemList()

	作者 :
	编写日期 :

	目的   : 删除腰带中的所有物品.
	输入   : INT nBeltInventoryNum
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::DeleteAllBeltItem()
{
	for ( INT nCnt = 0; nCnt < _MAX_BELT_ITEM; nCnt++ )
	{
		ZeroMemory(&m_stBeltItem[nCnt], sizeof(ITEMSET));
	}
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::AddInvenItem()

	作者 :
	编写日期 :

	目的   : 将物品放入背包。
	输入   : CItem xItem
	         INT nInvenNum
	         BOOL bUseInvenNum
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CInventoryWnd::AddInvenItem(CItem xItem, INT nInvenNum, BOOL bUseInvenNum)
{
	if ( bUseInvenNum )
	{
		if ( !m_stInventoryItem[nInvenNum].bSetted )
		{
			m_stInventoryItem[nInvenNum].bSetted = TRUE;
			m_stInventoryItem[nInvenNum].xItem = xItem;

			return TRUE;
		}
	}

	for ( INT nCnt = 0; nCnt < _MAX_INVEN_ITEM; nCnt++ )
	{
		if ( !m_stInventoryItem[nCnt].bSetted )
		{
			m_stInventoryItem[nCnt].bSetted = TRUE;
			m_stInventoryItem[nCnt].xItem = xItem;

			return TRUE;
		}
	}

	return FALSE;
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::AddBeltItem()

	作者 :
	创建日期 :

	目的   : 把物品放入腰带.
	输入   : CItem xItem
	         INT nBeltNum
	         BOOL bUseBeltNum
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CInventoryWnd::AddBeltItem(CItem xItem, INT nBeltNum, BOOL bUseBeltNum)
{
	if ( bUseBeltNum )
	{
		if ( !m_stBeltItem[nBeltNum].bSetted )
		{
			m_stBeltItem[nBeltNum].bSetted = TRUE;
			m_stBeltItem[nBeltNum].xItem = xItem;

			return TRUE;
		}
	}

	for ( INT nCnt = 0; nCnt < _MAX_BELT_ITEM; nCnt++ )
	{
		if ( !m_stBeltItem[nCnt].bSetted )
		{
			m_stBeltItem[nCnt].bSetted = TRUE;
			m_stBeltItem[nCnt].xItem = xItem;

			return TRUE;
		}
	}
	return FALSE;
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::ExtractInvenItem()

	作者 :
	编写日期 :

	目的   : 从背包中取出当前编号的物品。
	输入   : CItem* pxItem
	         INT nInventoryNum
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CInventoryWnd::ExtractInvenItem(CItem* pxItem, INT nInventoryNum)
{
	if ( m_stInventoryItem[nInventoryNum].bSetted )
	{
		memcpy(pxItem, &m_stInventoryItem[nInventoryNum].xItem, sizeof(CItem));
		DeleteInvenItem(nInventoryNum);

		return TRUE;
	}
	return FALSE;
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::ExtractBeltItem()

	作者 :
	编写日期 :

	目的   : 从背包窗口中取出当前编号的物品.
	输入   : CItem* pxItem
	         INT nBeltInventoryNum
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CInventoryWnd::ExtractBeltItem(CItem* pxItem, INT nBeltInventoryNum)
{
	if ( m_stBeltItem[nBeltInventoryNum].bSetted )
	{
		memcpy(pxItem, &m_stBeltItem[nBeltInventoryNum].xItem, sizeof(CItem));
		DeleteBeltItem(nBeltInventoryNum);

		return TRUE;
	}
	return FALSE;
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::ChangeInvenWithCommon()

	作者 :
	编写日期 :

	目的   : 将背包中的物品与公共(鼠标)物品交换.
	输入   : INT nInventoryNum
	         LPCOMMONITEMSET pstCommonItemSet
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::ChangeInvenWithCommon(INT nInventoryNum, LPCOMMONITEMSET pstCommonItemSet)
{
	if ( pstCommonItemSet->bSetted )
	{
		CItem xInvenItem;
		if ( ExtractInvenItem(&xInvenItem, nInventoryNum) )
		{
			AddInvenItem(pstCommonItemSet->xItem, nInventoryNum, TRUE);

			pstCommonItemSet->bSetted			= TRUE;
			pstCommonItemSet->xItem				= xInvenItem;
			pstCommonItemSet->bWaitResponse		= FALSE;
			pstCommonItemSet->bIsEquipItem		= FALSE;
			pstCommonItemSet->bIsHideItem		= FALSE;
			pstCommonItemSet->bIsBeltItem		= FALSE;
			pstCommonItemSet->xItem.m_shCellNum = nInventoryNum;
		}
	}
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::ChangeBeltWithCommon()

	作者 :
	日期 :

	目的   : 更换腰带物品与公共(鼠标)物品。
	输入   : INT nBeltInventoryNum
	         LPCOMMONITEMSET pstCommonItemSet
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::ChangeBeltWithCommon(INT nBeltInventoryNum, LPCOMMONITEMSET pstCommonItemSet)
{
	if ( pstCommonItemSet->bSetted )
	{
		CItem xBeltItem;
		if ( ExtractBeltItem(&xBeltItem, nBeltInventoryNum) )
		{
			AddBeltItem(pstCommonItemSet->xItem, nBeltInventoryNum, TRUE);

			pstCommonItemSet->bSetted			= TRUE;
			pstCommonItemSet->xItem				= xBeltItem;
			pstCommonItemSet->bWaitResponse		= FALSE;
			pstCommonItemSet->bIsEquipItem		= FALSE;
			pstCommonItemSet->bIsHideItem		= FALSE;
			pstCommonItemSet->bIsBeltItem		= TRUE;
			pstCommonItemSet->xItem.m_shCellNum = nBeltInventoryNum;
		}
	}
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::SetCommonFromInven()

	作者 :
	创建日期 :

	目的   : 把背包中对应编号的物品设置为公共(鼠标)物品.
	输入   : INT nInventoryNum
	         LPCOMMONITEMSET pstCommonItemSet
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::SetCommonFromInven(INT nInventoryNum, LPCOMMONITEMSET pstCommonItemSet)
{
	CItem xInvenItem;
	if ( ExtractInvenItem(&xInvenItem, nInventoryNum) )
	{
		pstCommonItemSet->bSetted			= TRUE;
		pstCommonItemSet->xItem				= xInvenItem;
		pstCommonItemSet->bWaitResponse		= FALSE;
		pstCommonItemSet->bIsEquipItem		= FALSE;
		pstCommonItemSet->bIsHideItem		= FALSE;
		pstCommonItemSet->bIsBeltItem		= FALSE;
		pstCommonItemSet->xItem.m_shCellNum = nInventoryNum;
	}
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::SetCommonFromBelt()

	作者 :
	编写日期 :

	目的   : 把腰带窗口中对应编号的物品设置为公共(鼠标)物品。
	输入   : INT nBeltInventoryNum
	         LPCOMMONITEMSET pstCommonItemSet
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::SetCommonFromBelt(INT nBeltInventoryNum, LPCOMMONITEMSET pstCommonItemSet)
{
	CItem xBeltItem;
	if ( ExtractBeltItem(&xBeltItem, nBeltInventoryNum) )
	{
		pstCommonItemSet->bSetted			= TRUE;
		pstCommonItemSet->xItem				= xBeltItem;
		pstCommonItemSet->bWaitResponse		= FALSE;
		pstCommonItemSet->bIsEquipItem		= FALSE;
		pstCommonItemSet->bIsHideItem		= FALSE;
		pstCommonItemSet->bIsBeltItem		= TRUE;
		pstCommonItemSet->xItem.m_shCellNum = nBeltInventoryNum;
	}
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::SetInvenFromCommon()

	作者 :
	编写日期 :

	目的   : 将公共(鼠标)物品设置到背包的对应编号。
	输入   : INT nInventoryNum
	         LPCOMMONITEMSET pstCommonItemSet
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::SetInvenFromCommon(INT nInventoryNum, LPCOMMONITEMSET pstCommonItemSet)
{
	if ( pstCommonItemSet->bSetted )
	{
		pstCommonItemSet->xItem.m_shCellNum = nInventoryNum;
		AddInvenItem(pstCommonItemSet->xItem, nInventoryNum, TRUE);

		ZeroMemory(pstCommonItemSet, sizeof(COMMONITEMSET));
	}
}



/******************************************************************************************************************

	函数名 : CInventoryWnd::SetBeltFromCommon()

	作者 :
	编写日期 :

	目的   : 把公共(鼠标)物品设置到腰带窗口的对应编号上.
	输入   : INT nBeltInventoryNum
	         LPCOMMONITEMSET pstCommonItemSet
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInventoryWnd::SetBeltFromCommon(INT nBeltInventoryNum, LPCOMMONITEMSET pstCommonItemSet)
{
	if ( pstCommonItemSet->bSetted )
	{
		pstCommonItemSet->xItem.m_shCellNum = nBeltInventoryNum;
		AddBeltItem(pstCommonItemSet->xItem, nBeltInventoryNum);

		ZeroMemory(pstCommonItemSet, sizeof(LPCOMMONITEMSET));
	}
}


VOID CInventoryWnd::AddNewBeltItem(LPCOMMONITEMSET pstCommonItemSet)
{
	INT nCnt;
	if ( pstCommonItemSet->bSetted && pstCommonItemSet->bIsBeltItem )
	{
		if ( /*pstCommonItemSet->xItem.m_stItemInfo.szMakeIndex[0] == 'G' && */
			 (pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode == 0 || pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode == 3) )
		{
			for ( nCnt = 0; nCnt < _MAX_INVEN_ITEM; nCnt++ )
			{
				if ( m_stInventoryItem[nCnt].bSetted &&
					 m_stInventoryItem[nCnt].xItem.m_stItemInfo.stStdItem.bStdMode == pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode &&
					 m_stInventoryItem[nCnt].xItem.m_stItemInfo.stStdItem.bShape == pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bShape &&
					 m_stInventoryItem[nCnt].xItem.m_stItemInfo.nDura == pstCommonItemSet->xItem.m_stItemInfo.nDura &&
					 m_stInventoryItem[nCnt].xItem.m_stItemInfo.nDuraMax == pstCommonItemSet->xItem.m_stItemInfo.nDuraMax )

				{
					if ( !m_stBeltItem[pstCommonItemSet->xItem.m_shCellNum].bSetted )
					{
						m_stBeltItem[pstCommonItemSet->xItem.m_shCellNum].xItem = m_stInventoryItem[nCnt].xItem;
						m_stBeltItem[pstCommonItemSet->xItem.m_shCellNum].bSetted = TRUE;
						DeleteInvenItem(nCnt);
					}
					break;
				}
			}
		}
	}
}












/******************************************************************************************************************

	Message Function.(鼠标输入)

*******************************************************************************************************************/
BOOL CInventoryWnd::OnKeyUp(LPCOMMONITEMSET pstCommonItemSet, WPARAM wParam, LPARAM lParam)
{
	INT nBeltInventoryNum;

	nBeltInventoryNum = -1;

	switch ( wParam )
	{
	case '1': 	nBeltInventoryNum = 0;		break;
	case '2': 	nBeltInventoryNum = 1;		break;
	case '3': 	nBeltInventoryNum = 2;		break;
	case '4': 	nBeltInventoryNum = 3;		break;
	case '5': 	nBeltInventoryNum = 4;		break;
	case '6': 	nBeltInventoryNum = 5;		break;
	}

	if ( nBeltInventoryNum != -1 )
	{
		if ( m_stBeltItem[nBeltInventoryNum].bSetted )
		{
			if ( !pstCommonItemSet->bSetted )
			{
				SetCommonFromBelt(nBeltInventoryNum, pstCommonItemSet);
			}
			else if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
			{
				if ( /*pstCommonItemSet->xItem.m_stItemInfo.szMakeIndex[0] == 'G' && */
					 (pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode == 0 || pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode == 3) )
					ChangeBeltWithCommon(nBeltInventoryNum, pstCommonItemSet);
			}
		}
		else
		{
			if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
			{
				if ( /*pstCommonItemSet->xItem.m_stItemInfo.szMakeIndex[0] == 'G' && */
					 (pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode == 0 || pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode == 3) )
					 SetBeltFromCommon(nBeltInventoryNum, pstCommonItemSet);
			}
		}

		if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
		{
			pstCommonItemSet->bWaitResponse = TRUE;
			pstCommonItemSet->bIsHideItem	= TRUE;
			g_xClientSocket.SendItemIndex(CM_EAT, pstCommonItemSet->xItem.m_stItemInfo.nMakeIndex, pstCommonItemSet->xItem.m_stItemInfo.stStdItem.szName);
			return TRUE;
		}
	}

	return FALSE;
}


BOOL CInventoryWnd::OnLButtonDown(LPCOMMONITEMSET pstCommonItemSet, POINT ptMouse)
{
	INT nCnt, nInventoryNum, nBeltInventoryNum;

	m_xInventoryBtn[_BTN_ID_INVENCLOSE	 ].ChangeRect(m_rcWnd.left+255, m_rcWnd.top+291);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLUP	 ].ChangeRect(m_rcWnd.left+247, m_rcWnd.top+ 20);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLDOWN].ChangeRect(m_rcWnd.left+247, m_rcWnd.top+167);


	if ( m_xInvenScrlBar.OnLButtonDown(ptMouse) )
	{
		FLOAT	fScrlRate;

		fScrlRate	= m_xInvenScrlBar.GetScrlRate();
		m_nStartLineNum = 11*fScrlRate;

		return FALSE;
	}


	for( nCnt = 0; nCnt < _MAX_INVEN_BTN; nCnt++ )
	{
		if ( m_xInventoryBtn[nCnt].OnLButtonDown(ptMouse) )
			return TRUE;
	}

	nInventoryNum	  = GetInvenNum(ptMouse);
	nBeltInventoryNum = GetBeltNum(ptMouse);

	if ( (nInventoryNum == -1 && nBeltInventoryNum == -1) )		m_bCanMove = TRUE;
	else
	{
		if ( nInventoryNum != -1 )
		{
			if ( m_stInventoryItem[nInventoryNum].bSetted )		m_bCanMove = FALSE;
			else												m_bCanMove = TRUE;
		}
		else if ( nBeltInventoryNum != -1 )
		{
			if ( m_stBeltItem[nInventoryNum].bSetted )			m_bCanMove = FALSE;
			else												m_bCanMove = TRUE;
		}
	}

	return FALSE;
}



BOOL CInventoryWnd::OnLButtonUp(LPCOMMONITEMSET pstCommonItemSet, POINT ptMouse)
{
	INT nInventoryNum, nBeltInventoryNum;

	m_xInventoryBtn[_BTN_ID_INVENCLOSE	 ].ChangeRect(m_rcWnd.left+255, m_rcWnd.top+291);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLUP	 ].ChangeRect(m_rcWnd.left+247, m_rcWnd.top+ 20);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLDOWN].ChangeRect(m_rcWnd.left+247, m_rcWnd.top+167);

	if ( m_xInvenScrlBar.OnLButtonUp(ptMouse) )
	{
		return TRUE;
	}

	// 关闭背包窗口.
	if ( m_xInventoryBtn[_BTN_ID_INVENCLOSE].OnLButtonUp(ptMouse) )
		return TRUE;

	// 滚动 Up, Down 处理.
//	m_xInventoryBtn[_BTN_ID_INVENSCRLUP	 ].OnLButtonUp(ptMouse);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLDOWN].OnLButtonUp(ptMouse);

	nInventoryNum	  = GetInvenNum(ptMouse);
	nBeltInventoryNum = GetBeltNum(ptMouse);

	// 通用(鼠标)物品栏中有来自装备窗口的物品，且不在等待服务器响应信号的状态时，关闭物品穿戴.
	if ( pstCommonItemSet->bSetted && pstCommonItemSet->bIsEquipItem && !pstCommonItemSet->bWaitResponse )
	{
		nInventoryNum = GetInvenNum(ptMouse);
		if ( nInventoryNum != -1 )
		{
			g_xClientSocket.SendTakeOnOffItem(CM_TAKEOFFITEM, pstCommonItemSet->xItem.m_shCellNum, pstCommonItemSet->xItem.m_stItemInfo.stStdItem.szName, pstCommonItemSet->xItem.m_stItemInfo.nMakeIndex);
			pstCommonItemSet->bWaitResponse	= TRUE;
			return FALSE;
		}
	}

	// 在背包格子区域内点击的情况.
	if ( nInventoryNum != -1 )
	{
		// 点击的背包中有物品时。
		if ( m_stInventoryItem[nInventoryNum].bSetted )
		{
			// 如果公共(鼠标)物品集为空，
			if ( !pstCommonItemSet->bSetted )
			{
				// 把物品附着在鼠标上.
				SetCommonFromInven(nInventoryNum, pstCommonItemSet);
			}
			// 存在公共(鼠标)物品集, 不在等待服务器应答信号的过程中, 且不是已穿戴物品时.
			else if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
			{
				ChangeInvenWithCommon(nInventoryNum, pstCommonItemSet);
			}
		}
		// 点击的背包中没有物品时。
		else
		{
			// 存在公共(鼠标)物品集，且不在等待服务器响应信号时，
			if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
			{
				SetInvenFromCommon(nInventoryNum, pstCommonItemSet);
			}
		}
	}

	if ( nBeltInventoryNum != -1 )
	{
		if ( m_stBeltItem[nBeltInventoryNum].bSetted )
		{
			if ( !pstCommonItemSet->bSetted )
			{
				SetCommonFromBelt(nBeltInventoryNum, pstCommonItemSet);
			}
			else if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
			{
				if ( /*pstCommonItemSet->xItem.m_stItemInfo.szMakeIndex[0] == 'G' && */
					 (pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode == 0 || pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode == 3) )
					ChangeBeltWithCommon(nBeltInventoryNum, pstCommonItemSet);
			}
		}
		else
		{
			if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
			{
				if ( /*pstCommonItemSet->xItem.m_stItemInfo.szMakeIndex[0] == 'G' && */
					 (pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode == 0 || pstCommonItemSet->xItem.m_stItemInfo.stStdItem.bStdMode == 3) )
					 SetBeltFromCommon(nBeltInventoryNum, pstCommonItemSet);
			}
		}
	}

	return FALSE;
}



BOOL CInventoryWnd::OnLButtonDoubleClick(LPCOMMONITEMSET pstCommonItemSet, POINT ptMouse)
{
	INT nInventoryNum	  = GetInvenNum(ptMouse);
	INT nBeltInventoryNum = GetBeltNum(ptMouse);

	if ( nInventoryNum != -1 )
	{
		// 点击的背包中有物品时。
		if ( m_stInventoryItem[nInventoryNum].bSetted )
		{
			// 如果公共(鼠标)物品集为空，
			if ( !pstCommonItemSet->bSetted )
			{
				// 把物品附着在鼠标上.
				SetCommonFromInven(nInventoryNum, pstCommonItemSet);
			}
			// 存在公共(鼠标)物品集，且不在等待服务器响应信号时，
			else if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
			{
				ChangeInvenWithCommon(nInventoryNum, pstCommonItemSet);
			}
		}
	}

	if ( nBeltInventoryNum != -1 )
	{
		if ( m_stBeltItem[nBeltInventoryNum].bSetted )
		{
			if ( !pstCommonItemSet->bSetted )
			{
				SetCommonFromBelt(nBeltInventoryNum, pstCommonItemSet);
			}
			else if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
			{
				ChangeBeltWithCommon(nBeltInventoryNum, pstCommonItemSet);
			}
		}
	}

	if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
	{
		pstCommonItemSet->bWaitResponse = TRUE;
		g_xClientSocket.SendItemIndex(CM_EAT, pstCommonItemSet->xItem.m_stItemInfo.nMakeIndex, pstCommonItemSet->xItem.m_stItemInfo.stStdItem.szName);
	}

	return TRUE;
}



VOID CInventoryWnd::OnMouseMove(POINT ptMouse)
{
	m_xInventoryBtn[_BTN_ID_INVENCLOSE	 ].ChangeRect(m_rcWnd.left+255, m_rcWnd.top+291);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLUP	 ].ChangeRect(m_rcWnd.left+247, m_rcWnd.top+ 20);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLDOWN].ChangeRect(m_rcWnd.left+247, m_rcWnd.top+167);

	for( INT nCnt = 0; nCnt < _MAX_INVEN_BTN; nCnt++ )
		m_xInventoryBtn[nCnt].OnMouseMove(ptMouse);
}



VOID CInventoryWnd::OnScrollDown()
{
	if ( m_nStartLineNum > 0 )												m_nStartLineNum--;
}



VOID CInventoryWnd::OnScrollUp()
{
//	if ( m_nStartLineNum < _INVEN_MAX_CELL_YCNT - _INVEN_CELL_YCNT )		m_nStartLineNum++;
	if ( m_nStartLineNum < (_INVEN_MAX_CELL_YCNT-1) )
	{
		m_nStartLineNum++;
	}
}

// StatusWnd
/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/






/******************************************************************************************************************

	CStatusWnd Class

*******************************************************************************************************************/

CStatusWnd::CStatusWnd()
{
	Init();
}


CStatusWnd::~CStatusWnd()
{
	Destory();
}


VOID CStatusWnd::Init()
{
	INT nCnt;
	CGameWnd::Init();

	m_bType	= _TYPE_EQUIP;
	m_nStartLineNum = 0;
	m_pstMyMagic = NULL;
	m_bMyMagicCnt = 0;
	ZeroMemory(&m_bMagicIdxTable, _MAX_MAGIC_SKILL);
	ZeroMemory(&m_rcCell, sizeof(RECT)*_EQUIP_MAX_CELL);
	ZeroMemory(&m_rcMagicCell, sizeof(RECT)*_MAGIC_MAX_CELL);
	ZeroMemory(&m_stEquipItem, sizeof(ITEMSET)*_EQUIP_MAX_CELL);
	ZeroMemory(&m_stTakeOnItemSet, sizeof(CItem));
	ZeroMemory(&m_szEquipItemIdx, sizeof(m_szEquipItemIdx));

	for ( nCnt  = 0; nCnt < _MAX_STAUTS_BTN; nCnt ++ )
	{
		m_xStatusBtn[nCnt].Init();
	}
	for ( nCnt  = 0; nCnt < 2; nCnt ++ )
	{
		m_xMagicScrlBtn[nCnt].Init();
	}

}

VOID CStatusWnd::Destory()
{
	CGameWnd::Destroy();
	SAFE_DELETE(m_pstMyMagic);
	Init();
}


VOID CStatusWnd::CreateStatusWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove)
{
	CreateGameWnd(nID, pxWndImage, nFrameImgIdx, bCanMove, nStartX, nStartY, nWidth, nHeight);

	// 按钮初始化.
	m_xStatusBtn[_BTN_ID_STATUSCLOSE].CreateGameBtn(pxWndImage,  214,  215, nStartX+222, nStartY+298);
	m_xStatusBtn[_BTN_ID_STATUS].CreateGameBtn(pxWndImage,  212,  213, nStartX+_BTN_ID_STATUS_X, nStartY+_BTN_ID_STATUS_Y);
	m_xStatusBtn[_BTN_ID_STATUSMAGIC].CreateGameBtn(pxWndImage,  99, 100, nStartX+_BTN_ID_STATUSMAGIC_X, nStartY+_BTN_ID_STATUSMAGIC_Y);

	// 装备窗口初始化。
	SetRect(&m_rcCell[_U_NECKLACE], 187,  70, 190+_INVENTORY_CELL_WIDTH,  70+_INVENTORY_CELL_HEIGHT);
	SetRect(&m_rcCell[_U_RIGHTHAND], 37, 264,  37+_INVENTORY_CELL_WIDTH, 264+_INVENTORY_CELL_HEIGHT);
	SetRect(&m_rcCell[_U_CHARM],	74, 264, 74+_INVENTORY_CELL_WIDTH, 264+_INVENTORY_CELL_HEIGHT);
	SetRect(&m_rcCell[_U_ARMRINGL],  37, 186,  37+_INVENTORY_CELL_WIDTH, 186+_INVENTORY_CELL_HEIGHT);
	SetRect(&m_rcCell[_U_ARMRINGR], 185, 186, 185+_INVENTORY_CELL_WIDTH, 186+_INVENTORY_CELL_HEIGHT);
	SetRect(&m_rcCell[_U_RINGL],	37, 227, 37+_INVENTORY_CELL_WIDTH, 227+_INVENTORY_CELL_HEIGHT);
	SetRect(&m_rcCell[_U_RINGR],   185, 227,185+_INVENTORY_CELL_WIDTH, 227+_INVENTORY_CELL_HEIGHT);
	SetRect(&m_rcCell[_U_HELMET],	104,  71, 143,  104);
	SetRect(&m_rcCell[_U_DRESS],	96,  114, 146,  204);
	SetRect(&m_rcCell[_U_WEAPON],	48,  70,   91,  154);

	// 魔法按钮初始化.
	m_xMagicScrlBtn[0].CreateGameBtn(pxWndImage, 74, 75, nStartX+232, nStartY+147);
	m_xMagicScrlBtn[1].CreateGameBtn(pxWndImage, 74, 75, nStartX+232, nStartY+253);

	// 魔法窗口初始化.
	SetRect(&m_rcMagicCell[0], 39,  77, 74, 112);
	SetRect(&m_rcMagicCell[1], 39, 115, 74, 149);
	SetRect(&m_rcMagicCell[2], 39, 152, 74, 186);
	SetRect(&m_rcMagicCell[3], 39, 188, 74, 223);
	SetRect(&m_rcMagicCell[4], 39, 225, 74, 260);

	// 魔法变量初始化.
	m_pstMyMagic  = NULL;
	m_bMyMagicCnt = 0;
}



VOID CStatusWnd::ShowPanel()
{
	RECT rcID;
	switch ( m_bType )
	{
	case _TYPE_EQUIP:
		ShowCharEquipWnd();
		SetRect(&rcID, m_rcWnd.left+81, m_rcWnd.top+38, m_rcWnd.left+173, m_rcWnd.top+50);
		g_xMainWnd.PutsHan(NULL, rcID, RGB(255, 250, 180), RGB(0, 0, 0), g_xGameProc.m_xMyHero.m_szName, NULL);

		rcID.top		+= 15;
		rcID.bottom		+= 15;

		g_xMainWnd.PutsHan(NULL, rcID, RGB(255, 250, 180), RGB(0, 0, 0), g_xGameProc.m_xMyHero.m_szGuildName, NULL);

		if ( g_xGameProc.m_xImage.m_xImageList[_IMAGE_INTER].NewSetIndex(204) )
		{
			g_xMainWnd.DrawWithImageForComp(
											m_rcWnd.left+187,
											m_rcWnd.top+266,
											g_xGameProc.m_xImage.m_xImageList[_IMAGE_INTER].m_lpstNewCurrWilImageInfo->shWidth,
											g_xGameProc.m_xImage.m_xImageList[_IMAGE_INTER].m_lpstNewCurrWilImageInfo->shHeight,
											(WORD*)(g_xGameProc.m_xImage.m_xImageList[_IMAGE_INTER].m_pbCurrImage));
		}


//		g_xMainWnd.DrawWithGDI(m_rcWnd, NULL, RGB(255, 250, 180), 1);

		break;
	case _TYPE_STATUS:

		ShowCharEquipWnd();
		SetRect(&rcID, m_rcWnd.left+81, m_rcWnd.top+38, m_rcWnd.left+173, m_rcWnd.top+50);
		g_xMainWnd.PutsHan(NULL, rcID, RGB(255, 250, 180), RGB(0, 0, 0), g_xGameProc.m_xMyHero.m_szName, g_xMainWnd.CreateGameFont("Arial", 10, 0, FW_BOLD));

		ShowCharStatus();

		if ( g_xGameProc.m_xImage.m_xImageList[_IMAGE_INTER].NewSetIndex(203) )
		{
			g_xMainWnd.DrawWithImageForComp(
											m_rcWnd.left+187,
											m_rcWnd.top+266,
											g_xGameProc.m_xImage.m_xImageList[_IMAGE_INTER].m_lpstNewCurrWilImageInfo->shWidth,
											g_xGameProc.m_xImage.m_xImageList[_IMAGE_INTER].m_lpstNewCurrWilImageInfo->shHeight,
											(WORD*)(g_xGameProc.m_xImage.m_xImageList[_IMAGE_INTER].m_pbCurrImage));
		}

//		g_xMainWnd.DrawWithGDI(m_rcWnd, NULL, RGB(255, 250, 180), 1);

		break;
	case _TYPE_SETMAGIC:
		ShowCharSetMagicWnd();
		SetRect(&rcID, m_rcWnd.left+81, m_rcWnd.top+38, m_rcWnd.left+173, m_rcWnd.top+50);
		g_xMainWnd.PutsHan(NULL, rcID, RGB(255, 250, 180), RGB(0, 0, 0), g_xGameProc.m_xMyHero.m_szName, g_xMainWnd.CreateGameFont("Arial", 10, 0, FW_BOLD));
		break;
	}

	m_xStatusBtn[_BTN_ID_STATUSCLOSE].ShowGameBtn();
	m_xStatusBtn[_BTN_ID_STATUS].ShowGameBtn();
	m_xStatusBtn[_BTN_ID_STATUSMAGIC].ShowGameBtn();
}



VOID CStatusWnd::SetStatusBtnInit()
{
	m_xStatusBtn[_BTN_ID_STATUSCLOSE].SetBtnState(_BTN_STATE_NORMAL);
	m_xStatusBtn[_BTN_ID_STATUS].SetBtnState(_BTN_STATE_NORMAL);
	m_xStatusBtn[_BTN_ID_STATUSMAGIC].SetBtnState(_BTN_STATE_NORMAL);
}











/******************************************************************************************************************

	角色装备窗口相关.

*******************************************************************************************************************/

VOID CStatusWnd::ShowCharEquipWnd()
{
	INT nStartX, nStartY;

	ShowGameWnd();

	// 区分是男性还是女性.
	if ( g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].NewSetIndex(g_xGameProc.m_xMyHero.m_stFeature.bGender) )
	{
		g_xMainWnd.DrawWithImageForComp(
						m_rcWnd.left+_EQUIP_CHAR_X+g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].m_lpstNewCurrWilImageInfo->shPX,
						m_rcWnd.top+_EQUIP_CHAR_Y+g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].m_lpstNewCurrWilImageInfo->shPY,
						g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].m_lpstNewCurrWilImageInfo->shWidth,
						g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].m_lpstNewCurrWilImageInfo->shHeight,
						(WORD*)(g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].m_pbCurrImage));
	}

	// 物品绘制.
	for ( INT nCnt = 0; nCnt < _EQUIP_MAX_CELL; nCnt++ )
	{
		if ( m_stEquipItem[nCnt].bSetted )
		{
//			m_stEquipItem[nCnt].xItem.m_stItemInfo.stStdItem.wLooks = 1;
			if ( nCnt!=_U_HELMET && nCnt!=_U_WEAPON && nCnt!=_U_DRESS )
			{
				nStartX = m_rcWnd.left+m_rcCell[nCnt].left;
				nStartY = m_rcWnd.top+m_rcCell[nCnt].top;

				m_stEquipItem[nCnt].xItem.DrawItem(nStartX, nStartY);
			}
			else
			{
				nStartX = m_rcWnd.left+_EQUIP_CHAR_X;
				nStartY = m_rcWnd.top+_EQUIP_CHAR_Y;

				m_stEquipItem[nCnt].xItem.DrawItem(nStartX, nStartY, _ITEM_TYPE_EQUIP);
			}

		}
	}

	ShowEquipItemState();
}


VOID CStatusWnd::ShowEquipItemState()
{
	INT nEquipNum = GetEquipNum(g_xGameProc.m_ptMousePos);

	if ( nEquipNum != -1 )
	{
		if ( m_stEquipItem[nEquipNum].bSetted )
		{
			m_stEquipItem[nEquipNum].xItem.ShowItemStatus(m_rcWnd.left+37, m_rcWnd.top+288);
		}
	}
}



INT CStatusWnd::GetEquipNum(POINT ptMouse)
{
	RECT rc;
	for ( INT nCnt = 0; nCnt < _EQUIP_MAX_CELL; nCnt++ )
	{
		SetRect(&rc, m_rcWnd.left+m_rcCell[nCnt].left, m_rcWnd.top+m_rcCell[nCnt].top,
			    m_rcWnd.left+m_rcCell[nCnt].left+(m_rcCell[nCnt].right-m_rcCell[nCnt].left), m_rcWnd.top+m_rcCell[nCnt].top+(m_rcCell[nCnt].bottom-m_rcCell[nCnt].top));
		if ( PtInRect(&rc, ptMouse) )
		{
			return nCnt;
		}
	}

	return -1;
}



BOOL CStatusWnd::CheckEquipItem(CItem* pxItem, BYTE bEquipCell, POINT ptMouse)
{
	RECT rc;

	for ( INT nCnt = 0; nCnt < _EQUIP_MAX_CELL; nCnt++ )
	{
		SetRect(&rc, m_rcWnd.left+m_rcCell[nCnt].left, m_rcWnd.top+m_rcCell[nCnt].top,
			    m_rcWnd.left+m_rcCell[nCnt].left+(m_rcCell[nCnt].right-m_rcCell[nCnt].left), m_rcWnd.top+m_rcCell[nCnt].top+(m_rcCell[nCnt].bottom-m_rcCell[nCnt].top));
		if ( PtInRect(&rc, ptMouse) )
		{
			if ( pxItem->m_bAttr == _U_ARMRINGL || pxItem->m_bAttr == _U_ARMRINGR )
			{
				if ( pxItem->m_stItemInfo.stStdItem.bStdMode == 25 )
				{
					if ( _U_ARMRINGL == bEquipCell )
						return TRUE;
				}
				else
				{
					if ( _U_ARMRINGL == bEquipCell || _U_ARMRINGR == bEquipCell )			return TRUE;
				}
			}
			else if ( pxItem->m_bAttr == _U_RINGL || pxItem->m_bAttr ==  _U_RINGR )
			{
				if ( _U_RINGL == bEquipCell || _U_RINGR == bEquipCell )					return TRUE;
			}
			else if ( pxItem->m_bAttr == bEquipCell )
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}



VOID CStatusWnd::DeleteEquipItem(INT nEquipNum)
{
	ZeroMemory(&m_stEquipItem[nEquipNum], sizeof(ITEMSET));
}



VOID CStatusWnd::DeleteEquipItem(INT nMakeIndex, CHAR* szName)
{
	for ( INT nCnt = 0; nCnt < _EQUIP_MAX_CELL; nCnt++ )
	{
		if ( m_stEquipItem[nCnt].bSetted )
		{
			if ( m_stEquipItem[nCnt].xItem.m_stItemInfo.nMakeIndex == nMakeIndex )
			{
				if ( !strcmp(m_stEquipItem[nCnt].xItem.m_stItemInfo.stStdItem.szName, szName) )
				{
					DeleteEquipItem(nCnt);
					break;
				}
			}
		}
	}
}



VOID CStatusWnd::DeleteAllEquipItem()
{
	for ( INT nCnt = 0; nCnt < _EQUIP_MAX_CELL; nCnt++ )
	{
		ZeroMemory(&m_stEquipItem[nCnt], sizeof(ITEMSET));
	}
}



BOOL CStatusWnd::AddEquipItem(CItem xItem, INT nEquipNum, BOOL bUseEquipNum)
{
	if ( bUseEquipNum )
	{
		if ( !m_stEquipItem[nEquipNum].bSetted )
		{
			m_stEquipItem[nEquipNum].bSetted = TRUE;
			m_stEquipItem[nEquipNum].xItem = xItem;
			m_stEquipItem[nEquipNum].xItem.m_shCellNum = nEquipNum;

			return TRUE;
		}
	}

	for ( INT nCnt = 0; nCnt < _EQUIP_MAX_CELL; nCnt++ )
	{
		if ( !m_stEquipItem[nCnt].bSetted )
		{
			m_stEquipItem[nCnt].bSetted = TRUE;
			m_stEquipItem[nCnt].xItem = xItem;
			m_stEquipItem[nCnt].xItem.m_shCellNum = nCnt;

			return TRUE;
		}
	}
	return FALSE;
}



BOOL CStatusWnd::ExtractEquipItem(CItem* pxItem, INT nEquipNum)
{
	if ( m_stEquipItem[nEquipNum].bSetted )
	{
		memcpy(pxItem, &m_stEquipItem[nEquipNum].xItem, sizeof(CItem));
		DeleteEquipItem(nEquipNum);

		return TRUE;
	}
	return FALSE;
}





VOID CStatusWnd::ChangeEquipWithCommon(INT nEquipNum, LPCOMMONITEMSET pstCommonItemSet)
{
	if ( pstCommonItemSet->bSetted )
	{
		CItem xEquipItem;
		if ( ExtractEquipItem(&xEquipItem, nEquipNum) )
		{
			AddEquipItem(pstCommonItemSet->xItem, nEquipNum, TRUE);

			pstCommonItemSet->bSetted			= TRUE;
			pstCommonItemSet->xItem				= xEquipItem;
			pstCommonItemSet->bWaitResponse		= FALSE;
			pstCommonItemSet->bIsEquipItem		= TRUE;
			pstCommonItemSet->bIsHideItem		= FALSE;
			pstCommonItemSet->bIsBeltItem		= FALSE;
			pstCommonItemSet->xItem.m_shCellNum = nEquipNum;
		}
	}
}




VOID CStatusWnd::SetCommonFromEquip(INT nEquipNum, LPCOMMONITEMSET pstCommonItemSet)
{
	CItem xEquipItem;
	if ( ExtractEquipItem(&xEquipItem, nEquipNum) )
	{
		pstCommonItemSet->bSetted			= TRUE;
		pstCommonItemSet->xItem				= xEquipItem;
		pstCommonItemSet->bWaitResponse		= FALSE;
		pstCommonItemSet->bIsEquipItem		= TRUE;
		pstCommonItemSet->bIsHideItem		= FALSE;
		pstCommonItemSet->bIsBeltItem		= FALSE;
		pstCommonItemSet->xItem.m_shCellNum = nEquipNum;
	}
}



VOID CStatusWnd::SetEquipFromCommon(INT nEquipNum, LPCOMMONITEMSET pstCommonItemSet)
{
	if ( pstCommonItemSet->bSetted )
	{
		pstCommonItemSet->xItem.m_shCellNum = nEquipNum;
		AddEquipItem(pstCommonItemSet->xItem, nEquipNum);
		ZeroMemory(pstCommonItemSet, sizeof(COMMONITEMSET));
	}
}



VOID CStatusWnd::SetTakeOnFromEquip(INT nEquipNum)
{
	if ( m_stEquipItem[nEquipNum].bSetted )
	{
		m_stTakeOnItemSet.xItem	  = m_stEquipItem[nEquipNum].xItem;
		m_stTakeOnItemSet.bSetted = TRUE;
		m_stTakeOnItemSet.xItem.m_shCellNum = nEquipNum;
		ZeroMemory(&m_stEquipItem[nEquipNum], sizeof(ITEMSET));
	}
}



VOID CStatusWnd::SetTakeOnFromCommon(INT nEquipNum, LPCOMMONITEMSET pstCommonItemSet)
{
	if ( pstCommonItemSet->bSetted )
	{
		m_stTakeOnItemSet.xItem   = pstCommonItemSet->xItem;
		m_stTakeOnItemSet.bSetted = TRUE;
		m_stTakeOnItemSet.xItem.m_shCellNum = nEquipNum;
		ZeroMemory(pstCommonItemSet, sizeof(COMMONITEMSET));
	}
}



VOID CStatusWnd::SetEquipFromTakeOn()
{
	if ( m_stTakeOnItemSet.bSetted )
	{
		AddEquipItem(m_stTakeOnItemSet.xItem, m_stTakeOnItemSet.xItem.m_shCellNum, TRUE);
		ZeroMemory(&m_stTakeOnItemSet, sizeof(ITEMSET));
	}
}





















/******************************************************************************************************************

	角色状态信息窗口相关.

*******************************************************************************************************************/

VOID CStatusWnd::ShowCharStatus()
{
	CHAR	szBuff[MAX_PATH];

	POINT	ptStart = {m_rcWnd.left+255+15, m_rcWnd.top+67};

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "LEVEL", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d", g_xGameProc.m_xMyHero.m_stAbility.bLevel);
	g_xMainWnd.PutsHan(NULL, ptStart.x+53, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "HP", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d / %d", g_xGameProc.m_xMyHero.m_stAbility.wHP, g_xGameProc.m_xMyHero.m_stAbility.wMaxHP);
	g_xMainWnd.PutsHan(NULL, ptStart.x+53, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "MP", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d / %d", g_xGameProc.m_xMyHero.m_stAbility.wMP, g_xGameProc.m_xMyHero.m_stAbility.wMaxMP);
	g_xMainWnd.PutsHan(NULL, ptStart.x+53, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "AC", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d-%d", LOBYTE(g_xGameProc.m_xMyHero.m_stAbility.wAC), HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.wAC));
	g_xMainWnd.PutsHan(NULL, ptStart.x+53, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "MAC", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d-%d", LOBYTE(g_xGameProc.m_xMyHero.m_stAbility.wMAC), HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.wMAC));
	g_xMainWnd.PutsHan(NULL, ptStart.x+53, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "DC", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d-%d", LOBYTE(g_xGameProc.m_xMyHero.m_stAbility.wDC), HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.wDC));
	g_xMainWnd.PutsHan(NULL, ptStart.x+53, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "MC", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d-%d", LOBYTE(g_xGameProc.m_xMyHero.m_stAbility.wMC), HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.wMC));
	g_xMainWnd.PutsHan(NULL, ptStart.x+53, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "SC", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d-%d", LOBYTE(g_xGameProc.m_xMyHero.m_stAbility.wSC), HIBYTE(g_xGameProc.m_xMyHero.m_stAbility.wSC));
	g_xMainWnd.PutsHan(NULL, ptStart.x+53, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.x = m_rcWnd.left+255+145;
	ptStart.y = m_rcWnd.top+67;

	FLOAT fExpRate = (FLOAT)((FLOAT)g_xGameProc.m_xMyHero.m_stAbility.dwExp/(FLOAT)g_xGameProc.m_xMyHero.m_stAbility.dwMaxExp);
	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "EXP", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%.2f%s", fExpRate*100, "%%");
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "Bag Weight", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d / %d", g_xGameProc.m_xMyHero.m_stAbility.wWeight, g_xGameProc.m_xMyHero.m_stAbility.wMaxWeight);
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "Equip Weight", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d / %d", g_xGameProc.m_xMyHero.m_stAbility.bWearWeight, g_xGameProc.m_xMyHero.m_stAbility.bMaxWearWeight);
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "Two-Hand Weight", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "%d / %d", g_xGameProc.m_xMyHero.m_stAbility.bHandWeight, g_xGameProc.m_xMyHero.m_stAbility.bMaxHandWeight);
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "Accuracy", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "+%d%", g_xGameProc.m_xMyHero.m_stSubAbility.bHitPoint);
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "Agility", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "+%d%", g_xGameProc.m_xMyHero.m_stSubAbility.bSpeedPoint);
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "Magic Evasion", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "+%d%", g_xGameProc.m_xMyHero.m_stSubAbility.wAntiMagic*10);
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "Poison Resist", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "+%d%", g_xGameProc.m_xMyHero.m_stSubAbility.bAntiPoison*10);
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "Poison Recovery", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "+%d%", g_xGameProc.m_xMyHero.m_stSubAbility.bPoisonRecover*10);
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "HP Recovery", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "+%d%", g_xGameProc.m_xMyHero.m_stSubAbility.bHealthRecover*10);
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);

	ptStart.y += 20;

	g_xMainWnd.PutsHan(NULL, ptStart.x, ptStart.y, RGB(200, 225, 250), RGB(0, 0, 0), "MP Recovery", g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_BOLD));
	sprintf(szBuff, "+%d%", g_xGameProc.m_xMyHero.m_stSubAbility.bSpellRecover*10);
	g_xMainWnd.PutsHan(NULL, ptStart.x+55, ptStart.y, RGB(250, 250, 250), RGB(0, 0, 0), szBuff);
}














/******************************************************************************************************************

	角色魔法窗口相关.

*******************************************************************************************************************/
VOID CStatusWnd::ShowCharSetMagicWnd()
{
	INT nLine, nCnt, nStartX, nStartY;
	CHAR szKey[MAX_PATH];
	CHAR szLevel[MAX_PATH];

/*	CHAR* szSkillName[] =
	{
		"半月剑法",   "暗燃术",   "火焰风",   "降魔真法", "大地援护",   "焰蛇掌",	"无怠步",     "雷魂击",   "亚空行法", "狮子轮回", "火焰掌",	    "恢复术",
		"结界",	      "大恢复",   "雷雪花",   "雷印掌",   "白骨召唤术", "冰雪风",	"大隐身",     "咒术之幕", "探气破燃", "地焰术",   "爆杀界",     "爆裂波",
		"金刚火焰掌", "锐刀",	  "御剑术",	  "强击",	  "焰火诀",     "隐身",		"神兽召唤",	  "日光剑法",  "外修剑法",
	};
*/
	ShowGameWnd();

	for ( nCnt = 0; nCnt < _MAGIC_MAX_CELL; nCnt++ )
	{
		nLine = nCnt + m_nStartLineNum;

		if ( nLine >= m_bMyMagicCnt )			break;

		nStartX = m_rcWnd.left+m_rcMagicCell[nCnt].left;
		nStartY = m_rcWnd.top +m_rcMagicCell[nCnt].top;

		INT nMagicImgIdx = m_pstMyMagic[nLine].stStdMagic.wMagicID * 2;
		g_xGameProc.m_xImage.m_xImageList[_IMAGE_MICON].NewSetIndex(nMagicImgIdx);
		g_xMainWnd.DrawWithImageForComp(
										nStartX,
										nStartY,
										g_xGameProc.m_xImage.m_xImageList[_IMAGE_MICON].m_lpstNewCurrWilImageInfo->shWidth,
										g_xGameProc.m_xImage.m_xImageList[_IMAGE_MICON].m_lpstNewCurrWilImageInfo->shHeight,
										(WORD*)(g_xGameProc.m_xImage.m_xImageList[_IMAGE_MICON].m_pbCurrImage));

		g_xMainWnd.PutsHan(NULL, nStartX+50, nStartY+5, RGB(255, 255, 220), RGB(0, 0, 0), m_pstMyMagic[nLine].stStdMagic.szMagicName);

		if ( m_pstMyMagic[nLine].bUseKey != 0 )
		{
			nMagicImgIdx = m_pstMyMagic[nLine].stStdMagic.wMagicID * 2 + 1;
			g_xGameProc.m_xImage.m_xImageList[_IMAGE_MICON].NewSetIndex(nMagicImgIdx);
			g_xMainWnd.DrawWithImageForComp(
											nStartX,
											nStartY,
											g_xGameProc.m_xImage.m_xImageList[_IMAGE_MICON].m_lpstNewCurrWilImageInfo->shWidth,
											g_xGameProc.m_xImage.m_xImageList[_IMAGE_MICON].m_lpstNewCurrWilImageInfo->shHeight,
											(WORD*)(g_xGameProc.m_xImage.m_xImageList[_IMAGE_MICON].m_pbCurrImage));

			sprintf(szKey, "F%c", (char)m_pstMyMagic[nLine].bUseKey);
			g_xMainWnd.PutsHan(NULL, nStartX+140, nStartY+7, RGB(255, 255, 255), RGB(125, 125, 255), szKey);
		}


		if ( m_pstMyMagic[nLine].bLevel < 3 )
		{
			sprintf(szLevel, "%d        %d/%d", m_pstMyMagic[nLine].bLevel, m_pstMyMagic[nLine].nCurrTrain, m_pstMyMagic[nLine].stStdMagic.nTrain[m_pstMyMagic[nLine].bLevel]);
			g_xMainWnd.PutsHan(NULL, nStartX+50, nStartY+23, RGB(255, 255, 220), RGB(0, 0, 0), szLevel);
		}
		else
		{
			sprintf(szLevel, "%d        ", m_pstMyMagic[nLine].bLevel);
			g_xMainWnd.PutsHan(NULL, nStartX+50, nStartY+23, RGB(255, 255, 220), RGB(0, 0, 0), szLevel);
		}

		m_xMagicScrlBtn[0].ShowGameBtn();
		m_xMagicScrlBtn[1].ShowGameBtn();
	}
}


BOOL CStatusWnd::SetMagicKey(POINT ptMouse, BYTE bKey)
{
	RECT rc;
	INT  nCnt, nLine;
	for ( nCnt = 0; nCnt < _MAGIC_MAX_CELL; nCnt++ )
	{
		nLine = nCnt + m_nStartLineNum;

		if ( nLine >= m_bMyMagicCnt )			break;

		rc.left   = m_rcWnd.left + m_rcMagicCell[nCnt].left;
		rc.top	  = m_rcWnd.top  + m_rcMagicCell[nCnt].top;
		rc.right  = m_rcWnd.left + m_rcMagicCell[nCnt].right;
		rc.bottom = m_rcWnd.top	 + m_rcMagicCell[nCnt].bottom;

		if ( PtInRect(&rc, ptMouse) )
		{
			for ( INT nLoop = 0; nLoop < m_bMyMagicCnt; nLoop++ )
			{
				if ( m_pstMyMagic[nLoop].bUseKey == bKey )
				{
					m_pstMyMagic[nLoop].bUseKey = 0;
					g_xClientSocket.SendMagicKeyChange(m_pstMyMagic[nLoop].bUseKey, m_pstMyMagic[nLoop].stStdMagic.wMagicID);
				}
			}

			if ( m_pstMyMagic[nLine].bUseKey != bKey )
			{
				m_pstMyMagic[nLine].bUseKey = bKey;

				g_xClientSocket.SendMagicKeyChange(m_pstMyMagic[nLine].bUseKey, m_pstMyMagic[nLine].stStdMagic.wMagicID);
			}
			return TRUE;
		}
	}

	return FALSE;
}



LPCLIENTMAGICRCD CStatusWnd::GetMagicByKey(BYTE bKey)
{
	LPCLIENTMAGICRCD pstMagic = NULL;

	for ( INT nCnt = 0; nCnt < m_bMyMagicCnt; nCnt++ )
	{
		pstMagic = &m_pstMyMagic[nCnt];
		if ( pstMagic->bUseKey == bKey )
		{
			break;
		}
	}
	return pstMagic;
}






















/******************************************************************************************************************

	Message Function.

*******************************************************************************************************************/

BOOL CStatusWnd::OnKeyDown(WPARAM wParam, LPARAM lParam, POINT ptMouse)
{
	INT	 nLine, nCnt;
	BOOL bCheck;
	RECT rc;

	if ( !PtInRect(&m_rcWnd, ptMouse) )		return TRUE;

	bCheck = FALSE;

	for ( nCnt = 0; nCnt < _MAGIC_MAX_CELL; nCnt++ )
	{
		nLine = nCnt + m_nStartLineNum;

		rc.left  = m_rcWnd.left + m_rcMagicCell[nCnt].left;
		rc.top	 = m_rcWnd.top  + m_rcMagicCell[nCnt].top;
		rc.right = m_rcWnd.left + m_rcMagicCell[nCnt].right;
		rc.bottom= m_rcWnd.top	+ m_rcMagicCell[nCnt].bottom;
		if ( PtInRect(&rc, ptMouse) )
		{
			bCheck = TRUE;
			break;
		}
	}

	if ( !bCheck ) 							return TRUE;

	if ( m_bType == _TYPE_SETMAGIC )
	{
		switch ( wParam )
		{
		case VK_F1:		SetMagicKey(ptMouse, '1');		return TRUE;
		case VK_F2:		SetMagicKey(ptMouse, '2');		return TRUE;
		case VK_F3:		SetMagicKey(ptMouse, '3');		return TRUE;
		case VK_F4:		SetMagicKey(ptMouse, '4');		return TRUE;
		case VK_F5:		SetMagicKey(ptMouse, '5');		return TRUE;
		case VK_F6:		SetMagicKey(ptMouse, '6');		return TRUE;
		case VK_F7:		SetMagicKey(ptMouse, '7');		return TRUE;
		case VK_F8:		SetMagicKey(ptMouse, '8');		return TRUE;
		}
	}

	return FALSE;
}



BOOL CStatusWnd::OnLButtonUp(LPCOMMONITEMSET pstCommonItemSet, POINT ptMouse)
{
	INT nEquipNum;

	m_xStatusBtn[_BTN_ID_STATUSCLOSE].ChangeRect(m_rcWnd.left+222, m_rcWnd.top+298);
	m_xStatusBtn[_BTN_ID_STATUS     ].ChangeRect(m_rcWnd.left+_BTN_ID_STATUS_X, m_rcWnd.top+_BTN_ID_STATUS_Y);
	m_xStatusBtn[_BTN_ID_STATUSMAGIC].ChangeRect(m_rcWnd.left+_BTN_ID_STATUSMAGIC_X, m_rcWnd.top+_BTN_ID_STATUSMAGIC_Y);

	if ( m_xStatusBtn[_BTN_ID_STATUSCLOSE].OnLButtonUp(ptMouse) )		return TRUE;
	if ( m_xStatusBtn[_BTN_ID_STATUS].OnLButtonUp(ptMouse) )
	{
		if ( m_bType == _TYPE_STATUS )
		{
			ResetWndImg(_WNDIMGIDX_ITEMSET);
			m_bType = _TYPE_EQUIP;
			m_xStatusBtn[_BTN_ID_STATUS].ResetGameBtn(212, 213);
		}
		else
		{
			ResetWndImg(_WNDIMGIDX_STATUS);
			m_bType = _TYPE_STATUS;
			m_xStatusBtn[_BTN_ID_STATUS].ResetGameBtn(210, 211);

			if ( m_rcWnd.right > 800 )
			{
				m_rcWnd.left  = 260;
				m_rcWnd.right = 800;
			}
		}
	}
	if ( m_xStatusBtn[_BTN_ID_STATUSMAGIC].OnLButtonUp(ptMouse) )
	{
		m_bType = _TYPE_SETMAGIC;
		ResetWndImg(_WNDIMGIDX_SKILLSET);
	}

	if ( m_bType == _TYPE_SETMAGIC )
	{
		m_xMagicScrlBtn[0].ChangeRect(m_rcWnd.left+232, m_rcWnd.top+147);
		m_xMagicScrlBtn[1].ChangeRect(m_rcWnd.left+232, m_rcWnd.top+253);
		if ( m_xMagicScrlBtn[0].OnLButtonUp(ptMouse) )
		{
			if ( m_nStartLineNum > 0 )									m_nStartLineNum--;
		}

		if ( m_xMagicScrlBtn[1].OnLButtonUp(ptMouse) )
		{
			if ( m_nStartLineNum < m_bMyMagicCnt-_MAGIC_MAX_CELL)		m_nStartLineNum++;
		}
	}

	// 装备窗口处于激活状态时.
	if ( m_bType == _TYPE_EQUIP || m_bType == _TYPE_STATUS )
	{
		nEquipNum = GetEquipNum(ptMouse);
		// 鼠标位于装备窗口区域时.
		if ( nEquipNum != -1)
		{
			// 点击的装备窗口中有物品时.
			if ( m_stEquipItem[nEquipNum].bSetted )
			{
				// 如果公共(鼠标)物品集为空，
				if ( !pstCommonItemSet->bSetted )
				{
					// 把物品挂在鼠标上时, 在背包窗口点击的瞬间会发送 CM_TAKEOFFITEM 消息.
					SetCommonFromEquip(nEquipNum, pstCommonItemSet);
				}
				// 存在公共(鼠标)物品集，且不在等待服务器响应信号时，
				// 把物品预先设置到公共物品 m_stTakeOnItemSet 中, 再把当前点击的装备窗口物品设置为公共物品后,
				// 发送 CM_TAKEONITEM 消息.
				else if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse )
				{
					// 检查发送前的区域是否相互匹配(戒指是否放进了戒指位置).
					if ( CheckEquipItem(&pstCommonItemSet->xItem, nEquipNum, ptMouse) )
					{
						SetTakeOnFromCommon(nEquipNum, pstCommonItemSet);
						SetCommonFromEquip(nEquipNum, pstCommonItemSet);
						g_xClientSocket.SendTakeOnOffItem(CM_TAKEONITEM, nEquipNum, m_stTakeOnItemSet.xItem.m_stItemInfo.stStdItem.szName, m_stTakeOnItemSet.xItem.m_stItemInfo.nMakeIndex);
						pstCommonItemSet->bWaitResponse	= TRUE;
					}
				}
			}
			// 点击的装备窗口中没有物品时.
			else
			{
				// 存在公共(鼠标)物品集, 且不是在等待服务器响应信号时, 就把公共物品设置到 m_stTakeOnItemSet 中.
				if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse )
				{
					if ( CheckEquipItem(&pstCommonItemSet->xItem, nEquipNum, ptMouse) )
					{
						// 无需向服务器发送消息。卸下物品后又重新穿戴的情况。
						if ( pstCommonItemSet->bIsEquipItem )
						{
							SetEquipFromCommon(pstCommonItemSet->xItem.m_shCellNum, pstCommonItemSet);
							ZeroMemory(pstCommonItemSet, sizeof(COMMONITEMSET));
						}
						else
						{
							SetTakeOnFromCommon(nEquipNum, pstCommonItemSet);
							g_xClientSocket.SendTakeOnOffItem(CM_TAKEONITEM, nEquipNum, m_stTakeOnItemSet.xItem.m_stItemInfo.stStdItem.szName, m_stTakeOnItemSet.xItem.m_stItemInfo.nMakeIndex);
							pstCommonItemSet->bWaitResponse	= TRUE;
						}
					}
				}
			}
		}
	}

	return FALSE;
}



VOID CStatusWnd::OnMouseMove(POINT ptMouse)
{
	if ( m_bType == _TYPE_SETMAGIC )
	{
		m_xMagicScrlBtn[0].ChangeRect(m_rcWnd.left+232, m_rcWnd.top+147);
		m_xMagicScrlBtn[1].ChangeRect(m_rcWnd.left+232, m_rcWnd.top+253);
		m_xMagicScrlBtn[0].OnMouseMove(ptMouse);
		m_xMagicScrlBtn[1].OnMouseMove(ptMouse);
	}

	m_xStatusBtn[_BTN_ID_STATUSCLOSE   ].ChangeRect(m_rcWnd.left+222, m_rcWnd.top+298);
	m_xStatusBtn[_BTN_ID_STATUS		   ].ChangeRect(m_rcWnd.left+_BTN_ID_STATUS_X, m_rcWnd.top+_BTN_ID_STATUS_Y);
	m_xStatusBtn[_BTN_ID_STATUSMAGIC   ].ChangeRect(m_rcWnd.left+_BTN_ID_STATUSMAGIC_X, m_rcWnd.top+_BTN_ID_STATUSMAGIC_Y);
	m_xStatusBtn[_BTN_ID_STATUSCLOSE   ].OnMouseMove(ptMouse);
	m_xStatusBtn[_BTN_ID_STATUS		   ].OnMouseMove(ptMouse);
	m_xStatusBtn[_BTN_ID_STATUSMAGIC   ].OnMouseMove(ptMouse);
}



BOOL CStatusWnd::OnLButtonDown(LPCOMMONITEMSET pstCommonItemSet, POINT ptMouse)
{
	INT nEquipNum;

	m_xStatusBtn[_BTN_ID_STATUSCLOSE   ].ChangeRect(m_rcWnd.left+222, m_rcWnd.top+298);
	m_xStatusBtn[_BTN_ID_STATUS	       ].ChangeRect(m_rcWnd.left+_BTN_ID_STATUS_X, m_rcWnd.top+_BTN_ID_STATUS_Y);
	m_xStatusBtn[_BTN_ID_STATUSMAGIC   ].ChangeRect(m_rcWnd.left+_BTN_ID_STATUSMAGIC_X, m_rcWnd.top+_BTN_ID_STATUSMAGIC_Y);

	if ( m_xStatusBtn[_BTN_ID_STATUSCLOSE   ].OnLButtonDown(ptMouse) )		return TRUE;
	if ( m_xStatusBtn[_BTN_ID_STATUS	    ].OnLButtonDown(ptMouse) )		return TRUE;
	if ( m_xStatusBtn[_BTN_ID_STATUSMAGIC   ].OnLButtonDown(ptMouse) )		return TRUE;

	if ( m_bType == _TYPE_SETMAGIC )
	{
		m_xMagicScrlBtn[0].ChangeRect(m_rcWnd.left+232, m_rcWnd.top+147);
		m_xMagicScrlBtn[1].ChangeRect(m_rcWnd.left+232, m_rcWnd.top+253);
		if ( m_xMagicScrlBtn[0].OnLButtonDown(ptMouse) )					return TRUE;
		if ( m_xMagicScrlBtn[1].OnLButtonDown(ptMouse) )					return TRUE;
	}

	nEquipNum = GetEquipNum(ptMouse);

	if ( nEquipNum == -1 )		m_bCanMove = TRUE;
	else
	{
		if ( m_stEquipItem[nEquipNum].bSetted )		m_bCanMove = FALSE;
		else										m_bCanMove = TRUE;
	}

	return FALSE;
}



VOID CStatusWnd::OnScrollDown()
{
	if ( m_bType == _TYPE_SETMAGIC )
	{
		if ( m_nStartLineNum > 0 )										m_nStartLineNum--;
	}
}



VOID CStatusWnd::OnScrollUp()
{
	if ( m_bType == _TYPE_SETMAGIC )
	{
		if ( m_nStartLineNum < m_bMyMagicCnt-_MAGIC_MAX_CELL)			m_nStartLineNum++;
	}
}





void CButtonPanel::Create(INT id, CWHWilImageData* image, INT background, INT frame,
                          INT x, INT y, INT moveX, INT moveY)
{
	CreateGameWnd(id, image, background, TRUE, 0, 0, 0, 0);
	button.CreateGameBtn(image, frame, frame + 1, x, y);
	moveOffset = {moveX, moveY};
}

void CButtonPanel::OnMouseMove(POINT mouse)
{
	button.ChangeRect(m_rcWnd.left + moveOffset.x, m_rcWnd.top + moveOffset.y);
	button.OnMouseMove(mouse);
}

// GuildWnd
/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/





CGuildWnd::CGuildWnd()
{
	Init();
}

CGuildWnd::~CGuildWnd()
{
	Destroy();
}



VOID CGuildWnd::CreateGuildWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove)
{
	CreateGameWnd(nID, pxWndImage, nFrameImgIdx, bCanMove, nStartX, nStartY, nWidth, nHeight);
}

VOID CGuildWnd::ShowPanel()
{
	ShowGameWnd();
}

BOOL CGuildWnd::OnLButtonUp(LPCOMMONITEMSET, POINT ptMouse)
{
	return FALSE;
}

BOOL CGuildWnd::OnLButtonDown(LPCOMMONITEMSET, POINT ptMouse)
{
	return FALSE;
}

VOID CGuildWnd::OnMouseMove(POINT ptMouse)
{
}

// GroupWnd
/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/


typedef struct tagINTERFACE_BUTTON
{
public:
	int		m_nLeft;
	int		m_nTop;
	int		m_nWidth;
	int		m_nHeight;
	BYTE	m_btCommand;
} INTERFACE_BUTTON;

#define GROUPBTN_ALLOWGROUP		1
#define GROUPBTN_CREATEGROUP	2
#define GROUPBTN_ADDMEMBER		3
#define GROUPBTN_DELMEMBER		4

INTERFACE_BUTTON GroupWndButton[] = {	{ 21, 57, 27, 28, GROUPBTN_ALLOWGROUP },
										{ 27, 200, 71, 13, GROUPBTN_CREATEGROUP },
										{ 91, 200, 77, 16, GROUPBTN_ADDMEMBER },
										{ 170, 200, 73, 16, GROUPBTN_DELMEMBER }
									};

BOOL g_fAllowGroup = TRUE;

CGroupWnd::CGroupWnd()
{
	Init();
}

CGroupWnd::~CGroupWnd()
{
	Destroy();
}

VOID CGroupWnd::CreateGroupWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove)
{
	CreateGameWnd(nID, pxWndImage, nFrameImgIdx, bCanMove, nStartX, nStartY, nWidth, nHeight);
}

VOID CGroupWnd::ShowPanel()
{
	CGroupMember*	pGroupMember;

	RECT tRect = GetGameWndRect();

	int				nX = tRect.left + 45, nY = tRect.top + 100;

	ShowGameWnd();

	g_xMainWnd.PutsHan(NULL, tRect.left + 45, tRect.top + 25, RGB(255, 255, 255), RGB(0, 0, 0), g_szUserID, NULL);

	if (m_xGroupList.GetCounter())
	{
		m_xGroupList.MoveCurrentToTop();

		for (int i = 1; i <= m_xGroupList.GetCounter(); i++)
		{
			pGroupMember = m_xGroupList.GetCurrentData();

			g_xMainWnd.PutsHan(NULL, nX, nY, RGB(255, 255, 255), RGB(0, 0, 0), pGroupMember->szMemberName);

			if (i % 2)
				nX += 100;
			else
			{
				nX -= 100;
				nY += 50;
			}

			m_xGroupList.MoveNextNode();
		}
	}
}



BOOL CGroupWnd::OnLButtonUp(LPCOMMONITEMSET, POINT ptMouse)
{
	RECT tRect = GetGameWndRect();

	for (int i = 0; i < sizeof(GroupWndButton)/sizeof(INTERFACE_BUTTON); i++)
	{
		if (tRect.left + GroupWndButton[i].m_nLeft < ptMouse.x && tRect.left + GroupWndButton[i].m_nLeft + GroupWndButton[i].m_nWidth > ptMouse.x &&
			tRect.top + GroupWndButton[i].m_nTop < ptMouse.y && tRect.top + GroupWndButton[i].m_nTop + GroupWndButton[i].m_nHeight > ptMouse.y)
		{
			switch (GroupWndButton[i].m_btCommand)
			{
				case GROUPBTN_ALLOWGROUP:
					(g_fAllowGroup ? g_fAllowGroup = FALSE : g_fAllowGroup = TRUE);
					g_xClientSocket.SendGroupMode(g_fAllowGroup);
					break;
				case GROUPBTN_CREATEGROUP:
					g_xClientSocket.SendCreateGroup("mandul");
					break;
				case GROUPBTN_ADDMEMBER:
					g_xClientSocket.SendAddGroupMember("mandul");
					break;
				case GROUPBTN_DELMEMBER:
					g_xClientSocket.SendDelGroupMember("mandul");
					break;
			}
		}
	}

	return FALSE;
}

BOOL CGroupWnd::OnLButtonDown(LPCOMMONITEMSET, POINT ptMouse)
{
	return FALSE;
}

VOID CGroupWnd::OnMouseMove(POINT ptMouse)
{
}

#undef GROUPBTN_ALLOWGROUP
#undef GROUPBTN_CREATEGROUP
#undef GROUPBTN_ADDMEMBER
#undef GROUPBTN_DELMEMBER

// GroupPopWnd
/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/






CGroupPopWnd::CGroupPopWnd()
{
	Init();
}

CGroupPopWnd::~CGroupPopWnd()
{
	Destroy();
}

VOID CGroupPopWnd::CreateGroupPopWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove)
{
	CreateGameWnd(nID, pxWndImage, nFrameImgIdx, bCanMove, nStartX, nStartY, nWidth, nHeight);
}




VOID CGroupPopWnd::ShowPanel()
{
	ShowGameWnd();
}

BOOL CGroupPopWnd::OnLButtonUp(LPCOMMONITEMSET, POINT ptMouse)
{
	return FALSE;
}

BOOL CGroupPopWnd::OnLButtonDown(LPCOMMONITEMSET, POINT ptMouse)
{
	return FALSE;
}

VOID CGroupPopWnd::OnMouseMove(POINT ptMouse)
{
}

// ChatPopWnd
/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/






CChatPopWnd::CChatPopWnd()
{
	Init();
}

CChatPopWnd::~CChatPopWnd()
{
	Destroy();
}


VOID CChatPopWnd::Init()
{
	CGameWnd::Init();

	m_nCanScrlCnt		 = 0;
	m_nCurrStartChatLine = 0;
	SetRect(&m_rcChatPopFrame, 0, 0, 0, 0);
	SetRect(&m_rcEditBoxFrame, 0, 0, 0, 0);

	m_xChatPopBtn.Init();
}

VOID CChatPopWnd::Destroy()
{
	m_xstrDividedChat.ClearAllNodes();
	Init();
}


VOID CChatPopWnd::CreateChatPopWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove)
{
	CreateGameWnd(nID, pxWndImage, nFrameImgIdx, bCanMove, nStartX, nStartY, nWidth, nHeight);
	SetRect(&m_rcChatPopFrame, 40, 29, 531, 308);
	SetRect(&m_rcEditBoxFrame, 36, 312, 535, 328);

	m_xChatPopBtn.CreateGameBtn(pxWndImage, 372, 373, nStartX+542, nStartY+353);
}

BOOL CChatPopWnd::MsgAdd(DWORD dwFontColor, DWORD dwFontBackColor, CHAR* szMsg)
{
	CHAR szChatMsg[MAX_PATH];

	if ( szMsg != NULL )
	{
		strcpy(szChatMsg, szMsg);

		if ( szChatMsg[0] != NULL )
		{
			INT nLineCnt;
			CHAR szDivied[MAX_PATH*2];
			CHAR szArg[5][MAX_PATH];

			ZeroMemory(szDivied, MAX_PATH*2);
			ZeroMemory(&szArg[0], MAX_PATH*5);

			g_xMainWnd.StringDivide(m_rcChatPopFrame.right-m_rcChatPopFrame.left, nLineCnt, szChatMsg, szDivied);

			sscanf(szDivied, "%[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c", szArg[0], szArg[1], szArg[2], szArg[3], szArg[4]);
			if ( nLineCnt > 5 )		nLineCnt = 5;

			for ( INT nCnt = 0; nCnt < nLineCnt; nCnt++ )
			{
				if ( m_nCanScrlCnt )
					m_nCurrStartChatLine = m_nCanScrlCnt;

				if ( m_xstrDividedChat.GetCounter() >= _MAX_CHATLINE_POPUP )
				{
					m_nCurrStartChatLine++;
					m_nCanScrlCnt++;
				}
				CHATSTRING stChatStr;
				stChatStr.dwFontColor = dwFontColor;
				stChatStr.dwFontBackColor = dwFontBackColor;
				stChatStr.strChat = szArg[nCnt];
				m_xstrDividedChat.AddNode(stChatStr);
			}
		}
		return TRUE;
	}
	return FALSE;
}

VOID CChatPopWnd::ShowPanel()
{
	ShowGameWnd();

	// 显示聊天列表.
	if ( !m_xstrDividedChat.CheckEmpty() )
	{
		m_xstrDividedChat.MoveCurrentToTop();
		m_xstrDividedChat.MoveNode(m_nCurrStartChatLine);

		INT nLine = 0;
		INT nMaxLine;
		if ( !m_nCurrStartChatLine )
		{
			if ( !m_nCanScrlCnt )
				nMaxLine = m_xstrDividedChat.GetCounter();
			else
				nMaxLine = _MAX_CHATLINE_POPUP;
		}
		else
			nMaxLine = (m_nCurrStartChatLine+_MAX_CHATLINE_POPUP) > m_xstrDividedChat.GetCounter() ?
					   m_xstrDividedChat.GetCounter() : m_nCurrStartChatLine+_MAX_CHATLINE_POPUP;

		for ( INT nCnt = m_nCurrStartChatLine; nCnt < nMaxLine; nCnt++ )
		{
			LPCHATSTRING pstChatString;
			pstChatString = m_xstrDividedChat.GetCurrentData();
			//g_xMainWnd.PutsHan(g_xMainWnd.GetBackBuffer(),
			//				   m_rcWnd.left+m_rcChatPopFrame.left, m_rcWnd.top+m_rcChatPopFrame.top+nLine*14,
			//				   pstChatString->dwFontBackColor, pstChatString->dwFontColor, pstChatString->strChat.begin());

			m_xstrDividedChat.MoveNextNode();
			nLine++;
		}
	}

	m_xChatPopBtn.ShowGameBtn();
}

BOOL CChatPopWnd::OnLButtonUp(LPCOMMONITEMSET, POINT ptMouse)
{
	MoveWindow(g_xChatEditBox.GetSafehWnd(), g_xMainWnd.m_rcWindow.left + m_rcWnd.left + m_rcEditBoxFrame.left,
			   g_xMainWnd.m_rcWindow.top + m_rcWnd.top + m_rcEditBoxFrame.top,
			   m_rcEditBoxFrame.right - m_rcEditBoxFrame.left,
			   m_rcEditBoxFrame.bottom - m_rcEditBoxFrame.top, TRUE);

	if ( m_xChatPopBtn.OnLButtonUp(ptMouse) )		return TRUE;

	return FALSE;
}

BOOL CChatPopWnd::OnLButtonDown(LPCOMMONITEMSET, POINT ptMouse)
{
	if ( m_xChatPopBtn.OnLButtonDown(ptMouse) )		return TRUE;

	return FALSE;
}

VOID CChatPopWnd::OnMouseMove(POINT ptMouse)
{
	m_xChatPopBtn.ChangeRect(m_rcWnd.left+542, m_rcWnd.top+353);
	m_xChatPopBtn.OnMouseMove(ptMouse);
}

VOID CChatPopWnd::OnScrollDown()
{
	if ( m_nCurrStartChatLine > 0 )															m_nCurrStartChatLine--;
}

VOID CChatPopWnd::OnScrollUp()
{
	if ( m_nCurrStartChatLine < m_xstrDividedChat.GetCounter()-_MAX_CHATLINE_POPUP )		m_nCurrStartChatLine++;
}

VOID CChatPopWnd::SetStatusBtnInit()
{
	m_xChatPopBtn.SetBtnState(_BTN_STATE_NORMAL);
}







// NPCWnd
/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/


#define MAX_NPCSTR_HEIGHT		129
#define MAX_SHOW_LINE			6
#define POS_DRAWSTART_Y			40
#define LINE_GAP				5

#define BUILD_GUILD				"@@BUILDGUILDNOW"			// 行会建立
#define GUILD_WAR				"@@GUILDWAR"				// 行会战申请
#define WITHDRAWAL_CASTLE		"@@WITHDRAWAL"				// 取出钱款.
#define RECEIPT_CASTLE			"@@RECEIPTS"				// 寄存金钱.

#define SCRIPT_GENTEXT		1
#define SCRIPT_BUTTON		2
#define SCRIPT_RETURN		3
#define SCRIPT_TAG			4

COLORREF ScriptTxtColor[] = {	RGB(0, 0, 0),		// 0 : Black
								RGB(255, 0, 0),		// 1 : Red
								RGB(0, 128, 0),		// 2 : Green
								RGB(128, 128, 0), 	// 3 : Olive
								RGB(128, 128, 128), // 4 : Gray
								RGB(128, 0, 0),		// 5 : Marron
								RGB(0, 128, 128),	// 6 : Teal
								RGB(0, 0, 128),		// 7 : Navy
								RGB(192, 192, 192),	// 8 : Silver
								RGB(128, 0, 128),	// 9 : Purple
								RGB(0, 255, 0),		// 10 : Lime
								RGB(0, 0, 255),		// 11 : Blue
								RGB(255, 255, 255),	// 12 : White
								RGB(255, 0, 255),	// 13 : Fuchsia
								RGB(0, 255, 255),	// 14 : Aqua
								RGB(255, 255, 0),	// 15 : Yellow
								RGB(1, 1, 1),		// 16 : Default
							};

class CScriptLine
{
public:
	BYTE	m_btScriptType;
	char	*m_pszScriptText;
	char	*m_pszScriptCommand;
	RECT	m_Rect;
	bool	m_fIsSelected;
	bool	m_fIsFocused;

public:
	CScriptLine()
	{
		m_pszScriptText		= NULL;
		m_pszScriptCommand	= NULL;

		m_fIsFocused		= FALSE;
		m_fIsSelected		= FALSE;
	}
	~CScriptLine()
	{
//		if (m_pszScriptText) delete [] m_pszScriptText;
//		if (m_pszScriptCommand) delete [] m_pszScriptCommand;
	}
};

CPDLList<CScriptLine>	ScriptList;
char					szQuestNPCName[64];

char *CheckText(char *pszScript)
{
	if (pszScript)
	{
		char	*pszText = pszScript;

		while (*pszText)
		{
			if (*pszText == '\\' || *pszText == '<' || *pszText == '{')
				return pszText;

			pszText++;
		}
	}

	return NULL;
}

char *AddScriptLine(char *pszLine, int nLineLen, int nType)
{
	CScriptLine* pScriptLine = new CScriptLine;

	if (pScriptLine)
	{
		pScriptLine->m_btScriptType = nType;

		switch (nType)
		{
			case SCRIPT_GENTEXT:
			{
				pScriptLine->m_pszScriptText	= new char[nLineLen];

				memcpy(pScriptLine->m_pszScriptText, pszLine, nLineLen);
				pScriptLine->m_pszScriptText[nLineLen] = '\0';

				ScriptList.AddNode(pScriptLine);

				return NULL;
			}
			case SCRIPT_RETURN:
			{
				ScriptList.AddNode(pScriptLine);

				return NULL;
			}
			case SCRIPT_BUTTON:
			{
				char *pszEndCmd;
				char *pszDevide;

				if (pszEndCmd = (char *)strchr(pszLine, '>'))
				{
					*pszEndCmd = '\0';

					if (pszDevide = (char *)strchr(pszLine, '/'))
					{
						*pszDevide = '\0';

						pScriptLine->m_pszScriptCommand = new char[pszEndCmd - pszDevide];
						pScriptLine->m_pszScriptText	= new char[pszDevide - pszLine];

						strcpy(pScriptLine->m_pszScriptCommand, ++pszDevide);
						strcpy(pScriptLine->m_pszScriptText, ++pszLine);

						ScriptList.AddNode(pScriptLine);
					}

					return ++pszEndCmd;
				}

				return NULL;
			}
			case SCRIPT_TAG:
			{
				char *pszEndCmd;
				char *pszDevide;

				if (pszEndCmd = (char *)strchr(pszLine, '}'))
				{
					*pszEndCmd = '\0';

					if (pszDevide = (char *)strchr(pszLine, '/'))
					{
						*pszDevide = '\0';

						pScriptLine->m_pszScriptCommand = new char[pszEndCmd - pszDevide];
						pScriptLine->m_pszScriptText	= new char[pszDevide - pszLine];

						strcpy(pScriptLine->m_pszScriptCommand, ++pszDevide);
						strcpy(pScriptLine->m_pszScriptText, ++pszLine);

						ScriptList.AddNode(pScriptLine);
					}

					return ++pszEndCmd;
				}

				return NULL;
			}
		}
	}

	delete pScriptLine;

	return NULL;
}

void DevideScript(char *pszScript)
{
	char *pszText = pszScript;
	char *pszPaser;
	int  nLineLen = 0;

	while (*pszText)
	{
		if (pszPaser = CheckText(pszText))
		{
			nLineLen = pszPaser - pszText;

			switch (*pszPaser)
			{
				case '\\':
				{
					if (nLineLen >= 1)
						AddScriptLine(pszText, nLineLen, SCRIPT_GENTEXT);

					AddScriptLine(NULL, 0, SCRIPT_RETURN);

					pszText = ++pszPaser;

					break;
				}
				case '<':
				{
					if (nLineLen >= 1)
						AddScriptLine(pszText, nLineLen, SCRIPT_GENTEXT);

					pszText = AddScriptLine(pszPaser, nLineLen, SCRIPT_BUTTON);

					break;
				}
				case '{':
				{
					if (nLineLen >= 1)
						AddScriptLine(pszText, nLineLen, SCRIPT_GENTEXT);

					pszText = AddScriptLine(pszPaser, nLineLen, SCRIPT_TAG);

					break;
				}
			}
		}
	}
}

CNPCWnd::CNPCWnd()
{
	m_nStartLine	= 0;
	m_nMaxLine		= MAX_SHOW_LINE;
}

CNPCWnd::~CNPCWnd()
{
	ResetDialog();
}


VOID CNPCWnd::CreateNPCChatWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove)
{
	CreateGameWnd(nID, pxWndImage, nFrameImgIdx, bCanMove, nStartX, nStartY, nWidth, nHeight);
	m_xMsgBox.Load(&(g_xGameProc.m_xInterface.m_xInterImgEx));
}

VOID CNPCWnd::ShowPanel()
{
	SIZE			tSize;
	RECT			tRect;
	CScriptLine*	pScriptLine;
	int				nLineCnt = 0;
	int				nLine = 0;
	int				POS_DRAWSTART_X		= 60;
	int				MAX_NPCSTR_WIDTH	= 430;

	tRect = GetGameWndRect();				// Get Window Rect

	ShowGameWnd();

	ScriptList.MoveCurrentToTop();

	COLORREF TextColor = ScriptTxtColor[12];

	for (int i = 0; i < ScriptList.GetCounter(); i++)
	{
		pScriptLine = ScriptList.GetCurrentData();

		switch (pScriptLine->m_btScriptType)
		{
			case SCRIPT_GENTEXT:
			{
				char	szTempText[1024];

				tSize = g_xMainWnd.GetStrLength(NULL, NULL, pScriptLine->m_pszScriptText);

				if (nLine + tSize.cx > MAX_NPCSTR_WIDTH)
				{
					int nLen	= strlen(pScriptLine->m_pszScriptText);
					int nCount	= 0;

					for (int i = 0; i < nLen; i++)
					{
						if (pScriptLine->m_pszScriptText[i] < 0)
						{
							szTempText[nCount++]	=  pScriptLine->m_pszScriptText[i++];
							szTempText[nCount]		=  pScriptLine->m_pszScriptText[i];
							szTempText[nCount + 1]	= '\0';
						}
						else
						{
							szTempText[nCount]		=  pScriptLine->m_pszScriptText[i];
							szTempText[nCount + 1]	= '\0';
						}

						tSize = g_xMainWnd.GetStrLength(NULL, NULL, szTempText);

						if (nLine + tSize.cx >= MAX_NPCSTR_WIDTH)
						{
							if (nLineCnt >= m_nStartLine && nLineCnt <= m_nStartLine + MAX_SHOW_LINE)
								g_xMainWnd.PutsHan(NULL, tRect.left + POS_DRAWSTART_X + nLine, tRect.top + ((nLineCnt - m_nStartLine) * (tSize.cy + LINE_GAP)) + POS_DRAWSTART_Y,
													TextColor, RGB(0, 0, 0), szTempText);

							nLineCnt++;
							nLine	= 0;
							nCount	= 0;
						}
						else
							nCount++;
					}

					if (nCount)
					{
						if (nLineCnt >= m_nStartLine && nLineCnt <= m_nStartLine + MAX_SHOW_LINE)
							g_xMainWnd.PutsHan(NULL, tRect.left + POS_DRAWSTART_X + nLine, tRect.top + ((nLineCnt - m_nStartLine) * (tSize.cy + LINE_GAP)) + POS_DRAWSTART_Y,
												TextColor, RGB(0, 0, 0), szTempText);
						nLine += tSize.cx;
					}
				}
				else
				{
					if (nLineCnt >= m_nStartLine && nLineCnt <= m_nStartLine + MAX_SHOW_LINE)
						g_xMainWnd.PutsHan(NULL, tRect.left + POS_DRAWSTART_X + nLine, tRect.top + ((nLineCnt - m_nStartLine) * (tSize.cy + LINE_GAP)) + POS_DRAWSTART_Y,
											TextColor, RGB(0, 0, 0), pScriptLine->m_pszScriptText);

					nLine += tSize.cx;
				}

				break;
			}
			case SCRIPT_RETURN:
				nLineCnt++;
				nLine = 0;
				break;
			case SCRIPT_BUTTON:
			{
				char		szTempText[1024];

				HFONT		hFont = NULL;
				COLORREF	color = RGB(255, 255, 0);

				if (pScriptLine->m_fIsFocused)
					hFont = g_xMainWnd.CreateGameFont("Arial", 9, 0, FW_NORMAL, FALSE, TRUE);

				if (pScriptLine->m_fIsSelected)
					color = RGB(255, 0, 0);

				tSize = g_xMainWnd.GetStrLength(NULL, NULL, pScriptLine->m_pszScriptText);

				if (nLine + tSize.cx > MAX_NPCSTR_WIDTH)
				{
					int nLen	= strlen(pScriptLine->m_pszScriptText);
					int nCount	= 0;
					int nRow	= 0;

					pScriptLine->m_Rect.left	= tRect.left + POS_DRAWSTART_X + nLine;
					pScriptLine->m_Rect.top		= tRect.top + (nLineCnt * (tSize.cy + LINE_GAP)) + POS_DRAWSTART_Y;
					pScriptLine->m_Rect.right	= pScriptLine->m_Rect.left + MAX_NPCSTR_WIDTH;

					for (int i = 0; i < nLen; i++)
					{
						if (pScriptLine->m_pszScriptText[i] < 0)
						{
							szTempText[nCount++]	=  pScriptLine->m_pszScriptText[i++];
							szTempText[nCount]		=  pScriptLine->m_pszScriptText[i];
							szTempText[nCount + 1]	= '\0';
						}
						else
						{
							szTempText[nCount]		=  pScriptLine->m_pszScriptText[i];
							szTempText[nCount + 1]	= '\0';
						}

						tSize = g_xMainWnd.GetStrLength(NULL, NULL, szTempText);

						if (nLine + tSize.cx >= MAX_NPCSTR_WIDTH)
						{
							if (nLineCnt >= m_nStartLine && nLineCnt <= m_nStartLine + MAX_SHOW_LINE)
								g_xMainWnd.PutsHan(NULL, tRect.left + POS_DRAWSTART_X + nLine, tRect.top + ((nLineCnt - m_nStartLine) * (tSize.cy + LINE_GAP)) + POS_DRAWSTART_Y,
													color, RGB(0, 0, 0), szTempText, hFont);

							nLineCnt++;
							nRow++;
							nLine	= 0;
							nCount	= 0;
						}
						else
							nCount++;
					}

					if (nCount)
					{
						if (nLineCnt >= m_nStartLine && nLineCnt <= m_nStartLine + MAX_SHOW_LINE)
							g_xMainWnd.PutsHan(NULL, tRect.left + POS_DRAWSTART_X + nLine, tRect.top + ((nLineCnt - m_nStartLine) * (tSize.cy + LINE_GAP)) + POS_DRAWSTART_Y,
												color, RGB(0, 0, 0), szTempText, hFont);

						nRow++;
						nLine += tSize.cx;
					}

					pScriptLine->m_Rect.bottom	= pScriptLine->m_Rect.top + (nRow * (tSize.cy + LINE_GAP));

				}
				else
				{
					pScriptLine->m_Rect.left	= tRect.left + POS_DRAWSTART_X + nLine;
					pScriptLine->m_Rect.top		= tRect.top + ((nLineCnt - m_nStartLine) * (tSize.cy + LINE_GAP)) + POS_DRAWSTART_Y;
					pScriptLine->m_Rect.right	= pScriptLine->m_Rect.left + tSize.cx;
					pScriptLine->m_Rect.bottom	= pScriptLine->m_Rect.top + tSize.cy;

					if (nLineCnt >= m_nStartLine && nLineCnt <= m_nStartLine + MAX_SHOW_LINE)
						g_xMainWnd.PutsHan(NULL, pScriptLine->m_Rect.left, pScriptLine->m_Rect.top,
											color, RGB(0, 0, 0), pScriptLine->m_pszScriptText, hFont);

					nLine += tSize.cx;
				}

				DeleteObject(hFont);

				break;
			}
			case SCRIPT_TAG:
			{
				if (strcmp(pScriptLine->m_pszScriptText, "FCOLOR") == 0)
				{
					int nColor = atoi(pScriptLine->m_pszScriptCommand);

					TextColor = ScriptTxtColor[nColor];
				}
				else if (strcmp(pScriptLine->m_pszScriptText, "NPCIMG") == 0)
				{
					if (!m_xNPCImage.m_pbCurrImage)
					{
					//	m_xNPCImage.DeleteImage();

						m_xNPCImage.NewLoad(".\\Data\\NPCFace.WIL", FALSE, TRUE);
					}

					if (m_xNPCImage.NewSetIndex(1))
						g_xMainWnd.DrawWithImageForComp(40, 30, m_xNPCImage.m_lpstNewCurrWilImageInfo->shWidth,
																m_xNPCImage.m_lpstNewCurrWilImageInfo->shHeight,
																(WORD *)m_xNPCImage.m_pbCurrImage);

					POS_DRAWSTART_X		= 150;
					MAX_NPCSTR_WIDTH	= 365;
				}

				break;
			}
		}

		ScriptList.MoveNextNode();
	}

	m_nMaxLine = nLineCnt;
}

BOOL CNPCWnd::OnLButtonDown(LPCOMMONITEMSET, POINT ptMouse)
{
	CScriptLine*	pScriptLine;

	ScriptList.MoveCurrentToTop();

	for (int i = 0; i < ScriptList.GetCounter(); i++)
	{
		pScriptLine = ScriptList.GetCurrentData();

		if (pScriptLine->m_btScriptType == SCRIPT_BUTTON)
		{
			if (PtInRect(&pScriptLine->m_Rect, ptMouse))
				pScriptLine->m_fIsSelected = TRUE;
			else
				pScriptLine->m_fIsSelected = FALSE;
		}

		ScriptList.MoveNextNode();
	}

	return FALSE;
}

BOOL CNPCWnd::OnLButtonUp(LPCOMMONITEMSET, POINT ptMouse)
{
	CScriptLine*	pScriptLine;

	ScriptList.MoveCurrentToTop();

	for (int i = 0; i < ScriptList.GetCounter(); i++)
	{
		pScriptLine = ScriptList.GetCurrentData();

		if (pScriptLine->m_btScriptType == SCRIPT_BUTTON)
		{
			if (PtInRect(&pScriptLine->m_Rect, ptMouse))
			{
				if (pScriptLine->m_pszScriptCommand[0] == '@' && pScriptLine->m_pszScriptCommand[1] == '@')
				{
					if (strcmp(pScriptLine->m_pszScriptCommand, "@@buildguildnow") == 0)
					{
						char szBuildGuild[128];

						sprintf(szBuildGuild, "%s%c%s", pScriptLine->m_pszScriptCommand, 13, "Gadget Detective Clan");

						g_xClientSocket.SendNPCMessage(CM_MERCHANTDLGSELECT,(DWORD)m_nNpcId, 0, szBuildGuild);
					}
				}
				else
					g_xClientSocket.SendNPCMessage(CM_MERCHANTDLGSELECT,(DWORD)m_nNpcId, 0, pScriptLine->m_pszScriptCommand);
			}
		}

		pScriptLine->m_fIsSelected = FALSE;

		ScriptList.MoveNextNode();
	}

	return FALSE;
}

void CNPCWnd::OnMouseMove(POINT ptMouse)
{
	CScriptLine*	pScriptLine;

	ScriptList.MoveCurrentToTop();

	for (int i = 0; i < ScriptList.GetCounter(); i++)
	{
		pScriptLine = ScriptList.GetCurrentData();

		if (pScriptLine->m_btScriptType == SCRIPT_BUTTON)
		{
			if (PtInRect(&pScriptLine->m_Rect, ptMouse))
				pScriptLine->m_fIsFocused	= TRUE;
			else
				pScriptLine->m_fIsFocused	= FALSE;

		}

		ScriptList.MoveNextNode();
	}
}

void CNPCWnd::ResetDialog()
{
	CScriptLine*	pScriptLine;

	ScriptList.MoveCurrentToTop();

	for (int i = 0; i < ScriptList.GetCounter();)
	{
		pScriptLine = ScriptList.GetCurrentData();

		if (pScriptLine)
			delete pScriptLine;

		ScriptList.DeleteCurrentNodeEx();
	}

	m_nStartLine	= 0;
	m_nMaxLine		= MAX_SHOW_LINE;
}

VOID CNPCWnd::OnScrollDown()
{
	if (m_nMaxLine > MAX_SHOW_LINE)
	{
		if (m_nStartLine <= 0)
			m_nStartLine = 0;
		else
			m_nStartLine--;
	}
}

VOID CNPCWnd::OnScrollUp()
{
	if (m_nMaxLine > MAX_SHOW_LINE)
	{
		if (m_nStartLine >= m_nMaxLine)
			m_nStartLine = m_nMaxLine;
		else
			m_nStartLine++;
	}
}
























//////////////////////////////////////////////////////////////////

BOOL	CNPCWnd::ClickCommandProcess(CMTxtBtn* ClickedBtn)		// 综合下面这些内容
{
/*	m_pClickedTxtBtn = ClickedBtn;
	if(ClickedBtn!=NULL)
	{
		if(strcmp(m_xTxtAnalysis.UpperCase(ClickedBtn->Param),BUILD_GUILD)==0)				// 行会建立处理
		{
			CHAR	szMsg[MAX_PATH];
			HINSTANCE hLib;
			hLib = LoadLibrary("Ats.dll"); // Load Dll
			LoadString(hLib,ASK_MAKE_BIG_GUILD,szMsg,MAX_PATH);
			FreeLibrary(hLib);
			if(!m_xMsgBox.IsActive())	SetWndRectExtend();
			m_xMsgBox.ShowMessageBox(szMsg,MSG_EDITEXIST|MSG_BTN_OK);	// OK
			SetNpcEditBoxPos();
			// 打开行会建立窗口并输入行会名称.
			m_nNpcMsgState = 1;
			return TRUE;
		}
		else if(strcmp(m_xTxtAnalysis.UpperCase(ClickedBtn->Param),GUILD_WAR)==0)			// 行会战争
		{
			CHAR	szMsg[MAX_PATH];
			HINSTANCE hLib;
			hLib = LoadLibrary("Ats.dll"); // Load Dll
			LoadString(hLib,ASK_OTHERSNAME,szMsg,MAX_PATH);
			FreeLibrary(hLib);
			if(!m_xMsgBox.IsActive())	SetWndRectExtend();
			m_xMsgBox.ShowMessageBox(szMsg,MSG_EDITEXIST|MSG_BTN_OK);	// OK
			SetNpcEditBoxPos();
			// 输入要交战的行会.
			m_nNpcMsgState = 2;
			return TRUE;
		}
		else if(strcmp(m_xTxtAnalysis.UpperCase(ClickedBtn->Param),WITHDRAWAL_CASTLE)==0)	// 从城金库中取钱
		{
			CHAR	szMsg[MAX_PATH];
			HINSTANCE hLib;
			hLib = LoadLibrary("Ats.dll"); // Load Dll
			LoadString(hLib,ASK_WITHDRAWAL,szMsg,MAX_PATH);
			FreeLibrary(hLib);
			if(!m_xMsgBox.IsActive())	SetWndRectExtend();
			m_xMsgBox.ShowMessageBox(szMsg,MSG_EDITEXIST|MSG_BTN_OK);	// OK
			SetNpcEditBoxPos();
			// 打开城金库窗口并输入要取出的金额.
			m_nNpcMsgState = 3;
			return TRUE;
		}
		else if(strcmp(m_xTxtAnalysis.UpperCase(ClickedBtn->Param),RECEIPT_CASTLE)==0)		// 向城金库中存钱
		{
			CHAR	szMsg[MAX_PATH];
			HINSTANCE hLib;
			hLib = LoadLibrary("Ats.dll"); // Load Dll
			LoadString(hLib,ASK_RECEIPT,szMsg,MAX_PATH);
			FreeLibrary(hLib);
			if(!m_xMsgBox.IsActive())	SetWndRectExtend();
			m_xMsgBox.ShowMessageBox(szMsg,MSG_EDITEXIST|MSG_BTN_OK);	// OK
			SetNpcEditBoxPos();
			// 打开城金库窗口并输入要存入的金额.
			m_nNpcMsgState = 4;
			return TRUE;
		}
		else
		{
			m_nNpcMsgState = 0;
			m_pClickedTxtBtn = NULL;
			SendCommandToSrv(ClickedBtn->Param);
		}
	}
//		不进行任何处理 */
	return FALSE;
}

BOOL CNPCWnd::SendCommandToSrv(CHAR* szMsg)
{
	// 默认消息处理
	g_xClientSocket.SendNPCMessage(CM_MERCHANTDLGSELECT,(DWORD)m_nNpcId,0,szMsg);
	return FALSE;
}

VOID CNPCWnd::SetWndRectExtend(VOID)
{
//	m_rectExtended = m_rcWnd;
//	m_rcWnd.left = 0;
//	m_rcWnd.top = 0;
//	m_rcWnd.right = 800;
//	m_rcWnd.bottom = 560;
}

VOID CNPCWnd::SetWndRectReduce(VOID)
{
//	m_rcWnd = m_rectExtended;
}

VOID CNPCWnd::SetNpcEditBoxPos(VOID)
{
	// 与 Edit Box 相关
//	m_rcEditBoxFrame.left = 247;
//	m_rcEditBoxFrame.top = 256;
//	m_rcEditBoxFrame.right = 467;
//	m_rcEditBoxFrame.bottom = 272;
//	MoveWindow(g_xChatEditBox.GetSafehWnd(), g_xMainWnd.m_rcWindow.left + m_rcWnd.left + m_rcEditBoxFrame.left,
//		g_xMainWnd.m_rcWindow.top + m_rcWnd.top + m_rcEditBoxFrame.top,
//		m_rcEditBoxFrame.right - m_rcEditBoxFrame.left,
//		m_rcEditBoxFrame.bottom - m_rcEditBoxFrame.top, TRUE);
//	SetWindowText(g_xChatEditBox.GetSafehWnd(),NULL);
//	g_xChatEditBox.SetLimitText(30);
//	SetFocus(g_xChatEditBox.GetSafehWnd());
//	ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_SHOW);
}

VOID CNPCWnd::OnEnterKeyDown(VOID)
{
	CHAR	szTxt[MAX_PATH];
	ZeroMemory(szTxt,MAX_PATH);
	if(m_xMsgBox.IsActive())
	{
		SetWndRectReduce();
		GetWindowText(g_xChatEditBox.GetSafehWnd(),szTxt,MAX_PATH);
		if(szTxt[0]==NULL)
		{
			strncpy(szTxt,g_xChatEditBox.m_szInputMsg,strlen(g_xChatEditBox.m_szInputMsg));
		}
		// For Editbox to Chatting
		SetFocus(g_xMainWnd.GetSafehWnd());
		ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
		g_xChatEditBox.SetLimitText(90);// 将聊天窗口改为 Limit length。
		MoveWindow(g_xChatEditBox.GetSafehWnd(), g_xMainWnd.m_rcWindow.left+_INPUT_EDITWND_XPOS,
			g_xMainWnd.m_rcWindow.top+_INPUT_EDITWND_YPOS, _INPUT_EDITWND_WIDTH, _INPUT_EDITWND_HEIGHT, TRUE);
		switch(m_nNpcMsgState)
		{
		case 1:
		case 2:
			// 创建行会
			// 行会战申请
			{
				CHAR szMsg[MAX_PATH*2];
				strcpy(szMsg,m_pClickedTxtBtn->Param);
				strcat(szMsg,"/\0");
				strcat(szMsg,szTxt);
				SendCommandToSrv(szMsg);
			break;
			}
		case 3:
		case 4:
			{
			// 查找圣紫金
			// 圣子金受击
			break;
			}
		}
		g_xChatEditBox.m_szInputMsg[0] = NULL;
		SetWindowText(g_xChatEditBox.GetSafehWnd(),NULL);
		m_xMsgBox.HideMessageBox();
	}
}

BOOL CNPCWnd::OnMsgInputted(VOID)
{
	if(!m_xMsgBox.IsActive())
		return FALSE;
	OnEnterKeyDown();
	return TRUE;
}

#undef MAX_NPCSTR_HEIGHT
#undef MAX_SHOW_LINE
#undef POS_DRAWSTART_Y
#undef LINE_GAP
#undef BUILD_GUILD
#undef GUILD_WAR
#undef WITHDRAWAL_CASTLE
#undef RECEIPT_CASTLE
#undef SCRIPT_GENTEXT
#undef SCRIPT_BUTTON
#undef SCRIPT_RETURN
#undef SCRIPT_TAG

// StoreWnd
/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/


/////////////////////////////////////////////////////
#define MAX_KINDS_LINE			11
#define MAX_ITEMLIST_LINE		5
#define LINE_GAP				5
/////////////////////////////////////////////////////
#define POS_KINDS_X				44
#define POS_KINDS_Y				43
#define POS_KINDSTITLE_X		POS_KINDS_X+12
#define POS_KINDSTITLE_Y		28
#define POS_ITEMSTITLE_X		POS_ITEMLIST_LINE_X+15
#define POS_ITEMSTITLE_Y		POS_ITEMLIST_LINE_Y

#define POS_ITEMLIST_LINE_X		159
#define POS_ITEMLIST_LINE_Y		90

#define POS_ITEMNAME_X			190
#define POS_ITEMNAME_Y			28
#define POS_ITEMIMAGE_X			188
#define	POS_ITEMIMAGE_Y			56

#define LEFT_KIND_RECT			42
#define RIGHT_KIND_RECT			130
#define TOP_KIND_RECT			25
#define BOTTOM_KIND_RECT		228

#define LEFT_ITEM_RECT			157
#define RIGHT_ITEM_RECT			309
#define TOP_ITEM_RECT			103
#define BOTTOM_ITEM_RECT		199

#define POS_X				84			// ex (钱:POS_X,POS_Y)(GOLD:POS__X,POS_Y)
#define POS_Y				246			// ex (重量:POS_X,POS__Y)(背包重量:POS__X,POS__Y)
#define POS__Y				270
#define POS__X				246

#define MAX_MSG_LENGTH			20480			// (20Kb)


CStoreWnd::CStoreWnd()
{
	POINT	Temp;

	m_nIsReadyToDisplay = 0;
	m_nCurrentItemTop = 0;
	m_nCurrentKindTop = 0;
	m_nMaxItemLine = 0;
	m_nMaxKindLine = 0;

	Temp.x = POS_KINDS_X;
	Temp.y = POS_KINDS_Y;


	m_xKindBtn.InitTxtBtn(MAX_KINDS_LINE,Temp,LINE_GAP);

	Temp.x = POS_ITEMLIST_LINE_X;
	Temp.y = POS_ITEMLIST_LINE_Y + 17;

//	tRect.top+POS_ITEMLIST_LINE_Y + 17 + (xtBtn->nLinePos-m_nCurrentItemTop)*(tSize.cy+LINE_GAP);

	m_xItemLstBtn.InitTxtBtn(MAX_ITEMLIST_LINE,Temp,LINE_GAP);
	m_bSelectedKind=FALSE;
	m_bSelectedItem=FALSE;

	m_xKindRect.left = LEFT_KIND_RECT;
	m_xKindRect.top	= TOP_KIND_RECT;
	m_xKindRect.right = RIGHT_KIND_RECT;
	m_xKindRect.bottom = BOTTOM_KIND_RECT;

	m_xItemRect.left = LEFT_ITEM_RECT;
	m_xItemRect.top	= TOP_ITEM_RECT;
	m_xItemRect.right = RIGHT_ITEM_RECT;
	m_xItemRect.bottom = BOTTOM_ITEM_RECT;
}

CStoreWnd::~CStoreWnd()
{
}

VOID CStoreWnd::CreateStoreWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove)
{
	CreateGameWnd(nID, pxWndImage, nFrameImgIdx, bCanMove, nStartX, nStartY, nWidth, nHeight);
}


VOID CStoreWnd::ShowPanel()
{
	RECT tRect;
	CMTxtBtn* xtBtn;
	ShowGameWnd();
	OnUpdateQueue();
	if(m_nIsReadyToDisplay>0)
	{
		tRect = GetGameWndRect();
		g_xMainWnd.PutsHan(NULL,tRect.left + POS_KINDSTITLE_X,tRect.top + POS_KINDSTITLE_Y,RGB(255,255,255),RGB(0,0,0)," I T E M  N A M E");
//		if(stdMod=1)
			g_xMainWnd.PutsHan(NULL,tRect.left + POS_ITEMSTITLE_X,tRect.top + POS_ITEMSTITLE_Y,RGB(255,255,255),RGB(0,0,0)," T Y P E            P R I C E");
		if(m_bSelectedKind)
		{// 选中 Kinds 时
			g_xMainWnd.PutsHan(NULL,tRect.left + POS_ITEMNAME_X+20,tRect.top + POS_ITEMNAME_Y + 10 ,RGB(255,255,255),RGB(0,0,0),"Crescent Blade");

			g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewSetIndex(1); // 填入该物品的 Looks
			g_xMainWnd.DrawWithImageForCompClipRgn(
				tRect.left + POS_ITEMIMAGE_X - (g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].m_lpstNewCurrWilImageInfo->shWidth / 2),tRect.top + POS_ITEMIMAGE_Y - (g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].m_lpstNewCurrWilImageInfo->shHeight / 2)
				,g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].m_lpstNewCurrWilImageInfo->shWidth,g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].m_lpstNewCurrWilImageInfo->shHeight,
				(WORD*)g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].m_pbCurrImage,800,600);
		}

		if(m_nIsReadyToDisplay&&1)
		{	// Kinds List 输出
			m_xKindBtn.m_pTxtBtn.MoveCurrentToTop();
			for(int i = 0 ; i <m_xKindBtn.GetButtonCount(); i++)
			{
				xtBtn = m_xKindBtn.m_pTxtBtn.GetCurrentData();
				if((xtBtn->nLinePos>=m_nCurrentKindTop)&&(xtBtn->nLinePos<(m_nCurrentKindTop+MAX_KINDS_LINE)))
				{
					SIZE tSize;
					tSize = g_xMainWnd.GetStrLength(NULL,NULL,xtBtn->Txt);
					g_xMainWnd.PutsHan(NULL,tRect.left + xtBtn->xRect.left +POS_KINDS_X,tRect.top + (xtBtn->nLinePos-m_nCurrentKindTop)*(tSize.cy+LINE_GAP) +POS_KINDS_Y,(!xtBtn->bIsClicked?xtBtn->nColor:xtBtn->cColor),RGB(0,0,0),xtBtn->Txt);
				}
				m_xKindBtn.m_pTxtBtn.MoveNextNode();
			}
		}
		if((m_nIsReadyToDisplay>>1)&&1)
		{	// Item List 输出
//			g_xMainWnd.PutsHan(NULL,tRect.left+POS_ITEMLIST_LINE_X,tRect.top+POS_ITEMLIST_LINE_Y+17,RGB(255,255,0),RGB(0,0,0),"123456789012345678901234567890");
			m_xItemLstBtn.m_pTxtBtn.MoveCurrentToTop();
			for(int i = 0 ; i <m_xItemLstBtn.GetButtonCount(); i++)
			{
				xtBtn = m_xItemLstBtn.m_pTxtBtn.GetCurrentData();
				if((xtBtn->nLinePos>=m_nCurrentItemTop)&&(xtBtn->nLinePos<(m_nCurrentItemTop+MAX_KINDS_LINE)))
				{
					SIZE tSize;
					tSize = g_xMainWnd.GetStrLength(NULL,NULL,xtBtn->Txt);
					g_xMainWnd.PutsHan(NULL,tRect.left+POS_ITEMLIST_LINE_X + xtBtn->xRect.left +80,tRect.top+POS_ITEMLIST_LINE_Y + 17 + (xtBtn->nLinePos-m_nCurrentItemTop)*(tSize.cy+LINE_GAP) ,(!xtBtn->bIsClicked?xtBtn->nColor:xtBtn->cColor),RGB(0,0,0),xtBtn->Txt);
					g_xMainWnd.PutsHan(NULL,tRect.left+POS_ITEMLIST_LINE_X + xtBtn->xRect.left ,tRect.top+POS_ITEMLIST_LINE_Y + 17 + (xtBtn->nLinePos-m_nCurrentItemTop)*(tSize.cy+LINE_GAP) ,(!xtBtn->bIsClicked?xtBtn->nColor:xtBtn->cColor),RGB(0,0,0),xtBtn->sTxt);
				}
				m_xItemLstBtn.m_pTxtBtn.MoveNextNode();
			}
		}
	}
}


BOOL CStoreWnd::OnLButtonUp(LPCOMMONITEMSET, POINT ptMouse)
{
//	BOOL	bResult;
	RECT tRect;
	POINT tPos;
//	CHAR*	szTest;
	CMTxtBtn* tBtn;
	tRect = GetGameWndRect();			// Get Window Rect
	tPos.x = tRect.left;
	tPos.y = tRect.top;
	//bResult = ClickCommandProcess(
	if(IsInRect(ptMouse,m_xKindRect))
	{	// 列表 List
		tBtn = m_xKindBtn.ClickCheck(tPos,ptMouse,m_nCurrentKindTop);
		if(tBtn!=NULL)
		{
			// 确认被按下的按钮是否有菜单.
			// 没有菜单时就停在选择处，有菜单时
			// 请求下级菜单。
			tBtn = m_xKindBtn.ClickCheck(tPos,ptMouse,m_nCurrentKindTop);
			if(tBtn->nColor==RGB(255,255,0))
			{
				// 有菜单吗?
				// 不可欺诈
				SendDeTailGoodsList(0,tBtn->Param);
			}
			else
			{
				// 没有菜单吗?
				// 那就直接选择。可以作弊。
			}
		}
		else
		{
			m_nIsReadyToDisplay = 1;
			m_bSelectedKind= FALSE;
		}
	}
	if(IsInRect(ptMouse,m_xItemRect))
	{	// 项目  List
		tBtn = NULL;
		tBtn = m_xItemLstBtn.ClickCheck(tPos,ptMouse,m_nCurrentItemTop);
		if(tBtn!=NULL)
		{
			// 可以购买
			m_bSelectedItem = TRUE;
			//临时
			SendButThisITme(tBtn);
		}
		else
		{
			m_bSelectedItem = FALSE;
		}
	}
	return FALSE;
}

VOID CStoreWnd::OnMouseMove(POINT ptMouse)
{

}

BOOL CStoreWnd::OnLButtonDown(LPCOMMONITEMSET, POINT ptMouse)
{
	RECT tRect;
	POINT tPos;
	tRect = GetGameWndRect();			// Get Window Rect
	tPos.x = tRect.left;
	tPos.y = tRect.top;

	if(IsInRect(ptMouse,m_xKindRect))
	{
		m_xKindBtn.ClickFreeAllBtn();
		m_xKindBtn.ClickCheck(tPos,ptMouse,m_nCurrentKindTop);
	}
	if(IsInRect(ptMouse,m_xItemRect))
	{
		m_xItemLstBtn.ClickFreeAllBtn();
	}
	return FALSE;
}

BOOL CStoreWnd::SetBuyKinds(CHAR* Msg,INT Count)   // Eatch Button Divider '\n
{
	CNPCTxtAnalysis	tTxtAnalysis;				// 只是为了使用 GetValidStr 函数..
	CHAR*	szParam;
	CHAR*	pszStr = new CHAR [MAX_MSG_LENGTH];


	if(m_nIsReadyToDisplay&&1)
	{
		m_xKindBtn.ReleaseButtons();
		m_bSelectedKind = FALSE;
		// 清除已保存的按键
		m_nIsReadyToDisplay= 0;	// Kinds List 设置已解除的通知
	}

	ZeroMemory(pszStr,MAX_MSG_LENGTH);
	strcpy(pszStr,Msg);

	m_nMaxKindLine = Count;

	for(int i = 0 ; i < Count ; i ++)
	{
		CHAR* szTemp;
		CHAR* szGName = new CHAR[MAX_PATH];
		CHAR* szGSubMenu = new CHAR[MAX_PATH];
		CHAR* szGPrice = new CHAR[MAX_PATH];
		CHAR* szGStock = new CHAR[MAX_PATH];
		INT   nSubMenuCount;

		szTemp = tTxtAnalysis.GetValidStr(pszStr,szGName,'/');
		strcpy(pszStr,szTemp);
		szTemp = tTxtAnalysis.GetValidStr(pszStr,szGSubMenu,'/');
		strcpy(pszStr,szTemp);
		nSubMenuCount = atoi(szGSubMenu);
		szTemp = tTxtAnalysis.GetValidStr(pszStr,szGPrice,'/');
		strcpy(pszStr,szTemp);
		szTemp = tTxtAnalysis.GetValidStr(pszStr,szGStock,'/');
		strcpy(pszStr,szTemp);

		szTemp = new CHAR[15];
		szTemp[14]=NULL;
		RECT	tRect;
		SIZE	tSize;
		ZeroMemory(&tRect,sizeof(RECT));	//
		tSize = g_xMainWnd.GetStrLength(NULL,NULL,szGName);
		tRect.top = 0;
		tRect.left = 0;				// List Button 的情况, Left  = 0;
		tRect.right = tSize.cx;
		tRect.bottom = tSize.cy;
		szParam = new CHAR[35];
		ZeroMemory(szParam,35);
		strcpy(szParam,szGName);	// 临时 Parameter 与物品名称一起处理。
		// Parameter...
		if(nSubMenuCount>0)
		{	// 存在 Sub Menu 的情况
			// 若存在菜单则为亮黄色
			sprintf(szTemp,"%-14s",szGName);
			m_xKindBtn.AddButton(szTemp,tRect,i,szParam,RGB(255,255,0),RGB(255,0,0));	// Add Button to class
		}
		else
		{	// Sub Menu 不存在的情况
			// 菜单不存在则为暗黄色
			sprintf(szTemp,"%-14s",szGName);
			m_xKindBtn.AddButton(szTemp,tRect,i,szParam,RGB(180,180,0),RGB(255,0,0));	// Add Button to class
		}
//		m_xKindBtn
		delete[] szGName;
		delete[] szGSubMenu;
		delete[] szGPrice;
		delete[] szGStock;
		delete[] szParam;
	}

	m_nIsReadyToDisplay = 1;		// 通知 Kinds List 已设置
	//
	delete[] pszStr;
	return	TRUE;
}

BOOL CStoreWnd::SetBuyItemLst(CHAR* Msg,INT Count)
{
	CNPCTxtAnalysis	tTxtAnalysis;				// 只是为了使用 GetValidStr 函数..
	CHAR*	szLastMsg = NULL;
	CHAR	szDetailitem[MAX_PATH];

	m_xItemLstBtn.ReleaseButtons();
	m_xItems.ClearAllNodes();
	// 清除已保存的键&数据
	m_nCurrentItemTop = 0;

	ZeroMemory(szDetailitem,MAX_PATH);

	m_nMaxItemLine = Count;

	INT nCurrLen =0;

	for ( INT nLoop = 0; nLoop < Count; nLoop++ )
	{
		szLastMsg = strchr(Msg+nCurrLen, '/');

		if ( szLastMsg )
		{
			CHAR*	szParam;
			CHAR	bCurrentData[MAX_PATH];


			CLIENTITEMRCD* pxClItem;
			pxClItem =	new	CLIENTITEMRCD;

			memcpy(szDetailitem, Msg + nCurrLen, strlen(Msg)-strlen(szLastMsg));

			nCurrLen = nCurrLen + (int)(szLastMsg - Msg)+1;

			fnDecode6BitBuf(szDetailitem, (char*)pxClItem, sizeof(CLIENTITEMRCD));

			ZeroMemory(bCurrentData,MAX_PATH);

			//		szTemp	 =	new CHAR[50];
			CHAR szTemp1[50];	// 价格
			CHAR szTemp2[50];	// 其他

			ZeroMemory(szTemp1,50);
			ZeroMemory(szTemp2,50);

			m_xItems.AddNode(pxClItem);

//			szMakeIndex[12]; // A:武器 B:衣服 C:戒指,手镯,项链 G:其他
			sprintf(szTemp1,"%d Gold",pxClItem->stStdItem.nPrice);
/*			switch(pxClItem->szMakeIndex[0])
			{
			case 'G':	// 其他
			case 'g':
				{
					switch(pxClItem->stStdItem.bStdMode)
					{
					case 0:	// 药水类
					case 1:	// 干肉
					case 2:	// 饭团
					case 3:	// 特殊药水 or 各种典书
					case 4:	// 各种书籍
					case 7:	// 捆绑系列
					case 9:	// 药剂
						break;
					case 5:	// 毒粉
						sprintf(szTemp2,"%d",pxClItem->nDura);
						break;
					case 8:	// 不能食用的肉
						sprintf(szTemp2,"品质 %d",pxClItem->nDura);
						break;
					case 10:	// 矿石
						sprintf(szTemp2,"纯度 %d",pxClItem->nDura);
						break;
					case 6:	// 火把,蜡烛
					default:	// 其他
						sprintf(szTemp2,"耐久 %d",pxClItem->nDura);
						break;
					}
					break;
				}
			default:
				sprintf(szTemp2,"耐久 %d",pxClItem->nDura);
				break;
			}
*/
			RECT	tRect;
			SIZE	tSize;
			ZeroMemory(&tRect,sizeof(RECT));
			tSize = g_xMainWnd.GetStrLength(NULL,NULL,szTemp1);
			tRect.top = 0;
			tRect.left = 0;											// List Button 的情况, Left  = 0;
			tRect.right = m_xItemRect.right - m_xItemRect.left;		//
			tRect.bottom = tSize.cy;

			// Parameter ...
			szParam = new CHAR[35];
			ZeroMemory(szParam,35);
//			strcpy(szParam,pxClItem->szMakeIndex);	// 临时 Parameter 与物品名称一起处理.

			// Dark Yellow
			m_xItemLstBtn.AddButton(szTemp1,szTemp2,tRect,nLoop,szParam,RGB(180,180,0),RGB(255,0,0));	// Add Button to class

			delete[] szParam;
		}
	}
	m_nIsReadyToDisplay=m_nIsReadyToDisplay|2;		// 通知 Item List 设置已完成
		//
	return TRUE;
}

VOID CStoreWnd::OnScrollDown()
{
	POINT tPos;

	tPos = g_xGameProc.m_ptMousePos;
	if(IsInRect(tPos,m_xKindRect))
	{
		m_nCurrentKindTop=m_nCurrentKindTop > 0 ? m_nCurrentKindTop - 1: 0;
	}

	if(IsInRect(tPos,m_xItemRect))
	{
		m_nCurrentItemTop=m_nCurrentItemTop>0 ? m_nCurrentItemTop - 1 : 0;
	}
}

VOID CStoreWnd::OnScrollUp()
{
	POINT tPos;

	tPos = g_xGameProc.m_ptMousePos;
	if(IsInRect(tPos,m_xKindRect))
	{
		m_nCurrentKindTop=m_nCurrentKindTop < m_nMaxKindLine-1 ? m_nCurrentKindTop + 1: m_nMaxKindLine-1;
	}

	if(IsInRect(tPos,m_xItemRect))
	{
		m_nCurrentItemTop=m_nCurrentItemTop < m_nMaxItemLine-1 ? m_nCurrentItemTop + 1 : m_nMaxItemLine-1;
	}
}

BOOL CStoreWnd::SetSellKinds(CInventoryWnd* pxInvenWnd,CHAR Kind,INT nStdMode)
{
	if(pxInvenWnd!=NULL)
	{
	}
	return FALSE;
}

BOOL CStoreWnd::SetSellItemLst(CInventoryWnd* pxInvenWnd,CHAR Kind,BYTE nStdMode,CHAR bKind)
{
	CLIENTITEMRCD*	pxItem;
	RECT	tRect;
	SIZE	tSize;
	CHAR szTemp1[50];
	CHAR szTemp2[50];
	CHAR szParam[50];
	INT nCount=0;
//	if(不是旅馆吗?)
//	{	// 如果不是旅馆就要过滤后再放入.
		if(pxInvenWnd!=NULL)
		{
			for(int i=0 ; i<60; i++)
			{
				pxItem = &pxInvenWnd->m_stInventoryItem[i].xItem.m_stItemInfo;
//				if(bKind==pxItem->szMakeIndex[0])
				{
					if(bKind!='a' && bKind!='A' && bKind!='b' && bKind!='B')
					{
						if(nStdMode==pxItem->stStdItem.bStdMode)
						{	// 其他项目必须再过滤一次.
							m_xItems.AddNode(pxItem);
							strcpy(szTemp1,pxItem->stStdItem.szName);
							itoa(pxItem->nDura,szTemp2,10);
							strcpy(szParam,pxItem->stStdItem.szName);

							tSize = g_xMainWnd.GetStrLength(NULL,NULL,szTemp1);
							tRect.top = 0;
							tRect.left = 0;											// List Button 的情况, Left  = 0;
							tRect.right = m_xItemRect.right - m_xItemRect.left;		//
							tRect.bottom = tSize.cy;

							m_xItemLstBtn.AddButton(szTemp1,szTemp2,tRect,nCount,szParam,RGB(180,180,0),RGB(255,0,0));
							nCount++;
						}
					}
					else
					{
						// 武器或衣服直接放入也无妨..
						m_xItems.AddNode(pxItem);
						strcpy(szTemp1,pxItem->stStdItem.szName);
						itoa(pxItem->nDura,szTemp2,10);
						strcpy(szParam,pxItem->stStdItem.szName);

						tSize = g_xMainWnd.GetStrLength(NULL,NULL,szTemp1);
						tRect.top = 0;
						tRect.left = 0;											// List Button 的情况, Left  = 0;
						tRect.right = m_xItemRect.right - m_xItemRect.left;		//
						tRect.bottom = tSize.cy;

						m_xItemLstBtn.AddButton(szTemp1,szTemp2,tRect,nCount,szParam,RGB(180,180,0),RGB(255,0,0));
						nCount++;
					}
				}
			}
		}
//	}
//	else
//	{	// 若是旅馆就可以随便放入。
//		for(int i=0 ; i<60; i++)
//		{
//			pxItem = &pxInvenWnd->m_stInventoryItem[i].xItem.m_stItemInfo;
			// 武器或衣服直接放入也无妨..
//			m_xItems.AddNode(pxItem);
//			strcpy(szTemp1,pxItem->stStdItem.szName);
//			itoa(pxItem->nDura,szTemp2,10);
//			strcpy(szParam,pxItem->stStdItem.szName);

//			tSize = g_xMainWnd.GetStrLength(NULL,NULL,szTemp1);
//			tRect.top = 0;
//			tRect.left = 0;											// List Button 的情况下, Left  = 0;
//			tRect.right = m_xItemRect.right - m_xItemRect.left;		//
//			tRect.bottom = tSize.cy;
//
//			m_xItemLstBtn.AddButton(szTemp1,szTemp2,tRect,nCount,szParam,RGB(180,180,0),RGB(255,0,0));
//			nCount++;
//		}
//	}
	return FALSE;
}


BOOL CStoreWnd::OnUpdateQueue(VOID)
{
	if(m_xPacketQueue.GetCount()>0)
	{
		LPPACKETMSG	lpPacketMsg;
		lpPacketMsg =(LPPACKETMSG) m_xPacketQueue.PopQ();
		if(lpPacketMsg->stDefMsg.nRecog == m_nNpcID)
		{
			switch(lpPacketMsg->stDefMsg.wIdent)
			{
			case SM_SENDGOODSLIST:
				{
					INT nPos;
					CHAR  szDecodeMsg[MAX_PATH*4];
					nPos = fnDecode6BitBuf(lpPacketMsg->szEncodeData, szDecodeMsg, sizeof(szDecodeMsg));
					SetBuyKinds(szDecodeMsg,lpPacketMsg->stDefMsg.wIdent);
					break;
				}
			case SM_SENDDETAILGOODSLIST:
				{
					SetBuyItemLst(lpPacketMsg->szEncodeData,(lpPacketMsg->stDefMsg).wSeries);
					break;
				}
			case SM_BUYITEM_FAIL:
				{
					break;
				}
				/*			default :
				{
				m_xPacketQueue.PushQ((BYTE*)lpPacketMsg);
				return FALSE
			}*/
			}
		}
		else
		{
			// 不是该 NPC
			// 待做事项
			// 将关闭 NPC 窗口
			// 将更新 NPC
			m_xPacketQueue.PushQ((BYTE*)lpPacketMsg);
			return FALSE;
		}
		delete lpPacketMsg;
		return TRUE;
	}
	return FALSE;
}

VOID CStoreWnd::SendDeTailGoodsList(INT nIndex,CHAR* szItemName)
{
	g_xClientSocket.SendGetDetailItem(m_nNpcID, nIndex, szItemName);
}

VOID CStoreWnd::SendButThisITme(CMTxtBtn* tBtn)
{
	CHAR	szItemName[MAX_PATH];

	ZeroMemory(szItemName,MAX_PATH);

	m_xItems.MoveCurrentToTop();
	for(INT i = 0 ; i<m_xItems.GetCounter();i++)
	{
//		if(strncmp(m_xItems.GetCurrentData()->szMakeIndex,tBtn->Param,12)==0)
		{
//			g_xClientSocket.SendBuyItem(m_nNpcID,m_xItems.GetCurrentData()->stStdItem.szName,tBtn->Param);					// MOON
			return;
		}
		m_xItems.MoveNextNode();
	}
}

#undef MAX_KINDS_LINE
#undef MAX_ITEMLIST_LINE
#undef LINE_GAP
#undef POS_KINDS_X
#undef POS_KINDS_Y
#undef POS_KINDSTITLE_X
#undef POS_KINDSTITLE_Y
#undef POS_ITEMSTITLE_X
#undef POS_ITEMSTITLE_Y
#undef POS_ITEMLIST_LINE_X
#undef POS_ITEMLIST_LINE_Y
#undef POS_ITEMNAME_X
#undef POS_ITEMNAME_Y
#undef POS_ITEMIMAGE_X
#undef POS_ITEMIMAGE_Y
#undef LEFT_KIND_RECT
#undef RIGHT_KIND_RECT
#undef TOP_KIND_RECT
#undef BOTTOM_KIND_RECT
#undef LEFT_ITEM_RECT
#undef RIGHT_ITEM_RECT
#undef TOP_ITEM_RECT
#undef BOTTOM_ITEM_RECT
#undef POS_X
#undef POS_Y
#undef POS__Y
#undef POS__X
#undef MAX_MSG_LENGTH

// Interface
/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/






CInterface::CInterface()
{
	Init();
}



CInterface::~CInterface()
{
	Destroy();
}



/******************************************************************************************************************

	函数名 : CInterface::Init()

	作者 :
	编写日期 :

	目的   : 接口相关变量及对象初始化.
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInterface::Init()
{
	m_nIdxMsgBox		= 0;
	m_nFstLine			= 0;
	m_shBeltHeight		= 0;
	m_bShowKey			= FALSE;
	m_bMoveFocusedWnd	= FALSE;
	m_bBeltState		= _BELT_STATE_STAY;

	SetRect(&m_rcMain,  0, 0, 0, 0);
	SetRect(&m_rcChat,  0, 0, 0, 0);

	ZeroMemory(&m_stCommonItem, sizeof(ITEMSET));

	m_xClientSysMsg.Init();

	m_pxInterImg = NULL;
	m_xInterImgEx.Init();

	m_xStatusWnd.Init();
	m_xStoreWnd.Init();
	m_xGuildWnd.Init();
	m_xGroupWnd.Init();
	m_xChatPopWnd.Init();
	m_xExchangeWnd.Init();
	m_xGroupPopWnd.Init();
	m_xInventoryWnd.Init();
	m_xGuildMasterWnd.Init();

	for ( INT nCnt = 0; nCnt < _MAX_INTER_BTN; nCnt++ )
	{
		m_xInterBtn[nCnt].Init();
	}

	m_xScrlBar.Init();

	m_xChat.ClearAllNodes();
	m_xWndOrderList.ClearAllNodes();
}



/******************************************************************************************************************

	函数名 : CInterface::Destroy()

	作者   :
	编写日期 :

	目的   : 接口相关变量、对象的销毁及初始化。
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInterface::Destroy()
{
	m_xClientSysMsg.Destroy();

	m_xInterImgEx.Destroy();
	m_pxInterImg = NULL;

	m_xInventoryWnd.Destroy();
	m_xStoreWnd.Destroy();
	m_xStatusWnd.Destroy();
	m_xGuildWnd.Destroy();
	m_xGroupWnd.Destroy();
	m_xExchangeWnd.Destroy();
	m_xGuildMasterWnd.Destroy();
	m_xGroupPopWnd.Destroy();
	m_xChatPopWnd.Destroy();

	for ( INT nCnt = 0; nCnt < _MAX_INTER_BTN; nCnt++ )
	{
		m_xInterBtn[nCnt].Destroy();
	}

	m_xChat.ClearAllNodes();
	m_xWndOrderList.ClearAllNodes();

	Init();
}


VOID CInterface::CreateInterface(CImageHandler* pxImgHandler)
{
	m_pxInterImg = &(pxImgHandler->m_xImageList[_IMAGE_INTER]);

	//------------------------------------------------------------------------------------------------------------
	// 聊天编辑窗口的位置指定.
	MoveWindow(g_xChatEditBox.GetSafehWnd(),
			   g_xMainWnd.m_rcWindow.left+_INPUT_EDITWND_XPOS, g_xMainWnd.m_rcWindow.top+_INPUT_EDITWND_YPOS,
			   _INPUT_EDITWND_WIDTH, _INPUT_EDITWND_HEIGHT, TRUE);
	//------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------
	// 加载接口图像.
	m_pxInterImg->NewLoad(".\\Data\\gameinter.wil");
	m_xInterImgEx.NewLoad(".\\Data\\interfacec2.wil");
	//------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------
	// 主界面区域设置.
	if ( m_pxInterImg->NewSetIndex(_WNDIMGIDX_MAIN) )
	{
		SetRect(&m_rcMain,
				0, (600-m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight),
				m_pxInterImg->m_lpstNewCurrWilImageInfo->shWidth, 600);
	}
	//------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------
	// 聊天区域设置。
	SetRect(&m_rcChat, 224, 471, 578, 545);
	//------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------
	// 创建各窗口。
	m_xInventoryWnd.CreateInventoryWnd(_WND_ID_INVENTORY, m_pxInterImg, _WNDIMGIDX_INVENTORY, 0, 0, 0, 0, TRUE);
	m_xStatusWnd.CreateStatusWnd(_WND_ID_STATUS, m_pxInterImg, _WNDIMGIDX_ITEMSET, 510, 0, 0, 0, TRUE);
	m_xStoreWnd.CreateStoreWnd(_WND_ID_STORE, m_pxInterImg, _WNDIMGIDX_STORE, 0, 0, 0, 0, TRUE);
	m_xExchangeWnd.Create(_WND_ID_EXCHANGE, m_pxInterImg, _WNDIMGIDX_EXCHANGE, 280, 255, 291, 567, 413);
	m_xGuildMasterWnd.Create(_WND_ID_GUILDMASTER, m_pxInterImg, _WNDIMGIDX_GUILDMASTER, 630, 567, 413, 567, 413);
	m_xGuildWnd.CreateGuildWnd(_WND_ID_GUILD, &m_xInterImgEx, _WNDIMGIDX_GUILD, 0, 0, 540, 442, TRUE);
	m_xGroupWnd.CreateGroupWnd(_WND_ID_GROUP, m_pxInterImg, _WNDIMGIDX_GROUP, 0, 0, 0, 0, TRUE);
	m_xGroupPopWnd.CreateGroupPopWnd(_WND_ID_GROUPPOP, &m_xInterImgEx, _WNDIMGIDX_GROUPPOP, 0, 0, 260, 114, TRUE);
	m_xChatPopWnd.CreateChatPopWnd(_WND_ID_CHATPOP, m_pxInterImg, _WNDIMGIDX_CHATPOP, 0, 0, 0, 0, TRUE);

	m_xOptionWnd.Create(_WND_ID_OPTION, m_pxInterImg, _WNDIMGIDX_OPTION, 764, 220, 11, 220, 11);
	m_xQuestWnd.Create(_WND_ID_QUEST, m_pxInterImg, _WNDIMGIDX_QUEST, 630, 567, 413, 567, 413);
	m_xHorseWnd.Create(_WND_ID_HORSE, m_pxInterImg, _WNDIMGIDX_HORSE, 630, 567, 413, 567, 413);


	m_xNPCWnd.CreateNPCChatWnd(_WND_ID_NPCCHAT, m_pxInterImg, _WNDIMGIDX_NPCCHAT, 0, 0, 0, 0, TRUE);				// 勇气
	//------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------
	// 按钮初始化。
	m_xInterBtn[ 0].CreateGameBtn(m_pxInterImg, 80, 81, m_rcMain.left+204, m_rcMain.top+0);		// 交易窗口。
	m_xInterBtn[ 1].CreateGameBtn(m_pxInterImg, 82, 83, m_rcMain.left+228, m_rcMain.top+0);		// 小地图.
	m_xInterBtn[ 2].CreateGameBtn(m_pxInterImg, 84, 85, m_rcMain.left+252, m_rcMain.top+0);		// 武功窗口.

	m_xInterBtn[ 3].CreateGameBtn(m_pxInterImg, 90, 91, m_rcMain.left+161, m_rcMain.top+65);	// 退出游戏.
	m_xInterBtn[ 4].CreateGameBtn(m_pxInterImg, 92, 93, m_rcMain.left+161, m_rcMain.top+101);	// 注销.
	m_xInterBtn[ 5].CreateGameBtn(m_pxInterImg, 94, 95, m_rcMain.left+616, m_rcMain.top+66);	// 队伍.
	m_xInterBtn[ 6].CreateGameBtn(m_pxInterImg, 96, 97, m_rcMain.left+616, m_rcMain.top+102);	// 行会.

	m_xInterBtn[ 7].CreateGameBtn(m_pxInterImg, 52, 53, m_rcMain.left+397, m_rcMain.top+13);	// 腰带 UP, DOWN.

	m_xInterBtn[ 8].CreateGameBtn(m_pxInterImg,100, 101, m_rcMain.left+703, m_rcMain.top+34);	// 技能窗口。
	m_xInterBtn[ 9].CreateGameBtn(m_pxInterImg,102, 103, m_rcMain.left+718, m_rcMain.top+50);	// 聊天弹窗.
	m_xInterBtn[10].CreateGameBtn(m_pxInterImg,104, 105, m_rcMain.left+718, m_rcMain.top+88);	// 任务.
	m_xInterBtn[11].CreateGameBtn(m_pxInterImg,106, 107, m_rcMain.left+703, m_rcMain.top+103);	// 选项.
	m_xInterBtn[12].CreateGameBtn(m_pxInterImg,108, 109, m_rcMain.left+664, m_rcMain.top+104);	// 帮助.
	m_xInterBtn[13].CreateGameBtn(m_pxInterImg,110, 111, m_rcMain.left+648, m_rcMain.top+88);	// 马窗口。
	m_xInterBtn[14].CreateGameBtn(m_pxInterImg,112, 113, m_rcMain.left+648, m_rcMain.top+50);	// 背包窗口.
	m_xInterBtn[15].CreateGameBtn(m_pxInterImg,114, 115, m_rcMain.left+665, m_rcMain.top+34);	// 状态窗口.
	//------------------------------------------------------------------------------------------------------------

	m_xScrlBar.CreateScrlBar(m_pxInterImg, 68, _MAX_CHATLINE, 12, 84, 12);

	m_xMsgBox.Load(&m_xInterImgEx);	// 勇气
}



VOID CInterface::MsgAdd(DWORD dwFontColor, DWORD dwFontBackColor, CHAR* pszMsg)
{
	if ( pszMsg )
	{
		if ( pszMsg[0] != NULL )
		{
			INT  nCnt;
			CHAT stChat;
			INT	 nLineCnt;
			CHAR pszDivied[MAX_PATH*2];
			CHAR pszArg[5][MAX_PATH];

			m_xChatPopWnd.MsgAdd(dwFontColor, dwFontBackColor, pszMsg);

			ZeroMemory(pszDivied, MAX_PATH*2);
			ZeroMemory(pszArg,	  MAX_PATH*5);

			g_xMainWnd.StringDivide(m_rcChat.right-m_rcChat.left, nLineCnt, pszMsg, pszDivied);

			sscanf(pszDivied, "%[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c %[^`]%*c", pszArg[0], pszArg[1], pszArg[2], pszArg[3], pszArg[4]);

			if ( nLineCnt > 5 )
			{
				nLineCnt = 5;
			}

			// 插入字符串。
			for ( nCnt = 0; nCnt < nLineCnt; nCnt++ )
			{
				// 格子进入下一格的条件.
				if ( m_xChat.GetCounter() - m_nFstLine == _MAX_CHATLINE )
				{
					m_nFstLine++;
				}

				stChat.dwFontColor = dwFontColor;
				stChat.dwBackColor = dwFontBackColor;
				strcpy(stChat.pszChat, pszArg[nCnt]);

				m_xChat.AddNode(stChat);
			}

			// 检查最大保存行数，超过则删除。
			while ( m_xChat.GetCounter() >= 50 )
			{
				m_xChat.MoveCurrentToTop();
				m_xChat.DeleteCurrentNode();

				if ( m_nFstLine > 0 )
					m_nFstLine--;
			}
		}
	}
}



/******************************************************************************************************************

	函数名 : CInterface::ShowWndList()

	编写者 :
	编写日期 :

	目的   : 显示处于 Active 状态的窗口列表.
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInterface::ShowWndList()
{
	if (!m_xWndOrderList.GetCounter()) return;
	m_xWndOrderList.MoveCurrentToTop();
	for (INT i = 0; i < m_xWndOrderList.GetCounter(); ++i)
	{
		if (CGameWnd* wnd = FindWindowById(*m_xWndOrderList.GetCurrentData()))
			wnd->ShowPanel();
		m_xWndOrderList.MoveNextNode();
	}
}



/******************************************************************************************************************

	函数名 : CInterface::ShowChatList()

	作者 :
	日期 :

	目的   : 显示聊天列表.
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInterface::ShowChatList()
{
	if ( !m_xChat.CheckEmpty() )
	{
		INT		nCnt, nShowLine;
		LPCHAT	pstChat;

		m_xChat.MoveCurrentToTop();
		m_xChat.MoveNode(m_nFstLine);

		nShowLine = ((m_xChat.GetCounter() - m_nFstLine ) >= _MAX_CHATLINE) ?  _MAX_CHATLINE : m_xChat.GetCounter() - m_nFstLine;

		for ( nCnt = 0; nCnt < nShowLine; nCnt++ )
		{
			pstChat = m_xChat.GetCurrentData();

			g_xMainWnd.PutsHan(g_xMainWnd.GetBackBuffer(),
							   m_rcChat.left, m_rcChat.top+nCnt*15,
							   pstChat->dwBackColor, pstChat->dwFontColor, pstChat->pszChat);

			m_xChat.MoveNextNode();
		}
	}
}



VOID CInterface::RenderInterface(INT nLoopTime, POINT ptMousePos)
{
	INT nX, nY, nCnt;

	// 窗口.
	ShowWndList();

	// 鼠标物品.
	if ( m_stCommonItem.bSetted && !m_stCommonItem.bIsHideItem )
	{
		nX = ptMousePos.x - _INVENTORY_CELL_WIDTH /2;
		nY = ptMousePos.y - _INVENTORY_CELL_HEIGHT/2;
		m_stCommonItem.xItem.DrawItem(nX, nY);
	}

	// 腰带物品。
	ShowBeltItem();

	// 主界面.
	if ( m_pxInterImg->NewSetIndex(_WNDIMGIDX_MAIN) )
	{
		g_xMainWnd.DrawWithImageForCompClipRgn(m_rcMain.left, m_rcMain.top,	m_rcMain.right-m_rcMain.left, m_rcMain.bottom-m_rcMain.top,
											   (WORD*)m_pxInterImg->m_pbCurrImage, _CLIP_WIDTH, 600);
	}

	// 聊天列表.
	ShowChatList();

	// 状态值.
	ShowGameStatus();

	// 按钮.
	for ( nCnt = 0; nCnt < _MAX_INTER_BTN; nCnt++)
	{
		m_xInterBtn[nCnt].ShowGameBtn();
	}

	// 滚动条.
	m_xScrlBar.ShowScrlBar(584, 404, m_nFstLine, m_xChat.GetCounter());

	// 客户端消息(左上角).
	m_xClientSysMsg.ShowSysMsg(nLoopTime, 30, 30);

	// 勇气
	m_xMsgBox.RenderMessageBox(0);

	ShowMagicKey();
}



VOID CInterface::ShowBeltItem()
{
	INT nX, nY;

	if ( m_bBeltState == _BELT_STATE_UP )
	{
		m_shBeltHeight += 10;

		if ( m_shBeltHeight >= 46 )
		{
			m_shBeltHeight = 46;
		}
	}
	else if ( m_bBeltState == _BELT_STATE_DOWN )
	{
		m_shBeltHeight -= 10;
		if ( m_shBeltHeight <= 0 )
		{
			m_shBeltHeight = 0;
			m_bBeltState   = _BELT_STATE_STAY;
		}
	}

	if ( m_bBeltState != _BELT_STATE_STAY )
	{
		if ( m_pxInterImg->NewSetIndex(_IMGIDX_BELT) )
		{
			g_xMainWnd.DrawWithImageForCompClipRgn(275, 457-m_shBeltHeight,
												   m_pxInterImg->m_lpstNewCurrWilImageInfo->shWidth,
												   m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight,
												   (WORD*)m_pxInterImg->m_pbCurrImage, _CLIP_WIDTH, 600);
			// 腰带物品绘制。
			for ( INT nCnt = 0; nCnt < _MAX_BELT_ITEM; nCnt++ )
			{
				if ( m_xInventoryWnd.m_stBeltItem[nCnt].bSetted )
				{
					nX = 275 + 8 + nCnt*40;
					nY = 411 + 6 + 46 - m_shBeltHeight;

					m_xInventoryWnd.m_stBeltItem[nCnt].xItem.DrawItem(nX, nY);
				}
			}
		}

	}
}



VOID CInterface::ShowGameStatus()
{
	FLOAT	fMyHPRate;
	FLOAT	fMyMPRate;
	FLOAT	fMyWetRate;
	FLOAT	fMyExpRate;
	WORD	wJobIcon;
	WORD	wImgNum;
	CHAR	pszBuff[MAX_PATH];
	RECT	rcStats;
	DWORD	dwFntClr;

	BYTE	bMyJob  = g_xGameProc.m_xMyHero.m_bJob;
	BYTE	bMyLevel= g_xGameProc.m_xMyHero.m_stAbility.bLevel;
	WORD	wMyAC   = g_xGameProc.m_xMyHero.m_stAbility.wAC;
	WORD	wMyDC   = g_xGameProc.m_xMyHero.m_stAbility.wDC;
	WORD	wMyMC   = g_xGameProc.m_xMyHero.m_stAbility.wMC;
	WORD	wMySC   = g_xGameProc.m_xMyHero.m_stAbility.wSC;
	BYTE	bDay	= g_xGameProc.m_bDayState;

	WORD	wHP		= g_xGameProc.m_xMyHero.m_stAbility.wHP;
	WORD	wMP		= g_xGameProc.m_xMyHero.m_stAbility.wMP;
	WORD	wWet	= g_xGameProc.m_xMyHero.m_stAbility.wWeight;
	DWORD	dwExp	= g_xGameProc.m_xMyHero.m_stAbility.dwExp;

	BYTE	bJob	= g_xGameProc.m_xMyHero.m_bJob;

	// 一天的状态值(夜晚, 白天, 傍晚, 黎明)
	switch ( bDay )
	{
	case _DAYSTATE_DAY:
		wImgNum = 130;
		break;
	case _DAYSTATE_DUSK:
		wImgNum = 131;
		break;
	case _DAYSTATE_NIGHT:
		wImgNum = 132;
		break;
	case _DAYSTATE_DAWN:
		wImgNum = 133;
		break;
	}
	if ( m_pxInterImg->NewSetIndex(wImgNum) )
	{
		g_xMainWnd.DrawWithImageForComp(m_rcMain.left+684, m_rcMain.top+70,
										m_pxInterImg->m_lpstNewCurrWilImageInfo->shWidth,
										m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight,
										(WORD*)m_pxInterImg->m_pbCurrImage, _CLIP_WIDTH, 600);
	}

	// 绘制 HP, MP.
	// 比例检查。
	if ( wHP )
	{
		fMyHPRate = (FLOAT)((FLOAT)wHP/(FLOAT)g_xGameProc.m_xMyHero.m_stAbility.wMaxHP);

		if ( fMyHPRate > 1.0f )
		{
			fMyHPRate = 1.0f;
		}
	}
	else
	{
		fMyHPRate = 0;
	}

	if ( wMP )
	{
		fMyMPRate = (FLOAT)((FLOAT)wMP/(FLOAT)g_xGameProc.m_xMyHero.m_stAbility.wMaxMP);

		if ( fMyMPRate > 1.0f )
		{
			fMyMPRate = 1.0f;
		}
	}
	else
	{
		fMyMPRate = 0;
	}

	if ( dwExp )
	{
		fMyExpRate = (FLOAT)((FLOAT)dwExp/(FLOAT)g_xGameProc.m_xMyHero.m_stAbility.dwMaxExp);

		if ( fMyExpRate > 1.0f )
		{
			fMyExpRate = 1.0f;
		}
	}
	else
	{
		fMyExpRate = 0;
	}

	if ( wWet )
	{
		fMyWetRate = (FLOAT)((FLOAT)wWet /(FLOAT)g_xGameProc.m_xMyHero.m_stAbility.wMaxWeight);

		if ( fMyWetRate > 1.0f )
		{
			fMyWetRate = 1.0f;
		}
	}
	else
	{
		fMyWetRate = 0;
	}

	if ( m_pxInterImg->NewSetIndex(60) )
	{
		RECT rcHP = { 0, m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight-(m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight*fMyHPRate),
					  m_pxInterImg->m_lpstNewCurrWilImageInfo->shWidth, m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight };
		g_xMainWnd.DrawWithImageForComp(m_rcMain.left+46, m_rcMain.top+34, rcHP, (WORD*)m_pxInterImg->m_pbCurrImage);
	}

	if ( m_pxInterImg->NewSetIndex(61) )
	{
		RECT rcMP = { 0, m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight-(m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight*fMyMPRate),
					  m_pxInterImg->m_lpstNewCurrWilImageInfo->shWidth, m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight };
		g_xMainWnd.DrawWithImageForComp(m_rcMain.left+104, m_rcMain.top+34, rcMP, (WORD*)m_pxInterImg->m_pbCurrImage);
	}

	// 绘制 EXP, WET。
	if ( m_pxInterImg->NewSetIndex(63) )
	{
		RECT rcEXP = { 0, m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight-(m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight*fMyExpRate),
					   m_pxInterImg->m_lpstNewCurrWilImageInfo->shWidth, m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight };
		g_xMainWnd.DrawWithImageForComp(m_rcMain.left+76, m_rcMain.top+35, rcEXP, (WORD*)m_pxInterImg->m_pbCurrImage);
	}

	if ( m_pxInterImg->NewSetIndex(67) )
	{
		RECT rcWET = { 0, m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight-(m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight*fMyWetRate),
					  m_pxInterImg->m_lpstNewCurrWilImageInfo->shWidth, m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight };
		g_xMainWnd.DrawWithImageForComp(m_rcMain.left+211, m_rcMain.top+35, rcWET, (WORD*)m_pxInterImg->m_pbCurrImage);
	}

	dwFntClr = RGB(225, 225, 0);
	// 练级6.
	sprintf(pszBuff, "%d", g_xGameProc.m_xMyHero.m_stAbility.bLevel);
	SetRect(&rcStats, 693, 528, 709, 544);
	g_xMainWnd.PutsHan(NULL, rcStats, dwFntClr, RGB(0, 0, 0), pszBuff, g_xMainWnd.CreateGameFont("Arial", 10, 0, FW_BOLD));

	dwFntClr = RGB(255, 255, 200);
	// 绘制地图名称及坐标.
	sprintf(pszBuff, "%s : [%d,%d]", g_xGameProc.m_szMapName, g_xGameProc.m_xMyHero.m_wPosX, g_xGameProc.m_xMyHero.m_wPosY);
	SetRect(&rcStats, 219, 584, 379, 599);
	g_xMainWnd.PutsHan(NULL, rcStats, dwFntClr, RGB(0, 0, 0), pszBuff);

	dwFntClr = RGB(255, 200, 50);
	// 绘制各职业图标及属性值.
	sprintf(pszBuff, "%d-%d", LOBYTE(wMyAC), HIBYTE(wMyAC));
	SetRect(&rcStats, 422, 584, 483, 599);
	g_xMainWnd.PutsHan(NULL, rcStats, dwFntClr, RGB(0, 0, 0), pszBuff);

	SetRect(&rcStats, 520, 584, 581, 599);
	switch ( bJob )
	{
	case _JOB_DOSA:
		{
			wJobIcon = 66;
			wImgNum	 = 123;
			sprintf(pszBuff, "%d-%d", LOBYTE(wMySC), HIBYTE(wMySC));
			g_xMainWnd.PutsHan(NULL, rcStats, dwFntClr, RGB(0, 0, 0), pszBuff);
		}
		break;
	case _JOB_SULSA:
		{
			wJobIcon = 65;
			wImgNum	 = 124;
			sprintf(pszBuff, "%d-%d", LOBYTE(wMyMC), HIBYTE(wMyMC));
			g_xMainWnd.PutsHan(NULL, rcStats, dwFntClr, RGB(0, 0, 0), pszBuff);
		}
		break;
	case _JOB_JUNSA:
		{
			wJobIcon = 64;
			wImgNum	 = 122;
			sprintf(pszBuff, "%d-%d", LOBYTE(wMyDC), HIBYTE(wMyDC));
			g_xMainWnd.PutsHan(NULL, rcStats, dwFntClr, RGB(0, 0, 0), pszBuff);
		}
		break;
	}
	if ( m_pxInterImg->NewSetIndex(wJobIcon) )
	{
		g_xMainWnd.DrawWithImageForComp(m_rcMain.left+82, m_rcMain.top+71,
										m_pxInterImg->m_lpstNewCurrWilImageInfo->shWidth,
										m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight,
										(WORD*)m_pxInterImg->m_pbCurrImage);
	}
	// AC 标记.
	if ( m_pxInterImg->NewSetIndex(120) )
	{
		g_xMainWnd.DrawWithImageForComp(m_rcMain.left+391, m_rcMain.top+136,
										m_pxInterImg->m_lpstNewCurrWilImageInfo->shWidth,
										m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight,
										(WORD*)m_pxInterImg->m_pbCurrImage);
	}
	// DC, MC, SC
	if ( m_pxInterImg->NewSetIndex(wImgNum) )
	{
		g_xMainWnd.DrawWithImageForComp(m_rcMain.left+487, m_rcMain.top+136,
										m_pxInterImg->m_lpstNewCurrWilImageInfo->shWidth,
										m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight,
										(WORD*)m_pxInterImg->m_pbCurrImage);
	}
}


VOID CInterface::ShowMagicKey()
{
	if ( m_bShowKey )
	{
		INT nCnt;

		for ( nCnt = 0; nCnt < 8; nCnt++ )
		{
			if ( m_pxInterImg->NewSetIndex(70+nCnt) )
			{
				g_xMainWnd.DrawWithABlendCompDataWithBackBuffer(nCnt*m_pxInterImg->m_lpstNewCurrWilImageInfo->shWidth, 0,
																m_pxInterImg->m_lpstNewCurrWilImageInfo->shWidth,
																m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight,
																(WORD*)m_pxInterImg->m_pbCurrImage, _CLIP_WIDTH, 600);

			}
		}
	}
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 窗口管理部分/////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************************************************************************

	函数名 : CInterface::GetWindowInMousePos()

	作者 :
	日期 :

	目的   : 返回鼠标指针所在区域内的窗口列表中第一个窗口的 ID.
	输入   : POINT ptMouse
	输出   :

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
CGameWnd* CInterface::FindWindowById(INT id)
{
	switch (id)
	{
	case _WND_ID_INVENTORY: return &m_xInventoryWnd;
	case _WND_ID_STATUS: return &m_xStatusWnd;
	case _WND_ID_STORE: return &m_xStoreWnd;
	case _WND_ID_EXCHANGE: return &m_xExchangeWnd;
	case _WND_ID_GUILDMASTER: return &m_xGuildMasterWnd;
	case _WND_ID_GUILD: return &m_xGuildWnd;
	case _WND_ID_GROUP: return &m_xGroupWnd;
	case _WND_ID_GROUPPOP: return &m_xGroupPopWnd;
	case _WND_ID_CHATPOP: return &m_xChatPopWnd;
	case _WND_ID_NPCCHAT: return &m_xNPCWnd;
	case _WND_ID_QUEST: return &m_xQuestWnd;
	case _WND_ID_OPTION: return &m_xOptionWnd;
	case _WND_ID_HORSE: return &m_xHorseWnd;
	default: return NULL;
	}
}

INT	CInterface::GetWindowInMousePos(POINT ptMouse)
{
	if (!m_xWndOrderList.GetCounter()) return -1;
	m_xWndOrderList.MoveCurrentToTail();
	for (INT i = 0; i < m_xWndOrderList.GetCounter(); ++i)
	{
		INT id = *m_xWndOrderList.GetCurrentData();
		CGameWnd* wnd = FindWindowById(id);
		if (wnd && PtInRect(&wnd->m_rcWnd, ptMouse)) return id;
		m_xWndOrderList.MovePreviousNode();
	}
	return -1;
}


/******************************************************************************************************************

	函数名 : CInterface::AddWindowToList()

	作者 :
	编写日期 :

	目的   : 在窗口列表的最后添加当前输入的 ID 为 nID 的窗口。
	输入   : INT nID
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInterface::AddWindowToList(INT nID)
{
	m_xWndOrderList.AddNode(nID);
}


/******************************************************************************************************************

	函数名 : CInterface::DeleteWindowToList()

	作者 :
	编写日期 :

	目的   : 从列表上删除具有 nID 标识的窗口.
	输入   : INT nID
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInterface::DeleteWindowToList(INT nID)
{
	if ( m_xWndOrderList.GetCounter() != 0 )
	{
		m_xWndOrderList.MoveCurrentToTop();

		INT* pnCurrID;
		for ( INT nCnt = 0; nCnt < m_xWndOrderList.GetCounter(); nCnt++ )
		{
			pnCurrID = m_xWndOrderList.GetCurrentData();

			if ( *pnCurrID == nID )
			{
				m_xWndOrderList.DeleteCurrentNode();
				break;
			}

			m_xWndOrderList.MoveNextNode();
		}
	}
}


/******************************************************************************************************************

	函数名 : CInterface::WindowActivate()

	作者 :
	编写日期 :

	目的   : 将 nID 窗口添加到窗口列表中或从列表中删除,添加时将窗口状态设置为最顶层状态。
			 是窗口接收键盘输入并显示到屏幕时调用的函数。
	输入   : INT nID
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CInterface::WindowActivate(INT nID)
{
	AllWindowsFocusOff();
	CGameWnd* wnd = FindWindowById(nID);
	if (!wnd) return FALSE;
	BOOL opening = !wnd->GetGameWndActive();
	if (nID == _WND_ID_CHATPOP)
	{
		if (opening)
		{
			MoveWindow(g_xChatEditBox.GetSafehWnd(), g_xMainWnd.m_rcWindow.left + m_xChatPopWnd.m_rcWnd.left + m_xChatPopWnd.m_rcEditBoxFrame.left,
						   g_xMainWnd.m_rcWindow.top + m_xChatPopWnd.m_rcWnd.top + m_xChatPopWnd.m_rcEditBoxFrame.top,
						   m_xChatPopWnd.m_rcEditBoxFrame.right - m_xChatPopWnd.m_rcEditBoxFrame.left,
						   m_xChatPopWnd.m_rcEditBoxFrame.bottom - m_xChatPopWnd.m_rcEditBoxFrame.top, TRUE);
			SetFocus(g_xChatEditBox.GetSafehWnd());
		}
		else
		{
			MoveWindow(g_xChatEditBox.GetSafehWnd(), g_xMainWnd.m_rcWindow.left+_INPUT_EDITWND_XPOS,
						   g_xMainWnd.m_rcWindow.top+_INPUT_EDITWND_YPOS, _INPUT_EDITWND_WIDTH, _INPUT_EDITWND_HEIGHT, TRUE);
		}
	}
	if (opening) AddWindowToList(nID);
	else DeleteWindowToList(nID);
	wnd->SetGameWndActive(opening);
	return opening;
}


/******************************************************************************************************************

	函数名 : CInterface::MoveTopWindow()

	作者 :
	编写日期 :

	目的   : 将最上层的窗口移动到鼠标坐标。
	输入   : POINT ptMouse
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInterface::MoveTopWindow(POINT ptMouse)
{
	if (!m_xWndOrderList.GetCounter()) return;
	m_xWndOrderList.MoveCurrentToTail();
	if (!m_bMoveFocusedWnd) return;
	INT id = *m_xWndOrderList.GetCurrentData();
	if (CGameWnd* wnd = FindWindowById(id)) wnd->MoveGameWnd(ptMouse);
	if (id == _WND_ID_CHATPOP) ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
}



/******************************************************************************************************************

	函数名 : CInterface::WindowFocusChanged()

	作者 :
	编写日期 :

	目的   : 在窗口区域点击鼠标时，将当前的顶层窗口切换为鼠标点击的窗口，并把焦点设置到该窗口.
	输入   : INT nID
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInterface::WindowFocusChangedAndSetReadyMove(INT nID, POINT ptMouse)
{
	if (!m_xWndOrderList.GetCounter()) return;
	AllWindowsFocusOff();
	DeleteWindowToList(nID);
	AddWindowToList(nID);
	m_bMoveFocusedWnd = TRUE;
	if (CGameWnd* wnd = FindWindowById(nID))
	{
		wnd->SetGameWndFocused(TRUE);
		wnd->SetReadyMove(ptMouse);
	}
}


VOID CInterface::AllWindowsFocusOff()
{
	if (!m_xWndOrderList.GetCounter()) return;
	m_xWndOrderList.MoveCurrentToTop();
	for (INT i = 0; i < m_xWndOrderList.GetCounter(); ++i)
	{
		if (CGameWnd* wnd = FindWindowById(*m_xWndOrderList.GetCurrentData()))
			wnd->SetGameWndFocused(FALSE);
		m_xWndOrderList.MoveNextNode();
	}
}

BOOL CInterface::IsTopWindow(INT nID)
{
	if ( m_xWndOrderList.GetCounter() != 0 )
	{
		INT* pnCurrID;
		m_xWndOrderList.MoveCurrentToTail();
		pnCurrID = m_xWndOrderList.GetCurrentData();

		if ( nID == *pnCurrID )
			return TRUE;
	}
	return FALSE;
}








///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 消息输入相关部分/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************************************************************************

	函数名 : CInterface::OnLButtonDown()

	作者 :
	编写日期 :

	目的   : 定义 Mouse LButtonDown 发生时接口中要做的工作.
	输入   : POINT ptMouse
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CInterface::OnLButtonDown(POINT ptMouse)
{
	if(m_xMsgBox.IsActive())	// 勇气
	{
		m_xMsgBox.OnButtonDown(ptMouse);
		return TRUE;
	}
	else
	{
		if ( m_xScrlBar.OnLButtonDown(ptMouse) )
		{
			FLOAT	fScrlRate;

			fScrlRate = m_xScrlBar.GetScrlRate();
			m_nFstLine  = (m_xChat.GetCounter()-1)*fScrlRate;

			return TRUE;
		}

		for( INT nCnt = 0; nCnt < _MAX_INTER_BTN; nCnt++ )
		{
			if ( m_xInterBtn[nCnt].OnLButtonDown(ptMouse) )
				return TRUE;
		}

		const INT id = GetWindowInMousePos(ptMouse);
		if (CGameWnd* wnd = FindWindowById(id))
		{
			if (!wnd->OnLButtonDown(&m_stCommonItem, ptMouse))
				WindowFocusChangedAndSetReadyMove(id, ptMouse);
			return TRUE;
		}
		else
			AllWindowsFocusOff();
	}
	return FALSE;
}



/******************************************************************************************************************

	函数名 : CInterface::OnLButtonUp()

	作者   :
	编写日期 :

	目的   : 定义鼠标 LButtonUp 发生时接口要做的处理。
	输入   : POINT ptMouse
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CInterface::OnLButtonUp(POINT ptMouse)
{
	m_bMoveFocusedWnd = FALSE;

	if ( m_xMsgBox.IsActive() )	// 勇气
	{
		HRESULT hResult;
		hResult = m_xMsgBox.OnButtonUp(ptMouse);
		if ( hResult != 0 )
		{
			switch ( m_nIdxMsgBox )
			{
			case 12:
				{
					switch ( hResult )	// 没有编辑框.
					{
					case 2:
						{
						   DWORD dwCurrTick = timeGetTime();
						   if ( (dwCurrTick - g_xGameProc.m_xMyHero.m_dwLastPKStruckTime> 10000) &&
								(dwCurrTick - g_xGameProc.m_xMyHero.m_dwLastMagicTime   > 10000) &&
								(dwCurrTick - g_xGameProc.m_xMyHero.m_dwLastHitTime		> 10000) ||
								(g_xGameProc.m_xMyHero.m_bCurrMtn					== _MT_DIE ) )
								SendMessage(g_xMainWnd.GetSafehWnd(), WM_DESTROY, NULL, NULL);
						   else
						   {
							   DWORD dwFont, dwBack;

							   dwFont = GetChatColor(3);
							   dwBack = GetChatColor(2);

							   MsgAdd(dwFont, dwBack, "You cannot disconnect during combat.");
						   }
						}
						break;
					case 4:
						m_xMsgBox.HideMessageBox();
						break;
					}
				}
				break;
			}
		}
	}
	else
	{
		if ( m_xScrlBar.OnLButtonUp(ptMouse) )
		{
			return TRUE;
		}

		for( INT nCnt = 0; nCnt < _MAX_INTER_BTN; nCnt++ )
		{
			if ( m_xInterBtn[nCnt].OnLButtonUp(ptMouse) )
			{
				switch ( nCnt )
				{
				case _BTN_ID_QUEST:		WindowActivate(_WND_ID_QUEST);					break;
				case _BTN_ID_OPTION:	WindowActivate(_WND_ID_OPTION);					break;
				case _BTN_ID_HORSE:		WindowActivate(_WND_ID_HORSE);					break;

				case _BTN_ID_CHANGE:	WindowActivate(_WND_ID_EXCHANGE);				break;
				case _BTN_ID_GUILD:
					g_xClientSocket.SendGuildHome();
					break;
				case _BTN_ID_GROUP:		WindowActivate(_WND_ID_GROUP);					break;
				case _BTN_ID_POPUP:		WindowActivate(_WND_ID_CHATPOP);				break;
				case _BTN_ID_BAG:		WindowActivate(_WND_ID_INVENTORY);				break;
				case _BTN_ID_MUGONG:	m_bShowKey = !m_bShowKey;						break;
				case _BTN_ID_BELT:
					{
						if ( m_shBeltHeight >= 46 )
						{
							m_shBeltHeight = 46;
						}
						else if ( m_shBeltHeight <= 0 )
						{
							m_shBeltHeight = 0;
						}

						if ( m_shBeltHeight == 46 )
						{
							m_bBeltState = _BELT_STATE_DOWN;
						}
						else if ( m_shBeltHeight == 0 )
						{
							m_bBeltState = _BELT_STATE_UP;
						}
					}
					break;
				case _BTN_ID_EXIT:
					{
						CHAR	  szMsg[MAX_PATH];
						HINSTANCE hLib;
						m_nIdxMsgBox = 12;
						hLib = LoadLibrary("Ats.dll");						// Load Dll
						LoadString(hLib,m_nIdxMsgBox,szMsg, MAX_PATH);
						FreeLibrary(hLib);
						m_xMsgBox.ShowMessageBox(szMsg, 2);					// YES/No ?
					}
					break;
				case _BTN_ID_MAGIC:
					if ( m_xStatusWnd.GetStatusWndType() != _TYPE_SETMAGIC )
					{
						if ( !WindowActivate(_WND_ID_STATUS) )		WindowActivate(_WND_ID_STATUS);
					}
					else
						WindowActivate(_WND_ID_STATUS);
					m_xStatusWnd.SetStatusWndType(_TYPE_SETMAGIC);
					break;
				case _BTN_ID_CHAR:
					if ( m_xStatusWnd.GetStatusWndType() != _TYPE_EQUIP )
					{
						if ( !WindowActivate(_WND_ID_STATUS) )		WindowActivate(_WND_ID_STATUS);
					}
					else
						WindowActivate(_WND_ID_STATUS);
					m_xStatusWnd.SetStatusWndType(_TYPE_EQUIP);
					break;
				}
			}
		}

		const INT id = GetWindowInMousePos(ptMouse);
		if (CGameWnd* wnd = FindWindowById(id))
		{
			if (wnd->OnLButtonUp(&m_stCommonItem, ptMouse))
				WindowActivate(id == _WND_ID_GUILDMASTER ? _WND_ID_EXCHANGE : id);
			return TRUE;
		}
	}
	return FALSE;
}


/******************************************************************************************************************

	函数名 : CInterface::OnMouseMove()

	作者 :
	编写日期 :

	目的   : 当 Mouse Move 发生时, 定义界面中要做的事.
	输入   : POINT ptMouse
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CInterface::OnMouseMove(POINT ptMouse)
{
	MoveTopWindow(ptMouse);

	if ( m_xScrlBar.OnMouseMove(ptMouse) )
	{
		FLOAT	fScrlRate;

		fScrlRate = m_xScrlBar.GetScrlRate();
		m_nFstLine  = (m_xChat.GetCounter()-1)*fScrlRate;
		return TRUE;
	}

	for( INT nCnt = 0; nCnt < _MAX_INTER_BTN; nCnt++ )
		m_xInterBtn[nCnt].OnMouseMove(ptMouse);

	if (CGameWnd* wnd = FindWindowById(GetWindowInMousePos(ptMouse)))
	{
		wnd->OnMouseMove(ptMouse);
		return TRUE;
	}
	else
	{
		m_xStatusWnd.SetStatusBtnInit();
		m_xInventoryWnd.SetInvenBtnInit();
		m_xChatPopWnd.SetStatusBtnInit();
		m_xGuildMasterWnd.SetStatusBtnInit();
		m_xOptionWnd.SetStatusBtnInit();
		m_xQuestWnd.SetStatusBtnInit();
		m_xHorseWnd.SetStatusBtnInit();
		m_xExchangeWnd.SetStatusBtnInit();
	}

	return FALSE;
}


BOOL CInterface::OnScrollDown()
{
	if ( IsTopWindow(m_xInventoryWnd.m_nID) )
	{
		m_xInventoryWnd.OnScrollDown();
	}
	else if ( IsTopWindow(m_xStatusWnd.m_nID) )
	{
		m_xStatusWnd.OnScrollDown();
	}
	else if (IsTopWindow(m_xNPCWnd.m_nID))		// 勇气
	{
		m_xNPCWnd.OnScrollDown();
	}
	else
	{
		if ( !m_xChatPopWnd.m_bActive )
		{
			if ( m_nFstLine > 0 )
			{
				m_nFstLine--;
			}
		}
		else
		{
			m_xChatPopWnd.OnScrollDown();
		}
	}
	return TRUE;
}


BOOL CInterface::OnScrollUp()
{
	if ( IsTopWindow(m_xInventoryWnd.m_nID) )
	{
		m_xInventoryWnd.OnScrollUp();
	}
	else if ( IsTopWindow(m_xStatusWnd.m_nID) )
	{
		m_xStatusWnd.OnScrollUp();
	}
	else if (IsTopWindow(m_xNPCWnd.m_nID))		// 勇气
	{
		m_xNPCWnd.OnScrollUp();
	}
	else
	{
		if ( !m_xChatPopWnd.m_bActive )
		{
			if ( m_nFstLine < (m_xChat.GetCounter()-1) )
			{
				m_nFstLine++;
			}
		}
		else
		{
			m_xChatPopWnd.OnScrollUp();
		}
	}
	return TRUE;
}

BOOL CInterface::OnLButtonDoubleClick(POINT ptMouse)
{
	if ( IsTopWindow(m_xInventoryWnd.m_nID) )
		m_xInventoryWnd.OnLButtonDoubleClick(&m_stCommonItem, ptMouse);
	return TRUE;
}


BOOL CInterface::OnKeyUp(WPARAM wParam, LPARAM lParam)
{
	if ( m_xInventoryWnd.OnKeyUp(&m_stCommonItem, wParam, lParam) )
	{
		return TRUE;
	}
	return FALSE;
}


BOOL CInterface::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	switch ( wParam )
	{
	case VK_RETURN:
		break;
	case VK_F9:
		WindowActivate(_WND_ID_INVENTORY);
		break;
	case VK_F11:
		if ( m_xStatusWnd.GetStatusWndType() != _TYPE_SETMAGIC )
		{
			if ( !WindowActivate(_WND_ID_STATUS) )
				WindowActivate(_WND_ID_STATUS);
		}
		else
			WindowActivate(_WND_ID_STATUS);

		m_xStatusWnd.SetStatusWndType(_TYPE_SETMAGIC);
		break;
	}

	INT nInterfaceWndInRect;
	nInterfaceWndInRect = GetWindowInMousePos(g_xGameProc.m_ptMousePos);

	if ( nInterfaceWndInRect != -1 )
	{
		if ( nInterfaceWndInRect == _WND_ID_STATUS )
		{
			if ( m_xStatusWnd.OnKeyDown(wParam, lParam, g_xGameProc.m_ptMousePos) )
				return TRUE;
		}
	}

	return FALSE;
}


VOID CInterface::OnSysKeyDown()
{
	// F10 Key 被按下时。
	if ( m_xStatusWnd.GetStatusWndType() != _TYPE_EQUIP )
	{
		if ( !WindowActivate(_WND_ID_STATUS) )
			WindowActivate(_WND_ID_STATUS);
	}
	else
		WindowActivate(_WND_ID_STATUS);

	m_xStatusWnd.SetStatusWndType(_TYPE_EQUIP);
}


LRESULT	CInterface::OnMsgInputted(WPARAM wParam, LPARAM lParam)
{
	if ( m_xNPCWnd.OnMsgInputted() )
		return TRUE;

	return 0L;
}

// BMWnd
#define WND_WIDTH		640
#define WND_HEIGHT		480

CBMWnd::CBMWnd()
{
	m_fIsActive = FALSE;
	m_fIsFocused = FALSE;
	SetRect(&m_rcWnd,0,0,0,0);
}

CBMWnd::~CBMWnd()
{

}

VOID CBMWnd::OpenWnd(VOID)
{
	m_fIsActive = TRUE;
}

VOID CBMWnd::CloseWnd(VOID)
{
	m_fIsActive = FALSE;
}

HRESULT CBMWnd::OnButtonDown(WPARAM wParam, LPARAM lParam)
{
	POINT	ptMouse;
	ptMouse.x = LOWORD(lParam);
	ptMouse.y = HIWORD(lParam);

	return OnButtonDown(ptMouse);
}

HRESULT CBMWnd::OnButtonDown(POINT ptMouse)
{

	return 0;
}

HRESULT	CBMWnd::OnButtonUp(WPARAM wParam, LPARAM lParam)
{
	POINT	ptMouse;
	ptMouse.x = LOWORD(lParam);
	ptMouse.y = HIWORD(lParam);

	return OnButtonUp(ptMouse);
}

HRESULT CBMWnd::OnButtonUp(POINT ptMouse)
{
	return 0;
}

HRESULT CBMWnd::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	return 0;
}


LRESULT CBMWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	return 0;
}


VOID CBMWnd::Render(INT nLoopTime)
{
}

#undef WND_WIDTH
#undef WND_HEIGHT

// LoginAvi
#define _RENDER_AVI_NONE		0
#define _RENDER_AVI_INTRO		1
#define _RENDER_AVI_LOGO		2
#define _RENDER_AVI_STILL		9

#define _LEFT_INTRO				0
#define _TOP_INTRO				60
#define _RIGHT_INTRO			640
#define _BOTTOM_INTRO			420

#define _INTRO_FILE_NAME		".\\ei_login.avi"
#define _LOGO_FILE_NAME			".\\ei_logo.avi"

CLoginAvi::CLoginAvi():CBMWnd()
{
	m_pxImage			= NULL;

	m_nRenderAviState	= _RENDER_AVI_NONE;

	SetRect(&m_rcWnd,0,0,640,480);
	SetRect(&m_rcSrc,_LEFT_INTRO,_TOP_INTRO,_RIGHT_INTRO,_BOTTOM_INTRO);
}

CLoginAvi::~CLoginAvi()
{
	StopAllAvis();
	m_xAvi.ReleaseAvi();
	m_xLogoAvi.ReleaseAvi();
}

VOID CLoginAvi::Create(CWHWilImageData* pxImage)
{
	m_pxImage = pxImage;
	m_xAvi.InitAvi(g_xSound.GetSoundObject()->GetDS());
	m_xLogoAvi.InitAvi(g_xSound.GetSoundObject()->GetDS());
	m_xAvi.Create(_INTRO_FILE_NAME,FALSE);
	m_xLogoAvi.Create(_LOGO_FILE_NAME,FALSE);
	m_xAvi.SetDestRect(m_rcSrc);
	m_xLogoAvi.SetDestRect(m_rcSrc);

}

VOID CLoginAvi::SetRenderAviState(INT nState)
{
	switch(m_nRenderAviState)
	{
		case _RENDER_AVI_INTRO:
		{
			m_xAvi.Stop();
			break;
		}
		case _RENDER_AVI_LOGO:
		{
			m_xLogoAvi.Stop();
			break;
		}
	}
	m_nRenderAviState = nState;
	switch(m_nRenderAviState)
	{
		case _RENDER_AVI_INTRO:
		{
			m_xAvi.Start();
			break;
		}
		case _RENDER_AVI_LOGO:
		{
			m_xLogoAvi.Start();
			break;
		}
	}
}

HRESULT CLoginAvi::OnButtonDown(WPARAM wParam, LPARAM lParam)
{
	return 0;
}


HRESULT CLoginAvi::OnButtonUp(WPARAM wParam, LPARAM lParam)
{
	return 0;
}



VOID CLoginAvi::Render(INT	nLoopTime)
{
	switch(m_nRenderAviState)
	{
		case _RENDER_AVI_INTRO:
		{	if(!m_xAvi.Draw(nLoopTime))
				m_nRenderAviState = _RENDER_AVI_STILL;
			break;
		}
		case _RENDER_AVI_LOGO:
		{
//			if(!m_xLogoAvi.DrawBlend(nLoopTime))
				m_nRenderAviState = _RENDER_AVI_STILL;
			break;
		}
		case _RENDER_AVI_STILL:
		{	m_pxImage->NewSetIndex(IMG_IDX_LOGIN_BACK);
			g_xMainWnd.DrawWithImageForComp(_LEFT_INTRO, _TOP_INTRO,
											m_pxImage->m_lpstNewCurrWilImageInfo->shWidth,
											m_pxImage->m_lpstNewCurrWilImageInfo->shHeight,
											(WORD*)(m_pxImage->m_pbCurrImage));
			break;
		}
	}
}


VOID CLoginAvi::StopAllAvis(VOID)
{
	m_xAvi.Stop();
	m_xLogoAvi.Stop();
}

#undef _RENDER_AVI_NONE
#undef _RENDER_AVI_INTRO
#undef _RENDER_AVI_LOGO
#undef _RENDER_AVI_STILL
#undef _LEFT_INTRO
#undef _TOP_INTRO
#undef _RIGHT_INTRO
#undef _BOTTOM_INTRO
#undef _INTRO_FILE_NAME
#undef _LOGO_FILE_NAME

// Login
#define LOGIN_BUTTON_COUNT			4
#define LOGIN_INPUT_COUNT			2
#define LOGIN_IMAGE_COUNT			4

#define POS_ID_INS_X				193
#define POS_ID_INS_Y				435
#define POS_PASS_INS_X				390
#define POS_PASS_INS_Y				435

#define _LEFT_LOGIN					0
#define _TOP_LOGIN					372
#define _RIGHT_LOGIN				640
#define _BOTTOM_LOGIN				480

#define _LEFT_ID_PASS				170
#define _TOP_ID_PASS				65

#define _IMG_IDX_LOGIN_BORDER		1
#define _IMG_IDX_LOGIN_IDPASS		2
#define _IMG_IDX_BTN_LOGIN			10
#define _IMG_IDX_BTN_NEW			12
#define _IMG_IDX_BTN_CHANGE			14
#define _IMG_IDX_BTN_EXIT			16


#define _LEFT_LOGIN_BTN				140
#define _LEFT_NEW_BTN				205
#define _LEFT_CHANGE_BTN			325
#define _LEFT_EXIT_BTN				475
#define _TOP_LOGIN_BTNS				8


CLogin::CLogin():CBMWnd()
{

}

CLogin::~CLogin()
{

}

VOID CLogin::Create(CWHWilImageData* pxImage)
{
	INT	nLoop;
	// Buttons
	BUTTONINFO LoginProcBtnInfo[] = {
									{ _IMG_IDX_BTN_LOGIN,		_LEFT_LOGIN_BTN,	_TOP_LOGIN_BTNS,			0,		0}, // Login Button
									{ _IMG_IDX_BTN_NEW,			_LEFT_NEW_BTN,		_TOP_LOGIN_BTNS,			0,		0},	// New Account
									{ _IMG_IDX_BTN_CHANGE,		_LEFT_CHANGE_BTN,	_TOP_LOGIN_BTNS,			0,		0},	// Change Password
									{ _IMG_IDX_BTN_EXIT,		_LEFT_EXIT_BTN,		_TOP_LOGIN_BTNS,			0,		0}	// Exit
									};
	// EditBox State
	INPUTSTATE LoginInputState[] =  {
									{ LGM_INPUT_ID,				POS_ID_INS_X,		POS_ID_INS_Y,			100,				15,			0,				10,				"\0"},			// Input ID in Login
									{ LGM_INPUT_PASSWORD,		POS_PASS_INS_X,		POS_PASS_INS_Y,			100,				15,			0,				10,				"\0"}			// Input Password In Login
									};

	m_pxImage		= pxImage;


	for(nLoop = 0 ; nLoop < LOGIN_INPUT_COUNT ; nLoop ++)
		memcpy(&m_xInputState[nLoop],&LoginInputState[nLoop],sizeof(INPUTSTATE));

	for(nLoop = 0 ; nLoop < LOGIN_BUTTON_COUNT ; nLoop ++)
		m_xButtons[nLoop].SetBtn(&LoginProcBtnInfo[nLoop]);

	m_fShowIDPASS = FALSE;

	ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
	SetFocus(g_xMainWnd.GetSafehWnd());

	SetRect(&m_rcWnd,_LEFT_LOGIN,_TOP_LOGIN,_RIGHT_LOGIN,_BOTTOM_LOGIN);
}

HRESULT	CLogin::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	if(m_fShowIDPASS)
	{
		if (wParam == VK_RETURN || wParam == VK_TAB)
		{	SetFocusBefore();
			switch(m_nUserState)
			{	case  LGM_INPUT_ID:
				{	m_nUserState = LGM_INPUT_PASSWORD;
					break;
				}
				case LGM_INPUT_PASSWORD:
				{	if( ( lstrlen( m_xInputState[LGM_INPUT_ID-1].szData) >= LIMIT_USERID )
									&& lstrlen( m_xInputState[LGM_INPUT_PASSWORD-1].szData ) )
					{	g_xClientSocket.OnLogin(m_xInputState[LGM_INPUT_ID-1].szData,m_xInputState[LGM_INPUT_PASSWORD-1].szData);
						strcpy(g_szUserID,m_xInputState[LGM_INPUT_ID-1].szData);
						return 0L;
					}
					m_nUserState = LGM_INPUT_ID;
					break;
				}
			}
			SetFocusAfter();
		}
	}
	return 0;
}

HRESULT CLogin::OnButtonDown(WPARAM wParam, LPARAM lParam)
{
	INT i;
	RECT tRect;

	m_fIsButtonDown = TRUE;

	if(m_fShowIDPASS)
		for( i = LGM_INPUT_ID-1 ; i < LGM_INPUT_PASSWORD; i ++)
		{	SetRect(&tRect
					,m_xInputState[i].Left,m_xInputState[i].Top
					,m_xInputState[i].Left+m_xInputState[i].Width,m_xInputState[i].Top+m_xInputState[i].Height);
			if( IsInRect( tRect, LOWORD( lParam ), HIWORD( lParam ) ) )
			{	SetFocusBefore();
				m_nUserState = i+1;
				SetFocusAfter();
			}
		}
	return 0;
}

HRESULT CLogin::OnButtonDown(POINT ptMouse)
{
	m_fIsButtonDown = TRUE;
	return 0;
}

HRESULT	CLogin::OnButtonUp(WPARAM wParam, LPARAM lParam)
{
	INT		i;
	RECT	tRect;

	m_fIsButtonDown = FALSE;

	for(i = BTN_LOGIN_ID; i <= BTN_EXIT_ID; i++)
	{//	m_xButtons[i].m_nState = BUTTON_STATE_UP;
		m_pxImage->NewSetIndex(m_xButtons[i].m_nButtonID + m_xButtons[i].m_nState - 1);
		SetRect(&tRect
				,m_xButtons[i].m_Rect.left + m_rcWnd.left ,m_xButtons[i].m_Rect.top + m_rcWnd.top
				,m_xButtons[i].m_Rect.left + m_rcWnd.left + m_pxImage->m_lpstNewCurrWilImageInfo->shWidth
				,m_xButtons[i].m_Rect.top  + m_rcWnd.top + m_pxImage->m_lpstNewCurrWilImageInfo->shHeight);
		if (IsInRect(tRect,LOWORD( lParam ), HIWORD( lParam ) ) )
		{
			switch(m_xButtons[i].m_nButtonID)
			{	case _IMG_IDX_BTN_LOGIN:
				{
					m_fShowIDPASS = !m_fShowIDPASS;
					break;
				}
			case _IMG_IDX_BTN_NEW:
				{
					m_fShowIDPASS = FALSE;

					break;
				}
			case _IMG_IDX_BTN_CHANGE:
				{
					m_fShowIDPASS = FALSE;

					break;
				}
			case _IMG_IDX_BTN_EXIT:
				{
					m_fShowIDPASS = FALSE;
					SendMessage(g_xMainWnd.GetSafehWnd(), WM_DESTROY, NULL, NULL);
					return 0L;break;
				}
			}// switch
		}// if
	}// for
	if(m_fShowIDPASS)
	{	ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_SHOW);
		SetFocus(g_xChatEditBox.GetSafehWnd());
	}
		else
	{
		ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
		SetFocus(g_xMainWnd.GetSafehWnd());
	}
	return 0;
}


LRESULT	CLogin::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	INT	i;
	RECT tRect;

	if(!m_fIsButtonDown)
	{
		for( i = BTN_LOGIN_ID ; i <= BTN_EXIT_ID ; i ++)
		{	m_pxImage->NewSetIndex(m_xButtons[i].m_nButtonID + m_xButtons[i].m_nState - 1);
			SetRect(&tRect
					,m_xButtons[i].m_Rect.left + m_rcWnd.left ,m_xButtons[i].m_Rect.top + m_rcWnd.top
					,m_xButtons[i].m_Rect.left + m_rcWnd.left + m_pxImage->m_lpstNewCurrWilImageInfo->shWidth
					,m_xButtons[i].m_Rect.top  + m_rcWnd.top + m_pxImage->m_lpstNewCurrWilImageInfo->shHeight);
			if (IsInRect(tRect,LOWORD( lParam ), HIWORD( lParam ) ) )
				m_xButtons[i].m_nState = BUTTON_STATE_ON;
			else
				m_xButtons[i].m_nState = BUTTON_STATE_UP;
		}
	}

	return 0;
}

VOID CLogin::Render(INT	nLoopTime)
{
	int i;
	char Pass[16]="";

	if(m_fIsActive)
	{

		MoveWindow(g_xChatEditBox.GetSafehWnd(),
					g_xMainWnd.m_rcWindow.left + m_xInputState[m_nUserState-1].Left +5,
					g_xMainWnd.m_rcWindow.top + m_xInputState[m_nUserState-1].Top + 5 ,	m_xInputState[m_nUserState-1].Width,		m_xInputState[m_nUserState-1].Height,		TRUE);

		ShowWindow(g_xChatEditBox.GetSafehWnd(), (m_fShowIDPASS ? SW_SHOW:SW_HIDE));
		// Draw Login Border
		if(!m_fShowIDPASS)
		{	m_pxImage->NewSetIndex(_IMG_IDX_LOGIN_BORDER);
			g_xMainWnd.DrawWithABlendCompDataWithBackBuffer(m_rcWnd.left ,m_rcWnd.top ,
															m_pxImage->m_lpstNewCurrWilImageInfo->shWidth ,m_pxImage->m_lpstNewCurrWilImageInfo->shHeight ,(WORD*)m_pxImage->m_pbCurrImage,640,480);
			// Draw Button Image
			for ( i = BTN_LOGIN_ID ; i <=  BTN_EXIT_ID ; i++)
			{
					m_pxImage->NewSetIndex(m_xButtons[i].m_nButtonID + m_xButtons[i].m_nState - 1);
					g_xMainWnd.DrawWithImageForComp(m_rcWnd.left + m_xButtons[i].m_Rect.left, m_rcWnd.top + m_xButtons[i].m_Rect.top,
												m_pxImage->m_lpstNewCurrWilImageInfo->shWidth,
												m_pxImage->m_lpstNewCurrWilImageInfo->shHeight, (WORD*)(m_pxImage->m_pbCurrImage));
			}
		}
		// Draw ID & Pass Image
		if(m_fShowIDPASS)
		{	m_pxImage->NewSetIndex(_IMG_IDX_LOGIN_IDPASS);
			g_xMainWnd.DrawWithImageForComp(_LEFT_ID_PASS,m_rcWnd.top + _TOP_ID_PASS,
												m_pxImage->m_lpstNewCurrWilImageInfo->shWidth ,m_pxImage->m_lpstNewCurrWilImageInfo->shHeight,(WORD*)m_pxImage->m_pbCurrImage);
			memset(Pass,'*',strlen(m_xInputState[1].szData));
			g_xMainWnd.PutsHan(NULL, m_xInputState[0].Left+7, m_xInputState[0].Top+6, RGB(255,255,255), RGB(0,0,0), m_xInputState[0].szData);
			g_xMainWnd.PutsHan(NULL, m_xInputState[1].Left+7, m_xInputState[1].Top+6, RGB(255,255,255), RGB(0,0,0), Pass);
		}
	}
}


VOID CLogin::SetFocusBefore(VOID)
{
	INT nTemp;

	nTemp =  m_nUserState - 1;

	if(g_xChatEditBox.m_szInputMsg[0]!=NULL)
			lstrcpy(m_xInputState[nTemp].szData , g_xChatEditBox.m_szInputMsg);
		else
			GetWindowText(g_xChatEditBox.GetSafehWnd(),m_xInputState[nTemp].szData ,m_xInputState[nTemp].nSize);

	ZeroMemory(g_xChatEditBox.m_szInputMsg,sizeof(g_xChatEditBox.m_szInputMsg));
	ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
	g_xChatEditBox.SetSelectAll();
}

VOID CLogin::SetFocusAfter(VOID)
{
	CHAR	cChr;
	INT		nTemp;

	ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_SHOW);

	nTemp = m_nUserState - 1;

	g_xChatEditBox.SetLimitText(m_xInputState[nTemp].nSize);

	if( m_nUserState != LGM_INPUT_PASSWORD )
		cChr = NULL;
	else
		cChr = '*';

	SendMessage(g_xChatEditBox.GetSafehWnd(),EM_SETPASSWORDCHAR,(WPARAM)cChr,0);
	SetWindowText(g_xChatEditBox.GetSafehWnd(), m_xInputState[nTemp].szData);

	SetFocus(g_xChatEditBox.GetSafehWnd());
	g_xChatEditBox.SetSelectAll();
}

#undef LOGIN_BUTTON_COUNT
#undef LOGIN_INPUT_COUNT
#undef LOGIN_IMAGE_COUNT
#undef POS_ID_INS_X
#undef POS_ID_INS_Y
#undef POS_PASS_INS_X
#undef POS_PASS_INS_Y
#undef _LEFT_LOGIN
#undef _TOP_LOGIN
#undef _RIGHT_LOGIN
#undef _BOTTOM_LOGIN
#undef _LEFT_ID_PASS
#undef _TOP_ID_PASS
#undef _IMG_IDX_LOGIN_BORDER
#undef _IMG_IDX_LOGIN_IDPASS
#undef _IMG_IDX_BTN_LOGIN
#undef _IMG_IDX_BTN_NEW
#undef _IMG_IDX_BTN_CHANGE
#undef _IMG_IDX_BTN_EXIT
#undef _LEFT_LOGIN_BTN
#undef _LEFT_NEW_BTN
#undef _LEFT_CHANGE_BTN
#undef _LEFT_EXIT_BTN
#undef _TOP_LOGIN_BTNS

// NewAccount
#define LOGIN_BUTTON_COUNT			3
#define LOGIN_INPUT_COUNT			2
#define LOGIN_IMAGE_COUNT			4



CNewAccount::CNewAccount()
{

}

CNewAccount::~CNewAccount()
{

}

VOID CNewAccount::Create(CWHWilImageData* pxImage)
{
/*	INT	nLoop;
	// Buttons
	BUTTONINFO LoginProcBtnInfo[] = {
									{ IMG_IDX_BTN_NEW,		POS_BTN_NEW_X,		POS_BTN_NEW_Y,			LOGBTN_WIDTH,		LOGBTN_HEIGHT}, // OK Button
									{ IMG_IDX_BTN_CHGPASS,	POS_BTN_CHG_X,		POS_BTN_CHG_Y,			LOGBTN_WIDTH,		LOGBTN_HEIGHT},	// New Account
									{ IMG_IDX_BTN_OK,		POS_BTN_OK_X,		POS_BTN_OK_Y,			LOGBTN_WIDTH,		LOGBTN_HEIGHT}	// Change Password
									};
	// EditBox State
	INPUTSTATE LoginInputState[] =  {
									{ LGM_INPUT_ID,				POS_ID_INS_X,		POS_ID_INS_Y,			126,				15,			0,				10,				"\0"},			// Input ID in Login
									{ LGM_INPUT_PASSWORD,		POS_PASS_INS_X,		POS_PASS_INS_Y,			126,				15,			0,				10,				"\0"}			// Input Password In Login
									};

	_LOGINIMAGEINFO LoginImageInfo[]	=	{
											{IMG_IDX_ID,		POS_ID_TITLE_X,		POS_ID_TITLE_Y},
											{IMG_IDX_PASS,		POS_PASS_TITLE_X,	POS_PASS_TITLE_Y},
											{IMG_IDX_ID_INS,	POS_ID_INS_X,		POS_ID_INS_Y},
											{IMG_IDX_PASS_INS,	POS_PASS_INS_X,		POS_PASS_INS_Y}
										};

	m_pxImage		= pxImage;


	for(nLoop = 0 ; nLoop < LOGIN_INPUT_COUNT ; nLoop ++)
		memcpy(&m_xInputState[nLoop],&LoginInputState[nLoop],sizeof(INPUTSTATE));
	for(nLoop = 0 ; nLoop < LOGIN_BUTTON_COUNT ; nLoop ++)
		m_xButtons[nLoop].SetBtn(&LoginProcBtnInfo[nLoop]);
	for(nLoop = 0 ; nLoop < LOGIN_IMAGE_COUNT ; nLoop ++)
		m_xInputImg[nLoop] = LoginImageInfo[nLoop];*/
}


HRESULT CNewAccount::OnButtonDown(WPARAM wParam, LPARAM lParam)
{
/*	INT i;
	RECT tRect;

	m_fIsButtonDown = TRUE;

	for( i = LGM_INPUT_ID-1 ; i < LGM_INPUT_PASSWORD; i ++)
	{	SetRect(&tRect
				,m_xInputState[i].Left,m_xInputState[i].Top
				,m_xInputState[i].Left+m_xInputState[i].Width,m_xInputState[i].Top+m_xInputState[i].Height);
		if( IsInRect( tRect, LOWORD( lParam ), HIWORD( lParam ) ) )
		{	SetFocusBefore();
			m_nUserState = i+1;
			SetFocusAfter();
		}

	}

	// Image Button Down Check
	for ( i = BTN_NEW_ID; i <= BTN_OK_ID ; i++)
	{	if (m_xButtons[i].CheckMouseOn( LOWORD( lParam ), HIWORD( lParam ) ) )
			m_xButtons[i].m_nState = BUTTON_STATE_DOWN;
		else
			m_xButtons[i].m_nState = BUTTON_STATE_UP;
	}*/
	return 0;
}

HRESULT CNewAccount::OnButtonDown(POINT ptMouse)
{
	m_fIsButtonDown = TRUE;
	return 0;
}

HRESULT	CNewAccount::OnButtonUp(WPARAM wParam, LPARAM lParam)
{
/*	INT	i;

	m_fIsButtonDown = FALSE;

	for(i = BTN_NEW_ID; i <= BTN_OK_ID; i++)
	{	m_xButtons[i].m_nState = BUTTON_STATE_UP;

		if( m_xButtons[i].CheckMouseOn( LOWORD( lParam ), HIWORD( lParam ) ) )
		{	switch(m_xButtons[i].m_nButtonID)
			{	case IMG_IDX_BTN_NEW:
				{
					//m_nProgress = PRG_NEW_ACCOUNT;	- > 切换到 New Account 的函数
					SetFocusBefore();
					SetWindowText(g_xChatEditBox.GetSafehWnd(),NULL);
					m_nUserState = LGM_INPUT_USER_ID;
					SetFocusAfter();
					break;
				}
			case IMG_IDX_BTN_CHGPASS:
				{	// 临时作为退出按钮使用
					//	结束 Avi 相关函数的代码
					//	break;
				}
//						case BTN_EXIT:
				{
					SendMessage(g_xMainWnd.GetSafehWnd(), WM_DESTROY, NULL, NULL);
					return 0L;break;
				}
			case IMG_IDX_BTN_OK:
				{
					SendMessage(g_xChatEditBox.GetSafehWnd(),WM_CHAR,VK_RETURN,0);
					g_xClientSocket.OnLogin(m_xInputState[LGM_INPUT_ID-1].szData,m_xInputState[LGM_INPUT_PASSWORD-1].szData);
					break;
				}
			}// switch
		}// if
	}// for*/
	return 0;
}



VOID CNewAccount::Render(INT	nLoopTime)
{
/*	if(m_fIsActive)
	{
		int i;
		char Pass[16]="";

		MoveWindow(g_xChatEditBox.GetSafehWnd(),
					g_xMainWnd.m_rcWindow.left + m_xInputState[m_nUserState-1].Left +5,
					g_xMainWnd.m_rcWindow.top + m_xInputState[m_nUserState-1].Top + 5 ,	m_xInputState[m_nUserState-1].Width,		m_xInputState[m_nUserState-1].Height,		TRUE);


		// Draw ID & Pass Image
		for( i = IMG_IDX_ID-1 ; i < IMG_IDX_PASS_INS ; i ++)
		{
			m_pxImage->NewSetIndex(m_xInputImg[i].nImgIdx);
			g_xMainWnd.DrawWithImageForComp(m_xInputImg[i].Left , m_xInputImg[i].Top,
						m_pxImage->m_lpstNewCurrWilImageInfo->shWidth,
						m_pxImage->m_lpstNewCurrWilImageInfo->shHeight,
						(WORD*)(m_pxImage->m_pbCurrImage));
		}

		// Draw Button Image
		for ( i = BTN_NEW_ID ; i <=  BTN_OK_ID ; i++)
		{
				m_pxImage->NewSetIndex(m_xButtons[i].m_nButtonID + m_xButtons[i].m_nState - 1 );
				g_xMainWnd.DrawWithImageForComp(m_xButtons[i].m_Rect.left, m_xButtons[i].m_Rect.top,
											m_xButtons[i].m_Rect.right - m_xButtons[i].m_Rect.left,
											m_xButtons[i].m_Rect.bottom - m_xButtons[i].m_Rect.top, (WORD*)(m_pxImage->m_pbCurrImage));
		}

		// Write ID & Password
		memset(Pass,'*',strlen(m_xInputState[1].szData));
		g_xMainWnd.PutsHan(NULL, m_xInputState[0].Left+7, m_xInputState[0].Top+6, RGB(5,5,5), RGB(0,0,0), m_xInputState[0].szData);
		g_xMainWnd.PutsHan(NULL, m_xInputState[1].Left+7, m_xInputState[1].Top+6, RGB(5,5,5), RGB(0,0,0), Pass);
	}*/
}


VOID CNewAccount::SetFocusBefore(VOID)
{
	INT nTemp;

	nTemp =  m_nUserState - 1;

	if(g_xChatEditBox.m_szInputMsg[0]!=NULL)
			lstrcpy(m_xInputState[nTemp].szData , g_xChatEditBox.m_szInputMsg);
		else
			GetWindowText(g_xChatEditBox.GetSafehWnd(),m_xInputState[nTemp].szData ,m_xInputState[nTemp].nSize);

	ZeroMemory(g_xChatEditBox.m_szInputMsg,sizeof(g_xChatEditBox.m_szInputMsg));
	ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
	g_xChatEditBox.SetSelectAll();
}

VOID CNewAccount::SetFocusAfter(VOID)
{
	CHAR	cChr;
	INT		nTemp;

	ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_SHOW);

	nTemp = m_nUserState - 1;

	g_xChatEditBox.SetLimitText(m_xInputState[nTemp].nSize);

	if( m_nUserState != LGM_INPUT_PASSWORD )
		cChr = NULL;
	else
		cChr = '*';

	SendMessage(g_xChatEditBox.GetSafehWnd(),EM_SETPASSWORDCHAR,(WPARAM)cChr,0);
	SetWindowText(g_xChatEditBox.GetSafehWnd(), m_xInputState[nTemp].szData);

	SetFocus(g_xChatEditBox.GetSafehWnd());
	g_xChatEditBox.SetSelectAll();
}

#undef LOGIN_BUTTON_COUNT
#undef LOGIN_INPUT_COUNT
#undef LOGIN_IMAGE_COUNT

// SelectSrv
#define POS_SERVERLIST_BORDER_X		50
#define POS_SERVERLIST_BORDER_Y		0
#define SERVERLIST_BORDER_WIDTH		151

#define POS_TOP_SERVER_BTN_X		0
#define POS_TOP_SERVER_BTN_Y		10

#define SERVER_BTN_WIDTH			100
#define SERVER_BTN_HEIGHT			20
#define	SERVER_BTN_GAP				10
#define COUNT_BUTTON_PER_COLUME		8

#define SERVER_BTN_TEXTCOLOR_NORMAL	RGB(10, 10, 10)			// 暗色
#define SERVER_BTN_TEXTCOLOR_FOCUS	RGB(0, 255, 255)		// YELLOW
#define SERVER_BTN_TEXTCOLOR_CLICK	RGB(0,0,0)				// COLORKEY

#define SERVER_BTN_BKCOLOR_NORMAL	RGB(125,125,125)				//
#define SERVER_BTN_BKCOLOR_FOCUS	RGB(95,95,95)				//
#define SERVER_BTN_BKCOLOR_CLICK	RGB(125,125,0)			//  YELLOW

#define _LEFT_BORDER				20
#define _TOP_BORDER					60
#define _RIGHT_BORDER				120
#define _BOTTOM_BORDER				310

CSelectSrv::CSelectSrv():CBMWnd()
{
	m_ppxSrvBtn = NULL;
	D3DVECTOR vNorm(0, 0, -1);
    m_avBillBoard[0]  = D3DVERTEX(D3DVECTOR(-0.5f, 0.5f, 0), vNorm, 0, 0);
    m_avBillBoard[1]  = D3DVERTEX(D3DVECTOR(-0.5f,-0.5f, 0), vNorm, 0, 1);
    m_avBillBoard[2]  = D3DVERTEX(D3DVECTOR( 0.5f, 0.5f, 0), vNorm, 1, 0);
    m_avBillBoard[3]  = D3DVERTEX(D3DVECTOR( 0.5f,-0.5f, 0), vNorm, 1, 1);
}

CSelectSrv::~CSelectSrv()
{
	INT	i;
	for(i = 0 ; i <m_nSrvCount; i++)
		delete m_ppxSrvBtn[i];
	delete[] m_ppxSrvBtn;
}

VOID CSelectSrv::Create(CWHWilImageData* pxImage,CPDLList<ServerList>* pxSList)
{
	INT		i,j;
	INT		nTop,nLeft;
	RECT	rcTemp;
	// Buttons
	m_nSrvCount = pxSList->GetCounter() + 1;
	m_ppxSrvBtn	= new _PSERVERBUTTON[m_nSrvCount];
	m_pxImage		= pxImage;

	pxSList->MoveCurrentToTop();
	for(i = 0 ; i <= ((m_nSrvCount - 1) / COUNT_BUTTON_PER_COLUME) ; i++)
	{	for(j = i * COUNT_BUTTON_PER_COLUME; (j < m_nSrvCount && j < (COUNT_BUTTON_PER_COLUME * (i + 1))) ; j ++)
		{	m_ppxSrvBtn[j] = new _SERVERBUTTON;

			strcpy(m_ppxSrvBtn[j]->szName, pxSList->GetCurrentData()->Name);
			strcpy(m_ppxSrvBtn[j]->szCaption, pxSList->GetCurrentData()->Caption);
			nLeft = POS_TOP_SERVER_BTN_X + i * 100;
			nTop  = POS_TOP_SERVER_BTN_Y + (j - i * COUNT_BUTTON_PER_COLUME) * (SERVER_BTN_HEIGHT+SERVER_BTN_GAP);
			SetRect(&rcTemp, nLeft, nTop, nLeft + SERVER_BTN_WIDTH, nTop + SERVER_BTN_HEIGHT);
			m_ppxSrvBtn[j]->rcBtn = rcTemp;
			m_ppxSrvBtn[j]->dwTxtColor = SERVER_BTN_TEXTCOLOR_NORMAL;
			m_ppxSrvBtn[j]->dwBkColor = SERVER_BTN_BKCOLOR_NORMAL;
			m_ppxSrvBtn[j]->nAlpha = 125;
			pxSList->MoveNextNode();

		}
	}
	SetRect(&m_rcWnd,_LEFT_BORDER,_TOP_BORDER,_LEFT_BORDER+SERVER_BTN_WIDTH*(m_nSrvCount/COUNT_BUTTON_PER_COLUME+1),(m_nSrvCount/COUNT_BUTTON_PER_COLUME) > 0 ? _BOTTOM_BORDER : POS_TOP_SERVER_BTN_Y + (m_nSrvCount * COUNT_BUTTON_PER_COLUME) * (SERVER_BTN_HEIGHT+SERVER_BTN_GAP));
}


HRESULT CSelectSrv::OnButtonDown(WPARAM wParam, LPARAM lParam)
{
	INT i;
	RECT	tRect;
	m_fIsButtonDown = TRUE;

	for( i = 0 ;  i < m_nSrvCount  ; i++)
	{	SetRect(&tRect,m_rcWnd.left + m_ppxSrvBtn[i]->rcBtn.left, m_rcWnd.top + m_ppxSrvBtn[i]->rcBtn.top, m_rcWnd.left +m_ppxSrvBtn[i]->rcBtn.right ,m_rcWnd.top +m_ppxSrvBtn[i]->rcBtn.bottom);
		if(IsInRect(tRect,LOWORD(lParam), HIWORD(lParam)))
		{	m_ppxSrvBtn[i]->dwTxtColor	= SERVER_BTN_TEXTCOLOR_CLICK;			// COLOR(BLACK)
			m_ppxSrvBtn[i]->dwBkColor	= SERVER_BTN_BKCOLOR_CLICK;
			m_ppxSrvBtn[i]->nAlpha		= 125;
		}
	}

	return 0;
}

HRESULT CSelectSrv::OnButtonDown(POINT ptMouse)
{
	m_fIsButtonDown = TRUE;

	return 0;
}

HRESULT	CSelectSrv::OnButtonUp(WPARAM wParam, LPARAM lParam)
{
	INT	i;
	RECT tRect;

	m_fIsButtonDown = FALSE;

	for( i = 0 ;  i < m_nSrvCount  ; i++)
	{	SetRect(&tRect,m_rcWnd.left + m_ppxSrvBtn[i]->rcBtn.left, m_rcWnd.top + m_ppxSrvBtn[i]->rcBtn.top, m_rcWnd.left +m_ppxSrvBtn[i]->rcBtn.right ,m_rcWnd.top +m_ppxSrvBtn[i]->rcBtn.bottom);
		if(IsInRect(tRect,LOWORD(lParam), HIWORD(lParam)))
		{
			g_xClientSocket.OnSelectServer(m_ppxSrvBtn[i]->szName);
		}
		m_ppxSrvBtn[i]->dwTxtColor	= SERVER_BTN_TEXTCOLOR_NORMAL;			// COLOR(YELLOW)
		m_ppxSrvBtn[i]->dwBkColor	= SERVER_BTN_BKCOLOR_NORMAL;
		m_ppxSrvBtn[i]->nAlpha		= 125;
	}

	return 0;
}



LRESULT	CSelectSrv::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	INT	i;
	RECT tRect;

	if(!m_fIsButtonDown)
		for( i = 0 ;  i < m_nSrvCount  ; i++)
		{	SetRect(&tRect,m_rcWnd.left + m_ppxSrvBtn[i]->rcBtn.left, m_rcWnd.top + m_ppxSrvBtn[i]->rcBtn.top, m_rcWnd.left +m_ppxSrvBtn[i]->rcBtn.right ,m_rcWnd.top +m_ppxSrvBtn[i]->rcBtn.bottom);
			if(IsInRect(tRect,LOWORD(lParam), HIWORD(lParam)))
			{	m_ppxSrvBtn[i]->dwTxtColor	= SERVER_BTN_TEXTCOLOR_FOCUS;			// COLOR(YELLOW)
				m_ppxSrvBtn[i]->dwBkColor	= SERVER_BTN_BKCOLOR_FOCUS;
				m_ppxSrvBtn[i]->nAlpha		= 125;
			}
			else
			{	m_ppxSrvBtn[i]->dwTxtColor	= SERVER_BTN_TEXTCOLOR_NORMAL;		// COLOR(WHITE)
				m_ppxSrvBtn[i]->dwBkColor	= SERVER_BTN_BKCOLOR_NORMAL;
				m_ppxSrvBtn[i]->nAlpha		= 125;
			}
		}
	return 0;
}

VOID CSelectSrv::Render(INT	nLoopTime)
{
	INT		i,j;
	RECT	tRect;

	COLORREF nBkColor;
	if(m_fIsActive)
	{
		// Write Texts;
		Draw3DBorder(m_rcWnd,RGB(125,125,125),125);
		for(i = 0 ; i <= ((m_nSrvCount - 1) / COUNT_BUTTON_PER_COLUME) ; i++)
		{
			for(j = i * COUNT_BUTTON_PER_COLUME; (j < m_nSrvCount && j < (COUNT_BUTTON_PER_COLUME * (i + 1))) ; j ++)
			{	nBkColor = m_ppxSrvBtn[j]->dwBkColor;
				SetRect(&tRect,m_rcWnd.left + m_ppxSrvBtn[j]->rcBtn.left, m_rcWnd.top + m_ppxSrvBtn[j]->rcBtn.top, m_rcWnd.left +m_ppxSrvBtn[j]->rcBtn.right ,m_rcWnd.top +m_ppxSrvBtn[j]->rcBtn.bottom);
				if(nBkColor!=RGB(125,125,125))Draw3DBorder(tRect, nBkColor,m_ppxSrvBtn[j]->nAlpha);
				g_xMainWnd.PutsHan(NULL, tRect,
										m_ppxSrvBtn[j]->dwTxtColor, RGB(0,0,0), m_ppxSrvBtn[j]->szCaption,g_xMainWnd.CreateGameFont("Arial",12 ));
			}
		}
	}
}


VOID CSelectSrv::Draw3DBorder(RECT rcWnd,COLORREF dwColor,INT nAlpha)
{
	if ( g_xMainWnd.Get3DDevice() )
	{
		D3DVECTOR vTrans;
		D3DMATRIX matTrans;
		D3DMATRIX matScale;
		D3DMATRIX matWorld;
		D3DMATRIX matView;
		D3DMATRIX matProj;
		D3DMATRIX matWorldOriginal;
		D3DMATERIAL7 mtrl;
		if( SUCCEEDED(g_xMainWnd.Get3DDevice()->BeginScene()))
		{
			g_xMainWnd.Get3DDevice()->GetTransform(D3DTRANSFORMSTATE_WORLD, &matWorldOriginal);

			ZeroMemory(&matView, sizeof(D3DMATRIX));
			D3DVECTOR vEyePt    = D3DVECTOR(0, 0, -(float)((g_xMainWnd.m_rcWindow.bottom - g_xMainWnd.m_rcWindow.top)/2));
			D3DVECTOR vLookatPt = D3DVECTOR(0, 0, 0);
			D3DVECTOR vUpVec    = D3DVECTOR(0, 1, 0);

			D3DUtil_SetViewMatrix(matView, vEyePt, vLookatPt, vUpVec);
			D3DUtil_SetProjectionMatrix(matProj, g_PI/2, float(240.0f/320.0f), -1.0f, 1.0f);

			g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_VIEW,       &matView);
			g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj);

			vTrans.x = (FLOAT)rcWnd.left - 320.0f + (rcWnd.right  - rcWnd.left)/2;
			vTrans.y = (FLOAT)-rcWnd.top + 240.0f - (rcWnd.bottom - rcWnd.top )/2;
			vTrans.z = 0;

			D3DUtil_SetTranslateMatrix(matTrans, vTrans);
			D3DUtil_SetScaleMatrix(matScale, (FLOAT)(rcWnd.right - rcWnd.left), (FLOAT)(rcWnd.bottom - rcWnd.top), 0.0f);
			D3DMath_MatrixMultiply(matWorld, matScale, matTrans);
			g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld);

			// 临时
			D3DUtil_InitMaterial(mtrl,
								 (FLOAT)GetRValue(dwColor)/255.0f,
								 (FLOAT)GetGValue(dwColor)/255.0f,
								 (FLOAT)GetBValue(dwColor)/255.0f);
			mtrl.diffuse.a = nAlpha/255.0f;					// alpha value

			g_xMainWnd.Get3DDevice()->SetMaterial(&mtrl);

			g_xMainWnd.Get3DDevice()->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			g_xMainWnd.Get3DDevice()->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, TRUE);
			g_xMainWnd.Get3DDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			g_xMainWnd.Get3DDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
			g_xMainWnd.Get3DDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);

			g_xMainWnd.Get3DDevice()->SetTexture(0, NULL);
			g_xMainWnd.Get3DDevice()->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX, m_avBillBoard, 4, NULL);

			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_SRCBLEND , D3DBLEND_ONE);
			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
			g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorldOriginal);

			g_xMainWnd.Get3DDevice()->EndScene();
		}
	}
}

#undef POS_SERVERLIST_BORDER_X
#undef POS_SERVERLIST_BORDER_Y
#undef SERVERLIST_BORDER_WIDTH
#undef POS_TOP_SERVER_BTN_X
#undef POS_TOP_SERVER_BTN_Y
#undef SERVER_BTN_WIDTH
#undef SERVER_BTN_HEIGHT
#undef SERVER_BTN_GAP
#undef COUNT_BUTTON_PER_COLUME
#undef SERVER_BTN_TEXTCOLOR_NORMAL
#undef SERVER_BTN_TEXTCOLOR_FOCUS
#undef SERVER_BTN_TEXTCOLOR_CLICK
#undef SERVER_BTN_BKCOLOR_NORMAL
#undef SERVER_BTN_BKCOLOR_FOCUS
#undef SERVER_BTN_BKCOLOR_CLICK
#undef _LEFT_BORDER
#undef _TOP_BORDER
#undef _RIGHT_BORDER
#undef _BOTTOM_BORDER

// BackGround
#define AVI_CREATECHAR						".\\CreateChr.avi"
#define AVI_STARTGAME						".\\StartGame.avi"

#define IMG_IDX_SELECT_CHR_BACK				50

#define IMG_IDX_CREATE_CHR_BACK				90

#define BACK_IMG_POS_X						0
#define BACK_IMG_POS_Y						0

CBackGround::CBackGround()
{
	m_nRenderState = 0;
	m_pxImage = NULL;
}

CBackGround::~CBackGround()
{
	m_xCreateChrAvi.ReleaseAvi();
	m_xStartGameAvi.ReleaseAvi();
}

VOID CBackGround::Create(CWHWilImageData* pxImage)
{
	m_pxImage = pxImage;
	m_xCreateChrAvi.InitAvi(g_xSound.GetSoundObject()->GetDS());
	m_xStartGameAvi.InitAvi(g_xSound.GetSoundObject()->GetDS());
}


HRESULT CBackGround::OnButtonDown(WPARAM wParam, LPARAM lParam)
{

	return 0;
}


HRESULT	CBackGround::OnButtonUp(WPARAM wParam, LPARAM lParam)
{

	return 0;
}



VOID CBackGround::Render(INT	nLoopTime)
{
	if(m_fIsActive)
	{
		switch(m_nRenderState)
		{
			case _RENDER_STILL_1:
			{
				m_pxImage->NewSetIndex(IMG_IDX_SELECT_CHR_BACK);
				g_xMainWnd.DrawWithImageForComp(BACK_IMG_POS_X, BACK_IMG_POS_Y,
												m_pxImage->m_lpstNewCurrWilImageInfo->shWidth,m_pxImage->m_lpstNewCurrWilImageInfo->shHeight,
												(WORD*)(m_pxImage->m_pbCurrImage));
				break;
			}
			case _RENDER_AVI_IN:
			{
				if(!m_xCreateChrAvi.Draw(nLoopTime))
				{
					m_nRenderState = _RENDER_STILL_2;
				}
				break;
			}
			case _RENDER_STILL_2:
			{
				m_pxImage->NewSetIndex(IMG_IDX_CREATE_CHR_BACK);
				g_xMainWnd.DrawWithImageForComp(BACK_IMG_POS_X, BACK_IMG_POS_Y,
												m_pxImage->m_lpstNewCurrWilImageInfo->shWidth,m_pxImage->m_lpstNewCurrWilImageInfo->shHeight,
												(WORD*)(m_pxImage->m_pbCurrImage));
				break;
			}
			case _RENDER_AVI_OUT:
			{
				if(!m_xCreateChrAvi.Draw(nLoopTime))
					m_nRenderState = _RENDER_STILL_1;
				break;
			}
			case _RENDER_AVI_START:
			{
				if(!m_xStartGameAvi.Draw(nLoopTime))
					m_nRenderState = _RENDER_STARTGAME;
				break;
			}
		}
	}
}


VOID CBackGround::SetRenderState(INT nState)
{
	RECT tRect;
	SetRect(&tRect,0,0,640,480);

	AllStopAvi();
	switch(m_nRenderState)
	{
		case _RENDER_AVI_IN:
		case _RENDER_AVI_OUT:
			m_xCreateChrAvi.Stop();
			break;
		case _RENDER_AVI_START:
			m_xStartGameAvi.Stop();
			break;
	}

	m_nRenderState = nState;

	switch(m_nRenderState)
	{
		case _RENDER_AVI_IN:
		{
			m_xCreateChrAvi.Create(AVI_CREATECHAR, FALSE);
			m_xCreateChrAvi.SetDestRect(tRect);
			m_xCreateChrAvi.Start();
			break;
		}
		case _RENDER_AVI_OUT:
		{
			m_xCreateChrAvi.Create(AVI_CREATECHAR, FALSE,FALSE);
			m_xCreateChrAvi.SetDestRect(tRect);
			m_xCreateChrAvi.Start();
			break;
		}
		case _RENDER_AVI_START:
		{
			m_xStartGameAvi.Create(AVI_STARTGAME,FALSE);
			m_xStartGameAvi.SetDestRect(tRect);
			m_xStartGameAvi.Start();
			break;
		}
	}
}

INT CBackGround::GetRenderState(VOID)
{
	return m_nRenderState;
}
VOID CBackGround::AllStopAvi(VOID)
{
	m_xCreateChrAvi.Stop();
	m_xStartGameAvi.Stop();
}

#undef AVI_CREATECHAR
#undef AVI_STARTGAME
#undef IMG_IDX_SELECT_CHR_BACK
#undef IMG_IDX_CREATE_CHR_BACK
#undef BACK_IMG_POS_X
#undef BACK_IMG_POS_Y

// CreateChr
#define GENDER_MALE		0
#define GENDER_FEMALE	1

#define JOB_WOR			0
#define JOB_TAO			1
#define JOB_SOR			2

CCreateChr::CCreateChr()
{
	m_pxImage = NULL;
	m_nSelectGender=GENDER_MALE;
	m_nSelectJob=JOB_WOR;
}

CCreateChr::~CCreateChr()
{

}

VOID CCreateChr::Create(CWHWilImageData* pxImage)
{
	INT	nLoop;
	// Buttons
	BUTTONINFO CharProcBtnInfo[] = {
									{ IMG_IDX_EMPTY_BTN_UP, 	CREATE_CHAR_OK_BTN_POS_X,		CREATE_CHAR_OK_BTN_POS_Y,		WIDTH_EMPTY_BTN,	HEIGHT_EMPTY_BTN},	// Create Charector Ok Button
									{ IMG_IDX_WAR_BTN_UP,	 	CREATE_CHAR_WAR_BTN_POS_X,		CREATE_CHAR_BTN_POS_Y,			WIDTH_JOB_BTN,		HEIGHT_JOB_BTN},	// 战士 Button
									{ IMG_IDX_SOR_BTN_UP,		CREATE_CHAR_SOR_BTN_POS_X,		CREATE_CHAR_BTN_POS_Y,			WIDTH_JOB_BTN,		HEIGHT_JOB_BTN},	// 术士 Button
									{ IMG_IDX_TAO_BTN_UP, 		CREATE_CHAR_TAO_BTN_POS_X,		CREATE_CHAR_BTN_POS_Y,			WIDTH_JOB_BTN,		HEIGHT_JOB_BTN}		// 道士 Button
									};

	m_pxImage		= pxImage;
	for(nLoop = 0 ; nLoop < 4; nLoop++)
	{
		m_xButtons[nLoop].SetBtn(&CharProcBtnInfo[nLoop]);
	}
}

HRESULT	CCreateChr::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	if (wParam == VK_RETURN || wParam == VK_TAB)
	{
		return 1;
	}
	return 0;
}

HRESULT CCreateChr::OnButtonDown(WPARAM wParam, LPARAM lParam)
{
//	INT i;
//	RECT tRect;

	m_fIsButtonDown = TRUE;

	return 0;
}

HRESULT CCreateChr::OnButtonDown(POINT ptMouse)
{
	m_fIsButtonDown = TRUE;
	return 0;
}

HRESULT	CCreateChr::OnButtonUp(WPARAM wParam, LPARAM lParam)
{
//	INT	i;

	m_fIsButtonDown = FALSE;

	return 0;
}



VOID CCreateChr::Render(INT	nLoopTime)
{
	INT i ;
	POINT nPos[] = { {120, 88}, {120, 85}, {150, 108}};

	if(m_fIsActive)
	{
		MoveWindow(g_xChatEditBox.GetSafehWnd(),
						   g_xMainWnd.m_rcWindow.left+280,
						   g_xMainWnd.m_rcWindow.top+400, 75, 15, TRUE);

		for (i = ID_CREATE_CHR_OK_BTN ; i <= ID_CREATE_CHR_TAO_BTN; i++)
		{
			m_pxImage->NewSetIndex(m_xButtons[i].m_nButtonID + m_xButtons[i].m_nState -1);
			g_xMainWnd.DrawWithImageForComp(m_xButtons[i].m_Rect.left, m_xButtons[i].m_Rect.top,
										m_pxImage->m_lpstNewCurrWilImageInfo->shWidth,
										m_pxImage->m_lpstNewCurrWilImageInfo->shHeight
										, (WORD*)(m_pxImage->m_pbCurrImage));
		}

		// 临时~!
		m_pxImage->NewSetIndex(IMG_IDX_WAR_MALE_CREATE + m_nSelectGender + m_nSelectJob * 2);

		g_xMainWnd.DrawWithImageForComp(nPos[m_nSelectJob].x,nPos[m_nSelectJob].y
				,m_pxImage->m_lpstNewCurrWilImageInfo->shWidth,m_pxImage->m_lpstNewCurrWilImageInfo->shHeight
				,(WORD*)(m_pxImage->m_pbCurrImage));
	}
}

/*
#define GENDER_MALE		0
#define GENDER_FEMALE	1

#define JOB_WOR			0
#define JOB_TAO			1
#define JOB_SOR			2
*/

#undef GENDER_MALE
#undef GENDER_FEMALE
#undef JOB_WOR
#undef JOB_TAO
#undef JOB_SOR

// SelectChr
#define CHR_ANI_COUNT			21
#define ANIMATION_TICK			200
#define BLEND_TICK				50


CSelectChr::CSelectChr()
{
	m_nRenderState = 0;
	m_nAnimationCount = 0;
	m_nRoll = 0;
	m_nBlend = -999;
	m_fIsButtonDown	= FALSE;
}

CSelectChr::~CSelectChr()
{
	if(m_pBtn!=NULL)
	{
		delete[] m_pBtn;
	}
}

VOID CSelectChr::Create(CWHWilImageData* pxImage,CPDLList<CHRINFO>* pxList, CPDLList<CHRRECT>* pxrcList,INT nChrCount)
{
	INT	nLoop;
	// Buttons
	BUTTONINFO CharProcBtnInfo[] ={
									{ IMG_IDX_SELECT_CREATE,	SELECT_CHAR_NEW_BTN_POS_X,		SELECT_CHAR_NEW_BTN_POS_Y,		0,					0},	// Create New Charector Button
									{ IMG_IDX_SELECT_DELETE,	SELECT_CHAR_ERASE_BTN_POS_X,	SELECT_CHAR_ERASE_BTN_POS_Y,	0,					0},	// Erase Charector Button
									{ IMG_IDX_SELECT_EXIT,		SELECT_CHAR_EXIT_BTN_POS_X,		SELECT_CHAR_EXIT_BTN_POS_Y,		0,					0},	// Exit Button
									{ IMG_IDX_SELECT_START,		SELECT_CHAR_START_BTN_POS_X,	SELECT_CHAR_START_BTN_POS_Y,	0,					0},	// Start Button
									};

	m_pBtn = new CMirButton*[sizeof(CharProcBtnInfo)/sizeof(BUTTONINFO)];

	m_pxImage			= pxImage;
	m_pxSList			= pxList;
	m_pxrcList			= pxrcList;
	m_nChrCount			= nChrCount;

	m_xLightFog.Init(RGB(125,125,125));

	for( nLoop = 0 ; nLoop < (sizeof(CharProcBtnInfo)/sizeof(BUTTONINFO)) ; nLoop++)
	{
		m_pBtn[nLoop] = new CMirButton;
		m_pxImage->NewSetIndex(CharProcBtnInfo[nLoop].nID);
		CharProcBtnInfo[nLoop].right = m_pxImage->m_lpstNewCurrWilImageInfo->shWidth;
		CharProcBtnInfo[nLoop].bottom = m_pxImage->m_lpstNewCurrWilImageInfo->shHeight;
		m_pBtn[nLoop]->SetBtn(&CharProcBtnInfo[nLoop]);
	}
}

VOID CSelectChr::OpenWnd(VOID)
{
	m_fIsActive = TRUE;
	m_nRenderState = 9;
	m_nBlend = 100;
}

HRESULT CSelectChr::OnButtonDown(WPARAM wParam, LPARAM lParam)
{
	if(m_fIsActive)
		for (int i = ID_SEL_CHR_NEW_BTN; i <= ID_SEL_CHR_START_BTN; i++)
		{
			if (IsInRect(m_pBtn[i]->m_Rect,LOWORD(lParam), HIWORD(lParam)))
			{
				m_pBtn[i]->m_nState = BUTTON_STATE_DOWN;
				m_fIsButtonDown = TRUE;
			}
			else
				m_pBtn[i]->m_nState = BUTTON_STATE_UP;
		}

	return 0;
}

HRESULT CSelectChr::OnButtonDown(POINT ptMouse)
{
	m_fIsButtonDown = TRUE;
	return 0;
}

HRESULT	CSelectChr::OnButtonUp(WPARAM wParam, LPARAM lParam)
{
	INT		i;
	int		Px,Py;
	if(m_fIsActive)
	{
		for(i = ID_SEL_CHR_NEW_BTN; i <= ID_SEL_CHR_START_BTN; i++)
		{	m_pBtn[i]->m_nState = BUTTON_STATE_UP;
			if (IsInRect(m_pBtn[i]->m_Rect,LOWORD(lParam), HIWORD(lParam)))
			{	switch(i)
				{
					case ID_SEL_CHR_NEW_BTN:
					{
						if(m_nChrCount<3)
						{	m_nBlend = 0;
							m_nRenderState = 1;
							return RETURN_SELECT_NEW_BTN;
						}
						else
							return RETURN_SELECT_NEW_ERR;
						break;
					}
					case ID_SEL_CHR_DEL_BTN:
					{
						if(m_nChrCount!=0)
						{
							return RETURN_SELECT_DEL_BTN;
						}
						else
							return RETURN_SELECT_DEL_ERR;
						break;
					}
					case ID_SEL_CHR_EXIT_BTN:
					{
						SendMessage(g_xMainWnd.GetSafehWnd(), WM_DESTROY, NULL, NULL);
						return 0L;
					}
					case ID_SEL_CHR_START_BTN:
					{
						if(m_nSelectChr != SELECT_NONE)
						{	m_nBlend = 0;
							m_pxSList->MoveCurrentToTop();
							m_pxSList->MoveNode(m_nSelectChr);
							g_xClientSocket.OnSelChar(m_pxSList->GetCurrentData()->ChrName);
							return RETURN_SELECT_START_BTN;
						}
						else
						{
							return RETURN_SELECT_START_ERR;
						}
						break;
					}
				}
			}
		}

		m_pxrcList->MoveCurrentToTop();
		m_pxSList->MoveCurrentToTop();
		for(i=0; i < m_pxrcList->ListLength(); i++)
		{
			Px = m_pxrcList->GetCurrentData()->nLeft - LOWORD(lParam);
			Py = m_pxrcList->GetCurrentData()->nTop - HIWORD(lParam);
			if(Px < 0 && ((Px +m_pxrcList->GetCurrentData()->nWidth)>0))
			{
				if(Py<0 && ((Py+m_pxrcList->GetCurrentData()->nHeight)>0))
				{
					m_nSelectChr = i;

				}
			}
			m_pxrcList->MoveNextNode();
		}
	}

	m_fIsButtonDown = FALSE;

	return 0;
}


LRESULT	CSelectChr::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	INT	i;
	BOOL	fOnButton = FALSE;
	if(m_fIsActive&&(!m_fIsButtonDown))
	{
		for (i = ID_SEL_CHR_NEW_BTN; i <= ID_SEL_CHR_START_BTN; i++)
		{
			if (IsInRect(m_pBtn[i]->m_Rect,LOWORD(lParam), HIWORD(lParam)))
			{
				m_pBtn[i]->m_nState = BUTTON_STATE_DOWN;
				fOnButton = TRUE;
			}
			else
				m_pBtn[i]->m_nState = BUTTON_STATE_UP;
		}

/*		if(fOnButton)
		{
			HCURSOR	hCursor;
			hCursor = LoadCursor(g_xMainWnd.GetSafehInstance(),MAKEINTRESOURCE(IDC_HAND1));
			SetCursor(hCursor);
			DestroyCursor(hCursor);
		}
		else
			g_xMainWnd.OnSetCursor(IDC_ARROW);
*/
	}
	return 0;
}

VOID CSelectChr::Render(INT	nLoopTime)
{
	static DWORD	dwTicks = 0;
	int i;
	INT	nPx,nPy;
	POINT	ptPos[]	=	{
						{310,310},	// One
						{300,270},	// Two-One
						{360,310},	// Two-Two
						{250,270},	// Three-One
						{320,310},	// Three-Two
						{390,340}
						};// Three-Three

	if(m_fIsActive)
	{
		m_xLightFog.ClearSavedLightColor();

		for (i = ID_SEL_CHR_NEW_BTN; i <= ID_SEL_CHR_START_BTN ; i++)	// 按钮(临时)
		{
			if(m_pBtn[i]->m_nState==BUTTON_STATE_DOWN)		// Draw Button when Button state is Downed..
			{
				m_pxImage->NewSetIndex(m_pBtn[i]->m_nButtonID);

				g_xMainWnd.DrawWithImageForComp(m_pBtn[i]->m_Rect.left, m_pBtn[i]->m_Rect.top,
											m_pxImage->m_lpstNewCurrWilImageInfo->shWidth,
											m_pxImage->m_lpstNewCurrWilImageInfo->shHeight
											, (WORD*)(m_pxImage->m_pbCurrImage));
			}
		}

		dwTicks +=nLoopTime;

		if(dwTicks > BLEND_TICK)
		{
			m_nBlend = m_nBlend +((m_nRenderState != 0)? dwTicks / BLEND_TICK *(m_nRenderState != 9 ? 1 : -1): 0);
			m_nRoll = (m_nRoll+ 2)%100;
			if(dwTicks > ANIMATION_TICK)
			{
				m_nAnimationCount++;
				m_nAnimationCount%=CHR_ANI_COUNT;
				dwTicks = 0;
			}
		}

		m_pxSList->MoveCurrentToTop();
		for( i = 0 ; i < m_nChrCount ; i ++)
		{
				// Draw Person
				m_pxImage->NewSetIndex(IDX_IMG_MALE_WAR
										+ (m_pxSList->GetCurrentData()->Gender) * 60
										+ (m_pxSList->GetCurrentData()->Class) * 120
										+ ((i == m_nSelectChr) ? m_nAnimationCount : 0)) ;
				nPx = ptPos[i + m_nChrCount - 1].x + m_pxImage->m_lpstNewCurrWilImageInfo->shPX;
				nPy = ptPos[i + m_nChrCount - 1].y + m_pxImage->m_lpstNewCurrWilImageInfo->shPY;

				g_xMainWnd.DrawWithABlendCompDataWithBackBuffer(nPx,nPy
								,m_pxImage->m_lpstNewCurrWilImageInfo->shWidth,m_pxImage->m_lpstNewCurrWilImageInfo->shHeight
								,(WORD*)(m_pxImage->m_pbCurrImage),640,480,0xffff,0xffff,((m_nBlend>=0)? m_nBlend : 0));

				// Draw Fire
				if(i == m_nSelectChr)
				{
					m_pxImage->NewSetIndex(IDX_IMG_MALE_WAR_FIRE
											+ (m_pxSList->GetCurrentData()->Gender) * 60
											+ (m_pxSList->GetCurrentData()->Class) * 120
											+ (m_nAnimationCount)) ;

					nPx = ptPos[i + m_nChrCount - 1].x + m_pxImage->m_lpstNewCurrWilImageInfo->shPX;
					nPy = ptPos[i + m_nChrCount - 1].y + m_pxImage->m_lpstNewCurrWilImageInfo->shPY;

					g_xMainWnd.DrawWithABlendCompDataWithBackBuffer(nPx,nPy
									,m_pxImage->m_lpstNewCurrWilImageInfo->shWidth,m_pxImage->m_lpstNewCurrWilImageInfo->shHeight
									,(WORD*)(m_pxImage->m_pbCurrImage),640,480,0xffff,0x1f1f,((m_nBlend>=0)? m_nBlend : 50));

					INT nTemp = (INT)(125.0f * sin((double)(m_nRoll/10)));

					m_xLightFog.SetLightRadiusWithCircle(nPx+100,nPy+100,((m_nBlend>=0) ? 5 - ((m_nBlend)/20) : 5),125 + nTemp,255 + nTemp,255 + nTemp);
				}

			m_pxSList->MoveNextNode();
		}
		m_xLightFog.ShowLightFog();

		ShowChrInfo();
		if((m_nBlend>=100 && m_nRenderState!=9)||(m_nBlend<=0 && m_nRenderState==9))
		{
			m_nBlend = -1;
			m_nRenderState = m_nRenderState + 10;
		}

	}
}

VOID CSelectChr::ShowChrInfo(VOID)
{
	char ChrName[20];
	char ChrLev[3];
	if(m_nChrCount!=0)
	{
		m_pxSList->MoveCurrentToTop();
		m_pxSList->MoveNode(m_nSelectChr);
		strcpy(ChrName,m_pxSList->GetCurrentData()->ChrName);
		strcpy(g_szCharName,m_pxSList->GetCurrentData()->ChrName);
		itoa(m_pxSList->GetCurrentData()->Level,ChrLev,10);
		g_xMainWnd.PutsHan(NULL,m_PosName.x,m_PosName.y,RGB(255,255,255),RGB(0,0,0),ChrName);
	}
}


VOID CSelectChr::CloseWnd(VOID)
{
	m_fIsActive = FALSE;
}

#undef CHR_ANI_COUNT
#undef ANIMATION_TICK
#undef BLEND_TICK

VOID CLoginProcess::RenderScene(INT nLoopTime)
{
	static	DWORD	dwLastTick = 0;

	m_Image.NewSetIndex(IMAGE_INDEX_LOGINBASE);
	if(m_Progress== PRG_QUIT){
		SendMessage(g_xMainWnd.GetSafehWnd(), WM_DESTROY, NULL, NULL);
		return;
	}

	if(m_fIsConnected==CONNECT_SUCCESS)
	{
		Clear(0x0000);
		m_xAvi.Render(nLoopTime);
		switch (m_Progress)
		{
			case PRG_CONNECT:
			{
				ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
				m_nAnimationCounter = 0;
				break;
			}
			case PRG_INTRO:
			{
				ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
				RenderIntro(nLoopTime);
				break;
			}
			// Render For Login
			case PRG_LOGIN:
			{
				m_xLogin.Render(nLoopTime);
				break;
			}
			// Render
			case PRG_TO_SELECT_CHR:
			{
				ShowWindow(g_xChatEditBox.GetSafehWnd(),SW_HIDE);
				RenderScroll(nLoopTime);
				break;
			}
			// Render For Server Selection
			case PRG_SERVER_SELE:
			{
				ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
				m_Image.NewSetIndex(IMG_IDX_LOGIN_BACK);					// 临时
				m_xSelectSrv.Render(nLoopTime);
				break;
			}
			// Render For New Account
			case PRG_NEW_ACCOUNT:
			{
				ShowWindow(g_xChatEditBox.GetSafehWnd(),( m_xMsgBox.IsActive() ? SW_HIDE : SW_SHOW ));
//				RenderNewAccount(nLoopTime);
				break;
			}
			// Render For Change Password
			case PRG_CHANGE_PASS:
			{
				ShowWindow(g_xChatEditBox.GetSafehWnd(),( m_xMsgBox.IsActive() ? SW_HIDE : SW_SHOW ));
				RenderPassword(nLoopTime);
				break;
			}
		}
		m_xMsgBox.RenderMessageBox(0);
		if ( FAILED(g_xMainWnd.Present()) )
				g_xMainWnd.RestoreSurfaces();
	}
}


VOID CLoginProcess::RenderIntro(INT nLoopTime)
{
	static DWORD dwLastTick = 0;
	dwLastTick += nLoopTime;
	if(dwLastTick>100)
	{
		m_nAnimationCounter++;
		if(m_nAnimationCounter==50)
			m_xAvi.SetRenderAviState(1);
		if(m_nAnimationCounter>500)
		{
			m_Progress = PRG_LOGIN;
			m_xLogin.OpenWnd();
			m_nAnimationCounter = -999;
		}

	}

}



VOID CLoginProcess::RenderNewAccount(int nLoopTime)
{
	/*char Pass[10];

	MoveWindow(g_xChatEditBox.GetSafehWnd(),
						g_xMainWnd.m_rcWindow.left + g_LoginInputState[m_UserState+2].Left,
						g_xMainWnd.m_rcWindow.left +  g_LoginInputState[m_UserState+2].Top,
						g_LoginInputState[m_UserState+2].Width,
						g_LoginInputState[m_UserState+2].Height, TRUE);
	// Draw Back Image
	g_xMainWnd.DrawWithImageForComp(0, 0,
					m_Image2.m_lpstNewCurrWilImageInfo->shWidth,
					m_Image2.m_lpstNewCurrWilImageInfo->shHeight,(WORD*)(m_Image2.m_pbCurrImage));

	// Draw Image
	m_Image2.NewSetIndex(IMAGE_INDEX_NEWACCOUNT);
	g_xMainWnd.DrawWithImageForComp(ACCOUNT_WIN_POS_X, ACCOUNT_WIN_POS_Y,
						m_Image2.m_lpstNewCurrWilImageInfo->shWidth,
						m_Image2.m_lpstNewCurrWilImageInfo->shHeight,(WORD*)(m_Image2.m_pbCurrImage));

	for (int i = BTN_NEW_OK_ID; i <= BTN_NEW_QUIT_ID; i++){

		if (m_pBtn[i].m_nState != BUTTON_STATE_UP) {

			m_Image2.NewSetIndex(m_pBtn[i].m_nButtonID + (m_pBtn[i].m_nState - 1 ) / 2 );
			g_xMainWnd.DrawWithImageForComp(m_pBtn[i].m_Rect.left, m_pBtn[i].m_Rect.top,
										m_pBtn[i].m_Rect.right - m_pBtn[i].m_Rect.left,
										m_pBtn[i].m_Rect.bottom - m_pBtn[i].m_Rect.top, (WORD*)(m_Image2.m_pbCurrImage));
			break;
		}
	}

	RenderNewAccountHelpTxt();

	for(i =LGM_INPUT_USER_ID ; i <= LGM_INPUT_ANSWER_2 ; i++){

		if( ( i != LGM_INPUT_USER_PASSWORD  ) && ( i!= LGM_INPUT_REPASS ) ){
			g_xMainWnd.PutsHan(NULL, g_LoginInputState[i].Left +2, g_LoginInputState[i].Top +2, RGB(255,255,255), RGB(0,0,0),g_LoginInputState[i].szData);
		}
		else{
			ZeroMemory(Pass,sizeof(Pass));
			memset(Pass,'*',strlen(g_LoginInputState[i].szData));
			g_xMainWnd.PutsHan(NULL, g_LoginInputState[i].Left +2, g_LoginInputState[i].Top +2, RGB(255,255,255), RGB(0,0,0), Pass);
		}
	}*/
}


VOID CLoginProcess::RenderPatch(int nLoopTime)
{
	RECT tRect;
	ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
	//
	Clear((DWORD)RGB(0,0,0));

/*
	// Go Login Without Connection
	m_Progress = PRG_INTRO;
	m_fIsConnected	= CONNECT_SUCCESS;
	g_xMainWnd.ResetDXG(_SCREEN_WIDTH, _SCREEN_HEIGHT, _SCREEN_BPP, _DXG_SCREENMODE_FULLSCREEN, _DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D);
*/

	if(pPatch->m_bEndPatch == TRUE)//GetFtpFiles()==0l)	// 如果 Patch 已结束
	{

		if(pPatch->m_bPatched == TRUE)	// 若已 Patch 则退出
		{
			delete pPatch;
			char PatchName[1024];
			ZeroMemory(PatchName,1024);
			GetCurrentDirectory(1024,PatchName);
			strcat(PatchName,MIR2_PATCH_FILE_NAME);
			//m_Progress = PRG_QUIT;
      m_Progress = PRG_INTRO;
			ShellExecute(NULL,"open",PatchName,NULL,NULL,SW_SHOWNORMAL);
			// 结束运行并执行外部 Patch 程序。
		}
		else
		{
			// 若没有已打补丁的内容
			delete pPatch;

			g_xClientSocket.ConnectToServer(g_xMainWnd.GetSafehWnd(), m_szServerIP, m_nServerPort);
			m_Progress = PRG_INTRO;


			// 切换到 Full Screen
			g_xMainWnd.ResetDXG(640, 480, _SCREEN_BPP, _DXG_SCREENMODE_WINDOW, _DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D);
//			g_xMainWnd.ResetDXG(640, 480, _SCREEN_BPP, _DXG_SCREENMODE_FULLSCREEN, _DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D);
			SetRect(&tRect, SCR_LEFT, SCR_TOP, SCR_RIGHT, SCR_BOTTOM);
		}
	}
	else
	{
		// 若需继续 Patch
		pPatch->GetFtpFiles();
	}

	if(m_xMsgBox.IsActive())
	{
		HINSTANCE hLib;
		hLib = LoadLibrary(MESSAGE_DLL_1);
		SetErrorMessage(300,m_szErrorMsg);		// 查找对应 Error 消息
		FreeLibrary(hLib);
		MessageBox(g_xMainWnd.GetSafehWnd(),m_szErrorMsg,"Error",MB_OK);
	}
}

VOID CLoginProcess::RenderPassword(int nLoopTime)
{
	ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
}

VOID CLoginProcess::RenderNewAccountHelpTxt(VOID)
{

}


VOID CLoginProcess::RenderScroll(INT nLoopTime)
{
	static INT	nScroll = 0;
	static INT	nDelay = 0;
	nDelay += nLoopTime;


	if(nDelay > 40)
	{	nScroll += 5;
		if(nScroll >= 180)
		{
			SetNextProc();												// 移动到角色选择服务器 (临时)
			return;
		}
	}

//	m_Image.NewSetIndex(IMG_IDX_LOGIN_BACK);
//	g_xMainWnd.DrawWithImageForCompClipRgn(SCR_LEFT, SCR_TOP + nScroll, SCR_RIGHT, SCR_BOTTOM, (WORD*)(m_Image.m_pbCurrImage),SCR_LEFT,SCR_BOTTOM-nScroll);
}

#define POS_NAME_X 300
#define POS_NAME_Y 460
VOID CCharacterProcess::RenderScene(INT nLoopTime)
{
	m_xBGround.Render(nLoopTime);
	switch(m_Progress)
	{
		case PRG_CHAR_SELE:
		{
			ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
			switch(m_xSelectChr.m_nRenderState)
			{
				case 11:	// Zoom in
				{
					m_xBGround.SetRenderState(_RENDER_AVI_IN);
					m_Progress = PRG_SEL_TO_CREA;
					m_xSelectChr.CloseWnd();
					break;
				}
				case 12:	// Zoom Out
				{
					m_xBGround.SetRenderState(_RENDER_AVI_START);
					m_Progress = PRG_SEL_TO_GAME;
					m_xSelectChr.CloseWnd();
					break;
				}
				case 19:	// Select;
				{
					m_xSelectChr.m_nRenderState = 0;
				}
			}
			m_xSelectChr.Render(nLoopTime);
			break;
		}
		case PRG_SEL_TO_CREA:
			ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
			if(m_xBGround.GetRenderState()==_RENDER_STILL_2)
				m_Progress = PRG_CREATE_CHAR;
			break;
		case PRG_CREATE_CHAR:
			ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_SHOW);
			RenderCharCreate(nLoopTime);
			break;
		case PRG_CREA_TO_SEL:
			ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
			if(m_xBGround.GetRenderState() == _RENDER_STILL_1)
			{	m_Progress = PRG_CHAR_SELE;
				m_xSelectChr.OpenWnd();
			}
			break;
		case PRG_SEL_TO_GAME:
			ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
			if(m_xBGround.GetRenderState()==_RENDER_STARTGAME)
			{	m_Progress	= PRG_PLAY_GAME;				// Play Game
				SetNextProc();
			}
			break;
	}
	m_xMsgBox.RenderMessageBox(0);
}

VOID CCharacterProcess::ShowStatus(INT nStartX, INT nStartY)
{
/*	TCHAR			szBuff[MAX_PATH];
	sprintf(szBuff, "Character Procedure");*/

	CWHDefProcess::ShowStatus(nStartX, nStartY+20);
}


VOID	CCharacterProcess::RenderCharCreate(INT nLoopTime)
{
	POINT nPos[] = { {120, 88}, {120, 85}, {150, 108}};
//	int i ;

	MoveWindow(g_xChatEditBox.GetSafehWnd(),
					   g_xMainWnd.m_rcWindow.left+POS_NAME_X,
					   g_xMainWnd.m_rcWindow.top+POS_NAME_Y, 75, 15, TRUE);

/*	for (i = ID_CREATE_CHR_OK_BTN ; i <= ID_CREATE_CHR_TAO_BTN; i++)
	{
		m_Image.NewSetIndex(m_pBtn[i].m_nButtonID + m_pBtn[i].m_nState -1);
		g_xMainWnd.DrawWithImageForComp(m_pBtn[i].m_Rect.left, m_pBtn[i].m_Rect.top,
									m_Image.m_lpstNewCurrWilImageInfo->shWidth,
									m_Image.m_lpstNewCurrWilImageInfo->shHeight
									, (WORD*)(m_Image.m_pbCurrImage));
	}
*/
	// 临时~!
	m_Image.NewSetIndex(IMG_IDX_WAR_MALE_CREATE + m_SelectSex + m_SelectJob * 2);

	g_xMainWnd.DrawWithImageForComp(nPos[m_SelectJob].x,nPos[m_SelectJob].y
			,m_Image.m_lpstNewCurrWilImageInfo->shWidth,m_Image.m_lpstNewCurrWilImageInfo->shHeight
			,(WORD*)(m_Image.m_pbCurrImage));
}

#undef POS_NAME_X
#undef POS_NAME_Y
