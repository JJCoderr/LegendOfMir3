#pragma once

// Login/selection notice layout from Mir3's Godot client. Uses GameInter art.
class CLoginNotice {
public:
    static constexpr int Waiting = 0;
    static constexpr int WaitingFrame = 1248;
    CLoginNotice();
    BOOL CreateWnd(CWHWilImageData* image, BOOL, INT, INT, INT, INT);
    BOOL ShowMessageBox(const CHAR* message, INT type, INT ignoredFrame);
    BOOL HideMessageBox();
    void SetOrigin(int x, int y, bool compactConfirmation = true) { m_offset.x = x-140; m_offset.y = y-110; m_compactConfirmation = compactConfirmation; }
    const char* Text() const { return m_text; }
    BOOL IsActive() const { return m_active; }
    HRESULT OnKeyDown(WPARAM key);
    HRESULT OnButtonDown(POINT point);
    HRESULT OnButtonUp(POINT point, bool releaseOnly=false);
    VOID MoveWnd(POINT) {} // The reference notice is fixed, not draggable.
    BOOL RenderMessageBox(INT);
private:
    int Hit(POINT point) const;
    void Draw(int frame, int x, int y);
    CWHWilImageData* m_image;
    BOOL m_active;
    int m_type, m_pressed;
    int m_selected = CLICK_YES;
    POINT m_offset;
    bool m_compactConfirmation = true;
    char m_text[512];
};
