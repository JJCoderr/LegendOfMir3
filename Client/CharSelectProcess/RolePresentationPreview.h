#pragma once
struct RolePresentationPreview {
    enum {First=292,LastCreation=297,Hover=298,Pressed=299,Last=Pressed};
    static void Checks() {
        FILE* report=fopen("role-presentation-check.txt","wb");if(!report) return;
        const auto check=[&](const char* name,bool ok){fprintf(report,"%s=%d\n",name,ok);};
        RoleAnimation a;a.Start(RoleAnimation::Create);
        check("strict_120",!a.Advance(120,0,0) && a.frame==0 && a.elapsed==120);
        check("voice_at_121",a.Advance(1,0,0) && a.frame==1 && a.elapsed==0);
        check("long_tick_one_frame",!a.Advance(1000,0,0) && a.frame==2 && a.elapsed==0);
        a.Start(RoleAnimation::Create);check("word_wrap",!a.Advance(65536,0,0) && a.frame==0);
        a.Start(RoleAnimation::Select);a.frame=19;a.Advance(121,0,0);
        check("selection_enters_loop",a.action==RoleAnimation::Selected && a.frame==0 && !a.Advance(121,0,0));
        a.Start(RoleAnimation::Deselect);a.frame=7;a.Advance(121,0,0);
        check("deselection_enters_idle",a.action==RoleAnimation::Idle && a.frame==0);
        constexpr int ends[]={17,14,16,15,16,14};
        for(int gender=0;gender<2;++gender) for(int job=0;job<3;++job) {
            a.Start(RoleAnimation::Create);bool loop=true;
            for(int i=0;i<=ends[job+3*gender];++i) loop&=a.Advance(121,job,gender)==(i==0);
            char label[40];sprintf(label,"creation_loop_%d_%d",job,gender);
            check(label,loop && a.frame==0 && a.Advance(121,job,gender));
        }
        auto& p=g_xChrSelProc;p.Preview("create");p.m_SelectSex=0;
        p.RenderCharCreate(120);check("creation_both_wait",p.m_creationAnimation[0].frame==0 && p.m_creationAnimation[1].frame==0);
        p.RenderCharCreate(1);check("creation_both_advance",p.m_creationAnimation[0].frame==1 && p.m_creationAnimation[1].frame==1);
        check("creation_bounds_after_tick",p.m_createBounds[0].right>p.m_createBounds[0].left && p.m_createBounds[1].right>p.m_createBounds[1].left);
        const RECT saved=p.m_createBounds[1];p.OnLButtonDown(0,MAKELPARAM(saved.left,saved.top));
        check("gender_resets_only_selected",p.m_SelectSex==1 && p.m_creationAnimation[0].frame==1 && p.m_creationAnimation[1].frame==0);
        p.RenderCharCreate(0);check("render_does_not_replace_bounds",EqualRect(&saved,&p.m_createBounds[1]));
        p.OnLButtonUp(0,MAKELPARAM(309,420));
        check("job_resets_both",p.m_SelectJob==1 && p.m_creationAnimation[0].frame==0 && p.m_creationAnimation[1].frame==0);
        p.DeleteProc();p.Preview("select");auto& s=p.m_xSelectChr;
        s.Render(120);check("selection_initial_wait",s.m_roleAnimation[0].frame==0);
        s.Render(1);check("selection_initial_advance",s.m_roleAnimation[0].frame==1);
        s.OnMouseMove(0,MAKELPARAM(445,98));check("hover_without_press",s.m_pointer.x==445 && !s.m_fIsButtonDown);
        s.OnButtonDown(0,MAKELPARAM(445,98));s.OnMouseMove(0,MAKELPARAM(10,10));
        check("drag_preserves_press_updates_hover",s.m_fIsButtonDown && s.m_pointer.x==10);
        p.DeleteProc();fclose(report);
    }
    static const char* Load(int mode) {
        LoginVisual::ResizeCanvas(640,480);if(mode==First) Checks();
        auto& p=g_xChrSelProc;p.Preview(mode<=LastCreation?"create":"select");
        if(mode<=LastCreation) {p.m_SelectJob=(mode-First)%3;p.m_SelectSex=(mode-First)/3;}
        else if(mode==Hover) p.OnMouseMove(0,MAKELPARAM(445,98));
        else p.OnLButtonDown(0,MAKELPARAM(445,98));
        static const char* files[]={"role-male-warrior.bmp","role-male-mage.bmp","role-male-tao.bmp",
            "role-female-warrior.bmp","role-female-mage.bmp","role-female-tao.bmp","role-selection-hover.bmp","role-selection-pressed.bmp"};
        return files[mode-First];
    }
};
