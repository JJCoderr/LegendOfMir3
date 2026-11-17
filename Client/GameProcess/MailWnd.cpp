#include "StdAfx.h"
#include "MailWnd.h"
#include "Common/LegacyPanel.h"
#include "Common/LoginMessages.h"
#include "Common/AnsiInput.h"
#include <cstdlib>

void CMailWnd::Create(CWHWilImageData* image,int id)
{
    CreateGameWnd(id,image,MailLayout::WindowFrame,TRUE,0,0,MailLayout::WindowWidth,MailLayout::WindowHeight);
}
void CMailWnd::SelectView(MailLayout::View next)
{
    ReadEditors();
    if(view==MailLayout::View::Compose) {composeRecipient=recipient;composeDraft=draft;}
    if(view==MailLayout::View::Gift) {giftRecipient=recipient;giftDraft=draft;}
    view=next;pressed=-1;hoverRow=-1;scrolling=false;
    if(view==MailLayout::View::Compose) {recipient=composeRecipient;draft=composeDraft;}
    if(view==MailLayout::View::Gift) {recipient=giftRecipient;draft=giftDraft;}
    for(auto& brush:editBackground) {if(brush) DeleteObject(brush);brush=nullptr;}
    if(edits[0]) SetWindowTextA(edits[0],recipient.c_str());
    if(edits[1]) SetWindowTextA(edits[1],draft.c_str());
    LayoutEditors();
    if(next!=MailLayout::View::Compose && next!=MailLayout::View::Gift) SetFocus(g_xMainWnd.GetSafehWnd());
}
std::vector<MailLayout::Button> CMailWnd::VisibleButtons() const
{
    using namespace MailLayout;
    std::vector<Button> result;
    for(auto button:Buttons) {
        const auto action=button.action;
        bool visible=action==Action::Close || (action>=Action::Outbox && action<=Action::Inbox);
        switch(view) {
        case View::Inbox:
            if(action==Action::Refresh) {visible=true;button.x=286;}
            if(action==Action::Delete) {visible=true;button.x=326;}
            break;
        case View::Read: visible=visible || action==Action::DeleteRead || action==Action::Reply || action==Action::Next || action==Action::Previous;break;
        case View::Compose: visible=visible || action==Action::CancelCompose || action==Action::SendCompose;break;
        case View::Gift: visible=visible || action==Action::CancelGift || action==Action::SendGift;break;
        case View::Outbox:
            if(action==Action::Refresh) {visible=true;button.x=246;}
            if(action==Action::Delete) {visible=true;button.x=326;}
            visible=visible || action==Action::CancelOutbox;break;
        }
        if(visible) result.push_back(button);
    }
    return result;
}
int CMailWnd::ButtonAt(POINT point) const
{
    const auto buttons=VisibleButtons();
    for(const auto& button:buttons) {
        if(!m_pxWndImage->NewSetIndex(button.normal)) continue;
        const auto& image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        const RECT box={m_rcWnd.left+button.x,m_rcWnd.top+button.y,m_rcWnd.left+button.x+image.shWidth,m_rcWnd.top+button.y+image.shHeight};
        if(PtInRect(&box,point)) return int(button.action);
    }
    return -1;
}
void CMailWnd::Show()
{
    using namespace MailLayout;
    if(!m_bActive) return;ReadEditors();LayoutEditors();LayoutBody();
    const int x=m_rcWnd.left,y=m_rcWnd.top;
    const auto frame=[&](int id,int left,int top) {DrawSelectionFrame(*m_pxWndImage,id,x+left,y+top,SelectionBlend::Normal,false);};
    if(m_pxWndImage->NewSetIndex(WindowFrame)) {
        const auto image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        frame(WindowFrame,-(image.shWidth-WindowWidth)/2,-(image.shHeight-WindowHeight)/2);
    }
    if(view==View::Read) frame(ReadFrameBackground,1,51);
    if(view==View::Compose) frame(ComposeFrame,1,51);
    if(view==View::Gift) frame(GiftFrame,1,50);
    if(view==View::Outbox) frame(OutboxFrame,1,51);
    for(const auto& button:VisibleButtons()) frame(int(button.action)==pressed?button.pressed:button.normal,button.x,button.y);
    if(view==View::Inbox || view==View::Outbox) {
        const auto& messages=view==View::Inbox?inbox:outbox;
        const int first=view==View::Inbox?firstInbox:firstOutbox,selected=view==View::Inbox?selectedInbox:selectedOutbox;
        for(int row=0;row<VisibleRows && first+row<int(messages.size());++row) {
            const auto& message=messages[first+row];
            const RECT area={x+FirstRow.left,y+FirstRow.top+row*RowHeight,x+FirstRow.right,y+FirstRow.bottom+row*RowHeight};
            if(first+row==hoverRow) DrawLegacyPanel(area,RGB(30,30,30),180,0,false);
            if(first+row==selected) DrawLegacyPanel(area,RGB(35,114,156),180,0,false);
            if(first+row==(view==View::Inbox?checkedInbox:checkedOutbox)) frame(RowMarkerFrame,CheckBox.left,CheckBox.top+2+row*RowHeight);
            frame(message.attachment?(message.read?AttachmentReadFrame:AttachmentFrame):(message.read?ReadFrame:UnreadFrame),message.attachment?371:374,62+row*RowHeight+(message.attachment && message.read?2:0));
        }
        frame(ScrollFrame,ScrollX,ScrollTop());
    } else if(view==View::Read) {
        if(auto* message=CurrentMessage()) if(message->attachment) frame(AttachmentFrame,Attachment.left,Attachment.top);
        frame(ScrollFrame,ScrollX,ScrollTop());
    }
    if(view==View::Gift && hasGiftItem) DrawGift();
    static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
    HDC dc;auto* surface=g_xMainWnd.GetBackBuffer();if(FAILED(surface->GetDC(&dc))) return;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font=g_xMainWnd.CreateGameFont(face,9);const auto old=SelectObject(dc,font);SetBkMode(dc,TRANSPARENT);
    if(view==View::Compose || view==View::Gift) {
        const Box boxes[]={Recipient,view==View::Gift?GiftBody:ComposeBody};
        for(int i=0;i<2;++i) if(edits[i] && !editBackground[i]) {
            RECT bounds;GetClientRect(edits[i],&bounds);HDC memory=CreateCompatibleDC(dc);
            HBITMAP bitmap=CreateCompatibleBitmap(dc,bounds.right,bounds.bottom);auto previous=SelectObject(memory,bitmap);const auto& box=boxes[i];
            StretchBlt(memory,0,0,bounds.right,bounds.bottom,dc,x+box.left,y+box.top,box.right-box.left,box.bottom-box.top,SRCCOPY);
            editBackground[i]=CreatePatternBrush(bitmap);SelectObject(memory,previous);DeleteObject(bitmap);DeleteDC(memory);InvalidateRect(edits[i],nullptr,TRUE);
        }
    }
    const auto text=[&](const std::string& value,Box box,COLORREF color,UINT flags=DT_LEFT|DT_SINGLELINE|DT_VCENTER) {
        RECT area={x+box.left,y+box.top,x+box.right,y+box.bottom};SetTextColor(dc,color);DrawTextA(dc,value.c_str(),int(value.size()),&area,flags);
    };
    const int titles[]={InboxTitle,ReadTitle,ComposeTitle,GiftTitle,OutboxTitle};
    text(captions.Get(titles[int(view)]),Title,TitleColor,DT_CENTER|DT_SINGLELINE|DT_VCENTER);
    if(view==View::Inbox || view==View::Outbox) {
        const auto& messages=view==View::Inbox?inbox:outbox;const int first=view==View::Inbox?firstInbox:firstOutbox;
        for(int row=0;row<VisibleRows && first+row<int(messages.size());++row) {
            const auto& message=messages[first+row];const int offset=RowHeight*row;
            text(message.person,{43,65+offset,137,80+offset},TextColor);
            text(message.subject,{141,65+offset,362,80+offset},TextColor);
        }
    } else if(view==View::Read) {
        if(auto* message=CurrentMessage()) {
            text(message->person,Recipient,TextColor);
            text(message->date,{275,62,377,79},TextColor);
            const int saved=SaveDC(dc);
            IntersectClipRect(dc,x+BodyX-1,y+BodyY-1,x+BodyX+BodyWidth+1,y+BodyY+BodyRows*BodyLineHeight+1);
            for(int row=0;row<BodyRows && firstBody+row<int(bodyLines.size());++row) {
                const auto& line=bodyLines[firstBody+row];
                const int left=x+BodyX,top=y+BodyY+row*BodyLineHeight;
                SetTextColor(dc,0);
                TextOutA(dc,left-1,top,line.data(),int(line.size()));TextOutA(dc,left+1,top,line.data(),int(line.size()));
                TextOutA(dc,left,top-1,line.data(),int(line.size()));TextOutA(dc,left,top+1,line.data(),int(line.size()));
                SetTextColor(dc,TextColor);TextOutA(dc,left,top,line.data(),int(line.size()));
            }
            RestoreDC(dc,saved);
        }
    } else if(view==View::Gift && !hasGiftItem) {
        text(captions.Get(GiftInstruction),{GiftItem.left,105,GiftItem.right,120},TextColor,DT_CENTER|DT_SINGLELINE);
        text(captions.Get(GiftDropInstruction),{GiftItem.left,123,GiftItem.right,138},TextColor,DT_CENTER|DT_SINGLELINE);
    }
    if(view==View::Gift && giftIsGold && giftGold) text(std::to_string(giftGold),{GiftItem.left,153,GiftItem.right,168},TextColor,DT_CENTER|DT_SINGLELINE);
    if(!editorsAvailable && (view==View::Compose || view==View::Gift)) {
        text(recipient,Recipient,TextColor);
        text(draft,view==View::Gift?GiftBody:ComposeBody,TextColor,DT_LEFT|DT_WORDBREAK);
    }
    SelectObject(dc,old);DeleteObject(font);surface->ReleaseDC(dc);
}

void CMailWnd::ReadEditors()
{
    if(!edits[0] || (view!=MailLayout::View::Compose && view!=MailLayout::View::Gift)) return;
    std::string* values[]={&recipient,&draft};
    for(int i=0;i<2;++i) {
        std::vector<char> buffer(GetWindowTextLengthA(edits[i])+1);GetWindowTextA(edits[i],buffer.data(),int(buffer.size()));
        const auto accepted=LimitAnsiEdit(*values[i],buffer.data(),MailLayout::EditLimit);
        if(accepted!=buffer.data()) SetWindowTextA(edits[i],accepted.c_str());
        *values[i]=accepted;
    }
}
void CMailWnd::LayoutEditors()
{
    using namespace MailLayout;
    const bool visible=m_bActive && editorsAvailable && (view==View::Compose || view==View::Gift);
    if(!visible) {for(HWND edit:edits) if(edit) ShowWindow(edit,SW_HIDE);return;}
    const auto& display=g_xMainWnd.m_rcWindow;
    const int width=display.right-display.left,height=display.bottom-display.top;
    const Box boxes[]={Recipient,view==View::Gift?GiftBody:ComposeBody};
    const int pixels=MulDiv(FontPixels,height,CanvasHeight);
    if(editFontHeight!=pixels) {
        const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
        HFONT next=CreateFontA(-pixels,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,face);
        for(HWND edit:edits) if(edit) SendMessage(edit,WM_SETFONT,(WPARAM)next,TRUE);
        if(editFont) DeleteObject(editFont);editFont=next;editFontHeight=pixels;
    }
    for(int i=0;i<2;++i) {
        if(!edits[i]) {
            edits[i]=CreateWindowExA(0,"EDIT",i?draft.c_str():recipient.c_str(),WS_POPUP|ES_LEFT|(i?ES_MULTILINE|ES_AUTOVSCROLL|ES_WANTRETURN:ES_AUTOHSCROLL),0,0,0,0,g_xMainWnd.GetSafehWnd(),nullptr,GetModuleHandle(nullptr),nullptr);
            SetWindowLongPtr(edits[i],GWLP_USERDATA,(LONG_PTR)this);
            editProcedure=(WNDPROC)SetWindowLongPtr(edits[i],GWLP_WNDPROC,(LONG_PTR)EditorProc);
            SendMessage(edits[i],EM_LIMITTEXT,EditLimit,0);SendMessage(edits[i],WM_SETFONT,(WPARAM)editFont,TRUE);
            SendMessage(edits[i],EM_SETMARGINS,EC_LEFTMARGIN|EC_RIGHTMARGIN,0);
        }
        const auto& box=boxes[i];
        const int left=display.left+MulDiv(m_rcWnd.left+box.left,width,CanvasWidth),top=display.top+MulDiv(m_rcWnd.top+box.top,height,CanvasHeight);
        const int w=MulDiv(box.right-box.left,width,CanvasWidth),h=MulDiv(box.bottom-box.top,height,CanvasHeight);
        RECT old;GetWindowRect(edits[i],&old);
        if(old.left!=left || old.top!=top || old.right!=left+w || old.bottom!=top+h) {
            MoveWindow(edits[i],left,top,w,h,TRUE);if(editBackground[i]) DeleteObject(editBackground[i]);editBackground[i]=nullptr;
        }
        ShowWindow(edits[i],SW_SHOWNOACTIVATE);
    }
}
HBRUSH CMailWnd::EditColor(HWND edit,HDC dc)
{
    SetBkMode(dc,TRANSPARENT);SetTextColor(dc,MailLayout::TextColor);SetBrushOrgEx(dc,0,0,nullptr);
    const int index=edit==edits[1]?1:0;return editBackground[index]?editBackground[index]:(HBRUSH)GetStockObject(BLACK_BRUSH);
}
void CMailWnd::Destroy()
{
    for(auto& edit:edits) {if(edit) DestroyWindow(edit);edit=nullptr;}
    for(auto& brush:editBackground) {if(brush) DeleteObject(brush);brush=nullptr;}
    if(editFont) DeleteObject(editFont);
    editFont=nullptr;editFontHeight=0;editProcedure=nullptr;
    inbox.clear();outbox.clear();requests.clear();bodyLines.clear();wrappedBody.clear();
    recipient.clear();draft.clear();composeRecipient.clear();composeDraft.clear();giftRecipient.clear();giftDraft.clear();
    view=MailLayout::View::Inbox;
    firstInbox=firstOutbox=firstBody=0;
    selectedInbox=selectedOutbox=checkedInbox=checkedOutbox=pressed=pressedRow=hoverRow=-1;
    dragging=captured=scrolling=readSent=pressedCheck=hasGiftItem=giftIsGold=false;
    giftGold=0;editorsAvailable=true;
    CGameWnd::Destroy();
}
void CMailWnd::Close()
{
    ReadEditors();
    // MIR3 45C450 clears both body edit controls when closing the window.
    draft.clear();composeDraft.clear();giftDraft.clear();
    if(edits[1]) SetWindowTextA(edits[1],"");
    m_bActive=FALSE;pressed=-1;dragging=captured=scrolling=false;
    hasGiftItem=giftIsGold=false;giftGold=0;
    LayoutEditors();SetFocus(g_xMainWnd.GetSafehWnd());
}
CMailWnd::Message* CMailWnd::CurrentMessage()
{
    const bool sent=view==MailLayout::View::Outbox || (view==MailLayout::View::Read && readSent);
    auto& messages=sent?outbox:inbox;
    const int selected=sent?selectedOutbox:selectedInbox;
    return selected>=0 && selected<int(messages.size())?&messages[selected]:nullptr;
}
void CMailWnd::OpenSelected()
{
    auto* message=CurrentMessage();
    if(!message) return;
    if(view!=MailLayout::View::Read) readSent=view==MailLayout::View::Outbox;
    if(!message->loaded) RequestAction(MailLayout::Action::Read);
    // The network layer fills loaded/body; empty message bodies are valid.
    if(message->loaded) {message->read=true;firstBody=0;wrappedBody.clear();bodyLines.clear();SelectView(MailLayout::View::Read);LayoutBody();}
}
void CMailWnd::Wheel(int delta)
{
    using namespace MailLayout;
    if(view==View::Read) {firstBody=max(0,min(firstBody+delta,max(0,int(bodyLines.size())-1)));return;}
    if(view!=View::Inbox && view!=View::Outbox) return;
    const auto& messages=view==View::Inbox?inbox:outbox;
    int& first=view==View::Inbox?firstInbox:firstOutbox;
    first=max(0,min(first+delta,max(0,int(messages.size())-1)));
}
int CMailWnd::RowAt(POINT point) const
{
    using namespace MailLayout;
    if(view!=View::Inbox && view!=View::Outbox) return -1;
    const int x=point.x-m_rcWnd.left,y=point.y-m_rcWnd.top;
    if(x<CheckBox.left || x>=FirstRow.right || y<FirstRow.top) return -1;
    const int row=(y-FirstRow.top)/RowHeight;
    const int first=view==View::Inbox?firstInbox:firstOutbox;
    const auto& messages=view==View::Inbox?inbox:outbox;
    return row<VisibleRows && first+row<int(messages.size())?first+row:-1;
}
void CMailWnd::Pointer(int kind,POINT point)
{
    using namespace MailLayout;
    if(Scroll(kind,point)) return;
    if(kind==1) {hoverRow=RowAt(point);if(dragging) {MoveGameWnd(point);LayoutEditors();}return;}
    const int hit=ButtonAt(point);
    if(kind==0) {
        captured=true;pressedRow=RowAt(point);pressedCheck=point.x-m_rcWnd.left>=CheckBox.left && point.x-m_rcWnd.left<CheckBox.right;pressed=hit;const RECT header={m_rcWnd.left,m_rcWnd.top,m_rcWnd.right,m_rcWnd.top+HeaderHeight};
        dragging=hit<0 && PtInRect(&header,point);if(dragging) {m_bFocused=TRUE;SetReadyMove(point);}return;
    }
    if(!captured) return;
    const bool activate=hit>=0 && hit==pressed;pressed=-1;dragging=false;captured=false;
    if(activate) {
        g_xSound.PlayLobbyEffect("105.wav");const auto action=Action(hit);
        switch(action) {
        case Action::Close:Close();return;
        case Action::Inbox:SelectView(View::Inbox);return;
        case Action::Outbox:SelectView(View::Outbox);return;
        case Action::Read:SelectView(View::Read);return;
        case Action::Previous:case Action::Next: {
            auto& messages=readSent?outbox:inbox;int& selected=readSent?selectedOutbox:selectedInbox;
            const int next=selected+(action==Action::Next?1:-1);
            if(next>=0 && next<int(messages.size())) {selected=next;OpenSelected();}
            return;
        }
        case Action::Compose:SelectView(View::Compose);return;
        case Action::Gift:SelectView(View::Gift);return;
        case Action::CancelCompose:case Action::CancelGift:
            recipient.clear();draft.clear();
            if(edits[0]) SetWindowTextA(edits[0],"");
            if(edits[1]) SetWindowTextA(edits[1],"");
            if(action==Action::CancelGift) {giftGold=0;hasGiftItem=giftIsGold=false;}
            return;
        case Action::Reply:
            if(readSent) return;
            if(auto* message=CurrentMessage()) composeRecipient=message->person;
            SelectView(View::Compose);return;
        default:RequestAction(action);return;
        }
    }
    if(view==View::Gift && SelectGift(point)) return;
    const int row=RowAt(point);
    if(row>=0 && row==pressedRow) {
        if(pressedCheck && (point.x-m_rcWnd.left<CheckBox.left || point.x-m_rcWnd.left>=CheckBox.right)) return;
        if(pressedCheck) {
            int& checked=view==View::Inbox?checkedInbox:checkedOutbox;
            checked=checked==row?-1:row;
        } else (view==View::Inbox?selectedInbox:selectedOutbox)=row;
    }
    pressedRow=-1;
}

int CMailWnd::ScrollTop() const
{
    using namespace MailLayout;
    const int count=view==View::Read?int(bodyLines.size()):int((view==View::Inbox?inbox:outbox).size());
    const int first=view==View::Read?firstBody:view==View::Inbox?firstInbox:firstOutbox;
    return ScrollY+(count>1?int(ScrollTravel*double(first)/(count-1)):0);
}
bool CMailWnd::Scroll(int kind,POINT point)
{
    using namespace MailLayout;
    if(view!=View::Inbox && view!=View::Outbox && view!=View::Read) return false;
    if(kind==2 && scrolling) {scrolling=false;captured=false;return true;}
    if(!m_pxWndImage->NewSetIndex(ScrollFrame)) return false;
    const auto image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
    const int x=point.x-m_rcWnd.left,y=point.y-m_rcWnd.top;
    const RECT track={ScrollX,ScrollY,ScrollX+image.shWidth,ScrollY+ScrollTravel+image.shHeight};
    if(kind==0 && PtInRect(&track,POINT{x,y})) {scrolling=true;captured=true;}
    if(!scrolling || kind==2) return false;
    const int count=view==View::Read?int(bodyLines.size()):int((view==View::Inbox?inbox:outbox).size());
    int& first=view==View::Read?firstBody:view==View::Inbox?firstInbox:firstOutbox;
    const double ratio=max(0.0,min(1.0,double(y-ScrollY)/(ScrollTravel+image.shHeight)));
    first=max(0,min(int(count*ratio),count-1));
    return true;
}

void CMailWnd::SyncEditors(bool available)
{
    editorsAvailable=available;
    LayoutEditors();
}
LRESULT CALLBACK CMailWnd::EditorProc(HWND window,UINT message,WPARAM key,LPARAM data)
{
    auto* self=(CMailWnd*)GetWindowLongPtr(window,GWLP_USERDATA);
    if(message==WM_KEYDOWN) {
        if(key==VK_ESCAPE) {g_xGameProc.m_xInterface.WindowActivate(_WND_ID_MAIL);return 0;}
        if(key==VK_TAB || (key==VK_RETURN && window==self->edits[0])) {
            SetFocus(window==self->edits[0]?self->edits[1]:self->edits[0]);return 0;
        }
    }
    if(message==WM_CHAR && (key==VK_TAB || key==VK_ESCAPE || (key==VK_RETURN && window==self->edits[0]))) return 0;
    return CallWindowProc(self->editProcedure,window,message,key,data);
}
void CMailWnd::LayoutBody()
{
    using namespace MailLayout;
    if(view!=View::Read) return;
    auto* message=CurrentMessage();
    const std::string body=message && message->loaded?message->body:std::string();
    if(body==wrappedBody) return;
    wrappedBody=body;bodyLines.clear();firstBody=0;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HDC dc=CreateCompatibleDC(nullptr);HFONT font=g_xMainWnd.CreateGameFont(face,9);const auto previous=SelectObject(dc,font);
    // MIR3 45BC70/45BEF0: CR-separated paragraphs, at most eleven wrapped rows per paragraph.
    size_t paragraph=0;
    while(paragraph<body.size()) {
        const auto delimiter=body.find_first_of("\r\n",paragraph);
        const size_t end=delimiter==std::string::npos?body.size():delimiter;
        size_t start=paragraph;int rows=0;
        while(start<end && rows<BodyRows) {
            size_t next=start,fit=start;
            while(next<end) {
                next+=IsDBCSLeadByteEx(CP_ACP,BYTE(body[next])) && next+1<end?2:1;
                SIZE extent={};GetTextExtentPoint32A(dc,body.data()+start,int(next-start),&extent);
                if(extent.cx>BodyWidth && fit>start) break;
                fit=next;
            }
            bodyLines.push_back(body.substr(start,fit-start));start=fit;++rows;
        }
        if(start==paragraph && start==end) bodyLines.emplace_back();
        if(delimiter==std::string::npos) break;
        paragraph=delimiter+1;
        if(body[delimiter]=='\r' && paragraph<body.size() && body[paragraph]=='\n') ++paragraph;
    }
    SelectObject(dc,previous);DeleteObject(font);DeleteDC(dc);
}

void CMailWnd::RequestAction(MailLayout::Action action)
{
    using namespace MailLayout;
    ReadEditors();
    Request request;request.action=action;
    request.sent=view==View::Outbox || (view==View::Read && readSent);
    switch(action) {
    case Action::Delete:case Action::CancelOutbox: {
        const auto& messages=request.sent?outbox:inbox;
        const int checked=request.sent?checkedOutbox:checkedInbox;
        if(checked<0 || checked>=int(messages.size())) return;
        request.messageId=messages[checked].id;
        break;
    }
    case Action::DeleteRead:case Action::Read:
        if(auto* message=CurrentMessage()) request.messageId=message->id;else return;
        break;
    case Action::SendCompose:case Action::SendGift:
        if(recipient.empty() || draft.empty()) return;
        request.recipient=recipient;request.body=draft;
        if(action==Action::SendGift) {
            if(!hasGiftItem || (giftIsGold && !giftGold)) return;
            if(giftIsGold) request.gold=giftGold;else memcpy(request.item.data(),giftItem.m_serverRecord,request.item.size());
        }
        break;
    case Action::Refresh:break;
    default:return;
    }
    // Keep an immutable GUI request until the mail protocol adapter consumes it.
    requests.push_back(std::move(request));
}

bool CMailWnd::SelectGift(POINT point)
{
    using namespace MailLayout;
    auto& ui=g_xGameProc.m_xInterface;auto& held=ui.m_stCommonItem;
    const RECT area={m_rcWnd.left+GiftItem.left,m_rcWnd.top+GiftItem.top,m_rcWnd.left+GiftItem.right,m_rcWnd.top+GiftItem.bottom};
    if(!PtInRect(&area,point) || !held.bSetted || held.bWaitResponse || held.bIsEquipItem || held.bIsBeltItem || held.bIsHideItem) return false;
    static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
    const bool gold=held.xItem.Name()==captions.Get(GoldCaption);
    // MIR3 keeps the selected item in the bag; the gift pane retains its record.
    if(!gold && !ui.m_xInventoryWnd.AddInvenItem(held.xItem,held.xItem.m_shCellNum,TRUE)) return false;
    giftItem=held.xItem;hasGiftItem=true;giftIsGold=gold;giftGold=0;
    held.bSetted=FALSE;
    g_xSound.PlayItemEffect(giftItem.m_stItemInfo.stStdItem.bStdMode,giftItem.m_stItemInfo.stStdItem.bShape,giftItem.Name());
    if(gold) ui.OpenNameDialog(GoldDialogAction);
    return true;
}
bool CMailWnd::SubmitGold(const char* text,unsigned balance)
{
    if(!m_bActive || view!=MailLayout::View::Gift || !giftIsGold || !*text) return false;
    for(const char* p=text;*p;++p) if(*p<'0' || *p>'9') return false;
    const auto amount=std::strtoull(text,nullptr,10);
    if(!amount || amount>balance) return false;
    giftGold=unsigned(amount);return true;
}
void CMailWnd::DrawGift()
{
    using namespace MailLayout;
    auto& icons=g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY];
    if(!icons.NewSetIndex(giftItem.m_stItemInfo.stStdItem.wLooks)) return;
    const auto info=*icons.m_lpstNewCurrWilImageInfo;
    const int left=m_rcWnd.left+GiftCenterX-info.shWidth/2;
    const int top=info.shHeight>GiftMaxHeight?78:m_rcWnd.top+GiftCenterY-info.shHeight/2;
    const int height=info.shHeight>GiftMaxHeight?GiftCroppedHeight:info.shHeight;
    const RECT border={left-15,top-7,left+info.shWidth+15,top+height+7};
    DrawLegacyPanel(border,RGB(30,30,30),180,RGB(200,200,200));
    const RECT clip={left,top,left+info.shWidth,top+height};
    giftItem.DrawItem(left,top,_ITEM_TYPE_INVENTORY,&clip);
}

void CMailWnd::ShowAttachmentHint(POINT point)
{
    using namespace MailLayout;
    auto& ui=g_xGameProc.m_xInterface;
    if(!m_bActive || ui.m_xMsgBox.IsActive() || ui.m_xGuildEditor.m_bActive || ui.m_stCommonItem.bSetted || ui.GetWindowInMousePos(point)!=_WND_ID_MAIL) return;
    const Box box=view==View::Gift?GiftItem:Attachment;
    const RECT bounds={m_rcWnd.left+box.left,m_rcWnd.top+box.top,m_rcWnd.left+box.right,m_rcWnd.top+box.bottom};
    if(!PtInRect(&bounds,point)) return;
    if(view==View::Gift && hasGiftItem && !giftIsGold) {
        giftItem.ShowItemStatus(bounds.left+15,bounds.top+15);
    } else if(view==View::Read) {
        if(auto* message=CurrentMessage()) if(message->item[0]) {
            CItem item;item.SetMir3Item(message->item.data(),0);item.ShowItemStatus(bounds.left+15,bounds.top+15);
        }
    }
}
