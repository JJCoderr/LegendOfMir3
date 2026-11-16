#pragma once

// Login/selection notice layout from Mir3's Godot client. Uses GameInter art.
class CLoginNotice {
public:
    CLoginNotice();
    BOOL CreateWnd(CWHWilImageData* image, BOOL, INT, INT, INT, INT);
    BOOL ShowMessageBox(const CHAR* message, INT type, INT ignoredFrame);
    BOOL HideMessageBox();
    void SetOrigin(int x, int y) { m_offset.x = x-140; m_offset.y = y-110; }
    BOOL IsActive() const { return m_active; }
    HRESULT OnButtonDown(POINT point);
    HRESULT OnButtonUp(POINT point);
    VOID MoveWnd(POINT) {} // The reference notice is fixed, not draggable.
    BOOL RenderMessageBox(INT);
private:
    int Hit(POINT point) const;
    void Draw(int frame, int x, int y);
    CWHWilImageData* m_image;
    BOOL m_active;
    int m_type, m_pressed;
    POINT m_offset;
    char m_text[512];
};
