#include "StdAfx.h"
#include "Common/LegacyPanel.h"

CMirSound			g_xSound;
CWHDXGraphicWindow	g_xMainWnd(640, 480, _SCREEN_BPP);
CLoginProcess		g_xLoginProc;
CCharacterProcess	g_xChrSelProc;
CGameProcess		g_xGameProc;
CSprite				g_xSpriteInfo;
CChatEdit			g_xChatEditBox;
CClientSocket		g_xClientSocket;

INT					g_nCertifyCode;
char				g_szUserID[20];
char				g_szCharName[20];
char				g_szServerIP[16];
INT					g_nServerPort;
INT					g_nVersion;


BYTE				g_bProcState = _LOGIN_PROC;



// Offline visual review uses the real interface without map or network updates.
class CHudPreviewProcess : public CWHDefProcess {
    POINT mouse = {0,0};
    const char* capturePath = NULL;
public:
    void OnConnectToServer() override {}
    char* OnMessageReceive(CHAR*) override { return NULL; }
    void Load(bool expanded, int chatMode, bool exitDialog, int groupPanel) {
        capturePath = chatMode == 1 ? "hud-group.bmp" : chatMode == 2 ? "hud-guild.bmp" : expanded ? "hud-expanded.bmp" : "hud-collapsed.bmp";
        if (exitDialog) capturePath = "hud-exit.bmp";
        if (groupPanel>0 && groupPanel<5) capturePath = groupPanel==2 ? "group-create.bmp" : groupPanel==3 ? "group-invite.bmp" : groupPanel==4 ? "group-remove.bmp" : "group-panel.bmp";
        if (groupPanel>=5 && groupPanel<=10) capturePath = groupPanel==5 ? "guild-overview.bmp" : groupPanel==6 ? "guild-members.bmp" : groupPanel==8 ? "guild-denied.bmp" : groupPanel==9 ? "guild-remove.bmp" : groupPanel==10 ? "guild-break-ally.bmp" : "guild-notice.bmp";
        if (groupPanel>=20 && groupPanel<=22) capturePath=groupPanel==22?"status-and-skills.bmp":groupPanel==20?"skills-panel.bmp":"skills-scrolled.bmp";
        if (groupPanel==11) capturePath="options-panel.bmp";
        if (groupPanel>=15 && groupPanel<=19) capturePath=groupPanel==19?"status-equipment.bmp":groupPanel==18?"status-identity.bmp":groupPanel==16?"status-elements.bmp":groupPanel==17?"status-tooltip.bmp":"status-panel.bmp";
        if (groupPanel>=12 && groupPanel<=14) capturePath=groupPanel==13?"inventory-place.bmp":groupPanel==14?"inventory-conflict.bmp":"inventory-panel.bmp";
        g_xMainWnd.ResetDXG(800,600,_SCREEN_BPP,_DXG_SCREENMODE_WINDOW,
            _DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D);
        g_xGameProc.m_xInterface.CreateInterface(&g_xGameProc.m_xImage);
        auto& hero = g_xGameProc.m_xMyHero;
        hero.m_bJob = 1;
        hero.m_stAbility.bLevel = 35;
        hero.m_stAbility.wHP = 320; hero.m_stAbility.wMaxHP = 400;
        hero.m_stAbility.wMP = 180; hero.m_stAbility.wMaxMP = 300;
        hero.m_stAbility.wWeight = 40; hero.m_stAbility.wMaxWeight = 100;
        hero.m_stAbility.dwExp = 650; hero.m_stAbility.dwMaxExp = 1000;
        hero.m_wPosX = 330; hero.m_wPosY = 270;
        lstrcpyA(g_xGameProc.m_szMapName,"HUD Preview");
        for (int i=1;i<=20;++i) {
            char line[128];
            sprintf(line,"[%02d] Native CLIENT chat preview - scroll to review history.",i);
            g_xGameProc.m_xInterface.MsgAdd(RGB(200,255,255),RGB(0,0,0),line);
        }
        if (expanded) {
            POINT toggle = {620,473};
            g_xGameProc.m_xInterface.OnLButtonDown(toggle);
            g_xGameProc.m_xInterface.OnLButtonUp(toggle);
        }
        if (groupPanel>0 && groupPanel<5) {
            lstrcpyA(g_szCharName,"MirPreview");
            for (int i=0;i<6;++i) {
                CGroupMember* member = new CGroupMember;
                sprintf(member->szMemberName,"Member%d",i+1);
                g_xGameProc.m_xInterface.m_xGroupWnd.m_xGroupList.AddNode(member);
            }
            g_xGameProc.m_xInterface.WindowActivate(_WND_ID_GROUP);
            if (groupPanel>1) {
                POINT button = {315+(groupPanel-2)*60,320};
                g_xGameProc.m_xInterface.OnLButtonDown(button);
                g_xGameProc.m_xInterface.OnLButtonUp(button);
                auto& ui = g_xGameProc.m_xInterface;
                const HWND edit = g_xChatEditBox.GetSafehWnd();
                RECT inputRect; GetWindowRect(edit,&inputRect);
                LOGFONTA font = {};
                GetObjectA((HFONT)SendMessage(edit,WM_GETFONT,0,0),sizeof(font),&font);
                const int limit = (int)SendMessage(edit,EM_GETLIMITTEXT,0,0);
                SetWindowTextA(edit,"PreviewMember");
                char input[91] = {}; GetWindowTextA(edit,input,sizeof(input));
                POINT cancel = {490,315};
                ui.OnLButtonDown(cancel); ui.OnLButtonUp(cancel);
                const bool restored = !ui.m_xMsgBox.IsActive() && !IsWindowVisible(edit) && GetWindowTextLengthA(edit)==0;
                if (FILE* report=fopen(groupPanel==2 ? "group-create-check.txt" : groupPanel==3 ? "group-invite-check.txt" : "group-remove-check.txt","wb")) {
                    fprintf(report,"font=%s height=%ld limit=%d input=%s cancel_restored=%d width=%ld height=%ld\n",
                        font.lfFaceName,font.lfHeight,limit,input,restored,inputRect.right-inputRect.left,inputRect.bottom-inputRect.top);
                    fclose(report);
                }
                ui.OnLButtonDown(button); ui.OnLButtonUp(button);
            }
        }
        if (groupPanel>=5 && groupPanel<=10) {
            auto& guild=g_xGameProc.m_xInterface.m_xGuildWnd;
            guild.m_name="Preview Guild";
            guild.m_pages[0].push_back("Native CLIENT guild overview");
            guild.m_pages[1].push_back("Leader: MirPreview");
            guild.m_pages[1].push_back("Member: WarriorPreview");
            guild.m_pages[2].push_back("Guild notice preview");
            g_xGameProc.m_xInterface.WindowActivate(_WND_ID_GUILD);
            POINT tab={145+(groupPanel>=8?0:groupPanel-5)*50,412};
            guild.OnLButtonDown(tab); guild.OnLButtonUp(tab);
            const auto savedPage=guild.m_pages[guild.m_tab];
            guild.m_pages[guild.m_tab].assign(40,"Scroll verification");
            guild.m_first=0; guild.m_scrollTick=timeGetTime()-11;
            POINT thumb={660,84}; guild.OnLButtonDown(thumb);
            POINT outside={760,590};
            g_xGameProc.m_xInterface.OnMouseMove(outside);
            const bool reachedEnd=guild.m_first==39;
            g_xGameProc.m_xInterface.OnLButtonUp(outside);
            const bool released=!guild.m_scrollCaptured;
            guild.m_first=0; guild.m_scrollTick=timeGetTime()-11;
            POINT track={660,200}; guild.OnLButtonDown(track);
            const bool pageStep=guild.m_first==18;
            guild.OnLButtonUp(track);
            if (FILE* report=fopen("guild-scroll-check.txt","wb")) {
                fprintf(report,"end=%d released=%d page=%d\n",reachedEnd,released,pageStep);
                fclose(report);
            }
            guild.m_pages[guild.m_tab]=savedPage; guild.m_first=0;
            if (groupPanel==9 || groupPanel==10) {
                guild.m_manager=true;
                POINT button={groupPanel==9?445:595,412};
                guild.OnLButtonDown(button); guild.OnLButtonUp(button);
            }
            if (groupPanel==8) {
                POINT edit={395,412};
                guild.OnLButtonDown(edit); guild.OnLButtonUp(edit);
            }

        }
        if (groupPanel>=12 && groupPanel<=14) {
            auto& ui=g_xGameProc.m_xInterface; auto& bag=ui.m_xInventoryWnd;
            ui.WindowActivate(_WND_ID_INVENTORY);
            const bool grid=bag.SlotAt(POINT{536,61})==0 && bag.SlotAt(POINT{762,363})==47 &&
                bag.SlotAt(POINT{534,61})==-1;
            auto& icons=g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY];
            icons.NewLoad("./Data/Inventory.wil");
            int samples=0;
            for (int icon=0;icon<100 && samples<6;++icon) {
                if (!icons.NewSetIndex(icon) || icons.m_lpstNewCurrWilImageInfo->shWidth<=0) continue;
                CLIENTITEMRCD record={}; record.stStdItem.wLooks=icon; record.nMakeIndex=1000+icon;
                wsprintfA(record.stStdItem.szName,"Preview%d",icon);
                CItem item; item.SetItem(record,0);
                bag.AddInvenItem(item,samples*7,TRUE); ++samples;
            }
            COMMONITEMSET held={};
            POINT thumb={770,75}; bag.OnLButtonDown(&held,thumb);
            POINT outside={810,590}; ui.OnMouseMove(outside);
            const bool scrollEnd=bag.ScrollRow()==92;
            ui.OnLButtonUp(outside);
            const bool released=!bag.m_scrollCaptured && GetCapture()!=g_xMainWnd.GetSafehWnd();
            for (int row=0;row<92;++row) bag.OnScrollDown();
            int largeIcon=-1,largeWidth=0,largeHeight=0;
            for (int icon=0;icon<5000;++icon) {
                if (!icons.NewSetIndex(icon)) continue;
                const auto& info=*icons.m_lpstNewCurrWilImageInfo;
                if (info.shWidth>38 && info.shHeight>38 && info.shWidth<=114 && info.shHeight<=152) {
                    largeIcon=icon; largeWidth=(info.shWidth+37)/38; largeHeight=(info.shHeight+37)/38; break;
                }
            }
            CBagLayout probe;
            const bool collision=probe.Place(0,2,3,0,0) && probe.Place(1,2,3,0,0) &&
                probe.anchors[0]==0 && probe.anchors[1]==2 && !probe.Fits(5,0,2,3) && !probe.Place(2,6,100);
            probe.Remove(0);
            const bool freed=probe.Fits(0,0,2,3) && probe.cells[2]==1;
            bool multiHit=false,picked=false,moved=false;
            if (largeIcon>=0) {
                CLIENTITEMRCD record={}; record.stStdItem.wLooks=largeIcon; record.nMakeIndex=1000+largeIcon;
                lstrcpyA(record.stStdItem.szName,"LargePreview");
                CItem item; item.SetItem(record,6); bag.AddInvenItem(item,6,TRUE);
                const int anchor=bag.ItemAnchor(6);
                POINT pick={535+(anchor%6+largeWidth-1)*38+2,60+(anchor/6+largeHeight-1)*38+2};
                multiHit=bag.GetInvenNum(pick)==6;
                bag.OnLButtonUp(&held,pick);
                picked=held.bSetted && held.xItem.m_stItemInfo.stStdItem.wLooks==largeIcon;
                for (int row=0;row<20;++row) bag.OnScrollUp();
                POINT drop={650,180}; bag.OnLButtonUp(&held,drop);
                const int movedId=bag.GetInvenNum(drop);
                moved=!held.bSetted && movedId>=0 && bag.ItemAnchor(movedId)>=120 &&
                    bag.m_stInventoryItem[movedId].xItem.m_stItemInfo.stStdItem.wLooks==largeIcon;
                bag.OnLButtonUp(&held,drop);
                for (int row=0;row<20;++row) bag.OnScrollDown();
                bag.OnLButtonUp(&held,drop);
            }
            ITEMSET savedItems[_MAX_INVEN_ITEM]; int savedAnchors[_MAX_INVEN_ITEM];
            memcpy(savedItems,bag.m_stInventoryItem,sizeof(savedItems));
            for (int id=0;id<_MAX_INVEN_ITEM;++id) savedAnchors[id]=bag.ItemAnchor(id);
            const bool layoutSaved=bag.SaveLayoutFile("./preview-item-layout.itms");
            bag.DeleteAllInvenItem();
            for (int id=_MAX_INVEN_ITEM-1;id>=0;--id) if (savedItems[id].bSetted) {
                CItem item=savedItems[id].xItem; item.m_stItemInfo.stStdItem.nPrice=12345;
                bag.AddInvenItem(item,0,FALSE);
            }
            bool layoutRestored=layoutSaved && bag.LoadLayoutFile("./preview-item-layout.itms");
            bool serverPreserved=true;
            for (int id=0;id<_MAX_INVEN_ITEM;++id) if (bag.m_stInventoryItem[id].bSetted) {
                const auto& record=bag.m_stInventoryItem[id].xItem.m_stItemInfo;
                serverPreserved=serverPreserved && record.stStdItem.nPrice==12345;
                for (int old=0;old<_MAX_INVEN_ITEM;++old)
                    if (savedItems[old].bSetted && savedItems[old].xItem.m_stItemInfo.nMakeIndex==record.nMakeIndex)
                        layoutRestored=layoutRestored && bag.ItemAnchor(id)==savedAnchors[old];
            }
            bag.UpdateGold(0xffffffffu);
            const bool goldUnsigned=g_xGameProc.m_xMyHero.m_nGlod==0xffffffffu;
            bag.UpdateGold(123456789u);
            const bool goldUpdated=g_xGameProc.m_xMyHero.m_nGlod==123456789u;
            bool green=false,red=false;
            for (int id=0;id<_MAX_INVEN_ITEM;++id) if (bag.m_stInventoryItem[id].bSetted &&
                bag.m_stInventoryItem[id].xItem.m_stItemInfo.stStdItem.wLooks==largeIcon) {
                RECT visible; bool fits;
                const auto item=bag.m_stInventoryItem[id].xItem;
                green=bag.PreviewPlacement(item,POINT{650,300},visible,fits) && fits;
                red=bag.PreviewPlacement(item,POINT{590,80},visible,fits) && !fits;
                if (groupPanel>=13) {
                    bag.SetCommonFromInven(id,&ui.m_stCommonItem);
                    mouse=groupPanel==13?POINT{650,300}:POINT{590,80};
                    g_xGameProc.m_ptMousePos=mouse;
                }
                break;
            }
            POINT close={770,435};
            const bool stray=!bag.OnLButtonUp(&held,close);
            bag.OnLButtonDown(&held,close);
            const bool closed=bag.OnLButtonUp(&held,close)!=FALSE;
            if (FILE* report=fopen("inventory-layout-check.txt","wb")) {
                fprintf(report,"grid=%d release_without_press=%d close_clicked=%d scroll_end=%d scroll_released=%d\n",grid,stray,closed,scrollEnd,released); fprintf(report,"icon_samples=%d large_icon=%d multi_hit=%d picked=%d moved=%d\n",samples,largeIcon,multiHit,picked,moved); fprintf(report,"collision=%d freed=%d layout_restored=%d server_records_preserved=%d\n",collision,freed,layoutRestored,serverPreserved); fprintf(report,"placement_green=%d placement_red=%d\n",green,red); fprintf(report,"gold_unsigned=%d gold_updated=%d\n",goldUnsigned,goldUpdated); fclose(report);
            }
        }
        if (groupPanel>=20 && groupPanel<=22) {
            auto& ui=g_xGameProc.m_xInterface; auto& skills=ui.m_xSkillWnd;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_MICON].NewLoad("./Data/MIcon.wil");
            const int ids[]={1,5,9,23,22,113,39,41,67,2,6,17,3,7,12,25,26,34,103,107,102,106,108};
            skills.m_bMyMagicCnt=sizeof(ids)/sizeof(ids[0]);
            skills.m_pstMyMagic=new CLIENTMAGICRCD[skills.m_bMyMagicCnt]{};
            for (int i=0;i<skills.m_bMyMagicCnt;++i) {
                auto& skill=skills.m_pstMyMagic[i]; skill.stStdMagic.wMagicID=ids[i];
                skill.bLevel=i%4; skill.nCurrTrain=120; skill.stStdMagic.nTrain[skill.bLevel]=500;
                wsprintfA(skill.stStdMagic.szMagicName,"Skill %d",ids[i]);
            }
            lstrcpyA(skills.m_pstMyMagic[0].stStdMagic.szMagicName,"\xbb\xf9\xb1\xbe\xbd\xa3\xca\xf5");
            skills.m_pstMyMagic[0].bUseKey='1';
            ui.WindowActivate(_WND_ID_SKILL); ui.WindowActivate(_WND_ID_STATUS);
            const bool origin=skills.m_rcWnd.left==348 && skills.m_rcWnd.top==0 && skills.m_nWidth==360 && skills.m_nHeight==488;
            ui.OnLButtonDown(POINT{700,410}); ui.OnMouseMove(POINT{690,420}); ui.OnLButtonUp(POINT{690,420});
            const bool draggedWindow=skills.m_rcWnd.left==338 && skills.m_rcWnd.top==10 && ui.m_xStatusWnd.m_rcWnd.left==0;
            ui.OnKeyDown(VK_F11,0); const bool hiddenSkill=!skills.GetGameWndActive() && ui.m_xStatusWnd.GetGameWndActive();
            ui.OnKeyDown(VK_F11,0); const bool restoredSkills=skills.GetGameWndActive() && skills.m_rcWnd.left==338 && skills.m_rcWnd.top==10;
            ui.OnSysKeyDown(); const bool hiddenStatus=!ui.m_xStatusWnd.GetGameWndActive() && skills.GetGameWndActive();
            ui.OnSysKeyDown(); const bool restoredStatus=ui.m_xStatusWnd.GetGameWndActive() && ui.m_xStatusWnd.m_rcWnd.left==0;
            const bool distinctHit=ui.GetWindowInMousePos(POINT{100,100})==_WND_ID_STATUS && ui.GetWindowInMousePos(POINT{400,100})==_WND_ID_SKILL;
            skills.m_rcWnd={348,0,708,488};
            COMMONITEMSET held={}; POINT close={670,430};
            const bool stray=!skills.OnLButtonUp(close);
            skills.OnLButtonDown(POINT{380,80});
            const bool selected=skills.m_skillSelected[0]==0 && skills.SkillAt(POINT{380,80})==0;
            skills.OnLButtonUp(POINT{380,80});
            ui.OnLButtonDown(POINT{635,35}); const bool buttonFocus=ui.IsTopWindow(_WND_ID_SKILL);
            ui.OnMouseMove(POINT{636,36}); const bool buttonNoDrag=skills.m_rcWnd.left==348 && skills.m_rcWnd.top==0;
            ui.OnLButtonUp(POINT{636,36});
            const bool tab=skills.m_skillTab==7;
            for (int i=0;i<8;++i) skills.OnScrollUp();
            const bool bottom=skills.m_skillScroll==190;
            for (int i=0;i<8;++i) skills.OnScrollDown();
            const bool top=skills.m_skillScroll==0;
            skills.m_skillScrollTime=GetTickCount()-20;
            const bool captured=skills.SkillScrollInput(0,POINT{673,104}) && skills.m_skillScrollCaptured;
            ui.OnMouseMove(POINT{790,580}); const bool dragged=skills.m_skillScroll==190;
            ui.OnLButtonUp(POINT{790,580}); const bool released=!skills.m_skillScrollCaptured;
            skills.OnLButtonDown(close); ui.OnLButtonUp(POINT{790,580});
            const bool outsideRelease=skills.m_skillPressed==-1 && !skills.OnLButtonUp(close);
            skills.OnLButtonDown(close); const bool closed=skills.OnLButtonUp(close)!=FALSE;
            ui.OnLButtonDown(close); ui.OnLButtonUp(close);
            const bool skillClose=!skills.GetGameWndActive() && ui.m_xStatusWnd.GetGameWndActive();
            ui.WindowActivate(_WND_ID_SKILL);
            ui.OnLButtonDown(POINT{300,25}); ui.OnLButtonUp(POINT{300,25});
            const bool statusClose=!ui.m_xStatusWnd.GetGameWndActive() && skills.GetGameWndActive();
            ui.WindowActivate(_WND_ID_STATUS);
            lstrcpyA(hero.m_szName,"MirPreview");
            if (FILE* report=fopen("skill-layout-check.txt","wb")) {
                fprintf(report,"stray_release=%d selected=%d tab=%d bottom=%d top=%d captured=%d dragged=%d released=%d close=%d\n",
                    stray,selected,tab,bottom,top,captured,dragged,released,closed);
                fprintf(report,"skill_close_independent=%d status_close_independent=%d\n",skillClose,statusClose);
                fprintf(report,"button_focus=%d button_no_drag=%d\n",buttonFocus,buttonNoDrag);
                fprintf(report,"dragged_window=%d skill_toggle=%d status_toggle=%d distinct_hit=%d\n",draggedWindow,hiddenSkill,hiddenStatus,distinctHit);
                fprintf(report,"origin=%d status_position=%d skill_position=%d outside_release=%d\n",origin,restoredStatus,restoredSkills,outsideRelease); fclose(report);
            }
            skills.m_skillTab=groupPanel==21?7:0; skills.m_skillScroll=groupPanel==21?190:0;
            if (groupPanel!=22) ui.WindowActivate(_WND_ID_STATUS);
            if (groupPanel==21) skills.m_skillSelected[7]=13;
        }
        if (groupPanel>=15 && groupPanel<=19) {
            auto& ui=g_xGameProc.m_xInterface; auto& status=ui.m_xStatusWnd;
            lstrcpyA(hero.m_szName,"MirPreview");
            hero.m_stAbility.wDC=MAKEWORD(12,24); hero.m_stAbility.wAC=MAKEWORD(5,10);
            hero.m_stAbility.wMC=MAKEWORD(7,18); hero.m_stAbility.wSC=MAKEWORD(3,6); hero.m_stAbility.wMAC=MAKEWORD(4,8);
            hero.m_stAbility.bWearWeight=12; hero.m_stAbility.bMaxWearWeight=40;
            hero.m_stAbility.bHandWeight=5; hero.m_stAbility.bMaxHandWeight=15;
            if (groupPanel>=16) {
                status.m_elementValues[0][0]=5; status.m_elementValues[0][3]=12; status.m_elementValues[0][6]=255;
                status.m_elementValues[1][1]=2; status.m_elementValues[1][4]=255; status.m_elementValues[2][2]=3;
                mouse=groupPanel==16?POINT{78,390}:POINT{220,140};
                g_xGameProc.m_ptMousePos=mouse;
            }
            if (groupPanel==18) {
                lstrcpyA(hero.m_szName,"\xb2\xe2\xca\xd4\xbd\xc7\xc9\xab");
                lstrcpyA(hero.m_szGuildName,"\xb2\xe2\xca\xd4\xd0\xd0\xbb\xe1");
                status.m_identityTertiary="\xd5\xc6\xc3\xc5"; status.m_identityBadge="\xc8\xd9\xd3\xfe\xd3\xc2\xca\xbf";
                mouse=POINT{0,0}; g_xGameProc.m_ptMousePos=mouse;
            }
            if (groupPanel==19) {
                g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
                CLIENTITEMRCD record={}; record.stStdItem.bStdMode=25; record.stStdItem.wLooks=75;
                ui.m_stCommonItem.xItem.SetItem(record,0); ui.m_stCommonItem.bSetted=TRUE;
                mouse=POINT{149,284}; g_xGameProc.m_ptMousePos=mouse;
            }
            ui.WindowActivate(_WND_ID_STATUS);
            COMMONITEMSET held={}; POINT close={300,25};
            const bool stray=!status.OnLButtonUp(&held,close);
            status.OnLButtonDown(&held,close); const bool closed=status.OnLButtonUp(&held,close)!=FALSE;
            const bool slots=status.GetEquipNum(POINT{100,120})==0 && status.GetEquipNum(POINT{15,200})==7;
            CLIENTITEMRCD charmRecord={}; charmRecord.stStdItem.bStdMode=25;
            CItem charm; charm.SetItem(charmRecord,0);
            const bool previewRules=status.EquipmentPreviewFrame(25,5)==-1 && status.EquipmentPreviewFrame(25,9)==1235 &&
                status.EquipmentPreviewFrame(53,10)==1236 && status.EquipmentPreviewFrame(60,11)==1235 &&
                status.CheckEquipItem(&charm,5,POINT{20,165}) && status.CheckEquipItem(&charm,9,POINT{140,275}) &&
                status.GetEquipNum(POINT{15,260})==10 && status.GetEquipNum(POINT{175,270})==11;
            CLIENTMAGICRCD skills[3]={}; skills[0].bUseKey='1'; skills[2].bUseKey='8';
            const auto savedMagic=ui.m_xSkillWnd.m_pstMyMagic; const BYTE savedCount=ui.m_xSkillWnd.m_bMyMagicCnt;
            ui.m_xSkillWnd.m_pstMyMagic=skills; ui.m_xSkillWnd.m_bMyMagicCnt=3;
            const bool keyMatches=ui.m_xSkillWnd.GetMagicByKey('1')==&skills[0] && ui.m_xSkillWnd.GetMagicByKey('8')==&skills[2];
            const bool missingKey=ui.m_xSkillWnd.GetMagicByKey('2')==NULL && ui.m_xSkillWnd.GetMagicByKey(0)==NULL;
            ui.m_xSkillWnd.m_bMyMagicCnt=0; const bool emptySkills=ui.m_xSkillWnd.GetMagicByKey('1')==NULL;
            ui.m_xSkillWnd.m_pstMyMagic=savedMagic; ui.m_xSkillWnd.m_bMyMagicCnt=savedCount;
            if (FILE* report=fopen("skill-key-check.txt","wb")) {
                fprintf(report,"bound_first_last=%d unbound_and_zero=%d empty_list=%d\n",keyMatches,missingKey,emptySkills); fclose(report);
            }
            if (FILE* report=fopen("status-layout-check.txt","wb")) {
                fprintf(report,"release_without_press=%d close_clicked=%d slots=%d signed_elements=%d\n",stray,closed,slots,groupPanel<16 || (status.ElementValue(0,6)==255 && status.ElementValue(1,4)==-1 && status.ElementValue(1,0)==0)); fprintf(report,"equipment_rules=%d\n",previewRules); fclose(report);
            }
        }
        if (groupPanel==11) {
            auto& ui=g_xGameProc.m_xInterface;
            auto& options=ui.m_xOptionWnd;
            ui.WindowActivate(_WND_ID_OPTION);
            POINT knob={605,150}; options.OnLButtonDown(knob);
            POINT left={400,150}; ui.OnMouseMove(left);
            const bool minimum=options.m_musicLevel==-100 && g_xSound.m_lMp3MaxVol==-4000;
            POINT right={760,150}; ui.OnMouseMove(right); ui.OnLButtonUp(right);
            const bool maximum=options.m_musicLevel==2 && g_xSound.m_lMp3MaxVol==0 && options.m_audioDrag==-1;
            POINT effects={605,210}; options.OnLButtonDown(effects);
            POINT middle={523,210}; ui.OnMouseMove(middle); ui.OnLButtonUp(middle);
            const bool independent=options.m_effectsLevel==-50 && g_xSound.m_lWavMaxVol==-2000 && g_xSound.m_lMp3MaxVol==0;
            POINT reverse={580,240}; options.OnLButtonDown(reverse); options.OnLButtonUp(reverse);
            const bool reversed=options.m_values[3] && g_xSound.m_reversePan;
            options.OnLButtonDown(reverse); options.OnLButtonUp(reverse);
            const bool restored=!options.m_values[3] && !g_xSound.m_reversePan;
            options.OnLButtonDown(reverse); options.OnLButtonUp(reverse);
            options.LoadAudioOptions("./preview-audio-options.ini");
            options.m_musicLevel=-25; options.m_effectsLevel=-75;
            options.m_values[1]=false; options.m_values[2]=true; options.m_values[3]=true;
            const bool saved=options.SaveAudioOptions();
            options.LoadAudioOptions(nullptr);
            options.LoadAudioOptions("./preview-audio-options.ini");
            const bool restoredSettings=saved && options.m_musicLevel==-25 && options.m_effectsLevel==-75 &&
                !options.m_values[1] && options.m_values[2] && options.m_values[3] &&
                g_xSound.m_lMp3MaxVol==-1000 && g_xSound.m_lWavMaxVol==-3000 && g_xSound.m_reversePan;
            char musicPath[MAX_PATH];
            GetFullPathNameA("Sound/SelChr.mp3",MAX_PATH,musicPath,NULL);
            const bool musicStarted=g_xSound.m_pMp3->LoadMp3(musicPath,g_xMainWnd.GetSafehWnd()) &&
                g_xSound.m_pMp3->OnPlay(-4000,TRUE) && g_xSound.m_pMp3->IsPlaying();
            g_xSound.SetDisableBGM();
            const bool musicStopped=!g_xSound.m_pMp3->IsPlaying();
            g_xSound.SetEnableBGM();
            const bool musicResumed=g_xSound.m_pMp3->IsPlaying();
            g_xSound.SetDisableBGM();
            if (FILE* report=fopen("options-audio-check.txt","wb")) {
                fprintf(report,"minimum=%d maximum_released=%d independent=%d\n",minimum,maximum,independent);
                fprintf(report,"reverse_enabled=%d reverse_restored=%d settings_restored=%d\n",reversed,restored,restoredSettings);
                fprintf(report,"music_started=%d music_stopped=%d music_resumed=%d\n",musicStarted,musicStopped,musicResumed);
                fclose(report);
            }
        }
        if (exitDialog) {
            POINT button = {10,542};
            g_xGameProc.m_xInterface.OnLButtonDown(button);
            g_xGameProc.m_xInterface.OnLButtonUp(button);
            auto& ui = g_xGameProc.m_xInterface;
            const int first = ui.m_nFstLine;
            ui.OnScrollDown(); ui.OnScrollUp();
            const bool wheelBlocked = first == ui.m_nFstLine;
            POINT cancel = {490,315};
            ui.OnLButtonDown(cancel); ui.OnLButtonUp(cancel);
            const bool cancelled = !ui.m_xMsgBox.IsActive();
            ui.OnLButtonDown(button); ui.OnLButtonUp(button);
            ui.OnKeyDown(VK_ESCAPE,0);
            const bool escapeBlocked = ui.m_xMsgBox.IsActive()!=FALSE;
            ui.OnKeyDown(VK_TAB,0); ui.OnKeyDown(VK_RETURN,0);
            const bool keyboardCancelled = !ui.m_xMsgBox.IsActive();
            ui.OnLButtonDown(button); ui.OnLButtonUp(button);
            if (FILE* report=fopen("hud-exit-check.txt","wb")) {
                fprintf(report,"wheel_blocked=%d cancelled=%d escape_blocked=%d keyboard_cancelled=%d reopened=%d\n",
                    wheelBlocked,cancelled,escapeBlocked,keyboardCancelled,ui.m_xMsgBox.IsActive()!=FALSE);
                fclose(report);
            }
        }
        if (chatMode) {
            auto& ui = g_xGameProc.m_xInterface;
            POINT button = {chatMode == 1 ? 270 : 310,560};
            ui.OnLButtonDown(button); ui.OnLButtonUp(button);
            ui.FocusChatMode();
            char prefix[16] = {};
            GetWindowTextA(g_xChatEditBox.GetSafehWnd(),prefix,sizeof(prefix));
            const bool selected = ui.m_chatMode == chatMode && strcmp(prefix,chatMode == 1 ? "!!" : "!~") == 0;
            ui.OnLButtonDown(button); ui.OnLButtonUp(button);
            const bool toggledOff = ui.m_chatMode == 0;
            ui.OnLButtonDown(button); ui.OnLButtonUp(button);
            if (FILE* report=fopen(chatMode == 1 ? "hud-group-check.txt" : "hud-guild-check.txt","wb")) {
                fprintf(report,"mode=%d prefix=%s selected=%d toggled_off=%d\n",ui.m_chatMode,prefix,selected,toggledOff);
                fclose(report);
            }
        }

    }
    void Render(int elapsed) {
        RECT canvas = {0,0,800,600};
        DrawLegacyPanel(canvas,RGB(20,24,28),255,0,false);
        g_xGameProc.m_xInterface.RenderInterface(elapsed,mouse);
        if (capturePath) {
            auto* buffer = g_xMainWnd.GetBackBuffer();
            DDSURFACEDESC2 surface = {}; surface.dwSize = sizeof(surface);
            if (SUCCEEDED(buffer->Lock(NULL,&surface,DDLOCK_WAIT|DDLOCK_READONLY,NULL))) {
                const int stride = (surface.dwWidth*3+3)&~3;
                std::vector<BYTE> pixels(stride*surface.dwHeight);
                for (DWORD y=0;y<surface.dwHeight;++y) {
                    const WORD* source = (WORD*)((BYTE*)surface.lpSurface+y*surface.lPitch);
                    BYTE* dest = &pixels[(surface.dwHeight-1-y)*stride];
                    for (DWORD x=0;x<surface.dwWidth;++x) {
                        const WORD color=source[x];
                        dest[x*3] = BYTE((color&31)*255/31);
                        dest[x*3+1] = BYTE(((color>>5)&63)*255/63);
                        dest[x*3+2] = BYTE((color>>11)*255/31);
                    }
                }
                buffer->Unlock(NULL);
                BITMAPFILEHEADER file = {}; BITMAPINFOHEADER info = {};
                file.bfType=0x4d42; file.bfOffBits=sizeof(file)+sizeof(info);
                file.bfSize=file.bfOffBits+(DWORD)pixels.size();
                info.biSize=sizeof(info); info.biWidth=surface.dwWidth; info.biHeight=surface.dwHeight;
                info.biPlanes=1; info.biBitCount=24; info.biSizeImage=(DWORD)pixels.size();
                // Capture the real native edit control, then map its display pixels to the 800x600 review image.
                const HWND edit = g_xChatEditBox.GetSafehWnd();
                if (IsWindowVisible(edit)) {
                    RECT editRect; GetWindowRect(edit,&editRect);
                    const RECT& display = g_xMainWnd.m_rcWindow;
                    const int width = editRect.right-editRect.left, height = editRect.bottom-editRect.top;
                    HDC target = CreateCompatibleDC(NULL), source = CreateCompatibleDC(NULL);
                    BITMAPINFO bitmapInfo = {}; bitmapInfo.bmiHeader=info;
                    void* bits = NULL;
                    HBITMAP combined = CreateDIBSection(target,&bitmapInfo,DIB_RGB_COLORS,&bits,NULL,0);
                    HGDIOBJ oldTarget = SelectObject(target,combined);
                    memcpy(bits,pixels.data(),pixels.size());
                    HBITMAP control = CreateCompatibleBitmap(target,width,height);
                    HGDIOBJ oldSource = SelectObject(source,control);
                    SendMessage(edit,WM_PRINT,(WPARAM)source,PRF_CLIENT|PRF_ERASEBKGND);
                    const int x=MulDiv(editRect.left-display.left,800,display.right-display.left);
                    const int y=MulDiv(editRect.top-display.top,600,display.bottom-display.top);
                    SetStretchBltMode(target,COLORONCOLOR);
                    StretchBlt(target,x,y,MulDiv(width,800,display.right-display.left),
                        MulDiv(height,600,display.bottom-display.top),source,0,0,width,height,SRCCOPY);
                    GdiFlush();
                    memcpy(pixels.data(),bits,pixels.size());
                    SelectObject(source,oldSource); SelectObject(target,oldTarget);
                    DeleteObject(control); DeleteObject(combined);
                    DeleteDC(source); DeleteDC(target);
                }
                if (FILE* output=fopen(capturePath,"wb")) {
                    fwrite(&file,sizeof(file),1,output); fwrite(&info,sizeof(info),1,output);
                    fwrite(pixels.data(),pixels.size(),1,output); fclose(output);
                }
                capturePath=NULL;
            }
        }

    }
    LRESULT DefMainWndProc(HWND window,UINT message,WPARAM wp,LPARAM lp) override {
        auto& ui = g_xGameProc.m_xInterface;
        switch (message) {
        case WM_MOUSEMOVE:
            mouse.x = (short)LOWORD(lp); mouse.y = (short)HIWORD(lp);
            ui.OnMouseMove(mouse); return 0;
        case WM_LBUTTONDOWN:
            mouse.x = (short)LOWORD(lp); mouse.y = (short)HIWORD(lp);
            SetCapture(window); ui.OnLButtonDown(mouse); return 0;
        case WM_LBUTTONUP:
            mouse.x = (short)LOWORD(lp); mouse.y = (short)HIWORD(lp);
            ui.OnLButtonUp(mouse); ReleaseCapture(); return 0;
        case WM_MOUSEWHEEL:
            if ((short)HIWORD(wp)>0) ui.OnScrollDown();
            else if ((short)HIWORD(wp)<0) ui.OnScrollUp();
            return 0;
        case WM_CTLCOLOREDIT:
            return g_xGameProc.DefMainWndProc(window,message,wp,lp);
        }
        return CWHDefProcess::DefMainWndProc(window,message,wp,lp);
    }
};

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG	msg;
	// Resolve config and resources beside the executable, including shortcut launches.
	char executable[MAX_PATH];
	GetModuleFileName(NULL, executable, MAX_PATH);
	char* separator = strrchr(executable, '\\');
	if (separator) { *separator = 0; SetCurrentDirectory(executable); }


	if (!g_xMainWnd.Create(hInstance, "传奇3", NULL, MAKEINTRESOURCE(IDI_ICON), _DXG_SCREENMODE_WINDOW, _DXG_DEVICEMODE_PRIMARY|_DXG_DEVICEMODE_D3D|_DXG_DEVICEMODE_ZBUFFER)) return -1;
	ShowWindow(g_xMainWnd.GetSafehWnd(), SW_HIDE);

	g_xSound.InitMirSound(g_xMainWnd.GetSafehWnd());
    g_xGameProc.m_xInterface.m_xOptionWnd.LoadAudioOptions(strstr(lpCmdLine,"--preview=") ? nullptr : "./Config.ini");
	g_xSpriteInfo.SetInfo();
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 编辑框创建.
	g_xChatEditBox.Create(g_xMainWnd.GetSafehInstance(), g_xMainWnd.GetSafehWnd(), 0, 0, 0, 0);
	ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Random Seed初始化.
	srand((unsigned)time(NULL));
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 初始化 Windows Socket DLL。
	WSAData wsd;
	if( WSAStartup( MAKEWORD(2, 2), &wsd ) != 0 )
		return -1;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  开始时设定为登录流程.
	const char* preview = "--preview=";
	CHudPreviewProcess hudPreview;
	const int previewChatMode = strcmp(lpCmdLine,"--preview=hud-group")==0 ? 1 : strcmp(lpCmdLine,"--preview=hud-guild")==0 ? 2 : 0;
	const int previewGroup = strcmp(lpCmdLine,"--preview=group-panel")==0 ? 1 :
        strcmp(lpCmdLine,"--preview=group-create")==0 ? 2 :
        strcmp(lpCmdLine,"--preview=group-invite")==0 ? 3 :
        strcmp(lpCmdLine,"--preview=group-remove")==0 ? 4 :
        strcmp(lpCmdLine,"--preview=guild-overview")==0 ? 5 :
        strcmp(lpCmdLine,"--preview=guild-members")==0 ? 6 :
        strcmp(lpCmdLine,"--preview=guild-notice")==0 ? 7 :
        strcmp(lpCmdLine,"--preview=guild-denied")==0 ? 8 :
        strcmp(lpCmdLine,"--preview=guild-remove")==0 ? 9 :
        strcmp(lpCmdLine,"--preview=guild-break-ally")==0 ? 10 :
        strcmp(lpCmdLine,"--preview=options-panel")==0 ? 11 :
        strcmp(lpCmdLine,"--preview=inventory-panel")==0 ? 12 :
        strcmp(lpCmdLine,"--preview=inventory-place")==0 ? 13 :
        strcmp(lpCmdLine,"--preview=inventory-conflict")==0 ? 14 :
        strcmp(lpCmdLine,"--preview=status-panel")==0 ? 15 :
        strcmp(lpCmdLine,"--preview=status-elements")==0 ? 16 :
        strcmp(lpCmdLine,"--preview=status-tooltip")==0 ? 17 :
        strcmp(lpCmdLine,"--preview=status-identity")==0 ? 18 :
        strcmp(lpCmdLine,"--preview=status-equipment")==0 ? 19 :
        strcmp(lpCmdLine,"--preview=skills-panel")==0 ? 20 :
        strcmp(lpCmdLine,"--preview=skills-scrolled")==0 ? 21 :
        strcmp(lpCmdLine,"--preview=status-and-skills")==0 ? 22 : 0;
	const bool previewExit = strcmp(lpCmdLine,"--preview=hud-exit")==0;
	const bool previewHud = strcmp(lpCmdLine,"--preview=hud")==0 || strcmp(lpCmdLine,"--preview=hud-expanded")==0 || previewChatMode != 0 || previewExit || previewGroup;
	if (previewHud) {
		g_xMainWnd.m_pxDefProcess = &hudPreview;
		hudPreview.Load(strcmp(lpCmdLine,"--preview=hud-expanded")==0 || previewChatMode != 0,previewChatMode,previewExit,previewGroup);
		g_bProcState = _GAME_PROC;
	} else if (strncmp(lpCmdLine, preview, strlen(preview)) == 0) {
		g_xMainWnd.m_pxDefProcess = &g_xChrSelProc;
		g_xChrSelProc.Preview(lpCmdLine + strlen(preview));
		g_bProcState = _CHAR_SEL_PROC;
	} else {
		g_xClientSocket.m_pxDefProc = g_xMainWnd.m_pxDefProcess = &g_xLoginProc;
		g_xLoginProc.Load();
		g_bProcState = _LOGIN_PROC;
	}

	UpdateWindow(g_xMainWnd.GetSafehWnd());
	ShowWindow(g_xMainWnd.GetSafehWnd(), SW_SHOW);
	g_nVersion = 20090403;


	DWORD dwLastTime, dwTime, dwDelay;
	dwLastTime = timeGetTime();


	while (TRUE)
    {
        if ( PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) )
        {
            if ( 0 == GetMessage(&msg, NULL, 0, 0) )
                return (int) msg.wParam;

            if (msg.message == WM_DSHOW_NOTIFY)
                g_xSound.MessageProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
	        TranslateMessage(&msg);
            DispatchMessage(&msg);
		}
        else
        {
			dwTime = timeGetTime();
			dwDelay = dwTime - dwLastTime;
			dwLastTime = dwTime;

			if ( dwDelay!=0 /*&& g_xMainWnd.m_bIsWindowActive && g_xMainWnd.m_bIsWindowReady */)
			{
				switch ( g_bProcState )
				{
				case _LOGIN_PROC:
					g_xLoginProc.RenderScene(dwDelay);
					break;
				case _CHAR_SEL_PROC:
					g_xChrSelProc.RenderScene(dwDelay);
					break;
				case _GAME_PROC:
					if (previewHud) hudPreview.Render(dwDelay);
					else g_xGameProc.RenderScene(dwDelay);
					break;
				}

/*				if ( HIBYTE(GetKeyState(VK_RBUTTON)) )
				{
					POINT ptMouse;
					GetCursorPos(&ptMouse);
					ScreenToClient(g_xMainWnd.GetSafehWnd(), &ptMouse);
					m_xWave.DropAirWaveMap( ptMouse.x, ptMouse.y, 40, -100 );
				}

				g_xMainWnd.DrawWithImageForComp(0, 0, 
								xImage.m_lpstNewCurrWilImageInfo->shWidth, xImage.m_lpstNewCurrWilImageInfo->shHeight,
								(WORD*)(xImage.m_pbCurrImage));
				m_xWave.UpdateAirWave();
				m_xWave.RenderAirWave();
*/
				if ( FAILED(g_xMainWnd.Present()) )
				{
					g_xMainWnd.RestoreSurfaces();
				}
			}
		}
	}

	return msg.wParam;
}
