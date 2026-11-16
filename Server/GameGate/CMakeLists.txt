add_executable(GameGate WIN32 
    Abusive.cpp
    ClientSockMsg.cpp
    ConfigDlgFunc.cpp
    GameGate.cpp
    MainWndProc.cpp
    ServerSockMsg.cpp
    StdAfx.cpp
    ThreadFuncForComm.cpp
    ThreadFuncForMsg.cpp
    GameGate.rc

    ../Def/EnDecode.cpp
    ../Def/Misc.cpp
    ../Def/RegstryHandler.cpp
    ../Def/ServerSockHandler.cpp
    ../Def/_OrzEx/syncobj.cpp
    ../Def/DynamicArray.cpp
    ../Def/List.cpp
    ../Def/Queue.cpp
)

target_include_directories(GameGate PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../Def
    ${CMAKE_CURRENT_SOURCE_DIR}/../Def/_OrzEx
)
