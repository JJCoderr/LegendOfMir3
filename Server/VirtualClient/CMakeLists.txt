add_executable(VirtualClient WIN32
    DlgFunc.cpp
    InitClient.cpp
    StdAfx.cpp
    VirtualClient.cpp
    VirtualClient.rc

    ../Def/EnDecode.cpp
    ../Def/Misc.cpp
)

target_include_directories(VirtualClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../Def
    ${CMAKE_CURRENT_SOURCE_DIR}/../Def/_OrzEx
)
