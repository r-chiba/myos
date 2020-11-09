set(CMAKE_C_LINK_EXECUTABLE "/usr/bin/ld.lld-10 <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> -T ${CMAKE_CURRENT_SOURCE_DIR}/<TARGET>.ld <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

file(GLOB_RECURSE SRCS_C "*.c")
list(APPEND SRCS_C
    "${CMAKE_CURRENT_SOURCE_DIR}/../../stand/libsa/printf.c"
)
add_library(c STATIC ${SRCS_C})
target_include_directories(c
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../../../include/
)
