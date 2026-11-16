
cmake_minimum_required(VERSION 4.3.1)

project(LegendOfMir3 VERSION 0.1 LANGUAGES CXX)

enable_language(RC)
enable_language(C)

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

add_compile_options(
    #/W3
    /EHsc
    /wd4996   # CRT 安全函数弃用提示
    /wd4267   # size_t -> int
    /wd4244   # 数值隐式窄化
    /wd4101   # 未引用的局部变量
    /wd4838   # 指针/整数转换
)

# 大量 VC6 时代代码依赖非标准 for 作用域（for (int i...) 之后继续使用 i）。
# 注意：不能写进 add_compile_options —— VS 生成器会把它映射成工程属性
# ForceConformanceInForLoopScope，在 VS2026 生成器下该属性会写成空值从而丢失选项，
# 导致 C2065 错误。直接并入 CXX 编译标志，可原样传到 AdditionalOptions。
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:forScope-")

# 现代外观（Common Controls 6.0 主题 / DPI 感知 / 兼容性）：
# 清单文件是仓库根目录的 app.manifest，各程序在自己的 *.rc 里以
#   1 RT_MANIFEST "..\..\app.manifest"
# 嵌入，并加 /MANIFEST:NO 关掉链接器自动生成的清单。
add_subdirectory(Client)
add_subdirectory(Server)
