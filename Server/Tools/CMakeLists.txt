add_executable(EncodeSize EncodeSize.cpp)
target_include_directories(EncodeSize PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../Def ${CMAKE_CURRENT_SOURCE_DIR}/../Def/_OrzEx)

add_executable(test_encode test_encode.cpp)
target_include_directories(test_encode PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../Def ${CMAKE_CURRENT_SOURCE_DIR}/../Def/_OrzEx)
