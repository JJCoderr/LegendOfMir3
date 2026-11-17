#pragma once
struct LoginTransitionPreview {
    enum {First=265,Half=266,Last=267};
    static void Begin() {
        g_xLoginProc.Load(false);
        LoginPreview::Reply(SM_PASSOK_SELECTSERVER);
        LoginPreview::Reply(SM_SELECTSERVER_OK,1234,0,0,0,"127.0.0.1/7100/");
    }
    static void Checks() {
        auto& process=g_xLoginProc;auto* owner=g_xMainWnd.m_pxDefProcess;auto* socketOwner=g_xClientSocket.m_pxDefProc;const BYTE state=g_bProcState;
        FILE* report=fopen("login-transition-check.txt","wb");if(!report) return;
        const auto check=[&](const char* name,bool value){fprintf(report,"%s=%d\n",name,value);};
        Begin();check("success_starts_fade",process.m_Progress==PRG_TO_SELECT_CHR && process.m_nAnimationCounter==0);
        check("endpoint_and_session",strcmp(g_szServerIP,"127.0.0.1")==0 && g_nServerPort==7100 && g_nCertifyCode==1234);
        check("editor_hidden",!IsWindowVisible(g_xChatEditBox.GetSafehWnd()));
        check("old_connection_closed",g_xClientSocket.m_sockClient==INVALID_SOCKET);
        const auto requests=process.m_previewRequests;
        process.OnKeyDown(VK_RETURN,0);process.OnLButtonDown(0,MAKELPARAM(45,80));process.OnLButtonUp(0,MAKELPARAM(45,80));
        check("input_frozen",process.m_Progress==PRG_TO_SELECT_CHR && process.m_previewRequests==requests);
        process.RenderScroll(1999);check("before_boundary",process.m_nAnimationCounter==1999 && g_xMainWnd.m_pxDefProcess==owner);
        process.RenderScroll(1);check("at_boundary",g_xMainWnd.m_pxDefProcess==&g_xChrSelProc && g_xClientSocket.m_pxDefProc==&g_xChrSelProc && g_bProcState==_CHAR_SEL_PROC);
        check("offline_handoff",g_xClientSocket.m_sockClient==INVALID_SOCKET);
        g_xChrSelProc.DeleteProc();g_xMainWnd.m_pxDefProcess=owner;g_xClientSocket.m_pxDefProc=socketOwner;g_bProcState=state;
        process.RenderScroll(1);check("handoff_once",g_xMainWnd.m_pxDefProcess==owner);
        Begin();check("reentry_resets",process.m_nAnimationCounter==0);
        process.RenderScroll(-1);check("negative_time_ignored",process.m_nAnimationCounter==0);
        process.RenderScroll(INT_MAX);check("large_tick_finishes",process.m_nAnimationCounter==LoginLayout::LeaveDuration && g_xMainWnd.m_pxDefProcess==&g_xChrSelProc);
        g_xChrSelProc.DeleteProc();g_xMainWnd.m_pxDefProcess=owner;g_xClientSocket.m_pxDefProc=socketOwner;g_bProcState=state;
        fclose(report);
    }
    static const char* Load(int mode) {
        if(mode==First) Checks();
        Begin();g_xLoginProc.m_nAnimationCounter=mode==First?0:mode==Half?1000:1999;
        static const char* files[]={"login-transition-start.bmp","login-transition-half.bmp","login-transition-end.bmp"};
        return files[mode-First];
    }
};
