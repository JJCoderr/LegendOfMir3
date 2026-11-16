#include "stdafx.h"
#include "LoginNotice.h"

CLoginNotice::CLoginNotice() : m_image(NULL), m_active(FALSE), m_type(MSG_BTN_OK), m_pressed(0) {
    m_text[0] = 0;
}

BOOL CLoginNotice::CreateWnd(CWHWilImageData* image, BOOL, INT, INT, INT, INT) {
    m_image = image;
    return image && image->NewSetIndex(1240) && image->NewSetIndex(1243);
}

BOOL CLoginNotice::ShowMessageBox(const CHAR* message, INT type, INT) {
    lstrcpynA(m_text, message, sizeof(m_text));
    m_type = type;
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
    if (!m_active) return CLICK_NONE;
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

HRESULT CLoginNotice::OnButtonDown(POINT point) { m_pressed = Hit(point); return 0; }
HRESULT CLoginNotice::OnButtonUp(POINT point) {
    const int hit = Hit(point);
    const int result = hit == m_pressed ? hit : CLICK_NONE;
    m_pressed = 0;
    return result;
}

void CLoginNotice::Draw(int frame, int x, int y) {
    if (!m_image || !m_image->NewSetIndex(frame)) return;
    const NEWWILIMAGEINFO& info = *m_image->m_lpstNewCurrWilImageInfo;
    g_xMainWnd.DrawWithImageForComp(x,y,info.shWidth,info.shHeight,(WORD*)m_image->m_pbCurrImage);
}

BOOL CLoginNotice::RenderMessageBox(INT) {
    if (!m_active) return FALSE;
    Draw(1240,140,110);
    if ((m_type & 127) == MSG_BTN_YESNO) {
        Draw(m_pressed == CLICK_YES ? 1242 : 1241,181,298);
        Draw(m_pressed == CLICK_NO ? 1246 : 1245,374,298);
    } else Draw(m_pressed == CLICK_OK ? 1244 : 1243,277,298);
    const char* next = m_text;
    for (int row = 0; row < 5 && *next; ++row) {
        char line[51] = {};
        int length = 0;
        while (*next && *next != '\n') {
            const int bytes = IsDBCSLeadByteEx(CP_ACP,(BYTE)*next) && next[1] ? 2 : 1;
            if (length + bytes > 50) break;
            memcpy(line + length,next,bytes); length += bytes; next += bytes;
        }
        if (*next == '\n') ++next;
        const int left = (m_type & 127) == MSG_BTN_YESNO ? 183 : 163;
        g_xMainWnd.PutsHan(NULL,left,194 + row * 18,0xc8faff,0,line);
    }
    return TRUE;
}
