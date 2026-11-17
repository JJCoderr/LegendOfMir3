if(MIR3_BUILD_EISERVER)
    add_library(LoginSvr SHARED)
    target_sources(LoginSvr PRIVATE ../EIServer/Module.cpp)
    target_compile_definitions(LoginSvr PRIVATE MIR3_EISERVER)
else()
    add_executable(LoginSvr WIN32)
endif()
target_sources(LoginSvr PRIVATE
    AddSvrListFunc.cpp
    ConfigDlgFunc.cpp
    GateCommSockMsg.cpp
    GateInfo.cpp
    LoginSvr.cpp
    MainWndProc.cpp
    ServerListProc.cpp
    StatusDlg.cpp
    StdAfx.cpp
    ThreadFuncForMsg.cpp
    LoginSvr.rc
)

# 现代外观：清单已在 LoginSvr.rc 里以 1 RT_MANIFEST "..\..\app.manifest" 嵌入，
# 这里关闭链接器自动生成的清单，避免同一个 exe 里出现两份 RT_MANIFEST 资源。
target_link_options(LoginSvr PRIVATE "/MANIFEST:NO")
