#include "StdAfx.h"
#include "GameProcess/AccessoryHintIds.h"
namespace { constexpr int PreviewVitalAccessory=141, PreviewDefenseAccessory=142, PreviewRecoveryAccessory=143, PreviewAccuracyBracelet=144; }
#include "Common/LegacyPanel.h"
#include "Common/LoginMessages.h"

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
    int damagePreview=0;
    const char* capturePath = NULL;
public:
    void OnConnectToServer() override {}
    char* OnMessageReceive(CHAR*) override { return NULL; }
    void Load(bool expanded, int chatMode, bool exitDialog, int groupPanel) {
        damagePreview=groupPanel==95 || groupPanel==96 ? groupPanel : 0;
        const int extraHintPreview=groupPanel>=67 && groupPanel<=71?groupPanel:0;
        if(extraHintPreview) { const int bases[]={1,5,20,33,42}; groupPanel=bases[extraHintPreview-67]; }
        const int hintPreview=groupPanel>=64 && groupPanel<=66?groupPanel:0;
        if(hintPreview) groupPanel=hintPreview==64?12:hintPreview==65?15:29;
        capturePath = chatMode == 1 ? "hud-group.bmp" : chatMode == 2 ? "hud-guild.bmp" : expanded ? "hud-expanded.bmp" : "hud-collapsed.bmp";
        if (exitDialog) capturePath = "hud-exit.bmp";
        if (groupPanel>0 && groupPanel<5) capturePath = groupPanel==2 ? "group-create.bmp" : groupPanel==3 ? "group-invite.bmp" : groupPanel==4 ? "group-remove.bmp" : "group-panel.bmp";
        if (groupPanel>=5 && groupPanel<=10) capturePath = groupPanel==5 ? "guild-overview.bmp" : groupPanel==6 ? "guild-members.bmp" : groupPanel==8 ? "guild-denied.bmp" : groupPanel==9 ? "guild-remove.bmp" : groupPanel==10 ? "guild-break-ally.bmp" : "guild-notice.bmp";
        if ((groupPanel>=20 && groupPanel<=22) || groupPanel==25 || groupPanel==26 || groupPanel==27) capturePath=groupPanel==27?"skill-shortcuts.bmp":groupPanel==26?"skill-description.bmp":groupPanel==25?"skill-key-menu.bmp":groupPanel==22?"status-and-skills.bmp":groupPanel==20?"skills-panel.bmp":"skills-scrolled.bmp";
        if (groupPanel==28 || groupPanel==29) capturePath=groupPanel==28?"quest-empty.bmp":"quest-list.bmp";
        if (groupPanel==30 || groupPanel==31) capturePath=groupPanel==30?"mount-panel.bmp":"mount-hint.bmp";
        if (groupPanel>=32 && groupPanel<=36) capturePath=groupPanel==36?"trade-return.bmp":groupPanel==35?"trade-gold.bmp":groupPanel==32?"trade-empty.bmp":groupPanel==33?"trade-list.bmp":"trade-confirmed.bmp";
        if (groupPanel>=60 && groupPanel<=63) capturePath=groupPanel==60?"portrait-empty.bmp":groupPanel==61?"portrait-preview.bmp":groupPanel==62?"portrait-scrolled.bmp":"portrait-hint.bmp";
        if (groupPanel==59) capturePath="npc-shop-layering.bmp";
        if (groupPanel==58) capturePath="npc-password.bmp";
        if (groupPanel==56 || groupPanel==57) capturePath=groupPanel==56?"merchant-sell-quantity.bmp":"merchant-deposit-quantity.bmp";
        if (groupPanel>=53 && groupPanel<=55) capturePath=groupPanel==53?"merchant-repair.bmp":groupPanel==54?"merchant-sell.bmp":"merchant-deposit.bmp";
        if (groupPanel>=50 && groupPanel<=52) capturePath=groupPanel==50?"storage-list.bmp":groupPanel==51?"storage-page.bmp":"storage-quantity.bmp";
        if (groupPanel>=46 && groupPanel<=49) capturePath=groupPanel==46?"shop-goods.bmp":groupPanel==47?"shop-offers.bmp":groupPanel==48?"shop-bulk.bmp":"shop-quantity.bmp";
        if (groupPanel>=42 && groupPanel<=45) capturePath=groupPanel==42?"npc-dialogue.bmp":groupPanel==43?"npc-scrolled.bmp":groupPanel==44?"npc-input.bmp":"npc-notclose.bmp";
        if (groupPanel==40 || groupPanel==41) capturePath=groupPanel==40?"guild-notice-editor.bmp":"guild-members-editor.bmp";
        if (groupPanel==39) capturePath="quick-binding.bmp";
        if (groupPanel==37 || groupPanel==38) capturePath=groupPanel==37?"quick-empty.bmp":"quick-items.bmp";
        if (groupPanel==11) capturePath="options-panel.bmp";
        if (groupPanel>=15 && groupPanel<=19) capturePath=groupPanel==19?"status-equipment.bmp":groupPanel==18?"status-identity.bmp":groupPanel==16?"status-elements.bmp":groupPanel==17?"status-tooltip.bmp":"status-panel.bmp";
        if (groupPanel==23 || groupPanel==24) capturePath=groupPanel==23?"inventory-clip-top.bmp":"inventory-clip-bottom.bmp";
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
        if ((groupPanel>=12 && groupPanel<=14) || groupPanel==23 || groupPanel==24) {
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
                if (groupPanel==13 || groupPanel==14) {
                    bag.SetCommonFromInven(id,&ui.m_stCommonItem);
                    mouse=groupPanel==13?POINT{650,300}:POINT{590,80};
                    g_xGameProc.m_ptMousePos=mouse;
                }
                break;
            }
            if (groupPanel==23 || groupPanel==24) {
                CLIENTITEMRCD record={}; record.stStdItem.wLooks=largeIcon; record.nMakeIndex=2000+largeIcon;
                lstrcpyA(record.stStdItem.szName,"ClipPreview"); CItem item; item.SetItem(record,0);
                icons.NewSetIndex(largeIcon);
                const int width=icons.m_lpstNewCurrWilImageInfo->shWidth,height=icons.m_lpstNewCurrWilImageInfo->shHeight;
                const RECT canvas={0,0,800,600};
                const auto pixels=[]() {
                    std::vector<WORD> result; auto* buffer=g_xMainWnd.GetBackBuffer();
                    DDSURFACEDESC2 desc={}; desc.dwSize=sizeof(desc);
                    if (SUCCEEDED(buffer->Lock(NULL,&desc,DDLOCK_WAIT|DDLOCK_READONLY,NULL))) {
                        result.resize(800*600);
                        for (int row=0;row<600;++row) memcpy(result.data()+row*800,(BYTE*)desc.lpSurface+row*desc.lPitch,800*sizeof(WORD));
                        buffer->Unlock(NULL);
                    }
                    return result;
                };
                DrawLegacyPanel(canvas,RGB(20,24,28),255,0,false); const auto background=pixels();
                item.DrawItem(200,200); const auto full=pixels();
                const int left=219-width/2,top=219-height/2;
                const RECT clips[4]={{left+width/2,top,left+width,top+height},{left,top,left+width/2,top+height},
                    {left,top+height/2,left+width,top+height},{left,top,left+width,top+height/2}};
                bool checks[4]={};
                for (int edge=0;edge<4;++edge) {
                    DrawLegacyPanel(canvas,RGB(20,24,28),255,0,false);
                    item.DrawItem(200,200,_ITEM_TYPE_INVENTORY,&clips[edge]); const auto cropped=pixels();
                    int different=0,visible=0;
                    if (background.size()==800*600 && full.size()==background.size() && cropped.size()==background.size()) {
                        for (int row=0;row<600;++row) for (int col=0;col<800;++col) {
                            const int index=row*800+col; const POINT point={col,row};
                            const bool inside=PtInRect(&clips[edge],point)!=FALSE;
                            const WORD expected=inside?full[index]:background[index];
                            different+=cropped[index]!=expected;
                            visible+=inside && full[index]!=background[index];
                        }
                        checks[edge]=different==0 && visible>0;
                    }
                }
                bag.DeleteAllInvenItem(); bag.AddInvenItem(item,0,TRUE);
                if (groupPanel==23) bag.OnScrollUp();
                else {
                    COMMONITEMSET moving={}; bag.SetCommonFromInven(0,&moving);
                    POINT drop={650,360}; bag.SetInvenFromCommon(0,&moving,&drop);
                }
                if (FILE* report=fopen("inventory-clip-check.txt","wb")) {
                    fprintf(report,"left=%d right=%d top=%d bottom=%d icon=%d width=%d height=%d row=%d anchor=%d\n",
                        checks[0],checks[1],checks[2],checks[3],largeIcon,width,height,bag.ScrollRow(),bag.ItemAnchor(0)); fclose(report);
                }
            }
            POINT close={770,435};
            const bool stray=!bag.OnLButtonUp(&held,close);
            bag.OnLButtonDown(&held,close);
            const bool closed=bag.OnLButtonUp(&held,close)!=FALSE;
            if (FILE* report=fopen("inventory-layout-check.txt","wb")) {
                fprintf(report,"grid=%d release_without_press=%d close_clicked=%d scroll_end=%d scroll_released=%d\n",grid,stray,closed,scrollEnd,released); fprintf(report,"icon_samples=%d large_icon=%d multi_hit=%d picked=%d moved=%d\n",samples,largeIcon,multiHit,picked,moved); fprintf(report,"collision=%d freed=%d layout_restored=%d server_records_preserved=%d\n",collision,freed,layoutRestored,serverPreserved); fprintf(report,"placement_green=%d placement_red=%d\n",green,red); fprintf(report,"gold_unsigned=%d gold_updated=%d\n",goldUnsigned,goldUpdated); fclose(report);
            }
        }
        if ((groupPanel>=20 && groupPanel<=22) || groupPanel==25 || groupPanel==26 || groupPanel==27) {
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
            lstrcpyA(skills.m_pstMyMagic[0].stStdMagic.szMagicName,"\xbb\xf0\xc7\xf2\xca\xf5");
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
            if (groupPanel==27) {
                int icons=0,labels=0;
                for (int key=1;key<=12;++key) {
                    skills.m_pstMyMagic[key-1].bUseKey=skills.LegacyKey(key);
                    const int id=skills.m_pstMyMagic[key-1].stStdMagic.wMagicID&255;
                    icons+=g_xGameProc.m_xImage.m_xImageList[_IMAGE_MICON].NewSetIndex(999+id)!=FALSE;
                    labels+=ui.m_pxInterImg->NewSetIndex(1449+key)!=FALSE;
                }
                ui.OnKeyDown('B',0); const bool shown=ui.m_bShowKey!=FALSE;
                ui.OnKeyDown('B',1L<<30); const bool repeatIgnored=ui.m_bShowKey!=FALSE;
                ui.OnKeyDown('B',0); const bool hidden=ui.m_bShowKey==FALSE;
                ui.OnKeyDown('B',0);
                if (FILE* report=fopen("skill-shortcuts-check.txt","wb")) {
                    fprintf(report,"shown=%d repeat_ignored=%d hidden=%d icon_frames=%d label_frames=%d\n",shown,repeatIgnored,hidden,icons,labels); fclose(report);
                }
            }
            if (groupPanel==26) {
                mouse=POINT{390,90}; g_xGameProc.m_ptMousePos=mouse;
                skills.UpdatePointer(mouse,true); const bool hovered=skills.DescriptionAt()==1;
                const auto& rows=skills.DescriptionRows(1); bool wrapped=!rows.empty(),title=false;
                for (const auto& row:rows) { wrapped=wrapped && row.width<=165; title=title || row.title; }
                skills.UpdatePointer(mouse,false); const bool covered=skills.DescriptionAt()==0;
                skills.UpdatePointer(mouse,true); skills.m_skillScrollCaptured=true; const bool scrolling=skills.DescriptionAt()==0;
                skills.m_skillScrollCaptured=false; skills.OpenKeyMenu(mouse); const bool menu=skills.DescriptionAt()==0; skills.m_keyMenu={};
                CLoginMessages captions; const bool messages=captions.Load("./CmList.dat") && !captions.Get(1074).empty();
                if (FILE* report=fopen("skill-description-check.txt","wb")) {
                    fprintf(report,"hovered=%d rows=%u wrapped=%d title=%d covered=%d scrolling=%d menu=%d messages=%d\n",hovered,unsigned(rows.size()),wrapped,title,covered,scrolling,menu,messages); fclose(report);
                }
            }
            if (groupPanel==25) {
                const POINT skillPoint={380,80};
                ui.OnLButtonDown(skillPoint); ui.OnLButtonUp(skillPoint); ui.OnLButtonDoubleClick(skillPoint);
                const bool opened=!IsRectEmpty(&skills.m_keyMenu) && skills.m_keyMenu.left==380 && skills.m_keyMenu.top==120 && skills.m_keyMenuTarget==1;
                const bool noDrag=!ui.m_bMoveFocusedWnd;
                skills.m_pstMyMagic[1].bUseKey='C';
                ui.OnLButtonDown(POINT{520,220}); ui.OnLButtonUp(POINT{520,220});
                const bool lastKey=skills.GetMagicByKey('C')==&skills.m_pstMyMagic[0] && skills.m_pstMyMagic[1].bUseKey==0;
                const bool keptOpen=!IsRectEmpty(&skills.m_keyMenu);
                ui.OnLButtonDown(POINT{440,220}); ui.OnLButtonUp(POINT{440,220});
                const bool tenthKey=skills.GetMagicByKey('A')==&skills.m_pstMyMagic[0] && skills.GetMagicByKey('C')==NULL;
                bool keys=true; for (int key=1;key<=12;++key) keys=keys && skills.KeyNumber(skills.LegacyKey(key))==key;
                ui.OnLButtonDown(POINT{20,20}); ui.OnLButtonUp(POINT{20,20});
                const bool outsideClosed=IsRectEmpty(&skills.m_keyMenu)!=FALSE;
                ui.OnLButtonDoubleClick(skillPoint); ui.WindowActivate(_WND_ID_SKILL);
                const bool panelClosed=IsRectEmpty(&skills.m_keyMenu)!=FALSE; ui.WindowActivate(_WND_ID_SKILL);
                ui.OnLButtonDoubleClick(skillPoint);

                skills.m_keyMenu={}; skills.m_skillTab=7; skills.m_skillSelected[7]=9;
                ui.OnLButtonDoubleClick(POINT{500,410});
                const bool outsideHit=skills.m_keyMenu.bottom>skills.m_rcWnd.bottom && ui.GetWindowInMousePos(POINT{510,520})==_WND_ID_SKILL;
                ui.OnLButtonDown(POINT{510,520}); ui.OnLButtonUp(POINT{510,520});
                const auto fifth=skills.GetMagicByKey('5'); const bool overHud=fifth && fifth->stStdMagic.wMagicID==34 && !ui.m_bMoveFocusedWnd;
                skills.m_keyMenu={}; skills.m_skillTab=0; ui.OnLButtonDoubleClick(skillPoint);
                if (FILE* report=fopen("skill-key-menu-check.txt","wb")) {
                    fprintf(report,"opened=%d no_drag=%d key12_conflict=%d kept_open=%d key10_rebound=%d encodings=%d outside_close=%d panel_close=%d\n",
                        opened,noDrag,lastKey,keptOpen,tenthKey,keys,outsideClosed,panelClosed);
                    fprintf(report,"outside_window_hit=%d popup_over_hud=%d\n",outsideHit,overHud); fclose(report);
                }
                skills.m_pstMyMagic[1].bUseKey='2'; skills.m_pstMyMagic[2].bUseKey='3';
            }

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
        if (groupPanel==28 || groupPanel==29) {
            auto& ui=g_xGameProc.m_xInterface; auto& quest=ui.m_xQuestWnd;
            ui.OnKeyDown('D',0); const bool opened=quest.GetGameWndActive()!=FALSE;
            ui.OnKeyDown('D',1L<<30); const bool repeat=quest.GetGameWndActive()!=FALSE;
            ui.OnKeyDown('D',0); const bool hidden=!quest.GetGameWndActive(); ui.OnKeyDown('D',0);
            const bool stray=!quest.OnLButtonUp(POINT{275,115});
            bool selected=true,expanded=true,child=true,dragged=true,released=true,description=true;
            if (groupPanel==29) {
                for (int i=0;i<22;++i) { CQuestWnd::Entry entry; char number[32]; wsprintfA(number," %02d",i+1);
                    entry.title="\xc8\xce\xce\xf1"+std::string(number); entry.summary="\xc7\xeb\xc7\xb0\xcd\xf9\xb1\xc8\xc6\xe6\xb3\xc7\xd1\xb0\xd5\xd2\xc0\xcf\xb1\xf8\xa3\xac\xc1\xcb\xbd\xe2\xc3\xb0\xcf\xd5\xcb\xf9\xd0\xe8\xb5\xc4\xd7\xb0\xb1\xb8\xba\xcd\xbc\xbc\xc4\xdc\xa1\xa3\xcd\xea\xb3\xc9\xbd\xbb\xcc\xb8\xba\xf3\xa3\xac\xc7\xb0\xcd\xf9\xb3\xc7\xcd\xe2\xbb\xf7\xb0\xdc\xb5\xbe\xb2\xdd\xc8\xcb\xa3\xac\xd4\xd9\xb7\xb5\xbb\xd8\xc1\xec\xc8\xa1\xbd\xb1\xc0\xf8\xa1\xa3\xc8\xce\xce\xf1\xbd\xf8\xb6\xc8\xbb\xe1\xcf\xd4\xca\xbe\xd4\xda\xd5\xe2\xc0\xef\xa3\xac\xc7\xeb\xc1\xf4\xd2\xe2\xb8\xf7\xb8\xf6\xbd\xd7\xb6\xce\xb5\xc4\xcb\xb5\xc3\xf7\xa1\xa3"; entry.completed=(i%3)==2;
                    quest.entries.push_back(entry);
                }
                quest.entries[0].title="\xb3\xf5\xb3\xf6\xc3\xa9\xc2\xae";
                quest.entries[0].children.push_back({"\xb0\xdd\xb7\xc3\xb1\xc8\xc6\xe6\xb3\xc7\xb5\xc4\xc0\xcf\xb1\xf8","\xc7\xeb\xc7\xb0\xcd\xf9\xb1\xc8\xc6\xe6\xb3\xc7\xd1\xb0\xd5\xd2\xc0\xcf\xb1\xf8\xa3\xac\xc1\xcb\xbd\xe2\xc3\xb0\xcf\xd5\xcb\xf9\xd0\xe8\xb5\xc4\xd7\xb0\xb1\xb8\xba\xcd\xbc\xbc\xc4\xdc\xa1\xa3\xcd\xea\xb3\xc9\xbd\xbb\xcc\xb8\xba\xf3\xa3\xac\xc7\xb0\xcd\xf9\xb3\xc7\xcd\xe2\xbb\xf7\xb0\xdc\xb5\xbe\xb2\xdd\xc8\xcb\xa3\xac\xd4\xd9\xb7\xb5\xbb\xd8\xc1\xec\xc8\xa1\xbd\xb1\xc0\xf8\xa1\xa3\xc8\xce\xce\xf1\xbd\xf8\xb6\xc8\xbb\xe1\xcf\xd4\xca\xbe\xd4\xda\xd5\xe2\xc0\xef\xa3\xac\xc7\xeb\xc1\xf4\xd2\xe2\xb8\xf7\xb8\xf6\xbd\xd7\xb6\xce\xb5\xc4\xcb\xb5\xc3\xf7\xa1\xa3",false});
                quest.entries[0].children.push_back({"\xbb\xf7\xb0\xdc\xb5\xbe\xb2\xdd\xc8\xcb","\xc7\xeb\xc7\xb0\xcd\xf9\xb1\xc8\xc6\xe6\xb3\xc7\xd1\xb0\xd5\xd2\xc0\xcf\xb1\xf8\xa3\xac\xc1\xcb\xbd\xe2\xc3\xb0\xcf\xd5\xcb\xf9\xd0\xe8\xb5\xc4\xd7\xb0\xb1\xb8\xba\xcd\xbc\xbc\xc4\xdc\xa1\xa3\xcd\xea\xb3\xc9\xbd\xbb\xcc\xb8\xba\xf3\xa3\xac\xc7\xb0\xcd\xf9\xb3\xc7\xcd\xe2\xbb\xf7\xb0\xdc\xb5\xbe\xb2\xdd\xc8\xcb\xa3\xac\xd4\xd9\xb7\xb5\xbb\xd8\xc1\xec\xc8\xa1\xbd\xb1\xc0\xf8\xa1\xa3\xc8\xce\xce\xf1\xbd\xf8\xb6\xc8\xbb\xe1\xcf\xd4\xca\xbe\xd4\xda\xd5\xe2\xc0\xef\xa3\xac\xc7\xeb\xc1\xf4\xd2\xe2\xb8\xf7\xb8\xf6\xbd\xd7\xb6\xce\xb5\xc4\xcb\xb5\xc3\xf7\xa1\xa3",true});
                ui.OnLButtonDown(POINT{80,95}); ui.OnLButtonUp(POINT{80,95}); selected=quest.selected==0 && !ui.m_bMoveFocusedWnd;
                ui.OnLButtonDown(POINT{275,82}); ui.OnLButtonUp(POINT{275,82}); expanded=quest.expanded==0 && quest.EntryAt(1)==-1 && quest.EntryAt(3)==1;
                ui.OnLButtonDown(POINT{85,110}); ui.OnLButtonUp(POINT{85,110}); child=quest.descriptionChild==0 && quest.selected==-1 && !quest.Description().empty();
                for (int i=0;i<8;++i) quest.Wheel(1,POINT{100,350}); description=quest.descriptionFirst==3 && quest.first==0;
                quest.descriptionFirst=0;
                ui.OnLButtonDown(POINT{278,165}); ui.OnMouseMove(POINT{780,580}); dragged=quest.scrollCaptured && quest.first==quest.LastRow();
                ui.OnLButtonUp(POINT{780,580}); released=!quest.scrollCaptured; quest.Scroll(-100);
            }
            ui.OnLButtonDown(POINT{275,115}); ui.OnLButtonUp(POINT{780,580}); const bool outside=quest.pressed==-1 && quest.GetGameWndActive();
            ui.OnLButtonDown(POINT{275,115}); ui.OnLButtonUp(POINT{275,115}); const bool closed=!quest.GetGameWndActive(); ui.OnKeyDown('D',0);
            const int savedFirst=quest.first; quest.Wheel(1,POINT{700,100}); const bool outsideWheel=quest.first==savedFirst;
            ui.OnLButtonDown(POINT{310,50}); ui.OnMouseMove(POINT{320,60}); ui.OnLButtonUp(POINT{320,60});
            const bool moved=quest.m_rcWnd.left==10 && quest.m_rcWnd.top==10; quest.m_rcWnd={0,0,340,440};
            if (FILE* report=fopen("quest-layout-check.txt","wb")) {
                fprintf(report,"opened=%d repeat=%d hidden=%d stray=%d selected=%d expanded=%d child=%d description_scroll=%d dragged=%d released=%d outside_release=%d closed=%d\n",
                    opened,repeat,hidden,stray,selected,expanded,child,description,dragged,released,outside,closed);
                fprintf(report,"outside_wheel=%d window_drag=%d\n",outsideWheel,moved); fclose(report);
            }
        }
        if (groupPanel==30 || groupPanel==31) {
            auto& ui=g_xGameProc.m_xInterface; auto& mount=ui.m_xHorseWnd;
            ui.OnKeyDown('S',0); const bool opened=mount.GetGameWndActive()!=FALSE;
            ui.OnKeyDown('S',1L<<30); const bool repeat=mount.GetGameWndActive()!=FALSE;
            ui.OnKeyDown('S',0); const bool hidden=!mount.GetGameWndActive(); ui.OnKeyDown('S',0);
            const bool buttons=mount.ButtonAt(POINT{240,285})==0 && mount.ButtonAt(POINT{30,245})==1 && mount.ButtonAt(POINT{90,245})==2 && mount.ButtonAt(POINT{145,245})==3 && mount.ButtonAt(POINT{205,245})==4;
            const bool stray=!mount.OnLButtonUp(POINT{240,285});
            const bool onFoot=mount.Command(1)==4106 && mount.Command(2)==0 && mount.Command(3)==4104 && mount.Command(4)==4105;
            mount.shape=1; const bool riding=mount.Command(1)==0 && mount.Command(2)==4105; mount.shape=0;
            mount.UpdatePointer(POINT{30,245},true); const bool hint=mount.HintAt()==4100;
            ui.OnLButtonDown(POINT{30,245}); const bool pressedHint=mount.HintAt()==0 && !ui.m_bMoveFocusedWnd;
            ui.OnLButtonUp(POINT{780,580}); const bool released=mount.pressed==-1;
            mount.UpdatePointer(POINT{30,245},false); const bool covered=mount.HintAt()==0;
            ui.OnLButtonDown(POINT{100,60}); ui.OnMouseMove(POINT{110,70}); ui.OnLButtonUp(POINT{110,70});
            const bool moved=mount.m_rcWnd.left==10 && mount.m_rcWnd.top==10; mount.m_rcWnd={0,0,276,322};
            ui.OnLButtonDown(POINT{240,285}); ui.OnLButtonUp(POINT{240,285}); const bool closed=!mount.GetGameWndActive(); ui.OnKeyDown('S',0);
            if (groupPanel==31) {
                mouse=POINT{30,245}; g_xGameProc.m_ptMousePos=mouse; mount.UpdatePointer(mouse,true);
                for (int i=0;i<24;++i) ui.ShowButtonHint(mouse);
            }
            if (FILE* report=fopen("mount-layout-check.txt","wb")) {
                fprintf(report,"opened=%d repeat=%d hidden=%d buttons=%d stray=%d on_foot=%d riding=%d hint=%d pressed_hint=%d outside_release=%d covered=%d window_drag=%d closed=%d\n",
                    opened,repeat,hidden,buttons,stray,onFoot,riding,hint,pressedHint,released,covered,moved,closed); fclose(report);
            }
        }
        if (groupPanel>=60 && groupPanel<=63) {
            auto& ui=g_xGameProc.m_xInterface; auto& picker=ui.m_xPortraitWnd; picker.directory="./portrait-preview";
            const POINT entry={550,573}; ui.OnLButtonDown(entry); const bool entryPressed=ui.m_portraitEntryPressed;
            ui.OnLButtonUp(POINT{790,10}); const bool entryOutside=!picker.m_bActive && !ui.m_portraitEntryPressed;
            ui.OnLButtonUp(entry); const bool opened=picker.m_bActive && picker.names.size()==17 && picker.first==0 && picker.selected==-1;
            const auto find=[&](const char* name) { for(int i=0;i<int(picker.names.size());++i) if(picker.names[i]==name) return i; return -1; };
            const int valid=find("portrait01.jpg"); const bool files=valid>=0 && find("oversized.jpg")==-1;
            picker.first=valid; ui.OnLButtonDown(POINT{40,35}); ui.OnLButtonUp(POINT{40,35});
            const bool selected=picker.selected==valid && picker.HasPreview() && !ui.m_bMoveFocusedWnd;
            ui.OnMouseMove(POINT{40,60}); ui.OnMouseMove(POINT{310,340}); const bool hover=picker.hovered==1;
            const bool gap=CPortraitWnd::RowAt(POINT{40,48})==-1 && CPortraitWnd::RowAt(POINT{264,35})==-1;
            picker.first=0; g_xGameProc.m_ptMousePos=POINT{100,60}; ui.OnScrollUp(); const bool wheel=picker.first==1;
            picker.first=0; picker.scrollTick=0; ui.OnLButtonDown(POINT{278,32}); const bool captured=picker.scrolling;
            ui.OnMouseMove(POINT{780,580}); ui.OnLButtonUp(POINT{780,580}); const bool bottom=picker.first==int(picker.names.size()) && !picker.scrolling;
            picker.Wheel(1); const bool bottomWheel=picker.first==int(picker.names.size())-1;
            const auto captureUpload=[&]() {
                SOCKET receiver=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP),sender=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
                sockaddr_in address={};address.sin_family=AF_INET;address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
                bind(receiver,(sockaddr*)&address,sizeof(address));int addressLength=sizeof(address);getsockname(receiver,(sockaddr*)&address,&addressLength);
                connect(sender,(sockaddr*)&address,sizeof(address));DWORD timeout=500;setsockopt(receiver,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));
                const SOCKET saved=g_xClientSocket.m_sockClient;g_xClientSocket.m_sockClient=sender;
                const bool sent=picker.SendRequest();g_xClientSocket.m_sockClient=saved;
                char wire[8192]={};const int received=recv(receiver,wire,sizeof(wire)-1,0);closesocket(sender);closesocket(receiver);
                if(!sent || received<19 || wire[0]!='#' || wire[received-1]!='!') return false;
                wire[received-1]=0;_TDEFAULTMESSAGE packet={};fnDecodeMessage(&packet,wire+2);
                BYTE decoded[4096]={};const int length=fnDecode6BitBuf(wire+2+DEFBLOCKSIZE,(char*)decoded,sizeof(decoded));
                return packet.wIdent==1151 && !packet.nRecog && !packet.wParam && !packet.wTag && !packet.wSeries && length==int(picker.requested.size()) &&
                    (length==0 || memcmp(decoded,picker.requested.data(),length)==0) && !picker.requestPending && !picker.SendRequest();
            };
            picker.first=valid; picker.selected=valid; picker.requestPending=false;
            ui.OnLButtonUp(POINT{65,350}); const bool stray=!picker.requestPending;
            ui.OnLButtonDown(POINT{65,350}); ui.OnLButtonUp(POINT{780,580}); const bool outside=!picker.requestPending && picker.pressed==-1;
            picker.lastUpload=0; picker.Request(1,1000); const bool upload=picker.requestPending && picker.requested.size()>2 && picker.requested.size()<=4095 && picker.requested[0]==0xff;
            const bool uploadWire=captureUpload();
            picker.requestPending=false; picker.Request(1,1500); const bool cooldown=!picker.requestPending;
            ui.OnLButtonDown(POINT{185,350}); ui.OnLButtonUp(POINT{185,350}); const bool reset=picker.requestPending && picker.requested.empty() && picker.lastUpload==1000;
            const bool resetWire=captureUpload();
            if(FILE* report=fopen("portrait-upload-wire-check.txt","wb")) {fprintf(report,"jpeg=%d reset=%d\n",uploadWire,resetWire);fclose(report);}
            picker.requestPending=false; picker.selected=find("zz-invalid.jpg"); picker.Request(1,2000); const bool invalid=ui.m_xMsgBox.IsActive() && !picker.requestPending; ui.m_xMsgBox.HideMessageBox();
            picker.names.push_back("oversized.jpg"); picker.selected=int(picker.names.size())-1; picker.Request(1,2000); const bool size=ui.m_xMsgBox.IsActive() && !picker.requestPending; ui.m_xMsgBox.HideMessageBox(); picker.names.pop_back();
            ui.OnLButtonDown(POINT{545,370}); ui.OnLButtonUp(POINT{545,370}); const bool closed=!picker.m_bActive && picker.names.empty() && !picker.HasPreview() && picker.lastUpload==1000;
            ui.OnLButtonUp(entry); ui.OnKeyDown(VK_ESCAPE,0); const bool escape=!picker.m_bActive;
            ui.m_chatExpanded=true; ui.OnLButtonUp(entry); const bool expanded=!picker.m_bActive; ui.m_chatExpanded=false;
            if(groupPanel==60) picker.directory="./portrait-preview-empty";
            ui.WindowActivate(_WND_ID_PORTRAIT);
            if(groupPanel!=60) { picker.first=find("portrait01.jpg"); picker.Pointer(0,POINT{40,35}); picker.Pointer(2,POINT{40,35}); if(groupPanel==62) picker.first=10; }
            mouse=POINT{40,35}; g_xGameProc.m_ptMousePos=mouse;
            POINT hintAnchor={};
            const bool uploadHint=picker.HintAt(POINT{65,350},hintAnchor)==4700;
            const bool resetHint=picker.HintAt(POINT{185,350},hintAnchor)==4701;
            const bool closeHint=picker.HintAt(POINT{545,370},hintAnchor)==1052;
            picker.pressed=1; const bool pressedHint=picker.HintAt(POINT{65,350},hintAnchor)==0; picker.pressed=-1;
            const bool entryHint=picker.HintAt(entry,hintAnchor)==4715;
            ui.m_chatExpanded=true; const bool expandedHint=picker.HintAt(entry,hintAnchor)==0; ui.m_chatExpanded=false;
            ui.ShowNotice(4716); const bool modalHint=picker.HintAt(entry,hintAnchor)==0 && picker.HintAt(POINT{65,350},hintAnchor)==0; ui.m_xMsgBox.HideMessageBox();
            if(FILE* report=fopen("portrait-hint-check.txt","wb")) { fprintf(report,"upload=%d reset=%d close=%d pressed=%d entry=%d expanded=%d modal=%d\n",uploadHint,resetHint,closeHint,pressedHint,entryHint,expandedHint,modalHint); fclose(report); }
            if(groupPanel==63) { mouse=POINT{65,350}; g_xGameProc.m_ptMousePos=mouse; for(int i=0;i<24;++i) ui.ShowButtonHint(mouse); }
            if(FILE* report=fopen("portrait-picker-check.txt","wb")) {
                fprintf(report,"entry_pressed=%d entry_outside=%d opened=%d files=%d selected=%d hover=%d gap=%d wheel=%d captured=%d bottom=%d bottom_wheel=%d stray=%d outside=%d upload=%d cooldown=%d reset=%d invalid=%d size=%d closed=%d escape=%d expanded=%d\n",entryPressed,entryOutside,opened,files,selected,hover,gap,wheel,captured,bottom,bottomWheel,stray,outside,upload,cooldown,reset,invalid,size,closed,escape,expanded); fclose(report);
            }
        }
        if (groupPanel==59) {
            auto& ui=g_xGameProc.m_xInterface; auto& npc=ui.m_xNPCWnd; auto& shop=ui.m_xStoreWnd; auto& bag=ui.m_xInventoryWnd;
            std::string script="窗口层级测试。\\<向商人询问详细服务内容以及仓库功能/@next>\\";
            for(int i=0;i<30;++i) script+="这是NPC对话第"+std::to_string(i+1)+"行。\\";
            npc.SetScript(77,script.c_str()); ui.WindowActivate(_WND_ID_NPCCHAT);
            const char* goods="金创药/2/100/20/0/0/0/魔法药/2/180/20/15/0/0/太阳水/2/300/20/20/0/0/";
            shop.SetGoods(77,goods,3); ui.WindowActivate(_WND_ID_STORE);
            ui.WindowActivate(_WND_ID_INVENTORY); bag.m_rcWnd={150,0,432,466};
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            const bool activation=ui.IsTopWindow(_WND_ID_INVENTORY);
            const bool npcHit=ui.GetWindowInMousePos(POINT{175,60})==_WND_ID_NPCCHAT;
            const bool shopHit=ui.GetWindowInMousePos(POINT{175,300})==_WND_ID_STORE;
            const bool bagHit=ui.GetWindowInMousePos(POINT{420,100})==_WND_ID_INVENTORY;
            npc.lastAction=0; ui.OnLButtonDown(POINT{175,60}); ui.OnLButtonUp(POINT{175,60});
            const bool npcClick=npc.lastAction && !ui.m_bMoveFocusedWnd;
            ui.DeleteWindowToList(_WND_ID_INVENTORY); ui.AddWindowToList(_WND_ID_INVENTORY);
            ui.OnLButtonDown(POINT{175,300}); ui.OnLButtonUp(POINT{175,300}); const bool shopClick=shop.highlighted==1;
            ui.DeleteWindowToList(_WND_ID_INVENTORY); ui.AddWindowToList(_WND_ID_INVENTORY);
            g_xGameProc.m_ptMousePos=POINT{175,300}; ui.OnScrollUp(); const bool shopWheel=shop.first==1 && npc.first==0;
            g_xGameProc.m_ptMousePos=POINT{175,60}; ui.OnScrollUp(); const bool npcWheel=npc.first==1 && bag.ScrollRow()==0;
            npc.first=0; shop.first=0; shop.fraction=0;
            ui.WindowActivate(_WND_ID_STORE); const bool revealed=ui.GetWindowInMousePos(POINT{175,300})==_WND_ID_NPCCHAT;
            shop.SetGoods(77,goods,3); ui.WindowActivate(_WND_ID_STORE);
            ui.DeleteWindowToList(_WND_ID_INVENTORY); ui.AddWindowToList(_WND_ID_INVENTORY);
            ui.ShowNotice(1437); npc.lastAction=0; ui.OnLButtonDown(POINT{175,60}); ui.OnLButtonUp(POINT{175,60});
            const bool modal=!npc.lastAction && ui.m_xMsgBox.IsActive(); ui.m_xMsgBox.HideMessageBox();
            const auto render=[&]() {
                auto* surface=g_xMainWnd.GetBackBuffer(); DDBLTFX fill={}; fill.dwSize=sizeof(fill); fill.dwFillColor=0x0841;
                surface->Blt(NULL,NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT,&fill); ui.RenderInterface(0,POINT{790,590});
                DDSURFACEDESC2 desc={}; desc.dwSize=sizeof(desc); std::vector<WORD> pixels(432*449);
                if (SUCCEEDED(surface->Lock(NULL,&desc,DDLOCK_WAIT,NULL))) {
                    for(int y=0;y<449;++y) memcpy(&pixels[y*432],(BYTE*)desc.lpSurface+y*desc.lPitch,432*2);
                    surface->Unlock(NULL);
                }
                return pixels;
            };
            const auto before=render();
            ui.DeleteWindowToList(_WND_ID_NPCCHAT); ui.AddWindowToList(_WND_ID_NPCCHAT);
            ui.DeleteWindowToList(_WND_ID_STORE); ui.AddWindowToList(_WND_ID_STORE);
            const bool pixels=before==render();
            ui.DeleteWindowToList(_WND_ID_INVENTORY); ui.AddWindowToList(_WND_ID_INVENTORY);
            if(FILE* report=fopen("npc-shop-layering-check.txt","wb")) {
                fprintf(report,"activation=%d npc_hit=%d shop_hit=%d bag_hit=%d npc_click=%d shop_click=%d shop_wheel=%d npc_wheel=%d revealed=%d modal=%d pixels=%d\n",activation,npcHit,shopHit,bagHit,npcClick,shopClick,shopWheel,npcWheel,revealed,modal,pixels); fclose(report);
            }
        }
        if (groupPanel==58) {
            auto& ui=g_xGameProc.m_xInterface; auto& npc=ui.m_xNPCWnd; const HWND edit=g_xChatEditBox.GetSafehWnd();
            npc.SetScript(77,"仓库密码验证。\\<请输入仓库密码/@@*6_enter_stgpasswd>"); ui.WindowActivate(_WND_ID_NPCCHAT);
            const auto open=[&](const char* command) { npc.promptActor=npc.m_nNpcId; npc.promptCommand=command; npc.prompt="请输入仓库密码"; ui.OpenNameDialog(7); };
            const auto limit=[&](const char* command,int expected) { npc.promptCommand=command; return npc.PromptLimit()==expected; };
            const bool rules=limit("@@*6_enter_stgpasswd",6) && limit("@@*8_Input_stgpasswd",8) && limit("@@*12_Change_stgpasswd",12) &&
                limit("@@*4_confirm_passwd",4) && limit("@@*confirm_passwd",10) && limit("@@*0000001_enter_stgpasswd",10) &&
                limit("@@*0_enter_stgpasswd",90) && limit("@@*bad_enter_stgpasswd",90) && limit("@@*+8_enter_stgpasswd",90) &&
                limit("@@*8_other",90) && limit("@@8_enter_stgpasswd",90);
            open("@@*4_enter_stgpasswd"); const bool secret=SendMessage(edit,EM_GETPASSWORDCHAR,0,0)=='*' && SendMessage(edit,EM_GETLIMITTEXT,0,0)==4;
            SetWindowTextA(edit,"abc"); SendMessage(edit,EM_SETSEL,3,3); SendMessageA(edit,EM_REPLACESEL,TRUE,(LPARAM)"中");
            char value[64]={}; GetWindowTextA(edit,value,sizeof(value)); const bool dbcs=!strcmp(value,"abc");
            SetWindowTextA(edit,"ab"); SendMessage(edit,EM_SETSEL,1,1); SendMessageA(edit,EM_REPLACESEL,TRUE,(LPARAM)"中文"); GetWindowTextA(edit,value,sizeof(value));
            const bool suffix=!strcmp(value,"a中b");
            ui.OnLButtonDown(POINT{490,315}); ui.OnLButtonUp(POINT{490,315});
            const bool cancelled=npc.promptCommand.empty() && !npc.promptActor && !SendMessage(edit,EM_GETPASSWORDCHAR,0,0) && SendMessage(edit,EM_GETLIMITTEXT,0,0)==90;
            open("@@*6_enter_stgpasswd"); npc.SetScript(78,"Different actor"); const bool changed=!npc.SubmitPrompt("1234") && npc.promptCommand.empty() && !npc.promptActor; ui.FinishNotice(CLICK_NO,false);
            open("@@answer"); const bool empty=npc.SubmitPrompt("") && npc.promptCommand.empty(); ui.FinishNotice(CLICK_NO,false);
            open("@@*300_enter_stgpasswd"); const std::string longAnswer(300,'a'); SetWindowTextA(edit,longAnswer.c_str());
            const bool longInput=GetWindowTextLengthA(edit)==300;
            SendMessage(edit,WM_CHAR,VK_RETURN,0); const bool keyboard=!ui.m_xMsgBox.IsActive() && npc.promptCommand.empty() && !IsWindowVisible(edit) && SendMessage(edit,EM_GETLIMITTEXT,0,0)==90;
            open("@@ordinary"); const bool ordinary=!SendMessage(edit,EM_GETPASSWORDCHAR,0,0) && SendMessage(edit,EM_GETLIMITTEXT,0,0)==90; ui.FinishNotice(CLICK_NO,false);
            npc.SetScript(77,"仓库密码验证。\\<请输入仓库密码/@@*6_enter_stgpasswd>"); open("@@*6_enter_stgpasswd"); SetWindowTextA(edit,"123456");
            if(FILE* report=fopen("npc-password-check.txt","wb")) {
                fprintf(report,"rules=%d secret=%d dbcs=%d suffix=%d cancelled=%d changed_actor=%d empty_answer=%d long_input=%d keyboard=%d ordinary=%d\n",rules,secret,dbcs,suffix,cancelled,changed,empty,longInput,keyboard,ordinary); fclose(report);
            }
        }
        if (groupPanel==56 || groupPanel==57) {
            auto& ui=g_xGameProc.m_xInterface; auto& bag=ui.m_xInventoryWnd;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            CLIENTITEMRCD record={}; record.nMakeIndex=1001; record.stStdItem.wLooks=0; strcpy(record.stStdItem.szName,"金创药");
            CItem item; item.SetItem(record,0); const bool legacy=item.m_count==1 && item.m_stackMode==0;
            item.m_count=30; item.m_stackMode=2; bag.AddInvenItem(item,0,TRUE);
            const auto quick=CQuickWnd::FromItem(item); const bool reference=quick.count==30 && quick.stackMode==2;
            bag.SetCommonFromInven(0,&ui.m_stCommonItem); const bool held=ui.m_stCommonItem.xItem.m_count==30 && ui.m_stCommonItem.xItem.m_stackMode==2;
            bag.SetInvenFromCommon(0,&ui.m_stCommonItem); const bool restored=bag.m_stInventoryItem[0].xItem.m_count==30;
            const auto open=[&](int mode) { bag.OpenService(77,mode); bag.SelectServiceItem(0); if(mode==2) bag.QuoteService(125,true); bag.ConfirmService(); };
            open(2); const bool sell=ui.m_xMsgBox.IsActive() && ui.m_nIdxMsgBox==10 && bag.service.stack && bag.service.phase==CInventoryWnd::ServicePhase::Quoted;
            ui.OnLButtonDown(POINT{490,315}); ui.OnLButtonUp(POINT{490,315}); const bool cancel=!ui.m_xMsgBox.IsActive() && bag.service.phase==CInventoryWnd::ServicePhase::Quoted;
            for(const char* value:{"","0","-1","31","65536","999999999999999999999999"}) bag.SubmitServiceQuantity(value);
            const bool invalid=bag.service.phase==CInventoryWnd::ServicePhase::Quoted;
            bag.SubmitServiceQuantity("10tail"); const bool prefix=bag.service.phase==CInventoryWnd::ServicePhase::Paying && bag.m_stInventoryItem[0].xItem.m_count==30;
            open(3); const bool deposit=ui.m_xMsgBox.IsActive() && ui.m_nIdxMsgBox==11;
            SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"*"); ui.OnLButtonDown(POINT{290,315}); ui.OnLButtonUp(POINT{290,315});
            const bool all=!ui.m_xMsgBox.IsActive() && bag.service.phase==CInventoryWnd::ServicePhase::Paying && bag.m_stInventoryItem[0].xItem.m_count==30;
            open(2); bag.DeleteInvenItem(0); SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"10"); ui.OnLButtonDown(POINT{290,315}); ui.OnLButtonUp(POINT{290,315});
            const bool missing=bag.service.phase==CInventoryWnd::ServicePhase::Choosing && !ui.m_xMsgBox.IsActive(); bag.AddInvenItem(item,0,TRUE);
            open(2); bag.OpenService(77,3); const bool replaced=!ui.m_xMsgBox.IsActive() && !IsWindowVisible(g_xChatEditBox.GetSafehWnd());
            open(3); ui.WindowActivate(_WND_ID_INVENTORY); const bool closed=!ui.m_xMsgBox.IsActive() && !bag.service.actor;
            open(2); ui.m_xStoreWnd.SetStorage(77,{}); const bool storage=!ui.m_xMsgBox.IsActive() && !bag.service.actor;
            ui.m_xStoreWnd.Reset();
            ui.m_xNPCWnd.SetScript(77,"请选择背包中的物品。\\可输入数量，或用 * 表示全部。\\<返回/@main>"); ui.WindowActivate(_WND_ID_NPCCHAT);
            open(groupPanel==56?2:3); SetWindowTextA(g_xChatEditBox.GetSafehWnd(),groupPanel==56?"10":"*");
            if(FILE* report=fopen("service-quantity-check.txt","wb")) {
                fprintf(report,"legacy=%d reference=%d held=%d restored=%d sell=%d cancel=%d invalid=%d prefix=%d deposit=%d all=%d missing=%d replaced=%d closed=%d storage=%d\n",legacy,reference,held,restored,sell,cancel,invalid,prefix,deposit,all,missing,replaced,closed,storage); fclose(report);
            }
        }
        if (groupPanel>=53 && groupPanel<=55) {
            auto& ui=g_xGameProc.m_xInterface; auto& bag=ui.m_xInventoryWnd;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            CLIENTITEMRCD record={}; record.nMakeIndex=1001; record.stStdItem.wLooks=0; strcpy(record.stStdItem.szName,"金创药");
            CItem item; item.SetItem(record,0); bag.AddInvenItem(item,0,TRUE);
            record.nMakeIndex=1002; record.stStdItem.wLooks=15; strcpy(record.stStdItem.szName,"魔法药"); item.SetItem(record,1); bag.AddInvenItem(item,1,TRUE);
            ui.m_xNPCWnd.SetScript(77,"请选择背包中的物品。\\再点击下方服务按钮。\\<返回/@main>"); ui.WindowActivate(_WND_ID_NPCCHAT);
            bag.OpenService(77,1); const bool opened=bag.m_bActive && bag.m_rcWnd.left==519 && bag.m_rcWnd.top==0 && ui.IsTopWindow(_WND_ID_INVENTORY);
            ui.OnLButtonDown(POINT{540,70}); ui.OnLButtonUp(POINT{540,70});
            const bool selected=bag.service.item==1001 && bag.service.phase==CInventoryWnd::ServicePhase::Quoting && bag.m_stInventoryItem[0].bSetted && !ui.m_stCommonItem.bSetted;
            ui.OnLButtonDown(POINT{580,70}); ui.OnLButtonUp(POINT{580,70}); const bool pending=bag.service.item==1001;
            bag.QuoteService(250,true); const bool mismatch=bag.service.price==0;
            bag.QuoteService(0,false); const bool unavailable=bag.service.phase==CInventoryWnd::ServicePhase::Unavailable;
            bag.QuoteService(250,false); const bool quote=bag.service.phase==CInventoryWnd::ServicePhase::Quoted && bag.service.price==250;
            ui.OnLButtonUp(POINT{722,407}); const bool stray=bag.service.phase==CInventoryWnd::ServicePhase::Quoted;
            ui.OnLButtonDown(POINT{722,407}); const bool pressed=bag.service.pressed && !ui.m_bMoveFocusedWnd;
            ui.OnLButtonUp(POINT{10,580}); const bool released=!bag.service.pressed && bag.service.phase==CInventoryWnd::ServicePhase::Quoted;
            ui.OnLButtonDown(POINT{722,407}); ui.OnLButtonUp(POINT{722,407}); const bool paying=bag.service.phase==CInventoryWnd::ServicePhase::Paying;
            ui.OnLButtonDown(POINT{580,70}); ui.OnLButtonUp(POINT{580,70}); bag.QuoteService(500,false);
            const bool blocked=bag.service.item==1001 && bag.service.price==250 && bag.service.phase==CInventoryWnd::ServicePhase::Paying;
            bag.OpenService(77,2); bag.SelectServiceItem(0); bag.QuoteService(125,true); const bool selling=bag.service.phase==CInventoryWnd::ServicePhase::Quoted;
            bag.DeleteInvenItem(0); bag.ConfirmService(); const bool missing=bag.service.phase==CInventoryWnd::ServicePhase::Choosing && bag.service.price==0;
            record.nMakeIndex=1001; record.stStdItem.wLooks=0; strcpy(record.stStdItem.szName,"金创药"); item.SetItem(record,0); bag.AddInvenItem(item,0,TRUE);
            bag.OpenService(77,3); bag.SelectServiceItem(0); const bool deposit=bag.service.phase==CInventoryWnd::ServicePhase::Quoted && bag.service.price==0;
            bag.ConfirmService(); const bool depositPending=bag.service.phase==CInventoryWnd::ServicePhase::Paying && bag.m_stInventoryItem[0].bSetted;
            ui.WindowActivate(_WND_ID_INVENTORY); const bool reset=bag.service.actor==0;
            bag.OpenService(77,1); g_xGameProc.CloseNPCWindow(); const bool closed=!bag.m_bActive && !bag.service.actor;
            ui.m_xNPCWnd.SetScript(77,"请选择背包中的物品。\\再点击下方服务按钮。\\<返回/@main>"); ui.WindowActivate(_WND_ID_NPCCHAT);
            bag.OpenService(77,groupPanel-52); bag.SelectServiceItem(0);
            if(groupPanel!=55) bag.QuoteService(groupPanel==53?250:125,groupPanel==54);
            if(FILE* report=fopen("merchant-service-check.txt","wb")) {
                fprintf(report,"opened=%d selected=%d pending=%d mismatch=%d unavailable=%d quote=%d stray=%d pressed=%d released=%d paying=%d blocked=%d selling=%d missing=%d deposit=%d deposit_pending=%d reset=%d closed=%d\n",opened,selected,pending,mismatch,unavailable,quote,stray,pressed,released,paying,blocked,selling,missing,deposit,depositPending,reset,closed); fclose(report);
            }
        }
        if (groupPanel>=50 && groupPanel<=52) {
            auto& ui=g_xGameProc.m_xInterface; auto& shop=ui.m_xStoreWnd;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            ui.m_xNPCWnd.SetScript(77,"仓库保管员。\\请选择要取回的物品。\\<返回/@main>"); ui.WindowActivate(_WND_ID_NPCCHAT);
            std::vector<CStoreWnd::Offer> items;
            for(int i=0;i<25;++i) items.push_back({"仓库药剂",i%2?15:0,0,DWORD(100+i),i?1:30,i?0:2});
            shop.SetStorage(77,items); ui.WindowActivate(_WND_ID_STORE);
            const bool bounds=shop.storage && shop.m_rcWnd.left==-292 && shop.m_rcWnd.right==196 && shop.m_rcWnd.top==234;
            const RECT last=shop.OfferCell(11); const bool cells=last.left==136 && last.top==342 && last.right==173 && last.bottom==379;
            ui.OnLButtonUp(POINT{117,393}); const bool stray=shop.page==0;
            ui.OnLButtonDown(POINT{117,393}); ui.OnLButtonUp(POINT{780,580}); const bool released=shop.page==0 && shop.pressed==-1;
            ui.OnLButtonDown(POINT{117,393}); ui.OnLButtonUp(POINT{117,393}); const bool next=shop.page==1 && shop.offerIndex==-1;
            ui.OnLButtonDown(POINT{25,270}); ui.OnLButtonUp(POINT{25,270}); const bool selected=shop.offerIndex==12;
            ui.OnLButtonDown(POINT{117,393}); ui.OnLButtonUp(POINT{117,393});
            ui.OnLButtonDown(POINT{117,393}); ui.OnLButtonUp(POINT{117,393}); const bool end=shop.page==2;
            ui.OnLButtonDown(POINT{65,270}); ui.OnLButtonUp(POINT{65,270}); const bool empty=shop.offerIndex==-1;
            shop.SetStorage(77,items); shop.Withdraw();
            const bool quantity=ui.m_xMsgBox.IsActive() && ui.m_nIdxMsgBox==9 && IsWindowVisible(g_xChatEditBox.GetSafehWnd());
            ui.OnLButtonDown(POINT{490,315}); ui.OnLButtonUp(POINT{490,315}); const bool cancel=!ui.m_xMsgBox.IsActive() && !shop.storagePending;
            shop.SubmitWithdrawal("31"); shop.SubmitWithdrawal("0"); const bool invalid=!shop.storagePending;
            shop.SubmitWithdrawal("*"); const bool all=shop.storagePending==100 && shop.offers.size()==25;
            ui.OnLButtonDown(POINT{117,393}); ui.OnLButtonUp(POINT{117,393}); const bool pending=shop.page==0;
            shop.SetStorage(99,items); const bool refreshed=shop.storagePending==100 && shop.actor==77;
            _TDEFAULTMESSAGE reply; fnMakeDefMessage(&reply,SM_STORAGE_WITHDRAW_OK,0,10,20,0); shop.Receive(reply,"");
            const bool partial=!shop.storagePending && shop.offers[0].count==10;
            shop.SubmitWithdrawal("10tail"); fnMakeDefMessage(&reply,SM_STORAGE_WITHDRAW_OK,0,0,10,0); shop.Receive(reply,"");
            const bool removed=shop.offers.size()==24 && !shop.storagePending && shop.offerIndex==-1;
            shop.SetStorage(77,items); shop.Withdraw(); g_xGameProc.CloseNPCWindow();
            const bool closed=!shop.m_bActive && !ui.m_xMsgBox.IsActive() && !IsWindowVisible(g_xChatEditBox.GetSafehWnd());
            BYTE record[100]={}; strcpy((char*)record,"StoredItem"); record[36]=15; record[62]=123; record[61]=2; record[71]=30;
            char encoded[256]={}; fnEncode6BitBuf(record,encoded,100,sizeof(encoded)); const auto wire=std::string(encoded)+"/";
            fnMakeDefMessage(&reply,SM_STORAGE_LIST,77,0,0,1); shop.Receive(reply,wire.c_str());
            const bool parsed=shop.m_bActive && shop.storage && shop.offers.size()==1 && shop.offers[0].count==30 && shop.offers[0].stackMode==2;
            const bool rejected=!shop.SetStorageRecords(77,"invalid/",1) && shop.offers.size()==1;
            shop.SetStorage(77,{}); ui.OnLButtonDoubleClick(POINT{25,270});
            const bool emptyList=shop.offerIndex==-1 && !shop.storagePending && shop.page==0;
            ui.OnLButtonDown(POINT{162,390}); ui.OnLButtonUp(POINT{162,390}); const bool closeButton=!shop.m_bActive && !shop.storage;
            shop.SetStorage(77,items); ui.WindowActivate(_WND_ID_STORE);
            ui.OnLButtonDoubleClick(POINT{65,270}); const bool single=shop.storagePending==101 && !ui.m_xMsgBox.IsActive();
            fnMakeDefMessage(&reply,SM_STORAGE_WITHDRAW_FAIL,0,0,0,0); shop.Receive(reply,"");
            const bool failed=!shop.storagePending && shop.offers.size()==25 && ui.m_xMsgBox.IsActive();
            ui.m_xMsgBox.HideMessageBox();
            ui.m_xNPCWnd.SetScript(77,"仓库保管员。\\请选择要取回的物品。\\<返回/@main>"); ui.WindowActivate(_WND_ID_NPCCHAT);
            shop.SetStorage(77,items); ui.DeleteWindowToList(_WND_ID_STORE); ui.AddWindowToList(_WND_ID_STORE);
            if(groupPanel==51) { ui.OnLButtonDown(POINT{117,393}); ui.OnLButtonUp(POINT{117,393}); }
            if(groupPanel==52) { shop.Withdraw(); SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"*"); }
            if(FILE* report=fopen("storage-layout-check.txt","wb")) {
                fprintf(report,"bounds=%d cells=%d stray=%d released=%d next=%d selected=%d end=%d empty=%d quantity=%d cancel=%d invalid=%d all=%d pending=%d refreshed=%d partial=%d removed=%d closed=%d parsed=%d rejected=%d empty_list=%d close_button=%d single=%d failed=%d\n",bounds,cells,stray,released,next,selected,end,empty,quantity,cancel,invalid,all,pending,refreshed,partial,removed,closed,parsed,rejected,emptyList,closeButton,single,failed); fclose(report);
            }
        }
        if (groupPanel>=46 && groupPanel<=49) {
            auto& ui=g_xGameProc.m_xInterface; auto& shop=ui.m_xStoreWnd; auto& npc=ui.m_xNPCWnd;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            npc.SetScript(77,"欢迎光临商店。\\请选择需要的商品。\\<返回/@main>"); ui.WindowActivate(_WND_ID_NPCCHAT);
            const char* list="金创药/2/100/20/0/0/0/强效金创药/2/180/30/1/0/0/魔法药/1/850/1/15/0/8/太阳水/1/500/1/20/0/12/技能书/1/1500/1/304/0/1/布衣/0/200/1/30/0/0/";
            const bool parsed=shop.SetGoods(77,list,6) && shop.goods.size()==6 && shop.goods[2].requirement==8;
            const bool invalid=!shop.SetGoods(77,"broken/1/x/",1) && shop.goods.size()==6 && !shop.SetGoods(77,"",256);
            ui.WindowActivate(_WND_ID_STORE);
            const bool bounds=shop.m_rcWnd.left==0 && shop.m_rcWnd.top==225 && shop.m_rcWnd.right==292;
            ui.OnLButtonUp(POINT{70,300}); const bool stray=shop.highlighted==0;
            ui.OnLButtonDown(POINT{70,300}); ui.OnLButtonUp(POINT{780,580}); const bool outside=shop.pressed==-1 && shop.highlighted==0;
            ui.OnLButtonDown(POINT{70,300}); ui.OnLButtonUp(POINT{70,300}); const bool selected=shop.highlighted==1;
            ui.OnLButtonDown(POINT{58,300}); ui.OnLButtonUp(POINT{58,300}); const bool gap=shop.highlighted==1 && !ui.m_bMoveFocusedWnd;
            g_xGameProc.m_ptMousePos=POINT{263,260}; ui.OnScrollUp(); const bool wheel=shop.first==1; for(int i=0;i<20;++i) ui.OnScrollUp(); const bool end=shop.first==5;
            for(int i=0;i<20;++i) ui.OnScrollDown();
            ui.OnLButtonDown(POINT{263,260}); const bool captured=shop.scrollCaptured;
            ui.OnMouseMove(POINT{780,580}); ui.OnLButtonUp(POINT{780,580}); const bool drag=shop.first==5 && !shop.scrollCaptured;
            shop.SetGoods(77,list,6); ui.OnLButtonDoubleClick(POINT{70,345});
            const bool requested=shop.selected==2 && shop.bulk==-1 && !shop.details;
            std::vector<CStoreWnd::Offer> offers; for (int i=0;i<12;++i) offers.push_back({"魔法药",15,850+i*10,DWORD(100+i)});
            BYTE record[100]={}; strcpy((char*)record,"WireItem"); record[36]=15; record[62]=123; record[68]=250;
            char encoded[256]={}; fnEncode6BitBuf(record,encoded,100,sizeof(encoded));
            const std::string wire=std::string(encoded)+"/";
            const bool wireParsed=shop.SetDetailRecords(wire.c_str(),1,20) && shop.offers[0].id==123 && shop.offers[0].price==250;
            const bool wireRejected=!shop.SetDetailRecords("broken/",1,0) && shop.offers.size()==1;
            std::string records; for(int i=0;i<12;++i) records+=wire;
            char encodedList[4096]={}; fnEncode6BitBuf((BYTE*)records.data(),encodedList,int(records.size()),sizeof(encodedList));
            _TDEFAULTMESSAGE reply; fnMakeDefMessage(&reply,SM_SENDDETAILGOODSLIST,77,12,40,0); shop.Receive(reply,encodedList);
            const bool detailReply=shop.offers.size()==12 && shop.page==4 && strlen(encodedList)>1040;
            const auto oldGold=g_xGameProc.m_xMyHero.m_nGlod;
            fnMakeDefMessage(&reply,SM_BUYITEM_SUCCESS,12345,0,0,0); shop.Receive(reply,"");
            const bool purchaseReply=shop.offers.size()==11 && g_xGameProc.m_xMyHero.m_nGlod==12345;
            g_xGameProc.m_xMyHero.m_nGlod=oldGold;
            shop.SetOffers(offers,20); const bool details=shop.details && shop.page==2 && shop.offerIndex==0 && shop.m_rcWnd.right==488;
            ui.OnLButtonDown(POINT{355,270}); ui.OnLButtonUp(POINT{355,270}); const bool cell=shop.offerIndex==1;
            ui.OnLButtonDown(POINT{410,392}); ui.OnLButtonUp(POINT{410,392}); const bool pending=shop.loading && shop.page==3;
            ui.OnLButtonDown(POINT{318,270}); ui.OnLButtonUp(POINT{318,270}); const bool blocked=shop.offerIndex==1;
            shop.SetOffers(offers,30); ui.OnLButtonDown(POINT{455,390}); ui.OnLButtonUp(POINT{455,390});
            const bool detailsClosed=!shop.details && shop.m_rcWnd.right==292 && shop.goods.size()==6;
            ui.OnLButtonDoubleClick(POINT{70,255}); const bool bulk=shop.bulk==0 && shop.m_rcWnd.right==516;
            ui.OnLButtonDown(POINT{375,290}); ui.OnLButtonUp(POINT{375,290});
            const bool quantity=ui.m_xMsgBox.IsActive() && ui.m_nIdxMsgBox==8 && IsWindowVisible(g_xChatEditBox.GetSafehWnd());
            ui.OnLButtonDown(POINT{490,315}); ui.OnLButtonUp(POINT{490,315}); const bool cancelled=!ui.m_xMsgBox.IsActive();
            ui.OnLButtonDown(POINT{375,290}); ui.OnLButtonUp(POINT{375,290}); g_xGameProc.CloseNPCWindow();
            const bool lifecycle=!shop.m_bActive && !npc.m_bActive && !ui.m_xMsgBox.IsActive() && !IsWindowVisible(g_xChatEditBox.GetSafehWnd());
            npc.SetScript(77,"欢迎光临商店。\\请选择需要的商品。\\<返回/@main>"); ui.WindowActivate(_WND_ID_NPCCHAT);
            shop.SetGoods(77,list,6); ui.WindowActivate(_WND_ID_STORE);
            if(groupPanel==47) { shop.highlighted=2; shop.OpenCategory(); shop.SetOffers(offers,0); }
            if(groupPanel>=48) { shop.OpenCategory(); if(groupPanel==49) { ui.OpenNameDialog(8); SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"10"); } }
            if(FILE* report=fopen("shop-layout-check.txt","wb")) {
                fprintf(report,"parsed=%d invalid=%d bounds=%d stray=%d outside=%d selected=%d gap=%d wheel=%d scroll_end=%d captured=%d drag=%d requested=%d details=%d cell=%d pending=%d blocked=%d details_closed=%d bulk=%d quantity=%d cancelled=%d lifecycle=%d wire_parsed=%d wire_rejected=%d detail_reply=%d purchase_reply=%d\n",parsed,invalid,bounds,stray,outside,selected,gap,wheel,end,captured,drag,requested,details,cell,pending,blocked,detailsClosed,bulk,quantity,cancelled,lifecycle,wireParsed,wireRejected,detailReply,purchaseReply); fclose(report);
            }
        }
        if (groupPanel>=42 && groupPanel<=45) {
            auto& ui=g_xGameProc.m_xInterface; auto& npc=ui.m_xNPCWnd;
            const std::string shortScript="欢迎来到比奇城！\\这里可以购买补给，也可以了解城中的消息。\\{FCOLOR/15}请选择需要的服务：\\<购买药品/@buy>  <询问任务/@quest>\\{FCOLOR/12}<建立行会/@@buildguildnow>\\旅途愉快。";
            const auto open=[&](const std::string& script) { npc.SetScript(123,script.c_str()); if (!npc.m_bActive) ui.WindowActivate(_WND_ID_NPCCHAT); };
            open("末尾纯文本"); const bool trailing=npc.rows.size()==1 && npc.rows[0].text=="末尾纯文本";
            open("前缀<未闭合"); const bool malformed=npc.rows.size()==1 && npc.rows[0].text=="前缀";
            open("{FCOLOR/1}红色{FCOLOR/12}白色"); const bool colors=npc.rows.size()==2 && npc.rows[0].color==0xff && npc.rows[1].color==0xffffff;
            open("{NOTCLOSE/1}不能关闭"); npc.SetScript(123,"仍然不能关闭"); const bool retained=!npc.canClose;
            npc.SetScript(124,"可以关闭"); const bool reset=npc.canClose;
            open(shortScript); const bool bounds=npc.Height()==224 && !npc.scrollEnabled;
            int option=-1; POINT link={};
            for (const auto& row:npc.rows) if (row.option>=0) { const RECT box=npc.TextBounds(row); link={box.left+1,box.top+1}; option=row.option; break; }
            const bool hit=npc.OptionAt(link)==option && npc.OptionAt(POINT{380,10})==-1;
            ui.OnLButtonDown(link); const bool pressed=npc.pressed==option && !ui.m_bMoveFocusedWnd;
            ui.OnLButtonUp(link); const DWORD sent=npc.lastAction; ui.OnLButtonUp(link); const bool cooldown=sent && npc.lastAction==sent;
            npc.lastAction=0; ui.OnLButtonUp(link); const bool releaseOnly=npc.lastAction!=0;
            const POINT close={350,npc.Height()-38}; ui.OnLButtonUp(close); const bool stray=npc.m_bActive;
            ui.OnLButtonDown(close); ui.OnLButtonUp(POINT{780,580}); const bool outside=npc.m_bActive && !npc.closePressed;
            ui.OnLButtonDown(close); ui.OnLButtonUp(close); const bool closed=!npc.m_bActive && npc.canClose;
            open("{NOTCLOSE/1}不可关闭\\<继续/@next>"); ui.OnLButtonDown(close); ui.OnLButtonUp(close); const bool locked=npc.m_bActive && !npc.canClose;
            g_xGameProc.CloseNPCWindow();
            std::string longScript; for (int i=0;i<32;++i) longScript+="第"+std::to_string(i+1)+"行：原生NPC长对话滚动检查。\\";
            open(longScript); const bool longBounds=npc.Height()==464 && npc.scrollEnabled && npc.scrollLimit>0;
            ui.OnLButtonDown(POINT{188,npc.Height()-26}); ui.OnLButtonUp(POINT{188,npc.Height()-26}); const bool arrow=npc.first==1;
            for (int i=0;i<100;++i) npc.OnScrollUp(); const bool end=npc.first==npc.scrollLimit;
            open("<请输入行会名称/@@buildguildnow>"); npc.lastAction=0; const RECT promptBox=npc.TextBounds(npc.rows[0]);
            ui.OnLButtonUp(POINT{promptBox.left+1,promptBox.top+1});
            const bool prompt=ui.m_xMsgBox.IsActive() && ui.m_nIdxMsgBox==7 && IsWindowVisible(g_xChatEditBox.GetSafehWnd());
            ui.OnLButtonDown(POINT{490,315}); ui.OnLButtonUp(POINT{490,315}); const bool cancelled=!ui.m_xMsgBox.IsActive();
            open("<Password/@@*storage>"); npc.lastAction=0;
            const RECT passwordBox=npc.TextBounds(npc.rows[0]); ui.OnLButtonUp(POINT{passwordBox.left+1,passwordBox.top+1});
            const int actor=npc.m_nNpcId; npc.SetScript(actor+1,"Other actor");
            const bool promptActor=npc.promptActor==actor && npc.m_nNpcId!=actor;
            const bool password=SendMessage(g_xChatEditBox.GetSafehWnd(),EM_GETPASSWORDCHAR,0,0)=='*';
            g_xGameProc.CloseNPCWindow();
            const bool promptClosed=!ui.m_xMsgBox.IsActive() && !IsWindowVisible(g_xChatEditBox.GetSafehWnd()) && !SendMessage(g_xChatEditBox.GetSafehWnd(),EM_GETPASSWORDCHAR,0,0);
            if (FILE* report=fopen("npc-dialogue-check.txt","wb")) {
                fprintf(report,"trailing=%d malformed=%d colors=%d retained_notclose=%d new_actor_reset=%d short_bounds=%d hit=%d pressed=%d cooldown=%d release_only=%d stray=%d outside_release=%d closed=%d locked=%d long_bounds=%d arrow=%d scroll_end=%d prompt=%d cancelled=%d prompt_actor=%d password=%d prompt_closed=%d\n",
                    trailing,malformed,colors,retained,reset,bounds,hit,pressed,cooldown,releaseOnly,stray,outside,closed,locked,longBounds,arrow,end,prompt,cancelled,promptActor,password,promptClosed); fclose(report);
            }
            g_xGameProc.CloseNPCWindow();
            if (groupPanel==43) { open(longScript); npc.OnScrollUp(); npc.OnScrollUp(); npc.downLit=true; }
            else if (groupPanel==45) open("{NOTCLOSE/1}请先完成当前对话。\\{FCOLOR/15}此时关闭按钮仍显示，但不可关闭窗口。\\<继续对话/@next>");
            else {
                open(shortScript); mouse=link; g_xGameProc.m_ptMousePos=mouse;
                if (groupPanel==44) {
                    for (const auto& row:npc.rows) if (row.command=="@@buildguildnow") {
                        const RECT box=npc.TextBounds(row); npc.lastAction=0; ui.OnLButtonUp(POINT{box.left+1,box.top+1}); break;
                    }
                    SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"测试行会");
                }
            }
        }
        if (groupPanel==40 || groupPanel==41) {
            auto& ui=g_xGameProc.m_xInterface; auto& guild=ui.m_xGuildWnd; auto& editor=ui.m_xGuildEditor;
            guild.m_name="GuildPreview"; ui.WindowActivate(_WND_ID_GUILD);
            const POINT launch={groupPanel==40?395:545,412};
            ui.OnLButtonDown(launch); ui.OnLButtonUp(launch); const bool denied=!editor.m_bActive && ui.m_xMsgBox.IsActive(); ui.m_xMsgBox.HideMessageBox();
            editor.Open(false); const bool emptyDenied=!editor.m_bActive && ui.m_xMsgBox.IsActive(); ui.m_xMsgBox.HideMessageBox();
            guild.m_manager=true; guild.m_pages[1]={"会长：测试角色","成员：原生客户端"};
            guild.m_editable[0]={"欢迎加入行会！","请遵守行会约定，互相帮助。","","今晚八点集合，一起挑战新的冒险。"};
            guild.m_editable[1]={"#1","会长","测试角色","#2","成员","原生客户端"};
            ui.OnLButtonDown(launch); ui.OnLButtonUp(launch);
            const bool opened=editor.m_bActive && IsWindowVisible(editor.edit) && editor.announcement==(groupPanel==40);
            char initial[4001]={}; GetWindowTextA(editor.edit,initial,sizeof(initial));
            const bool multiline=strstr(initial,"\r\n")!=NULL;
            ui.OnKeyDown('Z',0); const int first=guild.m_first; ui.OnScrollUp();
            const bool modal=!ui.m_xQuickWnd.m_bActive && guild.m_first==first;
            ui.OnLButtonDown(POINT{115,115}); ui.OnMouseMove(POINT{125,105}); ui.OnLButtonUp(POINT{125,105});
            const bool moved=editor.m_rcWnd.left==117 && editor.m_rcWnd.top==100 && !editor.dragging;
            editor.m_rcWnd={107,110,691,362}; editor.Open(groupPanel==40);
            const POINT close={660,326}; ui.OnLButtonUp(close); const bool stray=editor.m_bActive;
            ui.OnLButtonDown(close); ui.OnLButtonUp(POINT{780,580}); const bool outside=editor.m_bActive && editor.pressed==-1;
            ui.OnLButtonDown(close); ui.OnLButtonUp(close); const bool cancelled=!editor.m_bActive && !IsWindowVisible(editor.edit);
            editor.Open(groupPanel==40);
            const std::string huge(4100,'A'); SetWindowTextA(editor.edit,huge.c_str()); editor.Changed();
            const bool limit=GetWindowTextLengthA(editor.edit)==4000;
            const bool dbcs=CGuildEditor::AcceptEdit("",std::string(3999,'A')+"中")==std::string(3999,'A');
            const std::string chinese=std::string(3999,'A')+"中"; SetWindowTextA(editor.edit,""); editor.Changed(); SendMessage(editor.edit,EM_SETSEL,0,-1); SendMessageA(editor.edit,EM_REPLACESEL,FALSE,(LPARAM)chinese.c_str());
            const bool nativeDbcs=GetWindowTextLengthA(editor.edit)==3999;
            SetWindowTextA(editor.edit,"A"); editor.Changed(); SendMessage(editor.edit,EM_SETSEL,1,1); SendMessage(editor.edit,WM_CHAR,VK_RETURN,0);
            char entered[8]={}; GetWindowTextA(editor.edit,entered,sizeof(entered)); const bool enter=!strcmp(entered,"A\r\n");
            std::string longText; for (int row=0;row<60;++row) longText+="Line "+std::to_string(row)+"\r\n";
            SetWindowTextA(editor.edit,longText.c_str()); editor.Changed(); SendMessage(editor.edit,EM_LINESCROLL,0,25);
            const bool scrolled=SendMessage(editor.edit,EM_GETFIRSTVISIBLELINE,0,0)>0;
            editor.Show();
            const RECT display=g_xMainWnd.m_rcWindow; g_xMainWnd.m_rcWindow.right=display.left+(display.right-display.left)*3/4;
            editor.Show(); RECT scaled; GetClientRect(editor.edit,&scaled);
            bool resized=scaled.right==MulDiv(468,g_xMainWnd.m_rcWindow.right-display.left,800),brushMatches=true;
            HDC source; auto* surface=g_xMainWnd.GetBackBuffer();
            if (SUCCEEDED(surface->GetDC(&source))) {
                HDC actual=CreateCompatibleDC(source),expected=CreateCompatibleDC(source);
                HBITMAP a=CreateCompatibleBitmap(source,scaled.right,scaled.bottom),b=CreateCompatibleBitmap(source,scaled.right,scaled.bottom);
                auto oldA=SelectObject(actual,a),oldB=SelectObject(expected,b);
                FillRect(actual,&scaled,editor.EditColor(actual));
                StretchBlt(expected,0,0,scaled.right,scaled.bottom,source,editor.m_rcWnd.left+46,editor.m_rcWnd.top+54,468,170,SRCCOPY);
                for (int y=0;y<scaled.bottom;y+=13) for (int x=0;x<scaled.right;x+=11) brushMatches=brushMatches && GetPixel(actual,x,y)==GetPixel(expected,x,y);
                SelectObject(actual,oldA); SelectObject(expected,oldB); DeleteObject(a); DeleteObject(b); DeleteDC(actual); DeleteDC(expected); surface->ReleaseDC(source);
            } else brushMatches=false;
            resized=resized && brushMatches; g_xMainWnd.m_rcWindow=display; editor.Show();
            const bool payload=CGuildEditor::Payload("a\r\n\r\nb\r\n")=="a\r \rb\r" && CGuildEditor::Payload(std::string(4100,'A')).size()==3999;
            SetWindowTextA(editor.edit,"Preview\r\n"); ui.OnLButtonDown(POINT{573,127}); ui.OnLButtonUp(POINT{573,127});
            const bool saved=!editor.m_bActive && !IsWindowVisible(editor.edit);
            editor.Open(groupPanel==40);
            if (FILE* report=fopen("guild-editor-check.txt","wb")) {
                fprintf(report,"denied=%d empty_members=%d opened=%d multiline=%d modal=%d moved=%d stray=%d outside_release=%d cancelled=%d limit=%d dbcs=%d native_dbcs=%d enter=%d long_scroll=%d horizontal_resize=%d payload=%d saved=%d\n",
                    denied,emptyDenied,opened,multiline,modal,moved,stray,outside,cancelled,limit,dbcs,nativeDbcs,enter,scrolled,resized,payload,saved); fclose(report);
            }
        }
        if (groupPanel==39) {
            auto& ui=g_xGameProc.m_xInterface; auto& bag=ui.m_xInventoryWnd; auto& quick=ui.m_xQuickWnd; auto& held=ui.m_stCommonItem;
            ui.WindowActivate(_WND_ID_INVENTORY); ui.OnKeyDown('Z',0);
            auto& icons=g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY]; icons.NewLoad("./Data/Inventory.wil");
            CItem items[2]; int found=0;
            for (int icon=0;icon<100 && found<2;++icon) if (icons.NewSetIndex(icon) && icons.m_lpstNewCurrWilImageInfo->shWidth>0 && icons.m_lpstNewCurrWilImageInfo->shWidth<=38 && icons.m_lpstNewCurrWilImageInfo->shHeight<=38) {
                CLIENTITEMRCD record={}; record.nMakeIndex=2000+found; record.stStdItem.wLooks=icon; record.stStdItem.bStdMode=found?3:0;
                wsprintfA(record.stStdItem.szName,"QuickPreview%d",found); items[found].SetItem(record,found); bag.AddInvenItem(items[found],found,TRUE); ++found;
            }
            const auto count=[&]() { int total=0; for (const auto& entry:bag.m_stInventoryItem) if (entry.bSetted) ++total; return total; };
            bag.SetCommonFromInven(0,&held);
            const bool bound=quick.Release(0)==1068 && count()==2 && !held.bSetted && quick.slots[0].item.m_stItemInfo.nMakeIndex==2000;
            const bool picked=quick.Release(0)==0 && quick.heldSlot==0 && held.bIsBeltItem && count()==2 && !quick.slots[0].present;
            const bool moved=quick.Release(1)==0 && !held.bSetted && quick.heldSlot==-1 && quick.slots[1].present && count()==2;
            bag.SetCommonFromInven(0,&held);
            const bool duplicate=quick.Release(2)==1067 && held.bSetted && count()==1 && !quick.slots[2].present;
            const bool same=quick.Release(1)==1069 && held.xItem.m_stItemInfo.nMakeIndex==2000;
            bag.SetInvenFromCommon(0,&held);
            bag.SetCommonFromInven(1,&held);
            const bool swapped=quick.Release(1)==0 && quick.slots[1].item.m_stItemInfo.nMakeIndex==2001 && quick.heldSlot==1 && held.xItem.m_stItemInfo.nMakeIndex==2000 && count()==2;
            quick.Release(2); quick.Release(2);
            ui.OnLButtonDown(POINT{550,80}); ui.OnLButtonUp(POINT{550,80});
            const bool discarded=quick.heldSlot==-1 && !held.bSetted && !quick.slots[2].present && count()==2;
            bag.SetCommonFromInven(0,&held);
            held.bWaitResponse=TRUE; const bool pending=quick.Release(0)==-1 && held.bSetted; held.bWaitResponse=FALSE;
            held.bIsEquipItem=TRUE; const bool equipment=quick.Release(0)==-1; held.bIsEquipItem=FALSE;
            held.xItem.m_stItemInfo.stStdItem.bStdMode=5; const bool type=quick.Release(0)==-1; held.xItem.m_stItemInfo.stStdItem.bStdMode=0;
            ui.OnLButtonDown(POINT{95,425}); ui.OnLButtonUp(POINT{95,425});
            const bool routed=quick.slots[0].present && !held.bSetted && count()==2;
            for (int i=2;i<_MAX_INVEN_ITEM;++i) bag.AddInvenItem(items[0],i,TRUE);
            held.bSetted=TRUE; held.xItem=items[0]; lstrcpyA(held.xItem.m_stItemInfo.stStdItem.szName,"NoRoom");
            const bool full=quick.Release(3)==-1 && held.bSetted && count()==_MAX_INVEN_ITEM && !quick.slots[3].present;
            ZeroMemory(&held,sizeof(held)); for (int i=2;i<_MAX_INVEN_ITEM;++i) bag.DeleteInvenItem(i);
            const bool saved=bag.SaveLayoutFile("./preview-quick-layout.itms");
            bool disk=false;
            if (FILE* file=fopen("./preview-quick-layout.itms","rb")) {
                std::vector<BYTE> bytes(163392); const int size=int(fread(bytes.data(),1,bytes.size(),file)); fclose(file);
                const int quickOffset=46*3120+1200;
                disk=size==163392 && bytes[quickOffset]==1 && bytes[quickOffset+3112]==1 && bytes[quickOffset+2*3112]==0;
                if (FILE* bad=fopen("./preview-quick-invalid.itms","wb")) {
                    bytes[46*3120]=47; bytes[46*3120+1]=0; fwrite(bytes.data(),1,bytes.size(),bad); fclose(bad);
                }
            }
            bag.DeleteAllInvenItem();
            for (int i=0;i<2;++i) {
                items[i].m_stItemInfo.nMakeIndex=9000+i; items[i].m_stItemInfo.stStdItem.nPrice=76543;
                bag.AddInvenItem(items[i],i,TRUE);
            }
            CItem last=items[0]; last.m_stItemInfo.nMakeIndex=9002; last.m_stItemInfo.nDura=77; bag.AddInvenItem(last,2,TRUE);
            for (auto& slot:quick.slots) slot=CQuickWnd::Slot{};
            const bool restored=saved && bag.LoadLayoutFile("./preview-quick-layout.itms") && quick.slots[0].present && quick.slots[1].present;
            const bool current=quick.slots[0].item.m_stItemInfo.nMakeIndex==9002 && quick.slots[0].item.m_stItemInfo.nDura==77 &&
                quick.slots[1].item.m_stItemInfo.nMakeIndex==9001 && quick.slots[1].item.m_stItemInfo.stStdItem.nPrice==76543 && count()==3;
            const int anchor=bag.ItemAnchor(2);
            const bool invalid=!bag.LoadLayoutFile("./preview-quick-invalid.itms") && quick.slots[0].item.m_stItemInfo.nMakeIndex==9002 && bag.ItemAnchor(2)==anchor;
            bag.DeleteInvenItem(1); quick.slots[1]=CQuickWnd::Slot{};
            const bool missing=bag.LoadLayoutFile("./preview-quick-layout.itms") && !quick.slots[1].present && count()==2;
            bag.AddInvenItem(items[1],1,TRUE); bag.LoadLayoutFile("./preview-quick-layout.itms");
            quick.Release(0); const bool heldSaved=bag.SaveLayoutFile("./preview-quick-held.itms"); bool heldExcluded=false;
            if (FILE* file=fopen("./preview-quick-held.itms","rb")) {
                fseek(file,46*3120+1200,SEEK_SET); heldExcluded=heldSaved && fgetc(file)==0; fclose(file);
            }
            quick.Release(0);
            if (FILE* report=fopen("quick-persistence-check.txt","wb")) {
                fprintf(report,"saved=%d disk_layout=%d restored=%d current_records_last_match=%d invalid_unchanged=%d missing_ignored=%d held_excluded=%d\n",
                    saved,disk,restored,current,invalid,missing,heldExcluded); fclose(report);
            }
            if (FILE* report=fopen("quick-binding-check.txt","wb")) {
                fprintf(report,"bound=%d picked_reference=%d moved=%d duplicate=%d same=%d swapped=%d discarded_reference=%d pending=%d equipment=%d type=%d routed=%d full_rollback=%d\n",
                    bound,picked,moved,duplicate,same,swapped,discarded,pending,equipment,type,routed,full); fclose(report);
            }
        }
        if (groupPanel==37 || groupPanel==38) {
            auto& ui=g_xGameProc.m_xInterface; auto& quick=ui.m_xQuickWnd;
            ui.OnKeyDown('Z',0); const bool opened=quick.GetGameWndActive()!=FALSE;
            ui.OnKeyDown('Z',1L<<30); const bool repeat=quick.GetGameWndActive()!=FALSE;
            ui.OnKeyDown('Z',0); const bool hidden=!quick.GetGameWndActive(); ui.OnKeyDown('Z',0);
            bool slots=true; for (int i=0;i<6;++i) slots=slots && quick.SlotAt(POINT{90+42*i,419})==i && quick.SlotAt(POINT{127+42*i,456})==i;
            const bool gap=quick.SlotAt(POINT{128,420})==-1 && quick.SlotAt(POINT{90,457})==-1;
            const POINT close={339,447}; quick.OnLButtonUp(close); const bool stray=quick.GetGameWndActive()!=FALSE;
            ui.OnLButtonDown(close); ui.OnLButtonUp(POINT{790,10}); const bool outside=quick.GetGameWndActive() && !quick.closePressed;
            ui.OnLButtonDown(POINT{80,415}); ui.OnMouseMove(POINT{100,395}); ui.OnLButtonUp(POINT{100,395});
            const bool moved=quick.m_rcWnd.left==93 && quick.m_rcWnd.top==393 && !quick.backgroundPressed && !ui.m_bMoveFocusedWnd;
            quick.m_rcWnd={73,413,353,461};
            ui.OnLButtonDown(POINT{95,425}); ui.OnMouseMove(POINT{105,435}); ui.OnLButtonUp(POINT{105,435});
            const bool noDrag=quick.m_rcWnd.left==73 && quick.m_rcWnd.top==413;
            ui.OnLButtonDown(close); ui.OnLButtonUp(close); const bool closed=!quick.GetGameWndActive(); ui.OnKeyDown('Z',0);
            auto& trade=ui.m_xExchangeWnd; ui.WindowActivate(_WND_ID_EXCHANGE);
            trade.offers[0][0].present=true; trade.offers[0][0].makeIndex=123; trade.offers[0][0].name="Preview";
            trade.returnSlot=0; trade.returnPending=true;
            ui.OnLButtonDown(POINT{95,425}); ui.OnLButtonUp(POINT{95,425}); const bool pending=trade.returnSlot==0;
            trade.returnPending=false;
            ui.OnLButtonDown(POINT{95,425}); ui.OnLButtonUp(POINT{95,425});
            const bool returned=trade.returnSlot==-1 && trade.offers[0][0].present && !quick.slots[0].present;
            ui.WindowActivate(_WND_ID_EXCHANGE);
            const int modes[]={0,5,6,10,11,15,19,20,21,22,23,24,26,99};
            const int samples[]={108,111,111,112,112,116,115,115,115,113,113,114,114,118};
            bool sounds=true; for (int i=0;i<14;++i) sounds=sounds && CMirSound::ItemSoundSample(modes[i],0,"Preview")==samples[i];
            CLoginMessages captions; captions.Load("./CmList.dat");
            sounds=sounds && CMirSound::ItemSoundSample(24,0,captions.Get(2641).c_str())==117 && CMirSound::ItemSoundSample(26,0,captions.Get(2642).c_str())==117 && CMirSound::ItemSoundSample(31,105,"")==108 && CMirSound::ItemSoundSample(31,106,"")==118;
            if (groupPanel==38) {
                auto& icons=g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY]; icons.NewLoad("./Data/Inventory.wil");
                int slot=0; for (int icon=0;icon<100 && slot<6;++icon) if (icons.NewSetIndex(icon) && icons.m_lpstNewCurrWilImageInfo->shWidth>0 && icons.m_lpstNewCurrWilImageInfo->shWidth<=38 && icons.m_lpstNewCurrWilImageInfo->shHeight<=38) {
                    quick.slots[slot]={true,icon,slot?slot*11:1,2}; ++slot;
                }
            }
            if (FILE* report=fopen("quick-panel-check.txt","wb")) {
                fprintf(report,"opened=%d repeat_ignored=%d hidden=%d slots=%d gaps=%d stray=%d outside_release=%d moved=%d slot_no_drag=%d closed=%d pending_kept=%d trade_returned=%d sounds=%d\n",
                    opened,repeat,hidden,slots,gap,stray,outside,moved,noDrag,closed,pending,returned,sounds); fclose(report);
            }
        }
        if (groupPanel>=32 && groupPanel<=36) {
            auto& ui=g_xGameProc.m_xInterface; auto& trade=ui.m_xExchangeWnd;
            lstrcpyA(hero.m_szName,"MirPreview"); trade.partner="TradePartner";
            ui.WindowActivate(_WND_ID_EXCHANGE);
            auto& icons=g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY]; icons.NewLoad("./Data/Inventory.wil");
            if (groupPanel!=32) {
                for (int side=0;side<2;++side) {
                    auto& item=trade.offers[side][0]; item.present=true; item.icon=75; item.width=item.height=2; item.count=1;
                    for (int icon=0;icon<100;++icon) if (icons.NewSetIndex(icon) && icons.m_lpstNewCurrWilImageInfo->shWidth>0 && icons.m_lpstNewCurrWilImageInfo->shWidth<=38 && icons.m_lpstNewCurrWilImageInfo->shHeight<=38) {
                        trade.offers[side][3]={true,icon,1,1,side?9:25}; break;
                    }
                }
                trade.gold[0]=123456789; trade.gold[1]=987654;
            }
            const bool amount=trade.Amount(0)=="0" && trade.Amount(123456789)=="123,456,789" && trade.Amount(0xffffffffu)=="4,294,967,295";
            ui.OnLButtonDown(POINT{211,65}); ui.OnMouseMove(POINT{780,580});
            const bool left=trade.first[0]==33 && trade.first[1]==0; ui.OnLButtonUp(POINT{780,580});
            const bool released=trade.captured==-1; trade.first[0]=0;
            ui.OnLButtonDown(POINT{462,65}); ui.OnMouseMove(POINT{780,580});
            const bool right=trade.first[1]==33 && trade.first[0]==0; ui.OnLButtonUp(POINT{780,580}); trade.first[1]=0;
            const POINT confirm={240,290}; trade.OnLButtonUp(confirm); const bool stray=!trade.confirmed;
            bool pending=true;
            for (int kind=0;kind<3;++kind) {
                trade.goldPending=kind==0; trade.pendingSlot=kind==1?0:-1; trade.returnSlot=kind==2?0:-1;
                ui.OnLButtonDown(confirm); ui.OnLButtonUp(confirm); pending=pending && !trade.confirmed;
            }
            trade.goldPending=false; trade.pendingSlot=trade.returnSlot=-1;
            ui.OnLButtonDown(confirm); ui.OnLButtonUp(POINT{780,580}); const bool cancelled=!trade.confirmed && !trade.confirmPressed;
            ui.OnLButtonDown(POINT{30,70}); ui.OnMouseMove(POINT{40,80}); ui.OnLButtonUp(POINT{40,80});
            const bool bodyNoDrag=trade.m_rcWnd.left==0 && trade.m_rcWnd.top==0;
            ui.OnLButtonDown(POINT{480,20}); ui.OnMouseMove(POINT{490,30}); ui.OnLButtonUp(POINT{490,30});
            const bool moved=trade.m_rcWnd.left==10 && trade.m_rcWnd.top==10; trade.m_rcWnd={0,0,494,330};
            ui.OnLButtonDown(confirm); ui.OnLButtonUp(confirm); const bool confirmed=trade.confirmed;
            trade.confirmed=groupPanel==34;
            if (groupPanel==36) {
                auto& item=trade.offers[0][0]; item.makeIndex=123; item.name="PreviewItem";
                const bool footprint=trade.OfferAt(POINT{15,57})==0 && trade.OfferAt(POINT{88,130})==0 && trade.OfferAt(POINT{90,132})==-1;
                ui.OnLButtonDown(POINT{70,110}); ui.OnLButtonUp(POINT{70,110});
                const bool picked=trade.returnSlot==0 && !ui.m_stCommonItem.bSetted;
                ui.OnLButtonDown(confirm); ui.OnLButtonUp(confirm); const bool confirmBlocked=!trade.confirmed;
                ui.OnLButtonDown(POINT{280,80}); ui.OnLButtonUp(POINT{280,80}); const bool peerIgnored=trade.returnSlot==0;
                ui.OnLButtonDown(POINT{170,200}); ui.OnLButtonUp(POINT{170,200}); const bool cleared=trade.returnSlot==-1;
                trade.first[0]=1; const bool scrolled=trade.OfferAt(POINT{30,70})==0; trade.first[0]=0;
                item.makeIndex=0; const bool noIdentifier=trade.OfferAt(POINT{30,70})==-1; item.makeIndex=123;
                trade.gold[0]=0; hero.m_nGlod=100000;
                ui.OnLButtonDown(confirm); ui.OnLButtonUp(POINT{80,290}); const bool captured=!ui.m_xMsgBox.IsActive();
                ui.OnLButtonDown(POINT{30,70}); ui.OnLButtonUp(POINT{30,70});
                ui.WindowActivate(_WND_ID_INVENTORY);
                ui.OnLButtonDown(POINT{550,80}); ui.OnLButtonUp(POINT{550,80});
                const bool requested=trade.returnPending && trade.returnSlot==0 && item.present && !ui.m_stCommonItem.bSetted;
                const bool duplicate=!trade.RequestReturn();
                ui.OnLButtonDown(POINT{170,200}); ui.OnLButtonUp(POINT{170,200}); const bool pendingKept=trade.returnSlot==0;
                if (FILE* report=fopen("trade-return-check.txt","wb")) {
                    fprintf(report,"footprint=%d picked=%d confirm_blocked=%d peer_ignored=%d cleared=%d scrolled=%d no_identifier=%d captured=%d requested=%d duplicate_blocked=%d pending_kept=%d\n",
                        footprint,picked,confirmBlocked,peerIgnored,cleared,scrolled,noIdentifier,captured,requested,duplicate,pendingKept); fclose(report);
                }
                mouse=POINT{430,180}; g_xGameProc.m_ptMousePos=mouse;
            }
            if (groupPanel==35) {
                hero.m_nGlod=100000; trade.gold[0]=0;
                ui.OnLButtonDown(POINT{80,290}); ui.OnLButtonUp(POINT{80,290});
                const bool opened=ui.m_xMsgBox.IsActive() && ui.m_nIdxMsgBox==6 && IsWindowVisible(g_xChatEditBox.GetSafehWnd());
                ui.OnKeyDown(VK_ESCAPE,0); const bool escapeBlocked=ui.m_xMsgBox.IsActive()!=FALSE;
                ui.OnLButtonDown(POINT{490,315}); ui.OnLButtonUp(POINT{490,315});
                const bool cancelled=!ui.m_xMsgBox.IsActive() && !trade.goldPending && !IsWindowVisible(g_xChatEditBox.GetSafehWnd());
                bool rejected=true;
                const char* invalid[]={"","0","-1","+1"," 1","1 ","1x","100001","4294967296"};
                for (const auto value:invalid) rejected=rejected && !trade.SubmitGold(value,100000) && !trade.goldPending;
                const bool balanceAllowed=trade.SubmitGold("100000",100000) && trade.goldPending && trade.gold[0]==0 && hero.m_nGlod==100000;
                const bool pendingBlocked=!trade.SubmitGold("1",100000); trade.goldPending=false;
                ui.OnLButtonDown(POINT{80,290}); ui.OnLButtonUp(POINT{80,290});
                lstrcpyA(g_xChatEditBox.m_szInputMsg,"10000"); ui.OnMsgInputted(0,0);
                const bool submitted=trade.goldPending && !ui.m_xMsgBox.IsActive() && !IsWindowVisible(g_xChatEditBox.GetSafehWnd());
                trade.goldPending=false;
                ui.OnLButtonDown(POINT{80,290}); ui.OnLButtonUp(POINT{80,290});
                SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"10000");
                const int scrollBefore=trade.first[0]; ui.OnScrollUp(); const bool modal=ui.m_xMsgBox.IsActive() && trade.first[0]==scrollBefore;
                if (FILE* report=fopen("trade-gold-check.txt","wb")) {
                    fprintf(report,"opened=%d escape_blocked=%d cancelled=%d invalid_rejected=%d full_balance=%d pending_blocked=%d enter_submitted=%d modal=%d\n",
                        opened,escapeBlocked,cancelled,rejected,balanceAllowed,pendingBlocked,submitted,modal); fclose(report);
                }
            }

            if (FILE* report=fopen("trade-layout-check.txt","wb")) {
                fprintf(report,"amount=%d left_scroll=%d right_scroll=%d released=%d stray=%d pending_blocked=%d outside_release=%d body_no_drag=%d header_drag=%d confirmed=%d\n",
                    amount,left,right,released,stray,pending,cancelled,bodyNoDrag,moved,confirmed); fclose(report);
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


        if(hintPreview) {
            auto& ui=g_xGameProc.m_xInterface;
            const int ids[]={_WND_ID_INVENTORY,_WND_ID_STATUS,_WND_ID_QUEST};
            const POINT offsets[]={{245,426},{292,17},{270,78}};
            bool hits=true,pressed=true,outside=true,moved=true,modal=true;
            for(int i=0;i<3;++i) {
                CGameWnd* panel=i==0?(CGameWnd*)&ui.m_xInventoryWnd:i==1?(CGameWnd*)&ui.m_xStatusWnd:(CGameWnd*)&ui.m_xQuestWnd;
                if(panel->m_bActive) ui.WindowActivate(ids[i]);
            }
            for(int i=0;i<3;++i) {
                ui.WindowActivate(ids[i]);
                CGameWnd* panel=i==0?(CGameWnd*)&ui.m_xInventoryWnd:i==1?(CGameWnd*)&ui.m_xStatusWnd:(CGameWnd*)&ui.m_xQuestWnd;
                POINT point={panel->m_rcWnd.left+offsets[i].x,panel->m_rcWnd.top+offsets[i].y},anchor={};
                hits &= ui.ButtonHintAt(point,anchor)==(i==2?2200:1052);
                ui.OnLButtonDown(point); ui.OnMouseMove(point); pressed &= ui.ButtonHintAt(point,anchor)==0;
                ui.OnLButtonUp(POINT{450,450}); outside &= panel->m_bActive && ui.ButtonHintAt(point,anchor)==(i==2?2200:1052);
                const POINT original=anchor; OffsetRect(&panel->m_rcWnd,10,10); point.x+=10;point.y+=10;
                moved &= ui.ButtonHintAt(point,anchor)==(i==2?2200:1052) && anchor.x==original.x+10 && anchor.y==original.y+10;
                ui.ShowNotice(4716); modal &= ui.ButtonHintAt(point,anchor)==0; ui.m_xMsgBox.HideMessageBox();
                OffsetRect(&panel->m_rcWnd,-10,-10); ui.WindowActivate(ids[i]);
            }
            const int index=hintPreview-64; ui.WindowActivate(ids[index]);
            CGameWnd* panel=index==0?(CGameWnd*)&ui.m_xInventoryWnd:index==1?(CGameWnd*)&ui.m_xStatusWnd:(CGameWnd*)&ui.m_xQuestWnd;
            mouse={panel->m_rcWnd.left+offsets[index].x,panel->m_rcWnd.top+offsets[index].y};g_xGameProc.m_ptMousePos=mouse;
            ui.previousHint.clear();ui.hintHeight=6;ui.hintExpanding=true;
            ui.ShowButtonHint(mouse); const bool firstFrame=ui.hintHeight==7 && ui.hintExpanding;
            for(int i=0;i<24;++i) ui.ShowButtonHint(mouse);
            const bool expanded=!ui.hintExpanding && ui.hintHeight==6;
            if(FILE* report=fopen("panel-hint-check.txt","wb")) { fprintf(report,"hits=%d pressed=%d outside=%d moved=%d modal=%d first_frame=%d expanded=%d\n",hits,pressed,outside,moved,modal,firstFrame,expanded);fclose(report); }
            capturePath=index==0?"inventory-hint.bmp":index==1?"status-button-hint.bmp":"quest-hint.bmp";
        }


        if(extraHintPreview) {
            auto& ui=g_xGameProc.m_xInterface; POINT anchor={}; bool hints=true,pressed=true,released=true,modal=true,state=true;
            const int group[][3]={{220,203,1052},{41,195,1600},{101,195,1601},{161,195,1602}};
            const int guild[][3]={{557,404,1052},{42,388,4010},{92,388,4009},{142,388,4008},{292,388,4011},{342,388,4012},{392,388,4013},{442,388,4014},{492,388,4015}};
            const int skill[][3]={{319,425,1052},{49,33,5050},{83,33,5051},{117,33,5052},{151,33,5053},{185,33,5054},{219,33,5055},{253,33,5056},{287,33,5057}};
            const int trade[][3]={{229,282,1300}};
            const int npc[][3]={{349,ui.m_xNPCWnd.Height()-39,1052}};
            const int mode=extraHintPreview-67; const int (*buttons)[3]=mode==0?group:mode==1?guild:mode==2?skill:mode==3?trade:npc;
            CGameWnd* panel=mode==0?(CGameWnd*)&ui.m_xGroupWnd:mode==1?(CGameWnd*)&ui.m_xGuildWnd:mode==2?(CGameWnd*)&ui.m_xSkillWnd:mode==3?(CGameWnd*)&ui.m_xExchangeWnd:(CGameWnd*)&ui.m_xNPCWnd;
            const int count=mode==0?4:mode==1 || mode==2?9:1;
            for(int i=0;i<count;++i) {
                POINT point={panel->m_rcWnd.left+buttons[i][0],panel->m_rcWnd.top+buttons[i][1]};
                hints &= ui.ButtonHintAt(point,anchor)==buttons[i][2];
                ui.OnLButtonDown(point); ui.OnMouseMove(point); pressed &= ui.ButtonHintAt(point,anchor)==0;
                ui.OnLButtonUp(POINT{795,460}); released &= panel->m_bActive && ui.ButtonHintAt(point,anchor)==buttons[i][2];
            }
            const int chosen=mode==0 || mode==2?1:mode==1?4:0;
            mouse={panel->m_rcWnd.left+buttons[chosen][0],panel->m_rcWnd.top+buttons[chosen][1]};g_xGameProc.m_ptMousePos=mouse;
            if(mode==3) {ui.m_xExchangeWnd.confirmed=true;state=ui.ButtonHintAt(mouse,anchor)==1303;ui.m_xExchangeWnd.confirmed=false;}
            if(mode==4) {ui.m_xNPCWnd.canClose=false;state=ui.ButtonHintAt(mouse,anchor)==0;ui.m_xNPCWnd.canClose=true;}
            ui.ShowNotice(4716);modal=ui.ButtonHintAt(mouse,anchor)==0;ui.m_xMsgBox.HideMessageBox();
            for(int i=0;i<24;++i) ui.ShowButtonHint(mouse);
            if(FILE* report=fopen("remaining-hint-check.txt","wb")) {fprintf(report,"hints=%d pressed=%d released=%d modal=%d state=%d\n",hints,pressed,released,modal,state);fclose(report);}
            const char* paths[]={"group-hint.bmp","guild-hint.bmp","skill-button-hint.bmp","trade-hint.bmp","npc-close-hint.bmp"}; capturePath=paths[mode];
        }


        if(groupPanel>=72 && groupPanel<=74) {
            auto& ui=g_xGameProc.m_xInterface;POINT anchor={};
            const int messages[]={2600,2601,2602,2603,2604,2605,2606,2614,2607,2610,2608,2609,2615,2611,2612,2613,2616,2617,2618,2619,2637,2639,2643};
            const auto pointAt=[&](int i) {const auto& button=ui.m_xInterBtn[i]; const RECT rect=button.m_customHit?button.m_hitRect:button.m_rcBtn;return POINT{rect.left+2,rect.top+2};};
            bool messagesOk=true,anchors=true,pressed=true,reception=true;
            ui.m_chatExpanded=true;ui.LayoutChat();
            for(int i=0;i<23;++i) {
                const POINT point=pointAt(i);auto& button=ui.m_xInterBtn[i];const RECT rect=button.m_customHit?button.m_hitRect:button.m_rcBtn;
                messagesOk &= ui.ButtonHintAt(point,anchor)==messages[i];anchors &= anchor.x==rect.right && anchor.y==rect.bottom;
                button.SetBtnState(_BTN_STATE_CLICK);pressed &= ui.ButtonHintAt(point,anchor)==0;button.SetBtnState(_BTN_STATE_NORMAL);
            }
            ui.m_chatMode=1; const bool group=ui.ButtonHintAt(pointAt(18),anchor)==2631;
            ui.m_chatMode=2; const bool guild=ui.ButtonHintAt(pointAt(19),anchor)==2630;ui.m_chatMode=0;
            for(int i=0;i<3;++i) {
                const POINT point=pointAt(20+i);ui.m_chatRejected[i]=false;ui.OnLButtonDown(point);ui.OnLButtonUp(point);
                reception &= ui.ButtonHintAt(point,anchor)==messages[20+i]+1;
                ui.m_chatRejected[i]=true;ui.OnLButtonDown(point);ui.OnLButtonUp(point);
                reception &= ui.ButtonHintAt(point,anchor)==messages[20+i];
            }
            ui.m_chatExpanded=false;ui.LayoutChat();const bool hidden=ui.ButtonHintAt(pointAt(18),anchor)==0;
            ui.ShowNotice(4716);const bool modal=ui.ButtonHintAt(pointAt(14),anchor)==0;ui.m_xMsgBox.HideMessageBox();
            ui.WindowActivate(_WND_ID_INVENTORY);OffsetRect(&ui.m_xInventoryWnd.m_rcWnd,0,100);const bool covered=ui.ButtonHintAt(pointAt(14),anchor)==0;OffsetRect(&ui.m_xInventoryWnd.m_rcWnd,0,-100);ui.WindowActivate(_WND_ID_INVENTORY);
            if(groupPanel>=73) {ui.m_chatExpanded=true;ui.LayoutChat();}
            if(groupPanel==73) {ui.m_chatMode=1;ui.m_chatModeChanged=true;}
            mouse=pointAt(groupPanel==72?14:groupPanel==73?18:20);g_xGameProc.m_ptMousePos=mouse;
            for(int i=0;i<24;++i) ui.ShowButtonHint(mouse);
            capturePath=groupPanel==72?"hud-button-hint.bmp":groupPanel==73?"chat-mode-hint.bmp":"chat-reception-hint.bmp";
            if(FILE* report=fopen("hud-button-hint-check.txt","wb")) {fprintf(report,"messages=%d anchors=%d pressed=%d group=%d guild=%d reception=%d hidden=%d modal=%d covered=%d\n",messagesOk,anchors,pressed,group,guild,reception,hidden,modal,covered);fclose(report);}
        }


        if(groupPanel>=75 && groupPanel<=78) {
            auto& ui=g_xGameProc.m_xInterface;auto& map=ui.m_minimap;POINT anchor={};
            const bool request=map.Toggle(1000,1000) && map.pending && !map.visible;
            const bool invalid=!map.Receive(0,1500) && map.pending;
            const bool loaded=map.Receive(1001,2000) && map.visible && !map.pending;
            hero.m_wPosX=337;hero.m_wPosY=267;map.player={337,267};
            const RECT bounds=map.Bounds();const POINT source=map.Source();
            const bool compact=bounds.left==672 && bounds.top==0 && bounds.right==800 && bounds.bottom==128 && source.x==441 && source.y==203;
            const bool cooldown=!map.Toggle(3000,1000) && map.visible;
            const bool hit=ui.GetWindowInMousePos(POINT{700,40})==_WND_ID_MINIMAP;
            POINT alpha=map.ButtonPosition(0);alpha.x+=5;alpha.y+=5;
            const bool hint=ui.ButtonHintAt(alpha,anchor)==1800;
            ui.OnLButtonDown(alpha);const bool transparent=map.translucent && map.releaseCaptured && ui.ButtonHintAt(alpha,anchor)==1802;
            ui.OnLButtonUp(POINT{400,400});const bool release=!map.releaseCaptured;
            POINT resize=map.ButtonPosition(1);resize.x+=5;resize.y+=5;ui.OnLButtonDown(resize);ui.OnLButtonUp(POINT{400,400});
            const bool large=map.large && map.position.x==544 && map.Bounds().bottom==256;
            map.Pointer(0,POINT{560,20},true);map.Pointer(1,POINT{580,40},true);
            const bool dragged=map.position.x==564 && map.position.y==20 && map.dragging;
            map.Pointer(1,POINT{0,500},true);const bool dragOutside=map.position.x==564 && map.position.y==20;
            map.Pointer(1,POINT{590,45},false);const bool ctrl=map.position.x==564;map.Pointer(2,POINT{0,500},false);
            ui.ShowNotice(4716);const bool modal=ui.ButtonHintAt(POINT{600,50},anchor)==0;ui.m_xMsgBox.HideMessageBox();
            ui.OnKeyDown('T',0);const bool resizeKey=!map.large && map.position.x==672;ui.OnKeyDown('T',0);
            const bool oldAlpha=map.translucent;ui.OnKeyDown('Y',0);const bool alphaKey=map.translucent!=oldAlpha;
            ui.WindowActivate(_WND_ID_INVENTORY);ui.OnLButtonDown(POINT{560,80});ui.OnLButtonUp(POINT{560,80});
            const bool covered=ui.GetWindowInMousePos(POINT{560,80})==_WND_ID_MINIMAP && !ui.m_stCommonItem.bSetted;
            g_xGameProc.m_ptMousePos=POINT{560,80};const int oldRow=ui.m_xInventoryWnd.ScrollRow();ui.OnScrollUp();ui.OnScrollDown();
            const bool wheel=oldRow==ui.m_xInventoryWnd.ScrollRow();const bool doubleClick=ui.OnLButtonDoubleClick(POINT{560,80})!=FALSE;
            ui.WindowActivate(_WND_ID_INVENTORY);
            if(FILE* report=fopen("minimap-input-check.txt","wb")) {fprintf(report,"resize_key=%d alpha_key=%d covered=%d wheel=%d double_click=%d\n",resizeKey,alphaKey,covered,wheel,doubleClick);fclose(report);}
            map.Unavailable(4000);const bool unavailable=!map.visible && !map.pending && map.replyTime==4000;
            map.Receive(1001,4100);const bool unsolicited=!map.visible;
            map.Toggle(6000,100);const bool smallArchive=map.Receive(1,6100) && map.visible;
            map.pending=true;map.Receive(1001,6200);map.large=groupPanel!=75;map.translucent=groupPanel==77;map.Place();
            if(groupPanel==78) {map.Pointer(0,POINT{790,20},true);map.Pointer(1,POINT{600,80},true);map.Pointer(2,POINT{600,80},false);}
            map.monsters={{322,251}};map.green={{350,284}};map.orange={{360,276},{380,283}};
            mouse=POINT{400,460};g_xGameProc.m_ptMousePos=mouse;
            const char* paths[]={"minimap-small.bmp","minimap-large.bmp","minimap-transparent.bmp","minimap-dragged.bmp"};capturePath=paths[groupPanel-75];
            if(FILE* report=fopen("minimap-check.txt","wb")) {fprintf(report,"request=%d invalid=%d loaded=%d compact=%d cooldown=%d hit=%d hint=%d transparent=%d release=%d large=%d dragged=%d outside=%d ctrl=%d modal=%d unavailable=%d unsolicited=%d small_archive=%d\n",request,invalid,loaded,compact,cooldown,hit,hint,transparent,release,large,dragged,dragOutside,ctrl,modal,unavailable,unsolicited,smallArchive);fclose(report);}
        }


        if(groupPanel>=79 && groupPanel<=81) {
            auto& ui=g_xGameProc.m_xInterface;auto& map=ui.m_minimap;
            hero.m_wPosX=337;hero.m_wPosY=267;map.player={337,267};
            const bool request=map.TogglePortrait(1000) && map.portraitMode && map.portraitPending && !map.portraitVisible && !map.pending;
            const bool loaded=map.Receive(1001,2000) && map.portraitVisible && !map.visible;
            const RECT crop=map.PortraitSource();const POINT origin=map.PortraitOrigin(),player=map.PortraitMarker(map.player,true),nearby=map.PortraitMarker(POINT{350,284},false);
            const bool geometry=crop.left==377 && crop.top==139 && crop.right==633 && crop.bottom==395 && origin.x==510 && origin.y==470;
            const bool markers=player.x==572 && player.y==532 && nearby.x==583 && nearby.y==542;
            const bool cooldown=!map.TogglePortrait(3000) && map.portraitMode;
            const bool disabled=map.TogglePortrait(3001) && !map.portraitMode && !map.portraitVisible;
            ui.OnLButtonDown(POINT{540,570});ui.OnLButtonUp(POINT{550,510});const bool click=map.portraitMode && map.portraitPending;
            map.Receive(1001,0);ui.OnKeyDown('J',0);const bool key=!map.portraitMode;
            map.LoadOptions("./preview-information-mode.ini");map.replyTime=0;
            if(map.portraitMode) map.TogglePortrait(1000);map.TogglePortrait(2000);map.LoadOptions("./preview-information-mode.ini");map.Init();
            const bool persisted=map.portraitMode && map.portraitPending && !map.portraitVisible;
            map.LoadOptions(nullptr);map.TogglePortrait(3000);map.Unavailable(4000);
            const bool unavailable=map.portraitMode && !map.portraitVisible && !map.portraitPending;
            map.replyTime=0;map.portraitMode=false;map.TogglePortrait(5000);map.Receive(groupPanel==80?1:1001,6000);
            map.green={{350,284}};map.orange={{360,276}};map.portraitBlink=100;
            ui.m_chatExpanded=groupPanel==81;ui.LayoutChat();mouse={400,460};g_xGameProc.m_ptMousePos=mouse;
            capturePath=groupPanel==79?"portrait-map-full.bmp":groupPanel==80?"portrait-map-normal.bmp":"portrait-map-expanded.bmp";
            if(FILE* report=fopen("portrait-map-check.txt","wb")) {fprintf(report,"request=%d loaded=%d geometry=%d markers=%d cooldown=%d disabled=%d click=%d key=%d persisted=%d unavailable=%d\n",request,loaded,geometry,markers,cooldown,disabled,click,key,persisted,unavailable);fclose(report);}
        }


        if(groupPanel>=82 && groupPanel<=86) {
            auto& target=g_xGameProc.m_xInterface.m_targetDisplay;
            target.SetMonster(1,3,20,false,0,50,100,50,1000);
            const bool monster=target.Displayed()==1 && target.Frame()==20 && target.BarWidth()==41 && target.VitalText(0)=="50" && target.VitalText(2)=="100";
            target.Focus(0,1800);const bool retained=target.Displayed()==1;target.Focus(0,1801);const bool expired=target.Displayed()==0;
            target.SetMonster(2,3,127,false,0,0,100,100,2000);const bool excluded=target.Frame()==-1;
            target.SetMonster(3,3,65535,false,0,0,100,100,2000);const bool fallback=target.Frame()==1999;
            target.SetMonster(4,3,531,false,1,50,100,50,2000);const bool special=target.Frame()==531 || target.Frame()==1999;
            target.SetHuman(5,1,1,2000);const bool human=target.Frame()==653;target.SetHuman(6,0,255,2000);const bool profession=target.Frame()==0;
            target.SetMonster(1,3,20,false,0,50,100,50,3000);
            const BYTE body[]={2,0,3,5,0,BYTE(-3),2,0,BYTE(-1),2};
            const bool wire=target.ApplyAttributes(1,0,1,body,10) && !target.ApplyAttributes(100,0,1,body,10) && !target.ApplyAttributes(1,0,1,body,9);
            const auto& values=target.attributes[1];const auto order=values.Order();// Preserve the reference two-pass order, including negative values.
            const int expected[]={5,2,2,-3,-1,0,0};bool sorted=true;
            for(int i=0;i<7;++i) sorted &= values.modifiers[order[i]]==expected[i];
            target.SetHuman(7,0,1,4000);const bool late=target.ApplyAttributes(1,1,0,body,10) && target.Displayed()==7 && target.Frame()==652;
            target.Remove(7);const bool removed=target.Displayed()==0;
            target.SetMonster(1,3,20,false,0,0,100,0,5000);const bool unknown=target.VitalText(0)=="?" && target.VitalText(2)=="?" && target.BarWidth()==0;
            target.attributes.clear();
            if(groupPanel==84) target.SetHuman(7,1,1,6000);
            else target.SetMonster(1,3,groupPanel==86?531:20,false,0,groupPanel==85?0:50,100,groupPanel==85?0:50,6000);
            if(groupPanel==83) target.ApplyAttributes(1,0,1,body,10);
            mouse={400,460};g_xGameProc.m_ptMousePos=mouse;
            const char* paths[]={"target-monster.bmp","target-attributes.bmp","target-human.bmp","target-unknown.bmp","target-special.bmp"};capturePath=paths[groupPanel-82];
            if(FILE* report=fopen("target-display-check.txt","wb")) {fprintf(report,"monster=%d retained=%d expired=%d excluded=%d fallback=%d special=%d human=%d profession=%d wire=%d sorted=%d late=%d removed=%d unknown=%d\n",monster,retained,expired,excluded,fallback,special,human,profession,wire,sorted,late,removed,unknown);fclose(report);}
        }


        if(groupPanel==87 || groupPanel==88) {
            auto& ui=g_xGameProc.m_xInterface;auto& target=ui.m_targetDisplay;
            std::vector<BYTE> jpeg;if(FILE* file=fopen("./portrait-preview/portrait01.jpg","rb")) {fseek(file,0,SEEK_END);jpeg.resize(ftell(file));rewind(file);fread(jpeg.data(),1,jpeg.size(),file);fclose(file);}
            target.SetHuman(7,1,1,1000);const bool loaded=target.ApplyPortrait(7,jpeg.data(),jpeg.size()) && target.HasPortrait();
            const bool wrong=!target.ApplyPortrait(8,jpeg.data(),jpeg.size()) && target.HasPortrait();
            const BYTE bad[]={0xff,0xd8,0,0};const bool invalid=!target.ApplyPortrait(7,bad,sizeof(bad)) && target.HasPortrait();
            auto padded=jpeg;padded.resize(4096);const bool limit=target.ApplyPortrait(7,padded.data(),padded.size());padded.push_back(0);
            const bool oversized=!target.ApplyPortrait(7,padded.data(),padded.size()) && target.HasPortrait();
            target.SetHuman(7,0,1,1100);const bool same=target.HasPortrait();target.SetHuman(8,0,1,1200);const bool changed=!target.HasPortrait();
            target.ApplyPortrait(8,jpeg.data(),jpeg.size());const bool cleared=target.ApplyPortrait(8,nullptr,0) && !target.HasPortrait();
            target.SetMonster(9,3,20,false,0,50,100,50,1300);const bool monster=!target.ApplyPortrait(9,jpeg.data(),jpeg.size());
            target.SetHuman(7,1,1,1400);target.ApplyPortrait(7,jpeg.data(),jpeg.size());
            ui.WindowActivate(_WND_ID_OPTION);ui.OnLButtonDown(POINT{578,328});ui.OnLButtonUp(POINT{578,328});
            const bool option=!ui.m_xOptionWnd.m_values[20] && target.HasPortrait();ui.WindowActivate(_WND_ID_OPTION);
            ui.m_xOptionWnd.LoadAudioOptions("./preview-photo-options.ini");ui.m_xOptionWnd.m_values[20]=false;
            const bool saved=ui.m_xOptionWnd.SaveAudioOptions();ui.m_xOptionWnd.LoadAudioOptions(nullptr);
            const bool defaults=ui.m_xOptionWnd.m_values[20];ui.m_xOptionWnd.LoadAudioOptions("./preview-photo-options.ini");
            const bool persisted=saved && defaults && !ui.m_xOptionWnd.m_values[20];ui.m_xOptionWnd.LoadAudioOptions(nullptr);
            ui.m_xOptionWnd.m_values[20]=groupPanel==87;
            mouse={400,460};g_xGameProc.m_ptMousePos=mouse;capturePath=groupPanel==87?"target-photo.bmp":"target-photo-disabled.bmp";
            if(FILE* report=fopen("target-photo-check.txt","wb")) {fprintf(report,"loaded=%d wrong_actor=%d invalid=%d limit=%d oversized=%d same_actor=%d changed_actor=%d cleared=%d monster=%d option=%d persisted=%d\n",loaded,wrong,invalid,limit,oversized,same,changed,cleared,monster,option,persisted);fclose(report);}
        }


        if(groupPanel==89 || groupPanel==90) {
            auto& ui=g_xGameProc.m_xInterface;auto& options=ui.m_xOptionWnd;auto& map=ui.m_minimap;
            options.LoadAudioOptions("./preview-marker-options.ini");options.m_values[7]=true;options.SaveAudioOptions();
            ui.WindowActivate(_WND_ID_OPTION);const POINT toggle={options.m_rcWnd.left+189,options.m_rcWnd.top+58};
            ui.OnLButtonUp(toggle);const bool stray=options.m_values[7];
            ui.OnLButtonDown(toggle);ui.OnLButtonUp(toggle);const bool disabled=!options.m_values[7];
            options.LoadAudioOptions(nullptr);const bool defaults=options.m_values[7];options.LoadAudioOptions("./preview-marker-options.ini");const bool persisted=!options.m_values[7];
            ui.OnLButtonDown(toggle);ui.OnLButtonUp(toggle);const bool enabled=options.m_values[7];
            if(groupPanel==90) {ui.OnLButtonDown(toggle);ui.OnLButtonUp(toggle);}
            const bool finalValue=options.m_values[7];options.LoadAudioOptions(nullptr);options.m_values[7]=finalValue;
            map.pending=true;map.Receive(1001,0);hero.m_wPosX=337;hero.m_wPosY=267;map.monsters={{322,251}};map.green={{350,284}};map.orange={{360,276}};
            mouse=POINT{400,460};g_xGameProc.m_ptMousePos=mouse;capturePath=groupPanel==89?"options-markers-on.bmp":"options-markers-off.bmp";
            if(FILE* report=fopen("options-markers-check.txt","wb")) {fprintf(report,"stray=%d disabled=%d defaults=%d persisted=%d enabled=%d\n",stray,disabled,defaults,persisted,enabled);fclose(report);}
        }


        if(groupPanel==91 || groupPanel==92) {
            auto& ui=g_xGameProc.m_xInterface;auto& options=ui.m_xOptionWnd;
            options.LoadAudioOptions("./preview-shadow-options.ini");g_xGameProc.SetShadowBlend(false);options.SaveAudioOptions();
            ui.WindowActivate(_WND_ID_OPTION);const POINT toggle={options.m_rcWnd.left+433,options.m_rcWnd.top+238};
            ui.OnLButtonDown(toggle);ui.OnLButtonUp(toggle);
            bool enabled=options.m_values[4] && g_xGameProc.m_bShadowABlend;for(int i=0;i<800;++i) enabled &= g_xGameProc.m_wShadowClr[i]==0;
            options.LoadAudioOptions(nullptr);const bool defaults=!options.m_values[4] && !g_xGameProc.m_bShadowABlend;
            options.LoadAudioOptions("./preview-shadow-options.ini");const bool persisted=options.m_values[4] && g_xGameProc.m_bShadowABlend;
            ui.OnLButtonDown(toggle);ui.OnLButtonUp(toggle);bool disabled=!options.m_values[4] && !g_xGameProc.m_bShadowABlend;
            const WORD color=g_xMainWnd.ConvertColor24To16(RGB(20,20,20));for(int i=0;i<800;++i) disabled &= g_xGameProc.m_wShadowClr[i]==color;
            options.LoadAudioOptions(nullptr);g_xGameProc.SetShadowBlend(groupPanel==91);
            mouse={400,460};g_xGameProc.m_ptMousePos=mouse;capturePath=groupPanel==91?"options-shadow-on.bmp":"options-shadow-off.bmp";
            if(FILE* report=fopen("options-shadow-check.txt","wb")) {fprintf(report,"enabled=%d defaults=%d persisted=%d disabled=%d\n",enabled,defaults,persisted,disabled);fclose(report);}
        }


        if(groupPanel==93 || groupPanel==94) {
            auto& ui=g_xGameProc.m_xInterface;auto& options=ui.m_xOptionWnd;
            class LightProbe : public CLightFog {public: bool Lit() const {for(const auto& row:m_avLight) for(const auto& vertex:row) if(vertex.color) return true;return false;} } fog;
            CMagic magic;magic.m_bActiveState=_MAGIC_ACTIVE;magic.m_bFixed=FALSE;magic.m_bShowLight=TRUE;magic.m_bMagicNum=0;
            magic.m_shScrnX=200;magic.m_shScrnY=200;magic.m_bLightRadius[0]=2;magic.m_bLightRadius[1]=4;magic.m_bSwingCnt=5;magic.m_bCurrSwing=0;
            const BYTE colors[2][3]={{255,160,80},{60,40,20}};memcpy(magic.m_bLightColor,colors,sizeof(colors));magic.m_wCurrLightDelay=0;
            options.LoadAudioOptions("./preview-light-options.ini");options.m_values[9]=true;options.SaveAudioOptions();
            fog.Init(0);magic.DrawLight(&fog,0);const bool lit=fog.Lit();
            ui.WindowActivate(_WND_ID_OPTION);const POINT toggle={options.m_rcWnd.left+189,options.m_rcWnd.top+118};
            ui.OnLButtonDown(toggle);ui.OnLButtonUp(toggle);fog.Init(0);magic.DrawLight(&fog,0);
            const bool dark=!options.m_values[9] && !fog.Lit() && magic.m_bActiveState==_MAGIC_ACTIVE;
            options.LoadAudioOptions(nullptr);const bool defaults=options.m_values[9];options.LoadAudioOptions("./preview-light-options.ini");const bool persisted=!options.m_values[9];
            magic.m_bActiveState=_LIGHT_ACTIVE;magic.m_bLightRadius[0]=1;magic.m_bLightRadius[1]=2;magic.m_wCurrLightDelay=0;
            const bool lifecycle=!magic.DrawLight(&fog,201) && magic.m_bActiveState==_MAGIC_FINISH;
            ui.OnLButtonDown(toggle);ui.OnLButtonUp(toggle);magic.m_bActiveState=_MAGIC_ACTIVE;magic.m_bLightRadius[0]=2;magic.m_bLightRadius[1]=4;fog.Init(0);magic.DrawLight(&fog,0);const bool resumed=options.m_values[9] && fog.Lit();
            if(groupPanel==94) {ui.OnLButtonDown(toggle);ui.OnLButtonUp(toggle);}
            const bool value=options.m_values[9];options.LoadAudioOptions(nullptr);options.m_values[9]=value;
            mouse={400,460};g_xGameProc.m_ptMousePos=mouse;capturePath=groupPanel==93?"options-light-on.bmp":"options-light-off.bmp";
            if(FILE* report=fopen("options-light-check.txt","wb")) {fprintf(report,"lit=%d dark=%d defaults=%d persisted=%d lifecycle=%d resumed=%d\n",lit,dark,defaults,persisted,lifecycle,resumed);fclose(report);}
        }


        if(groupPanel==95 || groupPanel==96) {
            auto& ui=g_xGameProc.m_xInterface;auto& options=ui.m_xOptionWnd;
            options.LoadAudioOptions("./preview-damage-options.ini");options.m_values[10]=true;options.SaveAudioOptions();
            auto& actor=g_xGameProc.m_xMyHero;actor.m_damage=CDamageNumbers{};actor.HealthDamage(100,125);
            const bool healed=actor.m_damage.cursor==1 && actor.m_damage.values[0]==25;
            actor.HealthDamage(0,100);actor.HealthDamage(100,100);const bool unchanged=actor.m_damage.cursor==1;
            ui.WindowActivate(_WND_ID_OPTION);const POINT toggle={options.m_rcWnd.left+189,options.m_rcWnd.top+148};
            ui.OnLButtonDown(toggle);ui.OnLButtonUp(toggle);actor.AddDamage(-20);
            const bool disabled=!options.m_values[10] && actor.m_damage.cursor==1;
            options.LoadAudioOptions(nullptr);const bool defaults=options.m_values[10];options.LoadAudioOptions("./preview-damage-options.ini");const bool persisted=!options.m_values[10];
            CDamageNumbers probe;probe.Push(-99);auto glyphs=probe.Draw(100,100,true);
            const bool smallDigits=glyphs.size()==3 && glyphs[0].frame==1800 && glyphs[0].x==112 && glyphs[0].y==64 && glyphs[1].frame==1799;
            const bool large=CDamageNumbers::Glyphs(-100,0,0)[0].frame==1840 && CDamageNumbers::Glyphs(25,0,0)[0].frame==1821 && CDamageNumbers::Glyphs(0,0,0)[0].frame==1850;
            probe.ages[0]=40;const bool last=!probe.Draw(100,100,false).empty() && probe.Draw(100,100,false).empty();
            probe=CDamageNumbers{};for(int i=0;i<8;++i) probe.Push(-1);const bool wrap=probe.cursor==0 && probe.Draw(0,0,false).empty();
            options.LoadAudioOptions(nullptr);options.m_values[10]=groupPanel==95;
            mouse={400,460};g_xGameProc.m_ptMousePos=mouse;capturePath=groupPanel==95?"options-damage-on.bmp":"options-damage-off.bmp";
            if(FILE* report=fopen("options-damage-check.txt","wb")) {fprintf(report,"healed=%d unchanged=%d disabled=%d defaults=%d persisted=%d small=%d styles=%d lifetime=%d wrap=%d\n",healed,unchanged,disabled,defaults,persisted,smallDigits,large,last,wrap);fclose(report);}
        }

        if(groupPanel==97 || groupPanel==98) {
            auto& ui=g_xGameProc.m_xInterface;auto& options=ui.m_xOptionWnd;
            options.LoadAudioOptions("./preview-attack-options.ini");options.m_values[8]=true;options.SaveAudioOptions();
            ui.WindowActivate(_WND_ID_OPTION);const POINT toggle={options.m_rcWnd.left+189,options.m_rcWnd.top+88};
            const bool enabled=options.CanAttackHuman(0xffffff,false);
            ui.OnLButtonDown(toggle);ui.OnLButtonUp(toggle);
            const bool disabled=!options.m_values[8] && !options.CanAttackHuman(0xffffff,false);
            const bool exceptions=options.CanAttackHuman(255,false) && options.CanAttackHuman(27631,false) && options.CanAttackHuman(0xffffff,true);
            options.LoadAudioOptions(nullptr);const bool defaults=options.m_values[8];options.LoadAudioOptions("./preview-attack-options.ini");const bool persisted=!options.m_values[8];
            options.LoadAudioOptions(nullptr);options.m_values[8]=groupPanel==97;
            mouse={400,460};g_xGameProc.m_ptMousePos=mouse;capturePath=groupPanel==97?"options-attack-on.bmp":"options-attack-off.bmp";
            if(FILE* report=fopen("options-attack-check.txt","wb")) {fprintf(report,"enabled=%d disabled=%d exceptions=%d defaults=%d persisted=%d\n",enabled,disabled,exceptions,defaults,persisted);fclose(report);}
        }

        if(groupPanel==99 || groupPanel==100) {
            auto& ui=g_xGameProc.m_xInterface;auto& options=ui.m_xOptionWnd;
            ui.WindowActivate(_WND_ID_OPTION);options.ReceiveServerOptions(0,0);
            const POINT toggle={options.m_rcWnd.left+189,options.m_rcWnd.top+238};
            ui.OnLButtonDown(toggle);ui.OnLButtonUp(toggle);
            const bool predicted=options.m_values[13] && !options.m_confirmed[2];
            const bool unchanged=options.ReceiveServerOptions(0,0)==0 && options.m_values[13];
            bool commands=true;for(int i=0;i<7;++i) commands &= options.RequestServerOption(i)==2123+i;
            _TDEFAULTMESSAGE packet={};packet.wIdent=1401;packet.nRecog=LONG(0xcf800000u);packet.wParam=4;
            char encoded[64]={};fnEncodeMessage(&packet,encoded,sizeof(encoded));g_xGameProc.OnSocketMessageRecieve(encoded);
            bool confirmed=options.m_attackMode==4;for(int i=0;i<7;++i) confirmed &= options.m_confirmed[i];
            const bool reception=ui.m_chatRejected[0] && ui.m_chatRejected[1] && ui.m_chatRejected[2] && ui.m_chatReceptionFrame[0]==1338 && ui.m_chatReceptionFrame[1]==1340 && ui.m_chatReceptionFrame[2]==-1;
            const bool repeated=options.ReceiveServerOptions(0xcf800000u,4)==0 && ui.m_chatRejected[0];
            if(groupPanel==100) options.ReceiveServerOptions(0,0);
            mouse={400,460};g_xGameProc.m_ptMousePos=mouse;capturePath=groupPanel==99?"options-server-on.bmp":"options-server-off.bmp";
            if(FILE* report=fopen("options-server-check.txt","wb")) {fprintf(report,"predicted=%d unchanged=%d commands=%d confirmed=%d reception=%d repeated=%d\n",predicted,unchanged,commands,confirmed,reception,repeated);fclose(report);}
        }

        if(groupPanel==101 || groupPanel==102) {
            auto& map=g_xGameProc.m_xInterface.m_minimap;
            const auto receive=[&](WORD opcode,WORD number) {
                _TDEFAULTMESSAGE packet={};packet.wIdent=opcode;packet.wParam=number;
                char encoded[64]={};fnEncodeMessage(&packet,encoded,sizeof(encoded));g_xGameProc.OnSocketMessageRecieve(encoded);
            };
            map.pending=true;map.portraitMode=map.portraitPending=true;receive(710,1001);
            const bool opened=map.visible && map.portraitVisible && !map.pending && !map.portraitPending && map.width>0;
            map.monsters.push_back({10,10});map.green.push_back({20,20});map.orange.push_back({30,30});map.dragging=true;map.releaseCaptured=true;
            map.MapChanged();const bool cleared=!map.visible && !map.portraitVisible && map.pending && map.portraitPending && !map.width && !map.height && !map.replyTime && !map.dragging && !map.releaseCaptured && map.monsters.empty() && map.green.empty() && map.orange.empty();
            receive(710,0);const bool invalid=map.pending && !map.visible;
            receive(710,1002);const bool refreshed=map.visible && map.portraitVisible && !map.pending;
            receive(711,0);const bool unavailable=!map.visible && !map.pending && !map.portraitVisible && !map.portraitPending && map.replyTime;
            map.portraitMode=false;map.MapChanged();const bool closed=!map.pending && !map.portraitPending;
            if(groupPanel==101) {map.pending=true;map.portraitMode=map.portraitPending=true;receive(710,1001);}
            g_xGameProc.m_xMyHero.m_wPosX=337;g_xGameProc.m_xMyHero.m_wPosY=267;map.player={337,267};map.portraitBlink=100;
            mouse={400,460};g_xGameProc.m_ptMousePos=mouse;capturePath=groupPanel==101?"minimap-server-reply.bmp":"minimap-server-unavailable.bmp";
            if(FILE* report=fopen("minimap-server-check.txt","wb")) {fprintf(report,"opened=%d cleared=%d invalid=%d refreshed=%d unavailable=%d closed=%d\n",opened,cleared,invalid,refreshed,unavailable,closed);fclose(report);}
        }

        if(groupPanel==103 || groupPanel==104) {
            auto& target=g_xGameProc.m_xInterface.m_targetDisplay;
            const auto receive=[&](WORD opcode,DWORD actor,const BYTE* body,int length) {
                _TDEFAULTMESSAGE packet={};packet.wIdent=opcode;packet.nRecog=actor;packet.wParam=1;packet.wTag=2;
                char encoded[128]={};fnEncodeMessage(&packet,encoded,sizeof(encoded));
                if(length) fnEncode6BitBuf(const_cast<BYTE*>(body),encoded+DEFBLOCKSIZE,length,sizeof(encoded)-DEFBLOCKSIZE);
                g_xGameProc.OnSocketMessageRecieve(encoded);
            };
            const BYTE body[]={2,0,3,5,0,253,2,0,255,2};
            target.SetMonster(123,3,1,false,0,500,1000,50,GetTickCount());receive(1304,123,body,10);
            const bool decoded=target.attributes[123].category==1 && target.attributes[123].state==2 && target.attributes[123].icon==2 && target.attributes[123].modifiers[2]==-3;
            receive(1304,456,body,10);const bool unknown=target.attributes.find(456)==target.attributes.end();
            const BYTE invalid[]={4};receive(1304,123,invalid,1);const bool truncated=target.attributes[123].icon==2;
            target.SetMonster(456,3,1,false,0,20,40,50,GetTickCount());receive(1304,123,body,10);const bool delayed=target.Displayed()==456 && target.attributes[123].icon==2;
            receive(SM_DISAPPEAR,456,nullptr,0);const bool removed=!target.Displayed() && target.attributes.find(456)==target.attributes.end();
            target.Clear();const bool cleared=!target.Displayed() && target.attributes.empty() && target.Frame()==-1 && !target.HasPortrait();
            target.SetMonster(123,3,1,false,0,500,1000,50,GetTickCount());receive(1304,123,body,10);
            if(groupPanel==104) receive(SM_CLEAROBJECT,123,nullptr,0);
            const bool finalState=groupPanel==103?target.Displayed()==123:!target.Displayed();
            mouse={400,460};g_xGameProc.m_ptMousePos=mouse;capturePath=groupPanel==103?"target-server-attributes.bmp":"target-server-removed.bmp";
            if(FILE* report=fopen("target-server-check.txt","wb")) {fprintf(report,"decoded=%d unknown=%d truncated=%d delayed=%d removed=%d cleared=%d final=%d\n",decoded,unknown,truncated,delayed,removed,cleared,finalState);fclose(report);}
        }

        if(groupPanel==105 || groupPanel==106) {
            auto& target=g_xGameProc.m_xInterface.m_targetDisplay;target.Clear();
            CActor actor;actor.m_dwIdentity=123;actor.m_stFeature.bGender=_GENDER_MON;actor.m_stFeature.bDress=1;
            actor.m_wHP=500;actor.m_wMAXHP=1000;actor.m_bHPPercent=50;actor.m_wPosX=337;actor.m_wPosY=267;actor.m_bIsDead=FALSE;
            g_xGameProc.UpdateTargetDisplay(&actor,1000);
            const bool monster=target.Displayed()==123 && target.Frame()==1 && target.current==500 && target.maximum==1000 && target.percent==50 && target.attributes.count(123)==1;
            actor.m_wHP=250;actor.m_bHPPercent=25;g_xGameProc.UpdateTargetDisplay(&actor,1100);const bool health=target.current==250 && target.percent==25;
            g_xGameProc.UpdateTargetDisplay(nullptr,1800);const bool retained=target.Displayed()==123;
            g_xGameProc.UpdateTargetDisplay(nullptr,1801);const bool expired=!target.Displayed();
            actor.m_bIsDead=TRUE;g_xGameProc.UpdateTargetDisplay(&actor,1900);const bool dead=!target.Displayed();actor.m_bIsDead=FALSE;
            actor.m_stFeature.bGender=_GENDER_NPC;g_xGameProc.UpdateTargetDisplay(&actor,2000);const bool npc=!target.Displayed();
            actor.m_stFeature.bGender=_GENDER_MAN;g_xGameProc.UpdateTargetDisplay(&actor,2100);const bool unknown=target.Displayed()==123 && target.Frame()==0;
            actor.m_stFeature.bGender=_GENDER_MON;actor.m_wHP=500;actor.m_bHPPercent=50;
            if(groupPanel==105) g_xGameProc.UpdateTargetDisplay(&actor,2200);else target.Clear();
            mouse={400,460};g_xGameProc.m_ptMousePos=mouse;capturePath=groupPanel==105?"target-native-hover.bmp":"target-native-expired.bmp";
            if(FILE* report=fopen("target-hover-check.txt","wb")) {fprintf(report,"monster=%d health=%d retained=%d expired=%d dead=%d npc=%d unknown=%d\n",monster,health,retained,expired,dead,npc,unknown);fclose(report);}
        }

        if(groupPanel==107) {
            auto& target=g_xGameProc.m_xInterface.m_targetDisplay;target.Clear();
            target.SetHuman(123,0,0,1000);const bool empty=!target.QueryPortrait("");
            const bool first=target.QueryPortrait("Preview");const bool repeated=!target.QueryPortrait("Preview");
            target.SetHuman(123,0,1,1100);const bool same=!target.QueryPortrait("Preview");
            target.ApplyPortrait(123,nullptr,0);const bool noPhoto=!target.QueryPortrait("Preview");
            const bool renamed=target.QueryPortrait("Renamed");target.SetHuman(456,1,2,1200);const bool switched=target.QueryPortrait("Renamed");
            target.Remove(456);const bool removed=!target.QueryPortrait("Renamed");target.SetHuman(456,1,2,1300);const bool returned=target.QueryPortrait("Renamed");
            target.Clear();target.SetHuman(456,1,2,1400);const bool reset=target.QueryPortrait("Renamed");
            mouse={400,460};g_xGameProc.m_ptMousePos=mouse;capturePath="target-portrait-query.bmp";
            if(FILE* report=fopen("target-portrait-query-check.txt","wb")) {fprintf(report,"empty=%d first=%d repeated=%d same=%d no_photo=%d renamed=%d switched=%d removed=%d returned=%d reset=%d\n",empty,first,repeated,same,noPhoto,renamed,switched,removed,returned,reset);fclose(report);}
        }

        if(groupPanel==108 || groupPanel==109) {
            auto& target=g_xGameProc.m_xInterface.m_targetDisplay;
            std::vector<BYTE> jpeg;if(FILE* file=fopen("./portrait-preview/portrait01.jpg","rb")) {fseek(file,0,SEEK_END);jpeg.resize(ftell(file));rewind(file);fread(jpeg.data(),1,jpeg.size(),file);fclose(file);}
            const auto receive=[&](DWORD actor,WORD version,const std::vector<BYTE>& bytes) {
                _TDEFAULTMESSAGE packet={};packet.wIdent=1403;packet.nRecog=actor;packet.wTag=version;
                char encoded[8192]={};fnEncodeMessage(&packet,encoded,sizeof(encoded));
                if(!bytes.empty()) fnEncode6BitBuf(const_cast<BYTE*>(bytes.data()),encoded+DEFBLOCKSIZE,int(bytes.size()),sizeof(encoded)-DEFBLOCKSIZE);
                g_xGameProc.OnSocketMessageRecieve(encoded);
            };
            target.Clear();target.SetHuman(7,1,1,1000);receive(7,1,jpeg);const bool loaded=target.HasPortrait();
            receive(8,0,{});const bool wrong=target.HasPortrait();receive(7,2,{0xff,0xd8,0,0});const bool invalid=target.HasPortrait();
            receive(7,0,jpeg);const bool cleared=!target.HasPortrait();
            auto padded=jpeg;padded.resize(4096);receive(7,1,padded);const bool limit=target.HasPortrait();
            receive(7,0,{});padded.push_back(0);receive(7,1,padded);const bool oversized=!target.HasPortrait();
            target.SetHuman(8,0,2,1100);receive(7,1,jpeg);const bool delayed=!target.HasPortrait();
            target.SetHuman(7,1,1,1200);receive(7,1,jpeg);
            if(groupPanel==109) receive(7,0,{});
            mouse={400,460};g_xGameProc.m_ptMousePos=mouse;capturePath=groupPanel==108?"target-photo-reply.bmp":"target-photo-cleared.bmp";
            if(FILE* report=fopen("target-photo-reply-check.txt","wb")) {fprintf(report,"loaded=%d wrong=%d invalid=%d cleared=%d limit=%d oversized=%d delayed=%d\n",loaded,wrong,invalid,cleared,limit,oversized,delayed);fclose(report);}
        }

        if(groupPanel==110 || groupPanel==111) {
            auto& ui=g_xGameProc.m_xInterface;auto& picker=ui.m_xPortraitWnd;
            std::vector<BYTE> jpeg;if(FILE* file=fopen("./portrait-preview/portrait01.jpg","rb")) {fseek(file,0,SEEK_END);jpeg.resize(ftell(file));rewind(file);fread(jpeg.data(),1,jpeg.size(),file);fclose(file);}
            picker.requested=jpeg;picker.requestPending=true;picker.SendRequest();const bool uploaded=picker.HasOwnPhoto();
            picker.Close();const bool windowClosed=picker.HasOwnPhoto();
            const BYTE bad[]={0xff,0xd8,0,0};const bool invalid=!picker.ApplyOwnPhoto(bad,sizeof(bad)) && picker.HasOwnPhoto();
            picker.requested.clear();picker.requestPending=true;picker.SendRequest();const bool reset=!picker.HasOwnPhoto();
            g_xGameProc.m_xMyHero.m_dwIdentity=77;
            _TDEFAULTMESSAGE packet={};packet.wIdent=1403;packet.nRecog=77;packet.wParam=1;
            char encoded[8192]={};fnEncodeMessage(&packet,encoded,sizeof(encoded));fnEncode6BitBuf(jpeg.data(),encoded+DEFBLOCKSIZE,int(jpeg.size()),sizeof(encoded)-DEFBLOCKSIZE);
            g_xGameProc.OnSocketMessageRecieve(encoded);const bool reply=picker.HasOwnPhoto();
            packet.wParam=0;fnEncodeMessage(&packet,encoded,sizeof(encoded));g_xGameProc.OnSocketMessageRecieve(encoded);const bool cleared=!picker.HasOwnPhoto();
            if(groupPanel==110) picker.ApplyOwnPhoto(jpeg.data(),jpeg.size());
            ui.m_targetDisplay.Clear();mouse={400,460};g_xGameProc.m_ptMousePos=mouse;capturePath=groupPanel==110?"own-photo-uploaded.bmp":"own-photo-default.bmp";
            if(FILE* report=fopen("own-photo-check.txt","wb")) {fprintf(report,"uploaded=%d window_closed=%d invalid=%d reset=%d reply=%d cleared=%d\n",uploaded,windowClosed,invalid,reset,reply,cleared);fclose(report);}
        }

        if(groupPanel==112 || groupPanel==113) {
            auto& picker=g_xGameProc.m_xInterface.m_xPortraitWnd;
            CreateDirectoryA("./preview-photo-cache",nullptr);
            std::vector<BYTE> jpeg;if(FILE* file=fopen("./portrait-preview/portrait01.jpg","rb")) {fseek(file,0,SEEK_END);jpeg.resize(ftell(file));rewind(file);fread(jpeg.data(),1,jpeg.size(),file);fclose(file);}
            picker.SetCacheContext("ServerA","HeroA","./preview-photo-cache","./preview-photo-cache/UserPic.ini");picker.SaveOwnCache({});
            picker.requested=jpeg;picker.requestPending=true;picker.SendRequest();picker.SaveOwnVersion(123);
            const bool restored=picker.SetCacheContext("ServerA","HeroA","./preview-photo-cache","./preview-photo-cache/UserPic.ini") && picker.HasOwnPhoto();
            const bool actor=!picker.SetCacheContext("ServerA","HeroMissing","./preview-photo-cache","./preview-photo-cache/UserPic.ini") && !picker.HasOwnPhoto();
            const bool server=!picker.SetCacheContext("ServerMissing","HeroA","./preview-photo-cache","./preview-photo-cache/UserPic.ini") && !picker.HasOwnPhoto();
            const bool unsafe=!picker.SetCacheContext("../ServerA","HeroA","./preview-photo-cache","./preview-photo-cache/UserPic.ini") && !picker.SaveOwnCache(jpeg);
            picker.SetCacheContext("ServerA","HeroA","./preview-photo-cache","./preview-photo-cache/UserPic.ini");picker.requested.clear();picker.requestPending=true;picker.SendRequest();
            const bool reset=!picker.SetCacheContext("ServerA","HeroA","./preview-photo-cache","./preview-photo-cache/UserPic.ini");
            if(FILE* file=fopen("./preview-photo-cache/ServerA_HeroA.fac","wb")) {fwrite("corrupt",1,7,file);fclose(file);}
            const bool corrupt=!picker.SetCacheContext("ServerA","HeroA","./preview-photo-cache","./preview-photo-cache/UserPic.ini") && !picker.HasOwnPhoto();
            picker.SaveOwnCache(jpeg);const bool repaired=picker.SetCacheContext("ServerA","HeroA","./preview-photo-cache","./preview-photo-cache/UserPic.ini");
            if(groupPanel==113) {picker.SaveOwnCache({});picker.SetCacheContext("ServerA","HeroA","./preview-photo-cache","./preview-photo-cache/UserPic.ini");}
            mouse={400,460};g_xGameProc.m_ptMousePos=mouse;capturePath=groupPanel==112?"own-cache-restored.bmp":"own-cache-default.bmp";
            if(FILE* report=fopen("own-cache-check.txt","wb")) {fprintf(report,"restored=%d actor=%d server=%d unsafe=%d reset=%d corrupt=%d repaired=%d\n",restored,actor,server,unsafe,reset,corrupt,repaired);fclose(report);}
        }

        if(groupPanel==114) {
            auto& picker=g_xGameProc.m_xInterface.m_xPortraitWnd;CreateDirectoryA("./preview-photo-version",nullptr);
            const char* ini="./preview-photo-version/UserPic.ini";picker.SetCacheContext("ServerA","HeroA","./preview-photo-version",ini);picker.SaveOwnVersion(0);
            const auto ack=[&](DWORD version) {_TDEFAULTMESSAGE packet={};packet.wIdent=1402;packet.nRecog=LONG(version);char wire[64]={};fnEncodeMessage(&packet,wire,sizeof(wire));g_xGameProc.OnSocketMessageRecieve(wire);};
            ack(123456);const bool acknowledged=GetPrivateProfileIntA("UserPic","ServerA_HeroA",0,ini)==123456;
            ack(0);const bool zeroIgnored=GetPrivateProfileIntA("UserPic","ServerA_HeroA",0,ini)==123456;
            ack(0xf1234567u);const bool wide=GetPrivateProfileIntA("UserPic","ServerA_HeroA",0,ini)==0xf1234567u;
            picker.SetCacheContext("ServerA","HeroB","./preview-photo-version",ini);picker.SaveOwnVersion(42);
            const bool isolated=GetPrivateProfileIntA("UserPic","ServerA_HeroA",0,ini)==0xf1234567u && GetPrivateProfileIntA("UserPic","ServerA_HeroB",0,ini)==42;
            picker.SetCacheContext("ServerA","HeroA","./preview-photo-version",ini);
            const bool zeroSaved=picker.SaveOwnVersion(0) && GetPrivateProfileIntA("UserPic","ServerA_HeroA",99,ini)==0;
            ack(123456);mouse={400,460};g_xGameProc.m_ptMousePos=mouse;capturePath="portrait-upload-confirmed.bmp";
            if(FILE* report=fopen("portrait-version-check.txt","wb")) {fprintf(report,"acknowledged=%d zero_ignored=%d wide=%d isolated=%d zero_saved=%d\n",acknowledged,zeroIgnored,wide,isolated,zeroSaved);fclose(report);}
        }

        if(groupPanel==115) {
            auto& picker=g_xGameProc.m_xInterface.m_xPortraitWnd;CreateDirectoryA("./preview-dbcs-photo",nullptr);
            char name[32]={};WideCharToMultiByte(CP_ACP,0,L"\x4e57",-1,name,sizeof(name),nullptr,nullptr);
            const std::string filename=std::string(name)+".jpg",path="./preview-dbcs-photo/"+filename;
            CopyFileA("./portrait-preview/portrait01.jpg",path.c_str(),FALSE);
            picker.directory="./preview-dbcs-photo";g_xGameProc.m_xInterface.WindowActivate(_WND_ID_PORTRAIT);
            picker.selected=-1;for(int i=0;i<int(picker.names.size());++i) if(picker.names[i]==filename) picker.selected=i;
            const bool enumerated=picker.selected>=0;picker.lastUpload=0;picker.requestPending=false;picker.Request(1,1000);
            const bool uploaded=picker.requestPending && !picker.requested.empty();
            picker.SetCacheContext(name,name,"./preview-dbcs-photo","./preview-dbcs-photo/UserPic.ini");picker.SendRequest();picker.SaveOwnVersion(123);
            const bool restored=picker.SetCacheContext(name,name,"./preview-dbcs-photo","./preview-dbcs-photo/UserPic.ini") && picker.HasOwnPhoto();
            const bool pathRejected=!picker.SetCacheContext("Bad/Server",name,"./preview-dbcs-photo") && !picker.SaveOwnCache(picker.requested);
            picker.SetCacheContext(name,name,"./preview-dbcs-photo","./preview-dbcs-photo/UserPic.ini");
            if(enumerated) {const POINT row={30,30+24*picker.selected};picker.Pointer(0,row);picker.Pointer(2,row);}
            mouse={400,460};g_xGameProc.m_ptMousePos=mouse;capturePath="portrait-chinese-filename.bmp";
            if(FILE* report=fopen("portrait-dbcs-check.txt","wb")) {fprintf(report,"codepage=%u enumerated=%d uploaded=%d restored=%d path_rejected=%d\n",GetACP(),enumerated,uploaded,restored,pathRejected);fclose(report);}
        }

        if(groupPanel==136 || groupPanel==137) {
            auto& ui=g_xGameProc.m_xInterface;auto& bag=ui.m_xInventoryWnd;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            BYTE raw[100]={};strcpy((char*)raw,"头盔属性测试");raw[30]=15;raw[32]=3;raw[62]=11;raw[66]=0xD0;raw[67]=7;raw[68]=0x10;raw[69]=0x27;
            CItem item;item.SetMir3Item(raw,0);BYTE elements[7]={};const bool omitted=item.AccessoryHint(35,20,elements).size()==4;
            raw[40]=1;raw[41]=3;raw[86]=2;raw[49]=1;raw[50]=2;raw[97]=1;raw[98]=3;raw[52]=1;raw[91]=1;raw[53]=2;raw[92]=2;raw[47]=1;raw[48]=2;raw[89]=1;
            item.SetMir3Item(raw,0);const auto rows=item.AccessoryHint(35,20,elements);
            const bool defense=rows[3].text.find(": 1-5 (+2)")!=std::string::npos && rows[3].color==13172630;
            const bool elementsAdded=rows[4].text.find("+5")!=std::string::npos && rows[5].text.find("x2")!=std::string::npos && rows[6].text.find("x2")!=std::string::npos;
            const bool order=rows.size()==9 && rows[7].text.find(": 1-3 (+1)")!=std::string::npos;
            raw[30]=26;item.SetMir3Item(raw,0);const bool shared=item.AccessoryHint(35,20,elements).size()==rows.size();
            raw[30]=57;item.SetMir3Item(raw,0);const bool variant=item.AccessoryHint(35,20,elements).size()==5;
            raw[30]=groupPanel==136?15:26;item.SetMir3Item(raw,0);bag.AddInvenItem(item,0,TRUE);ui.WindowActivate(_WND_ID_INVENTORY);mouse={550,80};g_xGameProc.m_ptMousePos=mouse;
            capturePath=groupPanel==136?"helmet-hint.bmp":"accessory-26-hint.bmp";
            if(FILE* report=fopen("helmet-hint-check.txt","wb")) {fprintf(report,"omitted=%d defense=%d elements=%d order=%d shared=%d variant=%d\n",omitted,defense,elementsAdded,order,shared,variant);fclose(report);}
        }

        if(groupPanel==134 || groupPanel==135) {
            auto& ui=g_xGameProc.m_xInterface;auto& bag=ui.m_xInventoryWnd;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            BYTE raw[100]={};strcpy((char*)raw,"武器属性测试");raw[30]=5;raw[32]=8;raw[34]=2;raw[44]=5;raw[45]=12;raw[86]=3;raw[47]=1;raw[48]=4;raw[87]=2;raw[41]=102;raw[43]=12;raw[92]=2;raw[40]=1;raw[89]=2;raw[96]=1;raw[49]=1;raw[50]=2;raw[97]=2;raw[98]=3;raw[62]=11;raw[66]=150;raw[68]=0x10;raw[69]=0x27;raw[70]=3;raw[71]=0x30;raw[72]=0x75;
            CItem item;item.SetMir3Item(raw,0);BYTE elements[7]={};const auto lines=item.WeaponHint(35,20,elements);
            const auto has=[&](const std::vector<CItem::HintLine>& rows,const char* text) {for(const auto& row:rows) if(row.text.find(text)!=std::string::npos) return true;return false;};
            const bool title=has(lines,"(*)");const bool attack=has(lines,": 5-15 (+3)");const bool magic=has(lines,": 1-6 (+2)");
            const bool speed=has(lines,": +4 (+-8)");const bool signedAccuracy=has(lines,": -2");const bool experience=has(lines,"16.67 %");
            raw[70]=76;item.SetMir3Item(raw,0);const bool maximum=has(item.WeaponHint(35,20,elements),"MAX");
            raw[30]=54;item.SetMir3Item(raw,0);const auto special=item.WeaponHint(35,20,elements);const bool quantity=special[1].text.find(": 2")!=std::string::npos && !has(special,"MAX") && special[1].color==16777088;
            raw[30]=groupPanel==134?5:54;raw[70]=3;item.SetMir3Item(raw,0);bag.AddInvenItem(item,0,TRUE);ui.WindowActivate(_WND_ID_INVENTORY);mouse={550,80};g_xGameProc.m_ptMousePos=mouse;
            capturePath=groupPanel==134?"weapon-hint.bmp":"weapon-special-hint.bmp";
            if(FILE* report=fopen("weapon-hint-check.txt","wb")) {fprintf(report,"title=%d attack=%d magic=%d speed=%d signed_accuracy=%d experience=%d maximum=%d quantity=%d\n",title,attack,magic,speed,signedAccuracy,experience,maximum,quantity);fclose(report);}
        }

        if(groupPanel==132 || groupPanel==133) {
            auto& ui=g_xGameProc.m_xInterface;auto& bag=ui.m_xInventoryWnd;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            BYTE raw[100]={};strcpy((char*)raw,"防具属性测试");raw[30]=10;raw[32]=12;raw[40]=3;raw[41]=7;raw[42]=2;raw[43]=5;raw[44]=1;raw[45]=3;raw[86]=2;raw[54]=0;raw[55]=40;raw[62]=11;raw[66]=0xE8;raw[67]=3;raw[68]=0x10;raw[69]=0x27;
            CItem item;item.SetMir3Item(raw,0);BYTE elements[7]={10,10,10,10,20,20,20};
            const auto basic=item.ArmorHint(35,20,elements);
            const bool stats=basic.size()==7 && basic[3].text.find(": 1-3 ")!=std::string::npos && basic[4].text.find(": 3-9 (+2)")!=std::string::npos && basic[4].color==13172630;
            const bool low=basic[1].color==3289855 && basic.back().color==255;
            const bool level=item.ArmorHint(40,20,elements).back().color==0xffffff;
            raw[55]=135;item.SetMir3Item(raw,0);const bool rebirth=item.ArmorHint(35,20,elements).back().color==51200;
            raw[54]=2;raw[55]=10;item.SetMir3Item(raw,0);const bool all=item.ArmorHint(35,20,elements).back().color==0xffffff;elements[3]=9;const bool below=item.ArmorHint(35,20,elements).back().color==255;
            raw[54]=0;raw[55]=40;
            if(groupPanel==133) {raw[49]=1;raw[50]=2;raw[52]=1;raw[53]=2;raw[91]=1;raw[92]=2;raw[97]=1;raw[98]=3;raw[94]=2;raw[95]=1;}
            item.SetMir3Item(raw,0);const auto detail=item.ArmorHint(35,20,elements);
            const bool elementRows=groupPanel==132 || (detail.size()==14 && detail[9].text.find("x3")!=std::string::npos && detail[9].color==65408 && detail[10].text.find("x3")!=std::string::npos && detail[10].color==13172630 && detail[12].text.find(": 2-3 (+3)")!=std::string::npos);
            bag.AddInvenItem(item,0,TRUE);ui.WindowActivate(_WND_ID_INVENTORY);mouse={550,80};g_xGameProc.m_ptMousePos=mouse;
            capturePath=groupPanel==132?"armor-hint.bmp":"armor-elements-hint.bmp";
            if(FILE* report=fopen("armor-hint-check.txt","wb")) {fprintf(report,"stats=%d low=%d level=%d rebirth=%d all=%d below=%d elements=%d\n",stats,low,level,rebirth,all,below,elementRows);fclose(report);}
        }

        if(groupPanel>=129 && groupPanel<=131) {
            auto& ui=g_xGameProc.m_xInterface;auto& bag=ui.m_xInventoryWnd;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            BYTE raw[100]={};strcpy((char*)raw,"物品属性测试");raw[32]=1;raw[62]=11;raw[71]=1;raw[30]=4;raw[31]=1;raw[38]=40;
            CItem item;item.SetMir3Item(raw,0);const auto book=item.GeneralHint(1,35);
            const bool level=book.size()==4 && book.back().color==255;
            const bool allowed=item.GeneralHint(1,40).back().color==0xffffff;
            const bool profession=item.GeneralHint(0,40).back().color==255;
            raw[30]=25;raw[66]=150;raw[68]=138;raw[69]=2;item.SetMir3Item(raw,0);const auto charm=item.GeneralHint(1,35);
            const bool uses=charm.back().text.find(": 2/7")!=std::string::npos && charm.back().color==16777088;
            raw[30]=30;item.SetMir3Item(raw,0);const bool low=item.GeneralHint(1,35).back().color==3289855;
            raw[30]=60;raw[40]=2;raw[41]=5;raw[86]=3;item.SetMir3Item(raw,0);const auto stone=item.GeneralHint(1,35);
            const bool bonus=stone.back().text.find(": 2-8 (+3)")!=std::string::npos && stone.back().color==13172630;
            raw[86]=255;item.SetMir3Item(raw,0);const bool signedBonus=item.GeneralHint(1,35).back().text.find(": 2-5")!=std::string::npos && item.GeneralHint(1,35).back().color==0xffffff;
            raw[30]=groupPanel==129?4:groupPanel==130?25:60;raw[86]=3;item.SetMir3Item(raw,0);bag.AddInvenItem(item,0,TRUE);
            ui.WindowActivate(_WND_ID_INVENTORY);mouse={550,80};g_xGameProc.m_ptMousePos=mouse;
            capturePath=groupPanel==129?"book-hint.bmp":groupPanel==130?"charm-hint.bmp":"stone-hint.bmp";
            if(FILE* report=fopen("general-item-hint-check.txt","wb")) {fprintf(report,"level=%d allowed=%d profession=%d uses=%d low=%d bonus=%d signed_bonus=%d\n",level,allowed,profession,uses,low,bonus,signedBonus);fclose(report);}
        }

        if(groupPanel==127 || groupPanel==128) {
            auto& ui=g_xGameProc.m_xInterface;auto& bag=ui.m_xInventoryWnd;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            BYTE raw[100]={};strcpy((char*)raw,"金创药（小量）");raw[32]=3;raw[40]=50;raw[42]=30;raw[62]=11;raw[71]=1;
            CItem item;item.SetMir3Item(raw,0);const auto single=item.GeneralHint(1,35);
            const bool ordinary=single.size()==4 && single[0].color==64255 && single[1].text.find(": 3")!=std::string::npos && single[2].text.find("+50HP")!=std::string::npos && single[3].text.find("+30MP")!=std::string::npos;
            raw[61]=2;raw[71]=25;CItem stack;stack.SetMir3Item(raw,0);const auto multiple=stack.GeneralHint(1,35);
            const bool quantity=multiple.size()==5 && multiple[1].text.find(": 2")!=std::string::npos && multiple[2].text.find(": 25")!=std::string::npos;
            raw[61]=3;stack.SetMir3Item(raw,0);const bool weight=stack.GeneralHint(1,35)[1].text.find(": 75")!=std::string::npos;
            raw[61]=2;stack.SetMir3Item(raw,0);bag.AddInvenItem(groupPanel==127?item:stack,0,TRUE);
            ui.WindowActivate(_WND_ID_INVENTORY);mouse={550,80};g_xGameProc.m_ptMousePos=mouse;
            capturePath=groupPanel==127?"potion-hint.bmp":"potion-stack-hint.bmp";
            if(FILE* report=fopen("potion-hint-check.txt","wb")) {fprintf(report,"ordinary=%d quantity=%d weight=%d\n",ordinary,quantity,weight);fclose(report);}
        }

        if(groupPanel==126) {
            auto& ui=g_xGameProc.m_xInterface;auto& bag=ui.m_xInventoryWnd;auto& quick=ui.m_xQuickWnd;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            BYTE raw[100]={};memcpy(raw,"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234",30);raw[31]=106;raw[61]=2;raw[62]=11;raw[71]=5;
            CItem first,second;first.SetMir3Item(raw,0);raw[29]='5';raw[62]=22;raw[71]=7;second.SetMir3Item(raw,1);
            bag.AddInvenItem(first,0,TRUE);bag.AddInvenItem(second,1,TRUE);quick.slots[0]=CQuickWnd::FromItem(first);
            const bool saved=bag.SaveLayoutFile("./preview-item-layout-full.itms");
            BYTE stored[100]={};if(FILE* file=fopen("./preview-item-layout-full.itms","rb")) {fseek(file,16,SEEK_SET);fread(stored,1,100,file);fclose(file);}
            const bool record=!memcmp(stored,first.m_serverRecord,100);
            bag.DeleteAllInvenItem();quick.slots[0]=CQuickWnd::Slot{};first.SetCount(99);
            bag.AddInvenItem(second,0,TRUE);bag.AddInvenItem(first,1,TRUE);
            const bool restored=bag.LoadLayoutFile("./preview-item-layout-full.itms");
            const bool positions=bag.ItemAnchor(0)==1 && bag.ItemAnchor(1)==0;
            const bool reference=quick.slots[0].present && quick.slots[0].item.m_stItemInfo.nMakeIndex==11 && quick.slots[0].count==99 && strlen(quick.slots[0].item.Name())==30;
            const bool current=bag.m_stInventoryItem[1].xItem.m_count==99 && bag.m_stInventoryItem[1].xItem.m_serverRecord[71]==99;
            ui.WindowActivate(_WND_ID_INVENTORY);ui.WindowActivate(_WND_ID_QUICK);capturePath="item-layout-full-name.bmp";
            if(FILE* report=fopen("item-layout-full-name-check.txt","wb")) {fprintf(report,"saved=%d record=%d restored=%d positions=%d reference=%d current=%d\n",saved,record,restored,positions,reference,current);fclose(report);}
        }

        if(groupPanel==125) {
            auto& ui=g_xGameProc.m_xInterface;auto& equipment=ui.m_xStatusWnd.m_stEquipItem;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            BYTE raw[100]={};strcpy((char*)raw,"Durability Preview");raw[30]=25;raw[62]=11;
            equipment[9].xItem.SetMir3Item(raw,9);equipment[9].bSetted=TRUE;
            const auto receive=[&](LONG value,WORD slot=9,WORD opcode=SM_DURACHANGE) {
                _TDEFAULTMESSAGE packet={};packet.wIdent=opcode;packet.nRecog=value;packet.wParam=slot;packet.wTag=2000;packet.wSeries=99;
                char encoded[64]={};fnEncodeMessage(&packet,encoded,sizeof(encoded));g_xGameProc.OnSocketMessageRecieve(encoded);
            };
            auto& item=equipment[9].xItem;const auto before=ui.m_chatHistory.size();receive(500);
            const bool threshold=ui.m_chatHistory.size()==before && item.m_stItemInfo.nDura==500;
            receive(499);const bool warning=ui.m_chatHistory.size()==before+1 && ui.m_chatHistory.back().color==0xffffff && ui.m_chatHistory.back().background==33023;
            const bool rawBytes=item.m_serverRecord[66]==243 && item.m_serverRecord[67]==1 && item.m_serverRecord[68]==208 && item.m_serverRecord[69]==7;
            item.m_stItemInfo.stStdItem.bStdMode=54;receive(50);const bool unit=ui.m_chatHistory.size()==before+1;receive(49);const bool special=ui.m_chatHistory.size()==before+2;
            receive(-1);const bool negative=item.m_stItemInfo.nDura==65535 && ui.m_chatHistory.size()==before+3;
            receive(65536);const bool wide=item.m_stItemInfo.nDura==0 && ui.m_chatHistory.size()==before+3;
            receive(0,12);receive(0,8);const bool ignored=ui.m_chatHistory.size()==before+3 && !equipment[8].bSetted;
            receive(1234,9,SM_DROPITEM_FAIL);const bool drop=item.m_stItemInfo.nDura==0 && ui.m_chatHistory.size()==before+3;
            ui.WindowActivate(_WND_ID_STATUS);capturePath="equipment-durability.bmp";
            if(FILE* report=fopen("equipment-durability-check.txt","wb")) {fprintf(report,"threshold=%d warning=%d raw=%d unit=%d special=%d negative=%d wide=%d ignored=%d drop=%d\n",threshold,warning,rawBytes,unit,special,negative,wide,ignored,drop);fclose(report);}
        }

        if(groupPanel>=PreviewVitalAccessory && groupPanel<=PreviewAccuracyBracelet) {
            using namespace Mir3AccessoryHint;
            auto& ui=g_xGameProc.m_xInterface;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            const int modes[]={VitalNecklace,DefenseRing,RecoveryRing,AccuracyBracelet};
            const char* paths[]={"vital-accessory-hint.bmp","defense-accessory-hint.bmp","recovery-accessory-hint.bmp","accuracy-bracelet-hint.bmp"};
            const int selected=groupPanel-PreviewVitalAccessory;
            BYTE raw[sizeof(CItem().m_serverRecord)]={};strcpy((char*)raw,"饰品属性测试");
            raw[ModeOffset]=modes[selected];raw[DefenseOffset]=1;raw[FirstScalarOffset]=2;
            raw[MagicDefenseOffset]=1;raw[SecondScalarOffset]=2;raw[FirstBonusOffset]=1;raw[SecondBonusOffset]=1;
            raw[AttackElementOffset]=1;raw[AttackElementAmountOffset]=2;raw[AttackElementBonusKindOffset]=1;raw[AttackElementBonusAmountOffset]=3;
            raw[ExtraElementOffset]=1;raw[ExtraElementAmountOffset]=2;raw[StrongElementOffset]=1;raw[StrongElementOffset+1]=2;
            CItem item;item.SetMir3Item(raw,0);BYTE elements[ElementCount]={};const auto rows=item.AccessoryHint(35,20,elements);
            const auto has=[&](const std::vector<CItem::HintLine>& lines,const char* text,unsigned color) {for(const auto& line:lines) if(line.text.find(text)!=std::string::npos && line.color==color) return true;return false;};
            const bool scalar=selected==1?has(rows,": 1-3 (+1)",BonusColor):has(rows,selected==3?": +3 (+1)":": +30% (+10%)",BonusColor);
            const bool elemental=has(rows,"+5",ElementColor) && has(rows,selected==3?"x2":"x1",ElementColor) && has(rows,selected==3?"x2":"x1",BonusColor);
            const bool speed=(selected!=0 && selected!=2) || (has(rows,": +1 ",NormalColor) && has(rows,": -1 ",NormalColor));
            raw[FirstBonusOffset]=255;raw[SecondBonusOffset]=254;item.SetMir3Item(raw,0);const auto negative=item.AccessoryHint(35,20,elements);
            const bool signedBonus=selected==1?has(negative,": 1-2",NormalColor):has(negative,selected==3?": +2":": +20%",NormalColor);
            const bool trailing=selected!=2 || has(negative,": +20% ",NormalColor);
            raw[FirstBonusOffset]=1;raw[SecondBonusOffset]=1;item.SetMir3Item(raw,0);
            ui.m_xInventoryWnd.AddInvenItem(item,0,TRUE);ui.WindowActivate(_WND_ID_INVENTORY);mouse={550,80};g_xGameProc.m_ptMousePos=mouse;capturePath=paths[selected];
            const std::string reportPath=std::string(paths[selected])+".txt";
            if(FILE* report=fopen(reportPath.c_str(),"wb")) {fprintf(report,"scalar=%d elements=%d speed=%d signed=%d trailing=%d\n",scalar,elemental,speed,signedBonus,trailing);fclose(report);}
        }

        if(groupPanel==139 || groupPanel==140) {
            auto& ui=g_xGameProc.m_xInterface;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            BYTE raw[100]={};strcpy((char*)raw,"项链属性测试");raw[30]=groupPanel==140?20:19;raw[32]=2;raw[62]=11;
            raw[66]=0xD0;raw[67]=7;raw[68]=0x10;raw[69]=0x27;
            raw[41]=2;raw[86]=1;raw[42]=1;raw[43]=2;raw[87]=1;
            raw[49]=1;raw[50]=2;raw[97]=1;raw[98]=3;raw[91]=2;raw[92]=4;raw[52]=1;raw[53]=2;
            const bool accuracy=groupPanel==140;
            CItem item;item.SetMir3Item(raw,0);BYTE elements[7]={};auto rows=item.AccessoryHint(35,20,elements);
            const bool stats=rows.size()==(accuracy?9:10) && rows[3].text.find(accuracy?"+3 (+1)":"+30% (+10%)")!=std::string::npos && rows[3].color==13172630 && rows[accuracy?4:5].text.find("+3 (+1)")!=std::string::npos;
            const bool elemental=rows[accuracy?5:6].text.find("+5")!=std::string::npos && rows[accuracy?5:6].text.find("+4")!=std::string::npos && rows[accuracy?6:7].text.find("x1")!=std::string::npos && rows[accuracy?7:8].color==13172630;
            raw[86]=255;raw[87]=254;raw[52]=7;raw[53]=7;item.SetMir3Item(raw,0);rows=item.AccessoryHint(35,20,elements);
            const bool boundary=rows.size()==(accuracy?6:7) && rows[3].text.find(accuracy?"+2":"+20%")!=std::string::npos && rows[3].color==0xffffff;
            raw[86]=1;raw[87]=1;raw[52]=1;raw[53]=2;item.SetMir3Item(raw,0);ui.m_xInventoryWnd.AddInvenItem(item,0,TRUE);ui.WindowActivate(_WND_ID_INVENTORY);
            mouse={550,80};g_xGameProc.m_ptMousePos=mouse;capturePath=accuracy?"accuracy-necklace-hint.bmp":"necklace-hint.bmp";
            if(FILE* report=fopen(accuracy?"accuracy-necklace-hint-check.txt":"necklace-hint-check.txt","wb")) {fprintf(report,"stats=%d elements=%d boundary=%d\n",stats,elemental,boundary);fclose(report);}
        }

        if(groupPanel==138) {
            auto& ui=g_xGameProc.m_xInterface;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            BYTE raw[100]={};strcpy((char*)raw,"装备属性测试");raw[30]=26;raw[32]=3;raw[62]=11;
            raw[66]=0xD0;raw[67]=7;raw[68]=0x10;raw[69]=0x27;raw[40]=1;raw[41]=3;raw[86]=2;
            ui.m_xStatusWnd.m_stEquipItem[5].xItem.SetMir3Item(raw,5);
            ui.m_xStatusWnd.m_stEquipItem[5].bSetted=TRUE;
            ui.WindowActivate(_WND_ID_STATUS);mouse={20,165};g_xGameProc.m_ptMousePos=mouse;
            capturePath="equipment-hover-hint.bmp";
        }

        if(groupPanel==124) {
            auto& ui=g_xGameProc.m_xInterface;auto& equipment=ui.m_xStatusWnd.m_stEquipItem;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            BYTE raw[100]={};memcpy(raw,"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234",30);raw[30]=25;raw[62]=11;raw[71]=5;raw[66]=0xE8;raw[67]=3;
            char block[140]={};fnEncode6BitBuf(raw,block,100,sizeof(block));const std::string first="9/"+std::string(block)+"/";
            raw[62]=22;fnEncode6BitBuf(raw,block,100,sizeof(block));const std::string second="11/"+std::string(block)+"/";
            const auto receive=[&](const std::string& body) {
                _TDEFAULTMESSAGE packet={};packet.wIdent=SM_SENDUSEITEMS;std::vector<char> encoded(DEFBLOCKSIZE+body.size()+1);
                fnEncodeMessage(&packet,encoded.data(),int(encoded.size()));strcpy(encoded.data()+DEFBLOCKSIZE,body.c_str());g_xGameProc.OnSocketMessageRecieve(encoded.data());
            };
            receive(first+second);const bool loaded=equipment[9].bSetted && equipment[11].bSetted && equipment[9].xItem.m_stItemInfo.nMakeIndex==11 && equipment[11].xItem.m_shCellNum==11;
            const bool fullName=strlen(equipment[9].xItem.Name())==30 && equipment[9].xItem.m_serverRecord[71]==5;
            receive("5/"+std::string(block)+"/12/"+block+"/");const bool invalid=!equipment[5].bSetted && equipment[9].bSetted;
            receive("5/"+std::string(block));const bool truncated=!equipment[5].bSetted;
            receive("");const bool empty=equipment[9].bSetted && equipment[11].bSetted;
            receive("5x/"+std::string(block)+"/");const bool slot=!equipment[5].bSetted;
            receive("5/"+std::string(block)+"/");const bool partial=equipment[5].bSetted && equipment[9].bSetted && equipment[11].bSetted;
            ui.WindowActivate(_WND_ID_STATUS);capturePath="equipment-records-reply.bmp";
            if(FILE* report=fopen("equipment-records-check.txt","wb")) {fprintf(report,"loaded=%d full_name=%d invalid=%d truncated=%d empty=%d slot=%d partial=%d\n",loaded,fullName,invalid,truncated,empty,slot,partial);fclose(report);}
        }

        if(groupPanel==123) {
            auto& ui=g_xGameProc.m_xInterface;auto& bag=ui.m_xInventoryWnd;auto& equipment=ui.m_xStatusWnd.m_stEquipItem;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            BYTE raw[100]={};memcpy(raw,"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234",30);raw[61]=2;raw[62]=11;raw[71]=12;
            const auto receive=[&](WORD opcode) {
                _TDEFAULTMESSAGE packet={};packet.wIdent=opcode;char encoded[160]={};fnEncodeMessage(&packet,encoded,sizeof(encoded));fnEncode6BitBuf(raw,encoded+DEFBLOCKSIZE,100,sizeof(encoded)-DEFBLOCKSIZE);g_xGameProc.OnSocketMessageRecieve(encoded);
            };
            receive(SM_ADDITEM);raw[71]=35;receive(203);
            const bool updated=bag.m_stInventoryItem[0].xItem.m_count==35 && !bag.m_stInventoryItem[1].bSetted;
            raw[36]=raw[37]=255;receive(203);const bool invalid=bag.m_stInventoryItem[0].xItem.m_count==35;raw[36]=raw[37]=0;
            raw[62]=99;receive(203);const bool unknown=!bag.m_stInventoryItem[1].bSetted;raw[62]=11;
            raw[29]='5';receive(SM_DELITEM);const bool name=bag.m_stInventoryItem[0].bSetted;raw[29]='4';
            equipment[5].bSetted=TRUE;equipment[5].xItem=bag.m_stInventoryItem[0].xItem;
            raw[71]=50;receive(203);const bool equipped=equipment[0].bSetted && equipment[0].xItem.m_count==50 && !equipment[5].bSetted;
            receive(SM_DELITEM);const bool deleted=!bag.m_stInventoryItem[0].bSetted && !equipment[0].bSetted && bag.ItemAnchor(0)==-1;
            receive(SM_ADDITEM);raw[71]=60;receive(203);ui.WindowActivate(_WND_ID_INVENTORY);capturePath="item-update-reply.bmp";
            if(FILE* report=fopen("item-update-check.txt","wb")) {fprintf(report,"updated=%d invalid=%d unknown=%d name=%d equipped=%d deleted=%d\n",updated,invalid,unknown,name,equipped,deleted);fclose(report);}
        }

        if(groupPanel==122) {
            auto& ui=g_xGameProc.m_xInterface;auto& bag=ui.m_xInventoryWnd;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            BYTE raw[100]={};strcpy((char*)raw,"Potion");raw[61]=2;raw[62]=11;raw[71]=12;
            char block[140]={};fnEncode6BitBuf(raw,block,100,sizeof(block));std::string first=std::string(block)+"/";
            raw[62]=22;raw[71]=34;strcpy((char*)raw,"Second Potion");fnEncode6BitBuf(raw,block,100,sizeof(block));const std::string full=first+block+"/";
            const auto receive=[&](const std::string& body,WORD count) {
                _TDEFAULTMESSAGE packet={};packet.wIdent=SM_BAGITEMS;packet.wSeries=count;
                std::vector<char> encoded(DEFBLOCKSIZE+body.size()+1);fnEncodeMessage(&packet,encoded.data(),int(encoded.size()));strcpy(encoded.data()+DEFBLOCKSIZE,body.c_str());
                g_xGameProc.OnSocketMessageRecieve(encoded.data());
            };
            receive(full,2);const bool loaded=bag.m_stInventoryItem[0].xItem.m_count==12 && bag.m_stInventoryItem[1].xItem.m_count==34 && bag.ItemAnchor(0)!=bag.ItemAnchor(1);
            receive(first,2);const bool incomplete=bag.m_stInventoryItem[1].xItem.m_count==34;
            receive(full.substr(0,full.size()-1),2);const bool delimiter=bag.m_stInventoryItem[1].bSetted;
            receive(full,47);const bool capacity=bag.m_stInventoryItem[1].bSetted;
            receive(first,1);const bool replaced=bag.m_stInventoryItem[0].bSetted && !bag.m_stInventoryItem[1].bSetted;
            receive("",0);const bool cleared=!bag.m_stInventoryItem[0].bSetted && bag.ItemAnchor(0)==-1;
            receive(full,2);ui.WindowActivate(_WND_ID_INVENTORY);capturePath="bag-records-reply.bmp";
            if(FILE* report=fopen("bag-records-check.txt","wb")) {fprintf(report,"loaded=%d incomplete=%d delimiter=%d capacity=%d replaced=%d cleared=%d\n",loaded,incomplete,delimiter,capacity,replaced,cleared);fclose(report);}
        }

        if(groupPanel==121) {
            auto& ui=g_xGameProc.m_xInterface;auto& bag=ui.m_xInventoryWnd;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            BYTE raw[100]={};memcpy(raw,"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234",30);raw[31]=106;raw[61]=2;raw[62]=0x34;raw[63]=0x12;raw[66]=0xE8;raw[67]=3;raw[68]=0xD0;raw[69]=7;raw[71]=25;
            const auto receive=[&](int length) {
                _TDEFAULTMESSAGE packet={};packet.wIdent=SM_ADDITEM;
                char encoded[256]={};fnEncodeMessage(&packet,encoded,sizeof(encoded));fnEncode6BitBuf(raw,encoded+DEFBLOCKSIZE,length,sizeof(encoded)-DEFBLOCKSIZE);
                g_xGameProc.OnSocketMessageRecieve(encoded);
            };
            receive(99);const bool truncated=!bag.m_stInventoryItem[0].bSetted;
            receive(100);auto& item=bag.m_stInventoryItem[0].xItem;
            const bool decoded=bag.m_stInventoryItem[0].bSetted && item.m_stItemInfo.nMakeIndex==0x1234 && item.m_count==25 && item.m_stackMode==2;
            const bool fullName=strlen(item.Name())==30 && !memcmp(item.Name(),raw,30);
            const bool retained=!memcmp(item.m_serverRecord,raw,100) && item.m_stItemInfo.nDura==1000 && item.m_stItemInfo.nDuraMax==2000;
            ui.m_xQuickWnd.slots[0]=CQuickWnd::FromItem(item);bag.UpdateStackCount(0x1234,300);
            const bool count=item.m_serverRecord[71]==44 && item.m_serverRecord[72]==1 && ui.m_xQuickWnd.slots[0].item.m_serverRecord[72]==1;
            const bool quick=ui.m_xQuickWnd.Find("ABCDEFGHIJKLMNOPQRSTUVWXYZ1234")==0;
            raw[0]=0;receive(100);const bool empty=!bag.m_stInventoryItem[1].bSetted;
            ui.WindowActivate(_WND_ID_INVENTORY);ui.WindowActivate(_WND_ID_QUICK);capturePath="item-record-reply.bmp";
            if(FILE* report=fopen("item-record-check.txt","wb")) {fprintf(report,"truncated=%d decoded=%d full_name=%d retained=%d count=%d quick=%d empty=%d\n",truncated,decoded,fullName,retained,count,quick,empty);fclose(report);}
        }

        if(groupPanel==120) {
            auto& ui=g_xGameProc.m_xInterface;auto& bag=ui.m_xInventoryWnd;auto& quick=ui.m_xQuickWnd;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            CLIENTITEMRCD record={};record.nMakeIndex=1001;strcpy(record.stStdItem.szName,"Potion");
            CItem item;item.SetItem(record,0);item.m_count=10;item.m_stackMode=2;bag.AddInvenItem(item,0,TRUE);quick.slots[0]=CQuickWnd::FromItem(item);
            const auto receive=[&](DWORD identity,WORD count) {
                _TDEFAULTMESSAGE packet={};packet.wIdent=1313;packet.nRecog=identity;packet.wParam=count;
                char encoded[64]={};fnEncodeMessage(&packet,encoded,sizeof(encoded));g_xGameProc.OnSocketMessageRecieve(encoded);
            };
            const auto before=ui.m_chatHistory.size();receive(1001,25);
            const bool increase=bag.m_stInventoryItem[0].xItem.m_count==25 && quick.slots[0].count==25 && ui.m_chatHistory.size()==before+1;
            receive(1001,5);const bool decrease=bag.m_stInventoryItem[0].xItem.m_count==5 && quick.slots[0].item.m_count==5 && ui.m_chatHistory.size()==before+1;
            receive(9999,99);const bool unknown=bag.m_stInventoryItem[0].xItem.m_count==5;
            receive(1001,0);const bool zero=bag.m_stInventoryItem[0].bSetted && quick.slots[0].present && quick.slots[0].count==0;
            bag.SetCommonFromInven(0,&ui.m_stCommonItem);receive(1001,7);const bool held=ui.m_stCommonItem.xItem.m_count==7;
            bag.SetInvenFromCommon(0,&ui.m_stCommonItem);receive(1001,30);
            ui.WindowActivate(_WND_ID_INVENTORY);ui.WindowActivate(_WND_ID_QUICK);
            capturePath="item-stack-reply.bmp";
            if(FILE* report=fopen("item-stack-check.txt","wb")) {fprintf(report,"increase=%d decrease=%d unknown=%d zero=%d held=%d\n",increase,decrease,unknown,zero,held);fclose(report);}
        }

        if(groupPanel==119) {
            auto& ui=g_xGameProc.m_xInterface;
            ui.m_stCommonItem.bSetted=TRUE;ui.m_stCommonItem.bIsBeltItem=FALSE;
            ui.m_stCommonItem.xItem.m_stItemInfo.stStdItem.bStdMode=1;
            CLoginMessages captions;captions.Load("./CmList.dat");
            const std::string itemName=captions.Get(1409)+"3";
            lstrcpynA(ui.m_stCommonItem.xItem.m_stItemInfo.stStdItem.szName,itemName.c_str(),sizeof(ui.m_stCommonItem.xItem.m_stItemInfo.stStdItem.szName));
            const auto before=ui.m_chatHistory.size();
            _TDEFAULTMESSAGE packet={};packet.wIdent=SM_EAT_OK;
            char encoded[64]={};fnEncodeMessage(&packet,encoded,sizeof(encoded));
            g_xGameProc.OnSocketMessageRecieve(encoded);
            const bool consumed=!ui.m_stCommonItem.bSetted;
            std::string expected=captions.Get(1464);const auto at=expected.find("%s");if(at!=std::string::npos) expected.replace(at,2,captions.Get(1409));
            const bool message=ui.m_chatHistory.size()==before+1 && ui.m_chatHistory.back().text==expected;
            const bool colors=ui.m_chatHistory.back().color==0xffffff && ui.m_chatHistory.back().background==33023;
            g_xGameProc.OnSocketMessageRecieve(encoded);
            ui.WindowActivate(_WND_ID_INVENTORY);
            capturePath="item-use-confirmed.bmp";
            if(FILE* report=fopen("item-use-check.txt","wb")) {fprintf(report,"consumed=%d duplicate_empty=%d message=%d colors=%d duplicate_quiet=%d\n",consumed,!ui.m_stCommonItem.bSetted,message,colors,ui.m_chatHistory.size()==before+1);fclose(report);}
        }

        if(groupPanel==118) {
            hero.m_dwIdentity=123;hero.m_stFeature.bGender=0;hero.m_bCurrMtn=_MT_STAND;
            hero.m_bIsMon=FALSE;hero.m_bIsDead=FALSE;hero.m_dwLastStruckTime=0;hero.m_dwPortraitVersion=77;
            BYTE body[16]={0,1,1,BYTE((2<<6)|5),0,0,0,0,42};
            const auto receive=[&](DWORD identity,int length) {
                _TDEFAULTMESSAGE packet={};packet.wIdent=1309;packet.nRecog=identity;packet.wParam=275;packet.wTag=400;packet.wSeries=45;
                char encoded[128]={};fnEncodeMessage(&packet,encoded,sizeof(encoded));
                fnEncode6BitBuf(body,encoded+DEFBLOCKSIZE,length,sizeof(encoded)-DEFBLOCKSIZE);
                g_xGameProc.OnSocketMessageRecieve(encoded);
            };
            receive(456,16);const bool unknown=g_xGameProc.m_xWaitPacketQueue.GetCount()==0;
            receive(123,15);const bool truncated=g_xGameProc.m_xWaitPacketQueue.GetCount()==0;
            receive(123,16);g_xGameProc.ProcessPacket();const bool queued=hero.m_xPacketQueue.GetCount()==1;
            hero.UpdatePacketState();
            const bool health=hero.m_wHP==275 && hero.m_wMAXHP==400 && hero.m_stAbility.wHP==275;
            const bool appearance=hero.m_bProfession==2 && hero.m_stFeature.bHair==5 && hero.m_stFeature.bDress==1;
            const bool photo=hero.m_dwPortraitVersion==77;
            const bool motion=hero.m_bCurrMtn==_MT_HITTED;
            hero.m_szName[0]=0;g_xGameProc.UpdateTargetDisplay(&hero,GetTickCount());
            const bool portrait=g_xGameProc.m_xInterface.m_targetDisplay.Frame()==654;
            hero.m_dwLastStruckTime=timeGetTime();receive(123,16);g_xGameProc.ProcessPacket();
            const bool repeated=hero.m_xPacketQueue.GetCount()==1;
            hero.m_bCurrMtn=_MT_STAND;hero.UpdatePacketState();
            const auto ordinary=[&](WORD hp) {
                _TDEFAULTMESSAGE packet={};packet.wIdent=SM_STRUCK;packet.nRecog=123;packet.wParam=hp;packet.wTag=400;packet.wSeries=25;
                char encoded[128]={};fnEncodeMessage(&packet,encoded,sizeof(encoded));
                fnEncode6BitBuf(body,encoded+DEFBLOCKSIZE,16,sizeof(encoded)-DEFBLOCKSIZE);
                hero.m_dwLastStruckTime=timeGetTime();g_xGameProc.OnSocketMessageRecieve(encoded);g_xGameProc.ProcessPacket();
            };
            ordinary(250);const bool throttled=hero.m_xPacketQueue.GetCount()==0 && hero.m_wHP==250 && hero.m_stAbility.wHP==250;
            ordinary(0);const bool fatal=hero.m_xPacketQueue.GetCount()==1 && hero.m_stAbility.wHP==0;
            delete reinterpret_cast<PACKETMSG*>(hero.m_xPacketQueue.PopQ());
            hero.m_stAbility.wHP=hero.m_wHP=250;hero.m_bHPPercent=62;g_xGameProc.UpdateTargetDisplay(&hero,GetTickCount());

            mouse={400,460};g_xGameProc.m_ptMousePos=mouse;capturePath="human-hit-server.bmp";
            if(FILE* report=fopen("human-hit-check.txt","wb")) {fprintf(report,"unknown=%d truncated=%d queued=%d health=%d appearance=%d photo=%d motion=%d portrait=%d repeated=%d throttled=%d fatal=%d\n",unknown,truncated,queued,health,appearance,photo,motion,portrait,repeated,throttled,fatal);fclose(report);}
        }

        if(groupPanel==116 || groupPanel==117) {
            auto& picker=g_xGameProc.m_xInterface.m_xPortraitWnd;CreateDirectoryA("./preview-photo-restore",nullptr);
            const char* root="./preview-photo-restore";const char* ini="./preview-photo-restore/UserPic.ini";
            std::vector<BYTE> jpeg;if(FILE* file=fopen("./portrait-preview/portrait01.jpg","rb")) {fseek(file,0,SEEK_END);jpeg.resize(ftell(file));rewind(file);fread(jpeg.data(),1,jpeg.size(),file);fclose(file);}
            picker.SetCacheContext("ServerA","HeroA",root,ini);picker.SaveOwnCache(jpeg);picker.SaveOwnVersion(0);
            const bool unconfirmed=!picker.SetCacheContext("ServerA","HeroA",root,ini) && !picker.HasOwnPhoto();
            picker.SaveOwnVersion(123);const bool confirmed=picker.SetCacheContext("ServerA","HeroA",root,ini) && picker.HasOwnPhoto();
            const bool mismatch=!picker.RestoreOwnVersion(124) && !picker.HasOwnPhoto();
            const bool matched=picker.RestoreOwnVersion(123) && picker.HasOwnPhoto();
            const bool zero=!picker.RestoreOwnVersion(0) && !picker.HasOwnPhoto();
            if(groupPanel==116) picker.RestoreOwnVersion(123);
            mouse={400,460};g_xGameProc.m_ptMousePos=mouse;capturePath=groupPanel==116?"portrait-cache-version-match.bmp":"portrait-cache-version-zero.bmp";
            if(FILE* report=fopen("portrait-restore-version-check.txt","wb")) {fprintf(report,"unconfirmed=%d confirmed=%d mismatch=%d matched=%d zero=%d\n",unconfirmed,confirmed,mismatch,matched,zero);fclose(report);}
        }

    }
    void Render(int elapsed) {
        RECT canvas = {0,0,800,600};
        DrawLegacyPanel(canvas,RGB(20,24,28),255,0,false);
        g_xGameProc.m_xInterface.RenderInterface(elapsed,mouse);
        if(damagePreview) {
            auto& actor=g_xGameProc.m_xMyHero;
            const int amounts[]={-9,-99,-100,-65535,25,0};
            for(int i=0;i<6;++i) {
                actor.m_damage=CDamageNumbers{};actor.m_shScrnPosX=30+i*125;actor.m_shScrnPosY=55;
                actor.AddDamage(amounts[i]);actor.DrawDamage();
            }
        }
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
                const HWND edits[]={g_xChatEditBox.GetSafehWnd(),g_xGameProc.m_xInterface.m_xGuildEditor.edit};
                for (HWND edit:edits) if (IsWindowVisible(edit)) {
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
        case WM_LBUTTONDBLCLK:
            mouse.x = (short)LOWORD(lp); mouse.y = (short)HIWORD(lp);
            ui.OnLButtonDoubleClick(mouse); return 0;
        case WM_LBUTTONDOWN:
            mouse.x = (short)LOWORD(lp); mouse.y = (short)HIWORD(lp);
            SetCapture(window); ui.OnLButtonDown(mouse); return 0;
        case WM_LBUTTONUP:
            mouse.x = (short)LOWORD(lp); mouse.y = (short)HIWORD(lp);
            ui.OnLButtonUp(mouse); ReleaseCapture(); return 0;
        case WM_KEYDOWN:
            ui.OnKeyDown(wp,lp); return 0;
        case WM_MOUSEWHEEL:
            if ((short)HIWORD(wp)>0) ui.OnScrollDown();
            else if ((short)HIWORD(wp)<0) ui.OnScrollUp();
            return 0;
        case WM_COMMAND:
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
    g_xGameProc.m_xInterface.m_minimap.LoadOptions(strstr(lpCmdLine,"--preview=") ? nullptr : "./Config.ini");
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
        strcmp(lpCmdLine,"--preview=status-and-skills")==0 ? 22 :
        strcmp(lpCmdLine,"--preview=inventory-clip-top")==0 ? 23 :
        strcmp(lpCmdLine,"--preview=inventory-clip-bottom")==0 ? 24 :
        strcmp(lpCmdLine,"--preview=skill-key-menu")==0 ? 25 :
        strcmp(lpCmdLine,"--preview=skill-description")==0 ? 26 :
        strcmp(lpCmdLine,"--preview=skill-shortcuts")==0 ? 27 :
        strcmp(lpCmdLine,"--preview=quest-empty")==0 ? 28 :
        strcmp(lpCmdLine,"--preview=quest-list")==0 ? 29 :
        strcmp(lpCmdLine,"--preview=mount-panel")==0 ? 30 :
        strcmp(lpCmdLine,"--preview=mount-hint")==0 ? 31 :
        strcmp(lpCmdLine,"--preview=trade-empty")==0 ? 32 :
        strcmp(lpCmdLine,"--preview=trade-list")==0 ? 33 :
        strcmp(lpCmdLine,"--preview=trade-confirmed")==0 ? 34 :
        strcmp(lpCmdLine,"--preview=trade-gold")==0 ? 35 :
        strcmp(lpCmdLine,"--preview=trade-return")==0 ? 36 :
        strcmp(lpCmdLine,"--preview=quick-empty")==0 ? 37 :
        strcmp(lpCmdLine,"--preview=quick-items")==0 ? 38 :
        strcmp(lpCmdLine,"--preview=quick-binding")==0 ? 39 :
        strcmp(lpCmdLine,"--preview=guild-notice-editor")==0 ? 40 :
        strcmp(lpCmdLine,"--preview=guild-members-editor")==0 ? 41 :
        strcmp(lpCmdLine,"--preview=npc-dialogue")==0 ? 42 :
        strcmp(lpCmdLine,"--preview=npc-scrolled")==0 ? 43 :
        strcmp(lpCmdLine,"--preview=npc-input")==0 ? 44 :
        strcmp(lpCmdLine,"--preview=npc-notclose")==0 ? 45 :
        strcmp(lpCmdLine,"--preview=shop-goods")==0 ? 46 :
        strcmp(lpCmdLine,"--preview=shop-offers")==0 ? 47 :
        strcmp(lpCmdLine,"--preview=shop-bulk")==0 ? 48 :
        strcmp(lpCmdLine,"--preview=shop-quantity")==0 ? 49 :
        strcmp(lpCmdLine,"--preview=storage-list")==0 ? 50 :
        strcmp(lpCmdLine,"--preview=storage-page")==0 ? 51 :
        strcmp(lpCmdLine,"--preview=storage-quantity")==0 ? 52 :
        strcmp(lpCmdLine,"--preview=merchant-repair")==0 ? 53 :
        strcmp(lpCmdLine,"--preview=merchant-sell")==0 ? 54 :
        strcmp(lpCmdLine,"--preview=merchant-deposit")==0 ? 55 :
        strcmp(lpCmdLine,"--preview=merchant-sell-quantity")==0 ? 56 :
        strcmp(lpCmdLine,"--preview=merchant-deposit-quantity")==0 ? 57 :
        strcmp(lpCmdLine,"--preview=npc-password")==0 ? 58 :
        strcmp(lpCmdLine,"--preview=npc-shop-layering")==0 ? 59 :
        strcmp(lpCmdLine,"--preview=portrait-cache-version-match")==0 ? 116 :
        strcmp(lpCmdLine,"--preview=vital-accessory-hint")==0 ? PreviewVitalAccessory :
        strcmp(lpCmdLine,"--preview=defense-accessory-hint")==0 ? PreviewDefenseAccessory :
        strcmp(lpCmdLine,"--preview=recovery-accessory-hint")==0 ? PreviewRecoveryAccessory :
        strcmp(lpCmdLine,"--preview=accuracy-bracelet-hint")==0 ? PreviewAccuracyBracelet :
        strcmp(lpCmdLine,"--preview=accuracy-necklace-hint")==0 ? 140 :
        strcmp(lpCmdLine,"--preview=necklace-hint")==0 ? 139 :
        strcmp(lpCmdLine,"--preview=equipment-hover-hint")==0 ? 138 :
        strcmp(lpCmdLine,"--preview=helmet-hint")==0 ? 136 :
        strcmp(lpCmdLine,"--preview=accessory-26-hint")==0 ? 137 :
        strcmp(lpCmdLine,"--preview=weapon-hint")==0 ? 134 :
        strcmp(lpCmdLine,"--preview=weapon-special-hint")==0 ? 135 :
        strcmp(lpCmdLine,"--preview=armor-hint")==0 ? 132 :
        strcmp(lpCmdLine,"--preview=armor-elements-hint")==0 ? 133 :
        strcmp(lpCmdLine,"--preview=book-hint")==0 ? 129 :
        strcmp(lpCmdLine,"--preview=charm-hint")==0 ? 130 :
        strcmp(lpCmdLine,"--preview=stone-hint")==0 ? 131 :
        strcmp(lpCmdLine,"--preview=potion-hint")==0 ? 127 :
        strcmp(lpCmdLine,"--preview=potion-stack-hint")==0 ? 128 :
        strcmp(lpCmdLine,"--preview=item-layout-full-name")==0 ? 126 :
        strcmp(lpCmdLine,"--preview=equipment-durability")==0 ? 125 :
        strcmp(lpCmdLine,"--preview=equipment-records-reply")==0 ? 124 :
        strcmp(lpCmdLine,"--preview=item-update-reply")==0 ? 123 :
        strcmp(lpCmdLine,"--preview=bag-records-reply")==0 ? 122 :
        strcmp(lpCmdLine,"--preview=item-record-reply")==0 ? 121 :
        strcmp(lpCmdLine,"--preview=item-stack-reply")==0 ? 120 :
        strcmp(lpCmdLine,"--preview=item-use-confirmed")==0 ? 119 :
        strcmp(lpCmdLine,"--preview=human-hit-server")==0 ? 118 :
        strcmp(lpCmdLine,"--preview=portrait-cache-version-zero")==0 ? 117 :
        strcmp(lpCmdLine,"--preview=portrait-chinese-filename")==0 ? 115 :
        strcmp(lpCmdLine,"--preview=portrait-upload-confirmed")==0 ? 114 :
        strcmp(lpCmdLine,"--preview=own-cache-restored")==0 ? 112 :
        strcmp(lpCmdLine,"--preview=own-cache-default")==0 ? 113 :
        strcmp(lpCmdLine,"--preview=own-photo-uploaded")==0 ? 110 :
        strcmp(lpCmdLine,"--preview=own-photo-default")==0 ? 111 :
        strcmp(lpCmdLine,"--preview=target-photo-reply")==0 ? 108 :
        strcmp(lpCmdLine,"--preview=target-photo-cleared")==0 ? 109 :
        strcmp(lpCmdLine,"--preview=target-portrait-query")==0 ? 107 :
        strcmp(lpCmdLine,"--preview=target-native-hover")==0 ? 105 :
        strcmp(lpCmdLine,"--preview=target-native-expired")==0 ? 106 :
        strcmp(lpCmdLine,"--preview=target-server-attributes")==0 ? 103 :
        strcmp(lpCmdLine,"--preview=target-server-removed")==0 ? 104 :
        strcmp(lpCmdLine,"--preview=minimap-server-reply")==0 ? 101 :
        strcmp(lpCmdLine,"--preview=minimap-server-unavailable")==0 ? 102 :
        strcmp(lpCmdLine,"--preview=options-server-on")==0 ? 99 :
        strcmp(lpCmdLine,"--preview=options-server-off")==0 ? 100 :
        strcmp(lpCmdLine,"--preview=options-attack-on")==0 ? 97 :
        strcmp(lpCmdLine,"--preview=options-attack-off")==0 ? 98 :
        strcmp(lpCmdLine,"--preview=options-damage-on")==0 ? 95 :
        strcmp(lpCmdLine,"--preview=options-damage-off")==0 ? 96 :
        strcmp(lpCmdLine,"--preview=options-light-on")==0 ? 93 :
        strcmp(lpCmdLine,"--preview=options-light-off")==0 ? 94 :
        strcmp(lpCmdLine,"--preview=options-shadow-on")==0 ? 91 :
        strcmp(lpCmdLine,"--preview=options-shadow-off")==0 ? 92 :
        strcmp(lpCmdLine,"--preview=options-markers-on")==0 ? 89 :
        strcmp(lpCmdLine,"--preview=options-markers-off")==0 ? 90 :
        strcmp(lpCmdLine,"--preview=target-photo")==0 ? 87 :
        strcmp(lpCmdLine,"--preview=target-photo-disabled")==0 ? 88 :
        strcmp(lpCmdLine,"--preview=target-monster")==0 ? 82 :
        strcmp(lpCmdLine,"--preview=target-attributes")==0 ? 83 :
        strcmp(lpCmdLine,"--preview=target-human")==0 ? 84 :
        strcmp(lpCmdLine,"--preview=target-unknown")==0 ? 85 :
        strcmp(lpCmdLine,"--preview=target-special")==0 ? 86 :
        strcmp(lpCmdLine,"--preview=portrait-map-full")==0 ? 79 :
        strcmp(lpCmdLine,"--preview=portrait-map-normal")==0 ? 80 :
        strcmp(lpCmdLine,"--preview=portrait-map-expanded")==0 ? 81 :
        strcmp(lpCmdLine,"--preview=minimap-small")==0 ? 75 :
        strcmp(lpCmdLine,"--preview=minimap-large")==0 ? 76 :
        strcmp(lpCmdLine,"--preview=minimap-transparent")==0 ? 77 :
        strcmp(lpCmdLine,"--preview=minimap-dragged")==0 ? 78 :
        strcmp(lpCmdLine,"--preview=hud-button-hint")==0 ? 72 :
        strcmp(lpCmdLine,"--preview=chat-mode-hint")==0 ? 73 :
        strcmp(lpCmdLine,"--preview=chat-reception-hint")==0 ? 74 :
        strcmp(lpCmdLine,"--preview=group-hint")==0 ? 67 :
        strcmp(lpCmdLine,"--preview=guild-hint")==0 ? 68 :
        strcmp(lpCmdLine,"--preview=skill-button-hint")==0 ? 69 :
        strcmp(lpCmdLine,"--preview=trade-hint")==0 ? 70 :
        strcmp(lpCmdLine,"--preview=npc-close-hint")==0 ? 71 :
        strcmp(lpCmdLine,"--preview=inventory-hint")==0 ? 64 :
        strcmp(lpCmdLine,"--preview=status-button-hint")==0 ? 65 :
        strcmp(lpCmdLine,"--preview=quest-hint")==0 ? 66 :
        strcmp(lpCmdLine,"--preview=portrait-hint")==0 ? 63 :
        strcmp(lpCmdLine,"--preview=portrait-empty")==0 ? 60 :
        strcmp(lpCmdLine,"--preview=portrait-preview")==0 ? 61 :
        strcmp(lpCmdLine,"--preview=portrait-scrolled")==0 ? 62 : 0;
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
