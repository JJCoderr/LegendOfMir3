if(MIR3_BUILD_EISERVER)
    add_library(DBSvr SHARED)
    target_sources(DBSvr PRIVATE ../EIServer/Module.cpp)
    target_compile_definitions(DBSvr PRIVATE MIR3_EISERVER)
else()
    add_executable(DBSvr WIN32)
endif()
target_sources(DBSvr PRIVATE
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
)

# 现代外观：清单已在 DBSvr.rc 里以 1 RT_MANIFEST "..\..\app.manifest" 嵌入，
# 这里关闭链接器自动生成的清单，避免同一个 exe 里出现两份 RT_MANIFEST 资源。
target_link_options(DBSvr PRIVATE "/MANIFEST:NO")
