add_executable(DBSvr WIN32 
    ConfigDlgFunc.cpp
    DBSvr.cpp
    GameGateSockMsg.cpp
    GateCommSockMsg.cpp
    GateInfo.cpp
    GlobalUserList.cpp
    MainWndProc.cpp
    ProcessDBMsg.cpp
    ProcessGateMsg.cpp
    ProcessServerMsg.cpp
    ServerCommSockMsg.cpp
    StdAfx.cpp
    TableList.cpp
    DBSvr.rc

    ../Def/EnDecode.cpp
    ../Def/Misc.cpp
    ../Def/RegstryHandler.cpp
    ../Def/ServerSockHandler.cpp
    ../Def/_OrzEx/syncobj.cpp
    ../Def/dbmgr.cpp
    ../Def/_OrzEx/database.cpp
    ../Def/_OrzEx/error.cpp
    ../Def/_OrzEx/prime.cpp
    ../Def/_OrzEx/stringex.cpp
    ../Def/_OrzEx/util.cpp
)

target_include_directories(DBSvr PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../Def
    ${CMAKE_CURRENT_SOURCE_DIR}/../Def/_OrzEx
)
