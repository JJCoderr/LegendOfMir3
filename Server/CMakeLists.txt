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

add_subdirectory(EIServer)
