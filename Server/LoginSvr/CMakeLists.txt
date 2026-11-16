add_executable(LoginSvr WIN32 
    AddSvrListFunc.cpp
    ConfigDlgFunc.cpp
    GateCommSockMsg.cpp
    GateInfo.cpp
    LoginSvr.cpp
    MainWndProc.cpp
    ServerListProc.cpp
    StdAfx.cpp
    ThreadFuncForMsg.cpp
    LoginSvr.rc

    ../Def/EnDecode.cpp
    ../Def/Misc.cpp
    ../Def/RegstryHandler.cpp
    ../Def/ServerSockHandler.cpp
    ../Def/_OrzEx/syncobj.cpp
    ../Def/dbmgr.cpp
    ../Def/_OrzEx/database.cpp
)

target_include_directories(LoginSvr PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../Def
    ${CMAKE_CURRENT_SOURCE_DIR}/../Def/_OrzEx
)

# OrzEx 数据库容器直接调用 ODBC API，需要显式链接
target_link_libraries(LoginSvr PRIVATE odbc32 odbccp32)
