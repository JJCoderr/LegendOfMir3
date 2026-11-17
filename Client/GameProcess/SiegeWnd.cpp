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
    controlled=objectPresent=captured=dragging=entryPressed=false;objectId=0;
    firstValue=secondValue=style=0;pressed=-1;objectCell=target={};requests.clear();
    objects.clear();CGameWnd::Destroy();
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
        const POINT extent=map.SiegeExtent();
        for(const auto& entry:objects) if(entry.first!=objectId) {
            const POINT at=Marker(entry.second.cell,origin,crop);
            if(abs(at.x-center.x)<extent.x/2 && abs(at.y-center.y)<extent.y/2)
                marker(entry.second.cell,entry.second.kind?RGB(250,150,50):RGB(100,250,100),1,false);
        }
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
    SendRequest({action,target});
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
    SendRequest(std::move(request));held.bSetted=FALSE;return true;
}

void CSiegeWnd::ReceiveStatus(DWORD values)
{
    firstValue=LOWORD(values);secondValue=HIWORD(values);
}
void CSiegeWnd::ReceiveControl(DWORD values,WORD rangeStyle,const char* description)
{
    const char* separator=strchr(description,'\t');
    if(separator) objectId=DWORD(std::strtoul(separator+1,nullptr,10));
    ReceiveStatus(values);style=rangeStyle;controlled=true;
    const auto found=objects.find(objectId);objectPresent=found!=objects.end();
    if(objectPresent) objectCell=found->second.cell;
}
void CSiegeWnd::ReleaseControl()
{
    controlled=false;objectPresent=false;entryPressed=false;
    if(m_bActive) g_xGameProc.m_xInterface.WindowActivate(SiegeLayout::WindowId);
}

void CSiegeWnd::ReceiveObject(DWORD id,POINT cell,WORD kind,DWORD now)
{
    objects[id]={cell,kind,now};
    if(controlled && id==objectId) {objectCell=cell;objectPresent=true;}
}
void CSiegeWnd::ExpireObjects(DWORD now)
{
    for(auto it=objects.begin();it!=objects.end();) {
        if(DWORD(now-it->second.updated)>=SiegeLayout::ObjectLifetime) it=objects.erase(it);
        else ++it;
    }
    objectPresent=controlled && objects.find(objectId)!=objects.end();
}
void CSiegeWnd::ClearObjects()
{
    objects.clear();objectPresent=false;
}

RECT CSiegeWnd::EntryBounds() const
{
    using namespace SiegeLayout;
    if(!controlled || !m_pxWndImage->NewSetIndex(HudBackground)) return {};
    const int y=HudBottom-m_pxWndImage->m_lpstNewCurrWilImageInfo->shHeight-EntryAboveHud;
    if(!m_pxWndImage->NewSetIndex(EntryNormal)) return {};
    const auto image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
    return {EntryX,y,EntryX+image.shWidth,y+image.shHeight};
}
void CSiegeWnd::ShowEntry()
{
    const RECT area=EntryBounds();if(IsRectEmpty(&area)) return;
    DrawSelectionFrame(*m_pxWndImage,entryPressed?SiegeLayout::EntryPressed:SiegeLayout::EntryNormal,area.left,area.top,SelectionBlend::Normal,false);
}

bool CSiegeWnd::BuildRequest(const Request& request,_TDEFAULTMESSAGE& header,std::string& body)
{
    using namespace SiegeLayout;
    WORD message=0,quantity=0;DWORD recognition=0;body.clear();
    switch(request.action) {
    case Action::Attack:
        if(!request.target.x || !request.target.y) return false;
        message=AttackMessage;recognition=DWORD(WORD(request.target.x))|(DWORD(WORD(request.target.y))<<16);break;
    case Action::Wait:message=SiegeLayout::WaitMessage;break;
    case Action::Move:message=MoveMessage;break;
    case Action::Load:
        message=LoadMessage;recognition=request.item.m_stItemInfo.nMakeIndex;quantity=WORD(request.quantity);body=request.item.Name();break;
    default:return false;
    }
    fnMakeDefMessage(&header,message,recognition,quantity,0,0);return true;
}
void CSiegeWnd::SendRequest(Request request)
{
    _TDEFAULTMESSAGE header={};std::string body;
    if(!BuildRequest(request,header,body)) return;
    if(g_xClientSocket.m_sockClient!=INVALID_SOCKET)
        g_xClientSocket.SendPacket(&header,body.empty()?nullptr:body.data());
    requests.push_back(std::move(request));
}
