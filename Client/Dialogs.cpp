#ifdef MIR3_LEGACY_CLIENT

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
				// ÔÚ m_xMsg ÖÐ´æÈë String
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

			// ¶ÁÈ¡½ØÖÁµ±Ç°²¿·ÖµÄ×Ö·û´®¡£
			memcpy(&szTmpCheck, &szSrc[nStartLen], nEndLen-nStartLen);
			INT nsLen;
			nsLen = strlen(szTmpCheck);

			// µ±Ç°ÒÑ¶ÁÈ¡µÄ²¿·Ö³¬³öÔÊÐí¿í¶ÈÊ±¡£
			if (nsLen > nDivideLen )
			{
				// µ±Ç°Èç¹ûÊÇ 2 ×Ö½Ú×Ö·û.
				if ( szSrc[nEndLen-1] < 0 )
				{
					// Èç¹ûµ±Ç°Ç°ÃæµÄ×Ö·û²»ÊÇ 2 ×Ö½Ú×Ö·û
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
				// 1×Ö½Ú×Ö·û.
				// Ö»Ðè¿¼ÂÇµ½±Èµ±Ç°Î»ÖÃÍùÇ°Ò»¸ö×Ö½ÚµÄ²¿·Ö¼´¿É¡£
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
			else if(szSrc[nEndLen-1]=='\n')	// Ç¿ÖÆ»»ÐÐ     Èç¹ûÓÃ \n Ôò Error ????
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
	{	// Ö»ÔÚÕæÕýÐèÒª»æÖÆÊ±²Å»æÖÆ
		// È·ÈÏÒª»æÖÆµÄÍ¼ÏñÇøÓò
		g_xMainWnd.DrawWithImageForCompClipRgn(m_xBoxPos.x,m_xBoxPos.y,m_shWidth,m_shHeight,m_wpData,800,600);

		POINT	xTxtPos;
		nCount = m_xMsg.GetCounter();
		m_xMsg.MoveCurrentToTop();

		xTxtPos.x = m_xBoxPos.x +24;
		switch(nCount)
		{
		case 1:	// Òª»æÖÆµÄÏßÖ»ÓÐ 1 ÌõÊ±
			{
				xTxtPos.y = 68;
				INT n = strlen(m_szMsg);
				if(n<WRITESTRLENTH)
				{
					xTxtPos.x= xTxtPos.x + (WRITESTRLENTH-n)*2;
				}
				break;
			}
		case 2:	// Òª»æÖÆµÄÐÐÊýÎª 2 ¸öÊ±
			{
				xTxtPos.y = 61;
				break;
			}
		case 3:	// Òª»æÖÆµÄÐÐÊýÎª 2 ¸öÊ±
			{
				xTxtPos.y = 54;
				break;
			}
		case 4:	// Òª»æÖÆµÄÐÐÊýÎª 2 ¸öÊ±
			{
				xTxtPos.y = 47;
				break;
			}
		case 5:	// Òª»æÖÆµÄÐÐÊýÎª 2 ¸öÊ±
			{
				xTxtPos.y = 40;
				break;
			}
		case 6:	// Òª»æÖÆµÄÐÐÊýÎª 2 ¸öÊ±
			{
				xTxtPos.y = 33;
				break;
			}
		default: // µ±ÒªÊä³öµÄÐÐÊýÎª 3 ÐÐÒÔÉÏÊ±
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
		// Ö»ÔÚÕæÕýÐèÒª»æÖÆÊ±²Å»æÖÆ
		// È·ÈÏÒª»æÖÆµÄÍ¼ÏñÇøÓò
		POINT	xTxtPos;

		m_xBoxPos.x = m_rcWnd.left;
		m_xBoxPos.y = m_rcWnd.top;

		nCount = m_xMsg.GetCounter();
		m_xMsg.MoveCurrentToTop();

		xTxtPos.x = m_xBoxPos.x +24;
		switch(nCount)
		{
		case 1:	// Òª»æÖÆµÄÏßÖ»ÓÐ 1 ÌõÊ±
			{
				xTxtPos.y = 68;
				INT n = strlen(m_szMsg);
				if(n<WRITESTRLENTH)
				{
					xTxtPos.x= xTxtPos.x + (WRITESTRLENTH-n)*2;
				}
				break;
			}
		case 2:	// Òª»æÖÆµÄÐÐÊýÎª 2 ¸öÊ±
			{
				xTxtPos.y = 61;
				break;
			}
		case 3:	// ´òÓ¡µÄÐÐÊýÎª 3 ¸öÊ±
			{
				xTxtPos.y = 54;
				break;
			}
		case 4:	// Òª»æÖÆµÄÐÐÊýÎª 4 ¸öÊ±
			{
				xTxtPos.y = 47;
				break;
			}
		case 5:	// Òª»æÖÆµÄÐÐÊýÎª 5 ¸öÊ±
			{
				xTxtPos.y = 40;
				break;
			}
		case 6:	// Òª»æÖÆµÄÐÐÊýÎª 6 ¸öÊ±
			{
				xTxtPos.y = 33;
				break;
			}
		default: // Òª»æÖÆµÄÐÐÔÚ 7 ¸öÒÔÉÏÊ±
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

			// ¶ÁÈ¡½ØÖÁµ±Ç°²¿·ÖµÄ×Ö·û´®¡£
			szTmpCheck = szSrc+nStartLen;
			INT nsLen;
			nsLen = strlen(szTmpCheck);

			// µ±Ç°ÒÑ¶ÁÈ¡µÄ²¿·Ö³¬³öÔÊÐí¿í¶ÈÊ±¡£
			if (nsLen > nDivideLen )
			{
				// µ±Ç°Èç¹ûÊÇ 2 ×Ö½Ú×Ö·û.
				if ( szSrc[nEndLen-1] < 0 )
				{
					// Èç¹ûµ±Ç°Ç°ÃæµÄ×Ö·û²»ÊÇ 2 ×Ö½Ú×Ö·û
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
				// 1×Ö½Ú×Ö·û.
				// Ö»Ðè¿¼ÂÇµ½±Èµ±Ç°Î»ÖÃÍùÇ°Ò»¸ö×Ö½ÚµÄ²¿·Ö¼´¿É¡£
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
			else if(szSrc[nEndLen-1] == '\n' || szSrc[nEndLen-1] == 0x1B)	// Ç¿ÖÆ»»ÐÐ     Èç¹ûÓÃ \n Ôò Error ????

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

	Ä£¿éÃû:

	×÷Õß:
	±àÐ´ÈÕÆÚ:

	[ÈÕÆÚ][ÐÞ¸ÄÈË] : ÐÞ¸ÄÄÚÈÝ

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

	// 3D²âÊÔ
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

// ¼¤»î´°¿Ú¡£
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

// Ê¹´°¿Ú»ñµÃ½¹µã¡£
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

// ´°¿Ú¿ªÊ¼ÒÆ¶¯Ê±È¡µÃÓëÊó±êµÄ Gap.
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

	Ä£¿éÃû:

	×÷Õß:
	±àÐ´ÈÕÆÚ:

	[ÈÕÆÚ][ÐÞ¸ÄÈË] : ÐÞ¸ÄÄÚÈÝ

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
	// ´°¿Ú´´½¨.
	CreateGameWnd(nID, pxWndImage, nFrameImgIdx, bCanMove, nStartX, nStartY, nWidth, nHeight);

	// °´Å¥´´½¨.
	m_xInventoryBtn[_BTN_ID_INVENCLOSE	 ].CreateGameBtn(pxWndImage, 280, 281, nStartX+255, nStartY+291);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLUP	 ].CreateGameBtn(pxWndImage, 74, 75, nStartX+247, nStartY+ 20);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLDOWN].CreateGameBtn(pxWndImage, 74, 75, nStartX+247, nStartY+167);

	m_xInvenScrlBar.CreateScrlBar(pxWndImage, 270, _INVEN_MAX_CELL_YCNT, 10, 218, 10);

	// ±³°ü¸ñ×ÓÇøÓòµÄ Set.
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

	// Ñü´øµ¥Ôª¸ñÇøÓòµÄ Set¡£
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


VOID CInventoryWnd::ShowInventoryWnd()
{
	INT nCnt;

	// ´°¿Ú±³¾°»æÖÆ.
	ShowGameWnd();


	RECT rcTitle = {m_rcWnd.left+31, m_rcWnd.top+27, m_rcWnd.left+133, m_rcWnd.top+43};

	g_xMainWnd.PutsHan(NULL, rcTitle, RGB(200, 200, 255), RGB(0, 0, 0), "[Inventory]", g_xMainWnd.CreateGameFont("Arial", 10, 0, FW_BOLD));

	// ¹ö¶¯Ìõ.
	m_xInvenScrlBar.ShowScrlBar(m_rcWnd.left+254, m_rcWnd.top-165, m_nStartLineNum, 12);

	// »æÖÆÎïÆ·À¸ÎïÆ·.
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

	// Ñü´øÎïÆ·»æÖÆ¡£
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

	// »æÖÆ±³°ü´°¿Ú°´Å¥.
	for ( nCnt = 0; nCnt < _MAX_INVEN_BTN; nCnt++ )
	{
		m_xInventoryBtn[nCnt].ShowGameBtn();
	}

	// ÏÔÊ¾ÎïÆ·µÄ×´Ì¬Öµ¡£
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

	ÎïÆ··ÅÈëÓëÈ¡³öÏà¹Ø, ±³°üÎ»ÖÃµÄ±£´æÎÄ¼þ´æÅÌ¼°¼ÓÔØ.(±³°ü¼°Ñü´ø)

*******************************************************************************************************************/

BOOL CInventoryWnd::CheckItemPos(CHAR* szName)
{
	HANDLE	hFile;
	CHAR	szItemFile[MAX_PATH];
	INT		nCnt1, nCnt2;
	ITEMSET	stInvenItem[_MAX_INVEN_ITEM];						// ÁÙÊ±Êý¾Ý¼ÓÔØÓÃ±äÁ¿.
	ITEMSET	stBeltItem[_MAX_BELT_ITEM];
	ITEMSET	stCheckedInvenItem[_MAX_INVEN_ITEM];				// ±£´æÒÑ¼ì²éÊý¾ÝµÄ±äÁ¿.
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

		// ½«·þÎñÆ÷½ÓÊÕµÄ±³°üÊý¾ÝÓë¿Í»§¶Ë×îºó±£´æµÄÑü´ø´°¿ÚÊý¾Ý½øÐÐ±È½Ï£¬²¢·ÖÅäµ½ÕýÈ·µÄÎ»ÖÃ¡£
		for ( nCnt1 = 0; nCnt1 < _MAX_BELT_ITEM; nCnt1++ )
		{
			if ( stBeltItem[nCnt1].bSetted )
			{
				for ( nCnt2 = 0; nCnt2 < _MAX_INVEN_ITEM; nCnt2++ )
				{
					if ( m_stInventoryItem[nCnt2].bSetted )
					{
						// ÅÐ¶ÏÊÇ·ñÎªÏàÍ¬ÎïÆ·.
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

		// ±³°ü.
		for ( nCnt1 = 0; nCnt1 < _MAX_INVEN_ITEM; nCnt1++ )
		{
			if ( stInvenItem[nCnt1].bSetted )
			{
				for ( nCnt2 = 0; nCnt2 < _MAX_INVEN_ITEM; nCnt2++ )
				{
					if ( m_stInventoryItem[nCnt2].bSetted )
					{
						// ÅÐ¶ÏÊÇ·ñÎªÏàÍ¬ÎïÆ·.
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

	º¯ÊýÃû : CInventoryWnd::GetInvenNum()

	×÷Õß :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : »ñÈ¡Êó±êÎ»ÖÃ´¦µ±Ç°±³°üµÄµ¥Ôª¸ñ±àºÅ¡£
	ÊäÈë   : POINT ptMouse
	Êä³ö   : INT

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

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

	º¯ÊýÃû : CInventoryWnd::GetBeltNum()

	×÷Õß :
	´´½¨ÈÕÆÚ :

	Ä¿µÄ   : È¡µÃÊó±êÎ»ÖÃ´¦µ±Ç°Ñü´øµÄ¸ñ×Ó±àºÅ.
	ÊäÈë   : POINT ptMouse
	Êä³ö   : INT

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

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

	º¯ÊýÃû : CInventoryWnd::DeleteInventoryCurrItem()

	×÷Õß :
	´´½¨ÈÕÆÚ :

	Ä¿µÄ   : É¾³ý±³°üÖÐ¶ÔÓ¦¸ñ×ÓµÄÎïÆ·.
	ÊäÈë   : INT nInventoryNum
	Êä³ö   : VOID

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

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

	º¯ÊýÃû : CInventoryWnd::DeleteInventoryAllItem()

	×÷Õß :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : Çå³ý±³°üÖÐµÄËùÓÐÎïÆ·.
	Êä³ö   : VOID

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

*******************************************************************************************************************/
VOID CInventoryWnd::DeleteAllInvenItem()
{
	for ( INT nCnt = 0; nCnt < _MAX_INVEN_ITEM; nCnt++ )
	{
		ZeroMemory(&m_stInventoryItem[nCnt], sizeof(ITEMSET));
	}
}



/******************************************************************************************************************

	º¯ÊýÃû : CInventoryWnd::DeleteCurrBeltItemList()

	×÷Õß :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : É¾³ýÑü´ø¶ÔÓ¦¸ñ×ÓµÄÎïÆ·.
	ÊäÈë   : INT nBeltInventoryNum
	Êä³ö   : VOID

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

*******************************************************************************************************************/
VOID CInventoryWnd::DeleteBeltItem(INT nBeltInventoryNum)
{
	ZeroMemory(&m_stBeltItem[nBeltInventoryNum], sizeof(ITEMSET));
}



/******************************************************************************************************************

	º¯ÊýÃû : CInventoryWnd::DeleteAllBeltItemList()

	×÷Õß :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : É¾³ýÑü´øÖÐµÄËùÓÐÎïÆ·.
	ÊäÈë   : INT nBeltInventoryNum
	Êä³ö   : VOID

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

*******************************************************************************************************************/
VOID CInventoryWnd::DeleteAllBeltItem()
{
	for ( INT nCnt = 0; nCnt < _MAX_BELT_ITEM; nCnt++ )
	{
		ZeroMemory(&m_stBeltItem[nCnt], sizeof(ITEMSET));
	}
}



/******************************************************************************************************************

	º¯ÊýÃû : CInventoryWnd::AddInvenItem()

	×÷Õß :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : ½«ÎïÆ··ÅÈë±³°ü¡£
	ÊäÈë   : CItem xItem
	         INT nInvenNum
	         BOOL bUseInvenNum
	Êä³ö   : BOOL

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

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

	º¯ÊýÃû : CInventoryWnd::AddBeltItem()

	×÷Õß :
	´´½¨ÈÕÆÚ :

	Ä¿µÄ   : °ÑÎïÆ··ÅÈëÑü´ø.
	ÊäÈë   : CItem xItem
	         INT nBeltNum
	         BOOL bUseBeltNum
	Êä³ö   : BOOL

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

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

	º¯ÊýÃû : CInventoryWnd::ExtractInvenItem()

	×÷Õß :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : ´Ó±³°üÖÐÈ¡³öµ±Ç°±àºÅµÄÎïÆ·¡£
	ÊäÈë   : CItem* pxItem
	         INT nInventoryNum
	Êä³ö   : BOOL

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

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

	º¯ÊýÃû : CInventoryWnd::ExtractBeltItem()

	×÷Õß :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : ´Ó±³°ü´°¿ÚÖÐÈ¡³öµ±Ç°±àºÅµÄÎïÆ·.
	ÊäÈë   : CItem* pxItem
	         INT nBeltInventoryNum
	Êä³ö   : BOOL

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

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

	º¯ÊýÃû : CInventoryWnd::ChangeInvenWithCommon()

	×÷Õß :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : ½«±³°üÖÐµÄÎïÆ·Óë¹«¹²(Êó±ê)ÎïÆ·½»»».
	ÊäÈë   : INT nInventoryNum
	         LPCOMMONITEMSET pstCommonItemSet
	Êä³ö   : VOID

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

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

	º¯ÊýÃû : CInventoryWnd::ChangeBeltWithCommon()

	×÷Õß :
	ÈÕÆÚ :

	Ä¿µÄ   : ¸ü»»Ñü´øÎïÆ·Óë¹«¹²(Êó±ê)ÎïÆ·¡£
	ÊäÈë   : INT nBeltInventoryNum
	         LPCOMMONITEMSET pstCommonItemSet
	Êä³ö   : VOID

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

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

	º¯ÊýÃû : CInventoryWnd::SetCommonFromInven()

	×÷Õß :
	´´½¨ÈÕÆÚ :

	Ä¿µÄ   : °Ñ±³°üÖÐ¶ÔÓ¦±àºÅµÄÎïÆ·ÉèÖÃÎª¹«¹²(Êó±ê)ÎïÆ·.
	ÊäÈë   : INT nInventoryNum
	         LPCOMMONITEMSET pstCommonItemSet
	Êä³ö   : VOID

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

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

	º¯ÊýÃû : CInventoryWnd::SetCommonFromBelt()

	×÷Õß :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : °ÑÑü´ø´°¿ÚÖÐ¶ÔÓ¦±àºÅµÄÎïÆ·ÉèÖÃÎª¹«¹²(Êó±ê)ÎïÆ·¡£
	ÊäÈë   : INT nBeltInventoryNum
	         LPCOMMONITEMSET pstCommonItemSet
	Êä³ö   : VOID

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

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

	º¯ÊýÃû : CInventoryWnd::SetInvenFromCommon()

	×÷Õß :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : ½«¹«¹²(Êó±ê)ÎïÆ·ÉèÖÃµ½±³°üµÄ¶ÔÓ¦±àºÅ¡£
	ÊäÈë   : INT nInventoryNum
	         LPCOMMONITEMSET pstCommonItemSet
	Êä³ö   : VOID

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

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

	º¯ÊýÃû : CInventoryWnd::SetBeltFromCommon()

	×÷Õß :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : °Ñ¹«¹²(Êó±ê)ÎïÆ·ÉèÖÃµ½Ñü´ø´°¿ÚµÄ¶ÔÓ¦±àºÅÉÏ.
	ÊäÈë   : INT nBeltInventoryNum
	         LPCOMMONITEMSET pstCommonItemSet
	Êä³ö   : VOID

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

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

	Message Function.(Êó±êÊäÈë)

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

	// ¹Ø±Õ±³°ü´°¿Ú.
	if ( m_xInventoryBtn[_BTN_ID_INVENCLOSE].OnLButtonUp(ptMouse) )
		return TRUE;

	// ¹ö¶¯ Up, Down ´¦Àí.
//	m_xInventoryBtn[_BTN_ID_INVENSCRLUP	 ].OnLButtonUp(ptMouse);
//	m_xInventoryBtn[_BTN_ID_INVENSCRLDOWN].OnLButtonUp(ptMouse);

	nInventoryNum	  = GetInvenNum(ptMouse);
	nBeltInventoryNum = GetBeltNum(ptMouse);

	// Í¨ÓÃ(Êó±ê)ÎïÆ·À¸ÖÐÓÐÀ´×Ô×°±¸´°¿ÚµÄÎïÆ·£¬ÇÒ²»ÔÚµÈ´ý·þÎñÆ÷ÏìÓ¦ÐÅºÅµÄ×´Ì¬Ê±£¬¹Ø±ÕÎïÆ·´©´÷.
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

	// ÔÚ±³°ü¸ñ×ÓÇøÓòÄÚµã»÷µÄÇé¿ö.
	if ( nInventoryNum != -1 )
	{
		// µã»÷µÄ±³°üÖÐÓÐÎïÆ·Ê±¡£
		if ( m_stInventoryItem[nInventoryNum].bSetted )
		{
			// Èç¹û¹«¹²(Êó±ê)ÎïÆ·¼¯Îª¿Õ£¬
			if ( !pstCommonItemSet->bSetted )
			{
				// °ÑÎïÆ·¸½×ÅÔÚÊó±êÉÏ.
				SetCommonFromInven(nInventoryNum, pstCommonItemSet);
			}
			// ´æÔÚ¹«¹²(Êó±ê)ÎïÆ·¼¯, ²»ÔÚµÈ´ý·þÎñÆ÷Ó¦´ðÐÅºÅµÄ¹ý³ÌÖÐ, ÇÒ²»ÊÇÒÑ´©´÷ÎïÆ·Ê±.
			else if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse && !pstCommonItemSet->bIsEquipItem )
			{
				ChangeInvenWithCommon(nInventoryNum, pstCommonItemSet);
			}
		}
		// µã»÷µÄ±³°üÖÐÃ»ÓÐÎïÆ·Ê±¡£
		else
		{
			// ´æÔÚ¹«¹²(Êó±ê)ÎïÆ·¼¯£¬ÇÒ²»ÔÚµÈ´ý·þÎñÆ÷ÏìÓ¦ÐÅºÅÊ±£¬
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
		// µã»÷µÄ±³°üÖÐÓÐÎïÆ·Ê±¡£
		if ( m_stInventoryItem[nInventoryNum].bSetted )
		{
			// Èç¹û¹«¹²(Êó±ê)ÎïÆ·¼¯Îª¿Õ£¬
			if ( !pstCommonItemSet->bSetted )
			{
				// °ÑÎïÆ·¸½×ÅÔÚÊó±êÉÏ.
				SetCommonFromInven(nInventoryNum, pstCommonItemSet);
			}
			// ´æÔÚ¹«¹²(Êó±ê)ÎïÆ·¼¯£¬ÇÒ²»ÔÚµÈ´ý·þÎñÆ÷ÏìÓ¦ÐÅºÅÊ±£¬
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

	Ä£¿éÃû:

	×÷Õß:
	±àÐ´ÈÕÆÚ:

	[ÈÕÆÚ][ÐÞ¸ÄÈË] : ÐÞ¸ÄÄÚÈÝ

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

	// °´Å¥³õÊ¼»¯.
	m_xStatusBtn[_BTN_ID_STATUSCLOSE].CreateGameBtn(pxWndImage,  214,  215, nStartX+222, nStartY+298);
	m_xStatusBtn[_BTN_ID_STATUS].CreateGameBtn(pxWndImage,  212,  213, nStartX+_BTN_ID_STATUS_X, nStartY+_BTN_ID_STATUS_Y);
	m_xStatusBtn[_BTN_ID_STATUSMAGIC].CreateGameBtn(pxWndImage,  99, 100, nStartX+_BTN_ID_STATUSMAGIC_X, nStartY+_BTN_ID_STATUSMAGIC_Y);

	// ×°±¸´°¿Ú³õÊ¼»¯¡£
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

	// Ä§·¨°´Å¥³õÊ¼»¯.
	m_xMagicScrlBtn[0].CreateGameBtn(pxWndImage, 74, 75, nStartX+232, nStartY+147);
	m_xMagicScrlBtn[1].CreateGameBtn(pxWndImage, 74, 75, nStartX+232, nStartY+253);

	// Ä§·¨´°¿Ú³õÊ¼»¯.
	SetRect(&m_rcMagicCell[0], 39,  77, 74, 112);
	SetRect(&m_rcMagicCell[1], 39, 115, 74, 149);
	SetRect(&m_rcMagicCell[2], 39, 152, 74, 186);
	SetRect(&m_rcMagicCell[3], 39, 188, 74, 223);
	SetRect(&m_rcMagicCell[4], 39, 225, 74, 260);

	// Ä§·¨±äÁ¿³õÊ¼»¯.
	m_pstMyMagic  = NULL;
	m_bMyMagicCnt = 0;
}



VOID CStatusWnd::ShowStatusWnd()
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

	½ÇÉ«×°±¸´°¿ÚÏà¹Ø.

*******************************************************************************************************************/

VOID CStatusWnd::ShowCharEquipWnd()
{
	INT nStartX, nStartY;

	ShowGameWnd();

	// Çø·ÖÊÇÄÐÐÔ»¹ÊÇÅ®ÐÔ.
	if ( g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].NewSetIndex(g_xGameProc.m_xMyHero.m_stFeature.bGender) )
	{
		g_xMainWnd.DrawWithImageForComp(
						m_rcWnd.left+_EQUIP_CHAR_X+g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].m_lpstNewCurrWilImageInfo->shPX,
						m_rcWnd.top+_EQUIP_CHAR_Y+g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].m_lpstNewCurrWilImageInfo->shPY,
						g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].m_lpstNewCurrWilImageInfo->shWidth,
						g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].m_lpstNewCurrWilImageInfo->shHeight,
						(WORD*)(g_xGameProc.m_xImage.m_xImageList[_IMAGE_PROGUSE].m_pbCurrImage));
	}

	// ÎïÆ·»æÖÆ.
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

	½ÇÉ«×´Ì¬ÐÅÏ¢´°¿ÚÏà¹Ø.

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

	½ÇÉ«Ä§·¨´°¿ÚÏà¹Ø.

*******************************************************************************************************************/
VOID CStatusWnd::ShowCharSetMagicWnd()
{
	INT nLine, nCnt, nStartX, nStartY;
	CHAR szKey[MAX_PATH];
	CHAR szLevel[MAX_PATH];

/*	CHAR* szSkillName[] =
	{
		"°ëÔÂ½£·¨",   "°µÈ¼Êõ",   "»ðÑæ·ç",   "½µÄ§Õæ·¨", "´óµØÔ®»¤",   "ÑæÉßÕÆ",	"ÎÞµ¡²½",     "À×»ê»÷",   "ÑÇ¿ÕÐÐ·¨", "Ê¨×ÓÂÖ»Ø", "»ðÑæÕÆ",	    "»Ö¸´Êõ",
		"½á½ç",	      "´ó»Ö¸´",   "À×Ñ©»¨",   "À×Ó¡ÕÆ",   "°×¹ÇÕÙ»½Êõ", "±ùÑ©·ç",	"´óÒþÉí",     "ÖäÊõÖ®Ä»", "Ì½ÆøÆÆÈ¼", "µØÑæÊõ",   "±¬É±½ç",     "±¬ÁÑ²¨",
		"½ð¸Õ»ðÑæÕÆ", "Èñµ¶",	  "Óù½£Êõ",	  "Ç¿»÷",	  "Ñæ»ð¾÷",     "ÒþÉí",		"ÉñÊÞÕÙ»½",	  "ÈÕ¹â½£·¨",  "ÍâÐÞ½£·¨",
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

	// ×°±¸´°¿Ú´¦ÓÚ¼¤»î×´Ì¬Ê±.
	if ( m_bType == _TYPE_EQUIP || m_bType == _TYPE_STATUS )
	{
		nEquipNum = GetEquipNum(ptMouse);
		// Êó±êÎ»ÓÚ×°±¸´°¿ÚÇøÓòÊ±.
		if ( nEquipNum != -1)
		{
			// µã»÷µÄ×°±¸´°¿ÚÖÐÓÐÎïÆ·Ê±.
			if ( m_stEquipItem[nEquipNum].bSetted )
			{
				// Èç¹û¹«¹²(Êó±ê)ÎïÆ·¼¯Îª¿Õ£¬
				if ( !pstCommonItemSet->bSetted )
				{
					// °ÑÎïÆ·¹ÒÔÚÊó±êÉÏÊ±, ÔÚ±³°ü´°¿Úµã»÷µÄË²¼ä»á·¢ËÍ CM_TAKEOFFITEM ÏûÏ¢.
					SetCommonFromEquip(nEquipNum, pstCommonItemSet);
				}
				// ´æÔÚ¹«¹²(Êó±ê)ÎïÆ·¼¯£¬ÇÒ²»ÔÚµÈ´ý·þÎñÆ÷ÏìÓ¦ÐÅºÅÊ±£¬
				// °ÑÎïÆ·Ô¤ÏÈÉèÖÃµ½¹«¹²ÎïÆ· m_stTakeOnItemSet ÖÐ, ÔÙ°Ñµ±Ç°µã»÷µÄ×°±¸´°¿ÚÎïÆ·ÉèÖÃÎª¹«¹²ÎïÆ·ºó,
				// ·¢ËÍ CM_TAKEONITEM ÏûÏ¢.
				else if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse )
				{
					// ¼ì²é·¢ËÍÇ°µÄÇøÓòÊÇ·ñÏà»¥Æ¥Åä(½äÖ¸ÊÇ·ñ·Å½øÁË½äÖ¸Î»ÖÃ).
					if ( CheckEquipItem(&pstCommonItemSet->xItem, nEquipNum, ptMouse) )
					{
						SetTakeOnFromCommon(nEquipNum, pstCommonItemSet);
						SetCommonFromEquip(nEquipNum, pstCommonItemSet);
						g_xClientSocket.SendTakeOnOffItem(CM_TAKEONITEM, nEquipNum, m_stTakeOnItemSet.xItem.m_stItemInfo.stStdItem.szName, m_stTakeOnItemSet.xItem.m_stItemInfo.nMakeIndex);
						pstCommonItemSet->bWaitResponse	= TRUE;
					}
				}
			}
			// µã»÷µÄ×°±¸´°¿ÚÖÐÃ»ÓÐÎïÆ·Ê±.
			else
			{
				// ´æÔÚ¹«¹²(Êó±ê)ÎïÆ·¼¯, ÇÒ²»ÊÇÔÚµÈ´ý·þÎñÆ÷ÏìÓ¦ÐÅºÅÊ±, ¾Í°Ñ¹«¹²ÎïÆ·ÉèÖÃµ½ m_stTakeOnItemSet ÖÐ.
				if ( pstCommonItemSet->bSetted && !pstCommonItemSet->bWaitResponse )
				{
					if ( CheckEquipItem(&pstCommonItemSet->xItem, nEquipNum, ptMouse) )
					{
						// ÎÞÐèÏò·þÎñÆ÷·¢ËÍÏûÏ¢¡£Ð¶ÏÂÎïÆ·ºóÓÖÖØÐÂ´©´÷µÄÇé¿ö¡£
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

// ExchangeWnd
/******************************************************************************************************************

	Ä£¿éÃû:

	×÷Õß:
	±àÐ´ÈÕÆÚ:

	[ÈÕÆÚ][ÐÞ¸ÄÈË] : ÐÞ¸ÄÄÚÈÝ

*******************************************************************************************************************/






CExchangeWnd::CExchangeWnd()
{
	Init();
}

CExchangeWnd::~CExchangeWnd()
{
	Destroy();
}

VOID CExchangeWnd::Init()
{
	CGameWnd::Init();

	m_xExchangeBtn.Init();
}

VOID CExchangeWnd::Destroy()
{
	CGameWnd::Destroy();
}


VOID CExchangeWnd::CreateExchangeWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove)
{
	CreateGameWnd(nID, pxWndImage, nFrameImgIdx, bCanMove, nStartX, nStartY, nWidth, nHeight);
	m_xExchangeBtn.CreateGameBtn(pxWndImage, 280, 281, nStartX+255, nStartY+291);
}

VOID CExchangeWnd::ShowExchangeWnd()
{
	ShowGameWnd();
	m_xExchangeBtn.ShowGameBtn();
}

VOID CExchangeWnd::OnMouseMove(POINT ptMouse)
{
	m_xExchangeBtn.ChangeRect(m_rcWnd.left+567, m_rcWnd.top+413);
	m_xExchangeBtn.OnMouseMove(ptMouse);
}

BOOL CExchangeWnd::OnLButtonUp(POINT ptMouse)
{
	if ( m_xExchangeBtn.OnLButtonUp(ptMouse) )		return TRUE;
	return FALSE;
}

BOOL CExchangeWnd::OnLButtonDown(POINT ptMouse)
{
	if ( m_xExchangeBtn.OnLButtonDown(ptMouse) )		return TRUE;
	return FALSE;
}

VOID CExchangeWnd::SetStatusBtnInit()
{
	m_xExchangeBtn.SetBtnState(_BTN_STATE_NORMAL);
}

// GuildMasterWnd
/******************************************************************************************************************

	Ä£¿éÃû:

	×÷Õß:
	±àÐ´ÈÕÆÚ:

	[ÈÕÆÚ][ÐÞ¸ÄÈË] : ÐÞ¸ÄÄÚÈÝ

*******************************************************************************************************************/





CGuildMasterWnd::CGuildMasterWnd()
{
	Init();
}

CGuildMasterWnd::~CGuildMasterWnd()
{
	Destroy();
}

VOID CGuildMasterWnd::Init()
{
	CGameWnd::Init();

	m_xGuildMasterBtn.Init();
}

VOID CGuildMasterWnd::Destroy()
{
	CGameWnd::Destroy();
}

VOID CGuildMasterWnd::CreateGuildMasterWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove)
{
	CreateGameWnd(nID, pxWndImage, nFrameImgIdx, bCanMove, nStartX, nStartY, nWidth, nHeight);
	m_xGuildMasterBtn.CreateGameBtn(pxWndImage, 630, 631, nStartX+567, nStartY+413);
}

VOID CGuildMasterWnd::ShowGuildMasterWnd()
{
	ShowGameWnd();
	m_xGuildMasterBtn.ShowGameBtn();
}

BOOL CGuildMasterWnd::OnLButtonUp(POINT ptMouse)
{
	if ( m_xGuildMasterBtn.OnLButtonUp(ptMouse) )		return TRUE;

	return FALSE;
}

BOOL CGuildMasterWnd::OnLButtonDown(POINT ptMouse)
{
	if ( m_xGuildMasterBtn.OnLButtonDown(ptMouse) )		return TRUE;

	return FALSE;
}

VOID CGuildMasterWnd::OnMouseMove(POINT ptMouse)
{
	m_xGuildMasterBtn.ChangeRect(m_rcWnd.left+567, m_rcWnd.top+413);
	m_xGuildMasterBtn.OnMouseMove(ptMouse);
}

VOID CGuildMasterWnd::SetStatusBtnInit()
{
	m_xGuildMasterBtn.SetBtnState(_BTN_STATE_NORMAL);
}

// GuildWnd
/******************************************************************************************************************

	Ä£¿éÃû:

	×÷Õß:
	±àÐ´ÈÕÆÚ:

	[ÈÕÆÚ][ÐÞ¸ÄÈË] : ÐÞ¸ÄÄÚÈÝ

*******************************************************************************************************************/





CGuildWnd::CGuildWnd()
{
	Init();
}

CGuildWnd::~CGuildWnd()
{
	Destroy();
}

VOID CGuildWnd::Init()
{
	CGameWnd::Init();
}

VOID CGuildWnd::Destroy()
{
	CGameWnd::Destroy();
}

VOID CGuildWnd::CreateGuildWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove)
{
	CreateGameWnd(nID, pxWndImage, nFrameImgIdx, bCanMove, nStartX, nStartY, nWidth, nHeight);
}

VOID CGuildWnd::ShowGuildWnd()
{
	ShowGameWnd();
}

BOOL CGuildWnd::OnLButtonUp(POINT ptMouse)
{
	return FALSE;
}

BOOL CGuildWnd::OnLButtonDown(POINT ptMouse)
{
	return FALSE;
}

VOID CGuildWnd::OnMouseMove(POINT ptMouse)
{
}

// GroupWnd
/******************************************************************************************************************

	Ä£¿éÃû:

	×÷Õß:
	±àÐ´ÈÕÆÚ:

	[ÈÕÆÚ][ÐÞ¸ÄÈË] : ÐÞ¸ÄÄÚÈÝ

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

VOID CGroupWnd::ShowGroupWnd()
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

VOID CGroupWnd::Init()
{
	CGameWnd::Init();
}

VOID CGroupWnd::Destroy()
{
	CGameWnd::Destroy();
}

BOOL CGroupWnd::OnLButtonUp(POINT ptMouse)
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

BOOL CGroupWnd::OnLButtonDown(POINT ptMouse)
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

	Ä£¿éÃû:

	×÷Õß:
	±àÐ´ÈÕÆÚ:

	[ÈÕÆÚ][ÐÞ¸ÄÈË] : ÐÞ¸ÄÄÚÈÝ

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

VOID CGroupPopWnd::Init()
{
	CGameWnd::Init();
}

VOID CGroupPopWnd::Destroy()
{
	CGameWnd::Destroy();
}


VOID CGroupPopWnd::ShowGroupPopWnd()
{
	ShowGameWnd();
}

BOOL CGroupPopWnd::OnLButtonUp(POINT ptMouse)
{
	return FALSE;
}

BOOL CGroupPopWnd::OnLButtonDown(POINT ptMouse)
{
	return FALSE;
}

VOID CGroupPopWnd::OnMouseMove(POINT ptMouse)
{
}

// ChatPopWnd
/******************************************************************************************************************

	Ä£¿éÃû:

	×÷Õß:
	±àÐ´ÈÕÆÚ:

	[ÈÕÆÚ][ÐÞ¸ÄÈË] : ÐÞ¸ÄÄÚÈÝ

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

VOID CChatPopWnd::ShowChatPopWnd()
{
	ShowGameWnd();

	// ÏÔÊ¾ÁÄÌìÁÐ±í.
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

BOOL CChatPopWnd::OnLButtonUp(POINT ptMouse)
{
	MoveWindow(g_xChatEditBox.GetSafehWnd(), g_xMainWnd.m_rcWindow.left + m_rcWnd.left + m_rcEditBoxFrame.left,
			   g_xMainWnd.m_rcWindow.top + m_rcWnd.top + m_rcEditBoxFrame.top,
			   m_rcEditBoxFrame.right - m_rcEditBoxFrame.left,
			   m_rcEditBoxFrame.bottom - m_rcEditBoxFrame.top, TRUE);

	if ( m_xChatPopBtn.OnLButtonUp(ptMouse) )		return TRUE;

	return FALSE;
}

BOOL CChatPopWnd::OnLButtonDown(POINT ptMouse)
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

// QuestWnd
/******************************************************************************************************************

	Ä£¿éÃû:

	×÷Õß:
	±àÐ´ÈÕÆÚ:

	[ÈÕÆÚ][ÐÞ¸ÄÈË] : ÐÞ¸ÄÄÚÈÝ

*******************************************************************************************************************/





CQuestWnd::CQuestWnd()
{
	Init();
}

CQuestWnd::~CQuestWnd()
{
	Destroy();
}

VOID CQuestWnd::Init()
{
	CGameWnd::Init();

	m_xQuestBtn.Init();
}

VOID CQuestWnd::Destroy()
{
	CGameWnd::Destroy();
}

VOID CQuestWnd::CreateQuestWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove)
{
	CreateGameWnd(nID, pxWndImage, nFrameImgIdx, bCanMove, nStartX, nStartY, nWidth, nHeight);
	m_xQuestBtn.CreateGameBtn(pxWndImage, 630, 631, nStartX+567, nStartY+413);
}

VOID CQuestWnd::ShowQuestWnd()
{
	ShowGameWnd();
	m_xQuestBtn.ShowGameBtn();
}

BOOL CQuestWnd::OnLButtonUp(POINT ptMouse)
{
	if ( m_xQuestBtn.OnLButtonUp(ptMouse) )		return TRUE;

	return FALSE;
}

BOOL CQuestWnd::OnLButtonDown(POINT ptMouse)
{
	if ( m_xQuestBtn.OnLButtonDown(ptMouse) )		return TRUE;

	return FALSE;
}

VOID CQuestWnd::OnMouseMove(POINT ptMouse)
{
	m_xQuestBtn.ChangeRect(m_rcWnd.left+567, m_rcWnd.top+413);
	m_xQuestBtn.OnMouseMove(ptMouse);
}

VOID CQuestWnd::SetStatusBtnInit()
{
	m_xQuestBtn.SetBtnState(_BTN_STATE_NORMAL);
}

// OptionWnd
/******************************************************************************************************************

	Ä£¿éÃû:

	×÷Õß:
	±àÐ´ÈÕÆÚ:

	[ÈÕÆÚ][ÐÞ¸ÄÈË] : ÐÞ¸ÄÄÚÈÝ

*******************************************************************************************************************/





COptionWnd::COptionWnd()
{
	Init();
}

COptionWnd::~COptionWnd()
{
	Destroy();
}

VOID COptionWnd::Init()
{
	CGameWnd::Init();

	m_xOptionBtn.Init();
}

VOID COptionWnd::Destroy()
{
	CGameWnd::Destroy();
}

VOID COptionWnd::CreateOptionWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove)
{
	CreateGameWnd(nID, pxWndImage, nFrameImgIdx, bCanMove, nStartX, nStartY, nWidth, nHeight);
	m_xOptionBtn.CreateGameBtn(pxWndImage, 764, 765, nStartX+220, nStartY+11);
}

VOID COptionWnd::ShowOptionWnd()
{
	ShowGameWnd();
	m_xOptionBtn.ShowGameBtn();
}

BOOL COptionWnd::OnLButtonUp(POINT ptMouse)
{
	if ( m_xOptionBtn.OnLButtonUp(ptMouse) )		return TRUE;

	return FALSE;
}

BOOL COptionWnd::OnLButtonDown(POINT ptMouse)
{
	if ( m_xOptionBtn.OnLButtonDown(ptMouse) )		return TRUE;

	return FALSE;
}

VOID COptionWnd::OnMouseMove(POINT ptMouse)
{
	m_xOptionBtn.ChangeRect(m_rcWnd.left+220, m_rcWnd.top+11);
	m_xOptionBtn.OnMouseMove(ptMouse);
}

VOID COptionWnd::SetStatusBtnInit()
{
	m_xOptionBtn.SetBtnState(_BTN_STATE_NORMAL);
}

// HorseWnd
/******************************************************************************************************************

	Ä£¿éÃû:

	×÷Õß:
	±àÐ´ÈÕÆÚ:

	[ÈÕÆÚ][ÐÞ¸ÄÈË] : ÐÞ¸ÄÄÚÈÝ

*******************************************************************************************************************/





CHorseWnd::CHorseWnd()
{
	Init();
}

CHorseWnd::~CHorseWnd()
{
	Destroy();
}

VOID CHorseWnd::Init()
{
	CGameWnd::Init();

	m_xHorseBtn.Init();
}

VOID CHorseWnd::Destroy()
{
	CGameWnd::Destroy();
}

VOID CHorseWnd::CreateHorseWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove)
{
	CreateGameWnd(nID, pxWndImage, nFrameImgIdx, bCanMove, nStartX, nStartY, nWidth, nHeight);
	m_xHorseBtn.CreateGameBtn(pxWndImage, 630, 631, nStartX+567, nStartY+413);
}

VOID CHorseWnd::ShowHorseWnd()
{
	ShowGameWnd();
	m_xHorseBtn.ShowGameBtn();
}

BOOL CHorseWnd::OnLButtonUp(POINT ptMouse)
{
	if ( m_xHorseBtn.OnLButtonUp(ptMouse) )		return TRUE;

	return FALSE;
}

BOOL CHorseWnd::OnLButtonDown(POINT ptMouse)
{
	if ( m_xHorseBtn.OnLButtonDown(ptMouse) )		return TRUE;

	return FALSE;
}

VOID CHorseWnd::OnMouseMove(POINT ptMouse)
{
	m_xHorseBtn.ChangeRect(m_rcWnd.left+567, m_rcWnd.top+413);
	m_xHorseBtn.OnMouseMove(ptMouse);
}

VOID CHorseWnd::SetStatusBtnInit()
{
	m_xHorseBtn.SetBtnState(_BTN_STATE_NORMAL);
}

// NPCWnd
/******************************************************************************************************************

	Ä£¿éÃû:

	×÷Õß:
	±àÐ´ÈÕÆÚ:

	[ÈÕÆÚ][ÐÞ¸ÄÈË] : ÐÞ¸ÄÄÚÈÝ

*******************************************************************************************************************/


#define MAX_NPCSTR_HEIGHT		129
#define MAX_SHOW_LINE			6
#define POS_DRAWSTART_Y			40
#define LINE_GAP				5

#define BUILD_GUILD				"@@BUILDGUILDNOW"			// ÐÐ»á½¨Á¢
#define GUILD_WAR				"@@GUILDWAR"				// ÐÐ»áÕ½ÉêÇë
#define WITHDRAWAL_CASTLE		"@@WITHDRAWAL"				// È¡³öÇ®¿î.
#define RECEIPT_CASTLE			"@@RECEIPTS"				// ¼Ä´æ½ðÇ®.

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

VOID CNPCWnd::ShowNPCChatWnd()
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

BOOL CNPCWnd::OnLButtonDown(POINT ptMouse)
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

BOOL CNPCWnd::OnLButtonUp(POINT ptMouse)
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

BOOL	CNPCWnd::ClickCommandProcess(CMTxtBtn* ClickedBtn)		// ×ÛºÏÏÂÃæÕâÐ©ÄÚÈÝ
{
/*	m_pClickedTxtBtn = ClickedBtn;
	if(ClickedBtn!=NULL)
	{
		if(strcmp(m_xTxtAnalysis.UpperCase(ClickedBtn->Param),BUILD_GUILD)==0)				// ÐÐ»á½¨Á¢´¦Àí
		{
			CHAR	szMsg[MAX_PATH];
			HINSTANCE hLib;
			hLib = LoadLibrary("Ats.dll"); // Load Dll
			LoadString(hLib,ASK_MAKE_BIG_GUILD,szMsg,MAX_PATH);
			FreeLibrary(hLib);
			if(!m_xMsgBox.IsActive())	SetWndRectExtend();
			m_xMsgBox.ShowMessageBox(szMsg,MSG_EDITEXIST|MSG_BTN_OK);	// OK
			SetNpcEditBoxPos();
			// ´ò¿ªÐÐ»á½¨Á¢´°¿Ú²¢ÊäÈëÐÐ»áÃû³Æ.
			m_nNpcMsgState = 1;
			return TRUE;
		}
		else if(strcmp(m_xTxtAnalysis.UpperCase(ClickedBtn->Param),GUILD_WAR)==0)			// ÐÐ»áÕ½Õù
		{
			CHAR	szMsg[MAX_PATH];
			HINSTANCE hLib;
			hLib = LoadLibrary("Ats.dll"); // Load Dll
			LoadString(hLib,ASK_OTHERSNAME,szMsg,MAX_PATH);
			FreeLibrary(hLib);
			if(!m_xMsgBox.IsActive())	SetWndRectExtend();
			m_xMsgBox.ShowMessageBox(szMsg,MSG_EDITEXIST|MSG_BTN_OK);	// OK
			SetNpcEditBoxPos();
			// ÊäÈëÒª½»Õ½µÄÐÐ»á.
			m_nNpcMsgState = 2;
			return TRUE;
		}
		else if(strcmp(m_xTxtAnalysis.UpperCase(ClickedBtn->Param),WITHDRAWAL_CASTLE)==0)	// ´Ó³Ç½ð¿âÖÐÈ¡Ç®
		{
			CHAR	szMsg[MAX_PATH];
			HINSTANCE hLib;
			hLib = LoadLibrary("Ats.dll"); // Load Dll
			LoadString(hLib,ASK_WITHDRAWAL,szMsg,MAX_PATH);
			FreeLibrary(hLib);
			if(!m_xMsgBox.IsActive())	SetWndRectExtend();
			m_xMsgBox.ShowMessageBox(szMsg,MSG_EDITEXIST|MSG_BTN_OK);	// OK
			SetNpcEditBoxPos();
			// ´ò¿ª³Ç½ð¿â´°¿Ú²¢ÊäÈëÒªÈ¡³öµÄ½ð¶î.
			m_nNpcMsgState = 3;
			return TRUE;
		}
		else if(strcmp(m_xTxtAnalysis.UpperCase(ClickedBtn->Param),RECEIPT_CASTLE)==0)		// Ïò³Ç½ð¿âÖÐ´æÇ®
		{
			CHAR	szMsg[MAX_PATH];
			HINSTANCE hLib;
			hLib = LoadLibrary("Ats.dll"); // Load Dll
			LoadString(hLib,ASK_RECEIPT,szMsg,MAX_PATH);
			FreeLibrary(hLib);
			if(!m_xMsgBox.IsActive())	SetWndRectExtend();
			m_xMsgBox.ShowMessageBox(szMsg,MSG_EDITEXIST|MSG_BTN_OK);	// OK
			SetNpcEditBoxPos();
			// ´ò¿ª³Ç½ð¿â´°¿Ú²¢ÊäÈëÒª´æÈëµÄ½ð¶î.
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
//		²»½øÐÐÈÎºÎ´¦Àí */
	return FALSE;
}

BOOL CNPCWnd::SendCommandToSrv(CHAR* szMsg)
{
	// Ä¬ÈÏÏûÏ¢´¦Àí
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
	// Óë Edit Box Ïà¹Ø
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
		g_xChatEditBox.SetLimitText(90);// ½«ÁÄÌì´°¿Ú¸ÄÎª Limit length¡£
		MoveWindow(g_xChatEditBox.GetSafehWnd(), g_xMainWnd.m_rcWindow.left+_INPUT_EDITWND_XPOS,
			g_xMainWnd.m_rcWindow.top+_INPUT_EDITWND_YPOS, _INPUT_EDITWND_WIDTH, _INPUT_EDITWND_HEIGHT, TRUE);
		switch(m_nNpcMsgState)
		{
		case 1:
		case 2:
			// ´´½¨ÐÐ»á
			// ÐÐ»áÕ½ÉêÇë
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
			// ²éÕÒÊ¥×Ï½ð
			// Ê¥×Ó½ðÊÜ»÷
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

	Ä£¿éÃû:

	×÷Õß:
	±àÐ´ÈÕÆÚ:

	[ÈÕÆÚ][ÐÞ¸ÄÈË] : ÐÞ¸ÄÄÚÈÝ

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

#define POS_X				84			// ex (Ç®:POS_X,POS_Y)(GOLD:POS__X,POS_Y)
#define POS_Y				246			// ex (ÖØÁ¿:POS_X,POS__Y)(±³°üÖØÁ¿:POS__X,POS__Y)
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


VOID CStoreWnd::ShowStoreWnd()
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
		{// Ñ¡ÖÐ Kinds Ê±
			g_xMainWnd.PutsHan(NULL,tRect.left + POS_ITEMNAME_X+20,tRect.top + POS_ITEMNAME_Y + 10 ,RGB(255,255,255),RGB(0,0,0),"Crescent Blade");

			g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewSetIndex(1); // ÌîÈë¸ÃÎïÆ·µÄ Looks
			g_xMainWnd.DrawWithImageForCompClipRgn(
				tRect.left + POS_ITEMIMAGE_X - (g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].m_lpstNewCurrWilImageInfo->shWidth / 2),tRect.top + POS_ITEMIMAGE_Y - (g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].m_lpstNewCurrWilImageInfo->shHeight / 2)
				,g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].m_lpstNewCurrWilImageInfo->shWidth,g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].m_lpstNewCurrWilImageInfo->shHeight,
				(WORD*)g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].m_pbCurrImage,800,600);
		}

		if(m_nIsReadyToDisplay&&1)
		{	// Kinds List Êä³ö
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
		{	// Item List Êä³ö
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


BOOL CStoreWnd::OnLButtonUp(POINT ptMouse)
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
	{	// ÁÐ±í List
		tBtn = m_xKindBtn.ClickCheck(tPos,ptMouse,m_nCurrentKindTop);
		if(tBtn!=NULL)
		{
			// È·ÈÏ±»°´ÏÂµÄ°´Å¥ÊÇ·ñÓÐ²Ëµ¥.
			// Ã»ÓÐ²Ëµ¥Ê±¾ÍÍ£ÔÚÑ¡Ôñ´¦£¬ÓÐ²Ëµ¥Ê±
			// ÇëÇóÏÂ¼¶²Ëµ¥¡£
			tBtn = m_xKindBtn.ClickCheck(tPos,ptMouse,m_nCurrentKindTop);
			if(tBtn->nColor==RGB(255,255,0))
			{
				// ÓÐ²Ëµ¥Âð?
				// ²»¿ÉÆÛÕ©
				SendDeTailGoodsList(0,tBtn->Param);
			}
			else
			{
				// Ã»ÓÐ²Ëµ¥Âð?
				// ÄÇ¾ÍÖ±½ÓÑ¡Ôñ¡£¿ÉÒÔ×÷±×¡£
			}
		}
		else
		{
			m_nIsReadyToDisplay = 1;
			m_bSelectedKind= FALSE;
		}
	}
	if(IsInRect(ptMouse,m_xItemRect))
	{	// ÏîÄ¿  List
		tBtn = NULL;
		tBtn = m_xItemLstBtn.ClickCheck(tPos,ptMouse,m_nCurrentItemTop);
		if(tBtn!=NULL)
		{
			// ¿ÉÒÔ¹ºÂò
			m_bSelectedItem = TRUE;
			//ÁÙÊ±
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

BOOL CStoreWnd::OnLButtonDown(POINT ptMouse)
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
	CNPCTxtAnalysis	tTxtAnalysis;				// Ö»ÊÇÎªÁËÊ¹ÓÃ GetValidStr º¯Êý..
	CHAR*	szParam;
	CHAR*	pszStr = new CHAR [MAX_MSG_LENGTH];


	if(m_nIsReadyToDisplay&&1)
	{
		m_xKindBtn.ReleaseButtons();
		m_bSelectedKind = FALSE;
		// Çå³ýÒÑ±£´æµÄ°´¼ü
		m_nIsReadyToDisplay= 0;	// Kinds List ÉèÖÃÒÑ½â³ýµÄÍ¨Öª
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
		tRect.left = 0;				// List Button µÄÇé¿ö, Left  = 0;
		tRect.right = tSize.cx;
		tRect.bottom = tSize.cy;
		szParam = new CHAR[35];
		ZeroMemory(szParam,35);
		strcpy(szParam,szGName);	// ÁÙÊ± Parameter ÓëÎïÆ·Ãû³ÆÒ»Æð´¦Àí¡£
		// Parameter...
		if(nSubMenuCount>0)
		{	// ´æÔÚ Sub Menu µÄÇé¿ö
			// Èô´æÔÚ²Ëµ¥ÔòÎªÁÁ»ÆÉ«
			sprintf(szTemp,"%-14s",szGName);
			m_xKindBtn.AddButton(szTemp,tRect,i,szParam,RGB(255,255,0),RGB(255,0,0));	// Add Button to class
		}
		else
		{	// Sub Menu ²»´æÔÚµÄÇé¿ö
			// ²Ëµ¥²»´æÔÚÔòÎª°µ»ÆÉ«
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

	m_nIsReadyToDisplay = 1;		// Í¨Öª Kinds List ÒÑÉèÖÃ
	//
	delete[] pszStr;
	return	TRUE;
}

BOOL CStoreWnd::SetBuyItemLst(CHAR* Msg,INT Count)
{
	CNPCTxtAnalysis	tTxtAnalysis;				// Ö»ÊÇÎªÁËÊ¹ÓÃ GetValidStr º¯Êý..
	CHAR*	szLastMsg = NULL;
	CHAR	szDetailitem[MAX_PATH];

	m_xItemLstBtn.ReleaseButtons();
	m_xItems.ClearAllNodes();
	// Çå³ýÒÑ±£´æµÄ¼ü&Êý¾Ý
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
			CHAR szTemp1[50];	// ¼Û¸ñ
			CHAR szTemp2[50];	// ÆäËû

			ZeroMemory(szTemp1,50);
			ZeroMemory(szTemp2,50);

			m_xItems.AddNode(pxClItem);

//			szMakeIndex[12]; // A:ÎäÆ÷ B:ÒÂ·þ C:½äÖ¸,ÊÖïí,ÏîÁ´ G:ÆäËû
			sprintf(szTemp1,"%d Gold",pxClItem->stStdItem.nPrice);
/*			switch(pxClItem->szMakeIndex[0])
			{
			case 'G':	// ÆäËû
			case 'g':
				{
					switch(pxClItem->stStdItem.bStdMode)
					{
					case 0:	// Ò©Ë®Àà
					case 1:	// ¸ÉÈâ
					case 2:	// ·¹ÍÅ
					case 3:	// ÌØÊâÒ©Ë® or ¸÷ÖÖµäÊé
					case 4:	// ¸÷ÖÖÊé¼®
					case 7:	// À¦°óÏµÁÐ
					case 9:	// Ò©¼Á
						break;
					case 5:	// ¶¾·Û
						sprintf(szTemp2,"%d",pxClItem->nDura);
						break;
					case 8:	// ²»ÄÜÊ³ÓÃµÄÈâ
						sprintf(szTemp2,"Æ·ÖÊ %d",pxClItem->nDura);
						break;
					case 10:	// ¿óÊ¯
						sprintf(szTemp2,"´¿¶È %d",pxClItem->nDura);
						break;
					case 6:	// »ð°Ñ,À¯Öò
					default:	// ÆäËû
						sprintf(szTemp2,"ÄÍ¾Ã %d",pxClItem->nDura);
						break;
					}
					break;
				}
			default:
				sprintf(szTemp2,"ÄÍ¾Ã %d",pxClItem->nDura);
				break;
			}
*/
			RECT	tRect;
			SIZE	tSize;
			ZeroMemory(&tRect,sizeof(RECT));
			tSize = g_xMainWnd.GetStrLength(NULL,NULL,szTemp1);
			tRect.top = 0;
			tRect.left = 0;											// List Button µÄÇé¿ö, Left  = 0;
			tRect.right = m_xItemRect.right - m_xItemRect.left;		//
			tRect.bottom = tSize.cy;

			// Parameter ...
			szParam = new CHAR[35];
			ZeroMemory(szParam,35);
//			strcpy(szParam,pxClItem->szMakeIndex);	// ÁÙÊ± Parameter ÓëÎïÆ·Ãû³ÆÒ»Æð´¦Àí.

			// Dark Yellow
			m_xItemLstBtn.AddButton(szTemp1,szTemp2,tRect,nLoop,szParam,RGB(180,180,0),RGB(255,0,0));	// Add Button to class

			delete[] szParam;
		}
	}
	m_nIsReadyToDisplay=m_nIsReadyToDisplay|2;		// Í¨Öª Item List ÉèÖÃÒÑÍê³É
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
//	if(²»ÊÇÂÃ¹ÝÂð?)
//	{	// Èç¹û²»ÊÇÂÃ¹Ý¾ÍÒª¹ýÂËºóÔÙ·ÅÈë.
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
						{	// ÆäËûÏîÄ¿±ØÐëÔÙ¹ýÂËÒ»´Î.
							m_xItems.AddNode(pxItem);
							strcpy(szTemp1,pxItem->stStdItem.szName);
							itoa(pxItem->nDura,szTemp2,10);
							strcpy(szParam,pxItem->stStdItem.szName);

							tSize = g_xMainWnd.GetStrLength(NULL,NULL,szTemp1);
							tRect.top = 0;
							tRect.left = 0;											// List Button µÄÇé¿ö, Left  = 0;
							tRect.right = m_xItemRect.right - m_xItemRect.left;		//
							tRect.bottom = tSize.cy;

							m_xItemLstBtn.AddButton(szTemp1,szTemp2,tRect,nCount,szParam,RGB(180,180,0),RGB(255,0,0));
							nCount++;
						}
					}
					else
					{
						// ÎäÆ÷»òÒÂ·þÖ±½Ó·ÅÈëÒ²ÎÞ·Á..
						m_xItems.AddNode(pxItem);
						strcpy(szTemp1,pxItem->stStdItem.szName);
						itoa(pxItem->nDura,szTemp2,10);
						strcpy(szParam,pxItem->stStdItem.szName);

						tSize = g_xMainWnd.GetStrLength(NULL,NULL,szTemp1);
						tRect.top = 0;
						tRect.left = 0;											// List Button µÄÇé¿ö, Left  = 0;
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
//	{	// ÈôÊÇÂÃ¹Ý¾Í¿ÉÒÔËæ±ã·ÅÈë¡£
//		for(int i=0 ; i<60; i++)
//		{
//			pxItem = &pxInvenWnd->m_stInventoryItem[i].xItem.m_stItemInfo;
			// ÎäÆ÷»òÒÂ·þÖ±½Ó·ÅÈëÒ²ÎÞ·Á..
//			m_xItems.AddNode(pxItem);
//			strcpy(szTemp1,pxItem->stStdItem.szName);
//			itoa(pxItem->nDura,szTemp2,10);
//			strcpy(szParam,pxItem->stStdItem.szName);

//			tSize = g_xMainWnd.GetStrLength(NULL,NULL,szTemp1);
//			tRect.top = 0;
//			tRect.left = 0;											// List Button µÄÇé¿öÏÂ, Left  = 0;
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
			// ²»ÊÇ¸Ã NPC
			// ´ý×öÊÂÏî
			// ½«¹Ø±Õ NPC ´°¿Ú
			// ½«¸üÐÂ NPC
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

	Ä£¿éÃû:

	×÷Õß:
	±àÐ´ÈÕÆÚ:

	[ÈÕÆÚ][ÐÞ¸ÄÈË] : ÐÞ¸ÄÄÚÈÝ

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

	º¯ÊýÃû : CInterface::Init()

	×÷Õß :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : ½Ó¿ÚÏà¹Ø±äÁ¿¼°¶ÔÏó³õÊ¼»¯.
	Êä³ö   : VOID

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

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

	º¯ÊýÃû : CInterface::Destroy()

	×÷Õß   :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : ½Ó¿ÚÏà¹Ø±äÁ¿¡¢¶ÔÏóµÄÏú»Ù¼°³õÊ¼»¯¡£
	Êä³ö   : VOID

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

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
	// ÁÄÌì±à¼­´°¿ÚµÄÎ»ÖÃÖ¸¶¨.
	MoveWindow(g_xChatEditBox.GetSafehWnd(),
			   g_xMainWnd.m_rcWindow.left+_INPUT_EDITWND_XPOS, g_xMainWnd.m_rcWindow.top+_INPUT_EDITWND_YPOS,
			   _INPUT_EDITWND_WIDTH, _INPUT_EDITWND_HEIGHT, TRUE);
	//------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------
	// ¼ÓÔØ½Ó¿ÚÍ¼Ïñ.
	m_pxInterImg->NewLoad(".\\Data\\gameinter.wil");
	m_xInterImgEx.NewLoad(".\\Data\\interfacec2.wil");
	//------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------
	// Ö÷½çÃæÇøÓòÉèÖÃ.
	if ( m_pxInterImg->NewSetIndex(_WNDIMGIDX_MAIN) )
	{
		SetRect(&m_rcMain,
				0, (600-m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight),
				m_pxInterImg->m_lpstNewCurrWilImageInfo->shWidth, 600);
	}
	//------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------
	// ÁÄÌìÇøÓòÉèÖÃ¡£
	SetRect(&m_rcChat, 224, 471, 578, 545);
	//------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------
	// ´´½¨¸÷´°¿Ú¡£
	m_xInventoryWnd.CreateInventoryWnd(_WND_ID_INVENTORY, m_pxInterImg, _WNDIMGIDX_INVENTORY, 0, 0, 0, 0, TRUE);
	m_xStatusWnd.CreateStatusWnd(_WND_ID_STATUS, m_pxInterImg, _WNDIMGIDX_ITEMSET, 510, 0, 0, 0, TRUE);
	m_xStoreWnd.CreateStoreWnd(_WND_ID_STORE, m_pxInterImg, _WNDIMGIDX_STORE, 0, 0, 0, 0, TRUE);
	m_xExchangeWnd.CreateExchangeWnd(_WND_ID_EXCHANGE, m_pxInterImg, _WNDIMGIDX_EXCHANGE, 0, 0, 0, 0, TRUE);
	m_xGuildMasterWnd.CreateGuildMasterWnd(_WND_ID_GUILDMASTER, m_pxInterImg, _WNDIMGIDX_GUILDMASTER, 0, 0, 0, 0, TRUE);
	m_xGuildWnd.CreateGuildWnd(_WND_ID_GUILD, &m_xInterImgEx, _WNDIMGIDX_GUILD, 0, 0, 540, 442, TRUE);
	m_xGroupWnd.CreateGroupWnd(_WND_ID_GROUP, m_pxInterImg, _WNDIMGIDX_GROUP, 0, 0, 0, 0, TRUE);
	m_xGroupPopWnd.CreateGroupPopWnd(_WND_ID_GROUPPOP, &m_xInterImgEx, _WNDIMGIDX_GROUPPOP, 0, 0, 260, 114, TRUE);
	m_xChatPopWnd.CreateChatPopWnd(_WND_ID_CHATPOP, m_pxInterImg, _WNDIMGIDX_CHATPOP, 0, 0, 0, 0, TRUE);

	m_xOptionWnd.CreateOptionWnd(_WND_ID_OPTION, m_pxInterImg, _WNDIMGIDX_OPTION, 0, 0, 0, 0, TRUE);
	m_xQuestWnd.CreateQuestWnd(_WND_ID_QUEST, m_pxInterImg, _WNDIMGIDX_QUEST, 0, 0, 0, 0, TRUE);
	m_xHorseWnd.CreateHorseWnd(_WND_ID_HORSE, m_pxInterImg, _WNDIMGIDX_HORSE, 0, 0, 0, 0, TRUE);


	m_xNPCWnd.CreateNPCChatWnd(_WND_ID_NPCCHAT, m_pxInterImg, _WNDIMGIDX_NPCCHAT, 0, 0, 0, 0, TRUE);				// ÓÂÆø
	//------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------
	// °´Å¥³õÊ¼»¯¡£
	m_xInterBtn[ 0].CreateGameBtn(m_pxInterImg, 80, 81, m_rcMain.left+204, m_rcMain.top+0);		// ½»Ò×´°¿Ú¡£
	m_xInterBtn[ 1].CreateGameBtn(m_pxInterImg, 82, 83, m_rcMain.left+228, m_rcMain.top+0);		// Ð¡µØÍ¼.
	m_xInterBtn[ 2].CreateGameBtn(m_pxInterImg, 84, 85, m_rcMain.left+252, m_rcMain.top+0);		// Îä¹¦´°¿Ú.

	m_xInterBtn[ 3].CreateGameBtn(m_pxInterImg, 90, 91, m_rcMain.left+161, m_rcMain.top+65);	// ÍË³öÓÎÏ·.
	m_xInterBtn[ 4].CreateGameBtn(m_pxInterImg, 92, 93, m_rcMain.left+161, m_rcMain.top+101);	// ×¢Ïú.
	m_xInterBtn[ 5].CreateGameBtn(m_pxInterImg, 94, 95, m_rcMain.left+616, m_rcMain.top+66);	// ¶ÓÎé.
	m_xInterBtn[ 6].CreateGameBtn(m_pxInterImg, 96, 97, m_rcMain.left+616, m_rcMain.top+102);	// ÐÐ»á.

	m_xInterBtn[ 7].CreateGameBtn(m_pxInterImg, 52, 53, m_rcMain.left+397, m_rcMain.top+13);	// Ñü´ø UP, DOWN.

	m_xInterBtn[ 8].CreateGameBtn(m_pxInterImg,100, 101, m_rcMain.left+703, m_rcMain.top+34);	// ¼¼ÄÜ´°¿Ú¡£
	m_xInterBtn[ 9].CreateGameBtn(m_pxInterImg,102, 103, m_rcMain.left+718, m_rcMain.top+50);	// ÁÄÌìµ¯´°.
	m_xInterBtn[10].CreateGameBtn(m_pxInterImg,104, 105, m_rcMain.left+718, m_rcMain.top+88);	// ÈÎÎñ.
	m_xInterBtn[11].CreateGameBtn(m_pxInterImg,106, 107, m_rcMain.left+703, m_rcMain.top+103);	// Ñ¡Ïî.
	m_xInterBtn[12].CreateGameBtn(m_pxInterImg,108, 109, m_rcMain.left+664, m_rcMain.top+104);	// °ïÖú.
	m_xInterBtn[13].CreateGameBtn(m_pxInterImg,110, 111, m_rcMain.left+648, m_rcMain.top+88);	// Âí´°¿Ú¡£
	m_xInterBtn[14].CreateGameBtn(m_pxInterImg,112, 113, m_rcMain.left+648, m_rcMain.top+50);	// ±³°ü´°¿Ú.
	m_xInterBtn[15].CreateGameBtn(m_pxInterImg,114, 115, m_rcMain.left+665, m_rcMain.top+34);	// ×´Ì¬´°¿Ú.
	//------------------------------------------------------------------------------------------------------------

	m_xScrlBar.CreateScrlBar(m_pxInterImg, 68, _MAX_CHATLINE, 12, 84, 12);

	m_xMsgBox.Load(&m_xInterImgEx);	// ÓÂÆø
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

			// ²åÈë×Ö·û´®¡£
			for ( nCnt = 0; nCnt < nLineCnt; nCnt++ )
			{
				// ¸ñ×Ó½øÈëÏÂÒ»¸ñµÄÌõ¼þ.
				if ( m_xChat.GetCounter() - m_nFstLine == _MAX_CHATLINE )
				{
					m_nFstLine++;
				}

				stChat.dwFontColor = dwFontColor;
				stChat.dwBackColor = dwFontBackColor;
				strcpy(stChat.pszChat, pszArg[nCnt]);

				m_xChat.AddNode(stChat);
			}

			// ¼ì²é×î´ó±£´æÐÐÊý£¬³¬¹ýÔòÉ¾³ý¡£
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

	º¯ÊýÃû : CInterface::ShowWndList()

	±àÐ´Õß :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : ÏÔÊ¾´¦ÓÚ Active ×´Ì¬µÄ´°¿ÚÁÐ±í.
	Êä³ö   : VOID

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

*******************************************************************************************************************/
VOID CInterface::ShowWndList()
{
	if ( m_xWndOrderList.GetCounter() != 0 )
	{
		m_xWndOrderList.MoveCurrentToTop();

		INT* pnCurrID;

		for ( INT nCnt = 0; nCnt < m_xWndOrderList.GetCounter(); nCnt++ )
		{
			pnCurrID = m_xWndOrderList.GetCurrentData();

			switch ( *pnCurrID )
			{
			case _WND_ID_INVENTORY:
				m_xInventoryWnd.ShowInventoryWnd();
				break;
			case _WND_ID_STATUS:
				m_xStatusWnd.ShowStatusWnd();
				break;
			case _WND_ID_STORE:
				m_xStoreWnd.ShowStoreWnd();
				break;
			case _WND_ID_EXCHANGE:
				m_xExchangeWnd.ShowExchangeWnd();
				break;
			case _WND_ID_GUILDMASTER:
				m_xGuildMasterWnd.ShowGuildMasterWnd();
				break;
			case _WND_ID_GUILD:
				m_xGuildWnd.ShowGuildWnd();
				break;
			case _WND_ID_GROUP:
				m_xGroupWnd.ShowGroupWnd();
				break;
			case _WND_ID_GROUPPOP:
				m_xGroupPopWnd.ShowGroupPopWnd();
				break;
			case _WND_ID_CHATPOP:
				m_xChatPopWnd.ShowChatPopWnd();
				break;
			case _WND_ID_NPCCHAT:					// ÓÂÆø
				m_xNPCWnd.ShowNPCChatWnd();
				break;

			case _WND_ID_QUEST:
				m_xQuestWnd.ShowQuestWnd();
				break;
			case _WND_ID_OPTION:
				m_xOptionWnd.ShowOptionWnd();
				break;
			case _WND_ID_HORSE:
				m_xHorseWnd.ShowHorseWnd();
				break;
			}

			m_xWndOrderList.MoveNextNode();
		}
	}
}



/******************************************************************************************************************

	º¯ÊýÃû : CInterface::ShowChatList()

	×÷Õß :
	ÈÕÆÚ :

	Ä¿µÄ   : ÏÔÊ¾ÁÄÌìÁÐ±í.
	Êä³ö   : VOID

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

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

	// ´°¿Ú.
	ShowWndList();

	// Êó±êÎïÆ·.
	if ( m_stCommonItem.bSetted && !m_stCommonItem.bIsHideItem )
	{
		nX = ptMousePos.x - _INVENTORY_CELL_WIDTH /2;
		nY = ptMousePos.y - _INVENTORY_CELL_HEIGHT/2;
		m_stCommonItem.xItem.DrawItem(nX, nY);
	}

	// Ñü´øÎïÆ·¡£
	ShowBeltItem();

	// Ö÷½çÃæ.
	if ( m_pxInterImg->NewSetIndex(_WNDIMGIDX_MAIN) )
	{
		g_xMainWnd.DrawWithImageForCompClipRgn(m_rcMain.left, m_rcMain.top,	m_rcMain.right-m_rcMain.left, m_rcMain.bottom-m_rcMain.top,
											   (WORD*)m_pxInterImg->m_pbCurrImage, _CLIP_WIDTH, 600);
	}

	// ÁÄÌìÁÐ±í.
	ShowChatList();

	// ×´Ì¬Öµ.
	ShowGameStatus();

	// °´Å¥.
	for ( nCnt = 0; nCnt < _MAX_INTER_BTN; nCnt++)
	{
		m_xInterBtn[nCnt].ShowGameBtn();
	}

	// ¹ö¶¯Ìõ.
	m_xScrlBar.ShowScrlBar(584, 404, m_nFstLine, m_xChat.GetCounter());

	// ¿Í»§¶ËÏûÏ¢(×óÉÏ½Ç).
	m_xClientSysMsg.ShowSysMsg(nLoopTime, 30, 30);

	// ÓÂÆø
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
			// Ñü´øÎïÆ·»æÖÆ¡£
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

	// Ò»ÌìµÄ×´Ì¬Öµ(Ò¹Íí, °×Ìì, °øÍí, ÀèÃ÷)
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

	// »æÖÆ HP, MP.
	// ±ÈÀý¼ì²é¡£
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

	// »æÖÆ EXP, WET¡£
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
	// Á·¼¶6.
	sprintf(pszBuff, "%d", g_xGameProc.m_xMyHero.m_stAbility.bLevel);
	SetRect(&rcStats, 693, 528, 709, 544);
	g_xMainWnd.PutsHan(NULL, rcStats, dwFntClr, RGB(0, 0, 0), pszBuff, g_xMainWnd.CreateGameFont("Arial", 10, 0, FW_BOLD));

	dwFntClr = RGB(255, 255, 200);
	// »æÖÆµØÍ¼Ãû³Æ¼°×ø±ê.
	sprintf(pszBuff, "%s : [%d,%d]", g_xGameProc.m_szMapName, g_xGameProc.m_xMyHero.m_wPosX, g_xGameProc.m_xMyHero.m_wPosY);
	SetRect(&rcStats, 219, 584, 379, 599);
	g_xMainWnd.PutsHan(NULL, rcStats, dwFntClr, RGB(0, 0, 0), pszBuff);

	dwFntClr = RGB(255, 200, 50);
	// »æÖÆ¸÷Ö°ÒµÍ¼±ê¼°ÊôÐÔÖµ.
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
	// AC ±ê¼Ç.
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
// ´°¿Ú¹ÜÀí²¿·Ö/////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************************************************************************

	º¯ÊýÃû : CInterface::GetWindowInMousePos()

	×÷Õß :
	ÈÕÆÚ :

	Ä¿µÄ   : ·µ»ØÊó±êÖ¸ÕëËùÔÚÇøÓòÄÚµÄ´°¿ÚÁÐ±íÖÐµÚÒ»¸ö´°¿ÚµÄ ID.
	ÊäÈë   : POINT ptMouse
	Êä³ö   :

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

*******************************************************************************************************************/
INT	CInterface::GetWindowInMousePos(POINT ptMouse)
{
	if ( m_xWndOrderList.GetCounter() != 0 )
	{
		RECT rcWnd;
		INT* pnCurrID;
		m_xWndOrderList.MoveCurrentToTail();

		for ( INT nCnt = m_xWndOrderList.GetCounter()-1; nCnt >= 0 ; nCnt-- )
		{
			pnCurrID = m_xWndOrderList.GetCurrentData();

			switch ( *pnCurrID )
			{
			case _WND_ID_INVENTORY:
				rcWnd = m_xInventoryWnd.GetGameWndRect();
				break;
			case _WND_ID_STATUS:
				rcWnd = m_xStatusWnd.GetGameWndRect();
				break;
			case _WND_ID_STORE:
				rcWnd = m_xStoreWnd.GetGameWndRect();
				break;
			case _WND_ID_EXCHANGE:
				rcWnd = m_xExchangeWnd.GetGameWndRect();
				break;
			case _WND_ID_GUILDMASTER:
				rcWnd = m_xGuildMasterWnd.GetGameWndRect();
				break;
			case _WND_ID_GUILD:
				rcWnd = m_xGuildWnd.GetGameWndRect();
				break;
			case _WND_ID_GROUP:
				rcWnd = m_xGroupWnd.GetGameWndRect();
				break;
			case _WND_ID_GROUPPOP:
				rcWnd = m_xGroupPopWnd.GetGameWndRect();
				break;
			case _WND_ID_CHATPOP:
				rcWnd = m_xChatPopWnd.GetGameWndRect();
				break;
			case _WND_ID_NPCCHAT:	// ÓÂÆø
				rcWnd = m_xNPCWnd.GetGameWndRect();
				break;

			case _WND_ID_QUEST:
				rcWnd = m_xQuestWnd.GetGameWndRect();
				break;
			case _WND_ID_OPTION:
				rcWnd = m_xOptionWnd.GetGameWndRect();
				break;
			case _WND_ID_HORSE:
				rcWnd = m_xHorseWnd.GetGameWndRect();
				break;

			}

			if ( PtInRect(&rcWnd, ptMouse) )
			{
				return *pnCurrID;
			}

			m_xWndOrderList.MovePreviousNode();
		}
	}
	return -1;
}


/******************************************************************************************************************

	º¯ÊýÃû : CInterface::AddWindowToList()

	×÷Õß :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : ÔÚ´°¿ÚÁÐ±íµÄ×îºóÌí¼Óµ±Ç°ÊäÈëµÄ ID Îª nID µÄ´°¿Ú¡£
	ÊäÈë   : INT nID
	Êä³ö   : VOID

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

*******************************************************************************************************************/
VOID CInterface::AddWindowToList(INT nID)
{
	m_xWndOrderList.AddNode(nID);
}


/******************************************************************************************************************

	º¯ÊýÃû : CInterface::DeleteWindowToList()

	×÷Õß :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : ´ÓÁÐ±íÉÏÉ¾³ý¾ßÓÐ nID ±êÊ¶µÄ´°¿Ú.
	ÊäÈë   : INT nID
	Êä³ö   : VOID

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

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

	º¯ÊýÃû : CInterface::WindowActivate()

	×÷Õß :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : ½« nID ´°¿ÚÌí¼Óµ½´°¿ÚÁÐ±íÖÐ»ò´ÓÁÐ±íÖÐÉ¾³ý,Ìí¼ÓÊ±½«´°¿Ú×´Ì¬ÉèÖÃÎª×î¶¥²ã×´Ì¬¡£
			 ÊÇ´°¿Ú½ÓÊÕ¼üÅÌÊäÈë²¢ÏÔÊ¾µ½ÆÁÄ»Ê±µ÷ÓÃµÄº¯Êý¡£
	ÊäÈë   : INT nID
	Êä³ö   : VOID

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

*******************************************************************************************************************/
BOOL CInterface::WindowActivate(INT nID)
{
	// ÎªÁËÈÃÐÂ´°¿Ú»ñµÃ½¹µã£¬½«ËùÓÐ´°¿ÚµÄ½¹µãÉèÎª FALSE¡£
	BOOL bRet = FALSE;

	AllWindowsFocusOff();

	switch ( nID )
	{
	case _WND_ID_INVENTORY:
		{
			if ( m_xInventoryWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_INVENTORY);
				m_xInventoryWnd.SetGameWndActive(FALSE);
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_INVENTORY);
				m_xInventoryWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	case _WND_ID_STATUS:
		{
			if ( m_xStatusWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_STATUS);
				m_xStatusWnd.SetGameWndActive(FALSE);
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_STATUS);
				m_xStatusWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	case _WND_ID_STORE:
		{
			if ( m_xStoreWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_STORE);
				m_xStoreWnd.SetGameWndActive(FALSE);
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_STORE);
				m_xStoreWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	case _WND_ID_EXCHANGE:
		{
			if ( m_xExchangeWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_EXCHANGE);
				m_xExchangeWnd.SetGameWndActive(FALSE);
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_EXCHANGE);
				m_xExchangeWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	case _WND_ID_QUEST:
		{
			if ( m_xQuestWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_QUEST);
				m_xQuestWnd.SetGameWndActive(FALSE);
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_QUEST);
				m_xQuestWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	case _WND_ID_OPTION:
		{
			if ( m_xOptionWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_OPTION);
				m_xOptionWnd.SetGameWndActive(FALSE);
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_OPTION);
				m_xOptionWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	case _WND_ID_HORSE:
		{
			if ( m_xHorseWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_HORSE);
				m_xHorseWnd.SetGameWndActive(FALSE);
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_HORSE);
				m_xHorseWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;

	case _WND_ID_GUILDMASTER:
		{
			if ( m_xGuildMasterWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_GUILDMASTER);
				m_xGuildMasterWnd.SetGameWndActive(FALSE);
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_GUILDMASTER);
				m_xGuildMasterWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	case _WND_ID_GUILD:
		{
			if ( m_xGuildWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_GUILD);
				m_xGuildWnd.SetGameWndActive(FALSE);
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_GUILD);
				m_xGuildWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	case _WND_ID_GROUP:
		{
			if ( m_xGroupWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_GROUP);
				m_xGroupWnd.SetGameWndActive(FALSE);
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_GROUP);
				m_xGroupWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	case _WND_ID_GROUPPOP:
		{
			if ( m_xGroupPopWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_GROUPPOP);
				m_xGroupPopWnd.SetGameWndActive(FALSE);
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_GROUPPOP);
				m_xGroupPopWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	case _WND_ID_CHATPOP:
		{
			if ( m_xChatPopWnd.GetGameWndActive() )
			{
				MoveWindow(g_xChatEditBox.GetSafehWnd(), g_xMainWnd.m_rcWindow.left+_INPUT_EDITWND_XPOS,
						   g_xMainWnd.m_rcWindow.top+_INPUT_EDITWND_YPOS, _INPUT_EDITWND_WIDTH, _INPUT_EDITWND_HEIGHT, TRUE);

				DeleteWindowToList(_WND_ID_CHATPOP);
				m_xChatPopWnd.SetGameWndActive(FALSE);
				bRet = FALSE;
			}
			else
			{
				MoveWindow(g_xChatEditBox.GetSafehWnd(), g_xMainWnd.m_rcWindow.left + m_xChatPopWnd.m_rcWnd.left + m_xChatPopWnd.m_rcEditBoxFrame.left,
						   g_xMainWnd.m_rcWindow.top + m_xChatPopWnd.m_rcWnd.top + m_xChatPopWnd.m_rcEditBoxFrame.top,
						   m_xChatPopWnd.m_rcEditBoxFrame.right - m_xChatPopWnd.m_rcEditBoxFrame.left,
						   m_xChatPopWnd.m_rcEditBoxFrame.bottom - m_xChatPopWnd.m_rcEditBoxFrame.top, TRUE);

				SetFocus(g_xChatEditBox.GetSafehWnd());
				AddWindowToList(_WND_ID_CHATPOP);
				m_xChatPopWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	case _WND_ID_NPCCHAT:		// ÓÂÆø
		{
			if ( m_xNPCWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_NPCCHAT);
//				m_xNPCWnd.CloseWnd();
				m_xNPCWnd.SetGameWndActive(FALSE);
				bRet = FALSE;
			}
			else
			{
				// ÁÙÊ±
				AddWindowToList(_WND_ID_NPCCHAT);
//				m_xNPCWnd.SetNPCStr("ÑöÍûÌì¿Õ Ö±ÖÁÎÞÒ»Ë¿ÐßÀ¢ ÔÚÂÓ¹ýÒ¶¼äµÄ·çÖÐ ÎÒÒ²Ôø¸Ðµ½Í´¿à <ÐÇÐÇ/@STAR> ÒÔ¸è³ªµÄÐÄ È¥<°®/@LOVE>Ò»ÇÐÕýÔÚËÀÈ¥µÄÊÂÎï È»ºó»¹Òª×ßÍê±»´ÍÓèµÄÄÇÌõÂ·. ½ñÒ¹ÐÇÐÇÒÀ¾É±»·çÂÓ¹ý. ¶«º£Ö®Ë®Óë°×Í·É½¿Ý½ß´ù¾¡ Ô¸ÉÏÌì±£ÓÓ ÎÒ¹úÍòËê ÎÞÇî»¨ÈýÇ§Àï »ªÀö½­É½ ´óº«Ö®ÈË ÒÔ´óº«ÓÀÊÀ³¤´æ. Î÷ÃÅ Äã¿ÉÖªµÀ ¹Î×Ï²Ë°ü·¹²à±ßµÄÉùÒô Î÷ÃÅ Äã¿ÉÖªµÀ ¹Î¿ªµÄ²à±ß±ÀÁÑµÄÉùÒô ÄÇÎÞ±ÈÃÀÃîµÄÉùÒô Äãµ±Õæ²»ÖªµÀÂð\n <´´½¨ÐÐ»áÊÔÊÔ¿´?/@@buildguildnow>  \n <ÍË³ö/@end>",0);
				m_xNPCWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	}

	return bRet;
}


/******************************************************************************************************************

	º¯ÊýÃû : CInterface::MoveTopWindow()

	×÷Õß :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : ½«×îÉÏ²ãµÄ´°¿ÚÒÆ¶¯µ½Êó±ê×ø±ê¡£
	ÊäÈë   : POINT ptMouse
	Êä³ö   : VOID

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

*******************************************************************************************************************/
VOID CInterface::MoveTopWindow(POINT ptMouse)
{
	if ( m_xWndOrderList.GetCounter() != 0 )
	{
		INT* pnCurrID;
		m_xWndOrderList.MoveCurrentToTail();
		pnCurrID = m_xWndOrderList.GetCurrentData();
		if ( m_bMoveFocusedWnd )
		{
			switch ( *pnCurrID )
			{
			case _WND_ID_INVENTORY:
				m_xInventoryWnd.MoveGameWnd(ptMouse);
				break;
			case _WND_ID_STATUS:
				m_xStatusWnd.MoveGameWnd(ptMouse);
				break;
			case _WND_ID_STORE:
				m_xStoreWnd.MoveGameWnd(ptMouse);
				break;
			case _WND_ID_EXCHANGE:
				m_xExchangeWnd.MoveGameWnd(ptMouse);
				break;
			case _WND_ID_GUILDMASTER:
				m_xGuildMasterWnd.MoveGameWnd(ptMouse);
				break;
			case _WND_ID_GUILD:
				m_xGuildWnd.MoveGameWnd(ptMouse);
				break;
			case _WND_ID_GROUP:
				m_xGroupWnd.MoveGameWnd(ptMouse);
				break;
			case _WND_ID_GROUPPOP:
				m_xGroupPopWnd.MoveGameWnd(ptMouse);
				break;
			case _WND_ID_CHATPOP:
				m_xChatPopWnd.MoveGameWnd(ptMouse);
				ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
				break;
			case _WND_ID_NPCCHAT:		// ÓÂÆø
				m_xNPCWnd.MoveGameWnd(ptMouse);
				break;

			case _WND_ID_QUEST:
				m_xQuestWnd.MoveGameWnd(ptMouse);
				break;
			case _WND_ID_OPTION:
				m_xOptionWnd.MoveGameWnd(ptMouse);
				break;
			case _WND_ID_HORSE:
				m_xHorseWnd.MoveGameWnd(ptMouse);
				break;

			}
		}
	}
}



/******************************************************************************************************************

	º¯ÊýÃû : CInterface::WindowFocusChanged()

	×÷Õß :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : ÔÚ´°¿ÚÇøÓòµã»÷Êó±êÊ±£¬½«µ±Ç°µÄ¶¥²ã´°¿ÚÇÐ»»ÎªÊó±êµã»÷µÄ´°¿Ú£¬²¢°Ñ½¹µãÉèÖÃµ½¸Ã´°¿Ú.
	ÊäÈë   : INT nID
	Êä³ö   : VOID

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

*******************************************************************************************************************/
VOID CInterface::WindowFocusChangedAndSetReadyMove(INT nID, POINT ptMouse)
{
	if ( m_xWndOrderList.GetCounter() != 0 )
	{
		AllWindowsFocusOff();

		DeleteWindowToList(nID);
		AddWindowToList(nID);
		// Èç¹û´¦ÓÚÊó±êµã»÷×´Ì¬,Ôò´¦ÓÚ¿ÉÒÔÒÆ¶¯µÄ×¼±¸×´Ì¬.
		m_bMoveFocusedWnd = TRUE;

		switch ( nID )
		{
		case _WND_ID_INVENTORY:
			m_xInventoryWnd.SetGameWndFocused(TRUE);
			m_xInventoryWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_STATUS:
			m_xStatusWnd.SetGameWndFocused(TRUE);
			m_xStatusWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_STORE:
			m_xStoreWnd.SetGameWndFocused(TRUE);
			m_xStoreWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_EXCHANGE:
			m_xExchangeWnd.SetGameWndFocused(TRUE);
			m_xExchangeWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_GUILDMASTER:
			m_xGuildMasterWnd.SetGameWndFocused(TRUE);
			m_xGuildMasterWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_GUILD:
			m_xGuildWnd.SetGameWndFocused(TRUE);
			m_xGuildWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_GROUP:
			m_xGroupWnd.SetGameWndFocused(TRUE);
			m_xGroupWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_GROUPPOP:
			m_xGroupPopWnd.SetGameWndFocused(TRUE);
			m_xGroupPopWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_CHATPOP:
			m_xChatPopWnd.SetGameWndFocused(TRUE);
			m_xChatPopWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_NPCCHAT:		// ÓÂÆø
			m_xNPCWnd.SetGameWndFocused(TRUE);
			m_xNPCWnd.SetReadyMove(ptMouse);
			break;

		case _WND_ID_QUEST:
			m_xQuestWnd.SetGameWndFocused(TRUE);
			m_xQuestWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_OPTION:
			m_xOptionWnd.SetGameWndFocused(TRUE);
			m_xOptionWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_HORSE:
			m_xHorseWnd.SetGameWndFocused(TRUE);
			m_xHorseWnd.SetReadyMove(ptMouse);
			break;

		}
	}
}


VOID CInterface::AllWindowsFocusOff()
{
	if ( m_xWndOrderList.GetCounter() != 0 )
	{
		m_xWndOrderList.MoveCurrentToTop();

		INT* pnCurrID;
		for ( INT nCnt = 0; nCnt < m_xWndOrderList.GetCounter(); nCnt++ )
		{
			pnCurrID = m_xWndOrderList.GetCurrentData();

			switch ( *pnCurrID )
			{
			case _WND_ID_INVENTORY:
				m_xInventoryWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_STATUS:
				m_xStatusWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_STORE:
				m_xStoreWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_EXCHANGE:
				m_xExchangeWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_GUILDMASTER:
				m_xGuildMasterWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_GUILD:
				m_xGuildWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_GROUP:
				m_xGroupWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_GROUPPOP:
				m_xGroupPopWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_CHATPOP:
				m_xChatPopWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_NPCCHAT:		// ÓÂÆø
				m_xNPCWnd.SetGameWndFocused(FALSE);
				break;

			case _WND_ID_QUEST:
				m_xQuestWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_OPTION:
				m_xOptionWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_HORSE:
				m_xHorseWnd.SetGameWndFocused(FALSE);
				break;

			}
			m_xWndOrderList.MoveNextNode();
		}
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
// ÏûÏ¢ÊäÈëÏà¹Ø²¿·Ö/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************************************************************************

	º¯ÊýÃû : CInterface::OnLButtonDown()

	×÷Õß :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : ¶¨Òå Mouse LButtonDown ·¢ÉúÊ±½Ó¿ÚÖÐÒª×öµÄ¹¤×÷.
	ÊäÈë   : POINT ptMouse
	Êä³ö   : BOOL

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

*******************************************************************************************************************/
BOOL CInterface::OnLButtonDown(POINT ptMouse)
{
	if(m_xMsgBox.IsActive())	// ÓÂÆø
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

		INT nInterfaceWndInRect;
		nInterfaceWndInRect = GetWindowInMousePos(ptMouse);

		if ( nInterfaceWndInRect != -1 )
		{
			switch ( nInterfaceWndInRect )
			{
			case _WND_ID_INVENTORY:
				if ( !m_xInventoryWnd.OnLButtonDown(&m_stCommonItem, ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;
			case _WND_ID_STATUS:
				if ( !m_xStatusWnd.OnLButtonDown(&m_stCommonItem, ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;
			case _WND_ID_STORE:
				if ( !m_xStoreWnd.OnLButtonDown(ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;
			case _WND_ID_EXCHANGE:
				if ( !m_xExchangeWnd.OnLButtonDown(ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;
			case _WND_ID_GUILDMASTER:
				if ( !m_xGuildMasterWnd.OnLButtonDown(ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;
			case _WND_ID_GUILD:
				if ( !m_xGuildWnd.OnLButtonDown(ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;
			case _WND_ID_GROUP:
				if ( !m_xGroupWnd.OnLButtonDown(ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;
			case _WND_ID_GROUPPOP:
				if ( !m_xGroupPopWnd.OnLButtonDown(ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;
			case _WND_ID_CHATPOP:
				if ( !m_xChatPopWnd.OnLButtonDown(ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;
			case _WND_ID_NPCCHAT:	// ÓÂÆø
				if ( !m_xNPCWnd.OnLButtonDown(ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;


			case _WND_ID_QUEST:
				if ( !m_xQuestWnd.OnLButtonDown(ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;
			case _WND_ID_OPTION:
				if ( !m_xOptionWnd.OnLButtonDown(ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;
			case _WND_ID_HORSE:
				if ( !m_xHorseWnd.OnLButtonDown(ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;

			}
			return TRUE;
		}
		else
			AllWindowsFocusOff();
	}
	return FALSE;
}



/******************************************************************************************************************

	º¯ÊýÃû : CInterface::OnLButtonUp()

	×÷Õß   :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : ¶¨ÒåÊó±ê LButtonUp ·¢ÉúÊ±½Ó¿ÚÒª×öµÄ´¦Àí¡£
	ÊäÈë   : POINT ptMouse
	Êä³ö   : BOOL

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

*******************************************************************************************************************/
BOOL CInterface::OnLButtonUp(POINT ptMouse)
{
	m_bMoveFocusedWnd = FALSE;

	if ( m_xMsgBox.IsActive() )	// ÓÂÆø
	{
		HRESULT hResult;
		hResult = m_xMsgBox.OnButtonUp(ptMouse);
		if ( hResult != 0 )
		{
			switch ( m_nIdxMsgBox )
			{
			case 12:
				{
					switch ( hResult )	// Ã»ÓÐ±à¼­¿ò.
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

		INT nInterfaceWndInRect;
		nInterfaceWndInRect = GetWindowInMousePos(ptMouse);

		// ¸÷´°¿ÚµÄ OnLButtonUp ·µ»ØÖµÎª TRUE Ê±, ´°¿Ú½«±» Close¡£
		if ( nInterfaceWndInRect != -1 )
		{
			switch ( nInterfaceWndInRect )
			{
			case _WND_ID_INVENTORY:		if ( m_xInventoryWnd.OnLButtonUp(&m_stCommonItem, ptMouse) )	WindowActivate(_WND_ID_INVENTORY);		break;
			case _WND_ID_STATUS:		if ( m_xStatusWnd.OnLButtonUp(&m_stCommonItem, ptMouse) )		WindowActivate(_WND_ID_STATUS);			break;
			case _WND_ID_STORE:			if ( m_xStoreWnd.OnLButtonUp(ptMouse) )							WindowActivate(_WND_ID_STORE);			break;
			case _WND_ID_EXCHANGE:		if ( m_xExchangeWnd.OnLButtonUp(ptMouse) )						WindowActivate(_WND_ID_EXCHANGE);		break;
			case _WND_ID_GUILDMASTER:
			{
				if ( m_xGuildMasterWnd.OnLButtonUp(ptMouse) )
					WindowActivate(_WND_ID_EXCHANGE);
				break;
			}
			case _WND_ID_GUILD:			if ( m_xGuildWnd.OnLButtonUp(ptMouse) )							WindowActivate(_WND_ID_GUILD);			break;
			case _WND_ID_GROUP:			if ( m_xGroupWnd.OnLButtonUp(ptMouse) )							WindowActivate(_WND_ID_GROUP);			break;
			case _WND_ID_GROUPPOP:		if ( m_xGroupPopWnd.OnLButtonUp(ptMouse) )						WindowActivate(_WND_ID_GROUPPOP);		break;
			case _WND_ID_CHATPOP:		if ( m_xChatPopWnd.OnLButtonUp(ptMouse) )						WindowActivate(_WND_ID_CHATPOP);		break;

			case _WND_ID_QUEST:			if ( m_xQuestWnd.OnLButtonUp(ptMouse) )							WindowActivate(_WND_ID_QUEST);			break;
			case _WND_ID_OPTION:		if ( m_xOptionWnd.OnLButtonUp(ptMouse) )						WindowActivate(_WND_ID_OPTION);			break;
			case _WND_ID_HORSE:			if ( m_xHorseWnd.OnLButtonUp(ptMouse) )							WindowActivate(_WND_ID_HORSE);			break;

			case _WND_ID_NPCCHAT:		if ( m_xNPCWnd.OnLButtonUp(ptMouse) )							WindowActivate(_WND_ID_NPCCHAT);		break; // ÓÂÆø
			}
			return TRUE;
		}
	}
	return FALSE;
}


/******************************************************************************************************************

	º¯ÊýÃû : CInterface::OnMouseMove()

	×÷Õß :
	±àÐ´ÈÕÆÚ :

	Ä¿µÄ   : µ± Mouse Move ·¢ÉúÊ±, ¶¨Òå½çÃæÖÐÒª×öµÄÊÂ.
	ÊäÈë   : POINT ptMouse
	Êä³ö   : BOOL

	[ÈÕÆÚ][ÐÞ¸ÄÕß] : ÐÞ¸ÄÄÚÈÝ

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

	INT nInterfaceWndInRect;
	nInterfaceWndInRect = GetWindowInMousePos(ptMouse);

	if ( nInterfaceWndInRect != -1 )
	{
		switch ( nInterfaceWndInRect )
		{
		case _WND_ID_INVENTORY:
			m_xInventoryWnd.OnMouseMove(ptMouse);
			break;
		case _WND_ID_STATUS:
			m_xStatusWnd.OnMouseMove(ptMouse);
			break;
		case _WND_ID_STORE:
			m_xStoreWnd.OnMouseMove(ptMouse);
			break;
		case _WND_ID_EXCHANGE:
			m_xExchangeWnd.OnMouseMove(ptMouse);
			break;
		case _WND_ID_GUILDMASTER:
			m_xGuildMasterWnd.OnMouseMove(ptMouse);
			break;
		case _WND_ID_GUILD:
			m_xGuildWnd.OnMouseMove(ptMouse);
			break;
		case _WND_ID_GROUP:
			m_xGroupWnd.OnMouseMove(ptMouse);
			break;
		case _WND_ID_GROUPPOP:
			m_xGroupPopWnd.OnMouseMove(ptMouse);
			break;
		case _WND_ID_CHATPOP:
			m_xChatPopWnd.OnMouseMove(ptMouse);
			break;

		case _WND_ID_QUEST:
			m_xQuestWnd.OnMouseMove(ptMouse);
			break;
		case _WND_ID_OPTION:
			m_xOptionWnd.OnMouseMove(ptMouse);
			break;
		case _WND_ID_HORSE:
			m_xHorseWnd.OnMouseMove(ptMouse);
			break;

		case _WND_ID_NPCCHAT:		// ÓÂÆø
			m_xNPCWnd.OnMouseMove(ptMouse);
			break;
		}
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
	else if (IsTopWindow(m_xNPCWnd.m_nID))		// ÓÂÆø
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
	else if (IsTopWindow(m_xNPCWnd.m_nID))		// ÓÂÆø
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
	// F10 Key ±»°´ÏÂÊ±¡£
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

HRESULT CLoginAvi::OnButtonDown(POINT ptMouse)
{
	return 0;
}

HRESULT CLoginAvi::OnButtonUp(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

HRESULT CLoginAvi::OnButtonUp(POINT ptMouse)
{
	return 0;
}

LRESULT CLoginAvi::OnMouseMove(WPARAM wParam, LPARAM lParam)
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

HRESULT CLogin::OnButtonUp(POINT ptMouse)
{

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

HRESULT	CNewAccount::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
/*	if (wParam == VK_RETURN || wParam == VK_TAB)
	{	SetFocusBefore();
		switch(m_nUserState)
		{	case  LGM_INPUT_ID:
			{	m_nUserState = LGM_INPUT_PASSWORD;
				break;
			}
			case LGM_INPUT_PASSWORD:
			{	if( ( lstrlen( m_xInputState[LGM_INPUT_ID-1].szData) >= LIMIT_USERID )
								&& lstrlen( m_xInputState[LGM_INPUT_PASSWORD-1].szData ) )
				{
					g_xClientSocket.OnLogin(m_xInputState[LGM_INPUT_ID-1].szData,m_xInputState[LGM_INPUT_PASSWORD-1].szData);
					return 0L;
				}
				m_nUserState = LGM_INPUT_ID;
				break;
			}
		}
		SetFocusAfter();
	}*/
	return 0;
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
					//m_nProgress = PRG_NEW_ACCOUNT;	- > ÇÐ»»µ½ New Account µÄº¯Êý
					SetFocusBefore();
					SetWindowText(g_xChatEditBox.GetSafehWnd(),NULL);
					m_nUserState = LGM_INPUT_USER_ID;
					SetFocusAfter();
					break;
				}
			case IMG_IDX_BTN_CHGPASS:
				{	// ÁÙÊ±×÷ÎªÍË³ö°´Å¥Ê¹ÓÃ
					//	½áÊø Avi Ïà¹Øº¯ÊýµÄ´úÂë
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

HRESULT CNewAccount::OnButtonUp(POINT ptMouse)
{

	return 0;
}

LRESULT	CNewAccount::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
/*	INT	i;

	if(!m_fIsButtonDown)
	{
		for( i = BTN_NEW_ID ; i <= BTN_OK_ID ; i ++)
		{
			if (m_xButtons[i].CheckMouseOn(LOWORD(lParam), HIWORD(lParam)))
				m_xButtons[i].m_nState = BUTTON_STATE_ON;
			else
				m_xButtons[i].m_nState = BUTTON_STATE_UP;
		}
	}
*/
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

#define SERVER_BTN_TEXTCOLOR_NORMAL	RGB(10, 10, 10)			// °µÉ«
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

HRESULT	CSelectSrv::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	return 0;
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


HRESULT CSelectSrv::OnButtonUp(POINT ptMouse)
{
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

			// ÁÙÊ±
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

HRESULT	CBackGround::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

HRESULT CBackGround::OnButtonDown(WPARAM wParam, LPARAM lParam)
{

	return 0;
}

HRESULT CBackGround::OnButtonDown(POINT ptMouse)
{
	return 0;
}

HRESULT	CBackGround::OnButtonUp(WPARAM wParam, LPARAM lParam)
{

	return 0;
}

HRESULT CBackGround::OnButtonUp(POINT ptMouse)
{

	return 0;
}

LRESULT	CBackGround::OnMouseMove(WPARAM wParam, LPARAM lParam)
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
									{ IMG_IDX_WAR_BTN_UP,	 	CREATE_CHAR_WAR_BTN_POS_X,		CREATE_CHAR_BTN_POS_Y,			WIDTH_JOB_BTN,		HEIGHT_JOB_BTN},	// Õ½Ê¿ Button
									{ IMG_IDX_SOR_BTN_UP,		CREATE_CHAR_SOR_BTN_POS_X,		CREATE_CHAR_BTN_POS_Y,			WIDTH_JOB_BTN,		HEIGHT_JOB_BTN},	// ÊõÊ¿ Button
									{ IMG_IDX_TAO_BTN_UP, 		CREATE_CHAR_TAO_BTN_POS_X,		CREATE_CHAR_BTN_POS_Y,			WIDTH_JOB_BTN,		HEIGHT_JOB_BTN}		// µÀÊ¿ Button
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

HRESULT CCreateChr::OnButtonUp(POINT ptMouse)
{

	return 0;
}

LRESULT	CCreateChr::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
/*	INT	i;

	if(!m_fIsButtonDown)
	{
		for( i = BTN_NEW_ID ; i <= BTN_OK_ID ; i ++)
		{
			if (m_xButtons[i].CheckMouseOn(LOWORD(lParam), HIWORD(lParam)))
				m_xButtons[i].m_nState = BUTTON_STATE_ON;
			else
				m_xButtons[i].m_nState = BUTTON_STATE_UP;
		}
	}
*/
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

		// ÁÙÊ±~!
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
HRESULT	CSelectChr::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	return 0;
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

HRESULT CSelectChr::OnButtonUp(POINT ptMouse)
{

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

		for (i = ID_SEL_CHR_NEW_BTN; i <= ID_SEL_CHR_START_BTN ; i++)	// °´Å¥(ÁÙÊ±)
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
				m_Image.NewSetIndex(IMG_IDX_LOGIN_BACK);					// ÁÙÊ±
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

	if(pPatch->m_bEndPatch == TRUE)//GetFtpFiles()==0l)	// Èç¹û Patch ÒÑ½áÊø
	{

		if(pPatch->m_bPatched == TRUE)	// ÈôÒÑ Patch ÔòÍË³ö
		{
			delete pPatch;
			char PatchName[1024];
			ZeroMemory(PatchName,1024);
			GetCurrentDirectory(1024,PatchName);
			strcat(PatchName,MIR2_PATCH_FILE_NAME);
			//m_Progress = PRG_QUIT;
      m_Progress = PRG_INTRO;
			ShellExecute(NULL,"open",PatchName,NULL,NULL,SW_SHOWNORMAL);
			// ½áÊøÔËÐÐ²¢Ö´ÐÐÍâ²¿ Patch ³ÌÐò¡£
		}
		else
		{
			// ÈôÃ»ÓÐÒÑ´ò²¹¶¡µÄÄÚÈÝ
			delete pPatch;

			g_xClientSocket.ConnectToServer(g_xMainWnd.GetSafehWnd(), m_szServerIP, m_nServerPort);
			m_Progress = PRG_INTRO;


			// ÇÐ»»µ½ Full Screen
			g_xMainWnd.ResetDXG(640, 480, _SCREEN_BPP, _DXG_SCREENMODE_WINDOW, _DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D);
//			g_xMainWnd.ResetDXG(640, 480, _SCREEN_BPP, _DXG_SCREENMODE_FULLSCREEN, _DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D);
			SetRect(&tRect, SCR_LEFT, SCR_TOP, SCR_RIGHT, SCR_BOTTOM);
		}
	}
	else
	{
		// ÈôÐè¼ÌÐø Patch
		pPatch->GetFtpFiles();
	}

	if(m_xMsgBox.IsActive())
	{
		HINSTANCE hLib;
		hLib = LoadLibrary(MESSAGE_DLL_1);
		SetErrorMessage(300,m_szErrorMsg);		// ²éÕÒ¶ÔÓ¦ Error ÏûÏ¢
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
			SetNextProc();												// ÒÆ¶¯µ½½ÇÉ«Ñ¡Ôñ·þÎñÆ÷ (ÁÙÊ±)
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
	// ÁÙÊ±~!
	m_Image.NewSetIndex(IMG_IDX_WAR_MALE_CREATE + m_SelectSex + m_SelectJob * 2);

	g_xMainWnd.DrawWithImageForComp(nPos[m_SelectJob].x,nPos[m_SelectJob].y
			,m_Image.m_lpstNewCurrWilImageInfo->shWidth,m_Image.m_lpstNewCurrWilImageInfo->shHeight
			,(WORD*)(m_Image.m_pbCurrImage));
}

#undef POS_NAME_X
#undef POS_NAME_Y

#else

// CUI dialogs and panels.
#include "CuiLegacy/hud/actor_status.hpp"
#include "Dialogs.h"

#include "cui.h"
#include "CuiLegacy/resources/legacy_movie.hpp"
#include "CuiLegacy/resources/wil_archive.hpp"
#include "CuiLegacy/text/native_text.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <tuple>
#include <utility>

#include <optional>
#include <string_view>
#include <algorithm>
#include <cstdint>
#include "CuiLegacy/resources/messages.hpp"
#include <cstdlib>
#include <functional>
#include <vector>


namespace mir3 {
struct SelectionRole {
    std::string name;
    std::uint8_t profession=0,gender=0,level=0,action=0;
    int x=0,y=0,frame=0;
    std::uint16_t elapsed=0;
    bool occupied=false;
    int animation_index() const { return action+5*(gender+2*profession); }
    int first_frame() const { return 200+animation_index()*60; }
    int last_frame() const {
        static constexpr std::array<int,30> lengths{10,19,11,7,17,10,10,10,11,15,10,11,10,10,14,10,17,10,8,16,10,16,19,11,16,10,16,10,9,14};
        return first_frame()+lengths[animation_index()];
    }
    void start(std::uint8_t value) { action=value;frame=first_frame();elapsed=0; }
    void advance(std::uint32_t ms) {
        elapsed=std::uint16_t(elapsed+ms);
        if (elapsed<=120) return;
        elapsed=0;
        if (++frame<=last_frame()) return;
        if (action==1) start(2);
        else if (action==3) start(0);
        else frame=first_frame();
    }
};
}

namespace mir3 {
struct CreateCharacterPage {
    bool visible=false;
    int selected=0,profession=0;
    std::array<SelectionRole,2> roles;
    std::array<unsigned char,5> buttons{};
    void set_profession(int job) {
        if (job<0 || job>2) return;
        profession=job;buttons={};
        constexpr std::array<std::array<int,2>,6> positions{{{110,110},{110,120},{110,120},{400,160},{420,115},{425,118}}};
        for (int gender=0;gender<2;++gender) {
            auto& role=roles[gender];role={};role.occupied=true;role.profession=job;role.gender=gender;
            const auto position=positions[job+3*gender];role.x=position[0];role.y=position[1];role.start(4);
        }
    }
    void open() { visible=true;set_profession(0); }
    void close() { visible=false;buttons={}; }
    bool select(int gender) {
        if (gender<0 || gender>1 || gender==selected) return false;
        selected=gender;roles[selected].start(4);return true;
    }
    static int name_error(std::string_view name) {
        if (name.size()>13) return 223;
        constexpr std::string_view forbidden=" /@?'\"\\.,:;`~!#$%^&*()-_+=|[{}]<>";
        for (std::size_t i=0;i<name.size();) {
            const unsigned char c=name[i];
            if (!c || forbidden.find(char(c))!=std::string_view::npos) return 225;
            if ((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9')) ++i;
            else { if (i+1>=name.size() || !name[i+1]) return 225;i+=2; }
        }
        return 0;
    }
    std::optional<std::string> request(std::string_view account,std::string_view name) const {
        if (!visible || account.empty() || name.empty() || name_error(name)) return {};
        return std::string(account)+"/"+std::string(name)+"/1/"+std::to_string(profession)+"/"+std::to_string(selected);
    }
    static int failure_message(std::uint32_t reason) { return reason==2?224:(reason==3?226:232); }
    void advance(std::uint32_t ms) { if (visible) for (auto& role:roles) role.advance(ms); }
};
}


namespace mir3 {
// 0x4327E0 scrollbar+1900: page19, travel258, step12, vertical.
class GuildScroll {
public:
    bool captured=false;
    std::uint32_t last_time=0;
    static int thumb_y(int first,int count,int origin_y) {
        const float ratio=count>1 ? float(first)/(count-1) : 0.f;
        return int(origin_y+60.0+258.0*ratio);
    }
    bool input(int kind,int x,int y,int ox,int oy,int width,int height,int count,int& first,std::uint32_t now) {
        const int top=oy+60,bottom=top+258+height,left=ox+557;
        if (kind==2) { last_time=now;const bool was=captured;captured=false;return was; }
        if (kind==1) {
            if (!captured) return false;
            const float ratio=std::clamp(float(y-top)/(bottom-top),0.f,1.f);
            // 0x4339C3 fmul(count-1); Hex-Rays omits this x87 operand.
            first=int(double(std::max(0,count-1))*ratio);return true;
        }
        if (kind!=0 || width<=0 || height<=0 || x<left || x>=left+width || y<top || y>=bottom) return false;
        if (now-last_time<=10) return true;
        last_time=now;
        const int thumb=thumb_y(first,count,oy);
        if (y>=thumb && y<thumb+height) { captured=true;return true; }
        const float old=count>1 ? float(first)/(count-1) : 0.f;
        const float page=count>0 ? 19.f/count : 0.f;
        const float ratio=std::clamp(old+(y<thumb+6?-page:page),0.f,1.f);
        first=int(double(std::max(0,count-1))*ratio);return true;
    }
};
struct GuildLayout {
    // 0x4327E0 construction,0x432A40 post-positioning. frame,x,y,hint.
    static constexpr std::array<std::array<int,4>,9> buttons{{
        {1221,554,401,1052},{1301,39,385,4010},{1304,89,385,4009},
        {1307,139,385,4008},{1310,289,385,4011},{1313,339,385,4012},
        {1316,389,385,4013},{1320,439,385,4014},{1323,489,385,4015}
    }};
    int tab=0,pressed=-1;
    static int denial(int index,bool manager) {
        //0x433420 management controls require+148. Manager actions are separate.
        return index>=4 && index<=8 && !manager ? 4000 : -1;
    }
    int tooltip(int index) const {
        return index>=0 && index<9 && index!=pressed ? buttons[index][3] : -1;
    }
    std::array<int,2> frames(int index) const {
        if (index<0 || index>=9) return {-1,-1};
        // 0x4327E0 passes hover_draw=0, overlay=normal to0x41FA00.
        // 0x41FAF0 therefore draws one overlay even when hovered.
        return {buttons[index][0]+(pressed==index ? 1 : 0),-1};
    }
    bool release(int button) {
        const bool activate=button>=0 && button==pressed;pressed=-1;
        if (activate && button>=1 && button<=3) tab=button-1;
        return activate;
    }
    static int title(int tab) { return tab==0?4010:(tab==1?4009:4008); }
    static std::array<int,4> notice_rect(int row,int text_height) {
        return {35,60+(text_height+5)*row,514,text_height}; //0x433060
    }
};
}


namespace mir3 {
struct LoginAnnouncement {
    struct Row { std::string ansi;int points=9;bool bold=false;std::uint8_t foreground=2,background=0; };
    std::vector<Row> rows;
    int offset=0;
    std::uint32_t elapsed=0;
    bool load(std::span<const std::uint8_t> bytes,const std::function<int(std::string_view,int,bool)>& measure) {
        const auto decoded=decode_legacy_text(bytes);
        if (!decoded || decoded->size()>1024*1024) return false;
        LoginAnnouncement next;
        std::string_view input(*decoded);input=input.substr(0,input.find(char(0)));
        while (!input.empty()) {
            const auto newline=input.find('\n');
            const auto length=std::min<std::size_t>(259,newline==std::string_view::npos?input.size():newline+1);
            std::string line(input.substr(0,length));input.remove_prefix(length);
            if (!line.empty() && line[0]==';') continue;
            const auto end=line.find('\r');if (end!=std::string::npos) line.resize(end);
            else if (!line.empty() && line.back()=='\n') line.pop_back();
            Row style;
            const auto number=[&](char marker,int fallback) {
                const auto at=line.find(marker);if (at==std::string::npos) return fallback;
                return int(std::strtol(line.c_str()+at+1,nullptr,10));
            };
            style.points=number('!',9);if (style.points<1 || style.points>72) return false;
            style.background=std::uint8_t(number('@',0));style.foreground=std::uint8_t(number('#',2));
            style.bold=line.find('$')!=std::string::npos;
            const auto marker=line.find('*');std::string_view text(line);
            if (marker!=std::string::npos) text.remove_prefix(marker+1);
            std::string current;int count=0;
            const auto append_row=[&]() {
                auto row=style;row.ansi=std::move(current);next.rows.push_back(std::move(row));current.clear();++count;
            };
            if (text.empty()) append_row();
            for (std::size_t i=0;i<text.size() && count<10;) {
                const auto c=std::uint8_t(text[i]);const auto size=c>=0x80 && i+1<text.size()?2u:1u;
                const auto unit=text.substr(i,size);
                // 48C03Aï¼šæ°å¥½ç­‰å®½ä»ç•™åœ¨æœ¬è¡Œï¼›æº¢å‡ºçš„å®Œæ•´ANSIå­—ç¬¦ç§»åˆ°ä¸‹ä¸€è¡Œã€‚
                if (!current.empty() && measure(current+std::string(unit),style.points,style.bold)>224) append_row();
                if (count==10) break;
                current.append(unit);i+=size;
                if (i==text.size()) append_row();
            }
        }
        *this=std::move(next);return true;
    }
    void advance(std::uint32_t ms) {
        if (ms<10000) elapsed+=ms;
        if (elapsed<=100) return;
        elapsed=0;
        if (rows.size()*14<=238) return;
        --offset;
        if (std::abs(offset)>=int(rows.size()*14)-14) offset=224;
    }
};
}


namespace mir3 {
struct LoginAudio {
    enum class Page { none,credentials,selection,creation };
    Page pending=Page::none;
    std::uint64_t elapsed=0;
    void enter(Page page) { pending=page;elapsed=0; }
    std::string_view advance(std::uint32_t ms) {
        if (pending==Page::none) return {};
        elapsed+=ms;
        if (elapsed<=(pending==Page::credentials?1200u:1000u)) return {};
        const auto page=pending;pending=Page::none;elapsed=0;
        return page==Page::credentials?"Opening.wav":page==Page::selection?"SelChr.mp3":"CreateChr.mp3";
    }
    static std::string_view voice(bool creation,int profession,int gender) {
        if (profession<0 || profession>2 || gender<0 || gender>1) return {};
        constexpr std::array<std::string_view,6> create{"JMCre.wav","SMCre.wav","DMCre.wav","JWCre.wav","SWCre.wav","DWCre.wav"};
        constexpr std::array<std::string_view,6> select{"JMMSel.wav","SMMSel.wav","DMMSel.wav","JWMSel.wav","SWMSel.wav","DWMSel.wav"};
        return (creation?create:select)[profession+3*gender];
    }
};
}


namespace mir3 {
// 0x43F3B0 supplies these two values to 0x47D450 before decoding a FAC.
struct PortraitFac {
    static constexpr std::uint32_t length_key=static_cast<std::uint32_t>(-1901381136);
    static constexpr std::uint32_t checksum_key=static_cast<std::uint32_t>(-1612834157);
    static constexpr std::size_t max_payload=4095;

    static std::optional<std::string> encode(std::string_view jpeg) {
        if (jpeg.size()>max_payload) return std::nullopt;
        std::string file(8,'\0');
        for (unsigned i=0;i<4;++i)
            file[i]=char(((jpeg.size()>>(24-8*i))^(length_key>>(8*i)))&255);
        file.append(jpeg);
        transform(file); // 0x47D4C0: XOR four incrementing byte streams.
        const auto check=checksum(std::string_view(file).substr(8));
        for (unsigned i=0;i<4;++i) file[4+i]=char((check>>(8*i))&255);
        return file;
    }

    static std::optional<std::string> decode(std::string_view file) {
        if (file.size()<8 || file.size()>8+max_payload) return std::nullopt;
        std::uint32_t length=0, check=0;
        for (unsigned i=0;i<4;++i) {
            length=(length<<8)|(std::uint8_t(file[i])^std::uint8_t(length_key>>(8*i)));
            check|=std::uint32_t(std::uint8_t(file[4+i]))<<(8*i);
        }
        // 0x47D560 verifies checksum before reversing XOR. Also bound the declared length.
        if (length!=file.size()-8 || check!=checksum(file.substr(8))) return std::nullopt;
        std::string decoded(file);
        transform(decoded);
        return decoded.substr(8);
    }

private:
    static std::uint32_t checksum(std::string_view body) {
        std::uint32_t sum=0;
        for (std::size_t i=0;i<body.size();++i)
            sum+=static_cast<std::uint32_t>(i)*(std::uint8_t(body[i])+1u);
        return sum^checksum_key;
    }
    static void transform(std::string& file) {
        for (std::size_t i=0;i<file.size()-8;++i)
            for (unsigned j=0;j<4;++j)
                file[i+8]=char(std::uint8_t(file[i+8])^std::uint8_t(std::uint8_t(file[j])+i));
    }
};
}


namespace mir3 {
// Panel22: 0x41E540/0x41E680/0x41E790/0x41F100/0x41F340.
// Relative coordinates become absolute by adding panel+24/+28.
struct PortraitPicker {
    using Rect=std::array<int,4>;
    static constexpr Rect initial{0,0,580,404};
    static constexpr int frame=2090, visible_rows=12;
    static constexpr Rect preview{390,50,85,104};
    static constexpr std::array<std::array<int,3>,3> buttons{{
        {1221,539,364},{2092,59,345},{2094,179,345}
    }};
    // Draw repositions close from constructor+(400,267) to +(539,364).
    static constexpr std::array<int,3> scroll{1966,276,30};
    static constexpr int scroll_travel=274;
    static constexpr int scroll_page_rows=12; // 0x41FE80 stores page size at +20.
    static constexpr int scroll_page_midpoint=7; // 0x420200 uses +32 >> 1.
    static constexpr Rect row(int i) { return i>=0 && i<visible_rows?Rect{26,27+24*i,238,21}:Rect{}; }
    static constexpr std::array<int,2> instruction(int i) { return {320,165+15*i}; }
    static constexpr int instruction_message(int i) { return i>=0 && i<12?4702+i:-1; }
    static constexpr bool accepts_size(std::uint64_t size) { return size<=4095; }
    std::vector<std::string> names; // original linked list entries: name at +8.
    int hovered=-1;  // panel+724: row within viewport, retained outside rows.
    int selected=-1; // panel+728: absolute entry index.
    int first=0;     // panel+732.
    std::uint32_t last_upload=0; // panel+1188; close does not reset it.
    bool scrolling=false;
    std::uint32_t scroll_tick=0;
    int thumb_y() const {
        const float ratio=names.size()>1?float(double(first)/(names.size()-1)):0;
        return 30+int(scroll_travel*double(ratio));
    }
    void wheel(int direction) { // 0x41F400/0x41F420; +736 is count+1 after enumeration.
        if (direction>0 && first>0) --first;
        if (direction<0 && first<int(names.size())-1) ++first;
    }
    bool scroll_input(int kind,int x,int y,int width,int height,std::uint32_t now) {
        if (kind==2) { const bool used=scrolling;scrolling=false;scroll_tick=now;return used; }
        if (kind==1) {
            if (!scrolling) return false;
            drag(float(double(y-30)/(scroll_travel+height)));return true;
        }
        if (kind!=0 || width<=0 || height<=0 || x<276 || x>=276+width || y<30 || y>=30+scroll_travel+height) return false;
        if (now-scroll_tick<=10) return true;
        scroll_tick=now;
        const float ratio=names.size()>1?float(double(first)/(names.size()-1)):0;
        // 0x41FF91..0x41FFDF uses the image height for the vertical thumb RECT.
        const int top=thumb_y();
        if (y>=top && y<top+height) {
            scrolling=true;
            // 0x420200 captures the thumb, then 0x41F060 converts its existing
            // ratio to a row using the file count, including at ratio 1.
            drag(ratio);
            return true;
        }
        const float page=names.empty()?0:float(double(scroll_page_rows)/names.size());
        drag(ratio+(y<top+scroll_page_midpoint?-page:page));
        return true;
    }
    static int row_at(int x,int y) {
        for (int i=0;i<visible_rows;++i) {
            const auto r=row(i);
            if (x>=r[0] && x<r[0]+r[2] && y>=r[1] && y<r[1]+r[3]) return i;
        }
        return -1;
    }
    void move(int x,int y) {
        const int i=row_at(x,y);
        if (i>=0) hovered=i; // 0x41F340: no clearing when no RECT matches.
    }
    bool select(int x,int y) {
        const int i=row_at(x,y);
        if (i<0 || first+i>=static_cast<int>(names.size())) return false;
        selected=first+i;return true;
    }
    void drag(float ratio) {
        // 0x41F340 multiplies count, not count-1. At bottom an empty page is possible.
        first=static_cast<int>(names.size()*static_cast<double>(std::clamp(ratio,0.0f,1.0f)));
    }
    int selected_row() const {
        const int i=selected-first;
        return selected>=0 && i>=0 && i<visible_rows?i:-1;
    }
    bool accept_upload_tick(std::uint32_t now) {
        // Called only after JPEG validation and successful file read (0x41ECC0).
        if (selected<0 || selected>=static_cast<int>(names.size()) || now-last_upload<1000) return false;
        last_upload=now;return true;
    }
    void close() {
        names.clear();hovered=-1;selected=-1;first=0;scrolling=false; // 0x41F440.
    }
};
}


namespace mir3 {
enum class SelectionBlend { normal, inactive, shadow, additive };
// åŽŸç‰ˆçŸ©å½¢é‡‡ç”¨ ONE / INVSRCALPHAï¼›åˆ›å»ºé¡µå…ˆåˆæˆè¯´æ˜Žæ¡†ï¼Œå†ç»˜åˆ¶äººç‰©é˜´å½±ã€‚
inline void selection_rectangle(std::vector<std::uint8_t>& pixels,int x,int y,int width,int height,
        unsigned color,unsigned alpha) {
    if (pixels.size()!=640*480*4 || alpha>255) return;
    for (int py=std::max(0,y);py<std::min(480,y+height);++py)
        for (int px=std::max(0,x);px<std::min(640,x+width);++px) {
            const auto offset=(std::size_t(py)*640+px)*4;
            for (int c=0;c<3;++c) pixels[offset+c]=std::uint8_t(std::min(255u,
                ((color>>(8*c))&255u)+(pixels[offset+c]*(255-alpha)+127)/255));
            pixels[offset+3]=255;
        }
}
inline void selection_text(std::vector<std::uint8_t>& pixels,const std::vector<std::uint8_t>& glyphs,
        int x,int y,int width,int height,unsigned color) {
    if (pixels.size()!=640*480*4 || width<=0 || height<=0 || glyphs.size()!=std::size_t(width)*height*4) return;
    for (int sy=0;sy<height;++sy) for (int sx=0;sx<width;++sx) {
        const int dx=x+sx,dy=y+sy;if (dx<0 || dy<0 || dx>=640 || dy>=480) continue;
        const auto alpha=glyphs[(std::size_t(sy)*width+sx)*4+3];
        if (!alpha) continue;
        const auto offset=(std::size_t(dy)*640+dx)*4;
        for (int c=0;c<3;++c) pixels[offset+c]=std::uint8_t(
            (((color>>(8*c))&255u)*alpha+pixels[offset+c]*(255-alpha)+127)/255);
    }
}

inline void selection_blit(std::vector<std::uint8_t>& destination,const WilFrame& source,int x,int y,SelectionBlend blend) {
    if (destination.size()!=640*480*4) return;
    x+=source.offset_x;y+=source.offset_y;
    for (int sy=0;sy<source.height;++sy) for (int sx=0;sx<source.width;++sx) {
        const int dx=x+sx,dy=y+sy;
        if (dx<0 || dy<0 || dx>=640 || dy>=480) continue;
        const auto si=(std::size_t(sy)*source.width+sx)*4,di=(std::size_t(dy)*640+dx)*4;
        if (!source.pixels.rgba[si+3]) continue;
        int r=source.pixels.rgba[si]>>3,g=source.pixels.rgba[si+1]>>2,b=source.pixels.rgba[si+2]>>3;
        if (blend==SelectionBlend::inactive) {
            const int gray=(r+(g>>1)+b)/3;
            r=b=std::min(31,gray+int(gray*0.6f));g=r*2;
        } else if (blend==SelectionBlend::shadow) {
            r=(r+(destination[di]>>3))/2;g=(g+(destination[di+1]>>2))/2;b=(b+(destination[di+2]>>3))/2;
        } else if (blend==SelectionBlend::additive) {
            r=std::min(31,r+(destination[di]>>3));g=std::min(63,g+(destination[di+1]>>2));b=std::min(31,b+(destination[di+2]>>3));
        }
        destination[di]=r<<3;destination[di+1]=g<<2;destination[di+2]=b<<3;destination[di+3]=255;
    }
}
}


namespace mir3 {
struct ButtonSkin {
    int normal;          // +24
    int pressed;         // +28
    int overlay = -1;    // +32
    bool hover_draw = false; // +48
};
inline std::array<int, 2> button_frames(ButtonSkin skin, int state, bool enabled = true)
{
    if (state == 2) return {skin.pressed, -1};
    if (!enabled) return {-1, -1};
    if (state == 1) return {skin.hover_draw ? skin.normal : -1, skin.overlay};
    return {skin.overlay, -1};
}
// 0x435820 initial configuration; background GameInter[1160/1162] already
// contains the idle icons. Dynamic toggles can override these skins later.
inline std::array<int, 2> main_button_frames(int index, bool hovered, bool pressed)
{
    constexpr std::array<int, 23> normal{1170,1172,1174,1176,1178,1180,
        1182,1184,1186,1188,1190,1192,1194,1196,1168,1205,
        1330,1333,1335,1337,1338,1340,1342};
    if (index < 0 || index >= static_cast<int>(normal.size())) return {-1, -1};
    const bool pair = index >= 5 && index <= 13;
    return button_frames({normal[index], normal[index] + (pair ? 1 : 0), -1,
        index >= 5 && index <= 14}, pressed ? 2 : (hovered ? 1 : 0));
}
}

namespace mir3 {
// Display data only. Offline preview values are explicit, not server state.
struct HudModel {
    HudLayout layout;
    std::string map_name;
    int map_x=0, map_y=0, level=1, profession=0, appearance=0;
    std::uint16_t hp=0, max_hp=0, mp=0, max_mp=0;
    unsigned experience=0, next_experience=0, weight=0, max_weight=0;
    std::array<unsigned,2> wear_weight{}, hand_weight{}, status_bonus_percent{};
    std::array<int,2> defence{}, attack{}, magic_defence{}, magic_attack{}, spirit_attack{};
    std::array<bool,2> primary_stat{true,true};
    HudLayout::Point pointer{-1,-1};
    std::uint32_t emblem_selector=0;
    std::array<std::uint32_t,4> emblem_references{};
    int hovered=-1, pressed=-1;
    std::array<HudLayout::Point,23> button_sizes{};
    bool shared_hp() const { return profession==0 && level<24; }
    int portrait_frame() const { return profession>=0 && profession<=2?650+profession*2+(appearance!=0):-1; }
    std::array<int,3> stat(int side) const {
        const auto range=side==0?(primary_stat[0]?defence:magic_defence):
            (primary_stat[1]?attack:profession==2?spirit_attack:magic_attack);
        return {1500+side+(primary_stat[side]?0:2),range[0],range[1]};
    }
    bool press_stat(int x,int y) {
        if (y<570 || y>=590) return false;
        const int side=x>=634 && x<664?0:x>=717 && x<747?1:-1;
        if (side<0) return false;
        primary_stat[side]=!primary_stat[side];return true;
    }
    int orb_frame(int side) const { return side==0?1163:shared_hp()?1165:1164; }
    HudLayout::Rect orb_crop(int side,HudLayout::Point left,HudLayout::Point right) const {
        const auto size=side==0 || shared_hp()?left:right;
        if (side<0 || side>1 || size[0]<=0 || size[1]<=0) return {};
        const auto amount=side==0 || shared_hp()?hp:mp;
        const auto capacity=side==0 || shared_hp()?max_hp:max_mp;
        const float ratio=capacity?static_cast<float>(std::min(1.0,double(amount)/capacity)):0.0f;
        const int top=static_cast<int>(size[1]-size[1]*double(ratio));
        return {0,top,size[0],size[1]-top};
    }
    int meter_crop(int meter,int height) const {
        const auto amount=meter==0?experience:weight, capacity=meter==0?next_experience:max_weight;
        const float ratio=capacity?static_cast<float>(std::min(1.0,double(amount)/capacity)):0.0f;
        return static_cast<int>(height-height*double(ratio));
    }
    HudLayout::Rect button_rect(int index) const {
        if (index<0 || index>=HudLayout::count(layout.expanded)) return {};
        const auto size=button_sizes[index];
        return HudLayout::hit_rect(index,layout.expanded,size[0],size[1],layout.repositioned);
    }
};
class HudController {
public:
    explicit HudController(HudModel& model): model(model) {}
    int hit(int x,int y) const {
        for (int i=0;i<HudLayout::count(model.layout.expanded);++i) {
            const auto r=model.button_rect(i);
            if (x>=r[0] && y>=r[1] && x<r[0]+r[2] && y<r[1]+r[3]) return i;
        }
        return -1;
    }
    void motion(int x,int y) { model.pointer={x,y};model.hovered=hit(x,y); }
    int pointer(int x,int y,bool down) {
        motion(x,y);
        if (down) { model.pressed=model.press_stat(x,y)?-1:model.hovered; return -1; }
        model.pressed=-1;
        // 0x41FCD0 activates on release inside, even if pressed outside.
        const int action=model.hovered;
        if (action==14) { model.layout.toggle(); motion(x,y); }
        return action;
    }
    void cancel() { model.pressed=model.hovered=-1;model.pointer={-1,-1}; }
private:
    HudModel& model;
};
}

namespace mir3 {
namespace {
struct Surface {
    // The original PE has no DPI-awareness manifest. Isolate legacy GDI from
    // Godot's per-monitor DPI context without changing the window's awareness.
    DPI_AWARENESS_CONTEXT previous_dpi = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE);
    HDC dc = CreateCompatibleDC(nullptr);
    HBITMAP bitmap = nullptr;
    HFONT font = nullptr;
    HGDIOBJ old_bitmap = nullptr;
    HGDIOBJ old_font = nullptr;
    ~Surface() {
        if (old_font) SelectObject(dc, old_font);
        if (old_bitmap) SelectObject(dc, old_bitmap);
        if (font) DeleteObject(font);
        if (bitmap) DeleteObject(bitmap);
        if (dc) DeleteDC(dc);
        if (previous_dpi) SetThreadDpiAwarenessContext(previous_dpi);
    }
};
std::string encode(const wchar_t* text, int size, UINT page) {
    const int count = WideCharToMultiByte(page, 0, text, size, nullptr, 0, nullptr, nullptr);
    std::string result(count, '\0');
    if (count) WideCharToMultiByte(page, 0, text, size, result.data(), count, nullptr, nullptr);
    return result;
}
}

namespace {
std::string wrapped_ansi(HDC dc, const std::string& ansi, int width, int& count) {
    std::string buffer;
    int start = 0, high_bytes = 0; count = 1;
    for (int i = 0; i < static_cast<int>(ansi.size()); ++i) {
        SIZE extent{};
        GetTextExtentPoint32A(dc, ansi.data() + start, i - start + 1, &extent);
        const bool high = static_cast<signed char>(ansi[i]) < 0;
        if (extent.cx <= width) {
            if (high) ++high_bytes;
            buffer += ansi[i];
        } else if (high && high_bytes % 2) {
            buffer.back() = '`';
            start = i - 1;
            i -= 2;
            --high_bytes;
            ++count;
        } else {
            buffer += '`';
            start = i;
            --i;
            ++count;
        }
    }
    return buffer;
}
}

//0x44FD40: wrap with normal9pt font; even title width is measured in it.
std::vector<DescriptionRow> wrap_skill_description(const std::vector<std::string>& lines) {
    std::vector<DescriptionRow> out;
    Surface surface;if (!surface.dc) return out;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    surface.font=CreateFontA(-MulDiv(9,GetDeviceCaps(surface.dc,LOGPIXELSY),72),0,0,0,400,0,0,0,1,0,0,0,0,face);
    if (!surface.font) return out;
    surface.old_font=SelectObject(surface.dc,surface.font);
    for (const auto& line:lines) {
        if (line.size()>1299) continue; // original fixed line buffer
        int count=0;const auto buffer=wrapped_ansi(surface.dc,line,165,count);
        std::size_t pos=0;bool failed=false;
        for (int i=0;i<std::min(count,20);++i) {
            std::string row;
            if (!failed) {
                const auto end=buffer.find('`',pos);
                const auto length=(end==buffer.npos?buffer.size():end)-pos;
                if (!length) failed=true;
                else {
                    row=buffer.substr(pos,std::min<std::size_t>(length,259));
                    pos=end==buffer.npos?buffer.size():end+1;
                    while (pos<buffer.size() && (buffer[pos]==' ' || buffer[pos]=='\t' || buffer[pos]=='\r' || buffer[pos]=='\n')) ++pos;
                    if (end==buffer.npos) failed=true;
                }
            }
            SIZE size{};GetTextExtentPoint32A(surface.dc,row.data(),int(row.size()),&size);
            out.push_back({ansi_to_utf8(row),int(size.cx),!row.empty() && row[0]=='['});
        }
    }
    return out;
}

std::vector<std::string> wrap_system_notice(std::string_view utf8)
{
    if (utf8.empty() || utf8.size()>4096) return {};
    auto ansi = utf8_to_ansi(utf8);
    if (const auto zero=ansi.find('\0'); zero!=ansi.npos) ansi.resize(zero);
    if (ansi.empty() || ansi.size()>1023) return {};

    Surface surface;
    if (!surface.dc) return {};
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    surface.font=CreateFontA(-MulDiv(9,GetDeviceCaps(surface.dc,LOGPIXELSY),72),
        0,0,0,400,0,0,0,DEFAULT_CHARSET,0,0,0,0,face);
    if (!surface.font) return {};
    surface.old_font=SelectObject(surface.dc,surface.font);
    int count=0;
    const auto buffer=wrapped_ansi(surface.dc,ansi,514,count);
    char rows[5][260]{};
    std::sscanf(buffer.c_str(),"%259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c",
        rows[0],rows[1],rows[2],rows[3],rows[4]);
    std::vector<std::string> result;
    for (int i=0;i<std::min(count,5);++i) result.push_back(ansi_to_utf8(rows[i]));
    return result;
}

// 0x4366D0 inserts into separately wrapped compact and expanded histories.
void ChatHistory::post(std::string_view utf8, std::uint32_t foreground, std::uint32_t background)
{
    if (utf8.empty() || utf8.size() > 4096) return;
    const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(),
        static_cast<int>(utf8.size()), nullptr, 0);
    std::wstring wide(length, L'\0');
    if (length) MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(),
        static_cast<int>(utf8.size()), wide.data(), length);
    auto ansi = encode(wide.data(), length, CP_ACP);
    ansi.resize(ansi.find('\0') == std::string::npos ? ansi.size() : ansi.find('\0'));
    if (ansi.empty() || ansi.size() > 200) return; // 0x4366D0: strlen, not UTF-8 length.
    Surface surface;
    if (!surface.dc) return;
    const char face[] = {char(0xcb), char(0xce), char(0xcc), char(0xe5), 0};
    surface.font = CreateFontA(-MulDiv(9, GetDeviceCaps(surface.dc, LOGPIXELSY), 72),
        0, 0, 0, 400, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, face);
    if (!surface.font) return;
    surface.old_font = SelectObject(surface.dc, surface.font);
    for (int mode = 0; mode < 2; ++mode) {
        // 0x48C03A deliberately measures successive ANSI byte prefixes. Preserve
        // its signed-byte parity/backtracking, rather than Unicode word wrapping.
        int count=0;
        const auto buffer=wrapped_ansi(surface.dc,ansi,mode?429:341,count);
        // Original sscanf uses backticks and consumes whitespace after separators.
        // Existing backticks in a message therefore have the same legacy behavior.
        char rows[5][260]{};
        std::sscanf(buffer.c_str(), "%259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c",
            rows[0], rows[1], rows[2], rows[3], rows[4]);
        for (int i = 0; i < std::min(count, mode ? 3 : 5); ++i) {
            // 0x43682E / 0x436902: expanded index checked in BOTH insertion loops.
            if (static_cast<int>(logs[1].size()) - scrolls[1].position() == 14)
                scrolls[1].set_position(scrolls[1].position() + 1);
            if (!mode && static_cast<int>(logs[0].size()) - scrolls[0].position() == 5)
                scrolls[0].set_position(scrolls[0].position() + 1);
            logs[mode].push_back({ansi_to_utf8(rows[i]), foreground, background});
        }
    }
    // 0x436991: trim only after both insertion loops have finished.
    for (int mode = 0; mode < 2; ++mode) {
        while (logs[mode].size() >= 80) {
            logs[mode].pop_front();
            if (scrolls[mode].position() > 0)
                scrolls[mode].set_position(scrolls[mode].position() - 1);
        }
    }
}

std::vector<SpeechLine> wrap_speech(std::string_view utf8)
{
    if (utf8.empty() || utf8.size()>4096) return {};
    const int length=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,utf8.data(),static_cast<int>(utf8.size()),nullptr,0);
    if (!length) return {};
    std::wstring wide(length,L'\0');
    MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,utf8.data(),static_cast<int>(utf8.size()),wide.data(),length);
    auto ansi=encode(wide.data(),length,CP_ACP);
    const auto nul=ansi.find(char(0));if (nul!=ansi.npos) ansi.resize(nul);
    if (ansi.empty() || ansi.size()>259) return {}; // original actor buffer260
    Surface surface;
    if (!surface.dc) return {};
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    surface.font=CreateFontA(-MulDiv(9,GetDeviceCaps(surface.dc,LOGPIXELSY),72),0,0,0,400,0,0,0,DEFAULT_CHARSET,0,0,0,0,face);
    if (!surface.font) return {};
    surface.old_font=SelectObject(surface.dc,surface.font);
    int count=0;
    const auto buffer=wrapped_ansi(surface.dc,ansi,144,count);
    if (count>8) return {}; // reject legacy fixed-buffer overflow
    char rows[8][260]{};
    std::sscanf(buffer.c_str(),"%259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c",rows[0],rows[1],rows[2],rows[3],rows[4],rows[5],rows[6],rows[7]);
    std::vector<SpeechLine> result;
    for (int i=0;i<count;++i) {
        SIZE size{};GetTextExtentPoint32A(surface.dc,rows[i],static_cast<int>(std::strlen(rows[i])),&size);
        result.push_back({ansi_to_utf8(rows[i]),size.cx});
    }
    return result;
}

}

namespace mir3_client {

namespace {

int AnsiBytes(const char* utf8, int length)
{
    return static_cast<int>(mir3::utf8_to_ansi({utf8, static_cast<std::size_t>(length)}).size());
}

struct SceneSprite {
    cui_texture texture = 0;
    int x = 0, y = 0, width = 0, height = 0, row = 0;
    std::uint8_t opacity = 255;
    float sourceWidth = 1;
    std::optional<cui_effect_blend> effect;
    cui_color tint{255,255,255,255};
};

void ApplyActorBlend(SceneSprite& sprite,std::uint16_t status,int appearance,int layerWeight,
                     bool shadow,bool shadowBlend)
{
    if (status==1 || (shadow && (appearance>=0 || shadowBlend))) {
        sprite.effect=CUI_EFFECT_RGB565;
        sprite.tint.a=shadow ? 13 : 8;
    } else if (appearance>=0) {
        sprite.effect=status==65535 ? CUI_EFFECT_RGB565 : CUI_EFFECT_RGB565_AVERAGE;
        sprite.tint.a=std::uint8_t(layerWeight);
    }
}

struct SceneActorHit {
    cui_rect bounds{};
    std::uint32_t actor = 0;
    unsigned row = 0;
    bool monster = false;
};

struct GroundLabel {
    cui_texture texture;
    int x, y, width;
};

struct SceneCaption {
    cui_texture texture;
    std::array<int,4> rect;
    cui_color color;
};

struct SceneEffectQuad {
    cui_texture texture;
    std::array<cui_vec2,4> corners;
    cui_color tint;
    bool upright;
    cui_effect_blend blend;
};

struct SceneActorAnchor {
    std::uint32_t actor;
    cui_rect bounds;
    int x, y, order;
};

struct SceneMapArt {
    const std::vector<SceneSprite>* sprites = nullptr;
    const std::vector<SceneActorHit>* npcs = nullptr;
    const std::vector<SceneActorHit>* targets = nullptr;
    const std::vector<SceneActorHit>* corpses = nullptr;
    const std::vector<GroundLabel>* groundLabels = nullptr;
    const std::vector<SceneCaption>* names = nullptr;
    const std::vector<SceneCaption>* speech = nullptr;
    int cameraX = 0, cameraY = 0;
    std::size_t overlayStart = 0;
    const mir3::SceneLightGrid* light = nullptr;
    const std::vector<SceneEffectQuad>* weather = nullptr;
    const std::vector<SceneEffectQuad>* levelEffects = nullptr;
    const std::vector<SceneEffectQuad>* spellEffects = nullptr;
};

}

struct Dialogs::Assets {
public:
    bool Open(cui_ctx* ui, const std::filesystem::path& root)
    {
        mir3::WilArchive selection;
        mir3::WilArchive gameInterface;
        const auto data = root / "Data";
        movieDirectory_ = data;
        mapData_ = data;
        mapWeaponOrder_ = Read(root/"Weapon.ord");
        if (!selection.open(data / "Interface1c") || !gameInterface.open(data / "GameInter")) return false;
        if (!messages_.load(Read(root / "CmList.dat"))) return false;
        font_ = Message(1074);

        constexpr int indexes[] = {0, 2, 10, 11, 12, 13, 14, 15, 16, 17};
        for (std::size_t i = 0; i < backgrounds_.size(); ++i)
            backgrounds_[i] = UploadFrame(ui, selection, indexes[i]);
        for (std::size_t i = 0; i < dialog_.size(); ++i)
            dialog_[i] = UploadFrame(ui, gameInterface, i == 0 ? 1240 : 1242 + static_cast<int>(i));
        for (const int id : {112, 113, 114, 116, 117, 119, 121}) {
            const auto caption = Message(id);
            if (!caption.empty()) captions_.emplace(id, Raster(ui, caption, 308, 95, true, mir3::TextFont::dialog));
        }
        for (int i = 0; i < static_cast<int>(legends_.size()); ++i)
            legends_[i] = Raster(ui, Message(129 + i), 180, 16, false, mir3::TextFont::normal);
        exitUrl_ = Message(1077);
        editorFont_ = LoadEditorFont(ui, font_);
        clickSound_ = root / L"Sound" / L"105.wav";
        if (announcement_.load(Read(root / "Notice.ntc"), [](std::string_view ansi, int points, bool bold) {
            return mir3::raster_text(ConvertText(std::string(ansi), 936, CP_UTF8), 512, 128, false, mir3::TextFont::normal, {},
                false, points, bold ? 700 : 400).text_width;
        })) {
            for (const auto& row : announcement_.rows) {
                const auto bitmap = mir3::raster_text(ConvertText(row.ansi, 936, CP_UTF8), 224, 128, false,
                    mir3::TextFont::normal, {}, false, row.points, row.bold ? 700 : 400);
                announcementImages_.push_back({Upload(ui, 224, 128, bitmap.rgba, true), bitmap.text_width, bitmap.text_height});
            }
        }
        ready_ = std::all_of(backgrounds_.begin(), backgrounds_.end(),
            [](const Image& image) { return image.texture != 0; }) &&
            std::all_of(dialog_.begin(), dialog_.end(), [](const Image& image) { return image.texture != 0; });
        OpenGroups(ui, data / "LoginInterface");
        selectionArchive_ = std::move(selection);
        OpenSelection(ui, gameInterface);
        for (int id : {1241,1242,1245,1246})
            accountButtons_[id] = UploadFrame(ui,gameInterface,id);

        OpenParty(ui, gameInterface);
        OpenQuests(ui, gameInterface);
        OpenGuild(ui, gameInterface);
        OpenNpc(ui, gameInterface);
        OpenShop(ui, gameInterface, data / "Inventory");
        OpenGameHud(ui, gameInterface, data / "ProgUse");
        OpenGameOptions(ui, gameInterface);
        OpenSkills(ui, gameInterface, root);
        OpenMount(ui, gameInterface);
        OpenTrade(ui, gameInterface);
        OpenStatus(ui, gameInterface, data);
        constexpr std::array independentFrames{1288,1590,1890,1910,1960};
        for (std::size_t i = 0; i < independentFrames.size(); ++i)
            independentSkins_[i] = UploadFrame(ui, gameInterface, independentFrames[i], true);
        for (const int frame : {1221,1222,1592,1594,1596,1894,1896,1898,1900,1904,1906,1908,
                                1962,1964,1966,1970,1971,1974,1980,1982,1984,1986,1988,2012,2014,2016,2032,2034})
            independentButtons_.emplace(frame, UploadFrame(ui,gameInterface,frame,true));
        constexpr std::array mailModeFrames{2010,2030,2070};
        for (std::size_t i = 0; i < mailModeFrames.size(); ++i)
            mailModeSkins_[i] = UploadFrame(ui, gameInterface, mailModeFrames[i], true);
        OpenPortraitPicker(ui, gameInterface, root);
        return ready_;
    }

    void Close(cui_ctx* ui)
    {
        for (const auto tex : textures_) cui_image_destroy(ui, tex);
        *this = {};
    }

    bool Ready() const { return ready_; }
    bool GameHudReady() const { return gameHudReady_; }
    void DrawGameHud(cui_ctx* ui, LoginClient& client, bool pointerCovered, bool modal);
    void DrawChatShortcutOverlay(cui_ctx* ui, const mir3_cui::ChatPanel& chat);
    void PrepareMinimap(cui_ctx* ui, LoginClient& client);
    bool HandleMinimapInput(cui_ctx* ui, LoginClient& client, bool modal);
    void DrawMinimapPanel(cui_ctx* ui, const LoginClient& client);
    void DrawObjectOrderMap(cui_ctx* ui, LoginClient& client, float height);
    void DrawIndependentSkin(cui_ctx* ui, int panel, cui_rect bounds);
    cui_vec2 IndependentButtonSize(int frame) const;
    void DrawIndependentButton(cui_ctx* ui, int frame, float height, int state) const;
    cui_input_opts IndependentEditor(float height, float scale) const { return ModalEditorOptions(height,scale); }
    void DrawMailModeSkin(cui_ctx* ui, int mode, float height);
    void DrawIndependentItem(cui_ctx* ui, unsigned icon, float height);
    bool DrawInspectionEquipment(cui_ctx* ui, const IndependentWindows::Profile& profile, cui_rect bounds);
    void DrawTargetDetails(cui_ctx* ui, bool attributes);
    bool OptionsReady() const { return optionsFrames_[0].texture != 0; }
    bool HandleOptionsInput(cui_ctx* ui, LoginClient& client, bool blocked, bool audioOnly = false);
    void DrawOptionsPanel(cui_ctx* ui, const LoginClient& client);
    void LeaveOptions(bool resetPlacement = false);
    bool HandleUtilityPanels(cui_ctx* ui, LoginClient& client, bool covered, bool foreground, bool modal,
                             bool& escape, bool& pointerCaptured);
    void DrawUtilityPanels(cui_ctx* ui, LoginClient& client, bool inventoryModal);
    bool StatusReady() const { return statusSkin_.texture != 0; }
    void DrawTradePrompt(cui_ctx* ui, LoginClient& client);
    void DrawPanelHeld(cui_ctx* ui, LoginClient& client);
    void PostTradeFeedback(LoginClient& client);
    bool SkillsReady() const { return skillFrames_.contains(1620) && skillFrames_.at(1620).texture; }
    void ToggleSkills(LoginClient& client);
    bool HandleSkillInput(cui_ctx* ui, LoginClient& client, bool blocked, bool keyboard);
    void DrawSkillsPanel(cui_ctx* ui, const LoginClient& client);
    void DrawSkillShortcuts(cui_ctx* ui, const LoginClient& client);
    bool GroupsReady() const { return groupsReady_; }
    bool PartyReady() const { return partyReady_; }
    bool QuestReady() const { return questReady_; }
    bool GuildReady() const { return guildReady_; }
    bool NpcReady() const { return npcReady_; }
    bool ShopReady() const { return shopReady_; }
    bool ShopHasModal(const LoginClient& client) const;
    bool InventoryReady() const { return inventoryReady_; }
    bool InventoryBlocksScene(cui_ctx* ui, const LoginClient& client) const;
    void DrawInventoryPanel(cui_ctx* ui, LoginClient& client, bool interactive, bool shortcut);
    bool QuickReady() const { return quickFrames_[0].texture && quickFrames_[1].texture && quickFrames_[2].texture; }
    bool QuickBlocksScene(cui_ctx* ui, const LoginClient& client) const;
    void DrawQuickPanel(cui_ctx* ui, LoginClient& client, bool interactive);
    void PostMerchantResult(LoginClient& client) const;
    void LeaveInventory(bool resetView = false);
    bool ShopBlocksScene(cui_ctx* ui, const LoginClient& client, bool hitOnly=false) const;
    void DrawShopPanel(cui_ctx* ui, LoginClient& client, int wheel, bool interactive);
    bool NpcBlocksScene(cui_ctx* ui, const LoginClient& client, bool hitOnly=false) const;
    void DrawNpcPanel(cui_ctx* ui, LoginClient& client, bool interactive, int wheel);
    bool GuildBlocksScene(cui_ctx* ui) const;
    void DrawGuildPanel(cui_ctx* ui, LoginClient& client, bool drawPanel = true, bool drawDialog = true);
    bool HandleGuildInput(cui_ctx* ui, LoginClient& client, bool blocked, bool fallback = false);
    void LeaveGuild();
    void DrawQuestPanel(cui_ctx* ui, LoginClient& client);
    bool HandleQuestInput(cui_ctx* ui, LoginClient& client, bool blocked, bool fallback = false);
    static void PostQuestNotices(LoginClient& client, const Assets* assets);
    bool QuestBlocksScene(cui_ctx* ui) const;
    void LeaveQuests() {
        questFocused_ = questDragging_ = questScrollDrag_ = false;
        questPressed_ = questHint_ = -1;
    }
    bool PartyBlocksScene(cui_ctx* ui) const
    {
        if (partyDragging_ || partyPressed_ >= 0) return true;
        const auto viewport = cui_viewport_size(ui);
        const float scale = (std::min)(viewport.x / 800, viewport.y / 600);
        if (scale <= 0) return false;
        const auto mouse = cui_mouse_pos(ui);
        const float x = (mouse.x - (viewport.x - 800 * scale) / 2) / scale;
        const float y = (mouse.y - (viewport.y - 600 * scale) / 2) / scale;
        return x >= partyX_ && x < partyX_ + 258 && y >= partyY_ && y < partyY_ + 244;
    }
    void DrawPartyPanel(cui_ctx* ui, LoginClient& client, bool drawPanel = true, bool drawDialog = true);
    bool HandlePartyInput(cui_ctx* ui, LoginClient& client, bool blocked, bool fallback = false);
    void LeaveParty() {
        partyFocused_ = partyDragging_ = false;
        partyPressed_ = partyConfirmPressed_ = partyEditor_ = -1;
    }
    bool DrawTransition(cui_ctx* ui, LoginClient& client);
    bool DrawGameExit(cui_ctx* ui, LoginClient& client);
    void DrawGameExitFade(cui_ctx* ui, const LoginClient& client);
    bool PortraitNoticeOpen() const { return portraitNotice_ != 0; }
    void DrawPortraitNotice(cui_ctx* ui, LoginClient& client);
    void LeavePortraitPicker(cui_ctx* ui);
    bool DefersGameEntry() const { return moviePurpose_ == MoviePurpose::Game && !movieFinished_; }
    bool SelectionReady(const LoginClient& client) const
    {
        return selectionReady_ && (client.characterDialog != CharacterDialog::Create || creationReady_);
    }

    void DrawSelection(cui_ctx* ui, LoginClient& client);
    void LeaveSelection() { selectionTick_ = -1; selectionQueryAt_ = -1; }
    void DrawGroups(cui_ctx* ui, LoginClient& client)
    {
        auto& model = client.serverGroups;
        const auto viewport = cui_viewport_size(ui);
        const float scale = (std::min)(viewport.x / 800, viewport.y / 600);
        if (scale <= 0) return;
        const auto mouse = cui_mouse_pos(ui);
        const auto previousPointer = groupPointer_;
        groupPointer_ = {(mouse.x - (viewport.x - 800 * scale) / 2) / scale,
                         (mouse.y - (viewport.y - 600 * scale) / 2) / scale};
        const bool dragged = groupScrollDrag_ &&
            (groupPointer_.x != previousPointer.x || groupPointer_.y != previousPointer.y);
        const bool down = cui_mouse_clicked(ui, CUI_MOUSE_LEFT);
        const bool up = cui_mouse_released(ui, CUI_MOUSE_LEFT);
        const auto& exit = groupFrames_[3];
        const bool exitHit = GroupHit(760, 13, exit.width, exit.height);
        if (up) {
            groupScrollDrag_ = false;
            groupExitPressed_ = false;
            if (exitHit) cui_app_quit(nullptr);
        }
        if (down) {
            groupExitPressed_ = exitHit;
            for (int i = 0; i < static_cast<int>(model.groups.size()); ++i) {
                const auto& group = model.groups[i];
                if (GroupHit(group.x, group.y, 40, 40)) {
                    model.selected = i;
                    firstGroupRow_ = 0;
                    client.status.clear();
                }
            }
        }
        for (const auto& group : model.groups)
            CacheGroupText(ui, groupLabels_, group.label, 260, 16, false);
        if (model.selected >= 0) {
            const auto& group = model.groups[model.selected];
            const int count = static_cast<int>(group.rowCount);
            const int last = (std::max)(0, count - 1);
            firstGroupRow_ = std::clamp(firstGroupRow_ - static_cast<int>(std::lround(cui_mouse_scroll(ui).y)), 0, last);
            const auto& thumb = groupFrames_[15];
            const float span = 447.0f + thumb.height;
            if (dragged) {
                const float fraction = std::clamp((groupPointer_.y - 84) / span, 0.0f, 1.0f);
                firstGroupRow_ = std::clamp(static_cast<int>(count * fraction), 0, last);
            }
            if (down && GroupHit(204, 84, thumb.width, span) && uiGroupScrollTime_ + 0.01 < cui_time(ui)) {
                if (GroupHit(204, GroupThumbY(count), thumb.width, thumb.height)) {
                    groupScrollDrag_ = true;
                } else if (count) {
                    const float step = groupPointer_.y < GroupThumbY(count) + 15 ? -10.0f : 10.0f;
                    const float next = std::clamp(GroupScrollRatio(count) + step / count, 0.0f, 1.0f);
                    firstGroupRow_ = (std::min)(last, static_cast<int>(count * next));
                }
                uiGroupScrollTime_ = cui_time(ui);
            }
            CacheGroupText(ui, groupTitles_, group.title, 172, 15, true);
            for (const auto& line : group.lines) {
                const int row = static_cast<int>(line.row) - firstGroupRow_;
                if (row < 0 || row >= 20) continue;
                CacheGroupText(ui, groupLines_, line.caption, 190, 15, true);
                if (down && GroupHit(10, 70 + 25 * row, 190, 15) && IsGroupLineAvailable(line) &&
                    client.OpenGroupLine(line)) {
                    ResetCredentials();
                    cui_request_redraw(ui);
                    break;
                }
            }
        }

        const GroupPaint modelForPaint{this, &model};
        cui_canvas(ui, "server-groups-art", CUI_FILL, CUI_FILL, [](cui_ctx* ui, cui_rect rect, void* user) {
            const auto& model = *static_cast<const GroupPaint*>(user);
            model.assets->PaintGroups(ui, rect, *model.groups);
        }, &modelForPaint, sizeof modelForPaint);
    }

    bool HasNotice(int id) const
    {
        const auto found = captions_.find(id);
        return ready_ && found != captions_.end() && found->second.texture;
    }

    void DrawCredentials(cui_ctx* ui, LoginClient& client)
    {
        serverPage_ = false;
        groupScrollDrag_ = false;
        groupExitPressed_ = false;
        pressedServer_ = -1;
        const double now = cui_time(ui);
        if (announcementTime_ >= 0)
            announcement_.advance(static_cast<std::uint32_t>((now - announcementTime_) * 1000 + 0.5));
        announcementTime_ = now;
        if (announcement_.rows.size() * 14 > 238) cui_request_wakeup(ui, 0.016f);

        const bool editable = client.stage == Stage::Login && client.status.empty() &&
                              !client.settingsOpen && !client.audioOpen && !client.accountForm;
        selectedCaption_ = {};
        if (client.stage == Stage::Login && !client.status.empty() && !client.settingsOpen && !client.audioOpen) {
            if (HasNotice(client.loginNoticeId)) selectedCaption_=captions_.at(client.loginNoticeId);
            else {
                SetBagText(ui,loginStatusCaption_,client.status,308,95,true,mir3::TextFont::dialog);
                selectedCaption_=loginStatusCaption_.image;
            }
        }
        if (noticeDismissals_ != client.noticeDismissals) {
            noticeDismissals_ = client.noticeDismissals;
            field_ = 1;
            selectField_ = true;
        }
        Canvas(ui);
        const auto viewport = cui_viewport_size(ui);
        const auto scale = Scale({0, 0, viewport.x, viewport.y});
        if (scale <= 0) return;
        const float ox = (viewport.x - 640 * scale) / 2;
        const float oy = (viewport.y - 480 * scale) / 2;
        const auto mouse = cui_mouse_pos(ui);
        const float x = (mouse.x - ox) / scale;
        const float y = (mouse.y - oy) / scale;
        const auto hit = [x, y](int left, int top, int w, int h) {
            return x >= left && x < left + w && y >= top && y < top + h;
        };
        const bool down = cui_mouse_clicked(ui, CUI_MOUSE_LEFT);
        const bool up = cui_mouse_released(ui, CUI_MOUSE_LEFT);
        int button = -1;
        for (int i = 0; i < static_cast<int>(buttons_.size()); ++i) {
            const auto& image = backgrounds_[2 + 2 * i];
            if (hit(buttons_[i][0], buttons_[i][1], image.width, image.height)) button = i;
        }
        hoverSignIn_ = button == 0;
        if (up || !editable) pressedButton_ = -1;
        else if (down) pressedButton_ = button;

        const bool enter = editable && cui_key_pressed(ui,CUI_KEY_ENTER);
        const bool tab = editable && cui_key_pressed(ui,CUI_KEY_TAB);
        const bool mouseSubmit = editable && up && button == 0;
        if (editable) {
            const int fieldHit = hit(128, 440, 99, 14) ? 0 : hit(326, 440, 99, 14) ? 1 : -1;
            if (down && fieldHit >= 0) { field_ = fieldHit; selectField_ = true; }
            if (up && button >= 0) {
                client.audio.PlayEffect(clickSound_.c_str(), client.audioOptions);
                if ((button == 1 || button == 2) && (client.legacy || !(button == 1 ? client.registration[0] : client.recovery[0]))) {
                    client.CloseAccountForm();
                    client.accountForm = button == 1 ? CM_ADDNEWUSER : CM_CHANGEPASSWORD;
                } else if (button > 0) {
                    const char* url = button == 1 ? client.registration : button == 2 ? client.recovery : exitUrl_.c_str();
                    const std::string_view address(url);
                    if (address.starts_with("http://") || address.starts_with("https://")) {
                        cui_app_open_url(url);
                        cui_app_quit(nullptr);
                    }
                }
            }
        }

        const bool showEditor = editable && !client.accountForm && client.stage == Stage::Login && client.status.empty();
        showAccount_ = !showEditor || field_ != 0;
        showPassword_ = !showEditor || field_ != 1;
        if (showEditor) {
            const char* id = field_ == 0 ? "login-account" : "login-password";
            const float left = field_ == 0 ? 128 : 326;
            cui_overlay_begin(ui, {ox + left * scale, oy + 440 * scale, 99 * scale, 14 * scale});
            if (selectField_ || down || enter || tab || mouseSubmit) cui_text_field_focus(ui, id, selectField_);
            selectField_ = false;
            auto options = ModalEditorOptions(14*scale,scale);
            options.keep_escape = false;
            options.password = field_ == 1;
            options.password_mask = '*';
            options.byte_limit = 24;
            options.measure_bytes = AnsiBytes;
            char* value = options.password ? client.password : client.account;
            cui_input_ex(ui, id, value, sizeof client.account, &options);
            cui_box_end(ui);
        }
        if (tab || enter) {
            field_ = 1-field_;
            selectField_ = true;
            cui_request_redraw(ui);
        }
        if (mouseSubmit || (enter && field_==0 && client.account[0] && client.password[0]))
            client.SignIn();
        const unsigned passwordLength = client.password[0] ? AnsiBytes(client.password, static_cast<int>(std::strlen(client.password)))
                                                          : client.noticePasswordLength;
        UpdateFieldText(ui, client.account, std::string(passwordLength, '*'));

        noticeVisible_ = selectedCaption_.texture != 0;
        if (NoticeInput(ui, noticeVisible_ && !client.audioOpen && !client.settingsOpen, x, y))
            client.DismissNotice();
    }

    void DrawServerList(cui_ctx* ui, LoginClient& client)
    {
        serverPage_ = true;
        UpdateServers(ui, client.servers);
        UpdateBilling(ui, client.billing);
        selectedCaption_ = billingImage_;
        noticeVisible_ = client.billing.message != 0;
        Canvas(ui);

        if (client.stage == Stage::Handoff) {
            handoffShade_ = static_cast<int>(std::lround(255.0 * client.HandoffRemainingMs() / 2000.0));
            cui_request_wakeup(ui, 0.016f);
            return;
        }
        handoffShade_ = -1;
        const auto viewport = cui_viewport_size(ui);
        const auto scale = Scale({0, 0, viewport.x, viewport.y});
        if (scale <= 0) return;
        const auto mouse = cui_mouse_pos(ui);
        const float x = (mouse.x - (viewport.x - 640 * scale) / 2) / scale;
        const float y = (mouse.y - (viewport.y - 480 * scale) / 2) / scale;
        hoveredServer_ = -1;
        if (x >= 20 && x < 20 + serverWidth_ && y >= 70) {
            const auto row = (y - 70) / serverRowHeight_;
            if (row < serverRows_.size()) hoveredServer_ = static_cast<int>(row);
        }
        if (noticeVisible_) {
            pressedServer_ = -1;
            if (NoticeInput(ui, true, x, y)) client.billing = {};
            return;
        }
        pressed_ = false;
        if (client.stage != Stage::Servers) return;
        if (cui_mouse_clicked(ui, CUI_MOUSE_LEFT)) pressedServer_ = hoveredServer_;
        if (cui_mouse_released(ui, CUI_MOUSE_LEFT)) {
            const auto selected = std::exchange(pressedServer_, -1);
            if (selected >= 0 && selected == hoveredServer_) client.Select(client.servers[selected]);
        }
    }

    void ResetCredentials()
    {
        field_ = 0;
        selectField_ = true;
        pressedButton_ = -1;
        announcementTime_ = -1;
        pressed_ = false;
        pressedServer_ = -1;
        handoffShade_ = -1;
    }

private:
    struct Image { cui_texture texture = 0; int width = 0, height = 0; };
    struct AnnouncementImage { Image image; int width, height; };
    struct ServerRow { std::string label; unsigned load; Image image; };
    struct GroupPaint { const Assets* assets; const ServerGroups* groups; };

    bool ready_ = false, pressed_ = false;
    bool serverPage_ = false, noticeVisible_ = false;
    std::array<Image, 10> backgrounds_{};
    std::array<Image, 3> dialog_{};
    std::array<Image, 3> legends_{};
    std::array<Image, 16> groupFrames_{};
    std::map<std::string, Image> groupLabels_, groupTitles_, groupLines_;
    cui_vec2 groupPointer_{-1, -1}, groupGlowOffset_{};
    bool groupsReady_ = false, groupScrollDrag_ = false, groupExitPressed_ = false;
    int firstGroupRow_ = 0;
    double uiGroupScrollTime_ = 0;
    static constexpr std::array<std::array<int, 2>, 4> buttons_{{{459, 436}, {139, 379}, {279, 379}, {439, 379}}};
    Image selectedCaption_, accountImage_, passwordImage_;
    std::map<int, Image> captions_;
    mir3::Messages messages_;
    std::string font_, account_, password_, exitUrl_;
    std::filesystem::path clickSound_;
    cui_font editorFont_ = CUI_FONT_BODY;
    std::map<int,Image> accountButtons_;

    int accountButtonPress_ = -1;

public:
    void DrawAccountForm(cui_ctx* ui, LoginClient& client)
    {
        const bool create=client.accountForm==CM_ADDNEWUSER;
        const auto viewport=cui_viewport_size(ui);
        const float scale=(std::min)(viewport.x/800,viewport.y/600);
        const float ox=(viewport.x-800*scale)/2,oy=(viewport.y-600*scale)/2;
        const auto bounds=[&](float x,float y,float w,float h) {
            return cui_rect{ox+x*scale,oy+y*scale,w*scale,h*scale};
        };
        struct Field { const char* label; char* value; int x,y,width,limit; bool password; };
        std::vector<Field> fields;
        fields.push_back({"è´¦å·",client.account,223,113,114,create?9:12,false});
        if (!create) fields.push_back({"åŽŸå¯†ç ",client.currentSecret,223,145,160,12,true});
        fields.push_back({create?"å¯†ç ":"æ–°å¯†ç ",client.newSecret,223,create?133:177,create?114:160,create?9:12,true});
        fields.push_back({"ç¡®è®¤å¯†ç ",client.confirmSecret,223,create?153:209,create?114:160,create?9:12,true});
        if (create) {
            constexpr std::array<const char*,14> labels{"å§“å","è¯ä»¶å·ç ","ç”Ÿæ—¥","é‚®ç¼–","",
                "åœ°å€","è¯¦ç»†åœ°å€","ç”µè¯","æ‰‹æœº","ç”µå­é‚®ç®±","å¯†ä¿é—®é¢˜ä¸€","å¯†ä¿ç­”æ¡ˆä¸€","å¯†ä¿é—®é¢˜äºŒ","å¯†ä¿ç­”æ¡ˆäºŒ"};
            constexpr std::array<int,14> ys{186,206,226,259,259,279,299,330,350,370,401,422,443,464};
            constexpr std::array<int,14> limits{19,14,10,7,7,20,20,13,14,39,19,11,19,11};
            for (size_t i=0;i<labels.size();++i)
                fields.push_back({labels[i],client.accountDetails[i].data(),i==4?288:223,ys[i],
                    i==3||i==4?47:160,limits[i],i==11||i==13});
        }
        SetBagText(ui,accountLabels_[0],create?"åˆ›å»ºè´¦å·":"ä¿®æ”¹å¯†ç ",580,20,true,mir3::TextFont::dialog);
        for(size_t i=0;i<fields.size();++i)
            SetBagText(ui,accountLabels_[i+1],fields[i].label,110,18);
        SetBagText(ui,accountLabels_[18],client.formError,230,100,false,mir3::TextFont::dialog);
        struct PaintField { int x,y,width; };
        struct Model {
            Assets* art; bool create; int pressed,count; float ox,oy,scale;
            std::array<PaintField,17> fields;
        };
        Model model{this,create,accountButtonPress_,int(fields.size()),ox,oy,scale,{}};
        for(size_t i=0;i<fields.size();++i) model.fields[i]={fields[i].x,fields[i].y,fields[i].width};
        cui_overlay_begin(ui,{0,0,viewport.x,viewport.y});
        cui_canvas(ui,"account-form-art",CUI_FILL,CUI_FILL,
            [](cui_ctx* ui,cui_rect,void* pointer) {
                const auto& m=*static_cast<Model*>(pointer);
                auto* a=m.art;
                const auto rect=[&](float x,float y,float w,float h) {
                    return cui_rect{m.ox+x*m.scale,m.oy+y*m.scale,w*m.scale,h*m.scale};
                };
                const auto image=[&](const Image& frame,float x,float y,float w,float h) {
                    if(frame.texture) cui_draw_image(ui,frame.texture,rect(x,y,w,h),{0,0,1,1},{},{255,255,255,255});
                };
                const auto& skin=a->dialog_[0];
                const float height=m.create?480.0f:270.0f;
                const auto strip=[&](float y,float h,float v,float vh) {
                    cui_draw_image(ui,skin.texture,rect(80,y,640,h),{0,v,1,vh},{},{255,255,255,255});
                };
                strip(61,40,0,0.32f);
                strip(101,height-80,0.32f,0.46f);
                strip(61+height-40,40,0.78f,0.22f);
                const auto text=[&](int index,int x,int y) {
                    const auto& t=a->accountLabels_[index].image;
                    image(t,x,y,t.width,t.height);
                };
                text(0,110,79);
                for(int i=0;i<m.count;++i) {
                    const auto f=m.fields[i];
                    text(i+1,108,f.y);
                    cui_draw_rect(ui,rect(f.x-2,f.y-1,f.width+4,18),{},{5,5,5,220});
                }
                text(18,458,m.create?173:145);
                for(int i=0;i<2;++i) {
                    int id=(i?1245:1241)+(m.pressed==i);

                    const auto& button=a->accountButtons_.at(id);
                    image(button,i?586:479,m.create?483:279,button.width,button.height);
                }
            },&model,sizeof model);
        cui_box_end(ui);
        for(size_t i=0;i<fields.size();++i) {
            const auto& field=fields[i];
            cui_push_id_int(ui,int(i));
            cui_overlay_begin(ui,bounds(field.x+2,field.y+2,field.width-4,14));
            auto options=ModalEditorOptions(14*scale,scale);
            options.password=field.password;
            options.password_mask='*';
            options.byte_limit=field.limit;
            options.measure_bytes=AnsiBytes;
            cui_input_ex(ui,"account-form-field",field.value,65,&options);
            cui_box_end(ui);
            cui_pop_id(ui);
        }
        const auto mouse=cui_mouse_pos(ui);
        int hit=-1;
        for(int i=0;i<2;++i) {
            int id=i?1245:1241;
            const auto& image=accountButtons_.at(id);
            const auto r=bounds(i?586:479,create?483:279,image.width,image.height);
            if(mouse.x>=r.x && mouse.x<r.x+r.w && mouse.y>=r.y && mouse.y<r.y+r.h) hit=i;
        }
        if(cui_mouse_clicked(ui,CUI_MOUSE_LEFT)) accountButtonPress_=hit;
        if(cui_mouse_released(ui,CUI_MOUSE_LEFT)) {
            const int pressed=accountButtonPress_;accountButtonPress_=-1;
            if(hit>=0 && pressed==hit) {
                if(hit==1) client.CloseAccountForm();
                else client.SubmitAccountForm();
            }
        }
        if(cui_key_pressed(ui,CUI_KEY_ESCAPE)) client.CloseAccountForm();
    }

private:
    cui_input_opts ModalEditorOptions(float height, float scale) const
    {
        cui_input_opts opts{};
        opts.bare = opts.keep_tab = opts.keep_escape = true;
        opts.width = CUI_FILL;
        opts.height = height;
        opts.font_size = 12*scale;
        opts.font = editorFont_;
        opts.text_color = {223,223,223,255};
        opts.caret_color = {243,243,243,255};
        opts.solid_caret = true;
        return opts;
    }
    int field_ = 0, pressedButton_ = -1;
    unsigned noticeDismissals_ = 0;
    bool selectField_ = true, showAccount_ = false, showPassword_ = true, hoverSignIn_ = false;
    double announcementTime_ = -1;
    mir3::LoginAnnouncement announcement_;
    std::vector<AnnouncementImage> announcementImages_;
    std::vector<ServerRow> serverRows_;
    int serverWidth_ = 90, serverRowHeight_ = 20;
    int hoveredServer_ = -1, pressedServer_ = -1, handoffShade_ = -1;
    BillingNotice billing_{};
    Image billingImage_;
    std::vector<cui_texture> textures_;

    struct HudPoint {
        float scale, x, y;
        explicit HudPoint(cui_rect r) : scale((std::min)(r.w / 800, r.h / 600)),
            x(r.x + (r.w - 800 * scale) / 2), y(r.y + (r.h - 600 * scale) / 2) {}
        cui_vec2 ToLocal(cui_vec2 point) const { return {(point.x - x) / scale, (point.y - y) / scale}; }
        cui_rect Rect(float left, float top, float width, float height) const {
            return {x + left*scale, y + top*scale, width*scale, height*scale};
        }
    };
    static bool Contains(cui_rect r, cui_vec2 p)
    {
        return p.x >= r.x && p.x < r.x + r.w && p.y >= r.y && p.y < r.y + r.h;
    }
    struct HudHint {
        int previous = -1, height = 6;
        bool expanding = true;
        int Advance(cui_ctx* ui, int caption, int fullHeight);
        static void Paint(cui_ctx* ui, const HudPoint& base, const Image& hint, int height, int anchorX, int anchorY);
    } hudHint_;
    Image MakeHudHint(cui_ctx* ui, int caption);

    struct QuestLine {
        const mir3_cui::QuestRecord* record = nullptr;
        int parent = -1;
        bool child = false;
    };
    struct QuestText {
        std::string text;
        Image image;
        int width = 0;
        cui_color color{};
    };
    static std::vector<QuestLine> QuestLines(const mir3_cui::QuestPanel& quests);
    void ToggleQuest(LoginClient& client);
    bool questReady_ = false, questFocused_ = false, questDragging_ = false, questScrollDrag_ = false;
    int questX_ = 0, questY_ = 0, questGrabX_ = 0, questGrabY_ = 0;
    int questFirst_ = 0, questLast_ = 0, questDescriptionFirst_ = 0;
    int questPressed_ = -1, questHint_ = -1, questHintDrawHeight_ = 0;
    cui_vec2 questPreviousPointer_{};
    Image questBackground_, questThumb_;
    std::array<std::array<Image, 2>, 2> questButtons_{};
    std::array<Image, 2> questHints_{};
    std::array<QuestText, 19> questText_{};
    std::vector<Image> questDescriptionRows_;
    std::string questDescription_;
    static constexpr std::array<std::array<int, 2>, 2> questButtonOffsets_{{{267,75},{267,107}}};
    static constexpr std::array<int, 2> questHintMessages_{{2200,1052}};
    void OpenQuests(cui_ctx* ui, mir3::WilArchive& archive);
    void PaintQuests(cui_ctx* ui, cui_rect rect) const;
    void RefreshQuestText(cui_ctx* ui, mir3_cui::QuestPanel& quests, const std::vector<QuestLine>& lines);
    int QuestThumbY() const { return questY_ + 160 + (questLast_ ? 182 * questFirst_ / questLast_ : 0); }

    struct GuildPosition {
        int x, y, width, height;
        int grabX = 0, grabY = 0;
        bool dragging = false;
        cui_rect Rect() const { return {float(x), float(y), float(width), float(height)}; }
        void Grab(cui_vec2 point);
        void Drag(cui_vec2 point);
    };
    bool guildReady_ = false, guildFocused_ = false, guildDrawPanel_ = false, guildDrawEditor_ = false;
    bool guildDrawPrompt_ = false, guildDrawAlert_ = false, guildDrawAnnouncement_ = false;
    bool guildCancelSelected_ = false, guildTabDown_ = false;
    GuildPosition guildPosition_{102,22,594,444}, guildEditorPosition_{107,110,584,252};
    mir3::GuildLayout guildLayout_;
    mir3::GuildScroll guildScroll_;
    cui_vec2 guildPointer_{};
    int guildThumbY_ = 0, guildCount_ = 0, guildPressed_ = -1, guildEditorId_ = -1;
    int guildHint_ = -1, guildHintHeight_ = 0, guildLineHeight_ = 15;
    unsigned guildRevision_ = 0;
    std::map<int, Image> guildFrames_;
    std::array<Image,9> guildHints_{};
    std::array<Image,3> guildTitles_{};
    std::array<Image,5> guildEditorLabels_{};
    std::array<QuestText,18> guildRows_{};
    Image guildName_, guildCaption_, guildError_;
    std::string guildNameText_, guildCaptionText_, guildErrorText_;
    void OpenGuild(cui_ctx* ui, mir3::WilArchive& archive);
    void RefreshGuildText(cui_ctx* ui, const mir3_cui::GuildPanel& guild);
    void GuildInput(cui_ctx* ui, LoginClient& client, cui_vec2 mouse, bool down, bool up, bool fallback);
    void GuildModalInput(cui_ctx* ui, LoginClient& client, cui_vec2 mouse, bool down, bool up);
    void PaintGuild(cui_ctx* ui, cui_rect rect, bool drawPanel, bool drawDialog) const;
    int GuildRowCount(const mir3_cui::GuildPanel& guild) const;
    bool GuildHit(cui_vec2 mouse, int id, int x, int y) const;

    struct NpcPart {
        std::string text;
        int x, line, width, height, option;
        unsigned color;
        Image normal, underline;
    };
    std::array<mir3::WilFrame,3> npcBackgroundParts_{};
    std::array<Image,13> npcBackgrounds_{};
    std::map<int,Image> npcButtons_;
    std::vector<NpcPart> npcParts_;
    bool npcReady_ = false, npcScrollEnabled_ = false, npcClosePressed_ = false, npcDownLit_ = false;
    bool npcPromptVisible_ = false, npcPromptCancel_ = false, npcTabDown_ = false, npcCloseHover_ = false;
    int npcHeight_ = 224, npcStrips_ = 1, npcFirst_ = 0, npcScrollLimit_ = 0;
    int npcArrowPressed_ = 0, npcArrowFrame_ = 0, npcOptionPressed_ = -1, npcHovered_ = -1;
    int npcPromptPressed_ = -1, npcEditor_ = -1, npcHintHeight_ = 0;
    cui_vec2 npcPointer_{};
    std::uint32_t npcActor_ = 0;
    Image npcCaption_, npcPromptError_, npcCloseHint_;
    std::string npcCaptionText_;
    bool npcPromptTooLong_ = false;
    void OpenNpc(cui_ctx* ui, mir3::WilArchive& archive);
    void LayoutNpc(cui_ctx* ui, const mir3_cui::NpcDialogue& npc);
    void BuildNpcBackground(cui_ctx* ui);
    void NpcInput(cui_ctx* ui, LoginClient& client, cui_vec2 point, bool down, bool up);
    void NpcPromptInput(cui_ctx* ui, LoginClient& client, cui_vec2 point, bool down, bool up);
    void PaintNpc(cui_ctx* ui, cui_rect rect) const;
    int NpcOptionAt(cui_vec2 point) const;
    bool NpcHit(cui_vec2 point, int id, int x, int y) const;

    struct ShopText {
        std::string name, price, requirement;
        Image label, amount, condition;
        int nameWidth = 0;
        bool qualified = false;
    };
    struct ShopVisual {
        unsigned icon = 0, variants = 0;
        bool selected = false, visible = false;
        ShopText text;
    };
    enum class ShopControl { None, Details, Close, OfferClose, Buy, Previous, Next, Bulk };
    mir3::WilArchive itemArchive_;
    std::map<unsigned,Image> itemFrames_;
    std::map<int,Image> shopFrames_;
    std::array<ShopVisual,4> shopGoods_{};
    std::array<ShopVisual,12> shopOffers_{};
    ShopVisual shopBulkVisual_;
    Image shopPageImage_, shopPageTotal_, shopCaption_;
    std::string shopPageText_, shopPageTotalText_, shopCaptionText_;
    bool shopReady_ = false, shopScrollDrag_ = false, shopBulkPrompt_ = false;
    bool shopCancel_ = false, shopTabDown_ = false, shopInputFocus_ = false, shopNotice_ = false;
    bool shopShowsOffers_ = false, shopShowsBulk_ = false, shopDrawThumb_ = false, shopStorage_ = false;
    bool shopDrawQuantity_ = false;
    std::uint32_t shopActor_ = 0;
    unsigned shopCatalogue_ = 0;
    int shopFirst_ = 0, shopPressedRow_ = -1, shopLastRow_ = -1, shopModalPress_ = -1;
    int shopEditGeneration_ = 0;
    double shopFraction_ = 0, shopLastDown_ = -1;
    ShopControl shopPressed_ = ShopControl::None;
    cui_vec2 shopPointer_{};
    static constexpr int shopTop_ = 225;
    static int ShopWidth(const mir3_cui::NpcShop& shop) { return shop.bulk ? 516 : shop.offersVisible ? 488 : 292; }
    static cui_rect ShopOfferCell(int row, bool storage) {
        return storage ? cui_rect{float(22+38*(row%4)),float(266+38*(row/4)),37,37}
                       : cui_rect{float(315+38*(row%4)),float(shopTop_+42+38*(row/4)),35,35};
    }
    void OpenShop(cui_ctx* ui, mir3::WilArchive& archive, const std::filesystem::path& items);
    void ShopInput(cui_ctx* ui, LoginClient& client, cui_vec2 pointer);
    void ShopModalInput(cui_ctx* ui, LoginClient& client, cui_vec2 pointer);
    void FinishShopQuantity(LoginClient& client, bool confirm);
    void RefreshShop(cui_ctx* ui, const LoginClient& client);
    void PaintShop(cui_ctx* ui, cui_rect rect) const;
    void CacheItem(cui_ctx* ui, unsigned id);
    void ShopGoodText(cui_ctx* ui, ShopText& target, const mir3_cui::NpcShop::Good& good,
                      const LoginClient& client, bool bulk);
    void ShopAction(LoginClient& client, ShopControl control);
    bool ShopHit(cui_vec2 pointer, int id, int x, int y) const;

    struct BagText { std::string value; Image image; };
    std::array<BagText,19> accountLabels_;
    BagText loginStatusCaption_;
    struct BagSprite {
        Image icon, count;
        int x, y, labelX, labelY;
    };
    std::map<int, Image> inventoryFrames_;
    std::array<Image,4> inventoryTitles_;
    std::array<BagText,mir3_cui::InventoryPanel::Capacity> inventoryCounts_;
    std::vector<BagSprite> inventorySprites_;
    BagText inventoryWeight_, inventoryGold_, inventoryPrice_, inventoryCaption_;
    Image inventoryHeld_;
    cui_rect inventoryFootprint_{};
    cui_vec2 inventoryPointer_{-1,-1};
    int inventoryX_ = 518, inventoryY_ = 0, inventoryRow_ = 0;
    int inventoryDragX_ = 0, inventoryDragY_ = 0, inventoryButton_ = 0;
    int inventoryModalPress_ = -1, inventoryEditor_ = -1;
    unsigned inventoryServiceRevision_ = 0;
    bool inventoryReady_ = false, inventoryDragging_ = false, inventoryScrollDrag_ = false;
    bool inventoryClosePress_ = false, inventoryServicePress_ = false, inventoryFit_ = false;
    bool inventoryShow_ = false, inventoryQuantity_ = false, inventoryAlert_ = false;
    bool inventoryCancel_ = false, inventoryTabDown_ = false;
    static constexpr int inventoryWidth_ = 282, inventoryHeight_ = 466;
    struct HudChatRow { BagText text; unsigned ink = 0, backing = 0; int width = 0, height = 0; };
    struct WorldFrame { Image image; int x = 0, y = 0; std::uint64_t used = 0; };
    std::filesystem::path mapData_;
    std::vector<std::uint8_t> mapWeaponOrder_;
    std::string mapGraphicsName_;
    using WorldFrameKey = std::pair<std::string,int>;
    std::map<std::string,mir3::WilArchive> worldArchives_;
    std::map<WorldFrameKey,WorldFrame> worldFrames_;
    std::vector<SceneSprite> worldSprites_;
    std::vector<SceneActorHit> worldNpcs_, worldTargets_, worldCorpses_;
    std::vector<GroundLabel> worldGroundLabels_;
    std::vector<SceneActorAnchor> worldAnchors_;
    std::vector<SceneCaption> worldNames_, worldSpeech_;
    std::vector<SceneEffectQuad> worldWeather_, worldLevelEffects_, worldSpellEffects_;
    std::vector<mir3::SceneLight> worldSpellLights_;
    std::map<std::pair<std::string,int>,Image> worldText_;
    mir3::SceneLightGrid worldLight_{255};
    mir3_cui::MapTerrain::LightView mapLights_{};
    std::string lightMapName_;
    std::uint64_t lightMapRevision_ = 0;
    int lightLeft_ = 0, lightTop_ = 0;
    struct GroundName { std::string name; Image image; };
    std::array<GroundName,mir3::GroundItems::cell_capacity> groundNames_;
    mir3_cui::MapTerrain::GraphicsView mapGraphics_;
    std::uint64_t worldFrame_ = 0, mapGraphicsRevision_ = 0;
    std::uint64_t worldTextureBytes_ = 0;
    int mapGraphicsX_ = 0, mapGraphicsY_ = 0;
    const WorldFrame& GetWorldFrame(cui_ctx* ui, const std::string& library, int frame,
        std::uint16_t status=65535, bool highlighted=false,std::uint16_t dye=65535);
    const WorldFrame& GetProjectedWorldShadow(cui_ctx* ui, const std::string& library, int frame, bool flat, bool weapon=false);
    SceneMapArt PrepareMapArt(cui_ctx* ui, LoginClient& client, cui_vec2 pointer, bool interactive);
    void PrepareWorldText(cui_ctx* ui, const LoginClient& client, cui_vec2 pointer, bool interactive);
    void PrepareSceneLight(LoginClient& client, SceneMapArt& camera);
    void PrepareWeather(cui_ctx* ui, const LoginClient& client, SceneMapArt& camera);
    void PrepareLevelEffects(cui_ctx* ui, const LocalScene& scene, SceneMapArt& camera);
    void PrepareSpellEffects(cui_ctx* ui, LocalScene& scene, SceneMapArt& camera);
    void PrepareCast(cui_ctx* ui, const mir3::CastVisual& cast, int x, int y, int row);
    void PrepareGroundItems(cui_ctx* ui, const LocalScene& scene, const SceneMapArt& camera,
                            int startX, int startY, cui_vec2 pointer, bool interactive, std::vector<SceneSprite>& overlays);
    const WorldFrame& GroundFrame(cui_ctx* ui, int looks, bool highlighted);
    cui_rect AddWorldActor(cui_ctx* ui, const mir3_cui::CharacterAppearance& actor, int facing, int x, int y,
        const SceneMapArt& camera, int pose, bool shadowBlend, int mount = 0, int appearanceWeight=-1,
        bool highlighted=false, std::uint16_t status=65535);
    void TrimWorldFrames(cui_ctx* ui);
    std::array<HudChatRow,14> gameChatRows_;
    std::array<BagText,4> gameHudText_;
    struct ChatShortcutLabel { std::string source; BagText text; };
    std::array<std::array<ChatShortcutLabel,10>,2> chatShortcutLabels_{};
    std::array<bool,2> chatShortcutVisible_{};
    std::map<int,Image> gameHudFrames_, gamePortraits_, gameHudHints_;
    mir3::HudModel gameHud_;
    HudHint gameButtonHint_;
    BagText gameMeterTip_;
    mir3::HudLayout::Rect gameMeterTipBounds_{};
    int gameRowsDrawn_ = 0, gameThumbY_ = 0, gameMeterTipKind_ = -1;
    bool gameHudReady_ = false, gameChatModeChanged_ = false;
    bool gameChatFocused_ = false, gameScrollCaptured_ = false;
    std::array<bool,12> castKeysHeld_{};
    int gameChatEditor_ = -1, gameHintDrawn_ = -1, gameHintHeight_ = 0;
    int gameChannel_ = 0;
    std::array<bool,3> gameChatRejected_{};
    mir3::WilArchive minimapArchive_, fullMinimapArchive_;
    std::array<Image,7> minimapFrames_{};
    std::array<Image,4> minimapHints_{};
    Image minimapImage_;
    WORD minimapImageNumber_ = 0;
    bool pointerCoversPanel_ = false, portraitMapMode_ = false, portraitMapReady_ = false;
    bool showMinimapActors_ = true, minimapPlayerMark_ = false;
    double portraitMapBlinkMs_ = 0;
    int minimapHint_ = -1, minimapHintHeight_ = 0;
    HudHint minimapTooltip_;
    const mir3::Minimap* minimapDrawing_ = nullptr;
    void PaintMinimap(cui_ctx* ui, cui_rect rect) const;
    void PaintPortraitMinimap(cui_ctx* ui, const HudPoint& base) const;
    static constexpr std::array<int,10> optionsFrameIds_{1290,1291,1292,1293,1294,1295,1296,1297,1221,1222};
    static constexpr std::array<mir3::HudLayout::Point,6> localOptionPositions_{{
        {186,55},{186,85},{186,115},{186,145},{430,235},{430,295}
    }};
    static constexpr std::array<int,6> localOptionButtons_{7,8,9,10,4,20};
    static constexpr std::array<mir3::HudLayout::Point,7> serverOptionPositions_{{
        {186,175},{186,205},{186,235},{186,265},{186,295},{186,325},{430,55}
    }};
    static constexpr std::array<int,7> serverOptionButtons_{11,12,13,14,16,17,18};
    struct OptionsCaption { int message; mir3::HudLayout::Rect bounds; Image image; };
    std::array<OptionsCaption,17> optionsCaptions_{{
        {2104,{60,30,390,15}}, {2100,{266,89,156,21}}, {2101,{266,149,156,21}}, {2102,{266,209,156,21}},
        {2105,{20,62,155,15}}, {2106,{20,92,155,15}}, {2107,{20,122,155,15}}, {2108,{20,152,155,15}},
        {2109,{20,182,155,15}}, {2110,{20,212,155,15}}, {2111,{20,242,155,15}}, {2112,{20,272,155,15}},
        {2113,{20,302,155,15}}, {2114,{20,332,155,15}}, {2115,{266,59,156,21}}, {2103,{266,239,156,21}},
        {2130,{266,300,156,20}}
    }};
    static constexpr mir3::HudLayout::Rect initialOptionsBounds_{145,30,510,468};
    mir3::HudLayout::Rect optionsBounds_ = initialOptionsBounds_;
    std::array<Image,optionsFrameIds_.size()> optionsFrames_{};
    std::array<unsigned char,21> optionButtonStates_{};
    BagText optionsAttackMode_;
    cui_vec2 optionsPointer_{-1,-1};
    mir3::HudLayout::Point optionsGrab_{};
    std::uint32_t optionsConfirmed_ = 0;
    int optionsAudioDrag_ = -1;
    bool optionsDragging_ = false;
    const Image& OptionFrame(int id) const;
    void OpenGameOptions(cui_ctx* ui, mir3::WilArchive& archive);
    void PaintGameOptions(cui_ctx* ui, cui_rect rect, const LoginClient& client) const;
    void OpenSkills(cui_ctx* ui, mir3::WilArchive& archive, const std::filesystem::path& root);
    void PaintSkills(cui_ctx* ui, cui_rect rect, const LoginClient& client) const;
    void PaintSkillShortcuts(cui_ctx* ui, cui_rect rect, const LoginClient& client) const;
    const mir3::SkillPosition* SkillAt(cui_vec2 point, const mir3::SkillPanel& skills) const;
    mir3::WilArchive skillIconArchive_;
    mir3::SkillDescriptions skillDescriptions_;
    std::map<int,Image> skillFrames_, skillIcons_;
    std::array<BagText,15> skillLevels_{};
    std::array<BagText,3> skillSelected_{};
    struct SkillDescriptionLine { mir3::DescriptionRow row; Image image; };
    std::map<int,std::vector<SkillDescriptionLine>> skillDescriptionTextures_;
    BagText skillDescriptionProgress_;
    cui_rect skillMenu_{};
    cui_vec2 skillPointer_{-1,-1};
    int skillDescriptionId_ = 0;
    enum class UtilityPanel { Party, Guild, Quest, Skills, Status, Inventory, Options, Portrait, Mount, Trade, Quick };
    static constexpr std::array<UtilityPanel,11> initialUtilityOrder_{
        UtilityPanel::Party,UtilityPanel::Guild,UtilityPanel::Quest,UtilityPanel::Skills,UtilityPanel::Status,
        UtilityPanel::Inventory,UtilityPanel::Options,UtilityPanel::Portrait,
        UtilityPanel::Mount,UtilityPanel::Trade,UtilityPanel::Quick
    };
    std::array<UtilityPanel,11> utilityOrder_ = initialUtilityOrder_;
    bool UtilityVisible(UtilityPanel panel, const LoginClient& client) const;
    cui_rect UtilityBounds(UtilityPanel panel,const LoginClient& client) const;
    void ScrollUtility(cui_ctx* ui,LoginClient& client,bool blocked);
    void ScrollQuest(mir3_cui::QuestPanel& quests,cui_vec2 point,int wheel);
    void RaiseUtility(UtilityPanel panel);
    void ToggleParty(LoginClient& client);
    void ToggleQuick(LoginClient& client);
    bool HandleQuickInput(cui_ctx* ui, LoginClient& client, bool blocked);
    bool utilityQuickOpen_ = false;
    void PrepareInventoryPlacement(LoginClient& client);
    bool HandleInventoryUtility(cui_ctx* ui, LoginClient& client, bool blocked);
    bool utilityInventoryOpen_ = false;
    void OpenStatus(cui_ctx* ui, mir3::WilArchive& archive, const std::filesystem::path& data);
    void ToggleStatus(LoginClient& client);
    bool HandleStatusInput(cui_ctx* ui, LoginClient& client, bool blocked);
    void DrawStatusPanel(cui_ctx* ui, LoginClient& client);
    void PaintStatus(cui_ctx* ui, cui_rect rect) const;
    struct StatusLabel {
        std::array<int,4> rect{};
        BagText text;
        cui_color color{};
        WorldFrame icon;
    };
    mir3::DraggablePanel statusPanel_{{0,0,328,466}};
    mir3::WilArchive statusEquipArchive_;
    std::map<unsigned,WorldFrame> statusEquipFrames_;
    std::array<WorldFrame,2> statusBodies_{};
    std::array<WorldFrame,mir3_cui::EquipmentSlots> statusEquipped_{};
    WorldFrame statusBody_;
    std::array<int,2> statusPreviewOrigin_{};
    int statusPreview_ = -1;
    Image statusSkin_, statusCloseHint_;
    std::array<Image,5> independentSkins_;
    std::map<int,Image> independentButtons_;
    std::array<Image,3> mailModeSkins_;
    std::array<Image,2> statusClose_{}, statusGlows_{}, statusGlowShadows_{};
    std::array<std::array<Image,7>,2> statusElements_{};
    std::array<StatusLabel,54> statusLabels_{};
    std::array<StatusLabel,9> statusStatTips_{};
    std::array<StatusLabel,7> statusElementTips_{};
    const StatusLabel* statusTip_ = nullptr;
    HudHint statusHint_;
    cui_vec2 statusPointer_{-1,-1};
    int statusHintHeight_ = 0;
    bool statusClosePressed_ = false;
    void OpenPortraitPicker(cui_ctx* ui, mir3::WilArchive& archive, const std::filesystem::path& root);
    void TogglePortraitPicker(cui_ctx* ui);
    void RefreshPortraitCache(cui_ctx* ui, const LoginClient& client);
    bool HandlePortraitEntry(cui_ctx* ui, LoginClient& client, cui_vec2 pointer, bool blocked);
    bool HandlePortraitPicker(cui_ctx* ui, LoginClient& client, bool blocked);
    void DrawPortraitPicker(cui_ctx* ui);
    void PaintPortraitPicker(cui_ctx* ui, cui_rect rect) const;
    Image DecodePortrait(cui_ctx* ui, std::string_view bytes);
    int ReadSelectedPortrait(std::string& bytes) const;
    void ConfirmPortrait(cui_ctx* ui, LoginClient& client, int button, std::uint32_t tick);
    mir3::PortraitPicker portraitPicker_;
    std::filesystem::path portraitDirectory_, portraitCachePath_;
    std::map<int,Image> portraitPickerFrames_;
    std::array<Image,4> portraitHints_{};
    std::array<Image,mir3::PortraitPicker::visible_rows> portraitInstructions_{};
    std::array<BagText,mir3::PortraitPicker::visible_rows> portraitFileLabels_{};
    BagText portraitNoticeText_;
    Image portraitPreview_, portraitEquipped_;
    Image targetPortrait_, targetCustomPortrait_;
    mir3::WilArchive monsterPortraitArchive_;
    std::map<int,Image> monsterPortraits_;
    std::array<BagText,3> targetVitalsText_{};
    std::array<BagText,7> targetModifierText_{};
    mir3::TargetAttributes targetAttributes_;
    int targetBadge_=-1, targetHealthWidth_=0;
    bool targetMonster_=false, targetInformation_=false;
    std::uint32_t lastPortraitActor_=0;
    std::string targetPortraitBytes_;
    std::uint32_t targetActor_ = 0;
    std::uint64_t targetChangedAt_ = 0;
    void RefreshTargetPortrait(cui_ctx* ui, LoginClient& client, const SceneMapArt& scene, cui_vec2 pointer, bool interactive);
    static std::uint32_t SceneTargetAt(const SceneMapArt& scene, cui_vec2 pointer);
    void RefreshMonsterPortrait(cui_ctx* ui, const LocalScene::Player& actor);
    void PaintTargetDetails(cui_ctx* ui, const HudPoint& base, bool attributes) const;
    HudHint portraitHint_;
    int portraitPressed_ = -1, portraitHovered_ = -1, portraitHintHeight_ = 0, portraitEntryHintHeight_ = 0;
    int portraitNotice_ = 0;
    bool portraitPickerVisible_ = false, portraitEntryVisible_ = false, portraitEntryPressed_ = false;
    bool portraitNoticePressed_ = false;
    void ToggleOptions(LoginClient& client);
    void ToggleMount(LoginClient& client);
    void OpenMount(cui_ctx* ui, mir3::WilArchive& archive);
    bool HandleMountInput(cui_ctx* ui, LoginClient& client, bool blocked);
    void DrawMountPanel(cui_ctx* ui, const LoginClient& client);
    void PaintMount(cui_ctx* ui, cui_rect rect, const LoginClient& client) const;
    Image mountSkin_;
    std::array<std::array<Image,2>,mir3::MountPanel::buttons.size()> mountButtons_{};
    std::array<Image,mir3::MountPanel::buttons.size()> mountHints_{};
    HudHint mountHint_;
    int mountHintButton_ = -1, mountHintHeight_ = 0;
    void OpenTrade(cui_ctx* ui, mir3::WilArchive& archive);
    bool HandleTradeInput(cui_ctx* ui, LoginClient& client, bool blocked);
    void DrawTradePanel(cui_ctx* ui, const LoginClient& client);
    void PaintTrade(cui_ctx* ui, cui_rect rect, const LoginClient& client) const;
    Image tradeSkin_, tradeThumb_;
    std::array<Image,3> tradeConfirm_{};
    std::array<Image,2> tradeHints_{};
    std::array<BagText,2> tradeNames_{}, tradeAmounts_{};
    std::array<std::array<BagText,mir3_cui::TradePanel::Capacity>,2> tradeCounts_{};
    BagText tradePromptText_;
    HudHint tradeHint_;
    cui_vec2 tradePointer_{-1,-1};
    int tradeHintHeight_ = 0, tradePromptPressed_ = -1;
    unsigned tradeRevision_ = 0, tradePromptEditor_ = 0;
    bool tradePromptCancel_ = false, tradePromptTab_ = false;
    void OpenGameHud(cui_ctx* ui, mir3::WilArchive& archive, const std::filesystem::path& portraits);
    bool CanExitGame(LoginClient& client) const;
    void OpenGameExit(LoginClient& client, bool quit);
    BagText gameExitText_;
    int gameExitPressed_ = -1;
    bool gameExitCancel_ = false;
    void RefreshGameHud(cui_ctx* ui, const LoginClient& client);
    void PaintGameHud(cui_ctx* ui, cui_rect rect) const;
    void PrepareChatShortcuts(cui_ctx* ui, mir3_cui::ChatPanel& chat, bool editing);
    void GameHudInput(cui_ctx* ui, LoginClient& client, cui_vec2 pointer, bool pointerCovered, bool modal);
    void SetBagText(cui_ctx* ui, BagText& text, std::string value, int width, int height,
                    bool centered = false, mir3::TextFont font = mir3::TextFont::normal, bool inverted = false);
    void InventoryInput(cui_ctx* ui, LoginClient& client, cui_vec2 pointer);
    void InventoryModalInput(cui_ctx* ui, LoginClient& client, cui_vec2 pointer);
    void FinishInventoryQuantity(LoginClient& client, bool confirm);
    void RefreshInventory(cui_ctx* ui, LoginClient& client, cui_vec2 pointer);
    void PaintInventory(cui_ctx* ui, cui_rect rect) const;
    std::array<Image,3> quickFrames_;
    std::array<Image,6> quickIcons_;
    std::array<BagText,6> quickCounts_;
    std::array<int,6> quickCountOffsets_{};
    int quickX_ = 73, quickY_ = 413, quickDragX_ = 0, quickDragY_ = 0, quickLastSlot_ = -1;
    double quickLastClick_ = -1;
    bool quickDragging_ = false, quickClosePress_ = false, quickSkipRelease_ = false;
    void QuickInput(cui_ctx* ui, LoginClient& client, cui_vec2 pointer);
    void PaintQuick(cui_ctx* ui, cui_rect rect) const;
    static int InventoryButton(const mir3_cui::MerchantService& service) {
        using Kind = mir3_cui::MerchantService::Kind;
        if (!service.actor) return 0;
        return service.kind == Kind::Deposit ? 1231 : service.kind == Kind::Sell ? 1229 : 1227;
    }

    bool partyReady_ = false, partyFocused_ = false, partyDragging_ = false;
    bool partyDrawPanel_ = false, partyDrawModal_ = false, partyDrawAlert_ = false, partyDrawAllowed_ = false;
    int partyX_ = 272, partyY_ = 123, partyDragX_ = 0, partyDragY_ = 0;
    int partyPressed_ = -1, partyConfirmPressed_ = -1, partyModalKey_ = 0, partyEditor_ = -1;
    int partyHint_ = -1, partyHintDrawHeight_ = 0;
    std::map<int, Image> partyFrames_;
    std::array<Image, 2> partyAllowCaptions_{};
    std::array<Image, 4> partyHints_{};
    std::vector<std::string> partyRoster_;
    std::vector<Image> partyRosterText_;
    std::string partyPlayerName_, partyErrorText_;
    Image partyPlayer_, partyCaption_, partyErrorImage_;

    void OpenParty(cui_ctx* ui, mir3::WilArchive& archive);
    void PaintParty(cui_ctx* ui, cui_rect rect, bool drawPanel, bool drawDialog) const;
    void RefreshPartyText(cui_ctx* ui, const LoginClient& client);
    static constexpr std::array<int, 5> partyButtonFrames_{1221,1362,1365,1368,1370};
    static constexpr std::array<std::array<int, 2>, 5> partyButtonOffsets_{{{217,200},{38,192},{98,192},{158,192},{23,47}}};

    enum class MoviePurpose { None, Opening, Create, Return, Game };
    std::filesystem::path movieDirectory_;
    std::unique_ptr<mir3::LegacyMovie> movie_;
    Image movieSurface_;
    MoviePurpose moviePurpose_ = MoviePurpose::None;
    std::optional<Stage> moviePreviousStage_;
    bool movieWasCreating_ = false, movieFinished_ = false;
    double movieClock_ = 0;

    void StartTransition(cui_ctx* ui, LoginClient& client, MoviePurpose purpose, const wchar_t* filename);
    void StopTransition(cui_ctx* ui, LoginClient& client);
    void PaintTransition(cui_ctx* ui, cui_rect rect) const;

    mir3::WilArchive selectionArchive_;
    std::map<int, std::optional<mir3::WilFrame>> selectionSources_;
    std::map<int, Image> selectionButtons_;
    std::vector<Character> selectionCharacters_;
    std::array<mir3::SelectionRole, 4> selectionRoles_{};
    std::array<cui_rect, 4> selectionHits_{};
    std::array<cui_rect, 2> creationHits_{};
    std::array<Image, 3> selectionStats_{};
    std::array<Image, 5> creationHints_{};
    mir3::CreateCharacterPage creation_;
    Image selectionSurface_, selectionCaption_, selectionConnectingText_, deleteText_;
    std::optional<Character> selectionStatRole_;
    std::string selectionCaptionFallback_;
    std::vector<std::uint8_t> creationBackdrop_;
    int creationBackdropKey_ = -1;
    bool selectionReady_ = false, creationReady_ = false;
    bool selectionWaiting_ = false, selectionQuerying_ = false, selectionDirty_ = true, selectionNotice_ = false;
    unsigned selectionRevision_ = 0;
    int selectionPage_ = 0, selectionIndex_ = -1, selectionHover_ = -1, selectionPress_ = -1;
    int selectionCaptionId_ = 0, selectionStatWidth_ = 0, deletePress_ = -1, selectionEditor_ = -1;
    CharacterDialog selectionMode_ = CharacterDialog::None;
    cui_vec2 selectionMouse_{};
    double selectionTick_ = -1, selectionQueryAt_ = -1;

    void OpenSelection(cui_ctx* ui, mir3::WilArchive& gameInterface);
    const mir3::WilFrame* SelectionSource(int index);
    void SyncSelection(const LoginClient& client, bool entering);
    void PaintSelection(cui_ctx* ui, cui_rect rect) const;
    void RefreshSelectionSurface(cui_ctx* ui);
    void RefreshSelectionText(cui_ctx* ui, const LoginClient& client);
    const std::vector<std::uint8_t>& CreationBackdrop();
    void DrawSelectionEditor(cui_ctx* ui, LoginClient& client, float scale, float ox, float oy, bool focus);
    void SubmitCharacterCreation(LoginClient& client);
    void SelectionInput(cui_ctx* ui, LoginClient& client);
    void AdvanceSelectionRole(LoginClient& client, mir3::SelectionRole& role, cui_rect& hit,
                              std::uint32_t ms, bool creating, bool selected);
    bool SelectionHit(float x, float y, float w, float h) const;
    bool SelectionButtonHit(int index, float x, float y) const;
    void SetSelectedCharacter(LoginClient& client, int index, bool changePage);
    static constexpr std::array<cui_vec2, 4> selectionOrigins_{{{440,93}, {79,243}, {259,49}, {28,438}}};
    static constexpr std::array<cui_vec2, 5> creationOrigins_{{{266,419}, {308,419}, {352,419}, {450,444}, {491,444}}};
    static constexpr int creationFrames_[5][3] = {{91,92,93}, {94,95,96}, {97,98,98}, {85,86,87}, {88,89,90}};

    std::string Message(int id) const { return ConvertText(messages_.get(id), 936, CP_UTF8); }

    void OpenGroups(cui_ctx* ui, const std::filesystem::path& path)
    {
        mir3::WilArchive archive;
        if (!archive.open(path)) return;
        for (const int id : {0, 1, 3, 4, 7, 8, 11, 12, 13, 15}) {
            const auto frame = archive.frame(id);
            if (frame) {
                groupFrames_[id] = Upload(ui, frame->width, frame->height, frame->pixels.rgba);
                if (id == 1) groupGlowOffset_ = {static_cast<float>(frame->offset_x), static_cast<float>(frame->offset_y)};
            }
            if (id != 1 && !groupFrames_[id].texture) return;
        }
        groupsReady_ = true;
    }

    void CacheGroupText(cui_ctx* ui, std::map<std::string, Image>& cache, const std::string& value,
                        int width, int height, bool centered)
    {
        if (!cache.contains(value)) cache.emplace(value, Raster(ui, value, width, height, centered, mir3::TextFont::normal));
    }

    bool GroupHit(float x, float y, float w, float h) const
    {
        return groupPointer_.x >= x && groupPointer_.x < x + w &&
               groupPointer_.y >= y && groupPointer_.y < y + h;
    }

    float GroupScrollRatio(int count) const
    {
        return count > 1 ? static_cast<float>(firstGroupRow_) / (count - 1) : 0;
    }

    int GroupThumbY(int count) const { return 84 + static_cast<int>(447 * GroupScrollRatio(count)); }

    static bool IsGroupLineAvailable(const GroupLine& line)
    {
        return !line.loginSection.empty() && IniKey(line.loginSection) != "empty";
    }

    void PaintGroups(cui_ctx* ui, cui_rect rect, const ServerGroups& model) const
    {
        const float scale = (std::min)(rect.w / 800, rect.h / 600);
        const float ox = rect.x + (rect.w - 800 * scale) / 2;
        const float oy = rect.y + (rect.h - 600 * scale) / 2;
        const auto bounds = [&](float x, float y, float w, float h) {
            return cui_rect{ox + x * scale, oy + y * scale, w * scale, h * scale};
        };
        const cui_color white{255, 255, 255, 255};
        const cui_color titleInk{250, 250, 250, 255};
        const auto image = [&](const Image& frame, float x, float y, cui_color color) {
            if (frame.texture) cui_draw_image(ui, frame.texture, bounds(x, y, frame.width, frame.height), {0, 0, 1, 1}, {}, color);
        };
        cui_draw_rect(ui, rect, {}, {0, 0, 0, 255});
        cui_push_clip(ui, bounds(0, 0, 800, 600));
        image(groupFrames_[0], 0, 0, white);
        image(groupFrames_[groupExitPressed_ ? 4 : 3], 760, 13, white);
        for (int i = 0; i < static_cast<int>(model.groups.size()); ++i) {
            const auto& group = model.groups[i];
            const float x = static_cast<float>(group.x), y = static_cast<float>(group.y);
            const auto& label = groupLabels_.at(group.label);
            for (const auto& offset : std::array<cui_vec2, 4>{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}})
                image(label, x + 5 + offset.x, y + 25 + offset.y, {0, 0, 0, 255});
            image(label, x + 5, y + 25, {240, 240, 240, 255});
            const bool selected = model.selected == i;
            const int marker = selected ? 13 : GroupHit(x, y, 33, 21) ? 12 : 11;
            image(groupFrames_[marker], x - (selected ? 3 : 0), y - (selected ? 3 : 0), white);
        }
        const auto* group = model.selected < 0 ? nullptr : &model.groups[model.selected];
        if (group) {
            image(groupTitles_.at(group->title), 28, 20, titleInk);
            for (const auto& line : group->lines) {
                const int row = static_cast<int>(line.row) - firstGroupRow_;
                if (row < 0 || row >= 20) continue;
                const int y = 70 + 25 * row;
                image(groupFrames_[GroupHit(10, y, 190, 15) ? 8 : 7], 17, y - 5, white);
                if (const auto text = groupLines_.find(line.caption); text != groupLines_.end())
                    image(text->second, 10, y, titleInk);
            }
        }
        image(groupFrames_[15], 204, group ? GroupThumbY(group->rowCount) : 84, white);
        if (group && groupFrames_[1].texture) {
            const auto& glow = groupFrames_[1];
            cui_draw_image_additive(ui, glow.texture,
                bounds(static_cast<float>(group->x) - 55 + groupGlowOffset_.x,
                       static_cast<float>(group->y) - 120 + groupGlowOffset_.y, glow.width, glow.height));
        }
        cui_pop_clip(ui);
    }

    void Canvas(cui_ctx* ui)
    {
        auto* self = this;
        cui_canvas(ui, "login-art", CUI_FILL, CUI_FILL,
            [](cui_ctx* ui, cui_rect rect, void* model) {
                (*static_cast<Assets**>(model))->Paint(ui, rect);
            }, &self, sizeof self);
    }

    bool NoticeInput(cui_ctx* ui, bool enabled, float x, float y)
    {
        if (!enabled) { pressed_ = false; return false; }
        const bool hit = x >= 277 && x < 277 + dialog_[1].width && y >= 298 && y < 298 + dialog_[1].height;
        const bool up = cui_mouse_released(ui, CUI_MOUSE_LEFT);
        if (cui_mouse_clicked(ui, CUI_MOUSE_LEFT)) pressed_ = hit;
        if (up) pressed_ = false;
        return (up && hit) || cui_key_pressed(ui, CUI_KEY_ENTER) || cui_key_pressed(ui, CUI_KEY_SPACE);
    }

    void UpdateServers(cui_ctx* ui, const std::vector<Server>& servers)
    {
        if (serverRows_.size() == servers.size() &&
            std::equal(servers.begin(), servers.end(), serverRows_.begin(), [](const Server& server, const ServerRow& row) {
                return server.label == row.label && server.load == row.load;
            })) return;

        for (auto& row : serverRows_) Destroy(ui, row.image);
        serverRows_.clear();
        pressedServer_ = -1;
        int maxWidth = 0, maxHeight = 0;
        for (const auto& server : servers) {
            const auto extent = mir3::raster_text(server.label, 600, 30, false, mir3::TextFont::normal);
            maxWidth = (std::max)(maxWidth, extent.text_width);
            maxHeight = (std::max)(maxHeight, extent.text_height);
            const auto caption = server.load >= 1 && server.load <= 3
                ? server.label + " (" + Message(125 + static_cast<int>(server.load)) + ')' : server.label;
            serverRows_.push_back({server.label, server.load, Raster(ui, caption, 600, 20, false, mir3::TextFont::normal)});
        }
        serverWidth_ = maxWidth + 90;
        serverRowHeight_ = maxHeight + 20;
    }

    void UpdateBilling(cui_ctx* ui, const BillingNotice& notice)
    {
        if (notice.message == billing_.message && notice.argument == billing_.argument) return;
        billing_ = notice;
        Destroy(ui, billingImage_);
        auto caption = Message(notice.message);
        if (notice.argument) {
            const auto marker = caption.find("%d");
            if (marker != std::string::npos) caption.replace(marker, 2, std::to_string(notice.argument));
        }
        if (!caption.empty()) billingImage_ = Raster(ui, caption, 308, 95, true, mir3::TextFont::dialog);
    }

    static cui_font LoadEditorFont(cui_ctx* ui, const std::string& face)
    {
        if (face.empty()) return CUI_FONT_BODY;
        wchar_t name[LF_FACESIZE]{};
        if (!MultiByteToWideChar(CP_UTF8, 0, face.c_str(), -1, name, LF_FACESIZE)) return CUI_FONT_BODY;
        if (std::wstring_view(name)==L"\u5b8b\u4f53" || std::wstring_view(name)==L"SimSun") {
            wchar_t windows[MAX_PATH]{};
            const auto length = GetWindowsDirectoryW(windows,MAX_PATH);
            if (length && length<MAX_PATH) {
                // Read the collection directly so the original face retains its sfnt table offsets.
                const auto bytes = Read(std::filesystem::path(windows)/"Fonts"/"simsun.ttc");
                const auto loaded = cui_font_load_mono_memory(ui,bytes.data(),bytes.size(),true);
                if (loaded>=0) return loaded;
            }
        }
        HDC dc = CreateCompatibleDC(nullptr);
        HFONT font = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH, name);
        cui_font loaded = CUI_FONT_BODY;
        if (dc && font) {
            HGDIOBJ selected = SelectObject(dc, font);
            const DWORD size = GetFontData(dc, 0, 0, nullptr, 0);
            if (size && size != GDI_ERROR) {
                std::vector<std::uint8_t> bytes(size);
                if (GetFontData(dc, 0, 0, bytes.data(), size) == size) {
                    const auto handle = cui_font_load_mono_memory(ui, bytes.data(), size, true);
                    if (handle >= 0) loaded = handle;
                }
            }
            SelectObject(dc, selected);
        }
        if (font) DeleteObject(font);
        if (dc) DeleteDC(dc);
        return loaded;
    }

    static std::vector<std::uint8_t> Read(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::binary);
        return {std::istreambuf_iterator<char>(file), {}};
    }

    Image Upload(cui_ctx* ui, int width, int height, const std::vector<std::uint8_t>& rgba, bool nearest = false)
    {
        if (rgba.empty()) return {};
        const auto image = nearest ? cui_image_create_nearest(ui,width,height,rgba.data()) :
            cui_image_create(ui,width,height,rgba.data());
        if (image) textures_.push_back(image);
        return {image, width, height};
    }

    Image UploadFrame(cui_ctx* ui, mir3::WilArchive& archive, int index, bool nearest = false)
    {
        const auto frame = archive.frame(index);
        return frame ? Upload(ui,frame->width,frame->height,frame->pixels.rgba,nearest) : Image{};
    }

    Image Raster(cui_ctx* ui, const std::string& text, int width, int height, bool centered, mir3::TextFont font, bool inverted = false)
    {
        const auto bitmap = mir3::raster_text(text, width, height, centered, font, font_, inverted);
        return Upload(ui, width, height, bitmap.rgba, true);
    }

    void Destroy(cui_ctx* ui, Image& image)
    {
        if (!image.texture) return;
        cui_image_destroy(ui, image.texture);
        std::erase(textures_, image.texture);
        image = {};
    }

    void ReplaceText(cui_ctx* ui, Image& image, const std::string& value)
    {
        Destroy(ui, image);
        image = value.empty() ? Image{} : Raster(ui, value, 100, 14, false, mir3::TextFont::normal);
    }

    void UpdateFieldText(cui_ctx* ui, const std::string& account, const std::string& passwordMask)
    {
        if (account != account_) {
            ReplaceText(ui, accountImage_, account);
            account_ = account;
        }
        if (passwordMask != password_) {
            ReplaceText(ui, passwordImage_, passwordMask);
            password_ = passwordMask;
        }
    }

    static float Scale(cui_rect rect)
    {
        return (std::min)(rect.w / 640, rect.h / 480);
    }

    void Paint(cui_ctx* ui, cui_rect rect) const
    {
        const auto scale = Scale(rect);
        const float ox = rect.x + (rect.w - 640 * scale) / 2;
        const float oy = rect.y + (rect.h - 480 * scale) / 2;
        const auto bounds = [&](float x, float y, float w, float h) {
            return cui_rect{ox + x * scale, oy + y * scale, w * scale, h * scale};
        };
        const cui_color white{255, 255, 255, 255};
        const auto image = [&](const Image& frame, float x, float y, cui_color color) {
            if (frame.texture)
                cui_draw_image(ui, frame.texture, bounds(x, y, frame.width, frame.height), {0, 0, 1, 1}, {}, color);
        };
        const auto fill = [&](float x, float y, float w, float h, cui_color color) {
            cui_draw_rect(ui, bounds(x, y, w, h), {}, color);
        };
        const auto box = [&](int x, int y, int w, int h, cui_color tint, cui_color border) {
            cui_draw_rect_premultiplied(ui, bounds(x, y, w, h), tint);
            fill(x - 1, y - 1, w + 1, 1, border);
            fill(x - 1, y + h - 1, w + 1, 1, border);
            fill(x - 1, y, 1, h - 1, border);
            fill(x + w - 1, y, 1, h - 1, border);
        };

        cui_draw_rect(ui, rect, {}, {0, 0, 0, 255});
        cui_push_clip(ui, bounds(0, 0, 640, 480));
        image(backgrounds_[0], 0, 60, white);
        if (serverPage_) {
            box(20, 60, serverWidth_, serverRowHeight_ * static_cast<int>(serverRows_.size()) + 20,
                {100, 100, 100, 120}, {150, 150, 150, 255});
            for (int i = 0; i < static_cast<int>(serverRows_.size()); ++i) {
                const bool pressed = pressedServer_ == i;
                const bool hover = pressedServer_ < 0 && hoveredServer_ == i;
                const int y = 70 + i * serverRowHeight_;
                if (pressed || hover) {
                    const cui_color tint = pressed ? cui_color{255, 100, 100, 50} : cui_color{100, 100, 100, 50};
                    const cui_color border = pressed ? cui_color{255, 100, 100, 255} : cui_color{100, 100, 200, 255};
                    box(20, y, serverWidth_, serverRowHeight_, tint, border);
                }
                const cui_color ink = pressed ? cui_color{255, 100, 100, 255} :
                    hover ? cui_color{0, 0, 255, 255} : cui_color{30, 30, 30, 255};
                image(serverRows_[i].image, 40, y + 10, ink);
            }
            for (int i = 0; i < static_cast<int>(legends_.size()); ++i)
                image(legends_[i], 40 + 200 * i, 440, {200, 200, 50, 255});
        } else {
            PaintCredentials(image, fill, box);
        }
        if (noticeVisible_) {
            image(dialog_[0], 140, 110, white);
            image(dialog_[pressed_ ? 2 : 1], 277, 298, white);
            image(selectedCaption_, 163, 194, {255, 250, 200, 255});
        }
        if (serverPage_ && handoffShade_ >= 0) {
            const auto shade = static_cast<std::uint8_t>(handoffShade_);
            fill(0, 0, 640, 480, {shade, shade, shade, 255});
        }
        cui_pop_clip(ui);
    }

    void PaintCredentials(const auto& image, const auto& fill, const auto& box) const
    {
        const cui_color white{255, 255, 255, 255};
        if (!announcementImages_.empty()) {
            box(18, 115, 234, 238, {75, 50, 0, 100}, {100, 75, 50, 255});
            static constexpr std::uint32_t colors[] = {0, 657930, 0xffffff, 255, 65280, 16750230, 0x50ffff, 33023};
            const auto color = [](unsigned index) {
                const auto value = index < std::size(colors) ? colors[index] : 0;
                return cui_color{static_cast<std::uint8_t>(value), static_cast<std::uint8_t>(value >> 8),
                    static_cast<std::uint8_t>(value >> 16), 255};
            };
            for (std::size_t i = 0; i < announcementImages_.size(); ++i) {
                const int y = 120 + 14 * static_cast<int>(i) + announcement_.offset;
                if (y < 120 || y + 14 >= 353) continue;
                const auto& row = announcement_.rows[i];
                const auto& entry = announcementImages_[i];
                if (row.background) fill(23, y, entry.width, entry.height, color(row.background));
                image(entry.image, 23, y, color(row.foreground));
            }
        }
        box(0, 372, 640, 48, {50, 50, 60, 100}, {100, 100, 100, 255});
        for (int i = 0; i < static_cast<int>(buttons_.size()); ++i)
            image(backgrounds_[2 + 2 * i + (pressedButton_ == i)], buttons_[i][0], buttons_[i][1], white);
        if (hoverSignIn_) {
            const auto& frame = backgrounds_[2];
            const auto& origin = buttons_[0];
            box(origin[0], origin[1], frame.width, frame.height, {150, 100, 50, 50}, {200, 150, 100, 255});
        }
        image(backgrounds_[1], 100, 436, white);
        if (showAccount_) image(accountImage_, 128, 440, white);
        if (showPassword_) image(passwordImage_, 326, 440, white);
    }
};

void Dialogs::Assets::OpenSelection(cui_ctx* ui, mir3::WilArchive& gameInterface)
{
    const auto* background = SelectionSource(50);
    selectionReady_ = background && background->width == 640 && background->height == 480;
    for (int id = 51; id <= 58; ++id) {
        selectionButtons_[id] = UploadFrame(ui, selectionArchive_, id);
        selectionReady_ &= selectionButtons_[id].texture != 0;
    }
    for (int id : {1150,1151,1152,1153,1154,1155,1241,1242,1245,1246,1248}) {
        selectionButtons_[id] = UploadFrame(ui, gameInterface, id);
        selectionReady_ &= selectionButtons_[id].texture != 0;
    }
    selectionConnectingText_ = Raster(ui, Message(200), 308, 95, true, mir3::TextFont::dialog);
    deleteText_ = Raster(ui, Message(228), 278, 75, true, mir3::TextFont::dialog);
    const auto* creationBackground = SelectionSource(80);
    creationReady_ = creationBackground && creationBackground->width == 640 && creationBackground->height == 480;
    for (int id : {81,85,86,87,88,89,90,91,92,93,94,95,96,97,98}) {
        selectionButtons_[id] = UploadFrame(ui, selectionArchive_, id);
        creationReady_ &= selectionButtons_[id].texture != 0;
    }
    for (int i = 0; i < 5; ++i) {
        const auto text = Message(201 + i);
        const auto extent = mir3::raster_text(text,640,32,false,mir3::TextFont::normal);
        creationHints_[i] = Raster(ui,text,extent.text_width+2,extent.text_height,false,mir3::TextFont::normal);
    }
}

const mir3::WilFrame* Dialogs::Assets::SelectionSource(int index)
{
    auto it = selectionSources_.find(index);
    if (it == selectionSources_.end()) it = selectionSources_.emplace(index, selectionArchive_.frame(index)).first;
    return it->second ? &*it->second : nullptr;
}

bool Dialogs::Assets::SelectionHit(float x, float y, float w, float h) const
{
    return selectionMouse_.x >= x && selectionMouse_.x < x + w &&
           selectionMouse_.y >= y && selectionMouse_.y < y + h;
}

bool Dialogs::Assets::SelectionButtonHit(int index, float x, float y) const
{
    const auto& image = selectionButtons_.at(index);
    return image.texture && SelectionHit(x, y, image.width, image.height);
}

void Dialogs::Assets::SyncSelection(const LoginClient& client, bool entering)
{
    if (entering) {
        selectionCharacters_.clear();
        selectionRoles_ = {};
        selectionHits_ = {};
        selectionIndex_ = -1;
        selectionPage_ = 0;
        selectionMode_ = CharacterDialog::None;
        selectionEditor_ = -1;
        selectionPress_ = -1;
        deletePress_ = -1;
        selectionDirty_ = true;
    }
    if (selectionWaiting_) return;
    if (!entering && selectionRevision_ == client.characterListRevision && selectionCharacters_ == client.characters) return;
    selectionRevision_ = client.characterListRevision;
    selectionCharacters_ = client.characters;
    selectionPage_ = client.characterPage;
    selectionIndex_ = client.characterIndex;
    selectionRoles_ = {};
    selectionHits_ = {};
    for (int i = 0; i < static_cast<int>(selectionCharacters_.size()) && i < 4; ++i) {
        const auto& character = selectionCharacters_[i];
        auto& role = selectionRoles_[i];
        role.occupied = true;
        role.gender = static_cast<std::uint8_t>(character.gender);
        role.profession = static_cast<std::uint8_t>(character.profession);
        role.x = selectionCharacters_.size() == 1 ? 300 : i % 2 ? 350 : 250;
        role.y = i % 2 ? 250 : 210;
        role.start(i == selectionIndex_ ? 1 : 0);
    }
    selectionDirty_ = true;
}

void Dialogs::Assets::SetSelectedCharacter(LoginClient& client, int index, bool changePage)
{
    if (index < 0 || index >= static_cast<int>(selectionCharacters_.size()) || index == selectionIndex_) return;
    if (changePage) {
        if (selectionIndex_ >= 0) selectionRoles_[selectionIndex_].start(0);
        selectionPage_ = client.characterPage = index / 2;
    } else if (const int other = index ^ 1; other < static_cast<int>(selectionCharacters_.size())) {
        selectionRoles_[other].start(3);
    }
    selectionIndex_ = client.characterIndex = index;
    selectionRoles_[index].start(1);
    selectionDirty_ = true;
}

void Dialogs::Assets::AdvanceSelectionRole(LoginClient& client, mir3::SelectionRole& role, cui_rect& hit,
                                          std::uint32_t ms, bool creating, bool selected)
{
    if (role.elapsed + ms > 120) {
        if (const auto* frame = SelectionSource(role.frame + 1))
            hit = {static_cast<float>(role.x + frame->offset_x), static_cast<float>(role.y + frame->offset_y),
                   static_cast<float>(frame->width), static_cast<float>(frame->height)};
        if ((creating ? selected : role.action == 1) && role.frame == role.first_frame() && client.audioOptions.effects) {
            const auto voice = clickSound_.parent_path() / mir3::LoginAudio::voice(creating, role.profession, role.gender);
            if (std::filesystem::exists(voice)) client.audio.PlayEffect(voice.c_str(), client.audioOptions);
        }
        selectionDirty_ = true;
    }
    role.advance(ms);
}

void Dialogs::Assets::RefreshSelectionText(cui_ctx* ui, const LoginClient& client)
{
    const int noticeId = client.characterNotice.empty() ? 0 : client.characterNoticeId;
    if (noticeId != selectionCaptionId_ || selectionCaptionFallback_ != client.characterNotice) {
        selectionCaptionId_ = noticeId;
        selectionCaptionFallback_ = client.characterNotice;
        Destroy(ui, selectionCaption_);
        auto message = Message(noticeId);
        if (message.empty()) message = client.characterNotice;
        if (!message.empty()) selectionCaption_ = Raster(ui, message, 308, 95, true, mir3::TextFont::dialog);
    }
    if (selectionIndex_ < 0 || selectionIndex_ >= static_cast<int>(selectionCharacters_.size())) return;
    const auto& character = selectionCharacters_[selectionIndex_];
    if (selectionStatRole_ == character) return;
    selectionStatRole_ = character;
    const auto name = Message(206) + "   " + character.label;
    const auto level = Message(207) + "   " + std::to_string(character.level);
    const auto profession = Message(208 + static_cast<int>(character.profession));
    const auto measure = [&](const std::string& text) {
        return mir3::raster_text(text, 500, 16, false, mir3::TextFont::selection, font_).text_width;
    };
    selectionStatWidth_ = (std::max)({1, measure(name), measure(profession)});
    const std::array<std::string, 3> lines{name, level, profession};
    for (int i = 0; i < 3; ++i) {
        Destroy(ui, selectionStats_[i]);
        selectionStats_[i] = Raster(ui, lines[i], selectionStatWidth_, 16, true, mir3::TextFont::selection);
    }
}

void Dialogs::Assets::RefreshSelectionSurface(cui_ctx* ui)
{
    if (!selectionDirty_) return;
    selectionDirty_ = false;
    const bool creating = selectionMode_ == CharacterDialog::Create;
    auto pixels = creating ? CreationBackdrop() : SelectionSource(50)->pixels.rgba;
    const auto blit = [&](int index, int x, int y, mir3::SelectionBlend blend) {
        if (const auto* frame = SelectionSource(index)) mir3::selection_blit(pixels, *frame, x, y, blend);
    };
    using Blend = mir3::SelectionBlend;
    if (creating) {
        for (int i = 0; i < 2; ++i) {
            const auto& role = creation_.roles[i];
            const bool selected = i == creation_.selected;
            blit(role.frame + 20, role.x, role.y, Blend::shadow);
            blit(selected ? role.frame : role.first_frame(), role.x, role.y, selected ? Blend::normal : Blend::inactive);
            if (selected) blit(role.frame + 40, role.x, role.y, Blend::additive);
        }
        if (const auto* resource = SelectionSource(82)) {
            auto shadow = *resource;
            shadow.offset_x = shadow.offset_y = 0;
            mir3::selection_blit(pixels, shadow, 201, 434, Blend::shadow);
        }
    } else {
        for (int i = selectionPage_ * 2; i < (std::min)(static_cast<int>(selectionCharacters_.size()), selectionPage_ * 2 + 2); ++i) {
            const auto& role = selectionRoles_[i];
            blit(role.frame + 20, role.x, role.y, Blend::shadow);
            blit(role.frame, role.x, role.y, role.action ? Blend::normal : Blend::inactive);
            blit(role.frame + 40, role.x, role.y, Blend::additive);
        }
    }
    if (!cui_image_update(ui, selectionSurface_.texture, 640, 480, pixels.data())) {
        Destroy(ui, selectionSurface_);
        selectionSurface_ = Upload(ui, 640, 480, pixels);
    }
}

const std::vector<std::uint8_t>& Dialogs::Assets::CreationBackdrop()
{
    const int key = creation_.profession * 2 + creation_.selected + (selectionIndex_ >= 0 ? 6 : 0);
    if (!creationBackdrop_.empty() && creationBackdropKey_ == key) return creationBackdrop_;
    creationBackdropKey_ = key;
    creationBackdrop_ = SelectionSource(80)->pixels.rgba;
    const auto source = Message(216 + creation_.profession);
    std::string line;
    std::vector<std::string> lines;
    for (std::size_t i = 0; i < source.size();) {
        std::size_t end = i + 1;
        while (end < source.size() && (static_cast<unsigned char>(source[end]) & 0xc0) == 0x80) ++end;
        line.append(source, i, end - i);
        i = end;
        if (i == source.size() || mir3::raster_text(line, 500, 16, false, mir3::TextFont::normal).text_width >= 430) {
            lines.push_back(std::move(line));
            line.clear();
        }
    }
    const auto height = 18 * (static_cast<int>(lines.size()) + 1) + 20;
    mir3::selection_rectangle(creationBackdrop_, 100, 15, 450, height, 0x323232, 50);
    mir3::selection_rectangle(creationBackdrop_, 100, 15, 450, 1, 0x969696, 255);
    mir3::selection_rectangle(creationBackdrop_, 100, 15, 1, height, 0x969696, 255);
    mir3::selection_rectangle(creationBackdrop_, 549, 15, 1, height, 0x969696, 255);
    mir3::selection_rectangle(creationBackdrop_, 100, 14 + height, 450, 1, 0x969696, 255);
    const auto title = Message(211 + creation_.selected) + Message(213 + creation_.profession);
    const auto titleBitmap = mir3::raster_text(title, 430, 18, false, mir3::TextFont::gold, font_);
    constexpr unsigned colors[] = {0x96c8fa, 11184890, 0x96dc96};
    const auto tint = selectionIndex_ >= 0 && selectionIndex_ < 2 ? creation_.profession : 0;
    mir3::selection_text(creationBackdrop_, titleBitmap.rgba, 110, 25, 430, 18, colors[tint]);
    for (int i = 0; i < static_cast<int>(lines.size()); ++i) {
        const auto bitmap = mir3::raster_text(lines[i], 430, 18, false, mir3::TextFont::normal, font_);
        mir3::selection_text(creationBackdrop_, bitmap.rgba, 110, 50 + i * 18, 430, 18, 16775930);
    }
    return creationBackdrop_;
}


void Dialogs::Assets::DrawSelectionEditor(cui_ctx* ui, LoginClient& client, float scale, float ox, float oy, bool focus)
{
    const bool creating = selectionMode_ == CharacterDialog::Create;
    const char* id = creating ? "create-role-name" : "delete-role-confirmation";
    const float x = creating ? 288 : 164;
    const float y = creating ? 405 : 338;
    const float width = creating ? 75 : 309;
    const float height = creating ? 13 : 15;
    cui_overlay_begin(ui, {ox + x * scale, oy + y * scale, width * scale, height * scale});
    cui_push_id_int(ui, client.characterEditor);
    if (focus) cui_text_field_focus(ui, id, selectionEditor_ != client.characterEditor);
    selectionEditor_ = client.characterEditor;
    auto options = ModalEditorOptions(height*scale,scale);
    options.keep_tab = !creating;
    options.read_only = client.characterRequest != 0;
    options.byte_limit = creating ? 13 : 90;
    options.measure_bytes = AnsiBytes;
    cui_input_ex(ui, id, client.characterInput, sizeof client.characterInput, &options);
    cui_pop_id(ui);
    cui_box_end(ui);
}

void Dialogs::Assets::SubmitCharacterCreation(LoginClient& client)
{
    if (!client.characterInput[0]) return;
    const auto name = ConvertText(client.characterInput, CP_UTF8, 936);
    const int error = name.empty() ? 225 : mir3::CreateCharacterPage::name_error(name);
    if (error) client.ShowCharacterNotice(error, "The character name is invalid.");
    else client.SubmitCharacterDialog();
}

void Dialogs::Assets::SelectionInput(cui_ctx* ui, LoginClient& client)
{
    const bool down = cui_mouse_clicked(ui, CUI_MOUSE_LEFT);
    const bool up = cui_mouse_released(ui, CUI_MOUSE_LEFT);
    selectionHover_ = -1;
    if (selectionMode_ != CharacterDialog::Create) {
        for (int i = 0; i < 4; ++i) {
            const auto& origin = selectionOrigins_[i];
            if (SelectionButtonHit(52 + 2 * i, origin.x, origin.y)) selectionHover_ = i;
        }
        if (selectionCharacters_.size() > 2) {
            if (selectionPage_ == 0 && SelectionButtonHit(1153, 550, 400)) selectionHover_ = 4;
            if (selectionPage_ == 1 && SelectionButtonHit(1150, 500, 400)) selectionHover_ = 5;
        }
    }
    if (up) selectionPress_ = -1;
    if (selectionWaiting_) return;
    if (selectionNotice_) {
        if (NoticeInput(ui, true, selectionMouse_.x, selectionMouse_.y)) {
            client.characterNotice.clear();
            client.characterNoticeId = 0;
            if (selectionMode_ == CharacterDialog::Create) {
                SecureZeroMemory(client.characterInput, sizeof client.characterInput);
                ++client.characterEditor;
            }
            cui_request_redraw(ui);
        }
        return;
    }
    pressed_ = false;
    if (client.characterRequest) return;
    if (selectionMode_ == CharacterDialog::Delete) {
        if (cui_key_pressed(ui, CUI_KEY_TAB)) client.deleteCancelSelected = !client.deleteCancelSelected;
        const int hit = SelectionButtonHit(1241, 181, 298) ? 0 : SelectionButtonHit(1245, 374, 298) ? 1 : -1;
        if (down) {
            deletePress_ = hit;
            if (hit >= 0) client.deleteCancelSelected = hit == 1;
        }
        if (up) deletePress_ = -1;
        if (up && hit >= 0) {
            if (hit==1) client.CloseCharacterDialog();
            else client.SubmitCharacterDialog();
            cui_request_redraw(ui);
        }
        return;
    }
    if (selectionMode_ == CharacterDialog::Create) {
        int hit = -1;
        for (int i = 0; i < 5; ++i) {
            const auto& origin = creationOrigins_[i];
            if (SelectionButtonHit(creationFrames_[i][0], origin.x, origin.y)) hit = i;
        }
        for (int i = 0; i < 5; ++i)
            if (creation_.buttons[i] != 2) creation_.buttons[i] = i == hit ? 1 : 0;
        if (down) {
            for (int gender = 0; gender < 2; ++gender) {
                const auto& bounds = creationHits_[gender];
                if (SelectionHit(bounds.x, bounds.y, bounds.w, bounds.h) && creation_.select(gender)) {
                    client.gender = gender;
                    selectionDirty_ = true;
                }
            }
            if (hit >= 0) creation_.buttons[hit] = 2;
        }
        if (up) {
            creation_.buttons = {};
            if (hit >= 0 && client.audioOptions.effects && std::filesystem::exists(clickSound_))
                client.audio.PlayEffect(clickSound_.c_str(), client.audioOptions);
            if (hit >= 0 && hit < 3) {
                creation_.set_profession(hit);
                client.profession = hit;
                creationHits_ = {};
                selectionDirty_ = true;
            } else if (hit == 4) {
                client.CloseCharacterDialog();
                client.RefreshCharacterList();
            }
        }
        if (up && hit==3) SubmitCharacterCreation(client);
        return;
    }
    if (down) {
        selectionPress_ = selectionHover_;
        for (int i = selectionPage_ * 2; i < (std::min)(static_cast<int>(selectionCharacters_.size()), selectionPage_ * 2 + 2); ++i) {
            const auto& hit = selectionHits_[i];
            if (SelectionHit(hit.x, hit.y, hit.w, hit.h)) {
                SetSelectedCharacter(client, i, false);
                break;
            }
        }
    }
    if (!up || selectionHover_ < 0) return;
    if (client.audioOptions.effects && std::filesystem::exists(clickSound_))
        client.audio.PlayEffect(clickSound_.c_str(), client.audioOptions);
    switch (selectionHover_) {
    case 0:
        if (selectionCharacters_.size() >= 2) client.ShowCharacterNotice(226, "This account has no free character slot.");
        else client.OpenCharacterDialog(CharacterDialog::Create);
        break;
    case 1: client.OpenCharacterDialog(CharacterDialog::Delete); break;
    case 2: client.StartCharacter(); break;
    case 3:
        if (exitUrl_.starts_with("http://") || exitUrl_.starts_with("https://")) {
            cui_app_open_url(exitUrl_.c_str());
            cui_app_quit(nullptr);
        }
        break;
    case 4: SetSelectedCharacter(client, 2, true); break;
    case 5: SetSelectedCharacter(client, 0, true); break;
    }
    cui_request_redraw(ui);
}

void Dialogs::Assets::DrawSelection(cui_ctx* ui, LoginClient& client)
{
    const double now = cui_time(ui);
    const bool entering = selectionTick_ < 0;
    const auto elapsed = std::floor(now * 1000) - std::floor(selectionTick_ * 1000);
    const auto ms = entering ? 0u : static_cast<std::uint32_t>(std::clamp(elapsed, 0.0, 1000.0));
    selectionTick_ = now;
    const bool querying = client.stage == Stage::ConnectingCharacters || client.characterRequest == CM_QUERYCHR;
    if (querying && (!selectionQuerying_ || entering)) selectionQueryAt_ = now;
    selectionQuerying_ = querying;
    selectionWaiting_ = querying || (selectionQueryAt_ >= 0 && now - selectionQueryAt_ < 0.5);
    SyncSelection(client, entering);
    if (selectionMode_ != client.characterDialog) {
        selectionMode_ = client.characterDialog;
        selectionDirty_ = true;
        deletePress_ = -1;
        if (selectionMode_ == CharacterDialog::Create) {
            creation_.selected = client.gender;
            creation_.open();
            creationHits_ = {};
        }
    }
    selectionNotice_ = !client.characterNotice.empty();
    const auto viewport = cui_viewport_size(ui);
    const auto scale = Scale({0, 0, viewport.x, viewport.y});
    if (scale <= 0) return;
    const float ox = (viewport.x - 640 * scale) / 2, oy = (viewport.y - 480 * scale) / 2;
    const auto mouse = cui_mouse_pos(ui);
    selectionMouse_ = {(mouse.x - ox) / scale, (mouse.y - oy) / scale};
    if (!selectionWaiting_) {
        if (selectionMode_ == CharacterDialog::Create) {
            for (int i = 0; i < 2; ++i)
                AdvanceSelectionRole(client, creation_.roles[i], creationHits_[i], ms, true, i == creation_.selected);
        } else {
            for (int i = selectionPage_ * 2; i < (std::min)(static_cast<int>(selectionCharacters_.size()), selectionPage_ * 2 + 2); ++i)
                AdvanceSelectionRole(client, selectionRoles_[i], selectionHits_[i], ms, false, i == selectionIndex_);
        }
    }
    auto* self = this;
    cui_canvas(ui, "character-selection-art", CUI_FILL, CUI_FILL, [](cui_ctx* ui, cui_rect rect, void* model) {
        (*static_cast<Assets**>(model))->PaintSelection(ui, rect);
    }, &self, sizeof self);
    RefreshSelectionText(ui, client);
    SelectionInput(ui, client);
    RefreshSelectionSurface(ui);
    if (selectionMode_ != CharacterDialog::None && client.characterDialog == selectionMode_ &&
        !selectionNotice_ && client.characterNotice.empty() && !selectionWaiting_ &&
        (!client.characterRequest || selectionMode_ == CharacterDialog::Create)) {
        const bool deleteSpace = selectionMode_==CharacterDialog::Delete && cui_key_pressed(ui,CUI_KEY_SPACE);
        const bool submit = !client.characterRequest && (deleteSpace || cui_key_pressed(ui,CUI_KEY_ENTER));
        if (deleteSpace) cui_input_consume_text_prefix(ui," ");
        const bool focus = submit || selectionEditor_ != client.characterEditor || cui_mouse_clicked(ui,CUI_MOUSE_LEFT);
        DrawSelectionEditor(ui, client, scale, ox, oy, focus);
        if (submit) {
            if (selectionMode_==CharacterDialog::Create) SubmitCharacterCreation(client);
            else if (client.deleteCancelSelected) client.CloseCharacterDialog();
            else client.SubmitCharacterDialog();
            cui_request_redraw(ui);
        }
    }
    if (selectionWaiting_ || selectionMode_ == CharacterDialog::Create || !selectionCharacters_.empty())
        cui_request_wakeup(ui, 0.016f);
}

void Dialogs::Assets::PaintSelection(cui_ctx* ui, cui_rect rect) const
{
    const float scale = Scale(rect);
    const float ox = rect.x + (rect.w - 640 * scale) / 2, oy = rect.y + (rect.h - 480 * scale) / 2;
    const auto bounds = [&](float x, float y, float w, float h) {
        return cui_rect{ox + x * scale, oy + y * scale, w * scale, h * scale};
    };
    const auto image = [&](const Image& frame, float x, float y, cui_color color) {
        if (frame.texture) cui_draw_image(ui, frame.texture, bounds(x, y, frame.width, frame.height), {0,0,1,1}, {}, color);
    };
    const cui_color white{255,255,255,255}, captionInk{255,250,200,255};
    const auto sprite = [&](int id, float x, float y) { image(selectionButtons_.at(id), x, y, white); };
    const auto box = [&](int x, int y, int w, int h, cui_color fill, cui_color edge) {
        cui_draw_rect_premultiplied(ui, bounds(x, y, w, h), fill);
        for (const auto& side : std::array<cui_rect,4>{bounds(x-1,y-1,w+1,1), bounds(x-1,y+h-1,w+1,1),
                                                       bounds(x-1,y,1,h-1), bounds(x+w-1,y,1,h-1)})
            cui_draw_rect(ui, side, {}, edge);
    };
    cui_draw_rect(ui, rect, {}, {0,0,0,255});
    cui_push_clip(ui, bounds(0,0,640,480));
    cui_draw_rect(ui,bounds(0,0,640,480),{},{76,76,76,255});
    image(selectionSurface_, 0, 0, white);
    if (selectionMode_ == CharacterDialog::Create) {
        sprite(81, 247, 384);
        for (int i = 0; i < 5; ++i)
            sprite(creationFrames_[i][creation_.buttons[i]], creationOrigins_[i].x, creationOrigins_[i].y);
        box(287,404,77,15,{}, {120,90,60,255});
        constexpr std::array<cui_vec2, 5> hints{{{299,440},{342,440},{386,440},{479,460},{520,460}}};
        for (int i = 0; i < 5; ++i) {
            const auto& origin = creationOrigins_[i];
            if (!SelectionButtonHit(creationFrames_[i][0], origin.x, origin.y)) continue;
            const auto& hint = creationHints_[i];
            const int width=hint.width+5, height=hint.height+5;
            const int x=std::min(static_cast<int>(hints[i].x),640-width);
            const int y=std::min(static_cast<int>(hints[i].y),480-height);
            box(x,y,width,height,{125,125,75,50},{180,180,120,255});
            cui_push_clip(ui,bounds(x+1,y+1,width-2,height-2));
            for (const auto& offset : std::array<cui_vec2,4>{{{1,2},{3,2},{2,1},{2,3}}})
                image(hint, x + offset.x, y + offset.y, {25,25,25,255});
            image(hint, x + 2, y + 2, {255,255,180,255});
            cui_pop_clip(ui);
            break;
        }
    } else {
        for (int i = 0; i < 4; ++i) {
            const int id = 52 + 2 * i;
            const auto& origin = selectionOrigins_[i];
            sprite(id - (selectionPress_ == i), origin.x, origin.y);
            if (selectionHover_ == i) {
                const auto& button = selectionButtons_.at(id);
                box(static_cast<int>(origin.x), static_cast<int>(origin.y), button.width, button.height,
                    {150,100,50,50}, {200,150,100,255});
            }
        }
        if (selectionCharacters_.size() > 2) {
            sprite(selectionPage_ ? 1155 : selectionPress_ == 4 ? 1154 : 1153, 550, 400);
            sprite(selectionPage_ ? selectionPress_ == 5 ? 1151 : 1150 : 1152, 500, 400);
        }
        if (selectionIndex_ >= 0) {
            box(79,108,selectionStatWidth_+20,70,{50,50,25,100},{100,100,80,255});
            image(selectionStats_[0], 89, 118, {255,200,150,255});
            image(selectionStats_[1], 89, 138, {150,240,150,255});
            image(selectionStats_[2], 89, 158, {255,255,175,255});
        }
    }
    if (selectionWaiting_) {
        sprite(1248, 155, 150);
        image(selectionConnectingText_, 178, 234, captionInk);
    } else if (selectionNotice_) {
        image(dialog_[0], 140, 110, white);
        image(dialog_[pressed_ ? 2 : 1], 277, 298, white);
        image(selectionCaption_, 163, 194, captionInk);
    } else if (selectionMode_ == CharacterDialog::Delete) {
        image(dialog_[0], 140, 110, white);
        sprite(deletePress_ == 0 ? 1242 : 1241, 181, 298);
        sprite(deletePress_ == 1 ? 1246 : 1245, 374, 298);
        image(deleteText_, 183, 194, captionInk);
    }
    cui_pop_clip(ui);
}


int Dialogs::Assets::HudHint::Advance(cui_ctx* ui, int caption, int fullHeight)
{
    if (previous != caption) {
        previous = caption;
        expanding = true;
    }
    const int drawn = expanding ? (std::min)(++height, fullHeight) : fullHeight;
    if (expanding && height >= fullHeight) {
        expanding = false;
        height = 6;
    }
    if (expanding) cui_request_wakeup(ui, 0.016f);
    return drawn;
}

void Dialogs::Assets::HudHint::Paint(cui_ctx* ui, const HudPoint& base, const Image& hint, int height, int anchorX, int anchorY)
{
    const int left = (std::min)(800 - hint.width, anchorX), top = anchorY - height;
    const auto outer = base.Rect(left, top, hint.width, height);
    cui_draw_rect(ui, outer, {}, {0,0,0,255});
    cui_draw_rect(ui, base.Rect(left+1,top+1,hint.width-2,height-2), {}, {255,255,150,255});
    cui_push_clip(ui, outer);
    cui_draw_image(ui, hint.texture, base.Rect(left, top + (height - hint.height) / 2, hint.width, hint.height),
                   {0,0,1,1}, {}, {0,0,0,255});
    cui_pop_clip(ui);
}

Dialogs::Assets::Image Dialogs::Assets::MakeHudHint(cui_ctx* ui, int caption)
{
    const auto title = Message(caption);
    if (title.empty()) return {};
    const auto extent = mir3::raster_text(title, 800, 24, false, mir3::TextFont::normal);
    return Raster(ui, title, extent.text_width + 12, extent.text_height + 4, true, mir3::TextFont::normal);
}

void Dialogs::Assets::OpenQuests(cui_ctx* ui, mir3::WilArchive& archive)
{
    questBackground_ = UploadFrame(ui, archive, 1420);
    questThumb_ = UploadFrame(ui, archive, 1225);
    questReady_ = questBackground_.texture && questThumb_.texture;
    constexpr int buttons[] = {1422,1425};
    for (int i = 0; i < 2; ++i) {
        for (int pressed = 0; pressed < 2; ++pressed) {
            questButtons_[i][pressed] = UploadFrame(ui, archive, buttons[i] + pressed);
            questReady_ &= questButtons_[i][pressed].texture != 0;
        }
        questHints_[i] = MakeHudHint(ui, questHintMessages_[i]);
    }
}

bool Dialogs::Assets::QuestBlocksScene(cui_ctx* ui) const
{
    if (questDragging_ || questScrollDrag_ || questPressed_ >= 0) return true;
    const auto view = cui_viewport_size(ui);
    const HudPoint base({0,0,view.x,view.y});
    return base.scale > 0 && Contains({float(questX_),float(questY_),340,440}, base.ToLocal(cui_mouse_pos(ui)));
}

std::vector<Dialogs::Assets::QuestLine> Dialogs::Assets::QuestLines(const mir3_cui::QuestPanel& quests)
{
    std::vector<QuestLine> lines;
    for (const auto& entry : quests.entries) {
        lines.push_back({&entry.record, entry.record.id, false});
        if (entry.record.id == quests.expanded)
            for (const auto& child : entry.children) lines.push_back({&child, entry.record.id, true});
    }
    return lines;
}

void Dialogs::Assets::ToggleQuest(LoginClient& client)
{
    if (client.quests.open) {
        client.quests.open = false;
        LeaveQuests();
    } else {
        client.OpenQuests();
        if (QuestReady() && client.quests.open) RaiseUtility(UtilityPanel::Quest);
    }
}

void Dialogs::Assets::ScrollQuest(mir3_cui::QuestPanel& quests,cui_vec2 point,int wheel)
{
    if (!wheel) return;
    if (Contains({float(questX_+80),float(questY_+310),170,70},point)) {
        const auto* detail=quests.Description();
        if (detail && !detail->empty())
            questDescriptionFirst_=std::clamp(questDescriptionFirst_+wheel,0,3);
    }
    if (point.y<=questY_+310)
        questFirst_=std::clamp(questFirst_+wheel,0,(std::max)(0,int(QuestLines(quests).size())-1));
}

bool Dialogs::Assets::HandleQuestInput(cui_ctx* ui, LoginClient& client, bool blocked, bool fallback)
{
    const bool covered = QuestBlocksScene(ui);
    if (fallback && !blocked && questFocused_ && cui_shortcut(ui,0,CUI_KEY_D)) {
        ToggleQuest(client);
        return covered;
    }
    questFocused_ = true;
    auto& quests = client.quests;
    if (std::exchange(quests.resetScroll, false)) {
        questFirst_ = 0;
        questScrollDrag_ = false;
    }
    if (std::exchange(quests.resetDescription, false)) questDescriptionFirst_ = 0;
    if (blocked) {
        questDragging_ = questScrollDrag_ = false;
        questPressed_ = questHint_ = -1;
        return false;
    }
    if (!covered) {
        questHint_ = -1;
        return false;
    }
    const auto view = cui_viewport_size(ui);
    const HudPoint base({0,0,view.x,view.y});
    if (base.scale <= 0) return false;
    const auto local = base.ToLocal(cui_mouse_pos(ui));
    const auto previous = std::exchange(questPreviousPointer_, local);
    const int mouseX = static_cast<int>(local.x), mouseY = static_cast<int>(local.y);
    const auto hit = [&](float x, float y, float w, float h) {
        return Contains({x,y,w,h}, local);
    };
    const auto lines = QuestLines(quests);
    questLast_ = (std::max)(0, static_cast<int>(lines.size()) - 1);
    questFirst_ = std::clamp(questFirst_, 0, questLast_);
    const auto scroll = [&](int step) { questFirst_ = std::clamp(questFirst_ + step, 0, questLast_); };
    if (fallback && hit(questX_,questY_,340,440))
        ScrollQuest(quests,local,-static_cast<int>(std::lround(cui_take_mouse_scroll(ui).y)));

    int button = -1;
    for (int i = 0; i < 2; ++i) {
        const auto& [x,y] = questButtonOffsets_[i];
        const auto& frame = questButtons_[i][0];
        if (hit(questX_+x,questY_+y,frame.width,frame.height)) button = i;
    }
    const bool down = cui_mouse_clicked(ui, CUI_MOUSE_LEFT), up = cui_mouse_released(ui, CUI_MOUSE_LEFT);
    const int thumbY = QuestThumbY();
    const int trackHeight = 182 + questThumb_.height;
    const int trackTop = questY_ + 160;
    if (questScrollDrag_) {
        if (!up && local.y != previous.y)
            questFirst_ = questLast_ * std::clamp(mouseY - trackTop, 0, trackHeight) / trackHeight;
        if (up) questScrollDrag_ = false;
    } else if (down && hit(questX_ + 272, trackTop, questThumb_.width, trackHeight)) {
        if (mouseY >= thumbY && mouseY < thumbY + questThumb_.height) questScrollDrag_ = true;
        else scroll(mouseY < thumbY + 7 ? -5 : 5);
    } else if (down && hit(questX_,questY_,340,440)) {
        questPressed_ = button;
        const bool list = hit(questX_+65,questY_+90,200,15*19);
        if (list) {
            const int index = questFirst_ + (mouseY - questY_ - 90) / 15;
            if (index < static_cast<int>(lines.size())) {
                const auto& line = lines[index];
                client.SelectQuest(line.parent, line.child ? line.record->id : -1);
            }
        } else if (button < 0) {
            questDragging_ = true;
            questGrabX_ = mouseX - questX_;
            questGrabY_ = mouseY - questY_;
        }
    } else if (questDragging_ && !up) {
        questX_ = mouseX - questGrabX_;
        questY_ = mouseY - questGrabY_;
        if (questY_ + 440 > 570) questY_ = 565 - 440;
    }
    if (up) {
        const auto pressed = std::exchange(questPressed_, -1);
        questDragging_ = false;
        if (button >= 0 && pressed == button) {
            if (client.audioOptions.effects && std::filesystem::exists(clickSound_))
                client.audio.PlayEffect(clickSound_.c_str(), client.audioOptions);
            if (button == 1) {
                ToggleQuest(client);
                return covered;
            }
            if (quests.child < 0) {
                if (auto* selected = quests.Find(quests.parent)) client.ExpandQuest(*selected, false);
            }
        }
    }
    questHint_ = button >= 0 && button != questPressed_ && !questDragging_ && questHints_[button].texture ? button : -1;
    if (questHint_ >= 0)
        questHintDrawHeight_ = hudHint_.Advance(ui, questHintMessages_[questHint_], questHints_[questHint_].height);
    return covered;
}

void Dialogs::Assets::DrawQuestPanel(cui_ctx* ui, LoginClient& client)
{
    auto& quests = client.quests;
    const auto lines = QuestLines(quests);
    questLast_ = (std::max)(0, static_cast<int>(lines.size()) - 1);
    questFirst_ = std::clamp(questFirst_,0,questLast_);
    RefreshQuestText(ui,quests,lines);
    const auto view = cui_viewport_size(ui);
    cui_overlay_begin(ui, {0,0,view.x,view.y});
    auto* self = this;
    cui_canvas_paint(ui, CUI_FILL, CUI_FILL, [](cui_ctx* ui, cui_rect rect, void* model) {
        (*static_cast<Assets**>(model))->PaintQuests(ui, rect);
    }, &self, sizeof self);
    cui_box_end(ui);
}

void Dialogs::Assets::PostQuestNotices(LoginClient& client, const Assets* assets)
{
    for (const int id : client.quests.notices) {
        auto caption = assets ? assets->Message(id) : std::string{};
        if (caption.empty()) caption = id == 1432 ? "New quest step." : "New quest.";
        client.chat.Append(100,caption,0xffffff,0x0000ff);
    }
    client.quests.notices.clear();
}

void Dialogs::Assets::RefreshQuestText(cui_ctx* ui, mir3_cui::QuestPanel& quests, const std::vector<QuestLine>& lines)
{
    for (int row = 0; row < static_cast<int>(questText_.size()); ++row) {
        auto& slot = questText_[row];
        const int index = questFirst_ + row;
        const auto line = index < static_cast<int>(lines.size()) ? lines[index] : QuestLine{};
        const auto title = line.record ? std::string_view(line.record->title) : std::string_view{};
        const int width = line.child ? 190 : 200;
        if (slot.text != title || slot.width != width) {
            Destroy(ui, slot.image);
            slot.text = title;
            slot.width = width;
            if (!slot.text.empty()) slot.image = Raster(ui, slot.text, width, 15, false, mir3::TextFont::normal);
        }
        if (line.record) {
            const bool selected = quests.parent == line.parent &&
                (line.child ? quests.child == line.record->id : quests.child < 0);
            slot.color = line.record->completed
                ? cui_color{static_cast<std::uint8_t>(selected ? 50 : 100),26,25,255}
                : cui_color{static_cast<std::uint8_t>(selected ? 200 : 150),25,25,255};
        }
    }
    if (std::exchange(quests.resetDescription, false)) questDescriptionFirst_ = 0;
    const auto* detail = quests.Description();
    const auto description = detail ? std::string_view(*detail) : std::string_view{};
    if (description != questDescription_) {
        questDescription_ = description;
        for (auto& image : questDescriptionRows_) Destroy(ui, image);
        questDescriptionRows_.clear();
        for (const auto& line : mir3::wrap_quest_detail(questDescription_)) {
            if (questDescriptionRows_.size() == 6) break;
            questDescriptionRows_.push_back(Raster(ui, line, 165, 15, false, mir3::TextFont::normal));
        }
    }
}

void Dialogs::Assets::PaintQuests(cui_ctx* ui, cui_rect rect) const
{
    const HudPoint base(rect);
    const int x = questX_, y = questY_;
    const cui_color white{255,255,255,255};
    const auto image = [&](const Image& art, int left, int top, cui_color tint) {
        if (art.texture) cui_draw_image(ui, art.texture, base.Rect(left, top, art.width, art.height), {0,0,1,1}, {}, tint);
    };
    cui_push_clip(ui, base.Rect(0,0,800,600));
    image(questBackground_, x - (questBackground_.width - 340) / 2, y - (questBackground_.height - 440) / 2, {255,255,255,220});
    image(questThumb_, x+272, QuestThumbY(), white);
    for (int row = 0; row < static_cast<int>(questText_.size()); ++row) {
        const auto& slot = questText_[row];
        image(slot.image, x + (slot.width == 190 ? 75 : 65), y + 90 + 15*row, slot.color);
    }
    for (int row = 0; row < 3 && questDescriptionFirst_ + row < static_cast<int>(questDescriptionRows_.size()); ++row)
        image(questDescriptionRows_[questDescriptionFirst_ + row], x+84, y+347+15*row, {0,0,125,255});
    for (int i = 0; i < 2; ++i) {
        const auto& [dx,dy] = questButtonOffsets_[i];
        image(questButtons_[i][questPressed_ == i], x+dx,y+dy,white);
    }
    if (questHint_ >= 0) {
        const auto& [dx,dy] = questButtonOffsets_[questHint_];
        const auto& button = questButtons_[questHint_][0];
        HudHint::Paint(ui, base, questHints_[questHint_], questHintDrawHeight_, x+dx+button.width, y+dy+button.height);
    }
    cui_pop_clip(ui);
}


void Dialogs::Assets::GuildPosition::Grab(cui_vec2 point)
{
    grabX = static_cast<int>(point.x) - x;
    grabY = static_cast<int>(point.y) - y;
    dragging = true;
}

void Dialogs::Assets::GuildPosition::Drag(cui_vec2 point)
{
    x = static_cast<int>(point.x) - grabX;
    y = static_cast<int>(point.y) - grabY;
    if (y + height > 570) y = 565 - height;
}

void Dialogs::Assets::OpenGuild(cui_ctx* ui, mir3::WilArchive& archive)
{
    guildReady_ = dialog_[0].texture && dialog_[1].texture && dialog_[2].texture;
    const auto load = [&](int id) {
        guildFrames_[id] = UploadFrame(ui, archive, id);
        guildReady_ &= guildFrames_[id].texture != 0;
    };
    for (int id : {1300,1225,1326,1327,1328,1241,1242,1245,1246}) load(id);
    for (int i = 0; i < 9; ++i) {
        const auto& spec = mir3::GuildLayout::buttons[i];
        load(spec[0]);
        load(spec[0] + 1);
        guildHints_[i] = MakeHudHint(ui, spec[3]);
    }
    for (int i = 0; i < 3; ++i)
        guildTitles_[i] = Raster(ui, Message(mir3::GuildLayout::title(i)), 112, 25, true, mir3::TextFont::normal);
    for (int i = 0; i < 5; ++i)
        guildEditorLabels_[i] = Raster(ui, Message(1900+i), 420, 16, false,
            i == 0 || i == 3 ? mir3::TextFont::localized_normal : mir3::TextFont::normal);
}

void Dialogs::Assets::LeaveGuild()
{
    guildFocused_ = guildPosition_.dragging = guildEditorPosition_.dragging = guildScroll_.captured = false;
    guildLayout_.pressed = guildPressed_ = guildEditorId_ = -1;
}

bool Dialogs::Assets::GuildBlocksScene(cui_ctx* ui) const
{
    if (guildPosition_.dragging || guildScroll_.captured || guildLayout_.pressed >= 0) return true;
    const auto view = cui_viewport_size(ui);
    const HudPoint base({0,0,view.x,view.y});
    return base.scale > 0 && Contains(guildPosition_.Rect(), base.ToLocal(cui_mouse_pos(ui)));
}

int Dialogs::Assets::GuildRowCount(const mir3_cui::GuildPanel& guild) const
{
    if (guild.tab == 0) return 2 + static_cast<int>(guild.overview.rows.size());
    if (guild.tab == 1) return guild.roster.received ? 3 + static_cast<int>(guild.roster.rows.size()) : 0;
    return static_cast<int>(guild.history.size());
}

bool Dialogs::Assets::GuildHit(cui_vec2 mouse, int id, int x, int y) const
{
    const auto& art = guildFrames_.at(id);
    return Contains({float(x),float(y),float(art.width),float(art.height)}, mouse);
}

void Dialogs::Assets::GuildInput(cui_ctx* ui, LoginClient& client, cui_vec2 mouse, bool down, bool up, bool fallback)
{
    auto& guild = client.guild;
    if (!guild.open) return;
    const int x = guildPosition_.x, y = guildPosition_.y;
    const bool inside = Contains(guildPosition_.Rect(), mouse);
    if (fallback && inside) {
        const int wheel = -static_cast<int>(std::lround(cui_take_mouse_scroll(ui).y));
        guild.first = std::clamp(guild.first + wheel, 0, (std::max)(0, guildCount_ - 1));
    }

    const auto& thumb = guildFrames_.at(1225);
    const auto now = static_cast<std::uint32_t>(cui_time(ui) * 1000);
    const auto scroll = [&](int kind) {
        return guildScroll_.input(kind, static_cast<int>(mouse.x), static_cast<int>(mouse.y), x, y,
                                  thumb.width, thumb.height, guildCount_, guild.first, now);
    };
    const bool moved = mouse.x != guildPointer_.x || mouse.y != guildPointer_.y;
    if ((down && scroll(0)) || (moved && scroll(1) && !up) || (up && scroll(2))) return;

    int hit = -1;
    for (int i = 0; i < 9; ++i) {
        const auto& spec = mir3::GuildLayout::buttons[i];
        if (GuildHit(mouse, spec[0], x+spec[1], y+spec[2])) { hit = i; break; }
    }
    if (down && inside) {
        guildLayout_.pressed = hit;
        if (hit < 0) guildPosition_.Grab(mouse);
    } else if (guildPosition_.dragging && moved && !up) {
        guildPosition_.Drag(mouse);
    }
    if (up) {
        guildPosition_.dragging = false;
        if (guildLayout_.release(hit)) {
            if (client.audioOptions.effects && std::filesystem::exists(clickSound_))
                client.audio.PlayEffect(clickSound_.c_str(), client.audioOptions);
            using Action = mir3_cui::GuildAction;
            if (hit == 0) guild.open = false;
            if (hit >= 1 && hit <= 3) {
                client.RequestGuildPage(hit - 1);
            }
            if (mir3::GuildLayout::denial(hit, guild.overview.manager) == 4000) guild.notice = 4000;
            else if (hit == 7 && !guild.roster.received) guild.notice = 4017;
            else if (hit == 4 || hit == 5 || hit == 7 || hit == 8) {
                client.BeginGuildAction(hit == 4 ? Action::Notice : hit == 5 ? Action::Remove :
                                        hit == 7 ? Action::Ranks : Action::BreakAlliance);
                if (guild.Multiline() && guild.input[0]) {
                    const auto size = std::strlen(guild.input);
                    if (size + 1 < sizeof guild.input) strcat_s(guild.input, "\n");
                }
            }
        }
    }
    if (inside && hit >= 0 && guildLayout_.tooltip(hit) >= 0 && guildHints_[hit].texture) {
        guildHint_ = hit;
        guildHintHeight_ = hudHint_.Advance(ui, mir3::GuildLayout::buttons[hit][3], guildHints_[hit].height);
    }
}

void Dialogs::Assets::GuildModalInput(cui_ctx* ui, LoginClient& client, cui_vec2 mouse, bool down, bool up)
{
    auto& guild = client.guild;
    const bool alert = guild.notice != 0;
    const bool multiline = !alert && guild.Multiline();
    int hit = -1;
    if (multiline) {
        auto& pos = guildEditorPosition_;
        if (GuildHit(mouse,1221,pos.x+546,pos.y+212)) hit = 0;
        if (GuildHit(mouse,1327,pos.x+461,pos.y+12)) hit = 1;
        if (down && Contains(pos.Rect(), mouse) && hit < 0 &&
            !Contains({float(pos.x+46),float(pos.y+54),468,170}, mouse)) pos.Grab(mouse);
        else if (pos.dragging) pos.Drag(mouse);
        if (up) pos.dragging = false;
    } else {
        const int x = 400 - dialog_[0].width / 2, y = 246 - dialog_[0].height / 2;
        if (alert) {
            if (Contains({float(x+137),float(y+188),float(dialog_[1].width),float(dialog_[1].height)}, mouse)) hit = 1;
        } else {
            if (GuildHit(mouse,1241,x+41,y+188)) hit = 1;
            if (GuildHit(mouse,1245,x+234,y+188)) hit = 0;
        }
        if (guildPressed_ >= 0 && hit != guildPressed_) guildPressed_ = -1;
        const bool tab = cui_key_down(ui, CUI_KEY_TAB);
        if (!alert && !guildTabDown_ && (tab || cui_key_pressed(ui,CUI_KEY_TAB)))
            guildCancelSelected_ = !guildCancelSelected_;
        guildTabDown_ = tab;
    }
    if (down) guildPressed_ = hit;
    const bool click = up && hit >= 0 && hit == guildPressed_;
    if (up) guildPressed_ = -1;
    const bool key = alert && (cui_key_pressed(ui,CUI_KEY_ENTER) || cui_key_pressed(ui,CUI_KEY_SPACE));
    if (!click && !key) return;
    if (click && client.audioOptions.effects && std::filesystem::exists(clickSound_))
        client.audio.PlayEffect(clickSound_.c_str(), client.audioOptions);
    if (alert) guild.notice = 0;
    else if (hit == 1) client.SubmitGuildAction();
    else guild.Begin(mir3_cui::GuildAction::None);
    cui_request_redraw(ui);
}

bool Dialogs::Assets::HandleGuildInput(cui_ctx* ui, LoginClient& client, bool blocked, bool fallback)
{
    auto& guild = client.guild;
    if (fallback && !blocked && guildFocused_ && !client.chat.composing && cui_shortcut(ui,0,CUI_KEY_F))
        client.RequestGuildPage(0);
    if (guildRevision_ != guild.overviewRevision) {
        guildRevision_ = guild.overviewRevision;
        guildScroll_.captured = false;
    }
    guildFocused_ = true;
    guildHint_ = -1;
    const bool covered = GuildBlocksScene(ui);
    if (blocked) {
        guildPosition_.dragging = guildScroll_.captured = false;
        guildLayout_.pressed = -1;
        return false;
    }
    const auto view = cui_viewport_size(ui);
    const HudPoint base({0,0,view.x,view.y});
    if (!covered || base.scale <= 0) return false;
    guildCount_ = GuildRowCount(guild);
    guild.first = std::clamp(guild.first,0,(std::max)(0,guildCount_-1));
    const auto mouse = base.ToLocal(cui_mouse_pos(ui));
    GuildInput(ui,client,mouse,cui_mouse_clicked(ui,CUI_MOUSE_LEFT),cui_mouse_released(ui,CUI_MOUSE_LEFT),fallback);
    guildPointer_ = mouse;
    return true;
}

void Dialogs::Assets::DrawGuildPanel(cui_ctx* ui, LoginClient& client, bool drawPanel, bool drawDialog)
{
    auto& guild = client.guild;
    const auto view = cui_viewport_size(ui);
    const HudPoint base({0,0,view.x,view.y});
    if (base.scale <= 0) return;
    const auto mouse = base.ToLocal(cui_mouse_pos(ui));
    const bool down = cui_mouse_clicked(ui, CUI_MOUSE_LEFT), up = cui_mouse_released(ui, CUI_MOUSE_LEFT);
    const bool editorChanged = drawDialog && guildEditorId_ != guild.editor;
    if (editorChanged) {
        guildEditorId_ = guild.editor;
        guildCancelSelected_ = guildTabDown_ = false;
        guildPressed_ = -1;
    }
    const bool blocked = portraitNotice_ || client.showSignOut || client.returnToCharactersAt;
    if (drawDialog && !blocked && (guild.notice || guild.action != mir3_cui::GuildAction::None)) {
        guildPosition_.dragging = guildScroll_.captured = false;
        guildLayout_.pressed = -1;
        GuildModalInput(ui, client, mouse, down, up);
    }
    guildLayout_.tab = guild.tab;
    guildCount_ = GuildRowCount(guild);
    guild.first = std::clamp(guild.first,0,(std::max)(0,guildCount_-1));
    guildDrawPanel_ = guild.open;
    guildDrawAlert_ = guild.notice != 0;
    guildDrawEditor_ = !guildDrawAlert_ && guild.Multiline();
    guildDrawPrompt_ = !guildDrawAlert_ && !guildDrawEditor_ && guild.action != mir3_cui::GuildAction::None;
    if (!guildDrawPanel_ && !guildDrawAlert_ && !guildDrawEditor_ && !guildDrawPrompt_) {
        if (!guild.requested) LeaveGuild();
        return;
    }
    RefreshGuildText(ui, guild);
    cui_overlay_begin(ui, {0,0,view.x,view.y});
    const struct { const Assets* assets; bool panel, dialog; } paint{this,drawPanel,drawDialog};
    cui_canvas_paint(ui, CUI_FILL, CUI_FILL, [](cui_ctx* ui, cui_rect rect, void* model) {
        const auto& data = *static_cast<decltype(paint)*>(model);
        data.assets->PaintGuild(ui,rect,data.panel,data.dialog);
    }, &paint, sizeof paint);
    if (drawDialog && !blocked && (guildDrawEditor_ || guildDrawPrompt_)) {
        const auto field = guildDrawEditor_ ? base.Rect(guildEditorPosition_.x+46,guildEditorPosition_.y+54,468,170) :
            base.Rect(400-dialog_[0].width/2+24,246-dialog_[0].height/2+228,309,15);
        cui_overlay_begin(ui, field);
        cui_push_id_int(ui, guild.editor);
        const bool enter = !guildDrawEditor_ && cui_key_pressed(ui,CUI_KEY_ENTER);
        if (enter || editorChanged || guildEditorId_ != guild.editor ||
            (down && !Contains(field, cui_mouse_pos(ui)))) cui_text_field_focus(ui, "native-guild-text", false);
        auto options = ModalEditorOptions(field.h,base.scale);
        options.keep_tab = !guildDrawEditor_;
        options.byte_limit = guildDrawEditor_ ? 3999 : 90;
        options.measure_bytes = AnsiBytes;
        if (guildDrawEditor_) cui_textarea_ex(ui, "native-guild-text", guild.input, sizeof guild.input, &options);
        else cui_input_ex(ui, "native-guild-text", guild.input, sizeof guild.input, &options);
        if (enter) {
            if (guildCancelSelected_) guild.Begin(mir3_cui::GuildAction::None);
            else client.SubmitGuildAction();
            cui_request_redraw(ui);
        }
        cui_pop_id(ui);
        cui_box_end(ui);
    }
    cui_box_end(ui);
}


void Dialogs::Assets::RefreshGuildText(cui_ctx* ui, const mir3_cui::GuildPanel& guild)
{
    const auto name = ConvertText(guild.overview.name, 936, CP_UTF8);
    if (guildNameText_ != name) {
        guildNameText_ = name;
        Destroy(ui, guildName_);
        guildName_ = Raster(ui, name, 213, 24, true, mir3::TextFont::normal);
        const auto metrics = mir3::raster_text(name, 300, 24, false, mir3::TextFont::normal);
        guildLineHeight_ = metrics.text_height > 0 ? metrics.text_height : 15;
    }
    for (int i = 0; i < static_cast<int>(guildRows_.size()); ++i) {
        const int index = guild.first + i;
        std::string line;
        bool heading = false;
        if (index < guildCount_) {
            if (guild.tab == 0) {
                if (index == 0) line = Message(4003);
                else if (index == 1) line = " ";
                else if (const auto id = guild.overview.headingIds.find(index-2); id != guild.overview.headingIds.end()) {
                    line = Message(id->second);
                    heading = true;
                } else line = ConvertText(guild.overview.rows[index-2],936,CP_UTF8);
            } else if (guild.tab == 1) {
                if (index == 0) line = Message(4003);
                else if (index == 1) line = " ";
                else if (index == 2) { line = Message(4007); heading = true; }
                else line = ConvertText(guild.roster.rows[index-3],936,CP_UTF8);
            } else {
                const auto& entry = guild.history[index];
                line = entry.messageId ? Message(entry.messageId) : entry.text;
                if (line.empty() && entry.messageId) line = mir3_cui::GuildNoticeText(entry.messageId);
            }
        }
        auto& row = guildRows_[i];
        if (row.text != line || row.width != guildLineHeight_) {
            Destroy(ui,row.image);
            row.text = std::move(line);
            row.width = guildLineHeight_;
            if (!row.text.empty()) row.image = Raster(ui,row.text,514,guildLineHeight_,false,mir3::TextFont::normal);
        }
        row.color = guild.tab == 2 ? cui_color{0,255,0,255} : heading ? cui_color{255,150,0,255} : cui_color{255,255,255,255};
    }
    guildThumbY_ = mir3::GuildScroll::thumb_y(guild.first,guildCount_,guildPosition_.y);

    using Action = mir3_cui::GuildAction;
    auto caption = guildDrawAlert_ ? Message(guild.notice) : guildDrawPrompt_ ?
        guild.action == Action::Remove ? "'" + name + "' " + Message(4001) :
        guild.action == Action::BreakAlliance ? Message(4002) : Message(1426) : std::string{};
    if (guildDrawPrompt_ && guild.action == Action::Add) {
        if (const auto marker = caption.find("%s"); marker != std::string::npos) caption.replace(marker,2,name);
    }
    if (guildDrawAlert_ && caption.empty()) caption = mir3_cui::GuildNoticeText(guild.notice);
    const int captionWidth = guildDrawAlert_ ? 308 : 278;
    if (guildCaptionText_ != caption || (!caption.empty() && guildCaption_.width != captionWidth)) {
        guildCaptionText_ = std::move(caption);
        Destroy(ui,guildCaption_);
        if (!guildCaptionText_.empty()) guildCaption_ = Raster(ui,guildCaptionText_,captionWidth,
            guildDrawAlert_ ? 95 : 75,true,mir3::TextFont::dialog);
    }
    if (guildErrorText_ != guild.error) {
        guildErrorText_ = guild.error;
        Destroy(ui,guildError_);
        if (!guildErrorText_.empty()) guildError_ = Raster(ui,guildErrorText_,460,17,false,mir3::TextFont::normal);
    }
    guildDrawAnnouncement_ = guild.action == Action::Notice;
}

void Dialogs::Assets::PaintGuild(cui_ctx* ui, cui_rect rect, bool drawPanel, bool drawDialog) const
{
    const HudPoint base(rect);
    const cui_color white{255,255,255,255}, heading{255,200,150,255}, notice{255,250,200,255};
    const auto image = [&](const Image& art,int x,int y,cui_color tint) {
        if (art.texture) cui_draw_image(ui,art.texture,base.Rect(x,y,art.width,art.height),{0,0,1,1},{},tint);
    };
    const auto sprite = [&](int id,int x,int y) { image(guildFrames_.at(id),x,y,white); };
    const auto skin = [&](int id,const GuildPosition& pos) {
        const auto& art = guildFrames_.at(id);
        image(art,pos.x-(art.width-pos.width)/2,pos.y-(art.height-pos.height)/2,white);
    };
    cui_push_clip(ui,base.Rect(0,0,800,600));
    if (drawPanel && guildDrawPanel_) {
        const int x = guildPosition_.x, y = guildPosition_.y;
        skin(1300,guildPosition_);
        image(guildName_,x+340,y+10,heading);
        image(guildTitles_[guildLayout_.tab],x+45,y+10,heading);
        for (int i = 0; i < static_cast<int>(guildRows_.size()); ++i) {
            const auto& row = guildRows_[i];
            const int top = y + 60 + (guildLineHeight_+5)*i;
            if (guildLayout_.tab == 2) image(row.image,x+34,top-1,{10,20,10,255});
            image(row.image,x+35,top,row.color);
        }
        sprite(1225,x+557,guildThumbY_);
        for (int i = 0; i < 9; ++i) {
            const auto& spec = mir3::GuildLayout::buttons[i];
            sprite(guildLayout_.frames(i)[0],x+spec[1],y+spec[2]);
        }
        if (guildHint_ >= 0 && !guildDrawAlert_ && !guildDrawPrompt_ && !guildDrawEditor_) {
            const auto& spec = mir3::GuildLayout::buttons[guildHint_];
            const auto& button = guildFrames_.at(spec[0]);
            HudHint::Paint(ui,base,guildHints_[guildHint_],guildHintHeight_,x+spec[1]+button.width,y+spec[2]+button.height);
        }
    }
    if (drawDialog && guildDrawEditor_) {
        const int x = guildEditorPosition_.x, y = guildEditorPosition_.y;
        skin(1326,guildEditorPosition_);
        image(guildEditorLabels_[guildDrawAnnouncement_ ? 3 : 0],x+70,y+20,heading);
        if (guildDrawAnnouncement_) {
            image(guildEditorLabels_[4],x+154,y+21,{50,50,50,255});
            image(guildEditorLabels_[4],x+154,y+22,heading);
        } else {
            image(guildEditorLabels_[1],x+154,y+14,heading);
            image(guildEditorLabels_[2],x+154,y+28,heading);
        }
        sprite(1221+(guildPressed_==0),x+546,y+212);
        sprite(1327+(guildPressed_==1),x+461,y+12);
        image(guildError_,x+46,y+226,notice);
    } else if (drawDialog && (guildDrawAlert_ || guildDrawPrompt_)) {
        const int x = 400-dialog_[0].width/2, y = 246-dialog_[0].height/2;
        image(dialog_[0],x,y,white);
        if (guildDrawAlert_) {
            image(guildCaption_,x+23,y+84,notice);
            image(dialog_[guildPressed_==1 ? 2 : 1],x+137,y+188,white);
        } else {
            image(guildCaption_,x+43,y+84,notice);
            sprite(1241+(guildPressed_==1),x+41,y+188);
            sprite(1245+(guildPressed_==0),x+234,y+188);
            image(guildError_,x+23,y+153,notice);
        }
    }
    cui_pop_clip(ui);
}


void Dialogs::Assets::OpenNpc(cui_ctx* ui, mir3::WilArchive& archive)
{
    npcReady_ = dialog_[0].texture && selectionButtons_.at(1241).texture &&
        selectionButtons_.at(1242).texture && selectionButtons_.at(1245).texture && selectionButtons_.at(1246).texture;
    constexpr int parts[] = {1353,1351,1352};
    for (int i = 0; i < 3; ++i) {
        auto art = archive.frame(parts[i]);
        if (!art) { npcReady_ = false; continue; }
        npcBackgroundParts_[i] = std::move(*art);
    }
    for (int id : {1221,1222,1355,1356,1357,1358}) {
        npcButtons_[id] = UploadFrame(ui, archive, id);
        npcReady_ &= npcButtons_[id].texture != 0;
    }
    npcCloseHint_ = MakeHudHint(ui,1052);
}

void Dialogs::Assets::BuildNpcBackground(cui_ctx* ui)
{
    auto& result = npcBackgrounds_[npcStrips_ - 1];
    if (result.texture) return;
    constexpr int width = 386;
    std::vector<std::uint8_t> pixels(static_cast<std::size_t>(width) * npcHeight_ * 4);
    const auto compose = [&](const mir3::WilFrame& part, int top) {
        for (int y = 0; y < part.height && y + top < npcHeight_; ++y) {
            for (int x = 0; x < part.width && x < width; ++x) {
                const auto* src = part.pixels.rgba.data() + (static_cast<std::size_t>(y) * part.width + x) * 4;
                auto* dst = pixels.data() + (static_cast<std::size_t>(y + top) * width + x) * 4;
                if (!src[3]) continue;
                if (src[3] == 255 || !dst[3]) {
                    std::copy_n(src,4,dst);
                    continue;
                }
                const unsigned under = dst[3] * (255 - src[3]);
                const unsigned alpha = src[3] * 255 + under;
                for (int c = 0; c < 3; ++c)
                    dst[c] = static_cast<std::uint8_t>((src[c] * src[3] * 255 + dst[c] * under + alpha / 2) / alpha);
                dst[3] = static_cast<std::uint8_t>((alpha + 127) / 255);
            }
        }
    };
    compose(npcBackgroundParts_[0],0);
    for (int i = 0; i < npcStrips_; ++i) compose(npcBackgroundParts_[1],160 + 20*i);
    compose(npcBackgroundParts_[2],160 + 20*npcStrips_);
    result = Upload(ui,width,npcHeight_,pixels);
}

void Dialogs::Assets::LayoutNpc(cui_ctx* ui, const mir3_cui::NpcDialogue& npc)
{
    for (auto& part : npcParts_) {
        Destroy(ui,part.normal);
        Destroy(ui,part.underline);
    }
    npcParts_.clear();
    std::vector<int> ansiLengths(static_cast<std::size_t>(npc.explicitBreaks) + 1);
    int line = 0, explicitLine = 0, x = 0;
    for (int i = 0; i < static_cast<int>(npc.rows.size()); ++i) {
        const auto& row = npc.rows[i];
        if (row.line != explicitLine) {
            line += row.line - explicitLine;
            explicitLine = row.line;
            x = 0;
        }
        ansiLengths[row.line] += AnsiBytes(row.label.data(),static_cast<int>(row.label.size()));
        std::string_view text(row.label);
        const int option = row.command.empty() ? -1 : i;
        while (!text.empty()) {
            const int limit = option < 0 && line < 7 ? 198 : 305;
            const auto whole = text.size() <= 1024 ? mir3::raster_text(text,1,1,false,mir3::TextFont::normal) : mir3::NativeText{};
            const bool fits = x + whole.text_width <= limit && whole.text_height > 0;
            std::size_t length = fits ? text.size() : 0;
            int width = whole.text_width, height = whole.text_height;
            while (length < text.size()) {
                auto next = length + 1;
                while (next < text.size() && (static_cast<unsigned char>(text[next]) & 0xc0) == 0x80) ++next;
                const auto measured = mir3::raster_text(text.substr(0,next),1,1,false,mir3::TextFont::normal);
                length = next;
                width = measured.text_width;
                height = measured.text_height;
                if (x + width >= 305) break;
            }
            npcParts_.push_back({std::string(text.substr(0,length)),35+x,line,width,height,option,row.color});
            text.remove_prefix(length);
            if (!fits && x + width >= 305) { ++line; x = 0; }
            else x += width;
        }
    }
    int estimated = 0;
    for (int i = 0; i < static_cast<int>(ansiLengths.size()); ++i)
        if (ansiLengths[i] || i < npc.explicitBreaks) estimated += (std::max)(1,(ansiLengths[i]+49)/50);
    line += npc.explicitBreaks - explicitLine;
    npcStrips_ = std::clamp(estimated - 9,1,13);
    npcHeight_ = 204 + 20*npcStrips_;
    npcFirst_ = 0;
    npcScrollEnabled_ = estimated > 22;
    npcScrollLimit_ = npcScrollEnabled_ ? (std::max)(0,line-21) : 0;
    npcOptionPressed_ = -1;
    npcClosePressed_ = false;
    npcArrowPressed_ = 0;
    npcEditor_ = -1;
    npcPromptPressed_ = -1;
    if (npcActor_ != npc.actor) {
        npcActor_ = npc.actor;
        npcArrowFrame_ = 0;
        npcDownLit_ = false;
    }
    BuildNpcBackground(ui);
}

bool Dialogs::Assets::NpcHit(cui_vec2 point, int id, int x, int y) const
{
    const auto& image = npcButtons_.at(id);
    return Contains({float(x),float(y),float(image.width),float(image.height)},point);
}

int Dialogs::Assets::NpcOptionAt(cui_vec2 point) const
{
    int option = -1, rectangles = 0;
    for (const auto& part : npcParts_) {
        if (part.option < 0 || part.line < npcFirst_ || part.line > npcFirst_ + 21) continue;
        if (part.option != option) { option = part.option; rectangles = 0; }
        if (++rectangles <= 5 && Contains({float(part.x),float(40+(part.height+5)*(part.line-npcFirst_)),
            float(part.width),float(part.height)},point)) return option;
    }
    return -1;
}

bool Dialogs::Assets::NpcBlocksScene(cui_ctx* ui, const LoginClient& client, bool hitOnly) const
{
    if (!hitOnly && (!client.npcDialogue.prompt.command.empty() || npcClosePressed_ || npcArrowPressed_ || npcOptionPressed_>=0))
        return true;
    const auto view = cui_viewport_size(ui);
    const HudPoint base({0,0,view.x,view.y});
    return base.scale > 0 && Contains({0,0,386,float(client.npcDialogue.resetScroll ? 464 : npcHeight_)},
                                     base.ToLocal(cui_mouse_pos(ui)));
}

void Dialogs::Assets::NpcInput(cui_ctx* ui, LoginClient& client, cui_vec2 point, bool down, bool up)
{
    const auto& npc = client.npcDialogue;
    const bool close = NpcHit(point,1221,346,npcHeight_-42);
    npcCloseHover_ = close && !npcClosePressed_ && npcCloseHint_.texture;
    if (npcCloseHover_) npcHintHeight_ = hudHint_.Advance(ui,1052,npcCloseHint_.height);
    if (!down && !up) return;
    if (close || npcClosePressed_) {
        if (up && npcClosePressed_ && close && npc.canClose) client.CloseNpcDialogue();
        npcClosePressed_ = down && !up && close;
        return;
    }
    if (npcScrollEnabled_) {
        const int arrow = npcFirst_ > 0 && NpcHit(point,1357,183,npcHeight_-48) ? -1 :
            npcFirst_ < npcScrollLimit_ && NpcHit(point,1355,183,npcHeight_-30) ? 1 : 0;
        if (arrow || npcArrowPressed_) {
            if (up && arrow == npcArrowPressed_) npcFirst_ = std::clamp(npcFirst_ + arrow,0,npcScrollLimit_);
            npcArrowPressed_ = down && !up ? arrow : 0;
            npcOptionPressed_ = -1;
            return;
        }
    }
    const int option = NpcOptionAt(point);
    npcOptionPressed_ = down && !up ? option : -1;
    if (up && option >= 0) {
        client.ChooseNpcOption(npc.rows[option]);
        if (client.npcDialogue.prompt.command == "@@buildguildnow") client.npcDialogue.prompt.label = Message(2000);
    }
}

void Dialogs::Assets::NpcPromptInput(cui_ctx* ui, LoginClient& client, cui_vec2 point, bool down, bool up)
{
    const int x = 400-dialog_[0].width/2, y = 246-dialog_[0].height/2;
    const auto hit = [&](int id,int dx) {
        const auto& image = selectionButtons_.at(id);
        return Contains({float(x+dx),float(y+188),float(image.width),float(image.height)},point);
    };
    const int button = hit(1241,41) ? 0 : hit(1245,234) ? 1 : -1;
    if (npcPromptPressed_ >= 0 && npcPromptPressed_ != button) npcPromptPressed_ = -1;
    if (down) npcPromptPressed_ = button;
    const bool click = up && button >= 0 && button == npcPromptPressed_;
    if (up) npcPromptPressed_ = -1;
    const bool tab = cui_key_down(ui,CUI_KEY_TAB);
    if (!npcTabDown_ && (tab || cui_key_pressed(ui,CUI_KEY_TAB))) npcPromptCancel_ = !npcPromptCancel_;
    npcTabDown_ = tab;
    if (click) {
        if (client.audioOptions.effects && std::filesystem::exists(clickSound_))
            client.audio.PlayEffect(clickSound_.c_str(),client.audioOptions);
        client.EndNpcPrompt(button == 0);
        cui_request_redraw(ui);
    }
}


void Dialogs::Assets::DrawNpcPanel(cui_ctx* ui, LoginClient& client, bool interactive, int wheel)
{
    auto& npc = client.npcDialogue;
    const auto view = cui_viewport_size(ui);
    const HudPoint base({0,0,view.x,view.y});
    if (base.scale <= 0) return;
    if (std::exchange(npc.resetScroll,false) || npcActor_ != npc.actor) LayoutNpc(ui,npc);
    auto point = base.ToLocal(cui_mouse_pos(ui));
    const bool down = interactive && cui_mouse_clicked(ui,CUI_MOUSE_LEFT), up = interactive && cui_mouse_released(ui,CUI_MOUSE_LEFT);
    npcCloseHover_ = false;
    const bool hadPrompt = !npc.prompt.command.empty();
    if (wheel && !hadPrompt && Contains({0,0,386,float(npcHeight_)},point)) {
        npcFirst_=std::clamp(npcFirst_+wheel,0,npcScrollLimit_);
        npcOptionPressed_=-1;
        npcArrowPressed_=0;
    }
    if (interactive && !hadPrompt) {
        if (!down && !up && (point.x != npcPointer_.x || point.y != npcPointer_.y)) {
            npcClosePressed_ = false;
            npcArrowPressed_ = 0;
            npcOptionPressed_ = -1;
        }
        NpcInput(ui,client,point,down,up);
    } else if (!interactive) {
        npcClosePressed_ = false;
        npcArrowPressed_ = 0;
        npcOptionPressed_ = -1;
    }
    if (npcEditor_ != npc.editor) {
        npcPromptCancel_ = npcTabDown_ = false;
        npcPromptPressed_ = -1;
    }
    if (interactive && hadPrompt) NpcPromptInput(ui,client,point,down,up);
    npcPointer_ = point;
    if (!npc.actor) return;
    npcPromptVisible_ = !npc.prompt.command.empty();
    npcHovered_ = interactive && !npcPromptVisible_ ? NpcOptionAt(point) : -1;
    if (npcFirst_ < npcScrollLimit_) {
        if (npcArrowFrame_ == 16) {
            npcArrowFrame_ = 0;
            npcDownLit_ = !npcDownLit_;
        }
        ++npcArrowFrame_;
        cui_request_wakeup(ui,0.016f);
    }
    for (auto& part : npcParts_) {
        if (part.line < npcFirst_ || part.line > npcFirst_ + 21 || part.width <= 0 || part.height <= 0) continue;
        if (!part.normal.texture)
            part.normal = Raster(ui,part.text,part.width,part.height,false,mir3::TextFont::normal);
        if (part.option >= 0 && part.option == npcHovered_ && !part.underline.texture)
            part.underline = Raster(ui,part.text,part.width,part.height,false,mir3::TextFont::npc_link);
    }
    if (npcCaptionText_ != npc.prompt.label) {
        npcCaptionText_ = npc.prompt.label;
        Destroy(ui,npcCaption_);
        if (!npcCaptionText_.empty()) npcCaption_ = Raster(ui,npcCaptionText_,278,75,true,mir3::TextFont::dialog);
    }
    npcPromptTooLong_ = npc.answerTooLong;
    if (npcPromptTooLong_ && !npcPromptError_.texture)
        npcPromptError_ = Raster(ui,"The answer is too long or contains unsupported characters.",308,24,true,mir3::TextFont::dialog);
    cui_overlay_begin(ui,{0,0,view.x,view.y});
    auto* self = this;
    cui_canvas_paint(ui,CUI_FILL,CUI_FILL,[](cui_ctx* ui,cui_rect rect,void* model) {
        (*static_cast<Assets**>(model))->PaintNpc(ui,rect);
    },&self,sizeof self);
    if (npcPromptVisible_ && interactive) {
        const int x = 400-dialog_[0].width/2+24, y = 246-dialog_[0].height/2+228;
        const auto field = base.Rect(x,y,309,15);
        cui_overlay_begin(ui,field);
        cui_push_id_int(ui,npc.editor);
        const bool enter = cui_key_pressed(ui,CUI_KEY_ENTER);
        if (enter || npcEditor_ != npc.editor || (down && !Contains(field,cui_mouse_pos(ui))))
            cui_text_field_focus(ui,"npc-answer",false);
        npcEditor_ = npc.editor;
        auto options = ModalEditorOptions(field.h,base.scale);
        options.password = npc.prompt.command.starts_with("@@*");
        options.password_mask = '*';
        options.byte_limit = client.NpcAnswerLimit();
        options.measure_bytes = AnsiBytes;
        cui_input_ex(ui,"npc-answer",npc.answer,sizeof npc.answer,&options);
        if (enter) {
            client.EndNpcPrompt(!npcPromptCancel_);
            cui_request_redraw(ui);
        }
        cui_pop_id(ui);
        cui_box_end(ui);
    }
    cui_box_end(ui);
}

void Dialogs::Assets::PaintNpc(cui_ctx* ui, cui_rect rect) const
{
    const HudPoint base(rect);
    const cui_color white{255,255,255,255}, caption{255,250,200,255};
    const auto image = [&](const Image& art,int x,int y,cui_color tint) {
        if (art.texture) cui_draw_image(ui,art.texture,base.Rect(x,y,art.width,art.height),{0,0,1,1},{},tint);
    };
    const auto sprite = [&](int id,int x,int y) { image(npcButtons_.at(id),x,y,white); };
    cui_push_clip(ui,base.Rect(0,0,800,600));
    image(npcBackgrounds_[npcStrips_-1],0,0,{180,180,180,220});
    sprite(npcClosePressed_ ? 1222 : 1221,346,npcHeight_-42);
    if (npcScrollEnabled_) {
        sprite(npcArrowPressed_ == -1 ? 1358 : 1357,183,npcHeight_-48);
        if (npcFirst_ < npcScrollLimit_ && npcDownLit_)
            sprite(npcArrowPressed_ == 1 ? 1356 : 1355,183,npcHeight_-30);
    }
    constexpr unsigned palette[] = {0,0xff,0x8000,0x8080,0x808080,0x80,0x808000,0x800000,
        0xc0c0c0,0x800080,0xff00,0xff0000,0xffffff,0xff00ff,0xffff00,0xffff,0x10101};
    for (const auto& part : npcParts_) {
        if (part.line < npcFirst_ || part.line > npcFirst_+21) continue;
        const unsigned value = part.option >= 0 ? part.option == npcOptionPressed_ ? 0xff : 0xffff :
            part.color < std::size(palette) ? palette[part.color] : 0xffffff;
        const cui_color tint{static_cast<std::uint8_t>(value),static_cast<std::uint8_t>(value>>8),
                             static_cast<std::uint8_t>(value>>16),255};
        image(part.option >= 0 && part.option == npcHovered_ ? part.underline : part.normal,
              part.x,40+(part.height+5)*(part.line-npcFirst_),tint);
    }
    if (npcCloseHover_ && !npcPromptVisible_) {
        const auto& close = npcButtons_.at(1221);
        HudHint::Paint(ui,base,npcCloseHint_,npcHintHeight_,346+close.width,npcHeight_-42+close.height);
    }
    if (npcPromptVisible_) {
        const int x = 400-dialog_[0].width/2, y = 246-dialog_[0].height/2;
        image(dialog_[0],x,y,white);
        image(npcCaption_,x+43,y+84,caption);
        image(selectionButtons_.at(1241+(npcPromptPressed_==0)),x+41,y+188,white);
        image(selectionButtons_.at(1245+(npcPromptPressed_==1)),x+234,y+188,white);
        if (npcPromptTooLong_) image(npcPromptError_,x+23,y+156,caption);
    }
    cui_pop_clip(ui);
}


void Dialogs::Assets::OpenShop(cui_ctx* ui, mir3::WilArchive& archive, const std::filesystem::path& items)
{
    shopReady_ = dialog_[0].texture && dialog_[1].texture && dialog_[2].texture &&
        selectionButtons_.at(1241).texture && selectionButtons_.at(1242).texture &&
        selectionButtons_.at(1245).texture && selectionButtons_.at(1246).texture;
    for (const int id : {1221,1222,1225,1260,1261,1262,1263,1264,1265,1266,1268,1269,1270,1271,1272,1273}) {
        shopFrames_[id] = UploadFrame(ui,archive,id);
        shopReady_ &= shopFrames_[id].texture != 0;
    }
    inventoryReady_ = dialog_[0].texture && dialog_[1].texture && dialog_[2].texture;
    for (const int id : {1220,1226,1227,1228,1229,1230,1231}) {
        inventoryFrames_[id] = UploadFrame(ui,archive,id);
        inventoryReady_ &= inventoryFrames_[id].texture != 0;
    }
    for (const int id : {1221,1222,1225}) inventoryReady_ &= shopFrames_.at(id).texture != 0;
    for (const int id : {1241,1242,1245,1246}) inventoryReady_ &= selectionButtons_.at(id).texture != 0;
    for (int i = 0; i < static_cast<int>(inventoryTitles_.size()); ++i)
        inventoryTitles_[i] = Raster(ui,Message(1701+i),74,16,false,mir3::TextFont::normal);
    for (int i = 0; i < static_cast<int>(quickFrames_.size()); ++i)
        quickFrames_[i] = UploadFrame(ui,archive,1210+i);
    itemArchive_.open(items);
}

void Dialogs::Assets::CacheItem(cui_ctx* ui, unsigned id)
{
    if (!itemFrames_.contains(id)) itemFrames_[id] = UploadFrame(ui,itemArchive_,static_cast<int>(id));
}

void Dialogs::Assets::SetBagText(cui_ctx* ui, BagText& text, std::string value, int width, int height,
                                bool centered, mir3::TextFont font, bool inverted)
{
    if (text.value == value) return;
    text.value = std::move(value);
    Destroy(ui,text.image);
    if (!text.value.empty()) text.image = Raster(ui,text.value,width,height,centered,font,inverted);
}

void Dialogs::Assets::PostMerchantResult(LoginClient& client) const
{
    for (const auto& result : client.merchantResults) {
        auto message = Message(result.message);
        if (const auto at = message.find("%s"); at != std::string::npos)
            message.replace(at,2,result.item);
        if (const auto at = message.find("%d"); at != std::string::npos)
            message.replace(at,2,std::to_string(result.quantity));
        if (!message.empty()) client.chat.Append(100,message,11861940);
    }
    client.merchantResults.clear();
}

void Dialogs::Assets::LeaveInventory(bool resetView)
{
    inventoryDragging_ = inventoryScrollDrag_ = inventoryClosePress_ = inventoryServicePress_ = false;
    inventoryModalPress_ = inventoryEditor_ = -1;
    inventoryCancel_ = inventoryTabDown_ = false;
    if (resetView) {
        inventoryX_ = 518;
        inventoryY_ = inventoryRow_ = 0;
        inventoryServiceRevision_ = 0;
        quickX_ = 73;
        quickY_ = 413;
        quickDragging_ = quickClosePress_ = quickSkipRelease_ = false;
        quickLastSlot_ = -1;
    }
}

bool Dialogs::Assets::QuickBlocksScene(cui_ctx* ui, const LoginClient& client) const
{
    if (!client.inventory.shortcutsVisible) return false;
    if (quickDragging_ || quickClosePress_ || quickSkipRelease_) return true;
    const auto view = cui_viewport_size(ui);
    const HudPoint base({0,0,view.x,view.y});
    return base.scale > 0 && Contains({float(quickX_),float(quickY_),280,48},base.ToLocal(cui_mouse_pos(ui)));
}

void Dialogs::Assets::QuickInput(cui_ctx* ui, LoginClient& client, cui_vec2 pointer)
{
    const bool down = cui_mouse_clicked(ui,CUI_MOUSE_LEFT), up = cui_mouse_released(ui,CUI_MOUSE_LEFT);
    const auto within = [&](int x,int y,int w,int h) {
        return Contains({float(quickX_+x),float(quickY_+y),float(w),float(h)},pointer);
    };
    const auto& close = quickFrames_[1];
    const bool onClose = within(264,32,close.width,close.height);
    if (quickDragging_) {
        if (up) {
            quickDragging_ = false;
            return;
        }
        quickX_ = static_cast<int>(pointer.x)-quickDragX_;
        quickY_ = static_cast<int>(pointer.y)-quickDragY_;
        if (quickY_+48 > 570) quickY_ = 565-48;
        return;
    }
    int slot = -1;
    for (int i = 0; i < 6; ++i)
        if (within(17+42*i,6,38,38)) { slot = i; break; }
    if (down) {
        quickClosePress_ = onClose;
        const double now = cui_time(ui);
        if (slot >= 0 && slot == quickLastSlot_ && now-quickLastClick_ < 0.30) {
            quickSkipRelease_ = true;
            quickLastSlot_ = -1;
            client.ActivateShortcutSlot(static_cast<std::size_t>(slot),true);
        } else {
            quickLastSlot_ = slot;
            if (slot < 0 && !onClose && within(0,0,280,48)) {
                quickDragging_ = true;
                quickDragX_ = static_cast<int>(pointer.x)-quickX_;
                quickDragY_ = static_cast<int>(pointer.y)-quickY_;
            }
        }
        quickLastClick_ = now;
    }
    if (up) {
        const bool closePressed = std::exchange(quickClosePress_,false);
        const bool used = std::exchange(quickSkipRelease_,false);
        if (closePressed && onClose) ToggleQuick(client);
        else if (!closePressed && !used && slot >= 0) client.ReleaseShortcutSlot(static_cast<std::size_t>(slot));
    }
}

void Dialogs::Assets::ToggleQuick(LoginClient& client)
{
    client.ToggleItemShortcuts();
    if (QuickReady() && client.inventory.shortcutsVisible) RaiseUtility(UtilityPanel::Quick);
    quickDragging_ = quickClosePress_ = quickSkipRelease_ = false;
    quickLastSlot_ = -1;
}

bool Dialogs::Assets::HandleQuickInput(cui_ctx* ui, LoginClient& client, bool blocked)
{
    if (blocked) {
        quickDragging_ = quickClosePress_ = quickSkipRelease_ = false;
        quickLastSlot_ = -1;
        return false;
    }
    if (!QuickBlocksScene(ui,client)) return false;
    const auto view = cui_viewport_size(ui);
    const HudPoint base({0,0,view.x,view.y});
    if (base.scale <= 0) return false;
    QuickInput(ui,client,base.ToLocal(cui_mouse_pos(ui)));
    return true;
}

void Dialogs::Assets::DrawQuickPanel(cui_ctx* ui, LoginClient& client, bool interactive)
{
    if (!client.inventory.shortcutsVisible) {
        quickDragging_ = quickClosePress_ = quickSkipRelease_ = false;
        quickLastSlot_ = -1;
        return;
    }
    const auto view = cui_viewport_size(ui);
    const HudPoint base({0,0,view.x,view.y});
    if (base.scale <= 0) return;
    if (interactive) {
        if (!client.shop.actor && !client.guild.open && !client.party.open &&
            !client.quests.open && !client.equipment.open && cui_key_pressed(ui,CUI_KEY_ESCAPE)) ToggleQuick(client);
        else if (!pointerCoversPanel_) QuickInput(ui,client,base.ToLocal(cui_mouse_pos(ui)));
    }
    if (interactive && pointerCoversPanel_ && cui_mouse_released(ui,CUI_MOUSE_LEFT))
        quickDragging_ = quickClosePress_ = quickSkipRelease_ = false;
    if (!client.inventory.shortcutsVisible) return;
    for (std::size_t i = 0; i < quickIcons_.size(); ++i) {
        quickIcons_[i] = {};
        std::string count;
        if (const auto* item = client.inventory.ResolveShortcut(i)) {
            CacheItem(ui,item->icon);
            quickIcons_[i] = itemFrames_.at(item->icon);
            if (item->Stacks() && item->count) {
                count = std::to_string(item->count);
                quickCountOffsets_[i] = static_cast<int>(5*(5-std::log10(static_cast<double>(item->count))));
            }
        }
        SetBagText(ui,quickCounts_[i],std::move(count),38,16);
    }
    cui_overlay_begin(ui,{0,0,view.x,view.y});
    auto* self = this;
    cui_canvas_paint(ui,CUI_FILL,CUI_FILL,[](cui_ctx* ui,cui_rect rect,void* user) {
        (*static_cast<Assets**>(user))->PaintQuick(ui,rect);
    },&self,sizeof self);
    cui_box_end(ui);
}

void Dialogs::Assets::PaintQuick(cui_ctx* ui, cui_rect rect) const
{
    const HudPoint base(rect);
    const auto image = [&](const Image& art,int x,int y) {
        if (art.texture) cui_draw_image(ui,art.texture,base.Rect(x,y,art.width,art.height),{0,0,1,1},{},
            {255,255,255,255});
    };
    cui_push_clip(ui,base.Rect(0,0,800,600));
    const auto& skin = quickFrames_[0];
    image(skin,quickX_-(skin.width-280)/2,quickY_-(skin.height-48)/2);
    image(quickFrames_[quickClosePress_ ? 2 : 1],quickX_+264,quickY_+32);
    for (int i = 0; i < 6; ++i) {
        const int x = quickX_+17+42*i, y = quickY_+6;
        const auto& icon = quickIcons_[i];
        image(icon,x+19-icon.width/2,y+19-icon.height/2);
        image(quickCounts_[i].image,x+quickCountOffsets_[i],y+23);
    }
    cui_pop_clip(ui);
}

bool Dialogs::Assets::InventoryBlocksScene(cui_ctx* ui, const LoginClient& client) const
{
    if (client.merchant.notice || client.merchant.quantityOpen) return true;
    if (!client.inventory.open) return false;
    if (inventoryDragging_ || inventoryScrollDrag_ || inventoryClosePress_ || inventoryServicePress_) return true;
    const auto view = cui_viewport_size(ui);
    const HudPoint base({0,0,view.x,view.y});
    const bool opening = client.merchant.actor && inventoryServiceRevision_ != client.merchant.revision;
    return base.scale > 0 && Contains({opening ? 519.f : float(inventoryX_),opening ? 0.f : float(inventoryY_),
        inventoryWidth_,inventoryHeight_},base.ToLocal(cui_mouse_pos(ui)));
}

void Dialogs::Assets::InventoryInput(cui_ctx* ui, LoginClient& client, cui_vec2 pointer)
{
    const int x = inventoryX_, y = inventoryY_;
    const auto within = [&](int dx,int dy,int w,int h) {
        return Contains({float(x+dx),float(y+dy),float(w),float(h)},pointer);
    };
    const auto& close = shopFrames_.at(1221);
    const auto& thumb = shopFrames_.at(1225);
    const bool onClose = within(242,423,close.width,close.height);
    const bool down = cui_mouse_clicked(ui,CUI_MOUSE_LEFT), up = cui_mouse_released(ui,CUI_MOUSE_LEFT);
    const bool moved = pointer.x != inventoryPointer_.x || pointer.y != inventoryPointer_.y;
    const int button = InventoryButton(client.merchant);
    const bool onService = button && within(198,401,inventoryFrames_.at(button).width,inventoryFrames_.at(button).height);
    const int trackHeight = 254+thumb.height;
    const int thumbTop = 70+static_cast<int>(254.0*(static_cast<float>(inventoryRow_)/91));

    if (inventoryDragging_) {
        if (moved && !up) {
            inventoryX_ = static_cast<int>(pointer.x)-inventoryDragX_;
            inventoryY_ = static_cast<int>(pointer.y)-inventoryDragY_;
            if (inventoryY_+inventoryHeight_ > 570) inventoryY_ = 565-inventoryHeight_;
        }
        if (up) inventoryDragging_ = false;
        return;
    }
    if (inventoryScrollDrag_) {
        if (moved && !up) inventoryRow_ = static_cast<int>(std::clamp((pointer.y-y-70)/trackHeight,0.f,1.f)*92);
        if (up) inventoryScrollDrag_ = false;
        return;
    }
    if (onService || inventoryServicePress_) {
        if (down) inventoryServicePress_ = onService;
        if (up && std::exchange(inventoryServicePress_,false) && onService) client.ConfirmMerchantItem();
        return;
    }
    if (onClose || inventoryClosePress_) {
        if (down) inventoryClosePress_ = onClose;
        if (up && std::exchange(inventoryClosePress_,false) && onClose) {
            if (client.audioOptions.effects && std::filesystem::exists(clickSound_))
                client.audio.PlayEffect(clickSound_.c_str(),client.audioOptions);
            client.CloseInventory();
        }
        return;
    }
    if (down && within(250,70,thumb.width,trackHeight)) {
        inventoryScrollDrag_ = within(250,thumbTop,thumb.width,thumb.height);
        if (!inventoryScrollDrag_) {
            const auto step = pointer.y < y+thumbTop+7 ? -8.f : 8.f;
            inventoryRow_ = static_cast<int>(std::clamp(inventoryRow_/91.f+step/92,0.f,1.f)*92);
        }
        return;
    }
    if (within(15,390,42,27)) {
        if (up && !client.HeldItem()) client.inventory.heldGold = true;
        return;
    }
    if (within(17,60,228,304)) {
        if (up && !client.inventory.heldGold) {
            const int cellX = static_cast<int>(pointer.x)-x-17, cellY = static_cast<int>(pointer.y)-y-60;
            const int cell = (inventoryRow_+cellY/38)*6+cellX/38;
            if (client.merchant.actor) client.QuoteMerchantItem(cell);
            else client.ReleaseBagCell(cell,cellX%38>=19,cellY%38>=19);
        }
        return;
    }
    if (down && within(0,0,inventoryWidth_,inventoryHeight_)) {
        inventoryDragging_ = true;
        inventoryDragX_ = static_cast<int>(pointer.x)-x;
        inventoryDragY_ = static_cast<int>(pointer.y)-y;
    }
}

void Dialogs::Assets::InventoryModalInput(cui_ctx* ui, LoginClient& client, cui_vec2 pointer)
{
    auto& service = client.merchant;
    auto& bag = client.inventory;
    const int x = 400-dialog_[0].width/2, y = 246-dialog_[0].height/2;
    const auto hit = [&](const Image& art,int dx) {
        return Contains({float(x+dx),float(y+188),float(art.width),float(art.height)},pointer);
    };
    const bool alert = service.notice || bag.notice;
    const int button = alert ? (hit(dialog_[1],137) ? 0 : -1)
        : hit(selectionButtons_.at(1241),41) ? 0 : hit(selectionButtons_.at(1245),234) ? 1 : -1;
    if (inventoryModalPress_ != button) inventoryModalPress_ = -1;
    if (cui_mouse_clicked(ui,CUI_MOUSE_LEFT)) inventoryModalPress_ = button;
    const bool click = cui_mouse_released(ui,CUI_MOUSE_LEFT) &&
        std::exchange(inventoryModalPress_,-1) >= 0;
    const bool enter = cui_key_pressed(ui,CUI_KEY_ENTER);
    if (alert) {
        if (click || enter || cui_key_pressed(ui,CUI_KEY_SPACE)) {
            if (service.notice) service.notice = 0;
            else {
                bag.notice = false;
                bag.status.clear();
            }
        }
        return;
    }
    const bool tab = cui_key_down(ui,CUI_KEY_TAB);
    if (!inventoryTabDown_ && (tab || cui_key_pressed(ui,CUI_KEY_TAB))) inventoryCancel_ = !inventoryCancel_;
    inventoryTabDown_ = tab;
    if (click) FinishInventoryQuantity(client,button == 0);
}

void Dialogs::Assets::FinishInventoryQuantity(LoginClient& client, bool confirm)
{
    if (client.inventory.drop != mir3_cui::DropDialog::None) {
        if (confirm) client.SubmitInventoryDrop();
        else client.inventory.drop = mir3_cui::DropDialog::None;
    } else if (confirm) client.SubmitMerchantQuantity();
    else client.merchant.quantityOpen = false;
}

void Dialogs::Assets::RefreshInventory(cui_ctx* ui, LoginClient& client, cui_vec2 pointer)
{
    const auto& bag = client.inventory;
    const auto& service = client.merchant;
    inventoryShow_ = bag.open;
    inventoryButton_ = InventoryButton(service);
    inventoryAlert_ = service.notice || bag.notice;
    const bool dropping = bag.drop != mir3_cui::DropDialog::None;
    inventoryQuantity_ = service.quantityOpen || dropping;
    const int quantityMessage = dropping ? (bag.drop == mir3_cui::DropDialog::Gold ? 1415 : 1416)
        : service.kind == mir3_cui::MerchantService::Kind::Deposit ? 1706 : 1705;
    SetBagText(ui,inventoryCaption_,service.notice ? Message(service.notice) :
        bag.notice ? bag.status : inventoryQuantity_ ? Message(quantityMessage) : "",
        inventoryAlert_ ? 308 : 278,inventoryAlert_ ? 95 : 75,true,mir3::TextFont::dialog);
    inventoryHeld_ = {};
    inventoryFootprint_ = {};
    if (const auto* held = client.HeldItem(); held || bag.heldGold) {
        if (!GameHudReady() || inventoryAlert_ || inventoryQuantity_) {
            const auto icon = bag.heldGold ? 124u : held->icon;
            CacheItem(ui,icon);
            inventoryHeld_ = itemFrames_.at(icon);
        }
        if (held && !bag.heldGold && bag.open && !inventoryAlert_ && !inventoryQuantity_ && !pointerCoversPanel_ &&
            Contains({float(inventoryX_+17),float(inventoryY_+60),228,304},pointer)) {
            const int x = static_cast<int>(pointer.x)-inventoryX_-17, y = static_cast<int>(pointer.y)-inventoryY_-60;
            const int column = x/38-held->width/2+(held->width%2 == 0 && x%38 >= 19);
            const int row = inventoryRow_+y/38-held->height/2+(held->height%2 == 0 && y%38 >= 19);
            inventoryFootprint_ = {float(inventoryX_+17+column*38),float(inventoryY_+60+(row-inventoryRow_)*38),
                float(held->width*38),float(held->height*38)};
            inventoryFit_ = column >= 0 && column < 6 && row >= 0 &&
                mir3_cui::InventoryPanel::Fits(bag.Occupied(held),*held,row*6+column);
        }
    }
    if (!bag.open) return;
    const auto& weight = client.characterStatus.weight;
    SetBagText(ui,inventoryWeight_,Message(2501)+": "+std::to_string(weight[0])+" / "+std::to_string(weight[1]),126,16);
    SetBagText(ui,inventoryGold_,std::to_string(bag.gold),77,17,false,mir3::TextFont::gold);
    std::string quote;
    if (service.price) {
        const auto* item = client.inventory.Find(service.item);
        const bool stackedSale = service.kind == mir3_cui::MerchantService::Kind::Sell && item && item->Stacks();
        quote = Message(stackedSale ? 1451 : 1452);
        if (const auto at = quote.find("%d"); at != std::string::npos)
            quote.replace(at,2,std::to_string(service.price));
    }
    SetBagText(ui,inventoryPrice_,std::move(quote),85,14);
    inventorySprites_.clear();
    for (const auto& item : bag.items) {
        if (!mir3_cui::InventoryPanel::OnGrid(item,item.cell)) continue;
        const int x = inventoryX_+17+item.cell%6*38, y = inventoryY_+60+(item.cell/6-inventoryRow_)*38;
        CacheItem(ui,item.icon);
        const auto& art = itemFrames_.at(item.icon);
        const int iconX = x+(item.width*38-art.width)/2, iconY = y+(item.height*38-art.height)/2;
        if (!art.texture || iconY+art.height <= inventoryY_+60 || iconY >= inventoryY_+364) continue;
        const int lx = x+18, ly = y+22;
        auto& count = inventoryCounts_[inventorySprites_.size()];
        SetBagText(ui,count,item.count > 1 && ly >= inventoryY_+60 && ly+16 <= inventoryY_+364 ?
            std::to_string(item.count) : "",20,16);
        inventorySprites_.push_back({art,count.image,iconX,iconY,lx,ly});
    }
}

void Dialogs::Assets::PrepareInventoryPlacement(LoginClient& client)
{
    const auto& service = client.merchant;
    if (std::exchange(client.trade.repositionInventory,false)) {
        inventoryX_ = 518;
        inventoryY_ = 0;
        LeaveInventory();
    }
}

void Dialogs::Assets::DrawInventoryPanel(cui_ctx* ui, LoginClient& client, bool interactive, bool shortcut)
{
    auto& service = client.merchant;
    auto& bag = client.inventory;
    PrepareInventoryPlacement(client);
    if (service.actor && inventoryServiceRevision_ != service.revision) {
        inventoryServiceRevision_ = service.revision;
        inventoryX_ = 519;
        inventoryY_ = 0;
        LeaveInventory();
    }
    const auto view = cui_viewport_size(ui);
    const HudPoint base({0,0,view.x,view.y});
    if (base.scale <= 0) return;
    const auto pointer = base.ToLocal(cui_mouse_pos(ui));
    int editor = bag.drop != mir3_cui::DropDialog::None ? bag.editor*2+1 : service.quantityOpen ? service.editor*2 : -1;
    if (editor >= 0 && inventoryEditor_ != editor) {
        inventoryModalPress_ = -1;
        inventoryCancel_ = inventoryTabDown_ = false;
    }
    if (interactive) {
        const bool escape = !client.shop.actor && !client.guild.open && !client.quests.open &&
            !client.party.open && !client.equipment.open && cui_key_pressed(ui,CUI_KEY_ESCAPE);
        if (service.notice || bag.notice || editor >= 0) InventoryModalInput(ui,client,pointer);
        else if (bag.open && shortcut && cui_shortcut(ui,0,CUI_KEY_W)) client.CloseInventory();
        else if (bag.open && shortcut && escape) {
            if (bag.shortcutsVisible) {
                if (!QuickReady()) client.ToggleItemShortcuts();
            }
            else client.CloseInventory();
        } else if (bag.open && !pointerCoversPanel_) InventoryInput(ui,client,pointer);
    }
    if (pointerCoversPanel_ && cui_mouse_released(ui,CUI_MOUSE_LEFT))
        inventoryDragging_ = inventoryScrollDrag_ = inventoryClosePress_ = inventoryServicePress_ = false;
    inventoryPointer_ = pointer;
    const bool dropping = bag.drop != mir3_cui::DropDialog::None;
    editor = dropping ? bag.editor*2+1 : service.quantityOpen ? service.editor*2 : -1;
    if (editor < 0) inventoryEditor_ = -1;
    RefreshInventory(ui,client,pointer);
    if (!inventoryShow_ && !inventoryAlert_ && !inventoryQuantity_ && !inventoryHeld_.texture) return;
    cui_overlay_begin(ui,{0,0,view.x,view.y});
    auto* self = this;
    cui_canvas_paint(ui,CUI_FILL,CUI_FILL,[](cui_ctx* context,cui_rect rect,void* user) {
        (*static_cast<Assets**>(user))->PaintInventory(context,rect);
    },&self,sizeof self);
    if (editor >= 0 && !inventoryAlert_ && interactive) {
        const auto field = base.Rect(400-dialog_[0].width/2+24,246-dialog_[0].height/2+228,309,15);
        cui_overlay_begin(ui,field);
        cui_push_id_int(ui,editor);
        const bool enter = cui_key_pressed(ui,CUI_KEY_ENTER);
        if (enter || inventoryEditor_ != editor ||
            (cui_mouse_clicked(ui,CUI_MOUSE_LEFT) && !Contains(field,cui_mouse_pos(ui))))
            cui_text_field_focus(ui,"inventory-quantity",false);
        inventoryEditor_ = editor;
        const auto opts = ModalEditorOptions(field.h,base.scale);
        cui_input_ex(ui,"inventory-quantity",dropping ? bag.amount : service.quantity,
            dropping ? sizeof bag.amount : sizeof service.quantity,&opts);
        if (enter) {
            FinishInventoryQuantity(client,!inventoryCancel_);
            cui_request_redraw(ui);
        }
        cui_pop_id(ui);
        cui_box_end(ui);
    }
    cui_box_end(ui);
}

void Dialogs::Assets::PaintInventory(cui_ctx* ui, cui_rect rect) const
{
    if (HudPoint(rect).scale <= 0) return;
    const HudPoint base(rect);
    constexpr cui_color white{255,255,255,255}, title{250,220,248,255}, caption{255,250,200,255};
    const auto image = [&](const Image& art,int x,int y,cui_color tint) {
        if (art.texture) cui_draw_image(ui,art.texture,base.Rect(x,y,art.width,art.height),{0,0,1,1},{},tint);
    };
    const int x = inventoryX_, y = inventoryY_;
    cui_push_clip(ui,base.Rect(0,0,800,600));
    if (inventoryShow_) {
        const auto& skin = inventoryFrames_.at(1220);
        image(skin,x-(skin.width-inventoryWidth_)/2,y-(skin.height-inventoryHeight_)/2,white);
        const int titleIndex = inventoryButton_ ? (inventoryButton_-1227)/2+1 : 0;
        image(inventoryTitles_[titleIndex],x+12,y+20,title);
        if (inventoryButton_) {
            image(inventoryFrames_.at(inventoryButton_-int(inventoryServicePress_)),x+198,y+401,white);
            image(inventoryPrice_.image,x+70,y+423,title);
        }
        image(inventoryWeight_.image,x+95,y+21,{10,10,10,255});
        image(inventoryWeight_.image,x+94,y+20,{200,200,248,255});
        image(shopFrames_.at(inventoryClosePress_ ? 1222 : 1221),x+242,y+423,white);
        image(shopFrames_.at(1225),x+250,y+70+static_cast<int>(254.0*(static_cast<float>(inventoryRow_)/91)),white);
        image(inventoryGold_.image,x+72,y+402,{248,200,100,255});
        cui_push_clip(ui,base.Rect(x+17,y+60,228,304));
        if (inventoryFootprint_.w > 0) cui_draw_rect_premultiplied(ui,
            base.Rect(inventoryFootprint_.x,inventoryFootprint_.y,inventoryFootprint_.w,inventoryFootprint_.h),
            inventoryFit_ ? cui_color{50,100,50,1} : cui_color{100,50,50,1});
        for (const auto& sprite : inventorySprites_) {
            image(sprite.icon,sprite.x,sprite.y,white);
            image(sprite.count,sprite.labelX,sprite.labelY,white);
        }
        cui_pop_clip(ui);
    }
    if (!pointerCoversPanel_ && inventoryHeld_.texture && Contains({0,0,800,600},inventoryPointer_))
        image(inventoryHeld_,static_cast<int>(inventoryPointer_.x)-inventoryHeld_.width/2,
            static_cast<int>(inventoryPointer_.y)-inventoryHeld_.height/2,white);
    if (inventoryAlert_ || inventoryQuantity_) {
        const int dx = 400-dialog_[0].width/2, dy = 246-dialog_[0].height/2;
        image(dialog_[0],dx,dy,white);
        if (inventoryAlert_) {
            image(inventoryCaption_.image,dx+23,dy+84,caption);
            image(dialog_[inventoryModalPress_ == 0 ? 2 : 1],dx+137,dy+188,white);
        } else {
            image(inventoryCaption_.image,dx+43,dy+84,caption);
            image(selectionButtons_.at(1241+(inventoryModalPress_ == 0)),dx+41,dy+188,white);
            image(selectionButtons_.at(1245+(inventoryModalPress_ == 1)),dx+234,dy+188,white);
        }
    }
    cui_pop_clip(ui);
}

bool Dialogs::Assets::ShopHit(cui_vec2 pointer, int id, int x, int y) const
{
    const auto& frame = shopFrames_.at(id);
    return Contains({float(x),float(y),float(frame.width),float(frame.height)},pointer);
}

bool Dialogs::Assets::ShopHasModal(const LoginClient& client) const
{
    return client.shop.notice || client.shop.withdrawalPrompt ||
        (shopActor_==client.shop.actor && shopCatalogue_==client.shop.catalogueRevision && shopBulkPrompt_);
}

bool Dialogs::Assets::ShopBlocksScene(cui_ctx* ui, const LoginClient& client, bool hitOnly) const
{
    if (!hitOnly && (ShopHasModal(client) ||
        (shopActor_ == client.shop.actor && shopCatalogue_ == client.shop.catalogueRevision &&
         (shopScrollDrag_ || shopPressed_ != ShopControl::None || shopPressedRow_ >= 0)))) return true;
    const auto view = cui_viewport_size(ui);
    const HudPoint base({0,0,view.x,view.y});
    const cui_rect bounds = client.shop.storage ? cui_rect{-292,234,488,205}
        : cui_rect{0,float(shopTop_),float(ShopWidth(client.shop)),224};
    return base.scale > 0 && Contains(bounds,base.ToLocal(cui_mouse_pos(ui)));
}

void Dialogs::Assets::ShopAction(LoginClient& client, ShopControl control)
{
    auto& shop = client.shop;
    switch (control) {
    case ShopControl::Close:
        shop = {};
        break;
    case ShopControl::OfferClose:
        if (shop.storage) {
            shop = {};
            break;
        }
        shop.detailed = shop.bulk = shop.offersVisible = false;
        shop.category = shop.offer = -1;
        shop.offers.clear();
        shop.page = 0;
        shop.detailDeadline = 0;
        break;
    case ShopControl::Details:
        client.RequestShopOffers(shop.selected,0);
        break;
    case ShopControl::Previous:
    case ShopControl::Next: {
        const int direction = control == ShopControl::Next ? 1 : -1;
        if (shop.storage) {
            client.PageStorage(direction);
            break;
        }
        const auto page = std::clamp(static_cast<int>(shop.page) + direction,0,65535/10);
        if (page != static_cast<int>(shop.page)) client.RequestShopOffers(shop.category,static_cast<unsigned>(page));
        break;
    }
    case ShopControl::Buy:
        if (shop.storage) {
            client.WithdrawStorage();
            if (shop.withdrawalPrompt) {
                ++shopEditGeneration_;
                shopInputFocus_ = true;
                shopModalPress_ = -1;
                shopCancel_ = shopTabDown_ = false;
            }
        } else client.BuyShopItem();
        break;
    case ShopControl::Bulk:
        if (shop.crafting) {
            client.BuyShopItem();
            break;
        }
        if (shop.bulk && !client.purchase.deadline) {
            shopBulkPrompt_ = true;
            ++shopEditGeneration_;
            shopInputFocus_ = true;
            shopModalPress_ = -1;
            shopCancel_ = shopTabDown_ = false;
            shop.quantity[0] = 0;
        }
        break;
    case ShopControl::None:
        break;
    }
}

void Dialogs::Assets::ShopInput(cui_ctx* ui, LoginClient& client, cui_vec2 pointer)
{
    auto& shop = client.shop;
    constexpr int top = shopTop_+32, travel = 85;
    const auto& thumb = shopFrames_.at(1225);
    const int last = (std::max)(0,static_cast<int>(shop.goods.size())-1);
    const int thumbY = top + static_cast<int>(std::lround(travel*shopFraction_));
    const auto inside = [&](int x,int y,int w,int h) { return Contains({float(x),float(y),float(w),float(h)},pointer); };
    const bool down = cui_mouse_clicked(ui,CUI_MOUSE_LEFT), up = cui_mouse_released(ui,CUI_MOUSE_LEFT);
    const bool moved = pointer.x != shopPointer_.x || pointer.y != shopPointer_.y;
    if (last && down && inside(260,top,thumb.width,travel+thumb.height)) {
        shopScrollDrag_ = inside(260,thumbY,thumb.width,thumb.height);
        if (!shopScrollDrag_) {
            const int direction = pointer.y < thumbY+thumb.height/2 ? -1 : 1;
            shopFraction_ = std::clamp(shopFraction_ + direction*5.0/shop.goods.size(),0.0,1.0);
            shopFirst_ = static_cast<int>(std::lround(last*shopFraction_));
        }
        shopPressed_ = ShopControl::None;
        shopPressedRow_ = -1;
        return;
    }
    if (shopScrollDrag_) {
        if (moved) {
            shopFraction_ = std::clamp((pointer.y-top)/double(travel+thumb.height),0.0,1.0);
            shopFirst_ = static_cast<int>(std::lround(last*shopFraction_));
        }
        if (up) shopScrollDrag_ = false;
        return;
    }
    const int detailX = shop.storage ? -292 : 0;
    constexpr int detailY = 388;
    const auto hit = [&](int id,int x,int y) { return ShopHit(pointer,id,x,y); };
    ShopControl button = ShopControl::None;
    if (!shop.storage && hit(1221,258,shopTop_+183)) button = ShopControl::Close;
    else if ((shop.offersVisible && hit(1221,detailX+449,detailY-4)) ||
             (shop.bulk && hit(1221,475,shopTop_+56))) button = ShopControl::OfferClose;
    else if (shop.bulk && hit(1272,367,shopTop_+59)) button = ShopControl::Bulk;
    else if (shop.offersVisible && hit(1265,detailX+358,detailY)) button = ShopControl::Buy;
    else if (shop.offersVisible && hit(1268,detailX+316,detailY)) button = ShopControl::Previous;
    else if (shop.offersVisible && hit(1270,detailX+406,detailY)) button = ShopControl::Next;
    else if (!shop.storage && hit(1263,106,shopTop_+186)) button = ShopControl::Details;
    if (button != ShopControl::None || shopPressed_ != ShopControl::None) {
        const bool click = up && button != ShopControl::None && button == shopPressed_;
        if (down) shopPressed_ = button;
        if (up) shopPressed_ = ShopControl::None;
        if (click) ShopAction(client,button);
        shopPressedRow_ = -1;
        return;
    }
    if (!down && !up) return;
    int row = -1;
    if (shop.offersVisible && !shop.detailDeadline) {
        const int offset = shop.storage ? 12*static_cast<int>(shop.page) : 0;
        for (int i = 0; i < 12 && offset+i < static_cast<int>(shop.offers.size()); ++i)
            if (Contains(ShopOfferCell(i,shop.storage),pointer)) { row = 256+offset+i; break; }
    }
    if (row < 0) {
        for (int i = 0; i < 4 && shopFirst_+i < static_cast<int>(shop.goods.size()); ++i) {
            const int y = shopTop_+16+42*i;
            if (inside(20,y,36,36) || inside(62,y,189,36)) { row = shopFirst_+i; break; }
        }
    }
    constexpr int bulkRow = -2;
    if (shop.bulk && (inside(305,shopTop_+17,36,36) || inside(352,shopTop_+17,148,36))) row = bulkRow;
    if (down) {
        const double now = cui_time(ui);
        const bool twice = row != -1 && row == shopLastRow_ && now-shopLastDown_ < 0.30;
        shopPressedRow_ = twice ? -1 : row;
        shopLastRow_ = twice ? -1 : row;
        shopLastDown_ = now;
        if (twice) {
            if (row == bulkRow) {
                strcpy_s(shop.quantity,"1");
                ShopAction(client,ShopControl::Buy);
            } else if (row >= 256) {
                shop.offer = row-256;
                ShopAction(client,ShopControl::Buy);
            } else {
                shop.selected = row;
                client.RequestShopOffers(row,0);
            }
        }
    }
    if (up && row >= 0 && row == std::exchange(shopPressedRow_,-1)) {
        if (row >= 256) shop.offer = row-256;
        else shop.selected = row;
    } else if (up) shopPressedRow_ = -1;
}

void Dialogs::Assets::ShopModalInput(cui_ctx* ui, LoginClient& client, cui_vec2 pointer)
{
    auto& shop = client.shop;
    const int x = 400-dialog_[0].width/2, y = 246-dialog_[0].height/2;
    const auto hit = [&](const Image& art,int dx) {
        return Contains({float(x+dx),float(y+188),float(art.width),float(art.height)},pointer);
    };
    const bool alert = shop.notice != 0;
    const int button = alert ? (hit(dialog_[1],137) ? 0 : -1) :
        hit(selectionButtons_.at(1241),41) ? 0 : hit(selectionButtons_.at(1245),234) ? 1 : -1;
    if (shopModalPress_ >= 0 && shopModalPress_ != button) shopModalPress_ = -1;
    if (cui_mouse_clicked(ui,CUI_MOUSE_LEFT)) shopModalPress_ = button;
    const bool up = cui_mouse_released(ui,CUI_MOUSE_LEFT);
    const bool click = up && button >= 0 && button == shopModalPress_;
    if (up) shopModalPress_ = -1;
    const bool tab = cui_key_down(ui,CUI_KEY_TAB);
    if (!alert && !shopTabDown_ && (tab || cui_key_pressed(ui,CUI_KEY_TAB))) shopCancel_ = !shopCancel_;
    shopTabDown_ = tab;
    const bool key = alert && (cui_key_pressed(ui,CUI_KEY_ENTER) || cui_key_pressed(ui,CUI_KEY_SPACE));
    if (!key && !click) return;
    if (click && client.audioOptions.effects && std::filesystem::exists(clickSound_))
        client.audio.PlayEffect(clickSound_.c_str(),client.audioOptions);
    if (alert) shop.notice = 0;
    else FinishShopQuantity(client,button == 0);
    cui_request_redraw(ui);
}

void Dialogs::Assets::FinishShopQuantity(LoginClient& client, bool confirm)
{
    auto& shop = client.shop;
    shopBulkPrompt_ = false;
    if (confirm) {
        if (shop.storage) client.SubmitStorageQuantity();
        else client.BuyShopItem();
    } else shop.withdrawalPrompt = false;
    shop.quantity[0] = 0;
}


void Dialogs::Assets::ShopGoodText(cui_ctx* ui, ShopText& target, const mir3_cui::NpcShop::Good& good,
                                  const LoginClient& client, bool bulk)
{
    if (target.name != good.label) {
        target.name = good.label;
        target.nameWidth = mir3::raster_text(target.name,1,1,false,mir3::TextFont::normal).text_width;
        Destroy(ui,target.label);
        target.label = Raster(ui,target.name,bulk ? 126 : 184,16,false,mir3::TextFont::normal);
    }
    auto price = Message(2300);
    const auto marker = price.find("%d");
    if (marker == price.npos) price = std::to_string(good.price);
    else price.replace(marker,2,std::to_string(good.price));
    if (target.price != price) {
        target.price = std::move(price);
        Destroy(ui,target.amount);
        target.amount = Raster(ui,target.price,bulk ? 140 : 124,16,false,mir3::TextFont::normal);
    }
    std::string requirement;
    if (good.icon == 304 && good.requirement <= 2) {
        requirement = Message(5170+static_cast<int>(good.requirement));
        target.qualified = client.characterAppearance.profession == good.requirement;
    } else if (good.icon != 304 && good.mode && good.requirement && good.requirementType < 12) {
        const auto& stats = client.characterStatus;
        const auto& element = stats.elements[0];
        const std::array attributes{stats.level,stats.combat[0][1],
            *(std::min_element)(element.begin(),element.begin()+4),
            *(std::min_element)(element.begin()+4,element.end()),
            *(std::min_element)(element.begin(),element.end()),
            element[0],element[1],element[2],element[3],element[4],element[5],element[6]};
        requirement = Message(5140+static_cast<int>(good.requirementType)) + " " + std::to_string(good.requirement);
        target.qualified = attributes[good.requirementType] >= good.requirement;
    }
    if (target.requirement != requirement) {
        target.requirement = std::move(requirement);
        Destroy(ui,target.condition);
        if (!target.requirement.empty())
            target.condition = Raster(ui,target.requirement,bulk ? 142 : 184,16,false,mir3::TextFont::normal);
    }
}

void Dialogs::Assets::RefreshShop(cui_ctx* ui, const LoginClient& client)
{
    const auto& shop = client.shop;
    const auto good = [&](ShopVisual& visual, int index, bool bulk) {
        visual.visible = index >= 0 && index < static_cast<int>(shop.goods.size());
        if (!visual.visible) return;
        const auto& entry = shop.goods[index];
        visual.icon = entry.icon;
        visual.variants = entry.mode;
        visual.selected = index == shop.selected;
        CacheItem(ui,entry.icon);
        ShopGoodText(ui,visual.text,entry,client,bulk);
    };
    for (int i = 0; i < 4; ++i) good(shopGoods_[i],shopFirst_+i,false);
    good(shopBulkVisual_,shop.bulk ? shop.category : -1,true);
    shopDrawThumb_ = shop.goods.size() > 1;
    shopShowsBulk_ = shop.bulk;
    shopShowsOffers_ = shop.offersVisible;
    shopStorage_ = shop.storage;
    const int offset = shop.storage ? 12*static_cast<int>(shop.page) : 0;
    for (int i = 0; i < 12; ++i) {
        auto& visual = shopOffers_[i];
        const int index = offset+i;
        visual.visible = shopShowsOffers_ && index < static_cast<int>(shop.offers.size());
        if (!visual.visible) continue;
        const auto& entry = shop.offers[index];
        visual.selected = index == shop.offer;
        visual.icon = entry.icon;
        CacheItem(ui,entry.icon);
        if (shop.storage) continue;
        auto price = std::to_string(entry.price);
        if (visual.text.price != price) {
            visual.text.price = std::move(price);
            Destroy(ui,visual.text.amount);
            visual.text.amount = Raster(ui,visual.text.price,35,15,true,mir3::TextFont::skill_level);
        }
    }
    const auto page = shop.detailDeadline ? std::string("...") : std::to_string(shop.page+unsigned(shop.storage));
    const int pageWidth = shop.storage ? 40 : 163;
    if (page != shopPageText_ || shopPageImage_.width != pageWidth) {
        shopPageText_ = page;
        Destroy(ui,shopPageImage_);
        shopPageImage_ = Raster(ui,page,pageWidth,shop.storage ? 30 : 44,true,mir3::TextFont::normal);
    }
    if (shop.storage) {
        auto total = "/    "+std::to_string((std::max)(std::size_t{1},(shop.offers.size()+11)/12));
        if (total != shopPageTotalText_) {
            shopPageTotalText_ = std::move(total);
            Destroy(ui,shopPageTotal_);
            shopPageTotal_ = Raster(ui,shopPageTotalText_,50,30,true,mir3::TextFont::normal);
        }
    }
    shopNotice_ = shop.notice != 0;
    shopDrawQuantity_ = shopBulkPrompt_ || shop.withdrawalPrompt;
    auto caption = shopNotice_ ? Message(shop.notice) : shopDrawQuantity_ ? Message(shop.storage ? 2301 : 2302)
        : std::string{};
    const int width = shopNotice_ ? 308 : 278, height = shopNotice_ ? 95 : 75;
    if (caption != shopCaptionText_ || (!caption.empty() && shopCaption_.width != width)) {
        shopCaptionText_ = std::move(caption);
        Destroy(ui,shopCaption_);
        if (!shopCaptionText_.empty()) shopCaption_ = Raster(ui,shopCaptionText_,width,height,true,mir3::TextFont::dialog);
    }
}

void Dialogs::Assets::DrawShopPanel(cui_ctx* ui, LoginClient& client, int wheel, bool interactive)
{
    auto& shop = client.shop;
    const auto view = cui_viewport_size(ui);
    const HudPoint base({0,0,view.x,view.y});
    if (base.scale <= 0) return;
    const auto pointer = base.ToLocal(cui_mouse_pos(ui));
    if (shopActor_ != shop.actor || shopCatalogue_ != shop.catalogueRevision) {
        shopActor_ = shop.actor;
        shopCatalogue_ = shop.catalogueRevision;
        shopFirst_ = 0;
        shopFraction_ = 0;
        shopPressed_ = ShopControl::None;
        shopPressedRow_ = shopLastRow_ = shopModalPress_ = -1;
        shopScrollDrag_ = shopBulkPrompt_ = false;
    }
    if (!shop.bulk) shopBulkPrompt_ = false;
    shop.sourceLayout = true;
    if (wheel && !ShopHasModal(client)) {
        const int last=(std::max)(0,int(shop.goods.size())-1);
        shopFirst_=std::clamp(shopFirst_+wheel,0,last);
        shopFraction_=last ? double(shopFirst_)/last : 0;
        shopPressed_=ShopControl::None;
        shopPressedRow_=shopLastRow_=-1;
    }
    if (interactive) {
        if (shop.notice || shopBulkPrompt_ || shop.withdrawalPrompt) ShopModalInput(ui,client,pointer);
        else if (cui_key_pressed(ui,CUI_KEY_ESCAPE))
            ShopAction(client,shop.offersVisible || shop.bulk ? ShopControl::OfferClose : ShopControl::Close);
        else ShopInput(ui,client,pointer);
    }
    shopPointer_ = pointer;
    if (!shop.actor) return;
    RefreshShop(ui,client);
    cui_overlay_begin(ui,{0,0,view.x,view.y});
    auto* self = this;
    cui_canvas_paint(ui,CUI_FILL,CUI_FILL,[](cui_ctx* ui,cui_rect rect,void* user) {
        (*static_cast<Assets**>(user))->PaintShop(ui,rect);
    },&self,sizeof self);
    if (shopDrawQuantity_ && !shopNotice_ && interactive) {
        const auto field = base.Rect(400-dialog_[0].width/2+24,246-dialog_[0].height/2+228,309,15);
        cui_overlay_begin(ui,field);
        cui_push_id_int(ui,shopEditGeneration_);
        const bool enter = cui_key_pressed(ui,CUI_KEY_ENTER);
        if (std::exchange(shopInputFocus_,false) || enter ||
            (cui_mouse_clicked(ui,CUI_MOUSE_LEFT) && !Contains(field,cui_mouse_pos(ui))))
            cui_text_field_focus(ui,"shop-quantity",false);
        const auto opts = ModalEditorOptions(field.h,base.scale);
        cui_input_ex(ui,"shop-quantity",shop.quantity,sizeof shop.quantity,&opts);
        if (enter) {
            FinishShopQuantity(client,!shopCancel_);
            cui_request_redraw(ui);
        }
        cui_pop_id(ui);
        cui_box_end(ui);
    }
    cui_box_end(ui);
}

void Dialogs::Assets::PaintShop(cui_ctx* ui, cui_rect rect) const
{
    const HudPoint base(rect);
    const cui_color white{255,255,255,255}, border{250,200,150,255}, gray{180,180,180,255};
    const cui_color gold{250,250,0,255}, priceInk{250,180,100,255}, shadow{10,10,10,255};
    const auto image = [&](const Image& art, int x, int y, cui_color tint) {
        if (art.texture) cui_draw_image(ui,art.texture,base.Rect(x,y,art.width,art.height),{0,0,1,1},{},tint);
    };
    const auto sprite = [&](int id,int x,int y) { image(shopFrames_.at(id),x,y,white); };
    const auto outline = [&](int x,int y,int w,int h,cui_color color) {
        for (const auto& side : std::array<cui_rect,4>{base.Rect(x-1,y-1,w+1,1),base.Rect(x-1,y+h-1,w+1,1),
            base.Rect(x-1,y,1,h-1),base.Rect(x+w-1,y,1,h-1)}) cui_draw_rect(ui,side,{},color);
    };
    const auto tint = [&](int x,int y,int w,int h,cui_color color) {
        cui_draw_rect_premultiplied(ui,base.Rect(x,y,w,h),color);
    };
    const auto condition = [&](const ShopVisual& row,int x,int y) {
        const auto allowed = row.icon == 304 ? cui_color{200,255,200,255} : cui_color{240,240,255,255};
        image(row.text.condition,x,y,row.text.qualified ? allowed : cui_color{255,30,30,255});
    };
    cui_push_clip(ui,base.Rect(0,0,800,600));
    const auto& skin = shopFrames_.at(shopShowsBulk_ ? 1262 : shopShowsOffers_ ? 1261 : 1260);
    const int width = shopShowsBulk_ ? 516 : shopShowsOffers_ ? 488 : 292;
    const int detailX = shopStorage_ ? -292 : 0;
    const int top = shopStorage_ ? 234 : shopTop_, height = shopStorage_ ? 205 : 224;
    image(skin,detailX+(width-skin.width)/2,top+(height-skin.height)/2,white);
    if (shopDrawThumb_) sprite(1225,260,shopTop_+32+static_cast<int>(std::lround(85*shopFraction_)));
    if (shopPressed_ == ShopControl::Close) sprite(1222,258,shopTop_+183);
    if (shopPressed_ == ShopControl::Details) sprite(1264,106,shopTop_+186);

    constexpr int detailY = 388;
    if (shopShowsOffers_) {
        const cui_color pageInk{255,200,150,255};
        if (shopStorage_) {
            image(shopPageImage_,52,229,pageInk);
            image(shopPageTotal_,82,229,pageInk);
        } else image(shopPageImage_,310,shopTop_-3,pageInk);
        if (shopPressed_ == ShopControl::OfferClose) sprite(1222,detailX+449,detailY-4);
        if (shopPressed_ == ShopControl::Buy) sprite(1266,detailX+358,detailY);
        if (shopPressed_ == ShopControl::Previous) sprite(1269,detailX+316,detailY);
        if (shopPressed_ == ShopControl::Next) sprite(1271,detailX+406,detailY);
        for (int i = 0; i < 12; ++i) {
            const auto& row = shopOffers_[i];
            if (!row.visible) continue;
            const auto& art = itemFrames_.at(row.icon);
            if (!art.texture) continue;
            const auto cell = ShopOfferCell(i,shopStorage_);
            const int x = static_cast<int>(cell.x), y = static_cast<int>(cell.y);
            if (row.selected) {
                tint(x,y,static_cast<int>(cell.w),static_cast<int>(cell.h),{50,150,150,1});
                outline(x,y,static_cast<int>(cell.w),static_cast<int>(cell.h),gray);
            }
            image(art,x+19-art.width/2,y+19-art.height/2,white);
            if (shopStorage_) continue;
            image(row.text.amount,x-1,y+20,shadow);
            image(row.text.amount,x,y+21,white);
        }
    }
    if (shopShowsBulk_ && shopBulkVisual_.visible) {
        const auto& row = shopBulkVisual_;
        tint(305,shopTop_+17,36,36,{80,80,160,1});
        tint(352,shopTop_+17,148,36,{80,80,160,1});
        outline(305,shopTop_+17,36,36,gray);
        outline(352,shopTop_+17,148,36,border);
        const auto& art = itemFrames_.at(row.icon);
        image(art,324-art.width/2,shopTop_+36-art.height/2,white);
        const int y = shopTop_+(row.text.condition.texture ? 19 : 30);
        image(row.text.label,357,y-1,shadow);
        image(row.text.label,358,y,gold);
        image(row.text.amount,359+row.text.nameWidth,y-1,shadow);
        image(row.text.amount,360+row.text.nameWidth,y,priceInk);
        condition(row,358,y+20);
        if (shopPressed_ == ShopControl::OfferClose) sprite(1222,475,shopTop_+56);
        if (shopPressed_ == ShopControl::Bulk) sprite(1273,367,shopTop_+59);
    }
    for (int i = 0; i < 4; ++i) {
        const auto& row = shopGoods_[i];
        if (!row.visible) continue;
        const int y = shopTop_+16+42*i;
        if (row.selected) {
            const auto color = row.variants ? cui_color{40,120,120,1} : cui_color{80,80,160,1};
            tint(20,y,36,36,color);
            tint(62,y,189,36,color);
            outline(62,y,189,36,border);
        }
        const auto& art = itemFrames_.at(row.icon);
        image(art,39-art.width/2,y+19-art.height/2,white);
        if (row.selected || row.variants) outline(20,y,36,36,row.selected ? border : gray);
        const int labelY = y+(row.icon == 304 || row.text.condition.texture ? 2 : 13);
        if (row.selected) {
            image(row.text.label,65,labelY-1,shadow);
            image(row.text.amount,67+row.text.nameWidth,labelY-1,shadow);
        }
        image(row.text.label,66,labelY,gold);
        image(row.text.amount,68+row.text.nameWidth,labelY,priceInk);
        condition(row,66,labelY+20);
    }
    if (shopNotice_ || shopDrawQuantity_) {
        const int x = 400-dialog_[0].width/2, y = 246-dialog_[0].height/2;
        image(dialog_[0],x,y,white);
        const cui_color ink{255,250,200,255};
        if (shopNotice_) {
            image(shopCaption_,x+23,y+84,ink);
            image(dialog_[shopModalPress_==0 ? 2 : 1],x+137,y+188,white);
        } else {
            image(shopCaption_,x+43,y+84,ink);
            image(selectionButtons_.at(1241+(shopModalPress_==0)),x+41,y+188,white);
            image(selectionButtons_.at(1245+(shopModalPress_==1)),x+234,y+188,white);
        }
    }
    cui_pop_clip(ui);
}


void Dialogs::Assets::OpenParty(cui_ctx* ui, mir3::WilArchive& archive)
{
    partyReady_ = dialog_[0].texture && dialog_[1].texture && dialog_[2].texture;
    for (const int id : {1221,1222,1360,1362,1363,1365,1366,1368,1369,1370,1371}) {
        partyFrames_[id] = UploadFrame(ui, archive, id);
        partyReady_ &= partyFrames_[id].texture != 0;
    }
    for (const int id : {1241,1242,1245,1246}) partyReady_ &= selectionButtons_.at(id).texture != 0;
    for (int i = 0; i < 2; ++i)
        partyAllowCaptions_[i] = Raster(ui, Message(i ? 1603 : 1604), 140, 15, false, mir3::TextFont::normal);
    constexpr int hints[] = {1052, 1600, 1601, 1602};
    for (int i = 0; i < 4; ++i) partyHints_[i] = MakeHudHint(ui, hints[i]);
}

void Dialogs::Assets::RefreshPartyText(cui_ctx* ui, const LoginClient& client)
{
    const auto& party = client.party;
    if (partyPlayerName_ != client.gameCharacter) {
        partyPlayerName_ = client.gameCharacter;
        Destroy(ui, partyPlayer_);
        partyPlayer_ = Raster(ui, partyPlayerName_, 180, 15, false, mir3::TextFont::normal);
    }
    if (partyRoster_ != party.members) {
        partyRoster_ = party.members;
        for (auto& line : partyRosterText_) Destroy(ui, line);
        partyRosterText_.clear();
        for (int i = 0; i < 10 && i < static_cast<int>(partyRoster_.size()); ++i)
            partyRosterText_.push_back(Raster(ui, partyRoster_[i], 100, 15, false, mir3::TextFont::normal));
    }
    const int caption = party.notice.id ? party.notice.id :
        party.action == mir3_cui::PartyAction::None ? 0 :
        party.action == mir3_cui::PartyAction::Remove ? 1606 : 1605;
    if (caption != partyModalKey_) {
        partyModalKey_ = caption;
        Destroy(ui, partyCaption_);
        if (caption) partyCaption_ = Raster(ui, Message(caption), party.notice.id ? 308 : 278,
            party.notice.id ? 95 : 75, true, mir3::TextFont::dialog);
    }
    if (partyErrorText_ != party.error) {
        partyErrorText_ = party.error;
        Destroy(ui, partyErrorImage_);
        if (!partyErrorText_.empty())
            partyErrorImage_ = Raster(ui, partyErrorText_, 308, 30, true, mir3::TextFont::dialog);
    }
}

bool Dialogs::Assets::HandlePartyInput(cui_ctx* ui, LoginClient& client, bool blocked, bool fallback)
{
    using mir3_cui::PartyAction;
    auto& party = client.party;
    const bool covered = PartyBlocksScene(ui);
    if (fallback && !blocked && partyFocused_ && cui_shortcut(ui,0,CUI_KEY_G)) {
        party.open = false;
        LeaveParty();
        return covered;
    }
    partyFocused_ = true;
    partyHint_ = -1;
    if (blocked) {
        partyPressed_ = -1;
        partyDragging_ = false;
        return false;
    }
    if (!covered) return false;
    const auto viewport = cui_viewport_size(ui);
    const auto local = HudPoint({0,0,viewport.x,viewport.y}).ToLocal(cui_mouse_pos(ui));
    const float mx = local.x, my = local.y;
    const auto hit = [&](float x, float y, float w, float h) { return Contains({x,y,w,h},local); };
    const auto hits = [&](const Image& image, int x, int y) { return hit(x,y,image.width,image.height); };
    const bool down = cui_mouse_clicked(ui,CUI_MOUSE_LEFT), up = cui_mouse_released(ui,CUI_MOUSE_LEFT);
    partyConfirmPressed_ = -1;
    int button = -1;
    for (int i = 0; i < 5; ++i) {
        const auto& [x, y] = partyButtonOffsets_[i];
        if (hits(partyFrames_.at(partyButtonFrames_[i]), partyX_ + x, partyY_ + y)) button = i;
    }
    if (down && hit(partyX_, partyY_, 258, 244)) {
        partyPressed_ = button;
        partyDragging_ = button < 0;
        if (partyDragging_) {
            partyDragX_ = static_cast<int>(mx) - partyX_;
            partyDragY_ = static_cast<int>(my) - partyY_;
        }
    } else if (partyDragging_ && !up) {
        partyX_ = static_cast<int>(mx) - partyDragX_;
        partyY_ = static_cast<int>(my) - partyDragY_;
        if (partyY_ + 244 > 570) partyY_ = 565 - 244;
    }
    if (up) {
        const int pressed = std::exchange(partyPressed_, -1);
        partyDragging_ = false;
        if (button >= 0 && button == pressed) {
            if (client.audioOptions.effects && std::filesystem::exists(clickSound_))
                client.audio.PlayEffect(clickSound_.c_str(), client.audioOptions);
            if (button == 0) party.open = false;
            else if (button == 4) client.SetPartyAllowed(!party.allowed);
            else party.Begin(static_cast<PartyAction>(mir3_cui::PartyMode + button));
        }
    }
    if (button >= 0 && button < 4 && button != partyPressed_ && !partyDragging_ &&
        partyHints_[button].texture && party.action == PartyAction::None) {
        partyHint_ = button;
        constexpr int hints[] = {1052, 1600, 1601, 1602};
        partyHintDrawHeight_ = hudHint_.Advance(ui, hints[button], partyHints_[button].height);
    }
    return covered;
}

void Dialogs::Assets::DrawPartyPanel(cui_ctx* ui, LoginClient& client, bool drawPanel, bool drawDialog)
{
    using mir3_cui::PartyAction;
    auto& party = client.party;
    const auto viewport = cui_viewport_size(ui);
    const float scale = (std::min)(viewport.x / 800, viewport.y / 600);
    if (scale <= 0) return;
    const float ox = (viewport.x - 800 * scale) / 2, oy = (viewport.y - 600 * scale) / 2;
    const auto mouse = cui_mouse_pos(ui);
    const float mx = (mouse.x - ox) / scale, my = (mouse.y - oy) / scale;
    const auto hit = [&](float x, float y, float width, float height) {
        return mx >= x && mx < x + width && my >= y && my < y + height;
    };
    const auto hits = [&](const Image& image, int x, int y) { return hit(x, y, image.width, image.height); };
    const bool down = cui_mouse_clicked(ui, CUI_MOUSE_LEFT), up = cui_mouse_released(ui, CUI_MOUSE_LEFT);
    const bool alert = party.notice.id != 0;
    const bool modal = alert || party.action != PartyAction::None;
    const bool blocked = portraitNotice_ || client.showSignOut || client.returnToCharactersAt;
    if (drawDialog && modal && !blocked) {
        partyPressed_ = -1;
        partyDragging_ = false;
        const int x = 400 - dialog_[0].width / 2, y = 246 - dialog_[0].height / 2;
        int button = -1;
        if (alert) {
            if (hits(dialog_[1], x + 137, y + 188)) button = 0;
        } else {
            if (hits(selectionButtons_.at(1241), x + 41, y + 188)) button = 0;
            if (hits(selectionButtons_.at(1245), x + 234, y + 188)) button = 1;
        }
        if (partyConfirmPressed_ >= 0 && partyConfirmPressed_ != button) partyConfirmPressed_ = -1;
        if (down) partyConfirmPressed_ = button;
        const bool tab = cui_key_down(ui, CUI_KEY_TAB);
        if (!alert && !party.tabDown && (tab || cui_key_pressed(ui,CUI_KEY_TAB)))
            party.cancelSelected = !party.cancelSelected;
        party.tabDown = tab;
        const bool key = (alert && cui_key_pressed(ui,CUI_KEY_ENTER)) ||
            ((alert || !party.editorFocused) && cui_key_pressed(ui,CUI_KEY_SPACE));
        const bool click = up && button >= 0 && button == partyConfirmPressed_;
        if (up) partyConfirmPressed_ = -1;
        if (click && client.audioOptions.effects && std::filesystem::exists(clickSound_))
            client.audio.PlayEffect(clickSound_.c_str(), client.audioOptions);
        if (key || click) {
            const bool cancel = click ? button == 1 : party.cancelSelected;
            if (alert) party.notice = {};
            else if (cancel) party.Begin(PartyAction::None);
            else client.SubmitPartyMember();
            cui_request_redraw(ui);
        }
    }

    partyDrawPanel_ = party.open;
    partyDrawAlert_ = party.notice.id != 0;
    partyDrawModal_ = partyDrawAlert_ || party.action != PartyAction::None;
    partyDrawAllowed_ = party.allowed;
    if (!partyDrawPanel_ && !partyDrawModal_) {
        LeaveParty();
        return;
    }
    RefreshPartyText(ui, client);

    cui_overlay_begin(ui, {0, 0, viewport.x, viewport.y});
    const struct { const Assets* assets; bool panel, dialog; } paint{this,drawPanel,drawDialog};
    cui_canvas_paint(ui, CUI_FILL, CUI_FILL, [](cui_ctx* ui, cui_rect rect, void* model) {
        const auto& data = *static_cast<decltype(paint)*>(model);
        data.assets->PaintParty(ui,rect,data.panel,data.dialog);
    }, &paint, sizeof paint);
    if (drawDialog && !blocked && partyDrawModal_ && !partyDrawAlert_) {
        const int x = 400 - dialog_[0].width / 2 + 24, y = 246 - dialog_[0].height / 2 + 228;
        cui_overlay_begin(ui, {ox + x * scale, oy + y * scale, 309 * scale, 15 * scale});
        cui_push_id_int(ui, party.editor);
        const bool enter = cui_key_pressed(ui,CUI_KEY_ENTER);
        if (enter || partyEditor_ != party.editor || down)
            cui_text_field_focus(ui, "party-member", partyEditor_ != party.editor);
        partyEditor_ = party.editor;
        auto options = ModalEditorOptions(15*scale,scale);
        options.byte_limit = 90;
        options.measure_bytes = AnsiBytes;
        const int flags = cui_input_ex(ui, "party-member", party.name, sizeof party.name, &options);
        if (flags & CUI_FOCUSED) party.editorFocused = true;
        if (flags & CUI_BLURRED) party.editorFocused = false;
        if (enter) {
            if (party.cancelSelected) party.Begin(PartyAction::None);
            else client.SubmitPartyMember();
            cui_request_redraw(ui);
        }
        cui_pop_id(ui);
        cui_box_end(ui);
    }
    cui_box_end(ui);
}

void Dialogs::Assets::PaintParty(cui_ctx* ui, cui_rect rect, bool drawPanel, bool drawDialog) const
{
    const float scale = (std::min)(rect.w / 800, rect.h / 600);
    const float ox = rect.x + (rect.w - 800 * scale) / 2, oy = rect.y + (rect.h - 600 * scale) / 2;
    const auto bounds = [&](float x, float y, float w, float h) {
        return cui_rect{ox + x * scale, oy + y * scale, w * scale, h * scale};
    };
    const cui_color white{255,255,255,255}, pale{200,230,220,255}, yellow{255,250,200,255};
    const auto image = [&](const Image& frame, int x, int y, cui_color color) {
        if (frame.texture) cui_draw_image(ui, frame.texture, bounds(x,y,frame.width,frame.height), {0,0,1,1}, {}, color);
    };
    cui_push_clip(ui, bounds(0,0,800,600));
    if (drawPanel && partyDrawPanel_) {
        const int x = partyX_, y = partyY_;
        const auto& skin = partyFrames_.at(1360);
        image(skin, x - (skin.width - 258) / 2, y - (skin.height - 244) / 2, white);
        image(partyPlayer_, x + 52, y + 15, pale);
        for (int i = 0; i < static_cast<int>(partyRosterText_.size()); ++i)
            image(partyRosterText_[i], x + (i % 2 ? 45 : 145), y + 90 + 20 * (i / 2), white);
        image(partyAllowCaptions_[partyDrawAllowed_], x + 110, y + 52, pale);
        for (int i = 0; i < 5; ++i) {
            const auto& [dx, dy] = partyButtonOffsets_[i];
            if (i == 4) image(partyFrames_.at(partyDrawAllowed_ ? 1370 : 1371), x + dx, y + dy, white);
            else if (i == 0 || partyPressed_ == i)
                image(partyFrames_.at(partyButtonFrames_[i] + (partyPressed_ == i)), x + dx, y + dy, white);
        }
        if (!partyDrawModal_ && partyHint_ >= 0) {
            const auto& hint = partyHints_[partyHint_];
            const auto& button = partyFrames_.at(partyButtonFrames_[partyHint_]);
            const auto& [dx, dy] = partyButtonOffsets_[partyHint_];
            HudHint::Paint(ui, HudPoint(rect), hint, partyHintDrawHeight_, x + dx + button.width, y + dy + button.height);
        }
    }
    if (drawDialog && partyDrawModal_) {
        const int x = 400 - dialog_[0].width / 2, y = 246 - dialog_[0].height / 2;
        image(dialog_[0], x, y, white);
        if (partyDrawAlert_) {
            image(partyCaption_, x + 23, y + 84, yellow);
            image(dialog_[partyConfirmPressed_ == 0 ? 2 : 1], x + 137, y + 188, white);
        } else {
            image(partyCaption_, x + 43, y + 84, yellow);
            image(selectionButtons_.at(partyConfirmPressed_ == 0 ? 1242 : 1241), x + 41, y + 188, white);
            image(selectionButtons_.at(partyConfirmPressed_ == 1 ? 1246 : 1245), x + 234, y + 188, white);
            image(partyErrorImage_, x + 23, y + 153, yellow);
        }
    }
    cui_pop_clip(ui);
}


void Dialogs::Assets::StartTransition(cui_ctx* ui, LoginClient& client, MoviePurpose purpose, const wchar_t* filename)
{
    if (purpose == MoviePurpose::Create || purpose == MoviePurpose::Game) {
        const auto sound = clickSound_.parent_path() /
            (purpose == MoviePurpose::Create ? L"CreateChr.wav" : L"StartGame.wav");
        std::error_code error;
        if (client.audioOptions.effects && std::filesystem::is_regular_file(sound, error))
            client.audio.PlayEffect(sound.c_str(), client.audioOptions);
    }
    const auto path = movieDirectory_ / filename;
    std::error_code error;
    if (!std::filesystem::is_regular_file(path, error)) return;
    auto movie = std::make_unique<mir3::LegacyMovie>();
    if (!movie->open(path, purpose == MoviePurpose::Return, client.audioOptions.effects)) return;
    movieSurface_ = Upload(ui, movie->width(), movie->height(), movie->pixels());
    if (!movieSurface_.texture) return;
    moviePurpose_ = purpose;
    movieClock_ = cui_time(ui);
    const auto& audio = movie->audio();
    client.audio.PauseMusic(true);
    client.audio.PlayMovieAudio(audio.pcm, audio.channels, audio.rate, audio.bits, client.audioOptions);
    movie_ = std::move(movie);
}

void Dialogs::Assets::StopTransition(cui_ctx* ui, LoginClient& client)
{
    Destroy(ui, movieSurface_);
    movie_.reset();
    moviePurpose_ = MoviePurpose::None;
    movieFinished_ = false;
    client.audio.StopMovieAudio();
    client.audio.PauseMusic(false);
}

bool Dialogs::Assets::DrawTransition(cui_ctx* ui, LoginClient& client)
{
    const auto previous = std::exchange(moviePreviousStage_, client.stage);
    const bool creating = client.stage == Stage::Characters && client.characterDialog == CharacterDialog::Create;
    const bool wasCreating = std::exchange(movieWasCreating_, creating);
    if (movie_) {
        const bool activePage = moviePurpose_ == MoviePurpose::Opening ? client.stage == Stage::Login :
            moviePurpose_ == MoviePurpose::Game ? client.stage == Stage::ConnectingGame :
            moviePurpose_ == MoviePurpose::Create ? creating : client.stage == Stage::Characters && !creating;
        if (!activePage || movieFinished_) StopTransition(ui, client);
    }
    if (!movie_) {
        if (client.stage == Stage::Login && (!previous || previous == Stage::Groups))
            StartTransition(ui, client, MoviePurpose::Opening, L"wemade.dat");
        else if (client.stage == Stage::ConnectingGame && previous == Stage::Characters)
            StartTransition(ui, client, MoviePurpose::Game, L"StartGame.dat");
        else if (client.stage == Stage::Characters && creating != wasCreating)
            StartTransition(ui, client, creating ? MoviePurpose::Create : MoviePurpose::Return, L"CreateChr.dat");
    }
    if (!movie_) return false;

    const auto now = cui_time(ui);
    if (movie_->advance((now - movieClock_) * 1000)) {
        if (!cui_image_update(ui, movieSurface_.texture, movie_->width(), movie_->height(), movie_->pixels().data())) {
            Destroy(ui, movieSurface_);
            movieSurface_ = Upload(ui, movie_->width(), movie_->height(), movie_->pixels());
        }
    }
    movieClock_ = now;
    movieFinished_ = !movie_->playing() || !movieSurface_.texture;
    if (movieFinished_) cui_request_redraw(ui);
    else cui_request_wakeup(ui, 0.016f);
    auto* self = this;
    cui_box canvas{};
    canvas.width = CUI_FILL;
    canvas.height = CUI_FILL;
    cui_column_begin(ui, &canvas);
    cui_canvas(ui, "login-transition", CUI_FILL, CUI_FILL, [](cui_ctx* ui, cui_rect rect, void* model) {
        (*static_cast<Assets**>(model))->PaintTransition(ui, rect);
    }, &self, sizeof self);
    cui_box_end(ui);
    return true;
}

void Dialogs::Assets::PaintTransition(cui_ctx* ui, cui_rect rect) const
{
    cui_draw_rect(ui, rect, {}, {0, 0, 0, 255});
    const float scale = Scale(rect);
    const float w = 640 * scale, h = 480 * scale;
    const cui_rect frame{rect.x + (rect.w - w) / 2, rect.y + (rect.h - h) / 2, w, h};
    cui_draw_image(ui, movieSurface_.texture, frame, {0, 0, 1, 1}, {}, {255, 255, 255, 255});
}

Dialogs::Dialogs() : assets_(std::make_unique<Assets>()) {}
Dialogs::~Dialogs() = default;

bool Dialogs::Open(cui_ctx* ui, const std::filesystem::path& resourceRoot)
{
    Close(ui);
    return assets_->Open(ui, resourceRoot);
}

void Dialogs::Close(cui_ctx* ui)
{
    assets_->Close(ui);
}

bool Dialogs::DefersGameEntry() const
{
    return assets_->DefersGameEntry();
}

namespace {
int Input(cui_ctx* ui, const char* label, char* value, size_t capacity, bool password = false,
    bool autofocus = false)
{
    cui_input_opts options = {};
    options.label = label;
    options.password = password;
    options.autofocus = autofocus;
    options.width = CUI_FILL;
    return cui_input_ex(ui, label, value, capacity, &options);
}

bool Button(cui_ctx* ui, const char* label, bool primary = false, bool fullWidth = false)
{
    cui_button_opts options = {};
    options.variant = primary ? CUI_BUTTON_PRIMARY : CUI_BUTTON_SECONDARY;
    if (fullWidth) options.width = CUI_FILL;
    return cui_button_ex(ui, label, &options);
}

std::string BillingText(const BillingNotice& notice)
{
    const auto amount = std::to_string(notice.argument);
    switch (notice.message) {
    case 102: return "The gateway reports a billing notice for this account.";
    case 103: return "The gateway reports a billing balance of " + amount + " for this account.";
    case 105: return "The gateway reports a subscription notice for this account.";
    case 104: return "The gateway reports a subscription balance of " + amount + " for this account.";
    case 106: return "The gateway reports a remaining balance of " + amount + ".";
    case 107: return "The gateway reports an account balance of " + amount + ".";
    default: return {};
    }
}

void DrawServerGroups(cui_ctx* ui, LoginClient& client)
{
    auto& model = client.serverGroups;
    cui_box toolbar = {};
    toolbar.width = CUI_FILL;
    toolbar.align = CUI_ALIGN_CENTER;
    toolbar.justify = CUI_JUSTIFY_BETWEEN;
    cui_row_begin(ui, &toolbar);
    cui_heading(ui, "Server groups", 1);
    if (Button(ui, "Exit")) cui_app_quit(nullptr);
    cui_box_end(ui);

    cui_box panes = {};
    panes.width = CUI_FILL;
    panes.height = CUI_FILL;
    panes.gap = 18;
    cui_row_begin(ui, &panes);

    const ServerGroup* group = model.selected < 0 ? nullptr : &model.groups[model.selected];
    cui_box lines = {};
    lines.width = CUI_PCT(44);
    lines.height = CUI_FILL;
    lines.gap = 10;
    cui_column_begin(ui, &lines);
    const std::string title = group && !group->title.empty() ? group->title : "Server lines";
    cui_heading(ui, title.c_str(), 2);
    cui_scroll_opts list = {};
    list.id = "group-lines";
    list.width = CUI_FILL;
    list.height = CUI_FILL;
    list.gap = 6;
    cui_scroll_begin(ui, &list);
    if (!group) {
        cui_secondary(ui, "Choose a server group.");
    } else if (group->lines.empty()) {
        cui_secondary(ui, "This group has no server lines.");
    } else {
        for (size_t i = 0; i < group->lines.size(); ++i) {
            const auto& line = group->lines[i];
            cui_push_id_int(ui, static_cast<int>(i));
            cui_push_disabled(ui, line.loginSection.empty() || IniKey(line.loginSection) == "empty");
            const auto label = line.caption + "###line";
            if (Button(ui, label.c_str(), false, true) && client.OpenGroupLine(line))
                cui_request_redraw(ui);
            cui_pop_disabled(ui);
            cui_pop_id(ui);
        }
    }
    cui_scroll_end(ui);
    if (!client.status.empty()) cui_secondary(ui, client.status.c_str());
    cui_box_end(ui);

    list.id = "groups";
    cui_scroll_begin(ui, &list);
    bool groupChanged = false;
    for (size_t i = 0; i < model.groups.size(); ++i) {
        cui_push_id_int(ui, static_cast<int>(i));
        const auto& entry = model.groups[i];
        const auto label = (entry.label.empty() ? entry.title : entry.label) + "###group";
        if (Button(ui, label.c_str(), model.selected == static_cast<int>(i), true)) {
            model.selected = static_cast<int>(i);
            client.status.clear();
            groupChanged = true;
        }
        cui_pop_id(ui);
    }
    cui_scroll_end(ui);
    if (groupChanged) {
        cui_scroll_to(ui, "group-lines", 0);
        cui_request_redraw(ui);
    }
    cui_box_end(ui);
}

const char* const professions[] = {"Warrior", "Wizard", "Taoist"};
const char* const genders[] = {"Male", "Female"};

void DrawCharacters(cui_ctx* ui, LoginClient& client)
{
    cui_heading(ui, "Choose a character", 1);
    const bool blocked = client.characterRequest || !client.characterNotice.empty() ||
        client.characterDialog != CharacterDialog::None;
    cui_push_disabled(ui, blocked);

    cui_box row = {};
    row.width = CUI_FILL;
    row.gap = 12;
    cui_row_begin(ui, &row);
    const int first = client.characterPage * 2;
    for (int i = first; i < first + 2 && i < static_cast<int>(client.characters.size()); ++i) {
        const auto& role = client.characters[i];
        cui_push_id_int(ui, i);
        cui_card_opts card = {};
        card.width = CUI_FILL;
        card.pad = 16;
        card.gap = 8;
        card.selected = i == client.characterIndex;
        cui_card_begin(ui, &card);
        if (Button(ui, (role.label + "###role").c_str(), card.selected, true))
            client.characterIndex = i;
        cui_textf(ui, "%s / %s", professions[role.profession], genders[role.gender]);
        cui_textf(ui, "Level %u", role.level);
        cui_card_end(ui);
        cui_pop_id(ui);
    }
    cui_box_end(ui);

    if (client.characters.size() > 2) {
        cui_row_begin(ui, &row);
        if (Button(ui, "Previous page") && client.characterPage == 1) {
            client.characterPage = 0;
            client.characterIndex = 0;
        }
        cui_spacer(ui);
        if (Button(ui, "Next page") && client.characterPage == 0) {
            client.characterPage = 1;
            client.characterIndex = 2;
        }
        cui_box_end(ui);
    }
    cui_spacer(ui);
    cui_row_begin(ui, &row);
    if (Button(ui, "Create")) client.OpenCharacterDialog(CharacterDialog::Create);
    if (Button(ui, "Delete")) client.OpenCharacterDialog(CharacterDialog::Delete);
    if (Button(ui, "Start", true)) client.StartCharacter();
    if (Button(ui, "Back to sign-in")) client.Reset();
    cui_box_end(ui);
    cui_pop_disabled(ui);
    if (client.characterRequest) cui_secondary(ui, "Waiting for the character gateway...");
}

void DrawCharacterDialog(cui_ctx* ui, LoginClient& client, cui_dialog_opts modal)
{
    bool open = true;
    if (!client.characterNotice.empty()) {
        modal.description = client.characterNotice.c_str();
        if (cui_dialog_begin(ui, "Character notice", &open, &modal)) {
            cui_dialog_footer_begin(ui);
            const bool dismiss = Button(ui, "OK", true) || cui_key_pressed(ui, CUI_KEY_ENTER) ||
                cui_key_pressed(ui, CUI_KEY_SPACE);
            cui_dialog_footer_end(ui);
            cui_dialog_end(ui);
            if (dismiss) {
                client.characterNotice.clear();
                if (client.characterDialog == CharacterDialog::Create) {
                    SecureZeroMemory(client.characterInput, sizeof client.characterInput);
                    ++client.characterEditor;
                }
            }
        }
        return;
    }
    if (client.characterDialog == CharacterDialog::None) return;
    const bool creating = client.characterDialog == CharacterDialog::Create;
    const std::string prompt = creating ? "" : "Delete " + client.characters[client.characterIndex].label + "?";
    modal.description = prompt.c_str();
    if (!cui_dialog_begin(ui, creating ? "Create character" : "Delete character", &open, &modal)) return;
    cui_push_disabled(ui, client.characterRequest != 0);
    if (creating) {
        cui_label(ui, "Profession");
        cui_segmented_ex(ui, "profession", professions, 3, &client.profession, nullptr);
        cui_label(ui, "Appearance");
        cui_segmented_ex(ui, "appearance", genders, 2, &client.gender, nullptr);
    }
    cui_push_id_int(ui, client.characterEditor);
    const int input = Input(ui, creating ? "Name" : "Confirmation text", client.characterInput,
        sizeof client.characterInput, false, true);
    cui_pop_id(ui);
    if (!creating) {
        const bool tabDown = cui_key_down(ui, CUI_KEY_TAB);
        if (tabDown && !client.deleteTabDown) client.deleteCancelSelected = !client.deleteCancelSelected;
        client.deleteTabDown = tabDown;
    }
    const bool keyActivate = (input & CUI_SUBMITTED) || cui_key_pressed(ui, CUI_KEY_ENTER) ||
        (!creating && cui_key_pressed(ui, CUI_KEY_SPACE));
    cui_dialog_footer_begin(ui);
    const bool cancel = Button(ui, "Cancel", !creating && client.deleteCancelSelected);
    const bool confirm = Button(ui, creating ? "Create" : "Delete", creating || !client.deleteCancelSelected);
    cui_dialog_footer_end(ui);
    cui_pop_disabled(ui);
    cui_dialog_end(ui);
    if (client.characterRequest) return;
    if (!creating && keyActivate) {
        if (client.deleteCancelSelected) client.CloseCharacterDialog();
        else client.SubmitCharacterDialog();
    } else if (cancel) {
        client.CloseCharacterDialog();
    } else if (confirm || keyActivate) {
        client.SubmitCharacterDialog();
    }
}

void ChatShortcuts(cui_ctx* ui, mir3_cui::ChatPanel& chat, bool phrase)
{
    const auto& history = phrase ? chat.phrases : chat.contacts;
    if (history.empty()) return;
    auto& selected = phrase ? chat.phraseIndex : chat.contactIndex;
    const int last = static_cast<int>(history.size()) - 1;
    int target = -1;
    if (cui_key_pressed(ui, CUI_KEY_UP)) target = (std::max)(0, selected - 1);
    if (cui_key_pressed(ui, CUI_KEY_DOWN)) target = (std::min)(last, selected + 1);
    for (int i = 0; i <= last; ++i) {
        const auto digit = static_cast<cui_key>(CUI_KEY_0 + (i + 1) % 10);
        if (cui_key_pressed(ui, digit)) target = i;
    }
    cui_label(ui, phrase ? "Quick phrases" : "Recent contacts");
    for (int i = 0; i <= last; ++i) {
        cui_push_id_int(ui, i);
        const auto label = std::to_string((i + 1) % 10) + "  " + history[i] + "###shortcut";
        if (Button(ui, label.c_str(), i == selected)) target = i;
        cui_pop_id(ui);
    }
    if (target >= 0) {
        selected = target;
        chat.FillDraft(history[target]);
    }
}

void DrawItemShortcuts(cui_ctx* ui, LoginClient& client, bool active, bool bindHeld)
{
    auto& bag = client.inventory;
    cui_box row = {};
    row.width = CUI_FILL;
    row.gap = 5;
    row.wrap = true;
    cui_row_begin(ui, &row);
    for (std::size_t slot = 0; slot < bag.shortcuts.size(); ++slot) {
        const auto* item = bag.ResolveShortcut(slot);
        std::string title = std::to_string(slot + 1);
        if (item) {
            title += " " + item->label;
            if (item->Stacks()) title += " x" + std::to_string(item->count);
        } else if (!bag.shortcuts[slot].name.empty()) {
            title += " Unavailable";
        }
        title += "###quick-slot";
        cui_push_id_int(ui, static_cast<int>(slot));
        if (Button(ui, title.c_str()) && active) {
            const auto* held = bindHeld && client.equipment.heldSlot < 0 ? client.HeldItem() : nullptr;
            if (held) {
                bag.selection = held->key;
                client.BindItemShortcut(slot);
                if (bag.shortcuts[slot] == held->key) client.equipment.ReleaseHeld();
            } else {
                const auto now = GetTickCount64();
                if (bag.clickedShortcut == static_cast<int>(slot) && now - bag.shortcutClickTime <= 300) {
                    client.UseItemShortcut(slot);
                    bag.clickedShortcut = -1;
                } else {
                    bag.clickedShortcut = static_cast<int>(slot);
                    bag.shortcutClickTime = now;
                }
            }
        }
        if (active && cui_last_hovered(ui) && cui_mouse_clicked(ui, CUI_MOUSE_RIGHT))
            client.ClearItemShortcut(slot);
        cui_pop_id(ui);
    }
    cui_box_end(ui);
    if (!bag.shortcutStatus.empty()) cui_secondary(ui, bag.shortcutStatus.c_str());
    else cui_muted(ui, "Double-click to use. Right-click to clear.");
}

void DrawLocalScene(cui_ctx* ui, LoginClient& client, bool interactive, bool nativeInventory,
    bool legacyHud = false, SceneMapArt art = {});

void DrawGameChat(cui_ctx* ui, LoginClient& client, bool partyBlocksScene, bool questBlocksScene,
                  bool guildBlocksScene, bool npcBlocksScene, bool shopBlocksScene,
                  bool nativeInventory, bool inventoryBlocksScene, bool nativeQuick, bool quickBlocksScene,
                  bool independentCovered)
{
    auto& chat = client.chat;
    const auto hasOverlay = [&](bool scene = false, bool ignoreBag = false) {
        return client.showSignOut || independentCovered || chat.receptionOpen || client.serverOptions.open || client.audioOpen ||
            (client.party.open && (!scene || partyBlocksScene)) || client.party.notice.id ||
            client.party.action != mir3_cui::PartyAction::None ||
            (client.guild.open && (!scene || guildBlocksScene)) || client.guild.requested ||
            client.guild.notice || client.guild.action != mir3_cui::GuildAction::None ||
            (client.quests.open && (!scene || questBlocksScene)) ||
            (!ignoreBag && client.inventory.open && (!scene || inventoryBlocksScene)) ||
            client.inventory.notice || client.inventory.bindingShortcut ||
            client.inventory.drop != mir3_cui::DropDialog::None || client.merchant.notice ||
            client.merchant.quantityOpen || client.equipment.open ||
            (scene && quickBlocksScene) ||
            (client.shop.actor && (!scene || shopBlocksScene)) ||
            (client.npcDialogue.actor && (!scene || npcBlocksScene));
    };
    const bool hadOverlay = hasOverlay();
    const bool bagAndShortcuts = nativeInventory && client.inventory.open && !client.merchant.actor &&
        !hasOverlay(false,true);
    cui_push_disabled(ui, hadOverlay);
    cui_box row = {};
    row.width = CUI_FILL;
    row.gap = 10;
    row.align = CUI_ALIGN_CENTER;
    const auto& stats = client.characterStatus;
    if (stats.received) {
        cui_row_begin(ui, &row);
        cui_textf(ui, "Level %u", stats.level);
        cui_textf(ui, "HP %u / %u", stats.hp[0], stats.hp[1]);
        cui_textf(ui, "MP %u / %u", stats.mp[0], stats.mp[1]);
        cui_box_end(ui);
    }
    cui_box toolbar = row;
    toolbar.wrap = true;
    cui_row_begin(ui, &toolbar);
    cui_heading(ui, "Chat", 1);
    cui_spacer(ui);
    if (Button(ui, chat.expanded ? "Compact" : "Expand")) chat.expanded = !chat.expanded;
    if (Button(ui, "Reception")) chat.receptionOpen = true;
    if (Button(ui, "Party") || (!chat.composing && !hadOverlay && cui_shortcut(ui, 0, CUI_KEY_G)))
        client.party.open = true;
    if (Button(ui, "Guild") || (!chat.composing && !hadOverlay && cui_shortcut(ui, 0, CUI_KEY_F)))
        client.RequestGuildPage(0);
    if (Button(ui, "Quests") || (!chat.composing && !hadOverlay && cui_shortcut(ui, 0, CUI_KEY_D)))
        client.OpenQuests();
    if (Button(ui, "Inventory") || (!chat.composing && !hadOverlay && cui_shortcut(ui, 0, CUI_KEY_W)))
        client.OpenInventory();
    if (nativeInventory && !chat.composing && hadOverlay && !client.inventory.open &&
        (client.shop.actor || client.npcDialogue.actor) && !client.merchant.notice &&
        client.npcDialogue.prompt.command.empty() && cui_shortcut(ui,0,CUI_KEY_W)) client.OpenInventory();
    if (Button(ui, "Equipment") || (!chat.composing && !hadOverlay && cui_shortcut(ui, 0, CUI_KEY_Q)))
        client.OpenEquipment();
    if (Button(ui, client.inventory.shortcutsVisible ? "Hide shortcuts" : "Item shortcuts") ||
        (!chat.composing && (!hadOverlay || bagAndShortcuts) && cui_shortcut(ui, 0, CUI_KEY_Z)))
        client.ToggleItemShortcuts();
    if (Button(ui, "Server options") ||
        (!chat.composing && !hadOverlay && cui_shortcut(ui, 0, CUI_KEY_N)))
        client.serverOptions.open = true;
    if (Button(ui, "Audio")) client.audioOpen = true;
    if (Button(ui, "Sign out")) client.showSignOut = true;
    cui_box_end(ui);
    const bool overlay = hasOverlay();
    if (client.characterAppearance.available) {
        cui_pop_disabled(ui);
        const bool sceneBlocked = hasOverlay(true);
        cui_push_disabled(ui, sceneBlocked);
        DrawLocalScene(ui, client, !sceneBlocked, nativeInventory);
        cui_pop_disabled(ui);
        cui_push_disabled(ui, hadOverlay);
    }
    if ((!overlay || (nativeQuick && bagAndShortcuts)) && !chat.composing) {
        for (std::size_t slot = 0; slot < client.inventory.shortcuts.size(); ++slot) {
            if (cui_shortcut(ui, 0, static_cast<cui_key>(CUI_KEY_1 + slot))) {
                if (nativeQuick) client.ActivateShortcutSlot(slot,false);
                else client.UseItemShortcut(slot);
            }
        }
    }
    if (!nativeQuick && client.inventory.shortcutsVisible) {
        const bool alongsideBag = bagAndShortcuts && !inventoryBlocksScene;
        if (alongsideBag) cui_pop_disabled(ui);
        DrawItemShortcuts(ui, client, !overlay || alongsideBag, nativeInventory);
        if (alongsideBag) cui_push_disabled(ui,hadOverlay);
    }

    cui_scroll_opts list = {};
    list.id = "chat-history";
    list.width = CUI_FILL;
    const bool inlineShortcuts = !nativeQuick && client.inventory.shortcutsVisible;
    const float compactHeight = client.characterAppearance.available ?
        (inlineShortcuts ? 42.0f : 62.0f) : (inlineShortcuts ? 106.0f : 174.0f);
    list.height = chat.expanded ? CUI_FILL : CUI_PX(compactHeight);
    list.gap = 6;
    if (chat.scrollToEnd) {
        cui_scroll_to(ui, list.id, 100000.0f);
        chat.scrollToEnd = false;
    }
    cui_scroll_begin(ui, &list);
    for (const auto& line : chat.lines) {
        const auto text = std::string("[") + mir3_cui::ChatChannel(line.opcode) + "] " + line.text;
        cui_text_ex(ui, text.c_str(), nullptr);
    }
    if (chat.lines.empty()) cui_secondary(ui, "No messages yet.");
    cui_scroll_end(ui);

    const int mods = cui_key_mods(ui);
    if (chat.composing && !overlay && (mods & (CUI_MOD_CTRL | CUI_MOD_ALT))) {
        cui_box shortcuts = {};
        shortcuts.width = CUI_FILL;
        shortcuts.gap = 6;
        shortcuts.wrap = true;
        cui_row_begin(ui, &shortcuts);
        ChatShortcuts(ui, chat, (mods & CUI_MOD_ALT) != 0);
        cui_box_end(ui);
    }

    if (!chat.composing) {
        if (Button(ui, "Write a message") || (!overlay &&
            (cui_key_pressed(ui, CUI_KEY_ENTER) || cui_key_pressed(ui, CUI_KEY_SPACE)))) {
            chat.composing = true;
            ++chat.editor;
        }
    } else {
        const char* const channels[] = {"Nearby", "Party", "Guild"};
        cui_segmented_ex(ui, "chat-channel", channels, 3, &chat.channel, nullptr);
        cui_row_begin(ui, &row);
        cui_box editor = {};
        editor.width = CUI_FILL;
        cui_column_begin(ui, &editor);
        cui_push_id_int(ui, chat.editor);
        const int input = Input(ui, "Message", chat.draft, sizeof chat.draft, false, true);
        cui_pop_id(ui);
        cui_box_end(ui);
        const bool send = Button(ui, "Send", true);
        cui_box_end(ui);
        if (!overlay && cui_key_pressed(ui, CUI_KEY_ESCAPE)) {
            chat.composing = false;
        } else if (!overlay && (send || (input & CUI_SUBMITTED))) {
            client.SendChat();
        }
    }
    if (!chat.error.empty()) cui_secondary(ui, chat.error.c_str());
    cui_pop_disabled(ui);
}

void DrawChatReception(cui_ctx* ui, LoginClient& client)
{
    auto& chat = client.chat;
    if (!cui_dialog_begin(ui, "Chat reception", &chat.receptionOpen, nullptr)) return;
    static const char* const labels[] = {"Block private messages", "Block guild chat", "Block all chat"};
    for (std::size_t i = 0; i < chat.reception.displayed.size(); ++i) {
        bool blocked = chat.reception.displayed[i];
        if (cui_checkbox_ex(ui, labels[i], &blocked, nullptr))
            client.ToggleChatReception(static_cast<mir3_cui::ReceptionChannel>(i));
        if (chat.reception.displayed[i] != chat.reception.confirmed[i])
            cui_muted(ui, "Waiting for server confirmation");
    }
    if (!chat.reception.error.empty()) cui_secondary(ui, chat.reception.error.c_str());
    cui_dialog_footer_begin(ui);
    if (Button(ui, "Done", true)) chat.receptionOpen = false;
    cui_dialog_footer_end(ui);
    cui_dialog_end(ui);
}

void DrawParty(cui_ctx* ui, LoginClient& client)
{
    using mir3_cui::PartyAction;
    auto& party = client.party;
    cui_dialog_opts dialog = {};
    dialog.persistent = true;
    dialog.no_close_button = true;
    bool open = true;

    if (party.notice.id) {
        const bool crafting = party.notice.id >= 1440 && party.notice.id <= 1444;
        if (!cui_dialog_begin(ui, crafting ? "Crafting" : "Party notice", &open, &dialog)) return;
        if (party.notice.id == 1440) cui_label(ui, "Crafting complete.");
        else cui_textf(ui, "%s failed (server code %d).", party.notice.action, party.notice.reason);
        cui_dialog_footer_begin(ui);
        const bool dismiss = Button(ui, "OK", true) || cui_key_pressed(ui, CUI_KEY_ENTER) ||
            cui_key_pressed(ui, CUI_KEY_SPACE);
        cui_dialog_footer_end(ui);
        cui_dialog_end(ui);
        if (dismiss) party.notice = {};
        return;
    }

    if (party.action != PartyAction::None) {
        const char* title = party.action == PartyAction::Create ? "Create party" :
            party.action == PartyAction::Invite ? "Invite member" : "Remove member";
        if (!cui_dialog_begin(ui, title, &open, &dialog)) return;
        cui_push_id_int(ui, party.editor);
        const int input = Input(ui, "Character name", party.name, sizeof party.name, false, true);
        cui_pop_id(ui);
        if (input & CUI_FOCUSED) party.editorFocused = true;
        if (input & CUI_BLURRED) party.editorFocused = false;
        const bool tab = cui_key_down(ui, CUI_KEY_TAB);
        if (tab && !party.tabDown) party.cancelSelected = !party.cancelSelected;
        party.tabDown = tab;
        const bool activate = (input & CUI_SUBMITTED) || cui_key_pressed(ui, CUI_KEY_ENTER) ||
            (!party.editorFocused && cui_key_pressed(ui, CUI_KEY_SPACE));
        if (!party.error.empty()) cui_secondary(ui, party.error.c_str());
        cui_dialog_footer_begin(ui);
        const bool cancel = Button(ui, "Cancel", party.cancelSelected);
        const bool confirm = Button(ui, "Confirm", !party.cancelSelected);
        cui_dialog_footer_end(ui);
        cui_dialog_end(ui);
        if (activate) {
            if (party.cancelSelected) party.Begin(PartyAction::None);
            else client.SubmitPartyMember();
        } else if (cancel) {
            party.Begin(PartyAction::None);
        } else if (confirm) {
            client.SubmitPartyMember();
        }
        return;
    }

    if (!cui_dialog_begin(ui, "Party", &party.open, nullptr)) return;
    cui_label(ui, client.gameCharacter.c_str());
    bool allowed = party.allowed;
    if (cui_checkbox_ex(ui, "Allow party invitations", &allowed, nullptr)) client.SetPartyAllowed(allowed);
    if (party.members.empty()) {
        cui_secondary(ui, "No party members.");
    } else {
        cui_scroll_opts list = {};
        list.id = "party-roster";
        list.width = CUI_FILL;
        list.height = CUI_PX(144);
        list.gap = 6;
        cui_scroll_begin(ui, &list);
        for (const auto& member : party.members) cui_text_ex(ui, member.c_str(), nullptr);
        cui_scroll_end(ui);
    }
    if (!party.error.empty()) cui_secondary(ui, party.error.c_str());
    cui_box row = {};
    row.width = CUI_FILL;
    row.gap = 8;
    row.wrap = true;
    cui_row_begin(ui, &row);
    if (Button(ui, "Create party")) party.Begin(PartyAction::Create);
    if (Button(ui, "Invite member")) party.Begin(PartyAction::Invite);
    if (Button(ui, "Remove member")) party.Begin(PartyAction::Remove);
    cui_box_end(ui);
    cui_dialog_footer_begin(ui);
    if (Button(ui, "Done", true) || cui_shortcut(ui, 0, CUI_KEY_G)) party.open = false;
    cui_dialog_footer_end(ui);
    cui_dialog_end(ui);
}

const char* GuildActionTitle(mir3_cui::GuildAction action)
{
    using mir3_cui::GuildAction;
    switch (action) {
    case GuildAction::Add: return "Add member";
    case GuildAction::Remove: return "Remove member";
    case GuildAction::Notice: return "Edit announcement";
    case GuildAction::Ranks: return "Edit ranks";
    case GuildAction::BreakAlliance: return "End alliance";
    default: return "";
    }
}

void DrawGuild(cui_ctx* ui, LoginClient& client)
{
    using mir3_cui::GuildAction;
    auto& guild = client.guild;
    cui_dialog_opts modal = {};
    modal.width = 530;
    bool open = true;

    if (guild.notice) {
        modal.persistent = true;
        modal.no_close_button = true;
        if (!cui_dialog_begin(ui, "Guild notice", &open, &modal)) return;
        cui_text_ex(ui, mir3_cui::GuildNoticeText(guild.notice), nullptr);
        cui_dialog_footer_begin(ui);
        const bool dismiss = Button(ui, "OK", true) || cui_key_pressed(ui, CUI_KEY_ENTER) ||
            cui_key_pressed(ui, CUI_KEY_SPACE);
        cui_dialog_footer_end(ui);
        cui_dialog_end(ui);
        if (dismiss) guild.notice = 0;
        return;
    }

    if (guild.action != GuildAction::None) {
        modal.persistent = true;
        modal.no_close_button = true;
        if (!cui_dialog_begin(ui, GuildActionTitle(guild.action), &open, &modal)) return;
        cui_push_id_int(ui, guild.editor);
        int input = 0;
        if (guild.Multiline()) {
            cui_input_opts options = {};
            options.label = guild.action == GuildAction::Notice ? "Announcement" : "Ranks";
            options.height = 180;
            options.width = CUI_FILL;
            options.autofocus = true;
            cui_textarea_ex(ui, "guild-editor", guild.input, sizeof guild.input, &options);
        } else {
            input = Input(ui, "Name", guild.input, sizeof guild.input, false, true);
        }
        cui_pop_id(ui);
        if (!guild.error.empty()) cui_secondary(ui, guild.error.c_str());
        cui_dialog_footer_begin(ui);
        const bool cancel = Button(ui, "Cancel");
        const bool confirm = Button(ui, "Confirm", true);
        cui_dialog_footer_end(ui);
        cui_dialog_end(ui);
        if (cancel) guild.Begin(GuildAction::None);
        else if (confirm || (input & CUI_SUBMITTED)) client.SubmitGuildAction();
        return;
    }

    if (guild.requested && !guild.open) {
        if (cui_dialog_begin(ui, "Guild", &guild.requested, &modal)) {
            cui_secondary(ui, "Waiting for the guild overview...");
            if (!guild.error.empty()) cui_secondary(ui, guild.error.c_str());
            if (Button(ui, "Cancel")) guild.requested = false;
            cui_dialog_end(ui);
        }
        return;
    }

    if (!cui_dialog_begin(ui, "Guild", &guild.open, &modal)) return;
    const auto name = ConvertText(guild.overview.name, 936, CP_UTF8);
    cui_heading(ui, name.c_str(), 2);
    const char* const tabs[] = {"Overview", "Members", "History"};
    cui_box row = {};
    row.width = CUI_FILL;
    row.gap = 8;
    row.wrap = true;
    cui_row_begin(ui, &row);
    for (int tab = 0; tab < 3; ++tab) {
        if (Button(ui, tabs[tab], guild.tab == tab)) client.RequestGuildPage(tab);
    }
    cui_box_end(ui);
    const auto& rows = guild.tab == 0 ? guild.overview.rows : guild.roster.rows;
    cui_scroll_opts list = {};
    const char* const ids[] = {"guild-home", "guild-members", "guild-history"};
    list.id = ids[guild.tab];
    list.width = CUI_FILL;
    list.height = CUI_PX(150);
    list.gap = 4;
    cui_scroll_begin(ui, &list);
    if (guild.tab == 2) {
        if (guild.history.empty()) cui_secondary(ui, "No entries.");
        for (const auto& entry : guild.history)
            cui_text_ex(ui, entry.messageId ? mir3_cui::GuildNoticeText(entry.messageId) : entry.text.c_str(), nullptr);
    } else {
        if (rows.empty()) cui_secondary(ui, "No entries.");
        for (const auto& row : rows) cui_text_ex(ui, ConvertText(row, 936, CP_UTF8).c_str(), nullptr);
    }
    cui_scroll_end(ui);
    if (!guild.error.empty()) cui_secondary(ui, guild.error.c_str());
    cui_row_begin(ui, &row);
    for (auto action : {GuildAction::Notice, GuildAction::Ranks, GuildAction::Add, GuildAction::Remove,
                        GuildAction::BreakAlliance}) {
        if (Button(ui, GuildActionTitle(action))) client.BeginGuildAction(action);
    }
    cui_box_end(ui);
    cui_dialog_footer_begin(ui);
    if (Button(ui, "Done", true) || cui_shortcut(ui, 0, CUI_KEY_F)) guild.open = false;
    cui_dialog_footer_end(ui);
    cui_dialog_end(ui);
}

void DrawQuests(cui_ctx* ui, LoginClient& client)
{
    auto& quests = client.quests;
    cui_dialog_opts dialog = {};
    dialog.width = 510;
    if (!cui_dialog_begin(ui, "Quests", &quests.open, &dialog)) return;
    cui_scroll_opts list = {};
    list.id = "quest-list";
    list.width = CUI_FILL;
    list.height = CUI_PX(150);
    list.gap = 6;
    if (quests.resetScroll) {
        cui_scroll_to(ui, list.id, 0);
        quests.resetScroll = false;
    }
    cui_scroll_begin(ui, &list);
    if (quests.entries.empty())
        cui_secondary(ui, quests.waiting ? "Waiting for the quest list..." : "No quests.");

    cui_box row = {};
    row.width = CUI_FILL;
    row.gap = 6;
    for (auto& entry : quests.entries) {
        const auto& quest = entry.record;
        cui_push_id_int(ui, quest.id);
        cui_row_begin(ui, &row);
        const std::string title = quest.title + (quest.completed ? " (Completed)" : "") + "###quest";
        if (Button(ui, title.c_str(), quests.parent == quest.id && quests.child < 0, true))
            client.SelectQuest(quest.id);
        if (quest.childCount || !entry.children.empty()) {
            if (Button(ui, quests.expanded == quest.id ? "Collapse" : "Expand"))
                client.ExpandQuest(entry);
        }
        cui_box_end(ui);
        if (quests.expanded == quest.id) {
            if (entry.children.empty()) cui_muted(ui, entry.childrenLoaded ? "No steps." : "Waiting for quest steps...");
            for (const auto& child : entry.children) {
                cui_push_id_int(ui, child.id);
                const std::string label = "  " + child.title + "###step";
                if (Button(ui, label.c_str(), quests.parent == quest.id && quests.child == child.id, true))
                    client.SelectQuest(quest.id, child.id);
                cui_pop_id(ui);
            }
        }
        cui_pop_id(ui);
    }
    cui_scroll_end(ui);
    cui_separator(ui);
    cui_label(ui, "Description");
    cui_scroll_opts description = {};
    description.id = "quest-description";
    description.width = CUI_FILL;
    description.height = CUI_PX(96);
    if (quests.resetDescription) {
        cui_scroll_to(ui, description.id, 0);
        quests.resetDescription = false;
    }
    cui_scroll_begin(ui, &description);
    const auto* text = quests.Description();
    if (text && !text->empty()) cui_text_ex(ui, text->c_str(), nullptr);
    else cui_secondary(ui, quests.parent < 0 ? "Select a quest." : "No description yet.");
    cui_scroll_end(ui);
    if (!quests.error.empty()) cui_secondary(ui, quests.error.c_str());
    cui_dialog_footer_begin(ui);
    if (Button(ui, "Refresh")) client.RequestQuests();
    if (Button(ui, "Done", true) || cui_shortcut(ui, 0, CUI_KEY_D)) quests.open = false;
    cui_dialog_footer_end(ui);
    cui_dialog_end(ui);
}

void DrawInventory(cui_ctx* ui, LoginClient& client, cui_rect bounds)
{
    using mir3_cui::DropDialog;
    using mir3_cui::ItemRequest;
    auto& bag = client.inventory;
    auto& merchant = client.merchant;
    if (merchant.notice || merchant.quantityOpen) {
        const bool alert = merchant.notice != 0;
        bool open = true;
        cui_dialog_opts modal{};
        modal.persistent = modal.no_close_button = true;
        if (cui_dialog_begin(ui,alert ? "Merchant" : "Quantity",&open,&modal)) {
            int input = 0;
            if (alert) {
                const char* text = "The merchant could not complete this request.";
                switch (merchant.notice) {
                case 1445: text = "The item could not be withdrawn."; break;
                case 1446: text = "Your bag is full."; break;
                case 1449: text = "The warehouse is full."; break;
                case 1450: text = "The item could not be deposited."; break;
                case 1455: text = "The sale failed."; break;
                case 1457: text = "The repair failed."; break;
                }
                cui_label(ui,text);
            }
            else input = Input(ui,"Quantity",merchant.quantity,sizeof merchant.quantity,false,true);
            cui_dialog_footer_begin(ui);
            const bool confirm = Button(ui,"OK",true) || (input & CUI_SUBMITTED) || cui_key_pressed(ui,CUI_KEY_ENTER);
            const bool cancel = !alert && (Button(ui,"Cancel") || cui_key_pressed(ui,CUI_KEY_ESCAPE));
            cui_dialog_footer_end(ui);
            cui_dialog_end(ui);
            if (alert && (confirm || cui_key_pressed(ui,CUI_KEY_SPACE))) merchant.notice = 0;
            else if (cancel) merchant.quantityOpen = false;
            else if (confirm) client.SubmitMerchantQuantity();
        }
        return;
    }
    if (bag.bindingShortcut) {
        if (!cui_dialog_begin(ui, "Bind item shortcut", &bag.bindingShortcut, nullptr)) return;
        const auto* selected = bag.Find(bag.selection);
        if (selected) cui_label(ui, selected->label.c_str());
        for (std::size_t slot = 0; slot < bag.shortcuts.size(); ++slot) {
            cui_push_id_int(ui, static_cast<int>(slot));
            const auto* item = bag.ResolveShortcut(slot);
            const auto label = "Bind " + std::to_string(slot + 1) +
                (item ? " (" + item->label + ')' : "") + "###bind-slot";
            cui_push_disabled(ui, !selected || !selected->QuickEligible());
            if (Button(ui, label.c_str()) || cui_shortcut(ui, 0, static_cast<cui_key>(CUI_KEY_1 + slot)))
                client.BindItemShortcut(slot);
            cui_pop_disabled(ui);
            cui_pop_id(ui);
        }
        cui_dialog_footer_begin(ui);
        if (Button(ui, "Cancel")) bag.bindingShortcut = false;
        cui_dialog_footer_end(ui);
        cui_dialog_end(ui);
        return;
    }
    if (bag.notice) {
        cui_dialog_opts alert = {};
        alert.persistent = true;
        alert.no_close_button = true;
        if (cui_dialog_begin(ui, "Item notice", &bag.notice, &alert)) {
            cui_text_ex(ui, bag.status.c_str(), nullptr);
            cui_dialog_footer_begin(ui);
            const bool dismiss = Button(ui, "OK", true) || cui_key_pressed(ui, CUI_KEY_ENTER) ||
                cui_key_pressed(ui, CUI_KEY_SPACE);
            cui_dialog_footer_end(ui);
            cui_dialog_end(ui);
            if (dismiss) {
                bag.notice = false;
                bag.status.clear();
            }
        }
        return;
    }
    if (bag.drop != DropDialog::None) {
        cui_dialog_opts confirm = {};
        confirm.persistent = true;
        confirm.no_close_button = true;
        bool open = true;
        const auto* item = bag.Find(bag.selection);
        const bool gold = bag.drop == DropDialog::Gold;
        if (!cui_dialog_begin(ui, gold ? "Drop gold" : "Drop item", &open, &confirm)) return;
        if (gold) cui_textf(ui, "Gold balance: %u", bag.gold);
        else if (item) cui_label(ui, item->label.c_str());

        const bool amount = gold || (item && item->Stacks() && item->count > 1);
        int input = 0;
        if (amount) {
            cui_push_id_int(ui, bag.editor);
            input = Input(ui, "Amount", bag.amount, sizeof bag.amount, false, true);
            cui_pop_id(ui);
            if (!gold) cui_muted(ui, "Up to 100 items per drop.");
        }
        if (!bag.status.empty()) cui_secondary(ui, bag.status.c_str());
        cui_dialog_footer_begin(ui);
        const bool cancel = Button(ui, "Cancel") || cui_key_pressed(ui, CUI_KEY_ESCAPE);
        const bool submit = Button(ui, "Confirm drop", true) || (input & CUI_SUBMITTED);
        cui_dialog_footer_end(ui);
        cui_dialog_end(ui);
        if (cancel) bag.drop = DropDialog::None;
        else if (submit) client.SubmitInventoryDrop();
        return;
    }

    if (!cui_panel_begin(ui, "Inventory", &bag.open, bounds)) return;
    cui_textf(ui, "Gold: %u", bag.gold);
    const bool equipmentPending = client.equipment.request != mir3_cui::EquipmentRequest::None;
    if (bag.waiting && bag.items.empty()) cui_muted(ui, "Waiting for the inventory...");
    cui_scroll_opts list = {};
    list.id = "inventory-grid";
    list.width = CUI_FILL;
    list.height = CUI_PX((std::max)(102.0f, (std::min)(270.0f, bounds.h - 280.0f)));
    list.gap = 4;
    cui_scroll_begin(ui, &list);
    constexpr int columns = mir3_cui::InventoryPanel::Columns;
    int cellCount = 8 * columns;
    for (const auto& item : bag.items) {
        if (item.cell >= 0)
            cellCount = (std::max)(cellCount, (item.cell / columns + item.height + 1) * columns);
    }
    cellCount = (std::min)(cellCount, mir3_cui::InventoryPanel::Cells);
    cui_box cells = {};
    cells.width = CUI_FILL;
    cells.gap = 4;
    for (int start = 0; start < cellCount; start += columns) {
        cui_row_begin(ui, &cells);
        for (int cell = start; cell < start + columns; ++cell) {
            const auto* item = bag.AtCell(cell);
            auto label = item ? item->label : std::string();
            if (item && item->Stacks()) label += " x" + std::to_string(item->count);
            const auto tooltip = label + (item && item->key == client.equipment.held ? " (held)" : "");
            if (item && item->cell != cell) label.clear();
            label += "###item";
            cui_button_opts slot = {};
            slot.size = CUI_SIZE_SM;
            slot.width = CUI_FILL;
            slot.tooltip = item ? tooltip.c_str() : "Empty slot";
            slot.disabled = equipmentPending || bag.request != ItemRequest::None;
            slot.variant = item && item->key == bag.selection ? CUI_BUTTON_PRIMARY : CUI_BUTTON_SECONDARY;
            slot.selected = item && item->cell != cell;
            cui_push_id_int(ui, cell);
            if (cui_button_ex(ui, label.c_str(), &slot)) {
                const auto rect = cui_last_rect(ui);
                const auto pointer = cui_mouse_pos(ui);
                if (merchant.actor) client.QuoteMerchantItem(cell);
                else client.ReleaseBagCell(cell, pointer.x > rect.x + rect.w / 2, pointer.y > rect.y + rect.h / 2);
            }
            cui_pop_id(ui);
        }
        cui_box_end(ui);
    }
    for (const auto& item : bag.items) {
        if (item.cell >= 0) continue;
        const auto label = item.label + " (unplaced)";
        cui_push_disabled(ui, equipmentPending || bag.request != ItemRequest::None || client.equipment.heldSlot >= 0);
        if (Button(ui, label.c_str())) {
            client.equipment.ReleaseHeld();
            client.equipment.held = item.key;
            bag.selection = item.key;
        }
        cui_pop_disabled(ui);
    }
    cui_scroll_end(ui);
    if (merchant.actor) {
        if (merchant.price) cui_textf(ui,"Price: %u",merchant.price);
        using Kind = mir3_cui::MerchantService::Kind;
        const char* action = merchant.kind == Kind::Deposit ? "Deposit" : merchant.kind == Kind::Sell ? "Sell" : "Repair";
        if (Button(ui,action,true)) client.ConfirmMerchantItem();
        cui_panel_end(ui);
        if (!bag.open) client.CloseInventory();
        return;
    }
    const auto* held = client.HeldItem();
    const bool holdingEquipment = held && client.equipment.heldSlot >= 0;
    const auto* selected = bag.Find(bag.selection);
    if (held) cui_textf(ui, "Holding: %s", held->label.c_str());
    else if (selected) cui_label(ui, selected->label.c_str());
    if (selected && !holdingEquipment && (selected->maxDurability || selected->durability)) {
        if (selected->maxDurability)
            cui_textf(ui, "Durability: %u / %u", selected->durability, selected->maxDurability);
        else
            cui_textf(ui, "Durability: %u", selected->durability);
    }
    if (bag.request != ItemRequest::None) cui_muted(ui, "Waiting for the server...");
    if (!client.equipment.status.empty()) cui_secondary(ui, client.equipment.status.c_str());
    if (!bag.status.empty()) cui_secondary(ui, bag.status.c_str());
    cui_box actions = {};
    actions.width = CUI_FILL;
    actions.gap = 8;
    actions.wrap = true;
    cui_row_begin(ui, &actions);
    cui_push_disabled(ui, equipmentPending || holdingEquipment || bag.request != ItemRequest::None ||
        !selected || selected->IsGold());
    if (Button(ui, "Use", true)) client.UseInventoryItem();
    if (Button(ui, "Drop item")) client.BeginInventoryDrop(DropDialog::Item);
    cui_pop_disabled(ui);
    cui_push_disabled(ui, equipmentPending || holdingEquipment || bag.request != ItemRequest::None ||
        !selected || !selected->QuickEligible());
    if (Button(ui, "Bind shortcut")) bag.bindingShortcut = true;
    cui_pop_disabled(ui);
    bool wearable = false;
    if (selected) {
        for (int slot = 0; slot < mir3_cui::EquipmentSlots; ++slot)
            wearable |= mir3_cui::EquipmentFits(selected->mode, slot);
    }
    cui_push_disabled(ui, equipmentPending || holdingEquipment || !wearable || bag.request != ItemRequest::None);
    if (Button(ui, "Equip")) client.OpenEquipment(true);
    cui_pop_disabled(ui);
    if (holdingEquipment) {
        cui_push_disabled(ui, equipmentPending);
        if (Button(ui, "Place in bag", true)) client.UnequipItem();
        if (!client.equipment.open && Button(ui, "Open equipment")) client.OpenEquipment(true);
        cui_pop_disabled(ui);
    }
    cui_push_disabled(ui, equipmentPending || holdingEquipment || bag.request != ItemRequest::None || bag.gold < 2);
    if (Button(ui, "Drop gold")) client.BeginInventoryDrop(DropDialog::Gold);
    cui_pop_disabled(ui);
    cui_box_end(ui);
    cui_dialog_footer_begin(ui);
    if (Button(ui, "Refresh")) client.RequestInventory();
    if (Button(ui, "Done", true)) bag.open = false;
    cui_dialog_footer_end(ui);
    cui_panel_end(ui);
}

struct EquipmentPortrait {
    mir3_cui::CharacterAppearance appearance;
    bool helmet = false;
};

void PaintCharacterFigure(cui_ctx* ui, cui_rect rect, const EquipmentPortrait& portrait, float scale = 1.0f)
{
    const auto& appearance = portrait.appearance;
    if (!appearance.available) {
        cui_draw_icon(ui, CUI_ICON_USER, rect, 20, {100, 116, 139, 255});
        return;
    }

    static const cui_color coats[] = {{100, 116, 139, 255}, {38, 104, 144, 255}, {43, 115, 94, 255},
        {126, 71, 108, 255}, {158, 89, 55, 255}, {65, 92, 153, 255}, {133, 92, 55, 255},
        {46, 113, 134, 255}, {140, 64, 71, 255}};
    static const cui_color hairColors[] = {{64, 47, 38, 255}, {47, 44, 59, 255}, {113, 67, 49, 255},
        {127, 91, 57, 255}, {92, 96, 115, 255}};
    const auto point = [&](float x, float y) { return cui_vec2{rect.x + x * scale, rect.y + y * scale}; };
    const float shoulder = appearance.body == 0 ? 10.0f : 8.0f;
    const cui_vec2 coat[] = {point(24 - shoulder, 26), point(24 + shoulder, 26), point(38, 37),
        point(34, 41), point(14, 41), point(10, 37)};
    cui_draw_polygon(ui, coat, 6, coats[appearance.clothing]);
    cui_draw_circle(ui, point(24, 17), 8.0f * scale, {229, 180, 143, 255});

    if (portrait.helmet) {
        const cui_vec2 cap[] = {point(15, 16), point(18, 9), point(30, 9), point(33, 16), point(29, 19), point(19, 19)};
        cui_draw_polygon(ui, cap, 6, {70, 86, 105, 255});
    } else {
        const float top = 8.0f + static_cast<float>(appearance.hair % 3);
        const cui_vec2 fringe[] = {point(16, 16), point(19, top), point(29, top), point(32, 16), point(25, 18)};
        cui_draw_polygon(ui, fringe, 5, hairColors[appearance.hair % 5]);
    }
    if (appearance.weapon) {
        cui_draw_line(ui, point(38, 37), point(48, 14), 3.0f * scale, {75, 99, 122, 255});
        cui_draw_line(ui, point(36, 29), point(44, 33), 2.0f * scale, {54, 71, 91, 255});
    }
}

void PaintEquipmentPortrait(cui_ctx* ui, cui_rect rect, void* user)
{
    cui_draw_rect(ui, rect, {10, 10, 10, 10}, {231, 237, 243, 255});
    PaintCharacterFigure(ui, rect, *static_cast<const EquipmentPortrait*>(user));
}

struct LocalSceneDrawing {
    EquipmentPortrait portrait;
    unsigned x, y, facing;
    LocalScene::Movement pending;
    bool terrainReady;
    bool legacyHud;
    SceneMapArt art;
    mir3_cui::MapTerrain::View ground;
    const std::map<std::uint32_t, LocalScene::Player>* players;
    char location[832];
    char name[128];
};

void PaintLocalScene(cui_ctx* ui, cui_rect rect, void* user)
{
    const auto& model = *static_cast<const LocalSceneDrawing*>(user);
    if (model.art.sprites) {
        cui_draw_rect(ui,rect,{}, {0,0,0,255});
        cui_push_clip(ui,rect);
        const float scale=rect.w/800;
        const auto drawSprite=[&](const SceneSprite& sprite) {
            if (sprite.effect) {
                const float x=rect.x+sprite.x*scale,y=rect.y+sprite.y*scale;
                const float r=x+sprite.width*scale,b=y+sprite.height*scale;
                const cui_vec2 quad[]{{x,y},{r,y},{r,b},{x,b}};
                constexpr cui_vec2 uv[]{{0,0},{1,0},{1,1},{0,1}};
                cui_draw_effect_quad(ui,sprite.texture,quad,uv,sprite.tint,*sprite.effect);
                return;
            }
            cui_draw_image(ui,sprite.texture,{rect.x+sprite.x*scale,rect.y+sprite.y*scale,
                sprite.width*scale,sprite.height*scale},{0,0,sprite.sourceWidth,1},{},{255,255,255,sprite.opacity});
        };
        const auto& sprites=*model.art.sprites;
        for (std::size_t i=0;i<model.art.overlayStart;++i) drawSprite(sprites[i]);
        const auto effects=[&](const std::vector<SceneEffectQuad>* quads) {
            if (!quads) return;
            constexpr cui_vec2 uprightUV[]{{0,0},{1,0},{1,1},{0,1}};
            constexpr cui_vec2 particleUV[]{{1,1},{1,0},{0,0},{0,1}};
            for (const auto& sprite:*quads) {
                cui_vec2 positions[4];
                for (int k=0;k<4;++k)
                    positions[k]={rect.x+sprite.corners[k].x*scale,rect.y+sprite.corners[k].y*scale};
                cui_draw_effect_quad(ui,sprite.texture,positions,sprite.upright ? uprightUV : particleUV,sprite.tint,sprite.blend);
            }
        };
        effects(model.art.weather);
        if (const auto* grid=model.art.light) {
            const auto triangle=[&](const std::array<int,3>& vertices) {
                cui_vec2 points[3];
                cui_color colors[3];
                for (int k=0;k<3;++k) {
                    const int index=vertices[k];
                    points[k]={rect.x+(index%18)*48*scale,rect.y+(index/18)*32*scale};
                    const auto& rgb=grid->samples[index];
                    colors[k]={std::uint8_t(rgb[0]),std::uint8_t(rgb[1]),std::uint8_t(rgb[2]),255};
                }
                cui_draw_multiply_triangle(ui,points,colors);
            };
            for (int y=0;y<18;++y) for (int x=0;x<17;++x) {
                const int a=y*18+x, b=a+1, d=a+18, c=d+1;
                if (grid->diagonals[y*17+x]) {
                    triangle({a,b,c});
                    triangle({a,c,d});
                } else {
                    triangle({a,b,d});
                    triangle({b,c,d});
                }
            }
        }
        effects(model.art.levelEffects);
        effects(model.art.spellEffects);
        const auto captions=[&](const std::vector<SceneCaption>* lines) {
            if (!lines) return;
            for (const auto& line:*lines) {
                const auto [x,y,w,h]=line.rect;
                const cui_rect at{rect.x+x*scale,rect.y+y*scale,w*scale,h*scale};
                if (line.texture) cui_draw_image(ui,line.texture,at,{0,0,1,1},{},line.color);
                else cui_draw_rect_premultiplied(ui,at,line.color);
            }
        };
        captions(model.art.names);
        if (model.art.groundLabels) for (const auto& label:*model.art.groundLabels) {
            const auto at=[&](int dx,int dy) {
                return cui_rect{rect.x+(label.x+dx)*scale,rect.y+(label.y+dy)*scale,label.width*scale,15*scale};
            };
            cui_draw_rect_premultiplied(ui,at(0,0),{80,60,40,150});
            cui_draw_image(ui,label.texture,at(-1,2),{0,0,1,1},{},{0,0,0,255});
            cui_draw_image(ui,label.texture,at(0,1),{0,0,1,1},{},{255,255,125,255});
        }
        captions(model.art.speech);
        for (std::size_t i=model.art.overlayStart;i<sprites.size();++i) drawSprite(sprites[i]);
        cui_pop_clip(ui);
        return;
    }
    const auto corners=model.legacyHud ? cui_corners{} : cui_corners{10,10,10,10};
    cui_draw_rect(ui, rect, corners, {22, 33, 47, 255});
    cui_push_clip(ui, rect);
    const float cx = rect.x + rect.w / 2, cy = rect.y + rect.h / 2;
    const float pitch = SceneCellPitch*(model.legacyHud ? rect.w/800 : 1);
    const float padding = 6;
    const float markerY = cy + 10;
    if (model.terrainReady) {
        constexpr int columns = mir3_cui::MapTerrain::ViewColumns;
        constexpr int rows = mir3_cui::MapTerrain::ViewRows;
        static const cui_color cellColors[] = {{22, 33, 47, 255}, {40, 68, 66, 255}, {69, 63, 72, 255},
            {45, 104, 92, 255}, {134, 88, 50, 255}};
        for (int row = 0; row < rows; ++row) {
            const float y = markerY + pitch * (row - rows / 2) - pitch / 2;
            if (y + pitch < rect.y || y >= rect.y + rect.h) continue;
            for (int column = 0; column < columns; ++column) {
                const float x = cx + pitch * (column - columns / 2) - pitch / 2;
                if (x + pitch < rect.x || x >= rect.x + rect.w) continue;
                const auto cell = static_cast<unsigned>(model.ground[row * columns + column]);
                if (cell) cui_draw_rect(ui, {x + 1, y + 1, pitch - 2, pitch - 2}, {}, cellColors[cell]);
            }
        }
    } else {
        const cui_color grid{38, 53, 70, 255};
        for (float x = cx - pitch * static_cast<int>((rect.w / 2 - padding) / pitch); x < rect.x + rect.w - padding; x += pitch)
            cui_draw_line(ui, {x, rect.y + padding}, {x, rect.y + rect.h - padding}, 1, grid);
        for (float y = cy - pitch * static_cast<int>((rect.h / 2 - padding) / pitch); y < rect.y + rect.h - padding; y += pitch)
            cui_draw_line(ui, {rect.x + padding, y}, {rect.x + rect.w - padding, y}, 1, grid);
    }

    static const cui_vec2 directions[] = {{0,-1},{0.7f,-0.7f},{1,0},{0.7f,0.7f},
        {0,1},{-0.7f,0.7f},{-1,0},{-0.7f,-0.7f}};
    const auto direction = directions[model.facing];
    for (const auto& entry : *model.players) {
        const auto& player = entry.second;
        const float px = cx + pitch * (static_cast<int>(player.x) - static_cast<int>(model.x));
        const float py = markerY + pitch * (static_cast<int>(player.y) - static_cast<int>(model.y));
        if (px < rect.x - pitch || px > rect.x + rect.w + pitch || py < rect.y - pitch || py > rect.y + rect.h + pitch)
            continue;
        if (player.dead) {
            cui_draw_line(ui, {px - 8, py}, {px + 8, py}, 3, {187, 147, 149, 255});
        } else if (player.npcType) {
            cui_draw_circle(ui, {px, py}, 11, {115, 83, 57, 255});
            cui_draw_icon(ui, CUI_ICON_MESSAGE_SQUARE, {px - 10, py - 11, 20, 20}, 15, {250, 218, 165, 255});
        } else {
            constexpr float scale = 0.7f;
            const auto facing = directions[player.facing];
            cui_draw_circle(ui, {px, py}, 9, {67, 66, 106, 255});
            cui_draw_circle(ui, {px + facing.x * 13, py + facing.y * 12}, 2, {195, 182, 238, 255});
            PaintCharacterFigure(ui, {px - 24 * scale, py - 44 * scale, 56 * scale, 44 * scale},
                {player.appearance, false}, scale);
        }
        if (!player.name.empty())
            cui_draw_text(ui, CUI_FONT_BODY, 11, {px + 13, py - 7}, {224, 215, 247, 255}, player.name.c_str(), -1);
    }
    if (model.pending.deadline) {
        const float targetX = cx + (static_cast<int>(model.pending.x) - static_cast<int>(model.x)) * pitch;
        const float targetY = markerY + (static_cast<int>(model.pending.y) - static_cast<int>(model.y)) * pitch;
        cui_draw_circle(ui, {targetX, targetY}, 7, {229, 177, 82, 255});
    }
    cui_draw_circle(ui, {cx, markerY}, 18, {28, 72, 83, 255});
    cui_draw_circle(ui, {cx + direction.x * 25, markerY + direction.y * 21}, 3, {110, 237, 201, 255});
    PaintCharacterFigure(ui, {cx - 24, markerY - 43, 56, 44}, model.portrait);
    if (!model.legacyHud)
        cui_draw_text(ui, CUI_FONT_MEDIUM, 13, {rect.x + 10, rect.y + 8}, {233, 241, 249, 255}, model.location, -1);
    cui_draw_text(ui, CUI_FONT_BODY, 12, {cx + 32, markerY - 6}, {187, 223, 220, 255}, model.name, -1);
    cui_pop_clip(ui);
}

void DrawLocalScene(cui_ctx* ui, LoginClient& client, bool interactive, bool nativeInventory, bool legacyHud,
                   SceneMapArt art)
{
    auto& scene = client.scene;
    LocalSceneDrawing view{};
    view.portrait = {client.characterAppearance, !client.equipment.slots[4].key.name.empty()};
    view.x = scene.x;
    view.y = scene.y;
    view.facing = scene.facing;
    view.pending = scene.pending;
    view.terrainReady = scene.terrain.Ready();
    view.legacyHud = legacyHud;
    view.art = art;
    view.ground = scene.ground;
    view.players = &scene.players;
    const auto& map = scene.mapDescription.empty() ? scene.mapId : scene.mapDescription;
    std::snprintf(view.location, sizeof view.location, "%s  (%u, %u)", map.empty() ? "Scene" : map.c_str(), view.x, view.y);
    std::snprintf(view.name, sizeof view.name, "%s", client.gameCharacter.c_str());
    const auto now=GetTickCount64();
    const bool delayed=scene.InputDelayed(now);
    cui_push_disabled(ui,delayed);
    const auto canvas = cui_canvas(ui, "local-scene", CUI_FILL,
        legacyHud ? CUI_FILL : CUI_PX(client.inventory.shortcutsVisible ? 70 : 96),
        PaintLocalScene, &view, sizeof view);
    cui_pop_disabled(ui);
    const bool cutRelease=scene.crouchPointerCaptured && cui_mouse_released(ui,CUI_MOUSE_LEFT);
    const bool swingRelease=scene.meleePointerCaptured && cui_mouse_released(ui,CUI_MOUSE_LEFT);
    const bool walkRelease=scene.walkPointerAt && cui_mouse_released(ui,CUI_MOUSE_LEFT);
    if (!cui_mouse_down(ui,CUI_MOUSE_LEFT)) {
        scene.crouchHoldAt=0;
        scene.crouchPointerCaptured=false;
        scene.meleePointerCaptured=false;
        scene.walkPointerAt=0;
    }
    if (!cui_mouse_down(ui,CUI_MOUSE_RIGHT)) scene.runPointerCaptured=false;
    if (!interactive || delayed || !view.terrainReady || !canvas.hovered || cutRelease || swingRelease || walkRelease) return;
    if (nativeInventory && (client.inventory.heldGold || client.HeldItem())) {
        if (cui_mouse_clicked(ui,CUI_MOUSE_LEFT)) client.BeginHeldDrop();
        return;
    }
    if (client.trade.returnSlot>=0) return;
    if (!legacyHud) cui_tooltip(ui, "Left-click to walk. Right-click to run or turn.");
    const bool rightPress=cui_mouse_clicked(ui,CUI_MOUSE_RIGHT);
    if (rightPress && art.sprites) scene.runPointerCaptured=true;
    const bool run=rightPress || scene.runPointerCaptured;
    const auto mods=cui_key_mods(ui);
    const bool altCut=art.sprites && (mods&(CUI_MOD_ALT|CUI_MOD_SHIFT))==CUI_MOD_ALT;
    const bool cutPress=!run && altCut && cui_mouse_clicked(ui,CUI_MOUSE_LEFT);
    const bool cutRepeat=altCut && scene.crouchHoldAt && now-scene.crouchHoldAt>=300;
    const bool swingHeld=art.sprites && !run && !altCut && canvas.pressed &&
        (scene.meleePointerCaptured || cui_mouse_clicked(ui,CUI_MOUSE_LEFT));
    const bool walkHeld=canvas.pressed && scene.walkPointerAt && now-scene.walkPointerAt>=300;
    if (cutPress) scene.crouchPointerCaptured=true;
    if (!canvas.clicked && !run && !cutPress && !cutRepeat && !swingHeld && !walkHeld) return;
    if (art.sprites) {
        const float scale=canvas.rect.w/800;
        const float viewX=canvas.mouse.x/scale, viewY=canvas.mouse.y/scale;
        if (!run && (canvas.clicked || cutPress || swingHeld)) {
            const auto& npcs=*art.npcs;
            const auto hit=std::find_if(npcs.rbegin(),npcs.rend(),[&](const SceneActorHit& npc) {
                const auto& r=npc.bounds;
                return viewX>=r.x && viewX<r.x+r.w && viewY>=r.y && viewY<r.y+r.h;
            });
            if (hit!=npcs.rend() && ((canvas.clicked || cutPress) ? client.ClickNpcActor(hit->actor) : swingHeld)) return;
        }
        int x=static_cast<int>(viewX+art.cameraX)/48;
        int y=static_cast<int>(viewY+art.cameraY)/32;
        if (!run && (canvas.clicked || cutPress || swingHeld) && client.ClickNpc(x,y)) return;
        if (!run && (cutPress || cutRepeat)) {
            if (cutPress) scene.crouchHoldAt=now;
            const auto hit=[&](const SceneActorHit& actor) {
                const auto& r=actor.bounds;
                return viewX>=r.x && viewX<r.x+r.w && viewY>=r.y && viewY<r.y+r.h;
            };
            std::uint32_t monsterCorpse=0, humanCorpse=0;
            if (art.corpses && art.targets && std::none_of(art.targets->begin(),art.targets->end(),hit)) {
                for (const auto& candidate:*art.corpses) {
                    const auto& actor=scene.players.at(candidate.actor);
                    if (!hit(candidate) || !scene.CanButcher(actor)) continue;
                    auto& selected=candidate.monster ? monsterCorpse : humanCorpse;
                    selected=(std::max)(selected,candidate.actor);
                }
            }
            client.CrouchInScene(x,y,humanCorpse ? humanCorpse : monsterCorpse);
            return;
        }
        bool approachActor=false;
        if (!run && art.targets) {
            const auto& targets=*art.targets;
            const auto hit=std::find_if(targets.rbegin(),targets.rend(),[&](const SceneActorHit& target) {
                const auto& r=target.bounds;
                return viewX>=r.x && viewX<r.x+r.w && viewY>=r.y && viewY<r.y+r.h;
            });
            if (client.MeleeInScene(x,y,hit==targets.rend() ? 0 : hit->actor,(mods&CUI_MOD_SHIFT)!=0)) {
                if (swingHeld) {
                    scene.meleePointerCaptured=true;
                    cui_request_wakeup(ui,0.016f);
                }
                return;
            }
            const auto weapon=client.characterAppearance.weapon;
            if (hit!=targets.rend() && (weapon<34 || weapon>37)) {
                const auto& target=scene.players.at(hit->actor);
                x=int(target.x);
                y=int(target.y);
                approachActor=true;
            }
        }
        if (swingHeld) scene.walkPointerAt=now;
        if (scene.walkPointerAt || scene.runPointerCaptured) cui_request_wakeup(ui,0.05f);
        client.MoveInScene(x,y,run,approachActor,rightPress);
        return;
    }
    const float pitch=SceneCellPitch*(legacyHud ? canvas.rect.w/800 : 1);
    const int x = static_cast<int>(view.x) +
        static_cast<int>(std::floor((canvas.mouse.x - canvas.rect.w / 2) / pitch + 0.5f));
    const int y = static_cast<int>(view.y) +
        static_cast<int>(std::floor((canvas.mouse.y - canvas.rect.h / 2 - 10) / pitch + 0.5f));
    if (!run && client.ClickNpc(x, y)) return;
    client.MoveInScene(x,y,run,false,rightPress);
}

void DrawNpcDialogue(cui_ctx* ui, LoginClient& client)
{
    auto& npc = client.npcDialogue;
    const bool prompting = !npc.prompt.command.empty();
    bool open = true;
    cui_dialog_opts options = {};
    options.width = 440;
    options.persistent = true;
    options.no_close_button = true;
    if (!cui_dialog_begin(ui, "NPC dialogue", &open, &options)) return;
    const auto actor = client.scene.players.find(npc.actor);
    if (actor != client.scene.players.end() && !actor->second.name.empty()) cui_label(ui, actor->second.name.c_str());
    if (prompting) {
        cui_text_ex(ui, npc.prompt.label.c_str(), nullptr);
        cui_push_id_int(ui, npc.editor);
        cui_input_opts editor = {};
        editor.label = "Answer";
        editor.password = npc.prompt.command.starts_with("@@*");
        editor.password_mask = '*';
        editor.autofocus = true;
        editor.width = CUI_FILL;
        editor.byte_limit = client.NpcAnswerLimit();
        editor.measure_bytes = AnsiBytes;
        const int input = cui_input_ex(ui, "Answer", npc.answer, sizeof npc.answer, &editor);
        cui_pop_id(ui);
        if (npc.answerTooLong) cui_secondary(ui, "The answer is too long or contains unsupported characters.");
        cui_dialog_footer_begin(ui);
        const bool cancel = Button(ui, "Cancel") || cui_key_pressed(ui, CUI_KEY_ESCAPE);
        const bool send = Button(ui, "Confirm", true) || (input & CUI_SUBMITTED);
        cui_dialog_footer_end(ui);
        if (cancel || send) client.EndNpcPrompt(!cancel);
    } else {
        static const cui_color colors[] = {{100, 102, 111, 255},{77, 116, 232, 255},{52, 139, 79, 255},
            {47, 130, 139, 255},{118, 120, 130, 255},{66, 94, 190, 255},{150, 125, 55, 255},
            {168, 71, 83, 255},{160, 165, 179, 255},{143, 91, 160, 255},{58, 148, 72, 255},
            {191, 75, 71, 255},{},{179, 83, 155, 255},{148, 112, 46, 255},{56, 137, 152, 255},
            {103, 105, 114, 255}};
        cui_scroll_opts list = {};
        list.id = "npc-text";
        list.height = CUI_PX(244);
        list.width = CUI_FILL;
        list.gap = 5;
        if (npc.resetScroll) {
            cui_scroll_to(ui, list.id, 0);
            npc.resetScroll = false;
        }
        cui_scroll_begin(ui, &list);
        for (std::size_t index = 0; index < npc.rows.size(); ++index) {
            const auto& row = npc.rows[index];
            cui_color color{};
            if (row.color < 17) color = colors[row.color];
            if (row.command.empty()) {
                cui_text_opts text = {};
                text.color = color;
                cui_text_ex(ui, row.label.c_str(), &text);
            } else {
                cui_button_opts button = {};
                button.width = CUI_FILL;
                button.color = color;
                cui_push_id_int(ui, static_cast<int>(index));
                if (cui_button_ex(ui, row.label.c_str(), &button)) client.ChooseNpcOption(row);
                cui_pop_id(ui);
            }
        }
        cui_scroll_end(ui);
        if (npc.canClose) {
            cui_dialog_footer_begin(ui);
            const bool close = Button(ui, "Close", true) || cui_key_pressed(ui, CUI_KEY_ESCAPE);
            cui_dialog_footer_end(ui);
            if (close) client.CloseNpcDialogue();
        }
    }
    cui_dialog_end(ui);
}

void DrawStorageList(cui_ctx* ui, LoginClient& client)
{
    auto& shop = client.shop;
    bool open = true;
    cui_dialog_opts modal{};
    modal.persistent = true;
    if (!cui_dialog_begin(ui,shop.withdrawalPrompt ? "Withdraw quantity" : "Warehouse",&open,&modal)) return;
    if (shop.withdrawalPrompt) {
        const int input = Input(ui,"Quantity",shop.quantity,sizeof shop.quantity,false,true);
        cui_dialog_footer_begin(ui);
        const bool cancel = Button(ui,"Cancel") || cui_key_pressed(ui,CUI_KEY_ESCAPE);
        const bool confirm = Button(ui,"Withdraw",true) || (input & CUI_SUBMITTED);
        cui_dialog_footer_end(ui);
        cui_dialog_end(ui);
        if (cancel || !open) shop.withdrawalPrompt = false;
        else if (confirm) client.SubmitStorageQuantity();
        return;
    }
    const auto pages = (std::max)(std::size_t{1},(shop.offers.size()+11)/12);
    cui_textf(ui,"Page %u / %u",shop.page+1,static_cast<unsigned>(pages));
    cui_scroll_opts list{};
    list.id = "warehouse-items";
    list.width = CUI_FILL;
    list.height = CUI_PX(190);
    cui_scroll_begin(ui,&list);
    for (std::size_t i = shop.page*12, end = (std::min)(shop.offers.size(),i+12); i < end; ++i) {
        const auto& item = shop.offers[i];
        auto label = item.label;
        if (item.stackMode >= 2) label += " x"+std::to_string(item.count);
        cui_push_id_int(ui,static_cast<int>(i));
        if (Button(ui,label.c_str(),static_cast<int>(i) == shop.offer,true)) shop.offer = static_cast<int>(i);
        cui_pop_id(ui);
    }
    cui_scroll_end(ui);
    cui_dialog_footer_begin(ui);
    cui_push_disabled(ui,shop.withdrawing != 0);
    if (Button(ui,"Previous")) client.PageStorage(-1);
    if (Button(ui,"Next")) client.PageStorage(1);
    if (Button(ui,"Withdraw",true)) client.WithdrawStorage();
    cui_pop_disabled(ui);
    const bool close = Button(ui,"Close") || cui_key_pressed(ui,CUI_KEY_ESCAPE);
    cui_dialog_footer_end(ui);
    cui_dialog_end(ui);
    if (close || !open) shop = {};
}

void DrawNpcShop(cui_ctx* ui, LoginClient& client)
{
    if (client.shop.storage) {
        DrawStorageList(ui,client);
        return;
    }
    auto& shop = client.shop;
    bool open = true;
    cui_dialog_opts dialog = {};
    dialog.width = 600;
    dialog.persistent = true;
    dialog.no_close_button = true;
    if (!cui_dialog_begin(ui, shop.crafting ? "Crafting" : "Shop", &open, &dialog)) return;
    if (shop.crafting && shop.notice >= 1440 && shop.notice <= 1444) {
        if (shop.notice == 1440) cui_label(ui, "Crafting complete.");
        else cui_textf(ui, "Crafting failed (server code %d).", shop.notice - 1440);
        cui_dialog_footer_begin(ui);
        if (Button(ui, "OK", true) || cui_key_pressed(ui, CUI_KEY_ENTER) || cui_key_pressed(ui, CUI_KEY_SPACE))
            shop.notice = 0;
        cui_dialog_footer_end(ui);
        cui_dialog_end(ui);
        return;
    }
    cui_textf(ui, "Gold %u", client.inventory.gold);
    cui_box columns = {};
    columns.width = CUI_FILL;
    columns.gap = 10;
    cui_row_begin(ui, &columns);
    cui_scroll_opts list = {};
    list.id = "shop-categories";
    list.width = CUI_FILL;
    list.height = CUI_PX(204);
    list.gap = 4;
    std::pair<int, bool> clicked{-1, false};
    const auto selectRow = [&](int row, bool offer) {
        const auto now = GetTickCount64();
        const int key = offer ? row + 256 : row;
        const bool twice = shop.lastRow == key && now - shop.lastClick < 300;
        shop.lastRow = twice ? -1 : key;
        shop.lastClick = now;
        if (offer) shop.offer = row;
        else shop.selected = row;
        if (twice) clicked = {row, offer};
    };
    const bool busy = shop.detailDeadline || client.purchase.deadline;
    cui_scroll_begin(ui, &list);
    if (shop.goods.empty()) cui_muted(ui, "No goods available.");
    for (std::size_t row = 0; row < shop.goods.size(); ++row) {
        const auto label = shop.goods[row].label + "###good";
        cui_push_id_int(ui, static_cast<int>(row));
        cui_push_disabled(ui, busy);
        if (Button(ui, label.c_str(), shop.selected == static_cast<int>(row), true))
            selectRow(static_cast<int>(row), false);
        cui_pop_disabled(ui);
        cui_pop_id(ui);
    }
    cui_scroll_end(ui);

    cui_box detail = columns;
    detail.gap = 6;
    cui_column_begin(ui, &detail);
    const int category = shop.detailed || shop.bulk ? shop.category : shop.selected;
    if (category >= 0) {
        const auto& good = shop.goods[category];
        cui_label(ui, good.label.c_str());
        cui_textf(ui, "Price %u", good.price);
        if (good.requirement) cui_textf(ui, "Requirement %u (%u)", good.requirement, good.requirementType);
    }
    if (shop.bulk && !shop.crafting) {
        Input(ui, "Quantity", shop.quantity, sizeof shop.quantity);
        const auto count = shop.Quantity();
        if (count) cui_textf(ui, "Total %llu", std::uint64_t(shop.goods[shop.category].price) * count);
    } else if (shop.detailed) {
        list.id = "shop-offers";
        list.height = CUI_PX(134);
        cui_scroll_begin(ui, &list);
        if (shop.offers.empty()) cui_muted(ui, shop.detailDeadline ? "Loading..." : "No items on this page.");
        for (std::size_t row = 0; row < shop.offers.size(); ++row) {
            const auto& offer = shop.offers[row];
            const auto label = offer.label + "  " + std::to_string(offer.price) + "###offer";
            cui_push_id_int(ui, static_cast<int>(row));
            cui_push_disabled(ui, busy);
            if (Button(ui, label.c_str(), shop.offer == static_cast<int>(row), true))
                selectRow(static_cast<int>(row), true);
            cui_pop_disabled(ui);
            cui_pop_id(ui);
        }
        cui_scroll_end(ui);
        cui_textf(ui, "Page %u", shop.page + 1);
        cui_box paging = columns;
        paging.gap = 4;
        paging.wrap = true;
        cui_row_begin(ui, &paging);
        cui_push_disabled(ui, busy || !shop.page);
        if (Button(ui, "Previous")) client.RequestShopOffers(shop.category, shop.page - 1);
        cui_pop_disabled(ui);
        cui_push_disabled(ui, busy || shop.page >= 65535 / 10);
        if (Button(ui, "Next")) client.RequestShopOffers(shop.category, shop.page + 1);
        cui_pop_disabled(ui);
        cui_box_end(ui);
    }
    cui_box_end(ui);
    cui_box_end(ui);

    if (shop.detailDeadline || client.purchase.deadline) cui_muted(ui, "Waiting for the server...");
    else if (!shop.status.empty()) cui_secondary(ui, shop.status.c_str());
    cui_dialog_footer_begin(ui);
    const bool close = Button(ui, "Close") || cui_key_pressed(ui, CUI_KEY_ESCAPE);
    cui_push_disabled(ui, busy || shop.selected < 0);
    const bool details = Button(ui, "Select");
    cui_pop_disabled(ui);
    cui_push_disabled(ui, busy || (!shop.bulk && shop.offer < 0));
    const bool buy = Button(ui, shop.crafting && shop.bulk ? "Craft" : "Buy", true);
    cui_pop_disabled(ui);
    cui_dialog_footer_end(ui);
    cui_dialog_end(ui);
    if (close) shop = {};
    else if (buy || (clicked.first >= 0 && clicked.second)) client.BuyShopItem();
    else if (details || clicked.first >= 0) client.RequestShopOffers(shop.selected, 0);
}

class IndependentDialogTheme {
    cui_theme& theme_;
    const cui_theme previous_;

public:
    explicit IndependentDialogTheme(cui_ctx* ui) : theme_(*cui_theme_get(ui)), previous_(theme_)
    {
        auto& c = theme_.colors;
        c[CUI_TOKEN_SURFACE] = CUI_RGB(0x252316);
        c[CUI_TOKEN_SURFACE_RAISED] = CUI_RGB(0x26241b);
        c[CUI_TOKEN_SURFACE_MUTED] = CUI_RGB(0x353124);
        c[CUI_TOKEN_FOREGROUND] = CUI_RGB(0xf3e6c7);
        c[CUI_TOKEN_TEXT_SECONDARY] = CUI_RGB(0xd2c4a6);
        c[CUI_TOKEN_TEXT_MUTED] = CUI_RGB(0xa7997c);
        c[CUI_TOKEN_BORDER] = CUI_RGB(0x786544);
        c[CUI_TOKEN_BORDER_SUBTLE] = CUI_RGB(0x5d4e36);
        c[CUI_TOKEN_BORDER_STRONG] = CUI_RGB(0xb19a68);
        c[CUI_TOKEN_ACCENT] = CUI_RGB(0xcab17b);
        c[CUI_TOKEN_ACCENT_STRONG] = CUI_RGB(0xe0c899);
        c[CUI_TOKEN_ACCENT_SUBTLE] = CUI_RGB(0x514533);
        c[CUI_TOKEN_ACCENT_FOREGROUND] = CUI_RGB(0x201b11);
        c[CUI_TOKEN_HOVER] = CUI_RGBA(0xefd5a222);
        c[CUI_TOKEN_ACTIVE] = CUI_RGBA(0xefd5a244);
        c[CUI_TOKEN_SCRIM] = CUI_RGBA(0x00000025);
        theme_.radius_surface = theme_.radius_panel = theme_.radius_control = 2;
        theme_.control_sm = 21;
        theme_.control_md = 30;
        theme_.heading[1] = 17;
    }

    ~IndependentDialogTheme() { theme_ = previous_; }
};

// GameInter 1910 has 286 visible rows inside its 310-pixel logical frame.
constexpr std::array<cui_vec2,5> IndependentSizes{{{284,348},{194,285},{720,440},{408,286},{440,310}}};

bool IndependentOpen(const IndependentWindows& windows, int panel)
{
    switch (panel) {
    case 0: return windows.profile.open;
    case 1: return windows.orders.open;
    case 2: return windows.market.open && !windows.market.registration;
    case 3: return windows.market.registration;
    case 4: return windows.mail.open;
    }
    return false;
}

void CloseIndependentWindow(IndependentWindows& windows, int panel)
{
    switch (panel) {
    case 0: windows.profile.open = false; break;
    case 1:
        windows.orders.open = false;
        windows.orders.targetX = windows.orders.targetY = 0;
        break;
    case 2:
        if (windows.market.confirmation && !windows.market.registration) windows.market.confirmation = 0;
        else windows.market = {};
        break;
    case 3:
        if (windows.market.confirmation) windows.market.confirmation = 0;
        else {
            windows.market.registration = false;
            windows.market.item = {};
        }
        break;
    case 4:
        if (windows.mail.reading || windows.mail.composing) {
            windows.mail.reading = windows.mail.composing = false;
            windows.mail.readingId = 0;
        } else windows.mail.open = false;
        break;
    }
}

struct IndependentViewport {
    float scale, x, y;
    explicit IndependentViewport(cui_ctx* ui)
    {
        const auto view = cui_viewport_size(ui);
        scale = std::max(0.01f, std::min(view.x / 800, view.y / 600));
        x = (view.x - 800 * scale) / 2;
        y = (view.y - 600 * scale) / 2;
    }
    cui_rect Bounds(const IndependentWindows& windows, int panel) const
    {
        const auto p = windows.positions[panel], size = IndependentSizes[panel];
        return {x + p.x*scale, y + p.y*scale, size.x*scale, size.y*scale};
    }
    bool Hit(const IndependentWindows& windows, int panel, cui_vec2 mouse) const
    {
        const auto r = Bounds(windows,panel);
        return mouse.x >= r.x && mouse.x < r.x+r.w && mouse.y >= r.y && mouse.y < r.y+r.h;
    }
};

bool PrepareIndependentWindows(cui_ctx* ui, IndependentWindows& windows, bool blocked, bool& escape)
{
    const IndependentViewport view(ui);
    if (blocked || cui_mouse_clicked(ui,CUI_MOUSE_LEFT)) windows.pressedButtons.fill(0);
    const auto raise = [&](int panel) {
        const auto found = std::ranges::find(windows.stacking, panel);
        std::rotate(found, found+1, windows.stacking.end());
    };
    for (int i = 0; i < static_cast<int>(windows.stacking.size()); ++i) {
        const bool open = IndependentOpen(windows, i);
        if (open && !windows.wasOpen[i]) raise(i);
        windows.wasOpen[i] = open;
    }
    if (windows.dragging >= 0 && (!IndependentOpen(windows, windows.dragging) || blocked))
        windows.dragging = -1;
    const auto mouse = cui_mouse_pos(ui);
    if (windows.dragging >= 0) {
        const int panel = windows.dragging;
        if (cui_mouse_down(ui, CUI_MOUSE_LEFT)) {
            const auto size = IndependentSizes[panel];
            windows.positions[panel] = {
                std::clamp((mouse.x-view.x)/view.scale-windows.dragOffset.x, 0.0f, 800-size.x),
                std::clamp((mouse.y-view.y)/view.scale-windows.dragOffset.y, 0.0f, 600-size.y)};
        } else windows.dragging = -1;
        return true;
    }
    int top = -1, hit = -1;
    for (auto it = windows.stacking.rbegin(); it != windows.stacking.rend(); ++it) {
        if (!IndependentOpen(windows, *it)) continue;
        if (top < 0) top = *it;
        if (hit < 0 && view.Hit(windows,*it,mouse)) hit = *it;
    }
    if (!blocked && top >= 0 && !cui_wants_keyboard(ui) && cui_key_pressed(ui, CUI_KEY_ESCAPE)) {
        CloseIndependentWindow(windows, top);
        escape = true;
    }
    if (!blocked && hit >= 0 && cui_mouse_clicked(ui, CUI_MOUSE_LEFT)) {
        raise(hit);
        const auto r = view.Bounds(windows, hit);
        const float x = (mouse.x-r.x)/view.scale, y = (mouse.y-r.y)/view.scale;
        if (y < 22 && (hit != 4 || x < 175)) {
            windows.dragging = hit;
            windows.dragOffset = {x,y};
        }
    }
    return hit >= 0;
}

template<typename Assets>
struct IndependentPanel {
    cui_ctx* ui;
    cui_rect bounds;
    float scale;
    Assets* assets;
    cui_id& pressedButton;

    template<typename Draw>
    void At(float x, float y, float width, float height, Draw draw) const
    {
        // The skin is painted as a flow child. Controls must paint above it.
        cui_overlay_begin(ui, {x*scale,y*scale,width*scale,height*scale});
        draw();
        cui_box_end(ui);
    }

    void SpriteAt(float x, float y, int frame) const
    {
        if (!assets) return;
        const auto size=assets->IndependentButtonSize(frame);
        if (size.x>0) At(x,y,size.x,size.y,[&] {
            assets->DrawIndependentButton(ui,frame,size.y*scale,0);
        });
    }

    bool ButtonAt(float x, float y, int frame, const char* name, bool active = false, bool enabled = true) const
    {
        const auto size = assets ? assets->IndependentButtonSize(frame) : cui_vec2{};
        if (size.x <= 0) {
            bool clicked = false;
            At(x,y,48,26,[&] {
                cui_push_disabled(ui,!enabled);
                clicked=Button(ui,name,active,true);
                cui_pop_disabled(ui);
            });
            return clicked;
        }
        const auto id=cui_make_id(ui,name);
        const bool inside=ContainsPoint(x,y,size.x,size.y);
        const bool hover=inside && !cui_is_disabled(ui) && enabled;
        if (hover && cui_mouse_clicked(ui,CUI_MOUSE_LEFT)) {
            pressedButton=id;
        }
        const bool held=hover && pressedButton==id && cui_mouse_down(ui,CUI_MOUSE_LEFT);
        At(x,y,size.x,size.y,[&] {
            assets->DrawIndependentButton(ui,frame==1221 && held ? 1222 : frame,size.y*scale,
                !enabled ? -1 : held && frame!=1221 ? -2 : hover || active ? 1 : 0);
            if (hover) cui_tooltip(ui,name);
        });
        return hover && pressedButton==id && cui_mouse_released(ui,CUI_MOUSE_LEFT);
    }

    void TextAt(float x, float y, float width, float height, const std::string& text, bool centered = false) const
    {
        At(x,y,width,height,[&] {
            cui_box field{};
            field.width=field.height=CUI_FILL;
            field.clip=true;
            field.justify=CUI_JUSTIFY_CENTER;
            cui_box_begin(ui,&field);
            cui_text_opts options{};
            options.font=assets ? assets->IndependentEditor(0,scale).font : CUI_FONT_BODY;
            options.size=12*scale;
            options.color=CUI_RGB(0xfafafa);
            options.nowrap=true;
            options.align=centered ? CUI_TEXT_CENTER : CUI_TEXT_LEFT;
            options.line_height=1;
            options.width=CUI_FILL;
            cui_text_ex(ui,text.c_str(),&options);
            cui_box_end(ui);
        });
    }

    void HighlightAt(float x, float y, float width, float height) const
    {
        At(x,y,width,height,[&] {
            cui_canvas_paint(ui,CUI_FILL,CUI_PX(height*scale),[](cui_ctx* ui,cui_rect rect,void*) {
                cui_draw_rect(ui,rect,{},CUI_RGBA(0x8e6a383d));
            },nullptr,0);
        });
    }

    void EditAt(float x, float y, float width, float height, const char* id,
                char* value, std::size_t capacity, int limit, bool multiline = false) const
    {
        At(x,y,width,height,[&] {
            auto options = assets ? assets->IndependentEditor(height*scale,scale) : cui_input_opts{};
            options.width = CUI_FILL;
            options.bare = true;
            options.byte_limit = limit;
            options.measure_bytes = AnsiBytes;
            options.height = height*scale;
            cui_push_id(ui,id);
            if (multiline) cui_textarea_ex(ui,id,value,capacity,&options);
            else cui_input_ex(ui,id,value,capacity,&options);
            cui_pop_id(ui);
        });
    }

    bool ContainsPoint(float x, float y, float width, float height) const
    {
        const auto p = cui_mouse_pos(ui);
        return p.x >= bounds.x+x*scale && p.x < bounds.x+(x+width)*scale &&
            p.y >= bounds.y+y*scale && p.y < bounds.y+(y+height)*scale;
    }

    bool ReleasedIn(float x, float y, float width, float height) const
    {
        return !cui_is_disabled(ui) && cui_mouse_released(ui,CUI_MOUSE_LEFT) && ContainsPoint(x,y,width,height);
    }
};

template<typename DrawItem>
void DrawMarketRegistration(const auto& panel, LoginClient& client, DrawItem drawItem)
{
    auto& market = client.windows.market;
    panel.TextAt(46,18,80,15,"[ ç‰©å“ç™»è®° ]",true);
    if (panel.ReleasedIn(240,51,151,129)) {
        if (const auto* item = client.HeldItem(); item && client.equipment.heldSlot < 0 && !item->IsGold()) {
            market.item = item->key;
            client.equipment.ReleaseHeld();
        }
    }
    const auto* item = client.inventory.Find(market.item);
    panel.TextAt(247,58,148,15,item ? item->label : "æ”¾å…¥èƒŒåŒ…ç‰©å“",true);
    if (item) panel.At(245,88,146,93,[&] { drawItem(item->icon,93*panel.scale); });
    panel.EditAt(12,71,218,112,"registration-description",market.description,sizeof market.description,120,true);
    panel.EditAt(42,233,116,24,"registration-price",market.price,sizeof market.price,10);
    if (!market.status.empty()) panel.TextAt(16,208,224,17,market.status);
    else if (market.confirmation == 1130) panel.TextAt(16,208,224,17,"ç¡®è®¤ç™»è®°è¯¥ç‰©å“ï¼Ÿ");
    if (panel.ButtonAt(249,230,1904,market.confirmation ? "ç¡®è®¤ç™»è®°" : "ç™»è®°",market.confirmation!=0,item != nullptr)) {
        if (market.confirmation) {
            client.RegisterMarketItem();
            market.confirmation = 0;
        } else market.confirmation = 1130;
    }
    if (panel.ButtonAt(319,230,1908,"å–æ¶ˆ") || panel.ButtonAt(369,242,1221,"å…³é—­"))
        CloseIndependentWindow(client.windows,3);
}

template<typename DrawItem>
void DrawMarket(const auto& panel, LoginClient& client, DrawItem drawItem)
{
    auto& market = client.windows.market;
    panel.TextAt(10,17,105,14,"[ è´­ä¹°ç‰©å“ ]",true);
    panel.TextAt(65,18,94,15,std::to_string(market.page)+" / "+std::to_string(market.pages),true);
    panel.TextAt(16,56,102,15,"æœåŠ¡å™¨",true);
    panel.TextAt(121,56,136,15,"ç‰©å“åç§°",true);
    panel.TextAt(261,56,67,15,"ä»·æ ¼",true);
    panel.TextAt(331,56,137,15,"å‡ºå”®äºº",true);
    for (std::size_t i=0; i<std::min<std::size_t>(13,market.listings.size()); ++i) {
        const auto& listing = market.listings[i];
        const float y = 84+19*static_cast<float>(i);
        if (panel.ReleasedIn(14,y,454,18) && market.selected != listing.id) {
            market.selected = listing.id;
            market.confirmation = 0;
        }
        if (market.selected == listing.id) panel.HighlightAt(14,y,454,18);
        panel.TextAt(16,y+2,102,15,listing.server,true);
        panel.TextAt(121,y+2,136,15,listing.item.label,true);
        panel.TextAt(261,y+2,67,15,std::to_string(listing.price),true);
        panel.TextAt(331,y+2,137,15,listing.seller,true);
    }
    const auto selected = std::ranges::find(market.listings,market.selected,&IndependentWindows::Listing::id);
    if (selected != market.listings.end()) {
        panel.TextAt(485,25,213,14,selected->item.label);
        panel.At(482,55,213,128,[&] { drawItem(selected->item.icon,128*panel.scale); });
        panel.At(482,199,213,123,[&] {
            cui_text_opts options{};
            options.size=12*panel.scale;
            options.font=panel.assets ? panel.assets->IndependentEditor(0,panel.scale).font : CUI_FONT_BODY;
            cui_text_ex(panel.ui,selected->description.c_str(),&options);
        });
    }
    if (!market.status.empty()) panel.TextAt(482,319,213,17,market.status);
    panel.EditAt(268,363,137,17,"market-filter",market.filter,sizeof market.filter,30);
    panel.EditAt(268,393,137,17,"market-seller",market.sellerFilter,sizeof market.sellerFilter,30);
    if (panel.ButtonAt(413,361,1900,"æŸ¥ç‰©å“")) { market.byName = true; client.MarketRequest(1136); }
    if (panel.ButtonAt(413,391,1900,"æŸ¥å–å®¶")) { market.byName = false; client.MarketRequest(1136); }
    if (panel.ButtonAt(23,361,1894,"ä¸Šé¡µ")) client.MarketRequest(1132);
    if (panel.ButtonAt(73,361,1896,"ä¸‹é¡µ")) client.MarketRequest(1131);
    if (panel.ButtonAt(123,362,1898,"åˆ·æ–°")) client.MarketRequest(1134);
    if (panel.ButtonAt(514,371,1904,"ç™»è®°")) client.BeginMarketRegistration();
    const auto action = [&](unsigned opcode) {
        if (market.confirmation == opcode) { client.MarketRequest(opcode); market.confirmation = 0; }
        else market.confirmation = opcode;
    };
    const bool haveSelection = selected != market.listings.end();
    if (panel.ButtonAt(564,371,1906,market.confirmation == 1133 ? "ç¡®è®¤è´­ä¹°" : "è´­ä¹°",market.confirmation==1133,haveSelection)) action(1133);
    if (panel.ButtonAt(644,371,1908,market.confirmation == 1135 ? "ç¡®è®¤æ’¤é”€" : "æ’¤é”€",market.confirmation==1135,haveSelection)) action(1135);
    if (panel.ButtonAt(680,396,1221,"å…³é—­")) CloseIndependentWindow(client.windows,2);
}

template<typename DrawItem>
void DrawMailbox(const auto& panel, LoginClient& client, DrawItem drawItem)
{
    auto& mail = client.windows.mail;
    panel.TextAt(34,15,142,20,"ä¿¡ä»¶",true);
    if (panel.ButtonAt(183,14,1988,"æ”¶ä»¶",!mail.folder && !mail.composing && !mail.reading)) client.MailRequest(1137,0);
    if (panel.ButtonAt(363,14,1980,"å‘ä»¶",mail.folder && !mail.composing && !mail.reading)) client.MailRequest(1137,1);
    if (panel.ButtonAt(273,14,1984,"å†™ä¿¡",mail.composing)) {
        mail.composing = true;
        mail.recipient[0] = mail.draft[0] = '\0';
        mail.status.clear();
    }
    if (panel.assets) panel.ButtonAt(318,14,1982,"é€ç¤¼",false,false);
    if (panel.ButtonAt(228,14,1986,"è¯»å–",mail.reading,!mail.reading && !mail.composing && mail.selected))
        client.MailRequest(1138,mail.folder,mail.selected);
    const auto& letters = mail.folders[mail.folder];
    if (mail.composing) {
        panel.TextAt(21,61,47,16,"æ”¶ä¿¡äºº");
        panel.EditAt(68,62,148,15,"mail-recipient",mail.recipient,sizeof mail.recipient,24);
        panel.EditAt(21,84,373,141,"mail-draft",mail.draft,sizeof mail.draft,500,true);
        if (panel.ButtonAt(286,256,2034,"å‘é€")) client.SendLetter();
        if (panel.ButtonAt(326,256,2032,"å–æ¶ˆæ’°å†™")) mail.composing=false;
    } else if (mail.reading) {
        panel.TextAt(68,60,327,17,mail.sender+"   "+mail.subject);
        panel.At(25,81,379,135,[&] {
            cui_scroll_opts body = {};
            body.id = "mail-body";
            body.width = CUI_FILL;
            body.height = CUI_PX(135*panel.scale);
            cui_scroll_begin(panel.ui,&body);
            cui_text_ex(panel.ui,mail.text.c_str(),nullptr);
            cui_scroll_end(panel.ui);
        });
        if (!mail.gift.label.empty()) {
            panel.At(331,118,68,81,[&] { drawItem(mail.gift.icon,81*panel.scale); });
            panel.TextAt(25,225,364,15,"ç¤¼ç‰©  "+mail.gift.label);
        }
        else if (mail.gold) panel.TextAt(25,225,364,15,"é‡‘å¸  "+std::to_string(mail.gold));
        const auto current = std::ranges::find(letters, mail.selected, &IndependentWindows::Letter::id);
        const auto adjacent = [&](int offset, float x, int frame, const char* title) {
            const auto index = current - letters.begin() + offset;
            const bool valid = current != letters.end() && index >= 0 && index < std::ssize(letters);
            if (panel.ButtonAt(x,257,frame,title,false,valid)) {
                mail.selected = letters[index].id;
                client.MailRequest(1138,mail.folder,mail.selected);
            }
        };
        adjacent(-1,54,2014,"ä¸Šä¸€å°");
        adjacent(1,114,2012,"ä¸‹ä¸€å°");
        if (!mail.folder && panel.ButtonAt(286,256,2016,"ç­”å¤")) {
            strncpy_s(mail.recipient,mail.sender.c_str(),_TRUNCATE);
            mail.draft[0] = '\0';
            mail.composing = true;
        }
    } else {
        constexpr int visible=7;
        const int maxFirst=std::max(0,static_cast<int>(letters.size())-visible);
        auto& first=mail.firstVisible[mail.folder];
        const auto point=cui_mouse_pos(panel.ui);
        const float y=(point.y-panel.bounds.y)/panel.scale;
        if (!cui_is_disabled(panel.ui) && panel.ContainsPoint(21,51,411,179))
            first-=static_cast<int>(std::lround(cui_take_mouse_scroll(panel.ui).y));
        if (maxFirst && panel.ReleasedIn(405,51,28,177))
            first=static_cast<int>(std::lround((y-64)/149*maxFirst));
        first=std::clamp(first,0,maxFirst);
        const float knob=52+(maxFirst ? 149.0f*first/maxFirst : 0.0f);
        panel.SpriteAt(410,knob,1966);
        for (int i=first; i<std::min(first+visible,static_cast<int>(letters.size())); ++i) {
            const auto& letter=letters[i];
            const float row=59+24.0f*(i-first);
            if (panel.ReleasedIn(21,row,382,21) && mail.selected!=letter.id) {
                mail.selected=letter.id;
                mail.readingId=0;
            }
            if (letter.id==mail.selected) {
                if (panel.assets) panel.SpriteAt(22,row+6,1974);
                else panel.HighlightAt(21,row,382,21);
            }
            panel.TextAt(43,row+6,94,15,letter.name);
            panel.TextAt(141,row+6,221,15,letter.date.empty() ? letter.subject : letter.date);
            panel.SpriteAt(374,row+3,letter.read ? 1971 : 1970);
        }
        if (panel.ButtonAt(mail.folder ? 246 : 286,256,1964,"åˆ·æ–°")) client.MailRequest(1137,mail.folder);
        if (mail.folder && panel.ButtonAt(286,256,2032,"æ’¤å›ž",false,mail.selected != 0))
            client.MailRequest(1142,mail.folder,mail.selected);
    }
    if (!mail.status.empty()) panel.TextAt(22,232,388,16,mail.status);
    if (!mail.composing && panel.ButtonAt(326,256,1962,"åˆ é™¤",false,mail.selected != 0))
        client.MailRequest(1139,mail.folder,mail.selected);
    if (panel.ButtonAt(400,267,1221,"å…³é—­")) CloseIndependentWindow(client.windows,4);
}

template<typename DrawMap>
void DrawObjectOrders(const auto& panel, LoginClient& client, DrawMap drawMap)
{
    auto& orders = client.windows.orders;
    panel.TextAt(14,22,76,15,std::to_string(orders.x),true);
    panel.TextAt(100,22,80,15,std::to_string(orders.y),true);
    panel.At(6,65,180,165,drawMap);
    const auto ammo = std::ranges::find_if(client.inventory.items,
        [](const auto& item) { return item.icon == 310 || item.icon == 311; });
    if (ammo != client.inventory.items.end()) {
        if (panel.ButtonAt(12,40,1592,"è£…å¡«",orders.ammunition == ammo->key)) {
            if (orders.ammunition == ammo->key) client.LoadObjectAmmunition();
            else orders.ammunition = ammo->key;
        }
        if (orders.ammunition == ammo->key)
            panel.EditAt(69,40,97,25,"ammo-quantity",orders.quantity,sizeof orders.quantity,5);
    }
    if (panel.ButtonAt(17,231,1592,"æ”»å‡»",false,orders.targetX && orders.targetY)) client.SendObjectOrder(1120);
    if (panel.ButtonAt(67,231,1594,"ç­‰å¾…")) client.SendObjectOrder(1122);
    if (panel.ButtonAt(117,231,1596,"ç§»åŠ¨")) client.SendObjectOrder(1121);
    if (panel.ButtonAt(153,241,1221,"å…³é—­")) CloseIndependentWindow(client.windows,1);
}
void DrawEquipment(cui_ctx* ui, LoginClient& client, cui_rect bounds)
{
    auto& equipment = client.equipment;
    const auto& stats = client.characterStatus;
    if (!cui_panel_begin(ui, "Equipment", &equipment.open, bounds)) return;

    const EquipmentPortrait portrait{client.characterAppearance, !equipment.slots[4].key.name.empty()};
    cui_box identity = {};
    identity.gap = 10;
    identity.align = CUI_ALIGN_CENTER;
    cui_row_begin(ui, &identity);
    cui_canvas(ui, "character-portrait", CUI_PX(56), CUI_PX(44), PaintEquipmentPortrait, &portrait, sizeof portrait);
    cui_label(ui, client.gameCharacter.c_str());
    cui_box_end(ui);
    static const char* const names[] = {"Armor", "Weapon", "Off hand", "Necklace", "Helmet",
        "L. bracer", "R. bracer", "L. ring", "R. ring", "Charm", "Slot 10", "Slot 11"};
    cui_scroll_opts list = {};
    list.id = "character-status";
    list.width = CUI_FILL;
    list.height = CUI_PX((std::max)(112.0f, (std::min)(330.0f, bounds.h - 270.0f)));
    list.gap = 8;
    list.fade_edges = true;
    cui_scroll_begin(ui, &list);

    cui_box row = {};
    row.width = CUI_FILL;
    row.gap = 8;
    cui_box column = row;
    column.gap = 3;
    const auto drawSlot = [&](int slot) {
        const auto& item = equipment.slots[slot];
        const bool held = equipment.heldSlot == slot;
        const auto label = std::string(names[slot]) + "###equipment-slot";
        const auto tip = item.label.empty() ? std::string("Empty") :
            item.label + "  " + std::to_string(item.durability) + " / " + std::to_string(item.maxDurability);
        cui_button_opts button = {};
        button.width = CUI_FILL;
        button.size = CUI_SIZE_SM;
        button.variant = equipment.selected == slot ? CUI_BUTTON_PRIMARY : CUI_BUTTON_SECONDARY;
        button.selected = !held && !item.label.empty();
        button.tooltip = tip.c_str();
        cui_push_id_int(ui, slot);
        if (cui_button_ex(ui, label.c_str(), &button)) client.ReleaseEquipmentSlot(slot);
        cui_pop_id(ui);
    };
    cui_row_begin(ui, &row);
    cui_column_begin(ui, &column);
    constexpr int leftSlots[] = {1, 4, 0, 5, 7, 10};
    constexpr int rightSlots[] = {3, 6, 8, 2, 9, 11};
    cui_row_begin(ui, &row);
    for (const auto& slots : {leftSlots, rightSlots}) {
        cui_column_begin(ui, &column);
        for (int index = 0; index < 6; ++index) drawSlot(slots[index]);
        cui_box_end(ui);
    }
    cui_box_end(ui);
    cui_box_end(ui);

    column.width = CUI_PX(130);
    cui_column_begin(ui, &column);
    if (!stats.received) cui_muted(ui, "Waiting for character stats...");
    cui_textf(ui, "Level %u", stats.level);
    cui_textf(ui, "Experience %.2f%%", stats.ExperiencePercent());
    cui_textf(ui, "HP %u / %u", stats.hp[0], stats.hp[1]);
    cui_textf(ui, "MP %u / %u", stats.mp[0], stats.mp[1]);
    cui_textf(ui, "Bag %u / %u", stats.weight[0], stats.weight[1]);
    cui_textf(ui, "Wear %u / %u", stats.wear[0], stats.wear[1]);
    cui_textf(ui, "Hand %u / %u", stats.hand[0], stats.hand[1]);
    cui_textf(ui, "Accuracy +%u%%", stats.accuracy);
    cui_textf(ui, "Agility +%u%%", stats.agility);
    cui_box_end(ui);
    cui_box_end(ui);

    static const char* const combatNames[] = {"DC", "MC", "SC", "AC", "MAC"};
    row.wrap = true;
    cui_row_begin(ui, &row);
    for (std::size_t i = 0; i < stats.combat.size(); ++i)
        cui_textf(ui, "%s %u-%u", combatNames[i], stats.combat[i][0], stats.combat[i][1]);
    cui_box_end(ui);

    static const char* const elementNames[] = {"Fire", "Ice", "Lightning", "Wind", "Holy", "Dark", "Phantom"};
    static const char* const elementRows[] = {"Element attack", "Strong against", "Weak against"};
    for (int type = 0; type < 3; ++type) {
        cui_row_begin(ui, &row);
        cui_label(ui, elementRows[type]);
        bool any = false;
        for (int element = 0; element < 7; ++element) {
            const auto value = stats.DisplayedElement(type, element);
            if (!value) continue;
            any = true;
            cui_textf(ui, "%s %c%u", elementNames[element], type == 0 ? '+' : 'x', value);
        }
        if (!any) cui_muted(ui, "-");
        cui_box_end(ui);
    }
    cui_scroll_end(ui);

    const auto& worn = equipment.slots[equipment.selected];
    const auto* held = client.HeldItem();
    const auto* bagItem = equipment.heldSlot >= 0 ? nullptr :
        (held ? held : client.inventory.Find(client.inventory.selection));
    if (held) cui_textf(ui, "Holding: %s", held->label.c_str());
    else if (!worn.label.empty()) cui_textf(ui, "%s  |  Durability %u / %u", worn.label.c_str(), worn.durability, worn.maxDurability);
    else if (bagItem) cui_textf(ui, "Bag selection: %s", bagItem->label.c_str());
    const bool pending = equipment.request != mir3_cui::EquipmentRequest::None ||
        client.inventory.request != mir3_cui::ItemRequest::None;
    if (!equipment.status.empty()) cui_secondary(ui, equipment.status.c_str());
    else if (pending) cui_muted(ui, "Waiting for the server...");

    cui_box actions = {};
    actions.width = CUI_FILL;
    actions.gap = 8;
    actions.wrap = true;
    cui_row_begin(ui, &actions);
    cui_push_disabled(ui, pending || !bagItem || !mir3_cui::EquipmentFits(bagItem->mode, equipment.selected));
    if (Button(ui, "Wear", true)) client.EquipItem();
    cui_pop_disabled(ui);
    cui_push_disabled(ui, pending || worn.label.empty());
    if (Button(ui, "Remove")) client.UnequipItem();
    cui_pop_disabled(ui);
    cui_push_disabled(ui, pending || (held && equipment.heldSlot != equipment.selected) ||
        !mir3_cui::EquipmentActivates(worn, equipment.selected));
    if (Button(ui, "Activate")) client.ActivateEquipment();
    cui_pop_disabled(ui);
    cui_box_end(ui);
    cui_dialog_footer_begin(ui);
    if (!client.inventory.open && Button(ui, "Open bag")) client.OpenInventory();
    if (Button(ui, "Done", true)) equipment.open = false;
    cui_dialog_footer_end(ui);
    cui_panel_end(ui);
}

void DrawItemPanels(cui_ctx* ui, LoginClient& client, bool inventoryWasOpen, bool equipmentWasOpen,
                    bool nativeInventory, bool nativeStatus)
{
    auto& bag = client.inventory;
    auto& equipment = client.equipment;
    const bool hadPanels = inventoryWasOpen || (!nativeStatus && equipmentWasOpen);
    const bool bagDialog = bag.notice || bag.bindingShortcut || bag.drop != mir3_cui::DropDialog::None ||
        client.merchant.notice || client.merchant.quantityOpen;
    if (hadPanels && !bagDialog && !client.showSignOut) {
        if (!nativeInventory && cui_shortcut(ui, 0, CUI_KEY_W)) {
            if (bag.open) client.CloseInventory();
            else client.OpenInventory();
        } else if (!nativeStatus && cui_shortcut(ui, 0, CUI_KEY_Q)) {
            if (equipment.open) equipment.open = false;
            else client.OpenEquipment();
        } else if (cui_key_pressed(ui, CUI_KEY_ESCAPE)) {
            if (equipment.open && !nativeStatus) equipment.open = false;
            else if (!nativeInventory) client.CloseInventory();
        }
    }

    const auto viewport = cui_viewport_size(ui);
    const bool both = bag.open && equipment.open && !nativeStatus;
    cui_rect bagBounds{}, equipmentBounds{};
    const float height = viewport.y - 16;
    if (both && viewport.x >= 640) {
        const float width = (std::min)(460.0f, (viewport.x - 24.0f) / 2);
        const float x = (viewport.x - 2 * width - 8) / 2;
        equipmentBounds = {x, 8, width, height};
        bagBounds = {x + width + 8, 8, width, height};
    } else if (both) {
        const float half = (height - 8) / 2;
        equipmentBounds = {8, 8, viewport.x - 16, half};
        bagBounds = {8, 16 + half, viewport.x - 16, half};
    } else {
        const float width = (std::min)(equipment.open && !nativeStatus ? 610.0f : 500.0f, viewport.x - 32);
        equipmentBounds = bagBounds = {(viewport.x - width) / 2, 8, width, height};
    }

    if (equipment.open && !nativeStatus) DrawEquipment(ui, client, equipmentBounds);
    if (!nativeInventory ? bag.open || bagDialog : bag.bindingShortcut)
        DrawInventory(ui, client, bagBounds);
    if (!nativeInventory && hadPanels && !bag.open && !equipment.open &&
        equipment.request == mir3_cui::EquipmentRequest::None)
        equipment.ReleaseHeld();
}

void DrawServerOptions(cui_ctx* ui, LoginClient& client)
{
    auto& options = client.serverOptions;
    cui_dialog_opts dialog = {};
    dialog.width = 510;
    if (!cui_dialog_begin(ui, "Server options", &options.open, &dialog)) return;

    const auto drawOption = [&](const mir3_cui::ServerOption& option) {
        bool checked = (options.displayed & option.mask) != 0;
        if (cui_checkbox_ex(ui, option.label, &checked, nullptr))
            client.ToggleServerOption(option);
        if ((options.displayed ^ options.confirmed) & option.mask)
            cui_tooltip(ui, "Waiting for server confirmation");
    };
    cui_box row = {};
    row.width = CUI_FILL;
    row.gap = 14;
    cui_box column = {};
    column.width = CUI_FILL;
    column.gap = 10;
    cui_row_begin(ui, &row);
    cui_column_begin(ui, &column);
    for (std::size_t i = 0; i < 4; ++i) drawOption(mir3_cui::ServerOptionList[i]);
    cui_box_end(ui);
    cui_column_begin(ui, &column);
    for (std::size_t i = 4; i < mir3_cui::ServerOptionCount; ++i) drawOption(mir3_cui::ServerOptionList[i]);
    cui_box_end(ui);
    cui_box_end(ui);

    static const char* const attackModes[] = {"All targets", "Peaceful", "Party", "Guild", "Good and evil"};
    cui_separator(ui);
    if (options.attackMode < sizeof attackModes / sizeof *attackModes)
        cui_textf(ui, "Attack mode: %s", attackModes[options.attackMode]);
    else
        cui_textf(ui, "Attack mode: %u", static_cast<unsigned>(options.attackMode));
    if (Button(ui, "Change attack mode")) client.RequestAttackMode();
    if (options.attackModePending || options.displayed != options.confirmed)
        cui_muted(ui, "Waiting for server confirmation");
    if (!options.error.empty()) cui_secondary(ui, options.error.c_str());
    cui_dialog_footer_begin(ui);
    if (Button(ui,"Audio")) { options.open=false; client.audioOpen=true; }
    if (Button(ui, "Done", true)) options.open = false;
    cui_dialog_footer_end(ui);
    cui_dialog_end(ui);
}

void DrawAudioSettings(cui_ctx* ui, LoginClient& client)
{
    cui_dialog_opts dialog = {};
    dialog.persistent = true;
    dialog.no_close_button = true;
    if (!cui_dialog_begin(ui, "Audio settings", &client.audioOpen, &dialog)) return;

    auto& settings = client.audioOptions;
    bool changed = cui_checkbox_ex(ui, "Background music", &settings.music, nullptr);
    cui_slider_opts slider = {};
    slider.step = 1;
    slider.show_value = true;
    slider.format = "%.0f";
    slider.width = CUI_FILL;
    const auto drawLevel = [&](const char* label, int& level) {
        float value = static_cast<float>(level);
        if (cui_slider_ex(ui, label, &value, -100, 0, &slider)) {
            level = static_cast<int>(value);
            changed = true;
        }
    };
    drawLevel("Music volume", settings.musicLevel);
    if (cui_checkbox_ex(ui, "Sound effects", &settings.effects, nullptr)) changed = true;
    drawLevel("Effects volume", settings.effectsLevel);
    if (cui_checkbox_ex(ui, "Reverse stereo effects", &settings.reversePan, nullptr)) changed = true;
    if (changed) {
        client.audio.Apply(settings);
        client.audioDirty = true;
    }
    cui_push_disabled(ui, !settings.effects);
    if (Button(ui, "Play test sound")) client.audio.PlayEffect(ClickSound, settings);
    cui_pop_disabled(ui);
    if (!client.audio.Error().empty()) cui_secondary(ui, client.audio.Error().c_str());
    if (!client.audioSaveError.empty()) cui_secondary(ui, client.audioSaveError.c_str());
    cui_dialog_footer_begin(ui);
    const bool done = Button(ui, "Done", true) || cui_key_pressed(ui, CUI_KEY_ESCAPE);
    cui_dialog_footer_end(ui);
    cui_dialog_end(ui);
    if (done && client.SaveAudioSettings()) client.audioOpen = false;
}

} // namespace

const Dialogs::Assets::WorldFrame& Dialogs::Assets::GetWorldFrame(cui_ctx* ui, const std::string& library, int frame,
    std::uint16_t status,bool highlighted,std::uint16_t dye)
{
    const bool tinted=status!=65535 && status!=1;
    const bool brighten=status==65535 && highlighted;
    const auto cache=tinted ? library+(highlighted ? ":highlight-status:" : ":status:")+std::to_string(status) :
        (dye!=65535 || brighten) ? library+(brighten ? ":highlight-dye:" : ":dye:")+std::to_string(dye) : library;
    auto [entry,inserted]=worldFrames_.try_emplace({cache,frame});
    auto& result=entry->second;
    result.used=worldFrame_;
    if (!inserted) return result;
    auto [archive,open]=worldArchives_.try_emplace(library);
    if (open) archive->second.open(mapData_/library);
    if (auto art=archive->second.frame(frame)) {
        if (tinted) mir3::tint_status_pixels(art->pixels.rgba,art->pixels.regions,status,highlighted);
        else {
            if (dye!=65535) mir3::tint_wil_pixels(art->pixels.rgba,art->pixels.regions,dye,65535);
            if (brighten) mir3::highlight_actor_pixels(art->pixels.rgba);
        }
        result.image=Upload(ui,art->width,art->height,art->pixels.rgba);
        result.x=art->offset_x;
        result.y=art->offset_y;
        if (result.image.texture) worldTextureBytes_+=std::uint64_t(art->width)*art->height*4;
    }
    return result;
}

const Dialogs::Assets::WorldFrame& Dialogs::Assets::GroundFrame(cui_ctx* ui, int looks, bool highlighted)
{
    const auto& normal=GetWorldFrame(ui,"Ground",looks);
    if (!highlighted || !normal.image.texture) return normal;
    auto [entry,added]=worldFrames_.try_emplace({"Ground:highlight",looks});
    auto& result=entry->second;
    result.used=worldFrame_;
    if (added) {
        if (auto art=worldArchives_.at("Ground").frame(looks)) {
            auto& pixels=art->pixels.rgba;
            constexpr std::array steps{8,4,8};
            for (std::size_t i=0;i<pixels.size();i+=4) {
                if (!pixels[i+3]) continue;
                for (int c=0;c<3;++c) {
                    const int step=steps[c], highest=255/step, source=pixels[i+c]/step;
                    pixels[i+c]=static_cast<std::uint8_t>(step*(std::min)(highest,source+3*source*source/(2*highest)));
                }
                pixels[i+3]=255;
            }
            result.image=Upload(ui,art->width,art->height,pixels,true);
            if (result.image.texture) worldTextureBytes_+=std::uint64_t(art->width)*art->height*4;
        }
    }
    return result.image.texture ? result : normal;
}

void Dialogs::Assets::PrepareGroundItems(cui_ctx* ui, const LocalScene& scene, const SceneMapArt& camera,
                                        int startX, int startY, cui_vec2 pointer, bool interactive,
                                        std::vector<SceneSprite>& overlays)
{
    using View=mir3_cui::MapTerrain::GraphicsView;
    constexpr int columns=View::LastColumn-View::FirstColumn, rows=View::LastRow-View::FirstRow;
    std::array<unsigned,columns*rows> counts{};
    const int cellX=int(pointer.x+camera.cameraX)/48, cellY=int(pointer.y+camera.cameraY)/32;
    const auto coin=messages_.get(1409);
    bool showLabels=interactive;
    for (const auto& item:scene.groundItems.items) {
        const int x=item.x*48-camera.cameraX, y=item.y*32-camera.cameraY;
        const bool hover=interactive && item.x==cellX && item.y==cellY;
        if (hover && showLabels && worldGroundLabels_.size()<groundNames_.size()) {
            const bool coinVariant=!coin.empty() && item.name.size()==coin.size()+1 && item.name.starts_with(coin) &&
                item.name.back()>='0' && item.name.back()<='9';
            auto& cached=groundNames_[worldGroundLabels_.size()];
            if (cached.name!=item.name) {
                cached.name=item.name;
                Destroy(ui,cached.image);
                const auto full=ConvertText(item.name,936,CP_UTF8);
                const int width=mir3::raster_text(full,800,15,false,mir3::TextFont::normal).text_width;
                if (width>0) cached.image=Raster(ui,coinVariant ? ConvertText(coin,936,CP_UTF8) : full,
                    width,15,false,mir3::TextFont::normal);
            }
            const int line=static_cast<int>(worldGroundLabels_.size());
            worldGroundLabels_.push_back({cached.image.texture,x+24-cached.image.width/2,y-4-16*line,cached.image.width});
            showLabels=!coinVariant;
        }
        if (item.sparkle_frame>=0) {
            const auto found=gamePortraits_.find(40+item.sparkle_frame);
            if (found!=gamePortraits_.end() && found->second.texture) {
                const auto& art=found->second;
                overlays.push_back({art.texture,x+8,y,art.width,art.height,0});
            }
        }
        const int column=item.x-startX-View::FirstColumn, row=item.y-startY-View::FirstRow;
        if (column<0 || column>=columns || row<0 || row>=rows ||
            counts[row*columns+column]++>=mir3::GroundItems::cell_capacity) continue;
        const auto& art=GroundFrame(ui,item.looks,hover).image;
        if (art.texture) {
            const auto offset=mir3::GroundItems::image_offset(art.width,art.height);
            worldSprites_.push_back({art.texture,x+offset[0],y+offset[1],art.width,art.height,item.y});
        }
    }
}

const Dialogs::Assets::WorldFrame& Dialogs::Assets::GetProjectedWorldShadow(cui_ctx* ui, const std::string& library,
                                                                           int frame, bool flat, bool weapon)
{
    auto [entry,inserted]=worldFrames_.try_emplace({library+(weapon ? ":weapon-shadow" : flat ? ":flat-shadow" : ":shadow"),frame});
    auto& result=entry->second;
    result.used=worldFrame_;
    if (!inserted) return result;
    const auto body=worldArchives_.at(library).frame(frame);
    if (!body || (!weapon && !flat && body->shadow_mode!=49 && body->shadow_mode!=50)) return result;
    const bool shear=weapon || (!flat && body->shadow_mode==49);
    const int width=body->width+(shear ? body->height/2 : 0);
    const int height=weapon ? body->height/2+1 : shear ? (body->height+1)/2 : body->height;
    std::vector<std::uint8_t> rgba(std::size_t(width)*height*4);
    for (int row=weapon ? 1 : 0;row<body->height;row+=shear ? 2 : 1) {
        const int offset=weapon ? body->height/2-row/2 : shear ? (body->height-row)/2 : 0;
        const int y=shear ? (row+int(weapon))/2 : row;
        for (int x=0;x<body->width;++x)
            if (body->pixels.regions[std::size_t(row)*body->width+x])
                rgba[(std::size_t(y)*width+x+offset)*4+3]=255;
    }
    result.image=Upload(ui,width,height,rgba);
    result.x=flat ? body->offset_x+3 : body->shadow_x;
    result.y=flat ? body->offset_y+2 : body->shadow_y;
    if (weapon) {
        const int half=(body->offset_y-16)/2;
        result.x=body->offset_x-half-body->height/2;
        result.y=body->offset_y-half;
    }
    if (result.image.texture) worldTextureBytes_+=std::uint64_t(width)*height*4;
    return result;
}

void Dialogs::Assets::TrimWorldFrames(cui_ctx* ui)
{
    constexpr std::uint64_t textureBudget=128ull*1024*1024;
    if (worldTextureBytes_<=textureBudget) return;
    std::vector<decltype(worldFrames_)::iterator> stale;
    for (auto it=worldFrames_.begin();it!=worldFrames_.end();++it)
        if (it->second.used!=worldFrame_ && it->second.image.texture) stale.push_back(it);
    std::sort(stale.begin(),stale.end(),[](const auto& a,const auto& b) { return a->second.used<b->second.used; });
    for (auto it:stale) {
        auto& image=it->second.image;
        worldTextureBytes_-=std::uint64_t(image.width)*image.height*4;
        Destroy(ui,image);
        worldFrames_.erase(it);
        if (worldTextureBytes_<=textureBudget) break;
    }
}

cui_rect Dialogs::Assets::AddWorldActor(cui_ctx* ui, const mir3_cui::CharacterAppearance& actor, int facing,
    int x, int y, const SceneMapArt& camera, int pose, bool shadowBlend, int mount,
    int appearanceWeight, bool highlighted, std::uint16_t status)
{
    const std::string prefix=actor.body ? "WM-" : "M-";
    int motion=facing*10+pose;
    const auto library=prefix+"Hum";
    const WorldFrame* horse=nullptr;
    if (mount>=1 && mount<=3 && (pose<4 || (pose>=2560 && pose<2563) ||
        (pose>=2400 && pose<2406) || (pose>=2480 && pose<2486))) {
        const int mountedMotion=pose<4 ? motion+2320 : motion;
        const auto& frame=GetWorldFrame(ui,"Horse",400*(mount-1)+mountedMotion-2320);
        if (frame.image.texture && GetWorldFrame(ui,library,int(actor.clothing)*3000+mountedMotion).image.texture) {
            horse=&frame;
            motion=mountedMotion;
        }
    }
    const int frame=static_cast<int>(actor.clothing)*3000+motion;
    const auto& body=GetWorldFrame(ui,library,frame);
    if (!body.image.texture) return {};
    const cui_rect bounds{float(x*48-camera.cameraX+body.x),float(y*32-camera.cameraY+body.y),
        float(body.image.width),float(body.image.height)};
    if (appearanceWeight>=0 && highlighted) {
        appearanceWeight+=5;
        if (appearanceWeight>31) appearanceWeight=0;
    }
    const bool shadeHighlight=highlighted && (status!=65535 || appearanceWeight<0);
    const auto add=[&](const WorldFrame& layer,int weight,bool shadow=false) {
        const auto& art=layer.image;
        if (!art.texture) return;
        SceneSprite sprite{art.texture,x*48-camera.cameraX+layer.x,y*32-camera.cameraY+layer.y,art.width,art.height,y};
        ApplyActorBlend(sprite,status,appearanceWeight,weight,shadow,shadowBlend);
        worldSprites_.push_back(sprite);
    };
    const bool hasWeapon=actor.weapon && actor.weapon!=37 && motion<2000 && motion<static_cast<int>(mapWeaponOrder_.size());
    const WorldFrame* weapon=nullptr;
    if (hasWeapon) {
        const auto weaponLibrary=prefix+"Weapon"+std::to_string((actor.weapon-1)/10+1);
        const int weaponFrame=static_cast<int>((actor.weapon-1)%10)*3000+motion;
        weapon=&GetWorldFrame(ui,weaponLibrary,weaponFrame,status,shadeHighlight);
        if (weapon->image.texture && motion/80!=8 && pose!=HumanCorpsePose)
            add(GetProjectedWorldShadow(ui,weaponLibrary,weaponFrame,false,true),13,true);
    }
    if (horse) {
        const auto& horseShadow=GetWorldFrame(ui,"Horse",10000+motion-2320);
        add(horseShadow,13,true);
    }
    add(GetProjectedWorldShadow(ui,library,frame,pose==HumanCorpsePose),13,true);
    const bool behind=hasWeapon && mapWeaponOrder_[motion]!=0;
    if (horse) add(GetWorldFrame(ui,"Horse",400*(mount-1)+motion-2320,status,shadeHighlight),appearanceWeight);
    if (weapon && behind) add(*weapon,10);
    add(GetWorldFrame(ui,library,frame,status,shadeHighlight),appearanceWeight);
    if (actor.hair)
        add(GetWorldFrame(ui,prefix+(actor.hair<=10 ? "Hair" : actor.hair<=20 ? "Helmet1" : "Helmet2"),
            static_cast<int>((actor.hair-1)%10)*3000+motion,status,shadeHighlight),20);
    if (weapon && !behind) add(*weapon,22);
    return bounds;
}

SceneMapArt Dialogs::Assets::PrepareMapArt(cui_ctx* ui, LoginClient& client, cui_vec2 pointer, bool interactive)
{
    auto& scene=client.scene;
    if (!scene.terrain.Ready()) return {};
    const auto playerPixel=LocalActorPixel(scene);
    const auto mapCamera=LocalActorCamera(scene);
    SceneMapArt camera{&worldSprites_,&worldNpcs_,&worldTargets_,&worldCorpses_,&worldGroundLabels_,&worldNames_,&worldSpeech_,
        mapCamera[0],mapCamera[1]};
    const auto hoveredActor=interactive ? SceneTargetAt(camera,pointer) : 0;
    const int startX=camera.cameraX/96*2, startY=camera.cameraY/64*2;
    const int originX=startX*48-camera.cameraX, originY=startY*32-camera.cameraY;
    if (mapGraphicsName_!=scene.mapId || mapGraphicsRevision_!=scene.terrain.Revision() ||
        mapGraphicsX_!=startX || mapGraphicsY_!=startY) {
        mapGraphics_=scene.terrain.ReadGraphics(startX,startY);
        mapGraphicsName_=scene.mapId;
        mapGraphicsRevision_=scene.terrain.Revision();
        mapGraphicsX_=startX;
        mapGraphicsY_=startY;
    }
    ++worldFrame_;
    worldSprites_.clear();
    worldNpcs_.clear();
    worldTargets_.clear();
    worldCorpses_.clear();
    worldGroundLabels_.clear();
    worldAnchors_.clear();
    worldSpellLights_.clear();
    worldNames_.clear();
    worldSpeech_.clear();
    constexpr std::array<const char*,15> packages{"Tilesc","Tiles30c","Tiles5c","SmTilesc","Housesc","Cliffsc",
        "Dungeonsc","Innersc","Furnituresc","Wallsc","SmObjectsc","Animationsc","Object1c","Object2c","Custom"};
    constexpr std::array<const char*,5> regions{"","Wood/","Sand/","Snow/","Forest/"};
    const auto texture=[&](int package,int frame) -> const Image& {
        static const Image empty{};
        if (package>=75 || frame>=65535) return empty;
        return GetWorldFrame(ui,std::string(regions[package/15])+packages[package%15],frame).image;
    };
    constexpr int underActors=-1024;
    for (int i=0;i<static_cast<int>(mapGraphics_.tiles.size());++i) {
        const auto& tile=mapGraphics_.tiles[i];
        const auto& image=texture(tile.package,tile.frame);
        if (image.texture) worldSprites_.push_back({image.texture,originX+96*(i%10),originY+64*(i/10),
            image.width,image.height,underActors});
    }
    bool animated=false;
    const auto tick=static_cast<std::uint64_t>(cui_time(ui)*1000);
    const auto monsterNow=GetTickCount64();
    using View=mir3_cui::MapTerrain::GraphicsView;
    constexpr int columns=View::LastColumn-View::FirstColumn;
    constexpr std::array periods{150,200,250,300,350,400,420,450};
    for (int i=0;i<static_cast<int>(mapGraphics_.objects.size());++i) {
        const int column=View::FirstColumn+i%columns, row=View::FirstRow+i/columns;
        for (const auto& object:mapGraphics_.objects[i]) {
            if (object.package>=75 || object.package%15<3 || object.frame>=65535) continue;
            const int count=object.animation&15;
            const bool moving=object.package%15==11 && object.animation!=255 && count>1;
            const int frame=object.frame+(moving ? static_cast<int>(tick/periods[(object.animation>>4)&7]%count) : 0);
            const auto& image=texture(object.package,frame);
            if (!image.texture) continue;
            animated|=moving;
            const bool low=image.width<=48 && image.height<=32;
            worldSprites_.push_back({image.texture,originX+48*column,originY+32*(row+1)-image.height,
                image.width,image.height,low ? underActors : startY+row});
        }
    }
    if (worldSprites_.empty()) return {};
    if (scene.spellEffects) {
        const auto ground=[&](mir3::SpellCell cell,int index,std::uint8_t opacity) {
            const auto& frame=GetWorldFrame(ui,"ProgUse",index);
            if (frame.image.texture) worldSprites_.push_back({frame.image.texture,
                cell[0]*48-camera.cameraX+frame.x,cell[1]*32-camera.cameraY+frame.y,
                frame.image.width,frame.image.height,underActors,opacity});
        };
        const auto& spells=*scene.spellEffects;
        for (const auto& [id,tile]:spells.mapEvents.ground)
            if (tile.age>0) ground(tile.cell,tile.frame,tile.opacity);
        for (const auto& mark:spells.marks) ground(mark.cell,mark.frame,180);
        for (const auto& [cell,mark]:spells.mapEvents.iceMarks)
            if (mark.age) ground(cell,mark.frame,mark.opacity);
    }
    std::vector<SceneSprite> overlays;
    PrepareGroundItems(ui,scene,camera,startX,startY,pointer,interactive,overlays);
    const auto visible=[&](const LocalScene::Player& actor) {
        return static_cast<int>(actor.x)>=startX+View::FirstColumn && static_cast<int>(actor.x)<startX+View::LastColumn &&
            static_cast<int>(actor.y)>=startY+View::FirstRow && static_cast<int>(actor.y)<startY+View::LastRow;
    };
    for (const auto& [id,actor]:scene.players) {
        if (!actor.IsNpc() || actor.dead || !visible(actor)) continue;
        const auto& visual=actor.npcVisual;
        const auto motion=visual.standing.count ? visual.standing : LocalScene::NpcVisual::Idle(actor.npcType,actor.npcShape);
        if (!motion.count) continue;
        const int frame=visual.standing.count ? visual.frame : int(tick/motion.interval%motion.count);
        const auto age=visual.appearanceDuration ? monsterNow-visual.appearanceAt : 0;
        const int weight=age<visual.appearanceDuration ? int(31*age/visual.appearanceDuration) : -1;
        animated|=motion.count>1 || weight>=0;
        const auto& sprite=GetWorldFrame(ui,actor.npcType==4 ? "Flag" : "NPC",motion.first+frame);
        if (!sprite.image.texture) continue;
        const int x=static_cast<int>(actor.x)*48-camera.cameraX+sprite.x;
        const int y=static_cast<int>(actor.y)*32-camera.cameraY+sprite.y;
        SceneSprite body{sprite.image.texture,x,y,sprite.image.width,sprite.image.height,static_cast<int>(actor.y)};
        ApplyActorBlend(body,65535,weight,weight,false,false);
        worldSprites_.push_back(body);
        worldNpcs_.push_back({{float(x),float(y),float(sprite.image.width),float(sprite.image.height)},id,actor.y});
        worldAnchors_.push_back({id,worldNpcs_.back().bounds,x-sprite.x,y-sprite.y,0});
    }
    const int idle=static_cast<int>(tick/201%4);
    const auto humanDamage=[&](mir3::DamageNumbers& damage,int x,int y) {
        damage.enabled=client.damageNumbers;
        const auto glyphs=damage.draw(x,y,true);
        for (const auto& glyph:glyphs) {
            const auto& image=gameHudFrames_.at(glyph.frame);
            if (image.texture) overlays.push_back({image.texture,glyph.x,glyph.y,image.width,image.height,0});
        }
        animated|=!glyphs.empty();
    };
    const auto humanBar=[&](unsigned hp,unsigned maximum,int x,int y) {
        if (!maximum || !gamePortraits_.contains(2)) return;
        const auto& border=gamePortraits_.at(2), &fill=gamePortraits_.at(3);
        const auto percent=(std::min)(100u,hp*100/maximum);
        const int width=(std::min)(fill.width,int(double(percent)*double(0.01f)*36.0));
        if (border.texture) overlays.push_back({border.texture,x+7,y-56,border.width,border.height,0});
        if (fill.texture && width>0) overlays.push_back({fill.texture,x+7,y-56,width,fill.height,0,255,float(width)/fill.width});
    };
    for (auto& [id,actor]:scene.players) {
        if (actor.IsNpc() || !visible(actor)) continue;
        cui_rect bounds{};
        unsigned actorRow=actor.y;
        const bool targetable=!actor.dead && actor.monsterBurrowFrame<0;
        const bool highlighted=targetable && (hoveredActor==id || scene.lockedSpellActor==id);
        if (actor.IsMonster()) {
            const int shape=actor.npcShape;
            const int library=shape<1000 ? 88+shape/10 : shape>=2000 ? ((shape-2000)/100-121)&255 : -1;
            std::string archive;
            if (library>=88 && library<=107) archive="Mon-"+std::to_string(library-87);
            else if (library>=108 && library<=127) archive="MonS-"+std::to_string(library-107);
            else if (library==135) archive="DMon-1";
            else if (library==136) archive="DMonS-1";
            else if (library==141) archive="Mon-54";
            else continue;
            const bool walking=actor.monsterWalkFrame>=0;
            const bool combat=actor.monsterCombatFrame>=0;
            const bool corpse=actor.monsterDeathFrame>=0;
            const bool burrowing=actor.monsterBurrowFrame>=0;
            const bool dormant=actor.monsterDormant && !walking && !combat && !corpse && !burrowing;
            const bool single=shape<10 || shape==95 || mir3::TargetPortrait::special_shape(shape) || shape==538;
            int interval=300;
            if (shape>=145 && shape<=147) interval=150;
            else if (shape==158) interval=200;
            else if (shape==161 || shape==165 || shape==166) interval=250;
            const int group=shape<1000 ? shape%10 : (shape-2000)/10;
            const int walk=actor.monsterWalkFrame;
            const auto idleElapsed=actor.monsterIdleAt ? monsterNow-actor.monsterIdleAt : tick;
            const auto burrow=mir3::monster_burrow_motion(shape);
            const int action=burrowing || dormant ? burrow.base : corpse ? (actor.monsterSkeleton ? 880 : 320) :
                walking ? 80 : combat ? actor.monsterCombatMotion.base : 0;
            const int facing=mir3::monster_resource_direction(shape,action,dormant ? 0 : actor.facing);
            int pose=single ? 0 : int(idleElapsed/interval%4);
            if (burrowing)
                pose=actor.monsterBurrowReverse ? burrow.frames-1-actor.monsterBurrowFrame : actor.monsterBurrowFrame;
            else if (dormant) pose=0;
            else if (corpse) pose=actor.monsterDeathFrame;
            else if (walking) pose=actor.monsterBackstep ? 5-walk : walk;
            else if (combat) pose=actor.monsterCombatFrame;
            if (walking && actor.monsterNextStep>monsterNow)
                cui_request_wakeup(ui,float(actor.monsterNextStep-monsterNow)*0.001f);
            const int rowBase=action==320 ? mir3::monster_death_motion(shape).base : action;
            const int directionStride=action==320 && shape==141 ? 20 : 10;
            const int frame=group*1000+rowBase+facing*directionStride+pose;
            const auto age=actor.monsterAppearanceDuration ? monsterNow-actor.monsterAppearanceAt : 0;
            const int rawWeight=age<actor.monsterAppearanceDuration ?
                int(31*age/actor.monsterAppearanceDuration)+(highlighted ? 5 : 0) : -1;
            const int appearanceWeight=rawWeight>31 ? 0 : rawWeight;
            const auto status=mir3::actor_status_color(actor.monsterStatus);
            const bool shadeHighlight=highlighted && (status!=65535 || appearanceWeight<0);
            const auto& sprite=GetWorldFrame(ui,archive,frame,status,shadeHighlight,mir3::monster_body_dye(shape));
            animated|=actor.MonsterAnimating() || (!corpse && !single && !dormant) || appearanceWeight>=0;
            if (!sprite.image.texture) continue;
            const auto pixel=ActorWorldPixel(actor);
            const int x=int(pixel[0])-camera.cameraX+sprite.x, y=int(pixel[1])-camera.cameraY+sprite.y;
            if (!mir3::TargetPortrait::special_shape(shape)) {
                const WorldFrame* shadow=nullptr;
                if (library>=88 && library<=107) shadow=&GetWorldFrame(ui,"MonS-"+std::to_string(library-87),frame);
                else if (shape>=2000 && library==135) shadow=&GetWorldFrame(ui,"DMonS-1",frame);
                if (!shadow || !shadow->image.texture) {
                    const bool flat=corpse && shape!=127 && shape!=153 && !actor.MonsterDying();
                    shadow=&GetProjectedWorldShadow(ui,archive,frame,flat);
                }
                if (shadow->image.texture) {
                    SceneSprite shade{shadow->image.texture,x-sprite.x+shadow->x,y-sprite.y+shadow->y,
                        shadow->image.width,shadow->image.height,int(actor.y)};
                    ApplyActorBlend(shade,status,appearanceWeight,13,true,client.shadowBlend);
                    worldSprites_.push_back(shade);
                }
            }
            SceneSprite body{sprite.image.texture,x,y,sprite.image.width,sprite.image.height,int(actor.y)};
            ApplyActorBlend(body,status,appearanceWeight,appearanceWeight,false,client.shadowBlend);
            worldSprites_.push_back(body);
            worldAnchors_.push_back({id,{float(x),float(y),float(sprite.image.width),float(sprite.image.height)},
                x-sprite.x,y-sprite.y,1});
            const int width=(std::min)(58,sprite.image.width), height=(std::min)(40,sprite.image.height);
            bounds={float(x+(sprite.image.width-width)/2),float(y+(sprite.image.height-height)/2),float(width),float(height)};
            actor.monsterDamage.enabled=client.damageNumbers;
            const auto glyphs=actor.monsterDamage.draw(x-sprite.x,y-sprite.y,false);
            for (const auto& glyph:glyphs) {
                const auto& image=gameHudFrames_.at(glyph.frame);
                if (image.texture) overlays.push_back({image.texture,glyph.x,glyph.y,image.width,image.height,0});
            }
            animated|=!glyphs.empty();
        } else {
            const int crouch=CrouchPose(actor.crouchedAt,monsterNow);
            const int level=mir3::level_up_pose(actor.levelPoseAt,monsterNow);
            const int cast=actor.casting.pose();
            const auto reviveAge=actor.revivedAt ? monsterNow-actor.revivedAt : HumanReviveDuration;
            const int stance=!actor.mount && monsterNow<actor.combatUntil ?
                560+int((monsterNow-actor.combatIdleAt)/201%3) : actor.revivedAt ? int(reviveAge/201%4) : idle;
            int pose=actor.dead && !actor.dying ? HumanCorpsePose : cast>=0 ? cast : level>=0 ? level : crouch>=0 ? crouch : stance;
            unsigned mount=actor.dead ? 0 : actor.mount;
            const auto& step=actor.remoteStep;
            const auto pixel=ActorWorldPixel(actor);
            auto actorCamera=camera;
            actorCamera.cameraX+=int(actor.x)*48-int(pixel[0]);
            actorCamera.cameraY+=int(actor.y)*32-int(pixel[1]);
            if (step.frame>=0) {
                if (step.opcode==GameBackstep || step.opcode==GameBump) {
                    pose=step.opcode==GameBackstep ? 400 : 1840+step.frame;
                    mount=0;
                } else {
                    const int base=step.opcode==GameRun ? (mount ? 2480 : 1760) : (mount ? 2400 : 1680);
                    pose=base+step.frame;
                }
                actorRow=(int(pixel[1])+16)/32;
            }
            const auto firstLayer=worldSprites_.size();
            const int appearanceWeight=reviveAge<HumanReviveDuration ? int(31*reviveAge/HumanReviveDuration) : -1;
            bounds=AddWorldActor(ui,actor.appearance,actor.facing,actor.x,actor.y,actorCamera,
                pose,client.shadowBlend,mount,appearanceWeight,highlighted,mir3::actor_status_color(actor.actionFlags));
            for (std::size_t i=firstLayer;i<worldSprites_.size();++i) worldSprites_[i].row=int(actorRow);
            if (bounds.w>0) {
                const int x=int(pixel[0])-camera.cameraX,y=int(pixel[1])-camera.cameraY;
                worldAnchors_.push_back({id,bounds,x,y,2});
                if (!actor.dead) PrepareCast(ui,actor.casting,x,y,actor.y);
                humanDamage(actor.health.damage,x,y);
                if (monsterNow<actor.health.registeredUntil)
                    humanBar(actor.health.current,actor.health.maximum,x,y);
            }
            animated|=!actor.dead || actor.dying;
        }
        if (bounds.w<=0) continue;
        if (targetable) worldTargets_.push_back({bounds,id,actorRow,actor.IsMonster()});
        else if (actor.CanBeButchered()) worldCorpses_.push_back({bounds,id,actor.y,actor.IsMonster()});
    }
    if (client.characterAppearance.available) {
        const int crouch=CrouchPose(scene.crouchedAt,monsterNow);
        const int level=mir3::level_up_pose(scene.levelPoseAt,monsterNow);
        const int cast=scene.casting.pose();
        const auto& step=scene.localStep;
        int pose=cast>=0 ? cast : level>=0 ? level : crouch>=0 ? crouch : idle;
        unsigned mount=cast>=0 && scene.casting.clock.action()==32 ? 0 : client.mount.shape;
        if (scene.dead) {
            pose=scene.dying ? cast : HumanCorpsePose;
            mount=0;
        } else if (step.frame>=0) {
            if (step.walking()) {
                const int base=step.opcode==GameRun ? (mount ? 2480 : 1760) : (mount ? 2400 : 1680);
                pose=base+step.frame;
            }
            else {
                pose=step.opcode==GameBackstep ? 400 : 1840+step.frame;
                mount=0;
            }
        }
        auto localCamera=camera;
        localCamera.cameraX+=int(scene.x)*48-playerPixel[0];
        localCamera.cameraY+=int(scene.y)*32-playerPixel[1];
        const auto firstLayer=worldSprites_.size();
        const auto localBounds=AddWorldActor(ui,client.characterAppearance,scene.facing,scene.x,scene.y,localCamera,
            pose,client.shadowBlend,mount,-1,false,mir3::actor_status_color(scene.actionFlags));
        if (step.frame>=0) for (std::size_t i=firstLayer;i<worldSprites_.size();++i)
            worldSprites_[i].row=(playerPixel[1]+16)/32;
        if (localBounds.w>0) {
            const int x=playerPixel[0]-camera.cameraX, y=playerPixel[1]-camera.cameraY;
            worldAnchors_.push_back({0,localBounds,x,y,3});
            PrepareCast(ui,scene.casting,x,y,scene.y);
            humanBar(scene.dead ? 0 : client.characterStatus.hp[0],client.characterStatus.hp[1],x,y);
            humanDamage(scene.health.damage,x,y);
        }
        animated=true;
    }
    std::stable_sort(worldSprites_.begin(),worldSprites_.end(),[](const SceneSprite& a,const SceneSprite& b) {
        return a.row<b.row;
    });
    camera.overlayStart=worldSprites_.size();
    worldSprites_.insert(worldSprites_.end(),overlays.begin(),overlays.end());
    std::stable_sort(worldNpcs_.begin(),worldNpcs_.end(),[](const SceneActorHit& a,const SceneActorHit& b) {
        return a.row<b.row;
    });
    std::stable_sort(worldTargets_.begin(),worldTargets_.end(),[](const SceneActorHit& a,const SceneActorHit& b) {
        return a.row==b.row ? a.monster && !b.monster : a.row<b.row;
    });
    if (animated) cui_request_wakeup(ui,0.05f);
    PrepareWeather(ui,client,camera);
    PrepareSceneLight(client,camera);
    PrepareLevelEffects(ui,scene,camera);
    PrepareSpellEffects(ui,scene,camera);
    PrepareWorldText(ui,client,pointer,interactive);
    TrimWorldFrames(ui);
    return camera;
}

void Dialogs::Assets::PrepareWeather(cui_ctx* ui, const LoginClient& client, SceneMapArt& camera)
{
    const auto& weather=client.scene.weather;
    if ((!client.snowEnabled || !weather.fog) && !weather.other) return;
    worldWeather_.clear();
    camera.weather=&worldWeather_;
    const auto frame=[&](int index) { return GetWorldFrame(ui,"ProgUse",index).image.texture; };
    if (client.snowEnabled && weather.fog) {
        if (const auto texture=frame(550)) {
            const float x=-1600+weather.fog_x, y=-300-weather.fog_y;
            worldWeather_.push_back({texture,{{{x,y},{x+4000,y},{x+4000,y+1200},{x,y+1200}}},{100,200,200,255},true,CUI_EFFECT_SCREEN});
        }
    }
    const auto particle=[&](const mir3::SnowWeather::Particle& p,cui_texture texture,float halfWidth,float halfHeight,float angle,bool screen) {
        if (!texture || p.x+halfWidth+halfHeight<0 || p.x-halfWidth-halfHeight>800 ||
            p.y+halfWidth+halfHeight<0 || p.y-halfWidth-halfHeight>600) return;
        SceneEffectQuad sprite{texture,{{{-halfWidth,-halfHeight},{halfWidth,-halfHeight},
            {halfWidth,halfHeight},{-halfWidth,halfHeight}}},{p.rgb[0],p.rgb[1],p.rgb[2],255},false,
            screen ? CUI_EFFECT_SCREEN : CUI_EFFECT_PARTICLE};
        const float sn=std::sin(angle), cs=std::cos(angle);
        for (auto& point:sprite.corners) {
            const auto [x,y]=point;
            point={p.x+x*cs-y*sn,p.y+x*sn+y*cs};
        }
        worldWeather_.push_back(sprite);
    };
    if (client.snowEnabled && weather.snow) {
        const auto texture=frame(500);
        for (const auto& p:weather.particles)
            if (p.active) particle(p,texture,p.size*0.5f,p.size*0.5f,0,false);
    }
    if (weather.other) for (const auto& p:weather.rain_particles) {
        if (!p.active) continue;
        const float dx=p.x-p.previous_x;
        const float angle=dx==0 ? 0.0f : std::atan2((p.y-p.previous_y)/dx,1.0f)-1.57079632679f;
        particle(p,frame(p.texture_frame()),p.splash() ? 16.0f : 0.75f,p.splash() ? 16.0f : p.size*0.5f,angle,true);
    }
    cui_request_wakeup(ui,0.016f);
}

void Dialogs::Assets::PrepareLevelEffects(cui_ctx* ui, const LocalScene& scene, SceneMapArt& camera)
{
    if (scene.levelEffects.empty()) return;
    worldLevelEffects_.clear();
    for (const auto& playback:scene.levelEffects) {
        const auto& effect=playback.effect;
        const auto& frame=GetWorldFrame(ui,"MagicEx",static_cast<int>(effect.frame));
        const auto& image=frame.image;
        if (!image.texture) continue;
        const float x=float(effect.cell_x*48-camera.cameraX+frame.x), y=float(effect.cell_y*32-camera.cameraY+frame.y);
        const auto& tint=effect.resource.tint;
        worldLevelEffects_.push_back({image.texture,{{{x,y},{x+image.width,y},{x+image.width,y+image.height},{x,y+image.height}}},
            {tint[0],tint[1],tint[2],255},true,CUI_EFFECT_SCREEN});
    }
    camera.levelEffects=&worldLevelEffects_;
    cui_request_wakeup(ui,0.016f);
}

void Dialogs::Assets::PrepareCast(cui_ctx* ui, const mir3::CastVisual& cast, int x, int y, int row)
{
    if (!cast.resource || cast.clock.finished()) return;
    const auto& resource=*cast.resource;
    const int relative=cast.weapon ? cast.clock.frame() : cast.clock.spell_frame();
    const int limit=cast.clock.spell_frame_limit();
    const int radius=cast.weapon ? (resource.end-resource.first<=8 ? relative : relative/2)+3 :
        (relative<limit/2 ? relative+1 : limit-relative);
    if (radius>0 && resource.outer_radius) {
        const auto& inner=resource.inner_color;
        const auto& outer=resource.outer_color;
        worldSpellLights_.push_back({x+24,y+16,radius-1,{inner[0],inner[1],inner[2]},radius,{outer[0],outer[1],outer[2]}});
    }
    const auto* archive=mir3::spell_archive(resource.library);
    if (!archive || resource.blend>2 || resource.first>INT_MAX-100) return;
    const int index=int(resource.first)+10*cast.direction+relative;
    const auto& frame=GetWorldFrame(ui,archive,index);
    if (!frame.image.texture) return;
    const auto blend=resource.blend==0 ? CUI_EFFECT_PREMULTIPLIED :
        resource.blend==1 ? CUI_EFFECT_ADD : CUI_EFFECT_SCREEN;
    const auto& tint=cast.tint;
    worldSprites_.push_back({frame.image.texture,x+frame.x,y+frame.y,frame.image.width,frame.image.height,row,255,1,
        blend,{tint[0],tint[1],tint[2],resource.blend==0 ? resource.opacity : std::uint8_t(255)}});
    cui_request_wakeup(ui,0.016f);
}

void Dialogs::Assets::PrepareSpellEffects(cui_ctx* ui,LocalScene& scene,SceneMapArt& camera)
{
    if (!scene.spellEffects) return;
    auto& spells=*scene.spellEffects;
    worldSpellEffects_.clear();
    const auto sprite=[&](unsigned library,int index,mir3::SpellPixel position,cui_color color,cui_effect_blend blend) {
        const auto* archive=mir3::spell_archive(library);
        if (!archive) return;
        const auto& frame=GetWorldFrame(ui,archive,index);
        const auto& image=frame.image;
        if (!image.texture) return;
        spells.archiveOffsets[library]={frame.x,frame.y};
        const float x=position[0]-camera.cameraX+frame.x, y=position[1]-camera.cameraY+frame.y;
        worldSpellEffects_.push_back({image.texture,
            {{{x,y},{x+image.width,y},{x+image.width,y+image.height},{x,y+image.height}}},color,true,blend});
    };
    for (const auto& effect:spells.effects) {
        if (!effect.drawsSprite()) continue;
        const auto& tint=effect.resource.tint;
        const auto blend=effect.resource.blend==0 ? CUI_EFFECT_PREMULTIPLIED :
            effect.resource.blend==1 ? CUI_EFFECT_ADD : CUI_EFFECT_SCREEN;
        const auto strength=effect.timedResidue ? std::uint8_t(255-effect.fade) : std::uint8_t(255);
        const cui_color color=effect.timedResidue ? cui_color{strength,strength,strength,255} :
            cui_color{tint[0],tint[1],tint[2],effect.resource.blend==0 ? effect.resource.opacity : std::uint8_t(255)};
        sprite(effect.resource.library,effect.frame,effect.position,color,blend);
    }
    for (const auto& [id,event]:spells.mapEvents.sprites) {
        const auto& tint=event.resource.tint;
        const auto strength=event.residueTint;
        const auto color=event.remaining<0 ? cui_color{tint[0],tint[1],tint[2],255} : cui_color{strength,strength,strength,255};
        auto position=mir3::spell_pixel(event.cell);
        sprite(event.resource.library,event.frame,position,color,CUI_EFFECT_SCREEN);
        if (event.layered && event.frame<=100) {
            position[0]+=12;
            position[1]+=8;
            sprite(event.resource.library,event.frame+20,position,color,CUI_EFFECT_SCREEN);
        }
    }
    for (const auto* slots:{&spells.particles.sparks,&spells.particles.glows,&spells.particles.fragments}) {
        for (const auto& slot:*slots) {
            if (!slot || slot->size<=0) continue;
            const auto& p=*slot;
            const auto& image=GetWorldFrame(ui,"ProgUse",p.resource_frame()).image;
            if (!image.texture) continue;
            const float x=p.position[0]-p.size/2, y=p.position[1]-p.size/2;
            worldSpellEffects_.push_back({image.texture,{{{x,y},{x+p.size,y},{x+p.size,y+p.size},{x,y+p.size}}},
                {p.color[0],p.color[1],p.color[2],255},true,p.glow() ? CUI_EFFECT_SCREEN : CUI_EFFECT_SPARK});
        }
    }
    camera.spellEffects=&worldSpellEffects_;
    if (!spells.effects.empty() || !spells.marks.empty() || spells.mapEvents.active() || !worldSpellEffects_.empty())
        cui_request_wakeup(ui,0.016f);
}

void Dialogs::Assets::PrepareSceneLight(LoginClient& client, SceneMapArt& camera)
{
    auto& scene=client.scene;
    if (scene.ambient.level()==255) return;
    using Terrain=mir3_cui::MapTerrain;
    const int left=camera.cameraX/48-20, top=camera.cameraY/32-20;
    if (lightMapName_!=scene.mapId || lightMapRevision_!=scene.terrain.Revision() || lightLeft_!=left || lightTop_!=top) {
        mapLights_=scene.terrain.ReadLights(left,top);
        lightMapName_=scene.mapId;
        lightMapRevision_=scene.terrain.Revision();
        lightLeft_=left;
        lightTop_=top;
    }
    worldLight_=mir3::SceneLightGrid(scene.ambient.level());
    constexpr std::array radii{4,8,12,20};
    constexpr std::array<mir3::SceneLight::Rgb,10> colors{{
        {173,198,214},{90,140,165},{41,57,115},{222,198,99},{41,115,49},
        {255,255,255},{255,255,255},{255,255,255},{255,255,255},{255,255,255}}};
    for (int y=0;y<Terrain::LightRows;++y) for (int x=0;x<Terrain::LightColumns;++x) {
        const auto packed=mapLights_[x*Terrain::LightRows+y];
        if (packed) worldLight_.add({(x+left)*48-camera.cameraX,(y+top)*32-camera.cameraY,
            0,{},radii[packed>>14],colors[(packed>>4)&255]});
    }
    if (client.magicLight) {
        for (const auto& light:worldSpellLights_) worldLight_.add(light);
        for (const auto& playback:scene.levelEffects) worldLight_.add(playback.effect.light(camera.cameraX,camera.cameraY));
        if (scene.spellEffects) for (const auto& effect:scene.spellEffects->effects)
            if (!effect.delay && effect.light) worldLight_.add(effect.illumination(camera.cameraX,camera.cameraY));
    }
    worldLight_.add(mir3::SceneLight::actor(int(scene.x)*48-camera.cameraX,int(scene.y)*32-camera.cameraY,scene.light,true));
    for (int kind : {2,0,1}) for (const auto& anchor:worldAnchors_) {
        if (anchor.order!=kind) continue;
        const auto& actor=scene.players.at(anchor.actor);
        if (actor.IsMonster() && actor.dead) continue;
        worldLight_.add(mir3::SceneLight::actor(anchor.x+24,anchor.y+16,actor.light,false));
    }
    camera.light=&worldLight_;
}

void Dialogs::Assets::PrepareWorldText(cui_ctx* ui, const LoginClient& client, cui_vec2 pointer, bool interactive)
{
    if (worldText_.size()>256) {
        for (auto& [key,image]:worldText_) Destroy(ui,image);
        worldText_.clear();
    }
    const auto raster=[&](const std::string& caption,int width) -> const Image& {
        auto [entry,added]=worldText_.try_emplace({caption,width});
        if (added) {
            const bool name=width<0;
            const int size=name ? mir3::SceneName::rect(0,0,
                mir3::raster_text(caption,800,15,false,mir3::TextFont::normal).text_width,0)[2] : width;
            if (size>0) entry->second=Raster(ui,caption,size,name ? 15 : 14,name,mir3::TextFont::normal);
        }
        return entry->second;
    };
    const SceneActorAnchor* hovered=nullptr;
    for (const auto& anchor:worldAnchors_) {
        if (interactive && Contains(anchor.bounds,pointer) && (!hovered ||
            std::tie(anchor.y,anchor.order,anchor.actor)>std::tie(hovered->y,hovered->order,hovered->actor))) hovered=&anchor;
        const auto found=client.scene.speech.find(anchor.actor);
        if (found==client.scene.speech.end()) continue;
        const auto& bubble=found->second;
        worldSpeech_.push_back({0,bubble.background(anchor.x,anchor.y),{80,60,40,120}});
        for (int row=0;row<int(bubble.lines.size());++row) {
            const auto& line=bubble.lines[row];
            const auto& image=raster(line.text,line.width);
            if (!image.texture) continue;
            const auto [x,y]=bubble.position(row,anchor.x,anchor.y);
            worldSpeech_.push_back({image.texture,{x+1,y+1,line.width,14},{10,10,10,255}});
            worldSpeech_.push_back({image.texture,{x,y,line.width,14},{255,255,255,255}});
        }
    }
    if (!hovered) return;
    const mir3::SceneName local{{ConvertText(client.gameCharacter,CP_UTF8,936),{},{}},0xffffff};
    const auto& name=hovered->actor ? client.scene.players.at(hovered->actor).identity : local;
    if (name.lines[0].empty()) return;
    const cui_color ink{static_cast<std::uint8_t>(name.color),static_cast<std::uint8_t>(name.color>>8),
        static_cast<std::uint8_t>(name.color>>16),255};
    for (int line=0;line<int(name.lines.size());++line) {
        if (name.lines[line].empty()) continue;
        const auto& image=raster(ConvertText(name.lines[line],936,CP_UTF8),-1);
        if (!image.texture) continue;
        const auto rect=mir3::SceneName::rect(hovered->x,hovered->y,image.width,line);
        for (const auto& [dx,dy]:std::array<std::array<int,2>,4>{{{-1,-1},{1,-1},{-1,1},{1,1}}})
            worldNames_.push_back({image.texture,{rect[0]+dx,rect[1]+dy,rect[2],rect[3]},{10,10,10,255}});
        worldNames_.push_back({image.texture,rect,ink});
    }
}

void Dialogs::Assets::OpenGameHud(cui_ctx* ui, mir3::WilArchive& archive, const std::filesystem::path& portraits)
{
    using Layout = mir3::HudLayout;
    for (const int id : {1160,1161,1162,1163,1164,1165,1166,1167,1200,1201,1202,1203,1207,1490,1500,1501,1502,1503,1334,1336})
        gameHudFrames_[id] = UploadFrame(ui,archive,id);
    for (const auto& range : {std::pair{1790,1800},std::pair{1810,1821},std::pair{1830,1840},std::pair{1850,1850}})
        for (int id=range.first;id<=range.second;++id) gameHudFrames_[id]=UploadFrame(ui,archive,id,true);
    for (std::size_t i=0;i<Layout::buttons.size();++i) {
        const int id=Layout::buttons[i][0];
        auto& frame=gameHudFrames_[id];
        frame=UploadFrame(ui,archive,id);
        if (i>=5 && i<=13) gameHudFrames_[id+1]=UploadFrame(ui,archive,id+1);
        gameHud_.button_sizes[i]={frame.width,frame.height};
    }
    for (int id=2600;id<=2644;++id) gameHudHints_[id]=MakeHudHint(ui,id);
    mir3::WilArchive portraitsArchive;
    if (portraitsArchive.open(portraits)) {
        for (int id : {0,2,3,631,632,634,635,650,651,652,653,654,655,660,661})
            gamePortraits_[id]=UploadFrame(ui,portraitsArchive,id,true);
        for (int i=0;i<mir3::GroundItems::sparkle_frames;++i)
            gamePortraits_[40+i]=UploadFrame(ui,portraitsArchive,40+i,true);
        for (int id=600; id<=629; ++id) gamePortraits_[id]=UploadFrame(ui,portraitsArchive,id,true);
    }
    monsterPortraitArchive_.open(portraits.parent_path()/"MonImg");
    for (int i=0;i<7;++i) minimapFrames_[i]=UploadFrame(ui,archive,1482+i,true);
    for (int i=0;i<4;++i) minimapHints_[i]=MakeHudHint(ui,1800+i);
    minimapArchive_.open(portraits.parent_path()/"Mmap");
    fullMinimapArchive_.open(portraits.parent_path()/"Fmmap");
    gameHudReady_=gameHudFrames_.at(1160).texture && gameHudFrames_.at(1161).texture &&
        gameHudFrames_.at(1162).texture && gameHudFrames_.at(1207).texture;
}

void Dialogs::Assets::PrepareMinimap(cui_ctx* ui, LoginClient& client)
{
    auto& map=client.minimap;
    if (std::exchange(client.minimapUnavailable,false))
        client.chat.Append(100,Message(1458),0xffffff,0xff);
    if (const WORD number=std::exchange(client.minimapReply,0)) {
        if (number!=minimapImageNumber_) {
            const int frame=number-1;
            auto image=frame<1000 ? UploadFrame(ui,minimapArchive_,frame,true) :
                UploadFrame(ui,fullMinimapArchive_,frame-1000,true);
            if (image.texture) {
                Destroy(ui,minimapImage_);
                minimapImage_=image;
                minimapImageNumber_=number;
            }
        }
        if (number==minimapImageNumber_ && minimapImage_.texture) {
            map.receive(minimapImage_.width,minimapImage_.height,GetTickCount64());
            if (client.portraitMapPending) client.portraitMapReady=client.portraitMapMode;
            client.portraitMapPending=false;
            client.minimapDeadline=0;
        }
    }
    map.player_x=static_cast<int>(client.scene.x);
    map.player_y=static_cast<int>(client.scene.y);
    map.type_five_actors.clear();
    for (const auto& [id,actor]:client.scene.players)
        if (actor.npcType==5 && !actor.dead)
            map.type_five_actors.push_back({static_cast<int>(actor.x),static_cast<int>(actor.y)});
    minimapDrawing_=&map;
    portraitMapMode_=client.portraitMapMode;
    portraitMapReady_=client.portraitMapReady;
    const auto phase=static_cast<std::uint64_t>(cui_time(ui)*1000)%801;
    minimapPlayerMark_=phase<500;
    if (map.visible)
        cui_request_wakeup(ui,static_cast<float>((phase<500 ? 500 : 801)-phase)*0.001f);
    if (portraitMapReady_ && !client.chat.expanded) {
        portraitMapBlinkMs_=std::fmod(portraitMapBlinkMs_+cui_delta_time(ui)*1000,801.0);
        cui_request_wakeup(ui,static_cast<float>((portraitMapBlinkMs_<500 ? 500 : 801)-portraitMapBlinkMs_)*0.001f);
    }
}

bool Dialogs::Assets::HandleMinimapInput(cui_ctx* ui, LoginClient& client, bool modal)
{
    minimapHint_=-1;
    auto& map=client.minimap;
    const auto value=cui_viewport_size(ui);
    const HudPoint base({0,0,value.x,value.y});
    if (base.scale<=0 || !map.visible) return false;
    const auto pointer=base.ToLocal(cui_mouse_pos(ui));
    const auto [x,y,w,h]=map.bounds();
    const bool inside=Contains({float(x),float(y),float(w),float(h)},pointer);
    const bool down=cui_mouse_clicked(ui,CUI_MOUSE_LEFT), up=cui_mouse_released(ui,CUI_MOUSE_LEFT);
    const bool captured=map.release_captured;
    if (up) map.release_captured=map.dragging=false;
    if (up && captured) return true;
    if (modal) return false;
    const bool ctrl=(cui_key_mods(ui)&CUI_MOD_CTRL)!=0;
    if (inside) {
        const auto hit=[&](int index) {
            const auto [left,top]=map.button_position(index);
            return Contains({float(left),float(top),19,19},pointer);
        };
        if (down) {
            if (ctrl) {
                map.drag_offset={static_cast<int>(pointer.x)-x,static_cast<int>(pointer.y)-y};
                map.dragging=map.release_captured=true;
            } else if (hit(0)) {
                map.translucent=!map.translucent;
                map.release_captured=true;
            } else if (hit(1)) {
                map.resize();
                map.release_captured=true;
            }
        } else if (map.dragging && ctrl) map.position={static_cast<int>(pointer.x)-map.drag_offset[0],
            static_cast<int>(pointer.y)-map.drag_offset[1]};
        if (!map.release_captured)
            for (int i=0;i<2;++i) if (hit(i)) {
                const int hint=map.button_hint(i)-1800;
                if (minimapHints_[hint].texture) {
                    minimapHint_=i;
                    minimapHintHeight_=minimapTooltip_.Advance(ui,1800+hint,minimapHints_[hint].height);
                }
            }
    }
    return inside || captured;
}

void Dialogs::Assets::PaintMinimap(cui_ctx* ui, cui_rect rect) const
{
    const auto& map=*minimapDrawing_;
    const HudPoint base(rect);
    const auto [x,y,w,h]=map.bounds();
    const auto [sx,sy]=map.source();
    const auto scaleX=static_cast<float>(minimapImage_.width),scaleY=static_cast<float>(minimapImage_.height);
    cui_draw_image(ui,minimapImage_.texture,base.Rect(x,y,w,h),
        {sx/scaleX,sy/scaleY,w/scaleX,h/scaleY},{},{150,150,150,static_cast<std::uint8_t>(map.translucent ? 175 : 255)});
    constexpr cui_color border{100,100,100,255}, green{100,250,100,255}, orange{250,150,50,255};
    const auto outline=[&](int left,int top,int right,int bottom,cui_color color) {
        // The reference draws lines on integer coordinates with the top and left pixels outside the rectangle.
        cui_draw_rect(ui,base.Rect(left,top-1,right-left,1),{},color);
        cui_draw_rect(ui,base.Rect(left,bottom-1,right-left,1),{},color);
        cui_draw_rect(ui,base.Rect(left-1,top,1,bottom-top),{},color);
        cui_draw_rect(ui,base.Rect(right-1,top-1,1,bottom-top+1),{},color);
    };
    outline(x,y,x+w,y+h,border);
    cui_push_clip(ui,base.Rect(x,y,w,h));
    const int px=static_cast<int>(mir3::Minimap::map_pixel_x(map.player_x))-2, py=map.player_y-2;
    const auto markers=[&](const auto& list,cui_color color,bool nearby) {
        for (const auto& [cx,cy]:list) {
            const int mx=mir3::Minimap::actor_marker_x(cx), my=cy-1;
            if (nearby && (std::abs(mx-px)>=map.size()/2 || std::abs(my-py)>=map.size()/2)) continue;
            cui_draw_rect(ui,base.Rect(x+mx-sx-nearby,y+my-sy-nearby,nearby ? 3 : 2,nearby ? 3 : 2),{},color);
        }
    };
    if (showMinimapActors_) markers(map.type_five_actors,{255,255,0,255},false);
    markers(map.nearby_green,green,true);
    markers(map.nearby_orange,orange,true);
    cui_pop_clip(ui);
    if (minimapPlayerMark_) {
        const int left=(std::max)(x,x+px-sx), top=(std::max)(y,y+py-sy);
        const int right=(std::min)(x+w,x+px-sx+4), bottom=(std::min)(y+h,y+py-sy+4);
        if (left<right && top<bottom) outline(left,top,right,bottom,green);
    }
    const auto sprite=[&](int id,int left,int top) {
        const auto& art=minimapFrames_[id-1482];
        if (art.texture) cui_draw_image(ui,art.texture,base.Rect(left,top,art.width,art.height),{0,0,1,1},{},
            {255,255,255,255});
    };
    sprite(1486,x,y);
    sprite(1487,x+w-32,y);
    sprite(1488,x,y+h-35);
    const auto [tx,ty]=map.button_position(0);
    const auto [zx,zy]=map.button_position(1);
    sprite(map.translucent ? 1482 : 1483,tx,ty);
    sprite(map.large ? 1485 : 1484,zx,zy);
    if (minimapHint_>=0) {
        const auto [left,top]=map.button_position(minimapHint_);
        const auto& hint=minimapHints_[map.button_hint(minimapHint_)-1800];
        HudHint::Paint(ui,base,hint,minimapHintHeight_,left+19,top+19);
    }
}

void Dialogs::Assets::PaintPortraitMinimap(cui_ctx* ui, const HudPoint& base) const
{
    if (!minimapDrawing_ || !portraitMapReady_ || !minimapImage_.texture) return;
    const auto& map=*minimapDrawing_;
    const bool full=minimapImageNumber_>=1001;
    const auto [sx,sy,w,h]=map.portrait_source(full);
    const int x=!full || w>=256 ? 510 : 530, y=!full || h>=256 ? 470 : 485;
    const int sourceWidth=full ? 256 : 180, sourceHeight=full ? 256 : 130;
    const auto scaleX=static_cast<float>(minimapImage_.width), scaleY=static_cast<float>(minimapImage_.height);
    cui_push_clip(ui,base.Rect(x,y,128,128));
    cui_draw_rect(ui,base.Rect(x,y,128,128),{},{0,0,0,255});
    cui_draw_image(ui,minimapImage_.texture,base.Rect(x,y,w*128.f/sourceWidth,h*128.f/sourceHeight),
        {sx/scaleX,sy/scaleY,w/scaleX,h/scaleY},{},{150,150,150,255});
    constexpr cui_color border{100,100,100,255},green{100,250,100,255},orange{250,150,50,255};
    const auto outline=[&](int left,int top,int size,cui_color color) {
        cui_draw_rect(ui,base.Rect(left,top,size,1),{},color);
        cui_draw_rect(ui,base.Rect(left,top+size-1,size,1),{},color);
        cui_draw_rect(ui,base.Rect(left,top,1,size),{},color);
        cui_draw_rect(ui,base.Rect(left+size-1,top,1,size),{},color);
    };
    outline(x,y,128,border);
    const auto marker=[&](mir3::HudLayout::Point cell,bool player) {
        return mir3::Minimap::portrait_marker(cell,{sx,sy},{x,y},full,player);
    };
    const auto [px,py]=marker({map.player_x,map.player_y},true);
    const auto dots=[&](const auto& cells,cui_color color) {
        for (const auto& cell:cells) {
            const auto [mx,my]=marker(cell,false);
            if (std::abs(mx-px)>=sourceWidth/2 || std::abs(my-py)>=sourceHeight/2) continue;
            cui_draw_rect(ui,base.Rect(mx-1,my-1,3,3),{},color);
        }
    };
    dots(map.nearby_green,green);
    dots(map.nearby_orange,orange);
    if (portraitMapBlinkMs_>0 && portraitMapBlinkMs_<500) outline(px,py,4,green);
    cui_pop_clip(ui);
}

void Dialogs::Assets::DrawMinimapPanel(cui_ctx* ui, const LoginClient& client)
{
    if (!client.minimap.visible || !minimapImage_.texture) return;
    showMinimapActors_=client.minimapShowActors;
    const auto view=cui_viewport_size(ui);
    cui_overlay_begin(ui,{0,0,view.x,view.y});
    auto* self=this;
    cui_canvas_paint(ui,CUI_FILL,CUI_FILL,[](cui_ctx* ui,cui_rect r,void* data) {
        (*static_cast<Assets**>(data))->PaintMinimap(ui,r);
    },&self,sizeof self);
    cui_box_end(ui);
}

void Dialogs::Assets::DrawObjectOrderMap(cui_ctx* ui, LoginClient& client, float height)
{
    auto& orders = client.windows.orders;
    const auto object = client.scene.players.find(orders.object);
    const int x = object == client.scene.players.end() ? orders.x : object->second.x;
    const int y = object == client.scene.players.end() ? orders.y : object->second.y;
    const int width = std::min(minimapImage_.width, 180), mapHeight = std::min(minimapImage_.height, 165);
    const int sx = std::clamp(static_cast<int>(mir3::Minimap::map_pixel_x(x)) - width / 2, 0, minimapImage_.width - width);
    const int sy = std::clamp(y - mapHeight / 2, 0, minimapImage_.height - mapHeight);
    struct MapPaint {
        cui_texture image;
        cui_rect uv;
        cui_vec2 origin, target;
        float radiusX, radiusY;
    };
    const float mw = static_cast<float>(std::max(1, minimapImage_.width));
    const float mh = static_cast<float>(std::max(1, minimapImage_.height));
    const auto marker = [&](int px, int py) -> cui_vec2 {
        return {float(mir3::Minimap::map_pixel_x(px) - sx) / std::max(1, width), float(py - sy) / std::max(1, mapHeight)};
    };
    const bool ready = minimapImage_.texture && client.minimap.width == minimapImage_.width && width && mapHeight;
    const auto target = orders.targetX && orders.targetY ? marker(orders.targetX, orders.targetY) : cui_vec2{-1,-1};
    const MapPaint paint{ready ? minimapImage_.texture : 0, {sx/mw, sy/mh, width/mw, mapHeight/mh}, marker(x,y), target,
        (orders.range == 1 ? 80.0f : 50.0f) / std::max(1, width),
        (orders.range == 1 ? 50.0f : 29.0f) / std::max(1, mapHeight)};
    cui_canvas_paint(ui, CUI_FILL, CUI_PX(height), [](cui_ctx* ui, cui_rect box, void* data) {
        const auto& map = *static_cast<MapPaint*>(data);
        cui_draw_rect(ui, box, {}, {24,25,22,255});
        if (!map.image) return;
        cui_draw_image(ui, map.image, box, map.uv, {}, {165,165,165,255});
        const auto point = [&](cui_vec2 pos, cui_color color) {
            if (pos.x >= 0 && pos.y >= 0 && pos.x < 1 && pos.y < 1)
                cui_draw_rect(ui, {box.x+pos.x*box.w-2, box.y+pos.y*box.h-2, 5,5}, {}, color);
        };
        const float left = box.x+(map.origin.x-map.radiusX)*box.w, top = box.y+(map.origin.y-map.radiusY)*box.h;
        const float w = 2*map.radiusX*box.w, h = 2*map.radiusY*box.h;
        constexpr cui_color edge{172,155,114,255};
        cui_push_clip(ui, box);
        cui_draw_rect(ui, {left,top,w,1}, {}, edge);
        cui_draw_rect(ui, {left,top+h,w,1}, {}, edge);
        cui_draw_rect(ui, {left,top,1,h}, {}, edge);
        cui_draw_rect(ui, {left+w,top,1,h}, {}, edge);
        point(map.origin, {255,224,140,255});
        point(map.target, {255,90,62,255});
        cui_pop_clip(ui);
    }, &paint, sizeof paint);
    const auto rect = cui_last_rect(ui);
    const auto pointer = cui_mouse_pos(ui);
    if (!cui_is_disabled(ui) && ready && rect.w > 0 && rect.h > 0 && Contains(rect, pointer) && cui_mouse_clicked(ui, CUI_MOUSE_LEFT)) {
        const float px = (pointer.x - rect.x) / rect.w, py = (pointer.y - rect.y) / rect.h;
        const auto targetX = static_cast<int>((sx + px * width) * 2 / 3), targetY = static_cast<int>(sy + py * mapHeight);
        const float dx = std::abs(px-paint.origin.x), dy = std::abs(py-paint.origin.y);
        const float innerScaleX = orders.range == 1 ? 55.0f/80 : 28.0f/50;
        const float innerScaleY = orders.range == 1 ? 34.0f/50 : 16.0f/29;
        if (dx <= paint.radiusX && dy <= paint.radiusY &&
            (dx > paint.radiusX*innerScaleX || dy > paint.radiusY*innerScaleY) && client.scene.terrain.Contains(targetX,targetY)) {
            orders.targetX = targetX;
            orders.targetY = targetY;
        }
    }
}

void Dialogs::Assets::DrawIndependentSkin(cui_ctx* ui, int panel, cui_rect bounds)
{
    const auto& skin = independentSkins_[panel];
    const auto size = IndependentSizes[panel];
    struct Frame { cui_texture texture; cui_rect uv; } frame{skin.texture,{}};
    if (skin.texture) {
        const float w = static_cast<float>(skin.width), h = static_cast<float>(skin.height);
        frame.uv = {(w-size.x)/(2*w),(h-size.y)/(2*h),size.x/w,size.y/h};
    }
    cui_canvas_paint(ui,CUI_FILL,CUI_PX(bounds.h),[](cui_ctx* ui, cui_rect box, void* data) {
        const auto& art = *static_cast<Frame*>(data);
        cui_draw_rect(ui,box,{},CUI_RGB(0x252316));
        if (art.texture) cui_draw_image(ui,art.texture,box,art.uv,{},CUI_RGB(0xffffff));
    },&frame,sizeof frame);
}

cui_vec2 Dialogs::Assets::IndependentButtonSize(int frame) const
{
    const auto image=independentButtons_.find(frame);
    if (image==independentButtons_.end() || !image->second.texture) return {};
    return {float(image->second.width),float(image->second.height)};
}

void Dialogs::Assets::DrawIndependentButton(cui_ctx* ui, int frame, float height, int state) const
{
    const auto found=independentButtons_.find(frame);
    if (found==independentButtons_.end() || !found->second.texture) return;
    const struct { cui_texture texture; int state; } draw{found->second.texture,state};
    cui_canvas_paint(ui,CUI_FILL,CUI_PX(height),[](cui_ctx* ui,cui_rect box,void* value) {
        const auto& art=*static_cast<decltype(draw)*>(value);
        const uint8_t shade=art.state==-1 ? 125 : art.state==-2 ? 185 : 255;
        cui_draw_image(ui,art.texture,box,{0,0,1,1},{},{shade,shade,shade,255});
        if (art.state==1)
            cui_draw_image(ui,art.texture,box,{0,0,1,1},{},{255,240,180,80});
    },&draw,sizeof draw);
}

void Dialogs::Assets::DrawMailModeSkin(cui_ctx* ui, int mode, float height)
{
    const auto& art = mailModeSkins_[mode];
    if (!art.texture) return;
    cui_canvas_paint(ui,CUI_FILL,CUI_PX(height),[](cui_ctx* ui,cui_rect box,void* data) {
        const auto texture = *static_cast<cui_texture*>(data);
        cui_draw_image(ui,texture,box,{0,0,1,1},{},CUI_RGB(0xffffff));
    },&art.texture,sizeof art.texture);
}

void Dialogs::Assets::DrawIndependentItem(cui_ctx* ui, unsigned icon, float height)
{
    CacheItem(ui,icon);
    const auto& item = itemFrames_.at(icon);
    if (!item.texture) return;
    cui_canvas_paint(ui,CUI_FILL,CUI_PX(height),[](cui_ctx* ui, cui_rect box, void* data) {
        const auto& image = *static_cast<Image*>(data);
        const float scale = std::min({1.0f,box.w/image.width,box.h/image.height});
        const float w = image.width*scale, h = image.height*scale;
        cui_draw_image(ui,image.texture,{box.x+(box.w-w)/2,box.y+(box.h-h)/2,w,h},{0,0,1,1},{},CUI_RGB(0xffffff));
    },&item,sizeof item);
}

bool Dialogs::Assets::DrawInspectionEquipment(cui_ctx* ui, const IndependentWindows::Profile& profile, cui_rect bounds)
{
    if (!independentSkins_[0].texture) return false;
    constexpr std::array<cui_rect,12> slots{{
        {135,114,50,90}, {80,70,50,88}, {102,293,38,38}, {223,93,38,38},
        {135,75,38,27}, {22,183,38,38}, {223,183,38,38}, {22,224,38,38},
        {223,224,38,38}, {142,293,38,38}, {22,279,38,38}, {182,293,38,38}
    }};
    struct Paint {
        WorldFrame body;
        std::array<Image,12> icons;
        std::array<cui_rect,12> rects;
    } paint{statusBodies_[profile.body == 1], {}, slots};
    for (std::size_t i=0; i<slots.size(); ++i) {
        const auto& item = profile.equipment[i];
        if (item.label.empty()) continue;
        CacheItem(ui, item.icon);
        paint.icons[i] = itemFrames_.at(item.icon);
    }
    cui_canvas_paint(ui, CUI_FILL, CUI_PX(bounds.h), [](cui_ctx* ui, cui_rect box, void* data) {
        const auto& art = *static_cast<Paint*>(data);
        const float scale = std::min(box.w/284, box.h/348);
        const float x = box.x+(box.w-284*scale)/2, y = box.y+(box.h-348*scale)/2;
        const auto image = [&](const Image& image, cui_rect rect) {
            if (image.texture && rect.w > 0 && rect.h > 0)
                cui_draw_image(ui, image.texture, {x+rect.x*scale,y+rect.y*scale,rect.w*scale,rect.h*scale},
                    {0,0,1,1}, {}, {255,255,255,255});
        };
        image(art.body.image, {float(126+art.body.x),float(219+art.body.y),float(art.body.image.width),float(art.body.image.height)});
        for (std::size_t i=0; i<art.icons.size(); ++i) {
            const auto& icon = art.icons[i];
            const auto& rect = art.rects[i];
            const auto width = std::min(float(icon.width), rect.w), height = std::min(float(icon.height), rect.h);
            image(icon, {rect.x+(rect.w-width)/2,rect.y+(rect.h-height)/2,width,height});
        }
    }, &paint, sizeof paint);
    const auto rect = cui_last_rect(ui);
    const float scale = std::min(rect.w/284, rect.h/348);
    if (scale <= 0) return true;
    const auto pointer = cui_mouse_pos(ui);
    const cui_vec2 local{(pointer.x-rect.x-(rect.w-284*scale)/2)/scale, (pointer.y-rect.y-(rect.h-348*scale)/2)/scale};
    for (std::size_t i=0; i<slots.size(); ++i) {
        const auto& item = profile.equipment[i];
        if (!item.label.empty() && Contains(slots[i],local)) {
            const auto caption = item.label + "\næŒä¹… " + std::to_string(item.durability) + " / " + std::to_string(item.maxDurability);
            cui_tooltip(ui, caption.c_str());
            break;
        }
    }
    return true;
}

const Dialogs::Assets::Image& Dialogs::Assets::OptionFrame(int id) const
{
    const auto frame = std::find(optionsFrameIds_.begin(), optionsFrameIds_.end(), id);
    return optionsFrames_[frame - optionsFrameIds_.begin()];
}

void Dialogs::Assets::OpenGameOptions(cui_ctx* ui, mir3::WilArchive& archive)
{
    for (std::size_t i = 0; i < optionsFrameIds_.size(); ++i)
        optionsFrames_[i] = UploadFrame(ui, archive, optionsFrameIds_[i], true);
    if (!OptionsReady()) return;
    for (auto& caption : optionsCaptions_)
        caption.image = Raster(ui, Message(caption.message), caption.bounds[2], caption.bounds[3], true, mir3::TextFont::normal);
}

void Dialogs::Assets::LeaveOptions(bool resetPlacement)
{
    if (resetPlacement) {
        optionsBounds_ = initialOptionsBounds_;
        utilityOrder_ = initialUtilityOrder_;
        statusPanel_ = mir3::DraggablePanel{{0,0,328,466}};
        statusClosePressed_ = false;
        utilityInventoryOpen_ = false;
        utilityQuickOpen_ = false;
    }
    optionsDragging_ = false;
    optionsAudioDrag_ = -1;
    optionButtonStates_.fill(0);
    optionsPointer_ = {-1,-1};
}

bool Dialogs::Assets::HandleOptionsInput(cui_ctx* ui, LoginClient& client, bool blocked, bool audioOnly)
{
    if (!(audioOnly ? client.audioOpen : client.serverOptions.open)) return false;
    const auto close = [&] {
        if (audioOnly) {
            if (!client.SaveAudioSettings()) return;
            client.audioOpen=false;
        } else client.serverOptions.open=false;
        LeaveOptions();
    };
    if (audioOnly && !blocked && cui_key_pressed(ui,CUI_KEY_ESCAPE)) {
        close();
        return true;
    }
    using Audio = mir3_cui::AudioOptions;
    const auto view = cui_viewport_size(ui);
    const HudPoint base({0,0,view.x,view.y});
    if (base.scale <= 0) return false;
    const auto point = base.ToLocal(cui_mouse_pos(ui));
    const auto previous = std::exchange(optionsPointer_, point);
    const bool moved = point.x != previous.x || point.y != previous.y;
    const auto [x,y,width,height] = optionsBounds_;
    const cui_vec2 local{point.x - x, point.y - y};
    const bool inside = Contains({0,0,float(width),float(height)}, local);
    const bool captured = optionsDragging_ || optionsAudioDrag_ >= 0;
    const bool down = cui_mouse_clicked(ui,CUI_MOUSE_LEFT), up = cui_mouse_released(ui,CUI_MOUSE_LEFT);
    const auto changed = std::exchange(optionsConfirmed_,client.serverOptions.confirmed) ^ client.serverOptions.confirmed;
    for (std::size_t i = 0; i < serverOptionButtons_.size(); ++i)
        if (changed & mir3_cui::ServerOptionList[i].mask) optionButtonStates_[serverOptionButtons_[i]] = 0;
    if ((blocked && !captured) || (!inside && !captured)) {
        optionButtonStates_.fill(0);
        return false;
    }
    auto& audio = client.audioOptions;
    const auto level = [&](int i) -> int& { return i == 0 ? audio.musicLevel : audio.effectsLevel; };
    const auto hit = [&](int id, int left, int top) {
        const auto& frame = OptionFrame(id);
        return frame.texture && Contains({float(left),float(top),float(frame.width),float(frame.height)},local);
    };
    int button = -1;
    if (hit(1221,448,415)) button = 0;
    for (int i = 0; i < 2; ++i) {
        if (hit(1292,430,85+60*i)) button = 1+i;
        if (hit(1291,276+Audio::KnobFromLevel(level(i)),118+60*i)) button = 5+i;
    }
    if (hit(1292,430,205)) button = 3;
    if (!audioOnly) {
        for (std::size_t i = 0; i < localOptionButtons_.size(); ++i) {
            const auto [left,top] = localOptionPositions_[i];
            if (hit(1292,left,top)) button = localOptionButtons_[i];
        }
        for (std::size_t i = 0; i < serverOptionButtons_.size(); ++i) {
            const auto [left,top] = serverOptionPositions_[i];
            if (hit(1292,left,top)) button = serverOptionButtons_[i];
        }
        if (hit(1296,430,265)) button = 19;
    }
    if (moved && !captured)
        for (int i = 0; i < static_cast<int>(optionButtonStates_.size()); ++i)
            if (optionButtonStates_[i] != 2) optionButtonStates_[i] = i == button ? 1 : 0;
    if (down && button >= 0) optionButtonStates_[button] = 2;
    if (up) {
        const auto count = button < 0 ? optionButtonStates_.size() : static_cast<std::size_t>(button+1);
        std::fill_n(optionButtonStates_.begin(), count, 0);
    }
    if (audio.effects && ((down && (button == 5 || button == 6)) ||
        (up && !captured && button >= 0 && button != 5 && button != 6)) && std::filesystem::exists(clickSound_))
        client.audio.PlayEffect(clickSound_.c_str(), audio);

    bool audioChanged = false;
    if (optionsAudioDrag_ >= 0) {
        if (moved) {
            level(optionsAudioDrag_) = Audio::LevelFromKnob(Audio::KnobOffset(static_cast<int>(local.x)));
            audioChanged = true;
        }
        if (up) optionsAudioDrag_ = -1;
    } else if (optionsDragging_) {
        if (moved) {
            optionsBounds_[0] = static_cast<int>(point.x) - optionsGrab_[0];
            optionsBounds_[1] = static_cast<int>(point.y) - optionsGrab_[1];
            if (optionsBounds_[1] + height > 570) optionsBounds_[1] = 565 - height;
        }
        if (up) optionsDragging_ = false;
    } else if (down || up) {
        bool onControl = button >= 0;
        if (down && (button == 5 || button == 6)) optionsAudioDrag_ = button - 5;
        for (int i = 0; i < 2; ++i) {
            if (onControl || local.x <= 266 || local.x >= 487 || local.y <= 116+60*i || local.y >= 136+60*i) continue;
            onControl = true;
            if (up) {
                const auto offset = Audio::KnobFromLevel(level(i));
                level(i) = Audio::LevelFromKnob(Audio::TrackStep(offset, static_cast<int>(local.x)));
                audioChanged = true;
            }
        }
        if (down && !onControl) {
            optionsDragging_ = true;
            optionsGrab_ = {static_cast<int>(local.x),static_cast<int>(local.y)};
        }
        if (up) {
            if (button == 0) close();
            if (button >= 1 && button <= 3) {
                bool& value = button == 1 ? audio.music : button == 2 ? audio.effects : audio.reversePan;
                value = !value;
                audioChanged = true;
            }
            if (button == 19) client.RequestAttackMode();
            for (std::size_t i = 0; i < serverOptionButtons_.size(); ++i)
                if (button == serverOptionButtons_[i]) client.ToggleServerOption(mir3_cui::ServerOptionList[i]);
            for (std::size_t i = 0; i < localOptionButtons_.size(); ++i) {
                if (button != localOptionButtons_[i]) continue;
                const auto& option = LoginClient::LocalOptions[i];
                bool& value = client.*option.value;
                value = !value;
                if (!client.audioOptionsPath.empty())
                    WritePrivateProfileStringW(L"Options",option.key,value ? L"1" : L"0",client.audioOptionsPath.c_str());
                if (option.enabledMessage)
                    client.chat.Append(100,Message(value ? option.enabledMessage : option.disabledMessage),0xffffff,33023);
            }
        }
    }
    if (audioChanged) {
        client.audio.Apply(audio);
        client.audioDirty = true;
        client.SaveAudioSettings();
    }
    return inside || captured;
}

void Dialogs::Assets::PaintGameOptions(cui_ctx* ui, cui_rect rect, const LoginClient& client) const
{
    const HudPoint base(rect);
    const auto x = optionsBounds_[0], y = optionsBounds_[1];
    const auto image = [&](const Image& art, int left, int top, cui_color tint = {255,255,255,255}) {
        if (art.texture) cui_draw_image(ui,art.texture,base.Rect(x+left,y+top,art.width,art.height),{0,0,1,1},{},tint);
    };
    const auto sprite = [&](int id, int left, int top) { image(OptionFrame(id),left,top); };
    const auto toggle = [&](int index, bool value, int left, int top) {
        const auto state = optionButtonStates_[index];
        if (state == 1) sprite(value ? 1293 : 1294,left,top);
        sprite((state == 2 ? !value : value) ? 1292 : 1295,left,top);
    };
    const auto& audio = client.audioOptions;
    using Audio = mir3_cui::AudioOptions;
    sprite(1290,0,0);
    for (int i = 0; i < 2; ++i) {
        toggle(1+i,i == 0 ? audio.music : audio.effects,430,85+60*i);
        sprite(1291,276+Audio::KnobFromLevel(i == 0 ? audio.musicLevel : audio.effectsLevel),118+60*i);
    }
    toggle(3,audio.reversePan,430,205);
    sprite(optionButtonStates_[0] == 2 ? 1222 : 1221,448,415);
    for (std::size_t i = 0; i < localOptionButtons_.size(); ++i) {
        const auto [left,top] = localOptionPositions_[i];
        toggle(localOptionButtons_[i],client.*LoginClient::LocalOptions[i].value,left,top);
    }
    for (std::size_t i = 0; i < serverOptionButtons_.size(); ++i) {
        const auto [left,top] = serverOptionPositions_[i];
        toggle(serverOptionButtons_[i],(client.serverOptions.displayed & mir3_cui::ServerOptionList[i].mask) != 0,left,top);
    }
    sprite(optionButtonStates_[19] == 2 ? 1297 : 1296,430,265);
    constexpr cui_color orange{255,150,0,255};
    for (const auto& caption : optionsCaptions_)
        image(caption.image,caption.bounds[0],caption.bounds[1], caption.message == 2104 ? cui_color{250,250,248,255} : orange);
    image(optionsAttackMode_.image,266,269,orange);
}

void Dialogs::Assets::DrawOptionsPanel(cui_ctx* ui, const LoginClient& client)
{
    if (!client.serverOptions.open && !client.audioOpen) return;
    SetBagText(ui,optionsAttackMode_,Message(2116+client.serverOptions.attackMode),156,21,true);
    const auto view = cui_viewport_size(ui);
    cui_overlay_begin(ui,{0,0,view.x,view.y});
    const struct { Assets* assets; const LoginClient* client; } data{this,&client};
    cui_canvas_paint(ui,CUI_FILL,CUI_FILL,[](cui_ctx* ui,cui_rect rect,void* value) {
        const auto& draw = *static_cast<decltype(data)*>(value);
        draw.assets->PaintGameOptions(ui,rect,*draw.client);
    },&data,sizeof data);
    cui_box_end(ui);
}

void Dialogs::Assets::OpenSkills(cui_ctx* ui, mir3::WilArchive& archive, const std::filesystem::path& root)
{
    const auto load = [&](int frame) { skillFrames_[frame] = UploadFrame(ui,archive,frame,true); };
    for (int frame : {1620,1221,1222,1672}) load(frame);
    for (int tab=0;tab<8;++tab) {
        load(1622+2*tab);
        load(1640+2*tab);
        load(1641+2*tab);
    }
    for (int index=0;index<12;++index) {
        load(1660+index);
        load(1450+index);
    }
    skillIconArchive_.open(root/"Data"/"MIcon");
    skillDescriptions_.load(Read(root/"Magic.exp"));
}

void Dialogs::Assets::ToggleSkills(LoginClient& client)
{
    if (!SkillsReady()) return;
    client.skills.toggle();
    if (client.skills.visible) RaiseUtility(UtilityPanel::Skills);
    client.skills.pressed = -1;
    client.skills.scroll_captured = false;
    skillMenu_ = {};
    client.skills.menu_target = -1;
    skillDescriptionId_ = 0;
}

const mir3::SkillPosition* Dialogs::Assets::SkillAt(cui_vec2 point, const mir3::SkillPanel& skills) const
{
    for (const auto& position : mir3::SkillLayout::positions[skills.tab])
        if (Contains({float(skills.rect[0]+position.x),float(skills.rect[1]+position.y-skills.scroll),40,40},point))
            return &position;
    return nullptr;
}

bool Dialogs::Assets::HandleSkillInput(cui_ctx* ui, LoginClient& client, bool blocked, bool keyboard)
{
    auto& skills = client.skills;
    if (skills.menu_target<0) skillMenu_ = {};
    skillDescriptionId_ = 0;
    if (!skills.visible) return false;
    const auto view = cui_viewport_size(ui);
    const HudPoint base({0,0,view.x,view.y});
    if (base.scale <= 0) return false;
    const auto point = base.ToLocal(cui_mouse_pos(ui));
    const auto prior = std::exchange(skillPointer_,point);
    const auto [x,y,w,h] = skills.rect;
    const bool inside = Contains({float(x),float(y),float(w),float(h)},point);
    const bool down = cui_mouse_clicked(ui,CUI_MOUSE_LEFT), up = cui_mouse_released(ui,CUI_MOUSE_LEFT);
    const bool captured = skills.dragging || skills.scroll_captured || skills.pressed>=0;
    if (blocked && !captured) return false;
    const bool onMenu = !blocked && Contains(skillMenu_,point);
    if (inside && !blocked) {
        if (const auto* position = SkillAt(point,skills)) {
            if (keyboard)
                for (int i=0;i<12;++i)
                    if (cui_shortcut(ui,0,static_cast<cui_key>(CUI_KEY_F1+i))) client.BindSkill(position->id,i+1);
            if (!skills.scroll_captured && (skillMenu_.w==0 || (down && !onMenu)))
                skillDescriptionId_ = position->id;
        }
    }
    if (onMenu) {
        if (up && skills.menu_target>0 && skills.find(skills.menu_target))
            client.BindSkill(skills.menu_target,int(point.y-skillMenu_.y)/40*4+int(point.x-skillMenu_.x)/40+1);
        return true;
    }
    if (down && !blocked) {
        skillMenu_ = {};
        skills.menu_target = -1;
    }
    const auto& positions = mir3::SkillLayout::positions[skills.tab];
    if (inside && !blocked && cui_mouse_double_clicked(ui)) {
        for (int i=0;i<10;++i) {
            const auto [id,left,top] = positions[i];
            if (!id || !skills.find(id) || !Contains({float(x+left),float(y+top),40,40},point)) continue;
            const int selected = skills.selected[skills.tab];
            skills.menu_target = selected>=0 && selected<static_cast<int>(positions.size()) ? positions[selected].id : 0;
            const float menuX = static_cast<float>((std::min)(int(point.x),x+w-160));
            const float menuY = static_cast<float>((point.y+160>x+w ? y+h-120 : int(point.y))+40);
            skillMenu_ = {menuX,menuY,160,120};
            skills.end_drag();
            skills.pressed = -1;
            return true;
        }
    }
    const auto buttonAt = [&] {
        for (int i=0;i<9;++i) {
            const auto [id,left,top] = mir3::SkillPanel::button(i);
            const auto& art = skillFrames_.at(id);
            if (art.texture && Contains({float(x+left),float(y+top),float(art.width),float(art.height)},point)) return i;
        }
        return -1;
    };
    const auto& thumb = skillFrames_.at(1672);
    const auto scroll = [&](int kind) {
        return thumb.texture && skills.scroll_input(kind,int(point.x)-x,int(point.y)-y,
            thumb.width,thumb.height,static_cast<std::uint32_t>(GetTickCount64()));
    };
    if (down && inside && !blocked && !scroll(0)) {
        skills.pressed = buttonAt();
        if (skills.pressed<0)
            for (int i=0;i<static_cast<int>(positions.size());++i) {
                const auto& position = positions[i];
                if (position.id && Contains({float(x+position.x),float(y+position.y-skills.scroll),40,40},point))
                    skills.selected[skills.tab] = i;
            }
        skills.begin_drag(int(point.x),int(point.y),skills.pressed>=0);
    }
    if ((point.x!=prior.x || point.y!=prior.y) && !scroll(1)) skills.move_drag(int(point.x),int(point.y));
    if (up && !scroll(2)) {
        const int button = blocked ? -1 : buttonAt();
        skills.end_drag();
        if (skills.release_button(button)) {
            if (client.audioOptions.effects && std::filesystem::exists(clickSound_))
                client.audio.PlayEffect(clickSound_.c_str(),client.audioOptions);
            if (button==0) ToggleSkills(client);
        }
    }
    return (inside && !blocked) || captured;
}

void Dialogs::Assets::DrawSkillsPanel(cui_ctx* ui, const LoginClient& client)
{
    const auto& skills = client.skills;
    if (!skills.visible) return;
    const auto loadIcon = [&](int frame) {
        if (!skillIcons_.contains(frame)) skillIcons_.emplace(frame,UploadFrame(ui,skillIconArchive_,frame,true));
    };
    const auto& group = skills.learned[skills.tab];
    for (int i=0;i<(std::min)(int(group.size()),mir3::SkillPanel::visible_skill_limit);++i) {
        const auto& skill = group[i];
        loadIcon(2*skill.id-2);
        SetBagText(ui,skillLevels_[i],std::to_string(skill.level),15,15,true,mir3::TextFont::skill_level);
        const int slot = mir3::SkillLayout::slot(skills.tab,skill.id);
        if (skills.selected[skills.tab]==(slot<0 ? 15 : slot)) {
            SetBagText(ui,skillSelected_[0],skill.name,80,61,true,mir3::TextFont::normal,true);
            const auto level = skill.level>=3 ? std::string("Level : Max") : "Level : "+std::to_string(skill.level);
            SetBagText(ui,skillSelected_[1],level,80,62,true,mir3::TextFont::normal,true);
            SetBagText(ui,skillSelected_[2],skill.level<3 ? "Exp : "+skill.progress_text() : "",80,62,true,mir3::TextFont::normal,true);
        }
    }
    if (skillMenu_.w>0)
        for (int id : skills.bindings) if (id) loadIcon(2*id-2);
    if (skillDescriptionId_ && skillMenu_.w==0) {
        const int id = skillDescriptionId_;
        auto [entry,created] = skillDescriptionTextures_.try_emplace(id);
        if (created)
            for (auto& row : mir3::wrap_skill_description(skillDescriptions_.section(id))) {
                const auto font = row.title ? mir3::TextFont::skill_title : mir3::TextFont::normal;
                auto image = Raster(ui,row.text,300,24,false,font);
                entry->second.push_back({std::move(row),image});
            }
        const auto* skill = skills.find(id);
        const std::string progress = skill ? skill->level>=3 ? "Level : Max" : skill->progress_text() : "";
        SetBagText(ui,skillDescriptionProgress_,progress,skill && skill->level>=3 ? 80 : 50,15,true);
    }
    const auto view = cui_viewport_size(ui);
    cui_overlay_begin(ui,{0,0,view.x,view.y});
    const struct { Assets* assets; const LoginClient* client; } data{this,&client};
    cui_canvas_paint(ui,CUI_FILL,CUI_FILL,[](cui_ctx* ui,cui_rect rect,void* value) {
        const auto& draw = *static_cast<decltype(data)*>(value);
        draw.assets->PaintSkills(ui,rect,*draw.client);
    },&data,sizeof data);
    cui_box_end(ui);
}

void Dialogs::Assets::PaintSkills(cui_ctx* ui, cui_rect rect, const LoginClient& client) const
{
    const HudPoint view(rect);
    const auto& skills = client.skills;
    const auto [x,y,w,h] = skills.rect;
    constexpr cui_color white{255,255,255,255}, caption{250,250,250,255};
    const auto image = [&](const Image& art, int left, int top, cui_color color = {255,255,255,255}) {
        if (art.texture) cui_draw_image(ui,art.texture,view.Rect(left,top,art.width,art.height),{0,0,1,1},{},color);
    };
    const auto sprite = [&](int id,int left,int top) { image(skillFrames_.at(id),left,top); };
    const auto& skin = skillFrames_.at(1620);
    image(skin,x-(skin.width-w)/2,y-(skin.height-h)/2);
    const auto& tree = skillFrames_.at(skills.tree_frame());
    if (tree.texture) {
        constexpr auto origin = mir3::SkillPanel::tree_origin;
        constexpr int height = mir3::SkillPanel::tree_height;
        cui_draw_image(ui,tree.texture,view.Rect(x+origin[0],y+origin[1],tree.width,height),
            {0,float(skills.scroll)/tree.height,1,float(height)/tree.height},{},white);
    }
    sprite(1672,x+322,y+skills.thumb_y());
    const auto& group = skills.learned[skills.tab];
    for (int i=0;i<(std::min)(int(group.size()),mir3::SkillPanel::visible_skill_limit);++i) {
        const auto& skill = group[i];
        const int slot = mir3::SkillLayout::slot(skills.tab,skill.id);
        const auto pos = slot>=0 ? mir3::SkillLayout::positions[skills.tab][slot] :
            mir3::SkillPosition{skill.id,i<6 ? 55 : 0,i<6 ? 27+46*i : 0};
        const int top = pos.y-(slot>=0 ? skills.scroll : 0);
        if (skills.selected[skills.tab]==(slot<0 ? 15 : slot)) {
            if (top-1>62 && top-1<352) {
                const int left=x+pos.x-1, upper=y+top-1;
                constexpr cui_color edge{200,200,0,255};
                cui_draw_rect(ui,view.Rect(left,upper,42,1),{},edge);
                cui_draw_rect(ui,view.Rect(left,upper+41,42,1),{},edge);
                cui_draw_rect(ui,view.Rect(left,upper,1,42),{},edge);
                cui_draw_rect(ui,view.Rect(left+41,upper,1,42),{},edge);
            }
            image(skillSelected_[0].image,x+20,y+387,caption);
            image(skillSelected_[1].image,x+20,y+403,caption);
            image(skillSelected_[2].image,x+100,y+403,caption);
        }
        const auto crop = [&](const Image& art) {
            const int clippedTop = (std::max)(0,64-top);
            const int height = art.height-clippedTop-(std::max)(0,top-355);
            if (art.texture && height>0)
                cui_draw_image(ui,art.texture,view.Rect(x+pos.x,y+top+clippedTop,art.width,height),
                    {0,float(clippedTop)/art.height,1,float(height)/art.height},{},white);
        };
        crop(skillIcons_.at(2*skill.id-2));
        if (skill.key) crop(skillFrames_.at(1659+skill.key));
        if (top+37>56 && top+37<385) image(skillLevels_[i].image,x+pos.x+34,y+top+37,{250,250,100,255});
    }
    for (int index : {0,8,7,6,5,4,3,2,1}) {
        const auto [id,left,top] = mir3::SkillPanel::button(index);
        if (index==0 || skills.pressed==index || !skills.learned[index-1].empty())
            sprite(id+(index==0 && skills.pressed==0),x+left,y+top);
        if (index>0 && skills.tab==index-1) sprite(id+1,x+left,y+top);
    }
    if (skillMenu_.w>0) {
        const int left=int(skillMenu_.x), top=int(skillMenu_.y);
        cui_draw_rect_premultiplied(ui,view.Rect(left,top,160,120),{100,100,50,150});
        for (int i=0;i<12;++i) {
            const int u=left+40*(i%4), v=top+40*(i/4);
            if (const int id=skills.bindings[i+1]) image(skillIcons_.at(2*id-2),u,v);
            sprite(1660+i,u,v);
        }
    } else if (skillDescriptionId_) {
        const auto& rows = skillDescriptionTextures_.at(skillDescriptionId_);
        if (rows.empty()) return;
        int width=0;
        for (const auto& item : rows) if (item.row.width>width) width=item.row.width+7;
        const int left=skillPointer_.x+190>800 ? 630 : int(skillPointer_.x)+20;
        int top=int(skillPointer_.y)+20;
        cui_draw_rect_premultiplied(ui,view.Rect(left-2,top,width,int(rows.size())*15+30),{100,100,50,150});
        for (const auto& item : rows) {
            top+=15;
            if (item.row.title)
                for (const auto offset : mir3::HudLayout::coordinate_shadows)
                    image(item.image,left+offset[0],top+offset[1],{10,10,10,255});
            image(item.image,left,top,item.row.title ? cui_color{250,200,150,255} : caption);
            if (item.row.title) top+=15;
        }
        image(skillDescriptionProgress_.image,left,top,{250,20,20,255});
    }
}

void Dialogs::Assets::DrawSkillShortcuts(cui_ctx* ui, const LoginClient& client)
{
    if (!client.skillShortcutsVisible) return;
    for (int id : client.skills.bindings)
        if (id && !skillIcons_.contains(999+id)) skillIcons_.emplace(999+id,UploadFrame(ui,skillIconArchive_,999+id,true));
    const auto view = cui_viewport_size(ui);
    cui_overlay_begin(ui,{0,0,view.x,view.y});
    const struct { Assets* assets; const LoginClient* client; } data{this,&client};
    cui_canvas_paint(ui,CUI_FILL,CUI_FILL,[](cui_ctx* ui,cui_rect rect,void* value) {
        const auto& draw = *static_cast<decltype(data)*>(value);
        draw.assets->PaintSkillShortcuts(ui,rect,*draw.client);
    },&data,sizeof data);
    cui_box_end(ui);
}

void Dialogs::Assets::PaintSkillShortcuts(cui_ctx* ui, cui_rect rect, const LoginClient& client) const
{
    const HudPoint view(rect);
    const auto image = [&](const Image& art,int x,int y,cui_color color) {
        if (art.texture) cui_draw_image(ui,art.texture,view.Rect(x,y,art.width,art.height),{0,0,1,1},{},color);
    };
    for (int i=0;i<12;++i) {
        const int x=40*(i+i/4);
        if (const int id=client.skills.bindings[i+1]) image(skillIcons_.at(999+id),x,2,{255,255,255,200});
        image(skillFrames_.at(1450+i),x-13,-11,{150,150,150,150});
    }
}

bool Dialogs::Assets::UtilityVisible(UtilityPanel panel, const LoginClient& client) const
{
    switch (panel) {
    case UtilityPanel::Party: return PartyReady() && client.party.open;
    case UtilityPanel::Guild: return GuildReady() && client.guild.open;
    case UtilityPanel::Quest: return QuestReady() && client.quests.open;
    case UtilityPanel::Skills: return SkillsReady() && client.skills.visible;
    case UtilityPanel::Status: return StatusReady() && client.equipment.open;
    case UtilityPanel::Inventory: return InventoryReady() && client.inventory.open;
    case UtilityPanel::Options: return OptionsReady() && client.serverOptions.open;
    case UtilityPanel::Portrait: return portraitPickerVisible_;
    case UtilityPanel::Mount: return mountSkin_.texture && client.mount.visible;
    case UtilityPanel::Trade: return tradeSkin_.texture && client.trade.visible;
    case UtilityPanel::Quick: return QuickReady() && client.inventory.shortcutsVisible;
    }
    return false;
}

void Dialogs::Assets::OpenPortraitPicker(cui_ctx* ui, mir3::WilArchive& archive, const std::filesystem::path& root)
{
    portraitDirectory_ = root;
    for (int id : {1221,1222,1966,2090,2092,2093,2094,2095,2096,2097})
        portraitPickerFrames_[id] = UploadFrame(ui,archive,id,true);
    constexpr std::array messages{1052,4700,4701,4715};
    for (std::size_t i=0;i<messages.size();++i) portraitHints_[i] = MakeHudHint(ui,messages[i]);
    for (int i=0;i<mir3::PortraitPicker::visible_rows;++i)
        portraitInstructions_[i] = Raster(ui,Message(mir3::PortraitPicker::instruction_message(i)),
            260,15,false,mir3::TextFont::normal);
}

void Dialogs::Assets::TogglePortraitPicker(cui_ctx* ui)
{
    portraitPicker_.close();
    Destroy(ui,portraitPreview_);
    portraitPressed_ = portraitHovered_ = -1;
    portraitHintHeight_ = 0;
    portraitPickerVisible_ = !portraitPickerVisible_;
    if (!portraitPickerVisible_) return;
    RaiseUtility(UtilityPanel::Portrait);
    std::error_code error;
    for (std::filesystem::directory_iterator it(portraitDirectory_,error),end;!error && it!=end;it.increment(error)) {
        std::error_code ec;
        if (!it->is_regular_file(ec) || it->is_symlink(ec)) continue;
        const auto extension = it->path().extension().wstring();
        if (_wcsicmp(extension.c_str(),L".jpg")!=0) continue;
        const auto size = it->file_size(ec);
        if (ec || !mir3::PortraitPicker::accepts_size(size)) continue;
        const auto name = it->path().filename().u8string();
        portraitPicker_.names.emplace_back(reinterpret_cast<const char*>(name.data()),name.size());
    }
}

void Dialogs::Assets::LeavePortraitPicker(cui_ctx* ui)
{
    if (portraitPickerVisible_) TogglePortraitPicker(ui);
    portraitCachePath_.clear();
    Destroy(ui,portraitEquipped_);
    Destroy(ui,targetCustomPortrait_);
    targetPortraitBytes_.clear();
    targetPortrait_ = {};
    targetActor_ = 0;
    lastPortraitActor_ = 0;
    portraitPicker_.last_upload = 0;
    portraitNotice_ = 0;
    portraitEntryVisible_ = portraitEntryPressed_ = false;
}

Dialogs::Assets::Image Dialogs::Assets::DecodePortrait(cui_ctx* ui, std::string_view bytes)
{
    if (bytes.empty() || bytes.size()>4096) return {};
    Image image;
    image.texture = cui_image_load_jpeg(ui,bytes.data(),static_cast<int>(bytes.size()),4096,&image.width,&image.height);
    if (image.texture) textures_.push_back(image.texture);
    return image;
}

std::uint32_t Dialogs::Assets::SceneTargetAt(const SceneMapArt& scene,cui_vec2 pointer)
{
    if (!scene.targets) return 0;
    const auto& targets=*scene.targets;
    const auto hit=std::find_if(targets.rbegin(),targets.rend(),[&](const SceneActorHit& actor) { return Contains(actor.bounds,pointer); });
    return hit!=targets.rend() ? hit->actor : 0;
}

void Dialogs::Assets::RefreshTargetPortrait(cui_ctx* ui, LoginClient& client, const SceneMapArt& scene,
                                           cui_vec2 pointer, bool interactive)
{
    const auto now = GetTickCount64();
    const auto hovered=interactive ? SceneTargetAt(scene,pointer) : 0;
    const bool focusChanged=hovered && lastPortraitActor_!=hovered;
    if (hovered) {
        if (targetActor_!=hovered) targetChangedAt_ = now;
        targetActor_ = lastPortraitActor_ = hovered;
    } else if (now-targetChangedAt_>800) {
        targetActor_ = 0;
    }
    auto target = client.scene.players.find(targetActor_);
    targetMonster_ = targetInformation_ = false;
    targetBadge_ = -1;
    if (!scene.targets || target==client.scene.players.end() || target->second.dead || target->second.IsNpc()) {
        targetActor_ = 0;
        if (!scene.targets || !client.scene.players.contains(lastPortraitActor_)) lastPortraitActor_=0;
        targetPortrait_ = {};
        targetPortraitBytes_.clear();
        Destroy(ui,targetCustomPortrait_);
        return;
    }
    auto& actor = target->second;
    if (!hovered) cui_request_wakeup(ui,float(801-(now-targetChangedAt_))*0.001f);
    if (actor.IsMonster()) {
        if (focusChanged) client.RequestMonsterAttributes(targetActor_,actor);
        RefreshMonsterPortrait(ui,actor);
        return;
    }
    if (hovered) client.RequestTargetPortrait(actor);
    const auto& appearance = actor.appearance;
    const int frame = mir3::TargetPortrait::human_frame(appearance.profession,appearance.body);
    if (const auto it=gamePortraits_.find(frame);it!=gamePortraits_.end()) targetPortrait_ = it->second;
    else targetPortrait_ = {};
    const std::string_view jpeg = client.userPhotos ? std::string_view(actor.portraitJpeg) : std::string_view{};
    if (targetPortraitBytes_!=jpeg) {
        Destroy(ui,targetCustomPortrait_);
        targetPortraitBytes_ = jpeg;
        targetCustomPortrait_ = DecodePortrait(ui,jpeg);
        if (!jpeg.empty() && !targetCustomPortrait_.texture) {
            targetPortraitBytes_.clear();
            actor.portraitJpeg.clear();
            actor.resolvedPortraitVersion = 0;
        }
    }
}

void Dialogs::Assets::RefreshMonsterPortrait(cui_ctx* ui, const LocalScene::Player& actor)
{
    targetPortraitBytes_.clear();
    Destroy(ui,targetCustomPortrait_);
    targetPortrait_={};
    const auto shape=static_cast<std::uint16_t>(actor.npcShape);
    const int frame=mir3::TargetPortrait::monster_frame(3,shape,false);
    if (frame<0) return;
    const auto portrait=[&](int id) {
        auto [entry,added]=monsterPortraits_.try_emplace(id);
        if (added) entry->second=UploadFrame(ui,monsterPortraitArchive_,id,true);
        return entry->second;
    };
    targetPortrait_=portrait(frame);
    if (!targetPortrait_.texture) targetPortrait_=portrait(mir3::TargetPortrait::fallback_frame(frame,false));
    targetMonster_=targetPortrait_.texture!=0;
    targetInformation_=!mir3::TargetPortrait::special_shape(shape);
    targetBadge_=mir3::TargetPortrait::badge(shape,actor.monsterAttributes.state);
    targetHealthWidth_=actor.monsterVitals.bar_width();
    targetAttributes_=actor.monsterAttributes;
    for (int i=0;i<3;++i) {
        const auto rect=mir3::TargetPortraitVitals::text_rect(i);
        SetBagText(ui,targetVitalsText_[i],actor.monsterVitals.text(i),rect[2],rect[3],true);
    }
    for (std::size_t i=0;i<targetModifierText_.size();++i) {
        const int value=targetAttributes_.modifiers[i];
        SetBagText(ui,targetModifierText_[i],value ? (value>0 ? " +" : " ")+std::to_string(value)+"%" : "",40,18,false);
    }
}

void Dialogs::Assets::DrawTargetDetails(cui_ctx* ui, bool attributes)
{
    if (gameHud_.layout.expanded || !targetActor_ || (attributes ? !targetInformation_ : !targetMonster_)) return;
    const auto size=cui_viewport_size(ui);
    cui_overlay_begin(ui,{0,0,size.x,size.y});
    const struct { const Assets* assets; bool attributes; } draw{this,attributes};
    cui_canvas_paint(ui,CUI_FILL,CUI_FILL,[](cui_ctx* ui,cui_rect rect,void* value) {
        const auto& data=*static_cast<decltype(draw)*>(value);
        data.assets->PaintTargetDetails(ui,HudPoint(rect),data.attributes);
    },&draw,sizeof draw);
    cui_box_end(ui);
}

void Dialogs::Assets::PaintTargetDetails(cui_ctx* ui, const HudPoint& base, bool attributes) const
{
    using Layout=mir3::HudLayout;
    const auto image=[&](const Image& art,int x,int y,cui_color tint) {
        if (art.texture) cui_draw_image(ui,art.texture,base.Rect(x,y,art.width,art.height),{0,0,1,1},{},tint);
    };
    const auto icon=[&](int id,int x,int y) {
        if (const auto it=gamePortraits_.find(id);it!=gamePortraits_.end()) image(it->second,x,y,{255,255,255,255});
    };
    constexpr cui_color ink{230,230,230,255};
    if (!attributes) {
        for (int i=0;i<3;++i) {
            const auto rect=mir3::TargetPortraitVitals::text_rect(i);
            image(targetVitalsText_[i].image,rect[0],rect[1],ink);
        }
        const auto edge=Layout::image_position(Layout::Image::target_hp_border);
        const auto fill=Layout::image_position(Layout::Image::target_hp_fill);
        icon(660,edge[0],edge[1]);
        const auto bar=gamePortraits_.find(661);
        if (bar!=gamePortraits_.end() && bar->second.texture && targetHealthWidth_>0) {
            const auto& art=bar->second;
            const int width=(std::min)(targetHealthWidth_,art.width);
            cui_draw_image(ui,art.texture,base.Rect(fill[0],fill[1],width,art.height),
                {0,0,float(width)/art.width,1},{},{255,255,255,255});
        }
        return;
    }
    cui_draw_rect_premultiplied(ui,base.Rect(615,487,105,85),{50,50,50,230});
    constexpr cui_color outline{120,120,120,255};
    for (const auto& edge : std::array{base.Rect(615,487,105,1),base.Rect(615,571,105,1),
        base.Rect(615,488,1,83),base.Rect(719,488,1,83)}) cui_draw_rect(ui,edge,{},outline);
    if (!targetAttributes_.visible()) return;
    icon(targetAttributes_.category==0 ? 632 : 631,620,490);
    if (targetAttributes_.element>=1 && targetAttributes_.element<=7) icon(599+targetAttributes_.element,641,490);
    if (targetAttributes_.icon>=0 && targetAttributes_.icon<=9)
        icon(620+targetAttributes_.icon,targetAttributes_.element ? 662 : 641,490);
    int slot=0;
    for (const int index:targetAttributes_.order()) {
        if (!targetAttributes_.modifiers[index]) break;
        const int x=620+32*(slot%3), y=510+20*(slot/3);
        icon(610+index,x,y);
        image(targetModifierText_[index].image,x+15,y+5,ink);
        ++slot;
    }
}

void Dialogs::Assets::RefreshPortraitCache(cui_ctx* ui, const LoginClient& client)
{
    const auto path = client.PortraitCachePath();
    if (path==portraitCachePath_) return;
    portraitCachePath_ = path;
    Destroy(ui,portraitEquipped_);
    if (path.empty()) return;
    std::ifstream file(path,std::ios::binary);
    std::array<char,mir3::PortraitFac::max_payload+9> data{};
    if (!file) return;
    file.read(data.data(),data.size());
    if (!file.eof() || file.bad()) return;
    if (const auto jpeg = mir3::PortraitFac::decode({data.data(),static_cast<std::size_t>(file.gcount())}))
        portraitEquipped_ = DecodePortrait(ui,*jpeg);
}

int Dialogs::Assets::ReadSelectedPortrait(std::string& bytes) const
{
    const int selected = portraitPicker_.selected;
    if (selected<0 || selected>=static_cast<int>(portraitPicker_.names.size())) return 4716;
    const auto name = std::filesystem::u8path(portraitPicker_.names[selected]);
    if (name.filename()!=name || name.has_root_path() || name.native().find(L':')!=std::wstring::npos ||
        _wcsicmp(name.extension().c_str(),L".jpg")!=0) return 4716;
    const auto path = portraitDirectory_/name;
    std::error_code ec;
    if (!std::filesystem::is_regular_file(path,ec) || std::filesystem::is_symlink(path,ec)) return 4716;
    std::ifstream file(path,std::ios::binary);
    if (!file) return 4716;
    std::array<char,mir3::PortraitFac::max_payload+1> data{};
    file.read(data.data(),data.size());
    if (file.bad()) return 4716;
    if (!file.eof()) return 4714;
    bytes.assign(data.data(),static_cast<std::size_t>(file.gcount()));
    return 0;
}

void Dialogs::Assets::ConfirmPortrait(cui_ctx* ui, LoginClient& client, int button, std::uint32_t tick)
{
    std::string bytes;
    Image next;
    if (button==1) {
        if (portraitPicker_.selected<0) return;
        portraitNotice_ = ReadSelectedPortrait(bytes);
        if (portraitNotice_) return;
        next = DecodePortrait(ui,bytes);
        if (!next.texture) { portraitNotice_ = 4716; return; }
        if (!portraitPicker_.accept_upload_tick(tick)) { Destroy(ui,next); return; }
    }
    if (!client.SendPortrait(bytes)) { Destroy(ui,next); return; }
    if (!portraitCachePath_.empty()) {
        std::error_code ec;
        if (bytes.empty()) {
            std::filesystem::remove(portraitCachePath_,ec);
            if (ec) portraitNotice_ = 4716;
        } else {
            const auto encoded = mir3::PortraitFac::encode(bytes);
            auto temporary = portraitCachePath_;
            temporary += L"."+std::to_wstring(GetCurrentProcessId())+L".tmp";
            std::filesystem::create_directories(portraitCachePath_.parent_path(),ec);
            std::ofstream file(temporary,std::ios::binary|std::ios::trunc);
            if (encoded) file.write(encoded->data(),encoded->size());
            file.close();
            if (!encoded || !file || !MoveFileExW(temporary.c_str(),portraitCachePath_.c_str(),MOVEFILE_REPLACE_EXISTING)) {
                DeleteFileW(temporary.c_str());
                Destroy(ui,next);
                portraitNotice_ = 4716;
                return;
            }
        }
    }
    Destroy(ui,portraitEquipped_);
    portraitEquipped_ = next;
}

bool Dialogs::Assets::HandlePortraitEntry(cui_ctx* ui, LoginClient& client, cui_vec2 pointer, bool blocked)
{
    const auto hover = mir3::HudLayout::portrait_hover;
    const auto [x,y] = mir3::HudLayout::portrait_entry_position;
    const auto& entry = portraitPickerFrames_.at(2096);
    portraitEntryVisible_ = !client.chat.expanded && entry.texture &&
        Contains({float(hover[0]),float(hover[1]),float(hover[2]),float(hover[3])},pointer);
    const bool hit = portraitEntryVisible_ && !blocked && portraitPickerFrames_.at(2090).texture &&
        Contains({float(x),float(y),float(entry.width),float(entry.height)},pointer);
    if (cui_mouse_released(ui,CUI_MOUSE_LEFT) || blocked) portraitEntryPressed_ = false;
    if (hit && cui_mouse_clicked(ui,CUI_MOUSE_LEFT)) portraitEntryPressed_ = true;
    if (hit && cui_mouse_released(ui,CUI_MOUSE_LEFT)) {
        TogglePortraitPicker(ui);
        if (client.audioOptions.effects && std::filesystem::exists(clickSound_))
            client.audio.PlayEffect(clickSound_.c_str(),client.audioOptions);
    }
    portraitEntryHintHeight_ = hit && !portraitEntryPressed_ && portraitHints_[3].texture ?
        portraitHint_.Advance(ui,4715,portraitHints_[3].height) : 0;
    return hit;
}

bool Dialogs::Assets::HandlePortraitPicker(cui_ctx* ui, LoginClient& client, bool blocked)
{
    using Picker = mir3::PortraitPicker;
    const auto view = cui_viewport_size(ui);
    const HudPoint base({0,0,view.x,view.y});
    if (base.scale<=0) return false;
    const auto point = base.ToLocal(cui_mouse_pos(ui));
    const int x = static_cast<int>(std::floor(point.x)), y = static_cast<int>(std::floor(point.y));
    const bool released = cui_mouse_released(ui,CUI_MOUSE_LEFT), pressed = cui_mouse_clicked(ui,CUI_MOUSE_LEFT);
    const bool captured = portraitPicker_.scrolling || portraitPressed_>=0;
    portraitHovered_ = -1;
    portraitHintHeight_ = 0;
    if (blocked) {
        if (released) { portraitPicker_.scrolling = false; portraitPressed_ = -1; }
        return false;
    }
    const auto [left,top,width,height] = Picker::initial;
    const bool inside = Contains({float(left),float(top),float(width),float(height)},point);
    if (!inside && !captured) return false;
    const auto tick = GetTickCount();
    const auto& thumb = portraitPickerFrames_.at(Picker::scroll[0]);
    const int kind = released ? 2 : pressed ? 0 : 1;
    if (portraitPicker_.scroll_input(kind,x,y,thumb.width,thumb.height,tick)) return true;
    for (int i=0;i<static_cast<int>(Picker::buttons.size());++i) {
        const auto [id,bx,by] = Picker::buttons[i];
        const auto& art = portraitPickerFrames_.at(id);
        if (Contains({float(bx),float(by),float(art.width),float(art.height)},point)) portraitHovered_ = i;
    }
    if (pressed) {
        portraitPressed_ = portraitHovered_;
        if (portraitHovered_<0 && portraitPicker_.select(x,y)) {
            Destroy(ui,portraitPreview_);
            std::string bytes;
            if (!ReadSelectedPortrait(bytes)) portraitPreview_ = DecodePortrait(ui,bytes);
        }
    } else if (released) {
        if (portraitHovered_>=0 && portraitHovered_==portraitPressed_) {
            if (portraitHovered_==0) TogglePortraitPicker(ui);
            else ConfirmPortrait(ui,client,portraitHovered_,tick);
            if (client.audioOptions.effects && std::filesystem::exists(clickSound_))
                client.audio.PlayEffect(clickSound_.c_str(),client.audioOptions);
        }
        portraitPressed_ = -1;
    } else {
        portraitPicker_.move(x,y);
    }
    if (portraitHovered_>=0 && portraitHovered_!=portraitPressed_ && portraitHints_[portraitHovered_].texture) {
        constexpr std::array messages{1052,4700,4701};
        portraitHintHeight_ = portraitHint_.Advance(ui,messages[portraitHovered_],portraitHints_[portraitHovered_].height);
    }
    return inside || captured;
}

void Dialogs::Assets::DrawPortraitPicker(cui_ctx* ui)
{
    using Picker = mir3::PortraitPicker;
    for (int i=0;i<Picker::visible_rows;++i) {
        const auto [x,y,w,h] = Picker::row(i);
        const auto index = static_cast<std::size_t>(portraitPicker_.first+i);
        SetBagText(ui,portraitFileLabels_[i],index<portraitPicker_.names.size() ? portraitPicker_.names[index] : "",w,h);
    }
    const auto view = cui_viewport_size(ui);
    cui_overlay_begin(ui,{0,0,view.x,view.y});
    const auto* self = this;
    cui_canvas_paint(ui,CUI_FILL,CUI_FILL,[](cui_ctx* ui,cui_rect r,void* user) {
        (*static_cast<const Assets**>(user))->PaintPortraitPicker(ui,r);
    },&self,sizeof self);
    cui_box_end(ui);
}

void Dialogs::Assets::PaintPortraitPicker(cui_ctx* ui, cui_rect rect) const
{
    using Picker = mir3::PortraitPicker;
    const HudPoint base(rect);
    const auto image = [&](const Image& art,int x,int y,cui_color color = {255,255,255,255}) {
        if (art.texture) cui_draw_image(ui,art.texture,base.Rect(x,y,art.width,art.height),{0,0,1,1},{},color);
    };
    const auto sprite = [&](int id,int x,int y) { image(portraitPickerFrames_.at(id),x,y); };
    const auto& skin = portraitPickerFrames_.at(Picker::frame);
    image(skin,-(skin.width-Picker::initial[2])/2,-(skin.height-Picker::initial[3])/2);
    constexpr cui_color text{250,250,250,255};
    for (int i=0;i<Picker::visible_rows;++i) {
        const auto [x,y,w,h] = Picker::row(i);
        if (i==portraitPicker_.hovered) cui_draw_rect_premultiplied(ui,base.Rect(x,y,w,h),{30,30,30,180});
        if (i==portraitPicker_.selected_row()) cui_draw_rect_premultiplied(ui,base.Rect(x,y,w,h),{35,114,156,180});
        image(portraitFileLabels_[i].image,x,y+5,text);
        const auto [ix,iy] = Picker::instruction(i);
        image(portraitInstructions_[i],ix,iy,text);
    }
    const auto [x,y,w,h] = Picker::preview;
    if (portraitPreview_.texture)
        cui_draw_image(ui,portraitPreview_.texture,base.Rect(x,y,w,h),{0,0,1,1},{},{255,255,255,255});
    sprite(Picker::scroll[0],Picker::scroll[1],portraitPicker_.thumb_y());
    for (int i=0;i<static_cast<int>(Picker::buttons.size());++i) {
        const auto [id,bx,by] = Picker::buttons[i];
        const int state = portraitPressed_==i ? 2 : portraitHovered_==i ? 1 : 0;
        for (const int layer:mir3::button_frames({id,id+(i==0),i==0 ? id : -1,i==0},state))
            if (layer>=0) sprite(layer,bx,by);
        if (i==portraitHovered_ && portraitHintHeight_>0) {
            const auto& art = portraitPickerFrames_.at(id);
            HudHint::Paint(ui,base,portraitHints_[i],portraitHintHeight_,bx+art.width,by+art.height);
        }
    }
}

void Dialogs::Assets::DrawPortraitNotice(cui_ctx* ui, LoginClient& client)
{
    const auto view = cui_viewport_size(ui);
    const HudPoint base({0,0,view.x,view.y});
    if (base.scale<=0) return;
    const int x = 400-dialog_[0].width/2, y = 246-dialog_[0].height/2;
    const bool inside = Contains(base.Rect(x+137,y+188,dialog_[1].width,dialog_[1].height),cui_mouse_pos(ui));
    if (!inside) portraitNoticePressed_ = false;
    if (cui_mouse_clicked(ui,CUI_MOUSE_LEFT)) portraitNoticePressed_ = inside;
    const bool clicked = cui_mouse_released(ui,CUI_MOUSE_LEFT) && std::exchange(portraitNoticePressed_,false);
    if (clicked || cui_key_pressed(ui,CUI_KEY_ENTER) || cui_key_pressed(ui,CUI_KEY_SPACE)) {
        if (clicked && client.audioOptions.effects && std::filesystem::exists(clickSound_))
            client.audio.PlayEffect(clickSound_.c_str(),client.audioOptions);
        portraitNotice_ = 0;
        return;
    }
    SetBagText(ui,portraitNoticeText_,Message(portraitNotice_),308,95,true,mir3::TextFont::dialog);
    cui_overlay_begin(ui,{0,0,view.x,view.y});
    const struct { const Assets* self; int x,y; } data{this,x,y};
    cui_canvas_paint(ui,CUI_FILL,CUI_FILL,[](cui_ctx* ui,cui_rect rect,void* user) {
        const auto& draw = *static_cast<decltype(data)*>(user);
        const HudPoint base(rect);
        const auto image = [&](const Image& art,int x,int y,cui_color color = {255,255,255,255}) {
            if (art.texture) cui_draw_image(ui,art.texture,base.Rect(draw.x+x,draw.y+y,art.width,art.height),{0,0,1,1},{},color);
        };
        image(draw.self->dialog_[0],0,0);
        image(draw.self->portraitNoticeText_.image,23,84,{255,250,200,255});
        image(draw.self->dialog_[draw.self->portraitNoticePressed_ ? 2 : 1],137,188);
    },&data,sizeof data);
    cui_box_end(ui);
}

namespace {

using StatusRect = std::array<int,4>;
constexpr std::array<StatusRect,mir3_cui::EquipmentSlots> statusSlots{{
    {95,114,50,90},{40,70,50,95},{90,265,38,38},{170,88,38,38},
    {95,70,38,34},{12,155,38,38},{170,155,38,38},{12,195,38,38},
    {170,195,38,38},{130,265,38,38},{12,255,38,38},{170,265,38,38}
}};
constexpr std::array<StatusRect,5> statusCombatLabels{{
    {11,320,58,15},{13,349,58,15},{118,349,60,15},{118,320,60,15},{218,349,60,15}
}};
constexpr std::array<StatusRect,5> statusCombatValues{{
    {58,320,58,15},{60,349,58,15},{163,349,60,15},{163,320,60,15},{270,349,60,15}
}};
constexpr std::array<int,3> statusElementY{379,408,439};
constexpr std::array<int,2> statusCloseAnchor{289,14}, statusBodyAnchor{82,199};
constexpr cui_color statusTextColor{250,250,250,255}, statusWhite{255,255,255,255};

constexpr StatusRect StatusValue(int row, int part = 0)
{
    if (row<2 || row>=7) return {245,91+24*row,67,15};
    if (part==2) return {276,92+24*row,6,15};
    return {part==0 ? 245 : 282,91+24*row,30,15};
}

bool StatusContains(const StatusRect& rect, cui_vec2 point)
{
    return point.x>=rect[0] && point.y>=rect[1] && point.x<rect[0]+rect[2] && point.y<rect[1]+rect[3];
}

int StatusSlot(cui_vec2 point)
{
    for (int i=0;i<mir3_cui::EquipmentSlots;++i)
        if (StatusContains(statusSlots[i],point)) return i;
    return -1;
}

int StatusPreview(unsigned mode, int slot)
{
    if (!mir3_cui::EquipmentFits(mode,slot) || (mode==25 && slot!=9)) return -1;
    if (slot==10) return 1;
    return slot==0 || slot==1 || slot==4 ? -1 : 0;
}

}

void Dialogs::Assets::OpenStatus(cui_ctx* ui, mir3::WilArchive& archive, const std::filesystem::path& data)
{
    statusSkin_ = UploadFrame(ui,archive,1280,true);
    if (!StatusReady()) return;
    for (int i=0;i<2;++i) {
        statusClose_[i] = UploadFrame(ui,archive,1221+i,true);
        for (int column=0;column<7;++column)
            statusElements_[i][column] = UploadFrame(ui,archive,1510+i*10+column,true);
        if (auto glow=archive.frame(1235+i)) {
            auto& pixels = glow->pixels.rgba;
            statusGlows_[i] = Upload(ui,glow->width,glow->height,pixels,true);
            for (std::size_t at=0;at<pixels.size();at+=4) {
                pixels[at] = pixels[at+1] = pixels[at+2] = 0;
                pixels[at+3] = pixels[at+3]*20/255;
            }
            statusGlowShadows_[i] = Upload(ui,glow->width,glow->height,pixels,true);
        }
    }
    statusCloseHint_ = MakeHudHint(ui,1052);
    mir3::WilArchive portraits;
    if (portraits.open(data / "ProgUse")) {
        for (int i=0;i<2;++i) {
            const auto body = portraits.frame(i);
            if (body) statusBodies_[i] = {Upload(ui,body->width,body->height,body->pixels.rgba,true),
                body->offset_x,body->offset_y};
        }
    }
    statusEquipArchive_.open(data / "Equip");
    constexpr std::array<int,9> tips{2400,2401,-1,-1,2404,2402,2403,1053,1054};
    for (int row=0;row<9;++row) {
        const auto caption = row==2 ? "HP" : row==3 ? "MP" : Message(tips[row]);
        const auto measured = mir3::raster_text(caption,800,24,false,mir3::TextFont::normal,font_);
        auto& tip = statusStatTips_[row];
        tip.rect = {200,69+24*row,measured.text_width,18};
        tip.color = statusWhite;
        SetBagText(ui,tip.text,caption,tip.rect[2],tip.rect[3],true);
    }
    for (int column=0;column<7;++column) {
        auto& tip = statusElementTips_[column];
        tip.color = statusWhite;
        SetBagText(ui,tip.text,Message(5161+column),61,19,true);
    }
}

void Dialogs::Assets::ToggleStatus(LoginClient& client)
{
    if (client.equipment.open) client.equipment.open = false;
    else client.OpenEquipment();
    statusPanel_.visible = client.equipment.open;
    statusPanel_.end_drag();
    statusClosePressed_ = false;
    if (statusPanel_.visible && StatusReady()) RaiseUtility(UtilityPanel::Status);
}

bool Dialogs::Assets::HandleStatusInput(cui_ctx* ui, LoginClient& client, bool blocked)
{
    statusPointer_ = {-1,-1};
    statusHintHeight_ = 0;
    const auto view = cui_viewport_size(ui);
    const HudPoint base({0,0,view.x,view.y});
    if (base.scale<=0) return false;
    const auto point = base.ToLocal(cui_mouse_pos(ui));
    const auto [x,y,w,h] = statusPanel_.rect;
    const cui_vec2 local{point.x-x,point.y-y};
    const bool inside = !blocked && StatusContains(statusPanel_.rect,point);
    const bool captured = statusPanel_.dragging || statusClosePressed_;
    const auto [left,top] = statusCloseAnchor;
    const bool close = inside && StatusContains({left,top,statusClose_[0].width,statusClose_[0].height},local);
    const int slot = inside ? StatusSlot(local) : -1;
    if (inside) {
        if (!statusPanel_.dragging) statusPointer_ = local;
    }
    if (inside && cui_mouse_clicked(ui,CUI_MOUSE_LEFT)) {
        statusClosePressed_ = close;
        statusPanel_.begin_drag(int(point.x),int(point.y),close || slot>=0);
    } else if (cui_mouse_released(ui,CUI_MOUSE_LEFT)) {
        statusPanel_.end_drag();
        if (std::exchange(statusClosePressed_,false) && close) {
            if (client.audioOptions.effects && std::filesystem::exists(clickSound_))
                client.audio.PlayEffect(clickSound_.c_str(),client.audioOptions);
            ToggleStatus(client);
        } else if (!captured && slot>=0 && !client.TradeItemLocked()) {
            const auto* held = client.HeldItem();
            if (!held || held->key.id) client.ReleaseEquipmentSlot(slot);
        }
    } else if (!blocked && statusPanel_.dragging) {
        statusPanel_.move_drag(int(point.x),int(point.y));
    }
    if (close && !statusClosePressed_ && !statusPanel_.dragging && statusCloseHint_.texture)
        statusHintHeight_ = statusHint_.Advance(ui,1052,statusCloseHint_.height);
    else
        statusHint_.Advance(ui,-1,0);
    return inside || captured;
}

void Dialogs::Assets::DrawStatusPanel(cui_ctx* ui, LoginClient& client)
{
    const auto& stats = client.characterStatus;
    statusBody_ = statusBodies_[client.characterAppearance.body==1];
    statusPreview_ = -1;
    statusTip_ = nullptr;
    statusEquipped_ = {};
    const auto* held = client.HeldItem();
    for (int slot=0;slot<mir3_cui::EquipmentSlots;++slot) {
        const auto& item = client.equipment.slots[slot];
        if (item.key.name.empty() || &item==held ||
            (client.equipment.request==mir3_cui::EquipmentRequest::Wear && slot==client.equipment.pendingSlot)) continue;
        if (slot==0 || slot==1 || slot==4) {
            auto [it,inserted] = statusEquipFrames_.try_emplace(item.icon);
            if (inserted) {
                const auto frame = statusEquipArchive_.frame(item.icon);
                if (frame) it->second = {Upload(ui,frame->width,frame->height,frame->pixels.rgba,true),
                    frame->offset_x,frame->offset_y};
            }
            statusEquipped_[slot] = it->second;
            statusEquipped_[slot].x += statusBodyAnchor[0];
            statusEquipped_[slot].y += statusBodyAnchor[1];
        } else {
            CacheItem(ui,item.icon);
            const auto& icon = itemFrames_.at(item.icon);
            statusEquipped_[slot] = {icon,statusSlots[slot][0]+19-icon.width/2,statusSlots[slot][1]+19-icon.height/2};
        }
    }
    const int hoveredSlot = StatusSlot(statusPointer_);
    if (held && hoveredSlot>=0) {
        statusPreview_ = StatusPreview(held->mode,hoveredSlot);
        if (statusPreview_>=0)
            statusPreviewOrigin_ = {statusSlots[hoveredSlot][0]-2,statusSlots[hoveredSlot][1]-(statusPreview_==1 ? 21 : 2)};
    }

    auto label = statusLabels_.begin();
    const auto add = [&](StatusRect rect, std::string value, cui_color tint = statusTextColor,
                         bool centered = true, mir3::TextFont font = mir3::TextFont::normal) -> StatusLabel& {
        label->rect = rect;
        label->color = tint;
        SetBagText(ui,label->text,std::move(value),rect[2],rect[3],centered,font);
        return *label++;
    };
    add({100,30,130,20},client.gameCharacter,{220,255,220,255},true,mir3::TextFont::character_name);
    add(StatusValue(0),std::to_string(stats.level));
    char experience[32]{};
    std::snprintf(experience,sizeof experience,"%.2f%%",stats.ExperiencePercent());
    add(StatusValue(1),experience,{255,0,0,255});
    const std::array vitals{stats.hp,stats.mp,stats.weight,stats.wear,stats.hand};
    for (int i=0;i<5;++i) {
        add(StatusValue(i+2,0),std::to_string(vitals[i][0]));
        add(StatusValue(i+2,1),std::to_string(vitals[i][1]));
        add(StatusValue(i+2,2),"/",statusTextColor,false);
    }
    constexpr std::array combatMessages{1061,5101,5102,1060,1406};
    for (int i=0;i<5;++i) {
        add(statusCombatLabels[i],Message(combatMessages[i]));
        add(statusCombatValues[i],std::to_string(stats.combat[i][0])+"-"+std::to_string(stats.combat[i][1]));
    }
    add(StatusValue(7),"+"+std::to_string(stats.accuracy)+"%");
    add(StatusValue(8),"+"+std::to_string(stats.agility)+"%");
    constexpr std::array<StatusRect,3> elements{{{11,377,50,15},{11,408,50,15},{12,438,50,15}}};
    for (int row=0;row<3;++row) {
        add(elements[row],Message(5178+row));
        for (int column=6;column>=0;--column) {
            const auto amount = stats.DisplayedElement(row,column);
            auto& element = add({81+37*column,statusElementY[row],20,15},
                amount ? (row==0 ? "+" : "x")+std::to_string(amount) : "");
            element.icon = amount ? WorldFrame{statusElements_[row==2][column],63+37*column,375+30*row} : WorldFrame{};
            if (amount && StatusContains({62+37*column,382+30*row,36,19},statusPointer_)) {
                auto& tip = statusElementTips_[column];
                tip.rect = {72+37*column,372+30*row,61,19};
                statusTip_ = &tip;
            }
        }
    }
    for (int row=0;row<9;++row)
        if (StatusContains({210,84+24*row,78,18},statusPointer_)) statusTip_ = &statusStatTips_[row];

    const auto view = cui_viewport_size(ui);
    cui_overlay_begin(ui,{0,0,view.x,view.y});
    Assets* assets = this;
    cui_canvas_paint(ui,CUI_FILL,CUI_FILL,[](cui_ctx* ui,cui_rect rect,void* data) {
        (*static_cast<Assets**>(data))->PaintStatus(ui,rect);
    },&assets,sizeof assets);
    cui_box_end(ui);
}

void Dialogs::Assets::PaintStatus(cui_ctx* ui, cui_rect rect) const
{
    const HudPoint base(rect);
    const auto [x,y,w,h] = statusPanel_.rect;
    const auto image = [&](const Image& art, int left, int top, cui_color color = statusWhite) {
        if (art.texture) cui_draw_image(ui,art.texture,base.Rect(x+left,y+top,art.width,art.height),{0,0,1,1},{},color);
    };
    image(statusSkin_,-(statusSkin_.width-w)/2,-(statusSkin_.height-h)/2);
    if (statusPreview_>=0) {
        const auto [left,top] = statusPreviewOrigin_;
        const auto& glow = statusGlows_[statusPreview_];
        image(statusGlowShadows_[statusPreview_],left,top);
        if (glow.texture) cui_draw_image_additive(ui,glow.texture,base.Rect(x+left,y+top,glow.width,glow.height));
    }
    image(statusBody_.image,statusBodyAnchor[0]+statusBody_.x,statusBodyAnchor[1]+statusBody_.y);
    for (const auto& equipped : statusEquipped_)
        image(equipped.image,equipped.x,equipped.y);
    for (const auto& label : statusLabels_) {
        image(label.icon.image,label.icon.x,label.icon.y);
        image(label.text.image,label.rect[0],label.rect[1],label.color);
    }
    if (statusTip_ && statusTip_->text.image.texture) {
        const auto [left,top,width,height] = statusTip_->rect;
        cui_draw_rect_premultiplied(ui,base.Rect(x+left,y+top,width,height),{80,60,40,150});
        image(statusTip_->text.image,left,top);
        constexpr cui_color border{100,75,50,255};
        cui_draw_rect(ui,base.Rect(x+left,y+top-1,width,1),{},border);
        cui_draw_rect(ui,base.Rect(x+left,y+top+height-1,width,1),{},border);
        cui_draw_rect(ui,base.Rect(x+left-1,y+top,1,height),{},border);
        cui_draw_rect(ui,base.Rect(x+left+width-1,y+top,1,height),{},border);
    }
    const auto [left,top] = statusCloseAnchor;
    image(statusClose_[statusClosePressed_],left,top);
    if (statusHintHeight_>0)
        HudHint::Paint(ui,base,statusCloseHint_,statusHintHeight_,x+left+statusClose_[0].width,y+top+statusClose_[0].height);
}

void Dialogs::Assets::RaiseUtility(UtilityPanel panel)
{
    const auto position = std::find(utilityOrder_.begin(),utilityOrder_.end(),panel);
    std::rotate(position,position+1,utilityOrder_.end());
}

void Dialogs::Assets::ToggleParty(LoginClient& client)
{
    client.party.open = !client.party.open;
    if (client.party.open && PartyReady()) RaiseUtility(UtilityPanel::Party);
    else LeaveParty();
}

void Dialogs::Assets::ToggleOptions(LoginClient& client)
{
    client.serverOptions.open = !client.serverOptions.open;
    LeaveOptions();
    if (client.serverOptions.open && OptionsReady()) RaiseUtility(UtilityPanel::Options);
}

void Dialogs::Assets::OpenMount(cui_ctx* ui, mir3::WilArchive& archive)
{
    mountSkin_ = UploadFrame(ui,archive,mir3::MountPanel::background,true);
    for (std::size_t i=0;i<mir3::MountPanel::buttons.size();++i) {
        const auto [frame,x,y,hint] = mir3::MountPanel::buttons[i];
        mountButtons_[i][0] = UploadFrame(ui,archive,frame,true);
        mountButtons_[i][1] = UploadFrame(ui,archive,frame+1,true);
        mountHints_[i] = MakeHudHint(ui,hint);
    }
}

void Dialogs::Assets::ToggleMount(LoginClient& client)
{
    if (!mountSkin_.texture) return;
    client.mount.toggle();
    client.mount.pressed = -1;
    mountHintButton_ = -1;
    if (client.mount.visible) RaiseUtility(UtilityPanel::Mount);
}

bool Dialogs::Assets::HandleMountInput(cui_ctx* ui, LoginClient& client, bool blocked)
{
    auto& mount = client.mount;
    mountHintButton_ = -1;
    if (!mount.visible) return false;
    const auto size = cui_viewport_size(ui);
    const HudPoint base({0,0,size.x,size.y});
    if (base.scale <= 0) return false;
    const auto pointer = base.ToLocal(cui_mouse_pos(ui));
    const auto [x,y,w,h] = mount.rect;
    const bool captured = mount.dragging || mount.pressed>=0;
    const bool inside = Contains({float(x),float(y),float(w),float(h)},pointer);
    if (blocked && !captured) return false;
    int hit = -1;
    if (inside && !blocked) {
        for (std::size_t i=0;i<mir3::MountPanel::buttons.size();++i) {
            const auto [id,left,top,hint] = mir3::MountPanel::buttons[i];
            const auto& art = mountButtons_[i][0];
            if (Contains({float(x+left),float(y+top),float(art.width),float(art.height)},pointer)) {
                hit = static_cast<int>(i);
                break;
            }
        }
    }
    if (inside && !blocked && cui_mouse_clicked(ui,CUI_MOUSE_LEFT)) {
        mount.pressed = hit;
        mount.begin_drag(int(pointer.x),int(pointer.y),hit>=0);
    } else if (cui_mouse_released(ui,CUI_MOUSE_LEFT)) {
        mount.end_drag();
        const int pressed = std::exchange(mount.pressed,-1);
        if (hit>=0 && hit==pressed) {
            if (client.audioOptions.effects && std::filesystem::exists(clickSound_))
                client.audio.PlayEffect(clickSound_.c_str(),client.audioOptions);
            if (hit==0) ToggleMount(client);
            else if (const int command = mount.command(hit)) client.SendMountCommand(Message(command));
        }
    } else if (captured) {
        mount.move_drag(int(pointer.x),int(pointer.y));
    }
    if (mount.visible && hit>=0 && hit!=mount.pressed && !mount.dragging && mountHints_[hit].texture) {
        mountHintButton_ = hit;
        mountHintHeight_ = mountHint_.Advance(ui,mir3::MountPanel::buttons[hit][3],mountHints_[hit].height);
    }
    return (inside && !blocked) || captured;
}

void Dialogs::Assets::PaintMount(cui_ctx* ui, cui_rect rect, const LoginClient& client) const
{
    const HudPoint base(rect);
    const auto& mount = client.mount;
    const auto [x,y,w,h] = mount.rect;
    const auto image = [&](const Image& art, int left, int top, int alpha) {
        if (art.texture) cui_draw_image(ui,art.texture,base.Rect(left,top,art.width,art.height),{0,0,1,1},{},
            {255,255,255,static_cast<std::uint8_t>(alpha)});
    };
    image(mountSkin_,x-(mountSkin_.width-w)/2,y-(mountSkin_.height-h)/2,220);
    for (int i=0;i<static_cast<int>(mountButtons_.size());++i) {
        const auto [id,left,top,hint] = mir3::MountPanel::buttons[i];
        image(mountButtons_[i][i==mount.pressed],x+left,y+top,255);
    }
    if (mountHintButton_>=0) {
        const auto [id,left,top,hint] = mir3::MountPanel::buttons[mountHintButton_];
        const auto& art = mountButtons_[mountHintButton_][0];
        HudHint::Paint(ui,base,mountHints_[mountHintButton_],mountHintHeight_,x+left+art.width,y+top+art.height);
    }
}

void Dialogs::Assets::DrawMountPanel(cui_ctx* ui, const LoginClient& client)
{
    const auto view = cui_viewport_size(ui);
    cui_overlay_begin(ui,{0,0,view.x,view.y});
    const struct { Assets* assets; const LoginClient* client; } data{this,&client};
    cui_canvas_paint(ui,CUI_FILL,CUI_FILL,[](cui_ctx* ui,cui_rect rect,void* value) {
        const auto& draw = *static_cast<decltype(data)*>(value);
        draw.assets->PaintMount(ui,rect,*draw.client);
    },&data,sizeof data);
    cui_box_end(ui);
}

void Dialogs::Assets::OpenTrade(cui_ctx* ui, mir3::WilArchive& archive)
{
    tradeSkin_ = UploadFrame(ui,archive,1390,true);
    tradeThumb_ = UploadFrame(ui,archive,1225,true);
    for (int i=0;i<3;++i) tradeConfirm_[i] = UploadFrame(ui,archive,1392+2*i,true);
    tradeHints_[0] = MakeHudHint(ui,1300);
    tradeHints_[1] = MakeHudHint(ui,1303);
}

void Dialogs::Assets::PostTradeFeedback(LoginClient& client)
{
    for (const auto& event : client.trade.feedback) {
        if (event.message) {
            auto text = Message(event.message);
            if (const auto at = text.find("%s"); at!=text.npos) text.replace(at,2,event.item);
            if (!text.empty()) client.chat.Append(100,text,0xffffff,33023);
        }
        if (!client.audioOptions.effects) continue;
        int sample = event.sample;
        if (!sample) {
            switch (event.itemMode) {
            case 0: sample=108; break;
            case 5: case 6: sample=111; break;
            case 10: case 11: sample=112; break;
            case 15: sample=116; break;
            case 19: case 20: case 21: sample=115; break;
            case 22: case 23: sample=113; break;
            case 24: case 26:
                sample=114;
                for (int id : {2641,2642}) {
                    const auto word = Message(id);
                    if (!word.empty() && event.item.find(word)!=event.item.npos) sample=117;
                }
                break;
            case 31: sample=event.subMode<106 ? 108 : 118; break;
            default: sample=118; break;
            }
        }
        const auto sound = clickSound_.parent_path()/(std::to_wstring(sample)+L".wav");
        if (std::filesystem::exists(sound)) client.audio.PlayEffect(sound.c_str(),client.audioOptions);
    }
    client.trade.feedback.clear();
}

bool Dialogs::Assets::HandleTradeInput(cui_ctx* ui, LoginClient& client, bool blocked)
{
    auto& trade = client.trade;
    tradeHintHeight_ = 0;
    const auto size = cui_viewport_size(ui);
    const HudPoint base({0,0,size.x,size.y});
    if (base.scale<=0) return false;
    const auto pointer = base.ToLocal(cui_mouse_pos(ui));
    const auto previous = std::exchange(tradePointer_,pointer);
    const bool moved = previous.x!=pointer.x || previous.y!=pointer.y;
    const auto [x,y,w,h] = trade.rect;
    const bool captured = trade.dragging || trade.confirmPressed || trade.scroll.captured>=0;
    const bool inside = Contains({float(x),float(y),float(w),float(h)},pointer);
    if (blocked && !captured) return false;
    const bool down = cui_mouse_clicked(ui,CUI_MOUSE_LEFT), up = cui_mouse_released(ui,CUI_MOUSE_LEFT);
    const auto scroll = [&](int kind) {
        return tradeThumb_.texture && trade.scroll.pointer(kind,int(pointer.x)-x,int(pointer.y)-y,
            tradeThumb_.width,tradeThumb_.height);
    };
    if ((!blocked && down && scroll(0)) || (up && scroll(2)) || (!up && moved && scroll(1))) return true;
    const auto& confirm = tradeConfirm_[1];
    const bool onConfirm = !blocked && Contains({float(x+226),float(y+279),float(confirm.width),float(confirm.height)},pointer);
    if (inside && !blocked && down) {
        trade.confirmPressed = onConfirm;
        trade.begin_drag(int(pointer.x),int(pointer.y),onConfirm || pointer.y>=y+56);
    } else if (up) {
        trade.end_drag();
        if (std::exchange(trade.confirmPressed,false) && onConfirm) client.ConfirmTrade();
        if (inside && !blocked && !captured) {
            if (Contains({float(x+34),float(y+270),122,34},pointer)) client.BeginTradeGold();
            if (Contains({float(x+14),float(y+56),190,190},pointer)) {
                const int column = (int(pointer.x)-x-14)/38;
                const int row = trade.scroll.first[0]+(int(pointer.y)-y-56)/38;
                if (const int notice = client.ActivateTradeCell(row*5+column)) client.chat.Append(100,Message(notice),0xffff32);
            }
        }
    } else if (captured && moved) trade.move_drag(int(pointer.x),int(pointer.y));
    if (onConfirm && inside && !trade.confirmPressed && !trade.dragging && tradeHints_[trade.confirmed].texture)
        tradeHintHeight_ = tradeHint_.Advance(ui,trade.confirmed ? 1303 : 1300,tradeHints_[trade.confirmed].height);
    return (inside && !blocked) || captured;
}

void Dialogs::Assets::PaintTrade(cui_ctx* ui, cui_rect rect, const LoginClient& client) const
{
    const HudPoint base(rect);
    const auto& trade = client.trade;
    const auto [x,y,w,h] = trade.rect;
    constexpr cui_color white{255,255,255,255}, text{200,200,250,255};
    const auto image = [&](const Image& art,int left,int top,cui_color tint = {255,255,255,255}) {
        if (art.texture) cui_draw_image(ui,art.texture,base.Rect(left,top,art.width,art.height),{0,0,1,1},{},tint);
    };
    image(tradeSkin_,x-(tradeSkin_.width-w)/2,y-(tradeSkin_.height-h)/2);
    image(tradeNames_[0].image,x,y,text);
    image(tradeNames_[1].image,x+247,y,text);
    for (int side=0;side<2;++side) {
        const int left = x+(side==0 ? 14 : 264);
        const int first = trade.scroll.first[side];
        image(tradeThumb_,x+mir3::TradeScroll::x[side],y+mir3::TradeScroll::top+trade.scroll.offset(side));
        cui_push_clip(ui,base.Rect(left,y+56,190,190));
        for (std::size_t i=0;i<trade.items[side].size();++i) {
            const auto& item = trade.items[side][i];
            const int row = item.cell/5;
            if (row>=first+5 || row+item.height<=first) continue;
            const int dx = left+38*(item.cell%5), dy = y+56+38*(row-first);
            const auto& art = itemFrames_.at(item.icon);
            image(art,dx+(38*item.width-art.width)/2,dy+(38*item.height-art.height)/2,
                side==0 && item.cell==trade.returnSlot ? cui_color{255,255,255,128} : white);
            if (dx+38<=left+190 && dy+23>=y+56 && dy+38<=y+246 && art.texture)
                image(tradeCounts_[side][i].image,dx+22,dy+23);
        }
        cui_pop_clip(ui);
        image(tradeAmounts_[side].image,x+(side==0 ? 70 : 350),y+289,text);
    }
    image(tradeConfirm_[trade.confirmed ? 1 : trade.confirmPressed ? 2 : 0],x+226,y+279);
    if (tradeHintHeight_) {
        const auto& confirm = tradeConfirm_[1];
        HudHint::Paint(ui,base,tradeHints_[trade.confirmed],tradeHintHeight_,x+226+confirm.width,y+279+confirm.height);
    }
}

void Dialogs::Assets::DrawTradePanel(cui_ctx* ui, const LoginClient& client)
{
    const auto& trade = client.trade;
    SetBagText(ui,tradeNames_[0],client.gameCharacter,228,30,true);
    SetBagText(ui,tradeNames_[1],trade.partner,120,30,true);
    for (int side=0;side<2;++side) {
        for (std::size_t i=0;i<trade.items[side].size();++i) {
            const auto& item = trade.items[side][i];
            CacheItem(ui,item.icon);
            SetBagText(ui,tradeCounts_[side][i],item.count>1 ? std::to_string(item.count) : "",16,15);
        }
        auto amount = std::to_string(trade.gold[side]);
        for (int i=int(amount.size())-3;i>0;i-=3) amount.insert(i,1,',');
        SetBagText(ui,tradeAmounts_[side],std::move(amount),side==0 ? 113 : 85,25,true);
    }
    const auto size = cui_viewport_size(ui);
    cui_overlay_begin(ui,{0,0,size.x,size.y});
    const struct { Assets* assets; const LoginClient* client; } data{this,&client};
    cui_canvas_paint(ui,CUI_FILL,CUI_FILL,[](cui_ctx* ui,cui_rect r,void* user) {
        const auto& draw = *static_cast<decltype(data)*>(user);
        draw.assets->PaintTrade(ui,r,*draw.client);
    },&data,sizeof data);
    cui_box_end(ui);
}

void Dialogs::Assets::DrawPanelHeld(cui_ctx* ui, LoginClient& client)
{
    const auto* item = client.trade.SelectedReturn();
    if (!item) item = client.HeldItem();
    const bool gold = client.inventory.heldGold;
    if (!item && !gold) return;
    const unsigned icon = gold ? 124 : item->icon;
    CacheItem(ui,icon);
    const auto& art = itemFrames_.at(icon);
    if (!art.texture) return;
    const auto size = cui_viewport_size(ui);
    const HudPoint base({0,0,size.x,size.y});
    const auto mouse = base.ToLocal(cui_mouse_pos(ui));
    if (!Contains({0,0,800,600},mouse)) return;
    const auto where = base.Rect(std::floor(mouse.x-art.width*.5f),std::floor(mouse.y-art.height*.5f),art.width,art.height);
    cui_overlay_begin(ui,where);
    const auto texture = art.texture;
    cui_canvas_paint(ui,CUI_FILL,CUI_FILL,[](cui_ctx* ui,cui_rect r,void* value) {
        cui_draw_image(ui,*static_cast<const cui_texture*>(value),r,{0,0,1,1},{},{255,255,255,255});
    },&texture,sizeof texture);
    cui_box_end(ui);
}

void Dialogs::Assets::DrawTradePrompt(cui_ctx* ui, LoginClient& client)
{
    using mir3_cui::TradePrompt;
    auto& trade = client.trade;
    if (trade.prompt==TradePrompt::None) return;
    const auto size = cui_viewport_size(ui);
    const HudPoint base({0,0,size.x,size.y});
    const auto pointer = base.ToLocal(cui_mouse_pos(ui));
    const bool alert = trade.prompt==TradePrompt::Failed;
    const bool reset = tradePromptEditor_!=trade.editor;
    if (reset) {
        tradePromptEditor_ = trade.editor;
        tradePromptCancel_ = tradePromptTab_ = false;
        tradePromptPressed_ = -1;
    }
    const int x = 400-dialog_[0].width/2, y = 246-dialog_[0].height/2;
    const auto hit = [&](const Image& art,int dx) {
        return Contains({float(x+dx),float(y+188),float(art.width),float(art.height)},pointer);
    };
    const int button = alert ? (hit(dialog_[1],137) ? 0 : -1) :
        hit(selectionButtons_.at(1241),41) ? 0 : hit(selectionButtons_.at(1245),234) ? 1 : -1;
    if (button!=tradePromptPressed_) tradePromptPressed_ = -1;
    if (cui_mouse_clicked(ui,CUI_MOUSE_LEFT)) tradePromptPressed_ = button;
    const bool click = cui_mouse_released(ui,CUI_MOUSE_LEFT) && std::exchange(tradePromptPressed_,-1)>=0;
    const bool tab = cui_key_down(ui,CUI_KEY_TAB);
    if (!alert && !tradePromptTab_ && (tab || cui_key_pressed(ui,CUI_KEY_TAB))) tradePromptCancel_=!tradePromptCancel_;
    tradePromptTab_ = tab;
    const bool enter = cui_key_pressed(ui,CUI_KEY_ENTER);
    if (click || (alert && (enter || cui_key_pressed(ui,CUI_KEY_SPACE)))) {
        client.FinishTradePrompt(click && button==0);
        return;
    }
    const int message = alert ? 1434 : trade.prompt==TradePrompt::Gold ? 1304 : 1307;
    SetBagText(ui,tradePromptText_,Message(message),alert ? 308 : 278,alert ? 95 : 75,true,mir3::TextFont::dialog);
    cui_overlay_begin(ui,{0,0,size.x,size.y});
    const struct { Assets* assets; bool alert; } data{this,alert};
    cui_canvas_paint(ui,CUI_FILL,CUI_FILL,[](cui_ctx* ui,cui_rect r,void* user) {
        const auto& draw = *static_cast<decltype(data)*>(user);
        const auto& assets = *draw.assets;
        const HudPoint base(r);
        const int x = 400-assets.dialog_[0].width/2, y = 246-assets.dialog_[0].height/2;
        const auto image = [&](const Image& art,int left,int top,cui_color tint = {255,255,255,255}) {
            if (art.texture) cui_draw_image(ui,art.texture,base.Rect(x+left,y+top,art.width,art.height),{0,0,1,1},{},tint);
        };
        image(assets.dialog_[0],0,0);
        image(assets.tradePromptText_.image,draw.alert ? 23 : 43,84,{255,250,200,255});
        if (draw.alert) image(assets.dialog_[assets.tradePromptPressed_==0 ? 2 : 1],137,188);
        else {
            image(assets.selectionButtons_.at(1241+(assets.tradePromptPressed_==0)),41,188);
            image(assets.selectionButtons_.at(1245+(assets.tradePromptPressed_==1)),234,188);
        }
    },&data,sizeof data);
    if (!alert) {
        const auto field = base.Rect(x+24,y+228,309,15);
        cui_overlay_begin(ui,field);
        cui_push_id_int(ui,trade.editor);
        if (enter || reset || (cui_mouse_clicked(ui,CUI_MOUSE_LEFT) && !Contains(field,cui_mouse_pos(ui))))
            cui_text_field_focus(ui,"trade-amount",false);
        const auto input = ModalEditorOptions(field.h,base.scale);
        cui_input_ex(ui,"trade-amount",trade.input,sizeof trade.input,&input);
        if (enter) {
            client.FinishTradePrompt(!tradePromptCancel_);
            cui_request_redraw(ui);
        }
        cui_pop_id(ui);
        cui_box_end(ui);
    }
    cui_box_end(ui);
}

bool Dialogs::Assets::HandleInventoryUtility(cui_ctx* ui, LoginClient& client, bool blocked)
{
    const bool hit = InventoryBlocksScene(ui,client);
    if (blocked && cui_mouse_released(ui,CUI_MOUSE_LEFT))
        inventoryDragging_ = inventoryScrollDrag_ = inventoryClosePress_ = inventoryServicePress_ = false;
    if (!blocked && hit) {
        const auto view = cui_viewport_size(ui);
        const HudPoint base({0,0,view.x,view.y});
        if (base.scale>0) InventoryInput(ui,client,base.ToLocal(cui_mouse_pos(ui)));
    }
    return hit && !blocked;
}

cui_rect Dialogs::Assets::UtilityBounds(UtilityPanel panel,const LoginClient& client) const
{
    const auto rect=[](const auto& bounds) {
        return cui_rect{float(bounds[0]),float(bounds[1]),float(bounds[2]),float(bounds[3])};
    };
    switch (panel) {
    case UtilityPanel::Party: return {float(partyX_),float(partyY_),258,244};
    case UtilityPanel::Guild: return guildPosition_.Rect();
    case UtilityPanel::Quest: return {float(questX_),float(questY_),340,440};
    case UtilityPanel::Skills: return rect(client.skills.rect);
    case UtilityPanel::Status: return rect(statusPanel_.rect);
    case UtilityPanel::Inventory: return {float(inventoryX_),float(inventoryY_),inventoryWidth_,inventoryHeight_};
    case UtilityPanel::Options: return rect(optionsBounds_);
    case UtilityPanel::Portrait: return rect(mir3::PortraitPicker::initial);
    case UtilityPanel::Mount: return rect(client.mount.rect);
    case UtilityPanel::Trade: return rect(client.trade.rect);
    case UtilityPanel::Quick: return {float(quickX_),float(quickY_),280,48};
    }
    return {};
}

void Dialogs::Assets::ScrollUtility(cui_ctx* ui,LoginClient& client,bool blocked)
{
    const auto scroll=cui_mouse_scroll(ui);
    if (blocked || (!scroll.x && !scroll.y)) return;
    const auto view=cui_viewport_size(ui);
    const HudPoint base({0,0,view.x,view.y});
    if (base.scale<=0) return;
    const auto point=base.ToLocal(cui_mouse_pos(ui));
    if (client.minimap.visible) {
        const auto [x,y,w,h]=client.minimap.bounds();
        if (Contains({float(x),float(y),float(w),float(h)},point)) {
            cui_take_mouse_scroll(ui);
            return;
        }
    }
    const int wheel=static_cast<int>(std::lround(scroll.y));
    for (auto it=utilityOrder_.rbegin();it!=utilityOrder_.rend();++it) {
        const auto panel=*it;
        if (!UtilityVisible(panel,client)) continue;
        if (!Contains(UtilityBounds(panel,client),point)) {
            if (panel==UtilityPanel::Skills && client.skills.menu_target>=0 && Contains(skillMenu_,point)) {
                cui_take_mouse_scroll(ui);
                return;
            }
            continue;
        }
        cui_take_mouse_scroll(ui);
        switch (panel) {
        case UtilityPanel::Quest:
            if (std::exchange(client.quests.resetScroll,false)) {
                questFirst_=0;
                questScrollDrag_=false;
            }
            if (std::exchange(client.quests.resetDescription,false)) questDescriptionFirst_=0;
            ScrollQuest(client.quests,point,-wheel);
            break;
        case UtilityPanel::Guild:
            client.guild.first=std::clamp(client.guild.first-wheel,0,(std::max)(0,GuildRowCount(client.guild)-1));
            break;
        case UtilityPanel::Skills:
            client.skills.scroll=std::clamp(client.skills.scroll-30*wheel,0,190);
            break;
        case UtilityPanel::Portrait:
            for (int i=0;i<std::abs(wheel);++i) portraitPicker_.wheel(wheel);
            break;
        default: break;
        }
        return;
    }
    const bool expanded=client.chat.expanded;
    const auto [x,y,w,h]=mir3::HudLayout::chat_frame(expanded);
    if (Contains({float(x),float(y),float(w),float(h)},point)) {
        auto& history=client.chat.nativeHistory;
        cui_take_mouse_scroll(ui);
        if (wheel) history.scroll(expanded).wheel(wheel,int(history.lines(expanded).size()),expanded);
    }
}

bool Dialogs::Assets::HandleUtilityPanels(cui_ctx* ui, LoginClient& client, bool covered, bool foreground, bool modal,
                                         bool& escape, bool& pointerCaptured)
{
    RefreshPortraitCache(ui,client);
    PrepareInventoryPlacement(client);
    if (statusPanel_.visible!=client.equipment.open) {
        statusPanel_.visible = client.equipment.open;
        statusPanel_.end_drag();
        statusClosePressed_ = false;
        statusPointer_ = {-1,-1};
        if (statusPanel_.visible && StatusReady()) RaiseUtility(UtilityPanel::Status);
    }
    if (std::exchange(tradeRevision_,client.trade.openRevision)!=client.trade.openRevision && client.trade.visible)
        RaiseUtility(UtilityPanel::Trade);
    if (std::exchange(utilityInventoryOpen_,client.inventory.open)!=client.inventory.open && client.inventory.open)
        RaiseUtility(UtilityPanel::Inventory);
    if (std::exchange(utilityQuickOpen_,client.inventory.shortcutsVisible)!=client.inventory.shortcutsVisible && QuickReady()) {
        if (utilityQuickOpen_) RaiseUtility(UtilityPanel::Quick);
        else {
            quickDragging_ = quickClosePress_ = quickSkipRelease_ = false;
            quickLastSlot_ = -1;
        }
    }
    if (QuestReady() && client.quests.open && !questFocused_) RaiseUtility(UtilityPanel::Quest);
    if (PartyReady() && client.party.open && !partyFocused_) RaiseUtility(UtilityPanel::Party);
    if (GuildReady() && client.guild.open && !guildFocused_) RaiseUtility(UtilityPanel::Guild);
    ScrollUtility(ui,client,modal);
    const auto order = utilityOrder_;
    const bool close = !modal && !client.shop.actor && !client.chat.composing &&
        cui_shortcut(ui,0,CUI_KEY_ESCAPE);
    const auto handle=[&](UtilityPanel panel,bool blocked,bool keyboard) {
        switch (panel) {
        case UtilityPanel::Party: return HandlePartyInput(ui,client,blocked);
        case UtilityPanel::Guild: return HandleGuildInput(ui,client,blocked);
        case UtilityPanel::Quest: return HandleQuestInput(ui,client,blocked);
        case UtilityPanel::Skills: return HandleSkillInput(ui,client,blocked,keyboard && !client.chat.composing);
        case UtilityPanel::Status: return HandleStatusInput(ui,client,blocked);
        case UtilityPanel::Inventory: return HandleInventoryUtility(ui,client,blocked);
        case UtilityPanel::Options: return HandleOptionsInput(ui,client,blocked);
        case UtilityPanel::Portrait: return HandlePortraitPicker(ui,client,blocked);
        case UtilityPanel::Mount: return HandleMountInput(ui,client,blocked);
        case UtilityPanel::Trade: return HandleTradeInput(ui,client,blocked);
        case UtilityPanel::Quick: return HandleQuickInput(ui,client,blocked);
        }
        return false;
    };
    bool coveredOutsideBag = covered;
    std::optional<UtilityPanel> owner;
    if (!modal && (cui_mouse_down(ui,CUI_MOUSE_LEFT) || cui_mouse_released(ui,CUI_MOUSE_LEFT))) {
        const std::pair<UtilityPanel,bool> captures[]{
            {UtilityPanel::Quick,quickDragging_ || quickClosePress_ || quickSkipRelease_},
            {UtilityPanel::Trade,client.trade.dragging || client.trade.confirmPressed || client.trade.scroll.captured>=0},
            {UtilityPanel::Quest,questDragging_ || questScrollDrag_ || questPressed_>=0},
            {UtilityPanel::Mount,client.mount.dragging || client.mount.pressed>=0},
            {UtilityPanel::Portrait,portraitPicker_.scrolling || portraitPressed_>=0},
            {UtilityPanel::Options,optionsDragging_ || optionsAudioDrag_>=0},
            {UtilityPanel::Guild,guildPosition_.dragging || guildScroll_.captured || guildLayout_.pressed>=0},
            {UtilityPanel::Status,statusPanel_.dragging || statusClosePressed_},
            {UtilityPanel::Inventory,inventoryDragging_ || inventoryScrollDrag_ || inventoryClosePress_ || inventoryServicePress_},
            {UtilityPanel::Skills,client.skills.dragging || client.skills.scroll_captured || client.skills.pressed>=0},
            {UtilityPanel::Party,partyDragging_ || partyPressed_>=0},
        };
        for (const auto& [panel,captured]:captures) {
            if (!captured || !UtilityVisible(panel,client)) continue;
            owner=panel;
            pointerCaptured=true;
            handle(panel,false,!covered && !foreground);
            covered=true;
            coveredOutsideBag|=panel!=UtilityPanel::Inventory;
            break;
        }
    }
    for (auto it=order.rbegin();it!=order.rend();++it) {
        const auto panel = *it;
        if (!UtilityVisible(panel,client)) continue;
        const bool blocked = covered || foreground || modal;
        const bool hit = panel!=owner && handle(panel,blocked,true);
        if (hit && !blocked && cui_mouse_clicked(ui,CUI_MOUSE_LEFT)) RaiseUtility(panel);
        covered |= hit;
        if (panel!=UtilityPanel::Inventory) coveredOutsideBag |= hit;
        if (!close || escape) continue;
        switch (panel) {
        case UtilityPanel::Party: ToggleParty(client); break;
        case UtilityPanel::Guild:
            client.guild.open = false;
            LeaveGuild();
            break;
        case UtilityPanel::Quest: ToggleQuest(client); break;
        case UtilityPanel::Skills: ToggleSkills(client); break;
        case UtilityPanel::Status: ToggleStatus(client); break;
        case UtilityPanel::Inventory:
            if (!QuickReady() && client.inventory.shortcutsVisible) client.ToggleItemShortcuts();
            else client.CloseInventory();
            break;
        case UtilityPanel::Options: ToggleOptions(client); break;
        case UtilityPanel::Portrait: TogglePortraitPicker(ui); break;
        case UtilityPanel::Mount: ToggleMount(client); break;
        case UtilityPanel::Trade: break;
        case UtilityPanel::Quick: ToggleQuick(client); break;
        }
        escape = true;
    }
    pointerCoversPanel_ = coveredOutsideBag || foreground || portraitNotice_ || client.showSignOut || client.returnToCharactersAt;
    return covered;
}

void Dialogs::Assets::DrawUtilityPanels(cui_ctx* ui, LoginClient& client, bool inventoryModal)
{
    for (const auto panel : utilityOrder_) {
        if (!UtilityVisible(panel,client)) continue;
        switch (panel) {
        case UtilityPanel::Party: DrawPartyPanel(ui,client,true,false); break;
        case UtilityPanel::Guild: DrawGuildPanel(ui,client,true,false); break;
        case UtilityPanel::Quest: DrawQuestPanel(ui,client); break;
        case UtilityPanel::Skills: DrawSkillsPanel(ui,client); break;
        case UtilityPanel::Status: DrawStatusPanel(ui,client); break;
        case UtilityPanel::Inventory:
            if (!inventoryModal) DrawInventoryPanel(ui,client,false,false);
            break;
        case UtilityPanel::Options: DrawOptionsPanel(ui,client); break;
        case UtilityPanel::Portrait: DrawPortraitPicker(ui); break;
        case UtilityPanel::Mount: DrawMountPanel(ui,client); break;
        case UtilityPanel::Trade: DrawTradePanel(ui,client); break;
        case UtilityPanel::Quick: DrawQuickPanel(ui,client,false); break;
        }
    }
}

bool Dialogs::Assets::CanExitGame(LoginClient& client) const
{
    if (client.GameExitAllowed()) return true;
    const auto notice = Message(1407);
    if (!notice.empty()) client.chat.Append(100,notice,0xffffff,0x0000ff);
    return false;
}

void Dialogs::Assets::OpenGameExit(LoginClient& client, bool quit)
{
    if (!CanExitGame(client)) return;
    client.showSignOut = true;
    client.quitGameRequested = quit;
    client.chat.composing = false;
    LeaveInventory();
    LeaveOptions();
    LeaveParty();
    LeaveGuild();
    LeaveQuests();
    statusPanel_.end_drag();
    statusClosePressed_ = false;
    client.trade.end_drag();
    client.trade.scroll.captured = -1;
    client.trade.confirmPressed = false;
    client.mount.end_drag();
    client.mount.pressed = -1;
    quickDragging_ = quickClosePress_ = quickSkipRelease_ = false;
    pointerCoversPanel_ = true;
    gameExitCancel_ = false;
    gameExitPressed_ = -1;
    portraitEntryPressed_ = false;
    portraitPicker_.scrolling = false;
    portraitPressed_ = -1;
}

bool Dialogs::Assets::DrawGameExit(cui_ctx* ui, LoginClient& client)
{
    if (!dialog_[0].texture) return false;
    for (const int id : {1241,1242,1245,1246})
        if (!selectionButtons_.at(id).texture) return false;
    const auto size = cui_viewport_size(ui);
    const HudPoint base({0,0,size.x,size.y});
    if (base.scale<=0) return false;
    const auto [x,y] = std::pair{400-dialog_[0].width/2,246-dialog_[0].height/2};
    const auto pointer = base.ToLocal(cui_mouse_pos(ui));
    const auto hit = [&](int id, int left) {
        const auto& image = selectionButtons_.at(id);
        return Contains({float(x+left),float(y+188),float(image.width),float(image.height)},pointer);
    };
    const int button = hit(1241,41) ? 0 : hit(1245,234) ? 1 : -1;
    if (button!=gameExitPressed_) gameExitPressed_ = -1;
    if (cui_mouse_clicked(ui,CUI_MOUSE_LEFT)) gameExitPressed_ = button;
    const bool click = cui_mouse_released(ui,CUI_MOUSE_LEFT) && std::exchange(gameExitPressed_,-1)>=0;
    if (cui_key_pressed(ui,CUI_KEY_TAB)) gameExitCancel_ = !gameExitCancel_;
    if (click || cui_key_pressed(ui,CUI_KEY_ENTER) || cui_key_pressed(ui,CUI_KEY_SPACE)) {
        client.showSignOut = false;
        const bool quit = std::exchange(client.quitGameRequested,false);
        if (click && client.audioOptions.effects && std::filesystem::exists(clickSound_))
            client.audio.PlayEffect(clickSound_.c_str(),client.audioOptions);
        if ((click ? button==0 : !gameExitCancel_) && CanExitGame(client)) {
            if (quit) cui_app_quit(nullptr);
            else client.SignOutGame(SelectionReady(client));
        }
        return true;
    }
    SetBagText(ui,gameExitText_,Message(client.quitGameRequested ? 2636 : 1065),308,95,true,mir3::TextFont::dialog);
    cui_overlay_begin(ui,{0,0,size.x,size.y});
    const struct { const Assets* assets; int x,y,pressed; } draw{this,x,y,gameExitPressed_};
    cui_canvas_paint(ui,CUI_FILL,CUI_FILL,[](cui_ctx* ui,cui_rect rect,void* value) {
        const auto& data = *static_cast<decltype(draw)*>(value);
        const HudPoint base(rect);
        const auto image = [&](const Image& art,int left,int top,cui_color tint = {255,255,255,255}) {
            if (art.texture) cui_draw_image(ui,art.texture,base.Rect(data.x+left,data.y+top,art.width,art.height),
                {0,0,1,1},{},tint);
        };
        image(data.assets->dialog_[0],0,0);
        image(data.assets->gameExitText_.image,23,84,{255,250,200,255});
        image(data.assets->selectionButtons_.at(1241+(data.pressed==0)),41,188);
        image(data.assets->selectionButtons_.at(1245+(data.pressed==1)),234,188);
    },&draw,sizeof draw);
    cui_box_end(ui);
    return true;
}

void Dialogs::Assets::DrawGameExitFade(cui_ctx* ui, const LoginClient& client)
{
    const auto now = GetTickCount64();
    const auto start = client.returnToCharactersAt-LoginClient::GameExitFadeMs-1;
    const auto elapsed = now>start ? (std::min)(now-start,LoginClient::GameExitFadeMs) : 0;
    const auto opacity = static_cast<std::uint8_t>(std::lround(elapsed*double(0.1f)));
    const auto size = cui_viewport_size(ui);
    cui_overlay_begin(ui,{0,0,size.x,size.y});
    cui_canvas_paint(ui,CUI_FILL,CUI_FILL,[](cui_ctx* ui,cui_rect rect,void* value) {
        const HudPoint base(rect);
        const auto opacity = *static_cast<const std::uint8_t*>(value);
        if (opacity) cui_draw_rect(ui,base.Rect(0,0,800,600),{},{0,0,0,opacity});
    },&opacity,sizeof opacity);
    cui_box_end(ui);
    cui_request_wakeup(ui,0.016f);
}

void Dialogs::Assets::GameHudInput(cui_ctx* ui, LoginClient& client, cui_vec2 pointer, bool pointerCovered, bool modal)
{
    using Layout=mir3::HudLayout;
    auto& chat=client.chat;
    auto& history=chat.nativeHistory;
    const bool expanded=chat.expanded;
    const auto& thumb=gameHudFrames_.at(1207);
    auto& scroll=history.scroll(expanded);
    const int x=static_cast<int>(std::floor(pointer.x)), y=static_cast<int>(std::floor(pointer.y));
    const bool down=cui_mouse_clicked(ui,CUI_MOUSE_LEFT), up=cui_mouse_released(ui,CUI_MOUSE_LEFT);
    pointerCovered |= HandlePortraitEntry(ui,client,pointer,pointerCovered || modal);
    const auto tick=GetTickCount();
    mir3::HudController controls(gameHud_);
    gameHud_.layout.expanded=expanded;
    gameScrollCaptured_=scroll.captured();
    if (gameScrollCaptured_) {
        history.input(up ? 2 : 1,x,y,expanded,thumb.width,thumb.height,tick);
        gameScrollCaptured_=true;
        controls.cancel();
    } else if (pointerCovered || modal) {
        controls.cancel();
    } else {
        if (down && history.input(0,x,y,expanded,thumb.width,thumb.height,tick)) {
            controls.cancel();
            gameScrollCaptured_=true;
        } else {
            controls.motion(x,y);
            if (down || up) {
                const int action=controls.pointer(x,y,down);
                const auto r=Layout::portrait_hover;
                if (up && !expanded && Contains({float(r[0]),float(r[1]),float(r[2]),float(r[3])},pointer))
                    client.TogglePortraitMap();
                switch (action) {
                case 0: client.RequestTrade(); break;
                case 1: client.ToggleMinimap(100); break;
                case 2: client.skillShortcutsVisible=!client.skillShortcutsVisible; break;
                case 3: case 4: OpenGameExit(client,action==3); break;
                case 5: ToggleParty(client); break;
                case 6: client.RequestGuildPage(0); break;
                case 7: ToggleSkills(client); break;
                case 8: ToggleQuest(client); break;
                case 9: ToggleOptions(client); break;
                case 11: ToggleMount(client); break;
                case 12:
                    if (client.inventory.open) client.CloseInventory();
                    else {
                        client.OpenInventory();
                        if (InventoryReady()) RaiseUtility(UtilityPanel::Inventory);
                    }
                    break;
                case 13: ToggleStatus(client); break;
                case 14: chat.expanded=gameHud_.layout.expanded; break;
                case 16: case 17:
                    chat.composing=true;
                    chat.FillDraft(action==16 ? Message(1072) : "!");
                    break;
                case 18: case 19:
                    chat.channel=chat.channel==action-17 ? 0 : action-17;
                    gameChatModeChanged_=true;
                    chat.Append(100,Message(chat.channel==1 ? 2633 : chat.channel==2 ? 2635 : 2626),0xffffff,0x80ff);
                    break;
                case 20: case 21: case 22:
                    client.ToggleChatReception(static_cast<mir3_cui::ReceptionChannel>(action-20));
                    break;
                }
                if (action>=0 && client.audioOptions.effects && std::filesystem::exists(clickSound_))
                    client.audio.PlayEffect(clickSound_.c_str(),client.audioOptions);
            }
        }
    }
    if (modal || client.showSignOut) {
        gameChatFocused_=false;
        return;
    }
    const cui_rect field{185.f,chat.expanded ? 534.f : 575.f,chat.expanded ? 432.f : 344.f,15.f};
    const bool openWithSpace = !chat.composing && cui_key_pressed(ui,CUI_KEY_SPACE);
    const bool openWithKey = openWithSpace || (!chat.composing && cui_key_pressed(ui,CUI_KEY_ENTER));
    if (openWithKey || (down && !pointerCovered && Contains(field,pointer))) {
        if (!chat.composing) {
            if (openWithSpace) cui_input_consume_text_prefix(ui," ");
            if (openWithKey && chat.channel) chat.FillDraft(chat.ChannelPrefix());
            else ++chat.editor;
        }
        chat.composing=true;
    } else if (chat.composing && cui_key_pressed(ui,CUI_KEY_ESCAPE)) {
        chat.composing=false;
    } else if (!chat.composing) {
        if (cui_shortcut(ui,CUI_MOD_ALT,CUI_KEY_Q)) OpenGameExit(client,true);
        if (cui_shortcut(ui,CUI_MOD_ALT,CUI_KEY_X)) OpenGameExit(client,false);
        if (cui_shortcut(ui,0,CUI_KEY_V)) client.ToggleMinimap(1000);
        if (cui_shortcut(ui,0,CUI_KEY_J)) client.TogglePortraitMap();
        if (cui_shortcut(ui,0,CUI_KEY_T) && client.minimap.visible) client.minimap.resize();
        if (cui_shortcut(ui,0,CUI_KEY_Y) && client.minimap.visible) client.minimap.translucent=!client.minimap.translucent;
        if (cui_shortcut(ui,0,CUI_KEY_R)) {
            gameHud_.layout.toggle();
            chat.expanded=gameHud_.layout.expanded;
            controls.cancel();
        }
        if (cui_shortcut(ui,0,CUI_KEY_W)) {
            if (InventoryReady() && client.inventory.open) client.CloseInventory();
            else if (!client.inventory.open) {
                client.OpenInventory();
                if (InventoryReady()) RaiseUtility(UtilityPanel::Inventory);
            }
        }
        if (cui_shortcut(ui,0,CUI_KEY_E)) ToggleSkills(client);
        if (cui_shortcut(ui,0,CUI_KEY_B)) client.skillShortcutsVisible=!client.skillShortcutsVisible;
        if (cui_shortcut(ui,0,CUI_KEY_G)) ToggleParty(client);
        if (cui_shortcut(ui,0,CUI_KEY_F)) client.RequestGuildPage(0);
        if (cui_shortcut(ui,0,CUI_KEY_Q)) {
            if (StatusReady() || !client.equipment.open) ToggleStatus(client);
        }
        if (cui_shortcut(ui,0,CUI_KEY_D)) ToggleQuest(client);
        if (cui_shortcut(ui,0,CUI_KEY_Z)) ToggleQuick(client);
        if (cui_shortcut(ui,0,CUI_KEY_N)) ToggleOptions(client);
        if (cui_shortcut(ui,0,CUI_KEY_S)) ToggleMount(client);
        if (cui_shortcut(ui,0,CUI_KEY_C)) client.RequestTrade();
        for (std::size_t i=0;i<client.inventory.shortcuts.size();++i) {
            if (!cui_shortcut(ui,0,static_cast<cui_key>(CUI_KEY_1+i))) continue;
            if (QuickReady()) client.ActivateShortcutSlot(i,false);
            else client.UseItemShortcut(i);
        }
    }
}

void Dialogs::Assets::RefreshGameHud(cui_ctx* ui, const LoginClient& client)
{
    using Layout=mir3::HudLayout;
    const auto& chat=client.chat;
    const auto& stats=client.characterStatus;
    gameHud_.layout.expanded=chat.expanded;
    gameHud_.level=static_cast<int>(stats.level);
    gameHud_.profession=static_cast<int>(client.characterAppearance.profession);
    gameHud_.appearance=static_cast<int>(client.characterAppearance.body);
    gameHud_.hp=static_cast<WORD>(stats.hp[0]); gameHud_.max_hp=static_cast<WORD>(stats.hp[1]);
    gameHud_.mp=static_cast<WORD>(stats.mp[0]); gameHud_.max_mp=static_cast<WORD>(stats.mp[1]);
    gameHud_.experience=stats.experience; gameHud_.next_experience=stats.nextExperience;
    gameHud_.weight=stats.weight[0]; gameHud_.max_weight=stats.weight[1];
    const auto range=[&](int i) { return std::array{static_cast<int>(stats.combat[i][0]),static_cast<int>(stats.combat[i][1])}; };
    gameHud_.attack=range(0); gameHud_.magic_attack=range(1); gameHud_.spirit_attack=range(2);
    gameHud_.defence=range(3); gameHud_.magic_defence=range(4);
    SetBagText(ui,gameHudText_[0],std::to_string(stats.level),16,16,true,mir3::TextFont::level);
    const auto& map=client.scene.mapDescription.empty() ? client.scene.mapId : client.scene.mapDescription;
    SetBagText(ui,gameHudText_[1],map.empty() ? "" : map+" : ["+std::to_string(client.scene.x)+","+
        std::to_string(client.scene.y)+"]",133,17,true);
    for(int i=0;i<2;++i) {
        const auto value=gameHud_.stat(i);
        SetBagText(ui,gameHudText_[2+i],std::to_string(value[1])+"-"+std::to_string(value[2]),Layout::stat(i)[2],24,true);
    }
    const auto& history=chat.nativeHistory;
    const auto& lines=history.lines(chat.expanded);
    const int first=history.first(chat.expanded), width=Layout::chat_line(chat.expanded)[2];
    gameRowsDrawn_=std::clamp(static_cast<int>(lines.size())-first,0,Layout::chat_rows(chat.expanded));
    for(int i=0;i<gameRowsDrawn_;++i) {
        const auto& line=lines[first+i];
        auto& row=gameChatRows_[i];
        if (row.text.value!=line.text || (row.text.image.texture && row.text.image.width!=width)) {
            row.text.value.clear();
            Destroy(ui,row.text.image);
            SetBagText(ui,row.text,line.text,width,15);
            const auto measured=mir3::raster_text(line.text,1,1,false,mir3::TextFont::normal);
            row.width=measured.text_width;
            row.height=measured.text_height;
        }
        row.ink=line.foreground;
        row.backing=line.background;
    }
    gameThumbY_=mir3::ChatScrollLayout::thumb_y(first,static_cast<int>(lines.size()),chat.expanded);
    gameChannel_=chat.channel;
    gameChatRejected_=chat.reception.displayed;
    gameHintDrawn_=-1;
    if (gameHud_.hovered>=0 && gameHud_.hovered!=gameHud_.pressed) {
        const int button=gameHud_.hovered;
        int id=2600+button;
        if (button==18) id=gameChannel_==1 ? 2631 : 2618;
        if (button==19) id=gameChannel_==2 ? 2630 : 2619;
        if (button>=20) id=std::array{2637,2639,2643}[button-20]+gameChatRejected_[button-20];
        if (const auto it=gameHudHints_.find(id);it!=gameHudHints_.end() && it->second.texture) {
            gameHintDrawn_=id;
            gameHintHeight_=gameButtonHint_.Advance(ui,id,it->second.height);
        }
    }
    gameMeterTipKind_=Layout::tooltip_at(gameHud_.pointer,gameHud_.shared_hp());
    if (gameMeterTipKind_>=0) {
        const int kind=gameMeterTipKind_;
        char percent[32]{};
        std::snprintf(percent,sizeof percent,"%.2f%%",stats.ExperiencePercent());
        const auto amount=kind==4 ? std::string(percent) :
            std::to_string(kind==3 ? stats.weight[0] : kind==1 ? stats.mp[0] : stats.hp[0])+"/"+
            std::to_string(kind==3 ? stats.weight[1] : kind==1 ? stats.mp[1] : stats.hp[1]);
        auto caption=Message(kind==4 ? 2624 : kind==3 ? 2625 : kind==1 ? 2623 : 2622)+amount;
        const auto measured=mir3::raster_text(caption,1,1,false,mir3::TextFont::normal);
        gameMeterTipBounds_=Layout::tooltip_rect(kind,{measured.text_width,measured.text_height});
        SetBagText(ui,gameMeterTip_,std::move(caption),gameMeterTipBounds_[2],gameMeterTipBounds_[3],true);
    }
}

void Dialogs::Assets::PaintGameHud(cui_ctx* ui, cui_rect rect) const
{
    using Layout=mir3::HudLayout;
    const HudPoint base(rect);
    const bool expanded=gameHud_.layout.expanded;
    const auto color=[](unsigned bgr) {
        return cui_color{static_cast<std::uint8_t>(bgr),static_cast<std::uint8_t>(bgr>>8),
            static_cast<std::uint8_t>(bgr>>16),255};
    };
    const auto bounds=[&](Layout::Rect r) { return base.Rect(r[0],r[1],r[2],r[3]); };
    const auto image=[&](const Image& art,Layout::Point p,cui_color tint=cui_color{255,255,255,255}) {
        if (art.texture) cui_draw_image(ui,art.texture,base.Rect(p[0],p[1],art.width,art.height),{0,0,1,1},{},tint);
    };
    const auto sprite=[&](int id,Layout::Point p) {
        if (const auto it=gameHudFrames_.find(id);it!=gameHudFrames_.end()) image(it->second,p);
    };
    cui_push_clip(ui,base.Rect(0,0,800,600));
    if (!expanded) {
        const auto p=Layout::image_position(Layout::Image::portrait);
        sprite(1490,p);
        if (portraitMapMode_) PaintPortraitMinimap(ui,base);
        else if (portraitEquipped_.texture) {
            const auto [x,y,w,h] = Layout::custom_portrait_rect(false);
            cui_draw_image(ui,portraitEquipped_.texture,base.Rect(x,y,w,h),{0,0,1,1},{},{255,255,255,255});
        } else if (const auto it=gamePortraits_.find(gameHud_.portrait_frame());it!=gamePortraits_.end()) image(it->second,p);
        if (targetCustomPortrait_.texture) {
            const auto [x,y,w,h] = Layout::custom_portrait_rect(true);
            cui_draw_image(ui,targetCustomPortrait_.texture,base.Rect(x,y,w,h),{0,0,1,1},{},{255,255,255,255});
        } else {
            image(targetPortrait_,p);
        }
        if (const auto badge=gamePortraits_.find(targetBadge_);badge!=gamePortraits_.end())
            image(badge->second,Layout::image_position(Layout::Image::target_badge));
    } else cui_draw_rect_premultiplied(ui,bounds(Layout::chat_backing),{30,30,30,150});
    sprite(1160,Layout::origin);
    const auto chat=Layout::chat_frame(expanded);
    sprite(expanded ? 1162 : 1161,{chat[0],chat[1]});
    auto line=Layout::chat_line(expanded);
    for(int i=0;i<gameRowsDrawn_;++i) {
        const auto& row=gameChatRows_[i];
        if (row.backing) cui_draw_rect(ui,base.Rect(line[0],line[1],row.width,row.height),{},color(row.backing));
        image(row.text.image,{line[0],line[1]},color(row.ink));
        line[1]+=line[3];
    }
    sprite(Layout::level_emblem_frame(gameHud_.emblem_selector,gameHud_.emblem_references,gameHud_.experience),
        Layout::level_emblem_position(expanded));
    const auto cropSprite=[&](const Image& art,Layout::Point p,int top,int width,int height) {
        if (!art.texture || width<=0 || height<=0) return;
        cui_draw_image(ui,art.texture,base.Rect(p[0],p[1]+top,width,height),
            {0,float(top)/art.height,float(width)/art.width,float(height)/art.height},{},{255,255,255,255});
    };
    const auto& left=gameHudFrames_.at(1163);
    const auto& right=gameHudFrames_.at(gameHud_.orb_frame(1));
    if (left.texture && right.texture)
        for(int i=0;i<2;++i) {
            const auto crop=gameHud_.orb_crop(i,{left.width,left.height},{right.width,right.height});
            cropSprite(i==0 ? left : right,Layout::image_position(i==0 ? Layout::Image::hp : Layout::Image::mp),
                crop[1],crop[2],crop[3]);
        }
    for(int i : {1,0}) {
        const auto& art=gameHudFrames_.at(1166+i);
        const int top=gameHud_.meter_crop(i,art.height);
        cropSprite(art,Layout::image_position(i==0 ? Layout::Image::experience : Layout::Image::weight),
            top,art.width,art.height-top);
    }
    if (gameMeterTipKind_>=0) {
        constexpr cui_color back[]={{120,60,60,150},{60,60,120,150},{120,60,60,150},{100,100,0,150},{80,60,40,150}};
        constexpr cui_color edge[]={{160,80,80,255},{80,80,160,255},{160,80,80,255},{120,120,0,255},{100,75,50,255}};
        const auto r=gameMeterTipBounds_;
        cui_draw_rect_border(ui,bounds(r),{},back[gameMeterTipKind_],edge[gameMeterTipKind_],base.scale);
        image(gameMeterTip_.image,{r[0],r[1]});
    }
    const auto level=Layout::level(expanded);
    image(gameHudText_[0].image,{level[0],level[1]},color(Layout::text_color(1)));
    if (gameHudText_[1].image.texture) {
        const auto r=Layout::coordinates;
        cui_draw_rect_premultiplied(ui,bounds(r),{100,75,50,100});
        for (const auto offset:Layout::coordinate_shadows)
            image(gameHudText_[1].image,{r[0]+offset[0],r[1]+offset[1]},color(Layout::coordinate_shadow_color));
        image(gameHudText_[1].image,{r[0],r[1]},color(Layout::text_color(0)));
    }
    for(int i=0;i<2;++i) {
        const auto r=Layout::stat(i);
        sprite(gameHud_.stat(i)[0],Layout::image_position(i==0 ? Layout::Image::left_stat : Layout::Image::right_stat));
        image(gameHudText_[2+i].image,{r[0],r[1]},color(Layout::text_color(2+i)));
    }
    sprite(1207,{mir3::ChatScrollLayout::x,gameThumbY_});
    for(int i=Layout::count(expanded)-1;i>=0;--i) {
        const auto button=Layout::button(i,expanded);
        auto frames=mir3::main_button_frames(i,gameHud_.hovered==i,gameHud_.pressed==i);
        if (gameChatModeChanged_ && (i==18 || i==19)) {
            const int id=i==18 ? (gameChannel_==1 ? 1335 : 1334) : (gameChannel_==2 ? 1337 : 1336);
            frames={id,-1};
        } else if (i>=20 && gameChatRejected_[i-20]) frames={1338+2*(i-20),-1};
        for(const auto id:frames) if (id>=0) sprite(id,{button[1],button[2]});
    }
    if (gameHintDrawn_>=0) {
        const auto hit=gameHud_.button_rect(gameHud_.hovered);
        HudHint::Paint(ui,base,gameHudHints_.at(gameHintDrawn_),gameHintHeight_,hit[0]+hit[2],hit[1]+hit[3]);
    }
    if (!expanded && portraitEntryVisible_) {
        const auto [x,y] = Layout::portrait_entry_position;
        const auto& entry = portraitPickerFrames_.at(Layout::portrait_entry_frame(portraitEntryPressed_));
        image(entry,{x,y});
        if (portraitEntryHintHeight_>0)
            HudHint::Paint(ui,base,portraitHints_[3],portraitEntryHintHeight_,x+entry.width,y+entry.height);
    }
    cui_pop_clip(ui);
}

void Dialogs::Assets::DrawGameHud(cui_ctx* ui, LoginClient& client, bool pointerCovered, bool modal)
{
    const auto view=cui_viewport_size(ui);
    const cui_rect screen{0,0,view.x,view.y};
    const HudPoint base(screen);
    if (base.scale<=0) return;
    const auto pointer=base.ToLocal(cui_mouse_pos(ui));
    const auto& thumb=gameHudFrames_.at(1207);
    const bool wasComposing = client.chat.composing;
    GameHudInput(ui,client,pointer,pointerCovered,modal);
    portraitMapMode_=client.portraitMapMode;
    portraitMapReady_=client.portraitMapReady;
    auto& chat=client.chat;
    const auto chatFrame=mir3::HudLayout::chat_frame(chat.expanded);
    const bool overHud=pointer.y>=467 || gameScrollCaptured_ ||
        Contains({619.f,float(mir3::ChatScrollLayout::top(chat.expanded)),float(thumb.width),
            float(mir3::ChatScrollLayout::travel(chat.expanded)+thumb.height)},pointer) ||
        Contains({float(chatFrame[0]),float(chatFrame[1]),float(chatFrame[2]),float(chatFrame[3])},pointer);
    const bool interactive=!pointerCovered && !modal && !overHud && !client.minimap.visible;
    const auto map = client.characterAppearance.available ? PrepareMapArt(ui,client,pointer,interactive) : SceneMapArt{};
    for (int i=0;i<12;++i) {
        const auto key=static_cast<cui_key>(CUI_KEY_F1+i);
        const bool repeat=std::exchange(castKeysHeld_[i],cui_key_down(ui,key));
        if (!interactive || wasComposing || chat.composing || !map.sprites || pointer.x<0 || pointer.x>=800 || pointer.y<0 ||
            !cui_shortcut(ui,0,key)) continue;
        client.RequestSpell(i+1,int(pointer.x+map.cameraX)/48,int(pointer.y+map.cameraY)/32,SceneTargetAt(map,pointer),repeat);
        break;
    }
    RefreshTargetPortrait(ui,client,map,pointer,interactive);
    if (client.characterAppearance.available) {
        cui_overlay_begin(ui,base.Rect(0,0,800,map.sprites ? 600 : 467));
        cui_push_disabled(ui,!interactive);
        DrawLocalScene(ui,client,interactive,InventoryReady(),true,map);
        cui_pop_disabled(ui);
        cui_box_end(ui);
    }
    if (std::exchange(client.scene.pickupBagFull,false)) client.chat.Append(100,Message(1417),11861940);
    RefreshGameHud(ui,client);
    cui_overlay_begin(ui,screen);
    auto* self=this;
    cui_canvas_paint(ui,CUI_FILL,CUI_FILL,[](cui_ctx* ui,cui_rect rect,void* user) {
        (*static_cast<Assets**>(user))->PaintGameHud(ui,rect);
    },&self,sizeof self);
    const bool editing = chat.composing && !modal && !client.showSignOut;
    PrepareChatShortcuts(ui,chat,editing);
    if (editing) {
        const auto field=base.Rect(185,chat.expanded ? 534 : 575,chat.expanded ? 432 : 344,15);
        cui_overlay_begin(ui,field);
        cui_push_id_int(ui,chat.editor);
        if (!gameChatFocused_ || gameChatEditor_!=chat.editor) cui_text_field_focus_end(ui,"hud-chat");
        gameChatFocused_=true;
        gameChatEditor_=chat.editor;
        cui_input_opts opts{};
        opts.bare=true;
        opts.keep_escape=true;
        opts.width=CUI_FILL;
        opts.height=field.h;
        opts.font_size=12*base.scale;
        opts.font=editorFont_;
        opts.text_color={223,223,223,255};
        opts.caret_color={243,243,243,255};
        opts.solid_caret=true;
        opts.byte_limit=90;
        opts.measure_bytes=AnsiBytes;
        const int input = cui_input_ex(ui,"hud-chat",chat.draft,sizeof chat.draft,&opts);
        const bool submitted = wasComposing && (input&CUI_SUBMITTED);
        if (submitted) client.SendChat(false);
        if ((input&CUI_BLURRED) || (submitted && !chat.draft[0])) {
            chat.composing = false;
            gameChatFocused_ = false;
            chatShortcutVisible_ = {};
        }
        cui_pop_id(ui);
        cui_box_end(ui);
    } else gameChatFocused_=false;
    cui_box_end(ui);
}

void Dialogs::Assets::PrepareChatShortcuts(cui_ctx* ui, mir3_cui::ChatPanel& chat, bool editing)
{
    const int mods = cui_key_mods(ui);
    chatShortcutVisible_ = {editing && (mods&CUI_MOD_CTRL) && !chat.contacts.empty(),
        editing && (mods&CUI_MOD_ALT) && !chat.phrases.empty()};
    if (!editing || !(mods&(CUI_MOD_CTRL|CUI_MOD_ALT))) return;

    const bool usePhrases = !(mods&CUI_MOD_CTRL);
    const auto& history = usePhrases ? chat.phrases : chat.contacts;
    auto& selected = usePhrases ? chat.phraseIndex : chat.contactIndex;
    if (!history.empty()) {
        const int last = static_cast<int>((std::min)(history.size(),chatShortcutLabels_[0].size()))-1;
        bool chosen = true;
        if (cui_key_pressed(ui,CUI_KEY_UP)) selected = (std::max)(0,selected-1);
        else if (cui_key_pressed(ui,CUI_KEY_DOWN)) selected = (std::min)(last,selected+1);
        else {
            chosen = false;
            for (int i=0;i<10;++i) {
                if (!cui_key_pressed(ui,static_cast<cui_key>(CUI_KEY_0+(i+1)%10))) continue;
                if (i<=last) selected = i;
                chosen = true;
                break;
            }
        }
        if (chosen && selected>=0 && selected<=last) {
            const auto ansi = ConvertText(history[selected],CP_UTF8,936);
            std::size_t end = 0;
            while (end<ansi.size()) {
                const auto next = end+(IsDBCSLeadByteEx(936,static_cast<BYTE>(ansi[end])) ? 2u : 1u);
                if (next>90 || next>ansi.size()) break;
                end = next;
            }
            chat.FillDraft(ConvertText(ansi.substr(0,end),936,CP_UTF8));
        }
    }

    for (int kind=0;kind<2;++kind) {
        if (!chatShortcutVisible_[kind]) continue;
        const auto& values = kind ? chat.phrases : chat.contacts;
        auto& labels = chatShortcutLabels_[kind];
        for (std::size_t i=0;i<labels.size() && i<values.size();++i) {
            if (labels[i].source==values[i]) continue;
            labels[i].source = values[i];
            std::string caption = values[i];
            if (!kind && caption.starts_with('/')) caption.erase(0,1);
            if (kind && mir3::raster_text(caption,1,1,false,mir3::TextFont::normal,font_).text_width>358) {
                auto ansi = mir3::utf8_to_ansi(caption);
                caption = mir3::ansi_to_utf8(ansi.substr(0,58)+"...");
            }
            SetBagText(ui,labels[i].text,std::move(caption),600,16);
        }
    }
}

void Dialogs::Assets::DrawChatShortcutOverlay(cui_ctx* ui, const mir3_cui::ChatPanel& chat)
{
    if (!chatShortcutVisible_[0] && !chatShortcutVisible_[1]) return;
    const auto view = cui_viewport_size(ui);
    cui_overlay_begin(ui,{0,0,view.x,view.y});
    const struct { const Assets* assets; std::array<int,2> count, selected; bool expanded; } data{
        this,{int(chat.contacts.size()),int(chat.phrases.size())},{chat.contactIndex,chat.phraseIndex},chat.expanded};
    cui_canvas_paint(ui,CUI_FILL,CUI_FILL,[](cui_ctx* ui,cui_rect rect,void* value) {
        const auto& draw = *static_cast<decltype(data)*>(value);
        const HudPoint base(rect);
        const int top = draw.expanded ? 365 : 405;
        constexpr cui_color border{0,0,0,255}, shadow{10,10,10,255}, white{255,255,255,255};
        constexpr std::array<cui_color,2> background{{{200,100,0,180},{150,150,50,180}}};
        constexpr std::array<cui_color,2> highlight{{{255,0,0,120},{50,50,200,120}}};
        for (int kind=0;kind<2;++kind) {
            if (!draw.assets->chatShortcutVisible_[kind]) continue;
            const int width = kind && !draw.expanded ? 100 : 99;
            cui_draw_rect_premultiplied(ui,base.Rect(183,top,width,165),background[kind]);
            cui_draw_rect(ui,base.Rect(183,top,width,1),{},border);
            cui_draw_rect(ui,base.Rect(183,top+164,width,1),{},border);
            cui_draw_rect(ui,base.Rect(183,top+1,1,163),{},border);
            cui_draw_rect(ui,base.Rect(182+width,top+1,1,163),{},border);
            const auto& labels = draw.assets->chatShortcutLabels_[kind];
            for (int i=0;i<draw.count[kind] && i<static_cast<int>(labels.size());++i) {
                if (i==draw.selected[kind])
                    cui_draw_rect_premultiplied(ui,base.Rect(184,top+2+16*i,width-2,16),highlight[kind]);
                const auto& art = labels[i].text.image;
                if (!art.texture) continue;
                cui_draw_image(ui,art.texture,base.Rect(187,top+4+16*i,art.width,art.height),{0,0,1,1},{},shadow);
                cui_draw_image(ui,art.texture,base.Rect(188,top+5+16*i,art.width,art.height),{0,0,1,1},{},white);
            }
        }
    },&data,sizeof data);
    cui_box_end(ui);
}

void DrawDialogs(cui_ctx* ui, LoginClient& client, Dialogs* dialogs)
{
    auto* loginArt = dialogs ? dialogs->assets_.get() : nullptr;
    if (client.stage == Stage::GameSession && GetTickCount64()<client.mountInputUntil)
        cui_consume_mouse_press(ui,CUI_MOUSE_LEFT);
    const bool gameVisible = client.stage == Stage::GameSession || client.returnToCharactersAt;
    if (loginArt && !gameVisible) loginArt->LeavePortraitPicker(ui);
    const bool partyActive = gameVisible &&
        (client.party.open || client.party.notice.id || client.party.action != mir3_cui::PartyAction::None);
    if (loginArt && !partyActive) loginArt->LeaveParty();
    if (loginArt && !(gameVisible && client.quests.open)) loginArt->LeaveQuests();
    if (loginArt && !(gameVisible && (client.guild.open || client.guild.requested ||
                     client.guild.notice || client.guild.action != mir3_cui::GuildAction::None))) loginArt->LeaveGuild();
    if (loginArt && !client.audioOpen && !(gameVisible && client.serverOptions.open))
        loginArt->LeaveOptions(!gameVisible);
    if (loginArt && (!gameVisible ||
        (!client.inventory.open && !client.inventory.notice && !client.merchant.notice && !client.merchant.quantityOpen &&
         client.inventory.drop == mir3_cui::DropDialog::None)))
        loginArt->LeaveInventory(!gameVisible);
    if (loginArt && loginArt->Ready() && loginArt->DrawTransition(ui, client)) return;
    const bool characterPage = client.stage == Stage::ConnectingCharacters || client.stage == Stage::Characters;
    if (loginArt && characterPage && loginArt->SelectionReady(client)) {
        cui_box canvas{};
        canvas.width = CUI_FILL;
        canvas.height = CUI_FILL;
        cui_column_begin(ui, &canvas);
        loginArt->DrawSelection(ui, client);
        cui_box_end(ui);
        return;
    }
    if (loginArt) loginArt->LeaveSelection();
    if (loginArt && loginArt->GroupsReady() && client.stage == Stage::Groups) {
        cui_box canvas{};
        canvas.width = CUI_FILL;
        canvas.height = CUI_FILL;
        cui_column_begin(ui, &canvas);
        loginArt->DrawGroups(ui, client);
        cui_box_end(ui);
        return;
    }
    const bool signingIn = client.stage == Stage::Connecting || client.stage == Stage::SigningIn;
    const bool drawCredentials = loginArt && loginArt->Ready() && (client.stage == Stage::Login || signingIn);
    const bool drawServers = loginArt && loginArt->Ready() &&
        (client.stage == Stage::Servers || client.stage == Stage::Selecting || client.stage == Stage::Handoff);
    if (drawCredentials || drawServers) {
        const bool needsFallbackModal = drawCredentials && (client.audioOpen || client.settingsOpen || client.accountForm);
        cui_box canvas{};
        canvas.width = CUI_FILL;
        canvas.height = CUI_FILL;
        cui_column_begin(ui, &canvas);
        if (drawServers) loginArt->DrawServerList(ui, client);
        else loginArt->DrawCredentials(ui, client);
        cui_box_end(ui);
        if (!needsFallbackModal) return;
    } else if (loginArt) {
        loginArt->ResetCredentials();
    }

    cui_box page = {};
    page.width = CUI_FILL;
    page.height = CUI_FILL;
    page.pad = 28;
    page.gap = 16;
    if (client.stage == Stage::GameSession && cui_viewport_size(ui).x < 640) {
        page.pad = 16;
        page.gap = 10;
    }
    if (!drawCredentials) cui_column_begin(ui, &page);
    if (client.stage == Stage::Groups) {
        DrawServerGroups(ui, client);
        cui_box_end(ui);
        return;
    }
    if (client.stage == Stage::Characters) {
        DrawCharacters(ui, client);
        cui_box_end(ui);
        cui_dialog_opts modal = {};
        modal.width = 360;
        modal.persistent = true;
        modal.no_close_button = true;
        if (client.stage == Stage::Characters) DrawCharacterDialog(ui, client, modal);
        return;
    }
    if (gameVisible) {
        const bool inventoryWasOpen = client.inventory.open;
        const bool equipmentWasOpen = client.equipment.open;
        const bool nativeNpc = loginArt && loginArt->NpcReady() && client.npcDialogue.actor;
        const bool nativeShop = loginArt && loginArt->ShopReady() && client.shop.actor;
        const bool nativeInventory = loginArt && loginArt->InventoryReady();
        const bool nativeQuick = loginArt && loginArt->QuickReady();
        const bool nativeHud = loginArt && loginArt->GameHudReady();
        const bool nativeOptions = nativeHud && loginArt->OptionsReady();
        const bool fallbackOptionsOpen = !nativeOptions && client.serverOptions.open;
        const bool quickBlocksScene = nativeQuick && loginArt->QuickBlocksScene(ui,client);
        const bool inventoryBlocksScene = !nativeInventory || loginArt->InventoryBlocksScene(ui,client);
        const bool npcBlocksScene = !nativeNpc || loginArt->NpcBlocksScene(ui,client);
        const bool shopBlocksScene = !nativeShop || loginArt->ShopBlocksScene(ui,client);
        if (loginArt) {
            loginArt->PostMerchantResult(client);
            loginArt->PostTradeFeedback(client);
        }
        else client.merchantResults.clear();
        Dialogs::Assets::PostQuestNotices(client,loginArt);
        const bool merchantModal = client.merchant.notice || client.merchant.quantityOpen;
        const bool fading = client.returnToCharactersAt != 0;
        const bool portraitNotice = nativeHud && loginArt->PortraitNoticeOpen();
        const bool modal=fading || portraitNotice || client.showSignOut || client.chat.receptionOpen || fallbackOptionsOpen ||
            client.trade.prompt!=mir3_cui::TradePrompt::None ||
            client.audioOpen || client.party.notice.id || client.party.action!=mir3_cui::PartyAction::None ||
            client.guild.notice || client.guild.action!=mir3_cui::GuildAction::None ||
            client.inventory.notice || client.inventory.bindingShortcut || merchantModal ||
            (client.shop.actor && (!nativeShop || loginArt->ShopHasModal(client))) ||
            client.inventory.drop!=mir3_cui::DropDialog::None || !client.npcDialogue.prompt.command.empty();
        if (!modal && !client.chat.composing && !cui_wants_keyboard(ui)) {
            if (cui_shortcut(ui,0,CUI_KEY_P)) client.ToggleObjectOrders();
            if (cui_shortcut(ui,CUI_MOD_ALT,CUI_KEY_M)) client.ToggleMailbox();
        }
        bool independentEscape = false;
        const IndependentViewport independentView(ui);
        const auto mailSize = loginArt ? loginArt->IndependentButtonSize(1988) : cui_vec2{};
        const cui_rect mailIndicator{independentView.x+705*independentView.scale,independentView.y+440*independentView.scale,
            mailSize.x*independentView.scale,mailSize.y*independentView.scale};
        const bool hasMailIndicator = !modal && client.windows.mail.unreadNotice && !client.windows.Open();
        const auto pointer = cui_mouse_pos(ui);
        const bool overMailIndicator = hasMailIndicator && pointer.x>=mailIndicator.x && pointer.x<mailIndicator.x+mailIndicator.w &&
            pointer.y>=mailIndicator.y && pointer.y<mailIndicator.y+mailIndicator.h;
        const bool independentCovered = PrepareIndependentWindows(ui,client.windows,modal,independentEscape) || overMailIndicator;
        const bool overShop = nativeShop && loginArt->ShopBlocksScene(ui,client,true);
        const int shopWheel = overShop && !modal && !independentCovered
            ? -static_cast<int>(std::lround(cui_take_mouse_scroll(ui).y)) : 0;
        const int npcWheel = nativeNpc && !modal && !independentCovered && (!client.shop.actor || (nativeShop && !overShop)) &&
            loginArt->NpcBlocksScene(ui,client,true)
            ? -static_cast<int>(std::lround(cui_take_mouse_scroll(ui).y)) : 0;
        bool minimapCovered=false;
        bool utilityCovered=false, panelEscape=false, panelPointerCaptured=false;
        if (nativeHud) {
            const bool foreground = independentCovered || (nativeNpc && npcBlocksScene) || (nativeShop && shopBlocksScene);
            loginArt->PrepareMinimap(ui,client);
            minimapCovered=loginArt->HandleMinimapInput(ui,client,modal || foreground);
            utilityCovered=loginArt->HandleUtilityPanels(ui,client,minimapCovered,foreground,modal || independentEscape,panelEscape,panelPointerCaptured);
        }
        const bool partyCovered=!loginArt || !loginArt->PartyReady() || loginArt->PartyBlocksScene(ui);
        const bool questCovered=!loginArt || !loginArt->QuestReady() || loginArt->QuestBlocksScene(ui);
        const bool guildCovered=!loginArt || !loginArt->GuildReady() || loginArt->GuildBlocksScene(ui);
        if (nativeHud) {
            cui_box_end(ui);
            const bool covered=(client.party.open && partyCovered) || (client.quests.open && questCovered) ||
                (client.guild.open && guildCovered) || (client.inventory.open && inventoryBlocksScene) ||
                (client.npcDialogue.actor && npcBlocksScene) || (client.shop.actor && shopBlocksScene) ||
                quickBlocksScene || utilityCovered || independentCovered;
            loginArt->DrawGameHud(ui,client,covered,modal || independentEscape || (client.equipment.open && !loginArt->StatusReady()));
        } else {
            DrawGameChat(ui,client,partyCovered,questCovered,guildCovered,npcBlocksScene,shopBlocksScene,
                nativeInventory,inventoryBlocksScene,nativeQuick,quickBlocksScene,independentCovered || independentEscape);
            cui_box_end(ui);
        }
        const bool inventoryModal = merchantModal || (nativeInventory &&
            (client.inventory.notice || client.inventory.drop != mir3_cui::DropDialog::None));
        const bool bagOverlay = fading || portraitNotice || client.showSignOut || independentCovered || independentEscape || client.party.notice.id || client.guild.notice ||
            client.trade.prompt!=mir3_cui::TradePrompt::None ||
            client.party.action != mir3_cui::PartyAction::None || client.guild.action != mir3_cui::GuildAction::None ||
            client.chat.receptionOpen || fallbackOptionsOpen || client.audioOpen || (!nativeInventory && client.inventory.notice) ||
            client.inventory.bindingShortcut || (!inventoryModal && !client.npcDialogue.prompt.command.empty());
        const bool shopCoversPointer = client.shop.actor && shopBlocksScene;
        if (nativeInventory && !inventoryModal && (!nativeHud || !client.inventory.open))
            loginArt->DrawInventoryPanel(ui,client,!bagOverlay && !shopCoversPointer && !quickBlocksScene && !panelEscape,inventoryWasOpen);
        if (nativeQuick && !nativeHud)
            loginArt->DrawQuickPanel(ui,client,!bagOverlay && !inventoryModal && !shopCoversPointer && !panelEscape &&
                !(nativeNpc && npcBlocksScene));
        const bool nativeParty = loginArt && loginArt->PartyReady();
        const bool nativeGuild = loginArt && loginArt->GuildReady();
        const bool partyNotice = nativeParty && client.party.notice.id;
        const bool guildNotice = nativeGuild && client.guild.notice;
        const bool npcPrompt = nativeNpc && !client.npcDialogue.prompt.command.empty() &&
            !fading && !portraitNotice && !client.showSignOut && !inventoryModal;
        const bool shopOpen = client.shop.actor != 0;
        const bool fallbackNpc = client.npcDialogue.actor && !nativeNpc;
        const bool partyOpen = client.party.open || client.party.notice.id;
        const bool partyActionOpen = client.party.action!=mir3_cui::PartyAction::None;
        const bool guildActionOpen = client.guild.action!=mir3_cui::GuildAction::None;
        const bool guildOpen = client.guild.open || client.guild.requested || client.guild.notice || guildActionOpen;
        const bool settingsOpen = client.chat.receptionOpen || fallbackOptionsOpen || client.audioOpen ||
            client.trade.prompt!=mir3_cui::TradePrompt::None;
        const bool regularPanels = !partyNotice && !guildNotice && !npcPrompt && !shopOpen && !fallbackNpc && !settingsOpen;
        if (regularPanels) {
            if (partyOpen && !partyActionOpen && (!nativeHud || !nativeParty)) {
                if (nativeParty) {
                    loginArt->HandlePartyInput(ui,client,modal,true);
                    if (client.party.open) loginArt->DrawPartyPanel(ui,client);
                }
                else DrawParty(ui,client);
            } else if (guildOpen && !guildActionOpen && (!nativeHud || !nativeGuild)) {
                if (nativeGuild) {
                    loginArt->HandleGuildInput(ui,client,modal,true);
                    loginArt->DrawGuildPanel(ui,client);
                }
                else DrawGuild(ui,client);
            } else if (client.quests.open && (!nativeHud || !loginArt->QuestReady())) {
                if (loginArt && loginArt->QuestReady()) {
                    loginArt->HandleQuestInput(ui,client,modal,true);
                    if (client.quests.open) loginArt->DrawQuestPanel(ui,client);
                } else DrawQuests(ui,client);
            }
        }
        if (!npcPrompt && !fallbackNpc && !settingsOpen && !partyOpen && !guildOpen && !client.quests.open && !panelEscape &&
            (!shopOpen || nativeShop || merchantModal) &&
            (client.inventory.open || client.inventory.notice ||
                 client.inventory.drop != mir3_cui::DropDialog::None || client.merchant.notice || client.merchant.quantityOpen ||
                 client.equipment.open || inventoryWasOpen || equipmentWasOpen)) {
            cui_push_disabled(ui,utilityCovered || independentCovered || independentEscape || fading || client.showSignOut);
            DrawItemPanels(ui, client, inventoryWasOpen, equipmentWasOpen,nativeInventory,nativeHud && loginArt->StatusReady());
            cui_pop_disabled(ui);
        }
        if (nativeHud) {
            loginArt->DrawUtilityPanels(ui,client,inventoryModal);
            loginArt->DrawMinimapPanel(ui,client);
            loginArt->DrawTargetDetails(ui,false);
            if (loginArt->SkillsReady()) loginArt->DrawSkillShortcuts(ui,client);
        }
        if (nativeNpc) {
            const bool overlay = partyNotice || guildNotice || fading || portraitNotice || client.showSignOut || inventoryModal ||
                minimapCovered || panelPointerCaptured || independentCovered || independentEscape ||
                (shopOpen && shopBlocksScene && client.npcDialogue.prompt.command.empty());
            loginArt->DrawNpcPanel(ui,client,!overlay,npcWheel);
        }
        if (partyNotice) loginArt->DrawPartyPanel(ui,client,!nativeHud);
        else if (guildNotice) loginArt->DrawGuildPanel(ui,client,!nativeHud);
        else if (npcPrompt) return;
        else if (shopOpen) {
            if (nativeShop) loginArt->DrawShopPanel(ui,client,shopWheel,!fading && !client.showSignOut &&
                !inventoryModal && !minimapCovered && !panelPointerCaptured && !independentCovered && !independentEscape);
            else if (!inventoryModal) DrawNpcShop(ui,client);
        } else if (fallbackNpc) DrawNpcDialogue(ui,client);
        else if (client.chat.receptionOpen) DrawChatReception(ui,client);
        else if (!nativeOptions && client.serverOptions.open) DrawServerOptions(ui,client);
        else if (client.audioOpen) {
            if (nativeOptions) {
                loginArt->HandleOptionsInput(ui,client,fading || portraitNotice || client.showSignOut,true);
                loginArt->DrawOptionsPanel(ui,client);
            } else DrawAudioSettings(ui,client);
        }
        else if (partyActionOpen) {
            if (nativeParty) loginArt->DrawPartyPanel(ui,client,!nativeHud);
            else DrawParty(ui,client);
        } else if (guildActionOpen) {
            if (nativeGuild) loginArt->DrawGuildPanel(ui,client,!nativeHud);
            else DrawGuild(ui,client);
        }
        if (nativeInventory && inventoryModal) loginArt->DrawInventoryPanel(ui,client,!bagOverlay,inventoryWasOpen);
        if (nativeHud && !fading && !portraitNotice && !client.showSignOut && !partyNotice && !guildNotice && !inventoryModal) {
            loginArt->DrawTargetDetails(ui,true);
            loginArt->DrawChatShortcutOverlay(ui,client.chat);
            loginArt->DrawPanelHeld(ui,client);
            loginArt->DrawTradePrompt(ui,client);
        }
        if (fading && nativeHud) {
            loginArt->DrawGameExitFade(ui,client);
            return;
        }
        if (nativeHud && loginArt->PortraitNoticeOpen()) {
            loginArt->DrawPortraitNotice(ui,client);
            return;
        }
        if (nativeHud && client.showSignOut && loginArt->DrawGameExit(ui,client)) return;
        if (!fading && !client.showSignOut) {
            auto& windows = client.windows;
            if (hasMailIndicator) {
                if (mailSize.x>0) {
                    const IndependentPanel<Dialogs::Assets> indicator{
                        ui,mailIndicator,independentView.scale,loginArt,windows.pressedButtons[4]};
                    cui_overlay_begin(ui,mailIndicator);
                    cui_push_id(ui,"mail-indicator");
                    if (indicator.ButtonAt(0,0,1988,"æ–°ä¿¡ä»¶",true)) client.ToggleMailbox();
                    cui_pop_id(ui);
                    cui_box_end(ui);
                } else {
                    const auto view = cui_viewport_size(ui);
                    cui_overlay_begin(ui, {std::max(0.0f, view.x-95), std::max(0.0f, view.y-160), 84, 27});
                    if (Button(ui, "æ–°ä¿¡ä»¶")) client.ToggleMailbox();
                    cui_box_end(ui);
                }
            }
            if (windows.Open()) {
                const IndependentDialogTheme theme(ui);
                const IndependentViewport view(ui);
                const auto mouse = cui_mouse_pos(ui);
                int hit = -1;
                for (const int id : windows.stacking)
                    if (IndependentOpen(windows,id) && view.Hit(windows,id,mouse)) hit = id;
                const auto stacking = windows.stacking;
                const auto drawItem = [&](unsigned icon,float height) {
                    if (loginArt) loginArt->DrawIndependentItem(ui,icon,height);
                };
                for (const int id : stacking) {
                    if (!IndependentOpen(windows,id)) continue;
                    const auto bounds = view.Bounds(windows,id);
                    const IndependentPanel<Dialogs::Assets> panel{ui,bounds,view.scale,loginArt,windows.pressedButtons[id]};
                    cui_overlay_begin(ui,bounds);
                    cui_push_id_int(ui,id+700);
                    cui_push_disabled(ui,modal || windows.dragging >= 0 || (hit >= 0 && hit != id));
                    if (loginArt) loginArt->DrawIndependentSkin(ui,id,bounds);
                    else {
                        cui_box background = {};
                        background.width = background.height = CUI_FILL;
                        background.bg = CUI_COLOR_SURFACE_RAISED;
                        cui_box_begin(ui,&background);
                        cui_box_end(ui);
                    }
                    switch (id) {
                    case 0:
                        panel.At(0,0,284,348,[&] { if (loginArt) loginArt->DrawInspectionEquipment(ui,windows.profile,bounds); });
                        panel.TextAt(81,17,128,15,windows.profile.name);
                        panel.TextAt(163,62,112,15,windows.profile.guild);
                        if (panel.ButtonAt(248,313,1221,"å…³é—­")) CloseIndependentWindow(windows,id);
                        break;
                    case 1:
                        DrawObjectOrders(panel,client,[&] { if (loginArt) loginArt->DrawObjectOrderMap(ui,client,165*view.scale); });
                        break;
                    case 2: DrawMarket(panel,client,drawItem); break;
                    case 3: DrawMarketRegistration(panel,client,drawItem); break;
                    case 4:
                        if (loginArt && (windows.mail.reading || windows.mail.composing || windows.mail.folder)) {
                            const int mode = windows.mail.composing ? 1 : windows.mail.reading ? 0 : 2;
                            panel.At(1,51,440,260,[&] { loginArt->DrawMailModeSkin(ui,mode,260*view.scale); });
                        }
                        DrawMailbox(panel,client,drawItem);
                        break;
                    }
                    cui_pop_disabled(ui);
                    cui_pop_id(ui);
                    cui_box_end(ui);
                }
            }
        }
        if (!client.showSignOut) return;
    } else if (!drawCredentials) {
        cui_heading(ui, "Mir3 Client", 1);
        cui_spacer(ui);
        cui_box_end(ui);
    }

    cui_dialog_opts modal = {};
    modal.width = 340;
    modal.persistent = true;
    modal.no_close_button = true;

    bool open = true;
    switch (client.stage) {
    case Stage::Groups:
    case Stage::Characters:
        break;
    case Stage::Login: {
        if (client.accountForm) {
            if (loginArt) loginArt->DrawAccountForm(ui,client);
        } else if (client.audioOpen) {
            if (loginArt && loginArt->OptionsReady()) {
                loginArt->HandleOptionsInput(ui,client,false,true);
                loginArt->DrawOptionsPanel(ui,client);
            } else DrawAudioSettings(ui,client);
        } else if (!drawCredentials && !client.settingsOpen && !client.status.empty()) {
            modal.description = client.status.c_str();
            if (cui_dialog_begin(ui, "Sign-in notice", &open, &modal)) {
                cui_dialog_footer_begin(ui);
                const bool dismiss = Button(ui, "OK", true) || cui_key_pressed(ui, CUI_KEY_ENTER) ||
                    cui_key_pressed(ui, CUI_KEY_SPACE);
                cui_dialog_footer_end(ui);
                cui_dialog_end(ui);
                if (dismiss) client.DismissNotice();
            }
        } else if (client.settingsOpen && cui_dialog_begin(ui, "Gateway settings", &client.settingsOpen, &modal)) {
            Input(ui, "Gateway IPv4 address", client.host, sizeof client.host);
            Input(ui, "Port", client.port, sizeof client.port);
            cui_checkbox_ex(ui, "Use legacy server numbering", &client.legacy, nullptr);
            cui_dialog_footer_begin(ui);
            if (Button(ui, "Done", true)) client.settingsOpen = false;
            cui_dialog_footer_end(ui);
            cui_dialog_end(ui);
        }
        break;
    }
    case Stage::Servers:
    case Stage::Handoff: {
        const bool handingOff = client.stage == Stage::Handoff;
        if (client.billing.message) {
            const auto description = BillingText(client.billing);
            modal.description = description.c_str();
            if (cui_dialog_begin(ui, "Account notice", &open, &modal)) {
                cui_dialog_footer_begin(ui);
                if (Button(ui, "OK", true) || cui_key_pressed(ui, CUI_KEY_ENTER) ||
                    cui_key_pressed(ui, CUI_KEY_SPACE))
                    client.billing = {};
                cui_dialog_footer_end(ui);
                cui_dialog_end(ui);
            }
            break;
        }
        if (!cui_dialog_begin(ui, "Select a server", &open, &modal)) break;
        cui_push_disabled(ui, handingOff);
        if (!client.servers.empty()) {
            cui_scroll_opts list = {};
            list.id = "servers";
            list.height = CUI_PX(188);
            list.width = CUI_FILL;
            list.gap = 6;
            cui_scroll_begin(ui, &list);
            for (size_t i = 0; i < client.servers.size(); ++i) {
                cui_push_id_int(ui, static_cast<int>(i));
                const auto& server = client.servers[i];
                static const char* const loads[] = {"", " (Low load)", " (Medium load)", " (High load)"};
                const std::string label = server.label + loads[server.load < 4 ? server.load : 0] + "###server";
                if (Button(ui, label.c_str())) client.Select(server);
                cui_pop_id(ui);
            }
            cui_scroll_end(ui);
        } else {
            cui_secondary(ui, client.status.c_str());
        }
        cui_dialog_footer_begin(ui);
        if (Button(ui, "Back")) client.Reset();
        cui_dialog_footer_end(ui);
        cui_pop_disabled(ui);
        if (handingOff) cui_secondary(ui, "Opening character selection...");
        cui_dialog_end(ui);
        break;
    }
    case Stage::Complete:
        if (cui_dialog_begin(ui, "Character gateway", &open, &modal)) {
            cui_code(ui, client.status.c_str());
            cui_dialog_footer_begin(ui);
            if (Button(ui, "Back to sign-in")) client.Reset();
            cui_dialog_footer_end(ui);
            cui_dialog_end(ui);
        }
        break;
    case Stage::ConnectingGame:
    case Stage::AuthenticatingGame:
    case Stage::GameSession:
    case Stage::SigningOutGame:
    case Stage::GameDisconnected: {
        const bool disconnected = client.stage == Stage::GameDisconnected;
        modal.description = client.showSignOut ? "End the current game session?" : client.status.c_str();
        if (cui_dialog_begin(ui, client.showSignOut ? "Sign out" : "Game gateway", &open, &modal)) {
            const auto address = client.gameGateway.host + ':' + std::to_string(client.gameGateway.port);
            cui_code(ui, address.c_str());
            cui_label(ui, client.gameCharacter.c_str());
            cui_dialog_footer_begin(ui);
            if (client.showSignOut) {
                if (Button(ui, "Cancel")) {
                    client.showSignOut = false;
                    client.quitGameRequested = false;
                } else if (Button(ui, "Confirm", true)) {
                    if (client.quitGameRequested) cui_app_quit(nullptr);
                    else client.SignOutGame();
                }
            } else if (disconnected) {
                if (Button(ui, "Back to sign-in")) client.Reset();
            } else if (client.stage != Stage::SigningOutGame && Button(ui, "Cancel")) {
                client.Reset();
            }
            cui_dialog_footer_end(ui);
            cui_dialog_end(ui);
        }
        break;
    }
    default:
        modal.description = client.status.c_str();
        if (cui_dialog_begin(ui, "Connecting", &open, &modal)) {
            cui_dialog_footer_begin(ui);
            if (Button(ui, "Cancel")) client.Reset();
            cui_dialog_footer_end(ui);
            cui_dialog_end(ui);
        }
        break;
    }
}

} // namespace mir3_client

#endif // MIR3_LEGACY_CLIENT
