
add_library(AdminCmd SHARED 
	AdminCmd.cpp
	StdAfx.cpp
	AdminCmd.rc
)

target_compile_definitions(AdminCmd PRIVATE _USRDLL ADMINCMD_EXPORTS)
target_include_directories(AdminCmd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
