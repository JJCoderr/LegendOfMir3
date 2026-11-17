cmake_minimum_required(VERSION 4.3.1)

project(LegendOfMir3_Server VERSION 0.1 LANGUAGES CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(Def)
link_libraries(Def)

add_subdirectory(AdminCmd)
add_subdirectory(DBSvr)
add_subdirectory(GameSvr)
add_subdirectory(GameGate)
add_subdirectory(LoginGate)
add_subdirectory(LoginSvr)
add_subdirectory(SelGate)
add_subdirectory(ManConsole)
add_subdirectory(VirtualClient)
add_subdirectory(Tools)

add_subdirectory(Service)
if(MIR3_BUILD_EISERVER)
    add_executable(EIServer EIServer.cpp "${MIR3_SERVICE_STRINGS}")
    target_include_directories(EIServer PRIVATE . Def Def/_OrzEx)
    target_compile_definitions(EIServer PRIVATE UNICODE _UNICODE MIR3_EISERVER)
    target_link_libraries(EIServer PRIVATE ${core_libraries} mir_sqlite ws2_32 advapi32 odbc32 odbccp32 comctl32 iphlpapi)

endif()
