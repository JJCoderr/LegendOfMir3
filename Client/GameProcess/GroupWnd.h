#ifndef _CGROUPWND_H
#define _CGROUPWND_H

class CGroupMember
{
public:
	char		szMemberName[32];
};

class CGroupWnd : public CGameWnd
{ 
    int m_pressed = -1;
    int ButtonAt(POINT point);
public:
	CPDLList<CGroupMember>	m_xGroupList;

public:
	CGroupWnd();
	~CGroupWnd();
	virtual VOID Init();
	virtual VOID Destroy();

	VOID	CreateGroupWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove);
	VOID	ShowGroupWnd();

public:
//  ÏûÏ¢º¯Êý.
	BOOL	OnLButtonDown(POINT ptMouse);
	BOOL	OnLButtonUp(POINT ptMouse);
	VOID	OnMouseMove(POINT ptMouse);

	VOID	OnScrollDown();
	VOID	OnScrollUp();
};

#endif // _CGROUPWND_H