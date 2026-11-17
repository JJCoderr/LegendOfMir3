#pragma once

// MIR3 window 18: 436576 creation, 46E6E0 rendering, 46EBA0 targeting.
namespace SiegeLayout {
constexpr int WindowId=18, Background=1590;
constexpr int Left=100, Top=100, Width=194, Height=285;
constexpr int TextColor=0x64c8f8, InnerColor=0x6464c8, OuterColor=3289850;
constexpr int CloseCaption=1052, AttackCaption=4300, WaitCaption=4301,
    MoveCaption=4302, RangeCaption=4303, LoadCaption=4304;
constexpr int AttackMessage=1120, MoveMessage=1121, WaitMessage=1122;
constexpr int ObjectPositionMessage=1312;
constexpr int ControlMessage=1350, StatusMessage=1351, ReleaseMessage=1353;
constexpr int TargetMarker=1599;
constexpr int HudBackground=1160,EntryNormal=1570,EntryPressed=1572,EntryCaption=2645;
constexpr int EntryX=757,EntryAboveHud=20,HudBottom=601;
constexpr RECT FirstValue={14,22,90,37}, SecondValue={100,22,180,37};
constexpr RECT MapArea={6,65,186,250};
constexpr int MapWidth=180, MapHeight=150;
constexpr int LoadDialog=18, LoadMessage=1123;
constexpr int FirstAmmoIcon=310, SecondAmmoIcon=311;
enum class Action { Close, Attack, Wait, Move, Load };
struct Button { Action action; int normal,pressed,x,y,caption; };
constexpr Button Buttons[]={
    {Action::Close,1221,1222,153,241,CloseCaption},
    {Action::Attack,1592,1592,17,231,AttackCaption},
    {Action::Wait,1594,1594,67,231,WaitCaption},
    {Action::Move,1596,1596,117,231,MoveCaption}
};
// Preserve the original float constants before the x87 double intermediates.
constexpr float CellScaleX=1.04f, CellScaleY=0.59f;
constexpr float CropScaleX=0.70f, CropScaleY=0.58f;
constexpr float InverseScaleX=0.9615384936332703f, InverseScaleY=1.6949152946472168f;
inline POINT Marker(POINT cell,POINT origin,POINT crop)
{
    return {int(double(cell.x)*CellScaleX+origin.x-double(crop.x)*CropScaleX-1.0),
            int(double(cell.y)*CellScaleY+origin.y-double(crop.y)*CropScaleY-1.0)};
}
inline POINT Target(POINT mouse,POINT origin,POINT crop)
{
    return {int((double(crop.x)*CropScaleX+mouse.x-origin.x)*InverseScaleX),
            int((double(crop.y)*CropScaleY+mouse.y-origin.y)*InverseScaleY)};
}
inline RECT Range(POINT center,int style,bool inner)
{
    if(style<=0) return {};
    const int x=style==1?(inner?55:80):(inner?28:50);
    const int y=style==1?(inner?34:50):(inner?16:29);
    return {center.x-x,center.y-y,center.x+x,center.y+y};
}
inline bool CanTarget(POINT mouse,const RECT& map,const RECT& inner,const RECT& outer)
{
    return PtInRect(&map,mouse) && PtInRect(&outer,mouse) && !PtInRect(&inner,mouse);
}
}
