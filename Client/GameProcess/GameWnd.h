#ifndef _CGAMEWND_H
#define _CGAMEWND_H


class CGameWnd
{ 
public:
	INT				m_nID;					// 窗口管理器用于区分此窗口的标识符。
	RECT			m_rcImgWnd;				// 窗口图像的区域. 
	RECT			m_rcWnd;				// 窗口的屏幕区域. 

	INT				 m_nFrameImgIdx;		// 图像相关变量. 
	CWHWilImageData* m_pxWndImage;

	BOOL			m_bActive;				// 窗口是否激活。 
	BOOL			m_bFocused; 
	BOOL			m_bMoving;				// 窗口是否正在移动.
	BOOL			m_bCanMove;				// 窗口是否固定.

	INT				m_nWidth;
	INT				m_nHeight;

	POINT			m_bGrabbedMousePos;
	// 3D测试
	D3DVERTEX		m_avBoard[4];

public:
	__inline RECT GetGameWndRect()
	{
		return m_rcWnd;
	}
	__inline BOOL GetGameWndActive()
	{
		return m_bActive;
	}

	__inline CGameWnd()	
	{ 
		Init();
	}

	~CGameWnd()
	{
		Destroy();
	}

	virtual VOID Init();
	virtual VOID Destroy();
	virtual VOID ShowGameWnd();


	VOID ResetWndImg(INT nIdx);
	VOID CreateGameWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, BOOL bCanMove, INT nStartX, INT nStartY, INT nWidth = 0, INT nHeight = 0);
	VOID DestroyGameWnd();
	VOID SetGameWndActive(BOOL bActive);
	VOID SetGameWndFocused(BOOL bFocused);
	VOID SetReadyMove(POINT ptMouse);
	VOID MoveGameWnd(POINT ptMouse);
};


#endif // _CGAMEWND_H