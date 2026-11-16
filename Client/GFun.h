#ifndef _GFUN_H_
#define _GFUN_H_



// 汇集了多个 Class 中均可使用的函数和结构体的公用文件。



typedef struct tagCHAT
{
	DWORD	dwFontColor;
	DWORD	dwBackColor;
	CHAR	pszChat[MAX_PATH];
}CHAT, *LPCHAT;

typedef struct tagTEXTUREFILE
{
	WORD					wFileIdx;
	WORD					wImgIdx;
	LPDIRECTDRAWSURFACE7	lpddsTextr;
}TEXTUREFILE, *LPTEXTUREFILE;




DWORD GetChatColor(BYTE bIdx);

DWORD GetUserNameColor(BYTE bIdx);

#endif // _GFUN_H_
