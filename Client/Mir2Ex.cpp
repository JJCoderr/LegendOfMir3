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
        if (groupPanel==11) capturePath="options-panel.bmp";
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
        if (groupPanel==11) g_xGameProc.m_xInterface.WindowActivate(_WND_ID_OPTION);
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
        strcmp(lpCmdLine,"--preview=options-panel")==0 ? 11 : 0;
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
