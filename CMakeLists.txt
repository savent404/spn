cmake_minimum_required(VERSION 3.10)

project(SPN)

include(cmake/CPM.cmake)
include(cmake/lwip.cmake)

option(ENABLE_TESTS "Enable test" OFF)
option(ENABLE_EXAMPLES "Enable example" OFF)

# SPN library
# recursive search for source files and set to SOURCES
file(GLOB_RECURSE SOURCES source/src/*.c)
add_library(SPN ${SOURCES})
target_compile_options(SPN PRIVATE ${LWIP_COMPILER_FLAGS})
target_include_directories(SPN PUBLIC source/inc)
target_link_libraries(SPN PUBLIC lwip)

add_custom_target(size ALL
    COMMAND ${CMAKE_SIZE_TOOL} $<TARGET_FILE:SPN> -t
    DEPENDS SPN
    COMMENT "Print size of SPN"
)

# EXAMPLE MAIN
if (ENABLE_EXAMPLES)
    aux_source_directory(source/port/posix PORT_SOURCES)
    add_executable(main ${PORT_SOURCES} ${SOURCES})
    target_include_directories(main PUBLIC source/inc)
    target_link_libraries(main lwip pthread)
endif (ENABLE_EXAMPLES)

# Test part
if (ENABLE_TESTS)
    aux_source_directory(source/test TEST_SOURCES)
    CPMAddPackage("gh:google/googletest#v1.14.0")
    add_executable(tests ${TEST_SOURCES})
    target_link_libraries(tests PRIVATE gtest_main SPN pthread)
    target_include_directories(tests PRIVATE source/port/posix)

    if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
        include(cmake/CodeCoverage.cmake)
        # For tests
        setup_target_for_coverage(tests_cov tests coverage)
    endif()
endif (ENABLE_TESTS)