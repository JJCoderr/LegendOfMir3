#include "StdAfx.h"
#include "Common/LegacyPanel.h"
void CTargetDisplay::Init() { Destroy();monsters.NewLoad("./Data/MonImg.wil"); }
void CTargetDisplay::Destroy() {monsters.Destroy();shown=previous=changedAt=0;frame=badge=-1;attributes.clear();}
void CTargetDisplay::Focus(DWORD actor,DWORD now)
{
    if(actor) {shown=actor;if(actor!=previous) changedAt=now;previous=actor;}
    else if(now-changedAt>800) shown=0;
}
void CTargetDisplay::Remove(DWORD actor) {if(shown==actor) shown=0;if(previous==actor) previous=0;attributes.erase(actor);}
void CTargetDisplay::SetMonster(DWORD actor,BYTE type,WORD shape,bool excluded,int state,WORD hp,WORD maxHp,BYTE ratio,DWORD now)
{
    Focus(actor,now);if(!actor) return;human=false;special=shape>=530 && shape<=533;
    frame=type!=3 || excluded || shape==127 || shape==153?-1:(shape>=2000 || special?shape:shape&255);
    if(frame>=0 && !monsters.NewSetIndex(frame)) frame=1999;
    badge=frame<0 || special?-1:state==1?635:634;current=hp;maximum=maxHp;percent=ratio;
    attributes.try_emplace(actor);
}
void CTargetDisplay::SetHuman(DWORD actor,BYTE type,BYTE profession,DWORD now)
{
    if(!actor || type>1) return;Focus(actor,now);human=true;badge=-1;special=false;
    frame=profession<=2?650+2*profession+(type!=0):0;
}
bool CTargetDisplay::ApplyAttributes(DWORD actor,WORD category,WORD state,const BYTE* body,int length)
{
    if(!actor || length!=10 || ((actor!=shown || human) && attributes.find(actor)==attributes.end())) return false;
    auto& value=attributes[actor];value.category=category;value.state=state;value.icon=(signed char)body[0];value.element=body[2];
    for(int i=0;i<7;++i) value.modifiers[i]=(signed char)body[i+3];return true;
}
std::array<int,7> CTargetDisplay::Attributes::Order() const
{
    std::array<int,7> order={0,1,2,3,4,5,6};
    for(int end=6;end>0;--end) for(int i=end;i>0;--i) if(modifiers[order[i]]>modifiers[order[i-1]]) std::swap(order[i],order[i-1]);
    for(int i=0;i<7;++i) if(!modifiers[order[i]]) for(int next=i+1;next<7;++next) if(modifiers[order[next]]) {std::swap(order[i],order[next]);break;}
    return order;
}
std::string CTargetDisplay::VitalText(int part) const {return part==1?"/":current?std::to_string(part?maximum:current):"?";}
namespace {
void TargetText(const std::string& value,RECT box,bool centered)
{
    HDC dc;auto* surface=g_xMainWnd.GetBackBuffer();if(FAILED(surface->GetDC(&dc))) return;
    const char face[]={char(0xcb),char(0xce),char(0xcc),char(0xe5),0};HFONT font=g_xMainWnd.CreateGameFont(face,9);auto old=SelectObject(dc,font);
    const int saved=SaveDC(dc);IntersectClipRect(dc,box.left,box.top,box.right,box.bottom);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,0xe6e6e6);
    if(centered) DrawTextA(dc,value.data(),int(value.size()),&box,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    else TextOutA(dc,box.left,box.top,value.data(),int(value.size()));
    RestoreDC(dc,saved);SelectObject(dc,old);DeleteObject(font);surface->ReleaseDC(dc);
}
}
void CTargetDisplay::ShowPortrait(CWHWilImageData& portraits)
{
    if(!shown || frame<0) return;
    DrawSelectionFrame(human?portraits:monsters,frame,534,486,SelectionBlend::Normal,false);
    if(badge>=0) DrawSelectionFrame(portraits,badge,595,560,SelectionBlend::Normal,false);
}
void CTargetDisplay::ShowVitals(CWHWilImageData& portraits)
{
    if(!shown || human || frame<0 || !monsters.NewSetIndex(frame)) return;
    const RECT boxes[]={{532,570,562,585},{562,570,567,585},{565,570,595,585}};
    for(int i=0;i<3;++i) TargetText(VitalText(i),boxes[i],true);
    DrawSelectionFrame(portraits,660,533,582,SelectionBlend::Normal,false);
    if(portraits.NewSetIndex(661)) {
        const auto info=*portraits.m_lpstNewCurrWilImageInfo;const int width=min(BarWidth(),int(info.shWidth));
        if(width>0) {const RECT clip={534,583,534+width,583+info.shHeight};DrawSelectionFrame(portraits,661,534,583,SelectionBlend::Normal,false,0,&clip);}
    }
}
void CTargetDisplay::ShowInformation(CWHWilImageData& portraits)
{
    if(!shown || human || special || frame<0) return;
    DrawLegacyPanel(RECT{615,487,720,572},RGB(50,50,50),230,RGB(120,120,120));
    const auto found=attributes.find(shown);if(found==attributes.end()) return;const auto& value=found->second;
    if(value.category==-1 || value.state==-1 || value.icon==-1) return;
    const auto icon=[&](int id,int x,int y) {DrawSelectionFrame(portraits,id,x,y,SelectionBlend::Normal,false);};
    icon(value.category==0?632:631,620,490);if(value.element) icon(599+value.element,641,490);icon(620+value.icon,value.element?662:641,490);
    int slot=0;for(int index:value.Order()) {
        const int amount=value.modifiers[index];if(!amount) break;const int x=620+32*(slot%3),y=510+20*(slot/3);
        icon(610+index,x,y);TargetText(std::string(amount>0?" +":" ")+std::to_string(amount)+"%",RECT{x+15,y+5,x+55,y+23},false);++slot;
    }
}
