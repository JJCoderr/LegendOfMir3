add_library(Def STATIC
    EnDecode.cpp
    Misc.cpp
    RegstryHandler.cpp
    ServerSockHandler.cpp
    dbmgr.cpp
    ModernUI.cpp

    _OrzEx/database.cpp
    _OrzEx/error.cpp
    _OrzEx/prime.cpp
    _OrzEx/stringex.cpp
    _OrzEx/syncobj.cpp
    _OrzEx/util.cpp
)

target_include_directories(Def PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/_OrzEx
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_definitions(Def PRIVATE UNICODE _UNICODE)

target_link_libraries(Def PUBLIC ws2_32 advapi32 odbc32 odbccp32)

# List.cpp / Queue.cpp / DynamicArray.cpp �?VC6 遗留实现，与现行头文件冲突，不编�?
