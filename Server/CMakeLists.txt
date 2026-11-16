cmake_minimum_required(VERSION 4.3.1)

project(LegendOfMir3_Server VERSION 0.1 LANGUAGES CXX)

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
