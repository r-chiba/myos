#set(CMAKE_C_FLAGS
#        "${CMAKE_C_FLAGS} \
#         -fpie"
#)
#set(CMAKE_C_LINK_EXECUTABLE
#        "/usr/bin/ld.lld-10 \
#         <CMAKE_C_LINK_FLAGS> \
#         <LINK_FLAGS> \
#         -pie \
#         -T ${CMAKE_CURRENT_SOURCE_DIR}/<TARGET>.ld \
#         -o <TARGET> \
#         <OBJECTS> \
#         <LINK_LIBRARIES>")
set(CMAKE_C_LINK_EXECUTABLE
        "/usr/bin/ld.lld-10 \
         <CMAKE_C_LINK_FLAGS> \
         <LINK_FLAGS> \
         -T ${CMAKE_CURRENT_SOURCE_DIR}/<TARGET>.ld \
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
add_executable(kernel ${SRCS_ASM} ${SRCS_C})
target_include_directories(kernel
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include/
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include/kernel
)
