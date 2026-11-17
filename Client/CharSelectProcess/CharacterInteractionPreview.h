#pragma once
struct CharacterInteractionPreview {
    enum {First=289,Create=290,Female=291,Last=Female};
    static void Checks() {
        FILE* report=fopen("character-interaction-check.txt","wb");if(!report) return;
        const auto check=[&](const char* name,bool ok){fprintf(report,"%s=%d\n",name,ok);};
        auto& p=g_xChrSelProc;p.Preview("select-three");auto& s=p.m_xSelectChr;
        p.m_CRectList.MoveCurrentToTop();p.m_CRectList.MoveNode(1);
        *p.m_CRectList.GetCurrentData()=CHRRECT{};
        auto& bounds=*p.m_CRectList.GetCurrentData();bounds.nLeft=350;bounds.nTop=200;bounds.nWidth=50;bounds.nHeight=100;
        s.OnButtonUp(0,MAKELPARAM(350,200));check("release_does_not_select",s.m_nSelectChr==0);
        s.OnButtonDown(0,MAKELPARAM(350,200));check("press_selects_inclusive_origin",s.m_nSelectChr==1 && s.m_roleAnimation[1].action==1 && s.m_roleAnimation[0].action==3);
        s.m_roleAnimation[1].frame=5;s.OnButtonDown(0,MAKELPARAM(350,200));check("same_role_keeps_animation",s.m_roleAnimation[1].frame==5);
        p.OnLButtonUp(0,MAKELPARAM(30,440));check("exit_records_configured_url",p.m_offline && p.m_lastUrl==p.m_messages.Get(LoginLayout::ExitUrl));
        p.DeleteProc();p.Preview("select");
        p.OnLButtonUp(0,MAKELPARAM(441,94));check("create_without_old_fade",p.m_xSelectChr.m_nRenderState==11);
        p.m_SelectSex=1;p.m_SelectJob=2;p.m_Tick=900;p.m_createPressed=2;p.m_bCreatePending=TRUE;
        g_xChatEditBox.SetLimitText(90);SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"StaleName");
        p.OpenCreation();char name[100]={};GetWindowTextA(g_xChatEditBox.GetSafehWnd(),name,sizeof(name));
        check("entry_resets_job_keeps_gender",p.m_SelectJob==0 && p.m_SelectSex==1);
        check("entry_clears_editor",!name[0] && SendMessage(g_xChatEditBox.GetSafehWnd(),EM_GETLIMITTEXT,0,0)==SelectionDialog::NameBytes);
        check("entry_resets_pending_animation",!p.m_bCreatePending && p.m_Tick==0 && p.m_createPressed==-1);
        check("entry_focus",GetFocus()==g_xChatEditBox.GetSafehWnd());
        p.m_createBounds[0]={100,100,150,200};p.OnLButtonUp(0,MAKELPARAM(110,110));check("gender_release_ignored",p.m_SelectSex==1);
        p.OnLButtonDown(0,MAKELPARAM(110,110));check("gender_press_selects",p.m_SelectSex==0);
        p.m_Tick=900;p.OnLButtonUp(0,MAKELPARAM(267,420));check("same_job_restarts_animation",p.m_SelectJob==0 && p.m_Tick==0);
        p.OnLButtonDown(0,MAKELPARAM(309,420));check("job_waits_for_release",p.m_SelectJob==0 && p.m_createPressed==3);
        p.OnLButtonUp(0,MAKELPARAM(309,420));check("job_release_selects",p.m_SelectJob==1 && p.m_createPressed==-1);
        SetFocus(g_xMainWnd.GetSafehWnd());p.OnLButtonUp(0,MAKELPARAM(369,410));check("editor_margin_focus",GetFocus()==g_xChatEditBox.GetSafehWnd());
        p.DeleteProc();fclose(report);
    }
    static const char* Load(int mode) {
        LoginVisual::ResizeCanvas(640,480);if(mode==First) Checks();
        auto& p=g_xChrSelProc;p.Preview(mode==First?"select-three":"create");
        if(mode==Female) {p.m_SelectSex=1;p.m_SelectJob=2;}
        static const char* files[]={"character-interaction-select.bmp","character-interaction-create.bmp","character-interaction-female.bmp"};
        return files[mode-First];
    }
};
