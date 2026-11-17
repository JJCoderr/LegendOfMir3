#include "StdAfx.h"
#include "SiegeWnd.h"
#include "Common/LegacyPanel.h"
#include <cstdlib>

void CSiegeWnd::Create(CWHWilImageData* image)
{
    using namespace SiegeLayout;
    CreateGameWnd(WindowId,image,Background,TRUE,Left,Top,Width,Height);
}
void CSiegeWnd::Destroy()
{
    controlled=objectPresent=captured=dragging=false;objectId=0;
    firstValue=secondValue=style=0;pressed=-1;objectCell=target={};requests.clear();
    CGameWnd::Destroy();
}
void CSiegeWnd::Close(bool clearTarget)
{
    m_bActive=FALSE;captured=dragging=false;pressed=-1;
    if(clearTarget) target={};
}
POINT CSiegeWnd::MapOrigin() const
{
    return {m_rcWnd.left+SiegeLayout::MapArea.left,m_rcWnd.top+SiegeLayout::MapArea.top};
}
int CSiegeWnd::ButtonAt(POINT point) const
{
    for(const auto& button:SiegeLayout::Buttons) {
        if(!m_pxWndImage->NewSetIndex(button.normal)) continue;
        const auto image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        const RECT hit={m_rcWnd.left+button.x,m_rcWnd.top+button.y,
            m_rcWnd.left+button.x+image.shWidth,m_rcWnd.top+button.y+image.shHeight};
        if(PtInRect(&hit,point)) return int(button.action);
    }
    return -1;
}
int CSiegeWnd::HintAt(POINT point,POINT& anchor) const
{
    const int hit=ButtonAt(point);
    if(hit<0 || hit==pressed) return 0;
    for(const auto& button:SiegeLayout::Buttons) if(int(button.action)==hit) {
        const auto image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        anchor={m_rcWnd.left+button.x+image.shWidth,m_rcWnd.top+button.y+image.shHeight};return button.caption;
    }
    return 0;
}
void CSiegeWnd::Show()
{
    using namespace SiegeLayout;
    if(!m_bActive) return;
    auto& map=g_xGameProc.m_xInterface.m_minimap;
    if(objectPresent) {
        const POINT origin=MapOrigin(),crop=map.SiegeSource(objectCell);
        map.ShowSiege(origin,objectCell);
        const POINT center=Marker(objectCell,origin,crop);
        const RECT viewport={origin.x,origin.y,origin.x+MapWidth,origin.y+MapHeight};
        const auto marker=[&](POINT cell,COLORREF color,int radius,bool outline) {
            const POINT at=Marker(cell,origin,crop);
            const RECT box={at.x-radius,at.y-radius,at.x+radius+1,at.y+radius+1};RECT clip;
            if(IntersectRect(&clip,&box,&viewport)) DrawLegacyPanel(clip,outline?0:color,outline?0:255,color,outline);
        };
        if(map.showMonsters) for(const auto& cell:map.monsters) marker(cell,RGB(255,255,0),0,false);
        for(const auto& cell:map.green) marker(cell,RGB(100,250,100),1,false);
        for(const auto& cell:map.orange) marker(cell,RGB(250,150,50),1,false);
        marker(map.player,RGB(100,250,100),2,true);
        marker(objectCell,RGB(100,250,250),1,false);
        if(target.x && target.y) {
            const POINT at=Marker(target,origin,crop);
            DrawSelectionFrame(*m_pxWndImage,TargetMarker,at.x-2,at.y-9,SelectionBlend::Normal,false,0,&viewport);
        }
        if(style>0) {
            DrawLegacyPanel(Range(center,style,true),0,0,InnerColor);
            DrawLegacyPanel(Range(center,style,false),0,0,OuterColor);
        }
    }
    if(m_pxWndImage->NewSetIndex(Background)) {
        const auto image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        DrawSelectionFrame(*m_pxWndImage,Background,m_rcWnd.left-(image.shWidth-Width)/2,m_rcWnd.top-(image.shHeight-Height)/2,SelectionBlend::Normal,false);
    }
    for(const auto& button:Buttons) DrawSelectionFrame(*m_pxWndImage,
        pressed==int(button.action)?button.pressed:button.normal,m_rcWnd.left+button.x,m_rcWnd.top+button.y,SelectionBlend::Normal,false);
    HDC dc;auto* surface=g_xMainWnd.GetBackBuffer();if(FAILED(surface->GetDC(&dc))) return;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font=g_xMainWnd.CreateGameFont(face,9);const auto old=SelectObject(dc,font);
    SetBkMode(dc,TRANSPARENT);SetTextColor(dc,TextColor);
    const auto value=[&](int number,RECT box) {
        OffsetRect(&box,m_rcWnd.left,m_rcWnd.top);const auto text=std::to_string(number);
        DrawTextA(dc,text.c_str(),int(text.size()),&box,DT_LEFT|DT_SINGLELINE|DT_VCENTER);
    };
    value(firstValue,FirstValue);value(secondValue,SecondValue);
    SelectObject(dc,old);DeleteObject(font);surface->ReleaseDC(dc);
}
void CSiegeWnd::Pointer(int kind,POINT point)
{
    using namespace SiegeLayout;
    if(kind==1) {if(dragging) MoveGameWnd(point);return;}
    const int hit=ButtonAt(point);
    if(kind==0) {
        captured=true;pressed=hit;
        if(hit>=0) return;
        const POINT origin=MapOrigin();RECT area=MapArea;OffsetRect(&area,m_rcWnd.left,m_rcWnd.top);
        if(objectPresent && style>0) {
            const POINT crop=g_xGameProc.m_xInterface.m_minimap.SiegeSource(objectCell);
            const POINT center=Marker(objectCell,origin,crop);
            if(CanTarget(point,area,Range(center,style,true),Range(center,style,false))) target=Target(point,origin,crop);
        }
        dragging=point.y<m_rcWnd.top+MapArea.top;
        if(dragging) {m_bFocused=TRUE;SetReadyMove(point);}
        return;
    }
    if(!captured) return;
    const bool activate=hit>=0 && hit==pressed;captured=dragging=false;pressed=-1;
    if(BeginLoad()) return;
    if(activate) {g_xSound.PlayLobbyEffect("105.wav");Activate(Action(hit));}
}
void CSiegeWnd::Activate(SiegeLayout::Action action)
{
    using namespace SiegeLayout;
    if(action==Action::Close) {Close(true);return;}
    if(action==Action::Attack && (!target.x || !target.y)) return;
    if(action==Action::Move) target={};
    requests.push_back({action,target});
}

bool CSiegeWnd::BeginLoad()
{
    const auto& held=g_xGameProc.m_xInterface.m_stCommonItem;
    const int icon=held.xItem.m_stItemInfo.stStdItem.wLooks;
    if(!held.bSetted || held.bIsEquipItem || held.bIsBeltItem ||
        (icon!=SiegeLayout::FirstAmmoIcon && icon!=SiegeLayout::SecondAmmoIcon)) return false;
    g_xGameProc.m_xInterface.OpenNameDialog(SiegeLayout::LoadDialog);return true;
}
bool CSiegeWnd::SubmitLoad(const char* text)
{
    auto& ui=g_xGameProc.m_xInterface;auto& held=ui.m_stCommonItem;
    if(!held.bSetted) return false;
    const unsigned quantity=*text?unsigned(std::strtoul(text,nullptr,10)):held.xItem.m_count;
    if(!quantity || quantity>held.xItem.m_count) return false;
    Request request;request.action=SiegeLayout::Action::Load;request.target=target;
    request.item=held.xItem;request.quantity=quantity;
    if(!ui.m_xInventoryWnd.AddInvenItem(held.xItem,held.xItem.m_shCellNum,TRUE)) return false;
    requests.push_back(request);held.bSetted=FALSE;return true;
}
