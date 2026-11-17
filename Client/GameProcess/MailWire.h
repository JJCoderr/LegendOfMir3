#pragma once
namespace MailWire {
constexpr int ListMessage=1356, BodyMessage=1357, NotificationMessage=1358;
constexpr int Inbox=0, Outbox=2;
constexpr int NotificationFrame=2072, NotificationCaption=4622;
constexpr int HudFrame=1160,HudBottom=601,NotificationX=757,NotificationOffsetY=-20;
constexpr int HeaderBytes=80, BodyBytes=613;
constexpr int Id=0, Kind=4, ReadState=6, Person=8, Subject=33, Date=64;
constexpr int PersonBytes=25, SubjectBytes=31, DateBytes=16;
constexpr int Gold=8, Item=12, Text=112, TextBytes=501;
}
