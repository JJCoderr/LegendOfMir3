#pragma once
#include "ConsignmentLayout.h"
#include "ConsignmentWire.h"
#include <string>
#include <vector>

class CConsignmentWnd : public CGameWnd {
public:
    struct Listing {
        unsigned id=0,price=0;
        std::string server,seller,date,description;
        CItem item;
    };
    struct Request {
        ConsignmentLayout::Action action;
        unsigned id=0,price=0,sessionId=0;
        int page=0;
        std::string query,description;
        CItem item;
    };
    bool registration=false,captured=false,dragging=false,waiting=false,hasItem=false;
    int selected=-1,hovered=-1,pressed=-1,pressedRow=-1,page=0,pages=0;
    unsigned price=0,sessionId=0;
    CItem item;
    std::string nameQuery,sellerQuery,description;
    std::vector<Listing> listings;
    std::vector<Request> requests;
    HWND edits[2]={};
    static bool BuildRequest(const Request& request,_TDEFAULTMESSAGE& header,std::string& body);
    void ReceiveList(const char* encoded,int count,int currentPage,int totalPages);
    void Create(CWHWilImageData* image,bool registerPage);
    void Destroy() override;
    void Close();
    void Show();
    void Pointer(int kind,POINT point);
    void SyncEditors(bool available);
    HBRUSH EditColor(HWND edit,HDC dc);
    int ButtonAt(POINT point) const;
    int RowAt(POINT point) const;
    std::vector<ConsignmentLayout::Button> Buttons() const;
    RECT ItemBounds();
    void ShowItemHint(POINT point);
    bool SelectItem(POINT point);
    bool SubmitPrice(const char* value);
    void Activate(ConsignmentLayout::Action action,DWORD now=GetTickCount());
    void Confirm(bool accepted,DWORD now=GetTickCount());
    bool KeyDown(WPARAM key,DWORD now=GetTickCount());
private:
    HFONT editFont=nullptr;
    int fontHeight=0,animationFrame=0;
    DWORD animationTick=0,actionTick=0,searchTick=0;
    bool editorsAvailable=true,confirmationPending=false;
    Request confirmation;
    HBRUSH editBackground[2]={};
    WNDPROC editProcedure=nullptr;
    static LRESULT CALLBACK EditorProc(HWND,UINT,WPARAM,LPARAM);
    void SendRequest(Request request);
    void ReadEditors();
    void LayoutEditors();
    void DrawItemPreview();
};
