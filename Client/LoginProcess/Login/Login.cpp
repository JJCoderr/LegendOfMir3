#include "stdafx.h"
#include "Common/LegacyPanel.h"
#include "../LoginVisual.h"

void CLogin::Create(CWHWilImageData* image)
{
    m_image=image;m_values[0].clear();m_values[1].clear();
    m_field=0;m_pressed=-1;m_editorVisible=true;m_pointer={};
    SyncEditor();
}
void CLogin::ChangeField(int target)
{
    // MIR3 403F30 stores the opposite field, including repeated clicks.
    char text[LoginLayout::InputBytes+1]={};
    GetWindowTextA(g_xChatEditBox.GetSafehWnd(),text,sizeof(text));
    m_values[!target]=text;m_field=target;m_editorVisible=true;
}
void CLogin::SyncEditor()
{
    HWND edit=g_xChatEditBox.GetSafehWnd();
    if(!m_editorVisible) {ShowWindow(edit,SW_HIDE);SetFocus(g_xMainWnd.GetSafehWnd());return;}
    const RECT box=LoginLayout::Inputs[m_field];
    g_xChatEditBox.MoveScaled(g_xMainWnd.m_rcWindow.left+box.left,g_xMainWnd.m_rcWindow.top+box.top,
        box.right-box.left,box.bottom-box.top,TRUE);
    g_xChatEditBox.SetLimitText(LoginLayout::InputBytes);
    SendMessage(edit,EM_SETPASSWORDCHAR,m_field?'*':0,0);
    SetWindowTextA(edit,m_values[m_field].c_str());
    g_xChatEditBox.m_szInputMsg[0]=0;
    ShowWindow(edit,SW_SHOW);SetFocus(edit);g_xChatEditBox.SetSelectAll();
}
void CLogin::SetEditorVisible(bool visible)
{
    if(visible==m_editorVisible) return;
    if(m_editorVisible && !visible && m_field<2) {
        char text[LoginLayout::InputBytes+1]={};
        GetWindowTextA(g_xChatEditBox.GetSafehWnd(),text,sizeof(text));m_values[m_field]=text;
    }
    m_editorVisible=visible;SyncEditor();
}
CLogin::Action CLogin::SubmitEditor(bool mouse)
{
    ChangeField(!m_field);
    if(!mouse && (m_field!=0 || m_values[0].empty() || m_values[1].empty())) {SyncEditor();return None;}
    if(!mouse) m_field=2;
    m_editorVisible=false;SyncEditor();
    return Submit;
}
HRESULT CLogin::OnKeyDown(WPARAM key, LPARAM)
{
    if(key==VK_TAB) {ChangeField(!m_field);SyncEditor();}
    else if(key==VK_RETURN) return SubmitEditor(false);
    return None;
}
int CLogin::Hit(POINT point)
{
    for(int i=0;i<4;++i) if(m_image->NewSetIndex(LoginLayout::Frames[i])) {
        const auto& size=*m_image->m_lpstNewCurrWilImageInfo;
        const POINT at=LoginLayout::Buttons[i];RECT box={at.x,at.y,at.x+size.shWidth,at.y+size.shHeight};
        if(PtInRect(&box,point)) return i;
    }
    return -1;
}
HRESULT CLogin::OnButtonDown(WPARAM, LPARAM position)
{
    m_pointer={(short)LOWORD(position),(short)HIWORD(position)};
    for(int i=0;i<2;++i) if(PtInRect(&LoginLayout::Inputs[i],m_pointer)) {ChangeField(i);SyncEditor();return None;}
    m_pressed=Hit(m_pointer);return None;
}
HRESULT CLogin::OnButtonUp(WPARAM, LPARAM position)
{
    m_pointer={(short)LOWORD(position),(short)HIWORD(position)};m_pressed=-1;
    const int hit=Hit(m_pointer);
    if(hit<0) return None;
    g_xSound.PlayLobbyEffect(LoginLayout::ClickSound);
    return hit==0?SubmitEditor(true):Action(hit+1);
}
LRESULT CLogin::OnMouseMove(WPARAM, LPARAM position)
{
    m_pointer={(short)LOWORD(position),(short)HIWORD(position)};return 0;
}
void CLogin::ResetPassword()
{
    m_values[1].clear();m_field=1;m_editorVisible=true;m_pressed=-1;SyncEditor();
}
void CLogin::Render(INT)
{
    if(!m_fIsActive) return;
    DrawLegacyPanel(LoginLayout::ButtonBar,RGB(50,50,60),100,RGB(100,100,100));
    for(int i=0;i<4;++i) {
        const POINT at=LoginLayout::Buttons[i];
        DrawSelectionFrame(*m_image,LoginLayout::Frames[i]+(m_pressed==i),at.x,at.y,SelectionBlend::Normal,false);
    }
    if(Hit(m_pointer)==0) {
        m_image->NewSetIndex(LoginLayout::LoginButton);const auto& size=*m_image->m_lpstNewCurrWilImageInfo;
        DrawLegacyPanel({459,436,459+size.shWidth,436+size.shHeight},RGB(150,100,50),50,RGB(200,150,100));
    }
    DrawSelectionFrame(*m_image,LoginLayout::Fields,100,436,SelectionBlend::Normal,false);
    for(int i=0;i<2;++i) if(m_field!=i) {
        std::string text=i?std::string(m_values[1].size(),'*'):m_values[0];
        LoginVisual::Text(text,LoginLayout::Inputs[i].left,440,RGB(255,255,255),100);
    }
    if(m_editorVisible) {
        const RECT box=LoginLayout::Inputs[m_field];
        g_xChatEditBox.MoveScaled(g_xMainWnd.m_rcWindow.left+box.left,g_xMainWnd.m_rcWindow.top+box.top,
            box.right-box.left,box.bottom-box.top,TRUE);
    }
}
