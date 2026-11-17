#pragma once
inline bool CheckInputWindowMove()
{
    const HWND edit=g_xChatEditBox.GetSafehWnd(),main=g_xMainWnd.GetSafehWnd();
    RECT before,window;GetWindowRect(edit,&before);GetWindowRect(main,&window);
    const RECT display=g_xMainWnd.m_rcWindow;
    const HWND focus=GetFocus();const BOOL visible=IsWindowVisible(edit);
    std::vector<char> text(GetWindowTextLengthA(edit)+1);GetWindowTextA(edit,text.data(),int(text.size()));
    SetWindowPos(main,nullptr,window.left+20,window.top+15,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
    RECT after;GetWindowRect(edit,&after);const RECT current=g_xMainWnd.m_rcWindow;
    std::vector<char> moved(GetWindowTextLengthA(edit)+1);GetWindowTextA(edit,moved.data(),int(moved.size()));
    const bool ok=after.left-current.left==before.left-display.left && after.top-current.top==before.top-display.top &&
        after.right-after.left==before.right-before.left && after.bottom-after.top==before.bottom-before.top &&
        IsWindowVisible(edit)==visible && GetFocus()==focus && text==moved;
    SetWindowPos(main,nullptr,window.left,window.top,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
    return ok;
}

inline bool CheckInputWindowResize(bool minimize)
{
    const HWND edit=g_xChatEditBox.GetSafehWnd(),main=g_xMainWnd.GetSafehWnd();
    RECT before,window;GetWindowRect(edit,&before);GetWindowRect(main,&window);
    const RECT display=g_xMainWnd.m_rcWindow;const BOOL visible=IsWindowVisible(edit);
    std::vector<char> text(GetWindowTextLengthA(edit)+1);GetWindowTextA(edit,text.data(),int(text.size()));
    if(minimize) {ShowWindow(main,SW_MINIMIZE);ShowWindow(main,SW_RESTORE);}
    else SetWindowPos(main,nullptr,0,0,window.right-window.left+160,window.bottom-window.top+120,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
    RECT after;GetWindowRect(edit,&after);const RECT current=g_xMainWnd.m_rcWindow;
    const int width=display.right-display.left,height=display.bottom-display.top;
    const int nextWidth=current.right-current.left,nextHeight=current.bottom-current.top;
    const bool aligned=abs(after.left-current.left-MulDiv(before.left-display.left,nextWidth,width))<=1 &&
        abs(after.top-current.top-MulDiv(before.top-display.top,nextHeight,height))<=1 &&
        abs(after.right-after.left-MulDiv(before.right-before.left,nextWidth,width))<=1 &&
        abs(after.bottom-after.top-MulDiv(before.bottom-before.top,nextHeight,height))<=1;
    std::vector<char> moved(GetWindowTextLengthA(edit)+1);GetWindowTextA(edit,moved.data(),int(moved.size()));
    SetWindowPos(main,nullptr,window.left,window.top,window.right-window.left,window.bottom-window.top,SWP_NOZORDER|SWP_NOACTIVATE);
    RECT restored;GetWindowRect(edit,&restored);
    return aligned && EqualRect(&before,&restored) && IsWindowVisible(edit)==visible && text==moved;
}

// Panel editors are laid out by their normal per-frame update.
template<class Layout> bool CheckPanelEditorLayout(HWND edit,Layout layout)
{
    const HWND main=g_xMainWnd.GetSafehWnd();
    RECT before,window;GetWindowRect(edit,&before);GetWindowRect(main,&window);
    const RECT display=g_xMainWnd.m_rcWindow;
    const HFONT font=(HFONT)SendMessage(edit,WM_GETFONT,0,0);
    LOGFONTA oldFont={};GetObjectA(font,sizeof(oldFont),&oldFont);
    std::vector<char> text(GetWindowTextLengthA(edit)+1);GetWindowTextA(edit,text.data(),int(text.size()));
    ShowWindow(main,SW_MINIMIZE);layout();
    RECT minimized;GetWindowRect(edit,&minimized);
    const bool preserved=EqualRect(&before,&minimized) && (HFONT)SendMessage(edit,WM_GETFONT,0,0)==font;
    ShowWindow(main,SW_RESTORE);layout();
    SetWindowPos(main,nullptr,window.left+20,window.top+15,window.right-window.left+160,window.bottom-window.top+120,SWP_NOZORDER|SWP_NOACTIVATE);layout();
    RECT after;GetWindowRect(edit,&after);const RECT current=g_xMainWnd.m_rcWindow;
    const int width=display.right-display.left,height=display.bottom-display.top;
    const int nextWidth=current.right-current.left,nextHeight=current.bottom-current.top;
    LOGFONTA nextFont={};GetObjectA((HFONT)SendMessage(edit,WM_GETFONT,0,0),sizeof(nextFont),&nextFont);
    const bool scaled=abs(after.left-current.left-MulDiv(before.left-display.left,nextWidth,width))<=1 &&
        abs(after.top-current.top-MulDiv(before.top-display.top,nextHeight,height))<=1 &&
        abs(after.right-after.left-MulDiv(before.right-before.left,nextWidth,width))<=1 &&
        abs(after.bottom-after.top-MulDiv(before.bottom-before.top,nextHeight,height))<=1 &&
        abs(nextFont.lfHeight-MulDiv(oldFont.lfHeight,nextHeight,height))<=1;
    SetWindowPos(main,nullptr,window.left,window.top,window.right-window.left,window.bottom-window.top,SWP_NOZORDER|SWP_NOACTIVATE);layout();
    RECT restored;GetWindowRect(edit,&restored);
    std::vector<char> value(GetWindowTextLengthA(edit)+1);GetWindowTextA(edit,value.data(),int(value.size()));
    return preserved && scaled && EqualRect(&before,&restored) && IsWindowVisible(edit) && text==value;
}
