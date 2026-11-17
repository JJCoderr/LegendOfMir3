#ifndef _NOTICEBOX_H_
#define _NOTICEBOX_H_
#include <string>
#include <vector>
class CNoticeBox {
public:
    enum { Frame=1250,AcceptFrame=1251,AcceptPressedFrame=1252 };
    BOOL Load(CWHWilImageData* image);
    BOOL IsActive() const { return active; }
    BOOL IsInited() const { return image!=nullptr; }
    BOOL ShowMessageBox(char* text,int frame=Frame);
    BOOL HideMessageBox();
    BOOL DestoryMessageBox();
    HRESULT OnButtonDown(POINT point);
    HRESULT OnButtonUp(POINT point);
    bool OnKeyDown(WPARAM key);
    void MoveWnd(POINT point);
    BOOL RenderMessageBox(INT elapsed);
    RECT Bounds() const { return bounds; }
    const std::vector<std::string>& Lines() const { return lines; }
private:
    CWHWilImageData* image=nullptr;
    RECT bounds={};POINT grab={};
    bool active=false,dragging=false,pressed=false;
    int frame=Frame,lineHeight=15;
    std::vector<std::string> lines;
    RECT ButtonBounds();
    void LayoutText(const char* text);
};
#endif
