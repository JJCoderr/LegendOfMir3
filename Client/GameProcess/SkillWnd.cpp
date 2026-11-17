#include "StdAfx.h"
#include "Common/LegacyPanel.h"
#include "Common/LoginMessages.h"

void CSkillWnd::Init()
{
    CGameWnd::Init();
    m_keyMenu={}; m_keyMenuTarget=0;m_shortcuts.fill(0);
    m_descriptionLoaded=false; m_showDescription=false; m_descriptionText.clear(); m_descriptionRows.clear();
    m_skillTab=0; m_skillScroll=0; m_skillPressed=-1; m_skillScrollCaptured=false; m_skillScrollTime=0;
    for (int i=0;i<8;++i) m_skillSelected[i]=-1;
}
void CSkillWnd::Destroy()
{
    delete[] m_pstMyMagic; m_pstMyMagic=nullptr; m_bMyMagicCnt=0;
    CGameWnd::Destroy();
}
void CSkillWnd::SelectNonempty()
{
    int first=8;
    for (int i=0;i<m_bMyMagicCnt;++i) {
        const int category=SkillLayout::category(m_pstMyMagic[i].stStdMagic.wMagicID);
        if (category==m_skillTab) return;
        first=min(first,category);
    }
    if (first<8) m_skillTab=first;
}
BOOL CSkillWnd::OnLButtonDown(POINT point)
{
    if (PtInRect(&m_keyMenu,point)) return TRUE;
    if (SkillScrollInput(0,point)) return TRUE;
    m_skillPressed=SkillButtonAt(point);
    if (m_skillPressed>=0) return TRUE;
    const int skill=SkillAt(point);
    if (skill>=0) {
        const int slot=SkillLayout::slot(m_skillTab,m_pstMyMagic[skill].stStdMagic.wMagicID);
        m_skillSelected[m_skillTab]=slot<0?15:slot;
    }
    return FALSE;
}
BOOL CSkillWnd::OnLButtonUp(POINT point)
{
    if (PtInRect(&m_keyMenu,point)) {
        for (int i=0;i<m_bMyMagicCnt;++i) if (m_pstMyMagic[i].stStdMagic.wMagicID==m_keyMenuTarget) {
            const int key=(point.y-m_keyMenu.top)/40*4+(point.x-m_keyMenu.left)/40+1;
            BindMagic(m_keyMenuTarget,LegacyKey(key)); break;
        }
        return FALSE;
    }
    if (SkillScrollInput(2,point)) return FALSE;
    const int button=SkillButtonAt(point);
    const bool clicked=button>=0 && button==m_skillPressed; m_skillPressed=-1;
    if (clicked) {
        g_xSound.PlayLobbyEffect("105.wav");
        if (!button) return TRUE;
        m_skillTab=button-1;
    }
    return FALSE;
}

BOOL CSkillWnd::SetMagicKey(POINT point,BYTE key)
{
    if(!m_bActive || !PtInRect(&m_rcWnd,point)) return FALSE;
    for(const auto pos:SkillLayout::positions[m_skillTab]) {
        const RECT hit={m_rcWnd.left+pos.x,m_rcWnd.top+pos.y-m_skillScroll,m_rcWnd.left+pos.x+40,m_rcWnd.top+pos.y-m_skillScroll+40};
        if(PtInRect(&hit,point)) BindMagic(pos.id,key);
    }
    return TRUE;
}

void CSkillWnd::BindMagic(int id,BYTE key)
{
    const int targetSlot=KeyNumber(key);if(targetSlot) m_shortcuts[targetSlot-1]=0;
    for (int i=0;i<m_bMyMagicCnt;++i) if (m_pstMyMagic[i].bUseKey==key) {
        m_pstMyMagic[i].bUseKey=0;
        g_xClientSocket.SendMagicKeyChange(0,m_pstMyMagic[i].stStdMagic.wMagicID);
    }
    for (int i=0;i<m_bMyMagicCnt;++i) if (m_pstMyMagic[i].stStdMagic.wMagicID==id) {
        const int old=KeyNumber(m_pstMyMagic[i].bUseKey);if(old && m_shortcuts[old-1]==BYTE(id)) m_shortcuts[old-1]=0;
        m_pstMyMagic[i].bUseKey=key;
        const int slot=KeyNumber(key);if(slot) m_shortcuts[slot-1]=BYTE(id);
        g_xClientSocket.SendMagicKeyChange(key,id); break;
    }
}

LPCLIENTMAGICRCD CSkillWnd::GetMagicByKey(BYTE bKey)
{
    if (!bKey) return NULL;
    for (INT index=0; index<m_bMyMagicCnt; ++index)
        if (m_pstMyMagic[index].bUseKey==bKey) return &m_pstMyMagic[index];
    return NULL;
}

LPCLIENTMAGICRCD CSkillWnd::GetMagicByShortcut(int slot)
{
    if(slot<1 || slot>SkillWire::ShortcutCount) return nullptr;
    const BYTE id=m_shortcuts[slot-1];if(!id) return nullptr;
    for(int i=0;i<m_bMyMagicCnt;++i) if(m_pstMyMagic[i].stStdMagic.wMagicID==id) return &m_pstMyMagic[i];
    return nullptr;
}
BOOL CSkillWnd::OnKeyDown(WPARAM key, LPARAM flags, POINT point)
{
    const int slot=PhysicalKeySlot(key);if(!slot) return FALSE;
    if(flags & (1L<<30)) return m_bActive && PtInRect(&m_rcWnd,point);
    return SetMagicKey(point,LegacyKey(slot));
}

VOID CSkillWnd::OnScrollDown()
{
    m_skillScroll=max(0,m_skillScroll-30);
}

VOID CSkillWnd::OnScrollUp()
{
    m_skillScroll=min(190,m_skillScroll+30);
}

int CSkillWnd::SkillButtonAt(POINT point)
{
    for (int index=0;index<=8;++index) {
        const int frame=index?1640+2*(index-1):1221;
        if (!m_pxWndImage->NewSetIndex(frame)) continue;
        const auto& info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        const int x=m_rcWnd.left+(index?46+34*(index-1):316),y=m_rcWnd.top+(index?30:422);
        RECT bounds={x,y,x+info.shWidth,y+info.shHeight};
        if (PtInRect(&bounds,point)) return index;
    }
    return -1;
}

int CSkillWnd::SkillAt(POINT point)
{
    const int x=point.x-m_rcWnd.left,y=point.y-m_rcWnd.top;
    if (y<64 || y>=395) return -1;
    int ordinal=0;
    for (int i=0;i<m_bMyMagicCnt;++i) {
        const int id=m_pstMyMagic[i].stStdMagic.wMagicID;
        if (!id || SkillLayout::category(id)!=m_skillTab) continue;
        if (ordinal>=15) break;
        const int slot=SkillLayout::slot(m_skillTab,id);
        const auto pos=slot>=0?SkillLayout::positions[m_skillTab][slot]:SkillPosition{id,ordinal<6?55:0,ordinal<6?27+46*ordinal:0};
        const int top=pos.y-(slot>=0?m_skillScroll:0); ++ordinal;
        if (x>=pos.x && x<pos.x+40 && y>=top && y<top+40) return i;
    }
    return -1;
}

bool CSkillWnd::SkillScrollInput(int kind,POINT point)
{
    const DWORD now=GetTickCount();
    if (kind==2) { m_skillScrollTime=now; const bool captured=m_skillScrollCaptured; m_skillScrollCaptured=false; return captured; }
    if (!m_pxWndImage->NewSetIndex(1672)) return false;
    const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
    const int x=point.x-m_rcWnd.left,y=point.y-m_rcWnd.top;
    if (kind==1) {
        if (!m_skillScrollCaptured) return false;
        m_skillScroll=int(190.f*max(0.f,min(1.f,float(y-100)/(238+info.shHeight)))); return true;
    }
    if (x<322 || x>=322+info.shWidth || y<100 || y>=338+info.shHeight) return false;
    float ratio=float(double(m_skillScroll)/189.0);
    if (now-m_skillScrollTime>10) {
        const int thumb=100+int(238.f*ratio);
        if (y>=thumb && y<thumb+info.shHeight) m_skillScrollCaptured=true;
        else ratio=max(0.f,min(1.f,ratio+(y<thumb+7?-7.f/190:7.f/190)));
        m_skillScrollTime=now;
    }
    m_skillScroll=min(189,int(190.f*ratio)); return true;
}

VOID CSkillWnd::ShowSkillWnd()
{
    const int x=m_rcWnd.left,y=m_rcWnd.top;
    const auto sprite=[&](CWHWilImageData& images,int frame,int left,int top,int cropTop=-1,int cropHeight=0) {
        if (!images.NewSetIndex(frame)) return;
        const auto info=*images.m_lpstNewCurrWilImageInfo;
        if (cropTop>=0) {
            const int height=min(cropHeight,int(info.shHeight)-cropTop); if (height<=0) return;
            RECT clip={x+left,y+top,x+left+info.shWidth,y+top+height};
            DrawSelectionFrame(images,frame,x+left,y+top-cropTop,SelectionBlend::Normal,false,0,&clip);
        } else g_xMainWnd.DrawWithImageForComp(x+left,y+top,info.shWidth,info.shHeight,(WORD*)images.m_pbCurrImage);
    };
    if (m_pxWndImage->NewSetIndex(1620)) {
        const auto info=*m_pxWndImage->m_lpstNewCurrWilImageInfo;
        sprite(*m_pxWndImage,1620,-(info.shWidth-360)/2,-(info.shHeight-488)/2);
    }
    sprite(*m_pxWndImage,1622+2*m_skillTab,18,63,m_skillScroll,331);
    sprite(*m_pxWndImage,1672,322,100+int(238.f*float(double(m_skillScroll)/189.0)));
    bool populated[8]={}; int ordinal=0,selected=-1;
    struct Label { int index,x,y; }; Label labels[15]; int labelCount=0;
    for (int i=0;i<m_bMyMagicCnt;++i) {
        const auto& skill=m_pstMyMagic[i]; const int id=skill.stStdMagic.wMagicID;
        if (!id) continue;
        const int category=SkillLayout::category(id); populated[category]=true;
        if (category!=m_skillTab || ordinal>=15) continue;
        const int slot=SkillLayout::slot(m_skillTab,id);
        const auto pos=slot>=0?SkillLayout::positions[m_skillTab][slot]:SkillPosition{id,ordinal<6?55:0,ordinal<6?27+46*ordinal:0};
        const int top=pos.y-(slot>=0?m_skillScroll:0); ++ordinal;
        if (m_skillSelected[m_skillTab]==(slot<0?15:slot)) {
            selected=i;
            if (top-1>62 && top-1<352) {
                RECT edge={x+pos.x-1,y+top-1,x+pos.x+41,y+top+41};
                DrawLegacyPanel(edge,RGB(0,0,0),0,RGB(200,200,0));
            }
        }
        const auto icon=[&](CWHWilImageData& images,int frame) {
            if (!images.NewSetIndex(frame)) return;
            const int hiddenTop=max(0,64-top),hiddenBottom=max(0,top-355);
            const int height=images.m_lpstNewCurrWilImageInfo->shHeight-hiddenTop-hiddenBottom;
            if (height>0) sprite(images,frame,pos.x,top+hiddenTop,hiddenTop,height);
        };
        icon(g_xGameProc.m_xImage.m_xImageList[_IMAGE_MICON],2*id-2);
        if (const int key=KeyNumber(skill.bUseKey)) icon(*m_pxWndImage,1659+key);
        if (top+37>56 && top+37<385) labels[labelCount++]={i,pos.x+34,top+37};
    }
    for (int order=0;order<=8;++order) {
        const int index=order?9-order:0,base=index?1640+2*(index-1):1221;
        const int left=index?46+34*(index-1):316,top=index?30:422;
        if (!index || m_skillPressed==index || populated[index-1]) sprite(*m_pxWndImage,base+(!index && m_skillPressed==0),left,top);
        if (index && m_skillTab==index-1) sprite(*m_pxWndImage,base+1,left,top);
    }
    HDC dc; auto* surface=g_xMainWnd.GetBackBuffer(); if (FAILED(surface->GetDC(&dc))) return;
    const char normalFace[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    const char levelFace[]={char(0xb1),char(0xbc),char(0xb8),char(0xb2),0};
    HFONT normal=g_xMainWnd.CreateGameFont(normalFace,9),level=g_xMainWnd.CreateGameFont(levelFace,8);
    HGDIOBJ previous=SelectObject(dc,level); SetBkMode(dc,TRANSPARENT);
    const auto text=[&](const char* value,int left,int top,int width,int height,COLORREF color,bool center) {
        RECT area={x+left,y+top,x+left+width,y+top+height}; SetTextColor(dc,color);
        if (center) {
            RECT measured={0,0,width,height}; DrawTextA(dc,value,-1,&measured,DT_CALCRECT|DT_WORDBREAK);
            area.top+=height/2-(measured.bottom-measured.top)/2; area.bottom=area.top+measured.bottom-measured.top;
            DrawTextA(dc,value,-1,&area,DT_WORDBREAK);
        } else TextOutA(dc,area.left,area.top,value,lstrlenA(value));
    };
    char value[80];
    for (int i=0;i<labelCount;++i) {
        const auto label=labels[i]; wsprintfA(value,"%u",m_pstMyMagic[label.index].bLevel);
        text(value,label.x,label.y,15,15,0x64fafa,false);
    }
    SelectObject(dc,normal);
    if (selected>=0) {
        const auto& skill=m_pstMyMagic[selected];
        const std::string name(skill.stStdMagic.szMagicName,strnlen(skill.stStdMagic.szMagicName,sizeof(skill.stStdMagic.szMagicName)));
        text(name.c_str(),20,387,80,61,0xfafafa,true);
        if (skill.bLevel>=3) lstrcpyA(value,"Level : Max"); else wsprintfA(value,"Level : %u",skill.bLevel);
        text(value,20,403,80,62,0xfafafa,true);
        if (skill.bLevel<3) {
            wsprintfA(value,"Exp : %u/%d",skill.nCurrTrain,skill.stStdMagic.nTrain[skill.bLevel]);
            text(value,100,403,80,62,0xfafafa,true);
        }
    }
    SelectObject(dc,previous); DeleteObject(normal); DeleteObject(level); surface->ReleaseDC(dc);
    ShowKeyMenu();
    ShowDescription();
}

bool CSkillWnd::OpenKeyMenu(POINT point)
{
    const auto& positions=SkillLayout::positions[m_skillTab];
    for (int slot=0;slot<10;++slot) {
        const auto pos=positions[slot];
        RECT hit={m_rcWnd.left+pos.x,m_rcWnd.top+pos.y,m_rcWnd.left+pos.x+40,m_rcWnd.top+pos.y+40};
        if (!pos.id || !PtInRect(&hit,point)) continue;
        bool learned=false;
        for (int i=0;i<m_bMyMagicCnt;++i) if (m_pstMyMagic[i].stStdMagic.wMagicID==pos.id) learned=true;
        if (!learned) continue;
        const int left=min(point.x,m_rcWnd.right-160);
        const int top=point.y+160>m_rcWnd.right?m_rcWnd.bottom-120:point.y;
        m_keyMenu={left,top+40,left+160,top+160};
        const int selected=m_skillSelected[m_skillTab];
        m_keyMenuTarget=selected>=0 && selected<15?positions[selected].id:0;
        m_skillPressed=-1; return true;
    }
    return false;
}

void CSkillWnd::ShowKeyMenu()
{
    if (IsRectEmpty(&m_keyMenu)) return;
    DrawLegacyPanel(m_keyMenu,RGB(100,100,50),150,0,false);
    for (int index=0;index<12;++index) {
        const int x=m_keyMenu.left+40*(index%4),y=m_keyMenu.top+40*(index/4);
        if (const int id=Shortcut(index)) {
            if (id) DrawSelectionFrame(g_xGameProc.m_xImage.m_xImageList[_IMAGE_MICON],2*id-2,x,y,SelectionBlend::Normal,false);
        }
        DrawSelectionFrame(*m_pxWndImage,1660+index,x,y,SelectionBlend::Normal,false);
    }
}

namespace {
std::string WrapDescription(HDC dc,const std::string& line,int& count)
{
    std::string result; int start=0,highBytes=0; count=1;
    for (int i=0;i<int(line.size());++i) {
        SIZE extent={}; GetTextExtentPoint32A(dc,line.data()+start,i-start+1,&extent);
        const bool high=static_cast<signed char>(line[i])<0;
        if (extent.cx<=165) { if (high) ++highBytes; result+=line[i]; }
        else if (high && highBytes%2) { result.back()='`'; start=i-1; i-=2; --highBytes; ++count; }
        else { result+='`'; start=i; --i; ++count; }
    }
    return result;
}
}

const std::vector<CSkillWnd::DescriptionRow>& CSkillWnd::DescriptionRows(int id)
{
    const auto cached=m_descriptionRows.find(id);
    if (cached!=m_descriptionRows.end()) return cached->second;
    if (!m_descriptionLoaded) { LoadLegacyText("./Magic.exp",m_descriptionText); m_descriptionLoaded=true; }
    auto& rows=m_descriptionRows[id];
    HDC dc=CreateCompatibleDC(NULL);
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    HFONT font=g_xMainWnd.CreateGameFont(face,9); HGDIOBJ old=SelectObject(dc,font);
    bool active=false; size_t start=0;
    while (start<m_descriptionText.size()) {
        const auto end=m_descriptionText.find('\r',start);
        if (end==std::string::npos || end+1>=m_descriptionText.size() || m_descriptionText[end+1]!='\n') break;
        const auto line=m_descriptionText.substr(start,end-start); start=end+2;
        if (!line.empty() && line[0]==';') continue;
        if (!line.empty() && line[0]=='#') {
            if (strtol(line.c_str()+1,NULL,10)==id) active=true;
            else if (active) break;
        } else if (active && line.size()<=1299) {
            int count=0; const auto wrapped=WrapDescription(dc,line,count); size_t pos=0; bool failed=false;
            for (int i=0;i<min(count,20);++i) {
                std::string row;
                if (!failed) {
                    const auto next=wrapped.find('`',pos);
                    const auto length=(next==std::string::npos?wrapped.size():next)-pos;
                    if (!length) failed=true;
                    else {
                        row=wrapped.substr(pos,min(length,size_t(259))); pos=next==std::string::npos?wrapped.size():next+1;
                        while (pos<wrapped.size() && (wrapped[pos]==' ' || wrapped[pos]=='\t' || wrapped[pos]=='\r' || wrapped[pos]=='\n')) ++pos;
                        if (next==std::string::npos) failed=true;
                    }
                }
                SIZE size={}; GetTextExtentPoint32A(dc,row.data(),int(row.size()),&size);
                rows.push_back({row,int(size.cx),!row.empty() && row[0]=='['});
            }
        }
    }
    SelectObject(dc,old); DeleteObject(font); DeleteDC(dc);
    return rows;
}

int CSkillWnd::DescriptionAt() const
{
    if (!m_showDescription || m_skillScrollCaptured || !IsRectEmpty(&m_keyMenu) || !PtInRect(&m_rcWnd,m_descriptionPointer)) return 0;
    for (const auto pos:SkillLayout::positions[m_skillTab]) {
        const RECT hit={m_rcWnd.left+pos.x,m_rcWnd.top+pos.y-m_skillScroll,m_rcWnd.left+pos.x+40,m_rcWnd.top+pos.y-m_skillScroll+40};
        if (pos.id && PtInRect(&hit,m_descriptionPointer)) return pos.id;
    }
    return 0;
}

void CSkillWnd::ShowDescription()
{
    const int id=DescriptionAt(); if (!id) return;
    const auto& rows=DescriptionRows(id); if (rows.empty()) return;
    int width=0; for (const auto& row:rows) if (row.width>width) width=row.width+7;
    const int x=m_descriptionPointer.x+190>800?630:m_descriptionPointer.x+20;
    int y=m_descriptionPointer.y+20;
    RECT box={x-2,y,x-2+width,y+int(rows.size())*15+30};
    DrawLegacyPanel(box,RGB(100,100,50),150,0,false);
    auto* surface=g_xMainWnd.GetBackBuffer(); HDC dc; if (FAILED(surface->GetDC(&dc))) return;
    const char normalFace[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};
    const char titleFace[]={char(0xb1),char(0xbc),char(0xb8),char(0xb2),0};
    HFONT normal=g_xMainWnd.CreateGameFont(normalFace,9),title=g_xMainWnd.CreateGameFont(titleFace,10,0,FW_BOLD);
    HGDIOBJ old=SelectObject(dc,normal); SetBkMode(dc,TRANSPARENT);
    for (const auto& row:rows) {
        y+=15; SelectObject(dc,row.title?title:normal);
        if (row.title) {
            SetTextColor(dc,0x0a0a0a);
            const int shadow[4][2]={{-1,-1},{1,-1},{-1,1},{1,1}};
            for (const auto& offset:shadow) TextOutA(dc,x+offset[0],y+offset[1],row.text.data(),int(row.text.size()));
        }
        SetTextColor(dc,row.title?0x96c8fa:0xfafafa); TextOutA(dc,x,y,row.text.data(),int(row.text.size()));
        if (row.title) y+=15;
    }
    SelectObject(dc,normal); SetTextColor(dc,0x1414fa);
    for (int i=0;i<m_bMyMagicCnt;++i) if (m_pstMyMagic[i].stStdMagic.wMagicID==id) {
        const auto& skill=m_pstMyMagic[i]; char value[64];
        if (skill.bLevel>=3) lstrcpyA(value,"Level : Max");
        else wsprintfA(value,"%u/%d",skill.nCurrTrain,skill.stStdMagic.nTrain[skill.bLevel]);
        RECT area={x,y,x+(skill.bLevel>=3?80:50),y+15}; DrawTextA(dc,value,-1,&area,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        break;
    }
    SelectObject(dc,old); DeleteObject(normal); DeleteObject(title); surface->ReleaseDC(dc);
}

void CSkillWnd::UpdateShortcut(const CLIENTMAGICRCD& record,bool full)
{
    char key[SkillWire::RecordSize-SkillWire::KeyOffset+1]={};
    memcpy(key,reinterpret_cast<const BYTE*>(&record)+SkillWire::KeyOffset,sizeof(key)-1);
    const long decimal=std::strtol(key,nullptr,10);
    const int label=decimal>0 && decimal<10?int(decimal):BYTE(record.bUseKey)-55;
    const int slot=full?label:BYTE(decimal);
    if(slot>=1 && slot<=SkillWire::ShortcutCount) m_shortcuts[slot-1]=BYTE(record.stStdMagic.wMagicID);
}
bool CSkillWnd::Receive(const _TDEFAULTMESSAGE& header,const char* encoded)
{
    static_assert(sizeof(CLIENTMAGICRCD)==SkillWire::RecordSize,"MIR3 skill record layout");
    if(header.wIdent==SM_MAGIC_LVEXP || header.wIdent==SkillWire::DeleteMessage) {
        for(int i=0;i<m_bMyMagicCnt;++i) if(m_pstMyMagic[i].stStdMagic.wMagicID==header.nRecog) {
            if(header.wIdent==SM_MAGIC_LVEXP) {
                m_pstMyMagic[i].nCurrTrain=DWORD(header.wTag)|(DWORD(header.wSeries)<<16);
                m_pstMyMagic[i].bLevel=LOBYTE(header.wParam);
            } else {
                std::move(m_pstMyMagic+i+1,m_pstMyMagic+m_bMyMagicCnt,m_pstMyMagic+i);--m_bMyMagicCnt;
                if(m_keyMenuTarget==header.nRecog) {m_keyMenu={};m_keyMenuTarget=0;}
                SelectNonempty();
            }
            return true;
        }
        return false;
    }
    const bool full=header.wIdent==SM_SENDMYMAGIC;
    if(!full && header.wIdent!=SM_ADDMAGIC) return false;
    const int count=full?LOBYTE(header.wSeries):1;
    if(!full && m_bMyMagicCnt==SkillWire::MaximumRecords) return false;
    std::vector<CLIENTMAGICRCD> incoming;incoming.reserve(count);
    const char* cursor=encoded;
    for(int i=0;i<count;++i) {
        const char* end=full?strchr(cursor,'/'):cursor+strlen(cursor);
        if(!end || end-cursor!=SkillWire::EncodedRecordSize) return false;
        const std::string segment(cursor,end);
        for(unsigned char c:segment) if(c<SkillWire::EncodingFirst || c>SkillWire::EncodingLast) return false;
        BYTE bytes[SkillWire::RecordSize+1]={};
        if(fnDecode6BitBuf(const_cast<char*>(segment.c_str()),reinterpret_cast<char*>(bytes),sizeof(bytes))!=SkillWire::RecordSize) return false;
        CLIENTMAGICRCD record;memcpy(&record,bytes,sizeof(record));if(!record.stStdMagic.wMagicID) return false;
        incoming.push_back(record);cursor=full?end+1:end;
    }
    const int oldCount=full?0:m_bMyMagicCnt,newCount=oldCount+count;
    auto* next=newCount?new CLIENTMAGICRCD[newCount]:nullptr;
    if(oldCount) std::copy(m_pstMyMagic,m_pstMyMagic+oldCount,next);
    for(int i=0;i<count;++i) {next[oldCount+i]=incoming[i];UpdateShortcut(incoming[i],full);}
    delete[] m_pstMyMagic;m_pstMyMagic=next;m_bMyMagicCnt=BYTE(newCount);
    const int previous=m_skillTab;
    if(full) {m_skillTab=0;m_keyMenu={};m_keyMenuTarget=0;}
    SelectNonempty();if(full && !newCount) m_skillTab=previous;
    return true;
}
