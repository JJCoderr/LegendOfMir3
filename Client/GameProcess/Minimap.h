#pragma once
#include <vector>
class CMinimap {
    CWHWilImageData maps,fullMaps;
    int frame=-1;
    bool full=false;
    void Request();
    std::vector<DWORD> pixels;
    const char* optionsPath=nullptr;
public:
    bool visible=false,pending=false,translucent=false,large=false,dragging=false,releaseCaptured=false,showMonsters=true;
    bool portraitMode=false,portraitPending=false,portraitVisible=false;
    int portraitBlink=0;
    void LoadOptions(const char* path);
    bool TogglePortrait(DWORD now);
    RECT PortraitSource() const;
    POINT PortraitOrigin() const;
    POINT PortraitMarker(POINT cell,bool self) const;
    void ShowPortrait();
    POINT position={672,0},dragOffset={},player={};
    int width=0,height=0;
    DWORD replyTime=0;
    std::vector<POINT> monsters,green,orange;
    void Init();
    void Destroy();
    void MapChanged();
    int Size() const { return large?256:128; }
    RECT Bounds() const;
    POINT Source() const;
    POINT ButtonPosition(int index) const;
    int HintAt(POINT point,POINT& anchor,CWHWilImageData& skin);
    void Place();
    bool Toggle(DWORD now,DWORD cooldown);
    bool Receive(int number,DWORD now);
    void Unavailable(DWORD now);
    bool Pointer(int kind,POINT point,bool ctrl);
    void Show(CWHWilImageData& skin,DWORD now);
};
