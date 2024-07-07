set(LWIP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-part/lwip)
set(LWIP_CONTRIB_DIR ${LWIP_DIR}/contrib)

include(${LWIP_DIR}/contrib/ports/CMakeCommon.cmake)

set (LWIP_DEFINITIONS -DLWIP_DEBUG)
set (LWIP_INCLUDE_DIRS
    "${LWIP_DIR}/src/include"
    "${LWIP_CONTRIB_DIR}/"
    "${LWIP_CONTRIB_DIR}/ports/unix/port/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/port/posix"
)

set (LWIP_EXCLUDE_SLIPIF TRUE)
include(${LWIP_CONTRIB_DIR}/ports/unix/Filelists.cmake)
include(${LWIP_DIR}/src/Filelists.cmake)

add_library(lwip ${lwipnoapps_SRCS} ${lwipcontribportunix_SRCS} ${lwipcontribportunixnetifs_SRCS})
target_compile_options(lwip PUBLIC ${LWIP_COMPILER_FLAGS})
target_compile_definitions(lwip PUBLIC ${LWIP_DEFINITIONS} ${LWIP_MBEDTLS_DEFINITIONS})
target_include_directories(lwip PUBLIC ${LWIP_INCLUDE_DIRS} ${LWIP_MBEDTLS_INCLUDE_DIRS})
target_link_libraries(lwip ${LWIP_SANITIZER_LIBS})

find_library(LIBPTHREAD pthread)
target_link_libraries(lwip ${LIBPTHREAD})