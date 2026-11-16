#ifndef _COPTIONWND_H
#define _COPTIONWND_H


class COptionWnd : public CGameWnd
{ 
private:
protected:
    int ButtonAt(POINT point);
    const char* m_audioOptionsPath = nullptr;

public:
    bool m_values[21] = {};
    bool m_confirmed[7] = {};
    int RequestServerOption(int index);
    unsigned ReceiveServerOptions(DWORD flags,WORD mode);
    int m_pressed=-1, m_hover=-1;
    int m_musicLevel=0, m_effectsLevel=0, m_attackMode=0;
    int m_audioDrag=-1;
    bool AudioInput(int kind, POINT point);
    void LoadAudioOptions(const char* path);
    bool SaveAudioOptions();
    bool CanAttackHuman(DWORD nameColor,bool shift) const {return shift || m_values[8] || nameColor==255 || nameColor==27631;}
	COptionWnd();
	~COptionWnd();

	virtual VOID Init();
	virtual VOID Destroy();

	VOID	CreateOptionWnd(INT nID, CWHWilImageData* pxWndImage, INT nFrameImgIdx, INT nStartX, INT nStartY, INT nWidth, INT nHeight, BOOL bCanMove);
	VOID	ShowOptionWnd();

	VOID	SetStatusBtnInit();

public:
//  ÏûÏ¢º¯Êý.
	BOOL	OnLButtonDown(POINT ptMouse);
	BOOL	OnLButtonUp(POINT ptMouse);
	VOID	OnMouseMove(POINT ptMouse);

	VOID	OnScrollDown();
	VOID	OnScrollUp();
};


#endif // _COPTIONWND_H