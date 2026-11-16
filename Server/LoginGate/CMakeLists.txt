add_executable(LoginGate WIN32
    ClientSockMsg.cpp
    ConfigDlgFunc.cpp
    LoginGate.cpp
    MainWndProc.cpp
    ServerSockMsg.cpp
    StdAfx.cpp
    ThreadFuncForComm.cpp
    ThreadFuncForMsg.cpp
    LoginGate.rc
    #IOCPFunc.cpp

    ../Def/Misc.cpp
    ../Def/RegstryHandler.cpp
    ../Def/ServerSockHandler.cpp
    ../Def/_OrzEx/syncobj.cpp
)

target_include_directories(LoginGate PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../Def
    ${CMAKE_CURRENT_SOURCE_DIR}/../Def/_OrzEx
)
