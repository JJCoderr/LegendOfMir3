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
