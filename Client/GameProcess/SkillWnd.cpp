#include "StdAfx.h"
#include "Common/LegacyPanel.h"

void CSkillWnd::Init()
{
    CGameWnd::Init();
    m_keyMenu={}; m_keyMenuTarget=0;
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
    const int index=SkillAt(point);
    if (index<0) return FALSE;
    BindMagic(m_pstMyMagic[index].stStdMagic.wMagicID,key);
    return TRUE;
}

void CSkillWnd::BindMagic(int id,BYTE key)
{
    for (int i=0;i<m_bMyMagicCnt;++i) if (m_pstMyMagic[i].bUseKey==key) {
        m_pstMyMagic[i].bUseKey=0;
        g_xClientSocket.SendMagicKeyChange(0,m_pstMyMagic[i].stStdMagic.wMagicID);
    }
    for (int i=0;i<m_bMyMagicCnt;++i) if (m_pstMyMagic[i].stStdMagic.wMagicID==id) {
        m_pstMyMagic[i].bUseKey=key;
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

BOOL CSkillWnd::OnKeyDown(WPARAM wParam, LPARAM lParam, POINT ptMouse)
{
    if (SkillAt(ptMouse)<0) return FALSE;
    if (wParam>=VK_F1 && wParam<=VK_F8) return SetMagicKey(ptMouse,BYTE('1'+wParam-VK_F1));
    return FALSE;
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
        text(skill.stStdMagic.szMagicName,20,387,80,61,0xfafafa,true);
        if (skill.bLevel>=3) lstrcpyA(value,"Level : Max"); else wsprintfA(value,"Level : %u",skill.bLevel);
        text(value,20,403,80,62,0xfafafa,true);
        if (skill.bLevel<3) {
            wsprintfA(value,"Exp : %d/%d",skill.nCurrTrain,skill.stStdMagic.nTrain[skill.bLevel]);
            text(value,100,403,80,62,0xfafafa,true);
        }
    }
    SelectObject(dc,previous); DeleteObject(normal); DeleteObject(level); surface->ReleaseDC(dc);
    ShowKeyMenu();
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
        if (const auto skill=GetMagicByKey(LegacyKey(index+1))) {
            const int id=skill->stStdMagic.wMagicID&255;
            if (id) DrawSelectionFrame(g_xGameProc.m_xImage.m_xImageList[_IMAGE_MICON],2*id-2,x,y,SelectionBlend::Normal,false);
        }
        DrawSelectionFrame(*m_pxWndImage,1660+index,x,y,SelectionBlend::Normal,false);
    }
}
