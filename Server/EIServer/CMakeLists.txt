set(modules DBSvr LoginSvr GameSvr LoginGate SelGate GameGate AdminCmd)
set(resources "#pragma code_page(65001)\n")
set(resource_id 1)
foreach(module IN LISTS modules)
    string(APPEND resources "${resource_id} RCDATA \"$<TARGET_FILE:${module}>\"\n")
    math(EXPR resource_id "${resource_id} + 1")
endforeach()
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/Modules.rc" CONTENT "${resources}")
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/Embedded.rc"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/Modules.rc"
        "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/Embedded.rc"
    DEPENDS ${modules} "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/Modules.rc")
add_executable(EIServer WIN32 Main.cpp "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/Embedded.rc")
target_link_libraries(EIServer PRIVATE iphlpapi)

target_compile_definitions(EIServer PRIVATE UNICODE _UNICODE)
