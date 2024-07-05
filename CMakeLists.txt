cmake_minimum_required(VERSION 3.10)

project(SPN)

include(cmake/CPM.cmake)
aux_source_directory(source/src SOURCES)
aux_source_directory(source/port/posix PORT_SOURCES)
aux_source_directory(source/test TEST_SOURCES)

add_library(SPN ${SOURCES})
target_include_directories(SPN PUBLIC source/inc)
add_executable(main ${PORT_SOURCES})
target_link_libraries(main SPN)

# Test part
CPMAddPackage("gh:google/googletest#v1.14.0")
add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests SPN gtest_main)
target_include_directories(tests PUBLIC source/include)