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

    ../Def/EnDecode.cpp
    ../Def/Misc.cpp
    ../Def/RegstryHandler.cpp
    ../Def/ServerSockHandler.cpp
    ../Def/_OrzEx/syncobj.cpp
    ../Def/_OrzEx/database.cpp
)

target_include_directories(GameSvr PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../Def
    ${CMAKE_CURRENT_SOURCE_DIR}/../Def/_OrzEx
)

# OrzEx 数据库容器直接调用 ODBC API，需要显式链接
target_link_libraries(GameSvr PRIVATE odbc32 odbccp32)
