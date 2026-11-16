#ifndef _TYPEDEF_TEXT_H
#define _TYPEDEF_TEXT_H

typedef struct tag_CMTxtBtn
{
	COLORREF nColor;			// 普通状态下的颜色
	COLORREF cColor;			// 被 Click 时的颜色
	CHAR	Txt[70];			// 显示到 Main 的 Txt
	CHAR	sTxt[70];			// 以 Sub 显示的 Txt
	RECT	xRect;				// Top 必须始终为 0
	INT		nLinePos;			// 决定 Top.  Top 随滚轮与窗口移动而始终可变
	CHAR	Param[70];
	BOOL	bIsClicked;
} CMTxtBtn;

typedef struct tag_CMText
{
	COLORREF	fColor;				// Text Color
	BOOL		bDrawCenter;		// DrawCenter
	INT			nPosx;				// Draw Position x
	INT			nLine;				// Draw Line indicater	:Zerobase
	CHAR*		pText;				// Text
} CMText;

#endif _TYPEDEF_TEXT_H