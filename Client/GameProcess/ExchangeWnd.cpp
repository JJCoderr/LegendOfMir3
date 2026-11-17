#include "StdAfx.h"
#include "ItemHintStyle.h"
#include "TradeWire.h"
#include "Common/LoginMessages.h"
#include <charconv>
#include "Common/LegacyPanel.h"
#include <cstdlib>

void CExchangeWnd::Init()
{
    CGameWnd::Init(); partner.clear(); offerCount=0;
    for (int side=0;side<2;++side) {
        gold[side]=0; first[side]=0;
        for (auto& item:offers[side]) item=Offer{};
    }
    captured=pendingSlot=returnSlot=-1; confirmed=confirmPressed=goldPending=returnPending=headerPressed=false;
}
void CExchangeWnd::CreateExchangeWnd(INT id,CWHWilImageData* image,INT frame,INT x,INT y,INT width,INT height,BOOL movable)
{
    CreateGameWnd(id,image,frame,movable,x,y,width,height);
}
std::string CExchangeWnd::Amount(unsigned value)
{
    std::string result=std::to_string(value);
    for (int i=int(result.size())-3;i>0;i-=3) result.insert(i,1,',');
    return result;
}
bool CExchangeWnd::ConfirmHit(POINT point)
{
    if (!m_pxWndImage->NewSetIndex(1394)) return false;
    const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
    RECT hit={m_rcWnd.left+226,m_rcWnd.top+279,m_rcWnd.left+226+info.shWidth,m_rcWnd.top+279+info.shHeight};
    return PtInRect(&hit,point)!=FALSE;
}
bool CExchangeWnd::ScrollInput(int kind,POINT point)
{
    if (kind==2) { const bool was=captured>=0; captured=-1; return was; }
    if (!m_pxWndImage->NewSetIndex(1225)) return false;
    const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
    const int x=point.x-m_rcWnd.left,y=point.y-m_rcWnd.top;
    if (kind==1 && captured>=0) {
        first[captured]=int(max(0.f,min(1.f,float(y-58)/(158+info.shHeight)))*33); return true;
    }
    if (kind!=0 || y<58 || y>=58+158+info.shHeight) return false;
    const int track[2]={206,457};
    for (int side=0;side<2;++side) {
        if (x<track[side] || x>=track[side]+info.shWidth) continue;
        const int thumb=58+first[side]*158/33;
        if (y>=thumb && y<thumb+info.shHeight) captured=side;
        else first[side]=max(0,min(33,first[side]+(y<thumb?-6:6)));
        return true;
    }
    return false;
}
bool CExchangeWnd::SubmitGold(const char* text,unsigned balance)
{
    if (!m_bActive || confirmed || goldPending || gold[0]) return false;
    if (!*text) return false;
    for (const char* digit=text;*digit;++digit) if (*digit<'0' || *digit>'9') return false;
    const unsigned long long parsed=std::strtoull(text,NULL,10);
    if (!parsed || parsed>balance) return false;
    const unsigned amount=unsigned(parsed);
    goldPending=true;
    _TDEFAULTMESSAGE message;
    // Mir3 CM_TRADE_GOLD carries the amount in recognition; balance stays server-owned.
    fnMakeDefMessage(&message,TradeWire::Gold,INT(amount),0,0,0); g_xClientSocket.SendPacket(&message,NULL);
    return true;
}
int CExchangeWnd::OfferAt(POINT point,int side) const
{
    const int x=point.x-m_rcWnd.left-(side?RemoteGridLeft:LocalGridLeft),y=point.y-m_rcWnd.top-GridTop;
    if (x<0 || x>=GridWidth || y<0 || y>=GridWidth) return -1;
    const int column=x/CellSize,row=first[side]+y/CellSize;
    for (int slot=0;slot<SlotsPerSide;++slot) {
        const auto& item=offers[side][slot];
        if (item.present && item.makeIndex && column>=slot%GridColumns && column<slot%GridColumns+item.width &&
            row>=slot/GridColumns && row<slot/GridColumns+item.height) return slot;
    }
    return -1;
}
bool CExchangeWnd::SetOfferRecord(int side,int slot,const BYTE* record)
{
    CItem item;if(!item.SetMir3Item(record,slot)) return false;
    Offer offer=FromItem(item);if(!offer.present || side<0 || side>1 || slot<0 || slot>=SlotsPerSide) return false;
    offers[side][slot]=std::move(offer);return true;
}
bool CExchangeWnd::RequestReturn()
{
    if (!m_bActive || returnSlot<0 || returnPending) return false;
    auto& item=offers[0][returnSlot];
    returnPending=true;
    g_xClientSocket.SendItemIndex(TradeWire::Remove,INT(item.makeIndex),const_cast<char*>(item.name.c_str()));
    return true;
}
bool CExchangeWnd::CancelReturn()
{
    if (!m_bActive || returnSlot<0 || returnPending || pendingSlot>=0) return false;
    const auto& item=offers[0][returnSlot];
    g_xSound.PlayItemEffect(item.stdMode,item.record31,item.name.c_str());
    returnSlot=-1;
    return true;
}
void CExchangeWnd::ShowHeld(POINT point)
{
    if (!m_bActive || returnSlot<0 || point.x<0 || point.x>=800 || point.y<0 || point.y>=600) return;
    auto& icons=g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY];
    const auto& item=offers[0][returnSlot];
    if (!icons.NewSetIndex(item.icon)) return;
    const auto info=*icons.m_lpstNewCurrWilImageInfo;
    DrawSelectionFrame(icons,item.icon,point.x-(info.shWidth+1)/2,point.y-(info.shHeight+1)/2,SelectionBlend::Normal,false);
}
BOOL CExchangeWnd::OnLButtonDown(POINT point)
{
    headerPressed=false;
    if (ScrollInput(0,point)) return TRUE;
    confirmPressed=ConfirmHit(point);
    headerPressed=!confirmPressed && point.y<m_rcWnd.top+56;
    return confirmPressed || point.y>=m_rcWnd.top+56;
}
BOOL CExchangeWnd::OnLButtonUp(POINT point)
{
    if (ScrollInput(2,point)) return FALSE;
    const bool wasCaptured=headerPressed || confirmPressed; headerPressed=false;
    const bool activate=confirmPressed && ConfirmHit(point) && !confirmed && !goldPending && pendingSlot<0 && returnSlot<0;
    confirmPressed=false;
    if (activate) {
        confirmed=true;
        // Mir3 protocol/message_ids.inc: CM_TRADE_CONFIRM, no payload.
        g_xClientSocket.SendQueryMsg(TradeWire::Confirm);
    }
    if (wasCaptured) return FALSE;
    const RECT grid={m_rcWnd.left+14,m_rcWnd.top+56,m_rcWnd.left+204,m_rcWnd.top+246};
    if (!confirmed && pendingSlot<0 && !returnPending && !g_xGameProc.m_xInterface.m_stCommonItem.bSetted && PtInRect(&grid,point))
        returnSlot=OfferAt(point);
    const RECT amount={m_rcWnd.left+34,m_rcWnd.top+270,m_rcWnd.left+156,m_rcWnd.top+304};
    if (!confirmed && !goldPending && pendingSlot<0 && returnSlot<0 && !gold[0] &&
        !g_xGameProc.m_xInterface.m_stCommonItem.bSetted && g_xGameProc.m_xMyHero.m_nGlod && PtInRect(&amount,point))
        g_xGameProc.m_xInterface.OpenNameDialog(TradeWire::GoldDialog);
    auto& ui=g_xGameProc.m_xInterface;auto& held=ui.m_stCommonItem;
    if(!confirmed && pendingSlot<0 && returnSlot<0 && held.bSetted && held.xItem.m_stItemInfo.nMakeIndex &&
        !held.bIsEquipItem && ui.m_xQuickWnd.heldSlot<0 && !held.bWaitResponse && !ui.m_xInventoryWnd.m_usePending && !ui.m_xStatusWnd.m_equipmentRequest && PtInRect(&grid,point)) {
        const int slot=(first[0]+(point.y-grid.top)/CellSize)*GridColumns+(point.x-grid.left)/CellSize;
        const Offer item=FromItem(held.xItem);
        if(item.present && Count(offers[0])<TradeWire::MaximumOffers && Fits(offers[0],slot,item)) {
            pendingSlot=slot;
            if(item.stackMode>=2 && item.count>1) ui.OpenNameDialog(TradeWire::QuantityDialog);
            else SendOffer(max(1,item.count));
        } else if(OfferAt(point)>=0) {
            static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
            ui.MsgAdd(0xffff32,0,const_cast<char*>(captions.Get(TradeWire::Occupied).c_str()));
        }
    }
    return FALSE;
}
void CExchangeWnd::ShowExchangeWnd()
{
    const int x=m_rcWnd.left,y=m_rcWnd.top;
    if (m_pxWndImage->NewSetIndex(1390)) {
        const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        DrawSelectionFrame(*m_pxWndImage,1390,x-(info.shWidth-494)/2,y-(info.shHeight-330)/2,SelectionBlend::Normal,false);
    }
    auto& icons=g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY];
    for (int side=0;side<2;++side) {
        const int left=x+(side?264:14); RECT clip={left,y+56,left+190,y+246};
        DrawSelectionFrame(*m_pxWndImage,1225,x+(side?457:206),y+58+first[side]*158/33,SelectionBlend::Normal,false);
        for (int slot=0;slot<5*(first[side]+5);++slot) {
            const auto& item=offers[side][slot]; if (!item.present || slot/5+item.height<=first[side] || !icons.NewSetIndex(item.icon)) continue;
            const auto info=*icons.m_lpstNewCurrWilImageInfo;
            const int px=left+38*(slot%5)+(38*item.width-info.shWidth)/2;
            const int py=y+56+38*(slot/5-first[side])+(38*item.height-info.shHeight)/2;
            DrawSelectionFrame(icons,item.icon,px,py,side==0 && slot==returnSlot?SelectionBlend::Alpha:SelectionBlend::Normal,false,128,&clip);
        }
    }
    HDC dc; auto* surface=g_xMainWnd.GetBackBuffer(); if (FAILED(surface->GetDC(&dc))) return;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font=g_xMainWnd.CreateGameFont(face,9); HGDIOBJ old=SelectObject(dc,font); SetBkMode(dc,TRANSPARENT);
    const auto text=[&](const std::string& value,int left,int top,int width,int height,COLORREF color,bool centered) {
        RECT box={x+left,y+top,x+left+width,y+top+height}; SetTextColor(dc,color);
        if (centered) DrawTextA(dc,value.c_str(),-1,&box,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        else { const int saved=SaveDC(dc); IntersectClipRect(dc,box.left,box.top,box.right,box.bottom); TextOutA(dc,box.left,box.top,value.data(),int(value.size())); RestoreDC(dc,saved); }
    };
    text(g_xGameProc.m_xMyHero.m_szName,0,0,228,30,0xfac8c8,true); text(partner,247,0,120,30,0xfac8c8,true);
    for (int side=0;side<2;++side) {
        const int left=side?264:14;
        text(Amount(gold[side]),side?350:70,289,side?85:113,25,0xfac8c8,true);
        for (int slot=0;slot<5*(first[side]+5);++slot) {
            const auto& item=offers[side][slot]; if (!item.present || item.count<=1) continue;
            const int px=left+38*(slot%5)+22,py=56+38*(slot/5-first[side])+23;
            if (px>=left && px+16<=left+190 && py>=56 && py+15<=246) text(std::to_string(item.count),px,py,16,15,0xffffff,false);
        }
    }
    SelectObject(dc,old); DeleteObject(font); surface->ReleaseDC(dc);
    DrawSelectionFrame(*m_pxWndImage,confirmed?1394:confirmPressed?1396:1392,x+226,y+279,SelectionBlend::Normal,false);
    auto& ui=g_xGameProc.m_xInterface;const POINT pointer=g_xGameProc.m_ptMousePos;
    if(ui.m_stCommonItem.bSetted || returnSlot>=0 || ui.m_xMsgBox.IsActive() || ui.m_xGuildEditor.m_bActive || ui.GetWindowInMousePos(pointer)!=_WND_ID_EXCHANGE) return;
    for(int side=0;side<2;++side) {
        const int slot=OfferAt(pointer,side);if(slot<0) continue;
        CItem item;if(item.SetMir3Item(offers[side][slot].record.data(),slot)) item.ShowGeneralHint(pointer.x+ItemHintStyle::PointerOffset,pointer.y+ItemHintStyle::PointerOffset);
        break;
    }
}

int CExchangeWnd::Count(const Offer* items)
{
    int count=0;for(int i=0;i<SlotsPerSide;++i) if(items[i].present) ++count;return count;
}
bool CExchangeWnd::Fits(const Offer* items,int slot,const Offer& item)
{
    if(slot<0 || slot>=SlotsPerSide || item.width<1 || item.height<1 || slot%GridColumns+item.width>GridColumns || slot/GridColumns+item.height>SlotsPerSide/GridColumns) return false;
    const int x=slot%GridColumns,y=slot/GridColumns;
    for(int i=0;i<SlotsPerSide;++i) if(items[i].present && x<i%GridColumns+items[i].width && x+item.width>i%GridColumns && y<i/GridColumns+items[i].height && y+item.height>i/GridColumns) return false;
    return true;
}
int CExchangeWnd::Insert(Offer* items,const Offer& item,int preferred)
{
    if(!item.present || Count(items)>=TradeWire::MaximumOffers) return -1;
    if(Fits(items,preferred,item)) {items[preferred]=item;return preferred;}
    for(int i=0;i<SlotsPerSide;++i) if(Fits(items,i,item)) {items[i]=item;return i;}
    return -1;
}
CExchangeWnd::Offer CExchangeWnd::FromItem(const CItem& item)
{
    Offer offer;auto& icons=g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY];
    if(!icons.NewSetIndex(item.m_stItemInfo.stStdItem.wLooks)) return offer;
    const auto info=*icons.m_lpstNewCurrWilImageInfo;
    offer.present=true;offer.item=item;offer.icon=item.m_stItemInfo.stStdItem.wLooks;
    offer.width=max(1,(info.shWidth+CellSize-1)/CellSize);offer.height=max(1,(info.shHeight+CellSize-1)/CellSize);
    offer.count=item.m_count;offer.makeIndex=item.m_stItemInfo.nMakeIndex;offer.name=item.Name();offer.stdMode=item.m_stItemInfo.stStdItem.bStdMode;offer.record31=item.m_serverRecord[31];offer.stackMode=item.m_stackMode;
    memcpy(offer.record.data(),item.m_serverRecord,RecordBytes);return offer;
}
CItem CExchangeWnd::ToItem(const Offer& offer)
{
    CItem item=offer.item;item.m_stItemInfo.nMakeIndex=offer.makeIndex;item.m_stItemInfo.stStdItem.wLooks=offer.icon;item.m_stItemInfo.stStdItem.bStdMode=offer.stdMode;
    lstrcpynA(item.m_serverName,offer.name.c_str(),sizeof(item.m_serverName));item.m_stackMode=offer.stackMode;item.SetCount(offer.count);return item;
}
bool CExchangeWnd::RequestTrade()
{
    static const int directions[8][2]={{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}};
    auto& game=g_xGameProc;const auto& hero=game.m_xMyHero;if(hero.m_bCurrDir>=8) return false;
    const int x=int(hero.m_wPosX)+directions[hero.m_bCurrDir][0],y=int(hero.m_wPosY)+directions[hero.m_bCurrDir][1];
    game.m_xActorList.MoveCurrentToTop();
    for(int i=0;i<game.m_xActorList.GetCounter();++i) {
        const auto* actor=game.m_xActorList.GetCurrentData();game.m_xActorList.MoveNextNode();
        if(actor==&hero || actor->m_wPosX!=x || actor->m_wPosY!=y || actor->m_bIsDead || actor->m_stFeature.bGender>_GENDER_WOMAN || !actor->m_szName[0]) continue;
        _TDEFAULTMESSAGE request={};fnMakeDefMessage(&request,TradeWire::Request,0,0,0,0);g_xClientSocket.SendPacket(&request,const_cast<char*>(actor->m_szName));return true;
    }
    return false;
}
void CExchangeWnd::Open(const std::string& name)
{
    auto& ui=g_xGameProc.m_xInterface;
    if(!m_bActive) {
        for(auto& side:offers) for(auto& item:side) item=Offer{};
        partner=name;gold[0]=gold[1]=0;first[0]=first[1]=0;pendingSlot=returnSlot=captured=-1;offerCount=0;
        confirmed=confirmPressed=goldPending=returnPending=headerPressed=false;ui.WindowActivate(_WND_ID_EXCHANGE);
    }
    if(!ui.m_xInventoryWnd.m_bActive) {ui.m_xInventoryWnd.m_rcWnd={518,0,800,466};ui.WindowActivate(_WND_ID_INVENTORY);}
}
void CExchangeWnd::Close()
{
    auto& ui=g_xGameProc.m_xInterface;
    if(pendingSlot>=0 && offerCount && !ReplyOffer(TradeWire::AddFail,0,0)) return;
    pendingSlot=returnSlot=captured=-1;offerCount=0;
    if(ui.m_xMsgBox.IsActive() && (ui.m_nIdxMsgBox==TradeWire::QuantityDialog || ui.m_nIdxMsgBox==TradeWire::GoldDialog)) ui.FinishNotice(CLICK_NO,false);
    ui.DeleteWindowToList(_WND_ID_EXCHANGE);m_bActive=FALSE;partner.clear();gold[0]=gold[1]=0;
    confirmed=confirmPressed=goldPending=returnPending=headerPressed=false;
    for(auto& side:offers) for(auto& item:side) item=Offer{};
}
void CExchangeWnd::SendOffer(int quantity)
{
    auto& held=g_xGameProc.m_xInterface.m_stCommonItem;
    if(!m_bActive || pendingSlot<0 || offerCount || !held.bSetted || quantity<=0 || quantity>max(1,held.xItem.m_count)) return;
    offerCount=quantity;
    _TDEFAULTMESSAGE request={};fnMakeDefMessage(&request,TradeWire::Add,held.xItem.m_stItemInfo.nMakeIndex,0,WORD(quantity),0);g_xClientSocket.SendPacket(&request,const_cast<char*>(held.xItem.Name()));
}
void CExchangeWnd::SubmitOffer(const char* text,bool confirm)
{
    int quantity=0;const char* end=text+strlen(text);const auto parsed=std::from_chars(text,end,quantity);
    if(confirm && parsed.ec==std::errc{} && parsed.ptr==end && quantity>0 && quantity<=g_xGameProc.m_xInterface.m_stCommonItem.xItem.m_count) SendOffer(quantity);
    else pendingSlot=-1;
}
bool CExchangeWnd::ReplyOffer(WORD opcode,WORD remaining,DWORD instance)
{
    auto& ui=g_xGameProc.m_xInterface;auto& bag=ui.m_xInventoryWnd;auto& held=ui.m_stCommonItem;
    std::array<Offer,SlotsPerSide> next;std::copy(std::begin(offers[0]),std::end(offers[0]),next.begin());
    CItem sound;std::vector<CItem> restore;
    if(opcode==TradeWire::RemoveOk || opcode==TradeWire::RemoveFail) {
        if(!returnPending || returnSlot<0 || !next[returnSlot].present) return false;
        const Offer item=next[returnSlot];sound=ToItem(item);next[returnSlot]=Offer{};
        if(opcode==TradeWire::RemoveOk) {
            bool exists=false;if(item.stackMode>=2) for(const auto& entry:bag.m_stInventoryItem) if(entry.bSetted && item.name==entry.xItem.Name()) {exists=true;break;}
            static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
            if(!exists && item.name!=captions.Get(TradeWire::GoldName)) restore.push_back(sound);
        } else if(Insert(next.data(),item)<0) return false;
        if(!bag.RestoreItems(restore)) return false;
        returnSlot=-1;returnPending=false;
    } else {
        if(pendingSlot<0 || !offerCount || !held.bSetted || (opcode!=TradeWire::AddOk && opcode!=TradeWire::AddFail)) return false;
        sound=held.xItem;
        if(opcode==TradeWire::AddOk) {
            if(remaining && remaining>=held.xItem.m_count) return false;
            CItem offered=held.xItem;
            if(remaining) {
                offered.SetCount(offered.m_count-remaining);offered.m_stItemInfo.nMakeIndex=instance;
                for(int i=0;i<4;++i) offered.m_serverRecord[62+i]=BYTE(instance>>(8*i));
                CItem rest=held.xItem;rest.SetCount(remaining);restore.push_back(rest);
            }
            if(Insert(next.data(),FromItem(offered),pendingSlot)<0) return false;
        } else restore.push_back(held.xItem);
        if(!bag.RestoreItems(restore)) return false;
        held=COMMONITEMSET{};pendingSlot=-1;offerCount=0;
        if(opcode==TradeWire::AddOk) {
            auto& quick=ui.m_xQuickWnd;const int slot=quick.Find(sound.Name());
            if(slot>=0) {if(remaining) {quick.slots[slot].count=remaining;quick.slots[slot].item.SetCount(remaining);} else quick.slots[slot]=CQuickWnd::Slot{};}
        }
    }
    std::copy(next.begin(),next.end(),std::begin(offers[0]));g_xSound.PlayItemEffect(sound.m_stItemInfo.stStdItem.bStdMode,sound.m_serverRecord[31],sound.Name());return true;
}
bool CExchangeWnd::Cancel()
{
    if(!m_bActive) return false;
    auto& ui=g_xGameProc.m_xInterface;auto& bag=ui.m_xInventoryWnd;auto& held=ui.m_stCommonItem;
    std::vector<CItem> restore;for(const auto& item:offers[0]) if(item.present && item.stackMode<2) restore.push_back(ToItem(item));
    if(pendingSlot>=0 && held.bSetted) {
        bool exists=false;if(held.xItem.m_stackMode>=2) for(const auto& item:bag.m_stInventoryItem) if(item.bSetted && !strcmp(item.xItem.Name(),held.xItem.Name())) {exists=true;break;}
        static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
        if(!exists && strcmp(held.xItem.Name(),captions.Get(TradeWire::GoldName).c_str())) restore.push_back(held.xItem);
    }
    if(!bag.RestoreItems(restore)) return false;
    if(pendingSlot>=0) held=COMMONITEMSET{};
    pendingSlot=-1;offerCount=0;
    g_xGameProc.m_xMyHero.m_nGlod+=gold[0];Close();return true;
}
bool CExchangeWnd::Receive(const _TDEFAULTMESSAGE& header,const char* body)
{
    using namespace TradeWire;auto& ui=g_xGameProc.m_xInterface;
    if(header.wIdent==OpenFail) {ui.ShowNotice(OpenError);return true;}
    if(header.wIdent==TradeWire::Open) {
        if(m_bActive) {Open(partner);return true;}
        const int length=int(strlen(body));if(length>80) return false;char name[62]={};const int decoded=fnDecode6BitBuf(const_cast<char*>(body),name,sizeof(name)-1);
        if(decoded<=0 || decoded>60 || int(strlen(name))!=decoded) return false;Open(name);return true;
    }
    if(!m_bActive) return false;
    if(header.wIdent==AddOk || header.wIdent==AddFail || header.wIdent==RemoveOk || header.wIdent==RemoveFail) return ReplyOffer(header.wIdent,header.wParam,DWORD(header.nRecog));
    if(header.wIdent==Cancelled) return Cancel();
    if(header.wIdent==Complete) {
        if(pendingSlot>=0 || returnPending) return false;
        for(const auto& side:offers) for(const auto& item:side) if(item.present && item.stackMode>2) {
            bool exists=false;for(const auto& owned:ui.m_xInventoryWnd.m_stInventoryItem) if(owned.bSetted && owned.xItem.m_stItemInfo.stStdItem.wLooks==item.icon) {exists=true;break;}
            if(!exists) {const int slot=ui.m_xQuickWnd.Find(item.name.c_str());if(slot>=0) ui.m_xQuickWnd.slots[slot]=CQuickWnd::Slot{};}
        }
        Close();return true;
    }
    if(header.wIdent==LocalGold || header.wIdent==PeerGold) {
        const int side=header.wIdent==LocalGold?0:1;gold[side]=DWORD(header.nRecog);
        if(!side) {g_xGameProc.m_xMyHero.m_nGlod=MAKELONG(header.wParam,header.wTag);goldPending=false;}
        g_xSound.PlayLobbyEffect("106.wav");return true;
    }
    if(header.wIdent!=PeerAdd && header.wIdent!=PeerRemove) return false;
    BYTE raw[RecordBytes];if(strlen(body)!=134 || fnDecode6BitBuf(const_cast<char*>(body),(char*)raw,sizeof(raw))!=RecordBytes) return false;
    CItem item;if(!item.SetMir3Item(raw,0)) return false;const Offer incoming=FromItem(item);if(!incoming.present) return false;
    if(header.wIdent==PeerAdd) {if(Insert(offers[1],incoming)<0) return false;}
    else for(auto& other:offers[1]) if(other.present && other.makeIndex==incoming.makeIndex && other.name==incoming.name) {other=Offer{};break;}
    g_xSound.PlayItemEffect(item.m_stItemInfo.stStdItem.bStdMode,item.m_serverRecord[31],item.Name());
    static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");std::string text=captions.Get(header.wIdent==PeerAdd?PeerAdded:PeerRemoved);const auto at=text.find("%s");if(at!=std::string::npos) text.replace(at,2,item.Name());ui.MsgAdd(0xffffff,33023,const_cast<char*>(text.c_str()));return true;
}
