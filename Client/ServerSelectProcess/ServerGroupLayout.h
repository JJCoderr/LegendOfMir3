#pragma once
namespace ServerGroupLayout {
constexpr int Width=800,Height=600;
constexpr int Background=0,Glow=1,Exit=3,ExitDown=4,Row=7,RowHover=8,Marker=11,MarkerHover=12,MarkerSelected=13,Thumb=15;
constexpr int ExitX=760,ExitY=13,ScrollX=204,ScrollY=84,ScrollTravel=447;
constexpr int RowX=10,RowY=70,RowWidth=190,RowHeight=15,RowStep=25,ScrollPage=10;
constexpr DWORD ScrollDelay=10;
constexpr COLORREF LabelColor=15790320,TitleColor=16448250;
inline RECT RowBounds(int row) {return {RowX,RowY+RowStep*row,RowX+RowWidth,RowY+RowStep*row+RowHeight};}
}
