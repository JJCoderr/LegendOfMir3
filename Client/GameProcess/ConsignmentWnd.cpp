#include "StdAfx.h"
#include "ConsignmentWnd.h"
#include "Common/LegacyPanel.h"
#include "Common/LoginMessages.h"
#include "Common/AnsiInput.h"
#include <cstdlib>

namespace {
std::vector<std::string> DescriptionLines(HDC dc,const std::string& description)
{
    using namespace ConsignmentLayout;
    std::vector<std::string> result;
    size_t paragraph=0;
    for(int count=0;paragraph<description.size() && count<DescriptionRows;++count) {
        const auto delimiter=description.find('\r',paragraph);
        const auto line=description.substr(paragraph,delimiter==std::string::npos?delimiter:delimiter-paragraph);
        std::string wrapped;int start=0,highBytes=0;
        // MIR3 48C03A inserts a backtick separator, backing up over a split double-byte character.
        for(int i=0;i<int(line.size());++i) {
            SIZE extent={};GetTextExtentPoint32A(dc,line.data()+start,i-start+1,&extent);
            const bool high=static_cast<signed char>(line[i])<0;
            if(extent.cx<=DescriptionWidth) {if(high) ++highBytes;wrapped+=line[i];}
            else if(high && highBytes%2) {wrapped.back()='`';start=i-1;i-=2;--highBytes;}
            else {wrapped+='`';start=i;--i;}
        }
        size_t begin=0;
        for(int row=0;begin<wrapped.size() && row<DescriptionRows;++row) {
            const auto end=wrapped.find('`',begin);
            result.push_back(wrapped.substr(begin,end==std::string::npos?end:end-begin));
            if(end==std::string::npos) break;begin=end+1;
        }
        if(delimiter==std::string::npos) break;paragraph=delimiter+1;
    }
    return result;
}
}

void CConsignmentWnd::Create(CWHWilImageData* image,bool registerPage)
{
    using namespace ConsignmentLayout;
    registration=registerPage;
    const auto& layout=registration?Registration:Browser;
    CreateGameWnd(registration?RegistrationId:BrowserId,image,layout.frame,TRUE,layout.x,layout.y,layout.width,layout.height);
}
std::vector<ConsignmentLayout::Button> CConsignmentWnd::Buttons() const
{
    using namespace ConsignmentLayout;
    if(registration) return {std::begin(RegistrationButtons),std::end(RegistrationButtons)};
    return {std::begin(BrowserButtons),std::end(BrowserButtons)};
}
int CConsignmentWnd::ButtonAt(POINT point) const
{
    for(const auto& button:Buttons()) {
        if(!m_pxWndImage->NewSetIndex(button.normal)) continue;
        const auto image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        const RECT hit={m_rcWnd.left+button.x,m_rcWnd.top+button.y,m_rcWnd.left+button.x+image.shWidth,m_rcWnd.top+button.y+image.shHeight};
        if(PtInRect(&hit,point)) return int(button.action);
    }
    return -1;
}
int CConsignmentWnd::RowAt(POINT point) const
{
    using namespace ConsignmentLayout;
    if(registration) return -1;
    const int x=point.x-m_rcWnd.left,y=point.y-m_rcWnd.top;
    if(x<FirstRow.left || x>=FirstRow.right || y<FirstRow.top) return -1;
    const int row=(y-FirstRow.top)/RowHeight;
    if(row>=VisibleRows || y>=FirstRow.bottom+row*RowHeight) return -1;
    return row;
}
RECT CConsignmentWnd::ItemBounds()
{
    using namespace ConsignmentLayout;
    if(registration?!hasItem:(selected<0 || selected>=int(listings.size()))) return {};
    const auto& selectedItem=registration?item:listings[selected].item;
    auto& icons=g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY];
    if(!icons.NewSetIndex(selectedItem.m_stItemInfo.stStdItem.wLooks)) return {};
    const auto image=*icons.m_lpstNewCurrWilImageInfo;
    const int left=m_rcWnd.left+(registration?RegistrationIconX:BrowserIconX)-image.shWidth/2;
    int top=m_rcWnd.top+(registration?RegistrationIconY:BrowserIconY)-image.shHeight/2;
    int height=image.shHeight;
    if(height>IconMaxHeight) {top=registration?78:200;height=registration?RegistrationIconCrop:BrowserIconCrop;}
    return {left,top,left+image.shWidth,top+height};
}
void CConsignmentWnd::DrawItemPreview()
{
    using namespace ConsignmentLayout;
    const RECT clip=ItemBounds();if(IsRectEmpty(&clip)) return;
    const RECT border={clip.left-IconBorderX,clip.top-IconBorderY,clip.right+IconBorderX,clip.bottom+IconBorderY};
    DrawLegacyPanel(border,RGB(30,30,30),180,RGB(200,200,200));
    auto& selectedItem=registration?item:listings[selected].item;
    selectedItem.DrawItem(clip.left,clip.top,_ITEM_TYPE_INVENTORY,&clip);
}
void CConsignmentWnd::Show()
{
    using namespace ConsignmentLayout;
    if(!m_bActive) return;
    ReadEditors();LayoutEditors();
    const int x=m_rcWnd.left,y=m_rcWnd.top;
    const auto& layout=registration?Registration:Browser;
    const auto frame=[&](int id,int left,int top) {DrawSelectionFrame(*m_pxWndImage,id,x+left,y+top,SelectionBlend::Normal,false);};
    if(m_pxWndImage->NewSetIndex(layout.frame)) {
        const auto image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        frame(layout.frame,-(image.shWidth-layout.width)/2,-(image.shHeight-layout.height)/2);
    }
    for(const auto& button:Buttons()) frame(pressed==int(button.action)?button.pressed:button.normal,button.x,button.y);
    if(!registration) {
        const DWORD tick=GetTickCount();
        if((waiting || animationFrame) && tick-animationTick>DWORD(waiting?BusyInterval:IdleInterval)) {
            animationFrame=(animationFrame+1)%ActivityFrames;animationTick=tick;
        }
        frame(ActivityFirstFrame+animationFrame,390,14);
        for(int row=0;row<VisibleRows;++row) {
            const RECT area={x+FirstRow.left,y+FirstRow.top+row*RowHeight,x+FirstRow.right,y+FirstRow.bottom+row*RowHeight};
            if(row==hovered) DrawLegacyPanel(area,RGB(30,30,30),180,0,false);
            if(row==selected) DrawLegacyPanel(area,RGB(35,114,156),180,0,false);
        }
    }
    DrawItemPreview();
    static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
    HDC dc;auto* surface=g_xMainWnd.GetBackBuffer();if(FAILED(surface->GetDC(&dc))) return;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font=g_xMainWnd.CreateGameFont(face,9);const auto old=SelectObject(dc,font);SetBkMode(dc,TRANSPARENT);
    const auto text=[&](const std::string& value,Box box,COLORREF color=TextColor,UINT flags=DT_LEFT|DT_SINGLELINE|DT_VCENTER) {
        RECT area={x+box.left,y+box.top,x+box.right,y+box.bottom};SetTextColor(dc,color);DrawTextA(dc,value.data(),int(value.size()),&area,flags);
    };
    // Capture only the window skin for the native edit background.
    const Box editBoxes[]={registration?Description:NameQuery,SellerQuery};
    for(int i=0;i<(registration?1:2);++i) if(edits[i] && !editBackground[i]) {
        RECT bounds;GetClientRect(edits[i],&bounds);HDC memory=CreateCompatibleDC(dc);
        HBITMAP bitmap=CreateCompatibleBitmap(dc,bounds.right,bounds.bottom);const auto previous=SelectObject(memory,bitmap);
        const auto box=editBoxes[i];
        StretchBlt(memory,0,0,bounds.right,bounds.bottom,dc,x+box.left,y+box.top,box.right-box.left,box.bottom-box.top,SRCCOPY);
        editBackground[i]=CreatePatternBrush(bitmap);SelectObject(memory,previous);DeleteObject(bitmap);DeleteDC(memory);InvalidateRect(edits[i],nullptr,TRUE);
    }
    if(registration) {
        text(captions.Get(RegistrationTitleText),RegistrationTitle,TitleColor,DT_CENTER|DT_SINGLELINE|DT_VCENTER);
        if(hasItem) {if(price) text(CExchangeWnd::Amount(price),PriceText);}
        else {
            text(captions.Get(ItemInstruction),{ItemArea.left,107,390,123},TextColor,DT_CENTER|DT_SINGLELINE);
            text(captions.Get(DropInstruction),{ItemArea.left,127,390,143},TextColor,DT_CENTER|DT_SINGLELINE);
        }
        if(!editorsAvailable) text(description,Description,TextColor,DT_LEFT|DT_WORDBREAK);
    } else {
        text(captions.Get(BrowserTitleText),BrowserTitle,TitleColor,DT_CENTER|DT_SINGLELINE|DT_VCENTER);
        text(captions.Get(ServerColumn),{16,56,118,71});text(captions.Get(ItemColumn),{121,56,257,71});
        text(captions.Get(PriceColumn),{261,56,328,71});text(captions.Get(SellerColumn),{331,56,468,71});
        if(page || pages) text(std::to_string(page)+" / "+std::to_string(pages),{124,18,290,33});
        for(int row=0;row<min(VisibleRows,int(listings.size()));++row) {
            const auto& listing=listings[row];const int top=86+row*RowHeight,bottom=101+row*RowHeight;
            text(listing.server,{16,top,118,bottom});text(listing.item.Name(),{121,top,257,bottom});
            text(CExchangeWnd::Amount(listing.price),{261,top,328,bottom});text(listing.seller,{331,top,468,bottom});
        }
        if(selected>=0 && selected<int(listings.size())) {
            const auto& listing=listings[selected];
            text(listing.seller,{492,25,638,39});text(listing.date,{642,25,708,39});
            const auto lines=DescriptionLines(dc,listing.description);
            for(size_t i=0;i<lines.size();++i) {
                const auto& line=lines[i];const int left=x+BrowserDescription.left,top=y+BrowserDescription.top+int(i)*DescriptionLineHeight;
                SetTextColor(dc,0);
                TextOutA(dc,left-1,top,line.data(),int(line.size()));TextOutA(dc,left+1,top,line.data(),int(line.size()));
                TextOutA(dc,left,top-1,line.data(),int(line.size()));TextOutA(dc,left,top+1,line.data(),int(line.size()));
                SetTextColor(dc,TextColor);TextOutA(dc,left,top,line.data(),int(line.size()));
            }
        }
        if(!editorsAvailable) {text(nameQuery,NameQuery);text(sellerQuery,SellerQuery);}
    }
    SelectObject(dc,old);DeleteObject(font);surface->ReleaseDC(dc);
}

void CConsignmentWnd::ReadEditors()
{
    std::string* values[]={registration?&description:&nameQuery,&sellerQuery};
    for(int i=0;i<(registration?1:2);++i) if(edits[i]) {
        std::vector<char> buffer(GetWindowTextLengthA(edits[i])+1);
        GetWindowTextA(edits[i],buffer.data(),int(buffer.size()));
        const auto next=LimitAnsiEdit(*values[i],buffer.data(),registration?ConsignmentLayout::DescriptionLimit:ConsignmentLayout::QueryLimit);
        if(next!=buffer.data()) SetWindowTextA(edits[i],next.c_str());
        *values[i]=next;
    }
}
void CConsignmentWnd::SyncEditors(bool available)
{
    editorsAvailable=available;LayoutEditors();
}
void CConsignmentWnd::LayoutEditors()
{
    using namespace ConsignmentLayout;
    if(!m_bActive || !editorsAvailable) {
        for(HWND edit:edits) if(edit) ShowWindow(edit,SW_HIDE);
        return;
    }
    const auto display=g_xMainWnd.m_rcWindow;
    const int width=display.right-display.left,height=display.bottom-display.top;
    if(width<=0 || height<=0) return;
    const int pixels=MulDiv(FontPixels,height,CanvasHeight);
    if(pixels!=fontHeight) {
        const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
        HFONT next=CreateFontA(-pixels,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,face);
        for(HWND edit:edits) if(edit) SendMessage(edit,WM_SETFONT,(WPARAM)next,TRUE);
        if(editFont) DeleteObject(editFont);editFont=next;fontHeight=pixels;
    }
    const Box boxes[]={registration?Description:NameQuery,SellerQuery};
    const std::string* values[]={registration?&description:&nameQuery,&sellerQuery};
    for(int i=0;i<(registration?1:2);++i) {
        if(!edits[i]) {
            const DWORD style=WS_POPUP|ES_LEFT|(registration?ES_MULTILINE|ES_AUTOVSCROLL|ES_WANTRETURN:ES_AUTOHSCROLL);
            edits[i]=CreateWindowExA(0,"EDIT",values[i]->c_str(),style,0,0,0,0,g_xMainWnd.GetSafehWnd(),nullptr,GetModuleHandle(nullptr),nullptr);
            SetWindowLongPtr(edits[i],GWLP_USERDATA,(LONG_PTR)this);
            editProcedure=(WNDPROC)SetWindowLongPtr(edits[i],GWLP_WNDPROC,(LONG_PTR)EditorProc);
            SendMessage(edits[i],WM_SETFONT,(WPARAM)editFont,TRUE);
            SendMessage(edits[i],EM_LIMITTEXT,registration?DescriptionLimit:QueryLimit,0);
            SendMessage(edits[i],EM_SETMARGINS,EC_LEFTMARGIN|EC_RIGHTMARGIN,0);
        }
        const auto box=boxes[i];
        const int left=display.left+MulDiv(m_rcWnd.left+box.left,width,CanvasWidth);
        const int top=display.top+MulDiv(m_rcWnd.top+box.top,height,CanvasHeight);
        const int w=MulDiv(box.right-box.left,width,CanvasWidth),h=MulDiv(box.bottom-box.top,height,CanvasHeight);
        RECT previous;GetWindowRect(edits[i],&previous);
        if(previous.left!=left || previous.top!=top || previous.right!=left+w || previous.bottom!=top+h) {
            MoveWindow(edits[i],left,top,w,h,TRUE);
            if(editBackground[i]) DeleteObject(editBackground[i]);editBackground[i]=nullptr;
        }
        ShowWindow(edits[i],SW_SHOWNOACTIVATE);
    }
}
HBRUSH CConsignmentWnd::EditColor(HWND edit,HDC dc)
{
    SetBkMode(dc,TRANSPARENT);SetTextColor(dc,ConsignmentLayout::TextColor);SetBrushOrgEx(dc,0,0,nullptr);
    const int index=edit==edits[1]?1:0;
    return editBackground[index]?editBackground[index]:(HBRUSH)GetStockObject(BLACK_BRUSH);
}
LRESULT CALLBACK CConsignmentWnd::EditorProc(HWND window,UINT message,WPARAM key,LPARAM data)
{
    auto* self=(CConsignmentWnd*)GetWindowLongPtr(window,GWLP_USERDATA);
    if(message==WM_KEYDOWN) {
        if(key==VK_ESCAPE) {g_xGameProc.m_xInterface.WindowActivate(self->m_nID);return 0;}
        if(!self->registration && key==VK_TAB) {SetFocus(window==self->edits[0]?self->edits[1]:self->edits[0]);return 0;}
        if(!self->registration && key==VK_RETURN) {
            self->Activate(window==self->edits[0]?ConsignmentLayout::Action::SearchName:ConsignmentLayout::Action::SearchSeller);return 0;
        }
    }
    if(message==WM_CHAR && (key==VK_ESCAPE || (!self->registration && (key==VK_TAB || key==VK_RETURN)))) return 0;
    return CallWindowProc(self->editProcedure,window,message,key,data);
}
void CConsignmentWnd::Destroy()
{
    for(auto& edit:edits) {if(edit) DestroyWindow(edit);edit=nullptr;}
    for(auto& brush:editBackground) {if(brush) DeleteObject(brush);brush=nullptr;}
    if(editFont) DeleteObject(editFont);editFont=nullptr;editProcedure=nullptr;fontHeight=0;
    listings.clear();requests.clear();nameQuery.clear();sellerQuery.clear();description.clear();
    selected=hovered=pressed=pressedRow=-1;page=pages=0;price=sessionId=0;
    captured=dragging=waiting=hasItem=confirmationPending=false;editorsAvailable=true;
    animationFrame=0;animationTick=actionTick=searchTick=0;
    CGameWnd::Destroy();
}
void CConsignmentWnd::Close()
{
    ReadEditors();m_bActive=FALSE;captured=dragging=false;pressed=pressedRow=-1;sessionId=0;actionTick=searchTick=0;
    if(registration) {hasItem=false;price=0;description.clear();if(edits[0]) SetWindowTextA(edits[0],"");}
    else {listings.clear();selected=hovered=-1;page=pages=0;waiting=false;animationFrame=0;}
    LayoutEditors();SetFocus(g_xMainWnd.GetSafehWnd());
}
bool CConsignmentWnd::SelectItem(POINT point)
{
    using namespace ConsignmentLayout;
    auto& ui=g_xGameProc.m_xInterface;auto& held=ui.m_stCommonItem;
    const RECT area={m_rcWnd.left+ItemArea.left,m_rcWnd.top+ItemArea.top,m_rcWnd.left+ItemArea.right,m_rcWnd.top+ItemArea.bottom};
    if(!registration || !PtInRect(&area,point) || !held.bSetted || held.bWaitResponse || held.bIsEquipItem || held.bIsBeltItem || held.bIsHideItem) return false;
    static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
    if(held.xItem.Name()==captions.Get(GoldCaption)) return false;
    if(!ui.m_xInventoryWnd.AddInvenItem(held.xItem,held.xItem.m_shCellNum,TRUE)) return false;
    item=held.xItem;hasItem=true;held.bSetted=FALSE;
    g_xSound.PlayItemEffect(item.m_stItemInfo.stStdItem.bStdMode,item.m_stItemInfo.stStdItem.bShape,item.Name());
    return true;
}
bool CConsignmentWnd::SubmitPrice(const char* value)
{
    if(!m_bActive || !registration || !*value) return false;
    for(const char* p=value;*p;++p) if(*p<'0' || *p>'9') return false;
    const auto parsed=std::strtoull(value,nullptr,10);
    if(parsed>ConsignmentLayout::MaximumPrice) return false;
    if(parsed) price=unsigned(parsed);return true;
}
void CConsignmentWnd::Pointer(int kind,POINT point)
{
    using namespace ConsignmentLayout;
    if(kind==1) {hovered=RowAt(point);if(dragging) {MoveGameWnd(point);LayoutEditors();}return;}
    const int hit=ButtonAt(point),row=RowAt(point);
    if(kind==0) {
        captured=true;pressed=hit;pressedRow=row;
        dragging=hit<0 && point.y>=m_rcWnd.top && point.y<m_rcWnd.top+HeaderHeight;
        if(dragging) {m_bFocused=TRUE;SetReadyMove(point);}
        return;
    }
    if(!captured) return;
    const bool activate=hit>=0 && hit==pressed;
    captured=dragging=false;pressed=-1;
    if(activate) {g_xSound.PlayLobbyEffect("105.wav");Activate(Action(hit));return;}
    if(registration) {
        if(SelectItem(point)) return;
        const RECT hitPrice={m_rcWnd.left+PriceHit.left,m_rcWnd.top+PriceHit.top,m_rcWnd.left+PriceHit.right,m_rcWnd.top+PriceHit.bottom};
        if(PtInRect(&hitPrice,point)) g_xGameProc.m_xInterface.OpenNameDialog(PriceDialog);
    } else if(row>=0 && row==pressedRow) selected=row;
    pressedRow=-1;
}
void CConsignmentWnd::Activate(ConsignmentLayout::Action action,DWORD now)
{
    using namespace ConsignmentLayout;
    auto& ui=g_xGameProc.m_xInterface;
    if(action==Action::Close) {Close();return;}
    if(action==Action::Register) {ui.WindowActivate(RegistrationId);ui.m_xConsignmentRegistration.sessionId=sessionId;return;}
    if(action==Action::CancelRegistration) {hasItem=false;price=0;return;}
    const bool search=action==Action::SearchName || action==Action::SearchSeller;
    const bool browse=action==Action::NextPage || action==Action::PreviousPage || action==Action::Refresh;
    const bool transaction=action==Action::Buy || action==Action::CancelListing;
    if(search) {if(DWORD(now-searchTick)<=RequestInterval) return;searchTick=now;}
    if(browse || transaction) {if(DWORD(now-actionTick)<=RequestInterval) return;if(browse) actionTick=now;}
    ReadEditors();Request request;request.action=action;request.page=page;request.sessionId=sessionId;
    if(action==Action::NextPage) request.page=page+1;
    if(action==Action::PreviousPage) request.page=page-1;
    if(action==Action::SearchName) request.query=nameQuery;
    if(action==Action::SearchSeller) request.query=sellerQuery;
    int caption=0,dialog=0;
    if(action==Action::Buy || action==Action::CancelListing) {
        if(selected<0 || selected>=int(listings.size())) return;
        const auto& listing=listings[selected];request.id=listing.id;request.price=listing.price;request.item=listing.item;
        if(action==Action::Buy && listing.price>g_xGameProc.m_xMyHero.m_nGlod) {ui.ShowNotice(InsufficientGold);return;}
        caption=action==Action::Buy?PurchaseQuestion:CancelListingQuestion;
        dialog=action==Action::Buy?PurchaseDialog:CancelDialog;
    }
    if(action==Action::SubmitRegistration) {
        if(!hasItem || !price) return;
        request.item=item;request.price=price;request.description=description;
        caption=RegistrationQuestion;dialog=RegistrationDialog;
    }
    if(caption) {
        static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
        confirmation=std::move(request);confirmationPending=true;
        ui.m_nIdxMsgBox=dialog;ui.m_xMsgBox.SetOrigin(220,112,false);
        ui.PrepareModal();
        ui.m_xMsgBox.ShowMessageBox(captions.Get(caption).c_str(),MSG_BTN_YESNO,ConfirmationFrame);
        return;
    }
    SendRequest(std::move(request));
}
void CConsignmentWnd::Confirm(bool accepted,DWORD now)
{
    if(!confirmationPending) return;
    if(accepted && confirmation.action==ConsignmentLayout::Action::CancelListing && DWORD(now-actionTick)<=ConsignmentLayout::RequestInterval) accepted=false;
    if(accepted) {
        if(confirmation.action==ConsignmentLayout::Action::Buy || confirmation.action==ConsignmentLayout::Action::CancelListing) actionTick=now;
        SendRequest(std::move(confirmation));
    }
    confirmationPending=false;
}
void CConsignmentWnd::ShowItemHint(POINT point)
{
    auto& ui=g_xGameProc.m_xInterface;
    if(!m_bActive || ui.m_xMsgBox.IsActive() || ui.m_xGuildEditor.m_bActive || ui.m_stCommonItem.bSetted || ui.GetWindowInMousePos(point)!=m_nID) return;
    const RECT icon=ItemBounds();if(IsRectEmpty(&icon)) return;
    if(registration) {
        const auto box=ConsignmentLayout::ItemArea;
        const RECT area={m_rcWnd.left+box.left,m_rcWnd.top+box.top,m_rcWnd.left+box.right,m_rcWnd.top+box.bottom};
        if(PtInRect(&area,point)) item.ShowItemStatus(icon.left+25,icon.top+22);
    } else if(RowAt(point)==selected) listings[selected].item.ShowItemStatus(point.x+20,point.y+22);
    else {
        const RECT area={icon.left-15,icon.top-7,icon.right+15,icon.bottom+7};
        if(PtInRect(&area,point)) listings[selected].item.ShowItemStatus(area.left+35,area.top+22);
    }
}

void CConsignmentWnd::ReceiveList(const char* encoded,int count,int currentPage,int totalPages)
{
    using namespace ConsignmentWire;
    listings.clear();selected=hovered=-1;waiting=false;
    if(count<=0) return;
    page=currentPage;pages=totalPages;
    const char* begin=encoded;
    for(int index=0;index<count;++index) {
        const char* end=strchr(begin,'/');if(!end) break;
        std::string part(begin,end);BYTE record[RecordBytes]={};begin=end+1;
        if(fnDecode6BitBuf(part.data(),(char*)record,RecordBytes)!=RecordBytes) continue;
        const auto text=[&](int offset,int length) {
            const char* value=(char*)record+offset;
            const char* terminator=(const char*)memchr(value,0,length);
            return std::string(value,terminator?int(terminator-value):length);
        };
        Listing listing;
        memcpy(&listing.id,record+Id,sizeof(listing.id));
        memcpy(&listing.price,record+Price,sizeof(listing.price));
        listing.server=text(Server,ServerBytes);listing.seller=text(Seller,SellerBytes);
        listing.date=text(Date,DateBytes);listing.description=text(Description,DescriptionBytes);
        listing.item.SetMir3Item(record+Item,0);
        listings.push_back(std::move(listing));
    }
    if(!listings.empty()) selected=0;
}

bool CConsignmentWnd::BuildRequest(const Request& request,_TDEFAULTMESSAGE& header,std::string& body)
{
    using ConsignmentLayout::Action;
    using namespace ConsignmentWire;
    WORD message=0;DWORD parameter=0;body.clear();
    switch(request.action) {
    case Action::NextPage:message=NextPage;break;
    case Action::PreviousPage:message=PreviousPage;break;
    case Action::Refresh:message=Refresh;break;
    case Action::SearchName:case Action::SearchSeller: {
        message=Search;
        const auto query=request.query.empty()?"*":request.query;
        body=request.action==Action::SearchName?"*\r"+query+"\r0":query+"\r*\r0";
        break;
    }
    case Action::Buy:message=Buy;parameter=request.id;body=request.item.Name();break;
    case Action::CancelListing:message=Cancel;parameter=request.id;break;
    case Action::SubmitRegistration: {
        message=Register;
        parameter=request.item.m_stackMode>=2?request.item.m_count:0;
        const auto name=LimitAnsiEdit("",request.item.Name(),RegistrationNameLimit);
        const auto description=LimitAnsiEdit("",CGuildEditor::Payload(request.description),RegistrationDescriptionLimit);
        body=name+"\r"+std::to_string(request.item.m_stItemInfo.nMakeIndex)+"\r"+std::to_string(request.price)+"\r"+description;
        break;
    }
    default:return false;
    }
    fnMakeDefMessage(&header,message,request.sessionId,LOWORD(parameter),HIWORD(parameter),0);
    return true;
}
void CConsignmentWnd::SendRequest(Request request)
{
    _TDEFAULTMESSAGE header={};std::string body;
    if(!BuildRequest(request,header,body)) return;
    if(g_xClientSocket.m_sockClient!=INVALID_SOCKET)
        g_xClientSocket.SendPacket(&header,body.empty()?nullptr:body.data());
    const bool registered=request.action==ConsignmentLayout::Action::SubmitRegistration;
    waiting=true;requests.push_back(std::move(request));
    if(registered && m_bActive) g_xGameProc.m_xInterface.WindowActivate(ConsignmentLayout::RegistrationId);
}

bool CConsignmentWnd::KeyDown(WPARAM key,DWORD now)
{
    if(!m_bActive || registration) return false;
    using namespace ConsignmentLayout;
    switch(key) {
    case VK_DOWN:selected=min(selected+1,VisibleRows-1);return true;
    case VK_UP:selected=max(selected-1,0);return true;
    case VK_RIGHT:Activate(Action::NextPage,now);return true;
    case VK_LEFT:Activate(Action::PreviousPage,now);return true;
    default:return false;
    }
}
