
cmake_minimum_required(VERSION 4.3.1)

project(LegendOfMir3 VERSION 0.1 LANGUAGES CXX)

enable_language(RC)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/")
set(CMAKE_DEBUG_POSTFIX "d")

# cxx
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_compile_definitions(WIN32 _CRT_SECURE_NO_WARNINGS _WINDOWS _WINDOWS _CRT _MBCS _NON_CONFORMING_SWPRINTFS _WINSOCK_DEPRECATED_NO_WARNINGS)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/MP>)
#add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/utf-8>)
#add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/WX>)

target_compile_options(${target} PRIVATE
    /W3
    /EHsc
    # 代码大量依赖 VC6 时代的非标准 for 作用域（for (int i...) 之后继续使用 i）
    /Zc:forScope-
    /wd4996   # CRT 安全函数弃用提示
    /wd4267   # size_t -> int
    /wd4244   # 数值隐式窄化
    /wd4101   # 未引用的局部变量
    /wd4838   # 指针/整数转换
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(WARNING "代码按 32 位 Win32 编写，请使用 -A Win32 重新配置。")
endif()

add_subdirectory(Client)
add_subdirectory(Server)
