/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/



#include "StdAfx.h"
#include "Common/AnsiInput.h"
#include "Common/LoginMessages.h"

static const char* ChatInputFontFace()
{
    static CLoginMessages captions;
    static const bool loaded = captions.Load("./CmList.dat");
    return captions.Get(1074).c_str();
}



static CChatEdit*	g_xChatEdit = NULL;


/******************************************************************************************************************

	CChatEdit Functions

*******************************************************************************************************************/


CChatEdit::CChatEdit()
{
	m_hChatEdit = NULL;
	g_xChatEdit	= this;
	m_hFontChatEdit = NULL;
	m_dwFontColor = RGB(255, 0, 255);
	ZeroMemory(m_szInputMsg, MAX_PATH);
}



CChatEdit::~CChatEdit()
{
	DestroyDialog();
}



BOOL CChatEdit::DestroyDialog()
{
	if ( m_hFontChatEdit != NULL )
	{
		DeleteObject(m_hFontChatEdit);
		m_hFontChatEdit = NULL; 
	}

	DestroyWindow(m_hChatEdit);
	return TRUE;
}



INT_PTR CALLBACK ChatEditDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return g_xChatEdit->ChatEditProc(hWnd, uMsg, wParam, lParam);
}



LRESULT CALLBACK CChatEdit::ChatEditProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	switch (nMsg)
	{
        case WM_SYSKEYDOWN:
            if(wParam==VK_F10) {
                SetFocus(g_xMainWnd.GetSafehWnd());
                SendMessage(g_xMainWnd.GetSafehWnd(),WM_SYSKEYDOWN,wParam,lParam);return 0;
            }
            break;
		case WM_KEYDOWN:
		{
			switch ( (INT)wParam )
			{
			case VK_F1:
			case VK_F2:
			case VK_F3:
			case VK_F4:
			case VK_F5:
			case VK_F6:
			case VK_F7:
			case VK_F8:
            case VK_F9:case VK_F10:case VK_F11:case VK_F12:
			case VK_ESCAPE:
				{
					HWND m_hMainWindow = g_xMainWnd.GetSafehWnd();
					SetFocus(m_hMainWindow);
					SendMessage(m_hMainWindow, WM_KEYDOWN, (INT)wParam, lParam);
					return 0L;
				}
			}
			break;
		}
		case WM_CHAR:
		{
			switch ( (INT)wParam )
			{
			case VK_RETURN:
				{
                    auto& ui=g_xGameProc.m_xInterface;
                    if (ui.m_xMsgBox.IsActive() && ui.m_nIdxMsgBox==7) { ui.FinishNotice(CLICK_YES,false); return 0; }
					GetWindowText(hWnd, m_szInputMsg, MAX_PATH);
					SetWindowText(hWnd, "");
					HWND m_hMainWindow = g_xMainWnd.GetSafehWnd();
					SetFocus(m_hMainWindow);
					ShowWindow(m_hChatEdit, SW_HIDE);

					if ( m_szInputMsg )
					{
						if ( m_szInputMsg[0] == '@' )
							SendMessage(m_hMainWindow, _WM_USER_MSG_INPUTTED, TRUE, NULL);
						else
						{
							SendMessage(m_hMainWindow, _WM_USER_MSG_INPUTTED, NULL, NULL);
						}
					}

					return 0L;
				}
			case VK_TAB:
				{
					HWND m_hMainWindow = g_xMainWnd.GetSafehWnd();
					SetFocus(m_hMainWindow);
					SendMessage(m_hMainWindow, WM_KEYDOWN, (INT)wParam, lParam);
					return 0L;
				}
			}
			break;
		}
		default:
			break;
	}
	return CallWindowProc(m_WndProcChatEdit, hWnd, nMsg, wParam, lParam);
}



BOOL CChatEdit::Create(HINSTANCE hInstance, HWND hWndParent, INT nX, INT nY, INT nXsize, INT nYSize)
{
	m_hChatEdit = CreateWindowEx(0, _T("EDIT"), "", WS_POPUP | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL,
								 nX, nY, nXsize, nYSize,
								 hWndParent, NULL, hInstance, NULL);	  
	m_WndProcChatEdit = (WNDPROC)SetWindowLong(m_hChatEdit, GWL_WNDPROC, (LONG)ChatEditDlgProc);

	// 若字体已创建, 则 Clear 以便重新创建
	if ( m_hFontChatEdit != NULL )
	{
		DeleteObject(m_hFontChatEdit); 
		m_hFontChatEdit = NULL; 
	}

	m_hFontChatEdit = CreateFont(-12, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
								 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
								 DEFAULT_QUALITY, DEFAULT_PITCH, ChatInputFontFace());

	SendMessage(m_hChatEdit, WM_SETFONT, (WPARAM)m_hFontChatEdit, MAKELPARAM(TRUE, 0));
	// 输入编辑窗口的最大输入字符个数定义.
	SendMessage(m_hChatEdit, EM_LIMITTEXT, (WPARAM)90, 0);
	ShowWindow(m_hChatEdit, SW_HIDE);
    UpdateWindow(m_hChatEdit);
	return TRUE;
}



BOOL CChatEdit::SetLimitText(INT nLimitText)
{
    m_byteLimit=nLimitText; m_previous.clear();
	if ( m_hChatEdit )
	{
		SendMessage(m_hChatEdit, EM_LIMITTEXT, (WPARAM)nLimitText, 0);
		return TRUE;
	}
	return FALSE;
}
void CChatEdit::Changed()
{
    if (m_updating) return;
    std::string text(GetWindowTextLengthA(m_hChatEdit)+1,'\0'); GetWindowTextA(m_hChatEdit,&text[0],int(text.size())); text.resize(strlen(text.c_str()));
    m_previous=LimitAnsiEdit(m_previous,text,m_byteLimit);
    if (m_previous!=text) { m_updating=true; SetWindowTextA(m_hChatEdit,m_previous.c_str()); m_updating=false; }
}
VOID CChatEdit::MoveScaled(INT x, INT y, INT width, INT height, BOOL repaint)
{
    const RECT& target = g_xMainWnd.m_rcWindow;
    const int displayWidth = target.right - target.left;
    const int displayHeight = target.bottom - target.top;
    const int logicalWidth = g_xMainWnd.m_stDisplayInfo.wWidth;
    const int logicalHeight = g_xMainWnd.m_stDisplayInfo.wHeight;
    MoveWindow(m_hChatEdit, target.left + MulDiv(x - target.left, displayWidth, logicalWidth),
        target.top + MulDiv(y - target.top, displayHeight, logicalHeight),
        MulDiv(width, displayWidth, logicalWidth), MulDiv(height, displayHeight, logicalHeight), repaint);
    const int fontHeight = MulDiv(12, displayHeight, logicalHeight);
    if (m_fontHeight != fontHeight) {
        HFONT font = CreateFont(-fontHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, ChatInputFontFace());
        SendMessage(m_hChatEdit, WM_SETFONT, (WPARAM)font, TRUE);
        DeleteObject(m_hFontChatEdit); m_hFontChatEdit = font; m_fontHeight = fontHeight;
    }
}
