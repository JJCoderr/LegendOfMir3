#pragma once
#include "MailLayout.h"
#include <string>
#include <array>
#include <vector>

class CMailWnd : public CGameWnd {
    HFONT editFont=nullptr;
    int editFontHeight=0;
    bool editorsAvailable=true;
    WNDPROC editProcedure=nullptr;
    static LRESULT CALLBACK EditorProc(HWND,UINT,WPARAM,LPARAM);
    std::vector<std::string> bodyLines;
    std::string wrappedBody;
    void LayoutBody();
    HBRUSH editBackground[2]={};
    void LayoutEditors();
    void ReadEditors();
public:
    HWND edits[2]={};
    bool dragging=false,captured=false,readSent=false,scrolling=false;
    int pressedRow=-1,hoverRow=-1,checkedInbox=-1,checkedOutbox=-1;
    bool pressedCheck=false;
    struct Request {
        MailLayout::Action action;
        unsigned messageId=0;
        bool sent=false;
        std::string recipient,body;
        unsigned gold=0;
        std::array<BYTE,MailLayout::ItemRecordBytes> item={};
    };
    std::vector<Request> requests;
    void RequestAction(MailLayout::Action action);
    struct Message {
        unsigned id=0;
        std::string person,subject,body,date;
        bool read=false,attachment=false,loaded=false;
        unsigned gold=0;
        std::array<BYTE,MailLayout::ItemRecordBytes> item={};
    };
    MailLayout::View view=MailLayout::View::Inbox;
    std::vector<Message> inbox,outbox;
    int firstInbox=0,firstOutbox=0,selectedInbox=-1,selectedOutbox=-1,pressed=-1;
    std::string recipient,draft;
    std::string composeRecipient,composeDraft,giftRecipient,giftDraft;
    unsigned giftGold=0;
    CItem giftItem;
    bool hasGiftItem=false,giftIsGold=false;
    bool SelectGift(POINT point);
    bool SubmitGold(const char* text,unsigned balance);
    RECT GiftBounds();
    void DrawGift();
    void ShowAttachmentHint(POINT point);
    int firstBody=0;
    void SyncEditors(bool available);
    void Destroy() override;
    void Close();
    void Pointer(int kind,POINT point);
    void OpenSelected();
    void Wheel(int delta);
    int RowAt(POINT point) const;
    int ScrollTop() const;
    bool Scroll(int kind,POINT point);
    Message* CurrentMessage();
    HBRUSH EditColor(HWND edit,HDC dc);
    void Create(CWHWilImageData* image,int id);
    void Show();
    std::vector<MailLayout::Button> VisibleButtons() const;
    int ButtonAt(POINT point) const;
    void SelectView(MailLayout::View next);
};
