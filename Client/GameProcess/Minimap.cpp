#include "StdAfx.h"
#include "Common/LegacyPanel.h"
#include "Common/LoginMessages.h"
void CMinimap::Init()
{
    Destroy(); maps.NewLoad("./Data/Mmap.wil");fullMaps.NewLoad("./Data/Fmmap.wil");LoadOptions(optionsPath);if(portraitPending) Request();
}
void CMinimap::Destroy()
{
    maps.Destroy();fullMaps.Destroy();frame=-1;width=height=0;replyTime=0;
    visible=pending=translucent=large=dragging=releaseCaptured=false;position={672,0};
    monsters.clear();green.clear();orange.clear();pixels.clear();portraitMode=portraitPending=portraitVisible=false;portraitBlink=0;
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
    Request();return true;
}
bool CMinimap::Receive(int number,DWORD now)
{
    if(number<=0 || number>65535) return false;
    const int id=number-1;auto& archive=id<1000?maps:fullMaps;
    if(!archive.NewSetIndex(id<1000?id:id-1000)) return false;
    const auto info=*archive.m_lpstNewCurrWilImageInfo;
    frame=id<1000?id:id-1000;full=id>=1000;width=info.shWidth;height=info.shHeight;replyTime=now;
    pixels.assign(size_t(width)*height,0);
    const WORD* next=(const WORD*)archive.m_pbCurrImage;
    for(int y=0;y<height;++y) {
        const WORD* run=next+1;const WORD* end=run+*next;next=end;int x=0;
        while(run+1<end) {
            const WORD tag=*run++,count=*run++;if(tag==0xc0) {x+=count;continue;}
            if(tag<0xc1 || tag>0xc3 || run+count>end) break;
            for(int i=0;i<count;++i,++x) if(x<width) pixels[size_t(y)*width+x]=0x10000|run[i];
            run+=count;
        }
    }
    if(pending) visible=true;pending=false;
    if(portraitPending) portraitVisible=portraitMode;portraitPending=false;
    Place();return true;
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
    replyTime=now;visible=pending=dragging=false;portraitVisible=portraitPending=false;
    static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
    std::string text=captions.Get(1458);g_xGameProc.m_xInterface.MsgAdd(0xffffff,0xff,const_cast<char*>(text.c_str()));
}

void CMinimap::LoadOptions(const char* path)
{
    optionsPath=path;portraitMode=path && GetPrivateProfileIntA("Options","InformationMode",0,path)!=0;
    portraitPending=portraitMode;portraitVisible=false;
}
bool CMinimap::TogglePortrait(DWORD now)
{
    if(replyTime && now-replyTime<=1000) return false;
    portraitMode=!portraitMode;portraitPending=portraitMode;portraitVisible=false;
    if(portraitPending) Request();
    if(optionsPath) WritePrivateProfileStringA("Options","InformationMode",portraitMode?"1":"0",optionsPath);
    return true;
}
RECT CMinimap::PortraitSource() const
{
    const int w=full?256:180,h=full?256:130;
    const int x=max(0,min(max(0,width-w),int((__int64)player.x*3/2)-min(w,width)/2));
    const int y=max(0,min(max(0,height-h),int(player.y)-min(h,height)/2));
    return RECT{x,y,x+min(w,width),y+min(h,height)};
}
POINT CMinimap::PortraitOrigin() const
{
    const RECT crop=PortraitSource();return full?POINT{crop.right-crop.left>=256?510:530,crop.bottom-crop.top>=256?470:485}:POINT{510,470};
}
POINT CMinimap::PortraitMarker(POINT cell,bool self) const
{
    const RECT crop=PortraitSource();const POINT origin=PortraitOrigin();const int divisor=full?2:1,inset=self?2:1;
    return POINT{(3*(cell.x/divisor)+2*(origin.x-crop.left/divisor-inset))/2,cell.y/divisor+origin.y-crop.top/divisor-inset};
}
void CMinimap::ShowPortrait()
{
    if(!portraitVisible || pixels.empty()) return;
    const RECT crop=PortraitSource();const POINT origin=PortraitOrigin();const int sw=full?256:180,sh=full?256:130;
    const RECT bounds={origin.x,origin.y,origin.x+128,origin.y+128};DrawLegacyPanel(bounds,0,255,0,false);
    DDSURFACEDESC2 surface={};surface.dwSize=sizeof(surface);auto* buffer=g_xMainWnd.GetBackBuffer();
    if(SUCCEEDED(buffer->Lock(NULL,&surface,DDLOCK_WAIT,NULL))) {
        for(int dy=0;dy<128 && origin.y+dy<int(surface.dwHeight);++dy) {
            const int sy=crop.top+(2*dy+1)*sh/256;if(sy>=crop.bottom) continue;
            WORD* row=(WORD*)((BYTE*)surface.lpSurface+(origin.y+dy)*surface.lPitch);
            for(int dx=0;dx<128 && origin.x+dx<int(surface.dwWidth);++dx) {
                const int sx=crop.left+(2*dx+1)*sw/256;if(sx>=crop.right) continue;
                const DWORD value=pixels[size_t(sy)*width+sx];if(!value) continue;
                const WORD pixel=WORD(value);const int r=((pixel>>11)*150+127)/255,g=(((pixel>>5)&63)*150+127)/255,b=((pixel&31)*150+127)/255;
                row[origin.x+dx]=WORD((r<<11)|(g<<5)|b);
            }
        }
        buffer->Unlock(NULL);
    }
    DrawLegacyPanel(bounds,0,0,RGB(100,100,100));
    const POINT self=PortraitMarker(player,true);
    const auto markers=[&](const std::vector<POINT>& cells,COLORREF color) {
        for(const auto& cell:cells) {
            const POINT at=PortraitMarker(cell,false);if(abs(at.x-self.x)>=sw/2 || abs(at.y-self.y)>=sh/2) continue;
            const RECT area={at.x-1,at.y-1,at.x+2,at.y+2};RECT clip;
            if(IntersectRect(&clip,&area,&bounds)) DrawLegacyPanel(clip,color,255,0,false);
        }
    };
    markers(green,RGB(100,250,100));markers(orange,RGB(250,150,50));
    if(portraitBlink>0 && portraitBlink<500) DrawLegacyPanel(RECT{self.x,self.y,self.x+4,self.y+4},0,0,RGB(100,250,100));
}

void CMinimap::Request()
{
    _TDEFAULTMESSAGE message;fnMakeDefMessage(&message,1033,0,0,0,0);g_xClientSocket.SendPacket(&message,NULL);
}

void CMinimap::MapChanged()
{
    pending=visible || pending;visible=false;dragging=releaseCaptured=false;replyTime=0;
    frame=-1;width=height=0;pixels.clear();monsters.clear();green.clear();orange.clear();
    portraitVisible=false;portraitPending=portraitMode;
    if(pending || portraitPending) Request();
}
