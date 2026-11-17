add_library(mir_sqlite STATIC sqlite/sqlite3.c)
set_target_properties(mir_sqlite PROPERTIES COMPILE_OPTIONS "")
target_compile_definitions(mir_sqlite PRIVATE SQLITE_THREADSAFE=1)

add_library(Def STATIC
    EnDecode.cpp
    Misc.cpp
    RegstryHandler.cpp
    ServerSockHandler.cpp
    dbmgr.cpp
    ModernUI.cpp

    _OrzEx/database.cpp
    _OrzEx/database_sqlite.cpp
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

target_link_libraries(Def PUBLIC mir_sqlite ws2_32 advapi32 odbc32 odbccp32)

# List.cpp / Queue.cpp / DynamicArray.cpp �?VC6 遗留实现，与现行头文件冲突，不编�?

if(MIR3_BUILD_EISERVER)
    target_compile_definitions(Def PRIVATE MIR3_EISERVER)
endif()
