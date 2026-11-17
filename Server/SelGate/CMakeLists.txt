if(MIR3_BUILD_EISERVER)
    add_library(SelGate SHARED)
    target_sources(SelGate PRIVATE ../EIServer/Module.cpp)
    target_compile_definitions(SelGate PRIVATE MIR3_EISERVER)
else()
    add_executable(SelGate WIN32)
endif()
target_sources(SelGate PRIVATE
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
)

# 现代外观：清单已在 SelGate.rc 里以 1 RT_MANIFEST "..\..\app.manifest" 嵌入，
# 这里关闭链接器自动生成的清单，避免同一个 exe 里出现两份 RT_MANIFEST 资源。
target_link_options(SelGate PRIVATE "/MANIFEST:NO")
