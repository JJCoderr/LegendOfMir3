#include "StdAfx.h"
#include "MailWire.h"
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
    if(view==View::Gift && giftIsGold && giftGoldConfirmed) text(std::to_string(giftGold),{GiftItem.left,153,GiftItem.right,168},TextColor,DT_CENTER|DT_SINGLELINE);
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
    if(width<=0 || height<=0) return;
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
    inbox.clear();outbox.clear();receivedBodies.clear();requests.clear();bodyLines.clear();wrappedBody.clear();
    recipient.clear();draft.clear();composeRecipient.clear();composeDraft.clear();giftRecipient.clear();giftDraft.clear();
    view=MailLayout::View::Inbox;
    firstInbox=firstOutbox=firstBody=0;
    selectedInbox=selectedOutbox=checkedInbox=checkedOutbox=pressed=pressedRow=hoverRow=-1;
    dragging=captured=scrolling=readSent=pressedCheck=hasGiftItem=giftIsGold=giftGoldConfirmed=false;
    giftGold=0;editorsAvailable=true;notified=false;
    CGameWnd::Destroy();
}
void CMailWnd::Close()
{
    ReadEditors();
    // MIR3 45C450 clears both body edit controls when closing the window.
    draft.clear();composeDraft.clear();giftDraft.clear();
    if(edits[1]) SetWindowTextA(edits[1],"");
    m_bActive=FALSE;pressed=-1;dragging=captured=scrolling=false;
    hasGiftItem=giftIsGold=giftGoldConfirmed=false;giftGold=0;
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
            if(action==Action::CancelGift) {giftGold=0;hasGiftItem=giftIsGold=giftGoldConfirmed=false;}
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
    const float ratio=count>1?float(double(first)/(count-1)):0;
    return ScrollY+int(ScrollTravel*double(ratio));
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
    const int count=view==View::Read?int(bodyLines.size()):int((view==View::Inbox?inbox:outbox).size());
    int& first=view==View::Read?firstBody:view==View::Inbox?firstInbox:firstOutbox;
    if(kind==0) {
        if(!PtInRect(&track,POINT{x,y})) return false;
        captured=true;pressed=pressedRow=-1;
        const int thumb=ScrollTop();
        if(y>=thumb && y<thumb+image.shHeight) scrolling=true;
        else {
            const float position=count>1?float(double(first)/(count-1)):0;
            const float page=count?float(double(VisibleRows)/count):0;
            const float next=position+(y<thumb+image.shHeight/2?-page:page);
            first=max(0,min(int(count*double(max(0.f,min(1.f,next)))),count-1));
        }
        return true;
    }
    if(!scrolling || kind!=1) return false;
    const float ratio=float(max(0.0,min(1.0,double(y-ScrollY)/(ScrollTravel+image.shHeight))));
    first=max(0,min(int(count*double(ratio)),count-1));
    return true;
}

void CMailWnd::SyncEditors(bool available)
{
    editorsAvailable=available && !g_xGameProc.m_xNotice.IsActive();
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
        const auto delimiter=body.find('\r',paragraph);
        const size_t end=delimiter==std::string::npos?body.size():delimiter;
        size_t cursor=paragraph;
        int rows=0;
        while(cursor<end && rows<BodyRows) {
            std::string line;
            bool lead=false;
            for(;;) {
                SIZE extent={};GetTextExtentPoint32A(dc,line.data(),int(line.size()),&extent);
                // 482240 measures the existing row before appending its next byte.
                if(extent.cx>=BodyWidth || cursor==end) {
                    if(lead) {line.pop_back();--cursor;}
                    break;
                }
                const char byte=body[cursor++];
                lead=lead?false:static_cast<signed char>(byte)<0;
                line+=byte;
            }
            bodyLines.push_back(std::move(line));++rows;
        }
        if(delimiter==std::string::npos) break;
        paragraph=delimiter+1;
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
    _TDEFAULTMESSAGE header={};
    if(BuildRequest(request,header) && g_xClientSocket.m_sockClient!=INVALID_SOCKET) {
        if(action==Action::SendCompose) {
            std::array<BYTE,MailWire::SendBytes> body={};
            if(BuildCompose(request,body)) g_xClientSocket.SendPacket(&header,(char*)body.data(),int(body.size()));
        } else g_xClientSocket.SendPacket(&header,nullptr);
    }
    // Retain the immutable operation; only server replies may change mail contents.
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
    giftItem=held.xItem;hasGiftItem=true;giftIsGold=gold;giftGold=0;giftGoldConfirmed=false;
    held.bSetted=FALSE;
    g_xSound.PlayItemEffect(giftItem.m_stItemInfo.stStdItem.bStdMode,giftItem.m_stItemInfo.stStdItem.bShape,giftItem.Name());
    if(gold) ui.OpenNameDialog(GoldDialogAction);
    return true;
}
bool CMailWnd::SubmitGold(const char* text)
{
    if(!m_bActive || view!=MailLayout::View::Gift || !giftIsGold || !*text) return false;
    for(const char* p=text;*p;++p) if(*p<'0' || *p>'9') return false;
    const auto amount=std::strtoull(text,nullptr,10);
    if(amount>MailLayout::MaximumGiftGold) return false;
    if(amount) giftGold=unsigned(amount);
    giftGoldConfirmed=true;return true;
}
RECT CMailWnd::GiftBounds()
{
    using namespace MailLayout;
    auto& icons=g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY];
    if(!icons.NewSetIndex(giftItem.m_stItemInfo.stStdItem.wLooks)) return {};
    const auto info=*icons.m_lpstNewCurrWilImageInfo;
    const int left=m_rcWnd.left+GiftCenterX-info.shWidth/2;
    // 459670 uses an absolute y=78 for oversized icons; retain that original behavior.
    const int top=info.shHeight>GiftMaxHeight?GiftCroppedTop:m_rcWnd.top+GiftCenterY-info.shHeight/2;
    const int height=info.shHeight>GiftMaxHeight?GiftCroppedHeight:info.shHeight;
    return {left,top,left+info.shWidth,top+height};
}
void CMailWnd::DrawGift()
{
    using namespace MailLayout;
    const RECT clip=GiftBounds();
    if(IsRectEmpty(&clip)) return;
    const RECT border={clip.left-GiftBorderX,clip.top-GiftBorderY,clip.right+GiftBorderX,clip.bottom+GiftBorderY};
    DrawLegacyPanel(border,RGB(30,30,30),180,RGB(200,200,200));
    giftItem.DrawItem(clip.left,clip.top,_ITEM_TYPE_INVENTORY,&clip);
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
        const RECT icon=GiftBounds();
        if(!IsRectEmpty(&icon)) giftItem.ShowItemStatus(icon.left+GiftHintX,icon.top+GiftHintY);
    } else if(view==View::Read) {
        const auto* message=CurrentMessage();
        if(!message) return;
        if(message->item[0]) {
            CItem item;item.SetMir3Item(message->item.data(),0);
            item.ShowItemStatus(bounds.left+AttachmentHintOffset,bounds.top+AttachmentHintOffset);
        } else if(message->gold) {
            // 458C20: the money attachment uses its own fixed 90x20 tooltip.
            const RECT box={bounds.left+10,bounds.top+15,bounds.left+100,bounds.top+35};
            DrawLegacyPanel(box,RGB(90,90,45),230,GoldHintBorderColor);
            static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
            const auto label=captions.Get(GoldCaption)+" "+std::to_string(message->gold);
            HDC dc;auto* surface=g_xMainWnd.GetBackBuffer();
            if(FAILED(surface->GetDC(&dc))) return;
            const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
            HFONT font=g_xMainWnd.CreateGameFont(face,9);const auto previous=SelectObject(dc,font);
            SetBkMode(dc,TRANSPARENT);SetTextColor(dc,GoldHintTextColor);
            RECT text=box;DrawTextA(dc,label.data(),int(label.size()),&text,DT_LEFT|DT_SINGLELINE|DT_VCENTER);
            SelectObject(dc,previous);DeleteObject(font);surface->ReleaseDC(dc);
        }
    }
}

namespace {
unsigned MailNumber(const BYTE* record,int offset,int bytes)
{
    unsigned value=0;for(int i=0;i<bytes;++i) value|=unsigned(record[offset+i])<<(8*i);return value;
}
std::string MailText(const BYTE* record,int offset,int size)
{
    const char* text=(const char*)record+offset;int length=0;
    while(length<size && text[length]) ++length;return std::string(text,length);
}
}
void CMailWnd::ReceiveList(bool sent,const char* encoded,int count)
{
    using namespace MailWire;
    auto& messages=sent?outbox:inbox;messages.clear();
    const char* start=encoded;
    for(int index=0;index<count;++index) {
        const char* end=strchr(start,'/');if(!end) break;
        std::string part(start,end);BYTE record[HeaderBytes]={};
        if(fnDecode6BitBuf(part.data(),(char*)record,sizeof(record))==HeaderBytes) {
            Message message;message.id=MailNumber(record,Id,4);
            message.attachment=MailNumber(record,Kind,2)==1;message.read=MailNumber(record,ReadState,2)!=0;
            message.person=MailText(record,Person,PersonBytes);message.subject=MailText(record,Subject,SubjectBytes);message.date=MailText(record,Date,DateBytes);
            const auto cached=receivedBodies.find({sent,message.id});
            if(cached!=receivedBodies.end()) {
                message.body=cached->second.body;message.gold=cached->second.gold;
                message.item=cached->second.item;message.loaded=true;
            }
            messages.push_back(std::move(message));
        }
        start=end+1;
    }
    (sent?firstOutbox:firstInbox)=0;(sent?selectedOutbox:selectedInbox)=-1;
    (sent?checkedOutbox:checkedInbox)=-1;
}
bool CMailWnd::ReceiveBody(bool sent,const BYTE* record,int length)
{
    using namespace MailWire;
    if(length!=BodyBytes) return false;
    const unsigned id=MailNumber(record,Id,4);
    Message body;body.id=id;body.gold=MailNumber(record,Gold,4);
    std::copy(record+Item,record+Item+MailLayout::ItemRecordBytes,body.item.begin());
    body.body=MailText(record,Text,TextBytes);body.loaded=true;
    receivedBodies[{sent,id}]=body;
    auto& messages=sent?outbox:inbox;
    for(auto& message:messages) if(message.id==id) {
        message.gold=body.gold;message.item=body.item;message.body=body.body;message.loaded=true;
        const bool reading=view==MailLayout::View::Inbox || view==MailLayout::View::Outbox || view==MailLayout::View::Read;
        if(m_bActive && reading && readSent==sent && CurrentMessage()==&message) OpenSelected();
        break;
    }
    return true;
}

RECT CMailWnd::NotificationBounds() const
{
    using namespace MailWire;
    if(!notified || !m_pxWndImage->NewSetIndex(HudFrame)) return {};
    const int y=HudBottom-m_pxWndImage->m_lpstNewCurrWilImageInfo->shHeight+NotificationOffsetY;
    if(!m_pxWndImage->NewSetIndex(NotificationFrame)) return {};
    const auto image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
    return {NotificationX,y,NotificationX+image.shWidth,y+image.shHeight};
}
void CMailWnd::ShowNotification()
{
    const RECT area=NotificationBounds();if(IsRectEmpty(&area)) return;
    DrawSelectionFrame(*m_pxWndImage,MailWire::NotificationFrame,area.left,area.top,SelectionBlend::Normal,false);
}

bool CMailWnd::BuildRequest(const Request& request,_TDEFAULTMESSAGE& header)
{
    using MailLayout::Action;
    const WORD direction=request.sent?MailWire::Outbox:MailWire::Inbox;
    WORD message=0,parameter=direction;DWORD id=request.messageId;
    switch(request.action) {
    case Action::SendCompose:message=MailWire::Send;id=0;parameter=0;break;
    case Action::Refresh:message=MailWire::Query;id=0;break;
    case Action::Read:message=MailWire::Read;break;
    case Action::Delete:case Action::DeleteRead:message=MailWire::Delete;break;
    case Action::CancelOutbox:message=MailWire::Recall;parameter=0;break;
    default:return false;
    }
    fnMakeDefMessage(&header,message,id,parameter,0,0);return true;
}

bool CMailWnd::BuildCompose(const Request& request,std::array<BYTE,MailWire::SendBytes>& body)
{
    using namespace MailWire;
    body.fill(0);
    const auto recipient=LimitAnsiEdit("",request.recipient,RecipientLimit);
    const auto text=LimitAnsiEdit("",CGuildEditor::Payload(request.body),BodyLimit);
    if(recipient.empty() || text.empty()) return false;
    std::string subject=text.substr(0,text.find('\r'));
    if(subject.size()>SubjectLimit) {
        static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
        subject=LimitAnsiEdit("",subject,SubjectPrefix)+captions.Get(EllipsisCaption);
        subject=LimitAnsiEdit("",subject,SubjectLimit);
    }
    memcpy(body.data()+SendRecipient,recipient.data(),recipient.size());
    memcpy(body.data()+SendSubject,subject.data(),subject.size());
    memcpy(body.data()+SendText,text.data(),text.size());return true;
}
