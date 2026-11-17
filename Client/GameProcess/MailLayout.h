#pragma once

// MIR3: 457F70 creates controls, 458640 dispatches the five views.
namespace MailLayout {
enum class View { Inbox, Read, Compose, Gift, Outbox };
enum class Action { Close, Delete, Refresh, Outbox, Gift, Compose, Read, Inbox,
    DeleteRead, Reply, Next, Previous, CancelCompose, SendCompose, CancelGift,
    SendGift, CancelOutbox };
struct Button { Action action; int normal,pressed,x,y,caption; };
constexpr Button Buttons[]={
    {Action::Close,1221,1222,400,267,1052},
    {Action::Delete,1962,1962,305,256,4600},
    {Action::Refresh,1964,1964,266,256,4601},
    {Action::Outbox,1980,1980,363,14,4602},
    {Action::Gift,1982,1982,318,14,4603},
    {Action::Compose,1984,1984,273,14,4604},
    {Action::Read,1986,1986,228,14,4605},
    {Action::Inbox,1988,1988,183,14,4606},
    {Action::DeleteRead,2032,2032,326,256,4607},
    {Action::Reply,2016,2016,286,256,4612},
    {Action::Next,2012,2012,114,257,4608},
    {Action::Previous,2014,2014,54,257,4609},
    {Action::CancelCompose,2032,2032,326,256,4610},
    {Action::SendCompose,2034,2034,286,256,4611},
    {Action::CancelGift,2052,2052,326,256,4610},
    {Action::SendGift,2054,2054,286,256,4611},
    {Action::CancelOutbox,2032,2032,286,256,4610}
};
constexpr int WindowFrame=1960, WindowWidth=440, WindowHeight=310, ReadFrameBackground=2010;
constexpr int ItemRecordBytes=100;
constexpr int VisibleRows=7, RowHeight=24, EditLimit=4000;
constexpr int ScrollX=410, ScrollY=52, ScrollTravel=150, HeaderHeight=50;
constexpr int BodyWidth=355, BodyRows=11, BodyLineHeight=12, BodyX=29, BodyY=89;
constexpr int CanvasWidth=800, CanvasHeight=600, FontPixels=12;
constexpr int ScrollFrame=1966, UnreadFrame=1970, ReadFrame=1971, AttachmentFrame=1972, AttachmentReadFrame=1973, RowMarkerFrame=1974;
constexpr int ComposeFrame=2030, GiftFrame=2050, OutboxFrame=2070;
constexpr int InboxTitle=4618, ReadTitle=4619, ComposeTitle=4620, GiftTitle=4617, OutboxTitle=4621;
constexpr unsigned MaximumGiftGold=100000000;
constexpr int GoldDialogAction=13, GoldCaption=1050;
constexpr int GiftCenterX=315, GiftCenterY=138, GiftMaxHeight=170, GiftCroppedHeight=130;
constexpr int GiftCroppedTop=78, GiftHintX=25, GiftHintY=22, AttachmentHintOffset=15;
constexpr int GiftBorderX=15, GiftBorderY=7;
constexpr unsigned GoldHintTextColor=64250, GoldHintBorderColor=3315350;
constexpr int GiftInstruction=4614, GiftDropInstruction=4615, GoldQuestion=4616, NewMessage=4622;
constexpr unsigned TitleColor=16309498, TextColor=16448250;
struct Box { int left,top,right,bottom; };
constexpr Box Title={34,15,176,35};
constexpr Box Recipient={68,62,216,77};
constexpr Box ComposeBody={21,84,394,225};
constexpr Box GiftBody={21,84,214,225};
constexpr Box Attachment={230,60,250,80};
constexpr Box GiftItem={234,54,396,225};
constexpr Box SubjectHit={138,59,358,80};
constexpr Box CheckBox={22,63,35,76};
constexpr Box FirstRow={43,59,360,80};
}
