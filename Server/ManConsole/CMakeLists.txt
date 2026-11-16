include(CheckIncludeFileCXX)
check_include_file_cxx(afxwin.h MIR3_HAVE_MFC)

if(NOT MIR3_HAVE_MFC)
	message(STATUS "afxwin.h not found, ManConsole skipped (install the C++ MFC component to build it)")
else()
	add_executable(ManConsole WIN32
		ConsoleSocket.cpp
		ManConsole.cpp
		ManConsoleDlg.cpp
		StdAfx.cpp
		ManConsole.rc
	)
endif()