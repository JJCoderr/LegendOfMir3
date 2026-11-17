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
        p.m_SelectSex=1;p.m_SelectJob=2;p.m_creationAnimation[0].frame=5;p.m_createPressed=2;p.m_bCreatePending=TRUE;
        g_xChatEditBox.SetLimitText(90);SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"StaleName");
        p.OpenCreation();char name[100]={};GetWindowTextA(g_xChatEditBox.GetSafehWnd(),name,sizeof(name));
        check("entry_resets_job_keeps_gender",p.m_SelectJob==0 && p.m_SelectSex==1);
        check("entry_clears_editor",!name[0] && SendMessage(g_xChatEditBox.GetSafehWnd(),EM_GETLIMITTEXT,0,0)==SelectionDialog::NameBytes);
        check("entry_resets_pending_animation",!p.m_bCreatePending && p.m_creationAnimation[0].frame==0 && p.m_createPressed==-1);
        check("entry_focus",GetFocus()==g_xChatEditBox.GetSafehWnd());
        p.m_createBounds[0]={100,100,150,200};p.OnLButtonUp(0,MAKELPARAM(110,110));check("gender_release_ignored",p.m_SelectSex==1);
        p.OnLButtonDown(0,MAKELPARAM(110,110));check("gender_press_selects",p.m_SelectSex==0);
        p.m_creationAnimation[0].frame=5;p.OnLButtonUp(0,MAKELPARAM(267,420));check("same_job_restarts_animation",p.m_SelectJob==0 && p.m_creationAnimation[0].frame==0);
        p.OnLButtonDown(0,MAKELPARAM(309,420));check("job_waits_for_release",p.m_SelectJob==0 && p.m_createPressed==3);
        p.OnLButtonUp(0,MAKELPARAM(309,420));check("job_release_selects",p.m_SelectJob==1 && p.m_createPressed==-1);
        SetFocus(g_xMainWnd.GetSafehWnd());p.OnLButtonUp(0,MAKELPARAM(369,410));check("editor_margin_focus",GetFocus()==g_xChatEditBox.GetSafehWnd());
        p.DeleteProc();fclose(report);
    }
    enum {NameAscii=307,NameChinese=308,NameRestored=309};
    static const char* LoadName(int mode) {
        LoginVisual::ResizeCanvas(640,480);
        auto& p=g_xChrSelProc;p.Preview("create");
        const HWND edit=g_xChatEditBox.GetSafehWnd();
        FILE* report=fopen("creation-name-check.txt","wb");
        const auto check=[&](const char* name,bool ok){if(report) fprintf(report,"%s=%d\n",name,ok);};
        check("visible_focused",IsWindowVisible(edit) && GetFocus()==edit);
        check("name_limit",SendMessage(edit,EM_GETLIMITTEXT,0,0)==SelectionDialog::NameBytes);
        const auto text=[&](){char value[128]={};GetWindowTextA(edit,value,sizeof(value));return std::string(value);};
        SetWindowTextA(edit,"");SendMessageA(edit,EM_REPLACESEL,TRUE,(LPARAM)"1234567890123456");
        check("ascii_limit",text()=="1234567890123");
        SetWindowTextA(edit,"");SendMessageA(edit,EM_REPLACESEL,TRUE,(LPARAM)"传奇勇士测试中文");
        check("chinese_limit_whole",text()=="传奇勇士测试");
        SetWindowTextA(edit,"传奇勇士");SendMessage(edit,EM_SETSEL,0,1);SendMessageA(edit,EM_REPLACESEL,TRUE,(LPARAM)"神");
        check("chinese_replace",text()=="神奇勇士");
        SetWindowTextA(edit,"ABCDEFGHIJKL");SendMessage(edit,EM_SETSEL,6,6);
        SendMessageA(edit,EM_REPLACESEL,TRUE,(LPARAM)"中");
        check("dbcs_overflow_keeps_suffix",text()=="ABCDEFGHIJKL");
        SetWindowTextA(edit,"ABCDEFGHIJK");SendMessage(edit,EM_SETSEL,5,5);
        SendMessageA(edit,EM_REPLACESEL,TRUE,(LPARAM)"中");
        check("dbcs_exact_fit_keeps_suffix",text()=="ABCDE中FGHIJK");

        p.ShowNotice(SelectionDialog::NameExists);
        check("error_hides_name",!IsWindowVisible(edit));p.OnKeyDown(VK_RETURN,0);
        check("error_restores_empty_focus",IsWindowVisible(edit) && GetFocus()==edit && text().empty());
        SetWindowTextA(edit,"传奇勇士");p.OnLButtonUp(0,MAKELPARAM(309,420));
        check("job_preserves_name",text()=="传奇勇士" && p.m_SelectJob==1);
        p.m_createBounds[1]={400,200,500,390};p.OnLButtonDown(0,MAKELPARAM(450,250));
        check("gender_preserves_name",text()=="传奇勇士" && p.m_SelectSex==1);
        p.OpenCreation();check("reopen_clears_name",text().empty() && GetFocus()==edit);
        const auto previousState=g_bProcState;g_bProcState=_CHAR_SEL_PROC;
        SendMessage(edit,WM_KEYDOWN,VK_ESCAPE,0);
        check("creation_escape_keeps_focus",GetFocus()==edit && !p.m_xMsgBox.IsActive());
        SendMessage(edit,WM_CHAR,VK_TAB,0);
        check("creation_tab_keeps_focus",GetFocus()==edit);
        SendMessage(edit,WM_CHAR,VK_RETURN,0);
        check("empty_enter_keeps_editor",GetFocus()==edit && IsWindowVisible(edit) && !p.m_xMsgBox.IsActive());
        p.DeleteProc();p.Preview("select");p.OpenDeleteNotice();SetWindowTextA(edit,"confirmation");
        SendMessage(edit,WM_KEYDOWN,VK_ESCAPE,0);
        check("delete_escape_keeps_focus",p.m_xMsgBox.IsActive() && GetFocus()==edit);
        SendMessage(edit,WM_CHAR,VK_TAB,0);
        check("delete_tab_keeps_text_focus",GetFocus()==edit && text()=="confirmation");
        SendMessage(edit,WM_CHAR,VK_SPACE,0);
        check("delete_space_cancels_selected",!p.m_xMsgBox.IsActive() && !p.IsWaiting() && !IsWindowVisible(edit));
        p.OpenDeleteNotice();SendMessage(edit,WM_CHAR,VK_TAB,0);SendMessage(edit,WM_CHAR,VK_RETURN,0);
        check("delete_enter_cancels_selected",!p.m_xMsgBox.IsActive() && !p.IsWaiting());
        p.DeleteProc();p.Preview("create");g_bProcState=previousState;
        SetWindowTextA(edit,mode==NameAscii?"MirWarrior":mode==NameChinese?"传奇勇士":"再次创建");
        if(report) fclose(report);
        return mode==NameAscii?"creation-name-ascii.bmp":mode==NameChinese?"creation-name-chinese.bmp":"creation-name-restored.bmp";
    }
    enum {ReturnCancelled=316,ReturnCreated=317};
    static const char* LoadReturn(int mode) {
        LoginVisual::ResizeCanvas(640,480);
        auto& p=g_xChrSelProc;p.Preview("create");const HWND edit=g_xChatEditBox.GetSafehWnd();
        FILE* report=fopen("creation-return-check.txt","wb");
        const auto check=[&](const char* name,bool ok){if(report) fprintf(report,"%s=%d\n",name,ok);};
        const auto leave=[&](bool success) {
            SetWindowTextA(edit,"传奇勇士");strcpy(g_xChatEditBox.m_szInputMsg,"stale_name");p.m_createPressed=1;
            if(success) SelectionDialogPreview::Reply(SM_NEWCHR_SUCCESS);
            else p.OnLButtonUp(0,MAKELPARAM(500,450));
        };
        leave(false);
        check("cancel_clears_text",GetWindowTextLengthA(edit)==0 && !g_xChatEditBox.m_szInputMsg[0]);
        check("cancel_hides_releases_focus",!IsWindowVisible(edit) && GetFocus()!=edit);
        check("cancel_restores_limit",SendMessage(edit,EM_GETLIMITTEXT,0,0)==90);
        check("cancel_clears_press",p.m_createPressed==-1 && p.IsWaiting());
        p.OpenCreation();check("reopen_name_limit",SendMessage(edit,EM_GETLIMITTEXT,0,0)==SelectionDialog::NameBytes && GetWindowTextLengthA(edit)==0 && GetFocus()==edit);
        p.DeleteProc();p.Preview("create");leave(true);
        check("success_clears_text",GetWindowTextLengthA(edit)==0 && !g_xChatEditBox.m_szInputMsg[0]);
        check("success_hides_releases_focus",!IsWindowVisible(edit) && GetFocus()!=edit);
        check("success_restores_limit",SendMessage(edit,EM_GETLIMITTEXT,0,0)==90 && !SendMessage(edit,EM_GETPASSWORDCHAR,0,0));
        check("success_clears_press",p.m_createPressed==-1 && p.IsWaiting() && !p.m_bCreatePending);
        p.RenderScene(0);p.RenderScene(0);
        SelectionDialogPreview::Reply(SM_QUERYCHR);p.AdvanceRoster(SelectionRoster::Delay);
        check("roster_reply_keeps_editor_hidden",!p.IsWaiting() && !IsWindowVisible(edit) && GetWindowTextLengthA(edit)==0);
        p.OnKeyDown(VK_RETURN,0);p.OpenCreation();
        check("create_after_reply_restores_input",GetFocus()==edit && IsWindowVisible(edit) && SendMessage(edit,EM_GETLIMITTEXT,0,0)==SelectionDialog::NameBytes);

        p.DeleteProc();p.Preview("create");leave(mode==ReturnCreated);
        if(report) fclose(report);
        return mode==ReturnCreated?"creation-return-created.bmp":"creation-return-cancelled.bmp";
    }
    static const char* Load(int mode) {
        LoginVisual::ResizeCanvas(640,480);if(mode==First) Checks();
        auto& p=g_xChrSelProc;p.Preview(mode==First?"select-three":"create");
        if(mode==Female) {p.m_SelectSex=1;p.m_SelectJob=2;}
        static const char* files[]={"character-interaction-select.bmp","character-interaction-create.bmp","character-interaction-female.bmp"};
        return files[mode-First];
    }
};
