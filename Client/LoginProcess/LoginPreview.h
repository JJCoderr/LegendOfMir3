#pragma once
#include "Common/InputMovePreview.h"
struct LoginPreview {
    enum {First=249,Last=254};
    static void Reply(WORD opcode,int reason=0,WORD p0=0,WORD p1=0,WORD p2=0,const char* body="") {
        _TDEFAULTMESSAGE header={};fnMakeDefMessage(&header,opcode,reason,p0,p1,p2);
        char encoded[512]={},tail[512]={};fnEncodeMessage(&header,encoded,sizeof(encoded));
        fnEncode6BitBuf((unsigned char*)body,tail,int(strlen(body)),sizeof(tail));
        std::string wire=std::string(encoded)+tail;g_xLoginProc.OnSocketMessageRecieve(&wire[0]);
    }
    static void Checks() {
        auto& process=g_xLoginProc;auto& login=process.m_xLogin;auto& servers=process.m_xSelectSrv;
        FILE* report=fopen("login-gui-check.txt","wb");if(!report) return;
        const auto check=[&](const char* name,bool ok) {fprintf(report,"%s=%d\n",name,ok);};
        HWND edit=g_xChatEditBox.GetSafehWnd();
        check("offline",process.m_offline && g_xClientSocket.m_sockClient==INVALID_SOCKET);
        check("announcement_resource",!process.m_announcement.rows.empty());
        check("initial_account",login.m_field==0 && login.m_editorVisible);
        check("login_editor_window_move",CheckInputWindowMove());
        check("editor_window_resize",CheckInputWindowResize(false));
        check("editor_minimize_restore",CheckInputWindowResize(true));
        SetWindowTextA(edit,"1234567890123456789012345");g_xChatEditBox.Changed();
        check("account_byte_limit",GetWindowTextLengthA(edit)==LoginLayout::InputBytes);
        process.OnKeyDown(VK_TAB,0);
        check("tab_password",login.m_field==1 && login.m_values[0].size()==24 && SendMessage(edit,EM_GETPASSWORDCHAR,0,0)=='*');
        SetWindowTextA(edit,"offline_fixture");
        SendMessage(edit,WM_CHAR,VK_RETURN,0);
        check("native_enter_submit",process.m_previewRequests==1 && login.m_field==2 && !login.m_editorVisible && strlen(g_szUserID)==24);
        Reply(SM_PASSWD_FAIL,-1);process.RenderScene(0);
        check("password_error",process.m_xMsgBox.IsActive() && !IsWindowVisible(edit) && process.m_xMsgBox.Text()==process.m_messages.Get(LoginLayout::InvalidPassword));
        process.OnKeyDown(VK_ESCAPE,0);check("escape_keeps_notice",process.m_xMsgBox.IsActive());
        process.OnKeyDown(VK_RETURN,0);
        check("dismiss_clears_password",!process.m_xMsgBox.IsActive() && login.m_field==1 && login.m_values[1].empty() && login.m_values[0].size()==24 && login.m_editorVisible);
        Reply(SM_PASSWD_FAIL,-1);process.RenderScene(0);
        process.OnLButtonUp(0,MAKELPARAM(277,298));
        check("notice_release_at_origin",!process.m_xMsgBox.IsActive() && login.m_editorVisible && login.m_field==1);
        Reply(SM_PASSWD_FAIL,-1);process.RenderScene(0);
        process.OnLButtonUp(0,MAKELPARAM(356,329));
        check("notice_release_last_pixel",!process.m_xMsgBox.IsActive());
        Reply(SM_PASSWD_FAIL,-1);process.RenderScene(0);
        process.OnLButtonUp(0,MAKELPARAM(276,310));
        check("notice_left_outside",process.m_xMsgBox.IsActive());
        process.OnLButtonUp(0,MAKELPARAM(357,310));
        check("notice_right_exclusive",process.m_xMsgBox.IsActive());
        process.OnLButtonUp(0,MAKELPARAM(300,297));
        check("notice_top_outside",process.m_xMsgBox.IsActive());
        process.OnLButtonUp(0,MAKELPARAM(300,330));
        check("notice_bottom_exclusive",process.m_xMsgBox.IsActive());
        process.OnLButtonDown(0,MAKELPARAM(10,10));process.OnLButtonUp(0,MAKELPARAM(300,310));
        check("notice_outside_press_inside_release",!process.m_xMsgBox.IsActive() && login.m_values[1].empty());
        const unsigned requests=process.m_previewRequests;
        process.OnKeyDown(VK_RETURN,0);check("enter_empty_no_request",requests==process.m_previewRequests);
        process.OnLButtonUp(0,MAKELPARAM(470,440));
        check("mouse_empty_submits",process.m_previewRequests==requests+1 && !login.m_editorVisible);
        login.ResetPassword();
        process.OnLButtonUp(0,MAKELPARAM(150,385));
        char url[2048]={};GetPrivateProfileStringA("Initial","Param2","http://account.wemade.com/main01.html",url,sizeof(url),"./Mir3.ini");
        check("register_configured_url",process.m_lastUrl==url);
        process.OnLButtonUp(0,MAKELPARAM(290,385));
        GetPrivateProfileStringA("Initial","Param3","http://account.wemade.com/main03.html",url,sizeof(url),"./Mir3.ini");
        check("password_configured_url",process.m_lastUrl==url);
        DWORD playing=0;LONG volume=0;
        check("click_audio",SUCCEEDED(g_xSound.InspectInterfaceAudio(false,playing,volume)) && playing);
        g_xSound.UpdateLobbyAudio(CMirSound::LobbyPage::None,0);
        g_xSound.UpdateLobbyAudio(CMirSound::LobbyPage::Credentials,0);
        g_xSound.UpdateLobbyAudio(CMirSound::LobbyPage::Credentials,1200);
        g_xSound.InspectInterfaceAudio(true,playing,volume);check("opening_delay",!playing);
        g_xSound.UpdateLobbyAudio(CMirSound::LobbyPage::Credentials,1);
        check("opening_audio",SUCCEEDED(g_xSound.InspectInterfaceAudio(true,playing,volume)) && playing);
        if(servers.m_nSrvCount) {
            const std::string name=servers.m_ppxSrvBtn[0]->szName;
            Reply(SM_PASSOK_SELECTSERVER,0,0,2,0,(name+"(2);").c_str());
            check("server_load_reply",process.m_Progress==PRG_SERVER_SELE && servers.m_loads[0]==2);
            check("billing_notice",process.m_xMsgBox.IsActive() && !login.m_editorVisible);
            check("load_atomic",!servers.ApplyLoads((name+"(3);broken;").c_str()) && servers.m_loads[0]==2);
            process.OnKeyDown(VK_SPACE,0);
            check("server_notice_stays_hidden",!process.m_xMsgBox.IsActive() && !login.m_editorVisible);
            const RECT box=servers.m_ppxSrvBtn[0]->rcBtn;const LPARAM hit=MAKELPARAM(box.left+5,box.top+5);
            servers.OnButtonDown(0,hit);check("server_drag_cancel",servers.OnButtonUp(0,MAKELPARAM(630,470))==0);
            servers.OnButtonDown(0,hit);check("server_same_row",servers.OnButtonUp(0,hit)==1);
        }
        LoginAnnouncement announcement;
        announcement.Parse(";ignored\r\n!12@3#4$*Styled\r\n\r\n");
        check("notice_styles",announcement.rows.size()==2 && announcement.rows[0].points==12 && announcement.rows[0].bold && announcement.rows[0].background==3 && announcement.rows[0].foreground==4 && announcement.rows[1].text.empty());
        std::string longNotice;for(int i=0;i<20;++i) longNotice+="row\r\n";
        announcement.Parse(longNotice);announcement.Advance(100);check("notice_clock_boundary",announcement.offset==0);
        announcement.Advance(1);check("notice_scroll",announcement.offset==-1 && announcement.elapsed==0);
        announcement.Advance(10000);check("notice_long_tick_ignored",announcement.offset==-1);
        announcement.offset=-265;announcement.Advance(101);check("notice_wrap",announcement.offset==224);
        fclose(report);
        process.m_xMsgBox.HideMessageBox();process.m_Progress=PRG_LOGIN;login.Create(&process.m_Image);login.OpenWnd();
    }
    static const char* Load(int mode) {
        g_xMainWnd.ResetDXG(LoginLayout::Width,LoginLayout::Height,_SCREEN_BPP,_DXG_SCREENMODE_WINDOW,_DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D);
        HWND window=g_xMainWnd.GetSafehWnd();RECT bounds={0,0,LoginLayout::Width,LoginLayout::Height};
        AdjustWindowRectEx(&bounds,GetWindowLong(window,GWL_STYLE),FALSE,GetWindowLong(window,GWL_EXSTYLE));
        SetWindowPos(window,nullptr,0,0,bounds.right-bounds.left,bounds.bottom-bounds.top,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
        GetClientRect(window,&g_xMainWnd.m_rcWindow);ClientToScreen(window,(POINT*)&g_xMainWnd.m_rcWindow.left);ClientToScreen(window,(POINT*)&g_xMainWnd.m_rcWindow.right);
        auto& process=g_xLoginProc;process.Load(false);
        if(mode==First) Checks();
        SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"MirPreview");process.OnKeyDown(VK_TAB,0);
        if(mode==First+1) process.OnMouseMove(0,MAKELPARAM(470,440));
        if(mode==First+2) process.OnLButtonDown(0,MAKELPARAM(150,385));
        if(mode==First+3) Reply(SM_PASSWD_FAIL,-1);
        if(mode>=First+4) {
            if(mode==First+5) Reply(SM_PASSOK_SELECTSERVER,0,0,2);
            else {
                std::string loads;
                for(int i=0;i<process.m_xSelectSrv.m_nSrvCount;++i) loads+=std::string(process.m_xSelectSrv.m_ppxSrvBtn[i]->szName)+"(2);";
                Reply(SM_PASSOK_SELECTSERVER,0,0,0,0,loads.c_str());
            }
            process.OnMouseMove(0,MAKELPARAM(45,80));
        }
        static const char* files[]={"login.bmp","login-hover.bmp","login-pressed.bmp","login-error.bmp","login-servers.bmp","login-billing.bmp"};
        return files[mode-First];
    }
};
