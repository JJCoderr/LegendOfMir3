#ifndef	_LOGIN_TYPE_DEF
#define _LOGIN_TYPE_DEF


typedef struct	tagServerList
{
	char Caption[MAX_PATH];
	char Name[MAX_PATH];
}ServerList,* PServerList;

typedef struct tagIMAGEINFO
{
	UINT	nImgIdx;
	LONG	Left;
	LONG	Top;
}	_LOGINIMAGEINFO,* _PLOGINIMAGEINFO;

typedef struct tagSERVERBUTTON
{
	RECT		rcBtn;
	CHAR		szCaption[MAX_PATH];
	CHAR		szName[MAX_PATH];
	COLORREF	dwTxtColor;
	COLORREF	dwBkColor;
	INT			nAlpha;
} _SERVERBUTTON,*	_PSERVERBUTTON,	**_PPSERVERBUTTON;

typedef struct tagINPUTSTATE
{
	UINT	nID;
	LONG	Left;
	LONG	Top;
	LONG	Width;
	LONG	Height;
	INT		nHelptxt;
	INT		nSize;
	CHAR	szData[MAX_PATH];		
} INPUTSTATE;


#endif //	_LOGIN_TYPE_DEF