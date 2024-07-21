cmake_minimum_required(VERSION 3.10)

project(SPN)

include(cmake/CPM.cmake)
include(cmake/lwip.cmake)
include(cmake/spn.cmake)

option(ENABLE_TESTS "Enable test" OFF)
option(ENABLE_EXAMPLES "Enable example" OFF)

# SPN library in cmake/spn.cmake

# EXAMPLE MAIN
if (ENABLE_EXAMPLES)
    aux_source_directory(source/port/posix PORT_SOURCES)
    add_executable(main ${PORT_SOURCES} ${SOURCES})
    target_include_directories(main PUBLIC source/inc)
    target_link_libraries(main SPN)
endif (ENABLE_EXAMPLES)

# Test part
if (ENABLE_TESTS)
    aux_source_directory(source/test TEST_SOURCES)
    CPMAddPackage("gh:google/googletest#v1.14.0")
    add_executable(tests ${TEST_SOURCES})
    target_link_libraries(tests PRIVATE gtest_main SPN)

    if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
        include(cmake/CodeCoverage.cmake)
        # For tests
        setup_target_for_coverage(tests_cov tests coverage)
    endif()
endif (ENABLE_TESTS)
