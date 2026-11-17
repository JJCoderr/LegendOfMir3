#pragma once
#include "SelectionDialog.h"
struct SelectionDialogPreview {
    enum {First=272,Delete=279,Last=283};
    static void Reply(WORD opcode,int reason=0) {
        _TDEFAULTMESSAGE header={};fnMakeDefMessage(&header,opcode,reason,0,0,0);
        char encoded[512]={};fnEncodeMessage(&header,encoded,sizeof(encoded));g_xChrSelProc.OnSocketMessageRecieve(encoded);
    }
    static void Checks() {
        auto& p=g_xChrSelProc;p.Preview("create");
        FILE* report=fopen("selection-dialog-check.txt","wb");if(!report) return;
        const auto check=[&](const char* name,bool ok){fprintf(report,"%s=%d\n",name,ok);};
        check("name_ascii",!SelectionDialog::NameError("Mir123"));
        check("name_long",SelectionDialog::NameError("12345678901234")==SelectionDialog::NameLong);
        check("name_forbidden",SelectionDialog::NameError("Mir/3")==SelectionDialog::NameInvalid);
        const char pair[]={char(0x81),char(0x5c),0};
        check("name_dbcs_trail",!SelectionDialog::NameError(pair));
        check("name_dbcs_truncated",SelectionDialog::NameError(std::string_view(pair,1))==SelectionDialog::NameInvalid);
        Reply(SM_NEWCHR_FAIL,2);check("exists_text",p.m_xMsgBox.Text()==p.m_messages.Get(SelectionDialog::NameExists));
        check("error_hides_editor",!IsWindowVisible(g_xChatEditBox.GetSafehWnd()));
        p.OnKeyDown(VK_ESCAPE,0);check("escape_keeps_notice",p.m_xMsgBox.IsActive());
        SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"old_name");p.OnKeyDown(VK_RETURN,0);
        check("enter_clears_restores",!p.m_xMsgBox.IsActive() && IsWindowVisible(g_xChatEditBox.GetSafehWnd()) && GetWindowTextLengthA(g_xChatEditBox.GetSafehWnd())==0);
        Reply(SM_NEWCHR_FAIL,3);check("full_text",p.m_xMsgBox.Text()==p.m_messages.Get(SelectionDialog::Full));
        p.OnKeyDown(VK_SPACE,0);check("space_closes",!p.m_xMsgBox.IsActive());
        Reply(SM_NEWCHR_FAIL,99);check("fallback_text",p.m_xMsgBox.Text()==p.m_messages.Get(SelectionDialog::CreateFailed));
        p.OnLButtonUp(0,MAKELPARAM(280,305));check("release_closes",!p.m_xMsgBox.IsActive());
        Reply(SM_STARTFAIL);check("start_text",p.m_xMsgBox.Text()==p.m_messages.Get(SelectionDialog::StartFailed));
        Reply(SM_QUERYCHR_FAIL);check("query_text",p.m_xMsgBox.Text()==p.m_messages.Get(SelectionDialog::QueryFailed));
        Reply(SM_OUTOFCONNECTION);check("disconnect_text",p.m_xMsgBox.Text()==p.m_messages.Get(SelectionDialog::Disconnected));
        p.DeleteProc();p.Preview("select");p.OpenDeleteNotice();
        check("delete_editor",IsWindowVisible(g_xChatEditBox.GetSafehWnd()) && SendMessage(g_xChatEditBox.GetSafehWnd(),EM_GETLIMITTEXT,0,0)==SelectionDialog::ConfirmationBytes);
        p.OnKeyDown(VK_TAB,0);p.OnKeyDown(VK_RETURN,0);
        check("delete_keyboard_no",!p.m_xMsgBox.IsActive() && !IsWindowVisible(g_xChatEditBox.GetSafehWnd()));
        WORD port=0;SOCKET listener=ConnectionPreview::Listen(port);
        SOCKET client=socket(AF_INET,SOCK_STREAM,0);sockaddr_in address={};address.sin_family=AF_INET;address.sin_port=htons(port);address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
        check("loopback_connect",connect(client,(sockaddr*)&address,sizeof(address))==0);
        SOCKET peer=accept(listener,nullptr,nullptr);g_xClientSocket.m_sockClient=client;
        char account[sizeof(g_szUserID)]={};memcpy(account,g_szUserID,sizeof(account));strcpy(g_szUserID,"Preview");
        p.OpenDeleteNotice();SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"confirmation_text");
        p.m_CList.MoveCurrentToTop();p.m_CList.MoveNode(p.m_xSelectChr.m_nSelectChr);
        const std::string expected=std::string(p.m_CList.GetCurrentData()->ChrName)+"/confirmation_text";
        p.OnKeyDown(VK_RETURN,0);
        char wire[2048]={};int size=SOCKET_ERROR;
        for(int i=0;i<100 && size==SOCKET_ERROR;++i) {size=recv(peer,wire,sizeof(wire)-1,0);if(size==SOCKET_ERROR) Sleep(1);}
        char* end=strchr(wire,'!');if(end) *end=0;
        _TDEFAULTMESSAGE decoded={};char body[256]={};
        if(size>DEFBLOCKSIZE+2) {fnDecodeMessage(&decoded,wire+2);fnDecode6BitBuf(wire+2+DEFBLOCKSIZE,body,sizeof(body)-1);}
        check("delete_preserves_input",decoded.wIdent==CM_DELCHR && body==expected);
        check("delete_closes_editor",!p.m_xMsgBox.IsActive() && GetWindowTextLengthA(g_xChatEditBox.GetSafehWnd())==0);
        g_xClientSocket.DisconnectToServer();closesocket(peer);closesocket(listener);memcpy(g_szUserID,account,sizeof(account));
        p.m_xSelectChr.m_nSelectChr=SELECT_NONE;p.OpenDeleteNotice();
        check("delete_missing_text",p.m_xMsgBox.Text()==p.m_messages.Get(SelectionDialog::DeleteMissing));
        Reply(SM_QUERYCHR);check("empty_list_notice",p.m_ChrCount==0 && p.m_xMsgBox.Text()==p.m_messages.Get(SelectionDialog::Empty));
        p.DeleteProc();fclose(report);
    }
    static const char* Load(int mode) {
        if(mode==First) Checks();
        LoginVisual::ResizeCanvas(640,480);g_xChrSelProc.Preview(mode<=First+3 || mode==First+10?"create":"select");
        if(mode==First+8 || mode==First+9 || mode==Last) Reply(SM_QUERYCHR);
        static const int messages[]={SelectionDialog::NameExists,SelectionDialog::Full,SelectionDialog::CreateFailed,SelectionDialog::NameInvalid,
            SelectionDialog::QueryFailed,SelectionDialog::StartFailed,SelectionDialog::Disconnected,SelectionDialog::DeleteConfirmation,
            SelectionDialog::DeleteMissing,SelectionDialog::StartMissing,SelectionDialog::NameLong,SelectionDialog::Empty};
        if(mode==Delete) g_xChrSelProc.OpenDeleteNotice();else g_xChrSelProc.ShowNotice(messages[mode-First]);
        static const char* files[]={"character-name-exists.bmp","character-full.bmp","character-create-failed.bmp","character-name-invalid.bmp",
            "character-query-failed.bmp","character-start-failed.bmp","character-disconnected.bmp","character-delete-confirm.bmp",
            "character-delete-missing.bmp","character-start-missing.bmp","character-name-long.bmp","character-empty.bmp"};
        return files[mode-First];
    }
};
