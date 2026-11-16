/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/



#include "StdAfx.h"
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
    m_chatMode = 0;
    m_chatModeChanged = false;
    for (int i=0;i<3;++i) { m_chatRejected[i]=false; m_chatReceptionFrame[i]=-1; }
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
    m_xHorseWnd.Init();
	m_xStoreWnd.Init();
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
    m_portraits.Destroy();
	m_xClientSysMsg.Destroy();

	m_pxInterImg = NULL;

	m_xInventoryWnd.Destroy();
	m_xStoreWnd.Destroy();
	m_xStatusWnd.Destroy();
    m_xSkillWnd.Destroy();
    m_xQuestWnd.Destroy();
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
	m_xStoreWnd.CreateStoreWnd(_WND_ID_STORE, m_pxInterImg, _WNDIMGIDX_STORE, 0, 0, 0, 0, TRUE);
	m_xExchangeWnd.CreateExchangeWnd(_WND_ID_EXCHANGE, m_pxInterImg, 1390, 0, 0, 494, 330, TRUE);
	m_xGuildMasterWnd.CreateGuildMasterWnd(_WND_ID_GUILDMASTER, m_pxInterImg, _WNDIMGIDX_GUILDMASTER, 0, 0, 0, 0, TRUE);
	m_xGuildWnd.CreateGuildWnd(_WND_ID_GUILD, m_pxInterImg, 1300, 102, 22, 594, 444, TRUE);
	m_xGroupWnd.CreateGroupWnd(_WND_ID_GROUP, m_pxInterImg, 1360, 272, 123, 258, 244, TRUE);
	m_xGroupPopWnd.CreateGroupPopWnd(_WND_ID_GROUPPOP, m_pxInterImg, 1240, 220, 112, 360, 268, TRUE);
	m_xChatPopWnd.CreateChatPopWnd(_WND_ID_CHATPOP, m_pxInterImg, _WNDIMGIDX_CHATPOP, 0, 0, 0, 0, TRUE);

	m_xOptionWnd.CreateOptionWnd(_WND_ID_OPTION, m_pxInterImg, 1290, 145, 30, 510, 468, TRUE);
	m_xQuestWnd.CreateQuestWnd(_WND_ID_QUEST, m_pxInterImg, 1420, 0, 0, 340, 440, TRUE);
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
    if (!message || !*message) return;
    m_xChatPopWnd.MsgAdd(color,background,message);
    m_chatHistory.push_back(ChatMessage{color,background,message});
    if (m_chatHistory.size() > 50) m_chatHistory.erase(m_chatHistory.begin());
    ReflowChat();
}

void CInterface::ReflowChat()
{
    m_xChat.ClearAllNodes();
    HDC dc = GetDC(g_xMainWnd.GetSafehWnd());
    const char face[] = {char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font = g_xMainWnd.CreateGameFont(face,9);
    HGDIOBJ old = SelectObject(dc,font);
    const int width = m_chatExpanded ? 429 : 341;
    for (const auto& message : m_chatHistory) {
        const char* begin = message.text.c_str();
        while (*begin) {
            const char* end = begin;
            while (*end && *end != '\n' && *end != '\r') {
                const char* next = CharNextA(end);
                SIZE size = {};
                GetTextExtentPoint32A(dc,begin,int(next-begin),&size);
                if (end > begin && (size.cx > width || next-begin >= MAX_PATH)) break;
                end = next;
            }
            CHAT line = {};
            line.dwFontColor = message.color;
            line.dwBackColor = message.background;
            memcpy(line.pszChat,begin,end-begin);
            m_xChat.AddNode(line);
            if (*end == '\r') ++end;
            if (*end == '\n') ++end;
            begin = end;
        }
    }
    SelectObject(dc,old);
    DeleteObject(font);
    ReleaseDC(g_xMainWnd.GetSafehWnd(),dc);
    m_nFstLine = max(0,m_xChat.GetCounter()-ChatRows());
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
				m_xStoreWnd.ShowStoreWnd();
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
				m_xNPCWnd.ShowNPCChatWnd();
				break;

			case _WND_ID_QUEST:
				m_xQuestWnd.ShowQuestWnd();
				break;
			case _WND_ID_OPTION:
				m_xOptionWnd.ShowOptionWnd();
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
	INT nX, nY, nCnt;

	// 窗口.


	// 鼠标物品.


	// 腰带物品。
	ShowBeltItem();

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
    const int hoveredWindow=m_xMsgBox.IsActive()?-1:GetWindowInMousePos(ptMousePos);
    m_xInventoryWnd.UpdatePlacementPointer(ptMousePos,hoveredWindow==_WND_ID_INVENTORY);
    m_xStatusWnd.UpdatePointer(ptMousePos,hoveredWindow==_WND_ID_STATUS);
    m_xSkillWnd.UpdatePointer(ptMousePos,hoveredWindow==_WND_ID_SKILL);
    m_xHorseWnd.UpdatePointer(ptMousePos,hoveredWindow==_WND_ID_HORSE);
    ShowWndList();
	if ( m_stCommonItem.bSetted && !m_stCommonItem.bIsHideItem )
	{
		nX = ptMousePos.x - _INVENTORY_CELL_WIDTH /2;
		nY = ptMousePos.y - _INVENTORY_CELL_HEIGHT/2;
		m_stCommonItem.xItem.DrawItem(nX, nY);
	}
	m_xMsgBox.RenderMessageBox(0);

	ShowMagicKey();
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



VOID CInterface::ShowGameStatus()
{
    const auto& hero = g_xGameProc.m_xMyHero;
    const int portrait = 650 + 2 * hero.m_bJob + (hero.m_stFeature.bGender != 0);
    if (hero.m_bJob <= 2 && m_portraits.NewSetIndex(portrait)) {
        const auto& info = *m_portraits.m_lpstNewCurrWilImageInfo;
        g_xMainWnd.DrawWithImageForComp(m_rcMain.left+534,m_rcMain.top+20,
            info.shWidth,info.shHeight,(WORD*)m_portraits.m_pbCurrImage);
    }

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
        if (const auto skill=m_xSkillWnd.GetMagicByKey(CSkillWnd::LegacyKey(index+1))) {
            const int id=skill->stStdMagic.wMagicID&255;
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
				DeleteWindowToList(_WND_ID_EXCHANGE);
				m_xExchangeWnd.SetGameWndActive(FALSE); m_xExchangeWnd.confirmPressed=m_xExchangeWnd.headerPressed=false; m_xExchangeWnd.captured=-1;
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
				m_xNPCWnd.SetGameWndActive(FALSE);
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
				if ( !m_xOptionWnd.OnLButtonDown(ptMouse) )
					WindowFocusChangedAndSetReadyMove(nInterfaceWndInRect, ptMouse);
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
void CInterface::ShowNotice(int message)
{
    static CLoginMessages captions;
    static const bool loaded=captions.Load("./CmList.dat");
    m_nIdxMsgBox=0;
    m_xMsgBox.SetOrigin(220,112,false);
    m_xMsgBox.ShowMessageBox(captions.Get(message).c_str(),MSG_BTN_OK,1240);
}

void CInterface::OpenNameDialog(int action)
{
    static CLoginMessages captions;
    static const bool loaded = captions.Load("./CmList.dat");
    m_nIdxMsgBox = action;
    m_xMsgBox.SetOrigin(220,112,true);
    std::string prompt = action==4 ? "'"+m_xGuildWnd.m_name+"' "+captions.Get(4001) :
        captions.Get(action==6?1304:action==5?4002:action==3?1606:1605);
    m_xMsgBox.ShowMessageBox(prompt.c_str(),MSG_BTN_YESNO,1240);
    g_xChatEditBox.MoveScaled(g_xMainWnd.m_rcWindow.left+244,g_xMainWnd.m_rcWindow.top+340,309,15,TRUE);
    g_xChatEditBox.SetLimitText(90);
    g_xChatEditBox.m_szInputMsg[0]=0;
    SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"");
    ShowWindow(g_xChatEditBox.GetSafehWnd(),SW_SHOW);
    SetFocus(g_xChatEditBox.GetSafehWnd());
}

void CInterface::FinishNotice(int hResult, bool mouse)
{
    if (!hResult) return;
    if (m_nIdxMsgBox==0) {
        if (mouse) g_xSound.PlayLobbyEffect("105.wav");
        m_xMsgBox.HideMessageBox();
        return;
    }
    if (m_nIdxMsgBox >= 1 && m_nIdxMsgBox <= 6) {
        char name[91] = {};
        GetWindowTextA(g_xChatEditBox.GetSafehWnd(),name,sizeof(name));
        if (mouse) g_xSound.PlayLobbyEffect("105.wav");
        m_xMsgBox.HideMessageBox();
        SetWindowTextA(g_xChatEditBox.GetSafehWnd(),"");
        g_xChatEditBox.m_szInputMsg[0]=0;
        ShowWindow(g_xChatEditBox.GetSafehWnd(),SW_HIDE);
        SetFocus(g_xMainWnd.GetSafehWnd());
        g_xChatEditBox.MoveScaled(g_xMainWnd.m_rcWindow.left+185,
            g_xMainWnd.m_rcWindow.top+(m_chatExpanded?534:575),m_chatExpanded?432:344,15,TRUE);
        if (hResult == CLICK_YES) {
            if (m_nIdxMsgBox == 1) g_xClientSocket.SendCreateGroup(name);
            else if (m_nIdxMsgBox == 2) g_xClientSocket.SendAddGroupMember(name);
            else if (m_nIdxMsgBox == 3) g_xClientSocket.SendDelGroupMember(name);
            else if (m_nIdxMsgBox == 4) g_xClientSocket.SendGuildMemberRemoval(name);
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

            if (mouse) g_xSound.PlayLobbyEffect("105.wav");
			switch ( m_nIdxMsgBox )
			{
			case 12:
				{
					switch ( hResult )	// 没有编辑框.
					{
					case 2:
						{
						   DWORD dwCurrTick = timeGetTime();
						   if ( (dwCurrTick - g_xGameProc.m_xMyHero.m_dwLastPKStruckTime> 10000) &&
								(dwCurrTick - g_xGameProc.m_xMyHero.m_dwLastMagicTime   > 10000) &&
								(dwCurrTick - g_xGameProc.m_xMyHero.m_dwLastHitTime		> 10000) ||
								(g_xGameProc.m_xMyHero.m_bCurrMtn					== _MT_DIE ) )
								SendMessage(g_xMainWnd.GetSafehWnd(), WM_DESTROY, NULL, NULL);
						   else
						   {
							   DWORD dwFont, dwBack;

							   dwFont = GetChatColor(3);
							   dwBack = GetChatColor(2);

							   MsgAdd(dwFont, dwBack, "You cannot disconnect during combat.");
						   }
						}
						break;
					case 4:
						m_xMsgBox.HideMessageBox();
						break;
					}
				}
				break;
			}

}

BOOL CInterface::OnLButtonUp(POINT ptMouse)
{
    if (m_xExchangeWnd.captured>=0 || m_xExchangeWnd.confirmPressed || m_xExchangeWnd.headerPressed) { m_bMoveFocusedWnd=FALSE; m_xExchangeWnd.OnLButtonUp(ptMouse); return TRUE; }
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

		for( INT nCnt = 0; nCnt < VisibleButtons(); nCnt++ )
		{
			if ( m_xInterBtn[nCnt].OnLButtonUp(ptMouse) )
			{
                g_xSound.PlayLobbyEffect("105.wav");
				switch ( nCnt )
				{
                case 20:
                case 21:
                case 22:
                    {
                        const int channel = nCnt-20;
                        // The server owns rejection state; a click only predicts the overlay.
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

				case _BTN_ID_CHANGE:	WindowActivate(_WND_ID_EXCHANGE);				break;
				case _BTN_ID_GUILD:		
					g_xClientSocket.SendGuildHome();
					break;
				case _BTN_ID_GROUP:		WindowActivate(_WND_ID_GROUP);					break;
				case _BTN_ID_POPUP:		WindowActivate(_WND_ID_CHATPOP);				break;
				case _BTN_ID_BAG:		WindowActivate(_WND_ID_INVENTORY);				break;
				case _BTN_ID_MUGONG:	m_bShowKey = !m_bShowKey;						break;
				case _BTN_ID_BELT:
                    m_chatExpanded = !m_chatExpanded;
                    LayoutChat();
                    break;
                case _BTN_ID_EXIT:
                    {
                        static CLoginMessages captions;
                        static const bool loaded = captions.Load("./CmList.dat");
                        m_nIdxMsgBox = 12;
                        m_xMsgBox.SetOrigin(220,112,false);
                        m_xMsgBox.ShowMessageBox(captions.Get(2636).c_str(),MSG_BTN_YESNO,1240);
                    }
                    break;
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
			case _WND_ID_SKILL:		if ( m_xSkillWnd.OnLButtonUp(ptMouse) )		WindowActivate(_WND_ID_SKILL);			break;
case _WND_ID_STATUS:		if ( m_xStatusWnd.OnLButtonUp(&m_stCommonItem, ptMouse) )		WindowActivate(_WND_ID_STATUS);			break;
			case _WND_ID_STORE:			if ( m_xStoreWnd.OnLButtonUp(ptMouse) )							WindowActivate(_WND_ID_STORE);			break;
			case _WND_ID_EXCHANGE:		if ( m_xExchangeWnd.OnLButtonUp(ptMouse) )						WindowActivate(_WND_ID_EXCHANGE);		break;
			case _WND_ID_GUILDMASTER:
			{
				if ( m_xGuildMasterWnd.OnLButtonUp(ptMouse) )
					WindowActivate(_WND_ID_EXCHANGE);
				break;
			}
			case _WND_ID_GUILD:			if ( m_xGuildWnd.OnLButtonUp(ptMouse) )							WindowActivate(_WND_ID_GUILD);			break;
			case _WND_ID_GROUP:			if ( m_xGroupWnd.OnLButtonUp(ptMouse) )							WindowActivate(_WND_ID_GROUP);			break;
			case _WND_ID_GROUPPOP:		if ( m_xGroupPopWnd.OnLButtonUp(ptMouse) )						WindowActivate(_WND_ID_GROUPPOP);		break;
			case _WND_ID_CHATPOP:		if ( m_xChatPopWnd.OnLButtonUp(ptMouse) )						WindowActivate(_WND_ID_CHATPOP);		break;

			case _WND_ID_QUEST:			if ( m_xQuestWnd.OnLButtonUp(ptMouse) )							WindowActivate(_WND_ID_QUEST);			break;
			case _WND_ID_OPTION:		if ( m_xOptionWnd.OnLButtonUp(ptMouse) )						WindowActivate(_WND_ID_OPTION);			break;
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
    if (m_xExchangeWnd.captured>=0) { m_xExchangeWnd.OnMouseMove(ptMouse); return TRUE; }
    if (m_xQuestWnd.scrollCaptured) { m_xQuestWnd.OnMouseMove(ptMouse); return TRUE; }
    if (m_xSkillWnd.m_skillScrollCaptured) { m_xSkillWnd.OnMouseMove(ptMouse); return TRUE; }
    if (m_xInventoryWnd.m_scrollCaptured) { m_xInventoryWnd.OnMouseMove(ptMouse); return TRUE; }
    if (m_xOptionWnd.m_audioDrag>=0) { m_xOptionWnd.OnMouseMove(ptMouse); return TRUE; }
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
    if (m_xMsgBox.IsActive()) return TRUE;
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
	else if (IsTopWindow(m_xNPCWnd.m_nID))		// 勇气
	{
		m_xNPCWnd.OnScrollDown();
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
    if (m_xMsgBox.IsActive()) return TRUE;
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
	else if (IsTopWindow(m_xNPCWnd.m_nID))		// 勇气
	{
		m_xNPCWnd.OnScrollUp();
	}
	else
	{
		if ( !m_xChatPopWnd.m_bActive )
		{
			if ( m_nFstLine < max(0, m_xChat.GetCounter()-ChatRows()) )
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
    if (m_xMsgBox.IsActive()) return TRUE;
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
	switch ( wParam )
	{
    case 'S':
        if (!(lParam & (1L << 30))) WindowActivate(_WND_ID_HORSE);
        return TRUE;
    case 'D':
        if (!(lParam & (1L << 30))) WindowActivate(_WND_ID_QUEST);
        return TRUE;
    case 'B':
        if (!(lParam & (1L << 30))) m_bShowKey=!m_bShowKey;
        return TRUE;
	case VK_RETURN:	
		break;
	case VK_F9:	
		WindowActivate(_WND_ID_INVENTORY);			
		break;
	case VK_F11:
        WindowActivate(_WND_ID_SKILL);
        break;
	}

	INT nInterfaceWndInRect;
	nInterfaceWndInRect = GetWindowInMousePos(g_xGameProc.m_ptMousePos);

	if ( nInterfaceWndInRect != -1 )
	{
		if ( nInterfaceWndInRect == _WND_ID_SKILL )
		{
			if ( m_xSkillWnd.OnKeyDown(wParam, lParam, g_xGameProc.m_ptMousePos) )
				return TRUE;
		}
	}
		
	return FALSE;
}


VOID CInterface::OnSysKeyDown()
{
    WindowActivate(_WND_ID_STATUS);
}

LRESULT	CInterface::OnMsgInputted(WPARAM wParam, LPARAM lParam)
{
    if (m_xMsgBox.IsActive() && m_nIdxMsgBox >= 1 && m_nIdxMsgBox <= 6) {
        SetWindowTextA(g_xChatEditBox.GetSafehWnd(),g_xChatEditBox.m_szInputMsg);
        FinishNotice(CLICK_YES,false);
        return TRUE;
    }
	if ( m_xNPCWnd.OnMsgInputted() )
		return TRUE;

	return 0L;
}