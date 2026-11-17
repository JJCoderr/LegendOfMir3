#pragma once
namespace LoginLayout {
constexpr int LeaveDuration=2000;
constexpr int Width=640, Height=480, InputBytes=24;
constexpr int Background=0, Fields=2, LoginButton=10, RegisterButton=12, PasswordButton=14, ExitButton=16;
constexpr int MonthlyLastDay=102, MonthlyDays=103, HourlyRemaining=104, HourlyLast=105, PointsRemaining=106, CreditRemaining=107;
constexpr int InvalidAccount=112, InvalidPassword=113, Disconnected=121, LoadCaption=125, HelpCaption=129, ExitUrl=1077;
constexpr POINT Buttons[]={{459,436},{139,379},{279,379},{439,379}};
constexpr int Frames[]={LoginButton,RegisterButton,PasswordButton,ExitButton};
constexpr RECT Inputs[]={{128,440,227,454},{326,440,425,454}};
constexpr RECT ButtonBar={0,372,640,420};
constexpr char ClickSound[]="105.wav";
}
