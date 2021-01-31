#set(CMAKE_C_FLAGS
#        "${CMAKE_C_FLAGS} \
#         -mcmodel=large \
#         -fpie"
#)
set(CMAKE_C_FLAGS
        "${CMAKE_C_FLAGS} \
         -mcmodel=large"
)
#set(CMAKE_C_LINK_EXECUTABLE
#        "/usr/bin/ld.lld-10 \
#         <CMAKE_C_LINK_FLAGS> \
#         <LINK_FLAGS> \
#         -T ${CMAKE_CURRENT_SOURCE_DIR}/<TARGET>.ld \
#         -pie \
#         -o <TARGET> \
#         <OBJECTS> \
#         <LINK_LIBRARIES>")
set(CMAKE_C_LINK_EXECUTABLE
        "/usr/bin/ld.lld-10 \
         <CMAKE_C_LINK_FLAGS> \
         <LINK_FLAGS> \
         -T ${CMAKE_CURRENT_BINARY_DIR}/<TARGET>.ld \
         -o <TARGET> \
         <OBJECTS> \
         <LINK_LIBRARIES>")

#file(GLOB_RECURSE SRCS_ASM "*.asm")
#set_source_files_properties(${SRCS_ASM} PROPERTIES
#    LANGUAGE ASM_NASM
#    LINKER_LANGUAGE C)

file(GLOB_RECURSE SRCS_C "*.c")
list(APPEND SRCS_C
    "${CMAKE_CURRENT_SOURCE_DIR}/../freebsd/stand/libsa/printf.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../freebsd/lib/libc/string/strlen.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../freebsd/lib/libc/string/memcpy.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../freebsd/lib/libc/string/memset.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../freebsd/lib/libc/string/memcmp.c")
file(GLOB_RECURSE SRCS_ASM "*.S")
set_source_files_properties(${SRCS_ASM} PROPERTIES
    LANGUAGE C)
add_custom_target(ldscript
    COMMAND ${CMAKE_C_COMPILER}
                -E -x c -I${CMAKE_CURRENT_SOURCE_DIR}/../../include/kernel
                ${CMAKE_CURRENT_SOURCE_DIR}/kernel.ld
            | grep -v '^\#' > ${CMAKE_CURRENT_BINARY_DIR}/kernel.ld
)
add_compile_definitions(KERNEL=1)
add_executable(kernel ${SRCS_ASM} ${SRCS_C})
target_include_directories(kernel
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include/
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include/kernel
)
add_dependencies(kernel ldscript)
