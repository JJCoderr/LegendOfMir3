#include "StdAfx.h"
#include "ItemHintStyle.h"
#include "Common/LegacyPanel.h"
#include "Common/LoginMessages.h"
#include <cmath>
#include <cstdlib>
#include <cerrno>

void CStoreWnd::Init() { CGameWnd::Init(); Reset(); }
void CStoreWnd::Reset()
{
    actor=first=page=0; highlighted=selected=offerIndex=bulk=pressed=-1;
    storage=false; storagePending=0; details=loading=scrollCaptured=false; fraction=0; goods.clear(); offers.clear(); Layout();
}
void CStoreWnd::Layout() { m_rcWnd=storage?RECT{-292,234,196,439}:RECT{0,225,bulk>=0?516:details?488:292,449}; }
RECT CStoreWnd::OfferCell(int cell) const
{
    const int x=(storage?22:315)+38*(cell%4),y=(storage?266:267)+38*(cell/4),size=storage?37:35;
    return RECT{x,y,x+size,y+size};
}
void CStoreWnd::SetStorage(int id,std::vector<Offer> list)
{
    const DWORD pending=storage?storagePending:0; const int owner=storage?actor:id;
    g_xGameProc.m_xInterface.m_xInventoryWnd.ClearService();
    CloseOffers(); Reset(); actor=owner; storage=details=true; storagePending=pending; offers=std::move(list);
    offerIndex=offers.empty()?-1:0; Layout();
}
void CStoreWnd::Withdraw(int quantity)
{
    if (!storage || storagePending || offerIndex<0 || offerIndex>=int(offers.size())) return;
    const auto& offer=offers[offerIndex];
    if (!quantity && offer.stackMode>=2 && offer.count>1) { g_xGameProc.m_xInterface.OpenNameDialog(9); return; }
    if (!quantity && offer.stackMode>=2) quantity=1;
    if (quantity<0 || quantity>offer.count) return;
    storagePending=offer.id; Request(CM_USERTAKEBACKSTORAGEITEM,offer.name,offer.id,WORD(quantity));
}
void CStoreWnd::SubmitWithdrawal(const char* text)
{
    if (offerIndex<0 || offerIndex>=int(offers.size())) return;
    const long quantity=!strcmp(text,"*")?offers[offerIndex].count:strtol(text,NULL,10);
    if (quantity>0 && quantity<=65535) Withdraw(int(quantity));
}
void CStoreWnd::CreateStoreWnd(INT id,CWHWilImageData* image,INT,INT,INT,INT,INT,BOOL)
{ CreateGameWnd(id,image,1260,FALSE,0,225,292,224); }
bool CStoreWnd::SetGoods(int id,const char* text,int count)
{
    if (id<=0 || id!=g_xGameProc.m_xInterface.m_xNPCWnd.m_nNpcId || count<0 || count>255) return false;
    std::vector<Good> result; const char* cursor=text;
    for (int i=0;i<count;++i) {
        Good good; const char* end=strchr(cursor,'/'); if (!end || end==cursor) return false;
        good.name.assign(cursor,end); cursor=end+1;
        int* fields[]={&good.variants,&good.price,&good.stock,&good.icon,&good.requirementType,&good.requirement};
        for (auto field:fields) {
            char* tail; errno=0; const long value=strtol(cursor,&tail,10);
            if (*cursor<'0' || *cursor>'9' || *tail!='/' || value<0 || errno==ERANGE) return false;
            *field=int(value); cursor=tail+1;
        }
        if (good.icon>65535) return false;
        result.push_back(good);
    }
    if (*cursor) return false;
    CloseOffers(); Reset(); actor=id; goods=std::move(result); highlighted=goods.empty()?-1:0; return true;
}
void CStoreWnd::SetOffers(std::vector<Offer> list,int offset)
{
    auto& ui=g_xGameProc.m_xInterface;
    if (ui.m_xMsgBox.IsActive() && (ui.m_nIdxMsgBox==8 || ui.m_nIdxMsgBox==9)) ui.FinishNotice(CLICK_NO,false);
    offers=std::move(list); bulk=-1; details=true; loading=false; pressed=-1;
    offerIndex=offers.empty()?-1:0; page=offset/10; Layout();
}
static bool ReadShopRecords(const char* body,int count,std::vector<CStoreWnd::Offer>& result)
{
    const char* cursor=body;
    for (int i=0;i<count;++i) {
        const char* end=strchr(cursor,'/'); if (!end || end-cursor!=134) return false;
        const std::string encoded(cursor,end); BYTE record[101]={};
        if (fnDecode6BitBuf(const_cast<char*>(encoded.c_str()),(char*)record,100)!=100) return false;
        const char* name=(char*)record; const size_t length=strnlen(name,30); if (!length) return false;
        const DWORD id=DWORD(record[62])|(DWORD(record[63])<<8)|(DWORD(record[64])<<16)|(DWORD(record[65])<<24);
        if (!id) return false;
        result.push_back({std::string(name,length),record[36]|(record[37]<<8),record[68]|(record[69]<<8),id,record[71]|(record[72]<<8),record[61]}); memcpy(result.back().record.data(),record,result.back().record.size()); cursor=end+1;
    }
    return !*cursor;
}
bool CStoreWnd::SetDetailRecords(const char* body,int count,int offset)
{
    if (count<0 || count>255 || selected<0) return false;
    std::vector<Offer> result; if (!ReadShopRecords(body,count,result)) return false;
    SetOffers(std::move(result),offset); return true;
}
bool CStoreWnd::SetStorageRecords(int id,const char* body,int count)
{
    if ((!storage && id<=0) || count<0 || count>65535) return false;
    std::vector<Offer> result; if (!ReadShopRecords(body,count,result)) return false;
    SetStorage(id,std::move(result)); return true;
}
void CStoreWnd::Request(int opcode,const std::string& name,DWORD instance,WORD quantity)
{
    _TDEFAULTMESSAGE message; fnMakeDefMessage(&message,opcode,actor,LOWORD(instance),HIWORD(instance),quantity);
    g_xClientSocket.SendPacket(&message,const_cast<char*>(name.c_str()));
}
void CStoreWnd::OpenCategory()
{
    if (highlighted<0) return;
    if (goods[highlighted].variants==2) {
        bulk=highlighted; details=loading=false; selected=offerIndex=-1; page=0; offers.clear(); Layout(); return;
    }
    bulk=-1;
    if (selected!=highlighted) { offers.clear(); offerIndex=-1; }
    selected=highlighted; page=0; loading=details; Layout();
    Request(CM_USERGETDETAILITEM,goods[selected].name);
}
void CStoreWnd::CloseOffers()
{
    auto& ui=g_xGameProc.m_xInterface;
    if (ui.m_xMsgBox.IsActive() && (ui.m_nIdxMsgBox==8 || ui.m_nIdxMsgBox==9)) ui.FinishNotice(CLICK_NO,false);
    bulk=-1; details=loading=false; selected=offerIndex=pressed=-1; page=0; offers.clear(); Layout();
}
void CStoreWnd::BuyOffer()
{ if (storage) Withdraw(); else if (!loading && offerIndex>=0) Request(CM_USERBUYITEM,offers[offerIndex].name,offers[offerIndex].id); }
void CStoreWnd::BuyBulk(const char* text)
{
    if (bulk<0) return;
    const long quantity=*text?strtol(text,NULL,10):1;
    if (quantity>0 && quantity<=65535) Request(CM_USERBUYITEM,goods[bulk].name,goods[bulk].stock,WORD(quantity));
}
int CStoreWnd::Hit(POINT point)
{
    const auto button=[&](int frame,int x,int y) {
        if (!m_pxWndImage->NewSetIndex(frame)) return false;
        const auto& info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        const RECT box={x,y,x+info.shWidth,y+info.shHeight}; return PtInRect(&box,point)!=FALSE;
    };
    const int offset=storage?-292:0;
    if (!storage && button(1221,258,408)) return -7;
    if (details) {
        if (button(1221,offset+449,384)) return -5;
        if (button(1265,offset+358,388)) return -2;
        if (button(1268,offset+316,388)) return -3;
        if (button(1270,offset+406,388)) return -4;
    }
    if (bulk>=0) {
        if (button(1221,475,281)) return -5;
        if (button(1272,367,284)) return -8;
    }
    if (!storage && button(1263,106,411)) return -6;
    if (details) for (int i=0;i<12;++i) {
        const RECT cell=OfferCell(i); const int index=i+(storage?12*page:0);
        if (PtInRect(&cell,point)) return !loading && index<int(offers.size())?256+index:-1;
    }
    if (storage) return -1;
    for (int i=0;i<4;++i) {
        const int y=241+42*i; const RECT icon={20,y,56,y+36},label={62,y,251,y+36};
        if ((PtInRect(&icon,point) || PtInRect(&label,point)) && first+i<int(goods.size())) return first+i;
    }
    return -1;
}
bool CStoreWnd::Scroll(int kind,POINT point)
{
    if (goods.size()<2 || !m_pxWndImage->NewSetIndex(1225)) return false;
    const auto& info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
    const int y=257+int(std::lround(85*fraction));
    const RECT track={260,257,260+info.shWidth,342+info.shHeight},thumb={260,y,260+info.shWidth,y+info.shHeight};
    if (kind==0 && PtInRect(&track,point)) {
        scrollCaptured=PtInRect(&thumb,point)!=FALSE;
        if (!scrollCaptured) fraction=max(0.0,min(1.0,fraction+(point.y<y+info.shHeight/2?-5.0:5.0)/goods.size()));
    } else if (kind==1 && scrollCaptured) fraction=max(0.0,min(1.0,(point.y-257)/double(85+info.shHeight)));
    else if (kind==2 && scrollCaptured) scrollCaptured=false;
    else return false;
    first=int(std::lround((goods.size()-1)*fraction)); pressed=-1; return true;
}
BOOL CStoreWnd::OnLButtonDown(POINT point) { if (!Scroll(0,point)) pressed=Hit(point); return TRUE; }
BOOL CStoreWnd::OnLButtonUp(POINT point)
{
    if (Scroll(2,point)) return FALSE;
    const int control=Hit(point),previous=pressed; pressed=-1;
    if (control!=previous || control==-1) return FALSE;
    if (control==-7 || (control==-5 && storage)) return TRUE;
    if (control==-5) CloseOffers();
    else if (control==-6) OpenCategory();
    else if (control==-8) g_xGameProc.m_xInterface.OpenNameDialog(8);
    else if (control==-2) BuyOffer();
    else if ((control==-3 || control==-4) && storage && !storagePending) {
        page=max(0,min(max(0,(int(offers.size())-1)/12),page+(control==-3?-1:1))); offerIndex=-1;
    }
    else if ((control==-3 || control==-4) && selected>=0 && !loading) {
        const int next=max(0,min(6553,page+(control==-3?-1:1)));
        if (next!=page) { page=next; loading=true; Request(CM_USERGETDETAILITEM,goods[selected].name,10*page); }
    } else if (control>=256) offerIndex=control-256;
    else if (control>=0) highlighted=control;
    return FALSE;
}
void CStoreWnd::OnDoubleClick(POINT point)
{
    if (bulk>=0) {
        const RECT icon={305,242,341,278},label={352,242,500,278};
        if (PtInRect(&icon,point) || PtInRect(&label,point)) { pressed=-1; BuyBulk("1"); return; }
    }
    const int hit=Hit(point); pressed=-1;
    if (hit>=256) { offerIndex=hit-256; BuyOffer(); }
    else if (hit>=0) { highlighted=hit; OpenCategory(); }
}
void CStoreWnd::OnScrollDown() { first=max(0,first-1); fraction=goods.size()>1?double(first)/(goods.size()-1):0; pressed=-1; }
void CStoreWnd::OnScrollUp() { first=min(max(0,int(goods.size())-1),first+1); fraction=goods.size()>1?double(first)/(goods.size()-1):0; pressed=-1; }
void CStoreWnd::ShowStoreWnd()
{
    Layout();
    const int skin=bulk>=0?1262:details?1261:1260;
    if (m_pxWndImage->NewSetIndex(skin)) {
        const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        DrawSelectionFrame(*m_pxWndImage,skin,m_rcWnd.left+(m_rcWnd.right-m_rcWnd.left-info.shWidth)/2,m_rcWnd.top+(m_rcWnd.bottom-m_rcWnd.top-info.shHeight)/2,SelectionBlend::Normal,false);
    }
    if (goods.size()>1) DrawSelectionFrame(*m_pxWndImage,1225,260,257+int(std::lround(85*fraction)),SelectionBlend::Normal,false);
    const int buttons[][4]={{-7,1222,258,408},{-6,1264,106,411},{-5,1222,bulk>=0?475:449,bulk>=0?281:384},
        {-8,1273,367,284},{-2,1266,358,388},{-3,1269,316,388},{-4,1271,406,388}};
    for (const auto& button:buttons) if (pressed==button[0]) DrawSelectionFrame(*m_pxWndImage,button[1],button[2]+(storage?-292:0),button[3],SelectionBlend::Normal,false);
    auto& icons=g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY];
    const auto icon=[&](int frame,int x,int y) {
        if (!icons.NewSetIndex(frame)) return; const auto info=*icons.m_lpstNewCurrWilImageInfo;
        DrawSelectionFrame(icons,frame,x-info.shWidth/2,y-info.shHeight/2,SelectionBlend::Normal,false);
    };
    for (int i=0;i<4 && first+i<int(goods.size());++i) {
        const auto& good=goods[first+i]; const int y=241+42*i; const bool chosen=first+i==highlighted;
        const COLORREF tint=good.variants?RGB(40,120,120):RGB(80,80,160),border=RGB(250,200,150);
        if (chosen) DrawLegacyPanel(RECT{62,y,251,y+36},tint,1,border);
        if (chosen || good.variants) DrawLegacyPanel(RECT{20,y,56,y+36},chosen?tint:0,chosen?1:0,chosen?border:RGB(180,180,180));
        icon(good.icon,39,y+19);
    }
    if (bulk>=0) {
        DrawLegacyPanel(RECT{305,242,341,278},RGB(80,80,160),1,RGB(180,180,180));
        DrawLegacyPanel(RECT{352,242,500,278},RGB(80,80,160),1,RGB(250,200,150)); icon(goods[bulk].icon,324,261);
    }
    if (details) for (int i=0;i<12;++i) {
        const int index=i+(storage?12*page:0); if (index>=int(offers.size())) break;
        const RECT cell=OfferCell(i);
        if (index==offerIndex) DrawLegacyPanel(cell,RGB(50,150,150),1,RGB(180,180,180));
        icon(offers[index].icon,cell.left+19,cell.top+19);
    }
    HDC dc; auto* surface=g_xMainWnd.GetBackBuffer(); if (FAILED(surface->GetDC(&dc))) return;
    static CLoginMessages captions; static const bool loaded=captions.Load("./CmList.dat");
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0}; HFONT font=g_xMainWnd.CreateGameFont(face,9);
    auto old=SelectObject(dc,font); SetBkMode(dc,TRANSPARENT);
    const auto text=[&](const std::string& value,int x,int y,int width,COLORREF color) {
        const int saved=SaveDC(dc); IntersectClipRect(dc,x,y,x+width,y+16); SetTextColor(dc,color);
        TextOutA(dc,x,y,value.data(),int(value.size())); RestoreDC(dc,saved);
    };
    const auto requirement=[&](const Good& good,int x,int y,int width) {
        auto& hero=g_xGameProc.m_xMyHero;
        if (good.icon==304 && good.requirement<=2) text(captions.Get(5170+good.requirement),x,y,width,hero.m_bJob==good.requirement?13172680:1974015);
        else if (good.variants && good.requirement>0 && good.requirementType<12) {
            auto& attack=g_xGameProc.m_xInterface.m_xStatusWnd.m_elementValues[0];
            const auto low=[&](int a,int b) { int value=255; for (int i=a;i<b;++i) value=min(value,int(attack[i])); return value; };
            const int attributes[]={hero.m_stAbility.bLevel,HIBYTE(hero.m_stAbility.wDC),low(0,4),low(4,7),low(0,7),attack[0],attack[1],attack[2],attack[3],attack[4],attack[5],attack[6]};
            text(captions.Get(5140+good.requirementType)+" "+std::to_string(good.requirement),x,y,width,attributes[good.requirementType]>=good.requirement?16773360:1974015);
        }
    };
    const auto label=[&](const Good& good,int x,int y,int width,bool shadow) {
        SIZE size={}; GetTextExtentPoint32A(dc,good.name.data(),int(good.name.size()),&size);
        auto price=captions.Get(2300); const auto marker=price.find("%d");
        if (marker!=price.npos) price.replace(marker,2,std::to_string(good.price)); else price=std::to_string(good.price);
        if (shadow) { text(good.name,x-1,y-1,width,0x0a0a0a); text(price,x+1+size.cx,y-1,124,0x0a0a0a); }
        text(good.name,x,y,width,0xfafa); text(price,x+2+size.cx,y,124,0x64b4fa); requirement(good,x,y+20,width);
    };
    for (int i=0;i<4 && first+i<int(goods.size());++i) {
        const auto& good=goods[first+i]; label(good,66,241+42*i+(good.icon==304 || (good.variants && good.requirement)?2:13),184,first+i==highlighted);
    }
    if (bulk>=0) label(goods[bulk],358,225+(goods[bulk].requirement?19:30),126,true);
    if (storage) {
        RECT current={52,229,92,259},total={82,229,132,259}; SetTextColor(dc,0x96c8ff);
        const auto value=std::to_string(page+1),pages="/    "+std::to_string(max(1,(int(offers.size())+11)/12));
        DrawTextA(dc,value.c_str(),-1,&current,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        DrawTextA(dc,pages.c_str(),-1,&total,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    } else if (details) {
        RECT box={310,222,473,266}; SetTextColor(dc,0x96c8ff); const auto value=loading?std::string("..."):std::to_string(page);
        DrawTextA(dc,value.c_str(),-1,&box,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        const char priceFace[]={char(0xb1),char(0xbc),char(0xb8),char(0xb2),0};
        HFONT priceFont=g_xMainWnd.CreateGameFont(priceFace,8); SelectObject(dc,priceFont);
        for (int i=0;i<min(12,int(offers.size()));++i) {
            if (!icons.NewSetIndex(offers[i].icon) || icons.m_lpstNewCurrWilImageInfo->shWidth<=0) continue;
            const int x=315+38*(i%4),y=267+38*(i/4); const auto price=std::to_string(offers[i].price);
            text(price,x-1,y+20,35,0x0a0a0a); text(price,x,y+21,35,0xffffff);
        }
        SelectObject(dc,font); DeleteObject(priceFont);
    }
    SelectObject(dc,old); DeleteObject(font); surface->ReleaseDC(dc);
    auto& ui=g_xGameProc.m_xInterface;
    const POINT pointer=g_xGameProc.m_ptMousePos;
    if(details && !loading && !ui.m_stCommonItem.bSetted && !ui.m_xMsgBox.IsActive() && !ui.m_xGuildEditor.m_bActive && ui.GetWindowInMousePos(pointer)==_WND_ID_STORE) {
        for(int cellIndex=0;cellIndex<OffersPerPage;++cellIndex) {
            const int index=cellIndex+(storage?OffersPerPage*page:0);if(index>=int(offers.size())) break;
            const RECT cell=OfferCell(cellIndex);if(!PtInRect(&cell,pointer)) continue;
            CItem item;if(item.SetMir3Item(offers[index].record.data(),0)) item.ShowGeneralHint(pointer.x+ItemHintStyle::PointerOffset,pointer.y+ItemHintStyle::PointerOffset,!storage,ItemHintStyle::ShopIcon(item.m_stItemInfo.stStdItem.bStdMode));
            break;
        }
    }
}
void CStoreWnd::Receive(const _TDEFAULTMESSAGE& header,const char* body)
{
    if (header.wIdent==SM_STORAGE_LIST) {
        if (SetStorageRecords(header.nRecog,body,header.wSeries) && !m_bActive) g_xGameProc.m_xInterface.WindowActivate(_WND_ID_STORE);
    } else if (header.wIdent>=SM_STORAGE_WITHDRAW_OK && header.wIdent<=SM_STORAGE_BAG_FULL) {
        if (header.wIdent!=SM_STORAGE_WITHDRAW_OK) g_xGameProc.m_xInterface.ShowNotice(header.wIdent==SM_STORAGE_WITHDRAW_FAIL?1445:1446);
        if (!storage || !storagePending) return;
        const DWORD instance=storagePending; storagePending=0;
        if (header.wIdent==SM_STORAGE_WITHDRAW_OK) for (auto item=offers.begin();item!=offers.end();++item) if (item->id==instance) {
            if (header.wParam) item->count=header.wParam;
            else { offers.erase(item); offerIndex=-1; page=min(page,max(0,(int(offers.size())-1)/12)); }
            break;
        }
    } else if (header.wIdent==SM_SENDGOODSLIST) {
        char decoded[10241]; const int length=fnDecode6BitBuf(const_cast<char*>(body),decoded,sizeof(decoded)-1); decoded[length]=0;
        if (SetGoods(header.nRecog,decoded,header.wParam) && !m_bActive) g_xGameProc.m_xInterface.WindowActivate(_WND_ID_STORE);
    }
    else if (header.wIdent==SM_SENDDETAILGOODSLIST && header.nRecog==actor) {
        char decoded[8197]; const int length=fnDecode6BitBuf(const_cast<char*>(body),decoded,sizeof(decoded)-1); decoded[length]=0;
        SetDetailRecords(decoded,header.wParam,header.wTag);
    } else if (header.wIdent==SM_BUYITEM_SUCCESS || header.wIdent==SM_BUYITEM_FAIL) {
        const bool success=header.wIdent==SM_BUYITEM_SUCCESS;
        if (success || (header.nRecog>=1 && header.nRecog<=3)) {
            if ((success || header.nRecog==1) && details && offerIndex>=0 && offerIndex<int(offers.size())) {
                offers.erase(offers.begin()+offerIndex); if (offerIndex>=int(offers.size())) offerIndex=-1;
            }
            if (success) g_xGameProc.m_xMyHero.m_nGlod=header.nRecog;
            else g_xGameProc.m_xInterface.ShowNotice(1436+header.nRecog);
        }
    }
}
