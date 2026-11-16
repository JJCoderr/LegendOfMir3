#ifndef _CGAMEBTN_H
#define _CGAMEBTN_H

class CGameBtn
{ 
public:
	RECT			m_rcBtn;
    RECT m_hitRect = {};
    bool m_customHit = false;
    void SetHitRect(int x, int y, int width, int height) {
        SetRect(&m_hitRect,x,y,x+width,y+height);
        m_customHit = true;
    }
    bool Hit(POINT point) const {
        return PtInRect(m_customHit ? &m_hitRect : &m_rcBtn,point) != FALSE;
    }


	CWHWilImageData* m_pxBtnImage;
	INT				m_nFocusImgIdx;			// 图像相关变量. 
	INT				m_nClickImgIdx;			

	BYTE			m_bBtnType;				// 0 : 不具有焦点状态. 1: 有焦点.
	BYTE			m_bBtnState;			// 0 : 保持原样... 1: 鼠标焦点状态. 2: 鼠标点击状态.


public:
	__inline RECT GetGameBtnRect()
	{
		return m_rcBtn;
	}
	__inline VOID SetBtnState(BYTE bState)
	{
		m_bBtnState = bState;
		if ( m_bBtnType == _BTN_TYPE_NOFOCUS && m_bBtnState == _BTN_STATE_FOCUS )
		{
			m_bBtnState = _BTN_STATE_NORMAL;
		}
	}
	__inline BYTE GetGameBtnState()
	{
		return m_bBtnState;
	}

	__inline CGameBtn()	
	{ 
		Init();
	}

	~CGameBtn()
	{
		Destroy();
	}

	VOID Init();
	VOID Destroy();

	VOID ChangeRect(INT nStartX, INT nStartY);
	VOID CreateGameBtn(CWHWilImageData* pxBtnImage, INT nFocusImgIdx, INT nClickImgIdx, INT nStartX, INT nStartY, BYTE bBtnType = _BTN_TYPE_FOCUS);
	virtual VOID ShowGameBtn();

	VOID ResetGameBtn(INT nFocusImgIdx, INT nClickImgIdx);
	
	virtual VOID OnMouseMove(POINT ptMouse);
	virtual BOOL OnLButtonDown(POINT ptMouse);
	virtual BOOL OnLButtonUp(POINT ptMouse);
};



class CMsgBoxBtn : public CGameBtn
{
public:
	CMsgBoxBtn();
	~CMsgBoxBtn();
	virtual VOID ShowGameBtn();
};



class CScrlBar
{
public:
	CScrlBar();
	~CScrlBar();

public:
	CWHWilImageData*	m_pxScrlImage;		// 滚动条图像文件指针。
	INT					m_nScrlImgIdx;		// 滚动条图像。

	FLOAT				m_fScrlRate;		// 0.0f~1.0f 之间。滚动条的比例。
	FLOAT				m_fPageRate;		

	INT					m_nPageLine;

	BOOL				m_bGrapsed;			// 判断在滚动按钮区域内是否因 Mouse LButtonDown 而处于按下状态.

	WORD				m_wScrlWidth;
	WORD				m_wScrlHeight;

	WORD				m_wScrlBtnHeight;

	RECT				m_rcScrlRgn;		// 滚动条的区域.
	RECT				m_rcScrlBtnRgn;		// 滚动条中按钮的区域.

	__inline FLOAT GetScrlRate()
	{
		return m_fScrlRate;
	}

	__inline VOID Init()
	{
		m_pxScrlImage		= NULL;
		m_nScrlImgIdx		= 0;

		m_fPageRate			= 0.0f;
		m_fScrlRate			= 0.0f;

		m_nPageLine			= 0;

		m_bGrapsed			= FALSE;

		m_wScrlWidth		= 0;
		m_wScrlHeight		= 0;

		m_wScrlBtnHeight	= 0;

		SetRect(&m_rcScrlRgn, 0, 0, 0, 0);
		SetRect(&m_rcScrlBtnRgn, 0, 0, 0, 0);
	}

	VOID	CreateScrlBar(CWHWilImageData* pxImage, INT ImgIdx, INT nPageLine, WORD wScrlWidth, WORD wScrlHeight, WORD wScrlBtnHeight);
	VOID	ShowScrlBar(INT nX, INT nY, INT nFstPos, INT nCounter);
	BOOL	OnMouseMove(POINT ptMouse);
	BOOL	OnLButtonDown(POINT ptMouse);
	BOOL	OnLButtonUp(POINT ptMouse);
};



#endif // _CGAMEBTN_H