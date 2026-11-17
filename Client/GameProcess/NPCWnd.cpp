#include "StdAfx.h"
#include "Common/LegacyPanel.h"
#include "Common/LoginMessages.h"
#include <cstdlib>

void CNPCWnd::CreateNPCChatWnd(INT id,CWHWilImageData* image,INT,INT,INT,INT,INT,BOOL)
{
    Init(); CreateGameWnd(id,image,1353,FALSE,0,0,386,224);
}
void CNPCWnd::ResetDialog()
{
    rows.clear(); m_nNpcId=promptActor=0; first=extraRows=scrollLimit=arrowPressed=arrowFrame=0; pressed=-1;
    canClose=true; closePressed=scrollEnabled=downLit=false; prompt.clear(); promptCommand.clear();
}
void CNPCWnd::SetScript(int actor,const char* input)
{
    if (actor!=m_nNpcId) { canClose=true; arrowFrame=0; downLit=false; }
    m_nNpcId=actor; rows.clear(); first=0; pressed=-1; arrowPressed=0; closePressed=false;
    const COLORREF colors[]={0,0xff,0x8000,0x8080,0x808080,0x80,0x808000,0x800000,0xc0c0c0,0x800080,0xff00,0xff0000,0xffffff,0xff00ff,0xffff00,0xffff,0x10101};
    COLORREF color=0xffffff; int line=0,estimate=0,optionId=0;
    HDC dc; auto* surface=g_xMainWnd.GetBackBuffer(); if (FAILED(surface->GetDC(&dc))) return;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0}; HFONT font=g_xMainWnd.CreateGameFont(face,9); auto old=SelectObject(dc,font);
    const auto measure=[&](const std::string& text) { SIZE size={}; GetTextExtentPoint32A(dc,text.data(),int(text.size()),&size); return size; };
    std::string script=input;
    while (!script.empty()) {
        int x=0,bytes=0;
        const auto append=[&](std::string label,const std::string& command) {
            bytes+=int(label.size()); const int option=command.empty()?-1:optionId++;
            while (!label.empty()) {
                size_t length=0; SIZE size={}; const int limit=command.empty() && line<7?198:305;
                const SIZE whole=label.size()<=1024?measure(label):SIZE{};
                const bool fits=x+whole.cx<=limit && whole.cy>0;
                if (fits) { length=label.size(); size=whole; }
                while (length<label.size()) {
                    length+=IsDBCSLeadByteEx(CP_ACP,BYTE(label[length])) && length+1<label.size()?2:1;
                    size=measure(label.substr(0,length)); if (x+size.cx>=305) break;
                }
                rows.push_back({label.substr(0,length),command,35+x,line,int(size.cx),int(size.cy),color,option});
                label.erase(0,length); if (!fits && x+size.cx>=305) { ++line; x=0; } else x+=size.cx;
            }
        };
        while (!script.empty()) {
            const auto open=script.find_first_of("\\<{"); append(script.substr(0,open),"");
            if (open==std::string::npos) { script.clear(); break; }
            script.erase(0,open); if (script[0]=='\\') break;
            const bool option=script[0]=='<'; const auto close=script.find(option?'>':'}',1);
            if (close==std::string::npos) { script.clear(); break; }
            const auto tag=script.substr(1,close-1); const auto split=tag.find('/');
            if (split!=std::string::npos) {
                const auto name=tag.substr(0,split),value=tag.substr(split+1);
                if (option) append(name,value);
                else if (name=="NOTCLOSE") canClose=false;
                else if (name=="FCOLOR" && !value.empty() && (value[0]=='-' || (value[0]>='0' && value[0]<='9'))) {
                    char* end; const long index=std::strtol(value.c_str(),&end,10);
                    if (end!=value.c_str() && index>=0 && index<17) color=colors[index];
                }
            }
            script.erase(0,close+1);
        }
        if (bytes || !script.empty()) estimate+=max(1,(bytes+49)/50);
        if (script.empty()) break; script.erase(0,1); ++line;
    }
    SelectObject(dc,old); DeleteObject(font); surface->ReleaseDC(dc);
    extraRows=max(0,min(13,estimate-9)); scrollEnabled=estimate>22; scrollLimit=scrollEnabled?max(0,line-21):0;
    m_rcWnd={0,0,386,Height()}; m_nWidth=386; m_nHeight=Height();
}
RECT CNPCWnd::TextBounds(const Row& row) const
{
    const int y=40+(row.height+5)*(row.line-first); return RECT{row.x,y,row.x+row.width,y+row.height};
}
int CNPCWnd::OptionAt(POINT point) const
{
    int option=-1,rectangles=0;
    for (const auto& row:rows) {
        if (row.option<0 || row.line<first || row.line>first+21) continue;
        if (option!=row.option) { option=row.option; rectangles=0; }
        const RECT box=TextBounds(row); if (++rectangles<=5 && PtInRect(&box,point)) return option;
    }
    return -1;
}
bool CNPCWnd::Hit(int frame,int x,int y,POINT point)
{
    if (!m_pxWndImage->NewSetIndex(frame)) return false;
    const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo; const RECT box={x,y,x+info.shWidth,y+info.shHeight}; return PtInRect(&box,point)!=FALSE;
}
BOOL CNPCWnd::OnLButtonDown(POINT point)
{
    closePressed=Hit(1221,346,Height()-42,point); arrowPressed=0;
    if (scrollEnabled && !closePressed) {
        if (first>0 && Hit(1357,183,Height()-48,point)) arrowPressed=-1;
        else if (first<scrollLimit && Hit(1355,183,Height()-30,point)) arrowPressed=1;
    }
    pressed=closePressed || arrowPressed?-1:OptionAt(point); return TRUE;
}
BOOL CNPCWnd::OnLButtonUp(POINT point)
{
    if (closePressed || Hit(1221,346,Height()-42,point)) {
        const bool close=closePressed && Hit(1221,346,Height()-42,point) && canClose;
        closePressed=false; if (close) g_xGameProc.CloseNPCWindow(); return FALSE;
    }
    if (arrowPressed) {
        if (Hit(arrowPressed<0?1357:1355,183,Height()-(arrowPressed<0?48:30),point)) first=max(0,min(scrollLimit,first+arrowPressed));
        arrowPressed=0; pressed=-1; return FALSE;
    }
    const int option=OptionAt(point); pressed=-1;
    SelectOption(option); return FALSE;
}
bool CNPCWnd::SelectOption(int option)
{
    if (!GetGameWndActive()) return false;
    const DWORD now=GetTickCount();
    if (option<0 || (lastAction && now-lastAction<=1000)) return FALSE;
    for (const auto& row:rows) if (row.option==option) {
        lastAction=now;
        if (row.command.compare(0,2,"@@")==0) {
            promptActor=m_nNpcId; promptCommand=row.command; prompt.clear();
            for (const auto& part:rows) if (part.option==option) prompt+=part.text;
            if (row.command=="@@buildguildnow") { static CLoginMessages captions; static const bool loaded=captions.Load("./CmList.dat"); prompt=captions.Get(2000); }
            g_xGameProc.m_xInterface.OpenNameDialog(7);
        } else g_xClientSocket.SendNPCMessage(CM_MERCHANTDLGSELECT,m_nNpcId,0,const_cast<char*>(row.command.c_str()));
        return true;
    }
    return false;
}
int CNPCWnd::PromptLimit() const
{
    if (promptCommand.compare(0,3,"@@*")!=0) return 90;
    bool storage=false;
    for (const char* marker:{"enter_stgpasswd","Input_stgpasswd","Change_stgpasswd","confirm_passwd"})
        if (promptCommand.find(marker)!=promptCommand.npos) storage=true;
    if (!storage) return 90;
    const auto delimiter=promptCommand.find('_',3);
    if (delimiter>=10) return 10;
    int bytes=0;
    for (size_t i=3;i<delimiter;++i) {
        if (promptCommand[i]<'0' || promptCommand[i]>'9') return 90;
        bytes=bytes*10+promptCommand[i]-'0';
    }
    return bytes>0?bytes:90;
}
bool CNPCWnd::SubmitPrompt(const char* text)
{
    const int actor=promptActor; const bool valid=actor && actor==m_nNpcId && !promptCommand.empty();
    const std::string body=promptCommand+"\r"+text; ClearPrompt();
    if (valid) g_xClientSocket.SendNPCMessage(CM_MERCHANTDLGSELECT,actor,0,const_cast<char*>(body.c_str()));
    return valid;
}
void CNPCWnd::DrawBackground()
{
    const int strips=max(1,extraRows),height=Height();
    if (cachedStrips!=strips) {
        background.assign(386*height,0x10000); cachedStrips=strips;
        const auto compose=[&](int frame,int top) {
            if (!m_pxWndImage->NewSetIndex(frame)) return;
            const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo; const WORD* next=(WORD*)m_pxWndImage->m_pbCurrImage;
            for (int y=0;y<info.shHeight;++y) {
                const WORD* run=next+1; const WORD* end=run+*next; next=end; int x=0;
                while (run+1<end) {
                    const WORD tag=*run++,count=*run++; if (tag==0xc0) { x+=count; continue; }
                    if (tag<0xc1 || tag>0xc3 || run+count>end) break;
                    for (int i=0;i<count;++i,++x) if (x<386 && top+y<height) background[(top+y)*386+x]=run[i];
                    run+=count;
                }
            }
        };
        compose(1353,0); for (int i=0;i<strips;++i) compose(1351,160+20*i); compose(1352,160+20*strips);
    }
    DDSURFACEDESC2 desc={}; desc.dwSize=sizeof(desc); auto* surface=g_xMainWnd.GetBackBuffer();
    if (FAILED(surface->Lock(NULL,&desc,DDLOCK_WAIT,NULL))) return;
    for (int y=0;y<min(height,int(desc.dwHeight));++y) {
        WORD* row=(WORD*)((BYTE*)desc.lpSurface+y*desc.lPitch);
        for (int x=0;x<min(386,int(desc.dwWidth));++x) {
            const DWORD source=background[y*386+x]; if (source==0x10000) continue; const WORD dst=row[x];
            const int r=((source>>11)*180*220+(dst>>11)*35*255+32512)/65025;
            const int g=(((source>>5)&63)*180*220+((dst>>5)&63)*35*255+32512)/65025;
            const int b=((source&31)*180*220+(dst&31)*35*255+32512)/65025; row[x]=WORD((r<<11)|(g<<5)|b);
        }
    }
    surface->Unlock(NULL);
}
void CNPCWnd::ShowNPCChatWnd()
{
    DrawBackground(); const int bottom=Height();
    DrawSelectionFrame(*m_pxWndImage,closePressed?1222:1221,346,bottom-42,SelectionBlend::Normal,false);
    if (scrollEnabled) {
        if (arrowFrame++==16) { arrowFrame=1; downLit=!downLit; }
        DrawSelectionFrame(*m_pxWndImage,arrowPressed==-1?1358:1357,183,bottom-48,SelectionBlend::Normal,false);
        if (first<scrollLimit && downLit) DrawSelectionFrame(*m_pxWndImage,arrowPressed==1?1356:1355,183,bottom-30,SelectionBlend::Normal,false);
    }
    HDC dc; auto* surface=g_xMainWnd.GetBackBuffer(); if (FAILED(surface->GetDC(&dc))) return;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0}; HFONT normal=g_xMainWnd.CreateGameFont(face,9);
    LOGFONTA desc={}; GetObjectA(normal,sizeof(desc),&desc); desc.lfUnderline=TRUE; HFONT link=CreateFontIndirectA(&desc);
    auto old=SelectObject(dc,normal); SetBkMode(dc,TRANSPARENT); const int focused=hovered?OptionAt(pointer):-1;
    for (const auto& row:rows) if (row.line>=first && row.line<=first+21) {
        const bool clickable=!row.command.empty(); SelectObject(dc,clickable && focused==row.option?link:normal);
        SetTextColor(dc,clickable?(pressed==row.option?0x0000ff:0x00ffff):row.color);
        const RECT box=TextBounds(row); const int saved=SaveDC(dc); IntersectClipRect(dc,box.left,box.top,box.right,box.bottom);
        TextOutA(dc,box.left,box.top,row.text.data(),int(row.text.size())); RestoreDC(dc,saved);
    }
    SelectObject(dc,old); DeleteObject(normal); DeleteObject(link); surface->ReleaseDC(dc);
}
