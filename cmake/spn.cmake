if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux")
    set(SPN_C_FLAGS ${LWIP_COMPILER_FLAGS} 
        -Wno-c90-c99-compat
        -Wno-pedantic
        -Wno-redundant-decls)
else ()
    set(SPN_C_FLAGS "")
endif()

# recursive search for source files and set to SOURCES
file(GLOB_RECURSE SOURCES source/src/*.c)
add_library(SPN ${SOURCES})
target_compile_options(SPN PRIVATE ${SPN_C_FLAGS})
target_include_directories(SPN PUBLIC
    source/inc
    ${LWIP_INCLUDE_DIRS}
    ${LWIP_PORT_INC})
target_link_libraries(SPN PUBLIC ${LWIP_PORT_LIB})

if (CMAKE_SIZE_TOOL)
add_custom_target(size ALL
    COMMAND ${CMAKE_SIZE_TOOL} $<TARGET_FILE:SPN> -t
    DEPENDS SPN
    COMMENT "Print size of SPN"
)
endif ()
