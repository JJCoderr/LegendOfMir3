#include "StdAfx.h"
#include "ServerSelectProcess/SvrSelProc.h"
#include "GameProcess/MailWire.h"
#include "GameProcess/AccessoryHintIds.h"
#include "GameProcess/ItemHintStyle.h"
#include "GameProcess/MerchantServiceIds.h"
#include "GameProcess/EquipmentActionIds.h"
#include "GameProcess/TradeWire.h"
#include "GameProcess/SpellPreview.h"
#include "GameProcess/GroundItemPreview.h"
#include "GameProcess/LightingPreview.h"
namespace { constexpr int PreviewSpellGround=237,PreviewSpellOrbit=238,PreviewSpellStaged=239,PreviewSpellMap=240,PreviewGroundItems=241,PreviewGroundCoin=242,PreviewLightingDay=243,PreviewLightingNight=244,PreviewLightingDawn=245,PreviewWeatherSnow=246,PreviewWeatherRain=247,PreviewWeatherFog=248; constexpr int PreviewExitProgram=231,PreviewExitCharacter=232,PreviewExitBlocked=233,PreviewExitFade=234,PreviewExitWaiting=235,PreviewExitQuit=236; constexpr int PreviewKeyboard=228,PreviewKeyboardShortcuts=229,PreviewKeyboardCovered=230; constexpr int PreviewTradeTransaction=223,PreviewTradeQuantity=224,PreviewTradeCancelled=225,PreviewTradeGoldInput=226,PreviewTradeOpenError=227; constexpr int PreviewEquipSuccess=220,PreviewEquipFailure=221,PreviewEquipTakeOff=222; constexpr int PreviewStorageReply=219; constexpr int PreviewServiceRepair=215,PreviewServiceSell=216,PreviewServiceDeposit=217,PreviewServiceFailure=218; constexpr int PreviewItemUse=213,PreviewItemUseRestored=214; constexpr int PreviewInterfaceAudio=212; constexpr int PreviewNotice=210,PreviewNoticeMoved=211; constexpr int PreviewSpeechSingle=207,PreviewSpeechMultiple=208,PreviewSpeechEight=209; constexpr int PreviewChatHistory=205,PreviewChatHistoryExpanded=206; constexpr int PreviewSceneIdentity=204; constexpr int PreviewGuildIdentity=202,PreviewNoGuildIdentity=203; constexpr int PreviewGuildHistory=201; constexpr int PreviewGuildWire=195,PreviewGuildMembers=196,PreviewGuildEditorWire=197,PreviewGuildError=198,PreviewGroupWire=199,PreviewGroupError=200; constexpr int PreviewSkillWire=193,PreviewSkillProgress=194; constexpr int PreviewProgression=191,PreviewLevelTail=192; constexpr int PreviewAbility=190; constexpr int PreviewQuestWire=187,PreviewQuestDetail=188,PreviewQuestIncrement=189; constexpr int PreviewSceneMap=183,PreviewScenePortrait=184,PreviewSceneSiege=185,PreviewSceneHidden=186; constexpr int PreviewSiegeExpired=182; constexpr int PreviewConsignmentInput=181; constexpr int PreviewConsignmentWire=180; constexpr int PreviewMailWire=177,PreviewMailWireRead=178,PreviewMailNotify=179; constexpr int PreviewSiege=174,PreviewSiegeLoad=175,PreviewSiegeChecks=176; constexpr int PreviewConsignmentEmpty=165, PreviewConsignmentList=166, PreviewConsignmentRegistration=167, PreviewConsignmentItem=168, PreviewConsignmentPrice=169, PreviewConsignmentBuy=170, PreviewConsignmentCancel=171, PreviewConsignmentConfirm=172, PreviewConsignmentChecks=173; constexpr int PreviewMailInbox=154, PreviewMailRead=155, PreviewMailCompose=156, PreviewMailGift=157, PreviewMailOutbox=158, PreviewMailChecks=159, PreviewMailGold=160, PreviewMailGiftItem=161, PreviewMailBody=162, PreviewMailGoldHint=163, PreviewMailGiftHint=164; constexpr int PreviewTradeLocal=151, PreviewTradeRemote=152, PreviewTradeHeld=153; constexpr int PreviewRedArmor=149, PreviewBlueArmor=150; constexpr int PreviewShopArmorHint=148; constexpr int PreviewSpecialHint=145, PreviewShopHint=146, PreviewStorageHint=147; constexpr int PreviewVitalAccessory=141, PreviewDefenseAccessory=142, PreviewRecoveryAccessory=143, PreviewAccuracyBracelet=144; }
#include "Common/LegacyPanel.h"
#include "Common/LoginMessages.h"

CMirSound			g_xSound;
CWHDXGraphicWindow	g_xMainWnd(640, 480, _SCREEN_BPP);
CLoginProcess		g_xLoginProc;
CSvrSelProcess g_xServerGroups;
CCharacterProcess	g_xChrSelProc;
CGameProcess		g_xGameProc;
CSprite				g_xSpriteInfo;
CChatEdit			g_xChatEditBox;
CClientSocket		g_xClientSocket;

INT					g_nCertifyCode;
char				g_szUserID[25];
char				g_szCharName[20];
char				g_szServerIP[16];
INT					g_nServerPort;
INT					g_nVersion;


BYTE				g_bProcState = _LOGIN_PROC;



#include "LoginProcess/LoginPreview.h"
#include "LoginProcess/LoginTransitionPreview.h"
#include "ServerSelectProcess/ServerGroupsPreview.h"
#include "Common/ConnectionPreview.h"
#include "AVI/MoviePreview.h"
#include "CharSelectProcess/SelectionDialogPreview.h"
#include "CharSelectProcess/SelectionRosterPreview.h"
#include "CharSelectProcess/CharacterInteractionPreview.h"
#include "CharSelectProcess/RolePresentationPreview.h"

// Offline visual review uses the real interface without map or network updates.
class CHudPreviewProcess : public CWHDefProcess {
    POINT mouse = {0,0};
    int damagePreview=0;
    int modalPointerPreview=0;
    bool levelPreview=false;
    bool loginPreview=false;
    bool moviePreview=false;
    bool serverGroupsPreview=false;
    bool connectionPreview=false,connectionSelectionPreview=false;
    bool spellPreview=false,spellMapPreview=false;
    bool namePreview=false;
    bool speechPreview=false;
    bool noticePreview=false;
    bool exitFadePreview=false,exitWaitingPreview=false,exitQuitPreview=false;
    SOCKET exitListener=INVALID_SOCKET,exitPeer=INVALID_SOCKET;
    int exitPort=0,exitNetworkStage=0;
    const char* capturePath = NULL;
public:
    void OnConnectToServer() override {}
    char* OnMessageReceive(CHAR*) override { return NULL; }
    void Load(bool expanded, int chatMode, bool exitDialog, int groupPanel) {
        if(groupPanel>=RolePresentationPreview::First && groupPanel<=RolePresentationPreview::Last) {
            connectionSelectionPreview=true;capturePath=RolePresentationPreview::Load(groupPanel);return;
        }
        if(groupPanel>=CharacterInteractionPreview::First && groupPanel<=CharacterInteractionPreview::Last) {
            connectionSelectionPreview=true;capturePath=CharacterInteractionPreview::Load(groupPanel);return;
        }
        if(groupPanel>=SelectionRosterPreview::First && groupPanel<=SelectionRosterPreview::Last) {
            connectionSelectionPreview=true;capturePath=SelectionRosterPreview::Load(groupPanel);return;
        }
        if(groupPanel>=SelectionDialogPreview::First && groupPanel<=SelectionDialogPreview::Last) {
            connectionSelectionPreview=true;capturePath=SelectionDialogPreview::Load(groupPanel);return;
        }
        if(groupPanel>=MoviePreview::First && groupPanel<=MoviePreview::Last) {
            moviePreview=true;capturePath=MoviePreview::Load(groupPanel);return;
        }
        if(groupPanel>=LoginTransitionPreview::First && groupPanel<=LoginTransitionPreview::Last) {
            loginPreview=true;capturePath=LoginTransitionPreview::Load(groupPanel);return;
        }
        if(groupPanel>=ConnectionPreview::First && groupPanel<=ConnectionPreview::Last) {
            connectionPreview=true;connectionSelectionPreview=groupPanel==ConnectionPreview::Selection;loginPreview=!connectionSelectionPreview;
            capturePath=ConnectionPreview::Load(groupPanel);return;
        }
        if(groupPanel>=ServerGroupsPreview::First && groupPanel<=ServerGroupsPreview::Last) {
            serverGroupsPreview=true;capturePath=ServerGroupsPreview::Load(groupPanel);return;
        }
        if(groupPanel>=LoginPreview::First && groupPanel<=LoginPreview::Last) {
            loginPreview=true;capturePath=LoginPreview::Load(groupPanel);return;
        }
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
        if(groupPanel>=PreviewMailInbox && groupPanel<=PreviewWeatherFog) {
            // Review native controls at the same physical pixel size as the 800x600 backbuffer.
            const HWND window=g_xMainWnd.GetSafehWnd();
            RECT bounds={0,0,MailLayout::CanvasWidth,MailLayout::CanvasHeight};
            AdjustWindowRectEx(&bounds,GetWindowLong(window,GWL_STYLE),FALSE,GetWindowLong(window,GWL_EXSTYLE));
            SetWindowPos(window,nullptr,0,0,bounds.right-bounds.left,bounds.bottom-bounds.top,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
            GetClientRect(window,&g_xMainWnd.m_rcWindow);
            MapWindowPoints(window,nullptr,(POINT*)&g_xMainWnd.m_rcWindow,2);
        }
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
            skills.m_pstMyMagic[0].bUseKey='1';skills.UpdateShortcut(skills.m_pstMyMagic[0],true);
            ui.WindowActivate(_WND_ID_SKILL); ui.WindowActivate(_WND_ID_STATUS);
            const bool origin=skills.m_rcWnd.left==348 && skills.m_rcWnd.top==0 && skills.m_nWidth==360 && skills.m_nHeight==488;
            ui.OnLButtonDown(POINT{700,410}); ui.OnMouseMove(POINT{690,420}); ui.OnLButtonUp(POINT{690,420});
            const bool draggedWindow=skills.m_rcWnd.left==338 && skills.m_rcWnd.top==10 && ui.m_xStatusWnd.m_rcWnd.left==0;
            ui.OnKeyDown('E',0); const bool hiddenSkill=!skills.GetGameWndActive() && ui.m_xStatusWnd.GetGameWndActive();
            ui.OnKeyDown('E',0); const bool restoredSkills=skills.GetGameWndActive() && skills.m_rcWnd.left==338 && skills.m_rcWnd.top==10;
            ui.OnKeyDown('Q',0); const bool hiddenStatus=!ui.m_xStatusWnd.GetGameWndActive() && skills.GetGameWndActive();
            ui.OnKeyDown('Q',0); const bool restoredStatus=ui.m_xStatusWnd.GetGameWndActive() && ui.m_xStatusWnd.m_rcWnd.left==0;
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
                    skills.m_pstMyMagic[key-1].bUseKey=skills.LegacyKey(key);skills.UpdateShortcut(skills.m_pstMyMagic[key-1],true);
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
                skills.m_pstMyMagic[1].bUseKey='C';skills.UpdateShortcut(skills.m_pstMyMagic[1],true);
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
                skills.m_pstMyMagic[1].bUseKey='2'; skills.m_pstMyMagic[2].bUseKey='3';skills.UpdateShortcut(skills.m_pstMyMagic[1],true);skills.UpdateShortcut(skills.m_pstMyMagic[2],true);
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
            const POINT quickButton={160,475},inactiveButton={755,540};
            ui.OnLButtonDown(quickButton); ui.OnLButtonUp(quickButton);
            const bool hudOpened=quick.GetGameWndActive()!=FALSE;
            ui.OnLButtonDown(quickButton); ui.OnLButtonUp(quickButton);
            const bool hudClosed=!quick.GetGameWndActive();
            ui.OnLButtonDown(inactiveButton); ui.OnLButtonUp(inactiveButton);
            const bool noLegacyPopup=!ui.m_xChatPopWnd.GetGameWndActive();
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
                fprintf(report,"hud_opened=%d hud_closed=%d no_legacy_popup=%d opened=%d repeat_ignored=%d hidden=%d slots=%d gaps=%d stray=%d outside_release=%d moved=%d slot_no_drag=%d closed=%d pending_kept=%d trade_returned=%d sounds=%d\n",
                    hudOpened,hudClosed,noLegacyPopup,opened,repeat,hidden,slots,gap,stray,outside,moved,noDrag,closed,pending,returned,sounds); fclose(report);
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
        if(groupPanel>=304 && groupPanel<=306) {
            modalPointerPreview=groupPanel;
            auto& ui=g_xGameProc.m_xInterface;
            const POINT yes={280,310},no={490,310},outside={790,430};
            const auto open=[&](){ui.OpenNameDialog(MailLayout::GoldDialogAction);SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"23");};
            FILE* report=fopen("modal-pointer-check.txt","wb");
            const auto check=[&](const char* name,bool ok){if(report) fprintf(report,"%s=%d\n",name,ok);};
            open();ui.OnLButtonUp(yes);check("release_requires_press",ui.m_xMsgBox.IsActive());
            ui.OnLButtonDown(yes);ui.OnMouseMove(outside);ui.OnMouseMove(yes);ui.OnLButtonUp(yes);
            check("leave_return_cancels_click",ui.m_xMsgBox.IsActive());
            ui.OnLButtonDown(yes);ui.OnLButtonUp(no);check("cross_button_cancels",ui.m_xMsgBox.IsActive());
            ui.OnLButtonDown(no);ui.OnMouseMove(outside);
            check("mouse_keeps_keyboard_yes",ui.m_xMsgBox.OnKeyDown(VK_RETURN)==CLICK_YES);
            ui.OnKeyDown(VK_TAB,0);ui.OnLButtonDown(yes);ui.OnMouseMove(outside);
            check("mouse_keeps_keyboard_no",ui.m_xMsgBox.OnKeyDown(VK_RETURN)==CLICK_NO);
            ui.OnKeyDown(VK_RETURN,0);check("keyboard_cancel_closes",!ui.m_xMsgBox.IsActive());
            open();ui.OnLButtonDown(no);ui.OnMouseMove(no);ui.OnLButtonUp(no);
            check("paired_cancel_closes",!ui.m_xMsgBox.IsActive());
            ui.ShowNotice(4000);const POINT ok={370,310};
            ui.OnLButtonDown(ok);ui.OnMouseMove(outside);ui.OnMouseMove(ok);ui.OnLButtonUp(ok);
            check("alert_leave_return_cancels",ui.m_xMsgBox.IsActive());
            ui.OnLButtonDown(ok);ui.OnLButtonUp(ok);check("alert_pair_closes",!ui.m_xMsgBox.IsActive());
            if(report) fclose(report);
            open();ui.OnLButtonDown(yes);mouse=yes;
            if(groupPanel>=305) {ui.OnMouseMove(outside);mouse=outside;}
            if(groupPanel==306) {ui.OnMouseMove(yes);mouse=yes;}
            capturePath=groupPanel==304?"modal-button-pressed.bmp":groupPanel==305?"modal-button-left.bmp":"modal-button-returned.bmp";
        }
        if(groupPanel>=301 && groupPanel<=303) {
            auto& ui=g_xGameProc.m_xInterface;
            if(groupPanel==301) {
                FILE* report=fopen("modal-input-check.txt","wb");
                const auto check=[&](const char* name,bool ok){if(report) fprintf(report,"%s=%d\n",name,ok);};
                constexpr int PermissionNotice=4000;
                ShowWindow(g_xChatEditBox.GetSafehWnd(),SW_SHOW);SetFocus(g_xChatEditBox.GetSafehWnd());
                SetCapture(g_xMainWnd.GetSafehWnd());ui.ShowNotice(PermissionNotice);
                check("notice_releases_chat_and_capture",GetFocus()==g_xMainWnd.GetSafehWnd() && GetCapture()!=g_xMainWnd.GetSafehWnd());
                ui.OnKeyDown(VK_RETURN,0);
                auto& mail=ui.m_xMailWnd;mail.draft="Modal mail draft";mail.recipient="Recipient";
                mail.dragging=mail.captured=mail.scrolling=true;mail.pressed=0;
                auto& sale=ui.m_xConsignmentRegistration;sale.description="Modal sale draft";sale.waiting=true;
                sale.dragging=sale.captured=true;sale.pressed=0;
                auto& trade=ui.m_xExchangeWnd;trade.goldPending=true;trade.captured=0;trade.confirmPressed=true;
                ui.ShowNotice(PermissionNotice);
                check("mail_cancel_preserves_draft",!mail.dragging && !mail.captured && !mail.scrolling && mail.pressed==-1 && mail.draft=="Modal mail draft" && mail.recipient=="Recipient");
                check("sale_cancel_preserves_pending",!sale.dragging && !sale.captured && sale.pressed==-1 && sale.waiting && sale.description=="Modal sale draft");
                check("trade_cancel_preserves_pending",trade.captured==-1 && !trade.confirmPressed && trade.goldPending);
                ui.OnKeyDown(VK_RETURN,0);sale.waiting=false;trade.goldPending=false;
                ui.m_xQuickWnd.closePressed=true;ui.OpenNameDialog(TradeWire::QuantityDialog);
                check("input_dialog_cancels_and_focuses",!ui.m_xQuickWnd.closePressed && GetFocus()==g_xChatEditBox.GetSafehWnd());
                ui.OnKeyDown(VK_ESCAPE,0);
                check("input_dialog_escape_consumed",ui.m_xMsgBox.IsActive());
                ui.OnKeyDown(VK_TAB,0);ui.OnKeyDown(VK_RETURN,0);
                check("input_dialog_cancel_closes",!ui.m_xMsgBox.IsActive());
                ui.m_xNPCWnd.prompt="Input focus check";ui.m_xNPCWnd.promptCommand="@@test";ui.OpenNameDialog(7);
                const HWND input=g_xChatEditBox.GetSafehWnd();SetWindowTextA(input,"Draft");
                SendMessage(input,WM_KEYDOWN,VK_ESCAPE,0);
                check("native_escape_keeps_focus",ui.m_xMsgBox.IsActive() && GetFocus()==input);
                SendMessage(input,WM_KEYDOWN,VK_F10,0);
                check("native_function_keeps_focus",ui.m_xMsgBox.IsActive() && GetFocus()==input);
                SendMessage(input,WM_CHAR,VK_TAB,0);
                check("native_tab_keeps_focus",GetFocus()==input);
                SendMessage(input,WM_CHAR,VK_RETURN,1L<<30);
                check("native_repeat_does_not_submit",ui.m_xMsgBox.IsActive());
                SendMessage(input,WM_CHAR,VK_RETURN,0);
                check("native_cancel_clears_prompt",!ui.m_xMsgBox.IsActive() && ui.m_xNPCWnd.promptCommand.empty() && !IsWindowVisible(input));
                ui.m_xMailWnd.giftGold=17;ui.m_xMailWnd.giftGoldConfirmed=true;
                ui.OpenNameDialog(MailLayout::GoldDialogAction);SetWindowTextA(input,"23");
                SendMessage(input,WM_CHAR,VK_TAB,0);SendMessage(input,WM_CHAR,VK_RETURN,0);
                check("native_cancel_does_not_submit_gold",ui.m_xMailWnd.giftGold==17 && !ui.m_xMailWnd.giftGoldConfirmed);




                const POINT confirm={360,303},outside={790,590};
                auto& quest=ui.m_xQuestWnd;ui.WindowActivate(_WND_ID_QUEST);
                quest.entries.resize(30);for(auto& e:quest.entries) e.title="Quest";
                quest.first=0;quest.scrollCaptured=true;ui.ShowNotice(PermissionNotice);
                ui.OnMouseMove(outside);check("quest_scroll_blocked",quest.first==0 && !quest.scrollCaptured);
                g_xGameProc.m_ptMousePos=outside;ui.OnScrollUp();ui.OnScrollDown();check("wheel_blocked",quest.first==0);
                ui.OnLButtonDown(confirm);ui.OnLButtonUp(confirm);check("notice_click_over_scroll",!ui.m_xMsgBox.IsActive() && !quest.scrollCaptured);
                ui.OnMouseMove(outside);check("quest_does_not_resume",quest.first==0 && !quest.scrollCaptured);
                ui.WindowActivate(_WND_ID_QUEST);
                auto& options=ui.m_xOptionWnd;ui.WindowActivate(_WND_ID_OPTION);options.LoadAudioOptions(nullptr);
                options.m_audioDrag=0;options.m_musicLevel=-50;ui.ShowNotice(PermissionNotice);
                ui.OnMouseMove(outside);check("audio_drag_blocked",options.m_musicLevel==-50 && options.m_audioDrag==-1);
                ui.OnLButtonDown(confirm);ui.OnLButtonUp(confirm);check("notice_click_over_audio",!ui.m_xMsgBox.IsActive() && options.m_audioDrag==-1);
                ui.OnLButtonUp(outside);check("audio_release_after_notice",options.m_audioDrag==-1);
                ui.WindowActivate(_WND_ID_OPTION);
                ui.m_xGuildEditor.Open(true);auto& editor=ui.m_xGuildEditor;
                editor.dragging=true;const RECT before=editor.m_rcWnd;
                SetWindowTextA(editor.edit,"Preserved draft");ui.ShowNotice(PermissionNotice);editor.Show();
                check("editor_hidden_behind_notice",!IsWindowVisible(editor.edit) && GetFocus()!=editor.edit);
                ui.OnMouseMove(outside);check("editor_drag_blocked",!editor.dragging && EqualRect(&before,&editor.m_rcWnd));
                ui.OnLButtonDown(confirm);ui.OnLButtonUp(confirm);check("notice_click_over_editor",!ui.m_xMsgBox.IsActive() && editor.m_bActive);
                editor.Show();char draft[64]={};GetWindowTextA(editor.edit,draft,sizeof(draft));
                check("editor_restored_with_draft",IsWindowVisible(editor.edit) && GetFocus()==editor.edit && !strcmp(draft,"Preserved draft"));
                ui.ShowNotice(PermissionNotice);ui.OnKeyDown(VK_RETURN,0);
                check("notice_keyboard_over_editor",!ui.m_xMsgBox.IsActive() && editor.m_bActive);
                editor.dragging=false;editor.Close(false);
                ui.m_xQuickWnd.closePressed=true;ui.ShowNotice(PermissionNotice);
                ui.OnLButtonDown(confirm);ui.OnLButtonUp(confirm);check("notice_over_quick_capture",!ui.m_xMsgBox.IsActive() && !ui.m_xQuickWnd.closePressed);
                ui.m_xQuickWnd.closePressed=false;
                ui.m_xHorseWnd.pressed=0;ui.ShowNotice(PermissionNotice);
                ui.OnLButtonDown(confirm);ui.OnLButtonUp(confirm);check("notice_over_mount_capture",!ui.m_xMsgBox.IsActive() && ui.m_xHorseWnd.pressed==-1);
                ui.m_xHorseWnd.pressed=-1;
                ui.WindowActivate(_WND_ID_QUEST);quest.scrollCaptured=true;
                if(report) fclose(report);
            } else if(groupPanel==302) ui.WindowActivate(_WND_ID_OPTION);
            else ui.m_xGuildEditor.Open(true);
            ui.ShowNotice(4000);mouse={790,430};capturePath=groupPanel==301?"modal-scroll.bmp":groupPanel==302?"modal-audio.bmp":"modal-guild-editor.bmp";
        }
        if(groupPanel==300) {
            auto& ui=g_xGameProc.m_xInterface;auto& o=ui.m_xOptionWnd;
            ui.WindowActivate(_WND_ID_OPTION);o.LoadAudioOptions(nullptr);
            FILE* report=fopen("options-interaction-check.txt","wb");
            const auto check=[&](const char* name,bool ok){if(report) fprintf(report,"%s=%d\n",name,ok);};
            const auto point=[&](int x,int y){return POINT{o.m_rcWnd.left+x,o.m_rcWnd.top+y};};
            const bool before=o.m_values[3];ui.OnLButtonUp(point(432,207));
            check("release_toggles",o.m_values[3]!=before && g_xSound.m_reversePan==o.m_values[3]);
            const bool music=o.m_values[1],effects=o.m_values[2];
            ui.OnLButtonDown(point(432,87));ui.OnLButtonUp(point(432,147));
            check("release_target",o.m_values[1]==music && o.m_values[2]!=effects);
            ui.OnLButtonUp(point(432,147));check("effects_restored",o.m_values[2]==effects);
            const bool local=o.m_values[7];ui.OnLButtonUp(point(188,57));
            check("local_release",o.m_values[7]!=local);
            ui.OnLButtonDown(point(188,57));ui.OnLButtonUp(POINT{790,590});
            check("outside_no_toggle",o.m_values[7]!=local);
            const bool reverse=o.m_values[3];ui.OnLButtonDown(point(50,20));
            check("background_drag",ui.m_bMoveFocusedWnd && o.m_bFocused);
            ui.OnLButtonUp(point(432,207));check("drag_release_no_toggle",o.m_values[3]==reverse && !ui.m_bMoveFocusedWnd);
            ui.OnLButtonDown(point(432,207));check("control_no_drag",!ui.m_bMoveFocusedWnd);
            ui.OnLButtonUp(point(432,207));check("paired_still_works",o.m_values[3]!=reverse);
            ui.OnLButtonUp(point(450,417));check("release_closes",!o.m_bActive);
            ui.WindowActivate(_WND_ID_OPTION);o.LoadAudioOptions(nullptr);o.SetStatusBtnInit();
            if(report) fclose(report);mouse=POINT{790,430};capturePath="options-interaction.bmp";
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
            ui.OnLButtonUp(toggle);const bool releaseToggle=!options.m_values[7];
            ui.OnLButtonUp(toggle);
            ui.OnLButtonDown(toggle);ui.OnLButtonUp(toggle);const bool disabled=!options.m_values[7];
            options.LoadAudioOptions(nullptr);const bool defaults=options.m_values[7];options.LoadAudioOptions("./preview-marker-options.ini");const bool persisted=!options.m_values[7];
            ui.OnLButtonDown(toggle);ui.OnLButtonUp(toggle);const bool enabled=options.m_values[7];
            if(groupPanel==90) {ui.OnLButtonDown(toggle);ui.OnLButtonUp(toggle);}
            const bool finalValue=options.m_values[7];options.LoadAudioOptions(nullptr);options.m_values[7]=finalValue;
            map.pending=true;map.Receive(1001,0);hero.m_wPosX=337;hero.m_wPosY=267;map.monsters={{322,251}};map.green={{350,284}};map.orange={{360,276}};
            mouse=POINT{400,460};g_xGameProc.m_ptMousePos=mouse;capturePath=groupPanel==89?"options-markers-on.bmp":"options-markers-off.bmp";
            if(FILE* report=fopen("options-markers-check.txt","wb")) {fprintf(report,"release_toggle=%d disabled=%d defaults=%d persisted=%d enabled=%d\n",releaseToggle,disabled,defaults,persisted,enabled);fclose(report);}
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

        if(groupPanel>=PreviewMailWire && groupPanel<=PreviewMailNotify) {
            auto& ui=g_xGameProc.m_xInterface;auto& mail=ui.m_xMailWnd;
            BYTE header[MailWire::HeaderBytes]={},body[MailWire::BodyBytes]={};
            header[MailWire::Id]=7;header[MailWire::Kind]=1;
            strcpy((char*)header+MailWire::Person,"行会信使");strcpy((char*)header+MailWire::Subject,"新的旅程");strcpy((char*)header+MailWire::Date,"11-17");
            body[MailWire::Id]=7;body[MailWire::Gold]=100;strcpy((char*)body+MailWire::Text,"欢迎回到玛法大陆。\r这封信带来新的消息。");
            char encoded[256]={};fnEncode6BitBuf(header,encoded,sizeof(header),sizeof(encoded));const std::string list=std::string(encoded)+"/";
            const bool early=mail.ReceiveBody(false,body,sizeof(body));mail.ReceiveList(false,list.c_str(),1);
            const bool decoded=mail.inbox.size()==1 && mail.inbox[0].person=="行会信使" && mail.inbox[0].subject=="新的旅程" && mail.inbox[0].date=="11-17";
            const bool cached=decoded && mail.inbox[0].loaded && mail.inbox[0].gold==100;
            mail.ReceiveList(true,list.c_str(),1);const bool separated=mail.outbox.size()==1 && !mail.outbox[0].loaded;
            mail.ReceiveList(true,encoded,1);const bool delimiter=mail.outbox.empty();
            mail.ReceiveList(true,list.c_str(),1);const bool shortBody=!mail.ReceiveBody(false,body,sizeof(body)-1);
            ui.WindowActivate(_WND_ID_MAIL);mail.selectedInbox=0;mail.OpenSelected();const bool read=mail.view==MailLayout::View::Read;
            mail.SelectView(MailLayout::View::Compose);mail.ReceiveBody(false,body,sizeof(body));const bool draftKept=mail.view==MailLayout::View::Compose;
            mail.Close();mail.ReceiveBody(false,body,sizeof(body));const bool closed=!mail.m_bActive;
            ui.DeleteWindowToList(_WND_ID_MAIL);ui.WindowActivate(_WND_ID_MAIL);mail.SelectView(MailLayout::View::Inbox);
            mail.notified=true;POINT anchor={};const RECT badge=mail.NotificationBounds();const POINT badgePoint={badge.left+2,badge.top+2};
            const bool notification=ui.ButtonHintAt(badgePoint,anchor)==MailWire::NotificationCaption;
            if(FILE* report=fopen("mail-wire-check.txt","wb")) {fprintf(report,"early=%d decoded=%d cached=%d separated=%d delimiter=%d shortBody=%d read=%d draft=%d closed=%d notification=%d\n",early,decoded,cached,separated,delimiter,shortBody,read,draftKept,closed,notification);fclose(report);}
            CMailWnd::Request request={};_TDEFAULTMESSAGE packet={};
            request.sent=true;request.messageId=7;request.action=MailLayout::Action::Read;
            const bool readRequest=CMailWnd::BuildRequest(request,packet) && packet.wIdent==MailWire::Read && packet.nRecog==7 && packet.wParam==MailWire::Outbox;
            request.action=MailLayout::Action::Refresh;
            const bool queryRequest=CMailWnd::BuildRequest(request,packet) && packet.wIdent==MailWire::Query && packet.nRecog==0;
            request.sent=false;request.action=MailLayout::Action::DeleteRead;
            const bool deleteRequest=CMailWnd::BuildRequest(request,packet) && packet.wIdent==MailWire::Delete && packet.wParam==MailWire::Inbox;
            request.action=MailLayout::Action::CancelOutbox;
            const bool recallRequest=CMailWnd::BuildRequest(request,packet) && packet.wIdent==MailWire::Recall && packet.nRecog==7 && packet.wParam==0;
            request.action=MailLayout::Action::SendCompose;request.recipient="recipient";request.body="subject\r\n\r\nbody";
            std::array<BYTE,MailWire::SendBytes> outgoing={};
            const bool sendRequest=CMailWnd::BuildRequest(request,packet) && packet.wIdent==MailWire::Send && packet.nRecog==0 && packet.wParam==0 && packet.wTag==0 && packet.wSeries==0;
            const bool compose=CMailWnd::BuildCompose(request,outgoing) && strcmp((char*)outgoing.data()+MailWire::SendRecipient,"recipient")==0 && strcmp((char*)outgoing.data()+MailWire::SendSubject,"subject")==0 && strcmp((char*)outgoing.data()+MailWire::SendText,"subject\r \rbody")==0 && outgoing.back()==0;
            request.recipient=std::string(MailWire::RecipientLimit-1,'a')+"中文";request.body=std::string(MailWire::BodyLimit-1,'b')+"中文";
            const bool boundaries=CMailWnd::BuildCompose(request,outgoing) && strlen((char*)outgoing.data())==MailWire::RecipientLimit-1 && strlen((char*)outgoing.data()+MailWire::SendText)==MailWire::BodyLimit-1 && strlen((char*)outgoing.data()+MailWire::SendSubject)<=MailWire::SubjectLimit;
            char encodedSend[MailWire::SendBytes*2]={};std::array<BYTE,MailWire::SendBytes> decodedSend={};
            fnEncode6BitBuf(outgoing.data(),encodedSend,int(outgoing.size()),sizeof(encodedSend));
            const bool roundtrip=fnDecode6BitBuf(encodedSend,(char*)decodedSend.data(),int(decodedSend.size()))==MailWire::SendBytes && outgoing==decodedSend;
            request.recipient.clear();const bool empty=!CMailWnd::BuildCompose(request,outgoing);
            request.action=MailLayout::Action::SendGift;const bool giftUnsupported=!CMailWnd::BuildRequest(request,packet);
            if(FILE* report=fopen("mail-send-check.txt","wb")) {fprintf(report,"read=%d query=%d delete=%d recall=%d send=%d compose=%d boundaries=%d roundtrip=%d empty=%d giftUnsupported=%d\n",readRequest,queryRequest,deleteRequest,recallRequest,sendRequest,compose,boundaries,roundtrip,empty,giftUnsupported);fclose(report);}
            mouse={790,430};capturePath="mail-wire-list.bmp";
            if(groupPanel==PreviewMailWireRead) {mail.selectedInbox=0;mail.OpenSelected();capturePath="mail-wire-read.bmp";}
            if(groupPanel==PreviewMailNotify) {mouse=badgePoint;capturePath="mail-notification.bmp";}
        }








        if(groupPanel>=PreviewLightingDay && groupPanel<=PreviewWeatherFog) {
            spellMapPreview=LightingPreview::Scene(g_xGameProc,groupPanel-PreviewLightingDay);
            mouse=g_xGameProc.m_ptMousePos;
            const char* files[]={"lighting-day.bmp","lighting-night.bmp","lighting-dawn.bmp","weather-snow.bmp","weather-rain.bmp","weather-fog.bmp"};
            capturePath=files[groupPanel-PreviewLightingDay];
        }
        if(groupPanel==PreviewGroundItems || groupPanel==PreviewGroundCoin) {
            spellMapPreview=GroundItemPreview::Scene(g_xGameProc,groupPanel==PreviewGroundCoin);
            mouse=g_xGameProc.m_ptMousePos;
            capturePath=groupPanel==PreviewGroundCoin?"ground-coin.bmp":"ground-items.bmp";
        }
        if(groupPanel==PreviewSpellMap) {
            spellMapPreview=SpellPreview::MapScene(g_xGameProc);
            mouse={790,430};capturePath="spell-map.bmp";
        }
        if(groupPanel>=PreviewSpellGround && groupPanel<=PreviewSpellStaged) {
            SpellPreview::Checks(g_xGameProc);SpellPreview::Scene(g_xGameProc,groupPanel-PreviewSpellGround);
            spellPreview=true;mouse={790,430};
            capturePath=groupPanel==PreviewSpellGround?"spell-ground.bmp":groupPanel==PreviewSpellOrbit?"spell-orbit.bmp":"spell-staged.bmp";
        }
        if(groupPanel>=PreviewExitProgram && groupPanel<=PreviewExitQuit) {
            using namespace ExitFlow;
            auto& game=g_xGameProc;auto& ui=game.m_xInterface;
            FILE* report=fopen("exit-flow-check.txt","wb");
            const auto check=[&](const char* name,bool value) {if(report) fprintf(report,"%s=%d\n",name,int(value));};
            const DWORD now=100000;
            Guard guard;check("initial",guard.Allowed(now,0));
            for(int clock=Struck;clock<=Attack;++clock) {
                guard={};guard.ticks[clock]=now-CombatDelay;
                check(clock==Struck?"struck_8000":clock==Spell?"spell_8000":"attack_8000",!guard.Allowed(now,0));
                check(clock==Struck?"struck_8001":clock==Spell?"spell_8001":"attack_8001",guard.Allowed(now+1,0));
            }
            guard.ticks={now,now,now};check("death_exempt",guard.Allowed(now,DeathAction));check("action18_blocked",!guard.Allowed(now,18));
            guard.ticks={DWORD(0xfffffff0),DWORD(0xfffffff0),DWORD(0xfffffff0)};check("wrap_blocked",!guard.Allowed(0x10,0));check("wrap_allowed",guard.Allowed(0x2000,0));
            Transition fade;fade.Begin();check("fade_start",fade.Brightness()>0.999f);check("fade_2500",!fade.Advance(TransitionDuration) && fade.active && fade.Brightness()>0 && fade.Brightness()<0.02f);check("fade_2501",fade.Advance(1) && !fade.active);check("fade_once",!fade.Advance(1));
            check("spell_ids",RestrictsExit(1) && RestrictsExit(150) && !RestrictsExit(2) && !RestrictsExit(149));
            game.m_exitGuard={};hero.m_bCurrMtn=0;
            game.OnSysKeyDown('Q',1L<<29);check("alt_q",ui.m_xMsgBox.IsActive() && ui.m_nIdxMsgBox==ProgramDialog);
            ui.OnKeyDown(VK_ESCAPE,0);check("escape_modal",ui.m_xMsgBox.IsActive());
            ui.OnKeyDown(VK_TAB,0);ui.OnKeyDown(VK_RETURN,0);check("tab_cancel",!ui.m_xMsgBox.IsActive() && !game.m_exitTransition.active);
            game.OnSysKeyDown('X',1L<<29);check("alt_x",ui.m_xMsgBox.IsActive() && ui.m_nIdxMsgBox==CharacterDialog);
            game.m_exitGuard.ticks[Struck]=GetTickCount();ui.FinishNotice(CLICK_YES,false);check("confirm_recheck",!ui.m_xMsgBox.IsActive() && !game.m_exitTransition.active);
            ui.OpenExitDialog(ProgramDialog);check("open_blocked",!ui.m_xMsgBox.IsActive());
            game.m_exitGuard={};hero.m_bCurrMtn=DeathAction;ui.OpenExitDialog(ProgramDialog);check("death_dialog",ui.m_xMsgBox.IsActive());ui.FinishNotice(CLICK_NO,false);hero.m_bCurrMtn=0;
            check("assets",g_xChrSelProc.PrepareReturn());
            SOCKET receiver=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP),sender=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
            sockaddr_in address={};address.sin_family=AF_INET;address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);bind(receiver,(sockaddr*)&address,sizeof(address));int size=sizeof(address);getsockname(receiver,(sockaddr*)&address,&size);connect(sender,(sockaddr*)&address,sizeof(address));DWORD timeout=30;setsockopt(receiver,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));
            const SOCKET saved=g_xClientSocket.m_sockClient;g_xClientSocket.m_sockClient=sender;
            ui.OpenExitDialog(CharacterDialog);ui.FinishNotice(CLICK_NO,false);char wire[128]={};check("cancel_no_packet",recv(receiver,wire,sizeof(wire),0)==SOCKET_ERROR);
            const bool accepted=game.ConfirmExit(CharacterDialog,GetTickCount());const int length=recv(receiver,wire,sizeof(wire)-1,0);_TDEFAULTMESSAGE packet={};if(length>2) fnDecodeMessage(&packet,wire+2);
            check("logout_wire",accepted && length==DEFBLOCKSIZE+3 && packet.wIdent==Logout && !packet.nRecog && !packet.wParam && !packet.wTag && !packet.wSeries);
            check("repeat_rejected",!game.ConfirmExit(CharacterDialog,GetTickCount()) && recv(receiver,wire,sizeof(wire),0)==SOCKET_ERROR);
            game.m_exitTransition={};g_xClientSocket.m_sockClient=saved;closesocket(sender);closesocket(receiver);
            g_bProcState=_CHAR_SEL_PROC;check("old_page_rejected",!game.ConfirmExit(CharacterDialog,GetTickCount()));g_bProcState=_GAME_PROC;
            CLoginNotice waiting;waiting.ShowMessageBox("Waiting",CLoginNotice::Waiting,0);check("waiting_keys",!waiting.OnKeyDown(VK_RETURN) && !waiting.OnKeyDown(VK_SPACE) && !waiting.OnKeyDown(VK_ESCAPE));waiting.OnButtonDown(POINT{300,310});check("waiting_mouse",!waiting.OnButtonUp(POINT{300,310}));
            if(report) fclose(report);
            mouse={790,430};capturePath="exit-program.bmp";
            if(groupPanel==PreviewExitWaiting) {
                exitWaitingPreview=true;capturePath="exit-waiting.bmp";
                exitListener=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);sockaddr_in endpoint={};endpoint.sin_family=AF_INET;endpoint.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
                bind(exitListener,(sockaddr*)&endpoint,sizeof(endpoint));listen(exitListener,2);int bytes=sizeof(endpoint);getsockname(exitListener,(sockaddr*)&endpoint,&bytes);exitPort=ntohs(endpoint.sin_port);
                lstrcpyA(g_szServerIP,"127.0.0.1");g_nServerPort=exitPort;g_xChrSelProc.Load(true);
                SOCKET initial=accept(exitListener,nullptr,nullptr);closesocket(initial);PostMessage(g_xMainWnd.GetSafehWnd(),ID_SOCKCLIENT_EVENT_MSG,g_xClientSocket.m_sockClient,WSAMAKESELECTREPLY(FD_CLOSE,WSAECONNRESET));g_xClientSocket.DisconnectToServer();g_xChrSelProc.DeleteProc();
                lstrcpyA(g_szServerIP,"127.0.0.2");g_nServerPort=1;
                u_long nonblocking=1;ioctlsocket(exitListener,FIONBIO,&nonblocking);
            }
            else if(groupPanel==PreviewExitBlocked) {game.m_exitGuard.ticks[Struck]=GetTickCount();ui.OpenExitDialog(CharacterDialog);capturePath="exit-blocked.bmp";}
            else if(groupPanel==PreviewExitFade) {exitFadePreview=true;game.m_exitTransition.Begin();game.m_exitTransition.Advance(1200);capturePath="exit-fade.bmp";}
            else {ui.OpenExitDialog(groupPanel==PreviewExitCharacter?CharacterDialog:ProgramDialog);if(groupPanel==PreviewExitCharacter) capturePath="exit-character.bmp";}
            exitQuitPreview=groupPanel==PreviewExitQuit;
        }
        if(groupPanel>=PreviewKeyboard && groupPanel<=PreviewKeyboardCovered) {
            auto& ui=g_xGameProc.m_xInterface;auto& skills=ui.m_xSkillWnd;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_MICON].NewLoad("./Data/MIcon.wil");
            skills.m_bMyMagicCnt=2;skills.m_pstMyMagic=new CLIENTMAGICRCD[2]{};
            skills.m_pstMyMagic[0].stStdMagic.wMagicID=1;skills.m_pstMyMagic[0].bUseKey='1';skills.UpdateShortcut(skills.m_pstMyMagic[0],true);
            skills.m_pstMyMagic[1].stStdMagic.wMagicID=5;skills.m_pstMyMagic[1].nCurrTrain=42;skills.m_pstMyMagic[1].stStdMagic.nTrain[0]=500;
            lstrcpyA(skills.m_pstMyMagic[0].stStdMagic.szMagicName,"\xbb\xf0\xc7\xf2\xca\xf5");lstrcpyA(skills.m_pstMyMagic[1].stStdMagic.szMagicName,"\xb4\xf3\xbb\xf0\xc7\xf2");
            const auto pointer=[&](POINT point) {g_xGameProc.m_ptMousePos=point;};
            const auto hover=[&](int index) {const auto pos=SkillLayout::positions[skills.m_skillTab][index];pointer(POINT{skills.m_rcWnd.left+pos.x+5,skills.m_rcWnd.top+pos.y-skills.m_skillScroll+5});};
            using Binding=std::pair<int,int>;
            const auto capture=[&](auto action,const std::vector<Binding>& expected) {
                SOCKET receiver=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP),sender=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);sockaddr_in address={};address.sin_family=AF_INET;address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);bind(receiver,(sockaddr*)&address,sizeof(address));int size=sizeof(address);getsockname(receiver,(sockaddr*)&address,&size);connect(sender,(sockaddr*)&address,sizeof(address));DWORD timeout=30;setsockopt(receiver,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));const SOCKET saved=g_xClientSocket.m_sockClient;g_xClientSocket.m_sockClient=sender;action();g_xClientSocket.m_sockClient=saved;
                std::vector<Binding> actual;bool valid=true;char wire[128]={};int length=0;while((length=recv(receiver,wire,sizeof(wire)-1,0))!=SOCKET_ERROR) {if(length<19) {valid=false;break;}_TDEFAULTMESSAGE packet={};fnDecodeMessage(&packet,wire+2);valid &= packet.wIdent==CM_MAGICKEYCHANGE && !packet.wTag && !packet.wSeries && length==DEFBLOCKSIZE+3;actual.emplace_back(packet.nRecog,packet.wParam);}
                closesocket(sender);closesocket(receiver);return valid && actual==expected;
            };
            ui.OnKeyDown('E',0);const bool opened=skills.m_bActive;hover(1);
            const bool conflict=capture([&](){ui.OnKeyDown(VK_F1,0);},{{1,0},{5,'1'}}) && skills.Shortcut(0)==5 && !skills.m_pstMyMagic[0].bUseKey;
            const bool f12=capture([&](){ui.OnKeyDown(VK_F12,0);},{{5,'C'}}) && skills.Shortcut(11)==5 && !skills.Shortcut(0);
            const bool f10=capture([&](){g_xGameProc.OnSysKeyDown(VK_F10,0);},{{5,'A'}}) && skills.Shortcut(9)==5 && !ui.m_xStatusWnd.m_bActive;
            const bool f9=capture([&](){ui.OnKeyDown(VK_F9,0);},{{5,'9'}}) && skills.Shortcut(8)==5 && !ui.m_xInventoryWnd.m_bActive;
            const bool f11=capture([&](){ui.OnKeyDown(VK_F11,0);},{{5,'B'}}) && skills.Shortcut(10)==5 && skills.m_bActive;
            bool allKeys=true;for(int slot=1;slot<=SkillWire::ShortcutCount;++slot) {const BYTE key=CSkillWnd::LegacyKey(slot);std::vector<Binding> expected;if(skills.m_pstMyMagic[1].bUseKey==key) expected.emplace_back(5,0);expected.emplace_back(5,key);allKeys &= capture([&](){ui.OnKeyDown(VK_F1+slot-1,0);},expected) && skills.Shortcut(slot-1)==5;}
            const bool repeated=capture([&](){for(int slot=1;slot<=12;++slot) ui.OnKeyDown(VK_F1+slot-1,1L<<30);},{}) && skills.Shortcut(11)==5;
            hover(2);const bool unlearned=capture([&](){ui.OnKeyDown(VK_F12,0);},{{5,0}}) && !skills.Shortcut(11) && !skills.m_pstMyMagic[1].bUseKey;
            hover(1);skills.m_skillScroll=30;hover(1);const bool scrolled=capture([&](){ui.OnKeyDown(VK_F12,0);},{{5,'C'}}) && skills.Shortcut(11)==5;skills.m_skillScroll=0;
            hover(1);const bool editF12=capture([&](){g_xChatEditBox.ChatEditProc(g_xChatEditBox.GetSafehWnd(),WM_KEYDOWN,VK_F12,0);},{{5,0},{5,'C'}});
            const bool editF10=capture([&](){g_xChatEditBox.ChatEditProc(g_xChatEditBox.GetSafehWnd(),WM_SYSKEYDOWN,VK_F10,0);},{{5,'A'}});
            skills.BindMagic(5,'C');skills.m_rcWnd={0,0,360,488};hover(1);ui.m_xNPCWnd.SetScript(1,"药店老板/<购买/@buy>");if(!ui.m_xNPCWnd.m_bActive) ui.WindowActivate(_WND_ID_NPCCHAT);
            const bool covered=capture([&](){ui.OnKeyDown(VK_F2,0);},{}) && skills.Shortcut(11)==5;ui.WindowActivate(_WND_ID_NPCCHAT);skills.m_rcWnd={348,0,708,488};hover(1);
            ui.ShowNotice(1000);const bool modal=capture([&](){ui.OnKeyDown(VK_F3,0);},{}) && ui.m_xMsgBox.IsActive();ui.FinishNotice(CLICK_YES,false);
            ui.m_xGuildEditor.m_bActive=TRUE;const bool editor=capture([&](){ui.OnKeyDown(VK_F3,0);},{}) && skills.Shortcut(11)==5;ui.m_xGuildEditor.m_bActive=FALSE;
            pointer(POINT{150,150});const bool scene=!ui.OnKeyDown(VK_F12,0) && ui.SpellPointerAvailable();const bool unbound=ui.OnKeyDown(VK_F1,0)!=FALSE;
            const bool repeatScene=ui.OnKeyDown(VK_F1,1L<<30) && !ui.OnKeyDown(VK_F12,1L<<30);
            bool hud=true;for(POINT point:{POINT{-1,10},POINT{800,10},POINT{100,467},POINT{200,450},POINT{400,150}}) {pointer(point);hud &= ui.OnKeyDown(VK_F12,0)!=FALSE;}
            pointer(POINT{150,150});ui.m_minimap.visible=true;const bool minimap=ui.OnKeyDown(VK_F12,0)!=FALSE;ui.m_minimap.visible=false;const bool resumed=!ui.OnKeyDown(VK_F12,0);
            ui.OnKeyDown('R',0);pointer(POINT{200,300});const bool expanded=ui.m_chatExpanded && ui.OnKeyDown(VK_F12,0);ui.OnKeyDown('R',1L<<30);const bool chatRepeat=ui.m_chatExpanded;ui.OnKeyDown('R',0);const bool collapsed=!ui.m_chatExpanded && !ui.OnKeyDown(VK_F12,0);
            skills.m_shortcuts[10]=5;skills.m_pstMyMagic[0].stStdMagic.wMagicID=261;const bool lowByte=skills.GetMagicByShortcut(11)==&skills.m_pstMyMagic[1];skills.m_bMyMagicCnt=1;const bool deleted=skills.GetMagicByShortcut(11)==nullptr;skills.m_bMyMagicCnt=2;skills.m_pstMyMagic[0].stStdMagic.wMagicID=1;
            skills.m_pstMyMagic[0].bUseKey='1';skills.m_shortcuts[0]=5;skills.BindMagic(1,'3');const bool independent=skills.Shortcut(0)==5 && skills.Shortcut(2)==1;
            ui.OnKeyDown('E',0);bool panels=true,repeatPanels=true;
            const struct {WPARAM key;CGameWnd* panel;} panelKeys[]={{'E',&skills},{'Q',&ui.m_xStatusWnd},{'W',&ui.m_xInventoryWnd},{'G',&ui.m_xGroupWnd},{'N',&ui.m_xOptionWnd}};
            for(const auto& binding:panelKeys) {g_xGameProc.OnKeyDown(binding.key,0);panels &= binding.panel->m_bActive!=FALSE;g_xGameProc.OnKeyDown(binding.key,1L<<30);repeatPanels &= binding.panel->m_bActive!=FALSE;g_xGameProc.OnKeyDown(binding.key,0);panels &= !binding.panel->m_bActive;}
            ui.OnKeyDown('E',0);ui.OnKeyDown('Q',0);ui.OnKeyDown(VK_ESCAPE,0);const bool topClosed=skills.m_bActive && !ui.m_xStatusWnd.m_bActive;ui.OnKeyDown(VK_ESCAPE,0);const bool secondClosed=!skills.m_bActive;
            ui.m_xExchangeWnd.Open("TradePartner");if(ui.m_xInventoryWnd.m_bActive) ui.WindowActivate(_WND_ID_INVENTORY);ui.OnKeyDown(VK_ESCAPE,0);const bool tradeKept=ui.m_xExchangeWnd.m_bActive;ui.m_xExchangeWnd.Close();
            ui.WindowActivate(_WND_ID_STORE);ui.m_xStoreWnd.details=true;ui.OnKeyDown(VK_ESCAPE,0);const bool shopBack=ui.m_xStoreWnd.m_bActive && !ui.m_xStoreWnd.details;ui.OnKeyDown(VK_ESCAPE,0);const bool shopClosed=!ui.m_xStoreWnd.m_bActive;
            ui.OnKeyDown('E',0);hover(1);skills.BindMagic(5,'C');ui.m_bShowKey=true;skills.m_skillSelected[0]=1;mouse=g_xGameProc.m_ptMousePos;capturePath="keyboard-skill-binding.bmp";
            if(groupPanel==PreviewKeyboardShortcuts) {ui.OnKeyDown('E',0);mouse={150,150};pointer(mouse);capturePath="keyboard-skill-shortcuts.bmp";}
            if(groupPanel==PreviewKeyboardCovered) {skills.m_rcWnd={0,0,360,488};hover(1);mouse=g_xGameProc.m_ptMousePos;ui.m_xNPCWnd.SetScript(1,"药店老板/<购买/@buy>");ui.WindowActivate(_WND_ID_NPCCHAT);capturePath="keyboard-skill-covered.bmp";}
            if(FILE* report=fopen("keyboard-routing-check.txt","wb")) {fprintf(report,"opened=%d conflict=%d f12=%d f10=%d f9=%d f11=%d all_keys=%d repeat=%d unlearned=%d scrolled=%d edit_f12=%d edit_f10=%d covered=%d modal=%d editor=%d scene=%d unbound=%d repeat_scene=%d hud=%d minimap=%d resumed=%d expanded=%d chat_repeat=%d collapsed=%d low_byte=%d deleted=%d independent=%d panels=%d repeat_panels=%d top_closed=%d second_closed=%d trade_kept=%d shop_back=%d shop_closed=%d\n",opened,conflict,f12,f10,f9,f11,allKeys,repeated,unlearned,scrolled,editF12,editF10,covered,modal,editor,scene,unbound,repeatScene,hud,minimap,resumed,expanded,chatRepeat,collapsed,lowByte,deleted,independent,panels,repeatPanels,topClosed,secondClosed,tradeKept,shopBack,shopClosed);fclose(report);}
        }

        if(groupPanel>=PreviewTradeTransaction && groupPanel<=PreviewTradeOpenError) {
            using namespace TradeWire;
            auto& ui=g_xGameProc.m_xInterface;auto& trade=ui.m_xExchangeWnd;auto& bag=ui.m_xInventoryWnd;auto& held=ui.m_stCommonItem;auto& quick=ui.m_xQuickWnd;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            const auto item=[](const char* name,DWORD identity,int quantity,int stack,int icon=0) {
                BYTE raw[CExchangeWnd::RecordBytes]={};memcpy(raw,name,min(size_t(30),strlen(name)));raw[61]=BYTE(stack);
                raw[36]=LOBYTE(icon);raw[37]=HIBYTE(icon);for(int i=0;i<4;++i) raw[62+i]=BYTE(identity>>(8*i));raw[71]=LOBYTE(quantity);raw[72]=HIBYTE(quantity);
                CItem result;result.SetMir3Item(raw,0);return result;
            };
            const CItem potion=item("TradeLongName012345678901234567",4001,10,3),single=item("TradeSingle",4002,1,0),peer=item("PeerPotion",4003,8,3);
            const auto receive=[&](WORD opcode,int recognition=0,WORD low=0,WORD high=0,const BYTE* body=nullptr,int bytes=0) {
                _TDEFAULTMESSAGE message={};fnMakeDefMessage(&message,opcode,recognition,low,high,0);char encoded[256]={};fnEncodeMessage(&message,encoded,sizeof(encoded));if(bytes) fnEncode6BitBuf(const_cast<BYTE*>(body),encoded+DEFBLOCKSIZE,bytes,sizeof(encoded)-DEFBLOCKSIZE);g_xGameProc.OnSocketMessageRecieve(encoded);
            };
            const auto click=[&](POINT point) {ui.OnLButtonDown(point);ui.OnLButtonUp(point);};
            const auto bagCount=[&]() {int n=0;for(const auto& slot:bag.m_stInventoryItem) if(slot.bSetted) ++n;return n;};
            const auto reset=[&]() {
                trade.pendingSlot=-1;trade.offerCount=0;trade.Close();bag.DeleteAllInvenItem();bag.m_usePending=false;bag.m_lastRelease=0;held=COMMONITEMSET{};quick.heldSlot=-1;for(auto& slot:quick.slots) slot=CQuickWnd::Slot{};
                trade.m_rcWnd={0,0,494,330};hero.m_nGlod=100000;receive(TradeWire::Open,0,0,0,(const BYTE*)"TradePartner",12);
            };
            const auto pick=[&](const CItem& value) {bag.AddInvenItem(value,0,TRUE);bag.SetCommonFromInven(0,&held);};
            const auto capture=[&](auto action,WORD opcode,int identity,WORD quantity,const char* name) {
                SOCKET receiver=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP),sender=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);sockaddr_in address={};address.sin_family=AF_INET;address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);bind(receiver,(sockaddr*)&address,sizeof(address));int size=sizeof(address);getsockname(receiver,(sockaddr*)&address,&size);connect(sender,(sockaddr*)&address,sizeof(address));DWORD timeout=60;setsockopt(receiver,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));const SOCKET saved=g_xClientSocket.m_sockClient;g_xClientSocket.m_sockClient=sender;action();g_xClientSocket.m_sockClient=saved;
                char wire[256]={};const int length=recv(receiver,wire,sizeof(wire)-1,0);closesocket(sender);closesocket(receiver);if(!opcode) return length==SOCKET_ERROR;if(length<19) return false;
                _TDEFAULTMESSAGE header={};fnDecodeMessage(&header,wire+2);wire[length-1]=0;char text[128]={};fnDecode6BitBuf(wire+2+DEFBLOCKSIZE,text,sizeof(text));return header.wIdent==opcode && header.nRecog==identity && !header.wParam && header.wTag==quantity && !header.wSeries && !strcmp(text,name);
            };
            reset();const bool opened=trade.m_bActive && bag.m_bActive && trade.partner=="TradePartner" && bag.m_rcWnd.left==518;
            trade.first[0]=3;ui.WindowActivate(_WND_ID_INVENTORY);receive(TradeWire::Open);const bool reopened=bag.m_bActive && trade.first[0]==3 && trade.partner=="TradePartner";trade.first[0]=0;
            trade.Close();auto* target=new CHero;target->m_stFeature.bGender=_GENDER_MAN;target->m_wPosX=hero.m_wPosX;target->m_wPosY=hero.m_wPosY-1;lstrcpyA(target->m_szName,"TradeTarget");hero.m_bCurrDir=0;g_xGameProc.m_xActorList.AddNode(target);
            const bool request=capture([&](){ui.OnKeyDown('C',0);},Request,0,0,target->m_szName) && !trade.m_bActive;
            const bool repeat=capture([&](){ui.OnKeyDown('C',1L<<30);},0,0,0,"");target->m_bIsDead=TRUE;const bool dead=!trade.RequestTrade();target->m_bIsDead=FALSE;target->m_wPosX++;const bool direction=!trade.RequestTrade();target->m_wPosX--;target->m_stFeature.bGender=_GENDER_MON;const bool monster=!trade.RequestTrade();g_xGameProc.m_xActorList.MoveCurrentToTop();g_xGameProc.m_xActorList.DeleteCurrentNodeEx();delete target;
            reset();pick(potion);quick.slots[0]=CQuickWnd::FromItem(potion);click(POINT{30,75});const bool dialog=ui.m_xMsgBox.IsActive() && ui.m_nIdxMsgBox==QuantityDialog && trade.pendingSlot==0 && !trade.offerCount;
            const bool addRequest=capture([&](){lstrcpyA(g_xChatEditBox.m_szInputMsg,"4");ui.OnMsgInputted(0,0);},Add,4001,4,potion.Name());
            const bool pending=trade.offerCount==4 && held.bSetted && !ui.m_xMsgBox.IsActive();bag.OnLButtonUp(&held,POINT{550,80});quick.Activate(0,5000,false);
            const bool locked=held.bSetted && !bagCount() && !bag.m_usePending && !bag.RequestUse(5000);
            receive(RemoveOk);const bool wrong=trade.pendingSlot==0 && held.bSetted;receive(AddOk,5001,6);
            const bool split=trade.offers[0][0].present && trade.offers[0][0].count==4 && trade.offers[0][0].makeIndex==5001 && !held.bSetted && trade.pendingSlot<0 && bagCount()==1 && bag.m_stInventoryItem[0].xItem.m_count==6;
            const bool quickSplit=quick.slots[0].present && quick.slots[0].count==6 && quick.slots[0].item.m_count==6;
            const bool record=trade.offers[0][0].record[62]==LOBYTE(5001) && trade.offers[0][0].record[63]==HIBYTE(5001) && trade.offers[0][0].record[71]==4;
            DWORD playing=0;LONG volume=0;const bool itemSound=SUCCEEDED(g_xSound.InspectInterfaceAudio(false,playing,volume)) && (playing&DSBSTATUS_PLAYING);
            receive(AddOk,5001,6);const bool duplicate=CExchangeWnd::Count(trade.offers[0])==1 && bagCount()==1;
            click(POINT{30,75});const bool removeRequest=capture([&](){click(POINT{550,80});},Remove,5001,0,potion.Name());receive(RemoveOk);const bool merged=!trade.offers[0][0].present && bagCount()==1 && trade.returnSlot<0;
            reset();pick(single);const bool wholeRequest=capture([&](){click(POINT{30,75});},Add,4002,1,single.Name());receive(AddOk);const bool whole=trade.offers[0][0].present && !held.bSetted && !bagCount();
            click(POINT{30,75});click(POINT{550,80});receive(RemoveFail);const bool removeFail=trade.offers[0][0].present && trade.returnSlot<0 && !trade.returnPending && !bagCount();
            click(POINT{30,75});click(POINT{550,80});receive(RemoveOk);const bool returned=bagCount()==1 && !CExchangeWnd::Count(trade.offers[0]);
            reset();pick(single);click(POINT{30,75});receive(AddFail);const bool addFail=bagCount()==1 && !held.bSetted && !trade.offerCount && trade.pendingSlot<0;
            reset();pick(potion);click(POINT{30,75});ui.FinishNotice(CLICK_NO,false);const bool quantityCancel=held.bSetted && trade.pendingSlot<0 && !trade.offerCount;
            bool invalid=true;for(const char* value:{"0","-1","+1"," 1","1 ","11","9999999999999","1x",""}) {click(POINT{30,75});SetWindowTextA(g_xChatEditBox.GetSafehWnd(),value);invalid &= capture([&](){ui.FinishNotice(CLICK_YES,false);},0,0,0,"") && trade.pendingSlot<0 && held.bSetted;}
            reset();pick(single);click(POINT{30,75});for(int i=0;i<_MAX_INVEN_ITEM;++i) bag.AddInvenItem(single,i,TRUE);receive(AddFail);const bool full=trade.pendingSlot==0 && held.bSetted && trade.offerCount==1 && !CExchangeWnd::Count(trade.offers[0]);bag.DeleteAllInvenItem();receive(AddFail);const bool recovered=bagCount()==1 && !held.bSetted && trade.pendingSlot<0;
            reset();const auto offer=CExchangeWnd::FromItem(single);for(int i=0;i<MaximumOffers;++i) CExchangeWnd::Insert(trade.offers[0],offer);pick(single);const bool limit=capture([&](){click(POINT{180,180});},0,0,0,"") && CExchangeWnd::Count(trade.offers[0])==12 && trade.pendingSlot<0;
            const auto history=ui.m_chatHistory.size();click(POINT{30,75});const bool occupied=ui.m_chatHistory.size()==history+1 && trade.pendingSlot<0;
            auto wide=offer;wide.width=2;wide.height=3;std::array<CExchangeWnd::Offer,CExchangeWnd::SlotsPerSide> grid={};grid[0]=wide;
            const bool bounds=!CExchangeWnd::Fits(grid.data(),4,wide) && !CExchangeWnd::Fits(grid.data(),195,wide) && !CExchangeWnd::Fits(grid.data(),5,offer) && CExchangeWnd::Insert(grid.data(),offer,5)==2;
            reset();receive(PeerAdd,0,0,0,peer.m_serverRecord,CExchangeWnd::RecordBytes);const bool peerAdd=CExchangeWnd::Count(trade.offers[1])==1 && trade.offers[1][0].count==8;receive(PeerRemove,0,0,0,peer.m_serverRecord,CExchangeWnd::RecordBytes);const bool peerRemove=!CExchangeWnd::Count(trade.offers[1]);
            receive(PeerAdd,0,0,0,peer.m_serverRecord,CExchangeWnd::RecordBytes-1);const bool shortRecord=!CExchangeWnd::Count(trade.offers[1]);
            click(POINT{80,290});const bool goldDialog=ui.m_xMsgBox.IsActive() && ui.m_nIdxMsgBox==GoldDialog;
            const bool goldRequest=capture([&](){SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"300");ui.FinishNotice(CLICK_YES,false);},Gold,300,0,"") && trade.goldPending && hero.m_nGlod==100000;
            receive(LocalGold,300,LOWORD(99700),HIWORD(99700));receive(PeerGold,800);const bool goldReply=trade.gold[0]==300 && trade.gold[1]==800 && hero.m_nGlod==99700 && !trade.goldPending;
            const bool goldSound=SUCCEEDED(g_xSound.InspectInterfaceAudio(false,playing,volume)) && (playing&DSBSTATUS_PLAYING);
            const bool confirm=capture([&](){click(POINT{235,285});},Confirm,0,0,"") && trade.confirmed;
            receive(PeerAdd,0,0,0,peer.m_serverRecord,CExchangeWnd::RecordBytes);receive(Complete);const bool complete=!trade.m_bActive && !bagCount() && hero.m_nGlod==99700 && !CExchangeWnd::Count(trade.offers[1]);
            reset();pick(single);click(POINT{30,75});receive(AddOk);receive(LocalGold,300,LOWORD(99700),HIWORD(99700));receive(Cancelled);const bool cancel=!trade.m_bActive && bagCount()==1 && hero.m_nGlod==100000;
            reset();pick(single);click(POINT{30,75});receive(Cancelled);const bool cancelPending=!trade.m_bActive && bagCount()==1 && !held.bSetted;
            reset();pick(single);click(POINT{30,75});receive(AddOk);for(int i=0;i<_MAX_INVEN_ITEM;++i) bag.AddInvenItem(single,i,TRUE);receive(Cancelled);const bool cancelFull=trade.m_bActive && CExchangeWnd::Count(trade.offers[0])==1;bag.DeleteAllInvenItem();receive(Cancelled);const bool cancelRecovered=!trade.m_bActive && bagCount()==1;
            reset();pick(potion);click(POINT{30,75});receive(Cancelled);const bool modalClosed=!ui.m_xMsgBox.IsActive() && !IsWindowVisible(g_xChatEditBox.GetSafehWnd()) && !trade.m_bActive && !held.bSetted && bagCount()==1;
            reset();pick(single);click(POINT{30,75});trade.Close();const bool closeRestored=!trade.m_bActive && !held.bSetted && bagCount()==1;
            reset();pick(potion);quick.slots[0]=CQuickWnd::FromItem(potion);click(POINT{30,75});SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"10");ui.FinishNotice(CLICK_YES,false);receive(AddOk);const bool quickRemoved=!quick.slots[0].present && trade.offers[0][0].count==10;
            reset();CExchangeWnd::Insert(trade.offers[0],CExchangeWnd::FromItem(single));CExchangeWnd::Insert(trade.offers[0],CExchangeWnd::FromItem(single));for(int i=0;i<_MAX_INVEN_ITEM-1;++i) bag.AddInvenItem(single,i,TRUE);receive(Cancelled);const bool atomic=trade.m_bActive && CExchangeWnd::Count(trade.offers[0])==2 && bagCount()==_MAX_INVEN_ITEM-1;
            reset();trade.Close();receive(TradeWire::Open,0,0,0,(const BYTE*)"a\0b",3);const bool nullName=!trade.m_bActive;const std::string longName(61,'a');receive(TradeWire::Open,0,0,0,(const BYTE*)longName.data(),int(longName.size()));const bool longPartner=!trade.m_bActive;
            CLoginMessages tradeCaptions;tradeCaptions.Load("./CmList.dat");receive(OpenFail);const bool openError=ui.m_xMsgBox.IsActive() && !strcmp(ui.m_xMsgBox.Text(),tradeCaptions.Get(OpenError).c_str());ui.FinishNotice(CLICK_YES,false);
            reset();pick(single);held.bIsEquipItem=TRUE;click(POINT{30,75});const bool equipmentBlocked=trade.pendingSlot<0;held.bIsEquipItem=FALSE;quick.heldSlot=0;click(POINT{30,75});const bool quickBlocked=trade.pendingSlot<0;quick.heldSlot=-1;bag.m_usePending=true;click(POINT{30,75});const bool useBlocked=trade.pendingSlot<0;bag.m_usePending=false;
            reset();pick(potion);click(POINT{30,75});SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"4");ui.FinishNotice(CLICK_YES,false);receive(AddOk,5001,6);receive(PeerAdd,0,0,0,peer.m_serverRecord,CExchangeWnd::RecordBytes);receive(LocalGold,300,LOWORD(99700),HIWORD(99700));receive(PeerGold,800);
            lstrcpyA(hero.m_szName,"MirPreview");capturePath="trade-transaction.bmp";
            if(groupPanel==PreviewTradeQuantity) {bag.SetCommonFromInven(0,&held);click(POINT{75,75});SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"2");capturePath="trade-quantity.bmp";}
            if(groupPanel==PreviewTradeCancelled) {reset();pick(single);click(POINT{30,75});receive(AddOk);receive(Cancelled);capturePath="trade-cancelled.bmp";}
            if(groupPanel==PreviewTradeGoldInput) {trade.gold[0]=0;click(POINT{80,290});SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"300");capturePath="trade-gold-input.bmp";}
            if(groupPanel==PreviewTradeOpenError) {reset();trade.Close();receive(OpenFail);capturePath="trade-open-error.bmp";}
            mouse={790,430};
            if(FILE* report=fopen("trade-transaction-check.txt","wb")) {fprintf(report,"opened=%d reopened=%d request=%d repeat=%d dead=%d direction=%d monster=%d dialog=%d add_request=%d pending=%d locked=%d wrong=%d split=%d record=%d item_sound=%d duplicate=%d remove_request=%d merged=%d whole_request=%d whole=%d remove_fail=%d returned=%d add_fail=%d quantity_cancel=%d invalid=%d full=%d recovered=%d limit=%d occupied=%d bounds=%d peer_add=%d peer_remove=%d short_record=%d gold_dialog=%d gold_request=%d gold_reply=%d gold_sound=%d confirm=%d complete=%d cancel=%d cancel_pending=%d cancel_full=%d cancel_recovered=%d modal_closed=%d close_restored=%d equipment_blocked=%d quick_blocked=%d use_blocked=%d quick_split=%d quick_removed=%d atomic=%d null_name=%d long_partner=%d open_error=%d\n",opened,reopened,request,repeat,dead,direction,monster,dialog,addRequest,pending,locked,wrong,split,record,itemSound,duplicate,removeRequest,merged,wholeRequest,whole,removeFail,returned,addFail,quantityCancel,invalid,full,recovered,limit,occupied,bounds,peerAdd,peerRemove,shortRecord,goldDialog,goldRequest,goldReply,goldSound,confirm,complete,cancel,cancelPending,cancelFull,cancelRecovered,modalClosed,closeRestored,equipmentBlocked,quickBlocked,useBlocked,quickSplit,quickRemoved,atomic,nullName,longPartner,openError);fclose(report);}
        }

        if(groupPanel>=PreviewEquipSuccess && groupPanel<=PreviewEquipTakeOff) {
            auto& ui=g_xGameProc.m_xInterface;auto& bag=ui.m_xInventoryWnd;auto& status=ui.m_xStatusWnd;auto& held=ui.m_stCommonItem;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            CLIENTITEMRCD record={};record.nMakeIndex=3001;record.stStdItem.wLooks=0;record.stStdItem.bStdMode=22;lstrcpyA(record.stStdItem.szName,"NewRing");
            CItem incoming;incoming.SetItem(record,0);lstrcpyA(incoming.m_serverName,"EquipmentLongName0123456789012");
            record.nMakeIndex=3002;lstrcpyA(record.stStdItem.szName,"OldRing");CItem old;old.SetItem(record,7);
            const POINT ring={status.m_rcWnd.left+20,status.m_rcWnd.top+205};
            const auto count=[&]() {int n=0;for(const auto& slot:bag.m_stInventoryItem) if(slot.bSetted) ++n;return n;};
            const auto reset=[&]() {bag.DeleteAllInvenItem();bag.m_usePending=false;held=COMMONITEMSET{};for(auto& slot:status.m_stEquipItem) slot=ITEMSET{};status.m_stTakeOnItemSet=ITEMSET{};status.m_equipmentRequest=0;status.m_equipmentSlot=-1;status.m_equipmentRelease=0;ui.m_xQuickWnd.heldSlot=-1;status.m_stEquipItem[7]={TRUE,old};bag.AddInvenItem(incoming,0,TRUE);};
            const auto equip=[&]() {bag.SetCommonFromInven(0,&held);return status.ReleaseEquipment(ring,1000);};
            const auto reply=[&](WORD opcode) {_TDEFAULTMESSAGE message={};message.wIdent=opcode;memcpy(&message.nRecog,&hero.m_stFeature,sizeof(LONG));char encoded[64]={};fnEncodeMessage(&message,encoded,sizeof(encoded));g_xGameProc.OnSocketMessageRecieve(encoded);};
            const auto capture=[&](auto action,WORD opcode,int instance,const char* name) {
                SOCKET receiver=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP),sender=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);sockaddr_in address={};address.sin_family=AF_INET;address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);bind(receiver,(sockaddr*)&address,sizeof(address));int size=sizeof(address);getsockname(receiver,(sockaddr*)&address,&size);connect(sender,(sockaddr*)&address,sizeof(address));DWORD timeout=60;setsockopt(receiver,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));const SOCKET saved=g_xClientSocket.m_sockClient;g_xClientSocket.m_sockClient=sender;action();g_xClientSocket.m_sockClient=saved;
                char wire[256]={};const int length=recv(receiver,wire,sizeof(wire)-1,0);closesocket(sender);closesocket(receiver);if(!opcode) return length==SOCKET_ERROR;if(length<19) return false;
                _TDEFAULTMESSAGE header={};fnDecodeMessage(&header,wire+2);wire[length-1]=0;char text[128]={};fnDecode6BitBuf(wire+2+DEFBLOCKSIZE,text,sizeof(text));
                return header.wIdent==opcode && header.nRecog==instance && header.wParam==(opcode==EquipmentAction::Quick?0:7) && !header.wTag && !header.wSeries && !strcmp(text,name);
            };
            reset();const bool request=capture([&](){equip();},CM_TAKEONITEM,3001,incoming.Name());
            const bool pending=status.m_equipmentRequest==CM_TAKEONITEM && held.bSetted && held.bWaitResponse && held.xItem.m_stItemInfo.nMakeIndex==3002 && !status.m_stEquipItem[7].bSetted;
            reply(SM_TAKEOFF_OK);const bool wrong=status.m_equipmentRequest==CM_TAKEONITEM && held.bSetted;
            reply(SM_TAKEON_OK);const bool success=status.m_stEquipItem[7].bSetted && status.m_stEquipItem[7].xItem.m_stItemInfo.nMakeIndex==3001 && !held.bSetted && !held.bWaitResponse && !held.bIsEquipItem && !status.m_equipmentRequest && count()==0;
            DWORD playing=0;LONG volume=0;const bool sound=SUCCEEDED(g_xSound.InspectInterfaceAudio(false,playing,volume)) && (playing&DSBSTATUS_PLAYING);
            BYTE returned[100]={};lstrcpyA((char*)returned,"OldRing");returned[30]=22;returned[62]=LOBYTE(3002);returned[63]=HIBYTE(3002);returned[71]=1;
            _TDEFAULTMESSAGE added={};added.wIdent=SM_ADDITEM;char addedWire[180]={};fnEncodeMessage(&added,addedWire,sizeof(addedWire));fnEncode6BitBuf(returned,addedWire+DEFBLOCKSIZE,100,sizeof(addedWire)-DEFBLOCKSIZE);g_xGameProc.OnSocketMessageRecieve(addedWire);
            const bool independent=count()==1 && bag.m_stInventoryItem[0].xItem.m_stItemInfo.nMakeIndex==3002;
            held.bSetted=TRUE;held.xItem=old;reply(SM_TAKEON_OK);const bool duplicate=held.bSetted && held.xItem.m_stItemInfo.nMakeIndex==3002;
            reset();equip();reply(SM_TAKEON_FAIL);const bool rollback=status.m_stEquipItem[7].xItem.m_stItemInfo.nMakeIndex==3002 && count()==1 && !held.bSetted;
            reset();equip();for(int i=0;i<_MAX_INVEN_ITEM;++i) bag.AddInvenItem(old,i,TRUE);reply(SM_TAKEON_FAIL);const bool full=held.bSetted && held.xItem.m_stItemInfo.nMakeIndex==3001 && !held.bIsEquipItem && !held.bWaitResponse && !status.m_equipmentRequest;
            reset();status.ReleaseEquipment(ring,1000);const bool offRequest=capture([&](){bag.OnLButtonUp(&held,POINT{bag.m_rcWnd.left+36,bag.m_rcWnd.top+79});},CM_TAKEOFFITEM,3002,old.Name());reply(SM_TAKEOFF_FAIL);const bool offFail=status.m_stEquipItem[7].bSetted && !held.bSetted && !status.m_equipmentRequest;
            status.ReleaseEquipment(ring,2000);status.RequestTakeOff();reply(SM_TAKEOFF_OK);const bool offSuccess=!status.m_stEquipItem[7].bSetted && !held.bSetted && count()==1;
            reset();status.ReleaseEquipment(ring,1000);const bool quick=capture([&](){status.ReleaseEquipment(ring,1299);},EquipmentAction::Quick,7,"") && status.m_stEquipItem[7].bSetted && !status.m_equipmentRequest;
            reset();status.ReleaseEquipment(ring,1000);const bool boundary=capture([&](){status.ReleaseEquipment(ring,1300);},0,0,"") && status.m_stEquipItem[7].bSetted && !held.bSetted;
            reset();bag.SetCommonFromInven(0,&held);const bool rapidBag=capture([&](){status.ReleaseEquipment(ring,100);},0,0,"") && held.bSetted;
            reset();bag.SetCommonFromInven(0,&held);bag.m_usePending=true;const bool useBlocked=!status.ReleaseEquipment(ring,1000);bag.m_usePending=false;ui.m_xExchangeWnd.pendingSlot=0;const bool tradeBlocked=!status.ReleaseEquipment(ring,2000);ui.m_xExchangeWnd.pendingSlot=-1;
            reset();equip();const bool otherUse=!bag.RequestUse(4000);status.m_stEquipItem[7]={TRUE,old};reply(SM_TAKEON_OK);const bool conflict=status.m_stEquipItem[0].bSetted && status.m_stEquipItem[0].xItem.m_stItemInfo.nMakeIndex==3001 && status.m_stEquipItem[7].xItem.m_stItemInfo.nMakeIndex==3002;
            reset();equip();for(auto& slot:status.m_stEquipItem) slot={TRUE,old};reply(SM_TAKEON_OK);const bool allOccupied=status.m_equipmentRequest==CM_TAKEONITEM && status.m_stTakeOnItemSet.bSetted;status.m_stEquipItem[3]=ITEMSET{};reply(SM_TAKEON_OK);const bool recovered=!status.m_equipmentRequest && status.m_stEquipItem[3].xItem.m_stItemInfo.nMakeIndex==3001;
            reset();bag.SetCommonFromInven(0,&held);const bool invalid=!status.ReleaseEquipment(POINT{status.m_rcWnd.left+100,status.m_rcWnd.top+120},1000) && held.bSetted;
            reset();status.m_stEquipItem[7]=ITEMSET{};equip();reply(SM_TAKEON_OK);const bool emptySlot=status.m_stEquipItem[7].bSetted && !held.bSetted && count()==0;
            reset();held.bSetted=TRUE;record.stStdItem.bStdMode=5;held.xItem.SetItem(record,0);const auto notices=ui.m_chatHistory.size();status.ReleaseEquipment(POINT{status.m_rcWnd.left+50,status.m_rcWnd.top+80},100);
            const bool rejected=ui.m_chatHistory.size()==notices+1 && status.m_equipmentRequest==CM_TAKEONITEM;reply(SM_TAKEON_FAIL);
            reset();equip();reply(groupPanel==PreviewEquipFailure?SM_TAKEON_FAIL:SM_TAKEON_OK);
            if(groupPanel==PreviewEquipTakeOff) {status.ReleaseEquipment(ring,2000);status.RequestTakeOff();reply(SM_TAKEOFF_OK);}
            const bool finalState=!status.m_equipmentRequest && !held.bSetted && count()==(groupPanel==PreviewEquipFailure?1:0) && bool(status.m_stEquipItem[7].bSetted)==(groupPanel!=PreviewEquipTakeOff);
            if(!status.m_bActive) ui.WindowActivate(_WND_ID_STATUS);if(!bag.m_bActive) ui.WindowActivate(_WND_ID_INVENTORY);
            capturePath=groupPanel==PreviewEquipFailure?"equipment-rollback.bmp":groupPanel==PreviewEquipTakeOff?"equipment-takeoff.bmp":"equipment-transaction.bmp";
            if(FILE* report=fopen("equipment-transaction-check.txt","wb")) {fprintf(report,"request=%d pending=%d wrong=%d success=%d sound=%d duplicate=%d rollback=%d full=%d off_request=%d off_fail=%d off_success=%d quick=%d boundary=%d rapid_bag=%d use_blocked=%d trade_blocked=%d other_use=%d conflict=%d occupied=%d recovered=%d invalid=%d independent=%d empty_slot=%d rejected_notice=%d final_state=%d\n",request,pending,wrong,success,sound,duplicate,rollback,full,offRequest,offFail,offSuccess,quick,boundary,rapidBag,useBlocked,tradeBlocked,otherUse,conflict,allOccupied,recovered,invalid,independent,emptySlot,rejected,finalState);fclose(report);}
        }
        if(groupPanel==PreviewStorageReply) {
            auto& ui=g_xGameProc.m_xInterface;auto& bag=ui.m_xInventoryWnd;auto& shop=ui.m_xStoreWnd;auto& quick=ui.m_xQuickWnd;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            CLIENTITEMRCD record={};record.nMakeIndex=1001;record.stStdItem.wLooks=0;lstrcpyA(record.stStdItem.szName,"StoredPotion");
            CItem item;item.SetItem(record,0);item.m_stackMode=3;item.SetCount(4);bag.AddInvenItem(item,0,TRUE);quick.slots[0]=CQuickWnd::FromItem(item);
            const std::vector<CStoreWnd::Offer> list={{"StoredPotion",0,0,1001,30,3}};
            const auto begin=[&]() {ui.m_xMsgBox.HideMessageBox();shop.SetStorage(77,list);shop.offerIndex=0;shop.Withdraw(2);};
            const auto receive=[&](WORD opcode,WORD stored,WORD owned,bool extra=false) {_TDEFAULTMESSAGE packet={};packet.wIdent=opcode;packet.wParam=stored;packet.wTag=owned;char encoded[64]={};fnEncodeMessage(&packet,encoded,sizeof(encoded));if(extra) strcat(encoded,"x");g_xGameProc.OnSocketMessageRecieve(encoded);};
            begin();receive(SM_STORAGE_WITHDRAW_OK,28,6,true);const bool malformed=shop.storagePending==1001 && shop.offers[0].count==30 && bag.m_stInventoryItem[0].xItem.m_count==4;
            receive(SM_STORAGE_WITHDRAW_OK,28,6);const bool partial=!shop.storagePending && shop.offers[0].count==28 && bag.m_stInventoryItem[0].xItem.m_count==6 && quick.slots[0].count==6;
            const auto history=ui.m_chatHistory.size();receive(SM_STORAGE_WITHDRAW_OK,1,99);const bool duplicate=shop.offers[0].count==28 && bag.m_stInventoryItem[0].xItem.m_count==6 && ui.m_chatHistory.size()==history;
            begin();receive(SM_STORAGE_WITHDRAW_OK,0,8);const bool all=shop.offers.empty() && shop.offerIndex==-1 && shop.page==0 && bag.m_stInventoryItem[0].xItem.m_count==8 && quick.slots[0].count==8;
            begin();receive(SM_STORAGE_WITHDRAW_OK,20,0);const bool zero=shop.offers[0].count==20 && bag.m_stInventoryItem[0].xItem.m_count==8;
            begin();receive(SM_STORAGE_WITHDRAW_FAIL,0,0);const bool failed=!shop.storagePending && shop.offers[0].count==30 && ui.m_xMsgBox.IsActive() && bag.m_stInventoryItem[0].xItem.m_count==8;
            begin();receive(SM_STORAGE_BAG_FULL,0,0);const bool full=!shop.storagePending && ui.m_xMsgBox.IsActive() && quick.slots[0].count==8;
            begin();bag.SetCommonFromInven(0,&ui.m_stCommonItem);receive(SM_STORAGE_WITHDRAW_OK,20,10);const bool held=ui.m_stCommonItem.xItem.m_count==10;bag.SetInvenFromCommon(0,&ui.m_stCommonItem);
            begin();shop.Reset();receive(SM_STORAGE_WITHDRAW_OK,0,20);const bool closed=bag.m_stInventoryItem[0].xItem.m_count==10;
            begin();receive(SM_STORAGE_WITHDRAW_OK,15,15);
            if(!shop.m_bActive) ui.WindowActivate(_WND_ID_STORE);if(!bag.m_bActive) ui.WindowActivate(_WND_ID_INVENTORY);if(!quick.m_bActive) ui.WindowActivate(_WND_ID_QUICK);
            capturePath="storage-withdraw-reply.bmp";
            if(FILE* report=fopen("storage-withdraw-check.txt","wb")) {fprintf(report,"malformed=%d partial=%d duplicate=%d all=%d zero=%d failed=%d full=%d held=%d closed=%d\n",malformed,partial,duplicate,all,zero,failed,full,held,closed);fclose(report);}
        }
        if(groupPanel>=PreviewServiceRepair && groupPanel<=PreviewServiceFailure) {
            using namespace MerchantService;
            auto& ui=g_xGameProc.m_xInterface;auto& bag=ui.m_xInventoryWnd;auto& quick=ui.m_xQuickWnd;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            CLIENTITEMRCD record={};record.nMakeIndex=1001;record.stStdItem.wLooks=0;record.nDura=1000;record.nDuraMax=5000;lstrcpyA(record.stStdItem.szName,"ServicePotion");
            CItem item;item.SetItem(record,0);item.m_stackMode=3;item.SetCount(8);lstrcpyA(item.m_serverName,"ServicePotionLongName012345678");
            const auto begin=[&](int mode) {ui.m_xMsgBox.HideMessageBox();bag.DeleteAllInvenItem();bag.AddInvenItem(item,0,TRUE);quick.slots[0]=CQuickWnd::FromItem(item);bag.OpenService(77,mode);bag.SelectServiceItem(0);if(mode!=Deposit) bag.QuoteService(125,mode==Sell);bag.ConfirmService(mode==Repair?0:2);};
            const auto receive=[&](WORD opcode,DWORD balance,WORD first,WORD second,bool extra=false) {_TDEFAULTMESSAGE packet={};packet.wIdent=opcode;packet.nRecog=balance;packet.wParam=first;packet.wTag=second;char encoded[64]={};fnEncodeMessage(&packet,encoded,sizeof(encoded));if(extra) strcat(encoded,"x");g_xGameProc.OnSocketMessageRecieve(encoded);};
            begin(Sell);receive(StackSaleOk,240,3,5,true);const bool malformed=bag.service.phase==CInventoryWnd::ServicePhase::Paying && bag.m_stInventoryItem[0].xItem.m_count==8;
            receive(StackSaleOk,240,3,5);const bool sold=hero.m_nGlod==240 && bag.m_stInventoryItem[0].xItem.m_count==3 && quick.slots[0].count==3 && bag.service.phase==CInventoryWnd::ServicePhase::Choosing && !bag.service.price;
            const auto history=ui.m_chatHistory.size();receive(StackSaleOk,300,1,2);const bool repeated=hero.m_nGlod==300 && ui.m_chatHistory.size()==history && bag.m_stInventoryItem[0].xItem.m_count==3;
            begin(Sell);receive(StackSaleOk,600,0,8);const bool all=!bag.m_stInventoryItem[0].bSetted && !quick.slots[0].present && bag.ItemAnchor(0)==-1;
            begin(Sell);receive(SM_USERSELLITEM_OK,660,0,0);const bool ordinary=!bag.m_stInventoryItem[0].bSetted && quick.slots[0].present && hero.m_nGlod==660;
            begin(Sell);receive(StackSaleFail,0,0,0);const bool stackFail=bag.m_stInventoryItem[0].xItem.m_count==8 && !ui.m_xMsgBox.IsActive() && bag.service.phase==CInventoryWnd::ServicePhase::Choosing;
            begin(Sell);receive(SM_USERSELLITEM_FAIL,0,0,0);const bool saleFail=ui.m_xMsgBox.IsActive() && bag.m_stInventoryItem[0].xItem.m_count==8;
            begin(Repair);receive(RepairOk,880,5500,6000);const auto& repaired=bag.m_stInventoryItem[0].xItem;
            const bool repair=hero.m_nGlod==880 && repaired.m_stItemInfo.nDura==5500 && repaired.m_stItemInfo.nDuraMax==6000 && repaired.m_serverRecord[66]==LOBYTE(5500) && repaired.m_serverRecord[67]==HIBYTE(5500) && bag.service.phase==CInventoryWnd::ServicePhase::Choosing;
            begin(Repair);receive(RepairFail,0,0,0);const bool repairFail=ui.m_xMsgBox.IsActive() && bag.m_stInventoryItem[0].xItem.m_stItemInfo.nDura==1000 && !bag.service.price;
            begin(Deposit);receive(DepositOk,0,3,5);const bool deposit=bag.m_stInventoryItem[0].xItem.m_count==3 && quick.slots[0].count==3 && bag.service.phase==CInventoryWnd::ServicePhase::Choosing;
            begin(Deposit);receive(DepositOk,0,0,8);const bool storedAll=!bag.m_stInventoryItem[0].bSetted && !quick.slots[0].present;
            begin(Deposit);receive(StorageFull,0,0,0);const bool full=ui.m_xMsgBox.IsActive() && bag.m_stInventoryItem[0].xItem.m_count==8 && bag.service.phase==CInventoryWnd::ServicePhase::Choosing;
            begin(Deposit);receive(DepositFail,0,0,0);const bool depositFail=ui.m_xMsgBox.IsActive() && quick.slots[0].count==8 && bag.service.phase==CInventoryWnd::ServicePhase::Choosing;
            begin(Deposit);receive(RepairOk,777,1,1);const bool mismatch=hero.m_nGlod==777 && bag.service.phase==CInventoryWnd::ServicePhase::Paying && bag.m_stInventoryItem[0].xItem.m_stItemInfo.nDura==1000;
            bag.ClearService();receive(RepairOk,760,3000,6000);const bool late=hero.m_nGlod==760 && bag.m_stInventoryItem[0].xItem.m_stItemInfo.nDura==1000;
            ui.m_xMsgBox.HideMessageBox();receive(RepairFail,0,0,0);const bool lateFailure=ui.m_xMsgBox.IsActive();
            const int mode=groupPanel==PreviewServiceRepair?Repair:groupPanel==PreviewServiceSell?Sell:Deposit;begin(mode);
            receive(groupPanel==PreviewServiceFailure?StorageFull:mode==Repair?RepairOk:mode==Sell?StackSaleOk:DepositOk,900,mode==Repair?5500:3,mode==Repair?6000:5);
            if(!quick.m_bActive) ui.WindowActivate(_WND_ID_QUICK);
            capturePath=groupPanel==PreviewServiceRepair?"service-repair-reply.bmp":groupPanel==PreviewServiceSell?"service-sell-reply.bmp":groupPanel==PreviewServiceDeposit?"service-deposit-reply.bmp":"service-failure-reply.bmp";
            if(FILE* report=fopen("service-reply-check.txt","wb")) {fprintf(report,"malformed=%d sold=%d repeated=%d all=%d ordinary=%d stack_fail=%d sale_fail=%d repair=%d repair_fail=%d deposit=%d stored_all=%d full=%d deposit_fail=%d mismatch=%d late=%d late_failure=%d\n",malformed,sold,repeated,all,ordinary,stackFail,saleFail,repair,repairFail,deposit,storedAll,full,depositFail,mismatch,late,lateFailure);fclose(report);}
        }
        if(groupPanel==PreviewItemUse || groupPanel==PreviewItemUseRestored) {
            auto& ui=g_xGameProc.m_xInterface;auto& bag=ui.m_xInventoryWnd;auto& quick=ui.m_xQuickWnd;auto& held=ui.m_stCommonItem;
            auto& icons=g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY];icons.NewLoad("./Data/Inventory.wil");
            int icon=0;for(;icon<100;++icon) if(icons.NewSetIndex(icon) && icons.m_lpstNewCurrWilImageInfo->shWidth>0 && icons.m_lpstNewCurrWilImageInfo->shWidth<=38 && icons.m_lpstNewCurrWilImageInfo->shHeight<=38) break;
            CLIENTITEMRCD record={};record.nMakeIndex=2345;record.stStdItem.wLooks=icon;record.stStdItem.bStdMode=0;lstrcpyA(record.stStdItem.szName,"QuickMedicine");
            CItem item;item.SetItem(record,0);item.m_stackMode=3;item.SetCount(8);
            const auto reset=[&]() {bag.DeleteAllInvenItem();for(auto& slot:quick.slots) slot=CQuickWnd::Slot{};quick.heldSlot=-1;quick.heldReference=CQuickWnd::Slot{};ZeroMemory(&held,sizeof(held));bag.m_usePending=false;bag.m_lastUse=bag.m_lastRelease=0;bag.AddInvenItem(item,0,TRUE);quick.slots[0]=CQuickWnd::FromItem(item);};
            const auto receive=[&](WORD opcode,WORD remaining) {_TDEFAULTMESSAGE message={};message.wIdent=opcode;message.wParam=remaining;char encoded[64]={};fnEncodeMessage(&message,encoded,sizeof(encoded));g_xGameProc.OnSocketMessageRecieve(encoded);};
            const auto capture=[&](auto action,DWORD tick,bool expected) {
                SOCKET receiver=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP),sender=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
                sockaddr_in address={};address.sin_family=AF_INET;address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);bind(receiver,(sockaddr*)&address,sizeof(address));int size=sizeof(address);getsockname(receiver,(sockaddr*)&address,&size);connect(sender,(sockaddr*)&address,sizeof(address));
                DWORD timeout=60;setsockopt(receiver,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));const SOCKET saved=g_xClientSocket.m_sockClient;g_xClientSocket.m_sockClient=sender;action();g_xClientSocket.m_sockClient=saved;
                char wire[256]={};const int length=recv(receiver,wire,sizeof(wire)-1,0);closesocket(sender);closesocket(receiver);
                if(!expected) return length==SOCKET_ERROR;
                if(length<=19) return false;_TDEFAULTMESSAGE header={};fnDecodeMessage(&header,wire+2);
                const bool stamp=tick?MAKELONG(header.wParam,header.wTag)==tick:MAKELONG(header.wParam,header.wTag)!=0;
                char name[128]={};wire[length-1]=0;fnDecode6BitBuf(wire+2+DEFBLOCKSIZE,name,sizeof(name));
                return header.wIdent==CM_EAT && header.nRecog==2345 && stamp && !header.wSeries && !strcmp(name,"QuickMedicine");
            };
            reset();const DWORD tick=0x12345678;
            const bool wire=capture([&](){quick.Activate(0,tick);},tick,true) && bag.m_usePending && held.bWaitResponse && !quick.slots[0].present && bag.m_stInventoryItem[0].bSetted;
            const bool duplicate=capture([&](){quick.Activate(0,tick+3000);},0,false);
            receive(SM_EAT_OK,5);const bool remaining=quick.slots[0].count==5 && bag.m_stInventoryItem[0].xItem.m_count==5 && !held.bSetted && !bag.m_usePending;
            DWORD playing=0;LONG volume=0;const bool useSound=SUCCEEDED(g_xSound.InspectInterfaceAudio(false,playing,volume)) && (playing&DSBSTATUS_PLAYING);
            const auto history=ui.m_chatHistory.size();receive(SM_EAT_OK,5);const bool repeated=ui.m_chatHistory.size()==history;
            const bool boundary=capture([&](){quick.Activate(0,tick+2000);},0,false);
            const bool next=capture([&](){quick.Activate(0,tick+2001);},tick+2001,true);receive(SM_EAT_FAIL,0);
            const bool failed=quick.slots[0].count==5 && bag.m_stInventoryItem[0].xItem.m_count==5 && !held.bSetted;
            reset();quick.slots[0].item.SetCount(1);quick.slots[0].count=1;bag.m_stInventoryItem[0].xItem.SetCount(1);
            quick.Activate(0,3000);receive(SM_EAT_OK,0);const bool last=!quick.slots[0].present && !bag.m_stInventoryItem[0].bSetted;
            reset();bag.m_lastUse=0xfffffff0;const bool wrap=capture([&](){quick.Activate(0,0x800);},0x800,true);receive(SM_EAT_FAIL,0);
            CItem fast=item;fast.m_stItemInfo.stStdItem.wLooks=20;
            const bool fastBoundary=!bag.UseAllowed(fast,1000) && bag.UseAllowed(fast,1001) && !bag.UseAllowed(item,2000) && bag.UseAllowed(item,2001);
            reset();const bool key=capture([&](){ui.OnKeyDown('1',0);},0,true);const bool keyup=capture([&](){ui.OnKeyUp('1',0);},0,false);receive(SM_EAT_FAIL,0);
            bag.m_lastUse=0;const bool repeat=capture([&](){ui.OnKeyDown('1',1L<<30);},0,false);
            ui.m_xExchangeWnd.pendingSlot=0;const bool trade=capture([&](){quick.Activate(0,4000);},0,false);ui.m_xExchangeWnd.pendingSlot=-1;
            ui.m_xStatusWnd.m_stTakeOnItemSet.bSetted=TRUE;const bool equipment=capture([&](){quick.Activate(0,4000);},0,false);ui.m_xStatusWnd.m_stTakeOnItemSet.bSetted=FALSE;
            reset();quick.Release(0);const bool mouse=capture([&](){quick.Activate(0,4000,true);},4000,true);receive(SM_EAT_FAIL,0);
            reset();bag.SetCommonFromInven(0,&held);const bool bagRequest=capture([&](){bag.RequestUse(6000);},6000,true);receive(SM_EAT_OK,3);
            const bool bagRemaining=bag.m_stInventoryItem[0].xItem.m_count==3 && quick.slots[0].count==3 && !held.bSetted;
            const bool shared=capture([&](){quick.Activate(0,8000);},0,false);
            reset();bag.SetCommonFromInven(0,&held);bag.RequestUse(9000);receive(SM_EAT_FAIL,0);const bool bagFailed=bag.m_stInventoryItem[0].xItem.m_count==8 && !held.bSetted;
            reset();quick.Activate(0,10000);for(auto& slot:quick.slots) slot=CQuickWnd::FromItem(item);
            receive(SM_EAT_FAIL,0);const bool full=held.bSetted && bag.m_usePending;quick.slots[2]=CQuickWnd::Slot{};receive(SM_EAT_FAIL,0);const bool fallback=quick.slots[2].present && !held.bSetted;
            reset();ui.ShowNotice(1066);const bool modal=capture([&](){g_xGameProc.OnKeyDown('1',0);g_xGameProc.OnKeyUp('1',0);},0,false);ui.m_xMsgBox.HideMessageBox();
            BYTE keys[256]={},savedKeys[256]={};GetKeyboardState(savedKeys);memcpy(keys,savedKeys,sizeof(keys));keys[VK_SHIFT]=0x80;SetKeyboardState(keys);
            const bool shift=capture([&](){ui.OnKeyDown('1',0);},0,false);SetKeyboardState(savedKeys);
            reset();const POINT bagPoint={bag.m_rcWnd.left+36,bag.m_rcWnd.top+79};bag.m_lastRelease=GetTickCount()-301;
            bag.OnLButtonUp(&held,bagPoint);
            const bool bagClick=capture([&](){bag.OnLButtonUp(&held,bagPoint);},0,true);receive(SM_EAT_FAIL,0);
            reset();quick.Activate(0,12000);receive(groupPanel==PreviewItemUseRestored?SM_EAT_FAIL:SM_EAT_OK,3);
            ui.WindowActivate(_WND_ID_INVENTORY);ui.WindowActivate(_WND_ID_QUICK);
            capturePath=groupPanel==PreviewItemUseRestored?"item-use-restored.bmp":"item-use-chain.bmp";
            if(FILE* report=fopen("item-use-chain-check.txt","wb")) {fprintf(report,"wire=%d pending_duplicate=%d remaining=%d repeated_reply=%d boundary=%d next=%d failed=%d last=%d wrap=%d fast_boundary=%d key=%d keyup=%d repeat=%d trade=%d equipment=%d mouse=%d bag_request=%d bag_remaining=%d shared=%d bag_failed=%d full=%d fallback=%d modal=%d shift=%d bag_click=%d use_sound=%d\n",wire,duplicate,remaining,repeated,boundary,next,failed,last,wrap,fastBoundary,key,keyup,repeat,trade,equipment,mouse,bagRequest,bagRemaining,shared,bagFailed,full,fallback,modal,shift,bagClick,useSound);fclose(report);}
        }
        if(groupPanel==PreviewInterfaceAudio) {
            auto& ui=g_xGameProc.m_xInterface;auto& options=ui.m_xOptionWnd;
            options.LoadAudioOptions(nullptr);ui.WindowActivate(_WND_ID_OPTION);
            DWORD playing=0;LONG volume=0;
            const auto playingAt=[&](bool opening,LONG expected) {return SUCCEEDED(g_xSound.InspectInterfaceAudio(opening,playing,volume)) && playing>0 && volume==expected;};
            const bool device=g_xSound.GetSoundObject()->GetDS()!=nullptr;
            const bool click=g_xSound.PlayLobbyEffect("105.wav") && playingAt(false,0);
            g_xSound.SetInterfaceVolume(false,-50);const bool volumeApplied=playingAt(false,-2000);
            const LONG music=g_xSound.m_lMp3MaxVol;g_xSound.SetInterfaceVolume(false,-100);const bool minimum=playingAt(false,-4000) && g_xSound.m_lMp3MaxVol==music;
            const POINT toggle={options.m_rcWnd.left+432,options.m_rcWnd.top+147};
            options.OnLButtonDown(toggle);options.OnLButtonUp(toggle);g_xSound.InspectInterfaceAudio(false,playing,volume);
            const bool muted=!options.m_values[2] && !playing && !g_xSound.PlayLobbyEffect("105.wav");
            options.OnLButtonDown(toggle);options.OnLButtonUp(toggle);const bool enabled=options.m_values[2] && g_xSound.PlayLobbyEffect("105.wav") && playingAt(false,-4000);
            g_xSound.SetInterfaceVolume(false,0);
            const POINT reverse={options.m_rcWnd.left+432,options.m_rcWnd.top+207};
            options.OnLButtonDown(reverse);options.OnLButtonUp(reverse);const bool button=playingAt(false,0);
            const bool missing=!g_xSound.PlayLobbyEffect("missing-preview-sample.wav");g_xSound.InspectInterfaceAudio(false,playing,volume);const bool missingStopped=!playing;
            bool voices=true;int voiceCount=0;
            for(int creation=0;creation<2;++creation) for(int gender=0;gender<2;++gender) for(int profession=0;profession<3;++profession) {
                g_xSound.SetDisablePlay();g_xSound.SetEnablePlay();g_xSound.PlayRoleVoice(creation!=0,profession,gender);voices=playingAt(false,0) && voices;++voiceCount;
            }
            g_xSound.SetDisablePlay();g_xSound.SetEnablePlay();g_xSound.PlayUseItemEffect(0);const bool item=playingAt(false,0);
            g_xSound.UpdateLobbyAudio(CMirSound::LobbyPage::Credentials,0);g_xSound.UpdateLobbyAudio(CMirSound::LobbyPage::Credentials,1200);
            g_xSound.InspectInterfaceAudio(true,playing,volume);const bool delayed=!playing;
            g_xSound.UpdateLobbyAudio(CMirSound::LobbyPage::Credentials,1);const bool opening=playingAt(true,0);
            g_xSound.SetInterfaceVolume(false,-25);const bool openingVolume=playingAt(true,-1000);
            g_xSound.SetDisablePlay();g_xSound.InspectInterfaceAudio(true,playing,volume);const bool openingStopped=!playing;
            g_xSound.UpdateLobbyAudio(CMirSound::LobbyPage::None,0);g_xSound.SetEnablePlay();
            options.m_effectsLevel=-25;options.m_values[2]=true;mouse={790,430};capturePath="interface-audio.bmp";
            if(FILE* report=fopen("interface-audio-check.txt","wb")) {fprintf(report,"device=%d click=%d volumeApplied=%d minimum=%d muted=%d enabled=%d button=%d missing=%d missingStopped=%d voices=%d item=%d delayed=%d opening=%d openingVolume=%d openingStopped=%d voiceCount=%d\n",device,click,volumeApplied,minimum,muted,enabled,button,missing,missingStopped,voices,item,delayed,opening,openingVolume,openingStopped,voiceCount);fclose(report);}
        }

        if(groupPanel==PreviewNotice || groupPanel==PreviewNoticeMoved) {
            auto& process=g_xGameProc;auto& notice=process.m_xNotice;
            notice.Load(&process.m_xImage.m_xImageList[_IMAGE_INTER]);
            const std::string text="欢迎来到珐玛大陆！\n请妥善保管您的账号和密码。\n游戏公告：今晚八点开放行会活动，请各位勇士提前准备药水和回城卷轴。\n祝您游戏愉快！";
            const auto receive=[&](const std::string& value) {
                _TDEFAULTMESSAGE header={};header.wIdent=SM_SENDNOTICE;char wire[12000]={};fnEncodeMessage(&header,wire,sizeof(wire));fnEncode6BitBuf((BYTE*)value.data(),wire+DEFBLOCKSIZE,int(value.size()),sizeof(wire)-DEFBLOCKSIZE);process.OnSocketMessageRecieve(wire);
            };
            receive(text);const auto first=notice.Lines();const RECT original=notice.Bounds();
            const bool opened=notice.IsActive() && first.size()>3 && first.front()=="欢迎来到珐玛大陆！";
            receive("正在显示时不替换");const bool retained=notice.Lines()==first;
            const POINT button={original.left+133,original.top+431};
            const auto capture=[&](auto action,int expected) {
                SOCKET receiver=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP),sender=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
                sockaddr_in address={};address.sin_family=AF_INET;address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);bind(receiver,(sockaddr*)&address,sizeof(address));int size=sizeof(address);getsockname(receiver,(sockaddr*)&address,&size);connect(sender,(sockaddr*)&address,sizeof(address));
                DWORD timeout=60;setsockopt(receiver,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));const SOCKET saved=g_xClientSocket.m_sockClient;g_xClientSocket.m_sockClient=sender;action();g_xClientSocket.m_sockClient=saved;
                char wire[128]={};const int length=recv(receiver,wire,sizeof(wire)-1,0);closesocket(sender);closesocket(receiver);
                if(!expected) return length==SOCKET_ERROR;
                _TDEFAULTMESSAGE header={};if(length!=19) return false;fnDecodeMessage(&header,wire+2);return header.wIdent==CM_LOGINNOTICEOK && !header.nRecog && !header.wParam && !header.wTag && !header.wSeries;
            };
            const bool stray=capture([&](){notice.OnButtonUp(button);},0);
            const bool down=capture([&](){notice.OnButtonDown(button);},0);
            const bool outside=capture([&](){notice.OnButtonUp(POINT{799,0});},0);
            const bool click=capture([&](){notice.OnButtonDown(button);process.OnLButtonUp(0,MAKELPARAM(button.x,button.y));},1) && !notice.IsActive();
            receive(text);const bool modal=capture([&](){process.OnKeyDown(VK_ESCAPE,0);},0) && notice.IsActive();
            const bool enter=capture([&](){process.OnKeyDown(VK_RETURN,0);},1) && !notice.IsActive();
            receive("");const bool empty=!notice.IsActive();
            receive(std::string(2000,'W'));const bool capped=notice.Lines().size()==21;notice.HideMessageBox();receive(text);
            const POINT start={original.left+20,original.top+30},end={start.x+25,start.y-15};notice.OnButtonDown(start);notice.MoveWnd(end);notice.OnButtonUp(end);
            const RECT moved=notice.Bounds();const bool dragged=moved.left==original.left+25 && moved.top==original.top-15;
            if(groupPanel==PreviewNotice) {notice.HideMessageBox();receive(text);}
            noticePreview=true;mouse={790,430};capturePath=groupPanel==PreviewNotice?"server-notice.bmp":"server-notice-moved.bmp";
            if(FILE* report=fopen("server-notice-check.txt","wb")) {fprintf(report,"opened=%d retained=%d stray=%d down=%d outside=%d click=%d modal=%d enter=%d empty=%d capped=%d dragged=%d\n",opened,retained,stray,down,outside,click,modal,enter,empty,capped,dragged);fclose(report);}
        }

        if(groupPanel>=PreviewSpeechSingle && groupPanel<=PreviewSpeechEight) {
            auto& process=g_xGameProc;auto& hero=process.m_xMyHero;hero.m_dwIdentity=123;
            hero.m_shScrnPosX=380;hero.m_shScrnPosY=300;
            const auto receive=[&](WORD opcode,DWORD id,const std::string& text) {
                _TDEFAULTMESSAGE header={};header.wIdent=opcode;header.nRecog=id;header.wParam=2;
                char wire[2048]={};fnEncodeMessage(&header,wire,sizeof(wire));fnEncode6BitBuf((BYTE*)text.data(),wire+DEFBLOCKSIZE,int(text.size()),sizeof(wire)-DEFBLOCKSIZE);process.OnSocketMessageRecieve(wire);
            };
            receive(SM_HEAR,123,"你好，勇士。");const RECT single=hero.SpeechBounds();
            const bool routed=hero.m_nDividedChatLine==1 && std::string(hero.m_szChatMsg)=="你好，勇士。";
            const bool geometry=single.left==408-hero.m_speechWidths[0]/2 && single.top==227 && single.bottom==245 && hero.SpeechPosition(0).y==231;
            receive(SM_GROUPMESSAGE,123,"不会覆盖气泡");const bool channels=std::string(hero.m_szChatMsg)=="你好，勇士。";
            std::string longLine;for(int i=0;i<16;++i) longLine+="中文换行";receive(SM_HEAR,123,longLine);
            std::string combined;bool widths=true;for(int i=0;i<hero.m_nDividedChatLine;++i) {combined+=hero.m_szChatMsgArg[i];widths=widths && hero.m_speechWidths[i]<=144;}
            const bool wrapped=combined==longLine && hero.m_nDividedChatLine>1;
            const auto last=hero.SpeechPosition(hero.m_nDividedChatLine-1);const bool centered=last.x==336+(144-hero.m_speechWidths[hero.m_nDividedChatLine-1])/2 && last.y==231;
            hero.ShowMessage(3000);const bool boundary=hero.m_nDividedChatLine>0;hero.ShowMessage(1);const bool expired=!hero.m_nDividedChatLine && !hero.m_szChatMsg[0];
            std::string eight;for(int i=1;i<=259;++i) {std::string candidate(i,'W');if(!hero.ReceiveSpeech(candidate.c_str())) break;if(hero.m_nDividedChatLine==8) eight=candidate;}
            hero.ReceiveSpeech(eight.c_str());const bool eightRows=hero.m_nDividedChatLine==8;
            const bool oversized=!hero.ReceiveSpeech(std::string(260,'W').c_str()) && hero.m_nDividedChatLine==8;
            const bool excessRows=!hero.ReceiveSpeech(std::string(259,'W').c_str()) && std::string(hero.m_szChatMsg)==eight;
            receive(SM_HEAR,123,"");const bool cleared=!hero.m_nDividedChatLine;
            auto* remote=new CActor;remote->m_dwIdentity=456;remote->m_stFeature.bGender=_GENDER_MON;process.m_xActorList.AddNode(remote);
            receive(SM_MONSTERSAY,456,"怪物发言");const bool monster=std::string(remote->m_szChatMsg)=="怪物发言" && remote->m_nDividedChatLine==1;
            receive(SM_MONSTERSAY,789,"未知对象");const bool unknown=process.m_xActorList.GetCounter()==1 && !hero.m_nDividedChatLine;
            process.m_xActorList.MoveCurrentToTop();process.m_xActorList.DeleteCurrentNodeEx();delete remote;
            const std::string display=groupPanel==PreviewSpeechSingle?"你好，勇士。":groupPanel==PreviewSpeechEight?eight:"请大家带好药水和回城卷轴，今晚八点在沙巴克城门集合。";
            receive(SM_HEAR,123,display);speechPreview=true;mouse={790,430};
            capturePath=groupPanel==PreviewSpeechSingle?"speech-single.bmp":groupPanel==PreviewSpeechEight?"speech-eight.bmp":"speech-multiline.bmp";
            if(FILE* report=fopen("speech-check.txt","wb")) {fprintf(report,"routed=%d geometry=%d channels=%d widths=%d wrapped=%d centered=%d boundary=%d expired=%d eightRows=%d oversized=%d excessRows=%d cleared=%d monster=%d unknown=%d\n",routed,geometry,channels,widths,wrapped,centered,boundary,expired,eightRows,oversized,excessRows,cleared,monster,unknown);fclose(report);}
        }

        if(groupPanel==PreviewChatHistory || groupPanel==PreviewChatHistoryExpanded) {
            auto& ui=g_xGameProc.m_xInterface;
            const auto reset=[&]() {ui.m_chatHistory.clear();ui.m_chatLines[0].clear();ui.m_chatLines[1].clear();ui.m_chatFirst[0]=ui.m_chatFirst[1]=ui.m_nFstLine=0;ui.m_chatExpanded=false;ui.m_chatLayoutMode=0;ui.LayoutChat();};
            const auto post=[&](const std::string& value,DWORD color=0xffffff,DWORD background=0) {ui.MsgAdd(color,background,const_cast<char*>(value.c_str()));};
            reset();std::string longLine;for(int i=0;i<16;++i) longLine+="中文换行";post(longLine,0x00ff00,0x000080);
            bool wrapped=true,colors=true;for(int mode=0;mode<2;++mode) {std::string value;for(const auto& row:ui.m_chatLines[mode]) {value+=row.text;colors=colors && row.color==0x00ff00 && row.background==0x000080;}wrapped=wrapped && value==longLine;}
            const bool widths=ui.m_chatLines[0].size()>ui.m_chatLines[1].size();
            reset();post("first`second");const bool separators=ui.m_chatLines[0].size()==1 && ui.m_chatLines[0][0].text=="first";
            reset();post(std::string(200,'W'));const bool accepted=!ui.m_chatLines[0].empty();const auto count=ui.m_chatLines[0].size();post(std::string(201,'W'));post("");const bool limit=ui.m_chatLines[0].size()==count;
            reset();for(int i=0;i<90;++i) {char row[40];sprintf(row,"历史消息 %02d",i);post(row);}
            const bool retained=ui.m_chatLines[0].size()==79 && ui.m_chatLines[1].size()==79 && ui.m_chatLines[0].front().text=="历史消息 11";
            const bool following=ui.m_nFstLine==74 && ui.m_chatFirst[1]==65;
            ui.m_nFstLine=5;post("正在查看旧消息时收到新消息");const bool preserved=ui.m_nFstLine==4;
            ui.m_chatExpanded=true;ui.LayoutChat();const bool expanded=ui.m_nFstLine==65 && ui.m_xChat.GetCounter()==79;
            ui.m_nFstLine=7;ui.m_chatExpanded=false;ui.LayoutChat();const bool compact=ui.m_nFstLine==4;
            ui.m_chatExpanded=true;ui.LayoutChat();const bool restored=ui.m_nFstLine==7;
            reset();post("【系统】欢迎进入珐玛大陆。",0xffffff,0x0000ff);
            for(int i=1;i<=18;++i) {char row[100];sprintf(row,"【行会】成员 %02d 已准备就绪，请队长确认。",i);post(row,0x00ff00);}
            post("【队伍】请大家带好药水和回城卷轴。这条较长的中文消息用于检查普通模式与展开模式各自的换行位置。",0x00ffff);
            post("【私聊】集合完毕后回复。",0xffff00);
            ui.m_chatExpanded=groupPanel==PreviewChatHistoryExpanded;ui.LayoutChat();mouse={790,430};
            if(FILE* report=fopen("chat-history-check.txt","wb")) {fprintf(report,"wrapped=%d colors=%d widths=%d separators=%d accepted=%d limit=%d retained=%d following=%d preserved=%d expanded=%d compact=%d restored=%d\n",wrapped,colors,widths,separators,accepted,limit,retained,following,preserved,expanded,compact,restored);fclose(report);}
            capturePath=ui.m_chatExpanded?"chat-history-expanded.bmp":"chat-history-compact.bmp";
        }

        if(groupPanel==PreviewSceneIdentity) {
            auto& process=g_xGameProc;auto& hero=process.m_xMyHero;auto& ui=process.m_xInterface;auto& status=ui.m_xStatusWnd;
            hero.m_dwIdentity=123;status.m_identityBadge="荣誉勇士";
            const auto encode=[](const std::string& text) {std::vector<char> value((text.size()*8+5)/6+1);fnEncode6BitBuf((BYTE*)text.data(),value.data(),int(text.size()),int(value.size()));return std::string(value.data());};
            const auto receive=[&](DWORD id,const std::string& text,BYTE color=255) {
                _TDEFAULTMESSAGE header={};header.wIdent=SM_USERNAME;header.nRecog=id;header.wParam=color;
                char wire[2048]={};fnEncodeMessage(&header,wire,sizeof(wire));strcpy(wire+DEFBLOCKSIZE,encode(text).c_str());process.OnSocketMessageRecieve(wire);
            };
            receive(123,"珐玛勇士\r珐玛行会\r掌门");
            const bool three=std::string(hero.m_szName)=="珐玛勇士" && std::string(hero.m_szGuildName)=="珐玛行会" && std::string(hero.m_szGuildRank)=="掌门" && status.m_identityTertiary=="掌门" && status.m_identityBadge=="荣誉勇士";
            receive(123,"新名/ignored",249);const bool cleared=std::string(hero.m_szName)=="新名" && !hero.m_szGuildName[0] && !hero.m_szGuildRank[0] && status.m_identityTertiary.empty() && hero.m_dwNameClr==RGB(255,0,0);
            receive(123,"甲/乙\r行会\r成员/ignored");const bool delimiters=std::string(hero.m_szName)=="甲/乙" && std::string(hero.m_szGuildRank)=="成员";
            receive(123,std::string(64,'A'));const bool invalid=std::string(hero.m_szName)=="甲/乙" && !hero.ReceiveName("!",255);
            receive(123,std::string(63,'A'));const bool boundary=strlen(hero.m_szName)==63;
            auto* remote=new CHero;remote->m_dwIdentity=456;process.m_xActorList.AddNode(remote);
            receive(456,"远方勇士\r盟友行会\r成员",251);const bool other=std::string(remote->m_szGuildRank)=="成员" && remote->m_dwNameClr==RGB(255,255,0) && strlen(hero.m_szName)==63;
            const int actors=process.m_xActorList.GetCounter();receive(789,"不存在的角色");const bool unknown=process.m_xActorList.GetCounter()==actors;
            PACKETMSG packet={};packet.stDefMsg.wParam=254;strcpy(packet.szEncodeData,encode("队列角色\r行会\r成员").c_str());remote->OnUserName(&packet);
            const bool queued=std::string(remote->m_szName)=="队列角色" && std::string(remote->m_szGuildRank)=="成员";
            remote->OnChangeNameClr(&packet);const bool recolored=remote->m_dwNameClr==GetUserNameColor(254) && std::string(remote->m_szGuildRank)=="成员";
            process.m_xActorList.MoveCurrentToTop();process.m_xActorList.DeleteCurrentNodeEx();delete remote;
            receive(123,"");const bool empty=!hero.m_szName[0] && !hero.m_szGuildName[0] && !hero.m_szGuildRank[0];
            receive(123,"珐玛勇士\r珐玛行会\r掌门",251);ui.WindowActivate(_WND_ID_STATUS);
            hero.m_shScrnPosX=520;hero.m_shScrnPosY=220;namePreview=true;mouse={790,430};capturePath="scene-identity.bmp";
            if(FILE* report=fopen("scene-identity-check.txt","wb")) {fprintf(report,"three=%d cleared=%d delimiters=%d invalid=%d boundary=%d other=%d unknown=%d queued=%d recolored=%d empty=%d\n",three,cleared,delimiters,invalid,boundary,other,unknown,queued,recolored,empty);fclose(report);}
        }

        if(groupPanel==PreviewGuildIdentity || groupPanel==PreviewNoGuildIdentity) {
            auto& ui=g_xGameProc.m_xInterface;auto& status=ui.m_xStatusWnd;auto& hero=g_xGameProc.m_xMyHero;
            const auto encode=[](const std::string& text) {std::vector<char> value((text.size()*8+5)/6+1);fnEncode6BitBuf((BYTE*)text.data(),value.data(),int(text.size()),int(value.size()));return std::string(value.data());};
            const auto receive=[&](const std::string& text) {
                _TDEFAULTMESSAGE header={};header.wIdent=SM_CHANGEGUILDNAME;header.nRecog=123456;
                char wire[2048]={};fnEncodeMessage(&header,wire,sizeof(wire));strcpy(wire+DEFBLOCKSIZE,encode(text).c_str());g_xGameProc.OnSocketMessageRecieve(wire);
            };
            receive("珐玛行会/掌门/*荣誉勇士");
            const bool fields=std::string(hero.m_szGuildName)=="珐玛行会" && status.m_identityTertiary=="掌门" && status.m_identityBadge=="荣誉勇士";
            receive("珐玛行会/成员");const bool rank=std::string(hero.m_szGuildName)=="珐玛行会" && status.m_identityTertiary=="成员" && status.m_identityBadge.empty();
            receive("/*荣誉勇士");const bool badgeOnly=!hero.m_szGuildName[0] && status.m_identityTertiary.empty() && status.m_identityBadge=="荣誉勇士";
            receive("退出行会");const bool cleared=!hero.m_szGuildName[0] && status.m_identityTertiary.empty() && status.m_identityBadge.empty();
            receive("A/"+std::string(50,'B'));const bool limit=std::string(hero.m_szGuildName)=="A" && status.m_identityTertiary==std::string(39,'B');
            receive("A/BC*D");const bool preceding=status.m_identityTertiary=="B" && status.m_identityBadge=="D";
            const bool invalid=!status.ReceiveGuildIdentity("!") && status.m_identityTertiary=="B" && !status.ReceiveGuildIdentity(encode("/*"+std::string(60,'X')).c_str()) && status.m_identityBadge=="D";
            receive("");const bool empty=!hero.m_szGuildName[0] && status.m_identityTertiary.empty() && status.m_identityBadge.empty();
            strcpy(hero.m_szName,"珐玛勇士");hero.m_stAbility.bLevel=35;hero.m_stAbility.wHP=320;hero.m_stAbility.wMaxHP=400;hero.m_stAbility.wMP=160;hero.m_stAbility.wMaxMP=200;
            if(groupPanel==PreviewGuildIdentity) receive("珐玛行会/掌门/*荣誉勇士");
            ui.WindowActivate(_WND_ID_STATUS);mouse={790,430};
            if(FILE* report=fopen("guild-identity-check.txt","wb")) {fprintf(report,"fields=%d rank=%d badgeOnly=%d cleared=%d limit=%d preceding=%d invalid=%d empty=%d\n",fields,rank,badgeOnly,cleared,limit,preceding,invalid,empty);fclose(report);}
            capturePath=groupPanel==PreviewGuildIdentity?"guild-identity.bmp":"guild-identity-cleared.bmp";
        }

        if(groupPanel==PreviewGuildHistory) {
            auto& ui=g_xGameProc.m_xInterface;auto& guild=ui.m_xGuildWnd;
            guild.m_pages[2].clear();guild.m_first=0;ui.WindowActivate(_WND_ID_GUILD);guild.m_tab=2;
            const auto receive=[&](WORD opcode,const std::string& text) {
                _TDEFAULTMESSAGE header={};header.wIdent=opcode;header.wParam=255;header.wTag=255;
                char encoded[2048]={};fnEncodeMessage(&header,encoded,sizeof(encoded));
                fnEncode6BitBuf((BYTE*)text.data(),encoded+DEFBLOCKSIZE,int(text.size()),sizeof(encoded)-DEFBLOCKSIZE);
                g_xGameProc.OnSocketMessageRecieve(encoded);
            };
            receive(SM_GUILDMESSAGE,"行会通知：今晚八点在沙巴克集合。");
            const bool routed=guild.m_pages[2].size()==1 && guild.m_pages[2][0]=="行会通知：今晚八点在沙巴克集合。";
            const auto& chat=ui.m_chatHistory.back();
            const bool color=chat.color==GetChatColor(4) && chat.background==GetChatColor(0);
            receive(SM_SYSMESSAGE,"普通系统消息");receive(SM_GROUPMESSAGE,"组队消息");
            const bool isolated=guild.m_pages[2].size()==1;
            guild.m_pages[2].clear();guild.m_first=0;
            for(int i=0;i<90;++i) {char row[32];sprintf(row,"历史通知 %02d",i);receive(SM_GUILDMESSAGE,row);}
            const bool retained=guild.m_pages[2].size()==79 && guild.m_pages[2].front()=="历史通知 11" && guild.m_first==61;
            guild.m_first=5;receive(SM_GUILDMESSAGE,"新增通知");const bool scrolled=guild.m_first==4;
            guild.m_first=77;guild.OnScrollUp();guild.OnScrollUp();const bool scrollEnd=guild.m_first==78;guild.OnScrollDown();const bool scrollBack=guild.m_first==77;
            guild.m_pages[2].clear();guild.m_first=0;
            std::string longLine;for(int i=0;i<40;++i) longLine+="中文换行";
            const auto chatSize=ui.m_chatHistory.size();receive(SM_GUILDMESSAGE,longLine);
            std::string combined;for(const auto& row:guild.m_pages[2]) combined+=row;
            const bool wrapped=combined==longLine && guild.m_pages[2].size()>1 && guild.m_pages[2].size()<=5;
            const bool longChat=ui.m_chatHistory.size()==chatSize;
            guild.m_pages[2].clear();guild.AppendHistory(std::string(1023,'W').c_str());const bool capped=guild.m_pages[2].size()==5;
            guild.AppendHistory("");guild.AppendHistory(std::string(1024,'W').c_str());const bool bounds=guild.m_pages[2].size()==5;
            guild.m_pages[2].clear();guild.m_first=0;guild.m_name="珐玛行会";
            receive(SM_GUILDMESSAGE,"【行会通知】欢迎各位勇士回到珐玛大陆。");
            receive(SM_GUILDMESSAGE,"今晚八点集合，请成员提前整理装备、药水和回城卷轴。长通知按原版字体宽度自动换行，中文字符保持完整。");
            for(int i=1;i<=22;++i) {char row[100];sprintf(row,"历史通知 %02d：请各小队确认集合地点。",i);receive(SM_GUILDMESSAGE,row);}
            if(FILE* report=fopen("guild-history-check.txt","wb")) {fprintf(report,"routed=%d color=%d isolated=%d retained=%d scrolled=%d wrapped=%d longChat=%d capped=%d bounds=%d scrollEnd=%d scrollBack=%d\n",routed,color,isolated,retained,scrolled,wrapped,longChat,capped,bounds,scrollEnd,scrollBack);fclose(report);}
            guild.m_first=0;mouse={790,430};capturePath="guild-history.bmp";
        }

        if(groupPanel>=PreviewGuildWire && groupPanel<=PreviewGroupError) {
            auto& ui=g_xGameProc.m_xInterface;auto& guild=ui.m_xGuildWnd;auto& group=ui.m_xGroupWnd;
            static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
            const auto header=[](WORD opcode,int reason=0) {_TDEFAULTMESSAGE value={};fnMakeDefMessage(&value,opcode,reason,0,0,0);return value;};
            const auto encode=[](const std::string& text) {std::vector<char> value((text.size()*8+5)/6+1);fnEncode6BitBuf((BYTE*)text.data(),value.data(),int(text.size()),int(value.size()));return std::string(value.data());};
            const auto receive=[&](WORD opcode,const std::string& text) {return guild.Receive(header(opcode),encode(text).c_str());};
            const std::string overview="玛法行会\rignored\r1\r<Notice>\r欢迎加入行会。\r<KillGuilds>\r敌对行会\r<AllyGuilds>\r同盟行会\r尾";
            const std::string members="#1/*掌门/会长/副会长/#2/*成员/战士/法师/";
            const bool initial=receive(SM_OPENGUILDDLG,overview) && guild.m_bActive && !ui.m_xGuildMasterWnd.m_bActive && guild.m_manager && guild.m_name=="玛法行会" && guild.m_pages[0].size()==8 && guild.m_editable[0].size()==2;
            const bool continuation=receive(GuildWire::SM_GUILD_OVERVIEW,"<Notice>\r续\rignored") && guild.m_pages[0].back()=="尾续" && guild.m_editable[0].back()=="尾续续";
            const bool memberRows=receive(SM_SENDGUILDMEMBERLIST,members) && guild.m_pages[1].size()==7 && guild.m_pages[1][3]=="(1) <掌门>" && guild.m_editable[1][0]=="#1 <掌门>";
            const auto saved=guild.m_pages[0];const bool malformed=!receive(SM_OPENGUILDDLG,"bad") && !guild.Receive(header(SM_SENDGUILDMEMBERLIST),"!") && guild.m_pages[0]==saved && guild.m_pages[1].size()==7;
            receive(SM_OPENGUILDDLG,"普通行会\rignored\r0\r<Notice>\r公告\r");receive(GuildWire::SM_GUILD_MEMBERS_UPDATE,members);
            const bool permission=!guild.m_manager && guild.m_pages[1][3]=="<掌门>";
            guild.m_tab=1;guild.m_first=100;receive(GuildWire::SM_GUILD_MEMBERS_REFRESH,"");const bool emptyMembers=guild.m_pages[1].size()==3 && guild.m_first==2 && guild.m_editable[1].empty();
            const auto capture=[&](auto action,WORD expected) {
                SOCKET receiver=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP),sender=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
                sockaddr_in address={};address.sin_family=AF_INET;address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);bind(receiver,(sockaddr*)&address,sizeof(address));int size=sizeof(address);getsockname(receiver,(sockaddr*)&address,&size);connect(sender,(sockaddr*)&address,sizeof(address));
                DWORD timeout=500;setsockopt(receiver,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));const SOCKET savedSocket=g_xClientSocket.m_sockClient;g_xClientSocket.m_sockClient=sender;action();g_xClientSocket.m_sockClient=savedSocket;
                char wire[256]={};const int length=recv(receiver,wire,sizeof(wire)-1,0);closesocket(sender);closesocket(receiver);if(length!=19 || wire[0]!='#' || wire[length-1]!='!') return false;
                _TDEFAULTMESSAGE packet={};fnDecodeMessage(&packet,wire+2);return packet.wIdent==expected && !packet.nRecog && !packet.wParam && !packet.wTag && !packet.wSeries;
            };
            const bool home=capture([&](){ui.OnKeyDown('F',0);},CM_GUILDHOME);
            const bool memberRequest=capture([&](){const POINT point={guild.m_rcWnd.left+91,guild.m_rcWnd.top+387};guild.OnLButtonDown(point);guild.OnLButtonUp(point);},CM_GUILDMEMBERLIST);
            const bool refresh=capture([&](){guild.ReceiveNotice(header(SM_GUILDADDMEMBER_OK));},CM_GUILDMEMBERLIST) && std::string(ui.m_xMsgBox.Text())==captions.Get(1474);ui.m_xMsgBox.HideMessageBox();
            bool guildNotices=true;int noticeCount=0;
            const int errors[][3]={{SM_OPENGUILDDLG_FAIL,0,1473},{SM_BUILDGUILD_OK,0,1504},{SM_BUILDGUILD_FAIL,-1,1505},{SM_BUILDGUILD_FAIL,-2,1506},{SM_BUILDGUILD_FAIL,-3,1507},{SM_GUILDADDMEMBER_FAIL,1,1475},{SM_GUILDADDMEMBER_FAIL,5,1479},{SM_GUILDDELMEMBER_FAIL,1,1475},{SM_GUILDDELMEMBER_FAIL,4,1482},{SM_GUILDRANKUPDATE_FAIL,-2,1483},{SM_GUILDRANKUPDATE_FAIL,-7,1488},{SM_GUILDMAKEALLY_OK,-1,1489},{SM_GUILDMAKEALLY_FAIL,-4,1492},{SM_GUILDBREAKALLY_OK,-1,1489},{SM_GUILDBREAKALLY_FAIL,-3,1494}};
            for(const auto& item:errors) {guild.ReceiveNotice(header(WORD(item[0]),item[1]));guildNotices=guildNotices && ui.m_xMsgBox.IsActive() && std::string(ui.m_xMsgBox.Text())==captions.Get(item[2]);++noticeCount;ui.m_xMsgBox.HideMessageBox();}
            guild.ReceiveNotice(header(SM_BUILDGUILD_FAIL,-99));const bool unknown=!ui.m_xMsgBox.IsActive();
            const bool groupMembers=group.Receive(header(SM_GROUPMEMBERS),encode("队长/战士/法师/ignored").c_str()) && group.m_xGroupList.GetCounter()==3;
            const bool groupBad=!group.Receive(header(SM_GROUPMEMBERS),encode(std::string(32,'A')+"/").c_str()) && group.m_xGroupList.GetCounter()==3;
            group.Receive(header(SM_GROUPCANCEL),"");const bool cancelled=!group.m_xGroupList.GetCounter();
            bool groupNotices=true;
            const int groupErrors[][3]={{SM_CREATEGROUP_FAIL,-1,1495},{SM_CREATEGROUP_FAIL,-4,1498},{CGroupWnd::AddMemberFailed,-1,1499},{CGroupWnd::AddMemberFailed,-2,1500},{CGroupWnd::AddMemberFailed,-3,1501},{CGroupWnd::AddMemberFailed,-4,1498},{CGroupWnd::AddMemberFailed,-5,1502},{CGroupWnd::DeleteMemberFailed,-1,1499},{CGroupWnd::DeleteMemberFailed,-2,1500},{CGroupWnd::DeleteMemberFailed,-3,1503}};
            for(const auto& item:groupErrors) {group.Receive(header(WORD(item[0]),item[1]),"");groupNotices=groupNotices && std::string(ui.m_xMsgBox.Text())==captions.Get(item[2]);ui.m_xMsgBox.HideMessageBox();}
            receive(SM_OPENGUILDDLG,overview);receive(SM_SENDGUILDMEMBERLIST,members);group.Receive(header(SM_GROUPMEMBERS),encode("队长/战士/法师/").c_str());group.Receive(header(SM_CREATEGROUP_OK),"");
            if(FILE* report=fopen("social-wire-check.txt","wb")) {fprintf(report,"initial=%d continuation=%d members=%d malformed=%d permission=%d empty=%d home=%d memberRequest=%d refresh=%d guildNotices=%d unknown=%d groupMembers=%d groupBad=%d cancelled=%d groupNotices=%d\n",initial,continuation,memberRows,malformed,permission,emptyMembers,home,memberRequest,refresh,guildNotices,unknown,groupMembers,groupBad,cancelled,groupNotices);fclose(report);}
            mouse={790,430};capturePath="guild-wire-overview.bmp";
            if(groupPanel==PreviewGuildMembers) {guild.m_tab=1;capturePath="guild-wire-members.bmp";}
            if(groupPanel==PreviewGuildEditorWire) {guild.m_tab=1;ui.m_xGuildEditor.Open(false);capturePath="guild-wire-editor.bmp";}
            if(groupPanel==PreviewGuildError) {guild.ReceiveNotice(header(SM_GUILDADDMEMBER_FAIL,5));capturePath="guild-wire-error.bmp";}
            if(groupPanel>=PreviewGroupWire) {ui.WindowActivate(_WND_ID_GUILD);ui.WindowActivate(_WND_ID_GROUP);capturePath="group-wire-members.bmp";if(groupPanel==PreviewGroupError) {group.Receive(header(CGroupWnd::AddMemberFailed,-5),"");capturePath="group-wire-error.bmp";}}
        }

        if(groupPanel==PreviewSkillWire || groupPanel==PreviewSkillProgress) {
            auto& ui=g_xGameProc.m_xInterface;auto& skills=ui.m_xSkillWnd;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_MICON].NewLoad("./Data/MIcon.wil");
            const auto record=[](WORD id,char key,BYTE level,const char* name) {
                BYTE bytes[74]={};bytes[0]=120;bytes[4]=key;bytes[5]=level;bytes[6]=LOBYTE(id);bytes[7]=HIBYTE(id);
                memcpy(bytes+8,name,min(size_t(12),strlen(name)));
                const DWORD thresholds[]={500,1500,3000};memcpy(bytes+31,thresholds,sizeof(thresholds));
                char encoded[100]={};fnEncode6BitBuf(bytes,encoded,sizeof(bytes),sizeof(encoded));return std::string(encoded);
            };
            const auto receive=[&](WORD opcode,int id,WORD param,WORD tag,WORD series,const std::string& body=std::string()) {
                _TDEFAULTMESSAGE header={};fnMakeDefMessage(&header,opcode,id,param,tag,series);return skills.Receive(header,body.c_str());
            };
            const auto fire=record(1,'1',1,"火球术"),resist=record(5,'C',2,"抗拒火环"),ice=record(41,'2',1,"冰月神掌");
            const std::string list=fire+"/"+resist+"/"+ice+"/ignored";
            const bool full=receive(SM_SENDMYMAGIC,0,0,0,0x103,list) && skills.m_bMyMagicCnt==3 && skills.m_skillTab==0 && skills.Shortcut(0)==1 && skills.Shortcut(11)==5 && skills.Shortcut(1)==41;
            const bool fields=skills.m_pstMyMagic[0].nCurrTrain==120 && skills.m_pstMyMagic[0].stStdMagic.nTrain[1]==1500 && std::string(skills.m_pstMyMagic[0].stStdMagic.szMagicName)=="火球术";
            const bool progress=receive(SM_MAGIC_LVEXP,1,0x102,0x5678,0xFEDC) && skills.m_pstMyMagic[0].bLevel==2 && skills.m_pstMyMagic[0].nCurrTrain==0xFEDC5678u;
            const auto added=record(39,'A',1,"霹雳掌");const bool add=receive(SM_ADDMAGIC,0,0,0,0,added) && skills.m_bMyMagicCnt==4 && !skills.Shortcut(9) && skills.GetMagicByKey('A');
            std::string invalid=list;invalid[0]='!';
            const bool rejected=!receive(SM_SENDMYMAGIC,0,0,0,3,fire+"/"+resist+"/") && !receive(SM_SENDMYMAGIC,0,0,0,3,invalid) && skills.m_bMyMagicCnt==4 && skills.Shortcut(11)==5;
            skills.BindMagic(5,'1');const bool binding=skills.m_pstMyMagic[0].bUseKey==0 && skills.GetMagicByKey('1')->stStdMagic.wMagicID==5 && skills.Shortcut(0)==5 && !skills.Shortcut(11);
            skills.BindMagic(5,'C');const bool lastKey=skills.Shortcut(11)==5 && !skills.Shortcut(0);
            skills.m_keyMenu={1,1,40,40};skills.m_keyMenuTarget=5;
            const bool removed=receive(SkillWire::DeleteMessage,5,0,0,0) && skills.m_bMyMagicCnt==3 && skills.Shortcut(11)==5 && IsRectEmpty(&skills.m_keyMenu);
            const bool unknown=!receive(SkillWire::DeleteMessage,999,0,0,0) && !receive(SM_MAGIC_LVEXP,999,1,0,0);
            skills.m_skillTab=2;const bool empty=receive(SM_SENDMYMAGIC,0,0,0,0,"ignored") && !skills.m_bMyMagicCnt && skills.m_skillTab==2 && skills.Shortcut(11)==5;
            receive(SM_SENDMYMAGIC,0,0,0,1,record(0x101,'3',1,"高位技能")+"/");const bool lowByte=skills.Shortcut(2)==1;
            const int frame=skills.m_nFrameImgIdx;auto* image=skills.m_pxWndImage;const RECT bounds=skills.m_rcWnd;
            skills.Destroy();const bool cleared=!skills.m_bMyMagicCnt && !skills.m_pstMyMagic && !skills.Shortcut(11) && !skills.Shortcut(2);
            skills.CreateGameWnd(_WND_ID_SKILL,image,frame,TRUE,bounds.left,bounds.top,bounds.right-bounds.left,bounds.bottom-bounds.top);
            receive(SM_SENDMYMAGIC,0,0,0,3,list);skills.m_skillSelected[0]=0;
            if(groupPanel==PreviewSkillProgress) receive(SM_MAGIC_LVEXP,1,2,0x5678,0xFEDC);
            ui.WindowActivate(_WND_ID_SKILL);ui.m_bShowKey=true;mouse={790,430};
            capturePath=groupPanel==PreviewSkillProgress?"skill-wire-progress.bmp":"skill-wire-list.bmp";
            if(FILE* report=fopen("skill-wire-check.txt","wb")) {fprintf(report,"full=%d fields=%d progress=%d added=%d rejected=%d binding=%d lastKey=%d removed=%d unknown=%d empty=%d lowByte=%d cleared=%d\n",full,fields,progress,add,rejected,binding,lastKey,removed,unknown,empty,lowByte,cleared);fclose(report);}
        }

        if(groupPanel==PreviewProgression || groupPanel==PreviewLevelTail) {
            auto& process=g_xGameProc;auto& ui=process.m_xInterface;auto& effects=process.m_levelEffects;
            const auto receive=[&](WORD opcode,int recognition,WORD param,WORD tag,WORD series) {
                _TDEFAULTMESSAGE header={};fnMakeDefMessage(&header,opcode,recognition,param,tag,series);return process.ReceiveProgression(header);
            };
            hero.m_stAbility.dwMaxExp=100000;hero.m_stAbility.wMaxWeight=200;hero.m_stAbility.bMaxWearWeight=40;hero.m_stAbility.bMaxHandWeight=15;
            process.m_weaponTraining=65530;receive(SM_WINEXP,12000,250,10,0);
            const bool experience=hero.m_stAbility.dwExp==12000 && hero.m_stAbility.dwMaxExp==100000 && process.m_weaponTraining==4;
            receive(SM_WINEXP,15000,3000,75,0x0101);const bool training=process.m_weaponTraining==75;
            receive(SM_WINEXP,16000,1000,0,1);const bool zero=process.m_weaponTraining==75;
            receive(SM_WEIGHTCHANGED,0x1007B,0x102,0x103,0);
            const bool weight=hero.m_stAbility.wWeight==123 && hero.m_stAbility.bWearWeight==2 && hero.m_stAbility.bHandWeight==3 && hero.m_stAbility.wMaxWeight==200 && hero.m_stAbility.bMaxWearWeight==40;
            receive(SM_SUBABILITY,0x1234,MAKEWORD(15,20),MAKEWORD(3,4),MAKEWORD(5,6));const auto& sub=hero.m_stSubAbility;
            const bool bonuses=sub.wAntiMagic==0x34 && sub.bHitPoint==15 && sub.bSpeedPoint==20 && sub.bAntiPoison==3 && sub.bPoisonRecover==4 && sub.bHealthRecover==5 && sub.bSpellRecover==6;
            effects.Clear();const bool resource=effects.Load() && effects.resource.dwFstFrm==2020 && effects.resource.dwEndFrm==2030 && effects.resource.wDelay==80 && effects.resource.wImgIdx==Progression::LevelLibrary;
            receive(SM_LEVELUP,17000,43,0,0);
            const bool local=hero.m_stAbility.bLevel==43 && hero.m_stAbility.dwExp==17000 && hero.m_bCurrMtn==_MT_ATTACKMODE && hero.m_bCurrDir==Progression::LevelDirection && effects.active.size()==1;
            auto* remote=new CHero;remote->m_dwIdentity=12345;remote->m_stFeature.bGender=_GENDER_MAN;remote->m_wPosX=5;remote->m_wPosY=7;process.m_xActorList.AddNode(remote);
            receive(SM_LEVELUP,12345,99,0,1);
            const bool other=hero.m_stAbility.bLevel==43 && hero.m_stAbility.dwExp==17000 && remote->m_bCurrMtn==_MT_ATTACKMODE && effects.active.size()==2;
            receive(SM_LEVELUP,12346,99,0,1);receive(SM_LEVELUP,0,0,0,2);const bool ignored=effects.active.size()==2 && hero.m_stAbility.bLevel==43;
            process.m_xActorList.MoveCurrentToTop();process.m_xActorList.DeleteCurrentNodeEx();delete remote;
            bool boundary=false,tail=false,finished=false,stationary=false;
            if(effects.active.size()==2) {
                const POINT cell=effects.active[0].cell;hero.m_wPosX+=2;
                effects.Advance(80);boundary=effects.active[0].frame==2020;effects.Advance(1);boundary=boundary && effects.active[0].frame==2021;
                stationary=effects.active[0].cell.x==cell.x;
                for(int i=0;i<9;++i) effects.Advance(81);
                tail=!effects.active.empty() && effects.active[0].phase==1 && effects.active[0].frame==2029;
                for(int i=0;i<20;++i) effects.Advance(51);finished=effects.active.empty();
            }
            hero.m_stFeatureEx.bHorse=1;hero.m_bWarMode=FALSE;hero.m_bMotionLock=FALSE;
            receive(SM_LEVELUP,17000,43,0,0);hero.m_dwCurrFrame=hero.m_dwEndFrame-1;
            hero.UpdateMotionState(hero.m_wDelay+1,FALSE);
            const bool mounted=hero.m_stFeatureEx.bHorse==1 && hero.m_bCurrMtn==_MT_HORSESTAND;
            hero.m_stFeatureEx.bHorse=0;effects.Clear();
            if(FILE* report=fopen("progression-mounted-check.txt","wb")) {fprintf(report,"restored=%d\n",mounted);fclose(report);}
            const bool unknown=!receive(SM_ABILITY,0,0,0,0);
            effects.Start({hero.m_wPosX,hero.m_wPosY});effects.Start({5,7});
            if(groupPanel==PreviewLevelTail) for(int i=0;i<10;++i) effects.Advance(81);
            else for(int i=0;i<4;++i) effects.Advance(81);
            if(FILE* report=fopen("progression-check.txt","wb")) {fprintf(report,"experience=%d training=%d zero=%d weight=%d bonuses=%d resource=%d local=%d remote=%d ignored=%d boundary=%d stationary=%d tail=%d finished=%d unknown=%d\n",experience,training,zero,weight,bonuses,resource,local,other,ignored,boundary,stationary,tail,finished,unknown);fclose(report);}
            ui.WindowActivate(_WND_ID_STATUS);levelPreview=true;mouse={790,430};capturePath=groupPanel==PreviewLevelTail?"level-tail.bmp":"progression.bmp";
        }

        if(groupPanel==PreviewAbility) {
            auto& ui=g_xGameProc.m_xInterface;auto& status=ui.m_xStatusWnd;
            // Independent wire fixture: level, five ranges, HP/MP, experience, loads, seven elements.
            BYTE bytes[63]={42,5,10,4,8,12,24,7,18,3,6,0x90,1,0xC8,0,0xF4,1,0x2C,1,
                0x78,0x56,0x34,0x12,0x21,0x43,0x65,0x87,123,0,0xC8,0,12,40,5,15,
                1,5,2,0,3,0,4,12,5,0,6,0,7,255,0,2,0,0,255,0,0,0,0,3,0,0,0,0};
            char encoded[128]={};fnEncode6BitBuf(bytes,encoded,sizeof(bytes),sizeof(encoded));
            _TDEFAULTMESSAGE header={};fnMakeDefMessage(&header,SM_ABILITY,LONG(0xFEDCBA98u),1,0x1234,0x5678);
            const bool received=g_xGameProc.ReceiveAbility(header,encoded);
            const auto expected=hero.m_stAbility;
            const bool core=expected.bLevel==42 && expected.wAC==MAKEWORD(5,10) && expected.wMAC==MAKEWORD(4,8) && expected.wDC==MAKEWORD(12,24) && expected.wMC==MAKEWORD(7,18) && expected.wSC==MAKEWORD(3,6);
            const bool health=expected.wHP==400 && expected.wMP==200 && expected.wMaxHP==500 && expected.wMaxMP==300 && hero.m_wHP==400 && hero.m_wMP==200 && hero.m_wMAXHP==500 && hero.m_bHPPercent==80;
            const bool wide=hero.m_nGlod==0xFEDCBA98u && expected.dwExp==0x12345678u && expected.dwMaxExp==0x87654321u && hero.m_bJob==1;
            const bool loads=expected.wWeight==123 && expected.wMaxWeight==200 && expected.bWearWeight==12 && expected.bMaxWearWeight==40 && expected.bHandWeight==5 && expected.bMaxHandWeight==15;
            const bool elements=status.ElementValue(0,0)==5 && status.ElementValue(0,6)==255 && status.ElementValue(1,1)==2 && status.ElementValue(1,4)==-1 && status.ElementValue(2,2)==3;
            const bool retained=status.m_abilityExtension[0]==1 && status.m_abilityExtension[12]==7 && status.m_abilityHeaderOptions[0]==0x34 && status.m_abilityHeaderOptions[1]==0x20 && status.m_abilityHeaderOptions[2]==0x80 && status.m_abilityHeaderOptions[3]==0x60;
            std::string shortBody(encoded);shortBody.pop_back();const bool truncated=!g_xGameProc.ReceiveAbility(header,shortBody.c_str());
            header.wParam=3;const bool profession=!g_xGameProc.ReceiveAbility(header,encoded);header.wParam=1;
            header.wIdent=SM_SUBABILITY;const bool opcode=!g_xGameProc.ReceiveAbility(header,encoded);header.wIdent=SM_ABILITY;
            const bool unchanged=memcmp(&hero.m_stAbility,&expected,sizeof(expected))==0 && hero.m_nGlod==0xFEDCBA98u && status.ElementValue(0,6)==255;
            bytes[0]=43;bytes[36]=9;fnEncode6BitBuf(bytes,encoded,sizeof(bytes),sizeof(encoded));
            const bool replaced=g_xGameProc.ReceiveAbility(header,encoded) && hero.m_stAbility.bLevel==43 && status.ElementValue(0,0)==9;
            const int frame=status.m_nFrameImgIdx;auto* image=status.m_pxWndImage;const RECT bounds=status.m_rcWnd;
            status.Init();const bool cleared=status.ElementValue(0,0)==0 && status.m_abilityExtension[0]==0 && status.m_abilityHeaderOptions[0]==0;
            status.CreateStatusWnd(_WND_ID_STATUS,image,frame,bounds.left,bounds.top,bounds.right-bounds.left,bounds.bottom-bounds.top,TRUE);
            g_xGameProc.ReceiveAbility(header,encoded);ui.WindowActivate(_WND_ID_STATUS);
            if(FILE* report=fopen("ability-wire-check.txt","wb")) {fprintf(report,"received=%d core=%d health=%d wide=%d loads=%d elements=%d retained=%d truncated=%d profession=%d opcode=%d unchanged=%d replaced=%d cleared=%d\n",received,core,health,wide,loads,elements,retained,truncated,profession,opcode,unchanged,replaced,cleared);fclose(report);}
            mouse={790,430};capturePath="ability-wire.bmp";
        }

        if(groupPanel>=PreviewQuestWire && groupPanel<=PreviewQuestIncrement) {
            using namespace QuestWire;
            auto& ui=g_xGameProc.m_xInterface;auto& quest=ui.m_xQuestWnd;
            const auto receive=[&](WORD opcode,WORD count,WORD parent,const std::string& text) {
                char encoded[MaxEncodedBytes+1]={};fnEncode6BitBuf((BYTE*)text.data(),encoded,int(text.size()),sizeof(encoded));
                _TDEFAULTMESSAGE header={};fnMakeDefMessage(&header,opcode,0,count,0,parent);return quest.Receive(header,encoded);
            };
            ui.WindowActivate(_WND_ID_QUEST);
            const bool query=!quest.requests.empty() && quest.requests.back().wIdent==QueryList;
            const std::string list="20/后续历练/0/0//10/初入玛法/2/1/前往比奇城寻找老兵。/";
            const bool full=receive(ListMessage,2,0,list) && quest.entries.size()==2 && quest.entries[0].id==10 && quest.entries[0].completed && quest.entries[1].summary.empty();
            const auto beforeQuery=quest.requests.size();quest.Query();const bool cached=quest.requests.size()==beforeQuery;
            quest.selected=0;quest.Expand();const bool childrenQuery=quest.requests.back().wIdent==QueryChildren && quest.requests.back().wSeries==10;
            const bool children=receive(ChildrenMessage,2,10,"101/拜访老兵/102/击败稻草人/") && quest.entries[0].children.size()==2 && quest.entries[0].children[0].completed;
            const POINT childPoint={quest.m_rcWnd.left+80,quest.m_rcWnd.top+106};quest.OnLButtonDown(childPoint);quest.OnLButtonUp(childPoint);
            const bool detailQuery=quest.requests.back().wIdent==QueryText && quest.requests.back().nRecog==101 && quest.requests.back().wSeries==10;
            receive(TextMessage,102,10,"击败城外的稻草人。");const bool late=quest.Description().empty() && quest.entries[0].children[1].summary=="击败城外的稻草人。";
            const std::string detail="请前往比奇城寻找老兵，了解冒险所需的装备和技能。完成交谈后，前往城外击败稻草人，再返回领取奖励。任务进度会显示在这里，请留意各个阶段的说明。";
            const bool text=receive(TextMessage,101,10,detail) && quest.Description()==detail;
            receive(TextMessage,101,99,"未知任务");const bool unknown=quest.Description()==detail;
            const bool increment=receive(ChildAddedMessage,10,2,"10/103/收集药草/在城外收集药草。/101/重复记录/不应覆盖/") && quest.entries[0].children.size()==3 && quest.entries[0].children.back().completed && quest.Description()==detail;
            const bool added=receive(AddedMessage,0,2,"5/新的委托/10/重复任务/") && quest.entries.size()==3 && quest.entries[0].id==5 && quest.descriptionParent==-1;
            const bool malformed=!receive(ListMessage,1,0,"99/截断任务") && quest.entries.size()==3;
            const bool refreshed=receive(ListMessage,1,0,"10/初入玛法/2/0/尚未完成/") && quest.entries.size()==1 && !quest.entries[0].completed && quest.entries[0].children.empty();
            const bool empty=receive(ListMessage,0,0,"") && quest.entries.empty() && !quest.listInitialized;
            const auto beforeIncrement=quest.requests.size();receive(AddedMessage,0,1,"5/新的委托");
            const bool resync=quest.entries.empty() && quest.requests.size()==beforeIncrement+1 && quest.requests.back().wIdent==QueryList;
            receive(ListMessage,2,0,list);quest.selected=0;quest.Expand();receive(ChildrenMessage,2,10,"101/拜访老兵/102/击败稻草人/");
            quest.OnLButtonDown(childPoint);quest.OnLButtonUp(childPoint);receive(TextMessage,101,10,detail);
            if(groupPanel==PreviewQuestWire) {quest.expanded=-1;quest.selected=quest.descriptionParent=0;quest.descriptionChild=-1;capturePath="quest-wire-list.bmp";}
            else {capturePath="quest-wire-detail.bmp";if(groupPanel==PreviewQuestIncrement) {receive(ChildAddedMessage,10,1,"10/103/收集药草/在城外收集药草。/");capturePath="quest-wire-increment.bmp";}}
            mouse={400,400};
            if(FILE* report=fopen("quest-wire-check.txt","wb")) {fprintf(report,"query=%d full=%d cached=%d childrenQuery=%d children=%d detailQuery=%d late=%d text=%d unknown=%d increment=%d added=%d malformed=%d refreshed=%d empty=%d resync=%d\n",query,full,cached,childrenQuery,children,detailQuery,late,text,unknown,increment,added,malformed,refreshed,empty,resync);fclose(report);}
        }

        if(groupPanel>=PreviewSceneMap && groupPanel<=PreviewSceneHidden) {
            auto& ui=g_xGameProc.m_xInterface;auto& map=ui.m_minimap;auto& siege=ui.m_xSiegeWnd;
            auto& hero=g_xGameProc.m_xMyHero;hero.m_wPosX=hero.m_wPosY=140;
            map.Init();map.large=true;map.pending=true;map.Receive(1,GetTickCount());map.player={140,140};
            map.ClearActors();map.AddActor(_GENDER_NPC,false,POINT{145,145});map.AddActor(_GENDER_MON,false,POINT{130,145});
            map.AddActor(_GENDER_MAN,false,POINT{135,135});map.AddActor(_GENDER_WOMAN,false,POINT{150,135});
            map.AddActor(_GENDER_MON,true,POINT{125,125});map.AddActor(_GENDER_MAN,true,POINT{155,125});map.AddActor(255,false,POINT{160,125});
            const bool npcs=map.npcs.size()==1,monsters=map.monsters.size()==1,players=map.players.size()==2;
            siege.ClearObjects();siege.ReceiveObject(7,POINT{140,140});siege.ReceiveControl(MAKELONG(50,100),1,"engine\t7");
            siege.ReceiveObject(9,POINT{150,130},0);siege.ReceiveObject(10,POINT{130,160},1);siege.CopyMapMarkers(map);
            const bool positions=map.green.size()==2 && map.orange.size()==1 && map.orange[0].y==160;
            siege.ReceiveObject(10,POINT{135,160},0);siege.CopyMapMarkers(map);
            const bool changed=map.green.size()==3 && map.orange.empty();
            siege.ReceiveObject(10,POINT{130,160},1);siege.CopyMapMarkers(map);
            CMinimap empty;empty.AddActor(_GENDER_MON,false,POINT{1,1});empty.ClearActors();
            const bool cleared=empty.npcs.empty() && empty.monsters.empty() && empty.players.empty();
            ui.m_xOptionWnd.m_values[7]=groupPanel!=PreviewSceneHidden;
            capturePath=groupPanel==PreviewSceneHidden?"scene-map-hidden.bmp":"scene-map.bmp";mouse={400,400};
            if(groupPanel==PreviewScenePortrait) {map.visible=false;map.portraitMode=map.portraitVisible=true;map.portraitBlink=100;capturePath="scene-portrait.bmp";}
            if(groupPanel==PreviewSceneSiege) {map.visible=false;ui.WindowActivate(SiegeLayout::WindowId);capturePath="scene-siege.bmp";}
            if(FILE* report=fopen("scene-marker-check.txt","wb")) {fprintf(report,"npcs=%d monsters=%d players=%d positions=%d changed=%d cleared=%d\n",npcs,monsters,players,positions,changed,cleared);fclose(report);}
        }

        if((groupPanel>=PreviewSiege && groupPanel<=PreviewSiegeChecks) || groupPanel==PreviewSiegeExpired) {
            using namespace SiegeLayout;
            auto& ui=g_xGameProc.m_xInterface;auto& panel=ui.m_xSiegeWnd;auto& map=ui.m_minimap;
            panel.controlled=panel.objectPresent=true;panel.objectCell={140,140};panel.style=1;panel.firstValue=50;panel.secondValue=100;
            map.Init();map.Receive(1,GetTickCount());map.player={145,145};
            panel.ReceiveObject(7,POINT{140,140});panel.ReceiveControl(MAKELONG(50,100),1,"engine\t7");
            panel.ReceiveObject(9,POINT{150,150},0);panel.ReceiveObject(10,POINT{120,130},1);
            ui.WindowActivate(WindowId);capturePath="siege.bmp";mouse={790,460};
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            const auto holdAmmo=[&]() {
                constexpr int IconOffset=36,InstanceOffset=62,CountOffset=71;
                BYTE raw[100]={};strcpy((char*)raw,"炮弹");raw[IconOffset]=BYTE(FirstAmmoIcon);raw[IconOffset+1]=BYTE(FirstAmmoIcon>>8);raw[InstanceOffset]=7;raw[CountOffset]=10;
                ui.m_stCommonItem.xItem.SetMir3Item(raw,0);ui.m_stCommonItem.bSetted=TRUE;
            };
            if(groupPanel==PreviewSiegeExpired) {panel.ReceiveObject(7,POINT{140,140},0,GetTickCount()-ObjectLifetime);capturePath="siege-expired.bmp";}
            if(groupPanel==PreviewSiegeLoad) {holdAmmo();panel.BeginLoad();SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"5");capturePath="siege-load.bmp";}
            if(groupPanel==PreviewSiegeChecks) {
                const bool hit=ui.GetWindowInMousePos(POINT{110,110})==WindowId;
                const POINT origin=panel.MapOrigin(),crop=map.SiegeSource(panel.objectCell),center=Marker(panel.objectCell,origin,crop);
                const POINT valid={center.x+65,center.y};panel.Pointer(0,valid);panel.Pointer(2,valid);
                const bool selected=panel.target.x!=0 && panel.target.y!=0;
                const POINT saved=panel.target;panel.Pointer(0,center);panel.Pointer(2,center);
                const bool excluded=panel.target.x==saved.x && panel.target.y==saved.y;
                panel.Activate(Action::Attack);const bool attack=panel.requests.size()==1 && panel.requests[0].target.x==saved.x;
                panel.Activate(Action::Wait);panel.Activate(Action::Move);const bool move=panel.requests.size()==3 && !panel.target.x && !panel.target.y;
                holdAmmo();const bool modal=panel.BeginLoad() && ui.m_xMsgBox.IsActive();ui.FinishNotice(CLICK_NO,false);
                const bool cancel=ui.m_stCommonItem.bSetted && panel.requests.size()==3;
                const bool limit=!panel.SubmitLoad("11") && !panel.SubmitLoad("0");
                panel.BeginLoad();SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"");ui.FinishNotice(CLICK_YES,false);
                const bool loaded=panel.requests.size()==4 && panel.requests.back().quantity==10 && !ui.m_stCommonItem.bSetted && ui.m_xInventoryWnd.m_stInventoryItem[0].bSetted;
                ui.OnKeyDown('P',0);const bool shortcut=!panel.m_bActive;ui.OnKeyDown('P',0);
                if(FILE* report=fopen("siege-gui-check.txt","wb")) {fprintf(report,"hit=%d selected=%d excluded=%d attack=%d move=%d modal=%d cancel=%d limit=%d loaded=%d shortcut=%d\n",hit,selected,excluded,attack,move,modal,cancel,limit,loaded,shortcut);fclose(report);}
                panel.ReleaseControl();panel.ClearObjects();
                panel.ReceiveObject(7,POINT{130,140});panel.ReceiveControl(MAKELONG(12,34),1,"engine\t7");
                const bool before=panel.controlled && panel.objectPresent && panel.objectCell.x==130 && panel.firstValue==12 && panel.secondValue==34;
                panel.ReceiveStatus(MAKELONG(56,78));const bool status=panel.firstValue==56 && panel.secondValue==78;
                panel.ReceiveControl(MAKELONG(1,2),2,"engine\t8");const bool absent=!panel.objectPresent;
                panel.ReceiveObject(8,POINT{150,160});const bool after=panel.objectPresent && panel.objectCell.y==160;
                const RECT entry=panel.EntryBounds();const POINT click={entry.left+2,entry.top+2};POINT anchor={};
                const bool hint=ui.ButtonHintAt(click,anchor)==EntryCaption;
                ui.OnLButtonDown(click);ui.OnLButtonUp(click);const bool opened=panel.m_bActive;
                panel.ReleaseControl();const bool released=!panel.controlled && !panel.m_bActive && IsRectEmpty(&panel.m_rcWnd)==FALSE;
                const RECT hiddenEntry=panel.EntryBounds();const bool hidden=IsRectEmpty(&hiddenEntry)!=FALSE;
                panel.ClearObjects();panel.ReceiveControl(0,1,"engine\t8");const bool cleared=!panel.objectPresent;
                panel.ReceiveObject(8,POINT{140,140});ui.WindowActivate(WindowId);mouse=click;
                if(FILE* report=fopen("siege-state-check.txt","wb")) {fprintf(report,"before=%d status=%d absent=%d after=%d hint=%d opened=%d released=%d hidden=%d cleared=%d\n",before,status,absent,after,hint,opened,released,hidden,cleared);fclose(report);}
                const auto capturePacket=[&](auto perform,_TDEFAULTMESSAGE& packet,std::string& body) {
                    SOCKET receiver=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP),sender=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
                    sockaddr_in address={};address.sin_family=AF_INET;address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
                    bind(receiver,(sockaddr*)&address,sizeof(address));int addressLength=sizeof(address);getsockname(receiver,(sockaddr*)&address,&addressLength);
                    connect(sender,(sockaddr*)&address,sizeof(address));DWORD timeout=500;setsockopt(receiver,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));
                    const SOCKET saved=g_xClientSocket.m_sockClient;g_xClientSocket.m_sockClient=sender;perform();g_xClientSocket.m_sockClient=saved;
                    char wire[8192]={};const int received=recv(receiver,wire,sizeof(wire)-1,0);closesocket(sender);closesocket(receiver);
                    if(received<19 || wire[0]!='#' || wire[received-1]!='!') return false;
                    wire[received-1]=0;fnDecodeMessage(&packet,wire+2);
                    char decoded[8192]={};const int length=fnDecode6BitBuf(wire+2+DEFBLOCKSIZE,decoded,sizeof(decoded));body.assign(decoded,length);return true;
                };
                _TDEFAULTMESSAGE packet={};std::string body;
                ui.WindowActivate(WindowId);
                const bool mapRequest=capturePacket([&](){ui.WindowActivate(WindowId);},packet,body) && packet.wIdent==CMinimap::MapRequestMessage && !packet.nRecog && !packet.wParam && body.empty() && !map.visible;
                panel.target={0x1234,0x5678};
                const bool attackWire=capturePacket([&](){panel.Activate(Action::Attack);},packet,body) && packet.wIdent==AttackMessage && DWORD(packet.nRecog)==0x56781234u && !packet.wParam && !packet.wTag && !packet.wSeries && body.empty();
                const bool waitWire=capturePacket([&](){panel.Activate(Action::Wait);},packet,body) && packet.wIdent==SiegeLayout::WaitMessage && !packet.nRecog && body.empty();
                const bool moveWire=capturePacket([&](){panel.Activate(Action::Move);},packet,body) && packet.wIdent==MoveMessage && !packet.nRecog && !panel.target.x && !panel.target.y && body.empty();
                ui.m_xInventoryWnd.m_stInventoryItem[0].bSetted=FALSE;holdAmmo();
                const bool loadWire=capturePacket([&](){panel.SubmitLoad("5");},packet,body) && packet.wIdent==LoadMessage && packet.nRecog==7 && packet.wParam==5 && !packet.wTag && !packet.wSeries && body=="炮弹" && !ui.m_stCommonItem.bSetted && ui.m_xInventoryWnd.m_stInventoryItem[0].bSetted;
                CSiegeWnd::Request emptyAttack={};emptyAttack.action=Action::Attack;const bool noTarget=!CSiegeWnd::BuildRequest(emptyAttack,packet,body);
                emptyAttack.action=Action::Close;const bool noClosePacket=!CSiegeWnd::BuildRequest(emptyAttack,packet,body);
                panel.ReceiveObject(9,POINT{150,150},0);panel.ReceiveObject(10,POINT{120,130},1);
                if(FILE* report=fopen("siege-send-check.txt","wb")) {fprintf(report,"map=%d attack=%d wait=%d move=%d load=%d noTarget=%d noClosePacket=%d\n",mapRequest,attackWire,waitWire,moveWire,loadWire,noTarget,noClosePacket);fclose(report);}
                panel.ClearObjects();panel.ReceiveObject(7,POINT{140,140},0,100);
                panel.ReceiveControl(0,1,"engine\t7");panel.ExpireObjects(100+ObjectLifetime-1);
                const bool beforeExpiry=panel.objectPresent;
                panel.ExpireObjects(100+ObjectLifetime);const bool exactExpiry=!panel.objectPresent;
                const bool keepControl=panel.controlled && panel.m_bActive;
                panel.ReceiveObject(7,POINT{141,142},0,6000);const bool recovered=panel.objectPresent && panel.objectCell.x==141;
                panel.ReceiveObject(7,POINT{142,143},1,9000);panel.ExpireObjects(6000+ObjectLifetime);
                const bool refreshed=panel.objectPresent && panel.objectCell.y==143;
                panel.ExpireObjects(9000+ObjectLifetime);const bool refreshedExpiry=!panel.objectPresent;
                panel.ReceiveObject(7,POINT{140,140},0,100);panel.ReceiveObject(8,POINT{150,150},1,4000);panel.ExpireObjects(5100);
                panel.ReceiveControl(0,1,"engine\t8");const bool independent=panel.objectPresent;
                panel.ReceiveControl(0,1,"engine\t7");const bool removed=!panel.objectPresent;
                panel.ReceiveObject(7,POINT{140,140},0,0xffffff00u);panel.ExpireObjects(ObjectLifetime-257);
                const bool wrapBefore=panel.objectPresent;panel.ExpireObjects(ObjectLifetime-256);const bool wrapExpiry=!panel.objectPresent;
                panel.ReceiveObject(7,POINT{140,140},0,100);ui.WindowActivate(WindowId);panel.ExpireObjects(5100);
                const bool closedExpiry=!panel.objectPresent && !panel.m_bActive && panel.controlled;
                panel.ReceiveObject(7,POINT{140,140});panel.ReceiveObject(9,POINT{150,150},0);panel.ReceiveObject(10,POINT{120,130},1);
                panel.ReceiveStatus(MAKELONG(50,100));ui.WindowActivate(WindowId);
                if(FILE* report=fopen("siege-expiry-check.txt","wb")) {fprintf(report,"before=%d boundary=%d control=%d recovered=%d refreshed=%d refreshedExpiry=%d independent=%d removed=%d wrapBefore=%d wrapExpiry=%d closed=%d\n",beforeExpiry,exactExpiry,keepControl,recovered,refreshed,refreshedExpiry,independent,removed,wrapBefore,wrapExpiry,closedExpiry);fclose(report);}
                capturePath="siege-checks.bmp";
            }
        }

        if(groupPanel==PreviewConsignmentWire) {
            using namespace ConsignmentWire;
            auto& ui=g_xGameProc.m_xInterface;auto& browser=ui.m_xConsignmentWnd;
            BYTE record[RecordBytes]={};
            strcpy((char*)record+Server,"玛法一区");strcpy((char*)record+Seller,"远行勇士");
            strcpy((char*)record+Date,"11-17");strcpy((char*)record+Description,"旅途中收集的装备。\r欢迎有需要的勇士选购。");
            constexpr unsigned FixtureId=7,FixturePrice=12000;
            memcpy(record+Id,&FixtureId,sizeof(FixtureId));memcpy(record+Price,&FixturePrice,sizeof(FixturePrice));
            constexpr int ItemIcon=372,IconField=36,InstanceField=62;
            strcpy((char*)record+Item,"厚重的装备");record[Item+IconField]=BYTE(ItemIcon);record[Item+IconField+1]=BYTE(ItemIcon>>8);record[Item+InstanceField]=FixtureId;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            char encoded[RecordBytes*2]={};fnEncode6BitBuf(record,encoded,RecordBytes,sizeof(encoded));const std::string list=std::string(encoded)+"/";
            browser.waiting=true;browser.ReceiveList(list.c_str(),1,2,3);
            const bool decoded=browser.listings.size()==1 && browser.listings[0].id==FixtureId && browser.listings[0].price==FixturePrice && browser.listings[0].seller=="远行勇士" && browser.listings[0].server=="玛法一区";
            const bool details=decoded && browser.listings[0].date=="11-17" && browser.listings[0].description=="旅途中收集的装备。\r欢迎有需要的勇士选购。";
            const bool item=decoded && browser.listings[0].item.m_stItemInfo.stStdItem.wLooks==ItemIcon && strcmp(browser.listings[0].item.Name(),"厚重的装备")==0;
            const bool state=browser.selected==0 && browser.hovered==-1 && !browser.waiting && browser.page==2 && browser.pages==3;
            browser.ReceiveList("",0,0,0);const bool empty=browser.listings.empty() && browser.selected==-1 && browser.page==2;
            browser.ReceiveList(encoded,1,1,1);const bool delimiter=browser.listings.empty();
            browser.ReceiveList("abcd/",1,1,1);const bool shortRecord=browser.listings.empty();
            const std::string multiple=list+list;browser.ReceiveList(multiple.c_str(),1,1,2);const bool count=browser.listings.size()==1;
            browser.ReceiveList(multiple.c_str(),2,1,2);const bool multipleRows=browser.listings.size()==2;
            browser.sessionId=42;browser.Close();const bool cleared=browser.sessionId==0 && browser.listings.empty();
            browser.ReceiveList(list.c_str(),1,1,1);ui.WindowActivate(ConsignmentLayout::BrowserId);
            if(FILE* report=fopen("consignment-wire-check.txt","wb")) {fprintf(report,"decoded=%d details=%d item=%d state=%d empty=%d delimiter=%d shortRecord=%d count=%d multiple=%d cleared=%d\n",decoded,details,item,state,empty,delimiter,shortRecord,count,multipleRows,cleared);fclose(report);}
            CConsignmentWnd::Request request={};_TDEFAULTMESSAGE packet={};std::string payload;
            constexpr unsigned Session=123,ListingId=0x12345678;
            request.sessionId=Session;request.id=ListingId;request.item=browser.listings[0].item;
            request.action=ConsignmentLayout::Action::NextPage;
            const bool nextPage=CConsignmentWnd::BuildRequest(request,packet,payload) && packet.wIdent==NextPage && packet.nRecog==Session && !packet.wParam && !packet.wTag && payload.empty();
            request.action=ConsignmentLayout::Action::PreviousPage;const bool previous=CConsignmentWnd::BuildRequest(request,packet,payload) && packet.wIdent==PreviousPage;
            request.action=ConsignmentLayout::Action::Refresh;const bool refresh=CConsignmentWnd::BuildRequest(request,packet,payload) && packet.wIdent==Refresh;
            request.query="装备";request.action=ConsignmentLayout::Action::SearchName;
            const bool name=CConsignmentWnd::BuildRequest(request,packet,payload) && packet.wIdent==Search && payload=="*\r装备\r0";
            request.action=ConsignmentLayout::Action::SearchSeller;const bool seller=CConsignmentWnd::BuildRequest(request,packet,payload) && payload=="装备\r*\r0";
            request.query.clear();const bool wildcard=CConsignmentWnd::BuildRequest(request,packet,payload) && payload=="*\r*\r0";
            request.action=ConsignmentLayout::Action::Buy;
            const bool purchase=CConsignmentWnd::BuildRequest(request,packet,payload) && packet.wIdent==Buy && packet.wParam==LOWORD(ListingId) && packet.wTag==HIWORD(ListingId) && payload==request.item.Name();
            request.action=ConsignmentLayout::Action::CancelListing;
            const bool cancel=CConsignmentWnd::BuildRequest(request,packet,payload) && packet.wIdent==Cancel && packet.wParam==LOWORD(ListingId) && packet.wTag==HIWORD(ListingId) && payload.empty();
            request.action=ConsignmentLayout::Action::SubmitRegistration;request.price=12000;request.description="说明\r\n\r\n结束";request.item.m_stackMode=2;request.item.SetCount(9);
            const bool registration=CConsignmentWnd::BuildRequest(request,packet,payload) && packet.wIdent==Register && packet.nRecog==Session && packet.wParam==9 && packet.wTag==0 && payload=="厚重的装备\r7\r12000\r说明\r \r结束";
            request.item.m_stackMode=0;const bool unstacked=CConsignmentWnd::BuildRequest(request,packet,payload) && packet.wParam==0;
            request.description=std::string(RegistrationDescriptionLimit-1,'a')+"中文";
            CConsignmentWnd::BuildRequest(request,packet,payload);const bool boundary=payload.substr(payload.find_last_of('\r')+1)==std::string(RegistrationDescriptionLimit-1,'a');
            request.action=ConsignmentLayout::Action::Close;const bool unsupported=!CConsignmentWnd::BuildRequest(request,packet,payload);
            if(FILE* report=fopen("consignment-send-check.txt","wb")) {fprintf(report,"next=%d previous=%d refresh=%d name=%d seller=%d wildcard=%d purchase=%d cancel=%d registration=%d unstacked=%d boundary=%d unsupported=%d\n",nextPage,previous,refresh,name,seller,wildcard,purchase,cancel,registration,unstacked,boundary,unsupported);fclose(report);}
            capturePath="consignment-wire.bmp";mouse={790,460};
        }

        if((groupPanel>=PreviewConsignmentEmpty && groupPanel<=PreviewConsignmentChecks) || groupPanel==PreviewConsignmentInput) {
            using namespace ConsignmentLayout;
            auto& ui=g_xGameProc.m_xInterface;auto& browser=ui.m_xConsignmentWnd;auto& registration=ui.m_xConsignmentRegistration;
            ui.WindowActivate(BrowserId);browser.page=1;browser.pages=3;
            constexpr int ItemIcon=372, IconField=36, InstanceField=62, RecordBytes=100;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            BYTE raw[RecordBytes]={};strcpy((char*)raw,"珍藏的装备");raw[IconField]=BYTE(ItemIcon);raw[IconField+1]=BYTE(ItemIcon>>8);raw[InstanceField]=7;
            if(groupPanel!=PreviewConsignmentEmpty) for(int i=0;i<VisibleRows;++i) {
                CConsignmentWnd::Listing listing;listing.id=i+1;listing.price=1000*(i+1);listing.seller=i%2?"行会商人":"远行勇士";
                listing.server="玛法一区";listing.date="11-17";listing.description="旅途中收集的装备。\r欢迎有需要的勇士选购。";
                listing.item.SetMir3Item(raw,i);browser.listings.push_back(listing);
            }
            browser.selected=browser.listings.empty()?-1:0;g_xGameProc.m_xMyHero.m_nGlod=50000;
            capturePath=groupPanel==PreviewConsignmentEmpty?"consignment-empty.bmp":"consignment-list.bmp";mouse={790,460};
            const auto chooseItem=[&]() {
                ui.m_stCommonItem.xItem.SetMir3Item(raw,0);ui.m_stCommonItem.bSetted=TRUE;
                POINT point={registration.m_rcWnd.left+RegistrationIconX,registration.m_rcWnd.top+RegistrationIconY};
                ui.OnLButtonDown(point);ui.OnLButtonUp(point);
            };
            if(groupPanel>=PreviewConsignmentRegistration && groupPanel<=PreviewConsignmentConfirm && groupPanel!=PreviewConsignmentBuy && groupPanel!=PreviewConsignmentCancel) {
                ui.WindowActivate(BrowserId);ui.WindowActivate(RegistrationId);
                capturePath="consignment-registration.bmp";
                SetWindowTextA(registration.edits[0],"这件装备陪伴我走过许多旅程。\r\n希望能找到珍惜它的新主人。");
                if(groupPanel!=PreviewConsignmentRegistration) {chooseItem();registration.SubmitPrice("12000");capturePath="consignment-item.bmp";}
                if(groupPanel==PreviewConsignmentPrice) {ui.OpenNameDialog(PriceDialog);SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"12000");capturePath="consignment-price.bmp";}
                if(groupPanel==PreviewConsignmentConfirm) {registration.Activate(Action::SubmitRegistration);capturePath="consignment-confirm.bmp";}
            }
            if(groupPanel==PreviewConsignmentBuy) {browser.Activate(Action::Buy);capturePath="consignment-buy.bmp";}
            if(groupPanel==PreviewConsignmentCancel) {browser.Activate(Action::CancelListing);capturePath="consignment-cancel.bmp";}
            if(groupPanel==PreviewConsignmentInput) {
                constexpr DWORD StartTick=10001;
                browser.page=browser.pages=1;
                browser.Activate(Action::NextPage,StartTick);
                const bool nextAtEnd=browser.requests.size()==1 && browser.requests.back().action==Action::NextPage;
                browser.KeyDown(VK_RIGHT,StartTick+RequestInterval);
                const bool exactBoundary=browser.requests.size()==1;
                browser.KeyDown(VK_LEFT,StartTick+RequestInterval+1);
                const bool previousAtStart=browser.requests.size()==2 && browser.requests.back().action==Action::PreviousPage;
                browser.Activate(Action::SearchName,StartTick+RequestInterval+1);
                const bool independent=browser.requests.size()==3;
                browser.Activate(Action::SearchSeller,StartTick+RequestInterval+1);
                const bool searchBlocked=browser.requests.size()==3;
                browser.Activate(Action::SearchSeller,StartTick+2*(RequestInterval+1));
                const bool searchReady=browser.requests.size()==4;
                browser.selected=0;browser.Activate(Action::Buy,StartTick+2*(RequestInterval+1));
                ui.OnKeyDown(VK_DOWN,0);const bool modal=ui.m_xMsgBox.IsActive() && browser.selected==0;
                ui.m_xMsgBox.HideMessageBox();browser.Confirm(false,StartTick+2*(RequestInterval+1));
                const bool cancelled=browser.requests.size()==4;
                browser.Activate(Action::Buy,StartTick+2*(RequestInterval+1));
                const bool retry=ui.m_xMsgBox.IsActive();ui.m_xMsgBox.HideMessageBox();browser.Confirm(true,StartTick+2*(RequestInterval+1));
                browser.Activate(Action::Buy,StartTick+3*RequestInterval+2);
                const bool transactionBlocked=browser.requests.size()==5 && !ui.m_xMsgBox.IsActive();
                for(int i=0;i<VisibleRows+1;++i) ui.OnKeyDown(VK_DOWN,0);
                const bool bottom=browser.selected==VisibleRows-1;
                for(int i=0;i<VisibleRows+1;++i) ui.OnKeyDown(VK_UP,0);
                const bool top=browser.selected==0;
                browser.Activate(Action::Refresh,0xffffff00u);const auto beforeWrap=browser.requests.size();
                browser.KeyDown(VK_RIGHT,RequestInterval-256);
                const bool wrapBoundary=browser.requests.size()==beforeWrap;
                browser.KeyDown(VK_RIGHT,RequestInterval-255);
                const bool wrapReady=browser.requests.size()==beforeWrap+1;
                const auto saved=browser.listings;browser.listings.clear();
                const POINT emptyRow={browser.m_rcWnd.left+FirstRow.left+1,browser.m_rcWnd.top+FirstRow.top+(VisibleRows-1)*RowHeight+1};
                const bool emptyHit=browser.RowAt(emptyRow)==VisibleRows-1;
                browser.selected=VisibleRows-1;const auto beforeEmpty=browser.requests.size();browser.Activate(Action::Buy,StartTick);
                const bool emptyBuy=browser.requests.size()==beforeEmpty && !ui.m_xMsgBox.IsActive();
                ui.WindowActivate(BrowserId);
                const bool closed=!browser.KeyDown(VK_RIGHT,StartTick);
                ui.WindowActivate(BrowserId);browser.listings=saved;browser.selected=0;
                const auto beforeReset=browser.requests.size();browser.Activate(Action::Refresh,RequestInterval+1);
                const bool reset=browser.requests.size()==beforeReset+1;
                if(FILE* report=fopen("consignment-input-check.txt","wb")) {fprintf(report,"nextAtEnd=%d exactBoundary=%d previousAtStart=%d independent=%d searchBlocked=%d searchReady=%d modal=%d cancelled=%d retry=%d transactionBlocked=%d bottom=%d top=%d wrapBoundary=%d wrapReady=%d closed=%d reset=%d emptyHit=%d emptyBuy=%d\n",nextAtEnd,exactBoundary,previousAtStart,independent,searchBlocked,searchReady,modal,cancelled,retry,transactionBlocked,bottom,top,wrapBoundary,wrapReady,closed,reset,emptyHit,emptyBuy);fclose(report);}
                browser.selected=VisibleRows-1;capturePath="consignment-keyboard.bmp";
            }
            if(groupPanel==PreviewConsignmentChecks) {
                const bool hit=ui.GetWindowInMousePos(POINT{100,100})==BrowserId;
                SetWindowTextA(browser.edits[0],"装备查询");browser.Activate(Action::SearchName);
                const bool query=browser.requests.size()==1 && browser.requests.back().query=="装备查询";
                browser.Activate(Action::NextPage);const bool next=browser.requests.back().page==2 && browser.page==1;
                browser.Activate(Action::Buy,GetTickCount()+RequestInterval+1);const bool purchaseModal=ui.m_xMsgBox.IsActive();ui.FinishNotice(CLICK_NO,false);
                const bool purchaseCancelled=browser.requests.size()==2;
                browser.sessionId=123;browser.Activate(Action::Buy,GetTickCount()+RequestInterval+1);browser.sessionId=456;browser.listings[0].price=2000;ui.FinishNotice(CLICK_YES,false);
                const bool purchaseSnapshot=browser.requests.back().price==1000 && g_xGameProc.m_xMyHero.m_nGlod==50000;
                browser.Activate(Action::Register);
                const bool bag=ui.m_xInventoryWnd.m_bActive && registration.m_bActive && ui.IsTopWindow(RegistrationId);
                chooseItem();const bool itemKept=registration.hasItem && !ui.m_stCommonItem.bSetted && ui.m_xInventoryWnd.m_stInventoryItem[0].bSetted;
                const bool badPrice=!registration.SubmitPrice("12x") && !registration.SubmitPrice("100000001") && !registration.SubmitPrice("4294967296");
                const bool priceLimit=registration.SubmitPrice("100000000") && registration.price==100000000 && registration.SubmitPrice("0") && registration.price==100000000;
                if(FILE* report=fopen("amount-price-check.txt","wb")) {fprintf(report,"limitAndZero=%d\n",priceLimit);fclose(report);}
                ui.OpenNameDialog(PriceDialog);SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"12000");ui.FinishNotice(CLICK_YES,false);
                const bool price=registration.price==12000;
                SetWindowTextA(registration.edits[0],"寄售说明");registration.Activate(Action::SubmitRegistration);
                const bool registerModal=ui.m_xMsgBox.IsActive();ui.FinishNotice(CLICK_YES,false);
                const bool submit=registration.requests.size()==1 && registration.requests[0].description=="寄售说明" && registration.requests[0].price==12000 && registration.requests[0].sessionId==456 && !registration.m_bActive && registration.sessionId==0;
                registration.Activate(Action::CancelRegistration);const bool cancel=!registration.hasItem && !registration.price && ui.m_xInventoryWnd.m_stInventoryItem[0].bSetted;
                ui.WindowActivate(RegistrationId);
                registration.SyncEditors(false);const bool hidden=!IsWindowVisible(registration.edits[0]);registration.SyncEditors(true);
                SendMessage(registration.edits[0],WM_KEYDOWN,VK_ESCAPE,0);const bool close=!registration.m_bActive && !IsWindowVisible(registration.edits[0]);
                browser.Close();const bool browserReset=browser.listings.empty() && browser.selected==-1 && !browser.page && !browser.pages && !browser.waiting;
                if(FILE* report=fopen("consignment-gui-check.txt","wb")) {fprintf(report,"hit=%d query=%d next=%d purchaseModal=%d purchaseCancelled=%d purchaseSnapshot=%d bag=%d itemKept=%d badPrice=%d price=%d registerModal=%d submit=%d cancel=%d hidden=%d close=%d browserReset=%d\n",hit,query,next,purchaseModal,purchaseCancelled,purchaseSnapshot,bag,itemKept,badPrice,price,registerModal,submit,cancel,hidden,close,browserReset);fclose(report);}
                ui.WindowActivate(_WND_ID_INVENTORY);capturePath="consignment-checks.bmp";
            }
        }

        if(groupPanel>=PreviewMailInbox && groupPanel<=PreviewMailGiftHint) {
            auto& ui=g_xGameProc.m_xInterface;auto& mail=ui.m_xMailWnd;
            ui.WindowActivate(_WND_ID_MAIL);
            for(unsigned i=0;i<10;++i) {
                CMailWnd::Message message;message.id=i+1;message.person=i%2?"沙巴克城主":"道馆掌柜";
                message.subject=i%2?"行会活动通知":"欢迎来到玛法大陆";
                message.body="勇士，欢迎来到玛法大陆。\r请在出发前整理背包，检查装备的持久力。\r";
                for(int row=0;row<14;++row) message.body+="城镇中的商人可以为你修理装备，旅途中也别忘了与同伴互相照应。\r";
                message.read=i%2!=0;message.attachment=i%3==0;message.loaded=true;
                mail.inbox.push_back(message);message.person="远行的勇士";mail.outbox.push_back(message);
            }
            mail.selectedInbox=1;mail.selectedOutbox=0;mail.checkedInbox=0;mail.checkedOutbox=2;
            mail.composeRecipient="远行的勇士";mail.composeDraft="你好！\r\n明天一起前往沙巴克，记得带上补给。\r\n祝一路平安。";
            mail.giftRecipient="同行的伙伴";mail.giftDraft="这份礼物送给你。\r\n愿我们在玛法大陆再次相逢。";
            const char* paths[]={"mail-inbox.bmp","mail-read.bmp","mail-compose.bmp","mail-gift.bmp","mail-outbox.bmp"};
            const int page=groupPanel==PreviewMailChecks?0:(groupPanel==PreviewMailBody || groupPanel==PreviewMailGoldHint)?int(MailLayout::View::Read):groupPanel>=PreviewMailGold?int(MailLayout::View::Gift):groupPanel-PreviewMailInbox;
            mail.SelectView(MailLayout::View(page));capturePath=paths[page];
            mouse={700,400};
            enum MailFixtureField { Icon=36, Instance=62 };
            constexpr int GiftFixtureIcon=372;
            const auto prepareGift=[&]() {
                g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
                BYTE raw[MailLayout::ItemRecordBytes]={};strcpy((char*)raw,"送礼装备");
                raw[Icon]=BYTE(GiftFixtureIcon);raw[Icon+1]=BYTE(GiftFixtureIcon>>8);raw[Instance]=77;
                ui.m_stCommonItem.xItem.SetMir3Item(raw,0);ui.m_stCommonItem.bSetted=TRUE;
                return mail.SelectGift(POINT{MailLayout::GiftCenterX,MailLayout::GiftCenterY});
            };
            if(groupPanel==PreviewMailGiftItem || groupPanel==PreviewMailGiftHint) {
                prepareGift();capturePath=groupPanel==PreviewMailGiftHint?"mail-gift-hint.bmp":"mail-gift-item.bmp";
                if(groupPanel==PreviewMailGiftHint) mouse={MailLayout::GiftCenterX,MailLayout::GiftCenterY};
            }
            if(groupPanel==PreviewMailBody) {
                mail.inbox[1].body=std::string(59,'A')+"B\r\r中文边界："+std::string(110,'C')+"\r末行";
                capturePath="mail-body-boundary.bmp";
            }
            if(groupPanel==PreviewMailGoldHint) {
                mail.inbox[1].attachment=true;mail.inbox[1].gold=500;
                mouse={MailLayout::Attachment.left+2,MailLayout::Attachment.top+2};capturePath="mail-gold-hint.bmp";
            }
            if(groupPanel==PreviewMailGold) {
                ui.OpenNameDialog(MailLayout::GoldDialogAction);SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"500");capturePath="mail-gift-gold.bmp";
            }
            if(groupPanel==PreviewMailChecks) {
                const auto click=[&](MailLayout::Action action) {
                    for(const auto& button:mail.VisibleButtons()) if(button.action==action) {
                        POINT point={mail.m_rcWnd.left+button.x+2,mail.m_rcWnd.top+button.y+2};ui.OnLButtonDown(point);ui.OnLButtonUp(point);break;
                    }
                };
                const bool hit=ui.GetWindowInMousePos(POINT{100,100})==_WND_ID_MAIL;
                const POINT track={MailLayout::ScrollX+2,MailLayout::ScrollY+MailLayout::ScrollTravel-2};
                ui.OnLButtonDown(track);ui.OnLButtonUp(track);
                // Original 420200 stores 7/10 as float before multiplying by ten: truncation yields six.
                const bool pageScroll=mail.firstInbox==6 && !mail.scrolling && !mail.captured;
                const POINT thumb={MailLayout::ScrollX+2,mail.ScrollTop()+2};
                ui.OnLButtonDown(thumb);const bool thumbStable=mail.firstInbox==6 && mail.scrolling;
                ui.OnMouseMove(POINT{MailLayout::ScrollX+2,MailLayout::CanvasHeight});
                ui.OnLButtonUp(POINT{MailLayout::ScrollX+2,MailLayout::CanvasHeight});
                const bool dragEnd=mail.firstInbox==9 && !mail.scrolling && !mail.captured;
                mail.Wheel(-100);
                if(FILE* report=fopen("mail-scroll-check.txt","wb")) {fprintf(report,"page=%d thumb=%d end=%d\n",pageScroll,thumbStable,dragEnd);fclose(report);}

                click(MailLayout::Action::Compose);
                const bool editors=IsWindowVisible(mail.edits[0]) && IsWindowVisible(mail.edits[1]);
                SetWindowTextA(mail.edits[0],"独立收件人");SetWindowTextA(mail.edits[1],"写信草稿");
                click(MailLayout::Action::Gift);const bool giftDraft=mail.draft=="这份礼物送给你。\r\n愿我们在玛法大陆再次相逢。";
                SetWindowTextA(mail.edits[1],"送礼草稿");click(MailLayout::Action::Compose);
                const bool separate=mail.recipient=="独立收件人" && mail.draft=="写信草稿";
                click(MailLayout::Action::SendCompose);
                const bool request=mail.requests.size()==1 && mail.requests.back().recipient=="独立收件人" && mail.requests.back().body=="写信草稿";
                click(MailLayout::Action::CancelCompose);
                const bool cancel=mail.view==MailLayout::View::Compose && mail.recipient.empty() && mail.draft.empty();
                const bool snapshot=mail.requests.back().body=="写信草稿";
                click(MailLayout::Action::Outbox);mail.selectedOutbox=0;mail.outbox[0].body="";mail.OpenSelected();
                const bool empty=mail.view==MailLayout::View::Read && mail.readSent && mail.CurrentMessage()==&mail.outbox[0];
                click(MailLayout::Action::Next);const bool next=mail.selectedOutbox==1 && mail.CurrentMessage()==&mail.outbox[1];
                mail.Wheel(1);const bool bodyScroll=mail.firstBody==1;
                click(MailLayout::Action::Inbox);mail.Wheel(100);const bool bottom=mail.firstInbox==9;mail.Wheel(-100);
                const POINT check={MailLayout::CheckBox.left+2,MailLayout::CheckBox.top+2};ui.OnLButtonDown(check);ui.OnLButtonUp(check);
                const bool checked=mail.checkedInbox==-1;
                click(MailLayout::Action::Gift);
                const bool giftSelected=prepareGift() && mail.hasGiftItem && !ui.m_stCommonItem.bSetted && ui.m_xInventoryWnd.m_stInventoryItem[0].bSetted;
                mail.RequestAction(MailLayout::Action::SendGift);
                const bool giftRequest=mail.requests.back().action==MailLayout::Action::SendGift && mail.requests.back().item[Instance]==77;
                click(MailLayout::Action::CancelGift);
                const bool giftCancel=mail.view==MailLayout::View::Gift && !mail.hasGiftItem && ui.m_xInventoryWnd.m_stInventoryItem[0].bSetted;
                mail.hasGiftItem=mail.giftIsGold=true;
                const bool gold=!mail.SubmitGold("100000001") && !mail.SubmitGold("12x") && mail.SubmitGold("500") && mail.giftGold==500;
                const bool goldLimit=mail.SubmitGold("100000000") && mail.giftGold==100000000 && mail.giftGoldConfirmed;
                const bool goldZero=mail.SubmitGold("0") && mail.giftGold==100000000 && mail.giftGoldConfirmed;
                ui.OpenNameDialog(MailLayout::GoldDialogAction);ui.FinishNotice(CLICK_NO,false);
                const bool goldCancel=!mail.giftGoldConfirmed && mail.giftGold==100000000;
                mail.giftGold=0;const bool zeroConfirmed=mail.SubmitGold("0") && mail.giftGoldConfirmed && mail.giftGold==0;
                if(FILE* report=fopen("amount-mail-check.txt","wb")) {fprintf(report,"limit=%d zero=%d cancel=%d zeroConfirmed=%d\n",goldLimit,goldZero,goldCancel,zeroConfirmed);fclose(report);}
                if(FILE* report=fopen("mail-gift-check.txt","wb")) {fprintf(report,"selected=%d request=%d cancel=%d gold=%d\n",giftSelected,giftRequest,giftCancel,gold);fclose(report);}
                click(MailLayout::Action::Compose);mail.SyncEditors(false);
                const bool hidden=!IsWindowVisible(mail.edits[0]) && !IsWindowVisible(mail.edits[1]);mail.SyncEditors(true);
                SendMessage(mail.edits[0],WM_KEYDOWN,VK_TAB,0);const bool tab=GetFocus()==mail.edits[1];
                SendMessage(mail.edits[1],WM_KEYDOWN,VK_ESCAPE,0);
                const bool closed=!mail.m_bActive && !IsWindowVisible(mail.edits[0]) && ui.GetWindowInMousePos(POINT{100,100})!=_WND_ID_MAIL;
                if(FILE* report=fopen("mail-gui-check.txt","wb")) {fprintf(report,"hit=%d editors=%d giftDraft=%d separate=%d request=%d cancel=%d snapshot=%d empty=%d next=%d bodyScroll=%d bottom=%d checked=%d hidden=%d tab=%d closed=%d\n",hit,editors,giftDraft,separate,request,cancel,snapshot,empty,next,bodyScroll,bottom,checked,hidden,tab,closed);fclose(report);}
                ui.WindowActivate(_WND_ID_MAIL);mail.SelectView(MailLayout::View::Inbox);capturePath="mail-checks.bmp";
            }
        }

        if(groupPanel>=PreviewTradeLocal && groupPanel<=PreviewTradeHeld) {
            using namespace Mir3AccessoryHint;
            using Trade=CExchangeWnd;
            enum RecordField { NameBytes=30, Weight=32, Icon=36, Instance=62, Current=66, Maximum=68, LargeIcon=75 };
            auto& ui=g_xGameProc.m_xInterface;auto& trade=ui.m_xExchangeWnd;
            auto& icons=g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY];icons.NewLoad("./Data/Inventory.wil");
            BYTE raw[Trade::RecordBytes]={};memcpy(raw,"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234",NameBytes);
            raw[ModeOffset]=AccuracyBracelet;raw[Weight]=3;raw[Icon]=LargeIcon;raw[Instance]=1;raw[FirstScalarOffset]=2;raw[FirstBonusOffset]=1;
            raw[Current]=0xD0;raw[Current+1]=7;raw[Maximum]=0x10;raw[Maximum+1]=0x27;
            const bool local=trade.SetOfferRecord(0,0,raw);const bool name=trade.offers[0][0].name.size()==NameBytes;
            const bool retained=!memcmp(trade.offers[0][0].record.data(),raw,sizeof(raw));
            strcpy((char*)raw,"对方交易装备");raw[Instance]=2;const int remoteSlot=2*Trade::GridColumns;
            const bool remote=trade.SetOfferRecord(1,remoteSlot,raw);trade.first[1]=2;
            const POINT left={trade.m_rcWnd.left+Trade::LocalGridLeft+Trade::CellSize+1,trade.m_rcWnd.top+Trade::GridTop+Trade::CellSize+1};
            const POINT right={trade.m_rcWnd.left+Trade::RemoteGridLeft+1,trade.m_rcWnd.top+Trade::GridTop+1};
            const bool multi=trade.OfferAt(left)==0 && trade.offers[0][0].width>1 && trade.offers[0][0].height>1;
            const bool scroll=trade.OfferAt(right,1)==remoteSlot && trade.OfferAt(right)==-1 && trade.first[0]==0;
            const bool edge=trade.OfferAt({trade.m_rcWnd.left+Trade::LocalGridLeft-1,left.y})==-1;
            raw[0]=0;const bool invalid=!trade.SetOfferRecord(0,0,raw) && trade.offers[0][0].name.size()==NameBytes;
            trade.partner="TradePartner";ui.WindowActivate(_WND_ID_EXCHANGE);
            mouse=groupPanel==PreviewTradeRemote?right:left;g_xGameProc.m_ptMousePos=mouse;
            if(groupPanel==PreviewTradeHeld) trade.returnSlot=0;
            capturePath=groupPanel==PreviewTradeLocal?"trade-local-hint.bmp":groupPanel==PreviewTradeRemote?"trade-remote-hint.bmp":"trade-held-hint.bmp";
            if(FILE* report=fopen("trade-item-hint-check.txt","wb")) {fprintf(report,"local=%d remote=%d name=%d retained=%d multi=%d scroll=%d edge=%d invalid=%d\n",local,remote,name,retained,multi,scroll,edge,invalid);fclose(report);}
        }

        if(groupPanel>=PreviewSpecialHint && groupPanel<=PreviewBlueArmor) {
            using namespace Mir3AccessoryHint;
            enum RecordField { Weight=32, Icon=36, ShopMaximum=38, Instance=62, Current=66, Maximum=68 };
            auto& ui=g_xGameProc.m_xInterface;auto& shop=ui.m_xStoreWnd;
            g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY].NewLoad("./Data/Inventory.wil");
            BYTE raw[sizeof(CItem().m_serverRecord)]={};strcpy((char*)raw,"特殊装备属性测试");
            const auto setWord=[&](int field,int value) {raw[field]=BYTE(value);raw[field+1]=BYTE(value>>8);};
            raw[ModeOffset]=SpecialEquipment;raw[Weight]=3;raw[Instance]=1;
            setWord(Current,2000);setWord(Maximum,5000);setWord(ShopMaximum,10000);
            setWord(ComfortOffset,300);raw[FirstBonusOffset]=2;raw[CarryCapacityOffset]=3;raw[SecondBonusOffset]=1;raw[HandCapacityOffset]=4;raw[AttackBonusOffset]=2;
            raw[StrongElementOffset]=1;raw[ExtraElementOffset]=1;raw[StrongElementOffset+1]=2;raw[ExtraElementAmountOffset]=2;raw[HealthBonusOffset]=2;raw[ManaBonusOffset]=3;
            CItem item;item.SetMir3Item(raw,0);BYTE elements[ElementCount]={};const auto rows=item.AccessoryHint(35,20,elements);
            const bool attributes=rows.size()==11 && rows[3].text.find("302 (+2)")!=std::string::npos && rows[4].text.find("4 (+1)")!=std::string::npos && rows[5].text.find("6 (+2)")!=std::string::npos;
            const bool order=rows[6].text.find("x2")!=std::string::npos && rows[7].text.find("+20")!=std::string::npos && rows[8].text.find("+30")!=std::string::npos && rows[9].text.find("x2")!=std::string::npos;
            const bool maximum=rows[1].text.find("2/5")!=std::string::npos && item.AccessoryHint(35,20,elements,true)[1].text.find("2/10")!=std::string::npos;
            raw[FirstBonusOffset]=255;raw[HealthBonusOffset]=255;item.SetMir3Item(raw,0);const auto negative=item.AccessoryHint(35,20,elements);const bool signedValues=negative.size()==10 && negative[3].text.find(": 300")!=std::string::npos && negative[3].color==NormalColor;
            raw[FirstBonusOffset]=2;raw[HealthBonusOffset]=2;item.SetMir3Item(raw,0);
            char encoded[140]={};fnEncode6BitBuf(raw,encoded,sizeof(raw),sizeof(encoded));std::string packet=std::string(encoded)+"/";shop.selected=0;
            const bool parsed=shop.SetDetailRecords(packet.c_str(),1,0) && !memcmp(shop.offers[0].record.data(),raw,sizeof(raw));
            const bool rejected=!shop.SetDetailRecords("broken/",1,0) && shop.offers.size()==1;
            BYTE longName[sizeof(raw)];memcpy(longName,raw,sizeof(raw));memcpy(longName,"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234",30);fnEncode6BitBuf(longName,encoded,sizeof(longName),sizeof(encoded));
            const bool fullName=shop.SetDetailRecords((std::string(encoded)+"/").c_str(),1,0) && shop.offers[0].name.size()==30;
            if(groupPanel>=PreviewShopArmorHint) {
                raw[ModeOffset]=ItemHintStyle::MaleArmor;
                strcpy((char*)raw,"防具属性测试");setWord(AttackOffset,(3<<8)|1);
                if(groupPanel>=PreviewRedArmor) {
                    auto& icons=g_xGameProc.m_xImage.m_xImageList[_IMAGE_ITEM_INVENTORY];int dyedFrame=-1;
                    for(int frame=0;frame<icons.m_stNewWixImgaeInfo.nIndexCount && dyedFrame<0;++frame) {
                        if(!icons.NewSetIndex(frame)) continue;
                        const WORD* next=(WORD*)icons.m_pbCurrImage;
                        for(int row=0;row<icons.m_lpstNewCurrWilImageInfo->shHeight;++row) {
                            const WORD* run=next+1;const WORD* end=run+*next;next=end;
                            while(run+1<end) {const WORD tag=*run++,count=*run++;if(tag==0xc2 && count) {dyedFrame=frame;break;}if(tag!=0xc0) run+=count;}
                            if(dyedFrame>=0) break;
                        }
                    }
                    if(dyedFrame>=0) setWord(Icon,dyedFrame);
                    raw[ItemHintStyle::IconRedOffset]=groupPanel==PreviewRedArmor?255:0;
                    raw[ItemHintStyle::IconGreenOffset]=0;
                    raw[ItemHintStyle::IconBlueOffset]=groupPanel==PreviewBlueArmor?255:0;
                    if(FILE* report=fopen("item-dye-resource-check.txt","wb")) {fprintf(report,"dyed_frame=%d\n",dyedFrame);fclose(report);}
                }
                fnEncode6BitBuf(raw,encoded,sizeof(raw),sizeof(encoded));packet=std::string(encoded)+"/";
            }
            shop.SetDetailRecords(packet.c_str(),1,0);
            if(groupPanel==PreviewSpecialHint) {ui.m_xInventoryWnd.AddInvenItem(item,0,TRUE);ui.WindowActivate(_WND_ID_INVENTORY);mouse={550,80};capturePath="special-equipment-hint.bmp";}
            else {if(groupPanel==PreviewStorageHint) shop.SetStorageRecords(77,packet.c_str(),1);ui.WindowActivate(_WND_ID_STORE);const RECT cell=shop.OfferCell(0);mouse={cell.left+10,cell.top+10};capturePath=groupPanel==PreviewStorageHint?"storage-item-hint.bmp":groupPanel==PreviewShopArmorHint?"shop-armor-hint.bmp":"shop-item-hint.bmp";}
            if(groupPanel==PreviewRedArmor) capturePath="shop-red-armor-hint.bmp";
            if(groupPanel==PreviewBlueArmor) capturePath="shop-blue-armor-hint.bmp";
            g_xGameProc.m_ptMousePos=mouse;
            if(FILE* report=fopen("special-shop-hint-check.txt","wb")) {fprintf(report,"attributes=%d order=%d maximum=%d signed=%d parsed=%d rejected=%d full_name=%d\n",attributes,order,maximum,signedValues,parsed,rejected,fullName);fclose(report);}
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
            ui.m_stCommonItem.bSetted=TRUE;ui.m_stCommonItem.bIsBeltItem=FALSE;ui.m_stCommonItem.bWaitResponse=TRUE;ui.m_xInventoryWnd.m_usePending=true;
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
        if(moviePreview) MoviePreview::Render();
        else if(connectionSelectionPreview) g_xChrSelProc.RenderScene(elapsed);
        else if(serverGroupsPreview) g_xServerGroups.RenderScene(0);
        else if(loginPreview) g_xLoginProc.RenderScene(0);
        else if(exitWaitingPreview) {
            if(g_bProcState==_GAME_PROC) {
                g_xGameProc.ReturnToSelection();
                if(FILE* report=fopen("exit-return-check.txt","wb")) {
                    fprintf(report,"selection_endpoint=%d\nscene_cleared=%d\nwaiting_page=%d\n",g_nServerPort==exitPort && !strcmp(g_szServerIP,"127.0.0.1"),!g_xGameProc.m_xInterface.m_stCommonItem.bSetted && !g_xGameProc.m_xActorList.ListLength() && !g_xGameProc.m_exitTransition.active,g_bProcState==_CHAR_SEL_PROC && g_xChrSelProc.IsWaiting());fclose(report);
                }
                const SOCKET active=g_xClientSocket.m_sockClient;
                g_xClientSocket.OnSocketMessage(INVALID_SOCKET,WSAMAKESELECTREPLY(FD_CLOSE,WSAECONNRESET));
                if(FILE* report=fopen("exit-return-check.txt","ab")) {fprintf(report,"stale_socket_ignored=%d\n",g_xClientSocket.m_sockClient==active);fclose(report);}
                g_xChrSelProc.OnKeyDown(VK_F2,0);
            }
            if(exitPeer==INVALID_SOCKET) exitPeer=accept(exitListener,nullptr,nullptr);
            if(exitPeer!=INVALID_SOCKET && exitNetworkStage==0) {
                char query[256]={};const int count=recv(exitPeer,query,sizeof(query)-1,0);
                if(count>0) {
                    _TDEFAULTMESSAGE request={};fnDecodeMessage(&request,query+2);
                    if(FILE* report=fopen("exit-return-check.txt","ab")) {fprintf(report,"query_on_new_socket=%d\n",query[0]=='#' && query[1]=='2' && request.wIdent==CM_QUERYCHR);fclose(report);}
                    // Send an invalid list first: the waiting state must survive it.
                    _TDEFAULTMESSAGE reply={};fnMakeDefMessage(&reply,SM_QUERYCHR,0,4,0,0);char encoded[128]={};fnEncodeMessage(&reply,encoded,sizeof(encoded));std::string wire=std::string("#")+encoded+"!";send(exitPeer,wire.c_str(),int(wire.size()),0);exitNetworkStage=1;
                }
            } else if(exitNetworkStage==1 && !capturePath) {
                if(FILE* report=fopen("exit-return-check.txt","ab")) {fprintf(report,"invalid_list_waits=%d\n",g_xChrSelProc.IsWaiting());fclose(report);}
                _TDEFAULTMESSAGE reply={};fnMakeDefMessage(&reply,SM_QUERYCHR,0,0,0,0);char encoded[128]={};fnEncodeMessage(&reply,encoded,sizeof(encoded));std::string wire=std::string("#")+encoded+"!";send(exitPeer,wire.c_str(),int(wire.size()),0);exitNetworkStage=2;
            } else if(exitNetworkStage==2 && !g_xChrSelProc.IsWaiting()) {
                if(FILE* report=fopen("exit-return-check.txt","ab")) {fprintf(report,"real_list_clears_wait=%d\n",!g_xChrSelProc.IsWaiting());fclose(report);}exitNetworkStage=3;
            }
            g_xChrSelProc.RenderScene(elapsed);
        } else {
        if(spellMapPreview) g_xGameProc.RenderScene(0);
        else {
        RECT canvas = {0,0,800,600};
        DrawLegacyPanel(canvas,RGB(20,24,28),255,0,false);
        if(levelPreview) {
            const auto& effects=g_xGameProc.m_levelEffects.active;
            for(size_t i=0;i<effects.size();++i) g_xGameProc.m_levelEffects.DrawAt(effects[i],440+int(i)*180,300);
        }
        if(spellPreview) {g_xGameProc.DrawSpellMarks();g_xGameProc.DrawNativeSpells();g_xGameProc.DrawSpellParticles();}
        if(modalPointerPreview && capturePath) {
            auto& ui=g_xGameProc.m_xInterface;
            const POINT button={280,310},outside={790,430};
            ui.OnLButtonDown(button);mouse=button;
            if(modalPointerPreview>=305) {ui.OnMouseMove(outside);mouse=outside;}
            if(modalPointerPreview==306) {ui.OnMouseMove(button);mouse=button;}
        }
        g_xGameProc.m_xInterface.RenderInterface(elapsed,mouse);
        }
        if(namePreview) g_xGameProc.m_xMyHero.DrawName();
        if(speechPreview) g_xGameProc.m_xMyHero.ShowMessage(0);
        if(noticePreview) g_xGameProc.m_xNotice.RenderMessageBox(0);
        if(capturePath && !strcmp(capturePath,"equipment-hover-hint.bmp")) {
            auto& ui=g_xGameProc.m_xInterface;
            if(FILE* report=fopen("equipment-hover-check.txt","wb")) {fprintf(report,"window=%d status=%d slot=%d held=%d present=%d name=%s mouse=%ld,%ld\n",ui.GetWindowInMousePos(mouse),_WND_ID_STATUS,ui.m_xStatusWnd.GetEquipNum(mouse),ui.m_stCommonItem.bSetted,ui.m_xStatusWnd.m_stEquipItem[5].bSetted,ui.m_xStatusWnd.m_stEquipItem[5].xItem.Name(),mouse.x,mouse.y);fclose(report);}
        }
        if(damagePreview) {
            auto& actor=g_xGameProc.m_xMyHero;
            const int amounts[]={-9,-99,-100,-65535,25,0};
            for(int i=0;i<6;++i) {
                actor.m_damage=CDamageNumbers{};actor.m_shScrnPosX=30+i*125;actor.m_shScrnPosY=55;
                actor.AddDamage(amounts[i]);actor.DrawDamage();
            }
        }
        if(exitFadePreview) g_xGameProc.DrawExitTransition();
        }
        // Button hints expand over several frames; capture their final readable state.
        POINT hintAnchor={};
        auto& previewUi=g_xGameProc.m_xInterface;
        if(capturePath && previewUi.hintExpanding && previewUi.ButtonHintAt(mouse,hintAnchor)) return;
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
                const HWND edits[]={g_xChatEditBox.GetSafehWnd(),g_xGameProc.m_xInterface.m_xGuildEditor.edit,g_xGameProc.m_xInterface.m_xMailWnd.edits[0],g_xGameProc.m_xInterface.m_xMailWnd.edits[1],g_xGameProc.m_xInterface.m_xConsignmentWnd.edits[0],g_xGameProc.m_xInterface.m_xConsignmentWnd.edits[1],g_xGameProc.m_xInterface.m_xConsignmentRegistration.edits[0]};
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
                    const int x=MulDiv(editRect.left-display.left,surface.dwWidth,display.right-display.left);
                    const int y=MulDiv(editRect.top-display.top,surface.dwHeight,display.bottom-display.top);
                    SetStretchBltMode(target,COLORONCOLOR);
                    StretchBlt(target,x,y,MulDiv(width,surface.dwWidth,display.right-display.left),
                        MulDiv(height,surface.dwHeight,display.bottom-display.top),source,0,0,width,height,SRCCOPY);
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
                if(exitQuitPreview) g_xGameProc.m_xInterface.FinishNotice(CLICK_YES,false);
            }
        }

        if(connectionPreview) ConnectionPreview::AfterFrame();
    }
    LRESULT DefMainWndProc(HWND window,UINT message,WPARAM wp,LPARAM lp) override {
        if(connectionSelectionPreview) return g_xChrSelProc.DefMainWndProc(window,message,wp,lp);
        if(serverGroupsPreview) return g_xServerGroups.DefMainWndProc(window,message,wp,lp);
        if(loginPreview) return g_xLoginProc.DefMainWndProc(window,message,wp,lp);
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
        case WM_SYSKEYDOWN:
            if(wp==VK_F10) {ui.OnKeyDown(wp,lp);return 0;}
            break;
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
	const int previewGroup = strcmp(lpCmdLine,"--preview=modal-scroll")==0 ? 301 :
        strcmp(lpCmdLine,"--preview=modal-audio")==0 ? 302 :
        strcmp(lpCmdLine,"--preview=modal-guild-editor")==0 ? 303 :
        strcmp(lpCmdLine,"--preview=modal-button-pressed")==0 ? 304 :
        strcmp(lpCmdLine,"--preview=modal-button-left")==0 ? 305 :
        strcmp(lpCmdLine,"--preview=modal-button-returned")==0 ? 306 :
        strcmp(lpCmdLine,"--preview=options-interaction")==0 ? 300 :
        strcmp(lpCmdLine,"--preview=role-male-warrior")==0 ? RolePresentationPreview::First+0 :
        strcmp(lpCmdLine,"--preview=role-male-mage")==0 ? RolePresentationPreview::First+1 :
        strcmp(lpCmdLine,"--preview=role-male-tao")==0 ? RolePresentationPreview::First+2 :
        strcmp(lpCmdLine,"--preview=role-female-warrior")==0 ? RolePresentationPreview::First+3 :
        strcmp(lpCmdLine,"--preview=role-female-mage")==0 ? RolePresentationPreview::First+4 :
        strcmp(lpCmdLine,"--preview=role-female-tao")==0 ? RolePresentationPreview::First+5 :
        strcmp(lpCmdLine,"--preview=role-selection-hover")==0 ? RolePresentationPreview::First+6 :
        strcmp(lpCmdLine,"--preview=role-selection-pressed")==0 ? RolePresentationPreview::First+7 :
        strcmp(lpCmdLine,"--preview=character-interaction-select")==0 ? CharacterInteractionPreview::First :
        strcmp(lpCmdLine,"--preview=character-interaction-create")==0 ? CharacterInteractionPreview::Create :
        strcmp(lpCmdLine,"--preview=character-interaction-female")==0 ? CharacterInteractionPreview::Female :
        strcmp(lpCmdLine,"--preview=selection-roster-first")==0 ? SelectionRosterPreview::First+0 :
        strcmp(lpCmdLine,"--preview=selection-roster-waiting")==0 ? SelectionRosterPreview::First+1 :
        strcmp(lpCmdLine,"--preview=selection-roster-second")==0 ? SelectionRosterPreview::First+2 :
        strcmp(lpCmdLine,"--preview=selection-roster-long-name")==0 ? SelectionRosterPreview::First+3 :
        strcmp(lpCmdLine,"--preview=selection-roster-empty")==0 ? SelectionRosterPreview::First+4 :
        strcmp(lpCmdLine,"--preview=character-name-exists")==0 ? SelectionDialogPreview::First+0 :
        strcmp(lpCmdLine,"--preview=character-full")==0 ? SelectionDialogPreview::First+1 :
        strcmp(lpCmdLine,"--preview=character-create-failed")==0 ? SelectionDialogPreview::First+2 :
        strcmp(lpCmdLine,"--preview=character-name-invalid")==0 ? SelectionDialogPreview::First+3 :
        strcmp(lpCmdLine,"--preview=character-query-failed")==0 ? SelectionDialogPreview::First+4 :
        strcmp(lpCmdLine,"--preview=character-start-failed")==0 ? SelectionDialogPreview::First+5 :
        strcmp(lpCmdLine,"--preview=character-disconnected")==0 ? SelectionDialogPreview::First+6 :
        strcmp(lpCmdLine,"--preview=character-delete-confirm")==0 ? SelectionDialogPreview::First+7 :
        strcmp(lpCmdLine,"--preview=character-delete-missing")==0 ? SelectionDialogPreview::First+8 :
        strcmp(lpCmdLine,"--preview=character-start-missing")==0 ? SelectionDialogPreview::First+9 :
        strcmp(lpCmdLine,"--preview=character-name-long")==0 ? SelectionDialogPreview::First+10 :
        strcmp(lpCmdLine,"--preview=character-empty")==0 ? SelectionDialogPreview::First+11 :
        strcmp(lpCmdLine,"--preview=movie-forward")==0 ? MoviePreview::First :
        strcmp(lpCmdLine,"--preview=movie-reverse")==0 ? MoviePreview::Reverse :
        strcmp(lpCmdLine,"--preview=movie-opening-return")==0 ? MoviePreview::Opening :
        strcmp(lpCmdLine,"--preview=movie-creation-return")==0 ? MoviePreview::Creation :
        strcmp(lpCmdLine,"--preview=login-transition-start")==0 ? LoginTransitionPreview::First :
        strcmp(lpCmdLine,"--preview=login-transition-half")==0 ? LoginTransitionPreview::Half :
        strcmp(lpCmdLine,"--preview=login-transition-end")==0 ? LoginTransitionPreview::Last :
        strcmp(lpCmdLine,"--preview=connection-error")==0 ? ConnectionPreview::First :
        strcmp(lpCmdLine,"--preview=selection-connection-error")==0 ? ConnectionPreview::Selection :
        strcmp(lpCmdLine,"--preview=connection-checks")==0 ? ConnectionPreview::ChecksOnly :
        strcmp(lpCmdLine,"--preview=server-groups")==0 ? ServerGroupsPreview::First :
        strcmp(lpCmdLine,"--preview=server-group-hover")==0 ? ServerGroupsPreview::First+1 :
        strcmp(lpCmdLine,"--preview=server-group-selected")==0 ? ServerGroupsPreview::First+2 :
        strcmp(lpCmdLine,"--preview=server-group-row")==0 ? ServerGroupsPreview::First+3 :
        strcmp(lpCmdLine,"--preview=server-group-scroll")==0 ? ServerGroupsPreview::First+4 :
        strcmp(lpCmdLine,"--preview=server-group-exit")==0 ? ServerGroupsPreview::First+5 :
        strcmp(lpCmdLine,"--preview=server-group-login")==0 ? ServerGroupsPreview::First+6 :
        strcmp(lpCmdLine,"--preview=login")==0 ? LoginPreview::First :
        strcmp(lpCmdLine,"--preview=login-hover")==0 ? LoginPreview::First+1 :
        strcmp(lpCmdLine,"--preview=login-pressed")==0 ? LoginPreview::First+2 :
        strcmp(lpCmdLine,"--preview=login-error")==0 ? LoginPreview::First+3 :
        strcmp(lpCmdLine,"--preview=login-servers")==0 ? LoginPreview::First+4 :
        strcmp(lpCmdLine,"--preview=login-billing")==0 ? LoginPreview::First+5 :
        strcmp(lpCmdLine,"--preview=group-panel")==0 ? 1 :
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
        strcmp(lpCmdLine,"--preview=lighting-day")==0 ? PreviewLightingDay :
        strcmp(lpCmdLine,"--preview=lighting-night")==0 ? PreviewLightingNight :
        strcmp(lpCmdLine,"--preview=lighting-dawn")==0 ? PreviewLightingDawn :
        strcmp(lpCmdLine,"--preview=weather-snow")==0 ? PreviewWeatherSnow :
        strcmp(lpCmdLine,"--preview=weather-rain")==0 ? PreviewWeatherRain :
        strcmp(lpCmdLine,"--preview=weather-fog")==0 ? PreviewWeatherFog :
        strcmp(lpCmdLine,"--preview=ground-items")==0 ? PreviewGroundItems :
        strcmp(lpCmdLine,"--preview=ground-coin")==0 ? PreviewGroundCoin :
        strcmp(lpCmdLine,"--preview=spell-map")==0 ? PreviewSpellMap :
        strcmp(lpCmdLine,"--preview=spell-ground")==0 ? PreviewSpellGround :
        strcmp(lpCmdLine,"--preview=spell-orbit")==0 ? PreviewSpellOrbit :
        strcmp(lpCmdLine,"--preview=spell-staged")==0 ? PreviewSpellStaged :
        strcmp(lpCmdLine,"--preview=exit-program")==0 ? PreviewExitProgram :
        strcmp(lpCmdLine,"--preview=exit-character")==0 ? PreviewExitCharacter :
        strcmp(lpCmdLine,"--preview=exit-blocked")==0 ? PreviewExitBlocked :
        strcmp(lpCmdLine,"--preview=exit-fade")==0 ? PreviewExitFade :
        strcmp(lpCmdLine,"--preview=exit-waiting")==0 ? PreviewExitWaiting :
        strcmp(lpCmdLine,"--preview=exit-quit")==0 ? PreviewExitQuit :
        strcmp(lpCmdLine,"--preview=keyboard-skill-binding")==0 ? PreviewKeyboard :
        strcmp(lpCmdLine,"--preview=keyboard-skill-shortcuts")==0 ? PreviewKeyboardShortcuts :
        strcmp(lpCmdLine,"--preview=keyboard-skill-covered")==0 ? PreviewKeyboardCovered :
        strcmp(lpCmdLine,"--preview=trade-open-error")==0 ? PreviewTradeOpenError :
        strcmp(lpCmdLine,"--preview=trade-transaction")==0 ? PreviewTradeTransaction :
        strcmp(lpCmdLine,"--preview=trade-quantity")==0 ? PreviewTradeQuantity :
        strcmp(lpCmdLine,"--preview=trade-cancelled")==0 ? PreviewTradeCancelled :
        strcmp(lpCmdLine,"--preview=trade-gold-input")==0 ? PreviewTradeGoldInput :
        strcmp(lpCmdLine,"--preview=equipment-transaction")==0 ? PreviewEquipSuccess :
        strcmp(lpCmdLine,"--preview=equipment-rollback")==0 ? PreviewEquipFailure :
        strcmp(lpCmdLine,"--preview=equipment-takeoff")==0 ? PreviewEquipTakeOff :
        strcmp(lpCmdLine,"--preview=storage-withdraw-reply")==0 ? PreviewStorageReply :
        strcmp(lpCmdLine,"--preview=service-repair-reply")==0 ? PreviewServiceRepair :
        strcmp(lpCmdLine,"--preview=service-sell-reply")==0 ? PreviewServiceSell :
        strcmp(lpCmdLine,"--preview=service-deposit-reply")==0 ? PreviewServiceDeposit :
        strcmp(lpCmdLine,"--preview=service-failure-reply")==0 ? PreviewServiceFailure :
        strcmp(lpCmdLine,"--preview=item-use-chain")==0 ? PreviewItemUse :
        strcmp(lpCmdLine,"--preview=item-use-restored")==0 ? PreviewItemUseRestored :
        strcmp(lpCmdLine,"--preview=interface-audio")==0 ? PreviewInterfaceAudio :
        strcmp(lpCmdLine,"--preview=server-notice")==0 ? PreviewNotice :
        strcmp(lpCmdLine,"--preview=server-notice-moved")==0 ? PreviewNoticeMoved :
        strcmp(lpCmdLine,"--preview=speech-single")==0 ? PreviewSpeechSingle :
        strcmp(lpCmdLine,"--preview=speech-multiline")==0 ? PreviewSpeechMultiple :
        strcmp(lpCmdLine,"--preview=speech-eight")==0 ? PreviewSpeechEight :
        strcmp(lpCmdLine,"--preview=chat-history")==0 ? PreviewChatHistory :
        strcmp(lpCmdLine,"--preview=chat-history-expanded")==0 ? PreviewChatHistoryExpanded :
        strcmp(lpCmdLine,"--preview=scene-identity")==0 ? PreviewSceneIdentity :
        strcmp(lpCmdLine,"--preview=guild-identity")==0 ? PreviewGuildIdentity :
        strcmp(lpCmdLine,"--preview=guild-identity-cleared")==0 ? PreviewNoGuildIdentity :
        strcmp(lpCmdLine,"--preview=guild-history")==0 ? PreviewGuildHistory :
        strcmp(lpCmdLine,"--preview=guild-wire")==0 ? PreviewGuildWire :
        strcmp(lpCmdLine,"--preview=guild-wire-members")==0 ? PreviewGuildMembers :
        strcmp(lpCmdLine,"--preview=guild-wire-editor")==0 ? PreviewGuildEditorWire :
        strcmp(lpCmdLine,"--preview=guild-wire-error")==0 ? PreviewGuildError :
        strcmp(lpCmdLine,"--preview=group-wire")==0 ? PreviewGroupWire :
        strcmp(lpCmdLine,"--preview=group-wire-error")==0 ? PreviewGroupError :
        strcmp(lpCmdLine,"--preview=skill-wire")==0 ? PreviewSkillWire :
        strcmp(lpCmdLine,"--preview=skill-wire-progress")==0 ? PreviewSkillProgress :
        strcmp(lpCmdLine,"--preview=progression")==0 ? PreviewProgression :
        strcmp(lpCmdLine,"--preview=level-tail")==0 ? PreviewLevelTail :
        strcmp(lpCmdLine,"--preview=ability-wire")==0 ? PreviewAbility :
        strcmp(lpCmdLine,"--preview=quest-wire")==0 ? PreviewQuestWire :
        strcmp(lpCmdLine,"--preview=quest-wire-detail")==0 ? PreviewQuestDetail :
        strcmp(lpCmdLine,"--preview=quest-wire-increment")==0 ? PreviewQuestIncrement :
        strcmp(lpCmdLine,"--preview=scene-map")==0 ? PreviewSceneMap :
        strcmp(lpCmdLine,"--preview=scene-portrait")==0 ? PreviewScenePortrait :
        strcmp(lpCmdLine,"--preview=scene-siege")==0 ? PreviewSceneSiege :
        strcmp(lpCmdLine,"--preview=scene-map-hidden")==0 ? PreviewSceneHidden :
        strcmp(lpCmdLine,"--preview=siege-expired")==0 ? PreviewSiegeExpired :
        strcmp(lpCmdLine,"--preview=consignment-input")==0 ? PreviewConsignmentInput :
        strcmp(lpCmdLine,"--preview=consignment-wire")==0 ? PreviewConsignmentWire :
        strcmp(lpCmdLine,"--preview=mail-wire")==0 ? PreviewMailWire :
        strcmp(lpCmdLine,"--preview=mail-wire-read")==0 ? PreviewMailWireRead :
        strcmp(lpCmdLine,"--preview=mail-notification")==0 ? PreviewMailNotify :
        strcmp(lpCmdLine,"--preview=siege")==0 ? PreviewSiege :
        strcmp(lpCmdLine,"--preview=siege-load")==0 ? PreviewSiegeLoad :
        strcmp(lpCmdLine,"--preview=siege-checks")==0 ? PreviewSiegeChecks :
        strcmp(lpCmdLine,"--preview=consignment-empty")==0 ? PreviewConsignmentEmpty :
        strcmp(lpCmdLine,"--preview=consignment-list")==0 ? PreviewConsignmentList :
        strcmp(lpCmdLine,"--preview=consignment-registration")==0 ? PreviewConsignmentRegistration :
        strcmp(lpCmdLine,"--preview=consignment-item")==0 ? PreviewConsignmentItem :
        strcmp(lpCmdLine,"--preview=consignment-price")==0 ? PreviewConsignmentPrice :
        strcmp(lpCmdLine,"--preview=consignment-buy")==0 ? PreviewConsignmentBuy :
        strcmp(lpCmdLine,"--preview=consignment-cancel")==0 ? PreviewConsignmentCancel :
        strcmp(lpCmdLine,"--preview=consignment-confirm")==0 ? PreviewConsignmentConfirm :
        strcmp(lpCmdLine,"--preview=consignment-checks")==0 ? PreviewConsignmentChecks :
        strcmp(lpCmdLine,"--preview=mail-inbox")==0 ? PreviewMailInbox :
        strcmp(lpCmdLine,"--preview=mail-read")==0 ? PreviewMailRead :
        strcmp(lpCmdLine,"--preview=mail-compose")==0 ? PreviewMailCompose :
        strcmp(lpCmdLine,"--preview=mail-gift")==0 ? PreviewMailGift :
        strcmp(lpCmdLine,"--preview=mail-outbox")==0 ? PreviewMailOutbox :
        strcmp(lpCmdLine,"--preview=mail-checks")==0 ? PreviewMailChecks :
        strcmp(lpCmdLine,"--preview=mail-gift-gold")==0 ? PreviewMailGold :
        strcmp(lpCmdLine,"--preview=mail-gift-item")==0 ? PreviewMailGiftItem :
        strcmp(lpCmdLine,"--preview=mail-body-boundary")==0 ? PreviewMailBody :
        strcmp(lpCmdLine,"--preview=mail-gold-hint")==0 ? PreviewMailGoldHint :
        strcmp(lpCmdLine,"--preview=mail-gift-hint")==0 ? PreviewMailGiftHint :
        strcmp(lpCmdLine,"--preview=trade-local-hint")==0 ? PreviewTradeLocal :
        strcmp(lpCmdLine,"--preview=trade-remote-hint")==0 ? PreviewTradeRemote :
        strcmp(lpCmdLine,"--preview=trade-held-hint")==0 ? PreviewTradeHeld :
        strcmp(lpCmdLine,"--preview=shop-red-armor-hint")==0 ? PreviewRedArmor :
        strcmp(lpCmdLine,"--preview=shop-blue-armor-hint")==0 ? PreviewBlueArmor :
        strcmp(lpCmdLine,"--preview=shop-armor-hint")==0 ? PreviewShopArmorHint :
        strcmp(lpCmdLine,"--preview=special-equipment-hint")==0 ? PreviewSpecialHint :
        strcmp(lpCmdLine,"--preview=shop-item-hint")==0 ? PreviewShopHint :
        strcmp(lpCmdLine,"--preview=storage-item-hint")==0 ? PreviewStorageHint :
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
		g_bProcState = _GAME_PROC;
		hudPreview.Load(strcmp(lpCmdLine,"--preview=hud-expanded")==0 || previewChatMode != 0,previewChatMode,previewExit,previewGroup);
		g_bProcState = _GAME_PROC;
	} else if (strncmp(lpCmdLine, preview, strlen(preview)) == 0) {
		g_xMainWnd.m_pxDefProcess = &g_xChrSelProc;
		g_xChrSelProc.Preview(lpCmdLine + strlen(preview));
		g_bProcState = _CHAR_SEL_PROC;
    } else if(g_xServerGroups.Load()) {
        g_xMainWnd.m_pxDefProcess=&g_xServerGroups;g_bProcState=_SVR_SEL_PROC;
    } else {
        g_xClientSocket.m_pxDefProc=g_xMainWnd.m_pxDefProcess=&g_xLoginProc;
        g_xLoginProc.Load();g_bProcState=_LOGIN_PROC;
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
            if(g_xClientSocket.PollConnection(GetTickCount())) {
                dwLastTime=timeGetTime();Sleep(1);continue;
            }
			dwTime = timeGetTime();
			dwDelay = dwTime - dwLastTime;
			dwLastTime = dwTime;

			if ( dwDelay!=0 /*&& g_xMainWnd.m_bIsWindowActive && g_xMainWnd.m_bIsWindowReady */)
			{
				switch ( g_bProcState )
				{
				case _SVR_SEL_PROC:
                    g_xServerGroups.RenderScene(dwDelay);break;
				case _LOGIN_PROC:
					g_xLoginProc.RenderScene(dwDelay);
					break;
				case _CHAR_SEL_PROC:
					if(previewHud) hudPreview.Render(dwDelay);
                    else g_xChrSelProc.RenderScene(dwDelay);
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
