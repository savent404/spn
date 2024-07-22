set(LWIP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-part/lwip)
set(LWIP_CONTRIB_DIR ${LWIP_DIR}/contrib)

#add_subdirectory(${LWIP_DIR})
add_subdirectory(${LWIP_CONTRIB_DIR}/ports/unix/lib)

set (LWIP_INCLUDE_DIRS
    "${LWIP_DIR}/src/include"
    "${LWIP_DIR}/contrib/"
)

if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux"
    OR ${CMAKE_HOST_SYSTEM_NAME} MATCHES "Darwin")
    set(LWIP_COMPILER_FLAGS
        -Wno-c90-c99-compat
        -Wno-pedantic
        -Wno-redundant-decls)
    set(LWIP_PORT_INC 
        "${LWIP_DIR}/contrib/ports/unix/port/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/port/posix")
    set(LWIP_PORT_LIB
        lwip)
else ()
    message(FATAL_ERROR "Unsupported system")
endif ()
