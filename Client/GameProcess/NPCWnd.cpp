/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/

#include "StdAfx.h"

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