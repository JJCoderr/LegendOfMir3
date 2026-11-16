/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/



#include "StdAfx.h"
#include "Common/LoginMessages.h"

static const int optionButtons[21][3] = {
    {1221,448,415},{1292,430,85},{1292,430,145},{1292,430,205},{1292,430,235},
    {-1,0,0},{-1,0,0},{1292,186,55},{1292,186,85},{1292,186,115},{1292,186,145},
    {1292,186,175},{1292,186,205},{1292,186,235},{1292,186,265},{-1,0,0},
    {1292,186,295},{1292,186,325},{1292,430,55},{1296,430,265},{1292,430,295}
};
COptionWnd::COptionWnd() { Init(); }
COptionWnd::~COptionWnd() { Destroy(); }
VOID COptionWnd::Init()
{
    CGameWnd::Init();
    ZeroMemory(m_values,sizeof(m_values));
    m_values[1]=m_values[2]=m_values[7]=m_values[8]=m_values[9]=m_values[10]=m_values[20]=true;
    m_pressed=m_hover=-1;
    m_musicLevel=m_effectsLevel=m_attackMode=0; m_audioDrag=-1;
}
VOID COptionWnd::Destroy() { CGameWnd::Destroy(); }
VOID COptionWnd::CreateOptionWnd(INT id,CWHWilImageData* image,INT frame,INT x,INT y,INT width,INT height,BOOL move)
{
    CreateGameWnd(id,image,frame,move,x,y,width,height);
    LoadAudioOptions(m_audioOptionsPath);
}
VOID COptionWnd::ShowOptionWnd()
{
    if (!m_bActive) return;
    const int x=m_rcWnd.left,y=m_rcWnd.top;
    const auto draw=[&](int frame,int left,int top) {
        if (!m_pxWndImage->NewSetIndex(frame)) return;
        const auto& image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        g_xMainWnd.DrawWithImageForComp(x+left,y+top,image.shWidth,image.shHeight,(WORD*)m_pxWndImage->m_pbCurrImage);
    };
    draw(1290,0,0);
    for (int i=0;i<21;++i) {
        const auto& button=optionButtons[i];
        if (button[0]<0) continue;
        if (i==0 || i==19) draw(button[0]+(m_pressed==i),button[1],button[2]);
        else {
            if (m_hover==i && m_pressed!=i) draw(m_values[i]?1293:1294,button[1],button[2]);
            draw((m_pressed==i?!m_values[i]:m_values[i])?1292:1295,button[1],button[2]);
        }
    }
    draw(1291,276+9*(m_musicLevel+100)/5,118);
    draw(1291,276+9*(m_effectsLevel+100)/5,178);
    auto* surface=g_xMainWnd.GetBackBuffer(); HDC dc;
    if (FAILED(surface->GetDC(&dc))) return;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font=g_xMainWnd.CreateGameFont(face,9); HGDIOBJ previous=SelectObject(dc,font);
    SetBkMode(dc,TRANSPARENT);
    static CLoginMessages captions; static const bool loaded=captions.Load("./CmList.dat");
    const auto text=[&](int id,int left,int top,int width,int height,COLORREF color) {
        RECT area={x+left,y+top,x+left+width,y+top+height};
        SetTextColor(dc,color);
        DrawTextA(dc,captions.Get(id).c_str(),-1,&area,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    };
    text(2104,60,30,390,15,0xf8fafa);
    for (int i=0;i<3;++i) text(2100+i,266,89+60*i,156,21,0x0096ff);
    for (int i=0;i<10;++i) text(2105+i,20,62+30*i,155,15,0x0096ff);
    text(2115,266,59,156,21,0x0096ff);
    text(2103,266,239,156,21,0x0096ff);
    text(2116+m_attackMode,266,269,156,21,0x0096ff);
    text(2130,266,300,156,20,0x0096ff);
    SelectObject(dc,previous); DeleteObject(font); surface->ReleaseDC(dc);
}
int COptionWnd::ButtonAt(POINT point)
{
    for (int i=0;i<21;++i) {
        const auto& button=optionButtons[i];
        if (button[0]<0 || !m_pxWndImage->NewSetIndex(button[0])) continue;
        const auto& image=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        RECT hit={m_rcWnd.left+button[1],m_rcWnd.top+button[2],
            m_rcWnd.left+button[1]+image.shWidth,m_rcWnd.top+button[2]+image.shHeight};
        if (PtInRect(&hit,point)) return i;
    }
    return -1;
}
BOOL COptionWnd::OnLButtonDown(POINT point) { if (AudioInput(0,point)) return TRUE; m_pressed=ButtonAt(point); return m_pressed>=0; }
BOOL COptionWnd::OnLButtonUp(POINT point)
{
    if (AudioInput(2,point)) return FALSE;
    const int hit=ButtonAt(point); const bool clicked=hit>=0 && hit==m_pressed; m_pressed=-1;
    if (!clicked) return FALSE;
    g_xSound.PlayLobbyEffect("105.wav");
    if (hit==1) { m_values[1]=!m_values[1]; if (m_values[1]) g_xSound.SetEnableBGM(); else g_xSound.SetDisableBGM(); }
    if (hit==2) { m_values[2]=!m_values[2]; if (m_values[2]) g_xSound.SetEnablePlay(); else g_xSound.SetDisablePlay(); }
    if (hit==3) { m_values[3]=!m_values[3]; g_xSound.m_reversePan=m_values[3]; }
    if(hit==4) {g_xGameProc.SetShadowBlend(!m_values[4]);SaveAudioOptions();}
    if(hit==7 || hit==9 || hit==10 || hit==20) {
        m_values[hit]=!m_values[hit];
        static CLoginMessages captions;static const bool loaded=captions.Load("./CmList.dat");
        const int id=hit==7?(m_values[7]?2646:2647):hit==9?(m_values[9]?1518:1517):hit==10?(m_values[10]?2122:2121):(m_values[20]?2132:2131);
        std::string message=captions.Get(id);
        g_xGameProc.m_xInterface.MsgAdd(0xffffff,33023,const_cast<char*>(message.c_str()));
        SaveAudioOptions();
    }
    if (hit>=1 && hit<=3) SaveAudioOptions();
    return hit==0;
}
VOID COptionWnd::OnMouseMove(POINT point) { if (!AudioInput(1,point)) m_hover=ButtonAt(point); }
VOID COptionWnd::SetStatusBtnInit() { m_hover=m_pressed=-1; }

bool COptionWnd::AudioInput(int kind, POINT point)
{
    const int x=point.x-m_rcWnd.left,y=point.y-m_rcWnd.top;
    if (m_audioDrag>=0) {
        if (kind==1) {
            const int offset=x<288?0:x>468?185:x-288;
            int& level=m_audioDrag==0?m_musicLevel:m_effectsLevel;
            level=(5*offset-900)/9;
            g_xSound.SetInterfaceVolume(m_audioDrag==0,level);
            SaveAudioOptions();
        }
        if (kind==2) { m_audioDrag=-1; if (GetCapture()==g_xMainWnd.GetSafehWnd()) ReleaseCapture(); }
        return true;
    }
    if (kind==1 || !m_pxWndImage->NewSetIndex(1291)) return false;
    const auto& knob=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
    for (int i=0;i<2;++i) {
        int& level=i==0?m_musicLevel:m_effectsLevel;
        int offset=9*(level+100)/5;
        RECT hit={276+offset,118+60*i,276+offset+knob.shWidth,118+60*i+knob.shHeight};
        POINT local={x,y};
        if (PtInRect(&hit,local)) {
            if (kind==0) { m_audioDrag=i; SetCapture(g_xMainWnd.GetSafehWnd()); g_xSound.PlayLobbyEffect("105.wav"); }
            return true;
        }
        if (x>266 && x<487 && y>116+60*i && y<136+60*i) {
            if (kind==2) {
                if (x>offset+292 || x>477) offset=offset+9>180?185:offset+9;
                else if (x<offset+276 || x<275) offset=offset<9?0:offset-9;
                level=(5*offset-900)/9;
                g_xSound.SetInterfaceVolume(i==0,level);
                SaveAudioOptions();
            }
            return true;
        }
    }
    return false;
}

void COptionWnd::LoadAudioOptions(const char* path)
{
    m_audioOptionsPath=path;
    const auto read=[path](const char* key,int fallback) {
        return path ? (int)GetPrivateProfileIntA("Options",key,fallback,path) : fallback;
    };
    m_values[1]=read("BGM",1)!=0;
    m_values[2]=read("EffectSound",1)!=0;
    m_values[3]=read("ReversePan",0)==1;
    m_values[20]=read("UserPic",1)!=0;
    m_values[7]=read("ShowMon",1)!=0;
    m_values[9]=read("MagicLight",1)!=0;
    m_values[10]=read("BloodNum",1)!=0;
    g_xGameProc.SetShadowBlend(read("ShadowBlend",0)!=0);
    m_musicLevel=read("BGMLevel",0);
    m_effectsLevel=read("EffectSoundLevel",0);
    if (m_musicLevel < -100 || m_musicLevel > 0) m_musicLevel=0;
    if (m_effectsLevel < -100 || m_effectsLevel > 0) m_effectsLevel=0;
    if (m_values[1]) g_xSound.SetEnableBGM(); else g_xSound.SetDisableBGM();
    if (m_values[2]) g_xSound.SetEnablePlay(); else g_xSound.SetDisablePlay();
    g_xSound.m_reversePan=m_values[3];
    g_xSound.SetInterfaceVolume(true,m_musicLevel);
    g_xSound.SetInterfaceVolume(false,m_effectsLevel);
}

bool COptionWnd::SaveAudioOptions()
{
    if (!m_audioOptionsPath) return true;
    const auto write=[this](const char* key,int value) {
        char text[16]; wsprintfA(text,"%d",value);
        return WritePrivateProfileStringA("Options",key,text,m_audioOptionsPath)!=FALSE;
    };
    bool saved=write("BloodNum",m_values[10]);
    saved=write("MagicLight",m_values[9]) && saved;
    saved=write("ShadowBlend",m_values[4]) && saved;
    saved=write("ShowMon",m_values[7]) && saved;
    saved=write("UserPic",m_values[20]) && saved;
    saved=write("BGM",m_values[1]) && saved;
    saved=write("EffectSound",m_values[2]) && saved;
    saved=write("ReversePan",m_values[3]) && saved;
    saved=write("BGMLevel",m_musicLevel) && saved;
    saved=write("EffectSoundLevel",m_effectsLevel) && saved;
    if (!saved) OutputDebugStringA("Cannot save audio options.\n");
    return saved;
}
