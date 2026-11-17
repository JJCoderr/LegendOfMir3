#pragma once

// MIR3 43659C/4365C4 window creation, 454410/4567A0 controls,
// 4546F0/456960 final drawing positions (which differ from initial positions).
namespace ConsignmentLayout {
struct Box { int left,top,right,bottom; };
struct Window { int frame,x,y,width,height; };
constexpr Window Browser={1890,60,20,720,440};
constexpr Window Registration={1910,0,10,408,310};
constexpr int BrowserId=19, RegistrationId=20;
constexpr int VisibleRows=13, RowHeight=19, DescriptionLimit=4000;
constexpr unsigned TitleColor=16309498, TextColor=16448250;
enum class Action { Close, CancelListing, Buy, Register, SearchName, SearchSeller,
    Refresh, NextPage, PreviousPage, CancelRegistration, SubmitRegistration };
struct Button { Action action; int normal,pressed,x,y,caption; };
constexpr Button BrowserButtons[]={
    {Action::Close,1221,1222,680,396,1052},
    {Action::CancelListing,1908,1908,644,371,4400},
    {Action::Buy,1906,1906,564,371,4401},
    {Action::Register,1904,1904,514,371,4402},
    {Action::SearchName,1900,1900,413,361,4403},
    {Action::SearchSeller,1900,1900,413,391,4404},
    {Action::Refresh,1898,1898,123,362,4405},
    {Action::NextPage,1896,1896,73,361,4406},
    {Action::PreviousPage,1894,1894,23,361,4407}
};
constexpr Button RegistrationButtons[]={
    {Action::Close,1221,1222,369,254,1052},
    {Action::CancelRegistration,1908,1908,319,242,4501},
    {Action::SubmitRegistration,1904,1904,249,242,4502}
};
constexpr Box BrowserTitle={10,17,115,31};
constexpr Box FirstRow={14,84,468,102};
constexpr Box NameQuery={268,363,405,380};
constexpr Box SellerQuery={268,393,405,410};
constexpr Box RegistrationTitle={46,30,126,45};
constexpr Box Description={12,83,230,195};
constexpr Box PriceHit={19,235,172,282};
constexpr Box PriceText={61,250,150,265};
constexpr Box ItemArea={247,47,395,203};
constexpr int BrowserTitleText=4408, ServerColumn=4409, ItemColumn=4410,
    PriceColumn=4411, SellerColumn=4412, InsufficientGold=4413,
    CancelListingQuestion=4414, PurchaseQuestion=4415;
constexpr int PriceQuestion=4500, RegistrationTitleText=4503,
    ItemInstruction=4504, DropInstruction=4505, RegistrationQuestion=4506;
constexpr int ActivityFirstFrame=1920, ActivityFrames=11, BusyInterval=100, IdleInterval=150;
constexpr int QueryLimit=90, HeaderHeight=46;
constexpr unsigned MaximumPrice=0x7fffffff;
constexpr int CanvasWidth=800, CanvasHeight=600, FontPixels=12;
constexpr int PriceDialog=14, PurchaseDialog=15, CancelDialog=16, RegistrationDialog=17;
constexpr int GoldCaption=1050, ConfirmationFrame=1240;
constexpr int IconMaxHeight=170, BrowserIconCrop=145, RegistrationIconCrop=140;
constexpr int BrowserIconX=600, BrowserIconY=252, RegistrationIconX=316, RegistrationIconY=140;
constexpr int IconBorderX=15, IconBorderY=7;
constexpr int DescriptionWidth=217, DescriptionRows=9, DescriptionLineHeight=12;
constexpr Box BrowserDescription={491,53,708,161};
}
