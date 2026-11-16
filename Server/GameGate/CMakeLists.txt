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
    # 原工程未引用的遗留代码，且与 Def/Queue.h、Def/List.h 的现行实现冲突，暂不编译
    # ../Def/DynamicArray.cpp
    # ../Def/List.cpp
    # ../Def/Queue.cpp
)

target_include_directories(GameGate PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../Def
    ${CMAKE_CURRENT_SOURCE_DIR}/../Def/_OrzEx
)
