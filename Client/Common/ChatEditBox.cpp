/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/



#include "StdAfx.h"
#include "Common/AnsiInput.h"
#include "Common/ChatShortcuts.h"
#include "Common/LegacyPanel.h"
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
    if(g_bProcState==_GAME_PROC) {
        if(g_xGameProc.m_xNotice.IsActive() && (nMsg==WM_CHAR || nMsg==WM_KEYDOWN || nMsg==WM_KEYUP || nMsg==WM_SYSKEYDOWN || nMsg==WM_SYSKEYUP || nMsg==WM_SYSCHAR)) {
            if(nMsg==WM_CHAR && wParam==VK_RETURN) g_xGameProc.OnKeyDown(wParam,lParam);
            return 0;
        }
        auto& shortcuts=ChatShortcuts();
        if(!g_xGameProc.m_xInterface.m_xMsgBox.IsActive() && !g_xGameProc.m_xInterface.m_xGuildEditor.m_bActive && shortcuts.Input(nMsg,wParam)) return 0;
    }
    if(g_bProcState==_CHAR_SEL_PROC) {
        if(nMsg==WM_SYSKEYDOWN) return 0;
        if((nMsg==WM_KEYDOWN && (wParam==VK_ESCAPE || (wParam>=VK_F1 && wParam<=VK_F12))) ||
           (nMsg==WM_CHAR && (wParam==VK_RETURN || wParam==VK_TAB ||
             (wParam==VK_SPACE && g_xChrSelProc.HasNotice())))) {
            g_xChrSelProc.OnKeyDown(wParam,lParam);
            return 0;
        }
    }
    auto& ui=g_xGameProc.m_xInterface;
    if(g_bProcState==_GAME_PROC && ui.m_xMsgBox.IsActive()) {
        if(nMsg==WM_SYSKEYDOWN) return 0;
        if((nMsg==WM_KEYDOWN && (wParam==VK_ESCAPE || (wParam>=VK_F1 && wParam<=VK_F12))) ||
           (nMsg==WM_CHAR && (wParam==VK_RETURN || wParam==VK_TAB))) {
            ui.OnKeyDown(wParam,lParam);
            return 0;
        }
    }

	switch (nMsg)
	{
        case WM_SYSKEYDOWN:
            if(wParam==VK_F10 || ((lParam & (1L<<29)) && (wParam=='Q' || wParam=='X'))) {
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
    m_layout={x-target.left,y-target.top,x-target.left+width,y-target.top+height};m_hasLayout=true;
    const int displayWidth = target.right - target.left;
    const int displayHeight = target.bottom - target.top;
    if(!displayWidth || !displayHeight) return;
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

void CChatEdit::Relocate()
{
    if(!m_hChatEdit || !m_hasLayout) return;
    const auto& display=g_xMainWnd.m_rcWindow;
    MoveScaled(display.left+m_layout.left,display.top+m_layout.top,
        m_layout.right-m_layout.left,m_layout.bottom-m_layout.top,TRUE);
}

CChatShortcuts& ChatShortcuts() { static CChatShortcuts value;return value; }
void CChatShortcuts::Clear()
{
    contacts.clear();phrases.clear();privatePrefix.clear();storage.clear();
    contactSelected=phraseSelected=0;controlDown=altDown=false;
}
void CChatShortcuts::Load(const std::string& base)
{
    Clear();storage=base;if(base.empty()) return;
    for(bool phrase:{false,true}) {
        FILE* file=fopen((storage+(phrase?".mlt":".slt")).c_str(),"rb");if(!file) continue;
        fseek(file,0,SEEK_END);const long size=ftell(file);rewind(file);
        if(size<0 || size>10240) {fclose(file);continue;}
        std::string data(size,'\0');const auto read=fread(data.data(),1,data.size(),file);fclose(file);data.resize(read);
        auto& values=phrase?phrases:contacts;size_t start=0;
        while(values.size()<Capacity) {
            const auto end=data.find('/',start);if(end==std::string::npos || end-start>1023) break;
            values.push_back(data.substr(start,end-start));start=end+1;
        }
    }
}
void CChatShortcuts::Save(bool phrase) const
{
    if(storage.empty()) return;
    FILE* file=fopen((storage+(phrase?".mlt":".slt")).c_str(),"wb");if(!file) return;
    for(const auto& value:phrase?phrases:contacts) {fwrite(value.data(),1,value.size(),file);fputc('/',file);}
    fclose(file);
}
bool CChatShortcuts::Remember(const std::string& input)
{
    if(input.empty() || (input[0]!='/' && input[0]!='#')) return false;
    const bool phrase=input[0]=='#';auto& values=phrase?phrases:contacts;
    std::string value=input.substr(1);
    if(!phrase) {
        const auto end=input.find(' ',1);
        if(input.size()>1) privatePrefix="/"+input.substr(1,end==std::string::npos?end:end-1)+" ";
        value=privatePrefix.empty()?std::string{}:privatePrefix.substr(1);
    }
    if(std::find(values.begin(),values.end(),value)==values.end()) {
        values.push_back(value);if(values.size()>Capacity) values.erase(values.begin());Save(phrase);
    }
    return phrase;
}
bool CChatShortcuts::Select(bool phrase,int value,bool relative,std::string& output)
{
    const auto& values=phrase?phrases:contacts;auto& selected=phrase?phraseSelected:contactSelected;
    if(relative) selected=value<0?max(0,selected-1):min(int(values.size())-1,selected+1);
    else if(value>=0 && value<int(values.size())) selected=value;
    if(selected<0 || selected>=int(values.size())) return false;
    if(!phrase) privatePrefix="/"+values[selected];
    output=phrase?values[selected]:privatePrefix;return true;
}
bool CChatShortcuts::Input(UINT message,WPARAM key)
{
    const bool down=message==WM_KEYDOWN || message==WM_SYSKEYDOWN;
    const bool up=message==WM_KEYUP || message==WM_SYSKEYUP;
    if(down || up) {
        if(key==VK_CONTROL) {controlDown=down;return false;}
        if(key==VK_MENU) {altDown=down;return true;}
    }
    if(GetFocus()!=g_xChatEditBox.GetSafehWnd() || (!controlDown && !altDown)) return false;
    const bool number=key>='0' && key<='9';
    if((message==WM_CHAR || message==WM_SYSCHAR) && number) return true;
    if(!down || (!number && key!=VK_UP && key!=VK_DOWN)) return false;
    std::string value;
    if(Select(!controlDown,number?(key=='0'?9:int(key-'1')):(key==VK_UP?-1:1),!number,value)) {
        const HWND edit=g_xChatEditBox.GetSafehWnd();SetWindowTextA(edit,value.c_str());
        SendMessage(edit,EM_SETSEL,-1,-1);SetFocus(edit);
    }
    return true;
}
void CChatShortcuts::Draw(bool expanded)
{
    auto& ui=g_xGameProc.m_xInterface;
    if(GetFocus()!=g_xChatEditBox.GetSafehWnd() || ui.m_xMsgBox.IsActive() || ui.m_xGuildEditor.m_bActive) return;
    const int top=expanded?365:405;
    for(bool phrase:{false,true}) {
        const auto& values=phrase?phrases:contacts;
        if(values.empty() || !(phrase?altDown:controlDown)) continue;
        const int width=phrase && !expanded?100:99;
        DrawLegacyPanel(RECT{PanelLeft,top,PanelLeft+width,top+PanelHeight},phrase?RGB(150,150,50):RGB(200,100,0),180,RGB(0,0,0));
        const int selected=phrase?phraseSelected:contactSelected;
        if(selected>=0 && selected<int(values.size())) DrawLegacyPanel(RECT{184,top+2+RowHeight*selected,182+width,top+2+RowHeight*(selected+1)},phrase?RGB(50,50,200):RGB(255,0,0),120,0,false);
        HDC dc;auto* buffer=g_xMainWnd.GetBackBuffer();if(FAILED(buffer->GetDC(&dc))) continue;
        const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};HFONT font=g_xMainWnd.CreateGameFont(face,9);auto old=SelectObject(dc,font);SetBkMode(dc,TRANSPARENT);
        for(int i=0;i<int(values.size());++i) {
            std::string caption=values[i];SIZE size;GetTextExtentPoint32A(dc,caption.c_str(),int(caption.size()),&size);
            if(phrase && size.cx>358) caption=caption.substr(0,58)+"...";
            SetTextColor(dc,RGB(10,10,10));TextOutA(dc,187,top+4+RowHeight*i,caption.c_str(),int(caption.size()));
            SetTextColor(dc,RGB(255,255,255));TextOutA(dc,188,top+5+RowHeight*i,caption.c_str(),int(caption.size()));
        }
        SelectObject(dc,old);DeleteObject(font);buffer->ReleaseDC(dc);
    }
}

void CChatShortcuts::SetIdentity(const std::string& character,const std::string& server)
{
    const auto valid=[](const std::string& name) {
        if(name.empty() || name=="." || name=="..") return false;
        for(size_t i=0;i<name.size();++i) {
            if(IsDBCSLeadByteEx(CP_ACP,BYTE(name[i])) && i+1<name.size()) {++i;continue;}
            if(strchr("/\\:*?\"<>|",name[i])) return false;
        }
        return true;
    };
    if(!valid(character) || !valid(server)) {Clear();return;}
    Load("./Data/"+character+"("+server+")");
}
