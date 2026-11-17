#pragma once
#include "ServerGroups.h"
class CSvrSelProcess : public CWHDefProcess {
public:
    void OnConnectToServer() override {}
    char* OnMessageReceive(CHAR*) override {return nullptr;}
    bool Load(bool offline=false);
    void DeleteProc();
    void RenderScene(INT elapsed);
    LRESULT DefMainWndProc(HWND window,UINT message,WPARAM wp,LPARAM lp) override;
    bool InLogin() const {return m_inLogin;}
private:
    friend struct ServerGroupsPreview;
    void Enter(const LoginConfiguration& config);
    void Move(POINT point);
    void Down(POINT point,DWORD tick);
    void Up(POINT point);
    void Drag(int y);
    RECT ScrollThumb();
    RECT FrameBounds(int frame,int x,int y);
    void DrawLabel(const ServerGroups::Group& group);
    void DrawText(const std::string& text,RECT box,COLORREF color);
    CWHWilImageData m_image;
    ServerGroups m_groups;
    POINT m_pointer={-1,-1};
    bool m_exitPressed=false,m_dragging=false,m_offline=false,m_inLogin=false,m_exitRequested=false;
    DWORD m_scrollTick=0;
};
