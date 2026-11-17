#pragma once
struct SelectionRosterPreview {
    enum {First=284,Waiting=285,Second=286,LongName=287,Empty=288,Last=Empty};
    static constexpr const char* Four="First/0/0/10/0/Second/1/0/20/1/Third/2/0/30/0/Fourth/0/0/40/1/";
    static void Reply(int count,const char* body,int bytes=-1,WORD legacyCount=0) {
        _TDEFAULTMESSAGE header={};fnMakeDefMessage(&header,SM_QUERYCHR,count,legacyCount,0,0);
        char encoded[512]={},tail[512]={};fnEncodeMessage(&header,encoded,sizeof(encoded));
        fnEncode6BitBuf((unsigned char*)body,tail,bytes<0?int(strlen(body)):bytes,sizeof(tail));
        std::string wire=std::string(encoded)+tail;g_xChrSelProc.OnSocketMessageRecieve(&wire[0]);
    }
    static void Checks() {
        FILE* report=fopen("selection-roster-check.txt","wb");if(!report) return;
        const auto check=[&](const char* name,bool ok){fprintf(report,"%s=%d\n",name,ok);};
        SelectionRoster roster;
        check("four_text_roles",roster.Parse(4,Four) && roster.count==4 && !strcmp(roster.roles[3].ChrName,"Fourth"));
        check("atomic_bad_job",!roster.Parse(1,"Bad/3/0/1/0/") && roster.count==4);
        check("atomic_incomplete",!roster.Parse(2,"First/0/0/1/0/") && roster.count==4);
        check("name_24",roster.Parse(1,"123456789012345678901234/0/0/1/0/") && strlen(roster.roles[0].ChrName)==24);
        check("name_25_rejected",!roster.Parse(1,"1234567890123456789012345/0/0/1/0/"));
        check("byte_cast",roster.Parse(1,"Byte/256/0/257/256/") && roster.roles[0].Class==0 && roster.roles[0].Level==1);
        check("integer_overflow",!roster.Parse(1,"Bad/99999999999999999/0/1/0/"));
        check("empty",roster.Parse(0,"") && roster.count==0);
        CHRINFO legacy={};legacy.Class=1;legacy.Gender=1;legacy.Level=50;strcpy(legacy.ChrName,"Legacy");
        check("legacy_23",sizeof(legacy)==23 && roster.ParseLegacy(1,&legacy,sizeof(legacy)) && !strcmp(roster.roles[0].ChrName,"Legacy"));
        check("legacy_truncated",!roster.ParseLegacy(1,&legacy,sizeof(legacy)-1));
        auto& p=g_xChrSelProc;p.Load(false);p.BeginRosterWait();
        check("waiting_initial",p.IsWaiting() && p.m_selectionElapsed==0 && p.m_xMsgBox.IsActive());
        Reply(4,Four);check("early_list_pending",p.IsWaiting() && p.m_ChrCount==0 && p.m_pendingRoster.has_value());
        p.OnKeyDown(VK_RETURN,0);p.OnLButtonUp(0,MAKELPARAM(550,405));check("waiting_input_blocked",p.IsWaiting() && p.m_xSelectChr.m_page==0);
        p.AdvanceRoster(499);check("wait_499",p.IsWaiting() && p.m_ChrCount==0);
        p.AdvanceRoster(1);check("apply_500",!p.IsWaiting() && p.m_ChrCount==4 && !p.m_xMsgBox.IsActive());
        p.m_xSelectChr.OnButtonUp(0,MAKELPARAM(555,405));
        check("second_page",p.m_xSelectChr.m_page==1 && p.m_xSelectChr.m_nSelectChr==2);
        p.m_xSelectChr.OnButtonUp(0,MAKELPARAM(505,405));
        check("first_page",p.m_xSelectChr.m_page==0 && p.m_xSelectChr.m_nSelectChr==0);
        p.BeginRosterWait();Reply(4,Four);p.ShowNotice(SelectionDialog::StartFailed);
        check("error_deferred",p.IsWaiting() && p.m_pendingNotice==SelectionDialog::StartFailed);
        p.AdvanceRoster(500);check("pending_error_applied",p.m_ChrCount==4 && p.m_xMsgBox.Text()==p.m_messages.Get(SelectionDialog::StartFailed));
        Reply(0,(const char*)&legacy,sizeof(legacy),1);
        check("legacy_wire",p.m_ChrCount==1 && !strcmp(p.m_CList.GetCurrentData()->ChrName,"Legacy") && !p.m_xMsgBox.IsActive());
        p.BeginRosterWait();p.AdvanceRoster(INT_MAX);check("no_reply_keeps_waiting",p.IsWaiting() && p.m_selectionElapsed==SelectionRoster::Delay);
        Reply(0,"");check("empty_after_wait",!p.IsWaiting() && p.m_ChrCount==0 && p.m_nMessage==SelectionDialog::Empty);
        p.DeleteProc();fclose(report);
    }
    static const char* Load(int mode) {
        if(mode==First) Checks();
        LoginVisual::ResizeCanvas(640,480);g_xChrSelProc.Load(false);g_xChrSelProc.BeginRosterWait();
        if(mode!=Waiting) {
            if(mode==Empty) Reply(0,"");
            else if(mode==LongName) Reply(1,"123456789012345678901234/1/0/50/1/");
            else Reply(4,Four);
            g_xChrSelProc.AdvanceRoster(500);
            if(mode==Second) g_xChrSelProc.m_xSelectChr.OnButtonUp(0,MAKELPARAM(555,405));
        }
        static const char* files[]={"selection-roster-first.bmp","selection-roster-waiting.bmp","selection-roster-second.bmp","selection-roster-long-name.bmp","selection-roster-empty.bmp"};
        return files[mode-First];
    }
};
