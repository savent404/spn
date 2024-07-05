cmake_minimum_required(VERSION 3.10)

project(SPN)

include(cmake/CPM.cmake)
CPMAddPackage("gh:lwip-tcpip/lwip#STABLE-2_0_3_RELEASE")

aux_source_directory(source/src SOURCES)
aux_source_directory(source/port/posix PORT_SOURCES)

add_library(SPN ${SOURCES})
add_executable(main ${PORT_SOURCES})
target_link_libraries(main SPN)