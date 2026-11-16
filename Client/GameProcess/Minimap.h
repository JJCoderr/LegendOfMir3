#pragma once
#include <vector>
class CMinimap {
    CWHWilImageData maps,fullMaps;
    int frame=-1;
    bool full=false;
public:
    bool visible=false,pending=false,translucent=false,large=false,dragging=false,releaseCaptured=false,showMonsters=true;
    POINT position={672,0},dragOffset={},player={};
    int width=0,height=0;
    DWORD replyTime=0;
    std::vector<POINT> monsters,green,orange;
    void Init();
    void Destroy();
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
