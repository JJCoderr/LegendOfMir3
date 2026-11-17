#pragma once
#include "ServerGroupLayout.h"
struct ServerGroupsPreview {
    enum {First=255,Last=261};
    static POINT GroupPoint() {
        const auto& group=g_xServerGroups.m_groups.groups.front();return {group.x+5,group.y+5};
    }
    static void Click(POINT point,DWORD tick=GetTickCount()) {
        g_xServerGroups.Down(point,tick);g_xServerGroups.Up(point);
    }
    static void LongList() {
        auto& model=g_xServerGroups.m_groups;model.Choose(0);
        const auto original=model.groups[0].servers.front();model.groups[0].servers.clear();
        for(int i=0;i<ServerGroups::Limit;++i) {auto row=original;char label[32];sprintf(label,"Preview server %02d",i+1);row.caption=label;model.groups[0].servers.push_back(std::move(row));}
    }
    static void Checks() {
        auto& page=g_xServerGroups;auto& model=page.m_groups;
        FILE* report=fopen("server-groups-check.txt","wb");if(!report) return;
        const auto check=[&](const char* name,bool ok) {fprintf(report,"%s=%d\n",name,ok);};
        check("native_canvas",g_xMainWnd.m_stDisplayInfo.wWidth==800 && g_xMainWnd.m_stDisplayInfo.wHeight==600);
        check("initial_no_selection",model.selected==-1 && model.first==0 && !page.m_inLogin);
        check("external_groups",!model.groups.empty() && !model.groups[0].servers.empty());
        if(model.groups.empty() || model.groups[0].servers.empty()) {fclose(report);return;}
        using namespace ServerGroupLayout;
        bool frames=true;for(int frame:{Background,Exit,ExitDown,Row,RowHover,Marker,MarkerHover,MarkerSelected,Thumb}) frames=frames && page.m_image.NewSetIndex(frame);
        check("required_frames",frames);
        check("optional_glow_missing",!page.m_image.NewSetIndex(ServerGroupLayout::Glow));
        check("number_spaces",ServerGroups::Integer(" 695 ")==695 && !ServerGroups::Integer("695x"));
        auto saved=model;
        const POINT marker=GroupPoint();page.Move(marker);page.Down(marker,100);
        check("marker_selects_on_down",model.selected==0 && model.first==0);
        page.Up(marker);
        const auto config=model.Login(0);
        check("selected_configuration",config && !config->host.empty() && config->port && !config->servers.empty());
        check("row_range",!model.Login(-1) && !model.Login(ServerGroups::VisibleRows));
        model.groups[0].servers[0].config.reset();page.Down({50,75},200);
        check("empty_section_no_entry",!page.m_inLogin);model=saved;
        LongList();model.Scroll(999);check("wheel_clamps_last",model.first==39);
        model.Scroll(-999);check("wheel_clamps_first",model.first==0);
        page.DefMainWndProc(g_xMainWnd.GetSafehWnd(),WM_MOUSEWHEEL,MAKEWPARAM(0,WORD(-WHEEL_DELTA)),0);
        check("wheel_one_row",model.first==1);
        model.first=0;page.m_scrollTick=100;
        page.Down({210,400},110);check("scroll_strict_delay",model.first==0);
        page.Down({210,400},111);check("scroll_page",model.first==10);
        page.Up({210,400});model.first=0;page.m_scrollTick=0;
        page.Down({210,90},100);check("thumb_capture",page.m_dragging && GetCapture()==g_xMainWnd.GetSafehWnd());
        page.Move({210,599});check("drag_to_last",model.first==39);
        page.Up({600,580});check("drag_release",!page.m_dragging && GetCapture()!=g_xMainWnd.GetSafehWnd());
        page.m_scrollTick=0;page.Down({210,535},100);page.Move({210,-20});
        check("drag_to_first",model.first==0);page.Up({210,-20});
        model.first=17;page.Down(marker,300);check("group_resets_scroll",model.first==0);
        model.first=39;const RECT last=page.ScrollThumb();check("thumb_last_position",last.top==531);
        page.Down({770,20},400);check("exit_pressed",page.m_exitPressed);
        page.Up({600,580});check("exit_cancel",!page.m_exitRequested && !page.m_exitPressed);
        page.Up({770,20});check("exit_release_requests",page.m_exitRequested);page.m_exitRequested=false;
        model=saved;model.Choose(0);
        if(config) {
            page.Down({50,75},500);
            check("row_enters_on_down",page.m_inLogin && g_xLoginProc.m_offline);
            check("account_canvas",g_xMainWnd.m_stDisplayInfo.wWidth==640 && g_xMainWnd.m_stDisplayInfo.wHeight==480);
            check("gateway_handoff",g_xLoginProc.m_configuration.host==config->host && g_xLoginProc.m_nServerPort==config->port);
            check("server_list_handoff",g_xLoginProc.m_SList.GetCounter()==int(config->servers.size()));
            g_xLoginProc.LoginAction(CLogin::Register);check("register_url_handoff",g_xLoginProc.m_lastUrl==config->accountUrl);
            g_xLoginProc.LoginAction(CLogin::Password);check("password_url_handoff",g_xLoginProc.m_lastUrl==config->passwordUrl);
            check("offline_no_socket",g_xClientSocket.m_sockClient==INVALID_SOCKET);
            g_xLoginProc.DeleteProc();
        }
        check("reload_native_page",page.Load(true));
        check("reload_clears_selection",page.m_groups.selected==-1 && !page.m_inLogin && !page.m_exitRequested);
        fclose(report);
    }
    static const char* Load(int mode) {
        auto& page=g_xServerGroups;
        if(!page.Load(true)) {if(FILE* report=fopen("server-groups-check.txt","wb")) {fprintf(report,"resources=0\n");fclose(report);}}
        else if(mode==First) Checks();
        if(!page.m_groups.groups.empty()) {
            if(mode==First+1) page.Move(GroupPoint());
            if(mode>=First+2 && mode!=First+5) Click(GroupPoint());
            if(mode==First+3) page.Move({50,75});
            if(mode==First+4) {LongList();page.m_groups.first=20;page.Move({50,75});}
            if(mode==First+5) page.Down({770,20},GetTickCount());
            if(mode==First+6) {
                page.Down({50,75},GetTickCount());
                if(page.m_inLogin) LoginPreview::Reply(SM_PASSOK_SELECTSERVER);
            }
        }
        static const char* files[]={"server-groups.bmp","server-group-hover.bmp","server-group-selected.bmp","server-group-row.bmp","server-group-scroll.bmp","server-group-exit.bmp","server-group-login.bmp"};
        return files[mode-First];
    }
};
