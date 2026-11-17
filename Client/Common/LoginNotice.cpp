#include "stdafx.h"
#include "LoginNotice.h"

CLoginNotice::CLoginNotice() : m_image(NULL), m_active(FALSE), m_type(MSG_BTN_OK), m_pressed(0) {
    m_text[0] = 0;
    m_offset.x = m_offset.y = 0;
}

BOOL CLoginNotice::CreateWnd(CWHWilImageData* image, BOOL, INT, INT, INT, INT) {
    m_image = image;
    return image && image->NewSetIndex(1240) && image->NewSetIndex(1243);
}

BOOL CLoginNotice::ShowMessageBox(const CHAR* message, INT type, INT) {
    lstrcpynA(m_text, message, sizeof(m_text));
    m_type = type;
    m_selected = (type & 127) == MSG_BTN_YESNO ? CLICK_YES : CLICK_OK;
    m_pressed = 0;
    m_active = TRUE;
    return TRUE;
}

BOOL CLoginNotice::HideMessageBox() {
    m_active = FALSE;
    m_pressed = 0;
    return TRUE;
}

int CLoginNotice::Hit(POINT point) const {
    if (!m_active || m_type == Waiting) return CLICK_NONE;
    point.x -= m_offset.x; point.y -= m_offset.y;
    if ((m_type & 127) == MSG_BTN_YESNO) {
        RECT yes = {181,298,261,330}, no = {374,298,454,330};
        if (PtInRect(&yes, point)) return CLICK_YES;
        if (PtInRect(&no, point)) return CLICK_NO;
    } else {
        RECT ok = {277,298,357,330};
        if (PtInRect(&ok, point)) return CLICK_OK;
    }
    return CLICK_NONE;
}

HRESULT CLoginNotice::OnKeyDown(WPARAM key) {
    if (!m_active || m_type == Waiting) return CLICK_NONE;
    if (key == VK_TAB && (m_type & 127) == MSG_BTN_YESNO)
        m_selected = m_selected == CLICK_YES ? CLICK_NO : CLICK_YES;
    if (key == VK_RETURN || key == VK_SPACE) return m_selected;
    return CLICK_NONE;
}

HRESULT CLoginNotice::OnButtonDown(POINT point, bool selectWithMouse) { m_pressed = Hit(point); if(selectWithMouse && (m_pressed==CLICK_YES || m_pressed==CLICK_NO)) m_selected=m_pressed; return 0; }
void CLoginNotice::OnMouseMove(POINT point) { if(Hit(point)!=m_pressed) m_pressed=CLICK_NONE; }
HRESULT CLoginNotice::OnButtonUp(POINT point, bool releaseOnly) {
    const int hit = Hit(point);
    const int result = releaseOnly || hit == m_pressed ? hit : CLICK_NONE;
    m_pressed = 0;
    return result;
}

void CLoginNotice::Draw(int frame, int x, int y) {
    if (!m_image || !m_image->NewSetIndex(frame)) return;
    const NEWWILIMAGEINFO& info = *m_image->m_lpstNewCurrWilImageInfo;
    g_xMainWnd.DrawWithImageForComp(x+m_offset.x,y+m_offset.y,info.shWidth,info.shHeight,(WORD*)m_image->m_pbCurrImage);
}

BOOL CLoginNotice::RenderMessageBox(INT) {
    if (!m_active) return FALSE;
    Draw(m_type == Waiting ? WaitingFrame : 1240,140,110);
    if ((m_type & 127) == MSG_BTN_YESNO) {
        Draw(m_pressed == CLICK_YES ? 1242 : 1241,181,298);
        Draw(m_pressed == CLICK_NO ? 1246 : 1245,374,298);
    } else if (m_type != Waiting) Draw(m_pressed == CLICK_OK ? 1244 : 1243,277,298);
    // Match native_text.cpp dialog layout: measured wrapping and vertical centering.
    const bool confirmation = (m_type & 127) == MSG_BTN_YESNO && m_compactConfirmation;
    RECT rect = confirmation ? RECT{183,194,461,269} : RECT{163,194,471,289};
    OffsetRect(&rect,m_offset.x,m_offset.y);
    auto* surface = g_xMainWnd.GetBackBuffer();
    HDC dc;
    if (FAILED(surface->GetDC(&dc))) return FALSE;
    const char songti[] = {char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font = g_xMainWnd.CreateGameFont(songti,9,0,FW_NORMAL);
    HGDIOBJ previous = SelectObject(dc,font);
    SetBkMode(dc,TRANSPARENT);
    SetTextColor(dc,0xc8faff);
    SIZE extent = {};
    GetTextExtentPoint32A(dc,m_text,(int)strlen(m_text),&extent);
    const int saved = SaveDC(dc);
    IntersectClipRect(dc,rect.left,rect.top,rect.right,rect.bottom);
    if (extent.cx >= rect.right - rect.left) {
        RECT measured = rect;
        const int height = DrawTextA(dc,m_text,-1,&measured,DT_CALCRECT | DT_WORDBREAK);
        rect.top += (rect.bottom - rect.top - height) / 2;
        rect.bottom = rect.top + height;
        DrawTextA(dc,m_text,-1,&rect,DT_WORDBREAK);
    } else {
        DrawTextA(dc,m_text,-1,&rect,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    RestoreDC(dc,saved);
    SelectObject(dc,previous);
    DeleteObject(font);
    surface->ReleaseDC(dc);
    return TRUE;
}
