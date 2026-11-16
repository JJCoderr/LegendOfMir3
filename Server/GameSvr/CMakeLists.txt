add_executable(GameSvr WIN32
    ConfigDlgFunc.cpp
    GameSvr.cpp
    GateInfo.cpp
    Global.cpp
    Handler_CommonDB.cpp
    Handler_Item.cpp
    Handler_Magic.cpp
    Handler_Map.cpp
    MainWndProc.cpp
    ObjectEngine.cpp
    ObjectEvent.cpp
    ObjectMerchant.cpp
    ObjectMonster.cpp
    ObjectNPC.cpp
    ObjectPlayer.cpp
    ObjectPlayerAdmin.cpp
    ObjectPlayerAdminSaid.cpp
    ObjectPlayerClone.cpp
    ObjectPlayerSaid.cpp
    ObjectScripter.cpp
    ProcessEvents.cpp
    ProcessLogin.cpp
    ProcessMerchants.cpp
    ProcessMonster.cpp
    ProcessNPC.cpp
    ProcessUserHuman.cpp
    ProcessUserMsg.cpp
    SockMsg_DBSrv.cpp
    SockMsg_GateComm.cpp
    SockMsg_LogSvr.cpp
    StdAfx.cpp
    ThreadFuncForComm.cpp
    UserInfo.cpp
    GameSvr.rc
)

# 现代外观：清单已在 GameSvr.rc 里以 1 RT_MANIFEST "..\..\app.manifest" 嵌入，
# 这里关闭链接器自动生成的清单，避免同一个 exe 里出现两份 RT_MANIFEST 资源。
target_link_options(GameSvr PRIVATE "/MANIFEST:NO")
