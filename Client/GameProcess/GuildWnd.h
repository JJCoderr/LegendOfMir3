#ifndef _CGUILDWND_H
#define _CGUILDWND_H
#include <string>
#include <vector>


class CGuildWnd : public CGameWnd
{ 
private:
protected:
    int ButtonAt(POINT point);

public:
    bool m_manager = false;
    std::string m_name;
    std::vector<std::string> m_pages[3];
    std::vector<std::string> m_editable[2];
    int m_tab = 0, m_pressed = -1, m_first = 0;
    bool m_scrollCaptured = false;
    DWORD m_scrollTick = 0;
    bool ScrollInput(int kind, POINT point);
	CGuildWnd();
	~CGuildWnd();

	virtual VOID Init();
	virtual VOID Destroy();

	VOID	CreateGuildWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove);
	VOID	ShowGuildWnd();
public:
//  ÏûÏ¢º¯Êý.
	BOOL	OnLButtonDown(POINT ptMouse);
	BOOL	OnLButtonUp(POINT ptMouse);
	VOID	OnMouseMove(POINT ptMouse);

	VOID	OnScrollDown();
	VOID	OnScrollUp();
};


#endif // _CGUILDWND_H