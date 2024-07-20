set(LWIP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-part/lwip)
set(LWIP_CONTRIB_DIR ${LWIP_DIR}/contrib)

include(${LWIP_DIR}/contrib/ports/CMakeCommon.cmake)

set (LWIP_DEFINITIONS -DLWIP_DEBUG)

if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux")
    set (LWIP_PORT_INC 
        "${LWIP_CONTRIB_DIR}/ports/unix/port/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/port/posix")
    set (LWIP_PORT_SRC
        "${lwipcontribportunix_SRCS}"
        "${lwipcontribportunixnetifs_SRCS}")
    set (LWIP_PORT_LIB lwipcontribportunix)
else ()
    set (LWIP_PORT_INC 
        "${LWIP_CONTRIB_DIR}/ports/win32/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/port/win32")
    set (LWIP_PORT_SRC
        "${lwipcontribportwin32_SRCS}"
        "${lwipcontribportwin32netifs_SRCS}")
endif ()
set (LWIP_INCLUDE_DIRS
    "${LWIP_DIR}/src/include"
    "${LWIP_CONTRIB_DIR}/"
    ${LWIP_PORT_INC}
)

set (LWIP_EXCLUDE_SLIPIF TRUE)
include(${LWIP_CONTRIB_DIR}/ports/unix/Filelists.cmake)
include(${LWIP_DIR}/src/Filelists.cmake)

add_library(lwip ${lwipnoapps_SRCS} ${lwipsnmp_SRCS} ${LWIP_PORT_SRC})
target_compile_options(lwip PRIVATE ${LWIP_COMPILER_FLAGS})
target_compile_definitions(lwip PUBLIC ${LWIP_DEFINITIONS} ${LWIP_MBEDTLS_DEFINITIONS})
target_include_directories(lwip PUBLIC ${LWIP_INCLUDE_DIRS} ${LWIP_MBEDTLS_INCLUDE_DIRS})
target_link_libraries(lwip ${LWIP_SANITIZER_LIBS} ${LWIP_PORT_LIB})
