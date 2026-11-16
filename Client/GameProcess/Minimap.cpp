#include "StdAfx.h"
#include "Common/LegacyPanel.h"
#include "Common/LoginMessages.h"
void CMinimap::Init()
{
    Destroy(); maps.NewLoad("./Data/Mmap.wil");fullMaps.NewLoad("./Data/Fmmap.wil");
}
void CMinimap::Destroy()
{
    maps.Destroy();fullMaps.Destroy();frame=-1;width=height=0;replyTime=0;
    visible=pending=translucent=large=dragging=releaseCaptured=false;position={672,0};
    monsters.clear();green.clear();orange.clear();
}
RECT CMinimap::Bounds() const { return RECT{position.x,position.y,position.x+min(Size(),width),position.y+min(Size(),height)}; }
POINT CMinimap::Source() const
{
    const int w=min(Size(),width),h=min(Size(),height);
    return POINT{max(0,min(width-w,int((__int64)player.x*3/2)-w/2)),max(0,min(height-h,int(player.y)-h/2))};
}
POINT CMinimap::ButtonPosition(int index) const { const RECT area=Bounds();return POINT{area.right-38+19*index,area.bottom-19}; }
void CMinimap::Place() {position={800-min(Size(),width),0};dragging=false;}
bool CMinimap::Toggle(DWORD now,DWORD cooldown)
{
    if(replyTime && now-replyTime<=cooldown) return false;
    if(visible) {visible=false;dragging=false;return false;}
    pending=true;
    _TDEFAULTMESSAGE message;fnMakeDefMessage(&message,1033,0,0,0,0);g_xClientSocket.SendPacket(&message,NULL);return true;
}
bool CMinimap::Receive(int number,DWORD now)
{
    if(number<=0 || number>65535) return false;
    const int id=number-1;auto& archive=id<1000?maps:fullMaps;
    if(!archive.NewSetIndex(id<1000?id:id-1000)) return false;
    const auto info=*archive.m_lpstNewCurrWilImageInfo;
    frame=id<1000?id:id-1000;full=id>=1000;width=info.shWidth;height=info.shHeight;replyTime=now;
    if(pending) visible=true;pending=false;Place();return true;
}
bool CMinimap::Pointer(int kind,POINT point,bool ctrl)
{
    if(kind==2 && releaseCaptured) {releaseCaptured=dragging=false;return true;}
    if(!visible) return false;
    const RECT area=Bounds();const bool inside=PtInRect(&area,point)!=FALSE;
    const POINT a=ButtonPosition(0),b=ButtonPosition(1);
    const RECT alpha={a.x,a.y,a.x+19,a.y+19},resize={b.x,b.y,b.x+19,b.y+19};
    if(kind==0 && ctrl && inside) {dragOffset={point.x-position.x,point.y-position.y};dragging=releaseCaptured=true;}
    else if(kind==0 && PtInRect(&alpha,point)) {translucent=!translucent;releaseCaptured=true;}
    else if(kind==0 && PtInRect(&resize,point)) {large=!large;Place();releaseCaptured=true;}
    else if(kind==1 && dragging && ctrl && inside) position={point.x-dragOffset.x,point.y-dragOffset.y};
    else if(kind==1 || !inside) return false;
    return true;
}
int CMinimap::HintAt(POINT point,POINT& anchor,CWHWilImageData& skin)
{
    for(int i=1;i>=0;--i) {
        const int id=i?(large?1485:1484):(translucent?1482:1483);
        if(!skin.NewSetIndex(id)) continue;
        const auto info=*skin.m_lpstNewCurrWilImageInfo;const POINT p=ButtonPosition(i);
        const RECT hit={p.x,p.y,p.x+info.shWidth,p.y+info.shHeight};
        if(PtInRect(&hit,point)) {anchor={hit.right,hit.bottom};return i?(large?1803:1801):(translucent?1802:1800);}
    }
    return 0;
}
void CMinimap::Show(CWHWilImageData& skin,DWORD now)
{
    if(!visible || frame<0) return;
    const RECT area=Bounds();const POINT source=Source();
    DrawSelectionFrame(full?fullMaps:maps,frame,area.left-source.x,area.top-source.y,SelectionBlend::Alpha,false,translucent?175:255,&area,150);
    DrawLegacyPanel(area,0,0,RGB(100,100,100));
    const int px=area.left-source.x+int((__int64)player.x*3/2)-2,py=area.top-source.y+player.y-2;
    const auto fill=[&](RECT box,COLORREF color) {RECT clipped;if(IntersectRect(&clipped,&box,&area)) DrawLegacyPanel(clipped,color,255,0,false);};
    const auto markers=[&](const std::vector<POINT>& cells,COLORREF color,bool nearby) {
        for(const auto& cell:cells) {
            const int x=area.left-source.x+int(((__int64)cell.x*3-2)/2),y=area.top-source.y+cell.y-1;
            if(nearby && (abs(x-px)>=Size()/2 || abs(y-py)>=Size()/2)) continue;
            fill(RECT{x-int(nearby),y-int(nearby),x+2,y+2},color);
        }
    };
    if(showMonsters) markers(monsters,RGB(255,255,0),false);
    markers(green,RGB(100,250,100),true);markers(orange,RGB(250,150,50),true);
    if(now%801<500) {
        RECT clipped,box={px,py,px+4,py+4};
        if(IntersectRect(&clipped,&box,&area)) DrawLegacyPanel(clipped,0,0,RGB(100,250,100));
    }
    DrawSelectionFrame(skin,1486,area.left,area.top,SelectionBlend::Normal,false);
    DrawSelectionFrame(skin,1487,area.right-32,area.top,SelectionBlend::Normal,false);
    DrawSelectionFrame(skin,1488,area.left,area.bottom-35,SelectionBlend::Normal,false);
    for(int i=0;i<2;++i) {const POINT p=ButtonPosition(i);DrawSelectionFrame(skin,i?(large?1485:1484):(translucent?1482:1483),p.x,p.y,SelectionBlend::Normal,false);}
}

void CMinimap::Unavailable(DWORD now)
{
    replyTime=now;visible=pending=dragging=false;
    static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
    std::string text=captions.Get(1458);g_xGameProc.m_xInterface.MsgAdd(0xffffff,0xff,const_cast<char*>(text.c_str()));
}
