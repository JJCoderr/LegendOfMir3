# =============================================================================
#  Server/ManConsole —— MFC 管理控制台（对应旧 Server/ManConsole/ManConsole.dsp）
#  产物：ManConsole.exe
#
#  使用共享 MFC（对应旧工程 Use_MFC=6 / _AFXDLL），因此必须用动态 CRT（/MD、/MDd）。
#  需要 Visual Studio 的 “C++ MFC” 组件。
# =============================================================================

set(MIR3_MANCONSOLE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/ConsoleSocket.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ManConsole.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ManConsoleDlg.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ManConsole.rc"
)

# 共享 MFC（仅作用于本目录）
set(CMAKE_MFC_FLAG 2)

add_executable(ManConsole WIN32 ${MIR3_MANCONSOLE_SOURCES})

target_include_directories(ManConsole PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

# MFC 共享版本必须使用动态 CRT
set_target_properties(ManConsole PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)

target_link_libraries(ManConsole PRIVATE
    ws2_32 comctl32 winmm wininet
    ole32 oleaut32 uuid
    advapi32 user32 gdi32 shell32 comdlg32 winspool
)
