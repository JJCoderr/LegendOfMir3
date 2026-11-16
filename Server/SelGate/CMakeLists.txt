add_executable(SelGate WIN32 
    ClientSockMsg.cpp
    ConfigDlgFunc.cpp
    MainWndProc.cpp
    SelGate.cpp
    ServerSockMsg.cpp
    StdAfx.cpp
    ThreadFuncForComm.cpp
    ThreadFuncForMsg.cpp
    SelGate.rc
    # IOCPFunc.cpp
    # LoginGate.cpp

    ../Def/Misc.cpp
    ../Def/RegstryHandler.cpp
    ../Def/ServerSockHandler.cpp
    ../Def/_OrzEx/syncobj.cpp
)

target_include_directories(SelGate PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../Def
    ${CMAKE_CURRENT_SOURCE_DIR}/../Def/_OrzEx
)
