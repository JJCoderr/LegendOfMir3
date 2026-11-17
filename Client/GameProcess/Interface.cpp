/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/



#include "StdAfx.h"
#include "TradeWire.h"
#include "MailWire.h"
#include "Common/LegacyPanel.h"
#include "Common/LoginMessages.h"



CInterface::CInterface()
{
	Init();
}



CInterface::~CInterface()
{
	Destroy();
}



/******************************************************************************************************************

	函数名 : CInterface::Init()

	作者 :
	编写日期 :

	目的   : 接口相关变量及对象初始化.
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInterface::Init()
{
	m_nIdxMsgBox		= 0;
	m_nFstLine			= 0;
    previousHint.clear(); hintHeight=6; hintExpanding=true;
    m_chatMode = 0;
    m_chatModeChanged = false;
    for (int i=0;i<3;++i) { m_chatRejected[i]=false; m_chatReceptionFrame[i]=-1; m_chatReceptionHint[i]=i==0?2637:i==1?2639:2643; }
	m_shBeltHeight		= 0;
	m_bShowKey			= FALSE;
	m_bMoveFocusedWnd	= FALSE;
	m_bBeltState		= _BELT_STATE_STAY;

	SetRect(&m_rcMain,  0, 0, 0, 0);
	SetRect(&m_rcChat,  0, 0, 0, 0);

	ZeroMemory(&m_stCommonItem, sizeof(ITEMSET));

	m_xClientSysMsg.Init();

	m_pxInterImg = NULL;

	m_xStatusWnd.Init();
    m_xSkillWnd.Init();
    m_xQuestWnd.Init();
    m_xQuickWnd.Init();
    m_xHorseWnd.Init();
	m_xStoreWnd.Init();
    m_xPortraitWnd.Init(); m_xPortraitWnd.Close(); m_portraitEntryPressed=false;
	m_xGuildWnd.Init();
	m_xGroupWnd.Init();
	m_xChatPopWnd.Init();
	m_xExchangeWnd.Init();
	m_xGroupPopWnd.Init();
	m_xInventoryWnd.Init();
	m_xGuildMasterWnd.Init();

	for ( INT nCnt = 0; nCnt < _MAX_INTER_BTN; nCnt++ )
	{
		m_xInterBtn[nCnt].Init();
	}

	m_xScrlBar.Init();

	m_xChat.ClearAllNodes();
    m_chatHistory.clear();
    for(auto& lines:m_chatLines) lines.clear();
    m_chatFirst[0]=m_chatFirst[1]=0;m_chatLayoutMode=0;
	m_xWndOrderList.ClearAllNodes();
}



/******************************************************************************************************************

	函数名 : CInterface::Destroy()

	作者   :
	编写日期 :

	目的   : 接口相关变量、对象的销毁及初始化。
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInterface::Destroy()
{
    m_xPortraitWnd.Close();m_xPortraitWnd.ApplyOwnPhoto(nullptr,0);
    m_targetDisplay.Destroy();
    m_minimap.Destroy();
    m_portraits.Destroy();
	m_xClientSysMsg.Destroy();

	m_pxInterImg = NULL;

	m_xInventoryWnd.Destroy();
	m_xStoreWnd.Destroy();
	m_xStatusWnd.Destroy();
    m_xSkillWnd.Destroy();
    m_xQuestWnd.Destroy();
    m_xGuildEditor.Destroy();
    m_xMailWnd.Destroy();
    m_xConsignmentWnd.Destroy();m_xConsignmentRegistration.Destroy();
    m_xSiegeWnd.Destroy();
    m_xQuickWnd.Destroy();
    m_xHorseWnd.Destroy();
	m_xGuildWnd.Destroy();
	m_xGroupWnd.Destroy();
	m_xExchangeWnd.Destroy();
	m_xGuildMasterWnd.Destroy();
	m_xGroupPopWnd.Destroy();
	m_xChatPopWnd.Destroy();

	for ( INT nCnt = 0; nCnt < _MAX_INTER_BTN; nCnt++ )
	{
		m_xInterBtn[nCnt].Destroy();
	}

	m_xChat.ClearAllNodes();
	m_xWndOrderList.ClearAllNodes();

	Init();
}


VOID CInterface::CreateInterface(CImageHandler* pxImgHandler)
{
	m_pxInterImg = &(pxImgHandler->m_xImageList[_IMAGE_INTER]);

	//------------------------------------------------------------------------------------------------------------
	// 聊天编辑窗口的位置指定.
	g_xChatEditBox.MoveScaled(
			   g_xMainWnd.m_rcWindow.left+_INPUT_EDITWND_XPOS, g_xMainWnd.m_rcWindow.top+_INPUT_EDITWND_YPOS,
			   _INPUT_EDITWND_WIDTH, _INPUT_EDITWND_HEIGHT, TRUE);
	//------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------
	// 加载接口图像.
	m_pxInterImg->NewLoad(".\\Data\\gameinter.wil");
    m_portraits.Destroy();
    m_targetDisplay.Init();
    m_minimap.Init();
    m_portraits.NewLoad(".\\Data\\ProgUse.wil");
	//------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------
	// 主界面区域设置.
	if ( m_pxInterImg->NewSetIndex(_WNDIMGIDX_MAIN) )
	{
		SetRect(&m_rcMain, 
				0, (601-m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight),
				m_pxInterImg->m_lpstNewCurrWilImageInfo->shWidth, 601);
	}
	//------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------
	// 聊天区域设置。
	SetRect(&m_rcChat, 189, 492, 530, 567);
	//------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------
	// 创建各窗口。
	m_xInventoryWnd.CreateInventoryWnd(_WND_ID_INVENTORY, m_pxInterImg, 1220, 518, 0, 282, 466, TRUE);
	m_xStatusWnd.CreateStatusWnd(_WND_ID_STATUS, m_pxInterImg, 1280, 0, 0, 328, 466, TRUE);
    m_xSkillWnd.CreateGameWnd(_WND_ID_SKILL,m_pxInterImg,1620,TRUE,348,0,360,488);
    m_xPortraitWnd.CreateGameWnd(_WND_ID_PORTRAIT,m_pxInterImg,2090,FALSE,0,0,580,404);
    m_xMailWnd.Create(m_pxInterImg,_WND_ID_MAIL);
    m_xConsignmentWnd.Create(m_pxInterImg,false);m_xConsignmentRegistration.Create(m_pxInterImg,true);
    m_xSiegeWnd.Create(m_pxInterImg);
    m_xPortraitWnd.SetCacheContext(g_xClientSocket.selectedServerName,g_szCharName);
	m_xStoreWnd.CreateStoreWnd(_WND_ID_STORE, m_pxInterImg, _WNDIMGIDX_STORE, 0, 0, 0, 0, TRUE);
	m_xExchangeWnd.CreateExchangeWnd(_WND_ID_EXCHANGE, m_pxInterImg, 1390, 0, 0, 494, 330, TRUE);
	m_xGuildMasterWnd.CreateGuildMasterWnd(_WND_ID_GUILDMASTER, m_pxInterImg, _WNDIMGIDX_GUILDMASTER, 0, 0, 0, 0, TRUE);
	m_xGuildWnd.CreateGuildWnd(_WND_ID_GUILD, m_pxInterImg, 1300, 102, 22, 594, 444, TRUE);
	m_xGroupWnd.CreateGroupWnd(_WND_ID_GROUP, m_pxInterImg, 1360, 272, 123, 258, 244, TRUE);
	m_xGroupPopWnd.CreateGroupPopWnd(_WND_ID_GROUPPOP, m_pxInterImg, 1240, 220, 112, 360, 268, TRUE);
	m_xChatPopWnd.CreateChatPopWnd(_WND_ID_CHATPOP, m_pxInterImg, _WNDIMGIDX_CHATPOP, 0, 0, 0, 0, TRUE);

	m_xOptionWnd.CreateOptionWnd(_WND_ID_OPTION, m_pxInterImg, 1290, 145, 30, 510, 468, TRUE);
	m_xQuestWnd.CreateQuestWnd(_WND_ID_QUEST, m_pxInterImg, 1420, 0, 0, 340, 440, TRUE);
    m_xGuildEditor.CreateGameWnd(-1,m_pxInterImg,1326,TRUE,107,110,584,252);
    m_xQuickWnd.CreateGameWnd(_WND_ID_QUICK,m_pxInterImg,1210,TRUE,73,413,280,48);
	m_xHorseWnd.CreateHorseWnd(_WND_ID_HORSE, m_pxInterImg, 1400, 0, 0, 276, 322, TRUE);


	m_xNPCWnd.CreateNPCChatWnd(_WND_ID_NPCCHAT, m_pxInterImg, _WNDIMGIDX_NPCCHAT, 0, 0, 0, 0, TRUE);				// 勇气
	//------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------
	// 按钮初始化。
	m_xInterBtn[0].CreateGameBtn(m_pxInterImg, 1170, 1170, 637, 473, _BTN_TYPE_NOFOCUS);		// 交易窗口。
	m_xInterBtn[1].CreateGameBtn(m_pxInterImg, 1172, 1172, 637, 507, _BTN_TYPE_NOFOCUS);		// 小地图.
	m_xInterBtn[2].CreateGameBtn(m_pxInterImg, 1174, 1174, 637, 542, _BTN_TYPE_NOFOCUS);		// 武功窗口.

	m_xInterBtn[3].CreateGameBtn(m_pxInterImg, 1176, 1176, 7, 539, _BTN_TYPE_NOFOCUS);	// 退出游戏.
	m_xInterBtn[4].CreateGameBtn(m_pxInterImg, 1178, 1178, 108, 539, _BTN_TYPE_NOFOCUS);	// 注销.
	m_xInterBtn[5].CreateGameBtn(m_pxInterImg, 1180, 1181, 706, 475, _BTN_TYPE_FOCUS);	// 队伍.
	m_xInterBtn[6].CreateGameBtn(m_pxInterImg, 1182, 1183, 674, 478, _BTN_TYPE_FOCUS);	// 行会.

	m_xInterBtn[7].CreateGameBtn(m_pxInterImg,1168,1168,619,472);	// 腰带 UP, DOWN.

	m_xInterBtn[8].CreateGameBtn(m_pxInterImg, 1184, 1185, 739, 500, _BTN_TYPE_FOCUS);	// 技能窗口。
	m_xInterBtn[9].CreateGameBtn(m_pxInterImg, 1190, 1191, 742, 528, _BTN_TYPE_FOCUS);	// 聊天弹窗.
	m_xInterBtn[10].CreateGameBtn(m_pxInterImg, 1186, 1187, 742, 478, _BTN_TYPE_FOCUS);	// 任务.
	m_xInterBtn[11].CreateGameBtn(m_pxInterImg, 1188, 1189, 706, 539, _BTN_TYPE_FOCUS);	// 选项.
	m_xInterBtn[12].CreateGameBtn(m_pxInterImg,1205,1205,150,469,_BTN_TYPE_NOFOCUS);	// 帮助.
	m_xInterBtn[13].CreateGameBtn(m_pxInterImg, 1192, 1193, 674, 528, _BTN_TYPE_FOCUS);	// 马窗口。
	m_xInterBtn[14].CreateGameBtn(m_pxInterImg, 1194, 1195, 682, 500, _BTN_TYPE_FOCUS);	// 背包窗口.
	m_xInterBtn[15].CreateGameBtn(m_pxInterImg, 1196, 1197, 711, 498, _BTN_TYPE_FOCUS);	// 状态窗口.
	//------------------------------------------------------------------------------------------------------------

    const int chatButtons[7][3] = {
        {1330,186,557},{1333,226,557},{1335,265,556},{1337,305,556},
        {1338,346,556},{1340,386,556},{1342,426,556}
    };
    for (int i=0;i<7;++i)
        m_xInterBtn[16+i].CreateGameBtn(m_pxInterImg,chatButtons[i][0],chatButtons[i][0],
            chatButtons[i][1],chatButtons[i][2],_BTN_TYPE_NOFOCUS);

    // Godot HudLayout::hit_overrides, mapped to the CLIENT's event IDs.
    m_xInterBtn[_BTN_ID_GROUP].SetHitRect(708,476,34,19);
    m_xInterBtn[_BTN_ID_GUILD].SetHitRect(674,475,29,25);
    m_xInterBtn[_BTN_ID_MAGIC].SetHitRect(740,502,28,29);
    m_xInterBtn[_BTN_ID_QUEST].SetHitRect(742,478,35,24);
    m_xInterBtn[_BTN_ID_OPTION].SetHitRect(709,539,35,20);
    m_xInterBtn[_BTN_ID_POPUP].SetHitRect(744,531,36,24);
    m_xInterBtn[_BTN_ID_HORSE].SetHitRect(675,532,34,22);
    m_xInterBtn[_BTN_ID_BAG].SetHitRect(682,504,30,27);
    m_xInterBtn[_BTN_ID_CHAR].SetHitRect(712,498,28,37);
    m_xInterBtn[_BTN_ID_HELP].SetHitRect(150,469,24,15);

	m_xScrlBar.CreateScrlBar(m_pxInterImg, 68, _MAX_CHATLINE, 12, 84, 12);

	m_xMsgBox.CreateWnd(m_pxInterImg,FALSE,0,0,0,0);
    m_xMsgBox.SetOrigin(220,112,false);	// 勇气
}



VOID CInterface::MsgAdd(DWORD color, DWORD background, CHAR* message)
{
    constexpr int MaximumBytes=200,HistoryLimit=80;
    if(!message || !*message || strlen(message)>MaximumBytes) return;
    m_xChatPopWnd.MsgAdd(color,background,message);
    m_chatHistory.push_back(ChatMessage{color,background,message});
    if(m_chatHistory.size()>=HistoryLimit) m_chatHistory.erase(m_chatHistory.begin());
    m_chatFirst[m_chatLayoutMode]=m_nFstLine;
    HDC dc=GetDC(g_xMainWnd.GetSafehWnd());
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font=g_xMainWnd.CreateGameFont(face,9);HGDIOBJ old=SelectObject(dc,font);
    for(int mode=0;mode<2;++mode) {
        const int width=mode?429:341;
        std::string wrapped;int start=0,highBytes=0,count=1;
        for(int i=0;message[i];++i) {
            SIZE size={};GetTextExtentPoint32A(dc,message+start,i-start+1,&size);
            const bool high=static_cast<signed char>(message[i])<0;
            if(size.cx<=width) {if(high) ++highBytes;wrapped+=message[i];}
            else if(high && highBytes%2) {wrapped.back()='`';start=i-1;i-=2;--highBytes;++count;}
            else {wrapped+='`';start=i;--i;++count;}
        }
        char rows[5][260]={};
        sscanf(wrapped.c_str(),"%259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c %259[^`]%*c",rows[0],rows[1],rows[2],rows[3],rows[4]);
        for(int row=0;row<min(count,mode?3:5);++row) {
            // MIR3 43682E/436902 checks the expanded index in BOTH loops.
            if(int(m_chatLines[1].size())-m_chatFirst[1]==14) ++m_chatFirst[1];
            if(!mode && int(m_chatLines[0].size())-m_chatFirst[0]==5) ++m_chatFirst[0];
            m_chatLines[mode].push_back(ChatMessage{color,background,rows[row]});
        }
    }
    SelectObject(dc,old);DeleteObject(font);ReleaseDC(g_xMainWnd.GetSafehWnd(),dc);
    for(int mode=0;mode<2;++mode) while(m_chatLines[mode].size()>=HistoryLimit) {
        m_chatLines[mode].erase(m_chatLines[mode].begin());
        if(m_chatFirst[mode]>0) --m_chatFirst[mode];
    }
    m_nFstLine=m_chatFirst[m_chatLayoutMode];
    ReflowChat();
}

void CInterface::ReflowChat()
{
    m_chatFirst[m_chatLayoutMode]=m_nFstLine;
    m_chatLayoutMode=m_chatExpanded?1:0;
    m_nFstLine=m_chatFirst[m_chatLayoutMode];
    m_xChat.ClearAllNodes();
    for(const auto& message:m_chatLines[m_chatLayoutMode]) {
        CHAT line={};line.dwFontColor=message.color;line.dwBackColor=message.background;
        strcpy(line.pszChat,message.text.c_str());m_xChat.AddNode(line);
    }
}

/******************************************************************************************************************

	函数名 : CInterface::ShowWndList()

	编写者 :
	编写日期 :

	目的   : 显示处于 Active 状态的窗口列表.
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInterface::ShowWndList()
{	
	if ( m_xWndOrderList.GetCounter() != 0 )
	{
		m_xWndOrderList.MoveCurrentToTop();

		INT* pnCurrID;

		for ( INT nCnt = 0; nCnt < m_xWndOrderList.GetCounter(); nCnt++ )
		{
			pnCurrID = m_xWndOrderList.GetCurrentData();

			switch ( *pnCurrID )
			{
			case _WND_ID_INVENTORY:
				m_xInventoryWnd.ShowInventoryWnd();
				break;
			case _WND_ID_SKILL:
				m_xSkillWnd.ShowSkillWnd();
				break;
case _WND_ID_STATUS:
				m_xStatusWnd.ShowStatusWnd();
				break;
			case _WND_ID_STORE:
                // Merchant overlays are drawn after ordinary panels.
				break;
			case _WND_ID_EXCHANGE:
				m_xExchangeWnd.ShowExchangeWnd();
				break;
			case _WND_ID_GUILDMASTER:
				m_xGuildMasterWnd.ShowGuildMasterWnd();
				break;
			case _WND_ID_GUILD:
				m_xGuildWnd.ShowGuildWnd();
				break;
			case _WND_ID_GROUP:
				m_xGroupWnd.ShowGroupWnd();
				break;
			case _WND_ID_GROUPPOP:
				m_xGroupPopWnd.ShowGroupPopWnd();
				break;
			case _WND_ID_CHATPOP:
				m_xChatPopWnd.ShowChatPopWnd();
				break;
			case _WND_ID_NPCCHAT:					// 勇气
                // NPC overlays stay above the panel activation order.
				break;

			case _WND_ID_QUEST:
				m_xQuestWnd.ShowQuestWnd();
				break;
			case _WND_ID_OPTION:
				m_xOptionWnd.ShowOptionWnd();
				break;
            case SiegeLayout::WindowId:m_xSiegeWnd.Show();break;
            case ConsignmentLayout::BrowserId:m_xConsignmentWnd.Show();break;
            case ConsignmentLayout::RegistrationId:m_xConsignmentRegistration.Show();break;
            case _WND_ID_MAIL: m_xMailWnd.Show(); break;
            case _WND_ID_PORTRAIT: m_xPortraitWnd.Show(); break;
			case _WND_ID_QUICK:
                m_xQuickWnd.ShowQuickWnd();
                break;
			case _WND_ID_HORSE:
				m_xHorseWnd.ShowHorseWnd();
				break;
			}			

			m_xWndOrderList.MoveNextNode();
		}
	}	
}



/******************************************************************************************************************

	函数名 : CInterface::ShowChatList()

	作者 : 
	日期 : 

	目的   : 显示聊天列表.
	输出   : VOID 

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
void CInterface::LayoutChat()
{
    const int top = m_chatExpanded ? 320 : 492;
    SetRect(&m_rcChat,189,top,m_chatExpanded ? 618 : 530,top+ChatRows()*15);
    g_xChatEditBox.MoveScaled(g_xMainWnd.m_rcWindow.left+185,
        g_xMainWnd.m_rcWindow.top+(m_chatExpanded ? 534 : 575),
        m_chatExpanded ? 432 : 344,15,TRUE);
    m_xInterBtn[_BTN_ID_BELT].ChangeRect(619,m_chatExpanded ? 305 : 472);
    ReflowChat();
}

void CInterface::FocusChatMode()
{
    if (m_chatMode) {
        const char* prefix = m_chatMode == 1 ? "!!" : "!~";
        SetWindowTextA(g_xChatEditBox.GetSafehWnd(),prefix);
        SendMessage(g_xChatEditBox.GetSafehWnd(),EM_SETSEL,2,2);
    }
    ShowWindow(g_xChatEditBox.GetSafehWnd(),SW_SHOW);
    SetFocus(g_xChatEditBox.GetSafehWnd());
}

VOID CInterface::ShowChatList()
{
    if (m_xChat.CheckEmpty()) return;
    auto* surface = g_xMainWnd.GetBackBuffer();
    HDC dc;
    if (FAILED(surface->GetDC(&dc))) return;
    const char face[] = {char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font = g_xMainWnd.CreateGameFont(face,9);
    HGDIOBJ previous = SelectObject(dc,font);
    const int saved = SaveDC(dc);
    IntersectClipRect(dc,m_rcChat.left,m_rcChat.top,m_rcChat.right,m_rcChat.bottom);
    SetBkMode(dc,TRANSPARENT);
    m_xChat.MoveCurrentToTop();
    m_xChat.MoveNode(m_nFstLine);
    const int rows = min(ChatRows(),m_xChat.GetCounter()-m_nFstLine);
    for (int row=0;row<rows;++row) {
        const CHAT* line = m_xChat.GetCurrentData();
        const int length = (int)strlen(line->pszChat);
        const int y = m_rcChat.top+row*15;
        if (line->dwBackColor) {
            SIZE extent = {};
            GetTextExtentPoint32A(dc,line->pszChat,length,&extent);
            RECT background = {m_rcChat.left,y,m_rcChat.left+extent.cx,y+extent.cy};
            SetBkColor(dc,line->dwBackColor);
            ExtTextOutA(dc,m_rcChat.left,y,ETO_OPAQUE,&background,NULL,0,NULL);
        }
        SetTextColor(dc,line->dwFontColor);
        TextOutA(dc,m_rcChat.left,y,line->pszChat,length);
        m_xChat.MoveNextNode();
    }
    RestoreDC(dc,saved);
    SelectObject(dc,previous);
    DeleteObject(font);
    surface->ReleaseDC(dc);
}



VOID CInterface::RenderInterface(INT nLoopTime, POINT ptMousePos)
{
    m_xSiegeWnd.ExpireObjects(GetTickCount());
    m_minimap.showMonsters=m_xOptionWnd.m_values[7];
    m_xPortraitWnd.SendRequest();
	INT nX, nY, nCnt;

	// 窗口.


	// 鼠标物品.


	// 腰带物品。
	ShowBeltItem();
    m_minimap.player={g_xGameProc.m_xMyHero.m_wPosX,g_xGameProc.m_xMyHero.m_wPosY};
    m_minimap.portraitBlink+=nLoopTime;if(m_minimap.portraitBlink>800) m_minimap.portraitBlink=0;
    if(!m_chatExpanded) {
        DrawSelectionFrame(*m_pxInterImg,1490,534,486,SelectionBlend::Normal,false);
        if(m_minimap.portraitMode) m_minimap.ShowPortrait();
        else if(!m_xPortraitWnd.ShowOwnPhoto()) {
            const auto& hero=g_xGameProc.m_xMyHero;
            if(hero.m_bJob<=2) DrawSelectionFrame(m_portraits,650+2*hero.m_bJob+(hero.m_stFeature.bGender!=0),534,486,SelectionBlend::Normal,false);
        }
        m_targetDisplay.ShowPortrait(m_portraits,m_xOptionWnd.m_values[20]);
    }


	// 主界面.
    if (m_chatExpanded) {
        RECT backing = {185,317,618,530};
        DrawLegacyPanel(backing,RGB(30,30,30),150,0,false);
    }
	if ( m_pxInterImg->NewSetIndex(_WNDIMGIDX_MAIN) )
	{
 		g_xMainWnd.DrawWithImageForCompClipRgn(m_rcMain.left, m_rcMain.top,	m_rcMain.right-m_rcMain.left, m_rcMain.bottom-m_rcMain.top,
											   (WORD*)m_pxInterImg->m_pbCurrImage, _CLIP_WIDTH, 600);
	}

	// 聊天列表.
    if (m_pxInterImg->NewSetIndex(m_chatExpanded ? 1162 : 1161)) {
        const auto& skin = *m_pxInterImg->m_lpstNewCurrWilImageInfo;
        g_xMainWnd.DrawWithImageForComp(178,m_chatExpanded ? 281 : 448,skin.shWidth,skin.shHeight,
            (WORD*)m_pxInterImg->m_pbCurrImage);
    }
	ShowChatList();

	// 状态值.
	ShowGameStatus();

	// 按钮.
	for ( nCnt = 0; nCnt < VisibleButtons(); nCnt++)
	{
        if (m_chatExpanded && m_chatModeChanged && (nCnt == 18 || nCnt == 19)) {
            const int frame = nCnt == 18 ? (m_chatMode == 1 ? 1335 : 1334) : (m_chatMode == 2 ? 1337 : 1336);
            if (m_pxInterImg->NewSetIndex(frame)) {
                const auto& info = *m_pxInterImg->m_lpstNewCurrWilImageInfo;
                const RECT area = m_xInterBtn[nCnt].GetGameBtnRect();
                g_xMainWnd.DrawWithImageForComp(area.left,area.top,info.shWidth,info.shHeight,
                    (WORD*)m_pxInterImg->m_pbCurrImage);
            }
        }
        if (nCnt >= 20 && m_chatReceptionFrame[nCnt-20] >= 0 &&
            m_pxInterImg->NewSetIndex(m_chatReceptionFrame[nCnt-20])) {
            const auto& info = *m_pxInterImg->m_lpstNewCurrWilImageInfo;
            const RECT area = m_xInterBtn[nCnt].GetGameBtnRect();
            g_xMainWnd.DrawWithImageForComp(area.left,area.top,info.shWidth,info.shHeight,
                (WORD*)m_pxInterImg->m_pbCurrImage);
        }
		m_xInterBtn[nCnt].ShowGameBtn();
	}

	// 滚动条.
	m_xScrlBar.ShowChatScroll(m_chatExpanded, m_nFstLine, m_xChat.GetCounter());

	// 客户端消息(左上角).
	m_xClientSysMsg.ShowSysMsg(nLoopTime, 30, 30);

	// 勇气
    const int hoveredWindow=(m_xMsgBox.IsActive() || m_xGuildEditor.m_bActive)?-1:GetWindowInMousePos(ptMousePos);
    m_xInventoryWnd.UpdatePlacementPointer(ptMousePos,hoveredWindow==_WND_ID_INVENTORY);
    m_xStatusWnd.UpdatePointer(ptMousePos,hoveredWindow==_WND_ID_STATUS);
    m_xSkillWnd.UpdatePointer(ptMousePos,hoveredWindow==_WND_ID_SKILL);
    m_xHorseWnd.UpdatePointer(ptMousePos,hoveredWindow==_WND_ID_HORSE);
    m_xNPCWnd.UpdatePointer(ptMousePos,hoveredWindow==_WND_ID_NPCCHAT);
    m_xMailWnd.hoverRow=hoveredWindow==_WND_ID_MAIL?m_xMailWnd.RowAt(ptMousePos):-1;
    const RECT portraitHover={535,486,616,580};
    if (!m_chatExpanded && PtInRect(&portraitHover,ptMousePos)) DrawSelectionFrame(*m_pxInterImg,m_portraitEntryPressed?2097:2096,535,566,SelectionBlend::Normal,false);
    m_xMailWnd.SyncEditors(IsTopWindow(_WND_ID_MAIL) && !m_xMsgBox.IsActive() && !m_xGuildEditor.m_bActive && !m_xStoreWnd.m_bActive && !m_xNPCWnd.m_bActive);
    for(auto* panel:{&m_xConsignmentWnd,&m_xConsignmentRegistration}) {
        panel->hovered=hoveredWindow==panel->m_nID?panel->RowAt(ptMousePos):-1;
        panel->SyncEditors(IsTopWindow(panel->m_nID) && !m_xMsgBox.IsActive() && !m_xGuildEditor.m_bActive && !m_xStoreWnd.m_bActive && !m_xNPCWnd.m_bActive);
    }
    m_xSiegeWnd.ShowEntry();
    m_xMailWnd.ShowNotification();
    ShowWndList();
    m_minimap.player={g_xGameProc.m_xMyHero.m_wPosX,g_xGameProc.m_xMyHero.m_wPosY};
    m_minimap.Show(*m_pxInterImg,GetTickCount());
    if(!m_chatExpanded) m_targetDisplay.ShowVitals(m_portraits);
    ShowMagicKey();
    if (m_xNPCWnd.m_bActive) m_xNPCWnd.ShowNPCChatWnd();
    if (m_xStoreWnd.m_bActive) m_xStoreWnd.ShowStoreWnd();
	if ( m_stCommonItem.bSetted && !m_stCommonItem.bIsHideItem )
	{
		nX = ptMousePos.x - _INVENTORY_CELL_WIDTH /2;
		nY = ptMousePos.y - _INVENTORY_CELL_HEIGHT/2;
		m_stCommonItem.xItem.DrawItem(nX, nY);
	}
    m_xGuildEditor.Show();
    if(!m_chatExpanded) m_targetDisplay.ShowInformation(m_portraits);
	m_xMsgBox.RenderMessageBox(0);

    ShowButtonHint(ptMousePos);
    m_xMailWnd.ShowAttachmentHint(ptMousePos);
    m_xConsignmentWnd.ShowItemHint(ptMousePos);m_xConsignmentRegistration.ShowItemHint(ptMousePos);
    m_xExchangeWnd.ShowHeld(ptMousePos);
}



VOID CInterface::ShowBeltItem()
{
	INT nX, nY;

	if ( m_bBeltState == _BELT_STATE_UP )
	{
		m_shBeltHeight += 10;

		if ( m_shBeltHeight >= 46 )	
		{
			m_shBeltHeight = 46;
		}
	}
	else if ( m_bBeltState == _BELT_STATE_DOWN )
	{
		m_shBeltHeight -= 10;
		if ( m_shBeltHeight <= 0 )
		{
			m_shBeltHeight = 0;
			m_bBeltState   = _BELT_STATE_STAY;
		}
	}

	if ( m_bBeltState != _BELT_STATE_STAY )
	{
		if ( m_pxInterImg->NewSetIndex(_IMGIDX_BELT) )
		{
 			g_xMainWnd.DrawWithImageForCompClipRgn(275, 457-m_shBeltHeight,
												   m_pxInterImg->m_lpstNewCurrWilImageInfo->shWidth,
												   m_pxInterImg->m_lpstNewCurrWilImageInfo->shHeight,
												   (WORD*)m_pxInterImg->m_pbCurrImage, _CLIP_WIDTH, 600);
			// 腰带物品绘制。
			for ( INT nCnt = 0; nCnt < _MAX_BELT_ITEM; nCnt++ )
			{
				if ( m_xInventoryWnd.m_stBeltItem[nCnt].bSetted )
				{
					nX = 275 + 8 + nCnt*40;
					nY = 411 + 6 + 46 - m_shBeltHeight;

					m_xInventoryWnd.m_stBeltItem[nCnt].xItem.DrawItem(nX, nY);
				}
			}
		}

	}
}



bool CInterface::PortraitEntryHit(POINT point)
{
    const RECT hover={535,486,616,580};
    if (m_chatExpanded || !PtInRect(&hover,point) || !m_pxInterImg->NewSetIndex(2096)) return false;
    const auto& info=*m_pxInterImg->m_lpstNewCurrWilImageInfo;
    const RECT box={535,566,535+info.shWidth,566+info.shHeight}; return PtInRect(&box,point)!=FALSE;
}
VOID CInterface::ShowGameStatus()
{
	FLOAT	fMyHPRate;
	FLOAT	fMyMPRate;				
	FLOAT	fMyWetRate;				
	FLOAT	fMyExpRate;
	WORD	wJobIcon;
	CHAR	pszBuff[MAX_PATH];
	RECT	rcStats;
	DWORD	dwFntClr;

	BYTE	bMyJob  = g_xGameProc.m_xMyHero.m_bJob;
	BYTE	bMyLevel= g_xGameProc.m_xMyHero.m_stAbility.bLevel;
	WORD	wMyAC   = g_xGameProc.m_xMyHero.m_stAbility.wAC;
	WORD	wMyDC   = g_xGameProc.m_xMyHero.m_stAbility.wDC;
	WORD	wMyMC   = g_xGameProc.m_xMyHero.m_stAbility.wMC;
	WORD	wMySC   = g_xGameProc.m_xMyHero.m_stAbility.wSC;

	WORD	wHP		= g_xGameProc.m_xMyHero.m_stAbility.wHP;
	WORD	wMP		= g_xGameProc.m_xMyHero.m_stAbility.wMP;
	WORD	wWet	= g_xGameProc.m_xMyHero.m_stAbility.wWeight;
	DWORD	dwExp	= g_xGameProc.m_xMyHero.m_stAbility.dwExp;

	BYTE	bJob	= g_xGameProc.m_xMyHero.m_bJob;
	
	// 一天的状态值(夜晚, 白天, 傍晚, 黎明)
    // Mir3 HUD has no legacy 130..133 day-state overlay.

	// 绘制 HP, MP.
	// 比例检查。
    // Mir3 HudModel: zero capacity is empty; low-level warriors share HP in both halves.
    const auto& ability = g_xGameProc.m_xMyHero.m_stAbility;
    const auto ratio = [](double value, double maximum) -> float {
        return maximum > 0 ? float(min(1.0, value / maximum)) : 0.0f;
    };
    fMyHPRate = ratio(wHP, ability.wMaxHP);
    fMyMPRate = ratio(wMP, ability.wMaxMP);
    fMyExpRate = ratio(dwExp, ability.dwMaxExp);
    fMyWetRate = ratio(wWet, ability.wMaxWeight);
    const bool sharedHP = bJob == 0 && ability.bLevel < 24;
    const auto drawMeter = [&](int frame, int x, int y, float fill) {
        if (!m_pxInterImg->NewSetIndex(frame)) return;
        const auto& image = *m_pxInterImg->m_lpstNewCurrWilImageInfo;
        RECT crop = {0, LONG(image.shHeight - image.shHeight * double(fill)),
            image.shWidth, image.shHeight};
        if (crop.top < crop.bottom)
            g_xMainWnd.DrawWithImageForComp(m_rcMain.left+x,m_rcMain.top+y,
                crop,(WORD*)m_pxInterImg->m_pbCurrImage);
    };
    drawMeter(1163,33,8,fMyHPRate);
    drawMeter(sharedHP ? 1165 : 1164,73,8,sharedHP ? fMyHPRate : fMyMPRate);
    drawMeter(1167,165,26,fMyWetRate);
    drawMeter(1166,152,26,fMyExpRate);

    const auto centeredText = [&](RECT area, COLORREF color, HFONT font) {
        HDC dc;
        auto* surface = g_xMainWnd.GetBackBuffer();
        if (FAILED(surface->GetDC(&dc))) { DeleteObject(font); return; }
        HGDIOBJ old = SelectObject(dc,font);
        SetBkMode(dc,TRANSPARENT);
        SetTextColor(dc,color);
        DrawTextA(dc,pszBuff,-1,&area,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        SelectObject(dc,old);
        DeleteObject(font);
        surface->ReleaseDC(dc);
    };
    const char levelFace[] = {char(0xb1),char(0xbc),char(0xb8),char(0xb2),0};
    const char normalFace[] = {char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    sprintf(pszBuff,"%d",ability.bLevel);
    centeredText(m_chatExpanded ? RECT{393,298,409,314} : RECT{393,466,409,482},0x00e1e1,
        g_xMainWnd.CreateGameFont(levelFace,8,0,FW_BOLD));

    sprintf(pszBuff,"%s : [%d,%d]",g_xGameProc.m_szMapName,
        g_xGameProc.m_xMyHero.m_wPosX,g_xGameProc.m_xMyHero.m_wPosY);
    RECT coordinates = {6,575,139,592};
    DrawLegacyPanel(coordinates,RGB(100,75,50),100,0,false);
    const POINT shadows[4] = {{-1,-1},{1,-1},{-1,1},{1,1}};
    for (const auto& shadow : shadows) {
        RECT area = coordinates;
        OffsetRect(&area,shadow.x,shadow.y);
        centeredText(area,0x0a0a0a,g_xMainWnd.CreateGameFont(normalFace,9));
    }
    centeredText(coordinates,0xc8ffff,g_xMainWnd.CreateGameFont(normalFace,9));

    const WORD ranges[2] = {
        m_primaryStat[0] ? wMyAC : ability.wMAC,
        m_primaryStat[1] ? wMyDC : bJob == _JOB_DOSA ? wMySC : wMyMC
    };
    for (int side = 0; side < 2; ++side) {
        const int frame = 1500 + side + (m_primaryStat[side] ? 0 : 2);
        if (m_pxInterImg->NewSetIndex(frame)) {
            const auto& image = *m_pxInterImg->m_lpstNewCurrWilImageInfo;
            g_xMainWnd.DrawWithImageForComp(side ? 715 : 635,572,
                image.shWidth,image.shHeight,(WORD*)m_pxInterImg->m_pbCurrImage);
        }
        sprintf(pszBuff,"%d-%d",LOBYTE(ranges[side]),HIBYTE(ranges[side]));
        centeredText(side ? RECT{742,570,798,594} : RECT{663,570,713,594},
            0x32c8ff,g_xMainWnd.CreateGameFont(normalFace,9));
    }
}


VOID CInterface::ShowMagicKey()
{
    if (!m_bShowKey) return;
    for (int index=0;index<12;++index) {
        const int x=40*(index+index/4);
        if (const int id=m_xSkillWnd.Shortcut(index)) {
            if (id) DrawSelectionFrame(g_xGameProc.m_xImage.m_xImageList[_IMAGE_MICON],999+id,x,2,SelectionBlend::Alpha,false,200);
        }
        DrawSelectionFrame(*m_pxInterImg,1450+index,x-13,-11,SelectionBlend::Alpha,false,150,nullptr,150);
    }
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 窗口管理部分/////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************************************************************************

	函数名 : CInterface::GetWindowInMousePos()

	作者 : 
	日期 : 

	目的   : 返回鼠标指针所在区域内的窗口列表中第一个窗口的 ID.
	输入   : POINT ptMouse
	输出   : 

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
INT	CInterface::GetWindowInMousePos(POINT ptMouse)
{
    if (m_xStoreWnd.m_bActive && PtInRect(&m_xStoreWnd.m_rcWnd,ptMouse)) return _WND_ID_STORE;
    if (m_xNPCWnd.m_bActive && PtInRect(&m_xNPCWnd.m_rcWnd,ptMouse)) return _WND_ID_NPCCHAT;
    const RECT minimapBounds=m_minimap.Bounds();
    if(m_minimap.visible && PtInRect(&minimapBounds,ptMouse)) return _WND_ID_MINIMAP;
	if ( m_xWndOrderList.GetCounter() != 0 )
	{
		RECT rcWnd;
		INT* pnCurrID;
		m_xWndOrderList.MoveCurrentToTail();

		for ( INT nCnt = m_xWndOrderList.GetCounter()-1; nCnt >= 0 ; nCnt-- )
		{
			pnCurrID = m_xWndOrderList.GetCurrentData();

			switch ( *pnCurrID )
			{
			case _WND_ID_INVENTORY:
				rcWnd = m_xInventoryWnd.GetGameWndRect();
				break;
			case _WND_ID_SKILL:
				if (PtInRect(&m_xSkillWnd.m_keyMenu,ptMouse)) return _WND_ID_SKILL;
                rcWnd = m_xSkillWnd.GetGameWndRect();
				break;
case _WND_ID_STATUS:
				rcWnd = m_xStatusWnd.GetGameWndRect();
				break;
			case _WND_ID_STORE:
				rcWnd = m_xStoreWnd.GetGameWndRect();
				break;
			case _WND_ID_EXCHANGE:
				rcWnd = m_xExchangeWnd.GetGameWndRect();
				break;
			case _WND_ID_GUILDMASTER:
				rcWnd = m_xGuildMasterWnd.GetGameWndRect();
				break;
			case _WND_ID_GUILD:
				rcWnd = m_xGuildWnd.GetGameWndRect();
				break;
			case _WND_ID_GROUP:
				rcWnd = m_xGroupWnd.GetGameWndRect();
				break;
			case _WND_ID_GROUPPOP:
				rcWnd = m_xGroupPopWnd.GetGameWndRect();
				break;
			case _WND_ID_CHATPOP:
				rcWnd = m_xChatPopWnd.GetGameWndRect();
				break;
			case _WND_ID_NPCCHAT:	// 勇气
				rcWnd = m_xNPCWnd.GetGameWndRect();
				break;

			case _WND_ID_QUEST:
				rcWnd = m_xQuestWnd.GetGameWndRect();
				break;
			case _WND_ID_OPTION:
				rcWnd = m_xOptionWnd.GetGameWndRect();
				break;
            case SiegeLayout::WindowId:rcWnd=m_xSiegeWnd.m_rcWnd;break;
            case ConsignmentLayout::BrowserId:rcWnd=m_xConsignmentWnd.m_rcWnd;break;
            case ConsignmentLayout::RegistrationId:rcWnd=m_xConsignmentRegistration.m_rcWnd;break;
            case _WND_ID_MAIL: rcWnd=m_xMailWnd.m_rcWnd; break;
            case _WND_ID_PORTRAIT: rcWnd=m_xPortraitWnd.m_rcWnd; break;
			case _WND_ID_QUICK:
                rcWnd = m_xQuickWnd.GetGameWndRect();
                break;
			case _WND_ID_HORSE:
				rcWnd = m_xHorseWnd.GetGameWndRect();
				break;

			}

			if ( PtInRect(&rcWnd, ptMouse) )
			{
				return *pnCurrID;
			}

			m_xWndOrderList.MovePreviousNode();
		}
	}
	return -1;
}


/******************************************************************************************************************

	函数名 : CInterface::AddWindowToList()

	作者 :
	编写日期 :

	目的   : 在窗口列表的最后添加当前输入的 ID 为 nID 的窗口。
	输入   : INT nID
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInterface::AddWindowToList(INT nID)
{
	m_xWndOrderList.AddNode(nID);
}


/******************************************************************************************************************

	函数名 : CInterface::DeleteWindowToList()

	作者 :
	编写日期 :

	目的   : 从列表上删除具有 nID 标识的窗口.
	输入   : INT nID
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInterface::DeleteWindowToList(INT nID)
{
	if ( m_xWndOrderList.GetCounter() != 0 )
	{
		m_xWndOrderList.MoveCurrentToTop();

		INT* pnCurrID;
		for ( INT nCnt = 0; nCnt < m_xWndOrderList.GetCounter(); nCnt++ )
		{
			pnCurrID = m_xWndOrderList.GetCurrentData();

			if ( *pnCurrID == nID )
			{
				m_xWndOrderList.DeleteCurrentNode();
				break;
			}

			m_xWndOrderList.MoveNextNode();
		}
	}	
}


/******************************************************************************************************************

	函数名 : CInterface::WindowActivate()

	作者 :
	编写日期 :

	目的   : 将 nID 窗口添加到窗口列表中或从列表中删除,添加时将窗口状态设置为最顶层状态。
			 是窗口接收键盘输入并显示到屏幕时调用的函数。
	输入   : INT nID
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CInterface::WindowActivate(INT nID)
{
	// 为了让新窗口获得焦点，将所有窗口的焦点设为 FALSE。
	BOOL bRet = FALSE;

	AllWindowsFocusOff();

	switch ( nID )
	{
	case _WND_ID_INVENTORY:
		{
			if ( m_xInventoryWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_INVENTORY);
				m_xInventoryWnd.SetGameWndActive(FALSE);
                m_xInventoryWnd.ClearService();
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_INVENTORY);
				m_xInventoryWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	case _WND_ID_SKILL:
		{
			if ( m_xSkillWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_SKILL);
				m_xSkillWnd.SetGameWndActive(FALSE);
                m_xSkillWnd.m_skillPressed=-1; m_xSkillWnd.m_skillScrollCaptured=false; m_xSkillWnd.m_keyMenu={};
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_SKILL);
				m_xSkillWnd.SelectNonempty();
                m_xSkillWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
case _WND_ID_STATUS:
		{
			if ( m_xStatusWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_STATUS);
				m_xStatusWnd.SetGameWndActive(FALSE);
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_STATUS);
				m_xStatusWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	case _WND_ID_STORE:
		{
			if ( m_xStoreWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_STORE);
				m_xStoreWnd.SetGameWndActive(FALSE);
                m_xStoreWnd.CloseOffers(); m_xStoreWnd.Reset();
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_STORE);
				m_xStoreWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	case _WND_ID_EXCHANGE:
		{
			if ( m_xExchangeWnd.GetGameWndActive() )
			{
				m_xExchangeWnd.Close();
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_EXCHANGE);
				m_xExchangeWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	case _WND_ID_QUEST:
		{
			if ( m_xQuestWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_QUEST);
				m_xQuestWnd.SetGameWndActive(FALSE);
                m_xQuestWnd.pressed=-1; m_xQuestWnd.scrollCaptured=false;
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_QUEST);
				m_xQuestWnd.SetGameWndActive(TRUE);
                m_xQuestWnd.Query();
                m_xQuestWnd.pressed=-1; m_xQuestWnd.scrollCaptured=false;
				bRet = TRUE;
			}
		}
		break;
	case _WND_ID_OPTION:
		{
			if ( m_xOptionWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_OPTION);
				m_xOptionWnd.SetGameWndActive(FALSE);
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_OPTION);
				m_xOptionWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
    case SiegeLayout::WindowId:
        if(m_xSiegeWnd.m_bActive) {DeleteWindowToList(nID);m_xSiegeWnd.Close(false);}
        else {AddWindowToList(nID);m_xSiegeWnd.m_bActive=TRUE;m_minimap.Request();bRet=TRUE;}
        break;
    case ConsignmentLayout::BrowserId:case ConsignmentLayout::RegistrationId: {
        auto& panel=nID==ConsignmentLayout::BrowserId?m_xConsignmentWnd:m_xConsignmentRegistration;
        if(panel.m_bActive) {DeleteWindowToList(nID);panel.Close();}
        else {
            if(panel.registration && !m_xInventoryWnd.m_bActive) WindowActivate(_WND_ID_INVENTORY);
            AddWindowToList(nID);panel.m_bActive=TRUE;panel.SyncEditors(true);bRet=TRUE;
        }
        break;
    }
    case _WND_ID_MAIL:
        if(m_xMailWnd.m_bActive) {DeleteWindowToList(nID);m_xMailWnd.Close();}
        else {AddWindowToList(nID);m_xMailWnd.m_bActive=TRUE;m_xMailWnd.SelectView(m_xMailWnd.view);bRet=TRUE;}
        break;
    case _WND_ID_PORTRAIT:
        if (m_xPortraitWnd.m_bActive) { DeleteWindowToList(nID); m_xPortraitWnd.m_bActive=FALSE; m_xPortraitWnd.Close(); }
        else { AddWindowToList(nID); m_xPortraitWnd.m_bActive=TRUE; m_xPortraitWnd.Open(); bRet=TRUE; }
        break;
	case _WND_ID_QUICK:
		{
			if ( m_xQuickWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_QUICK);
				m_xQuickWnd.SetGameWndActive(FALSE); m_xQuickWnd.closePressed=m_xQuickWnd.backgroundPressed=false;
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_QUICK);
				m_xQuickWnd.SetGameWndActive(TRUE); m_xQuickWnd.closePressed=m_xQuickWnd.backgroundPressed=false;
				bRet = TRUE;
			}
		}
		break;

	case _WND_ID_HORSE:
		{
			if ( m_xHorseWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_HORSE);
				m_xHorseWnd.SetGameWndActive(FALSE); m_xHorseWnd.pressed=-1;
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_HORSE);
				m_xHorseWnd.SetGameWndActive(TRUE); m_xHorseWnd.pressed=-1;
				bRet = TRUE;
			}
		}
		break;

	case _WND_ID_GUILDMASTER:
		{
			if ( m_xGuildMasterWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_GUILDMASTER);
				m_xGuildMasterWnd.SetGameWndActive(FALSE);
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_GUILDMASTER);
				m_xGuildMasterWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	case _WND_ID_GUILD:
		{
			if ( m_xGuildWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_GUILD);
				m_xGuildWnd.SetGameWndActive(FALSE);
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_GUILD);
				m_xGuildWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	case _WND_ID_GROUP:
		{
			if ( m_xGroupWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_GROUP);
				m_xGroupWnd.SetGameWndActive(FALSE);
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_GROUP);
				m_xGroupWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	case _WND_ID_GROUPPOP:
		{
			if ( m_xGroupPopWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_GROUPPOP);
				m_xGroupPopWnd.SetGameWndActive(FALSE);
				bRet = FALSE;
			}
			else
			{
				AddWindowToList(_WND_ID_GROUPPOP);
				m_xGroupPopWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	case _WND_ID_CHATPOP:
		{
			if ( m_xChatPopWnd.GetGameWndActive() )
			{
				g_xChatEditBox.MoveScaled( g_xMainWnd.m_rcWindow.left+_INPUT_EDITWND_XPOS,
                           g_xMainWnd.m_rcWindow.top+(m_chatExpanded ? 534 : 575), m_chatExpanded ? 432 : 344, _INPUT_EDITWND_HEIGHT, TRUE);

				DeleteWindowToList(_WND_ID_CHATPOP);
				m_xChatPopWnd.SetGameWndActive(FALSE);
				bRet = FALSE;
			}
			else
			{
				g_xChatEditBox.MoveScaled( g_xMainWnd.m_rcWindow.left + m_xChatPopWnd.m_rcWnd.left + m_xChatPopWnd.m_rcEditBoxFrame.left,
	 					   g_xMainWnd.m_rcWindow.top + m_xChatPopWnd.m_rcWnd.top + m_xChatPopWnd.m_rcEditBoxFrame.top, 
						   m_xChatPopWnd.m_rcEditBoxFrame.right - m_xChatPopWnd.m_rcEditBoxFrame.left, 
						   m_xChatPopWnd.m_rcEditBoxFrame.bottom - m_xChatPopWnd.m_rcEditBoxFrame.top, TRUE);

				SetFocus(g_xChatEditBox.GetSafehWnd());
				AddWindowToList(_WND_ID_CHATPOP);
				m_xChatPopWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	case _WND_ID_NPCCHAT:		// 勇气
		{
			if ( m_xNPCWnd.GetGameWndActive() )
			{
				DeleteWindowToList(_WND_ID_NPCCHAT);
//				m_xNPCWnd.CloseWnd();
				m_xNPCWnd.SetGameWndActive(FALSE); m_xNPCWnd.ResetDialog();
				bRet = FALSE;
			}
			else
			{
				// 临时
				AddWindowToList(_WND_ID_NPCCHAT);
//				m_xNPCWnd.SetNPCStr("仰望天空 直至无一丝羞愧 在掠过叶间的风中 我也曾感到痛苦 <星星/@STAR> 以歌唱的心 去<爱/@LOVE>一切正在死去的事物 然后还要走完被赐予的那条路. 今夜星星依旧被风掠过. 东海之水与白头山枯竭殆尽 愿上天保佑 我国万岁 无穷花三千里 华丽江山 大韩之人 以大韩永世长存. 西门 你可知道 刮紫菜包饭侧边的声音 西门 你可知道 刮开的侧边崩裂的声音 那无比美妙的声音 你当真不知道吗\n <创建行会试试看?/@@buildguildnow>  \n <退出/@end>",0);
				m_xNPCWnd.SetGameWndActive(TRUE);
				bRet = TRUE;
			}
		}
		break;
	}

	return bRet;
}


/******************************************************************************************************************

	函数名 : CInterface::MoveTopWindow()

	作者 :
	编写日期 :

	目的   : 将最上层的窗口移动到鼠标坐标。
	输入   : POINT ptMouse
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInterface::MoveTopWindow(POINT ptMouse)
{
	if ( m_xWndOrderList.GetCounter() != 0 )
	{
		INT* pnCurrID;
		m_xWndOrderList.MoveCurrentToTail();
		pnCurrID = m_xWndOrderList.GetCurrentData();
		if ( m_bMoveFocusedWnd )
		{
			switch ( *pnCurrID )
			{
			case _WND_ID_INVENTORY:
				m_xInventoryWnd.MoveGameWnd(ptMouse);
				break;
			case _WND_ID_SKILL:
				m_xSkillWnd.MoveGameWnd(ptMouse);
				break;
case _WND_ID_STATUS:
				m_xStatusWnd.MoveGameWnd(ptMouse);
				break;
			case _WND_ID_STORE:
				m_xStoreWnd.MoveGameWnd(ptMouse);
				break;
			case _WND_ID_EXCHANGE:
				m_xExchangeWnd.MoveGameWnd(ptMouse);
				break;
			case _WND_ID_GUILDMASTER:
				m_xGuildMasterWnd.MoveGameWnd(ptMouse);
				break;
			case _WND_ID_GUILD:
				m_xGuildWnd.MoveGameWnd(ptMouse);
				break;
			case _WND_ID_GROUP:
				m_xGroupWnd.MoveGameWnd(ptMouse);
				break;
			case _WND_ID_GROUPPOP:
				m_xGroupPopWnd.MoveGameWnd(ptMouse);
				break;
			case _WND_ID_CHATPOP:
				m_xChatPopWnd.MoveGameWnd(ptMouse);
				ShowWindow(g_xChatEditBox.GetSafehWnd(), SW_HIDE);
				break;
			case _WND_ID_NPCCHAT:		// 勇气
				m_xNPCWnd.MoveGameWnd(ptMouse);
				break;

			case _WND_ID_QUEST:
				m_xQuestWnd.MoveGameWnd(ptMouse);
				break;
			case _WND_ID_OPTION:
				m_xOptionWnd.MoveGameWnd(ptMouse);
				break;
			case _WND_ID_QUICK:
                m_xQuickWnd.MoveGameWnd(ptMouse);
                break;
			case _WND_ID_HORSE:
				m_xHorseWnd.MoveGameWnd(ptMouse);
				break;

			}
		}
	}
}



/******************************************************************************************************************

	函数名 : CInterface::WindowFocusChanged()

	作者 :
	编写日期 :

	目的   : 在窗口区域点击鼠标时，将当前的顶层窗口切换为鼠标点击的窗口，并把焦点设置到该窗口.
	输入   : INT nID
	输出   : VOID

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
VOID CInterface::WindowFocusChangedAndSetReadyMove(INT nID, POINT ptMouse)
{
	if ( m_xWndOrderList.GetCounter() != 0 )
	{
		AllWindowsFocusOff();

		DeleteWindowToList(nID);
		AddWindowToList(nID);
		// 如果处于鼠标点击状态,则处于可以移动的准备状态.
		m_bMoveFocusedWnd = TRUE;

		switch ( nID )
		{
		case _WND_ID_INVENTORY:
			m_xInventoryWnd.SetGameWndFocused(TRUE);
			m_xInventoryWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_SKILL:
			m_xSkillWnd.SetGameWndFocused(TRUE);
			m_xSkillWnd.SetReadyMove(ptMouse);
			break;
case _WND_ID_STATUS:
			m_xStatusWnd.SetGameWndFocused(TRUE);
			m_xStatusWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_STORE:
			m_xStoreWnd.SetGameWndFocused(TRUE);
			m_xStoreWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_EXCHANGE:
			m_xExchangeWnd.SetGameWndFocused(TRUE);
			m_xExchangeWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_GUILDMASTER:
			m_xGuildMasterWnd.SetGameWndFocused(TRUE);
			m_xGuildMasterWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_GUILD:
			m_xGuildWnd.SetGameWndFocused(TRUE);
			m_xGuildWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_GROUP:
			m_xGroupWnd.SetGameWndFocused(TRUE);
			m_xGroupWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_GROUPPOP:
			m_xGroupPopWnd.SetGameWndFocused(TRUE);
			m_xGroupPopWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_CHATPOP:
			m_xChatPopWnd.SetGameWndFocused(TRUE);
			m_xChatPopWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_NPCCHAT:		// 勇气
			m_xNPCWnd.SetGameWndFocused(TRUE);
			m_xNPCWnd.SetReadyMove(ptMouse);
			break;

		case _WND_ID_QUEST:
			m_xQuestWnd.SetGameWndFocused(TRUE);
			m_xQuestWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_OPTION:
			m_xOptionWnd.SetGameWndFocused(TRUE);
			m_xOptionWnd.SetReadyMove(ptMouse);
			break;
		case _WND_ID_QUICK:
            m_xQuickWnd.SetGameWndFocused(TRUE); m_xQuickWnd.SetReadyMove(ptMouse); break;
		case _WND_ID_HORSE:
			m_xHorseWnd.SetGameWndFocused(TRUE);
			m_xHorseWnd.SetReadyMove(ptMouse);
			break;

		}
	}	
}


VOID CInterface::AllWindowsFocusOff()
{
	if ( m_xWndOrderList.GetCounter() != 0 )
	{
		m_xWndOrderList.MoveCurrentToTop();

		INT* pnCurrID;
		for ( INT nCnt = 0; nCnt < m_xWndOrderList.GetCounter(); nCnt++ )
		{
			pnCurrID = m_xWndOrderList.GetCurrentData();

			switch ( *pnCurrID )
			{
			case _WND_ID_INVENTORY:
				m_xInventoryWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_SKILL:
				m_xSkillWnd.SetGameWndFocused(FALSE);
				break;
case _WND_ID_STATUS:
				m_xStatusWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_STORE:
				m_xStoreWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_EXCHANGE:
				m_xExchangeWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_GUILDMASTER:
				m_xGuildMasterWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_GUILD:
				m_xGuildWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_GROUP:
				m_xGroupWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_GROUPPOP:
				m_xGroupPopWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_CHATPOP:
				m_xChatPopWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_NPCCHAT:		// 勇气
				m_xNPCWnd.SetGameWndFocused(FALSE);
				break;

			case _WND_ID_QUEST:
				m_xQuestWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_OPTION:
				m_xOptionWnd.SetGameWndFocused(FALSE);
				break;
			case _WND_ID_QUICK:
                m_xQuickWnd.SetGameWndFocused(FALSE);
                break;
			case _WND_ID_HORSE:
				m_xHorseWnd.SetGameWndFocused(FALSE);
				break;

			}
			m_xWndOrderList.MoveNextNode();
		}
	}
}

BOOL CInterface::IsTopWindow(INT nID)
{
	if ( m_xWndOrderList.GetCounter() != 0 )
	{
		INT* pnCurrID;
		m_xWndOrderList.MoveCurrentToTail();
		pnCurrID = m_xWndOrderList.GetCurrentData();

		if ( nID == *pnCurrID )
			return TRUE;
	}
	return FALSE;
}








///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 消息输入相关部分/////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************************************************************************

	函数名 : CInterface::OnLButtonDown()

 	作者 :
	编写日期 :

	目的   : 定义 Mouse LButtonDown 发生时接口中要做的工作.
	输入   : POINT ptMouse
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CInterface::OnLButtonDown(POINT ptMouse)
{
    if(m_xMsgBox.IsActive()) {m_xMsgBox.OnButtonDown(ptMouse);return TRUE;}
    if (m_xGuildEditor.m_bActive) { m_xGuildEditor.Pointer(0,ptMouse); return TRUE; }
    const RECT siegeEntry=m_xSiegeWnd.EntryBounds();
    if(!m_xMsgBox.IsActive() && GetWindowInMousePos(ptMouse)==-1 && PtInRect(&siegeEntry,ptMouse)) {m_xSiegeWnd.entryPressed=true;return TRUE;}
    if (!m_xMsgBox.IsActive() && GetWindowInMousePos(ptMouse)==_WND_ID_MINIMAP && m_minimap.Pointer(0,ptMouse,(GetKeyState(VK_CONTROL)&0x8000)!=0)) {m_bMoveFocusedWnd=FALSE;return TRUE;}
    if (!m_xMsgBox.IsActive() && GetWindowInMousePos(ptMouse)==-1 && PortraitEntryHit(ptMouse)) { m_portraitEntryPressed=true; return TRUE; }
    if (!m_xMsgBox.IsActive() && PtInRect(&m_xSkillWnd.m_keyMenu,ptMouse) && GetWindowInMousePos(ptMouse)==_WND_ID_SKILL) {
        WindowFocusChangedAndSetReadyMove(_WND_ID_SKILL,ptMouse); m_bMoveFocusedWnd=FALSE;
        m_xSkillWnd.OnLButtonDown(ptMouse); return TRUE;
    }
    if (!m_xMsgBox.IsActive() && !PtInRect(&m_xSkillWnd.m_keyMenu,ptMouse)) m_xSkillWnd.m_keyMenu={};
	if(m_xMsgBox.IsActive())	// 勇气
	{
		m_xMsgBox.OnButtonDown(ptMouse);
		return TRUE;
	}
	else
	{
        if (GetWindowInMousePos(ptMouse) == -1 && ptMouse.y >= 570 && ptMouse.y < 590) {
            const int side = ptMouse.x >= 634 && ptMouse.x < 664 ? 0 :
                ptMouse.x >= 717 && ptMouse.x < 747 ? 1 : -1;
            if (side >= 0) {
                m_primaryStat[side] = !m_primaryStat[side];
                return TRUE;
            }
        }
		if ( m_xScrlBar.OnLButtonDown(ptMouse) )
		{
			FLOAT	fScrlRate;

			fScrlRate = m_xScrlBar.GetScrlRate();
			m_nFstLine  = max(0,m_xChat.GetCounter()-1)*fScrlRate;

			return TRUE;
		}

		for( INT nCnt = 0; nCnt < VisibleButtons(); nCnt++ )
		{
			if ( m_xInterBtn[nCnt].OnLButtonDown(ptMouse) )
				return TRUE;
		}

		INT nInterfaceWndInRect;
		nInterfaceWndInRect = GetWindowInMousePos(ptMouse);

		if ( nInterfaceWndInRect != -1 )
		{
			switch ( nInterfaceWndInRect )
			{
			case _WND_ID_INVENTORY:
				if ( !m_xInventoryWnd.OnLButtonDown(&m_stCommonItem, ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;
            case SiegeLayout::WindowId:
                DeleteWindowToList(SiegeLayout::WindowId);AddWindowToList(SiegeLayout::WindowId);m_bMoveFocusedWnd=FALSE;m_xSiegeWnd.Pointer(0,ptMouse);break;
            case ConsignmentLayout::BrowserId:case ConsignmentLayout::RegistrationId: {
                auto& panel=nInterfaceWndInRect==ConsignmentLayout::BrowserId?m_xConsignmentWnd:m_xConsignmentRegistration;
                DeleteWindowToList(nInterfaceWndInRect);AddWindowToList(nInterfaceWndInRect);m_bMoveFocusedWnd=FALSE;panel.Pointer(0,ptMouse);break;
            }
            case _WND_ID_MAIL:
                DeleteWindowToList(_WND_ID_MAIL);AddWindowToList(_WND_ID_MAIL);m_bMoveFocusedWnd=FALSE;m_xMailWnd.Pointer(0,ptMouse);break;
            case _WND_ID_PORTRAIT:
                DeleteWindowToList(_WND_ID_PORTRAIT); AddWindowToList(_WND_ID_PORTRAIT); m_bMoveFocusedWnd=FALSE; m_xPortraitWnd.Pointer(0,ptMouse); break;
			case _WND_ID_SKILL:
                WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect,ptMouse);
                if (m_xSkillWnd.OnLButtonDown(ptMouse)) m_bMoveFocusedWnd=FALSE;
                break;
case _WND_ID_STATUS:
				if ( !m_xStatusWnd.OnLButtonDown(&m_stCommonItem, ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;
			case _WND_ID_STORE:
				if ( !m_xStoreWnd.OnLButtonDown(ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;
			case _WND_ID_EXCHANGE:
                WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect,ptMouse);
                if (m_xExchangeWnd.OnLButtonDown(ptMouse)) m_bMoveFocusedWnd=FALSE;
                break;
			case _WND_ID_GUILDMASTER:
				if ( !m_xGuildMasterWnd.OnLButtonDown(ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;
			case _WND_ID_GUILD:
				if ( !m_xGuildWnd.OnLButtonDown(ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;
			case _WND_ID_GROUP:
				if ( !m_xGroupWnd.OnLButtonDown(ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;
			case _WND_ID_GROUPPOP:
				if ( !m_xGroupPopWnd.OnLButtonDown(ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;
			case _WND_ID_CHATPOP:
				if ( !m_xChatPopWnd.OnLButtonDown(ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;
			case _WND_ID_NPCCHAT:	// 勇气
				if ( !m_xNPCWnd.OnLButtonDown(ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
				break;


			case _WND_ID_QUEST:
                WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect,ptMouse);
                if (m_xQuestWnd.OnLButtonDown(ptMouse)) m_bMoveFocusedWnd=FALSE;
                break;
			case _WND_ID_OPTION:
                if (!m_xOptionWnd.OnLButtonDown(ptMouse))
                    WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect,ptMouse);
                else m_bMoveFocusedWnd=FALSE;
                break;
			case _WND_ID_QUICK:
                WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect,ptMouse);
                if (m_xQuickWnd.OnLButtonDown(ptMouse)) m_bMoveFocusedWnd=FALSE;
                break;
			case _WND_ID_HORSE:
                WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect,ptMouse);
                if (m_xHorseWnd.OnLButtonDown(ptMouse)) m_bMoveFocusedWnd=FALSE;
                break;

			}
			return TRUE;
		}
		else
			AllWindowsFocusOff();
	}
	return FALSE;
}



/******************************************************************************************************************

	函数名 : CInterface::OnLButtonUp()

	作者   :
	编写日期 :

	目的   : 定义鼠标 LButtonUp 发生时接口要做的处理。
	输入   : POINT ptMouse
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
void CInterface::OpenExitDialog(int action)
{
    if(g_bProcState!=_GAME_PROC || g_xGameProc.m_exitTransition.active || !g_xGameProc.CheckExitAllowed(GetTickCount())) return;
    static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
    m_nIdxMsgBox=action;
    m_xMsgBox.SetOrigin(220,112,false);
    m_xMsgBox.ShowMessageBox(captions.Get(action==ExitFlow::ProgramDialog?ExitFlow::ProgramCaption:ExitFlow::CharacterCaption).c_str(),MSG_BTN_YESNO,1240);
    ShowWindow(g_xChatEditBox.GetSafehWnd(),SW_HIDE);SetFocus(g_xMainWnd.GetSafehWnd());
    m_bMoveFocusedWnd=FALSE;
}

void CInterface::ShowNotice(int message)
{
    static CLoginMessages captions;
    static const bool loaded=captions.Load("./CmList.dat");
    m_nIdxMsgBox=0;
    m_xMsgBox.SetOrigin(220,112,false);
    m_xMsgBox.ShowMessageBox(captions.Get(message).c_str(),MSG_BTN_OK,1240);
    if(m_xGuildEditor.m_bActive) {ShowWindow(m_xGuildEditor.edit,SW_HIDE);SetFocus(g_xMainWnd.GetSafehWnd());}
}

void CInterface::OpenNameDialog(int action)
{
    static CLoginMessages captions;
    static const bool loaded = captions.Load("./CmList.dat");
    m_nIdxMsgBox = action;
    m_xMsgBox.SetOrigin(220,112,true);
    std::string prompt = action==4 ? "'"+m_xGuildWnd.m_name+"' "+captions.Get(4001) :
        captions.Get(action==6?1304:action==5?4002:action==3?1606:1605);
    if(action==TradeWire::QuantityDialog) prompt=captions.Get(TradeWire::QuantityDialog);
    if(action==SiegeLayout::LoadDialog) prompt=captions.Get(SiegeLayout::LoadCaption);
    if(action==ConsignmentLayout::PriceDialog) prompt=captions.Get(ConsignmentLayout::PriceQuestion);
    if (action==MailLayout::GoldDialogAction) prompt=captions.Get(MailLayout::GoldQuestion);
    if (action==7) prompt=m_xNPCWnd.prompt;
    if (action==8) prompt=captions.Get(2302);
    if (action==9) prompt=captions.Get(2301);
    if (action==10 || action==11) prompt=captions.Get(action==10?1705:1706);
    m_xMsgBox.ShowMessageBox(prompt.c_str(),MSG_BTN_YESNO,1240);
    g_xChatEditBox.MoveScaled(g_xMainWnd.m_rcWindow.left+244,g_xMainWnd.m_rcWindow.top+340,309,15,TRUE);
    g_xChatEditBox.SetLimitText(action==7?m_xNPCWnd.PromptLimit():90);
    SendMessage(g_xChatEditBox.GetSafehWnd(),EM_SETPASSWORDCHAR,action==7 && m_xNPCWnd.promptCommand.compare(0,3,"@@*")==0?'*':0,0);
    g_xChatEditBox.m_szInputMsg[0]=0;
    SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"");
    ShowWindow(g_xChatEditBox.GetSafehWnd(),SW_SHOW);
    SetFocus(g_xChatEditBox.GetSafehWnd());
}

void CInterface::FinishNotice(int hResult, bool mouse)
{
    if (!hResult) return;
    if(m_nIdxMsgBox==ExitFlow::ProgramDialog || m_nIdxMsgBox==ExitFlow::CharacterDialog) {
        const int action=m_nIdxMsgBox;m_nIdxMsgBox=0;
        if(mouse) g_xSound.PlayLobbyEffect("105.wav");
        m_xMsgBox.HideMessageBox();
        if(hResult==CLICK_YES) g_xGameProc.ConfirmExit(action,GetTickCount());
        return;
    }
    if (m_nIdxMsgBox==0) {
        if (mouse) g_xSound.PlayLobbyEffect("105.wav");
        m_xMsgBox.HideMessageBox();
        return;
    }
    if(m_nIdxMsgBox==ConsignmentLayout::PurchaseDialog || m_nIdxMsgBox==ConsignmentLayout::CancelDialog || m_nIdxMsgBox==ConsignmentLayout::RegistrationDialog) {
        if(mouse) g_xSound.PlayLobbyEffect("105.wav");
        m_xMsgBox.HideMessageBox();
        (m_nIdxMsgBox==ConsignmentLayout::RegistrationDialog?m_xConsignmentRegistration:m_xConsignmentWnd).Confirm(hResult==CLICK_YES);
        return;
    }
    if ((m_nIdxMsgBox >= 1 && m_nIdxMsgBox <= 11) || m_nIdxMsgBox==TradeWire::QuantityDialog || m_nIdxMsgBox==MailLayout::GoldDialogAction || m_nIdxMsgBox==ConsignmentLayout::PriceDialog || m_nIdxMsgBox==SiegeLayout::LoadDialog) {
        std::vector<char> answer(GetWindowTextLengthA(g_xChatEditBox.GetSafehWnd())+1);
        char* name=answer.data(); GetWindowTextA(g_xChatEditBox.GetSafehWnd(),name,int(answer.size()));
        if (mouse) g_xSound.PlayLobbyEffect("105.wav");
        m_xMsgBox.HideMessageBox();
        SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"");
        g_xChatEditBox.m_szInputMsg[0]=0;
        ShowWindow(g_xChatEditBox.GetSafehWnd(),SW_HIDE);
        SendMessage(g_xChatEditBox.GetSafehWnd(),EM_SETPASSWORDCHAR,0,0);
        g_xChatEditBox.SetLimitText(90);
        SetFocus(g_xMainWnd.GetSafehWnd());
        g_xChatEditBox.MoveScaled(g_xMainWnd.m_rcWindow.left+185,
            g_xMainWnd.m_rcWindow.top+(m_chatExpanded?534:575),m_chatExpanded?432:344,15,TRUE);
        if(m_nIdxMsgBox==MailLayout::GoldDialogAction && hResult!=CLICK_YES) m_xMailWnd.giftGoldConfirmed=false;
        if (m_nIdxMsgBox==7 && hResult!=CLICK_YES) m_xNPCWnd.ClearPrompt();
        if(m_nIdxMsgBox==TradeWire::QuantityDialog && hResult!=CLICK_YES) m_xExchangeWnd.SubmitOffer(name,false);
        if (hResult == CLICK_YES) {
            if(m_nIdxMsgBox==TradeWire::QuantityDialog) m_xExchangeWnd.SubmitOffer(name,true);
            else if(m_nIdxMsgBox==SiegeLayout::LoadDialog) m_xSiegeWnd.SubmitLoad(name);
            else if(m_nIdxMsgBox==ConsignmentLayout::PriceDialog) m_xConsignmentRegistration.SubmitPrice(name);
            else if(m_nIdxMsgBox==MailLayout::GoldDialogAction) m_xMailWnd.SubmitGold(name);
            else if (m_nIdxMsgBox == 1) g_xClientSocket.SendCreateGroup(name);
            else if (m_nIdxMsgBox == 2) g_xClientSocket.SendAddGroupMember(name);
            else if (m_nIdxMsgBox == 3) g_xClientSocket.SendDelGroupMember(name);
            else if (m_nIdxMsgBox == 4) g_xClientSocket.SendGuildMemberRemoval(name);
            else if (m_nIdxMsgBox == 7) m_xNPCWnd.SubmitPrompt(name);
            else if (m_nIdxMsgBox == 8) m_xStoreWnd.BuyBulk(name);
            else if (m_nIdxMsgBox == 9) m_xStoreWnd.SubmitWithdrawal(name);
            else if (m_nIdxMsgBox == 10 || m_nIdxMsgBox == 11) m_xInventoryWnd.SubmitServiceQuantity(name);
            else if (m_nIdxMsgBox == 6) m_xExchangeWnd.SubmitGold(name,g_xGameProc.m_xMyHero.m_nGlod);
            else {
                static CLoginMessages captions;
                static const bool loaded=captions.Load("./CmList.dat");
                std::string command=captions.Get(4016)+name;
                g_xClientSocket.SendSay(const_cast<char*>(command.c_str()));
                g_xClientSocket.SendGuildHome();
            }
        }
        return;
    }

}

BOOL CInterface::OnLButtonUp(POINT ptMouse)
{
    if(m_xMsgBox.IsActive()) {
        m_bMoveFocusedWnd=FALSE;
        FinishNotice(m_xMsgBox.OnButtonUp(ptMouse),true);return TRUE;
    }
    if(!m_xMsgBox.IsActive() && m_bMoveFocusedWnd && m_xOptionWnd.m_bActive && m_xOptionWnd.m_bFocused) {
        m_bMoveFocusedWnd=FALSE;m_xOptionWnd.SetStatusBtnInit();return TRUE;
    }
    m_portraitEntryPressed=false;
    if(m_minimap.releaseCaptured) {m_minimap.Pointer(2,ptMouse,false);return TRUE;}
    if (!m_xMsgBox.IsActive() && !m_xGuildEditor.m_bActive) {
        if (m_xInventoryWnd.ClosePressed()) { m_bMoveFocusedWnd=FALSE; if(m_xInventoryWnd.OnLButtonUp(&m_stCommonItem,ptMouse)) WindowActivate(_WND_ID_INVENTORY); return TRUE; }
        if (m_xStatusWnd.ClosePressed()) { m_bMoveFocusedWnd=FALSE; if(m_xStatusWnd.OnLButtonUp(&m_stCommonItem,ptMouse)) WindowActivate(_WND_ID_STATUS); return TRUE; }
    }
    if (!m_xMsgBox.IsActive() && (m_xPortraitWnd.scrolling || m_xPortraitWnd.pressed>=0)) { if (m_xPortraitWnd.Pointer(2,ptMouse)) WindowActivate(_WND_ID_PORTRAIT); return TRUE; }
    if (!m_xMsgBox.IsActive() && m_xInventoryWnd.service.pressed) { m_xInventoryWnd.OnLButtonUp(&m_stCommonItem,ptMouse); return TRUE; }
    if (!m_xMsgBox.IsActive() && (m_xStoreWnd.pressed!=-1 || m_xStoreWnd.scrollCaptured)) {
        if (m_xStoreWnd.OnLButtonUp(ptMouse)) WindowActivate(_WND_ID_STORE);
        return TRUE;
    }
    if (m_xNPCWnd.closePressed || m_xNPCWnd.arrowPressed) { m_xNPCWnd.OnLButtonUp(ptMouse); return TRUE; }
    if (m_xGuildEditor.m_bActive) { m_xGuildEditor.Pointer(2,ptMouse); return TRUE; }
    if(!m_xMsgBox.IsActive() && m_xSiegeWnd.entryPressed) {
        m_xSiegeWnd.entryPressed=false;const RECT area=m_xSiegeWnd.EntryBounds();
        if(GetWindowInMousePos(ptMouse)==-1 && PtInRect(&area,ptMouse)) {g_xSound.PlayLobbyEffect("105.wav");WindowActivate(SiegeLayout::WindowId);}
        return TRUE;
    }
    if(!m_xMsgBox.IsActive() && m_xSiegeWnd.captured) {m_xSiegeWnd.Pointer(2,ptMouse);if(!m_xSiegeWnd.m_bActive) DeleteWindowToList(SiegeLayout::WindowId);return TRUE;}
    if(!m_xMsgBox.IsActive() && !m_xGuildEditor.m_bActive) for(auto* panel:{&m_xConsignmentWnd,&m_xConsignmentRegistration}) if(panel->captured) {
        panel->Pointer(2,ptMouse);if(!panel->m_bActive) DeleteWindowToList(panel->m_nID);return TRUE;
    }
    if(!m_xMsgBox.IsActive() && m_xMailWnd.captured) {m_xMailWnd.Pointer(2,ptMouse);if(!m_xMailWnd.m_bActive) DeleteWindowToList(_WND_ID_MAIL);return TRUE;}
    if (m_xExchangeWnd.captured>=0 || m_xExchangeWnd.confirmPressed || m_xExchangeWnd.headerPressed) { m_bMoveFocusedWnd=FALSE; m_xExchangeWnd.OnLButtonUp(ptMouse); return TRUE; }
    if (m_xQuickWnd.closePressed || m_xQuickWnd.backgroundPressed) {
        m_bMoveFocusedWnd=FALSE;
        if (m_xQuickWnd.OnLButtonUp(ptMouse)) WindowActivate(_WND_ID_QUICK);
        return TRUE;
    }
    if (m_xHorseWnd.pressed>=0) {
        if (m_xHorseWnd.OnLButtonUp(ptMouse)) WindowActivate(_WND_ID_HORSE);
        return TRUE;
    }
    if (m_xQuestWnd.scrollCaptured || m_xQuestWnd.pressed>=0) {
        if (m_xQuestWnd.OnLButtonUp(ptMouse)) WindowActivate(_WND_ID_QUEST);
        return TRUE;
    }
    if (!m_xMsgBox.IsActive() && PtInRect(&m_xSkillWnd.m_keyMenu,ptMouse) && GetWindowInMousePos(ptMouse)==_WND_ID_SKILL) {
        m_xSkillWnd.OnLButtonUp(ptMouse); return TRUE;
    }
    if (m_xSkillWnd.m_skillScrollCaptured || m_xSkillWnd.m_skillPressed>=0) {
        if (m_xSkillWnd.OnLButtonUp(ptMouse)) WindowActivate(_WND_ID_SKILL);
        return TRUE;
    }
    if (m_xInventoryWnd.m_scrollCaptured) { m_xInventoryWnd.OnLButtonUp(&m_stCommonItem,ptMouse); return TRUE; }
    if (m_xOptionWnd.m_audioDrag>=0) { m_xOptionWnd.OnLButtonUp(ptMouse); return TRUE; }
    if (m_xGroupWnd.PressedButton()>=0) { if(m_xGroupWnd.OnLButtonUp(ptMouse)) WindowActivate(_WND_ID_GROUP); return TRUE; }
    if (m_xGuildWnd.m_pressed>=0) { if(m_xGuildWnd.OnLButtonUp(ptMouse)) WindowActivate(_WND_ID_GUILD); return TRUE; }
    if (m_xGuildWnd.m_scrollCaptured) { m_xGuildWnd.OnLButtonUp(ptMouse); return TRUE; }
	m_bMoveFocusedWnd = FALSE;

	if ( m_xMsgBox.IsActive() )	// 勇气
	{
		HRESULT hResult;
		hResult = m_xMsgBox.OnButtonUp(ptMouse);
        FinishNotice(hResult,true);
	}
	else
	{
		if ( m_xScrlBar.OnLButtonUp(ptMouse) )
		{
			return TRUE;
		}

        if (GetWindowInMousePos(ptMouse)==-1 && PortraitEntryHit(ptMouse)) { WindowActivate(_WND_ID_PORTRAIT); g_xSound.PlayLobbyEffect("105.wav"); return TRUE; }
        const RECT portraitArea={535,486,616,580};
        if(!m_chatExpanded && GetWindowInMousePos(ptMouse)==-1 && PtInRect(&portraitArea,ptMouse)) {m_minimap.TogglePortrait(GetTickCount());return TRUE;}
		for( INT nCnt = 0; nCnt < VisibleButtons(); nCnt++ )
		{
			if ( m_xInterBtn[nCnt].OnLButtonUp(ptMouse) )
			{
                g_xSound.PlayLobbyEffect("105.wav");
				switch ( nCnt )
				{
                case _BTN_ID_MINIMAP: m_minimap.Toggle(GetTickCount(),100); break;
                case 20:
                case 21:
                case 22:
                    {
                        const int channel = nCnt-20;
                        // The server owns rejection state; a click only predicts the overlay.
                        m_chatReceptionHint[channel]=(channel==0?2637:channel==1?2639:2643)+(m_chatRejected[channel]?0:1);
                        m_chatReceptionFrame[channel] = m_chatRejected[channel] ? -1 : 1338+channel*2;
                        static CLoginMessages captions;
                        static const bool loaded = captions.Load(".\\CmList.dat");
                        const int commandId = channel == 0 ? 1070 : channel == 1 ? 1071 : 1073;
                        std::string command = captions.Get(commandId);
                        g_xClientSocket.SendSay(const_cast<char*>(command.c_str()));
                    }
                    break;
                case 18:
                case 19:
                    {
                        const int requested = nCnt == 18 ? 1 : 2;
                        m_chatMode = m_chatMode == requested ? 0 : requested;
                        m_chatModeChanged = true;
                        m_xInterBtn[18].ResetGameBtn(m_chatMode == 1 ? 1335 : 1334,m_chatMode == 1 ? 1335 : 1334);
                        m_xInterBtn[19].ResetGameBtn(m_chatMode == 2 ? 1337 : 1336,m_chatMode == 2 ? 1337 : 1336);
                        static CLoginMessages captions;
                        static const bool loaded = captions.Load(".\\CmList.dat");
                        std::string message = captions.Get(m_chatMode == 0 ? 2626 : (m_chatMode == 1 ? 2633 : 2635));
                        MsgAdd(0xffffff,0x80ff,const_cast<char*>(message.c_str()));
                    }
                    break;
                case 16:
                case 17:
                    {
                        static CLoginMessages captions;
                        static const bool loaded = captions.Load(".\\CmList.dat");
                        const char* prefix = nCnt == 16 ? captions.Get(1072).c_str() : "!";
                        g_xChatEditBox.MoveScaled(g_xMainWnd.m_rcWindow.left+185,
                            g_xMainWnd.m_rcWindow.top+534,432,15,TRUE);
                        SetWindowTextA(g_xChatEditBox.GetSafehWnd(),prefix);
                        ShowWindow(g_xChatEditBox.GetSafehWnd(),SW_SHOW);
                        SetFocus(g_xChatEditBox.GetSafehWnd());
                        SendMessage(g_xChatEditBox.GetSafehWnd(),EM_SETSEL,strlen(prefix),strlen(prefix));
                    }
                    break;

				case _BTN_ID_QUEST:		WindowActivate(_WND_ID_QUEST);					break;
				case _BTN_ID_OPTION:	WindowActivate(_WND_ID_OPTION);					break;
				case _BTN_ID_HORSE:		WindowActivate(_WND_ID_HORSE);					break;

				case _BTN_ID_CHANGE: m_xExchangeWnd.RequestTrade(); break;
				case _BTN_ID_GUILD:		
					g_xClientSocket.SendGuildHome();
					break;
				case _BTN_ID_GROUP:		WindowActivate(_WND_ID_GROUP);					break;
				case _BTN_ID_POPUP: break; // MIR3 HUD action 10 has no window action.
                case _BTN_ID_HELP: WindowActivate(_WND_ID_QUICK); break;
				case _BTN_ID_BAG:		WindowActivate(_WND_ID_INVENTORY);				break;
				case _BTN_ID_MUGONG:	m_bShowKey = !m_bShowKey;						break;
				case _BTN_ID_BELT:
                    m_chatExpanded = !m_chatExpanded;
                    LayoutChat();
                    break;
                case _BTN_ID_EXIT: OpenExitDialog(ExitFlow::ProgramDialog);break;
                case _BTN_ID_LOGOUT: OpenExitDialog(ExitFlow::CharacterDialog);break;
				case _BTN_ID_MAGIC:
                    WindowActivate(_WND_ID_SKILL);
                    break;
				case _BTN_ID_CHAR:
                    WindowActivate(_WND_ID_STATUS);
                    break;
				}
			}
		}

		INT nInterfaceWndInRect;
		nInterfaceWndInRect = GetWindowInMousePos(ptMouse);

		// 各窗口的 OnLButtonUp 返回值为 TRUE 时, 窗口将被 Close。
		if ( nInterfaceWndInRect != -1 )
		{
			switch ( nInterfaceWndInRect )
			{
			case _WND_ID_INVENTORY:		if ( m_xInventoryWnd.OnLButtonUp(&m_stCommonItem, ptMouse) )	WindowActivate(_WND_ID_INVENTORY);		break;
            case _WND_ID_PORTRAIT: if (m_xPortraitWnd.Pointer(2,ptMouse)) WindowActivate(_WND_ID_PORTRAIT); break;
			case _WND_ID_SKILL:		if ( m_xSkillWnd.OnLButtonUp(ptMouse) )		WindowActivate(_WND_ID_SKILL);			break;
case _WND_ID_STATUS:		if ( m_xStatusWnd.OnLButtonUp(&m_stCommonItem, ptMouse) )		WindowActivate(_WND_ID_STATUS);			break;
			case _WND_ID_STORE:			if ( m_xStoreWnd.OnLButtonUp(ptMouse) )							WindowActivate(_WND_ID_STORE);			break;
			case _WND_ID_EXCHANGE:		if ( m_xExchangeWnd.OnLButtonUp(ptMouse) )						WindowActivate(_WND_ID_EXCHANGE);		break;
			case _WND_ID_GUILDMASTER:
			{
				if ( m_xGuildMasterWnd.OnLButtonUp(ptMouse) )
					WindowActivate(_WND_ID_GUILDMASTER);
				break;
			}
			case _WND_ID_GUILD:			if ( m_xGuildWnd.OnLButtonUp(ptMouse) )							WindowActivate(_WND_ID_GUILD);			break;
			case _WND_ID_GROUP:			if ( m_xGroupWnd.OnLButtonUp(ptMouse) )							WindowActivate(_WND_ID_GROUP);			break;
			case _WND_ID_GROUPPOP:		if ( m_xGroupPopWnd.OnLButtonUp(ptMouse) )						WindowActivate(_WND_ID_GROUPPOP);		break;
			case _WND_ID_CHATPOP:		if ( m_xChatPopWnd.OnLButtonUp(ptMouse) )						WindowActivate(_WND_ID_CHATPOP);		break;

			case _WND_ID_QUEST:			if ( m_xQuestWnd.OnLButtonUp(ptMouse) )							WindowActivate(_WND_ID_QUEST);			break;
			case _WND_ID_OPTION:		if ( m_xOptionWnd.OnLButtonUp(ptMouse) )						WindowActivate(_WND_ID_OPTION);			break;
            case _WND_ID_QUICK: if (m_xQuickWnd.OnLButtonUp(ptMouse)) WindowActivate(_WND_ID_QUICK); break;
			case _WND_ID_HORSE:			if ( m_xHorseWnd.OnLButtonUp(ptMouse) )							WindowActivate(_WND_ID_HORSE);			break;

			case _WND_ID_NPCCHAT:		if ( m_xNPCWnd.OnLButtonUp(ptMouse) )							WindowActivate(_WND_ID_NPCCHAT);		break; // 勇气
			}
			return TRUE;
		}
	}
	return FALSE;
}


/******************************************************************************************************************

	函数名 : CInterface::OnMouseMove()

	作者 :
	编写日期 :

	目的   : 当 Mouse Move 发生时, 定义界面中要做的事.
	输入   : POINT ptMouse
	输出   : BOOL

	[日期][修改者] : 修改内容

*******************************************************************************************************************/
BOOL CInterface::OnMouseMove(POINT ptMouse)
{
    if(m_xMsgBox.IsActive()) return TRUE;
    if(!m_xMsgBox.IsActive() && !m_xGuildEditor.m_bActive && GetWindowInMousePos(ptMouse)==_WND_ID_MINIMAP && m_minimap.Pointer(1,ptMouse,(GetKeyState(VK_CONTROL)&0x8000)!=0)) return TRUE;
    if (!m_xMsgBox.IsActive() && m_xPortraitWnd.scrolling) { m_xPortraitWnd.Pointer(1,ptMouse); return TRUE; }
    if (!m_xMsgBox.IsActive() && m_xStoreWnd.scrollCaptured) { m_xStoreWnd.OnMouseMove(ptMouse); return TRUE; }
    if (m_xGuildEditor.m_bActive) { m_xGuildEditor.Pointer(1,ptMouse); return TRUE; }
    if(!m_xMsgBox.IsActive() && m_xSiegeWnd.captured) {m_xSiegeWnd.Pointer(1,ptMouse);return TRUE;}
    if(!m_xMsgBox.IsActive() && !m_xGuildEditor.m_bActive) for(auto* panel:{&m_xConsignmentWnd,&m_xConsignmentRegistration}) if(panel->captured) {panel->Pointer(1,ptMouse);return TRUE;}
    if(!m_xMsgBox.IsActive() && m_xMailWnd.captured) {m_xMailWnd.Pointer(1,ptMouse);return TRUE;}
    if (m_xExchangeWnd.captured>=0) { m_xExchangeWnd.OnMouseMove(ptMouse); return TRUE; }
    if (m_xQuestWnd.scrollCaptured) { m_xQuestWnd.OnMouseMove(ptMouse); return TRUE; }
    if (m_xSkillWnd.m_skillScrollCaptured) { m_xSkillWnd.OnMouseMove(ptMouse); return TRUE; }
    if (m_xInventoryWnd.m_scrollCaptured) { m_xInventoryWnd.OnMouseMove(ptMouse); return TRUE; }
    if (m_xOptionWnd.m_audioDrag>=0) { m_xOptionWnd.OnMouseMove(ptMouse); return TRUE; }
    if (m_xGroupWnd.PressedButton()>=0) return TRUE;
    if (m_xGuildWnd.m_pressed>=0) return TRUE;
    if (m_xGuildWnd.m_scrollCaptured) { m_xGuildWnd.OnMouseMove(ptMouse); return TRUE; }
    if (m_xMsgBox.IsActive()) return TRUE;
	MoveTopWindow(ptMouse);

	if ( m_xScrlBar.OnMouseMove(ptMouse) )
	{
		FLOAT	fScrlRate;

		fScrlRate = m_xScrlBar.GetScrlRate();
		m_nFstLine  = max(0,m_xChat.GetCounter()-1)*fScrlRate;
		return TRUE;
	}

	for( INT nCnt = 0; nCnt < VisibleButtons(); nCnt++ )
		m_xInterBtn[nCnt].OnMouseMove(ptMouse);

	INT nInterfaceWndInRect;
	nInterfaceWndInRect = GetWindowInMousePos(ptMouse);

	if ( nInterfaceWndInRect != -1 )
	{
		switch ( nInterfaceWndInRect )
		{
		case _WND_ID_INVENTORY:
			m_xInventoryWnd.OnMouseMove(ptMouse);
			break;
        case ConsignmentLayout::BrowserId:m_xConsignmentWnd.Pointer(1,ptMouse);break;
        case ConsignmentLayout::RegistrationId:m_xConsignmentRegistration.Pointer(1,ptMouse);break;
        case _WND_ID_MAIL: m_xMailWnd.Pointer(1,ptMouse);break;
        case _WND_ID_PORTRAIT: m_xPortraitWnd.Pointer(1,ptMouse); break;
		case _WND_ID_SKILL:
			m_xSkillWnd.OnMouseMove(ptMouse);
			break;
		case _WND_ID_STORE:
			m_xStoreWnd.OnMouseMove(ptMouse);
			break;
		case _WND_ID_EXCHANGE:
			m_xExchangeWnd.OnMouseMove(ptMouse);
			break;
		case _WND_ID_GUILDMASTER:
			m_xGuildMasterWnd.OnMouseMove(ptMouse);
			break;
		case _WND_ID_GUILD:
			m_xGuildWnd.OnMouseMove(ptMouse);
			break;
		case _WND_ID_GROUP:
			m_xGroupWnd.OnMouseMove(ptMouse);
			break;
		case _WND_ID_GROUPPOP:
			m_xGroupPopWnd.OnMouseMove(ptMouse);
			break;
		case _WND_ID_CHATPOP:
			m_xChatPopWnd.OnMouseMove(ptMouse);
			break;

		case _WND_ID_QUEST:
			m_xQuestWnd.OnMouseMove(ptMouse);
			break;
		case _WND_ID_OPTION:
			m_xOptionWnd.OnMouseMove(ptMouse);
			break;

		case _WND_ID_NPCCHAT:		// 勇气
			m_xNPCWnd.OnMouseMove(ptMouse);
			break;
		}
		return TRUE;
	}
	else
	{
		m_xInventoryWnd.SetInvenBtnInit();
		m_xChatPopWnd.SetStatusBtnInit();
		m_xGuildMasterWnd.SetStatusBtnInit();
		m_xOptionWnd.SetStatusBtnInit();
	}

	return FALSE;
}


BOOL CInterface::OnScrollDown()
{
    if (m_xGuildEditor.m_bActive) {  return TRUE; }
    if (m_xMsgBox.IsActive()) return TRUE;
    const int hovered=GetWindowInMousePos(g_xGameProc.m_ptMousePos);
    if (hovered==_WND_ID_MINIMAP) return TRUE;
    if (hovered==_WND_ID_MAIL) {m_xMailWnd.Wheel(-1);return TRUE;}
    if (hovered==_WND_ID_PORTRAIT) { m_xPortraitWnd.Wheel(1); return TRUE; }
    if (hovered==_WND_ID_STORE) { m_xStoreWnd.OnScrollDown(); return TRUE; }
    if (hovered==_WND_ID_NPCCHAT) { m_xNPCWnd.OnScrollDown(); m_xNPCWnd.pressed=-1; m_xNPCWnd.arrowPressed=0; return TRUE; }
    if (IsTopWindow(_WND_ID_QUEST)) { m_xQuestWnd.OnScrollDown(); return TRUE; }
    if (IsTopWindow(_WND_ID_GUILD)) { m_xGuildWnd.OnScrollDown(); return TRUE; }
	if ( IsTopWindow(m_xInventoryWnd.m_nID) )
	{
		m_xInventoryWnd.OnScrollDown();
	}
	else if ( IsTopWindow(m_xSkillWnd.m_nID) )
	{
		m_xSkillWnd.OnScrollDown();
	}
	else
	{
		if ( !m_xChatPopWnd.m_bActive )
		{
			if ( m_nFstLine > 0 )	
			{
				m_nFstLine--;
			}
		}
		else
		{
			m_xChatPopWnd.OnScrollDown();
		}
	}
	return TRUE;
}


BOOL CInterface::OnScrollUp()
{
    if (m_xGuildEditor.m_bActive) {  return TRUE; }
    if (m_xMsgBox.IsActive()) return TRUE;
    const int hovered=GetWindowInMousePos(g_xGameProc.m_ptMousePos);
    if (hovered==_WND_ID_MINIMAP) return TRUE;
    if (hovered==_WND_ID_MAIL) {m_xMailWnd.Wheel(1);return TRUE;}
    if (hovered==_WND_ID_PORTRAIT) { m_xPortraitWnd.Wheel(-1); return TRUE; }
    if (hovered==_WND_ID_STORE) { m_xStoreWnd.OnScrollUp(); return TRUE; }
    if (hovered==_WND_ID_NPCCHAT) { m_xNPCWnd.OnScrollUp(); m_xNPCWnd.pressed=-1; m_xNPCWnd.arrowPressed=0; return TRUE; }
    if (IsTopWindow(_WND_ID_QUEST)) { m_xQuestWnd.OnScrollUp(); return TRUE; }
    if (IsTopWindow(_WND_ID_GUILD)) { m_xGuildWnd.OnScrollUp(); return TRUE; }
	if ( IsTopWindow(m_xInventoryWnd.m_nID) )
	{
		m_xInventoryWnd.OnScrollUp();
	}
	else if ( IsTopWindow(m_xSkillWnd.m_nID) )
	{
		m_xSkillWnd.OnScrollUp();
	}
	else
	{
		if ( !m_xChatPopWnd.m_bActive )
		{
			if ( m_nFstLine < max(0, m_xChat.GetCounter()-1) )
			{
				m_nFstLine++;
			}
		}
		else
		{
			m_xChatPopWnd.OnScrollUp();
		}
	}
	return TRUE;
}

BOOL CInterface::OnLButtonDoubleClick(POINT ptMouse)
{
    if(GetWindowInMousePos(ptMouse)==_WND_ID_MINIMAP) return TRUE;
    if (m_xGuildEditor.m_bActive) {  return TRUE; }
    if (m_xMsgBox.IsActive()) return TRUE;
    if (GetWindowInMousePos(ptMouse)==_WND_ID_MAIL) {
        const int row=m_xMailWnd.RowAt(ptMouse);
        if(row>=0 && ptMouse.x>=m_xMailWnd.m_rcWnd.left+MailLayout::SubjectHit.left && ptMouse.x<m_xMailWnd.m_rcWnd.left+MailLayout::SubjectHit.right) {
            (m_xMailWnd.view==MailLayout::View::Outbox?m_xMailWnd.selectedOutbox:m_xMailWnd.selectedInbox)=row;
            m_xMailWnd.OpenSelected();
        }
        return TRUE;
    }
    if (GetWindowInMousePos(ptMouse)==_WND_ID_QUICK) { m_xQuickWnd.Activate(m_xQuickWnd.SlotAt(ptMouse),GetTickCount(),true); return TRUE; }
    if (GetWindowInMousePos(ptMouse)==_WND_ID_STORE) { m_xStoreWnd.OnDoubleClick(ptMouse); return TRUE; }
    if (GetWindowInMousePos(ptMouse)==_WND_ID_SKILL && m_xSkillWnd.OpenKeyMenu(ptMouse)) {
        WindowFocusChangedAndSetReadyMove(_WND_ID_SKILL,ptMouse); m_bMoveFocusedWnd=FALSE;
        return TRUE;
    }
	if ( IsTopWindow(m_xInventoryWnd.m_nID) )
		m_xInventoryWnd.OnLButtonDoubleClick(&m_stCommonItem, ptMouse);
	return TRUE;
}


BOOL CInterface::OnKeyUp(WPARAM wParam, LPARAM lParam)
{
    if (m_xGuildEditor.m_bActive) {  return TRUE; }
    if (m_xMsgBox.IsActive()) return TRUE;
	if ( m_xInventoryWnd.OnKeyUp(&m_stCommonItem, wParam, lParam) )
	{
		return TRUE;
	}
	return FALSE;
}


BOOL CInterface::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    if (m_xMsgBox.IsActive()) {
        if (!(lParam & (1L << 30))) FinishNotice(m_xMsgBox.OnKeyDown(wParam),false);
        return TRUE;
    }
    if (m_xGuildEditor.m_bActive) return TRUE;
    const int skillSlot=CSkillWnd::PhysicalKeySlot(wParam);
    if(skillSlot) {
        const bool repeated=(lParam & (1L<<30))!=0;
        if(repeated && skillSlot!=10 && skillSlot!=12) return TRUE;
        if(GetWindowInMousePos(g_xGameProc.m_ptMousePos)==_WND_ID_SKILL &&
            m_xSkillWnd.OnKeyDown(wParam,lParam,g_xGameProc.m_ptMousePos)) return TRUE;
        return !SpellPointerAvailable() || !m_xSkillWnd.GetMagicByShortcut(skillSlot);
    }
    if(lParam & (1L<<30)) return TRUE;
    const struct { WPARAM key;int window; } panels[]={
        {'E',_WND_ID_SKILL},{'Q',_WND_ID_STATUS},{'W',_WND_ID_INVENTORY},{'G',_WND_ID_GROUP},{'N',_WND_ID_OPTION}};
    for(const auto& panel:panels) if(wParam==panel.key) {WindowActivate(panel.window);return TRUE;}
    if(wParam=='R') {m_chatExpanded=!m_chatExpanded;LayoutChat();return TRUE;}
    if(wParam==VK_ESCAPE) {
        if(m_xStoreWnd.m_bActive) {
            if(m_xStoreWnd.details || m_xStoreWnd.bulk>=0) m_xStoreWnd.CloseOffers();
            else WindowActivate(_WND_ID_STORE);
            return TRUE;
        }
        m_xWndOrderList.MoveCurrentToTail();
        for(int i=m_xWndOrderList.GetCounter();i>0;--i) {
            const int window=*m_xWndOrderList.GetCurrentData();
            switch(window) {
            case _WND_ID_EXCHANGE:return TRUE;
            case _WND_ID_GROUP:case _WND_ID_SKILL:case _WND_ID_STATUS:case _WND_ID_INVENTORY:
            case _WND_ID_GUILD:case _WND_ID_OPTION:case _WND_ID_PORTRAIT:case _WND_ID_HORSE:
            case _WND_ID_QUEST:case _WND_ID_QUICK:case _WND_ID_MAIL:case SiegeLayout::WindowId:
            case ConsignmentLayout::BrowserId:case ConsignmentLayout::RegistrationId:
                WindowActivate(window);return TRUE;
            }
            m_xWndOrderList.MovePreviousNode();
        }
        return TRUE;
    }
    if(wParam>='1' && wParam<='6' && !(wParam<='2' && (GetKeyState(VK_SHIFT)&0x8000))) {
        m_xQuickWnd.Activate(int(wParam-'1'),GetTickCount());
        return TRUE;
    }
    if(IsTopWindow(ConsignmentLayout::BrowserId) && m_xConsignmentWnd.KeyDown(wParam)) return TRUE;
    if(m_minimap.visible && (wParam=='T' || wParam=='Y')) {
        if(wParam=='T') {m_minimap.large=!m_minimap.large;m_minimap.Place();} else m_minimap.translucent=!m_minimap.translucent;
        return TRUE;
    }
    if(wParam=='P' && m_xSiegeWnd.controlled) {WindowActivate(SiegeLayout::WindowId);return TRUE;}
	switch ( wParam )
	{
    case 'C': m_xExchangeWnd.RequestTrade(); return TRUE;
    case 'F': g_xClientSocket.SendGuildHome(); return TRUE;
    case 'J': m_minimap.TogglePortrait(GetTickCount()); return TRUE;
    case 'V': m_minimap.Toggle(GetTickCount(),1000); return TRUE;
    case 'Z':
        WindowActivate(_WND_ID_QUICK);
        return TRUE;
    case 'S':
        WindowActivate(_WND_ID_HORSE);
        return TRUE;
    case 'D':
        WindowActivate(_WND_ID_QUEST);
        return TRUE;
    case 'B':
        m_bShowKey=!m_bShowKey;
        return TRUE;
	case VK_RETURN:	
		break;

	}

	return FALSE;
}


bool CInterface::SpellPointerAvailable()
{
    constexpr int CanvasWidth=800,CanvasHeight=600,SceneBottom=467,ChatLeft=178,ChatWidth=456,ExpandedChatTop=281,CollapsedChatTop=448;
    const POINT point=g_xGameProc.m_ptMousePos;
    const RECT scene={0,0,CanvasWidth,SceneBottom};
    const RECT chat={ChatLeft,m_chatExpanded?ExpandedChatTop:CollapsedChatTop,ChatLeft+ChatWidth,CanvasHeight};
    return !m_minimap.visible && !m_xMsgBox.IsActive() && !m_xGuildEditor.m_bActive &&
        PtInRect(&scene,point) && !PtInRect(&chat,point) && GetWindowInMousePos(point)==-1;
}

LRESULT	CInterface::OnMsgInputted(WPARAM wParam, LPARAM lParam)
{
    if (m_xMsgBox.IsActive() && ((m_nIdxMsgBox >= 1 && m_nIdxMsgBox <= 11) || m_nIdxMsgBox==TradeWire::QuantityDialog || m_nIdxMsgBox==MailLayout::GoldDialogAction || m_nIdxMsgBox==ConsignmentLayout::PriceDialog || m_nIdxMsgBox==SiegeLayout::LoadDialog)) {
        SetWindowTextA(g_xChatEditBox.GetSafehWnd(),g_xChatEditBox.m_szInputMsg);
        FinishNotice(CLICK_YES,false);
        return TRUE;
    }
	if ( m_xNPCWnd.OnMsgInputted() )
		return TRUE;

	return 0L;
}
int CInterface::ButtonHintAt(POINT point,POINT& anchor)
{
    if (m_xMsgBox.IsActive() || m_xGuildEditor.m_bActive) return 0;
    const auto resolve=[&](const RECT& panel,int frame,int x,int y,int message,bool pressed) {
        if (pressed || !m_pxInterImg->NewSetIndex(frame)) return 0;
        const auto info=*m_pxInterImg->m_lpstNewCurrWilImageInfo;
        const RECT hit={panel.left+x,panel.top+y,panel.left+x+info.shWidth,panel.top+y+info.shHeight};
        if (!PtInRect(&hit,point)) return 0;
        anchor={hit.right,hit.bottom}; return message;
    };
    const RECT mailNotification=m_xMailWnd.NotificationBounds();
    if(GetWindowInMousePos(point)==-1 && PtInRect(&mailNotification,point)) {
        anchor={mailNotification.right,mailNotification.bottom};return MailWire::NotificationCaption;
    }
    const RECT siegeEntry=m_xSiegeWnd.EntryBounds();
    if(GetWindowInMousePos(point)==-1 && !m_xSiegeWnd.entryPressed && PtInRect(&siegeEntry,point)) {
        anchor={siegeEntry.right,siegeEntry.bottom};return SiegeLayout::EntryCaption;
    }
    switch (GetWindowInMousePos(point)) {
    case SiegeLayout::WindowId:return m_xSiegeWnd.HintAt(point,anchor);
    case ConsignmentLayout::BrowserId:case ConsignmentLayout::RegistrationId: {
        auto& panel=GetWindowInMousePos(point)==ConsignmentLayout::BrowserId?m_xConsignmentWnd:m_xConsignmentRegistration;
        for(const auto& button:panel.Buttons()) if(const int hint=resolve(panel.m_rcWnd,button.normal,button.x,button.y,button.caption,panel.pressed==int(button.action))) return hint;
        return 0;
    }
    case _WND_ID_MAIL:
        for(const auto& button:m_xMailWnd.VisibleButtons()) if(const int hint=resolve(m_xMailWnd.m_rcWnd,button.normal,button.x,button.y,button.caption,m_xMailWnd.pressed==int(button.action))) return hint;
        return 0;
    case _WND_ID_MINIMAP: return m_minimap.HintAt(point,anchor,*m_pxInterImg);
    case _WND_ID_GROUP: {
        const int buttons[][4]={{1221,217,200,1052},{1362,38,192,1600},{1365,98,192,1601},{1368,158,192,1602}};
        for (int i=0;i<4;++i) if (const int hint=resolve(m_xGroupWnd.m_rcWnd,buttons[i][0],buttons[i][1],buttons[i][2],buttons[i][3],m_xGroupWnd.PressedButton()==i)) return hint;
        return 0;
    }
    case _WND_ID_GUILD: {
        const int buttons[][4]={{1221,554,401,1052},{1301,39,385,4010},{1304,89,385,4009},{1307,139,385,4008},{1310,289,385,4011},{1313,339,385,4012},{1316,389,385,4013},{1320,439,385,4014},{1323,489,385,4015}};
        for (int i=0;i<9;++i) if (const int hint=resolve(m_xGuildWnd.m_rcWnd,buttons[i][0],buttons[i][1],buttons[i][2],buttons[i][3],m_xGuildWnd.m_pressed==i)) return hint;
        return 0;
    }
    case _WND_ID_SKILL:
        for (int i=0;i<9;++i) if (const int hint=resolve(m_xSkillWnd.m_rcWnd,i?1640+2*(i-1):1221,i?46+34*(i-1):316,i?30:422,i?5049+i:1052,m_xSkillWnd.m_skillPressed==i)) return hint;
        return 0;
    case _WND_ID_EXCHANGE: return resolve(m_xExchangeWnd.m_rcWnd,1394,226,279,m_xExchangeWnd.confirmed?1303:1300,m_xExchangeWnd.confirmPressed);
    case _WND_ID_NPCCHAT: return resolve(RECT{0,0,386,m_xNPCWnd.Height()},1221,346,m_xNPCWnd.Height()-42,1052,m_xNPCWnd.closePressed || !m_xNPCWnd.canClose);
    case _WND_ID_INVENTORY: return resolve(m_xInventoryWnd.m_rcWnd,1221,242,423,1052,m_xInventoryWnd.ClosePressed());
    case _WND_ID_STATUS: return resolve(m_xStatusWnd.m_rcWnd,1221,289,14,1052,m_xStatusWnd.ClosePressed());
    case _WND_ID_QUEST:
        for (int i=0;i<2;++i) if (const int hint=resolve(m_xQuestWnd.m_rcWnd,i?1425:1422,267,i?107:75,i?1052:2200,m_xQuestWnd.pressed==i)) return hint;
        return 0;
    case _WND_ID_HORSE: {
        const int buttons[][4]={{1221,236,279,1052},{1402,19,232,4100},{1405,77,232,4101},{1408,134,232,4102},{1410,191,232,4103}};
        for (int i=0;i<5;++i) if (const int hint=resolve(m_xHorseWnd.m_rcWnd,buttons[i][0],buttons[i][1],buttons[i][2],buttons[i][3],m_xHorseWnd.pressed==i)) return hint;
        return 0;
    }
    case -1: {
        if (const int hint=m_xPortraitWnd.HintAt(point,anchor)) return hint;
        // Native button indices 7..15 predate the MIR3 ordering.
        const int reference[]={0,1,2,3,4,5,6,14,7,10,8,9,15,11,12,13,16,17,18,19,20,21,22};
        for (int i=0;i<VisibleButtons();++i) {
            auto& button=m_xInterBtn[i]; if (!button.Hit(point)) continue;
            if (button.GetGameBtnState()==_BTN_STATE_CLICK) return 0;
            const RECT bounds=button.m_customHit?button.m_hitRect:button.m_rcBtn; anchor={bounds.right,bounds.bottom};
            if (i==18) return m_chatMode==1?2631:2618;
            if (i==19) return m_chatMode==2?2630:2619;
            if (i>=20) return m_chatReceptionHint[i-20];
            return 2600+reference[i];
        }
        return 0;
    }
    default: return m_xPortraitWnd.HintAt(point,anchor);
    }
}
void CInterface::ShowButtonHint(POINT point)
{
    POINT anchor; const int hint=ButtonHintAt(point,anchor); if (!hint) return;
    static CLoginMessages captions; static const bool loaded=captions.Load("./CmList.dat");
    const auto& caption=captions.Get(hint); if (caption.empty()) return;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0}; HFONT font=g_xMainWnd.CreateGameFont(face,9);
    HDC measure=CreateCompatibleDC(NULL); auto old=SelectObject(measure,font); SIZE size={};
    GetTextExtentPoint32A(measure,caption.data(),int(caption.size()),&size); SelectObject(measure,old); DeleteDC(measure);
    if (previousHint!=caption) { previousHint=caption; hintExpanding=true; }
    const int width=size.cx+12,fullHeight=size.cy+4;
    if (hintExpanding) ++hintHeight;
    const int height=hintExpanding?min(hintHeight,fullHeight):fullHeight;
    if (hintExpanding && hintHeight>=fullHeight) { hintExpanding=false; hintHeight=6; }
    const int left=min(anchor.x,800-width); RECT box={left,anchor.y-height,left+width,anchor.y};
    DrawLegacyPanel(box,RGB(255,255,150),255,RGB(0,0,0));
    HDC dc; auto* surface=g_xMainWnd.GetBackBuffer();
    if (SUCCEEDED(surface->GetDC(&dc))) {
        old=SelectObject(dc,font); SetBkMode(dc,TRANSPARENT); SetTextColor(dc,0);
        const int saved=SaveDC(dc); IntersectClipRect(dc,box.left,box.top,box.right,box.bottom);
        DrawTextA(dc,caption.data(),int(caption.size()),&box,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        RestoreDC(dc,saved); SelectObject(dc,old); surface->ReleaseDC(dc);
    }
    DeleteObject(font);
}
