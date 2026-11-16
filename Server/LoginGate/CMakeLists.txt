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
)

# 现代外观：清单已在 LoginGate.rc 里以 1 RT_MANIFEST "..\..\app.manifest" 嵌入，
# 这里关闭链接器自动生成的清单，避免同一个 exe 里出现两份 RT_MANIFEST 资源。
target_link_options(LoginGate PRIVATE "/MANIFEST:NO")
